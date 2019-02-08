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
#ifndef GUARD_DEX_JIT_ERROR_C
#define GUARD_DEX_JIT_ERROR_C 1

#include "libjit.h"
#include <deemon/error.h>

DECL_BEGIN

/* RT Exception handlers. */
INTERN ATTR_COLD int DCALL
err_invalid_argc_len(char const *function_name, size_t function_size,
                     size_t argc_cur, size_t argc_min, size_t argc_max) {
 if (argc_min == argc_max) {
  return DeeError_Throwf(&DeeError_TypeError,
                         "function%s%$s expects %Iu arguments when %Iu w%s given",
                         function_size ? " " : "",function_size,function_name,
                         argc_min,argc_cur,argc_cur == 1 ? "as" : "ere");
 } else {
  return DeeError_Throwf(&DeeError_TypeError,
                         "function%s%$s expects between %Iu and %Iu arguments when %Iu w%s given",
                         function_size ? " " : "",function_size,function_name,
                         argc_min,argc_max,argc_cur,argc_cur == 1 ? "as" : "ere");
 }
}

INTERN ATTR_COLD int DCALL err_no_active_exception(void) {
 return DeeError_Throwf(&DeeError_RuntimeError,"No active exception");
}

INTERN ATTR_COLD int DCALL
err_unknown_global(DeeObject *__restrict key) {
 ASSERT_OBJECT(key);
 return DeeError_Throwf(&DeeError_KeyError,
                        "Unknown global `%k'",
                        key);
}
INTERN ATTR_COLD int DCALL
err_unknown_global_str_len(char const *__restrict key, size_t keylen) {
 return DeeError_Throwf(&DeeError_KeyError,
                        "Unknown global `%$s'",
                        keylen,key);
}

INTERN ATTR_COLD int DCALL
err_invalid_unpack_size(DeeObject *__restrict unpack_object,
                        size_t need_size, size_t real_size) {
 ASSERT_OBJECT(unpack_object);
 (void)unpack_object;
 return DeeError_Throwf(&DeeError_UnpackError,
                        "Expected %Iu object%s when %Iu w%s given",
                        need_size,need_size > 1 ? "s" : "",real_size,
                        real_size == 1 ? "as" : "ere");
}
INTERN ATTR_COLD int DCALL
err_invalid_unpack_iter_size(DeeObject *__restrict unpack_object,
                             DeeObject *__restrict unpack_iterator,
                             size_t need_size) {
 ASSERT_OBJECT(unpack_object);
 ASSERT_OBJECT(unpack_iterator);
 (void)unpack_object;
 (void)unpack_iterator;
 return DeeError_Throwf(&DeeError_UnpackError,
                        "Expected %Iu object%s when at least %Iu w%s given",
                        need_size,need_size > 1 ? "s" : "",need_size+1,
                        need_size == 0 ? "as" : "ere");
}








/* Syntax Exception handlers. */
LOCAL void FCALL syn_trace_here(JITLexer *__restrict self) {
 JITLexer_ErrorTrace(self,self->jl_tokstart);
 self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
}

INTERN ATTR_COLD int FCALL
syn_if_expected_lparen_after_if(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `if', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_if_expected_rparen_after_if(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `if (...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}

INTERN ATTR_COLD int FCALL
syn_with_expected_lparen_after_with(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `with', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_with_expected_rparen_after_with(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `with (...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}

INTERN ATTR_COLD int FCALL
syn_for_expected_lparen_after_for(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `for', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_for_expected_rparen_after_for(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `for (...;...;...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_for_expected_rparen_after_foreach(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `for (...: ...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_for_expected_semi1_after_for(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `for', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_for_expected_semi2_after_for(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected a second `;' after `for', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}

INTERN ATTR_COLD int FCALL
syn_throw_expected_semi_after_throw(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `throw', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_yield_expected_semi_after_yield(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `yield', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_break_expected_semi_after_break(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `break', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_continue_expected_semi_after_continue(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `continue', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_return_expected_semi_after_return(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `return', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}

INTERN ATTR_COLD int FCALL
syn_foreach_expected_lparen_after_foreach(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `foreach', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_foreach_expected_collon_after_foreach(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `:' after `foreach (...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_foreach_expected_rparen_after_foreach(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `foreach (...: ...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}

INTERN ATTR_COLD int FCALL
syn_while_expected_lparen_after_while(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `while', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_while_expected_rparen_after_while(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `while (...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}



INTERN ATTR_COLD int FCALL
syn_dowhile_expected_while_after_do(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `while' after `do ...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_dowhile_expected_lparen_after_while(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `do ... while', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_dowhile_expected_rparen_after_while(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `do ... while (...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_dowhile_expected_semi_after_while(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `do ... while (...)', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}

INTERN ATTR_COLD int FCALL
syn_asm_nonempty_string(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "User-assembly statements with non-empty assembly text are not supported");
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_string_after_asm(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `{' or a string after `__asm__', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_semi_after_asm(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after `__asm__(...)', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_lparen_after_asm(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `__asm__', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_rparen_after_asm(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `__asm__(...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_keyword_after_lbracket(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected a keyword as operand name after `[', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_rbracket_after_lbracket(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `]' after `[' followed by an operand name, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_string_before_operand(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected a string before the operand value, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_lparen_before_operand(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' before an assembly operand value, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_rparen_after_operand(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after an assembly operand value, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_asm_expected_keyword_for_label_operand(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected a keyword as label operand, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}



INTERN ATTR_COLD int FCALL
syn_try_expected_lparen_after_catch(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `catch', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_try_expected_rparen_after_catch(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected a keyword as label operand, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_brace_expected_rbrace(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `}' to end a brace initializer, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_brace_expected_keyword_after_dot(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected keyword after `.' in brace initializer, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_brace_expected_equals_after_dot(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `=' after `.' in mapping-like brace initializer, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_brace_expected_collon_after_key(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `:' after key in mapping-like brace initializer, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_expr_expected_semi_after_expr(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `;' after expression, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_expr_unexpected_token(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Unexpected token `%$s' in expression",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_function_expected_lparen_after_function(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `(' after `function', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_function_expected_arrow_or_lbrace_after_function(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `{' or `->' after `function(...)' or `[](...)', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_call_expected_rparen_after_call(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' to end call operation, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_bound_cannot_test(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Cannot test binding of expression. "
                        "Expected a symbol, or attribute expression");
}


INTERN ATTR_COLD int FCALL
syn_pack_expected_rparen_after_lparen(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `pack(', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_paren_expected_rparen_after_lparen(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `)' after `(', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_bracket_expected_rbracket_after_lbracket(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `]' after `[', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_attr_expected_keyword(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected a keyword after `.', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_item_expected_rbracket_after_lbracket(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `]' to end `getitem' operator, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_isin_expected_is_or_in_after_exclaim(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `is' or `in' after `!', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_operator_expected_empty_string(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected an empty string for `operator str'");
}
INTERN ATTR_COLD int FCALL
syn_operator_expected_lbracket_or_dot_after_del(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `[' or `.' after `del' in operator name, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}
INTERN ATTR_COLD int FCALL
syn_operator_unknown_name(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Unknown operator name `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_module_expected_dot_keyword_or_string(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `.', a keyword or a string in a module- or symbol-import list, but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}


INTERN ATTR_COLD int FCALL
syn_anno_expected_rbracket(JITLexer *__restrict self) {
 syn_trace_here(self);
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Expected `]' after `@[...', but got `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
}



DECL_END

#endif /* !GUARD_DEX_JIT_ERROR_C */
