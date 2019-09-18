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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C 1
#define _KOS_SOURCE 1

#include <deemon/compiler/compiler.h>

#include <deemon/HashSet.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/tuple.h>
#include <deemon/util/cache.h>

#include <string.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

#ifndef __USE_KOS
#define strend(x) ((x) + strlen(x))
#endif /* !__USE_KOS */

#ifdef CONFIG_AST_IS_STRUCT
DECLARE_STRUCT_CACHE(ast, struct ast)
#else  /* CONFIG_AST_IS_STRUCT */
DECLARE_OBJECT_CACHE(ast, struct ast)
#endif /* !CONFIG_AST_IS_STRUCT */


INTERN int DCALL
compiler_init(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
	DeeObject *module       = Dee_None;
	struct keyword kwlist[] = { K(module), KEND };
	/* TODO: All those other arguments, like compiler options, etc. */
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|o", &module))
		goto err;
	if (DeeNone_Check(module)) {
		module = DeeModule_New(Dee_EmptyString);
		if
			unlikely(!module)
		goto err;
	} else if (DeeString_Check(module)) {
		module = DeeModule_New(module);
		if
			unlikely(!module)
		goto err;
	} else {
		Dee_Incref(module);
	}
	/* Create the new root scope object. */
	self->cp_scope = (DREF DeeScopeObject *)DeeObject_New(&DeeRootScope_Type, 1,
	                                                      (DeeObject **)&module);
	Dee_Decref(module);
	if
		unlikely(!self->cp_scope)
	goto err;
	weakref_support_init(self);
	memset(&self->cp_tags, 0, sizeof(self->cp_tags));
	memset(&self->cp_items, 0, sizeof(self->cp_items));
	self->cp_flags           = COMPILER_FNORMAL;
	self->cp_prev            = NULL;
	self->cp_recursion       = 0;
	self->cp_options         = NULL;
	self->cp_inner_options   = NULL;
	self->cp_parser_flags    = PARSE_FNORMAL;
	self->cp_optimizer_flags = OPTIMIZE_FNORMAL;
	self->cp_unwind_limit    = 0;
#ifndef CONFIG_LANGUAGE_NO_ASM
	self->cp_uasm_unique = 0;
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	if
		unlikely(!TPPLexer_Init(&self->cp_lexer))
	goto err_scope;
#ifdef _MSC_VER
	/* Mirror MSVC's file-and-line syntax. */
	self->cp_lexer.l_flags |= TPPLEXER_FLAG_MSVC_MESSAGEFORMAT;
#endif /* _MSC_VER */
	self->cp_lexer.l_extokens = TPPLEXER_TOKEN_LANG_DEEMON;
	parser_errors_init(&self->cp_errors);
	return 0;
err_scope:
	Dee_Decref(self->cp_scope);
err:
	return -1;
}


INTERN DREF DeeObject *DCALL
compiler_get_lexer(DeeCompilerObject *__restrict self) {
	return DeeCompiler_GetLexer(self);
}

INTERN DREF DeeObject *DCALL
compiler_get_parser(DeeCompilerObject *__restrict self) {
	return DeeCompiler_GetParser(self);
}

INTERN DREF DeeObject *DCALL
compiler_get_scope(DeeCompilerObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self);
	result = DeeCompiler_GetScope(current_scope);
	COMPILER_END();
	return result;
}

INTERN int DCALL
compiler_set_scope(DeeCompilerObject *__restrict self,
                   DeeCompilerScopeObject *__restrict value) {
	int result = 0;
	if (DeeObject_AssertType((DeeObject *)value, &DeeCompilerScope_Type))
		return -1;
	if (value->ci_compiler != self)
		return err_invalid_scope_compiler(value);
	COMPILER_BEGIN(self);
	if (value->ci_value->s_base != current_basescope) {
		result = err_different_base_scope();
	} else {
		Dee_Incref(value->ci_value);
		Dee_Decref(current_scope);
		current_scope = value->ci_value;
	}
	COMPILER_END();
	return result;
}

INTERN DREF DeeObject *DCALL
compiler_get_basescope(DeeCompilerObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self);
	result = DeeCompiler_GetScope((DeeScopeObject *)current_basescope);
	COMPILER_END();
	return result;
}

INTERN int DCALL
compiler_set_basescope(DeeCompilerObject *__restrict self,
                       DeeCompilerScopeObject *__restrict value) {
	int result = 0;
	if (DeeObject_AssertType((DeeObject *)value, &DeeCompilerBaseScope_Type))
		return -1;
	if (value->ci_compiler != self)
		return err_invalid_scope_compiler(value);
	COMPILER_BEGIN(self);
	if (((DeeBaseScopeObject *)value->ci_value)->bs_root != current_rootscope) {
		result = err_different_root_scope();
	} else {
		Dee_Incref(value->ci_value);
		Dee_Decref(current_scope);
		current_scope     = value->ci_value;
		current_basescope = (DeeBaseScopeObject *)value->ci_value;
	}
	COMPILER_END();
	return result;
}

INTERN DREF DeeObject *DCALL
compiler_get_rootscope(DeeCompilerObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self);
	result = DeeCompiler_GetObjItem(&DeeCompilerRootScope_Type,
	                                (DeeObject *)current_rootscope);
	COMPILER_END();
	return result;
}

INTERN DREF DeeObject *DCALL
compiler_get_module(DeeCompilerObject *__restrict self) {
	DREF DeeModuleObject *result;
	COMPILER_BEGIN(self);
	result = current_rootscope->rs_module;
	Dee_Incref(result);
	COMPILER_END();
	return (DREF DeeObject *)result;
}


INTERN struct type_getset compiler_getsets[] = {
	{ "lexer", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & compiler_get_lexer, NULL, NULL,
	  DOC("->?ALexer?Ert:Compiler\n"
	      "Returns the lexer (tokenizer) of @this compiler") },
	{ "parser", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & compiler_get_parser, NULL, NULL,
	  DOC("->?AParser?Ert:Compiler\n"
	      "Returns the parser (token to ast converter) of @this compiler") },
	{ "scope",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & compiler_get_scope, NULL,
	  (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict)) & compiler_set_scope,
	  DOC("->?AScope?Ert:Compiler\n"
	      "@throw ValueError Attempted to set a scope who's compiler doesn't match @this\n"
	      "@throw ReferenceError Attempted to set a scope not apart of the same base-scope as #basescope\n"
	      "Get or set the current scope used for parsing new #{ast}s") },
	{ "basescope",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & compiler_get_basescope, NULL,
	  (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict)) & compiler_set_basescope,
	  DOC("->?ABaseScope?Ert:Compiler\n"
	      "@throw ValueError Attempted to set a scope who's compiler doesn't match @this\n"
	      "@throw ReferenceError Attempted to set a scope not apart of the same root-scope as #rootscope\n"
	      "Get or set the current base-scope, representing the current function-context\n"
	      "When setting the base-scope, #scope is set to the same scope") },
	{ "rootscope",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & compiler_get_rootscope, NULL, NULL,
	  DOC("->?ARootScope?Ert:Compiler\n"
	      "Get the root-scope active within @this compiler\n"
	      "Note that this scope is fixed and cannot be changed") },
	{ DeeString_STR(&str_module),
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & compiler_get_module, NULL, NULL,
	  DOC("->?DModule\n"
	      "Returns the module being constructed by @this compiler\n"
	      "Warning: The returned module is incomplete and uninitialized, "
	      "and can't actually be used, yet") },
	{ NULL }
};


#ifndef NDEBUG
#define ast_new(scope, loc) ast_dbgnew(scope, loc, __FILE__, __LINE__)
PRIVATE DREF struct ast *DCALL
ast_dbgnew(DeeScopeObject *__restrict scope,
           DeeObject *loc, char const *file, int line)
#else /* !NDEBUG */
PRIVATE DREF struct ast *DCALL
ast_new(DeeScopeObject *__restrict scope, DeeObject *loc)
#endif /* NDEBUG */
{
	DREF struct ast *result;
#ifndef CONFIG_NO_THREADS
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
#endif /* !CONFIG_NO_THREADS */
#ifndef NDEBUG
	result = ast_dbgalloc(file, line);
#else /* !NDEBUG */
	result = ast_alloc();
#endif /* NDEBUG */
	if
		likely(result)
	{
		if
			unlikely(set_astloc_from_obj(loc, result))
		{
			ast_free(result);
			result = NULL;
		}
		else {
#ifdef CONFIG_AST_IS_STRUCT
			result->a_refcnt = 1;
#else /* CONFIG_AST_IS_STRUCT */
			DeeObject_Init(result, &DeeAst_Type);
#endif /* !CONFIG_AST_IS_STRUCT */
			result->a_scope      = scope;
			result->a_ddi.l_file = NULL;
			Dee_Incref(scope);
		}
	}
	return result;
}


PRIVATE DeeScopeObject *DCALL
get_scope(DeeCompilerScopeObject *scope) {
	if (DeeNone_Check(scope))
		return current_scope;
	if (DeeObject_AssertType((DeeObject *)scope, &DeeCompilerScope_Type))
		goto err;
	if
		unlikely(scope->ci_compiler != DeeCompiler_Current)
	{
		err_invalid_scope_compiler(scope);
		goto err;
	}
	return scope->ci_value;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
ast_makeconstexpr(DeeCompilerObject *__restrict self,
                  size_t argc, DeeObject **__restrict argv,
                  DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeObject *value;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	PRIVATE struct keyword kwlist[] = { K(value), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|oo:makeconstexpr", &value, &scope, &loc) ||
	    unlikely((ast_scope = get_scope(scope)) == NULL))
		goto done;
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type      = AST_CONSTEXPR;
	result_ast->a_constexpr = value;
	Dee_Incref(value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

PRIVATE struct keyword makesym_kwlist[] = { K(sym), K(scope), K(loc), KEND };

PRIVATE DREF DeeObject *DCALL
ast_makesym(DeeCompilerObject *__restrict self,
            size_t argc, DeeObject **__restrict argv,
            DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeCompilerSymbolObject *sym;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, makesym_kwlist, "o|oo:makesym", &sym, &scope, &loc))
		goto done;
	if (DeeObject_AssertTypeExact((DeeObject *)sym, &DeeCompilerSymbol_Type))
		goto done;
	if
		unlikely(sym->ci_compiler != self)
	{
		err_invalid_symbol_compiler(sym);
		goto done;
	}
	if
		unlikely(!sym->ci_value)
	{
		err_compiler_item_deleted((DeeCompilerItemObject *)sym);
		goto done;
	}
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(!scope_reaches_symbol(ast_scope, sym->ci_value))
	{
		err_symbol_not_reachable(ast_scope, sym->ci_value);
		goto done;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type = AST_SYM;
	result_ast->a_flag = AST_FNORMAL;
	result_ast->a_sym  = sym->ci_value;
	SYMBOL_INC_NREAD(sym->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makeunbind(DeeCompilerObject *__restrict self,
               size_t argc, DeeObject **__restrict argv,
               DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeCompilerSymbolObject *sym;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, makesym_kwlist, "o|oo:makeunbind", &sym, &scope, &loc))
		goto done;
	if (DeeObject_AssertTypeExact((DeeObject *)sym, &DeeCompilerSymbol_Type))
		goto done;
	if
		unlikely(sym->ci_compiler != self)
	{
		err_invalid_symbol_compiler(sym);
		goto done;
	}
	if
		unlikely(!sym->ci_value)
	{
		err_compiler_item_deleted((DeeCompilerItemObject *)sym);
		goto done;
	}
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(!scope_reaches_symbol(ast_scope, sym->ci_value))
	{
		err_symbol_not_reachable(ast_scope, sym->ci_value);
		goto done;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type   = AST_UNBIND;
	result_ast->a_unbind = sym->ci_value;
	SYMBOL_INC_NWRITE(sym->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makebound(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeCompilerSymbolObject *sym;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, makesym_kwlist, "o|oo:makebound", &sym, &scope, &loc))
		goto done;
	if (DeeObject_AssertTypeExact((DeeObject *)sym, &DeeCompilerSymbol_Type))
		goto done;
	if
		unlikely(sym->ci_compiler != self)
	{
		err_invalid_symbol_compiler(sym);
		goto done;
	}
	if
		unlikely(!sym->ci_value)
	{
		err_compiler_item_deleted((DeeCompilerItemObject *)sym);
		goto done;
	}
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(!scope_reaches_symbol(ast_scope, sym->ci_value))
	{
		err_symbol_not_reachable(ast_scope, sym->ci_value);
		goto done;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type   = AST_BOUND;
	result_ast->a_unbind = sym->ci_value;
	SYMBOL_INC_NBOUND(sym->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

INTERN uint16_t DCALL
get_ast_multiple_typing(DeeTypeObject *__restrict typing) {
	uint16_t result;
	if (DeeNone_Check(typing)) {
		result = AST_FMULTIPLE_KEEPLAST;
	} else if (typing == &DeeTuple_Type) {
		result = AST_FMULTIPLE_TUPLE;
	} else if (typing == &DeeList_Type) {
		result = AST_FMULTIPLE_LIST;
	} else if (typing == &DeeHashSet_Type) {
		result = AST_FMULTIPLE_HASHSET;
	} else if (typing == &DeeDict_Type) {
		result = AST_FMULTIPLE_DICT;
	} else if (typing == &DeeSeq_Type) {
		result = AST_FMULTIPLE_GENERIC;
	} else if (typing == &DeeMapping_Type) {
		result = AST_FMULTIPLE_GENERIC_KEYS;
	} else {
		DeeError_Throwf(&DeeError_TypeError,
		                "Invalid multi-branch typing: %k",
		                typing);
		result = (uint16_t)-1;
	}
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makemultiple(DeeCompilerObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv,
                 DeeObject *kw) {
	DREF DeeObject *result = NULL;
	uint16_t flags;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	DeeObject *branches;
	DeeTypeObject *typing = (DeeTypeObject *)Dee_None;
	DREF DeeCompilerAstObject **branch_v;
	size_t i, branch_c;
	PRIVATE struct keyword kwlist[] = { K(branches), K(typing), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|ooo:makemultiple", &branches, &typing, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	flags = get_ast_multiple_typing(typing);
	if
		unlikely(flags == (uint16_t)-1)
	goto done;
	branch_v = (DREF DeeCompilerAstObject **)DeeSeq_AsHeapVector(branches, &branch_c);
	if
		unlikely(!branch_v)
	goto done;
#ifdef CONFIG_AST_IS_STRUCT
#error "This loop doesn't work when asts are structs"
#endif /* CONFIG_AST_IS_STRUCT */
	for (i = 0; i < branch_c; ++i) {
		struct ast *branch_ast;
		/* Load the ast objects from each of the branches. */
		if (DeeObject_AssertTypeExact((DeeObject *)branch_v[i], &DeeCompilerAst_Type))
			goto err_branch_v;
		if
			unlikely(branch_v[i]->ci_compiler != self)
		{
			err_invalid_ast_compiler(branch_v[i]);
			goto err_branch_v;
		}
		/* Load the internal ast object into the vector slot. */
		branch_ast = branch_v[i]->ci_value;
		if (branch_ast->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(branch_v[i], ast_scope->s_base);
			goto err_branch_v;
		}
		ast_incref(branch_ast);
		Dee_Decref(branch_v[i]);
		branch_v[i] = (DREF DeeCompilerAstObject *)branch_ast;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto err_branch_v;
	result_ast->a_type            = AST_MULTIPLE;
	result_ast->a_flag            = flags;
	result_ast->a_multiple.m_astc = branch_c;
	result_ast->a_multiple.m_astv = (DREF struct ast **)branch_v;
	result                        = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
	__IF0 {
	err_branch_v:
		while (branch_c--)
			Dee_Decref(branch_v[branch_c]);
		Dee_Free(branch_v);
	}
done:
	COMPILER_END();
	return result;
}

PRIVATE struct keyword makeexpr_kwlist[] = { K(expr), K(scope), K(loc), KEND };

PRIVATE DREF DeeObject *DCALL
ast_makereturn(DeeCompilerObject *__restrict self,
               size_t argc, DeeObject **__restrict argv,
               DeeObject *kw) {
	DREF DeeObject *result        = NULL;
	DeeCompilerAstObject *expr    = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, makeexpr_kwlist, "|ooo:makereturn", &expr, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if (!DeeNone_Check(expr)) {
		if (DeeObject_AssertTypeExact((DeeObject *)expr, &DeeCompilerSymbol_Type))
			goto done;
		if
			unlikely(expr->ci_compiler != self)
		{
			err_invalid_ast_compiler(expr);
			goto done;
		}
		if
			unlikely(expr->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(expr, ast_scope->s_base);
			goto done;
		}
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type   = AST_RETURN;
	result_ast->a_return = NULL;
	if (!DeeNone_Check(expr)) {
		result_ast->a_return = expr->ci_value;
		ast_incref(expr->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makeyield(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
	DREF DeeObject *result        = NULL;
	DeeCompilerAstObject *expr    = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, makeexpr_kwlist, "o|oo:makeyield", &expr, &scope, &loc))
		goto done;
	if (DeeObject_AssertTypeExact((DeeObject *)expr, &DeeCompilerSymbol_Type))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(expr->ci_compiler != self)
	{
		err_invalid_ast_compiler(expr);
		goto done;
	}
	if
		unlikely(expr->ci_value->a_scope->s_base != ast_scope->s_base)
	{
		err_invalid_ast_basescope(expr, ast_scope->s_base);
		goto done;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type  = AST_YIELD;
	result_ast->a_throw = expr->ci_value;
	ast_incref(expr->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makethrow(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
	DREF DeeObject *result        = NULL;
	DeeCompilerAstObject *expr    = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, makeexpr_kwlist, "|ooo:makethrow", &expr, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if (!DeeNone_Check(expr)) {
		if (DeeObject_AssertTypeExact((DeeObject *)expr, &DeeCompilerSymbol_Type))
			goto done;
		if
			unlikely(expr->ci_compiler != self)
		{
			err_invalid_ast_compiler(expr);
			goto done;
		}
		if
			unlikely(expr->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(expr, ast_scope->s_base);
			goto done;
		}
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type  = AST_THROW;
	result_ast->a_throw = NULL;
	if (!DeeNone_Check(expr)) {
		result_ast->a_throw = expr->ci_value;
		ast_incref(expr->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}


LOCAL int DCALL
parse_handler_flags(char const *__restrict flags,
                    struct catch_expr *__restrict result) {
	char const *next_flag;
	size_t flag_length;
	while (*flags) {
		next_flag = strchr(flags, ',');
		if (!next_flag) {
			next_flag   = strend(flags);
			flag_length = (size_t)(next_flag - flags);
		} else {
			flag_length = (size_t)(next_flag - flags);
			++next_flag;
		}
		if (flag_length) {
#define IS_FLAG(x) (flag_length == COMPILER_STRLEN(x) && memcmp(flags, x, COMPILER_STRLEN(x)) == 0)
			/**/ if (IS_FLAG("finally"))
				result->ce_flags |= EXCEPTION_HANDLER_FFINALLY;
			else if (IS_FLAG("interrupt"))
				result->ce_flags |= EXCEPTION_HANDLER_FINTERPT;
			else {
				return DeeError_Throwf(&DeeError_ValueError,
				                       "Unknown handler flag %$q",
				                       flag_length,
				                       flags);
			}
#undef IS_FLAG
		}
		flags = next_flag;
	}
	return 0;
}


LOCAL int DCALL
unpack_catch_expression(DeeObject *__restrict triple,
                        struct catch_expr *__restrict result,
                        DeeBaseScopeObject *__restrict base_scope) {
	/* (:flags:?Dstring, #ast mask, #code:?AAst?Ert:Compiler) */
	DeeCompilerAstObject *args[3];
	if (DeeObject_Unpack(triple, 3, (DeeObject **)args))
		goto err;
	if (!DeeNone_Check(args[1])) {
		if (DeeObject_AssertTypeExact((DeeObject *)args[1], &DeeCompilerAst_Type))
			goto err;
		if
			unlikely(args[1]->ci_compiler != DeeCompiler_Current)
		{
			err_invalid_ast_compiler(args[1]);
			goto err;
		}
		if
			unlikely(args[1]->ci_value->a_scope->s_base != base_scope)
		{
			err_invalid_ast_basescope(args[1], base_scope);
			goto err;
		}
	}
	if (DeeObject_AssertTypeExact((DeeObject *)args[2], &DeeCompilerAst_Type))
		goto err;
	if
		unlikely(args[2]->ci_compiler != DeeCompiler_Current)
	{
		err_invalid_ast_compiler(args[2]);
		goto err;
	}
	if
		unlikely(args[2]->ci_value->a_scope->s_base != base_scope)
	{
		err_invalid_ast_basescope(args[2], base_scope);
		goto err;
	}
	/* Parse flags. */
	result->ce_mode  = CATCH_EXPR_FNORMAL;
	result->ce_flags = EXCEPTION_HANDLER_FNORMAL;
	if (DeeString_Check(args[0])) {
		if
			unlikely(parse_handler_flags(DeeString_STR((DeeObject *)args[0]), result))
		goto err;
	} else {
		if (DeeObject_AsUInt16((DeeObject *)args[0], &result->ce_flags))
			goto err;
	}
	/* Fill in the code and mask branches. */
	if (DeeNone_Check(args[1])) {
		result->ce_mask = NULL;
	} else {
		result->ce_mask = args[1]->ci_value;
		ast_incref(result->ce_mask);
	}
	result->ce_code = args[2]->ci_value;
	ast_incref(result->ce_code);
	return 0;
err:
	return -1;
}


INTERN struct catch_expr *DCALL
unpack_catch_expressions(DeeObject *__restrict handlers,
                         size_t *__restrict pcatch_c,
                         DeeBaseScopeObject *__restrict base_scope) {
	struct catch_expr *catch_v;
	size_t catch_c, catch_a, i;
	int error;
	DREF DeeObject *iterator, *elem;
	catch_c = DeeFastSeq_GetSize(handlers);
	/* Make use of fast-sequence optimizations. */
	if (catch_c != DEE_FASTSEQ_NOTFAST) {
		catch_v = (struct catch_expr *)Dee_Malloc(catch_c * sizeof(struct catch_expr));
		if
			unlikely(!catch_v)
		goto done;
		for (i = 0; i < catch_c; ++i) {
			elem = DeeFastSeq_GetItem(handlers, i);
			if
				unlikely(!elem)
			goto err_fast;
			error = unpack_catch_expression(elem, &catch_v[i], base_scope);
			Dee_Decref(elem);
			if
				unlikely(error)
			goto err_fast;
		}
		goto done;
	err_fast:
		while (i--) {
			ast_xdecref(catch_v[i].ce_mask);
			ast_decref(catch_v[i].ce_code);
		}
		Dee_Free(catch_v);
		catch_v = NULL;
		goto done;
	}
	/* Use an iterator. */
	catch_v = NULL;
	catch_c = catch_a = 0;
	iterator          = DeeObject_IterSelf(handlers);
	if
		unlikely(!iterator)
	goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		ASSERT(catch_c <= catch_a);
		if (catch_c >= catch_a) {
			size_t new_catch_a = catch_a * 2;
			struct catch_expr *new_catch_v;
			if
				unlikely(!new_catch_a)
			new_catch_a = 2;
			new_catch_v = (struct catch_expr *)Dee_TryRealloc(catch_v, new_catch_a *
			                                                           sizeof(struct catch_expr));
			if
				unlikely(!new_catch_v)
			{
				new_catch_a = catch_c + 1;
				new_catch_v = (struct catch_expr *)Dee_Realloc(catch_v, new_catch_a *
				                                                        sizeof(struct catch_expr));
				if
					unlikely(!new_catch_v)
				goto err_catch_elem;
			}
			catch_v = new_catch_v;
			catch_a = new_catch_a;
		}
		if
			unlikely(unpack_catch_expression(elem, &catch_v[catch_c], base_scope))
		goto err_catch_elem;
		++catch_c;
	}
	Dee_Decref(iterator);
	if
		unlikely(!elem)
	goto err_catch;
	/* Release unused memory. */
	if (catch_c != catch_a) {
		struct catch_expr *new_catch_v;
		new_catch_v = (struct catch_expr *)Dee_TryRealloc(catch_v, catch_c *
		                                                           sizeof(struct catch_expr));
		if
			likely(new_catch_v)
		catch_v = new_catch_v;
	}
done:
	*pcatch_c = catch_c;
	return catch_v;
err_catch_elem:
	Dee_Decref(elem);
	Dee_Decref(iterator);
err_catch:
	i = catch_c;
	goto err_fast;
}


PRIVATE DREF DeeObject *DCALL
ast_maketry(DeeCompilerObject *__restrict self,
            size_t argc, DeeObject **__restrict argv,
            DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeCompilerAstObject *guard;
	DeeObject *handlers;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	PRIVATE struct keyword kwlist[] = { K(guard), K(handlers), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "oo|oo:maketry", &guard, &handlers, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if (DeeObject_AssertTypeExact((DeeObject *)guard, &DeeCompilerSymbol_Type))
		goto done;
	if
		unlikely(guard->ci_compiler != self)
	{
		err_invalid_ast_compiler(guard);
		goto done;
	}
	if
		unlikely(guard->ci_value->a_scope->s_base != ast_scope->s_base)
	{
		err_invalid_ast_basescope(guard, ast_scope->s_base);
		goto done;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	/* Unpack the given handler expressions vector. */
	result_ast->a_try.t_catchv = unpack_catch_expressions(handlers,
	                                                      &result_ast->a_try.t_catchc,
	                                                      ast_scope->s_base);
	if
		unlikely(!result_ast->a_try.t_catchv)
	{
		Dee_DecrefNokill(ast_scope);
		Dee_DecrefNokill(&DeeAst_Type);
		ast_free(result_ast);
		goto done;
	}
	result_ast->a_type        = AST_TRY;
	result_ast->a_try.t_guard = guard->ci_value;
	ast_incref(guard->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}


INTERN int DCALL
parse_loop_flags(char const *__restrict flags,
                 uint16_t *__restrict presult) {
	char const *next_flag;
	size_t flag_length;
	while (*flags) {
		next_flag = strchr(flags, ',');
		if (!next_flag) {
			next_flag   = strend(flags);
			flag_length = (size_t)(next_flag - flags);
		} else {
			flag_length = (size_t)(next_flag - flags);
			++next_flag;
		}
		if (flag_length) {
#define IS_FLAG(x) (flag_length == COMPILER_STRLEN(x) && memcmp(flags, x, COMPILER_STRLEN(x)) == 0)
			/**/ if (IS_FLAG("foreach"))
				*presult |= AST_FLOOP_FOREACH;
			else if (IS_FLAG("postcond"))
				*presult |= AST_FLOOP_POSTCOND;
			else if (IS_FLAG("unlikely"))
				*presult |= AST_FLOOP_UNLIKELY;
			else {
				return DeeError_Throwf(&DeeError_ValueError,
				                       "Unknown loop flag %$q",
				                       flag_length,
				                       flags);
			}
#undef IS_FLAG
		}
		flags = next_flag;
	}
	return 0;
}



PRIVATE DREF DeeObject *DCALL
ast_makeloop(DeeCompilerObject *__restrict self,
             size_t argc, DeeObject **__restrict argv,
             DeeObject *kw) {
	DREF DeeObject *result        = NULL;
	uint16_t flags                = 0;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	DeeCompilerAstObject *cond = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *next = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *loop = (DeeCompilerAstObject *)Dee_None;
	/* "(flags:?Dstring,ast elem=!N,ast iter,ast loop=!N,scope:?AScope?Ert:Compiler=!N)->?AAst?Ert:Compiler\n"
  * "(flags:?Dstring,ast cond=!N,ast next=!N,ast loop=!N,scope:?AScope?Ert:Compiler=!N)->?AAst?Ert:Compiler\n" */
	if
		unlikely(!argc)
	{
		err_invalid_argc("makeloop", argc, 1, 5);
		goto done2;
	}
	if (DeeString_Check(argv[0])) {
		if
			unlikely(DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
		goto done2;
		if
			unlikely(parse_loop_flags(DeeString_STR(argv[0]), &flags))
		goto done2;
	} else {
		if
			unlikely(DeeObject_AsUInt16(argv[0], &flags))
		goto done2;
	}
	--argc, ++argv;
	COMPILER_BEGIN(self);
	if (flags & AST_FLOOP_FOREACH) {
		PRIVATE struct keyword kwlist[] = { K(elem), K(iter), K(loop), K(scope), K(loc), KEND };
		if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|ooooo:makeloop", &cond, &next, &loop, &scope, &loc))
			goto done;
		if
			unlikely((ast_scope = get_scope(scope)) == NULL)
		goto done;
	check_next:
		/* The next (iter) operand is mandatory in foreach loop branches. */
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)next, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(next->ci_compiler != DeeCompiler_Current)
		{
			err_invalid_ast_compiler(next);
			goto done;
		}
		if
			unlikely(next->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(next, ast_scope->s_base);
			goto done;
		}
	} else {
		PRIVATE struct keyword kwlist[] = { K(cond), K(next), K(loop), K(scope), K(loc), KEND };
		if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|ooooo:makeloop", &cond, &next, &loop, &scope, &loc))
			goto done;
		if
			unlikely((ast_scope = get_scope(scope)) == NULL)
		goto done;
		if (!DeeNone_Check(next))
			goto check_next;
	}
	if (!DeeNone_Check(cond)) {
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)cond, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(cond->ci_compiler != DeeCompiler_Current)
		{
			err_invalid_ast_compiler(cond);
			goto done;
		}
		if
			unlikely(cond->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(cond, ast_scope->s_base);
			goto done;
		}
	}
	if (!DeeNone_Check(loop)) {
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)loop, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(loop->ci_compiler != DeeCompiler_Current)
		{
			err_invalid_ast_compiler(loop);
			goto done;
		}
		if
			unlikely(loop->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(loop, ast_scope->s_base);
			goto done;
		}
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type        = AST_LOOP;
	result_ast->a_flag        = flags;
	result_ast->a_loop.l_cond = NULL;
	result_ast->a_loop.l_next = NULL;
	result_ast->a_loop.l_loop = NULL;
	if (!DeeNone_Check(cond)) {
		result_ast->a_loop.l_cond = cond->ci_value;
		ast_incref(cond->ci_value);
	}
	if (!DeeNone_Check(next)) {
		result_ast->a_loop.l_next = next->ci_value;
		ast_incref(next->ci_value);
	}
	if (!DeeNone_Check(loop)) {
		result_ast->a_loop.l_loop = loop->ci_value;
		ast_incref(loop->ci_value);
	}

	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
done2:
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makeloopctl(DeeCompilerObject *__restrict self,
                size_t argc, DeeObject **__restrict argv,
                DeeObject *kw) {
	DREF DeeObject *result = NULL;
	bool isbreak;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	PRIVATE struct keyword kwlist[] = { K(isbreak), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "b|oo:makeloopctl", &isbreak, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type = AST_LOOPCTL;
	result_ast->a_flag = isbreak ? AST_FLOOPCTL_BRK : AST_FLOOPCTL_CON;
	result             = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}


INTERN int DCALL
parse_conditional_flags(char const *__restrict flags,
                        uint16_t *__restrict presult) {
	char const *next_flag;
	size_t flag_length;
	while (*flags) {
		next_flag = strchr(flags, ',');
		if (!next_flag) {
			next_flag   = strend(flags);
			flag_length = (size_t)(next_flag - flags);
		} else {
			flag_length = (size_t)(next_flag - flags);
			++next_flag;
		}
		if (flag_length) {
#define IS_FLAG(x) (flag_length == COMPILER_STRLEN(x) && memcmp(flags, x, sizeof(x) - sizeof(char)) == 0)
#define IS_FLAG_S(len, s) (flag_length == (len) && memcmp(flags, s, (len) * sizeof(char)) == 0)
			/**/ if (IS_FLAG_S(4, DeeString_STR(&str_bool)))
				*presult |= AST_FCOND_BOOL;
			else if (IS_FLAG("likely"))
				*presult |= AST_FCOND_LIKELY;
			else if (IS_FLAG("unlikely"))
				*presult |= AST_FCOND_UNLIKELY;
			else {
				return DeeError_Throwf(&DeeError_ValueError,
				                       "Unknown conditional flag %$q",
				                       flag_length,
				                       flags);
			}
#undef IS_FLAG_S
#undef IS_FLAG
		}
		flags = next_flag;
	}
	return 0;
}

PRIVATE DREF DeeObject *DCALL
ast_makeconditional(DeeCompilerObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv,
                    DeeObject *kw) {
	DREF DeeObject *result = NULL;
	uint16_t flags         = 0;
	DeeCompilerAstObject *cond;
	DeeCompilerAstObject *tt      = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *ff      = (DeeCompilerAstObject *)Dee_None;
	DeeStringObject *flags_str    = (DeeStringObject *)Dee_EmptyString;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	PRIVATE struct keyword kwlist[] = { K(cond), K(tt), K(ff), K(flags), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|ooooo:makeconditional", &cond, &tt, &ff, &flags_str, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if (flags_str != (DeeStringObject *)Dee_EmptyString) {
		if (DeeString_Check(flags_str)) {
			if (parse_conditional_flags(DeeString_STR(flags_str), &flags))
				goto done;
		} else {
			if (DeeObject_AsUInt16((DeeObject *)flags_str, &flags))
				goto done;
		}
	}
	if
		unlikely(DeeObject_AssertTypeExact((DeeObject *)cond, &DeeCompilerAst_Type))
	goto done;
	if
		unlikely(cond->ci_compiler != self)
	{
		err_invalid_ast_compiler(cond);
		goto done;
	}
	if
		unlikely(cond->ci_value->a_scope->s_base != ast_scope->s_base)
	{
		err_invalid_ast_basescope(cond, ast_scope->s_base);
		goto done;
	}
	if (!DeeNone_Check(tt)) {
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)tt, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(tt->ci_compiler != self)
		{
			err_invalid_ast_compiler(tt);
			goto done;
		}
		if
			unlikely(tt->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(tt, ast_scope->s_base);
			goto done;
		}
	} else if
		unlikely(DeeNone_Check(ff))
	{
		DeeError_Throwf(&DeeError_TypeError,
		                "Both the true-, as well as the false-branch have been given as `none'");
		goto done;
	}
	if (!DeeNone_Check(ff)) {
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)ff, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(ff->ci_compiler != self)
		{
			err_invalid_ast_compiler(ff);
			goto done;
		}
		if
			unlikely(ff->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(ff, ast_scope->s_base);
			goto done;
		}
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type               = AST_CONDITIONAL;
	result_ast->a_flag               = flags;
	result_ast->a_conditional.c_cond = cond->ci_value;
	result_ast->a_conditional.c_tt   = NULL;
	result_ast->a_conditional.c_ff   = NULL;
	ast_incref(cond->ci_value);
	if (!DeeNone_Check(tt)) {
		result_ast->a_conditional.c_tt = tt->ci_value;
		ast_incref(tt->ci_value);
	}
	if (!DeeNone_Check(ff)) {
		result_ast->a_conditional.c_ff = ff->ci_value;
		ast_incref(ff->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}


PRIVATE DREF DeeObject *DCALL
ast_makebool(DeeCompilerObject *__restrict self,
             size_t argc, DeeObject **__restrict argv,
             DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeCompilerAstObject *expr;
	bool negate                   = false;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	PRIVATE struct keyword kwlist[] = { K(expr), K(negate), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|boo:makebool", &expr, &negate, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(DeeObject_AssertTypeExact((DeeObject *)expr, &DeeCompilerAst_Type))
	goto done;
	if
		unlikely(expr->ci_compiler != self)
	{
		err_invalid_ast_compiler(expr);
		goto done;
	}
	if
		unlikely(expr->ci_value->a_scope->s_base != ast_scope->s_base)
	{
		err_invalid_ast_basescope(expr, ast_scope->s_base);
		goto done;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type = AST_BOOL;
	result_ast->a_flag = negate ? AST_FBOOL_NEGATE : AST_FBOOL_NORMAL;
	result_ast->a_bool = expr->ci_value;
	ast_incref(expr->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makeexpand(DeeCompilerObject *__restrict self,
               size_t argc, DeeObject **__restrict argv,
               DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeCompilerAstObject *expr;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, makeexpr_kwlist, "o|oo:makeexpand", &expr, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(DeeObject_AssertTypeExact((DeeObject *)expr, &DeeCompilerAst_Type))
	goto done;
	if
		unlikely(expr->ci_compiler != self)
	{
		err_invalid_ast_compiler(expr);
		goto done;
	}
	if
		unlikely(expr->ci_value->a_scope->s_base != ast_scope->s_base)
	{
		err_invalid_ast_basescope(expr, ast_scope->s_base);
		goto done;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type = AST_EXPAND;
	result_ast->a_bool = expr->ci_value;
	ast_incref(expr->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

INTERN int DCALL
check_function_code_scope(DeeBaseScopeObject *code_scope,
                          DeeBaseScopeObject *ast_base_scope) {
	if
		unlikely(code_scope == ast_base_scope)
	{
		return DeeError_Throwf(&DeeError_ReferenceError,
		                       "Function code cannot be located in the same "
		                       "base-scope as the function initializer");
	}
	/* Make sure that the base-scope of the function
  * initializer can be reached from the function itself. */
	for (;;) {
		code_scope = code_scope->bs_prev;
		if (code_scope == ast_base_scope)
			break;
		if
			unlikely(!code_scope)
		{
			return DeeError_Throwf(&DeeError_ReferenceError,
			                       "Function initializer scope is not "
			                       "reachable from function code");
		}
	}
	return 0;
}

PRIVATE DREF DeeObject *DCALL
ast_makefunction(DeeCompilerObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv,
                 DeeObject *kw) {
	/* "(code:?AAst?Ert:Compiler,scope:?AScope?Ert:Compiler=!N)->?AAst?Ert:Compiler\n" */
	DREF DeeObject *result = NULL;
	DeeCompilerAstObject *code;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	DeeBaseScopeObject *code_scope;
	PRIVATE struct keyword kwlist[] = { K(code), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|oo:makeexpand", &code, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(DeeObject_AssertTypeExact((DeeObject *)code, &DeeCompilerAst_Type))
	goto done;
	if
		unlikely(code->ci_compiler != self)
	{
		err_invalid_ast_compiler(code);
		goto done;
	}
	code_scope = code->ci_value->a_scope->s_base;
	if
		unlikely(check_function_code_scope(code_scope, ast_scope->s_base))
	goto done;
	/* Setup a new function branch. */
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type             = AST_FUNCTION;
	result_ast->a_function.f_code  = code->ci_value;
	result_ast->a_function.f_scope = code_scope;
	ast_incref(code->ci_value);
	Dee_Incref((DeeObject *)code_scope);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

INTERN int DCALL
get_operator_id(DeeObject *__restrict opid, uint16_t *__restrict presult) {
	if (DeeString_Check(opid)) {
		char *name = DeeString_STR(opid);
		*presult   = Dee_OperatorFromName(NULL, name);
		if (*presult != (uint16_t)-1)
			return 0;
		/* Resolve special operator names. */
		switch (name[0]) {
		case '+':
			if (name[1])
				goto unknown_str;
			*presult = AST_OPERATOR_POS_OR_ADD;
			break;
		case '-':
			if (name[1])
				goto unknown_str;
			*presult = AST_OPERATOR_NEG_OR_SUB;
			break;
		case '[':
			if (name[1] == ':') {
				if (name[2] != ']')
					goto unknown_str;
				if (name[3])
					goto unknown_str;
				*presult = AST_OPERATOR_GETRANGE_OR_SETRANGE;
			} else {
				if (name[1] != ']')
					goto unknown_str;
				if (name[2])
					goto unknown_str;
				*presult = AST_OPERATOR_GETITEM_OR_SETITEM;
			}
			break;
		case '.':
			if (name[1])
				goto unknown_str;
			*presult = AST_OPERATOR_GETATTR_OR_SETATTR;
			break;
		default:
		unknown_str:
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Unknown operator %q",
			                       name);
		}
		return 0;
	}
	return DeeObject_AsUInt16(opid, presult);
}


PRIVATE DREF DeeObject *DCALL
ast_makeoperatorfunc(DeeCompilerObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
	/* "(string name,binding:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N)->?AAst?Ert:Compiler\n"
  * "(int name,binding:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N)->?AAst?Ert:Compiler\n" */
	DREF DeeObject *result = NULL;
	DeeObject *name;
	uint16_t id;
	DeeCompilerAstObject *binding = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	PRIVATE struct keyword kwlist[] = { K(name), K(binding), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|ooo:makeoperatorfunc", &name, &binding, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(get_operator_id(name, &id))
	goto done;
	if (!DeeNone_Check(binding)) {
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)binding, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(binding->ci_compiler != self)
		{
			err_invalid_ast_compiler(binding);
			goto done;
		}
		if
			unlikely(binding->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(binding, ast_scope->s_base);
			goto done;
		}
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type = AST_OPERATOR_FUNC;
	result_ast->a_flag = id;
	if (!DeeNone_Check(binding)) {
		result_ast->a_operator_func.of_binding = binding->ci_value;
		ast_incref(binding->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

INTERN int DCALL
parse_operator_flags(char const *__restrict flags,
                     uint16_t *__restrict presult) {
	char const *next_flag;
	size_t flag_length;
	while (*flags) {
		next_flag = strchr(flags, ',');
		if (!next_flag) {
			next_flag   = strend(flags);
			flag_length = (size_t)(next_flag - flags);
		} else {
			flag_length = (size_t)(next_flag - flags);
			++next_flag;
		}
		if (flag_length) {
#define IS_FLAG(x) (flag_length == COMPILER_STRLEN(x) && memcmp(flags, x, COMPILER_STRLEN(x)) == 0)
			/**/ if (IS_FLAG("post"))
				*presult |= AST_OPERATOR_FPOSTOP;
			else if (IS_FLAG("varargs"))
				*presult |= AST_OPERATOR_FVARARGS;
			else if (IS_FLAG("maybeprefix"))
				*presult |= AST_OPERATOR_FMAYBEPFX;
			else if (IS_FLAG("dontoptimize"))
				*presult |= AST_OPERATOR_FDONTOPT;
			else {
				return DeeError_Throwf(&DeeError_ValueError,
				                       "Unknown operator flag %$q",
				                       flag_length,
				                       flags);
			}
#undef IS_FLAG
		}
		flags = next_flag;
	}
	return 0;
}

PRIVATE DREF DeeObject *DCALL
ast_makeoperator(DeeCompilerObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv,
                 DeeObject *kw) {
	/* "(string name,a:?AAst?Ert:Compiler,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,d:?AAst?Ert:Compiler=!N,flags=!P{},scope=!N)->?AAst?Ert:Compiler\n"
  * "(int name,a:?AAst?Ert:Compiler,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,d:?AAst?Ert:Compiler=!N,flags=!P{},scope=!N)->?AAst?Ert:Compiler\n" */
	DREF DeeObject *result = NULL;
	DeeObject *name;
	uint16_t id, flags = 0;
	DeeCompilerAstObject *a;
	DeeCompilerAstObject *b       = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *c       = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *d       = (DeeCompilerAstObject *)Dee_None;
	DeeStringObject *flags_str    = (DeeStringObject *)Dee_EmptyString;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	PRIVATE struct keyword kwlist[] = { K(name), K(a), K(b), K(c), K(d), K(flags), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "oo|oooooo:makeoperator", &name, &a, &b, &c, &d, &flags_str, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(get_operator_id(name, &id))
	goto done;
	if (flags_str != (DeeStringObject *)Dee_EmptyString) {
		if (DeeString_Check(flags_str)) {
			if (parse_operator_flags(DeeString_STR(flags_str), &flags))
				goto done;
		} else {
			if (DeeObject_AsUInt16((DeeObject *)flags_str, &flags))
				goto done;
		}
	}
	if
		unlikely(DeeObject_AssertTypeExact((DeeObject *)a, &DeeCompilerAst_Type))
	goto done;
	if
		unlikely(a->ci_compiler != self)
	{
		err_invalid_ast_compiler(a);
		goto done;
	}
	if
		unlikely(a->ci_value->a_scope->s_base != ast_scope->s_base)
	{
		err_invalid_ast_basescope(a, ast_scope->s_base);
		goto done;
	}
	if (!DeeNone_Check(b)) {
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)b, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(b->ci_compiler != self)
		{
			err_invalid_ast_compiler(b);
			goto done;
		}
		if
			unlikely(b->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(b, ast_scope->s_base);
			goto done;
		}
		if (!DeeNone_Check(c)) {
			if
				unlikely(DeeObject_AssertTypeExact((DeeObject *)c, &DeeCompilerAst_Type))
			goto done;
			if
				unlikely(c->ci_compiler != self)
			{
				err_invalid_ast_compiler(c);
				goto done;
			}
			if
				unlikely(c->ci_value->a_scope->s_base != ast_scope->s_base)
			{
				err_invalid_ast_basescope(c, ast_scope->s_base);
				goto done;
			}
			if (!DeeNone_Check(d)) {
				if
					unlikely(DeeObject_AssertTypeExact((DeeObject *)d, &DeeCompilerAst_Type))
				goto done;
				if
					unlikely(d->ci_compiler != self)
				{
					err_invalid_ast_compiler(d);
					goto done;
				}
				if
					unlikely(d->ci_value->a_scope->s_base != ast_scope->s_base)
				{
					err_invalid_ast_basescope(d, ast_scope->s_base);
					goto done;
				}
			}
		}
	}
	switch (id) {
	case AST_OPERATOR_POS_OR_ADD:
		id = DeeNone_Check(b) ? OPERATOR_POS : OPERATOR_ADD;
		break;
	case AST_OPERATOR_NEG_OR_SUB:
		id = DeeNone_Check(b) ? OPERATOR_NEG : OPERATOR_SUB;
		break;
	case AST_OPERATOR_GETITEM_OR_SETITEM:
		id = DeeNone_Check(c) ? OPERATOR_GETITEM : OPERATOR_SETITEM;
		break;
	case AST_OPERATOR_GETRANGE_OR_SETRANGE:
		id = DeeNone_Check(c) ? OPERATOR_GETRANGE : OPERATOR_SETRANGE;
		break;
	case AST_OPERATOR_GETATTR_OR_SETATTR:
		id = DeeNone_Check(c) ? OPERATOR_GETATTR : OPERATOR_SETATTR;
		break;
	default: break;
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type              = AST_OPERATOR;
	result_ast->a_flag              = id;
	result_ast->a_operator.o_exflag = flags;
	result_ast->a_operator.o_op0    = a->ci_value;
	result_ast->a_operator.o_op1    = NULL;
	result_ast->a_operator.o_op2    = NULL;
	result_ast->a_operator.o_op3    = NULL;
	ast_incref(a->ci_value);
	if (!DeeNone_Check(b)) {
		result_ast->a_operator.o_op1 = b->ci_value;
		ast_incref(b->ci_value);
		if (!DeeNone_Check(c)) {
			result_ast->a_operator.o_op2 = c->ci_value;
			ast_incref(c->ci_value);
			if (!DeeNone_Check(d)) {
				result_ast->a_operator.o_op3 = d->ci_value;
				ast_incref(d->ci_value);
			}
		}
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}


INTERN int32_t DCALL
get_action_by_name(char const *__restrict name) {
#define EQAT(ptr, str) (memcmp(ptr, str, sizeof(str)) == 0)
#define RETURN(id)     \
	do {               \
		result = (id); \
		goto done;     \
	}                  \
	__WHILE0
	int32_t result;
	switch (name[0]) {
	case 'a':
		if (name[1] == 's' && !name[2])
			RETURN(AST_FACTION_AS);
		if (EQAT(name + 1, "ny"))
			RETURN(AST_FACTION_ANY);
		if (EQAT(name + 1, "ll"))
			RETURN(AST_FACTION_ALL);
		if (EQAT(name + 1, "ssert"))
			RETURN(AST_FACTION_ASSERT);
		break;
	case 'b':
		if (EQAT(name + 1, "oundattr"))
			RETURN(AST_FACTION_BOUNDATTR);
		if (EQAT(name + 1, "ounditem"))
			RETURN(AST_FACTION_BOUNDITEM);
		break;
	case 'c':
		if (EQAT(name + 1, "lassof"))
			RETURN(AST_FACTION_CLASSOF);
		if (EQAT(name + 1, "allkw"))
			RETURN(AST_FACTION_CALL_KW);
		break;
	case 'd':
		if (EQAT(name + 1, "iffobj"))
			RETURN(AST_FACTION_DIFFOBJ);
		break;
	case 'i':
		if (name[1] == 'n' && !name[2])
			RETURN(AST_FACTION_IN);
		if (name[1] == 's' && !name[2])
			RETURN(AST_FACTION_IS);
		break;
	case 'p':
		if (name[1] == 'r' && name[2] == 'i' &&
		    name[3] == 'n' && name[4] == 't') {
			if (!name[5])
				RETURN(AST_FACTION_PRINT);
			if (name[5] == 'l' && name[6] == 'n' && !name[7])
				RETURN(AST_FACTION_PRINTLN);
		}
		break;
	case 'f':
		if (name[1] == 'p' &&
		    name[2] == 'r' && name[3] == 'i' &&
		    name[4] == 'n' && name[5] == 't') {
			if (!name[6])
				RETURN(AST_FACTION_FPRINT);
			if (name[6] == 'l' && name[7] == 'n' && !name[8])
				RETURN(AST_FACTION_FPRINTLN);
		}
		break;
	case 'm':
		if (name[1] == 'i' && name[2] == 'n' && !name[3])
			RETURN(AST_FACTION_MIN);
		if (name[1] == 'a' && name[2] == 'x' && !name[3])
			RETURN(AST_FACTION_MAX);
		break;
	case 'r':
		if (EQAT(name + 1, "ange"))
			RETURN(AST_FACTION_RANGE);
		break;
	case 's':
		if (EQAT(name + 1, "uperof"))
			RETURN(AST_FACTION_SUPEROF);
		if (EQAT(name + 1, "um"))
			RETURN(AST_FACTION_SUM);
		if (EQAT(name + 1, "tore"))
			RETURN(AST_FACTION_STORE);
		if (EQAT(name + 1, "ameobj"))
			RETURN(AST_FACTION_SAMEOBJ);
		break;
	case 't':
		if (EQAT(name + 1, "ypeof"))
			RETURN(AST_FACTION_TYPEOF);
		break;
	default: break;
	}
	result = DeeError_Throwf(&DeeError_ValueError,
	                         "Unknown action %q", name);
done:
	return result;
#undef EQAT
#undef RETURN
}

INTERN char const *DCALL
get_action_name(uint16_t action) {
	char const *result;
	switch (action & AST_FACTION_KINDMASK) {
#define ACTION(x) case (x)&AST_FACTION_KINDMASK
		ACTION(AST_FACTION_TYPEOF):
			result = "typeof";
			break;

		ACTION(AST_FACTION_CLASSOF):
			result = "classof";
			break;

		ACTION(AST_FACTION_SUPEROF):
			result = "superof";
			break;

		ACTION(AST_FACTION_PRINT):
			result = "print";
			break;

		ACTION(AST_FACTION_PRINTLN):
			result = "println";
			break;

		ACTION(AST_FACTION_FPRINT):
			result = "fprint";
			break;

		ACTION(AST_FACTION_FPRINTLN):
			result = "fprintln";
			break;

		ACTION(AST_FACTION_RANGE):
			result = "range";
			break;

		ACTION(AST_FACTION_IS):
			result = "is";
			break;

		ACTION(AST_FACTION_IN):
			result = "in";
			break;

		ACTION(AST_FACTION_AS):
			result = "as";
			break;

		ACTION(AST_FACTION_MIN):
			result = "min";
			break;

		ACTION(AST_FACTION_MAX):
			result = "max";
			break;

		ACTION(AST_FACTION_SUM):
			result = "sum";
			break;

		ACTION(AST_FACTION_ANY):
			result = "any";
			break;

		ACTION(AST_FACTION_ALL):
			result = "all";
			break;

		ACTION(AST_FACTION_STORE):
			result = "store";
			break;

		ACTION(AST_FACTION_ASSERT):
		ACTION(AST_FACTION_ASSERT_M):
			result = "assert";
			break;

		ACTION(AST_FACTION_BOUNDATTR):
			result = "boundattr";
			break;

		ACTION(AST_FACTION_BOUNDITEM):
			result = "bounditem";
			break;

		ACTION(AST_FACTION_SAMEOBJ):
			result = "sameobj";
			break;

		ACTION(AST_FACTION_DIFFOBJ):
			result = "diffobj";
			break;

		ACTION(AST_FACTION_CALL_KW):
			result = "callkw";
			break;

	default: result = NULL; break;
	}
	return result;
}


PRIVATE DREF DeeObject *DCALL
ast_makeaction(DeeCompilerObject *__restrict self,
               size_t argc, DeeObject **__restrict argv,
               DeeObject *kw) {
	/* "(string name,a:?AAst?Ert:Compiler=!N,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,bool mustrun=true,scope=!N)->?AAst?Ert:Compiler\n" */
	DREF DeeObject *result = NULL;
	DeeObject *name;
	int32_t id;
	DeeCompilerAstObject *a       = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *b       = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *c       = (DeeCompilerAstObject *)Dee_None;
	bool mustrun                  = true;
	DeeCompilerScopeObject *scope = (DeeCompilerScopeObject *)Dee_None;
	DeeScopeObject *ast_scope;
	DeeObject *loc = NULL;
	DREF struct ast *result_ast;
	uint8_t opc;
	PRIVATE struct keyword kwlist[] = { K(name), K(a), K(b), K(c), K(mustrun), K(scope), K(loc), KEND };
	COMPILER_BEGIN(self);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|oooboo:makeaction", &name, &a, &b, &c, &mustrun, &scope, &loc))
		goto done;
	if
		unlikely((ast_scope = get_scope(scope)) == NULL)
	goto done;
	if
		unlikely(DeeObject_AssertTypeExact(name, &DeeString_Type))
	goto done;
	if
		unlikely((id = get_action_by_name(DeeString_STR(name))) < 0)
	goto done;
	opc = 0;
	if (!DeeNone_Check(a)) {
		opc = 1;
		if
			unlikely(DeeObject_AssertTypeExact((DeeObject *)a, &DeeCompilerAst_Type))
		goto done;
		if
			unlikely(a->ci_compiler != self)
		{
			err_invalid_ast_compiler(a);
			goto done;
		}
		if
			unlikely(a->ci_value->a_scope->s_base != ast_scope->s_base)
		{
			err_invalid_ast_basescope(a, ast_scope->s_base);
			goto done;
		}
		if (!DeeNone_Check(b)) {
			opc = 2;
			if
				unlikely(DeeObject_AssertTypeExact((DeeObject *)b, &DeeCompilerAst_Type))
			goto done;
			if
				unlikely(b->ci_compiler != self)
			{
				err_invalid_ast_compiler(b);
				goto done;
			}
			if
				unlikely(b->ci_value->a_scope->s_base != ast_scope->s_base)
			{
				err_invalid_ast_basescope(b, ast_scope->s_base);
				goto done;
			}
			if (!DeeNone_Check(c)) {
				opc = 3;
				if
					unlikely(DeeObject_AssertTypeExact((DeeObject *)c, &DeeCompilerAst_Type))
				goto done;
				if
					unlikely(c->ci_compiler != self)
				{
					err_invalid_ast_compiler(c);
					goto done;
				}
				if
					unlikely(c->ci_value->a_scope->s_base != ast_scope->s_base)
				{
					err_invalid_ast_basescope(c, ast_scope->s_base);
					goto done;
				}
			}
		}
	}
	if (AST_FACTION_ARGC_GT((uint16_t)id) != opc) {
		if ((uint16_t)id == AST_FACTION_ASSERT && opc == 2)
			id = (int32_t)AST_FACTION_ASSERT_M;
		else {
			DeeError_Throwf(&DeeError_TypeError,
			                "Invalid operand count for action %k expecting %u operands when %u were given",
			                name,
			                (unsigned int)AST_FACTION_ARGC_GT((uint16_t)id),
			                (unsigned int)opc);
			goto done;
		}
	}
	result_ast = ast_new(ast_scope, loc);
	if
		unlikely(!result_ast)
	goto done;
	result_ast->a_type = AST_ACTION;
	result_ast->a_flag = (uint16_t)id;
	if (!mustrun)
		result_ast->a_flag |= AST_FACTION_MAYBERUN;
	result_ast->a_action.a_act0 = NULL;
	result_ast->a_action.a_act1 = NULL;
	result_ast->a_action.a_act2 = NULL;
	if (!DeeNone_Check(a)) {
		result_ast->a_action.a_act0 = a->ci_value;
		ast_incref(a->ci_value);
		if (!DeeNone_Check(b)) {
			result_ast->a_action.a_act1 = b->ci_value;
			ast_incref(b->ci_value);
			if (!DeeNone_Check(c)) {
				result_ast->a_action.a_act2 = c->ci_value;
				ast_incref(c->ci_value);
			}
		}
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
ast_makeclass(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
ast_makelabel(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
ast_makegoto(DeeCompilerObject *__restrict self,
             size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
ast_makeswitch(DeeCompilerObject *__restrict self,
               size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
ast_makeassembly(DeeCompilerObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN struct type_method compiler_methods[] = {
	{ "makeconstexpr", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeconstexpr,
	  DOC("(value,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @scope doesn't match @this\n"
	      "Construct a new constant-expression ast referring to @value"),
	  TYPE_METHOD_FKWDS },
	{ "makesym", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makesym,
	  DOC("(sym:?ASymbol?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @sym or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @sym is not reachable from the effectively used @scope\n"
	      "Construct a new branch that is using a symbol @sym\n"
	      "By default, the branch uses @sym for reading, however if the branch "
	      "is later used as target of a store-action branch, @sym will be used "
	      "for writing instead. Similarly, special handling is done when the "
	      "branch is used in an assembly output operand"),
	  TYPE_METHOD_FKWDS },
	{ "makeunbind", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeunbind,
	  DOC("(sym:?ASymbol?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @sym or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @sym is not reachable from the effectively used @scope\n"
	      "Construct a branch for unbinding the value of @sym at runtime"),
	  TYPE_METHOD_FKWDS },
	{ "makebound", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makebound,
	  DOC("(sym:?ASymbol?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @sym or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @sym is not reachable from the effectively used @scope\n"
	      "Construct a branch for checking if a given symbol @sym is bound"),
	  TYPE_METHOD_FKWDS },
	{ "makemultiple", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makemultiple,
	  DOC("(branches:?S?AAst?Ert:Compiler,typing:?DType=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of one of the given @branches or @scope doesn't match @this\n"
	      "@throw TypeError The given @typing is neither :none, nor one of the types listed below\n"
	      "@throw ReferenceError One of the given @branches is not part of the basescope of the effective @scope\n"
	      "Construct a multi-branch, which either behaves as keep-last (only the last ast from @branches "
	      "is used as expression value of the returned branch, while all others are evaluated before then), "
	      "when @typing is :none, or construct a sequence expression for the associated type when @typeing "
	      "is one of the following\n"
	      "%{table Type|Example|Description\n"
	      ":deemon:Tuple|${(a,b,c)}|Construct a Tuple expression\n"
	      ":deemon:List|${[a,b,c]}|Construct a List expression\n"
	      ":deemon:HashSet|-|Construct a mutable HashSet sequence expression\n"
	      ":deemon:Dict|-|Construct a mutable Dict-like mapping expression\n"
	      ":deemon:Sequence|${{ a, b, c }}|Construct an abstract sequence expression\n"
	      ":deemon:Mapping|${{ a: b, c: d }}|Construct an abstract mapping expression}\n"
	      "Note that in any kind of sequence branch, asts created by @makeexpand may "
	      "appear, and will be inlined as part of the greater whole expression"),
	  TYPE_METHOD_FKWDS },
	{ "makereturn", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makereturn,
	  DOC("(expr:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @expr or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @expr is not part of the basescope of the effective @scope\n"
	      "Construct a return-branch that either returns @expr, or :none when @expr is :none"),
	  TYPE_METHOD_FKWDS },
	{ "makeyield", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeyield,
	  DOC("(expr:?AAst?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @expr or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @expr is not part of the basescope of the effective @scope\n"
	      "Construct a yield-branch that either returns @expr, or :none when @expr is :none"),
	  TYPE_METHOD_FKWDS },
	{ "makethrow", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makethrow,
	  DOC("(expr:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @expr or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @expr is not part of the basescope of the effective @scope\n"
	      "Construct a throw-branch that either throws @expr, or re-throws the last exception when @expr is :none"),
	  TYPE_METHOD_FKWDS },
	{ "maketry", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_maketry,
	  DOC("(guard:?AAst?Ert:Compiler,handlers:?S?T3?Dstring?AAst?Ert:Compiler?AAst?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(guard:?AAst?Ert:Compiler,handlers:?S?T3?Dint?AAst?Ert:Compiler?AAst?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of one of the given branches or @scope doesn't match @this\n"
	      "@throw ValueError One of the flags-strings contains an unknown flag\n"
	      "@throw ReferenceError One of the given branch is not part of the basescope of the effective @scope\n"
	      "Construct a try-branch guarding @guard, referring to @handlers, which is a sequences "
	      "of tuples in the form of (:string flags, #ast mask, #ast code), where `mask' may also be "
	      "passed as :none in order to indicate the lack of a catch-mask\n"
	      "The flags in this triple is a $\",\"-separated string containing "
	      "zero or more of the following options, with empty options being ignored:\n"
	      "%{table Name|Description\n"
	      "$\"finally\"|The handler is a finally-handler, meaning it always gets executed\n"
	      "$\"interrupt\"|The handler is capable of catching interrupt-exceptions (ignored when $\"finally\" is given)}"),
	  TYPE_METHOD_FKWDS },
	{ "makeloop", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeloop,
	  DOC("(flags:?Dstring,elem:?AAst?Ert:Compiler=!N,iter:?AAst?Ert:Compiler,loop:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(flags:?Dstring,cond:?AAst?Ert:Compiler=!N,next:?AAst?Ert:Compiler=!N,loop:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(flags:?Dint,elem:?AAst?Ert:Compiler=!N,iter:?AAst?Ert:Compiler,loop:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(flags:?Dint,cond:?AAst?Ert:Compiler=!N,next:?AAst?Ert:Compiler=!N,loop:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of one of the given branches or @scope doesn't match @this\n"
	      "@throw ValueError The given @flags contains an unknown flag\n"
	      "@throw ReferenceError One of the given branch is not part of the basescope of the effective @scope\n"
	      "Construct a loop, with the type of loop being determined by @flags, which is a "
	      "$\",\"-separated string containing zero or more of the following options, "
	      "with empty options being ignored:\n"
	      "%{table Name|Description\n"
	      "$\"foreach\"|The loop is a foreach-loop and the first prototype is "
	      "used, with @iter having to be passed as an #ast object. "
	      "Otherwise, the second prototype is used, and any of the "
	      "given branches may be none when omitted\n"
	      "$\"postcond\"|The given @cond is evaluated after the loop (as seen in ${do loop; while (cond);}). "
	      "This flag is ignored in foreach-loops\n"
	      "$\"unlikely\"|The block (@loop) of the loop is unlikely to be reached, and "
	      "should be placed in a section of code that is rarely used}"),
	  TYPE_METHOD_FKWDS },
	{ "makeloopctl", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeloopctl,
	  DOC("(isbreak:?Dbool,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @scope doesn't match @this\n"
	      "Construct a loop control branch, that is either a $continue (when "
	      "@isbreak is :false), or a $break statement (when @isbreak is :true)"),
	  TYPE_METHOD_FKWDS },
	{ "makeconditional", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeconditional,
	  DOC("(cond:?AAst?Ert:Compiler,tt:?AAst?Ert:Compiler=!N,ff:?AAst?Ert:Compiler=!N,flags=!P{},scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(cond:?AAst?Ert:Compiler,tt:?AAst?Ert:Compiler=!N,ff:?AAst?Ert:Compiler=!N,flags=!0,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The given @flags contains an unrecognized, or invalid flag\n"
	      "@throw ValueError The compiler of one of the given branches or @scope doesn't match @this\n"
	      "@throw TypeError Both @tt and @ff have been passed as :none\n"
	      "@throw ReferenceError One of the given branch is not part of the basescope of the effective @scope\n"
	      "Construct a conditional branch for executing @tt or @ff, based on the runtime value of @cond\n"
	      "You may additionally pass @cond for @tt or @ff in order to propagate the value of @cond "
	      "as the result of the conditional branch, when that value is used.\n"
	      "For example, in ${cond ? : ff}, the value of `cond' is propagated when it is :true, "
	      "and replaced with `ff' when not. This is equivalent to ${makeconditional(cond,cond,ff)}, "
	      "but must be noted specifially, as the conditional branch is only evaluated once, meaning "
	      "that any side-effects only happen once, too\n"
	      "Using this knowledge, you could also construct a branch ${makeconditional(cond,tt,cond)}, which simply does the opposite\n"
	      "When @tt is :none, the true-branch returns :none at runtime\n"
	      "When @ff is :none, the false-branch returns :none at runtime\n"
	      "The given @flags is a $\",\"-separated string containing zero or "
	      "more of the following options, with empty options being ignored:\n"
	      "%{table Name|Description\n"
	      "$\"bool\"|The values of @cond, @tt and @ff are cast to a boolean during evaluation. "
	      "Using this, code like ${a || b} results in a branch ${makeconditional(a,a,b,\"bool\")}, "
	      "and ${a && b} results in a branch ${makeconditional(a,b,a,\"bool\")}\n"
	      "$\"likely\"|When given, assembly for @ff is placed in a section of code that is rarely used\n"
	      "$\"unlikely\"|When given, assembly for @tt is placed in a section of code that is rarely used}"),
	  TYPE_METHOD_FKWDS },
	{ "makebool", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makebool,
	  DOC("(expr:?AAst?Ert:Compiler,negate=!f,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @expr or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @expr is not part of the basescope of the effective @scope\n"
	      "Construct a branch for casting @expr to a boolean, optionally inverting the "
	      "underlying boolean logic when @negate is :true\n"
	      "The expression ${!!a} results in ${makebool(a,false)}, while ${!a} results in ${makebool(a,true)}"),
	  TYPE_METHOD_FKWDS },
	{ "makeexpand", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeexpand,
	  DOC("(expr:?AAst?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @expr or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @expr is not part of the basescope of the effective @scope\n"
	      "Construct an expand-branch that will unpack a sequence expression @expr at runtime"),
	  TYPE_METHOD_FKWDS },
	{ "makefunction", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makefunction,
	  DOC("(code:?AAst?Ert:Compiler,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The compiler of @code or @scope doesn't match @this\n"
	      "@throw ReferenceError The effective @scope is not reachable from ${code.scope}\n"
	      "@throw ReferenceError The effective ${scope.base} is identical to ${code.scope.base}\n"
	      "Construct a new lambda-like function that will execute @code\n"
	      "The base-scope of the function is set to ${code.scope.base}, while the returned "
	      "branch will be executed in the context of @scope, or the current scope when :none"),
	  TYPE_METHOD_FKWDS },
	{ "makeoperatorfunc", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeoperatorfunc,
	  DOC("(name:?Dstring,binding:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(name:?Dint,binding:?AAst?Ert:Compiler=!N,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param name The name of the operator, or one of ${[\"+\",\"-\",\"[]\",\"[:]\",\".\"]} "
	      "for ambiguous operators resolved at runtime\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The given @name is not recognized as a valid operator\n"
	      "@throw ValueError The compiler of @binding or @scope doesn't match @this\n"
	      "@throw ReferenceError The given @binding is not part of the same base-scope as the effective @scope\n"
	      "Create a new branch a reference to one of the operator functions, or to "
	      "construct an instance-bound operator function\n"
	      "For example ${operator add} results in ${makeoperatorfunc(\"add\")}, while "
	      "${binding.operator add} results in ${makeoperatorfunc(\"add\",binding)}"),
	  TYPE_METHOD_FKWDS },
	{ "makeoperator", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeoperator,
	  DOC("(name:?Dstring,a:?AAst?Ert:Compiler,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,d:?AAst?Ert:Compiler=!N,flags=!P{},scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(name:?Dint,a:?AAst?Ert:Compiler,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,d:?AAst?Ert:Compiler=!N,flags=!P{},scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(name:?Dstring,a:?AAst?Ert:Compiler,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,d:?AAst?Ert:Compiler=!N,flags=!0,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "(name:?Dint,a:?AAst?Ert:Compiler,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,d:?AAst?Ert:Compiler=!N,flags=!0,scope:?AScope?Ert:Compiler=!N,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?AAst?Ert:Compiler\n"
	      "@param name The name of the operator, or one of ${[\"+\",\"-\",\"[]\",\"[:]\",\".\"]} "
	      "for ambiguous operators resolved based on argument count\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The given @name is not recognized as a valid operator\n"
	      "@throw ValueError The compiler of one of the given branches or @scope doesn't match @this\n"
	      "@throw ReferenceError One of the given branches is not part of the same base-scope as the effective @scope\n"
	      "Create an operator branch for invoking the operator @name with the given branches @a, @b, @c and @d\n"
	      "The given @flags is a $\",\"-separated string containing zero or "
	      "more of the following options, with empty options being ignored:\n"
	      "%{table Flag|Description\n"
	      "$\"post\"|The invocation is assembled as ${ ({ __stack local _res = copy a; a.operator <name> (b[,[c,d]]); _res; }); } "
	      "when the result of the expression is being used. Otherwise, this flag is ignored. "
	      "This is mainly used to implement ${a++}, such that the old value of @a is returned\n"
	      "$\"varargs\"|May only be passed when exactly 2 operands (@a and @b) are given: @b should be "
	      "interpreted as a sequence expression containing the actual operands then used "
	      "to invoke the operator at runtime. This is mainly used to implement operator "
	      "invocations with variable secondary operand counts, as caused by an expand "
	      "expression appearing within a multi-branch argument\n"
	      "$\"maybeprefix\"|Still generate valid assembly for dealing with the problem at runtime when an "
	      "inplace operator is used when the @a operand cannot actually be written to\n"
	      "$\"dontoptimize\"|Don't perform constant optimizations within this branch during the ast-optimization pass}"),
	  TYPE_METHOD_FKWDS },
	{ "makeaction", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeaction,
	  DOC("(name:?Dstring,a:?AAst?Ert:Compiler=!N,b:?AAst?Ert:Compiler=!N,c:?AAst?Ert:Compiler=!N,mustrun=!t,scope:?AScope?Ert:Compiler=!N)->?AAst?Ert:Compiler\n"
	      "@param name The name of the action (see table below)\n"
	      "@param mustrun When :false, ast-optimization may optimize away side-effects caused by action operands. "
	      "Otherwise, all operands are required to execute as required by the action (which usually means executed-in-order)\n"
	      "@param scope The scope to-be used for the new branch, or :none to use #scope\n"
	      "@param loc The location of the ast for DDI, omitted to use the current token position, or :none when not available\n"
	      "@throw ValueError The given @name is not recognized as a valid action\n"
	      "@throw ValueError The compiler of one of the given branches or @scope doesn't match @this\n"
	      "@throw ReferenceError One of the given branches is not part of the same base-scope as the effective @scope\n"
	      "@throw TypeError Too many or too few operand-branches provided for the specified action\n"
	      "Similar to #makeoperator, but instead used to construct action-branches that are then used "
	      "to perform operator-unrelated actions, such as storing an expression into a symbol\n"
	      "The given @name is must be one of the following\n"
	      "%{table Action|Example|Operands|Description\n"
	      "$\"typeof\"|${type a}|1|Returns the type of a given expression\n"
	      "$\"classof\"|${a.class}|1|Returns the class of a given expression (which is the bound type in :super objects)\n"
	      "$\"superof\"|${a.super}|1|Returns a view for the super-class of a given object\n"
	      "$\"print\"|${print a...,;}|1|Print the individual elements of a sequence @a, separated by spaces\n"
	      "$\"println\"|${print a...;}|1|Print the individual elements of a sequence @a, separated by spaces, and followed by a line-feed\n"
	      "$\"fprint\"|${print a: b...,;}|2|Same as $\"print\", but print a sequence @b, and write data to a file @a\n"
	      "$\"fprintln\"|${print a: b...;}|2|Same as $\"println\", but print a sequence @b, and write data to a file @a\n"
	      "$\"range\"|${[a:b,c]}|3|Construct a range expression. Note that @a and @c may evaluate to :none at runtime, in which case the behavior is the same as when omitted in user-code\n"
	      "$\"is\"|${a is b}|2|Check if @a is an instance of @b at runtime, and evaluate to :true or :false\n"
	      "$\"in\"|${a in b}|2|Same as ${b.operator contains(a)}, however operands are evaluated in reverse order\n"
	      "$\"as\"|${a as b}|2|Construct a super-wrapper for @a with a typing of @b\n"
	      "$\"min\"|${a < ...}|1|Evaluate to the lowest element from a sequence in @a, with the side-effect of enumerating @a\n"
	      "$\"max\"|${a > ...}|1|Evaluate to the greatest element from a sequence in @a, with the side-effect of enumerating @a\n"
	      "$\"sum\"|${a + ...}|1|Evaluate to the sum of all element from a sequence in @a, with the side-effect of enumerating @a\n"
	      "$\"any\"|${a || ...}|1|Evaluate to :true if any element from @a evaluates to :true, or :false when @a is empty or has no such elements, with the side-effect of enumerating @a\n"
	      "$\"all\"|${a && ...}|1|Evaluate to :true if all elements from @a evaluate to :true or when @a is empty, or :false otherwise, with the side-effect of enumerating @a\n"
	      "$\"store\"|${a = b}|2|Store the expression in @b into the branch @a (@a may be a #makesym, #makemultiple, or a $\"getitem\", $\"getrange\", or $\"getattr\" #makeoperator branch)\n"
	      "$\"assert\"|${assert(a)} or ${assert(a,b)}|1 or 2|Assert that @a evaluates to :true when cast to a boolean, otherwise throwing an :AssertionError at runtime, alongside an optional message @b. "
	      "When :true and used in an expression, evaluate to the propagated value of @a, such that ${print assert(42);} would output $42 to :file.stdout\n"
	      "$\"boundattr\"|${a.operator . (b) is bound}|2|Evaluate to :true / :false when attribute @b of @a is bound at runtime\n"
	      "$\"sameobj\"|${a === b is bound}|2|Evaluate to :true when @a and @b are the same object at runtime, or :false otherwise\n"
	      "$\"diffobj\"|${a !== b is bound}|2|Evaluate to :true when @a and @b are different objects at runtime, or :false otherwise\n"
	      "$\"callkw\"|${a(b...,**c)}|3|Perform a call to @a, using positional arguments from @b, and a keyword list from @c}"),
	  TYPE_METHOD_FKWDS },
	{ "makeclass", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeclass,
	  DOC("TODO"),
	  TYPE_METHOD_FKWDS },
	{ "makelabel", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makelabel,
	  DOC("TODO"),
	  TYPE_METHOD_FKWDS },
	{ "makegoto", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makegoto,
	  DOC("TODO"),
	  TYPE_METHOD_FKWDS },
	{ "makeswitch", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeswitch,
	  DOC("TODO"),
	  TYPE_METHOD_FKWDS },
	{ "makeassembly", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & ast_makeassembly,
	  DOC("TODO"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};

INTERN struct type_member compiler_class_members[] = {
	TYPE_MEMBER_CONST("Lexer", &DeeCompilerLexer_Type),
	TYPE_MEMBER_CONST("Parser", &DeeCompilerParser_Type),
	TYPE_MEMBER_CONST("Symbol", &DeeCompilerSymbol_Type),
	TYPE_MEMBER_CONST("Ast", &DeeCompilerAst_Type),
	TYPE_MEMBER_CONST("Scope", &DeeCompilerScope_Type),
	TYPE_MEMBER_CONST("BaseScope", &DeeCompilerBaseScope_Type),
	TYPE_MEMBER_CONST("RootScope", &DeeCompilerRootScope_Type),
	TYPE_MEMBER_END
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C */
