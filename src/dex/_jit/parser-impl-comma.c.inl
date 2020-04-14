/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "parser-impl.c.inl"
#endif

#include <deemon/util/objectlist.h>

DECL_BEGIN

#ifdef JIT_EVAL
INTERN WUNUSED DREF DeeObject *DCALL
JITLexer_EvalComma(JITLexer *__restrict self, uint16_t mode,
                   DeeTypeObject *seq_type,
                   uint16_t *pout_mode)
#else /* JIT_EVAL */
INTERN int DCALL
JITLexer_SkipComma(JITLexer *__restrict self, uint16_t mode,
                   uint16_t *pout_mode)
#endif /* !JIT_EVAL */
{
	RETURN_TYPE current;
	JITSmallLexer smlex;
	bool need_semi;
	unsigned int lookup_mode;
#ifdef JIT_EVAL
	int error;
#endif /* JIT_EVAL */
	/* In: "foo, bar = 10, x, y = getvalue()..., 7"
	 *               |      |                  | 
	 *               +----------------------------  expr_batch = { }
	 *                      |                  |    expr_comma = { foo, bar }
	 *                      |                  | 
	 *                      +---------------------  expr_batch = { foo, (bar = 10) }
	 *                                         |    expr_comma = { x }
	 *                                         | 
	 *                                         +--  expr_batch = { foo, (bar = 10), (x,y = getvalue()...) }
	 *                                              expr_comma = { }
	 */
#ifdef JIT_EVAL
	JITSymbol var_symbol;
	size_t expand_count;
	struct objectlist expr_batch = OBJECTLIST_INIT;    /* Expressions that have already been written to. */
	JITLValueList expr_comma     = JITLVALUELIST_INIT; /* Expressions that are pending getting assigned. */
#endif /* JIT_EVAL */
#if AST_COMMA_ALLOWVARDECLS == LOOKUP_SYM_ALLOWDECL
	lookup_mode = mode & AST_COMMA_ALLOWVARDECLS;
#else /* AST_COMMA_ALLOWVARDECLS == LOOKUP_SYM_ALLOWDECL */
	lookup_mode = ((mode & AST_COMMA_ALLOWVARDECLS)
	               ? LOOKUP_SYM_ALLOWDECL
	               : LOOKUP_SYM_NORMAL);
#endif /* AST_COMMA_ALLOWVARDECLS != LOOKUP_SYM_ALLOWDECL */
	/* Allow explicit visibility modifiers when variable can be declared. */
	if (mode & AST_COMMA_ALLOWVARDECLS) {
		if unlikely(JITLexer_ParseLookupMode(self, &lookup_mode))
			goto err;
	}

next_expr:
	need_semi = !!(mode & AST_COMMA_PARSESEMI);
	/* Parse an expression (special handling for functions/classes) */
#if 0 /* TODO: Class declarations */
	if (self->jl_tok == KWD_final || self->jl_tok == KWD_class) {
		/* Declare a new class */
		uint16_t class_flags          = current_tags.at_class_flags & 0xf; /* From tags. */
		struct TPPKeyword *class_name = NULL;
		unsigned int symbol_mode      = lookup_mode;
		if (self->jl_tok == KWD_final) {
			class_flags |= TP_FFINAL;
			if unlikely(yield() < 0)
				goto err;
			if (unlikely(self->jl_tok != KWD_class) &&
			    WARN(W_EXPECTED_CLASS_AFTER_FINAL))
				goto err;
		}
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (self->jl_tok == KWD_class && !(class_flags & TP_FFINAL)) {
			class_flags |= TP_FFINAL;
			if unlikely(yield() < 0)
				goto err;
		}
		if (TPP_ISKEYWORD(self->jl_tok)) {
			class_name = token.t_kwd;
			if unlikely(yield() < 0)
				goto err;
			if ((symbol_mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT) {
				/* Use the default mode appropriate for the current scope. */
				if (current_scope == &current_rootscope->rs_scope.bs_scope)
					symbol_mode |= LOOKUP_SYM_VGLOBAL;
				else {
					symbol_mode |= LOOKUP_SYM_VLOCAL;
				}
			}
		}
		current = ast_setddi(ast_parse_class(class_flags, class_name,
		                                     class_name != NULL,
		                                     symbol_mode),
		                     &loc);
		if unlikely(!current)
			goto err;
		need_semi = false; /* Classes always have braces and don't need semicolons. */
	} else
#endif
#if 1
	if (JITLexer_ISKWD(self, "function")) {
		/* Declare a new function */
#ifdef JIT_EVAL
		unsigned char *name_start;
		unsigned char *name_end;
		unsigned char *param_start;
		unsigned char *param_end;
		unsigned int recursion;
		name_start = name_end = NULL;
		var_symbol.js_kind    = JIT_SYMBOL_NONE;
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
		if (self->jl_tok == JIT_KEYWORD) {
			unsigned int symbol_mode = lookup_mode;
			if ((symbol_mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT) {
				/* Use the default mode appropriate for the current scope. */
				if (JITContext_IsGlobalScope(self->jl_context))
					symbol_mode |= LOOKUP_SYM_VGLOBAL;
				else {
					symbol_mode |= LOOKUP_SYM_VLOCAL;
				}
			}
			/* Create the symbol that will be used by the function. */
#ifdef JIT_EVAL
			name_start = self->jl_tokstart;
			name_end   = self->jl_tokend;
			if (JITContext_Lookup(self->jl_context,
			                      &var_symbol,
			                      (char const *)name_start,
			                      (size_t)(name_end - name_start),
			                      symbol_mode) < 0)
				goto err;
#endif /* JIT_EVAL */
			JITLexer_Yield(self);
		}
		if (self->jl_tok == '(') {
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			param_end = param_start = self->jl_tokstart;
			recursion               = 1;
			while (self->jl_tok) {
				if (self->jl_tok == '(') {
					++recursion;
				} else if (self->jl_tok == ')') {
					--recursion;
					if (!recursion) {
						JITLexer_Yield(self);
						break;
					}
				}
				param_end = self->jl_tokend;
				JITLexer_Yield(self);
			}
#else /* JIT_EVAL */
			if (JITLexer_SkipPair(self, '(', ')'))
				goto err;
#endif /* !JIT_EVAL */
		} else {
			syn_function_expected_lparen_after_function(self);
err_var_symbol:
#ifdef JIT_EVAL
			JITSymbol_Fini(&var_symbol);
#endif /* JIT_EVAL */
			goto err;
		}
		if (self->jl_tok == TOK_ARROW) {
#ifdef JIT_EVAL
			unsigned char *source_start;
			unsigned char *source_end;
			JITLexer_Yield(self);
			source_start = self->jl_tokstart;
			if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FSECONDARY))
				goto err;
			source_end = self->jl_tokstart;
			/* Trim trailing whitespace. */
			while (source_end > source_start) {
				uint32_t ch;
				char const *next = (char const *)source_end;
				ch               = utf8_readchar_rev(&next, (char const *)source_start);
				if (!DeeUni_IsSpace(ch))
					break;
				source_end = (unsigned char *)next;
			}
			current = JITFunction_New((char const *)name_start,
			                          (char const *)name_end,
			                          (char const *)param_start,
			                          (char const *)param_end,
			                          (char const *)source_start,
			                          (char const *)source_end,
			                          self->jl_context->jc_locals.otp_tab,
			                          self->jl_text,
			                          self->jl_context->jc_impbase,
			                          self->jl_context->jc_globals,
			                          JIT_FUNCTION_FRETEXPR);
			if (!current && DeeError_CurrentIs(&DeeError_SyntaxError))
				self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
#else /* JIT_EVAL */
			JITLexer_Yield(self);
			current = JITLexer_SkipExpression(self, JITLEXER_EVAL_FSECONDARY);
#endif /* !JIT_EVAL */
			need_semi = true;
		} else if (self->jl_tok == '{') {
#ifdef JIT_EVAL
			/* Lambda function. */
			unsigned char *source_start;
			unsigned char *source_end;
			unsigned int recursion = 1;
			JITLexer_Yield(self);
			source_end = source_start = self->jl_tokstart;
			/* Scan the body of the function. */
			while (self->jl_tok) {
				if (self->jl_tok == '{') {
					++recursion;
				} else if (self->jl_tok == '}') {
					--recursion;
					if (!recursion) {
						JITLexer_Yield(self);
						break;
					}
				}
				source_end = self->jl_tokend;
				JITLexer_Yield(self);
			}
			current = JITFunction_New((char const *)name_start,
			                          (char const *)name_end,
			                          (char const *)param_start,
			                          (char const *)param_end,
			                          (char const *)source_start,
			                          (char const *)source_end,
			                          self->jl_context->jc_locals.otp_tab,
			                          self->jl_text,
			                          self->jl_context->jc_impbase,
			                          self->jl_context->jc_globals,
			                          JIT_FUNCTION_FNORMAL);
			if (!current && DeeError_CurrentIs(&DeeError_SyntaxError))
				self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
#else /* JIT_EVAL */
			JITLexer_Yield(self);
			current = JITLexer_SkipPair(self, '{', '}');
#endif /* !JIT_EVAL */
			need_semi = false;
		} else {
			syn_function_expected_arrow_or_lbrace_after_function(self);
			goto err_var_symbol;
		}
		if (ISERR(current))
			goto err_var_symbol;
#ifdef JIT_EVAL
		if (var_symbol.js_kind != JIT_SYMBOL_NONE) {
			/* Now store the generated value into the target symbol. */
			error = JITLValue_SetValue((JITLValue *)&var_symbol,
			                           self->jl_context,
			                           current);
			if unlikely(error)
				goto err_currrent_var_symbol;
		}
		JITSymbol_Fini(&var_symbol);
#endif /* JIT_EVAL */
	} else
#endif
	{
		if (mode & AST_COMMA_ALLOWKWDLIST) {
			if (self->jl_tok == JIT_KEYWORD) {
				/* If the next token is a `:', then we're currently at a keyword list label,
				 * in which case we're supposed to stop and let the caller deal with this. */
				memcpy(&smlex, self, sizeof(JITSmallLexer));
				JITLexer_Yield((JITLexer *)&smlex);
				if (smlex.jl_tok == ':')
					goto done_expression_nocurrent;
			}
			if (self->jl_tok == TOK_POW) /* foo(**bar) --> Invoke using `bar' for keyword arguments. */
				goto done_expression_nocurrent;
		}
		current = CALL_PRIMARYF(Expression, lookup_mode);
		/* Check for errors. */
		if (ISERR(current))
			goto err;
		if ((lookup_mode & LOOKUP_SYM_ALLOWDECL) && self->jl_tok == JIT_KEYWORD &&
		    (!(mode & AST_COMMA_NOSUFFIXKWD) /* TODO: || !is_reserved_symbol_name(token.t_kwd)*/)) {
			/* C-style variable declarations. */
			RETURN_TYPE args;
#ifdef JIT_EVAL
			RETURN_TYPE merge;
			{
				unsigned int used_lookup_mode;
				used_lookup_mode = lookup_mode;
				LOAD_LVALUE(current, err);
				if (!(lookup_mode & LOOKUP_SYM_VMASK) &&
				    JITContext_IsGlobalScope(self->jl_context))
					used_lookup_mode |= LOOKUP_SYM_VGLOBAL;
				if (JITContext_Lookup(self->jl_context,
				                      &var_symbol,
				                      (char const *)self->jl_tokstart,
				                      (size_t)(self->jl_tokend - self->jl_tokstart),
				                      used_lookup_mode) < 0)
					goto err_current;
			}
#endif /* JIT_EVAL */
			JITLexer_Yield(self);

			/* Allow syntax like this:
			 * >> List my_list;
			 * >> int my_int = 42;
			 * >> MyClass my_instance(10,20,30);
			 * >> Thread my_thread = []{
			 * >>     print "Thread execution...";
			 * >> };
			 * >> my_thread.start();
			 * Same as:
			 * >> local my_list = List();
			 * >> local my_int = int(42);
			 * >> local my_instance = MyClass(10,20,30);
			 * >> local my_thread = Thread([]{
			 * >>     print "Thread execution...";
			 * >> });
			 * >> my_thread.start();
			 * Now that C compatibility is gone, there's no ambiguity to this!
			 */
			if (self->jl_tok == '=' || self->jl_tok == '{') {
				/* Single-operand argument list. */
				if (self->jl_tok == '=')
					JITLexer_Yield(self);
				/* Parse a preferred-type brace expression. */
				args = CALL_PRIMARYF(Expression, LOOKUP_SYM_NORMAL);
				if (ISERR(args)) {
err_currrent_var_symbol:
#ifdef JIT_EVAL
					JITSymbol_Fini(&var_symbol);
#endif /* JIT_EVAL */
					goto err_current;
				}
#ifdef JIT_EVAL
				LOAD_LVALUE(args, err_currrent_var_symbol);
				merge = DeeTuple_PackSymbolic(1, args);
				if unlikely(!merge) {
					Dee_Decref(args);
					goto err_currrent_var_symbol;
				}
				args = merge;
#endif /* JIT_EVAL */
			} else if (self->jl_tok == '(' || JITLexer_ISKWD(self, "pack")) {
				/* Comma-separated argument list. */
				bool has_paren = self->jl_tok == '(';
				JITLexer_Yield(self);
				/* TODO: Add support for applying annotations here! */
				if (self->jl_tok == ')' ||
				    (!has_paren && !JIT_MaybeExpressionBegin(self->jl_tok))) {
					/* Empty argument list (Same as none at all). */
#ifdef JIT_EVAL
					args = Dee_EmptyTuple;
					Dee_Incref(Dee_EmptyTuple);
#endif /* JIT_EVAL */
				} else {
#ifdef JIT_EVAL
					args = JITLexer_EvalComma(self, AST_COMMA_FORCEMULTIPLE, &DeeTuple_Type, NULL);
#else /* JIT_EVAL */
					args = JITLexer_SkipComma(self, AST_COMMA_FORCEMULTIPLE, NULL);
#endif /* !JIT_EVAL */
					if (ISERR(args))
						goto err_currrent_var_symbol;
					LOAD_LVALUE(args, err_currrent_var_symbol);
				}
				if (has_paren) {
					if likely(self->jl_tok == ')') {
						JITLexer_Yield(self);
					} else {
						syn_call_expected_rparen_after_call(self);
						DECREF(args);
						goto err_currrent_var_symbol;
					}
				}
			} else {
				/* No argument list. */
#ifdef JIT_EVAL
				args = Dee_EmptyTuple;
				Dee_Incref(Dee_EmptyTuple);
#endif /* JIT_EVAL */
			}
#ifdef JIT_EVAL
			merge = DeeObject_CallTuple(current, args);
			Dee_Decref(args);
			if unlikely(!merge)
				goto err_currrent_var_symbol;
			Dee_Decref(current);
			current = merge;
			/* Now store the generated value into the target symbol. */
			error = JITLValue_SetValue((JITLValue *)&var_symbol,
			                           self->jl_context,
			                           current);
			if unlikely(error)
				goto err_currrent_var_symbol;
			JITSymbol_Fini(&var_symbol);
#endif /* JIT_EVAL */
		}
	}

	if (self->jl_tok == TOK_DOTS) {
		/* Expand expression (append everything from `current' to the resulting expression) */
		JITLexer_Yield(self);
		if (pout_mode)
			*pout_mode |= AST_COMMA_OUT_FMULTIPLE;
#ifdef JIT_EVAL
		LOAD_LVALUE(current, err);
		if (expr_comma.ll_size) {
			error = JITLValueList_CopyObjects(&expr_comma,
			                                  &expr_batch,
			                                  self->jl_context);
			if unlikely(error)
				goto err_current;
		}
		expand_count = objectlist_extendseq(&expr_batch, current);
		if unlikely(expand_count == (size_t)-1)
			goto err_current;
		Dee_Decref(current);
#endif /* JIT_EVAL */
		goto continue_expression_after_dots;
	}

	if (self->jl_tok == ',' && !(mode & AST_COMMA_PARSESINGLE)) {
		if (mode & AST_COMMA_STRICTCOMMA) {
			/* Peek the next token to check if it might be an expression. */
			memcpy(&smlex, self, sizeof(JITSmallLexer));
			JITLexer_Yield((JITLexer *)&smlex);
			if (!JIT_MaybeExpressionBegin(smlex.jl_tok))
				goto done_expression;
		}
		if (pout_mode)
			*pout_mode |= AST_COMMA_OUT_FMULTIPLE;

			/* Append to the current comma-sequence. */
#ifdef JIT_EVAL
		if (current == JIT_LVALUE) {
			error = JITLValueList_Append(&expr_comma,
			                             &self->jl_lvalue);
			if unlikely(error)
				goto err_current;
			self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
		} else {
			error = JITLValueList_AppendRValue(&expr_comma, current);
			if unlikely(error)
				goto err_current;
			Dee_Decref(current);
		}
#endif /* JIT_EVAL */
		/* Yield the ',' token. */
continue_at_comma:
		JITLexer_Yield(self);
#if 1
		if (!JIT_MaybeExpressionBegin(self->jl_tok))
			goto done_expression_nocurrent;
#else
		if (!JIT_MaybeExpressionBegin(self->jl_tok)) {
			/* Special case: `x = (10,)'
			 * Same as `x = pack(10)', in that a single-element tuple is created. */
#ifdef JIT_EVAL
			/* Flush any remaining entries from the comma-list. */
			ASSERT(expr_comma.ll_size != 0);
			if (!seq_type) {
				/* Evaluate to the last referenced expression. */
				current = JITLValue_GetValue(&expr_comma.ll_list[expr_comma.ll_size - 1],
				                             self->jl_context);
				if unlikely(!current)
					goto err;
				objectlist_fini(&expr_batch);
				JITLValueList_Fini(&expr_comma);
			} else {
				error = JITLValueList_CopyObjects(&expr_comma,
				                                  &expr_batch,
				                                  self->jl_context);
				if unlikely(error)
					goto err;
				JITLValueList_Fini(&expr_comma);
				/* Pack the branch together to form a multi-branch AST. */
				current = seq_type == &DeeTuple_Type
				          ? objectlist_packtuple(&expr_batch)
				          : objectlist_packlist(&expr_batch);
				if unlikely(!current)
					goto err_nocomma;
			}
			/* WARNING: At this point, both `expr_batch' and `expr_comma' are
			 *          in an undefined state, but don't hold any real data. */
#endif /* JIT_EVAL */
			goto done_expression_nomerge;
		}
#endif
		goto next_expr;
	}
	if (self->jl_tok == '=') {
#ifdef JIT_EVAL
		JITLValue current_lvalue;
#endif /* JIT_EVAL */
		RETURN_TYPE store_source;
#ifdef JIT_EVAL
		memcpy(&current_lvalue, &self->jl_lvalue, sizeof(JITLValue));
		self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
#endif /* JIT_EVAL */

		/* This is where the magic happens and where we
		 * assign to expression in the active comma-list. */
		JITLexer_Yield(self);
		/* TODO: Add support for applying annotations here! */
		{
			uint16_t store_source_mode;
			/* Must pass a pointer for pout_mode, since this call might
			 * otherwise try to consume a trailing ';'-character. */
#ifdef JIT_EVAL
			store_source = JITLexer_EvalComma(self,
			                                  AST_COMMA_PARSESINGLE |
			                                  (mode & AST_COMMA_STRICTCOMMA),
			                                  NULL,
			                                  &store_source_mode);
#else /* JIT_EVAL */
			store_source = JITLexer_SkipComma(self,
			                                  AST_COMMA_PARSESINGLE |
			                                  (mode & AST_COMMA_STRICTCOMMA),
			                                  &store_source_mode);
#endif /* !JIT_EVAL */
		}
		if (ISERR(store_source)) {
#ifdef JIT_EVAL
err_current_lvalue:
			JITLValue_Fini(&current_lvalue);
#endif /* JIT_EVAL */
			goto err_current;
		}
		LOAD_LVALUE(store_source, err_current_lvalue);
		need_semi = !!(mode & AST_COMMA_PARSESEMI);
		/* Now everything depends on whether or not what
		 * we've just parsed is an expand-expression:
		 * >> a,b,c = get_value()...; // >> (((a,b,c) = get_value())...);
		 * >> a,b,c = get_value();    // >> (a,b,(c = get_value())); */
		if (self->jl_tok == TOK_DOTS) {
			/* Append the last expression (in the example above, that is `c') */
			if (pout_mode)
				*pout_mode |= AST_COMMA_OUT_FMULTIPLE;
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			if (current == JIT_LVALUE) {
				ASSERT(current_lvalue.lv_kind != JIT_LVALUE_NONE);
				error = JITLValueList_Append(&expr_comma,
				                             &current_lvalue);
				if unlikely(error) {
err_store_source_current_lvalue:
					Dee_Decref(store_source);
					goto err_current_lvalue;
				}
			} else {
				ASSERT(current_lvalue.lv_kind == JIT_LVALUE_NONE);
				error = JITLValueList_AppendRValue(&expr_comma, current);
				if unlikely(error) {
/*err_store_source_current:*/
					Dee_Decref(store_source);
					goto err_current;
				}
				Dee_Decref(current);
			}
			/* At this point, we have to unpack `store_source' into `expr_comma.ll_size'
			 * different objects, then proceed to assign each of those to the objects
			 * to their respective l-values in `expr_comma' */
			{
				DREF DeeObject **buf;
				size_t i;
				expand_count = expr_comma.ll_size;
				buf          = objectlist_alloc(&expr_batch, expand_count);
				if unlikely(!buf) {
err_store_source:
					Dee_Decref(store_source);
					goto err;
				}
				error = DeeObject_Unpack(store_source,
				                         expand_count,
				                         buf);
				if unlikely(error) {
					expr_batch.ol_size -= expand_count;
					goto err_store_source;
				}
				Dee_Decref(store_source);
				/* Assign values to L-value targets. */
				for (i = 0; i < expand_count; ++i) {
					error = JITLValue_SetValue(&expr_comma.ll_list[i],
					                           self->jl_context,
					                           buf[i]);
					if unlikely(error)
						goto err;
				}

continue_expression_after_dots:
				/* At this point, all L-values have been assigned, and all
				 * R-values have been appended to the expression batch. */
				JITLValueList_Fini(&expr_comma);

				/* Check for further comma or store expressions. */
				if (self->jl_tok == ',' && !(mode & AST_COMMA_PARSESINGLE)) {
					JITLValueList_Init(&expr_comma);
					if (!(mode & AST_COMMA_STRICTCOMMA))
						goto set_multiple_and_continue_at_comma;
					memcpy(&smlex, self, sizeof(JITSmallLexer));
					JITLexer_Yield((JITLexer *)&smlex);
					if (JIT_MaybeExpressionBegin(smlex.jl_tok))
						goto set_multiple_and_continue_at_comma_continue;
				}
				/* Pack the expression to-be returned. */
				if (!seq_type) {
					ASSERT(expr_batch.ol_size >= expand_count);
					current = DeeTuple_NewVectorSymbolic(expand_count,
					                                     expr_batch.ol_list +
					                                     expr_batch.ol_size -
					                                     expand_count);
					if unlikely(!current)
						goto err_nocomma;
					expr_batch.ol_size -= expand_count;
					objectlist_fini(&expr_batch);
				} else if (seq_type == &DeeTuple_Type) {
					current = objectlist_packtuple(&expr_batch);
					if unlikely(!current)
						goto err_nocomma;
				} else {
					current = objectlist_packlist(&expr_batch);
					if unlikely(!current)
						goto err_nocomma;
				}
				goto done_expression_nomerge;
			}
#endif /* JIT_EVAL */
		} else {
			/* Second case: assign `store_source' to `current' after
			 *              flushing everything from the comma-list. */
#ifdef JIT_EVAL
			if (current != JIT_LVALUE) {
				JITLexer_ErrorTrace(self, self->jl_tokstart);
				self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
				DeeError_Throwf(&DeeError_SyntaxError,
				                "Cannot store to R-value");
				goto err_store_source_current_lvalue;
			}
			error = JITLValueList_CopyObjects(&expr_comma,
			                                  &expr_batch,
			                                  self->jl_context);
			if unlikely(error)
				goto err_store_source_current_lvalue;
			JITLValueList_Fini(&expr_comma);
			JITLValueList_Init(&expr_comma);
			/* Actually perform the store to the L-value. */
			error = JITLValue_SetValue(&current_lvalue,
			                           self->jl_context,
			                           store_source);
			if unlikely(error)
				goto err_store_source_current_lvalue;
			JITLValue_Fini(&current_lvalue);
			current = store_source; /* Inherit reference. */
#endif /* JIT_EVAL */
		}
#ifndef JIT_EVAL
continue_expression_after_dots:
#else /* !JIT_EVAL */
		ASSERT(current != JIT_LVALUE);
#endif /* JIT_EVAL */
		/* Check for further comma or store expressions. */
		if (self->jl_tok == ',' && !(mode & AST_COMMA_PARSESINGLE)) {
			if (!(mode & AST_COMMA_STRICTCOMMA)) {
				/* Append the generated expression to the batch. */
#ifdef JIT_EVAL
				error = objectlist_append(&expr_batch, current);
				if unlikely(error)
					goto err_current;
				Dee_Decref(current);
set_multiple_and_continue_at_comma:
#endif /* JIT_EVAL */
				if (pout_mode)
					*pout_mode |= AST_COMMA_OUT_FMULTIPLE;
				goto continue_at_comma;
			} else {
				memcpy(&smlex, self, sizeof(JITSmallLexer));
				JITLexer_Yield((JITLexer *)&smlex);
				if (JIT_MaybeExpressionBegin(smlex.jl_tok)) {
#ifdef JIT_EVAL
					error = objectlist_append(&expr_batch, current);
					if unlikely(error)
						goto err_current;
					Dee_Decref(current);
set_multiple_and_continue_at_comma_continue:
#endif /* JIT_EVAL */
					if (pout_mode)
						*pout_mode |= AST_COMMA_OUT_FMULTIPLE;
					goto continue_at_comma;
				}
			}
		}
	}
done_expression:

#ifdef JIT_EVAL
	/* Merge the current expression with the batch and comma lists. */
	if (expr_batch.ol_size || expr_comma.ll_size) {
		/* Flush any remaining entries from the comma-list. */
		if (!seq_type) {
			JITLValueList_Fini(&expr_comma);
			objectlist_fini(&expr_batch);
		} else {
			if (expr_comma.ll_size) {
				error = JITLValueList_CopyObjects(&expr_comma,
				                                  &expr_batch,
				                                  self->jl_context);
				if unlikely(error)
					goto err_current;
			}
			/* Append the remaining expression to the batch. */
			LOAD_LVALUE(current, err);
			error = objectlist_append(&expr_batch, current);
			if unlikely(error)
				goto err_current;
			Dee_Decref(current);
			/* Pack the branch together to form a multi-branch AST. */
			if (seq_type == &DeeTuple_Type) {
				current = objectlist_packtuple(&expr_batch);
			} else {
				current = objectlist_packlist(&expr_batch);
			}
			if unlikely(!current)
				goto err;
			/* Free an remaining buffers. */
			/*Dee_Free(expr_batch.ast_v);*/ /* This one was inherited. */
			JITLValueList_Fini(&expr_comma);
		}
		/* WARNING: At this point, both `expr_batch' and `expr_comma' are
		 *          in an undefined state, but don't hold any real data. */
	} else {
		ASSERT(!expr_batch.ol_list);
		ASSERT(!expr_comma.ll_list);
		if (mode & AST_COMMA_FORCEMULTIPLE) {
			/* If the caller wants to force us to package
			 * everything in a multi-branch, grant that wish. */
			DREF DeeObject *merge;
			LOAD_LVALUE(current, err);
			if (seq_type == &DeeTuple_Type) {
				merge = DeeTuple_PackSymbolic(1, current);
				if unlikely(!merge)
					goto err_current;
				current = merge;
			} else if (seq_type == &DeeList_Type) {
				merge = DeeList_NewVectorInherited(1, (DREF DeeObject *const *)&current);
				if unlikely(!merge)
					goto err_current;
				current = merge;
			}
		}
	}
#endif /* JIT_EVAL */

done_expression_nomerge:
	if (pout_mode) {
		if (need_semi)
			*pout_mode |= AST_COMMA_OUT_FNEEDSEMI;
	} else if (need_semi) {
		/* Consume a `;' token as part of the expression. */
		if likely(self->jl_tok == ';') {
			JITLexer_Yield(self);
		} else {
			syn_expr_expected_semi_after_expr(self);
/*err_clear_current_only:*/
			DECREF(current);
			current = ERROR;
		}
	}
	return current;
done_expression_nocurrent:
#ifndef JIT_EVAL
	current = 0;
#else /* !JIT_EVAL */
	/* Merge the current expression with the batch and comma lists. */
	if (expr_batch.ol_size || expr_comma.ll_size) {
		if (!seq_type) {
			if (expr_comma.ll_size) {
				current = JITLValue_GetValue(&expr_comma.ll_list[expr_comma.ll_size - 1],
				                             self->jl_context);
				if unlikely(!current)
					goto err;
			} else {
				ASSERT(expr_batch.ol_size);
				--expr_batch.ol_size;
				current = expr_batch.ol_list[expr_batch.ol_size]; /* Inherit */
			}
			JITLValueList_Fini(&expr_comma);
			objectlist_fini(&expr_batch);
		} else {
			/* Flush any remaining entries from the comma-list. */
			if (expr_comma.ll_size) {
				error = JITLValueList_CopyObjects(&expr_comma,
				                                  &expr_batch,
				                                  self->jl_context);
				if unlikely(error)
					goto err;
				JITLValueList_Fini(&expr_comma);
			}
			/* Pack the branch together to form a multi-branch AST. */
			if (seq_type == &DeeTuple_Type) {
				current = objectlist_packtuple(&expr_batch);
			} else {
				current = objectlist_packlist(&expr_batch);
			}
			if unlikely(!current)
				goto err_nocomma;
			/* Free an remaining buffers. */
			/*Dee_Free(expr_batch.ast_v);*/ /* This one was inherited. */
		}
		/* WARNING: At this point, both `expr_batch' and `expr_comma' are
		 *          in an undefined state, but don't hold any real data. */
	} else {
		ASSERT(!expr_batch.ol_list);
		ASSERT(!expr_comma.ll_list);
		/* If the caller wants to force us to package
		 * everything in a multi-branch, grant that wish. */
		if (!seq_type) {
			current = Dee_EmptySeq;
			Dee_Incref(Dee_EmptySeq);
		} else if (seq_type == &DeeTuple_Type) {
			current = Dee_EmptyTuple;
			Dee_Incref(Dee_EmptyTuple);
		} else {
			current = DeeList_New();
			if unlikely(!current)
				goto err_noexpr;
		}
	}
#endif /* JIT_EVAL */
	goto done_expression_nomerge;
err_current:
	DECREF_MAYBE_LVALUE(current);
err:
#ifdef JIT_EVAL
	JITLValueList_Fini(&expr_comma);
err_nocomma:
	objectlist_fini(&expr_batch);
err_noexpr:
#endif /* JIT_EVAL */
	return ERROR;
}


DECL_END
