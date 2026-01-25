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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_Free, Dee_Reallocc, Dee_TryReallocc */
#include <deemon/arg.h>                /* DeeArg_UnpackStructKw */
#include <deemon/code.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/error.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/hashset.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>    /* strend() */
#include <deemon/tuple.h>
#include <deemon/util/cache.h>         /* DECLARE_OBJECT_CACHE, DECLARE_STRUCT_CACHE */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_cinit */

#include "../../runtime/kwlist.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* int32_t, uint8_t, uint16_t */

DECL_BEGIN

#ifdef CONFIG_AST_IS_STRUCT
DECLARE_STRUCT_CACHE(ast, struct ast)
#else /* CONFIG_AST_IS_STRUCT */
DECLARE_OBJECT_CACHE(ast, struct ast)
#endif /* !CONFIG_AST_IS_STRUCT */


INTERN WUNUSED NONNULL((1)) int DCALL
compiler_init(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject *const *argv,
              DeeObject *kw) {
	/* TODO: All those other arguments, like compiler options, etc. */
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("Compiler", params: """
	DeeObject *module:?X2?N?Dstring?DModule = Dee_None;
");]]]*/
	struct {
		DeeObject *module_;
	} args;
	args.module_ = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__module, "|o:Compiler", &args))
		goto err;
/*[[[end]]]*/
	if (DeeNone_Check(args.module_)) {
		args.module_ = (DeeObject *)DeeModule_New(Dee_EmptyString);
		if unlikely(!args.module_)
			goto err;
	} else if (DeeString_Check(args.module_)) {
		args.module_ = (DeeObject *)DeeModule_New(args.module_);
		if unlikely(!args.module_)
			goto err;
	} else {
		Dee_Incref(args.module_);
	}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Create the new root scope object. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	(void)argc; /* XXX: Arguments? */
	(void)argv;
	(void)kw;
	self->cp_scope = (DREF DeeScopeObject *)DeeObject_NewDefault(&DeeRootScope_Type);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	self->cp_scope = (DREF DeeScopeObject *)DeeObject_New(&DeeRootScope_Type, 1,
	                                                      (DeeObject **)&args.module_);
	Dee_Decref(args.module_);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if unlikely(!self->cp_scope)
		goto err;
	Dee_weakref_support_init(self);
	bzero(&self->cp_tags, sizeof(self->cp_tags));
	bzero(&self->cp_items, sizeof(self->cp_items));
	Dee_atomic_rwlock_cinit(&self->cp_items.cis_lock);
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
	if unlikely(!TPPLexer_Init(&self->cp_lexer))
		goto err_scope;
#ifdef CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
	/* Mirror MSVC's file-and-line syntax. */
	self->cp_lexer.l_flags |= TPPLEXER_FLAG_MSVC_MESSAGEFORMAT;
#endif /* CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC */
	self->cp_lexer.l_extokens = TPPLEXER_TOKEN_LANG_DEEMON;
	parser_errors_init(&self->cp_errors);
	return 0;
err_scope:
	Dee_Decref(self->cp_scope);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
compiler_get_lexer(DeeCompilerObject *__restrict self) {
	return DeeCompiler_GetLexer(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
compiler_get_parser(DeeCompilerObject *__restrict self) {
	return DeeCompiler_GetParser(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
compiler_get_scope(DeeCompilerObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self))
		goto err;
	result = DeeCompiler_GetScope(current_scope);
	COMPILER_END();
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
compiler_set_scope(DeeCompilerObject *__restrict self,
                   DeeCompilerScopeObject *__restrict value) {
	int result = 0;
	if (DeeObject_AssertType(value, &DeeCompilerScope_Type))
		goto err;
	if (value->ci_compiler != self)
		return err_invalid_scope_compiler(value);
	if (COMPILER_BEGIN(self))
		goto err;
	if (value->ci_value->s_base != current_basescope) {
		result = err_different_base_scope();
	} else {
		Dee_Incref(value->ci_value);
		Dee_Decref(current_scope);
		current_scope = value->ci_value;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
compiler_get_basescope(DeeCompilerObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self))
		goto err;
	result = DeeCompiler_GetScope((DeeScopeObject *)current_basescope);
	COMPILER_END();
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
compiler_set_basescope(DeeCompilerObject *__restrict self,
                       DeeCompilerScopeObject *__restrict value) {
	int result = 0;
	if (DeeObject_AssertType(value, &DeeCompilerBaseScope_Type))
		goto err;
	if (value->ci_compiler != self)
		return err_invalid_scope_compiler(value);
	if (COMPILER_BEGIN(self))
		goto err;
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
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
compiler_get_rootscope(DeeCompilerObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self))
		goto err;
	result = DeeCompiler_GetObjItem(&DeeCompilerRootScope_Type,
	                                Dee_AsObject(&current_rootscope->rs_scope.bs_scope));
	COMPILER_END();
	return result;
err:
	return NULL;
}

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
compiler_get_module(DeeCompilerObject *__restrict self) {
	DREF DeeModuleObject *result;
	if (COMPILER_BEGIN(self))
		goto err;
	result = current_rootscope->rs_module;
	Dee_Incref(result);
	COMPILER_END();
	return Dee_AsObject(result);
err:
	return NULL;
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


INTDEF struct type_getset tpconst compiler_getsets[];
INTERN_TPCONST struct type_getset tpconst compiler_getsets[] = {
	TYPE_GETTER("lexer", &compiler_get_lexer,
	            "->?#Lexer\n"
	            "Returns the lexer (tokenizer) of @this compiler"),
	TYPE_GETTER("parser", &compiler_get_parser,
	            "->?#Parser\n"
	            "Returns the parser (token to ast converter) of @this compiler"),
	TYPE_GETSET("scope", &compiler_get_scope, NULL, &compiler_set_scope,
	            "->?#Scope\n"
	            "#tValueError{Attempted to set a scope who's compiler doesn't match @this}"
	            "#tReferenceError{Attempted to set a scope not apart of the same base-scope as ?#basescope}"
	            "Get or set the current scope used for parsing new ?#{ast}s"),
	TYPE_GETSET("basescope", &compiler_get_basescope, NULL, &compiler_set_basescope,
	            "->?#BaseScope\n"
	            "#tValueError{Attempted to set a scope who's compiler doesn't match @this}"
	            "#tReferenceError{Attempted to set a scope not apart of the same root-scope as ?#rootscope}"
	            "Get or set the current base-scope, representing the current function-context\n"
	            "When setting the base-scope, ?#scope is set to the same scope"),
	TYPE_GETTER("rootscope", &compiler_get_rootscope,
	            "->?#RootScope\n"
	            "Get the root-scope active within @this compiler\n"
	            "Note that this scope is fixed and cannot be changed"),
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	TYPE_GETTER(STR_module, &compiler_get_module,
	            "->?DModule\n"
	            "Returns the module being constructed by @this compiler\n"
	            "Warning: The returned module is incomplete and uninitialized, "
	            "and can't actually be used, yet"),
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	TYPE_GETSET_END
};


#ifndef NDEBUG
#define ast_new(scope, loc) ast_dbgnew(scope, loc, __FILE__, __LINE__)
PRIVATE WUNUSED DREF struct ast *DCALL
ast_dbgnew(DeeScopeObject *__restrict scope,
           DeeObject *loc, char const *file, int line)
#else /* !NDEBUG */
PRIVATE WUNUSED DREF struct ast *DCALL
ast_new(DeeScopeObject *__restrict scope, DeeObject *loc)
#endif /* NDEBUG */
{
	DREF struct ast *result;
#ifndef CONFIG_NO_THREADS
	ASSERT(DeeCompiler_LockReading());
#endif /* !CONFIG_NO_THREADS */
#ifndef NDEBUG
	result = ast_dbgalloc(file, line);
#else /* !NDEBUG */
	result = ast_alloc();
#endif /* NDEBUG */
	if likely(result) {
		if unlikely(set_astloc_from_obj(loc, result)) {
			ast_free(result);
			result = NULL;
		} else {
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
	if (DeeObject_AssertType(scope, &DeeCompilerScope_Type))
		goto err;
	if unlikely(scope->ci_compiler != DeeCompiler_Current) {
		err_invalid_scope_compiler(scope);
		goto err;
	}
	return scope->ci_value;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeconstexpr(DeeCompilerObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeconstexpr", params: """
	DeeObject *value;
	DeeCompilerScopeObject *scope: ?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeconstexpr_params "value,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeObject *value;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value_scope_loc, "o|oo:makeconstexpr", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	ast_scope = get_scope(args.scope);
	if unlikely(!ast_scope)
		goto done_compiler_end;
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type      = AST_CONSTEXPR;
	result_ast->a_constexpr = args.value;
	Dee_Incref(args.value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makesym(DeeCompilerObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makesym", params: """
	DeeCompilerSymbolObject *sym:?#Symbol;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makesym_params "sym:?#Symbol,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerSymbolObject *sym;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__sym_scope_loc, "o|oo:makesym", &args))
		goto done;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.sym, &DeeCompilerSymbol_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely(args.sym->ci_compiler != self) {
		err_invalid_symbol_compiler(args.sym);
		goto done_compiler_end;
	}
	if unlikely(!args.sym->ci_value) {
		err_compiler_item_deleted((DeeCompilerItemObject *)args.sym);
		goto done_compiler_end;
	}
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(!scope_reaches_symbol(ast_scope, args.sym->ci_value)) {
		err_symbol_not_reachable(ast_scope, args.sym->ci_value);
		goto done_compiler_end;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type = AST_SYM;
	result_ast->a_flag = AST_FNORMAL;
	result_ast->a_sym  = args.sym->ci_value;
	SYMBOL_INC_NREAD(args.sym->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeunbind(DeeCompilerObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeunbind", params: """
	DeeCompilerSymbolObject *sym:?#Symbol;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeunbind_params "sym:?#Symbol,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerSymbolObject *sym;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__sym_scope_loc, "o|oo:makeunbind", &args))
		goto done;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.sym, &DeeCompilerSymbol_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely(args.sym->ci_compiler != self) {
		err_invalid_symbol_compiler(args.sym);
		goto done_compiler_end;
	}
	if unlikely(!args.sym->ci_value) {
		err_compiler_item_deleted((DeeCompilerItemObject *)args.sym);
		goto done_compiler_end;
	}
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(!scope_reaches_symbol(ast_scope, args.sym->ci_value)) {
		err_symbol_not_reachable(ast_scope, args.sym->ci_value);
		goto done_compiler_end;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type   = AST_UNBIND;
	result_ast->a_unbind = args.sym->ci_value;
	SYMBOL_INC_NWRITE(args.sym->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makebound(DeeCompilerObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makebound", params: """
	DeeCompilerSymbolObject *sym:?#Symbol;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makebound_params "sym:?#Symbol,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerSymbolObject *sym;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__sym_scope_loc, "o|oo:makebound", &args))
		goto done;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.sym, &DeeCompilerSymbol_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely(args.sym->ci_compiler != self) {
		err_invalid_symbol_compiler(args.sym);
		goto done_compiler_end;
	}
	if unlikely(!args.sym->ci_value) {
		err_compiler_item_deleted((DeeCompilerItemObject *)args.sym);
		goto done_compiler_end;
	}
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(!scope_reaches_symbol(ast_scope, args.sym->ci_value)) {
		err_symbol_not_reachable(ast_scope, args.sym->ci_value);
		goto done_compiler_end;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type   = AST_BOUND;
	result_ast->a_unbind = args.sym->ci_value;
	SYMBOL_INC_NBOUND(args.sym->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

/* For AST_MULTIPLE: Return the flags for constructing a sequence for `typing'
 * NOTE: `typing' doesn't necessarily need to be a type object!
 * @return: (uint16_t)-1: Error. */
INTERN WUNUSED NONNULL((1)) uint16_t DCALL
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
		result = AST_FMULTIPLE_GENERIC_MAP;
	} else {
		DeeError_Throwf(&DeeError_TypeError,
		                "Invalid multi-branch typing: %k",
		                typing);
		result = (uint16_t)-1;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makemultiple(DeeCompilerObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	uint16_t flags;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
	DREF DeeCompilerAstObject **branch_v;
	size_t i, branch_c;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makemultiple", params: """
	DeeObject *branches:?S?#Ast;
	DeeTypeObject *typing:?DType=!N = (DeeTypeObject *)Dee_None;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makemultiple_params "branches:?S?#Ast,typing:?DType=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeObject *branches;
		DeeTypeObject *typing;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.typing = (DeeTypeObject *)Dee_None;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__branches_typing_scope_loc, "o|ooo:makemultiple", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	flags = get_ast_multiple_typing(args.typing);
	if unlikely(flags == (uint16_t)-1)
		goto done_compiler_end;
	branch_v = (DREF DeeCompilerAstObject **)DeeSeq_AsHeapVector(args.branches, &branch_c);
	if unlikely(!branch_v)
		goto done_compiler_end;
#ifdef CONFIG_AST_IS_STRUCT
#error "This loop doesn't work when asts are structs"
#endif /* CONFIG_AST_IS_STRUCT */
	for (i = 0; i < branch_c; ++i) {
		struct ast *branch_ast;
		/* Load the ast objects from each of the branches. */
		if (DeeObject_AssertTypeExact(branch_v[i], &DeeCompilerAst_Type))
			goto err_branch_v;
		if unlikely(branch_v[i]->ci_compiler != self) {
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
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto err_branch_v;
	result_ast->a_type            = AST_MULTIPLE;
	result_ast->a_flag            = flags;
	result_ast->a_multiple.m_astc = branch_c;
	result_ast->a_multiple.m_astv = (DREF struct ast **)branch_v;
	result                        = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
	__IF0 {
err_branch_v:
		Dee_Decrefv(branch_v, branch_c);
		Dee_Free(branch_v);
	}
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makereturn(DeeCompilerObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makereturn", params: """
	DeeCompilerAstObject *expr:?#Ast=!N       = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerScopeObject *scope:?#Scope=!N  = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makereturn_params "expr:?#Ast=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *expr;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.expr = (DeeCompilerAstObject *)Dee_None;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__expr_scope_loc, "|ooo:makereturn", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if (!DeeNone_Check(args.expr)) {
		if (DeeObject_AssertTypeExact(args.expr, &DeeCompilerSymbol_Type))
			goto done_compiler_end;
		if unlikely(args.expr->ci_compiler != self) {
			err_invalid_ast_compiler(args.expr);
			goto done_compiler_end;
		}
		if unlikely(args.expr->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.expr, ast_scope->s_base);
			goto done_compiler_end;
		}
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type   = AST_RETURN;
	result_ast->a_return = NULL;
	if (!DeeNone_Check(args.expr)) {
		result_ast->a_return = args.expr->ci_value;
		ast_incref(args.expr->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeyield(DeeCompilerObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeyield", params: """
	DeeCompilerAstObject *expr:?#Ast;
	DeeCompilerScopeObject *scope:?#Scope=!N  = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeyield_params "expr:?#Ast,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *expr;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__expr_scope_loc, "o|oo:makeyield", &args))
		goto done;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.expr, &DeeCompilerSymbol_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(args.expr->ci_compiler != self) {
		err_invalid_ast_compiler(args.expr);
		goto done_compiler_end;
	}
	if unlikely(args.expr->ci_value->a_scope->s_base != ast_scope->s_base) {
		err_invalid_ast_basescope(args.expr, ast_scope->s_base);
		goto done_compiler_end;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type  = AST_YIELD;
	result_ast->a_throw = args.expr->ci_value;
	ast_incref(args.expr->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makethrow(DeeCompilerObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makethrow", params: """
	DeeCompilerAstObject *expr:?#Ast=!N       = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerScopeObject *scope:?#Scope=!N  = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makethrow_params "expr:?#Ast=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *expr;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.expr = (DeeCompilerAstObject *)Dee_None;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__expr_scope_loc, "|ooo:makethrow", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if (!DeeNone_Check(args.expr)) {
		if (DeeObject_AssertTypeExact(args.expr, &DeeCompilerSymbol_Type))
			goto done_compiler_end;
		if unlikely(args.expr->ci_compiler != self) {
			err_invalid_ast_compiler(args.expr);
			goto done_compiler_end;
		}
		if unlikely(args.expr->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.expr, ast_scope->s_base);
			goto done_compiler_end;
		}
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type  = AST_THROW;
	result_ast->a_throw = NULL;
	if (!DeeNone_Check(args.expr)) {
		result_ast->a_throw = args.expr->ci_value;
		ast_incref(args.expr->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
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
		/* TODO: Strip leading/trailing spaces! */
		if (flag_length) {
#define IS_FLAG(x) (flag_length == COMPILER_STRLEN(x) && bcmpc(flags, x, COMPILER_STRLEN(x), sizeof(char)) == 0)
			if (IS_FLAG("finally")) {
				result->ce_flags |= Dee_EXCEPTION_HANDLER_FFINALLY;
			} else if (IS_FLAG("interrupt")) {
				result->ce_flags |= Dee_EXCEPTION_HANDLER_FINTERPT;
			} else {
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


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
unpack_catch_expression(DeeObject *__restrict triple,
                        struct catch_expr *__restrict result,
                        DeeBaseScopeObject *__restrict base_scope) {
	/* (flags:?Dstring,mask:?#Ast,code:?#Ast) */
	DeeCompilerAstObject *args[3];
	if (DeeSeq_Unpack(triple, 3, (DeeObject **)args))
		goto err;
	if (!DeeNone_Check(args[1])) {
		if (DeeObject_AssertTypeExact(args[1], &DeeCompilerAst_Type))
			goto err;
		if unlikely(args[1]->ci_compiler != DeeCompiler_Current) {
			err_invalid_ast_compiler(args[1]);
			goto err;
		}
		if unlikely(args[1]->ci_value->a_scope->s_base != base_scope) {
			err_invalid_ast_basescope(args[1], base_scope);
			goto err;
		}
	}
	if (DeeObject_AssertTypeExact(args[2], &DeeCompilerAst_Type))
		goto err;
	if unlikely(args[2]->ci_compiler != DeeCompiler_Current) {
		err_invalid_ast_compiler(args[2]);
		goto err;
	}
	if unlikely(args[2]->ci_value->a_scope->s_base != base_scope) {
		err_invalid_ast_basescope(args[2], base_scope);
		goto err;
	}
	/* Parse flags. */
	result->ce_mode  = CATCH_EXPR_FNORMAL;
	result->ce_flags = Dee_EXCEPTION_HANDLER_FNORMAL;
	if (DeeString_Check(args[0])) {
		if unlikely(parse_handler_flags(DeeString_STR(args[0]), result))
			goto err;
	} else {
		if (DeeObject_AsUInt16(Dee_AsObject(args[0]), &result->ce_flags))
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


struct unpack_catch_expressions_foreach_data {
	DeeBaseScopeObject *ucef_bscope; /* [1..1][const] Base scope */
	struct catch_expr  *ucef_v;      /* [0..ucef_c|ALLOC(ucef_a)] Output buffer */
	size_t              ucef_c;      /* Used count */
	size_t              ucef_a;      /* Allocated count */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
unpack_catch_expressions_foreach_cb(void *arg, DeeObject *item) {
	struct unpack_catch_expressions_foreach_data *data;
	data = (struct unpack_catch_expressions_foreach_data *)arg;
	ASSERT(data->ucef_c <= data->ucef_a);
	if (data->ucef_c >= data->ucef_a) {
		size_t new_alloc = data->ucef_a * 2;
		struct catch_expr *new_vectr;
		if unlikely(!new_alloc)
			new_alloc = 2;
		new_vectr = (struct catch_expr *)Dee_TryReallocc(data->ucef_v, new_alloc,
		                                                 sizeof(struct catch_expr));
		if unlikely(!new_vectr) {
			new_alloc = data->ucef_c + 1;
			new_vectr = (struct catch_expr *)Dee_Reallocc(data->ucef_v, new_alloc,
			                                                sizeof(struct catch_expr));
			if unlikely(!new_vectr)
				goto err;
		}
		data->ucef_v = new_vectr;
		data->ucef_a = new_alloc;
	}
	if unlikely(unpack_catch_expression(item, &data->ucef_v[data->ucef_c],
	                                    data->ucef_bscope))
		goto err;
	++data->ucef_c;
	return 0;
err:
	return -1;
}

/* Unpack and validate a sequence `{(string, ast, ast)...} handlers'.
 * @return: NULL: Error (*p_catch_c != 0), or no catch handlers (*p_catch_c == 0) */
INTERN WUNUSED NONNULL((1, 2, 3)) struct catch_expr *DCALL
unpack_catch_expressions(DeeObject *__restrict handlers,
                         size_t *__restrict p_catch_c,
                         DeeBaseScopeObject *__restrict base_scope) {
	struct unpack_catch_expressions_foreach_data data;
	data.ucef_bscope = base_scope;
	data.ucef_c = 0;
	data.ucef_a = 0;
	data.ucef_v = NULL;
	if (DeeObject_Foreach(handlers, &unpack_catch_expressions_foreach_cb, &data))
		goto err_catch;

	/* Release unused memory. */
	ASSERT(data.ucef_c <= data.ucef_a);
	if (data.ucef_c < data.ucef_a) {
		struct catch_expr *new_vectr;
		new_vectr = (struct catch_expr *)Dee_TryReallocc(data.ucef_v, data.ucef_c,
		                                                 sizeof(struct catch_expr));
		if likely(new_vectr)
			data.ucef_v = new_vectr;
	}
done:
	*p_catch_c = data.ucef_c;
	return data.ucef_v;
err_catch:
	while (data.ucef_c--) {
		ast_xdecref(data.ucef_v[data.ucef_c].ce_mask);
		ast_decref(data.ucef_v[data.ucef_c].ce_code);
	}
	Dee_Free(data.ucef_v);
	data.ucef_v = NULL;
	data.ucef_c = 1; /* Error indicator */
	goto done;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_maketry(DeeCompilerObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("maketry", params: """
	DeeCompilerAstObject *guard:?#Ast;
	DeeObject *handlers:?S?T3?X2?Dstring?Dint?#Ast?#Ast;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_maketry_params "guard:?#Ast,handlers:?S?T3?X2?Dstring?Dint?#Ast?#Ast,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *guard;
		DeeObject *handlers;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__guard_handlers_scope_loc, "oo|oo:maketry", &args))
		goto done;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.guard, &DeeCompilerSymbol_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(args.guard->ci_compiler != self) {
		err_invalid_ast_compiler(args.guard);
		goto done_compiler_end;
	}
	if unlikely(args.guard->ci_value->a_scope->s_base != ast_scope->s_base) {
		err_invalid_ast_basescope(args.guard, ast_scope->s_base);
		goto done_compiler_end;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	/* Unpack the given handler expressions vector. */
	result_ast->a_try.t_catchv = unpack_catch_expressions(args.handlers,
	                                                      &result_ast->a_try.t_catchc,
	                                                      ast_scope->s_base);
	if unlikely(!result_ast->a_try.t_catchv && result_ast->a_try.t_catchc) {
		Dee_DecrefNokill(ast_scope);
		Dee_DecrefNokill(&DeeAst_Type);
		ast_free(result_ast);
		goto done_compiler_end;
	}
	result_ast->a_type        = AST_TRY;
	result_ast->a_try.t_guard = args.guard->ci_value;
	ast_incref(args.guard->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}


/* Parse the flags for a loop-ast from a string (:rt:Compiler.makeloop) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
parse_loop_flags(char const *__restrict flags,
                 uint16_t *__restrict p_result) {
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
		/* TODO: Strip leading/trailing spaces! */
		if (flag_length) {
#define IS_FLAG(x) (flag_length == COMPILER_STRLEN(x) && bcmpc(flags, x, COMPILER_STRLEN(x), sizeof(char)) == 0)
			if (IS_FLAG("foreach")) {
				*p_result |= AST_FLOOP_FOREACH;
			} else if (IS_FLAG("postcond")) {
				*p_result |= AST_FLOOP_POSTCOND;
			} else if (IS_FLAG("unlikely")) {
				*p_result |= AST_FLOOP_UNLIKELY;
			} else {
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



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeloop(DeeCompilerObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result        = NULL;
	uint16_t flags                = 0;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
	struct {
		DeeCompilerAstObject *cond;
		DeeCompilerAstObject *next;
		DeeCompilerAstObject *loop;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.cond  = (DeeCompilerAstObject *)Dee_None;
	args.next  = (DeeCompilerAstObject *)Dee_None;
	args.loop  = (DeeCompilerAstObject *)Dee_None;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc   = NULL;
	/* "(flags:?Dstring,elem:?#Ast=!N,iter:?#Ast,loop:?#Ast=!N,scope:?#Scope=!N)->?#Ast\n"
	 * "(flags:?Dstring,cond:?#Ast=!N,next:?#Ast=!N,loop:?#Ast=!N,scope:?#Scope=!N)->?#Ast\n" */
	if unlikely(!argc) {
		err_invalid_argc("makeloop", argc, 1, 5);
		goto done;
	}
	if (DeeString_Check(argv[0])) {
		if unlikely(DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
			goto done;
		if unlikely(parse_loop_flags(DeeString_STR(argv[0]), &flags))
			goto done;
	} else {
		if unlikely(DeeObject_AsUInt16(argv[0], &flags))
			goto done;
	}
	--argc;
	++argv;
	if (COMPILER_BEGIN(self))
		goto done;
	if (flags & AST_FLOOP_FOREACH) {
		if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__elem_iter_loop_scope_loc,
		                          "|ooooo:makeloop", &args))
			goto done_compiler_end;
		if unlikely((ast_scope = get_scope(args.scope)) == NULL)
			goto done_compiler_end;
check_next:
		/* The next (iter) operand is mandatory in foreach loop branches. */
		if unlikely(DeeObject_AssertTypeExact(args.next, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.next->ci_compiler != DeeCompiler_Current) {
			err_invalid_ast_compiler(args.next);
			goto done_compiler_end;
		}
		if unlikely(args.next->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.next, ast_scope->s_base);
			goto done_compiler_end;
		}
	} else {
		if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__cond_next_loop_scope_loc,
		                          "|ooooo:makeloop", &args))
			goto done_compiler_end;
		if unlikely((ast_scope = get_scope(args.scope)) == NULL)
			goto done_compiler_end;
		if (!DeeNone_Check(args.next))
			goto check_next;
	}
	if (!DeeNone_Check(args.cond)) {
		if unlikely(DeeObject_AssertTypeExact(args.cond, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.cond->ci_compiler != DeeCompiler_Current) {
			err_invalid_ast_compiler(args.cond);
			goto done_compiler_end;
		}
		if unlikely(args.cond->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.cond, ast_scope->s_base);
			goto done_compiler_end;
		}
	}
	if (!DeeNone_Check(args.loop)) {
		if unlikely(DeeObject_AssertTypeExact(args.loop, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.loop->ci_compiler != DeeCompiler_Current) {
			err_invalid_ast_compiler(args.loop);
			goto done_compiler_end;
		}
		if unlikely(args.loop->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.loop, ast_scope->s_base);
			goto done_compiler_end;
		}
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type        = AST_LOOP;
	result_ast->a_flag        = flags;
	result_ast->a_loop.l_cond = NULL;
	result_ast->a_loop.l_next = NULL;
	result_ast->a_loop.l_loop = NULL;
	if (!DeeNone_Check(args.cond)) {
		result_ast->a_loop.l_cond = args.cond->ci_value;
		ast_incref(args.cond->ci_value);
	}
	if (!DeeNone_Check(args.next)) {
		result_ast->a_loop.l_next = args.next->ci_value;
		ast_incref(args.next->ci_value);
	}
	if (!DeeNone_Check(args.loop)) {
		result_ast->a_loop.l_loop = args.loop->ci_value;
		ast_incref(args.loop->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeloopctl(DeeCompilerObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeloopctl", params: """
	bool isbreak;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeloopctl_params "isbreak:?Dbool,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		bool isbreak;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__isbreak_scope_loc, "b|oo:makeloopctl", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type = AST_LOOPCTL;
	result_ast->a_flag = args.isbreak ? AST_FLOOPCTL_BRK : AST_FLOOPCTL_CON;
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}


/* Parse the flags for a conditional-ast from a string (:rt:Compiler.makeconditional) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
parse_conditional_flags(char const *__restrict flags,
                        uint16_t *__restrict p_result) {
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
		/* TODO: Strip leading/trailing spaces! */
		if (flag_length) {
#define IS_FLAG_S(len, s) (flag_length == (len) && bcmpc(flags, s, len, sizeof(char)) == 0)
			if (IS_FLAG_S(4, STR_bool)) {
				*p_result |= AST_FCOND_BOOL;
			} else if (IS_FLAG_S(6, "likely")) {
				*p_result |= AST_FCOND_LIKELY;
			} else if (IS_FLAG_S(8, "unlikely")) {
				*p_result |= AST_FCOND_UNLIKELY;
			} else {
				return DeeError_Throwf(&DeeError_ValueError,
				                       "Unknown conditional flag %$q",
				                       flag_length,
				                       flags);
			}
#undef IS_FLAG_S
		}
		flags = next_flag;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeconditional(DeeCompilerObject *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	uint16_t flags         = 0;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeconditional", params: """
	DeeCompilerAstObject *cond:?#Ast;
	DeeCompilerAstObject *tt:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *ff:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeStringObject *flags:?X2?Dstring?Dint=!P{} = (DeeStringObject *)Dee_EmptyString;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeconditional_params "cond:?#Ast,tt:?#Ast=!N,ff:?#Ast=!N,flags:?X2?Dstring?Dint=!P{},scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *cond;
		DeeCompilerAstObject *tt;
		DeeCompilerAstObject *ff;
		DeeStringObject *flags;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.tt = (DeeCompilerAstObject *)Dee_None;
	args.ff = (DeeCompilerAstObject *)Dee_None;
	args.flags = (DeeStringObject *)Dee_EmptyString;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__cond_tt_ff_flags_scope_loc, "o|ooooo:makeconditional", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if (args.flags != (DeeStringObject *)Dee_EmptyString) {
		if (DeeString_Check(args.flags)) {
			if (parse_conditional_flags(DeeString_STR(args.flags), &flags))
				goto done_compiler_end;
		} else {
			if (DeeObject_AsUInt16(Dee_AsObject(args.flags), &flags))
				goto done_compiler_end;
		}
	}
	if unlikely(DeeObject_AssertTypeExact(args.cond, &DeeCompilerAst_Type))
		goto done_compiler_end;
	if unlikely(args.cond->ci_compiler != self) {
		err_invalid_ast_compiler(args.cond);
		goto done_compiler_end;
	}
	if unlikely(args.cond->ci_value->a_scope->s_base != ast_scope->s_base) {
		err_invalid_ast_basescope(args.cond, ast_scope->s_base);
		goto done_compiler_end;
	}
	if (!DeeNone_Check(args.tt)) {
		if unlikely(DeeObject_AssertTypeExact(args.tt, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.tt->ci_compiler != self) {
			err_invalid_ast_compiler(args.tt);
			goto done_compiler_end;
		}
		if unlikely(args.tt->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.tt, ast_scope->s_base);
			goto done_compiler_end;
		}
	} else if unlikely(DeeNone_Check(args.ff)) {
		DeeError_Throwf(&DeeError_TypeError,
		                "Both the true-, as well as the false-branch have been given as `none'");
		goto done_compiler_end;
	}
	if (!DeeNone_Check(args.ff)) {
		if unlikely(DeeObject_AssertTypeExact(args.ff, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.ff->ci_compiler != self) {
			err_invalid_ast_compiler(args.ff);
			goto done_compiler_end;
		}
		if unlikely(args.ff->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.ff, ast_scope->s_base);
			goto done_compiler_end;
		}
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type               = AST_CONDITIONAL;
	result_ast->a_flag               = flags;
	result_ast->a_conditional.c_cond = args.cond->ci_value;
	result_ast->a_conditional.c_tt   = NULL;
	result_ast->a_conditional.c_ff   = NULL;
	ast_incref(args.cond->ci_value);
	if (!DeeNone_Check(args.tt)) {
		result_ast->a_conditional.c_tt = args.tt->ci_value;
		ast_incref(args.tt->ci_value);
	}
	if (!DeeNone_Check(args.ff)) {
		result_ast->a_conditional.c_ff = args.ff->ci_value;
		ast_incref(args.ff->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makebool(DeeCompilerObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makebool", params: """
	DeeCompilerAstObject *expr:?#Ast;
	bool negate                               = false;
	DeeCompilerScopeObject *scope:?#Scope=!N  = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makebool_params "expr:?#Ast,negate=!f,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *expr;
		bool negate;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.negate = false;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__expr_negate_scope_loc, "o|boo:makebool", &args))
		goto done;
/*[[[end]]]*/
	if unlikely(DeeObject_AssertTypeExact(args.expr, &DeeCompilerAst_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(args.expr->ci_compiler != self) {
		err_invalid_ast_compiler(args.expr);
		goto done_compiler_end;
	}
	if unlikely(args.expr->ci_value->a_scope->s_base != ast_scope->s_base) {
		err_invalid_ast_basescope(args.expr, ast_scope->s_base);
		goto done_compiler_end;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type = AST_BOOL;
	result_ast->a_flag = args.negate ? AST_FBOOL_NEGATE : AST_FBOOL_NORMAL;
	result_ast->a_bool = args.expr->ci_value;
	ast_incref(args.expr->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeexpand(DeeCompilerObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeexpand", params: """
	DeeCompilerAstObject *expr:?#Ast;
	DeeCompilerScopeObject *scope:?#Scope=!N  = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeexpand_params "expr:?#Ast,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *expr;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__expr_scope_loc, "o|oo:makeexpand", &args))
		goto done;
/*[[[end]]]*/
	if unlikely(DeeObject_AssertTypeExact(args.expr, &DeeCompilerAst_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(args.expr->ci_compiler != self) {
		err_invalid_ast_compiler(args.expr);
		goto done_compiler_end;
	}
	if unlikely(args.expr->ci_value->a_scope->s_base != ast_scope->s_base) {
		err_invalid_ast_basescope(args.expr, ast_scope->s_base);
		goto done_compiler_end;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type = AST_EXPAND;
	result_ast->a_bool = args.expr->ci_value;
	ast_incref(args.expr->ci_value);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
check_function_code_scope(DeeBaseScopeObject *code_scope,
                          DeeBaseScopeObject *ast_base_scope) {
	if unlikely(code_scope == ast_base_scope) {
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
		if unlikely(!code_scope) {
			return DeeError_Throwf(&DeeError_ReferenceError,
			                       "Function initializer scope is not "
			                       "reachable from function code");
		}
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makefunction(DeeCompilerObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
	DeeBaseScopeObject *code_scope;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makefunction", params: """
	DeeCompilerAstObject *code:?#Ast;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makefunction_params "code:?#Ast,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeCompilerAstObject *code;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__code_scope_loc, "o|oo:makefunction", &args))
		goto done;
/*[[[end]]]*/
	if unlikely(DeeObject_AssertTypeExact(args.code, &DeeCompilerAst_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(args.code->ci_compiler != self) {
		err_invalid_ast_compiler(args.code);
		goto done_compiler_end;
	}
	code_scope = args.code->ci_value->a_scope->s_base;
	if unlikely(check_function_code_scope(code_scope, ast_scope->s_base))
		goto done_compiler_end;
	/* Setup a new function branch. */
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type             = AST_FUNCTION;
	result_ast->a_function.f_code  = args.code->ci_value;
	result_ast->a_function.f_scope = code_scope;
	ast_incref(args.code->ci_value);
	Dee_Incref(&code_scope->bs_scope);
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

/* Parse the operator name and determine its ID. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
get_operator_id(DeeObject *__restrict opid, Dee_operator_t *__restrict p_result) {
	if (DeeString_Check(opid)) {
		char const *name = DeeString_STR(opid);
		struct Dee_opinfo const *info;
		info = DeeTypeType_GetOperatorByName(&DeeType_Type, name, (size_t)-1);
		if (info != NULL) {
			*p_result = info->oi_id;
			return 0;
		}
		/* Resolve special operator names. */
		switch (name[0]) {

		case '+':
			if (name[1])
				goto unknown_str;
			*p_result = AST_OPERATOR_POS_OR_ADD;
			break;

		case '-':
			if (name[1])
				goto unknown_str;
			*p_result = AST_OPERATOR_NEG_OR_SUB;
			break;

		case '[':
			if (name[1] == ':') {
				if (name[2] != ']')
					goto unknown_str;
				if (name[3])
					goto unknown_str;
				*p_result = AST_OPERATOR_GETRANGE_OR_SETRANGE;
			} else {
				if (name[1] != ']')
					goto unknown_str;
				if (name[2])
					goto unknown_str;
				*p_result = AST_OPERATOR_GETITEM_OR_SETITEM;
			}
			break;

		case '.':
			if (name[1])
				goto unknown_str;
			*p_result = AST_OPERATOR_GETATTR_OR_SETATTR;
			break;

		default:
unknown_str:
			/* TODO: Must support loading the operator ID at
			 *       runtime, and remembering its name until
			 *       then! */
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Unknown operator %q",
			                       name);
		}
		return 0;
	}
	return DeeObject_AsUInt16(opid, p_result);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeoperatorfunc(DeeCompilerObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	/* "(name:?Dstring,binding:?#Ast=!N,scope:?#Scope=!N)->?#Ast\n"
	 * "(name:?Dint,binding:?#Ast=!N,scope:?#Scope=!N)->?#Ast\n" */
	DREF DeeObject *result = NULL;
	DREF struct ast *result_ast;
	Dee_operator_t id;
	DeeScopeObject *ast_scope;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeoperatorfunc", params: """
	DeeObject *name:?X2?Dstring?Dint;
	DeeCompilerAstObject *binding:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeoperatorfunc_params "name:?X2?Dstring?Dint,binding:?#Ast=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeObject *name;
		DeeCompilerAstObject *binding;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.binding = (DeeCompilerAstObject *)Dee_None;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_binding_scope_loc, "o|ooo:makeoperatorfunc", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(get_operator_id(args.name, &id))
		goto done_compiler_end;
	if (!DeeNone_Check(args.binding)) {
		if unlikely(DeeObject_AssertTypeExact(args.binding, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.binding->ci_compiler != self) {
			err_invalid_ast_compiler(args.binding);
			goto done_compiler_end;
		}
		if unlikely(args.binding->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.binding, ast_scope->s_base);
			goto done_compiler_end;
		}
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type = AST_OPERATOR_FUNC;
	result_ast->a_flag = id;
	if (!DeeNone_Check(args.binding)) {
		result_ast->a_operator_func.of_binding = args.binding->ci_value;
		ast_incref(args.binding->ci_value);
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

/* Parse the flags for an operator-ast from a string (:rt:Compiler.makeoperator) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
parse_operator_flags(char const *__restrict flags,
                     uint16_t *__restrict p_result) {
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
		/* TODO: Strip leading/trailing spaces! */
		if (flag_length) {
#define IS_FLAG(x) (flag_length == COMPILER_STRLEN(x) && bcmpc(flags, x, COMPILER_STRLEN(x), sizeof(char)) == 0)
			if (IS_FLAG("post")) {
				*p_result |= AST_OPERATOR_FPOSTOP;
			} else if (IS_FLAG("varargs")) {
				*p_result |= AST_OPERATOR_FVARARGS;
			} else if (IS_FLAG("maybeprefix")) {
				*p_result |= AST_OPERATOR_FMAYBEPFX;
			} else if (IS_FLAG("dontoptimize")) {
				*p_result |= AST_OPERATOR_FDONTOPT;
			} else {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeoperator(DeeCompilerObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	/* "(name:?Dstring,a:?#Ast,b:?#Ast=!N,c:?#Ast=!N,d:?#Ast=!N,flags=!P{},scope=!N)->?#Ast\n"
	 * "(name:?Dint,a:?#Ast,b:?#Ast=!N,c:?#Ast=!N,d:?#Ast=!N,flags=!P{},scope=!N)->?#Ast\n" */
	DREF DeeObject *result = NULL;
	uint16_t id, flags = 0;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeoperator", params: """
	DeeObject *name:?X2?Dstring?Dint;
	DeeCompilerAstObject *a:?#Ast;
	DeeCompilerAstObject *b:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *c:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *d:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeStringObject *flags:?X2?Dstring?Dint=!P{} = (DeeStringObject *)Dee_EmptyString;
	DeeCompilerScopeObject *scope:?#Scope=!N = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeoperator_params "name:?X2?Dstring?Dint,a:?#Ast,b:?#Ast=!N,c:?#Ast=!N,d:?#Ast=!N,flags:?X2?Dstring?Dint=!P{},scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeObject *name;
		DeeCompilerAstObject *a;
		DeeCompilerAstObject *b;
		DeeCompilerAstObject *c;
		DeeCompilerAstObject *d;
		DeeStringObject *flags;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.b = (DeeCompilerAstObject *)Dee_None;
	args.c = (DeeCompilerAstObject *)Dee_None;
	args.d = (DeeCompilerAstObject *)Dee_None;
	args.flags = (DeeStringObject *)Dee_EmptyString;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_a_b_c_d_flags_scope_loc, "oo|oooooo:makeoperator", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely(get_operator_id(args.name, &id))
		goto done_compiler_end;
	if (args.flags != (DeeStringObject *)Dee_EmptyString) {
		if (DeeString_Check(args.flags)) {
			if (parse_operator_flags(DeeString_STR(args.flags), &flags))
				goto done_compiler_end;
		} else {
			if (DeeObject_AsUInt16(Dee_AsObject(args.flags), &flags))
				goto done_compiler_end;
		}
	}
	if unlikely(DeeObject_AssertTypeExact(args.a, &DeeCompilerAst_Type))
		goto done_compiler_end;
	if unlikely(args.a->ci_compiler != self) {
		err_invalid_ast_compiler(args.a);
		goto done_compiler_end;
	}
	if unlikely(args.a->ci_value->a_scope->s_base != ast_scope->s_base) {
		err_invalid_ast_basescope(args.a, ast_scope->s_base);
		goto done_compiler_end;
	}
	if (!DeeNone_Check(args.b)) {
		if unlikely(DeeObject_AssertTypeExact(args.b, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.b->ci_compiler != self) {
			err_invalid_ast_compiler(args.b);
			goto done_compiler_end;
		}
		if unlikely(args.b->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.b, ast_scope->s_base);
			goto done_compiler_end;
		}
		if (!DeeNone_Check(args.c)) {
			if unlikely(DeeObject_AssertTypeExact(args.c, &DeeCompilerAst_Type))
				goto done_compiler_end;
			if unlikely(args.c->ci_compiler != self) {
				err_invalid_ast_compiler(args.c);
				goto done_compiler_end;
			}
			if unlikely(args.c->ci_value->a_scope->s_base != ast_scope->s_base) {
				err_invalid_ast_basescope(args.c, ast_scope->s_base);
				goto done_compiler_end;
			}
			if (!DeeNone_Check(args.d)) {
				if unlikely(DeeObject_AssertTypeExact(args.d, &DeeCompilerAst_Type))
					goto done_compiler_end;
				if unlikely(args.d->ci_compiler != self) {
					err_invalid_ast_compiler(args.d);
					goto done_compiler_end;
				}
				if unlikely(args.d->ci_value->a_scope->s_base != ast_scope->s_base) {
					err_invalid_ast_basescope(args.d, ast_scope->s_base);
					goto done_compiler_end;
				}
			}
		}
	}
	switch (id) {
	case AST_OPERATOR_POS_OR_ADD:
		id = DeeNone_Check(args.b) ? OPERATOR_POS : OPERATOR_ADD;
		break;
	case AST_OPERATOR_NEG_OR_SUB:
		id = DeeNone_Check(args.b) ? OPERATOR_NEG : OPERATOR_SUB;
		break;
	case AST_OPERATOR_GETITEM_OR_SETITEM:
		id = DeeNone_Check(args.c) ? OPERATOR_GETITEM : OPERATOR_SETITEM;
		break;
	case AST_OPERATOR_GETRANGE_OR_SETRANGE:
		id = DeeNone_Check(args.c) ? OPERATOR_GETRANGE : OPERATOR_SETRANGE;
		break;
	case AST_OPERATOR_GETATTR_OR_SETATTR:
		id = DeeNone_Check(args.c) ? OPERATOR_GETATTR : OPERATOR_SETATTR;
		break;
	default: break;
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type              = AST_OPERATOR;
	result_ast->a_flag              = id;
	result_ast->a_operator.o_exflag = flags;
	result_ast->a_operator.o_op0    = args.a->ci_value;
	result_ast->a_operator.o_op1    = NULL;
	result_ast->a_operator.o_op2    = NULL;
	result_ast->a_operator.o_op3    = NULL;
	ast_incref(args.a->ci_value);
	if (!DeeNone_Check(args.b)) {
		result_ast->a_operator.o_op1 = args.b->ci_value;
		ast_incref(args.b->ci_value);
		if (!DeeNone_Check(args.c)) {
			result_ast->a_operator.o_op2 = args.c->ci_value;
			ast_incref(args.c->ci_value);
			if (!DeeNone_Check(args.d)) {
				result_ast->a_operator.o_op3 = args.d->ci_value;
				ast_incref(args.d->ci_value);
			}
		}
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}


INTERN WUNUSED NONNULL((1)) int32_t DCALL
get_action_by_name(char const *__restrict name) {
#define EQAT(ptr, str) (bcmp(ptr, str, sizeof(str)) == 0)
#define RETURN(id)     \
	do {               \
		result = (id); \
		goto done;     \
	}	__WHILE0
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

INTERN WUNUSED char const *DCALL
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


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeaction(DeeCompilerObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	/* "(name:?Dstring,a:?#Ast=!N,b:?#Ast=!N,c:?#Ast=!N,bool mustrun=true,scope=!N)->?#Ast\n" */
	DREF DeeObject *result = NULL;
	int32_t id;
	DeeScopeObject *ast_scope;
	DREF struct ast *result_ast;
	uint8_t opc;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("makeaction", params: """
	DeeObject *name:?Dstring;
	DeeCompilerAstObject *a:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *b:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	DeeCompilerAstObject *c:?#Ast=!N = (DeeCompilerAstObject *)Dee_None;
	bool mustrun                              = true;
	DeeCompilerScopeObject *scope:?#Scope=!N  = (DeeCompilerScopeObject *)Dee_None;
	DeeObject *loc:?T3?AFile?#Lexer?Dint?Dint = NULL;
""", docStringPrefix: "ast", err: "done");]]]*/
#define ast_makeaction_params "name:?Dstring,a:?#Ast=!N,b:?#Ast=!N,c:?#Ast=!N,mustrun=!t,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint"
	struct {
		DeeObject *name;
		DeeCompilerAstObject *a;
		DeeCompilerAstObject *b;
		DeeCompilerAstObject *c;
		bool mustrun;
		DeeCompilerScopeObject *scope;
		DeeObject *loc;
	} args;
	args.a = (DeeCompilerAstObject *)Dee_None;
	args.b = (DeeCompilerAstObject *)Dee_None;
	args.c = (DeeCompilerAstObject *)Dee_None;
	args.mustrun = true;
	args.scope = (DeeCompilerScopeObject *)Dee_None;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_a_b_c_mustrun_scope_loc, "o|oooboo:makeaction", &args))
		goto done;
/*[[[end]]]*/
	if unlikely(DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto done;
	if (COMPILER_BEGIN(self))
		goto done;
	if unlikely((ast_scope = get_scope(args.scope)) == NULL)
		goto done_compiler_end;
	if unlikely((id = get_action_by_name(DeeString_STR(args.name))) < 0)
		goto done_compiler_end;
	opc = 0;
	if (!DeeNone_Check(args.a)) {
		opc = 1;
		if unlikely(DeeObject_AssertTypeExact(args.a, &DeeCompilerAst_Type))
			goto done_compiler_end;
		if unlikely(args.a->ci_compiler != self) {
			err_invalid_ast_compiler(args.a);
			goto done_compiler_end;
		}
		if unlikely(args.a->ci_value->a_scope->s_base != ast_scope->s_base) {
			err_invalid_ast_basescope(args.a, ast_scope->s_base);
			goto done_compiler_end;
		}
		if (!DeeNone_Check(args.b)) {
			opc = 2;
			if unlikely(DeeObject_AssertTypeExact(args.b, &DeeCompilerAst_Type))
				goto done_compiler_end;
			if unlikely(args.b->ci_compiler != self) {
				err_invalid_ast_compiler(args.b);
				goto done_compiler_end;
			}
			if unlikely(args.b->ci_value->a_scope->s_base != ast_scope->s_base) {
				err_invalid_ast_basescope(args.b, ast_scope->s_base);
				goto done_compiler_end;
			}
			if (!DeeNone_Check(args.c)) {
				opc = 3;
				if unlikely(DeeObject_AssertTypeExact(args.c, &DeeCompilerAst_Type))
					goto done_compiler_end;
				if unlikely(args.c->ci_compiler != self) {
					err_invalid_ast_compiler(args.c);
					goto done_compiler_end;
				}
				if unlikely(args.c->ci_value->a_scope->s_base != ast_scope->s_base) {
					err_invalid_ast_basescope(args.c, ast_scope->s_base);
					goto done_compiler_end;
				}
			}
		}
	}
	if (AST_FACTION_ARGC_GT((uint16_t)id) != opc) {
		if ((uint16_t)id == AST_FACTION_ASSERT && opc == 2) {
			id = (int32_t)AST_FACTION_ASSERT_M;
		} else {
			DeeError_Throwf(&DeeError_TypeError,
			                "Invalid operand count for action %k expecting %u operands when %u were given",
			                args.name,
			                (unsigned int)AST_FACTION_ARGC_GT((uint16_t)id),
			                (unsigned int)opc);
			goto done_compiler_end;
		}
	}
	result_ast = ast_new(ast_scope, args.loc);
	if unlikely(!result_ast)
		goto done_compiler_end;
	result_ast->a_type = AST_ACTION;
	result_ast->a_flag = (uint16_t)id;
	if (!args.mustrun)
		result_ast->a_flag |= AST_FACTION_MAYBERUN;
	result_ast->a_action.a_act0 = NULL;
	result_ast->a_action.a_act1 = NULL;
	result_ast->a_action.a_act2 = NULL;
	if (!DeeNone_Check(args.a)) {
		result_ast->a_action.a_act0 = args.a->ci_value;
		ast_incref(args.a->ci_value);
		if (!DeeNone_Check(args.b)) {
			result_ast->a_action.a_act1 = args.b->ci_value;
			ast_incref(args.b->ci_value);
			if (!DeeNone_Check(args.c)) {
				result_ast->a_action.a_act2 = args.c->ci_value;
				ast_incref(args.c->ci_value);
			}
		}
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeclass(DeeCompilerObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makelabel(DeeCompilerObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makegoto(DeeCompilerObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeswitch(DeeCompilerObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_makeassembly(DeeCompilerObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTDEF struct type_method tpconst compiler_methods[];
INTERN_TPCONST struct type_method tpconst compiler_methods[] = {
	TYPE_KWMETHOD("makeconstexpr", &ast_makeconstexpr,
	              "(" ast_makeconstexpr_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @scope doesn't match @this}"
	              "Construct a new constant-expression ast referring to @value"),
	TYPE_KWMETHOD("makesym", &ast_makesym,
	              "(" ast_makesym_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @sym or @scope doesn't match @this}"
	              "#tReferenceError{The given @sym is not reachable from the effectively used @scope}"
	              "Construct a new branch that is using a symbol @sym\n"
	              "By default, the branch uses @sym for reading, however if the branch "
	              /**/ "is later used as target of a store-action branch, @sym will be used "
	              /**/ "for writing instead. Similarly, special handling is done when the "
	              /**/ "branch is used in an assembly output operand"),
	TYPE_KWMETHOD("makeunbind", &ast_makeunbind,
	              "(" ast_makeunbind_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @sym or @scope doesn't match @this}"
	              "#tReferenceError{The given @sym is not reachable from the effectively used @scope}"
	              "Construct a branch for unbinding the value of @sym at runtime"),
	TYPE_KWMETHOD("makebound", &ast_makebound,
	              "(" ast_makebound_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @sym or @scope doesn't match @this}"
	              "#tReferenceError{The given @sym is not reachable from the effectively used @scope}"
	              "Construct a branch for checking if a given symbol @sym is bound"),
	TYPE_KWMETHOD("makemultiple", &ast_makemultiple,
	              "(" ast_makemultiple_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of one of the given @branches or @scope doesn't match @this}"
	              "#tTypeError{The given @typing is neither ?N, nor one of the types listed below}"
	              "#tReferenceError{One of the given @branches is not part of the basescope of the effective @scope}"
	              "Construct a multi-branch, which either behaves as keep-last (only the last ast from @branches "
	              /**/ "is used as expression value of the returned branch, while all others are evaluated before then), "
	              /**/ "when @typing is ?N, or construct a sequence expression for the associated type when @typing "
	              /**/ "is one of the following\n"
	              "#T{Type|Example|Description~"
	              /**/ "?DTuple|${(a, b, c)}|Construct a Tuple expression&"
	              /**/ "?DList|${[a, b, c]}|Construct a List expression&"
	              /**/ "?DHashSet|-|Construct a mutable HashSet sequence expression&"
	              /**/ "?DDict|-|Construct a mutable Dict-like mapping expression&"
	              /**/ "?DSequence|${{ a, b, c }}|Construct an abstract sequence expression&"
	              /**/ "?DMapping|${{ a: b, c: d }}|Construct an abstract mapping expression"
	              "}\n"
	              "Note that in any kind of sequence branch, asts created by @makeexpand may "
	              /**/ "appear, and will be inlined as part of the greater whole expression"),
	TYPE_KWMETHOD("makereturn", &ast_makereturn,
	              "(" ast_makereturn_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @expr or @scope doesn't match @this}"
	              "#tReferenceError{The given @expr is not part of the basescope of the effective @scope}"
	              "Construct a return-branch that either returns @expr, or ?N when @expr is ?N"),
	TYPE_KWMETHOD("makeyield", &ast_makeyield,
	              "(" ast_makeyield_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @expr or @scope doesn't match @this}"
	              "#tReferenceError{The given @expr is not part of the basescope of the effective @scope}"
	              "Construct a yield-branch that either returns @expr, or ?N when @expr is ?N"),
	TYPE_KWMETHOD("makethrow", &ast_makethrow,
	              "(" ast_makethrow_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @expr or @scope doesn't match @this}"
	              "#tReferenceError{The given @expr is not part of the basescope of the effective @scope}"
	              "Construct a throw-branch that either throws @expr, or re-throws the last exception when @expr is ?N"),
	TYPE_KWMETHOD("maketry", &ast_maketry,
	              "(" ast_maketry_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of one of the given branches or @scope doesn't match @this}"
	              "#tValueError{One of the flags-strings contains an unknown flag}"
	              "#tReferenceError{One of the given branch is not part of the basescope of the effective @scope}"
	              "Construct a try-branch guarding @guard, referring to @handlers, which is a sequences "
	              /**/ "of tuples in the form of (:string flags, ?#Ast mask, ?#Ast code), where `mask' may also be "
	              /**/ "passed as ?N in order to indicate the lack of a catch-mask\n"
	              "The flags in this triple is a $\",\"-separated string containing "
	              /**/ "zero or more of the following options, with empty options being ignored:\n"
	              "#T{Name|Description~"
	              /**/ "$\"finally\"|The handler is a finally-handler, meaning it always gets executed&"
	              /**/ "$\"interrupt\"|The handler is capable of catching interrupt-exceptions (ignored when $\"finally\" is given)"
	              "}"),
	TYPE_KWMETHOD("makeloop", &ast_makeloop,
	              "(flags:?Dstring,elem:?#Ast=!N,iter:?#Ast,loop:?#Ast=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint)->?#Ast\n"
	              "(flags:?Dstring,cond:?#Ast=!N,next:?#Ast=!N,loop:?#Ast=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint)->?#Ast\n"
	              "(flags:?Dint,elem:?#Ast=!N,iter:?#Ast,loop:?#Ast=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint)->?#Ast\n"
	              "(flags:?Dint,cond:?#Ast=!N,next:?#Ast=!N,loop:?#Ast=!N,scope:?#Scope=!N,loc?:?T3?AFile?#Lexer?Dint?Dint)->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of one of the given branches or @scope doesn't match @this}"
	              "#tValueError{The given @flags contains an unknown flag}"
	              "#tReferenceError{One of the given branch is not part of the basescope of the effective @scope}"
	              "Construct a loop, with the type of loop being determined by @flags, which is a "
	              /**/ "$\",\"-separated string containing zero or more of the following options, "
	              /**/ "with empty options being ignored:\n"
	              "#T{Name|Description~"
	              /**/ "$\"foreach\"|The loop is a foreach-loop and the first prototype is "
	              /**/ /*        */ "used, with @iter having to be passed as an ?#Ast object. "
	              /**/ /*        */ "Otherwise, the second prototype is used, and any of the "
	              /**/ /*        */ "given branches may be none when omitted&"
	              /**/ "$\"postcond\"|The given @cond is evaluated after the loop (as seen in ${do loop; while (cond);}). "
	              /**/ /*         */ "This flag is ignored in foreach-loops&"
	              /**/ "$\"unlikely\"|The block (@loop) of the loop is unlikely to be reached, and "
	              /**/ /*         */ "should be placed in a section of code that is rarely used"
	              "}"),
	TYPE_KWMETHOD("makeloopctl", &ast_makeloopctl,
	              "(" ast_makeloopctl_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @scope doesn't match @this}"
	              "Construct a loop control branch, that is either a $continue (when "
	              /**/ "@isbreak is ?f), or a $break statement (when @isbreak is ?t)"),
	TYPE_KWMETHOD("makeconditional", &ast_makeconditional,
	              "(" ast_makeconditional_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The given @flags contains an unrecognized, or invalid flag}"
	              "#tValueError{The compiler of one of the given branches or @scope doesn't match @this}"
	              "#tTypeError{Both @tt and @ff have been passed as ?N}"
	              "#tReferenceError{One of the given branch is not part of the basescope of the effective @scope}"
	              "Construct a conditional branch for executing @tt or @ff, based on the runtime value of @cond\n"
	              "You may additionally pass @cond for @tt or @ff in order to propagate the value of @cond "
	              "as the result of the conditional branch, when that value is used.\n"
	              "For example, in ${cond ? : ff}, the value of `cond' is propagated when it is ?t, "
	              "and replaced with `ff' when not. This is equivalent to ${makeconditional(cond, cond, ff)}, "
	              "but must be noted specifially, as the conditional branch is only evaluated once, meaning "
	              "that any side-effects only happen once, too\n"
	              "Using this knowledge, you could also construct a branch ${makeconditional(cond, tt, cond)}, which simply does the opposite\n"
	              "When @tt is ?N, the true-branch returns ?N at runtime\n"
	              "When @ff is ?N, the false-branch returns ?N at runtime\n"
	              "The given @flags is a $\",\"-separated string containing zero or "
	              "more of the following options, with empty options being ignored:\n"
	              "#T{Name|Description~"
	              /**/ "$\"bool\"|The values of @cond, @tt and @ff are cast to a boolean during evaluation. "
	              /**/ /*     */ "Using this, code like ${a || b} results in a branch ${makeconditional(a, a, b, \"bool\")}, "
	              /**/ /*     */ "and ${a && b} results in a branch ${makeconditional(a, b, a, \"bool\")}&"
	              /**/ "$\"likely\"|When given, assembly for @ff is placed in a section of code that is rarely used&"
	              /**/ "$\"unlikely\"|When given, assembly for @tt is placed in a section of code that is rarely used"
	              "}"),
	TYPE_KWMETHOD("makebool", &ast_makebool,
	              "(" ast_makebool_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @expr or @scope doesn't match @this}"
	              "#tReferenceError{The given @expr is not part of the basescope of the effective @scope}"
	              "Construct a branch for casting @expr to a boolean, optionally inverting the "
	              /**/ "underlying boolean logic when @negate is ?t\n"
	              "The expression ${!!a} results in ${makebool(a, false)}, while ${!a} results in ${makebool(a, true)}"),
	TYPE_KWMETHOD("makeexpand", &ast_makeexpand,
	              "(" ast_makeexpand_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @expr or @scope doesn't match @this}"
	              "#tReferenceError{The given @expr is not part of the basescope of the effective @scope}"
	              "Construct an expand-branch that will unpack a sequence expression @expr at runtime"),
	TYPE_KWMETHOD("makefunction", &ast_makefunction,
	              "(" ast_makefunction_params ")->?#Ast\n"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The compiler of @code or @scope doesn't match @this}"
	              "#tReferenceError{The effective @scope is not reachable from ${code.scope}}"
	              "#tReferenceError{The effective ${scope.base} is identical to ${code.scope.base}}"
	              "Construct a new lambda-like function that will execute @code\n"
	              "The base-scope of the function is set to ${code.scope.base}, while the returned "
	              /**/ "branch will be executed in the context of @scope, or the current scope when ?N"),
	TYPE_KWMETHOD("makeoperatorfunc", &ast_makeoperatorfunc,
	              "(" ast_makeoperatorfunc_params ")->?#Ast\n"
	              "#pname{The name of the operator, or one of ${[\"+\", \"-\", \"[]\", \"[:]\", \".\"]} "
	              /*  */ "for ambiguous operators resolved at runtime}"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The given @name is not recognized as a valid operator}"
	              "#tValueError{The compiler of @binding or @scope doesn't match @this}"
	              "#tReferenceError{The given @binding is not part of the same base-scope as the effective @scope}"
	              "Create a new branch a reference to one of the operator functions, or to "
	              /**/ "construct an instance-bound operator function\n"
	              "For example ${operator add} results in ${makeoperatorfunc(\"add\")}, while "
	              /**/ "${binding.operator add} results in ${makeoperatorfunc(\"add\", binding)}"),
	TYPE_KWMETHOD("makeoperator", &ast_makeoperator,
	              "(" ast_makeoperator_params ")->?#Ast\n"
	              "#pname{The name of the operator, or one of ${[\"+\", \"-\", \"[]\", \"[:]\", \".\"]} "
	              /*  */ "for ambiguous operators resolved based on argument count}"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The given @name is not recognized as a valid operator}"
	              "#tValueError{The compiler of one of the given branches or @scope doesn't match @this}"
	              "#tReferenceError{One of the given branches is not part of the same base-scope as the effective @scope}"
	              "Create an operator branch for invoking the operator @name with the given branches @a, @b, @c and @d\n"
	              "The given @flags is a $\",\"-separated string containing zero or "
	              /**/ "more of the following options, with empty options being ignored:\n"
	              "#T{Flag|Description~"
	              /**/ "$\"post\"|The invocation is assembled as ${({ __stack local _res = copy a; a.operator <name> (b[, [c, d]]); _res; })} "
	              /**/ /*     */ "when the result of the expression is being used. Otherwise, this flag is ignored. "
	              /**/ /*     */ "This is mainly used to implement ${a++}, such that the old value of @a is returned&"
	              /**/ "$\"varargs\"|May only be passed when exactly 2 operands (@a and @b) are given: @b should be "
	              /**/ /*        */ "interpreted as a sequence expression containing the actual operands then used "
	              /**/ /*        */ "to invoke the operator at runtime. This is mainly used to implement operator "
	              /**/ /*        */ "invocations with variable secondary operand counts, as caused by an expand "
	              /**/ /*        */ "expression appearing within a multi-branch argument&"
	              /**/ "$\"maybeprefix\"|Still generate valid assembly for dealing with the problem at runtime when an "
	              /**/ /*            */ "inplace operator is used when the @a operand cannot actually be written to&"
	              /**/ "$\"dontoptimize\"|Don't perform constant optimizations within this branch during the ast-optimization pass}"),
	TYPE_KWMETHOD("makeaction", &ast_makeaction,
	              "(" ast_makeaction_params ")->?#Ast\n"
	              "#pname{The name of the action (see table below)}"
	              "#pmustrun{When ?f, ast-optimization may optimize away side-effects caused by action operands. "
	              /*     */ "Otherwise, all operands are required to execute as required by the action (which usually means executed-in-order)}"
	              "#pscope{The scope to-be used for the new branch, or ?N to use ?#scope}"
	              "#ploc{The location of the ast for DDI, omitted to use the current token position, or ?N when not available}"
	              "#tValueError{The given @name is not recognized as a valid action}"
	              "#tValueError{The compiler of one of the given branches or @scope doesn't match @this}"
	              "#tReferenceError{One of the given branches is not part of the same base-scope as the effective @scope}"
	              "#tTypeError{Too many or too few operand-branches provided for the specified action}"
	              "Similar to ?#makeoperator, but instead used to construct action-branches that are then used "
	              /**/ "to perform operator-unrelated actions, such as storing an expression into a symbol\n"
	              "The given @name is must be one of the following\n"
	              "#T{Action|Example|Operands|Description~"
	              /**/ "$\"typeof\"|${type a}|1|Returns the type of a given expression&"
	              /**/ "$\"classof\"|${a.class}|1|Returns the class of a given expression (which is the bound type in :super objects)&"
	              /**/ "$\"superof\"|${a.super}|1|Returns a view for the super-class of a given object&"
	              /**/ "$\"print\"|${print a...,;}|1|Print the individual elements of a sequence @a, separated by spaces&"
	              /**/ "$\"println\"|${print a...;}|1|Print the individual elements of a sequence @a, separated by spaces, and followed by a line-feed&"
	              /**/ "$\"fprint\"|${print a: b...,;}|2|Same as $\"print\", but print a sequence @b, and write data to a file @a&"
	              /**/ "$\"fprintln\"|${print a: b...;}|2|Same as $\"println\", but print a sequence @b, and write data to a file @a&"
	              /**/ "$\"range\"|${[a:b, c]}|3|Construct a range expression. Note that @a and @c may evaluate to ?N at runtime, in which case the behavior is the same as when omitted in user-code&"
	              /**/ "$\"is\"|${a is b}|2|Check if @a is an instance of @b at runtime, and evaluate to ?t or ?f&"
	              /**/ "$\"in\"|${a in b}|2|Same as ${b.operator contains(a)}, however operands are evaluated in reverse order&"
	              /**/ "$\"as\"|${a as b}|2|Construct a super-wrapper for @a with a typing of @b&"
	              /**/ "$\"min\"|${a < ...}|1|Evaluate to the lowest element from a sequence in @a, with the side-effect of enumerating @a&"
	              /**/ "$\"max\"|${a > ...}|1|Evaluate to the greatest element from a sequence in @a, with the side-effect of enumerating @a&"
	              /**/ "$\"sum\"|${a + ...}|1|Evaluate to the sum of all element from a sequence in @a, with the side-effect of enumerating @a&"
	              /**/ "$\"any\"|${a || ...}|1|Evaluate to ?t if any element from @a evaluates to ?t, or ?f when @a is empty or has no such elements, with the side-effect of enumerating @a&"
	              /**/ "$\"all\"|${a && ...}|1|Evaluate to ?t if all elements from @a evaluate to ?t or when @a is empty, or ?f otherwise, with the side-effect of enumerating @a&"
	              /**/ "$\"store\"|${a = b}|2|Store the expression in @b into the branch @a (@a may be a ?#makesym, ?#makemultiple, or a $\"getitem\", $\"getrange\", or $\"getattr\" ?#makeoperator branch)&"
	              /**/ "$\"assert\"|${assert(a)} or ${assert(a, b)}|1 or 2|Assert that @a evaluates to ?t when cast to a boolean, otherwise throwing an :AssertionError at runtime, alongside an optional message @b. "
	              /**/ /*                                              */ "When ?t and used in an expression, evaluate to the propagated value of @a, such that ${print assert(42);} would output $42 to :File.stdout&"
	              /**/ "$\"boundattr\"|${a.operator . (b) is bound}|2|Evaluate to ?t / ?f when attribute @b of @a is bound at runtime&"
	              /**/ "$\"sameobj\"|${a === b is bound}|2|Evaluate to ?t when @a and @b are the same object at runtime, or ?f otherwise&"
	              /**/ "$\"diffobj\"|${a !== b is bound}|2|Evaluate to ?t when @a and @b are different objects at runtime, or ?f otherwise&"
	              /**/ "$\"callkw\"|${a(b..., **c)}|3|Perform a call to @a, using positional arguments from @b, and a keyword list from @c"
	              "}"),
	TYPE_KWMETHOD("makeclass", &ast_makeclass, "TODO"),
	TYPE_KWMETHOD("makelabel", &ast_makelabel, "TODO"),
	TYPE_KWMETHOD("makegoto", &ast_makegoto, "TODO"),
	TYPE_KWMETHOD("makeswitch", &ast_makeswitch, "TODO"),
	TYPE_KWMETHOD("makeassembly", &ast_makeassembly, "TODO"),
	TYPE_METHOD_END
};

INTDEF struct type_member tpconst compiler_class_members[];
INTERN_TPCONST struct type_member tpconst compiler_class_members[] = {
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
