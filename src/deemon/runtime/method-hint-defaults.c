/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_C 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/class.h>
#include <deemon/format.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/overflow.h>
#include <hybrid/typecore.h>

/**/
#include "../objects/seq/default-compare.h"
#include "../objects/seq/default-sequences.h"
#include "../objects/seq/range.h"
#include "method-hint-defaults.h"
#include "method-hints.h"
#include "runtime_error.h"
#include "strings.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

#define do_fix_negative_range_index(index, size) \
	((size) - ((size_t)(-(index)) % (size)))

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



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
call_getter_for_bound(DeeObject *getter, DeeObject *self) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(getter, self, 0, NULL);
	if (result) {
		Dee_Decref(result);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundAttribute))
		return Dee_BOUND_NO;
	return Dee_BOUND_ERR;
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




#ifndef Dee_int_SIZE_MAX_DEFINED
#define Dee_int_SIZE_MAX_DEFINED
#if __SIZEOF_SIZE_T__ == 4
DEFINE_UINT32(Dee_int_SIZE_MAX, (uint32_t)-1);
#elif __SIZEOF_SIZE_T__ == 8
DEFINE_UINT64(Dee_int_SIZE_MAX, (uint64_t)-1);
#elif __SIZEOF_SIZE_T__ == 2
DEFINE_UINT16(Dee_int_SIZE_MAX, (uint16_t)-1);
#elif __SIZEOF_SIZE_T__ == 1
DEFINE_UINT8(Dee_int_SIZE_MAX, (uint8_t)-1);
#elif !defined(__DEEMON__)
#error "Unsupported __SIZEOF_SIZE_T__"
#endif /* __SIZEOF_SIZE_T__ != ... */
#endif /* !Dee_int_SIZE_MAX_DEFINED */





/* clang-format off */
/*[[[deemon (printDefaultImpls from "..method-hints.method-hints")();]]]*/
/* seq_operator_bool */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with_callattr___seq_bool__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_bool__, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with_callobjectcache___seq_bool__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_object, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with_callmethodcache___seq_bool__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_method, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with_callkwmethodcache___seq_bool__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__unsupported(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "operator bool()");
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_cb(void *arg, DeeObject *elem);
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__seq_operator_foreach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__seq_operator_size(DeeObject *__restrict self) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return size != 0;
err:
	return -1;
}


/* seq_operator_sizeob */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with_callattr___seq_size__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_size__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with_callobjectcache___seq_size__(DeeObject *self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_size__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with_callmethodcache___seq_size__(DeeObject *self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_size__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with_callkwmethodcache___seq_size__(DeeObject *self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_size__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__unsupported(DeeObject *self) {
	err_seq_unsupportedf(self, "operator size()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__empty(DeeObject *self) { return_reference_(DeeInt_Zero); }

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with__seq_operator_size(DeeObject *self) {
	size_t seqsize = DeeSeq_OperatorSize(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(seqsize);
err:
	return NULL;
}


/* seq_operator_size */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__unsupported(DeeObject *self) {
	return (size_t)err_seq_unsupportedf(self, "operator size()");
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__empty(DeeObject *self) {
	return 0;
}

INTDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_cb(void *arg, DeeObject *elem);
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__seq_operator_foreach(DeeObject *self) {
	return (size_t)DeeSeq_OperatorForeach(self, &default_seq_size_with_foreach_cb, NULL);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__seq_operator_sizeob(DeeObject *self) {
	size_t result;
	DREF DeeObject *sizeob;
	sizeob = DeeSeq_OperatorSizeOb(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}


/* seq_operator_iter */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with_callattr___seq_iter__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_iter__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with_callobjectcache___seq_iter__(DeeObject *__restrict self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_iter__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with_callmethodcache___seq_iter__(DeeObject *__restrict self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_iter__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with_callkwmethodcache___seq_iter__(DeeObject *__restrict self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_iter__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "operator iter()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__empty(DeeObject *__restrict self) { return_empty_iterator; }


/* seq_operator_foreach */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = DeeSeq_OperatorIter(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, proc, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}


/* seq_operator_foreach_pair */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_foreach_pair__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = DeeSeq_OperatorIter(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, proc, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}


/* seq_operator_iterkeys */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iterkeys__with_callattr___seq_iterkeys__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_iterkeys__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iterkeys__with_callobjectcache___seq_iterkeys__(DeeObject *self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_iterkeys__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iterkeys__with_callmethodcache___seq_iterkeys__(DeeObject *self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_iterkeys__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iterkeys__with_callkwmethodcache___seq_iterkeys__(DeeObject *self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_iterkeys__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iterkeys__unsupported(DeeObject *self) {
	err_seq_unsupportedf(self, "__seq_iterkeys__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iterkeys__with__seq_size(DeeObject *self) {
	size_t size;
	DREF IntRangeIterator *result;
	size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(IntRangeIterator);
	if unlikely(!result)
		goto err;
	result->iri_index = 0;
	result->iri_end   = (Dee_ssize_t)size; /* TODO: Need another range iterator type that uses unsigned indices */
	result->iri_step  = 1;
	DeeObject_Init(result, &SeqIntRangeIterator_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


/* seq_operator_enumerate */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_enumerate__, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_object, self, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_method, self, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_kwmethod, self, 1, &cb_wrapper, NULL);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__unsupported(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) { return err_seq_unsupportedf(self, "__seq_enumerate__(...)"); }

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with__seq_operator_size_and_seq_operator_getitem_index(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	DREF DeeObject *indexob, *index_value;
	size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = DeeSeq_OperatorTryGetItemIndex(self, i);
		if unlikely(!index_value)
			goto err_indexob;
		if (index_value == ITER_DONE) {
			temp = (*proc)(arg, indexob, NULL);
		} else {
			temp = (*proc)(arg, indexob, index_value);
			Dee_Decref(index_value);
		}
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with__seq_operator_sizeob_and_seq_operator_getitem(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value, *sizeob;
	sizeob = DeeSeq_OperatorSizeOb(self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	for (;;) {
		int index_is_less_than_size = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (index_is_less_than_size <= 0) {
			if unlikely(index_is_less_than_size < 0)
				goto err_sizeob_indexob;
			break;
		}
		index_value = DeeSeq_OperatorGetItem(self, indexob);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_indexob;
		}
		temp = (*proc)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_sizeob_indexob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_indexob:
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return temp;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

#ifndef DEFINED_default_enumerate_with_counter_and_foreach_cb
#define DEFINED_default_enumerate_with_counter_and_foreach_cb
struct default_enumerate_with_counter_and_foreach_data {
	Dee_enumerate_t dewcaf_proc;    /* [1..1] Wrapped callback */
	void           *dewcaf_arg;     /* [?..?] Cookie for `dewcaf_proc' */
	size_t          dewcaf_counter; /* Index of the next element that will be enumerated */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_with_counter_and_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_enumerate_with_counter_and_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with__counter_and_seq_operator_foreach(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	struct default_enumerate_with_counter_and_foreach_data data;
	data.dewcaf_proc    = proc;
	data.dewcaf_arg     = arg;
	data.dewcaf_counter = 0;
	return DeeSeq_OperatorForeach(self, &default_enumerate_with_counter_and_foreach_cb, &data);
}


/* seq_operator_enumerate_index */
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_enumerate__, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_object, self, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_method, self, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_kwmethod, self, 1, &cb_wrapper, NULL);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__unsupported(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) { return err_seq_unsupportedf(self, "__seq_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); }

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with__seq_operator_size_and_seq_operator_getitem_index(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = DeeSeq_OperatorGetItemIndex(self, i);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*proc)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

#ifndef DEFINED_default_enumerate_index_with_counter_and_foreach_cb
#define DEFINED_default_enumerate_index_with_counter_and_foreach_cb
#define default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP \
	(__SSIZE_MIN__ + 99) /* Shhht. We don't talk about this one... */

struct default_enumerate_index_with_counter_and_foreach_data {
	Dee_enumerate_index_t deiwcaf_proc;  /* [1..1] Wrapped callback */
	void                 *deiwcaf_arg;   /* [?..?] Cookie for `deiwcaf_proc' */
	size_t                deiwcaf_index; /* Index of the next element that will be enumerate_indexd */
	size_t                deiwcaf_start; /* Enumeration start index */
	size_t                deiwcaf_end;   /* Enumeration end index */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_index_with_counter_and_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_enumerate_index_with_counter_and_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with__counter_and_seq_operator_foreach(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	struct default_enumerate_index_with_counter_and_foreach_data data;
	Dee_ssize_t result;
	data.deiwcaf_proc  = proc;
	data.deiwcaf_arg   = arg;
	data.deiwcaf_index = 0;
	data.deiwcaf_start = start;
	data.deiwcaf_end   = end;
	result = DeeSeq_OperatorForeach(self, &default_enumerate_index_with_counter_and_foreach_cb, &data);
	if unlikely(result == default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP)
		result = 0;
	return result;
}


/* seq_operator_getitem */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__with_callattr___seq_getitem__(DeeObject *self, DeeObject *index) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_getitem__, 1, &index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__with_callobjectcache___seq_getitem__(DeeObject *self, DeeObject *index) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_getitem__.c_object, self, 1, &index);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__with_callmethodcache___seq_getitem__(DeeObject *self, DeeObject *index) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_getitem__.c_method, self, 1, &index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__with_callkwmethodcache___seq_getitem__(DeeObject *self, DeeObject *index) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_getitem__.c_kwmethod, self, 1, &index, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__unsupported(DeeObject *self, DeeObject *index) {
	err_seq_unsupportedf(self, "operator [](%r)", index);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__empty(DeeObject *self, DeeObject *index) {
	err_index_out_of_bounds_ob(self, index);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__with__seq_operator_getitem_index(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorGetItemIndex(self, index_value);
err:
	return NULL;
}


/* seq_operator_getitem_index */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__unsupported(DeeObject *__restrict self, size_t index) {
	err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__empty(DeeObject *__restrict self, size_t index) {
	err_index_out_of_bounds(self, index, 0);
	return NULL;
}

#ifndef DEFINED_default_getitem_index_with_foreach
#define DEFINED_default_getitem_index_with_foreach
struct default_getitem_index_with_foreach_data {
	DREF DeeObject *dgiiwfd_result; /* [?..1][out] Item lookup result */
	size_t          dgiiwfd_nskip;  /* Number of indices left to skip. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_getitem_index_with_foreach */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index) {
	struct default_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dgiiwfd_nskip = index;
	status = DeeSeq_OperatorForeach(self, &default_getitem_index_with_foreach_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	return data.dgiiwfd_result;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, index - data.dgiiwfd_nskip);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__with__seq_operator_getitem(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeSeq_OperatorGetItem(self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}


/* seq_operator_trygetitem_index */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__empty(DeeObject *__restrict self, size_t index) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result = DeeSeq_OperatorGetItemIndex(self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

#ifndef DEFINED_default_getitem_index_with_foreach
#define DEFINED_default_getitem_index_with_foreach
struct default_getitem_index_with_foreach_data {
	DREF DeeObject *dgiiwfd_result; /* [?..1][out] Item lookup result */
	size_t          dgiiwfd_nskip;  /* Number of indices left to skip. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_getitem_index_with_foreach */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index) {
	struct default_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dgiiwfd_nskip = index;
	status = DeeSeq_OperatorForeach(self, &default_getitem_index_with_foreach_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	return data.dgiiwfd_result;
err_bad_bounds:
	return ITER_DONE;
err:
	return NULL;
}


/* seq_operator_trygetitem */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem__empty(DeeObject *self, DeeObject *index) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index) {
	DREF DeeObject *result = DeeSeq_OperatorGetItem(self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorTryGetItemIndex(self, index_value);
err:
	return NULL;
}


/* seq_operator_hasitem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem__unsupported(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "operator [](%r)", index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem__with__seq_operator_hasitem_index(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorHasItemIndex(self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index) {
	DREF DeeObject *value = DeeSeq_OperatorGetItem(self, index);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}


/* seq_operator_hasitem_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem_index__unsupported(DeeObject *__restrict self, size_t index) {
	return err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem_index__with__seq_operator_size(DeeObject *__restrict self, size_t index) {
	size_t seqsize = DeeSeq_OperatorSize(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return index < seqsize ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *value = DeeSeq_OperatorGetItemIndex(self, index);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}


/* seq_operator_bounditem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_bounditem__unsupported(DeeObject *self, DeeObject *index) {
	err_seq_unsupportedf(self, "operator [](%r)", index);
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_bounditem__with__seq_operator_bounditem_index(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorBoundItemIndex(self, index_value);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_bounditem__with__seq_operator_getitem(DeeObject *self, DeeObject *index) {
	DREF DeeObject *value = DeeSeq_OperatorGetItem(self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}


/* seq_operator_bounditem_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_bounditem_index__unsupported(DeeObject *__restrict self, size_t index) {
	err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_bounditem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *value = DeeSeq_OperatorGetItemIndex(self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}


/* seq_operator_delitem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__with_callattr___seq_delitem__(DeeObject *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_delitem__, 1, &index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__with_callobjectcache___seq_delitem__(DeeObject *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__.c_object, self, 1, &index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__with_callmethodcache___seq_delitem__(DeeObject *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__.c_method, self, 1, &index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__with_callkwmethodcache___seq_delitem__(DeeObject *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__.c_kwmethod, self, 1, &index, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__unsupported(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "operator del[](%r)", index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__empty(DeeObject *self, DeeObject *index) {
	return err_index_out_of_bounds_ob(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__with__seq_operator_delitem_index(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorDelItemIndex(self, index_value);
err:
	return -1;
}


/* seq_operator_delitem_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem_index__with_callattr___seq_delitem__(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_delitem__, PCKuSIZ, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem_index__with_callobjectcache___seq_delitem__(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__.c_object, self, PCKuSIZ, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem_index__with_callmethodcache___seq_delitem__(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__.c_method, self, PCKuSIZ, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem_index__with_callkwmethodcache___seq_delitem__(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__.c_kwmethod, self, PCKuSIZ, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem_index__unsupported(DeeObject *__restrict self, size_t index) {
	return err_seq_unsupportedf(self, "operator del[](%" PRFuSIZ ")", index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem_index__empty(DeeObject *__restrict self, size_t index) {
	return err_index_out_of_bounds(self, index, 0);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase(DeeObject *__restrict self, size_t index) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	return DeeSeq_InvokeErase(self, index, 1);
err_oob:
	err_index_out_of_bounds(self, index, size);
err:
	return -1;
}


/* seq_operator_setitem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__with_callattr___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_setitem__, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__with_callobjectcache___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__.c_object, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__with_callmethodcache___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__.c_method, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__with_callkwmethodcache___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__.c_kwmethod, self, 2, args, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__unsupported(DeeObject *self, DeeObject *index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []=(%r, %r)", index, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__empty(DeeObject *self, DeeObject *index, DeeObject *value) {
	return err_index_out_of_bounds_ob(self, index);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__with__seq_operator_setitem_index(DeeObject *self, DeeObject *index, DeeObject *value) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorSetItemIndex(self, index_value, value);
err:
	return -1;
}


/* seq_operator_setitem_index */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem_index__with_callattr___seq_setitem__(DeeObject *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_setitem__, PCKuSIZ "o", index, value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem_index__with_callobjectcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__.c_object, self, PCKuSIZ "o", index, value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem_index__with_callmethodcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__.c_method, self, PCKuSIZ "o", index, value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem_index__with_callkwmethodcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__.c_kwmethod, self, PCKuSIZ "o", index, value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem_index__unsupported(DeeObject *self, size_t index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []=(%" PRFuSIZ ", %r)", index, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem_index__empty(DeeObject *self, size_t index, DeeObject *value) {
	return err_index_out_of_bounds(self, index, 0);
}


/* seq_operator_getrange */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__with_callattr___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_getrange__, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__with_callobjectcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_getrange__.c_object, self, 2, args);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__with_callmethodcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_getrange__.c_method, self, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__with_callkwmethodcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_getrange__.c_kwmethod, self, 2, args, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_seq_unsupportedf(self, "operator [:](%r, %r)", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__empty(DeeObject *self, DeeObject *start, DeeObject *end) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeSeq_OperatorGetRangeIndexN(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeSeq_OperatorGetRangeIndex(self, start_index, end_index);
err:
	return NULL;
}


/* seq_operator_getrange_index */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	err_seq_unsupportedf(self, "operator [:](%" PRFdSIZ ", %" PRFdSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *startob, *endob, *result;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeSeq_OperatorGetRange(self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	struct Dee_seq_range range;
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	struct Dee_seq_range range;
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeAndGetItem *result;
	struct Dee_seq_range range;
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = DeeInt_NewSize(range.sr_start);
	if unlikely(!result->dssg_start)
		goto err_r;
	result->dssg_end = DeeInt_NewSize(range.sr_end);
	if unlikely(!result->dssg_end)
		goto err_r_start;
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return (DREF DeeObject *)result;
err_r_start:
	Dee_Decref(result->dssg_start);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithIterAndLimit *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = DeeSeq_OperatorSize(self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		goto empty_seq;
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsial_seq     = self;
	result->dsial_start   = range.sr_start;
	result->dsial_limit   = range.sr_end - range.sr_start;
	result->dsial_tp_iter = DeeType_RequireSeqOperatorIter(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return (DREF DeeObject *)result;
empty_seq:
	return_empty_seq;
err:
	return NULL;
}


/* seq_operator_getrange_index_n */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start) {
	err_seq_unsupportedf(self, "operator [:](%" PCKdSIZ ", none)", start);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__empty(DeeObject *self, Dee_ssize_t start) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start) {
	DREF DeeObject *startob, *result;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeSeq_OperatorGetRange(self, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getrange_index(DeeObject *self, Dee_ssize_t start) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeSeq_OperatorGetRangeIndex(self, start, (Dee_ssize_t)size);
empty_range:
	return_empty_seq;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = (size_t)start;
	result->dssgi_end              = size;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = (size_t)start;
	result->dssgi_end              = size;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItem *result;
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = DeeInt_NewSize((size_t)start);
	if unlikely(!result->dssg_start)
		goto err_r;
	result->dssg_end = DeeInt_NewSize(size);
	if unlikely(!result->dssg_end)
		goto err_r_start;
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err_r_start:
	Dee_Decref(result->dssg_start);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start) {
	return default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter(self, start, (size_t)-1);
}


/* seq_operator_delrange */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__with_callattr___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_delrange__, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__with_callobjectcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_delrange__.c_object, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__with_callmethodcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_delrange__.c_method, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__with_callkwmethodcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_delrange__.c_kwmethod, self, 2, args, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end) {
	return err_seq_unsupportedf(self, "operator del[:](%r, %r)", start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__with__seq_operator_delrange_index__and__seq_operator_delrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeSeq_OperatorDelRangeIndexN(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeSeq_OperatorDelRangeIndex(self, start_index, end_index);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__with__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end) {
	return DeeSeq_OperatorSetRange(self, start, end, Dee_None);
}


/* seq_operator_delrange_index */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return err_seq_unsupportedf(self, "operator del[:](%" PRFdSIZ ", %" PRFdSIZ ")", start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange_index__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeSeq_OperatorDelRange(self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeSeq_InvokeErase(self, (size_t)start, size - (size_t)start);
empty_range:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange_index__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return DeeSeq_OperatorSetRangeIndex(self, start, end, Dee_None);
}


/* seq_operator_delrange_index_n */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start) {
	return err_seq_unsupportedf(self, "operator del[:](%" PCKdSIZ ", none)", start);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delrange_index_n__with__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeSeq_OperatorDelRangeIndex(self, start, (Dee_ssize_t)size);
empty_range:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delrange_index_n__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start) {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeSeq_OperatorDelRange(self, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start) {
	return DeeSeq_OperatorSetRangeIndexN(self, start, Dee_None);
}


/* seq_operator_setrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with_callattr___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = values;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_setrange__, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with_callobjectcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = values;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_setrange__.c_object, self, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with_callmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = values;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_setrange__.c_method, self, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with_callkwmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = values;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_setrange__.c_kwmethod, self, 3, args, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	return err_seq_unsupportedf(self, "operator [:]=(%r, %r, %r)", start, end, values);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__empty(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	int values_empty = DeeSeq_OperatorBool(values);
	if unlikely(values_empty < 0)
		goto err;
	if (values_empty)
		return 0;
	return default__seq_operator_setrange__unsupported(self, start, end, values);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeSeq_OperatorSetRangeIndexN(self, start_index, values);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeSeq_OperatorSetRangeIndex(self, start_index, end_index, values);
err:
	return -1;
}


/* seq_operator_setrange_index */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	return err_seq_unsupportedf(self, "operator [:]=(%" PRFdSIZ ", %" PRFdSIZ ", %r)", start, end, values);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	int values_empty = DeeSeq_OperatorBool(values);
	if unlikely(values_empty < 0)
		goto err;
	if (values_empty)
		return 0;
	return default__seq_operator_setrange_index__unsupported(self, start, end, values);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange_index__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeSeq_OperatorSetRange(self, startob, endob, values);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	struct Dee_seq_range range;
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	if (range.sr_end > range.sr_start) {
		/* Erase what was there before... */
		if unlikely(DeeSeq_InvokeErase(self, range.sr_start, range.sr_end - range.sr_start))
			goto err;
	}
	/* Insert new values. */
	return DeeSeq_InvokeInsertAll(self, range.sr_start, values);
err:
	return -1;
}


/* seq_operator_setrange_index_n */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	return err_seq_unsupportedf(self, "operator [:]=(%" PCKdSIZ ", none, %r)", start, values);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setrange_index_n__empty(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	int values_empty = DeeSeq_OperatorBool(values);
	if unlikely(values_empty < 0)
		goto err;
	if (values_empty)
		return 0;
	return default__seq_operator_setrange_index_n__unsupported(self, start, values);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setrange_index_n__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0) {
					start = 0;
				} else {
					start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
				}
			}
		}
	}
	return DeeSeq_OperatorSetRangeIndex(self, start, (Dee_ssize_t)size, values);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setrange_index_n__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeSeq_OperatorSetRange(self, startob, Dee_None, values);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}


/* seq_operator_hash */
#ifndef DEFINED_DeeSeq_HandleHashError
#define DEFINED_DeeSeq_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSeq_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with_callattr___seq_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(self, (DeeObject *)&str___seq_hash__, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

#ifndef DEFINED_DeeSeq_HandleHashError
#define DEFINED_DeeSeq_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSeq_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with_callobjectcache___seq_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_hash__.c_object, self, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#ifndef DEFINED_DeeSeq_HandleHashError
#define DEFINED_DeeSeq_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSeq_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with_callmethodcache___seq_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_hash__.c_method, self, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

#ifndef DEFINED_DeeSeq_HandleHashError
#define DEFINED_DeeSeq_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSeq_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with_callkwmethodcache___seq_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_hash__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSeq_HandleHashError(self);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__unsupported(DeeObject *self) {
	return (Dee_hash_t)err_seq_unsupportedf(self, "operator hash()");
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__empty(DeeObject *self) {
	return DEE_HASHOF_EMPTY_SEQUENCE;
}

#ifndef DEFINED_default_seq_hash_with_foreach_cb
#define DEFINED_default_seq_hash_with_foreach_cb
struct default_seq_hash_with_foreach_data {
	Dee_hash_t sqhwf_result;   /* Hash result (or DEE_HASHOF_EMPTY_SEQUENCE when sqhwf_nonempty=false) */
	bool       sqhwf_nonempty; /* True after the first element */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_hash_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_seq_hash_with_foreach_cb */
#ifndef DEFINED_DeeSeq_HandleHashError
#define DEFINED_DeeSeq_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSeq_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_foreach(DeeObject *self) {
	struct default_seq_hash_with_foreach_data data;
	data.sqhwf_result   = DEE_HASHOF_EMPTY_SEQUENCE;
	data.sqhwf_nonempty = false;
	if unlikely(DeeSeq_OperatorForeach(self, &default_seq_hash_with_foreach_cb, &data))
		goto err;
	return data.sqhwf_result;
err:
	return DeeSeq_HandleHashError(self);
}


/* seq_operator_compare */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with_callattr___seq_compare__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_compare__, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with_callobjectcache___seq_compare__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare__.c_object, lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with_callmethodcache___seq_compare__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare__.c_method, lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with_callkwmethodcache___seq_compare__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare__.c_kwmethod, lhs, 1, &rhs, NULL);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__unsupported(DeeObject *lhs, DeeObject *rhs) {
	return err_seq_unsupportedf(lhs, "__seq_compare__(%r)", rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__empty(DeeObject *lhs, DeeObject *rhs) {
	int rhs_nonempty = DeeSeq_OperatorBool(rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? -1 : 0;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	Dee_ssize_t (DCALL *rhs_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	Dee_ssize_t result;
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	rhs_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(rhs_tp_foreach);
	if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = DeeSeq_OperatorForeach(lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_trygetitem_index;
		result = DeeSeq_OperatorForeach(lhs, &seq_compare__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index;
		result = DeeSeq_OperatorForeach(lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = rhs;
			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;
			result = DeeSeq_OperatorForeach(lhs, &seq_compare__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				Dee_Decref(data.scf_sg_oindex);
				if unlikely(temp < 0) {
					result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPARE_FOREACH_RESULT_LESS;
				}
			} else {
				Dee_Decref(data.scf_sg_oindex);
			}
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = (*tp_rhs->tp_seq->tp_iter)(rhs);
		if unlikely(!rhs_iter)
			goto err;
		result = DeeSeq_OperatorForeach(lhs, &seq_compare__lhs_foreach__rhs_iter__cb, rhs_iter);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			DREF DeeObject *next = DeeObject_IterNext(rhs_iter);
			Dee_Decref(rhs_iter);
			if unlikely(!next)
				goto err;
			if (next != ITER_DONE) {
				Dee_Decref(next);
				result = SEQ_COMPARE_FOREACH_RESULT_LESS;
			}
		} else {
			Dee_Decref(rhs_iter);
		}
	}
	ASSERT(result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
	       result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
	       result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
	       result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
	if unlikely(result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
		goto err;
	if (result == SEQ_COMPARE_FOREACH_RESULT_LESS)
		return -1;
	if (result == SEQ_COMPARE_FOREACH_RESULT_GREATER)
		return 1;
	return 0; /* Equal */
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *rhs_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	rhs_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(rhs_tp_foreach);
	lhs_trygetitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(lhs));
	lhs_size = DeeSeq_OperatorSize(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                       rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                     rhs, rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                  rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_trygetitem_index,
		                                                                              rhs, rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_trygetitem_index;
		foreach_result = (*rhs_tp_foreach)(rhs, &seq_compare__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			result = 0;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *rhs_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	rhs_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(rhs_tp_foreach);
	lhs_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(lhs));
	lhs_size = DeeSeq_OperatorSize(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index,
		                                                                                    rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index,
		                                                                                  rhs, rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index,
		                                                                               rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index,
		                                                                           rhs, rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index;
		foreach_result = (*rhs_tp_foreach)(rhs, &seq_compare__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			result = 0;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *lhs_sizeob;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index);
	Dee_ssize_t (DCALL *rhs_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	rhs_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(rhs_tp_foreach);
	lhs_getitem = DeeType_RequireSeqOperatorGetItem(Dee_TYPE(lhs));
	lhs_sizeob = DeeSeq_OperatorSizeOb(lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                              rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                           rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                       rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		Dee_ssize_t foreach_result;
		DREF DeeObject *lhs_indexob;
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		lhs_indexob = DeeObject_NewDefault(Dee_TYPE(lhs_sizeob));
		if unlikely(!lhs_indexob)
			goto err_lhs_sizeob;
		data.scf_sg_other    = lhs;
		data.scf_sg_osize    = lhs_sizeob;
		data.scf_sg_oindex   = lhs_indexob;
		data.scf_sg_ogetitem = lhs_getitem;
		foreach_result = (*rhs_tp_foreach)(rhs, &seq_compare__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		Dee_Decref(lhs_indexob);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err_lhs_sizeob;
		result = foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ? 0 : 1;
	}
	Dee_Decref(lhs_sizeob);
	return result;
err_lhs_sizeob:
	Dee_Decref(lhs_sizeob);
	return Dee_COMPARE_ERR;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}


/* seq_operator_compare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with_callattr___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_compare_eq__, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare_eq__.c_object, lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with_callmethodcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare_eq__.c_method, lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with_callkwmethodcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare_eq__.c_kwmethod, lhs, 1, &rhs, NULL);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	return err_seq_unsupportedf(lhs, "__seq_compare_eq__(%r)", rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	Dee_ssize_t result;
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = DeeSeq_OperatorForeach(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_trygetitem_index;
		result = DeeSeq_OperatorForeach(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index;
		result = DeeSeq_OperatorForeach(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = rhs;
			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;
			result = DeeSeq_OperatorForeach(lhs, &seq_compareeq__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				Dee_Decref(data.scf_sg_oindex);
				if unlikely(temp < 0) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
				}
			} else {
				Dee_Decref(data.scf_sg_oindex);
			}
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = (*tp_rhs->tp_seq->tp_iter)(rhs);
		if unlikely(!rhs_iter)
			goto err;
		result = DeeSeq_OperatorForeach(lhs, &seq_compareeq__lhs_foreach__rhs_iter__cb, rhs_iter);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			DREF DeeObject *next = DeeObject_IterNext(rhs_iter);
			Dee_Decref(rhs_iter);
			if unlikely(!next)
				goto err;
			if (next != ITER_DONE) {
				Dee_Decref(next);
				result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
			}
		} else {
			Dee_Decref(rhs_iter);
		}
	}
	ASSERT(result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
	       result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
	       result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
	if unlikely(result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
		goto err;
	if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL)
		return 0;
	return 1;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *lhs, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict lhs, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_trygetitem_index = Dee_TYPE(lhs)->tp_seq->tp_trygetitem_index;
	lhs_size = (*Dee_TYPE(lhs)->tp_seq->tp_size)(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (tp_rhs->tp_seq->tp_size_fast != NULL) {
		size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
		if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
			return 1;
	}
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                         rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                       rhs, rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                    rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                rhs, rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_trygetitem_index;
		foreach_result = (*other_tp_foreach)(rhs, &seq_compareeq__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&
		    data.scf_sgi_oindex >= data.scf_sgi_osize) {
			result = 0;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *lhs, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict lhs, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem_index = Dee_TYPE(lhs)->tp_seq->tp_getitem_index;
	lhs_size = (*Dee_TYPE(lhs)->tp_seq->tp_size)(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (tp_rhs->tp_seq->tp_size_fast != NULL) {
		size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
		if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
			return 1;
	}
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index,
		                                                                                      rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index,
		                                                                                    rhs, rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index,
		                                                                                 rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index,
		                                                                             rhs, rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index;
		foreach_result = (*other_tp_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&
		    data.scf_sgi_oindex >= data.scf_sgi_osize) {
			result = 0;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *lhs_sizeob;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *lhs, DeeObject *index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict lhs, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem = Dee_TYPE(lhs)->tp_seq->tp_getitem;
	lhs_sizeob = (*Dee_TYPE(lhs)->tp_seq->tp_sizeob)(lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                                  rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                             rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                         rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		Dee_ssize_t foreach_result;
		DREF DeeObject *lhs_indexob;
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		lhs_indexob = DeeObject_NewDefault(Dee_TYPE(lhs_sizeob));
		if unlikely(!lhs_indexob)
			goto err_lhs_sizeob;
		data.scf_sg_other    = lhs;
		data.scf_sg_osize    = lhs_sizeob;
		data.scf_sg_oindex   = lhs_indexob;
		data.scf_sg_ogetitem = lhs_getitem;
		foreach_result = (*other_tp_foreach)(rhs, &seq_compareeq__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		lhs_indexob = data.scf_sg_oindex;
		Dee_Decref(lhs_indexob);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err_lhs_sizeob;
		result = foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ? 0 : 1;
	}
	Dee_Decref(lhs_sizeob);
	return result;
err_lhs_sizeob:
	Dee_Decref(lhs_sizeob);
	return Dee_COMPARE_ERR;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}


/* seq_operator_trycompare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with_callattr___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_trycompare_eq__, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with_callobjectcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_trycompare_eq__.c_object, lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with_callmethodcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_trycompare_eq__.c_method, lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with_callkwmethodcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_trycompare_eq__.c_kwmethod, lhs, 1, &rhs, NULL);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	return err_seq_unsupportedf(lhs, "__seq_trycompare_eq__(%r)", rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__empty(DeeObject *lhs, DeeObject *rhs) {
	DeeMH_seq_operator_bool_t seq_operator_bool;
	int rhs_nonempty;
	seq_operator_bool = (DeeMH_seq_operator_bool_t)DeeType_GetMethodHint(Dee_TYPE(rhs), Dee_TMH_seq_operator_bool);
	if (seq_operator_bool == &default__seq_operator_bool__unsupported)
		return 1;
	rhs_nonempty = (*seq_operator_bool)(rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_foreach(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = DeeSeq_OperatorCompareEq(lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}


/* seq_operator_eq */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__with_callattr___seq_eq__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_eq__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__with_callobjectcache___seq_eq__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_eq__.c_object, lhs, 1, &rhs);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__with_callmethodcache___seq_eq__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_eq__.c_method, lhs, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__with_callkwmethodcache___seq_eq__(DeeObject *lhs, DeeObject *rhs) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_eq__.c_kwmethod, lhs, 1, &rhs, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator ==(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = DeeSeq_OperatorCompareEq(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}


/* seq_operator_ne */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__with_callattr___seq_ne__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_ne__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__with_callobjectcache___seq_ne__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ne__.c_object, lhs, 1, &rhs);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__with_callmethodcache___seq_ne__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ne__.c_method, lhs, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__with_callkwmethodcache___seq_ne__(DeeObject *lhs, DeeObject *rhs) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ne__.c_kwmethod, lhs, 1, &rhs, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator !=(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = DeeSeq_OperatorCompareEq(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}


/* seq_operator_lo */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__with_callattr___seq_lo__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_lo__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__with_callobjectcache___seq_lo__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_lo__.c_object, lhs, 1, &rhs);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__with_callmethodcache___seq_lo__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_lo__.c_method, lhs, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__with_callkwmethodcache___seq_lo__(DeeObject *lhs, DeeObject *rhs) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_lo__.c_kwmethod, lhs, 1, &rhs, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator <(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs) {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}


/* seq_operator_le */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__with_callattr___seq_le__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_le__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__with_callobjectcache___seq_le__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_le__.c_object, lhs, 1, &rhs);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__with_callmethodcache___seq_le__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_le__.c_method, lhs, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__with_callkwmethodcache___seq_le__(DeeObject *lhs, DeeObject *rhs) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_le__.c_kwmethod, lhs, 1, &rhs, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator <=(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs) {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}


/* seq_operator_gr */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__with_callattr___seq_gr__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_gr__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__with_callobjectcache___seq_gr__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_gr__.c_object, lhs, 1, &rhs);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__with_callmethodcache___seq_gr__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_gr__.c_method, lhs, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__with_callkwmethodcache___seq_gr__(DeeObject *lhs, DeeObject *rhs) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_gr__.c_kwmethod, lhs, 1, &rhs, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator >(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs) {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}


/* seq_operator_ge */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__with_callattr___seq_ge__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_ge__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__with_callobjectcache___seq_ge__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_ThisCall(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ge__.c_object, lhs, 1, &rhs);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__with_callmethodcache___seq_ge__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ge__.c_method, lhs, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__with_callkwmethodcache___seq_ge__(DeeObject *lhs, DeeObject *rhs) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ge__.c_kwmethod, lhs, 1, &rhs, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator >=(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs) {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}


/* seq_trygetfirst */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with_callattr_first(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_GetAttr(self, (DeeObject *)&str_first);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with_callattr___seq_first__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_GetAttr(self, (DeeObject *)&str___seq_first__);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_Call(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__.c_object, 1, (DeeObject *const *)&self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "first()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__empty(DeeObject *__restrict self) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with__size__and__getitem_index_fast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index_fast)(self, 0);
	if (!result)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with__seq_operator_trygetitem_index(DeeObject *__restrict self) {
	return DeeSeq_OperatorTryGetItemIndex(self, 0);
}

#ifndef DEFINED_seq_default_getfirst_with_foreach_cb
#define DEFINED_seq_default_getfirst_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_getfirst_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	*(DREF DeeObject **)arg = item;
	return -2;
}
#endif /* !DEFINED_seq_default_getfirst_with_foreach_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with__seq_operator_foreach(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &seq_default_getfirst_with_foreach_cb, &result);
	if likely(foreach_status == -2)
		return result;
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}


/* seq_getfirst */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with_callattr_first(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with_callattr___seq_first__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___seq_first__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self) {
	return DeeObject_Call(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__.c_object, 1, (DeeObject *const *)&self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__empty(DeeObject *__restrict self) {
	err_unbound_attribute_string(Dee_TYPE(self), "first");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with__seq_operator_getitem_index(DeeObject *__restrict self) {
	return DeeSeq_OperatorGetItemIndex(self, 0);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with__seq_trygetfirst(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeSeq_InvokeTryGetFirst(self);
	if unlikely(result == ITER_DONE) {
		err_unbound_attribute_string(Dee_TYPE(self), "first");
		result = NULL;
	}
	return result;
}


/* seq_boundfirst */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with_callattr_first(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with_callattr___seq_first__(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str___seq_first__);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self) {
	return call_getter_for_bound(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__.c_object, self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__empty(DeeObject *__restrict self) {
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with__seq_operator_bounditem_index(DeeObject *__restrict self) {
	return DeeSeq_OperatorBoundItemIndex(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with__seq_trygetfirst(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeSeq_InvokeTryGetFirst(self);
	if (result == ITER_DONE)
		return Dee_BOUND_NO;
	if unlikely(!result)
		return Dee_BOUND_ERR;
	Dee_Decref(result);
	return Dee_BOUND_YES;
}


/* seq_delfirst */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__with_callattr_first(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__with_callattr___seq_first__(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str___seq_first__);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self) {
	return call_delete(Dee_TYPE(self)->tp_mhcache->mhc_del___seq_first__.c_object, self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__unsupported(DeeObject *__restrict self) { return err_seq_unsupportedf(self, "del first"); }

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__with__seq_operator_delitem_index(DeeObject *__restrict self) {
	int result = DeeSeq_OperatorDelItemIndex(self, 0);
	if (result < 0 && DeeError_Catch(&DeeError_IndexError))
		result = 0;
	return result;
}


/* seq_setfirst */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__with_callattr_first(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str_first, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__with_callattr___seq_first__(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str___seq_first__, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__with_callobjectcache___seq_first__(DeeObject *self, DeeObject *value) {
	return call_setter(Dee_TYPE(self)->tp_mhcache->mhc_set___seq_first__.c_object, self, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__unsupported(DeeObject *self, DeeObject *value) { return err_seq_unsupportedf(self, "first = %r", value); }

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__empty(DeeObject *self, DeeObject *value) { return err_empty_sequence(self); }

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__with__seq_operator_setitem_index(DeeObject *self, DeeObject *value) {
	return DeeSeq_OperatorSetItemIndex(self, 0, value);
}


/* seq_trygetlast */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__with_callattr_last(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_GetAttr(self, (DeeObject *)&str_last);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__with_callattr___seq_last__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_GetAttr(self, (DeeObject *)&str___seq_last__);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__with_callobjectcache___seq_last__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_Call(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_last__.c_object, 1, (DeeObject *const *)&self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "last()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__empty(DeeObject *__restrict self) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__with__size__and__getitem_index_fast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index_fast)(self, size - 1);
	if (!result)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return ITER_DONE;
	return DeeSeq_OperatorTryGetItemIndex(self, size - 1);
err:
	return NULL;
}

#ifndef DEFINED_seq_default_getlast_with_foreach_cb
#define DEFINED_seq_default_getlast_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_getlast_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	Dee_Decref_unlikely(*(DREF DeeObject **)arg);
	*(DREF DeeObject **)arg = item;
	return 1;
}
#endif /* !DEFINED_seq_default_getlast_with_foreach_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__with__seq_operator_foreach(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	Dee_Incref(Dee_None);
	result = Dee_None;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &seq_default_getlast_with_foreach_cb, &result);
	if likely(foreach_status > 0)
		return result;
	Dee_Decref_unlikely(result);
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}


/* seq_getlast */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast__with_callattr_last(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast__with_callattr___seq_last__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___seq_last__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast__with_callobjectcache___seq_last__(DeeObject *__restrict self) {
	return DeeObject_Call(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_last__.c_object, 1, (DeeObject *const *)&self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast__empty(DeeObject *__restrict self) {
	err_unbound_attribute_string(Dee_TYPE(self), "last");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return default__seq_getlast__empty(self);
	return DeeSeq_OperatorGetItemIndex(self, 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast__with__seq_trygetlast(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeSeq_InvokeTryGetLast(self);
	if unlikely(result == ITER_DONE)
		return default__seq_getlast__empty(self);
	return result;
}


/* seq_boundlast */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__with_callattr_last(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__with_callattr___seq_last__(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str___seq_last__);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__with_callobjectcache___seq_last__(DeeObject *__restrict self) {
	return call_getter_for_bound(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_last__.c_object, self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__empty(DeeObject *__restrict self) {
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__with__seq_operator_size__and__seq_operator_bounditem_index(DeeObject *__restrict self) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return Dee_BOUND_MISSING;
	return DeeSeq_OperatorBoundItemIndex(self, size - 1);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__with__seq_trygetlast(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeSeq_InvokeTryGetLast(self);
	if (result == ITER_DONE)
		return Dee_BOUND_NO;
	if unlikely(!result)
		return Dee_BOUND_ERR;
	Dee_Decref(result);
	return Dee_BOUND_YES;
}


/* seq_dellast */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_dellast__with_callattr_last(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_dellast__with_callattr___seq_last__(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str___seq_last__);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_dellast__with_callobjectcache___seq_last__(DeeObject *__restrict self) {
	return call_delete(Dee_TYPE(self)->tp_mhcache->mhc_del___seq_last__.c_object, self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_dellast__unsupported(DeeObject *__restrict self) { return err_seq_unsupportedf(self, "del last"); }

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_dellast__with__seq_operator_size__and__seq_operator_delitem_index(DeeObject *__restrict self) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return 0;
	return DeeSeq_OperatorDelItemIndex(self, size - 1);
err:
	return -1;
}


/* seq_setlast */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__with_callattr_last(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str_last, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__with_callattr___seq_last__(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str___seq_last__, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__with_callobjectcache___seq_last__(DeeObject *self, DeeObject *value) {
	return call_setter(Dee_TYPE(self)->tp_mhcache->mhc_set___seq_last__.c_object, self, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__unsupported(DeeObject *self, DeeObject *value) { return err_seq_unsupportedf(self, "last = %r", value); }

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__empty(DeeObject *self, DeeObject *value) { return err_empty_sequence(self); }

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__with__seq_operator_size__and__seq_operator_setitem_index(DeeObject *self, DeeObject *value) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return err_empty_sequence(self);
	return DeeSeq_OperatorSetItemIndex(self, size - 1, value);
err:
	return -1;
}


/* seq_any */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any__with_callattr_any(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str_any, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any__with_callattr___seq_any__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_any__, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any__with_callobjectcache___seq_any__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_object, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any__with_callmethodcache___seq_any__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_method, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any__with_callkwmethodcache___seq_any__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any__unsupported(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "__seq_any__()");
}

#ifndef DEFINED_seq_any_foreach_cb
#define DEFINED_seq_any_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp > 0)
		temp = -2;
	return temp;
}
#endif /* !DEFINED_seq_any_foreach_cb */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any__with__seq_foreach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_any_foreach_cb, NULL);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}


/* seq_any_with_key */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__with_callattr_any(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_any, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__with_callattr___seq_any__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_any__, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__with_callobjectcache___seq_any__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_object, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__with_callmethodcache___seq_any__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_method, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__with_callkwmethodcache___seq_any__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_kwmethod, self, 3, args, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__unsupported(DeeObject *self, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_any__(%r)", key);
}

#ifndef DEFINED_seq_any_foreach_with_key_cb
#define DEFINED_seq_any_foreach_with_key_cb
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
#endif /* !DEFINED_seq_any_foreach_with_key_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__with__seq_foreach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_any_foreach_with_key_cb, key);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}


/* seq_any_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callattr_any(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_any, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callattr___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_any__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callobjectcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callkwmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_any__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}

#ifndef DEFINED_seq_any_foreach_cb
#define DEFINED_seq_any_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp > 0)
		temp = -2;
	return temp;
}
#endif /* !DEFINED_seq_any_foreach_cb */
#ifndef DEFINED_seq_any_enumerate_cb
#define DEFINED_seq_any_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_any_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_any_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_any_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_any_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}


/* seq_any_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_any_with_range_and_key__with_callattr_any(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_any, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_any_with_range_and_key__with_callattr___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_any__, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_any_with_range_and_key__with_callobjectcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_object, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_any_with_range_and_key__with_callmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_method, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_any_with_range_and_key__with_callkwmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_kwmethod, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_any_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_any__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}

#ifndef DEFINED_seq_any_foreach_with_key_cb
#define DEFINED_seq_any_foreach_with_key_cb
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
#endif /* !DEFINED_seq_any_foreach_with_key_cb */
#ifndef DEFINED_seq_any_enumerate_with_key_cb
#define DEFINED_seq_any_enumerate_with_key_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_any_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_any_foreach_with_key_cb(arg, item);
}
#endif /* !DEFINED_seq_any_enumerate_with_key_cb */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_any_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_any_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}


/* seq_all */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all__with_callattr_all(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str_all, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all__with_callattr___seq_all__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_all__, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all__with_callobjectcache___seq_all__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_object, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all__with_callmethodcache___seq_all__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_method, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all__with_callkwmethodcache___seq_all__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all__unsupported(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "__seq_all__()");
}

#ifndef DEFINED_seq_all_foreach_cb
#define DEFINED_seq_all_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp == 0)
		temp = -2;
	return temp;
}
#endif /* !DEFINED_seq_all_foreach_cb */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all__with__seq_foreach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_all_foreach_cb, NULL);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_all_with_key */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__with_callattr_all(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_all, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__with_callattr___seq_all__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_all__, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__with_callobjectcache___seq_all__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_object, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__with_callmethodcache___seq_all__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_method, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__with_callkwmethodcache___seq_all__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_kwmethod, self, 3, args, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__unsupported(DeeObject *self, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_all__(%r)", key);
}

#ifndef DEFINED_seq_all_foreach_with_key_cb
#define DEFINED_seq_all_foreach_with_key_cb
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
#endif /* !DEFINED_seq_all_foreach_with_key_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__with__seq_foreach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_all_foreach_with_key_cb, key);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_all_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callattr_all(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_all, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callattr___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_all__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callobjectcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callkwmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_all__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}

#ifndef DEFINED_seq_all_foreach_cb
#define DEFINED_seq_all_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp == 0)
		temp = -2;
	return temp;
}
#endif /* !DEFINED_seq_all_foreach_cb */
#ifndef DEFINED_seq_all_enumerate_cb
#define DEFINED_seq_all_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_all_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_all_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_all_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_all_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_all_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_all_with_range_and_key__with_callattr_all(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_all, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_all_with_range_and_key__with_callattr___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_all__, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_all_with_range_and_key__with_callobjectcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_object, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_all_with_range_and_key__with_callmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_method, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_all_with_range_and_key__with_callkwmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_kwmethod, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_all_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_all__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}

#ifndef DEFINED_seq_all_foreach_with_key_cb
#define DEFINED_seq_all_foreach_with_key_cb
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
#endif /* !DEFINED_seq_all_foreach_with_key_cb */
#ifndef DEFINED_seq_all_enumerate_with_key_cb
#define DEFINED_seq_all_enumerate_with_key_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_all_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_all_foreach_with_key_cb(arg, item);
}
#endif /* !DEFINED_seq_all_enumerate_with_key_cb */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_all_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_all_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_erase */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__with_callattr_erase(DeeObject *__restrict self, size_t index, size_t count) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_erase, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__with_callattr___seq_erase__(DeeObject *__restrict self, size_t index, size_t count) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_erase__, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__with_callobjectcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_erase__.c_object, self, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__with_callmethodcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_erase__.c_method, self, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__with_callkwmethodcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_erase__.c_kwmethod, self, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__unsupported(DeeObject *__restrict self, size_t index, size_t count) {
	return err_seq_unsupportedf(self, "__seq_erase__(%" PRFuSIZ ", %" PRFuSIZ ")", index, count);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index, size_t count) {
	size_t end_index;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	return DeeSeq_OperatorDelRangeIndex(self, (Dee_ssize_t)index, (Dee_ssize_t)end_index);
err_overflow:
	return err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_erase__with__seq_pop(DeeObject *__restrict self, size_t index, size_t count) {
	size_t end_index;
	DeeMH_seq_pop_t seq_pop;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	seq_pop = DeeType_RequireSeqPop(Dee_TYPE(self));
	while (end_index > index) {
		--end_index;
		if unlikely((*seq_pop)(self, (Dee_ssize_t)end_index))
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


/* seq_insert */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insert__with_callattr_insert(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_insert, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insert__with_callattr___seq_insert__(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_insert__, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insert__with_callobjectcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_insert__.c_object, self, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insert__with_callmethodcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_insert__.c_method, self, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insert__with_callkwmethodcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_insert__.c_kwmethod, self, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insert__unsupported(DeeObject *self, size_t index, DeeObject *item) {
	return err_seq_unsupportedf(self, "__seq_insert__(%" PRFuSIZ ", %r)", index, item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insert__with__seq_insertall(DeeObject *self, size_t index, DeeObject *item) {
	int result;
	DREF DeeTupleObject *item_tuple;
	item_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!item_tuple)
		goto err;
	DeeTuple_SET(item_tuple, 0, item);
	result = DeeSeq_InvokeInsertAll(self, index, (DeeObject *)item_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)item_tuple);
	return result;
err:
	return -1;
}


/* seq_insertall */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__with_callattr_insertall(DeeObject *self, size_t index, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_insertall, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__with_callattr___seq_insertall__(DeeObject *self, size_t index, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_insertall__, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__with_callobjectcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_insertall__.c_object, self, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__with_callmethodcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_insertall__.c_method, self, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__with_callkwmethodcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_insertall__.c_kwmethod, self, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__unsupported(DeeObject *self, size_t index, DeeObject *items) {
	return err_seq_unsupportedf(self, "__seq_insertall__(%" PRFuSIZ ", %r)", index, items);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__empty(DeeObject *self, size_t index, DeeObject *items) {
	int items_empty = DeeSeq_OperatorBool(items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_insertall__unsupported(self, index, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *items) {
	return DeeSeq_OperatorSetRangeIndex(self, (Dee_ssize_t)index, (Dee_ssize_t)index, items);
}

#ifndef DEFINED_seq_insertall_with_foreach_insert_cb
#define DEFINED_seq_insertall_with_foreach_insert_cb
struct seq_insertall_with_foreach_insert_data {
	DeeMH_seq_insert_t dsiawfid_insert; /* [1..1] Insert callback */
	DeeObject         *dsiawfid_self;   /* [1..1] The sequence to insert into */
	size_t             dsiawfid_index;  /* Next index for insertion */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_insertall_with_foreach_insert_cb(void *arg, DeeObject *item) {
	struct seq_insertall_with_foreach_insert_data *data;
	data = (struct seq_insertall_with_foreach_insert_data *)arg;
	return (Dee_ssize_t)(*data->dsiawfid_insert)(data->dsiawfid_self, data->dsiawfid_index++, item);
}
#endif /* !DEFINED_seq_insertall_with_foreach_insert_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_insertall__with__seq_insert(DeeObject *self, size_t index, DeeObject *items) {
	struct seq_insertall_with_foreach_insert_data data;
	data.dsiawfid_insert = DeeType_RequireSeqInsert(Dee_TYPE(self));
	data.dsiawfid_self   = self;
	data.dsiawfid_index  = index;
	return (int)DeeSeq_OperatorForeach(items, &seq_insertall_with_foreach_insert_cb, &data);
}


/* seq_pushfront */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_pushfront__with_callattr_pushfront(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_pushfront, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_pushfront__with_callattr___seq_pushfront__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_pushfront__, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_pushfront__with_callobjectcache___seq_pushfront__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_pushfront__.c_object, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_pushfront__with_callmethodcache___seq_pushfront__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_pushfront__.c_method, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_pushfront__with_callkwmethodcache___seq_pushfront__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_pushfront__.c_kwmethod, self, 1, &item, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_pushfront__unsupported(DeeObject *self, DeeObject *item) {
	return err_seq_unsupportedf(self, "__seq_pushfront__(%r)", item);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_pushfront__with__seq_insert(DeeObject *self, DeeObject *item) {
	return DeeSeq_InvokeInsert(self, 0, item);
}


/* seq_append */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__with_callattr_append(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_append, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__with_callattr_pushback(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_pushback, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__with_callattr___seq_append__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_append__, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__with_callobjectcache___seq_append__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_append__.c_object, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__with_callmethodcache___seq_append__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_append__.c_method, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__with_callkwmethodcache___seq_append__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_append__.c_kwmethod, self, 1, &item, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__unsupported(DeeObject *self, DeeObject *item) {
	return err_seq_unsupportedf(self, "__seq_append__(%r)", item);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_append__with__seq_extend(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeTupleObject *item_tuple;
	item_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!item_tuple)
		goto err;
	DeeTuple_SET(item_tuple, 0, item);
	result = DeeSeq_InvokeExtend(self, (DeeObject *)item_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)item_tuple);
	return result;
err:
	return -1;
}


/* seq_extend */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with_callattr_extend(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_extend, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with_callattr___seq_extend__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_extend__, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with_callobjectcache___seq_extend__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_extend__.c_object, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with_callmethodcache___seq_extend__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_extend__.c_method, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with_callkwmethodcache___seq_extend__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_extend__.c_kwmethod, self, 1, &items, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__unsupported(DeeObject *self, DeeObject *items) {
	return err_seq_unsupportedf(self, "__seq_extend__(%r)", items);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, DeeObject *items) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return DeeSeq_OperatorSetRangeIndex(self, size, size, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with__seq_operator_size__and__seq_insertall(DeeObject *self, DeeObject *items) {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return DeeSeq_InvokeInsertAll(self, size, items);
err:
	return -1;
}

#ifndef DEFINED_seq_extend_with_foreach_append_cb
#define DEFINED_seq_extend_with_foreach_append_cb
struct seq_extend_with_foreach_append_data {
	DeeMH_seq_append_t dsewfad_append; /* [1..1] Append callback */
	DeeObject         *dsewfad_self;   /* [1..1] The sequence to append to */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_extend_with_foreach_append_cb(void *arg, DeeObject *item) {
	struct seq_extend_with_foreach_append_data *data;
	data = (struct seq_extend_with_foreach_append_data *)arg;
	return (Dee_ssize_t)(*data->dsewfad_append)(data->dsewfad_self, item);
}
#endif /* !DEFINED_seq_extend_with_foreach_append_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with__seq_append(DeeObject *self, DeeObject *items) {
	struct seq_extend_with_foreach_append_data data;
	data.dsewfad_append = DeeType_RequireSeqAppend(Dee_TYPE(self));
	data.dsewfad_self   = self;
	return (int)DeeSeq_OperatorForeach(items, &seq_extend_with_foreach_append_cb, &data);
}


/* seq_xchitem_index */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callattr_xchitem(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_xchitem, PCKuSIZ "o", index, item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callattr___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_xchitem__, PCKuSIZ "o", index, item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callobjectcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_xchitem__.c_object, self, PCKuSIZ "o", index, item);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_xchitem__.c_method, self, PCKuSIZ "o", index, item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callkwmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_xchitem__.c_kwmethod, self, PCKuSIZ "o", index, item);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__unsupported(DeeObject *self, size_t index, DeeObject *item) {
	err_seq_unsupportedf(self, "__seq_xchitem__(%" PRFuSIZ ", %r)", index, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result = DeeSeq_OperatorGetItemIndex(self, index);
	if likely(result) {
		if (DeeSeq_OperatorSetItemIndex(self, index, item))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}


/* seq_clear */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with_callattr_clear(DeeObject *self) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_clear, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with_callattr___seq_clear__(DeeObject *self) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_clear__, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with_callobjectcache___seq_clear__(DeeObject *self) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_clear__.c_object, self, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with_callmethodcache___seq_clear__(DeeObject *self) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_clear__.c_method, self, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with_callkwmethodcache___seq_clear__(DeeObject *self) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_clear__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__unsupported(DeeObject *self) {
	return err_seq_unsupportedf(self, "__seq_clear__()");
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_operator_delrange(DeeObject *self) {
	return DeeSeq_OperatorDelRange(self, DeeInt_Zero, Dee_None);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_operator_delrange_index_n(DeeObject *self) {
	return DeeSeq_OperatorDelRangeIndexN(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_erase(DeeObject *self) {
	return DeeSeq_InvokeErase(self, 0, (size_t)-1);
}


/* seq_pop */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__with_callattr_pop(DeeObject *self, Dee_ssize_t index) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_pop, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__with_callattr___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_pop__, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__with_callobjectcache___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_pop__.c_object, self, PCKdSIZ, index);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__with_callmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_pop__.c_method, self, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__with_callkwmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_pop__.c_kwmethod, self, PCKdSIZ, index);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__unsupported(DeeObject *self, Dee_ssize_t index) {
	err_seq_unsupportedf(self, "__seq_pop__(%" PRFdSIZ ")", index);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase(DeeObject *self, Dee_ssize_t index) {
	size_t used_index = (size_t)index;
	DREF DeeObject *result;
	if (index < 0) {
		size_t size = DeeSeq_OperatorSize(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, size);
	}
	result = DeeSeq_OperatorGetItemIndex(self, used_index);
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index(DeeObject *self, Dee_ssize_t index) {
	size_t used_index = (size_t)index;
	DREF DeeObject *result;
	if (index < 0) {
		size_t size = DeeSeq_OperatorSize(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, size);
	}
	result = DeeSeq_OperatorGetItemIndex(self, used_index);
	if likely(result) {
		if unlikely(DeeSeq_OperatorDelItemIndex(self, index))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_C */
