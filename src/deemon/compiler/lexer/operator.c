/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_LEXER_OPERATOR_C
#define GUARD_DEEMON_COMPILER_LEXER_OPERATOR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/tuple.h>

#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>

#include "../../runtime/strings.h"

DECL_BEGIN


/* Special symbol names to generate function calls to
	* when the argument count cannot be determined. */
INTDEF DeeStringObject *tpconst rt_operator_names[1 + (AST_OPERATOR_MAX - AST_OPERATOR_MIN)];
INTERN_CONST DeeStringObject *tpconst rt_operator_names[1 + (AST_OPERATOR_MAX - AST_OPERATOR_MIN)] = {
	/* [AST_OPERATOR_POS_OR_ADD           - AST_OPERATOR_MIN] = */ &str___pooad,
	/* [AST_OPERATOR_NEG_OR_SUB           - AST_OPERATOR_MIN] = */ &str___neosb,
	/* [AST_OPERATOR_GETITEM_OR_SETITEM   - AST_OPERATOR_MIN] = */ &str___giosi,
	/* [AST_OPERATOR_GETRANGE_OR_SETRANGE - AST_OPERATOR_MIN] = */ &str___grosr,
	/* [AST_OPERATOR_GETATTR_OR_SETATTR   - AST_OPERATOR_MIN] = */ &str___gaosa
};

PRIVATE WUNUSED NONNULL((1)) bool DCALL
convert_operator_name(uint16_t *__restrict pname, size_t argc) {
	/* Convert an operator name based on the provided number of arguments. */
	if (*pname >= AST_OPERATOR_MIN &&
	    *pname <= AST_OPERATOR_MAX) {
		switch (*pname) {

		case AST_OPERATOR_POS_OR_ADD:
			if (argc == 1) {
				*pname = OPERATOR_POS;
				goto ok;
			}
			if (argc == 2) {
				*pname = OPERATOR_ADD;
				goto ok;
			}
			break;

		case AST_OPERATOR_NEG_OR_SUB:
			if (argc == 1) {
				*pname = OPERATOR_NEG;
				goto ok;
			}
			if (argc == 2) {
				*pname = OPERATOR_SUB;
				goto ok;
			}
			break;

		case AST_OPERATOR_GETITEM_OR_SETITEM:
			if (argc == 2) {
				*pname = OPERATOR_GETITEM;
				goto ok;
			}
			if (argc == 3) {
				*pname = OPERATOR_SETITEM;
				goto ok;
			}
			break;

		case AST_OPERATOR_GETRANGE_OR_SETRANGE:
			if (argc == 3) {
				*pname = OPERATOR_GETRANGE;
				goto ok;
			}
			if (argc == 4) {
				*pname = OPERATOR_SETRANGE;
				goto ok;
			}
			break;

		case AST_OPERATOR_GETATTR_OR_SETATTR:
			if (argc == 2) {
				*pname = OPERATOR_GETATTR;
				goto ok;
			}
			if (argc == 3) {
				*pname = OPERATOR_SETATTR;
				goto ok;
			}
			break;

		default: break;
		}
		return false;
	}
ok:
	return true;
}


INTERN WUNUSED NONNULL((3)) DREF struct ast *DCALL
ast_build_operator(uint16_t name, uint16_t flags,
                   struct ast *__restrict args) {
	ASSERT(!(flags & AST_OPERATOR_FVARARGS));
	if (args->a_type == AST_MULTIPLE &&
	    args->a_flag == AST_FMULTIPLE_TUPLE) {
		/* Simple case: the amount of arguments are know at compile-time. */
		size_t argc = args->a_multiple.m_astc;
		if (!convert_operator_name(&name, argc))
			goto do_generic;
		switch (argc) {

		case 1:
			return ast_operator1(name, flags, args->a_multiple.m_astv[0]);

		case 2:
			return ast_operator2(name, flags, args->a_multiple.m_astv[0],
			                     args->a_multiple.m_astv[1]);

		case 3:
			return ast_operator3(name, flags, args->a_multiple.m_astv[0],
			                     args->a_multiple.m_astv[1],
			                     args->a_multiple.m_astv[2]);

		case 4:
			return ast_operator4(name, flags, args->a_multiple.m_astv[0],
			                     args->a_multiple.m_astv[1],
			                     args->a_multiple.m_astv[2],
			                     args->a_multiple.m_astv[3]);

		default: break;
		}
	}
	if (args->a_type == AST_CONSTEXPR &&
	    DeeTuple_Check(args->a_constexpr)) {
		/* Another special case: The argument AST is a constant-expression tuple. */
		size_t argc = DeeTuple_SIZE(args->a_constexpr);
		if likely(argc < 4 && argc != 0) {
			DeeObject *tuple        = args->a_constexpr;
			DREF struct ast *result = NULL, *argv[4] = { NULL, NULL, NULL, NULL };
			if (!convert_operator_name(&name, argc))
				goto do_generic;
			if (argc >= 4 && (argv[3] = ast_constexpr(DeeTuple_GET(tuple, 3))) == NULL)
				goto end_argv;
			if (argc >= 3 && (argv[2] = ast_constexpr(DeeTuple_GET(tuple, 2))) == NULL)
				goto end_argv;
			if (argc >= 2 && (argv[1] = ast_constexpr(DeeTuple_GET(tuple, 1))) == NULL)
				goto end_argv;
			if (argc >= 1 && (argv[0] = ast_constexpr(DeeTuple_GET(tuple, 0))) == NULL)
				goto end_argv;
			switch (argc) {

			case 1:
				result = ast_operator1(name, flags, argv[0]);
				break;

			case 2:
				result = ast_operator2(name, flags, argv[0], argv[1]);
				break;

			case 3:
				result = ast_operator3(name, flags, argv[0], argv[1], argv[2]);
				break;

			default:
				result = ast_operator4(name, flags, argv[0], argv[1], argv[2], argv[3]);
				break;
			}
end_argv:
			ast_xdecref(argv[3]);
			ast_xdecref(argv[2]);
			ast_xdecref(argv[1]);
			ast_xdecref(argv[0]);
			return result;
		}
	}

do_generic:
	/* We can't determine the argument count at compile-time.
	 * Instead, generate a call to an external function. */
	if (name >= AST_OPERATOR_MIN &&
	    name <= AST_OPERATOR_MAX) {
		DREF struct ast *function_ast;
		struct symbol *function_symbol;
		DeeStringObject *function_name;
		function_name   = rt_operator_names[name - AST_OPERATOR_MIN];
		function_symbol = new_unnamed_symbol();
		if unlikely(!function_symbol)
			goto err;
		function_symbol->s_type            = SYMBOL_TYPE_EXTERN;
		function_symbol->s_extern.e_module = DeeModule_GetDeemon();
		Dee_Incref(function_symbol->s_extern.e_module);
		function_symbol->s_extern.e_symbol = DeeModule_GetSymbolString(function_symbol->s_extern.e_module,
		                                                               DeeString_STR(function_name),
		                                                               DeeString_Hash((DeeObject *)function_name));
		ASSERTF(function_symbol->s_extern.e_symbol,
		        "Missing runtime function `%s'",
		        DeeString_STR(function_name));
		function_ast = ast_sym(function_symbol);
		if unlikely(!function_ast)
			goto err;
		args = ast_operator2(OPERATOR_CALL, 0, function_ast, args);
		ast_decref(function_ast);
		return args;
err:
		return NULL;
	}
	/* Encode a regular, old varargs operator. */
	return ast_operator1(name, flags | AST_OPERATOR_FVARARGS, args);
}

INTERN WUNUSED NONNULL((3, 4)) DREF struct ast *DCALL
ast_build_bound_operator(uint16_t name, uint16_t flags,
                         struct ast *__restrict self,
                         struct ast *__restrict args) {
	ASSERT(!(flags & AST_OPERATOR_FVARARGS));
	if (args->a_type == AST_MULTIPLE &&
	    args->a_flag == AST_FMULTIPLE_TUPLE) {
		/* Simple case: the amount of arguments are know at compile-time. */
		size_t argc = args->a_multiple.m_astc;
		if (!convert_operator_name(&name, 1 + argc))
			goto do_generic;
		switch (argc) {

		case 0:
			return ast_operator1(name, flags, self);

		case 1:
			return ast_operator2(name, flags, self, args->a_multiple.m_astv[0]);

		case 2:
			return ast_operator3(name, flags, self, args->a_multiple.m_astv[0],
			                     args->a_multiple.m_astv[1]);

		case 3:
			return ast_operator4(name, flags, self, args->a_multiple.m_astv[0],
			                     args->a_multiple.m_astv[1],
			                     args->a_multiple.m_astv[2]);

		default: break;
		}
	}
	if (args->a_type == AST_CONSTEXPR &&
	    DeeTuple_Check(args->a_constexpr)) {
		/* Another special case: The argument AST is a constant-expression tuple. */
		size_t argc = DeeTuple_SIZE(args->a_constexpr);
		if likely(argc < 4) {
			DeeObject *tuple        = args->a_constexpr;
			DREF struct ast *result = NULL, *argv[3] = { NULL, NULL, NULL };
			if (!convert_operator_name(&name, 1 + argc))
				goto do_generic;
			if (argc >= 3 && (argv[2] = ast_constexpr(DeeTuple_GET(tuple, 2))) == NULL)
				goto end_argv;
			if (argc >= 2 && (argv[1] = ast_constexpr(DeeTuple_GET(tuple, 1))) == NULL)
				goto end_argv;
			if (argc >= 1 && (argv[0] = ast_constexpr(DeeTuple_GET(tuple, 0))) == NULL)
				goto end_argv;
			switch (argc) {
			case 0: result = ast_operator1(name, flags, self); break;
			case 1: result = ast_operator2(name, flags, self, argv[0]); break;
			case 2: result = ast_operator3(name, flags, self, argv[0], argv[1]); break;
			default: result = ast_operator4(name, flags, self, argv[0], argv[1], argv[2]); break;
			}
end_argv:
			ast_xdecref(argv[2]);
			ast_xdecref(argv[1]);
			ast_xdecref(argv[0]);
			return result;
		}
	}

do_generic:
	/* We can't determine the argument count at compile-time.
	 * Instead, generate a call to an external function. */
	if (name >= AST_OPERATOR_MIN &&
	    name <= AST_OPERATOR_MAX) {
		DREF struct ast **argv, *new_args;
		DREF struct ast *function_ast;
		struct symbol *function_symbol;
		DeeStringObject *function_name;
		function_name = rt_operator_names[name - AST_OPERATOR_MIN];
		/* Pack together the argument list. */
		if unlikely((args = ast_expand(args)) == NULL)
			goto err;
		argv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
		if unlikely(!argv)
			goto err_args;
		argv[0] = self;
		argv[1] = args;
		ast_incref(self);
		new_args = ast_multiple(AST_FMULTIPLE_TUPLE, 2, argv);
		if unlikely(!new_args) {
			ast_decref(self);
			Dee_Free(argv);
			goto err_args;
		}
		args            = new_args; /* This is now the argument tuple for the builtin function call. */
		function_symbol = new_unnamed_symbol();
		if unlikely(!function_symbol)
			goto err_args;
		function_symbol->s_type            = SYMBOL_TYPE_EXTERN;
		function_symbol->s_extern.e_module = DeeModule_GetDeemon();
		Dee_Incref(function_symbol->s_extern.e_module);
		function_symbol->s_extern.e_symbol = DeeModule_GetSymbolString(function_symbol->s_extern.e_module,
		                                                               DeeString_STR(function_name),
		                                                               DeeString_Hash((DeeObject *)function_name));
		ASSERTF(function_symbol->s_extern.e_symbol,
		        "Missing runtime function `%s'",
		        DeeString_STR(function_name));
		function_ast = ast_sym(function_symbol);
		if unlikely(!function_ast)
			goto err_args;
		new_args = ast_operator2(OPERATOR_CALL, 0, function_ast, args);
		ast_decref(function_ast);
		ast_decref(args);
		return new_args;
err_args:
		ast_decref(args);
err:
		return NULL;
	}
	/* Encode a regular, old varargs operator. */
	return ast_operator2(name, flags | AST_OPERATOR_FVARARGS, self, args);
}

INTDEF struct opinfo const basic_opinfo[OPERATOR_USERCOUNT];
INTDEF struct opinfo const file_opinfo[FILE_OPERATOR_COUNT];

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 3)) struct opinfo const *DCALL
find_opinfo(struct opinfo const *__restrict v, unsigned int c,
            char const *__restrict str, size_t len) {
	unsigned int i;
	for (i = 0; i < c; ++i) {
		char const *name = v[i].oi_sname;
		if (bcmpc(name, str, len, sizeof(char)) == 0 && !name[len])
			return &v[i];
	}
	return NULL;
}


/* Parse and return an operator name.
 * @param: features: Set of `P_OPERATOR_F*'
 * @return: * : One of `OPERATOR_*' or `AST_OPERATOR_*'
 * @return: -1: An error occurred. */
INTERN WUNUSED int32_t DCALL
ast_parse_operator_name(uint16_t features) {
	int32_t result;
	uint32_t old_flags;
	switch (tok) {

	case TOK_CHAR: {
		tint_t intval;
		if (!HAS(EXT_CHARACTER_LITERALS))
			goto parse_string;
		ATTR_FALLTHROUGH
	case TOK_INT:
		/* Special case: Invoke an operator using its internal index. */
		if (TPP_Atoi(&intval) == TPP_ATOI_ERR)
			goto err;
		result = (int32_t)(uint16_t)intval;
	}	goto done_y1;

	case '+':
		result = AST_OPERATOR_POS_OR_ADD;
		goto done_y1;

	case '-':
		result = AST_OPERATOR_NEG_OR_SUB;
		goto done_y1;

	case '*':
		result = OPERATOR_MUL;
		goto done_y1;

	case '/':
		result = OPERATOR_DIV;
		goto done_y1;

	case '%':
		result = OPERATOR_MOD;
		goto done_y1;

	case '&':
		result = OPERATOR_AND;
		goto done_y1;

	case '|':
		result = OPERATOR_OR;
		goto done_y1;

	case '^':
		result = OPERATOR_XOR;
		goto done_y1;

	case '~':
		result = OPERATOR_INV;
		goto done_y1;

	case TOK_SHL:
		result = OPERATOR_SHL;
		goto done_y1;

	case TOK_SHR:
		result = OPERATOR_SHR;
		goto done_y1;

	case TOK_POW:
		result = OPERATOR_POW;
		goto done_y1;

	case TOK_ADD_EQUAL:
		result = OPERATOR_INPLACE_ADD;
		goto done_y1;

	case TOK_SUB_EQUAL:
		result = OPERATOR_INPLACE_SUB;
		goto done_y1;

	case TOK_MUL_EQUAL:
		result = OPERATOR_INPLACE_MUL;
		goto done_y1;

	case TOK_DIV_EQUAL:
		result = OPERATOR_INPLACE_DIV;
		goto done_y1;

	case TOK_MOD_EQUAL:
		result = OPERATOR_INPLACE_MOD;
		goto done_y1;

	case TOK_SHL_EQUAL:
		result = OPERATOR_INPLACE_SHL;
		goto done_y1;

	case TOK_SHR_EQUAL:
		result = OPERATOR_INPLACE_SHR;
		goto done_y1;

	case TOK_AND_EQUAL:
		result = OPERATOR_INPLACE_AND;
		goto done_y1;

	case TOK_OR_EQUAL:
		result = OPERATOR_INPLACE_OR;
		goto done_y1;

	case TOK_XOR_EQUAL:
		result = OPERATOR_INPLACE_XOR;
		goto done_y1;

	case TOK_POW_EQUAL:
		result = OPERATOR_INPLACE_POW;
		goto done_y1;

	case TOK_INC:
		result = OPERATOR_INC;
		goto done_y1;

	case TOK_DEC:
		result = OPERATOR_DEC;
		goto done_y1;

	case KWD_repr:
		result = OPERATOR_REPR;
		goto done_y1;

	case TOK_EQUAL:
		result = OPERATOR_EQ;
		goto done_y1;

	case TOK_NOT_EQUAL:
		result = OPERATOR_NE;
		goto done_y1;

	case TOK_LOWER:
do_operator_lo:
		result = OPERATOR_LO;
		goto done_y1;

	case TOK_LOWER_EQUAL:
		result = OPERATOR_LE;
		goto done_y1;

	case TOK_GREATER:
do_operator_gr:
		result = OPERATOR_GR;
		goto done_y1;

	case TOK_GREATER_EQUAL:
		result = OPERATOR_GE;
		goto done_y1;

	case '#':
		result = OPERATOR_SIZE;
		goto done_y1;

	case KWD_copy:
		result = OPERATOR_COPY;
		goto done_y1;

	case KWD_deepcopy:
		result = OPERATOR_DEEPCOPY;
		goto done_y1;

	case KWD_super:
		if (!(features & P_OPERATOR_FCLASS))
			goto default_case;
		result = CLASS_OPERATOR_SUPERARGS;
		goto done_y1;

	case '=':
		if (WARN(W_EXPECTED_COLON_EQUALS_AS_OPERATOR_NAME))
			goto err;
		ATTR_FALLTHROUGH
	case TOK_COLON_EQUAL:
		result = OPERATOR_ASSIGN;
		if unlikely(yield() < 0)
			goto err;
		if (TPP_ISKEYWORD(tok) && token.t_kwd->k_size == 4 &&
		    UNALIGNED_GET32((uint32_t *)token.t_kwd->k_name) == ENCODE_INT32('m', 'o', 'v', 'e')) {
			/* `= move' move-assign operator. */
			result = OPERATOR_MOVEASSIGN;
			goto done_y1;
		}
		goto done;

	case '(':
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		if (tok == ')') {
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			result = OPERATOR_CALL;
			goto done_y1;
		}
		/* Parenthesis around operator name. */
		result = ast_parse_operator_name(features);
		if unlikely(result < 0)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if unlikely(tok != ')') {
			if (WARN(W_EXPECTED_RPAREN_AFTER_LPAREN))
				goto err;
			goto done;
		}
		goto done_y1;

	case TOK_STRING:
parse_string:
		if (advance_wraplf(advance_wraplf(token.t_begin)) != token.t_end &&
		    WARN(W_EXPECTED_EMPTY_STRING_FOR_OPERATOR_NAME))
			goto err;
		ATTR_FALLTHROUGH
	case KWD_str:
		result = OPERATOR_STR;
		goto done_y1;

	case '[':
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		result = AST_OPERATOR_GETITEM_OR_SETITEM;
		if (tok == ':') {
			result = AST_OPERATOR_GETRANGE_OR_SETRANGE;
			if unlikely(yield() < 0)
				goto err_flags;
		}
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(']', W_EXPECTED_RBRACKET_AFTER_LBRACKET))
			goto err;
		if (tok == '=') {
			if unlikely(yield() < 0)
				goto err;
			result = (result == AST_OPERATOR_GETITEM_OR_SETITEM
			          ? OPERATOR_SETITEM
			          : OPERATOR_SETRANGE);
		}
		goto done;

	case KWD_del:
		if unlikely(yield() < 0)
			goto err;
		if (tok == '[') {
			old_flags = TPPLexer_Current->l_flags;
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err_flags;
			result = OPERATOR_DELITEM;
			if (tok == ':') {
				result = OPERATOR_DELRANGE;
				if unlikely(yield() < 0)
					goto err_flags;
			}
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if unlikely(tok != ']') {
				if (WARN(W_EXPECTED_RBRACKET_AFTER_LBRACKET))
					goto err;
				goto done;
			}
			goto done_y1;
		}
		result = OPERATOR_DELATTR;
		if unlikely(tok != '.') {
			if (WARN(W_EXPECTED_LBRACKET_OR_DOT_AFTER_DEL_FOR_OPERATOR_NAME))
				goto err;
			goto done;
		}
		goto done_y1;

	case '.':
		result = AST_OPERATOR_GETATTR_OR_SETATTR;
		if unlikely(yield() < 0)
			goto err;
		if (tok == '=') {
			result = OPERATOR_SETATTR;
			goto done_y1;
		}
		goto done;

	default: {
		char const *name_begin;
		size_t name_size;
		uint32_t name;
	default_case:
		if (!TPP_ISKEYWORD(tok))
			goto unknown;
		name_begin = token.t_kwd->k_name;
		name_size  = token.t_kwd->k_size;
		/* Other operator names that technically should have their own
		 * keyword, but since this is the only place that keyword would
		 * ever get used, the overhead of manually checking for them is
		 * smaller, causing me to opt for this route instead. */
		switch (name_size) {
		case 3:
		case 4:
			name = UNALIGNED_GET32((uint32_t *)name_begin);
#ifndef __OPTIMIZE_SIZE__
			if (name == ENCODE_INT32('h', 'a', 's', 'h')) {
				result = OPERATOR_HASH;
				goto done_y1;
			}
#endif /* !__OPTIMIZE_SIZE__ */
			if (name == ENCODE_INT32('n', 'e', 'x', 't')) {
				result = OPERATOR_ITERNEXT;
				goto done_y1;
			}
			if (name == ENCODE_INT32('i', 't', 'e', 'r')) {
				result = OPERATOR_ITERSELF;
				goto done_y1;
			}
			if (name == ENCODE_INT32('f', 'o', 'r', 0) && (features & P_OPERATOR_FCLASS)) {
				result = AST_OPERATOR_FOR;
				goto done_y1;
			}
			if (name == ENCODE_INT32('m', 'o', 'v', 'e')) {
				if unlikely(yield() < 0)
					goto err;
				result = OPERATOR_MOVEASSIGN;
				if unlikely(tok == '=') {
					if (WARN(W_EXPECTED_COLON_EQUALS_AS_OPERATOR_NAME))
						goto err;
				} else if unlikely(tok != TOK_COLON_EQUAL) {
					if (WARN(W_EXPECTED_EQUAL_AFTER_MOVE_IN_OPERATOR_NAME))
						goto err;
					goto done;
				}
				goto done_y1;
			}
			break;
#ifndef __OPTIMIZE_SIZE__
		case 5:
			name = UNALIGNED_GET32((uint32_t *)name_begin);
			if (name == ENCODE_INT32('e', 'n', 't', 'e') && *((uint8_t *)(name_begin + 4)) == 'r') {
				result = OPERATOR_ENTER;
				goto done_y1;
			}
			if (name == ENCODE_INT32('l', 'e', 'a', 'v') && *((uint8_t *)(name_begin + 4)) == 'e') {
				result = OPERATOR_LEAVE;
				goto done_y1;
			}
			break;
		case 8:
			if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('c', 'o', 'n', 't') &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE_INT32('a', 'i', 'n', 's')) {
				result = OPERATOR_CONTAINS;
				goto done_y1;
			}
			break;
		case 10:
			if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('d', 'e', 's', 't') &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE_INT32('r', 'u', 'c', 't') &&
			    UNALIGNED_GET16((uint16_t *)(name_begin + 8)) == ENCODE_INT16('o', 'r')) {
				result = OPERATOR_DESTRUCTOR;
				goto done_y1;
			}
			break;
		case 11:
			if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('c', 'o', 'n', 's') &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE_INT32('t', 'r', 'u', 'c') &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 8)) == ENCODE_INT32('t', 'o', 'r', 0)) {
				result = OPERATOR_CONSTRUCTOR;
				goto done_y1;
			}
			break;
#endif /* !__OPTIMIZE_SIZE__ */
		default: break;
		}
		while (name_size && *name_begin == '_')
			++name_begin, --name_size;
		while (name_size && name_begin[name_size - 1] == '_')
			--name_size;

#if 0 /* Already handled by generic opinfo searches. */
		/* Some special operators that didn't merit their own keyword. */
		if (name_size == 4 &&
		    UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('b', 'o', 'o', 'l')) {
			result = OPERATOR_BOOL;
			goto done_y1;
		}
		if (name_size == 3 &&
		    UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('i', 'n', 't', 0)) {
			result = OPERATOR_INT;
			goto done_y1;
		}
		if (name_size == 5 &&
		    UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('f', 'l', 'o', 'a') &&
		    *(uint8_t *)(name_begin + 4) == 't') {
			result = OPERATOR_FLOAT;
			goto done_y1;
		}
#endif
		/* Query an explicit operator by its name.
		 * NOTE: This is also where a lot of backwards-compatibility lies, as
		 *       the old deemon used to only accept e.g.: `operator __contains__'. */
		{
			struct opinfo const *info;
			info = find_opinfo(basic_opinfo, COMPILER_LENOF(basic_opinfo), name_begin, name_size);
			if (info) {
				result = (uint16_t)(info - basic_opinfo);
				goto done_y1;
			}
			if (!(features & P_OPERATOR_FNOFILE)) {
				info = find_opinfo(file_opinfo, COMPILER_LENOF(file_opinfo), name_begin, name_size);
				if (info) {
					result = OPERATOR_EXTENDED(0) + (uint16_t)(info - file_opinfo);
					goto done_y1;
				}
			}
			/* Even more backwards compatibility. */
			if (name_size == 2) {
				if (UNALIGNED_GET16((uint16_t *)(name_begin + 0)) == ENCODE_INT16('l', 't'))
					goto do_operator_lo;
				if (UNALIGNED_GET16((uint16_t *)(name_begin + 0)) == ENCODE_INT16('g', 't'))
					goto do_operator_gr;
			}
			if (name_size == 6 &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('r', 'e', 'a', 'd') &&
			    UNALIGNED_GET16((uint16_t *)(name_begin + 4)) == ENCODE_INT16('n', 'p')) {
				result = FILE_OPERATOR_READ;
				goto done_y1;
			}
			if (name_size == 7 &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('w', 'r', 'i', 't') &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE_INT32('e', 'n', 'p', 0)) {
				result = FILE_OPERATOR_WRITE;
				goto done_y1;
			}
			if (name_size == 9 &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('s', 'u', 'p', 'e') &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE_INT32('r', 'a', 'r', 'g') &&
			    *(uint8_t *)(name_begin + 8) == 's' && (features & P_OPERATOR_FCLASS)) {
				result = CLASS_OPERATOR_SUPERARGS;
				goto done_y1;
			}
			if (name_size == 6 &&
			    UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('d', 'o', 'u', 'b') &&
			    UNALIGNED_GET16((uint16_t *)(name_begin + 4)) == ENCODE_INT16('l', 'e')) {
				result = OPERATOR_FLOAT;
				goto done_y1;
			}
			if (name_size == 5 &&
			    ((UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('i', 'n', 't', '3') &&
			      *(uint8_t *)(name_begin + 4) == '2') ||
			     (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('i', 'n', 't', '6') &&
			      *(uint8_t *)(name_begin + 4) == '4'))) {
				result = OPERATOR_INT;
				goto done_y1;
			}
		}
unknown:
		if (WARN(W_UNKNOWN_OPERATOR_NAME))
			goto err;
		result = (int32_t)0; /* Default to whatever operator #0 is. */
		goto done;
	}	break;

	}

done_y1:
	if unlikely(yield() < 0)
		goto err;
done:
	return result;
err_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_OPERATOR_C */
