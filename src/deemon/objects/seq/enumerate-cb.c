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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_ENUMERATE_CB_C
#define GUARD_DEEMON_OBJECTS_SEQ_ENUMERATE_CB_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/callable.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/thread.h>
#include <deemon/util/rlock.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>
/**/

#include "enumerate-cb.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

/************************************************************************/
/* USER -> NATIVE                                                       */
/************************************************************************/

INTERN WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_enumerate_cb(void *arg, DeeObject *key_or_index,
                 /*nullable*/ DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	struct seq_enumerate_data *data;
	data    = (struct seq_enumerate_data *)arg;
	args[0] = key_or_index;
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

INTERN WUNUSED Dee_ssize_t DCALL
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




/* Helpers for enumerating a sequence by invoking a given callback. */
INTERN NONNULL((1, 2)) DREF DeeObject *DCALL
seq_call_enumerate(DeeObject *self, DeeObject *cb) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeObject_InvokeMethodHint(seq_enumerate, self, &seq_enumerate_cb, &data);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

INTERN NONNULL((1, 2)) DREF DeeObject *DCALL
seq_call_enumerate_with_intrange(DeeObject *self, DeeObject *cb,
                                 size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeObject_InvokeMethodHint(seq_enumerate_index, self,
	                                            &seq_enumerate_index_cb,
	                                            &data, start, end);
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
seq_call_enumerate_with_range(DeeObject *self, DeeObject *cb,
                              DeeObject *start, DeeObject *end) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_with_filter_data data;
	data.sedwf_cb    = cb;
	data.sedwf_start = start;
	data.sedwf_end   = end;
	foreach_status = DeeObject_InvokeMethodHint(seq_enumerate, self, &seq_enumerate_with_filter_cb, &data);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sedwf_result;
	return_none;
err:
	return NULL;
}



/* Helpers for enumerating a mapping by invoking a given callback. */
INTERN NONNULL((1, 2)) DREF DeeObject *DCALL
map_call_enumerate(DeeObject *self, DeeObject *cb) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeObject_InvokeMethodHint(map_enumerate, self, &seq_enumerate_cb, &data);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

INTERN NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
map_call_enumerate_with_range(DeeObject *self, DeeObject *cb,
                              DeeObject *start, DeeObject *end) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeObject_InvokeMethodHint(map_enumerate_range, self,
	                                            &seq_enumerate_cb, &data, start, end);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}




/************************************************************************/
/* NATIVE -> USER                                                       */
/************************************************************************/

/* Special non-none value that indicates that the enumeration should stop. */
PRIVATE DeeObject sew_stop = { OBJECT_HEAD_INIT(&DeeObject_Type) };

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sew_docall(SeqEnumerateWrapper *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t result;
	DeeObject *key, *value = NULL;
	_DeeArg_Unpack1Or2(err, argc, argv, "SeqEnumerateWrapper.__call__", &key, &value);
	if (Dee_TYPE(self) == &SeqEnumerateWrapper_Type) {
		result = (*self->sew_cb.cb_enumerate)(self->sew_arg, key, value);
	} else {
		size_t index;
		if unlikely(DeeObject_AsSize(key, &index))
			goto err;
		result = (*self->sew_cb.cb_enumerate_index)(self->sew_arg, index, value);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sew_call(SeqEnumerateWrapper *self, size_t argc, DeeObject *const *argv) {
	DeeThreadObject *thread;
	Dee_ssize_t foreach_status;
	uint16_t old_exceptsz;
	if (Dee_rshared_lock_acquire(&self->sew_lock))
		goto err;
	if unlikely(self->sew_res < 0) {
stop_unlock:
		Dee_rshared_lock_release(&self->sew_lock);
		return_reference_(&sew_stop);
	}
	thread = DeeThread_Self();
	old_exceptsz   = thread->t_exceptsz;
	foreach_status = sew_docall(self, argc, argv);
	if (foreach_status < 0) {
		/* Have to be careful here: only a return value of "-1" guaranties
		 * that an error was thrown, but any other negative status can still
		 * be used to propagate specific error (or not propagate errors at
		 * all).
		 *
		 * To keep things simple, allow any negative return value to indicate
		 * an optional error (and just check if one was thrown). */
		ASSERT((old_exceptsz + 0) == thread->t_exceptsz ||
		       (old_exceptsz + 1) == thread->t_exceptsz);
		self->sew_res = foreach_status;
		ASSERT(!self->sew_err);
		if (old_exceptsz != thread->t_exceptsz) {
			self->sew_err = thread->t_except->ef_error;
			Dee_Incref(self->sew_err);
			goto err_unlock;
		}
		goto stop_unlock;
	} else {
		ASSERT(old_exceptsz == thread->t_exceptsz);
		/* For safety: usually we don't check for overflow, but here we
		 *             do because this callback is under the complete
		 *             control of user-code (which might do weird stuff) */
		if (OVERFLOW_SADD(self->sew_res, foreach_status, &self->sew_res))
			self->sew_res = SSIZE_MAX;
		self->sew_res += foreach_status;
	}
	Dee_rshared_lock_release(&self->sew_lock);
	return_none; /* Continue enumeration */
err_unlock:
	Dee_rshared_lock_release(&self->sew_lock);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
sew_fini(SeqEnumerateWrapper *__restrict self) {
	Dee_XDecref(self->sew_err);
}

PRIVATE NONNULL((1, 2)) void DCALL
sew_visit(SeqEnumerateWrapper *__restrict self, dvisit_t proc, void *arg) {
	Dee_rshared_lock_acquire_noint(&self->sew_lock);
	Dee_XVisit(self->sew_err);
	Dee_rshared_lock_release(&self->sew_lock);
}

PRIVATE struct type_member tpconst sew_members[] = {
	TYPE_MEMBER_FIELD("__status__", STRUCT_SSIZE_T | STRUCT_CONST | STRUCT_ATOMIC,
	                  offsetof(SeqEnumerateWrapper, sew_res)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqEnumerateWrapper_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumerateWrapper",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(SeqEnumerateWrapper)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sew_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&object_repr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default__printrepr__with__repr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sew_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__8F384E6A64571883),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sew_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject *const *))&sew_call,
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject SeqEnumerateIndexWrapper_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumerateIndexWrapper",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(SeqEnumerateWrapper)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sew_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&object_repr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default__printrepr__with__repr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sew_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__8F384E6A64571883),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sew_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject *const *))&sew_call,
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
deleted_enumerate_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	(void)arg;
	(void)index;
	(void)value;
	return DeeError_Throwf(&DeeError_RuntimeError, "Enumeration has stopped");
}

/* Destroy "self" if not shared, else clear the "sew_cb" callback,
 * to ensure that it won't be called anymore. This function also
 * briefly acquires a lock to "sew_lock" (without interrupts) to
 * ensure that after a call to this function, nothing may still
 * invoke the callback.
 *
 * @param userproc_result: The return value of the user-defined callback.
 *                         May be "NULL" if the user-defined callback threw
 *                         an error. */
INTERN WUNUSED NONNULL((1)) Dee_ssize_t DCALL
SeqEnumerateWrapper_Decref(/*inherit(always)*/ DREF SeqEnumerateWrapper *self,
                        /*inherit(always)*/ DREF DeeObject *userproc_result) {
	DREF DeeObject *err;
	Dee_ssize_t result;
	bool shared;
	ASSERT_OBJECT(self);
	ASSERT(Dee_TYPE(self) == &SeqEnumerateWrapper_Type ||
	       Dee_TYPE(self) == &SeqEnumerateIndexWrapper_Type);
	shared = DeeObject_IsShared(self);
	if (shared)
		Dee_rshared_lock_acquire_noint(&self->sew_lock);
	if unlikely(!userproc_result) {
		result = -1;
	} else {
		result = self->sew_res;
		if (self->sew_err) {
			ASSERT(result < 0);
			DeeError_Throw(self->sew_err);
		}
	}
	err = self->sew_err;
	if (shared) {
		self->sew_cb.cb_enumerate = &deleted_enumerate_cb;
		self->sew_res = 0;
		self->sew_err = NULL;
		Dee_rshared_lock_release(&self->sew_lock);
		Dee_Decref_unlikely(self);
	} else {
		Dee_DecrefNokill(Dee_TYPE(self));
		DeeObject_FREE(self);
	}
	Dee_XDecref(err);
	return result;
}

/* Create new enumerate/enumerate_index wrapper objects. */
INTERN WUNUSED NONNULL((1)) DREF SeqEnumerateWrapper *DCALL
SeqEnumerateWrapper_New(Dee_seq_enumerate_t cb, void *arg) {
	DREF SeqEnumerateWrapper *result;
	result = DeeObject_MALLOC(SeqEnumerateWrapper);
	if unlikely(!result)
		goto err;
	result->sew_cb.cb_enumerate = cb;
	result->sew_arg = arg;
	result->sew_res = 0;
	result->sew_err = NULL;
	Dee_rshared_lock_init(&result->sew_lock);
	DeeObject_Init(result, &SeqEnumerateWrapper_Type);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF SeqEnumerateWrapper *DCALL
SeqEnumerateIndexWrapper_New(Dee_seq_enumerate_index_t cb, void *arg) {
	DREF SeqEnumerateWrapper *result;
	result = DeeObject_MALLOC(SeqEnumerateWrapper);
	if unlikely(!result)
		goto err;
	result->sew_cb.cb_enumerate_index = cb;
	result->sew_arg = arg;
	result->sew_res = 0;
	result->sew_err = NULL;
	Dee_rshared_lock_init(&result->sew_lock);
	DeeObject_Init(result, &SeqEnumerateIndexWrapper_Type);
	return result;
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_ENUMERATE_CB_C */
