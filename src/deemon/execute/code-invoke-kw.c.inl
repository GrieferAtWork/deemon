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
#ifdef __INTELLISENSE__
#include "code-invoke.c.inl"
#define CODE_FLAGS   (CODE_FYIELDING)
//#define KW_IS_MAPPING 1
#endif /* __INTELLISENSE__ */

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* UINT32_C, UINT64_C, uintptr_t */


#if (CODE_FLAGS & ~(CODE_FVARKWDS | CODE_FYIELDING)) != 0
#error "Unsupported code flags for keyword invocation (must be a set of `CODE_FVARKWDS | CODE_FYIELDING')"
#endif /* (CODE_FLAGS & ~(CODE_FVARKWDS | CODE_FYIELDING)) != 0 */


#ifdef KW_IS_MAPPING
#if CODE_FLAGS == 0
#define UNIQUE(x) x##KW0
#elif CODE_FLAGS == CODE_FVARKWDS
#define UNIQUE(x) x##KWV
#elif CODE_FLAGS == CODE_FYIELDING
#define UNIQUE(x) x##KWY
#elif CODE_FLAGS == (CODE_FVARKWDS | CODE_FYIELDING)
#define UNIQUE(x) x##KWVY
#else
#error "Unsupported combination of flags"
#endif
#else /* KW_IS_MAPPING */
#if CODE_FLAGS == 0
#define UNIQUE(x) x##0
#elif CODE_FLAGS == CODE_FVARKWDS
#define UNIQUE(x) x##V
#elif CODE_FLAGS == CODE_FYIELDING
#define UNIQUE(x) x##Y
#elif CODE_FLAGS == (CODE_FVARKWDS | CODE_FYIELDING)
#define UNIQUE(x) x##VY
#else
#error "Unsupported combination of flags"
#endif
#endif /* !KW_IS_MAPPING */


#ifdef __INTELLISENSE__
DECL_BEGIN

INTERN WUNUSED DREF DeeObject *DCALL
PP_CAT2(LOCAL_DeeFunction_Call, IntellisenseInternal)
                (DeeFunctionObject *__restrict self
#ifdef CALL_THIS
                 , DeeObject *__restrict this_arg
#endif /* CALL_THIS */
#ifdef CALL_TUPLE
#define GET_ARGC() DeeTuple_SIZE(args)
#define GET_ARGV() DeeTuple_ELEM(args)
                 , DeeObject *__restrict args
#else /* CALL_TUPLE */
#define GET_ARGC() argc
#define GET_ARGV() argv
                 , size_t argc
                 , DeeObject *const *argv
#endif /* !CALL_TUPLE */
#ifdef CALL_KW
                 , DeeObject *kw
#endif /* CALL_KW */
                 )
#endif /* __INTELLISENSE__ */
{
#ifdef __INTELLISENSE__
	DREF DeeYieldFunctionObject *yf;
	DREF DeeObject *result;
	DeeCodeObject *code;
	struct code_frame frame;
	size_t i;
	size_t kw_argc;      /* # of keyword arguments passed (DeeKwds_SIZE(kw)). */
	size_t ex_argc;      /* # of objects in the keyword-overlay vector (code->co_argc_max - frame.cf_argc) */
	size_t kw_used;      /* # of keyword arguments that have been loaded from `kw'.
	                      * NOTE: Once all provided arguments have been loaded, this is used
	                      *       to check if _all_ keywords have actually been used, which
	                      *       is a requirement when `CODE_FVARKWDS' isn't set. */
#define err_ex_frame  err
#endif /* __INTELLISENSE__ */
#if CODE_FLAGS & CODE_FYIELDING
#undef err_ex_frame
#define err_ex_frame      err_ex_frame_full
#elif (CODE_FLAGS & (CODE_FYIELDING | CODE_FVARKWDS)) == CODE_FVARKWDS
#undef err_ex_frame
#ifdef Dee_Alloca
#define err_ex_frame      err
#else /* Dee_Alloca */
#define err_ex_frame      err_ex_frame_full
#endif /* !Dee_Alloca */
#endif
#ifdef KW_IS_MAPPING
	frame.cf_argc = GET_ARGC();
#else /* KW_IS_MAPPING */
	kw_argc = DeeKwds_SIZE(kw);
	if unlikely(kw_argc > GET_ARGC()) {
		/* Argument list is too short of the given keywords */
		err_keywords_bad_for_argc((DeeKwdsObject *)kw, GET_ARGC(), GET_ARGV());
		goto err;
	}
	frame.cf_argc = GET_ARGC() - kw_argc; /* # of positional, non-keyword arguments. */
#endif /* !KW_IS_MAPPING */

	if unlikely(frame.cf_argc > code->co_argc_max) {
		if (!(code->co_flags & CODE_FVARARGS)) {
			/* ERROR: Too many positional arguments. */
			err_invalid_argc(DeeCode_NAME(code),
			                 frame.cf_argc,
			                 code->co_argc_min,
			                 code->co_argc_max);
			goto err;
		}
		ex_argc = 0;
	} else {
		ex_argc = code->co_argc_max - frame.cf_argc;
	}

	/* Allocate the frame extension for storing keyword arguments.
	 * This needs to be done before we check if all required arguments
	 * have been given, since keyword arguments are allowed to substitute
	 * ones that are positional. */
#if CODE_FLAGS & (CODE_FYIELDING | CODE_FVARKWDS)
#if defined(Dee_Alloca) && !(CODE_FLAGS & CODE_FYIELDING)
	frame.cf_kw = (struct code_frame_kwds *)Dee_Allocaoc(offsetof(struct code_frame_kwds, fk_kargv),
	                                                     ex_argc, sizeof(DeeObject *));
#else /* Dee_Alloca && !(CODE_FLAGS & CODE_FYIELDING) */
	frame.cf_kw = (struct code_frame_kwds *)Dee_Mallococ(offsetof(struct code_frame_kwds, fk_kargv),
	                                                     ex_argc, sizeof(DeeObject *));
	if unlikely(!frame.cf_kw)
		goto err;
#endif /* !Dee_Alloca || (CODE_FLAGS & CODE_FYIELDING) != 0 */
#else /* CODE_FLAGS & (CODE_FYIELDING | CODE_FVARKWDS) */
#ifdef Dee_Allocac
	frame.cf_kw = (struct code_frame_kwds *)((uintptr_t)Dee_Allocac(ex_argc, sizeof(DeeObject *)) -
	                                         offsetof(struct code_frame_kwds, fk_kargv));
#else /* Dee_Allocac */
	frame.cf_kw = (struct code_frame_kwds *)Dee_Mallocc(ex_argc, sizeof(DeeObject *));
	if unlikely(!frame.cf_kw)
		goto err;
	frame.cf_kw = (struct code_frame_kwds *)((uintptr_t)frame.cf_kw - offsetof(struct code_frame_kwds, fk_kargv));
#endif /* !Dee_Allocac */
#endif /* !(CODE_FLAGS & (CODE_FYIELDING | CODE_FVARKWDS)) */

	i = 0;
#if !(CODE_FLAGS & CODE_FVARKWDS)
	kw_used = 0;
#endif /* !(CODE_FLAGS & CODE_FVARKWDS) */
	if (frame.cf_argc < code->co_argc_min) {
		/* Mandatory arguments must be loaded from kwargs. */
		size_t mand_kwds = code->co_argc_min - frame.cf_argc;
		for (; i < mand_kwds; ++i) {
#ifdef KW_IS_MAPPING
			DREF DeeObject *val;
#else /* KW_IS_MAPPING */
			size_t index;
			DeeObject *val;
#endif /* !KW_IS_MAPPING */
			DeeStringObject *name = code->co_keywords[frame.cf_argc + i];
			ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
#ifdef KW_IS_MAPPING
			val = DeeKw_TryGetItemNR(kw, Dee_AsObject(name));
			if unlikely(!ITER_ISOK(val)) {
				if (val != NULL) {
					/* Missing, mandatory argument. */
					err_invalid_argc_missing_kw(DeeString_STR(name),
					                            DeeCode_NAME(code),
					                            GET_ARGC(),
					                            code->co_argc_min,
					                            code->co_argc_max);
				}
				goto err_ex_frame;
			}
#else /* KW_IS_MAPPING */
			index = DeeKwds_IndexOf(kw, Dee_AsObject(name));
			if unlikely(index == (size_t)-1) {
				/* Missing, mandatory argument. */
				err_invalid_argc_missing_kw(DeeString_STR(name),
				                            DeeCode_NAME(code),
				                            GET_ARGC(),
				                            code->co_argc_min,
				                            code->co_argc_max);
				goto err_ex_frame;
			}
			val = (GET_ARGV() + frame.cf_argc)[index];
			ASSERT_OBJECT(val);
#endif /* !KW_IS_MAPPING */
			frame.cf_kw->fk_kargv[i] = val;
#if !(CODE_FLAGS & CODE_FVARKWDS)
			++kw_used;
#endif /* !(CODE_FLAGS & CODE_FVARKWDS) */
		}
	}
	for (; i < ex_argc; ++i) {
#ifdef KW_IS_MAPPING
		DREF DeeObject *val;
#else /* KW_IS_MAPPING */
		size_t index;
		DeeObject *val;
#endif /* !KW_IS_MAPPING */
		DeeStringObject *name = code->co_keywords[frame.cf_argc + i];
		ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
#ifdef KW_IS_MAPPING
		val = DeeKw_TryGetItemNR(kw, Dee_AsObject(name));
		if (!ITER_ISOK(val)) {
			if unlikely(!val)
				goto err_ex_frame;
			frame.cf_kw->fk_kargv[i] = NULL; /* Unset value. */
			continue;
		}
#else /* KW_IS_MAPPING */
		index = DeeKwds_IndexOf(kw, Dee_AsObject(name));
		if (index == (size_t)-1) {
			/* Missing argument (leave as unbound / allow fallthrough to argument defaults) */
			frame.cf_kw->fk_kargv[i] = NULL;
			continue;
		}
		val = (GET_ARGV() + frame.cf_argc)[index];
		ASSERT_OBJECT(val);
#endif /* !KW_IS_MAPPING */
		frame.cf_kw->fk_kargv[i] = val;
#if !(CODE_FLAGS & CODE_FVARKWDS)
		++kw_used;
#endif /* !(CODE_FLAGS & CODE_FVARKWDS) */
	}
#if CODE_FLAGS & CODE_FVARKWDS
	/* Lazily initialized. */
	frame.cf_kw->fk_varkwds = NULL;
	frame.cf_kw->fk_kw      = kw;
#else /* CODE_FLAGS & CODE_FVARKWDS */
	/* Check to make sure that all arguments have been used. */
#ifdef KW_IS_MAPPING
	kw_argc = DeeObject_Size(kw);
	if unlikely(kw_argc == (size_t)-1)
		goto err_ex_frame;
#endif /* KW_IS_MAPPING */
	if unlikely(kw_used < kw_argc) {
		/* ERROR: Not all keyword arguments have been used. */
		/* TODO: Include the names of the unused arguments here! */
		err_invalid_argc(DeeCode_NAME(code),
		                 GET_ARGC(),
		                 code->co_argc_min,
		                 code->co_argc_max);
		goto err_ex_frame;
	}
#endif /* !(CODE_FLAGS & CODE_FVARKWDS) */
#if CODE_FLAGS & CODE_FYIELDING
	/* Yield-function invocation. */
	yf = (DREF DeeYieldFunctionObject *)DeeObject_Malloc(DeeYieldFunction_Sizeof(GET_ARGC()));
	if unlikely(!yf) {
		/*Dee_XDecref(frame.cf_kw->fk_varkwds);*/
		goto err_ex_frame;
	}
	yf->yf_func = self;
	Dee_Incref(self);

	/* Pack together an argument tuple for the yield-function. */
	yf->yf_pargc = frame.cf_argc;
	yf->yf_argc  = GET_ARGC();
	Dee_Movrefv(yf->yf_argv, GET_ARGV(), GET_ARGC());
#ifdef CALL_THIS
	yf->yf_this = this_arg;
	Dee_Incref(this_arg);
#else /* CALL_THIS */
	yf->yf_this = NULL;
#endif /* !CALL_THIS */

	/* Initialize references stored within the keyword argument extension. */
#if CODE_FLAGS & CODE_FVARKWDS
	/* ... */
#elif defined(NDEBUG)
	/* ... */
#elif __SIZEOF_POINTER__ == 4 && defined(UINT32_C)
	frame.cf_kw->fk_varkwds = (DREF DeeObject *)UINT32_C(0xcccccccc);
#elif __SIZEOF_POINTER__ == 8 && defined(UINT64_C)
	frame.cf_kw->fk_varkwds = (DREF DeeObject *)UINT64_C(0xcccccccccccccccc);
#else /* __SIZEOF_POINTER__... */
	DBG_memset(&frame.cf_kw->fk_varkwds, 0xcc, sizeof(void *));
#endif /* !__SIZEOF_POINTER__... */
#if CODE_FLAGS & CODE_FYIELDING
	frame.cf_kw->fk_kw = kw;
	Dee_Incref(kw); /* Always a reference when yielding */
#elif CODE_FLAGS & CODE_FVARKWDS
	ASSERT(frame.cf_kw->fk_kw == kw);
	Dee_Incref(kw); /* The reference stored in `frame.cf_kw->fk_kw' (Only valid when `CODE_FVARKWDS') */
#endif /* CODE_FLAGS & CODE_FVARKWDS ) */
	yf->yf_kw = frame.cf_kw; /* Inherit data. */
	DeeObject_Init(yf, &DeeYieldFunction_Type);
	return Dee_AsObject(yf);
#else /* CODE_FLAGS & CODE_FYIELDING */
	/* Direct function invocation */
#ifdef Dee_Alloca
	if (!(code->co_flags & CODE_FHEAPFRAME)) {
		frame.cf_frame = (DeeObject **)Dee_Alloca(code->co_framesize);
	} else
#endif /* Dee_Alloca */
	{
		frame.cf_frame = (DeeObject **)Dee_Malloc(code->co_framesize);
		if unlikely(!frame.cf_frame) {
#if CODE_FLAGS & CODE_FVARKWDS
			/*Dee_XDecref(frame.cf_kw->fk_varkwds);*/
#endif /* CODE_FLAGS & CODE_FVARKWDS */
			goto err_ex_frame;
		}
	}
	/* Per-initialize local variable memory to ZERO. */
	bzeroc(frame.cf_frame,
	       code->co_localc,
	       sizeof(DREF DeeObject *));
#ifndef NDEBUG
	frame.cf_prev  = CODE_FRAME_NOT_EXECUTING;
#endif /* !NDEBUG */
	frame.cf_stack = frame.cf_frame + code->co_localc;
	frame.cf_sp    = frame.cf_stack;
	frame.cf_ip    = code->co_code;
	frame.cf_vargs = NULL;
#ifdef CALL_TUPLE
#ifdef KW_IS_MAPPING
	if (!code->co_argc_max)
#else /* KW_IS_MAPPING */
	if (!code->co_argc_max && !kw_argc)
#endif /* !KW_IS_MAPPING */
	{
		/* Can directly forward varargs passed by the caller! */
		frame.cf_vargs = (DREF struct tuple_object *)args;
	}
#endif /* CALL_TUPLE */
	frame.cf_result = NULL;
	frame.cf_func   = self;
	frame.cf_argv   = GET_ARGV();
	INIT_THIS(frame);

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

#if CODE_FLAGS & CODE_FVARKWDS
	if (frame.cf_kw->fk_varkwds)
		DeeKwBlackList_Decref(frame.cf_kw->fk_varkwds);
#endif /* CODE_FLAGS & CODE_FVARKWDS */

#ifdef Dee_Alloca
	if (code->co_flags & CODE_FHEAPFRAME)
#endif /* Dee_Alloca */
	{
		Dee_Free(frame.cf_frame);
	}

#ifdef CALL_TUPLE
	if (frame.cf_vargs != (DREF struct tuple_object *)args)
#endif /* CALL_TUPLE */
	{
		Dee_XDecref(frame.cf_vargs);
	}

	/* Cleanup keyword extension data. */
#ifndef Dee_Alloca
#if CODE_FLAGS & CODE_FVARKWDS
	Dee_Free(frame.cf_kw);
#else /* CODE_FLAGS & CODE_FVARKWDS */
	Dee_Free((void *)((uintptr_t)frame.cf_kw + offsetof(struct code_frame_kwds, fk_kargv)));
#endif /* !(CODE_FLAGS & CODE_FVARKWDS) */
#endif /* !Dee_Alloca */
#endif /* !(CODE_FLAGS & CODE_FYIELDING) */
#if (CODE_FLAGS & CODE_FYIELDING) || (CODE_FLAGS & (CODE_FYIELDING | CODE_FVARKWDS)) == CODE_FVARKWDS
#undef err_ex_frame
#ifdef Dee_Alloca
#define err_ex_frame err
#endif /* Dee_Alloca */
#endif /* (CODE_FLAGS & CODE_FYIELDING) || (CODE_FLAGS & (CODE_FYIELDING | CODE_FVARKWDS)) == CODE_FVARKWDS */
#ifdef __INTELLISENSE__
	return result;
err_ex_frame_full:
	Dee_Free(frame.cf_kw);
#ifndef Dee_Alloca
err_ex_frame:
	Dee_Free((void *)((uintptr_t)frame.cf_kw + offsetof(struct code_frame_kwds, fk_kargv)));
#endif /* !Dee_Alloca */
err:
	DeeObject_FREE(yf);
#undef err_ex_frame
err:
	return NULL;
#endif /* __INTELLISENSE__ */
}

#ifdef __INTELLISENSE__
DECL_END
#endif

#undef UNIQUE
#undef KW_IS_MAPPING
#undef CODE_FLAGS

