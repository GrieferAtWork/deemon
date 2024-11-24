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

typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_foreach_reverse_t)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_enumerate_index_t)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_enumerate_index_reverse_t)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_trygetfirst_t)(DeeObject *__restrict self); /* @return: ITER_DONE: Sequence is empty */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_getfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_boundfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_delfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_setfirst_t)(DeeObject *self, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_trygetlast_t)(DeeObject *__restrict self); /* @return: ITER_DONE: Sequence is empty */
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

	/* Operators... (TODO: `tsc_operator_iter' should replace `generic_seq_iter' from `seq.c') */
	Dee_tsc_operator_bool_t             tsc_operator_bool;
	Dee_tsc_operator_iter_t             tsc_operator_iter;
	Dee_tsc_operator_sizeob_t           tsc_operator_sizeob;
	Dee_tsc_operator_contains_t         tsc_operator_contains;
	Dee_tsc_operator_getitem_t          tsc_operator_getitem;
	Dee_tsc_operator_delitem_t          tsc_operator_delitem;
	Dee_tsc_operator_setitem_t          tsc_operator_setitem;
	Dee_tsc_operator_getrange_t         tsc_operator_getrange;
	Dee_tsc_operator_delrange_t         tsc_operator_delrange;
	Dee_tsc_operator_setrange_t         tsc_operator_setrange;
	Dee_tsc_operator_foreach_t          tsc_operator_foreach;
	Dee_tsc_operator_enumerate_t        tsc_operator_enumerate;
	Dee_tsc_operator_enumerate_index_t  tsc_operator_enumerate_index;
	Dee_tsc_operator_bounditem_t        tsc_operator_bounditem;
	Dee_tsc_operator_hasitem_t          tsc_operator_hasitem;
	Dee_tsc_operator_size_t             tsc_operator_size;
	Dee_tsc_operator_size_fast_t        tsc_operator_size_fast;
	Dee_tsc_operator_getitem_index_t    tsc_operator_getitem_index;
	Dee_tsc_operator_delitem_index_t    tsc_operator_delitem_index;
	Dee_tsc_operator_setitem_index_t    tsc_operator_setitem_index;
	Dee_tsc_operator_bounditem_index_t  tsc_operator_bounditem_index;
	Dee_tsc_operator_hasitem_index_t    tsc_operator_hasitem_index;
	Dee_tsc_operator_getrange_index_t   tsc_operator_getrange_index;
	Dee_tsc_operator_delrange_index_t   tsc_operator_delrange_index;
	Dee_tsc_operator_setrange_index_t   tsc_operator_setrange_index;
	Dee_tsc_operator_getrange_index_n_t tsc_operator_getrange_index_n;
	Dee_tsc_operator_delrange_index_n_t tsc_operator_delrange_index_n;
	Dee_tsc_operator_setrange_index_n_t tsc_operator_setrange_index_n;
	Dee_tsc_operator_trygetitem_t       tsc_operator_trygetitem;
	Dee_tsc_operator_trygetitem_index_t tsc_operator_trygetitem_index;
	Dee_tsc_operator_hash_t             tsc_operator_hash;
	Dee_tsc_operator_compare_eq_t       tsc_operator_compare_eq;
	Dee_tsc_operator_compare_t          tsc_operator_compare;
	Dee_tsc_operator_trycompare_eq_t    tsc_operator_trycompare_eq;
	Dee_tsc_operator_eq_t               tsc_operator_eq;
	Dee_tsc_operator_ne_t               tsc_operator_ne;
	Dee_tsc_operator_lo_t               tsc_operator_lo;
	Dee_tsc_operator_le_t               tsc_operator_le;
	Dee_tsc_operator_gr_t               tsc_operator_gr;
	Dee_tsc_operator_ge_t               tsc_operator_ge;
	Dee_tsc_operator_inplace_add_t      tsc_operator_inplace_add;
	Dee_tsc_operator_inplace_mul_t      tsc_operator_inplace_mul;

	/* Common utility functions... */
	Dee_tsc_foreach_reverse_t         tsc_foreach_reverse;
	Dee_tsc_enumerate_index_t         tsc_enumerate_index; /* Same as normal enumerate-index, but treated like `(self as Sequence).<enumerate_index>' */
	Dee_tsc_enumerate_index_reverse_t tsc_enumerate_index_reverse;

	/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
	Dee_tsc_makeenumeration_t                tsc_makeenumeration;
	Dee_tsc_makeenumeration_with_int_range_t tsc_makeenumeration_with_int_range;
	Dee_tsc_makeenumeration_with_range_t     tsc_makeenumeration_with_range;

	/* Returns the first element of the sequence.
	 * Calls `err_empty_sequence()' when it is empty. */
	Dee_tsc_trygetfirst_t tsc_trygetfirst;
	Dee_tsc_getfirst_t    tsc_getfirst;
	union Dee_tsc_uslot   tsc_getfirst_data;
	Dee_tsc_boundfirst_t  tsc_boundfirst;
	Dee_tsc_delfirst_t    tsc_delfirst;
	union Dee_tsc_uslot   tsc_delfirst_data;
	Dee_tsc_setfirst_t    tsc_setfirst;
	union Dee_tsc_uslot   tsc_setfirst_data;

	/* Returns the last element of the sequence.
	 * Calls `err_empty_sequence()' when it is empty. */
	Dee_tsc_trygetlast_t tsc_trygetlast;
	Dee_tsc_getlast_t    tsc_getlast;
	union Dee_tsc_uslot  tsc_getlast_data;
	Dee_tsc_boundlast_t  tsc_boundlast;
	Dee_tsc_dellast_t    tsc_dellast;
	union Dee_tsc_uslot  tsc_dellast_data;
	Dee_tsc_setlast_t    tsc_setlast;
	union Dee_tsc_uslot  tsc_setlast_data;

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
	/* For `deemon.Set'                                                     */
	/************************************************************************/
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
INTDEF WUNUSED NONNULL((1)) Dee_tsc_foreach_reverse_t DCALL DeeType_SeqCache_TryRequireForeachReverse(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_reverse_t DCALL DeeType_SeqCache_TryRequireEnumerateIndexReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL DeeType_SeqCache_HasPrivateEnumerateIndexReverse(DeeTypeObject *orig_type, DeeTypeObject *self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_t DCALL DeeType_SeqCache_RequireEnumerateIndex(DeeTypeObject *__restrict self);

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_t DCALL DeeType_SeqCache_RequireMakeEnumeration(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_with_int_range_t DCALL DeeType_SeqCache_RequireMakeEnumerationWithIntRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_with_range_t DCALL DeeType_SeqCache_RequireMakeEnumerationWithRange(DeeTypeObject *__restrict self);

/* Sequence operators... */
/*[[[begin:seq_operators]]]*/
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_bool_t DCALL DeeType_SeqCache_RequireOperatorBool(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_iter_t DCALL DeeType_SeqCache_RequireOperatorIter(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_sizeob_t DCALL DeeType_SeqCache_RequireOperatorSizeOb(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_contains_t DCALL DeeType_SeqCache_RequireOperatorContains(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getitem_t DCALL DeeType_SeqCache_RequireOperatorGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delitem_t DCALL DeeType_SeqCache_RequireOperatorDelItem(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setitem_t DCALL DeeType_SeqCache_RequireOperatorSetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getrange_t DCALL DeeType_SeqCache_RequireOperatorGetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delrange_t DCALL DeeType_SeqCache_RequireOperatorDelRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setrange_t DCALL DeeType_SeqCache_RequireOperatorSetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_foreach_t DCALL DeeType_SeqCache_RequireOperatorForeach(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_enumerate_t DCALL DeeType_SeqCache_RequireOperatorEnumerate(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_enumerate_index_t DCALL DeeType_SeqCache_RequireOperatorEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_bounditem_t DCALL DeeType_SeqCache_RequireOperatorBoundItem(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_hasitem_t DCALL DeeType_SeqCache_RequireOperatorHasItem(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_size_t DCALL DeeType_SeqCache_RequireOperatorSize(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_size_fast_t DCALL DeeType_SeqCache_RequireOperatorSizeFast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getitem_index_t DCALL DeeType_SeqCache_RequireOperatorGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delitem_index_t DCALL DeeType_SeqCache_RequireOperatorDelItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setitem_index_t DCALL DeeType_SeqCache_RequireOperatorSetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_bounditem_index_t DCALL DeeType_SeqCache_RequireOperatorBoundItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_hasitem_index_t DCALL DeeType_SeqCache_RequireOperatorHasItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getrange_index_t DCALL DeeType_SeqCache_RequireOperatorGetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delrange_index_t DCALL DeeType_SeqCache_RequireOperatorDelRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setrange_index_t DCALL DeeType_SeqCache_RequireOperatorSetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getrange_index_n_t DCALL DeeType_SeqCache_RequireOperatorGetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delrange_index_n_t DCALL DeeType_SeqCache_RequireOperatorDelRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setrange_index_n_t DCALL DeeType_SeqCache_RequireOperatorSetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_trygetitem_t DCALL DeeType_SeqCache_RequireOperatorTryGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_trygetitem_index_t DCALL DeeType_SeqCache_RequireOperatorTryGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_hash_t DCALL DeeType_SeqCache_RequireOperatorHash(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_compare_eq_t DCALL DeeType_SeqCache_RequireOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_compare_t DCALL DeeType_SeqCache_RequireOperatorCompare(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_trycompare_eq_t DCALL DeeType_SeqCache_RequireOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_eq_t DCALL DeeType_SeqCache_RequireOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_ne_t DCALL DeeType_SeqCache_RequireOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_lo_t DCALL DeeType_SeqCache_RequireOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_le_t DCALL DeeType_SeqCache_RequireOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_gr_t DCALL DeeType_SeqCache_RequireOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_ge_t DCALL DeeType_SeqCache_RequireOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_inplace_add_t DCALL DeeType_SeqCache_RequireOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_inplace_mul_t DCALL DeeType_SeqCache_RequireOperatorInplaceMul(DeeTypeObject *__restrict self);
/*[[[end:seq_operators]]]*/

/*
 * List default implementations returned by `DeeType_SeqCache_RequireOperator*'
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
	local START_MARKER = "DeeType_SeqCache_RequireOperator";
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
		f"-DDEFINE_DeeType_SeqCache_RequireOperator{name}",
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
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_trygetfirst_t DCALL DeeType_SeqCache_RequireTryGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getfirst_t DCALL DeeType_SeqCache_RequireGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundfirst_t DCALL DeeType_SeqCache_RequireBoundFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_delfirst_t DCALL DeeType_SeqCache_RequireDelFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setfirst_t DCALL DeeType_SeqCache_RequireSetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_trygetlast_t DCALL DeeType_SeqCache_RequireTryGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getlast_t DCALL DeeType_SeqCache_RequireGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundlast_t DCALL DeeType_SeqCache_RequireBoundLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_dellast_t DCALL DeeType_SeqCache_RequireDelLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setlast_t DCALL DeeType_SeqCache_RequireSetLast(DeeTypeObject *__restrict self);
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

/* Set functions */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_insert_t DCALL DeeType_SeqCache_RequireSetInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_remove_t DCALL DeeType_SeqCache_RequireSetRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_unify_t DCALL DeeType_SeqCache_RequireSetUnify(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_insertall_t DCALL DeeType_SeqCache_RequireSetInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_removeall_t DCALL DeeType_SeqCache_RequireSetRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_pop_t DCALL DeeType_SeqCache_RequireSetPop(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_pop_with_default_t DCALL DeeType_SeqCache_RequireSetPopWithDefault(DeeTypeObject *__restrict self);

/* Map functions */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setold_t DCALL DeeType_SeqCache_RequireMapSetOld(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setold_ex_t DCALL DeeType_SeqCache_RequireMapSetOldEx(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setnew_t DCALL DeeType_SeqCache_RequireMapSetNew(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setnew_ex_t DCALL DeeType_SeqCache_RequireMapSetNewEx(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setdefault_t DCALL DeeType_SeqCache_RequireMapSetDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_update_t DCALL DeeType_SeqCache_RequireMapUpdate(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_remove_t DCALL DeeType_SeqCache_RequireMapRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_removekeys_t DCALL DeeType_SeqCache_RequireMapRemoveKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_pop_t DCALL DeeType_SeqCache_RequireMapPop(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_pop_with_default_t DCALL DeeType_SeqCache_RequireMapPopWithDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_popitem_t DCALL DeeType_SeqCache_RequireMapPopItem(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_keys_t DCALL DeeType_SeqCache_RequireMapKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_values_t DCALL DeeType_SeqCache_RequireMapValues(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_iterkeys_t DCALL DeeType_SeqCache_RequireMapIterKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_itervalues_t DCALL DeeType_SeqCache_RequireMapIterValues(DeeTypeObject *__restrict self);


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



/* Generic sequence operators: treat "self" as a (possibly writable) indexable sequence.
 *
 * When "self" doesn't override any sequence operators, throw errors (unless
 * "self" explicitly uses operators from "Sequence", in which case it is treated
 * like an empty sequence).
 *
 * For this purpose, trust the return value of `DeeType_GetSeqClass()',
 * and wrap/modify operator invocation such that the object behaves as
 * though it was an indexable sequence. */
#define DeeSeq_OperatorBool(self)                                  (*DeeType_SeqCache_RequireOperatorBool(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorIter(self)                                  (*DeeType_SeqCache_RequireOperatorIter(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorSizeOb(self)                                (*DeeType_SeqCache_RequireOperatorSizeOb(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorContains(self, some_object)                 (*DeeType_SeqCache_RequireOperatorContains(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGetItem(self, index)                        (*DeeType_SeqCache_RequireOperatorGetItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorDelItem(self, index)                        (*DeeType_SeqCache_RequireOperatorDelItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSetItem(self, index, value)                 (*DeeType_SeqCache_RequireOperatorSetItem(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_OperatorGetRange(self, start, end)                  (*DeeType_SeqCache_RequireOperatorGetRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorDelRange(self, start, end)                  (*DeeType_SeqCache_RequireOperatorDelRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorSetRange(self, start, end, values)          (*DeeType_SeqCache_RequireOperatorSetRange(Dee_TYPE(self)))(self, start, end, values)
#define DeeSeq_OperatorForeach(self, proc, arg)                    (*DeeType_SeqCache_RequireOperatorForeach(Dee_TYPE(self)))(self, proc, arg)
#define DeeSeq_OperatorEnumerate(self, proc, arg)                  (*DeeType_SeqCache_RequireOperatorEnumerate(Dee_TYPE(self)))(self, proc, arg)
#define DeeSeq_OperatorEnumerateIndex(self, proc, arg, start, end) (*DeeType_SeqCache_RequireOperatorEnumerateIndex(Dee_TYPE(self)))(self, proc, arg, start, end)
#define DeeSeq_OperatorBoundItem(self, index)                      (*DeeType_SeqCache_RequireOperatorBoundItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHasItem(self, index)                        (*DeeType_SeqCache_RequireOperatorHasItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSize(self)                                  (*DeeType_SeqCache_RequireOperatorSize(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorSizeFast(self)                              (*DeeType_SeqCache_RequireOperatorSizeFast(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorGetItemIndex(self, index)                   (*DeeType_SeqCache_RequireOperatorGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorDelItemIndex(self, index)                   (*DeeType_SeqCache_RequireOperatorDelItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSetItemIndex(self, index, value)            (*DeeType_SeqCache_RequireOperatorSetItemIndex(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_OperatorBoundItemIndex(self, index)                 (*DeeType_SeqCache_RequireOperatorBoundItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHasItemIndex(self, index)                   (*DeeType_SeqCache_RequireOperatorHasItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorGetRangeIndex(self, start, end)             (*DeeType_SeqCache_RequireOperatorGetRangeIndex(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorDelRangeIndex(self, start, end)             (*DeeType_SeqCache_RequireOperatorDelRangeIndex(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorSetRangeIndex(self, start, end, values)     (*DeeType_SeqCache_RequireOperatorSetRangeIndex(Dee_TYPE(self)))(self, start, end, values)
#define DeeSeq_OperatorGetRangeIndexN(self, start)                 (*DeeType_SeqCache_RequireOperatorGetRangeIndexN(Dee_TYPE(self)))(self, start)
#define DeeSeq_OperatorDelRangeIndexN(self, start)                 (*DeeType_SeqCache_RequireOperatorDelRangeIndexN(Dee_TYPE(self)))(self, start)
#define DeeSeq_OperatorSetRangeIndexN(self, start, values)         (*DeeType_SeqCache_RequireOperatorSetRangeIndexN(Dee_TYPE(self)))(self, start, values)
#define DeeSeq_OperatorTryGetItem(self, index)                     (*DeeType_SeqCache_RequireOperatorTryGetItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorTryGetItemIndex(self, index)                (*DeeType_SeqCache_RequireOperatorTryGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHash(self)                                  (*DeeType_SeqCache_RequireOperatorHash(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorCompareEq(self, some_object)                (*DeeType_SeqCache_RequireOperatorCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorCompare(self, some_object)                  (*DeeType_SeqCache_RequireOperatorCompare(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorTryCompareEq(self, some_object)             (*DeeType_SeqCache_RequireOperatorTryCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorEq(self, some_object)                       (*DeeType_SeqCache_RequireOperatorEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorNe(self, some_object)                       (*DeeType_SeqCache_RequireOperatorNe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorLo(self, some_object)                       (*DeeType_SeqCache_RequireOperatorLo(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorLe(self, some_object)                       (*DeeType_SeqCache_RequireOperatorLe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGr(self, some_object)                       (*DeeType_SeqCache_RequireOperatorGr(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGe(self, some_object)                       (*DeeType_SeqCache_RequireOperatorGe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorInplaceAdd(p_self, some_object)             (*DeeType_SeqCache_RequireOperatorInplaceAdd(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeSeq_OperatorInplaceMul(p_self, some_object)             (*DeeType_SeqCache_RequireOperatorInplaceMul(Dee_TYPE(*(p_self))))(p_self, some_object)
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
#define DeeSeq_TryGetFirst(self) (*DeeType_SeqCache_RequireTryGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_GetFirst(self)    (*DeeType_SeqCache_RequireGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_BoundFirst(self)  (*DeeType_SeqCache_RequireBoundFirst(Dee_TYPE(self)))(self)
#define DeeSeq_DelFirst(self)    (*DeeType_SeqCache_RequireDelFirst(Dee_TYPE(self)))(self)
#define DeeSeq_SetFirst(self, v) (*DeeType_SeqCache_RequireSetFirst(Dee_TYPE(self)))(self, v)
#define DeeSeq_TryGetLast(self)  (*DeeType_SeqCache_RequireTryGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_GetLast(self)     (*DeeType_SeqCache_RequireGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_BoundLast(self)   (*DeeType_SeqCache_RequireBoundLast(Dee_TYPE(self)))(self)
#define DeeSeq_DelLast(self)     (*DeeType_SeqCache_RequireDelLast(Dee_TYPE(self)))(self)
#define DeeSeq_SetLast(self, v)  (*DeeType_SeqCache_RequireSetLast(Dee_TYPE(self)))(self, v)
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

/* Set functions */
#define new_DeeSet_Insert(self, key)              (*DeeType_SeqCache_RequireSetInsert(Dee_TYPE(self)))(self, key)
#define new_DeeSet_Remove(self, key)              (*DeeType_SeqCache_RequireSetRemove(Dee_TYPE(self)))(self, key)
#define new_DeeSet_Unify(self, key)               (*DeeType_SeqCache_RequireSetUnify(Dee_TYPE(self)))(self, key)
#define new_DeeSet_InsertAll(self, keys)          (*DeeType_SeqCache_RequireSetInsertAll(Dee_TYPE(self)))(self, keys)
#define new_DeeSet_RemoveAll(self, keys)          (*DeeType_SeqCache_RequireSetRemoveAll(Dee_TYPE(self)))(self, keys)
#define new_DeeSet_Pop(self)                      (*DeeType_SeqCache_RequireSetPop(Dee_TYPE(self)))(self)
#define new_DeeSet_PopWithDefault(self, default_) (*DeeType_SeqCache_RequireSetPopWithDefault(Dee_TYPE(self)))(self, default_)

/* Map functions */
#define new_DeeMap_SetOld(self, key, value)            (*DeeType_SeqCache_RequireMapSetOld(Dee_TYPE(self)))(self, key, value)
#define new_DeeMap_SetOldEx(self, key, value)          (*DeeType_SeqCache_RequireMapSetOldEx(Dee_TYPE(self)))(self, key, value)
#define new_DeeMap_SetNew(self, key, value)            (*DeeType_SeqCache_RequireMapSetNew(Dee_TYPE(self)))(self, key, value)
#define new_DeeMap_SetNewEx(self, key, value)          (*DeeType_SeqCache_RequireMapSetNewEx(Dee_TYPE(self)))(self, key, value)
#define new_DeeMap_SetDefault(self, key, value)        (*DeeType_SeqCache_RequireMapSetDefault(Dee_TYPE(self)))(self, key, value)
#define new_DeeMap_Update(self, items)                 (*DeeType_SeqCache_RequireMapUpdate(Dee_TYPE(self)))(self, items)
#define new_DeeMap_Remove(self, key)                   (*DeeType_SeqCache_RequireMapRemove(Dee_TYPE(self)))(self, key)
#define new_DeeMap_RemoveKeys(self, keys)              (*DeeType_SeqCache_RequireMapRemoveKeys(Dee_TYPE(self)))(self, keys)
#define new_DeeMap_Pop(self, key)                      (*DeeType_SeqCache_RequireMapPop(Dee_TYPE(self)))(self, key)
#define new_DeeMap_PopWithDefault(self, key, default_) (*DeeType_SeqCache_RequireMapPopWithDefault(Dee_TYPE(self)))(self, key, default_)
#define new_DeeMap_PopItem(self)                       (*DeeType_SeqCache_RequireMapPopItem(Dee_TYPE(self)))(self)
#define new_DeeMap_Keys(self)                          (*DeeType_SeqCache_RequireMapKeys(Dee_TYPE(self)))(self)
#define new_DeeMap_Values(self)                        (*DeeType_SeqCache_RequireMapValues(Dee_TYPE(self)))(self)
#define new_DeeMap_IterKeys(self)                      (*DeeType_SeqCache_RequireMapIterKeys(Dee_TYPE(self)))(self)
#define new_DeeMap_IterValues(self)                    (*DeeType_SeqCache_RequireMapIterValues(Dee_TYPE(self)))(self)

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

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithTSCGetFirst(DeeObject *__restrict self);
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
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithTSCFirstAndTSCSetRemove(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithTSCFirstAndMapDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithCallAttrSetFirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithCallSetFirstDataFunction(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItemIndex(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItem(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithError(DeeObject *self, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithTSCGetLast(DeeObject *__restrict self);
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
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithTSCLastAndTSCSetRemove(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithTSCLastAndMapDelItem(DeeObject *__restrict self);
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
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithTSCRemoveAllForSet(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithTSCRemoveKeysForMap(DeeObject *self);
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



/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallAttrInsert(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithSizeAndTSCInsertAll(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithMapSetNew(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithSeqTSCContainsAndTSCAppend(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithSizeAndTSCRemoveAll(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithMapGetItemAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithSeqTSCRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallAttrUnify(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithTSCInsertAndForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithTSCLocateAndTSCAppend(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithInplaceAdd(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithInplaceOr(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithTSCInsert(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithInplaceSub(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithTSCRemove(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallAttrPop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithTSCFirstAndTSCRemove(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithMapPopItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithSeqPop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataFunction(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataMethod(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataKwMethod(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithTSCFirstAndTSCRemove(DeeObject *self, DeeObject *default_);
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
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithTSCSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value);
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
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithTSCSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithBoundItemAndTSCSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithBoundItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithTryGetItemAndTSCSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithGetItemAndTSCSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallAttrSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithTryGetItemAndTSCSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithGetItemAndTSCSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallAttrSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithTSCSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithTSCSetNewAndGetItem(DeeObject *self, DeeObject *key, DeeObject *value);
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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithSizeAndTSCRemoveKeys(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallAttrRemoveKeys(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithDelItem(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithTSCRemove(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallAttrPop(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithGetItemAndTSCRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithGetItemAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataFunction(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataMethod(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithTryGetItemAndTSCRemove(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithTryGetItemAndDelItem(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithError(DeeObject *self, DeeObject *key, DeeObject *default_);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallAttrPopItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithTSCFirstAndTSCRemove(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithTSCFirstAndDelItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithCallAttrKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithCallKeysDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithTSCIterKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithCallAttrValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithCallValuesDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithTSCIterValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithCallAttrIterKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithCallIterKeysDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithTSCKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithCallAttrIterValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithCallIterValuesDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithTSCValues(DeeObject *self);
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
