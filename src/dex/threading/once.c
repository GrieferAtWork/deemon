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
#ifndef GUARD_DEX_THREADING_ONCE_C
#define GUARD_DEX_THREADING_ONCE_C 1
#define CONFIG_BUILDING_LIBTHREADING
#define DEE_SOURCE

#include "libthreading.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/once.h>

#include <hybrid/sched/yield.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	Dee_once_t      o_once;  /* Once controller. */
	DREF DeeObject *o_value; /* [0..1] The callback to execute, or the result of the callback.
	                          * When NULL:
	                          * - `o_once' hasn't run yet (or is currently running,
	                          *   and no callback was set during construction).
	                          * - `o_once' has already run, but the Once-object was GC-cleared
	                          * When non-NULL:
	                          * - the callback set during construction
	                          * - the result of the callback (if `o_once' says that the call
	                          *   has already been made)
	                          * Note that this field is [lock(o_inuse && ATOMIC)] when it
	                          * represents the result of a once-callback, but [lock(o_once)]
	                          * when the once-operation hasn't been executed, yet. */
#ifndef CONFIG_NO_THREADS
	size_t          o_inuse; /* [lock(ATOMIC)] Non-zero if someone is reading from `o_value' */
#endif /* !CONFIG_NO_THREADS */
} DeeOnceObject;

#ifndef CONFIG_NO_THREADS
#define DeeOnce_InUseInc(self)     atomic_inc(&(self)->o_inuse)
#define DeeOnce_InUseDec(self)     atomic_dec(&(self)->o_inuse)
#define DeeOnce_InUseWaitFor(self)                 \
	do {                                           \
		while (atomic_read(&(self)->o_inuse) != 0) \
			SCHED_YIELD();                         \
	}	__WHILE0
#else /* !CONFIG_NO_THREADS */
#define DeeOnce_InUseInc(self)     (void)0
#define DeeOnce_InUseDec(self)     (void)0
#define DeeOnce_InUseWaitFor(self) (void)0
#endif /* CONFIG_NO_THREADS */

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_attribute_string(DeeTypeObject *__restrict tp,
                             char const *__restrict name) {
	ASSERT_OBJECT(tp);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%k.%s'",
	                       tp, name);
}

PRIVATE ATTR_COLD int DCALL err_unbound_once_callback(void) {
	return err_unbound_attribute_string(&DeeOnce_Type, "callback");
}

PRIVATE ATTR_COLD int DCALL err_unbound_once_result(void) {
	return err_unbound_attribute_string(&DeeOnce_Type, "result");
}

PRIVATE ATTR_COLD int DCALL err_once_already_finished(void) {
	return DeeError_Throwf(&DeeError_ValueError, "Once-object has already finished");
}

PRIVATE ATTR_COLD int DCALL err_once_not_finished(void) {
	return DeeError_Throwf(&DeeError_ValueError, "Once-object has not finished");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_ctor(DeeOnceObject *__restrict self) {
	Dee_once_init(&self->o_once);
	self->o_value = NULL;
#ifndef CONFIG_NO_THREADS
	self->o_inuse = 0;
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
once_copy(DeeOnceObject *__restrict self,
          DeeOnceObject *__restrict other) {
	int error;
	error = Dee_once_begin(&other->o_once);
	if unlikely(error < 0)
		goto err;
	if (error != 0) {
		self->o_value = other->o_value;
		Dee_XIncref(self->o_value);
		Dee_once_abort(&other->o_once);
		Dee_once_init(&self->o_once);
	} else {
		DeeOnce_InUseInc(other);
		self->o_value = atomic_read(&other->o_value);
		Dee_XIncref(self->o_value);
		DeeOnce_InUseDec(other);
		Dee_once_init_didrun(&self->o_once);
	}
#ifndef CONFIG_NO_THREADS
	self->o_inuse = 0;
#endif /* !CONFIG_NO_THREADS */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_deepload(DeeOnceObject *__restrict self) {
	int error;
	error = Dee_once_begin(&self->o_once);
	if unlikely(error < 0)
		goto err;
	if (error != 0) {
		error = 0;
		if (self->o_value)
			error = DeeObject_InplaceDeepCopy(&self->o_value);
		Dee_once_abort(&self->o_once);
		if unlikely(error)
			goto err;
	} else {
		DREF DeeObject *value;
		DeeOnce_InUseInc(self);
		value = atomic_read(&self->o_value);
		Dee_XIncref(value);
		DeeOnce_InUseDec(self);
		if (value) {
			error = DeeObject_InplaceDeepCopy(&value);
			if unlikely(error) {
				Dee_Decref(value);
				goto err;
			}
			value = atomic_xch(&self->o_value, value);
			if likely(value) {
				DeeOnce_InUseWaitFor(self);
				Dee_Decref(value);
			}
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_init(DeeOnceObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	int error;
	Dee_once_init(&self->o_once);
	self->o_value = NULL;
#ifndef CONFIG_NO_THREADS
	self->o_inuse = 0;
#endif /* !CONFIG_NO_THREADS */
	error = DeeArg_Unpack(argc, argv, "|o:Once", &self->o_value);
	if likely(error == 0)
		Dee_XIncref(self->o_value);
	return error;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_init_kw(DeeOnceObject *__restrict self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	PRIVATE struct keyword kwlist[] = { K(callback), KEND };
	int error;
	Dee_once_init(&self->o_once);
	self->o_value = NULL;
#ifndef CONFIG_NO_THREADS
	self->o_inuse = 0;
#endif /* !CONFIG_NO_THREADS */
	error = DeeArg_UnpackKw(argc, argv, kw, kwlist, "|o:Once", &self->o_value);
	if likely(error == 0)
		Dee_XIncref(self->o_value);
	return error;
}

PRIVATE NONNULL((1)) void DCALL
once_fini(DeeOnceObject *__restrict self) {
	Dee_XDecref(self->o_value);
}

PRIVATE NONNULL((1)) void DCALL
once_visit(DeeOnceObject *__restrict self, dvisit_t proc, void *arg) {
	switch (Dee_once_trybegin(&self->o_once)) {

	case 0: {
		DeeObject *value;
		/* Once already run. */
		DeeOnce_InUseInc(self);
		value = atomic_read(&self->o_value);
		Dee_XVisit(value);
		DeeOnce_InUseDec(self);
	}	break;

	case 1:
		/* Once-lock acquired. */
		Dee_XVisit(self->o_value);
		Dee_once_abort(&self->o_once);
		break;

	case 2:
		/* Another thread is currently executing the once-function */
		break;

	default: __builtin_unreachable();
	}
}

PRIVATE NONNULL((1)) void DCALL
once_clear(DeeOnceObject *__restrict self) {
	DREF DeeObject *value;
	switch (Dee_once_trybegin(&self->o_once)) {

	case 0:
		/* Once already run. */
		value = atomic_xch(&self->o_value, NULL);
		if (value)
			DeeOnce_InUseWaitFor(self);
		break;

	case 1:
		/* Once-lock acquired. */
		value = self->o_value;
		self->o_value = NULL;
		Dee_once_abort(&self->o_once);
		break;

	case 2:
		/* Another thread is currently executing the once-function */
		value = NULL;
		break;

	default: __builtin_unreachable();
	}
	Dee_XDecref(value);
}


PRIVATE struct type_gc tpconst once_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&once_clear
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
once_call_kw(DeeOnceObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int status;
	DREF DeeObject *callback;
	DREF DeeObject *result;

	/* Check for simple case: already run */
	if (Dee_once_hasrun(&self->o_once)) {
already_run:
		DeeOnce_InUseInc(self);
		result = atomic_read(&self->o_value);
		Dee_XIncref(result);
		DeeOnce_InUseDec(self);
		if unlikely(!result)
			goto err_unbound_result;
		return result;
	}

	/* Try to start the once-operation.
	 *
	 * NOTE: In paren, so we don't use the macro fast-pass, since
	 *       that one just checks `Dee_once_hasrun()', which we
	 *       already do manually. */
	status = (Dee_once_begin)(&self->o_once);
	if unlikely(status <= 0) {
		if unlikely(status < 0)
			goto err;
		goto already_run;
	}

	/* We're now responsible for executing the once-operation. */
	callback = self->o_value;
	if (callback) {
		/* A callback has been pre-defined. In this case, the caller-
		 * given arguments are to-be passed along as-it to the callback. */
		result = DeeObject_CallKw(callback, argc, argv, kw);
	} else {
		/* No callback has been given. In this case, we must process the
		 * caller-given arguments for 2 arguments `(callback,args)' */
		DeeObject *cb, *cb_args = Dee_EmptyTuple, *cb_kwds = NULL;
		PRIVATE struct keyword kwlist[] = { K(callback), K(args), K(kwds), KEND };
		if unlikely(DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|oo:Once.operator()", &cb, &cb_args, &cb_kwds))
			goto err_abort;
		if unlikely(DeeObject_AssertTypeExact(cb_args, &DeeTuple_Type))
			goto err_abort;
		result = DeeObject_CallTupleKw(cb, cb_args, cb_kwds);
	}

	/* Check if the callback completed with an error. */
	if unlikely(!result)
		goto err_abort;

	/* Store the result in the once-controller. */
	Dee_Incref(result);
	ASSERT(self->o_value == callback);
	self->o_value = result; /* This like causes `callback' to inherit a reference. */

	/* Commit the once-operation (thus indicating that it has completed) */
	Dee_once_commit(&self->o_once);
	Dee_XDecref(callback);
	return result;
err_unbound_result:
	err_unbound_once_result();
	goto err;
err_abort:
	Dee_once_abort(&self->o_once);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
once_finish(DeeOnceObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	int status;
	DeeObject *result;
	bool override_ = false;
	PRIVATE struct keyword kwlist[] = { K(result), K(override), KEND };
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|b:finish", &result, &override_))
		goto err;
	status = Dee_once_begin(&self->o_once);
	if (status <= 0) {
		if unlikely(status < 0)
			goto err;
		if (!override_)
			goto err_already_finished;
		Dee_Incref(result);
		result = atomic_xch(&self->o_value, result);
		DeeOnce_InUseWaitFor(self);
		Dee_XDecref(result);
	} else {
		/* Complete the once-objct with the given value */
		DREF DeeObject *callback;
		callback = self->o_value;
		Dee_Incref(result);
		self->o_value = result;
		Dee_once_commit(&self->o_once);
		Dee_XDecref(callback);
	}
	return_none;
err_already_finished:
	err_once_already_finished();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
once_callback_get(DeeOnceObject *__restrict self) {
	DREF DeeObject *result;
	if (Dee_once_trybegin(&self->o_once) != 1)
		goto err_already_finished;

	/* Once-lock acquired. */
	result = self->o_value;
	Dee_XIncref(result);
	Dee_once_abort(&self->o_once);
	if unlikely(!result)
		err_unbound_once_callback();
	return result;
err_already_finished:
	err_once_already_finished();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_callback_del(DeeOnceObject *__restrict self) {
	DREF DeeObject *value;
	if (Dee_once_trybegin(&self->o_once) != 1)
		goto err_already_finished;

	/* Once-lock acquired. */
	value = self->o_value;
	self->o_value = NULL;
	Dee_once_abort(&self->o_once);
	Dee_XDecref(value);
	return 0;
err_already_finished:
	return err_once_already_finished();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
once_callback_set(DeeOnceObject *self, DeeObject *new_value) {
	DREF DeeObject *old_value;
	if (Dee_once_trybegin(&self->o_once) != 1)
		goto err_already_finished;

	/* Once-lock acquired. */
	Dee_Incref(new_value);
	old_value = self->o_value;
	self->o_value = new_value;
	Dee_once_abort(&self->o_once);
	Dee_XDecref(old_value);
	return 0;
err_already_finished:
	return err_once_already_finished();
}

PRIVATE struct type_method once_methods[] = {
	TYPE_KWMETHOD("finish", &once_finish,
	              "(result,override=!f)\n"
	              "#tValueError{@this ?GOnce-object has already finished, and @override is false}"
	              "By-pass any stored callback and force @this ?GOnce-object to finish with "
	              /**/ "the given @result. When @override is true, override a stored ?#result "
	              /**/ "with the given @result in case the ?GOnce-object has already completed."),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
once_result_get(DeeOnceObject *__restrict self) {
	DREF DeeObject *result;
	if (!Dee_once_hasrun(&self->o_once))
		goto err_not_finished;

	DeeOnce_InUseInc(self);
	result = self->o_value;
	if unlikely(!result) {
		DeeOnce_InUseDec(self);
		err_unbound_once_result();
		goto err;
	}
	Dee_Incref(result);
	DeeOnce_InUseDec(self);
	return result;
err_not_finished:
	err_once_not_finished();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_result_del(DeeOnceObject *__restrict self) {
	DREF DeeObject *value;
	if (!Dee_once_hasrun(&self->o_once))
		goto err_not_finished;
	value = self->o_value;
	self->o_value = NULL;
	DeeOnce_InUseWaitFor(self);
	Dee_XDecref(value);
	return 0;
err_not_finished:
	return err_once_not_finished();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_result_set(DeeOnceObject *self, DeeObject *new_value) {
	DREF DeeObject *old_value;
	if (!Dee_once_hasrun(&self->o_once))
		goto err_not_finished;
	Dee_Incref(new_value);
	old_value = self->o_value;
	self->o_value = new_value;
	DeeOnce_InUseWaitFor(self);
	Dee_XDecref(old_value);
	return 0;
err_not_finished:
	return err_once_not_finished();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
once_bool(DeeOnceObject *__restrict self) {
	return Dee_once_hasrun(&self->o_once) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
once_hasrun_get(DeeOnceObject *__restrict self) {
	return_bool(Dee_once_hasrun(&self->o_once));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
once_isrunning_get(DeeOnceObject *__restrict self) {
	return_bool(Dee_once_isrunning(&self->o_once));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
once_ispending_get(DeeOnceObject *__restrict self) {
	return_bool(Dee_once_ispending(&self->o_once));
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
once_print(DeeOnceObject *__restrict self, dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *value;
	switch (Dee_once_trybegin(&self->o_once)) {

	case 0:
		/* Once already run. */
		DeeOnce_InUseInc(self);
		value = atomic_read(&self->o_value);
		Dee_XIncref(value);
		DeeOnce_InUseDec(self);
		if likely(value) {
			result = DeeFormat_Printf(printer, arg, "<Once with result %k>", value);
			Dee_Decref(value);
		} else {
			result = DeeFormat_PRINT(printer, arg, "<Once with result <unbound>>");
		}
		break;

	case 1:
		/* Once-lock acquired. */
		value = self->o_value;
		Dee_XIncref(value);
		Dee_once_abort(&self->o_once);
		if (value) {
			result = DeeFormat_Printf(printer, arg, "<Once with callback %k>", value);
			Dee_Decref(value);
		} else {
			result = DeeFormat_PRINT(printer, arg, "<Once with callback <unbound>>");
		}
		break;

	case 2:
		/* Another thread is currently executing the once-function */
		result = DeeFormat_PRINT(printer, arg, "<Once that is currently running>");
		break;

	default: __builtin_unreachable();
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
once_printrepr(DeeOnceObject *__restrict self, dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *value;
	switch (Dee_once_trybegin(&self->o_once)) {

	case 0:
		/* Once already run. */
		DeeOnce_InUseInc(self);
		value = atomic_read(&self->o_value);
		Dee_XIncref(value);
		DeeOnce_InUseDec(self);
		if likely(value) {
			result = DeeFormat_Printf(printer, arg, "Once(() -> %r)<.hasrun>", value);
			Dee_Decref(value);
		} else {
			result = DeeFormat_PRINT(printer, arg, "Once(() -> <unbound>)<.hasrun>");
		}
		break;

	case 1:
		/* Once-lock acquired. */
		value = self->o_value;
		Dee_XIncref(value);
		Dee_once_abort(&self->o_once);
		if (value) {
			result = DeeFormat_Printf(printer, arg, "Once(callback: %r)", value);
			Dee_Decref(value);
		} else {
			result = DeeFormat_PRINT(printer, arg, "Once()");
		}
		break;

	case 2:
		/* Another thread is currently executing the once-function */
		result = DeeFormat_PRINT(printer, arg, "Once()<.isrunning>");
		break;

	default: __builtin_unreachable();
	}
	return result;
}


PRIVATE struct type_getset once_getsets[] = {
	TYPE_GETSET("callback", &once_callback_get, &once_callback_del, &once_callback_set,
	            "->?DCallable\n"
	            "#tUnboundAttribute{No callback has been assigned}"
	            "#tValueError{The ?GOnce-object has already been invoked (or is being "
	            /*             */ "invoked right now), and the callback can no longer be "
	            /*             */ "accessed or modified.}"
	            "Get/set the callback invoked by this ?GOnce-object"),
	TYPE_GETSET("result", &once_result_get, &once_result_del, &once_result_set,
	            "->\n"
	            "#tUnboundAttribute{No result has been assigned}"
	            "#tValueError{The ?GOnce-object has not yet been invoked, "
	            /*             */ "so the result cannot be accessed (yet).}"
	            "Get/set the result of this ?GOnce-object"),
	TYPE_GETTER("hasrun", &once_hasrun_get,
	            "->?Dbool\n"
	            "Returns ?t if @this ?GOnce-object has already been executed (same as ?#{op:bool})"),
	TYPE_GETTER("isrunning", &once_isrunning_get,
	            "->?Dbool\n"
	            "Returns ?t if @this ?GOnce-object is currently running"),
	TYPE_GETTER("ispending", &once_ispending_get,
	            "->?Dbool\n"
	            "Returns ?t if @this ?GOnce-object is currently pending execution"),
	TYPE_GETSET_END
};


INTERN DeeTypeObject DeeOnce_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Once",
	/* .tp_doc      = */ DOC("Helper to ensure that some operation only happens once. "
	                         /**/ "?GOnce can be used in one of 2 ways:\n"
	                         "${"
	                         /**/ "import Once from threading;\n"
	                         /**/ "local o = Once(x -\\> { print \"Calculating...\"; return x * 2; });\n"
	                         /**/ "print o(42);        /* Calculating... 84 */\n"
	                         /**/ "print o();          /* 84 */\n"
	                         /**/ "print o(\"ignored\"); /* 84 */\n"
	                         /**/ "print o(\"ignored\"); /* 84 */"
	                         "}\n"
	                         "${"
	                         /**/ "import Once from threading;\n"
	                         /**/ "function gen(x) {\n"
	                         /**/ "	print \"Calculating...\";\n"
	                         /**/ "	return x * 2;\n"
	                         /**/ "}\n"
	                         /**/ "local o = Once();\n"
	                         /**/ "print o(gen, (42,));        /* Calculating... 84 */\n"
	                         /**/ "print o(gen);               /* 84 */\n"
	                         /**/ "print o(gen, (\"ignored\",)); /* 84 */"
	                         "}\n"
	                         "\n"

	                         "()\n"
	                         "Construct a ?GOnce-object without a pre-defined callback. In this "
	                         /**/ "case, the callback must instead be given to ?#{op:call}\n"
	                         "\n"

	                         "(callback:?DCallable)\n"
	                         "Construct a ?GOnce-object with a pre-defined callback. This callback "
	                         /**/ "is invoked only the first time ?#{op:call} is invoked.\n"
	                         "\n"

	                         "call(args!,kwds!!)->\n"
	                         "#tUnboundAttribute{@this ?GOnce-object's has finished, but then its ?#result was unbound}"
	                         "Invoke the callback given to ?#{op:constructor} with @args and @kwds "
	                         /**/ "and return its result. The callback is only invoked the first time, "
	                         /**/ "and any subsequence call only ever returns the return value of "
	                         /**/ "that initial call.\n"
	                         "This version of the operator is only used when A callback has been set"
	                         /**/ "by the initial call to ?#{op:constructor} (iow: a single arguments was "
	                         /**/ "given to the constructor of @this ?GOnce-object).\n"
	                         "\n"

	                         "call(callback:?DCallable,args:?DTuple=!T0,kwds?:?M?Dstring?O)->\n"
	                         "#tUnboundAttribute{@this ?GOnce-object's has finished, but then its ?#result was unbound}"
	                         "Invoke @callback with @args and @kwds if @this ?GOnce-object hasn't been executed already.\n"
	                         "The return value of this call is what gets returned by @callback, or what was "
	                         /**/ "returned the first time @this ?GOnce-object was invoked."
	                         "This version of the operator is only used when NO callback has been set"
	                         /**/ "by the initial call to ?#{op:constructor} (iow: no arguments were given "
	                         /**/ "to the constructor of @this ?GOnce-object).\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?GOnce-object has already been executed (same as ?#hasrun)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&once_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&once_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&once_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&once_init,
				TYPE_FIXED_ALLOCATOR_GC(DeeOnceObject),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&once_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&once_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&once_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&once_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&once_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&once_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&once_visit,
	/* .tp_gc            = */ &once_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ once_methods,
	/* .tp_getsets       = */ once_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&once_call_kw
};

DECL_END

#endif /* !GUARD_DEX_THREADING_ONCE_C */
