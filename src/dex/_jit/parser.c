/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_JIT_PARSER_C
#define GUARD_DEX_JIT_PARSER_C 1
#define DEE_SOURCE 1

#include "libjit.h"
/**/

#include <deemon/class.h>
#include <deemon/compiler/lexer.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/stringutils.h>
#include <deemon/super.h>
#include <deemon/tuple.h>
#include <deemon/system-features.h> /* memcpy(), ... */

#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>

DECL_BEGIN

/* Special symbol names to generate function calls to
 * when the argument count cannot be determined. */
INTERN_CONST char const rt_operator_names[1 + (AST_OPERATOR_MAX - AST_OPERATOR_MIN)][8] = {
	/* [AST_OPERATOR_POS_OR_ADD           - AST_OPERATOR_MIN] = */ "__pooad",
	/* [AST_OPERATOR_NEG_OR_SUB           - AST_OPERATOR_MIN] = */ "__neosb",
	/* [AST_OPERATOR_GETITEM_OR_SETITEM   - AST_OPERATOR_MIN] = */ "__giosi",
	/* [AST_OPERATOR_GETRANGE_OR_SETRANGE - AST_OPERATOR_MIN] = */ "__grosr",
	/* [AST_OPERATOR_GETATTR_OR_SETATTR   - AST_OPERATOR_MIN] = */ "__gaosa"
};

/* Check if the current token may refer to the start of an expression.
 * The currently selected token is not altered/is restored before this function returns.
 * NOTE: This function may also be used with `JITSmallLexer' */
INTERN bool FCALL
JITLexer_MaybeExpressionBegin(JITLexer *__restrict self) {
	switch (self->jl_tok) {

	case '+':
	case '-':
	case '~':
	case '(':
	case '#':
	case '[': /* For lists. */
	case '{': /* Brace initializers. */
	case TOK_INC:
	case TOK_DEC:
	case TOK_INT:
	case JIT_STRING:
	case JIT_RAWSTRING:
	case TOK_FLOAT:
	case TOK_DOTS:
		goto yes;

	case '!': {
		bool result;
		unsigned char *orig_tok_start;
		orig_tok_start = self->jl_tokstart;
		/* Check if this ! is eventually followed by `is' or `in'
		 * If this is the case, then this can't be the start of an
		 * expression! */
		do {
			JITLexer_Yield(self);
		} while (self->jl_tok == '!');
		if (self->jl_tok != JIT_KEYWORD) {
			result = true;
		} else if (self->jl_tokstart + 2 == self->jl_tokend &&
		           self->jl_tokstart[0] == 'i' &&
		           (self->jl_tokstart[1] == 's' || self->jl_tokstart[1] == 'n')) {
			/* `is' or `in' after `!' cannot appear at the start of an expression! */
			result = false;
		} else {
			result = true;
		}
		JITLexer_YieldAt(self, orig_tok_start);
		return result;
	}	break;

	case JIT_KEYWORD: {
		/* Black-list a couple of keywords that cannot appear in expressions */
		char *tokptr;
		size_t toklen;
		tokptr = JITLexer_TokPtr(self);
		toklen = JITLexer_TokLen(self);
		switch (toklen) {

			/* Keywords that can only appear inside of expressions */
		case 2:
			if (tokptr[0] == 'a') {
				if (tokptr[1] == 's')
					goto no; /* `as' */
			} else if (tokptr[0] == 'i') {
				if (tokptr[1] == 'n' || tokptr[1] == 's')
					goto no; /* `in', `is' */
			}
			break;

		case 4: {
			uint32_t name;
			name = UNALIGNED_GET32((uint32_t *)tokptr);
			if (name == ENCODE_INT32('e', 'l', 's', 'e'))
				goto no;
			if (name == ENCODE_INT32('f', 'r', 'o', 'm'))
				goto no;
#if 0 /* Unsupported */
			if (name == ENCODE_INT32('g', 'o', 't', 'o'))
				goto no;
#endif
			if (name == ENCODE_INT32('c', 'a', 's', 'e'))
				goto no;
		}	break;

		case 5: {
			uint32_t name;
			name = UNALIGNED_GET32((uint32_t *)tokptr);
			if (name == ENCODE_INT32('c', 'a', 't', 'c') && tokptr[4] == 'h')
				goto no;
			if (name == ENCODE_INT32('y', 'i', 'e', 'l') && tokptr[4] == 'd')
				goto no;
			if (name == ENCODE_INT32('t', 'h', 'r', 'o') && tokptr[4] == 'w')
				goto no;
			if (name == ENCODE_INT32('p', 'r', 'i', 'n') && tokptr[4] == 't')
				goto no;
			if (name == ENCODE_INT32('b', 'r', 'e', 'a') && tokptr[4] == 'k')
				goto no;
			if (name == ENCODE_INT32('_', '_', 'a', 's') && tokptr[4] == 'm')
				goto no;
		}	break;

		case 6: {
			uint32_t name;
			uint16_t name2;
			name  = UNALIGNED_GET32((uint32_t *)tokptr);
			name2 = UNALIGNED_GET16((uint16_t *)(tokptr + 4));
			if (name == ENCODE_INT32('r', 'e', 't', 'u') && name2 == ENCODE_INT16('r', 'n'))
				goto no;
			if (name == ENCODE_INT32('s', 'w', 'i', 't') && name2 == ENCODE_INT16('c', 'h'))
				goto no;
		}	break;

		case 7: {
			uint32_t name;
			uint16_t name2;
			name  = UNALIGNED_GET32((uint32_t *)tokptr);
			name2 = UNALIGNED_GET16((uint16_t *)(tokptr + 4));
			if (name == ENCODE_INT32('f', 'i', 'n', 'a') && name2 == ENCODE_INT16('l', 'l') && tokptr[6] == 'y')
				goto no;
			if (name == ENCODE_INT32('d', 'e', 'f', 'a') && name2 == ENCODE_INT16('u', 'l') && tokptr[6] == 't')
				goto no;
			if (name == ENCODE_INT32('_', '_', 'a', 's') && name2 == ENCODE_INT16('m', '_') && tokptr[6] == '_')
				goto no;
		}	break;

		case 8: {
			uint32_t namea, nameb;
			namea = UNALIGNED_GET32((uint32_t *)tokptr);
			nameb = UNALIGNED_GET32((uint32_t *)(tokptr + 4));
			if (namea == ENCODE_INT32('c', 'o', 'n', 't') &&
			    nameb == ENCODE_INT32('i', 'n', 'u', 'e'))
				goto no;
		}	break;

		default:
			break;
		}
	}	goto yes;

	default:
		goto no;
	}
no:
	return false;
yes:
	return true;
}

PRIVATE DEFINE_STRING(str_operators, "operators");


INTERN WUNUSED DREF DeeObject *FCALL
JIT_GetOperatorFunction(uint16_t opname) {
	DREF DeeObject *result;
	DREF DeeModuleObject *operators_module;
	char const *symbol_name = NULL;
	dhash_t hash            = 0;
	if (opname >= AST_OPERATOR_MIN &&
	    opname <= AST_OPERATOR_MAX) {
		/* Special, ambiguous operator. */
		symbol_name = rt_operator_names[opname - AST_OPERATOR_MIN];
		hash        = Dee_HashPtr(symbol_name, 8);
	} else {
		struct opinfo const *info;
		/* Default case: determine the operator symbol using generic-operator info. */
		info = Dee_OperatorInfo(NULL, opname);
		if (info) {
			symbol_name = info->oi_sname;
			hash        = Dee_HashStr(symbol_name);
		}
	}
	operators_module = (DREF DeeModuleObject *)DeeModule_OpenGlobal((DeeObject *)&str_operators,
	                                                                NULL,
	                                                                true);
	if unlikely(!operators_module)
		goto err;
	if (symbol_name) {
		result = DeeObject_GetAttrStringHash((DeeObject *)operators_module,
		                                     symbol_name,
		                                     hash);
	} else {
		/* Fallback: Invoke `operator(id)' to generate the default callback. */
		result = DeeObject_GetAttrStringHash((DeeObject *)operators_module, "operator",
		                                     Dee_HashPtr("operator", COMPILER_STRLEN("operator")));
		if likely(result) {
			DREF DeeObject *callback_result;
			callback_result = DeeObject_Callf(result, "I16u", opname);
			Dee_Decref(result);
			result = callback_result;
		}
	}
	Dee_Decref(operators_module);
	return result;
err:
	return NULL;
}




INTERN int32_t FCALL
JITLexer_ParseOperatorName(JITLexer *__restrict self,
                           uint16_t features) {
	int32_t result;
	switch (self->jl_tok) {

	case TOK_INT: {
		uint16_t val;
		if (Dee_Atou16((char const *)self->jl_tokstart,
		               JITLexer_TokLen(self),
		               DEEINT_STRING(0, DEEINT_STRING_FNORMAL),
		               &val))
			goto err_trace;
		result = (int32_t)(uint16_t)val;
		goto done_y1;
	}

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

	case '=':
	case TOK_COLLON_EQUAL:
		result = OPERATOR_ASSIGN;
		JITLexer_Yield(self);
		if (JITLexer_ISKWD(self, "move")) {
			/* `= move' move-assign operator. */
			result = OPERATOR_MOVEASSIGN;
			goto done_y1;
		}
		goto done;

	case '(':
		JITLexer_Yield(self);
		if (self->jl_tok == ')') {
			result = OPERATOR_CALL;
			goto done_y1;
		}
		/* Parenthesis around operator name. */
		result = JITLexer_ParseOperatorName(self, features);
		if unlikely(result < 0)
			goto err;
		if unlikely(self->jl_tok != ')') {
			syn_paren_expected_rparen_after_lparen(self);
			goto err_trace;
		}
		goto done_y1;

	case JIT_RAWSTRING:
		if (self->jl_tokend != self->jl_tokstart + 3)
			goto err_empty_string;
		result = OPERATOR_STR;
		break;

	case JIT_STRING:
		if (self->jl_tokend != self->jl_tokstart + 2) {
err_empty_string:
			syn_operator_expected_empty_string(self);
			goto err_trace;
		}
		result = OPERATOR_STR;
		break;

	case '[':
		JITLexer_Yield(self);
		result = AST_OPERATOR_GETITEM_OR_SETITEM;
		if (self->jl_tok == ':') {
			result = AST_OPERATOR_GETRANGE_OR_SETRANGE;
			JITLexer_Yield(self);
		}
		if likely(self->jl_tok == ']') {
			JITLexer_Yield(self);
		} else {
err_rbrck_after_lbrck:
			syn_bracket_expected_rbracket_after_lbracket(self);
			goto err_trace;
		}
		if (self->jl_tok == '=') {
			JITLexer_Yield(self);
			result = (result == AST_OPERATOR_GETITEM_OR_SETITEM
			          ? OPERATOR_SETITEM
			          : OPERATOR_SETRANGE);
		}
		goto done;

	case '.':
		result = AST_OPERATOR_GETATTR_OR_SETATTR;
		JITLexer_Yield(self);
		if (self->jl_tok == '=') {
			result = OPERATOR_SETATTR;
			goto done_y1;
		}
		goto done;

		{
			char const *name_begin;
			size_t name_size;
			uint32_t name;
		default:
/*default_case:*/
			if (self->jl_tok != JIT_KEYWORD)
				goto unknown;
			name_begin = (char const *)self->jl_tokstart;
			name_size  = (size_t)((char const *)self->jl_tokend - name_begin);
			/* Other operator names that technically should have their own
			 * keyword, but since this is the only place that keyword would
			 * ever get used, the overhead of manually checking for them is
			 * smaller, causing me to opt for this route instead. */
			switch (name_size) {

			case 3:
				if (name_begin[0] == 's' &&
				    name_begin[1] == 't' &&
				    name_begin[2] == 'r') {
					result = OPERATOR_STR;
					goto done_y1;
				}
				if (name_begin[0] == 'd' &&
				    name_begin[1] == 'e' &&
				    name_begin[2] == 'l') {
					JITLexer_Yield(self);
					if (self->jl_tok == '[') {
						JITLexer_Yield(self);
						result = OPERATOR_DELITEM;
						if (self->jl_tok == ':') {
							result = OPERATOR_DELRANGE;
							JITLexer_Yield(self);
						}
						if unlikely(self->jl_tok != ']')
							goto err_rbrck_after_lbrck;
						goto done_y1;
					}
					result = OPERATOR_DELATTR;
					if unlikely(self->jl_tok != '.') {
						syn_operator_expected_lbracket_or_dot_after_del(self);
						goto err_trace;
					}
					goto done_y1;
				}
				if (name_begin[0] == 'f' &&
				    name_begin[1] == 'o' &&
				    name_begin[2] == 'r' &&
				    (features & P_OPERATOR_FCLASS)) {
					result = AST_OPERATOR_FOR;
					goto done_y1;
				}
				break;

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
				if (name == ENCODE_INT32('r', 'e', 'p', 'r')) {
					result = OPERATOR_REPR;
					goto done_y1;
				}
				if (name == ENCODE_INT32('c', 'o', 'p', 'y')) {
					result = OPERATOR_COPY;
					goto done_y1;
				}
				if (name == ENCODE_INT32('m', 'o', 'v', 'e')) {
					JITLexer_Yield(self);
					result = OPERATOR_MOVEASSIGN;
					if unlikely(self->jl_tok != '=' &&
					            self->jl_tok != TOK_COLLON_EQUAL) {
						DeeError_Throwf(&DeeError_SyntaxError,
						                "Expected `:=' or `=' after `move' in operator name, but got `%$s'",
						                JITLexer_TokLen(self),
						                JITLexer_TokPtr(self));
						goto err_trace;
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
				if (name == ENCODE_INT32('s', 'u', 'p', 'e') && *((uint8_t *)(name_begin + 4)) == 'r' && (features & P_OPERATOR_FCLASS)) {
					result = CLASS_OPERATOR_SUPERARGS;
					goto done_y1;
				}
				break;

			case 8:
				if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('c', 'o', 'n', 't') &&
				    UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE_INT32('a', 'i', 'n', 's')) {
					result = OPERATOR_CONTAINS;
					goto done_y1;
				}
				if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE_INT32('d', 'e', 'e', 'p') &&
				    UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE_INT32('c', 'o', 'p', 'y')) {
					result = OPERATOR_DEEPCOPY;
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
				uint16_t opid;
				DeeTypeObject *typetype;
				typetype = features & P_OPERATOR_FNOFILE ? NULL : &DeeFileType_Type;
				opid     = Dee_OperatorFromNameLen(typetype, name_begin, name_size);
				if (opid != (uint16_t)-1) {
					result = opid;
					goto done_y1;
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
			syn_operator_unknown_name(self);
			goto err_trace;
		}
		break;
	}
done_y1:
	JITLexer_Yield(self);
done:
	return result;
err_trace:
	JITLexer_ErrorTrace(self, self->jl_tokstart);
err:
	return -1;
}


/* @return:  1: OK (the parsed object most certainly was a module)
 * @return:  0: OK
 * @return: -1: Error */
PRIVATE int DCALL
print_module_name(JITLexer *__restrict self,
                  struct unicode_printer *printer) {
	int result = 0;
	for (;;) {
		if (self->jl_tok == '.' || self->jl_tok == TOK_DOTS) {
			if (printer &&
			    unicode_printer_printascii(printer, "...", self->jl_tok == '.' ? 1 : 3) < 0)
				goto err_trace;
			result = 1;
			JITLexer_Yield(self);
			if (self->jl_tok != JIT_KEYWORD &&
			    self->jl_tok != JIT_STRING &&
			    self->jl_tok != JIT_RAWSTRING &&
			    self->jl_tok != '.' &&
			    self->jl_tok != TOK_DOTS)
				break; /* Special case: `.' is a valid name for the current module. */
		} else if (self->jl_tok == JIT_KEYWORD) {
			if (printer &&
			    unicode_printer_print(printer,
			                          JITLexer_TokPtr(self),
			                          JITLexer_TokLen(self)) < 0)
				goto err_trace;
			JITLexer_Yield(self);
			if (self->jl_tok != '.' && self->jl_tok != TOK_DOTS)
				break;
		} else if (self->jl_tok == JIT_STRING ||
		           self->jl_tok == JIT_RAWSTRING) {
			if (printer) {
				if (self->jl_tok == JIT_RAWSTRING) {
					if (unicode_printer_print(printer,
					                          JITLexer_TokPtr(self) + 2,
					                          JITLexer_TokLen(self) - 3) < 0)
						goto err_trace;
				} else {
					if (DeeString_DecodeBackslashEscaped(printer,
					                                     JITLexer_TokPtr(self) + 1,
					                                     JITLexer_TokLen(self) - 2,
					                                     STRING_ERROR_FSTRICT) < 0)
						goto err_trace;
				}
			}
			JITLexer_Yield(self);
			if (self->jl_tok != '.' &&
			    self->jl_tok != TOK_DOTS &&
			    self->jl_tok != JIT_STRING &&
			    self->jl_tok != JIT_RAWSTRING)
				break;
		} else {
			syn_module_expected_dot_keyword_or_string(self);
			goto err_trace;
		}
	}
	return result;
err_trace:
	JITLexer_ErrorTrace(self, self->jl_tokstart);
/*err:*/
	return -1;
}


/* Parse a module name, either writing it to `*printer' (if non-NULL),
 * or storing the name's start and end pointers in `*pname_start' and
 * `*pname_end'
 * @return:  1: Successfully parsed the module name and stored it in `*printer'
 *              In this case, this function will have also initialized `*printer'
 * @return:  0: Successfully parsed the module name and stored it in `*pname_start' / `*pname_end'
 * @return: -1: An error occurred. */
INTERN int FCALL
JITLexer_ParseModuleName(JITLexer *__restrict self,
                         struct unicode_printer *printer,
                         /*utf-8*/ unsigned char **pname_start,
                         /*utf-8*/ unsigned char **pname_end) {
	int error;
	/* Optimization for simple/inline module names, such that we
	 * don't have to actually copy the module name at this point! */
	if (self->jl_tok == '.' || self->jl_tok == JIT_KEYWORD) {
		unsigned char *start, *end;
		start = self->jl_tokstart;
		end   = self->jl_tokend;
		while (end < self->jl_end) {
			unsigned char ch = *end++;
			if (ch == '.')
				continue;
			if (ch > 0x7f) {
				uint32_t ch32;
				unsigned char *temp;
				--end;
				temp = end;
				ch32 = utf8_readchar((char const **)&end,
				                     (char const *)self->jl_end);
				if (!DeeUni_IsSymCont(ch32)) {
					end = temp;
					break;
				}
			} else {
				if (!DeeUni_IsSymCont(ch)) {
					--end;
					break;
				}
			}
		}
		if (pname_start)
			*pname_start = start;
		if (pname_end)
			*pname_end = end;
		/* Check if the keyword following the simple module
		 * name is something that would belong to our name. */
		JITLexer_YieldAt(self, end);
		if (self->jl_tok == '.' ||
		    self->jl_tok == JIT_KEYWORD ||
		    self->jl_tok == JIT_STRING ||
		    self->jl_tok == JIT_RAWSTRING) {
			JITLexer_YieldAt(self, start);
			goto use_printer;
		}
		return 0;
	}
use_printer:
	if (printer)
		unicode_printer_init(printer);
	error = print_module_name(self,
	                          printer);
	if (error == 0)
		error = 1;
	return error;
}




INTERN WUNUSED DREF /*Module*/ DeeObject *FCALL
JITLexer_EvalModule(JITLexer *__restrict self) {
	int error;
	DREF /*Module*/ DeeObject *result;
	struct unicode_printer printer;
	unsigned char *name_start, *name_end;
	error = JITLexer_ParseModuleName(self,
	                                 &printer,
	                                 &name_start,
	                                 &name_end);
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		/* The printer was used. */
		DREF DeeObject *str;
		str = unicode_printer_pack(&printer);
		if unlikely(!str)
			goto err_trace;
		if (DeeString_STR(str)[0] != '.') {
			result = DeeModule_OpenGlobal(str, NULL, true);
		} else {
			DeeModuleObject *base = self->jl_context->jc_impbase;
			if unlikely(!base) {
				DeeError_Throwf(&DeeError_CompilerError,
				                "Cannot import relative module %r",
				                str);
				goto err_trace;
			}
			result = DeeModule_ImportRel((DeeObject *)base, str, NULL, true);
		}
		Dee_Decref(str);
	} else if (name_start[0] != '.') {
		result = DeeModule_OpenGlobalString((char const *)name_start,
		                                    (size_t)(name_end - name_start),
		                                    NULL,
		                                    true);
	} else {
		DeeModuleObject *base = self->jl_context->jc_impbase;
		if unlikely(!base) {
			DeeError_Throwf(&DeeError_CompilerError,
			                "Cannot import relative module %$q",
			                (size_t)(name_end - name_start),
			                name_start);
			goto err_trace;
		}
		result = DeeModule_ImportRelString((DeeObject *)base,
		                                   (char const *)name_start,
		                                   (size_t)(name_end - name_start),
		                                   NULL,
		                                   true);
	}
	if unlikely(!result)
		goto err_trace;
	if unlikely(DeeModule_RunInit(result) < 0)
		goto err_rt_r;
	return result;
err_rt_r:
	Dee_Decref(result);
err_trace:
	JITLexer_ErrorTrace(self, self->jl_tokstart);
err:
	return NULL;
}


/* Parse lookup mode modifiers:
 * >> local x = 42;
 *    ^     ^
 */
INTERN int DCALL
JITLexer_ParseLookupMode(JITLexer *__restrict self,
                         unsigned int *__restrict pmode) {
next_modifier:
	if (self->jl_tok == JIT_KEYWORD) {
		if (JITLexer_ISTOK(self, "local")) {
			*pmode &= ~LOOKUP_SYM_VMASK;
			*pmode |= LOOKUP_SYM_VLOCAL;
continue_modifier:
			JITLexer_Yield(self);
			goto next_modifier;
		}
		if (JITLexer_ISTOK(self, "global")) {
			*pmode &= ~LOOKUP_SYM_VMASK;
			*pmode |= LOOKUP_SYM_VGLOBAL;
			goto continue_modifier;
		}
		if (JITLexer_ISTOK(self, "static")) {
			*pmode &= ~LOOKUP_SYM_STACK;
			*pmode |= LOOKUP_SYM_STATIC;
			goto continue_modifier;
		}
		if (JITLexer_ISTOK(self, "__stack")) {
			*pmode &= ~LOOKUP_SYM_STATIC;
			*pmode |= LOOKUP_SYM_STACK;
			goto continue_modifier;
		}
		if (JITLexer_ISTOK(self, "final")) {
			*pmode |= LOOKUP_SYM_FINAL;
			goto continue_modifier;
		}
	}
	return 0;
/*
err:
	return -1;*/
}


/* Recursively skip a pair of tokens, such as `{' and `}' or `(' and `)'
 * NOTE: Entry is expected to be after the initial instance of `pair_open' */
INTERN int FCALL
JITLexer_SkipPair(JITLexer *__restrict self,
                  unsigned int pair_open,
                  unsigned int pair_close) {
	unsigned char *start   = self->jl_tokstart;
	unsigned int recursion = 1;
	for (;;) {
		if (!self->jl_tok)
			goto err_eof;
		if (self->jl_tok == pair_open) {
			++recursion;
		} else if (self->jl_tok == pair_close) {
			--recursion;
			if (!recursion) {
				JITLexer_Yield(self);
				break;
			}
		}
		JITLexer_Yield(self);
	}
	return 0;
err_eof:
	JITLexer_ErrorTrace(self, start);
	self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Missing `%c' after `%c'",
	                       pair_close, pair_open);
}



PRIVATE DREF DeeObject *FCALL
JITLexer_ParseCatchExprItem(JITLexer *__restrict self) {
	DREF DeeObject *result;
	result = JITLexer_EvalUnary(self, JITLEXER_EVAL_FPRIMARY);
	if (result == JIT_LVALUE)
		result = JITLexer_PackLValue(self);
	return result;
}

/* Parse the catch-mask expression:
 * >> try {
 * >>     throw "Foo";
 * >> } catch (string as s) {
 *             ^      ^
 * >> }
 * Also handles multi-catch masks. */
PRIVATE DREF DeeObject *FCALL
JITLexer_ParseCatchExpr(JITLexer *__restrict self) {
	DREF DeeObject *result, *next, *new_tuple;
	result = JITLexer_ParseCatchExprItem(self);
	if (self->jl_tok == '|' && result) {
		JITLexer_Yield(self);
		next = JITLexer_ParseCatchExprItem(self);
		if unlikely(!next)
			goto err_r;
		new_tuple = DeeTuple_NewUninitialized(2);
		if unlikely(!new_tuple)
			goto err_r_next;
		DeeTuple_SET(new_tuple, 0, result); /* Inherit reference */
		DeeTuple_SET(new_tuple, 1, next);   /* Inherit reference */
		result = new_tuple;
		while (self->jl_tok == '|') {
			size_t length;
			JITLexer_Yield(self);
			/* Parse another catch expression item. */
			next = JITLexer_ParseCatchExprItem(self);
			if unlikely(!next)
				goto err_r;
			length    = DeeTuple_SIZE(result);
			new_tuple = DeeTuple_ResizeUninitialized(result, length + 1);
			if unlikely(!new_tuple)
				goto err_r_next;
			DeeTuple_SET(new_tuple, length, next); /* Inherit reference */
			result = new_tuple;
		}
	}
	return result;
err_r_next:
	Dee_Decref(next);
err_r:
	Dee_Decref(result);
	return NULL;
}

/* Parse the mask portion of a catch statement:
 * >> try {
 * >>     throw "Foo";
 * >> } catch (string as s) {
 *             ^            ^
 * >> }
 */
INTERN int FCALL
JITLexer_ParseCatchMask(JITLexer *__restrict self,
                        DREF DeeObject **__restrict ptypemask,
                        char const **__restrict psymbol_name,
                        size_t *__restrict psymbol_size) {
	if (self->jl_tok == TOK_DOTS) {
		/* >> catch (...) */
		/* >> catch (...var)  (This syntax is allowed, but is rarely ever used;
		 *                     code usually uses `catch (var...)' instead) */
		JITLexer_Yield(self);
		*ptypemask    = NULL;
		*psymbol_name = NULL;
		*psymbol_size = 0;
		if (self->jl_tok == JIT_KEYWORD) {
			/* Specify the catch symbol! */
			*psymbol_name = JITLexer_TokPtr(self);
			*psymbol_size = JITLexer_TokLen(self);
			JITLexer_Yield(self);
		}
	} else {
		if (self->jl_tok == JIT_KEYWORD) {
			unsigned char *start = self->jl_tokstart;
			unsigned char *end   = self->jl_tokend;
			JITLexer_Yield(self);
			if (self->jl_tok == TOK_DOTS) {
				/* `catch (e...)' (catch all into `e') */
				*ptypemask    = NULL;
				*psymbol_name = (char *)start;
				*psymbol_size = (size_t)(end - start);
				JITLexer_Yield(self);
				goto done_skip_rparen;
			}
			JITLexer_YieldAt(self, start);
		}
		/* Parse the catch expression. */
		*ptypemask = JITLexer_ParseCatchExpr(self);
		if unlikely(!*ptypemask)
			goto err;
		*psymbol_name = NULL;
		*psymbol_size = 0;
		/* Check for an `as foo' suffix. */
		if (self->jl_tok == JIT_KEYWORD) {
			/* the `as' is optional */
			if (JITLexer_ISTOK(self, "as")) {
				JITLexer_Yield(self);
				if unlikely(self->jl_tok != JIT_KEYWORD) {
					syn_try_expected_keyword_after_as_in_catch(self);
					goto err_mask;
				}
			}
			*psymbol_name = JITLexer_TokPtr(self);
			*psymbol_size = JITLexer_TokLen(self);
			JITLexer_Yield(self);
		}
	}
done_skip_rparen:
	if unlikely(self->jl_tok != ')') {
		syn_try_expected_rparen_after_catch(self);
		goto err_mask;
	}
	JITLexer_Yield(self);
	return 0;
err_mask:
	Dee_XDecref(*ptypemask);
err:
	return -1;
}

/* Check if `thrown_object' can be caught with `typemask'
 * NOTE: Assumes that interrupt catches are allowed.
 *       If such catches aren't allowed, the caller should
 *       call this function as:
 *       >> can_catch = (!mask || JIT_IsCatchable(obj, mask)) &&
 *       >>             (allow_interrupts || !DeeObject_IsInterrupt(obj)); */
INTERN NONNULL((1, 2)) bool FCALL
JIT_IsCatchable(DeeObject *thrown_object,
                DeeObject *typemask) {
	DeeTypeObject *thrown_object_type;
	/* The runtime uses `instanceof' for dynamic catch-mask detection.
	 * As such, the special case for `foo is none' applies, such that
	 * a type mask of `none' must match itself! */
	if (DeeNone_Check(typemask))
		typemask = (DeeObject *)&DeeNone_Type;
	thrown_object_type = Dee_TYPE(thrown_object);
	/* Special case when matching a thrown object that is a super-view */
	if (thrown_object_type == &DeeSuper_Type)
		thrown_object_type = DeeSuper_TYPE(thrown_object);
	/* Special case for multi-masks */
	if (DeeTuple_Check(typemask)) {
		size_t i;
		for (i = 0; i < DeeTuple_SIZE(typemask); ++i) {
			DeeTypeObject *mask;
			mask = (DeeTypeObject *)DeeTuple_GET(typemask, i);
			if (DeeNone_Check(mask))
				mask = &DeeNone_Type;
			if (DeeType_IsInherited(thrown_object_type, mask))
				return true; /* Got a match! */
		}
		return false;
	}
	/* Fallback: Do a regular is-instance check.
	 * NOTE: `DeeType_IsInherited()' simply returns `false' when
	 *       its second argument isn't actually a type at runtime. */
	return DeeType_IsInherited(thrown_object_type, (DeeTypeObject *)typemask);
}



DECL_END

#ifndef __INTELLISENSE__
#define JIT_SKIP 1
#include "parser-impl.c.inl"

#define JIT_EVAL 1
#include "parser-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEX_JIT_PARSER_C */
