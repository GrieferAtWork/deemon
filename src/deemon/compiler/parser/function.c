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
#ifndef GUARD_DEEMON_COMPILER_PARSER_FUNCTION_C
#define GUARD_DEEMON_COMPILER_PARSER_FUNCTION_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* Dee_*alloc*, Dee_CollectMemoryc, Dee_Free */
#include <deemon/code.h>              /* Dee_CODE_F* */
#include <deemon/compiler/ast.h>      /* AST_*, ast, ast_* */
#include <deemon/compiler/lexer.h>    /* DeeLexer, DeeLexer_*, TPP_*, ast_loc, tpp_* */
#include <deemon/compiler/optimize.h> /* ast_optimize_all */
#include <deemon/compiler/parser.h>   /* AST_PARSE_WASEXPR_NO, ast_annotation*, ast_parse_*, current_tags */
#include <deemon/compiler/symbol.h>   /* BASESCOPE_FRETURN, DAST_NONE, DeeScopeObject, LOOKUP_SYM_NORMAL, SYMBOL_F*, SYMBOL_TYPE_*, basescope_pop, basescope_push, current_basescope, current_scope, decl_ast*, has_local_symbol, new_local_symbol, new_unnamed_symbol, symbol */
#include <deemon/none.h>              /* DeeNone_NewRef */
#include <deemon/object.h>            /* DREF, DeeObject, Dee_Clear, Dee_Decref, Dee_Incref */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER

INTERN NONNULL((1)) void DCALL
Dee_ast_function_builder_fini(struct Dee_ast_function_builder *__restrict self) {
	ASSERT(self->afb_argc_min <= self->afb_argc_max);
	ASSERT(self->afb_argc_max <= self->afb_argc_alc);
	if (self->afb_func) {
		size_t i;
		for (i = 0; i < self->afb_argc_max; ++i) {
			struct Dee_ast_function_param *this_param;
			this_param = &DeeAst_GetFunctionParams(self->afb_func)[i];
			Dee_ast_function_param_fini(this_param);
		}
		if (self->afb_hasreturn)
			Dee_ast_typedef_fini(DeeAst_GetFunctionReturnTypedef(self->afb_func));
		DeeAst_FreeUninitializedFunction(self->afb_func);
	} else {
		ASSERT(self->afb_argc_min == 0);
		ASSERT(self->afb_argc_max == 0);
		ASSERT(self->afb_argc_alc == 0);
	}
	Dee_ast_function_param_base_xdestroy(self->afb_varargs);
	Dee_ast_function_param_base_xdestroy(self->afb_varkwds);
}


/* Package a function builder into a proper AST. The caller should still add
 * location info to the returned AST, as it comes without any by default. */
INTERN WUNUSED NONNULL((1, 3)) DeeAst *DCALL
Dee_ast_function_builder_pack(/*inherit(always)*/ struct Dee_ast_function_builder *__restrict self,
                              tpp_keyword const *opt_name, /*inherit(always)*/ DeeAst *__restrict body) {
	DeeAst *result = self->afb_func;
	ASSERT(self->afb_argc_min <= self->afb_argc_max);
	ASSERT(self->afb_argc_max <= self->afb_argc_alc);
	if (result == NULL) {
		ASSERT(self->afb_argc_min == 0);
		ASSERT(self->afb_argc_max == 0);
		ASSERT(self->afb_argc_alc == 0);
		result = DeeAst_MallocUninitializedFunction(0);
		if unlikely(!result)
			goto err_self_body;
	}

	/* If a return-typedef expression was never parsed, initialize it as empty now. */
	if (!self->afb_hasreturn)
		Dee_ast_typedef_init(DeeAst_GetFunctionReturnTypedef(result));

	/* Package everything */
	result = DeeAst_PackFunction(self->afb_func,
	                             self->afb_flag,
	                             opt_name, body,
	                             self->afb_varargs,
	                             self->afb_varkwds,
	                             self->afb_argc_min,
	                             self->afb_argc_max);
	DBG_memset(self, 0xcc, sizeof(*self));
	return result;
err_self_body:
	Dee_ast_function_builder_fini(self);
	DeeAst_Destroy(body);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeParser_ParseFunctionParam_VarArgs(DeeParser *__restrict self,
                                     struct Dee_ast_function_builder *__restrict builder,
                                     Dee_parser_functionparam_flag flag,
                                     DeeAstLoc const *__restrict argument_loc,
                                     /*0..1*/ tpp_keyword const *argument_name) {
	struct Dee_ast_function_param_base *varargs;
	if ((varargs = builder->afb_varargs) != NULL) {
		if (DeeParser_WarnfLoc(self, argument_loc, TPP_W_VARARGS_PARAMETER_ALREADY_DEFINED, varargs))
			goto err;
		Dee_ast_function_param_base_fini(varargs);
		builder->afb_varargs = NULL;
	} else {
		varargs = Dee_ast_function_param_base_alloc();
		if unlikely(!varargs)
			goto err;
	}
	if (TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err_varargs;
	if (DeeParser_GetTok(self) == ':') {
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_varargs;
		if (DeeParser_ParseTypeDef(self, &varargs->afpb_typedef))
			goto err_varargs;
	} else {
		Dee_ast_typedef_init(&varargs->afpb_typedef);
	}
	varargs->afpb_loc    = *argument_loc;
	varargs->afpb_name   = argument_name;
	varargs->afpb_flag   = flag & (Dee_AST_SYMREF_BYNAME_FLAG_FINAL | Dee_AST_SYMREF_BYNAME_FLAG_VARYING);
	varargs->afpb_sym    = NULL;
	builder->afb_varargs = varargs;
	return 0;
err_varargs:
	Dee_ast_function_param_base_free(varargs);
err:
	return -1;
}

/* Parse a function parameter;
 * >> function(a: int = 42) { ... }
 * >> function(b?: int) { ... }
 * >> function(varargs...: {Object...}) { ... }
 * >> function(...: {Object...}) { ... }
 * >> function(**varkwds: {string: Object}) { ... }
 *
 * - In all examples, enter after `function(` and exit *on* `)`
 * - Only parses at most 1 parameter.
 * - Safely handles cases where `pack` causes a lack of a closing `)`
 *
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DFCALL
DeeParser_ParseFunctionParam(DeeParser *__restrict self,
                             struct Dee_ast_function_builder *__restrict builder,
                             Dee_parser_functionparam_flag flag) {
	DeeAstLoc argument_loc;
	tpp_keyword const *argument_name;
	struct Dee_ast_function_param *this_param;
	STATIC_ASSERT(Dee_PARSER_FUNCTIONPARAM_FLAG_GOTLOCAL != Dee_PARSER_FUNCTIONPARAM_FLAG_GOTFINAL);
	STATIC_ASSERT(Dee_PARSER_FUNCTIONPARAM_FLAG_GOTLOCAL != Dee_PARSER_FUNCTIONPARAM_FLAG_GOTVARYING);
	STATIC_ASSERT(Dee_PARSER_FUNCTIONPARAM_FLAG_GOTFINAL == Dee_AST_SYMREF_BYNAME_FLAG_FINAL);
	STATIC_ASSERT(Dee_PARSER_FUNCTIONPARAM_FLAG_GOTVARYING == Dee_AST_SYMREF_BYNAME_FLAG_VARYING);

	/* Parse (additional) modifiers */
again_modifier:
	switch (DeeParser_GetTok(self)) {
		Dee_parser_functionparam_flag modifier_flag;
		__IF0 { case TPP_KWD_final: modifier_flag = Dee_PARSER_FUNCTIONPARAM_FLAG_GOTFINAL; }
		__IF0 { case TPP_KWD_varying: modifier_flag = Dee_PARSER_FUNCTIONPARAM_FLAG_GOTVARYING; }
		__IF0 { case TPP_KWD_local: modifier_flag = Dee_PARSER_FUNCTIONPARAM_FLAG_GOTLOCAL; }
		if ((flag & modifier_flag) != 0) {
			/* Modifier already present */
			if (DeeParser_Warnf(self, TPP_W_VARIABLE_MODIFIER_DUPLICATED))
				goto err;
		}
		flag |= modifier_flag;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		goto again_modifier;

	case TPP_TOK_STAR_STAR: {
		/* Special case: varkwds argument */
		struct Dee_ast_function_param_base *varkwds;
		if ((varkwds = builder->afb_varkwds) != NULL) {
			if (DeeParser_Warnf(self, TPP_W_VARKWDS_PARAMETER_ALREADY_DEFINED, varkwds))
				goto err;
			Dee_ast_function_param_base_fini(varkwds);
			builder->afb_varkwds = NULL;
		} else {
			varkwds = Dee_ast_function_param_base_alloc();
			if unlikely(!varkwds)
				goto err;
		}
		if (DeeParser_GetLoc(self, &varkwds->afpb_loc)) {
err_varkwds:
			Dee_ast_function_param_base_free(varkwds);
			goto err;
		}
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_varkwds;
		if (!DeeParser_HasTokenKwd(self)) {
			if (DeeParser_Warnf(self, TPP_W_EXPECTED_KEYWORD_FOR_PARAMETER_NAME))
				goto err_varkwds;
			varkwds->afpb_name = NULL;
		} else {
			if (DeeParser_GetLoc(self, &varkwds->afpb_loc))
				goto err_varkwds;
			varkwds->afpb_name = DeeParser_GetTokenKwd(self);
			if (tpp_keyword_getid(varkwds->afpb_name) == TPP_KWD_none) {
				varkwds->afpb_name = NULL;
			} else if (DeeParser_IsIdentifier(self, varkwds->afpb_name)) {
				if (DeeParser_Warnf(self, TPP_W_RESERVED_PARAMETER_NAME,
				                    tpp_keyword_getcstr(varkwds->afpb_name)))
					goto err_varkwds;
			}
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_varkwds;
		}
		if (DeeParser_GetTok(self) == ':') {
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_varkwds;
			if (DeeParser_ParseTypeDef(self, &varkwds->afpb_typedef))
				goto err_varkwds;
		} else {
			Dee_ast_typedef_init(&varkwds->afpb_typedef);
		}
		varkwds->afpb_flag = flag & (Dee_AST_SYMREF_BYNAME_FLAG_FINAL | Dee_AST_SYMREF_BYNAME_FLAG_VARYING);
		varkwds->afpb_sym = NULL;
		builder->afb_varkwds = varkwds;
		return 0;
	}	break;

	case TPP_TOK_DOT_DOT_DOT:
		/* Special case: anonymous varargs argument */
		if (DeeParser_GetLoc(self, &argument_loc)) 
			goto err;
		return DeeParser_ParseFunctionParam_VarArgs(self, builder, flag,
		                                            &argument_loc, NULL);

	default: break;
	}

	/* After modifiers, there should be the argument name (which may be `none` if unnamed) */
	if (DeeParser_GetLoc(self, &argument_loc))
		goto err;
	if (!DeeParser_HasTokenKwd(self)) {
		if (DeeParser_Warnf(self, TPP_W_EXPECTED_KEYWORD_FOR_PARAMETER_NAME))
			goto err;
		argument_name = NULL;
	} else {
		argument_name = DeeParser_GetTokenKwd(self);
		if (tpp_keyword_getid(argument_name) == TPP_KWD_none) {
			argument_name = NULL;
		} else if (DeeParser_IsIdentifier(self, argument_name)) {
			if (DeeParser_Warnf(self, TPP_W_RESERVED_PARAMETER_NAME,
			                    tpp_keyword_getcstr(argument_name)))
				goto err;
		}
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
	}

	/* After the argument's name, there should be one of:
	 * - `?`: Optional argument
	 * - `...`: (named) varargs argument
	 * - `=`: Argument with default value
	 * - `:`: Argument typedef
	 * - ...: Anything else means that it's a regular argument, and
	 *        the current token is no longer part of that argument.
	 *
	 * Deal with `...` first, since all other types of arguments must
	 * trigger warnings if used after a varargs/varkwds argument was
	 * already defined. */
	if (DeeParser_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
		return DeeParser_ParseFunctionParam_VarArgs(self, builder, flag,
		                                            &argument_loc,
		                                            argument_name);
	}

	/* Emit a warning if positional arguments are used after `...` or `**` */
	if (builder->afb_varargs || builder->afb_varkwds) {
		if (DeeParser_WarnfLoc(self, &argument_loc,
		                       TPP_W_POSITIONAL_PARAMETER_AFTER_VARARGS_OR_VARKWDS,
		                       builder->afb_varargs ? builder->afb_varargs : builder->afb_varkwds,
		                       argument_name))
			goto err;
	}

	/* Allocate space for the extra parameter. */
	ASSERT(builder->afb_argc_alc <= builder->afb_argc_max);
	if (builder->afb_argc_alc < builder->afb_argc_max) {
		DeeAst *new_function_ast;
		size_t new_argc_alc = builder->afb_argc_alc * 2;
		if (new_argc_alc < 3)
			new_argc_alc = 3;
		ASSERT(new_argc_alc > builder->afb_argc_max);
		new_function_ast = DeeAst_TryReallocUninitializedFunction(builder->afb_func, new_argc_alc);
		if unlikely(!new_function_ast) {
			new_argc_alc = builder->afb_argc_max + 1;
			new_function_ast = DeeAst_ReallocUninitializedFunction(builder->afb_func, new_argc_alc);
			if unlikely(!new_function_ast)
				goto err;
		}
		builder->afb_func = new_function_ast;
		builder->afb_argc_alc = new_argc_alc;
	}
	this_param = &DeeAst_GetFunctionParams(builder->afb_func)[builder->afb_argc_max];

	/* Initialize parameter (except for `afpb_typedef`) */
	this_param->afp_base.afpb_name = argument_name;
	this_param->afp_base.afpb_loc  = argument_loc;
	this_param->afp_base.afpb_flag = flag;
	this_param->afp_base.afpb_sym  = NULL;
	this_param->afp_default        = NULL;

	/* Parse parameter suffix modifiers: `:`, `?` and `=` */
	switch (DeeParser_GetTok(self)) {
	case '?':
		/* Optional parameter */
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetTok(self) == ':') {
			/* Type annotation can still appear after `?`-modifier */
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
			if (DeeParser_ParseTypeDef(self, &this_param->afp_base.afpb_typedef))
				goto err;
		} else {
			Dee_ast_typedef_init(&this_param->afp_base.afpb_typedef);
		}
		/* Fallthru to `++builder->afb_argc_max` -- by not changing
		 * `afb_argc_min`, the parameter will be registered as optional! */
		break;

	case ':':
		/* Parse parameter suffix modifiers: `:` */
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_ParseTypeDef(self, &this_param->afp_base.afpb_typedef))
			goto err;
		if (DeeParser_GetTok(self) == '=')
			goto parse_default_value; /* Default can still appear after `:`-typedef */
		goto setup_as_positional;

	case '=': {
		DeeAst *default_value;
		Dee_ast_typedef_init(&this_param->afp_base.afpb_typedef);
parse_default_value:
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_this_param_typedef;
		default_value = DeeParser_ParseExpr(self);
		if (!default_value)
			goto err_this_param_typedef;
		this_param->afp_default = default_value;
	}	break;

	default:
		Dee_ast_typedef_init(&this_param->afp_base.afpb_typedef);
setup_as_positional:
		if (builder->afb_argc_min == builder->afb_argc_max) {
			/* Increment min-argc to go hand-in-hand with `++builder->afb_argc_max` below */
			++builder->afb_argc_min;
		} else {
			/* Syntax error: non-optional argument after optional/default argument */
			struct Dee_ast_function_param *prev_param;
			ASSERT(builder->afb_argc_max >= 1);
			prev_param = &DeeAst_GetFunctionParams(builder->afb_func)[builder->afb_argc_max - 1];
			if (DeeParser_WarnfLoc(self, &this_param->afp_base.afpb_loc,
			                       TPP_W_POSITIONAL_PARAMETER_AFTER_OPTIONAL_OR_DEFAULT,
			                       prev_param, &this_param->afp_base))
				goto err_this_param_typedef;
		}
		break;
	}

	++builder->afb_argc_max;
	return 0;
err_this_param_typedef:
	Dee_ast_typedef_fini(&this_param->afp_base.afpb_typedef);
err:
	return -1;
}

/* Check if the current token is `:`, and if so: parse the return-type expression for `builder`
 * The caller must ensure that this function is called at most once for any `builder`.
 *
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DFCALL
DeeParser_MaybeParseFunctionReturnType(DeeParser *__restrict self,
                                       struct Dee_ast_function_builder *__restrict builder) {
	ASSERT(!builder->afb_hasreturn);
	if (DeeParser_GetTok(self) == ':') {
		DeeAst *function_ast;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		function_ast = builder->afb_func;
		if (function_ast == NULL) {
			function_ast = DeeAst_MallocUninitializedFunction(0);
			if unlikely(!function_ast)
				goto err;
			builder->afb_func = function_ast;
		}
		if (DeeParser_ParseTypeDef(self, DeeAst_GetFunctionReturnTypedef(function_ast)))
			goto err;
		builder->afb_hasreturn = true;
	}
	return 0;
err:
	return -1;
}


/* Parse a function declaration, starting at the `function` keyword.
 * @param: p_requires_semicolon: When non-NULL, set to `true` or `false` on success,
 *                               indicative of the function's body being `-> expr`,
 *                               as opposed to `{ expr; }` */
INTERN WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseFunction(DeeParser *__restrict self,
                        enum Dee_ast_symref_byname_kind decl_kind,
                        Dee_ast_symref_byname_flag decl_flags,
                        bool *p_requires_semicolon) {
	DeeAst *result;
	DeeAstLoc function_loc;
	DeeAstLoc function_name_loc;
	tpp_keyword const *function_name = NULL;
	ASSERT(DeeParser_GetTok(self) == TPP_KWD_function);
	if (DeeParser_GetLoc(self, &function_loc))
		goto err;
	if (TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err;
	function_name = NULL;
	function_name_loc = function_loc;
	if (DeeParser_HasTokenKwd(self)) {
		function_name = DeeParser_GetTokenKwd(self);
		if (tpp_keyword_getid(function_name) == TPP_KWD_none) {
			function_name = NULL;
		} else {
			if (DeeParser_GetLoc(self, &function_name_loc))
				goto err;
			if (DeeParser_IsIdentifier(self, function_name)) {
				if (DeeParser_Warnf(self, TPP_W_RESERVED_SYMBOL_NAME,
				                    tpp_keyword_getcstr(function_name)))
					goto err;
			}
		}
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
	} else if (decl_flags != Dee_AST_SYMREF_BYNAME_FLAG_NORMAL ||
	           decl_kind != Dee_AST_SYMREF_BYNAME_KIND_DECL) {
		/* Warn about something like `final local function(a, b, c) {}`
		 * (iow: missing function name when modifiers are used) */
		/* TODO */
	}
	result = DeeParser_ParseFunction2(self, function_name, p_requires_semicolon);
	if unlikely(!result)
		goto err;
	DeeAst_SetLoc(result, &function_loc);
	if (function_name) {
		/* Assign function to a symbol */
		DeeAst *function_name_ast;
		function_name_ast = DeeAst_NewSymRefByName(function_name, decl_kind, decl_flags);
		if unlikely(!function_name_ast)
			goto err_r;
		DeeAst_SetLoc(function_name_ast, &function_name_loc);
		result = DeeAst_NewAct2(Dee_AST_ACT2_INIT, function_name_ast, result);
		if unlikely(!result)
			goto err;
		DeeAst_SetLoc(result, &function_name_loc);
	}
	return result;
err_r:
	DeeAst_Destroy(result);
err:
	return NULL;
}

/* Similar to `DeeParser_ParseFunction()`, but (should) start at the `(` of the
 * function's parameter list. Additionally, the function isn't wrapped in a
 * `Dee_AST_ACT2_STORE` AST to assign it to a symbol bearing its name, or will
 * the returned function has location information yet. */
INTERN WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseFunction2(DeeParser *__restrict self,
                         tpp_keyword const *opt_function_name,
                         bool *p_requires_semicolon) {
	DeeAst *function_body;
	struct Dee_ast_function_builder builder;
	Dee_ast_function_builder_init(&builder);
	Dee_ast_function_builder_consume_attr(&builder, self);

	/* Parse parameter list */
	if (DeeParser_GetTok(self) == '(' ||
	    DeeParser_GetTok(self) == TPP_KWD_pack) {
		bool has_paren;
		if (DeeParser_ParenBegin(self, &has_paren))
			goto err_builder;
		DeeParser_NoLf_Push(self);
		for (;;) {
			if (DeeParser_ParseFunctionParam(self, &builder, Dee_PARSER_FUNCTIONPARAM_FLAG_NORMAL)) {
err_builder_nolf_break:
				DeeParser_NoLf_Break(self);
				goto err_builder;
			}
			if (DeeParser_GetTok(self) != ',')
				break;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_builder_nolf_break;
		}
		DeeParser_NoLf_Pop(self);
		if (DeeParser_ParenEnd(self, has_paren))
			goto err_builder;
	} else {
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_NO_PARAMETER_LIST))
			goto err_builder;
	}

	/* Initialize return typedef */
	if (DeeParser_MaybeParseFunctionReturnType(self, &builder))
		goto err_builder;
	if (DeeParser_GetTok(self) == '{') {
		if (p_requires_semicolon)
			*p_requires_semicolon = false;
		function_body = DeeParser_ParseStatementsAtBrace(self);
	} else {
		if (p_requires_semicolon)
			*p_requires_semicolon = true;
		if (DeeParser_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
			DeeAstLoc arrow_loc;
			if (DeeParser_GetLoc(self, &arrow_loc))
				goto err_builder;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_builder;
			function_body = DeeParser_ParseExpr(self);
			if unlikely(!function_body)
				goto err_builder;
			function_body = DeeAst_NewReturn(function_body);
			function_body = DeeAst_TrySetLoc(function_body, &arrow_loc);
		} else {
			if (DeeParser_Warnf(self, TPP_W_EXPECTED_LBRACE_AFTER_FUNCTION))
				goto err_builder;
			function_body = DeeAst_NewConst(DeeNone_NewRef());
		}
	}
	if unlikely(!function_body)
		goto err_builder;
	return Dee_ast_function_builder_pack(&builder, opt_function_name, function_body);
err_builder:
	Dee_ast_function_builder_fini(&builder);
err:
	return NULL;
}


#else /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

PRIVATE WUNUSED NONNULL((1)) int DFCALL
skip_argument_name(DeeLexer *self) {
	if unlikely(!DeeLexer_HasTokenKwd(self)) {
		if (DeeLexer_Warnf(self, TPP_W_EXPECTED_KEYWORD_FOR_ARGUMENT_NAME))
			goto err;
	} else {
		if (DeeLexer_GetTok(self) != TPP_KWD_none) {
			if (has_local_symbol(DeeLexer_GetTokenKwd(self))) {
				if (DeeLexer_Warnf(self, TPP_W_ARGUMENT_NAME_ALREADY_IN_USE))
					goto err;
			} else if (DeeLexer_IsIdentifier(self, DeeLexer_GetTokenKwd(self))) {
				if (DeeLexer_Warnf(self, TPP_W_RESERVED_ARGUMENT_NAME,
				                   DeeLexer_GetTokenKwdCStr(self)))
					goto err;
			}
		}
		if unlikely(DeeLexer_Yield(self) < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) struct symbol *DCALL
parse_argument_name(DeeLexer *self) {
	struct symbol *result;
	tpp_keyword const *argument_name;
	if unlikely(!DeeLexer_HasTokenKwd(self)) {
		if (DeeLexer_Warnf(self, TPP_W_EXPECTED_KEYWORD_FOR_ARGUMENT_NAME))
			goto err;
		result = new_unnamed_symbol();
	} else {
		if (DeeLexer_GetTok(self) == TPP_KWD_none) {
			/* Special case: Allow `none` to be used for placeholder/pending arguments. */
create_anon_argument:
			/* Create a new symbol for the argument. */
			result = new_unnamed_symbol();
			if unlikely(!result)
				goto err;
			if (DeeLexer_GetLoc(self, &result->s_decl))
				goto err;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
			if (result->s_decl.l_file)
				TPPFile_Incref(result->s_decl.l_file);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		} else {
			argument_name = DeeLexer_GetTokenKwd(self);
			if (has_local_symbol(argument_name)) {
				if (DeeLexer_Warnf(self, TPP_W_ARGUMENT_NAME_ALREADY_IN_USE))
					goto err;
				goto create_anon_argument;
			}

			/* Check if the argument name is a reserved identifier. */
			if (DeeLexer_IsIdentifier(self, argument_name)) {
				if (DeeLexer_Warnf(self, TPP_W_RESERVED_ARGUMENT_NAME,
				                   tpp_keyword_getcstr(argument_name)))
					goto err;
			}

			/* Create a new symbol for the argument. */
			result = new_local_symbol(self, argument_name, NULL);
		}
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
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
INTERN WUNUSED NONNULL((1)) int DFCALL
parse_arglist(DeeLexer *self) {
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
	if (DeeLexer_GetTok(self) != TPP_TOK_EOF &&
	    DeeLexer_GetTok(self) != ')') {
		for (;;) {
			uint16_t symbol_flags;

			/* Special case: unnamed varargs. */
			if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
				if unlikely(current_basescope->bs_flags & Dee_CODE_FVARARGS) {
					arg = current_basescope->bs_varargs;
					if likely(arg) {
						if (DeeLexer_Warnf(self, TPP_W_VARIABLE_ARGUMENT_ALREADY_DEFINED, arg))
							goto err;
						if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
							goto err;
						goto parse_varargs_suffix;
					}
				}
				ASSERT(!current_basescope->bs_varargs);
				arg = new_unnamed_symbol();
				if unlikely(!arg)
					goto err;
				if (DeeLexer_GetLoc(self, &arg->s_decl))
					goto err;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
				if (arg->s_decl.l_file)
					TPPFile_Incref(arg->s_decl.l_file);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
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
				current_basescope->bs_flags |= Dee_CODE_FVARARGS;
parse_varargs_suffix:
				if unlikely(DeeLexer_GetTok(self) == '?') {
					if (DeeLexer_Warnf(self, TPP_W_UNEXPECTED_OPTIONAL_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;
				}
				if unlikely(DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
					if (DeeLexer_Warnf(self, TPP_W_UNEXPECTED_DOTS_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;
				}
				if (DeeLexer_GetTok(self) == ':') {
					/* Parse argument declaration information. */
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;
					if unlikely(decl_ast_parse_for_symbol(self, arg))
						goto err;
				}
				if unlikely(DeeLexer_GetTok(self) == '=') {
					if (DeeLexer_Warnf(self, TPP_W_UNEXPECTED_DEFAULT_AFTER_VARARGS_OR_VARKWDS, arg))
						goto err;
					goto skip_default_suffix;
				}
				goto next_argument;
			}

			/* Parse variable modifier flags. */
			symbol_flags = SYMBOL_FNORMAL;
			for (;;) {
				if (DeeLexer_GetTok(self) == TPP_KWD_local) {
					/* ... */
				} else if (DeeLexer_GetTok(self) == TPP_KWD_final) {
					if ((symbol_flags & SYMBOL_FFINAL)) {
						if (DeeLexer_Warnf(self, TPP_W_VARIABLE_MODIFIER_DUPLICATED))
							goto err;
					}
					symbol_flags |= SYMBOL_FFINAL;
				} else if (DeeLexer_GetTok(self) == TPP_KWD_varying) {
					if ((symbol_flags & SYMBOL_FVARYING)) {
						if (DeeLexer_Warnf(self, TPP_W_VARIABLE_MODIFIER_DUPLICATED))
							goto err;
					}
					symbol_flags |= SYMBOL_FVARYING;
				} else {
					break;
				}
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
			}

			/* Check for keyword arguments parameter. */
			if (DeeLexer_GetTok(self) == TPP_TOK_STAR_STAR) {
				if unlikely(current_basescope->bs_flags & Dee_CODE_FVARKWDS) {
					arg = current_basescope->bs_varkwds;
					if likely(arg) {
						if (DeeLexer_Warnf(self, TPP_W_KEYWORD_ARGUMENT_ALREADY_DEFINED, arg))
							goto err;
						if (skip_argument_name(self))
							goto err;
						goto parse_varargs_suffix;
					}
				}
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;

				/* Parse the argument name. */
				arg = parse_argument_name(self);
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
				current_basescope->bs_flags |= Dee_CODE_FVARKWDS;
				goto parse_varargs_suffix;
			}

			/* Parse the argument name. */
			arg = parse_argument_name(self);
			if unlikely(!arg)
				goto err;
			if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
				/* Varargs argument. */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if likely(!current_basescope->bs_varargs) {
					arg->s_flag = SYMBOL_FALLOC | symbol_flags;
					goto set_arg_as_varargs_argument;
				}
				ASSERT(current_basescope->bs_flags & Dee_CODE_FVARARGS);
				if (DeeLexer_Warnf(self, TPP_W_VARIABLE_ARGUMENT_ALREADY_DEFINED, arg))
					goto err;
				arg->s_type = SYMBOL_TYPE_LOCAL;
				arg->s_flag = symbol_flags;
				goto parse_varargs_suffix;
			} else if (current_basescope->bs_varkwds || current_basescope->bs_varargs) {
				if (current_basescope->bs_varkwds
				    ? DeeLexer_Warnf(self, TPP_W_POSITIONAL_ARGUMENT_AFTER_VARKWDS,
				                     arg, current_basescope->bs_varkwds)
				    : DeeLexer_Warnf(self, TPP_W_POSITIONAL_ARGUMENT_AFTER_VARARGS,
				                     arg, current_basescope->bs_varargs))
					goto err;
set_argument_as_local:
				arg->s_type = SYMBOL_TYPE_LOCAL;
				arg->s_flag = symbol_flags;
				if (DeeLexer_GetTok(self) == '?' && TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if (DeeLexer_GetTok(self) == ':') {
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;
					if unlikely(decl_ast_parse_for_symbol(self, arg))
						goto err;
				}
				if (DeeLexer_GetTok(self) == '=')
					goto skip_default_suffix;
			} else if (DeeLexer_GetTok(self) == '?') { /* Optional argument */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
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
				if (DeeLexer_GetTok(self) == ':') {
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;
					if unlikely(decl_ast_parse_for_symbol(self, arg))
						goto err;
				}
				if unlikely(DeeLexer_GetTok(self) == '=') {
					DREF struct ast *default_expr;
					if (DeeLexer_Warnf(self, TPP_W_UNEXPECTED_DEFAULT_AFTER_OPTIONAL, arg))
						goto err;
skip_default_suffix:
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;

					/* Parse & discard the default expression. */
					default_expr = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
					if unlikely(!default_expr)
						goto err;
					ast_decref(default_expr);
					goto next_argument;
				}
			} else if (DeeLexer_GetTok(self) == ':') { /* Declaration suffix. */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if unlikely(decl_ast_parse_for_symbol(self, arg))
					goto err;
				if (DeeLexer_GetTok(self) == '=')
					goto parse_default_suffix;
				goto set_arg_as_normal;
			} else if (DeeLexer_GetTok(self) == '=') { /* Default argument */
				DREF DeeObject *default_value;
				DREF struct ast *default_expr;
parse_default_suffix:
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				default_expr = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
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
					if (DeeLexer_WarnfAst(self, default_expr,
					                      TPP_W_EXPECTED_CONSTANT_EXPRESSION_FOR_ARGUMENT_DEFAULT,
					                      arg))
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
					if (DeeLexer_Warnf(self, TPP_W_POSITIONAL_ARGUMENT_AFTER_OPTIONAL_OR_DEFAULT, arg))
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
			if (DeeLexer_GetTok(self) != ',')
				break;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (DeeLexer_GetTok(self) == ')')
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
	ASSERT((current_basescope->bs_varargs == NULL) || (current_basescope->bs_flags & Dee_CODE_FVARARGS));
	ASSERT((current_basescope->bs_varkwds == NULL) || (current_basescope->bs_flags & Dee_CODE_FVARKWDS));
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

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function(DeeLexer *self, tpp_keyword const *name, bool *p_need_semi,
                   bool allow_missing_params, struct ast_loc *name_loc,
                   struct decl_ast *decl, /*[0..1]*/ struct symbol *function_symbol) {
	DREF struct ast *result;
	struct ast_annotations annotations;
	ast_annotations_get(&annotations);
	if unlikely(basescope_push())
		goto err_anno;
	current_basescope->bs_flags |= current_tags.at_code_flags;
	result = ast_parse_function_noscope(self, name, p_need_semi, allow_missing_params,
	                                    name_loc, decl, function_symbol);
	basescope_pop();
	if unlikely(!result)
		goto err_anno;
	return ast_annotations_apply(&annotations, result);
err_anno:
	ast_annotations_free(&annotations);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function_noscope(DeeLexer *self, tpp_keyword const *name, bool *p_need_semi,
                           bool allow_missing_params, struct ast_loc *name_loc,
                           struct decl_ast *decl, /*[0..1]*/ struct symbol *function_symbol) {
	struct decl_ast my_decl;
	struct symbol *funcself_symbol = NULL;
	DREF struct ast *result, *code;
	/* Add information from tags. */
	if (name) {
		/* Save the function name in the base scope. */
		current_basescope->bs_name = name;

		/* Create a new symbol to allow for function-self-referencing. */
		funcself_symbol = new_local_symbol(self, name, name_loc);
		if unlikely(!funcself_symbol)
			goto err;
		funcself_symbol->s_type = SYMBOL_TYPE_MYFUNC;
	}

	/* Declaration meta-information */
	decl_ast_initfunc(&my_decl, NULL, current_basescope);

	if (DeeLexer_GetTok(self) == '(') {
		/* Argument list. */
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_decl_lparen_flags:
			DeeLexer_NoLf_Break(self);
			goto err_decl;
		}
		if unlikely(parse_arglist(self))
			goto err_decl_lparen_flags;
		DeeLexer_NoLf_Pop(self);
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_ARGLIST))
			goto err_decl;
	} else if (!allow_missing_params) {
		if (DeeLexer_Warnf(self, TPP_W_DEPRECATED_NO_PARAMETER_LIST))
			goto err_decl;
	}

	if (DeeLexer_GetTok(self) == ':') {
		struct decl_ast *return_type;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_decl;
		/* Function return type information. */
		ASSERT(!my_decl.da_func.f_ret);
		return_type = (struct decl_ast *)Dee_Malloc(sizeof(struct decl_ast));
		if unlikely(!return_type)
			goto err_decl;
		if unlikely(decl_ast_parse(self, return_type)) {
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
				if (DeeLexer_Warnf(self, TPP_W_SYMBOL_TYPE_DECLARATION_CHANGED, funcself_symbol))
					goto err;
			}
		} else {
			decl_ast_move(&funcself_symbol->s_decltype, &my_decl);
		}
		my_decl.da_type = DAST_NONE;
	}

	if (DeeLexer_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
		struct ast_loc arrow_loc;
		if (DeeLexer_GetLoc(self, &arrow_loc))
			goto err_decl;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_decl;

		/* Expression function. */
		code = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!code)
			goto err_decl;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= Dee_CODE_FYIELDING, ast_yield(code))
		         : (current_basescope->bs_cflags |= BASESCOPE_FRETURN, ast_return(code));
		ast_decref(code);
		code = ast_setddi(result, &arrow_loc);
		if (p_need_semi)
			*p_need_semi = true;
	} else if (DeeLexer_GetTok(self) == '{') {
		struct ast_loc brace_loc;
		if (DeeLexer_GetLoc(self, &brace_loc))
			goto err_decl;
		DeeLexer_EnableLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
/*err_decl_lbrace_flags:*/
			DeeLexer_EnableLf_Break(self);
			goto err_decl;
		}
		code = ast_parse_statements_until(self, AST_FMULTIPLE_KEEPLAST, TPP_TOK_OFCHAR('}'));
		code = ast_putddi(code, &brace_loc);
		DeeLexer_EnableLf_Pop(self);
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_FUNCTION))
			goto err_decl_xcode;
		if (p_need_semi)
			*p_need_semi = false;
	} else {
		/* Missing function body (this was allowed in deemon 100+, where
		 * this was interpreted the same way an `{ }`-like empty body would
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
		 * >>         // Self-reference via reference (`x` already had
		 * >>         // a value when the function was assigned, thus
		 * >>         // allowing that value to be referenced like any
		 * >>         // other referenced variable)
		 * >>         x(n + 1);
		 * >>     }
		 * >> }
		 * >> x(0);
		 *
		 * But that's no longer allowed since functions don't implement the
		 * `operator assign` anymore (they are immutable once created), and
		 * self-referencing functions are done by ASM_THIS_FUNCTION which
		 * will push the current function onto the stack (thus allowing a
		 * function to reference itself)
		 *
		 * Still: we *do* parse functions without a body for the sake of
		 *        syntax compatibility with deemon 100+.
		 */
		if (DeeLexer_Warnf(self, TPP_W_EXPECTED_LBRACE_AFTER_FUNCTION))
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
err_decl_xcode:
	ast_xdecref(code);
err_decl:
	decl_ast_fini(&my_decl);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function_noscope_noargs(DeeLexer *self, bool *p_need_semi) {
	DREF struct ast *result, *code;
	if (DeeLexer_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
		struct ast_loc arrow_loc;
		if (DeeLexer_GetLoc(self, &arrow_loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		/* Expression function. */
		code = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!code)
			goto err;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= Dee_CODE_FYIELDING, ast_yield(code))
		         : (current_basescope->bs_cflags |= BASESCOPE_FRETURN, ast_return(code));
		ast_decref(code);
		code = ast_setddi(result, &arrow_loc);
		if (p_need_semi)
			*p_need_semi = true;
	} else if (DeeLexer_GetTok(self) == '{') {
		struct ast_loc brace_loc;
		if (DeeLexer_GetLoc(self, &brace_loc))
			goto err;
		DeeLexer_EnableLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
/*err_lbrace_flags:*/
			DeeLexer_EnableLf_Break(self);
			goto err;
		}
		code = ast_parse_statements_until(self, AST_FMULTIPLE_KEEPLAST, TPP_TOK_OFCHAR('}'));
		code = ast_putddi(code, &brace_loc);
		DeeLexer_EnableLf_Pop(self);
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_FUNCTION))
			goto err_xcode;
		if (p_need_semi)
			*p_need_semi = false;
	} else {
		/* Missing function body (this was allowed in deemon 100+, where
		 * this was interpreted the same way an `{ }`-like empty body would
		 * have been) */
		if (DeeLexer_Warnf(self, TPP_W_EXPECTED_LBRACE_AFTER_FUNCTION))
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
err_xcode:
	ast_xdecref(code);
err:
	return NULL;
}



/* Parse a `() -> 42` or `a -> a+42`-style lambda.
 * In either case, upon entry the current token must be the '->' */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function_java_lambda(DeeLexer *self,
                               tpp_keyword const *first_argument_name,
                               struct ast_loc *first_argument_loc) {
	struct ast_loc arrow_loc;
	DREF struct ast *result, *code;
	if unlikely(basescope_push())
		goto err;
	if (first_argument_name) {
		struct symbol *arg;
		if (tpp_keyword_getid(first_argument_name) == TPP_KWD_none) {
			/* Create a new symbol for the argument. */
			arg = new_unnamed_symbol();
			if unlikely(!arg)
				goto err_scope;
			arg->s_decl = *first_argument_loc;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
			if (arg->s_decl.l_file)
				TPPFile_Incref(arg->s_decl.l_file);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		} else {
			/* Check if the argument name is a reserved identifier. */
			if (DeeLexer_IsIdentifier(self, first_argument_name)) {
				if (DeeLexer_Warnf(self, TPP_W_RESERVED_ARGUMENT_NAME,
				                   tpp_keyword_getcstr(first_argument_name)))
					goto err_scope;
			}

			/* Create a new symbol for the argument. */
			arg = new_local_symbol(self, first_argument_name, first_argument_loc);
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
	} else if (DeeLexer_GetTok(self) != TPP_TOK_MINUS_RANGLE && DeeLexer_GetTok(self) != ':') {
		int error;
		DeeLexer_NoLf_Push(self);
		error = parse_arglist(self);
		DeeLexer_NoLf_Pop(self);
		if unlikely(error)
			goto err_scope;
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_ARGLIST))
			goto err_scope;
	} else {
		/* No arguments */
		ASSERT(current_basescope->bs_argc_min == 0);
		ASSERT(current_basescope->bs_argc_max == 0);
		ASSERT(current_basescope->bs_argv == NULL);
	}

	if (DeeLexer_GetTok(self) == ':') {
		struct decl_ast temp;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_scope;
		if unlikely(decl_ast_parse(self, &temp))
			goto err_scope;
		decl_ast_fini(&temp);
	}

	ASSERT(DeeLexer_GetTok(self) == TPP_TOK_MINUS_RANGLE);
	if (DeeLexer_GetLoc(self, &arrow_loc))
		goto err;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;

	/* At this point, we're at the start of the lambda expression,
	 * or the '{' in case it uses statements (or returns a sequence) */
	if (DeeLexer_GetTok(self) == '{') {
		unsigned int was_expression;
		code = ast_parse_statement_or_braces(self, &was_expression);
		if (was_expression != AST_PARSE_WASEXPR_NO)
			goto wrap_code_with_return;
	} else {
		code = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
wrap_code_with_return:
		if unlikely(!code)
			goto err_scope;
		result = code->a_type == AST_EXPAND
		         ? (current_basescope->bs_flags |= Dee_CODE_FYIELDING, ast_yield(code))
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

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
/* Check if the parser is located after the '(' of a java-style lambda.
 * @return:  1: Yes
 * @return:  0: No
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DFCALL
ast_is_after_lparen_of_java_lambda_impl(DeeLexer *self) {
	if (!DeeLexer_HasTokenKwd(self)) {
		if (DeeLexer_GetTok(self) == TPP_TOK_STAR_STAR) {
			/* `**` can only appear in argument- and parameter lists.
			 * Since out parent is parsing an argument-list, with a
			 * comma-list as fallback, we know that this is varkwds! */
			goto yes;
		}
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			/* Special case: there are 4 cases where this can still be an argument list:
			 * >> (...) -> [...][0];           // anonymous varargs
			 * >> (...,) -> [...][0];          // *ditto*
			 * >> (..., **kwds) -> [...][0];   // anonymous varargs, followed by varkwds
			 * >> (..., **kwds,) -> [...][0];  // *ditto* */
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (DeeLexer_GetTok(self) == ',' && TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (DeeLexer_GetTok(self) == TPP_TOK_STAR_STAR) {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if (!DeeLexer_HasTokenKwd(self))
					goto nope;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if (DeeLexer_GetTok(self) == ',' && TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
			}
			goto check_and_consume_rparen;
		}
		goto nope;
	}

	/* Now try to skip the argument list. */
	for (;;) {
		/* Special case: unnamed varargs. */
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
		} else {
			/* Parse variable modifier flags. */
			while (DeeLexer_GetTok(self) == TPP_KWD_local ||
			       DeeLexer_GetTok(self) == TPP_KWD_final ||
			       DeeLexer_GetTok(self) == TPP_KWD_varying) {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
			}
			if (DeeLexer_GetTok(self) == TPP_TOK_STAR_STAR && TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (!DeeLexer_HasTokenKwd(self))
				goto nope;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) /* Argument name. */
				goto err;
			if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT && TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
		}
		if (DeeLexer_GetTok(self) == '?') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (DeeLexer_GetTok(self) == ',' || DeeLexer_GetTok(self) == ')')
				goto yes; /* Something like `(foo?)` is guarantied to be a paren-lambda. */
			if (DeeLexer_GetTok(self) == ':') {
				/* Parse argument declaration information. */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if unlikely(decl_ast_skip(self))
					goto err;
			}
		} else {
			if (DeeLexer_GetTok(self) == ':') /* Something like `(foo: int)` is guarantied to be a paren-lambda. */
				goto yes;
		}
		if unlikely(DeeLexer_GetTok(self) == '=') {
			struct ast *temp;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			/* Parse & discard the default expression. */
			temp = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!temp)
				goto err;
			ast_decref(temp);
		}
		if (DeeLexer_GetTok(self) != ',')
			break;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetTok(self) == ')')
			break;
	}
check_and_consume_rparen:
	if (DeeLexer_GetTok(self) != ')')
		goto nope;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;
	if (DeeLexer_GetTok(self) == ':') {
		/* Parse argument declaration information. */
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if unlikely(decl_ast_skip(self))
			goto err;
	}
	if (DeeLexer_GetTok(self) != TPP_TOK_MINUS_RANGLE)
		goto nope;
yes:
	return 1;
nope:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DFCALL
ast_is_after_lparen_of_java_lambda(DeeLexer *self) {
	int result;
	tpp_char const *after_lparen_ptr;
	tpp_file *const file = DeeLexer_GetFile(self);
	tpp_size rel_after_lparen_ptr;
	tpp_size saved_token_len;
	tpp_lexer_autopopfile_pushoff(&self->dl_lexer);
	tpp_lexer_pushfeatures(&self->dl_lexer);
	tpp_lexer_disablefeature(&self->dl_lexer, TPP_FEAT_CPP_DIRECTIVES);
	tpp_lexer_disablefeature(&self->dl_lexer, TPP_FEAT_CPP_MACROS);
	tpp_lexer_nowarnings_pushon(&self->dl_lexer);
	tpp_lexer_pushtokenstate(&self->dl_lexer);
	after_lparen_ptr = DeeLexer_GetTokenStart(self);
	saved_token_len  = DeeLexer_GetTokenLen(self);
	ASSERT(after_lparen_ptr == tpp_file_getlastpos(file));
	tpp_file_pushkeep(file, after_lparen_ptr);
	rel_after_lparen_ptr = tpp_file_keep_ptr2rel(file, after_lparen_ptr);

	result = ast_is_after_lparen_of_java_lambda_impl(self);

	after_lparen_ptr = tpp_file_keep_rel2ptr(file, rel_after_lparen_ptr);
	tpp_lexer_settokenrange(&self->dl_lexer, after_lparen_ptr,
	                        after_lparen_ptr + saved_token_len);
	tpp_file_popkeep(file);
	tpp_lexer_poptokenstate(&self->dl_lexer);
	tpp_lexer_nowarnings_pop(&self->dl_lexer);
	tpp_lexer_popfeatures(&self->dl_lexer);
	tpp_lexer_autopopfile_pop(&self->dl_lexer);
	return result;
}
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
/* Check if the parser is located after the '(' of a java-style lambda.
 * @return:  1: Yes
 * @return:  0: No
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DFCALL
ast_is_after_lparen_of_java_lambda(DeeLexer *self) {
	struct TPPLexerPosition pos;
	if (!DeeLexer_HasTokenKwd(self)) {
		if (DeeLexer_GetTok(self) == TPP_TOK_STAR_STAR) {
			/* `**` can only appear in argument- and parameter lists.
			 * Since out parent is parsing an argument-list, with a
			 * comma-list as fallback, we know that this is varkwds! */
			goto yes;
		}
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			/* Special case: there are 4 cases where this can still be an argument list:
			 * >> (...) -> [...][0];           // anonymous varargs
			 * >> (...,) -> [...][0];          // *ditto*
			 * >> (..., **kwds) -> [...][0];   // anonymous varargs, followed by varkwds
			 * >> (..., **kwds,) -> [...][0];  // *ditto* */
			if unlikely(!TPPLexer_SavePosition(&pos))
				goto err;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_restore;
			if (DeeLexer_GetTok(self) == ',' && TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_restore;
			if (DeeLexer_GetTok(self) == TPP_TOK_STAR_STAR) {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_restore;
				if (!DeeLexer_HasTokenKwd(self))
					goto nope_restore;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_restore;
				if (DeeLexer_GetTok(self) == ',' && TPP_TOK_ISERR(DeeLexer_Yield(self)))
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
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_restore;
		} else {
			/* Parse variable modifier flags. */
			while (DeeLexer_GetTok(self) == TPP_KWD_local || DeeLexer_GetTok(self) == TPP_KWD_final || DeeLexer_GetTok(self) == TPP_KWD_varying) {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_restore;
			}
			if (DeeLexer_GetTok(self) == TPP_TOK_STAR_STAR && TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_restore;
			if (!DeeLexer_HasTokenKwd(self))
				goto nope_restore;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) /* Argument name. */
				goto err_restore;
			if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT && TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_restore;
		}
		if (DeeLexer_GetTok(self) == '?') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_restore;
			if (DeeLexer_GetTok(self) == ',' || DeeLexer_GetTok(self) == ')')
				goto yes_restore; /* Something like `(foo?)` is guarantied to be a paren-lambda. */
			if (DeeLexer_GetTok(self) == ':') {
				/* Parse argument declaration information. */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_restore;
				if unlikely(decl_ast_skip(self))
					goto err_restore;
			}
		} else {
			if (DeeLexer_GetTok(self) == ':') /* Something like `(foo: int)` is guarantied to be a paren-lambda. */
				goto yes_restore;
		}
		if unlikely(DeeLexer_GetTok(self) == '=') {
			struct ast *temp;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_restore;
			/* Parse & discard the default expression. */
			temp = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!temp)
				goto err_restore;
			ast_decref(temp);
		}
		if (DeeLexer_GetTok(self) != ',')
			break;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_restore;
		if (DeeLexer_GetTok(self) == ')')
			break;
	}
check_and_consume_rparen:
	if (DeeLexer_GetTok(self) != ')')
		goto nope_restore;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err_restore;
	if (DeeLexer_GetTok(self) == ':') {
		/* Parse argument declaration information. */
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_restore;
		if unlikely(decl_ast_skip(self))
			goto err_restore;
	}
	if (DeeLexer_GetTok(self) != TPP_TOK_MINUS_RANGLE)
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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
#endif /* !CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_PARSER_FUNCTION_C */
