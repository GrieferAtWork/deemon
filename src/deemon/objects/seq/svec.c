/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SVEC_C
#define GUARD_DEEMON_OBJECTS_SEQ_SVEC_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FreeTracker, DeeObject_MALLOC, Dee_CollectMemoryc, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TryMallocc */
#include <deemon/bool.h>               /* Dee_True, return_bool, return_false, return_true */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_Throwf, DeeError_ValueError */
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint, Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>               /* DeeNone_NewRef */
#include <deemon/object.h>             /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_FROMBOOL, Dee_BOUND_MISSING, Dee_COMPARE_*, Dee_Decref*, Dee_Incref, Dee_Movrefv, Dee_XDecref, Dee_XDecref_unlikely, Dee_XIncref, Dee_foreach_t, Dee_ssize_t, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_Type, Dee_seq_range */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/system-features.h>    /* memcpyc */
#include <deemon/type.h>               /* DeeObject_Init, DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visitv, Dee_visit_t, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_read */
#include <deemon/util/lock.h>          /* Dee_atomic_read_with_atomic_rwlock, Dee_atomic_rwlock_init, Dee_atomic_rwlock_t */

#include <hybrid/limitcore.h> /* __SSIZE_MAX__ */

#include "../../runtime/method-hint-defaults.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "svec.h"

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

#ifndef CONFIG_TINY_DEEMON
#define WANT_rvec_contains
#define WANT_rvec_getitem_index
#define WANT_rvec_bounditem_index
#define WANT_rvec_foreach
#define WANT_rvec_mh_seq_enumerate_index
#define WANT_rvec_mh_seq_enumerate_index_reverse
#define WANT_svec_getitem_index
#define WANT_svec_foreach
#define WANT_svec_mh_seq_enumerate_index_reverse
#define WANT_svec_mh_find
#define WANT_svec_mh_rfind
#endif /* !CONFIG_TINY_DEEMON */

DECL_BEGIN

STATIC_ASSERT(offsetof(RefVector, rv_owner) == offsetof(ProxyObject, po_obj));
#define rvec_fini  generic_proxy__fini
#define rvec_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_bool(RefVector *__restrict self) {
	return self->rv_length != 0;
}

#ifdef WANT_rvec_contains
#define PTR_rvec_contains &rvec_contains
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rvec_contains(RefVector *self, DeeObject *other) {
	size_t index;
	int temp;
	for (index = 0; index < self->rv_length; ++index) {
		DREF DeeObject *item;
		RefVector_XLockRead(self);
		item = self->rv_vector[index];
		Dee_XIncref(item);
		RefVector_XLockEndRead(self);
		if (!item)
			continue;
		temp = DeeObject_TryCompareEq(other, item);
		Dee_Decref(item);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp))
			return_true;
	}
	return_false;
err:
	return NULL;
}
#else /* WANT_rvec_contains */
#define PTR_rvec_contains DEFIMPL(&default__seq_operator_contains__with__seq_contains)
#endif /* !WANT_rvec_contains */

PRIVATE ATTR_COLD int DCALL err_readonly_rvec(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Reference vector is not writable");
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rvec_size(RefVector *__restrict self) {
	ASSERT(self->rv_length != (size_t)-1);
	return self->rv_length;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_getitem_index_fast(RefVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->rv_length);
	RefVector_XLockRead(self);
	result = self->rv_vector[index];
	Dee_XIncref(result);
	RefVector_XLockEndRead(self);
	return result;
}

#ifdef WANT_rvec_getitem_index
#define PTR_rvec_getitem_index &rvec_getitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_getitem_index(RefVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->rv_length) {
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rv_length);
		return NULL;
	}
	RefVector_XLockRead(self);
	result = self->rv_vector[index];
	if unlikely(!result) {
		RefVector_XLockEndRead(self);
		DeeRT_ErrUnboundIndex(self, index);
		return NULL;
	}
	Dee_Incref(result);
	RefVector_XLockEndRead(self);
	return result;
}
#else /* WANT_rvec_getitem_index */
#define PTR_rvec_getitem_index DEFIMPL(&default__getitem_index__with__size__and__getitem_index_fast)
#endif /* !WANT_rvec_getitem_index */

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_delitem_index(RefVector *__restrict self, size_t index) {
	DREF DeeObject *oldobj;
	if unlikely(index >= self->rv_length)
		return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rv_length);
	if unlikely(!RefVector_IsWritable(self))
		return err_readonly_rvec();
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = NULL;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
rvec_setitem_index(RefVector *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldobj;
	if unlikely(index >= self->rv_length)
		return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rv_length);
	if unlikely(!RefVector_IsWritable(self))
		return err_readonly_rvec();
	Dee_Incref(value);
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = value;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
	return 0;
}

#ifdef WANT_rvec_bounditem_index
#define PTR_rvec_bounditem_index &rvec_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_bounditem_index(RefVector *self, size_t index) {
	DeeObject *value;
	if unlikely(index >= self->rv_length)
		return Dee_BOUND_MISSING;
	value = RefVector_IsWritable(self)
	        ? Dee_atomic_read_with_atomic_rwlock(&self->rv_vector[index], self->rv_plock)
	        : self->rv_vector[index];
	return Dee_BOUND_FROMBOOL(value != NULL);
}
#else /* WANT_rvec_bounditem_index */
#define PTR_rvec_bounditem_index DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast)
#endif /* !WANT_rvec_bounditem_index */

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
rvec_xchitem_index(RefVector *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	if unlikely(index >= self->rv_length) {
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rv_length);
		goto err;
	}
	if unlikely(!RefVector_IsWritable(self)) {
		err_readonly_rvec();
		goto err;
	}
	RefVector_LockWrite(self);
	result = self->rv_vector[index];
	if unlikely(!result) {
		RefVector_LockEndWrite(self);
		DeeRT_ErrUnboundIndex(self, index);
		goto err;
	}
	Dee_Incref(value);
	self->rv_vector[index] = value;
	RefVector_LockEndWrite(self);
	return result;
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
rvec_delitem_index_fast(RefVector *__restrict self, size_t index) {
	DREF DeeObject *oldobj;
	ASSERT(index < self->rv_length);
	ASSERT(RefVector_IsWritable(self));
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = NULL;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_delrange_index(RefVector *__restrict self,
                    Dee_ssize_t i_begin, Dee_ssize_t i_end) {
	struct Dee_seq_range range;
	size_t i;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->rv_length);
	for (i = range.sr_start; i < range.sr_end; ++i)
		rvec_delitem_index_fast(self, i);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_delrange_index_n(RefVector *__restrict self, Dee_ssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return rvec_delrange_index(self, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	size_t i, start;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	start = DeeSeqRange_Clamp_n(i_begin, self->rv_length);
	for (i = start; i < self->rv_length; ++i)
		rvec_delitem_index_fast(self, i);
	return 0;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE NONNULL((1)) void DCALL
rvec_setitem_index_fast(RefVector *self, size_t index,
                        /*inherit(always)*/ DREF DeeObject *value) {
	DREF DeeObject *oldobj;
	ASSERT(index < self->rv_length);
	ASSERT(RefVector_IsWritable(self));
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = value;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
}

struct rvec_setrange_enumerate_data {
	RefVector           *rvsre_self;  /* [1..1] Output ref-vector */
	struct Dee_seq_range rvsre_range; /* Range of indicates of "self" to assign */
	size_t               rvsre_rsize; /* [== rvsre_range.sr_end - rvsre_range.sr_start] */
	size_t               rvsre_mxidx; /* Last encountered index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
rvec_setrange_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	struct rvec_setrange_enumerate_data *data;
	data = (struct rvec_setrange_enumerate_data *)arg;
	data->rvsre_mxidx = index;
	if unlikely(index < data->rvsre_rsize) {
		Dee_XIncref(item); /* Inherited by `rvec_setitem_index_fast()' */
		rvec_setitem_index_fast(data->rvsre_self,
		                        data->rvsre_range.sr_start + index,
		                        item);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
rvec_setrange_index(RefVector *self, Dee_ssize_t start,
                    Dee_ssize_t end, DeeObject *values) {
	size_t fsize;
	struct rvec_setrange_enumerate_data data;
	if unlikely(!RefVector_IsWritable(self))
		return err_readonly_rvec();
	DeeSeqRange_Clamp(&data.rvsre_range, start, end, self->rv_length);
	data.rvsre_rsize = data.rvsre_range.sr_end - data.rvsre_range.sr_start;
	fsize = DeeObject_SizeFast(values);
	if unlikely(fsize != (size_t)-1 && fsize != data.rvsre_rsize)
		return DeeRT_ErrUnpackError(values, data.rvsre_rsize, fsize);
	data.rvsre_self  = self;
	data.rvsre_mxidx = (size_t)-1;
	if unlikely(DeeObject_InvokeMethodHint(seq_enumerate_index, values,
	                                       &rvec_setrange_enumerate_cb,
	                                       &data, 0, fsize))
		goto err;
	++data.rvsre_mxidx;
	if unlikely(data.rvsre_mxidx != data.rvsre_rsize)
		return DeeRT_ErrUnpackError(values, data.rvsre_rsize, data.rvsre_mxidx);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
rvec_setrange_index_n(RefVector *self, Dee_ssize_t start, DeeObject *values) {
	return rvec_setrange_index(self, start, SSIZE_MAX, values);
}

#ifdef WANT_rvec_foreach
#define PTR_rvec_foreach &rvec_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rvec_foreach(RefVector *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i < self->rv_length; ++i) {
		DREF DeeObject *elem;
		RefVector_XLockRead(self);
		while ((elem = self->rv_vector[i]) == NULL) {
			++i;
			if (i >= self->rv_length) {
				RefVector_XLockEndRead(self);
				goto done;
			}
		}
		Dee_Incref(elem);
		RefVector_XLockEndRead(self);
		temp = (*proc)(arg, elem);
		Dee_Decref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
done:
	return result;
err_temp:
	return temp;
}
#else /* WANT_rvec_foreach */
#define PTR_rvec_foreach DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
#endif /* !WANT_rvec_foreach */

#ifdef WANT_rvec_mh_seq_enumerate_index
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rvec_mh_seq_enumerate_index(RefVector *self, Dee_seq_enumerate_index_t proc,
                            void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	if (end > self->rv_length)
		end = self->rv_length;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		RefVector_XLockRead(self);
		elem = self->rv_vector[i];
		Dee_XIncref(elem);
		RefVector_XLockEndRead(self);
		temp = (*proc)(arg, i, elem);
		Dee_XDecref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}
#endif /* WANT_rvec_mh_seq_enumerate_index */

#ifdef WANT_rvec_mh_seq_enumerate_index_reverse
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rvec_mh_seq_enumerate_index_reverse(RefVector *self, Dee_seq_enumerate_index_t proc,
                                    void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (end > self->rv_length)
		end = self->rv_length;
	while (end > start) {
		DREF DeeObject *elem;
		RefVector_XLockRead(self);
		elem = self->rv_vector[--end];
		Dee_XIncref(elem);
		RefVector_XLockEndRead(self);
		temp = (*proc)(arg, end, elem);
		Dee_XDecref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}
#endif /* WANT_rvec_mh_seq_enumerate_index_reverse */

PRIVATE struct type_method tpconst rvec_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst rvec_method_hints[] = {
	TYPE_METHOD_HINT(seq_xchitem_index, &rvec_xchitem_index),
#ifdef WANT_rvec_mh_seq_enumerate_index
	TYPE_METHOD_HINT(seq_enumerate_index, &rvec_mh_seq_enumerate_index),
#endif /* WANT_rvec_mh_seq_enumerate_index */
#ifdef WANT_rvec_mh_seq_enumerate_index_reverse
	TYPE_METHOD_HINT(seq_enumerate_index_reverse, &rvec_mh_seq_enumerate_index_reverse),
#endif /* WANT_rvec_mh_seq_enumerate_index_reverse */
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq rvec_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))PTR_rvec_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem                    = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__delrange__with__delrange_index__and__delrange_index_n),
	/* .tp_setrange                   = */ DEFIMPL(&default__setrange__with__setrange_index__and__setrange_index_n),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))PTR_rvec_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rvec_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rvec_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))PTR_rvec_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rvec_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rvec_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&rvec_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))PTR_rvec_bounditem_index,
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rvec_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&rvec_setrange_index,
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&rvec_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&rvec_setrange_index_n,
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__size__and__getitem_index_fast),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst rvec_members[] = {
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT_AB, offsetof(RefVector, rv_owner)),
	TYPE_MEMBER_FIELD("__len__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RefVector, rv_length)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_get_writable(RefVector *__restrict self) {
	return_bool(RefVector_IsWritable(self));
}

PRIVATE struct type_getset tpconst rvec_getsets[] = {
	TYPE_GETTER_F("__writable__", &rvec_get_writable, METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_init(RefVector *__restrict self) {
	self->rv_owner  = DeeNone_NewRef();
	self->rv_length = 0;
	self->rv_vector = NULL;
#ifndef CONFIG_NO_THREADS
	self->rv_plock = NULL;
#else /* !CONFIG_NO_THREADS */
	self->rv_writable = false;
#endif /* CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rvec_copy(RefVector *__restrict self,
          RefVector *__restrict other) {
	self->rv_owner = other->rv_owner;
	Dee_Incref(self->rv_owner);
	self->rv_length = other->rv_length;
	self->rv_vector = other->rv_vector;
#ifndef CONFIG_NO_THREADS
	self->rv_plock = other->rv_plock;
#else /* !CONFIG_NO_THREADS */
	self->rv_writable = other->rv_writable;
#endif /* CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rvec_serialize(RefVector *__restrict self,
               DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	RefVector *out;
#define ADDROF(field) (addr + offsetof(RefVector, field))
	if (DeeSerial_PutObject(writer, ADDROF(rv_owner), self->rv_owner))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, RefVector);
	out->rv_length = self->rv_length;
#ifdef CONFIG_NO_THREADS
	out->rv_writable = self->rv_writable;
#else /* CONFIG_NO_THREADS */
	if (DeeSerial_PutPointer(writer, ADDROF(rv_plock), self->rv_plock))
		goto err;
#endif /* !CONFIG_NO_THREADS */
	return DeeSerial_PutPointer(writer, ADDROF(rv_vector), self->rv_vector);
err:
	return -1;
#undef ADDROF
}

INTERN DeeTypeObject RefVector_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RefVector",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ RefVector,
			/* tp_ctor:        */ &rvec_init,
			/* tp_copy_ctor:   */ &rvec_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rvec_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rvec_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rvec_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rvec_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__22D95991F3D69B20),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__1CA4BB8FD7207D06),
	/* .tp_seq           = */ &rvec_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rvec_methods,
	/* .tp_getsets       = */ rvec_getsets,
	/* .tp_members       = */ rvec_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ rvec_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

/* Construct a new reference-vector object that can be iterated
 * and used to potentially modify the elements of a given `vector'.
 * NOTE: When write-access is granted, `vector' should be `[0..1][0..length]',
 *       whereas when write-access is not possible, then the disposition of
 *       elements of `vector' doesn't matter and can either be `[0..1]' or `[1..1]'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRefVector_New(DeeObject *owner, size_t length,
                 DeeObject **vector,
#ifndef CONFIG_NO_THREADS
                 Dee_atomic_rwlock_t *plock
#else /* !CONFIG_NO_THREADS */
                 bool writable
#endif /* CONFIG_NO_THREADS */
                 ) {
	DREF RefVector *result;
	ASSERT_OBJECT(owner);
	ASSERT(!length || vector);
	result = DeeObject_MALLOC(RefVector);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &RefVector_Type);
	Dee_Incref(owner); /* Create the reference for `rv_owner' */
	result->rv_length = length;
	result->rv_vector = vector;
	result->rv_owner  = owner;
#ifndef CONFIG_NO_THREADS
	result->rv_plock = plock;
#else /* !CONFIG_NO_THREADS */
	result->rv_writable = writable;
#endif /* CONFIG_NO_THREADS */
done:
	return Dee_AsObject(result);
}




PRIVATE NONNULL((1)) void DCALL
svec_fini(SharedVector *__restrict self) {
	Dee_Decrefv(self->sv_vector, self->sv_length);
	Dee_Free((void *)self->sv_vector);
}

PRIVATE NONNULL((1, 2)) void DCALL
svec_visit(SharedVector *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visitv(self->sv_vector, self->sv_length);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
svec_size(SharedVector *__restrict self) {
	ASSERT(self->sv_length != (size_t)-1);
	return atomic_read(&self->sv_length);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
svec_getitem_index_fast(SharedVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	SharedVector_LockRead(self);
	/* Still to check for length==0 in case the vector got unshared. */
	if unlikely(!self->sv_length) {
		SharedVector_LockEndRead(self);
		return NULL;
	}
	ASSERT(index < self->sv_length);
	result = self->sv_vector[index];
	Dee_Incref(result);
	SharedVector_LockEndRead(self);
	return result;
}

#ifdef WANT_svec_getitem_index
#define PTR_svec_getitem_index &svec_getitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
svec_getitem_index(SharedVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	SharedVector_LockRead(self);
	if unlikely(index >= self->sv_length) {
		size_t my_length = self->sv_length;
		SharedVector_LockEndRead(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, my_length);
		return NULL;
	}
	result = self->sv_vector[index];
	Dee_Incref(result);
	SharedVector_LockEndRead(self);
	return result;
}
#else /* WANT_svec_getitem_index */
#define PTR_svec_getitem_index DEFIMPL(&default__getitem_index__with__size__and__getitem_index_fast)
#endif /* !WANT_svec_getitem_index */

#ifdef WANT_svec_foreach
#define PTR_svec_foreach &svec_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
svec_foreach(SharedVector *self, Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	SharedVector_LockRead(self);
	for (i = 0; i < self->sv_length; ++i) {
		DREF DeeObject *list_elem;
		list_elem = self->sv_vector[i];
		Dee_Incref(list_elem);
		SharedVector_LockEndRead(self);
		temp = (*proc)(arg, list_elem);
		Dee_Decref_unlikely(list_elem);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
	return result;
err:
	return temp;
}
#else /* WANT_svec_foreach */
#define PTR_svec_foreach DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
#endif /* !WANT_svec_foreach */


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
svec_mh_seq_enumerate_index(SharedVector *self, Dee_seq_enumerate_index_t proc,
                            void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = start; i < end; ++i) {
		DREF DeeObject *list_elem;
		SharedVector_LockRead(self);
		if (i >= self->sv_length) {
			SharedVector_LockEndRead(self);
			break;
		}
		list_elem = self->sv_vector[i];
		Dee_Incref(list_elem);
		SharedVector_LockEndRead(self);
		temp = (*proc)(arg, i, list_elem);
		Dee_Decref_unlikely(list_elem);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

#ifdef WANT_svec_mh_seq_enumerate_index_reverse
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
svec_mh_seq_enumerate_index_reverse(SharedVector *self, Dee_seq_enumerate_index_t proc,
                                    void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	while (end > start) {
		DREF DeeObject *list_elem;
		SharedVector_LockRead(self);
		if (end > self->sv_length) {
			end = self->sv_length;
			if (end <= start) {
				SharedVector_LockEndRead(self);
				break;
			}
		}
		list_elem = self->sv_vector[--end];
		Dee_Incref(list_elem);
		SharedVector_LockEndRead(self);
		temp = (*proc)(arg, end, list_elem);
		Dee_Decref_unlikely(list_elem);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}
#endif /* WANT_svec_mh_seq_enumerate_index_reverse */

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
svec_asvector_nothrow(SharedVector *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t realsize;
	SharedVector_LockRead(self);
	realsize = self->sv_length;
	if likely(dst_length >= realsize)
		Dee_Movrefv(dst, self->sv_vector, realsize);
	SharedVector_LockEndRead(self);
	return realsize;
}


#ifdef WANT_svec_mh_find
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
svec_mh_find(SharedVector *self, DeeObject *item, size_t start, size_t end) {
	size_t i = start;
	SharedVector_LockRead(self);
	for (; i < end && i < self->sv_length; ++i) {
		DREF DeeObject *myitem;
		int temp;
		myitem = self->sv_vector[i];
		Dee_Incref(myitem);
		SharedVector_LockEndRead(self);
		temp = DeeObject_TryCompareEq(item, myitem);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp))
			return i;
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
svec_mh_find_with_key(SharedVector *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t i = start;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	SharedVector_LockRead(self);
	for (; i < end && i < self->sv_length; ++i) {
		DREF DeeObject *myitem;
		int temp;
		myitem = self->sv_vector[i];
		Dee_Incref(myitem);
		SharedVector_LockEndRead(self);
		temp = DeeObject_TryCompareKeyEq(item, myitem, key);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp)) {
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
			Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
			return i;
		}
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)-1;
err_item:
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
err:
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)Dee_COMPARE_ERR;
}
#endif /* WANT_svec_mh_find */


#ifdef WANT_svec_mh_rfind
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
svec_mh_rfind(SharedVector *self, DeeObject *item, size_t start, size_t end) {
	size_t i = end;
	SharedVector_LockRead(self);
	for (;;) {
		DREF DeeObject *myitem;
		int temp;
		if (i > self->sv_length)
			i = self->sv_length;
		if (i <= start)
			break;
		--i;
		myitem = self->sv_vector[i];
		Dee_Incref(myitem);
		SharedVector_LockEndRead(self);
		temp = DeeObject_TryCompareEq(item, myitem);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp))
			return i;
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
svec_mh_rfind_with_key(SharedVector *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t i = end;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	SharedVector_LockRead(self);
	for (;;) {
		DREF DeeObject *myitem;
		int temp;
		if (i > self->sv_length)
			i = self->sv_length;
		if (i <= start)
			break;
		--i;
		myitem = self->sv_vector[i];
		Dee_Incref(myitem);
		SharedVector_LockEndRead(self);
		temp = DeeObject_TryCompareKeyEq(item, myitem, key);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp)) {
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
			Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
			return i;
		}
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)-1;
err_item:
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
err:
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)Dee_COMPARE_ERR;
}
#endif /* WANT_svec_mh_rfind */


PRIVATE struct type_method tpconst svec_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
#ifdef WANT_svec_mh_find
	TYPE_METHOD_HINTREF(Sequence_find),
#endif /* WANT_svec_mh_find */
#ifdef WANT_svec_mh_rfind
	TYPE_METHOD_HINTREF(Sequence_rfind),
#endif /* WANT_svec_mh_rfind */
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst svec_method_hints[] = {
	TYPE_METHOD_HINT(seq_enumerate_index, &svec_mh_seq_enumerate_index),
#ifdef WANT_svec_mh_seq_enumerate_index_reverse
	TYPE_METHOD_HINT(seq_enumerate_index_reverse, &svec_mh_seq_enumerate_index_reverse),
#endif /* WANT_svec_mh_seq_enumerate_index_reverse */
#ifdef WANT_svec_mh_find
	TYPE_METHOD_HINT(seq_find, &svec_mh_find),
	TYPE_METHOD_HINT(seq_find_with_key, &svec_mh_find_with_key),
#endif /* WANT_svec_mh_find */
#ifdef WANT_svec_mh_rfind
	TYPE_METHOD_HINT(seq_rfind, &svec_mh_rfind),
	TYPE_METHOD_HINT(seq_rfind_with_key, &svec_mh_rfind_with_key),
#endif /* WANT_svec_mh_rfind */
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq svec_seq = {
	/* .tp_iter                       = */ &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))PTR_svec_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&svec_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&svec_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))PTR_svec_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&svec_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__size__and__getitem_index_fast),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&svec_asvector_nothrow,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&svec_asvector_nothrow,
};

PRIVATE struct type_getset tpconst svec_getsets[] = {
	TYPE_GETTER_AB(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst svec_class_members[] = {
	TYPE_MEMBER_CONST(STR_Frozen, &DeeSharedVector_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
svec_ctor(SharedVector *__restrict self) {
	Dee_atomic_rwlock_init(&self->sv_lock);
	self->sv_length = 0;
	self->sv_vector = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
svec_copy(SharedVector *__restrict self,
          SharedVector *__restrict other) {
again:
	SharedVector_LockRead(other);
	self->sv_length = other->sv_length;
	self->sv_vector = (DREF DeeObject **)Dee_TryMallocc(self->sv_length,
	                                                    sizeof(DREF DeeObject *));
	if unlikely(!self->sv_vector) {
		SharedVector_LockEndRead(other);
		if (Dee_CollectMemoryc(self->sv_length, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	Dee_Movrefv((DREF DeeObject **)self->sv_vector,
	            other->sv_vector, self->sv_length);
	SharedVector_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->sv_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
svec_serialize(SharedVector *__restrict self,
               DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(SharedVector, field))
	SharedVector *out;
	size_t self__sv_length;
	Dee_seraddr_t out__sv_vector;
	DREF DeeObject **ou__sv_vector;
again:
	SharedVector_LockRead(self);
	self__sv_length = self->sv_length;
	out__sv_vector = DeeSerial_TryMalloc(writer, self__sv_length * sizeof(DREF DeeObject *), NULL);
	if unlikely(!Dee_SERADDR_ISOK(out__sv_vector)) {
		SharedVector_LockEndRead(self);
		out__sv_vector = DeeSerial_Malloc(writer, self__sv_length * sizeof(DREF DeeObject *), NULL);
		if unlikely(!Dee_SERADDR_ISOK(out__sv_vector))
			goto err;
		SharedVector_LockRead(self);
		if unlikely(self__sv_length != self->sv_length) {
			SharedVector_LockEndRead(self);
			DeeSerial_Free(writer, out__sv_vector, NULL);
			goto again;
		}
	}
	ou__sv_vector = DeeSerial_Addr2Mem(writer, out__sv_vector, DREF DeeObject *);
	Dee_Movrefv(ou__sv_vector, self->sv_vector, self__sv_length);
	SharedVector_LockEndRead(self);
	if (DeeSerial_InplacePutObjectv(writer, out__sv_vector, self__sv_length))
		goto err;
	if (DeeSerial_PutAddr(writer, ADDROF(sv_vector), out__sv_vector))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, SharedVector);
	out->sv_length = self__sv_length;
	Dee_atomic_rwlock_init(&out->sv_lock);
	return 0;
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
svec_bool(SharedVector *__restrict self) {
	return atomic_read(&self->sv_length) != 0;
}

PUBLIC DeeTypeObject DeeSharedVector_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SharedVector",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SharedVector,
			/* tp_ctor:        */ &svec_ctor,
			/* tp_copy_ctor:   */ &svec_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &svec_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&svec_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&svec_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&svec_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__1CA4BB8FD7207D06),
	/* .tp_seq           = */ &svec_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ svec_methods,
	/* .tp_getsets       = */ svec_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ svec_class_members,
	/* .tp_method_hints  = */ svec_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedVector_Decref()' is called.
 * NOTE: This function can implicitly inherit a reference to each item of the
 *       given vector, though does not actually inherit the vector itself:
 *       - DeeSharedVector_Decref:            The `vector' arg here is `DREF DeeObject *const *'
 *       - DeeSharedVector_DecrefNoGiftItems: The `vector' arg here is `DeeObject *const *'
 * NOTE: The returned object cannot be used to change out the elements
 *       of the given `vector', meaning that _it_ can still be [const] */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSharedVector_NewShared(size_t length, DREF DeeObject *const *vector) {
	DREF SharedVector *result;
	result = SharedVector_Malloc();
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeSharedVector_Type);
	Dee_atomic_rwlock_init(&result->sv_lock);
	result->sv_length = length;
	result->sv_vector = vector;
done:
	return Dee_AsObject(result);
}

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `sv_vector',
 * but still decref() all contained objects.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_SEQ' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
PUBLIC NONNULL((1)) void DCALL
DeeSharedVector_Decref(DREF DeeObject *__restrict self) {
	size_t length;
	DREF DeeObject *const *vector;
	DREF DeeObject **vector_copy;
	SharedVector *me = (SharedVector *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSharedVector_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		Dee_Decrefv(me->sv_vector, me->sv_length);
		Dee_DecrefNokill(&DeeSharedVector_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		SharedVector_Free(me);
		return;
	}

	/* Difficult case: must duplicate the vector. */
	SharedVector_LockWrite(me);
	vector_copy = (DREF DeeObject **)Dee_TryMallocc(me->sv_length,
	                                                sizeof(DREF DeeObject *));
	if unlikely(!vector_copy)
		goto err_cannot_inherit;

	/* Simply copy all the elements, transferring
	 * all the references that they represent. */
	vector_copy = (DREF DeeObject **)memcpyc(vector_copy, me->sv_vector,
	                                         me->sv_length, sizeof(DREF DeeObject *));

	/* Give the SharedVector its very own copy
	 * which it will take to its grave. */
	me->sv_vector = vector_copy;
	SharedVector_LockEndWrite(me);
	Dee_Decref(me);
	return;

err_cannot_inherit:
	/* Special case: failed to create a copy that the vector may call its own. */
	vector = me->sv_vector;
	length = me->sv_length;

	/* Override with an empty vector. */
	me->sv_vector = NULL;
	me->sv_length = 0;
	SharedVector_LockEndWrite(me);

	/* Destroy the items that the caller wanted the vector to inherit. */
	Dee_Decrefv(vector, length);
	Dee_Decref(me);
}

/* Same as `DeeSharedVector_Decref()', but should be used if the caller
 * does *not* want to gift the vector references to all of its items.
 * s.a.: the "maybe DREF" annotated on the `vector' argument of
 *       `DeeSharedVector_NewShared()' */
PUBLIC NONNULL((1)) void DCALL
DeeSharedVector_DecrefNoGiftItems(DREF DeeObject *__restrict self) {
	DREF DeeObject **vector_copy;
	SharedVector *me = (SharedVector *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSharedVector_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		Dee_DecrefNokill(&DeeSharedVector_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		SharedVector_Free(me);
		return;
	}

	/* Difficult case: must duplicate the vector. */
	SharedVector_LockWrite(me);
	vector_copy = (DREF DeeObject **)Dee_TryMallocc(me->sv_length,
	                                                sizeof(DREF DeeObject *));
	if unlikely(!vector_copy)
		goto err_cannot_inherit;

	/* Copy all the elements, but also incref them at the same time. */
	vector_copy = Dee_Movrefv(vector_copy, me->sv_vector, me->sv_length);

	/* Give the SharedVector its very own copy
	 * which it will take to its grave. */
	me->sv_vector = vector_copy;
	SharedVector_LockEndWrite(me);
	Dee_Decref(me);
	return;

err_cannot_inherit:
	/* Override with an empty vector. */
	me->sv_vector = NULL;
	me->sv_length = 0;
	SharedVector_LockEndWrite(me);
	Dee_Decref(me);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SVEC_C */
