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
/*!export **/
/*!export AST_ANNOTATION_F**/
/*!export AST_COMMA_**/
/*!export AST_OPERATOR_**/
/*!export AST_PARSE_WASEXPR_**/
/*!export CASE_TOKEN_IS_**/
/*!export TOKEN_IS_**/
/*!export ast_annotation**/
/*!export ast_**/
/*!export ast_parse_**/
/*!export ast_tags**/
/*!export astlist**/
/*!export parse_**/
#ifndef GUARD_DEEMON_COMPILER_PARSER_H
#define GUARD_DEEMON_COMPILER_PARSER_H 1 /*!export-*/

#include "../api.h"

#ifdef CONFIG_BUILDING_DEEMON
#include "../object.h"          /* Dee_Decref */
#include "../string.h"          /* Dee_unicode_printer_fini */
#include "../system-features.h" /* bzero, memcpy */
#include "../type.h"            /* Dee_operator_t */
#include "../types.h"           /* DREF, DeeObject, ITER_DONE */
#include "ast.h"                /* ast */
#include "lexer.h"              /* DeeLexer, TPP_KWD_*, ast_loc, tpp_keyword, tpp_token_id */
#include "symbol.h"             /* LOOKUP_SYM_NORMAL, decl_ast, symbol */

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* int32_t, uint16_t */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER
typedef struct Dee_parser {
	DeeLexer     p_lexer; /* Underlying lexer */
#define DeeParser_F_NORMAL 0x00 /* Normal parser flags. */
#define DeeParser_F_LFSTMT 0x01 /* Parse line-feeds as statement terminators in certain places. */
	uint_fast8_t p_flags; /* Parsing flags (set of `DeeParser_F_*`) */
} DeeParser;

/* Consume annotation flags (preventing warnings about unused annotations)
 * Returns `true` if the resp. annotation was present (and consumed) */
#define DeeParser_ConsumeAnnotation_Likely(self)   false
#define DeeParser_ConsumeAnnotation_Unlikely(self) false
#define DeeParser_ConsumeAnnotation_Copyable(self) false
#define DeeParser_ConsumeAnnotation_ThisCall(self) false

/* Describe a region of code where `TPP_TOK_LF` should not be produced. */
#define DeeParser_NoLf_Push(self)                                                           \
	do {                                                                                    \
		bool const _dpnlf_olf = !!tpp_lexer_getfeature(&(self)->p_lexer.dl_lexer, TPP_FEAT_TOK_LF); \
		tpp_lexer_disablefeature(&(self)->p_lexer.dl_lexer, TPP_FEAT_TOK_LF)
#define DeeParser_NoLf_Break(self) \
		tpp_lexer_setfeature(&(self)->p_lexer.dl_lexer, TPP_FEAT_TOK_LF, _dpnlf_olf)
#define DeeParser_NoLf_Pop(self)    \
		DeeParser_NoLf_Break(self); \
	}	__WHILE0

/* Describe a region of code where `TPP_TOK_LF` is enabled if `PARSE_FLFSTMT` is set */
#define DeeParser_EnableLf_Push(self)                            \
	do {                                                         \
		bool const _dpelf_olf = !!tpp_lexer_getfeature(&(self)->p_lexer.dl_lexer, TPP_FEAT_TOK_LF); \
		if (!(parser_flags & PARSE_FLFSTMT)) {                   \
		} else                                                   \
			tpp_lexer_enablefeature(&(self)->p_lexer.dl_lexer, TPP_FEAT_TOK_LF)
#define DeeParser_EnableLf_Break(self) \
		tpp_lexer_setfeature(&(self)->p_lexer.dl_lexer, TPP_FEAT_TOK_LF, _dpelf_olf)
#define DeeParser_EnableLf_Pop(self)    \
		DeeParser_EnableLf_Break(self); \
	}	__WHILE0


/* Convenience wrappers... */
#define DeeParser_OfTPP(p_tpp_lexer)                     COMPILER_CONTAINER_OF(p_tpp_lexer, DeeParser, p_lexer.dl_lexer)
#define DeeParser_Has(self, feat)                        DeeLexer_Has(&(self)->p_lexer, feat)
#define DeeParser_IsIdentifier(self, name)               DeeLexer_IsIdentifier(&(self)->p_lexer, name)
#define DeeParser_GetTok(self)                           DeeLexer_GetTok(&(self)->p_lexer)
#define DeeParser_HasTokenKwd(self)                      DeeLexer_HasTokenKwd(&(self)->p_lexer)
#define DeeParser_GetTokenKwd(self)                      DeeLexer_GetTokenKwd(&(self)->p_lexer)
#define DeeParser_GetTokenKwdCStr(self)                  DeeLexer_GetTokenKwdCStr(&(self)->p_lexer)
#define DeeParser_GetTokenKwdStr(self)                   DeeLexer_GetTokenKwdStr(&(self)->p_lexer)
#define DeeParser_GetTokenKwdLen(self)                   DeeLexer_GetTokenKwdLen(&(self)->p_lexer)
#define DeeParser_GetTokenStart(self)                    DeeLexer_GetTokenStart(&(self)->p_lexer)
#define DeeParser_GetTokenEnd(self)                      DeeLexer_GetTokenEnd(&(self)->p_lexer)
#define DeeParser_GetTokenLen(self)                      DeeLexer_GetTokenLen(&(self)->p_lexer)
#define DeeParser_GetFile(self)                          DeeLexer_GetFile(&(self)->p_lexer)
#define DeeParser_Yield(self)                            DeeLexer_Yield(&(self)->p_lexer)
#define DeeParser_Require(self, tok)                     DeeLexer_Require(&(self)->p_lexer, tok)
#define DeeParser_Skip(self, expected_tok)               DeeLexer_Skip(&(self)->p_lexer, expected_tok)
#define DeeParser_ParenBegin(self, p_has_paren)          DeeLexer_ParenBegin(&(self)->p_lexer, p_has_paren)
#define DeeParser_ParenEnd(self, has_paren)              DeeLexer_ParenEnd(&(self)->p_lexer, has_paren)
#define DeeParser_Warnf(self, ...)                       DeeLexer_Warnf(&(self)->p_lexer, __VA_ARGS__)
#define DeeParser_VWarnf(self, id, args)                 DeeLexer_VWarnf(&(self)->p_lexer, id, args)
#define DeeParser_WarnfAt(self, file, pos, ...)          DeeLexer_WarnfAt(&(self)->p_lexer, file, pos, __VA_ARGS__)
#define DeeParser_VWarnfAt(self, file, pos, id, args)    DeeLexer_VWarnfAt(&(self)->p_lexer, file, pos, id, args)
#define DeeParser_WarnfLc(self, filename, lc, ...)       DeeLexer_WarnfLc(&(self)->p_lexer, filename, lc, __VA_ARGS__)
#define DeeParser_VWarnfLc(self, filename, lc, id, args) DeeLexer_VWarnfLc(&(self)->p_lexer, filename, lc, id, args)
#define DeeParser_WarnfLoc(self, loc, ...)               DeeLexer_WarnfLoc(&(self)->p_lexer, loc, __VA_ARGS__)
#define DeeParser_VWarnfLoc(self, loc, id, args)         DeeLexer_VWarnfLoc(&(self)->p_lexer, loc, id, args)
#define DeeParser_WarnfAst(self, p_ast, ...)             DeeLexer_WarnfAst(&(self)->p_lexer, p_ast, __VA_ARGS__)
#define DeeParser_VWarnfAst(self, p_ast, id, args)       DeeLexer_VWarnfAst(&(self)->p_lexer, p_ast, id, args)
#define DeeParser_WarnfSym(self, p_sym, ...)             DeeLexer_WarnfSym(&(self)->p_lexer, p_sym, __VA_ARGS__)
#define DeeParser_VWarnfSym(self, p_sym, id, args)       DeeLexer_VWarnfSym(&(self)->p_lexer, p_sym, id, args)
#define DeeParser_Errf(self, ...)                        DeeLexer_Errf(&(self)->p_lexer, __VA_ARGS__)
#define DeeParser_VErrf(self, id, args)                  DeeLexer_VErrf(&(self)->p_lexer, id, args)
#define DeeParser_ErrfAt(self, file, pos, ...)           DeeLexer_ErrfAt(&(self)->p_lexer, file, pos, __VA_ARGS__)
#define DeeParser_VErrfAt(self, file, pos, id, args)     DeeLexer_VErrfAt(&(self)->p_lexer, file, pos, id, args)
#define DeeParser_ErrfLc(self, filename, lc, ...)        DeeLexer_ErrfLc(&(self)->p_lexer, filename, lc, __VA_ARGS__)
#define DeeParser_VErrfLc(self, filename, lc, id, args)  DeeLexer_VErrfLc(&(self)->p_lexer, filename, lc, id, args)
#define DeeParser_ErrfLoc(self, loc, ...)                DeeLexer_ErrfLoc(&(self)->p_lexer, loc, __VA_ARGS__)
#define DeeParser_VErrfLoc(self, loc, id, args)          DeeLexer_VErrfLoc(&(self)->p_lexer, loc, id, args)
#define DeeParser_ErrfAst(self, p_ast, ...)              DeeLexer_ErrfAst(&(self)->p_lexer, p_ast, __VA_ARGS__)
#define DeeParser_VErrfAst(self, p_ast, id, args)        DeeLexer_VErrfAst(&(self)->p_lexer, p_ast, id, args)
#define DeeParser_ErrfSym(self, p_sym, ...)              DeeLexer_ErrfSym(&(self)->p_lexer, p_sym, __VA_ARGS__)
#define DeeParser_VErrfSym(self, p_sym, id, args)        DeeLexer_VErrfSym(&(self)->p_lexer, p_sym, id, args)
#define DeeParser_GetLoc(self, info)                     DeeLexer_GetLoc(&(self)->p_lexer, info)


/* Helper function for opening (sub-)modules, whilst
 * dealing with file-not-found and recursion warnings,
 * as well as context flags.
 * WARNING: The caller still needs to deal with case
 *          where `.` is used as module name!
 *
 * @return: * :        The module in question, or something to substitute it
 * @return: NULL:      An error was thrown
 * @return: ITER_DONE: Cannot load module (caller should load module at runtime) */
INTDEF WUNUSED NONNULL((1, 3)) DREF struct Dee_module_object *DCALL
DeeLexer_OpenModule(DeeLexer *__restrict self, /*0..1*/ DeeAstLoc const *loc,
                    /*utf-8*/ char const *__restrict module_name);
#define DeeParser_OpenModule(self, loc, module_name) \
	DeeLexer_OpenModule(&(self)->p_lexer, loc, module_name)

/* @return: * :        Module symbol in question
 * @return: NULL:      An error was thrown
 * @return: ITER_DONE: No such symbol (a warning `TPP_W_MODULE_IMPORT_NOT_FOUND` was already emit) */
INTDEF WUNUSED NONNULL((1, 3, 4)) struct Dee_module_symbol *DCALL
DeeLexer_GetModuleSymbol(DeeLexer *__restrict self,
                         /*0..1*/ DeeAstLoc const *loc,
                         struct Dee_module_object *__restrict mod,
                         /*utf-8*/ char const *__restrict symbol_name);
#define DeeParser_GetModuleSymbol(self, loc, mod, symbol_name) \
	DeeLexer_GetModuleSymbol(&(self)->p_lexer, loc, mod, symbol_name)

/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/import.c                   */
/************************************************************************/
struct Dee_unicode_printer;
/* Parse a module name, which is defined as:
 * >> MODULE_NAME      ::= [DOTS] [MODULE_NAME_PART [MODULE_NAME_TAIL]]
 * >> MODULE_NAME_TAIL ::= DOTS MODULE_NAME_PART
 * >> MODULE_NAME_PART ::= <KEYWORD> | <STRING>
 * >> DOTS             ::= ('.' | '...')...
 */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
DeeParser_ParseModuleName(DeeParser *__restrict self,
                          struct Dee_unicode_printer *__restrict printer);

/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/typedef.c                 */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL DeeParser_ParseTypeDef(DeeParser *__restrict self, /*out*/ struct Dee_ast_typedef *__restrict result);

/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/operator.c                 */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL DeeParser_ParseOperatorName(DeeParser *__restrict self, /*out*/ struct Dee_ast_operator_id *__restrict result);

/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/function.c                 */
/************************************************************************/
struct Dee_ast_function_builder {
	DeeAst                             *afb_func;      /* [0..afb_paramc][owned] The function AST being built */
	size_t                              afb_argc_min;  /* Min # of parameters */
	size_t                              afb_argc_max;  /* Used # of parameters (== argc_max) */
	size_t                              afb_argc_alc;  /* Allocated # of parameters */
	Dee_ast_function_flag               afb_flag;      /* Function flags */
	struct Dee_ast_function_param_base *afb_varargs;   /* [0..1][owned] Parameter for `varargs...` parameter (or `NULL` if function doesn't take varargs) */
	struct Dee_ast_function_param_base *afb_varkwds;   /* [0..1][owned] Parameter for `**varkwds` parameter (or `NULL` if function doesn't take varkwds) */
	bool                                afb_hasreturn; /* True if `DeeAst_GetFunctionReturnTypedef()` is initialized */
};

#define Dee_ast_function_builder_init(self)                      \
	(void)((self)->afb_func      = NULL,                         \
	       (self)->afb_argc_min  = 0,                            \
	       (self)->afb_argc_max  = 0,                            \
	       (self)->afb_flag      = Dee_AST_FUNCTION_FLAG_NORMAL, \
	       (self)->afb_varargs   = NULL,                         \
	       (self)->afb_varkwds   = NULL,                         \
	       (self)->afb_hasreturn = false)
INTDEF NONNULL((1)) void DCALL
Dee_ast_function_builder_fini(struct Dee_ast_function_builder *__restrict self);

/* Consume function-related @attributes from `parser` */
#define Dee_ast_function_builder_consume_attr(self, parser)     \
	do {                                                        \
		if (DeeParser_ConsumeAnnotation_Copyable(parser))       \
			(self)->afb_flag |= Dee_AST_FUNCTION_FLAG_COPYABLE; \
		if (DeeParser_ConsumeAnnotation_ThisCall(parser))       \
			(self)->afb_flag |= Dee_AST_FUNCTION_FLAG_THISCALL; \
	}	__WHILE0


/* Package a function builder into a proper AST. The caller should still add
 * location info to the returned AST, as it comes without any by default. */
INTDEF WUNUSED NONNULL((1, 3)) DeeAst *DCALL
Dee_ast_function_builder_pack(/*inherit(always)*/ struct Dee_ast_function_builder *__restrict self,
                              tpp_keyword const *opt_name, /*inherit(always)*/ DeeAst *__restrict body);

typedef uint_fast8_t Dee_parser_functionparam_flag;
#define Dee_PARSER_FUNCTIONPARAM_FLAG_NORMAL     Dee_AST_SYMREF_BYNAME_FLAG_NORMAL
#define Dee_PARSER_FUNCTIONPARAM_FLAG_GOTFINAL   Dee_AST_SYMREF_BYNAME_FLAG_FINAL   /* `final` encountered */
#define Dee_PARSER_FUNCTIONPARAM_FLAG_GOTVARYING Dee_AST_SYMREF_BYNAME_FLAG_VARYING /* `varying` encountered */
#define Dee_PARSER_FUNCTIONPARAM_FLAG_GOTLOCAL   0x04                               /* `local` encountered */

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
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
DeeParser_ParseFunctionParam(DeeParser *__restrict self,
                             struct Dee_ast_function_builder *__restrict builder,
                             Dee_parser_functionparam_flag flag);

/* Check if the current token is `:`, and if so: parse the return-type expression for `builder`
 * The caller must ensure that this function is called at most once for any `builder`.
 *
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
DeeParser_MaybeParseFunctionReturnType(DeeParser *__restrict self,
                                       struct Dee_ast_function_builder *__restrict builder);

/* Parse a function declaration, starting at the `function` keyword.
 * @param: p_requires_semicolon: When non-NULL, set to `true` or `false` on success,
 *                               indicative of the function's body being `-> expr`,
 *                               as opposed to `{ expr; }` */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseFunction(DeeParser *__restrict self,
                        enum Dee_ast_symref_byname_kind decl_kind,
                        Dee_ast_symref_byname_flag decl_flags,
                        bool *p_requires_semicolon);

/* Similar to `DeeParser_ParseFunction()`, but (should) start at the `(` of the
 * function's parameter list. Additionally, the function isn't wrapped in a
 * `Dee_AST_ACT2_STORE` AST to assign it to a symbol bearing its name, or will
 * the returned function has location information yet. */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseFunction2(DeeParser *__restrict self,
                         tpp_keyword const *opt_function_name,
                         bool *p_requires_semicolon);


/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/expression.c               */
/************************************************************************/
INTDEF ATTR_CONST WUNUSED bool DFCALL TPP_TOK_MAYBE_EXPR_START(tpp_token_id tok);
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL DeeParser_ParseUnaryHead(DeeParser *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseUnaryOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL DeeParser_ParseUnary(DeeParser *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseExprOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
#define DeeParser_ParseExprHead DeeParser_ParseUnaryHead

INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL DeeParser_ParseExpr(DeeParser *__restrict self);


/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/comma.c                    */
/************************************************************************/
struct Dee_ast_comma_builder {
	DeeAst                         *acb_multi;   /* [0..acb_multi_c] Uninitialized `Dee_AST_MULTI` describing result (first `acb_multi_c` elem are initialized) */
	size_t                          acb_multi_c; /* Used # of elements in `acb_multi` (both committed, and uncommitted) */
	size_t                          acb_multi_a; /* Allocated # of elements in `acb_multi` */
	enum Dee_ast_symref_byname_kind acb_symref_byname_kind; /* Variable declaration prefix (or `Dee_AST_SYMREF_BYNAME_KIND_AUTO` if no prefix) */
	Dee_ast_symref_byname_flag      acb_symref_byname_flag; /* Variable declaration flags */
	uint_fast8_t                    acb_flags;   /* Parser state flags (set of `Dee_AST_COMMA_BUILDER_FLAG_*`) */
#define Dee_AST_COMMA_BUILDER_FLAG_NORMAL 0x00 /* Normal flags */
#define Dee_AST_COMMA_BUILDER_FLAG_NOFUNC 0x01 /* Encountered something implying that this isn't a parameter-list
                                                * When not set, `Dee_ast_function_builder_fromcomma()` can be used:
                                                * - `a`                (flag not set)
                                                * - `a, b`             (flag not set)
                                                * - `a, b = 10`        (flag not set)
                                                * - `a, b = 10...`     (flag not set)
                                                * - `a, b = 10..., c`  (flag set)
                                                * - `a, (b)`           (flag set)
                                                * - `a, b?:c`          (flag not set)
                                                * - `a, (b?:c)`        (flag set) */
#define Dee_AST_COMMA_BUILDER_FLAG_LOCAL  0x02 /* `local` was explicitly written for `acb_symref_byname_kind` */
};

/* Transform a `Dee_ast_comma_builder` into a `Dee_ast_function_builder`. This
 * function may only be used when `Dee_AST_COMMA_BUILDER_FLAG_NOFUNC` isn't set.
 *
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
Dee_ast_function_builder_fromcomma(/*out*/ struct Dee_ast_function_builder *__restrict self,
                                   /*inherit(always)*/ struct Dee_ast_comma_builder *__restrict from);


/* HINT: `DeeParser_ParseCommaOperand()` also deals with stuff like `a, b, c = init...` */
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DFCALL
DeeParser_ParseCommaOperand(DeeParser *__restrict self,
                            /*inherit(always)*/ DeeAst *__restrict lhs,
                            enum Dee_ast_multi_kind kind);

/* WARNING: The AST returned here may not include location information */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseExprComma(DeeParser *__restrict self,
                         enum Dee_ast_multi_kind kind,
                         uint_fast8_t flags);
#define Dee_PARSER_PARSEEXPRCOMMA_NORMAL     0x00
#define Dee_PARSER_PARSEEXPRCOMMA_FORCEMULTI 0x01 /* Always return a Dee_AST_MULTI as per `kind` */
#define Dee_PARSER_PARSEEXPRCOMMA_ALLOWDECLS 0x02 /* DeeParser_ParseExprComma-only: Allow `local`, `global`, `static`, `__stack`, `final`, `varying`, as well as @attributes and typedefs */
#define Dee_PARSER_PARSEEXPRCOMMA_PARSESEMI  0x04 /* Parse a trailing `;`, if one is required */

struct Dee_parser_expr_arglist {
	DeeAst *peal_args; /* [1..1][owned] AST for function arguments (always a Dee_AST_MULTI+Dee_AST_MULTI_KIND_TUPLE) */
	DeeAst *peal_kwds; /* [0..1][owned] AST for keyword arguments (may be any kind of AST) */
};
#define Dee_parser_expr_arglist_fini(self) \
	(DeeAst_Destroy((self)->peal_args),    \
	 DeeAst_XDestroy((self)->peal_kwds))

/* Parse an argument list, including support for keyword arguments. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
DeeParser_ParseExprArgList(DeeParser *__restrict self,
                           /*out*/ struct Dee_parser_expr_arglist *__restrict result);

/* Parse the contents of a `{ ... }` expression as either:
 * >> { a, b, c }         // Dee_AST_MULTI+Dee_AST_MULTI_KIND_GENERIC_SEQ
 * >> { a: b, c: d }      // Dee_AST_MULTI+Dee_AST_MULTI_KIND_GENERIC_MAP
 * >> { a, b, c = d... }  // Compile as `{ ((a, b, c) = d)... }`, aka `{ (a, b, c = d...)... }`
 *
 * NOTE: This function must be called with the current token pointing **AFTER** the leading `{`
 * IMPORTANT: The caller must still amend location information to the returned AST. */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseExprSequenceOrMapping(DeeParser *__restrict self);



/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/hybrid.c                   */
/************************************************************************/

/* Call after having parsed a `{` that can either be treated as the start
 * of a statement-scope, or the start of a `{...}`-style sequence. This
 * function will continue parsing until an unmatched `}`-token is reached,
 * and must be called while the current token is `{`
 *
 * Examples:
 * >> ({ 42 })       ->   Dee_AST_MULTI:Dee_AST_MULTI_KIND_GENERIC_SEQ { Dee_AST_CONST:42 }
 *     ^     ^
 *     enter exit
 *
 * Examples:
 * >> ({ 42: 10 })   ->   Dee_AST_MULTI:Dee_AST_MULTI_KIND_GENERIC_MAP { Dee_AST_CONST:42, Dee_AST_CONST:10 }
 *     ^         ^
 *     enter     exit
 *
 * >> ({ 42; })      ->  Dee_AST_ACT1_SCOPE { Dee_AST_MULTI:Dee_AST_MULTI_KIND_LAST { Dee_AST_CONST:42 } }
 *     ^      ^
 *     enter  exit
 *
 * If the AST was parsed as parsed as a statement, use
 * `DeeParser_ParseStatementsOrBraces_WASSTMT()` */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseStatementsOrBraces(DeeParser *__restrict self);
#define DeeParser_ParseStatementsOrBraces_WASSTMT(returned_ast) \
	((returned_ast)->a_kind == Dee_AST_ACT1_SCOPE)


/************************************************************************/
/* file: ../../../src/deemon/compiler/parser/statement.c                */
/************************************************************************/

/* Parse a singular statement (obviously including the trailing `;` if one is required) */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL DeeParser_ParseStatement(DeeParser *__restrict self);

/* Parse statements until a token is reached that does cannot be used to start an(other)
 * statement (which will usually be a `}`-token). All statements parsed until that point
 * are then wrapped as a `Dee_AST_MULTI:Dee_AST_MULTI_KIND_LAST` and returned.
 *
 * IMPORTANT: This function does **NOT** add `Dee_AST_ACT1_SCOPE` to its return!
 * IMPORTANT: The caller must still amend location information to the returned AST. */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL DeeParser_ParseStatements(DeeParser *__restrict self);

/* With the current token being a `{`, parse a sequence of statements until
 * a matching `}`-token is encountered, then return everything that was parsed
 * in the form of a `Dee_AST_MULTI:Dee_AST_MULTI_KIND_LAST`.
 *
 * IMPORTANT: This function does **NOT** add `Dee_AST_ACT1_SCOPE` to its return! */
INTDEF WUNUSED NONNULL((1)) DeeAst *DFCALL DeeParser_ParseStatementsAtBrace(DeeParser *__restrict self);


#else /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

struct Dee_unicode_printer;
struct Dee_compiler_options;
struct Dee_module_symbol;

#define PARSE_FNORMAL 0x0000 /* Normal parser flags. */
#define PARSE_FLFSTMT 0x0001 /* Parse line-feeds as statement terminators in certain places. */

/* Parser flags (Set of `PARSE_F*`) */
INTDEF uint16_t parser_flags;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
INTDEF struct Dee_compiler_options *inner_compiler_options;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */


/* Parse a constant string. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
ast_parse_string_const(DeeLexer *self);

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
/* Parse a TPP_TOK_ISSTRING()-like token into an AST (includes template string handling) */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_string_ast(DeeLexer *self);
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
/* Parse a template string. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_template_string(DeeLexer *self);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

/* Decode the current token (which must be a TOK_STRING) as a unicode string. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
ast_parse_string_const_printer(DeeLexer *self, struct Dee_unicode_printer *__restrict printer);

#define PARSE_UNARY_DISALLOW_CASTS 0x10000

/* @param: lookup_mode: Set of `LOOKUP_SYM_*`, optionally or'd with `PARSE_UNARY_DISALLOW_CASTS` */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_unaryhead(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_unary(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_prod(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_sum(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_shift(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_cmp(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_cmpeq(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_and(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_xor(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_or(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_as(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_land(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_lor(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_cond(DeeLexer *self, unsigned int lookup_mode);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_assign(DeeLexer *self, unsigned int lookup_mode); /* NOTE: Also handled inplace operators. */

/* With the current token one of the unary operator symbols, consume
 * it and parse the second operand before returning the combination */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_prod_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_sum_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_shift_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_cmp_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_cmpeq_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_and_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_xor_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_or_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_as_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_land_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_lor_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_cond_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL ast_parse_assign_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs);

/* Check if the given token qualifies for the associated operation parser function. */
#define TOKEN_IS_PROD(tok)   ((tok) == '*' || (tok) == '/' || (tok) == '%' || (tok) == TPP_TOK_STAR_STAR)
#define TOKEN_IS_SUM(tok)    ((tok) == '+' || (tok) == '-')
#define TOKEN_IS_SHIFT(tok)  ((tok) == TPP_TOK_LANGLE_LANGLE || (tok) == TPP_TOK_RANGLE_RANGLE)
#define TOKEN_IS_CMP(tok)    ((tok) == TPP_TOK_LANGLE || (tok) == TPP_TOK_LANGLE_EQUAL || (tok) == TPP_TOK_RANGLE || (tok) == TPP_TOK_RANGLE_EQUAL)
#define TOKEN_IS_CMPEQ(tok)  ((tok) == TPP_TOK_EQUAL_EQUAL || (tok) == TPP_TOK_EXCLAIM_EQUAL || (tok) == TPP_TOK_EQUAL_EQUAL_EQUAL || (tok) == TPP_TOK_EXCLAIM_EQUAL_EQUAL || (tok) == TPP_TOK_QMARK_QMARK || (tok) == TPP_KWD_is || (tok) == TPP_KWD_in || (tok) == '!')
#define TOKEN_IS_AND(tok)    ((tok) == '&')
#define TOKEN_IS_XOR(tok)    ((tok) == '^')
#define TOKEN_IS_OR(tok)     ((tok) == '|')
#define TOKEN_IS_AS(tok)     ((tok) == TPP_KWD_as)
#define TOKEN_IS_LAND(tok)   ((tok) == TPP_TOK_AMP_AMP)
#define TOKEN_IS_LOR(tok)    ((tok) == TPP_TOK_PIPE_PIPE)
#define TOKEN_IS_COND(tok)   ((tok) == '?')
#define TOKEN_IS_ASSIGN(tok) ((tok) == TPP_TOK_COLON_EQUAL || (tok) == TPP_TOK_PLUS_EQUAL || (tok) == TPP_TOK_MINUS_EQUAL || (tok) == TPP_TOK_STAR_EQUAL || (tok) == TPP_TOK_SLASH_EQUAL || (tok) == TPP_TOK_PERCENT_EQUAL || (tok) == TPP_TOK_LANGLE_LANGLE_EQUAL || (tok) == TPP_TOK_RANGLE_RANGLE_EQUAL || (tok) == TPP_TOK_AMP_EQUAL || (tok) == TPP_TOK_PIPE_EQUAL || (tok) == TPP_TOK_HAT_EQUAL || (tok) == TPP_TOK_STAR_STAR_EQUAL)
#define CASE_TOKEN_IS_PROD   case '*': case '/': case '%': case TPP_TOK_STAR_STAR
#define CASE_TOKEN_IS_SUM    case '+': case '-'
#define CASE_TOKEN_IS_SHIFT  case TPP_TOK_LANGLE_LANGLE: case TPP_TOK_RANGLE_RANGLE
#define CASE_TOKEN_IS_CMP    case TPP_TOK_LANGLE: case TPP_TOK_LANGLE_EQUAL: case TPP_TOK_RANGLE: case TPP_TOK_RANGLE_EQUAL
#define CASE_TOKEN_IS_CMPEQ  case TPP_TOK_EQUAL_EQUAL: case TPP_TOK_EXCLAIM_EQUAL: case TPP_TOK_EQUAL_EQUAL_EQUAL: case TPP_TOK_EXCLAIM_EQUAL_EQUAL: case TPP_TOK_QMARK_QMARK: case TPP_KWD_is: case TPP_KWD_in: case '!'
#define CASE_TOKEN_IS_AND    case '&'
#define CASE_TOKEN_IS_XOR    case '^'
#define CASE_TOKEN_IS_OR     case '|'
#define CASE_TOKEN_IS_AS     case TPP_KWD_as
#define CASE_TOKEN_IS_LAND   case TPP_TOK_AMP_AMP
#define CASE_TOKEN_IS_LOR    case TPP_TOK_PIPE_PIPE
#define CASE_TOKEN_IS_COND   case '?'
#define CASE_TOKEN_IS_ASSIGN case TPP_TOK_COLON_EQUAL: case TPP_TOK_PLUS_EQUAL: case TPP_TOK_MINUS_EQUAL: case TPP_TOK_STAR_EQUAL: case TPP_TOK_SLASH_EQUAL: case TPP_TOK_PERCENT_EQUAL: \
                             case TPP_TOK_LANGLE_LANGLE_EQUAL: case TPP_TOK_RANGLE_RANGLE_EQUAL: case TPP_TOK_AMP_EQUAL: case TPP_TOK_PIPE_EQUAL: case TPP_TOK_HAT_EQUAL: case TPP_TOK_STAR_STAR_EQUAL


/* Parse a top-level expression. */
#define ast_parse_expr(self, lookup_mode) ast_parse_assign(self, lookup_mode)

/* Given a basic unary expression `ast`, parse its unary
 * suffix (including attribute, call, range & item operators). */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_unary_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict baseexpr);

/* Given a unary expression `ast`, parse anything that may
 * follow it before it could be considered a full expression. */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_postexpr(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict baseexpr);


/* Given an `key`-expression in `{ key : foo }`, parse the remainder
 * of a brace expression with the current token being the one after the `:` */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_mapping(DeeLexer *self, struct ast *__restrict initial_key);

/* Given an `item`-expression in `{ item, foo }`, parse the remainder
 * of a brace expression with the current token being a `,` */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_brace_list(DeeLexer *self, struct ast *__restrict initial_item);



/* Parse an import statement/expression.
 * @param: allow_symbol_define: When true, allow the `import foo = x from y;` syntax,
 *                              as well as define local symbols `x, y` for `import x, y from z`
 *                              When false, no symbols are defined by the AST, and the returned
 *                              AST contains either a single, or multiple symbols describing
 *                              what was imported either as an `AST_SYM` (single) or as an
 *                             `AST_MULTIPLE:AST_FMULTIPLE_TUPLE` (more than one symbol).
 * >> foo = import x from y;
 *          ^ Entry        ^ exit
 * >> import foo = bar from baz;
 *    ^ Entry                  ^ exit
 * >> import("foo");
 *    ^ Entry      ^ exit
 * >> from foo import bar;
 *    ^ Entry            ^ exit
 * Accepted formats:
 * >> from <module-name> import <import-list>
 * >> import <import-list> from <module-name>
 * >> import <module-list>
 * >> import <symbol-name> = <module-name>
 * >> import <symbol-name> = <import-name> from <module-name>
 * >> import(<expression>)
 */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_import(DeeLexer *self);

/* Parse a module name and generate an AST to reference a single symbol `import_name`. */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_import_single(DeeLexer *self, tpp_keyword const *__restrict import_name);

INTDEF WUNUSED NONNULL((1, 2)) struct symbol *DFCALL
ast_parse_import_single_sym(DeeLexer *self, tpp_keyword const *__restrict import_name);

/* Parse a comma-separated list of expressions,
 * as well as assignment/inplace expressions.
 * >> foo = 42;               // (foo = (42));
 * >> foo += 42;              // (foo += (42));
 * >> foo, bar = (10, 20)...; // (foo, bar = (10, 20)...);
 * >> foo, bar = 10;          // (foo, (bar = 10));
 * >> { 10 }                  // (List { 10 }); // When `AST_COMMA_ALLOWBRACE` is set
 * >> { "foo": 10 }           // (Dict { "foo": 10 }); // When `AST_COMMA_ALLOWBRACE` is set
 * @param: mode:       Set of `AST_COMMA_*`     - What is allowed and when should we pack values.
 * @param: flags:      Set of `AST_FMULTIPLE_*` - How should multiple values be packaged.
 * @param: p_out_mode: When non-NULL, instead of parsing a `;` when required,
 *                     set to `AST_COMMA_OUT_FNEEDSEMI` indicative of this. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_comma(DeeLexer *self, uint16_t mode, uint16_t flags, uint16_t *p_out_mode);
#define AST_COMMA_NORMAL        0x0000
#define AST_COMMA_FORCEMULTIPLE 0x0001 /* Always pack objects according to `flags` */
#define AST_COMMA_STRICTCOMMA   0x0002 /* Strictly enforce the rule of a `,` being followed by another expression.
                                        * NOTE: When this flag is set, trailing `,` are not parsed, but remain as the active token upon exit. */
#define AST_COMMA_ALLOWNONBLOCK 0x0040 /* Allow non-blocking yields for a trailing `;`. */
#define AST_COMMA_NOSUFFIXKWD   0x0080 /* Don't parse c-style variable declarations for reserved keywords.
                                        * This is required for `else`, `catch`, `finally`, etc.
                                        * >> `try foo catch (...)` (don't interpret as `local catch = foo(...)` when starting with `foo`) */
#define AST_COMMA_ALLOWTYPEDECL 0x0800 /* Allow type declaration to be appended to variables, as well as documentation strings to be consumed. */
#define AST_COMMA_ALLOWKWDLIST  0x1000 /* Stop if what a keyword list label is encountered. */
#define AST_COMMA_PARSESINGLE   0x2000 /* Only parse a single expression. */
#define AST_COMMA_PARSESEMI     0x4000 /* Parse a `;` as part of the expression (if a `;` is required). */
#define AST_COMMA_ALLOWVARDECLS 0x8000 /* Allow new variables to be declared. */

#define AST_COMMA_OUT_FNORMAL   0x0000 /* Normal comma output flags. */
#define AST_COMMA_OUT_FNEEDSEMI 0x0001 /* Set if a semicolon is required. */


/* Parse an argument list using `ast_parse_comma`,
 * and (if present) also parse a trailing keyword label list, which is then saved as a
 * constant ast and returned through `*p_keyword_labels`.
 * If no keyword labels are present, `*p_keyword_labels` is filled in as `NULL`
 * @param: mode: Set of `AST_COMMA_*` - What is allowed and when should we pack values. */
INTDEF WUNUSED NONNULL((1, 3)) DREF struct ast *DFCALL
ast_parse_argument_list(DeeLexer *self, uint16_t mode,
                        /*out*/ DREF struct ast **__restrict p_keyword_labels);


/* Parse lookup mode modifiers:
 * >> local x = 42;
 *    ^     ^
 */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
ast_parse_lookup_mode(DeeLexer *self, unsigned int *__restrict p_mode);

/* Return 1 if the current token may be the begin of an expression.
 * @return: 1:  Yes
 * @return: 0:  No
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DFCALL
maybe_expression_begin(DeeLexer *self);

/* Same as `maybe_expression_begin()`, but for the next (peeked) token. */
INTDEF WUNUSED NONNULL((1)) int DFCALL
maybe_expression_begin_peek(DeeLexer *self);

/* Parse a try-statement/expression. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_try(DeeLexer *self, bool is_statement);

/* Parse a with-statement/expression.
 * NOTE: This function expects the current token to be `with` */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_with(DeeLexer *self, bool is_statement, bool allow_nonblock);

/* Parse a regular, old statement. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statement(DeeLexer *self, bool allow_nonblock);

/* Parse a sequence of statements until `end_token` is
 * encountered at the start of a statement, or until
 * the end of the current input-file-stack is reached.
 * NOTE: The returned ast is usually an `AST_MULTIPLE`,
 *       which will have the given `flags` assigned.
 * WARNING: If only a single AST would be contained
 *          and `flags` is `AST_FMULTIPLE_KEEPLAST`,
 *          the inner expression is automatically
 *          returned instead.
 * NOTE: If desired, the caller is responsible to setup
 *       or teardown a new scope before/after this function. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statements_until(DeeLexer *self, uint16_t flags, tpp_token_id end_token);

/* Parse and return an operator name.
 * @param: features: Set of `P_OPERATOR_F*`
 * @return: * : One of `OPERATOR_*` or `AST_OPERATOR_*`
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1)) int32_t DFCALL
ast_parse_operator_name(DeeLexer *self, uint16_t features);

#define P_OPERATOR_FNORMAL 0x0000 /* Normal features */
#define P_OPERATOR_FCLASS  0x0001 /* Allow class-specific operator names. */


/* Ambiguous operator codes.
 * The caller should resolved these based on operand count. */
#define AST_OPERATOR_POS_OR_ADD           0xf000 /* `+` */
#define AST_OPERATOR_NEG_OR_SUB           0xf001 /* `-` */
#define AST_OPERATOR_GETITEM_OR_SETITEM   0xf002 /* `[]` */
#define AST_OPERATOR_GETRANGE_OR_SETRANGE 0xf003 /* `[:]` */
#define AST_OPERATOR_GETATTR_OR_SETATTR   0xf004 /* `.` */
#define AST_OPERATOR_MIN                  0xf000
#define AST_OPERATOR_MAX                  0xf004

/* Special class operators. */
#define AST_OPERATOR_FOR                  0xf005 /* `for` */
#define AST_OPERATOR_STR_OR_PRINT         0xf006 /* `str` */
#define AST_OPERATOR_REPR_OR_PRINTREPR    0xf007 /* `repr` */
#define AST_OPERATOR_MAX_FOR_CLASS        0xf007

/* Build a call to an operator, given the operator's name and arguments.
 * @param: name:  One of `OPERATOR_*` or `AST_OPERATOR_*`
 * @param: flags: Set of `AST_OPERATOR_F*`
 *       WARNING: This flags set may not contain `AST_OPERATOR_FVARARGS`! */
INTDEF WUNUSED NONNULL((3, 4)) DREF struct ast *DCALL
ast_build_bound_operator(Dee_operator_t name, uint16_t flags,
                         struct ast *__restrict self,
                         struct ast *__restrict args);

/* Same as `ast_build_bound_operator`, but used to build free-standing operators. */
INTDEF WUNUSED NONNULL((3)) DREF struct ast *DCALL
ast_build_operator(Dee_operator_t name, uint16_t flags,
                   struct ast *__restrict args);

/* Parse a loop statement that appears in an expression:
 * When called, the current token must be one of
 * `TPP_KWD_for`, `TPP_KWD_foreach`, `TPP_KWD_while` or `TPP_KWD_do`
 * The returned expression is usually a call-operator
 * on an anonymous lambda function. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_loopexpr(DeeLexer *self);

/* Parse a new function declaration, starting at either the argument
 * list, or when not present at the following `->` or `{` token.
 * The returned AST is of type `AST_FUNCTION`.
 * NOTE: The caller is responsible for allocating a symbol in their
 *       scope if the desire is to address the function by name.
 *       This parser function will merely return the `AST_FUNCTION`,
 *       not some wrapper that assigns it to a symbol using `AST_STORE`. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function(DeeLexer *self, tpp_keyword const *name, bool *p_need_semi,
                   bool allow_missing_params, struct ast_loc *name_loc,
                   struct decl_ast *decl, /*[0..1]*/ struct symbol *function_symbol);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function_noscope(DeeLexer *self, tpp_keyword const *name, bool *p_need_semi,
                           bool allow_missing_params, struct ast_loc *name_loc,
                           struct decl_ast *decl, /*[0..1]*/ struct symbol *function_symbol);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function_noscope_noargs(DeeLexer *self, bool *p_need_semi);

/* Parse a java-style lambda. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_function_java_lambda(DeeLexer *self,
                               tpp_keyword const *first_argument_name,
                               struct ast_loc *first_argument_loc);

/* Check if the parser is located after the '(' of a java-style lambda.
 * @return:  1: Yes
 * @return:  0: No
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DFCALL
ast_is_after_lparen_of_java_lambda(DeeLexer *self);

/* Parse everything following a `del` keyword in a statement, or expression:
 * >> foo = 7;
 * >> print foo;
 * >> del foo;        // Unbind + delete
 *        ^  ^
 * >> foo = 42;
 * >> print foo;
 * >> print del(foo); // Unbind only
 *              ^  ^
 * NOTE: When `LOOKUP_SYM_ALLOWDECL` is set in `lookup_mode`,
 *       the function is allocated to delete locally defined
 *       variable symbols.
 *       However, in all cases is this function allowed to
 *       unbind variables (deleting only referring to the
 *       compile-time symbol becoming unknown and being added to
 *       the current scope's chain of deleted/anonymous symbols) */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_del(DeeLexer *self, unsigned int lookup_mode);

/* Parse a user-defined assembly block. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_asm(DeeLexer *self);

/* Parse the argument list of a function definition,
 * automatically creating new symbols for arguments,
 * as well as setting code flags for variadic arguments. */
INTDEF WUNUSED NONNULL((1)) int DFCALL
parse_arglist(DeeLexer *self);

/* Parse the contents of a brace initializer,
 * starting after the '{' token and ending on '}'. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_brace_items(DeeLexer *self);

/* Parse a class definition, starting at the `{` token (or at `:` when a base exists).
 * The returned AST is of type `AST_CLASS` (create_symbol == false) or `AST_STORE` (create_symbol == true).
 * @param: class_flags:   Set of `TP_F* & 0xf`
 * @param: create_symbol: When true, assign the class to its own symbol (also requiring that `name` != NULL).
 * @param: symbol_mode:   The mode with which to create the class symbol. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_class(DeeLexer *self, uint16_t class_flags, tpp_keyword const *name,
                bool create_symbol, unsigned int symbol_mode);

/* Parse the head header of a for-statement, returning the appropriate
 * AST flags for creating the loop (usually `AST_FLOOP_NORMAL` or `AST_FLOOP_FOREACH`),
 * as well as filling in the given pointers to used asts.
 * NOTE: The caller is responsible for wrapping this function in its own
 *       scope, should they choose to with initializers/loop element symbols
 *       to be placed in their own scope.
 * NOTE: Any of the given pointers may be filled with NULL if that AST is not present,
 *       unless the loop is actually a foreach-loop, in which case they _must_ always
 *       be present.
 * WARNING: The caller is responsible for wrapping `*p_iter_or_next` in an `__iterself__()`
 *          operator call when `AST_FLOOP_FOREACH` is part of the return mask, unless they wish
 *          to enumerate an iterator itself (which is possible using the `__foreach` statement). */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int32_t DFCALL
ast_parse_for_head(DeeLexer *self,
                   DREF struct ast **__restrict p_init,
                   DREF struct ast **__restrict p_elem_or_cond,
                   DREF struct ast **__restrict p_iter_or_next);

/* Parse an assertion statement. (must be started ontop of the `assert` keyword) */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_assert(DeeLexer *self, bool needs_parenthesis);

/* Parse a cast expression suffix following parenthesis, or
 * re-return the given `typeexpr` if there is no cast operand
 * at the current lexer position:
 * >> local x = (int)get_value();
 *                   ^          ^
 *                   entry      exit
 * >> local y = (get_value());
 *                           ^
 *                           entry
 *                           ^
 *                           exit
 */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_cast(DeeLexer *self, struct ast *__restrict typeexpr);




#define AST_PARSE_WASEXPR_NO     0 /* It's a statement. */
#define AST_PARSE_WASEXPR_YES    1 /* It's an expression for sure. */
#define AST_PARSE_WASEXPR_MAYBE  2 /* It could either be an expression, or a statement. */

/* @param: p_was_expression: When non-NULL, set to one of `AST_PARSE_WASEXPR_*` */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statement_or_expression(DeeLexer *self, unsigned int *p_was_expression);

/* Parse a primary and second expression in hybrid mode. */
#define ast_parse_hybrid_primary(self, p_was_expression) \
	ast_parse_statement_or_expression(self, p_was_expression)
LOCAL WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_hybrid_secondary(DeeLexer *self, unsigned int *__restrict p_was_expression) {
	DREF struct ast *result;
	switch (*p_was_expression) {
	case AST_PARSE_WASEXPR_NO:
		result = ast_parse_statement(self, false);
		break;
	case AST_PARSE_WASEXPR_YES:
		result = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		break;
	case AST_PARSE_WASEXPR_MAYBE:
		result = ast_parse_statement_or_expression(self, p_was_expression);
		break;
	default: __builtin_unreachable();
	}
	return result;
}




/* Parse a statement or a brace-expression, with the current token being a `{` */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statement_or_braces(DeeLexer *self, unsigned int *p_was_expression);

/* With the current token being `try`, parse the construct and
 * try to figure out if it's a statement or an expression. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_try_hybrid(DeeLexer *self, unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid` but for if statements / expressions. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_if_hybrid(DeeLexer *self, unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid` but for with statements / expressions. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_with_hybrid(DeeLexer *self, unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid` but for assert statements / expressions. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_assert_hybrid(DeeLexer *self, unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid` but for import statements / expressions. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_import_hybrid(DeeLexer *self, unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid` but for loopexpr statements / expressions. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_loopexpr_hybrid(DeeLexer *self, unsigned int *p_was_expression);





struct Dee_module_object;

/* Special return value to the module currently being compiled. */
#define MODULE_CURRENT                ((struct Dee_module_object *)ITER_DONE)
#define decref_parse_module_byname(x) ((x) == MODULE_CURRENT ? (void)0 : Dee_Decref(x))

/* Parse a module name and return the associated module object.
 * @param: for_alias: Should be `true` if the name is used in `foo = <name>`,
 *                    or if no alias can be used where the name appears,
 *                    else `false`
 * @return: * :             The named module
 * @return: NULL:           Error was thrown
 * @return: MODULE_CURRENT: The module currently being compiled */
INTDEF WUNUSED NONNULL((1)) DREF struct Dee_module_object *DFCALL
parse_module_byname(DeeLexer *self, bool for_alias);

INTDEF WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL
import_module_symbol(struct Dee_module_object *__restrict mod,
                     tpp_keyword const *__restrict name);



struct astlist {
	size_t            ast_c; /* Amount of branches in use. */
	size_t            ast_a; /* [>= ast_c] Allocated amount of branches. */
	DREF struct ast **ast_v; /* [1..1][0..ast_c|ALLOC(ast_a)][owned] Vector of branches. */
};
#define ASTLIST_INIT { 0, 0, NULL }

INTDEF NONNULL((1)) void DCALL astlist_fini(struct astlist *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL astlist_upsize(struct astlist *__restrict self, size_t min_add);
INTDEF NONNULL((1)) void DCALL astlist_trunc(struct astlist *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL astlist_append(struct astlist *__restrict self, struct ast *__restrict branch);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL astlist_appendall(struct astlist *__restrict self, struct astlist *__restrict other);



struct ast_annotation {
	/* AST Annotations can be used to transform the value generated by some declaration.
	 * Annotations are implemented through call invocations done at runtime, and
	 * function through one of 2 ways:
	 * >> @MyAnnotation
	 * >> function foo() {
	 * >> }
	 * >> @MyAnnotation("foobar")
	 * >> function bar() {
	 * >> }
	 * Compiled as:
	 * >> [global] final foo = MyAnnotation(function() {
	 * >> });
	 * >> [global] final bar = MyAnnotation(function() {
	 * >> }, "foobar");
	 */
	DREF struct ast       *aa_func;   /* [1..1] The expression invoked for the purposes of
	                                   *        transforming the annotated declaration. */
#define AST_ANNOTATION_FNORMAL 0x0000 /* Normal annotation flags. */
#define AST_ANNOTATION_FNOFUNC 0x0001 /* When set, don't try to insert the annotated object
	                                   * as an argument of an `OPERATOR_CALL` branch in `aa_func`
	                                   * Instead, always pack the annotated object as a single-
	                                   * element argument list, before using it to invoke `aa_func`
	                                   * and continuing to use the returned result. */
	uint16_t               aa_flag;   /* Annotation flags (Set of `AST_ANNOTATION_F*`) */
	uint16_t               aa_pad[(sizeof(void *)-2)/2];
};

struct ast_annotations {
	size_t                 an_annoc; /* Amount of used annotations. */
	size_t                 an_annoa; /* Amount of allocated annotations. */
	struct ast_annotation *an_annov; /* [0..at_anno_c|ALLOC(at_anno_a)][owned]
	                                  * Amount of allocated annotations. */
};

#define AST_ANNOTATIONS_INIT { 0, 0, NULL }

struct ast_tags_printers {
	struct Dee_unicode_printer at_decl; /* A custom declaration overwrite (for creating a custom documentation prefix). */
	struct Dee_unicode_printer at_doc;  /* The documentation string that should be applied to the following declaration. */
};

struct ast_tags {
	struct Dee_unicode_printer at_decl;         /* A custom declaration overwrite (for creating a custom documentation prefix). */
	struct Dee_unicode_printer at_doc;          /* The documentation string that should be applied to the following declaration. */
	struct ast_annotations     at_anno;         /* AST Annotations. */
	uint16_t                   at_expect;       /* Set of `AST_FCOND_LIKELY|AST_FCOND_UNLIKELY` */
	uint16_t                   at_class_flags;  /* Set of `TP_F*` or'd to flags during creation of a new class. */
	uint16_t                   at_code_flags;   /* Set of `CODE_F*` or'd to flags during creation of a new function. */
	uint16_t                   at_attr_flags;   /* Set of `CLASS_ATTRIBUTE_F*` or'd to flags during creation of a new class attribute. */
};

#define AST_TAGS_BACKUP_PRINTERS(buf)                                       \
	(memcpy(&(buf), &current_tags, 2 * sizeof(struct Dee_unicode_printer)), \
	 bzero(&current_tags, 2 * sizeof(struct Dee_unicode_printer)))
#define AST_TAGS_RESTORE_PRINTERS(buf)                \
	(Dee_unicode_printer_fini(&current_tags.at_decl), \
	 Dee_unicode_printer_fini(&current_tags.at_doc),  \
	 memcpy(&current_tags, &(buf), 2 * sizeof(struct Dee_unicode_printer)))


/* Current set of active AST tags.
 * This set is cleared at least every time a statement ends,
 * but is cleared more than that when inside of a class branch,
 * or when declaring a function. */
INTDEF struct ast_tags current_tags;

/* Reset the current set of active tags to an empty (unallocated) state. */
INTDEF WUNUSED NONNULL((1)) int DFCALL ast_tags_clear(DeeLexer *self);

/* Pack together the current documentation string. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_tags_doc(struct decl_ast const *__restrict decl);

/* Escape documentation text from "Encoded Documentation Text"
 * into "Fully Encoded Documentation Text" */
INTDEF WUNUSED NONNULL((1)) int DCALL
doctext_escape(struct Dee_unicode_printer *__restrict doctext);

/* Add a new annotation to the current set of tags. */
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_annotations_add(struct ast *__restrict func, uint16_t flag);

/* Capture all currently saved annotations. */
INTDEF NONNULL((1)) void DCALL ast_annotations_get(struct ast_annotations *__restrict result);
INTDEF NONNULL((1)) void DCALL ast_annotations_free(struct ast_annotations *__restrict self);

/* Clear annotations, and warn if some were given. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
ast_annotations_clear(DeeLexer *lexer, struct ast_annotations *__restrict self);

/* Apply & free annotations to the given `input` ast. */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *DCALL
ast_annotations_apply(struct ast_annotations *__restrict self,
                      /*inherit(always)*/ DREF struct ast *__restrict input);



/* Parse tags at the current lexer position, starting
 * immediately at the `@` token.
 * >> @doc("foo"), doc("bar")
 *    ^                      ^
 *    entry                  exit */
INTDEF WUNUSED NONNULL((1)) int DFCALL
parse_tags_block(DeeLexer *self);
#endif /* !CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_PARSER_H */
