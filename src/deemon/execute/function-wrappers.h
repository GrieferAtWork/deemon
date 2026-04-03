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
#ifndef GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_H
#define GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_H 1

#include <deemon/api.h>

#include <deemon/code.h>      /* DeeCodeObject, DeeFunctionObject, DeeYieldFunctionObject */
#include <deemon/object.h>    /* DREF, DeeObject, DeeTypeObject, Dee_AsObject */
#include <deemon/traceback.h> /* DeeFrameObject */
#include <deemon/util/lock.h> /* Dee_atomic_lock_* */

#include "../objects/generic-proxy.h"

#include <stdint.h> /* uint16_t, uint32_t */

DECL_BEGIN

/************************************************************************/
/* FunctionStatics_Type                                                 */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFunctionObject, fs_func); /* [1..1][const] Function in question */
} FunctionStatics;

#define FunctionStatics_New(self) \
	((DREF FunctionStatics *)ProxyObject_New(&FunctionStatics_Type, Dee_AsObject(self)))

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFunctionObject, fsi_func); /* [1..1][const] Function in question */
	uint16_t                                fsi_sid;   /* [>= fsi_func->fo_code->co_refc][lock(ATOMIC)] Index of next static to enumerate. */
	uint16_t                                fsi_end;   /* [== fsi_func->fo_code->co_refstaticc][const] Static enumeration end */
} FunctionStaticsIterator;

INTDEF DeeTypeObject FunctionStaticsIterator_Type;
INTDEF DeeTypeObject FunctionStatics_Type;



/************************************************************************/
/* FunctionSymbolsByName_Type                                           */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFunctionObject, fsbn_func);     /* [1..1][const] Function in question */
	uint16_t                                fsbn_rid_start; /* [const] First RID/SID to enumerate. */
	uint16_t                                fsbn_rid_end;   /* [const] Last RID/SID to enumerate, plus 1. */
} FunctionSymbolsByName;

typedef struct {
	PROXY_OBJECT_HEAD_EX(FunctionSymbolsByName, fsbni_seq); /* [1..1][const] Function whose references/statics are being enumerated. */
	DeeFunctionObject                          *fsbni_func; /* [== fsbni_seq->fsbn_func][1..1][const] Function whose references/statics are being enumerated. */
	uint16_t                                    fsbni_rid;  /* [lock(ATOMIC)] Next rid (overflowing into sids) to enumerate. */
	uint16_t                                    fsbni_end;  /* [== fsbni_seq->fsbn_rid_end][const] RIS/SID end index. */
} FunctionSymbolsByNameIterator;

INTDEF DeeTypeObject FunctionSymbolsByNameIterator_Type;
INTDEF DeeTypeObject FunctionSymbolsByNameKeysIterator_Type;
INTDEF DeeTypeObject FunctionSymbolsByName_Type;



/************************************************************************/
/* YieldFunctionSymbolsByName                                           */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeYieldFunctionObject, yfsbn_yfunc);    /* [1..1][const] The function of the frame (as a cache) */
	uint16_t                                     yfsbn_nargs;     /* [<= yfsbn_yfunc->yf_func->fo_code->co_argc_max][const] The # of arguments to enumerate. */
	uint16_t                                     yfsbn_rid_start; /* [<= frsbn_rid_end][const] First RID/SID to enumerate. */
	uint16_t                                     yfsbn_rid_end;   /* [<= yfsbn_yfunc->yf_func->fo_code->co_refstaticc][const] Last RID/SID to enumerate, plus 1. */
} YieldFunctionSymbolsByName;

typedef union {
	uint32_t yfsbnii_word; /* Index word */
	struct {
		uint16_t i_aid;    /* Next argument index to enumerate */
		uint16_t i_rid;    /* Next reference/static index to enumerate */
	} yfsbnii_idx;
} YieldFunctionSymbolsByNameIteratorIndex;

typedef struct {
	PROXY_OBJECT_HEAD_EX(YieldFunctionSymbolsByName, yfsbni_seq); /* [1..1][const] Underlying frame-symbols sequence. */
	YieldFunctionSymbolsByNameIteratorIndex          yfsbni_idx;  /* Iterator index */
} YieldFunctionSymbolsByNameIterator;

INTDEF DeeTypeObject YieldFunctionSymbolsByNameIterator_Type;
INTDEF DeeTypeObject YieldFunctionSymbolsByNameKeysIterator_Type;
INTDEF DeeTypeObject YieldFunctionSymbolsByName_Type;



/************************************************************************/
/* FrameArgs                                                            */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeFrameObject, fa_frame, /* [1..1][const] The frame in question */
	                      DeeCodeObject,  fa_code); /* [1..1][const] The code running in `fa_frame' (cache) */
} FrameArgs;

INTDEF DeeTypeObject FrameArgs_Type;



/************************************************************************/
/* FrameLocals                                                          */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFrameObject, fl_frame); /* [1..1][const] The frame in question */
	uint16_t                             fl_localc; /* [const] The # of local variables there are (cache) */
} FrameLocals;

INTDEF DeeTypeObject FrameLocals_Type;



/************************************************************************/
/* FrameStack                                                           */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFrameObject, fs_frame); /* [1..1][const] The frame in question */
} FrameStack;

#define FrameStack_New(frame) \
	((DREF FrameStack *)ProxyObject_New(&FrameStack_Type, Dee_AsObject(frame)))

INTDEF DeeTypeObject FrameStack_Type;



/************************************************************************/
/* FrameSymbolsByName                                                   */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeFrameObject,    frsbn_frame,     /* [1..1][const] The frame in question */
	                      DeeFunctionObject, frsbn_func);     /* [1..1][const] The function of the frame (as a cache) */
	uint16_t                                 frsbn_nargs;     /* [<= frsbn_func->fo_code->co_argc_max][const] The # of arguments to enumerate. */
	uint16_t                                 frsbn_rid_start; /* [<= frsbn_rid_end][const] First RID/SID to enumerate. */
	uint16_t                                 frsbn_rid_end;   /* [<= frsbn_func->fo_code->co_refstaticc][const] Last RID/SID to enumerate, plus 1. */
	uint16_t                                 frsbn_localc;    /* [<= frsbn_func->fo_code->co_localc][const] The # of locals to enumerate. */
	uint16_t                                 frsbn_stackc;    /* [const] The # of stack slots to enumerate (during enum, stop early if less than this remain). */
} FrameSymbolsByName;

typedef struct {
	uint16_t frsbnii_aid; /* [<= frsbni_seq->frsbn_nargs] Next arg to enumerate, or `>= frsbn_nargs' if all were enumerated. */
	uint16_t frsbnii_rid; /* [>= frsbni_seq->frsbn_rid_start && <= frsbni_seq->frsbn_rid_end] Next ref/static to enumerate, or `>= frsbn_rid_end' if all were enumerated. */
	uint16_t frsbnii_lid; /* [<= frsbni_seq->frsbn_localc] Next local to enumerate, or `>= frsbn_localc' if all were enumerated. */
	uint16_t frsbnii_nsp; /* [<= frsbni_seq->frsbn_stackc] NextStackPointer to enumerate, or `>= frsbn_stackc' if all were enumerated. */
} FrameSymbolsByNameIteratorIndex;

typedef struct {
	PROXY_OBJECT_HEAD_EX(FrameSymbolsByName, frsbni_seq); /* [1..1][const] Underlying frame-symbols sequence. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t                        frsbni_lock; /* Lock for the below indices */
#endif /* !CONFIG_NO_THREADS */
	FrameSymbolsByNameIteratorIndex          frsbni_idx;  /* Iterator index */
} FrameSymbolsByNameIterator;

#define FrameSymbolsByNameIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->frsbni_lock)

INTDEF DeeTypeObject FrameSymbolsByNameIterator_Type;
INTDEF DeeTypeObject FrameSymbolsByNameKeysIterator_Type;
INTDEF DeeTypeObject FrameSymbolsByName_Type;




/* Callbacks to create specialized function wrappers. */

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFunction_GetStaticsWrapper(DeeFunctionObject *__restrict self);       /* ?S?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFunction_GetRefsByNameWrapper(DeeFunctionObject *__restrict self);    /* ?M?X2?Dstring?Dint?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFunction_GetStaticsByNameWrapper(DeeFunctionObject *__restrict self); /* ?M?X2?Dstring?Dint?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFunction_GetSymbolsByNameWrapper(DeeFunctionObject *__restrict self); /* ?M?X2?Dstring?Dint?O */

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeYieldFunction_GetArgsByNameWrapper(DeeYieldFunctionObject *__restrict self);    /* ?M?X2?Dstring?Dint?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeYieldFunction_GetSymbolsByNameWrapper(DeeYieldFunctionObject *__restrict self); /* ?M?X2?Dstring?Dint?O */

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetArgsWrapper(DeeFrameObject *__restrict self);            /* ?S?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetLocalsWrapper(DeeFrameObject *__restrict self);          /* ?S?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetStackWrapper(DeeFrameObject *__restrict self);           /* ?S?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetArgsByNameWrapper(DeeFrameObject *__restrict self);      /* ?M?X2?Dstring?Dint?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetLocalsByNameWrapper(DeeFrameObject *__restrict self);    /* ?M?X2?Dstring?Dint?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetStackByNameWrapper(DeeFrameObject *__restrict self);     /* ?M?X2?Dstring?Dint?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetVariablesByNameWrapper(DeeFrameObject *__restrict self); /* ?M?X2?Dstring?Dint?O */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeFrame_GetSymbolsByNameWrapper(DeeFrameObject *__restrict self);   /* ?M?X2?Dstring?Dint?O */

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_H */
