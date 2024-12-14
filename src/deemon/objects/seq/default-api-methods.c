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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_METHODS_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_METHODS_C 1

#include "default-api.h"
/**/

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/callable.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/map.h>
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
#include "default-iterators.h"
#include "default-map-proxy.h"
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

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

DECL_BEGIN

/* Mutable sequence functions */
PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_api_vunsupportedf(char const *api, DeeObject *self, char const *method_format, va_list args) {
	int result;
	DREF DeeObject *message, *error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(unicode_printer_printf(&printer, "type %k does not support: %s.", Dee_TYPE(self), api) < 0)
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
err_seq_unsupportedf(DeeObject *self, char const *method_format, ...) {
	int result;
	va_list args;
	va_start(args, method_format);
	result = err_api_vunsupportedf("Sequence", self, method_format, args);
	va_end(args);
	return result;
}

PRIVATE ATTR_COLD NONNULL((1)) int
err_set_unsupportedf(DeeObject *self, char const *method_format, ...) {
	int result;
	va_list args;
	va_start(args, method_format);
	result = err_api_vunsupportedf("Set", self, method_format, args);
	va_end(args);
	return result;
}

PRIVATE ATTR_COLD NONNULL((1)) int
err_map_unsupportedf(DeeObject *self, char const *method_format, ...) {
	int result;
	va_list args;
	va_start(args, method_format);
	result = err_api_vunsupportedf("Mapping", self, method_format, args);
	va_end(args);
	return result;
}


/* Error implementations for `Sequence.enumerate()' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithError(DeeObject *self) {
	err_seq_unsupportedf(self, "enumerate()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithError(DeeObject *self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "enumerate(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_seq_unsupportedf(self, "enumerate(%r, %r)", start, end);
	return NULL;
}







/************************************************************************/
/* Sequence.__foreach_reverse__                                         */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self,
                                                        Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index_fast)(self, size);
		if likely(item) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex(DeeObject *__restrict self,
                                                    Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index)(self, size);
		if likely(item) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self,
                                                       Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_trygetitem_index)(self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeObAndGetItem(DeeObject *__restrict self,
                                                 Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	for (;;) {
		DREF DeeObject *item;
		int size_is_nonzero = DeeObject_Bool(sizeob);
		if unlikely(size_is_nonzero < 0)
			goto err_sizeob;
		if (!size_is_nonzero)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob;
		item = (*seq->tp_getitem)(self, sizeob);
		if likely(item) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp_sizeob;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob;
		}
		if (DeeThread_CheckInterrupt())
			goto err_sizeob;
	}
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob:
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

/************************************************************************/
/* Sequence.__enumerate_index_reverse__                                 */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                               void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index_fast)(self, size);
		temp = (*proc)(arg, size, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                           void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index)(self, size);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*proc)(arg, size, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                              void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_trygetitem_index)(self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*proc)(arg, size, item);
			Dee_Decref(item);
		} else {
			temp = (*proc)(arg, size, NULL);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                        void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *startob = NULL;
	DREF DeeObject *sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	if (end != (size_t)-1) {
		int error;
		DREF DeeObject *wanted_end;
		wanted_end = DeeInt_NewSize(end);
		if unlikely(!wanted_end)
			goto err_sizeob;
		/* if (sizeob > wanted_end)
		 *     sizeob = wanted_end; */
		error = DeeObject_CmpGrAsBool(sizeob, wanted_end);
		if unlikely(error == 0) {
			Dee_Decref(wanted_end);
		} else {
			Dee_Decref(sizeob);
			sizeob = wanted_end;
			if unlikely(error < 0)
				goto err_sizeob;
		}
	}
	if (start != 0) {
		startob = DeeInt_NewSize(start);
		if unlikely(!startob)
			goto err_sizeob;
	}
	for (;;) {
		size_t index_value;
		DREF DeeObject *item;
		int size_is_greater_start;
		if (startob) {
			size_is_greater_start = DeeObject_CmpGrAsBool(sizeob, startob);
		} else {
			size_is_greater_start = DeeObject_Bool(sizeob);
		}
		if unlikely(size_is_greater_start < 0)
			goto err_sizeob_startob;
		if (!size_is_greater_start)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob_startob;
		item = (*seq->tp_getitem)(self, sizeob);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_startob;
		}
		if unlikely(DeeObject_AsSize(sizeob, &index_value))
			goto err_sizeob_startob;
		temp = 0;
		if likely(index_value >= start && index_value < end)
			temp = (*proc)(arg, index_value, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp_sizeob_startob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_startob;
	}
	Dee_XDecref(startob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_startob:
	Dee_XDecref(startob);
/*err_temp_sizeob:*/
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob_startob:
	Dee_XDecref(startob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}







/************************************************************************/
/* enumerate()                                                          */
/************************************************************************/
/* Helpers for enumerating a sequence by invoking a given callback. */

struct seq_enumerate_data {
	DeeObject      *sed_cb;     /* [1..1] Enumeration callback */
	DREF DeeObject *sed_result; /* [?..1][valid_if(return == -2)] Enumeration result */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_enumerate_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	struct seq_enumerate_data *data;
	data    = (struct seq_enumerate_data *)arg;
	args[0] = index;
	args[1] = value;
	result  = DeeObject_Call(data->sed_cb, value ? 2 : 1, args);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return 0;
	}
	data->sed_result = result;
	return -2; /* Stop enumeration! */
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_enumerate_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	struct seq_enumerate_data *data;
	data    = (struct seq_enumerate_data *)arg;
	args[0] = DeeInt_NewSize(index);
	if unlikely(!args[0])
		goto err;
	args[1] = value;
	result  = DeeObject_Call(data->sed_cb, value ? 2 : 1, args);
	Dee_Decref(args[0]);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return 0;
	}
	data->sed_result = result;
	return -2; /* Stop enumeration! */
err:
	return -1;
}

INTERN NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Enumerate(DeeObject *self, DeeObject *cb) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeSeq_OperatorEnumerate(self, &seq_enumerate_cb, &data);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

INTERN NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_EnumerateWithIntRange(DeeObject *self, DeeObject *cb, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_enumerate_index_cb, &data, start, end);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

struct seq_enumerate_with_filter_data {
	DeeObject      *sedwf_cb;     /* [1..1] Enumeration callback */
	DREF DeeObject *sedwf_result; /* [?..1][valid_if(return == -2)] Enumeration result */
	DeeObject      *sedwf_start;  /* [1..1] Filter start */
	DeeObject      *sedwf_end;    /* [1..1] Filter end */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_enumerate_with_filter_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	int temp;
	DREF DeeObject *result;
	DeeObject *args[2];
	struct seq_enumerate_with_filter_data *data;
	data = (struct seq_enumerate_with_filter_data *)arg;
	/* if (data->sedwf_start <= index && data->sedwf_end > index) ... */
	temp = DeeObject_CmpLeAsBool(data->sedwf_start, index);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0;
	temp = DeeObject_CmpGrAsBool(data->sedwf_end, index);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0;
	args[0] = index;
	args[1] = value;
	result  = DeeObject_Call(data->sedwf_cb, value ? 2 : 1, args);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return 0;
	}
	data->sedwf_result = result;
	return -2; /* Stop enumeration! */
err:
	return -1;
}

INTERN NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeSeq_EnumerateWithRange(DeeObject *self, DeeObject *cb, DeeObject *start, DeeObject *end) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_with_filter_data data;
	data.sedwf_cb    = cb;
	data.sedwf_start = start;
	data.sedwf_end   = end;
	foreach_status = DeeSeq_OperatorEnumerate(self, &seq_enumerate_with_filter_cb, &data);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sedwf_result;
	return_none;
err:
	return NULL;
}





/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */

/************************************************************************/
/* any()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp > 0)
		temp = -2;
	return temp;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_foreach_with_key_cb(void *arg, DeeObject *item) {
	int temp;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	temp = DeeObject_BoolInherited(item);
	if (temp > 0)
		temp = -2;
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_any_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_any_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_any_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_any_foreach_with_key_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAnyWithSeqForeach(DeeObject *self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_any_foreach_cb, NULL);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAnyWithKeyWithSeqForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_any_foreach_with_key_cb, key);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAnyWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_any_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultAnyWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_any_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}






/************************************************************************/
/* all()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp == 0)
		temp = -2;
	return temp;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_foreach_with_key_cb(void *arg, DeeObject *item) {
	int temp;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	temp = DeeObject_BoolInherited(item);
	if (temp == 0)
		temp = -2;
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_all_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_all_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_all_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_all_foreach_with_key_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAllWithSeqForeach(DeeObject *self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_all_foreach_cb, NULL);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAllWithKeyWithSeqForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_all_foreach_with_key_cb, key);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAllWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_all_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultAllWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_all_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}






/************************************************************************/
/* parity()                                                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_parity_foreach_cb(void *arg, DeeObject *item) {
	(void)arg;
	return (Dee_ssize_t)DeeObject_Bool(item);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_parity_foreach_with_key_cb(void *arg, DeeObject *item) {
	(void)arg;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	return (Dee_ssize_t)DeeObject_BoolInherited(item);
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_parity_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_parity_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_parity_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_parity_foreach_with_key_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultParityWithSeqCount(DeeObject *self) {
	size_t count = DeeSeq_InvokeCount(self, Dee_True);
	if unlikely(count == (size_t)-1)
		goto err;
	return count & 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultParityWithSeqForeach(DeeObject *self) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeSeq_OperatorForeach(self, &seq_parity_foreach_cb, NULL);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultParityWithKeyWithSeqForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeSeq_OperatorForeach(self, &seq_parity_foreach_with_key_cb, key);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultParityWithRangeWithSeqCountWithRange(DeeObject *self, size_t start, size_t end) {
	size_t count = DeeSeq_InvokeCountWithRange(self, Dee_True, start, end);
	if unlikely(count == (size_t)-1)
		goto err;
	return count & 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultParityWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_parity_enumerate_cb, NULL, start, end);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultParityWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_parity_enumerate_with_key_cb, key, start, end);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}






/************************************************************************/
/* reduce()                                                             */
/************************************************************************/

struct seq_reduce_data {
	DeeObject      *gsr_combine; /* [1..1] Combinatory predicate (invoke as `gsr_combine(gsr_init, item)') */
	DREF DeeObject *gsr_result;  /* [0..1] Current reduction result, or NULL if no init given and at first item. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_reduce_foreach_with_init_cb(void *arg, DeeObject *item) {
	DeeObject *args[2];
	DREF DeeObject *reduced;
	struct seq_reduce_data *data;
	data    = (struct seq_reduce_data *)arg;
	args[0] = data->gsr_result;
	args[1] = item;
	reduced = DeeObject_Call(data->gsr_combine, 2, args);
	if unlikely(!reduced)
		goto err;
	Dee_Decref(data->gsr_result);
	data->gsr_result = reduced;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_reduce_foreach_cb(void *arg, DeeObject *item) {
	struct seq_reduce_data *data;
	data = (struct seq_reduce_data *)arg;
	if (data->gsr_result)
		return seq_reduce_foreach_with_init_cb(arg, item);
	data->gsr_result = item;
	Dee_Incref(item);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_reduce_enumerate_with_init_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return seq_reduce_foreach_with_init_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_reduce_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return seq_reduce_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithSeqForeach(DeeObject *self, DeeObject *combine) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_reduce_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		err_empty_sequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithInitWithSeqForeach(DeeObject *self, DeeObject *combine, DeeObject *init) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_reduce_foreach_with_init_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_reduce_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		err_empty_sequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithRangeAndInitWithSeqEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_reduce_enumerate_with_init_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}






/************************************************************************/
/* min()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_min_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = item;
		Dee_Incref(item);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, item);
	if (temp == 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(item);
		*(DeeObject **)arg = item;
	}
	return temp;
}

struct seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_min_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DREF DeeObject *kelem;
	struct seq_minmax_with_key_data *data;
	data = (struct seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(item);
		data->gsmmwk_result = item; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &item);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp > 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(item);
	data->gsmmwk_result  = item;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_min_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_min_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_min_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_min_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithSeqForeach(DeeObject *self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_min_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithKeyWithSeqForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_min_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_min_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_min_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}






/************************************************************************/
/* max()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_max_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = item;
		Dee_Incref(item);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, item);
	if (temp > 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(item);
		*(DeeObject **)arg = item;
	}
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_max_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DREF DeeObject *kelem;
	struct seq_minmax_with_key_data *data;
	data = (struct seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(item);
		data->gsmmwk_result = item; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &item);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp == 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(item);
	data->gsmmwk_result  = item;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_max_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_max_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_max_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_max_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithSeqForeach(DeeObject *self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_max_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithKeyWithSeqForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_max_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if (!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_max_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_max_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}






/************************************************************************/
/* sum()                                                                */
/************************************************************************/

struct seq_sum_data {
#define GENERIC_SEQ_SUM_MODE_FIRST  0 /* Mode not yet determined (expecting first item) */
#define GENERIC_SEQ_SUM_MODE_SECOND 1 /* Always comes after `GENERIC_SEQ_SUM_MODE_FIRST' and selects which mode to use */
#define GENERIC_SEQ_SUM_MODE_OBJECT 2 /* Generic object sum mode (using "operator +") */
#define GENERIC_SEQ_SUM_MODE_STRING 3 /* Use a unicode printer */
#define GENERIC_SEQ_SUM_MODE_BYTES  4 /* Use a bytes printer */
#define GENERIC_SEQ_SUM_MODE_INT    5 /* Got a single item, which had type "int" */
#ifndef CONFIG_NO_FPU
#define GENERIC_SEQ_SUM_MODE_FLOAT  6 /* Got a single item, which had type "float" */
#endif /* !CONFIG_NO_FPU */
	uintptr_t gss_mode; /* Sum-mode (one of `GENERIC_SEQ_SUM_MODE_*') */
	union {
		DREF DeeObject        *v_object; /* GENERIC_SEQ_SUM_MODE_SECOND, GENERIC_SEQ_SUM_MODE_OBJECT */
		struct unicode_printer v_string; /* GENERIC_SEQ_SUM_MODE_STRING */
		struct bytes_printer   v_bytes;  /* GENERIC_SEQ_SUM_MODE_BYTES */
		Dee_ssize_t            v_int;    /* GENERIC_SEQ_SUM_MODE_INT2 */
#ifndef CONFIG_NO_FPU
		double                 v_float;  /* GENERIC_SEQ_SUM_MODE_FLOAT2 */
#endif /* !CONFIG_NO_FPU */
		/* TODO: Special optimization for Sequence concat */
		/* TODO: Special optimization for Tuple */
		/* TODO: Special optimization for List */
	} gss_value;
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_sum_data_pack(struct seq_sum_data *__restrict self) {
	switch (self->gss_mode) {
	case GENERIC_SEQ_SUM_MODE_FIRST:
		return_none;
	case GENERIC_SEQ_SUM_MODE_SECOND:
	case GENERIC_SEQ_SUM_MODE_OBJECT:
		return self->gss_value.v_object;
	case GENERIC_SEQ_SUM_MODE_STRING:
		return unicode_printer_pack(&self->gss_value.v_string);
	case GENERIC_SEQ_SUM_MODE_BYTES:
		return bytes_printer_pack(&self->gss_value.v_bytes);
	case GENERIC_SEQ_SUM_MODE_INT:
		return DeeInt_NewSSize(self->gss_value.v_int);
#ifndef CONFIG_NO_FPU
	case GENERIC_SEQ_SUM_MODE_FLOAT:
		return DeeFloat_New(self->gss_value.v_float);
#endif /* !CONFIG_NO_FPU */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE NONNULL((1)) void DCALL
seq_sum_data_fini(struct seq_sum_data *__restrict self) {
	switch (self->gss_mode) {
	case GENERIC_SEQ_SUM_MODE_FIRST:
	case GENERIC_SEQ_SUM_MODE_INT:
#ifndef CONFIG_NO_FPU
	case GENERIC_SEQ_SUM_MODE_FLOAT:
#endif /* !CONFIG_NO_FPU */
		break;
	case GENERIC_SEQ_SUM_MODE_OBJECT:
	case GENERIC_SEQ_SUM_MODE_SECOND:
		Dee_Decref(self->gss_value.v_object);
		break;
	case GENERIC_SEQ_SUM_MODE_STRING:
		unicode_printer_fini(&self->gss_value.v_string);
		break;
	case GENERIC_SEQ_SUM_MODE_BYTES:
		bytes_printer_fini(&self->gss_value.v_bytes);
		break;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_sum_foreach_cb(void *arg, DeeObject *item) {
	struct seq_sum_data *data;
	data = (struct seq_sum_data *)arg;
	switch (data->gss_mode) {

	case GENERIC_SEQ_SUM_MODE_FIRST:
		Dee_Incref(item);
		data->gss_value.v_object = item;
		data->gss_mode = GENERIC_SEQ_SUM_MODE_SECOND;
		break;

	case GENERIC_SEQ_SUM_MODE_SECOND: {
		DeeTypeObject *tp_elem;
		DREF DeeObject *first = data->gss_value.v_object;
		tp_elem = Dee_TYPE(first);
		if (tp_elem == &DeeString_Type) {
			Dee_ssize_t temp;
			data->gss_mode = GENERIC_SEQ_SUM_MODE_STRING;
			unicode_printer_init(&data->gss_value.v_string);
			temp = unicode_printer_printstring(&data->gss_value.v_string, first);
			Dee_Decref(first);
			if unlikely(temp < 0)
				return temp;
			goto do_print_string;
		} else if (tp_elem == &DeeBytes_Type) {
			Dee_ssize_t temp;
			data->gss_mode = GENERIC_SEQ_SUM_MODE_BYTES;
			bytes_printer_init(&data->gss_value.v_bytes);
			temp = Dee_bytes_printer_printbytes(&data->gss_value.v_bytes, first);
			Dee_Decref(first);
			if unlikely(temp < 0)
				return temp;
			goto do_print_bytes;
		} else if (tp_elem == &DeeInt_Type) {
			Dee_ssize_t a, b;
			if (Dee_TYPE(item) != &DeeInt_Type)
				goto generic_second;
			if unlikely(!DeeInt_TryAsSSize(first, &a))
				goto generic_second;
			if unlikely(!DeeInt_TryAsSSize(item, &b))
				goto generic_second;
			if unlikely(OVERFLOW_SADD(a, b, &a))
				goto generic_second;
			Dee_Decref(first);
			data->gss_mode = GENERIC_SEQ_SUM_MODE_INT;
			data->gss_value.v_int = a;
			break;
#ifndef CONFIG_NO_FPU
		} else if (tp_elem == &DeeFloat_Type) {
			double total;
			if (DeeObject_AsDouble(item, &total))
				goto err;
			total += DeeFloat_VALUE(first);
			Dee_Decref(first);
			data->gss_mode = GENERIC_SEQ_SUM_MODE_FLOAT;
			data->gss_value.v_float = total;
			break;
#endif /* !CONFIG_NO_FPU */
		} else {
			/* ... */
		}
generic_second:
		item = DeeObject_Add(first, item);
		if unlikely(!item)
			goto err;
		Dee_Decref(first);
		data->gss_value.v_object = item;
		data->gss_mode = GENERIC_SEQ_SUM_MODE_OBJECT;
	}	break;

	case GENERIC_SEQ_SUM_MODE_OBJECT: {
		DREF DeeObject *result;
do_handle_object:
		result = DeeObject_Add(data->gss_value.v_object, item);
		if unlikely(!result)
			goto err;
		Dee_Decref(data->gss_value.v_object);
		data->gss_value.v_object = result;
	}	break;

	case GENERIC_SEQ_SUM_MODE_STRING:
do_print_string:
		return unicode_printer_printobject(&data->gss_value.v_string, item);

	case GENERIC_SEQ_SUM_MODE_BYTES:
do_print_bytes:
		return bytes_printer_printobject(&data->gss_value.v_bytes, item);

	case GENERIC_SEQ_SUM_MODE_INT: {
		Dee_ssize_t total;
		if (!DeeInt_Check(item))
			goto switch_to_handle_object_from_int2;
		if unlikely(!DeeInt_TryAsSSize(item, &total))
			goto switch_to_handle_object_from_int2;
		if unlikely(OVERFLOW_SADD(data->gss_value.v_int, total, &total))
			goto switch_to_handle_object_from_int2;
		data->gss_value.v_int = total;
	}	break;

#ifndef CONFIG_NO_FPU
	case GENERIC_SEQ_SUM_MODE_FLOAT: {
		double total;
		if unlikely(!DeeObject_AsDouble(item, &total))
			goto err;
		data->gss_value.v_float += total;
	}	break;
#endif /* !CONFIG_NO_FPU */

	default: __builtin_unreachable();
	}
	return 0;
err:
	return -1;
switch_to_handle_object_from_int2:
	data->gss_value.v_object = DeeInt_NewSSize(data->gss_value.v_int);
	if unlikely(!data->gss_value.v_object)
		goto err;
	data->gss_mode = GENERIC_SEQ_SUM_MODE_OBJECT;
	goto do_handle_object;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_sum_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_sum_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSumWithSeqForeach(DeeObject *self) {
	Dee_ssize_t foreach_status;
	struct seq_sum_data data;
	data.gss_mode = GENERIC_SEQ_SUM_MODE_FIRST;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_sum_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	return seq_sum_data_pack(&data);
err:
	seq_sum_data_fini(&data);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSumWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct seq_sum_data data;
	data.gss_mode = GENERIC_SEQ_SUM_MODE_FIRST;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_sum_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err;
	return seq_sum_data_pack(&data);
err:
	seq_sum_data_fini(&data);
	return NULL;
}






/************************************************************************/
/* count()                                                              */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_count_foreach_cb(void *arg, DeeObject *item) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}

struct seq_count_with_key_data {
	DeeObject *gscwk_key;   /* [1..1] Key predicate */
	DeeObject *gscwk_kelem; /* [1..1] Keyed search element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_count_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct seq_count_with_key_data *data;
	data = (struct seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, item, data->gscwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_count_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_count_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_count_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_count_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultCountWithSeqFind(DeeObject *self, DeeObject *item) {
	return DeeSeq_DefaultCountWithRangeWithSeqFind(self, item, 0, (size_t)-1);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultCountWithSeqForeach(DeeObject *self, DeeObject *item) {
	return (size_t)DeeSeq_OperatorForeach(self, &seq_count_foreach_cb, item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_DefaultCountWithKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item, DeeObject *key) {
	return DeeSeq_DefaultCountWithRangeAndKeyWithSeqFindWithKey(self, item, 0, (size_t)-1, key);
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_DefaultCountWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_count_with_key_foreach_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultCountWithRangeWithSeqFind(DeeObject *self, DeeObject *item,
                                        size_t start, size_t end) {
	size_t result = 0;
	Dee_mh_seq_find_t mh_seq_find = DeeType_RequireSeqFind(Dee_TYPE(self));
	while (start < end) {
		size_t match = (*mh_seq_find)(self, item, start, end);
		if unlikely(match == (size_t)Dee_COMPARE_ERR)
			goto err;
		if (match == (size_t)-1)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		start = match + 1;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultCountWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return (size_t)DeeSeq_OperatorEnumerateIndex(self, &seq_count_enumerate_cb, item, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultCountWithRangeAndKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item,
                                                     size_t start, size_t end,
                                                     DeeObject *key) {
	size_t result = 0;
	Dee_mh_seq_find_with_key_t mh_seq_find_with_key = DeeType_RequireSeqFindWithKey(Dee_TYPE(self));
	while (start < end) {
		size_t match = (*mh_seq_find_with_key)(self, item, start, end, key);
		if unlikely(match == (size_t)Dee_COMPARE_ERR)
			goto err;
		if (match == (size_t)-1)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		start = match + 1;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultCountWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_count_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}






/************************************************************************/
/* contains()                                                           */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_contains_foreach_cb(void *arg, DeeObject *item) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? -2 : 0;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_contains_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct seq_count_with_key_data *data;
	data = (struct seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, item, data->gscwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? -2 : 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_contains_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_contains_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_contains_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_contains_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultContainsWithSeqFind(DeeObject *self, DeeObject *item) {
	return DeeSeq_DefaultContainsWithRangeWithSeqFind(self, item, 0, (size_t)-1);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultContainsWithForeach(DeeObject *self, DeeObject *item) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_contains_foreach_cb, item);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultContainsWithKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item, DeeObject *key) {
	return DeeSeq_DefaultContainsWithRangeAndKeyWithSeqFindWithKey(self, item, 0, (size_t)-1, key);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultContainsWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_contains_with_key_foreach_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultContainsWithRangeWithSeqFind(DeeObject *self, DeeObject *item,
                                           size_t start, size_t end) {
	size_t match = DeeSeq_InvokeFind(self, item, start, end);
	if unlikely(match == (size_t)Dee_COMPARE_ERR)
		goto err;
	return match != (size_t)-1 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultContainsWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                                     size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_contains_enumerate_cb, item, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultContainsWithRangeAndKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item,
                                                        size_t start, size_t end, DeeObject *key) {
	size_t match = DeeSeq_InvokeFindWithKey(self, item, start, end, key);
	if unlikely(match == (size_t)Dee_COMPARE_ERR)
		goto err;
	return match != (size_t)-1 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultContainsWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                                           size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_contains_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gscwk_kelem);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
err:
	return -1;
}






/************************************************************************/
/* locate()                                                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_foreach_cb(void *arg, DeeObject *item) {
	DeeObject *elem_to_locate = *(DeeObject **)arg;
	int temp = DeeObject_TryCompareEq(elem_to_locate, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	if (temp == 0) {
		Dee_Incref(item);
		*(DeeObject **)arg = item;
		return -2;
	}
	return 0;
}

struct seq_locate_with_key_data {
	DeeObject *gslwk_kelem; /* [1..1] Keyed search element. */
	DeeObject *gslwk_key;   /* [1..1][in] Search key predicate
	                         * [1..1][out:DREF] Located element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct seq_locate_with_key_data *data;
	data = (struct seq_locate_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gslwk_kelem, item, data->gslwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(item);
		data->gslwk_key = item;
		return -2;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_locate_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_locate_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_locate_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_locate_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithSeqForeach(DeeObject *self, DeeObject *item) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_locate_foreach_cb, &item);
	if likely(foreach_status == -2)
		return item;
	if (foreach_status == 0)
		err_item_not_found(self, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_locate_with_key_foreach_cb, &data);
	Dee_Decref(data.gslwk_kelem);
	if likely(foreach_status == -2)
		return data.gslwk_key;
	if (foreach_status == 0)
		err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_locate_enumerate_cb, &item, start, end);
	if likely(foreach_status == -2)
		return item;
	if (foreach_status == 0)
		err_item_not_found(self, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                                         size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_locate_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gslwk_kelem);
	if likely(foreach_status == -2)
		return data.gslwk_key;
	if (foreach_status == 0)
		err_item_not_found(self, item);
err:
	return NULL;
}






/************************************************************************/
/* rlocate()                                                            */
/************************************************************************/
struct seq_rlocate_with_foreach_data {
	DeeObject      *gsrlwf_elem;   /* [1..1] Element to search for. */
	DREF DeeObject *gsrlwf_result; /* [0..1] Most recent match. */
};

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_rlocate_with_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	int temp;
	struct seq_rlocate_with_foreach_data *data;
	(void)index;
	if (!item)
		return 0;
	data = (struct seq_rlocate_with_foreach_data *)arg;
	temp = DeeObject_TryCompareEq(data->gsrlwf_elem, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(item);
		Dee_XDecref(data->gsrlwf_result);
		data->gsrlwf_result = item;
	}
	return 0;
err:
	return -1;
}

struct seq_rlocate_with_key_and_foreach_data {
	DeeObject      *gsrlwkf_kelem;  /* [1..1] Keyed element to search for. */
	DREF DeeObject *gsrlwkf_result; /* [0..1] Most recent match. */
	DeeObject      *gsrlwkf_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_rlocate_with_key_and_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	int temp;
	struct seq_rlocate_with_key_and_foreach_data *data;
	(void)index;
	if (!item)
		return 0;
	data = (struct seq_rlocate_with_key_and_foreach_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gsrlwkf_kelem, item, data->gsrlwkf_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(item);
		Dee_XDecref(data->gsrlwkf_result);
		data->gsrlwkf_result = item;
	}
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item,
                                                           size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	Dee_mh_seq_enumerate_index_reverse_t op;
	op = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(op);
	foreach_status = (*op)(self, &seq_locate_enumerate_cb, &item, start, end);
	if likely(foreach_status == -2)
		return item;
	if (foreach_status == 0)
		err_item_not_found(self, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                                    size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct seq_rlocate_with_foreach_data data;
	data.gsrlwf_elem   = item;
	data.gsrlwf_result = NULL;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_rlocate_with_enumerate_cb, &data, start, end);
	if likely(foreach_status == 0) {
		if (data.gsrlwf_result)
			return data.gsrlwf_result;
		err_item_not_found(self, item);
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item,
                                                                 size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	Dee_mh_seq_enumerate_index_reverse_t op;
	struct seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	op = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(op);
	foreach_status = (*op)(self, &seq_locate_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gslwk_kelem);
	if likely(foreach_status == -2)
		return data.gslwk_key;
	if (foreach_status == 0)
		err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                                          size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_rlocate_with_key_and_foreach_data data;
	data.gsrlwkf_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsrlwkf_kelem)
		goto err;
	data.gsrlwkf_key    = key;
	data.gsrlwkf_result = NULL;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_rlocate_with_key_and_enumerate_cb,
	                                               &data, start, end);
	Dee_Decref(data.gsrlwkf_kelem);
	if likely(foreach_status == 0) {
		if (data.gsrlwkf_result)
			return data.gsrlwkf_result;
		err_item_not_found(self, item);
	}
err:
	return NULL;
}






/************************************************************************/
/* startswith()                                                         */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultStartsWithWithSeqTryGetFirst(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeObject *first;
	first = DeeSeq_InvokeTryGetFirst(self);
	if unlikely(!first)
		goto err;
	if (first == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, first);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultStartsWithWithKeyWithSeqTryGetFirst(DeeObject *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *first;
	first = DeeSeq_InvokeTryGetFirst(self);
	if unlikely(!first)
		goto err;
	if (first == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_elem;
	result = DeeObject_TryCompareKeyEq(item, first, key);
	Dee_Decref(item);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_elem:
	Dee_Decref(item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultStartsWithWithRangeWithSeqTryGetItemIndex(DeeObject *self, DeeObject *item,
                                                        size_t start, size_t end) {
	int result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return 0;
	selfitem = DeeSeq_OperatorTryGetItemIndex(self, start);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, selfitem);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultStartsWithWithRangeAndKeyWithSeqTryGetItemIndex(DeeObject *self, DeeObject *item,
                                                              size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return 0;
	selfitem = DeeSeq_OperatorTryGetItemIndex(self, start);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_selfitem;
	result = DeeObject_TryCompareKeyEq(item, selfitem, key);
	Dee_Decref(item);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_selfitem:
	Dee_Decref(selfitem);
err:
	return -1;
}






/************************************************************************/
/* endswith()                                                           */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultEndsWithWithSeqTryGetLast(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeObject *last = DeeSeq_InvokeTryGetLast(self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, last);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultEndsWithWithKeyWithSeqTryGetLast(DeeObject *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *last = DeeSeq_InvokeTryGetLast(self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_elem;
	result = DeeObject_TryCompareKeyEq(item, last, key);
	Dee_Decref(item);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_elem:
	Dee_Decref(item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultEndsWithWithRangeWithSeqSizeAndSeqTryGetItemIndex(DeeObject *self, DeeObject *item,
                                                                size_t start, size_t end) {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize = DeeSeq_OperatorSize(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = DeeSeq_OperatorTryGetItemIndex(self, end - 1);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, selfitem);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultEndsWithWithRangeAndKeyWithSeqSizeAndSeqTryGetItemIndex(DeeObject *self, DeeObject *item,
                                                                      size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize = DeeSeq_OperatorSize(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = DeeSeq_OperatorTryGetItemIndex(self, end - 1);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_selfitem;
	result = DeeObject_TryCompareKeyEq(item, selfitem, key);
	Dee_Decref(item);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_selfitem:
	Dee_Decref(selfitem);
err:
	return -1;
}






/************************************************************************/
/* find()                                                               */
/************************************************************************/
union seq_find_data {
	DeeObject *gsfd_elem;  /* [in][1..1] Element to search for */
	size_t     gsfd_index; /* [out] Located index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_find_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	union seq_find_data *data;
	data = (union seq_find_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsfd_elem, value);
	if (cmp == 0) {
		/* Found the index! */
		data->gsfd_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultFindWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                        size_t start, size_t end) {
	Dee_ssize_t status;
	union seq_find_data data;
	data.gsfd_elem = item;
	status = DeeSeq_OperatorEnumerateIndex(self, &seq_find_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* find() (with key)                                                    */
/************************************************************************/
struct seq_find_with_key_data {
	union seq_find_data gsfwk_base; /* Base find data */
	DeeObject                  *gsfwk_key;  /* Find element key */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_find_with_key_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct seq_find_with_key_data *data;
	data = (struct seq_find_with_key_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareKeyEq(data->gsfwk_base.gsfd_elem, value, data->gsfwk_key);
	if (cmp == 0) {
		/* Found the index! */
		data->gsfwk_base.gsfd_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                               size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct seq_find_with_key_data data;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	status = DeeSeq_OperatorEnumerateIndex(self, &seq_find_with_key_cb, &data, start, end);
	Dee_Decref(data.gsfwk_base.gsfd_elem);
	if likely(status == -2) {
		if unlikely(data.gsfwk_base.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfwk_base.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* rfind()                                                              */
/************************************************************************/
struct seq_rfind_data {
	DeeObject *gsrfd_elem;   /* [1..1] The element to search for */
	size_t     gsrfd_result; /* The last-matched index. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_rfind_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct seq_rfind_data *data;
	data = (struct seq_rfind_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfd_elem, value);
	if (cmp == 0)
		data->gsrfd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRFindWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item,
                                                size_t start, size_t end) {
	Dee_ssize_t status;
	union seq_find_data data;
	Dee_mh_seq_enumerate_index_reverse_t renum;
	data.gsfd_elem = item;
	renum = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(renum);
	status = (*renum)(self, &seq_find_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRFindWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                         size_t start, size_t end) {
	Dee_ssize_t status;
	struct seq_rfind_data data;
	data.gsrfd_elem   = item;
	data.gsrfd_result = (size_t)-1;
	status = DeeSeq_OperatorEnumerateIndex(self, &seq_rfind_cb, &data, start, end);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.gsrfd_result == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return data.gsrfd_result;
err:
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* rfind() (with key)                                                   */
/************************************************************************/
struct seq_rfind_with_key_data {
	DeeObject *gsrfwkd_kelem;   /* [1..1] The element to search for */
	size_t     gsrfwkd_result; /* The last-matched index. */
	DeeObject *gsrfwkd_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_rfind_with_key_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct seq_rfind_with_key_data *data;
	data = (struct seq_rfind_with_key_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfwkd_kelem, value);
	if (cmp == 0)
		data->gsrfwkd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item,
                                                       size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct seq_find_with_key_data data;
	Dee_mh_seq_enumerate_index_reverse_t renum;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	renum = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(renum);
	status = (*renum)(self, &seq_find_with_key_cb, &data, start, end);
	Dee_Decref(data.gsfwk_base.gsfd_elem);
	if likely(status == -2) {
		if unlikely(data.gsfwk_base.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfwk_base.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item,
                                                size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct seq_rfind_with_key_data data;
	data.gsrfwkd_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsrfwkd_kelem)
		goto err;
	data.gsrfwkd_result = (size_t)-1;
	status = DeeSeq_OperatorEnumerateIndex(self, &seq_rfind_with_key_cb, &data, start, end);
	Dee_Decref(data.gsrfwkd_kelem);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.gsrfwkd_result == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return data.gsrfwkd_result;
err:
	return (size_t)Dee_COMPARE_ERR;
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
	Dee_mh_seq_pop_t tsc_pop;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	tsc_pop = DeeType_RequireSeqPop(Dee_TYPE(self));
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
	return err_seq_unsupportedf(self, "erase(%" PRFuSIZ ", %" PRFuSIZ ")", index, count);
}






/************************************************************************/
/* insert()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertWithSeqInsertAll(DeeObject *self, size_t index, DeeObject *item) {
	int result;
	DREF DeeTupleObject *elem_tuple;
	elem_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!elem_tuple)
		goto err;
	DeeTuple_SET(elem_tuple, 0, item);
	result = DeeSeq_InvokeInsertAll(self, index, (DeeObject *)elem_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)elem_tuple);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertWithError(DeeObject *self, size_t index, DeeObject *item) {
	return err_seq_unsupportedf(self, "insert(%" PRFuSIZ ", %r)", index, item);
}






/************************************************************************/
/* insertall()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithSetRangeIndex(DeeObject *self, size_t index, DeeObject *items) {
	size_t end_index;
	size_t items_size = DeeSeq_OperatorSize(items);
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

struct seq_insertall_with_foreach_insert_data {
	Dee_mh_seq_insert_t dsiawfid_insert; /* [1..1] Insert callback */
	DeeObject           *dsiawfid_self;   /* [1..1] The sequence to insert into */
	size_t               dsiawfid_index;  /* Next index for insertion */
};

PRIVATE WUNUSED_T NONNULL_T((2)) Dee_ssize_t DCALL
seq_insertall_with_foreach_insert_cb(void *arg, DeeObject *item) {
	struct seq_insertall_with_foreach_insert_data *data;
	data = (struct seq_insertall_with_foreach_insert_data *)arg;
	return (Dee_ssize_t)(*data->dsiawfid_insert)(data->dsiawfid_self, data->dsiawfid_index++, item);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithSeqInsert(DeeObject *self, size_t index, DeeObject *items) {
	struct seq_insertall_with_foreach_insert_data data;
	data.dsiawfid_self   = self;
	data.dsiawfid_index  = index;
	data.dsiawfid_insert = DeeType_RequireSeqInsert(Dee_TYPE(self));
	return (int)DeeSeq_OperatorForeach(items, &seq_insertall_with_foreach_insert_cb, &data);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithError(DeeObject *self, size_t index, DeeObject *items) {
	return err_seq_unsupportedf(self, "insertall(%" PRFuSIZ ", %r)", index, items);
}






/************************************************************************/
/* pushfront()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultPushFrontWithSeqInsert(DeeObject *self, DeeObject *item) {
	return DeeSeq_InvokeInsert(self, 0, item);
}






/************************************************************************/
/* append()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithSeqExtend(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeTupleObject *elem_tuple;
	elem_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!elem_tuple)
		goto err;
	DeeTuple_SET(elem_tuple, 0, item);
	result = DeeSeq_InvokeExtend(self, (DeeObject *)elem_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)elem_tuple);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithSizeAndSeqInsert(DeeObject *self, DeeObject *item) {
	size_t selfsize;
	selfsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	return DeeSeq_InvokeInsert(self, selfsize, item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithError(DeeObject *self, DeeObject *item) {
	return err_seq_unsupportedf(self, "append(%r)", item);
}






/************************************************************************/
/* extend()                                                             */
/************************************************************************/
struct seq_extend_with_foreach_append_data {
	Dee_mh_seq_append_t dsewfad_append; /* [1..1] Append callback */
	DeeObject           *dsewfad_self;   /* [1..1] The sequence to append to */
};

PRIVATE WUNUSED_T NONNULL_T((2)) Dee_ssize_t DCALL
seq_extend_with_foreach_append_cb(void *arg, DeeObject *item) {
	struct seq_extend_with_foreach_append_data *data;
	data = (struct seq_extend_with_foreach_append_data *)arg;
	return (Dee_ssize_t)(*data->dsewfad_append)(data->dsewfad_self, item);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithSeqAppend(DeeObject *self, DeeObject *items) {
	struct seq_extend_with_foreach_append_data data;
	data.dsewfad_self   = self;
	data.dsewfad_append = DeeType_RequireSeqAppend(Dee_TYPE(self));
	return (int)DeeSeq_OperatorForeach(items, &seq_extend_with_foreach_append_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithSizeAndSeqInsertAll(DeeObject *self, DeeObject *items) {
	size_t selfsize;
	selfsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	return DeeSeq_InvokeInsertAll(self, selfsize, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithError(DeeObject *self, DeeObject *items) {
	return err_seq_unsupportedf(self, "extend(%r)", items);
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
	err_seq_unsupportedf(self, "xchitem(%" PRFuSIZ ", %r)", index, value);
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
DeeSeq_DefaultClearWithSeqErase(DeeObject *self) {
	return DeeSeq_InvokeErase(self, 0, (size_t)-1);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithSeqRemoveAll(DeeObject *self) {
	return DeeSet_InvokeRemoveAll(self, self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithMapRemoveKeys(DeeObject *self) {
	int result;
	DREF DeeObject *keys;
	keys = DeeMap_InvokeKeys(self);
	if unlikely(!keys)
		goto err;
	result = DeeMap_InvokeRemoveKeys(self, keys);
	Dee_Decref(keys);
	return result;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithError(DeeObject *self) {
	return err_seq_unsupportedf(self, "clear()");
}






/************************************************************************/
/* pop()                                                                */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultPopWithSizeAndGetItemIndexAndSeqErase(DeeObject *self, Dee_ssize_t index) {
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
		if unlikely(DeeSeq_InvokeErase(self, index, 1))
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
	err_seq_unsupportedf(self, "pop(" PRFdSIZ ")", index);
	return NULL;
}






/************************************************************************/
/* remove()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithSeqRemoveAll(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end) {
	size_t result;
	result = DeeSeq_InvokeRemoveAll(self, item, start, end, 1);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
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
	/* .tp_iterator      = */ NULL,
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
	/* .tp_iterator      = */ NULL,
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
DeeSeq_DefaultRemoveWithSeqRemoveIf(DeeObject *self, DeeObject *item,
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
	result = DeeSeq_InvokeRemoveIf(self, (DeeObject *)pred, start, end, 1);
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
DeeSeq_DefaultRemoveWithSeqFindAndDelItemIndex(DeeObject *self, DeeObject *item,
                                               size_t start, size_t end) {
	int result;
	size_t index = DeeSeq_InvokeFind(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                         size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &default_remove_with_enumerate_index_and_delitem_index_cb,
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
	return err_seq_unsupportedf(self, "remove(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}






/************************************************************************/
/* remove() (with key)                                                  */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithSeqRemoveAllWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, DeeObject *key) {
	size_t result;
	result = DeeSeq_InvokeRemoveAllWithKey(self, item, start, end, 1, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithSeqRemoveIf(DeeObject *self, DeeObject *item,
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
	result = DeeSeq_InvokeRemoveIf(self, (DeeObject *)pred, start, end, 1);
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
DeeSeq_DefaultRemoveWithKeyWithSeqFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                             size_t start, size_t end, DeeObject *key) {
	int result;
	size_t index = DeeSeq_InvokeFindWithKey(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &default_remove_with_key_with_enumerate_index_and_delitem_index_cb,
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
	return err_seq_unsupportedf(self, "remove(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}






/************************************************************************/
/* rremove()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithTSeqFindAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                 size_t start, size_t end) {
	int result;
	size_t index = DeeSeq_InvokeRFind(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithSeqEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                 size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	Dee_mh_seq_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	tsc_enumerate_index_reverse = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
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
DeeSeq_DefaultRRemoveWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                          size_t start, size_t end) {
	size_t index;
	index = DeeSeq_DefaultRFindWithSeqEnumerateIndex(self, item, start, end);
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
	return err_seq_unsupportedf(self, "rremove(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}






/************************************************************************/
/* rremove() (with key)                                                 */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithSeqRFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                               size_t start, size_t end, DeeObject *key) {
	int result;
	size_t index = DeeSeq_InvokeRFindWithKey(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                        size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	Dee_mh_seq_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	tsc_enumerate_index_reverse = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
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
DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                 size_t start, size_t end, DeeObject *key) {
	size_t index;
	index = DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex(self, item, start, end, key);
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
	return err_seq_unsupportedf(self, "rremove(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}






/************************************************************************/
/* removeall()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithSeqRemoveIf(DeeObject *self, DeeObject *item,
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
	result = DeeSeq_InvokeRemoveIf(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithSeqRemove(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end, size_t max) {
	size_t result = 0;
	Dee_mh_seq_remove_t tsc_remove;
	tsc_remove = DeeType_RequireSeqRemove(Dee_TYPE(self));
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
	return err_seq_unsupportedf(self, "removeall(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")",
	                            item, start, end, max);
}






/************************************************************************/
/* removeall() (with key)                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveIf(DeeObject *self, DeeObject *item,
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
	result = DeeSeq_InvokeRemoveIf(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err_pred:
	DeeObject_FREE(pred);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, size_t max,
                                                   DeeObject *key) {
	size_t result = 0;
	Dee_mh_seq_remove_with_key_t tsc_remove_with_key;
	tsc_remove_with_key = DeeType_RequireSeqRemoveWithKey(Dee_TYPE(self));
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
	return err_seq_unsupportedf(self, "removeall(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ", %r)",
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
	/* .tp_iterator      = */ NULL,
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
	PROXY_OBJECT_HEAD(sriwrak_should) /* [1..1] Predicate to determine if an element should be removed. */
} SeqRemoveIfWithRemoveAllKey;

STATIC_ASSERT(offsetof(SeqRemoveIfWithRemoveAllKey, sriwrak_should) == offsetof(ProxyObject, po_obj));
#define seq_removeif_with_removeall_key_init  generic_proxy_init
#define seq_removeif_with_removeall_key_fini  generic_proxy_fini
#define seq_removeif_with_removeall_key_visit generic_proxy_visit

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
	/* .tp_iterator      = */ NULL,
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
DeeSeq_DefaultRemoveIfWithSeqRemoveAllWithKey(DeeObject *self, DeeObject *should,
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
	result = DeeSeq_InvokeRemoveAllWithKey(self, &SeqRemoveIfWithRemoveAllItem_DummyInstance,
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
	return err_seq_unsupportedf(self, "removeif(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")",
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
DeeSeq_DefaultResizeWithSizeAndSeqEraseAndSeqExtend(DeeObject *self, size_t newsize, DeeObject *filler) {
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
		result = DeeSeq_InvokeExtend(self, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return DeeSeq_InvokeErase(self, newsize, oldsize - newsize);
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
	return err_seq_unsupportedf(self, "fill(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, filler);
}






/************************************************************************/
/* reverse()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithSeqReversedAndSetRangeIndex(DeeObject *self, size_t start, size_t end) {
	int result;
	DREF DeeObject *reversed;
	reversed = DeeSeq_InvokeReversed(self, start, end);
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
	return err_seq_unsupportedf(self, "reverse(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
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
DeeSeq_DefaultSortWithSeqSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end) {
	int result;
	DREF DeeObject *sorted;
	sorted = DeeSeq_InvokeSorted(self, start, end);
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
	return err_seq_unsupportedf(self, "sort(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}






/************************************************************************/
/* sort() (with key)                                                    */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultSortWithKeyWithSeqSortedAndSetRangeIndex(DeeObject *self, size_t start,
                                                       size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *sorted;
	sorted = DeeSeq_InvokeSortedWithKey(self, start, end, key);
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
	return err_seq_unsupportedf(self, "sort(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
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
/* bfind()                                                              */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBFindWithSeqBRange(DeeObject *self, DeeObject *item,
                                 size_t start, size_t end) {
	size_t result_range[2];
	if unlikely(DeeSeq_InvokeBRange(self, item, start, end, result_range))
		goto err;
	if (result_range[0] == result_range[1])
		return (size_t)-1; /* Not found */
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBFindWithError(DeeObject *self, DeeObject *item,
                             size_t start, size_t end) {
	err_seq_unsupportedf(self, "bfind(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* bfind() (with key)                                                   */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBFindWithKeyWithSeqBRangeWithKey(DeeObject *self, DeeObject *item,
                                               size_t start, size_t end, DeeObject *key) {
	size_t result_range[2];
	if unlikely(DeeSeq_InvokeBRangeWithKey(self, item, start, end, key, result_range))
		goto err;
	if (result_range[0] == result_range[1])
		return (size_t)-1; /* Not found */
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBFindWithKeyWithError(DeeObject *self, DeeObject *item,
                                    size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "bfind(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* bposition()                                                          */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBPositionWithSeqBRange(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end) {
	size_t result_range[2];
	if unlikely(DeeSeq_InvokeBRange(self, item, start, end, result_range))
		goto err;
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBPositionWithError(DeeObject *self, DeeObject *item,
                                 size_t start, size_t end) {
	err_seq_unsupportedf(self, "bposition(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* bposition() (with key)                                               */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBPositionWithKeyWithSeqBRangeWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, DeeObject *key) {
	size_t result_range[2];
	if unlikely(DeeSeq_InvokeBRangeWithKey(self, item, start, end, key, result_range))
		goto err;
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBPositionWithKeyWithError(DeeObject *self, DeeObject *item,
                                        size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "bposition(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* brange()                                                             */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultBRangeWithError(DeeObject *self, DeeObject *item,
                              size_t start, size_t end,
                              size_t result_range[2]) {
	(void)result_range;
	return err_seq_unsupportedf(self, "brange(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}






/************************************************************************/
/* brange() (with key)                                                  */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
DeeSeq_DefaultBRangeWithKeyWithError(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end, DeeObject *key,
                                     size_t result_range[2]) {
	(void)result_range;
	return err_seq_unsupportedf(self, "brange(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}






/************************************************************************/
/* blocate()                                                            */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithSeqBFindAndGetItemIndex(DeeObject *self, DeeObject *item,
                                                 size_t start, size_t end) {
	size_t index = DeeSeq_InvokeBFind(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		goto err_not_found;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index);
err_not_found:
	err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithError(DeeObject *self, DeeObject *item,
                               size_t start, size_t end) {
	err_seq_unsupportedf(self, "blocate(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return NULL;
}






/************************************************************************/
/* blocate() (with key)                                                 */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithKeyWithSeqBFindWithKeyAndGetItemIndex(DeeObject *self, DeeObject *item,
                                                               size_t start, size_t end, DeeObject *key) {
	size_t index = DeeSeq_InvokeBFindWithKey(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		goto err_not_found;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index);
err_not_found:
	err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithKeyWithError(DeeObject *self, DeeObject *item,
                                      size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "blocate(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return NULL;
}













/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/






/************************************************************************/
/* Set.insert()                                                         */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertWithSeqSizeAndSetInsertAll(DeeObject *self, DeeObject *key) {
	int temp;
	size_t old_size, new_size;
	DREF DeeObject *items;
	old_size = DeeSeq_OperatorSize(self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	items = DeeTuple_NewVectorSymbolic(1, &key);
	if unlikely(!items)
		goto err;
	temp = DeeSet_InvokeInsertAll(self, items);
	DeeTuple_DecrefSymbolic(items);
	if unlikely(temp)
		goto err;
	new_size = DeeSeq_OperatorSize(self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertWithMapSetNew(DeeObject *self, DeeObject *key) {
	int result;
	DREF DeeObject *map_key_and_value[2];
	if unlikely(DeeObject_Unpack(key, 2, map_key_and_value))
		goto err;
	result = DeeMap_InvokeSetNew(self, map_key_and_value[0], map_key_and_value[1]);
	Dee_Decref(map_key_and_value[1]);
	Dee_Decref(map_key_and_value[0]);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertWithSeqSeqContainsAndSeqAppend(DeeObject *self, DeeObject *key) {
	int contains = DeeSeq_InvokeContains(self, key);
	if unlikely(contains < 0)
		goto err;
	if (contains)
		return 0;
	if unlikely(DeeSeq_InvokeAppend(self, key))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertWithError(DeeObject *self, DeeObject *key) {
	return err_set_unsupportedf(self, "insert(%r)", key);
}







/************************************************************************/
/* Set.remove()                                                         */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultRemoveWithSeqSizeAndSeqRemoveAll(DeeObject *self, DeeObject *key) {
	int temp;
	size_t old_size, new_size;
	DREF DeeObject *items;
	old_size = DeeSeq_OperatorSize(self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	items = DeeTuple_NewVectorSymbolic(1, &key);
	if unlikely(!items)
		goto err;
	temp = DeeSet_InvokeRemoveAll(self, items);
	DeeTuple_DecrefSymbolic(items);
	if unlikely(temp)
		goto err;
	new_size = DeeSeq_OperatorSize(self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultRemoveWithMapTryGetItemAndMapDelItem(DeeObject *self, DeeObject *key) {
	int temp;
	DREF DeeObject *current_value;
	DREF DeeObject *map_key_and_value[2];
	if unlikely(DeeObject_Unpack(key, 2, map_key_and_value))
		goto err;
	current_value = DeeMap_OperatorTryGetItem(self, map_key_and_value[0]);
	if unlikely(!current_value)
		goto err_map_key_and_value;
	if (current_value == ITER_DONE) {
		/* map-key doesn't exist -> can't remove */
		Dee_Decref(map_key_and_value[1]);
		Dee_Decref(map_key_and_value[0]);
		return 0;
	}
	temp = DeeObject_TryCompareEq(map_key_and_value[1], current_value);
	Dee_Decref(map_key_and_value[1]);
	Dee_Decref(current_value);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err_map_key;
	temp = DeeMap_OperatorDelItem(self, map_key_and_value[0]);
	Dee_Decref(map_key_and_value[0]);
	if unlikely(temp)
		goto err;
	return 1;
err_map_key_and_value:
	Dee_Decref(map_key_and_value[1]);
err_map_key:
	Dee_Decref(map_key_and_value[0]);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultRemoveWithSeqRemove(DeeObject *self, DeeObject *key) {
	return DeeSeq_InvokeRemove(self, key, 0, (size_t)-1);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultRemoveWithError(DeeObject *self, DeeObject *key) {
	return err_set_unsupportedf(self, "remove(%r)", key);
}







/************************************************************************/
/* Set.unify()                                                         */
/************************************************************************/
struct set_unify_foreach_data {
	DeeObject      *sufd_key;    /* [1..1] Key to find */
	DREF DeeObject *sufd_result; /* [?..1] Matching duplicate */
};

#define SET_UNIFY_FOREACH_FOUND ((Dee_ssize_t)-2)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_unify_foreach_cb(void *arg, DeeObject *key) {
	int temp;
	struct set_unify_foreach_data *data;
	data = (struct set_unify_foreach_data *)arg;
	temp = DeeObject_TryCompareEq(data->sufd_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(key);
		data->sufd_result = key;
		return SET_UNIFY_FOREACH_FOUND;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultUnifyWithSetInsertAndSeqForeach(DeeObject *self, DeeObject *key) {
	int status = DeeSet_InvokeInsert(self, key);
	if unlikely(status < 0)
		goto err;
	if (!status) {
		struct set_unify_foreach_data data;
		Dee_ssize_t fe_status;
		data.sufd_key = key;
		DBG_memset(&data.sufd_result, 0xcc, sizeof(data.sufd_result));
		fe_status = DeeSeq_OperatorForeach(self, &set_unify_foreach_cb, &data);
		if likely(fe_status == SET_UNIFY_FOREACH_FOUND) {
			ASSERT_OBJECT(data.sufd_result);
			return data.sufd_result; /* Inherit reference */
		}
		if unlikely(fe_status < 0)
			goto err;
	}
	return_reference_(key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultUnifyWithSeqLocateAndSeqAppend(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result = DeeSeq_InvokeLocate(self, key);
	if (result)
		return result;
	if unlikely(!DeeError_Catch(&DeeError_ValueError))
		goto err;
	if unlikely(DeeSeq_InvokeAppend(self, key))
		goto err;
	return_reference_(key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultUnifyWithError(DeeObject *self, DeeObject *key) {
	err_set_unsupportedf(self, "unify(%r)", key);
	return NULL;
}







/************************************************************************/
/* Set.insertall()                                                      */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertAllWithInplaceAdd(DeeObject *self, DeeObject *keys) {
	int result;
	Dee_Incref(self);
	result = (Dee_TYPE(self)->tp_math->tp_inplace_add)(&self, keys);
	Dee_Decref(self);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertAllWithInplaceOr(DeeObject *self, DeeObject *keys) {
	int result;
	Dee_Incref(self);
	result = (Dee_TYPE(self)->tp_math->tp_inplace_or)(&self, keys);
	Dee_Decref(self);
	return result;
}

#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define set_insertall_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireSetInsert(Dee_TYPE(self)))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define set_insertall_foreach_cb_PTR &set_insertall_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_insertall_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeSet_InvokeInsert((DeeObject *)arg, elem);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertAllWithSetInsert(DeeObject *self, DeeObject *keys) {
	Dee_ssize_t status = DeeSeq_OperatorForeach(keys, set_insertall_foreach_cb_PTR, self);
	return likely(status >= 0) ? 0 : -1;
}
#undef set_insertall_foreach_cb

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultInsertAllWithError(DeeObject *self, DeeObject *keys) {
	return err_set_unsupportedf(self, "insertall(%r)", keys);
}







/************************************************************************/
/* Set.removeall()                                                      */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultRemoveAllWithInplaceSub(DeeObject *self, DeeObject *keys) {
	int result;
	Dee_Incref(self);
	result = (Dee_TYPE(self)->tp_math->tp_inplace_sub)(&self, keys);
	Dee_Decref(self);
	return result;
}

#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define set_removeall_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireSetRemove(Dee_TYPE(self)))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define set_removeall_foreach_cb_PTR &set_removeall_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_removeall_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeSet_InvokeRemove((DeeObject *)arg, elem);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultRemoveAllWithSetRemove(DeeObject *self, DeeObject *keys) {
	Dee_ssize_t status = DeeSeq_OperatorForeach(keys, set_removeall_foreach_cb_PTR, self);
	return likely(status >= 0) ? 0 : -1;
}
#undef set_removeall_foreach_cb_PTR

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultRemoveAllWithError(DeeObject *self, DeeObject *keys) {
	return err_set_unsupportedf(self, "removeall(%r)", keys);
}







/************************************************************************/
/* Set.pop()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithSetFirstAndSetRemove(DeeObject *self) {
	DREF DeeObject *result = DeeSeq_InvokeGetFirst(self);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSet_InvokeRemove(self, result) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithMapPopItem(DeeObject *self) {
	DREF DeeObject *result = DeeMap_InvokePopItem(self);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		err_empty_sequence(self);
		goto err;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithSeqPop(DeeObject *self) {
	return DeeSeq_InvokePop(self, -1);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithError(DeeObject *self) {
	err_set_unsupportedf(self, "pop()");
	return NULL;
}







/************************************************************************/
/* Set.pop() (with default)                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithDefaultWithSeqTryGetFirstAndSetRemove(DeeObject *self, DeeObject *default_) {
	DREF DeeObject *result = DeeSeq_InvokeTryGetFirst(self);
	if (result == ITER_DONE)
		return_reference_(default_);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSet_InvokeRemove(self, result) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithDefaultWithMapPopItem(DeeObject *self, DeeObject *default_) {
	DREF DeeObject *result = DeeMap_InvokePopItem(self);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		Dee_Incref(default_);
		result = default_;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithDefaultWithSeqPop(DeeObject *self, DeeObject *default_) {
	DREF DeeObject *result = DeeSeq_InvokePop(self, -1);
	if unlikely(!result) {
		if (!DeeError_Catch(&DeeError_ValueError))
			goto err;
		return_reference_(default_);
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultPopWithDefaultWithError(DeeObject *self, DeeObject *default_) {
	err_set_unsupportedf(self, "pop(%r)", default_);
	return NULL;
}













/************************************************************************/
/* For `deemon.Mapping'                                                 */
/************************************************************************/






/************************************************************************/
/* Map.setold()                                                         */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetOldWithMapSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = DeeMap_InvokeSetOldEx(self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return 0;
	Dee_Decref(old_value);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetOldWithBoundItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	int bound = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, key);
	if (bound <= 0) {
		if unlikely(bound == -1)
			goto err;
		return 0; /* Key doesn't exist */
	}
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetOldWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if (!ITER_ISOK(old_value)) {
		if unlikely(!old_value)
			goto err;
		return 0; /* Key doesn't exist */
	}
	Dee_Decref(old_value);
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetOldWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if (!old_value) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			return 0; /* Key doesn't exist */
		goto err;
	}
	Dee_Decref(old_value);
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetOldWithError(DeeObject *self, DeeObject *key, DeeObject *value) {
	return err_map_unsupportedf(self, "setold(%r, %r)", key, value);
}







/************************************************************************/
/* Map.setold_ex()                                                      */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetOldExWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if (ITER_ISOK(old_value)) {
		if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
			goto err_old_value;
	}
	return old_value;
err_old_value:
	Dee_Decref(old_value);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetOldExWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if unlikely(!old_value) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			return ITER_DONE; /* Key doesn't exist */
		goto err;
	}
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err_old_value;
	return old_value;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetOldExWithError(DeeObject *self, DeeObject *key, DeeObject *value) {
	err_map_unsupportedf(self, "setold_ex(%r, %r)", key, value);
	return NULL;
}







/************************************************************************/
/* Map.setnew()                                                         */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithMapSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = DeeMap_InvokeSetNewEx(self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return 1;
	Dee_Decref(old_value);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithBoundItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *temp;
	int bound = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, key);
	if unlikely(bound == -1)
		goto err;
	if (bound > 0)
		return 0; /* Key already exists */
	temp = DeeMap_InvokeSetDefault(self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithBoundItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	int bound = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, key);
	if unlikely(bound == -1)
		goto err;
	if (bound > 0)
		return 0; /* Key already exists */
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithTryGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *temp;
	DREF DeeObject *bound = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if (bound != ITER_DONE) {
		if unlikely(!bound)
			goto err;
		Dee_Decref(bound);
		return 0; /* Key already exists */
	}
	temp = DeeMap_InvokeSetDefault(self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *bound = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if (bound != ITER_DONE) {
		if unlikely(!bound)
			goto err;
		Dee_Decref(bound);
		return 0; /* Key already exists */
	}
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *temp;
	DREF DeeObject *bound = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if (bound) {
		Dee_Decref(bound);
		return 0; /* Key already exists */
	} else {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem)) {
			/* Key doesn't exist */
		} else {
			goto err;
		}
	}
	temp = DeeMap_InvokeSetDefault(self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *bound = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if (bound) {
		Dee_Decref(bound);
		return 0; /* Key already exists */
	} else {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem)) {
			/* Key doesn't exist */
		} else {
			goto err;
		}
	}
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultSetNewWithError(DeeObject *self, DeeObject *key, DeeObject *value) {
	return err_map_unsupportedf(self, "setnew(%r, %r)", key, value);
}







/************************************************************************/
/* Map.setnew_ex()                                                      */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetNewExWithTryGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *temp;
	DREF DeeObject *old_value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	temp = DeeMap_InvokeSetDefault(self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetNewExWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetNewExWithGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *temp;
	DREF DeeObject *old_value = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if (old_value) {
		return old_value;
	} else {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem)) {
			/* Key doesn't exist */
		} else {
			goto err;
		}
	}
	temp = DeeMap_InvokeSetDefault(self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetNewExWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if (old_value) {
		return old_value;
	} else {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem)) {
			/* Key doesn't exist */
		} else {
			goto err;
		}
	}
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetNewExWithError(DeeObject *self, DeeObject *key, DeeObject *value) {
	err_map_unsupportedf(self, "setnew_ex(%r, %r)", key, value);
	return NULL;
}







/************************************************************************/
/* Map.setdefault()                                                     */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetDefaultWithMapSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = DeeMap_InvokeSetNewEx(self, key, value);
	if (old_value == ITER_DONE) {
		/* Value was just inserted */
		old_value = value;
		Dee_Incref(value);
	}
	return old_value;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetDefaultWithMapSetNewAndGetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp = DeeMap_InvokeSetNew(self, key, value);
	if unlikely(temp < 0)
		goto err;
	if (temp > 0)
		return_reference_(value);
	return (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetDefaultWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if (result == ITER_DONE) {
		int temp = DeeMap_InvokeSetNew(self, key, value);
		if unlikely(temp < 0)
			goto err;
		if likely(temp) {
			result = value;
			Dee_Incref(value);
		} else {
			result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
			if unlikely(result == ITER_DONE) {
				result = value;
				Dee_Incref(value);
			}
		}
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetDefaultWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if (!result) {
		int temp;
		if (!DeeError_Catch(&DeeError_KeyError) &&
		    !DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		temp = DeeMap_InvokeSetNew(self, key, value);
		if unlikely(temp < 0)
			goto err;
		if likely(temp) {
			result = value;
			Dee_Incref(value);
		} else {
			result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
			if unlikely(result == ITER_DONE) {
				result = value;
				Dee_Incref(value);
			}
		}
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultSetDefaultWithError(DeeObject *self, DeeObject *key, DeeObject *value) {
	err_map_unsupportedf(self, "setdefault(%r, %r)", key, value);
	return NULL;
}







/************************************************************************/
/* Map.update()                                                         */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultUpdateWithInplaceAdd(DeeObject *self, DeeObject *items) {
	int result;
	Dee_Incref(self);
	result = (*Dee_TYPE(self)->tp_math->tp_inplace_add)(&self, items);
	Dee_Decref(self);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultUpdateWithInplaceOr(DeeObject *self, DeeObject *items) {
	int result;
	Dee_Incref(self);
	result = (*Dee_TYPE(self)->tp_math->tp_inplace_or)(&self, items);
	Dee_Decref(self);
	return result;
}

#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define map_update_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *, DeeObject *))(Dee_funptr_t)&DeeObject_SetItem)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define map_update_foreach_cb_PTR &map_update_foreach_cb
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
map_update_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	return (Dee_ssize_t)DeeObject_SetItem((DeeObject *)arg, key, value);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultUpdateWithSetItem(DeeObject *self, DeeObject *items) {
	return (int)DeeObject_ForeachPair(items, map_update_foreach_cb_PTR, self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultUpdateWithError(DeeObject *self, DeeObject *items) {
	return err_map_unsupportedf(self, "update(%r)", items);
}







/************************************************************************/
/* Map.removekeys()                                                     */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultRemoveWithSizeAndDelItem(DeeObject *self, DeeObject *key) {
	size_t old_size, new_size;
	old_size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	if unlikely(old_size == 0)
		return 0;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem)(self, key))
		goto err;
	new_size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultRemoveWithBoundItemAndDelItem(DeeObject *self, DeeObject *key) {
	int bound = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, key);
	if unlikely(bound == -1)
		goto err;
	if (bound <= 0)
		return 0;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem)(self, key))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultRemoveWithSizeAndMapRemoveKeys(DeeObject *self, DeeObject *key) {
	size_t old_size, new_size;
	DREF DeeObject *keys;
	int temp;
	old_size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	if unlikely(old_size == 0)
		return 0;
	keys = DeeTuple_NewVectorSymbolic(1, &key);
	if unlikely(!keys)
		goto err;
	temp = DeeMap_InvokeRemoveKeys(self, keys);
	DeeTuple_DecrefSymbolic(keys);
	if unlikely(temp)
		goto err;
	new_size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultRemoveWithError(DeeObject *self, DeeObject *key) {
	return err_map_unsupportedf(self, "remove(%r)", key);
}







/************************************************************************/
/* Map.removekeys()                                                     */
/************************************************************************/
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define map_removekeys_foreach_delitem_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)Dee_TYPE(self)->tp_seq->tp_delitem)
#define map_removekeys_foreach_remove_cb_PTR  ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireMapRemove(Dee_TYPE(self)))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define map_removekeys_foreach_delitem_cb_PTR &map_removekeys_foreach_delitem_cb
#define map_removekeys_foreach_remove_cb_PTR  &map_removekeys_foreach_remove_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
map_removekeys_foreach_delitem_cb(void *arg, DeeObject *key) {
	return (Dee_ssize_t)(*Dee_TYPE((DeeObject *)arg)->tp_seq->tp_delitem)((DeeObject *)arg, key);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
map_removekeys_foreach_remove_cb(void *arg, DeeObject *key) {
	return (Dee_ssize_t)DeeMap_InvokeRemove((DeeObject *)arg, key);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultRemoveKeysWithDelItem(DeeObject *self, DeeObject *keys) {
	return (int)DeeSeq_OperatorForeach(keys, map_removekeys_foreach_delitem_cb_PTR, self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultRemoveKeysWithMapRemove(DeeObject *self, DeeObject *keys) {
	Dee_ssize_t result = DeeSeq_OperatorForeach(keys, map_removekeys_foreach_remove_cb_PTR, self);
	if likely(result > 0)
		result = 0;
	return (int)result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultRemoveKeysWithError(DeeObject *self, DeeObject *keys) {
	return err_map_unsupportedf(self, "removekeys(%r)", keys);
}







/************************************************************************/
/* Map.pop()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultPopWithGetItemAndMapRemove(DeeObject *self, DeeObject *key) {
	int temp;
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if unlikely(!result)
		goto err;
	temp = DeeMap_InvokeRemove(self, key);
	if unlikely(temp < 0)
		goto err_r;
	if likely(temp)
		return result;
	err_unknown_key(self, key);
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultPopWithGetItemAndDelItem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, key);
	if unlikely(!result)
		goto err;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem)(self, key))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultPopWithError(DeeObject *self, DeeObject *key) {
	err_map_unsupportedf(self, "pop(%r)", key);
	return NULL;
}







/************************************************************************/
/* Map.pop() (with default)                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultPopWithDefaultWithTryGetItemAndMapRemove(DeeObject *self, DeeObject *key, DeeObject *default_) {
	int temp;
	DREF DeeObject *result;
	result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if unlikely(!result)
		goto err;
	if (result == ITER_DONE)
		goto return_default;
	temp = DeeMap_InvokeRemove(self, key);
	if unlikely(temp < 0)
		goto err_r;
	if (temp)
		return result;
	Dee_Decref(result);
return_default:
	return_reference_(default_);
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultPopWithDefaultWithTryGetItemAndDelItem(DeeObject *self, DeeObject *key, DeeObject *default_) {
	DREF DeeObject *result;
	result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, key);
	if unlikely(!result)
		goto err;
	if (result == ITER_DONE)
		return_reference_(default_);
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem)(self, key))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultPopWithDefaultWithError(DeeObject *self, DeeObject *key, DeeObject *default_) {
	err_map_unsupportedf(self, "pop(%r, %r)", key, default_);
	return NULL;
}







/************************************************************************/
/* Map.popitem()                                                        */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultPopItemWithSeqTryGetFirstAndMapRemove(DeeObject *self) {
	int temp;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *result;
	for (;;) {
		result = DeeSeq_InvokeTryGetFirst(self);
		if (result == ITER_DONE)
			return_none;
		if unlikely(!result)
			goto err;
		if unlikely(DeeObject_Unpack(result, 2, key_and_value))
			goto err_r;
		Dee_Decref(key_and_value[1]);
		temp = DeeMap_InvokeRemove(self, key_and_value[0]);
		Dee_Decref(key_and_value[0]);
		if unlikely(temp < 0)
			goto err_r;
		if (temp)
			return result;
		Dee_Decref(result);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultPopItemWithSeqTryGetFirstAndDelItem(DeeObject *self) {
	int temp;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *result = DeeSeq_InvokeTryGetFirst(self);
	if (result == ITER_DONE)
		return_none;
	if unlikely(!result)
		goto err;
	if unlikely(DeeObject_Unpack(result, 2, key_and_value))
		goto err_r;
	Dee_Decref(key_and_value[1]);
	temp = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if unlikely(temp)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultPopItemWithError(DeeObject *self) {
	err_map_unsupportedf(self, "popitem()");
	return NULL;
}







/************************************************************************/
/* Map.keys                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultKeysWithMapIterKeys(DeeObject *self) {
	DREF DefaultSequence_MapProxy *result;
	result = DeeObject_MALLOC(DefaultSequence_MapProxy);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsmp_map = self;
	DeeObject_Init(result, &DefaultSequence_MapKeys_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultKeysWithError(DeeObject *self) {
	err_map_unsupportedf(self, "keys");
	return NULL;
}








/************************************************************************/
/* Map.values                                                           */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultValuesWithMapIterValues(DeeObject *self) {
	DREF DefaultSequence_MapProxy *result;
	result = DeeObject_MALLOC(DefaultSequence_MapProxy);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsmp_map = self;
	DeeObject_Init(result, &DefaultSequence_MapValues_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultValuesWithError(DeeObject *self) {
	err_map_unsupportedf(self, "values");
	return NULL;
}








/************************************************************************/
/* Map.iterkeys                                                         */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultIterKeysWithMapKeys(DeeObject *self) {
	DREF DeeObject *result;
	DREF DeeObject *keys = DeeMap_InvokeKeys(self);
	if unlikely(!keys)
		goto err;
	result = DeeObject_Iter(keys);
	Dee_Decref(keys);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultIterKeysWithError(DeeObject *self) {
	err_map_unsupportedf(self, "iterkeys");
	return NULL;
}








/************************************************************************/
/* Map.itervalues                                                       */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultIterValuesWithMapValues(DeeObject *self) {
	DREF DeeObject *result;
	DREF DeeObject *values = DeeMap_InvokeValues(self);
	if unlikely(!values)
		goto err;
	result = DeeObject_Iter(values);
	Dee_Decref(values);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultIterValuesWithIter(DeeObject *self) {
	/* NOTE: This only works when the mapping can't have unbound keys! */
	DREF DefaultIterator_PairSubItem *result;
	DeeTypeObject *itertyp;
	result = DeeObject_MALLOC(DefaultIterator_PairSubItem);
	if unlikely(!result)
		goto err;
	ASSERT(Dee_TYPE(self)->tp_seq);
	ASSERT(Dee_TYPE(self)->tp_seq->tp_iter);
	result->dipsi_iter = (*Dee_TYPE(self)->tp_seq->tp_iter)(self);
	if unlikely(!result->dipsi_iter)
		goto err_r;
	itertyp = Dee_TYPE(result->dipsi_iter);
	if unlikely((!itertyp->tp_iterator ||
	             !itertyp->tp_iterator->tp_nextvalue) &&
	            !DeeType_InheritIterNext(itertyp))
		goto err_r_iter_no_next;
	ASSERT(itertyp->tp_iterator);
	ASSERT(itertyp->tp_iterator->tp_nextvalue);
	result->dipsi_next = itertyp->tp_iterator->tp_nextvalue;
	DeeObject_Init(result, &DefaultIterator_WithNextValue);
	return (DREF DeeObject *)result;
err_r_iter_no_next:
	Dee_Decref(result->dipsi_iter);
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultIterValuesWithError(DeeObject *self) {
	err_map_unsupportedf(self, "itervalues");
	return NULL;
}















/************************************************************************/
/* Deemon user-code wrappers                                            */
/************************************************************************/

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("first");
print define_Dee_HashStr("last");
print define_Dee_HashStr("keys");
print define_Dee_HashStr("values");
print define_Dee_HashStr("iterkeys");
print define_Dee_HashStr("itervalues");
print define_Dee_HashStr("cb");
print define_Dee_HashStr("start");
print define_Dee_HashStr("end");
]]]*/
#define Dee_HashStr__first _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)
#define Dee_HashStr__last _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)
#define Dee_HashStr__keys _Dee_HashSelectC(0x97e36be1, 0x654d31bc4825131c)
#define Dee_HashStr__values _Dee_HashSelectC(0x33b551c8, 0xf6e3e991b86d1574)
#define Dee_HashStr__iterkeys _Dee_HashSelectC(0x62bd6adc, 0x535ac8ab28094ab3)
#define Dee_HashStr__itervalues _Dee_HashSelectC(0xcb00bab3, 0xe9a89082a994930a)
#define Dee_HashStr__cb _Dee_HashSelectC(0x75ffadba, 0x2501dbb50208b92e)
#define Dee_HashStr__start _Dee_HashSelectC(0xa2ed6890, 0x80b621ce3c3982d5)
#define Dee_HashStr__end _Dee_HashSelectC(0x37fb4a05, 0x6de935c204dc3d01)
/*[[[end]]]*/


/* Helper functions that (ab-)use the attribute cache system of types
 * to inject the most optimized version of getsets into top-level objects:
 * - Sequence.first
 * - Sequence.last
 * - Mapping.keys
 * - Mapping.values
 * - Mapping.iterkeys
 * - Mapping.itervalues
 */
#ifdef __OPTIMIZE_SIZE__
#define maybe_cache_optimized_seq_first_in_membercache(self)      (void)0
#define maybe_cache_optimized_seq_last_in_membercache(self)       (void)0
#define maybe_cache_optimized_map_keys_in_membercache(self)       (void)0
#define maybe_cache_optimized_map_values_in_membercache(self)     (void)0
#define maybe_cache_optimized_map_iterkeys_in_membercache(self)   (void)0
#define maybe_cache_optimized_map_itervalues_in_membercache(self) (void)0
#else /* __OPTIMIZE_SIZE__ */
PRIVATE struct type_getset tpconst gs_default_seq_first =
TYPE_GETSET_BOUND_NODOC(NULL, &default_seq_getfirst, &default_seq_delfirst,
                        &default_seq_setfirst, &default_seq_boundfirst);
PRIVATE NONNULL((1)) void DCALL
maybe_cache_optimized_seq_first_in_membercache(DeeTypeObject *__restrict self) {
	struct type_getset new_getset;
	new_getset.gs_name  = STR_first;
	new_getset.gs_get   = DeeType_RequireSeqGetFirst(self);
	new_getset.gs_del   = DeeType_RequireSeqDelFirst(self);
	new_getset.gs_set   = DeeType_RequireSeqSetFirst(self);
	new_getset.gs_bound = DeeType_RequireSeqBoundFirst(self);
	DeeTypeMRO_PatchGetSet(self, &DeeSeq_Type, Dee_HashStr__first,
	                       &new_getset, &gs_default_seq_first);
}

PRIVATE struct type_getset tpconst gs_default_seq_last =
TYPE_GETSET_BOUND_NODOC(NULL, &default_seq_getlast, &default_seq_dellast,
                        &default_seq_setlast, &default_seq_boundlast);
PRIVATE NONNULL((1)) void DCALL
maybe_cache_optimized_seq_last_in_membercache(DeeTypeObject *__restrict self) {
	struct type_getset new_getset;
	new_getset.gs_name  = STR_last;
	new_getset.gs_get   = DeeType_RequireSeqGetLast(self);
	new_getset.gs_del   = DeeType_RequireSeqDelLast(self);
	new_getset.gs_set   = DeeType_RequireSeqSetLast(self);
	new_getset.gs_bound = DeeType_RequireSeqBoundLast(self);
	DeeTypeMRO_PatchGetSet(self, &DeeSeq_Type, Dee_HashStr__last,
	                       &new_getset, &gs_default_seq_last);
}

PRIVATE struct type_getset tpconst gs_default_map_keys =
TYPE_GETTER_NODOC(NULL, &default_map_keys);
PRIVATE NONNULL((1)) void DCALL
maybe_cache_optimized_map_keys_in_membercache(DeeTypeObject *__restrict self) {
	struct type_getset new_getset;
	new_getset.gs_name  = STR_keys;
	new_getset.gs_get   = DeeType_RequireMapKeys(self);
	new_getset.gs_del   = NULL;
	new_getset.gs_set   = NULL;
	new_getset.gs_bound = NULL;
	DeeTypeMRO_PatchGetSet(self, &DeeMapping_Type, Dee_HashStr__keys,
	                       &new_getset, &gs_default_map_keys);
}

PRIVATE struct type_getset tpconst gs_default_map_values =
TYPE_GETTER_NODOC(NULL, &default_map_values);
PRIVATE NONNULL((1)) void DCALL
maybe_cache_optimized_map_values_in_membercache(DeeTypeObject *__restrict self) {
	struct type_getset new_getset;
	new_getset.gs_name  = STR_values;
	new_getset.gs_get   = DeeType_RequireMapValues(self);
	new_getset.gs_del   = NULL;
	new_getset.gs_set   = NULL;
	new_getset.gs_bound = NULL;
	DeeTypeMRO_PatchGetSet(self, &DeeMapping_Type, Dee_HashStr__values,
	                       &new_getset, &gs_default_map_values);
}

PRIVATE struct type_getset tpconst gs_default_map_iterkeys =
TYPE_GETTER_NODOC(NULL, &default_map_iterkeys);
PRIVATE NONNULL((1)) void DCALL
maybe_cache_optimized_map_iterkeys_in_membercache(DeeTypeObject *__restrict self) {
	struct type_getset new_getset;
	new_getset.gs_name  = STR_iterkeys;
	new_getset.gs_get   = DeeType_RequireMapIterKeys(self);
	new_getset.gs_del   = NULL;
	new_getset.gs_set   = NULL;
	new_getset.gs_bound = NULL;
	DeeTypeMRO_PatchGetSet(self, &DeeMapping_Type, Dee_HashStr__iterkeys,
	                       &new_getset, &gs_default_map_iterkeys);
}

PRIVATE struct type_getset tpconst gs_default_map_itervalues =
TYPE_GETTER_NODOC(NULL, &default_map_itervalues);
PRIVATE NONNULL((1)) void DCALL
maybe_cache_optimized_map_itervalues_in_membercache(DeeTypeObject *__restrict self) {
	struct type_getset new_getset;
	new_getset.gs_name  = STR_itervalues;
	new_getset.gs_get   = DeeType_RequireMapIterValues(self);
	new_getset.gs_del   = NULL;
	new_getset.gs_set   = NULL;
	new_getset.gs_bound = NULL;
	DeeTypeMRO_PatchGetSet(self, &DeeMapping_Type, Dee_HashStr__itervalues,
	                       &new_getset, &gs_default_map_itervalues);
}
#endif /* !__OPTIMIZE_SIZE__ */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_getfirst(DeeObject *__restrict self) {
	maybe_cache_optimized_seq_first_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeGetFirst(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_boundfirst(DeeObject *__restrict self) {
	maybe_cache_optimized_seq_first_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeBoundFirst(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_delfirst(DeeObject *__restrict self) {
	maybe_cache_optimized_seq_first_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeDelFirst(self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default_seq_setfirst(DeeObject *self, DeeObject *value) {
	maybe_cache_optimized_seq_first_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeSetFirst(self, value);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_getlast(DeeObject *__restrict self) {
	maybe_cache_optimized_seq_last_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeGetLast(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_boundlast(DeeObject *__restrict self) {
	maybe_cache_optimized_seq_last_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeBoundLast(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_dellast(DeeObject *__restrict self) {
	maybe_cache_optimized_seq_last_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeDelLast(self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default_seq_setlast(DeeObject *self, DeeObject *value) {
	maybe_cache_optimized_seq_last_in_membercache(Dee_TYPE(self));
	return DeeSeq_InvokeSetLast(self, value);
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
do_seq_enumerate_with_kw(DeeObject *self, size_t argc,
                         DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeObject *cb, *startob, *endob;
	size_t start, end;
	DeeKwArgs kwds;
	if (DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
	switch (argc) {

	case 0: {
		if unlikely((cb = DeeKwArgs_TryGetItemNRStringHash(&kwds, "cb", Dee_HashStr__cb)) == NULL)
			goto err;
		if unlikely((startob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
			goto err;
		if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (cb != ITER_DONE) {
handle_with_cb:
			if (endob != ITER_DONE) {
				if (startob != ITER_DONE) {
					if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
					    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
						result = DeeSeq_EnumerateWithIntRange(self, cb, start, end);
					} else {
						result = DeeSeq_EnumerateWithRange(self, cb, startob, endob);
					}
				} else if (DeeInt_Check(endob) && DeeInt_TryAsSize(endob, &end)) {
					result = DeeSeq_EnumerateWithIntRange(self, cb, 0, end);
				} else {
					startob = DeeObject_NewDefault(Dee_TYPE(endob));
					if unlikely(!startob)
						goto err;
					result = DeeSeq_EnumerateWithRange(self, cb, startob, endob);
					Dee_Decref(startob);
				}
			} else if (startob == ITER_DONE) {
				result = DeeSeq_Enumerate(self, cb);
			} else {
				ASSERT(startob != ITER_DONE);
				ASSERT(endob == ITER_DONE);
				if (DeeObject_AsSize(startob, &start))
					goto err;
				result = DeeSeq_EnumerateWithIntRange(self, cb, start, (size_t)-1);
			}
		} else {
			if (endob != ITER_DONE) {
				if (startob != ITER_DONE) {
					if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
					    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
						result = DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end);
					} else {
						result = DeeSeq_InvokeMakeEnumerationWithRange(self, startob, endob);
					}
				} else if (DeeInt_Check(endob) && DeeInt_TryAsSize(endob, &end)) {
					result = DeeSeq_InvokeMakeEnumerationWithIntRange(self, 0, end);
				} else {
					startob = DeeObject_NewDefault(Dee_TYPE(endob));
					if unlikely(!startob)
						goto err;
					result = DeeSeq_InvokeMakeEnumerationWithRange(self, startob, endob);
					Dee_Decref(startob);
				}
			} else if (startob == ITER_DONE) {
				result = DeeSeq_InvokeMakeEnumeration(self);
			} else {
				ASSERT(startob != ITER_DONE);
				ASSERT(endob == ITER_DONE);
				if (DeeObject_AsSize(startob, &start))
					goto err;
				result = DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, (size_t)-1);
			}
		}
	}	break;

	case 1: {
		cb = argv[0];
		if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (DeeCallable_Check(cb)) {
			if unlikely((startob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
				goto err;
			goto handle_with_cb;
		}
		startob = cb;
		if (endob != ITER_DONE) {
			if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
			    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
				result = DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end);
			} else {
				result = DeeSeq_InvokeMakeEnumerationWithRange(self, startob, endob);
			}
		} else {
			if (DeeObject_AsSize(startob, &start))
				goto err;
			result = DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, (size_t)-1);
		}
	}	break;

	case 2: {
		cb = argv[0];
		if (DeeCallable_Check(cb)) {
			if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
				goto err;
			startob = argv[1];
			goto handle_with_cb;
		}
		startob = argv[0];
		endob   = argv[1];
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
			result = DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end);
		} else {
			result = DeeSeq_InvokeMakeEnumerationWithRange(self, startob, endob);
		}
	}	break;

	case 3: {
		cb      = argv[0];
		startob = argv[1];
		endob   = argv[2];
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
			result = DeeSeq_EnumerateWithIntRange(self, cb, start, end);
		} else {
			result = DeeSeq_EnumerateWithRange(self, cb, startob, endob);
		}
	}	break;

	default:
		goto err_bad_args;
	}
	if unlikely(DeeKwArgs_Done(&kwds, argc, "enumerate"))
		goto err_r;
	return result;
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
	goto err;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_enumerate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start, end;
	if unlikely(kw)
		return do_seq_enumerate_with_kw(self, argc, argv, kw);
	if likely(argc == 0)
		return DeeSeq_InvokeMakeEnumeration(self);
	if (DeeCallable_Check(argv[0])) {
		if (argc == 1)
			return DeeSeq_Enumerate(self, argv[0]);
		if unlikely(argc == 2) {
			if (DeeObject_AsSize(argv[1], &start))
				goto err;
			return DeeSeq_EnumerateWithIntRange(self, argv[0], start, (size_t)-1);
		}
		if (argc != 3)
			goto err_bad_args;
		if ((DeeInt_Check(argv[1]) && DeeInt_Check(argv[2])) &&
		    (DeeInt_TryAsSize(argv[1], &start) && DeeInt_TryAsSize(argv[2], &end)))
			return DeeSeq_EnumerateWithIntRange(self, argv[0], start, end);
		return DeeSeq_EnumerateWithRange(self, argv[0], argv[1], argv[2]);
	} else {
		if unlikely(argc == 1) {
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			return DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, (size_t)-1);
		}
		if (argc != 2)
			goto err_bad_args;
		if ((DeeInt_Check(argv[0]) && DeeInt_Check(argv[1])) &&
		    (DeeInt_TryAsSize(argv[0], &start) && DeeInt_TryAsSize(argv[1], &end)))
			return DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end);
		return DeeSeq_InvokeMakeEnumerationWithRange(self, argv[0], argv[1]);
	}
	__builtin_unreachable();
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_any(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:any",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAnyWithKey(self, key)
		         : DeeSeq_InvokeAny(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAnyWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeAnyWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_all(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:all",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAllWithKey(self, key)
		         : DeeSeq_InvokeAll(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAllWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeAllWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_parity(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:parity",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeParityWithKey(self, key)
		         : DeeSeq_InvokeParity(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeParityWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeParityWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_reduce(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *combine, *init = NULL;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__combine_start_end_init,
	                    "o|" UNPuSIZ UNPuSIZ "o:reduce",
	                    &combine, &start, &end, &init))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (init)
			return DeeSeq_InvokeReduceWithInit(self, combine, init);
		return DeeSeq_InvokeReduce(self, combine);
	}
	if (init)
		return DeeSeq_InvokeReduceWithRangeAndInit(self, combine, start, end, init);
	return DeeSeq_InvokeReduceWithRange(self, combine, start, end);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_min(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:min",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key))
			return DeeSeq_InvokeMin(self);
		return DeeSeq_InvokeMinWithKey(self, key);
	}
	if (DeeNone_Check(key))
		return DeeSeq_InvokeMinWithRange(self, start, end);
	return DeeSeq_InvokeMinWithRangeAndKey(self, start, end, key);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_max(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:max",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key))
			return DeeSeq_InvokeMax(self);
		return DeeSeq_InvokeMaxWithKey(self, key);
	}
	if (DeeNone_Check(key))
		return DeeSeq_InvokeMaxWithRange(self, start, end);
	return DeeSeq_InvokeMaxWithRangeAndKey(self, start, end, key);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_sum(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":sum",
	                    &start, &end))
		goto err;
	if (start == 0 && end == (size_t)-1)
		return DeeSeq_InvokeSum(self);
	return DeeSeq_InvokeSumWithRange(self, start, end);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_count(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:count",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeCount(self, item);
		} else {
			result = DeeSeq_InvokeCountWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeCountWithRange(self, item, start, end);
		} else {
			result = DeeSeq_InvokeCountWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_contains(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:contains",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeContains(self, item);
		} else {
			result = DeeSeq_InvokeContainsWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeContainsWithRange(self, item, start, end);
		} else {
			result = DeeSeq_InvokeContainsWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_locate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:locate",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key))
			return DeeSeq_InvokeLocate(self, item);
		return DeeSeq_InvokeLocateWithKey(self, item, key);
	}
	if (DeeNone_Check(key))
		return DeeSeq_InvokeLocateWithRange(self, item, start, end);
	return DeeSeq_InvokeLocateWithRangeAndKey(self, item, start, end, key);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_rlocate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rlocate",
	                    &item, &start, &end, &key))
		goto err;
	if (DeeNone_Check(key))
		return DeeSeq_InvokeRLocateWithRange(self, item, start, end);
	return DeeSeq_InvokeRLocateWithRangeAndKey(self, item, start, end, key);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_startswith(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:startswith",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeStartsWith(self, item);
		} else {
			result = DeeSeq_InvokeStartsWithWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeStartsWithWithRange(self, item, start, end);
		} else {
			result = DeeSeq_InvokeStartsWithWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_endswith(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:endswith",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeEndsWith(self, item);
		} else {
			result = DeeSeq_InvokeEndsWithWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeEndsWithWithRange(self, item, start, end);
		} else {
			result = DeeSeq_InvokeEndsWithWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}



PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_find(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:find",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeFind(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_reference_(DeeInt_MinusOne);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_rfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rfind",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeRFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeRFind(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_reference_(DeeInt_MinusOne);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_erase(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_count,
	                    UNPuSIZ "|" UNPuSIZ ":erase",
	                    &index, &count))
		goto err;
	if unlikely(DeeSeq_InvokeErase(self, index, count))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_insert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_item,
	                    UNPuSIZ "o:insert",
	                    &index, &item))
		goto err;
	if unlikely(DeeSeq_InvokeInsert(self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_insertall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_items,
	                    UNPuSIZ "o:insertall",
	                    &index, &items))
		goto err;
	if unlikely(DeeSeq_InvokeInsertAll(self, index, items))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_pushfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:pushfront", &item))
		goto err;
	if unlikely(DeeSeq_InvokePushFront(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_append(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:append", &item))
		goto err;
	if unlikely(DeeSeq_InvokeAppend(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_extend(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:extend", &items))
		goto err;
	if unlikely(DeeSeq_InvokeExtend(self, items))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_xchitem(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *value;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_value,
	                    UNPuSIZ "o:xchitem", &index, &value))
		goto err;
	return DeeSeq_InvokeXchItemIndex(self, index, value);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_clear(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	if unlikely(DeeSeq_InvokeClear(self))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_pop(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t index = -1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index,
	                    "|" UNPdSIZ ":pop", &index))
		goto err;
	return DeeSeq_InvokePop(self, index);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_remove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:remove",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeRemoveWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeRemove(self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_rremove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rremove",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeRRemoveWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeRRemove(self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_removeall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_max_key,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ "o:removeall",
	                    &item, &start, &end, &max, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeRemoveAllWithKey(self, item, start, end, max, key)
	         : DeeSeq_InvokeRemoveAll(self, item, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_removeif(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *should;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__should_start_end_max,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ ":removeif",
	                    &should, &start, &end, &max))
		goto err;
	result = DeeSeq_InvokeRemoveIf(self, should, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_resize(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__size_filler,
	                    UNPuSIZ "|o:resize", &size, &filler))
		goto err;
	if unlikely(DeeSeq_InvokeResize(self, size, filler))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_fill(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_filler,
	                    "|" UNPuSIZ UNPuSIZ "o:fill",
	                    &start, &end, &filler))
		goto err;
	if unlikely(DeeSeq_InvokeFill(self, start, end, filler))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_reverse(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reverse",
	                    &start, &end))
		goto err;
	if unlikely(DeeSeq_InvokeReverse(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_reversed(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reversed",
	                    &start, &end))
		goto err;
	return DeeSeq_InvokeReversed(self, start, end);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_sort(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sort",
	                    &start, &end, &key))
		goto err;
	if unlikely(!DeeNone_Check(key)
	            ? DeeSeq_InvokeSortWithKey(self, start, end, key)
	            : DeeSeq_InvokeSort(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_sorted(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sorted",
	                    &start, &end, &key))
		goto err;
	return !DeeNone_Check(key)
	       ? DeeSeq_InvokeSortedWithKey(self, start, end, key)
	       : DeeSeq_InvokeSorted(self, start, end);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_bfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:bfind",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeBFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeBFind(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_bposition(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:bposition",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeBPositionWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeBPosition(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_brange(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, result_range[2];
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:brange",
	                    &item, &start, &end, &key))
		goto err;
	if (!DeeNone_Check(key)
	    ? DeeSeq_InvokeBRangeWithKey(self, item, start, end, key, result_range)
	    : DeeSeq_InvokeBRange(self, item, start, end, result_range))
		goto err;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, result_range[0], result_range[1]);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_seq_blocate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:blocate",
	                    &item, &start, &end, &key))
		goto err;
	return !DeeNone_Check(key)
	       ? DeeSeq_InvokeBLocateWithKey(self, item, start, end, key)
	       : DeeSeq_InvokeBLocate(self, item, start, end);
err:
	return NULL;
}


/* Default set function pointers (including ones for mutable sets). */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_set_insert(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:insert", &key))
		goto err;
	result = DeeSet_InvokeInsert(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_set_remove(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:remove", &key))
		goto err;
	result = DeeSet_InvokeRemove(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_set_insertall(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:insertall", &keys))
		goto err;
	if unlikely(DeeSet_InvokeInsertAll(self, keys))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_set_removeall(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:removeall", &keys))
		goto err;
	if unlikely(DeeSet_InvokeRemoveAll(self, keys))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_set_unify(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:unify", &key))
		goto err;
	return DeeSet_InvokeUnify(self, key);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_set_pop(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *def = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:pop", &def))
		goto err;
	return def ? DeeSet_InvokePopWithDefault(self, def)
	           : DeeSet_InvokePop(self);
err:
	return NULL;
}



PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_setold(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:setold", &key, &value))
		goto err;
	result = DeeMap_InvokeSetOld(self, key, value);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE DEFINE_TUPLE(setold_failed_result, 2, { Dee_False, Dee_None });

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_setold_ex(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value;
	DREF DeeObject *old_value;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo:setold_ex", &key, &value))
		goto err;
	old_value = DeeMap_InvokeSetOldEx(self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return_reference_((DeeObject *)&setold_failed_result);
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_old_value;
	Dee_Incref(Dee_True);
	DeeTuple_SET(result, 0, Dee_True);
	DeeTuple_SET(result, 1, old_value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_setnew(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:setnew", &key, &value))
		goto err;
	result = DeeMap_InvokeSetNew(self, key, value);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE DEFINE_TUPLE(setnew_success_result, 2, { Dee_True, Dee_None });

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_setnew_ex(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value;
	DREF DeeObject *old_value;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo:setnew_ex", &key, &value))
		goto err;
	old_value = DeeMap_InvokeSetNewEx(self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return_reference_((DeeObject *)&setnew_success_result);
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_old_value;
	Dee_Incref(Dee_False);
	DeeTuple_SET(result, 0, Dee_False);
	DeeTuple_SET(result, 1, old_value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_setdefault(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:setdefault", &key, &value))
		goto err;
	return DeeMap_InvokeSetDefault(self, key, value);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_update(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:update", &items))
		goto err;
	if unlikely(DeeMap_InvokeUpdate(self, items))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_remove(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:remove", &key))
		goto err;
	result = DeeMap_InvokeRemove(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_removekeys(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:removekeys", &keys))
		goto err;
	if unlikely(DeeMap_InvokeRemoveKeys(self, keys))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_pop(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:pop", &key, &def))
		goto err;
	return likely(def) ? DeeMap_InvokePopWithDefault(self, key, def)
	                   : DeeMap_InvokePop(self, key);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMH_map_popitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popitem"))
		goto err;
	return DeeMap_InvokePopItem(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL default_map_keys)(DeeObject *self) {
	maybe_cache_optimized_map_keys_in_membercache(Dee_TYPE(self));
	return DeeMap_InvokeKeys(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL default_map_values)(DeeObject *self) {
	maybe_cache_optimized_map_values_in_membercache(Dee_TYPE(self));
	return DeeMap_InvokeValues(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL default_map_iterkeys)(DeeObject *self) {
	maybe_cache_optimized_map_iterkeys_in_membercache(Dee_TYPE(self));
	return DeeMap_InvokeIterKeys(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL default_map_itervalues)(DeeObject *self) {
	maybe_cache_optimized_map_itervalues_in_membercache(Dee_TYPE(self));
	return DeeMap_InvokeIterValues(self);
}





#if 0
PRIVATE struct type_getset tpconst DeeMH_seq_getsets[] = {
	TYPE_GETSET_BOUND(STR_first, &default_seq_getfirst, &default_seq_delfirst, &default_seq_setfirst, &default_seq_boundfirst, "->"),
	TYPE_GETSET_BOUND(STR_last, &default_seq_getlast, &default_seq_dellast, &default_seq_setlast, &default_seq_boundlast, "->"),
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst DeeMH_seq_methods[] = {
	TYPE_KWMETHOD(STR_any, &DeeMH_seq_enumerate, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?S?T2?Dint?O\n"
	                                               "(cb:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N"),
	TYPE_KWMETHOD(STR_any, &DeeMH_seq_any, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_all, &DeeMH_seq_all, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_parity, &DeeMH_seq_parity, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_reduce, &DeeMH_seq_reduce, "(combine:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,init?)->"),
	TYPE_KWMETHOD(STR_min, &DeeMH_seq_min, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N"),
	TYPE_KWMETHOD(STR_max, &DeeMH_seq_max, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N"),
	TYPE_KWMETHOD(STR_sum, &DeeMH_seq_sum, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N"),
	TYPE_KWMETHOD(STR_count, &DeeMH_seq_count, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_contains, &DeeMH_seq_contains, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_locate, &DeeMH_seq_locate, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->"),
	TYPE_KWMETHOD(STR_rlocate, &DeeMH_seq_rlocate, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->"),
	TYPE_KWMETHOD(STR_startswith, &DeeMH_seq_startswith, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_endswith, &DeeMH_seq_endswith, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_find, &DeeMH_seq_find, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_rfind, &DeeMH_seq_rfind, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_reversed, &DeeMH_seq_reversed, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?DSequence"),
	TYPE_KWMETHOD(STR_sorted, &DeeMH_seq_sorted, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?DSequence"),
	TYPE_KWMETHOD(STR_insert, &DeeMH_seq_insert, "(index:?Dint,item)"),
	TYPE_KWMETHOD(STR_insertall, &DeeMH_seq_insertall, "(index:?Dint,items:?DSequence)"),
	TYPE_METHOD(STR_append, &DeeMH_seq_append, "(item)"),
	TYPE_METHOD(STR_extend, &DeeMH_seq_extend, "(items:?DSequence)"),
	TYPE_KWMETHOD(STR_erase, &DeeMH_seq_erase, "(index:?Dint,count=!1)"),
	TYPE_KWMETHOD(STR_xchitem, &DeeMH_seq_xchitem, "(index:?Dint,value)->"),
	TYPE_KWMETHOD(STR_pop, &DeeMH_seq_pop, "(index=!-1)->"),
	TYPE_METHOD(STR_popfront, &DeeMH_seq_popfront, "->"),
	TYPE_METHOD(STR_popback, &DeeMH_seq_popback, "->"),
	TYPE_METHOD(STR_pushfront, &DeeMH_seq_pushfront, "(item)"),
	TYPE_KWMETHOD(STR_remove, &DeeMH_seq_remove, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_rremove, &DeeMH_seq_rremove, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_removeall, &DeeMH_seq_removeall, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_removeif, &DeeMH_seq_removeif, "(should:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint"),
	TYPE_METHOD(STR_clear, &DeeMH_seq_clear, "()"),
	TYPE_KWMETHOD(STR_resize, &DeeMH_seq_resize, "(size:?Dint,filler=!N)"),
	TYPE_KWMETHOD(STR_fill, &DeeMH_seq_fill, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,filler=!N)"),
	TYPE_KWMETHOD(STR_reverse, &DeeMH_seq_reverse, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)"),
	TYPE_KWMETHOD(STR_sort, &DeeMH_seq_sort, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)"),
	TYPE_KWMETHOD(STR_bfind, &DeeMH_seq_bfind, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_bposition, &DeeMH_seq_bposition, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_brange, &DeeMH_seq_brange, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?Dint?Dint"),
	TYPE_KWMETHOD(STR_blocate, &DeeMH_seq_blocate, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->"),
	TYPE_METHOD("__bool__", &default_seq___bool__, "->?Dbool"),
	TYPE_METHOD("__iter__", &default_seq___iter__, "->?DIterator"),
	TYPE_METHOD("__size__", &default_seq___size__, "->?Dint"),
	TYPE_METHOD("__contains__", &default_seq___contains__, "(item)->?Dbool"),
	TYPE_METHOD("__getitem__", &default_seq___getitem__, "(index:?Dint)->"),
	TYPE_METHOD("__delitem__", &default_seq___delitem__, "(index:?Dint)"),
	TYPE_METHOD("__setitem__", &default_seq___setitem__, "(index:?Dint,value)"),
	TYPE_KWMETHOD("__getrange__", &default_seq___getrange__, "(start=!0,end?:?X2?N?Dint)->?S?O"),
	TYPE_KWMETHOD("__delrange__", &default_seq___delrange__, "(start=!0,end?:?X2?N?Dint)"),
	TYPE_KWMETHOD("__setrange__", &default_seq___setrange__, "(start=!0,end?:?X2?N?Dint,values:?S?O)"),
	TYPE_METHOD("__foreach__", &default_seq___foreach__, "(cb)->"),
	TYPE_METHOD("__foreach_pair__", &default_seq___foreach_pair__, "(cb)->"),
	TYPE_METHOD("__enumerate__", &default_seq___enumerate__, "(cb)->"),
	TYPE_KWMETHOD("__enumerate_index__", &default_seq___enumerate_index__, "(cb,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->"),
	TYPE_METHOD("__iterkeys__", &default_seq___iterkeys__, "->?DIterator"),
	TYPE_METHOD("__bounditem__", &default_seq___bounditem__, "(index:?Dint,allow_missing=!t)->?Dbool"),
	TYPE_METHOD("__hasitem__", &default_seq___hasitem__, "(index:?Dint)->?Dbool"),
	TYPE_METHOD("__size_fast__", &default_seq___size_fast__, "->?X2?N?Dint"),
	TYPE_METHOD("__getitem_index__", &default_seq___getitem_index__, "(index:?Dint)->"),
	TYPE_METHOD("__delitem_index__", &default_seq___delitem_index__, "(index:?Dint)"),
	TYPE_METHOD("__setitem_index__", &default_seq___setitem_index__, "(index:?Dint,value)"),
	TYPE_METHOD("__bounditem_index__", &default_seq___bounditem_index__, "(index:?Dint,allow_missing=!t)->?Dbool"),
	TYPE_METHOD("__hasitem_index__", &default_seq___hasitem_index__, "(index:?Dint)->?Dbool"),
	TYPE_KWMETHOD("__getrange_index__", &default_seq___getrange_index__, "(start=!0,end?:?X2?N?Dint)->?S?O"),
	TYPE_KWMETHOD("__delrange_index__", &default_seq___delrange_index__, "(start=!0,end?:?X2?N?Dint)"),
	TYPE_KWMETHOD("__setrange_index__", &default_seq___setrange_index__, "(start=!0,end?:?X2?N?Dint,values:?S?O)"),
	TYPE_METHOD("__trygetitem__", &default_seq___trygetitem__, "(index:?Dint,def=!N)->"),
	TYPE_METHOD("__trygetitem_index__", &default_seq___trygetitem_index__, "(index:?Dint,def=!N)->"),
	TYPE_METHOD("__hash__", &default_seq___hash__, "->?Dint"),
	TYPE_METHOD("__compare_eq__", &default_seq___compare_eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__compare__", &default_seq___compare__, "(rhs:?S?O)->?Dint"),
	TYPE_METHOD("__trycompare_eq__", &default_seq___trycompare_eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__eq__", &default_seq___eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__ne__", &default_seq___ne__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__lo__", &default_seq___lo__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__le__", &default_seq___le__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__gr__", &default_seq___gr__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__ge__", &default_seq___ge__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__inplace_add__", &default_seq___inplace_add__, "(rhs:?S?O)->?."),
	TYPE_METHOD("__inplace_mul__", &default_seq___inplace_mul__, "(factor:?Dint)->?."),
	TYPE_METHOD_END
};

PRIVATE struct type_method tpconst DeeMH_set_methods[] = {
	TYPE_METHOD(STR_insert, &DeeMH_set_insert, "(key)->?Dbool"),
	TYPE_METHOD(STR_remove, &DeeMH_set_remove, "(key)->?Dbool"),
	TYPE_METHOD(STR_insertall, &DeeMH_set_insertall, "(keys:?S?O)"),
	TYPE_METHOD(STR_removeall, &DeeMH_set_removeall, "(keys:?S?O)"),
	TYPE_METHOD(STR_unify, &DeeMH_set_unify, "(key)->"),
	TYPE_METHOD(STR_pop, &DeeMH_set_pop, "(def?)->"),
	TYPE_METHOD("__iter__", &default_set___iter__, "->?DIterator"),
	TYPE_METHOD("__size__", &default_set___size__, "->?Dint"),
	TYPE_METHOD("__foreach__", &default_set___foreach__, "(cb)->"),
	TYPE_METHOD("__foreach_pair__", &default_set___foreach_pair__, "(cb)->"),
	TYPE_METHOD("__hash__", &default_set___hash__, "->?Dint"),
	TYPE_METHOD("__compare_eq__", &default_set___compare_eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__trycompare_eq__", &default_set___trycompare_eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__eq__", &default_set___eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__ne__", &default_set___ne__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__lo__", &default_set___lo__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__le__", &default_set___le__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__gr__", &default_set___gr__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__ge__", &default_set___ge__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD_END
};

PRIVATE struct type_method tpconst DeeMH_map_methods[] = {
	TYPE_METHOD(STR_get, &DeeMH_map_get, "(key,def)->"),
	TYPE_METHOD(STR_setold, &DeeMH_map_setold, "(key,value)->?Dbool"),
	TYPE_METHOD(STR_setold_ex, &DeeMH_map_setold_ex, "(key,value)->?T2?Dbool?X2?O?N"),
	TYPE_METHOD(STR_setnew, &DeeMH_map_setnew, "(key,value)->?Dbool"),
	TYPE_METHOD(STR_setnew_ex, &DeeMH_map_setnew_ex, "(key,value)->?T2?Dbool?X2?O?N"),
	TYPE_METHOD(STR_setdefault, &DeeMH_map_setdefault, "(key,value)->"),
	TYPE_METHOD(STR_update, &DeeMH_map_update, "(items:?M?O?O)"),
	TYPE_METHOD(STR_remove, &DeeMH_map_remove, "(key)->?Dbool"),
	TYPE_METHOD(STR_removekeys, &DeeMH_map_removekeys, "(keys:?S?O)"),
	TYPE_METHOD(STR_pop, &DeeMH_map_pop, "(key,def?)->"),
	TYPE_METHOD(STR_popitem, &DeeMH_map_popitem, "->?X2?T2?O?O?N"),
	TYPE_METHOD("__contains__", &default_map___contains__, "(item)->?Dbool"),
	TYPE_METHOD("__getitem__", &default_map___getitem__, "(key)->"),
	TYPE_METHOD("__delitem__", &default_map___delitem__, "(key)"),
	TYPE_METHOD("__setitem__", &default_map___setitem__, "(key,value)"),
	TYPE_METHOD("__enumerate__", &default_map___enumerate__, "(cb)->"),
	TYPE_KWMETHOD("__enumerate_index__", &default_map___enumerate_index__, "(cb,start=!0,end=!N)->"),
	TYPE_METHOD("__iterkeys__", &default_map___iterkeys__, "->?DIterator"),
	TYPE_METHOD("__bounditem__", &default_map___bounditem__, "(key,allow_missing=!t)->?Dbool"),
	TYPE_METHOD("__hasitem__", &default_map___hasitem__, "(key)->?Dbool"),
	TYPE_METHOD("__getitem_index__", &default_map___getitem_index__, "(key:?Dint)->"),
	TYPE_METHOD("__delitem_index__", &default_map___delitem_index__, "(key:?Dint)"),
	TYPE_METHOD("__setitem_index__", &default_map___setitem_index__, "(key:?Dint,value)"),
	TYPE_METHOD("__bounditem_index__", &default_map___bounditem_index__, "(key:?Dint,allow_missing=!t)->?Dbool"),
	TYPE_METHOD("__hasitem_index__", &default_map___hasitem_index__, "(key:?Dint)->?Dbool"),
	TYPE_METHOD("__trygetitem__", &default_map___trygetitem__, "(key,def=!N)->"),
	TYPE_METHOD("__trygetitem_index__", &default_map___trygetitem_index__, "(key:?Dint,def=!N)->"),
	TYPE_METHOD("__trygetitem_string__", &default_map___trygetitem_string__, "(key:?X2?DBytes?Dstring,def=!N)->"),
	TYPE_METHOD("__getitem_string__", &default_map___getitem_string__, "(key:?X2?DBytes?Dstring)->"),
	TYPE_METHOD("__delitem_string__", &default_map___delitem_string__, "(key:?X2?DBytes?Dstring)"),
	TYPE_METHOD("__setitem_string__", &default_map___setitem_string__, "(key:?X2?DBytes?Dstring,value)"),
	TYPE_METHOD("__bounditem_string__", &default_map___bounditem_string__, "(key:?X2?DBytes?Dstring,allow_missing=!t)->?Dbool"),
	TYPE_METHOD("__hasitem_string__", &default_map___hasitem_string__, "(key:?X2?DBytes?Dstring)->?Dbool"),
	TYPE_METHOD("__hash__", &default_map___hash__, "->?Dint"),
	TYPE_METHOD("__compare_eq__", &default_map___compare_eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__trycompare_eq__", &default_map___trycompare_eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__eq__", &default_map___eq__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__ne__", &default_map___ne__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__lo__", &default_map___lo__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__le__", &default_map___le__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__gr__", &default_map___gr__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD("__ge__", &default_map___ge__, "(rhs:?S?O)->?Dbool"),
	TYPE_METHOD_END
};
#endif


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
call_getter_for_bound(DeeObject *getter, DeeObject *self) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(getter, self, 0, NULL);
	if (result) {
		Dee_Decref(result);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundAttribute))
		return 0;
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
call_delete(DeeObject *delete_, DeeObject *self) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(delete_, self, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
call_setter(DeeObject *setter, DeeObject *self, DeeObject *value) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(setter, self, 1, &value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}


DECL_END

/* Define attribute proxy implementations */
#ifndef __INTELLISENSE__
#define DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#include "default-api-methods-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction
#include "default-api-methods-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod
#include "default-api-methods-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod
#include "default-api-methods-attrproxy-impl.c.inl"
#endif /* !__INTELLISENSE__ */

/* Define implementations of bsearch functions */
#ifndef __INTELLISENSE__
#define DEFINE_DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBLocateWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#endif /* !__INTELLISENSE__ */

/* Define implementations of misc. functions */
#ifndef __INTELLISENSE__
#include "default-api-methods-first.c.inl"
#include "default-api-methods-last.c.inl"
#include "default-api-operators.c.inl"
#include "default-api-operator-methods.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_METHODS_C */
