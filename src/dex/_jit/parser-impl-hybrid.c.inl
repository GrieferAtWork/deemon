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

DECL_BEGIN

INTERN RETURN_TYPE FCALL
FUNC(StatementOrBraces)(JITLexer *__restrict self,
                        unsigned int *pwas_expression) {
	RETURN_TYPE result;
	unsigned int was_expression;
	ASSERT(self->jl_tok == '{');
	JITLexer_Yield(self);
	switch (self->jl_tok) {

	case '}':
		/* Special case: empty sequence. */
		result = IFELSE(Dee_EmptySeq, 0);
		IF_EVAL(Dee_Incref(Dee_EmptySeq));
		JITLexer_Yield(self);
		if (pwas_expression)
			*pwas_expression = AST_PARSE_WASEXPR_MAYBE;
		break;

	case '.':
		result = FUNC(BraceItems)(self);
		if (ISERR(result))
			goto err;
		if likely(self->jl_tok == '}') {
			JITLexer_Yield(self);
		} else {
err_rbrace_missing:
			syn_brace_expected_rbrace(self);
			goto err_r;
		}
		if (pwas_expression)
			*pwas_expression = AST_PARSE_WASEXPR_YES;
		break;

	case '{': /* Recursion! */
		IF_EVAL(JITContext_PushScope(self->jl_context));
		result = FUNC(StatementOrBraces)(self, &was_expression);
parse_remainder_after_hybrid_popscope:
		if (ISERR(result))
			goto err;
parse_remainder_after_hybrid_popscope_resok:
		if (was_expression == AST_PARSE_WASEXPR_NO)
			goto parse_remainder_after_statement;
		if (was_expression == AST_PARSE_WASEXPR_YES) {
			result = CALL_SECONDARY(Operand, result);
			if (ISERR(result))
				goto err;
check_recursion_after_expression_suffix:
			if (self->jl_tok == ';') {
				JITLexer_Yield(self);
				goto parse_remainder_after_statement;
			}
			if (self->jl_tok == ':')
				goto parse_remainder_after_colon_popscope;
			if (self->jl_tok == ',' || self->jl_tok == TOK_DOTS) {
parse_remainder_after_comma_popscope:
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				result = CALL_SECONDARY(CommaListOperand, result);
				if (ISERR(result))
					goto err;
				if likely(self->jl_tok == '}') {
					JITLexer_Yield(self);
				} else {
					goto err_rbrace_missing;
				}
				if (pwas_expression)
					*pwas_expression = AST_PARSE_WASEXPR_YES;
				break;
			}
			if likely(self->jl_tok == '}') {
parse_remainder_before_rbrace_popscope_wrap:
				JITLexer_Yield(self);
			} else {
				goto err_rbrace_missing;
			}
			/* Wrap the result as a single sequence. */
#ifdef JIT_EVAL
			{
				DREF DeeObject *result_list;
				result_list = DeeList_NewVectorInherited(1, (DeeObject *const *)&result);
				if unlikely(!result_list)
					goto err_r;
				result = result_list;
			}
#endif /* JIT_EVAL */
			goto parse_remainder_after_rbrace_popscope;
		}
		if (self->jl_tok == ',' || self->jl_tok == TOK_DOTS)
			goto parse_remainder_after_comma_popscope;
		if (self->jl_tok == ':')
			goto parse_remainder_after_colon_popscope;
		if (self->jl_tok == '}')
			goto parse_remainder_before_rbrace_popscope_wrap;
		{
			unsigned char *token_start = self->jl_tokstart;
			result                     = CALL_SECONDARY(Operand, result);
			if (ISERR(result))
				goto err;
			if (token_start != self->jl_tokstart)
				goto check_recursion_after_expression_suffix;
		}
		goto parse_remainder_after_statement;

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
				result = FUNC(IfHybrid)(self, &was_expression);
				goto parse_remainder_after_hybrid_popscope;
			}
			if (tok_begin[0] == 'd' && tok_begin[1] == 'o') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(DoHybrid)(self, &was_expression);
				goto parse_remainder_after_hybrid_popscope;
			}
			break;

		case 3:
			if (tok_begin[0] == 't' && tok_begin[1] == 'r' &&
			    tok_begin[2] == 'y') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(TryHybrid)(self, &was_expression);
				goto parse_remainder_after_hybrid_popscope;
			}
			if (tok_begin[0] == 'f' && tok_begin[1] == 'o' &&
			    tok_begin[2] == 'r') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(ForHybrid)(self, &was_expression);
				goto parse_remainder_after_hybrid_popscope;
			}
			if (tok_begin[0] == 'd' && tok_begin[1] == 'e' &&
			    tok_begin[2] == 'l') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(DelHybrid)(self, &was_expression);
				goto parse_remainder_after_semicolon_hybrid_popscope;
			}
			break;

		case 4:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('w', 'i', 't', 'h')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(WithHybrid)(self, &was_expression);
				goto parse_remainder_after_hybrid_popscope;
			}
			if (name == ENCODE_INT32('f', 'r', 'o', 'm')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(ImportHybrid)(self, true, &was_expression);
				if (ISERR(result))
					goto err;
				ASSERT(was_expression == AST_PARSE_WASEXPR_NO);
				/* Same as `assert': `import' requires a trailing `;' */
				goto parse_remainder_after_semicolon_hybrid_popscope;
			}
			break;

		case 5:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('t', 'h', 'r', 'o') &&
			    *(uint8_t *)(tok_begin + 4) == 'w')
				goto is_a_statement;
			if (name == ENCODE_INT32('y', 'i', 'e', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'd')
				goto is_a_statement;
			if (name == ENCODE_INT32('p', 'r', 'i', 'n') &&
			    *(uint8_t *)(tok_begin + 4) == 't')
				goto is_a_statement;
			if (name == ENCODE_INT32('b', 'r', 'e', 'a') &&
			    *(uint8_t *)(tok_begin + 4) == 'k')
				goto is_a_statement;
			if (name == ENCODE_INT32('w', 'h', 'i', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'e') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(WhileHybrid)(self, &was_expression);
				goto parse_remainder_after_hybrid_popscope;
			}
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    *(uint8_t *)(tok_begin + 4) == 'm')
				goto is_a_statement;
			break;

		case 6:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('a', 's', 's', 'e') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 't')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(AssertHybrid)(self, &was_expression);
parse_remainder_after_semicolon_hybrid_popscope:
				if (ISERR(result))
					goto err;
				/* Special case: `assert' statements require a trailing `;' token.
				 *                If that token exists, we know for sure that this is a statement! */
				if (self->jl_tok == ';') {
					was_expression = AST_PARSE_WASEXPR_NO;
					JITLexer_Yield(self);
				}
				goto parse_remainder_after_hybrid_popscope_resok;
			}
			if (name == ENCODE_INT32('i', 'm', 'p', 'o') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 't')) {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(ImportHybrid)(self, false, &was_expression);
				if (ISERR(result))
					goto err;
				/* Same as `assert': `import' requires a trailing `;' */
				goto parse_remainder_after_semicolon_hybrid_popscope;
			}
			if (name == ENCODE_INT32('s', 'w', 'i', 't') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('c', 'h'))
				goto is_a_statement;
			if (name == ENCODE_INT32('r', 'e', 't', 'u') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 'n'))
				goto is_a_statement;
			break;

		case 7:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('a', 'c') &&
			    *(uint8_t *)(tok_begin + 6) == 'h') {
				IF_EVAL(JITContext_PushScope(self->jl_context));
				result = FUNC(ForeachHybrid)(self, &was_expression);
				goto parse_remainder_after_hybrid_popscope;
			}
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('m', '_') &&
			    *(uint8_t *)(tok_begin + 6) == '_')
				goto is_a_statement;
			break;

		case 8: {
			uint32_t nam2;
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			nam2 = UNALIGNED_GET32((uint32_t *)(tok_begin + 4));
			if (name == ENCODE_INT32('c', 'o', 'n', 't') &&
			    nam2 == ENCODE_INT32('i', 'n', 'u', 'e'))
				goto is_a_statement;
		}	break;

		default: break;
		}
		goto default_case;
	}	break;


	case '@':
	case ';':
is_a_statement:
		IF_EVAL(JITContext_PushScope(self->jl_context));
		/* Enter a new scope and parse expressions. */
		result = FUNC(StatementBlock)(self);
		if (ISERR(result))
			goto err;
		LOAD_LVALUE(result, err_popscope);
		IF_EVAL(JITContext_PopScope(self->jl_context));
		if (pwas_expression)
			*pwas_expression = AST_PARSE_WASEXPR_NO;
		break;

	default: {
		uint16_t comma_mode;
		IF_EVAL(JITObjectTable * old_tab;)
default_case:
#if 0
		/* Check for a label definition. */
		if (self->jl_tok == JIT_KEYWORD) {
			JITSmallLexer smlex;
			memcpy(&smlex, self, sizeof(JITSmallLexer));
			JITLexer_Yield((JITLexer *)&smlex);
			if (smlex.jl_tok == ':')
				goto is_a_statement; /* label */
		}
#endif
		/* Figure out what we're dealing with as we go. */
		IF_EVAL(JITContext_PushScope(self->jl_context));
		IF_EVAL(old_tab = JITContext_GetROLocals(self->jl_context);)
		comma_mode = 0;
		result = FUNC(Comma)(self,
		                     AST_COMMA_NORMAL |
		                     /*AST_COMMA_FORCEMULTIPLE |*/
		                     AST_COMMA_ALLOWVARDECLS |
		                     AST_COMMA_ALLOWTYPEDECL |
		                     AST_COMMA_PARSESEMI,
		                     IF_EVAL(&DeeTuple_Type, ) & comma_mode);
		if (ISERR(result))
			goto err;
#ifdef JIT_EVAL
		if (old_tab == JITContext_GetROLocals(self->jl_context))
#endif /* JIT_EVAL */
		{
			if (self->jl_tok == '}') {
#ifdef JIT_EVAL
				/* The NEEDSEMI flag is set when only a single expression was parsed.
				 * If this was the case, then we must pack that expression in a sequence. */
				if ((comma_mode & (AST_COMMA_OUT_FNEEDSEMI | AST_COMMA_OUT_FMULTIPLE)) ==
				    /*         */ (AST_COMMA_OUT_FNEEDSEMI)) {
					DREF DeeObject *seq;
					seq = DeeTuple_NewVectorSymbolic(1, (DeeObject *const *)&result);
					if unlikely(!seq)
						goto err_r;
					result = seq;
				}
#endif /* JIT_EVAL */
/*parse_remainder_before_rbrace_popscope:*/
				/* Sequence-like brace expression. */
				JITLexer_Yield(self);
parse_remainder_after_rbrace_popscope:
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				if (pwas_expression)
					*pwas_expression = AST_PARSE_WASEXPR_YES;
				break;
			}
			if (self->jl_tok == ':' &&
			    !(comma_mode & AST_COMMA_OUT_FMULTIPLE)) {
parse_remainder_after_colon_popscope:
				LOAD_LVALUE(result, err_popscope);
				IF_EVAL(JITContext_PopScope(self->jl_context));
				/* mapping-like brace expression. */
				result = CALL_SECONDARY(CommaDictOperand, result);
				if (ISERR(result))
					goto err;
				if unlikely(self->jl_tok != '}')
					goto err_rbrace_missing;
				JITLexer_Yield(self);
				if (pwas_expression)
					*pwas_expression = AST_PARSE_WASEXPR_YES;
				break;
			}
		}
		/* Statement expression. */
		if (comma_mode & AST_COMMA_OUT_FNEEDSEMI) {
			/* Consume a `;' token as part of the expression. */
			if likely(self->jl_tok == ';') {
#ifdef JIT_EVAL
				if (comma_mode & AST_COMMA_OUT_FMULTIPLE) {
					/* >> ({ "foo", 42; })
					 * >> result        = ("foo", 42);
					 * >> wanted_result = 42; */
					DREF DeeObject *wanted_result;
					ASSERT(DeeTuple_Check(result));
					wanted_result = DeeTuple_GET(result, DeeTuple_SIZE(result) - 1);
					Dee_Incref(wanted_result);
					Dee_Decref(result);
					result = wanted_result;
				}
#endif /* JIT_EVAL */
				JITLexer_Yield(self);
			} else {
				syn_expr_expected_semi_after_expr(self);
				goto err_r;
			}
		}
parse_remainder_after_statement:
		if (self->jl_tok == '}') {
			JITLexer_Yield(self);
		} else {
#ifdef JIT_EVAL
			if (result == JIT_LVALUE) {
				JITLValue_Fini(&self->jl_lvalue);
				JITLValue_Init(&self->jl_lvalue);
			} else {
				Dee_Decref(result);
			}
#endif /* JIT_EVAL */
			/* Parse the remainder */
			result = FUNC(StatementBlock)(self);
			if (ISERR(result))
				goto err;
		}
		LOAD_LVALUE(result, err_popscope);
		IF_EVAL(JITContext_PopScope(self->jl_context));
		if (pwas_expression)
			*pwas_expression = AST_PARSE_WASEXPR_NO;
	}	break;
	}
	return result;
#ifdef JIT_EVAL
err_popscope:
	IF_EVAL(JITContext_PopScope(self->jl_context));
	goto err;
#endif /* JIT_EVAL */
err_r:
	DECREF_MAYBE_LVALUE(result);
err:
	return ERROR;
}

INTERN RETURN_TYPE FCALL
FUNC(Hybrid)(JITLexer *__restrict self,
             unsigned int *pwas_expression) {
	unsigned int was_expression;
	RETURN_TYPE result;
	switch (self->jl_tok) {

	case '{':
		result = FUNC(StatementOrBraces)(self, &was_expression);
parse_unary_suffix_if_notexpr:
		if (ISERR(result))
			goto err;
		if (was_expression != AST_PARSE_WASEXPR_NO) {
			/* Try to parse a suffix expression.
			 * If there was one, then we know that it actually was an expression. */
			unsigned char *token_start;
			token_start = self->jl_tokstart;
			result      = CALL_SECONDARY(Operand, result);
			if (token_start != self->jl_tokstart)
				was_expression = AST_PARSE_WASEXPR_YES;
		}
check_semi_after_expression:
		if (ISERR(result))
			goto err;
		if (self->jl_tok == ';' && was_expression != AST_PARSE_WASEXPR_NO) {
			JITLexer_Yield(self);
			was_expression = AST_PARSE_WASEXPR_NO;
		}
		if (pwas_expression)
			*pwas_expression = was_expression;
		break;

	case '@':
	case ';':
is_a_statement:
		result = FUNC(Statement)(self);
		if (pwas_expression)
			*pwas_expression = AST_PARSE_WASEXPR_NO;
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
				/* Expression:           `if (foo) bar else baz'   (may also be written as `(if (foo) bar else baz)') */
				/* Expression-Statement: `if (foo) bar else baz;'  (may also be written as `(if (foo) bar else baz);') */
				/* Pure Statement:       `if (foo) bar; else baz;' (this would be a syntax error: `(if (foo) bar; else baz);') */
				/* Pure Statement:       `if (foo) { bar; } else { baz; }' */
				result = FUNC(IfHybrid)(self, &was_expression);
				goto check_semi_after_expression;
			}
			if (tok_begin[0] == 'd' && tok_begin[1] == 'o') {
				result = FUNC(DoHybrid)(self, &was_expression);
				goto parse_unary_suffix_if_notexpr;
			}
			break;

		case 3:
			if (tok_begin[0] == 't' && tok_begin[1] == 'r' &&
			    tok_begin[2] == 'y') {
				result = FUNC(TryHybrid)(self, &was_expression);
				goto check_semi_after_expression;
			}
			if (tok_begin[0] == 'f' && tok_begin[1] == 'o' &&
			    tok_begin[2] == 'r') {
				result = FUNC(ForHybrid)(self, &was_expression);
				goto check_semi_after_expression;
			}
			if (tok_begin[0] == 'd' && tok_begin[1] == 'e' &&
			    tok_begin[2] == 'l') {
				result = FUNC(DelHybrid)(self, &was_expression);
				goto parse_unary_suffix_if_notexpr;
			}
			break;

		case 4:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('w', 'i', 't', 'h')) {
				result = FUNC(WithHybrid)(self, &was_expression);
				goto check_semi_after_expression;
			}
			if (name == ENCODE_INT32('f', 'r', 'o', 'm')) {
				result = FUNC(ImportHybrid)(self, true, &was_expression);
				goto check_semi_after_expression;
			}
			break;

		case 5:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('w', 'h', 'i', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'e') {
				result = FUNC(WhileHybrid)(self, &was_expression);
				goto check_semi_after_expression;
			}
			if (name == ENCODE_INT32('y', 'i', 'e', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'd')
				goto is_a_statement;
			if (name == ENCODE_INT32('t', 'h', 'r', 'o') &&
			    *(uint8_t *)(tok_begin + 4) == 'w')
				goto is_a_statement;
			if (name == ENCODE_INT32('p', 'r', 'i', 'n') &&
			    *(uint8_t *)(tok_begin + 4) == 't')
				goto is_a_statement;
			if (name == ENCODE_INT32('b', 'r', 'e', 'a') &&
			    *(uint8_t *)(tok_begin + 4) == 'k')
				goto is_a_statement;
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    *(uint8_t *)(tok_begin + 4) == 'm')
				goto is_a_statement;
			break;

		case 6:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('r', 'e', 't', 'u') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 'n'))
				goto is_a_statement;
			if (name == ENCODE_INT32('a', 's', 's', 'e') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 't')) {
				result = FUNC(AssertHybrid)(self, &was_expression);
				goto check_semi_after_expression;
			}
			if (name == ENCODE_INT32('i', 'm', 'p', 'o') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('r', 't')) {
				result = FUNC(ImportHybrid)(self, false, &was_expression);
				goto parse_unary_suffix_if_notexpr;
			}
			if (name == ENCODE_INT32('s', 'w', 'i', 't') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('c', 'h'))
				goto is_a_statement;
			break;

		case 7:
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('a', 'c') &&
			    *(uint8_t *)(tok_begin + 6) == 'h') {
				result = FUNC(ForeachHybrid)(self, &was_expression);
				goto check_semi_after_expression;
			}
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE_INT16('m', '_') &&
			    *(uint8_t *)(tok_begin + 6) == '_')
				goto is_a_statement;
			break;

		case 8: {
			uint32_t nam2;
			name = UNALIGNED_GET32((uint32_t *)tok_begin);
			nam2 = UNALIGNED_GET32((uint32_t *)(tok_begin + 4));
			if (name == ENCODE_INT32('c', 'o', 'n', 't') &&
			    nam2 == ENCODE_INT32('i', 'n', 'u', 'e'))
				goto is_a_statement;
		}	break;

		default:
			break;
		}
	}	goto default_case;

	default: {
		uint16_t comma_mode;
#ifdef JIT_EVAL
		size_t old_varc;
		JITObjectTable *old_tab;
#endif /* JIT_EVAL */
default_case:
#ifdef JIT_EVAL
		old_tab  = JITContext_GetROLocals(self->jl_context);
		old_varc = old_tab ? old_tab->ot_size : 0;
#endif /* JIT_EVAL */
		comma_mode = 0;
		result = FUNC(Comma)(self,
		                     AST_COMMA_PARSESINGLE |
		                     AST_COMMA_NOSUFFIXKWD |
		                     AST_COMMA_ALLOWVARDECLS |
		                     AST_COMMA_PARSESEMI,
		                     IF_EVAL(NULL, ) & comma_mode);
		if (ISERR(result))
			goto err;
		if (self->jl_tok == ';' && (comma_mode & AST_COMMA_OUT_FNEEDSEMI)) {
			JITLexer_Yield(self);
			if (pwas_expression)
				*pwas_expression = AST_PARSE_WASEXPR_NO;
#ifdef JIT_EVAL
		} else if (old_tab != JITContext_GetROLocals(self->jl_context) ||
		           (old_tab && old_varc != old_tab->ot_size)) {
			if ((comma_mode & AST_COMMA_OUT_FNEEDSEMI) &&
			    /* Hack: Allow a missing trailing ';'-token when at the end
			     *       of the input code, and inside of the global scope. */
			    (self->jl_tok != 0 || !JITContext_IsGlobalScope(self->jl_context))) {
				syn_expr_expected_semi_after_expr(self);
				Dee_Decref(result);
				goto err;
			}
			if (pwas_expression)
				*pwas_expression = AST_PARSE_WASEXPR_NO;
#endif /* JIT_EVAL */
		} else if (comma_mode & AST_COMMA_OUT_FNEEDSEMI) {
			if (pwas_expression)
				*pwas_expression = AST_PARSE_WASEXPR_YES;
		} else {
			if (pwas_expression)
				*pwas_expression = AST_PARSE_WASEXPR_MAYBE;
		}
	}	break;

	}
/*done:*/
	return result;
err:
	return ERROR;
}


DECL_END


#ifndef __INTELLISENSE__
#define JIT_HYBRID 1
#include "parser-impl-hybrid-parts.c.inl"
/**/
#include "parser-impl-hybrid-parts.c.inl"
#endif
