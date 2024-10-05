/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "parser-impl-hybrid.c.inl"
//#define JIT_HYBRID 1
#endif

#include <deemon/alloc.h>
#include <deemon/thread.h>

#ifdef JIT_HYBRID
#define JIT_ARGS       unsigned int *p_was_expression
#ifdef JIT_EVAL
#define EVAL_PRIMARY   JITLexer_EvalHybrid
#define EVAL_SECONDARY JITLexer_EvalHybridSecondary
#define H_FUNC(x)      JITLexer_Eval##x##Hybrid
#else /* JIT_EVAL */
#define EVAL_PRIMARY   JITLexer_SkipHybrid
#define EVAL_SECONDARY JITLexer_SkipHybridSecondary
#define H_FUNC(x)      JITLexer_Skip##x##Hybrid
#endif /* !JIT_EVAL */
#define SKIP_PRIMARY   JITLexer_SkipHybrid
#define SKIP_SECONDARY JITLexer_SkipHybridSecondary
#define IF_HYBRID(...)  __VA_ARGS__
#define IF_NHYBRID(...) /* nothing */
#else /* JIT_HYBRID */
#define JIT_ARGS     bool is_statement
#ifdef JIT_EVAL
#define EVAL_PRIMARY(self, p_was_expression)   (is_statement ? JITLexer_EvalStatement(self) : JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL))
#define EVAL_SECONDARY(self, p_was_expression) (is_statement ? JITLexer_EvalStatement(self) : JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL))
#define H_FUNC(x)                             JITLexer_Eval##x
#else /* JIT_EVAL */
#define EVAL_PRIMARY(self, p_was_expression)   (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
#define EVAL_SECONDARY(self, p_was_expression) (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
#define H_FUNC(x)                             JITLexer_Skip##x
#endif /* !JIT_EVAL */
#define SKIP_PRIMARY(self, p_was_expression)   (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
#define SKIP_SECONDARY(self, p_was_expression) (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
#define IF_HYBRID(...)  /* nothing */
#define IF_NHYBRID(...) __VA_ARGS__
#endif /* !JIT_HYBRID */

DECL_BEGIN


INTERN RETURN_TYPE DFCALL
H_FUNC(Try)(JITLexer *__restrict self, JIT_ARGS) {
	RETURN_TYPE result;
	IF_HYBRID(unsigned int was_expression;)
#ifdef JIT_EVAL
	ASSERT(JITLexer_ISKWD(self, "try"));
	ASSERT(!(self->jl_context->jc_flags & JITCONTEXT_FSYNERR));
	JITLexer_Yield(self);
	{
		unsigned char *start;
		start  = self->jl_tokstart;
		result = EVAL_PRIMARY(self, &was_expression);
		if (ISERR(result)) {
			/* An exception was thrown. - If it's a syntax error,
			 * don't allow it to be handled, but force propagation. */
			if (self->jl_context->jc_flags & JITCONTEXT_FSYNERR)
				goto err;
			/* Fully parse the primary expression so we end up at the catch/finally.
			 * This must be done since an exception being thrown causes token parsing
			 * to stop in its track, meaning that the current lexer position is
			 * entirely undefined. */
			JITLexer_YieldAt(self, start);
			if (SKIP_PRIMARY(self, &was_expression))
				goto err;
		}
		ASSERT(!(self->jl_context->jc_flags & JITCONTEXT_FSYNERR));
	}
	/* Resolve l-value expressions for the try-block. */
	if (result == JIT_LVALUE)
		result = JITLexer_PackLValue(self);
	while (self->jl_tok == JIT_KEYWORD) {
		bool allow_interrupts = false;
		/* XXX: Full tagging support? */
		if (self->jl_tok == '@') {
			JITLexer_Yield(self);
			if (self->jl_tok == '[') {
				JITLexer_Yield(self);
				if (JITLexer_ISKWD(self, "interrupt")) {
					JITLexer_Yield(self);
					allow_interrupts = true;
				}
			}
			if (self->jl_tok == ']') {
				JITLexer_Yield(self);
			} else {
				syn_anno_expected_rbracket(self);
				goto err;
			}
		}
		if (JITLexer_ISTOK(self, "finally")) {
			DREF DeeObject *finally_value;
			DREF DeeObject *old_return_expr;
			JITLValue old_lvalue;
			unsigned char *start;
			JITLexer_Yield(self);


			/* Temporarily reset the return override to handle things
			 * like double-return/break/continue (in which case the
			 * later return/break/continue always takes precedence) */
			old_return_expr             = self->jl_context->jc_retval;
			old_lvalue                  = self->jl_lvalue;
			self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
			JITLValue_Init(&self->jl_lvalue);

			/* Parse the finally-block */
			start         = self->jl_tokstart;
			finally_value = EVAL_SECONDARY(self, &was_expression);

			/* Finally expression values are never used. */
			if (finally_value == JIT_LVALUE)
				JITLValue_Fini(&self->jl_lvalue);
			self->jl_lvalue = old_lvalue;
			if (ISOK(finally_value)) {
				/* Preserve the old return value */
				Dee_Decref(finally_value);
			} else {
				/* We get here for a number of reasons:
				 *  - Syntax error in finally-block
				 *     -> Immediatly propagate the error
				 *  - finally-block contains a break/continue/return statement
				 *    finally-block throws an exception
				 *     -> Keep the old return override and continue scanning
				 *        for more catch/finally blocks, handling them as
				 *        though the new return override had been set by
				 *        the original try-block.
				 */
				/* Make sure to clear the return value. */
				Dee_XClear(result);

				/* If it was set, restore the old return override. */
				if (self->jl_context->jc_retval &&
				    JITCONTEXT_RETVAL_ISSET(self->jl_context->jc_retval)) {
					if (old_return_expr && JITCONTEXT_RETVAL_ISSET(old_return_expr)) {
						Dee_Decref(self->jl_context->jc_retval);
						self->jl_context->jc_retval = old_return_expr;
					} else {
						/* Keep the new return override! */
					}
				} else {
					self->jl_context->jc_retval = old_return_expr;
				}

				/* Check if this is a syntax error. */
				if (self->jl_context->jc_flags & JITCONTEXT_FSYNERR)
					goto err;
				/* Re-parse the finally-block */
				JITLexer_YieldAt(self, start);
				if (SKIP_SECONDARY(self, &was_expression))
					goto err;
				/* Continue scanning for catch/finally blocks. */
				continue;
			}
			/* Restore the old special return branch, if it was set. */
			ASSERT(self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET);
			self->jl_context->jc_retval = old_return_expr; /* Inherit reference. */
		} else if (JITLexer_ISTOK(self, "catch")) {
			/* Skip catch statements. */
			JITLexer_Yield(self);
			if unlikely(self->jl_tok != '(') {
				syn_try_expected_lparen_after_catch(self);
				goto err_r;
			}
			JITLexer_Yield(self);
			if (!result &&
			    self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET) {
				DREF DeeObject *typemask;
				char const *symbol_name;
				size_t symbol_size;
				uint16_t old_except;
				DeeObject *current;
				DeeThreadObject *ts = DeeThread_Self();
				unsigned char *start;
				old_except = ts->t_exceptsz;
				current    = DeeError_Current();
				ASSERT(current != NULL);
				JITContext_PushScope(self->jl_context);
				if (JITLexer_ParseCatchMask(self, &typemask, &symbol_name, &symbol_size))
					goto err_r_popscope;
				/* Check if we're allowed to handle this exception! */
				if ((!typemask || JIT_IsCatchable(current, typemask)) &&
				    (allow_interrupts || !DeeObject_IsInterrupt(current))) {
					Dee_XDecref(typemask);
					/* Store the current exception into a local symbol. */
					if (symbol_size) {
						JITObjectTable *tab;
						tab = JITContext_GetRWLocals(self->jl_context);
						if unlikely(!tab)
							goto err_r_popscope;
						if (JITObjectTable_Update(tab,
						                          symbol_name,
						                          symbol_size,
						                          Dee_HashUtf8(symbol_name, symbol_size),
						                          current,
						                          true) < 0)
							goto err_r_popscope;
					}
					/* Override the previous return expression with the new one. */
					Dee_XDecref(result);
					start  = self->jl_tokstart;
					result = EVAL_SECONDARY(self, &was_expression);
					if (result == JIT_LVALUE)
						result = JITLexer_PackLValue(self);
					if unlikely(!result) {
						if (self->jl_context->jc_flags & JITCONTEXT_FSYNERR)
							goto err_popscope;
						/* `result' may also be NULL if the catch-body contained
						 * a return/break/continue statement. If that was the case,
						 * then we must still mark the exception as handled, handle
						 * the exception, proceed with a set return value. */
						if (self->jl_context->jc_retval != JITCONTEXT_RETVAL_UNSET) {
							ASSERT(ts->t_exceptsz == old_except);
							DeeError_Handled(ERROR_HANDLED_INTERRUPT);
							JITContext_PopScope(self->jl_context);
							JITLexer_YieldAt(self, start);
							if (SKIP_SECONDARY(self, &was_expression))
								goto err;
							continue;
						}
						goto err_handle_catch_except;
					}
					/* The exception was handled normally. */
					DeeError_Handled(ERROR_HANDLED_INTERRUPT);
					JITContext_PopScope(self->jl_context);
					continue;
err_handle_catch_except:
					JITContext_PopScope(self->jl_context);
					JITLexer_YieldAt(self, start);
					if (SKIP_SECONDARY(self, &was_expression))
						goto err;
					/* Must still handle the original exception. */
					ASSERT(ts->t_exceptsz >= old_except);
					if (ts->t_exceptsz == old_except) {
						ASSERT(ts->t_except);
						ASSERT(ts->t_except->ef_error == current);
						continue; /* The previous exception was re-thrown */
					}
					/* A new exception was thrown on-top of ours. (we must still handle our old one) */
					{
						uint16_t ind = ts->t_exceptsz - old_except;
						struct except_frame *exc, **p_exc;
						exc = *(p_exc = &ts->t_except);
						while (ind--) {
							p_exc = &exc->ef_prev;
							exc  = *p_exc;
						}
						*p_exc = exc->ef_prev;
						--ts->t_exceptsz;
						/* Destroy the frame in question. */
						if (ITER_ISOK(exc->ef_trace))
							Dee_Decref((DeeObject *)exc->ef_trace);
						Dee_Decref(exc->ef_error);
						except_frame_free(exc);
					}
					continue;
				} else {
					/* Don't execute this handler. */
					JITContext_PopScope(self->jl_context);
					Dee_XDecref(typemask);
					if (SKIP_SECONDARY(self, &was_expression))
						goto err_r;
				}
			} else {
				if (JITLexer_SkipPair(self, '(', ')'))
					goto err_r;
				if (SKIP_SECONDARY(self, &was_expression))
					goto err_r;
			}
		} else {
			break;
		}
	}
#else /* JIT_EVAL */
	ASSERT(JITLexer_ISKWD(self, "try"));
	JITLexer_Yield(self);
	result = SKIP_PRIMARY(self, &was_expression);
	if (ISERR(result))
		goto err;
	while (self->jl_tok == JIT_KEYWORD) {
		if (self->jl_tok == '@') {
			JITLexer_Yield(self);
			if (self->jl_tok == '[') {
				JITLexer_Yield(self);
				if (JITLexer_SkipPair(self, '[', ']'))
					goto err;
			}
		}
		if (JITLexer_ISTOK(self, "finally")) {
			JITLexer_Yield(self);
			result = SKIP_SECONDARY(self, &was_expression);
			if (ISERR(result))
				goto err;
		} else if (JITLexer_ISTOK(self, "catch")) {
			JITLexer_Yield(self);
			if unlikely(self->jl_tok != '(') {
				syn_try_expected_lparen_after_catch(self);
				goto err;
			}
			JITLexer_Yield(self);
			if (JITLexer_SkipPair(self, '(', ')'))
				goto err;
			result = SKIP_SECONDARY(self, &was_expression);
			if (ISERR(result))
				goto err;
		} else {
			break;
		}
	}
#endif /* !JIT_EVAL */
	IF_HYBRID(if (p_was_expression) *p_was_expression = was_expression;)
	return result;
#ifdef JIT_EVAL
err_popscope:
	JITContext_PopScope(self->jl_context);
	goto err;
err_r_popscope:
	JITContext_PopScope(self->jl_context);
err_r:
	Dee_XDecref(result);
#endif /* JIT_EVAL */
err:
	return ERROR;
}


INTERN RETURN_TYPE DFCALL
H_FUNC(If)(JITLexer *__restrict self, JIT_ARGS) {
	RETURN_TYPE result;
	ASSERT(JITLexer_ISKWD(self, "if"));
do_if_statement:
	JITLexer_Yield(self);
	/* TODO: Support for `pack' */
	if likely(self->jl_tok == '(') {
		JITLexer_Yield(self);
	} else {
		syn_if_expected_lparen_after_if(self);
		goto err;
	}
#ifdef JIT_EVAL
	JITContext_PushScope(self->jl_context);
	result = JITLexer_EvalComma(self,
	                            AST_COMMA_NORMAL |
	                            AST_COMMA_ALLOWVARDECLS,
	                            NULL,
	                            NULL);
	if (ISERR(result))
		goto err_scope;
	LOAD_LVALUE(result, err);
	if likely(self->jl_tok == ')') {
		JITLexer_Yield(self);
	} else {
		syn_if_expected_rparen_after_if(self);
		goto err_scope_r;
	}
#else /* JIT_EVAL */
	if (JITLexer_SkipPair(self, '(', ')'))
		goto err;
	result = 0;
#endif /* !JIT_EVAL */
	{
#ifdef JIT_EVAL
		int b = DeeObject_Bool(result);
		if unlikely(b < 0)
			goto err_scope_r;
		Dee_Decref(result);
		if (b) {
			result = EVAL_PRIMARY(self, p_was_expression);
			if unlikely(!result)
				goto err_scope;
			if (self->jl_tok == JIT_KEYWORD) {
				if (JITLexer_ISTOK(self, "elif")) {
					self->jl_tokstart += 2;
					goto do_else_branch;
				}
				if (JITLexer_ISTOK(self, "else")) {
					JITLexer_Yield(self);
do_else_branch:
					if (SKIP_SECONDARY(self, p_was_expression)) {
						DECREF_MAYBE_LVALUE(result);
						goto err_scope;
					}
				}
			}
		} else
#endif /* JIT_EVAL */
		{
			if (SKIP_PRIMARY(self, p_was_expression))
				goto err_scope;
			if (self->jl_tok == JIT_KEYWORD) {
				if (JITLexer_ISTOK(self, "elif"))
					goto do_if_statement;
				if (JITLexer_ISTOK(self, "else")) {
					JITLexer_Yield(self);
					result = EVAL_SECONDARY(self, p_was_expression);
					/*if (ISERR(result)) goto err;*/
					goto if_done;
				}
			}
#ifdef JIT_EVAL
			result = Dee_None;
			Dee_Incref(Dee_None);
#endif /* JIT_EVAL */
		}
	}
if_done:
#ifdef JIT_EVAL
	JITContext_PopScope(self->jl_context);
#endif /* JIT_EVAL */
	return result;
#ifdef JIT_EVAL
err_scope_r:
	DECREF(result);
#endif /* JIT_EVAL */
err_scope:
#ifdef JIT_EVAL
	JITContext_PopScope(self->jl_context);
#endif /* JIT_EVAL */
err:
	return ERROR;
}

INTERN RETURN_TYPE DFCALL
H_FUNC(Del)(JITLexer *__restrict self, JIT_ARGS) {
	ASSERT(JITLexer_ISKWD(self, "del"));
#ifdef JIT_HYBRID
	(void)p_was_expression;
#else /* JIT_HYBRID */
	(void)is_statement;
#endif /* !JIT_HYBRID */
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return ERROR;
}

INTERN RETURN_TYPE DFCALL
H_FUNC(For)(JITLexer *__restrict self, JIT_ARGS) {
	RETURN_TYPE result;
	ASSERT(JITLexer_ISKWD(self, "for"));
#ifdef JIT_HYBRID
	if (p_was_expression)
		*p_was_expression = AST_PARSE_WASEXPR_NO;
	/* XXX: Differentiate between expressions and statements */
	result = FUNC(For)(self, true);
#else /* JIT_HYBRID */
	JITLexer_Yield(self);
	if likely(self->jl_tok == '(') {
		JITLexer_Yield(self);
	} else {
		syn_for_expected_lparen_after_for(self);
		goto err;
	}
	if (is_statement) {
#ifdef JIT_EVAL
		DREF DeeObject *init;
		JITContext_PushScope(self->jl_context);
		if (self->jl_tok == ';')
			goto do_normal_for_noinit;
		init = JITLexer_EvalComma(self,
		                          AST_COMMA_NORMAL |
		                          AST_COMMA_ALLOWVARDECLS,
		                          NULL,
		                          NULL);
		if unlikely(!init)
			goto err_scope;
		if (self->jl_tok == ':') {
			/* TODO: Multiple targets (`for (local x, y, z: triples)') */
			JITLValue iterator_storage;
			DREF DeeObject *seq, *iterator, *elem;
			unsigned char *block_start;
			bool is_first_loop = true;
			/* For-each loop. */
			if (init == JIT_LVALUE) {
				iterator_storage        = self->jl_lvalue;
				self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
			} else {
				iterator_storage.lv_kind   = JIT_LVALUE_RVALUE;
				iterator_storage.lv_rvalue = init; /* Inherit reference. */
			}
			JITLexer_Yield(self);
			seq = JITLexer_EvalRValue(self);
			if unlikely(!seq)
				goto err_scope;
			if likely(self->jl_tok == ')') {
				JITLexer_Yield(self);
			} else {
				syn_for_expected_rparen_after_foreach(self);
				goto err_seq;
			}
			iterator = DeeObject_Iter(seq);
			if unlikely(!iterator) {
				goto err_seq;
err_iter:
				Dee_Decref(iterator);
err_seq:
				Dee_Decref(seq);
				JITLValue_Fini(&iterator_storage);
				goto err_scope;
			}
			block_start = self->jl_tokstart;

			while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
				int error;
				error = JITLValue_SetValue(&iterator_storage,
				                           self->jl_context,
				                           elem);
				Dee_Decref(elem);
				if unlikely(error)
					goto err_iter;
				if (!is_first_loop) {
					/* Check for interrupts before jumping back. */
					if (DeeThread_CheckInterrupt())
						goto err_iter;
					self->jl_tokend = block_start;
					JITLexer_Yield(self);
				}
				is_first_loop = false;
				result        = JITLexer_EvalStatement(self);
				if unlikely(!result) {
					/* Handle special loop signal codes. */
					if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_BREAK) {
						self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
						self->jl_tokend             = block_start;
						JITLexer_Yield(self);
						if (JITLexer_SkipStatement(self))
							goto err_iter;
						break;
					}
					if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_CONTINUE) {
						self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
						continue;
					}
					goto err_iter;
				}
				/* Discard the loop block value */
				if (result == JIT_LVALUE) {
					JITLValue_Fini(&self->jl_lvalue);
					self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
				} else {
					Dee_Decref(result);
				}
			}
			JITLValue_Fini(&iterator_storage);
			Dee_Decref(iterator);
			Dee_Decref(seq);
			if unlikely(!elem)
				goto err_scope;
			if (is_first_loop) {
				if (JITLexer_SkipStatement(self))
					goto err_scope;
			}
		} else {
			unsigned char *cond_start;
			unsigned char *next_start;
			unsigned char *block_start;
			/* Regular for-loop */
			if (init == JIT_LVALUE) {
				JITLValue_Fini(&self->jl_lvalue);
				self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
			} else {
				Dee_Decref(init);
			}
			if (self->jl_tok == ';') {
do_normal_for_noinit:
				JITLexer_Yield(self);
			} else {
				syn_for_expected_semi1_after_for(self);
				goto err_scope;
			}
			cond_start = next_start = NULL;
			if (self->jl_tok == ';') {
				JITLexer_Yield(self);
			} else {
				int temp;
				cond_start = self->jl_tokstart;
				result     = JITLexer_EvalRValue(self);
				if unlikely(!result)
					goto err_scope; /* XXX: Doesn't the real compiler allow `break/continue' in the cond-expression? */
				/* Perform the initial condition check. */
				temp = DeeObject_BoolInherited(result);
				if unlikely(temp < 0)
					goto err;
				if (self->jl_tok == ';') {
					JITLexer_Yield(self);
				} else {
					syn_for_expected_semi2_after_for(self);
					goto err_scope;
				}
				if (!temp) {
					/* The loop- or next-expression never get executed. */
					if (self->jl_tok != ')' &&
					    JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
						goto err_scope;
					if likely(self->jl_tok == ')') {
						JITLexer_Yield(self);
					} else {
err_missing_rparen_after_for:
						syn_for_expected_rparen_after_for(self);
						goto err_scope;
					}
					if (JITLexer_SkipStatement(self))
						goto err_scope;
					goto done_for;
				}
				/* The initial condition check was successful. */
			}
			if (self->jl_tok == ')') {
				JITLexer_Yield(self);
			} else {
				next_start = self->jl_tokstart;
				if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
					goto err_scope;
				if likely(self->jl_tok == ')') {
					JITLexer_Yield(self);
				} else {
					goto err_missing_rparen_after_for;
				}
			}
			block_start = self->jl_tokstart;
			/* The actual for-loop */
			for (;;) {
				unsigned char *block_end;
				result = JITLexer_EvalStatement(self);
				if unlikely(!result) {
					/* Handle special loop signal codes. */
					if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_BREAK) {
						self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
						self->jl_tokend             = block_start;
						JITLexer_Yield(self);
						if (JITLexer_SkipStatement(self))
							goto err_scope;
						break;
					}
					if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_CONTINUE) {
						self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
						goto do_continue_normal_forloop;
					}
					goto err_scope;
				}
do_continue_normal_forloop:
				if (result == JIT_LVALUE) {
					JITLValue_Fini(&self->jl_lvalue);
					self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
				} else {
					Dee_Decref(result);
				}
				block_end = self->jl_tokstart;
				/* Execute the next-expression. */
				if (next_start) {
					self->jl_tokend = next_start;
					JITLexer_Yield(self);
					result = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
					if unlikely(!result)
						goto err_scope; /* XXX: Doesn't the real compiler allow `break/continue' in the next-expression? */
					if (result == JIT_LVALUE) {
						JITLValue_Fini(&self->jl_lvalue);
						self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
					} else {
						Dee_Decref(result);
					}
				}
				/* Re-check the condition */
				if (cond_start) {
					int temp;
					self->jl_tokend = cond_start;
					JITLexer_Yield(self);
					result = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
					if unlikely(!result)
						goto err_scope; /* XXX: Doesn't the real compiler allow `break/continue' in the cond-expression? */
					temp = DeeObject_BoolInherited(result);
					if unlikely(temp < 0)
						goto err_scope;
					if (!temp) {
						/* Stop iteration (jump to the end of the for-block). */
						self->jl_tokend = block_end;
						JITLexer_Yield(self);
						break;
					}
				}
				/* Check for interrupts before looping back */
				if (DeeThread_CheckInterrupt())
					goto err_scope;
				/* Jump back to the start of the for-block */
				self->jl_tokend = block_start;
				JITLexer_Yield(self);
			}
		}
done_for:
		JITContext_PopScope(self->jl_context);
		result = Dee_None;
		Dee_Incref(Dee_None);
#else /* JIT_EVAL */
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
		result = JITLexer_SkipStatement(self);
#endif /* !JIT_EVAL */
	} else {
#ifdef JIT_EVAL
		/* TODO */
		DERROR_NOTIMPLEMENTED();
		result = ERROR;
#else /* JIT_EVAL */
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
		result = JITLexer_SkipGeneratorExpression(self, JITLEXER_EVAL_FNORMAL);
#endif /* !JIT_EVAL */
	}
#endif /* !JIT_HYBRID */
	return result;
#ifndef JIT_HYBRID
#ifdef JIT_EVAL
err_scope:
	JITContext_PopScope(self->jl_context);
#endif /* JIT_EVAL */
err:
	return ERROR;
#endif /* !JIT_HYBRID */
}

INTERN RETURN_TYPE DFCALL
H_FUNC(Foreach)(JITLexer *__restrict self, JIT_ARGS) {
	RETURN_TYPE result;
	ASSERT(JITLexer_ISKWD(self, "foreach"));
#ifdef JIT_HYBRID
	if (p_was_expression)
		*p_was_expression = AST_PARSE_WASEXPR_NO;
	/* XXX: Differentiate between expressions and statements */
	result = FUNC(Foreach)(self, true);
#else /* JIT_HYBRID */
	JITLexer_Yield(self);
	if likely(self->jl_tok == '(') {
		JITLexer_Yield(self);
	} else {
		syn_foreach_expected_lparen_after_foreach(self);
		goto err;
	}
	if (is_statement) {
#ifdef JIT_EVAL
		DREF DeeObject *init;
		JITContext_PushScope(self->jl_context);
		init = JITLexer_EvalComma(self,
		                          AST_COMMA_NORMAL |
		                          AST_COMMA_ALLOWVARDECLS,
		                          NULL,
		                          NULL);
		if unlikely(!init)
			goto err_scope;
		if (self->jl_tok == ':') {
			JITLexer_Yield(self);
		} else {
			if (init == JIT_LVALUE) {
				JITLValue_Fini(&self->jl_lvalue);
				JITLValue_Init(&self->jl_lvalue);
			} else {
				Dee_Decref(init);
			}
			syn_foreach_expected_colon_after_foreach(self);
			goto err_scope;
		}
		/* TODO: Multiple targets (`foreach (local x, y, z: triples)') */
		JITLValue iterator_storage;
		DREF DeeObject *iterator, *elem;
		unsigned char *block_start;
		bool is_first_loop = true;
		/* For-each loop. */
		if (init == JIT_LVALUE) {
			iterator_storage        = self->jl_lvalue;
			self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
		} else {
			iterator_storage.lv_kind   = JIT_LVALUE_RVALUE;
			iterator_storage.lv_rvalue = init; /* Inherit reference. */
		}
		iterator = JITLexer_EvalRValue(self);
		if unlikely(!iterator)
			goto err_scope;
		if likely(self->jl_tok == ')') {
			JITLexer_Yield(self);
		} else {
			syn_foreach_expected_rparen_after_foreach(self);
err_iter:
			Dee_Decref(iterator);
			goto err_scope;
		}
		block_start = self->jl_tokstart;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			int error;
			error = JITLValue_SetValue(&iterator_storage,
			                           self->jl_context,
			                           elem);
			Dee_Decref(elem);
			if unlikely(error)
				goto err_iter;
			if (!is_first_loop) {
				/* Check for interrupts before jumping back. */
				if (DeeThread_CheckInterrupt())
					goto err_iter;
				self->jl_tokend = block_start;
				JITLexer_Yield(self);
			}
			is_first_loop = false;
			result        = JITLexer_EvalStatement(self);
			if unlikely(!result) {
				/* Handle special loop signal codes. */
				if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_BREAK) {
					self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
					self->jl_tokend             = block_start;
					JITLexer_Yield(self);
					if (JITLexer_SkipStatement(self))
						goto err_iter;
					break;
				}
				if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_CONTINUE) {
					self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
					continue;
				}
				goto err_iter;
			}
			/* Discard the loop block value */
			if (result == JIT_LVALUE) {
				JITLValue_Fini(&self->jl_lvalue);
				self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
			} else {
				Dee_Decref(result);
			}
		}
		JITLValue_Fini(&iterator_storage);
		Dee_Decref(iterator);
		if unlikely(!elem)
			goto err_scope;
		if (is_first_loop) {
			if (JITLexer_SkipStatement(self))
				goto err_scope;
		}
		JITContext_PopScope(self->jl_context);
		result = Dee_None;
		Dee_Incref(Dee_None);
#else /* JIT_EVAL */
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
		result = JITLexer_SkipStatement(self);
#endif /* !JIT_EVAL */
	} else {
#ifdef JIT_EVAL
		/* TODO */
		DERROR_NOTIMPLEMENTED();
		result = ERROR;
#else /* JIT_EVAL */
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
		result = JITLexer_SkipGeneratorExpression(self, JITLEXER_EVAL_FNORMAL);
#endif /* !JIT_EVAL */
	}
#endif /* !JIT_HYBRID */
	return result;
#ifndef JIT_HYBRID
#ifdef JIT_EVAL
err_scope:
	JITContext_PopScope(self->jl_context);
#endif /* JIT_EVAL */
err:
	return ERROR;
#endif /* !JIT_HYBRID */
}

INTERN RETURN_TYPE DFCALL
H_FUNC(While)(JITLexer *__restrict self, JIT_ARGS) {
	RETURN_TYPE result;
	ASSERT(JITLexer_ISKWD(self, "while"));
#ifdef JIT_HYBRID
	if (p_was_expression)
		*p_was_expression = AST_PARSE_WASEXPR_NO;
	/* XXX: Differentiate between expressions and statements */
	result = FUNC(While)(self, true);
#else /* JIT_HYBRID */
	JITLexer_Yield(self);
	if likely(self->jl_tok == '(') {
		JITLexer_Yield(self);
	} else {
		syn_while_expected_lparen_after_while(self);
		goto err;
	}
	if (is_statement) {
#ifdef JIT_EVAL
		int temp;
		unsigned char *cond_start;
		unsigned char *block_start;
		unsigned char *block_end;
		JITContext_PushScope(self->jl_context);
		/* Parse the loop condition for the first time. */
		cond_start = self->jl_tokstart;
		result     = JITLexer_EvalRValue(self);
		if unlikely(!result)
			goto err_scope;
		if (self->jl_tok == ')') {
			JITLexer_Yield(self);
		} else {
			syn_while_expected_rparen_after_while(self);
			Dee_Decref(result);
			goto err_scope;
		}
		temp = DeeObject_BoolInherited(result);
		if unlikely(temp < 0)
			goto err_scope;
		if (!temp) {
			/* The loop doesn't actually get executed. */
do_skip_loop:
			if (JITLexer_SkipStatement(self))
				goto err_scope;
			goto done_loop;
		}
		block_start = self->jl_tokstart;
		/* Evaluate the loop block. */
do_eval_while_loop:
		result = JITLexer_EvalStatement(self);
		if (!result) {
			/* Check for special signal conditions. */
			if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_BREAK) {
				self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
				self->jl_tokend             = block_start;
				JITLexer_Yield(self);
				goto do_skip_loop;
			}
			if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_CONTINUE) {
				self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
				block_end                   = NULL;
				goto do_check_while_condition;
			}
			goto err_scope;
		}
		if (result == JIT_LVALUE) {
			JITLValue_Fini(&self->jl_lvalue);
			self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
		} else {
			Dee_Decref(result);
		}
		block_end = self->jl_tokstart;
do_check_while_condition:
		/* Check the loop condition once again. */
		self->jl_tokend = cond_start;
		JITLexer_Yield(self);
		result = JITLexer_EvalRValue(self);
		if unlikely(!result)
			goto err_scope; /* XXX: Doesn't the real compiler allow `break/continue' in the cond-expression? */
		temp = DeeObject_BoolInherited(result);
		if unlikely(temp < 0)
			goto err_scope;
		if (temp) {
			/* Loop back around, but check for interrupts first. */
			if (DeeThread_CheckInterrupt())
				goto err_scope;
			self->jl_tokend = block_start;
			JITLexer_Yield(self);
			goto do_eval_while_loop;
		}

		/* Stop looping and jump to the end of the while-block. */
		if likely(block_end) {
			self->jl_tokend = block_end;
			JITLexer_Yield(self);
		} else {
			self->jl_tokend = block_start;
			JITLexer_Yield(self);
			if (JITLexer_SkipStatement(self))
				goto err_scope;
		}
done_loop:
		JITContext_PopScope(self->jl_context);
		result = Dee_None;
		Dee_Incref(Dee_None);
#else /* JIT_EVAL */
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
		result = JITLexer_SkipStatement(self);
#endif /* !JIT_EVAL */
	} else {
#ifdef JIT_EVAL
		/* TODO */
		DERROR_NOTIMPLEMENTED();
		result = ERROR;
#else /* JIT_EVAL */
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
		result = JITLexer_SkipGeneratorExpression(self, JITLEXER_EVAL_FNORMAL);
#endif /* !JIT_EVAL */
	}
#endif /* !JIT_HYBRID */
	return result;
#ifndef JIT_HYBRID
#ifdef JIT_EVAL
err_scope:
	JITContext_PopScope(self->jl_context);
#endif /* JIT_EVAL */
err:
	return ERROR;
#endif /* !JIT_HYBRID */
}

INTERN RETURN_TYPE DFCALL
H_FUNC(Do)(JITLexer *__restrict self, JIT_ARGS) {
	RETURN_TYPE result;
	ASSERT(JITLexer_ISKWD(self, "do"));
#ifdef JIT_HYBRID
	if (p_was_expression)
		*p_was_expression = AST_PARSE_WASEXPR_NO;
	/* XXX: Differentiate between expressions and statements */
	result = FUNC(Do)(self, true);
#else /* JIT_HYBRID */
	JITLexer_Yield(self);
	if (is_statement) {
#ifdef JIT_EVAL
		int temp;
		unsigned char *block_start;
		block_start = self->jl_tokstart;

		/* Parse the block for the first time. */
do_parse_block:
		result = JITLexer_EvalStatement(self);
		if unlikely(!result) {
			/* Check for special signal codes. */
			if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_BREAK) {
				/* `break' */
				self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
				self->jl_tokend             = block_start;
				JITLexer_Yield(self);
				if (JITLexer_SkipStatement(self))
					goto err;
				if (JITLexer_ISKWD(self, "while")) {
					JITLexer_Yield(self);
				} else {
					syn_dowhile_expected_while_after_do(self);
					goto err;
				}
				if (self->jl_tok == '(') {
					JITLexer_Yield(self);
				} else {
					syn_dowhile_expected_lparen_after_while(self);
					goto err;
				}
				if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
					goto err;
				goto done_loop_rparen;
			}
			if (self->jl_context->jc_retval == JITCONTEXT_RETVAL_CONTINUE) {
				self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
				self->jl_tokend             = block_start;
				JITLexer_Yield(self);
				if (JITLexer_SkipStatement(self))
					goto err;
				goto continue_with_loop_cond;
			}
		}
		if (result == JIT_LVALUE) {
			JITLValue_Fini(&self->jl_lvalue);
			self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
		} else {
			Dee_Decref(result);
		}
continue_with_loop_cond:
		if (JITLexer_ISKWD(self, "while")) {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_while_after_do(self);
			goto err;
		}
		if (self->jl_tok == '(') {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_lparen_after_while(self);
			goto err;
		}

		/* Parse the condition code. */
		result = JITLexer_EvalRValue(self);
		if unlikely(!result)
			goto err; /* XXX: Doesn't the real compiler allow `break/continue' in the cond-expression? */
		temp = DeeObject_BoolInherited(result);
		if unlikely(temp < 0)
			goto err;
		if (temp) {
			/* Loop back to the start of the loop-block */
			if (DeeThread_CheckInterrupt())
				goto err;
			self->jl_tokend = block_start;
			JITLexer_Yield(self);
			goto do_parse_block;
		}
done_loop_rparen:
		if (self->jl_tok == ')') {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_rparen_after_while(self);
			goto err;
		}
		if (self->jl_tok == ';') {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_semi_after_while(self);
			goto err;
		}
		result = Dee_None;
		Dee_Incref(Dee_None);
#else /* JIT_EVAL */
		if (JITLexer_SkipStatement(self))
			goto err;
do_skip_while_suffix:
		if (JITLexer_ISKWD(self, "while")) {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_while_after_do(self);
			goto err;
		}
		if (self->jl_tok == '(') {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_lparen_after_while(self);
			goto err;
		}
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
		if (self->jl_tok == ';') {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_semi_after_while(self);
			goto err;
		}
		result = 0;
#endif /* !JIT_EVAL */
	} else {
#ifdef JIT_EVAL
		/* TODO */
		DERROR_NOTIMPLEMENTED();
		result = ERROR;
#else /* JIT_EVAL */
		if (JITLexer_SkipGeneratorExpression(self, JITLEXER_EVAL_FNORMAL))
			goto err;
		goto do_skip_while_suffix;
#endif /* !JIT_EVAL */
	}
#endif /* !JIT_HYBRID */
	return result;
#ifndef JIT_HYBRID
err:
	return ERROR;
#endif /* !JIT_HYBRID */
}

INTERN RETURN_TYPE DFCALL
H_FUNC(With)(JITLexer *__restrict self, JIT_ARGS) {
	RETURN_TYPE result;
#ifdef JIT_EVAL
	DREF DeeObject *with_obj;
#endif /* JIT_EVAL */
	ASSERT(JITLexer_ISKWD(self, "with"));
	JITLexer_Yield(self);
	if likely(self->jl_tok == '(') {
consume_lparen:
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		with_obj = JITLexer_EvalRValue(self);
		if unlikely(!with_obj)
			goto err;
#else /* JIT_EVAL */
		result = JITLexer_SkipRValue(self);
		if unlikely(result)
			goto err;
#endif /* !JIT_EVAL */
		if likely(self->jl_tok == ')') {
			JITLexer_Yield(self);
		} else {
			syn_with_expected_rparen_after_with(self);
			goto err_with_obj;
		}
	} else if (JITLexer_ISKWD(self, "pack")) {
		JITLexer_Yield(self);
		if (self->jl_tok == '(')
			goto consume_lparen;
#ifdef JIT_EVAL
		with_obj = JITLexer_EvalRValue(self);
		if unlikely(!with_obj)
			goto err;
#else /* JIT_EVAL */
		result = JITLexer_SkipRValue(self);
		if unlikely(result)
			goto err;
#endif /* !JIT_EVAL */
	} else {
		syn_with_expected_lparen_after_with(self);
		goto err;
	}
#ifdef JIT_EVAL
	if (DeeObject_Enter(with_obj))
		goto err_with_obj;
#endif /* JIT_EVAL */
	result = EVAL_PRIMARY(self, p_was_expression);

#ifdef JIT_EVAL
	/* Always leave the with-object.
	 * WARNING: This operation may cause a secondary exception to
	 *          be thrown. - If this happens, that exception will
	 *          be dumped by exec() before returning to user-code. */
	if (DeeObject_Leave(with_obj)) {
		DECREF_MAYBE_LVALUE(result);
		result = ERROR;
	}
	Dee_Decref(with_obj);
#endif /* JIT_EVAL */
	return result;
err_with_obj:
#ifdef JIT_EVAL
	Dee_Decref(with_obj);
#endif /* JIT_EVAL */
err:
	return ERROR;
}

INTERN RETURN_TYPE DFCALL
H_FUNC(Assert)(JITLexer *__restrict self, JIT_ARGS) {
	ASSERT(JITLexer_ISKWD(self, "assert"));
#ifdef JIT_HYBRID
	(void)p_was_expression;
#else /* JIT_HYBRID */
	(void)is_statement;
#endif /* !JIT_HYBRID */
	DERROR_NOTIMPLEMENTED();
	return ERROR;
}

#ifdef JIT_HYBRID
INTERN RETURN_TYPE DFCALL
H_FUNC(Import)(JITLexer *__restrict self, JIT_ARGS) {
	if (self->jl_tok == '(' || JITLexer_ISKWD(self, "pack")) {
		/* Special handling for `import(...)' expressions. */
		*p_was_expression = AST_PARSE_WASEXPR_YES;
	} else {
		/* All other uses of `import' and `from' are statements. */
		*p_was_expression = AST_PARSE_WASEXPR_NO;
	}
#ifdef JIT_EVAL
	return JITLexer_EvalImport(self);
#else /* JIT_EVAL */
	return JITLexer_SkipImport(self);
#endif /* !JIT_EVAL */
}
#endif /* JIT_HYBRID */


DECL_END

#undef JIT_ARGS
#undef EVAL_PRIMARY
#undef EVAL_SECONDARY
#undef SKIP_PRIMARY
#undef SKIP_SECONDARY
#undef IF_HYBRID
#undef IF_NHYBRID
#undef H_FUNC
#undef JIT_HYBRID
