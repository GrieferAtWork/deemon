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
#ifndef GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_H
#define GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_H 1

#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/object.h>
#include <deemon/traceback.h>

DECL_BEGIN

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
