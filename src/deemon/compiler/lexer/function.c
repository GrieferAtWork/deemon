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
#ifndef GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C
#define GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/none.h>
#include <deemon/object.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t, uint32_t */

DECL_BEGIN

PRIVATE int DCALL skip_argument_name(void) {
	if unlikely(!TPP_ISKEYWORD(tok)) {
		if (WARN(W_EXPECTED_KEYWORD_FOR_ARGUMENT_NAME))
			goto err;
	} else {
		if (tok != KWD_none) {
			if (has_local_symbol(token.t_kwd)) {
				if (WARN(W_ARGUMENT_NAME_ALREADY_IN_USE))
					goto err;
			} else if (is_reserved_symbol_name(token.t_kwd)) {
				if (WARN(W_RESERVED_ARGUMENT_NAME, token.t_kwd))
					goto err;
			}
		}
		if unlikely(yield() < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE struct symbol *DCALL parse_argument_name(void) {
	struct symbol *result;
	struct TPPKeyword *argument_name;
	if unlikely(!TPP_ISKEYWORD(tok)) {
		if (WARN(W_EXPECTED_KEYWORD_FOR_ARGUMENT_NAME))
			goto err;
		result = new_unnamed_symbol();
	} else {
		if (tok == KWD_none) {
			/* Special case: Allow `none' to be used for placeholder/pending arguments. */
create_anon_argument:
			/* Create a new symbol for the argument. */
			result = new_unnamed_symbol();
			if unlikely(!result)
				goto err;
			loc_here(&result->s_decl);
			if (result->s_decl.l_file)
				TPPFile_Incref(result->s_decl.l_file);
		} else {
			argument_name = token.t_kwd;
			if (has_local_symbol(argument_name)) {
				if (WARN(W_ARGUMENT_NAME_ALREADY_IN_USE))
					goto err;
				goto create_anon_argument;
			}

			/* Check if the argument name is a reserved identifier. */
			if (is_reserved_symbol_name(argument_name)) {
				if (WARN(W_RESERVED_ARGUMENT_NAME, argument_name))
					goto err;
			}

			/* Create a new symbol for the argument. */
			result = new_local_symbol(argument_name, NULL);
		}
		if unlikely(yield() < 0)
			goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
resize_argument_list(uint16_t *__restrict p_arga) {
	struct symbol **new_symv;
	ASSERT(current_basescope->bs_argc <= *p_arga);
	if (current_basescope->bs_argc >= *p_arga) {
		uint16_t new_arga = *p_arga * 2;
		if (!new_arga)
			new_arga = 2;
do_realloc_symv:
		new_symv = (struct symbol **)Dee_TryReallocc(current_basescope->bs_argv,
		                                             new_arga, sizeof(struct symbol *));
		if unlikely(!new_symv) {
			if (new_arga != current_basescope->bs_argc + 1) {
				new_arga = current_basescope->bs_argc + 1;
				goto do_realloc_symv;
			}
			if (Dee_CollectMemoryc(new_arga, sizeof(struct symbol *)))
				goto do_realloc_symv;
			return -1;
		}
		current_basescope->bs_argv = new_symv;
		*p_arga                    = new_arga;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
resize_default_list(uint16_t *__restrict p_defaulta) {
	DREF DeeObject **new_defaultv;
	uint16_t defaultc;
	defaultc = (size_t)(current_basescope->bs_argc_max -
	                    current_basescope->bs_argc_min);
	ASSERT(defaultc <= *p_defaulta);
	if (defaultc >= *p_defaulta) {
		uint16_t new_defaulta = *p_defaulta * 2;
		if (!new_defaulta)
			new_defaulta = 2;
do_realloc_symv:
		new_defaultv = (DREF DeeObject **)Dee_TryReallocc(current_basescope->bs_default,
		                                                  new_defaulta, sizeof(DREF DeeObject *));
		if unlikely(!new_defaultv) {
			if (new_defaulta != defaultc + 1) {
				new_defaulta = defaultc + 1;
				goto do_realloc_symv;
			}
			if (Dee_CollectMemoryc(new_defaulta, sizeof(DREF DeeObject *)))
				goto do_realloc_symv;
			return -1;
		}
		current_basescope->bs_default = new_defaultv;
		*p_defaulta                   = new_defaulta;
	}
	return 0;
}


/* Parse the argument list of a function definition,
 * automatically creating new symbols for arguments,
 * as well as setting code flags for variadic arguments. */
INTERN WUNUSED int DCALL parse_arglist(void) {
	uint16_t defaulta, arga;
	DREF DeeObject **new_defaultv;
	struct symbol *arg;
	DREF DeeScopeObject *old_current_scope;
	ASSERT(!current_basescope->bs_argc_min);
	ASSERT(!current_basescope->bs_argc_max);
	ASSERT(!current_basescope->bs_argc);
	ASSERT(!current_basescope->bs_argv);
	ASSERT(!current_basescope->bs_varargs);
	ASSERT(!current_basescope->bs_varkwds);
	ASSERT(!current_basescope->bs_default);
	old_current_scope = current_scope;
	current_scope     = (DREF DeeScopeObject *)current_basescope;
	Dee_Incref(current_scope);
	defaulta = arga = 0;
	if (tok > 0 && tok != ')') {
		for (;;) {
			uint16_t symbol_flags;

			/* Special case: unnamed varargs. */
			if (tok == TOK_DOTS) {
				if unlikely(current_basescope->bs_flags & CODE_FVARARGS) {
					arg = current_basescope->bs_varargs;
					if likely(arg) {
						if (WARN(W_VARIABLE_ARGUMENT_ALREADY_DEFINED, arg))
							goto err;
						if unlikely(yield() < 0)
							goto err;
						goto parse_varargs_suffix;
					}
				}
				ASSERT(!current_basescope->bs_varargs);
				arg = new_unnamed_symbol();
				if unlikely(!arg)
					goto err;
				loc_here(&arg->s_decl);
				if (arg->s_decl.l_file)
					TPPFile_Incref(arg->s_decl.l_file);
				if unlikely(yield() < 0)
					goto err;
				arg->s_flag = SYMBOL_FALLOC;
set_arg_as_varargs_argument:
				if unlikely(resize_argument_list(&arga))
					goto err;
				/* Add the symbol to the argument symbol vector. */
				arg->s_type  = SYMBOL_TYPE_ARG;
				arg->s_symid = current_basescope->bs_argc;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				current_basescope->bs_varargs = arg;
				current_basescope->bs_flags |= CODE_FVARARGS;
parse_varargs_suffix:
				if unlikely(tok == '?') {
					if (WARN(W_UNEXPECTED_OPTIONAL_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					if unlikely(yield() < 0)
						goto err;
				}
				if unlikely(tok == TOK_DOTS) {
					if (WARN(W_UNEXPECTED_DOTS_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					if unlikely(yield() < 0)
						goto err;
				}
				if (tok == ':') {
					/* Parse argument declaration information. */
					if unlikely(yield() < 0)
						goto err;
					if unlikely(decl_ast_parse_for_symbol(arg))
						goto err;
				}
				if unlikely(tok == '=') {
					if (WARN(W_UNEXPECTED_DEFAULT_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					goto skip_default_suffix;
				}
				goto next_argument;
			}

			/* Parse variable modifier flags. */
			symbol_flags = SYMBOL_FNORMAL;
			for (;;) {
				if (tok == KWD_local) {
					/* ... */
				} else if (tok == KWD_final) {
					if ((symbol_flags & SYMBOL_FFINAL) &&
					    WARN(W_VARIABLE_MODIFIER_DUPLICATED))
						goto err;
					symbol_flags |= SYMBOL_FFINAL;
				} else if (tok == KWD_varying) {
					if ((symbol_flags & SYMBOL_FVARYING) &&
					    WARN(W_VARIABLE_MODIFIER_DUPLICATED))
						goto err;
					symbol_flags |= SYMBOL_FVARYING;
				} else {
					break;
				}
				if (yield() < 0)
					goto err;
			}

			/* Check for keyword arguments parameter. */
			if (tok == TOK_POW) {
				if unlikely(current_basescope->bs_flags & CODE_FVARKWDS) {
					arg = current_basescope->bs_varkwds;
					if likely(arg) {
						if (WARN(W_KEYWORD_ARGUMENT_ALREADY_DEFINED, arg))
							goto err;
						if (skip_argument_name())
							goto err;
						goto parse_varargs_suffix;
					}
				}
				if unlikely(yield() < 0)
					goto err;

				/* Parse the argument name. */
				arg = parse_argument_name();
				if unlikely(!arg)
					goto err;
				if unlikely(resize_argument_list(&arga))
					goto err;

				/* Add the symbol to the argument symbol vector. */
				arg->s_type  = SYMBOL_TYPE_ARG;
				arg->s_flag  = SYMBOL_FALLOC | symbol_flags;
				arg->s_symid = current_basescope->bs_argc;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				current_basescope->bs_varkwds = arg;
				current_basescope->bs_flags |= CODE_FVARKWDS;
				goto parse_varargs_suffix;
			}

			/* Parse the argument name. */
			arg = parse_argument_name();
			if unlikely(!arg)
				goto err;
			if (tok == TOK_DOTS) {
				/* Varargs argument. */
				if unlikely(yield() < 0)
					goto err;
				if likely(!current_basescope->bs_varargs) {
					arg->s_flag = SYMBOL_FALLOC | symbol_flags;
					goto set_arg_as_varargs_argument;
				}
				ASSERT(current_basescope->bs_flags & CODE_FVARARGS);
				if (WARN(W_VARIABLE_ARGUMENT_ALREADY_DEFINED, arg))
					goto err;
				arg->s_type = SYMBOL_TYPE_LOCAL;
				arg->s_flag = symbol_flags;
				goto parse_varargs_suffix;
			} else if (current_basescope->bs_varkwds || current_basescope->bs_varargs) {
				if (current_basescope->bs_varkwds
				    ? WARN(W_POSITIONAL_ARGUMENT_AFTER_VARKWDS, arg, current_basescope->bs_varkwds)
				    : WARN(W_POSITIONAL_ARGUMENT_AFTER_VARARGS, arg, current_basescope->bs_varargs))
					goto err;
set_argument_as_local:
				arg->s_type = SYMBOL_TYPE_LOCAL;
				arg->s_flag = symbol_flags;
				if (tok == '?' && unlikely(yield() < 0))
					goto err;
				if (tok == ':') {
					if unlikely(yield() < 0)
						goto err;
					if unlikely(decl_ast_parse_for_symbol(arg))
						goto err;
				}
				if (tok == '=')
					goto skip_default_suffix;
			} else if (tok == '?') { /* Optional argument */
				if unlikely(yield() < 0)
					goto err;
				arg->s_type  = SYMBOL_TYPE_ARG;
				arg->s_flag  = SYMBOL_FALLOC | symbol_flags;
				arg->s_symid = current_basescope->bs_argc;
				if unlikely(resize_argument_list(&arga))
					goto err;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				if unlikely(resize_default_list(&defaulta))
					goto err;

				/* Set a default value of NULL to indicate a variable that is unbound by default. */
				current_basescope->bs_default[current_basescope->bs_argc_max -
				                              current_basescope->bs_argc_min] = NULL;
				++current_basescope->bs_argc_max;
				if (tok == ':') {
					if unlikely(yield() < 0)
						goto err;
					if unlikely(decl_ast_parse_for_symbol(arg))
						goto err;
				}
				if unlikely(tok == '=') {
					DREF struct ast *default_expr;
					if (WARN(W_UNEXPECTED_DEFAULT_AFTER_OPTIONAL, arg))
						goto err;
skip_default_suffix:
					if unlikely(yield() < 0)
						goto err;

					/* Parse & discard the default expression. */
					default_expr = ast_parse_expr(LOOKUP_SYM_NORMAL);
					if unlikely(!default_expr)
						goto err;
					ast_decref(default_expr);
					goto next_argument;
				}
			} else if (tok == ':') { /* Declaration suffix. */
				if unlikely(yield() < 0)
					goto err;
				if unlikely(decl_ast_parse_for_symbol(arg))
					goto err;
				if (tok == '=')
					goto parse_default_suffix;
				goto set_arg_as_normal;
			} else if (tok == '=') { /* Default argument */
				DREF DeeObject *default_value;
				DREF struct ast *default_expr;
parse_default_suffix:
				if unlikely(yield() < 0)
					goto err;
				default_expr = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if unlikely(!default_expr)
					goto err;
				if (ast_optimize_all(default_expr, true)) {
err_default_expr:
					ast_decref(default_expr);
					goto err;
				}
				if unlikely(resize_argument_list(&arga))
					goto err;
				if unlikely(resize_default_list(&defaulta))
					goto err;
				if (default_expr->a_type != AST_CONSTEXPR) {
					if (WARNAST(default_expr, W_EXPECTED_CONSTANT_EXPRESSION_FOR_ARGUMENT_DEFAULT, arg))
						goto err_default_expr;
					default_value = DeeNone_NewRef();
				} else {
					default_value = default_expr->a_constexpr;
					Dee_Incref(default_value);
				}
				ast_decref(default_expr);
				arg->s_type  = SYMBOL_TYPE_ARG;
				arg->s_flag  = SYMBOL_FALLOC | symbol_flags;
				arg->s_symid = current_basescope->bs_argc;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				current_basescope->bs_default[current_basescope->bs_argc_max -
				                              current_basescope->bs_argc_min] = default_value; /* Inherit reference. */
				++current_basescope->bs_argc_max;
			} else {
set_arg_as_normal:
				ASSERT(current_basescope->bs_argc_min <= current_basescope->bs_argc_max);
				if (current_basescope->bs_argc_min < current_basescope->bs_argc_max) {
					/* Positional-after-optional */
					if (WARN(W_POSITIONAL_ARGUMENT_AFTER_OPTIONAL_OR_DEFAULT, arg))
						goto err;
					goto set_argument_as_local;
				}

				/* Mandatory, positional argument. */
				arg->s_type  = SYMBOL_TYPE_ARG;
				arg->s_flag  = SYMBOL_FALLOC | symbol_flags;
				arg->s_symid = current_basescope->bs_argc;
				ASSERT(current_basescope->bs_argc_min == current_basescope->bs_argc);
				ASSERT(current_basescope->bs_argc_min == current_basescope->bs_argc_max);
				if unlikely(resize_argument_list(&arga))
					goto err;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				++current_basescope->bs_argc_min;
				++current_basescope->bs_argc_max;
			}
next_argument:
			if (tok != ',')
				break;
			if unlikely(yield() < 0)
				goto err;
			if (tok == ')')
				break;
		}
	}
/*done:*/
	ASSERT(current_basescope->bs_argc_min <=
	       current_basescope->bs_argc_max);
	ASSERT(arga >= current_basescope->bs_argc);

	/* Truncate the argument vector. */
	if (arga != current_basescope->bs_argc) {
		struct symbol **new_symv;
		new_symv = (struct symbol **)Dee_TryReallocc(current_basescope->bs_argv,
		                                             current_basescope->bs_argc,
		                                             sizeof(struct symbol *));
		if likely(new_symv)
			current_basescope->bs_argv = new_symv;
	}
	if (current_basescope->bs_default) {
		/* Truncate the default argument vector. */
		uint16_t req_defaulta = (current_basescope->bs_argc_max - current_basescope->bs_argc_min);
		if (defaulta != req_defaulta) {
			new_defaultv = (DREF DeeObject **)Dee_TryReallocc(current_basescope->bs_default,
			                                                  req_defaulta, sizeof(DREF DeeObject *));
			if likely(new_defaultv)
				current_basescope->bs_default = new_defaultv;
		}
	}
	ASSERT(current_basescope->bs_argc_max >= current_basescope->bs_argc_min);
	ASSERT((current_basescope->bs_varargs == NULL) || (current_basescope->bs_flags & CODE_FVARARGS));
	ASSERT((current_basescope->bs_varkwds == NULL) || (current_basescope->bs_flags & CODE_FVARKWDS));
	ASSERT((current_basescope->bs_default != NULL) == (current_basescope->bs_argc_max > current_basescope->bs_argc_min));
	ASSERT(current_basescope->bs_argc ==
	       current_basescope->bs_argc_max +
	       (current_basescope->bs_varkwds ? 1 : 0) +
	       (current_basescope->bs_varargs ? 1 : 0));
	Dee_Decref(current_scope);
	current_scope = old_current_scope;
	return 0;
err:
	/* This needs to be done to ensure a consistent scope on exit. */
	Dee_Decref(current_scope);
	current_scope = old_current_scope;
	return -1;
}

INTERN WUNUSED DREF struct ast *DCALL
ast_parse_function(struct TPPKeyword *name, bool *p_need_semi,
                   bool allow_missing_params,
                   struct ast_loc *name_loc,
                   struct decl_ast *decl,
                   /*[0..1]*/ struct symbol *function_symbol) {
	DREF struct ast *result;
	struct ast_annotations annotations;
	ast_annotations_get(&annotations);
	if unlikely(basescope_push())
		goto err_anno;
	current_basescope->bs_flags |= current_tags.at_code_flags;
	result = ast_parse_function_noscope(name, p_need_semi, allow_missing_params,
	                                    name_loc, decl, function_symbol);
	basescope_pop();
	if unlikely(!result)
		goto err_anno;
	return ast_annotations_apply(&annotations, result);
err_anno:
	ast_annotations_free(&annotations);
	return NULL;
}

INTERN WUNUSED DREF struct ast *DCALL
ast_parse_function_noscope(struct TPPKeyword *name,
                           bool *p_need_semi,
                           bool allow_missing_params,
                           struct ast_loc *name_loc,
                           struct decl_ast *decl,
                           /*[0..1]*/ struct symbol *function_symbol) {
	struct decl_ast my_decl;
	struct symbol *funcself_symbol = NULL;
	uint32_t old_flags;
	DREF struct ast *result, *code;
	/* Add information from tags. */
	if (name) {
		/* Save the function name in the base scope. */
		current_basescope->bs_name = name;

		/* Create a new symbol to allow for function-self-referencing. */
		funcself_symbol = new_local_symbol(name, name_loc);
		if unlikely(!funcself_symbol)
			goto err;
		funcself_symbol->s_type = SYMBOL_TYPE_MYFUNC;
	}

	/* Declaration meta-information */
	decl_ast_initfunc(&my_decl, NULL, current_basescope);

	if (tok == '(') {
		/* Argument list. */
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags_decl;
		if unlikely(parse_arglist())
			goto err_flags_decl;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_ARGLIST))
			goto err_decl;
	} else if (!allow_missing_params) {
		if (WARN(W_DEPRECATED_NO_PARAMETER_LIST))
			goto err_decl;
	}

	if (tok == ':') {
		struct decl_ast *return_type;
		if unlikely(yield() < 0)
			goto err_decl;
		/* Function return type information. */
		ASSERT(!my_decl.da_func.f_ret);
		return_type = (struct decl_ast *)Dee_Malloc(sizeof(struct decl_ast));
		if unlikely(!return_type)
			goto err_decl;
		if unlikely(decl_ast_parse(return_type)) {
			Dee_Free(return_type);
			goto err_decl;
		}
		my_decl.da_func.f_ret = return_type; /* Inherit */
	}

	/* Copy declaration information into the function symbol (if it exists) */
	if (funcself_symbol) {
		if (funcself_symbol->s_decltype.da_type != DAST_NONE) {
			bool are_equal;
			are_equal = decl_ast_equal(&funcself_symbol->s_decltype,
			                           &my_decl);
			decl_ast_fini(&my_decl);
			if (!are_equal) {
				if (WARN(W_SYMBOL_TYPE_DECLARATION_CHANGED, funcself_symbol))
					goto err;
			}
		} else {
			decl_ast_move(&funcself_symbol->s_decltype, &my_decl);
		}
		my_decl.da_type = DAST_NONE;
	}

	if (tok == TOK_ARROW) {
		struct ast_loc arrow_loc;
		loc_here(&arrow_loc);
		if unlikely(yield() < 0)
			goto err_decl;

		/* Expression function. */
		code = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!code)
			goto err_decl;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= CODE_FYIELDING, ast_yield(code))
		         : (current_basescope->bs_cflags |= BASESCOPE_FRETURN, ast_return(code));
		ast_decref(code);
		code = ast_setddi(result, &arrow_loc);
		if (p_need_semi)
			*p_need_semi = true;
	} else if (tok == '{') {
		struct ast_loc brace_loc;
		loc_here(&brace_loc);
		old_flags = TPPLexer_Current->l_flags;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags_decl;
		code = ast_putddi(ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, '}'), &brace_loc);
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip('}', W_EXPECTED_RBRACE_AFTER_FUNCTION))
			goto err_decl_xcode;
		if (p_need_semi)
			*p_need_semi = false;
	} else {
		/* Missing function body (this was allowed in deemon 100+, where
		 * this was interpreted the same way an `{ }'-like empty body would
		 * have been)
		 *
		 * Back then, the intend was to go hand-in-hand with the user being
		 * able to re-assign the code of a function after that function had
		 * already been created, thus allowing for self-referencing functions
		 * which were generated as:
		 * >> local x = function();
		 * >> local x := function(n) {
		 * >>     print "x(" + n + ")";
		 * >>     if (n < 10) {
		 * >>         // Self-reference via reference (`x' already had
		 * >>         // a value when the function was assigned, thus
		 * >>         // allowing that value to be referenced like any
		 * >>         // other referenced variable)
		 * >>         x(n + 1);
		 * >>     }
		 * >> }
		 * >> x(0);
		 *
		 * But that's no longer allowed since functions don't implement the
		 * `operator assign' anymore (they are immutable once created), and
		 * self-referencing functions are done by ASM_THIS_FUNCTION which
		 * will push the current function onto the stack (thus allowing a
		 * function to reference itself)
		 *
		 * Still: we *do* parse functions without a body for the sake of
		 *        syntax compatibility with deemon 100+.
		 */
		if (WARN(W_EXPECTED_LBRACE_AFTER_FUNCTION))
			goto err;
		/* Make the symbol that the function will be stored
		 * in as "varying" so it can be reassigned later. */
		if (function_symbol) {
			function_symbol->s_flag |= SYMBOL_FVARYING;
			function_symbol->s_flag &= ~SYMBOL_FFINAL;
		}
		code = ast_multiple(AST_FMULTIPLE_KEEPLAST, 0, NULL);
		if (p_need_semi)
			*p_need_semi = true;
	}
	if unlikely(!code)
		goto err_decl;
	result = ast_function(code, current_basescope);
	ast_decref(code);
	if unlikely(!result)
		goto err_decl;

	/* Hack: The function AST itself must be located in the caller's scope. */
	Dee_Decref(result->a_scope);
	ASSERT(current_basescope->bs_scope.s_prev);
	result->a_scope = current_basescope->bs_scope.s_prev;
	Dee_Incref(result->a_scope);
	ASSERT(!funcself_symbol || my_decl.da_type == DAST_NONE);
	if (decl) {
		/* Pass Declaration information to the caller. */
		if (funcself_symbol) {
			if unlikely(decl_ast_copy(decl, &funcself_symbol->s_decltype))
				Dee_Clear(result);
		} else {
			decl_ast_move(decl, &my_decl);
		}
	} else {
		decl_ast_fini(&my_decl);
	}
	return ast_setddi(result, name_loc);
err_flags_decl:
	decl_ast_fini(&my_decl);
/*err_flags:*/
	TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	goto err;
err_decl_xcode:
	ast_xdecref(code);
err_decl:
	decl_ast_fini(&my_decl);
err:
	return NULL;
}

INTERN WUNUSED DREF struct ast *DCALL
ast_parse_function_noscope_noargs(bool *p_need_semi) {
	uint32_t old_flags;
	DREF struct ast *result, *code;
	if (tok == TOK_ARROW) {
		struct ast_loc arrow_loc;
		loc_here(&arrow_loc);
		if unlikely(yield() < 0)
			goto err;
		/* Expression function. */
		code = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!code)
			goto err;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= CODE_FYIELDING, ast_yield(code))
		         : (current_basescope->bs_cflags |= BASESCOPE_FRETURN, ast_return(code));
		ast_decref(code);
		code = ast_setddi(result, &arrow_loc);
		if (p_need_semi)
			*p_need_semi = true;
	} else if (tok == '{') {
		struct ast_loc brace_loc;
		loc_here(&brace_loc);
		old_flags = TPPLexer_Current->l_flags;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		code = ast_putddi(ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, '}'), &brace_loc);
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip('}', W_EXPECTED_RBRACE_AFTER_FUNCTION))
			goto err_xcode;
		if (p_need_semi)
			*p_need_semi = false;
	} else {
		/* Missing function body (this was allowed in deemon 100+, where
		 * this was interpreted the same way an `{ }'-like empty body would
		 * have been) */
		if (WARN(W_EXPECTED_LBRACE_AFTER_FUNCTION))
			goto err;
		code = ast_multiple(AST_FMULTIPLE_KEEPLAST, 0, NULL);
		if (p_need_semi)
			*p_need_semi = true;
	}
	if unlikely(!code)
		goto err;
	result = ast_function(code, current_basescope);
	ast_decref(code);
	if unlikely(!result)
		goto err;

	/* Hack: The function AST itself must be located in the caller's scope. */
	Dee_Decref(result->a_scope);
	ASSERT(current_basescope->bs_scope.s_prev);
	result->a_scope = current_basescope->bs_scope.s_prev;
	Dee_Incref(result->a_scope);
	return result;
err_flags:
	TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	goto err;
err_xcode:
	ast_xdecref(code);
err:
	return NULL;
}



/* Parse a `() -> 42' or `a -> a+42'-style lambda.
 * In either case, upon entry the current token must be the '->' */
INTERN WUNUSED DREF struct ast *DCALL
ast_parse_function_java_lambda(struct TPPKeyword *first_argument_name,
                               struct ast_loc *first_argument_loc) {
	struct ast_loc arrow_loc;
	DREF struct ast *result, *code;
	if unlikely(basescope_push())
		goto err;
	if (first_argument_name) {
		struct symbol *arg;
		if (first_argument_name->k_id == KWD_none) {
			/* Create a new symbol for the argument. */
			arg = new_unnamed_symbol();
			if unlikely(!arg)
				goto err_scope;
			arg->s_decl = *first_argument_loc;
			if (arg->s_decl.l_file)
				TPPFile_Incref(arg->s_decl.l_file);
		} else {
			/* Check if the argument name is a reserved identifier. */
			if (is_reserved_symbol_name(first_argument_name)) {
				if (WARN(W_RESERVED_ARGUMENT_NAME, first_argument_name))
					goto err_scope;
			}

			/* Create a new symbol for the argument. */
			arg = new_local_symbol(first_argument_name, first_argument_loc);
		}
		if unlikely(!arg)
			goto err_scope;
		ASSERT(current_basescope->bs_argc_min == 0);
		ASSERT(current_basescope->bs_argc_max == 0);
		ASSERT(current_basescope->bs_argv == NULL);
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_scope;

		/* Mandatory, positional argument. */
		arg->s_type  = SYMBOL_TYPE_ARG;
		arg->s_flag  = SYMBOL_FALLOC | SYMBOL_FNORMAL;
		arg->s_symid = 0;
		current_basescope->bs_argv[0] = arg;
		current_basescope->bs_argc_min = 1;
		current_basescope->bs_argc_max = 1;
		current_basescope->bs_argc     = 1;
	} else if (tok != TOK_ARROW && tok != ':') {
		uint32_t old_flags;
		int error;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		error = parse_arglist();
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if unlikely(error)
			goto err_scope;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_ARGLIST))
			goto err_scope;
	} else {
		/* No arguments */
		ASSERT(current_basescope->bs_argc_min == 0);
		ASSERT(current_basescope->bs_argc_max == 0);
		ASSERT(current_basescope->bs_argv == NULL);
	}

	if (tok == ':') {
		struct decl_ast temp;
		if unlikely(yield() < 0)
			goto err_scope;
		if unlikely(decl_ast_parse(&temp))
			goto err_scope;
		decl_ast_fini(&temp);
	}

	ASSERT(tok == TOK_ARROW);
	loc_here(&arrow_loc);
	if (yield() < 0)
		goto err;

	/* At this point, we're at the start of the lambda expression,
	 * or the '{' in case it uses statements (or returns a sequence) */
	if (tok == '{') {
		unsigned int was_expression;
		code = ast_parse_statement_or_braces(&was_expression);
		if (was_expression != AST_PARSE_WASEXPR_NO)
			goto wrap_code_with_return;
	} else {
		code = ast_parse_expr(LOOKUP_SYM_NORMAL);
wrap_code_with_return:
		if unlikely(!code)
			goto err_scope;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= CODE_FYIELDING, ast_yield(code))
		         : (current_basescope->bs_cflags |= BASESCOPE_FRETURN, ast_return(code));
		ast_decref(code);
		code = ast_setddi(result, &arrow_loc);
	}

	if unlikely(!code)
		goto err;
	result = ast_function(code, current_basescope);
	ast_decref(code);
	if unlikely(!result)
		goto err;

	/* Hack: The function AST itself must be located in the caller's scope. */
	Dee_Decref(result->a_scope);
	ASSERT(current_basescope->bs_scope.s_prev);
	result->a_scope = current_basescope->bs_scope.s_prev;
	Dee_Incref(result->a_scope);
	basescope_pop();
	return result;
err_scope:
	basescope_pop();
err:
	return NULL;
}


/* Check if the parser is located after the '(' of a java-style lambda.
 * @return:  1: Yes
 * @return:  0: No
 * @return: -1: Error */
INTERN WUNUSED int DCALL ast_is_after_lparen_of_java_lambda(void) {
	struct TPPLexerPosition pos;
	if (!TPP_ISKEYWORD(tok)) {
		if (tok == TOK_POW) {
			/* `**' can only appear in argument- and parameter lists.
			 * Since out parent is parsing an argument-list, with a
			 * comma-list as fallback, we know that this is varkwds! */
			goto yes;
		}
		if (tok == TOK_DOTS) {
			/* Special case: there are 4 cases where this can still be an argument list:
			 * >> (...) -> [...][0];           // anonymous varargs
			 * >> (...,) -> [...][0];          // *ditto*
			 * >> (..., **kwds) -> [...][0];   // anonymous varargs, followed by varkwds
			 * >> (..., **kwds,) -> [...][0];  // *ditto* */
			if unlikely(!TPPLexer_SavePosition(&pos))
				goto err;
			if unlikely(yield() < 0)
				goto err_restore;
			if (tok == ',' && unlikely(yield() < 0))
				goto err_restore;
			if (tok == TOK_POW) {
				if unlikely(yield() < 0)
					goto err_restore;
				if (!TPP_ISKEYWORD(tok))
					goto nope_restore;
				if unlikely(yield() < 0)
					goto err_restore;
				if (tok == ',' && unlikely(yield() < 0))
					goto err_restore;
			}
			goto check_and_consume_rparen;
		}
		goto nope;
	}
	if unlikely(!TPPLexer_SavePosition(&pos))
		goto err;

	/* Now try to skip the argument list. */
	for (;;) {
		/* Special case: unnamed varargs. */
		if (tok == TOK_DOTS) {
			if unlikely(yield() < 0)
				goto err_restore;
		} else {
			/* Parse variable modifier flags. */
			while (tok == KWD_local || tok == KWD_final || tok == KWD_varying) {
				if unlikely(yield() < 0)
					goto err_restore;
			}
			if (tok == TOK_POW && unlikely(yield() < 0))
				goto err_restore;
			if (!TPP_ISKEYWORD(tok))
				goto nope_restore;
			if unlikely(yield() < 0) /* Argument name. */
				goto err_restore;
			if (tok == TOK_DOTS && unlikely(yield() < 0))
				goto err_restore;
		}
		if (tok == '?') {
			if unlikely(yield() < 0)
				goto err_restore;
			if (tok == ',' || tok == ')')
				goto yes_restore; /* Something like `(foo?)' is guarantied to be a paren-lambda. */
			if (tok == ':') {
				/* Parse argument declaration information. */
				if unlikely(yield() < 0)
					goto err_restore;
				if unlikely(decl_ast_skip())
					goto err_restore;
			}
		} else {
			if (tok == ':') /* Something like `(foo: int)' is guarantied to be a paren-lambda. */
				goto yes_restore;
		}
		if unlikely(tok == '=') {
			struct ast *temp;
			if unlikely(yield() < 0)
				goto err_restore;
			/* Parse & discard the default expression. */
			temp = ast_parse_expr(LOOKUP_SYM_NORMAL);
			if unlikely(!temp)
				goto err_restore;
			ast_decref(temp);
		}
		if (tok != ',')
			break;
		if unlikely(yield() < 0)
			goto err_restore;
		if (tok == ')')
			break;
	}
check_and_consume_rparen:
	if (tok != ')')
		goto nope_restore;
	if unlikely(yield() < 0)
		goto err_restore;
	if (tok == ':') {
		/* Parse argument declaration information. */
		if unlikely(yield() < 0)
			goto err_restore;
		if unlikely(decl_ast_skip())
			goto err_restore;
	}
	if (tok != TOK_ARROW)
		goto nope_restore;
yes_restore:
	TPPLexer_LoadPosition(&pos);
yes:
	return 1;
nope_restore:
	TPPLexer_LoadPosition(&pos);
nope:
	return 0;
err_restore:
	TPPLexer_LoadPosition(&pos);
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C */
