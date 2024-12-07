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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_INHERIT_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_INHERIT_C 1

#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/mro.h>
#include <deemon/object.h>
#include <deemon/seq.h>

#include <hybrid/align.h>
#include <hybrid/typecore.h>

#include "../objects/seq/default-api.h"
#include "operator-require.h"
#include "strings.h"

#undef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__
#undef byte_t
#define byte_t __BYTE_TYPE__

/************************************************************************/
/* Operator inheritance.                                                */
/************************************************************************/


/* Trace self-optimizing operator inheritance. */
#if 1
#define LOG_INHERIT(base, self, what)                        \
	Dee_DPRINTF("[RT] Inherit `" what "' from %q into %q\n", \
	            (base)->tp_name, (self)->tp_name)
#else
#define LOG_INHERIT(base, self, what) (void)0
#endif

DECL_BEGIN

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_size(struct type_seq const *__restrict self) {
	return (self->tp_size != NULL) &&
	       !DeeType_IsDefaultSize(self->tp_size);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_sizeob(struct type_seq const *__restrict self) {
	return (self->tp_sizeob != NULL) &&
	       !DeeType_IsDefaultSizeOb(self->tp_sizeob);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_foreach(struct type_seq const *__restrict self) {
	return (self->tp_foreach != NULL) &&
	       !DeeType_IsDefaultForeach(self->tp_foreach);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem(struct type_seq const *__restrict self) {
	return (self->tp_getitem != NULL) &&
	       !DeeType_IsDefaultGetItem(self->tp_getitem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_index(struct type_seq const *__restrict self) {
	return (self->tp_getitem_index != NULL) &&
	       !DeeType_IsDefaultGetItemIndex(self->tp_getitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_getitem_string_hash != NULL) &&
	       !DeeType_IsDefaultGetItemStringHash(self->tp_getitem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_getitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultGetItemStringLenHash(self->tp_getitem_string_len_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem != NULL) &&
	       !DeeType_IsDefaultTryGetItem(self->tp_trygetitem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem_index(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem_index != NULL) &&
	       !DeeType_IsDefaultTryGetItemIndex(self->tp_trygetitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem_string_hash != NULL) &&
	       !DeeType_IsDefaultTryGetItemStringHash(self->tp_trygetitem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultTryGetItemStringLenHash(self->tp_trygetitem_string_len_hash);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem(struct type_seq const *__restrict self) {
	return (self->tp_bounditem != NULL) &&
	       !DeeType_IsDefaultBoundItem(self->tp_bounditem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_index(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_index != NULL) &&
	       !DeeType_IsDefaultBoundItemIndex(self->tp_bounditem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_string_hash != NULL) &&
	       !DeeType_IsDefaultBoundItemStringHash(self->tp_bounditem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultBoundItemStringLenHash(self->tp_bounditem_string_len_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem(struct type_seq const *__restrict self) {
	return (self->tp_hasitem != NULL) &&
	       !DeeType_IsDefaultHasItem(self->tp_hasitem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem_index(struct type_seq const *__restrict self) {
	return (self->tp_hasitem_index != NULL) &&
	       !DeeType_IsDefaultHasItemIndex(self->tp_hasitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_hasitem_string_hash != NULL) &&
	       !DeeType_IsDefaultHasItemStringHash(self->tp_hasitem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_hasitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultHasItemStringLenHash(self->tp_hasitem_string_len_hash);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_delitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_delitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultDelItemStringLenHash(self->tp_delitem_string_len_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_setitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_setitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultSetItemStringLenHash(self->tp_setitem_string_len_hash);
}

/* Optimizations when inheriting certain operators. */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeType_tp_bool_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_iter_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_sizeob_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_contains_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_getitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_delitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeType_tp_setitem_t)(DeeObject *self, DeeObject *index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *DeeType_tp_getrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeType_tp_delrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1, 2, 3, 4)) int (DCALL *DeeType_tp_setrange_t)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeType_tp_foreach_t)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeType_tp_foreach_pair_t)(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeType_tp_enumerate_t)(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeType_tp_enumerate_index_t)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_iterkeys_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_bounditem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_hasitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *DeeType_tp_size_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *DeeType_tp_size_fast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_getitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeType_tp_delitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeType_tp_setitem_index_t)(DeeObject *self, size_t index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeType_tp_bounditem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeType_tp_hasitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_getrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeType_tp_delrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *DeeType_tp_setrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_getrange_index_n_t)(DeeObject *self, Dee_ssize_t start);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeType_tp_delrange_index_n_t)(DeeObject *self, Dee_ssize_t start);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeType_tp_setrange_index_n_t)(DeeObject *self, Dee_ssize_t start, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_trygetitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_trygetitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_trygetitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_getitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_delitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2, 4)) int (DCALL *DeeType_tp_setitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_bounditem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_hasitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_trygetitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_getitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_delitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *DeeType_tp_setitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_bounditem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_hasitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

typedef WUNUSED_T NONNULL_T((1)) Dee_hash_t (DCALL *DeeType_tp_hash_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_compare_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_compare_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_trycompare_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_ne_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_lo_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_le_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_gr_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_ge_t)(DeeObject *self, DeeObject *some_object);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeType_tp_inv_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_add_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_sub_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_and_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_or_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_xor_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_inplace_add_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_inplace_sub_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_inplace_mul_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_inplace_and_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_inplace_or_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_inplace_xor_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);




/* Define sequence operator implementation selectors */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bool_t DCALL DeeType_RequireSeqOperatorBool_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_iter_t DCALL DeeType_RequireSeqOperatorIter_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_sizeob_t DCALL DeeType_RequireSeqOperatorSizeOb_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_contains_t DCALL DeeType_RequireSeqOperatorContains_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getitem_t DCALL DeeType_RequireSeqOperatorGetItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delitem_t DCALL DeeType_RequireSeqOperatorDelItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setitem_t DCALL DeeType_RequireSeqOperatorSetItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_t DCALL DeeType_RequireSeqOperatorGetRange_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_t DCALL DeeType_RequireSeqOperatorDelRange_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_t DCALL DeeType_RequireSeqOperatorSetRange_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_foreach_t DCALL DeeType_RequireSeqOperatorForeach_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_foreach_pair_t DCALL DeeType_RequireSeqOperatorForeachPair_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_enumerate_t DCALL DeeType_RequireSeqOperatorEnumerate_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_enumerate_index_t DCALL DeeType_RequireSeqOperatorEnumerateIndex_for_optimize(DeeTypeObject *__restrict self);
//IVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_iterkeys_t DCALL DeeType_RequireSeqOperatorIterKeys_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bounditem_t DCALL DeeType_RequireSeqOperatorBoundItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hasitem_t DCALL DeeType_RequireSeqOperatorHasItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_size_t DCALL DeeType_RequireSeqOperatorSize_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_size_fast_t DCALL DeeType_RequireSeqOperatorSizeFast_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getitem_index_t DCALL DeeType_RequireSeqOperatorGetItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delitem_index_t DCALL DeeType_RequireSeqOperatorDelItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setitem_index_t DCALL DeeType_RequireSeqOperatorSetItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bounditem_index_t DCALL DeeType_RequireSeqOperatorBoundItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hasitem_index_t DCALL DeeType_RequireSeqOperatorHasItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_index_t DCALL DeeType_RequireSeqOperatorGetRangeIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_index_t DCALL DeeType_RequireSeqOperatorDelRangeIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_index_t DCALL DeeType_RequireSeqOperatorSetRangeIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_index_n_t DCALL DeeType_RequireSeqOperatorGetRangeIndexN_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_index_n_t DCALL DeeType_RequireSeqOperatorDelRangeIndexN_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_index_n_t DCALL DeeType_RequireSeqOperatorSetRangeIndexN_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trygetitem_t DCALL DeeType_RequireSeqOperatorTryGetItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trygetitem_index_t DCALL DeeType_RequireSeqOperatorTryGetItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hash_t DCALL DeeType_RequireSeqOperatorHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_compare_eq_t DCALL DeeType_RequireSeqOperatorCompareEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_compare_t DCALL DeeType_RequireSeqOperatorCompare_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trycompare_eq_t DCALL DeeType_RequireSeqOperatorTryCompareEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_eq_t DCALL DeeType_RequireSeqOperatorEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_ne_t DCALL DeeType_RequireSeqOperatorNe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_lo_t DCALL DeeType_RequireSeqOperatorLo_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_le_t DCALL DeeType_RequireSeqOperatorLe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_gr_t DCALL DeeType_RequireSeqOperatorGr_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_ge_t DCALL DeeType_RequireSeqOperatorGe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_inplace_add_t DCALL DeeType_RequireSeqOperatorInplaceAdd_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_inplace_mul_t DCALL DeeType_RequireSeqOperatorInplaceMul_for_optimize(DeeTypeObject *__restrict self);

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_hash_t DCALL DeeType_RequireSetOperatorHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_compare_eq_t DCALL DeeType_RequireSetOperatorCompareEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_trycompare_eq_t DCALL DeeType_RequireSetOperatorTryCompareEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_eq_t DCALL DeeType_RequireSetOperatorEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_ne_t DCALL DeeType_RequireSetOperatorNe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_lo_t DCALL DeeType_RequireSetOperatorLo_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_le_t DCALL DeeType_RequireSetOperatorLe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_gr_t DCALL DeeType_RequireSetOperatorGr_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_ge_t DCALL DeeType_RequireSetOperatorGe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inv_t DCALL DeeType_RequireSetOperatorInv_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_add_t DCALL DeeType_RequireSetOperatorAdd_for_optimize(DeeTypeObject *__restrict self); /* {"a"} + {"b"}         -> {"a","b"} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_sub_t DCALL DeeType_RequireSetOperatorSub_for_optimize(DeeTypeObject *__restrict self); /* {"a","b"} - {"b"}     -> {"a"} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_and_t DCALL DeeType_RequireSetOperatorAnd_for_optimize(DeeTypeObject *__restrict self); /* {"a","b"} & {"a"}     -> {"a"} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_xor_t DCALL DeeType_RequireSetOperatorXor_for_optimize(DeeTypeObject *__restrict self); /* {"a","b"} ^ {"a","c"} -> {"b","c"} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_add_t DCALL DeeType_RequireSetOperatorInplaceAdd_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_sub_t DCALL DeeType_RequireSetOperatorInplaceSub_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_and_t DCALL DeeType_RequireSetOperatorInplaceAnd_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_xor_t DCALL DeeType_RequireSetOperatorInplaceXor_for_optimize(DeeTypeObject *__restrict self);

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_contains_t DCALL DeeType_RequireMapOperatorContains_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_t DCALL DeeType_RequireMapOperatorGetItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_t DCALL DeeType_RequireMapOperatorDelItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_t DCALL DeeType_RequireMapOperatorSetItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_enumerate_t DCALL DeeType_RequireMapOperatorEnumerate_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_enumerate_index_t DCALL DeeType_RequireMapOperatorEnumerateIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_t DCALL DeeType_RequireMapOperatorBoundItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_t DCALL DeeType_RequireMapOperatorHasItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_index_t DCALL DeeType_RequireMapOperatorGetItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_index_t DCALL DeeType_RequireMapOperatorDelItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_index_t DCALL DeeType_RequireMapOperatorSetItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_index_t DCALL DeeType_RequireMapOperatorBoundItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_index_t DCALL DeeType_RequireMapOperatorHasItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_t DCALL DeeType_RequireMapOperatorTryGetItem_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_index_t DCALL DeeType_RequireMapOperatorTryGetItemIndex_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_string_hash_t DCALL DeeType_RequireMapOperatorTryGetItemStringHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_string_hash_t DCALL DeeType_RequireMapOperatorGetItemStringHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_string_hash_t DCALL DeeType_RequireMapOperatorDelItemStringHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_string_hash_t DCALL DeeType_RequireMapOperatorSetItemStringHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_string_hash_t DCALL DeeType_RequireMapOperatorBoundItemStringHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_string_hash_t DCALL DeeType_RequireMapOperatorHasItemStringHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_string_len_hash_t DCALL DeeType_RequireMapOperatorTryGetItemStringLenHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_string_len_hash_t DCALL DeeType_RequireMapOperatorGetItemStringLenHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_string_len_hash_t DCALL DeeType_RequireMapOperatorDelItemStringLenHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_string_len_hash_t DCALL DeeType_RequireMapOperatorSetItemStringLenHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_string_len_hash_t DCALL DeeType_RequireMapOperatorBoundItemStringLenHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_string_len_hash_t DCALL DeeType_RequireMapOperatorHasItemStringLenHash_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_compare_eq_t DCALL DeeType_RequireMapOperatorCompareEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trycompare_eq_t DCALL DeeType_RequireMapOperatorTryCompareEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_eq_t DCALL DeeType_RequireMapOperatorEq_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_ne_t DCALL DeeType_RequireMapOperatorNe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_lo_t DCALL DeeType_RequireMapOperatorLo_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_le_t DCALL DeeType_RequireMapOperatorLe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_gr_t DCALL DeeType_RequireMapOperatorGr_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_ge_t DCALL DeeType_RequireMapOperatorGe_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_add_t DCALL DeeType_RequireMapOperatorAdd_for_optimize(DeeTypeObject *__restrict self); /* {"a":1} + {"b":2}       -> {"a":1,"b":2} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_sub_t DCALL DeeType_RequireMapOperatorSub_for_optimize(DeeTypeObject *__restrict self); /* {"a":1,"b":2} - {"a"}   -> {"b":2} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_and_t DCALL DeeType_RequireMapOperatorAnd_for_optimize(DeeTypeObject *__restrict self); /* {"a":1,"b":2} & {"a"}   -> {"a":1} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_xor_t DCALL DeeType_RequireMapOperatorXor_for_optimize(DeeTypeObject *__restrict self); /* {"a":1,"b":2} ^ {"a":3} -> {"b":2} */
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_add_t DCALL DeeType_RequireMapOperatorInplaceAdd_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_sub_t DCALL DeeType_RequireMapOperatorInplaceSub_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_and_t DCALL DeeType_RequireMapOperatorInplaceAnd_for_optimize(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_xor_t DCALL DeeType_RequireMapOperatorInplaceXor_for_optimize(DeeTypeObject *__restrict self);

#ifndef __INTELLISENSE__
#define LOCAL_FOR_OPTIMIZE
DECL_END

#define DEFINE_DeeType_RequireSeqOperatorBool
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorIter
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSizeOb
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorContains
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetRange
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelRange
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetRange
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorForeach
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorForeachPair
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorEnumerate
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorEnumerateIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
/*#define DEFINE_DeeType_RequireSeqOperatorIterKeys
#include "../objects/seq/default-api-require-operator-impl.c.inl"*/
#define DEFINE_DeeType_RequireSeqOperatorBoundItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorHasItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSize
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSizeFast
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorBoundItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorHasItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetRangeIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelRangeIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetRangeIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorTryGetItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorCompareEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorCompare
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorTryCompareEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorNe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorLo
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorLe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGr
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorInplaceAdd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorInplaceMul
#include "../objects/seq/default-api-require-operator-impl.c.inl"

#define DEFINE_DeeType_RequireSetOperatorHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorCompareEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorTryCompareEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorNe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorLo
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorLe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorGr
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorGe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInv
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorAdd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorSub
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorAnd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorXor
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceAdd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceSub
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceAnd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceXor
#include "../objects/seq/default-api-require-operator-impl.c.inl"

#define DEFINE_DeeType_RequireMapOperatorContains
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorEnumerate
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorEnumerateIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItem
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItemIndex
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItemStringHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItemStringHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItemStringHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItemStringHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItemStringHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItemStringHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItemStringLenHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItemStringLenHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItemStringLenHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItemStringLenHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItemStringLenHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItemStringLenHash
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorCompareEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryCompareEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorEq
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorNe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorLo
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorLe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGr
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGe
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorAdd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSub
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorAnd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorXor
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceAdd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceSub
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceAnd
#include "../objects/seq/default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceXor
#include "../objects/seq/default-api-require-operator-impl.c.inl"

DECL_BEGIN
#undef LOCAL_FOR_OPTIMIZE
#endif /* !__INTELLISENSE__ */


#define OPTIMIZE_SET_OPERATOR(OperatorFoo, dst, tp_foo)      \
	if (0 DeeSet_VariantsFor_##OperatorFoo(|| (tp_foo) == )) \
		return DeeType_RequireSet##OperatorFoo##_for_optimize(dst)
#define OPTIMIZE_SEQ_OPERATOR(OperatorFoo, dst, tp_foo)      \
	if (0 DeeSeq_VariantsFor_##OperatorFoo(|| (tp_foo) == )) \
		return DeeType_RequireSeq##OperatorFoo##_for_optimize(dst)
#define OPTIMIZE_MAP_OPERATOR(OperatorFoo, dst, tp_foo)      \
	if (0 DeeMap_VariantsFor_##OperatorFoo(|| (tp_foo) == )) \
		return DeeType_RequireMap##OperatorFoo##_for_optimize(dst)
#define OPTIMIZE_SEQ_OR_SET_OPERATOR(OperatorFoo, dst, tp_foo) \
	if (0 DeeSeq_VariantsFor_##OperatorFoo(|| (tp_foo) == )             \
	      DeeSet_VariantsFor_##OperatorFoo(|| (tp_foo) == )) {          \
		switch (DeeType_GetSeqClass(dst)) {                             \
		case Dee_SEQCLASS_SEQ:                                          \
			return DeeType_RequireSeq##OperatorFoo##_for_optimize(dst); \
		case Dee_SEQCLASS_SET:                                          \
			return DeeType_RequireSet##OperatorFoo##_for_optimize(dst); \
		default: break;                                                 \
		}                                                               \
	}
#define OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorFoo, dst, tp_foo)   \
	if (0 DeeSeq_VariantsFor_##OperatorFoo(|| (tp_foo) == )             \
	      DeeSet_VariantsFor_##OperatorFoo(|| (tp_foo) == )             \
	      DeeMap_VariantsFor_##OperatorFoo(|| (tp_foo) == )) {          \
		switch (DeeType_GetSeqClass(dst)) {                             \
		case Dee_SEQCLASS_SEQ:                                          \
			return DeeType_RequireSeq##OperatorFoo##_for_optimize(dst); \
		case Dee_SEQCLASS_SET:                                          \
			return DeeType_RequireSet##OperatorFoo##_for_optimize(dst); \
		case Dee_SEQCLASS_MAP:                                          \
			return DeeType_RequireMap##OperatorFoo##_for_optimize(dst); \
		default: break;                                                 \
		}                                                               \
	}
#define OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorFoo, dst, tp_foo)          \
	if (0 DeeSeq_VariantsFor_##OperatorFoo(|| (tp_foo) == )             \
	      DeeMap_VariantsFor_##OperatorFoo(|| (tp_foo) == )) {          \
		switch (DeeType_GetSeqClass(dst)) {                             \
		case Dee_SEQCLASS_SEQ:                                          \
			return DeeType_RequireSeq##OperatorFoo##_for_optimize(dst); \
		case Dee_SEQCLASS_MAP:                                          \
			return DeeType_RequireMap##OperatorFoo##_for_optimize(dst); \
		default: break;                                                 \
		}                                                               \
	}
#define OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorFoo, dst, tp_foo)          \
	if (0 DeeSet_VariantsFor_##OperatorFoo(|| (tp_foo) == )             \
	      DeeMap_VariantsFor_##OperatorFoo(|| (tp_foo) == )) {          \
		switch (DeeType_GetSeqClass(dst)) {                             \
		case Dee_SEQCLASS_SET:                                          \
			return DeeType_RequireSet##OperatorFoo##_for_optimize(dst); \
		case Dee_SEQCLASS_MAP:                                          \
			return DeeType_RequireMap##OperatorFoo##_for_optimize(dst); \
		default: break;                                                 \
		}                                                               \
	}


/************************************************************************/
/* Optimize context-sensitive operators                                 */
/************************************************************************/

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_bool_t DCALL
DeeType_Optimize_tp_bool(DeeTypeObject *__restrict dst, DeeType_tp_bool_t tp_bool) {
	OPTIMIZE_SEQ_OPERATOR(OperatorBool, dst, tp_bool);
	return tp_bool;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_iter_t DCALL
DeeType_Optimize_tp_iter(DeeTypeObject *__restrict dst, DeeType_tp_iter_t tp_iter) {
	OPTIMIZE_SEQ_OPERATOR(OperatorIter, dst, tp_iter);
	return tp_iter;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_sizeob_t DCALL
DeeType_Optimize_tp_sizeob(DeeTypeObject *__restrict dst, DeeType_tp_sizeob_t tp_sizeob) {
	OPTIMIZE_SEQ_OPERATOR(OperatorSizeOb, dst, tp_sizeob);
	return tp_sizeob;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_contains_t DCALL
DeeType_Optimize_tp_contains(DeeTypeObject *__restrict dst, DeeType_tp_contains_t tp_contains) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorContains, dst, tp_contains);
	return tp_contains;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_getitem_t DCALL
DeeType_Optimize_tp_getitem(DeeTypeObject *__restrict dst, DeeType_tp_getitem_t tp_getitem) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorGetItem, dst, tp_getitem);
	return tp_getitem;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_delitem_t DCALL
DeeType_Optimize_tp_delitem(DeeTypeObject *__restrict dst, DeeType_tp_delitem_t tp_delitem) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorDelItem, dst, tp_delitem);
	return tp_delitem;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_setitem_t DCALL
DeeType_Optimize_tp_setitem(DeeTypeObject *__restrict dst, DeeType_tp_setitem_t tp_setitem) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorSetItem, dst, tp_setitem);
	return tp_setitem;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_getrange_t DCALL
DeeType_Optimize_tp_getrange(DeeTypeObject *__restrict dst, DeeType_tp_getrange_t tp_getrange) {
	OPTIMIZE_SEQ_OPERATOR(OperatorGetRange, dst, tp_getrange);
	return tp_getrange;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_delrange_t DCALL
DeeType_Optimize_tp_delrange(DeeTypeObject *__restrict dst, DeeType_tp_delrange_t tp_delrange) {
	OPTIMIZE_SEQ_OPERATOR(OperatorDelRange, dst, tp_delrange);
	return tp_delrange;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_setrange_t DCALL
DeeType_Optimize_tp_setrange(DeeTypeObject *__restrict dst, DeeType_tp_setrange_t tp_setrange) {
	OPTIMIZE_SEQ_OPERATOR(OperatorSetRange, dst, tp_setrange);
	return tp_setrange;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_foreach_t DCALL
DeeType_Optimize_tp_foreach(DeeTypeObject *__restrict dst, DeeType_tp_foreach_t tp_foreach) {
	OPTIMIZE_SEQ_OPERATOR(OperatorForeach, dst, tp_foreach);
	return tp_foreach;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_foreach_pair_t DCALL
DeeType_Optimize_tp_foreach_pair(DeeTypeObject *__restrict dst, DeeType_tp_foreach_pair_t tp_foreach_pair) {
	OPTIMIZE_SEQ_OPERATOR(OperatorForeachPair, dst, tp_foreach_pair);
	return tp_foreach_pair;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_enumerate_t DCALL
DeeType_Optimize_tp_enumerate(DeeTypeObject *__restrict dst, DeeType_tp_enumerate_t tp_enumerate) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorEnumerate, dst, tp_enumerate);
	return tp_enumerate;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_enumerate_index_t DCALL
DeeType_Optimize_tp_enumerate_index(DeeTypeObject *__restrict dst, DeeType_tp_enumerate_index_t tp_enumerate_index) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorEnumerateIndex, dst, tp_enumerate_index);
	return tp_enumerate_index;
}

/*PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_iterkeys_t DCALL
DeeType_Optimize_tp_iterkeys(DeeTypeObject *__restrict dst, DeeType_tp_iterkeys_t tp_iterkeys) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorIterKeys, dst, tp_iterkeys);
	return tp_iterkeys;
}*/

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_bounditem_t DCALL
DeeType_Optimize_tp_bounditem(DeeTypeObject *__restrict dst, DeeType_tp_bounditem_t tp_bounditem) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorBoundItem, dst, tp_bounditem);
	return tp_bounditem;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_hasitem_t DCALL
DeeType_Optimize_tp_hasitem(DeeTypeObject *__restrict dst, DeeType_tp_hasitem_t tp_hasitem) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorHasItem, dst, tp_hasitem);
	return tp_hasitem;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_size_t DCALL
DeeType_Optimize_tp_size(DeeTypeObject *__restrict dst, DeeType_tp_size_t tp_size) {
	OPTIMIZE_SEQ_OPERATOR(OperatorSize, dst, tp_size);
	return tp_size;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_size_fast_t DCALL
DeeType_Optimize_tp_size_fast(DeeTypeObject *__restrict dst, DeeType_tp_size_fast_t tp_size_fast) {
	OPTIMIZE_SEQ_OPERATOR(OperatorSizeFast, dst, tp_size_fast);
	return tp_size_fast;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_getitem_index_t DCALL
DeeType_Optimize_tp_getitem_index(DeeTypeObject *__restrict dst, DeeType_tp_getitem_index_t tp_getitem_index) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorGetItemIndex, dst, tp_getitem_index);
	return tp_getitem_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_delitem_index_t DCALL
DeeType_Optimize_tp_delitem_index(DeeTypeObject *__restrict dst, DeeType_tp_delitem_index_t tp_delitem_index) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorDelItemIndex, dst, tp_delitem_index);
	return tp_delitem_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_setitem_index_t DCALL
DeeType_Optimize_tp_setitem_index(DeeTypeObject *__restrict dst, DeeType_tp_setitem_index_t tp_setitem_index) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorSetItemIndex, dst, tp_setitem_index);
	return tp_setitem_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_bounditem_index_t DCALL
DeeType_Optimize_tp_bounditem_index(DeeTypeObject *__restrict dst, DeeType_tp_bounditem_index_t tp_bounditem_index) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorBoundItemIndex, dst, tp_bounditem_index);
	return tp_bounditem_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_hasitem_index_t DCALL
DeeType_Optimize_tp_hasitem_index(DeeTypeObject *__restrict dst, DeeType_tp_hasitem_index_t tp_hasitem_index) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorHasItemIndex, dst, tp_hasitem_index);
	return tp_hasitem_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_getrange_index_t DCALL
DeeType_Optimize_tp_getrange_index(DeeTypeObject *__restrict dst, DeeType_tp_getrange_index_t tp_getrange_index) {
	OPTIMIZE_SEQ_OPERATOR(OperatorGetRangeIndex, dst, tp_getrange_index);
	return tp_getrange_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_delrange_index_t DCALL
DeeType_Optimize_tp_delrange_index(DeeTypeObject *__restrict dst, DeeType_tp_delrange_index_t tp_delrange_index) {
	OPTIMIZE_SEQ_OPERATOR(OperatorDelRangeIndex, dst, tp_delrange_index);
	return tp_delrange_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_setrange_index_t DCALL
DeeType_Optimize_tp_setrange_index(DeeTypeObject *__restrict dst, DeeType_tp_setrange_index_t tp_setrange_index) {
	OPTIMIZE_SEQ_OPERATOR(OperatorSetRangeIndex, dst, tp_setrange_index);
	return tp_setrange_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_getrange_index_n_t DCALL
DeeType_Optimize_tp_getrange_index_n(DeeTypeObject *__restrict dst, DeeType_tp_getrange_index_n_t tp_getrange_index_n) {
	OPTIMIZE_SEQ_OPERATOR(OperatorGetRangeIndexN, dst, tp_getrange_index_n);
	return tp_getrange_index_n;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_delrange_index_n_t DCALL
DeeType_Optimize_tp_delrange_index_n(DeeTypeObject *__restrict dst, DeeType_tp_delrange_index_n_t tp_delrange_index_n) {
	OPTIMIZE_SEQ_OPERATOR(OperatorDelRangeIndexN, dst, tp_delrange_index_n);
	return tp_delrange_index_n;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_setrange_index_n_t DCALL
DeeType_Optimize_tp_setrange_index_n(DeeTypeObject *__restrict dst, DeeType_tp_setrange_index_n_t tp_setrange_index_n) {
	OPTIMIZE_SEQ_OPERATOR(OperatorSetRangeIndexN, dst, tp_setrange_index_n);
	return tp_setrange_index_n;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_trygetitem_t DCALL
DeeType_Optimize_tp_trygetitem(DeeTypeObject *__restrict dst, DeeType_tp_trygetitem_t tp_trygetitem) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorTryGetItem, dst, tp_trygetitem);
	return tp_trygetitem;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_trygetitem_index_t DCALL
DeeType_Optimize_tp_trygetitem_index(DeeTypeObject *__restrict dst, DeeType_tp_trygetitem_index_t tp_trygetitem_index) {
	OPTIMIZE_SEQ_OR_MAP_OPERATOR(OperatorTryGetItemIndex, dst, tp_trygetitem_index);
	return tp_trygetitem_index;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_trygetitem_string_hash_t DCALL
DeeType_Optimize_tp_trygetitem_string_hash(DeeTypeObject *__restrict dst, DeeType_tp_trygetitem_string_hash_t tp_trygetitem_string_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorTryGetItemStringHash, dst, tp_trygetitem_string_hash);
	return tp_trygetitem_string_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_getitem_string_hash_t DCALL
DeeType_Optimize_tp_getitem_string_hash(DeeTypeObject *__restrict dst, DeeType_tp_getitem_string_hash_t tp_getitem_string_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorGetItemStringHash, dst, tp_getitem_string_hash);
	return tp_getitem_string_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_delitem_string_hash_t DCALL
DeeType_Optimize_tp_delitem_string_hash(DeeTypeObject *__restrict dst, DeeType_tp_delitem_string_hash_t tp_delitem_string_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorDelItemStringHash, dst, tp_delitem_string_hash);
	return tp_delitem_string_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_setitem_string_hash_t DCALL
DeeType_Optimize_tp_setitem_string_hash(DeeTypeObject *__restrict dst, DeeType_tp_setitem_string_hash_t tp_setitem_string_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorSetItemStringHash, dst, tp_setitem_string_hash);
	return tp_setitem_string_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_bounditem_string_hash_t DCALL
DeeType_Optimize_tp_bounditem_string_hash(DeeTypeObject *__restrict dst, DeeType_tp_bounditem_string_hash_t tp_bounditem_string_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorBoundItemStringHash, dst, tp_bounditem_string_hash);
	return tp_bounditem_string_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_hasitem_string_hash_t DCALL
DeeType_Optimize_tp_hasitem_string_hash(DeeTypeObject *__restrict dst, DeeType_tp_hasitem_string_hash_t tp_hasitem_string_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorHasItemStringHash, dst, tp_hasitem_string_hash);
	return tp_hasitem_string_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_trygetitem_string_len_hash_t DCALL
DeeType_Optimize_tp_trygetitem_string_len_hash(DeeTypeObject *__restrict dst, DeeType_tp_trygetitem_string_len_hash_t tp_trygetitem_string_len_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorTryGetItemStringLenHash, dst, tp_trygetitem_string_len_hash);
	return tp_trygetitem_string_len_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_getitem_string_len_hash_t DCALL
DeeType_Optimize_tp_getitem_string_len_hash(DeeTypeObject *__restrict dst, DeeType_tp_getitem_string_len_hash_t tp_getitem_string_len_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorGetItemStringLenHash, dst, tp_getitem_string_len_hash);
	return tp_getitem_string_len_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_delitem_string_len_hash_t DCALL
DeeType_Optimize_tp_delitem_string_len_hash(DeeTypeObject *__restrict dst, DeeType_tp_delitem_string_len_hash_t tp_delitem_string_len_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorDelItemStringLenHash, dst, tp_delitem_string_len_hash);
	return tp_delitem_string_len_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_setitem_string_len_hash_t DCALL
DeeType_Optimize_tp_setitem_string_len_hash(DeeTypeObject *__restrict dst, DeeType_tp_setitem_string_len_hash_t tp_setitem_string_len_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorSetItemStringLenHash, dst, tp_setitem_string_len_hash);
	return tp_setitem_string_len_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_bounditem_string_len_hash_t DCALL
DeeType_Optimize_tp_bounditem_string_len_hash(DeeTypeObject *__restrict dst, DeeType_tp_bounditem_string_len_hash_t tp_bounditem_string_len_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorBoundItemStringLenHash, dst, tp_bounditem_string_len_hash);
	return tp_bounditem_string_len_hash;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_hasitem_string_len_hash_t DCALL
DeeType_Optimize_tp_hasitem_string_len_hash(DeeTypeObject *__restrict dst, DeeType_tp_hasitem_string_len_hash_t tp_hasitem_string_len_hash) {
	OPTIMIZE_MAP_OPERATOR(OperatorHasItemStringLenHash, dst, tp_hasitem_string_len_hash);
	return tp_hasitem_string_len_hash;
}




PRIVATE WUNUSED NONNULL((1)) DeeType_tp_hash_t DCALL
DeeType_Optimize_tp_hash(DeeTypeObject *__restrict dst,
                         DeeType_tp_hash_t tp_hash) {
	OPTIMIZE_SEQ_OR_SET_OPERATOR(OperatorHash, dst, tp_hash);
	return tp_hash;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_compare_t DCALL
DeeType_Optimize_tp_compare(DeeTypeObject *__restrict dst, DeeType_tp_compare_t tp_compare) {
	OPTIMIZE_SEQ_OPERATOR(OperatorCompare, dst, tp_compare);
	return tp_compare;
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_compare_eq_t DCALL
DeeType_Optimize_tp_compare_eq(DeeTypeObject *__restrict dst,
                               DeeType_tp_compare_eq_t tp_compare_eq) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorCompareEq, dst, tp_compare_eq);
	/* fast-forward (because "compare_eq" can be substituted with "compare") */
	return DeeType_Optimize_tp_compare(dst, tp_compare_eq);
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_trycompare_eq_t DCALL
DeeType_Optimize_tp_trycompare_eq(DeeTypeObject *__restrict dst,
                                  DeeType_tp_trycompare_eq_t tp_trycompare_eq) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorTryCompareEq, dst, tp_trycompare_eq);
	return tp_trycompare_eq;
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_eq_t DCALL
DeeType_Optimize_tp_eq(DeeTypeObject *__restrict dst,
                       DeeType_tp_eq_t tp_eq) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorEq, dst, tp_eq);
	return tp_eq;
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_ne_t DCALL
DeeType_Optimize_tp_ne(DeeTypeObject *__restrict dst,
                       DeeType_tp_ne_t tp_ne) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorNe, dst, tp_ne);
	return tp_ne;
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_lo_t DCALL
DeeType_Optimize_tp_lo(DeeTypeObject *__restrict dst,
                       DeeType_tp_lo_t tp_lo) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorLo, dst, tp_lo);
	return tp_lo;
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_le_t DCALL
DeeType_Optimize_tp_le(DeeTypeObject *__restrict dst,
                       DeeType_tp_le_t tp_le) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorLe, dst, tp_le);
	return tp_le;
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_gr_t DCALL
DeeType_Optimize_tp_gr(DeeTypeObject *__restrict dst,
                       DeeType_tp_gr_t tp_gr) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorGr, dst, tp_gr);
	return tp_gr;
}

PRIVATE WUNUSED NONNULL((1)) DeeType_tp_ge_t DCALL
DeeType_Optimize_tp_ge(DeeTypeObject *__restrict dst,
                       DeeType_tp_ge_t tp_ge) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorGe, dst, tp_ge);
	return tp_ge;
}

PRIVATE WUNUSED NONNULL((1)) struct type_cmp *DCALL
DeeType_Optimize_tp_cmp(DeeTypeObject *__restrict dst,
                        struct type_cmp *tp_cmp) {
	if (tp_cmp == &DeeSeq_OperatorCmp ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeAndGetItemIndexFast ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeAndTryGetItemIndex ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeAndGetItemIndex ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeObAndGetItem ||
	    tp_cmp == &DeeSeq_DefaultCmpWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			bool has_tp_size = Dee_type_seq_has_custom_tp_size(dst_seq);
			if (has_tp_size && dst_seq->tp_getitem_index_fast) {
				return &DeeSeq_DefaultCmpWithSizeAndGetItemIndexFast;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_trygetitem_index(dst_seq)) {
				return &DeeSeq_DefaultCmpWithSizeAndTryGetItemIndex;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_getitem_index(dst_seq)) {
				return &DeeSeq_DefaultCmpWithSizeAndGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_sizeob(dst_seq) &&
			           Dee_type_seq_has_custom_tp_getitem(dst_seq)) {
				return &DeeSeq_DefaultCmpWithSizeObAndGetItem;
			} else if (dst_seq->tp_foreach || DeeType_InheritIter(dst)) {
				return &DeeSeq_DefaultCmpWithForeachDefault;
			}
		}
		return &DeeSeq_OperatorCmp;
	} else if (tp_cmp == &DeeSet_OperatorCmp ||
	           tp_cmp == &DeeSet_DefaultCmpWithForeachDefault) {
		if (DeeType_InheritIter(dst))
			return &DeeSet_DefaultCmpWithForeachDefault;
		return &DeeSet_OperatorCmp;
	} else if (tp_cmp == &DeeMap_OperatorCmp ||
	           tp_cmp == &DeeMap_DefaultCmpWithForeachPairDefault) {
		if (DeeType_InheritIter(dst))
			return &DeeMap_DefaultCmpWithForeachPairDefault;
		return &DeeMap_OperatorCmp;
	}
	return tp_cmp;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_inv_t DCALL
DeeType_Optimize_tp_inv(DeeTypeObject *__restrict dst, DeeType_tp_inv_t tp_inv) {
	OPTIMIZE_SET_OPERATOR(OperatorInv, dst, tp_inv);
	return tp_inv;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_add_t DCALL
DeeType_Optimize_tp_add(DeeTypeObject *__restrict dst, DeeType_tp_add_t tp_add) {
	OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorAdd, dst, tp_add);
	return tp_add;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_sub_t DCALL
DeeType_Optimize_tp_sub(DeeTypeObject *__restrict dst, DeeType_tp_sub_t tp_sub) {
	OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorSub, dst, tp_sub);
	return tp_sub;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_and_t DCALL
DeeType_Optimize_tp_and(DeeTypeObject *__restrict dst, DeeType_tp_and_t tp_and) {
	OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorAnd, dst, tp_and);
	return tp_and;
}

#define DeeType_Optimize_tp_or(self, tp_or) DeeType_Optimize_tp_add(self, tp_or)

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_xor_t DCALL
DeeType_Optimize_tp_xor(DeeTypeObject *__restrict dst, DeeType_tp_xor_t tp_xor) {
	OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorXor, dst, tp_xor);
	return tp_xor;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_inplace_add_t DCALL
DeeType_Optimize_tp_inplace_add(DeeTypeObject *__restrict dst, DeeType_tp_inplace_add_t tp_inplace_add) {
	OPTIMIZE_SEQ_OR_SET_OR_MAP_OPERATOR(OperatorInplaceAdd, dst, tp_inplace_add);
	return tp_inplace_add;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_inplace_sub_t DCALL
DeeType_Optimize_tp_inplace_sub(DeeTypeObject *__restrict dst, DeeType_tp_inplace_sub_t tp_inplace_sub) {
	OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorInplaceSub, dst, tp_inplace_sub);
	return tp_inplace_sub;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_inplace_mul_t DCALL
DeeType_Optimize_tp_inplace_mul(DeeTypeObject *__restrict dst, DeeType_tp_inplace_mul_t tp_inplace_mul) {
	OPTIMIZE_SEQ_OPERATOR(OperatorInplaceMul, dst, tp_inplace_mul);
	return tp_inplace_mul;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_inplace_and_t DCALL
DeeType_Optimize_tp_inplace_and(DeeTypeObject *__restrict dst, DeeType_tp_inplace_and_t tp_inplace_and) {
	OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorInplaceAnd, dst, tp_inplace_and);
	return tp_inplace_and;
}

#define DeeType_Optimize_tp_inplace_or(self, tp_inplace_or) \
	DeeType_Optimize_tp_inplace_add(self, tp_inplace_or)

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeType_tp_inplace_xor_t DCALL
DeeType_Optimize_tp_inplace_xor(DeeTypeObject *__restrict dst, DeeType_tp_inplace_xor_t tp_inplace_xor) {
	OPTIMIZE_SET_OR_MAP_OPERATOR(OperatorInplaceXor, dst, tp_inplace_xor);
	return tp_inplace_xor;
}

#define DeeType_Optimize_tp_asvector(self, tp_asvector)                 tp_asvector
#define DeeType_Optimize_tp_asvector_nothrow(self, tp_asvector_nothrow) tp_asvector_nothrow
#define DeeType_Optimize_tp_unpack(self, tp_unpack)                     tp_unpack
#define DeeType_Optimize_tp_unpack_ub(self, tp_unpack_ub)               tp_unpack_ub

#define DeeType_Optimize_tp_deepload(self, tp_deepload)       tp_deepload
#define DeeType_Optimize_tp_assign(self, tp_assign)           tp_assign
#define DeeType_Optimize_tp_move_assign(self, tp_move_assign) tp_move_assign
#define DeeType_Optimize_tp_str(self, tp_str)                 tp_str
#define DeeType_Optimize_tp_print(self, tp_print)             tp_print
#define DeeType_Optimize_tp_repr(self, tp_repr)               tp_repr
#define DeeType_Optimize_tp_printrepr(self, tp_printrepr)     tp_printrepr
#define DeeType_Optimize_tp_call(self, tp_call)               tp_call
#define DeeType_Optimize_tp_call_kw(self, tp_call_kw)         tp_call_kw
#define DeeType_Optimize_tp_int32(self, tp_int32)             tp_int32
#define DeeType_Optimize_tp_int64(self, tp_int64)             tp_int64
#define DeeType_Optimize_tp_int(self, tp_int)                 tp_int
#define DeeType_Optimize_tp_double(self, tp_double)           tp_double
#define DeeType_Optimize_tp_inc(self, tp_inc)                 tp_inc
#define DeeType_Optimize_tp_dec(self, tp_dec)                 tp_dec
#define DeeType_Optimize_tp_pos(self, tp_pos)                 tp_pos
#define DeeType_Optimize_tp_neg(self, tp_neg)                 tp_neg
#define DeeType_Optimize_tp_mul(self, tp_mul)                 tp_mul
#define DeeType_Optimize_tp_div(self, tp_div)                 tp_div
#define DeeType_Optimize_tp_mod(self, tp_mod)                 tp_mod
#define DeeType_Optimize_tp_shl(self, tp_shl)                 tp_shl
#define DeeType_Optimize_tp_shr(self, tp_shr)                 tp_shr
#define DeeType_Optimize_tp_pow(self, tp_pow)                 tp_pow
#define DeeType_Optimize_tp_inplace_div(self, tp_inplace_div) tp_inplace_div
#define DeeType_Optimize_tp_inplace_mod(self, tp_inplace_mod) tp_inplace_mod
#define DeeType_Optimize_tp_inplace_shl(self, tp_inplace_shl) tp_inplace_shl
#define DeeType_Optimize_tp_inplace_shr(self, tp_inplace_shr) tp_inplace_shr
#define DeeType_Optimize_tp_inplace_pow(self, tp_inplace_pow) tp_inplace_pow


/* Assuming that the operator-class at offset `oi_class' is non-NULL,
 * check if that class has been inherited from a direct base of `self'.
 *
 * If so, return that base-type. If not, return `NULL'. */
INTERN WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeType_GetOpClassOrigin(DeeTypeObject *__restrict self, uint16_t oi_class) {
	DeeTypeMRO mro;
	void *cls = *(void **)((byte_t *)self + oi_class);
	DeeTypeObject *base;
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		void *base_cls = *(void **)((byte_t *)base + oi_class);
		if (cls == base_cls)
			return base;
	}
	return NULL;
}

#define DeeType_GetMathOrigin(self)   DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_math))
#define DeeType_GetCmpOrigin(self)    DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_cmp))
#define DeeType_GetSeqOrigin(self)    DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_seq))
#define DeeType_GetWithOrigin(self)   DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_with))
#define DeeType_GetBufferOrigin(self) DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_buffer))


INTERN NONNULL((1)) bool DCALL
DeeType_InheritConstructors(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	if (!(self->tp_flags & TP_FINHERITCTOR))
		return false;
	base = self->tp_base;
	if (base == NULL)
		return false;
	DeeType_InheritConstructors(base);
	ASSERT((base->tp_flags & TP_FVARIABLE) ==
	       (self->tp_flags & TP_FVARIABLE));
	LOG_INHERIT(base, self, "operator constructor");
	if (self->tp_flags & TP_FVARIABLE) {
		self->tp_init.tp_var.tp_ctor        = base->tp_init.tp_var.tp_ctor;
		self->tp_init.tp_var.tp_copy_ctor   = base->tp_init.tp_var.tp_copy_ctor;
		self->tp_init.tp_var.tp_deep_ctor   = base->tp_init.tp_var.tp_deep_ctor;
		self->tp_init.tp_var.tp_any_ctor    = base->tp_init.tp_var.tp_any_ctor;
		self->tp_init.tp_var.tp_free        = base->tp_init.tp_var.tp_free;
		self->tp_init.tp_var.tp_any_ctor_kw = base->tp_init.tp_var.tp_any_ctor_kw;
	} else {
#if 0 /* Allocators should not be inheritable! */
		if (base->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS
			ASSERT((base->tp_flags & TP_FGC) == (self->tp_flags & TP_FGC));
#else /* !CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
			ASSERTF(!(base->tp_flags & TP_FGC) || (self->tp_flags & TP_FGC),
			        "Non-GC object is inheriting its constructors for a GC-enabled object");
#endif /* CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
			self->tp_init.tp_alloc.tp_alloc = base->tp_init.tp_alloc.tp_alloc;
			self->tp_init.tp_alloc.tp_free  = base->tp_init.tp_alloc.tp_free;
		}
#endif
		self->tp_init.tp_alloc.tp_ctor        = base->tp_init.tp_alloc.tp_ctor;
		self->tp_init.tp_alloc.tp_copy_ctor   = base->tp_init.tp_alloc.tp_copy_ctor;
		self->tp_init.tp_alloc.tp_deep_ctor   = base->tp_init.tp_alloc.tp_deep_ctor;
		self->tp_init.tp_alloc.tp_any_ctor    = base->tp_init.tp_alloc.tp_any_ctor;
		self->tp_init.tp_alloc.tp_any_ctor_kw = base->tp_init.tp_alloc.tp_any_ctor_kw;
	}
	self->tp_init.tp_deepload = DeeType_Optimize_tp_deepload(self, base->tp_init.tp_deepload);

	/* Only inherit assign operators if the class itself doesn't define any already. */
	if (self->tp_init.tp_assign == NULL)
		self->tp_init.tp_assign = DeeType_Optimize_tp_assign(self, base->tp_init.tp_assign);
	if (self->tp_init.tp_move_assign == NULL)
		self->tp_init.tp_move_assign = DeeType_Optimize_tp_move_assign(self, base->tp_init.tp_move_assign);
	return true;
}


INTERN NONNULL((1)) bool DCALL
DeeType_InheritStr(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	DeeTypeMRO mro;
	if (self->tp_cast.tp_print) {
		/* Substitute str with print */
		self->tp_cast.tp_str = &DeeObject_DefaultStrWithPrint;
		return true;
	} else if (self->tp_cast.tp_str) {
		/* Substitute print with str */
		self->tp_cast.tp_print = &DeeObject_DefaultPrintWithStr;
		return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_cast.tp_str || !base->tp_cast.tp_print) {
			if (!DeeType_InheritStr(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator str");
		self->tp_cast.tp_str   = DeeType_Optimize_tp_str(self, base->tp_cast.tp_str);
		self->tp_cast.tp_print = DeeType_Optimize_tp_print(self, base->tp_cast.tp_print);
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritRepr(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	DeeTypeMRO mro;
	if (self->tp_cast.tp_printrepr) {
		/* Substitute repr with printrepr */
		self->tp_cast.tp_repr = &DeeObject_DefaultReprWithPrintRepr;
		return true;
	} else if (self->tp_cast.tp_repr) {
		/* Substitute printrepr with repr */
		self->tp_cast.tp_printrepr = &DeeObject_DefaultPrintReprWithRepr;
		return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_cast.tp_repr || !base->tp_cast.tp_printrepr) {
			if (!DeeType_InheritRepr(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator repr");
		self->tp_cast.tp_repr      = DeeType_Optimize_tp_repr(self, base->tp_cast.tp_repr);
		self->tp_cast.tp_printrepr = DeeType_Optimize_tp_printrepr(self, base->tp_cast.tp_printrepr);
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritCall(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	/* Substitute tp_call <===> tp_call_kw */
	if (self->tp_call) {
		if (self->tp_call_kw == NULL)
			self->tp_call_kw = &DeeObject_DefaultCallKwWithCall;
		return true;
	} else if (self->tp_call_kw) {
		if (self->tp_call == NULL)
			self->tp_call = &DeeObject_DefaultCallWithCallKw;
		return true;
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_call && !base->tp_call_kw) {
			if (!DeeType_InheritCall(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator call");
		self->tp_call    = DeeType_Optimize_tp_call(self, base->tp_call);
		self->tp_call_kw = DeeType_Optimize_tp_call_kw(self, base->tp_call_kw);
		return true;
	}
	return false;
}


INTERN NONNULL((1)) bool DCALL
DeeType_InheritInt(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	struct type_math *math = self->tp_math;
	/* Assign proxy operators instead of letting the caller do the select. */
	if (math) {
		if (math->tp_int) {
			if (math->tp_int32 == NULL)
				math->tp_int32 = &DeeObject_DefaultInt32WithInt;
			if (math->tp_int64 == NULL)
				math->tp_int64 = &DeeObject_DefaultInt64WithInt;
			if (math->tp_double == NULL)
				math->tp_double = &DeeObject_DefaultDoubleWithInt;
			return true;
		} else if (math->tp_double) {
			if (math->tp_int32 == NULL)
				math->tp_int32 = &DeeObject_DefaultInt32WithDouble;
			if (math->tp_int64 == NULL)
				math->tp_int64 = &DeeObject_DefaultInt64WithDouble;
			if (math->tp_int == NULL)
				math->tp_int = &DeeObject_DefaultIntWithDouble;
			return true;
		} else if (math->tp_int64) {
			if (math->tp_int32 == NULL)
				math->tp_int32 = &DeeObject_DefaultInt32WithInt64;
			if (math->tp_double == NULL)
				math->tp_double = &DeeObject_DefaultDoubleWithInt64;
			if (math->tp_int == NULL)
				math->tp_int = &DeeObject_DefaultIntWithInt64;
			return true;
		} else if (math->tp_int32) {
			if (math->tp_int64 == NULL)
				math->tp_int64 = &DeeObject_DefaultInt64WithInt32;
			if (math->tp_double == NULL)
				math->tp_double = &DeeObject_DefaultDoubleWithInt32;
			if (math->tp_int == NULL)
				math->tp_int = &DeeObject_DefaultIntWithInt32;
			return true;
		}
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		struct type_math *base_math = base->tp_math;
		if (base_math == NULL ||
		    (!base_math->tp_int && !base_math->tp_int32 &&
		     !base_math->tp_int64 && !base_math->tp_double)) {
			if (!DeeType_InheritInt(base))
				continue;
			base_math = base->tp_math;
		}
		if (self->tp_math != NULL) {
			DeeTypeObject *origin = DeeType_GetMathOrigin(self);
			if unlikely(origin)
				return DeeType_InheritInt(origin);
			self->tp_math->tp_int32  = DeeType_Optimize_tp_int32(self, base_math->tp_int32);
			self->tp_math->tp_int64  = DeeType_Optimize_tp_int64(self, base_math->tp_int64);
			self->tp_math->tp_int    = DeeType_Optimize_tp_int(self, base_math->tp_int);
			self->tp_math->tp_double = DeeType_Optimize_tp_double(self, base_math->tp_double);
		} else {
			self->tp_math = base_math;
		}
		LOG_INHERIT(base, self, "operator int");
		return true;
	}
	return false;
}



/* inc, dec, add, sub, iadd & isub are all apart of the same operator group. */
INTERN NONNULL((1)) bool DCALL
DeeType_InheritAdd(DeeTypeObject *__restrict self) {
	struct type_math *base_math;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_math = self->tp_math;
	if (base_math) {
		bool ok = false;
		if (base_math->tp_add) {
			if (base_math->tp_sub == NULL)
				base_math->tp_sub = &DeeObject_DefaultSubWithAdd;
			if (base_math->tp_inplace_add == NULL)
				base_math->tp_inplace_add = &DeeObject_DefaultInplaceAddWithAdd;
			if (base_math->tp_inplace_sub == NULL)
				base_math->tp_inplace_sub = &DeeObject_DefaultInplaceSubWithAdd;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithAdd;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithAdd;
			ok = true;
		} else if (base_math->tp_inplace_add) {
			if (base_math->tp_add == NULL)
				base_math->tp_add = &DeeObject_DefaultAddWithInplaceAdd;
			if (base_math->tp_sub == NULL)
				base_math->tp_sub = &DeeObject_DefaultSubWithInplaceAdd;
			if (base_math->tp_inplace_sub == NULL)
				base_math->tp_inplace_sub = &DeeObject_DefaultInplaceSubWithInplaceAdd;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithInplaceAdd;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithInplaceAdd;
			ok = true;
		}
		if (base_math->tp_sub) {
			if (base_math->tp_add == NULL)
				base_math->tp_add = &DeeObject_DefaultAddWithSub;
			if (base_math->tp_inplace_add == NULL)
				base_math->tp_inplace_add = &DeeObject_DefaultInplaceAddWithSub;
			if (base_math->tp_inplace_sub == NULL)
				base_math->tp_inplace_sub = &DeeObject_DefaultInplaceSubWithSub;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithSub;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithSub;
			ok = true;
		} else if (base_math->tp_inplace_sub) {
			if (base_math->tp_add == NULL)
				base_math->tp_add = &DeeObject_DefaultAddWithInplaceSub;
			if (base_math->tp_inplace_add == NULL)
				base_math->tp_inplace_add = &DeeObject_DefaultInplaceAddWithInplaceSub;
			if (base_math->tp_sub == NULL)
				base_math->tp_sub = &DeeObject_DefaultSubWithInplaceSub;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithInplaceSub;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithInplaceSub;
			ok = true;
		}
		if (ok)
			return true;
		if (base_math->tp_inc || base_math->tp_dec)
			return true;
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_math = base->tp_math;
		if (base_math == NULL ||
		    (!base_math->tp_add && !base_math->tp_inplace_add &&
		     !base_math->tp_sub && !base_math->tp_inplace_sub &&
		     !base_math->tp_inc && !base_math->tp_dec)) {
			if (!DeeType_InheritAdd(base))
				continue;
			base_math = base->tp_math;
		}
		if (self->tp_math) {
			DeeTypeObject *origin = DeeType_GetMathOrigin(self);
			if unlikely(origin)
				return DeeType_InheritAdd(origin);
			self->tp_math->tp_inc         = DeeType_Optimize_tp_inc(self, base_math->tp_inc);
			self->tp_math->tp_dec         = DeeType_Optimize_tp_dec(self, base_math->tp_dec);
			self->tp_math->tp_add         = DeeType_Optimize_tp_add(self, base_math->tp_add);
			self->tp_math->tp_sub         = DeeType_Optimize_tp_sub(self, base_math->tp_sub);
			self->tp_math->tp_inplace_add = DeeType_Optimize_tp_inplace_add(self, base_math->tp_inplace_add);
			self->tp_math->tp_inplace_sub = DeeType_Optimize_tp_inplace_sub(self, base_math->tp_inplace_sub);
		} else {
			self->tp_math = base_math;
		}
		LOG_INHERIT(base, self, "operator add");
		return true;
	}
	return false;
}

#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field, Field)                                                              \
	INTERN NONNULL((1)) bool DCALL                                                                                            \
	name(DeeTypeObject *__restrict self) {                                                                                    \
		struct type_math *base_math;                                                                                          \
		DeeTypeMRO mro;                                                                                                       \
		DeeTypeObject *base;                                                                                                  \
		base_math = self->tp_math;                                                                                            \
		if (base_math) {                                                                                                      \
			if (base_math->tp_##field) {                                                                                      \
				if (base_math->tp_inplace_##field == NULL)                                                                    \
					base_math->tp_inplace_##field = &DeeObject_DefaultInplace##Field##With##Field;                            \
				return true;                                                                                                  \
			} else if (base_math->tp_inplace_##field) {                                                                       \
				if (base_math->tp_##field == NULL)                                                                            \
					base_math->tp_##field = &DeeObject_Default##Field##WithInplace##Field;                                    \
				return true;                                                                                                  \
			}                                                                                                                 \
		}                                                                                                                     \
		base = DeeTypeMRO_Init(&mro, self);                                                                                   \
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {                                                      \
			base_math = base->tp_math;                                                                                        \
			if (base_math == NULL ||                                                                                          \
			    (!base_math->tp_##field &&                                                                                    \
			     !base_math->tp_inplace_##field)) {                                                                           \
				if (!name(base))                                                                                              \
					continue;                                                                                                 \
				base_math = base->tp_math;                                                                                    \
			}                                                                                                                 \
			if (self->tp_math) {                                                                                              \
				DeeTypeObject *origin = DeeType_GetMathOrigin(self);                                                          \
				if unlikely(origin)                                                                                           \
					return name(origin);                                                                                      \
				self->tp_math->tp_##field         = DeeType_Optimize_tp_##field(self, base_math->tp_##field);                 \
				self->tp_math->tp_inplace_##field = DeeType_Optimize_tp_inplace_##field(self, base_math->tp_inplace_##field); \
			} else {                                                                                                          \
				self->tp_math = base_math;                                                                                    \
			}                                                                                                                 \
			LOG_INHERIT(base, self, opname);                                                                                  \
			return true;                                                                                                      \
		}                                                                                                                     \
		return false;                                                                                                         \
	}
#define DEFINE_TYPE_INHERIT_FUNCTION1(name, opname, field)                                            \
	INTERN NONNULL((1)) bool DCALL                                                                    \
	name(DeeTypeObject *__restrict self) {                                                            \
		struct type_math *base_math;                                                                  \
		DeeTypeMRO mro;                                                                               \
		DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);                                            \
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {                              \
			base_math = base->tp_math;                                                                \
			if (base_math == NULL || !base_math->tp_##field) {                                        \
				if (!name(base))                                                                      \
					continue;                                                                         \
				base_math = base->tp_math;                                                            \
			}                                                                                         \
			if (self->tp_math) {                                                                      \
				DeeTypeObject *origin = DeeType_GetMathOrigin(self);                                  \
				if unlikely(origin)                                                                   \
					return name(origin);                                                              \
				self->tp_math->tp_##field = DeeType_Optimize_tp_##field(self, base_math->tp_##field); \
			} else {                                                                                  \
				self->tp_math = base_math;                                                            \
			}                                                                                         \
			LOG_INHERIT(base, self, opname);                                                          \
			return true;                                                                              \
		}                                                                                             \
		return false;                                                                                 \
	}
DEFINE_TYPE_INHERIT_FUNCTION1(DeeType_InheritInv, "operator inv", inv)
DEFINE_TYPE_INHERIT_FUNCTION1(DeeType_InheritPos, "operator pos", pos)
DEFINE_TYPE_INHERIT_FUNCTION1(DeeType_InheritNeg, "operator neg", neg)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritMul, "operator mul", mul, Mul)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritDiv, "operator div", div, Div)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritMod, "operator mod", mod, Mod)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritShl, "operator shl", shl, Shl)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritShr, "operator shr", shr, Shr)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritAnd, "operator and", and, And)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritOr, "operator or", or, Or)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritXor, "operator xor", xor, Xor)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritPow, "operator pow", pow, Pow)
#undef DEFINE_TYPE_INHERIT_FUNCTION1
#undef DEFINE_TYPE_INHERIT_FUNCTION



INTERN NONNULL((1)) bool DCALL
DeeType_InheritIterNext(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	if (self->tp_iter_next) {
		if (self->tp_iterator == NULL) {
			self->tp_iterator = &DeeObject_DefaultIteratorWithIterNext;
		} else {
			struct type_iterator *iter = self->tp_iterator;
			if (iter->tp_advance == NULL) {
				if (iter->tp_nextkey && !DeeType_IsDefaultIterNextKey(iter->tp_nextkey)) {
					iter->tp_advance = &DeeObject_DefaultIterAdvanceWithIterNextKey;
				} else if (iter->tp_nextvalue && !DeeType_IsDefaultIterNextValue(iter->tp_nextvalue)) {
					iter->tp_advance = &DeeObject_DefaultIterAdvanceWithIterNextValue;
				} else if (iter->tp_nextpair && !DeeType_IsDefaultIterNextPair(iter->tp_nextpair)) {
					iter->tp_advance = &DeeObject_DefaultIterAdvanceWithIterNextPair;
				} else {
					iter->tp_advance = &DeeObject_DefaultIterAdvanceWithIterNext;
				}
			}
			if (iter->tp_nextpair && !DeeType_IsDefaultIterNextPair(iter->tp_nextpair)) {
				if (iter->tp_nextkey == NULL)
					iter->tp_nextkey = &DeeObject_DefaultIterNextKeyWithIterNextPair;
				if (iter->tp_nextvalue == NULL)
					iter->tp_nextvalue = &DeeObject_DefaultIterNextValueWithIterNextPair;
			} else {
				if (iter->tp_nextkey == NULL)
					iter->tp_nextkey = &DeeObject_DefaultIterNextKeyWithIterNext;
				if (iter->tp_nextvalue == NULL)
					iter->tp_nextvalue = &DeeObject_DefaultIterNextValueWithIterNext;
			}
			if (iter->tp_nextpair == NULL)
				iter->tp_nextpair = &DeeObject_DefaultIterNextPairWithIterNext;
		}
		return true;
	} else if (self->tp_iterator) {
		struct type_iterator *iter = self->tp_iterator;
		if (iter->tp_nextpair) {
			self->tp_iter_next = &DeeObject_DefaultIterNextWithIterNextPair;
			if (iter->tp_advance == NULL) {
				if (iter->tp_nextkey && !DeeType_IsDefaultIterNextKey(iter->tp_nextkey)) {
					iter->tp_advance = &DeeObject_DefaultIterAdvanceWithIterNextKey;
				} else if (iter->tp_nextvalue && !DeeType_IsDefaultIterNextValue(iter->tp_nextvalue)) {
					iter->tp_advance = &DeeObject_DefaultIterAdvanceWithIterNextValue;
				} else {
					iter->tp_advance = &DeeObject_DefaultIterAdvanceWithIterNextPair;
				}
			}
			if (iter->tp_nextkey == NULL)
				iter->tp_nextkey = &DeeObject_DefaultIterNextKeyWithIterNextPair;
			if (iter->tp_nextvalue == NULL)
				iter->tp_nextvalue = &DeeObject_DefaultIterNextValueWithIterNextPair;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_iter_next) {
			if (!DeeType_InheritIterNext(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator iternext");
		self->tp_iter_next = base->tp_iter_next;
		self->tp_iterator  = base->tp_iterator;
		return true;
	}
	return false;
}

/* Sequence feature flags. */
enum seq_feature {
	FEAT_tp_iter,
	FEAT_tp_sizeob,
	FEAT_tp_contains,
	FEAT_tp_getitem,
	FEAT_tp_delitem,
	FEAT_tp_setitem,
	FEAT_tp_getrange,
	FEAT_tp_delrange,
	FEAT_tp_setrange,
	FEAT_tp_foreach,
	FEAT_tp_foreach_pair,
	FEAT_tp_enumerate,
	FEAT_tp_enumerate_index,
	FEAT_tp_iterkeys,
	FEAT_tp_bounditem,
	FEAT_tp_hasitem,
	FEAT_tp_size,
	FEAT_tp_getitem_index,
	FEAT_tp_delitem_index,
	FEAT_tp_setitem_index,
	FEAT_tp_bounditem_index,
	FEAT_tp_hasitem_index,
	FEAT_tp_getrange_index,
	FEAT_tp_delrange_index,
	FEAT_tp_setrange_index,
	FEAT_tp_getrange_index_n,
	FEAT_tp_delrange_index_n,
	FEAT_tp_setrange_index_n,
	/* FEAT_tp_size_fast, */ /* Default this one can't be substituted, it doesn't need a feature flag. */
	/* FEAT_tp_getitem_index_fast, */ /* Default this one can't be substituted, it doesn't need a feature flag. */
	FEAT_tp_trygetitem,
	FEAT_tp_trygetitem_index,
	FEAT_tp_trygetitem_string_hash,
	FEAT_tp_getitem_string_hash,
	FEAT_tp_delitem_string_hash,
	FEAT_tp_setitem_string_hash,
	FEAT_tp_bounditem_string_hash,
	FEAT_tp_hasitem_string_hash,
	FEAT_tp_trygetitem_string_len_hash,
	FEAT_tp_getitem_string_len_hash,
	FEAT_tp_delitem_string_len_hash,
	FEAT_tp_setitem_string_len_hash,
	FEAT_tp_bounditem_string_len_hash,
	FEAT_tp_hasitem_string_len_hash,
	/*FEAT_tp_unpack,*/
	/*FEAT_tp_unpack_ex,*/
	/*FEAT_tp_unpack_ub,*/
	FEAT_TP_COUNT
};

#define _SEQ_FEATURESET_NWORDS (CEILDIV(FEAT_TP_COUNT, sizeof(uintptr_t) * CHAR_BIT))
typedef uintptr_t seq_featureset_t[_SEQ_FEATURESET_NWORDS];
#define _seq_featureset_test_slot(i) ((i) / (sizeof(uintptr_t) * CHAR_BIT))
#define _seq_featureset_test_mask(i) ((uintptr_t)1 << ((i) % (sizeof(uintptr_t) * CHAR_BIT)))
#define seq_featureset_set(self, feat)  (void)((self)[_seq_featureset_test_slot(feat)] |= _seq_featureset_test_mask(feat))
#define seq_featureset_test(self, feat) ((self)[_seq_featureset_test_slot(feat)] & _seq_featureset_test_mask(feat))

LOCAL NONNULL((1)) void DCALL
seq_featureset_clear(seq_featureset_t self) {
	size_t i;
	for (i = 0; i < _SEQ_FEATURESET_NWORDS; ++i)
		self[i] = 0;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
seq_featureset_any(seq_featureset_t self) {
	size_t i;
	for (i = 0; i < _SEQ_FEATURESET_NWORDS; ++i) {
		if (self[i])
			return true;
	}
	return false;
}

/* Initialize "self" from features of "seq" */
PRIVATE NONNULL((1, 2, 3)) void DCALL
seq_featureset_init(seq_featureset_t self, struct type_seq *__restrict seq,
                    DeeTypeObject *__restrict tp_self, unsigned int seqclass) {
	seq_featureset_clear(self);
	/* Figure out what the type can do natively. */
	if (seq->tp_iter && !DeeType_IsDefaultIter(seq->tp_iter))
		seq_featureset_set(self, FEAT_tp_iter);
	if (Dee_type_seq_has_custom_tp_sizeob(seq))
		seq_featureset_set(self, FEAT_tp_sizeob);
	if (seq->tp_contains && !DeeType_IsDefaultContains(seq->tp_contains))
		seq_featureset_set(self, FEAT_tp_contains);
	if (seq->tp_getitem && !DeeType_IsDefaultGetItem(seq->tp_getitem))
		seq_featureset_set(self, FEAT_tp_getitem);
	if (seq->tp_delitem && !DeeType_IsDefaultDelItem(seq->tp_delitem))
		seq_featureset_set(self, FEAT_tp_delitem);
	if (seq->tp_setitem && !DeeType_IsDefaultSetItem(seq->tp_setitem))
		seq_featureset_set(self, FEAT_tp_setitem);
	if (seq->tp_getrange && !DeeType_IsDefaultGetRange(seq->tp_getrange))
		seq_featureset_set(self, FEAT_tp_getrange);
	if (seq->tp_delrange && !DeeType_IsDefaultDelRange(seq->tp_delrange))
		seq_featureset_set(self, FEAT_tp_delrange);
	if (seq->tp_setrange && !DeeType_IsDefaultSetRange(seq->tp_setrange))
		seq_featureset_set(self, FEAT_tp_setrange);
	if (Dee_type_seq_has_custom_tp_foreach(seq))
		seq_featureset_set(self, FEAT_tp_foreach);
	if (seq->tp_foreach_pair && !DeeType_IsDefaultForeachPair(seq->tp_foreach_pair))
		seq_featureset_set(self, FEAT_tp_foreach_pair);
	if (seq->tp_iterkeys) {
		if (!DeeType_IsDefaultIterKeys(seq->tp_iterkeys))
			seq_featureset_set(self, FEAT_tp_iterkeys);
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* Special case: map types can define a member "iterkeys" (or "keys") in order to implement "tp_iterkeys" */
		struct attrinfo info;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(tp_self, NULL, STR_iterkeys, 8, Dee_HashStr__iterkeys, &info) ||
		    DeeObject_TFindPrivateAttrInfoStringLenHash(tp_self, NULL, STR_keys, 4, Dee_HashStr__keys, &info)) {
			seq->tp_iterkeys = DeeType_RequireMapIterKeys(tp_self);
			ASSERT(seq->tp_iterkeys != NULL);
			ASSERT(seq->tp_iterkeys != &DeeMap_DefaultIterKeysWithError);
			seq_featureset_set(self, FEAT_tp_iterkeys);
		}
	}

	if (seq->tp_enumerate) {
		if (!DeeType_IsDefaultEnumerate(seq->tp_enumerate))
			seq_featureset_set(self, FEAT_tp_enumerate);
	} else if (seq_featureset_test(self, FEAT_tp_foreach_pair)) {
		if (seqclass == Dee_SEQCLASS_MAP && !seq_featureset_test(self, FEAT_tp_iterkeys)) {
			seq->tp_enumerate = seq->tp_foreach_pair; /* Binary compatible! (so cheat a little) */
			seq_featureset_set(self, FEAT_tp_enumerate);
		}
	}
	if (seq->tp_enumerate_index && !DeeType_IsDefaultEnumerateIndex(seq->tp_enumerate_index))
		seq_featureset_set(self, FEAT_tp_enumerate_index);
	if (seq->tp_bounditem && !DeeType_IsDefaultBoundItem(seq->tp_bounditem))
		seq_featureset_set(self, FEAT_tp_bounditem);
	if (seq->tp_hasitem && !DeeType_IsDefaultHasItem(seq->tp_hasitem))
		seq_featureset_set(self, FEAT_tp_hasitem);
	if (Dee_type_seq_has_custom_tp_size(seq))
		seq_featureset_set(self, FEAT_tp_size);
	if (Dee_type_seq_has_custom_tp_getitem_index(seq))
		seq_featureset_set(self, FEAT_tp_getitem_index);
	if (seq->tp_delitem_index && !DeeType_IsDefaultDelItemIndex(seq->tp_delitem_index))
		seq_featureset_set(self, FEAT_tp_delitem_index);
	if (seq->tp_setitem_index && !DeeType_IsDefaultSetItemIndex(seq->tp_setitem_index))
		seq_featureset_set(self, FEAT_tp_setitem_index);
	if (Dee_type_seq_has_custom_tp_bounditem_index(seq))
		seq_featureset_set(self, FEAT_tp_bounditem_index);
	if (Dee_type_seq_has_custom_tp_hasitem_index(seq))
		seq_featureset_set(self, FEAT_tp_hasitem_index);
	if (seq->tp_getrange_index && !DeeType_IsDefaultGetRangeIndex(seq->tp_getrange_index))
		seq_featureset_set(self, FEAT_tp_getrange_index);
	if (seq->tp_delrange_index && !DeeType_IsDefaultDelRangeIndex(seq->tp_delrange_index))
		seq_featureset_set(self, FEAT_tp_delrange_index);
	if (seq->tp_setrange_index && !DeeType_IsDefaultSetRangeIndex(seq->tp_setrange_index))
		seq_featureset_set(self, FEAT_tp_setrange_index);
	if (seq->tp_getrange_index_n && !DeeType_IsDefaultGetRangeIndexN(seq->tp_getrange_index_n))
		seq_featureset_set(self, FEAT_tp_getrange_index_n);
	if (seq->tp_delrange_index_n && !DeeType_IsDefaultDelRangeIndexN(seq->tp_delrange_index_n))
		seq_featureset_set(self, FEAT_tp_delrange_index_n);
	if (seq->tp_setrange_index_n && !DeeType_IsDefaultSetRangeIndexN(seq->tp_setrange_index_n))
		seq_featureset_set(self, FEAT_tp_setrange_index_n);
	if (Dee_type_seq_has_custom_tp_trygetitem(seq))
		seq_featureset_set(self, FEAT_tp_trygetitem);
	if (Dee_type_seq_has_custom_tp_trygetitem_index(seq))
		seq_featureset_set(self, FEAT_tp_trygetitem_index);
	if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq))
		seq_featureset_set(self, FEAT_tp_trygetitem_string_hash);
	if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq))
		seq_featureset_set(self, FEAT_tp_getitem_string_hash);
	if (seq->tp_delitem_string_hash && !DeeType_IsDefaultDelItemStringHash(seq->tp_delitem_string_hash))
		seq_featureset_set(self, FEAT_tp_delitem_string_hash);
	if (seq->tp_setitem_string_hash && !DeeType_IsDefaultSetItemStringHash(seq->tp_setitem_string_hash))
		seq_featureset_set(self, FEAT_tp_setitem_string_hash);
	if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq))
		seq_featureset_set(self, FEAT_tp_bounditem_string_hash);
	if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq))
		seq_featureset_set(self, FEAT_tp_hasitem_string_hash);
	if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq))
		seq_featureset_set(self, FEAT_tp_trygetitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq))
		seq_featureset_set(self, FEAT_tp_getitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_delitem_string_len_hash(seq))
		seq_featureset_set(self, FEAT_tp_delitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_setitem_string_len_hash(seq))
		seq_featureset_set(self, FEAT_tp_setitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq))
		seq_featureset_set(self, FEAT_tp_bounditem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq))
		seq_featureset_set(self, FEAT_tp_hasitem_string_len_hash);
	/*if (seq->tp_unpack && !DeeType_IsDefaultUnpack(seq->tp_unpack))
		seq_featureset_set(self, FEAT_tp_unpack);*/
	/*if (seq->tp_unpack_ub && !DeeType_IsDefaultUnpackUb(seq->tp_unpack_ub))
		seq_featureset_set(self, FEAT_tp_unpack_ub);*/
}

PRIVATE NONNULL((1, 2)) void DCALL
DeeSeqType_SubstituteDefaultOperators(DeeTypeObject *self, seq_featureset_t features, int seqclass) {
	struct type_seq *seq = self->tp_seq;
	struct attrinfo info;
	ASSERT(seq);

	/* tp_iter */
	if (!seq->tp_iter) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_getitem) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_iter = &DeeObject_DefaultIterWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_foreach_pair)) {
			seq->tp_iter = &DeeObject_DefaultIterWithForeachPair;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_iter = seqclass == Dee_SEQCLASS_MAP ? &DeeMap_DefaultIterWithEnumerate
			                                            : &DeeObject_DefaultIterWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_iter = seqclass == Dee_SEQCLASS_MAP ? &DeeMap_DefaultIterWithEnumerateIndex
			                                            : &DeeObject_DefaultIterWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_iterkeys)) {
			if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
				seq->tp_iter = seqclass == Dee_SEQCLASS_MAP ? &DeeMap_DefaultIterWithIterKeysAndTryGetItem
				                                            : &DeeObject_DefaultIterWithIterKeysAndTryGetItem;
			} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
				seq->tp_iter = seqclass == Dee_SEQCLASS_MAP ? &DeeMap_DefaultIterWithIterKeysAndGetItem
				                                            : &DeeObject_DefaultIterWithIterKeysAndGetItem;
			} else if ((seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq_featureset_test(features, FEAT_tp_getitem_index)) ||
			           (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) || seq_featureset_test(features, FEAT_tp_getitem_string_hash)) ||
			           (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) || seq_featureset_test(features, FEAT_tp_getitem_string_len_hash))) {
				seq->tp_iter = seqclass == Dee_SEQCLASS_MAP ? &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault
				                                            : &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault;
			}
		}
	}

	/* tp_size */
	if (!seq->tp_size) {
		if (seq_featureset_test(features, FEAT_tp_sizeob)) {
			seq->tp_size = &DeeObject_DefaultSizeWithSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_foreach_pair)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithForeachPair;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithIter;
		}
	}

	/* tp_sizeob */
	if (!seq->tp_sizeob) {
		if (seq_featureset_test(features, FEAT_tp_size)) {
			seq->tp_sizeob = &DeeObject_DefaultSizeObWithSize;
		} else if (seq->tp_size) {
			seq->tp_sizeob = &DeeObject_DefaultSizeObWithSizeDefault;
		}
	}

	/* tp_foreach */
	if (!seq->tp_foreach) {
		if (seq_featureset_test(features, FEAT_tp_foreach_pair)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithForeachPair;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_getitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithIter;
		} else if (seq->tp_size &&
		           (seq_featureset_test(features, FEAT_tp_getitem) ||
		            seq_featureset_test(features, FEAT_tp_getitem_index) ||
		            seq_featureset_test(features, FEAT_tp_trygetitem)) &&
		           (seqclass == Dee_SEQCLASS_SEQ)) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault;
		} else if ((seq_featureset_test(features, FEAT_tp_getitem) ||
		            seq_featureset_test(features, FEAT_tp_getitem_index)) &&
		           (seqclass == Dee_SEQCLASS_SEQ)) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithGetItemIndexDefault;
		}
	}

	/* tp_contains (for maps) */
	if (!seq->tp_contains && seqclass == Dee_SEQCLASS_MAP) {
		if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithEnumerate;
		}
	}

	/* tp_getitem */
	if (!seq->tp_getitem) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeSeq_DefaultGetItemWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeSeq_DefaultGetItemWithTryGetItemAndSize;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			       seq->tp_getitem = &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndexOb;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seqclass != Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem = &DeeMap_DefaultGetItemWithEnumerate;
		}
	}

	/* tp_getitem_index */
	if (!seq->tp_getitem_index) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) &&
		           seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) &&
		           seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndexOb;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) &&
		           seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSize;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) &&
		           seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItem;
		} else if (seq->tp_foreach && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithForeachDefault;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_index = &DeeMap_DefaultGetItemIndexWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
		} else if (seq->tp_getitem) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithGetItemDefault;
		}
	}

	/* tp_foreach */
	if (!seq->tp_foreach) {
		if (seqclass == Dee_SEQCLASS_MAP && seq_featureset_test(features, FEAT_tp_iterkeys)) {
			if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
				seq->tp_foreach = &DeeObject_DefaultForeachWithIterKeysAndTryGetItem;
			} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
				seq->tp_foreach = &DeeObject_DefaultForeachWithIterKeysAndGetItem;
			} else if (seq->tp_getitem_index) {
				seq->tp_foreach = &DeeObject_DefaultForeachWithIterKeysAndTryGetItemDefault;
			}
		} else if (seq->tp_size && seq->tp_getitem_index && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault;
		} else if (seq->tp_getitem && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithGetItemIndexDefault;
		}
	}

	/* tp_foreach_pair */
	if (!seq->tp_foreach_pair) {
		if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithIter;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_foreach_pair = &DeeMap_DefaultForeachPairWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_foreach_pair = &DeeMap_DefaultForeachPairWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerateIndex;
		} else if (seq->tp_foreach) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithForeachDefault;
		}
	}

	/* tp_getitem */
	if (!seq->tp_getitem) {
		if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem = &DeeMap_DefaultGetItemWithEnumerateDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemIndexDefault;
		}
	}

	/* tp_enumerate */
	if (!seq->tp_enumerate) {
		if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_enumerate = &DeeObject_DefaultEnumerateWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_iterkeys) && seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_enumerate = &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_iterkeys) && seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_enumerate = &DeeObject_DefaultEnumerateWithIterKeysAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_iterkeys) && seq->tp_getitem) {
			seq->tp_enumerate = &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault; /* tp_trygetitem is always available when tp_getitem is! */
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) && seq_featureset_test(features, FEAT_tp_getitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_foreach) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithCounterAndForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithCounterAndIter;
		} else if (seq_featureset_test(features, FEAT_tp_iter) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_enumerate = &DeeMap_DefaultEnumerateWithIter;
		} else if (seq->tp_size && seq->tp_getitem_index && seqclass == Dee_SEQCLASS_SEQ) {
			if (seq->tp_size == &DeeSeq_DefaultSizeWithForeachPair ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithForeach ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithIter ||
			    seq->tp_getitem_index == &DeeSeq_DefaultGetItemIndexWithForeachDefault) {
				seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithCounterAndForeachDefault;
			} else {
				seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault;
			}
		} else if (seq->tp_foreach_pair && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_enumerate = &DeeMap_DefaultEnumerateWithForeachPairDefault;
		}
	}
	if (!seq->tp_foreach_pair && seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP)
		seq->tp_foreach_pair = &DeeMap_DefaultForeachPairWithEnumerateDefault;
	if (!seq->tp_foreach && seq->tp_foreach_pair)
		seq->tp_foreach = &DeeObject_DefaultForeachWithForeachPairDefault;

	/* tp_contains */
	if (!seq->tp_contains) {
		if (seqclass == Dee_SEQCLASS_MAP && seq->tp_enumerate) {
			seq->tp_contains = &DeeMap_DefaultContainsWithEnumerateDefault;
		} else if (seqclass == Dee_SEQCLASS_SEQ && seq->tp_foreach) {
			seq->tp_contains = &DeeSeq_DefaultContainsWithForeachDefault;
		}
	}

	/* tp_getitem_index */
	if (!seq->tp_getitem_index && seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP)
		seq->tp_getitem_index = &DeeMap_DefaultGetItemIndexWithEnumerateDefault;

	/* tp_delitem_index */
	if (!seq->tp_delitem_index && seq_featureset_test(features, FEAT_tp_delitem))
		seq->tp_delitem_index = &DeeObject_DefaultDelItemIndexWithDelItem;

	/* tp_setitem_index */
	if (!seq->tp_setitem_index && seq_featureset_test(features, FEAT_tp_setitem))
		seq->tp_setitem_index = &DeeObject_DefaultSetItemIndexWithSetItem;

	/* tp_enumerate_index */
	if (!seq->tp_enumerate_index) {
		if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_enumerate_index = &DeeObject_DefaultEnumerateIndexWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_iterkeys)) {
			seq->tp_enumerate_index = &DeeObject_DefaultEnumerateIndexWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) && seq_featureset_test(features, FEAT_tp_getitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_foreach) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithCounterAndIter;
		} else if (seq->tp_size && seq->tp_getitem_index && seqclass == Dee_SEQCLASS_SEQ) {
			if (seq->tp_size == &DeeSeq_DefaultSizeWithForeachPair ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithForeach ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithIter ||
			    seq->tp_getitem_index == &DeeSeq_DefaultGetItemIndexWithForeachDefault) {
				seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault;
			} else {
				seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault;
			}
		} else if (seq->tp_enumerate) {
			seq->tp_enumerate_index = &DeeObject_DefaultEnumerateIndexWithEnumerateDefault;
		}
	}

	/* tp_setrange_index */
	if (!seq->tp_setrange_index) {
		if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_setrange_index = &DeeObject_DefaultSetRangeIndexWithSetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			/* Impl when type has insert/insertall + erase/pop */
			if ((seq->tp_size != NULL) &&
			    (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_insert, 6, Dee_HashStr__insert, &info) ||
			     DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_insertall, 9, Dee_HashStr__insertall, &info)) &&
			    (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_erase, 5, Dee_HashStr__erase, &info) ||
			     DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pop, 3, Dee_HashStr__pop, &info))) {
				seq->tp_setrange_index = &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll;
			}
		}
	}

	/* tp_setrange_index_n */
	if (!seq->tp_setrange_index_n) {
		if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_setrange_index_n = &DeeObject_DefaultSetRangeIndexNWithSetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			if (seq_featureset_test(features, FEAT_tp_size) &&
			    seq_featureset_test(features, FEAT_tp_setrange_index)) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_setrange_index)) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex;
			} else if (seq->tp_size && seq->tp_setrange_index) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault;
			}
		}
	}

	/* tp_setrange */
	if (!seq->tp_setrange) {
		if (seq_featureset_test(features, FEAT_tp_setrange_index) &&
		    seq_featureset_test(features, FEAT_tp_setrange_index_n)) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN;
		} else if (seq->tp_setrange_index) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault;
		}
	}

	/* tp_delrange_index */
	if (!seq->tp_delrange_index) {
		if (seq_featureset_test(features, FEAT_tp_delrange)) {
			seq->tp_delrange_index = &DeeObject_DefaultDelRangeIndexWithDelRange;
		} else if (seq_featureset_test(features, FEAT_tp_setrange_index)) {
			seq->tp_delrange_index = &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone;
		} else if (seq->tp_setrange_index) {
			seq->tp_delrange_index = &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault;
		} else if ((seq->tp_size != NULL) && seqclass == Dee_SEQCLASS_SEQ &&
		           (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_erase, 5, Dee_HashStr__erase, &info) ||
		            DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pop, 3, Dee_HashStr__pop, &info))) {
			seq->tp_delrange_index = &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndTSCErase;
		} else if (seq->tp_size && seq->tp_delitem_index && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_delrange_index = &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault;
		}
	}

	/* tp_delrange_index_n */
	if (!seq->tp_delrange_index_n) {
		if (seq_featureset_test(features, FEAT_tp_delrange)) {
			seq->tp_delrange_index_n = &DeeObject_DefaultDelRangeIndexNWithDelRange;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_delrange_index) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_delrange_index) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex;
		} else if (seq_featureset_test(features, FEAT_tp_setrange_index_n)) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone;
		} else if (seq->tp_setrange_index_n) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault;
		} else if ((seq->tp_size != NULL) && seqclass == Dee_SEQCLASS_SEQ &&
		           (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_erase, 5, Dee_HashStr__erase, &info) ||
		            DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pop, 3, Dee_HashStr__pop, &info))) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndTSCErase;
		} else if (seq->tp_size && seq->tp_delitem_index && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault;
		}
	}

	/* tp_delrange */
	if (!seq->tp_delrange) {
		if (seq_featureset_test(features, FEAT_tp_delrange_index) &&
		    seq_featureset_test(features, FEAT_tp_delrange_index_n)) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN;
		} else if (seq->tp_delrange_index || seq->tp_delrange_index_n) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault;
		} else if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNone;
		} else if (seq->tp_setrange) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNoneDefault;
		}
	}

	/* tp_delitem_index */
	if (!seq->tp_delitem_index) {
		if (seq->tp_delrange_index && seqclass == Dee_SEQCLASS_SEQ)
			seq->tp_delitem_index = &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault;
	}

	/* tp_setitem_index */
	if (!seq->tp_setitem_index) {
		if (seq->tp_setrange_index && seqclass == Dee_SEQCLASS_SEQ)
			seq->tp_setitem_index = &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault;
	}

	/* tp_delitem */
	if (!seq->tp_delitem) {
		if (seq_featureset_test(features, FEAT_tp_delitem_index)) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemIndex;
		} else if (seq->tp_delitem_index) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemIndexDefault;
		} else if (seq_featureset_test(features, FEAT_tp_delitem_string_hash)) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_delitem_string_len_hash)) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringLenHash;
		}
	}

	/* tp_setitem */
	if (!seq->tp_setitem) {
		if (seq_featureset_test(features, FEAT_tp_setitem_index)) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemIndex;
		} else if (seq->tp_setitem_index) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemIndexDefault;
		} else if (seq_featureset_test(features, FEAT_tp_setitem_string_hash)) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_setitem_string_len_hash)) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringLenHash;
		}
	}

	/* tp_getrange_index */
	if (!seq->tp_getrange_index) {
		if (seq_featureset_test(features, FEAT_tp_getrange)) {
			seq->tp_getrange_index = &DeeObject_DefaultGetRangeIndexWithGetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItem;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_iter)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIter;
			} else if (seq->tp_size && seq->tp_iter) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIterDefault;
			}
		}
	}

	/* tp_getrange_index_n */
	if (!seq->tp_getrange_index_n) {
		if (seq_featureset_test(features, FEAT_tp_getrange)) {
			seq->tp_getrange_index_n = &DeeObject_DefaultGetRangeIndexNWithGetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			if (seq_featureset_test(features, FEAT_tp_size) &&
			    seq_featureset_test(features, FEAT_tp_getrange_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetRangeIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getrange_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex;
			} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItem;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_iter)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIter;
			} else if (seq->tp_size && seq->tp_iter) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIterDefault;
			}
		}
	}

	/* tp_getrange */
	if (!seq->tp_getrange) {
		if (seq_featureset_test(features, FEAT_tp_getrange_index) &&
		    seq_featureset_test(features, FEAT_tp_getrange_index_n)) {
			seq->tp_getrange = &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getrange = &DeeSeq_DefaultGetRangeWithSizeDefaultAndGetItemIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getrange = &DeeSeq_DefaultGetRangeWithSizeDefaultAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_getitem) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getrange = &DeeSeq_DefaultGetRangeWithSizeObAndGetItem;
		} else if (seq->tp_getrange_index && seq->tp_getrange_index_n) {
			seq->tp_getrange = &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault;
		}
	}

	/* tp_delrange */
	if (!seq->tp_delrange) {
		if (seq_featureset_test(features, FEAT_tp_delrange_index) &&
		    seq_featureset_test(features, FEAT_tp_delrange_index_n)) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN;
		} else if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNone;
		} else if (seq->tp_setrange) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNoneDefault;
		} else if (seq->tp_delrange_index && seq->tp_delrange_index_n) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault;
		}
	}

	/* tp_setrange */
	if (!seq->tp_setrange) {
		if (seq_featureset_test(features, FEAT_tp_setrange_index) &&
		    seq_featureset_test(features, FEAT_tp_setrange_index_n)) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN;
		} else if (seq->tp_setrange_index && seq->tp_setrange_index_n) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault;
		}
	}

	/* tp_bounditem_index */
	if (!seq->tp_bounditem_index) {
		if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq->tp_getitem_index_fast && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_index = &DeeMap_DefaultBoundItemIndexWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem_index = &DeeSeq_DefaultBoundItemIndexWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem_index = &DeeSeq_DefaultBoundItemIndexWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_index = &DeeMap_DefaultBoundItemIndexWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_index = &DeeMap_DefaultBoundItemIndexWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithErrorRequiresString;
		} else if (seq->tp_getitem_index) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault;
		}
	}

	/* tp_bounditem */
	if (!seq->tp_bounditem) {
		if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			/* TODO: If it's a map and implements one of the "hasitem" operators, assume that
			 *       items cannot be unbound and emulate "bounditem" using hasitem only! */
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem = &DeeMap_DefaultBoundItemWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem = &DeeMap_DefaultBoundItemWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem = &DeeMap_DefaultBoundItemWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem = &DeeSeq_DefaultBoundItemWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem = &DeeSeq_DefaultBoundItemWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHash;
		} else if (seq->tp_getitem) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemDefault;
		}
	}

	/* tp_hasitem_index */
	if (!seq->tp_hasitem_index) {
		if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_index = &DeeMap_DefaultHasItemIndexWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSize;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithErrorRequiresString;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_index = &DeeMap_DefaultHasItemIndexWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_index = &DeeMap_DefaultHasItemIndexWithEnumerateDefault;
		} else if (seq->tp_size && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSizeDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItemIndexDefault;
		}
	}

	/* tp_hasitem */
	if (!seq->tp_hasitem) {
		if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem = &DeeMap_DefaultHasItemWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem = &DeeSeq_DefaultHasItemWithSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem = &DeeSeq_DefaultHasItemWithSize;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem = &DeeMap_DefaultHasItemWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem = &DeeMap_DefaultHasItemWithEnumerateDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemDefault;
		}
	}

	/* tp_trygetitem */
	if (!seq->tp_trygetitem) {
		if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem = &DeeMap_DefaultTryGetItemWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem = &DeeMap_DefaultTryGetItemWithEnumerateDefault;
		} else if (seq->tp_getitem) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemDefault;
		}
	}
	if (!seq->tp_iter && seq_featureset_test(features, FEAT_tp_iterkeys) && seq->tp_trygetitem)
		seq->tp_iter = &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault;

	/* tp_iterkeys */
	if (!seq->tp_iterkeys) {
		if (seq_featureset_test(features, FEAT_tp_enumerate) &&
		    seq->tp_enumerate != seq->tp_foreach_pair) {
			/* This type of iterator is expensive, so if "tp_enumerate" is the same as "tp_foreach_pair",
			 * that means that the mapping doesn't have unbound keys, also meaning that tp_iterkeys can
			 * be implemented as a proxy for the normal iterator (in case it's a map).
			 *
			 * As such, try not to make use of `tp_enumerate' unless we have to (s.a. the
			 * second case that assigns `DeeObject_DefaultIterKeysWithEnumerate' below). */
			seq->tp_iterkeys = &DeeObject_DefaultIterKeysWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_iterkeys = &DeeObject_DefaultIterKeysWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iterkeys = &DeeSeq_DefaultIterKeysWithSize;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iterkeys = &DeeSeq_DefaultIterKeysWithSizeOb;
		} else if (seq->tp_size && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iterkeys = &DeeSeq_DefaultIterKeysWithSizeDefault;
		} else if (seq_featureset_test(features, FEAT_tp_iter) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_iterkeys = &DeeMap_DefaultIterKeysWithIter;
		} else {
			if (seqclass == Dee_SEQCLASS_MAP) {
				/* if (has_private_attr("keys")) tp_iterkeys = () -> self.keys.operator iter(); */
				if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_iterkeys, 8, Dee_HashStr__iterkeys, &info) ||
				    DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_keys, 4, Dee_HashStr__keys, &info)) {
					seq->tp_iterkeys = DeeType_RequireMapIterKeys(self);
				} else if (seq->tp_iter) {
					seq->tp_iterkeys = &DeeMap_DefaultIterKeysWithIterDefault;
				}
			}
			if (!seq->tp_iterkeys && seq_featureset_test(features, FEAT_tp_enumerate))
				seq->tp_iterkeys = &DeeObject_DefaultIterKeysWithEnumerate;
		}
	}

	/* tp_trygetitem_index */
	if (!seq->tp_trygetitem_index) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
		} else if (seq->tp_foreach && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_trygetitem_index = &DeeSeq_DefaultTryGetItemIndexWithForeachDefault;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_index = &DeeMap_DefaultTryGetItemIndexWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_index = &DeeMap_DefaultTryGetItemIndexWithEnumerateDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItemIndexDefault;
		}
	}

	/* tp_trygetitem_string_hash */
	if (!seq->tp_trygetitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_hash = &DeeMap_DefaultTryGetItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_hash = &DeeMap_DefaultTryGetItemStringHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithErrorRequiresInt;
		} else if (seq->tp_trygetitem) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemDefault;
		}
	}

	/* tp_trygetitem_string_len_hash */
	if (!seq->tp_trygetitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_len_hash = &DeeMap_DefaultTryGetItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_len_hash = &DeeMap_DefaultTryGetItemStringLenHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithErrorRequiresInt;
		} else if (seq->tp_trygetitem) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemDefault;
		}
	}

	/* tp_getitem_string_hash */
	if (!seq->tp_getitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithErrorRequiresInt;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_hash = &DeeMap_DefaultGetItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_hash = &DeeMap_DefaultGetItemStringHashWithEnumerateDefault;
		} else if (seq->tp_getitem) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemDefault;
		}
	}

	/* tp_getitem_string_len_hash */
	if (!seq->tp_getitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithErrorRequiresInt;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_len_hash = &DeeMap_DefaultGetItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_len_hash = &DeeMap_DefaultGetItemStringLenHashWithEnumerateDefault;
		} else if (seq->tp_getitem) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemDefault;
		}
	}

	/* tp_bounditem_string_hash */
	if (!seq->tp_bounditem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_hash = &DeeMap_DefaultBoundItemStringHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_hash = &DeeMap_DefaultBoundItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_hash = &DeeMap_DefaultBoundItemStringHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt;
		} else if (seq->tp_bounditem) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemDefault;
		}
	}

	/* tp_bounditem_string_len_hash */
	if (!seq->tp_bounditem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_len_hash = &DeeMap_DefaultBoundItemStringLenHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_len_hash = &DeeMap_DefaultBoundItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_len_hash = &DeeMap_DefaultBoundItemStringLenHashWithEnumerateDefault;
		} else if (seq->tp_bounditem) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault;
		}
	}

	/* tp_hasitem_string_hash */
	if (!seq->tp_hasitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_hash = &DeeMap_DefaultHasItemStringHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_hash = &DeeMap_DefaultHasItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_hash = &DeeMap_DefaultHasItemStringHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) ||
		           seq->tp_getitem_index_fast) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithErrorRequiresInt;
		} else if (seq->tp_hasitem) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemDefault;
		}
	}

	/* tp_hasitem_string_len_hash */
	if (!seq->tp_hasitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_len_hash = &DeeMap_DefaultHasItemStringLenHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_len_hash = &DeeMap_DefaultHasItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_len_hash = &DeeMap_DefaultHasItemStringLenHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) ||
		           seq->tp_getitem_index_fast) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt;
		} else if (seq->tp_hasitem) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemDefault;
		}
	}

	/* tp_delitem_string_hash */
	if (!seq->tp_delitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_delitem_string_len_hash)) {
			seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_delitem)) {
			seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItem;
		} else if (seq->tp_delitem) {
			seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithErrorRequiresInt;
		}
	}

	/* tp_delitem_string_len_hash */
	if (!seq->tp_delitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_delitem_string_hash)) {
			seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_delitem)) {
			seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItem;
		} else if (seq->tp_delitem) {
			seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithErrorRequiresInt;
		}
	}

	/* tp_setitem_string_hash */
	if (!seq->tp_setitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_setitem_string_len_hash)) {
			seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_setitem)) {
			seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItem;
		} else if (seq->tp_setitem) {
			seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithErrorRequiresInt;
		}
	}

	/* tp_setitem_string_len_hash */
	if (!seq->tp_setitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_setitem_string_hash)) {
			seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_setitem)) {
			seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItem;
		} else if (seq->tp_setitem) {
			seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithErrorRequiresInt;
		}
	}

	/* tp_size_fast (simply set to return `(size_t)-1') */
	if (!seq->tp_size_fast)
		seq->tp_size_fast = &DeeObject_DefaultSizeFastWithErrorNotFast;

	/* tp_unpack */
	if (!seq->tp_unpack) {
		if (seq->tp_asvector) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithAsVector;
		} else if (seq->tp_unpack_ex && !DeeType_IsDefaultUnpackEx(seq->tp_unpack_ex)) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithUnpackEx;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithSizeAndGetItemIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithSizeDefaultAndGetItemIndexDefault;
		} else if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithIter;
		} else if (seq->tp_foreach) {
			seq->tp_unpack = &DeeSeq_DefaultUnpackWithForeachDefault;
		}
	}

	/* tp_unpack_ex */
	if (!seq->tp_unpack_ex) {
		if (seq->tp_asvector) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithAsVector;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithSizeDefaultAndGetItemIndexDefault;
		} else if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithIter;
		} else if (seq->tp_foreach) {
			seq->tp_unpack_ex = &DeeSeq_DefaultUnpackExWithForeachDefault;
		}
	}

	/* tp_unpack_ub */
	if (!seq->tp_unpack_ub) {
		if (seq->tp_unpack && !DeeType_IsDefaultUnpack(seq->tp_unpack)) {
			seq->tp_unpack_ub = seq->tp_unpack;
		} else if (seq->tp_unpack_ex && !DeeType_IsDefaultUnpackEx(seq->tp_unpack_ex)) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithUnpackEx;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_enumerate_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault;
		} else if (seq->tp_asvector) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithAsVector;
		} else if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithIter;
		} else if (seq->tp_foreach) {
			seq->tp_unpack_ub = &DeeSeq_DefaultUnpackUbWithForeachDefault;
		}
	}
}

INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeAndGetItemIndexFast = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeAndGetItemIndexFast,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeAndTryGetItemIndex = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeAndTryGetItemIndex,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeAndGetItemIndex = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeAndGetItemIndex,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeAndGetItemIndex,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeObAndGetItem = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeObAndGetItem,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeObAndGetItem,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeObAndGetItem,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithForeachDefault = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithForeachDefault,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithForeachDefault,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithForeachDefault,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithForeachDefault,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSet_DefaultCmpWithForeachDefault = {
	/* .tp_hash          = */ &DeeSet_DefaultHashWithForeachDefault,
	/* .tp_compare_eq    = */ &DeeSet_DefaultCompareEqWithForeachDefault,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &DeeSet_DefaultTryCompareEqWithForeachDefault,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeSet_DefaultLoWithForeachDefault,
	/* .tp_le            = */ &DeeSet_DefaultLeWithForeachDefault,
	/* .tp_gr            = */ &DeeSet_DefaultGrWithForeachDefault,
	/* .tp_ge            = */ &DeeSet_DefaultGeWithForeachDefault,
};
INTERN struct type_cmp DeeMap_DefaultCmpWithForeachPairDefault = {
	/* .tp_hash          = */ &DeeMap_DefaultHashWithForeachPairDefault,
	/* .tp_compare_eq    = */ &DeeMap_DefaultCompareEqWithForeachPairDefault,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &DeeMap_DefaultTryCompareEqWithForeachPairDefault,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeMap_DefaultLoWithForeachPairDefault,
	/* .tp_le            = */ &DeeMap_DefaultLeWithForeachPairDefault,
	/* .tp_gr            = */ &DeeMap_DefaultGrWithForeachPairDefault,
	/* .tp_ge            = */ &DeeMap_DefaultGeWithForeachPairDefault,
};




/* Inherit OPERATOR_ITER, OPERATOR_SIZE and OPERTOR_GETITEM for
 * a type with `DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ' */
PRIVATE NONNULL((1)) bool DCALL
DeeType_InheritSeqOperators(DeeTypeObject *__restrict self, unsigned int seqclass) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_seq = self->tp_seq;
	if (base_seq) {
		seq_featureset_t features;
		seq_featureset_init(features, base_seq, self, seqclass);

		/* If the type is implementing sequence features, auto-complete those
		 * features and don't try to import operators from base classes (when
		 * extending sequence types, sequence operators must be inherited all-
		 * at-once) */
		if (seq_featureset_any(features) || base_seq->tp_getitem_index_fast) {
			DeeSeqType_SubstituteDefaultOperators(self, features, seqclass);
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (DeeType_GetSeqClass(base) != seqclass)
			break; /* Stop when base of a different sequence class is reached. */
		if (!DeeType_InheritSeqOperators(base, seqclass))
			continue;
		base_seq = base->tp_seq;
		ASSERT(base_seq);
		LOG_INHERIT(base, self, "operator <sequence>");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSeqOperators(origin, seqclass);
			self->tp_seq->tp_iter                       = DeeType_Optimize_tp_iter(self, base_seq->tp_iter);
			self->tp_seq->tp_sizeob                     = DeeType_Optimize_tp_sizeob(self, base_seq->tp_sizeob);
			self->tp_seq->tp_contains                   = DeeType_Optimize_tp_contains(self, base_seq->tp_contains);
			self->tp_seq->tp_getitem                    = DeeType_Optimize_tp_getitem(self, base_seq->tp_getitem);
			self->tp_seq->tp_delitem                    = DeeType_Optimize_tp_delitem(self, base_seq->tp_delitem);
			self->tp_seq->tp_setitem                    = DeeType_Optimize_tp_setitem(self, base_seq->tp_setitem);
			self->tp_seq->tp_getrange                   = DeeType_Optimize_tp_getrange(self, base_seq->tp_getrange);
			self->tp_seq->tp_delrange                   = DeeType_Optimize_tp_delrange(self, base_seq->tp_delrange);
			self->tp_seq->tp_setrange                   = DeeType_Optimize_tp_setrange(self, base_seq->tp_setrange);
			self->tp_seq->tp_nsi                        = base_seq->tp_nsi;
			self->tp_seq->tp_foreach                    = DeeType_Optimize_tp_foreach(self, base_seq->tp_foreach);
			self->tp_seq->tp_foreach_pair               = DeeType_Optimize_tp_foreach_pair(self, base_seq->tp_foreach_pair);
			self->tp_seq->tp_enumerate                  = DeeType_Optimize_tp_enumerate(self, base_seq->tp_enumerate);
			self->tp_seq->tp_enumerate_index            = DeeType_Optimize_tp_enumerate_index(self, base_seq->tp_enumerate_index);
			self->tp_seq->tp_bounditem                  = DeeType_Optimize_tp_bounditem(self, base_seq->tp_bounditem);
			self->tp_seq->tp_hasitem                    = DeeType_Optimize_tp_hasitem(self, base_seq->tp_hasitem);
			self->tp_seq->tp_size                       = DeeType_Optimize_tp_size(self, base_seq->tp_size);
			self->tp_seq->tp_getitem_index              = DeeType_Optimize_tp_getitem_index(self, base_seq->tp_getitem_index);
			self->tp_seq->tp_delitem_index              = DeeType_Optimize_tp_delitem_index(self, base_seq->tp_delitem_index);
			self->tp_seq->tp_setitem_index              = DeeType_Optimize_tp_setitem_index(self, base_seq->tp_setitem_index);
			self->tp_seq->tp_bounditem_index            = DeeType_Optimize_tp_bounditem_index(self, base_seq->tp_bounditem_index);
			self->tp_seq->tp_hasitem_index              = DeeType_Optimize_tp_hasitem_index(self, base_seq->tp_hasitem_index);
			self->tp_seq->tp_getrange_index             = DeeType_Optimize_tp_getrange_index(self, base_seq->tp_getrange_index);
			self->tp_seq->tp_delrange_index             = DeeType_Optimize_tp_delrange_index(self, base_seq->tp_delrange_index);
			self->tp_seq->tp_setrange_index             = DeeType_Optimize_tp_setrange_index(self, base_seq->tp_setrange_index);
			self->tp_seq->tp_getrange_index_n           = DeeType_Optimize_tp_getrange_index_n(self, base_seq->tp_getrange_index_n);
			self->tp_seq->tp_delrange_index_n           = DeeType_Optimize_tp_delrange_index_n(self, base_seq->tp_delrange_index_n);
			self->tp_seq->tp_setrange_index_n           = DeeType_Optimize_tp_setrange_index_n(self, base_seq->tp_setrange_index_n);
			self->tp_seq->tp_size_fast                  = DeeType_Optimize_tp_size_fast(self, base_seq->tp_size_fast);
			self->tp_seq->tp_getitem_index_fast         = base_seq->tp_getitem_index_fast;
			self->tp_seq->tp_trygetitem                 = DeeType_Optimize_tp_trygetitem(self, base_seq->tp_trygetitem);
			self->tp_seq->tp_trygetitem_string_hash     = DeeType_Optimize_tp_trygetitem_string_hash(self, base_seq->tp_trygetitem_string_hash);
			self->tp_seq->tp_getitem_string_hash        = DeeType_Optimize_tp_getitem_string_hash(self, base_seq->tp_getitem_string_hash);
			self->tp_seq->tp_delitem_string_hash        = DeeType_Optimize_tp_delitem_string_hash(self, base_seq->tp_delitem_string_hash);
			self->tp_seq->tp_setitem_string_hash        = DeeType_Optimize_tp_setitem_string_hash(self, base_seq->tp_setitem_string_hash);
			self->tp_seq->tp_bounditem_string_hash      = DeeType_Optimize_tp_bounditem_string_hash(self, base_seq->tp_bounditem_string_hash);
			self->tp_seq->tp_hasitem_string_hash        = DeeType_Optimize_tp_hasitem_string_hash(self, base_seq->tp_hasitem_string_hash);
			self->tp_seq->tp_trygetitem_string_len_hash = DeeType_Optimize_tp_trygetitem_string_len_hash(self, base_seq->tp_trygetitem_string_len_hash);
			self->tp_seq->tp_getitem_string_len_hash    = DeeType_Optimize_tp_getitem_string_len_hash(self, base_seq->tp_getitem_string_len_hash);
			self->tp_seq->tp_delitem_string_len_hash    = DeeType_Optimize_tp_delitem_string_len_hash(self, base_seq->tp_delitem_string_len_hash);
			self->tp_seq->tp_setitem_string_len_hash    = DeeType_Optimize_tp_setitem_string_len_hash(self, base_seq->tp_setitem_string_len_hash);
			self->tp_seq->tp_bounditem_string_len_hash  = DeeType_Optimize_tp_bounditem_string_len_hash(self, base_seq->tp_bounditem_string_len_hash);
			self->tp_seq->tp_hasitem_string_len_hash    = DeeType_Optimize_tp_hasitem_string_len_hash(self, base_seq->tp_hasitem_string_len_hash);
			self->tp_seq->tp_asvector                   = DeeType_Optimize_tp_asvector(self, base_seq->tp_asvector);
			self->tp_seq->tp_asvector_nothrow           = DeeType_Optimize_tp_asvector_nothrow(self, base_seq->tp_asvector_nothrow);
			self->tp_seq->tp_unpack                     = DeeType_Optimize_tp_unpack(self, base_seq->tp_unpack);
			self->tp_seq->tp_unpack_ub                  = DeeType_Optimize_tp_unpack_ub(self, base_seq->tp_unpack_ub);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}


INTERN NONNULL((1)) bool DCALL
DeeType_InheritIter(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_iter != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_iter) {
			if (base_seq->tp_foreach == NULL)
				base_seq->tp_foreach = &DeeObject_DefaultForeachWithIter;
			if (base_seq->tp_foreach_pair == NULL)
				base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithIter;
			if (base_seq->tp_unpack == NULL)
				base_seq->tp_unpack = &DeeSeq_DefaultUnpackWithIter;
			if (base_seq->tp_unpack_ub == NULL)
				base_seq->tp_unpack_ub = base_seq->tp_unpack;
			return true;
		} else if (base_seq->tp_foreach) {
			base_seq->tp_iter = &DeeObject_DefaultIterWithForeach;
			if (base_seq->tp_foreach_pair == NULL)
				base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithForeach;
			if (base_seq->tp_unpack == NULL)
				base_seq->tp_unpack = &DeeSeq_DefaultUnpackWithForeach;
			if (base_seq->tp_unpack_ub == NULL)
				base_seq->tp_unpack_ub = base_seq->tp_unpack;
			return true;
		} else if (base_seq->tp_foreach_pair) {
			base_seq->tp_iter    = &DeeObject_DefaultIterWithForeachPair;
			base_seq->tp_foreach = &DeeObject_DefaultForeachWithForeachPair;
			if (base_seq->tp_unpack == NULL)
				base_seq->tp_unpack = &DeeSeq_DefaultUnpackWithForeach;
			if (base_seq->tp_unpack_ub == NULL)
				base_seq->tp_unpack_ub = base_seq->tp_unpack;
			return true;
		} else if (base_seq->tp_enumerate) {
			base_seq->tp_iter         = &DeeObject_DefaultIterWithEnumerate;
			base_seq->tp_foreach      = &DeeObject_DefaultForeachWithEnumerate;
			base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerate;
			if (base_seq->tp_enumerate_index == NULL)
				base_seq->tp_enumerate_index = &DeeObject_DefaultEnumerateIndexWithEnumerate;
			return true;
		} else if (base_seq->tp_enumerate_index) {
			base_seq->tp_iter         = &DeeObject_DefaultIterWithEnumerateIndex;
			base_seq->tp_foreach      = &DeeObject_DefaultForeachWithEnumerateIndex;
			base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerateIndex;
			base_seq->tp_enumerate    = &DeeObject_DefaultEnumerateWithEnumerateIndex;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_iter ||
		                         !base_seq->tp_foreach ||
		                         !base_seq->tp_foreach_pair)) {
			if (!DeeType_InheritIter(base))
				continue;
		}
		base_seq = base->tp_seq;
		LOG_INHERIT(base, self, "operator iter");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritIter(origin);
			self->tp_seq->tp_iter             = DeeType_Optimize_tp_iter(self, base_seq->tp_iter);
			self->tp_seq->tp_foreach          = DeeType_Optimize_tp_foreach(self, base_seq->tp_foreach);
			self->tp_seq->tp_foreach_pair     = DeeType_Optimize_tp_foreach_pair(self, base_seq->tp_foreach_pair);
			self->tp_seq->tp_asvector         = DeeType_Optimize_tp_asvector(self, base_seq->tp_asvector);
			self->tp_seq->tp_asvector_nothrow = DeeType_Optimize_tp_asvector_nothrow(self, base_seq->tp_asvector_nothrow);
			self->tp_seq->tp_unpack           = DeeType_Optimize_tp_unpack(self, base_seq->tp_unpack);
			self->tp_seq->tp_unpack_ub        = DeeType_Optimize_tp_unpack_ub(self, base_seq->tp_unpack_ub);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritSize(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_sizeob != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_size) {
			if (base_seq->tp_sizeob == NULL)
				base_seq->tp_sizeob = &DeeObject_DefaultSizeObWithSize;
			if (base_seq->tp_size_fast == NULL)
				base_seq->tp_size_fast = &DeeObject_DefaultSizeFastWithErrorNotFast;
			return true;
		} else if (base_seq->tp_sizeob) {
			if (base_seq->tp_size == NULL)
				base_seq->tp_size = &DeeObject_DefaultSizeWithSizeOb;
			if (base_seq->tp_size_fast == NULL)
				base_seq->tp_size_fast = &DeeObject_DefaultSizeFastWithErrorNotFast;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_size ||
		                         !base_seq->tp_sizeob ||
		                         !base_seq->tp_size_fast)) {
			if (!DeeType_InheritSize(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator size");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSize(origin);
			self->tp_seq->tp_size      = DeeType_Optimize_tp_size(self, base_seq->tp_size);
			self->tp_seq->tp_sizeob    = DeeType_Optimize_tp_sizeob(self, base_seq->tp_sizeob);
			self->tp_seq->tp_size_fast = DeeType_Optimize_tp_size_fast(self, base_seq->tp_size_fast);
			if (!self->tp_seq->tp_getitem_index_fast) {
				if (self->tp_seq->tp_getitem ||
				    self->tp_seq->tp_getitem_index ||
				    self->tp_seq->tp_getitem_string_hash ||
				    self->tp_seq->tp_getitem_string_len_hash ||
				    self->tp_seq->tp_trygetitem ||
				    self->tp_seq->tp_trygetitem_string_hash ||
				    self->tp_seq->tp_trygetitem_string_len_hash) {
					/* Can't inherit "tp_getitem_index_fast" */
				} else {
					self->tp_seq->tp_getitem_index_fast = base_seq->tp_getitem_index_fast;
				}
			} else if (self->tp_seq->tp_getitem_index_fast != base_seq->tp_getitem_index_fast) {
				self->tp_seq->tp_getitem_index_fast = NULL;
			}
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritContains(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_contains != NULL;
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if ((!base->tp_seq || !base->tp_seq->tp_contains) &&
		    (!DeeType_InheritContains(base)))
			continue;
		base_seq = base->tp_seq;
		LOG_INHERIT(base, self, "operator contains");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritContains(origin);
			self->tp_seq->tp_contains = DeeType_Optimize_tp_contains(self, base_seq->tp_contains);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritGetItem(DeeTypeObject *__restrict self) {
	struct type_seq *seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_getitem != NULL;
	}

	seq = self->tp_seq;
	if (seq && ((seq->tp_getitem_index_fast && seq->tp_size) ||
	            seq->tp_getitem ||
	            seq->tp_getitem_index ||
	            seq->tp_getitem_string_hash ||
	            seq->tp_getitem_string_len_hash ||
	            seq->tp_trygetitem ||
	            seq->tp_trygetitem_index ||
	            seq->tp_trygetitem_string_hash ||
	            seq->tp_trygetitem_string_len_hash)) {

		/* Substitute related core operators. */
		if (!seq->tp_getitem_string_hash) {
			if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash;
			}
		}
		if (!seq->tp_getitem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash;
			}
		}
		if (!seq->tp_trygetitem_string_hash) {
			if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash;
			}
		}
		if (!seq->tp_trygetitem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash;
			}
		}

		/* Figure out what's the "main" way that getitem should be implemented. */
		if (seq->tp_getitem_index_fast && seq->tp_size)  {
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast;
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast;
			if (!seq->tp_bounditem_index)
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast;
			if (!seq->tp_hasitem)
				seq->tp_hasitem = &DeeSeq_DefaultHasItemWithSize; /* Special case: seq operator here is correct! */
			if (!seq->tp_hasitem_index)
				seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSize; /* Special case: seq operator here is correct! */

			/* No string-based getitem allowed -> set getitem operators. */
set_string_operators_as_error:
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithErrorRequiresInt;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithErrorRequiresInt;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithErrorRequiresInt;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithErrorRequiresInt;

			/* No string-based getitem allowed -> set related operators. */
			if (!seq->tp_bounditem_string_hash)
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt;
			if (!seq->tp_bounditem_string_len_hash)
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt;
			if (!seq->tp_hasitem_string_hash)
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithErrorRequiresInt;
			if (!seq->tp_hasitem_string_len_hash)
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt;
		} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemIndex;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItemIndex;
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemIndex;
			goto set_string_operators_as_error;
		} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemDefault;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItemIndex;
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemIndex;
			goto set_string_operators_as_error;
		} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithGetItem;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItem;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItem;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItem;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItem;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItem;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItem;
		} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItem;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItem;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItem;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItem;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithTryGetItem;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItem;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem;
		} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringHash;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash;
			if (!seq->tp_trygetitem_string_len_hash) {
				if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
					seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
				} else {
					seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash;
				}
			}
		} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringLenHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringLenHash;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
		} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash;
			if (!seq->tp_getitem_string_len_hash) {
				if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
					seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
				} else {
					seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash;
				}
			}
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringHash;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash;
		} else {
			ASSERT(Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq));
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringLenHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash;
		}

		/* Substitute sub-operators via equivalents within their group. */
		if (!seq->tp_bounditem) {
			if (Dee_type_seq_has_custom_tp_bounditem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemIndex;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq) && Dee_type_seq_has_custom_tp_hasitem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHash;
			} else {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemDefault;
			}
		}
		if (!seq->tp_bounditem_index) {
			if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq) && Dee_type_seq_has_custom_tp_hasitem_index(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithErrorRequiresString;
			} else {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault;
			}
		}
		if (!seq->tp_bounditem_string_hash) {
			if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt;
			} else {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemDefault;
			}
		}
		if (!seq->tp_bounditem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt;
			} else {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault;
			}
		}

		if (!seq->tp_hasitem) {
			if (Dee_type_seq_has_custom_tp_hasitem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemIndex;
			} else if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemIndex;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringLenHash;
			} else {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemDefault;
			}
		}
		if (!seq->tp_hasitem_index) {
			if (Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithErrorRequiresString;
			} else {
				seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSizeDefault; /* Shouldn't get here... */
			}
		}

		if (!seq->tp_hasitem_string_hash) {
			if (Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_hasitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithErrorRequiresInt;
			} else {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemDefault;
			}
		}

		if (!seq->tp_hasitem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_hasitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt;
			} else {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemDefault;
			}
		}

		return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!DeeType_InheritGetItem(base))
			continue;
		seq = base->tp_seq;
		LOG_INHERIT(base, self, "operator getitem");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritGetItem(origin);
			self->tp_seq->tp_getitem                    = DeeType_Optimize_tp_getitem(self, seq->tp_getitem);
			self->tp_seq->tp_getitem_index              = DeeType_Optimize_tp_getitem_index(self, seq->tp_getitem_index);
			self->tp_seq->tp_getitem_string_hash        = DeeType_Optimize_tp_getitem_string_hash(self, seq->tp_getitem_string_hash);
			self->tp_seq->tp_getitem_string_len_hash    = DeeType_Optimize_tp_getitem_string_len_hash(self, seq->tp_getitem_string_len_hash);
			self->tp_seq->tp_trygetitem                 = DeeType_Optimize_tp_trygetitem(self, seq->tp_trygetitem);
			self->tp_seq->tp_trygetitem_index           = DeeType_Optimize_tp_trygetitem_index(self, seq->tp_trygetitem_index);
			self->tp_seq->tp_trygetitem_string_hash     = DeeType_Optimize_tp_trygetitem_string_hash(self, seq->tp_trygetitem_string_hash);
			self->tp_seq->tp_trygetitem_string_len_hash = DeeType_Optimize_tp_trygetitem_string_len_hash(self, seq->tp_trygetitem_string_len_hash);
			self->tp_seq->tp_bounditem                  = DeeType_Optimize_tp_bounditem(self, seq->tp_bounditem);
			self->tp_seq->tp_bounditem_index            = DeeType_Optimize_tp_bounditem_index(self, seq->tp_bounditem_index);
			self->tp_seq->tp_bounditem_string_hash      = DeeType_Optimize_tp_bounditem_string_hash(self, seq->tp_bounditem_string_hash);
			self->tp_seq->tp_bounditem_string_len_hash  = DeeType_Optimize_tp_bounditem_string_len_hash(self, seq->tp_bounditem_string_len_hash);
			self->tp_seq->tp_hasitem                    = DeeType_Optimize_tp_hasitem(self, seq->tp_hasitem);
			self->tp_seq->tp_hasitem_index              = DeeType_Optimize_tp_hasitem_index(self, seq->tp_hasitem_index);
			self->tp_seq->tp_hasitem_string_hash        = DeeType_Optimize_tp_hasitem_string_hash(self, seq->tp_hasitem_string_hash);
			self->tp_seq->tp_hasitem_string_len_hash    = DeeType_Optimize_tp_hasitem_string_len_hash(self, seq->tp_hasitem_string_len_hash);

			/* Special handling for "tp_getitem_index_fast", so it's `index < size'
			 * invariant doesn't break when someone overwrites "operator size" */
			if ((seq->tp_getitem_index_fast != NULL) &&
			    (self->tp_seq->tp_size || DeeType_InheritSize(self)) &&
			    (self->tp_seq->tp_size == seq->tp_size)) {
				/* Can only inherit "tp_getitem_index_fast" if "tp_size" is also being inherited. */
				self->tp_seq->tp_getitem_index_fast = seq->tp_getitem_index_fast;
			} else {
				/* Else: use of "tp_getitem_index_fast" would be illegal. */
				self->tp_seq->tp_getitem_index_fast = NULL;
			}
		} else {
			self->tp_seq = seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritDelItem(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_delitem != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_delitem_string_hash) {
			if (!base_seq->tp_delitem_string_len_hash)
				base_seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash;
		} else {
			if (base_seq->tp_delitem_string_len_hash)
				base_seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash;
		}
		if (base_seq->tp_delitem_index) {
			if (base_seq->tp_delitem == NULL)
				base_seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemIndex;
			if (base_seq->tp_delitem_string_hash == NULL)
				base_seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithErrorRequiresInt;
			if (base_seq->tp_delitem_string_len_hash == NULL)
				base_seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithErrorRequiresInt;
			return true;
		} else if (base_seq->tp_delitem) {
			if (base_seq->tp_delitem_index == NULL)
				base_seq->tp_delitem_index = &DeeObject_DefaultDelItemIndexWithDelItem;
			if (base_seq->tp_delitem_string_hash == NULL)
				base_seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItem;
			if (base_seq->tp_delitem_string_len_hash == NULL)
				base_seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItem;
			return true;
		} else if (base_seq->tp_delitem_string_hash) {
			ASSERT(base_seq->tp_delitem_string_len_hash);
			if (base_seq->tp_delitem == NULL) {
				if (Dee_type_seq_has_custom_tp_delitem_string_len_hash(base_seq)) {
					base_seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringLenHash;
				} else {
					base_seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringHash;
				}
			}
			if (base_seq->tp_delitem_index == NULL)
				base_seq->tp_delitem_index = &DeeObject_DefaultDelItemIndexWithErrorRequiresString;
			return true;
		} else {
			ASSERT(!base_seq->tp_delitem_string_len_hash);
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_delitem ||
		                         !base_seq->tp_delitem_index ||
		                         !base_seq->tp_delitem_string_hash ||
		                         !base_seq->tp_delitem_string_len_hash)) {
			if (!DeeType_InheritDelItem(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator delitem");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritDelItem(origin);
			self->tp_seq->tp_delitem                 = DeeType_Optimize_tp_delitem(self, base_seq->tp_delitem);
			self->tp_seq->tp_delitem_index           = DeeType_Optimize_tp_delitem_index(self, base_seq->tp_delitem_index);
			self->tp_seq->tp_delitem_string_hash     = DeeType_Optimize_tp_delitem_string_hash(self, base_seq->tp_delitem_string_hash);
			self->tp_seq->tp_delitem_string_len_hash = DeeType_Optimize_tp_delitem_string_len_hash(self, base_seq->tp_delitem_string_len_hash);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritSetItem(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_setitem != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_setitem_string_hash) {
			if (!base_seq->tp_setitem_string_len_hash)
				base_seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash;
		} else {
			if (base_seq->tp_setitem_string_len_hash)
				base_seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash;
		}
		if (base_seq->tp_setitem_index) {
			if (base_seq->tp_setitem == NULL)
				base_seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemIndex;
			if (base_seq->tp_setitem_string_hash == NULL)
				base_seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithErrorRequiresInt;
			if (base_seq->tp_setitem_string_len_hash == NULL)
				base_seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithErrorRequiresInt;
			return true;
		} else if (base_seq->tp_setitem) {
			if (base_seq->tp_setitem_index == NULL)
				base_seq->tp_setitem_index = &DeeObject_DefaultSetItemIndexWithSetItem;
			if (base_seq->tp_setitem_string_hash == NULL)
				base_seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItem;
			if (base_seq->tp_setitem_string_len_hash == NULL)
				base_seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItem;
			return true;
		} else if (base_seq->tp_setitem_string_hash) {
			ASSERT(base_seq->tp_setitem_string_len_hash);
			if (base_seq->tp_setitem == NULL) {
				if (Dee_type_seq_has_custom_tp_setitem_string_len_hash(base_seq)) {
					base_seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringLenHash;
				} else {
					base_seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringHash;
				}
			}
			if (base_seq->tp_setitem_index == NULL)
				base_seq->tp_setitem_index = &DeeObject_DefaultSetItemIndexWithErrorRequiresString;
			return true;
		} else {
			ASSERT(!base_seq->tp_setitem_string_len_hash);
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_setitem ||
		                         !base_seq->tp_setitem_index ||
		                         !base_seq->tp_setitem_string_hash ||
		                         !base_seq->tp_setitem_string_len_hash)) {
			if (!DeeType_InheritSetItem(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator setitem");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSetItem(origin);
			self->tp_seq->tp_setitem                 = DeeType_Optimize_tp_setitem(self, base_seq->tp_setitem);
			self->tp_seq->tp_setitem_index           = DeeType_Optimize_tp_setitem_index(self, base_seq->tp_setitem_index);
			self->tp_seq->tp_setitem_string_hash     = DeeType_Optimize_tp_setitem_string_hash(self, base_seq->tp_setitem_string_hash);
			self->tp_seq->tp_setitem_string_len_hash = DeeType_Optimize_tp_setitem_string_len_hash(self, base_seq->tp_setitem_string_len_hash);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritGetRange(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_getrange != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_getrange_index && base_seq->tp_getrange_index_n) {
			if (base_seq->tp_getrange == NULL)
				base_seq->tp_getrange = &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN;
			return true;
		} else if (base_seq->tp_getrange) {
			if (base_seq->tp_getrange_index == NULL)
				base_seq->tp_getrange_index = &DeeObject_DefaultGetRangeIndexWithGetRange;
			if (base_seq->tp_getrange_index_n == NULL)
				base_seq->tp_getrange_index_n = &DeeObject_DefaultGetRangeIndexNWithGetRange;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_getrange ||
		                         !base_seq->tp_getrange_index ||
		                         !base_seq->tp_getrange_index_n)) {
			if (!DeeType_InheritGetRange(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator getrange");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritGetRange(origin);
			self->tp_seq->tp_getrange         = DeeType_Optimize_tp_getrange(self, base_seq->tp_getrange);
			self->tp_seq->tp_getrange_index   = DeeType_Optimize_tp_getrange_index(self, base_seq->tp_getrange_index);
			self->tp_seq->tp_getrange_index_n = DeeType_Optimize_tp_getrange_index_n(self, base_seq->tp_getrange_index_n);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritDelRange(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_delrange != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_delrange_index && base_seq->tp_delrange_index_n) {
			if (base_seq->tp_delrange == NULL)
				base_seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN;
			return true;
		} else if (base_seq->tp_delrange) {
			if (base_seq->tp_delrange_index == NULL)
				base_seq->tp_delrange_index = &DeeObject_DefaultDelRangeIndexWithDelRange;
			if (base_seq->tp_delrange_index_n == NULL)
				base_seq->tp_delrange_index_n = &DeeObject_DefaultDelRangeIndexNWithDelRange;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_delrange ||
		                         !base_seq->tp_delrange_index ||
		                         !base_seq->tp_delrange_index_n)) {
			if (!DeeType_InheritDelRange(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator delrange");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritDelRange(origin);
			self->tp_seq->tp_delrange         = DeeType_Optimize_tp_delrange(self, base_seq->tp_delrange);
			self->tp_seq->tp_delrange_index   = DeeType_Optimize_tp_delrange_index(self, base_seq->tp_delrange_index);
			self->tp_seq->tp_delrange_index_n = DeeType_Optimize_tp_delrange_index_n(self, base_seq->tp_delrange_index_n);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritSetRange(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_setrange != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_setrange_index && base_seq->tp_setrange_index_n) {
			if (base_seq->tp_setrange == NULL)
				base_seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN;
			return true;
		} else if (base_seq->tp_setrange) {
			if (base_seq->tp_setrange_index == NULL)
				base_seq->tp_setrange_index = &DeeObject_DefaultSetRangeIndexWithSetRange;
			if (base_seq->tp_setrange_index_n == NULL)
				base_seq->tp_setrange_index_n = &DeeObject_DefaultSetRangeIndexNWithSetRange;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_setrange ||
		                         !base_seq->tp_setrange_index ||
		                         !base_seq->tp_setrange_index_n)) {
			if (!DeeType_InheritSetRange(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator setrange");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSetRange(origin);
			self->tp_seq->tp_setrange         = DeeType_Optimize_tp_setrange(self, base_seq->tp_setrange);
			self->tp_seq->tp_setrange_index   = DeeType_Optimize_tp_setrange_index(self, base_seq->tp_setrange_index);
			self->tp_seq->tp_setrange_index_n = DeeType_Optimize_tp_setrange_index_n(self, base_seq->tp_setrange_index_n);
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

/* Try to substitute default compare operators. */
PRIVATE NONNULL((1)) void DCALL
DeeType_SubstituteDefaultCompareOperators(DeeTypeObject *__restrict self) {
	struct type_cmp *cmp = (ASSERT(self->tp_cmp), self->tp_cmp);
	bool has_eq = cmp->tp_eq && !DeeType_IsDefaultEq(cmp->tp_eq);
	bool has_ne = cmp->tp_ne && !DeeType_IsDefaultNe(cmp->tp_ne);
	bool has_lo = cmp->tp_lo && !DeeType_IsDefaultLo(cmp->tp_lo);
	bool has_le = cmp->tp_le && !DeeType_IsDefaultLe(cmp->tp_le);
	bool has_gr = cmp->tp_gr && !DeeType_IsDefaultGr(cmp->tp_gr);
	bool has_ge = cmp->tp_ge && !DeeType_IsDefaultGe(cmp->tp_ge);
	bool has_compare_eq = cmp->tp_compare_eq && !DeeType_IsDefaultCompareEq(cmp->tp_compare_eq);
	bool has_compare = cmp->tp_compare && !DeeType_IsDefaultCompare(cmp->tp_compare);

	if (!cmp->tp_compare) {
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
			/* Sets and maps cannot be <=> compared:
			 *
			 *           +-----------+
			 *           |           |
			 *     +-----|-----+     |
			 *     |  A  |  B  |  C  |
			 *     +-----|-----+     |
			 *       ^   |           |
			 *       |   +-----------+
			 *       |       ^
			 *       |       |
			 *     SET_1   SET_2
			 *
			 * Here, SET_1 and SET_2 are neither sub-sets, nor super-set
			 * of each other, nor equal to each other:
			 * >> assert SET_1 != SET_2;
			 * >> assert !(SET_1 < SET_2);   // not subset
			 * >> assert !(SET_1 > SET_2);   // not superset
			 *
			 * As such, there is no valid value that could be returned
			 * by the rocketship operator (<=>), meaning that such a
			 * compare operation must not be substituted. */
			if (has_eq && has_lo) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndLo;
			} else if (has_eq && has_le) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndLe;
			} else if (has_eq && has_gr) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndGr;
			} else if (has_eq && has_ge) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndGe;
			} else if (has_ne && has_lo) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndLo;
			} else if (has_ne && has_le) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndLe;
			} else if (has_ne && has_gr) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndGr;
			} else if (has_ne && has_ge) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndGe;
			} else if (has_lo && has_gr) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithLoAndGr;
			} else if (has_le && has_ge) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithLeAndGe;
			}
		}
	}

	if (!cmp->tp_compare_eq) {
		if (has_compare) {
			cmp->tp_compare_eq = cmp->tp_compare;
		} else if (has_eq) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithEq;
		} else if (has_ne) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithNe;
		} else if (has_lo && has_gr) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithLeAndGe;
		}
	}

	if (!cmp->tp_trycompare_eq) {
		if (has_compare_eq) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithCompareEq;
		} else if (has_eq) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithEq;
		} else if (has_ne) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithNe;
		} else if (has_compare) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithCompare;
		} else if (has_lo && has_gr) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithLeAndGe;
		}
	}

	if (!cmp->tp_eq) {
		if (has_compare_eq) {
			cmp->tp_eq = &DeeObject_DefaultEqWithCompareEq;
		} else if (has_ne) {
			cmp->tp_eq = &DeeObject_DefaultEqWithNe;
		} else if (has_lo && has_gr) {
			cmp->tp_eq = &DeeObject_DefaultEqWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_eq = &DeeObject_DefaultEqWithLeAndGe;
		} else if (cmp->tp_compare_eq) {
			cmp->tp_eq = &DeeObject_DefaultEqWithCompareEqDefault;
		}
	}

	if (!cmp->tp_ne) {
		if (has_compare_eq) {
			cmp->tp_ne = &DeeObject_DefaultNeWithCompareEq;
		} else if (has_eq) {
			cmp->tp_ne = &DeeObject_DefaultNeWithEq;
		} else if (has_lo && has_gr) {
			cmp->tp_ne = &DeeObject_DefaultNeWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_ne = &DeeObject_DefaultNeWithLeAndGe;
		} else if (cmp->tp_compare_eq) {
			cmp->tp_ne = &DeeObject_DefaultNeWithCompareEqDefault;
		}
	}

	if (!cmp->tp_lo) {
		if (has_compare) {
			cmp->tp_lo = &DeeObject_DefaultLoWithCompare;
		} else if (has_ge) {
			cmp->tp_lo = &DeeObject_DefaultLoWithGe;
		} else if (cmp->tp_compare) {
			cmp->tp_lo = &DeeObject_DefaultLoWithCompareDefault;
		}
	}

	if (!cmp->tp_le) {
		if (has_compare) {
			cmp->tp_le = &DeeObject_DefaultLeWithCompare;
		} else if (has_gr) {
			cmp->tp_le = &DeeObject_DefaultLeWithGr;
		} else if (cmp->tp_compare) {
			cmp->tp_le = &DeeObject_DefaultLeWithCompareDefault;
		}
	}

	if (!cmp->tp_gr) {
		if (has_compare) {
			cmp->tp_gr = &DeeObject_DefaultGrWithCompare;
		} else if (has_le) {
			cmp->tp_gr = &DeeObject_DefaultGrWithLe;
		} else if (cmp->tp_compare) {
			cmp->tp_gr = &DeeObject_DefaultGrWithCompareDefault;
		}
	}

	if (!cmp->tp_ge) {
		if (has_compare) {
			cmp->tp_ge = &DeeObject_DefaultGeWithCompare;
		} else if (has_lo) {
			cmp->tp_ge = &DeeObject_DefaultGeWithLo;
		} else if (cmp->tp_compare) {
			cmp->tp_ge = &DeeObject_DefaultGeWithCompareDefault;
		}
	}
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritCompare(DeeTypeObject *__restrict self) {
	struct type_cmp *base_cmp;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_cmp = self->tp_cmp;
	if (base_cmp) {
		DeeType_SubstituteDefaultCompareOperators(self);
		if (base_cmp->tp_hash || base_cmp->tp_eq || base_cmp->tp_lo || base_cmp->tp_le)
			return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_cmp = base->tp_cmp;
		if (base_cmp == NULL ||
		    (!base_cmp->tp_hash ||
		     !base_cmp->tp_compare_eq || !base_cmp->tp_compare ||
		     !base_cmp->tp_trycompare_eq ||
		     !base_cmp->tp_eq || !base_cmp->tp_ne ||
		     !base_cmp->tp_lo || !base_cmp->tp_le ||
		     !base_cmp->tp_gr || !base_cmp->tp_ge)) {
			if (!DeeType_InheritCompare(base))
				continue;
			base_cmp = base->tp_cmp;
		}
		LOG_INHERIT(base, self, "operator <compare>");
		if (self->tp_cmp) {
			DeeTypeObject *origin = DeeType_GetCmpOrigin(self);
			if unlikely(origin)
				return DeeType_InheritCompare(origin);
			self->tp_cmp->tp_hash          = DeeType_Optimize_tp_hash(self, base_cmp->tp_hash);
			self->tp_cmp->tp_compare       = DeeType_Optimize_tp_compare(self, base_cmp->tp_compare);
			self->tp_cmp->tp_compare_eq    = DeeType_Optimize_tp_compare_eq(self, base_cmp->tp_compare_eq);
			self->tp_cmp->tp_trycompare_eq = DeeType_Optimize_tp_trycompare_eq(self, base_cmp->tp_trycompare_eq);
			self->tp_cmp->tp_eq            = DeeType_Optimize_tp_eq(self, base_cmp->tp_eq);
			self->tp_cmp->tp_ne            = DeeType_Optimize_tp_ne(self, base_cmp->tp_ne);
			self->tp_cmp->tp_lo            = DeeType_Optimize_tp_lo(self, base_cmp->tp_lo);
			self->tp_cmp->tp_le            = DeeType_Optimize_tp_le(self, base_cmp->tp_le);
			self->tp_cmp->tp_gr            = DeeType_Optimize_tp_gr(self, base_cmp->tp_gr);
			self->tp_cmp->tp_ge            = DeeType_Optimize_tp_ge(self, base_cmp->tp_ge);
		} else {
			self->tp_cmp = DeeType_Optimize_tp_cmp(self, base_cmp);
		}
		return true;
	}
	return false;
}


INTERN NONNULL((1)) bool DCALL
DeeType_InheritBool(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_cast.tp_bool) {
			if (!DeeType_InheritBool(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator bool");
		self->tp_cast.tp_bool = DeeType_Optimize_tp_bool(self, base->tp_cast.tp_bool);
		return true;
	}
	return false;
}



INTDEF int DCALL none_i1(void *UNUSED(a));

INTERN NONNULL((1)) bool DCALL
DeeType_InheritWith(DeeTypeObject *__restrict self) {
	struct type_with *base_with;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_with = self->tp_with;
	if (base_with) {
		if (base_with->tp_enter) {
			/* Special case: When `tp_enter' is implemented,
			 * a missing `tp_leave' behaves as a no-op. */
			if (base_with->tp_leave == NULL)
				base_with->tp_leave = (int (DCALL *)(DeeObject *__restrict))&none_i1;
			return true;
		} else if (base_with->tp_leave) {
			/* Special case: When `tp_leave' is implemented,
			 * a missing `tp_enter' behaves as a no-op. */
			if (base_with->tp_enter == NULL)
				base_with->tp_enter = (int (DCALL *)(DeeObject *__restrict))&none_i1;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_with = base->tp_with;
		if (base_with == NULL || (base_with->tp_enter == NULL ||
		                          base_with->tp_leave == NULL)) {
			if (!DeeType_InheritWith(base))
				continue;
			base_with = base->tp_with;
		}
		LOG_INHERIT(base, self, "operator <with>");
		if unlikely(self->tp_with) {
			self->tp_with->tp_enter = base_with->tp_enter;
			self->tp_with->tp_leave = base_with->tp_leave;
		} else {
			self->tp_with = base_with;
		}
		return true;
	}
	return false;
}


INTERN NONNULL((1)) bool DCALL
DeeType_InheritBuffer(DeeTypeObject *__restrict self) {
	struct type_buffer *base_buffer;
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_buffer = base->tp_buffer;
		if (base_buffer == NULL || !base_buffer->tp_getbuf) {
			if (!DeeType_InheritBuffer(base))
				continue;
			base_buffer = base->tp_buffer;
		}
		LOG_INHERIT(base, self, "<BUFFER>");
		if unlikely(self->tp_buffer) {
			memcpy(self->tp_buffer, base_buffer, sizeof(struct type_buffer));
		} else {
			self->tp_buffer = base_buffer;
		}
		return true;
	}
	return false;
}




INTERN NONNULL((1)) bool DCALL
DeeType_InheritNSI(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || !base_seq->tp_nsi) {
			if (!DeeType_InheritNSI(base))
				continue;
		}
		if (self->tp_seq != NULL) /* Some other sequence interface has already been implemented! */
			return false;
		LOG_INHERIT(base, self, "<NSI>");
		self->tp_seq = base->tp_seq;
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritNII(DeeTypeObject *__restrict self) {
	struct type_cmp *base_cmp;
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_cmp = base->tp_cmp;
		if (base_cmp == NULL || !base_cmp->tp_nii) {
			if (!DeeType_InheritNSI(base))
				continue;
		}
		if (self->tp_cmp != NULL) /* Some other iterator-compare interface has already been implemented! */
			return false;
		LOG_INHERIT(base, self, "<NII>");
		self->tp_cmp = base->tp_cmp;
		return true;
	}
	return false;
}

#undef LOG_INHERIT

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_INHERIT_C */
