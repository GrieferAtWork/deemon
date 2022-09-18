/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
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

#include "../objects/seq/varkwds.h"

DECL_BEGIN

#ifdef CALL_THIS
#ifdef CALL_KW
#ifdef CALL_TUPLE
#define MY_FUNCTION_NAME DeeFunction_ThisCallTupleKw
#else /* CALL_TUPLE */
#define MY_FUNCTION_NAME DeeFunction_ThisCallKw
#endif /* !CALL_TUPLE */
#else /* CALL_KW */
#ifdef CALL_TUPLE
#define MY_FUNCTION_NAME DeeFunction_ThisCallTuple
#else /* CALL_TUPLE */
#define MY_FUNCTION_NAME DeeFunction_ThisCall
#endif /* !CALL_TUPLE */
#endif /* !CALL_KW */
#else /* CALL_THIS */
#ifdef CALL_KW
#ifdef CALL_TUPLE
#define MY_FUNCTION_NAME DeeFunction_CallTupleKw
#else /* CALL_TUPLE */
#define MY_FUNCTION_NAME DeeFunction_CallKw
#endif /* !CALL_TUPLE */
#else /* CALL_KW */
#ifdef CALL_TUPLE
#define MY_FUNCTION_NAME DeeFunction_CallTuple
#else /* CALL_TUPLE */
#define MY_FUNCTION_NAME DeeFunction_Call
#endif /* !CALL_TUPLE */
#endif /* !CALL_KW */
#endif /* !CALL_THIS */



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


#ifdef CALL_KW
#ifndef KWDS_FIND_INDEX_DEFINED
#define KWDS_FIND_INDEX_DEFINED 1
LOCAL size_t DCALL
kwds_find_index(DeeKwdsObject *__restrict self,
                DeeStringObject *__restrict name) {
	dhash_t i, perturb, hash;
	hash    = DeeString_Hash((DeeObject *)name);
	perturb = i = hash & self->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		struct kwds_entry *entry;
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (DeeString_SIZE(entry->ke_name) != DeeString_SIZE(name))
			continue;
		if (bcmpc(DeeString_STR(entry->ke_name),
		          DeeString_STR(name),
		          DeeString_SIZE(name),
		          sizeof(char)) != 0)
			continue;
		return entry->ke_index;
	}
	return (size_t)-1;
}
#endif /* !KWDS_FIND_INDEX_DEFINED */
#endif /* CALL_KW */


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
MY_FUNCTION_NAME(DeeFunctionObject *self
#ifdef CALL_THIS
                 ,
                 DeeObject *this_arg
#endif /* CALL_THIS */
#ifdef CALL_TUPLE
#define GET_ARGC() DeeTuple_SIZE(args)
#define GET_ARGV() DeeTuple_ELEM(args)
                 ,
                 DeeObject *args
#else /* CALL_TUPLE */
#define GET_ARGC() argc
#define GET_ARGV() argv
                 ,
                 size_t argc,
                 DeeObject *const *argv
#endif /* !CALL_TUPLE */
#ifdef CALL_KW
                 ,
                 DeeObject *kw
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
#ifdef CALL_THIS
	if unlikely(!(code->co_flags & CODE_FTHISCALL)) {
		DREF DeeObject *packed_args;
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
		result = DeeFunction_CallTupleKw(self,
		                                 packed_args,
		                                 kw);
#else /* CALL_KW */
		result = DeeFunction_CallTuple(self,
		                               packed_args);
#endif /* !CALL_KW */
		/* The tuple we've created above only contained symbolic references. */
		DeeTuple_DecrefSymbolic(packed_args);
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
#endif /* CALL_KW */

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
			result           = DeeCode_ExecFrameSafe(&frame);
			/* Delete remaining stack objects. */
			while (frame.cf_sp != frame.cf_stack) {
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
			while (frame.cf_sp != frame.cf_stack) {
				--frame.cf_sp;
				Dee_Decref(*frame.cf_sp);
			}
		}
		/* Delete remaining local variables. */
		while (frame.cf_sp != frame.cf_frame) {
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
		yf = DeeObject_MALLOC(DeeYieldFunctionObject);
		if unlikely(!yf)
			goto err;
		yf->yf_func = self;
		Dee_Incref(self);
		/* Pack together an argument tuple for the yield-function. */
#ifdef CALL_TUPLE
		yf->yf_args = (DREF DeeTupleObject *)args;
		Dee_Incref(args);
#else /* CALL_TUPLE */
		yf->yf_args = (DREF DeeTupleObject *)DeeTuple_NewVector(GET_ARGC(),
		                                                        GET_ARGV());
		if unlikely(!yf->yf_args)
			goto err_r;
#endif /* !CALL_TUPLE */
#ifdef CALL_THIS
		yf->yf_this = this_arg;
		Dee_Incref(this_arg);
#else /* CALL_THIS */
		yf->yf_this = NULL;
#endif /* !CALL_THIS */
		yf->yf_kw = NULL;
		DeeObject_Init(yf, &DeeYieldFunction_Type);
		return (DREF DeeObject *)yf;
#ifndef CALL_TUPLE
err_r:
		DeeObject_FREE(yf);
#endif /* !CALL_TUPLE */
	}
err:
	return NULL;
}

#ifndef __INTELLISENSE__
#undef INIT_THIS
#undef GET_ARGC
#undef GET_ARGV
#undef MY_FUNCTION_NAME
#undef CALL_KW
#undef CALL_THIS
#undef CALL_TUPLE
#endif /* !__INTELLISENSE__ */

DECL_END
