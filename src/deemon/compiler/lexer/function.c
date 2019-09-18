/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C
#define GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>
#include <deemon/none.h>

DECL_BEGIN

PRIVATE int DCALL skip_argument_name(void) {
	if
		unlikely(!TPP_ISKEYWORD(tok))
	{
		if (WARN(W_EXPECTED_KEYWORD_FOR_ARGUMENT_NAME))
			goto err;
	}
	else {
		if (tok != KWD_none) {
			if (has_local_symbol(token.t_kwd)) {
				if (WARN(W_ARGUMENT_NAME_ALREADY_IN_USE))
					goto err;
			} else if (is_reserved_symbol_name(token.t_kwd)) {
				if (WARN(W_RESERVED_ARGUMENT_NAME, token.t_kwd))
					goto err;
			}
		}
		if
			unlikely(yield() < 0)
		goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE struct symbol *DCALL parse_argument_name(void) {
	struct symbol *result;
	struct TPPKeyword *argument_name;
	if
		unlikely(!TPP_ISKEYWORD(tok))
	{
		if (WARN(W_EXPECTED_KEYWORD_FOR_ARGUMENT_NAME))
			goto err;
		result = new_unnamed_symbol();
	}
	else {
		if (tok == KWD_none) {
			/* Special case: Allow `none' to be used for placeholder/pending arguments. */
		create_anon_argument:
			/* Create a new symbol for the argument. */
			result = new_unnamed_symbol();
			if
				unlikely(!result)
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
		if
			unlikely(yield() < 0)
		goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE int DCALL resize_argument_list(uint16_t *__restrict parga) {
	struct symbol **new_symv;
	ASSERT(current_basescope->bs_argc <= *parga);
	if (current_basescope->bs_argc >= *parga) {
		uint16_t new_arga = *parga * 2;
		if (!new_arga)
			new_arga = 2;
	do_realloc_symv:
		new_symv = (struct symbol **)Dee_TryRealloc(current_basescope->bs_argv,
		                                            new_arga * sizeof(struct symbol *));
		if
			unlikely(!new_symv)
		{
			if (new_arga != current_basescope->bs_argc + 1) {
				new_arga = current_basescope->bs_argc + 1;
				goto do_realloc_symv;
			}
			if (Dee_CollectMemory(new_arga * sizeof(struct symbol *)))
				goto do_realloc_symv;
			return -1;
		}
		current_basescope->bs_argv = new_symv;
		*parga                     = new_arga;
	}
	return 0;
}

PRIVATE int DCALL resize_default_list(uint16_t *__restrict pdefaulta) {
	DREF DeeObject **new_defaultv;
	uint16_t defaultc;
	defaultc = (size_t)(current_basescope->bs_argc_max - current_basescope->bs_argc_min);
	ASSERT(defaultc <= *pdefaulta);
	if (defaultc >= *pdefaulta) {
		uint16_t new_defaulta = *pdefaulta * 2;
		if (!new_defaulta)
			new_defaulta = 2;
	do_realloc_symv:
		new_defaultv = (DREF DeeObject **)Dee_TryRealloc(current_basescope->bs_default,
		                                                 new_defaulta * sizeof(DREF DeeObject *));
		if
			unlikely(!new_defaultv)
		{
			if (new_defaulta != defaultc + 1) {
				new_defaulta = defaultc + 1;
				goto do_realloc_symv;
			}
			if (Dee_CollectMemory(new_defaulta * sizeof(DREF DeeObject *)))
				goto do_realloc_symv;
			return -1;
		}
		current_basescope->bs_default = new_defaultv;
		*pdefaulta                    = new_defaulta;
	}
	return 0;
}


INTERN int DCALL parse_arglist(void) {
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
	if (tok > 0 && tok != ')')
		for (;;) {
			uint16_t symbol_flags;

			/* Special case: unnamed varargs. */
			if (tok == TOK_DOTS) {
				if
					unlikely(current_basescope->bs_flags & CODE_FVARARGS)
				{
					arg = current_basescope->bs_varargs;
					if
						likely(arg)
					{
						if (WARN(W_VARIABLE_ARGUMENT_ALREADY_DEFINED, arg))
							goto err;
						if
							unlikely(yield() < 0)
						goto err;
						goto parse_varargs_suffix;
					}
				}
				ASSERT(!current_basescope->bs_varargs);
				arg = new_unnamed_symbol();
				if
					unlikely(!arg)
				goto err;
				loc_here(&arg->s_decl);
				if (arg->s_decl.l_file)
					TPPFile_Incref(arg->s_decl.l_file);
				if
					unlikely(yield() < 0)
				goto err;
				arg->s_flag = SYMBOL_FALLOC;
			set_arg_as_varargs_argument:
				if
					unlikely(resize_argument_list(&arga))
				goto err;
				/* Add the symbol to the argument symbol vector. */
				arg->s_type                                              = SYMBOL_TYPE_ARG;
				arg->s_symid                                             = current_basescope->bs_argc;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				current_basescope->bs_varargs                            = arg;
				current_basescope->bs_flags |= CODE_FVARARGS;
			parse_varargs_suffix:
				if
					unlikely(tok == '?')
				{
					if (WARN(W_UNEXPECTED_OPTIONAL_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					if
						unlikely(yield() < 0)
					goto err;
				}
				if
					unlikely(tok == TOK_DOTS)
				{
					if (WARN(W_UNEXPECTED_DOTS_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					if
						unlikely(yield() < 0)
					goto err;
				}
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
				if (tok == ':') {
					/* Parse argument declaration information. */
					if
						unlikely(yield() < 0)
					goto err;
					if
						unlikely(decl_ast_parse_for_symbol(arg))
					goto err;
				}
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
				if
					unlikely(tok == '=')
				{
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
				} else if (tok == KWD_final) {
					if (symbol_flags & SYMBOL_FFINAL &&
					    WARN(W_VARIABLE_MODIFIER_DUPLICATED))
						goto err;
					symbol_flags |= SYMBOL_FFINAL;
				} else if (tok == KWD_varying) {
					if (symbol_flags & SYMBOL_FVARYING &&
					    WARN(W_VARIABLE_MODIFIER_DUPLICATED))
						goto err;
					symbol_flags |= SYMBOL_FVARYING;
				} else
					break;
				if (yield() < 0)
					goto err;
			}
			/* Check for keyword arguments parameter. */
			if (tok == TOK_POW) {
				if
					unlikely(current_basescope->bs_flags & CODE_FVARKWDS)
				{
					arg = current_basescope->bs_varkwds;
					if
						likely(arg)
					{
						if (WARN(W_KEYWORD_ARGUMENT_ALREADY_DEFINED, arg))
							goto err;
						if (skip_argument_name())
							goto err;
						goto parse_varargs_suffix;
					}
				}
				if
					unlikely(yield() < 0)
				goto err;
				/* Parse the argument name. */
				arg = parse_argument_name();
				if
					unlikely(!arg)
				goto err;
				if
					unlikely(resize_argument_list(&arga))
				goto err;
				/* Add the symbol to the argument symbol vector. */
				arg->s_type                                              = SYMBOL_TYPE_ARG;
				arg->s_flag                                              = SYMBOL_FALLOC | symbol_flags;
				arg->s_symid                                             = current_basescope->bs_argc;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				current_basescope->bs_varkwds                            = arg;
				current_basescope->bs_flags |= CODE_FVARKWDS;
				goto parse_varargs_suffix;
			}

			/* Parse the argument name. */
			arg = parse_argument_name();
			if
				unlikely(!arg)
			goto err;
			if (tok == TOK_DOTS) {
				/* Varargs argument. */
				if
					unlikely(yield() < 0)
				goto err;
				if
					likely(!current_basescope->bs_varargs)
				{
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
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
				if (tok == ':') {
					if
						unlikely(yield() < 0)
					goto err;
					if
						unlikely(decl_ast_parse_for_symbol(arg))
					goto err;
				}
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
				if (tok == '=')
					goto skip_default_suffix;
			} else if (tok == '?') { /* Optional argument */
				if
					unlikely(yield() < 0)
				goto err;
				arg->s_type  = SYMBOL_TYPE_ARG;
				arg->s_flag  = SYMBOL_FALLOC | symbol_flags;
				arg->s_symid = current_basescope->bs_argc;
				if
					unlikely(resize_argument_list(&arga))
				goto err;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				if
					unlikely(resize_default_list(&defaulta))
				goto err;
				/* Set a default value of NULL to indicate a variable that is unbound by default. */
				current_basescope->bs_default[current_basescope->bs_argc_max -
				                              current_basescope->bs_argc_min] = NULL;
				++current_basescope->bs_argc_max;
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
				if (tok == ':') {
					if
						unlikely(yield() < 0)
					goto err;
					if
						unlikely(decl_ast_parse_for_symbol(arg))
					goto err;
				}
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
				if
					unlikely(tok == '=')
				{
					DREF struct ast *default_expr;
					if (WARN(W_UNEXPECTED_DEFAULT_AFTER_OPTIONAL, arg))
						goto err;
				skip_default_suffix:
					if
						unlikely(yield() < 0)
					goto err;
					/* Parse & discard the default expression. */
					default_expr = ast_parse_expr(LOOKUP_SYM_NORMAL);
					if
						unlikely(!default_expr)
					goto err;
					ast_decref(default_expr);
					goto next_argument;
				}
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
			} else if (tok == ':') { /* Declaration suffix. */
				if
					unlikely(yield() < 0)
				goto err;
				if
					unlikely(decl_ast_parse_for_symbol(arg))
				goto err;
				if (tok == '=')
					goto parse_default_suffix;
				goto set_arg_as_normal;
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
			} else if (tok == '=') { /* Default argument */
				DREF DeeObject *default_value;
				DREF struct ast *default_expr;
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
			parse_default_suffix:
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
				if
					unlikely(yield() < 0)
				goto err;
				default_expr = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if
					unlikely(!default_expr)
				goto err;
				if (ast_optimize_all(default_expr, true)) {
				err_default_expr:
					ast_decref(default_expr);
					goto err;
				}
				if
					unlikely(resize_argument_list(&arga))
				goto err;
				if
					unlikely(resize_default_list(&defaulta))
				goto err;
				if (default_expr->a_type != AST_CONSTEXPR) {
					if (WARNAST(default_expr, W_EXPECTED_CONSTANT_EXPRESSION_FOR_ARGUMENT_DEFAULT, arg))
						goto err_default_expr;
					default_value = Dee_None;
					Dee_Incref(Dee_None);
				} else {
					default_value = default_expr->a_constexpr;
					Dee_Incref(default_value);
				}
				ast_decref(default_expr);
				arg->s_type                                                   = SYMBOL_TYPE_ARG;
				arg->s_flag                                                   = SYMBOL_FALLOC | symbol_flags;
				arg->s_symid                                                  = current_basescope->bs_argc;
				current_basescope->bs_argv[current_basescope->bs_argc++]      = arg;
				current_basescope->bs_default[current_basescope->bs_argc_max -
				                              current_basescope->bs_argc_min] = default_value; /* Inherit reference. */
				++current_basescope->bs_argc_max;
			} else {
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
			set_arg_as_normal:
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
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
				if
					unlikely(resize_argument_list(&arga))
				goto err;
				current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
				++current_basescope->bs_argc_min;
				++current_basescope->bs_argc_max;
			}
		next_argument:
			if (tok != ',')
				break;
			if
				unlikely(yield() < 0)
			goto err;
		}
	/*done:*/
	ASSERT(current_basescope->bs_argc_min <=
	       current_basescope->bs_argc_max);
	ASSERT(arga >= current_basescope->bs_argc);
	/* Truncate the argument vector. */
	if (arga != current_basescope->bs_argc) {
		struct symbol **new_symv;
		new_symv = (struct symbol **)Dee_TryRealloc(current_basescope->bs_argv,
		                                            current_basescope->bs_argc *
		                                            sizeof(struct symbol *));
		if
			likely(new_symv)
		current_basescope->bs_argv = new_symv;
	}
	if (current_basescope->bs_default) {
		/* Truncate the default argument vector. */
		uint16_t req_defaulta = (current_basescope->bs_argc_max - current_basescope->bs_argc_min);
		if (defaulta != req_defaulta) {
			new_defaultv = (DREF DeeObject **)Dee_TryRealloc(current_basescope->bs_default,
			                                                 req_defaulta * sizeof(DREF DeeObject *));
			if
				likely(new_defaultv)
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

INTERN DREF struct ast *DCALL
ast_parse_function(struct TPPKeyword *name, bool *pneed_semi,
                   bool allow_missing_params,
                   struct ast_loc *name_loc
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
                   ,
                   struct decl_ast *decl
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
                   ) {
	DREF struct ast *result;
	struct ast_annotations annotations;
	ast_annotations_get(&annotations);
	if
		unlikely(basescope_push())
	goto err_anno;
	current_basescope->bs_flags |= current_tags.at_code_flags;
	result = ast_parse_function_noscope(name, pneed_semi, allow_missing_params, name_loc
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
	                                    ,
	                                    decl
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
	                                    );
	basescope_pop();
	if
		unlikely(!result)
	goto err_anno;
	return ast_annotations_apply(&annotations, result);
err_anno:
	ast_annotations_free(&annotations);
	return NULL;
}

INTERN DREF struct ast *DCALL
ast_parse_function_noscope(struct TPPKeyword *name,
                           bool *pneed_semi,
                           bool allow_missing_params,
                           struct ast_loc *name_loc
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
                           ,
                           struct decl_ast *__restrict decl
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
) {
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
	struct decl_ast my_decl;
	struct symbol *funcself_symbol = NULL;
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
	uint32_t old_flags;
	DREF struct ast *result, *code;
	/* Add information from tags. */
	if (name) {
#ifndef CONFIG_HAVE_DECLARATION_DOCUMENTATION
		struct symbol *funcself_symbol;
#endif /* !CONFIG_HAVE_DECLARATION_DOCUMENTATION */
		/* Save the function name in the base scope. */
		current_basescope->bs_name = name;
		/* Create a new symbol to allow for function-self-referencing. */
		funcself_symbol = new_local_symbol(name, name_loc);
		if
			unlikely(!funcself_symbol)
		goto err;
		SYMBOL_TYPE(funcself_symbol) = SYMBOL_TYPE_MYFUNC;
	}
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
	/* Declaration meta-information */
	decl_ast_initfunc(&my_decl, NULL, current_basescope);
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
	if (tok == '(') {
		/* Argument list. */
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if
			unlikely(yield() < 0)
		goto err_flags_decl;
		if
			unlikely(parse_arglist())
		goto err_flags_decl;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if
			unlikely(likely(tok == ')') ? (yield() < 0) : WARN(W_EXPECTED_RPAREN_AFTER_ARGLIST))
		goto err_decl;
	} else if (!allow_missing_params) {
		if (WARN(W_DEPRECATED_NO_PARAMETER_LIST))
			goto err_decl;
	}
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
	if (tok == ':') {
		struct decl_ast *return_type;
		if
			unlikely(yield() < 0)
		goto err_decl;
		/* Function return type information. */
		ASSERT(!my_decl.da_func.f_ret);
		return_type = (struct decl_ast *)Dee_Malloc(sizeof(struct decl_ast));
		if
			unlikely(!return_type)
		goto err_decl;
		if
			unlikely(decl_ast_parse(return_type))
		{
			Dee_Free(return_type);
			goto err_decl;
		}
		my_decl.da_func.f_ret = return_type; /* Inherit */
	}
	/* Copy declaration information into the function symbol (if it exists) */
	if (funcself_symbol) {
		if (funcself_symbol->s_decltype.da_type != DAST_NONE &&
		    !decl_ast_equal(&funcself_symbol->s_decltype, &my_decl)) {
			decl_ast_fini(&my_decl);
			if (WARN(W_SYMBOL_TYPE_DECLARATION_CHANGED, funcself_symbol))
				goto err;
		} else {
			decl_ast_move(&funcself_symbol->s_decltype, &my_decl);
		}
		my_decl.da_type = DAST_NONE;
	}
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
	if (tok == TOK_ARROW) {
		struct ast_loc arrow_loc;
		loc_here(&arrow_loc);
		if
			unlikely(yield() < 0)
		goto err_decl;
		/* Expression function. */
		code = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if
			unlikely(!code)
		goto err_decl;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= CODE_FYIELDING, ast_yield(code))
		         : (current_basescope->bs_cflags |= BASESCOPE_FRETURN, ast_return(code));
		ast_decref(code);
		code = ast_setddi(result, &arrow_loc);
		if (pneed_semi)
			*pneed_semi = true;
	} else if (tok == '{') {
		struct ast_loc brace_loc;
		loc_here(&brace_loc);
		old_flags = TPPLexer_Current->l_flags;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
		if
			unlikely(yield() < 0)
		goto err_flags_decl;
		code = ast_putddi(ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, '}'), &brace_loc);
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if
			unlikely(likely(tok == '}') ? (yield() < 0) : WARN(W_EXPECTED_RBRACE_AFTER_FUNCTION))
		goto err_xcode_decl;
		if (pneed_semi)
			*pneed_semi = false;
	} else {
		/* Missing function body (this was allowed in deemon 100+, where
		 * this was interpreted the same way an `{ }'-like empty body would
		 * have been)
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
		 */
		if (WARN(W_EXPECTED_LBRACE_AFTER_FUNCTION))
			goto err;
		code = ast_multiple(AST_FMULTIPLE_KEEPLAST, 0, NULL);
		if (pneed_semi)
			*pneed_semi = true;
	}
	if
		unlikely(!code)
	goto err_decl;
	result = ast_function(code, current_basescope);
	ast_decref(code);
	if
		unlikely(!result)
	goto err_decl;
	/* Hack: The function AST itself must be located in the caller's scope. */
	Dee_Decref(result->a_scope);
	ASSERT(current_basescope);
	ASSERT(current_basescope->bs_scope.s_prev);
	result->a_scope = current_basescope->bs_scope.s_prev;
	Dee_Incref(result->a_scope);
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
	ASSERT(!funcself_symbol || my_decl.da_type == DAST_NONE);
	if (decl) {
		/* Pass Declaration information to the caller. */
		if (funcself_symbol) {
			if
				unlikely(decl_ast_copy(decl, &funcself_symbol->s_decltype))
			Dee_Clear(result);
		} else {
			decl_ast_move(decl, &my_decl);
		}
	} else {
		decl_ast_fini(&my_decl);
	}
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
	return ast_setddi(result, name_loc);
err_flags_decl:
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
	decl_ast_fini(&my_decl);
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
	/*err_flags:*/
	TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	goto err;
err_xcode_decl:
	ast_xdecref(code);
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
err_decl:
	decl_ast_fini(&my_decl);
	goto err;
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
err:
	return NULL;
}

INTERN DREF struct ast *DCALL
ast_parse_function_noscope_noargs(bool *pneed_semi) {
	uint32_t old_flags;
	DREF struct ast *result, *code;
	if (tok == TOK_ARROW) {
		struct ast_loc arrow_loc;
		loc_here(&arrow_loc);
		if
			unlikely(yield() < 0)
		goto err;
		/* Expression function. */
		code = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if
			unlikely(!code)
		goto err;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= CODE_FYIELDING, ast_yield(code))
		         : (current_basescope->bs_cflags |= BASESCOPE_FRETURN, ast_return(code));
		ast_decref(code);
		code = ast_setddi(result, &arrow_loc);
		if (pneed_semi)
			*pneed_semi = true;
	} else if (tok == '{') {
		struct ast_loc brace_loc;
		loc_here(&brace_loc);
		old_flags = TPPLexer_Current->l_flags;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
		if
			unlikely(yield() < 0)
		goto err_flags;
		code = ast_putddi(ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, '}'), &brace_loc);
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if
			unlikely(likely(tok == '}') ? (yield() < 0) : WARN(W_EXPECTED_RBRACE_AFTER_FUNCTION))
		goto err_xcode;
		if (pneed_semi)
			*pneed_semi = false;
	} else {
		/* Missing function body (this was allowed in deemon 100+, where
		 * this was interpreted the same way an `{ }'-like empty body would
		 * have been) */
		if (WARN(W_EXPECTED_LBRACE_AFTER_FUNCTION))
			goto err;
		code = ast_multiple(AST_FMULTIPLE_KEEPLAST, 0, NULL);
		if (pneed_semi)
			*pneed_semi = true;
	}
	if
		unlikely(!code)
	goto err;
	result = ast_function(code, current_basescope);
	ast_decref(code);
	if
		unlikely(!result)
	goto err;
	/* Hack: The function AST itself must be located in the caller's scope. */
	Dee_Decref(result->a_scope);
	ASSERT(current_basescope);
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


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C */
