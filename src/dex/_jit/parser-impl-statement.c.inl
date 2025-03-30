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
#include "parser-impl.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/thread.h>

DECL_BEGIN

INTERN RETURN_TYPE DFCALL
FUNC(StatementBlock)(JITLexer *__restrict self) {
	RETURN_TYPE result;
#ifndef JIT_EVAL
	/* Special case optimization:
	 *  -> Since we're supposed to skip the statement, we can optimize for
	 *     this case by scanning ahead for the next matching `}' token! */
	if (JITLexer_SkipPair(self, '{', '}'))
		goto err;
	result = 0;
#else /* !JIT_EVAL */
	for (;;) {
#ifdef JIT_EVAL
		result = JITLexer_EvalStatement(self);
#else /* JIT_EVAL */
		result = JITLexer_SkipStatement(self);
#endif /* !JIT_EVAL */
		if (ISERR(result))
			goto err;
		if (self->jl_tok == '}')
			break;
#ifdef JIT_EVAL
		/* Discard the intermediate statement result. */
		if (result == JIT_LVALUE) {
			JITLValue_Fini(&self->jl_lvalue);
			JITLValue_Init(&self->jl_lvalue);
		} else {
			Dee_Decref(result);
		}
#endif /* JIT_EVAL */
	}
	/* Make sure to unwind l-values to prevent corrupt access
	 * to local variables from the scope we're about to pop. */
	JITLexer_Yield(self);
#endif /* JIT_EVAL */
	return result;
err:
	return ERROR;
}


#ifndef IS_COL_DEFINED
#define IS_COL_DEFINED 1
LOCAL WUNUSED bool DFCALL
JITLexer_IsColumn(JITLexer *__restrict self) {
	switch (self->jl_tok) {
	case ':': break;
	case TOK_COLON_EQUAL:
		self->jl_tok = ':';
		--self->jl_tokend;
		break;
	default:
		return false;
	}
	return true;
}
#endif /* !IS_COL_DEFINED */

#ifndef SKIP_ASSEMBLY_OPERAND_NAME_DEFINED
#define SKIP_ASSEMBLY_OPERAND_NAME_DEFINED 1
LOCAL WUNUSED int DFCALL
JITLexer_SkipAssemblyOperandName(JITLexer *__restrict self) {
	if (self->jl_tok == '[') {
		JITLexer_Yield(self);
		if likely(self->jl_tok == JIT_KEYWORD) {
			JITLexer_Yield(self);
		} else {
			syn_asm_expected_keyword_after_lbracket(self);
			goto err;
		}
		if likely(self->jl_tok == ']') {
			JITLexer_Yield(self);
		} else {
			syn_asm_expected_rbracket_after_lbracket(self);
			goto err;
		}
	}
	return 0;
err:
	return -1;
}
#endif /* !SKIP_ASSEMBLY_OPERAND_NAME_DEFINED */


LOCAL WUNUSED int DFCALL
FUNC(AssemblyOperands)(JITLexer *__restrict self
#ifdef JIT_EVAL
                       , bool is_input
#endif /* JIT_EVAL */
                       ) {
#ifdef JIT_EVAL
	DREF DeeObject *temp;
#endif /* JIT_EVAL */
	while (self->jl_tok == '[' ||
	       self->jl_tok == JIT_STRING ||
	       self->jl_tok == JIT_RAWSTRING) {
		if unlikely(JITLexer_SkipAssemblyOperandName(self))
			goto err;
		if (self->jl_tok == JIT_STRING ||
		    self->jl_tok == JIT_RAWSTRING) {
			do {
				JITLexer_Yield(self);
			} while (self->jl_tok == JIT_STRING ||
			         self->jl_tok == JIT_RAWSTRING);
		} else {
			syn_asm_expected_string_before_operand(self);
			goto err;
		}
		if (self->jl_tok == '(') {
			/* Parse the assembly operand. */
do_parse_lparen:
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			temp = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
			if unlikely(!temp)
				goto err;
#else /* JIT_EVAL */
			if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
				goto err;
#endif /* !JIT_EVAL */
			if (self->jl_tok == ')') {
				JITLexer_Yield(self);
			} else {
#ifdef JIT_EVAL
				if (temp == JIT_LVALUE) {
					JITLValue_Fini(&self->jl_lvalue);
					self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
				} else {
					Dee_Decref(temp);
				}
#endif /* JIT_EVAL */
				syn_asm_expected_rparen_after_operand(self);
				goto err;
			}
		} else if (JITLexer_ISKWD(self, "pack")) {
			JITLexer_Yield(self);
			if (self->jl_tok == '(')
				goto do_parse_lparen;
#ifdef JIT_EVAL
			temp = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
			if unlikely(!temp)
				goto err;
#else /* JIT_EVAL */
			if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
				goto err;
#endif /* !JIT_EVAL */
		} else {
			syn_asm_expected_lparen_before_operand(self);
			goto err;
		}
#ifdef JIT_EVAL
		if (temp == JIT_LVALUE) {
			if (is_input) {
				/* Input operands are dereferenced. */
				temp = JITLexer_PackLValue(self);
				if unlikely(!temp)
					goto err;
				goto do_decref_temp_rvalue;
			}
			JITLValue_Fini(&self->jl_lvalue);
			self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
		} else {
do_decref_temp_rvalue:
			Dee_Decref(temp);
		}
#endif /* JIT_EVAL */
		/* Operand parsed. - Check for a `,', followed by another one. */
		if (self->jl_tok != ',')
			break;
		JITLexer_Yield(self);
		/* continue; */
	}
	return 0;
err:
	return -1;
}

#ifndef SKIP_ASSEMBLY_CLOBBER_DEFINED
#define SKIP_ASSEMBLY_CLOBBER_DEFINED 1
LOCAL void DFCALL
JITLexer_SkipAssemblyClobber(JITLexer *__restrict self) {
	while (self->jl_tok == JIT_STRING ||
	       self->jl_tok == JIT_RAWSTRING) {
		do {
			JITLexer_Yield(self);
		} while (self->jl_tok == JIT_STRING ||
		         self->jl_tok == JIT_RAWSTRING);
		if (self->jl_tok != ',')
			break;
		JITLexer_Yield(self);
	}
}
#endif /* !SKIP_ASSEMBLY_CLOBBER_DEFINED */

#ifndef SKIP_ASSEMBLY_LABELS_DEFINED
#define SKIP_ASSEMBLY_LABELS_DEFINED 1
LOCAL WUNUSED int DFCALL
JITLexer_SkipAssemblyLabels(JITLexer *__restrict self) {
	while (self->jl_tok == JIT_KEYWORD || self->jl_tok == '[') {
		if unlikely(JITLexer_SkipAssemblyOperandName(self))
			goto err;
		if (self->jl_tok != JIT_KEYWORD) {
			syn_asm_expected_keyword_for_label_operand(self);
			goto err;
		}
		JITLexer_Yield(self);
		if (self->jl_tok != ',')
			break;
		JITLexer_Yield(self);
	}
	return 0;
err:
	return -1;
}
#endif /* !SKIP_ASSEMBLY_LABELS_DEFINED */



INTERN RETURN_TYPE DFCALL
FUNC(Statement)(JITLexer *__restrict self) {
	RETURN_TYPE result;
	switch (self->jl_tok) {

	case ';':
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		result = DeeNone_NewRef();
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
		break;

	case '{':
		JITLexer_Yield(self);
		if (self->jl_tok == '}') {
#ifdef JIT_EVAL
			result = DeeNone_NewRef();
#else /* JIT_EVAL */
			result = 0;
#endif /* !JIT_EVAL */
			JITLexer_Yield(self);
		} else {
			IF_EVAL(JITContext_PushScope(self->jl_context));
			result = FUNC(StatementBlock)(self);
			LOAD_LVALUE(result, err_popscope);
			IF_EVAL(JITContext_PopScope(self->jl_context));
		}
		break;

	case JIT_KEYWORD: {
		char const *tok_begin;
		size_t tok_length;
		uint32_t name;
		tok_begin  = (char *)self->jl_tokstart;
		tok_length = (size_t)((char *)self->jl_tokend - tok_begin);
		switch (tok_length) {

		case 2:
			if (tok_begin[0] == 'i' && tok_begin[1] == 'f') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(If)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (tok_begin[0] == 'd' && tok_begin[1] == 'o') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(Do)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			break;

		case 3:
			if (tok_begin[0] == 't' && tok_begin[1] == 'r' &&
			    tok_begin[2] == 'y') {
				result = FUNC(Try)(self, true);
				goto done;
			}
			if (tok_begin[0] == 'f' && tok_begin[1] == 'o' &&
			    tok_begin[2] == 'r') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(For)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (tok_begin[0] == 'd' && tok_begin[1] == 'e' &&
			    tok_begin[2] == 'l') {
				result = FUNC(Del)(self, true);
				goto done;
			}
			break;

		case 4:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('w', 'i', 't', 'h')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(With)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('f', 'r', 'o', 'm')) {
				JITLexer_Yield(self);
				result = FUNC(FromImport)(self);
				goto done;
			}
			break;

		case 5:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('t', 'h', 'r', 'o') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'w') {
				JITLexer_Yield(self);
				if (self->jl_tok == ';') {
					JITLexer_Yield(self);
#ifdef JIT_EVAL
					/* Rethrow the last active exception. */
					ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
					if (DeeThread_Self()->t_exceptsz <= self->jl_context->jc_except)
						err_no_active_exception();
					result = NULL;
#else /* JIT_EVAL */
					result = 0;
#endif /* !JIT_EVAL */
				} else {
					result = FUNC(Comma)(self, AST_COMMA_NORMAL, IF_EVAL(&DeeTuple_Type, ) NULL);
					if (ISERR(result))
						goto err;
					LOAD_LVALUE(result, err);
					if likely(self->jl_tok == ';') {
						JITLexer_Yield(self);
					} else {
						syn_throw_expected_semi_after_throw(self);
						goto err_r;
					}
#ifdef JIT_EVAL
					ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
					/* Throw the given object. */
					DeeError_ThrowInherited(result);
					result = NULL;
#endif /* JIT_EVAL */
				}
				goto done;
			}
			if (name == ENCODE_INT32('w', 'h', 'i', 'l') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'e') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(While)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('y', 'i', 'e', 'l') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'd') {
#ifdef JIT_EVAL
				(void)SYNTAXERROR("Yield statements are not supported at this location");
				goto err;
#else /* JIT_EVAL */
				JITLexer_Yield(self);
				if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
					goto err;
				if likely(self->jl_tok == ';') {
					JITLexer_Yield(self);
				} else {
					syn_yield_expected_semi_after_yield(self);
					goto err_r;
				}
				result = 0;
				goto done;
#endif /* !JIT_EVAL */
			}
			if (name == ENCODE_INT32('p', 'r', 'i', 'n') &&
			    UNALIGNED_GET8(tok_begin + 4) == 't') {
				/* TODO */
				DERROR_NOTIMPLEMENTED();
				goto err;
			}
			if (name == ENCODE_INT32('b', 'r', 'e', 'a') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'k') {
				JITLexer_Yield(self);
				if likely(self->jl_tok == ';') {
					JITLexer_Yield(self);
				} else {
					syn_break_expected_semi_after_break(self);
					goto err;
				}
#ifdef JIT_EVAL
				result = NULL;
				ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
				self->jl_context->jc_retval = JITCONTEXT_RETVAL_BREAK;
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done;
			}
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'm')
				goto do_asm;
			break;

		case 6:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('r', 'e', 't', 'u') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 'n')) {
				JITLexer_Yield(self);
				if (self->jl_tok == ';') {
					JITLexer_Yield(self);
#ifdef JIT_EVAL
					ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
					self->jl_context->jc_retval = DeeNone_NewRef();
					result = NULL;
#else /* JIT_EVAL */
					result = 0;
#endif /* !JIT_EVAL */
				} else {
					result = FUNC(Comma)(self, AST_COMMA_NORMAL, IF_EVAL(&DeeTuple_Type, ) NULL);
					if (ISERR(result))
						goto err;
					LOAD_LVALUE(result, err);
					if likely(self->jl_tok == ';') {
						JITLexer_Yield(self);
					} else {
						syn_return_expected_semi_after_return(self);
						goto err_r;
					}
#ifdef JIT_EVAL
					ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
					self->jl_context->jc_retval = result; /* Inherit reference. */
					result                      = NULL;   /* Propagate the return value. */
#endif /* JIT_EVAL */
				}
				goto done;
			}
			if (name == ENCODE_INT32('a', 's', 's', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(Assert)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('i', 'm', 'p', 'o') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't')) {
				JITLexer_Yield(self);
				result = FUNC(Import)(self);
				goto done;
			}
			if (name == ENCODE_INT32('s', 'w', 'i', 't') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('c', 'h')) {
				/* TODO */
				DERROR_NOTIMPLEMENTED();
				goto err;
			}
			break;

		case 7:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('a', 'c') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'h') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(Foreach)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('m', '_') &&
			    UNALIGNED_GET8(tok_begin + 6) == '_') {
				bool is_asm_goto;
				bool need_trailing_rparen;
do_asm:
				JITLexer_Yield(self);
				/* Skip assembly modifiers. */
				is_asm_goto = false;
				while (self->jl_tok == JIT_KEYWORD) {
					unsigned char *kwd_start = self->jl_tokstart;
					unsigned char *kwd_end   = self->jl_tokend;
					while (kwd_start < kwd_end && *kwd_start == '_')
						++kwd_start;
					while (kwd_end > kwd_start && kwd_end[-1] == '_')
						--kwd_end;
#define IS_MOD(x)                                           \
	((size_t)(kwd_end - kwd_start) == COMPILER_STRLEN(x) && \
	 bcmpc(kwd_start, x, COMPILER_STRLEN(x), sizeof(char)) == 0)
					if (IS_MOD("volatile")) {
						JITLexer_Yield(self);
						continue;
					}
					if (IS_MOD("goto")) {
						is_asm_goto = true;
						JITLexer_Yield(self);
						continue;
					}
					break;
#undef IS_MOD
				}
				need_trailing_rparen = true;
				if (self->jl_tok == '(') {
do_parse_asm_leading_lparen:
					JITLexer_Yield(self);
#ifndef JIT_EVAL
					if (JITLexer_SkipPair(self, '(', ')'))
						goto err;
					goto check_trailing_asm_semicolon;
#endif /* !JIT_EVAL */
				} else if (JITLexer_ISKWD(self, "pack")) {
					need_trailing_rparen = false;
					JITLexer_Yield(self);
					if (self->jl_tok == '(')
						goto do_parse_asm_leading_lparen;
				} else {
					syn_asm_expected_lparen_after_asm(self);
					goto err;
				}
				/* Parse the assembly string (which must be empty!). */
				if (self->jl_tok == JIT_STRING) {
again_eval_asm_string:
#ifdef JIT_EVAL
					{
						size_t str_len;
						str_len = JITLexer_TokLen(self) - 2;
						if (str_len != 0) {
							size_t i, len;
							DREF DeeObject *str;
							str = DeeString_FromBackslashEscaped(JITLexer_TokPtr(self) + 1,
							                                     str_len, STRING_ERROR_FSTRICT);
							if unlikely(!str)
								goto err;
							len = DeeString_WLEN(str);
							/* Make sure that the string only contains space characters! */
							for (i = 0; i < len; ++i) {
								if (!DeeUni_IsSpace(DeeString_GetChar(str, i))) {
									Dee_Decref(str);
									goto err_unsupported_assembly;
								}
							}
							Dee_Decref(str);
						}
					}
#endif /* JIT_EVAL */
					JITLexer_Yield(self);
					/* Check for further strings that would have to be concatenated. */
					if (self->jl_tok == JIT_STRING)
						goto again_eval_asm_string;
					if (self->jl_tok == JIT_RAWSTRING)
						goto again_eval_asm_rawstring;
				} else if (self->jl_tok == JIT_RAWSTRING) {
					unsigned char *str_start;
					unsigned char *str_end;
again_eval_asm_rawstring:
					str_start = self->jl_tokstart + 2;
					str_end   = self->jl_tokend - 1;
					/* Make sure that the string only contains space characters! */
					while (str_start < str_end) {
						uint32_t ch;
						ch = unicode_readutf8_n(&str_start, str_end);
						if unlikely(!DeeUni_IsSpace(ch))
							goto err_unsupported_assembly;
					}
					JITLexer_Yield(self);
					/* Check for further strings that would have to be concatenated. */
					if (self->jl_tok == JIT_STRING)
						goto again_eval_asm_string;
					if (self->jl_tok == JIT_RAWSTRING)
						goto again_eval_asm_rawstring;
				} else if (self->jl_tok == '{') {
					JITLexer_Yield(self);
					/* We mustn't encounter anything before the closing brace! (JIT doesn't support
					 * inline assembly, as it literally doesn't have an intermediate bytecode, or
					 * assembler layer) */
					if (self->jl_tok == '}') {
						JITLexer_Yield(self);
					} else {
err_unsupported_assembly:
						syn_asm_nonempty_string(self);
						goto err;
					}
				} else {
					syn_asm_expected_string_after_asm(self);
					goto err;
				}
				/* Check for assembly I/O operands. */
				if (JITLexer_IsColumn(self)) {
					JITLexer_Yield(self);
#ifdef JIT_EVAL
					if unlikely(FUNC(AssemblyOperands)(self, false)) /* OUTPUT */
						goto err;
#else /* JIT_EVAL */
					if unlikely(FUNC(AssemblyOperands)(self)) /* OUTPUT */
						goto err;
#endif /* !JIT_EVAL */
					if (JITLexer_IsColumn(self)) {
						JITLexer_Yield(self);
#ifdef JIT_EVAL
						if unlikely(FUNC(AssemblyOperands)(self, false)) /* INPUT */
							goto err;
#else /* JIT_EVAL */
						if unlikely(FUNC(AssemblyOperands)(self)) /* INPUT */
							goto err;
#endif /* !JIT_EVAL */
						if (JITLexer_IsColumn(self)) {
							JITLexer_Yield(self);
							JITLexer_SkipAssemblyClobber(self); /* CLOBBER */
							if (is_asm_goto && JITLexer_IsColumn(self)) {
								JITLexer_Yield(self);
								if unlikely(JITLexer_SkipAssemblyLabels(self)) /* LABELS */
								goto err;
							}
						}
					}
				}
				if (need_trailing_rparen) {
					if (self->jl_tok == ')') {
						JITLexer_Yield(self);
					} else {
						syn_asm_expected_rparen_after_asm(self);
						goto err;
					}
				}
#ifndef JIT_EVAL
check_trailing_asm_semicolon:
#endif /* !JIT_EVAL */
				if (self->jl_tok == ';') {
					JITLexer_Yield(self);
				} else {
					syn_asm_expected_semi_after_asm(self);
				}
#ifdef JIT_EVAL
				result = DeeNone_NewRef();
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done;
			}
			break;

		case 8: {
			uint32_t nam2;
			name = UNALIGNED_GET32(tok_begin);
			nam2 = UNALIGNED_GET32(tok_begin + 4);
			if (name == ENCODE_INT32('c', 'o', 'n', 't') &&
			    nam2 == ENCODE_INT32('i', 'n', 'u', 'e')) {
				JITLexer_Yield(self);
				if likely(self->jl_tok == ';') {
					JITLexer_Yield(self);
				} else {
					syn_continue_expected_semi_after_continue(self);
					goto err;
				}
#ifdef JIT_EVAL
				result = NULL;
				ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
				self->jl_context->jc_retval = JITCONTEXT_RETVAL_CONTINUE;
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done;
			}
		}	break;

		default: break;
		}
		goto parse_expr;
	}	break;

	default:
parse_expr:
		/* Fallback: parse a regular, old expression. */
		result = FUNC(Comma)(self,
		                     AST_COMMA_NORMAL |
		                     AST_COMMA_ALLOWVARDECLS |
		                     AST_COMMA_ALLOWTYPEDECL |
		                     AST_COMMA_PARSESEMI,
		                     IF_EVAL(NULL, )
		                     NULL);
		break;
	}
done:
	return result;
#ifdef JIT_EVAL
err_popscope:
	IF_EVAL(JITContext_PopScope(self->jl_context));
	goto err;
#endif /* JIT_EVAL */
err_r:
#ifdef JIT_EVAL
	Dee_XDecref(result);
#endif /* JIT_EVAL */
err:
	return ERROR;
}


#ifdef JIT_EVAL
#ifndef CONFIG_HAVE_memrend
#define CONFIG_HAVE_memrend
#define memrend dee_memrend
DeeSystem_DEFINE_memrend(dee_memrend)
#endif /* !CONFIG_HAVE_memrend */

/* Fill in `result->ii_symbol_name' and `result->ii_symbol_size' a module name `.foo.bar', etc. */
PRIVATE WUNUSED NONNULL((1, 2)) int DFCALL
get_module_symbol_name(JITLexer *__restrict self,
                       struct jit_import_item *__restrict result,
                       bool is_module) {
	DeeStringObject *module_name = result->ii_import_name;
	char *utf8_repr, *symbol_start;
	size_t symbol_length;
	utf8_repr = DeeString_AsUtf8((DeeObject *)module_name);
	if unlikely(!utf8_repr)
		goto err;
	symbol_start  = (char *)memrend(utf8_repr, '.', WSTR_LENGTH(utf8_repr)) + 1;
	symbol_length = (size_t)((utf8_repr + WSTR_LENGTH(utf8_repr)) - symbol_start);

	/* Make sure that the symbol name is valid. */
	{
		char *iter, *end;
		end = (iter = symbol_start) + symbol_length;
		if unlikely(!symbol_length)
			goto bad_symbol_name;
		for (; iter < end; ++iter) {
			uint32_t ch;
			uniflag_t flags;
			ch    = unicode_readutf8_n(&iter, end);
			flags = DeeUni_Flags(ch);
			if (iter == symbol_start ? !(flags & UNICODE_ISSYMSTRT)
			                         : !(flags & UNICODE_ISSYMCONT)) {
bad_symbol_name:
				if (is_module) {
					syn_import_invalid_name_for_module_symbol(self, result);
				} else {
					syn_import_invalid_name_for_import_symbol(self, result);
				}
				goto err;
			}
		}
	}

	/* Lookup/create a keyword for the module's symbol name. */
	result->ii_symbol_name = symbol_start;
	result->ii_symbol_size = symbol_length;
	return 0;
err:
	return -1;
}
#endif /* JIT_EVAL */


/* Evaluate a symbol name for an import statement and write it to `printer'
 * @return:  0: Successfully.
 * @return: -1: An error occurred. */
INTERN WUNUSED IFELSE(NONNULL((1, 2)), NONNULL((1))) int DFCALL
FUNC(SymbolNameIntoPrinter)(JITLexer *__restrict self
                            IF_EVAL(, struct unicode_printer *__restrict printer)) {
	if (self->jl_tok == JIT_KEYWORD) {
#ifdef JIT_EVAL
		if (unicode_printer_print(printer,
		                          JITLexer_TokPtr(self),
		                          JITLexer_TokLen(self)) < 0)
			goto err;
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
	} else if (self->jl_tok == TOK_STRING) {
#ifdef JIT_EVAL
		if (DeeString_DecodeBackslashEscaped(printer,
		                                     JITLexer_TokPtr(self) + 1,
		                                     JITLexer_TokLen(self) - 2,
		                                     STRING_ERROR_FSTRICT))
			goto err;
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
	} else {
		syn_import_expected_keyword_or_string_in_import_list(self);
		goto err;
	}
	return 0;
err:
	return -1;
}


/* @return:  1: OK (when `allow_module_name' is true, a module import was parsed)
 * @return:  0: OK
 * @return: -1: Error */
INTERN WUNUSED IFELSE(NONNULL((1, 2)), NONNULL((1))) int DFCALL
FUNC(ImportItem)(JITLexer *__restrict self,
                 IF_EVAL(struct jit_import_item *__restrict result, )
                 bool allow_module_name) {
#ifdef JIT_EVAL
	struct unicode_printer printer;
#endif /* JIT_EVAL */
	int return_value = 0;
	if (self->jl_tok == JIT_KEYWORD) {
		/* - `foo'
		 * - `foo = bar'
		 * - `foo = .foo.bar'
		 * - `foo = "bar"'
		 * - `foo as bar'
		 * - `foo.bar'
		 * - `foo.bar as foobar' */
#ifdef JIT_EVAL
		result->ii_symbol_name = JITLexer_TokPtr(self);
		result->ii_symbol_size = JITLexer_TokLen(self);
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
		if (self->jl_tok == '=') {
			/* - `foo = bar'
			 * - `foo = .foo.bar'
			 * - `foo = "bar"' */
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			unicode_printer_init(&printer);
			return_value = allow_module_name
			               ? JITLexer_EvalModuleNameIntoPrinter(self, &printer)
			               : JITLexer_EvalSymbolNameIntoPrinter(self, &printer);
			if unlikely(return_value < 0)
				goto err_printer;
			result->ii_import_name = (DREF DeeStringObject *)unicode_printer_pack(&printer);
			if unlikely(!result->ii_import_name)
				goto err;
#else /* JIT_EVAL */
			return_value = allow_module_name
			               ? JITLexer_SkipModuleNameIntoPrinter(self)
			               : JITLexer_SkipSymbolNameIntoPrinter(self);
			if unlikely(return_value < 0)
				goto err_printer;
#endif /* !JIT_EVAL */
		} else if (JITLexer_ISKWD(self, "as")) {
			/* - `foo as bar' */
			JITLexer_Yield(self);
			if (self->jl_tok == JIT_KEYWORD) {
#ifdef JIT_EVAL
				result->ii_import_name = (DREF DeeStringObject *)DeeString_NewUtf8(result->ii_symbol_name,
				                                                                   result->ii_symbol_size,
				                                                                   STRING_ERROR_FSTRICT);
				if unlikely(!result->ii_import_name)
					goto err;
				result->ii_symbol_name = JITLexer_TokPtr(self);
				result->ii_symbol_size = JITLexer_TokLen(self);
#endif /* JIT_EVAL */
				JITLexer_Yield(self);
			} else {
				syn_import_expected_keyword_after_as(self);
				goto err;
			}
		} else if (TOKEN_IS_DOT(self) && allow_module_name) {
			/* - `foo.bar'
			 * - `foo.bar as foobar' */
#ifdef JIT_EVAL
			unicode_printer_init(&printer);
			if unlikely(unicode_printer_print(&printer,
			                                  result->ii_symbol_name,
			                                  result->ii_symbol_size) < 0)
				goto err_printer;
#endif /* JIT_EVAL */
			goto complete_module_name;
		} else {
#ifdef JIT_EVAL
			result->ii_import_name = NULL;
#endif /* JIT_EVAL */
		}
	} else if (TOKEN_IS_DOT(self) && allow_module_name) {
		/* - `.foo.bar'
		 * - `.foo.bar as foobar' */
#ifdef JIT_EVAL
		unicode_printer_init(&printer);
#endif /* JIT_EVAL */
complete_module_name:
		return_value = 1;
#ifdef JIT_EVAL
		if unlikely(unicode_printer_printascii(&printer, "...", TOKEN_IS_DOT_count(self)) < 0)
			goto err_printer;
#endif /* JIT_EVAL */
		JITLexer_Yield(self);

		/* Make sure to properly parse `import . as me' */
		if ((self->jl_tok == JIT_KEYWORD && !JITLexer_ISTOK(self, "as")) ||
		    TOKEN_IS_DOT(self) || self->jl_tok == TOK_STRING) {
#ifdef JIT_EVAL
			if unlikely(JITLexer_EvalModuleNameIntoPrinter(self, &printer) < 0)
				goto err_printer;
#else /* JIT_EVAL */
			if unlikely(JITLexer_SkipModuleNameIntoPrinter(self) < 0)
				goto err;
#endif /* !JIT_EVAL */
		}
#ifdef JIT_EVAL
		result->ii_import_name = (DREF DeeStringObject *)unicode_printer_pack(&printer);
		if unlikely(!result->ii_import_name)
			goto err;
#endif /* JIT_EVAL */
		if (JITLexer_ISKWD(self, "as")) {
			/* - `.foo.bar as foobar' */
			JITLexer_Yield(self);
			if unlikely(self->jl_tok != JIT_KEYWORD) {
				syn_import_expected_keyword_after_as(self);
				goto err_name;
			}
#ifdef JIT_EVAL
			result->ii_symbol_name = JITLexer_TokPtr(self);
			result->ii_symbol_size = JITLexer_TokLen(self);
#endif /* JIT_EVAL */
			JITLexer_Yield(self);
		} else {
			/* - `.foo.bar' */

			/* Autogenerate the module import symbol name. */
autogenerate_symbol_name:;
#ifdef JIT_EVAL
			if unlikely(get_module_symbol_name(self, result, return_value != 0))
				goto err_name;
#endif /* JIT_EVAL */
		}
	} else if (self->jl_tok == TOK_STRING) {
		/* - `"foo"'
		 * - `"foo" as foobar'
		 * - `"foo.bar"'
		 * - `"foo.bar" as foobar' */
#ifdef JIT_EVAL
		unicode_printer_init(&printer);
		return_value = allow_module_name
		               ? JITLexer_EvalModuleNameIntoPrinter(self, &printer)
		               : JITLexer_EvalSymbolNameIntoPrinter(self, &printer);
#else /* JIT_EVAL */
		return_value = allow_module_name
		               ? JITLexer_SkipModuleNameIntoPrinter(self)
		               : JITLexer_SkipSymbolNameIntoPrinter(self);
#endif /* !JIT_EVAL */
		if unlikely(return_value < 0)
			goto err_printer;
#ifdef JIT_EVAL
		result->ii_import_name = (DREF DeeStringObject *)unicode_printer_pack(&printer);
		if unlikely(!result->ii_import_name)
			goto err;
#endif /* JIT_EVAL */
		if (!JITLexer_ISKWD(self, "as"))
			goto autogenerate_symbol_name;

		/* An import alias was given. */
		JITLexer_Yield(self);
		if unlikely(self->jl_tok != JIT_KEYWORD) {
			syn_import_expected_keyword_after_as(self);
			goto err_name;
		}
#ifdef JIT_EVAL
		result->ii_symbol_name = JITLexer_TokPtr(self);
		result->ii_symbol_size = JITLexer_TokLen(self);
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
	} else {
		syn_import_expected_keyword_or_string_in_import_list(self);
		goto err;
	}
	return return_value;
err_printer:
#ifdef JIT_EVAL
	unicode_printer_fini(&printer);
	goto err;
#endif /* JIT_EVAL */
err_name:
#ifdef JIT_EVAL
	Dee_Decref(result->ii_import_name);
#endif /* JIT_EVAL */
err:
	return -1;
}


/* NOTE: Unlike other statements, the Import-statement parsers expect the
 *       lexer to point *after* the leading `import' or `from' keyword */
INTERN RETURN_TYPE DFCALL
FUNC(Import)(JITLexer *__restrict self) {
#ifdef JIT_EVAL
	struct jit_import_item item;
	DREF DeeObject *mod;
	struct jit_import_item *item_v;
	size_t item_a, item_c;
#endif /* JIT_EVAL */
	bool allow_modules, has_star;
	int error;

	/* Special handling for `import(...)' expressions. */
	if (self->jl_tok == '(' || JITLexer_ISKWD(self, "pack")) {
#ifdef JIT_EVAL
		RETURN_TYPE result;
		result = JITLexer_EvalImportExpression(self);
		if unlikely(!result)
			goto err;
#endif /* JIT_EVAL */
		return CALL_SECONDARY(Operand, result);
	}

	/* - import deemon;
	 * - import deemon, util;
	 * - import my_deemon = deemon;
	 * - import my_deemon = "deemon";
	 * - import my_deemon = deemon, my_util = util;
	 * - import deemon as my_deemon;
	 * - import "deemon" as my_deemon;
	 * - import deemon as my_deemon, util as my_util;
	 * - import * from deemon;
	 * - import Object from deemon;
	 * - import Object, List from deemon;
	 * - import MyObject = Object from deemon;
	 * - import MyObject = "Object" from deemon;
	 * - import MyObject = Object, MyList = List from deemon;
	 * - import Object as MyObject from deemon;
	 * - import "Object" as MyObject from deemon;
	 * - import Object as MyObject, List as MyList from deemon;
	 *
	 * NOTE: *-import is facilitated via `JITObjectTable_AddImportStar(JITContext_GetRWLocals(self->jl_context))'
	 * NOTE: single-import is facilitated via `JITObjectTable_Create(JITContext_GetRWLocals(self->jl_context))',
	 *       and then creating `JIT_OBJECT_ENTRY_EXTERN_SYMBOL' or `JIT_OBJECT_ENTRY_EXTERN_ATTR[STR]' symbols.
	 */

	allow_modules = true;
	has_star      = false; /* When true, import all */
	if (self->jl_tok == '*') {
		has_star = true;
		JITLexer_Yield(self);
		if (JITLexer_ISKWD(self, "from")) {
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			mod = JITLexer_EvalModule(self);
			if unlikely(!mod)
				goto err;
			if unlikely(JITContext_DoImportStar(self->jl_context, mod))
				goto err_mod_invoke;
			Dee_Decref(mod);
#else /* JIT_EVAL */
			if unlikely(JITLexer_SkipModule(self))
				goto err;
#endif /* !JIT_EVAL */
			goto done;
		} else if (self->jl_tok == ',') {
#ifdef JIT_EVAL
			item_c = 0;
			item_a = 0;
			item_v = NULL;
#endif /* JIT_EVAL */
			allow_modules = false;
			goto import_parse_list;
		}
		syn_import_expected_comma_or_from_after_star(self);
		goto err;
	}
#ifdef JIT_EVAL
	error = JITLexer_EvalImportItem(self, &item, true);
#else /* JIT_EVAL */
	error = JITLexer_SkipImportItem(self, true);
#endif /* !JIT_EVAL */
	if unlikely(error < 0)
		goto err;
	if (error) {
		/* Module import list */
		for (;;) {
import_item_as_module:
#ifdef JIT_EVAL
			if (JITContext_DoImportModule(self->jl_context, &item))
				goto err_item_invoke;
			Dee_XDecref(item.ii_import_name);
#endif /* JIT_EVAL */
parse_module_import_list:
			if (self->jl_tok != ',')
				break;
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			error = JITLexer_EvalImportItem(self, &item, true);
#else /* JIT_EVAL */
			error = JITLexer_SkipImportItem(self, true);
#endif /* !JIT_EVAL */
			if unlikely(error < 0)
				goto err;
		}

		/* Warn if the module import list is followed by a `from' */
		if (JITLexer_ISKWD(self, "from")) {
			syn_import_unexpected_from_after_module_import_list(self);
			goto err;
		}
	} else if (JITLexer_ISKWD(self, "from")) {
		/*  - `import foo from bar' */
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		mod = JITLexer_EvalModule(self);
		if unlikely(!mod)
			goto err_item;
		error = JITContext_DoImportSymbol(self->jl_context, &item, mod);
		Dee_Decref(mod);
		if unlikely(error)
			goto err_item_invoke;
		Dee_XDecref(item.ii_import_name);
#else /* JIT_EVAL */
		if unlikely(JITLexer_SkipModule(self))
			goto err;
#endif /* !JIT_EVAL */
	} else if (self->jl_tok == ',') {
#ifdef JIT_EVAL
		item_a = 4;
		item_v = (struct jit_import_item *)Dee_TryMallocc(4, sizeof(struct jit_import_item));
		if unlikely(!item_v) {
			item_a = 2;
			item_v = (struct jit_import_item *)Dee_Mallocc(2, sizeof(struct jit_import_item));
			if unlikely(!item_v)
				goto err_item;
		}
		item_v[0] = item;
		item_c    = 1;
#endif /* JIT_EVAL */
import_parse_list:
		do {
			ASSERT(self->jl_tok == ',');
			JITLexer_Yield(self);
			if (self->jl_tok == '*') {
				if (has_star) {
					syn_import_unexpected_star_duplication_in_import_list(self);
					goto err_item_v;
				}
				has_star = true;
				JITLexer_Yield(self);

				/* Don't allow modules after `*' confirmed that symbols are being imported.
				 * -> There is no such thing as import-all-modules. */
				allow_modules = false;
			} else {
				/* Parse the next import item. */
#ifdef JIT_EVAL
				ASSERT(item_c <= item_a);
				if (item_c >= item_a) {
					/* Allocate more space. */
					struct jit_import_item *new_item_v;
					size_t new_item_a = item_a * 2;
					if unlikely(!new_item_a)
						new_item_a = 2;
					new_item_v = (struct jit_import_item *)Dee_TryReallocc(item_v, new_item_a,
					                                                       sizeof(struct jit_import_item));
					if unlikely(!new_item_v) {
						new_item_a = item_c + 1;
						new_item_v = (struct jit_import_item *)Dee_Reallocc(item_v, new_item_a,
						                                                    sizeof(struct jit_import_item));
						if unlikely(!new_item_v)
							goto err_item_v;
					}
					item_v = new_item_v;
					item_a = new_item_a;
				}
				error = JITLexer_EvalImportItem(self, &item_v[item_c], allow_modules);
#else /* JIT_EVAL */
				error = JITLexer_SkipImportItem(self, allow_modules);
#endif /* !JIT_EVAL */
				if unlikely(error < 0)
					goto err_item_v;
#ifdef JIT_EVAL
				++item_c; /* Import parsing confirmed. */
#endif /* JIT_EVAL */
				if (error) {
					/* We're dealing with a module import list!
					 * -> Import all items already parsed as modules. */
					ASSERT(allow_modules != false);
#ifdef JIT_EVAL
					{
						size_t i;
						for (i = 0; i < item_c; ++i) {
							if unlikely(JITContext_DoImportModule(self->jl_context, &item_v[i]))
								goto err_item_v_invoke;
							Dee_XClear(item_v[i].ii_import_name);
						}
						Dee_Free(item_v);
					}
#endif /* JIT_EVAL */
					/* Then continue by parsing the remainder as a module import list. */
					goto parse_module_import_list;
				}
			}
		} while (self->jl_tok == ',');

		/* A multi-item, comma-separated import list has now been parsed. */
		if (JITLexer_ISKWD(self, "from")) {

			/* import foo, bar, foobar from foobarfoo;  (symbol import) */
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			mod = JITLexer_EvalModule(self);
			if unlikely(!mod)
				goto err_item_v;
#else /* JIT_EVAL */
			if unlikely(JITLexer_SkipModule(self))
				goto err_item_v;
#endif /* !JIT_EVAL */

#ifdef JIT_EVAL
			/* If `*' was apart of the symbol import list,
			 * start by importing all symbols from the module. */
			if (has_star) {
				if unlikely(JITContext_DoImportStar(self->jl_context, mod))
					goto err_item_v_mod_invoke;
			}

			/* Now import all the explicitly defined symbols. */
			{
				size_t i;
				for (i = 0; i < item_c; ++i) {
					if unlikely(JITContext_DoImportSymbol(self->jl_context, &item_v[i], mod))
						goto err_item_v_mod_invoke;
					Dee_XClear(item_v[i].ii_import_name);
				}
				Dee_Decref(mod);
			}
#endif /* JIT_EVAL */
		} else {
			if unlikely(!allow_modules) {
				/* Warn if there is a `from' missing following a symbol import list. */
				syn_import_expected_from_after_symbol_import_list(self);
				goto err_item_v;
			}

			/* import foo, bar, foobar;  (module import) */
#ifdef JIT_EVAL
			{
				size_t i;
				for (i = 0; i < item_c; ++i) {
					if unlikely(JITContext_DoImportModule(self->jl_context, &item_v[i]))
						goto err_item_v_invoke;
					Dee_XClear(item_v[i].ii_import_name);
				}
			}
#endif /* JIT_EVAL */
		}
#ifdef JIT_EVAL
		Dee_Free(item_v);
#endif /* JIT_EVAL */
	} else {
		/* This is simply a single-module, stand-along import statement. */
		goto import_item_as_module;
	}
done:
#ifdef JIT_EVAL
	return_none;
#else /* JIT_EVAL */
	return 0;
#endif /* !JIT_EVAL */
#ifdef JIT_EVAL
err_item_v_mod_invoke:
	Dee_Decref(mod);
err_item_v_invoke:
	JITLexer_ErrorTrace(self, self->jl_tokstart);
err_item_v:
	while (item_c--)
		Dee_XDecref(item_v[item_c].ii_import_name);
	Dee_Free(item_v);
	goto err;
err_item_invoke:
	JITLexer_ErrorTrace(self, self->jl_tokstart);
err_item:
	Dee_XDecref(item.ii_import_name);
	goto err;
err_mod_invoke:
	JITLexer_ErrorTrace(self, self->jl_tokstart);
/*err_mod:*/
	Dee_Decref(mod);
#else /* JIT_EVAL */
err_item_v:
#endif /* !JIT_EVAL */
err:
	return ERROR;
}

INTERN RETURN_TYPE DFCALL
FUNC(FromImport)(JITLexer *__restrict self) {
	bool has_star = false;
#ifdef JIT_EVAL
	DREF DeeObject *mod;
#endif /* JIT_EVAL */

	/* - from deemon import *;
	 * - from deemon import Object;
	 * - from deemon import Object, List;
	 * - from deemon import MyObject = Object;
	 * - from deemon import MyObject = "Object";
	 * - from deemon import MyObject = Object, MyList = List;
	 * - from deemon import Object as MyObject;
	 * - from deemon import "Object" as MyObject;
	 * - from deemon import Object as MyObject, List as MyList; */
#ifdef JIT_EVAL
	mod = JITLexer_EvalModule(self);
	if unlikely(!mod)
		goto err;
#else /* JIT_EVAL */
	if unlikely(JITLexer_SkipModule(self))
		goto err;
#endif /* !JIT_EVAL */

	/* All right! we've got the module. */
	if (!JITLexer_ISKWD(self, "import")) {
		syn_import_expected_import_after_from(self);
		goto err_mod;
	}
	JITLexer_Yield(self);

	for (;;) {
		/* Parse an entire import list. */
		if (self->jl_tok == '*') {
			if (has_star) {
				syn_import_unexpected_star_duplication_in_import_list(self);
				goto err_mod;
			}
#ifdef JIT_EVAL
			if (JITContext_DoImportStar(self->jl_context, mod))
				goto err_mod_invoke;
#endif /* JIT_EVAL */
			JITLexer_Yield(self);
			has_star = true;
		} else {
#ifdef JIT_EVAL
			struct jit_import_item item;
#endif /* JIT_EVAL */
			int error;
#ifdef JIT_EVAL
			error = JITLexer_EvalImportItem(self, &item, false);
#else /* JIT_EVAL */
			error = JITLexer_SkipImportItem(self, false);
#endif /* !JIT_EVAL */
			if unlikely(error < 0)
				goto err_mod;
#ifdef JIT_EVAL
			error = JITContext_DoImportSymbol(self->jl_context, &item, mod);
			Dee_XDecref(item.ii_import_name);
			if unlikely(error)
				goto err_mod_invoke;
#endif /* JIT_EVAL */
		}
		if (self->jl_tok != ',')
			break;
		JITLexer_Yield(self);
	}
#ifdef JIT_EVAL
	Dee_Decref(mod);
#endif /* JIT_EVAL */

#ifdef JIT_EVAL
	return_none;
#else /* JIT_EVAL */
	return 0;
#endif /* !JIT_EVAL */
#ifdef JIT_EVAL
err_mod_invoke:
	JITLexer_ErrorTrace(self, self->jl_tokstart);
#endif /* JIT_EVAL */
err_mod:
#ifdef JIT_EVAL
	Dee_Decref(mod);
#endif /* JIT_EVAL */
err:
	return ERROR;
}


DECL_END

