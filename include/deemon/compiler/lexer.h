/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_LEXER_H
#define GUARD_DEEMON_COMPILER_LEXER_H 1

#include "../api.h"

#define PARSE_FNORMAL 0x0000 /* Normal parser flags. */
#define PARSE_FLFSTMT 0x0001 /* Parse line-feeds as statement terminators in certain places. */

#ifdef CONFIG_BUILDING_DEEMON
#include "../object.h" /* Dee_operator_t */
#include "../string.h"
#include "../system-features.h" /* memcpy */
#include "../types.h"
#include "ast.h"
#include "symbol.h"
#include "tpp.h"
/**/

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

struct unicode_printer;
struct compiler_options;
struct module_symbol;

/* Parser flags (Set of `PARSE_F*') */
INTDEF uint16_t parser_flags;
INTDEF struct compiler_options *inner_compiler_options;


/* Parse a string. */
INTDEF WUNUSED DREF DeeObject *DFCALL ast_parse_string(void);

/* Parse a template string. */
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_template_string(void);

/* Decode the current token (which must be a TOK_STRING) as a unicode string. */
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_decode_unicode_string(struct unicode_printer *__restrict printer);

#define PARSE_UNARY_DISALLOW_CASTS 0x10000

/* @param: lookup_mode: Set of `LOOKUP_SYM_*', optionally or'd with `PARSE_UNARY_DISALLOW_CASTS' */
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_unaryhead(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_unary(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_prod(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_sum(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_shift(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_cmp(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_cmpeq(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_and(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_xor(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_or(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_as(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_land(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_lor(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_cond(unsigned int lookup_mode);
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_assign(unsigned int lookup_mode); /* NOTE: Also handled inplace operators. */

/* With the current token one of the unary operator symbols, consume
 * it and parse the second operand before returning the combination */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_prod_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_sum_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_shift_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_cmp_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_cmpeq_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_and_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_xor_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_or_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_as_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_land_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_lor_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_cond_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_parse_assign_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs);

/* Check if the given token qualifies for the associated operation parser function. */
#define TOKEN_IS_PROD(tok)   ((tok) == '*' || (tok) == '/' || (tok) == '%' || (tok) == TOK_POW)
#define TOKEN_IS_SUM(tok)    ((tok) == '+' || (tok) == '-')
#define TOKEN_IS_SHIFT(tok)  ((tok) == TOK_SHL || (tok) == TOK_SHR)
#define TOKEN_IS_CMP(tok)    ((tok) == TOK_LOWER || (tok) == TOK_LOWER_EQUAL || (tok) == TOK_GREATER || (tok) == TOK_GREATER_EQUAL)
#define TOKEN_IS_CMPEQ(tok)  ((tok) == TOK_EQUAL || (tok) == TOK_NOT_EQUAL || (tok) == TOK_EQUAL3 || (tok) == TOK_NOT_EQUAL3 || (tok) == TOK_QMARK_QMARK || (tok) == KWD_is || (tok) == KWD_in || (tok) == '!')
#define TOKEN_IS_AND(tok)    ((tok) == '&')
#define TOKEN_IS_XOR(tok)    ((tok) == '^')
#define TOKEN_IS_OR(tok)     ((tok) == '|')
#define TOKEN_IS_AS(tok)     ((tok) == KWD_as)
#define TOKEN_IS_LAND(tok)   ((tok) == TOK_LAND)
#define TOKEN_IS_LOR(tok)    ((tok) == TOK_LOR)
#define TOKEN_IS_COND(tok)   ((tok) == '?')
#define TOKEN_IS_ASSIGN(tok) ((tok) == TOK_COLON_EQUAL || ((tok) >= TOK_ADD_EQUAL && (tok) <= TOK_POW_EQUAL))

#define CASE_TOKEN_IS_PROD   case '*': case '/': case '%': case TOK_POW
#define CASE_TOKEN_IS_SUM    case '+': case '-'
#define CASE_TOKEN_IS_SHIFT  case TOK_SHL: case TOK_SHR
#define CASE_TOKEN_IS_CMP    case TOK_LOWER: case TOK_LOWER_EQUAL: case TOK_GREATER: case TOK_GREATER_EQUAL
#define CASE_TOKEN_IS_CMPEQ  case TOK_EQUAL: case TOK_NOT_EQUAL: case TOK_EQUAL3: case TOK_NOT_EQUAL3: case TOK_QMARK_QMARK: case KWD_is: case KWD_in: case '!'
#define CASE_TOKEN_IS_AND    case '&'
#define CASE_TOKEN_IS_XOR    case '^'
#define CASE_TOKEN_IS_OR     case '|'
#define CASE_TOKEN_IS_AS     case KWD_as
#define CASE_TOKEN_IS_LAND   case TOK_LAND
#define CASE_TOKEN_IS_LOR    case TOK_LOR
#define CASE_TOKEN_IS_COND   case '?'
#define CASE_TOKEN_IS_ASSIGN case TOK_COLON_EQUAL: case TOK_ADD_EQUAL: case TOK_SUB_EQUAL: case TOK_MUL_EQUAL: case TOK_DIV_EQUAL: case TOK_MOD_EQUAL: \
                             case TOK_SHL_EQUAL: case TOK_SHR_EQUAL: case TOK_AND_EQUAL: case TOK_OR_EQUAL: case TOK_XOR_EQUAL: case TOK_POW_EQUAL


/* Parse a top-level expression. */
#define ast_parse_expr(lookup_mode) ast_parse_assign(lookup_mode)

/* Given a basic unary expression `ast', parse its unary
 * suffix (including attribute, call, range & item operators). */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_unary_operand(/*inherit(always)*/ DREF struct ast *__restrict baseexpr);

/* Given a unary expression `ast', parse anything that may
 * follow it before it could be considered a full expression. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_postexpr(/*inherit(always)*/ DREF struct ast *__restrict baseexpr);


/* Given an `key'-expression in `{ key : foo }', parse the remainder
 * of a brace expression with the current token being the one after the `:' */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_mapping(struct ast *__restrict initial_key);

/* Given an `item'-expression in `{ item, foo }', parse the remainder
 * of a brace expression with the current token being a `,' */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_brace_list(struct ast *__restrict initial_item);



/* Parse an import statement/expression.
 * @param: allow_symbol_define: When true, allow the `import foo = x from y;' syntax,
 *                              as well as define local symbols `x, y' for `import x, y from z'
 *                              When false, no symbols are defined by the AST, and the returned
 *                              AST contains either a single, or multiple symbols describing
 *                              what was imported either as an `AST_SYM' (single) or as an
 *                             `AST_MULTIPLE:AST_FMULTIPLE_TUPLE' (more than one symbol).
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
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_import(void);

/* Parse a module name and generate an AST to reference a single symbol `import_name'. */
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_import_single(struct TPPKeyword *__restrict import_name);

INTDEF WUNUSED NONNULL((1)) struct symbol *DFCALL
ast_parse_import_single_sym(struct TPPKeyword *__restrict import_name);

/* Parse a comma-separated list of expressions,
 * as well as assignment/inplace expressions.
 * >> foo = 42;               // (foo = (42));
 * >> foo += 42;              // (foo += (42));
 * >> foo, bar = (10, 20)...; // (foo, bar = (10, 20)...);
 * >> foo, bar = 10;          // (foo, (bar = 10));
 * >> { 10 }                  // (List { 10 }); // When `AST_COMMA_ALLOWBRACE' is set
 * >> { "foo": 10 }           // (Dict { "foo": 10 }); // When `AST_COMMA_ALLOWBRACE' is set
 * @param: mode:       Set of `AST_COMMA_*'     - What is allowed and when should we pack values.
 * @param: flags:      Set of `AST_FMULTIPLE_*' - How should multiple values be packaged.
 * @param: p_out_mode: When non-NULL, instead of parsing a `;' when required,
 *                     set to `AST_COMMA_OUT_FNEEDSEMI' indicative of this. */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_comma(uint16_t mode, uint16_t flags,
                uint16_t *p_out_mode);
#define AST_COMMA_NORMAL        0x0000
#define AST_COMMA_FORCEMULTIPLE 0x0001 /* Always pack objects according to `flags' */
#define AST_COMMA_STRICTCOMMA   0x0002 /* Strictly enforce the rule of a `,' being followed by another expression.
                                        * NOTE: When this flag is set, trailing `,' are not parsed, but remain as the active token upon exit. */
#define AST_COMMA_ALLOWNONBLOCK 0x0040 /* Allow non-blocking yields for a trailing `;'. */
#define AST_COMMA_NOSUFFIXKWD   0x0080 /* Don't parse c-style variable declarations for reserved keywords.
                                        * This is required for `else', `catch', `finally', etc.
                                        * >> `try foo catch (...)' (don't interpret as `local catch = foo(...)' when starting with `foo') */
#define AST_COMMA_ALLOWTYPEDECL 0x0800 /* Allow type declaration to be appended to variables, as well as documentation strings to be consumed. */
#define AST_COMMA_ALLOWKWDLIST  0x1000 /* Stop if what a keyword list label is encountered. */
#define AST_COMMA_PARSESINGLE   0x2000 /* Only parse a single expression. */
#define AST_COMMA_PARSESEMI     0x4000 /* Parse a `;' as part of the expression (if a `;' is required). */
#define AST_COMMA_ALLOWVARDECLS 0x8000 /* Allow new variables to be declared. */

#define AST_COMMA_OUT_FNORMAL   0x0000 /* Normal comma output flags. */
#define AST_COMMA_OUT_FNEEDSEMI 0x0001 /* Set if a semicolon is required. */


/* Parse an argument list using `ast_parse_comma',
 * and (if present) also parse a trailing keyword label list, which is then saved as a
 * constant ast and returned through `*p_keyword_labels'.
 * If no keyword labels are present, `*p_keyword_labels' is filled in as `NULL'
 * @param: mode: Set of `AST_COMMA_*' - What is allowed and when should we pack values. */
INTDEF WUNUSED NONNULL((2)) DREF struct ast *DCALL
ast_parse_argument_list(uint16_t mode,
                        /*out*/ DREF struct ast **__restrict p_keyword_labels);


/* Parse lookup mode modifiers:
 * >> local x = 42;
 *    ^     ^
 */
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_parse_lookup_mode(unsigned int *__restrict p_mode);

/* Return 1 if the current token may be the begin of an expression.
 * @return: 1:  Yes
 * @return: 0:  No
 * @return: -1: Error */
INTDEF WUNUSED int DCALL
maybe_expression_begin(void);

/* Same as `maybe_expression_begin()', but for the next (peeked) token. */
INTDEF WUNUSED int DCALL
maybe_expression_begin_peek(void);

/* Parse a try-statement/expression. */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_try(bool is_statement);

/* Parse a with-statement/expression.
 * NOTE: This function expects the current token to be `with' */
INTDEF WUNUSED DREF struct ast *DFCALL
ast_parse_with(bool is_statement, bool allow_nonblock);

/* Parse a regular, old statement. */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_statement(bool allow_nonblock);

/* Parse a sequence of statements until `end_token' is
 * encountered at the start of a statement, or until
 * the end of the current input-file-stack is reached.
 * NOTE: The returned ast is usually an `AST_MULTIPLE',
 *       which will have the given `flags' assigned.
 * WARNING: If only a single AST would be contained
 *          and `flags' is `AST_FMULTIPLE_KEEPLAST',
 *          the inner expression is automatically
 *          returned instead.
 * NOTE: If desired, the caller is responsible to setup
 *       or teardown a new scope before/after this function. */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_statements_until(uint16_t flags, tok_t end_token);

/* Parse and return an operator name.
 * @param: features: Set of `P_OPERATOR_F*'
 * @return: * : One of `OPERATOR_*' or `AST_OPERATOR_*'
 * @return: -1: An error occurred. */
INTDEF WUNUSED int32_t DCALL
ast_parse_operator_name(uint16_t features);

#define P_OPERATOR_FNORMAL 0x0000 /* Normal features */
#define P_OPERATOR_FCLASS  0x0001 /* Allow class-specific operator names. */


/* Ambiguous operator codes.
 * The caller should resolved these based on operand count. */
#define AST_OPERATOR_POS_OR_ADD           0xf000 /* `+' */
#define AST_OPERATOR_NEG_OR_SUB           0xf001 /* `-' */
#define AST_OPERATOR_GETITEM_OR_SETITEM   0xf002 /* `[]' */
#define AST_OPERATOR_GETRANGE_OR_SETRANGE 0xf003 /* `[:]' */
#define AST_OPERATOR_GETATTR_OR_SETATTR   0xf004 /* `.' */
#define AST_OPERATOR_MIN                  0xf000
#define AST_OPERATOR_MAX                  0xf004

/* Special class operators. */
#define AST_OPERATOR_FOR                  0xf005 /* `for' */
#define AST_OPERATOR_STR_OR_PRINT         0xf006 /* `str' */
#define AST_OPERATOR_REPR_OR_PRINTREPR    0xf007 /* `repr' */
#define AST_OPERATOR_MAX_FOR_CLASS        0xf007

/* Build a call to an operator, given the operator's name and arguments.
 * @param: name:  One of `OPERATOR_*' or `AST_OPERATOR_*'
 * @param: flags: Set of `AST_OPERATOR_F*'
 *       WARNING: This flags set may not contain `AST_OPERATOR_FVARARGS'! */
INTDEF WUNUSED NONNULL((3, 4)) DREF struct ast *DCALL
ast_build_bound_operator(Dee_operator_t name, uint16_t flags,
                         struct ast *__restrict self,
                         struct ast *__restrict args);

/* Same as `ast_build_bound_operator', but used to build free-standing operators. */
INTDEF WUNUSED NONNULL((3)) DREF struct ast *DCALL
ast_build_operator(Dee_operator_t name, uint16_t flags,
                   struct ast *__restrict args);

/* Parse a loop statement that appears in an expression:
 * When called, the current token must be one of
 * `KWD_for', `KWD_foreach', `KWD_while' or `KWD_do'
 * The returned expression is usually a call-operator
 * on an anonymous lambda function. */
INTDEF WUNUSED DREF struct ast *DFCALL
ast_parse_loopexpr(void);

/* Parse a new function declaration, starting at either the argument
 * list, or when not present at the following `->' or `{' token.
 * The returned AST is of type `AST_FUNCTION'.
 * NOTE: The caller is responsible for allocating a symbol in their
 *       scope if the desire is to address the function by name.
 *       This parser function will merely return the `AST_FUNCTION',
 *       not some wrapper that assigns it to a symbol using `AST_STORE'. */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_function(struct TPPKeyword *name, bool *p_need_semi,
                   bool allow_missing_params, struct ast_loc *name_loc,
                   struct decl_ast *decl);
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_function_noscope(struct TPPKeyword *name, bool *p_need_semi,
                           bool allow_missing_params, struct ast_loc *name_loc,
                           struct decl_ast *decl);
#else /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_function(struct TPPKeyword *name, bool *p_need_semi,
                   bool allow_missing_params, struct ast_loc *name_loc);
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_function_noscope(struct TPPKeyword *name, bool *p_need_semi,
                           bool allow_missing_params, struct ast_loc *name_loc);
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_function_noscope_noargs(bool *p_need_semi);

#ifdef CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS
/* Parse a java-style lambda. */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_function_java_lambda(struct TPPKeyword *first_argument_name,
                               struct ast_loc *first_argument_loc);

/* Check if the parser is located after the '(' of a java-style lambda.
 * @return:  1: Yes
 * @return:  0: No
 * @return: -1: Error */
INTDEF WUNUSED int DCALL ast_is_after_lparen_of_java_lambda(void);
#endif /* CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS */

/* Parse everything following a `del' keyword in a statement, or expression:
 * >> foo = 7;
 * >> print foo;
 * >> del foo;        // Unbind + delete
 *        ^  ^
 * >> foo = 42;
 * >> print foo;
 * >> print del(foo); // Unbind only
 *              ^  ^
 * NOTE: When `LOOKUP_SYM_ALLOWDECL' is set in `lookup_mode',
 *       the function is allocated to delete locally defined
 *       variable symbols.
 *       However, in all cases is this function allowed to
 *       unbind variables (deleting only referring to the
 *       compile-time symbol becoming unknown and being added to
 *       the current scope's chain of deleted/anonymous symbols) */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_del(unsigned int lookup_mode);

/* Parse a user-defined assembly block. */
INTDEF WUNUSED DREF struct ast *DCALL ast_parse_asm(void);

/* Parse the argument list of a function definition,
 * automatically creating new symbols for arguments,
 * as well as setting code flags for variadic arguments. */
INTDEF WUNUSED int DCALL parse_arglist(void);

/* Parse the contents of a brace initializer,
 * starting after the '{' token and ending on '}'. */
INTDEF WUNUSED DREF struct ast *DFCALL ast_parse_brace_items(void);

/* Parse a class definition, starting at the `{' token (or at `:' when a base exists).
 * The returned AST is of type `AST_CLASS' (create_symbol == false) or `AST_STORE' (create_symbol == true).
 * @param: class_flags:   Set of `TP_F* & 0xf'
 * @param: create_symbol: When true, assign the class to its own symbol (also requiring that `name' != NULL).
 * @param: symbol_mode:   The mode with which to create the class symbol. */
INTDEF WUNUSED DREF struct ast *DCALL
ast_parse_class(uint16_t class_flags, struct TPPKeyword *name,
                bool create_symbol, unsigned int symbol_mode);

/* Parse the head header of a for-statement, returning the appropriate
 * AST flags for creating the loop (usually `AST_FLOOP_NORMAL' or `AST_FLOOP_FOREACH'),
 * as well as filling in the given pointers to used asts.
 * NOTE: The caller is responsible for wrapping this function in its own
 *       scope, should they choose to with initializers/loop element symbols
 *       to be placed in their own scope.
 * NOTE: Any of the given pointers may be filled with NULL if that AST is not present,
 *       unless the loop is actually a foreach-loop, in which case they _must_ always
 *       be present.
 * WARNING: The caller is responsible for wrapping `*p_iter_or_next' in an `__iterself__()'
 *          operator call when `AST_FLOOP_FOREACH' is part of the return mask, unless they wish
 *          to enumerate an iterator itself (which is possible using the `__foreach' statement). */
INTDEF WUNUSED NONNULL((1, 2, 3)) int32_t DCALL
ast_parse_for_head(DREF struct ast **__restrict p_init,
                   DREF struct ast **__restrict p_elem_or_cond,
                   DREF struct ast **__restrict p_iter_or_next);

/* Parse an assertion statement. (must be started ontop of the `assert' keyword) */
INTDEF WUNUSED DREF struct ast *DFCALL
ast_parse_assert(bool needs_parenthesis);

/* Parse a cast expression suffix following parenthesis, or
 * re-return the given `typeexpr' if there is no cast operand
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
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_cast(struct ast *__restrict typeexpr);




#define AST_PARSE_WASEXPR_NO     0 /* It's a statement. */
#define AST_PARSE_WASEXPR_YES    1 /* It's an expression for sure. */
#define AST_PARSE_WASEXPR_MAYBE  2 /* It could either be an expression, or a statement. */

/* @param: p_was_expression: When non-NULL, set to one of `AST_PARSE_WASEXPR_*' */
INTERN WUNUSED DREF struct ast *DFCALL
ast_parse_statement_or_expression(unsigned int *p_was_expression);

/* Parse a primary and second expression in hybrid mode. */
#define ast_parse_hybrid_primary(p_was_expression) \
	ast_parse_statement_or_expression(p_was_expression)
LOCAL WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_hybrid_secondary(unsigned int *__restrict p_was_expression) {
	DREF struct ast *result;
	switch (*p_was_expression) {
	case AST_PARSE_WASEXPR_NO:
		result = ast_parse_statement(false);
		break;
	case AST_PARSE_WASEXPR_YES:
		result = ast_parse_expr(LOOKUP_SYM_NORMAL);
		break;
	case AST_PARSE_WASEXPR_MAYBE:
		result = ast_parse_statement_or_expression(p_was_expression);
		break;
	default: __builtin_unreachable();
	}
	return result;
}




/* Parse a statement or a brace-expression, with the current token being a `{' */
INTDEF WUNUSED DREF struct ast *DFCALL
ast_parse_statement_or_braces(unsigned int *p_was_expression);

/* With the current token being `try', parse the construct and
 * try to figure out if it's a statement or an expression. */
INTERN WUNUSED DREF struct ast *DFCALL
ast_parse_try_hybrid(unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid' but for if statements / expressions. */
INTERN WUNUSED DREF struct ast *DFCALL
ast_parse_if_hybrid(unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid' but for with statements / expressions. */
INTERN WUNUSED DREF struct ast *DFCALL
ast_parse_with_hybrid(unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid' but for assert statements / expressions. */
INTERN WUNUSED DREF struct ast *DFCALL
ast_parse_assert_hybrid(unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid' but for import statements / expressions. */
INTERN WUNUSED DREF struct ast *DFCALL
ast_parse_import_hybrid(unsigned int *p_was_expression);

/* Same as `ast_parse_try_hybrid' but for loopexpr statements / expressions. */
INTDEF WUNUSED DREF struct ast *DFCALL
ast_parse_loopexpr_hybrid(unsigned int *p_was_expression);





struct module_object;

/* Parse a module name and return the associated module object.
 * @param: for_alias: Should be `true' if the name is used in `foo = <name>',
 *                    or if no alias can be used where the name appears,
 *                    else `false' */
INTDEF WUNUSED DREF struct module_object *DCALL
parse_module_byname(bool for_alias);

INTDEF WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
import_module_symbol(struct module_object *__restrict mod,
                     struct TPPKeyword *__restrict name);



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
	                                   * as an argument of an `OPERATOR_CALL' branch in `aa_func'
	                                   * Instead, always pack the annotated object as a single-
	                                   * element argument list, before using it to invoke `aa_func'
	                                   * and continuing to use the returned result. */
	uint16_t               aa_flag;   /* Annotation flags (Set of `AST_ANNOTATION_F*') */
	uint16_t               aa_pad[(sizeof(void *)-2)/2];
};

struct ast_annotations {
	size_t                 an_annoc; /* Amount of used annotations. */
	size_t                 an_annoa; /* Amount of allocated annotations. */
	struct ast_annotation *an_annov; /* [0..at_anno_c|ALLOC(at_anno_a)][owned]
	                                  * Amount of allocated annotations. */
};

struct ast_tags_printers {
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	struct unicode_printer at_decl; /* A custom declaration overwrite (for creating a custom documentation prefix). */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	struct unicode_printer at_doc;  /* The documentation string that should be applied to the following declaration. */
};

struct ast_tags {
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	struct unicode_printer at_decl;         /* A custom declaration overwrite (for creating a custom documentation prefix). */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	struct unicode_printer at_doc;          /* The documentation string that should be applied to the following declaration. */
	struct ast_annotations at_anno;         /* AST Annotations. */
	uint16_t               at_expect;       /* Set of `AST_FCOND_LIKELY|AST_FCOND_UNLIKELY' */
	uint16_t               at_class_flags;  /* Set of `TP_F*' or'd to flags during creation of a new class. */
	uint16_t               at_code_flags;   /* Set of `CODE_F*' or'd to flags during creation of a new function. */
	uint16_t               at_attr_flags;   /* Set of `CLASS_ATTRIBUTE_F*' or'd to flags during creation of a new class attribute. */
};

#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
#define AST_TAGS_BACKUP_PRINTERS(buf)                                   \
	(memcpy(&(buf), &current_tags, 2 * sizeof(struct unicode_printer)), \
	 bzero(&current_tags, 2 * sizeof(struct unicode_printer)))
#define AST_TAGS_RESTORE_PRINTERS(buf)            \
	(unicode_printer_fini(&current_tags.at_decl), \
	 unicode_printer_fini(&current_tags.at_doc),  \
	 memcpy(&current_tags, &(buf), 2 * sizeof(struct unicode_printer)))
#else /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
#define AST_TAGS_BACKUP_PRINTERS(buf)                               \
	(memcpy(&(buf), &current_tags, sizeof(struct unicode_printer)), \
	 bzero(&current_tags, sizeof(struct unicode_printer)))
#define AST_TAGS_RESTORE_PRINTERS(buf)           \
	(unicode_printer_fini(&current_tags.at_doc), \
	 memcpy(&current_tags, &(buf), sizeof(struct unicode_printer)))
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */


/* Current set of active AST tags.
 * This set is cleared at least every time a statement ends,
 * but is cleared more than that when inside of a class branch,
 * or when declaring a function. */
INTDEF struct ast_tags current_tags;

/* Reset the current set of active tags to an empty (unallocated) state. */
INTDEF WUNUSED int (DCALL ast_tags_clear)(void);

/* Pack together the current documentation string. */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_tags_doc(struct decl_ast const *__restrict decl);
#else /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
INTDEF WUNUSED DREF DeeObject *DCALL ast_tags_doc(void);
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */

#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
/* Escape documentation text from "Encoded Documentation Text"
 * into "Fully Encoded Documentation Text" */
INTDEF WUNUSED NONNULL((1)) int DCALL
doctext_escape(struct unicode_printer *__restrict doctext);
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */

/* Add a new annotation to the current set of tags. */
INTDEF WUNUSED NONNULL((1)) int
(DCALL ast_annotations_add)(struct ast *__restrict func, uint16_t flag);

/* Capture all currently saved annotations. */
INTDEF NONNULL((1)) void (DCALL ast_annotations_get)(struct ast_annotations *__restrict result);
INTDEF NONNULL((1)) void (DCALL ast_annotations_free)(struct ast_annotations *__restrict self);

/* Clear annotations, and warn if some were given. */
INTDEF WUNUSED NONNULL((1)) int
(DCALL ast_annotations_clear)(struct ast_annotations *__restrict self);

/* Apply & free annotations to the given `input' ast. */
INTDEF WUNUSED NONNULL((1, 2)) DREF struct ast *
(DCALL ast_annotations_apply)(struct ast_annotations *__restrict self,
                              /*inherit(always)*/ DREF struct ast *__restrict input);



/* Parse tags at the current lexer position, starting
 * immediately after the (probably) `@' token.
 * >> @doc("foo"), doc("bar")
 *     ^                     ^
 *     entry                 exit
 * Note that for backwards compatibility, deemon still
 * parses `__attribute__', `__attribute' and `__declspec'
 * using this function. */
INTDEF WUNUSED int DCALL parse_tags(void);

/* Same as `parse_tags()', but also parses the leading `@'
 * token and doesn't do anything if that token wasn't found. */
INTDEF WUNUSED int DCALL parse_tags_block(void);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define parse_tags() __builtin_expect(parse_tags(), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_LEXER_H */
