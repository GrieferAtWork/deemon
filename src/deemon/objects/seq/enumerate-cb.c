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
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/thread.h>

#include <hybrid/overflow.h>

/**/
#include "enumerate-cb.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

/************************************************************************/
/* USER -> NATIVE                                                       */
/************************************************************************/

INTERN WUNUSED NONNULL((2)) Dee_ssize_t DCALL
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




/************************************************************************/
/* NATIVE -> USER                                                       */
/************************************************************************/

/* Special non-none value that indicates that the enumeration should stop. */
PRIVATE DeeObject ew_stop = { OBJECT_HEAD_INIT(&DeeObject_Type) };

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
ew_docall(EnumerateWrapper *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t result;
	DeeObject *key, *value = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:EnumerateWrapper.__call__", &key, &value))
		goto err;
	if (Dee_TYPE(self) == &EnumerateWrapper_Type) {
		result = (*self->ew_cb.cb_enumerate)(self->ew_arg, key, value);
	} else {
		size_t index;
		if (DeeObject_AsSize(key, &index))
			goto err;
		result = (*self->ew_cb.cb_enumerate_index)(self->ew_arg, index, value);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ew_call(EnumerateWrapper *self, size_t argc, DeeObject *const *argv) {
	DeeThreadObject *thread;
	Dee_ssize_t foreach_status;
	uint16_t old_exceptsz;
	if (Dee_rshared_lock_acquire(&self->ew_lock))
		goto err;
	if unlikely(self->ew_res < 0) {
stop_unlock:
		Dee_rshared_lock_release(&self->ew_lock);
		return_reference_(&ew_stop);
	}
	thread = DeeThread_Self();
	old_exceptsz   = thread->t_exceptsz;
	foreach_status = ew_docall(self, argc, argv);
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
		self->ew_res = foreach_status;
		ASSERT(!self->ew_err);
		if (old_exceptsz != thread->t_exceptsz) {
			self->ew_err = thread->t_except->ef_error;
			Dee_Incref(self->ew_err);
			goto err_unlock;
		}
		goto stop_unlock;
	} else {
		ASSERT(old_exceptsz == thread->t_exceptsz);
		/* For safety: usually we don't check for overflow, but here we
		 *             do because this callback is under the complete
		 *             control of user-code (which might do weird stuff) */
		if (OVERFLOW_SADD(self->ew_res, foreach_status, &self->ew_res))
			self->ew_res = SSIZE_MAX;
		self->ew_res += foreach_status;
	}
	Dee_rshared_lock_release(&self->ew_lock);
	return_none; /* Continue enumeration */
err_unlock:
	Dee_rshared_lock_release(&self->ew_lock);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
ew_fini(EnumerateWrapper *__restrict self) {
	Dee_XDecref(self->ew_err);
}

PRIVATE NONNULL((1, 2)) void DCALL
ew_visit(EnumerateWrapper *__restrict self, dvisit_t proc, void *arg) {
	Dee_rshared_lock_acquire_noint(&self->ew_lock);
	Dee_XVisit(self->ew_err);
	Dee_rshared_lock_release(&self->ew_lock);
}

INTERN DeeTypeObject EnumerateWrapper_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_EnumerateWrapper",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(EnumerateWrapper)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ew_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject *const *))&ew_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ew_visit,
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
	/* .tp_getsets       = */ NULL, /* TODO: Expose `EnumerateWrapper.ew_res' (read-only) */
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject EnumerateIndexWrapper_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_EnumerateIndexWrapper",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(EnumerateWrapper)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ew_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject *const *))&ew_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ew_visit,
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
	/* .tp_getsets       = */ NULL, /* TODO: Expose `EnumerateWrapper.ew_res' (read-only) */
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
deleted_enumerate_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	(void)arg;
	(void)index;
	(void)value;
	return DeeError_Throwf(&DeeError_RuntimeError, "Enumeration has stopped");
}

/* Destroy "self" if not shared, else clear the "ew_cb" callback,
 * to ensure that it won't be called anymore. This function also
 * briefly acquires a lock to "ew_lock" (without interrupts) to
 * ensure that after a call to this function, nothing may still
 * invoke the callback.
 *
 * @param userproc_result: The return value of the user-defined callback.
 *                         May be "NULL" if the user-defined callback threw
 *                         an error. */
INTERN WUNUSED NONNULL((1)) Dee_ssize_t DCALL
EnumerateWrapper_Decref(/*inherit(always)*/ DREF EnumerateWrapper *self,
                        /*inherit(always)*/ DREF DeeObject *userproc_result) {
	DREF DeeObject *err;
	Dee_ssize_t result;
	bool shared;
	ASSERT_OBJECT(self);
	ASSERT(Dee_TYPE(self) == &EnumerateWrapper_Type ||
	       Dee_TYPE(self) == &EnumerateIndexWrapper_Type);
	shared = DeeObject_IsShared(self);
	if (shared)
		Dee_rshared_lock_acquire_noint(&self->ew_lock);
	if unlikely(!userproc_result) {
		result = -1;
	} else {
		result = self->ew_res;
		if (self->ew_err) {
			ASSERT(result < 0);
			DeeError_Throw(self->ew_err);
		}
	}
	err = self->ew_err;
	if (shared) {
		self->ew_cb.cb_enumerate = &deleted_enumerate_cb;
		self->ew_res = 0;
		self->ew_err = NULL;
		Dee_rshared_lock_release(&self->ew_lock);
		Dee_Decref_unlikely(self);
	} else {
		Dee_DecrefNokill(Dee_TYPE(self));
		DeeObject_FREE(self);
	}
	Dee_XDecref(err);
	return result;
}

/* Create new enumerate/enumerate_index wrapper objects. */
INTERN WUNUSED NONNULL((1)) DREF EnumerateWrapper *DCALL
EnumerateWrapper_New(Dee_enumerate_t cb, void *arg) {
	DREF EnumerateWrapper *result;
	result = DeeObject_MALLOC(EnumerateWrapper);
	if unlikely(!result)
		goto err;
	result->ew_cb.cb_enumerate = cb;
	result->ew_arg = arg;
	result->ew_res = 0;
	result->ew_err = NULL;
	Dee_rshared_lock_init(&result->ew_lock);
	DeeObject_Init(result, &EnumerateWrapper_Type);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF EnumerateWrapper *DCALL
EnumerateIndexWrapper_New(Dee_enumerate_index_t cb, void *arg) {
	DREF EnumerateWrapper *result;
	result = DeeObject_MALLOC(EnumerateWrapper);
	if unlikely(!result)
		goto err;
	result->ew_cb.cb_enumerate_index = cb;
	result->ew_arg = arg;
	result->ew_res = 0;
	result->ew_err = NULL;
	Dee_rshared_lock_init(&result->ew_lock);
	DeeObject_Init(result, &EnumerateIndexWrapper_Type);
	return result;
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_ENUMERATE_CB_C */
