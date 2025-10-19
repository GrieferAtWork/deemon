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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_IPARSER_C
#define GUARD_DEEMON_COMPILER_INTERFACE_IPARSER_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/thread.h>
/**/

#include "../../runtime/kwlist.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN


INTDEF WUNUSED NONNULL((1, 2)) int DCALL
get_scope_lookupmode(DeeObject *__restrict value,
                     unsigned int *__restrict p_result);


#define DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(name, func, IF_SUFFIX, is_suffix) \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                         \
	parser_##name(DeeCompilerWrapperObject *self, size_t argc,                 \
	              DeeObject *const *argv, DeeObject *kw) {                     \
		DREF DeeObject *result = NULL;                                         \
		DeeCompilerAstObject *head;                                            \
		DREF struct ast *result_ast;                                           \
		if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__head, "o:" #name, &head)) \
			goto done;                                                         \
		if (DeeObject_AssertTypeExact(head, &DeeCompilerAst_Type))             \
			goto done;                                                         \
		if (COMPILER_BEGIN(self->cw_compiler))                                 \
			goto done;                                                         \
		if unlikely(head->ci_compiler != self->cw_compiler) {                  \
			err_invalid_ast_compiler(head);                                    \
			goto done_compiler_end;                                            \
		}                                                                      \
		if unlikely(head->ci_value->a_scope->s_base != current_basescope) {    \
			err_invalid_ast_basescope(head, current_basescope);                \
			goto done_compiler_end;                                            \
		}                                                                      \
		ast_incref(head->ci_value);                                            \
		IF_SUFFIX(if (!(is_suffix)) {                                          \
			result_ast = head->ci_value;                                       \
		} else) {                                                              \
			uint16_t old_exceptsz = DeeThread_Self()->t_exceptsz;              \
			result_ast            = func(head->ci_value);                      \
			if unlikely(!result_ast) {                                         \
				if (old_exceptsz == DeeThread_Self()->t_exceptsz) {            \
					result = Dee_None;                                         \
					Dee_Incref(result);                                        \
				}                                                              \
				goto done_compiler_end;                                        \
			}                                                                  \
		}                                                                      \
		result = DeeCompiler_GetAst(result_ast);                               \
		ast_decref_unlikely(result_ast);                                       \
	done_compiler_end:                                                         \
		COMPILER_END();                                                        \
	done:                                                                      \
		return result;                                                         \
	}
#define DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(name, func)                                   \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                                         \
	parser_##name(DeeCompilerWrapperObject *self, size_t argc,                                 \
	              DeeObject *const *argv, DeeObject *kw) {                                     \
		DREF DeeObject *result = NULL;                                                         \
		DREF struct ast *result_ast;                                                           \
		unsigned int lookup_mode;                                                              \
		DeeObject *lookup_mode_ob = Dee_EmptyString;                                           \
		uint16_t old_exceptsz;                                                                 \
		if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lookupmode, "|o:" #name, &lookup_mode_ob)) \
			goto done;                                                                         \
		if (COMPILER_BEGIN(self->cw_compiler))                                                 \
			goto done;                                                                         \
		if unlikely(get_scope_lookupmode(lookup_mode_ob, &lookup_mode))                        \
			goto done_compiler_end;                                                            \
		result_ast   = func(lookup_mode);                                                      \
		old_exceptsz = DeeThread_Self()->t_exceptsz;                                           \
		if unlikely(!result_ast) {                                                             \
			if (old_exceptsz == DeeThread_Self()->t_exceptsz) {                                \
				result = Dee_None;                                                             \
				Dee_Incref(result);                                                            \
			}                                                                                  \
			goto done_compiler_end;                                                            \
		}                                                                                      \
		result = DeeCompiler_GetAst(result_ast);                                               \
		ast_decref_unlikely(result_ast);                                                       \
	done_compiler_end:                                                                         \
		COMPILER_END();                                                                        \
	done:                                                                                      \
		return result;                                                                         \
	}
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_unaryhead, ast_parse_unaryhead)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_unary, ast_parse_unary)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_prod, ast_parse_prod)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_sum, ast_parse_sum)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_shift, ast_parse_shift)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_cmp, ast_parse_cmp)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_cmpeq, ast_parse_cmpeq)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_and, ast_parse_and)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_xor, ast_parse_xor)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_or, ast_parse_or)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_as, ast_parse_as)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_land, ast_parse_land)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_lor, ast_parse_lor)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_cond, ast_parse_cond)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_assign, ast_parse_assign)
#define IF_TRUE(x) x
#define IF_FALSE(x)
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_unarytail, ast_parse_unary_operand, IF_FALSE, )
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_prodtail, ast_parse_prod_operand, IF_TRUE, TOKEN_IS_PROD(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_sumtail, ast_parse_sum_operand, IF_TRUE, TOKEN_IS_SUM(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_shifttail, ast_parse_shift_operand, IF_TRUE, TOKEN_IS_SHIFT(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_cmptail, ast_parse_cmp_operand, IF_TRUE, TOKEN_IS_CMP(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_cmpeqtail, ast_parse_cmpeq_operand, IF_TRUE, TOKEN_IS_CMPEQ(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_andtail, ast_parse_and_operand, IF_TRUE, TOKEN_IS_AND(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_xortail, ast_parse_xor_operand, IF_TRUE, TOKEN_IS_XOR(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_ortail, ast_parse_or_operand, IF_TRUE, TOKEN_IS_OR(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_astail, ast_parse_as_operand, IF_TRUE, TOKEN_IS_AS(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_landtail, ast_parse_land_operand, IF_TRUE, TOKEN_IS_LAND(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_lortail, ast_parse_lor_operand, IF_TRUE, TOKEN_IS_LOR(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_condtail, ast_parse_cond_operand, IF_TRUE, TOKEN_IS_COND(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_assigntail, ast_parse_assign_operand, IF_TRUE, TOKEN_IS_ASSIGN(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_exprtail, ast_parse_postexpr, IF_FALSE, )
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_maptail, ast_parse_mapping, IF_TRUE, tok == (tok_t)':')
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_seqtail, ast_parse_brace_list, IF_TRUE, tok == (tok_t)',')
#undef IF_FALSE
#undef IF_TRUE
#undef DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION
#undef DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
parser_parse_stmt(DeeCompilerWrapperObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DREF struct ast *result_ast;
	uint16_t old_exceptsz;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("parse_stmt", params: """
	bool nonblocking = false;
""", docStringPrefix: "file", err: "done");]]]*/
#define file_parse_stmt_params "nonblocking=!f"
	struct {
		bool nonblocking;
	} args;
	args.nonblocking = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__nonblocking, "|b:parse_stmt", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self->cw_compiler))
		goto done;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	result_ast   = ast_parse_statement(args.nonblocking);
	if unlikely(!result_ast) {
		if (old_exceptsz == DeeThread_Self()->t_exceptsz) {
			result = Dee_None;
			Dee_Incref(result);
		}
		goto done_compiler_end;
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
parser_parse_allstmt(DeeCompilerWrapperObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	DREF struct ast *result_ast;
	tok_t end_token = TOK_EOF;
	uint16_t old_exceptsz;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("parse_allstmt", params: """
	DeeStringObject *end:?X2?Dstring?Dint = (DeeStringObject *)Dee_EmptyString;
""", docStringPrefix: "file", err: "done");]]]*/
#define file_parse_allstmt_params "end:?X2?Dstring?Dint=!P{}"
	struct {
		DeeStringObject *end;
	} args;
	args.end = (DeeStringObject *)Dee_EmptyString;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__end, "|o:parse_allstmt", &args))
		goto done;
/*[[[end]]]*/
	if (COMPILER_BEGIN(self->cw_compiler))
		goto done;
	if (args.end != (DeeStringObject *)Dee_EmptyString) {
		end_token = get_token_from_obj((DeeObject *)args.end, true);
		if unlikely(end_token == TOK_ERR)
			goto done_compiler_end;
	}
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	result_ast = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST,
	                                        end_token);
	if unlikely(!result_ast) {
		if (old_exceptsz == DeeThread_Self()->t_exceptsz) {
			result = Dee_None;
			Dee_Incref(result);
		}
		goto done_compiler_end;
	}
	result = DeeCompiler_GetAst(result_ast);
	ast_decref_unlikely(result_ast);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}


PRIVATE struct type_method tpconst parser_methods[] = {
	TYPE_KWMETHOD("parse_unaryhead", &parser_parse_unaryhead,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a unary head-expression, that is a unary expression without any tail- or binary suffix"),
	TYPE_KWMETHOD("parse_unary", &parser_parse_unary,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a whole unary expression"),
	TYPE_KWMETHOD("parse_prod", &parser_parse_prod,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a product expression"),
	TYPE_KWMETHOD("parse_sum", &parser_parse_sum,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a sum expression"),
	TYPE_KWMETHOD("parse_shift", &parser_parse_shift,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a shift expression"),
	TYPE_KWMETHOD("parse_cmp", &parser_parse_cmp,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a compare expression (non-equal)"),
	TYPE_KWMETHOD("parse_cmpeq", &parser_parse_cmpeq,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a compare expression (equal)"),
	TYPE_KWMETHOD("parse_and", &parser_parse_and,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a bit-wise and expression"),
	TYPE_KWMETHOD("parse_xor", &parser_parse_xor,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a bit-wise xor expression"),
	TYPE_KWMETHOD("parse_or", &parser_parse_or,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a bit-wise or expression"),
	TYPE_KWMETHOD("parse_as", &parser_parse_as,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse an as expression"),
	TYPE_KWMETHOD("parse_land", &parser_parse_land,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a logical and expression"),
	TYPE_KWMETHOD("parse_lor", &parser_parse_lor,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a logical or expression"),
	TYPE_KWMETHOD("parse_cond", &parser_parse_cond,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse a conditional expression"),
	TYPE_KWMETHOD("parse_assign", &parser_parse_assign,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Parse an assignment (not store), or inplace expression"),
	TYPE_KWMETHOD("parse_expr", &parser_parse_assign,
	              "(lookupmode=!P{})->?#Ast\n"
	              "(lookupmode=!0)->?#Ast\n"
	              "Alias for ?#parse_assign"),
	TYPE_KWMETHOD("parse_unarytail", &parser_parse_unarytail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a unary expression (which includes attribute lookups, and call operations)"),
	TYPE_KWMETHOD("parse_prodtail", &parser_parse_prodtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a product expression"),
	TYPE_KWMETHOD("parse_sumtail", &parser_parse_sumtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a sum expression"),
	TYPE_KWMETHOD("parse_shifttail", &parser_parse_shifttail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a shift expression"),
	TYPE_KWMETHOD("parse_cmptail", &parser_parse_cmptail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a cmp expression"),
	TYPE_KWMETHOD("parse_cmpeqtail", &parser_parse_cmpeqtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a cmpeq expression"),
	TYPE_KWMETHOD("parse_andtail", &parser_parse_andtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a and expression"),
	TYPE_KWMETHOD("parse_xortail", &parser_parse_xortail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a xor expression"),
	TYPE_KWMETHOD("parse_ortail", &parser_parse_ortail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a or expression"),
	TYPE_KWMETHOD("parse_astail", &parser_parse_astail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a as expression"),
	TYPE_KWMETHOD("parse_landtail", &parser_parse_landtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a land expression"),
	TYPE_KWMETHOD("parse_lortail", &parser_parse_lortail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a lor expression"),
	TYPE_KWMETHOD("parse_condtail", &parser_parse_condtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a cond expression"),
	TYPE_KWMETHOD("parse_assigntail", &parser_parse_assigntail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a assign expression"),
	TYPE_KWMETHOD("parse_exprtail", &parser_parse_exprtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a expr expression"),
	TYPE_KWMETHOD("parse_maptail", &parser_parse_maptail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a generic mapping expression"),
	TYPE_KWMETHOD("parse_seqtail", &parser_parse_seqtail,
	              "(head:?#Ast)->?#Ast\n"
	              "Parse the tail of a generic sequence expression"),
	TYPE_KWMETHOD("parse_stmt", &parser_parse_stmt,
	              "(bool nonblocking=false)->?#Ast\n"
	              "Parse a statement or ?#parse_comma expression"),
	TYPE_KWMETHOD("parse_allstmt", &parser_parse_allstmt,
	              "(" file_parse_allstmt_params ")->?#Ast\n"
	              "Parse statements (?#parse_stmt) and pack them togerther into "
	              "a multiple/keep-last branch, until the input file ends, or "
	              "a token equal to @end (s.a. :Compiler.lexer.token.op:eq) is "
	              "encountered at the start of a statement"),
	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
parser_getlfstmt(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeBool_For(parser_flags & PARSE_FLFSTMT);
	Dee_Incref(result);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
parser_setlfstmt(DeeCompilerWrapperObject *self,
                 DeeObject *value) {
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	if (newval) {
		parser_flags |= PARSE_FLFSTMT;
	} else {
		parser_flags &= ~PARSE_FLFSTMT;
	}
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE struct type_getset tpconst parser_getsets[] = {
	TYPE_GETSET("allowlfstatements", &parser_getlfstmt, NULL, &parser_setlfstmt,
	            "->?Dbool\n"
	            "Get or set if line-feeds should be recognized as statement terminators "
	            /**/ "(you must also enable line-feed token created in :Compiler.lexer.wantlf)\n"
	            "When enabled, most statements can be termined by line-feeds, in addition to "
	            /**/ "semicolon tokens."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst parser_class_members[] = {
	TYPE_MEMBER_CONST("Ast", &DeeCompilerAst_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeCompilerParser_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Parser",
	/* .tp_doc      = */ DOC("Parse data from the input token stream (" DR_CLexer ") of the associated ?#compiler\n"
	                         "For more information on what exactly is parsed by individual #C{parse_*} functions, see the "
	                         /**/ "following table, or consult the #C{/lib/LANGUAGE.txt} file within the deemon source tree\n"
	                         "#T{Function|Example|Description~"
	                         /**/ "?#parse_unaryhead|${[foo].bar}, ${[42] + 7}, ${[(10 + 20)]}, ${[[]{ return 42; }]}|Parse the head of a unary expression, including unary prefix operators, and the unary operand itself&"
	                         /**/ "?#parse_unary|${[foo.bar]}, ${[getitems(a, b, c)]}|Parse a whole unary expression&"
	                         /**/ "?#parse_prod|${[x * y / z % v ** w]}|Parse product expressions (using ?#parse_unary for operands)&"
	                         /**/ "?#parse_sum|${[x + y - z]}|Parse sum expressions (using ?#parse_prod for operands)&"
	                         /**/ "?#parse_shift|${[x << y >> z]}|Parse shift expressions (using ?#parse_sum for operands)&"
	                         /**/ "?#parse_cmp|${[x < y <= z > v >= w]}|Parse compare expressions (non-equal) (using ?#parse_shift for operands)&"
	                         /**/ "?#parse_cmpeq|${[x == y != z === a !== b is c in d !is e !in f]}|Parse compare expressions (equal) and in/is-expressions (using ?#parse_cmp for operands)&"
	                         /**/ "?#parse_and|${[x & y]}|Parse bit-wise and expressions (using ?#parse_cmpeq for operands)&"
	                         /**/ "?#parse_xor|${[x ^ y]}|Parse bit-wise xor expressions (using ?#parse_and for operands)&"
	                         /**/ "?#parse_or|${[x | y]}|Parse bit-wise or expressions (using ?#parse_xor for operands)&"
	                         /**/ "?#parse_as|${[x as y]}|Parse bit-wise or expressions (using ?#parse_or for operands)&"
	                         /**/ "?#parse_land|${[x && y]}|Parse logical and expressions (using ?#parse_as for operands)&"
	                         /**/ "?#parse_lor|${[x || y]}|Parse logical or expressions (using ?#parse_land for operands)&"
	                         /**/ "?#parse_cond|${[x ? y : z]}, ${[x ? : z]}, ${[x ? y]}, ${[x ? y : ]}|Parse conditional expressions (using ?#parse_lor for `x', and ?#parse_cond for `y' and `z')&"
	                         /**/ "?#parse_assign|${[x := y += z -= v += w /= a %= b <<= c >>= d &= e |= f ^= g **= h]}|Parse assignment expressions (using ?#parse_cond for operands)&"
	                         /**/ "?#parse_expr|-|Alias for ?#parse_assign&"
	                         /**/ "?#parse_unarytail|${foo[.bar]}, ${getitems[(a, b, c)]}|Parse the tail of a unary expression, including all tightly bound suffix expressions&"
	                         /**/ "?#parse_prodtail|${x [* y / z % v ** w]}|Given a ?#parse_unary operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_sumtail|${x [+ y - z]}|Given a ?#parse_prod operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_shifttail|${x [<< y >> z]}|Given a ?#parse_sum operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_cmptail|${x [< y <= z > v >= w]}|Given a ?#parse_shift operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_cmpeqtail|${x [== y != z === a !== b is c in d !is e !in f]}|Given a ?#parse_cmp operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_andtail|${x [& y]}|Given a ?#parse_cmpeq operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_xortail|${x [^ y]}|Given a ?#parse_and operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_ortail|${x [| y]}|Given a ?#parse_xor operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_astail|${x [as y]}|Given a ?#parse_or operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_landtail|${x [&& y]}|Given a ?#parse_as operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_lortail|${x [|| y]}|Given a ?#parse_land operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_condtail|${x [? y : z]}, ${x [? : z]}, ${x [? y]}, ${x [? y : ]}|Given a ?#parse_lor operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_assigntail|${x [:= y += z -= v += w /= a %= b <<= c >>= d &= e |= f ^= g **= h]}|Given a ?#parse_cond operand `x', parse the suffix also parsed by ?#parse_prod&"
	                         /**/ "?#parse_exprtail|${x [<any-non-unary-tail> y]}|Does the work of all the *-tail functions, excluding ?#parse_unarytail&"
	                         /**/ "?#parse_maptail|${{x [: y, v: w]}}|Process the tail in a generic mapping expression. - No-op when the current token isn't a $\":\"&"
	                         /**/ "?#parse_seqtail|${{x [, y, z]}}|Process the tail in a generic sequence expression. - No-op when the current token isn't a $\",\""
	                         "}\n"
	                         "Note the processed portion of an expression is written in []-brackets\n"
	                         "In order to explain what tail parsers do, look at this example:\n"
	                         "${"
	                         /**/ "/* input: \"a.foo + 14 * 7 ? x : y-7\" */\n"
	                         /**/ "x = parser.parse_unaryhead();  /* x = `a' */\n"
	                         /**/ "x = parser.parse_unarytail(x); /* x = `a' + `.foo' (same as an initial call to `parse_unary') */\n"
	                         /**/ "x = parser.parse_exprtail(x);  /* x = `a.foo' + `+ 14 * 7 ? x : y-7' (same as an initial call to `parse_expr') */"
	                         "}\n"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerWrapper_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ parser_methods,
	/* .tp_getsets       = */ parser_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ parser_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_IPARSER_C */
