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
#ifdef __INTELLISENSE__
#include "parser-impl.c.inl"
#endif

DECL_BEGIN

#ifdef JIT_EVAL
INTERN DREF DeeObject *FCALL
JITLexer_EvalStatementOrBraces(JITLexer *__restrict self,
                               unsigned int *pwas_expression)
#else
INTERN int FCALL
JITLexer_SkipStatementOrBraces(JITLexer *__restrict self,
                               unsigned int *pwas_expression)
#endif
{
 RETURN_TYPE result;
 /* TODO: Auto-detect if it's a statement, or an expression. */
#ifdef JIT_EVAL
 result = JITLexer_EvalStatement(self);
#else
 result = JITLexer_SkipStatement(self);
#endif
 if (pwas_expression)
    *pwas_expression = AST_PARSE_WASEXPR_NO;
 return result;
}

DECL_END

