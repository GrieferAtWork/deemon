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
#include <deemon/callable.h>
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
#include "default-reversed.h"
#include "repeat.h"
#include "sort.h"

/**/
#include "../../runtime/kwlist.h"
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


PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_erase_t DCALL DeeType_SeqCache_RequireErase_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_insert_t DCALL DeeType_SeqCache_RequireInsert_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_insertall_t DCALL DeeType_SeqCache_RequireInsertAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_pushfront_t DCALL DeeType_SeqCache_RequirePushFront_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_append_t DCALL DeeType_SeqCache_RequireAppend_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_extend_t DCALL DeeType_SeqCache_RequireExtend_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_xchitem_index_t DCALL DeeType_SeqCache_RequireXchItemIndex_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_clear_t DCALL DeeType_SeqCache_RequireClear_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_pop_t DCALL DeeType_SeqCache_RequirePop_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_remove_t DCALL DeeType_SeqCache_RequireRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_remove_with_key_t DCALL DeeType_SeqCache_RequireRemoveWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rremove_t DCALL DeeType_SeqCache_RequireRRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rremove_with_key_t DCALL DeeType_SeqCache_RequireRRemoveWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_removeall_t DCALL DeeType_SeqCache_RequireRemoveAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_removeall_with_key_t DCALL DeeType_SeqCache_RequireRemoveAllWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_removeif_t DCALL DeeType_SeqCache_RequireRemoveIf_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_resize_t DCALL DeeType_SeqCache_RequireResize_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_fill_t DCALL DeeType_SeqCache_RequireFill_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reverse_t DCALL DeeType_SeqCache_RequireReverse_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reversed_t DCALL DeeType_SeqCache_RequireReversed_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sort_t DCALL DeeType_SeqCache_RequireSort_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sort_with_key_t DCALL DeeType_SeqCache_RequireSortWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sorted_t DCALL DeeType_SeqCache_RequireSorted_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sorted_with_key_t DCALL DeeType_SeqCache_RequireSortedWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);

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
DeeSeq_DefaultEraseWithDelRangeIndex(DeeObject *self, size_t index, size_t count) {
	struct type_seq *seq;
	size_t end_index;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	seq = Dee_TYPE(self)->tp_seq;
	return (*seq->tp_delrange_index)(self, (Dee_ssize_t)index, (Dee_ssize_t)end_index);
err_overflow:
	return err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultEraseWithPop(DeeObject *self, size_t index, size_t count) {
	size_t end_index;
	Dee_tsc_pop_t tsc_pop;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	tsc_pop = DeeType_SeqCache_RequirePop(Dee_TYPE(self));
	while (end_index > index) {
		--end_index;
		if unlikely((*tsc_pop)(self, (Dee_ssize_t)end_index))
			goto err;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return 0;
err_overflow:
	err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultEraseWithError(DeeObject *self, size_t index, size_t count) {
	return err_seq_not_mutablef(self, "erase(%" PRFuSIZ ", %" PRFuSIZ ")", index, count);
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
DeeSeq_DefaultPopWithSizeAndGetItemIndexAndTSCErase(DeeObject *self, Dee_ssize_t index) {
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
		if unlikely((*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, index, 1))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

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
	size_t result;
	result = (*DeeType_SeqCache_RequireRemoveAll(Dee_TYPE(self)))(self, item, start, end, 1);
	if unlikely(result == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *srwrip_item; /* [1..1][const] Item to remove */
} SeqRemoveWithRemoveIfPredicate;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *srwripwk_item; /* [1..1][const] Keyed item to remove */
	DREF DeeObject *srwripwk_key;  /* [1..1][const] Key to use during compare */
} SeqRemoveWithRemoveIfPredicateWithKey;

PRIVATE WUNUSED NONNULL((1)) int DCALL
srwrip_init(SeqRemoveWithRemoveIfPredicate *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicate", &self->srwrip_item))
		goto err;
	Dee_Incref(self->srwrip_item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
srwripwk_init(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_SeqRemoveWithRemoveIfPredicateWithKey",
	                  &self->srwripwk_item, &self->srwripwk_key))
		goto err;
	Dee_Incref(self->srwripwk_item);
	Dee_Incref(self->srwripwk_key);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
srwrip_fini(SeqRemoveWithRemoveIfPredicate *__restrict self) {
	Dee_Decref(self->srwrip_item);
}

PRIVATE NONNULL((1, 2)) void DCALL
srwrip_visit(SeqRemoveWithRemoveIfPredicate *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->srwrip_item);
}

PRIVATE NONNULL((1)) void DCALL
srwripwk_fini(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self) {
	Dee_Decref(self->srwripwk_item);
	Dee_Decref(self->srwripwk_key);
}

PRIVATE NONNULL((1, 2)) void DCALL
srwripwk_visit(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->srwripwk_item);
	Dee_Visit(self->srwripwk_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
srwrip_call(SeqRemoveWithRemoveIfPredicate *self, size_t argc, DeeObject *const *argv) {
	int equals;
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicate", &item))
		goto err;
	equals = DeeObject_TryCompareEq(self->srwrip_item, item);
	if unlikely(equals == Dee_COMPARE_ERR)
		goto err;
	return_bool_(equals == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
srwripwk_call(SeqRemoveWithRemoveIfPredicateWithKey *self, size_t argc, DeeObject *const *argv) {
	int equals;
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicateWithKey", &item))
		goto err;
	equals = DeeObject_TryCompareKeyEq(self->srwripwk_item, item, self->srwripwk_key);
	if unlikely(equals == Dee_COMPARE_ERR)
		goto err;
	return_bool_(equals == 0);
err:
	return NULL;
}

STATIC_ASSERT(offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_item) ==
              offsetof(SeqRemoveWithRemoveIfPredicate, srwrip_item));
#define srwrip_members (srwripwk_members + 1)
PRIVATE struct type_member tpconst srwripwk_members[] = {
	TYPE_MEMBER_FIELD("__item__", STRUCT_OBJECT, offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_item)),
	TYPE_MEMBER_FIELD("__key__", STRUCT_OBJECT, offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_key)),
	TYPE_MEMBER_END
};

PRIVATE DeeTypeObject SeqRemoveWithRemoveIfPredicate_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveWithRemoveIfPredicate",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&srwrip_init,
				TYPE_FIXED_ALLOCATOR(SeqRemoveWithRemoveIfPredicate)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&srwrip_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&srwrip_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&srwrip_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ srwrip_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

PRIVATE DeeTypeObject SeqRemoveWithRemoveIfPredicateWithKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveWithRemoveIfPredicateWithKey",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&srwripwk_init,
				TYPE_FIXED_ALLOCATOR(SeqRemoveWithRemoveIfPredicateWithKey)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&srwripwk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&srwripwk_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&srwripwk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ srwripwk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};




INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                    size_t start, size_t end) {
	/* >> return !!self.removeif(x -> deemon.equals(item, x), start, end, 1); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicate *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicate);
	if unlikely(!pred)
		goto err;
	Dee_Incref(item);
	pred->srwrip_item = item;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicate_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, 1);
	Dee_Decref_likely(pred);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}



struct default_remove_with_enumerate_index_and_delitem_index_data {
	DeeObject *drweiadiid_self; /* [1..1] The sequence from which to remove the object. */
	DeeObject *drweiadiid_item; /* [1..1] The object to remove. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_remove_with_enumerate_index_and_delitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int equal;
	struct default_remove_with_enumerate_index_and_delitem_index_data *data;
	data = (struct default_remove_with_enumerate_index_and_delitem_index_data *)arg;
	if (!value)
		return 0;
	equal = DeeObject_TryCompareEq(data->drweiadiid_item, value);
	if unlikely(equal == Dee_COMPARE_ERR)
		goto err;
	if (equal != 0)
		return 0;
	if unlikely((*Dee_TYPE(data->drweiadiid_self)->tp_seq->tp_delitem_index)(data->drweiadiid_self, index))
		goto err;
	return -2;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                      size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_enumerate_index)(self, &default_remove_with_enumerate_index_and_delitem_index_cb,
	                                                               &data, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
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
	size_t result;
	result = (*DeeType_SeqCache_RequireRemoveAllWithKey(Dee_TYPE(self)))(self, item, start, end, 1, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                           size_t start, size_t end, DeeObject *key) {
	/* >> local keyedElem = key(item);
	 * >> return !!self.removeif(x -> deemon.equals(keyedElem, key(x)), start, end, 1); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicateWithKey *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicateWithKey);
	if unlikely(!pred)
		goto err;
	pred->srwripwk_item = DeeObject_Call(key, 1, &item);
	if unlikely(!pred->srwripwk_item)
		goto err_pred;
	Dee_Incref(key);
	pred->srwripwk_key = key;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicateWithKey_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, 1);
	Dee_Decref_likely(pred);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err_pred:
	DeeObject_FREE(pred);
err:
	return -1;
}

struct default_remove_with_key_with_enumerate_index_and_delitem_index_data {
	DeeObject *drwkweiadiid_self; /* [1..1] The sequence from which to remove the object. */
	DeeObject *drwkweiadiid_item; /* [1..1] The object to remove (already keyed). */
	DeeObject *drwkweiadiid_key;  /* [1..1] The key used for object compare. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_remove_with_key_with_enumerate_index_and_delitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int equal;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data *data;
	data = (struct default_remove_with_key_with_enumerate_index_and_delitem_index_data *)arg;
	if (!value)
		return 0;
	equal = DeeObject_TryCompareKeyEq(data->drwkweiadiid_item, value, data->drwkweiadiid_key);
	if unlikely(equal == Dee_COMPARE_ERR)
		goto err;
	if (equal != 0)
		return 0;
	if unlikely((*Dee_TYPE(data->drwkweiadiid_self)->tp_seq->tp_delitem_index)(data->drwkweiadiid_self, index))
		goto err;
	return -2;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                             size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_enumerate_index)(self, &default_remove_with_key_with_enumerate_index_and_delitem_index_cb,
	                                                               &data, start, end);
	Dee_Decref(data.drwkweiadiid_item);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
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
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	Dee_tsc_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	tsc_enumerate_index_reverse = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(tsc_enumerate_index_reverse);
	foreach_status = (*tsc_enumerate_index_reverse)(self, &default_remove_with_enumerate_index_and_delitem_index_cb,
	                                                &data, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                       size_t start, size_t end) {
	size_t index;
	index = DeeSeq_DefaultRFindWithEnumerateIndex(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		return 0;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index))
		goto err;
	return 1;
err:
	return -1;
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
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	Dee_tsc_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	tsc_enumerate_index_reverse = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(tsc_enumerate_index_reverse);
	foreach_status = (*tsc_enumerate_index_reverse)(self, &default_remove_with_key_with_enumerate_index_and_delitem_index_cb,
	                                                &data, start, end);
	Dee_Decref(data.drwkweiadiid_item);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                              size_t start, size_t end, DeeObject *key) {
	size_t index;
	index = DeeSeq_DefaultRFindWithKeyWithEnumerateIndex(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		return 0;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index))
		goto err;
	return 1;
err:
	return -1;
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
	/* >> return self.removeif(x -> deemon.equals(item, x), start, end, max); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicate *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicate);
	if unlikely(!pred)
		goto err;
	Dee_Incref(item);
	pred->srwrip_item = item;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicate_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err:
	return (size_t)-1;
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
DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                              size_t start, size_t end, size_t max) {
	int sequence_size_changes_after_delitem = -1;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize, result = 0;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while (start < end) {
		DREF DeeObject *elem;
		elem = (*seq->tp_trygetitem_index)(self, start);
		if unlikely(!elem)
			goto err;
		if (elem != ITER_DONE) {
			int equal;
			equal = DeeObject_TryCompareEq(item, elem);
			Dee_Decref(elem);
			if unlikely(equal == Dee_COMPARE_ERR)
				goto err;
			if (equal == 0) {
				/* Found one! (delete it) */
				if unlikely((*seq->tp_delitem_index)(self, start))
					goto err;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*seq->tp_size)(self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithError(DeeObject *self, DeeObject *item,
                                 size_t start, size_t end, size_t max) {
	return err_seq_not_mutablef(self, "removeall(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")",
	                            item, start, end, max);
}


/************************************************************************/
/* removeall() (with key)                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                              size_t start, size_t end, size_t max,
                                              DeeObject *key) {
	/* >> local keyedElem = key(item);
	 * >> return self.removeif(x -> deemon.equals(keyedElem, key(x)), start, end, max); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicateWithKey *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicateWithKey);
	if unlikely(!pred)
		goto err;
	pred->srwripwk_item = DeeObject_Call(key, 1, &item);
	if unlikely(!pred->srwripwk_item)
		goto err_pred;
	Dee_Incref(key);
	pred->srwripwk_key = key;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicateWithKey_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err_pred:
	DeeObject_FREE(pred);
err:
	return (size_t)-1;
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
DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                     size_t start, size_t end, size_t max,
                                                                     DeeObject *key) {
	int sequence_size_changes_after_delitem = -1;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize, result = 0;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start >= end)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	do {
		DREF DeeObject *elem;
		elem = (*seq->tp_trygetitem_index)(self, start);
		if unlikely(!elem)
			goto err_item;
		if (elem != ITER_DONE) {
			int equal;
			equal = DeeObject_TryCompareKeyEq(item, elem, key);
			Dee_Decref(elem);
			if unlikely(equal == Dee_COMPARE_ERR)
				goto err_item;
			if (equal == 0) {
				/* Found one! (delete it) */
				if unlikely((*seq->tp_delitem_index)(self, start))
					goto err_item;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*seq->tp_size)(self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err_item;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err_item;
	} while (start < end);
	Dee_Decref(item);
	return result;
err_item:
	Dee_Decref(item);
err:
	return (size_t)-1;
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
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seq_removeif_with_removeall_item_compare_eq(DeeObject *self, DeeObject *should_result) {
	int result;
	(void)self;
	result = DeeObject_Bool(should_result);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_removeif_with_removeall_item_eq(DeeObject *self, DeeObject *should_result) {
	(void)self;
	return_reference_(should_result);
}

PRIVATE struct type_cmp seq_removeif_with_removeall_item_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ &seq_removeif_with_removeall_item_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &seq_removeif_with_removeall_item_compare_eq,
	/* .tp_eq            = */ &seq_removeif_with_removeall_item_eq,
};

PRIVATE DeeTypeObject SeqRemoveIfWithRemoveAllItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveIfWithRemoveAllItem",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &seq_removeif_with_removeall_item_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

PRIVATE DeeObject SeqRemoveIfWithRemoveAllItem_DummyInstance = {
	OBJECT_HEAD_INIT(&SeqRemoveIfWithRemoveAllItem_Type)
};

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *sriwrak_should; /* [1..1] Predicate to determine if an element should be removed. */
} SeqRemoveIfWithRemoveAllKey;

PRIVATE WUNUSED NONNULL((1)) int DCALL
seq_removeif_with_removeall_key_init(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                     size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveIfWithRemoveAllKey", &self->sriwrak_should))
		goto err;
	Dee_Incref(self->sriwrak_should);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
seq_removeif_with_removeall_key_fini(SeqRemoveIfWithRemoveAllKey *__restrict self) {
	Dee_Decref(self->sriwrak_should);
}

PRIVATE NONNULL((1, 2)) void DCALL
seq_removeif_with_removeall_key_visit(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                      dvisit_t proc, void *arg) {
	Dee_Visit(self->sriwrak_should);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_removeif_with_removeall_key_printrepr(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                          Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "rt.SeqRemoveIfWithRemoveAllKey(%r)", self->sriwrak_should);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_removeif_with_removeall_key_call(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                     size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveIfWithRemoveAllKey", &item))
		goto err;
	if (item == &SeqRemoveIfWithRemoveAllItem_DummyInstance)
		return_reference_(item);
	return DeeObject_Call(self->sriwrak_should, argc, argv);
err:
	return NULL;
}

PRIVATE DeeTypeObject SeqRemoveIfWithRemoveAllKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveIfWithRemoveAllKey",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&seq_removeif_with_removeall_key_init,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *))&seq_removeif_with_removeall_key_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&seq_removeif_with_removeall_key_printrepr,
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&seq_removeif_with_removeall_key_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *, dvisit_t, void *))&seq_removeif_with_removeall_key_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};



INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *should,
                                              size_t start, size_t end, size_t max) {
	/* >> global final class SeqRemoveIfWithRemoveAllItem { operator == (other) -> other; };
	 * >> global final SeqRemoveIfWithRemoveAllItem_DummyInstance = SeqRemoveIfWithRemoveAllItem();
	 * >>
	 * >> class SeqRemoveIfWithRemoveAllItem { operator == (other) -> other; };
	 * >> return self.removeall(SeqRemoveIfWithRemoveAllItem_DummyInstance, start, end, max, key: x -> {
	 * >>     return x === SeqRemoveIfWithRemoveAllItem_DummyInstance ? x : should(x);
	 * >> }); */
	size_t result;
	DREF SeqRemoveIfWithRemoveAllKey *key;
	key = DeeObject_MALLOC(SeqRemoveIfWithRemoveAllKey);
	if unlikely(!key)
		goto err;
	Dee_Incref(should);
	key->sriwrak_should = should;
	DeeObject_Init(key, &SeqRemoveIfWithRemoveAllKey_Type);
	result = (*DeeType_SeqCache_RequireRemoveAllWithKey(Dee_TYPE(self)))(self, &SeqRemoveIfWithRemoveAllItem_DummyInstance,
	                                                                     start, end, max, (DeeObject *)key);
	Dee_Decref_likely(key);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *should,
                                                             size_t start, size_t end, size_t max) {
	int sequence_size_changes_after_delitem = -1;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize, result = 0;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while (start < end) {
		DREF DeeObject *elem;
		elem = (*seq->tp_trygetitem_index)(self, start);
		if unlikely(!elem)
			goto err;
		if (elem != ITER_DONE) {
			int should_remove;
			DREF DeeObject *pred_result;
			pred_result = DeeObject_Call(should, 1, &elem);
			Dee_Decref(elem);
			if unlikely(!pred_result)
				goto err;
			should_remove = DeeObject_BoolInherited(pred_result);
			if unlikely(should_remove < 0)
				goto err;
			if (should_remove) {
				/* Delete this one */
				if unlikely((*seq->tp_delitem_index)(self, start))
					goto err;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*seq->tp_size)(self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
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

struct default_fill_with_enumerate_index_and_setitem_index_data {
	DeeObject *dfweiasiid_seq;    /* [1..1] Sequence whose items to set. */
	DeeObject *dfweiasiid_filler; /* [1..1] Value to assign to indices. */
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *dfweiasiid_setitem_index)(DeeObject *self, size_t index, DeeObject *value);
};

PRIVATE WUNUSED Dee_ssize_t DCALL
default_fill_with_enumerate_index_and_setitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	struct default_fill_with_enumerate_index_and_setitem_index_data *data;
	(void)value;
	data = (struct default_fill_with_enumerate_index_and_setitem_index_data *)arg;
	return (*data->dfweiasiid_setitem_index)(data->dfweiasiid_seq, index, data->dfweiasiid_filler);
}


INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex(DeeObject *self, size_t start,
                                                    size_t end, DeeObject *filler) {
	struct default_fill_with_enumerate_index_and_setitem_index_data data;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	data.dfweiasiid_seq    = self;
	data.dfweiasiid_filler = filler;
	data.dfweiasiid_setitem_index = seq->tp_setitem_index;
	return (int)(*seq->tp_enumerate_index)(self, &default_fill_with_enumerate_index_and_setitem_index_cb,
	                                       &data, start, end);
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
DeeSeq_DefaultReversedWithProxySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = seq->tp_getitem_index_fast;
	Dee_Incref(self);
	result->drwgii_seq  = self;
	result->drwgii_max  = end - 1; /* It's ok if this underflows */
	result->drwgii_size = end - start;
	DeeObject_Init(result, &DefaultReversed_WithGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = seq->tp_getitem_index;
	Dee_Incref(self);
	result->drwgii_seq  = self;
	result->drwgii_max  = end - 1; /* It's ok if this underflows */
	result->drwgii_size = end - start;
	DeeObject_Init(result, &DefaultReversed_WithGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithProxySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = seq->tp_trygetitem_index;
	Dee_Incref(self);
	result->drwgii_seq  = self;
	result->drwgii_max  = end - 1; /* It's ok if this underflows */
	result->drwgii_size = end - start;
	DeeObject_Init(result, &DefaultReversed_WithTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

struct foreach_subrange_as_tuple_data {
	DREF DeeTupleObject *fesrat_result;  /* [1..1] The tuple being constructed. */
	size_t               fesrat_used;    /* Used # of elements of `fesrat_result' */
	size_t               fesrat_maxsize; /* Max value for `fesrat_used' */
	size_t               fesrat_start;   /* # of elements that still need to be skipped. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
foreach_subrange_as_tuple_cb(void *arg, DeeObject *elem) {
	struct foreach_subrange_as_tuple_data *data;
	data = (struct foreach_subrange_as_tuple_data *)arg;
	if (data->fesrat_start) {
		--data->fesrat_start; /* Skip leading. */
		return 0;
	}
	if (data->fesrat_used >= DeeTuple_SIZE(data->fesrat_result)) {
		DREF DeeTupleObject *new_tuple;
		size_t new_size = DeeTuple_SIZE(data->fesrat_result) * 2;
		if (new_size < 16)
			new_size = 16;
		new_tuple = DeeTuple_TryResizeUninitialized(data->fesrat_result, new_size);
		if unlikely(!new_tuple) {
			new_size  = data->fesrat_used + 1;
			new_tuple = DeeTuple_ResizeUninitialized(data->fesrat_result, new_size);
			if unlikely(!new_tuple)
				goto err;
		}
		data->fesrat_result = new_tuple;
	}
	Dee_Incref(elem);
	data->fesrat_result->t_elem[data->fesrat_used++] = elem;
	if (data->fesrat_used >= data->fesrat_maxsize)
		return -2; /* Stop enumeration */
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetForeachSubRangeAsTuple(DeeObject *self, size_t start, size_t end) {
	size_t fast_size;
	Dee_ssize_t foreach_status;
	struct foreach_subrange_as_tuple_data data;
	if unlikely(start >= end)
		return_empty_tuple;
	fast_size = (*Dee_TYPE(self)->tp_seq->tp_size_fast)(self);
	if (fast_size != (size_t)-1) {
		data.fesrat_result = DeeTuple_NewUninitialized(fast_size);
		if unlikely(!data.fesrat_result)
			goto err;
	} else {
		Dee_Incref(Dee_EmptyTuple);
		data.fesrat_result = (DREF DeeTupleObject *)Dee_EmptyTuple;
	}
	data.fesrat_used    = 0;
	data.fesrat_maxsize = end - start;
	data.fesrat_start   = start;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &foreach_subrange_as_tuple_cb, &data);
	ASSERT(foreach_status == 0 || foreach_status == -1);
	if unlikely(foreach_status < 0)
		goto err_r;
	data.fesrat_result = DeeTuple_TruncateUninitialized(data.fesrat_result, data.fesrat_used);
	return (DREF DeeObject *)data.fesrat_result;
err_r:
	Dee_Decrefv(data.fesrat_result->t_elem, data.fesrat_used);
	DeeTuple_FreeUninitialized(data.fesrat_result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if likely(result) {
		DREF DeeObject **lo, **hi;
		lo = DeeTuple_ELEM(result);
		hi = lo + DeeTuple_SIZE(result);
		while (lo < hi) {
			DeeObject *temp;
			temp  = *lo;
			*lo++ = *--hi;
			*hi   = temp;
		}
	}
	return result;
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
DeeSeq_DefaultSortWithKeyWithTSCSortedAndSetRangeIndex(DeeObject *self, size_t start,
                                                       size_t end, DeeObject *key) {
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
DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start,
                                                                size_t end, DeeObject *key) {
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
DeeSeq_DefaultSortedWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFast(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                        self, start, seq->tp_getitem_index_fast))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */

	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndex(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                       self, start, seq->tp_trygetitem_index))
		goto err_r;
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end) {
	DREF DeeTupleObject *base, *result;
	base = (DREF DeeTupleObject *)DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if unlikely(!base)
		goto err;
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(base));
	if unlikely(!result)
		goto err_base;
	if unlikely(DeeSeq_SortVector(DeeTuple_SIZE(result),
	                              DeeTuple_ELEM(result),
	                              DeeTuple_ELEM(base)))
		goto err_base_r;
	DeeTuple_FreeUninitialized(base);
	return (DREF DeeObject *)result;
err_base_r:
	DeeTuple_FreeUninitialized(result);
err_base:
	Dee_Decref(base);
err:
	return NULL;
}


/************************************************************************/
/* sorted() (with key)                                                  */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithKeyWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFastWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                               self, start, seq->tp_getitem_index_fast, key))
		goto err_r;
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithKeyWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndexWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                              self, start, seq->tp_trygetitem_index, key))
		goto err_r;
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *base;
	DREF DeeTupleObject *result;
	base = DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if unlikely(!base)
		goto err;
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(base));
	if unlikely(!result)
		goto err_base;
	if unlikely(DeeSeq_SortVectorWithKey(DeeTuple_SIZE(result),
	                                     DeeTuple_ELEM(result),
	                                     DeeTuple_ELEM(base),
	                                     key))
		goto err_base_r;
	return (DREF DeeObject *)result;
err_base_r:
	DeeTuple_FreeUninitialized(result);
err_base:
	Dee_Decref(base);
err:
	return NULL;
}




/************************************************************************/
/* Deemon user-code wrappers                                            */
/************************************************************************/

/* Generic sequence mutable function pointers. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_erase(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_count,
	                    UNPuSIZ "|" UNPuSIZ ":erase",
	                    &index, &count))
		goto err;
	if unlikely(new_DeeSeq_Erase(self, index, count))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_insert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_item,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_items,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_value,
	                    UNPuSIZ "o:xchitem", &index, &value))
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
	Dee_ssize_t index = -1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index,
	                    "|" UNPdSIZ ":pop", &index))
		goto err;
	return new_DeeSeq_Pop(self, index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	return new_DeeSeq_Pop(self, 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
	return new_DeeSeq_Pop(self, -1);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_remove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_max_key,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__should_start_end_max,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__size_filler,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_filler,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
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
