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
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>

/**/
#include "../objects/seq/range.h"
#include "method-hint-defaults.h"
#include "method-hints.h"
#include "runtime_error.h"
#include "strings.h"

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
default__seq_operator_bool__with_callattr___seq_bool__(DeeObject *self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_bool__, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with_callobjectcache___seq_bool__(DeeObject *self) {
	DREF DeeObject *result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_object, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with_callmethodcache___seq_bool__(DeeObject *self) {
	DREF DeeObject *result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_method, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with_callkwmethodcache___seq_bool__(DeeObject *self) {
	DREF DeeObject *result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__unsupported(DeeObject *self) {
	return err_seq_unsupportedf(self, "operator bool()");
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_cb(void *arg, DeeObject *elem);
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__seq_operator_foreach(DeeObject *self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__seq_operator_size(DeeObject *self) {
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
default__seq_operator_iter__with_callattr___seq_iter__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_iter__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with_callobjectcache___seq_iter__(DeeObject *self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_iter__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with_callmethodcache___seq_iter__(DeeObject *self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_iter__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with_callkwmethodcache___seq_iter__(DeeObject *self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_iter__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__unsupported(DeeObject *self) {
	err_seq_unsupportedf(self, "operator iter()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__empty(DeeObject *self) { return_empty_iterator; }


/* seq_operator_foreach */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_iter(DeeObject *self, Dee_foreach_t proc, void *arg) {
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
default__seq_operator_foreach_pair__with__seq_operator_iter(DeeObject *self, Dee_foreach_pair_t proc, void *arg) {
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
default__seq_operator_enumerate__with_callattr___seq_enumerate__(DeeObject *self, Dee_enumerate_t proc, void *arg) {
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
default__seq_operator_enumerate__with_callobjectcache___seq_enumerate__(DeeObject *self, Dee_enumerate_t proc, void *arg) {
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
default__seq_operator_enumerate__with_callmethodcache___seq_enumerate__(DeeObject *self, Dee_enumerate_t proc, void *arg) {
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
default__seq_operator_enumerate__with_callkwmethodcache___seq_enumerate__(DeeObject *self, Dee_enumerate_t proc, void *arg) {
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
default__seq_operator_enumerate__unsupported(DeeObject *self, Dee_enumerate_t proc, void *arg) { return err_seq_unsupportedf(self, "__seq_enumerate__(...)"); }

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with__seq_operator_size_and_seq_operator_try_getitem_index(DeeObject *self, Dee_enumerate_t proc, void *arg) {
	/* TODO */
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with__seq_operator_sizeob_and_seq_operator_try_getitem(DeeObject *self, Dee_enumerate_t proc, void *arg) {
	/* TODO */
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with__counter_and_seq_operator_foreach(DeeObject *self, Dee_enumerate_t proc, void *arg) {
	/* TODO */
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default__seq_operator_enumerate__with__counter_and_seq_operator_iter(DeeObject *self, Dee_enumerate_t proc, void *arg) {
	/* TODO */
}


/* seq_operator_enumerate_index */
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with_callattr___seq_enumerate__(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
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
default__seq_operator_enumerate_index__with_callobjectcache___seq_enumerate__(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
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
default__seq_operator_enumerate_index__with_callmethodcache___seq_enumerate__(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
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
default__seq_operator_enumerate_index__with_callkwmethodcache___seq_enumerate__(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
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
default__seq_operator_enumerate_index__unsupported(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) { return err_seq_unsupportedf(self, "__seq_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); }

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with__seq_operator_size_and_seq_operator_try_getitem_index(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	/* TODO */
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with__counter_and_seq_operator_foreach(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	/* TODO */
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL
default__seq_operator_enumerate_index__with__counter_and_seq_operator_iter(DeeObject *self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	/* TODO */
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
default__seq_any_with_range__with_callattr_any(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_any, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callattr___seq_any__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_any__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callobjectcache___seq_any__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callmethodcache___seq_any__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__with_callkwmethodcache___seq_any__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_any_with_range__unsupported(DeeObject *self, size_t start, size_t end) {
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
default__seq_any_with_range__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end) {
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
default__seq_all_with_range__with_callattr_all(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_all, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callattr___seq_all__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_all__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callobjectcache___seq_all__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callmethodcache___seq_all__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__with_callkwmethodcache___seq_all__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_all_with_range__unsupported(DeeObject *self, size_t start, size_t end) {
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
default__seq_all_with_range__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end) {
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


/* seq_trygetfirst */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with_callattr_first(DeeObject *self) {
	DREF DeeObject *result = DeeObject_GetAttr(self, (DeeObject *)&str_first);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with_callattr___seq_first__(DeeObject *self) {
	DREF DeeObject *result = DeeObject_GetAttr(self, (DeeObject *)&str___seq_first__);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with_callobjectcache___seq_first__(DeeObject *self) {
	DREF DeeObject *result = DeeObject_Call(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__.c_object, 1, (DeeObject *const *)&self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__unsupported(DeeObject *self) {
	err_seq_unsupportedf(self, "first()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__empty(DeeObject *self) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with__size_and_getitem_index_fast(DeeObject *self) {
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
default__seq_trygetfirst__with__seq_operator_trygetitem_index(DeeObject *self) {
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
default__seq_trygetfirst__with__seq_operator_foreach(DeeObject *self) {
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
default__seq_getfirst__with_callattr_first(DeeObject *self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with_callattr___seq_first__(DeeObject *self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___seq_first__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with_callobjectcache___seq_first__(DeeObject *self) {
	return DeeObject_Call(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__.c_object, 1, (DeeObject *const *)&self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__empty(DeeObject *self) {
	err_unbound_attribute_string(Dee_TYPE(self), "first");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with__seq_operator_getitem_index(DeeObject *self) {
	return DeeSeq_OperatorGetItemIndex(self, 0);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with__seq_trygetfirst(DeeObject *self) {
	DREF DeeObject *result = DeeSeq_InvokeTryGetFirst(self);
	if unlikely(result == ITER_DONE) {
		err_unbound_attribute_string(Dee_TYPE(self), "first");
		result = NULL;
	}
	return result;
}


/* seq_boundfirst */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with_callattr_first(DeeObject *self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with_callattr___seq_first__(DeeObject *self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str___seq_first__);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with_callobjectcache___seq_first__(DeeObject *self) {
	return call_getter_for_bound(Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__.c_object, self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__empty(DeeObject *self) {
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with__seq_operator_bounditem_index(DeeObject *self) {
	return DeeSeq_OperatorBoundItemIndex(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with__seq_trygetfirst(DeeObject *self) {
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
default__seq_delfirst__with_callattr_first(DeeObject *self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__with_callattr___seq_first__(DeeObject *self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str___seq_first__);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__with_callobjectcache___seq_first__(DeeObject *self) {
	return call_delete(Dee_TYPE(self)->tp_mhcache->mhc_del___seq_first__.c_object, self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__unsupported(DeeObject *self) { return err_seq_unsupportedf(self, "del first"); }

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_delfirst__with__seq_operator_delitem_index(DeeObject *self) {
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
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_C */
