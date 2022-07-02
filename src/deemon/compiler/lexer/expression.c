/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_LEXER_EXPRESSION_C
#define GUARD_DEEMON_COMPILER_LEXER_EXPRESSION_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/tpp.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memchr() */
#include <deemon/tuple.h>

#include "../../runtime/strings.h"

DECL_BEGIN

#define GET_CHOP(x) (ASSERT((x) < 128), chops[x])
INTERN uint8_t const chops[128] = {
/*[[[deemon
#include <util>

local table = dict {
	"+": "OPERATOR_ADD",
	"-": "OPERATOR_SUB",
	"*": "OPERATOR_MUL",
	"/": "OPERATOR_DIV",
	"%": "OPERATOR_MOD",
	"<": "OPERATOR_LO",
	">": "OPERATOR_GR",
	"&": "OPERATOR_AND",
	"|": "OPERATOR_OR",
	"^": "OPERATOR_XOR",
};

for (local x: util::range(128)) {
	local key   = util::chr((uint8_t)x);
	local value = table.get(key);
	if (value is none)
		value = "0";
	print "\t/" "* " + repr(key) + " *" "/", value + ",";
}
]]]*/
	/* "\0" */ 0,
	/* "\1" */ 0,
	/* "\2" */ 0,
	/* "\3" */ 0,
	/* "\4" */ 0,
	/* "\5" */ 0,
	/* "\6" */ 0,
	/* "\a" */ 0,
	/* "\b" */ 0,
	/* "\t" */ 0,
	/* "\n" */ 0,
	/* "\v" */ 0,
	/* "\f" */ 0,
	/* "\r" */ 0,
	/* "\16" */ 0,
	/* "\17" */ 0,
	/* "\20" */ 0,
	/* "\21" */ 0,
	/* "\22" */ 0,
	/* "\23" */ 0,
	/* "\24" */ 0,
	/* "\25" */ 0,
	/* "\26" */ 0,
	/* "\27" */ 0,
	/* "\30" */ 0,
	/* "\31" */ 0,
	/* "\32" */ 0,
	/* "\e" */ 0,
	/* "\34" */ 0,
	/* "\35" */ 0,
	/* "\36" */ 0,
	/* "\37" */ 0,
	/* " " */ 0,
	/* "!" */ 0,
	/* "\"" */ 0,
	/* "#" */ 0,
	/* "$" */ 0,
	/* "%" */ OPERATOR_MOD,
	/* "&" */ OPERATOR_AND,
	/* "\'" */ 0,
	/* "(" */ 0,
	/* ")" */ 0,
	/* "*" */ OPERATOR_MUL,
	/* "+" */ OPERATOR_ADD,
	/* "," */ 0,
	/* "-" */ OPERATOR_SUB,
	/* "." */ 0,
	/* "/" */ OPERATOR_DIV,
	/* "0" */ 0,
	/* "1" */ 0,
	/* "2" */ 0,
	/* "3" */ 0,
	/* "4" */ 0,
	/* "5" */ 0,
	/* "6" */ 0,
	/* "7" */ 0,
	/* "8" */ 0,
	/* "9" */ 0,
	/* ":" */ 0,
	/* ";" */ 0,
	/* "<" */ OPERATOR_LO,
	/* "=" */ 0,
	/* ">" */ OPERATOR_GR,
	/* "?" */ 0,
	/* "@" */ 0,
	/* "A" */ 0,
	/* "B" */ 0,
	/* "C" */ 0,
	/* "D" */ 0,
	/* "E" */ 0,
	/* "F" */ 0,
	/* "G" */ 0,
	/* "H" */ 0,
	/* "I" */ 0,
	/* "J" */ 0,
	/* "K" */ 0,
	/* "L" */ 0,
	/* "M" */ 0,
	/* "N" */ 0,
	/* "O" */ 0,
	/* "P" */ 0,
	/* "Q" */ 0,
	/* "R" */ 0,
	/* "S" */ 0,
	/* "T" */ 0,
	/* "U" */ 0,
	/* "V" */ 0,
	/* "W" */ 0,
	/* "X" */ 0,
	/* "Y" */ 0,
	/* "Z" */ 0,
	/* "[" */ 0,
	/* "\\" */ 0,
	/* "]" */ 0,
	/* "^" */ OPERATOR_XOR,
	/* "_" */ 0,
	/* "`" */ 0,
	/* "a" */ 0,
	/* "b" */ 0,
	/* "c" */ 0,
	/* "d" */ 0,
	/* "e" */ 0,
	/* "f" */ 0,
	/* "g" */ 0,
	/* "h" */ 0,
	/* "i" */ 0,
	/* "j" */ 0,
	/* "k" */ 0,
	/* "l" */ 0,
	/* "m" */ 0,
	/* "n" */ 0,
	/* "o" */ 0,
	/* "p" */ 0,
	/* "q" */ 0,
	/* "r" */ 0,
	/* "s" */ 0,
	/* "t" */ 0,
	/* "u" */ 0,
	/* "v" */ 0,
	/* "w" */ 0,
	/* "x" */ 0,
	/* "y" */ 0,
	/* "z" */ 0,
	/* "{" */ 0,
	/* "|" */ OPERATOR_OR,
	/* "}" */ 0,
	/* "~" */ 0,
	/* "\x7F" */ 0,
//[[[end]]]
};



/* Lookup mode used by secondary AST operands */
#define LOOKUP_SYM_SECONDARY  LOOKUP_SYM_NORMAL

/* Parser flags (Set of `PARSE_F*') */
INTERN uint16_t parser_flags = PARSE_FNORMAL;

/* Return 1 if the current token may be the begin of an expression.
 * @return: 1:  Yes
 * @return: 0:  No
 * @return: -1: Error */
INTERN WUNUSED int DCALL maybe_expression_begin(void) {
	switch (tok) {
	case '+':
	case '-':
	case '~':
	case '(':
	case '#':
	case '<': /* For cells. (deprecated syntax) */
	case '[': /* For lists. */
	case '{': /* Brace initializers. */
	case TOK_INC:
	case TOK_DEC:
	case TOK_INT:
	case TOK_CHAR:
	case TOK_FLOAT:
	case TOK_STRING:
	case TOK_DOTS:
	case TOK_COLON_COLON: /* Deprecated global-accessor syntax. */
		goto yes;

		/* Keywords that can only appear inside of expressions */
	case KWD_as:
	case KWD_in:
	case KWD_is:
		/* Keywords that can only appear inside of expressions/statements */
	case KWD_else:
	case KWD_catch:
	case KWD_finally:
		/* Keywords that can only appear inside of statements */
	case KWD_from:
	case KWD_return:
	case KWD_yield:
	case KWD_throw:
	case KWD_print:
	case KWD_break:
	case KWD_continue:
	case KWD___asm:
	case KWD___asm__:
	case KWD_goto:
	case KWD_switch:
	case KWD_case:
	case KWD_default:
		goto no;

	case '!': {
		struct TPPFile *tok_file;
		struct TPPKeyword *kwd;
		char *tok_begin;
		/* Check if this ! is eventually followed by `is' or `in'
		 * If this is the case, then this can't be the start of an
		 * expression! */
		tok_begin = peek_next_token(&tok_file);
		for (;;) {
			if unlikely(!tok_begin)
				goto err;
			if (*tok_begin != '!')
				break;
			tok_begin = peek_next_advance(tok_begin + 1, &tok_file);
		}
		kwd = peek_keyword(tok_file, tok_begin, 0);
		if (!kwd) {
			if unlikely(tok == TOK_ERR)
				goto err;
		} else {
			/* This isn't an expression. */
			if (kwd->k_id == KWD_is || kwd->k_id == KWD_in)
				goto no;
		}
		goto yes;
	}	break;

	default:
		/* Any kind of keyword can appear in expressions, either
		 * a native expression keyword, or as a variable name. */
		if (TPP_ISKEYWORD(tok))
			goto yes;
		/* XXX: TPP_ISUSERKEYWORD()? */
		goto no;
	}
no:
	return 0;
yes:
	return 1;
err:
	return -1;
}

/* Same as `maybe_expression_begin()', but for the next (peeked) token. */
INTERN WUNUSED int DCALL maybe_expression_begin_peek(void) {
	char *tok_begin, peek;
	struct TPPFile *tok_file;
	tok_begin = peek_next_token(&tok_file);
	if unlikely(!tok_begin)
		goto err;
	peek = *tok_begin;
	switch (peek) {

	case '~':
	case '(':
	case '#':
	case '[': /* For lists. */
	case '{': /* Brace initializers. */
	case '_': /* For identifiers */
	case '$': /* For identifiers */
		goto yes;

	case '+':
	case '-':
	case '<': /* For cells. (deprecated syntax) */
		tok_begin = advance_wraplf(tok_begin);
		if unlikely(!tok_begin)
			goto err;
		{
			char next;
			next = *tok_begin;
			if (next == peek) {
				if (peek == '<')
					goto no; /* `<<' cannot appear at the start of expression */
				goto yes; /* `++' and `--' can appear, though */
			}
			if (next == '=')
				goto no; /* +=, -=, <= can only appear in the middle of expressions! */
		}
		goto yes;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		/* Integer / float */
	case '\'':
	case '\"':
		/* String constants */
		goto yes;

	case '.': /* TOK_DOTS */
		tok_begin = advance_wraplf(tok_begin);
		if (*tok_begin != '.')
			goto no;
		tok_begin = advance_wraplf(tok_begin);
		if (*tok_begin != '.')
			goto no;
		goto yes; /* ... */

	case ':': /* Deprecated global-accessor syntax. */
		tok_begin = advance_wraplf(tok_begin);
		if (*tok_begin != ':')
			goto no;
		goto yes; /* :: */

	case '!': {
		struct TPPKeyword *kwd;
		/* Check if this ! is eventually followed by `is' or `in'
		 * If this is the case, then this can't be the start of an
		 * expression! */
		for (;;) {
			tok_begin = peek_next_advance(tok_begin + 1, &tok_file);
			if unlikely(!tok_begin)
				goto err;
			if (*tok_begin != '!')
				break;
		}
		kwd = peek_keyword(tok_file, tok_begin, 0);
		if (!kwd) {
			if unlikely(tok == TOK_ERR)
				goto err;
		} else {
			/* This isn't an expression. */
			if (kwd->k_id == KWD_is || kwd->k_id == KWD_in)
				goto no;
		}
		goto yes;
	}	break;

	default: {
		struct TPPKeyword *kwd;
		if (!tpp_is_keyword_start(peek))
			goto no;
		kwd = peek_keyword(tok_file, tok_begin, 0);
		if unlikely(!kwd) {
			if unlikely(tok == TOK_ERR)
				goto err;
			goto yes; /* First-time-used user-defined keyword token. */
		}
		switch (kwd->k_id) {

			/* Keywords that can only appear inside of expressions */
		case KWD_as:
		case KWD_in:
		case KWD_is:
			/* Keywords that can only appear inside of expressions/statements */
		case KWD_else:
		case KWD_catch:
		case KWD_finally:
			/* Keywords that can only appear inside of statements */
		case KWD_from:
		case KWD_return:
		case KWD_yield:
		case KWD_throw:
		case KWD_print:
		case KWD_break:
		case KWD_continue:
		case KWD___asm:
		case KWD___asm__:
		case KWD_goto:
		case KWD_switch:
		case KWD_case:
		case KWD_default:
			goto no;

		default:
			break;
		}
		goto yes;
	}

	}
no:
	return 0;
yes:
	return 1;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF struct ast *FCALL
make_bound_expression(struct ast *__restrict base_expr,
                      struct ast_loc *__restrict loc) {
	DREF struct ast *result;
	if (base_expr->a_type == AST_SYM) {
		/* Check if a given symbol is bound. */
		result = ast_setddi(ast_bound(base_expr->a_sym), &base_expr->a_ddi);
	} else if (base_expr->a_type == AST_OPERATOR &&
	           base_expr->a_flag == OPERATOR_GETATTR &&
	           base_expr->a_operator.o_op1) {
		/* Check if a given attribute is bound. */
		result = ast_action2(AST_FACTION_BOUNDATTR,
		                     base_expr->a_operator.o_op0,
		                     base_expr->a_operator.o_op1);
	} else if (base_expr->a_type == AST_OPERATOR &&
	           base_expr->a_flag == OPERATOR_GETITEM &&
	           base_expr->a_operator.o_op1) {
		/* Check if a given item is bound. */
		result = ast_action2(AST_FACTION_BOUNDITEM,
		                     base_expr->a_operator.o_op0,
		                     base_expr->a_operator.o_op1);
	} else if (WARNAST(base_expr, W_CANNOT_TEST_EXPRESSION_BINDING)) {
		result = NULL;
	} else {
		/* Fallback-after-warning: Return `true' */
		result = ast_constexpr(Dee_True);
	}
	return ast_putddi(result, loc);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeString_DecodeLFEscaped(struct unicode_printer *__restrict printer,
                          /*utf-8*/ char const *__restrict start,
                          size_t length) {
	/* Still allow escaped line-feeds! */
	char *flush_start = (char *)start;
	char *end         = (char *)start + length;
	for (;;) {
		char *candidate;
		uint32_t ch;
		candidate = (char *)memchr(start, '\\', (size_t)(end - (char *)start));
		if (!candidate)
			break;
		if (unicode_printer_printutf8(printer, flush_start,
		                              (size_t)(candidate - flush_start)) < 0)
			goto err;
		flush_start = candidate;
		++candidate;
		start = (char *)candidate;
		ASSERT(start <= end);
		if (start < end) {
			ch = utf8_readchar((char const **)&candidate, end);
			if (DeeUni_IsLF(ch)) {
				if (ch == '\r' && candidate < end && *candidate == '\n')
					++candidate; /* CRLF */
				start = flush_start = candidate;
			}
		}
	}
	if (unicode_printer_printutf8(printer, flush_start,
	                              (size_t)(end - flush_start)) < 0)
		goto err;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
ast_decode_unicode_string(struct unicode_printer *__restrict printer) {
	ASSERT(tok == TOK_STRING || tok == TOK_CHAR);
	char *escape_start = token.t_begin;
	char *escape_end   = token.t_end;
	if (escape_start < escape_end && escape_start[0] == 'r') {
		++escape_start;
		if (escape_start < escape_end &&
		    (escape_start[0] == '\"' || escape_start[0] == '\''))
			++escape_start;
		if (escape_end > escape_start &&
		    (escape_end[-1] == '\"' || escape_end[-1] == '\''))
			--escape_end;
		if unlikely(escape_end < escape_start)
			escape_end = escape_start;
		if unlikely(DeeString_DecodeLFEscaped(printer,
		                                      escape_start,
		                                      (size_t)(escape_end - escape_start)))
			goto err;
	} else {
		if (escape_start < escape_end &&
		    (escape_start[0] == '\"' || escape_start[0] == '\''))
			++escape_start;
		if (escape_end > escape_start &&
		    (escape_end[-1] == '\"' || escape_end[-1] == '\''))
			--escape_end;
		if unlikely(escape_end < escape_start)
			escape_end = escape_start;
		if unlikely(DeeString_DecodeBackslashEscaped(printer,
		                                             escape_start,
		                                             (size_t)(escape_end - escape_start),
		                                             STRING_ERROR_FSTRICT))
			goto err;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED DREF DeeObject *FCALL
ast_parse_string(void) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	ASSERT(tok == TOK_STRING || tok == TOK_CHAR);
	do {
		if unlikely(ast_decode_unicode_string(&printer))
			goto err;
		if unlikely(yield() < 0)
			goto err;
	} while (tok == TOK_STRING ||
	         (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS)));
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_sym_import_from_deemon(void) {
	struct symbol *import_symbol;
	import_symbol = new_unnamed_symbol();
	if unlikely(!import_symbol)
		goto err;
	/* Setup an external symbol pointing at `import from deemon' */
	import_symbol->s_type            = SYMBOL_TYPE_EXTERN;
	import_symbol->s_extern.e_module = DeeModule_GetDeemon();
	Dee_Incref(import_symbol->s_extern.e_module);
	import_symbol->s_extern.e_symbol = DeeModule_GetSymbolString(import_symbol->s_extern.e_module,
	                                                             DeeString_STR(&str_import),
	                                                             DeeString_Hash((DeeObject *)&str_import));
	ASSERT(import_symbol->s_extern.e_symbol);
	return ast_sym(import_symbol);
err:
	return NULL;
}


INTERN WUNUSED DREF struct ast *FCALL
ast_parse_unaryhead(unsigned int lookup_mode) {
	DREF struct ast *result;
	DREF struct ast *merge;
	struct ast_loc loc;
	uint32_t old_flags;
	switch (tok) {

	case TOK_INT: {
		tint_t value;
		DREF DeeObject *resval;
		/* Use our own integer parser, so we can process infinite-precision integers. */
		resval = DeeInt_FromString(token.t_begin, (size_t)(token.t_end - token.t_begin),
		                           DEEINT_STRING(0, DEEINT_STRING_FESCAPED |
		                                            DEEINT_STRING_FTRY));
		/* Check if the integer failed to be parsed. */
		if unlikely(resval == ITER_DONE) {
			if (WARN(W_INVALID_INTEGER))
				goto err;
			goto create_none;
		}
create_constexpr:
		if unlikely(!resval)
			goto err;
		result = ast_sethere(ast_constexpr(resval));
		Dee_Decref(resval);
		if unlikely(!result)
			goto err;
		if unlikely(yield() < 0)
			goto err_r;
		break;
	case TOK_CHAR:
		if (!HAS(EXT_CHARACTER_LITERALS))
			goto decode_string;
		if unlikely(TPP_Atoi(&value) == TPP_ATOI_ERR)
			goto err;
		if (WARN(W_DEPRECATED_CHARACTER_INT))
			goto err;
		resval = DeeInt_NewS64(value);
		goto create_constexpr;
	}	break;

	case TOK_STRING: {
		DREF DeeObject *resval;
decode_string:
		loc_here(&loc);
		resval = ast_parse_string();
		if unlikely(!resval)
			goto err;
		result = ast_setddi(ast_constexpr(resval), &loc);
		Dee_Decref(resval);
	}	break;

	case KWD_f:
	case KWD_F:
		/* Check if this might be a template string. */
		if ((*token.t_end == '\"') ||
		    (*token.t_end == '\'' && !HAS(EXT_CHARACTER_LITERALS))) {
			if unlikely(yield() < 0)
				goto err;
			ASSERT(tok == TOK_STRING || tok == TOK_CHAR);
			result = ast_parse_template_string();
			break;
		}
		goto do_keyword;

	case TOK_FLOAT: {
		tfloat_t value;
		DREF DeeObject *resval;
		if (TPP_Atof(&value) == TPP_ATOF_ERR)
			goto err;
		resval = DeeFloat_New((double)value);
		if unlikely(!resval)
			goto err;
		/* Construct a new branch for the constant float value. */
		result = ast_sethere(ast_constexpr(resval));
		Dee_Decref(resval);
		if unlikely(!result)
			goto err;
		if unlikely(yield() < 0)
			goto err_r;
	}	break;

	case KWD_none: {
		DeeObject *constval;
create_none:
		constval = Dee_None;
mkconst:
		result = ast_sethere(ast_constexpr(constval));
		if unlikely(!result)
			goto err;
		if unlikely(yield() < 0)
			goto err_r;
		break;
	case KWD_true:
		constval = Dee_True;
		goto mkconst;
	case KWD_false:
		constval = Dee_False;
		goto mkconst;
	}	break;

	case KWD_bound: {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (tok == '(') {
			result = ast_parse_unaryhead(LOOKUP_SYM_SECONDARY |
			                             PARSE_UNARY_DISALLOW_CASTS);
		} else {
			result = ast_parse_unary(LOOKUP_SYM_SECONDARY);
		}
		if unlikely(!result)
			goto err;
		merge = make_bound_expression(result, &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		result = merge;
	}	break;


	case KWD_str: {
		uint16_t opid; /* Unary expressions. */
		opid = OPERATOR_STR;
		goto do_unary_operator_kwd;
	case KWD_repr:
		opid = OPERATOR_REPR;
		goto do_unary_operator_kwd;
	case KWD_copy:
		opid = OPERATOR_COPY;
		goto do_unary_operator_kwd;
	case KWD_deepcopy:
		opid = OPERATOR_DEEPCOPY;
do_unary_operator_kwd:
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (tok == '(') {
			result = ast_parse_unaryhead(LOOKUP_SYM_SECONDARY |
			                             PARSE_UNARY_DISALLOW_CASTS);
		} else {
			result = ast_parse_unary(LOOKUP_SYM_SECONDARY);
		}
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_operator1(opid, 0, result), &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		result = merge;
		break;
	case '#':
		opid = OPERATOR_SIZE;
		goto do_unary_operator;
	case '~':
		opid = OPERATOR_INV;
		goto do_unary_operator;
	case '+':
		opid = OPERATOR_POS;
		goto do_unary_operator;
	case '-':
		opid = OPERATOR_NEG;
		goto do_unary_operator;
	case TOK_INC:
		opid = OPERATOR_INC;
		goto do_unary_operator;
	case TOK_DEC:
		opid = OPERATOR_DEC;
do_unary_operator:
		loc_here(&loc);
		if (yield() < 0)
			goto err;
		result = ast_parse_unary(LOOKUP_SYM_SECONDARY);
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_operator1(opid, 0, result), &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		result = merge;
		break;
	case KWD_type:
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (tok == '(') {
			result = ast_parse_unaryhead(LOOKUP_SYM_SECONDARY |
			                             PARSE_UNARY_DISALLOW_CASTS);
		} else {
			result = ast_parse_unary(LOOKUP_SYM_SECONDARY);
		}
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_action1(AST_FACTION_TYPEOF, result), &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		result = merge;
	}	break;

	case '!': /* not */
		loc_here(&loc);
		if (yield() < 0)
			goto err;
		result = ast_parse_unary(LOOKUP_SYM_SECONDARY);
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_bool(AST_FBOOL_NEGATE, result), &loc);
		ast_decref(result);
		result = merge;
		break;

		/* I admit it. I was an idiot for adding this dedicated syntax for constructing
		 * Cell objects. I wasn't thinking and as a result of that, I have to maintain
		 * it for backwards compatibility.
		 * Anyways... The most I can do for now is have it emit a warning, telling that
		 * you should be using `Cell from deemon' instead (which actually won't even
		 * break backwards-compatibility with the old deemon, who's `Cell' object
		 * offered you the same functionality)
		 * NOTE: To ensure backwards-compatibility, you may place this
		 *       in your code in order to simply always use `Cell(...)':
		 * >> #if __DEEMON__ >= 200
		 * >> import Cell from deemon;
		 * >> #else
		 * >> #define Cell(...) < __VA_ARGS__ >
		 * >> #endif */
	case TOK_LOGT:
		loc_here(&loc);
		if (WARN(W_DEPRECATED_CELL_SYNTAX))
			goto err;
		goto do_empty_cell;

	case '<': /* Cell (deprecated syntax) */
		loc_here(&loc);
		if (WARN(W_DEPRECATED_CELL_SYNTAX))
			goto err;
		if (yield() < 0)
			goto err;
		if (tok == '>') {
			/* empty Cell. */
do_empty_cell:
			result = ast_action0(AST_FACTION_CELL0);
		} else {
			result = ast_parse_unary(LOOKUP_SYM_SECONDARY);
			if unlikely(!result)
				goto err;
			if (tok != '>' && WARN(W_EXPECTED_RANGLE_AFTER_LANGLE))
				goto err_r;
			merge = ast_action1(AST_FACTION_CELL1, result);
			ast_decref(result);
			result = merge;
		}
		ast_setddi(result, &loc);
		if unlikely(yield() < 0)
			goto err_r;
		break;

	case KWD_if: {
		/* if-in-expressions. */
		DREF struct ast *tt_branch;
		DREF struct ast *ff_branch;
		uint16_t expect;
		expect = current_tags.at_expect;
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_IF))
			goto err_flags;
		result = ast_parse_expr(LOOKUP_SYM_SECONDARY);
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if unlikely(!result)
			goto err;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_IF))
			goto err;
		tt_branch = NULL;
		if (tok != KWD_else && tok != KWD_elif) {
			tt_branch = ast_parse_expr(LOOKUP_SYM_SECONDARY);
			if unlikely(!tt_branch)
				goto err_r;
		}
		ff_branch = NULL;
		if (tok == KWD_elif) {
			token.t_id = KWD_if; /* Cheat a bit... */
			goto do_else_branch;
		}
		if (tok == KWD_else) {
			if unlikely(yield() < 0) {
err_tt:
				ast_xdecref(tt_branch);
				goto err_r;
			}
do_else_branch:
			ff_branch = ast_parse_expr(LOOKUP_SYM_SECONDARY);
			if unlikely(!ff_branch)
				goto err_tt;
		}
		merge = ast_setddi(ast_conditional(AST_FCOND_EVAL | expect, result, tt_branch, ff_branch), &loc);
		ast_xdecref(ff_branch);
		ast_xdecref(tt_branch);
		ast_xdecref(result);
		result = merge;
	}	break;

	case KWD_assert:
		result = ast_parse_assert(true);
		break;

	case KWD_function: {
		struct TPPKeyword *function_name;
		if (WARN(W_DEPRECATED_FUNCTION_IN_EXPRESSION))
			goto err;
		/* Create a new function */
		loc_here(&loc);
		function_name = NULL;
		if unlikely(yield() < 0)
			goto err;
		if (TPP_ISKEYWORD(tok)) {
			function_name = token.t_kwd;
			if unlikely(yield() < 0)
				goto err;
		}
		result = ast_parse_function(function_name, NULL, false, &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
		                            ,
		                            NULL
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
		                            );
	}	break;

	case KWD_final: {
		struct TPPKeyword *class_name;
		uint16_t class_flags;
		class_flags = TP_FFINAL;
		if unlikely(yield() < 0)
			goto err;
		if (unlikely(tok != KWD_class) &&
		    WARN(W_EXPECTED_CLASS_AFTER_FINAL))
			goto err;
		goto do_create_class;
	case KWD_class:
		class_flags = TP_FNORMAL;
do_create_class:
		/* Create a new function */
		loc_here(&loc);
		class_name = NULL;
		if unlikely(yield() < 0)
			goto err;
		if (TPP_ISKEYWORD(tok)) {
			if (tok == KWD_final && !(class_flags & TP_FFINAL)) {
				/* allow `class final' as an alias for `final class' */
				if unlikely(yield() < 0)
					goto err;
				class_flags |= TP_FFINAL;
			}
			if (TPP_ISKEYWORD(tok)) {
				class_name = token.t_kwd;
				if unlikely(yield() < 0)
					goto err;
			}
		}
		/* Actually do parse the class now. */
		result = ast_setddi(ast_parse_class(class_flags,
		                                    class_name,
		                                    false,
		                                    LOOKUP_SYM_NORMAL),
		                    &loc);
	}	break;

	case KWD_pack: {
		int has_paren;
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		has_paren = 0;
		old_flags = TPPLexer_Current->l_flags;
		if (tok == '(') {
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err_flags;
			has_paren = tok == '(' ? 2 : 1;
		} else {
			if unlikely(parser_warn_pack_used(&loc))
				goto err;
		}
		if (tok == '{') {
			/* Statements in expressions. */
			result = ast_parse_statement_or_braces(NULL);
		} else {
			int temp;
			temp = maybe_expression_begin();
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err_flags;
				result = ast_constexpr(Dee_EmptyTuple);
			} else {
				/* Parse the packed expression. */
				result = ast_parse_comma(has_paren
				                         ? AST_COMMA_FORCEMULTIPLE
				                         : AST_COMMA_FORCEMULTIPLE | AST_COMMA_STRICTCOMMA,
				                         AST_FMULTIPLE_TUPLE,
				                         NULL);
#if 0 /* Because of the `AST_COMMA_FORCEMULTIPLE', this is unnecessary */
				if likely(result &&
				          result->a_type == AST_EXPAND) {
					/* Wrap into a single-item tuple multiple-branch:
					 * >> print pack get_items()...; // Convert to tuple. */
					DREF struct ast **exprv;
					ast_setddi(result, &loc);
					exprv = (DREF struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
					if unlikely(!exprv)
						goto err_r_flags;
					exprv[0] = result; /* Inherit */
					merge    = ast_multiple(AST_FMULTIPLE_TUPLE, 1, exprv);
					if unlikely(!merge) {
						Dee_Free(exprv);
						goto err_r_flags;
					}
					result = merge; /* Inherit */
				}
#endif
			}
		}
		ast_setddi(result, &loc);
		if unlikely(!result)
			goto err_flags;
		if (has_paren) {
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if (skip(')', W_EXPECTED_RPAREN_AFTER_PACK))
				goto err_r;
#if 0 /* The `result->a_type != AST_MULTIPLE' would never \
       * fly, because of the `AST_COMMA_FORCEMULTIPLE' */
			if (has_paren == 1 && result->a_type != AST_MULTIPLE &&
			    !(lookup_mode & PARSE_UNARY_DISALLOW_CASTS)) {
				/* C-style cast expression (only for single-parenthesis expressions) */
				merge = ast_parse_cast(result);
				ast_decref(result);
				result = merge;
			}
#endif
		}
	}	break;

	case KWD_import:
		result = ast_sethere(ast_sym_import_from_deemon());
		if unlikely(!result)
			goto err;
		if unlikely(yield() < 0)
			goto err_r;
		/* The specs officially only allow `import' in expression, when followed by `(' or `pack'
		 * So we simply emit an error if what follows isn't one of those. */
		if (tok != '(' && tok != KWD_pack &&
		    WARN(W_EXPECTED_LPAREN_AFTER_IMPORT))
			goto err_r;
		break;

	case KWD_do:
	case KWD_while:
	case KWD_for:
	case KWD_foreach:
		/* Loop expressions. */
		result = ast_parse_loopexpr();
		break;

	case '(': {
		bool allow_cast;
		/* Parenthesis. */
		loc_here(&loc);
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		allow_cast = tok != '(' && !(lookup_mode & PARSE_UNARY_DISALLOW_CASTS);
		if (tok == '{') {
			unsigned int was_expression;
			/* Statements in expressions. */
			result = ast_parse_statement_or_braces(&was_expression);
			if unlikely(!result)
				goto err_flags;
			allow_cast = false; /* Don't allow braces, or statements as cast expressions. */
			if (tok == ',' && was_expression != AST_PARSE_WASEXPR_NO) {
				DREF struct ast **tuple_branchv;
				if unlikely(yield() < 0)
					goto err_r_flags;
				if (tok == ')') {
					/* single-element tuple expression, where the single element is a sequence. */
					tuple_branchv = (struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
					if unlikely(!tuple_branchv)
						goto err_r_flags;
					tuple_branchv[0] = result; /* Inherit reference. */
					merge            = ast_multiple(AST_FMULTIPLE_TUPLE, 1, tuple_branchv);
					if unlikely(!merge) {
						Dee_Free(tuple_branchv);
						goto err_r_flags;
					}
					result = merge;
				} else {
					/* There are more items! */
					merge = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
					                        AST_FMULTIPLE_TUPLE,
					                        NULL);
					if unlikely(!merge)
						goto err_r_flags;
					ASSERT(merge->a_type == AST_MULTIPLE ||
					       (merge->a_type == AST_CONSTEXPR &&
					        merge->a_constexpr == Dee_EmptyTuple));
					if (merge->a_constexpr == Dee_EmptyTuple) {
						tuple_branchv = (struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
						if unlikely(!tuple_branchv)
							goto err_flags_merge_r;
						Dee_DecrefNokill(merge->a_constexpr);
						merge->a_type            = AST_MULTIPLE;
						merge->a_flag            = AST_FMULTIPLE_TUPLE;
						merge->a_multiple.m_astc = 1;
					} else {
						tuple_branchv = (struct ast **)Dee_Realloc(merge->a_multiple.m_astv,
						                                           (merge->a_multiple.m_astc + 1) *
						                                           sizeof(DREF struct ast *));
						if unlikely(!tuple_branchv)
							goto err_flags_merge_r;
						memmoveupc(tuple_branchv + 1,
						           tuple_branchv,
						           merge->a_multiple.m_astc,
						           sizeof(DREF struct ast *));
						++merge->a_multiple.m_astc;
					}
					tuple_branchv[0]         = result; /* Inherit reference. */
					merge->a_multiple.m_astv = tuple_branchv;
					result                   = merge; /* Inherit reference. */
				}
			}
			if (tok != ')' && was_expression != AST_PARSE_WASEXPR_NO) {
				result = ast_parse_postexpr(result);
				if unlikely(!result)
					goto err_flags;
				/*was_expression = AST_PARSE_WASEXPR_YES;*/
			}
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if (skip(')', W_EXPECTED_RPAREN_AFTER_LPAREN))
				goto err_r;
		} else {
			if (tok == ')') {
				TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
				if (skip(')', W_EXPECTED_RPAREN_AFTER_LPAREN))
					goto err;

				/* Support for java-style lambda with empty argument list. */
#ifdef CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS
				if (tok == TOK_ARROW) {
					result = ast_parse_function_java_lambda(NULL, NULL);
					result = ast_setddi(result, &loc);
					break;
				}
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				if (tok == ':') {
					bool isarrow;
					struct TPPLexerPosition pos;
					if unlikely(!TPPLexer_SavePosition(&pos))
						goto err;
					if unlikely(yield() < 0) {
err_restore_pos:
						TPPLexer_LoadPosition(&pos);
						goto err;
					}
					if unlikely(decl_ast_skip())
						goto err_restore_pos;
					isarrow = tok == TOK_ARROW;
					TPPLexer_LoadPosition(&pos);
					if (isarrow) {
						result = ast_parse_function_java_lambda(NULL, NULL);
						result = ast_setddi(result, &loc);
						break;
					}
				}
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
#endif /* CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS */

				/* Empty tuple. */
				result = ast_constexpr(Dee_EmptyTuple);
				if unlikely(!result)
					goto err;
				allow_cast = false; /* Don't allow empty tuples for cast expressions. */
			} else {
				/* Lambda function. */
#ifdef CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS
				int error = ast_is_after_lapren_of_java_lambda();
				if (error != 0) {
					if unlikely(error < 0)
						goto err;
					TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
					result = ast_parse_function_java_lambda(NULL, NULL);
					result = ast_setddi(result, &loc);
					break;
				}
#endif /* CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS */

				/* Parenthesis / tuple expression. */
				result = ast_parse_comma(AST_COMMA_NORMAL,
				                         AST_FMULTIPLE_TUPLE,
				                         NULL);
				if unlikely(!result)
					goto err_flags;
				if (result->a_type == AST_EXPAND) {
					/* Wrap into a single-item tuple multiple-branch:
					 * >> print (get_items()...); // Convert to tuple. */
					DREF struct ast **exprv;
					ast_setddi(result, &loc);
					exprv = (DREF struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
					if unlikely(!exprv)
						goto err_r_flags;
					exprv[0] = result; /* Inherit */
					merge    = ast_multiple(AST_FMULTIPLE_TUPLE, 1, exprv);
					if unlikely(!merge) {
						Dee_Free(exprv);
						goto err_r_flags;
					}
					result = merge; /* Inherit */
				}
				if (result->a_type == AST_MULTIPLE)
					allow_cast = false; /* Don't allow comma-lists for cast expressions. */
				TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
				if (skip(')', W_EXPECTED_RPAREN_AFTER_LPAREN))
					goto err_r;
			}
			if (tok == '{' && WARN(W_PROBABLY_MISSING_ARROW))
				goto err;
		}
		result = ast_putddi(result, &loc);
		if (allow_cast &&
		    result->a_type != AST_MULTIPLE) {
			/* C-style cast expression (only for single-parenthesis expressions) */
			merge = ast_parse_cast(result);
			ast_decref(result);
			result = merge;
		}
	}	break;

	case '{':
		/* Brace-style sequence or mapping expression. */
		loc_here(&loc);
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		result = ast_setddi(ast_parse_brace_items(), &loc);
		if unlikely(!result)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip('}', W_EXPECTED_RBRACE_AFTER_BRACEINIT))
			goto err_r;
		break;

	case KWD_del:
		/* Delete expression. */
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_DEL))
			goto err_flags;
		result = ast_putddi(ast_parse_del(lookup_mode & ~PARSE_UNARY_DISALLOW_CASTS), &loc);
		if unlikely(!result)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_DEL))
			goto err_r;
		break;

	case KWD_with:
		result = ast_parse_with(false, false);
		break;

	case KWD_try:
		result = ast_parse_try(false);
		break;

	case KWD_operator: {
		int32_t name;
		/* Named, but unbound operator invocation. */
		if unlikely(yield() < 0)
			goto err;
		loc_here(&loc);
		name = ast_parse_operator_name(P_OPERATOR_FNORMAL);
		if unlikely(name < 0)
			goto err;
		if (tok != '(') {
			/* Bound-operator expression. */
			merge = ast_operator_func((uint16_t)name, NULL);
		} else {
			DREF struct ast *other;
			old_flags = TPPLexer_Current->l_flags;
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err_flags;
			if (tok == ')') {
				other = ast_constexpr(Dee_EmptyTuple);
			} else {
				other = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
				                        AST_FMULTIPLE_TUPLE,
				                        NULL);
			}
			if unlikely(!other)
				goto err_flags;
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if (skip(')', W_EXPECTED_RPAREN))
				goto err;
			merge = ast_build_operator((uint16_t)name,
			                           /* Set the MAYBEPFX flag to suppress errors that
			                            * would normally cause the assembler to fail when
			                            * attempting to use an inplace operator on a
			                            * non-inplace symbol. */
			                           AST_OPERATOR_FMAYBEPFX,
			                           other);
			ast_decref(other);
		}
		if unlikely(!merge)
			goto err;
		result = ast_setddi(merge, &loc);
	}	break;


	case '[': /* List */
		loc_here(&loc);
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		if (tok == '&' || tok == '=') {
			if (WARN(W_DEPRECATED_LAMBDA_MODE))
				goto err_flags;
			loc_here(&loc);
			if unlikely(yield() < 0)
				goto err_flags;
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if (skip(']', W_EXPECTED_RBRACKET_AFTER_LAMBDA))
				goto err;
			goto do_lambda;
		}
		if (tok == ']') {
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err;
			if (tok == ':') {
				/* If the current token is ':', try to skip over the return type
				 * annotation and check if the next token thereafter is '->' or '{'. */
				tok_t token_after;
				struct TPPLexerPosition saved;
				if unlikely(!TPPLexer_SavePosition(&saved))
					goto err;
				if unlikely(yield() < 0) {
err_restore_pos_in_old_lambda:
					TPPLexer_LoadPosition(&saved);
					goto err;
				}
				if unlikely(decl_ast_skip())
					goto err_restore_pos_in_old_lambda;
				token_after = tok;
				TPPLexer_LoadPosition(&saved);
				if (token_after == '{' || token_after == TOK_ARROW)
					goto do_lambda; /* Yup: it's a lambda! */
			}
			if (tok == '(' || tok == '{' || tok == TOK_ARROW) {
do_lambda:
				old_flags = TPPLexer_Current->l_flags;
				TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
				if unlikely(parse_tags_block())
					goto err_flags;
				result = ast_parse_function(NULL, NULL, true, &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				                            ,
				                            NULL
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
				                            );
				TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
				break;
			}
			result = ast_multiple(AST_FMULTIPLE_LIST, 0, NULL);
		} else {
			if (tok == ':') {
				DREF struct ast *begin_expression;
				DREF struct ast *step_expression;
				result = ast_constexpr(Dee_None);
				if unlikely(!result)
					goto err_flags;
				/* Range without begin index. */
do_range_expression:
				if unlikely(yield() < 0)
					goto err_r_flags;
				begin_expression = result; /* Inherit reference. */
				/* Parse the end index. */
				if (tok == ',') {
					/* No end index given. */
					result = ast_constexpr(Dee_None);
				} else {
					result = ast_parse_expr(LOOKUP_SYM_SECONDARY);
				}
				if unlikely(!result) {
					ast_decref(begin_expression);
					goto err_flags;
				}
				if (tok == ',') {
					if unlikely(yield() < 0) {
err_begin_expr:
						ast_decref(begin_expression);
						goto err_r_flags;
					}
					step_expression = ast_parse_expr(LOOKUP_SYM_SECONDARY);
				} else {
					step_expression = ast_constexpr(Dee_None);
				}
				if unlikely(!step_expression)
					goto err_begin_expr;
				/* Create the range expression. */
				merge = ast_action3(AST_FACTION_RANGE, begin_expression, result, step_expression);
				ast_decref(begin_expression);
				ast_decref(result);
				ast_decref(step_expression);
				if unlikely(!merge)
					goto err_flags;
				result = merge;
			} else {
				result = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
				                         AST_FMULTIPLE_LIST,
				                         NULL);
				if unlikely(!result)
					goto err_flags;
				if (tok == ':' &&
				    result->a_type == AST_MULTIPLE &&
				    result->a_multiple.m_astc == 1) {
					merge = result->a_multiple.m_astv[0];
					ast_incref(merge);
					ast_decref(result);
					result = merge;
					/* Range with custom start index. */
					goto do_range_expression;
				}
			}
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if (skip(']', W_EXPECTED_RBRACKET_AFTER_LIST))
				goto err_r;
		}
		ast_setddi(result, &loc);
		break;

	case KWD_this: {
		struct symbol *this_sym;
		this_sym = get_current_this();
		if (!this_sym)
			goto default_case;
		result = ast_sethere(ast_sym(this_sym));
		if unlikely(yield() < 0)
			goto err_r;
	}	break;

	case KWD_super: {
		struct symbol *this_sym;
		DREF struct ast *this_ast;
		if (!current_scope->s_class ||
		    !current_scope->s_class->cs_super)
			goto default_case;
		this_sym = get_current_this();
		if (!this_sym)
			goto default_case;
		this_ast = ast_sethere(ast_sym(this_sym));
		if unlikely(!this_ast)
			goto err;
		merge = ast_sethere(ast_sym(current_scope->s_class->cs_super));
		if unlikely(!merge) {
			ast_decref(this_ast);
			goto err;
		}
		result = ast_sethere(ast_action2(AST_FACTION_AS, this_ast, merge));
		ast_decref(merge);
		ast_decref(this_ast);
		if unlikely(!result)
			goto err;
		if unlikely(yield() < 0)
			goto err_r;
	}	break;

	case KWD___nth:
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_NTH))
			goto err_flags;
		result = ast_parse_expr(LOOKUP_SYM_SECONDARY);
		if unlikely(!result)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		/* Optimize the ast-expression to propagate constant, thus
		 * allowing the use of `__nth(2+3)' instead of forcing the
		 * user to write `__nth(5)' or `__nth(__TPP_EVAL(2+3))' */
		if (ast_optimize_all(result, true))
			goto err_r;
		if (result->a_type != AST_CONSTEXPR &&
		    WARN(W_EXPECTED_CONSTANT_AFTER_NTH))
			goto err_r;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_NTH))
			goto err_r;
		if (TPP_ISKEYWORD(tok)) {
			unsigned int nth_symbol = 0;
			struct symbol *sym;
			if (result->a_type == AST_CONSTEXPR &&
			    DeeObject_AsUInt(result->a_constexpr, &nth_symbol)) {
				DeeError_Handled(ERROR_HANDLED_RESTORE);
				if (WARN(W_EXPECTED_CONSTANT_AFTER_NTH))
					goto err_r;
			}
			ast_decref(result);
			sym = lookup_nth(nth_symbol, token.t_kwd);
			if likely(sym) {
				result = ast_sym(sym);
			} else {
				if (WARN(W_UNKNOWN_NTH_SYMBOL, nth_symbol))
					goto err;
				result = ast_constexpr(Dee_None);
			}
			if unlikely(!result)
				goto err;
			ast_sethere(result);
			if unlikely(yield() < 0)
				goto err_r;
		} else {
			if (WARN(W_EXPECTED_KEYWORD_AFTER_NTH))
				goto err_r;
		}
		break;

	case TOK_COLON_COLON:
		if (WARN(W_DEPRECATED_GLOBAL_PREFIX))
			goto err;
		ATTR_FALLTHROUGH
	case KWD_global:
		lookup_mode |= LOOKUP_SYM_VGLOBAL;
		goto do_warn_deprecated_modifier;
	case KWD_local:
		lookup_mode |= LOOKUP_SYM_VLOCAL;
do_warn_deprecated_modifier:
		if (WARN(W_DEPRECATED_PREFIX_IN_EXPRESSION))
			goto err;
		if unlikely(yield() < 0)
			goto err;
		goto default_case;

	case TOK_DOTS:
		if (current_basescope->bs_varargs) {
			DREF struct ast *new_result;
			/* Reference the varargs portion of the argument list. */
			result = ast_sethere(ast_sym(current_basescope->bs_varargs));
			if unlikely(!result)
				goto err;
			/* The old deemon neglected to do this, but we re-package
			 * the varargs portion of the argument list as an expand
			 * expression, thus allowing it to simply be forwarded
			 * in other function calls:
			 * >> function foo(a, b) {
			 * >>     print a, b;
			 * >> }
			 * >> function bar(...) {
			 * >>     foo(...); // In the old deemon you'd have to write `foo((...)...);'
			 * >> }
			 */
			new_result = ast_sethere(ast_expand(result));
			ast_decref(result);
			if unlikely(!new_result)
				goto err;
			result = new_result;
			if unlikely(yield() < 0)
				goto err_r;
			break;
		}
		ATTR_FALLTHROUGH
	default:
default_case:
		if (TPP_ISKEYWORD(tok)) {
			/* Perform a regular symbol lookup. */
			struct TPPKeyword *name;
do_keyword:
			name = token.t_kwd;
			loc_here(&loc);
			if unlikely(yield() < 0)
				goto err;
			if (tok == KWD_from) {
				/* `Error from deemon' - Short form of `import Error from deemon' */
				loc_here(&loc);
				if unlikely(yield() < 0)
					goto err;
				result = ast_parse_import_single(name);
				if (result && (tok != ';' && tok != ',' && tok != ')' &&
				               tok != '}' && tok != ']' && tok > 0 &&
				               /* Don't emit this warning from macros! */
				               token.t_file->f_kind == TPPFILE_KIND_TEXT)) {
					/* Warn about bad readability in code like:
					 * >> int from deemon(42)
					 * Which should really be written as:
					 * >> (int from deemon)(42) */
					char const *symname = "<symbol>";
					char const *modname = "<module>";
					if (result->a_type == AST_SYM) {
						struct symbol *sym = result->a_sym;
						if (sym->s_type == SYMBOL_TYPE_EXTERN) {
							symname = sym->s_extern.e_symbol->ss_name;
							modname = DeeString_STR(sym->s_extern.e_module->mo_name);
						}
					}
					if (WARNAT(&loc, W_UNCLEAR_SYMBOL_FROM_MODULE, symname, modname))
						goto err;
				}
#ifdef CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS
			} else if (tok == TOK_ARROW) {
				/* Support for java-style lambda with singular argument. */
				result = ast_parse_function_java_lambda(name, &loc);
#endif /* CONFIG_LANGUAGE_HAVE_JAVA_LAMBDAS */
			} else {
				DREF struct symbol *sym;
				sym = lookup_symbol(lookup_mode & ~PARSE_UNARY_DISALLOW_CASTS, name, &loc);
				if unlikely(!sym)
					goto err;
				result = ast_sym(sym);
			}
		} else {
			if (WARN(W_UNEXPECTED_TOKEN_IN_EXPRESSION))
				goto err;
			result = ast_constexpr(Dee_None);
		}
		result = ast_setddi(result, &loc);
		break;
	}
	return result;
err_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	goto err;
err_r_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err_r:
	ast_decref(result);
err:
	return NULL;
err_flags_merge_r:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
/*err_merge_r:*/
	Dee_Decref(merge);
	goto err_r;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_unary_operand(/*inherit(always)*/ DREF struct ast *__restrict result) {
	DREF struct ast *merge;
	DREF struct ast *other;
	struct ast_loc loc;
	uint32_t old_flags;
	do {
		switch (tok) {

		case TOK_COLON_COLON:
			/* Backwards compatibility with deemon 100+ */
			if (WARN(W_DEPRECATED_ATTRIBUTE_SYNTAX))
				goto err_r;
			ATTR_FALLTHROUGH
		case '.': /* Attribute lookup */
			loc_here(&loc);
			if (yield() < 0)
				goto err_r;
			if (TPP_ISKEYWORD(tok)) {
				DREF DeeObject *attr_name;
				if (tok == KWD_this)
					goto got_attr; /* This is really just a no-op. */
				if (tok == KWD_class) {
					/* Return the real class of a type, properly unwinding super. */
					merge = ast_sethere(ast_action1(AST_FACTION_CLASSOF, result));
				} else if (tok == KWD_super) {
					/* Return the real class of a type, properly unwinding super. */
					merge = ast_sethere(ast_action1(AST_FACTION_SUPEROF, result));
				} else if (tok == KWD_operator) {
					/* Named & bound operator invocation. */
					int32_t name;
					if unlikely(yield() < 0)
						goto err_r;
					loc_here(&loc);
					name = ast_parse_operator_name(P_OPERATOR_FNORMAL);
					if unlikely(name < 0)
						goto err_r;
					if (tok != '(') {
						/* Bound-operator expression. */
						merge = ast_operator_func((uint16_t)name, result);
					} else {
						old_flags = TPPLexer_Current->l_flags;
						TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
						if (yield() < 0)
							goto err_r_flags;
						if (tok == ')') {
							other = ast_constexpr(Dee_EmptyTuple);
						} else {
							other = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
							                        AST_FMULTIPLE_TUPLE,
							                        NULL);
						}
						if unlikely(!other)
							goto err_r_flags;
						TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
						if (skip(')', W_EXPECTED_RPAREN))
							goto err_r;
						merge = ast_build_bound_operator((uint16_t)name,
						                                 /* Set the MAYBEPFX flag to suppress errors that
						                                  * would normally cause the assembler to fail when
						                                  * attempting to use an inplace operator on a
						                                  * non-inplace symbol. */
						                                 AST_OPERATOR_FMAYBEPFX,
						                                 result, other);
						ast_decref(other);
					}
					ast_decref(result);
					if unlikely(!merge)
						goto err;
					result = ast_setddi(merge, &loc);
					goto got_attr2;
				} else {
					if (is_reserved_symbol_name(token.t_kwd) &&
					    WARN(W_RESERVED_ATTRIBUTE_NAME, token.t_kwd))
						goto err;
					attr_name = DeeString_NewSized(token.t_kwd->k_name,
					                               token.t_kwd->k_size);
					if unlikely(!attr_name)
						goto err_r;
					other = ast_sethere(ast_constexpr(attr_name));
					Dee_Decref(attr_name);
					if unlikely(!other)
						goto err_r;
					merge = ast_setddi(ast_operator2(OPERATOR_GETATTR, 0, result, other), &loc);
					ast_decref(other);
					ast_decref(result);
					if unlikely(!merge)
						goto err;
					result = merge;
					if unlikely(yield() < 0)
						goto err_r;
					goto got_attr2;
				}
				ast_decref(result);
				if unlikely(!merge)
					goto err;
				result = merge;
got_attr:
				if unlikely(yield() < 0)
					goto err_r;
got_attr2:;
			} else {
				if (WARN(W_EXPECTED_KEYWORD_AFTER_DOT))
					goto err_r;
			}
			break;

		case '[': {
			/* sequence operator */
			loc_here(&loc);
			old_flags = TPPLexer_Current->l_flags;
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err_r_flags;
			if (tok == ':') {
				other = ast_constexpr(Dee_None);
				if unlikely(!other)
					goto err_r_flags;
				goto do_range;
			}
			other = ast_parse_expr(LOOKUP_SYM_SECONDARY);
			if unlikely(!other)
				goto err_r_flags;
			if (tok == ':') {
				DREF struct ast *third;
do_range:
				/* range operator. */
				if unlikely(yield() < 0)
					goto err_2_flags;
				if (tok == ']') {
					third = ast_constexpr(Dee_None);
				} else {
					third = ast_parse_expr(LOOKUP_SYM_SECONDARY);
				}
				if unlikely(!third)
					goto err_2_flags;
				merge = ast_operator3(OPERATOR_GETRANGE, 0, result, other, third);
				ast_decref(third);
			} else {
				merge = ast_operator2(OPERATOR_GETITEM, 0, result, other);
			}
			ast_decref(other);
			ast_decref(result);
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if unlikely(!merge)
				goto err;
			result = merge;
			if (skip(']', W_EXPECTED_RBRACKET_AFTER_GETITEM))
				goto err_r;
			ast_setddi(result, &loc);
		}	break;

		case '{': /* Brace initializers. */
			loc_here(&loc);
			other = ast_parse_unaryhead(LOOKUP_SYM_SECONDARY);
			if unlikely(!other)
				goto err_r;
			/* Use the brace AST in a single-argument call to `result' */
			DREF struct ast **elemv;
			elemv = (DREF struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
			if unlikely(!elemv) {
err_other:
				ast_decref(other);
				goto err_r;
			}
			elemv[0] = other;
			merge    = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE, 1, elemv), &loc);
			if unlikely(!merge) {
				Dee_Free(elemv);
				goto err_other;
			}
			other = ast_setddi(ast_operator2(OPERATOR_CALL, 0, result, merge), &loc);
			ast_decref(merge);
			/* Override the result AST when a special type-initialization was performed. */
			ast_decref(result);
			result = other;
			break;

		case KWD_pack: {
			int temp;
			DREF struct ast *kw_labels;
			/* Call expression without parenthesis. */
			loc_here(&loc);
			if unlikely(yield() < 0)
				goto err_r;
			if (tok == '(')
				goto do_normal_call_with_loc;
			if unlikely(parser_warn_pack_used(&loc))
				goto err;
			temp = maybe_expression_begin();
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err_r;
				other = ast_setddi(ast_constexpr(Dee_EmptyTuple), &loc);
				kw_labels = NULL;
			} else {
				other = ast_parse_argument_list(AST_COMMA_FORCEMULTIPLE |
				                                AST_COMMA_STRICTCOMMA,
				                                &kw_labels);
			}
			if unlikely(!other)
				goto err_r;
			if (kw_labels) {
				merge = ast_action3(AST_FACTION_CALL_KW,
				                    result,
				                    other,
				                    kw_labels);
				ast_decref(kw_labels);
			} else {
				merge = ast_operator2(OPERATOR_CALL, 0, result, other);
			}
			merge = ast_setddi(merge, &loc);
			ast_decref(other);
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
		}	break;

		case '(': {
			DREF struct ast *kw_labels;
			/* Call expression. */
			loc_here(&loc);
do_normal_call_with_loc:
			old_flags = TPPLexer_Current->l_flags;
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err_r_flags;
			if (tok == ')') {
				other     = ast_setddi(ast_constexpr(Dee_EmptyTuple), &loc);
				kw_labels = NULL;
			} else {
				other = ast_parse_argument_list(AST_COMMA_FORCEMULTIPLE, &kw_labels);
			}
			if unlikely(!other)
				goto err_r_flags;
			if (kw_labels) {
				merge = ast_action3(AST_FACTION_CALL_KW,
				                    result,
				                    other,
				                    kw_labels);
				ast_decref(kw_labels);
			} else {
				merge = ast_operator2(OPERATOR_CALL, 0, result, other);
			}
			merge = ast_setddi(merge, &loc);
			ast_decref(other);
			ast_decref(result);
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if unlikely(!merge)
				goto err;
			result = merge;
			if (skip(')', W_EXPECTED_RPAREN_AFTER_CALL))
				goto err_r;
		}	break;

		case TOK_INC: {
			uint16_t opid;
			/* Inplace operators. */
			opid = OPERATOR_INC;
			goto do_inplace_op;
		case TOK_DEC:
			opid = OPERATOR_DEC;
do_inplace_op:
			merge = ast_sethere(ast_operator1(opid, AST_OPERATOR_FPOSTOP, result));
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if unlikely(yield() < 0)
				goto err_r;
		}	break;

		case TOK_DOTS: /* Expand expression. */
			merge = ast_sethere(ast_expand(result));
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if unlikely(yield() < 0)
				goto err_r;
			break;

		default:
			goto done;
		}
	} while (result);
done:
	return result;
err_2_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	goto err_2;
err_r_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	goto err_r;
err_2:
	ast_decref(other);
err_r:
	ast_decref(result);
err:
	return NULL;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_unary(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_unaryhead(lookup_mode);
	if likely(result)
		result = ast_parse_unary_operand(result);
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_prod_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = tok;
	ASSERT(TOKEN_IS_PROD(cmd));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		rhs = ast_parse_unary(LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_setddi(ast_operator2(cmd == TOK_POW
		                                 ? OPERATOR_POW
		                                 : GET_CHOP(cmd),
		                                 0, lhs, rhs),
		                   &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		cmd = tok;
		if (!TOKEN_IS_PROD(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_sum_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = tok;
	ASSERT(TOKEN_IS_SUM(cmd));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		if (tok == TOK_DOTS && cmd == '+') { /* sum */
			if unlikely(yield() < 0)
				goto err_r;
			merge = ast_action1(AST_FACTION_SUM, lhs);
		} else {
			rhs = ast_parse_prod(LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			merge = ast_operator2(GET_CHOP(cmd), 0, lhs, rhs);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			break;
		cmd = tok;
		if (!TOKEN_IS_SUM(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_shift_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = tok;
	ASSERT(TOKEN_IS_SHIFT(cmd));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		rhs = ast_parse_sum(LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_setddi(ast_operator2(cmd == TOK_SHL
		                                 ? OPERATOR_SHL
		                                 : OPERATOR_SHR,
		                                 0, lhs, rhs),
		                   &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		cmd = tok;
		if (!TOKEN_IS_SHIFT(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_cmp_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = tok;
	ASSERT(TOKEN_IS_CMP(cmd));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		if (tok == TOK_DOTS && (cmd == '<' || cmd == '>')) {
			if unlikely(yield() < 0)
				goto err_r;
			merge = ast_action1(cmd == '<'
			                    ? AST_FACTION_MIN
			                    : AST_FACTION_MAX,
			                    lhs);
		} else {
			rhs = ast_parse_shift(LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			merge = ast_operator2(cmd == TOK_LOWER_EQUAL
			                      ? OPERATOR_LE
			                      : cmd == TOK_GREATER_EQUAL
			                        ? OPERATOR_GE
			                        : GET_CHOP(cmd),
			                      0, lhs, rhs);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			break;
		cmd = tok;
		if (!TOKEN_IS_SHIFT(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_cmpeq_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = tok;
	ASSERT(TOKEN_IS_CMPEQ(cmd));
	for (;;) {
		bool invert;
		loc_here(&loc);
		invert = cmd == '!';
yield_again:
		if unlikely(yield() < 0)
			goto err_r;
		if (tok == '!') {
			invert ^= 1;
			goto yield_again;
		}
		if (cmd == '!') {
			if (tok == KWD_is || tok == KWD_in) {
				cmd = tok;
				if unlikely(yield() < 0)
					goto err_r;
			} else {
				if (WARN(W_EXPECTED_IS_OR_IN_AFTER_EXCLAIM))
					goto err_r;
				cmd = KWD_is;
			}
		}
#if 0 /* XXX: Ambiguity with unary not operator? */
		/* Allow the operator to be inverted afterwards, too. */
		while (tok == '!') {
			invert ^= 1;
			if unlikely(yield() < 0)
				goto err;
		}
#endif
		if (tok == KWD_bound && cmd == KWD_is) {
			/* Special cast: `foo is bound' --> `bound(foo)' */
			if unlikely(yield() < 0)
				goto err_r;
			merge = make_bound_expression(lhs, &loc);
		} else {
			rhs = ast_parse_cmp(LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			if (cmd == TOK_EQUAL || cmd == TOK_NOT_EQUAL) {
				merge = ast_operator2(cmd == TOK_EQUAL
				                      ? OPERATOR_EQ
				                      : OPERATOR_NE,
				                      0, lhs, rhs);
			} else {
				merge = ast_action2(cmd == KWD_is
				                    ? AST_FACTION_IS
				                    : cmd == TOK_EQUAL3
				                      ? AST_FACTION_SAMEOBJ
				                      : cmd == TOK_NOT_EQUAL3
				                        ? AST_FACTION_DIFFOBJ
				                        : AST_FACTION_IN,
				                    lhs, rhs);
			}
			ast_setddi(merge, &loc);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		/* Invert the result, if required. */
		if (invert) {
			merge = ast_setddi(ast_bool(AST_FBOOL_NEGATE, lhs), &loc);
			ast_decref(lhs);
			lhs = merge;
			if unlikely(!lhs)
				break;
		}
		cmd = tok;
		if (!TOKEN_IS_CMPEQ(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_and_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_AND(tok));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		rhs = ast_parse_cmpeq(LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_setddi(ast_operator2(OPERATOR_AND, 0, lhs, rhs), &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_AND(tok))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_xor_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_XOR(tok));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		rhs = ast_parse_and(LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_setddi(ast_operator2(OPERATOR_XOR, 0, lhs, rhs), &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_XOR(tok))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_or_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_OR(tok));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		rhs = ast_parse_xor(LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_setddi(ast_operator2(OPERATOR_OR, 0, lhs, rhs), &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_OR(tok))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_as_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_AS(tok));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		rhs = ast_parse_or(LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_setddi(ast_action2(AST_FACTION_AS, lhs, rhs), &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_AS(tok))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_land_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_LAND(tok));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		if (tok == TOK_DOTS) {
			if unlikely(yield() < 0)
				goto err_r;
			merge = ast_action1(AST_FACTION_ALL, lhs);
		} else {
			rhs = ast_parse_as(LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			merge = ast_land(lhs, rhs);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			goto err;
		if (!TOKEN_IS_LAND(tok))
			break;
	}
	if (tok == TOK_LOR &&
	    WARN(W_CONSIDER_PAREN_AROUND_LAND))
		goto err_r;
	return lhs;
err_r:
	ast_decref(lhs);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_lor_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_LOR(tok));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		if (tok == TOK_DOTS) {
			if unlikely(yield() < 0)
				goto err_r;
			merge = ast_action1(AST_FACTION_ANY, lhs);
		} else {
			rhs = ast_parse_lor(LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			if (TOKEN_IS_LAND(tok)) {
				/* Suggest parenthesis around logical-and. */
				if (WARN(W_CONSIDER_PAREN_AROUND_LAND))
					goto err_r_rhs;
				rhs = ast_parse_land_operand(rhs);
				if unlikely(!rhs)
					goto err_r;
			}

			merge = ast_lor(lhs, rhs);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			goto err;
		if (!TOKEN_IS_LOR(tok))
			break;
	}
	return lhs;
err_r_rhs:
	ast_decref(rhs);
err_r:
	ast_decref(lhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_cond_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *merge, *tt, *ff;
	struct ast_loc loc;
	/* >>  x ? y : z // >> x ? y : z
	 * >>  x ?: z    // >> x ? x : z
	 * >> (x ? y : ) // >> x ? y : x
	 * >> (x ? y)    // >> x ? y : none
	 */
	ASSERT(TOKEN_IS_COND(tok));
	for (;;) {
		uint16_t expect;
		expect = current_tags.at_expect;
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		if (tok == ':') {
			/* Missing true-branch. (Reuse the condition branch!) */
			tt = lhs;
			ast_incref(lhs);
		} else {
			tt = ast_parse_cond(LOOKUP_SYM_SECONDARY);
			if unlikely(!tt)
				goto err_r;
		}
		if (tok == ':') {
			/* Parse the false-branch. */
			if unlikely(yield() < 0)
				goto err_tt;
			if (tt != lhs) {
				int temp;
				temp = maybe_expression_begin();
				if (temp > 0)
					goto do_parse_ff_branch;
				if unlikely(temp < 0)
					goto err_tt;
				/* Missing false-branch. (Reuse the condition branch!)
				 * >> This is a new extension of deemon that completes semantics
				 *    by allowing the reverse of what `foo() ?: bar()' already does
				 *    by specifying the syntax `(foo() ? bar() :)' */
				ff = lhs;
				ast_incref(lhs);
			} else {
do_parse_ff_branch:
				ff = ast_parse_cond(LOOKUP_SYM_SECONDARY);
				if unlikely(!ff)
					goto err_tt;
			}
		} else {
			/* Missing false-branch will be evaluated to `none' */
			ff = NULL;
		}
		merge = ast_setddi(ast_conditional(AST_FCOND_EVAL | expect, lhs, tt, ff), &loc);
		ast_xdecref(ff);
		ast_decref(tt);
		ast_decref(lhs);
		lhs = merge;
		if (!TOKEN_IS_COND(tok))
			break;
	}
	return lhs;
err_tt:
	ast_decref(tt);
err_r:
	ast_decref(lhs);
	return NULL;
}



#define TOK_INPLACE_MIN       TOK_ADD_EQUAL
#define OPERATOR_INPLACE_MIN  OPERATOR_INPLACE_ADD

STATIC_ASSERT((TOK_ADD_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_ADD - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SUB_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SUB - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_MUL_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MUL - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_DIV_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_DIV - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_MOD_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MOD - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SHL_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHL - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SHR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_AND_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_AND - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_OR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_OR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_XOR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_XOR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_POW_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_POW - OPERATOR_INPLACE_MIN));
PRIVATE uint16_t const inplace_fops[] = {
	/* [TOK_ADD_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_ADD,
	/* [TOK_SUB_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SUB,
	/* [TOK_MUL_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_MUL,
	/* [TOK_DIV_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_DIV,
	/* [TOK_MOD_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_MOD,
	/* [TOK_SHL_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SHL,
	/* [TOK_SHR_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SHR,
	/* [TOK_AND_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_AND,
	/* [TOK_OR_EQUAL  - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_OR,
	/* [TOK_XOR_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_XOR,
	/* [TOK_POW_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_POW
};

INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_assign_operand(/*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = tok;
	ASSERT(TOKEN_IS_ASSIGN(cmd));
	for (;;) {
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err_r;
		rhs = ast_parse_cond(LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		if (cmd == TOK_COLON_EQUAL) {
			/* Special case: move-assign. */
			merge = ast_operator2(AST_SHOULD_MOVEASSIGN(rhs)
			                      ? OPERATOR_MOVEASSIGN
			                      : OPERATOR_ASSIGN,
			                      0, lhs, rhs);
		} else {
			/* Inplace operation. */
			merge = ast_operator2(inplace_fops[cmd - TOK_INPLACE_MIN], 0, lhs, rhs);
		}
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			break;
		cmd = tok;
		if (!TOKEN_IS_ASSIGN(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED DREF struct ast *FCALL
ast_parse_prod(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_unary(lookup_mode);
	if (likely(result) && TOKEN_IS_PROD(tok))
		result = ast_parse_prod_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_sum(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_prod(lookup_mode);
	if (likely(result) && TOKEN_IS_SUM(tok))
		result = ast_parse_sum_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_shift(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_sum(lookup_mode);
	if (likely(result) && TOKEN_IS_SHIFT(tok))
		result = ast_parse_shift_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_cmp(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_shift(lookup_mode);
	if (likely(result) && TOKEN_IS_CMP(tok))
		result = ast_parse_cmp_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_cmpeq(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_cmp(lookup_mode);
	if (likely(result) && TOKEN_IS_CMPEQ(tok))
		result = ast_parse_cmpeq_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_and(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_cmpeq(lookup_mode);
	if (likely(result) && TOKEN_IS_AND(tok))
		result = ast_parse_and_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_xor(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_and(lookup_mode);
	if (likely(result) && TOKEN_IS_XOR(tok))
		result = ast_parse_xor_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_or(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_xor(lookup_mode);
	if (likely(result) && TOKEN_IS_OR(tok))
		result = ast_parse_or_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_as(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_or(lookup_mode);
	if (likely(result) && TOKEN_IS_AS(tok))
		result = ast_parse_as_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_land(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_as(lookup_mode);
	if (likely(result) && TOKEN_IS_LAND(tok))
		result = ast_parse_land_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_lor(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_land(lookup_mode);
	if (likely(result) && TOKEN_IS_LOR(tok))
		result = ast_parse_lor_operand(result);
	return result;
}

INTERN WUNUSED DREF struct ast *FCALL
ast_parse_cond(unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_lor(lookup_mode);
	if (likely(result) && TOKEN_IS_COND(tok))
		result = ast_parse_cond_operand(result);
	return result;
}


INTERN WUNUSED DREF struct ast *FCALL
ast_parse_assign(unsigned int lookup_mode) {
#ifdef __OPTIMIZE_SIZE__
	DREF struct ast *result;
	result = ast_parse_cond(lookup_mode);
	if (likely(result) && TOKEN_IS_ASSIGN(tok))
		result = ast_parse_assign_operand(result);
	return result;
#elif 1
	DREF struct ast *result;
	result = ast_parse_unary(lookup_mode);
	if unlikely(!result)
		goto done;
	switch (tok) {
	CASE_TOKEN_IS_PROD:
		result = ast_parse_prod_operand(result);
		if unlikely(!result)
			goto done;
		if (TOKEN_IS_SUM(tok)) {
	CASE_TOKEN_IS_SUM:
			result = ast_parse_sum_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_SHIFT(tok)) {
	CASE_TOKEN_IS_SHIFT:
			result = ast_parse_shift_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_CMP(tok)) {
	CASE_TOKEN_IS_CMP:
			result = ast_parse_cmp_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_CMPEQ(tok)) {
	CASE_TOKEN_IS_CMPEQ:
			result = ast_parse_cmpeq_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_AND(tok)) {
	CASE_TOKEN_IS_AND:
			result = ast_parse_and_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_XOR(tok)) {
	CASE_TOKEN_IS_XOR:
			result = ast_parse_xor_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_OR(tok)) {
	CASE_TOKEN_IS_OR:
			result = ast_parse_or_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_AS(tok)) {
	CASE_TOKEN_IS_AS:
			result = ast_parse_as_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_LAND(tok)) {
	CASE_TOKEN_IS_LAND:
			result = ast_parse_land_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_LOR(tok)) {
	CASE_TOKEN_IS_LOR:
			result = ast_parse_lor_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_COND(tok)) {
	CASE_TOKEN_IS_COND:
			result = ast_parse_cond_operand(result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_ASSIGN(tok)) {
	CASE_TOKEN_IS_ASSIGN:
			result = ast_parse_assign_operand(result);
			if unlikely(!result)
				goto done;
		}
		break;
	default: break;
	}
done:
	return result;
#elif 1
	DREF struct ast *result;
	result = ast_parse_unary(lookup_mode);
	if unlikely(!result)
		goto done;
	/* parse binary operators */
	if (TOKEN_IS_PROD(tok)) {
		result = ast_parse_prod_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_SUM(tok)) {
		result = ast_parse_sum_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_SHIFT(tok)) {
		result = ast_parse_shift_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_CMP(tok)) {
		result = ast_parse_cmp_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_CMPEQ(tok)) {
		result = ast_parse_cmpeq_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_AND(tok)) {
		result = ast_parse_and_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_XOR(tok)) {
		result = ast_parse_xor_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_OR(tok)) {
		result = ast_parse_or_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_AS(tok)) {
		result = ast_parse_as_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_LAND(tok)) {
		result = ast_parse_land_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_LOR(tok)) {
		result = ast_parse_lor_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_COND(tok)) {
		result = ast_parse_cond_operand(result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_ASSIGN(tok)) {
		result = ast_parse_assign_operand(result);
		if unlikely(!result)
			goto done;
	}
done:
	return result;
#else
	DREF struct ast *result;
	result = ast_parse_cond(lookup_mode);
	if (likely(result) && TOKEN_IS_ASSIGN(tok))
		result = ast_parse_assign_operand(result);
	return result;
#endif
}



INTERN WUNUSED NONNULL((1)) DREF struct ast *FCALL
ast_parse_postexpr(/*inherit(always)*/ DREF struct ast *__restrict baseexpr) {
	baseexpr = ast_parse_unary_operand(baseexpr);
	if unlikely(!baseexpr)
		goto done;
	/* parse binary operators */
	if (TOKEN_IS_PROD(tok)) {
		baseexpr = ast_parse_prod_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_SUM(tok)) {
		baseexpr = ast_parse_sum_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_SHIFT(tok)) {
		baseexpr = ast_parse_shift_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_CMP(tok)) {
		baseexpr = ast_parse_cmp_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_CMPEQ(tok)) {
		baseexpr = ast_parse_cmpeq_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_AND(tok)) {
		baseexpr = ast_parse_and_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_XOR(tok)) {
		baseexpr = ast_parse_xor_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_OR(tok)) {
		baseexpr = ast_parse_or_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_AS(tok)) {
		baseexpr = ast_parse_as_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_LAND(tok)) {
		baseexpr = ast_parse_land_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_LOR(tok)) {
		baseexpr = ast_parse_lor_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_COND(tok)) {
		baseexpr = ast_parse_cond_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_ASSIGN(tok)) {
		baseexpr = ast_parse_assign_operand(baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
done:
	return baseexpr;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_EXPRESSION_C */
