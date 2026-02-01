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
#ifndef GUARD_DEX_STREXEC_ERROR_C
#define GUARD_DEX_STREXEC_ERROR_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/api.h>

#include <deemon/error.h>  /* DeeError_* */
#include <deemon/format.h> /* PRFuSIZ */
#include <deemon/object.h> /* ASSERT_OBJECT, DeeObject */

#include <stddef.h> /* size_t */

DECL_BEGIN

/* RT Exception handlers. */
INTERN ATTR_COLD ATTR_INS(1, 2) int DCALL
err_invalid_argc_len(char const *function_name, size_t function_size,
                     size_t argc_cur, size_t argc_min, size_t argc_max) {
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%$s expects %" PRFuSIZ " arguments when %" PRFuSIZ " w%s given",
		                       function_size ? " " : "", function_size, function_name,
		                       argc_min, argc_cur, argc_cur == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%$s expects between %" PRFuSIZ " and "
		                       "%" PRFuSIZ " arguments when %" PRFuSIZ " w%s given",
		                       function_size ? " " : "", function_size, function_name,
		                       argc_min, argc_max, argc_cur, argc_cur == 1 ? "as" : "ere");
	}
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unknown_global(DeeObject *__restrict key) {
	ASSERT_OBJECT(key);
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Unknown global `%k'",
	                       key);
}

INTERN ATTR_COLD ATTR_INS(1, 2) int DCALL
err_unknown_global_str_len(char const *__restrict key, size_t keylen) {
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Unknown global `%$s'",
	                       keylen, key);
}







/* Syntax Exception handlers. */
PRIVATE NONNULL((1)) void DFCALL
syn_trace_here(JITLexer *__restrict self) {
	JITLexer_ErrorTrace(self, self->jl_tokstart);
	self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_if_expected_lparen_after_if(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `if', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_if_expected_rparen_after_if(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `if (...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_with_expected_lparen_after_with(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `with', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_with_expected_rparen_after_with(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `with (...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_for_expected_lparen_after_for(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `for', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_for_expected_rparen_after_for(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `for (...;...;...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_for_expected_rparen_after_foreach(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `for (...: ...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_for_expected_semi1_after_for(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `for', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_for_expected_semi2_after_for(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a second `;' after `for', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_throw_expected_semi_after_throw(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `throw', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_yield_expected_semi_after_yield(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `yield', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_break_expected_semi_after_break(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `break', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_continue_expected_semi_after_continue(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `continue', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_return_expected_semi_after_return(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `return', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_foreach_expected_lparen_after_foreach(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `foreach', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_foreach_expected_colon_after_foreach(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `:' after `foreach (...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_foreach_expected_rparen_after_foreach(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `foreach (...: ...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_while_expected_lparen_after_while(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `while', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_while_expected_rparen_after_while(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `while (...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}



INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_dowhile_expected_while_after_do(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `while' after `do ...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_dowhile_expected_lparen_after_while(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `do ... while', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_dowhile_expected_rparen_after_while(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `do ... while (...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_dowhile_expected_semi_after_while(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `do ... while (...)', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_nonempty_string(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "User-assembly statements with non-empty "
	                       "assembly text are not supported");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_string_after_asm(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `{' or a string after `__asm__', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_semi_after_asm(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after `__asm__(...)', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_lparen_after_asm(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `__asm__', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_rparen_after_asm(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `__asm__(...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_keyword_after_lbracket(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a keyword as operand name after `[', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_rbracket_after_lbracket(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `]' after `[' followed by an operand name, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_string_before_operand(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a string before the operand value, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_lparen_before_operand(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' before an assembly operand value, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_rparen_after_operand(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after an assembly operand value, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_asm_expected_keyword_for_label_operand(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a keyword as label operand, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}



INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_try_expected_lparen_after_catch(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `catch', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_try_expected_rparen_after_catch(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `catch', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_try_expected_keyword_after_as_in_catch(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a keyword after `catch (... as', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_brace_expected_rbrace(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `}' to end a brace initializer, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_brace_expected_keyword_after_dot(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected keyword after `.' in brace initializer, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_brace_expected_equals_after_dot(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `=' after `.' in mapping-like brace initializer, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_brace_expected_colon_after_key(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `:' after key in mapping-like brace initializer, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_expr_expected_semi_after_expr(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `;' after expression, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_expr_unexpected_token(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Unexpected token `%$s' in expression",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_function_expected_lparen_after_function(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `function', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_function_expected_arrow_or_lbrace_after_function(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `{' or `->' after `function(...)' or `[](...)', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_call_expected_rparen_after_call(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' to end call operation, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_bound_cannot_test(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Cannot test binding of expression. "
	                       "Expected a symbol, or attribute expression");
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_pack_expected_rparen_after_lparen(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `pack(', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_paren_expected_rparen_after_lparen(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `(', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_bracket_expected_rbracket_after_lbracket(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `]' after `[', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_attr_expected_keyword(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a keyword after `.', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_item_expected_rbracket_after_lbracket(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `]' to end `getitem' operator, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_isin_expected_is_or_in_after_exclaim(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `is' or `in' after `!', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_operator_expected_empty_string(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected an empty string for `operator str'");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_operator_expected_lbracket_or_dot_after_del(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `[' or `.' after `del' in operator name, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_operator_unknown_name(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Unknown operator name `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_anno_expected_rbracket(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `]' after `@[...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_class_expected_class_after_final(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `class' after `final', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_class_expected_rparen_after_lparen_base(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `class ... (...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_class_expected_lbrace_after_class(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `{' after `class ...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_class_expected_rbrace_after_class(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `}' after `class { ...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_class_not_thiscall(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "`this' or `super' are only allowed in thiscall functions");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_nth_expected_lparen(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `(' after `__nth', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_nth_expected_rparen(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after `__nth(...', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_type_annotation_unexpected_token(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Unexpected token `%$s' in type annotation",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_type_annotation_expected_dots_or_colon(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `...' or `:' after `{type' in type annotation, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_type_annotation_expected_rbrace(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `}' after sequence or mapping declaration, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_type_annotation_expected_rparen(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `)' after Tuple declaration or parenthesis, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_type_annotation_expected_string_after_asm(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a string after `__asm__', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_template_string_unterminated(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError, "Unterminated template string");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_template_string_unmatched_lbrace(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError, "Unmatched '{' in template string");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_template_string_unmatched_rbrace(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Unmatched '}' in template string");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_template_string_no_digit_or_hex_after_backslash_x_u_U(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "No digits, or hex-chars found after `\\x', `\\u' or `\\U'");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_template_string_undefined_escape(JITLexer *__restrict self, int ch) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Unknown escape character `%c' in template string",
	                       ch);
}


INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_expected_dot_keyword_or_string(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `.', a keyword or a string in a module- or symbol-import list, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_expected_keyword_after_as(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a keyword after `as' in a module- or symbol-import list, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_expected_keyword_or_string_in_import_list(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected a keyword or a string in a symbol-import list, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1, 2)) int DFCALL
syn_import_invalid_name_for_module_symbol(JITLexer *__restrict self,
                                          struct jit_import_item const *__restrict item) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Cannot import module %r: %$q is not a valid variable name",
	                       item->ii_import_name,
	                       item->ii_symbol_size,
	                       item->ii_symbol_name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DFCALL
syn_import_invalid_name_for_import_symbol(JITLexer *__restrict self,
                                          struct jit_import_item const *__restrict item) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Cannot import symbol %r: %$q is not a valid variable name",
	                       item->ii_import_name,
	                       item->ii_symbol_size,
	                       item->ii_symbol_name);
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_expected_comma_or_from_after_star(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `,' or `from' after `*' in symbol import list, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_expected_from_after_symbol_import_list(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `from' after symbol import list, but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_expected_import_after_from(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Expected `import' after `from', but got `%$s'",
	                       JITLexer_TokLen(self),
	                       JITLexer_TokPtr(self));
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_unexpected_from_after_module_import_list(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Unexpected `from' following a module import list");
}

INTERN ATTR_COLD NONNULL((1)) int DFCALL
syn_import_unexpected_star_duplication_in_import_list(JITLexer *__restrict self) {
	syn_trace_here(self);
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Unexpected `*' in import list, when `*' had already been encountered before");
}


INTERN ATTR_COLD int
(DFCALL err_cannot_import_relative)(char const *module_name,
                                    size_t module_namelen) {
	return DeeError_Throwf(&DeeError_CompilerError,
	                       "Cannot import relative module %$q",
	                       module_namelen, module_name);
}

DECL_END

#endif /* !GUARD_DEX_STREXEC_ERROR_C */
