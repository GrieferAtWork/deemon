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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C 1

#include "default-api.h"
/**/

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/overflow.h>

/**/
#include "repeat.h"

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#undef SSIZE_MIN
#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

#define DeeType_RequireBool(tp_self)           (((tp_self)->tp_cast.tp_bool) || DeeType_InheritBool(tp_self))
#define DeeType_RequireSize(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size) || DeeType_InheritSize(tp_self))
#define DeeType_RequireIter(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iter) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeach(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach) || DeeType_InheritIter(tp_self))
#define DeeType_RequireEnumerateIndex(tp_self) (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate_index) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate_index))
#define DeeType_RequireGetItemIndex(tp_self)   (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireDelItemIndex(tp_self)   (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem_index) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireSetItemIndex(tp_self)   (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem_index) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireGetRangeIndex(tp_self)  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange_index && (tp_self)->tp_seq->tp_getrange_index_n) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireDelRangeIndex(tp_self)  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange_index && (tp_self)->tp_seq->tp_delrange_index_n) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireSetRangeIndex(tp_self)  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange_index && (tp_self)->tp_seq->tp_setrange_index_n) || DeeType_InheritSetRange(tp_self))


PRIVATE WUNUSED NONNULL((1)) Dee_tsc_erase_t DCALL DeeType_SeqCache_RequireErase_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_insert_t DCALL DeeType_SeqCache_RequireInsert_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_insertall_t DCALL DeeType_SeqCache_RequireInsertAll_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_pushfront_t DCALL DeeType_SeqCache_RequirePushFront_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_append_t DCALL DeeType_SeqCache_RequireAppend_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_extend_t DCALL DeeType_SeqCache_RequireExtend_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_xchitem_index_t DCALL DeeType_SeqCache_RequireXchItemIndex_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_clear_t DCALL DeeType_SeqCache_RequireClear_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_pop_t DCALL DeeType_SeqCache_RequirePop_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_remove_t DCALL DeeType_SeqCache_RequireRemove_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_remove_with_key_t DCALL DeeType_SeqCache_RequireRemoveWithKey_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_rremove_t DCALL DeeType_SeqCache_RequireRRemove_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_rremove_with_key_t DCALL DeeType_SeqCache_RequireRRemoveWithKey_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_removeall_t DCALL DeeType_SeqCache_RequireRemoveAll_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_removeall_with_key_t DCALL DeeType_SeqCache_RequireRemoveAllWithKey_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_removeif_t DCALL DeeType_SeqCache_RequireRemoveIf_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_resize_t DCALL DeeType_SeqCache_RequireResize_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_fill_t DCALL DeeType_SeqCache_RequireFill_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_reverse_t DCALL DeeType_SeqCache_RequireReverse_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_reversed_t DCALL DeeType_SeqCache_RequireReversed_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sort_t DCALL DeeType_SeqCache_RequireSort_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sort_with_key_t DCALL DeeType_SeqCache_RequireSortWithKey_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sorted_t DCALL DeeType_SeqCache_RequireSorted_private_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sorted_with_key_t DCALL DeeType_SeqCache_RequireSortedWithKey_private_uncached(DeeTypeObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) Dee_tsc_erase_t DCALL DeeType_SeqCache_RequireErase_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_insert_t DCALL DeeType_SeqCache_RequireInsert_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_insertall_t DCALL DeeType_SeqCache_RequireInsertAll_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_pushfront_t DCALL DeeType_SeqCache_RequirePushFront_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_append_t DCALL DeeType_SeqCache_RequireAppend_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_extend_t DCALL DeeType_SeqCache_RequireExtend_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_xchitem_index_t DCALL DeeType_SeqCache_RequireXchItemIndex_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_clear_t DCALL DeeType_SeqCache_RequireClear_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_pop_t DCALL DeeType_SeqCache_RequirePop_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_remove_t DCALL DeeType_SeqCache_RequireRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_remove_with_key_t DCALL DeeType_SeqCache_RequireRemoveWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_rremove_t DCALL DeeType_SeqCache_RequireRRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_rremove_with_key_t DCALL DeeType_SeqCache_RequireRRemoveWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_removeall_t DCALL DeeType_SeqCache_RequireRemoveAll_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_removeall_with_key_t DCALL DeeType_SeqCache_RequireRemoveAllWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_removeif_t DCALL DeeType_SeqCache_RequireRemoveIf_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_resize_t DCALL DeeType_SeqCache_RequireResize_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_fill_t DCALL DeeType_SeqCache_RequireFill_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_reverse_t DCALL DeeType_SeqCache_RequireReverse_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_reversed_t DCALL DeeType_SeqCache_RequireReversed_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sort_t DCALL DeeType_SeqCache_RequireSort_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sort_with_key_t DCALL DeeType_SeqCache_RequireSortWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sorted_t DCALL DeeType_SeqCache_RequireSorted_uncached(DeeTypeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) Dee_tsc_sorted_with_key_t DCALL DeeType_SeqCache_RequireSortedWithKey_uncached(DeeTypeObject *__restrict self);


/* Mutable sequence functions */
PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_seq_not_vmutablef(DeeObject *self, char const *method_format, va_list args) {
	int result;
	DREF DeeObject *message, *error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(unicode_printer_printf(&printer, "Sequence type %k is not mutable or does not support call: ", Dee_TYPE(self)) < 0)
		goto err_printer;
	if unlikely(unicode_printer_vprintf(&printer, method_format, args) < 0)
		goto err_printer;
	message = unicode_printer_pack(&printer);
	if unlikely(!message)
		goto err;
	error = DeeObject_New(&DeeError_SequenceError, 1, &message);
	Dee_Decref_unlikely(message);
	if unlikely(!error)
		goto err;
	result = DeeError_Throw(error);
	Dee_Decref_unlikely(error);
	return result;
err_printer:
	unicode_printer_fini(&printer);
err:
	return -1;
}

PRIVATE ATTR_COLD NONNULL((1)) int
err_seq_not_mutablef(DeeObject *self, char const *method_format, ...) {
	int result;
	va_list args;
	va_start(args, method_format);
	result = err_seq_not_vmutablef(self, method_format, args);
	va_end(args);
	return result;
}

/************************************************************************/
/* erase()                                                              */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultEraseWithDelRangeIndex(DeeObject *self, size_t start, size_t count) {
	struct type_seq *seq;
	size_t end_index;
	if unlikely(OVERFLOW_UADD(start, count, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	seq = Dee_TYPE(self)->tp_seq;
	return (*seq->tp_delrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end_index);
err_overflow:
	return err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultEraseWithError(DeeObject *self, size_t start, size_t count) {
	return err_seq_not_mutablef(self, "erase(%" PRFuSIZ ", %" PRFuSIZ ")", start, count);
}


/************************************************************************/
/* insert()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertWithTSCInsertAll(DeeObject *self, size_t index, DeeObject *item) {
	int result;
	DREF DeeTupleObject *elem_tuple;
	elem_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!elem_tuple)
		goto err;
	DeeTuple_SET(elem_tuple, 0, item);
	result = (*DeeType_SeqCache_RequireInsertAll(Dee_TYPE(self)))(self, index, (DeeObject *)elem_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)elem_tuple);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertWithError(DeeObject *self, size_t index, DeeObject *item) {
	return err_seq_not_mutablef(self, "insert(%" PRFuSIZ ", %r)", index, item);
}


/************************************************************************/
/* insertall()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithSetRangeIndex(DeeObject *self, size_t index, DeeObject *items) {
	size_t end_index;
	size_t items_size = DeeObject_Size(items);
	if unlikely(items_size == (size_t)-1)
		goto err;
	if unlikely(OVERFLOW_UADD(index, items_size, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	return (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)index, (Dee_ssize_t)end_index, items);
err_overflow:
	err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
err:
	return -1;
}

struct default_insertall_with_foreach_insert_data {
	Dee_tsc_insert_t diawfid_insert; /* [1..1] Insert callback */
	DeeObject       *diawfid_self;   /* [1..1] The sequence to insert into */
	size_t           diawfid_index;  /* Next index for insertion */
};

PRIVATE WUNUSED_T NONNULL_T((2)) Dee_ssize_t DCALL
default_insertall_with_foreach_insert_cb(void *arg, DeeObject *item) {
	struct default_insertall_with_foreach_insert_data *data;
	data = (struct default_insertall_with_foreach_insert_data *)arg;
	return (Dee_ssize_t)(*data->diawfid_insert)(data->diawfid_self, data->diawfid_index++, item);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithTSCInsertForeach(DeeObject *self, size_t index, DeeObject *items) {
	struct default_insertall_with_foreach_insert_data data;
	data.diawfid_self   = self;
	data.diawfid_index  = index;
	data.diawfid_insert = DeeType_SeqCache_RequireInsert(Dee_TYPE(self));
	return (int)DeeObject_Foreach(items, &default_insertall_with_foreach_insert_cb, &data);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithError(DeeObject *self, size_t index, DeeObject *items) {
	return err_seq_not_mutablef(self, "insertall(%" PRFuSIZ ", %r)", index, items);
}


/************************************************************************/
/* pushfront()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultPushFrontWithTSCInsert(DeeObject *self, DeeObject *item) {
	return (*DeeType_SeqCache_RequireInsert(Dee_TYPE(self)))(self, 0, item);
}


/************************************************************************/
/* append()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithTSCExtend(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeTupleObject *elem_tuple;
	elem_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!elem_tuple)
		goto err;
	DeeTuple_SET(elem_tuple, 0, item);
	result = (*DeeType_SeqCache_RequireExtend(Dee_TYPE(self)))(self, (DeeObject *)elem_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)elem_tuple);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithSizeAndTSCInsert(DeeObject *self, DeeObject *item) {
	size_t selfsize;
	selfsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	return (*DeeType_SeqCache_RequireInsert(Dee_TYPE(self)))(self, selfsize, item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithError(DeeObject *self, DeeObject *item) {
	return err_seq_not_mutablef(self, "append(%r)", item);
}


/************************************************************************/
/* extend()                                                             */
/************************************************************************/
struct default_extend_with_foreach_append_data {
	Dee_tsc_append_t dewfad_append; /* [1..1] Append callback */
	DeeObject       *dewfad_self;   /* [1..1] The sequence to append to */
};

PRIVATE WUNUSED_T NONNULL_T((2)) Dee_ssize_t DCALL
default_extend_with_foreach_append_cb(void *arg, DeeObject *item) {
	struct default_extend_with_foreach_append_data *data;
	data = (struct default_extend_with_foreach_append_data *)arg;
	return (Dee_ssize_t)(*data->dewfad_append)(data->dewfad_self, item);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithTSCAppendForeach(DeeObject *self, DeeObject *items) {
	struct default_extend_with_foreach_append_data data;
	data.dewfad_self   = self;
	data.dewfad_append = DeeType_SeqCache_RequireAppend(Dee_TYPE(self));
	return (int)DeeObject_Foreach(items, &default_extend_with_foreach_append_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithSizeAndTSCInsertAll(DeeObject *self, DeeObject *items) {
	size_t selfsize;
	selfsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	return (*DeeType_SeqCache_RequireInsertAll(Dee_TYPE(self)))(self, selfsize, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithError(DeeObject *self, DeeObject *items) {
	return err_seq_not_mutablef(self, "extend(%r)", items);
}


/************************************************************************/
/* xchitem()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultXchItemIndexWithGetItemIndexAndSetItemIndex(DeeObject *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	result = (*seq->tp_getitem_index)(self, index);
	if likely(result) {
		if unlikely((*seq->tp_setitem_index)(self, index, value))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultXchItemIndexWithError(DeeObject *self, size_t index, DeeObject *value) {
	err_seq_not_mutablef(self, "xchitem(%" PRFuSIZ ", %r)", index, value);
	return NULL;
}


/************************************************************************/
/* clear()                                                              */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithDelRangeIndexN(DeeObject *self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delrange_index_n)(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithSetRangeIndexN(DeeObject *self) {
	return (*Dee_TYPE(self)->tp_seq->tp_setrange_index_n)(self, 0, Dee_EmptySeq);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithTSCErase(DeeObject *self) {
	return (*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, 0, (size_t)-1);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithError(DeeObject *self) {
	return err_seq_not_mutablef(self, "clear()");
}


/************************************************************************/
/* pop()                                                                */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, Dee_ssize_t index) {
	size_t used_index = (size_t)index;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *result;
	if (index < 0) {
		size_t selfsize;
		selfsize = (*seq->tp_size)(self);
		if unlikely(selfsize == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, selfsize);
	}
	result = (*seq->tp_getitem_index)(self, used_index);
	if likely(result) {
		if unlikely((*seq->tp_delitem_index)(self, index))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultPopWithError(DeeObject *self, Dee_ssize_t index) {
	err_seq_not_mutablef(self, "pop(" PRFdSIZ ")", index);
	return NULL;
}


/************************************************************************/
/* remove()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithTSCRemoveAll(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end) {
	return (*DeeType_SeqCache_RequireRemoveAll(Dee_TYPE(self)))(self, item, start, end, 1);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                    size_t start, size_t end) {
	/* TODO:
	 * >> self.removeif(start, end, x -> {
	 * >>     static local didRemove = false;
	 * >>     if (didRemove)
	 * >>         return false;
	 * >>     if (!util.equals(item, x))
	 * >>         return false;
	 * >>     didRemove = true;
	 * >>     return true;
	 * >> });
	 */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                      size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithError(DeeObject *self, DeeObject *item,
                              size_t start, size_t end) {
	return err_seq_not_mutablef(self, "remove(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}


/************************************************************************/
/* remove() (with key)                                                  */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, DeeObject *key) {
	return (*DeeType_SeqCache_RequireRemoveAllWithKey(Dee_TYPE(self)))(self, item, start, end, 1, key);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                           size_t start, size_t end, DeeObject *key) {
	/* TODO:
	 * >> local keyedElem = key(item);
	 * >> self.removeif(start, end, x -> {
	 * >>     static local didRemove = false;
	 * >>     if (didRemove)
	 * >>         return false;
	 * >>     if (!util.equals(keyedElem, key(x)))
	 * >>         return false;
	 * >>     didRemove = true;
	 * >>     return true;
	 * >> });
	 */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                             size_t start, size_t end, DeeObject *key) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithError(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end, DeeObject *key) {
	return err_seq_not_mutablef(self, "remove(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}


/************************************************************************/
/* rremove()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                              size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                       size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithError(DeeObject *self, DeeObject *item,
                               size_t start, size_t end) {
	return err_seq_not_mutablef(self, "rremove(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}


/************************************************************************/
/* rremove() (with key)                                                 */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                     size_t start, size_t end, DeeObject *key) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                              size_t start, size_t end, DeeObject *key) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithError(DeeObject *self, DeeObject *item,
                                      size_t start, size_t end, DeeObject *key) {
	return err_seq_not_mutablef(self, "rremove(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}


/************************************************************************/
/* removeall()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                       size_t start, size_t end, size_t max) {
	/* TODO:
	 * >> self.removeif(start, end, x -> {
	 * >>     static local removeCount = 0;
	 * >>     if (removeCount >= max)
	 * >>         return false;
	 * >>     if (!util.equals(item, x))
	 * >>         return false;
	 * >>     ++removeCount;
	 * >>     return true;
	 * >> });
	 */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)max;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithTSCRemove(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end, size_t max) {
	size_t result = 0;
	Dee_tsc_remove_t tsc_remove;
	tsc_remove = DeeType_SeqCache_RequireRemove(Dee_TYPE(self));
	while (result < max) {
		int temp;
		temp = (*tsc_remove)(self, item, start, end);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                         size_t start, size_t end, size_t max) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)max;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithError(DeeObject *self, DeeObject *item,
                                 size_t start, size_t end, size_t max) {
	return err_seq_not_mutablef(self, "removeall(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end, max);
}


/************************************************************************/
/* removeall() (with key)                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                              size_t start, size_t end, size_t max,
                                              DeeObject *key) {
	/* TODO:
	 * >> local keyedElem = key(item);
	 * >> self.removeif(start, end, x -> {
	 * >>     static local removeCount = 0;
	 * >>     if (removeCount >= max)
	 * >>         return false;
	 * >>     if (!util.equals(keyedElem, key(x)))
	 * >>         return false;
	 * >>     ++removeCount;
	 * >>     return true;
	 * >> });
	 */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)max;
	(void)key;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, size_t max,
                                                   DeeObject *key) {
	size_t result = 0;
	Dee_tsc_remove_with_key_t tsc_remove_with_key;
	tsc_remove_with_key = DeeType_SeqCache_RequireRemoveWithKey(Dee_TYPE(self));
	while (result < max) {
		int temp;
		temp = (*tsc_remove_with_key)(self, item, start, end, key);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                size_t start, size_t end, size_t max,
                                                                DeeObject *key) {
	/* TODO */
	(void)self;
	(void)item;
	(void)start;
	(void)end;
	(void)max;
	(void)key;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithError(DeeObject *self, DeeObject *item,
                                        size_t start, size_t end, size_t max,
                                        DeeObject *key) {
	return err_seq_not_mutablef(self, "removeall(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ", %r)",
	                            item, start, end, max, key);
}


/************************************************************************/
/* removeif()                                                           */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *should,
                                              size_t start, size_t end, size_t max) {
	/* TODO:
	 * >> class Dummy { operator == (other) -> other; };
	 * >> self.removeall(start, end, max, Dummy(), key: x -> {
	 * >>     return x is Dummy ? x : should(x);
	 * >> });
	 */
	(void)self;
	(void)should;
	(void)start;
	(void)end;
	(void)max;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *should,
                                                        size_t start, size_t end, size_t max) {
	/* TODO */
	(void)self;
	(void)should;
	(void)start;
	(void)end;
	(void)max;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithError(DeeObject *self, DeeObject *should,
                                size_t start, size_t end, size_t max) {
	return err_seq_not_mutablef(self, "removeif(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")",
	                            should, start, end, max);
}


/************************************************************************/
/* resize()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	oldsize = (*seq->tp_size)(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*seq->tp_setrange_index)(self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*seq->tp_delrange_index)(self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultResizeWithSizeAndSetRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	oldsize = (*seq->tp_size)(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*seq->tp_setrange_index)(self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*seq->tp_setrange_index)(self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize, Dee_EmptySeq);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultResizeWithSizeAndTSCEraseAndTSCExtend(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize;
	oldsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*DeeType_SeqCache_RequireExtend(Dee_TYPE(self)))(self, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, newsize, oldsize - newsize);
	}
	return 0;
err:
	return -1;
}


/************************************************************************/
/* fill()                                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultFillWithSizeAndSetRangeIndex(DeeObject *self, size_t start,
                                           size_t end, DeeObject *filler) {
	int result;
	size_t selfsize;
	DREF DeeObject *repeat;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	ASSERT(seq == Dee_TYPE(self)->tp_seq);
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	repeat = DeeSeq_RepeatItem(filler, end - start);
	if unlikely(!repeat)
		goto err;
	result = (*seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, repeat);
	Dee_Decref(repeat);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex(DeeObject *self, size_t start,
                                                    size_t end, DeeObject *filler) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	(void)filler;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultFillWithError(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	return err_seq_not_mutablef(self, "fill(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, filler);
}


/************************************************************************/
/* reverse()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithTSCReversedAndSetRangeIndex(DeeObject *self, size_t start, size_t end) {
	int result;
	DREF DeeObject *reversed;
	reversed = (*DeeType_SeqCache_RequireReversed(Dee_TYPE(self)))(self, start, end);
	if unlikely(!reversed)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, reversed);
	Dee_Decref(reversed);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndexAndDelItemIndex(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeObject *lo_elem;
	DREF DeeObject *hi_elem;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	ASSERT(seq == Dee_TYPE(self)->tp_seq);
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = (*seq->tp_getitem_index)(self, start);
		if unlikely(!lo_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		hi_elem = (*seq->tp_getitem_index)(self, end - 1);
		if unlikely(!hi_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err_lo_elem;
		if (hi_elem) {
			if unlikely((*seq->tp_setitem_index)(self, start, hi_elem))
				goto err_lo_elem_hi_elem;
			Dee_Decref(hi_elem);
		} else {
			if unlikely((*seq->tp_delitem_index)(self, start))
				goto err_lo_elem;
		}
		if (lo_elem) {
			if unlikely((*seq->tp_setitem_index)(self, end - 1, lo_elem))
				goto err_lo_elem;
			Dee_Decref(lo_elem);
		} else {
			if unlikely((*seq->tp_delitem_index)(self, end - 1))
				goto err;
		}
		++start;
		--end;
	}
	return 0;
err_lo_elem_hi_elem:
	Dee_XDecref(hi_elem);
err_lo_elem:
	Dee_XDecref(lo_elem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeObject *lo_elem;
	DREF DeeObject *hi_elem;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	ASSERT(seq == Dee_TYPE(self)->tp_seq);
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = (*seq->tp_getitem_index)(self, start);
		if unlikely(!lo_elem)
			goto err;
		hi_elem = (*seq->tp_getitem_index)(self, end - 1);
		if unlikely(!hi_elem)
			goto err_lo_elem;
		if unlikely((*seq->tp_setitem_index)(self, start, hi_elem))
			goto err_lo_elem_hi_elem;
		Dee_Decref(hi_elem);
		if unlikely((*seq->tp_setitem_index)(self, end - 1, lo_elem))
			goto err_lo_elem;
		Dee_Decref(lo_elem);
		++start;
		--end;
	}
	return 0;
err_lo_elem_hi_elem:
	Dee_Decref(hi_elem);
err_lo_elem:
	Dee_Decref(lo_elem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithError(DeeObject *self, size_t start, size_t end) {
	return err_seq_not_mutablef(self, "reverse(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}


/************************************************************************/
/* reversed()                                                           */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex(DeeObject *self, size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithProxyCopyDefault(DeeObject *self, size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}


/************************************************************************/
/* sort()                                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithTSCSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end) {
	int result;
	DREF DeeObject *sorted;
	sorted = (*DeeType_SeqCache_RequireSorted(Dee_TYPE(self)))(self, start, end);
	if unlikely(!sorted)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, sorted);
	Dee_Decref(sorted);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithError(DeeObject *self, size_t start, size_t end) {
	return err_seq_not_mutablef(self, "sort(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}


/************************************************************************/
/* sort() (with key)                                                    */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultSortWithKeyWithTSCSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *sorted;
	sorted = (*DeeType_SeqCache_RequireSortedWithKey(Dee_TYPE(self)))(self, start, end, key);
	if unlikely(!sorted)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, sorted);
	Dee_Decref(sorted);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithKeyWithError(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_not_mutablef(self, "sort(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}


/************************************************************************/
/* sorted()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithProxyCopyDefault(DeeObject *self, size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}


/************************************************************************/
/* sorted() (with key)                                                  */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithKeyWithProxyCopyDefault(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	(void)key;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}




/************************************************************************/
/* Deemon user-code wrappers                                            */
/************************************************************************/

PRIVATE DEFINE_KWLIST(kwlist_start_count, { K(start), K(count), KEND });
PRIVATE DEFINE_KWLIST(kwlist_index_item, { K(index), K(item), KEND });
PRIVATE DEFINE_KWLIST(kwlist_index_items, { K(index), K(items), KEND });
PRIVATE DEFINE_KWLIST(kwlist_index_value, { K(index), K(value), KEND });
PRIVATE DEFINE_KWLIST(kwlist_index, { K(index), KEND });
PRIVATE DEFINE_KWLIST(kwlist_item_start_end_key, { K(item), K(start), K(end), K(key), KEND });
PRIVATE DEFINE_KWLIST(kwlist_item_start_end_max_key, { K(item), K(start), K(end), K(max), K(key), KEND });
PRIVATE DEFINE_KWLIST(kwlist_should_start_end_max, { K(should), K(start), K(end), K(max), KEND });
PRIVATE DEFINE_KWLIST(kwlist_size_filler, { K(size), K(filler), KEND });
PRIVATE DEFINE_KWLIST(kwlist_start_end_filler, { K(start), K(end), K(filler), KEND });
PRIVATE DEFINE_KWLIST(kwlist_start_end, { K(start), K(end), KEND });
PRIVATE DEFINE_KWLIST(kwlist_start_end_key, { K(start), K(end), K(key), KEND });

/* Generic sequence mutable function pointers. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_erase(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_count,
	                    UNPuSIZ "|" UNPuSIZ ":erase",
	                    &start, &count))
		goto err;
	if unlikely(new_DeeSeq_Erase(self, start, count))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_insert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_index_item,
	                    UNPuSIZ "o:insert",
	                    &index, &item))
		goto err;
	if unlikely(new_DeeSeq_Insert(self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_insertall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_index_items,
	                    UNPuSIZ "o:insertall",
	                    &index, &items))
		goto err;
	if unlikely(new_DeeSeq_InsertAll(self, index, items))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_pushfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:pushfront", &item))
		goto err;
	if unlikely(new_DeeSeq_PushFront(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_append(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:append", &item))
		goto err;
	if unlikely(new_DeeSeq_Append(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_extend(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:extend", &items))
		goto err;
	if unlikely(new_DeeSeq_Extend(self, items))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_xchitem(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *value;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_index_value,
	                    UNPuSIZ "o:xchitem",
	                    &index, &value))
		goto err;
	if unlikely(new_DeeSeq_XchItemIndex(self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_clear(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	if unlikely(new_DeeSeq_Clear(self))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_pop(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t index;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_index,
	                    UNPdSIZ ":pop", &index))
		goto err;
	return new_DeeSeq_Pop(self, index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_remove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:remove",
	                    &item, &start, &end, &key))
		goto err;
	result = DeeNone_Check(key)
	         ? new_DeeSeq_Remove(self, item, start, end)
	         : new_DeeSeq_RemoveWithKey(self, item, start, end, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_rremove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rremove",
	                    &item, &start, &end, &key))
		goto err;
	result = DeeNone_Check(key)
	         ? new_DeeSeq_RRemove(self, item, start, end)
	         : new_DeeSeq_RRemoveWithKey(self, item, start, end, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_removeall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_item_start_end_max_key,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ "o:removeall",
	                    &item, &start, &end, &max, &key))
		goto err;
	result = DeeNone_Check(key)
	         ? new_DeeSeq_RemoveAll(self, item, start, end, max)
	         : new_DeeSeq_RemoveAllWithKey(self, item, start, end, max, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_removeif(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *should;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_should_start_end_max,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ "o:removeall",
	                    &should, &start, &end, &max))
		goto err;
	result = new_DeeSeq_RemoveIf(self, should, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_resize(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_size_filler,
	                    UNPuSIZ "|o:resize", &size, &filler))
		goto err;
	if unlikely(new_DeeSeq_Resize(self, size, filler))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_fill(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end_filler,
	                    "|" UNPuSIZ UNPuSIZ "o:fill",
	                    &start, &end, &filler))
		goto err;
	if unlikely(new_DeeSeq_Fill(self, start, end, filler))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_reverse(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reverse",
	                    &start, &end))
		goto err;
	if unlikely(new_DeeSeq_Reverse(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_reversed(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reversed",
	                    &start, &end))
		goto err;
	return new_DeeSeq_Reversed(self, start, end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_sort(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sort",
	                    &start, &end, &key))
		goto err;
	if unlikely(DeeNone_Check(key)
	            ? new_DeeSeq_Sort(self, start, end)
	            : new_DeeSeq_SortWithKey(self, start, end, key))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_sorted(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sorted",
	                    &start, &end, &key))
		goto err;
	return DeeNone_Check(key)
	       ? new_DeeSeq_Sorted(self, start, end)
	       : new_DeeSeq_SortedWithKey(self, start, end, key);
err:
	return NULL;
}


DECL_END

/* Define mutable sequence function implementation selectors */
#ifndef __INTELLISENSE__
#define DEFINE_DeeType_SeqCache_RequireErase
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireInsert
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireInsertAll
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequirePushFront
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAppend
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireExtend
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireXchItemIndex
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireClear
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequirePop
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemove
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveWithKey
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRRemove
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRRemoveWithKey
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveAll
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveIf
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireResize
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireFill
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReverse
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReversed
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSort
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSortWithKey
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSorted
#include "default-api-mutable-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSortedWithKey
#include "default-api-mutable-require-impl.c.inl"
#endif /* !__INTELLISENSE__ */


/* Define attribute proxy implementations */
#ifndef __INTELLISENSE__
#define DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#include "default-api-mutable-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction
#include "default-api-mutable-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod
#include "default-api-mutable-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod
#include "default-api-mutable-attrproxy-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C */
