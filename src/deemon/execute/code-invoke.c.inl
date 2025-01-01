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
#ifdef __INTELLISENSE__
#include "code.c"
#define CALL_THIS  1
#define CALL_TUPLE 1
#define CALL_KW    1
#endif /* __INTELLISENSE__ */

#include <deemon/kwds.h>

DECL_BEGIN

#ifdef CALL_THIS
#ifdef CALL_KW
#ifdef CALL_TUPLE
#define LOCAL_DeeFunction_Call            DeeFunction_ThisCallTupleKw
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndThisCallTupleKw
#else /* CALL_TUPLE */
#define LOCAL_DeeFunction_Call            DeeFunction_ThisCallKw
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndThisCallKw
#endif /* !CALL_TUPLE */
#else /* CALL_KW */
#ifdef CALL_TUPLE
#define LOCAL_DeeFunction_Call            DeeFunction_ThisCallTuple
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndThisCallTuple
#else /* CALL_TUPLE */
#define LOCAL_DeeFunction_Call            DeeFunction_ThisCall
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndThisCall
#endif /* !CALL_TUPLE */
#endif /* !CALL_KW */
#else /* CALL_THIS */
#ifdef CALL_KW
#ifdef CALL_TUPLE
#define LOCAL_DeeFunction_Call            DeeFunction_CallTupleKw
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndCallTupleKw
#else /* CALL_TUPLE */
#define LOCAL_DeeFunction_Call            DeeFunction_CallKw
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndCallKw
#endif /* !CALL_TUPLE */
#else /* CALL_KW */
#ifdef CALL_TUPLE
#define LOCAL_DeeFunction_Call            DeeFunction_CallTuple
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndCallTuple
#else /* CALL_TUPLE */
#define LOCAL_DeeFunction_Call            DeeFunction_Call
#define LOCAL_DeeFunction_OptimizeAndCall DeeFunction_OptimizeAndCall
#endif /* !CALL_TUPLE */
#endif /* !CALL_KW */
#endif /* !CALL_THIS */


#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#ifdef CONFIG_HAVE_CODE_METRICS
INTERN WUNUSED DREF DeeObject *DCALL
LOCAL_DeeFunction_OptimizeAndCall(DeeFunctionObject *self
#ifdef CALL_THIS
                                  , DeeObject *this_arg
#endif /* CALL_THIS */
#ifdef CALL_TUPLE
                                  , DeeObject *args
#else /* CALL_TUPLE */
                                  , size_t argc, DeeObject *const *argv
#endif /* !CALL_TUPLE */
#ifdef CALL_KW
                                  , DeeObject *kw
#endif /* CALL_KW */
                                  ) {
	int status;
	host_cc_t cc = 0 |
#ifdef CALL_THIS
	               HOST_CC_F_THIS |
#endif /* CALL_THIS */
#ifdef CALL_TUPLE
	               HOST_CC_F_TUPLE |
#endif /* CALL_TUPLE */
#ifdef CALL_KW
	               HOST_CC_F_KW |
#endif /* CALL_KW */
	               0;
	DeeCodeObject *code = self->fo_code;
	size_t num_funcs = atomic_read(&code->co_metrics.com_functions);
#if defined(CALL_TUPLE) && defined(CALL_KW)
	size_t num_calls = atomic_read(&code->co_metrics.com_call_tuple_kw);
#elif defined(CALL_TUPLE) && !defined(CALL_KW)
	size_t num_calls = atomic_read(&code->co_metrics.com_call_tuple);
#elif !defined(CALL_TUPLE) && defined(CALL_KW)
	size_t num_calls = atomic_read(&code->co_metrics.com_call_kw);
#elif !defined(CALL_TUPLE) && !defined(CALL_KW)
	size_t num_calls = atomic_read(&code->co_metrics.com_call);
#endif /* ... */
	if (num_funcs > 1 && num_funcs >= (num_calls / 4))
		cc |= HOST_CC_F_FUNC; /* Lots of individual function -> re-compile as function-independent */
	status = DeeFunction_HostAsmRecompile(self, cc, true);
	if unlikely(status < 0)
		return NULL; /* Error during recompilation */
	if likely(status == 0) {
		if (cc & HOST_CC_F_FUNC) {
#if defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call_tuple_kw.c_this);
			return (*code->co_hostasm.haco_call_tuple_kw.c_this)(self, this_arg, args, kw);
#elif defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call_kw.c_this);
			return (*code->co_hostasm.haco_call_kw.c_this)(self, this_arg, argc, argv, kw);
#elif defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call_tuple.c_this);
			return (*code->co_hostasm.haco_call_tuple.c_this)(self, this_arg, args);
#elif defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call.c_this);
			return (*code->co_hostasm.haco_call.c_this)(self, this_arg, argc, argv);
#elif !defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call_tuple_kw.c_norm);
			return (*code->co_hostasm.haco_call_tuple_kw.c_norm)(self, args, kw);
#elif !defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call_kw.c_norm);
			return (*code->co_hostasm.haco_call_kw.c_norm)(self, argc, argv, kw);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call_tuple.c_norm);
			return (*code->co_hostasm.haco_call_tuple.c_norm)(self, args);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(code->co_hostasm.haco_call.c_norm);
			return (*code->co_hostasm.haco_call.c_norm)(self, argc, argv);
#endif /* ... */
		} else {
#if defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call_tuple_kw.c_this);
			return (*self->fo_hostasm.hafu_call_tuple_kw.c_this)(this_arg, args, kw);
#elif defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call_kw.c_this);
			return (*self->fo_hostasm.hafu_call_kw.c_this)(this_arg, argc, argv, kw);
#elif defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call_tuple.c_this);
			return (*self->fo_hostasm.hafu_call_tuple.c_this)(this_arg, args);
#elif defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call.c_this);
			return (*self->fo_hostasm.hafu_call.c_this)(this_arg, argc, argv);
#elif !defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call_tuple_kw.c_norm);
			return (*self->fo_hostasm.hafu_call_tuple_kw.c_norm)(args, kw);
#elif !defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call_kw.c_norm);
			return (*self->fo_hostasm.hafu_call_kw.c_norm)(argc, argv, kw);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call_tuple.c_norm);
			return (*self->fo_hostasm.hafu_call_tuple.c_norm)(args);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
			ASSERT(self->fo_hostasm.hafu_call.c_norm);
			return (*self->fo_hostasm.hafu_call.c_norm)(argc, argv);
#endif /* ... */
		}
	}

	/* Fallback: do a normal invocation (in this case,
	 * `DeeCode_OptimizeCallThreshold' was set to (size_t)-1,
	 * or `CODE_FNOOPTIMIZE' was set, so this won't loop) */
#if defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
	return DeeFunction_ThisCallTupleKw(self, this_arg, args, kw);
#elif defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
	return DeeFunction_ThisCallKw(self, this_arg, argc, argv, kw);
#elif defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
	return DeeFunction_ThisCallTuple(self, this_arg, args);
#elif defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
	return DeeFunction_ThisCall(self, this_arg, argc, argv);
#elif !defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
	return DeeFunction_CallTupleKw(self, args, kw);
#elif !defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
	return DeeFunction_CallKw(self, argc, argv, kw);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
	return DeeFunction_CallTuple(self, args);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
	return DeeFunction_Call(self, argc, argv);
#endif /* ... */
}
#endif /* CONFIG_HAVE_CODE_METRICS */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */




#ifdef CALL_THIS
#define INIT_THIS(frame) frame.cf_this = this_arg
#elif defined(NDEBUG)
#define INIT_THIS(frame) (void)0 /*frame.cf_this  = NULL;*/ /* Can be left uninitialized. */
#elif defined(UINT32_C) && defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
#define INIT_THIS(frame) frame.cf_this = (DeeObject *)UINT32_C(0xcccccccc)
#elif defined(UINT64_C) && defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
#define INIT_THIS(frame) frame.cf_this = (DeeObject *)UINT64_C(0xcccccccccccccccc)
#else /* ... */
#define INIT_THIS(frame) DBG_memset(&frame.cf_this, 0xcc, sizeof(DeeObject *))
#endif /* !... */


INTERN WUNUSED
#ifdef CALL_TUPLE
#ifdef CALL_THIS
	NONNULL((1, 2, 3))
#else /* CALL_THIS */
	NONNULL((1, 2))
#endif /* !CALL_THIS */
#else /* CALL_TUPLE */
#ifdef CALL_THIS
	NONNULL((1, 2))
#else /* CALL_THIS */
	NONNULL((1))
#endif /* !CALL_THIS */
#endif /* !CALL_TUPLE */
DREF DeeObject *DCALL
LOCAL_DeeFunction_Call(DeeFunctionObject *self
#ifdef CALL_THIS
                       , DeeObject *this_arg
#endif /* CALL_THIS */
#ifdef CALL_TUPLE
#define GET_ARGC() DeeTuple_SIZE(args)
#define GET_ARGV() DeeTuple_ELEM(args)
                       , DeeObject *args
#else /* CALL_TUPLE */
#define GET_ARGC() argc
#define GET_ARGV() argv
                       , size_t argc, DeeObject *const *argv
#endif /* !CALL_TUPLE */
#ifdef CALL_KW
                       , DeeObject *kw
#endif /* CALL_KW */
                       )
{
	DREF DeeObject *result;
	DeeCodeObject *code;
	struct code_frame frame;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeFunction_Type);
#ifdef CALL_THIS
	ASSERT_OBJECT(this_arg);
#endif /* CALL_THIS */
	code = self->fo_code;

	/* Handle miss-match the THISCALL calling behavior. */
#ifdef CALL_THIS
	if unlikely(!(code->co_flags & CODE_FTHISCALL)) {
		DREF DeeTupleObject *packed_args;

		/* Re-package the argument tuple and perform a regular call. */
		packed_args = DeeTuple_NewUninitialized(1 + GET_ARGC());
		if unlikely(!packed_args)
			goto err;
		DeeTuple_SET(packed_args, 0, this_arg);
		memcpyc(DeeTuple_ELEM(packed_args) + 1,
		        GET_ARGV(), GET_ARGC(),
		        sizeof(DeeObject *));

		/* Perform a regular callback. */
#ifdef CALL_KW
		result = DeeFunction_CallTupleKw(self, (DeeObject *)packed_args, kw);
#else /* CALL_KW */
		result = DeeFunction_CallTuple(self, (DeeObject *)packed_args);
#endif /* !CALL_KW */

		/* The tuple we've created above only contained symbolic references. */
		DeeTuple_DecrefSymbolic((DeeObject *)packed_args);
		return result;
	}
#else /* CALL_THIS */
	if unlikely(code->co_flags & CODE_FTHISCALL) {
		/* Special case: Invoke the function as a this-call. */
		if unlikely(!GET_ARGC()) {
			err_invalid_argc(DeeCode_NAME(code), 0, code->co_argc_min + 1,
			                 code->co_flags & CODE_FVARARGS
			                 ? (size_t)-1
			                 : ((size_t)code->co_argc_max + 1));
			goto err;
		}
#ifdef CALL_KW
		return DeeFunction_ThisCallKw(self,
		                              GET_ARGV()[0],
		                              GET_ARGC() - 1,
		                              GET_ARGV() + 1,
		                              kw);
#else /* CALL_KW */
		return DeeFunction_ThisCall(self,
		                            GET_ARGV()[0],
		                            GET_ARGC() - 1,
		                            GET_ARGV() + 1);
#endif /* !CALL_KW */
	}
#endif /* !CALL_THIS */

	/* Check if the Function object has been re-compiled into host assembly. */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#if defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_tuple_kw.c_this)
		return (*self->fo_hostasm.hafu_call_tuple_kw.c_this)(this_arg, args, kw);
#elif defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_kw.c_this)
		return (*self->fo_hostasm.hafu_call_kw.c_this)(this_arg, argc, argv, kw);
#elif defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_tuple.c_this)
		return (*self->fo_hostasm.hafu_call_tuple.c_this)(this_arg, args);
#elif defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call.c_this)
		return (*self->fo_hostasm.hafu_call.c_this)(this_arg, argc, argv);
#elif !defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_tuple_kw.c_norm)
		return (*self->fo_hostasm.hafu_call_tuple_kw.c_norm)(args, kw);
#elif !defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_kw.c_norm)
		return (*self->fo_hostasm.hafu_call_kw.c_norm)(argc, argv, kw);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_tuple.c_norm)
		return (*self->fo_hostasm.hafu_call_tuple.c_norm)(args);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call.c_norm)
		return (*self->fo_hostasm.hafu_call.c_norm)(argc, argv);
#endif /* ... */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */

	/* Check if the Code object has been re-compiled into host assembly. */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#if defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_tuple_kw.c_this)
		return (*code->co_hostasm.haco_call_tuple_kw.c_this)(self, this_arg, args, kw);
#elif defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_kw.c_this)
		return (*code->co_hostasm.haco_call_kw.c_this)(self, this_arg, argc, argv, kw);
#elif defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_tuple.c_this)
		return (*code->co_hostasm.haco_call_tuple.c_this)(self, this_arg, args);
#elif defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call.c_this)
		return (*code->co_hostasm.haco_call.c_this)(self, this_arg, argc, argv);
#elif !defined(CALL_THIS) && defined(CALL_KW) && defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_tuple_kw.c_norm)
		return (*code->co_hostasm.haco_call_tuple_kw.c_norm)(self, args, kw);
#elif !defined(CALL_THIS) && defined(CALL_KW) && !defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_kw.c_norm)
		return (*code->co_hostasm.haco_call_kw.c_norm)(self, argc, argv, kw);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_tuple.c_norm)
		return (*code->co_hostasm.haco_call_tuple.c_norm)(self, args);
#elif !defined(CALL_THIS) && !defined(CALL_KW) && !defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call.c_norm)
		return (*code->co_hostasm.haco_call.c_norm)(self, argc, argv);
#endif /* ... */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */

#ifdef CALL_KW
	if (kw) {
		DREF DeeYieldFunctionObject *yf;
		size_t i;
		size_t kw_argc; /* # of keyword arguments passed (DeeKwds_SIZE(kw)). */
		size_t ex_argc; /* # of objects in the keyword-overlay vector (code->co_argc_max - frame.cf_argc) */
		size_t kw_used; /* # of keyword arguments that have been loaded from `kw'.
		                 * NOTE: Once all provided arguments have been loaded, this is used
		                 *       to check if _all_ keywords have actually been used, which
		                 *       is a requirement when `CODE_FVARKWDS' isn't set. */
	
		/* Keep track of metrics. */
#ifdef CONFIG_HAVE_CODE_METRICS
#ifdef CALL_TUPLE
		atomic_inc(&code->co_metrics.com_call_tuple_kw);
#else /* CALL_TUPLE */
		atomic_inc(&code->co_metrics.com_call_kw);
#endif /* !CALL_TUPLE */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#ifdef CALL_TUPLE
		if unlikely(code->co_metrics.com_call_tuple_kw > DeeCode_OptimizeCallThreshold)
#else /* CALL_TUPLE */
		if unlikely(code->co_metrics.com_call_kw > DeeCode_OptimizeCallThreshold)
#endif /* !CALL_TUPLE */
		{
			if (!(code->co_flags & CODE_FNOOPTIMIZE)) {
#if defined(CALL_TUPLE) && defined(CALL_THIS)
				return DeeFunction_OptimizeAndThisCallTupleKw(self, this_arg, args, kw);
#elif defined(CALL_TUPLE) && !defined(CALL_THIS)
				return DeeFunction_OptimizeAndCallTupleKw(self, args, kw);
#elif !defined(CALL_TUPLE) && defined(CALL_THIS)
				return DeeFunction_OptimizeAndThisCallKw(self, this_arg, argc, argv, kw);
#elif !defined(CALL_TUPLE) && !defined(CALL_THIS)
				return DeeFunction_OptimizeAndCallKw(self, argc, argv, kw);
#endif /* ... */
			}
		}
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#endif /* CONFIG_HAVE_CODE_METRICS */

#ifdef Dee_Alloca
#define err_ex_frame err
#endif /* Dee_Alloca */
#ifndef __INTELLISENSE__
		switch (code->co_flags & (CODE_FVARKWDS | CODE_FYIELDING)) {

		case 0:
			if likely(DeeKwds_Check(kw)) {
#define CODE_FLAGS    0
#include "code-invoke-kw.c.inl"
			} else {
#define KW_IS_MAPPING 1
#define CODE_FLAGS    0
#include "code-invoke-kw.c.inl"
			}
			break;

		case CODE_FVARKWDS:
			if likely(DeeKwds_Check(kw)) {
#define CODE_FLAGS    CODE_FVARKWDS
#include "code-invoke-kw.c.inl"
			} else {
#define KW_IS_MAPPING 1
#define CODE_FLAGS    CODE_FVARKWDS
#include "code-invoke-kw.c.inl"
			}
			break;

		case CODE_FYIELDING:
			if likely(DeeKwds_Check(kw)) {
#define CODE_FLAGS    CODE_FYIELDING
#include "code-invoke-kw.c.inl"
			} else {
#define KW_IS_MAPPING 1
#define CODE_FLAGS    CODE_FYIELDING
#include "code-invoke-kw.c.inl"
			}
			break;

		case CODE_FVARKWDS | CODE_FYIELDING:
			if likely(DeeKwds_Check(kw)) {
#define CODE_FLAGS   (CODE_FVARKWDS | CODE_FYIELDING)
#include "code-invoke-kw.c.inl"
			} else {
#define KW_IS_MAPPING 1
#define CODE_FLAGS   (CODE_FVARKWDS | CODE_FYIELDING)
#include "code-invoke-kw.c.inl"
			}
			break;

		default:
			__builtin_unreachable();
		}
#endif /* !__INTELLISENSE__ */
		return result;
err_ex_frame_full:
		Dee_Free(frame.cf_kw);
		goto err;
#ifndef Dee_Alloca
err_ex_frame:
		Dee_Free((void *)((uintptr_t)frame.cf_kw + offsetof(struct code_frame_kwds, fk_kargv)));
		goto err;
#else /* !Dee_Alloca */
#undef err_ex_frame
#endif /* Dee_Alloca */
	}

	/* Check if the Function object was hostasm recompiled w/o keyword arguments */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#if defined(CALL_THIS) && defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_tuple.c_this)
		return (*self->fo_hostasm.hafu_call_tuple.c_this)(this_arg, args);
#elif defined(CALL_THIS) && !defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call.c_this)
		return (*self->fo_hostasm.hafu_call.c_this)(this_arg, argc, argv);
#elif !defined(CALL_THIS) && defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call_tuple.c_norm)
		return (*self->fo_hostasm.hafu_call_tuple.c_norm)(args);
#elif !defined(CALL_THIS) && !defined(CALL_TUPLE)
	if (self->fo_hostasm.hafu_call.c_norm)
		return (*self->fo_hostasm.hafu_call.c_norm)(argc, argv);
#endif /* ... */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */

	/* Check if the Code object was hostasm recompiled w/o keyword arguments */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#if defined(CALL_THIS) && defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_tuple.c_this)
		return (*code->co_hostasm.haco_call_tuple.c_this)(self, this_arg, args);
#elif defined(CALL_THIS) && !defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call.c_this)
		return (*code->co_hostasm.haco_call.c_this)(self, this_arg, argc, argv);
#elif !defined(CALL_THIS) && defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call_tuple.c_norm)
		return (*code->co_hostasm.haco_call_tuple.c_norm)(self, args);
#elif !defined(CALL_THIS) && !defined(CALL_TUPLE)
	if (code->co_hostasm.haco_call.c_norm)
		return (*code->co_hostasm.haco_call.c_norm)(self, argc, argv);
#endif /* ... */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#endif /* CALL_KW */

	/* Keep track of metrics. */
#ifdef CONFIG_HAVE_CODE_METRICS
#ifdef CALL_TUPLE
	atomic_inc(&code->co_metrics.com_call_tuple);
#else /* CALL_TUPLE */
	atomic_inc(&code->co_metrics.com_call);
#endif /* !CALL_TUPLE */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#ifdef CALL_TUPLE
	if unlikely(code->co_metrics.com_call_tuple > DeeCode_OptimizeCallThreshold)
#else /* CALL_TUPLE */
	if unlikely(code->co_metrics.com_call > DeeCode_OptimizeCallThreshold)
#endif /* !CALL_TUPLE */
	{
		if (!(code->co_flags & CODE_FNOOPTIMIZE)) {
#if defined(CALL_TUPLE) && defined(CALL_THIS)
			return DeeFunction_OptimizeAndThisCallTuple(self, this_arg, args);
#elif defined(CALL_TUPLE) && !defined(CALL_THIS)
			return DeeFunction_OptimizeAndCallTuple(self, args);
#elif !defined(CALL_TUPLE) && defined(CALL_THIS)
			return DeeFunction_OptimizeAndThisCall(self, this_arg, argc, argv);
#elif !defined(CALL_TUPLE) && !defined(CALL_THIS)
			return DeeFunction_OptimizeAndCall(self, argc, argv);
#endif /* ... */
		}
	}
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#endif /* CONFIG_HAVE_CODE_METRICS */


	if unlikely(GET_ARGC() < code->co_argc_min ||
	            (GET_ARGC() > code->co_argc_max &&
	             !(code->co_flags & CODE_FVARARGS))) {
		/* ERROR: Invalid argument count! */
		err_invalid_argc(DeeCode_NAME(code),
		                 GET_ARGC(), code->co_argc_min,
		                 code->co_flags & CODE_FVARARGS
		                 ? (size_t)-1
		                 : (size_t)code->co_argc_max);
		goto err;
	}

	if (!(code->co_flags & CODE_FYIELDING)) {
		/* Default scenario: Perform a this-call. */
		frame.cf_func   = self;
		frame.cf_argc   = GET_ARGC();
		frame.cf_argv   = GET_ARGV();
		frame.cf_result = NULL;
		frame.cf_kw     = NULL;
#ifdef Dee_Alloca
		if (!(code->co_flags & CODE_FHEAPFRAME)) {
			frame.cf_frame = (DeeObject **)Dee_Alloca(code->co_framesize);
		} else
#endif /* Dee_Alloca */
		{
			frame.cf_frame = (DeeObject **)Dee_Malloc(code->co_framesize);
			if unlikely(!frame.cf_frame)
				goto err;
		}

		/* Per-initialize local variable memory to ZERO. */
		bzeroc(frame.cf_frame,
		       code->co_localc,
		       sizeof(DREF DeeObject *));
#ifndef NDEBUG
		frame.cf_prev = CODE_FRAME_NOT_EXECUTING;
#endif /* !NDEBUG */
		frame.cf_stack = frame.cf_frame + code->co_localc;
		frame.cf_sp    = frame.cf_stack;
		frame.cf_ip    = code->co_code;
		frame.cf_vargs = NULL;
		INIT_THIS(frame);

#ifdef CALL_TUPLE
		/* Optimization: When the function only uses variable arguments, we can
		 *               forward the caller's argument tuple, if we've been given it. */
		if (code->co_argc_max == 0)
			frame.cf_vargs = (DREF DeeTupleObject *)args;
#endif /* CALL_TUPLE */

		/* With the frame now set up, actually invoke the code. */
		if unlikely(code->co_flags & CODE_FASSEMBLY) {
			frame.cf_stacksz = 0;
			result = DeeCode_ExecFrameSafe(&frame);

			/* Delete remaining stack objects. */
			while (frame.cf_sp > frame.cf_stack) {
				--frame.cf_sp;
				Dee_Decref(*frame.cf_sp);
			}

			/* Safe code execution allows for stack-space extension into heap memory.
			 * >> Free that memory now that `DeeCode_ExecFrameSafe()' has finished. */
			if (frame.cf_stacksz)
				Dee_Free(frame.cf_stack);
			frame.cf_sp = frame.cf_frame + code->co_localc;
		} else {
			result = DeeCode_ExecFrameFast(&frame);

			/* Delete remaining stack objects. */
			while (frame.cf_sp > frame.cf_stack) {
				--frame.cf_sp;
				Dee_Decref(*frame.cf_sp);
			}
		}

		/* Delete remaining local variables. */
		while (frame.cf_sp > frame.cf_frame) {
			--frame.cf_sp;
			Dee_XDecref(*frame.cf_sp);
		}

#ifdef Dee_Alloca
		if (code->co_flags & CODE_FHEAPFRAME)
#endif /* Dee_Alloca */
		{
			Dee_Free(frame.cf_frame);
		}

#ifdef CALL_TUPLE
		if (code->co_argc_max != 0)
#endif /* CALL_TUPLE */
		{
			Dee_XDecref(frame.cf_vargs);
		}
		return result;
	}

	/* Special case: Create a yield-function callback. */
	{
		DREF DeeYieldFunctionObject *yf;
		yf = (DREF DeeYieldFunctionObject *)DeeObject_Malloc(DeeYieldFunction_Sizeof(GET_ARGC()));
		if unlikely(!yf)
			goto err;
		yf->yf_func = self;
		Dee_Incref(self);
		yf->yf_argc = yf->yf_pargc = GET_ARGC();
		Dee_Movrefv(yf->yf_argv, GET_ARGV(), GET_ARGC());
#ifdef CALL_THIS
		yf->yf_this = this_arg;
		Dee_Incref(this_arg);
#else /* CALL_THIS */
		yf->yf_this = NULL;
#endif /* !CALL_THIS */
		yf->yf_kw = NULL;
		DeeObject_Init(yf, &DeeYieldFunction_Type);
		return (DREF DeeObject *)yf;
	}
err:
	return NULL;
}

#ifndef __INTELLISENSE__
#undef INIT_THIS
#undef GET_ARGC
#undef GET_ARGV
#undef LOCAL_DeeFunction_Call
#undef LOCAL_DeeFunction_OptimizeAndCall
#undef CALL_KW
#undef CALL_THIS
#undef CALL_TUPLE
#endif /* !__INTELLISENSE__ */

DECL_END
