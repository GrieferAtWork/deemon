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

	/* Functions for mutable sequences. */
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
	union Dee_tsc_uslot          tsc_xchitem_index_data;
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


	/************************************************************************/
	/* For `deemon.Set' (only allocated if derived from "Set")              */
	/************************************************************************/
	/* TODO */


	/************************************************************************/
	/* For `deemon.Mapping' (only allocated if derived from "Mapping")      */
	/************************************************************************/
	/* TODO */
};

INTDEF NONNULL((1)) void DCALL Dee_type_seq_cache_destroy(struct Dee_type_seq_cache *__restrict self);

INTDEF WUNUSED NONNULL((1)) struct Dee_type_seq_cache *DCALL
DeeType_TryRequireSeqCache(DeeTypeObject *__restrict self);

/* Type sequence operator definition functions. */
INTDEF WUNUSED NONNULL((1)) Dee_tsc_foreach_reverse_t DCALL DeeType_SeqCache_TryRequireForeachReverse(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) bool DCALL DeeType_SeqCache_HasPrivateForeachReverse(DeeTypeObject *__restrict self);
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

/* Mutable sequence functions */
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
#define DeeSeq_Find(self, item, start, end) \
	(*DeeType_SeqCache_RequireFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_RFind(self, item, start, end) \
	(*DeeType_SeqCache_RequireRFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_FindWithKey(self, item, start, end, key) \
	(*DeeType_SeqCache_RequireFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_RFindWithKey(self, item, start, end, key) \
	(*DeeType_SeqCache_RequireRFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)

/* Invoke mutable-sequence operators */
/* TODO: Remove the "new_" prefix and remove functions from "seq_mutable.c" */
#define new_DeeSeq_Erase(self, index, count) \
	(*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, index, count)
#define new_DeeSeq_Insert(self, index, item) \
	(*DeeType_SeqCache_RequireInsert(Dee_TYPE(self)))(self, index, item)
#define new_DeeSeq_InsertAll(self, index, items) \
	(*DeeType_SeqCache_RequireInsertAll(Dee_TYPE(self)))(self, index, items)
#define new_DeeSeq_PushFront(self, item) \
	(*DeeType_SeqCache_RequirePushFront(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_Append(self, item) \
	(*DeeType_SeqCache_RequireAppend(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_Extend(self, items) \
	(*DeeType_SeqCache_RequireExtend(Dee_TYPE(self)))(self, items)
#define new_DeeSeq_XchItemIndex(self, index, value) \
	(*DeeType_SeqCache_RequireXchItemIndex(Dee_TYPE(self)))(self, index, value)
#define new_DeeSeq_Clear(self) \
	(*DeeType_SeqCache_RequireClear(Dee_TYPE(self)))(self)
#define new_DeeSeq_Pop(self, index) \
	(*DeeType_SeqCache_RequirePop(Dee_TYPE(self)))(self, index)
#define new_DeeSeq_Remove(self, item, start, end) \
	(*DeeType_SeqCache_RequireRemove(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_RemoveWithKey(self, item, start, end, key) \
	(*DeeType_SeqCache_RequireRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_RRemove(self, item, start, end) \
	(*DeeType_SeqCache_RequireRRemove(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_RRemoveWithKey(self, item, start, end, key) \
	(*DeeType_SeqCache_RequireRRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_RemoveAll(self, item, start, end, max) \
	(*DeeType_SeqCache_RequireRemoveAll(Dee_TYPE(self)))(self, item, start, end, max)
#define new_DeeSeq_RemoveAllWithKey(self, item, start, end, max, key) \
	(*DeeType_SeqCache_RequireRemoveAllWithKey(Dee_TYPE(self)))(self, item, start, end, max, key)
#define new_DeeSeq_RemoveIf(self, should, start, end, max) \
	(*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, should, start, end, max)
#define new_DeeSeq_Resize(self, newsize, filler) \
	(*DeeType_SeqCache_RequireResize(Dee_TYPE(self)))(self, newsize, filler)
#define new_DeeSeq_Fill(self, start, end, filler) \
	(*DeeType_SeqCache_RequireFill(Dee_TYPE(self)))(self, start, end, filler)
#define new_DeeSeq_Reverse(self, start, end) \
	(*DeeType_SeqCache_RequireReverse(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_Reversed(self, start, end) \
	(*DeeType_SeqCache_RequireReversed(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_Sort(self, start, end) \
	(*DeeType_SeqCache_RequireSort(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_SortWithKey(self, start, end, key) \
	(*DeeType_SeqCache_RequireSortWithKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_Sorted(self, start, end) \
	(*DeeType_SeqCache_RequireSorted(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_SortedWithKey(self, start, end, key) \
	(*DeeType_SeqCache_RequireSortedWithKey(Dee_TYPE(self)))(self, start, end, key)

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

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithTSCEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithTSCEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithTSCEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);


/* Mutable sequence functions */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallAttrErase(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataFunction(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataMethod(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataKwMethod(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithDelRangeIndex(DeeObject *self, size_t index, size_t count);
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
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemIndexDataFunction(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemIndexDataMethod(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemIndexDataKwMethod(DeeObject *self, size_t index, DeeObject *value);
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
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithError(DeeObject *self, Dee_ssize_t index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithTSCRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithTSCRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);

INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataFunction(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataKwMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
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
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxyCopyDefault(DeeObject *self, size_t start, size_t end);

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
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithProxyCopyDefault(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallAttrSorted(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithProxyCopyDefault(DeeObject *self, size_t start, size_t end, DeeObject *key);



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
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL generic_seq_count(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL generic_seq_count_with_key(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_seq_contains_with_key(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_locate(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL generic_seq_locate_with_key(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_rlocate(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL generic_seq_rlocate_with_key(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_startswith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_seq_startswith_with_key(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_endswith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_seq_endswith_with_key(DeeObject *self, DeeObject *item, DeeObject *key);


/* Generic sequence mutable function pointers. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_erase(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_insert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_insertall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_pushfront(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_append(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_extend(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_xchitem(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_clear(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_pop(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_remove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_rremove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_removeall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_removeif(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_resize(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_fill(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_reverse(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_reversed(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_sort(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_seq_sorted(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H */
