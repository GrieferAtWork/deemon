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
#ifndef GUARD_DEEMON_COMPILER_LEXER_EXPRESSION_C
#define GUARD_DEEMON_COMPILER_LEXER_EXPRESSION_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* Dee_Free, Dee_Mallocc, Dee_Reallocc */
#include <deemon/bool.h>              /* Dee_False, Dee_True */
#include <deemon/compiler/ast.h>      /* AST_*, ast, ast_* */
#include <deemon/compiler/lexer.h>    /* AST_COMMA_*, AST_PARSE_WASEXPR_NO, CASE_TOKEN_IS_*, PARSE_UNARY_DISALLOW_CASTS, P_OPERATOR_FNORMAL, TOKEN_IS_*, ast_*, current_tags, parse_tags_block */
#include <deemon/compiler/optimize.h> /* ast_optimize_all */
#include <deemon/compiler/symbol.h>   /* LOOKUP_SYM_*, SYMBOL_TYPE_EXTERN, SYMBOL_TYPE_MYMOD, current_basescope, current_scope, decl_ast_skip, get_current_this, is_reserved_symbol_name, lookup_nth, lookup_symbol, new_unnamed_symbol, symbol */
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>             /* DeeError_Handled, ERROR_HANDLED_RESTORE */
#include <deemon/float.h>             /* DeeFloat_New */
#include <deemon/int.h>               /* DeeInt_FromString, DeeInt_NewInt64, Dee_INT_STRING* */
#include <deemon/module.h>            /* DeeModule* */
#include <deemon/none.h>              /* Dee_None */
#include <deemon/object.h>            /* DREF, DeeObject, DeeObject_AsUInt, Dee_AsObject, Dee_Decref, Dee_DecrefNokill, Dee_Incref, ITER_DONE */
#include <deemon/string.h>            /* DeeString_DecodeBackslashEscaped, DeeString_NewSized, DeeUni_IsLF, Dee_UNICODE_PRINTER_INIT, Dee_unicode_printer*, STRING_ERROR_FSTRICT */
#include <deemon/stringutils.h>       /* Dee_unicode_readutf8_n */
#include <deemon/system-features.h>   /* DeeSystem_DEFINE_memrchr, memchr, memmoveupc */
#include <deemon/tuple.h>             /* Dee_EmptyTuple */
#include <deemon/type.h>              /* Dee_operator_t, OPERATOR_*, TP_FFINAL, TP_FNORMAL */

#include "../../runtime/strings.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* int32_t, uint8_t, uint16_t, uint32_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr Dee_libc_memrchr
DeeSystem_DEFINE_memrchr(Dee_libc_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

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
	/* "\33" */ 0,
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
	/* "\177" */ 0,
/*[[[end]]]*/
};



/* Lookup mode used by secondary AST operands */
#define LOOKUP_SYM_SECONDARY  LOOKUP_SYM_NORMAL

/* Parser flags (Set of `PARSE_F*`) */
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
INTERN uint16_t parser_flags = PARSE_FNORMAL;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

/* Return 1 if the current token may be the begin of an expression.
 * @return: 1:  Yes
 * @return: 0:  No
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DFCALL
maybe_expression_begin(DeeLexer *self) {
	switch (DeeLexer_GetTok(self)) {
	case '+':
	case '-':
	case '~':
	case '(':
	case '#':
	case '<': /* For cells. (deprecated syntax) */
	case '[': /* For lists. */
	case '{': /* Brace initializers. */
	case TPP_TOK_PLUS_PLUS:
	case TPP_TOK_MINUS_MINUS:
	TPP_CASE_TPP_TOK_NUMBER
	TPP_CASE_TPP_TOK_STRING_SQUOTE
	TPP_CASE_TPP_TOK_STRING_DQUOTE
	case TPP_TOK_DOT_DOT_DOT:
	case TPP_TOK_COLON_COLON: /* Deprecated global-accessor syntax. */
		goto yes;

		/* Keywords that can only appear inside of expressions */
	case TPP_KWD_as:
	case TPP_KWD_in:
	case TPP_KWD_is:
		/* Keywords that can only appear inside of expressions/statements */
	case TPP_KWD_else:
	case TPP_KWD_catch:
	case TPP_KWD_finally:
		/* Keywords that can only appear inside of statements */
	case TPP_KWD_from:
	case TPP_KWD_return:
	case TPP_KWD_yield:
	case TPP_KWD_throw:
	case TPP_KWD_print:
	case TPP_KWD_break:
	case TPP_KWD_continue:
	case TPP_KWD___asm:
	case TPP_KWD___asm__:
	case TPP_KWD_goto:
	case TPP_KWD_switch:
	case TPP_KWD_case:
	case TPP_KWD_default:
		goto no;

	case '!': {
		struct TPPFile *tok_file;
		tpp_keyword *kwd;
		char const *tok_begin;
		/* Check if this ! is eventually followed by `is` or `in`
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
			if unlikely(TPPLexer_Current->l_token.t_id == TOK_ERR)
				goto err;
		} else {
			/* This isn't an expression. */
			if (tpp_keyword_getid(kwd) == TPP_KWD_is ||
			    tpp_keyword_getid(kwd) == TPP_KWD_in)
				goto no;
		}
		goto yes;
	}	break;

	default:
		/* Any kind of keyword can appear in expressions, either
		 * a native expression keyword, or as a variable name. */
		if (DeeLexer_HasTokenKwd(self))
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

/* Same as `maybe_expression_begin()`, but for the next (peeked) token. */
INTERN WUNUSED NONNULL((1)) int DFCALL
maybe_expression_begin_peek(DeeLexer *self) {
	char const *tok_begin;
	char peek;
	struct TPPFile *tok_file;
	(void)self;
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
					goto no; /* `<<` cannot appear at the start of expression */
				goto yes; /* `++` and `--` can appear, though */
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

	case '.': /* TPP_TOK_DOT_DOT_DOT */
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
		tpp_keyword *kwd;
		/* Check if this ! is eventually followed by `is` or `in`
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
			if unlikely(TPPLexer_Current->l_token.t_id == TOK_ERR)
				goto err;
		} else {
			/* This isn't an expression. */
			if (tpp_keyword_getid(kwd) == TPP_KWD_is ||
			    tpp_keyword_getid(kwd) == TPP_KWD_in)
				goto no;
		}
		goto yes;
	}	break;

	default: {
		tpp_keyword *kwd;
		if (!tpp_is_keyword_start(peek))
			goto no;
		kwd = peek_keyword(tok_file, tok_begin, 0);
		if unlikely(!kwd) {
			if unlikely(TPPLexer_Current->l_token.t_id == TOK_ERR)
				goto err;
			goto yes; /* First-time-used user-defined keyword token. */
		}
		switch (kwd->k_id) {

			/* Keywords that can only appear inside of expressions */
		case TPP_KWD_as:
		case TPP_KWD_in:
		case TPP_KWD_is:
			/* Keywords that can only appear inside of expressions/statements */
		case TPP_KWD_else:
		case TPP_KWD_catch:
		case TPP_KWD_finally:
			/* Keywords that can only appear inside of statements */
		case TPP_KWD_from:
		case TPP_KWD_return:
		case TPP_KWD_yield:
		case TPP_KWD_throw:
		case TPP_KWD_print:
		case TPP_KWD_break:
		case TPP_KWD_continue:
		case TPP_KWD___asm:
		case TPP_KWD___asm__:
		case TPP_KWD_goto:
		case TPP_KWD_switch:
		case TPP_KWD_case:
		case TPP_KWD_default:
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


PRIVATE WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
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
		/* Fallback-after-warning: Return `true` */
		result = ast_constexpr(Dee_True);
	}
	return ast_putddi(result, loc);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeString_DecodeLFEscaped(struct Dee_unicode_printer *__restrict printer,
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
		if (Dee_unicode_printer_printutf8(printer, flush_start,
		                                  (size_t)(candidate - flush_start)) < 0)
			goto err;
		flush_start = candidate;
		++candidate;
		start = (char *)candidate;
		ASSERT(start <= end);
		if (start < end) {
			ch = Dee_unicode_readutf8_n(&candidate, end);
			if (DeeUni_IsLF(ch)) {
				if (ch == '\r' && candidate < end && *candidate == '\n')
					++candidate; /* CRLF */
				start = flush_start = candidate;
			}
		}
	}
	if (Dee_unicode_printer_printutf8(printer, flush_start,
	                                  (size_t)(end - flush_start)) < 0)
		goto err;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DFCALL
ast_decode_unicode_string(DeeLexer *self, struct Dee_unicode_printer *__restrict printer) {
	ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));
	char const *escape_start = (char const *)DeeLexer_GetTokenStart(self);
	char const *escape_end   = (char const *)DeeLexer_GetTokenEnd(self);
	(void)self;
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
ast_parse_string(DeeLexer *self) {
	struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
	ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));
	do {
		if unlikely(ast_decode_unicode_string(self, &printer))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
	} while (DeeLexer_IsStringToken(self));
	return Dee_unicode_printer_pack(&printer);
err:
	Dee_unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED DREF struct ast *DFCALL
ast_sym___import___from_deemon(void) {
	struct symbol *import_symbol;
	import_symbol = new_unnamed_symbol();
	if unlikely(!import_symbol)
		goto err;
	/* Setup an external symbol pointing at `import from deemon` */
	import_symbol->s_type            = SYMBOL_TYPE_EXTERN;
	import_symbol->s_extern.e_module = DeeModule_GetDeemon();
	Dee_Incref(import_symbol->s_extern.e_module);
	import_symbol->s_extern.e_symbol = DeeModule_GetSymbol(import_symbol->s_extern.e_module,
	                                                       Dee_AsObject(&str___import__));
	ASSERT(import_symbol->s_extern.e_symbol);
	return ast_sym(import_symbol);
err:
	return NULL;
}

/* Inject a reference to "this_module" at the start of the argument list. */
PRIVATE WUNUSED NONNULL((1)) int DFCALL
ast_multiple_tuple_inject_this_module(struct ast *__restrict self) {
	DREF struct ast **new_astv;
	DREF struct ast *mymod_ast;
	struct symbol *sym;
	ASSERT(self->a_type == AST_MULTIPLE);
	ASSERT(self->a_flag == AST_FMULTIPLE_TUPLE);
	sym = new_unnamed_symbol();
	if unlikely(!sym)
		goto err;
	sym->s_type = SYMBOL_TYPE_MYMOD;
	mymod_ast = ast_sym(sym);
	if unlikely(!mymod_ast)
		goto err;
	new_astv = (DREF struct ast **)Dee_Reallocc(self->a_multiple.m_astv,
	                                            self->a_multiple.m_astc + 1,
	                                            sizeof(DREF struct ast *));
	if unlikely(!new_astv)
		goto err_mymod_ast;
	/* Inject the reference to "mymod_ast" */
	memmoveupc(new_astv + 1, new_astv,
	           self->a_multiple.m_astc,
	           sizeof(DREF struct ast *));
	new_astv[0] = mymod_ast; /* Inherit reference */
	self->a_multiple.m_astv = new_astv;
	++self->a_multiple.m_astc;
	return 0;
err_mymod_ast:
	ast_decref(mymod_ast);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_import_expression_after_import(DeeLexer *self, struct ast_loc *__restrict import_loc) {
	bool has_paren;
	struct ast_loc loc;
	DREF struct ast *result, *kw_labels, *other, *merge;
	result = ast_setddi(ast_sym___import___from_deemon(), import_loc);
	if unlikely(!result)
		goto err;
	DeeLexer_NoLf_Push(self);
	if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_IMPORT)) {
err_r_flags:
		DeeLexer_NoLf_Break(self);
		goto err_r;
	}
	if (DeeLexer_GetLoc(self, &loc))
		goto err_r_flags;
	other = ast_parse_argument_list(self, AST_COMMA_FORCEMULTIPLE, &kw_labels);
	if unlikely(!other)
		goto err_r_flags;
	/* Inject a hidden argument "this_module" at the start of "other". */
	if unlikely(ast_multiple_tuple_inject_this_module(other)) {
		ast_decref(other);
		goto err_r_flags;
	}
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
	DeeLexer_NoLf_Pop(self);
	if unlikely(!merge)
		goto err;
	result = merge;
	if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_IMPORT))
		goto err_r;
	return result;
err_r:
	ast_decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_import_expression(DeeLexer *self) {
	struct ast_loc import_loc;
	if (DeeLexer_GetLoc(self, &import_loc))
		goto err;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;
	return ast_parse_import_expression_after_import(self, &import_loc);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DFCALL
count_chars_without_escaped_linefeeds(char const *start,
                                      char const *end) {
	size_t result = 0;
	while (start < end) {
		uint32_t ch;
		ch = Dee_unicode_readutf8_n(&start, end);
		if (ch == '\\') {
			ch = Dee_unicode_readutf8_n(&start, end);
			if (ch == '\r')
				Dee_unicode_readutf8_n(&start, end);
			continue;
		}
		++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DFCALL
verify_consistent_grouping(DeeLexer *self) {
	char const *tok_start, *tok_end, *iter, *lastsep;
	size_t interval, wanted_interval;
	tok_start = (char const *)DeeLexer_GetTokenStart(self);
	tok_end   = (char const *)DeeLexer_GetTokenEnd(self);
	iter      = (char const *)memrchr(tok_start, '_', (size_t)(tok_end - tok_start));
	if likely(!iter)
		return 0; /* No separators present. */
	ASSERTF(iter > tok_start, "If the '_' was at the start, it'd be a keyword token");

	/* Figure out the  */
	interval = count_chars_without_escaped_linefeeds(iter + 1, tok_end);
	lastsep  = iter;
	while (iter > tok_start) {
		size_t offset;
		--iter;
		if (*iter != '_')
			continue;
		offset = count_chars_without_escaped_linefeeds(iter + 1, lastsep);
		if (offset != interval) {
			return WARN(W_INCONSISTENT_THOUSANDS_SEPERATORS,
			            interval, offset);
		}
		lastsep = iter;
	}

	/* Decimals should use a thousands-interval of `3`.
	 * For every other radix, the interval should be `4`. */
	wanted_interval = *tok_start == '0' ? 4 : 3;
	if (wanted_interval != interval) {
		return WARN(W_INCORRECT_THOUSANDS_SEPERATORS,
		            wanted_interval, interval);
	}

	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_unaryhead(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	DREF struct ast *merge;
	struct ast_loc loc;
	switch (DeeLexer_GetTok(self)) {

	TPP_CASE_TPP_TOK_INT {
		tint_t value;
		DREF DeeObject *resval;
		size_t toklen;

		/* Verify that thousands-separators (if present) are used consistently. */
		if unlikely(verify_consistent_grouping(self))
			goto err;

		/* Use our own integer parser, so we can process arbitrary-precision integers. */
		toklen = DeeLexer_GetTokenLen(self);
		resval = DeeInt_FromString((char const *)DeeLexer_GetTokenStart(self), toklen,
		                           Dee_INT_STRING(0, Dee_INT_STRING_FESCAPED |
		                                            Dee_INT_STRING_FTRY));

		/* Check if the integer failed to be parsed. */
		if unlikely(resval == ITER_DONE) {
			if (WARN(W_INVALID_INTEGER))
				goto err;
			goto create_none;
		}
create_constexpr:
		if unlikely(!resval)
			goto err;
		result = ast_sethere(self, ast_constexpr(resval));
		Dee_Decref(resval);
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		return result;

	TPP_CASE_TPP_TOK_STRING_SQUOTE
		if (!DeeLexer_Has(self, CHARACTER_LITERALS))
			goto decode_string;
		if unlikely(TPP_Atoi(&value) == TPP_ATOI_ERR)
			goto err;
		if (WARN(W_DEPRECATED_CHARACTER_INT))
			goto err;
		resval = DeeInt_NewInt64(value);
		goto create_constexpr;
	}	break;

	TPP_CASE_TPP_TOK_STRING_DQUOTE {
		DREF DeeObject *resval;
decode_string:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		resval = ast_parse_string(self);
		if unlikely(!resval)
			goto err;
		result = ast_setddi(ast_constexpr(resval), &loc);
		Dee_Decref(resval);
		return result;
	}	break;

	case TPP_KWD_f:
	case TPP_KWD_F:
		/* Check if this might be a template string. */
		if ((*TPPLexer_Current->l_token.t_end == '\"') ||
		    (*TPPLexer_Current->l_token.t_end == '\'' && !DeeLexer_Has(self, CHARACTER_LITERALS))) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));
			return ast_parse_template_string(self);
		}
		goto do_keyword;

	TPP_CASE_TPP_TOK_FLOAT {
		tfloat_t value;
		DREF DeeObject *resval;
		if (TPP_Atof(&value) == TPP_ATOF_ERR)
			goto err;
		resval = DeeFloat_New((double)value);
		if unlikely(!resval)
			goto err;
		/* Construct a new branch for the constant float value. */
		result = ast_sethere(self, ast_constexpr(resval));
		Dee_Decref(resval);
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		return result;
	}	break;

	case TPP_KWD_none: {
		DeeObject *constval;
create_none:
		constval = Dee_None;
mkconst:
		result = ast_sethere(self, ast_constexpr(constval));
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		return result;
	case TPP_KWD_true:
		constval = Dee_True;
		goto mkconst;
	case TPP_KWD_false:
		constval = Dee_False;
		goto mkconst;
	}	break;

	case TPP_KWD_bound: {
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetTok(self) == '(') {
			result = ast_parse_unaryhead(self,
			                             LOOKUP_SYM_SECONDARY |
			                             PARSE_UNARY_DISALLOW_CASTS);
		} else {
			result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
		}
		if unlikely(!result)
			goto err;
		merge = make_bound_expression(result, &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		return merge;
	}	break;


	case TPP_KWD_str: {
		Dee_operator_t opid; /* Unary expressions. */
		opid = OPERATOR_STR;
		goto do_unary_operator_kwd;
	case TPP_KWD_repr:
		opid = OPERATOR_REPR;
		goto do_unary_operator_kwd;
	case TPP_KWD_deepcopy:
		opid = AST_FACTION_DEEPCOPY;
		goto do_unary_action_kwd;
	case TPP_KWD_copy:
		opid = OPERATOR_COPY;
do_unary_operator_kwd:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetTok(self) == '(') {
			result = ast_parse_unaryhead(self,
			                             LOOKUP_SYM_SECONDARY |
			                             PARSE_UNARY_DISALLOW_CASTS);
		} else {
			result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
		}
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_operator1(opid, 0, result), &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		return merge;

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
	case TPP_TOK_PLUS_PLUS:
		opid = OPERATOR_INC;
		goto do_unary_operator;
	case TPP_TOK_MINUS_MINUS:
		opid = OPERATOR_DEC;
do_unary_operator:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_operator1(opid, 0, result), &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		return merge;

	case TPP_KWD_type:
		opid = AST_FACTION_TYPEOF;
do_unary_action_kwd:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetTok(self) == '(') {
			result = ast_parse_unaryhead(self,
			                             LOOKUP_SYM_SECONDARY |
			                             PARSE_UNARY_DISALLOW_CASTS);
		} else {
			result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
		}
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_action1(opid, result), &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		return merge;
	}	break;

	case '!': /* not */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_bool(AST_FBOOL_NEGATE, result), &loc);
		ast_decref(result);
		return merge;

		/* I admit it. I was an idiot for adding this dedicated syntax for constructing
		 * Cell objects. I wasn't thinking and as a result of that, I have to maintain
		 * it for backwards compatibility.
		 * Anyways... The most I can do for now is have it emit a warning, telling that
		 * you should be using `Cell from deemon` instead (which actually won't even
		 * break backwards-compatibility with the old deemon, who's `Cell` object
		 * offered you the same functionality)
		 * NOTE: To ensure backwards-compatibility, you may place this
		 *       in your code in order to simply always use `Cell(...)`:
		 * >> #if __DEEMON__ >= 200
		 * >> import Cell from deemon;
		 * >> #else
		 * >> #define Cell(...) < __VA_ARGS__ >
		 * >> #endif */
	case TPP_TOK_LANGLE_RANGLE:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (WARN(W_DEPRECATED_CELL_SYNTAX))
			goto err;
		goto do_empty_cell;

	case '<': /* Cell (deprecated syntax) */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (WARN(W_DEPRECATED_CELL_SYNTAX))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetTok(self) == '>') {
			/* empty Cell. */
do_empty_cell:
			result = ast_action0(AST_FACTION_CELL0);
		} else {
			result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!result)
				goto err;
			if (DeeLexer_GetTok(self) != '>' && WARN(W_EXPECTED_RANGLE_AFTER_LANGLE))
				goto err_r;
			merge = ast_action1(AST_FACTION_CELL1, result);
			ast_decref(result);
			result = merge;
		}
		ast_setddi(result, &loc);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		return result;

	case TPP_KWD_if: {
		/* if-in-expressions. */
		DREF struct ast *tt_branch;
		DREF struct ast *ff_branch;
		uint16_t expect;
		bool has_paren;
		expect = current_tags.at_expect;
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_if_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_IF))
			goto err_if_flags;
		result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_IF))
			goto err;
		tt_branch = NULL;
		if (DeeLexer_GetTok(self) != TPP_KWD_else && DeeLexer_GetTok(self) != TPP_KWD_elif) {
			tt_branch = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!tt_branch)
				goto err_r;
		}
		ff_branch = NULL;
		if (DeeLexer_GetTok(self) == TPP_KWD_elif) {
			DeeLexer_SetTokenId(self, TPP_KWD_if); /* Cheat a bit... */
			goto do_else_branch;
		}
		if (DeeLexer_GetTok(self) == TPP_KWD_else) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_tt:
				ast_xdecref(tt_branch);
				goto err_r;
			}
do_else_branch:
			ff_branch = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!ff_branch)
				goto err_tt;
		}
		merge = ast_setddi(ast_conditional(AST_FCOND_EVAL | expect, result, tt_branch, ff_branch), &loc);
		ast_xdecref(ff_branch);
		ast_xdecref(tt_branch);
		ast_xdecref(result);
		return merge;
	}	break;

	case TPP_KWD_assert:
		return ast_parse_assert(self, true);

	case TPP_KWD_function: {
		tpp_keyword *function_name;
		if (WARN(W_DEPRECATED_FUNCTION_IN_EXPRESSION))
			goto err;
		/* Create a new function */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		function_name = NULL;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_HasTokenKwd(self)) {
			function_name = DeeLexer_GetTokenKwd(self);
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
		}
		return ast_parse_function(self, function_name, NULL, false, &loc, NULL, NULL);
	}	break;

	case TPP_KWD_final: {
		tpp_keyword *class_name;
		uint16_t class_flags;
		class_flags = TP_FFINAL;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (unlikely(DeeLexer_GetTok(self) != TPP_KWD_class) &&
		    WARN(W_EXPECTED_CLASS_AFTER_FINAL))
			goto err;
		goto do_create_class;
	case TPP_KWD_class:
		class_flags = TP_FNORMAL;
do_create_class:
		/* Create a new function */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		class_name = NULL;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_HasTokenKwd(self)) {
			if (DeeLexer_GetTok(self) == TPP_KWD_final && !(class_flags & TP_FFINAL)) {
				/* allow `class final` as an alias for `final class` */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				class_flags |= TP_FFINAL;
			}
			if (DeeLexer_HasTokenKwd(self)) {
				class_name = DeeLexer_GetTokenKwd(self);
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
			}
		}
		/* Actually do parse the class now. */
		result = ast_parse_class(self, class_flags, class_name,
		                         false, LOOKUP_SYM_NORMAL);
		return ast_setddi(result, &loc);
	}	break;

	case TPP_KWD_pack: {
		int has_paren;
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		has_paren = 0;
		DeeLexer_NoLf_Push(self);
		if (DeeLexer_GetTok(self) == '(') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_pack_flags:
				DeeLexer_NoLf_Break(self);
				goto err;
			}
			has_paren = DeeLexer_GetTok(self) == '(' ? 2 : 1;
		} else {
			if unlikely(parser_warn_pack_used(&loc))
				goto err_pack_flags;
		}
		if (DeeLexer_GetTok(self) == '{') {
			/* Statements in expressions. */
			result = ast_parse_statement_or_braces(self, NULL);
		} else {
			int temp;
			temp = maybe_expression_begin(self);
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err_pack_flags;
				result = ast_constexpr(Dee_EmptyTuple);
			} else {
				/* Parse the packed expression. */
				result = ast_parse_comma(self,
				                         has_paren
				                         ? AST_COMMA_FORCEMULTIPLE
				                         : AST_COMMA_FORCEMULTIPLE | AST_COMMA_STRICTCOMMA,
				                         AST_FMULTIPLE_TUPLE,
				                         NULL);
#if 0 /* Because of the `AST_COMMA_FORCEMULTIPLE`, this is unnecessary */
				if likely(result && result->a_type == AST_EXPAND) {
					/* Wrap into a single-item tuple multiple-branch:
					 * >> print pack get_items()...; // Convert to tuple. */
					DREF struct ast **exprv;
					ast_setddi(result, &loc);
					exprv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
					if unlikely(!exprv) {
err_pack_flags_r:
						ast_decref(result);
						goto err_pack_flags;
					}
					exprv[0] = result; /* Inherit */
					merge    = ast_multiple(AST_FMULTIPLE_TUPLE, 1, exprv);
					if unlikely(!merge) {
						Dee_Free(exprv);
						goto err_pack_flags_r;
					}
					result = merge; /* Inherit */
				}
#endif
			}
		}
		ast_setddi(result, &loc);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (has_paren) {
			if (DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN_AFTER_PACK))
				goto err_r;
#if 0 /* The `result->a_type != AST_MULTIPLE` would never \
       * fly, because of the `AST_COMMA_FORCEMULTIPLE` */
			if (has_paren == 1 && result->a_type != AST_MULTIPLE &&
			    !(lookup_mode & PARSE_UNARY_DISALLOW_CASTS)) {
				/* C-style cast expression (only for single-parenthesis expressions) */
				merge = ast_parse_cast(self, result);
				ast_decref(result);
				result = merge;
			}
#endif
		}
		return result;
	}	break;

	case TPP_KWD_import:
		return ast_parse_import_expression(self);

	case TPP_KWD_do:
	case TPP_KWD_while:
	case TPP_KWD_for:
	case TPP_KWD_foreach:
		/* Loop expressions. */
		return ast_parse_loopexpr(self);

	case '(': {
		bool allow_cast;
		unsigned int was_expression;

		/* Parenthesis. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_lparen_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		allow_cast = DeeLexer_GetTok(self) != '(' && !(lookup_mode & PARSE_UNARY_DISALLOW_CASTS);
		if (DeeLexer_GetTok(self) != '{') {
			if (DeeLexer_GetTok(self) == ')') {
				DeeLexer_NoLf_Break(self);
				if (DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN_AFTER_LPAREN))
					goto err;

				/* Support for java-style lambda with empty argument list. */
				if (DeeLexer_GetTok(self) == TOK_ARROW) {
					result = ast_parse_function_java_lambda(self, NULL, NULL);
					result = ast_setddi(result, &loc);
					return result;
				}
				if (DeeLexer_GetTok(self) == ':') {
					bool isarrow;
					struct TPPLexerPosition pos;
					if unlikely(!TPPLexer_SavePosition(&pos))
						goto err;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_restore_pos:
						TPPLexer_LoadPosition(&pos);
						goto err;
					}
					if unlikely(decl_ast_skip(self))
						goto err_restore_pos;
					isarrow = DeeLexer_GetTok(self) == TOK_ARROW;
					TPPLexer_LoadPosition(&pos);
					if (isarrow) {
						result = ast_parse_function_java_lambda(self, NULL, NULL);
						result = ast_setddi(result, &loc);
						return result;
					}
				}

				/* Empty tuple. */
				result = ast_constexpr(Dee_EmptyTuple);
				if unlikely(!result)
					goto err;
				allow_cast = false; /* Don't allow empty tuples for cast expressions. */
			} else {
				/* Lambda function. */
				int error = ast_is_after_lparen_of_java_lambda(self);
				if (error != 0) {
					DeeLexer_NoLf_Break(self);
					if unlikely(error < 0)
						goto err;
					result = ast_parse_function_java_lambda(self, NULL, NULL);
					result = ast_setddi(result, &loc);
					return result;
				}

				/* Parenthesis / tuple expression. */
				result = ast_parse_comma(self,
				                         AST_COMMA_NORMAL,
				                         AST_FMULTIPLE_TUPLE,
				                         NULL);
				if unlikely(!result)
					goto err_lparen_flags;
				if (result->a_type == AST_EXPAND) {
					/* Wrap into a single-item tuple multiple-branch:
					 * >> print (get_items()...); // Convert to tuple. */
					DREF struct ast **exprv;
					ast_setddi(result, &loc);
					exprv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
					if unlikely(!exprv) {
err_lparen_flags_r:
						ast_decref(result);
						goto err_lparen_flags;
					}
					exprv[0] = result; /* Inherit */
					merge = ast_multiple(AST_FMULTIPLE_TUPLE, 1, exprv);
					if unlikely(!merge) {
/*err_lparen_flags_r_exprv:*/
						Dee_Free(exprv);
						goto err_lparen_flags_r;
					}
					result = merge; /* Inherit */
				}
				if (result->a_type == AST_MULTIPLE)
					allow_cast = false; /* Don't allow comma-lists for cast expressions. */
				DeeLexer_NoLf_Break(self);
				if (DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN_AFTER_LPAREN))
					goto err_r;
			}
			if (DeeLexer_GetTok(self) == '{' && !allow_cast && WARN(W_PROBABLY_MISSING_ARROW))
				goto err_r;
			goto set_lparen_ddi;
		}

		/* Statements in expressions. */
		result = ast_parse_statement_or_braces(self, &was_expression);
		if unlikely(!result)
			goto err_lparen_flags;
		allow_cast = false; /* Don't allow braces, or statements as cast expressions. */
		if (DeeLexer_GetTok(self) == ',' && was_expression != AST_PARSE_WASEXPR_NO) {
			DREF struct ast **tuple_branchv;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_lparen_flags_r;
			if (DeeLexer_GetTok(self) == ')') {
				/* single-element tuple expression, where the single element is a sequence. */
				tuple_branchv = (struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
				if unlikely(!tuple_branchv)
					goto err_lparen_flags_r;
				tuple_branchv[0] = result; /* Inherit reference. */
				merge            = ast_multiple(AST_FMULTIPLE_TUPLE, 1, tuple_branchv);
				if unlikely(!merge) {
					Dee_Free(tuple_branchv);
					goto err_lparen_flags_r;
				}
				result = merge;
			} else {
				/* There are more items! */
				merge = ast_parse_comma(self,
				                        AST_COMMA_FORCEMULTIPLE,
				                        AST_FMULTIPLE_TUPLE,
				                        NULL);
				if unlikely(!merge)
					goto err_lparen_flags_r;
				ASSERT(merge->a_type == AST_MULTIPLE ||
				       (merge->a_type == AST_CONSTEXPR &&
				        merge->a_constexpr == Dee_EmptyTuple));
				if (merge->a_constexpr == Dee_EmptyTuple) {
					tuple_branchv = (struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
					if unlikely(!tuple_branchv) {
err_lparen_flags_r_merge:
						ast_decref(merge);
						goto err_lparen_flags_r;
					}
					Dee_DecrefNokill(merge->a_constexpr);
					merge->a_type            = AST_MULTIPLE;
					merge->a_flag            = AST_FMULTIPLE_TUPLE;
					merge->a_multiple.m_astc = 1;
				} else {
					tuple_branchv = (struct ast **)Dee_Reallocc(merge->a_multiple.m_astv,
					                                            merge->a_multiple.m_astc + 1,
					                                            sizeof(DREF struct ast *));
					if unlikely(!tuple_branchv)
						goto err_lparen_flags_r_merge;
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
		if (DeeLexer_GetTok(self) != ')' && was_expression != AST_PARSE_WASEXPR_NO) {
			result = ast_parse_postexpr(self, result);
			if unlikely(!result)
				goto err_lparen_flags;
			/*was_expression = AST_PARSE_WASEXPR_YES;*/
		}
		DeeLexer_NoLf_Pop(self);
		if (DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN_AFTER_LPAREN))
			goto err_r;

set_lparen_ddi:
		result = ast_putddi(result, &loc);
		if (allow_cast && result->a_type != AST_MULTIPLE) {
			/* C-style cast expression (only for single-parenthesis expressions) */
			merge = ast_parse_cast(self, result);
			ast_decref(result);
			result = merge;
		}
		return result;
	}	break;

	case '{':
		/* Brace-style sequence or mapping expression. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
/*err_lbrace_flags:*/
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		result = ast_setddi(ast_parse_brace_items(self), &loc);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeLexer_Skip2(self, '}', W_EXPECTED_RBRACE_AFTER_BRACEINIT))
			goto err_r;
		return result;

	case TPP_KWD_del: {
		bool has_paren;

		/* Delete expression. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_del_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_DEL))
			goto err_del_flags;
		result = ast_putddi(ast_parse_del(self, lookup_mode & ~PARSE_UNARY_DISALLOW_CASTS), &loc);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_DEL))
			goto err_r;
		return result;
	}	break;

	case TPP_KWD_with:
		return ast_parse_with(self, false, false);

	case TPP_KWD_try:
		return ast_parse_try(self, false);

	case TPP_KWD_operator: {
		int32_t name;
		/* Named, but unbound operator invocation. */
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		name = ast_parse_operator_name(self, P_OPERATOR_FNORMAL);
		if unlikely(name < 0)
			goto err;
		if (DeeLexer_GetTok(self) != '(') {
			/* Bound-operator expression. */
			merge = ast_operator_func((Dee_operator_t)name, NULL);
		} else {
			DeeLexer_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
/*err_operator_flags:*/
				DeeLexer_NoLf_Break(self);
				goto err;
			}
			if (DeeLexer_GetTok(self) == ')') {
				result = ast_constexpr(Dee_EmptyTuple);
			} else {
				result = ast_parse_comma(self,
				                        AST_COMMA_FORCEMULTIPLE,
				                        AST_FMULTIPLE_TUPLE,
				                        NULL);
			}
			DeeLexer_NoLf_Pop(self);
			if unlikely(!result)
				goto err;
			if (DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN))
				goto err_r;
			merge = ast_build_operator((Dee_operator_t)name,
			                           /* Set the MAYBEPFX flag to suppress errors that
			                            * would normally cause the assembler to fail when
			                            * attempting to use an inplace operator on a
			                            * non-inplace symbol. */
			                           AST_OPERATOR_FMAYBEPFX,
			                           result);
			ast_decref(result);
		}
		if unlikely(!merge)
			goto err;
		return ast_setddi(merge, &loc);
	}	break;

	case '[': /* List */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_lbracket_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_GetTok(self) == '&' || DeeLexer_GetTok(self) == '=') {
			if (WARN(W_DEPRECATED_LAMBDA_MODE))
				goto err_lbracket_flags;
			if (DeeLexer_GetLoc(self, &loc))
				goto err_lbracket_flags;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_lbracket_flags;
			DeeLexer_NoLf_Break(self);
			if (DeeLexer_Skip2(self, ']', W_EXPECTED_RBRACKET_AFTER_LAMBDA))
				goto err;
			goto do_lambda;
		}
		if (DeeLexer_GetTok(self) == ']') {
			DeeLexer_NoLf_Break(self);
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (DeeLexer_GetTok(self) == ':') {
				/* If the current token is ':', try to skip over the return type
				 * annotation and check if the next token thereafter is '->' or '{'. */
				tok_t token_after;
				struct TPPLexerPosition saved;
				if unlikely(!TPPLexer_SavePosition(&saved))
					goto err;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_restore_pos_in_old_lambda:
					TPPLexer_LoadPosition(&saved);
					goto err;
				}
				if unlikely(decl_ast_skip(self))
					goto err_restore_pos_in_old_lambda;
				token_after = DeeLexer_GetTok(self);
				TPPLexer_LoadPosition(&saved);
				if (token_after == '{' || token_after == TOK_ARROW)
					goto do_lambda; /* Yup: it's a lambda! */
			}
			if (DeeLexer_GetTok(self) == '(' ||
			    DeeLexer_GetTok(self) == '{' ||
			    DeeLexer_GetTok(self) == TOK_ARROW)
				goto do_lambda;
			result = ast_multiple(AST_FMULTIPLE_LIST, 0, NULL);
			goto set_list_loc;
		}
		if (DeeLexer_GetTok(self) == ':') {
			DREF struct ast *start_expression;
			DREF struct ast *step_expression;
			result = ast_constexpr(Dee_None);
			if unlikely(!result)
				goto err_lbracket_flags;
			/* Range without begin index. */
do_range_expression:
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_lbracket_flags_r:
				ast_decref(result);
				goto err_lbracket_flags;
			}
			start_expression = result; /* Inherit reference. */
			/* Parse the end index. */
			if (DeeLexer_GetTok(self) == ',') {
				/* No end index given. */
				result = ast_constexpr(Dee_None);
			} else {
				result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			}
			if unlikely(!result) {
				ast_decref(start_expression);
				goto err_lbracket_flags;
			}
			if (DeeLexer_GetTok(self) == ',') {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_lbracket_flags_r_start:
					ast_decref(start_expression);
					goto err_lbracket_flags_r;
				}
				step_expression = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			} else {
				step_expression = ast_constexpr(Dee_None);
			}
			if unlikely(!step_expression)
				goto err_lbracket_flags_r_start;
			/* Create the range expression. */
			merge = ast_action3(AST_FACTION_RANGE, start_expression, result, step_expression);
			ast_decref(start_expression);
			ast_decref(result);
			ast_decref(step_expression);
			if unlikely(!merge)
				goto err_lbracket_flags;
			result = merge;
		} else {
			result = ast_parse_comma(self,
			                         AST_COMMA_FORCEMULTIPLE,
			                         AST_FMULTIPLE_LIST,
			                         NULL);
			if unlikely(!result)
				goto err_lbracket_flags;
			if (DeeLexer_GetTok(self) == ':' &&
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
		DeeLexer_NoLf_Pop(self);
		if (DeeLexer_Skip2(self, ']', W_EXPECTED_RBRACKET_AFTER_LIST))
			goto err_r;
set_list_loc:
		return ast_setddi(result, &loc);
do_lambda:
		DeeLexer_NoLf_Push(self);
		if unlikely(parse_tags_block(self)) {
/*err_lambda_flags:*/
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		result = ast_parse_function(self, NULL, NULL, true, &loc, NULL, NULL);
		DeeLexer_NoLf_Pop(self);
		return result;

	case TPP_KWD_this: {
		struct symbol *this_sym;
		this_sym = get_current_this();
		if (!this_sym)
			goto default_case;
		result = ast_sethere(self, ast_sym(this_sym));
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		return result;
	}	break;

	case TPP_KWD_super: {
		struct symbol *this_sym;
		DREF struct ast *this_ast;
		if (!current_scope->s_class ||
		    !current_scope->s_class->cs_super)
			goto default_case;
		this_sym = get_current_this();
		if (!this_sym)
			goto default_case;
		this_ast = ast_sethere(self, ast_sym(this_sym));
		if unlikely(!this_ast)
			goto err;
		merge = ast_sethere(self, ast_sym(current_scope->s_class->cs_super));
		if unlikely(!merge) {
			ast_decref(this_ast);
			goto err;
		}
		result = ast_sethere(self, ast_action2(AST_FACTION_AS, this_ast, merge));
		ast_decref(merge);
		ast_decref(this_ast);
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		return result;
	}	break;

	case TPP_KWD___nth: {
		bool has_paren;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_nth_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_NTH))
			goto err_nth_flags;
		result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		/* Optimize the ast-expression to propagate constant, thus
		 * allowing the use of `__nth(2+3)` instead of forcing the
		 * user to write `__nth(5)` or `__nth(__TPP_EVAL(2+3))` */
		if (ast_optimize_all(result, true))
			goto err_r;
		if (result->a_type != AST_CONSTEXPR &&
		    WARN(W_EXPECTED_CONSTANT_AFTER_NTH))
			goto err_r;
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_NTH))
			goto err_r;
		if (DeeLexer_HasTokenKwd(self)) {
			unsigned int nth_symbol = 0;
			struct symbol *sym;
			if (result->a_type == AST_CONSTEXPR &&
			    DeeObject_AsUInt(result->a_constexpr, &nth_symbol)) {
				DeeError_Handled(ERROR_HANDLED_RESTORE);
				if (WARN(W_EXPECTED_CONSTANT_AFTER_NTH))
					goto err_r;
			}
			ast_decref(result);
			sym = lookup_nth(nth_symbol, DeeLexer_GetTokenKwd(self));
			if likely(sym) {
				result = ast_sym(sym);
			} else {
				if (WARN(W_UNKNOWN_NTH_SYMBOL, nth_symbol))
					goto err;
				result = ast_constexpr(Dee_None);
			}
			if unlikely(!result)
				goto err;
			result = ast_sethere(self, result);
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
		} else {
			if (WARN(W_EXPECTED_KEYWORD_AFTER_NTH))
				goto err_r;
		}
		return result;
	}	break;

	case TOK_COLON_COLON:
		if (WARN(W_DEPRECATED_GLOBAL_PREFIX))
			goto err;
		ATTR_FALLTHROUGH
	case TPP_KWD_global:
		lookup_mode |= LOOKUP_SYM_VGLOBAL;
		goto do_warn_deprecated_modifier;
	case TPP_KWD_local:
		lookup_mode |= LOOKUP_SYM_VLOCAL;
do_warn_deprecated_modifier:
		if (WARN(W_DEPRECATED_PREFIX_IN_EXPRESSION))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		goto default_case;

	case TPP_TOK_DOT_DOT_DOT:
		if (current_basescope->bs_varargs) {
			DREF struct ast *new_result;
			/* Reference the varargs portion of the argument list. */
			result = ast_sethere(self, ast_sym(current_basescope->bs_varargs));
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
			 * >>     foo(...); // In the old deemon you'd have to write `foo((...)...);`
			 * >> }
			 */
			new_result = ast_sethere(self, ast_expand(result));
			ast_decref(result);
			if unlikely(!new_result)
				goto err;
			result = new_result;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			return result;
		}
		ATTR_FALLTHROUGH
	default:
default_case:
		if (DeeLexer_HasTokenKwd(self)) {
			/* Perform a regular symbol lookup. */
			tpp_keyword *name;
do_keyword:
			name = DeeLexer_GetTokenKwd(self);
			if (DeeLexer_GetLoc(self, &loc))
				goto err;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (DeeLexer_GetTok(self) == TPP_KWD_from) {
				/* `Error from deemon` - Short form of `import Error from deemon` */
				if (DeeLexer_GetLoc(self, &loc))
					goto err;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				result = ast_parse_import_single(self, name);
				if (result && (DeeLexer_GetTok(self) != ';' &&
				               DeeLexer_GetTok(self) != ',' &&
				               DeeLexer_GetTok(self) != ')' &&
				               DeeLexer_GetTok(self) != '}' &&
				               DeeLexer_GetTok(self) != ']' &&
				               DeeLexer_GetTok(self) != '\n' &&
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
				               TPPLexer_Current->l_token.t_id != TOK_ERR &&
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
				               /* Don't emit this warning from macros! */
				               !tpp_file_ismacro(DeeLexer_GetFile(self)))) {
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
							modname = DeeModule_GetShortName(sym->s_extern.e_module);
						}
					}
					if (WARNAT(&loc, W_UNCLEAR_SYMBOL_FROM_MODULE, symname, modname))
						goto err;
				}
			} else if (DeeLexer_GetTok(self) == TOK_ARROW) {
				/* Support for java-style lambda with singular argument. */
				result = ast_parse_function_java_lambda(self, name, &loc);
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
		return ast_setddi(result, &loc);
	}
	__builtin_unreachable();
err_r:
	ast_decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_unary_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict result) {
	DREF struct ast *merge;
	DREF struct ast *other;
	struct ast_loc loc;
	do {
		switch (DeeLexer_GetTok(self)) {

		case TOK_COLON_COLON:
			/* Backwards compatibility with deemon 100+ */
			if (WARN(W_DEPRECATED_ATTRIBUTE_SYNTAX))
				goto err_r;
			ATTR_FALLTHROUGH
		case '.': /* Attribute lookup */
			if (DeeLexer_GetLoc(self, &loc))
				goto err_r;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			if (DeeLexer_HasTokenKwd(self)) {
				DREF DeeObject *attr_name;
				if (DeeLexer_GetTok(self) == TPP_KWD_this)
					goto got_attr; /* This is really just a no-op. */
				if (DeeLexer_GetTok(self) == TPP_KWD_class) {
					/* Return the real class of a type, properly unwinding super. */
					merge = ast_sethere(self, ast_action1(AST_FACTION_CLASSOF, result));
				} else if (DeeLexer_GetTok(self) == TPP_KWD_super) {
					/* Return the real class of a type, properly unwinding super. */
					merge = ast_sethere(self, ast_action1(AST_FACTION_SUPEROF, result));
				} else if (DeeLexer_GetTok(self) == TPP_KWD_operator) {
					/* Named & bound operator invocation. */
					int32_t name;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_r;
					if (DeeLexer_GetLoc(self, &loc))
						goto err_r;
					name = ast_parse_operator_name(self, P_OPERATOR_FNORMAL);
					if unlikely(name < 0)
						goto err_r;
					if (DeeLexer_GetTok(self) != '(') {
						/* Bound-operator expression. */
						merge = ast_operator_func((Dee_operator_t)name, result);
					} else {
						DeeLexer_NoLf_Push(self);
						if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
/*err_r_operator_lparen_flags:*/
							DeeLexer_NoLf_Break(self);
							goto err_r;
						}
						if (DeeLexer_GetTok(self) == ')') {
							other = ast_constexpr(Dee_EmptyTuple);
						} else {
							other = ast_parse_comma(self,
							                        AST_COMMA_FORCEMULTIPLE,
							                        AST_FMULTIPLE_TUPLE,
							                        NULL);
						}
						DeeLexer_NoLf_Pop(self);
						if unlikely(!other)
							goto err_r;
						if (DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN))
							goto err_r_other;
						merge = ast_build_bound_operator((Dee_operator_t)name,
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
					if (is_reserved_symbol_name(DeeLexer_GetTokenKwd(self)) &&
					    WARN(W_RESERVED_ATTRIBUTE_NAME, DeeLexer_GetTokenKwd(self)))
						goto err;
					attr_name = DeeString_NewSized(DeeLexer_GetTokenKwdCStr(self),
					                               DeeLexer_GetTokenKwdLen(self));
					if unlikely(!attr_name)
						goto err_r;
					other = ast_sethere(self, ast_constexpr(attr_name));
					Dee_Decref(attr_name);
					if unlikely(!other)
						goto err_r;
					merge = ast_setddi(ast_operator2(OPERATOR_GETATTR, 0, result, other), &loc);
					ast_decref(other);
					ast_decref(result);
					if unlikely(!merge)
						goto err;
					result = merge;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_r;
					goto got_attr2;
				}
				ast_decref(result);
				if unlikely(!merge)
					goto err;
				result = merge;
got_attr:
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_r;
got_attr2:;
			} else {
				if (WARN(W_EXPECTED_KEYWORD_AFTER_DOT))
					goto err_r;
			}
			break;

		case '[': {
			/* sequence operator */
			if (DeeLexer_GetLoc(self, &loc))
				goto err_r;
			DeeLexer_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_r_lbracket_flags:
				DeeLexer_NoLf_Break(self);
				goto err_r;
			}
			if (DeeLexer_GetTok(self) == ':') {
				other = ast_constexpr(Dee_None);
				if unlikely(!other)
					goto err_r_lbracket_flags;
				goto do_range;
			}
			other = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!other)
				goto err_r_lbracket_flags;
			if (DeeLexer_GetTok(self) == ':') {
				DREF struct ast *third;
do_range:
				/* range operator. */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_r_lbracket_flags_other:
					ast_decref(other);
					goto err_r_lbracket_flags;
				}
				if (DeeLexer_GetTok(self) == ']') {
					third = ast_constexpr(Dee_None);
				} else {
					third = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
				}
				if unlikely(!third)
					goto err_r_lbracket_flags_other;
				merge = ast_operator3(OPERATOR_GETRANGE, 0, result, other, third);
				ast_decref(third);
			} else {
				merge = ast_operator2(OPERATOR_GETITEM, 0, result, other);
			}
			DeeLexer_NoLf_Pop(self);
			ast_decref(other);
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (DeeLexer_Skip2(self, ']', W_EXPECTED_RBRACKET_AFTER_GETITEM))
				goto err_r;
			ast_setddi(result, &loc);
		}	break;

		case '{': /* Brace initializers. */
			if (DeeLexer_GetLoc(self, &loc))
				goto err_r;
			other = ast_parse_unaryhead(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!other)
				goto err_r;
			/* Use the brace AST in a single-argument call to `result` */
			DREF struct ast **elemv;
			elemv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
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

		case TPP_KWD_pack: {
			int temp;
			DREF struct ast *kw_labels;
			/* Call expression without parenthesis. */
			if (DeeLexer_GetLoc(self, &loc))
				goto err_r;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			if (DeeLexer_GetTok(self) == '(')
				goto do_normal_call_with_loc;
			if unlikely(parser_warn_pack_used(&loc))
				goto err_r;
			temp = maybe_expression_begin(self);
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err_r;
				other = ast_setddi(ast_constexpr(Dee_EmptyTuple), &loc);
				kw_labels = NULL;
			} else {
				other = ast_parse_argument_list(self,
				                                AST_COMMA_FORCEMULTIPLE |
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
			if (DeeLexer_GetLoc(self, &loc))
				goto err_r;
do_normal_call_with_loc:
			DeeLexer_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_r_lparen_flags:
				DeeLexer_NoLf_Break(self);
				goto err_r;
			}
			if (DeeLexer_GetTok(self) == ')') {
				other     = ast_setddi(ast_constexpr(Dee_EmptyTuple), &loc);
				kw_labels = NULL;
			} else {
				other = ast_parse_argument_list(self, AST_COMMA_FORCEMULTIPLE, &kw_labels);
			}
			if unlikely(!other)
				goto err_r_lparen_flags;
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
			DeeLexer_NoLf_Pop(self);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN_AFTER_CALL))
				goto err_r;
		}	break;

		case TPP_TOK_PLUS_PLUS: {
			Dee_operator_t opid;
			/* Inplace operators. */
			opid = OPERATOR_INC;
			goto do_inplace_op;
		case TPP_TOK_MINUS_MINUS:
			opid = OPERATOR_DEC;
do_inplace_op:
			merge = ast_sethere(self, ast_operator1(opid, AST_OPERATOR_FPOSTOP, result));
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
		}	break;

		case TPP_TOK_DOT_DOT_DOT: /* Expand expression. */
			merge = ast_sethere(self, ast_expand(result));
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			break;

		default:
			goto done;
		}
	} while (result);
done:
	return result;
err_r_other:
	ast_decref(other);
err_r:
	ast_decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_unary(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_unaryhead(self, lookup_mode);
	if likely(result)
		result = ast_parse_unary_operand(self, result);
	return result;
}


INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_prod_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = DeeLexer_GetTok(self);
	ASSERT(TOKEN_IS_PROD(cmd));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		rhs = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_operator2(cmd == TPP_TOK_STAR_STAR ? OPERATOR_POW : GET_CHOP(cmd), 0, lhs, rhs);
		merge = ast_setddi(merge, &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		cmd = DeeLexer_GetTok(self);
		if (!TOKEN_IS_PROD(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_sum_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = DeeLexer_GetTok(self);
	ASSERT(TOKEN_IS_SUM(cmd));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT && cmd == '+') { /* sum */
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			merge = ast_action1(AST_FACTION_SUM, lhs);
		} else {
			rhs = ast_parse_prod(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			merge = ast_operator2(GET_CHOP(cmd), 0, lhs, rhs);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			break;
		cmd = DeeLexer_GetTok(self);
		if (!TOKEN_IS_SUM(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_shift_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = DeeLexer_GetTok(self);
	ASSERT(TOKEN_IS_SHIFT(cmd));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		rhs = ast_parse_sum(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_operator2(cmd == TPP_TOK_LANGLE_LANGLE ? OPERATOR_SHL : OPERATOR_SHR, 0, lhs, rhs);
		merge = ast_setddi(merge, &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		cmd = DeeLexer_GetTok(self);
		if (!TOKEN_IS_SHIFT(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_cmp_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = DeeLexer_GetTok(self);
	ASSERT(TOKEN_IS_CMP(cmd));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT && (cmd == '<' || cmd == '>')) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			merge = ast_action1(cmd == '<'
			                    ? AST_FACTION_MIN
			                    : AST_FACTION_MAX,
			                    lhs);
		} else {
			rhs = ast_parse_shift(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			merge = ast_operator2(cmd == TPP_TOK_LANGLE_EQUAL
			                      ? OPERATOR_LE
			                      : cmd == TPP_TOK_RANGLE_EQUAL
			                        ? OPERATOR_GE
			                        : GET_CHOP(cmd),
			                      0, lhs, rhs);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			break;
		cmd = DeeLexer_GetTok(self);
		if (!TOKEN_IS_SHIFT(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


/* Create a ??-expression AST. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF struct ast *DFCALL
ast_substitute_none(struct ast *__restrict this_value,
                    struct ast *__restrict or_this_if_none,
                    struct ast_loc *__restrict info) {
	DREF struct ast *none_ast;
	DREF struct ast *is_none_ast;
	DREF struct ast *result;
	none_ast = ast_constexpr(Dee_None);
	none_ast = ast_setddi(none_ast, info);
	if unlikely(!none_ast)
		goto err;
	is_none_ast = ast_action2(AST_FACTION_IS, this_value, none_ast);
	ast_decref(none_ast);
	is_none_ast = ast_setddi(is_none_ast, info);
	if unlikely(!is_none_ast)
		goto err;
	result = ast_conditional(AST_FCOND_EVAL, is_none_ast, or_this_if_none, this_value);
	ast_decref(is_none_ast);
	return ast_setddi(result, info);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_cmpeq_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = DeeLexer_GetTok(self);
	ASSERT(TOKEN_IS_CMPEQ(cmd));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (cmd == TPP_TOK_QMARK_QMARK) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			/* Code like "a ?? b" gets compiled as "a !is none ? REUSE(a) : b",
			 * where REUSE() means that the branch isn't evaluated a second time. */
			rhs = ast_parse_cmp(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			merge = ast_substitute_none(lhs, rhs, &loc);
			ast_decref(lhs);
			ast_decref(rhs);
			lhs = merge;
			if unlikely(!lhs)
				break;
		} else {
			bool invert;
			invert = cmd == '!';
yield_again:
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			if (DeeLexer_GetTok(self) == '!') {
				invert ^= 1;
				goto yield_again;
			}
			if (cmd == '!') {
				if (DeeLexer_GetTok(self) == TPP_KWD_is || DeeLexer_GetTok(self) == TPP_KWD_in) {
					cmd = DeeLexer_GetTok(self);
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_r;
				} else {
					/* TODO: Must rewind to start of '!'-sequence in this case and
					 *       not try to parse any additional in/is expressions:
					 * >> local foo = "value";
					 * >> local bar = f"foo = {foo!r}"; // << we mustn't parse the '!' here!
					 *
					 * iow: the `W_EXPECTED_IS_OR_IN_AFTER_EXCLAIM` warning needs to go away
					 */
					if (WARN(W_EXPECTED_IS_OR_IN_AFTER_EXCLAIM))
						goto err_r;
					cmd = TPP_KWD_is;
				}
			}
			if (DeeLexer_GetTok(self) == TPP_KWD_bound && cmd == TPP_KWD_is) {
				/* Special cast: `foo is bound` --> `bound(foo)` */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_r;
				merge = make_bound_expression(lhs, &loc);
			} else {
				rhs = ast_parse_cmp(self, LOOKUP_SYM_SECONDARY);
				if unlikely(!rhs)
					goto err_r;
				if (cmd == TPP_TOK_EQUAL_EQUAL || cmd == TPP_TOK_EXCLAIM_EQUAL) {
					merge = ast_operator2(cmd == TPP_TOK_EQUAL_EQUAL
					                      ? OPERATOR_EQ
					                      : OPERATOR_NE,
					                      0, lhs, rhs);
				} else {
					merge = ast_action2(cmd == TPP_KWD_is
					                    ? AST_FACTION_IS
					                    : cmd == TPP_TOK_EQUAL_EQUAL_EQUAL
					                      ? AST_FACTION_SAMEOBJ
					                      : cmd == TPP_TOK_EXCLAIM_EQUAL_EQUAL
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
				merge = ast_bool(AST_FBOOL_NEGATE, lhs);
				merge = ast_setddi(merge, &loc);
				ast_decref(lhs);
				lhs = merge;
				if unlikely(!lhs)
					break;
			}
		}
		cmd = DeeLexer_GetTok(self);
		if (!TOKEN_IS_CMPEQ(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_and_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_AND(DeeLexer_GetTok(self)));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		rhs = ast_parse_cmpeq(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_operator2(OPERATOR_AND, 0, lhs, rhs);
		merge = ast_setddi(merge, &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_AND(DeeLexer_GetTok(self)))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_xor_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_XOR(DeeLexer_GetTok(self)));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		rhs = ast_parse_and(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_operator2(OPERATOR_XOR, 0, lhs, rhs);
		merge = ast_setddi(merge, &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_XOR(DeeLexer_GetTok(self)))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_or_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_OR(DeeLexer_GetTok(self)));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		rhs = ast_parse_xor(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_operator2(OPERATOR_OR, 0, lhs, rhs);
		merge = ast_setddi(merge, &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_OR(DeeLexer_GetTok(self)))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_as_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_AS(DeeLexer_GetTok(self)));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		rhs = ast_parse_or(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		merge = ast_action2(AST_FACTION_AS, lhs, rhs);
		merge = ast_setddi(merge, &loc);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = merge;
		if unlikely(!lhs)
			break;
		if (!TOKEN_IS_AS(DeeLexer_GetTok(self)))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_land_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_LAND(DeeLexer_GetTok(self)));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			merge = ast_action1(AST_FACTION_ALL, lhs);
		} else {
			rhs = ast_parse_as(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			merge = ast_land(lhs, rhs);
			ast_decref(rhs);
		}
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			goto err;
		if (!TOKEN_IS_LAND(DeeLexer_GetTok(self)))
			break;
	}
	if (DeeLexer_GetTok(self) == TPP_TOK_PIPE_PIPE &&
	    WARN(W_CONSIDER_PAREN_AROUND_LAND))
		goto err_r;
	return lhs;
err_r:
	ast_decref(lhs);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_lor_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	ASSERT(TOKEN_IS_LOR(DeeLexer_GetTok(self)));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			merge = ast_action1(AST_FACTION_ANY, lhs);
		} else {
			rhs = ast_parse_lor(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			if (TOKEN_IS_LAND(DeeLexer_GetTok(self))) {
				/* Suggest parenthesis around logical-and. */
				if (WARN(W_CONSIDER_PAREN_AROUND_LAND))
					goto err_r_rhs;
				rhs = ast_parse_land_operand(self, rhs);
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
		if (!TOKEN_IS_LOR(DeeLexer_GetTok(self)))
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

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_cond_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *merge, *tt, *ff;
	struct ast_loc loc;
	/* >>  x ? y : z // >> x ? y : z
	 * >>  x ?: z    // >> x ? x : z
	 * >> (x ? y : ) // >> x ? y : x
	 * >> (x ? y)    // >> x ? y : none
	 */
	ASSERT(TOKEN_IS_COND(DeeLexer_GetTok(self)));
	for (;;) {
		uint16_t expect;
		expect = current_tags.at_expect;
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		if (DeeLexer_GetTok(self) == ':') {
			/* Missing true-branch. (Reuse the condition branch!) */
			tt = lhs;
			ast_incref(lhs);
		} else {
			tt = ast_parse_cond(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!tt)
				goto err_r;
		}
		if (DeeLexer_GetTok(self) == ':') {
			/* Parse the false-branch. */
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_tt;
			if (tt != lhs) {
				int temp;
				temp = maybe_expression_begin(self);
				if (temp > 0)
					goto do_parse_ff_branch;
				if unlikely(temp < 0)
					goto err_tt;
				/* Missing false-branch. (Reuse the condition branch!)
				 * >> This is a new extension of deemon that completes semantics
				 *    by allowing the reverse of what `foo() ?: bar()` already does
				 *    by specifying the syntax `(foo() ? bar() :)` */
				ff = lhs;
				ast_incref(lhs);
			} else {
do_parse_ff_branch:
				ff = ast_parse_cond(self, LOOKUP_SYM_SECONDARY);
				if unlikely(!ff)
					goto err_tt;
			}
		} else {
			/* Missing false-branch will be evaluated to `none` */
			ff = NULL;
		}
		merge = ast_conditional(AST_FCOND_EVAL | expect, lhs, tt, ff);
		merge = ast_setddi(merge, &loc);
		ast_xdecref(ff);
		ast_decref(tt);
		ast_decref(lhs);
		lhs = merge;
		if (!TOKEN_IS_COND(DeeLexer_GetTok(self)))
			break;
	}
	return lhs;
err_tt:
	ast_decref(tt);
err_r:
	ast_decref(lhs);
	return NULL;
}



#define TOK_INPLACE_MIN       TPP_TOK_PLUS_EQUAL
#define OPERATOR_INPLACE_MIN  OPERATOR_INPLACE_ADD
STATIC_ASSERT((TPP_TOK_PLUS_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_ADD - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_MINUS_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SUB - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_STAR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MUL - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_SLASH_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_DIV - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_PERCENT_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MOD - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_LANGLE_LANGLE_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHL - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_RANGLE_RANGLE_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_AMP_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_AND - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_PIPE_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_OR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_HAT_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_XOR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TPP_TOK_STAR_STAR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_POW - OPERATOR_INPLACE_MIN));
PRIVATE Dee_operator_t const inplace_fops[] = {
	/* [TPP_TOK_PLUS_EQUAL          - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_ADD,
	/* [TPP_TOK_MINUS_EQUAL         - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SUB,
	/* [TPP_TOK_STAR_EQUAL          - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_MUL,
	/* [TPP_TOK_SLASH_EQUAL         - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_DIV,
	/* [TPP_TOK_PERCENT_EQUAL       - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_MOD,
	/* [TPP_TOK_LANGLE_LANGLE_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SHL,
	/* [TPP_TOK_RANGLE_RANGLE_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SHR,
	/* [TPP_TOK_AMP_EQUAL           - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_AND,
	/* [TPP_TOK_PIPE_EQUAL          - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_OR,
	/* [TPP_TOK_HAT_EQUAL           - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_XOR,
	/* [TPP_TOK_STAR_STAR_EQUAL     - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_POW
};

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_assign_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tok_t cmd = DeeLexer_GetTok(self);
	ASSERT(TOKEN_IS_ASSIGN(cmd));
	for (;;) {
		if (DeeLexer_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		rhs = ast_parse_cond(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
		if (cmd == TPP_TOK_COLON_EQUAL) {
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
		cmd = DeeLexer_GetTok(self);
		if (!TOKEN_IS_ASSIGN(cmd))
			break;
	}
	return lhs;
err_r:
	ast_decref(lhs);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_prod(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_unary(self, lookup_mode);
	if (likely(result) && TOKEN_IS_PROD(DeeLexer_GetTok(self)))
		result = ast_parse_prod_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_sum(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_prod(self, lookup_mode);
	if (likely(result) && TOKEN_IS_SUM(DeeLexer_GetTok(self)))
		result = ast_parse_sum_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_shift(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_sum(self, lookup_mode);
	if (likely(result) && TOKEN_IS_SHIFT(DeeLexer_GetTok(self)))
		result = ast_parse_shift_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_cmp(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_shift(self, lookup_mode);
	if (likely(result) && TOKEN_IS_CMP(DeeLexer_GetTok(self)))
		result = ast_parse_cmp_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_cmpeq(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_cmp(self, lookup_mode);
	if (likely(result) && TOKEN_IS_CMPEQ(DeeLexer_GetTok(self)))
		result = ast_parse_cmpeq_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_and(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_cmpeq(self, lookup_mode);
	if (likely(result) && TOKEN_IS_AND(DeeLexer_GetTok(self)))
		result = ast_parse_and_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_xor(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_and(self, lookup_mode);
	if (likely(result) && TOKEN_IS_XOR(DeeLexer_GetTok(self)))
		result = ast_parse_xor_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_or(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_xor(self, lookup_mode);
	if (likely(result) && TOKEN_IS_OR(DeeLexer_GetTok(self)))
		result = ast_parse_or_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_as(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_or(self, lookup_mode);
	if (likely(result) && TOKEN_IS_AS(DeeLexer_GetTok(self)))
		result = ast_parse_as_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_land(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_as(self, lookup_mode);
	if (likely(result) && TOKEN_IS_LAND(DeeLexer_GetTok(self)))
		result = ast_parse_land_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_lor(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_land(self, lookup_mode);
	if (likely(result) && TOKEN_IS_LOR(DeeLexer_GetTok(self)))
		result = ast_parse_lor_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_cond(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_lor(self, lookup_mode);
	if (likely(result) && TOKEN_IS_COND(DeeLexer_GetTok(self)))
		result = ast_parse_cond_operand(self, result);
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_assign(DeeLexer *self, unsigned int lookup_mode) {
#ifdef __OPTIMIZE_SIZE__
	DREF struct ast *result;
	result = ast_parse_cond(self, lookup_mode);
	if (likely(result) && TOKEN_IS_ASSIGN(DeeLexer_GetTok(self)))
		result = ast_parse_assign_operand(self, result);
	return result;
#elif 1
	DREF struct ast *result;
	result = ast_parse_unary(self, lookup_mode);
	if unlikely(!result)
		goto done;
	switch (DeeLexer_GetTok(self)) {
	CASE_TOKEN_IS_PROD:
		result = ast_parse_prod_operand(self, result);
		if unlikely(!result)
			goto done;
		if (TOKEN_IS_SUM(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_SUM:
			result = ast_parse_sum_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_SHIFT(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_SHIFT:
			result = ast_parse_shift_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_CMP(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_CMP:
			result = ast_parse_cmp_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_CMPEQ(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_CMPEQ:
			result = ast_parse_cmpeq_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_AND(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_AND:
			result = ast_parse_and_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_XOR(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_XOR:
			result = ast_parse_xor_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_OR(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_OR:
			result = ast_parse_or_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_AS(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_AS:
			result = ast_parse_as_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_LAND(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_LAND:
			result = ast_parse_land_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_LOR(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_LOR:
			result = ast_parse_lor_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_COND(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_COND:
			result = ast_parse_cond_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_ASSIGN(DeeLexer_GetTok(self))) {
	CASE_TOKEN_IS_ASSIGN:
			result = ast_parse_assign_operand(self, result);
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
	result = ast_parse_unary(self, lookup_mode);
	if unlikely(!result)
		goto done;
	/* parse binary operators */
	if (TOKEN_IS_PROD(DeeLexer_GetTok(self))) {
		result = ast_parse_prod_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_SUM(DeeLexer_GetTok(self))) {
		result = ast_parse_sum_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_SHIFT(DeeLexer_GetTok(self))) {
		result = ast_parse_shift_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_CMP(DeeLexer_GetTok(self))) {
		result = ast_parse_cmp_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_CMPEQ(DeeLexer_GetTok(self))) {
		result = ast_parse_cmpeq_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_AND(DeeLexer_GetTok(self))) {
		result = ast_parse_and_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_XOR(DeeLexer_GetTok(self))) {
		result = ast_parse_xor_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_OR(DeeLexer_GetTok(self))) {
		result = ast_parse_or_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_AS(DeeLexer_GetTok(self))) {
		result = ast_parse_as_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_LAND(DeeLexer_GetTok(self))) {
		result = ast_parse_land_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_LOR(DeeLexer_GetTok(self))) {
		result = ast_parse_lor_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_COND(DeeLexer_GetTok(self))) {
		result = ast_parse_cond_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_ASSIGN(DeeLexer_GetTok(self))) {
		result = ast_parse_assign_operand(self, result);
		if unlikely(!result)
			goto done;
	}
done:
	return result;
#else
	DREF struct ast *result;
	result = ast_parse_cond(self, lookup_mode);
	if (likely(result) && TOKEN_IS_ASSIGN(DeeLexer_GetTok(self)))
		result = ast_parse_assign_operand(self, result);
	return result;
#endif
}



INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_postexpr(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict baseexpr) {
	baseexpr = ast_parse_unary_operand(self, baseexpr);
	if unlikely(!baseexpr)
		goto done;
	/* parse binary operators */
	if (TOKEN_IS_PROD(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_prod_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_SUM(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_sum_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_SHIFT(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_shift_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_CMP(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_cmp_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_CMPEQ(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_cmpeq_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_AND(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_and_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_XOR(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_xor_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_OR(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_or_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_AS(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_as_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_LAND(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_land_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_LOR(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_lor_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_COND(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_cond_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_ASSIGN(DeeLexer_GetTok(self))) {
		baseexpr = ast_parse_assign_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
done:
	return baseexpr;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_EXPRESSION_C */
