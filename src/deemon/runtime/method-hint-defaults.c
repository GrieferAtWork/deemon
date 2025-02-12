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
#include <deemon/accu.h>
#include <deemon/alloc.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/simple-hashset.h>

#include <hybrid/overflow.h>
#include <hybrid/typecore.h>

/**/
#include "../objects/seq/concat.h"
#include "../objects/seq/default-compare.h"
#include "../objects/seq/default-iterators.h"
#include "../objects/seq/default-sequences.h"
#include "../objects/seq/enumerate-cb.h"
#include "../objects/seq/range.h"
#include "../objects/seq/repeat.h"
#include "../objects/seq/unique-iterator.h"
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
default__seq_operator_bool(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bool))(self);
}

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
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__seq_operator_size(DeeObject *__restrict self) {
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return size != 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__seq_operator_sizeob(DeeObject *__restrict self) {
	int result;
	DREF DeeObject *sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	if (DeeObject_AssertTypeExact(sizeob, &DeeInt_Type))
		goto err;
	result = DeeInt_IsZero(sizeob) ? 0 : 1;
	Dee_Decref(sizeob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__seq_operator_compare_eq(DeeObject *__restrict self) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_compare_eq))(self, Dee_EmptySeq);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}


/* seq_operator_sizeob */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
}

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
	size_t seqsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(seqsize);
err:
	return NULL;
}


/* seq_operator_size */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__unsupported(DeeObject *self) {
	return (size_t)err_seq_unsupportedf(self, "operator size()");
}

#ifndef DEFINED_default_seq_size_with_foreach_cb
#define DEFINED_default_seq_size_with_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return 1;
}
#endif /* !DEFINED_default_seq_size_with_foreach_cb */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__seq_operator_foreach(DeeObject *self) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_seq_size_with_foreach_cb, NULL);
}

#ifndef DEFINED_default_seq_size_with_foreach_pair_cb
#define DEFINED_default_seq_size_with_foreach_pair_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	(void)arg;
	(void)key;
	(void)value;
	return 1;
}
#endif /* !DEFINED_default_seq_size_with_foreach_pair_cb */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__seq_operator_foreach_pair(DeeObject *self) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach_pair))(self, &default_seq_size_with_foreach_pair_cb, NULL);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__seq_operator_sizeob(DeeObject *self) {
	DREF DeeObject *sizeob;
	sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}

#ifndef DEFINED_default_seq_size_with_foreach_pair_cb
#define DEFINED_default_seq_size_with_foreach_pair_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	(void)arg;
	(void)key;
	(void)value;
	return 1;
}
#endif /* !DEFINED_default_seq_size_with_foreach_pair_cb */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__map_enumerate(DeeObject *self) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_seq_size_with_foreach_pair_cb, NULL);
}


/* seq_operator_iter */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
}

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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__seq_operator_getitem_index(DeeObject *__restrict self) {
	DREF DefaultIterator_WithGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->digi_seq              = self;
	result->digi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	result->digi_index            = 0;
	DeeObject_Init(result, &DefaultIterator_WithGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self) {
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	DREF DeeObject *sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	result = DeeGCObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err_size_ob;
	result->disg_index = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!result->disg_index) {
		DeeGCObject_FREE(result);
		goto err_size_ob;
	}
	Dee_Incref(self);
	result->disg_seq        = self; /* Inherit reference */
	result->disg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	result->disg_end        = sizeob; /* Inherit reference */
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItem_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err_size_ob:
	Dee_Decref(sizeob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__seq_operator_getitem(DeeObject *__restrict self) {
	DREF DefaultIterator_WithGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithGetItem);
	if unlikely(!result)
		goto err;
	Dee_Incref(DeeInt_Zero);
	result->dig_index = DeeInt_Zero;
	Dee_Incref(self);
	result->dig_seq        = self; /* Inherit reference */
	result->dig_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	Dee_atomic_lock_init(&result->dig_lock);
	DeeObject_Init(result, &DefaultIterator_WithGetItem_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__map_enumerate(DeeObject *__restrict self) {
	/* TODO: Custom iterator type that uses "tp_enumerate" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self) {
	/* Custom iterator type:
	 * >> local it = self.operator iterkeys();
	 * >> return (() -> {
	 * >>     foreach (local key: it) {
	 * >>         local value = self.trygetitem(key);
	 * >>         if (value != ITER_DONE)
	 * >>             yield (key, value);
	 * >>     }
	 * >> })().operator iter();
	 */
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_iterkeys))(self);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	if unlikely(!Dee_TYPE(result->diikgi_iter)->tp_iter_next &&
	            !DeeType_InheritIterNext(Dee_TYPE(result->diikgi_iter)))
		goto err_r_iter_no_iter_next;
	Dee_Incref(self);
	result->diikgi_seq        = self;
	result->diikgi_tp_next    = Dee_TYPE(result->diikgi_iter)->tp_iter_next;
	result->diikgi_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem);
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndTryGetItemMap_Type);
	return (DREF DeeObject *)result;
err_r_iter_no_iter_next:
	err_unimplemented_operator(Dee_TYPE(result->diikgi_iter), OPERATOR_ITERNEXT);
/*err_r_iter:*/
	Dee_Decref(result->diikgi_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem(DeeObject *__restrict self) {
	/* Custom iterator type:
	 * >> local it = self.operator iterkeys();
	 * >> return (() -> {
	 * >>     foreach (local key: it) {
	 * >>         local value = self.trygetitem(key);
	 * >>         if (value != ITER_DONE)
	 * >>             yield (key, value);
	 * >>     }
	 * >> })().operator iter();
	 */
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_iterkeys))(self);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	if unlikely(!Dee_TYPE(result->diikgi_iter)->tp_iter_next &&
	            !DeeType_InheritIterNext(Dee_TYPE(result->diikgi_iter)))
		goto err_r_iter_no_iter_next;
	Dee_Incref(self);
	result->diikgi_seq        = self;
	result->diikgi_tp_next    = Dee_TYPE(result->diikgi_iter)->tp_iter_next;
	result->diikgi_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem);
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndGetItemMap_Type);
	return (DREF DeeObject *)result;
err_r_iter_no_iter_next:
	err_unimplemented_operator(Dee_TYPE(result->diikgi_iter), OPERATOR_ITERNEXT);
/*err_r_iter:*/
	Dee_Decref(result->diikgi_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


/* seq_operator_foreach */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, cb, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_ssize_t temp, result = 0;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = (*tp_getitem_index_fast)(self, i);
		if unlikely(!elem)
			continue;
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = (*cached_seq_operator_trygetitem_index)(self, i);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem == ITER_DONE)
				continue; /* Unbound item */
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *i, *size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	if unlikely(!size)
		goto err;
	i = DeeObject_NewDefault(Dee_TYPE(size));
	if unlikely(!i)
		goto err_size;
	for (;;) {
		DREF DeeObject *elem;
		int cmp_status;
		cmp_status = DeeObject_CmpLoAsBool(i, size);
		if unlikely(cmp_status < 0)
			goto err_size_i;
		if (!cmp_status)
			break;
		elem = (*cached_seq_operator_getitem)(self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err_size_i;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0) {
			result = temp;
			break;
		}
		result += temp;
		if (DeeObject_Inc(&i))
			goto err_size_i;
	}
	Dee_Decref(i);
	Dee_Decref(size);
	return result;
err_size_i:
	Dee_Decref(i);
err_size:
	Dee_Decref(size);
err:
	return -1;
}

#ifndef DECLARED_default_foreach_with_foreach_pair_cb
#define DECLARED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DECLARED_default_foreach_with_foreach_pair_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach_pair))(self, &default_foreach_with_foreach_pair_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}

#ifndef DEFINED_default_foreach_with_map_enumerate_cb
#define DEFINED_default_foreach_with_map_enumerate_cb
struct default_foreach_with_map_enumerate_data {
	Dee_foreach_t dfwme_cb;  /* [1..1] Underlying callback */
	void         *dfwme_arg; /* [?..?] Cookie for `dfwme_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	Dee_ssize_t result;
	DREF DeeTupleObject *pair;
	struct default_foreach_with_map_enumerate_data *data;
	data = (struct default_foreach_with_map_enumerate_data *)arg;
	if unlikely(!value)
		return 0;
	pair = DeeTuple_NewUninitializedPair();
	if unlikely(!pair)
		goto err;
	pair->t_elem[0] = key;
	pair->t_elem[1] = value;
	result = (*data->dfwme_cb)(data->dfwme_arg, (DeeObject *)pair);
	DeeTuple_DecrefSymbolic((DREF DeeObject *)pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_with_map_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	struct default_foreach_with_map_enumerate_data data;
	data.dfwme_cb  = cb;
	data.dfwme_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_foreach_with_map_enumerate_cb, &data);
}


/* seq_operator_foreach_pair */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach_pair))(self, cb, arg);
}

#ifndef DECLARED_default_foreach_pair_with_foreach_cb
#define DECLARED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DECLARED_default_foreach_pair_with_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach_pair__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_foreach_pair_with_foreach_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach_pair__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}

#ifndef DEFINED_default_foreach_pair_with_map_enumerate_cb
#define DEFINED_default_foreach_pair_with_map_enumerate_cb
struct default_foreach_pair_with_map_enumerate_data {
	Dee_foreach_pair_t dfpwme_cb;  /* [1..1] Underlying callback */
	void              *dfpwme_arg; /* [?..?] Cookie for `dfpwme_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_pair_with_map_enumerate_data *data;
	data = (struct default_foreach_pair_with_map_enumerate_data *)arg;
	if likely(value)
		return (*data->dfpwme_cb)(data->dfpwme_arg, key, value);
	return 0;
}
#endif /* !DEFINED_default_foreach_pair_with_map_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach_pair__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct default_foreach_pair_with_map_enumerate_data data;
	data.dfpwme_cb  = cb;
	data.dfpwme_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_foreach_pair_with_map_enumerate_cb, &data);
}


/* seq_operator_getitem */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem(DeeObject *self, DeeObject *index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, index);
}

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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, index_value);
err:
	return NULL;
}


/* seq_operator_getitem_index */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getitem_index(DeeObject *__restrict self, size_t index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__unsupported(DeeObject *__restrict self, size_t index) {
	err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_getitem_index_with_foreach_data *data;
	data = (struct default_getitem_index_with_foreach_data *)arg;
	if (data->dgiiwfd_nskip == 0) {
		Dee_Incref(elem);
		data->dgiiwfd_result = elem; /* Inherit reference */
		return -2;                   /* Stop enumeration */
	}
	--data->dgiiwfd_nskip;
	return 0;
}
#endif /* !DEFINED_default_getitem_index_with_foreach */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index) {
	struct default_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dgiiwfd_nskip = index;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_getitem_index_with_foreach_cb, &data);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__with__seq_operator_getitem(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}

#ifndef DEFINED_default_getitem_index_with_map_enumerate
#define DEFINED_default_getitem_index_with_map_enumerate
struct default_getitem_index_with_map_enumerate_data {
	DREF DeeObject *dgiiwme_result; /* [0..1][out] Item lookup result */
	size_t          dgiiwme_nskip;  /* Number of indices left to skip. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_getitem_index_with_map_enumerate_data *data;
	data = (struct default_getitem_index_with_map_enumerate_data *)arg;
	if (data->dgiiwme_nskip == 0) {
		if (value) {
			DREF DeeTupleObject *pair;
			pair = DeeTuple_NewUninitializedPair();
			if unlikely(!pair)
				goto err;
			Dee_Incref(key);
			pair->t_elem[0] = key;                         /* Inherit reference */
			Dee_Incref(value);
			pair->t_elem[1] = value;                       /* Inherit reference */
			data->dgiiwme_result = (DREF DeeObject *)pair; /* Inherit reference */
		} else {
			data->dgiiwme_result = NULL;
		}
		return -2; /* Stop enumeration */
	}
	--data->dgiiwme_nskip;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_getitem_index_with_map_enumerate */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getitem_index__with__map_enumerate(DeeObject *__restrict self, size_t index) {
	struct default_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dgiiwme_nskip = index;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dgiiwme_result)
		goto err_unbound;
	return data.dgiiwme_result;
err_unbound:
	err_unbound_index(self, index);
	goto err;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, index - data.dgiiwme_nskip);
err:
	return NULL;
}


/* seq_operator_trygetitem */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem(DeeObject *self, DeeObject *index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem))(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem__empty(DeeObject *self, DeeObject *index) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, index);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, index_value);
err:
	return NULL;
}


/* seq_operator_trygetitem_index */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__empty(DeeObject *__restrict self, size_t index) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, index);
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_getitem_index_with_foreach_data *data;
	data = (struct default_getitem_index_with_foreach_data *)arg;
	if (data->dgiiwfd_nskip == 0) {
		Dee_Incref(elem);
		data->dgiiwfd_result = elem; /* Inherit reference */
		return -2;                   /* Stop enumeration */
	}
	--data->dgiiwfd_nskip;
	return 0;
}
#endif /* !DEFINED_default_getitem_index_with_foreach */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index) {
	struct default_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dgiiwfd_nskip = index;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_getitem_index_with_foreach_cb, &data);
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

#ifndef DEFINED_default_getitem_index_with_map_enumerate
#define DEFINED_default_getitem_index_with_map_enumerate
struct default_getitem_index_with_map_enumerate_data {
	DREF DeeObject *dgiiwme_result; /* [0..1][out] Item lookup result */
	size_t          dgiiwme_nskip;  /* Number of indices left to skip. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_getitem_index_with_map_enumerate_data *data;
	data = (struct default_getitem_index_with_map_enumerate_data *)arg;
	if (data->dgiiwme_nskip == 0) {
		if (value) {
			DREF DeeTupleObject *pair;
			pair = DeeTuple_NewUninitializedPair();
			if unlikely(!pair)
				goto err;
			Dee_Incref(key);
			pair->t_elem[0] = key;                         /* Inherit reference */
			Dee_Incref(value);
			pair->t_elem[1] = value;                       /* Inherit reference */
			data->dgiiwme_result = (DREF DeeObject *)pair; /* Inherit reference */
		} else {
			data->dgiiwme_result = NULL;
		}
		return -2; /* Stop enumeration */
	}
	--data->dgiiwme_nskip;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_getitem_index_with_map_enumerate */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__with__map_enumerate(DeeObject *__restrict self, size_t index) {
	struct default_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dgiiwme_nskip = index;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dgiiwme_result)
		goto err_unbound;
	return data.dgiiwme_result;
err_unbound:
err_bad_bounds:
	return ITER_DONE;
err:
	return NULL;
}


/* seq_operator_hasitem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem(DeeObject *self, DeeObject *index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_hasitem))(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem__unsupported(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "operator [](%r)", index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem__with__seq_operator_sizeob(DeeObject *self, DeeObject *index) {
	int result;
	DREF DeeObject *sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	result = DeeObject_CmpLoAsBool(index, sizeob);
	Dee_Decref(sizeob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem__with__seq_operator_hasitem_index(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_hasitem_index))(self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_hasitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index) {
	DREF DeeObject *value = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, index);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}


/* seq_operator_hasitem_index */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_hasitem_index(DeeObject *__restrict self, size_t index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_hasitem_index))(self, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_hasitem_index__unsupported(DeeObject *__restrict self, size_t index) {
	return err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_hasitem_index__with__seq_operator_size(DeeObject *__restrict self, size_t index) {
	size_t seqsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return index < seqsize ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_hasitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *value = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, index);
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
default__seq_operator_bounditem(DeeObject *self, DeeObject *index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bounditem))(self, index);
}

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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bounditem_index))(self, index_value);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_bounditem__with__seq_operator_getitem(DeeObject *self, DeeObject *index) {
	DREF DeeObject *value = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, index);
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
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bounditem_index(DeeObject *__restrict self, size_t index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bounditem_index))(self, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bounditem_index__unsupported(DeeObject *__restrict self, size_t index) {
	err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bounditem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *value = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, index);
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

#ifndef DEFINED_default_getitem_index_with_map_enumerate
#define DEFINED_default_getitem_index_with_map_enumerate
struct default_getitem_index_with_map_enumerate_data {
	DREF DeeObject *dgiiwme_result; /* [0..1][out] Item lookup result */
	size_t          dgiiwme_nskip;  /* Number of indices left to skip. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_getitem_index_with_map_enumerate_data *data;
	data = (struct default_getitem_index_with_map_enumerate_data *)arg;
	if (data->dgiiwme_nskip == 0) {
		if (value) {
			DREF DeeTupleObject *pair;
			pair = DeeTuple_NewUninitializedPair();
			if unlikely(!pair)
				goto err;
			Dee_Incref(key);
			pair->t_elem[0] = key;                         /* Inherit reference */
			Dee_Incref(value);
			pair->t_elem[1] = value;                       /* Inherit reference */
			data->dgiiwme_result = (DREF DeeObject *)pair; /* Inherit reference */
		} else {
			data->dgiiwme_result = NULL;
		}
		return -2; /* Stop enumeration */
	}
	--data->dgiiwme_nskip;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_getitem_index_with_map_enumerate */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bounditem_index__with__map_enumerate(DeeObject *__restrict self, size_t index) {
	struct default_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dgiiwme_nskip = index;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dgiiwme_result)
		goto err_unbound;
	Dee_Decref(data.dgiiwme_result);
	return Dee_BOUND_YES;
err_unbound:
	return Dee_BOUND_NO;
err_bad_bounds:
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}


/* seq_operator_delitem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem(DeeObject *self, DeeObject *index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem))(self, index);
}

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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, index_value);
err:
	return -1;
}


/* seq_operator_delitem_index */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delitem_index(DeeObject *__restrict self, size_t index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
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

INTERN WUNUSED NONNULL((1)) int DCALL
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
INTERN WUNUSED NONNULL((1)) int DCALL
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

INTERN WUNUSED NONNULL((1)) int DCALL
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

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delitem_index__unsupported(DeeObject *__restrict self, size_t index) {
	return err_seq_unsupportedf(self, "operator del[](%" PRFuSIZ ")", index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delitem_index__empty(DeeObject *__restrict self, size_t index) {
	return err_index_out_of_bounds(self, index, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delitem_index__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index))(self, (Dee_ssize_t)index, (Dee_ssize_t)index + 1);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase(DeeObject *__restrict self, size_t index) {
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, index, 1);
err_oob:
	err_index_out_of_bounds(self, index, size);
err:
	return -1;
}


/* seq_operator_setitem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem(DeeObject *self, DeeObject *index, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem))(self, index, value);
}

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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, index_value, value);
err:
	return -1;
}


/* seq_operator_setitem_index */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, index, value);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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
INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setitem_index__unsupported(DeeObject *self, size_t index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []=(%" PRFuSIZ ", %r)", index, value);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setitem_index__empty(DeeObject *self, size_t index, DeeObject *value) {
	return err_index_out_of_bounds(self, index, 0);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setitem_index__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *value) {
	int result;
	DREF DeeTupleObject *values = DeeTuple_NewUninitialized(1);
	if unlikely(!values)
		goto err;
	values->t_elem[0] = value;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, (Dee_ssize_t)index, (Dee_ssize_t)index + 1, (DeeObject *)values);
	DeeTuple_DecrefSymbolic((DeeObject *)values);
	return result;
err:
	return -1;
}


/* seq_operator_getrange */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange))(self, start, end);
}

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
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange_index_n))(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange_index))(self, start_index, end_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end) {
	int temp;
	DREF DefaultSequence_WithSizeAndGetItem *result;
	DREF DeeObject *startob_and_endob[2];
	DREF DeeObject *startob_and_endob_tuple;
	DREF DeeObject *sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	/* Make a call to "util.clamprange()" to do the range-fixup. */
	startob_and_endob_tuple = DeeModule_CallExternStringf("util", "clamprange", "ooo", start, end, sizeob);
	Dee_Decref(sizeob);
	if unlikely(!startob_and_endob_tuple)
		goto err;
	temp = DeeObject_Unpack(startob_and_endob_tuple, 2, startob_and_endob);
	Dee_Decref(startob_and_endob_tuple);
	if unlikely(temp)
		goto err;
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = startob_and_endob[0]; /* Inherit reference */
	result->dssg_end   = startob_and_endob[1]; /* Inherit reference */
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


/* seq_operator_getrange_index */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange_index))(self, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	err_seq_unsupportedf(self, "operator [:](%" PRFdSIZ ", %" PRFdSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *startob, *endob, *result;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange))(self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	struct Dee_seq_range range;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	struct Dee_seq_range range;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	struct Dee_seq_range range;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeAndGetItem *result;
	struct Dee_seq_range range;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithIterAndLimit *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n(DeeObject *self, Dee_ssize_t start) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange_index_n))(self, start);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start) {
	err_seq_unsupportedf(self, "operator [:](%" PCKdSIZ ", none)", start);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__empty(DeeObject *self, Dee_ssize_t start) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start) {
	DREF DeeObject *startob, *result;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange))(self, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	result->dssgi_tp_getitem_index = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	result->dssgi_start            = (size_t)start;
	result->dssgi_end              = size;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getrange_index(DeeObject *self, Dee_ssize_t start) {
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange_index))(self, start, (Dee_ssize_t)size);
empty_range:
	return_empty_seq;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItem *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithIterAndLimit *result;
	size_t used_start;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsial_seq     = self;
	result->dsial_start   = used_start;
	result->dsial_limit   = (size_t)-1;
	result->dsial_tp_iter = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter);
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


/* seq_operator_delrange */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange))(self, start, end);
}

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
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index_n))(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index))(self, start_index, end_index);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_delrange__with__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, start, end, Dee_None);
}


/* seq_operator_delrange_index */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index))(self, start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return err_seq_unsupportedf(self, "operator del[:](%" PRFdSIZ ", %" PRFdSIZ ")", start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange))(self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, (size_t)start, size - (size_t)start);
empty_range:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, start, end, Dee_None);
}


/* seq_operator_delrange_index_n */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index_n(DeeObject *self, Dee_ssize_t start) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index_n))(self, start);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start) {
	return err_seq_unsupportedf(self, "operator del[:](%" PCKdSIZ ", none)", start);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index_n__with__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start) {
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index))(self, start, (Dee_ssize_t)size);
empty_range:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index_n__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start) {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange))(self, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index_n))(self, start, Dee_None);
}


/* seq_operator_setrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, start, end, items);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with_callattr___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = items;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_setrange__, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with_callobjectcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = items;
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
default__seq_operator_setrange__with_callmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = items;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_setrange__.c_method, self, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with_callkwmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = items;
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
default__seq_operator_setrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	return err_seq_unsupportedf(self, "operator [:]=(%r, %r, %r)", start, end, items);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__empty(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	int items_empty = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bool))(items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange__unsupported(self, start, end, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index_n))(self, start_index, items);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, start_index, end_index, items);
err:
	return -1;
}


/* seq_operator_setrange_index */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, start, end, items);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_operator_setrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items) {
	return err_seq_unsupportedf(self, "operator [:]=(%" PRFdSIZ ", %" PRFdSIZ ", %r)", start, end, items);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_operator_setrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items) {
	int items_empty = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bool))(items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange_index__unsupported(self, start, end, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_operator_setrange_index__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items) {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, startob, endob, items);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items) {
	struct Dee_seq_range range;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	if (range.sr_end > range.sr_start) {
		/* Erase what was there before... */
		if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, range.sr_start, range.sr_end - range.sr_start))
			goto err;
	}
	/* Insert new items. */
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insertall))(self, range.sr_start, items);
err:
	return -1;
}


/* seq_operator_setrange_index_n */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index_n))(self, start, items);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start, DeeObject *items) {
	return err_seq_unsupportedf(self, "operator [:]=(%" PCKdSIZ ", none, %r)", start, items);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setrange_index_n__empty(DeeObject *self, Dee_ssize_t start, DeeObject *items) {
	int items_empty = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bool))(items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange_index_n__unsupported(self, start, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setrange_index_n__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, DeeObject *items) {
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, start, (Dee_ssize_t)size, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setrange_index_n__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, DeeObject *items) {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, startob, Dee_None, items);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}


/* seq_operator_assign */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign(DeeObject *self, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_assign))(self, items);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with_callattr___seq_bool__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_bool__, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with_callobjectcache___seq_bool__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_object, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with_callmethodcache___seq_bool__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_method, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with_callkwmethodcache___seq_bool__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__.c_kwmethod, self, 1, &items, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__unsupported(DeeObject *self, DeeObject *items) {
	return err_seq_unsupportedf(self, "operator :=(%r)", items);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with__seq_operator_setrange(DeeObject *self, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, Dee_None, Dee_None, items);
}


/* seq_operator_hash */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_hash))(self);
}

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
	return DeeObject_HashGeneric(self);
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_seq_hash_with_foreach_cb, &data))
		goto err;
	return data.sqhwf_result;
err:
	return DeeSeq_HandleHashError(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self) {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_hash_t result;
	DREF DeeObject *elem;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	elem = (*tp_getitem_index_fast)(self, 0);
	if unlikely(!elem) {
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = (*tp_getitem_index_fast)(self, i);
		if unlikely(!elem) {
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
	}
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self) {
	Dee_hash_t result;
	DREF DeeObject *elem;
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	elem = (*cached_seq_operator_trygetitem_index)(self, 0);
	if unlikely(!elem)
		goto err;
	if unlikely(elem == ITER_DONE) {
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = (*cached_seq_operator_trygetitem_index)(self, i);
		if unlikely(!elem)
			goto err;
		if unlikely(elem == ITER_DONE) {
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self) {
	Dee_hash_t result;
	DREF DeeObject *elem;
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	elem = (*cached_seq_operator_getitem_index)(self, 0);
	if unlikely(!elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err;
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *indexob, *elem;
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	DREF DeeObject *sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	temp = DeeObject_CmpLoAsBool(indexob, sizeob);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err_sizeob_indexob;
		Dee_Decref(indexob);
		Dee_Decref(sizeob);
		return DEE_HASHOF_EMPTY_SEQUENCE;
	}
	elem = (*cached_seq_operator_getitem)(self, indexob);
	if unlikely(!elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_sizeob_indexob;
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (;;) {
		Dee_hash_t elem_hash;
		if (DeeObject_Inc(&indexob))
			goto err_sizeob_indexob;
		temp = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (temp <= 0) {
			if unlikely(temp < 0)
				goto err_sizeob_indexob;
			break;
		}
		elem = (*cached_seq_operator_getitem)(self, indexob);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_indexob;
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return DeeSeq_HandleHashError(self);
}


/* seq_operator_compare */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with_callattr___seq_compare__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_compare__, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else if (DeeInt_IsNeg(resultob)) {
		result = -1;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
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
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else if (DeeInt_IsNeg(resultob)) {
		result = -1;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
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
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else if (DeeInt_IsNeg(resultob)) {
		result = -1;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
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
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else if (DeeInt_IsNeg(resultob)) {
		result = -1;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
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
	int rhs_nonempty = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_bool))(rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? -1 : 0;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__lo(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_eq))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_lo))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__le(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_eq))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_le))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__gr(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_eq))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_gr))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__ge(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_eq))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ge))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__lo(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ne))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_lo))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__le(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ne))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_le))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__gr(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ne))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_gr))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__ge(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ne))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ge))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t result;
	DeeMH_seq_operator_foreach_t cached_seq_operator_foreach = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_foreach);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = rhs;
			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;
			result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				if unlikely(temp < 0) {
					result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPARE_FOREACH_RESULT_LESS;
				}
			}
			Dee_Decref(data.scf_sg_oindex);
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_iter))(rhs);
		if unlikely(!rhs_iter)
			goto err;
		result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_iter__cb, rhs_iter);
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
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeNO_getitem_index_fast_t lhs_getitem_index_fast = Dee_TYPE(lhs)->tp_seq->tp_getitem_index_fast;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
	    DeeMH_seq_operator_trygetitem_index_t rhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                       rhs, rhs_size, rhs_seq_operator_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeMH_seq_operator_getitem_index_t rhs_seq_operator_getitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                    rhs, rhs_size, rhs_seq_operator_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeMH_seq_operator_getitem_t rhs_seq_operator_getitem = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem);
		DREF DeeObject *rhs_sizeob = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                rhs, rhs_sizeob, rhs_seq_operator_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compare__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeMH_seq_operator_trygetitem_index_t lhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_trygetitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                       rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeMH_seq_operator_trygetitem_index_t rhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                     rhs, rhs_size, rhs_seq_operator_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		DeeMH_seq_operator_getitem_index_t rhs_seq_operator_getitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                  rhs, rhs_size, rhs_seq_operator_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeMH_seq_operator_getitem_t rhs_seq_operator_getitem = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem);
		DREF DeeObject *rhs_sizeob = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                              rhs, rhs_sizeob, rhs_seq_operator_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_trygetitem_index;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compare__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeMH_seq_operator_getitem_index_t lhs_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_getitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                       rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeMH_seq_operator_trygetitem_index_t rhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                  rhs, rhs_size, rhs_seq_operator_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeMH_seq_operator_getitem_index_t rhs_seq_operator_getitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                  rhs, rhs_size, rhs_seq_operator_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeMH_seq_operator_getitem_t rhs_seq_operator_getitem = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem);
		DREF DeeObject *rhs_sizeob = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                              rhs, rhs_sizeob, rhs_seq_operator_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_getitem_index;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compare__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_getitem);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	DREF DeeObject *lhs_sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_sizeob))(lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                              rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                           rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
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
		data.scf_sg_ogetitem = cached_seq_operator_getitem;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compare__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		ASSERT(data.scf_sg_osize == lhs_sizeob);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, lhs_sizeob);
			if unlikely(temp < 0) {
				result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
			} else if (temp) {
				result = SEQ_COMPARE_FOREACH_RESULT_LESS;
			}
		}
		Dee_Decref(data.scf_sg_oindex);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err_lhs_sizeob;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = 0;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	Dee_Decref(lhs_sizeob);
	return result;
err_lhs_sizeob:
	Dee_Decref(lhs_sizeob);
err:
	return Dee_COMPARE_ERR;
}


/* seq_operator_compare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare_eq))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with_callattr___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(lhs, (DeeObject *)&str___seq_compare_eq__, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	if (DeeBool_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return DeeBool_IsTrue(resultob) ? 0 : 1;
	}
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
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
	if (DeeBool_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return DeeBool_IsTrue(resultob) ? 0 : 1;
	}
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
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
	if (DeeBool_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return DeeBool_IsTrue(resultob) ? 0 : 1;
	}
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
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
	if (DeeBool_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return DeeBool_IsTrue(resultob) ? 0 : 1;
	}
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
err:
	return Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	return err_seq_unsupportedf(lhs, "__seq_compare_eq__(%r)", rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_eq(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_eq))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_ne(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ne))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_lo__and__seq_operator__gr(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_lo))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Different */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_gr))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_le__and__seq_operator__ge(DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_le))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 1; /* Different */
	cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_gr))(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return -1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t result;
	DeeMH_seq_operator_foreach_t cached_seq_operator_foreach = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_foreach);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = rhs;
			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;
			result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				if unlikely(temp < 0) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
				}
			}
			Dee_Decref(data.scf_sg_oindex);
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_iter))(rhs);
		if unlikely(!rhs_iter)
			goto err;
		result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_iter__cb, rhs_iter);
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
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeNO_getitem_index_fast_t lhs_getitem_index_fast = Dee_TYPE(lhs)->tp_seq->tp_getitem_index_fast;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                           rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
	    DeeMH_seq_operator_trygetitem_index_t rhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                         rhs, rhs_size, rhs_seq_operator_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeMH_seq_operator_getitem_index_t rhs_seq_operator_getitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                      rhs, rhs_size, rhs_seq_operator_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeMH_seq_operator_getitem_t rhs_seq_operator_getitem = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem);
		DREF DeeObject *rhs_sizeob = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                  rhs, rhs_sizeob, rhs_seq_operator_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
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
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeMH_seq_operator_trygetitem_index_t lhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_trygetitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeMH_seq_operator_trygetitem_index_t rhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                       rhs, rhs_size, rhs_seq_operator_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		DeeMH_seq_operator_getitem_index_t rhs_seq_operator_getitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                    rhs, rhs_size, rhs_seq_operator_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeMH_seq_operator_getitem_t rhs_seq_operator_getitem = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem);
		DREF DeeObject *rhs_sizeob = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                rhs, rhs_sizeob, rhs_seq_operator_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_trygetitem_index;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compareeq__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
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
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeMH_seq_operator_getitem_index_t lhs_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_getitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeMH_seq_operator_trygetitem_index_t rhs_seq_operator_trygetitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_trygetitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                    rhs, rhs_size, rhs_seq_operator_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeMH_seq_operator_getitem_index_t rhs_seq_operator_getitem_index = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem_index);
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                    rhs, rhs_size, rhs_seq_operator_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeMH_seq_operator_getitem_t rhs_seq_operator_getitem = DeeType_RequireMethodHint(tp_rhs, seq_operator_getitem);
		DREF DeeObject *rhs_sizeob = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_sizeob))(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                rhs, rhs_sizeob, rhs_seq_operator_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_getitem_index;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
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
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_getitem);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(tp_rhs, seq_operator_foreach);
	DREF DeeObject *lhs_sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_sizeob))(lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                                  rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		size_t rhs_size = (*DeeType_RequireMethodHint(tp_rhs, seq_operator_size))(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                             rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
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
		data.scf_sg_ogetitem = cached_seq_operator_getitem;
		foreach_result = (*rhs_seq_operator_foreach)(rhs, &seq_compareeq__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		ASSERT(data.scf_sg_osize == lhs_sizeob);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, lhs_sizeob);
			if unlikely(temp < 0) {
				result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
			} else if (temp) {
				result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
			}
		}
		Dee_Decref(data.scf_sg_oindex);
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
err:
	return Dee_COMPARE_ERR;
}


/* seq_operator_trycompare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_trycompare_eq))(lhs, rhs);
}

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
	int rhs_nonempty;
	DeeMH_seq_operator_bool_t rhs_seq_operator_bool = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_bool);
	if (rhs_seq_operator_bool == &default__seq_operator_bool__unsupported)
		return 1;
	rhs_nonempty = (*rhs_seq_operator_bool)(rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_foreach);
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__unsupported)
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_foreach(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *lhs, DeeObject *rhs) {
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_foreach);
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__unsupported)
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs) {
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_foreach);
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__unsupported)
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs) {
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_foreach);
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__unsupported)
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs) {
	DeeMH_seq_operator_foreach_t rhs_seq_operator_foreach = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_foreach);
	if (rhs_seq_operator_foreach == &default__seq_operator_foreach__unsupported)
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare_eq))(lhs, rhs);
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
default__seq_operator_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_eq))(lhs, rhs);
}

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
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare_eq))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}


/* seq_operator_ne */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ne))(lhs, rhs);
}

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
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare_eq))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}


/* seq_operator_lo */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_lo))(lhs, rhs);
}

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
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}


/* seq_operator_le */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_le))(lhs, rhs);
}

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
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}


/* seq_operator_gr */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_gr))(lhs, rhs);
}

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
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}


/* seq_operator_ge */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ge))(lhs, rhs);
}

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
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_compare))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}


/* seq_operator_inplace_add */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_add(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), seq_operator_inplace_add))(p_self, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_add__with_callattr___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___seq_inplace_add__, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_add__.c_object, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_add__with_callmethodcache___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_add__.c_method, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_add__with_callkwmethodcache___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_add__.c_kwmethod, *p_self, 1, &rhs, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_add__with__seq_extend(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), seq_extend))(*p_self, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_add__with__DeeSeq_Concat(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result = DeeSeq_Concat(*p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}


/* seq_operator_inplace_mul */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_mul(DREF DeeObject **__restrict p_self, DeeObject *repeat) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), seq_operator_inplace_mul))(p_self, repeat);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_mul__with_callattr___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___seq_inplace_mul__, 1, &repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_mul__.c_object, *p_self, 1, &repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_mul__with_callmethodcache___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_mul__.c_method, *p_self, 1, &repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_mul__with_callkwmethodcache___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_mul__.c_kwmethod, *p_self, 1, &repeat, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_mul__with__seq_clear__and__seq_extend(DREF DeeObject **__restrict p_self, DeeObject *repeat) {
	int result;
	DREF DeeObject *extend_with_this;
	size_t repeatval;
	if (DeeObject_AsSize(repeat, &repeatval))
		goto err;
	if (repeatval == 0)
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), seq_clear))(*p_self);
	if (repeatval == 1)
		return 0;
	extend_with_this = DeeSeq_Repeat(*p_self, repeatval - 1);
	if unlikely(!extend_with_this)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), seq_extend))(*p_self, extend_with_this);
	Dee_Decref_likely(extend_with_this);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_inplace_mul__with__DeeSeq_Repeat(DREF DeeObject **__restrict p_self, DeeObject *repeat) {
	size_t repeatval;
	DREF DeeObject *result;
	if (DeeObject_AsSize(repeat, &repeatval))
		goto err;
	if (repeatval == 0) {
		result = Dee_EmptySeq;
		Dee_Incref(Dee_EmptySeq);
	} else if (repeatval == 1) {
		return 0;
	} else {
		result = DeeSeq_Repeat(*p_self, repeatval);
		if unlikely(!result)
			goto err;
	}
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}


/* seq_enumerate */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_enumerate__, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_object, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_method, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_kwmethod, self, 1, (DeeObject *const *)&wrapper, NULL);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) { return err_seq_unsupportedf(self, "__seq_enumerate__(...)"); }

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_ssize_t temp, result = 0;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *indexob, *index_value;
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = (*tp_getitem_index_fast)(self, i);
		temp = (*proc)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		Dee_Decref(indexob);
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

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, i);
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

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value, *sizeob;
	sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
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
		index_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, indexob);
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

#ifndef DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb
#define DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb
struct default_seq_enumerate_with_counter__and__seq_foreach_data {
	Dee_seq_enumerate_t dewcaf_proc;    /* [1..1] Wrapped callback */
	void           *dewcaf_arg;     /* [?..?] Cookie for `dewcaf_proc' */
	size_t          dewcaf_counter; /* Index of the next element that will be enumerated */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_enumerate_with_counter__and__seq_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	DREF DeeObject *indexob;
	struct default_seq_enumerate_with_counter__and__seq_foreach_data *data;
	data = (struct default_seq_enumerate_with_counter__and__seq_foreach_data *)arg;
	indexob = DeeInt_NewSize(data->dewcaf_counter);
	if unlikely(!indexob)
		goto err;
	++data->dewcaf_counter;
	result = (*data->dewcaf_proc)(data->dewcaf_arg, indexob, elem);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__counter__and__seq_operator_foreach(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	struct default_seq_enumerate_with_counter__and__seq_foreach_data data;
	data.dewcaf_proc    = proc;
	data.dewcaf_arg     = arg;
	data.dewcaf_counter = 0;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_seq_enumerate_with_counter__and__seq_foreach_cb, &data);
}


/* seq_enumerate_index */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_enumerate__, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_object, self, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_method, self, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) { return err_seq_unsupportedf(self, "__seq_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); }

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_ssize_t temp, result = 0;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = (*tp_getitem_index_fast)(self, i);
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

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, i);
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

#ifndef DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb
#define DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb
#define default_seq_enumerate_index_with_counter__and__seq_foreach_cb_MAGIC_EARLY_STOP \
	(__SSIZE_MIN__ + 99) /* Shhht. We don't talk about this one... */

struct default_seq_enumerate_index_with_counter__and__seq_foreach_data {
	Dee_seq_enumerate_index_t deiwcaf_proc;  /* [1..1] Wrapped callback */
	void                     *deiwcaf_arg;   /* [?..?] Cookie for `deiwcaf_proc' */
	size_t                    deiwcaf_index; /* Index of the next element that will be enumerate_indexd */
	size_t                    deiwcaf_start; /* Enumeration start index */
	size_t                    deiwcaf_end;   /* Enumeration end index */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_enumerate_index_with_counter__and__seq_foreach_cb(void *arg, DeeObject *elem) {
	size_t index;
	struct default_seq_enumerate_index_with_counter__and__seq_foreach_data *data;
	data = (struct default_seq_enumerate_index_with_counter__and__seq_foreach_data *)arg;
	if (data->deiwcaf_index >= data->deiwcaf_end)
		return default_seq_enumerate_index_with_counter__and__seq_foreach_cb_MAGIC_EARLY_STOP;
	index = data->deiwcaf_index++;
	if (index < data->deiwcaf_start)
		return 0; /* Skipped... */
	return (*data->deiwcaf_proc)(data->deiwcaf_arg, index, elem);
}
#endif /* !DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with__counter__and__seq_operator_foreach(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	struct default_seq_enumerate_index_with_counter__and__seq_foreach_data data;
	Dee_ssize_t result;
	data.deiwcaf_proc  = proc;
	data.deiwcaf_arg   = arg;
	data.deiwcaf_index = 0;
	data.deiwcaf_start = start;
	data.deiwcaf_end   = end;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_seq_enumerate_index_with_counter__and__seq_foreach_cb, &data);
	if unlikely(result == default_seq_enumerate_index_with_counter__and__seq_foreach_cb_MAGIC_EARLY_STOP)
		result = 0;
	return result;
}


/* seq_makeenumeration */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with_callattr___seq_enumerate_items__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_enumerate_items__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__(DeeObject *self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with_callmethodcache___seq_enumerate_items__(DeeObject *self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with_callkwmethodcache___seq_enumerate_items__(DeeObject *self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__unsupported(DeeObject *self) {
	err_seq_unsupportedf(self, "__seq_enumerate_items__()");
	return NULL;
}


/* seq_makeenumeration_with_int_range */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_int_range__with_callattr___seq_enumerate_items__(DeeObject *self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_enumerate_items__, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_int_range__with_callobjectcache___seq_enumerate_items__(DeeObject *self, size_t start, size_t end) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_int_range__with_callmethodcache___seq_enumerate_items__(DeeObject *self, size_t start, size_t end) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_int_range__with_callkwmethodcache___seq_enumerate_items__(DeeObject *self, size_t start, size_t end) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_int_range__unsupported(DeeObject *self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_enumerate_items__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}


/* seq_makeenumeration_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__with_callattr___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_enumerate_items__, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_object, self, 2, args);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__with_callmethodcache___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_method, self, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__with_callkwmethodcache___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__.c_kwmethod, self, 2, args, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__unsupported(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_seq_unsupportedf(self, "__seq_enumerate_items__(%r, %r)", start, end);
	return NULL;
}


/* seq_foreach_reverse */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_foreach_reverse__with__seq_operator_size__and__getitem_index_fast(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeObject *(DCALL *tp_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t temp, result = 0;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*tp_getitem_index_fast)(self, size);
		if likely(item) {
			temp = (*cb)(arg, item);
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
default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	Dee_ssize_t temp, result = 0;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	seq_operator_trygetitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self));
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq_operator_trygetitem_index)(self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*cb)(arg, item);
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
default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	Dee_ssize_t temp, result = 0;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	seq_operator_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self));
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq_operator_getitem_index)(self, size);
		if likely(item) {
			temp = (*cb)(arg, item);
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
default__seq_foreach_reverse__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DeeMH_seq_operator_getitem_t seq_operator_getitem;
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *sizeob = DeeType_InvokeMethodHint0(self, seq_operator_sizeob);
	if unlikely(!sizeob)
		goto err;
	seq_operator_getitem = DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self));
	for (;;) {
		DREF DeeObject *item;
		int size_is_nonzero = DeeObject_Bool(sizeob);
		if unlikely(size_is_nonzero < 0)
			goto err_sizeob;
		if (!size_is_nonzero)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob;
		item = (*seq_operator_getitem)(self, sizeob);
		if likely(item) {
			temp = (*cb)(arg, item);
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


/* seq_enumerate_index_reverse */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index_reverse__with__seq_operator_size__and__getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DREF DeeObject *(DCALL *tp_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t temp, result = 0;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*tp_getitem_index_fast)(self, size);
		temp = (*cb)(arg, size, item);
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
default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	Dee_ssize_t temp, result = 0;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	seq_operator_trygetitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self));
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq_operator_trygetitem_index)(self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*cb)(arg, size, item);
			Dee_Decref(item);
		} else {
			temp = (*cb)(arg, size, NULL);
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
default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	Dee_ssize_t temp, result = 0;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	seq_operator_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self));
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq_operator_getitem_index)(self, size);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*cb)(arg, size, item);
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
default__seq_enumerate_index_reverse__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DeeMH_seq_operator_getitem_t seq_operator_getitem;
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *startob = NULL;
	DREF DeeObject *sizeob = DeeType_InvokeMethodHint0(self, seq_operator_sizeob);
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
	seq_operator_getitem = DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self));
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
		item = (*seq_operator_getitem)(self, sizeob);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_startob;
		}
		if unlikely(DeeObject_AsSize(sizeob, &index_value))
			goto err_sizeob_startob;
		temp = 0;
		if likely(index_value >= start && index_value < end)
			temp = (*cb)(arg, index_value, item);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, 0);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, 0);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst__with__seq_trygetfirst(DeeObject *__restrict self) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetfirst))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bounditem_index))(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__with__seq_trygetfirst(DeeObject *__restrict self) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetfirst))(self);
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
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, 0);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, 0, value);
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
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return ITER_DONE;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, size - 1);
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
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return default__seq_getlast__empty(self);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast__with__seq_trygetlast(DeeObject *__restrict self) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetlast))(self);
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
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return Dee_BOUND_MISSING;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bounditem_index))(self, size - 1);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__with__seq_trygetlast(DeeObject *__restrict self) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetlast))(self);
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
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return 0;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, size - 1);
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
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return err_empty_sequence(self);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, size - 1, value);
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
default__seq_any__with__seq_operator_foreach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_any_foreach_cb, NULL);
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

#ifndef DEFINED_seq_any_with_key_foreach_cb
#define DEFINED_seq_any_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_with_key_foreach_cb(void *arg, DeeObject *item) {
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
#endif /* !DEFINED_seq_any_with_key_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_any_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_any_with_key_foreach_cb, key);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}


/* seq_any_with_range */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any_with_range__with_callattr_any(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_any, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any_with_range__with_callattr___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_any__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any_with_range__with_callobjectcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any_with_range__with_callmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any_with_range__with_callkwmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
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
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_any_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_any_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}


/* seq_any_with_range_and_key */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_any_with_range_and_key__with_callattr_any(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_any, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_any_with_range_and_key__with_callattr___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_any__, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_any_with_range_and_key__with_callobjectcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_object, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_any_with_range_and_key__with_callmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_method, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_any_with_range_and_key__with_callkwmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_any__.c_kwmethod, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_any_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_any__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}

#ifndef DEFINED_seq_any_with_key_foreach_cb
#define DEFINED_seq_any_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_with_key_foreach_cb(void *arg, DeeObject *item) {
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
#endif /* !DEFINED_seq_any_with_key_foreach_cb */
#ifndef DEFINED_seq_any_with_key_enumerate_cb
#define DEFINED_seq_any_with_key_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_any_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_any_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_any_with_key_enumerate_cb */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_any_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_any_with_key_enumerate_cb, key, start, end);
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
default__seq_all__with__seq_operator_foreach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_all_foreach_cb, NULL);
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

#ifndef DEFINED_seq_all_with_key_foreach_cb
#define DEFINED_seq_all_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_with_key_foreach_cb(void *arg, DeeObject *item) {
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
#endif /* !DEFINED_seq_all_with_key_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_all_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_all_with_key_foreach_cb, key);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_all_with_range */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all_with_range__with_callattr_all(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_all, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all_with_range__with_callattr___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_all__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all_with_range__with_callobjectcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all_with_range__with_callmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all_with_range__with_callkwmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
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
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_all_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_all_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_all_with_range_and_key */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_all_with_range_and_key__with_callattr_all(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_all, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_all_with_range_and_key__with_callattr___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_all__, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_all_with_range_and_key__with_callobjectcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_object, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_all_with_range_and_key__with_callmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_method, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_all_with_range_and_key__with_callkwmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_all__.c_kwmethod, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_all_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_all__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}

#ifndef DEFINED_seq_all_with_key_foreach_cb
#define DEFINED_seq_all_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_with_key_foreach_cb(void *arg, DeeObject *item) {
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
#endif /* !DEFINED_seq_all_with_key_foreach_cb */
#ifndef DEFINED_seq_all_with_key_enumerate_cb
#define DEFINED_seq_all_with_key_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_all_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_all_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_all_with_key_enumerate_cb */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_all_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_all_with_key_enumerate_cb, key, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_parity */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__with_callattr_parity(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str_parity, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__with_callattr___seq_parity__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_parity__, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__with_callobjectcache___seq_parity__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_object, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__with_callmethodcache___seq_parity__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_method, self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__with_callkwmethodcache___seq_parity__(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__unsupported(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "__seq_parity__()");
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__with__seq_count(DeeObject *__restrict self) {
	size_t count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count))(self, Dee_True);
	if unlikely(count == (size_t)-1)
		goto err;
	return count & 1;
err:
	return -1;
}

#ifndef DEFINED_seq_parity_foreach_cb
#define DEFINED_seq_parity_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_parity_foreach_cb(void *arg, DeeObject *item) {
	(void)arg;
	return (Dee_ssize_t)DeeObject_Bool(item);
}
#endif /* !DEFINED_seq_parity_foreach_cb */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity__with__seq_operator_foreach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_parity_foreach_cb, NULL);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_parity_with_key */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_parity_with_key__with_callattr_parity(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_parity, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_parity_with_key__with_callattr___seq_parity__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_parity__, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_parity_with_key__with_callobjectcache___seq_parity__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_object, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_parity_with_key__with_callmethodcache___seq_parity__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_method, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_parity_with_key__with_callkwmethodcache___seq_parity__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_kwmethod, self, 3, args, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_parity_with_key__unsupported(DeeObject *self, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_parity__(%r)", key);
}

#ifndef DEFINED_seq_parity_foreach_with_key_cb
#define DEFINED_seq_parity_foreach_with_key_cb
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
#endif /* !DEFINED_seq_parity_foreach_with_key_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_parity_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_parity_foreach_with_key_cb, key);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_parity_with_range */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__with_callattr_parity(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_parity, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__with_callattr___seq_parity__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_parity__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__with_callobjectcache___seq_parity__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__with_callmethodcache___seq_parity__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__with_callkwmethodcache___seq_parity__(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_parity__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__with__seq_count_with_range(DeeObject *__restrict self, size_t start, size_t end) {
	size_t count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count_with_range))(self, Dee_True, start, end);
	if unlikely(count == (size_t)-1)
		goto err;
	return count & 1;
err:
	return -1;
}

#ifndef DEFINED_seq_parity_foreach_cb
#define DEFINED_seq_parity_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_parity_foreach_cb(void *arg, DeeObject *item) {
	(void)arg;
	return (Dee_ssize_t)DeeObject_Bool(item);
}
#endif /* !DEFINED_seq_parity_foreach_cb */
#ifndef DEFINED_seq_parity_enumerate_cb
#define DEFINED_seq_parity_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_parity_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_parity_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_parity_enumerate_cb */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_parity_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_parity_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_parity_with_range_and_key */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_parity_with_range_and_key__with_callattr_parity(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str_parity, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_parity_with_range_and_key__with_callattr___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_parity__, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_object, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_parity_with_range_and_key__with_callmethodcache___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_method, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_parity_with_range_and_key__with_callkwmethodcache___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__.c_kwmethod, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_parity_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_parity__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}

#ifndef DEFINED_seq_parity_foreach_with_key_cb
#define DEFINED_seq_parity_foreach_with_key_cb
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
#endif /* !DEFINED_seq_parity_foreach_with_key_cb */
#ifndef DEFINED_seq_parity_enumerate_with_key_cb
#define DEFINED_seq_parity_enumerate_with_key_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_parity_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_parity_foreach_with_key_cb(arg, item);
}
#endif /* !DEFINED_seq_parity_enumerate_with_key_cb */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_parity_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_parity_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}


/* seq_min */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min__with_callattr_min(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_min, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min__with_callattr___seq_min__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_min__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min__with_callobjectcache___seq_min__(DeeObject *__restrict self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min__with_callmethodcache___seq_min__(DeeObject *__restrict self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min__with_callkwmethodcache___seq_min__(DeeObject *__restrict self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "__seq_min__()");
	return NULL;
}

#ifndef DEFINED_seq_min_foreach_cb
#define DEFINED_seq_min_foreach_cb
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
#endif /* !DEFINED_seq_min_foreach_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min__with__seq_operator_foreach(DeeObject *__restrict self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_min_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}


/* seq_min_with_key */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_min_with_key__with_callattr_min(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObject_CallAttr(self, (DeeObject *)&str_min, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_min_with_key__with_callattr___seq_min__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_min__, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_min_with_key__with_callobjectcache___seq_min__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_object, self, 3, args);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_min_with_key__with_callmethodcache___seq_min__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_method, self, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_min_with_key__with_callkwmethodcache___seq_min__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_kwmethod, self, 3, args, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_min_with_key__unsupported(DeeObject *self, DeeObject *key) {
	err_seq_unsupportedf(self, "__seq_min__(%r)", key);
	return NULL;
}

#ifndef DEFINED_seq_minmax_with_key_data
#define DEFINED_seq_minmax_with_key_data
struct seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};
#endif /* !DEFINED_seq_minmax_with_key_data */
#ifndef DEFINED_seq_min_with_key_foreach_cb
#define DEFINED_seq_min_with_key_foreach_cb
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
#endif /* !DEFINED_seq_min_with_key_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_min_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_min_with_key_foreach_cb, &data);
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


/* seq_min_with_range */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min_with_range__with_callattr_min(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_min, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min_with_range__with_callattr___seq_min__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_min__, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min_with_range__with_callobjectcache___seq_min__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min_with_range__with_callmethodcache___seq_min__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min_with_range__with_callkwmethodcache___seq_min__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_min__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

#ifndef DEFINED_seq_min_foreach_cb
#define DEFINED_seq_min_foreach_cb
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
#endif /* !DEFINED_seq_min_foreach_cb */
#ifndef DEFINED_seq_min_enumerate_cb
#define DEFINED_seq_min_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_min_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_min_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_min_enumerate_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_min_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_min_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}


/* seq_min_with_range_and_key */
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_min_with_range_and_key__with_callattr_min(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_min, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_min_with_range_and_key__with_callattr___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_min__, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_min_with_range_and_key__with_callobjectcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_object, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_min_with_range_and_key__with_callmethodcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_method, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_min_with_range_and_key__with_callkwmethodcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_min__.c_kwmethod, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_min_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "__seq_min__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
	return NULL;
}

#ifndef DEFINED_seq_minmax_with_key_data
#define DEFINED_seq_minmax_with_key_data
struct seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};
#endif /* !DEFINED_seq_minmax_with_key_data */
#ifndef DEFINED_seq_min_with_key_foreach_cb
#define DEFINED_seq_min_with_key_foreach_cb
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
#endif /* !DEFINED_seq_min_with_key_foreach_cb */
#ifndef DEFINED_seq_min_with_key_enumerate_cb
#define DEFINED_seq_min_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_min_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_min_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_min_with_key_enumerate_cb */
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_min_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_min_with_key_enumerate_cb, &data, start, end);
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


/* seq_max */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max__with_callattr_max(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_max, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max__with_callattr___seq_max__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_max__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max__with_callobjectcache___seq_max__(DeeObject *__restrict self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max__with_callmethodcache___seq_max__(DeeObject *__restrict self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max__with_callkwmethodcache___seq_max__(DeeObject *__restrict self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "__seq_max__()");
	return NULL;
}

#ifndef DEFINED_seq_max_foreach_cb
#define DEFINED_seq_max_foreach_cb
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
#endif /* !DEFINED_seq_max_foreach_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max__with__seq_operator_foreach(DeeObject *__restrict self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_max_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}


/* seq_max_with_key */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_max_with_key__with_callattr_max(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObject_CallAttr(self, (DeeObject *)&str_max, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_max_with_key__with_callattr___seq_max__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_max__, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_max_with_key__with_callobjectcache___seq_max__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_object, self, 3, args);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_max_with_key__with_callmethodcache___seq_max__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_method, self, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_max_with_key__with_callkwmethodcache___seq_max__(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_kwmethod, self, 3, args, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_max_with_key__unsupported(DeeObject *self, DeeObject *key) {
	err_seq_unsupportedf(self, "__seq_max__(%r)", key);
	return NULL;
}

#ifndef DEFINED_seq_minmax_with_key_data
#define DEFINED_seq_minmax_with_key_data
struct seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};
#endif /* !DEFINED_seq_minmax_with_key_data */
#ifndef DEFINED_seq_max_with_key_foreach_cb
#define DEFINED_seq_max_with_key_foreach_cb
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
#endif /* !DEFINED_seq_max_with_key_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_max_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_max_with_key_foreach_cb, &data);
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


/* seq_max_with_range */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max_with_range__with_callattr_max(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_max, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max_with_range__with_callattr___seq_max__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_max__, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max_with_range__with_callobjectcache___seq_max__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max_with_range__with_callmethodcache___seq_max__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max_with_range__with_callkwmethodcache___seq_max__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_max__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

#ifndef DEFINED_seq_max_foreach_cb
#define DEFINED_seq_max_foreach_cb
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
#endif /* !DEFINED_seq_max_foreach_cb */
#ifndef DEFINED_seq_max_enumerate_cb
#define DEFINED_seq_max_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_max_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_max_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_max_enumerate_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_max_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_max_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}


/* seq_max_with_range_and_key */
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_max_with_range_and_key__with_callattr_max(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_max, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_max_with_range_and_key__with_callattr___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_max__, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_max_with_range_and_key__with_callobjectcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_object, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_max_with_range_and_key__with_callmethodcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_method, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_max_with_range_and_key__with_callkwmethodcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_max__.c_kwmethod, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_max_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "__seq_max__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
	return NULL;
}

#ifndef DEFINED_seq_minmax_with_key_data
#define DEFINED_seq_minmax_with_key_data
struct seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};
#endif /* !DEFINED_seq_minmax_with_key_data */
#ifndef DEFINED_seq_max_with_key_foreach_cb
#define DEFINED_seq_max_with_key_foreach_cb
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
#endif /* !DEFINED_seq_max_with_key_foreach_cb */
#ifndef DEFINED_seq_max_with_key_enumerate_cb
#define DEFINED_seq_max_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_max_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_max_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_max_with_key_enumerate_cb */
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_max_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_max_with_key_enumerate_cb, &data, start, end);
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


/* seq_sum */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__with_callattr_sum(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_sum, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__with_callattr___seq_sum__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_sum__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__with_callobjectcache___seq_sum__(DeeObject *__restrict self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__with_callmethodcache___seq_sum__(DeeObject *__restrict self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__with_callkwmethodcache___seq_sum__(DeeObject *__restrict self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "__seq_sum__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__empty(DeeObject *__restrict self) {
	return_none;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__with__seq_operator_foreach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	struct Dee_accu accu;
	Dee_accu_init(&accu);
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &Dee_accu_add, &accu);
	if unlikely(foreach_status < 0)
		goto err;
	return Dee_accu_pack(&accu);
err:
	Dee_accu_fini(&accu);
	return NULL;
}


/* seq_sum_with_range */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__with_callattr_sum(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_sum, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__with_callattr___seq_sum__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_sum__, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__with_callobjectcache___seq_sum__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__.c_object, self, PCKuSIZ PCKuSIZ, start, end);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__with_callmethodcache___seq_sum__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__.c_method, self, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__with_callkwmethodcache___seq_sum__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__.c_kwmethod, self, PCKuSIZ PCKuSIZ, start, end);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_sum__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__empty(DeeObject *__restrict self, size_t start, size_t end) {
	return_none;
}

#ifndef DEFINED_seq_sum_enumerate_cb
#define DEFINED_seq_sum_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_sum_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return Dee_accu_add(arg, item);
}
#endif /* !DEFINED_seq_sum_enumerate_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct Dee_accu accu;
	Dee_accu_init(&accu);
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_sum_enumerate_cb, &accu, start, end);
	if unlikely(foreach_status < 0)
		goto err;
	return Dee_accu_pack(&accu);
err:
	Dee_accu_fini(&accu);
	return NULL;
}


/* seq_count */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with_callattr_count(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_count, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with_callattr___seq_count__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_count__, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_object, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_method, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_kwmethod, self, 1, &item, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__unsupported(DeeObject *self, DeeObject *item) {
	return (size_t)err_seq_unsupportedf(self, "__seq_count__(%r)", item);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with__set_operator_contains(DeeObject *self, DeeObject *item) {
	DREF DeeObject *contains = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_contains))(self, item);
	if unlikely(!contains)
		goto err;
	return (size_t)DeeObject_BoolInherited(contains);
err:
	return (size_t)-1;
}

#ifndef DEFINED_seq_count_foreach_cb
#define DEFINED_seq_count_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_count_foreach_cb(void *arg, DeeObject *item) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}
#endif /* !DEFINED_seq_count_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with__seq_operator_foreach(DeeObject *self, DeeObject *item) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_count_foreach_cb, item);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count__with__seq_find(DeeObject *self, DeeObject *item) {
	return default__seq_count_with_range__with__seq_find(self, item, 0, (size_t)-1);
}


/* seq_count_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__with_callattr_count(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_count, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__with_callattr___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_count__, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_object, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_method, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_kwmethod, self, 4, args, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key) {
	return (size_t)err_seq_unsupportedf(self, "count(%r, key: %r)", item, key);
}

#ifndef DEFINED_seq_count_with_key_foreach_cb
#define DEFINED_seq_count_with_key_foreach_cb
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
#endif /* !DEFINED_seq_count_with_key_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_count_with_key_foreach_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
default__seq_count_with_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, DeeObject *key) {
	return default__seq_count_with_range_and_key__with__seq_find_with_key(self, item, 0, (size_t)-1, key);
}


/* seq_count_with_range */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__with_callattr_count(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_count, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__with_callattr___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_count__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_object, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_method, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return (size_t)err_seq_unsupportedf(self, "__seq_count__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

#ifndef DEFINED_seq_count_foreach_cb
#define DEFINED_seq_count_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_count_foreach_cb(void *arg, DeeObject *item) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}
#endif /* !DEFINED_seq_count_foreach_cb */
#ifndef DEFINED_seq_count_enumerate_cb
#define DEFINED_seq_count_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_count_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_count_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_count_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_count_enumerate_cb, item, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_count_with_range__with__seq_find(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result = 0;
	DeeMH_seq_find_t seq_find = DeeType_RequireSeqFind(Dee_TYPE(self));
	while (start < end) {
		size_t match = (*seq_find)(self, item, start, end);
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


/* seq_count_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__with_callattr_count(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_count, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__with_callattr___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_count__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_count__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return (size_t)err_seq_unsupportedf(self, "__seq_count__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

#ifndef DEFINED_seq_count_with_key_foreach_cb
#define DEFINED_seq_count_with_key_foreach_cb
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
#endif /* !DEFINED_seq_count_with_key_foreach_cb */
#ifndef DEFINED_seq_count_with_key_enumerate_cb
#define DEFINED_seq_count_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_count_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_count_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_count_with_key_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_count_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_count_with_range_and_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result = 0;
	DeeMH_seq_find_with_key_t seq_find_with_key = DeeType_RequireSeqFindWithKey(Dee_TYPE(self));
	while (start < end) {
		size_t match = (*seq_find_with_key)(self, item, start, end, key);
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


/* seq_contains */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with_callattr___seq_contains__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_contains__, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_object, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_method, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_kwmethod, self, 1, &item, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__unsupported(DeeObject *self, DeeObject *item) {
	return err_seq_unsupportedf(self, "__seq_contains__(%r)", item);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with__seq_operator_contains(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_contains))(self, item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with__map_operator_trygetitem(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeObject *real_value;
	DREF DeeObject *key_and_value[2];
	if (DeeObject_Unpack(item, 2, key_and_value))
		goto err_trycatch;
	real_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if unlikely(!real_value) {
		Dee_Decref(key_and_value[1]);
		goto err;
	}
	result = DeeObject_TryCompareEq(key_and_value[1], real_value);
	Dee_Decref(key_and_value[1]);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_trycatch:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return 0;
err:
	return -1;
}

#ifndef DEFINED_default_contains_with_foreach_cb
#define DEFINED_default_contains_with_foreach_cb
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_contains_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_contains_with_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with__seq_operator_foreach(DeeObject *self, DeeObject *item) {
	Dee_ssize_t status;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_contains_with_foreach_cb, item);
	if unlikely(status == -1)
		goto err;
	return status /*== -2*/ ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains__with__seq_find(DeeObject *self, DeeObject *item) {
	return default__seq_contains_with_range__with__seq_find(self, item, 0, (size_t)-1);
}


/* seq_contains_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_contains_with_key__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_contains__, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_contains_with_key__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_object, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_contains_with_key__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_method, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_contains_with_key__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_kwmethod, self, 4, args, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_contains_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_contains__(%r, %r)", item, key);
}

#ifndef DEFINED_seq_count_with_key_foreach_cb
#define DEFINED_seq_count_with_key_foreach_cb
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
#endif /* !DEFINED_seq_count_with_key_foreach_cb */
#ifndef DEFINED_seq_contains_with_key_foreach_cb
#define DEFINED_seq_contains_with_key_foreach_cb
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
#endif /* !DEFINED_seq_contains_with_key_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_contains_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_contains_with_key_foreach_cb, &data);
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

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_contains_with_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, DeeObject *key) {
	return default__seq_contains_with_range_and_key__with__seq_find_with_key(self, item, 0, (size_t)-1, key);
}


/* seq_contains_with_range */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_contains__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_object, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_method, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_contains__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

#ifndef DEFINED_default_contains_with_foreach_cb
#define DEFINED_default_contains_with_foreach_cb
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_contains_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_contains_with_foreach_cb */
#ifndef DEFINED_seq_contains_enumerate_cb
#define DEFINED_seq_contains_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_contains_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_contains_with_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_contains_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_contains_enumerate_cb, item, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__with__seq_find(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t match = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find))(self, item, start, end);
	if unlikely(match == (size_t)Dee_COMPARE_ERR)
		goto err;
	return match != (size_t)-1 ? 1 : 0;
err:
	return -1;
}


/* seq_contains_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_contains_with_range_and_key__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_contains__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_contains_with_range_and_key__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_contains_with_range_and_key__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_contains_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_contains__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

#ifndef DEFINED_seq_count_with_key_foreach_cb
#define DEFINED_seq_count_with_key_foreach_cb
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
#endif /* !DEFINED_seq_count_with_key_foreach_cb */
#ifndef DEFINED_seq_contains_with_key_foreach_cb
#define DEFINED_seq_contains_with_key_foreach_cb
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
#endif /* !DEFINED_seq_contains_with_key_foreach_cb */
#ifndef DEFINED_seq_contains_with_key_enumerate_cb
#define DEFINED_seq_contains_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_contains_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_contains_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_contains_with_key_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_contains_with_range_and_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_contains_with_key_enumerate_cb, &data, start, end);
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

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_contains_with_range_and_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t match = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find_with_key))(self, item, start, end, key);
	if unlikely(match == (size_t)Dee_COMPARE_ERR)
		goto err;
	return match != (size_t)-1 ? 1 : 0;
err:
	return -1;
}


/* seq_operator_contains */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_contains(DeeObject *self, DeeObject *item) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_contains))(self, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_contains__unsupported(DeeObject *self, DeeObject *item) {
	err_seq_unsupportedf(self, "operator contains(%r)", item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_contains__empty(DeeObject *self, DeeObject *item) {
	return_false;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_contains__with__seq_contains(DeeObject *self, DeeObject *item) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains))(self, item);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}


/* seq_locate */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__with_callattr_locate(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObject_CallAttr(self, (DeeObject *)&str_locate, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__with_callattr___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_locate__, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__with_callobjectcache___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__.c_object, self, 2, args);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__with_callmethodcache___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__.c_method, self, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__with_callkwmethodcache___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__.c_kwmethod, self, 2, args, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_locate__(%r, %r)", match, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__empty(DeeObject *self, DeeObject *match, DeeObject *def) {
	return_reference_(def);
}

#ifndef DEFINED_seq_locate_foreach_cb
#define DEFINED_seq_locate_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	DeeObject *match = *(DeeObject **)arg;
	match_result_ob = DeeObject_Call(match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		*(DeeObject **)arg = item;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_locate_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__with__seq_operator_foreach(DeeObject *self, DeeObject *match, DeeObject *def) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_locate_foreach_cb, &match);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}


/* seq_locate_with_range */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__with_callattr_locate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_locate, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__with_callattr___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_locate__, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__with_callobjectcache___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__with_callmethodcache___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__with_callkwmethodcache___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_locate__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", match, start, end, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__empty(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return_reference_(def);
}

#ifndef DEFINED_seq_locate_foreach_cb
#define DEFINED_seq_locate_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	DeeObject *match = *(DeeObject **)arg;
	match_result_ob = DeeObject_Call(match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		*(DeeObject **)arg = item;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_locate_foreach_cb */
#ifndef DEFINED_seq_locate_enumerate_index_cb
#define DEFINED_seq_locate_enumerate_index_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_locate_enumerate_index_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_locate_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_locate_enumerate_index_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_locate_enumerate_index_cb, &match, start, end);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}


/* seq_rlocate */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__with_callattr_rlocate(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObject_CallAttr(self, (DeeObject *)&str_rlocate, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__with_callattr___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_rlocate__, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__with_callobjectcache___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__.c_object, self, 2, args);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__with_callmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__.c_method, self, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__with_callkwmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__.c_kwmethod, self, 2, args, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_rlocate__(%r, %r)", match, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__empty(DeeObject *self, DeeObject *match, DeeObject *def) {
	return_reference_(def);
}

#ifndef DEFINED_seq_locate_foreach_cb
#define DEFINED_seq_locate_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	DeeObject *match = *(DeeObject **)arg;
	match_result_ob = DeeObject_Call(match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		*(DeeObject **)arg = item;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_locate_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__with__seq_foreach_reverse(DeeObject *self, DeeObject *match, DeeObject *def) {
	Dee_ssize_t foreach_status;
	DeeMH_seq_foreach_reverse_t rforeach;
	rforeach = DeeType_TryRequireSeqForeachReverse(Dee_TYPE(self));
	ASSERT(rforeach);
	foreach_status = (*rforeach)(self, &seq_locate_foreach_cb, &match);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}

#ifndef DEFINED_seq_rlocate_foreach_cb
#define DEFINED_seq_rlocate_foreach_cb
struct seq_rlocate_with_foreach_data {
	DeeObject      *gsrlwf_match;  /* [1..1] Matching function. */
	DREF DeeObject *gsrlwf_result; /* [1..1] Match result. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_rlocate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	struct seq_rlocate_with_foreach_data *data;
	data = (struct seq_rlocate_with_foreach_data *)arg;
	match_result_ob = DeeObject_Call(data->gsrlwf_match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		Dee_Decref(data->gsrlwf_result);
		data->gsrlwf_result = item;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_rlocate_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__with__seq_operator_foreach(DeeObject *self, DeeObject *match, DeeObject *def) {
	Dee_ssize_t foreach_status;
	struct seq_rlocate_with_foreach_data data;
	data.gsrlwf_match  = match;
	data.gsrlwf_result = def;
	Dee_Incref(def);
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_rlocate_foreach_cb, &data);
	if likely(foreach_status == 0)
		return data.gsrlwf_result;
	Dee_Decref_unlikely(data.gsrlwf_result);
	return NULL;
}


/* seq_rlocate_with_range */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__with_callattr_rlocate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_rlocate, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__with_callattr___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_rlocate__, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__with_callmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__with_callkwmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_rlocate__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", match, start, end, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__empty(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return_reference_(def);
}

#ifndef DEFINED_seq_locate_foreach_cb
#define DEFINED_seq_locate_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	DeeObject *match = *(DeeObject **)arg;
	match_result_ob = DeeObject_Call(match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		*(DeeObject **)arg = item;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_locate_foreach_cb */
#ifndef DEFINED_seq_locate_enumerate_index_cb
#define DEFINED_seq_locate_enumerate_index_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_locate_enumerate_index_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_locate_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_locate_enumerate_index_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	Dee_ssize_t foreach_status;
	DeeMH_seq_enumerate_index_reverse_t renum;
	renum = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(renum);
	foreach_status = (*renum)(self, &seq_locate_enumerate_index_cb, &match, start, end);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}

#ifndef DEFINED_seq_rlocate_foreach_cb
#define DEFINED_seq_rlocate_foreach_cb
struct seq_rlocate_with_foreach_data {
	DeeObject      *gsrlwf_match;  /* [1..1] Matching function. */
	DREF DeeObject *gsrlwf_result; /* [1..1] Match result. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_rlocate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	struct seq_rlocate_with_foreach_data *data;
	data = (struct seq_rlocate_with_foreach_data *)arg;
	match_result_ob = DeeObject_Call(data->gsrlwf_match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		Dee_Decref(data->gsrlwf_result);
		data->gsrlwf_result = item;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_rlocate_foreach_cb */
#ifndef DEFINED_seq_rlocate_enumerate_index_cb
#define DEFINED_seq_rlocate_enumerate_index_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_rlocate_enumerate_index_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_rlocate_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_rlocate_enumerate_index_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	Dee_ssize_t foreach_status;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_rlocate_enumerate_index_cb, &match, start, end);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}


/* seq_startswith */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith__with_callattr_startswith(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_startswith, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_startswith__, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_object, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_method, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_kwmethod, self, 1, &item, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith__unsupported(DeeObject *self, DeeObject *item) {
	return err_seq_unsupportedf(self, "__seq_startswith__(%r)", item);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith__with__seq_trygetfirst(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeObject *first = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetfirst))(self);
	if (first == ITER_DONE)
		return 0;
	if unlikely(!first)
		goto err;
	result = DeeObject_TryCompareEq(item, first);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}


/* seq_startswith_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_startswith_with_key__with_callattr_startswith(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_startswith, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_startswith_with_key__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_startswith__, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_startswith_with_key__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_object, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_startswith_with_key__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_method, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_startswith_with_key__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_kwmethod, self, 4, args, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_startswith_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_startswith__(%r, %r)", item, key);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_startswith_with_key__with__seq_trygetfirst(DeeObject *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *first;
	first = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetfirst))(self);
	if unlikely(!first)
		goto err;
	if (first == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_first;
	result = DeeObject_TryCompareKeyEq(item, first, key);
	Dee_Decref(item);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_first:
	Dee_Decref(first);
err:
	return -1;
}


/* seq_startswith_with_range */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith_with_range__with_callattr_startswith(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_startswith, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith_with_range__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_startswith__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith_with_range__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_object, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith_with_range__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_method, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith_with_range__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_startswith__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_startswith_with_range__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return 0;
	selfitem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, start);
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


/* seq_startswith_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_startswith_with_range_and_key__with_callattr_startswith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_startswith, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_startswith_with_range_and_key__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_startswith__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_startswith_with_range_and_key__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_startswith_with_range_and_key__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_startswith_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_startswith__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_startswith_with_range_and_key__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return 0;
	selfitem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, start);
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


/* seq_endswith */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith__with_callattr_endswith(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_endswith, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_endswith__, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_object, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_method, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_kwmethod, self, 1, &item, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith__unsupported(DeeObject *self, DeeObject *item) {
	return err_seq_unsupportedf(self, "__seq_endswith__(%r)", item);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith__with__seq_trygetlast(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeObject *last = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetlast))(self);
	if (last == ITER_DONE)
		return 0;
	if unlikely(!last)
		goto err;
	result = DeeObject_TryCompareEq(item, last);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}


/* seq_endswith_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_endswith_with_key__with_callattr_endswith(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_endswith, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_endswith_with_key__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_endswith__, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_endswith_with_key__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_object, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_endswith_with_key__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_method, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_endswith_with_key__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_kwmethod, self, 4, args, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_endswith_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_endswith__(%r, %r)", item, key);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_endswith_with_key__with__seq_trygetlast(DeeObject *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *last;
	last = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetlast))(self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_last;
	result = DeeObject_TryCompareKeyEq(item, last, key);
	Dee_Decref(item);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_last:
	Dee_Decref(last);
err:
	return -1;
}


/* seq_endswith_with_range */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith_with_range__with_callattr_endswith(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_endswith, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith_with_range__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_endswith__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith_with_range__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_object, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith_with_range__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_method, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith_with_range__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_endswith__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_endswith_with_range__with__seq_operator_size__and__operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, end - 1);
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


/* seq_endswith_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_endswith_with_range_and_key__with_callattr_endswith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_endswith, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_endswith_with_range_and_key__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_endswith__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_endswith_with_range_and_key__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_endswith_with_range_and_key__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_endswith_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_endswith__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_endswith_with_range_and_key__with__seq_operator_size__and__operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, end - 1);
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


/* seq_find */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_find__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_find, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_find__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_find__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_find__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_find__.c_object, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_find__with_callmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_find__.c_method, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_find__with_callkwmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_find__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_find__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return (size_t)err_seq_unsupportedf(self, "__seq_find__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

#ifndef DEFINED_seq_find_cb
#define DEFINED_seq_find_cb
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
#endif /* !DEFINED_seq_find_cb */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_find__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t status;
	union seq_find_data data;
	data.gsfd_elem = item;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_find_cb, &data, start, end);
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


/* seq_find_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_find_with_key__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_find, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_find_with_key__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_find__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_find_with_key__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_find__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_find_with_key__with_callmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_find__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_find_with_key__with_callkwmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_find__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_find_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return (size_t)err_seq_unsupportedf(self, "__seq_find__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

#ifndef DEFINED_seq_find_with_key_cb
#define DEFINED_seq_find_with_key_cb
struct seq_find_with_key_data {
	union seq_find_data gsfwk_base; /* Base find data */
	DeeObject          *gsfwk_key;  /* Find element key */
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
#endif /* !DEFINED_seq_find_with_key_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_find_with_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct seq_find_with_key_data data;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_find_with_key_cb, &data, start, end);
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


/* seq_rfind */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__with_callattr_rfind(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_rfind, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__with_callattr___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_rfind__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__with_callobjectcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__.c_object, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__with_callmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__.c_method, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__with_callkwmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return (size_t)err_seq_unsupportedf(self, "__seq_rfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

#ifndef DEFINED_seq_find_cb
#define DEFINED_seq_find_cb
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
#endif /* !DEFINED_seq_find_cb */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t status;
	union seq_find_data data;
	DeeMH_seq_enumerate_index_reverse_t renum;
	data.gsfd_elem = item;
	renum = DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index_reverse);
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

#ifndef DEFINED_seq_rfind_cb
#define DEFINED_seq_rfind_cb
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
#endif /* !DEFINED_seq_rfind_cb */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t status;
	struct seq_rfind_data data;
	data.gsrfd_elem   = item;
	data.gsrfd_result = (size_t)-1;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_rfind_cb, &data, start, end);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.gsrfd_result == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return data.gsrfd_result;
err:
	return (size_t)Dee_COMPARE_ERR;
}


/* seq_rfind_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__with_callattr_rfind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_rfind, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__with_callattr___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_rfind__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__with_callobjectcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__.c_object, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__with_callmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__.c_method, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__with_callkwmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__.c_kwmethod, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return (size_t)err_seq_unsupportedf(self, "__seq_rfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

#ifndef DEFINED_seq_find_with_key_cb
#define DEFINED_seq_find_with_key_cb
struct seq_find_with_key_data {
	union seq_find_data gsfwk_base; /* Base find data */
	DeeObject          *gsfwk_key;  /* Find element key */
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
#endif /* !DEFINED_seq_find_with_key_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct seq_find_with_key_data data;
	DeeMH_seq_enumerate_index_reverse_t renum;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	renum = DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index_reverse);
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

#ifndef DEFINED_seq_rfind_with_key_cb
#define DEFINED_seq_rfind_with_key_cb
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
#endif /* !DEFINED_seq_rfind_with_key_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct seq_rfind_with_key_data data;
	data.gsrfwkd_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsrfwkd_kelem)
		goto err;
	data.gsrfwkd_result = (size_t)-1;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_rfind_with_key_cb, &data, start, end);
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


/* seq_erase */
INTERN WUNUSED NONNULL((1)) int DCALL
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

INTERN WUNUSED NONNULL((1)) int DCALL
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

INTERN WUNUSED NONNULL((1)) int DCALL
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
INTERN WUNUSED NONNULL((1)) int DCALL
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

INTERN WUNUSED NONNULL((1)) int DCALL
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

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_erase__unsupported(DeeObject *__restrict self, size_t index, size_t count) {
	return err_seq_unsupportedf(self, "__seq_erase__(%" PRFuSIZ ", %" PRFuSIZ ")", index, count);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_erase__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index, size_t count) {
	size_t end_index;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index))(self, (Dee_ssize_t)index, (Dee_ssize_t)end_index);
err_overflow:
	return err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
}

INTERN WUNUSED NONNULL((1)) int DCALL
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
INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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
INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_insert__unsupported(DeeObject *self, size_t index, DeeObject *item) {
	return err_seq_unsupportedf(self, "__seq_insert__(%" PRFuSIZ ", %r)", index, item);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_insert__with__seq_insertall(DeeObject *self, size_t index, DeeObject *item) {
	int result;
	DREF DeeTupleObject *item_tuple;
	item_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!item_tuple)
		goto err;
	DeeTuple_SET(item_tuple, 0, item);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insertall))(self, index, (DeeObject *)item_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)item_tuple);
	return result;
err:
	return -1;
}


/* seq_insertall */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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
INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_insertall__unsupported(DeeObject *self, size_t index, DeeObject *items) {
	return err_seq_unsupportedf(self, "__seq_insertall__(%" PRFuSIZ ", %r)", index, items);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_insertall__empty(DeeObject *self, size_t index, DeeObject *items) {
	int items_empty = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bool))(items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_insertall__unsupported(self, index, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_insertall__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, (Dee_ssize_t)index, (Dee_ssize_t)index, items);
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
INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_insertall__with__seq_insert(DeeObject *self, size_t index, DeeObject *items) {
	struct seq_insertall_with_foreach_insert_data data;
	data.dsiawfid_insert = DeeType_RequireSeqInsert(Dee_TYPE(self));
	data.dsiawfid_self   = self;
	data.dsiawfid_index  = index;
	return (int)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(items, &seq_insertall_with_foreach_insert_cb, &data);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insert))(self, 0, item);
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
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_extend))(self, (DeeObject *)item_tuple);
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
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, size, size, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_extend__with__seq_operator_size__and__seq_insertall(DeeObject *self, DeeObject *items) {
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insertall))(self, size, items);
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
	return (int)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(items, &seq_extend_with_foreach_append_cb, &data);
}


/* seq_xchitem_index */
INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callattr_xchitem(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_xchitem, PCKuSIZ "o", index, item);
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callattr___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_xchitem__, PCKuSIZ "o", index, item);
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callobjectcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_xchitem__.c_object, self, PCKuSIZ "o", index, item);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_xchitem__.c_method, self, PCKuSIZ "o", index, item);
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with_callkwmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_xchitem__.c_kwmethod, self, PCKuSIZ "o", index, item);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__unsupported(DeeObject *self, size_t index, DeeObject *item) {
	err_seq_unsupportedf(self, "__seq_xchitem__(%" PRFuSIZ ", %r)", index, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
default__seq_xchitem_index__with__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, index);
	if likely(result) {
		if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, index, item))
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange))(self, DeeInt_Zero, Dee_None);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_operator_delrange_index_n(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index_n))(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_erase(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, 0, (size_t)-1);
}


/* seq_pop */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__with_callattr_pop(DeeObject *self, Dee_ssize_t index) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_pop, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__with_callattr___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_pop__, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__with_callobjectcache___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeObject_ThisCallf(Dee_TYPE(self)->tp_mhcache->mhc___seq_pop__.c_object, self, PCKdSIZ, index);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__with_callmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_pop__.c_method, self, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__with_callkwmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index) {
	return DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_mhcache->mhc___seq_pop__.c_kwmethod, self, PCKdSIZ, index);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__unsupported(DeeObject *self, Dee_ssize_t index) {
	err_seq_unsupportedf(self, "__seq_pop__(%" PRFdSIZ ")", index);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase(DeeObject *self, Dee_ssize_t index) {
	size_t used_index = (size_t)index;
	DREF DeeObject *result;
	if (index < 0) {
		size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, size);
	}
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, used_index);
	if likely(result) {
		if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, index, 1))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index(DeeObject *self, Dee_ssize_t index) {
	size_t used_index = (size_t)index;
	DREF DeeObject *result;
	if (index < 0) {
		size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, size);
	}
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, used_index);
	if likely(result) {
		if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, index))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


/* set_operator_iter */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_iter))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__with_callattr___set_iter__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___set_iter__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__with_callobjectcache___set_iter__(DeeObject *__restrict self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___set_iter__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__with_callmethodcache___set_iter__(DeeObject *__restrict self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___set_iter__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__with_callkwmethodcache___set_iter__(DeeObject *__restrict self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___set_iter__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__unsupported(DeeObject *__restrict self) {
	err_set_unsupportedf(self, "operator iter()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__with__seq_operator_iter(DeeObject *__restrict self) {
	DREF DeeObject *iter;
	DREF DistinctIterator *result;
	iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	result = DeeGCObject_MALLOC(DistinctIterator);
	if unlikely(!result)
		goto err_iter;
	result->di_tp_next = Dee_TYPE(iter)->tp_iter_next;
	if unlikely(!result->di_tp_next) {
		if unlikely(!DeeType_InheritIterNext(Dee_TYPE(iter))) {
			err_unimplemented_operator(Dee_TYPE(iter), OPERATOR_ITERNEXT);
			goto err_iter_result;
		}
		result->di_tp_next = Dee_TYPE(iter)->tp_iter_next;
		ASSERT(result->di_tp_next);
	}
	result->di_iter = iter; /* Inherit reference */
	Dee_simple_hashset_with_lock_init(&result->di_encountered);
	DeeObject_Init(result, &DistinctIterator_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err_iter_result:
	DeeGCObject_FREE(result);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}


/* set_operator_foreach */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__set_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_foreach))(self, cb, arg);
}

#ifndef DEFINED_default_set_foreach_unique_cb
#define DEFINED_default_set_foreach_unique_cb
struct default_set_foreach_unique_cb_data {
	Dee_foreach_t dsfucd_cb;  /* [1..1] user-defined callback */
	void         *dsfucd_arg; /* [?..?] Cookie for `dsfucd_cb' */
};

struct default_set_foreach_unique_data {
	struct Dee_simple_hashset                 dsfud_encountered; /* Set of objects already encountered. */
	struct default_set_foreach_unique_cb_data dsfud_cb;          /* Callback data */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_foreach_unique_cb(void *arg, DeeObject *item) {
	int insert_status;
	struct default_set_foreach_unique_data *data;
	data = (struct default_set_foreach_unique_data *)arg;
	insert_status = Dee_simple_hashset_insert(&data->dsfud_encountered, item);
	if likely(insert_status > 0)
		return (*data->dsfud_cb.dsfucd_cb)(data->dsfud_cb.dsfucd_arg, item);
	return insert_status; /* error, or already-exists */
}
#endif /* !DEFINED_default_set_foreach_unique_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__set_operator_foreach__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	struct default_set_foreach_unique_data data;
	data.dsfud_cb.dsfucd_cb  = cb;
	data.dsfud_cb.dsfucd_arg = arg;
	Dee_simple_hashset_init(&data.dsfud_encountered);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_set_foreach_unique_cb, &data);
	Dee_simple_hashset_fini(&data.dsfud_encountered);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__set_operator_foreach__with__set_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}


/* set_operator_foreach_pair */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__set_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_foreach_pair))(self, cb, arg);
}

#ifndef DECLARED_default_foreach_pair_with_foreach_cb
#define DECLARED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DECLARED_default_foreach_pair_with_foreach_cb */
#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_foreach_pair_with_foreach_data *data;
	Dee_ssize_t result;
	data = (struct default_foreach_pair_with_foreach_data *)arg;
	if likely(DeeTuple_Check(elem) && DeeTuple_SIZE(elem) == 2) {
		result = (*data->dfpwf_cb)(data->dfpwf_arg,
		                           DeeTuple_GET(elem, 0),
		                           DeeTuple_GET(elem, 1));
	} else {
		DREF DeeObject *pair[2];
		if unlikely(DeeObject_Unpack(elem, 2, pair))
			goto err;
		result = (*data->dfpwf_cb)(data->dfpwf_arg, pair[0], pair[1]);
		Dee_Decref_unlikely(pair[1]);
		Dee_Decref_unlikely(pair[0]);
	}
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_pair_with_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__set_operator_foreach_pair__with__set_operator_foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_foreach))(self, &default_foreach_pair_with_foreach_cb, &data);
}


/* set_operator_sizeob */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with_callattr___set_size__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___set_size__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with_callobjectcache___set_size__(DeeObject *self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___set_size__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with_callmethodcache___set_size__(DeeObject *self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___set_size__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with_callkwmethodcache___set_size__(DeeObject *self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___set_size__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__unsupported(DeeObject *self) {
	err_set_unsupportedf(self, "operator size()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__empty(DeeObject *self) { return_reference_(DeeInt_Zero); }

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with__set_operator_size(DeeObject *self) {
	size_t setsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_size))(self);
	if unlikely(setsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(setsize);
err:
	return NULL;
}


/* set_operator_size */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_size))(self);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size__unsupported(DeeObject *self) {
	return (size_t)err_set_unsupportedf(self, "operator size()");
}

#ifndef DEFINED_default_seq_size_with_foreach_cb
#define DEFINED_default_seq_size_with_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return 1;
}
#endif /* !DEFINED_default_seq_size_with_foreach_cb */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size__with__set_operator_foreach(DeeObject *self) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_foreach))(self, &default_seq_size_with_foreach_cb, NULL);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size__with__set_operator_sizeob(DeeObject *self) {
	DREF DeeObject *sizeob;
	sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}


/* set_operator_hash */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_hash))(self);
}

#ifndef DEFINED_DeeSet_HandleHashError
#define DEFINED_DeeSet_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSet_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with_callattr___set_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(self, (DeeObject *)&str___set_hash__, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSet_HandleHashError(self);
}

#ifndef DEFINED_DeeSet_HandleHashError
#define DEFINED_DeeSet_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSet_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with_callobjectcache___set_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___set_hash__.c_object, self, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSet_HandleHashError(self);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#ifndef DEFINED_DeeSet_HandleHashError
#define DEFINED_DeeSet_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSet_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with_callmethodcache___set_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___set_hash__.c_method, self, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSet_HandleHashError(self);
}

#ifndef DEFINED_DeeSet_HandleHashError
#define DEFINED_DeeSet_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSet_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with_callkwmethodcache___set_hash__(DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___set_hash__.c_kwmethod, self, 0, NULL, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSet_HandleHashError(self);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__empty(DeeObject *self) {
	return DEE_HASHOF_EMPTY_SEQUENCE;
}

#ifndef DEFINED_default_set_hash_with_foreach_cb
#define DEFINED_default_set_hash_with_foreach_cb
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_hash_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_set_hash_with_foreach_cb */
#ifndef DEFINED_DeeSet_HandleHashError
#define DEFINED_DeeSet_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSet_HandleHashError */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with__set_operator_foreach(DeeObject *self) {
	Dee_hash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_foreach))(self, &default_set_hash_with_foreach_cb, &result))
		goto err;
	return result;
err:
	return DeeSet_HandleHashError(self);
}


/* map_operator_getitem */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem(DeeObject *self, DeeObject *key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with_callattr___map_getitem__(DeeObject *self, DeeObject *key) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_getitem__, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with_callobjectcache___map_getitem__(DeeObject *self, DeeObject *key) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___map_getitem__.c_object, self, 1, &key);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with_callmethodcache___map_getitem__(DeeObject *self, DeeObject *key) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_getitem__.c_method, self, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with_callkwmethodcache___map_getitem__(DeeObject *self, DeeObject *key) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_getitem__.c_kwmethod, self, 1, &key, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__unsupported(DeeObject *self, DeeObject *key) {
	err_map_unsupportedf(self, "operator [](%r)", key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__empty(DeeObject *self, DeeObject *key) {
	err_unknown_key(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_index))(self, key_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_hash(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_index))(self, key_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with__map_operator_getitem_string_len_hash(DeeObject *self, DeeObject *key) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with__map_operator_getitem_string_hash(DeeObject *self, DeeObject *key) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with__map_operator_getitem_index(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_index))(self, key_value);
err:
	return NULL;
}

#ifndef DEFINED_default_map_getitem_with_enumerate_cb
#define DEFINED_default_map_getitem_with_enumerate_cb
struct default_map_getitem_with_enumerate_data {
	DeeObject      *mgied_key;    /* [1..1] The key we're looking for. */
	DREF DeeObject *mgied_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_with_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__with__map_enumerate(DeeObject *self, DeeObject *key) {
	struct default_map_getitem_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgied_key = key;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if unlikely(status == -3) {
		err_unbound_key(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key(self, key);
err:
	return NULL;
}


/* map_operator_trygetitem */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem(DeeObject *self, DeeObject *key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__empty(DeeObject *self, DeeObject *key) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_index))(self, key_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_index))(self, key_value);
err:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return ITER_DONE;
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__with__map_operator_trygetitem_string_len_hash(DeeObject *self, DeeObject *key) {
	if (!DeeString_Check(key))
		return ITER_DONE;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key));
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__with__map_operator_trygetitem_string_hash(DeeObject *self, DeeObject *key) {
	if (!DeeString_Check(key))
		return ITER_DONE;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key));
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__with__map_operator_trygetitem_index(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_index))(self, key_value);
err:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return ITER_DONE;
	return NULL;
}

#ifndef DEFINED_default_map_getitem_with_enumerate_cb
#define DEFINED_default_map_getitem_with_enumerate_cb
struct default_map_getitem_with_enumerate_data {
	DeeObject      *mgied_key;    /* [1..1] The key we're looking for. */
	DREF DeeObject *mgied_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_with_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__with__map_enumerate(DeeObject *self, DeeObject *key) {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	data.mgied_key = key;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	ASSERT(status == -1);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__with__map_operator_getitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, key);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}


/* map_operator_getitem_index */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_index(DeeObject *self, size_t key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_index))(self, key);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_index__unsupported(DeeObject *self, size_t key) {
	err_map_unsupportedf(self, "operator [](%" PRFuSIZ ")", key);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_index__empty(DeeObject *self, size_t key) {
	err_unknown_key_int(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_index__with__map_operator_getitem(DeeObject *self, size_t key) {
	DREF DeeObject *result, *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}


/* map_operator_trygetitem_index */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_index(DeeObject *self, size_t key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_index))(self, key);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_index__empty(DeeObject *self, size_t key) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_index__with__map_operator_trygetitem(DeeObject *self, size_t key) {
	DREF DeeObject *result, *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}


/* map_operator_getitem_string_hash */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_hash))(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash) {
	err_map_unsupportedf(self, "operator [](%q)", key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash) {
	err_unknown_key_str(self, key);
	return NULL;
}

#ifndef DEFINED_default_map_getitem_string_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_hash_with_enumerate_cb
struct default_map_getitem_string_hash_with_enumerate_data {
	char const     *mgished_key;    /* [1..1] The key we're looking for. */
	Dee_hash_t      mgished_hash;   /* Hash for `mgished_key'. */
	DREF DeeObject *mgished_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_string_hash_with_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash__with__map_enumerate(DeeObject *self, char const *key, Dee_hash_t hash) {
	struct default_map_getitem_string_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	if unlikely(status == -3) {
		err_unbound_key_str(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str(self, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash__with__map_operator_getitem(DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}


/* map_operator_trygetitem_string_hash */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_hash))(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash) {
	return ITER_DONE;
}

#ifndef DEFINED_default_map_getitem_string_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_hash_with_enumerate_cb
struct default_map_getitem_string_hash_with_enumerate_data {
	char const     *mgished_key;    /* [1..1] The key we're looking for. */
	Dee_hash_t      mgished_hash;   /* Hash for `mgished_key'. */
	DREF DeeObject *mgished_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_string_hash_with_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_hash__with__map_enumerate(DeeObject *self, char const *key, Dee_hash_t hash) {
	struct default_map_getitem_string_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem(DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}


/* map_operator_getitem_string_len_hash */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_len_hash))(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	err_map_unsupportedf(self, "operator [](%$q)", keylen, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	err_unknown_key_str_len(self, key, keylen);
	return NULL;
}

#ifndef DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
struct default_map_getitem_string_len_hash_with_enumerate_data {
	char const     *mgislhed_key;    /* [1..1] The key we're looking for. */
	size_t          mgislhed_keylen; /* Length of `mgislhed_key'. */
	Dee_hash_t      mgislhed_hash;   /* Hash for `mgislhed_key'. */
	DREF DeeObject *mgislhed_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash__with__map_enumerate(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if unlikely(status == -3) {
		err_unbound_key_str(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str(self, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash__with__map_operator_getitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}


/* map_operator_trygetitem_string_len_hash */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_len_hash))(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return ITER_DONE;
}

#ifndef DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
struct default_map_getitem_string_len_hash_with_enumerate_data {
	char const     *mgislhed_key;    /* [1..1] The key we're looking for. */
	size_t          mgislhed_keylen; /* Length of `mgislhed_key'. */
	Dee_hash_t      mgislhed_hash;   /* Hash for `mgislhed_key'. */
	DREF DeeObject *mgislhed_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_len_hash__with__map_enumerate(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}


/* map_operator_bounditem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem(DeeObject *self, DeeObject *key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem))(self, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem__unsupported(DeeObject *self, DeeObject *key) {
	default__map_operator_getitem__unsupported(self, key);
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem__empty(DeeObject *self, DeeObject *key) {
	return Dee_BOUND_MISSING;
}

#ifndef DEFINED_default_map_bounditem_with_enumerate_cb
#define DEFINED_default_map_bounditem_with_enumerate_cb
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_bounditem_with_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem__with__map_enumerate(DeeObject *self, DeeObject *key) {
	Dee_ssize_t status;
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_map_bounditem_with_enumerate_cb, key);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = Dee_BOUND_YES;
	} else if (status == -3) {
		status = Dee_BOUND_NO;
	} else if (status == 0) {
		status = Dee_BOUND_MISSING;
	} else {
#if Dee_BOUND_ERR != -1
		status = Dee_BOUND_ERR;
#endif /* Dee_BOUND_ERR != -1 */
	}
	return (int)status;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem__with__map_operator_contains(DeeObject *self, DeeObject *key) {
	int result_status;
	DREF DeeObject *result;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_contains))(self, key);
	if unlikely(!result)
		goto err;
	result_status = DeeObject_BoolInherited(result);
	return Dee_BOUND_FROMHAS_BOUND(result_status);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem__with__map_operator_getitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *value;
	value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, key);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}


/* map_operator_bounditem_index */
INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_index(DeeObject *self, size_t key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem_index))(self, key);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_index__unsupported(DeeObject *self, size_t key) {
	default__map_operator_getitem_index__unsupported(self, key);
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_index__empty(DeeObject *self, size_t key) {
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_index__with__map_operator_bounditem(DeeObject *self, size_t key) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_index__with__map_operator_getitem_index(DeeObject *self, size_t key) {
	DREF DeeObject *value;
	value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_index))(self, key);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}


/* map_operator_bounditem_string_hash */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem_string_hash))(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash) {
	default__map_operator_getitem_string_hash__unsupported(self, key, hash);
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash) {
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem_string_hash__with__map_operator_bounditem(DeeObject *self, char const *key, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *value;
	value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_hash))(self, key, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}


/* map_operator_bounditem_string_len_hash */
INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem_string_len_hash))(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	default__map_operator_getitem_string_len_hash__unsupported(self, key, keylen, hash);
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_string_len_hash__with__map_operator_bounditem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *value;
	value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_len_hash))(self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}


/* map_operator_hasitem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_hasitem(DeeObject *self, DeeObject *key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_hasitem))(self, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_hasitem__with__map_operator_bounditem(DeeObject *self, DeeObject *key) {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem))(self, key);
	return Dee_BOUND_ASHAS(result);
}


/* map_operator_hasitem_index */
INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_hasitem_index(DeeObject *self, size_t key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_hasitem_index))(self, key);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_hasitem_index__with__map_operator_bounditem_index(DeeObject *self, size_t key) {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem_index))(self, key);
	return Dee_BOUND_ASHAS(result);
}


/* map_operator_hasitem_string_hash */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_hasitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_hasitem_string_hash))(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem_string_hash))(self, key, hash);
	return Dee_BOUND_ASHAS(result);
}


/* map_operator_hasitem_string_len_hash */
INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_hasitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_hasitem_string_len_hash))(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem_string_len_hash))(self, key, keylen, hash);
	return Dee_BOUND_ASHAS(result);
}


/* map_operator_delitem */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem(DeeObject *self, DeeObject *key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with_callattr___map_delitem__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_delitem__, 1, &key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with_callobjectcache___map_delitem__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___map_delitem__.c_object, self, 1, &key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with_callmethodcache___map_delitem__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_delitem__.c_method, self, 1, &key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with_callkwmethodcache___map_delitem__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_delitem__.c_kwmethod, self, 1, &key, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__unsupported(DeeObject *self, DeeObject *key) {
	return err_map_unsupportedf(self, "operator del[](%r)", key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_len_hash(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_index))(self, key_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_hash(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_index))(self, key_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with__map_operator_delitem_string_len_hash(DeeObject *self, DeeObject *key) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with__map_operator_delitem_string_hash(DeeObject *self, DeeObject *key) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with__map_operator_delitem_index(DeeObject *self, DeeObject *key) {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_index))(self, key_value);
err:
	return -1;
}


/* map_operator_delitem_index */
INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_delitem_index(DeeObject *self, size_t key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_index))(self, key);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_delitem_index__unsupported(DeeObject *self, size_t key) {
	return err_map_unsupportedf(self, "operator del[](%" PRFuSIZ ")", key);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_delitem_index__with__map_operator_delitem(DeeObject *self, size_t key) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


/* map_operator_delitem_string_hash */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_string_hash))(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash) {
	return err_map_unsupportedf(self, "operator del[](%q)", key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem_string_hash__with__map_operator_delitem(DeeObject *self, char const *key, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


/* map_operator_delitem_string_len_hash */
INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_delitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem_string_len_hash))(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_delitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return err_map_unsupportedf(self, "operator del[](%$q)", keylen, key);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_delitem_string_len_hash__with__map_operator_delitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


/* map_operator_setitem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, key, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with_callattr___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_setitem__, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with_callobjectcache___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___map_setitem__.c_object, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with_callmethodcache___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_setitem__.c_method, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with_callkwmethodcache___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_setitem__.c_kwmethod, self, 2, args, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__unsupported(DeeObject *self, DeeObject *key, DeeObject *value) {
	return err_map_unsupportedf(self, "operator []=(%r, %r)", key, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__empty(DeeObject *self, DeeObject *key, DeeObject *value) {
	(void)value;
	return err_unknown_key(self, key);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_len_hash(DeeObject *self, DeeObject *key, DeeObject *value) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key), value);
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_index))(self, key_value, value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_hash(DeeObject *self, DeeObject *key, DeeObject *value) {
	size_t key_value;
	if (DeeString_Check(key)) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key), value);
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_index))(self, key_value, value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with__map_operator_setitem_string_len_hash(DeeObject *self, DeeObject *key, DeeObject *value) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_len_hash))(self, DeeString_STR(key), DeeString_SIZE(key), DeeString_Hash(key), value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with__map_operator_setitem_string_hash(DeeObject *self, DeeObject *key, DeeObject *value) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_hash))(self, DeeString_STR(key), DeeString_Hash(key), value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__with__map_operator_setitem_index(DeeObject *self, DeeObject *key, DeeObject *value) {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_index))(self, key_value, value);
err:
	return -1;
}


/* map_operator_setitem_index */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__map_operator_setitem_index(DeeObject *self, size_t key, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_index))(self, key, value);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__map_operator_setitem_index__unsupported(DeeObject *self, size_t key, DeeObject *value) {
	return err_map_unsupportedf(self, "operator []=(%" PRFuSIZ ", %r)", key, value);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__map_operator_setitem_index__empty(DeeObject *self, size_t key, DeeObject *value) {
	(void)value;
	return err_unknown_key_int(self, key);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__map_operator_setitem_index__with__map_operator_setitem(DeeObject *self, size_t key, DeeObject *value) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


/* map_operator_setitem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__map_operator_setitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_hash))(self, key, hash, value);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__map_operator_setitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	return err_map_unsupportedf(self, "operator []=(%q)", key);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__map_operator_setitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	(void)hash;
	(void)value;
	return err_unknown_key_str(self, key);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__map_operator_setitem_string_hash__with__map_operator_setitem(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


/* map_operator_setitem_string_len_hash */
INTERN WUNUSED NONNULL((1, 5)) int DCALL
default__map_operator_setitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_len_hash))(self, key, keylen, hash, value);
}

INTERN WUNUSED NONNULL((1, 5)) int DCALL
default__map_operator_setitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return err_map_unsupportedf(self, "operator []=(%$q)", keylen, key);
}

INTERN WUNUSED NONNULL((1, 5)) int DCALL
default__map_operator_setitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	(void)hash;
	(void)value;
	return err_unknown_key_str_len(self, key, keylen);
}

INTERN WUNUSED NONNULL((1, 5)) int DCALL
default__map_operator_setitem_string_len_hash__with__map_operator_setitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


/* map_operator_contains */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains(DeeObject *self, DeeObject *key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_contains))(self, key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__with_callattr___map_contains__(DeeObject *self, DeeObject *key) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_contains__, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__with_callobjectcache___map_contains__(DeeObject *self, DeeObject *key) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___map_contains__.c_object, self, 1, &key);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__with_callmethodcache___map_contains__(DeeObject *self, DeeObject *key) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_contains__.c_method, self, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__with_callkwmethodcache___map_contains__(DeeObject *self, DeeObject *key) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_contains__.c_kwmethod, self, 1, &key, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__unsupported(DeeObject *self, DeeObject *key) {
	err_map_unsupportedf(self, "operator contains(%r)", key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__empty(DeeObject *self, DeeObject *key) {
	return_false;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__with__map_operator_trygetitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if (value == ITER_DONE)
		return_false;
	if (value) {
		Dee_Decref(value);
		return_true;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__with__map_operator_bounditem(DeeObject *self, DeeObject *key) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem))(self, key);
	if unlikely(Dee_BOUND_ISERR(result))
		goto err;
	return_bool_(Dee_BOUND_ISBOUND(result));
err:
	return NULL;
}


/* map_iterkeys */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with_callattr___map_iterkeys__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_iterkeys__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with_callobjectcache___map_iterkeys__(DeeObject *self) {
	return DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___map_iterkeys__.c_object, self, 0, NULL);
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with_callmethodcache___map_iterkeys__(DeeObject *self) {
	return DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_iterkeys__.c_method, self, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with_callkwmethodcache___map_iterkeys__(DeeObject *self) {
	return DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_iterkeys__.c_kwmethod, self, 0, NULL, NULL);
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__unsupported(DeeObject *self) {
	err_map_unsupportedf(self, "__map_iterkeys__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with__map_enumerate(DeeObject *self) {
	/* TODO: Custom iterator type that uses "tp_enumerate" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with__set_operator_iter(DeeObject *self) {
	/* NOTE: This only works when the mapping can't have unbound keys! */
	DeeTypeObject *itertyp;
	DREF DefaultIterator_PairSubItem *result;
	result = DeeObject_MALLOC(DefaultIterator_PairSubItem);
	if unlikely(!result)
		goto err;
	result->dipsi_iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_iter))(self);
	if unlikely(!result->dipsi_iter)
		goto err_r;
	itertyp = Dee_TYPE(result->dipsi_iter);
	if unlikely((!itertyp->tp_iterator ||
	             !itertyp->tp_iterator->tp_nextkey) &&
	            !DeeType_InheritIterNext(itertyp))
		goto err_r_iter_no_next;
	ASSERT(itertyp->tp_iterator);
	ASSERT(itertyp->tp_iterator->tp_nextkey);
	result->dipsi_next = itertyp->tp_iterator->tp_nextkey;
	DeeObject_Init(result, &DefaultIterator_WithNextKey);
	return (DREF DeeObject *)result;
err_r_iter_no_next:
	Dee_Decref(result->dipsi_iter);
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


/* map_enumerate */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, cb, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__with_callattr___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_enumerate__, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__with_callobjectcache___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__.c_object, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__with_callmethodcache___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__.c_method, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__with_callkwmethodcache___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__.c_kwmethod, self, 1, (DeeObject *const *)&wrapper, NULL);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	return err_map_unsupportedf(self, "__map_enumerate__(<cb>)");
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *key;
	DREF DeeObject *iterkeys;
	DeeMH_map_operator_trygetitem_t cached_map_operator_trygetitem = DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem);
	iterkeys = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_iterkeys))(self);
	if unlikely(!iterkeys)
		goto err;
	while (ITER_ISOK(key = DeeObject_IterNext(iterkeys))) {
		DREF DeeObject *value;
		value = (*cached_map_operator_trygetitem)(self, key);
		if unlikely(!value)
			goto err_iterkeys_key;
		if (value != ITER_DONE) {
			temp = (*cb)(arg, key, value);
			Dee_Decref(value);
		} else {
			temp = (*cb)(arg, key, NULL);
		}
		Dee_Decref(key);
		if unlikely(temp < 0)
			goto err_iterkeys_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_iterkeys;
	}
	Dee_Decref(iterkeys);
	if unlikely(!key)
		goto err;
	return result;
err_iterkeys_temp:
	Dee_Decref(iterkeys);
	return temp;
err_iterkeys_key:
	Dee_Decref(key);
err_iterkeys:
	Dee_Decref(iterkeys);
err:
	return -1;
}


/* map_enumerate_range */
INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__with_callattr___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	DeeObject *args[3];
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	args[0] = (DeeObject *)wrapper;
	args[1] = start;
	args[2] = end;
	result  = DeeObject_CallAttr(self, (DeeObject *)&str___map_enumerate__, 3, args);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__with_callobjectcache___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	DeeObject *args[3];
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	args[0] = (DeeObject *)wrapper;
	args[1] = start;
	args[2] = end;
	result  = DeeObject_ThisCall(Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__.c_object, self, 3, args);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__with_callmethodcache___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	DeeObject *args[3];
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	args[0] = (DeeObject *)wrapper;
	args[1] = start;
	args[2] = end;
	result  = DeeObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__.c_method, self, 3, args);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__with_callkwmethodcache___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	DeeObject *args[3];
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	args[0] = (DeeObject *)wrapper;
	args[1] = start;
	args[2] = end;
	result  = DeeKwObjMethod_CallFunc(Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__.c_kwmethod, self, 3, args, NULL);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */

INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__unsupported(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) { return err_map_unsupportedf(self, "__map_enumerate__(<cb>, %r, %r)", start, end); }

#ifndef DEFINED_map_enumerate_with_filter_cb
#define DEFINED_map_enumerate_with_filter_cb
struct map_enumerate_with_filter_data {
	Dee_seq_enumerate_t mewfd_cb;           /* [1..1] Underlying callback. */
	void               *mewfd_arg;          /* Cookie for `mewfd_cb' */
	DeeObject          *mewfd_filter_start; /* [1..1] Filter start. */
	DeeObject          *mewfd_filter_end;   /* [1..1] Filter end. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
map_enumerate_with_filter_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	int temp;
	struct map_enumerate_with_filter_data *data;
	data = (struct map_enumerate_with_filter_data *)arg;
	/* if (!(mewfd_filter_start <= index))
	 *     return 0; */
	temp = DeeObject_CmpLeAsBool(data->mewfd_filter_start, index);
	if (temp <= 0)
		return temp;

	/* if (!(mewfd_filter_end > index))
	 *     return 0; */
	temp = DeeObject_CmpGrAsBool(data->mewfd_filter_end, index);
	if (temp <= 0)
		return temp;

	return (*data->mewfd_cb)(data->mewfd_arg, index, value);
}
#endif /* !DEFINED_map_enumerate_with_filter_cb */
INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__with__map_enumerate(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	struct map_enumerate_with_filter_data data;
	data.mewfd_cb           = cb;
	data.mewfd_arg          = arg;
	data.mewfd_filter_start = start;
	data.mewfd_filter_end   = end;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &map_enumerate_with_filter_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	/* TODO */
	(void)self;
	(void)cb;
	(void)arg;
	return DeeError_NOTIMPLEMENTED();
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_C */
