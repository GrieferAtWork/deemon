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
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/simple-hashset.h>

#include <hybrid/overflow.h>
#include <hybrid/typecore.h>

/**/
#include "../objects/int_logic.h" /* int_inc() */
#include "../objects/seq/cached-seq.h"
#include "../objects/seq/concat.h"
#include "../objects/seq/default-compare.h"
#include "../objects/seq/default-enumerate.h"
#include "../objects/seq/default-iterators.h"
#include "../objects/seq/default-map-proxy.h"
#include "../objects/seq/default-maps.h"
#include "../objects/seq/default-reversed.h"
#include "../objects/seq/default-sequences.h"
#include "../objects/seq/default-sets.h"
#include "../objects/seq/enumerate-cb.h"
#include "../objects/seq/removeif-cb.h"
#include "../objects/seq/repeat.h"
#include "../objects/seq/sort.h"
#include "../objects/seq/unique-iterator.h"
#include "method-hint-defaults.h"
#include "method-hints.h"
#include "runtime_error.h"
#include "strings.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#define do_fix_negative_range_index(index, size) \
	((size) - ((size_t)(-(index)) % (size)))

/* Mutable sequence functions */
PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_api_vunsupportedf(char const *api, DeeObject *self, char const *method_format, va_list args) {
	int result;
	DREF DeeObject *message, *error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(unicode_printer_printf(&printer, "type %k does not support: %s.",
	                                   DeeObject_Class(self), api) < 0)
		goto err_printer;
	if unlikely(unicode_printer_vprintf(&printer, method_format, args) < 0)
		goto err_printer;
	message = unicode_printer_pack(&printer);
	if unlikely(!message)
		goto err;
	error = DeeObject_New(&DeeError_NotImplemented, 1, &message);
	/*error = DeeObject_New(&DeeError_SequenceError, 1, &message);*/
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


LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mhcache_call(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
             size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *cb;
	cb = DeeClass_GetMember(tp_self, addr);
	if unlikely(!cb)
		return NULL;
	result = DeeObject_Call(cb, argc, argv);
	Dee_Decref_unlikely(cb);
	return result;
}


LOCAL WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
mhcache_thiscall(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
                 DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *cb;
	cb = DeeClass_GetMember(tp_self, addr);
	if unlikely(!cb)
		return NULL;
	result = DeeObject_ThisCall(cb, self, argc, argv);
	Dee_Decref_unlikely(cb);
	return result;
}

LOCAL WUNUSED NONNULL((1, 3, 4)) DREF DeeObject *
mhcache_thiscallf(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
                  DeeObject *self, char const *format, ...) {
	va_list args;
	DREF DeeObject *result, *cb;
	cb = DeeClass_GetMember(tp_self, addr);
	if unlikely(!cb)
		return NULL;
	va_start(args, format);
	result = DeeObject_VThisCallf(cb, self, format, args);
	va_end(args);
	Dee_Decref_unlikely(cb);
	return result;
}

LOCAL WUNUSED NONNULL((1)) int DCALL
mhcache_call_bound(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
                   size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = mhcache_call(tp_self, addr, argc, argv);
	if (result) {
		Dee_Decref(result);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundAttribute))
		return Dee_BOUND_NO;
	return Dee_BOUND_ERR;
}

LOCAL WUNUSED NONNULL((1)) int DCALL
mhcache_call_int(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
                 size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	result = mhcache_call(tp_self, addr, argc, argv);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

LOCAL WUNUSED NONNULL((1, 3)) int DCALL
mhcache_thiscall_int(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
                     DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, addr, self, argc, argv);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

#if 0
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mhcache_thiscall_result(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
                        size_t argc1, DeeObject *const *argv1,
                        size_t argc2, DeeObject *const *argv2) {
	DREF DeeObject *result1, *result2;
	result1 = mhcache_call(tp_self, addr, argc1, argv1);
	if unlikely(!result1)
		return NULL;
	result2 = DeeObject_Call(result1, argc2, argv2);
	Dee_Decref_unlikely(result1);
	return result2;
}

LOCAL WUNUSED NONNULL((1, 5)) DREF DeeObject *
mhcache_thiscall_resultf(DeeTypeObject *tp_self, Dee_mhc_slot_t addr,
                         size_t argc1, DeeObject *const *argv1,
                         char const *format, ...) {
	va_list args;
	DREF DeeObject *result1, *result2;
	result1 = mhcache_call(tp_self, addr, argc1, argv1);
	if unlikely(!result1)
		return NULL;
	va_start(format, args);
	result2 = DeeObject_VCallf(result1, format, args);
	va_end(args);
	Dee_Decref_unlikely(result1);
	return result2;
}

LOCAL WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
mhcache_instancemethod(DeeTypeObject *tp_self, Dee_mhc_slot_t addr, DeeObject *self) {
	DREF DeeObject *result, *cb;
	cb = DeeClass_GetMember(tp_self, addr);
	if unlikely(!cb)
		return NULL;
	result = DeeInstanceMethod_New(cb, self);
	Dee_Decref_unlikely(cb);
	return result;
}
#endif




#ifndef Dee_int_SIZE_MAX_DEFINED
#define Dee_int_SIZE_MAX_DEFINED
#if __SIZEOF_SIZE_T__ == 4
PRIVATE DEFINE_UINT32(Dee_int_SIZE_MAX, (uint32_t)-1);
#elif __SIZEOF_SIZE_T__ == 8
PRIVATE DEFINE_UINT64(Dee_int_SIZE_MAX, (uint64_t)-1);
#elif __SIZEOF_SIZE_T__ == 2
PRIVATE DEFINE_UINT16(Dee_int_SIZE_MAX, (uint16_t)-1);
#elif __SIZEOF_SIZE_T__ == 1
PRIVATE DEFINE_UINT8(Dee_int_SIZE_MAX, (uint8_t)-1);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_bool__with_callobjectcache___seq_bool__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_bool__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__unsupported(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "operator bool()");
}

#ifndef DEFINED_default_seq_bool_with_foreach_cb
#define DEFINED_default_seq_bool_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return -2;
}
#endif /* !DEFINED_default_seq_bool_with_foreach_cb */
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

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_bool__with__set_operator_compare_eq(DeeObject *__restrict self) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_compare_eq))(self, Dee_EmptySet);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}


/* seq_operator_sizeob */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with_callattr___seq_size__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_size__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with_callobjectcache___seq_size__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_sizeob__with_callobjectcache___seq_size__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_size__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "operator size()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__with__seq_operator_size(DeeObject *__restrict self) {
	size_t seqsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(seqsize);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_sizeob__empty(DeeObject *__restrict UNUSED(self)) { return_reference_(DeeInt_Zero); }


/* seq_operator_size */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__unsupported(DeeObject *__restrict self) {
	return (size_t)err_seq_unsupportedf(self, "operator size()");
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__seq_operator_sizeob(DeeObject *__restrict self) {
	DREF DeeObject *sizeob;
	sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
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
default__seq_operator_size__with__seq_operator_foreach(DeeObject *__restrict self) {
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
default__seq_operator_size__with__seq_operator_foreach_pair(DeeObject *__restrict self) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach_pair))(self, &default_seq_size_with_foreach_pair_cb, NULL);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__set_operator_sizeob(DeeObject *__restrict self) {
	DREF DeeObject *sizeob;
	sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__seq_operator_size__with__map_enumerate(DeeObject *__restrict self) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_iter__with_callobjectcache___seq_iter__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_iter__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "operator iter()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_iter__empty(DeeObject *__restrict UNUSED(self)) { return_empty_iterator; }

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
	DeeTypeObject *itertyp;
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_iterkeys))(self);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	Dee_Incref(self);
	result->diikgi_seq        = self;
	itertyp                   = Dee_TYPE(result->diikgi_iter);
	result->diikgi_tp_next    = DeeType_RequireNativeOperator(itertyp, iter_next);
	result->diikgi_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem);
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndTryGetItemMap_Type);
	return (DREF DeeObject *)result;
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
	DeeTypeObject *itertyp;
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_iterkeys))(self);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	Dee_Incref(self);
	result->diikgi_seq        = self;
	itertyp                   = Dee_TYPE(result->diikgi_iter);
	result->diikgi_tp_next    = DeeType_RequireNativeOperator(itertyp, iter_next);
	result->diikgi_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem);
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndGetItemMap_Type);
	return (DREF DeeObject *)result;
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

#ifndef DEFINED_default_foreach_with_foreach_pair_cb
#define DEFINED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_with_foreach_pair_data *data;
	Dee_ssize_t result;
	DREF DeeTupleObject *pair;
	data = (struct default_foreach_with_foreach_pair_data *)arg;
	pair = DeeTuple_NewUninitializedPair();
	if unlikely(!pair)
		goto err;
	pair->t_elem[0] = key;   /* Symbolic reference */
	pair->t_elem[1] = value; /* Symbolic reference */
	result = (*data->dfwfp_cb)(data->dfwfp_arg, (DeeObject *)pair);
	DeeTuple_DecrefSymbolic((DREF DeeObject *)pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_with_foreach_pair_cb */
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

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__map_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_foreach_pair))(self, &default_foreach_with_foreach_pair_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0;; ++i) {
		DREF DeeObject *elem;
		elem = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_operator_foreach__with__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	Dee_ssize_t temp, result = 0;
	DREF DeeIntObject *index = (DREF DeeIntObject *)DeeInt_Zero;
	Dee_Incref(DeeInt_Zero);
	for (;;) {
		DREF DeeObject *elem;
		elem = (*cached_seq_operator_getitem)(self, (DeeObject *)index);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err_index;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_index;
		if (int_inc(&index))
			goto err_index;
	}
	Dee_Decref(index);
	return result;
err_index:
	Dee_Decref(index);
/*err:*/
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

#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_getitem__with_callobjectcache___seq_getitem__(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_getitem__, self, 1, &index);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__unsupported(DeeObject *self, DeeObject *index) {
	err_seq_unsupportedf(self, "operator [](%r)", index);
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_getitem__empty(DeeObject *self, DeeObject *index) {
	err_index_out_of_bounds_ob(self, index);
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
default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t index) {
	size_t size;
	DREF DeeObject *result;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, index);
	if likely(ITER_ISOK(result))
		return result;
	if unlikely(!result)
		goto err;
	size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_bad_bounds;
	err_unbound_index(self, index);
	goto err;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, size);
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_trygetitem__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(index)) {
	return ITER_DONE;
}


/* seq_operator_trygetitem_index */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t index) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, index);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_trygetitem_index__empty(DeeObject *__restrict UNUSED(self), size_t UNUSED(index)) {
	return ITER_DONE;
}

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

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_operator_hasitem_index__with__seq_operator_size(DeeObject *__restrict self, size_t index) {
	size_t seqsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return index < seqsize ? 1 : 0;
err:
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_delitem__with_callobjectcache___seq_delitem__(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__, self, 1, &index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__unsupported(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "operator del[](%r)", index);
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

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_delitem__empty(DeeObject *self, DeeObject *index) {
	return err_index_out_of_bounds_ob(self, index);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_delitem_index__with_callobjectcache___seq_delitem__(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_delitem__, self, PCKuSIZ, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_setitem__with_callobjectcache___seq_setitem__(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__unsupported(DeeObject *self, DeeObject *index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []=(%r, %r)", index, value);
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

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__seq_operator_setitem__empty(DeeObject *self, DeeObject *index, DeeObject *UNUSED(value)) {
	return err_index_out_of_bounds_ob(self, index);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_setitem_index__with_callobjectcache___seq_setitem__(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_setitem__, self, PCKuSIZ "o", index, value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setitem_index__unsupported(DeeObject *self, size_t index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []=(%" PRFuSIZ ", %r)", index, value);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setitem_index__empty(DeeObject *self, size_t index, DeeObject *UNUSED(value)) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_getrange__with_callobjectcache___seq_getrange__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_getrange__, self, 2, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_seq_unsupportedf(self, "operator [:](%r, %r)", start, end);
	return NULL;
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
default__seq_operator_getrange__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(start), DeeObject *UNUSED(end)) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end) {
	int temp;
	DREF DefaultSequence_WithSizeObAndGetItem *result;
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
	result = DeeObject_MALLOC(DefaultSequence_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = startob_and_endob[0]; /* Inherit reference */
	result->dssg_end   = startob_and_endob[1]; /* Inherit reference */
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	DeeObject_Init(result, &DefaultSequence_WithSizeObAndGetItem_Type);
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
default__seq_operator_getrange_index__empty(DeeObject *UNUSED(self), Dee_ssize_t UNUSED(start), Dee_ssize_t UNUSED(end)) {
	return_empty_seq;
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
	result->dssgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
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
	result->dssgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithSizeObAndGetItem *result;
	struct Dee_seq_range range;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeObAndGetItem);
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
	result->dssg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	DeeObject_Init(result, &DefaultSequence_WithSizeObAndGetItem_Type);
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
	result->dsial_tp_iter = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter);
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
default__seq_operator_getrange_index_n__empty(DeeObject *UNUSED(self), Dee_ssize_t UNUSED(start)) {
	return_empty_seq;
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
	result->dssgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
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
	result->dssgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
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
	DREF DefaultSequence_WithSizeObAndGetItem *result;
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
	result = DeeObject_MALLOC(DefaultSequence_WithSizeObAndGetItem);
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
	result->dssg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	DeeObject_Init(result, &DefaultSequence_WithSizeObAndGetItem_Type);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_delrange__with_callobjectcache___seq_delrange__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_delrange__, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase(DeeObject *self, Dee_ssize_t start, Dee_ssize_t UNUSED(end)) {
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
default__seq_operator_delrange_index_n__with__seq_operator_size__and__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_setrange__with_callobjectcache___seq_setrange__(Dee_TYPE(self), self, start, end, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = items;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_setrange__, self, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	return err_seq_unsupportedf(self, "operator [:]=(%r, %r, %r)", start, end, items);
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

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__seq_operator_setrange__empty(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange__unsupported(self, start, end, items);
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
default__seq_operator_setrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items) {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange_index__unsupported(self, start, end, items);
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
default__seq_operator_setrange_index_n__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, DeeObject *items) {
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_operator_setrange_index_n__empty(DeeObject *self, Dee_ssize_t start, DeeObject *items) {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange_index_n__unsupported(self, start, items);
err:
	return -1;
}


/* seq_operator_assign */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign(DeeObject *self, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_assign))(self, items);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with_callattr___seq_assign__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_assign__, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with_callobjectcache___seq_assign__(DeeObject *self, DeeObject *items) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_assign__with_callobjectcache___seq_assign__(Dee_TYPE(self), self, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_assign__, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__unsupported(DeeObject *self, DeeObject *items) {
	return err_seq_unsupportedf(self, "operator :=(%r)", items);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__empty(DeeObject *self, DeeObject *items) {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_assign__unsupported(self, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_assign__with__seq_operator_setrange(DeeObject *self, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, Dee_None, Dee_None, items);
}


/* seq_operator_hash */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_hash))(self);
}

#ifndef DEFINED_seq_handle_hash_error
#define DEFINED_seq_handle_hash_error
PRIVATE NONNULL((1)) Dee_hash_t DCALL
seq_handle_hash_error(DeeObject *self) {
	DeeError_Print("Unhandled error in `Sequence.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}
#endif /* !DEFINED_seq_handle_hash_error */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with_callattr___seq_hash__(DeeObject *__restrict self) {
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
	return seq_handle_hash_error(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with_callobjectcache___seq_hash__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_hash__with_callobjectcache___seq_hash__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_hash__, 1, (DeeObject *const *)&self);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return seq_handle_hash_error(self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__unsupported(DeeObject *__restrict self) {
	return DeeObject_HashGeneric(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__empty(DeeObject *__restrict UNUSED(self)) {
	return DEE_HASHOF_EMPTY_SEQUENCE;
}

#ifndef DEFINED_default_seq_hash_with_foreach_cb
#define DEFINED_default_seq_hash_with_foreach_cb
struct default_seq_hash_with_foreach_data {
	Dee_hash_t sqhwf_result;   /* Hash result (or DEE_HASHOF_EMPTY_SEQUENCE when sqhwf_nonempty=false) */
	bool       sqhwf_nonempty; /* True after the first element */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_hash_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_seq_hash_with_foreach_data *data;
	Dee_hash_t elem_hash;
	data = (struct default_seq_hash_with_foreach_data *)arg;
	elem_hash = DeeObject_Hash(elem);
	if (data->sqhwf_nonempty) {
		data->sqhwf_result = Dee_HashCombine(data->sqhwf_result, elem_hash);
	} else {
		data->sqhwf_result = elem_hash;
		data->sqhwf_nonempty = true;
	}
	return 0;
}
#endif /* !DEFINED_default_seq_hash_with_foreach_cb */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_foreach(DeeObject *__restrict self) {
	struct default_seq_hash_with_foreach_data data;
	data.sqhwf_result   = DEE_HASHOF_EMPTY_SEQUENCE;
	data.sqhwf_nonempty = false;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_seq_hash_with_foreach_cb, &data))
		goto err;
	return data.sqhwf_result;
err:
	return seq_handle_hash_error(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self) {
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
	return seq_handle_hash_error(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self) {
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
	return seq_handle_hash_error(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self) {
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
	return seq_handle_hash_error(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__seq_operator_hash__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self) {
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
	return seq_handle_hash_error(self);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_compare__with_callobjectcache___seq_compare__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare__, lhs, 1, &rhs);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__unsupported(DeeObject *lhs, DeeObject *rhs) {
	return err_seq_unsupportedf(lhs, "__seq_compare__(%r)", rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_compare__empty(DeeObject *UNUSED(lhs), DeeObject *rhs) {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = DeeObject_SizeOb(rhs);
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
		DREF DeeObject *rhs_iter = DeeObject_Iter(rhs);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
	    DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                       rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                    rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                       rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                     rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                  rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                              rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_trygetitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                       rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                  rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                  rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                              rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_getitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	DREF DeeObject *lhs_sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_sizeob))(lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                              rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                           rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
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
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		ASSERT(data.scf_sg_osize == lhs_sizeob);
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, lhs_sizeob);
			if unlikely(temp < 0) {
				foreach_result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
			} else if (temp) {
				foreach_result = SEQ_COMPARE_FOREACH_RESULT_LESS;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_compare_eq__, lhs, 1, &rhs);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		result = (*cached_seq_operator_foreach)(lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = DeeObject_SizeOb(rhs);
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
		DREF DeeObject *rhs_iter = DeeObject_Iter(rhs);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {
	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
		return 1;
}
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                           rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
	    DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                         rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                      rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                  rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {
	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
		return 1;
}
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                       rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                    rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_trygetitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_size))(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {
	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
		return 1;
}
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                    rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                    rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_getitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
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
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	DREF DeeObject *lhs_sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_sizeob))(lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                                  rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, cached_seq_operator_getitem, rhs,
		                                                                             rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
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
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		ASSERT(data.scf_sg_osize == lhs_sizeob);
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, lhs_sizeob);
			if unlikely(temp < 0) {
				foreach_result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
			} else if (temp) {
				foreach_result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
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

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_operator_trycompare_eq__empty(DeeObject *UNUSED(lhs), DeeObject *rhs) {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? 0 : 1 /*or: -1*/;
err:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return -1;
	return Dee_COMPARE_ERR;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_eq__with_callobjectcache___seq_eq__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_eq__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator ==(%r)", rhs);
	return NULL;
}

#ifndef DEFINED_xinvoke_not
#define DEFINED_xinvoke_not
PRIVATE WUNUSED DREF DeeObject *DCALL
xinvoke_not(/*[0..1],inherit(always)*/ DREF DeeObject *ob) {
	if (ob) {
		int temp = DeeObject_BoolInherited(ob);
		if likely(temp >= 0) {
			ob = DeeBool_For(!temp);
			Dee_Incref(ob);
		} else {
			ob = NULL;
		}
	}
	return ob;
}
#endif /* !DEFINED_xinvoke_not */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_eq__with__seq_operator_ne(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ne))(lhs, rhs);
	return xinvoke_not(result);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_ne__with_callobjectcache___seq_ne__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ne__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator !=(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ne__with__seq_operator_eq(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_eq))(lhs, rhs);
	return xinvoke_not(result);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_lo__with_callobjectcache___seq_lo__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_lo__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator <(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_lo__with__seq_operator_ge(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_ge))(lhs, rhs);
	return xinvoke_not(result);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_le__with_callobjectcache___seq_le__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_le__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator <=(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_le__with__seq_operator_gr(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_gr))(lhs, rhs);
	return xinvoke_not(result);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_gr__with_callobjectcache___seq_gr__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_gr__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator >(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_gr__with__seq_operator_le(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_le))(lhs, rhs);
	return xinvoke_not(result);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_ge__with_callobjectcache___seq_ge__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___seq_ge__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_seq_unsupportedf(lhs, "operator >=(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_ge__with__seq_operator_lo(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), seq_operator_lo))(lhs, rhs);
	return xinvoke_not(result);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__(Dee_TYPE(*p_self), p_self, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_add__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__(Dee_TYPE(*p_self), p_self, repeat);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___seq_inplace_mul__, *p_self, 1, &repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
default__seq_enumerate__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___seq_enumerate__, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_enumerate__with_callobjectcache___seq_enumerate__(Dee_TYPE(self), self, cb, arg);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t UNUSED(cb), void *UNUSED(arg)) { return err_seq_unsupportedf(self, "__seq_enumerate__(...)"); }

#ifndef DEFINED_default_enumerate_with_enumerate_index_cb
#define DEFINED_default_enumerate_with_enumerate_index_cb
struct default_enumerate_with_enumerate_index_data {
	Dee_seq_enumerate_t dewei_cb;  /* [1..1] Wrapped callback. */
	void               *dewei_arg; /* [?..?] Cookie for `dewei_cb' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_enumerate_with_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	Dee_ssize_t result;
	DREF DeeObject *indexob;
	struct default_enumerate_with_enumerate_index_data *data;
	data = (struct default_enumerate_with_enumerate_index_data *)arg;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*data->dewei_cb)(data->dewei_arg, indexob, value);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_enumerate_with_enumerate_index_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__seq_enumerate_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	struct default_enumerate_with_enumerate_index_data data;
	data.dewei_cb  = cb;
	data.dewei_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &default_enumerate_with_enumerate_index_cb, &data, 0, (size_t)-1);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
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
		temp = (*cb)(arg, indexob, index_value);
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
default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_indexob;
			}
		}
		temp = (*cb)(arg, indexob, index_value);
		Dee_XDecref(index_value);
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
default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value;
	size_t i, size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = (*cached_seq_operator_trygetitem_index)(self, i);
		if unlikely(!index_value)
			goto err_indexob;
		if (index_value == ITER_DONE) {
			temp = (*cb)(arg, indexob, NULL);
		} else {
			temp = (*cb)(arg, indexob, index_value);
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
default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
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
		index_value = (*cached_seq_operator_getitem)(self, indexob);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_sizeob_indexob;
			}
		}
		temp = (*cb)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_sizeob_indexob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
		if unlikely(DeeObject_Inc(&indexob))
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

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value;
	size_t i;
	for (i = 0;; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_indexob;
			}
		}
		temp = (*cb)(arg, indexob, index_value);
		Dee_XDecref(index_value);
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
default__seq_enumerate__with__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	Dee_ssize_t temp, result = 0;
	DREF DeeIntObject *indexob;
	Dee_Incref(DeeInt_Zero);
	indexob = (DREF DeeIntObject *)DeeInt_Zero;
	for (;;) {
		DREF DeeObject *index_value;
		index_value = (*cached_seq_operator_getitem)(self, (DeeObject *)indexob);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_indexob;
			}
		}
		temp = (*cb)(arg, (DeeObject *)indexob, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_indexob;
		result += temp;
		if unlikely(DeeThread_CheckInterrupt())
			goto err_indexob;
		if unlikely(int_inc(&indexob))
			goto err_indexob;
	}
	Dee_Decref(indexob);
	return result;
err_temp_indexob:
	Dee_Decref(indexob);
	return temp;
err_indexob:
	Dee_Decref(indexob);
/*err:*/
	return -1;
}

#ifndef DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb
#define DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb
struct default_seq_enumerate_with_counter__and__seq_foreach_data {
	Dee_seq_enumerate_t dewcaf_cb;      /* [1..1] Wrapped callback */
	void               *dewcaf_arg;     /* [?..?] Cookie for `dewcaf_cb' */
	size_t              dewcaf_counter; /* Index of the next element that will be enumerated */
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
	result = (*data->dewcaf_cb)(data->dewcaf_arg, indexob, elem);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate__with__seq_operator_foreach__and__counter(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	struct default_seq_enumerate_with_counter__and__seq_foreach_data data;
	data.dewcaf_cb      = cb;
	data.dewcaf_arg     = arg;
	data.dewcaf_counter = 0;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_seq_enumerate_with_counter__and__seq_foreach_cb, &data);
}


/* seq_enumerate_index */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_enumerate__, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_enumerate_index__with_callobjectcache___seq_enumerate__(Dee_TYPE(self), self, cb, arg, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate__, self, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_index_t UNUSED(cb), void *UNUSED(arg), size_t start, size_t end) { return err_seq_unsupportedf(self, "__seq_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); }

#ifndef DEFINED_default_enumerate_index_with_enumerate_cb
#define DEFINED_default_enumerate_index_with_enumerate_cb
struct default_enumerate_index_with_enumerate_data {
	Dee_seq_enumerate_index_t deiwe_cb;    /* [1..1] Underlying callback. */
	void                     *deiwe_arg;   /* [?..?] Cookie for `deiwe_cb' */
	size_t                    deiwe_start; /* Enumeration start index */
	size_t                    deiwe_end;   /* Enumeration end index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_enumerate_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	size_t index;
	struct default_enumerate_index_with_enumerate_data *data;
	data = (struct default_enumerate_index_with_enumerate_data *)arg;
	if (DeeObject_AsSize(key, &index)) /* TODO: Handle overflow (by skipping this item) */
		goto err;
	if (index >= data->deiwe_start && index < data->deiwe_end)
		return (*data->deiwe_cb)(data->deiwe_arg, index, value);
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_enumerate_index_with_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with__seq_enumerate(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	struct default_enumerate_index_with_enumerate_data data;
	data.deiwe_cb    = cb;
	data.deiwe_arg   = arg;
	data.deiwe_start = start;
	data.deiwe_end   = end;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate))(self, &default_enumerate_index_with_enumerate_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
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
		temp = (*cb)(arg, i, index_value);
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
default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err;
			}
		}
		temp = (*cb)(arg, i, index_value);
		Dee_XDecref(index_value);
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
default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = (*cached_seq_operator_trygetitem_index)(self, i);
		if unlikely(!index_value)
			goto err;
		if (index_value == ITER_DONE) {
			temp = (*cb)(arg, i, NULL);
		} else {
			temp = (*cb)(arg, i, index_value);
			Dee_Decref(index_value);
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
default__seq_enumerate_index__with__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err;
			}
		}
		temp = (*cb)(arg, i, index_value);
		Dee_XDecref(index_value);
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

#ifndef DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb
#define DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb
#define default_seq_enumerate_index_with_counter__and__seq_foreach_cb_MAGIC_EARLY_STOP \
	(__SSIZE_MIN__ + 99) /* Shhht. We don't talk about this one... */

struct default_seq_enumerate_index_with_counter__and__seq_foreach_data {
	Dee_seq_enumerate_index_t deiwcaf_cb;    /* [1..1] Wrapped callback */
	void                     *deiwcaf_arg;   /* [?..?] Cookie for `deiwcaf_cb' */
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
	return (*data->deiwcaf_cb)(data->deiwcaf_arg, index, elem);
}
#endif /* !DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_enumerate_index__with__seq_operator_foreach__and__counter(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	struct default_seq_enumerate_index_with_counter__and__seq_foreach_data data;
	Dee_ssize_t result;
	data.deiwcaf_cb    = cb;
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
default__seq_makeenumeration__with_callattr___seq_enumerate_items__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_enumerate_items__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "__seq_enumerate_items__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__empty(DeeObject *__restrict UNUSED(self)) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_operator_getitem_index(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_getitem_index, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_operator_getitem(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_getitem, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_operator_iter__and__counter(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_iter__and__counter, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration__with__seq_enumerate(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_enumerate, self);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__, self, 2, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__unsupported(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_seq_unsupportedf(self, "__seq_enumerate_items__(%r, %r)", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__with__seq_makeenumeration_with_intrange(DeeObject *self, DeeObject *start, DeeObject *end) {
	size_t start_index, end_index;
	if (DeeObject_AsSize(start, &start_index))
		goto err;
	if (DeeObject_AsSize(end, &end_index))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_intrange))(self, start_index, end_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(start), DeeObject *UNUSED(end)) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem, self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_range__with__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__seq_operator_getitem, self, start, end);
}


/* seq_makeenumeration_with_intrange */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with_callattr___seq_enumerate_items__(DeeObject *__restrict self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_enumerate_items__, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with_callobjectcache___seq_enumerate_items__(DeeObject *__restrict self, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_makeenumeration_with_intrange__with_callobjectcache___seq_enumerate_items__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_enumerate_items__, self, PCKuSIZ PCKuSIZ, start, end);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_enumerate_items__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with__seq_makeenumeration_with_range(DeeObject *__restrict self, size_t start, size_t end) {
	DREF DeeObject *result, *startob, *endob;
	startob = DeeInt_NewSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_range))(self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__empty(DeeObject *__restrict UNUSED(self), size_t UNUSED(start), size_t UNUSED(end)) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t start, size_t end) {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast, self, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t start, size_t end) {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index, self, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t start, size_t end) {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index, self, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t start, size_t end) {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index, self, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with__seq_operator_iter__and__counter(DeeObject *__restrict self, size_t start, size_t end) {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter, self, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_makeenumeration_with_intrange__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end) {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_enumerate_index, self, start, end);
}


/* seq_foreach_reverse */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_foreach_reverse__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeObject *(DCALL *tp_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t temp, result = 0;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	Dee_ssize_t temp, result = 0;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*cached_seq_operator_trygetitem_index)(self, size);
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
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	Dee_ssize_t temp, result = 0;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*cached_seq_operator_getitem_index)(self, size);
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
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
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
		item = (*cached_seq_operator_getitem)(self, sizeob);
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
default__seq_enumerate_index_reverse__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DREF DeeObject *(DCALL *tp_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t temp, result = 0;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	Dee_ssize_t temp, result = 0;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*cached_seq_operator_trygetitem_index)(self, size);
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
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
	Dee_ssize_t temp, result = 0;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*cached_seq_operator_getitem_index)(self, size);
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
	DeeMH_seq_operator_getitem_t cached_seq_operator_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem);
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *startob = NULL;
	DREF DeeObject *sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
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
		item = (*cached_seq_operator_getitem)(self, sizeob);
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


/* seq_unpack */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with_callattr_unpack(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_unpack, PCKuSIZ, count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	if (DeeTuple_SIZE(resultob) != count) {
		err_invalid_unpack_size(resultob, count, DeeTuple_SIZE(resultob));
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return 0;
err_r:
	Dee_Decref_likely(resultob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with_callattr___seq_unpack__(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_unpack__, PCKuSIZ, count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	if (DeeTuple_SIZE(resultob) != count) {
		err_invalid_unpack_size(resultob, count, DeeTuple_SIZE(resultob));
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return 0;
err_r:
	Dee_Decref_likely(resultob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with_callobjectcache___seq_unpack__(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_unpack__with_callobjectcache___seq_unpack__(Dee_TYPE(self), self, count, result);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_unpack__, self, PCKuSIZ, count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	if (DeeTuple_SIZE(resultob) != count) {
		err_invalid_unpack_size(resultob, count, DeeTuple_SIZE(resultob));
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return 0;
err_r:
	Dee_Decref_likely(resultob);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__unsupported(DeeObject *__restrict self, size_t count, DREF DeeObject *UNUSED(result)[]) {
	return err_seq_unsupportedf(self, "__seq_unpack__(%" PRFuSIZ ")", count);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with__seq_unpack_ex(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	size_t real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_unpack_ex))(self, count, count, result);
	if likely(real_count != (size_t)-1)
		real_count = 0;
	return (int)(Dee_ssize_t)real_count;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__empty(DeeObject *__restrict self, size_t count, DREF DeeObject *UNUSED(result)[]) {
	if unlikely(count != 0)
		return err_invalid_unpack_size(self, count, 0);
	return 0;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with__tp_asvector(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	size_t real_count = (*Dee_TYPE(self)->tp_seq->tp_asvector)(self, count, result);
	if likely(real_count == count)
		return 0;
	err_invalid_unpack_size(self, count, real_count);
	if (real_count < count)
		Dee_Decrefv(result, real_count);
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	DeeNO_getitem_index_fast_t getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count != count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return err_invalid_unpack_size(self, count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*getitem_index_fast)(self, i);
		if unlikely(!item) {
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			Dee_Decrefv(result, i);
			return err_invalid_unpack_size(self, count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) { /* requires: __seq_getitem_always_bound__ */
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count != count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return err_invalid_unpack_size(self, count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*cached_seq_operator_trygetitem_index)(self, i);
		if unlikely(!ITER_ISOK(item)) {
			Dee_Decrefv(result, i);
			if unlikely(!item)
				goto err;
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			return err_invalid_unpack_size(self, count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) { /* requires: __seq_getitem_always_bound__ */
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count != count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return err_invalid_unpack_size(self, count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!item) {
			Dee_Decrefv(result, i);
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if (DeeError_Catch(&DeeError_IndexError))
				return err_invalid_unpack_size(self, count, i);
			goto err;
		}
		result[i] = item; /* Inherit reference */
	}
	return 0;
err:
	return -1;
}

#ifndef DEFINED_default_unpack_with_foreach_cb
#define DEFINED_default_unpack_with_foreach_cb
struct default_unpack_with_foreach_data {
	size_t           duqfd_count;  /* Remaining destination length */
	DREF DeeObject **duqfd_result; /* [?..?][0..duqfd_count] Pointer to next destination */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_unpack_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_unpack_with_foreach_data *data;
	data = (struct default_unpack_with_foreach_data *)arg;
	if likely(data->duqfd_count) {
		Dee_Incref(elem);
		*data->duqfd_result = elem;
		--data->duqfd_count;
		++data->duqfd_result;
	}
	return 1;
}
#endif /* !DEFINED_default_unpack_with_foreach_cb */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with__seq_operator_foreach(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	Dee_ssize_t foreach_status;
	struct default_unpack_with_foreach_data data;
	data.duqfd_count  = count;
	data.duqfd_result = result;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_unpack_with_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	ASSERT(((size_t)foreach_status == (size_t)(data.duqfd_result - result)) ||
	       ((size_t)foreach_status > count));
	if likely((size_t)foreach_status == count)
		return 0;
	Dee_Decrefv(result, (size_t)(data.duqfd_result - result));
	err_invalid_unpack_size(self, count, (size_t)foreach_status);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_unpack__with__seq_operator_iter(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	size_t i, remainder;
	DREF DeeObject *elem;
	DREF DeeObject *iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	for (i = 0; i < count; ++i) {
		elem = DeeObject_IterNext(iter);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem)
				err_invalid_unpack_size(self, count, i);
			goto err_iter_result_i;
		}
		result[i] = elem; /* Inherit reference. */
	}

	/* Check to make sure that the iterator actually ends here. */
	remainder = DeeObject_IterAdvance(iter, (size_t)-2);
	if unlikely(remainder != 0) {
		if unlikely(remainder == (size_t)-1)
			goto err_iter_result_i;
		if (OVERFLOW_UADD(remainder, count, &remainder))
			remainder = (size_t)-1;
		err_invalid_unpack_size(self, count, remainder);
		goto err_iter_result_i;
	}
	Dee_Decref(iter);
	return 0;
err_iter_result_i:
	Dee_Decrefv(result, i);
/*err_iter:*/
	Dee_Decref(iter);
err:
	return -1;
}


/* seq_unpack_ex */
INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with_callattr_unpack(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t result_count;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_unpack, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), result_count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return result_count;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with_callattr___seq_unpack__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t result_count;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_unpack__, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), result_count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return result_count;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with_callobjectcache___seq_unpack__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_unpack_ex__with_callobjectcache___seq_unpack__(Dee_TYPE(self), self, min_count, max_count, result);
#else /* __OPTIMIZE_SIZE__ */
	size_t result_count;
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_unpack__, self, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), result_count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return result_count;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__unsupported(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *UNUSED(result)[]) {
	return err_seq_unsupportedf(self, "__seq_unpack__(%" PRFuSIZ ", %" PRFuSIZ ")", min_count, max_count);
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__empty(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *UNUSED(result)[]) {
	if unlikely(min_count > 0)
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, 0);
	return 0;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with__tp_asvector(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t real_count = (*Dee_TYPE(self)->tp_seq->tp_asvector)(self, max_count, result);
	if unlikely(real_count < min_count || real_count > max_count) {
		if (real_count < min_count) 
			Dee_Decrefv(result, real_count);
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, real_count);
	}
	return real_count;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	DeeNO_getitem_index_fast_t getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*getitem_index_fast)(self, i);
		if unlikely(!item) {
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if (i >= min_count)
				return i;
			Dee_Decrefv(result, i);
			return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) { /* requires: __seq_getitem_always_bound__ */
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*cached_seq_operator_trygetitem_index)(self, i);
		if unlikely(!ITER_ISOK(item)) {
			Dee_Decrefv(result, i);
			if unlikely(!item)
				goto err;
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if (i >= min_count)
				return i;
			return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) { /* requires: __seq_getitem_always_bound__ */
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!item) {
			Dee_Decrefv(result, i);
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if (DeeError_Catch(&DeeError_IndexError)) {
				if (i >= min_count)
					return i;
				return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, i);
			}
			goto err;
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with__seq_operator_foreach(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	Dee_ssize_t foreach_status;
	struct default_unpack_with_foreach_data data;
	data.duqfd_count  = max_count;
	data.duqfd_result = result;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_unpack_with_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	ASSERT(((size_t)foreach_status == (size_t)(data.duqfd_result - result)) ||
	       ((size_t)foreach_status > max_count));
	if likely((size_t)foreach_status >= min_count && (size_t)foreach_status <= max_count)
		return (size_t)foreach_status;
	Dee_Decrefv(result, (size_t)(data.duqfd_result - result));
	err_invalid_unpack_size_minmax(self, min_count, max_count, (size_t)foreach_status);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ex__with__seq_operator_iter(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t i, remainder;
	DREF DeeObject *elem;
	DREF DeeObject *iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	for (i = 0; i < max_count; ++i) {
		elem = DeeObject_IterNext(iter);
		if (!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err_iter_result_i;
			if likely(i >= min_count) {
				Dee_Decref(iter);
				return i;
			}
			err_invalid_unpack_size_minmax(self, min_count, max_count, i);
			goto err_iter_result_i;
		}
		result[i] = elem; /* Inherit reference. */
	}

	/* Check to make sure that the iterator actually ends here. */
	remainder = DeeObject_IterAdvance(iter, (size_t)-2);
	if unlikely(remainder != 0) {
		if unlikely(remainder == (size_t)-1)
			goto err_iter_result_i;
		if (OVERFLOW_UADD(remainder, max_count, &remainder))
			remainder = (size_t)-1;
		err_invalid_unpack_size_minmax(self, min_count, max_count, remainder);
		goto err_iter_result_i;
	}
	Dee_Decref(iter);
	return max_count;
err_iter_result_i:
	Dee_Decrefv(result, i);
/*err_iter:*/
	Dee_Decref(iter);
err:
	return (size_t)-1;
}


/* seq_unpack_ub */
INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__with_callattr_unpackub(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t result_count;
	DREF DeeObject *resultob;
	resultob = min_count == max_count ? DeeObject_CallAttrf(self, (DeeObject *)&str_unpackub, PCKuSIZ, min_count)
	                                  : DeeObject_CallAttrf(self, (DeeObject *)&str_unpackub, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (!DeeObject_InstanceOfExact(resultob, &DeeTuple_Type) &&
	    !DeeObject_InstanceOfExact(resultob, &DeeNullableTuple_Type)) {
		DeeObject_TypeAssertFailed2(resultob, &DeeTuple_Type, &DeeNullableTuple_Type);
		goto err_r;
	}
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	/* XXX: DeeNullableTuple_DecrefSymbolic(resultob); */
	Dee_XMovrefv(result, DeeTuple_ELEM(resultob), result_count);
	Dee_Decref_likely(resultob);
	return result_count;
err_r:
	Dee_Decref_likely(resultob);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__with_callattr___seq_unpackub__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t result_count;
	DREF DeeObject *resultob;
	resultob = min_count == max_count ? DeeObject_CallAttrf(self, (DeeObject *)&str___seq_unpackub__, PCKuSIZ, min_count)
	                                  : DeeObject_CallAttrf(self, (DeeObject *)&str___seq_unpackub__, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (!DeeObject_InstanceOfExact(resultob, &DeeTuple_Type) &&
	    !DeeObject_InstanceOfExact(resultob, &DeeNullableTuple_Type)) {
		DeeObject_TypeAssertFailed2(resultob, &DeeTuple_Type, &DeeNullableTuple_Type);
		goto err_r;
	}
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	/* XXX: DeeNullableTuple_DecrefSymbolic(resultob); */
	Dee_XMovrefv(result, DeeTuple_ELEM(resultob), result_count);
	Dee_Decref_likely(resultob);
	return result_count;
err_r:
	Dee_Decref_likely(resultob);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__with_callobjectcache___seq_unpackub__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_unpack_ub__with_callobjectcache___seq_unpackub__(Dee_TYPE(self), self, min_count, max_count, result);
#else /* __OPTIMIZE_SIZE__ */
	size_t result_count;
	DREF DeeObject *resultob;
	resultob = min_count == max_count ? mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_unpackub__, self, PCKuSIZ, min_count)
	                                  : mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_unpackub__, self, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (!DeeObject_InstanceOfExact(resultob, &DeeTuple_Type) &&
	    !DeeObject_InstanceOfExact(resultob, &DeeNullableTuple_Type)) {
		DeeObject_TypeAssertFailed2(resultob, &DeeTuple_Type, &DeeNullableTuple_Type);
		goto err_r;
	}
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	/* XXX: DeeNullableTuple_DecrefSymbolic(resultob); */
	Dee_XMovrefv(result, DeeTuple_ELEM(resultob), result_count);
	Dee_Decref_likely(resultob);
	return result_count;
err_r:
	Dee_Decref_likely(resultob);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__unsupported(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *UNUSED(result)[]) {
	return err_seq_unsupportedf(self, "__seq_unpackub__(%" PRFuSIZ ", %" PRFuSIZ ")", min_count, max_count);
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__empty(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *UNUSED(result)[]) {
	if unlikely(min_count > 0)
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, 0);
	return 0;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	DeeNO_getitem_index_fast_t getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i)
		result[i] = (*getitem_index_fast)(self, i); /* Inherit reference */
	return real_count;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*cached_seq_operator_trygetitem_index)(self, i);
		if unlikely(!ITER_ISOK(item)) {
			if unlikely(!item)
				goto err_result_i;
			item = NULL; /* Unbound */
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err_result_i:
	Dee_XDecrefv(result, i);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	DeeMH_seq_operator_getitem_index_t cached_seq_operator_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);;
	size_t i, real_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*cached_seq_operator_getitem_index)(self, i);
		if unlikely(!item) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* It's just unbound (which is allowed) */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				/* Early sequence end (sequence may have been truncated) */
				if (i >= min_count)
					return i;
				err_invalid_unpack_size_minmax(self, min_count, max_count, i);
				goto err_result_i;
			} else {
				goto err_result_i;
			}
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err_result_i:
	Dee_XDecrefv(result, i);
err:
	return (size_t)-1;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_trygetfirst__with_callobjectcache___seq_first__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__, 1, (DeeObject *const *)&self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "first()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__empty(DeeObject *__restrict UNUSED(self)) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_default_getfirst_with_foreach_cb, &result);
	if likely(foreach_status == -2)
		return result;
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}


/* seq_getfirst */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getfirst(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_getfirst))(self);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_getfirst__with_callobjectcache___seq_first__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
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
default__seq_boundfirst(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_boundfirst))(self);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_boundfirst__with_callobjectcache___seq_first__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call_bound(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_first__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundfirst__empty(DeeObject *__restrict UNUSED(self)) {
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
default__seq_delfirst(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_delfirst))(self);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_delfirst__with_callobjectcache___seq_first__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call_int(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_del___seq_first__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
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
default__seq_setfirst(DeeObject *self, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_setfirst))(self, value);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_setfirst__with_callobjectcache___seq_first__(Dee_TYPE(self), self, value);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall_int(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_set___seq_first__, self, 1, (DeeObject *const *)&value);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__unsupported(DeeObject *self, DeeObject *value) { return err_seq_unsupportedf(self, "first = %r", value); }

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setfirst__empty(DeeObject *self, DeeObject *UNUSED(value)) { return err_empty_sequence(self); }

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_trygetlast__with_callobjectcache___seq_last__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_last__, 1, (DeeObject *const *)&self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "last()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__empty(DeeObject *__restrict UNUSED(self)) {
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_default_getlast_with_foreach_cb, &result);
	if likely(foreach_status > 0)
		return result;
	Dee_Decref_unlikely(result);
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}


/* seq_getlast */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_getlast(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_getlast))(self);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_getlast__with_callobjectcache___seq_last__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_last__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
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
default__seq_boundlast(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_boundlast))(self);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_boundlast__with_callobjectcache___seq_last__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call_bound(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_last__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_boundlast__empty(DeeObject *__restrict UNUSED(self)) {
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
default__seq_dellast(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_dellast))(self);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_dellast__with_callobjectcache___seq_last__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call_int(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_del___seq_last__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
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
default__seq_setlast(DeeObject *self, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_setlast))(self, value);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_setlast__with_callobjectcache___seq_last__(Dee_TYPE(self), self, value);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall_int(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_set___seq_last__, self, 1, (DeeObject *const *)&value);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__unsupported(DeeObject *self, DeeObject *value) { return err_seq_unsupportedf(self, "last = %r", value); }

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_setlast__empty(DeeObject *self, DeeObject *UNUSED(value)) { return err_empty_sequence(self); }

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


/* seq_cached */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_cached))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached__with_callattr_cached(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_cached);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached__with_callattr___seq_cached__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___seq_cached__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached__with_callobjectcache___seq_cached__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_cached__with_callobjectcache___seq_cached__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_cached__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached__with__seq_operator_iter(DeeObject *__restrict self) {
	DREF DeeObject *iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	return (DREF DeeObject *)CachedSeq_WithIter_New(iter);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self) {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self) {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_cached__with__seq_operator_getitem(DeeObject *__restrict self) {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}


/* seq_frozen */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_frozen(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_frozen))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_frozen__with_callattr_frozen(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_frozen);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_frozen__with_callattr___seq_frozen__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___seq_frozen__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_frozen__with_callobjectcache___seq_frozen__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_frozen__with_callobjectcache___seq_frozen__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___seq_frozen__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_frozen__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "__seq_frozen__()");
	return NULL;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_any__with_callobjectcache___seq_any__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_any__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_any_with_key__with_callobjectcache___seq_any__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_any__, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_any_with_range__with_callobjectcache___seq_any__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_any__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_any_with_range_and_key__with_callobjectcache___seq_any__(Dee_TYPE(self), self, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_any__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_all__with_callobjectcache___seq_all__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_all__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_all_with_key__with_callobjectcache___seq_all__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_all__, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_all_with_range__with_callobjectcache___seq_all__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_all__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_all_with_range_and_key__with_callobjectcache___seq_all__(Dee_TYPE(self), self, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_all__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_parity__with_callobjectcache___seq_parity__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_parity_with_key__with_callobjectcache___seq_parity__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_parity_with_range__with_callobjectcache___seq_parity__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__(Dee_TYPE(self), self, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_parity__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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


/* seq_reduce */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce__with_callattr_reduce(DeeObject *self, DeeObject *combine) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_reduce, 1, &combine);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_reduce__, 1, &combine);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_reduce__with_callobjectcache___seq_reduce__(Dee_TYPE(self), self, combine);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_reduce__, self, 1, &combine);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce__unsupported(DeeObject *self, DeeObject *combine) {
	err_seq_unsupportedf(self, "__seq_reduce__(%r)", combine);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce__empty(DeeObject *self, DeeObject *combine) {
	(void)combine;
	err_empty_sequence(self);
	return NULL;
}

#ifndef DEFINED_seq_reduce_data
#define DEFINED_seq_reduce_data
struct seq_reduce_data {
	DeeObject      *gsr_combine; /* [1..1] Combinatory predicate (invoke as `gsr_combine(gsr_init, item)') */
	DREF DeeObject *gsr_result;  /* [0..1] Current reduction result, or NULL if no init given and at first item. */
};
#endif /* !DEFINED_seq_reduce_data */
#ifndef DEFINED_seq_reduce_foreach_cb
#define DEFINED_seq_reduce_foreach_cb
#ifndef DEFINED_seq_reduce_foreach_with_init_cb
#define DEFINED_seq_reduce_foreach_with_init_cb
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
#endif /* !DEFINED_seq_reduce_foreach_with_init_cb */
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
#endif /* !DEFINED_seq_reduce_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce__with__seq_operator_foreach(DeeObject *self, DeeObject *combine) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_reduce_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		err_empty_sequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}


/* seq_reduce_with_init */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_reduce_with_init__with_callattr_reduce(DeeObject *self, DeeObject *combine, DeeObject *init) {
	DeeObject *args[4];
	args[0] = combine;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = init;
	return DeeObject_CallAttr(self, (DeeObject *)&str_reduce, 4, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_reduce_with_init__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine, DeeObject *init) {
	DeeObject *args[4];
	args[0] = combine;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = init;
	return DeeObject_CallAttr(self, (DeeObject *)&str___seq_reduce__, 4, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_reduce_with_init__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine, DeeObject *init) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_reduce_with_init__with_callobjectcache___seq_reduce__(Dee_TYPE(self), self, combine, init);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[4];
	args[0] = combine;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = init;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_reduce__, self, 4, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_reduce_with_init__unsupported(DeeObject *self, DeeObject *combine, DeeObject *init) {
	err_seq_unsupportedf(self, "__seq_reduce__(%r, %r)", combine, init);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_reduce_with_init__empty(DeeObject *self, DeeObject *combine, DeeObject *init) {
	(void)self;
	(void)combine;
	return_reference_(init);
}

#ifndef DEFINED_seq_reduce_foreach_with_init_cb
#define DEFINED_seq_reduce_foreach_with_init_cb
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
#endif /* !DEFINED_seq_reduce_foreach_with_init_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_reduce_with_init__with__seq_operator_foreach(DeeObject *self, DeeObject *combine, DeeObject *init) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_reduce_foreach_with_init_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_Decref(data.gsr_result);
	return NULL;
}


/* seq_reduce_with_range */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce_with_range__with_callattr_reduce(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_reduce, "o" PCKuSIZ PCKuSIZ, combine, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce_with_range__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_reduce__, "o" PCKuSIZ PCKuSIZ, combine, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce_with_range__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_reduce_with_range__with_callobjectcache___seq_reduce__(Dee_TYPE(self), self, combine, start, end);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_reduce__, self, "o" PCKuSIZ PCKuSIZ, combine, start, end);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce_with_range__unsupported(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_reduce__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", combine, start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce_with_range__empty(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	(void)combine;
	(void)start;
	(void)end;
	err_empty_sequence(self);
	return NULL;
}

#ifndef DEFINED_seq_reduce_enumerate_cb
#define DEFINED_seq_reduce_enumerate_cb
#ifndef DEFINED_seq_reduce_foreach_cb
#define DEFINED_seq_reduce_foreach_cb
#ifndef DEFINED_seq_reduce_foreach_with_init_cb
#define DEFINED_seq_reduce_foreach_with_init_cb
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
#endif /* !DEFINED_seq_reduce_foreach_with_init_cb */
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
#endif /* !DEFINED_seq_reduce_foreach_cb */
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_reduce_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return seq_reduce_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_reduce_enumerate_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_reduce_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_reduce_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		err_empty_sequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}


/* seq_reduce_with_range_and_init */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_reduce_with_range_and_init__with_callattr_reduce(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_reduce, "o" PCKuSIZ PCKuSIZ "o", combine, start, end, init);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_reduce_with_range_and_init__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_reduce__, "o" PCKuSIZ PCKuSIZ "o", combine, start, end, init);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_reduce_with_range_and_init__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_reduce_with_range_and_init__with_callobjectcache___seq_reduce__(Dee_TYPE(self), self, combine, start, end, init);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_reduce__, self, "o" PCKuSIZ PCKuSIZ "o", combine, start, end, init);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_reduce_with_range_and_init__unsupported(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	err_seq_unsupportedf(self, "__seq_reduce__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", combine, start, end, init);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_reduce_with_range_and_init__empty(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	(void)self;
	(void)combine;
	(void)start;
	(void)end;
	return_reference_(init);
}

#ifndef DEFINED_seq_reduce_enumerate_with_init_cb
#define DEFINED_seq_reduce_enumerate_with_init_cb
#ifndef DEFINED_seq_reduce_foreach_with_init_cb
#define DEFINED_seq_reduce_foreach_with_init_cb
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
#endif /* !DEFINED_seq_reduce_foreach_with_init_cb */
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_reduce_enumerate_with_init_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return seq_reduce_foreach_with_init_cb(arg, item);
}
#endif /* !DEFINED_seq_reduce_enumerate_with_init_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_reduce_with_range_and_init__with__seq_enumerate_index(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_reduce_enumerate_with_init_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_Decref(data.gsr_result);
	return NULL;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_min__with_callobjectcache___seq_min__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_min__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_min_with_key__with_callobjectcache___seq_min__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_min__, self, 3, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_min_with_range__with_callobjectcache___seq_min__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_min__, self, PCKuSIZ PCKuSIZ, start, end);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_min_with_range_and_key__with_callobjectcache___seq_min__(Dee_TYPE(self), self, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_min__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_max__with_callobjectcache___seq_max__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_max__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_max_with_key__with_callobjectcache___seq_max__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_max__, self, 3, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_max_with_range__with_callobjectcache___seq_max__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_max__, self, PCKuSIZ PCKuSIZ, start, end);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_max_with_range_and_key__with_callobjectcache___seq_max__(Dee_TYPE(self), self, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_max__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_sum__with_callobjectcache___seq_sum__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__unsupported(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "__seq_sum__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum__empty(DeeObject *__restrict UNUSED(self)) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_sum_with_range__with_callobjectcache___seq_sum__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_sum__, self, PCKuSIZ PCKuSIZ, start, end);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_sum__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sum_with_range__empty(DeeObject *__restrict UNUSED(self), size_t UNUSED(start), size_t UNUSED(end)) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_count__with_callobjectcache___seq_count__(Dee_TYPE(self), self, item);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_count__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_count_with_key__with_callobjectcache___seq_count__(Dee_TYPE(self), self, item, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_count__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_count_with_range__with_callobjectcache___seq_count__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_count__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
	DeeMH_seq_find_t cached_seq_find = DeeType_RequireMethodHint(Dee_TYPE(self), seq_find);
	size_t result = 0;
	while (start < end) {
		size_t match = (*cached_seq_find)(self, item, start, end);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_count_with_range_and_key__with_callobjectcache___seq_count__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_count__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
	DeeMH_seq_find_with_key_t cached_seq_find_with_key = DeeType_RequireMethodHint(Dee_TYPE(self), seq_find_with_key);
	size_t result = 0;
	while (start < end) {
		size_t match = (*cached_seq_find_with_key)(self, item, start, end, key);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_contains__with_callobjectcache___seq_contains__(Dee_TYPE(self), self, item);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
	if unlikely(!ITER_ISOK(real_value)) {
		Dee_Decref(key_and_value[1]);
		if (real_value == ITER_DONE)
			return 0;
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
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_contains_with_foreach_cb(void *arg, DeeObject *elem) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	if (temp == 0)
		return -2;
	return 0;
}
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_contains_with_key__with_callobjectcache___seq_contains__(Dee_TYPE(self), self, item, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_contains_with_range__with_callobjectcache___seq_contains__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_contains_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_contains__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

#ifndef DEFINED_default_contains_with_foreach_cb
#define DEFINED_default_contains_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_contains_with_foreach_cb(void *arg, DeeObject *elem) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	if (temp == 0)
		return -2;
	return 0;
}
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_contains__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
default__seq_operator_contains__with__seq_contains(DeeObject *self, DeeObject *item) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains))(self, item);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__seq_operator_contains__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(item)) {
	return_false;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_locate__with_callobjectcache___seq_locate__(Dee_TYPE(self), self, match, def);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__, self, 2, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_locate__(%r, %r)", match, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_locate__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(match), DeeObject *def) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_locate_with_range__with_callobjectcache___seq_locate__(Dee_TYPE(self), self, match, start, end, def);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_locate__, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_locate__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", match, start, end, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_locate_with_range__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(match), size_t UNUSED(start), size_t UNUSED(end), DeeObject *def) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_rlocate__with_callobjectcache___seq_rlocate__(Dee_TYPE(self), self, match, def);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__, self, 2, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_rlocate__(%r, %r)", match, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__seq_rlocate__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(match), DeeObject *def) {
	return_reference_(def);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__(Dee_TYPE(self), self, match, start, end, def);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_rlocate__, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	err_seq_unsupportedf(self, "__seq_rlocate__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", match, start, end, def);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
default__seq_rlocate_with_range__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(match), size_t UNUSED(start), size_t UNUSED(end), DeeObject *def) {
	return_reference_(def);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_startswith__with_callobjectcache___seq_startswith__(Dee_TYPE(self), self, item);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_startswith_with_key__with_callobjectcache___seq_startswith__(Dee_TYPE(self), self, item, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_startswith_with_range__with_callobjectcache___seq_startswith__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_startswith__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_endswith__with_callobjectcache___seq_endswith__(Dee_TYPE(self), self, item);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_endswith_with_key__with_callobjectcache___seq_endswith__(Dee_TYPE(self), self, item, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_endswith_with_range__with_callobjectcache___seq_endswith__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_endswith__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_find__with_callobjectcache___seq_find__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_find__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_find_with_key__with_callobjectcache___seq_find__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_find__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_rfind__with_callobjectcache___seq_rfind__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_rfind__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return (size_t)err_seq_unsupportedf(self, "__seq_rfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_rfind_with_key__with_callobjectcache___seq_rfind__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_rfind__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_rfind_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return (size_t)err_seq_unsupportedf(self, "__seq_rfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_erase__with_callobjectcache___seq_erase__(Dee_TYPE(self), self, index, count);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_erase__, self, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
	DeeMH_seq_pop_t cached_seq_pop = DeeType_RequireMethodHint(Dee_TYPE(self), seq_pop);
	size_t end_index;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	while (end_index > index) {
		--end_index;
		if unlikely((*cached_seq_pop)(self, (Dee_ssize_t)end_index))
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_insert__with_callobjectcache___seq_insert__(Dee_TYPE(self), self, index, item);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_insert__, self, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_insertall__with_callobjectcache___seq_insertall__(Dee_TYPE(self), self, index, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_insertall__, self, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
	data.dsiawfid_insert = DeeType_RequireMethodHint(Dee_TYPE(self), seq_insert);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_pushfront__with_callobjectcache___seq_pushfront__(Dee_TYPE(self), self, item);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_pushfront__, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_append__with_callobjectcache___seq_append__(Dee_TYPE(self), self, item);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_append__, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_extend__with_callobjectcache___seq_extend__(Dee_TYPE(self), self, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_extend__, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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
	data.dsewfad_append = DeeType_RequireMethodHint(Dee_TYPE(self), seq_append);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_xchitem_index__with_callobjectcache___seq_xchitem__(Dee_TYPE(self), self, index, item);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_xchitem__, self, PCKuSIZ "o", index, item);
#endif /* !__OPTIMIZE_SIZE__ */
}

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
default__seq_clear__with_callattr_clear(DeeObject *__restrict self) {
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
default__seq_clear__with_callattr___seq_clear__(DeeObject *__restrict self) {
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
default__seq_clear__with_callobjectcache___seq_clear__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_clear__with_callobjectcache___seq_clear__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_clear__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__unsupported(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "__seq_clear__()");
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_operator_delrange(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange))(self, DeeInt_Zero, Dee_None);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_operator_delrange_index_n(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index_n))(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_clear__with__seq_erase(DeeObject *__restrict self) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_pop__with_callobjectcache___seq_pop__(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_pop__, self, PCKdSIZ, index);
#endif /* !__OPTIMIZE_SIZE__ */
}

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


/* seq_remove */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_remove__with_callattr_remove(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_remove, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_remove__with_callattr___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_remove__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_remove__with_callobjectcache___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_remove__with_callobjectcache___seq_remove__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_remove__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_remove__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_remove__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_remove__with__seq_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall))(self, item, start, end, 1);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}

#ifndef DEFINED_default_remove_with_enumerate_index_and_delitem_index_cb
#define DEFINED_default_remove_with_enumerate_index_and_delitem_index_cb
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
#endif /* !DEFINED_default_remove_with_enumerate_index_and_delitem_index_cb */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_remove__with__seq_enumerate_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
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
default__seq_remove__with__seq_find__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t index = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find))(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, index))
		goto err;
	return 1;
err:
	return -1;
}


/* seq_remove_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_remove_with_key__with_callattr_remove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_remove, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_remove_with_key__with_callattr___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_remove__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_remove_with_key__with_callobjectcache___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_remove_with_key__with_callobjectcache___seq_remove__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_remove__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_remove_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_remove__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_remove_with_key__with__seq_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall_with_key))(self, item, start, end, 1, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}

#ifndef DEFINED_default_remove_with_key_with_enumerate_index_and_delitem_index_cb
#define DEFINED_default_remove_with_key_with_enumerate_index_and_delitem_index_cb
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
#endif /* !DEFINED_default_remove_with_key_with_enumerate_index_and_delitem_index_cb */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_remove_with_key__with__seq_enumerate_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &default_remove_with_key_with_enumerate_index_and_delitem_index_cb, &data, start, end);
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
default__seq_remove_with_key__with__seq_find_with_key__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t index = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find_with_key))(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, index))
		goto err;
	return 1;
err:
	return -1;
}


/* seq_rremove */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_rremove__with_callattr_rremove(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_rremove, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_rremove__with_callattr___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_rremove__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_rremove__with_callobjectcache___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_rremove__with_callobjectcache___seq_rremove__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_rremove__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_rremove__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_rremove__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__seq_rremove__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index_reverse))(self, &default_remove_with_enumerate_index_and_delitem_index_cb, &data, start, end);
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
default__seq_rremove__with__seq_rfind__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t index = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rfind))(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, index))
		goto err;
	return 1;
err:
	return -1;
}


/* seq_rremove_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_rremove_with_key__with_callattr_rremove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_rremove, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_rremove_with_key__with_callattr___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_rremove__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_rremove_with_key__with_callobjectcache___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_rremove_with_key__with_callobjectcache___seq_rremove__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_rremove__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_rremove_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_rremove__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_rremove_with_key__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index_reverse))(self, &default_remove_with_key_with_enumerate_index_and_delitem_index_cb, &data, start, end);
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
default__seq_rremove_with_key__with__seq_rfind_with_key__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t index = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rfind_with_key))(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, index))
		goto err;
	return 1;
err:
	return -1;
}


/* seq_removeall */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeall__with_callattr_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_removeall, "o" PCKuSIZ PCKuSIZ PCKuSIZ, item, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeall__with_callattr___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_removeall__, "o" PCKuSIZ PCKuSIZ PCKuSIZ, item, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeall__with_callobjectcache___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_removeall__with_callobjectcache___seq_removeall__(Dee_TYPE(self), self, item, start, end, max);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_removeall__, self, "o" PCKuSIZ PCKuSIZ PCKuSIZ, item, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeall__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
	return (size_t)err_seq_unsupportedf(self, "__seq_removeall__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end, max);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeall__with__seq_removeif(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
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
default__seq_removeall__with__seq_operator_size__and__seq_remove(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
	DeeMH_seq_remove_t cached_seq_remove = DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove);
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(!max)
		goto done;
	selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	for (;;) {
		int temp = (*cached_seq_remove)(self, item, start, end);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (result >= max)
			break;
		if (sequence_size_changes_after_delitem == -1) {
			size_t new_selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
			if unlikely(new_selfsize == (size_t)-1)
				goto err;
			sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
		}
		if (sequence_size_changes_after_delitem) {
			--end;
			if (start >= end)
				break;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
done:
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeall__with__seq_remove__once(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) { /* When "self" is a set */
	return max ? (size_t)(Dee_ssize_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove))(self, item, start, end) : 0;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeall__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	DeeMH_seq_operator_delitem_index_t cached_seq_operator_delitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index);
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(start >= end)
		return 0;
	selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while (start < end) {
		DREF DeeObject *elem;
		elem = (*cached_seq_operator_trygetitem_index)(self, start);
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
				if unlikely((*cached_seq_operator_delitem_index)(self, start))
					goto err;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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


/* seq_removeall_with_key */
INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__with_callattr_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_removeall, "o" PCKuSIZ PCKuSIZ PCKuSIZ "o", item, start, end, max, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__with_callattr___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_removeall__, "o" PCKuSIZ PCKuSIZ PCKuSIZ "o", item, start, end, max, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__with_callobjectcache___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_removeall_with_key__with_callobjectcache___seq_removeall__(Dee_TYPE(self), self, item, start, end, max, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_removeall__, self, "o" PCKuSIZ PCKuSIZ PCKuSIZ "o", item, start, end, max, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	return (size_t)err_seq_unsupportedf(self, "__seq_removeall__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, max, key);
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(item), size_t UNUSED(start), size_t UNUSED(end), size_t UNUSED(max), DeeObject *UNUSED(key)) {
	return 0;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__with__seq_removeif(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	/* >> local keyedElem = key(item);
	 * >> return !!self.removeallif(x -> deemon.equals(keyedElem, key(x)), start, end, max); */
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
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeif))(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err_pred:
	DeeObject_FREE(pred);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__with__seq_operator_size__and__seq_remove_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	DeeMH_seq_remove_with_key_t cached_seq_remove_with_key = DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove_with_key);
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(!max)
		goto done;
	selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	for (;;) {
		int temp = (*cached_seq_remove_with_key)(self, item, start, end, key);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (result >= max)
			break;
		if (sequence_size_changes_after_delitem == -1) {
			size_t new_selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
			if unlikely(new_selfsize == (size_t)-1)
				goto err;
			sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
		}
		if (sequence_size_changes_after_delitem) {
			--end;
			if (start >= end)
				break;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
done:
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__with__seq_remove_with_key__once(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) { /* When "self" is a set */
	return max ? (size_t)(Dee_ssize_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove_with_key))(self, item, start, end, key) : 0;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
default__seq_removeall_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	DeeMH_seq_operator_delitem_index_t cached_seq_operator_delitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index);
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(start >= end)
		return 0;
	selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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
		elem = (*cached_seq_operator_trygetitem_index)(self, start);
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
				if unlikely((*cached_seq_operator_delitem_index)(self, start))
					goto err_item;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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


/* seq_removeif */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeif__with_callattr_removeif(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_removeif, "o" PCKuSIZ PCKuSIZ PCKuSIZ, should, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeif__with_callattr___seq_removeif__(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_removeif__, "o" PCKuSIZ PCKuSIZ PCKuSIZ, should, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeif__with_callobjectcache___seq_removeif__(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_removeif__with_callobjectcache___seq_removeif__(Dee_TYPE(self), self, should, start, end, max);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_removeif__, self, "o" PCKuSIZ PCKuSIZ PCKuSIZ, should, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeif__unsupported(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
	return (size_t)err_seq_unsupportedf(self, "__seq_removeif__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")", should, start, end, max);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeif__with__seq_removeall_with_key(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
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
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall_with_key))(self, &SeqRemoveIfWithRemoveAllItem_DummyInstance, start, end, max, (DeeObject *)key);
	Dee_Decref_likely(key);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_removeif__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(start >= end)
		return 0;
	selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while (start < end) {
		DREF DeeObject *elem;
		elem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index))(self, start);
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
				if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, start))
					goto err;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
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


/* seq_resize */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__with_callattr_resize(DeeObject *self, size_t newsize, DeeObject *filler) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_resize, PCKuSIZ "o", newsize, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__with_callattr___seq_resize__(DeeObject *self, size_t newsize, DeeObject *filler) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_resize__, PCKuSIZ "o", newsize, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__with_callobjectcache___seq_resize__(DeeObject *self, size_t newsize, DeeObject *filler) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_resize__with_callobjectcache___seq_resize__(Dee_TYPE(self), self, newsize, filler);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_resize__, self, PCKuSIZ "o", newsize, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__unsupported(DeeObject *self, size_t newsize, DeeObject *filler) {
	return err_seq_unsupportedf(self, "__seq_resize__(%" PRFuSIZ ", %r)", newsize, filler);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__empty(DeeObject *self, size_t newsize, DeeObject *filler) {
	return likely(newsize == 0) ? 0 : default__seq_resize__unsupported(self, newsize, filler);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index__and__seq_operator_delrange_index(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange_index))(self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize, Dee_EmptySeq);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__seq_resize__with__seq_operator_size__and__seq_erase__and__seq_extend(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_extend))(self, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, newsize, oldsize - newsize);
	}
	return 0;
err:
	return -1;
}


/* seq_fill */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_fill__with_callattr_fill(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_fill, PCKuSIZ PCKuSIZ "o", start, end, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_fill__with_callattr___seq_fill__(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_fill__, PCKuSIZ PCKuSIZ "o", start, end, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_fill__with_callobjectcache___seq_fill__(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_fill__with_callobjectcache___seq_fill__(Dee_TYPE(self), self, start, end, filler);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_fill__, self, PCKuSIZ PCKuSIZ "o", start, end, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_fill__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	return err_seq_unsupportedf(self, "__seq_fill__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, filler);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_fill__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	int result;
	DREF DeeObject *repeat;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	repeat = DeeSeq_RepeatItem(filler, end - start);
	if unlikely(!repeat)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, (Dee_ssize_t)start, (Dee_ssize_t)end, repeat);
	Dee_Decref(repeat);
	return result;
err:
	return -1;
}

#ifndef DEFINED_default_fill_with_enumerate_index_and_setitem_index_cb
#define DEFINED_default_fill_with_enumerate_index_and_setitem_index_cb
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
#endif /* !DEFINED_default_fill_with_enumerate_index_and_setitem_index_cb */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_fill__with__seq_enumerate_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	struct default_fill_with_enumerate_index_and_setitem_index_data data;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	data.dfweiasiid_seq    = self;
	data.dfweiasiid_filler = filler;
	data.dfweiasiid_setitem_index = seq->tp_setitem_index;
	return (int)(*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &default_fill_with_enumerate_index_and_setitem_index_cb, &data, start, end);
}


/* seq_reverse */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_reverse__with_callattr_reverse(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_reverse, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_reverse__with_callattr___seq_reverse__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_reverse__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_reverse__with_callobjectcache___seq_reverse__(DeeObject *self, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_reverse__with_callobjectcache___seq_reverse__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_reverse__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_reverse__unsupported(DeeObject *self, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_reverse__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_reverse__with__seq_reversed__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *reversed = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reversed))(self, start, end);
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, start, end, reversed);
	Dee_Decref(reversed);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index__and__seq_operator_delitem_index(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *lo_elem, *hi_elem;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, start);
		if unlikely(!lo_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		hi_elem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, end - 1);
		if unlikely(!hi_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err_lo_elem;
		if (hi_elem) {
			if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, start, hi_elem))
				goto err_lo_elem_hi_elem;
			Dee_Decref(hi_elem);
		} else {
			if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, start))
				goto err_lo_elem;
		}
		if (lo_elem) {
			if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, end - 1, lo_elem))
				goto err_lo_elem;
			Dee_Decref(lo_elem);
		} else {
			if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem_index))(self, end - 1))
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
default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *lo_elem, *hi_elem;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, start);
		if unlikely(!lo_elem)
			goto err;
		hi_elem = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index))(self, end - 1);
		if unlikely(!hi_elem)
			goto err_lo_elem;
		if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, start, hi_elem))
			goto err_lo_elem_hi_elem;
		Dee_Decref(hi_elem);
		if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem_index))(self, end - 1, lo_elem))
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


/* seq_reversed */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_reversed__with_callattr_reversed(DeeObject *self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_reversed, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_reversed__with_callattr___seq_reversed__(DeeObject *self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_reversed__, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_reversed__with_callobjectcache___seq_reversed__(DeeObject *self, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_reversed__with_callobjectcache___seq_reversed__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_reversed__, self, PCKuSIZ PCKuSIZ, start, end);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_reversed__unsupported(DeeObject *self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_reversed__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_reversed__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
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
default__seq_reversed__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem_index);
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
default__seq_reversed__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	Dee_Incref(self);
	result->drwgii_seq  = self;
	result->drwgii_max  = end - 1; /* It's ok if this underflows */
	result->drwgii_size = end - start;
	DeeObject_Init(result, &DefaultReversed_WithTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

#ifndef DEFINED_DeeSeq_GetForeachSubRangeAsTuple
#define DEFINED_DeeSeq_GetForeachSubRangeAsTuple
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
	fast_size = DeeObject_SizeFast(self);
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
	foreach_status = DeeObject_InvokeMethodHint(seq_operator_foreach, self, &foreach_subrange_as_tuple_cb, &data);
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
#endif /* !DEFINED_DeeSeq_GetForeachSubRangeAsTuple */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_reversed__with__seq_operator_foreach(DeeObject *self, size_t start, size_t end) {
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


/* seq_sort */
INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_sort__with_callattr_sort(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_sort, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_sort__with_callattr___seq_sort__(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_sort__, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_sort__with_callobjectcache___seq_sort__(DeeObject *self, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_sort__with_callobjectcache___seq_sort__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_sort__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_sort__unsupported(DeeObject *self, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "__seq_sort__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_sort__with__seq_sorted__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *sorted = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted))(self, start, end);
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, start, end, sorted);
	Dee_Decref(sorted);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__seq_sort__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}


/* seq_sort_with_key */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_sort_with_key__with_callattr_sort(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str_sort, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_sort_with_key__with_callattr___seq_sort__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_sort__, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_sort_with_key__with_callobjectcache___seq_sort__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_sort_with_key__with_callobjectcache___seq_sort__(Dee_TYPE(self), self, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_sort__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_sort_with_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "__seq_sort__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_sort_with_key__with__seq_sorted_with_key__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *sorted = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted_with_key))(self, start, end, key);
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange_index))(self, start, end, sorted);
	Dee_Decref(sorted);
	return result;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__seq_sort_with_key__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}


/* seq_sorted */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sorted__with_callattr_sorted(DeeObject *self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_sorted, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sorted__with_callattr___seq_sorted__(DeeObject *self, size_t start, size_t end) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_sorted__, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sorted__with_callobjectcache___seq_sorted__(DeeObject *self, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_sorted__with_callobjectcache___seq_sorted__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_sorted__, self, PCKuSIZ PCKuSIZ, start, end);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sorted__unsupported(DeeObject *self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_sorted__(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sorted__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, size_t start, size_t end) {
	DREF DeeTupleObject *result;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFast(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self,
	                                        start, Dee_TYPE(self)->tp_seq->tp_getitem_index_fast))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sorted__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, size_t start, size_t end) {
	DREF DeeTupleObject *result;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndex(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self, start,
	                                       DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index)))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_sorted__with__seq_operator_foreach(DeeObject *self, size_t start, size_t end) {
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


/* seq_sorted_with_key */
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_sorted_with_key__with_callattr_sorted(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str_sorted, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_sorted_with_key__with_callattr___seq_sorted__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return DeeObject_CallAttrf(self, (DeeObject *)&str___seq_sorted__, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_sorted_with_key__with_callobjectcache___seq_sorted__(DeeObject *self, size_t start, size_t end, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_sorted_with_key__with_callobjectcache___seq_sorted__(Dee_TYPE(self), self, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_sorted__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_sorted_with_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "__seq_sorted__(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_sorted_with_key__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeTupleObject *result;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFastWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self,
	                                               start, Dee_TYPE(self)->tp_seq->tp_getitem_index_fast, key))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_sorted_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeTupleObject *result;
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndexWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self, start,
	                                              DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index), key))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
default__seq_sorted_with_key__with__seq_operator_foreach(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeTupleObject *base, *result;
	base = (DREF DeeTupleObject *)DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if unlikely(!base)
		goto err;
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(base));
	if unlikely(!result)
		goto err_base;
	if unlikely(DeeSeq_SortVectorWithKey(DeeTuple_SIZE(result),
	                                     DeeTuple_ELEM(result),
	                                     DeeTuple_ELEM(base), key))
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


/* seq_bfind */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bfind__with_callattr_bfind(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_bfind, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bfind__with_callattr___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_bfind__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bfind__with_callobjectcache___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_bfind__with_callobjectcache___seq_bfind__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_bfind__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bfind__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_bfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bfind__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(item), size_t UNUSED(start), size_t UNUSED(end)) {
	return (size_t)-1;
}

#ifndef DEFINED_overflowsafe_mid
#define DEFINED_overflowsafe_mid
LOCAL ATTR_CONST size_t overflowsafe_mid(size_t a, size_t b) {
	size_t result;
	if unlikely(OVERFLOW_UADD(a, b, &result)) {
		size_t a_div2 = a >> 1;
		size_t b_div2 = b >> 1;
		result = (a_div2 + b_div2);
		if ((a & 1) && (b & 1))
			++result;
		return result;
	}
	return result >> 1;
}
#endif /* !DEFINED_overflowsafe_mid */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bfind__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_Compare(item, seq_item);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR)
					goto err_item_overflow;
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	return (size_t)-1;
err_item_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}


/* seq_bfind_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bfind_with_key__with_callattr_bfind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_bfind, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bfind_with_key__with_callattr___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_bfind__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bfind_with_key__with_callobjectcache___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_bfind_with_key__with_callobjectcache___seq_bfind__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_bfind__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bfind_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *UNUSED(key)) {
	err_seq_unsupportedf(self, "__seq_bfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bfind_with_key__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(item), size_t UNUSED(start), size_t UNUSED(end), DeeObject *UNUSED(key)) {
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bfind_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		item = DeeObject_Call(key, 1, &item);
		if unlikely(!item)
			goto err;
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err_item;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_CompareKey(item, seq_item, key);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err_item;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR)
					goto err_item_overflow;
				Dee_Decref(item);
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	Dee_Decref(item);
	return (size_t)-1;
err_item_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}


/* seq_bposition */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bposition__with_callattr_bposition(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_bposition, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bposition__with_callattr___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_bposition__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bposition__with_callobjectcache___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_bposition__with_callobjectcache___seq_bposition__(Dee_TYPE(self), self, item, start, end);
#else /* __OPTIMIZE_SIZE__ */
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_bposition__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bposition__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	err_seq_unsupportedf(self, "__seq_bposition__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
default__seq_bposition__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_Compare(item, seq_item);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR)
					goto err_item_overflow;
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	if unlikely(end == (size_t)-1 || end == (size_t)Dee_COMPARE_ERR)
		goto err_item_overflow;
	return end;
err_item_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}


/* seq_bposition_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bposition_with_key__with_callattr_bposition(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_bposition, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bposition_with_key__with_callattr___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_bposition__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bposition_with_key__with_callobjectcache___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_bposition_with_key__with_callobjectcache___seq_bposition__(Dee_TYPE(self), self, item, start, end, key);
#else /* __OPTIMIZE_SIZE__ */
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_bposition__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bposition_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *UNUSED(key)) {
	err_seq_unsupportedf(self, "__seq_bposition__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
default__seq_bposition_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		item = DeeObject_Call(key, 1, &item);
		if unlikely(!item)
			goto err;
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err_item;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_CompareKey(item, seq_item, key);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err_item;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR)
					goto err_item_overflow;
				Dee_Decref(item);
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	Dee_Decref(item);
	if unlikely(end == (size_t)-1 || end == (size_t)Dee_COMPARE_ERR)
		goto err_item_overflow;
	return end;
err_item_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}


/* seq_brange */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_brange__with_callattr_brange(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]) {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_brange, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_brange__with_callattr___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]) {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_brange__, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_brange__with_callobjectcache___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_brange__with_callobjectcache___seq_brange__(Dee_TYPE(self), self, item, start, end, result_range);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_brange__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_brange__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t UNUSED(result_range)[2]) {
	return err_seq_unsupportedf(self, "__seq_brange__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_brange__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(item), size_t UNUSED(start), size_t UNUSED(end), size_t result_range[2]) {
	result_range[0] = 0;
	result_range[1] = 0;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__seq_brange__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_Compare(item, seq_item);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				size_t result_range_start = mid;
				size_t result_range_end   = mid + 1;

				/* Widen the result range's lower bound */
				while (result_range_start > start) {
					mid = overflowsafe_mid(start, result_range_start);
					seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareEq(item, seq_item);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_start = mid;
					} else {
						/* No longer part of returned range! */
						start = mid + 1;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Widen the result range's upper bound */
				while (result_range_end < end) {
					mid = overflowsafe_mid(result_range_end, end);
					seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareEq(item, seq_item);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_end = mid + 1;
					} else {
						/* No longer part of returned range! */
						end = mid;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Write-back the result range bounds */
				result_range[0] = result_range_start;
				result_range[1] = result_range_end;
				return 0;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	result_range[0] = start;
	result_range[1] = end;
	return 0;
err:
	return -1;
}


/* seq_brange_with_key */
INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
default__seq_brange_with_key__with_callattr_brange(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]) {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str_brange, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
default__seq_brange_with_key__with_callattr___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]) {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = DeeObject_CallAttrf(self, (DeeObject *)&str___seq_brange__, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
default__seq_brange_with_key__with_callobjectcache___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__seq_brange_with_key__with_callobjectcache___seq_brange__(Dee_TYPE(self), self, item, start, end, key, result_range);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = mhcache_thiscallf(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___seq_brange__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
default__seq_brange_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *UNUSED(key), size_t UNUSED(result_range)[2]) {
	return err_seq_unsupportedf(self, "__seq_brange__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
default__seq_brange_with_key__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(item), size_t UNUSED(start), size_t UNUSED(end), DeeObject *UNUSED(key), size_t result_range[2]) {
	result_range[0] = 0;
	result_range[1] = 0;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
default__seq_brange_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]) {
	DeeMH_seq_operator_trygetitem_index_t cached_seq_operator_trygetitem_index = DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_trygetitem_index);
	size_t selfsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		item = DeeObject_Call(key, 1, &item);
		if unlikely(!item)
			goto err;
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err_item;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_CompareKey(item, seq_item, key);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err_item;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				size_t result_range_start = mid;
				size_t result_range_end   = mid + 1;

				/* Widen the result range's lower bound */
				while (result_range_start > start) {
					mid = overflowsafe_mid(start, result_range_start);
					seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err_item;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err_item;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_start = mid;
					} else {
						/* No longer part of returned range! */
						start = mid + 1;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Widen the result range's upper bound */
				while (result_range_end < end) {
					mid = overflowsafe_mid(result_range_end, end);
					seq_item = (*cached_seq_operator_trygetitem_index)(self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err_item;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err_item;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_end = mid + 1;
					} else {
						/* No longer part of returned range! */
						end = mid;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Write-back the result range bounds */
				result_range[0] = result_range_start;
				result_range[1] = result_range_end;
				Dee_Decref(item);
				return 0;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	Dee_Decref(item);
	result_range[0] = start;
	result_range[1] = end;
	return 0;
err_item:
	Dee_Decref(item);
err:
	return -1;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_iter__with_callobjectcache___set_iter__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_iter__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__unsupported(DeeObject *__restrict self) {
	err_set_unsupportedf(self, "operator iter()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_iter__with__seq_operator_iter(DeeObject *__restrict self) {
	DREF DeeObject *iter;
	DeeTypeObject *itertyp;
	DREF DistinctIterator *result;
	iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	result = DeeGCObject_MALLOC(DistinctIterator);
	if unlikely(!result)
		goto err_iter;
	itertyp            = Dee_TYPE(iter);
	result->di_tp_next = DeeType_RequireNativeOperator(itertyp, iter_next);
	result->di_iter    = iter; /* Inherit reference */
	Dee_simple_hashset_with_lock_init(&result->di_encountered);
	DeeObject_Init(result, &DistinctIterator_Type);
	return DeeGC_Track((DREF DeeObject *)result);
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

#ifndef DEFINED_default_set_foreach_unique_cb
#define DEFINED_default_set_foreach_unique_cb
struct default_set_foreach_unique_data {
	struct Dee_simple_hashset dsfud_encountered; /* Set of objects already encountered. */
	Dee_foreach_t             dsfud_cb;          /* [1..1] user-defined callback */
	void                     *dsfud_arg;         /* [?..?] Cookie for `dsfud_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_foreach_unique_cb(void *arg, DeeObject *item) {
	int insert_status;
	struct default_set_foreach_unique_data *data;
	data = (struct default_set_foreach_unique_data *)arg;
	insert_status = Dee_simple_hashset_insert(&data->dsfud_encountered, item);
	if likely(insert_status > 0)
		return (*data->dsfud_cb)(data->dsfud_arg, item);
	return insert_status; /* error, or already-exists */
}
#endif /* !DEFINED_default_set_foreach_unique_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__set_operator_foreach__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	struct default_set_foreach_unique_data data;
	data.dsfud_cb  = cb;
	data.dsfud_arg = arg;
	Dee_simple_hashset_init(&data.dsfud_encountered);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &default_set_foreach_unique_cb, &data);
	Dee_simple_hashset_fini(&data.dsfud_encountered);
	return result;
}


/* set_operator_sizeob */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with_callattr___set_size__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___set_size__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with_callobjectcache___set_size__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_sizeob__with_callobjectcache___set_size__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_size__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__unsupported(DeeObject *__restrict self) {
	err_set_unsupportedf(self, "operator size()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_sizeob__with__set_operator_size(DeeObject *__restrict self) {
	size_t setsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_size))(self);
	if unlikely(setsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(setsize);
err:
	return NULL;
}


/* set_operator_size */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_size))(self);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size__unsupported(DeeObject *__restrict self) {
	return (size_t)err_set_unsupportedf(self, "operator size()");
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size__with__set_operator_sizeob(DeeObject *__restrict self) {
	DREF DeeObject *sizeob;
	sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__set_operator_size__with__set_operator_foreach(DeeObject *__restrict self) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_foreach))(self, &default_seq_size_with_foreach_cb, NULL);
}


/* set_operator_hash */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_hash))(self);
}

#ifndef DEFINED_set_handle_hash_error
#define DEFINED_set_handle_hash_error
PRIVATE NONNULL((1)) Dee_hash_t DCALL
set_handle_hash_error(DeeObject *self) {
	DeeError_Print("Unhandled error in `Set.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}
#endif /* !DEFINED_set_handle_hash_error */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with_callattr___set_hash__(DeeObject *__restrict self) {
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
	return set_handle_hash_error(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with_callobjectcache___set_hash__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_hash__with_callobjectcache___set_hash__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_hash__, 1, (DeeObject *const *)&self);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return set_handle_hash_error(self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__empty(DeeObject *__restrict UNUSED(self)) {
	return DEE_HASHOF_EMPTY_SEQUENCE;
}

#ifndef DEFINED_default_set_hash_with_foreach_cb
#define DEFINED_default_set_hash_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_hash_with_foreach_cb(void *arg, DeeObject *elem) {
	*(Dee_hash_t *)arg ^= DeeObject_Hash(elem);
	return 0;
}
#endif /* !DEFINED_default_set_hash_with_foreach_cb */
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
default__set_operator_hash__with__set_operator_foreach(DeeObject *__restrict self) {
	Dee_hash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_foreach))(self, &default_set_hash_with_foreach_cb, &result))
		goto err;
	return result;
err:
	return set_handle_hash_error(self);
}


/* set_operator_compare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_compare_eq))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_compare_eq__with_callattr___set_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(lhs, (DeeObject *)&str___set_compare_eq__, 1, &rhs);
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
default__set_operator_compare_eq__with_callobjectcache___set_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_compare_eq__with_callobjectcache___set_compare_eq__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_compare_eq__, lhs, 1, &rhs);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	return err_set_unsupportedf(lhs, "__set_compare_eq__(%r)", rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_compare_eq__with__set_operator_eq(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_eq))(lhs, rhs);
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
default__set_operator_compare_eq__with__set_operator_ne(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_ne))(lhs, rhs);
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
default__set_operator_compare_eq__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireNativeOperator(tp_rhs, contains);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_foreach))(lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		return 1; /* "rhs" is missing some element of "lhs" */
	ASSERT(tp_rhs == Dee_TYPE(rhs));
	rhs_size = DeeObject_Size(rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status != rhs_size)
		return 1; /* Sets have different sizes */
	return 0;
err:
	return Dee_COMPARE_ERR;
}


/* set_operator_trycompare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_trycompare_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_trycompare_eq))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_trycompare_eq__with__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_compare_eq))(lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}


/* set_operator_eq */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_eq))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_eq__with_callattr___set_eq__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_eq__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_eq__with_callobjectcache___set_eq__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_eq__with_callobjectcache___set_eq__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_eq__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_set_unsupportedf(lhs, "__set_eq__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_eq__with__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_compare_eq))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}


/* set_operator_ne */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ne(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_ne))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ne__with_callattr___set_ne__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_ne__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ne__with_callobjectcache___set_ne__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_ne__with_callobjectcache___set_ne__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_ne__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_set_unsupportedf(lhs, "__set_ne__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ne__with__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_compare_eq))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}


/* set_operator_lo */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_lo(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_lo))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_lo__with_callattr___set_lo__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_lo__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_lo__with_callobjectcache___set_lo__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_lo__with_callobjectcache___set_lo__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_lo__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_set_unsupportedf(lhs, "__set_lo__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_lo__empty(DeeObject *UNUSED(lhs), DeeObject *rhs) {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return_bool(rhs_nonempty != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_lo__with__set_operator_ge(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_ge))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_lo__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireNativeOperator(Dee_TYPE(rhs), contains);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_foreach))(lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	rhs_size = DeeObject_Size(rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains elements not found in "lhs" */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}


/* set_operator_le */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_le))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le__with_callattr_issubset(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_issubset, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le__with_callattr___set_le__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_le__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le__with_callobjectcache___set_le__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_le__with_callobjectcache___set_le__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_le__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_set_unsupportedf(lhs, "__set_le__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le__empty(DeeObject *UNUSED(lhs), DeeObject *UNUSED(rhs)) {
	return_true;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le__with__set_operator_gr(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_gr))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_le__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireNativeOperator(Dee_TYPE(rhs), contains);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_foreach))(lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}


/* set_operator_gr */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_gr(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_gr))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_gr__with_callattr___set_gr__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_gr__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_gr__with_callobjectcache___set_gr__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_gr__with_callobjectcache___set_gr__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_gr__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_set_unsupportedf(lhs, "__set_gr__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_gr__empty(DeeObject *UNUSED(lhs), DeeObject *UNUSED(rhs)) {
	return_false;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_gr__with__set_operator_le(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_le))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_gr__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireNativeOperator(Dee_TYPE(rhs), contains);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_foreach))(lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}


/* set_operator_ge */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_ge))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge__with_callattr_issuperset(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_issuperset, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge__with_callattr___set_ge__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_ge__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge__with_callobjectcache___set_ge__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_ge__with_callobjectcache___set_ge__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_ge__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_set_unsupportedf(lhs, "__set_ge__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge__empty(DeeObject *UNUSED(lhs), DeeObject *rhs) {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return_bool(rhs_nonempty == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge__with__set_operator_lo(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_lo))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_ge__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireNativeOperator(Dee_TYPE(rhs), contains);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_foreach))(lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	rhs_size = DeeObject_Size(rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains elements not found in "lhs" */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}


/* set_operator_inv */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_inv(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inv))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_inv__with_callattr___set_size__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___set_size__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_inv__with_callobjectcache___set_size__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_inv__with_callobjectcache___set_size__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_size__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_operator_inv__unsupported(DeeObject *__restrict self) {
	return (DREF DeeObject *)SetInversion_New(self);
}


/* set_operator_add */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_add(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_add))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_add__with_callattr_union(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_union, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_add__with_callattr___set_add__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_add__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_add__with_callobjectcache___set_add__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_add__with_callobjectcache___set_add__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_add__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_add__unsupported(DeeObject *lhs, DeeObject *rhs) {
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a | ~b' --> `~(~a & b)'
		 * -> Keep the inversion on the outside, since it prevents enumeration. */
		SetInversion *xrhs = (SetInversion *)rhs;
		DREF SetInversion *inv_lhs;
		DREF SetIntersection *intersection;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return_reference_(lhs);
		inv_lhs = SetInversion_New(lhs);
		if unlikely(!inv_lhs)
			goto err;
		intersection = SetIntersection_New_inherit_b(inv_lhs, xrhs->si_set);
		if unlikely(!intersection)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(intersection);
	}
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs);
	return (DREF DeeObject *)SetUnion_New(lhs, rhs);
err:
	return NULL;
}


/* set_operator_sub */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_sub(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_sub))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_sub__with_callattr_difference(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_difference, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_sub__with_callattr___set_sub__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_sub__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_sub__with_callobjectcache___set_sub__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_sub__with_callobjectcache___set_sub__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_sub__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_sub__unsupported(DeeObject *lhs, DeeObject *rhs) {
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a - ~b' -> `a & b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeObject_InvokeMethodHint(set_operator_and, lhs, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs); /* `a - {}' -> `a' */
	return (DREF DeeObject *)SetDifference_New(lhs, rhs);
}


/* set_operator_and */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_and(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_and))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_and__with_callattr_intersection(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_intersection, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_and__with_callattr___set_and__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_and__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_and__with_callobjectcache___set_and__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_and__with_callobjectcache___set_and__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_and__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_and__unsupported(DeeObject *lhs, DeeObject *rhs) {
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a & ~b' -> `a - b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeObject_InvokeMethodHint(set_operator_sub, lhs, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(Dee_EmptySet); /* `a & {}' -> `{}' */
	return (DREF DeeObject *)SetIntersection_New(lhs, rhs);
}


/* set_operator_xor */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_xor(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), set_operator_xor))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_xor__with_callattr_symmetric_difference(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_symmetric_difference, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_xor__with_callattr___set_xor__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___set_xor__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_xor__with_callobjectcache___set_xor__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_xor__with_callobjectcache___set_xor__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___set_xor__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_operator_xor__unsupported(DeeObject *lhs, DeeObject *rhs) {
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a ^ ~b' -> `~(a ^ b)'
		 * -> Keep the inversion on the outside, since it prevents enumeration. */
		SetInversion *xrhs = (SetInversion *)rhs;
		DREF SetSymmetricDifference *symdiff;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return DeeObject_InvokeMethodHint(set_operator_inv, lhs); /* `a ^ ~{}' -> `~a' */
		symdiff = SetSymmetricDifference_New(lhs, xrhs->si_set);
		if unlikely(!symdiff)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(symdiff);
	}
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs); /* `a ^ {}' -> `a' */
	return (DREF DeeObject *)SetSymmetricDifference_New(lhs, rhs);
err:
	return NULL;
}


/* set_operator_inplace_add */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_add(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_add))(p_self, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_add__with_callattr___set_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___set_inplace_add__, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_add__with_callobjectcache___set_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_inplace_add__with_callobjectcache___set_inplace_add__(Dee_TYPE(*p_self), p_self, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___set_inplace_add__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_add__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(set_operator_add, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_add__with__set_insertall(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_insertall))(*p_self, rhs);
}


/* set_operator_inplace_sub */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_sub(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_sub))(p_self, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_sub__with_callattr___set_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___set_inplace_sub__, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_sub__with_callobjectcache___set_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_inplace_sub__with_callobjectcache___set_inplace_sub__(Dee_TYPE(*p_self), p_self, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___set_inplace_sub__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_sub__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a -= ~b' -> `a &= b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_and))(p_self, xrhs->si_set);
	}
	result = DeeObject_InvokeMethodHint(set_operator_sub, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_sub__with__set_operator_foreach__and__set_removeall(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a -= ~b' -> `a &= b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_and))(p_self, xrhs->si_set);
	}
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_removeall))(*p_self, rhs);
}


/* set_operator_inplace_and */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_and(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_and))(p_self, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_and__with_callattr___set_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___set_inplace_and__, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_and__with_callobjectcache___set_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_inplace_and__with_callobjectcache___set_inplace_and__(Dee_TYPE(*p_self), p_self, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___set_inplace_and__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_and__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_sub))(p_self, xrhs->si_set);
	}
	result = DeeObject_InvokeMethodHint(set_operator_and, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_and__with__set_operator_foreach__and__set_removeall(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	int result;
	DREF DeeObject *keys_to_remove_proxy;
	DREF DeeObject *keys_to_remove;
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_sub))(p_self, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(rhs))
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), seq_clear))(*p_self);

	/* `a &= b' -> `(a as Set).removeall((((a as Set) - b) as Set).frozen)' */
	keys_to_remove_proxy = DeeObject_InvokeMethodHint(set_operator_sub, *p_self, rhs);
	if unlikely(!keys_to_remove_proxy)
		goto err;
	keys_to_remove = DeeObject_InvokeMethodHint(set_frozen, (DeeObject *)keys_to_remove_proxy);
	Dee_Decref_likely(keys_to_remove_proxy);
	if unlikely(!keys_to_remove)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_removeall))(*p_self, keys_to_remove);
	Dee_Decref_likely(keys_to_remove);
	return result;
err:
	return -1;
}


/* set_operator_inplace_xor */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_xor(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_xor))(p_self, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_xor__with_callattr___set_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___set_inplace_xor__, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_xor__with_callobjectcache___set_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_operator_inplace_xor__with_callobjectcache___set_inplace_xor__(Dee_TYPE(*p_self), p_self, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___set_inplace_xor__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_xor__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(set_operator_xor, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_operator_inplace_xor__with__set_operator_foreach__and__set_insertall__and__set_removeall(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *only_in_rhs_proxy;
	DREF DeeObject *only_in_rhs;
	if (DeeSet_CheckEmpty(rhs))
		return 0;

	/* >> a ^= b
	 * <=>
	 * >> local only_in_rhs = (((b as Set) - a) as Set).frozen;
	 * >> a.removeall(b);
	 * >> a.insertall(only_in_rhs); */
	only_in_rhs_proxy = DeeObject_InvokeMethodHint(set_operator_sub, rhs, *p_self);
	if unlikely(!only_in_rhs_proxy)
		goto err;
	only_in_rhs = DeeObject_InvokeMethodHint(set_frozen, (DeeObject *)only_in_rhs_proxy);
	Dee_Decref(only_in_rhs_proxy);
	if unlikely(!only_in_rhs)
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_removeall))(*p_self, rhs))
		goto err_only_in_rhs;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_insertall))(*p_self, only_in_rhs))
		goto err_only_in_rhs;
	Dee_Decref(only_in_rhs);
	return 0;
err_only_in_rhs:
	Dee_Decref(only_in_rhs);
err:
	return -1;
}


/* set_frozen */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_frozen(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_frozen))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_frozen__with_callattr_frozen(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_frozen);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_frozen__with_callattr___set_frozen__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___set_frozen__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_frozen__with_callobjectcache___set_frozen__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_frozen__with_callobjectcache___set_frozen__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___set_frozen__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_frozen__unsupported(DeeObject *__restrict self) {
	err_set_unsupportedf(self, "__set_frozen__()");
	return NULL;
}


/* set_unify */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_unify__with_callattr_unify(DeeObject *self, DeeObject *key) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_unify, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_unify__with_callattr___set_unify__(DeeObject *self, DeeObject *key) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___set_unify__, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_unify__with_callobjectcache___set_unify__(DeeObject *self, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_unify__with_callobjectcache___set_unify__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_unify__, self, 1, &key);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_unify__unsupported(DeeObject *self, DeeObject *key) {
	err_set_unsupportedf(self, "__set_unify__(%r)", key);
	return NULL;
}

#ifndef DEFINED_set_unify_foreach_cb
#define DEFINED_set_unify_foreach_cb
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
#endif /* !DEFINED_set_unify_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_unify__with__seq_operator_foreach__and__set_insert(DeeObject *self, DeeObject *key) {
	int status = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_insert))(self, key);
	if unlikely(status < 0)
		goto err;
	if (!status) {
		struct set_unify_foreach_data data;
		Dee_ssize_t fe_status;
		data.sufd_key = key;
		DBG_memset(&data.sufd_result, 0xcc, sizeof(data.sufd_result));
		fe_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &set_unify_foreach_cb, &data);
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

#ifndef DEFINED_seq_locate_item_foreach_cb
#define DEFINED_seq_locate_item_foreach_cb
#define SET_LOCATE_ITEM_FOREACH_FOUND ((Dee_ssize_t)-2)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_item_foreach_cb(void *arg, DeeObject *key) {
	int temp;
	DeeObject *search_key = *(DeeObject **)arg;
	temp = DeeObject_TryCompareEq(search_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(key);
		*(DeeObject **)arg = key;
		return SET_LOCATE_ITEM_FOREACH_FOUND;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_locate_item_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_unify__with__seq_operator_foreach__and__seq_append(DeeObject *self, DeeObject *key) {
	Dee_ssize_t status;
	Dee_Incref(key);
	status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach))(self, &seq_locate_item_foreach_cb, &key);
	if (status == SET_LOCATE_ITEM_FOREACH_FOUND)
		return key;
	if (status < 0)
		goto err_key_nokill;
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_append))(self, key))
		goto err_key_nokill;
	return key;
err_key_nokill:
	Dee_DecrefNokill(key); /* *Nokill because caller still has a reference */
/*err:*/
	return NULL;
}


/* set_insert */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insert__with_callattr_insert(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_insert, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insert__with_callattr___set_insert__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___set_insert__, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insert__with_callobjectcache___set_insert__(DeeObject *self, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_insert__with_callobjectcache___set_insert__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_insert__, self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insert__unsupported(DeeObject *self, DeeObject *key) {
	return err_set_unsupportedf(self, "__set_insert__(%r)", key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insert__with__map_setnew(DeeObject *self, DeeObject *key) {
	int result;
	DREF DeeObject *map_key_and_value[2];
	if unlikely(DeeObject_Unpack(key, 2, map_key_and_value))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew))(self, map_key_and_value[0], map_key_and_value[1]);
	Dee_Decref(map_key_and_value[1]);
	Dee_Decref(map_key_and_value[0]);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insert__with__seq_operator_size__and__set_insertall(DeeObject *self, DeeObject *key) {
	int temp;
	size_t old_size, new_size;
	DREF DeeObject *items;
	old_size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	items = DeeTuple_NewVectorSymbolic(1, &key);
	if unlikely(!items)
		goto err;
	temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_insertall))(self, items);
	DeeTuple_DecrefSymbolic(items);
	if unlikely(temp)
		goto err;
	new_size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insert__with__seq_contains__and__seq_append(DeeObject *self, DeeObject *key) {
	int contains = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains))(self, key);
	if unlikely(contains < 0)
		goto err;
	if (contains)
		return 0;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_append))(self, key))
		goto err;
	return 1;
err:
	return -1;
}


/* set_insertall */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insertall__with_callattr_insertall(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_insertall, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insertall__with_callattr___set_insertall__(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___set_insertall__, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insertall__with_callobjectcache___set_insertall__(DeeObject *self, DeeObject *keys) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_insertall__with_callobjectcache___set_insertall__(Dee_TYPE(self), self, keys);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_insertall__, self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insertall__unsupported(DeeObject *self, DeeObject *keys) {
	return err_set_unsupportedf(self, "__set_insertall__(%r)", keys);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insertall__with__set_operator_inplace_add(DeeObject *self, DeeObject *keys) {
	int result;
	Dee_Incref(self);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_add))(&self, keys);
	Dee_Decref(self);
	return result;
}

#ifndef set_insertall_foreach_cb_PTR
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define set_insertall_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireMethodHint(Dee_TYPE(self), set_insert))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define set_insertall_foreach_cb_PTR &set_insertall_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_insertall_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(set_insert, (DeeObject *)arg, elem);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !set_insertall_foreach_cb_PTR */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_insertall__with__set_insert(DeeObject *self, DeeObject *keys) {
	Dee_ssize_t status = DeeObject_InvokeMethodHint(seq_operator_foreach, keys, set_insertall_foreach_cb_PTR, self);
	return likely(status >= 0) ? 0 : -1;
}


/* set_remove */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_remove__with_callattr_remove(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_remove, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_remove__with_callattr___set_remove__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___set_remove__, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_remove__with_callobjectcache___set_remove__(DeeObject *self, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_remove__with_callobjectcache___set_remove__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_remove__, self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_remove__unsupported(DeeObject *self, DeeObject *key) {
	return err_set_unsupportedf(self, "__set_remove__(%r)", key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_remove__with__map_operator_trygetitem__and__map_operator_delitem(DeeObject *self, DeeObject *key) {
	int temp;
	DREF DeeObject *current_value;
	DREF DeeObject *map_key_and_value[2];
	if unlikely(DeeObject_Unpack(key, 2, map_key_and_value))
		goto err;
	current_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, map_key_and_value[0]);
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
	temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, map_key_and_value[0]);
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
default__set_remove__with__seq_operator_size__and__set_removeall(DeeObject *self, DeeObject *key) {
	int temp;
	size_t old_size, new_size;
	DREF DeeObject *items;
	old_size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	items = DeeTuple_NewVectorSymbolic(1, &key);
	if unlikely(!items)
		goto err;
	temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_removeall))(self, items);
	DeeTuple_DecrefSymbolic(items);
	if unlikely(temp)
		goto err;
	new_size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_remove__with__seq_removeall(DeeObject *self, DeeObject *key) {
	size_t count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall))(self, key, 0, (size_t)-1, (size_t)-1);
	if unlikely(count == (size_t)-1)
		goto err;
	return count ? 1 : 0;
err:
	return -1;
}


/* set_removeall */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_removeall__with_callattr_removeall(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_removeall, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_removeall__with_callattr___set_removeall__(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___set_removeall__, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_removeall__with_callobjectcache___set_removeall__(DeeObject *self, DeeObject *keys) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_removeall__with_callobjectcache___set_removeall__(Dee_TYPE(self), self, keys);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_removeall__, self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_removeall__unsupported(DeeObject *self, DeeObject *keys) {
	return err_set_unsupportedf(self, "__set_removeall__(%r)", keys);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_removeall__with__set_operator_inplace_sub(DeeObject *self, DeeObject *keys) {
	int result;
	Dee_Incref(self);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_sub))(&self, keys);
	Dee_Decref(self);
	return result;
}

#ifndef set_removeall_foreach_cb_PTR
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define set_removeall_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireMethodHint(Dee_TYPE(self), set_remove))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define set_removeall_foreach_cb_PTR &set_removeall_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_removeall_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(set_remove, (DeeObject *)arg, elem);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !set_removeall_foreach_cb_PTR */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__set_removeall__with__set_remove(DeeObject *self, DeeObject *keys) {
	Dee_ssize_t status = DeeObject_InvokeMethodHint(seq_operator_foreach, keys, set_removeall_foreach_cb_PTR, self);
	return likely(status >= 0) ? 0 : -1;
}


/* set_pop */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_pop__with_callattr_pop(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_pop, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_pop__with_callattr___set_pop__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___set_pop__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_pop__with_callobjectcache___set_pop__(DeeObject *self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_pop__with_callobjectcache___set_pop__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_pop__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_pop__unsupported(DeeObject *self) {
	err_set_unsupportedf(self, "__set_pop__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_pop__empty(DeeObject *self) {
	err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_pop__with__seq_trygetfirst__and__set_remove(DeeObject *self) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetfirst))(self);
	if unlikely(!ITER_ISOK(result)) {
		if (result == ITER_DONE)
			err_empty_sequence(self);
		goto err;
	}
	if unlikely(DeeSet_InvokeRemove(self, result) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__set_pop__with__map_popitem(DeeObject *self) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_popitem))(self);
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
default__set_pop__with__seq_pop(DeeObject *self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_pop))(self, -1);
}


/* set_pop_with_default */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__with_callattr_pop(DeeObject *self, DeeObject *default_) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_pop, 1, &default_);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__with_callattr___set_pop__(DeeObject *self, DeeObject *default_) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___set_pop__, 1, &default_);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__with_callobjectcache___set_pop__(DeeObject *self, DeeObject *default_) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__set_pop_with_default__with_callobjectcache___set_pop__(Dee_TYPE(self), self, default_);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___set_pop__, self, 1, &default_);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__unsupported(DeeObject *self, DeeObject *default_) {
	err_set_unsupportedf(self, "__set_pop__(%r)", default_);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__empty(DeeObject *UNUSED(self), DeeObject *default_) {
	return_reference_(default_);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__with__seq_trygetfirst__and__set_remove(DeeObject *self, DeeObject *default_) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetfirst))(self);
	if unlikely(!ITER_ISOK(result)) {
		if (result == ITER_DONE)
			return_reference_(default_);
		goto err;
	}
	if unlikely(DeeSet_InvokeRemove(self, result) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__with__map_popitem(DeeObject *self, DeeObject *default_) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_popitem))(self);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		return_reference_(default_);
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__set_pop_with_default__with__seq_pop(DeeObject *self, DeeObject *default_) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_pop))(self, -1);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_ValueError))
			return_reference_(default_);
		goto err;
	}
	return result;
err:
	return NULL;
}


/* map_operator_iter */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_iter(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_iter))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_iter__with_callattr___map_iter__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_iter__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_iter__with_callobjectcache___map_iter__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_iter__with_callobjectcache___map_iter__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_iter__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_iter__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "operator iter()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_iter__with__seq_operator_iter(DeeObject *__restrict self) {
	DREF DeeObject *iter;
	DeeTypeObject *itertyp;
	DREF DistinctMappingIterator *result;
	iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	result = DeeGCObject_MALLOC(DistinctMappingIterator);
	if unlikely(!result)
		goto err_iter;
	itertyp                 = Dee_TYPE(iter);
	result->dmi_tp_nextpair = DeeType_RequireNativeOperator(itertyp, nextpair);
	result->dmi_iter        = iter; /* Inherit reference */
	Dee_simple_hashset_with_lock_init(&result->dmi_encountered);
	DeeObject_Init(result, &DistinctMappingIterator_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}


/* map_operator_foreach_pair */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_foreach_pair))(self, cb, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_operator_foreach_pair__with__map_operator_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_iter))(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}

#ifndef DEFINED_default_map_foreach_pair_unique_cb
#define DEFINED_default_map_foreach_pair_unique_cb
struct default_map_foreach_pair_unique_data {
	struct Dee_simple_hashset dmfpud_encountered; /* Set of keys already encountered. */
	Dee_foreach_pair_t        dmfpud_cb;          /* [1..1] user-defined callback */
	void                     *dmfpud_arg;         /* [?..?] Cookie for `dmfpud_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_map_foreach_pair_unique_cb(void *arg, DeeObject *key, DeeObject *value) {
	int insert_status;
	struct default_map_foreach_pair_unique_data *data;
	data = (struct default_map_foreach_pair_unique_data *)arg;
	insert_status = Dee_simple_hashset_insert(&data->dmfpud_encountered, key);
	if likely(insert_status > 0)
		return (*data->dmfpud_cb)(data->dmfpud_arg, key, value);
	return insert_status; /* error, or already-exists */
}
#endif /* !DEFINED_default_map_foreach_pair_unique_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_operator_foreach_pair__with__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result;
	struct default_map_foreach_pair_unique_data data;
	data.dmfpud_cb  = cb;
	data.dmfpud_arg = arg;
	Dee_simple_hashset_init(&data.dmfpud_encountered);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_foreach_pair))(self, &default_map_foreach_pair_unique_cb, &data);
	Dee_simple_hashset_fini(&data.dmfpud_encountered);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_operator_foreach_pair__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct default_foreach_pair_with_map_enumerate_data data;
	data.dfpwme_cb  = cb;
	data.dfpwme_arg = arg;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &default_foreach_pair_with_map_enumerate_cb, &data);
}


/* map_operator_sizeob */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_sizeob(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sizeob))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_sizeob__with_callattr___map_size__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_size__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_sizeob__with_callobjectcache___map_size__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_sizeob__with_callobjectcache___map_size__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_size__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_sizeob__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "operator size()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_sizeob__with__map_operator_size(DeeObject *__restrict self) {
	size_t mapsize = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_size))(self);
	if unlikely(mapsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(mapsize);
err:
	return NULL;
}


/* map_operator_size */
INTERN WUNUSED NONNULL((1)) size_t DCALL
default__map_operator_size(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_size))(self);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__map_operator_size__unsupported(DeeObject *__restrict self) {
	return (size_t)err_map_unsupportedf(self, "operator size()");
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__map_operator_size__with__map_operator_sizeob(DeeObject *__restrict self) {
	DREF DeeObject *sizeob;
	sizeob = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sizeob))(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__map_operator_size__with__map_operator_foreach_pair(DeeObject *__restrict self) {
	return (size_t)(*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_foreach_pair))(self, &default_seq_size_with_foreach_pair_cb, NULL);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_getitem__with_callobjectcache___map_getitem__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_getitem__, self, 1, &key);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__unsupported(DeeObject *self, DeeObject *key) {
	err_map_unsupportedf(self, "operator [](%r)", key);
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem__empty(DeeObject *self, DeeObject *key) {
	err_unknown_key(self, key);
	return NULL;
}

#ifndef DEFINED_default_map_getitem_with_enumerate_cb
#define DEFINED_default_map_getitem_with_enumerate_cb
struct default_map_getitem_with_enumerate_data {
	DeeObject      *mgied_key;    /* [1..1] The key we're looking for. */
	DREF DeeObject *mgied_result; /* [?..1][out] Result value. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	struct default_map_getitem_with_enumerate_data *data;
	data = (struct default_map_getitem_with_enumerate_data *)arg;
	temp = DeeObject_TryCompareEq(data->mgied_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgied_result = value;
		return -2;
	}
	return 0;
err:
	return -1;
}
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(key)) {
	return ITER_DONE;
}

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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_index__empty(DeeObject *self, size_t key) {
	err_unknown_key_int(self, key);
	return NULL;
}


/* map_operator_trygetitem_index */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_index(DeeObject *self, size_t key) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_index))(self, key);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_index__empty(DeeObject *UNUSED(self), size_t UNUSED(key)) {
	return ITER_DONE;
}


/* map_operator_getitem_string_hash */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_hash))(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t UNUSED(hash)) {
	err_map_unsupportedf(self, "operator [](%q)", key);
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_getitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t UNUSED(hash)) {
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

#ifndef DEFINED_string_hash_equals_object
#define DEFINED_string_hash_equals_object
PRIVATE WUNUSED NONNULL((1, 3)) bool DCALL
string_hash_equals_object(char const *lhs, Dee_hash_t lhs_hash, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return (DeeString_Hash(rhs) == lhs_hash && strcmp(lhs, DeeString_STR(rhs)) == 0);
	if (DeeBytes_Check(rhs))
		return (strlen(lhs) == DeeBytes_SIZE(rhs) && bcmp(lhs, DeeBytes_DATA(rhs), DeeBytes_SIZE(rhs)) == 0);
	/* `string.operator ==' isn't implemented for any other types. */
	return false;
}
#endif /* !DEFINED_string_hash_equals_object */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_getitem_string_hash_with_enumerate_data *data;
	data = (struct default_map_getitem_string_hash_with_enumerate_data *)arg;
	if (string_hash_equals_object(data->mgished_key, data->mgished_hash, key)) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgished_result = value;
		return -2;
	}
	return 0;
}
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


/* map_operator_trygetitem_string_hash */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_hash))(self, key, hash);
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_hash__empty(DeeObject *UNUSED(self), char const *UNUSED(key), Dee_hash_t UNUSED(hash)) {
	return ITER_DONE;
}

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


/* map_operator_getitem_string_len_hash */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem_string_len_hash))(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t UNUSED(hash)) {
	err_map_unsupportedf(self, "operator [](%$q)", keylen, key);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_getitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t UNUSED(hash)) {
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

#ifndef DEFINED_string_len_hash_equals_object
#define DEFINED_string_len_hash_equals_object
PRIVATE WUNUSED NONNULL((1, 4)) bool DCALL
string_len_hash_equals_object(char const *lhs, size_t lhs_len, Dee_hash_t lhs_hash, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return (DeeString_Hash(rhs) == lhs_hash && DeeString_EqualsBuf(rhs, lhs, lhs_len));
	if (DeeBytes_Check(rhs))
		return (lhs_len == DeeBytes_SIZE(rhs) && bcmp(lhs, DeeBytes_DATA(rhs), lhs_len) == 0);
	/* `string.operator ==' isn't implemented for any other types. */
	return false;
}
#endif /* !DEFINED_string_len_hash_equals_object */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_getitem_string_len_hash_with_enumerate_data *data;
	data = (struct default_map_getitem_string_len_hash_with_enumerate_data *)arg;
	if (string_len_hash_equals_object(data->mgislhed_key, data->mgislhed_keylen, data->mgislhed_hash, key)) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgislhed_result = value;
		return -2;
	}
	return 0;
}
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


/* map_operator_trygetitem_string_len_hash */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem_string_len_hash))(self, key, keylen, hash);
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_operator_trygetitem_string_len_hash__empty(DeeObject *UNUSED(self), char const *UNUSED(key), size_t UNUSED(keylen), Dee_hash_t UNUSED(hash)) {
	return ITER_DONE;
}

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

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(key)) {
	return Dee_BOUND_MISSING;
}

#ifndef DEFINED_default_map_bounditem_with_enumerate_cb
#define DEFINED_default_map_bounditem_with_enumerate_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	(void)value;
	temp = DeeObject_TryCompareEq((DeeObject *)arg, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0)
		return value ? -2 : -3; /* Stop iteration */
	return 0;
err:
	return -1;
}
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

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_index__empty(DeeObject *UNUSED(self), size_t UNUSED(key)) {
	return Dee_BOUND_MISSING;
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

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_bounditem_string_hash__empty(DeeObject *UNUSED(self), char const *UNUSED(key), Dee_hash_t UNUSED(hash)) {
	return Dee_BOUND_MISSING;
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

INTERN WUNUSED NONNULL((1)) int DCALL
default__map_operator_bounditem_string_len_hash__empty(DeeObject *UNUSED(self), char const *UNUSED(key), size_t UNUSED(keylen), Dee_hash_t UNUSED(hash)) {
	return Dee_BOUND_MISSING;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_delitem__with_callobjectcache___map_delitem__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_delitem__, self, 1, &key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

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

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with__map_remove(DeeObject *self, DeeObject *key) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_remove))(self, key);
	if (result > 0)
		result = 0;
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_delitem__with__map_removekeys(DeeObject *self, DeeObject *key) {
	int result;
	DREF DeeTupleObject *keys = DeeTuple_NewUninitialized(1);
	if unlikely(!keys)
		goto err;
	keys->t_elem[0] = key;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_removekeys))(self, (DeeObject *)keys);
	DeeTuple_DecrefSymbolic((DeeObject *)keys);
	return result;
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
default__map_operator_delitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t UNUSED(hash)) {
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
default__map_operator_delitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t UNUSED(hash)) {
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_setitem__with_callobjectcache___map_setitem__(Dee_TYPE(self), self, key, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_setitem__, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__unsupported(DeeObject *self, DeeObject *key, DeeObject *value) {
	return err_map_unsupportedf(self, "operator []=(%r, %r)", key, value);
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

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_operator_setitem__empty(DeeObject *self, DeeObject *key, DeeObject *value) {
	(void)value;
	return err_unknown_key(self, key);
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

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__map_operator_setitem_index__empty(DeeObject *self, size_t key, DeeObject *value) {
	(void)value;
	return err_unknown_key_int(self, key);
}


/* map_operator_setitem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__map_operator_setitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_hash))(self, key, hash, value);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__map_operator_setitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t UNUSED(hash), DeeObject *UNUSED(value)) {
	return err_map_unsupportedf(self, "operator []=(%q)", key);
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

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__map_operator_setitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	(void)hash;
	(void)value;
	return err_unknown_key_str(self, key);
}


/* map_operator_setitem_string_len_hash */
INTERN WUNUSED NONNULL((1, 5)) int DCALL
default__map_operator_setitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem_string_len_hash))(self, key, keylen, hash, value);
}

INTERN WUNUSED NONNULL((1, 5)) int DCALL
default__map_operator_setitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t UNUSED(hash), DeeObject *UNUSED(value)) {
	return err_map_unsupportedf(self, "operator []=(%$q)", keylen, key);
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

INTERN WUNUSED NONNULL((1, 5)) int DCALL
default__map_operator_setitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	(void)hash;
	(void)value;
	return err_unknown_key_str_len(self, key, keylen);
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_contains__with_callobjectcache___map_contains__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_contains__, self, 1, &key);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__unsupported(DeeObject *self, DeeObject *key) {
	err_map_unsupportedf(self, "operator contains(%r)", key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_contains__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(key)) {
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


/* map_keys */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_keys(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_keys))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_keys__with_callattr_keys(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_keys);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_keys__with_callattr___map_keys__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___map_keys__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_keys__with_callobjectcache___map_keys__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_keys__with_callobjectcache___map_keys__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___map_keys__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_keys__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "__map_keys__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_keys__empty(DeeObject *__restrict UNUSED(self)) {
	return_reference_(Dee_EmptySet);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_keys__with__map_iterkeys(DeeObject *__restrict self) {
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


/* map_iterkeys */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_iterkeys))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with_callattr_iterkeys(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_iterkeys);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with_callattr___map_iterkeys__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___map_iterkeys__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with_callobjectcache___map_iterkeys__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_iterkeys__with_callobjectcache___map_iterkeys__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___map_iterkeys__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "__map_iterkeys__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with__map_keys(DeeObject *__restrict self) {
	DREF DeeObject *result, *keys = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_keys))(self);
	if unlikely(!keys)
		goto err;
	result = DeeObject_Iter(keys);
	Dee_Decref_unlikely(keys); /* *_unlikely because it's probably referenced by the iterator */
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with__map_enumerate(DeeObject *__restrict self) {
	/* TODO: Custom iterator type that uses "tp_enumerate" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_iterkeys__with__map_operator_iter(DeeObject *__restrict self) {
	/* NOTE: This only works when the mapping can't have unbound keys! */
	DeeTypeObject *itertyp;
	DREF DefaultIterator_PairSubItem *result;
	result = DeeObject_MALLOC(DefaultIterator_PairSubItem);
	if unlikely(!result)
		goto err;
	result->dipsi_iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_iter))(self);
	if unlikely(!result->dipsi_iter)
		goto err_r;
	itertyp            = Dee_TYPE(result->dipsi_iter);
	result->dipsi_next = DeeType_RequireNativeOperator(itertyp, nextkey);
	DeeObject_Init(result, &DefaultIterator_WithNextKey);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


/* map_values */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_values(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_values))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_values__with_callattr_values(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_values);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_values__with_callattr___map_values__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___map_values__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_values__with_callobjectcache___map_values__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_values__with_callobjectcache___map_values__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___map_values__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_values__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "__map_values__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_values__empty(DeeObject *__restrict UNUSED(self)) {
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_values__with__map_itervalues(DeeObject *__restrict self) {
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


/* map_itervalues */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_itervalues(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_itervalues))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_itervalues__with_callattr_itervalues(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_itervalues);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_itervalues__with_callattr___map_itervalues__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___map_itervalues__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_itervalues__with_callobjectcache___map_itervalues__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_itervalues__with_callobjectcache___map_itervalues__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___map_itervalues__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_itervalues__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "__map_itervalues__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_itervalues__with__map_values(DeeObject *__restrict self) {
	DREF DeeObject *result, *values = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_values))(self);
	if unlikely(!values)
		goto err;
	result = DeeObject_Iter(values);
	Dee_Decref_unlikely(values); /* *_unlikely because it's probably referenced by the iterator */
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_itervalues__with__map_operator_iter(DeeObject *__restrict self) {
	DeeTypeObject *itertyp;
	DREF DefaultIterator_PairSubItem *result;
	result = DeeObject_MALLOC(DefaultIterator_PairSubItem);
	if unlikely(!result)
		goto err;
	result->dipsi_iter = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_iter))(self);
	if unlikely(!result->dipsi_iter)
		goto err_r;
	itertyp            = Dee_TYPE(result->dipsi_iter);
	result->dipsi_next = DeeType_RequireNativeOperator(itertyp, nextvalue);
	DeeObject_Init(result, &DefaultIterator_WithNextValue);
	return (DREF DeeObject *)result;
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_enumerate__with_callobjectcache___map_enumerate__(Dee_TYPE(self), self, cb, arg);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t UNUSED(cb), void *UNUSED(arg)) {
	return err_map_unsupportedf(self, "__map_enumerate__(<callback>)");
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__map_enumerate__with__map_enumerate_range(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	/* TODO */
	(void)self;
	(void)cb;
	(void)arg;
	return DeeError_NOTIMPLEMENTED();
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
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_enumerate_range__with_callobjectcache___map_enumerate__(Dee_TYPE(self), self, cb, arg, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[3];
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	args[0] = (DeeObject *)wrapper;
	args[1] = start;
	args[2] = end;
	result  = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate__, self, 3, args);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
default__map_enumerate_range__unsupported(DeeObject *self, Dee_seq_enumerate_t UNUSED(cb), void *UNUSED(arg), DeeObject *start, DeeObject *end) {
	return err_map_unsupportedf(self, "__map_enumerate__(<callable>, %r, %r)", start, end);
}

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
default__map_enumerate_range__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *UNUSED(start), DeeObject *UNUSED(end)) {
	/* TODO */
	(void)self;
	(void)cb;
	(void)arg;
	return DeeError_NOTIMPLEMENTED();
}


/* map_makeenumeration */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_makeenumeration__with_callattr___map_enumerate_items__(DeeObject *__restrict self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_enumerate_items__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_makeenumeration__with_callobjectcache___map_enumerate_items__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_makeenumeration__with_callobjectcache___map_enumerate_items__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate_items__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_makeenumeration__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "__map_enumerate_items__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_makeenumeration__with__map_operator_iter(DeeObject *__restrict self) {
	return DeeSuper_New(&DeeMapping_Type, self); /* DefaultEnumeration__with__map_operator_iter__and__unpack */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_makeenumeration__with__map_iterkeys__and__map_operator_getitem(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_makeenumeration__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_makeenumeration__with__map_enumerate(DeeObject *__restrict self) {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__map_enumerate, self);
}


/* map_makeenumeration_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_makeenumeration_with_range__with_callattr___map_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_enumerate_items__, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_makeenumeration_with_range__with_callobjectcache___map_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_makeenumeration_with_range__with_callobjectcache___map_enumerate_items__(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_enumerate_items__, self, 2, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_makeenumeration_with_range__unsupported(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_map_unsupportedf(self, "__map_enumerate_items__(%r, %r)", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_makeenumeration_with_range__with__map_operator_iter(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack, self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_makeenumeration_with_range__with__map_iterkeys__and__map_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem, self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_makeenumeration_with_range__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem, self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_makeenumeration_with_range__with__map_enumerate_range(DeeObject *self, DeeObject *start, DeeObject *end) {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_enumerate_range, self, start, end);
}


/* map_operator_compare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_compare_eq))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_compare_eq__with_callattr___map_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = DeeObject_CallAttr(lhs, (DeeObject *)&str___map_compare_eq__, 1, &rhs);
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
default__map_operator_compare_eq__with_callobjectcache___map_compare_eq__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_compare_eq__with_callobjectcache___map_compare_eq__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_compare_eq__, lhs, 1, &rhs);
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
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	return err_map_unsupportedf(lhs, "__map_compare_eq__(%r)", rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_compare_eq__with__map_operator_eq(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_eq))(lhs, rhs);
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
default__map_operator_compare_eq__with__map_operator_ne(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_ne))(lhs, rhs);
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
default__map_operator_compare_eq__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs; /* Important: must treat "rhs" as a mapping for this compare! */
	data.mc_lfr_rtrygetitem = DeeType_RequireMethodHint(Dee_TYPE(rhs), map_operator_trygetitem);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_foreach_pair))(lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		return 1; /* "rhs" is missing some element of "lhs", or has a different value for it */
	rhs_size = DeeObject_InvokeMethodHint(set_operator_size, rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status != rhs_size)
		return 1; /* Maps have different sizes */
	return 0;
err:
	return Dee_COMPARE_ERR;
}


/* map_operator_trycompare_eq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_trycompare_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_trycompare_eq))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_trycompare_eq__with__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_compare_eq))(lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}


/* map_operator_eq */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_eq(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_eq))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_eq__with_callattr___map_eq__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_eq__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_eq__with_callobjectcache___map_eq__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_eq__with_callobjectcache___map_eq__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_eq__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_map_unsupportedf(lhs, "__map_eq__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_eq__with__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_compare_eq))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}


/* map_operator_ne */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ne(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_ne))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ne__with_callattr___map_ne__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_ne__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ne__with_callobjectcache___map_ne__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_ne__with_callobjectcache___map_ne__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_ne__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_map_unsupportedf(lhs, "__map_ne__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ne__with__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs) {
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_compare_eq))(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}


/* map_operator_lo */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_lo(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_lo))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_lo__with_callattr___map_lo__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_lo__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_lo__with_callobjectcache___map_lo__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_lo__with_callobjectcache___map_lo__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_lo__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_map_unsupportedf(lhs, "__map_lo__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_lo__with__map_operator_ge(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_ge))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_lo__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_foreach_pair))(lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	rhs_size = DeeMap_OperatorSize(rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains element not found in "lhs" */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}


/* map_operator_le */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_le(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_le))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_le__with_callattr___map_le__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_le__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_le__with_callobjectcache___map_le__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_le__with_callobjectcache___map_le__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_le__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_map_unsupportedf(lhs, "__map_le__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_le__with__map_operator_gr(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_gr))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_le__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_foreach_pair))(lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}


/* map_operator_gr */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_gr(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_gr))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_gr__with_callattr___map_gr__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_gr__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_gr__with_callobjectcache___map_gr__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_gr__with_callobjectcache___map_gr__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_gr__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_map_unsupportedf(lhs, "__map_gr__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_gr__with__map_operator_le(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_le))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_gr__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_foreach_pair))(lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}


/* map_operator_ge */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ge(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_ge))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ge__with_callattr___map_ge__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_ge__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ge__with_callobjectcache___map_ge__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_ge__with_callobjectcache___map_ge__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_ge__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs) {
	err_map_unsupportedf(lhs, "__map_ge__(%r)", rhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ge__with__map_operator_lo(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_lo))(lhs, rhs);
	return xinvoke_not(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_ge__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_foreach_pair))(lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	rhs_size = DeeMap_OperatorSize(rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains element not found in "lhs" */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}


/* map_operator_add */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_add(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_add))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_add__with_callattr_union(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_union, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_add__with_callattr___map_add__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_add__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_add__with_callobjectcache___map_add__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_add__with_callobjectcache___map_add__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_add__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_add__unsupported(DeeObject *lhs, DeeObject *rhs) {
	if (DeeMap_CheckEmpty(rhs))
		return_reference_(lhs);
	return (DREF DeeObject *)MapUnion_New(lhs, rhs);
}


/* map_operator_sub */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_sub(DeeObject *lhs, DeeObject *keys) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_sub))(lhs, keys);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_sub__with_callattr_difference(DeeObject *lhs, DeeObject *keys) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_difference, 1, &keys);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_sub__with_callattr___map_sub__(DeeObject *lhs, DeeObject *keys) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_sub__, 1, &keys);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_sub__with_callobjectcache___map_sub__(DeeObject *lhs, DeeObject *keys) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_sub__with_callobjectcache___map_sub__(Dee_TYPE(lhs), lhs, keys);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_sub__, lhs, 1, &keys);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_sub__unsupported(DeeObject *lhs, DeeObject *keys) {
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a - ~b' -> `a & b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return DeeObject_InvokeMethodHint(map_operator_and, lhs, xkeys->si_set);
	}
	if (DeeSet_CheckEmpty(keys))
		return_reference_(lhs); /* `a - {}' -> `a' */
	return (DREF DeeObject *)MapDifference_New(lhs, keys);
}


/* map_operator_and */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_and(DeeObject *lhs, DeeObject *keys) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_and))(lhs, keys);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_and__with_callattr_intersection(DeeObject *lhs, DeeObject *keys) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_intersection, 1, &keys);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_and__with_callattr___map_and__(DeeObject *lhs, DeeObject *keys) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_and__, 1, &keys);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_and__with_callobjectcache___map_and__(DeeObject *lhs, DeeObject *keys) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_and__with_callobjectcache___map_and__(Dee_TYPE(lhs), lhs, keys);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_and__, lhs, 1, &keys);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_and__unsupported(DeeObject *lhs, DeeObject *keys) {
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a & ~b' -> `a - b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return DeeObject_InvokeMethodHint(map_operator_sub, lhs, xkeys->si_set);
	}
	if (DeeSet_CheckEmpty(keys))
		return_reference_(Dee_EmptyMapping); /* `a & {}' -> `{}' */
	return (DREF DeeObject *)MapIntersection_New(lhs, keys);
}


/* map_operator_xor */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_xor(DeeObject *lhs, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(lhs), map_operator_xor))(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_xor__with_callattr_symmetric_difference(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str_symmetric_difference, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_xor__with_callattr___map_xor__(DeeObject *lhs, DeeObject *rhs) {
	return DeeObject_CallAttr(lhs, (DeeObject *)&str___map_xor__, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_xor__with_callobjectcache___map_xor__(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_xor__with_callobjectcache___map_xor__(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(lhs), Dee_TYPE(lhs)->tp_mhcache->mhc___map_xor__, lhs, 1, &rhs);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_operator_xor__unsupported(DeeObject *lhs, DeeObject *rhs) {
	if (DeeMap_CheckEmpty(rhs))
		return_reference_(lhs);
	return (DREF DeeObject *)MapSymmetricDifference_New(lhs, rhs);
}


/* map_operator_inplace_add */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_add(DREF DeeObject **__restrict p_self, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_add))(p_self, items);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_add__with_callattr___map_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *items) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___map_inplace_add__, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_add__with_callobjectcache___map_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *items) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_inplace_add__with_callobjectcache___map_inplace_add__(Dee_TYPE(*p_self), p_self, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___map_inplace_add__, *p_self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_add__unsupported(DREF DeeObject **__restrict p_self, DeeObject *items) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(map_operator_add, *p_self, items);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_add__with__map_update(DREF DeeObject **__restrict p_self, DeeObject *items) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_update))(*p_self, items);
}


/* map_operator_inplace_sub */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_sub(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_sub))(p_self, keys);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_sub__with_callattr___map_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___map_inplace_sub__, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_sub__with_callobjectcache___map_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *keys) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_inplace_sub__with_callobjectcache___map_inplace_sub__(Dee_TYPE(*p_self), p_self, keys);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___map_inplace_sub__, *p_self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_sub__unsupported(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	DREF DeeObject *result;
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a -= ~b' -> `a &= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_and))(p_self, xkeys->si_set);
	}
	result = DeeObject_InvokeMethodHint(map_operator_sub, *p_self, keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_sub__with__map_removekeys(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a -= ~b' -> `a &= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_and))(p_self, xkeys->si_set);
	}
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_removekeys))(*p_self, keys);
}


/* map_operator_inplace_and */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_and(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_and))(p_self, keys);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_and__with_callattr___map_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___map_inplace_and__, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_and__with_callobjectcache___map_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *keys) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_inplace_and__with_callobjectcache___map_inplace_and__(Dee_TYPE(*p_self), p_self, keys);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___map_inplace_and__, *p_self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_and__unsupported(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	DREF DeeObject *result;
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_sub))(p_self, xkeys->si_set);
	}
	result = DeeObject_InvokeMethodHint(map_operator_and, *p_self, keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_and__with__map_operator_foreach_pair__and__map_removekeys(DREF DeeObject **__restrict p_self, DeeObject *keys) {
	int result;
	DREF DeeObject *a_keys;
	DREF DeeObject *a_keys_without_b_proxy;
	DREF DeeObject *a_keys_without_b;
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_sub))(p_self, xkeys->si_set);
	}

	/* `a &= {}' -> `(a as Sequence).clear()' */
	if (DeeSet_CheckEmpty(keys))
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), seq_clear))(*p_self);

	/* `a &= b' -> `(a as Mapping).removekeys(((((a as Mapping).keys as Set) - b) as Set).frozen)' */
	a_keys = (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_keys))(*p_self);
	if unlikely(!a_keys)
		goto err;
	a_keys_without_b_proxy = DeeObject_InvokeMethodHint(set_operator_sub, a_keys, keys);
	Dee_Decref(a_keys);
	if unlikely(!a_keys_without_b_proxy)
		goto err;
	a_keys_without_b = DeeObject_InvokeMethodHint(set_frozen, a_keys_without_b_proxy);
	Dee_Decref(a_keys_without_b_proxy);
	if unlikely(!a_keys_without_b)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_removekeys))(*p_self, a_keys_without_b);
	Dee_Decref(a_keys_without_b);
	return result;
err:
	return -1;
}


/* map_operator_inplace_xor */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_xor(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_xor))(p_self, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_xor__with_callattr___map_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(*p_self, (DeeObject *)&str___map_inplace_xor__, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_xor__with_callobjectcache___map_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_operator_inplace_xor__with_callobjectcache___map_inplace_xor__(Dee_TYPE(*p_self), p_self, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(*p_self), Dee_TYPE(*p_self)->tp_mhcache->mhc___map_inplace_xor__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_xor__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(map_operator_xor, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_operator_inplace_xor__with__map_operator_foreach_pair__and__map_update__and__map_removekeys(DREF DeeObject **__restrict p_self, DeeObject *rhs) {
	/* >> a ^= b
	 * <=>
	 * >> local a_keys = (a as Mapping).keys;
	 * >> local b_keys = (b as Mapping).keys;
	 * >> local a_and_b_keys = (((a_keys as Set) & b_keys) as Set).frozen;
	 * >> (a as Mapping).removekeys(b_keys);
	 * >> (a as Mapping).update((b as Mapping) - a_and_b_keys); */
	int result;
	DREF DeeObject *a_keys;
	DREF DeeObject *b_keys;
	DREF DeeObject *a_and_b_keys_proxy;
	DREF DeeObject *a_and_b_keys;
	DREF DeeObject *b_without_a_keys;
	a_keys = (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_keys))(*p_self);
	if unlikely(!a_keys)
		goto err;
	b_keys = DeeObject_InvokeMethodHint(map_keys, rhs);
	if unlikely(!b_keys)
		goto err_a_keys;
	a_and_b_keys_proxy = DeeObject_InvokeMethodHint(set_operator_and, a_keys, b_keys);
	Dee_Decref(a_keys);
	if unlikely(!a_and_b_keys_proxy)
		goto err_b_keys;
	a_and_b_keys = DeeObject_InvokeMethodHint(set_frozen, a_and_b_keys_proxy);
	Dee_Decref(a_and_b_keys_proxy);
	if unlikely(!a_and_b_keys)
		goto err_b_keys;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_removekeys))(*p_self, b_keys);
	Dee_Decref(b_keys);
	if unlikely(result)
		goto err_a_and_b_keys;
	b_without_a_keys = DeeObject_InvokeMethodHint(map_operator_sub, rhs, a_and_b_keys);
	Dee_Decref(a_and_b_keys);
	if unlikely(!b_without_a_keys)
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_update))(*p_self, b_without_a_keys);
	Dee_Decref(b_without_a_keys);
	return result;
err_a_and_b_keys:
	Dee_Decref(a_and_b_keys);
err:
	return -1;
err_a_keys:
	Dee_Decref(a_keys);
	goto err;
err_b_keys:
	Dee_Decref(b_keys);
	goto err;
}


/* map_frozen */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_frozen(DeeObject *__restrict self) {
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_frozen))(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_frozen__with_callattr_frozen(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_frozen);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_frozen__with_callattr___map_frozen__(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str___map_frozen__);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_frozen__with_callobjectcache___map_frozen__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_frozen__with_callobjectcache___map_frozen__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc_get___map_frozen__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_frozen__unsupported(DeeObject *__restrict self) {
	err_map_unsupportedf(self, "__map_frozen__()");
	return NULL;
}


/* map_setold */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setold__with_callattr_setold(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_setold, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setold__with_callattr___map_setold__(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_setold__, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setold__with_callobjectcache___map_setold__(DeeObject *self, DeeObject *key, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_setold__with_callobjectcache___map_setold__(Dee_TYPE(self), self, key, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_setold__, self, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setold__unsupported(DeeObject *self, DeeObject *key, DeeObject *value) {
	return err_map_unsupportedf(self, "__map_setold__(%r, %r)", key, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setold__with__map_setold_ex(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setold_ex))(self, key, value);
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
default__map_setold__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if (!ITER_ISOK(old_value)) {
		if unlikely(!old_value)
			goto err;
		return 0; /* Key doesn't exist */
	}
	Dee_Decref(old_value);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}


/* map_setold_ex */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setold_ex__with_callattr_setold_ex(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_setold_ex, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setold_ex__with_callattr___map_setold_ex__(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_setold_ex__, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setold_ex__with_callobjectcache___map_setold_ex__(DeeObject *self, DeeObject *key, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_setold_ex__with_callobjectcache___map_setold_ex__(Dee_TYPE(self), self, key, value);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_setold_ex__, self, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setold_ex__unsupported(DeeObject *self, DeeObject *key, DeeObject *value) {
	err_map_unsupportedf(self, "__map_setold_ex__(%r, %r)", key, value);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setold_ex__with__map_operator_trygetitem__and__map_setold(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if (ITER_ISOK(old_value)) {
		int status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setold))(self, key, value);
		if unlikely(status <= 0) {
			if unlikely(status < 0)
				goto err_old_value;
			Dee_Decref(old_value);
			return ITER_DONE;
		}
	}
	return old_value;
err_old_value:
	Dee_Decref(old_value);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setold_ex__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
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


/* map_setnew */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setnew__with_callattr_setnew(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_setnew, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setnew__with_callattr___map_setnew__(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_setnew__, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setnew__with_callobjectcache___map_setnew__(DeeObject *self, DeeObject *key, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_setnew__with_callobjectcache___map_setnew__(Dee_TYPE(self), self, key, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_setnew__, self, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setnew__unsupported(DeeObject *self, DeeObject *key, DeeObject *value) {
	return err_map_unsupportedf(self, "__map_setnew__(%r, %r)", key, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setnew__with__map_setnew_ex(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew_ex))(self, key, value);
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
default__map_setnew__with__map_operator_trygetitem__and__map_setdefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *temp;
	DREF DeeObject *bound = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if (bound != ITER_DONE) {
		if unlikely(!bound)
			goto err;
		Dee_Decref(bound);
		return 0; /* Key already exists */
	}
	temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setdefault))(self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__map_setnew__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *bound = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if (bound != ITER_DONE) {
		if unlikely(!bound)
			goto err;
		Dee_Decref(bound);
		return 0; /* Key already exists */
	}
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, key, value))
		goto err;
	return 1;
err:
	return -1;
}


/* map_setnew_ex */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setnew_ex__with_callattr_setnew_ex(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_setnew_ex, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setnew_ex__with_callattr___map_setnew_ex__(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_setnew_ex__, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setnew_ex__with_callobjectcache___map_setnew_ex__(DeeObject *self, DeeObject *key, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_setnew_ex__with_callobjectcache___map_setnew_ex__(Dee_TYPE(self), self, key, value);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_setnew_ex__, self, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setnew_ex__unsupported(DeeObject *self, DeeObject *key, DeeObject *value) {
	err_map_unsupportedf(self, "__map_setnew_ex__(%r, %r)", key, value);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setnew_ex__with__map_operator_trygetitem__and__map_setnew(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew))(self, key, value) < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setnew_ex__with__map_operator_trygetitem__and__map_setdefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *temp;
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setdefault))(self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, key, value))
		goto err;
	return ITER_DONE;
err:
	return NULL;
}


/* map_setdefault */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setdefault__with_callattr_setdefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	return DeeObject_CallAttr(self, (DeeObject *)&str_setdefault, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setdefault__with_callattr___map_setdefault__(DeeObject *self, DeeObject *key, DeeObject *value) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_setdefault__, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setdefault__with_callobjectcache___map_setdefault__(DeeObject *self, DeeObject *key, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_setdefault__with_callobjectcache___map_setdefault__(Dee_TYPE(self), self, key, value);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_setdefault__, self, 2, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setdefault__unsupported(DeeObject *self, DeeObject *key, DeeObject *value) {
	err_map_unsupportedf(self, "__map_setdefault__(%r, %r)", key, value);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setdefault__with__map_setnew_ex(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew_ex))(self, key, value);
	if (old_value == ITER_DONE) {
		/* Value was just inserted */
		old_value = value;
		Dee_Incref(value);
	}
	return old_value;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setdefault__with__map_setnew__and__map_operator_getitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew))(self, key, value);
	if unlikely(temp < 0)
		goto err;
	if (temp > 0)
		return_reference_(value);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_setdefault__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if (result == ITER_DONE) {
		if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, key, value))
			goto err;
		result = value;
		Dee_Incref(value);
	}
	return result;
err:
	return NULL;
}


/* map_update */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_update__with_callattr_update(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str_update, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_update__with_callattr___map_update__(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___map_update__, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_update__with_callobjectcache___map_update__(DeeObject *self, DeeObject *items) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_update__with_callobjectcache___map_update__(Dee_TYPE(self), self, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_update__, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_update__unsupported(DeeObject *self, DeeObject *items) {
	return err_map_unsupportedf(self, "__map_update__(%r)", items);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_update__empty(DeeObject *self, DeeObject *items) {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__map_update__unsupported(self, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_update__with__map_operator_inplace_add(DeeObject *self, DeeObject *items) {
	int result;
	Dee_Incref(self);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_add))(&self, items);
	Dee_Decref(self);
	return result;
}

#ifndef map_update_foreach_cb_PTR
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define map_update_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *, DeeObject *))(Dee_funptr_t)&DeeObject_SetItem)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define map_update_foreach_cb_PTR &map_update_foreach_cb
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
map_update_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	return (Dee_ssize_t)DeeObject_SetItem((DeeObject *)arg, key, value);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !map_update_foreach_cb_PTR */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_update__with__map_operator_setitem(DeeObject *self, DeeObject *items) {
	return (int)DeeObject_InvokeMethodHint(seq_operator_foreach_pair, items, map_update_foreach_cb_PTR, self);
}


/* map_remove */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_remove__with_callattr_remove(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str_remove, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_remove__with_callattr___map_remove__(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result = DeeObject_CallAttr(self, (DeeObject *)&str___map_remove__, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_remove__with_callobjectcache___map_remove__(DeeObject *self, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_remove__with_callobjectcache___map_remove__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_remove__, self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_remove__unsupported(DeeObject *self, DeeObject *key) {
	return err_map_unsupportedf(self, "__map_remove__(%r)", key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_remove__with__map_operator_bounditem__and__map_operator_delitem(DeeObject *self, DeeObject *key) {
	int bound = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_bounditem))(self, key);
	if unlikely(Dee_BOUND_ISERR(bound))
		goto err;
	if (!Dee_BOUND_ISBOUND(bound))
		return 0;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_remove__with__seq_operator_size__and__map_operator_delitem(DeeObject *self, DeeObject *key) {
	size_t new_size;
	size_t old_size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	if unlikely(old_size == 0)
		return 0;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key))
		goto err;
	new_size = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_size))(self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}


/* map_removekeys */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_removekeys__with_callattr_removekeys(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_removekeys, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_removekeys__with_callattr___map_removekeys__(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str___map_removekeys__, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_removekeys__with_callobjectcache___map_removekeys__(DeeObject *self, DeeObject *keys) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_removekeys__with_callobjectcache___map_removekeys__(Dee_TYPE(self), self, keys);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	result = mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_removekeys__, self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_removekeys__unsupported(DeeObject *self, DeeObject *keys) {
	return err_map_unsupportedf(self, "__map_removekeys__(%r)", keys);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_removekeys__with__map_operator_inplace_sub(DeeObject *self, DeeObject *keys) {
	int result;
	Dee_Incref(self);
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_sub))(&self, keys);
	Dee_Decref(self);
	return result;
}

#ifndef map_removekeys_foreach_cb_PTR
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define map_removekeys_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireMethodHint(Dee_TYPE(self), map_remove))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define map_removekeys_foreach_cb_PTR &map_removekeys_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
map_removekeys_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(map_remove, (DeeObject *)arg, elem);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !map_removekeys_foreach_cb_PTR */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__map_removekeys__with__map_remove(DeeObject *self, DeeObject *keys) {
	Dee_ssize_t status = DeeObject_InvokeMethodHint(seq_operator_foreach, keys, map_removekeys_foreach_cb_PTR, self);
	return likely(status >= 0) ? 0 : -1;
}


/* map_pop */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_pop__with_callattr_pop(DeeObject *self, DeeObject *key) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_pop, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_pop__with_callattr___map_pop__(DeeObject *self, DeeObject *key) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_pop__, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_pop__with_callobjectcache___map_pop__(DeeObject *self, DeeObject *key) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_pop__with_callobjectcache___map_pop__(Dee_TYPE(self), self, key);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_pop__, self, 1, &key);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_pop__unsupported(DeeObject *self, DeeObject *key) {
	err_map_unsupportedf(self, "__map_pop__(%r)", key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_pop__empty(DeeObject *self, DeeObject *key) {
	err_unknown_key(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__map_pop__with__map_operator_getitem__and__map_operator_delitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, key);
	if unlikely(!result)
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


/* map_pop_with_default */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_pop_with_default__with_callattr_pop(DeeObject *self, DeeObject *key, DeeObject *default_) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = default_;
	return DeeObject_CallAttr(self, (DeeObject *)&str_pop, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_pop_with_default__with_callattr___map_pop__(DeeObject *self, DeeObject *key, DeeObject *default_) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = default_;
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_pop__, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_pop_with_default__with_callobjectcache___map_pop__(DeeObject *self, DeeObject *key, DeeObject *default_) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_pop_with_default__with_callobjectcache___map_pop__(Dee_TYPE(self), self, key, default_);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = key;
	args[1] = default_;
	return mhcache_thiscall(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_pop__, self, 2, args);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_pop_with_default__unsupported(DeeObject *self, DeeObject *key, DeeObject *default_) {
	err_map_unsupportedf(self, "__map_pop__(%r, %r)", key, default_);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_pop_with_default__empty(DeeObject *UNUSED(self), DeeObject *UNUSED(key), DeeObject *default_) {
	return_reference_(default_);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__map_pop_with_default__with__map_operator_trygetitem__and__map_operator_delitem(DeeObject *self, DeeObject *key, DeeObject *default_) {
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem))(self, key);
	if unlikely(!result)
		goto err;
	if (result == ITER_DONE)
		return_reference_(default_);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


/* map_popitem */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_popitem__with_callattr_popitem(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_popitem, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_popitem__with_callattr___map_popitem__(DeeObject *self) {
	return DeeObject_CallAttr(self, (DeeObject *)&str___map_popitem__, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_popitem__with_callobjectcache___map_popitem__(DeeObject *self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__map_popitem__with_callobjectcache___map_popitem__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return mhcache_call(Dee_TYPE(self), Dee_TYPE(self)->tp_mhcache->mhc___map_popitem__, 1, (DeeObject *const *)&self);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_popitem__unsupported(DeeObject *self) {
	err_map_unsupportedf(self, "__map_popitem__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_popitem__empty(DeeObject *UNUSED(self)) {
	return_none;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__map_popitem__with__seq_trygetlast__and__map_operator_delitem(DeeObject *self) {
	int temp;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetlast))(self);
	if (result == ITER_DONE)
		return_none;
	if unlikely(!result)
		goto err;
	if unlikely(DeeObject_Unpack(result, 2, key_and_value))
		goto err_r;
	Dee_Decref(key_and_value[1]);
	temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key_and_value[0]);
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
default__map_popitem__with__seq_trygetfirst__and__map_operator_delitem(DeeObject *self) {
	int temp;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_trygetfirst))(self);
	if (result == ITER_DONE)
		return_none;
	if unlikely(!result)
		goto err;
	if unlikely(DeeObject_Unpack(result, 2, key_and_value))
		goto err_r;
	Dee_Decref(key_and_value[1]);
	temp = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if unlikely(temp)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */




























/************************************************************************/
/* TYPED WRAPPERS (for Super)                                           */
/************************************************************************/

/* clang-format off */
/*[[[deemon (printTypedDefaultImpls from "..method-hints.method-hints")();]]]*/
/* seq_operator_bool */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_operator_bool__with_callobjectcache___seq_bool__(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result = mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_bool__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_operator_sizeob */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_operator_sizeob__with_callobjectcache___seq_size__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_size__, 1, (DeeObject *const *)&self);
}

/* seq_operator_iter */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_operator_iter__with_callobjectcache___seq_iter__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_iter__, 1, (DeeObject *const *)&self);
}

/* seq_operator_getitem */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_operator_getitem__with_callobjectcache___seq_getitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_getitem__, self, 1, &index);
}

/* seq_operator_delitem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_operator_delitem__with_callobjectcache___seq_delitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_delitem__, self, 1, &index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_operator_delitem_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_operator_delitem_index__with_callobjectcache___seq_delitem__(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_delitem__, self, PCKuSIZ, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_operator_setitem */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__seq_operator_setitem__with_callobjectcache___seq_setitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_setitem__, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_operator_setitem_index */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__seq_operator_setitem_index__with_callobjectcache___seq_setitem__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_setitem__, self, PCKuSIZ "o", index, value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_operator_getrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__seq_operator_getrange__with_callobjectcache___seq_getrange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_getrange__, self, 2, args);
}

/* seq_operator_delrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__seq_operator_delrange__with_callobjectcache___seq_delrange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_delrange__, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_operator_setrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
tdefault__seq_operator_setrange__with_callobjectcache___seq_setrange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = items;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_setrange__, self, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_operator_assign */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_operator_assign__with_callobjectcache___seq_assign__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_assign__, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_operator_hash */
#ifndef DEFINED_seq_handle_hash_error
#define DEFINED_seq_handle_hash_error
PRIVATE NONNULL((1)) Dee_hash_t DCALL
seq_handle_hash_error(DeeObject *self) {
	DeeError_Print("Unhandled error in `Sequence.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}
#endif /* !DEFINED_seq_handle_hash_error */
INTERN WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL
tdefault__seq_operator_hash__with_callobjectcache___seq_hash__(DeeTypeObject *tp_self, DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_hash__, 1, (DeeObject *const *)&self);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return seq_handle_hash_error(self);
}

/* seq_operator_compare */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_operator_compare__with_callobjectcache___seq_compare__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_compare__, lhs, 1, &rhs);
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

/* seq_operator_compare_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_compare_eq__, lhs, 1, &rhs);
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

/* seq_operator_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_operator_eq__with_callobjectcache___seq_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_eq__, lhs, 1, &rhs);
}

/* seq_operator_ne */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_operator_ne__with_callobjectcache___seq_ne__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_ne__, lhs, 1, &rhs);
}

/* seq_operator_lo */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_operator_lo__with_callobjectcache___seq_lo__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_lo__, lhs, 1, &rhs);
}

/* seq_operator_le */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_operator_le__with_callobjectcache___seq_le__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_le__, lhs, 1, &rhs);
}

/* seq_operator_gr */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_operator_gr__with_callobjectcache___seq_gr__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_gr__, lhs, 1, &rhs);
}

/* seq_operator_ge */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_operator_ge__with_callobjectcache___seq_ge__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_ge__, lhs, 1, &rhs);
}

/* seq_operator_inplace_add */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_inplace_add__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* seq_operator_inplace_mul */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *repeat) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_inplace_mul__, *p_self, 1, &repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* seq_enumerate */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_enumerate__with_callobjectcache___seq_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t cb, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_enumerate__, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

/* seq_enumerate_index */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_enumerate_index__with_callobjectcache___seq_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_enumerate__, self, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

/* seq_makeenumeration */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_enumerate_items__, 1, (DeeObject *const *)&self);
}

/* seq_makeenumeration_with_range */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_enumerate_items__, self, 2, args);
}

/* seq_makeenumeration_with_intrange */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_makeenumeration_with_intrange__with_callobjectcache___seq_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_enumerate_items__, self, PCKuSIZ PCKuSIZ, start, end);
}

/* seq_unpack */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__seq_unpack__with_callobjectcache___seq_unpack__(DeeTypeObject *tp_self, DeeObject *self, size_t count, DREF DeeObject *result[]) {
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_unpack__, self, PCKuSIZ, count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	if (DeeTuple_SIZE(resultob) != count) {
		err_invalid_unpack_size(resultob, count, DeeTuple_SIZE(resultob));
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return 0;
err_r:
	Dee_Decref_likely(resultob);
err:
	return -1;
}

/* seq_unpack_ex */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
tdefault__seq_unpack_ex__with_callobjectcache___seq_unpack__(DeeTypeObject *tp_self, DeeObject *self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t result_count;
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_unpack__, self, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), result_count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return result_count;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)-1;
}

/* seq_unpack_ub */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
tdefault__seq_unpack_ub__with_callobjectcache___seq_unpackub__(DeeTypeObject *tp_self, DeeObject *self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	size_t result_count;
	DREF DeeObject *resultob;
	resultob = min_count == max_count ? mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_unpackub__, self, PCKuSIZ, min_count)
	                                  : mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_unpackub__, self, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (!DeeObject_InstanceOfExact(resultob, &DeeTuple_Type) &&
	    !DeeObject_InstanceOfExact(resultob, &DeeNullableTuple_Type)) {
		DeeObject_TypeAssertFailed2(resultob, &DeeTuple_Type, &DeeNullableTuple_Type);
		goto err_r;
	}
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		err_invalid_unpack_size_minmax(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	/* XXX: DeeNullableTuple_DecrefSymbolic(resultob); */
	Dee_XMovrefv(result, DeeTuple_ELEM(resultob), result_count);
	Dee_Decref_likely(resultob);
	return result_count;
err_r:
	Dee_Decref_likely(resultob);
err:
	return (size_t)-1;
}

/* seq_trygetfirst */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_trygetfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result = mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___seq_first__, 1, (DeeObject *const *)&self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

/* seq_getfirst */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_getfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___seq_first__, 1, (DeeObject *const *)&self);
}

/* seq_boundfirst */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_boundfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call_bound(tp_self, tp_self->tp_mhcache->mhc_get___seq_first__, 1, (DeeObject *const *)&self);
}

/* seq_delfirst */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_delfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call_int(tp_self, tp_self->tp_mhcache->mhc_del___seq_first__, 1, (DeeObject *const *)&self);
}

/* seq_setfirst */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_setfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	return mhcache_thiscall_int(tp_self, tp_self->tp_mhcache->mhc_set___seq_first__, self, 1, (DeeObject *const *)&value);
}

/* seq_trygetlast */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_trygetlast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result = mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___seq_last__, 1, (DeeObject *const *)&self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

/* seq_getlast */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_getlast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___seq_last__, 1, (DeeObject *const *)&self);
}

/* seq_boundlast */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_boundlast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call_bound(tp_self, tp_self->tp_mhcache->mhc_get___seq_last__, 1, (DeeObject *const *)&self);
}

/* seq_dellast */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_dellast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call_int(tp_self, tp_self->tp_mhcache->mhc_del___seq_last__, 1, (DeeObject *const *)&self);
}

/* seq_setlast */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_setlast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	return mhcache_thiscall_int(tp_self, tp_self->tp_mhcache->mhc_set___seq_last__, self, 1, (DeeObject *const *)&value);
}

/* seq_cached */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_cached__with_callobjectcache___seq_cached__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___seq_cached__, 1, (DeeObject *const *)&self);
}

/* seq_frozen */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_frozen__with_callobjectcache___seq_frozen__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___seq_frozen__, 1, (DeeObject *const *)&self);
}

/* seq_any */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_any__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result = mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_any__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_any_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_any_with_key__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_any__, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_any_with_range */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_any_with_range__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_any__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_any_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
tdefault__seq_any_with_range_and_key__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_any__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_all */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_all__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result = mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_all__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_all_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_all_with_key__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_all__, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_all_with_range */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_all_with_range__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_all__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_all_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
tdefault__seq_all_with_range_and_key__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_all__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_parity */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_parity__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result = mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_parity__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_parity_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_parity_with_key__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_parity__, self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_parity_with_range */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_parity_with_range__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_parity__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_parity_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
tdefault__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_parity__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_reduce */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_reduce__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_reduce__, self, 1, &combine);
}

/* seq_reduce_with_init */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__seq_reduce_with_init__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine, DeeObject *init) {
	DeeObject *args[4];
	args[0] = combine;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = init;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_reduce__, self, 4, args);
}

/* seq_reduce_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_reduce_with_range__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_reduce__, self, "o" PCKuSIZ PCKuSIZ, combine, start, end);
}

/* seq_reduce_with_range_and_init */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *DCALL
tdefault__seq_reduce_with_range_and_init__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_reduce__, self, "o" PCKuSIZ PCKuSIZ "o", combine, start, end, init);
}

/* seq_min */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_min__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_min__, 1, (DeeObject *const *)&self);
}

/* seq_min_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_min_with_key__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_min__, self, 3, args);
}

/* seq_min_with_range */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_min_with_range__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_min__, self, PCKuSIZ PCKuSIZ, start, end);
}

/* seq_min_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
tdefault__seq_min_with_range_and_key__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_min__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}

/* seq_max */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_max__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_max__, 1, (DeeObject *const *)&self);
}

/* seq_max_with_key */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__seq_max_with_key__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_max__, self, 3, args);
}

/* seq_max_with_range */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_max_with_range__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_max__, self, PCKuSIZ PCKuSIZ, start, end);
}

/* seq_max_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
tdefault__seq_max_with_range_and_key__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_max__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}

/* seq_sum */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_sum__with_callobjectcache___seq_sum__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_sum__, 1, (DeeObject *const *)&self);
}

/* seq_sum_with_range */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_sum_with_range__with_callobjectcache___seq_sum__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_sum__, self, PCKuSIZ PCKuSIZ, start, end);
}

/* seq_count */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_count__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_count__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

/* seq_count_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL
tdefault__seq_count_with_key__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_count__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

/* seq_count_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_count_with_range__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_count__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

/* seq_count_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL
tdefault__seq_count_with_range_and_key__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_count__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

/* seq_contains */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_contains__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_contains__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_contains_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__seq_contains_with_key__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_contains__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_contains_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_contains_with_range__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_contains__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_contains_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_contains__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_locate */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__seq_locate__with_callobjectcache___seq_locate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_locate__, self, 2, args);
}

/* seq_locate_with_range */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *DCALL
tdefault__seq_locate_with_range__with_callobjectcache___seq_locate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_locate__, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

/* seq_rlocate */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__seq_rlocate__with_callobjectcache___seq_rlocate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_rlocate__, self, 2, args);
}

/* seq_rlocate_with_range */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *DCALL
tdefault__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_rlocate__, self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

/* seq_startswith */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_startswith__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_startswith__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_startswith_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__seq_startswith_with_key__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_startswith__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_startswith_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_startswith_with_range__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_startswith__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_startswith_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_startswith__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_endswith */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_endswith__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_endswith__, self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_endswith_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__seq_endswith_with_key__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_endswith__, self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_endswith_with_range */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_endswith_with_range__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_endswith__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_endswith_with_range_and_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_endswith__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_find */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_find__with_callobjectcache___seq_find__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_find__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
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

/* seq_find_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL
tdefault__seq_find_with_key__with_callobjectcache___seq_find__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_find__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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

/* seq_rfind */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_rfind__with_callobjectcache___seq_rfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_rfind__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
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

/* seq_rfind_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL
tdefault__seq_rfind_with_key__with_callobjectcache___seq_rfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_rfind__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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

/* seq_erase */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_erase__with_callobjectcache___seq_erase__(DeeTypeObject *tp_self, DeeObject *self, size_t index, size_t count) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_erase__, self, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_insert */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__seq_insert__with_callobjectcache___seq_insert__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_insert__, self, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_insertall */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__seq_insertall__with_callobjectcache___seq_insertall__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *items) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_insertall__, self, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_pushfront */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_pushfront__with_callobjectcache___seq_pushfront__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_pushfront__, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_append */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_append__with_callobjectcache___seq_append__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_append__, self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_extend */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_extend__with_callobjectcache___seq_extend__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___seq_extend__, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_xchitem_index */
INTERN WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
tdefault__seq_xchitem_index__with_callobjectcache___seq_xchitem__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *item) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_xchitem__, self, PCKuSIZ "o", index, item);
}

/* seq_clear */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_clear__with_callobjectcache___seq_clear__(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
	result = mhcache_call(tp_self, tp_self->tp_mhcache->mhc___seq_clear__, 1, (DeeObject *const *)&self);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

/* seq_pop */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_pop__with_callobjectcache___seq_pop__(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t index) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_pop__, self, PCKdSIZ, index);
}

/* seq_remove */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_remove__with_callobjectcache___seq_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_remove__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_remove_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__seq_remove_with_key__with_callobjectcache___seq_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_remove__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_rremove */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__seq_rremove__with_callobjectcache___seq_rremove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_rremove__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_rremove_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__seq_rremove_with_key__with_callobjectcache___seq_rremove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_rremove__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* seq_removeall */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_removeall__with_callobjectcache___seq_removeall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_removeall__, self, "o" PCKuSIZ PCKuSIZ PCKuSIZ, item, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

/* seq_removeall_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 7)) size_t DCALL
tdefault__seq_removeall_with_key__with_callobjectcache___seq_removeall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_removeall__, self, "o" PCKuSIZ PCKuSIZ PCKuSIZ "o", item, start, end, max, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

/* seq_removeif */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_removeif__with_callobjectcache___seq_removeif__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_removeif__, self, "o" PCKuSIZ PCKuSIZ PCKuSIZ, should, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

/* seq_resize */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__seq_resize__with_callobjectcache___seq_resize__(DeeTypeObject *tp_self, DeeObject *self, size_t newsize, DeeObject *filler) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_resize__, self, PCKuSIZ "o", newsize, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

/* seq_fill */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
tdefault__seq_fill__with_callobjectcache___seq_fill__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_fill__, self, PCKuSIZ PCKuSIZ "o", start, end, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

/* seq_reverse */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_reverse__with_callobjectcache___seq_reverse__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_reverse__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

/* seq_reversed */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_reversed__with_callobjectcache___seq_reversed__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_reversed__, self, PCKuSIZ PCKuSIZ, start, end);
}

/* seq_sort */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__seq_sort__with_callobjectcache___seq_sort__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_sort__, self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

/* seq_sort_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
tdefault__seq_sort_with_key__with_callobjectcache___seq_sort__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_sort__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}

/* seq_sorted */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_sorted__with_callobjectcache___seq_sorted__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_sorted__, self, PCKuSIZ PCKuSIZ, start, end);
}

/* seq_sorted_with_key */
INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
tdefault__seq_sorted_with_key__with_callobjectcache___seq_sorted__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_sorted__, self, PCKuSIZ PCKuSIZ "o", start, end, key);
}

/* seq_bfind */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_bfind__with_callobjectcache___seq_bfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_bfind__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

/* seq_bfind_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL
tdefault__seq_bfind_with_key__with_callobjectcache___seq_bfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_bfind__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

/* seq_bposition */
INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
tdefault__seq_bposition__with_callobjectcache___seq_bposition__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end) {
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_bposition__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

/* seq_bposition_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL
tdefault__seq_bposition_with_key__with_callobjectcache___seq_bposition__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_bposition__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}

/* seq_brange */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__seq_brange__with_callobjectcache___seq_brange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]) {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_brange__, self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}

/* seq_brange_with_key */
INTERN WUNUSED NONNULL((1, 2, 3, 6, 7)) int DCALL
tdefault__seq_brange_with_key__with_callobjectcache___seq_brange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]) {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = mhcache_thiscallf(tp_self, tp_self->tp_mhcache->mhc___seq_brange__, self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}

/* set_operator_iter */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__set_operator_iter__with_callobjectcache___set_iter__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___set_iter__, 1, (DeeObject *const *)&self);
}

/* set_operator_sizeob */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__set_operator_sizeob__with_callobjectcache___set_size__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___set_size__, 1, (DeeObject *const *)&self);
}

/* set_operator_hash */
#ifndef DEFINED_set_handle_hash_error
#define DEFINED_set_handle_hash_error
PRIVATE NONNULL((1)) Dee_hash_t DCALL
set_handle_hash_error(DeeObject *self) {
	DeeError_Print("Unhandled error in `Set.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}
#endif /* !DEFINED_set_handle_hash_error */
INTERN WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL
tdefault__set_operator_hash__with_callobjectcache___set_hash__(DeeTypeObject *tp_self, DeeObject *self) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = mhcache_call(tp_self, tp_self->tp_mhcache->mhc___set_hash__, 1, (DeeObject *const *)&self);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return set_handle_hash_error(self);
}

/* set_operator_compare_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_operator_compare_eq__with_callobjectcache___set_compare_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_compare_eq__, lhs, 1, &rhs);
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

/* set_operator_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_eq__with_callobjectcache___set_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_eq__, lhs, 1, &rhs);
}

/* set_operator_ne */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_ne__with_callobjectcache___set_ne__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_ne__, lhs, 1, &rhs);
}

/* set_operator_lo */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_lo__with_callobjectcache___set_lo__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_lo__, lhs, 1, &rhs);
}

/* set_operator_le */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_le__with_callobjectcache___set_le__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_le__, lhs, 1, &rhs);
}

/* set_operator_gr */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_gr__with_callobjectcache___set_gr__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_gr__, lhs, 1, &rhs);
}

/* set_operator_ge */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_ge__with_callobjectcache___set_ge__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_ge__, lhs, 1, &rhs);
}

/* set_operator_inv */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__set_operator_inv__with_callobjectcache___set_size__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___set_size__, 1, (DeeObject *const *)&self);
}

/* set_operator_add */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_add__with_callobjectcache___set_add__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_add__, lhs, 1, &rhs);
}

/* set_operator_sub */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_sub__with_callobjectcache___set_sub__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_sub__, lhs, 1, &rhs);
}

/* set_operator_and */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_and__with_callobjectcache___set_and__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_and__, lhs, 1, &rhs);
}

/* set_operator_xor */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_operator_xor__with_callobjectcache___set_xor__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_xor__, lhs, 1, &rhs);
}

/* set_operator_inplace_add */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_operator_inplace_add__with_callobjectcache___set_inplace_add__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_inplace_add__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* set_operator_inplace_sub */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_operator_inplace_sub__with_callobjectcache___set_inplace_sub__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_inplace_sub__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* set_operator_inplace_and */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_operator_inplace_and__with_callobjectcache___set_inplace_and__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_inplace_and__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* set_operator_inplace_xor */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_operator_inplace_xor__with_callobjectcache___set_inplace_xor__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_inplace_xor__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* set_frozen */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__set_frozen__with_callobjectcache___set_frozen__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___set_frozen__, 1, (DeeObject *const *)&self);
}

/* set_unify */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_unify__with_callobjectcache___set_unify__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_unify__, self, 1, &key);
}

/* set_insert */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_insert__with_callobjectcache___set_insert__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_insert__, self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* set_insertall */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_insertall__with_callobjectcache___set_insertall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_insertall__, self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

/* set_remove */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_remove__with_callobjectcache___set_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_remove__, self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* set_removeall */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__set_removeall__with_callobjectcache___set_removeall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_removeall__, self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

/* set_pop */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__set_pop__with_callobjectcache___set_pop__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___set_pop__, 1, (DeeObject *const *)&self);
}

/* set_pop_with_default */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__set_pop_with_default__with_callobjectcache___set_pop__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *default_) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___set_pop__, self, 1, &default_);
}

/* map_operator_iter */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_operator_iter__with_callobjectcache___map_iter__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___map_iter__, 1, (DeeObject *const *)&self);
}

/* map_operator_sizeob */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_operator_sizeob__with_callobjectcache___map_size__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___map_size__, 1, (DeeObject *const *)&self);
}

/* map_operator_getitem */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_getitem__with_callobjectcache___map_getitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_getitem__, self, 1, &key);
}

/* map_operator_delitem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_operator_delitem__with_callobjectcache___map_delitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_delitem__, self, 1, &key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

/* map_operator_setitem */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__map_operator_setitem__with_callobjectcache___map_setitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_setitem__, self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}

/* map_operator_contains */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_contains__with_callobjectcache___map_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_contains__, self, 1, &key);
}

/* map_keys */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_keys__with_callobjectcache___map_keys__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___map_keys__, 1, (DeeObject *const *)&self);
}

/* map_iterkeys */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_iterkeys__with_callobjectcache___map_iterkeys__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___map_iterkeys__, 1, (DeeObject *const *)&self);
}

/* map_values */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_values__with_callobjectcache___map_values__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___map_values__, 1, (DeeObject *const *)&self);
}

/* map_itervalues */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_itervalues__with_callobjectcache___map_itervalues__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___map_itervalues__, 1, (DeeObject *const *)&self);
}

/* map_enumerate */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__map_enumerate__with_callobjectcache___map_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t cb, void *arg) {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_enumerate__, self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

/* map_enumerate_range */
INTERN WUNUSED NONNULL((1, 2, 3, 5, 6)) Dee_ssize_t DCALL
tdefault__map_enumerate_range__with_callobjectcache___map_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	DeeObject *args[3];
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	args[0] = (DeeObject *)wrapper;
	args[1] = start;
	args[2] = end;
	result  = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_enumerate__, self, 3, args);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}

/* map_makeenumeration */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_makeenumeration__with_callobjectcache___map_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___map_enumerate_items__, 1, (DeeObject *const *)&self);
}

/* map_makeenumeration_with_range */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__map_makeenumeration_with_range__with_callobjectcache___map_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_enumerate_items__, self, 2, args);
}

/* map_operator_compare_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_operator_compare_eq__with_callobjectcache___map_compare_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *resultob;
	resultob = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_compare_eq__, lhs, 1, &rhs);
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

/* map_operator_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_eq__with_callobjectcache___map_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_eq__, lhs, 1, &rhs);
}

/* map_operator_ne */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_ne__with_callobjectcache___map_ne__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_ne__, lhs, 1, &rhs);
}

/* map_operator_lo */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_lo__with_callobjectcache___map_lo__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_lo__, lhs, 1, &rhs);
}

/* map_operator_le */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_le__with_callobjectcache___map_le__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_le__, lhs, 1, &rhs);
}

/* map_operator_gr */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_gr__with_callobjectcache___map_gr__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_gr__, lhs, 1, &rhs);
}

/* map_operator_ge */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_ge__with_callobjectcache___map_ge__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_ge__, lhs, 1, &rhs);
}

/* map_operator_add */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_add__with_callobjectcache___map_add__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_add__, lhs, 1, &rhs);
}

/* map_operator_sub */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_sub__with_callobjectcache___map_sub__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *keys) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_sub__, lhs, 1, &keys);
}

/* map_operator_and */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_and__with_callobjectcache___map_and__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *keys) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_and__, lhs, 1, &keys);
}

/* map_operator_xor */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_operator_xor__with_callobjectcache___map_xor__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_xor__, lhs, 1, &rhs);
}

/* map_operator_inplace_add */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_operator_inplace_add__with_callobjectcache___map_inplace_add__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *items) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_inplace_add__, *p_self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* map_operator_inplace_sub */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_operator_inplace_sub__with_callobjectcache___map_inplace_sub__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *keys) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_inplace_sub__, *p_self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* map_operator_inplace_and */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_operator_inplace_and__with_callobjectcache___map_inplace_and__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *keys) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_inplace_and__, *p_self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* map_operator_inplace_xor */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_operator_inplace_xor__with_callobjectcache___map_inplace_xor__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_inplace_xor__, *p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* map_frozen */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_frozen__with_callobjectcache___map_frozen__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc_get___map_frozen__, 1, (DeeObject *const *)&self);
}

/* map_setold */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__map_setold__with_callobjectcache___map_setold__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_setold__, self, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* map_setold_ex */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__map_setold_ex__with_callobjectcache___map_setold_ex__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_setold_ex__, self, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

/* map_setnew */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__map_setnew__with_callobjectcache___map_setnew__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_setnew__, self, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* map_setnew_ex */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__map_setnew_ex__with_callobjectcache___map_setnew_ex__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_setnew_ex__, self, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

/* map_setdefault */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__map_setdefault__with_callobjectcache___map_setdefault__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_setdefault__, self, 2, args);
}

/* map_update */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_update__with_callobjectcache___map_update__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *items) {
	DREF DeeObject *result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_update__, self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

/* map_remove */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_remove__with_callobjectcache___map_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	DREF DeeObject *result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_remove__, self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

/* map_removekeys */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__map_removekeys__with_callobjectcache___map_removekeys__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_removekeys__, self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

/* map_pop */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__map_pop__with_callobjectcache___map_pop__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key) {
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_pop__, self, 1, &key);
}

/* map_pop_with_default */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__map_pop_with_default__with_callobjectcache___map_pop__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *default_) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = default_;
	return mhcache_thiscall(tp_self, tp_self->tp_mhcache->mhc___map_pop__, self, 2, args);
}

/* map_popitem */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__map_popitem__with_callobjectcache___map_popitem__(DeeTypeObject *tp_self, DeeObject *self) {
	return mhcache_call(tp_self, tp_self->tp_mhcache->mhc___map_popitem__, 1, (DeeObject *const *)&self);
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_C */
