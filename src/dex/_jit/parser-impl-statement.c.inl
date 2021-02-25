/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "parser-impl.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/thread.h>

DECL_BEGIN

INTERN RETURN_TYPE FCALL
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
LOCAL WUNUSED bool FCALL
JITLexer_IsColumn(JITLexer *__restrict self) {
	switch (self->jl_tok) {
	case ':': break;
	case TOK_COLLON_EQUAL:
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
LOCAL WUNUSED int FCALL
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


LOCAL WUNUSED int FCALL
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
LOCAL void FCALL
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
LOCAL WUNUSED int FCALL
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



INTERN RETURN_TYPE FCALL
FUNC(Statement)(JITLexer *__restrict self) {
	RETURN_TYPE result;
	switch (self->jl_tok) {

	case ';':
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		result = Dee_None;
		Dee_Incref(Dee_None);
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
		break;

	case '{':
		JITLexer_Yield(self);
		if (self->jl_tok == '}') {
#ifdef JIT_EVAL
			result = Dee_None;
			Dee_Incref(Dee_None);
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
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('w', 'i', 't', 'h')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(With)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('f', 'r', 'o', 'm')) {
				result = FUNC(Import)(self, true);
				goto done;
			}
			break;

		case 5:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('t', 'h', 'r', 'o') &&
			    *(uint8_t *)(tok_begin + 4) == 'w') {
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
					DeeError_Throw(result);
					Dee_Clear(result);
#endif /* JIT_EVAL */
				}
				goto done;
			}
			if (name == ENCODE_INT32('w', 'h', 'i', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'e') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(While)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('y', 'i', 'e', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'd') {
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
			    *(uint8_t *)(tok_begin + 4) == 't') {
				/* TODO */
				DERROR_NOTIMPLEMENTED();
				goto err;
			}
			if (name == ENCODE_INT32('b', 'r', 'e', 'a') &&
			    *(uint8_t *)(tok_begin + 4) == 'k') {
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
			    *(uint8_t *)(tok_begin + 4) == 'm')
				goto do_asm;
			break;

		case 6:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('r', 'e', 't', 'u') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 'n')) {
				JITLexer_Yield(self);
				if (self->jl_tok == ';') {
					JITLexer_Yield(self);
#ifdef JIT_EVAL
					ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
					self->jl_context->jc_retval = Dee_None;
					Dee_Incref(Dee_None);
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
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 't')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(Assert)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('i', 'm', 'p', 'o') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 't')) {
				result = FUNC(Import)(self, false);
				goto done;
			}
			if (name == ENCODE_INT32('s', 'w', 'i', 't') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('c', 'h')) {
				/* TODO */
				DERROR_NOTIMPLEMENTED();
				goto err;
			}
			break;

		case 7:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('a', 'c') &&
			    *(uint8_t *)(tok_begin + 6) == 'h') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(Foreach)(self, true);
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				goto done;
			}
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('m', '_') &&
			    *(uint8_t *)(tok_begin + 6) == '_') {
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
	 memcmp(kwd_start, x, COMPILER_STRLEN(x) * sizeof(char)) == 0)
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
					goto check_trailing_asm_semicollon;
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
						ch = utf8_readchar((char const **)&str_start, (char const *)str_end);
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
check_trailing_asm_semicollon:
#endif /* !JIT_EVAL */
				if (self->jl_tok == ';') {
					JITLexer_Yield(self);
				} else {
					syn_asm_expected_semi_after_asm(self);
				}
#ifdef JIT_EVAL
				result = Dee_None;
				Dee_Incref(Dee_None);
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done;
			}
			break;

		case 8: {
			uint32_t nam2;
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			nam2 = UNALIGNED_GET32((uint32_t *)(tok_begin + 4));
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

DECL_END

