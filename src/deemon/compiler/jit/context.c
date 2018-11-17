/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_JIT_CONTEXT_C
#define GUARD_DEEMON_COMPILER_JIT_CONTEXT_C 1

#include <deemon/api.h>
#include <deemon/compiler/jit.h>
#ifndef CONFIG_NO_JIT
#include <deemon/int.h>
#include <deemon/error.h>
#include <deemon/class.h>
#include <deemon/compiler/lexer.h>
#include <hybrid/unaligned.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

INTERN DREF DeeObject *FCALL
JITContext_Lookup(JITContext *__restrict self,
                  /*utf-8*/char const *__restrict name,
                  size_t namelen) {
 /* TODO */
 (void)self;
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot resolve JIT symbol `%$s'",
                 namelen,name);
 return NULL;
}

INTERN DREF DeeObject *FCALL
JITContext_LookupNth(JITContext *__restrict self,
                     /*utf-8*/char const *__restrict name,
                     size_t namelen, size_t nth) {
 /* TODO */
 (void)self;
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot resolve %Iu%s JIT symbol `%$s'",nth,
                 nth == 1 ? "st" :
                 nth == 2 ? "nd" :
                 nth == 3 ? "rd" : "th",
                 namelen,name);
 return NULL;
}



DECL_END

#endif /* !CONFIG_NO_JIT */

#endif /* !GUARD_DEEMON_COMPILER_JIT_CONTEXT_C */
