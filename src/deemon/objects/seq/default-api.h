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

/* @return: * :         Index of `elem' in `self'
 * @return: (size_t)-1: `elem' could not be located in `self'
 * @return: (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_find_t)(DeeObject *self, DeeObject *elem, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_find_with_key_t)(DeeObject *self, DeeObject *elem, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_rfind_t)(DeeObject *self, DeeObject *elem, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_rfind_with_key_t)(DeeObject *self, DeeObject *elem, size_t start, size_t end, DeeObject *key);

struct Dee_type_seq_cache {
	Dee_tsc_foreach_reverse_t         tsc_foreach_reverse;
	Dee_tsc_enumerate_index_t         tsc_enumerate_index; /* Same as normal enumerate-index, but treated like `(self as Sequence).<enumerate_index>' */
	Dee_tsc_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	Dee_tsc_nonempty_t                tsc_nonempty;

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

	/* Find functions. */
	Dee_tsc_find_t           tsc_find;
	Dee_tsc_find_with_key_t  tsc_find_with_key;
	Dee_tsc_rfind_t          tsc_rfind;
	Dee_tsc_rfind_with_key_t tsc_rfind_with_key;
};

/* Type sequence operator definition functions. */
INTDEF WUNUSED NONNULL((1)) Dee_tsc_foreach_reverse_t DCALL DeeType_SeqCache_TryRequireForeachReverse(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_reverse_t DCALL DeeType_SeqCache_TryRequireEnumerateIndexReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_t DCALL DeeType_SeqCache_RequireEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_nonempty_t DCALL DeeType_SeqCache_RequireNonEmpty(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getfirst_t DCALL DeeType_SeqCache_RequireGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundfirst_t DCALL DeeType_SeqCache_RequireBoundFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_delfirst_t DCALL DeeType_SeqCache_RequireDelFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setfirst_t DCALL DeeType_SeqCache_RequireSetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getlast_t DCALL DeeType_SeqCache_RequireGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundlast_t DCALL DeeType_SeqCache_RequireBoundLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_dellast_t DCALL DeeType_SeqCache_RequireDelLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setlast_t DCALL DeeType_SeqCache_RequireSetLast(DeeTypeObject *__restrict self);

INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_find_t DCALL DeeType_SeqCache_RequireFind(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_find_with_key_t DCALL DeeType_SeqCache_RequireFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rfind_t DCALL DeeType_SeqCache_RequireRFind(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rfind_with_key_t DCALL DeeType_SeqCache_RequireRFindWithKey(DeeTypeObject *__restrict self);

/* Same as `DeeObject_EnumerateIndex()', but also works for treats `self' as `self as Sequence' */
#define DeeSeq_EnumerateIndex(self, proc, arg, start, end) \
	(*DeeType_SeqCache_RequireEnumerateIndex(Dee_TYPE(self)))(self, proc, arg, start, end)

/* Helpers to quickly invoke default sequence functions. */
#define DeeSeq_GetFirst(self)    (*DeeType_SeqCache_RequireGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_BoundFirst(self)  (*DeeType_SeqCache_RequireBoundFirst(Dee_TYPE(self)))(self)
#define DeeSeq_DelFirst(self)    (*DeeType_SeqCache_RequireDelFirst(Dee_TYPE(self)))(self)
#define DeeSeq_SetFirst(self, v) (*DeeType_SeqCache_RequireSetFirst(Dee_TYPE(self)))(self, v)
#define DeeSeq_GetLast(self)     (*DeeType_SeqCache_RequireGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_BoundLast(self)   (*DeeType_SeqCache_RequireBoundLast(Dee_TYPE(self)))(self)
#define DeeSeq_DelLast(self)     (*DeeType_SeqCache_RequireDelLast(Dee_TYPE(self)))(self)
#define DeeSeq_SetLast(self, v)  (*DeeType_SeqCache_RequireSetLast(Dee_TYPE(self)))(self, v)

#define DeeSeq_NonEmpty(self) \
	(*DeeType_SeqCache_RequireNonEmpty(Dee_TYPE(self)))(self)
#define DeeSeq_Find(self, elem, start, end) \
	(*DeeType_SeqCache_RequireFind(Dee_TYPE(self)))(self, elem, start, end)
#define DeeSeq_RFind(self, elem, start, end) \
	(*DeeType_SeqCache_RequireRFind(Dee_TYPE(self)))(self, elem, start, end)
#define DeeSeq_FindWithKey(self, elem, start, end, key) \
	(*DeeType_SeqCache_RequireFindWithKey(Dee_TYPE(self)))(self, elem, start, end, key)
#define DeeSeq_RFindWithKey(self, elem, start, end, key) \
	(*DeeType_SeqCache_RequireRFindWithKey(Dee_TYPE(self)))(self, elem, start, end, key)


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

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithTSCEnumerateIndex(DeeObject *self, DeeObject *elem, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithTSCEnumerateIndex(DeeObject *self, DeeObject *elem, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *elem, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithTSCEnumerateIndex(DeeObject *self, DeeObject *elem, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *elem, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndex(DeeObject *self, DeeObject *elem, size_t start, size_t end, DeeObject *key);


/* Generic sequence function hooks (used as function pointers of `type_method' / `type_getset' of Sequence/Set/Mapping) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_getfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_seq_boundfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_seq_delfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_setfirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_getlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_seq_boundlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_seq_dellast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_setlast(DeeObject *self, DeeObject *value);


/* Uncached default sequence operations. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_reduce(DeeObject *self, DeeObject *combine, /*nullable*/ DeeObject *init);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_seq_parity(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_min_with_key(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_max_with_key(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL generic_seq_count(DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL generic_seq_count_with_key(DeeObject *self, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_seq_contains_with_key(DeeObject *self, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_locate(DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL generic_seq_locate_with_key(DeeObject *self, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_rlocate(DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL generic_seq_rlocate_with_key(DeeObject *self, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_startswith(DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_seq_startswith_with_key(DeeObject *self, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_endswith(DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_seq_endswith_with_key(DeeObject *self, DeeObject *elem, DeeObject *key);


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H */
