/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_FORMAT_C
#define GUARD_DEEMON_OBJECTS_UNICODE_FORMAT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/objectlist.h>

#include <stdbool.h>

#include "../../runtime/runtime_error.h"

#undef SSIZE_MIN
#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

/*
 * =========================== Format strings ===========================
 *
 * These format patterns are used to drive template strings, as well as
 * provide for more advanced patterns. They come in 2 variants:
 * - Simple
 * - Advanced
 * - These patterns are mutually exclusive, allowing detection of which
 *   one we're dealing with. Additionally, even though you might think
 *   that you're allowed to mix these 2 variants, you can't. You must
 *   always use either one, or the other for the *entire* format pattern.
 *
 *
 * Examples:
 * - Simple:
 *   >> print r"\{foo = {}, bar = {}\}".format({ 10, 20 }); // "{foo = 10, bar = 20}"
 *   >> print r"{{foo = {}, bar = {}}}".format({ 10, 20 }); // "{foo = 10, bar = 20}"
 * - Advanced:
 *   >> // "Hello Mr Simpson\nWe are contacting you regarding your INSURANCE"
 *   >> print "Hello {salutation} {surname}\nWe are contacting you regarding your {subject.upper()}"
 *   >>           .format({ "salutation": "Mr", "surname": "Simpson", "subject": "Insurance" });
 *   >>
 *   >> // "Hello Mr Simpson\nWe are contacting you regarding your INSURANCE"
 *   >> print "Hello {0} {1}\nWe are contacting you regarding your {2.upper()}"
 *   >>           .format({ "Mr", "Simpson", "Insurance" });
 *
 *
 * Definitions:
 * >> TEXT ::= <any character> | '{{' | '}}';
 * >>
 * >> SIMPLE_TEMPLATE ::= [TEXT...]
 * >>                     [   // Start of template argument
 * >>                         '{'
 * >>                         // Template argument spec (when not given, defaults to "!s")
 * >>                         [
 * >>                             ('!' (
 * >>                                 'r' | // Insert `DeeObject_Repr()' of next argument
 * >>                                 's' | // Insert `DeeObject_Str()' of next argument
 * >>                                 'a'   // Insert `DeeObject_Str()' of next argument
 * >>                             )) |
 * >>                             // Insert `DeeObject_PrintFormatString()' of next argument
 * >>                             // The format string used cannot include other arguments.
 * >>                             (':' [TEXT...])
 * >>                         ]
 * >>                         '}'
 * >>                     ]
 * >>                     [SIMPLE_TEMPLATE];
 * >>
 * >> ADVANCED_TEMPLATE ::= [TEXT...]
 * >>                       [   // Start of template argument
 * >>                           '{'
 * >>                           // NOTE: Contents of "ARGUMENT_EXPR" here are always interpreted as utf-8,
 * >>                           //       even when the rest of the template string originates from Bytes.
 * >>                           ARGUMENT_EXPR
 * >>                           [
 * >>                               ('!' (
 * >>                                   'r' | // Insert `DeeObject_Repr()' of "ARGUMENT_EXPR"
 * >>                                   's' | // Insert `DeeObject_Str()' of "ARGUMENT_EXPR"
 * >>                                   'a'   // Insert `DeeObject_Str()' of "ARGUMENT_EXPR"
 * >>                               )) |
 * >>                               // Insert `DeeObject_PrintFormatString()' of "ARGUMENT_EXPR"
 * >>                               // The format string used is a recursive template string!
 * >>                               // NOTE: Like with ARGUMENT_EXPR, this part is always interpreted as utf-8
 * >>                               (':' [ADVANCED_TEMPLATE...])
 * >>                           ]
 * >>                           '}'
 * >>                       ]
 * >>                       [ADVANCED_TEMPLATE];
 * >>
 * >> ARGUMENT_EXPR_UNARY_BASE ::= (
 * >>                                  NUMBER                               // Uses "arg = args[int(<content>)]"
 * >>                                | SYMBOL                               // Uses "arg = args[string(<content>)]"
 * >>                                | ('"' [<any character>...] '"')       // Uses "arg = string(<content>)" (accepts c-escape)
 * >>                                | ("'" [<any character>...] "'")       // Uses "arg = string(<content>)" (accepts c-escape)
 * >>                                | ('$' NUMBER)                         // Uses "arg = int(<content>)"
 * >>                                | ('$' [NUMBER] '.' NUMBER)            // Uses "arg = float(<content>)"  // XXX: Document that this also accepts that E-suffix
 * >>                                | ('$true')                            // Uses "arg = true"
 * >>                                | ('$false')                           // Uses "arg = false"
 * >>                                | ('$none')                            // Uses "arg = none"
 * >>                                | ('$str' '(' ARGUMENT_EXPR ')')       // Uses "arg = str(ARGUMENT_EXPR)"
 * >>                                | ('$repr' '(' ARGUMENT_EXPR ')')      // Uses "arg = repr(ARGUMENT_EXPR)"
 * >>                                | ('$copy' '(' ARGUMENT_EXPR ')')      // Uses "arg = copy(ARGUMENT_EXPR)"
 * >>                                | ('$deepcopy' '(' ARGUMENT_EXPR ')')  // Uses "arg = deepcopy(ARGUMENT_EXPR)"
 * >>                                | ('$type' '(' ARGUMENT_EXPR ')')      // Uses "arg = type(ARGUMENT_EXPR)"
 * >>                                | ('!' ARGUMENT_EXPR_UNARY)            // Uses "arg = !ARGUMENT_EXPR"  (Not allowed at start of expr; "{!foo}" is malformed; use "{(!foo)}" instead!)
 * >>                                | ('#' ARGUMENT_EXPR_UNARY)            // Uses "arg = #ARGUMENT_EXPR"
 * >>                                | ('+' ARGUMENT_EXPR_UNARY)            // Uses "arg = +ARGUMENT_EXPR"
 * >>                                | ('-' ARGUMENT_EXPR_UNARY)            // Uses "arg = -ARGUMENT_EXPR"
 * >>                                | ('~' ARGUMENT_EXPR_UNARY)            // Uses "arg = ~ARGUMENT_EXPR"
 * >>                                | ('(' ARGUMENT_EXPR ')')              // Uses "arg = ARGUMENT_EXPR"
 * >>                                | ('(' ARGUMENT_EXPR_EXP ',' [',' ~~ (ARGUMENT_EXPR_EXP...)] ')')   // Uses "arg = Sequence({ARGUMENT_EXPR,...})" (Yes: sequence type isn't mandatory here; not necessary a Tuple)
 * >>                                | ('[' ARGUMENT_EXPR_EXP [',' [',' ~~ (ARGUMENT_EXPR_EXP...)]] ']') // Uses "arg = Sequence({ARGUMENT_EXPR,...})" (Yes: sequence type isn't mandatory here; not necessary a List)
 * >>                                | ('$bound' '(' ARGUMENT_EXPR_UNARY_BASE '.' SYMBOL ')')
 * >>                                | ('$bound' '(' ARGUMENT_EXPR_UNARY_BASE '.' '{' ARGUMENT_EXPR '}' ')')
 * >>                              );
 * >>
 * >> ARGUMENT_EXPR_UNARY ::= ARGUMENT_EXPR_UNARY_BASE [(
 * >>                             ('.' SYMBOL)                              // Modify `arg = arg.operator . (SYMBOL)'                        // Getattr operation
 * >>                             ('.' '{' ARGUMENT_EXPR '}')               // Modify `arg = arg.operator . (ARGUMENT_EXPR)'                 // Getattr operation
 * >>                           | ('[' ARGUMENT_EXPR ']')                   // Modify `arg = arg.operator [] (ARGUMENT_EXPR)'                // Getitem operation
 * >>                           | ('[' ARGUMENT_EXPR ':' ARGUMENT_EXPR ']') // Modify `arg = arg.operator [] (ARGUMENT_EXPR, ARGUMENT_EXPR)' // Getrange operation
 * >>                           | ('(' ARGUMENT_EXPR_CALL_ARGS ')')         // Modify `arg = arg.operator () (ARGUMENT_EXPR_CALL_ARGS)'      // Call operation
 * >>                         )...];
 * >>
 * >> ARGUMENT_EXPR_CALL_ARGS ::= [',' ~~ (ARGUMENT_EXPR_EXP...)] [
 * >>                                 [',']  // If unlabelled arguments were also used
 * >>                                 (',' ~~ (
 * >>                                     SYMBOL ':' ARGUMENT_EXPR
 * >>                                 )...)
 * >>                             ];
 * >>
 * >> ARGUMENT_EXPR_PROD ::= ARGUMENT_EXPR_UNARY [(
 * >>                            ('*' ARGUMENT_EXPR_UNARY)
 * >>                          | ('/' ARGUMENT_EXPR_UNARY)
 * >>                          | ('%' ARGUMENT_EXPR_UNARY)
 * >>                          | ('**' ARGUMENT_EXPR_UNARY)
 * >>                        )...];
 * >>
 * >> ARGUMENT_EXPR_SUM ::= ARGUMENT_EXPR_PROD [(
 * >>                           ('+' ARGUMENT_EXPR_PROD)
 * >>                         | ('-' ARGUMENT_EXPR_PROD)
 * >>                       )...];
 * >>
 * >> ARGUMENT_EXPR_SHIFT ::= ARGUMENT_EXPR_SUM [(
 * >>                             ('<<' ARGUMENT_EXPR_SUM)
 * >>                           | ('>>' ARGUMENT_EXPR_SUM)
 * >>                         )...];
 * >>
 * >> ARGUMENT_EXPR_CMP ::= ARGUMENT_EXPR_SHIFT [(
 * >>                           ('<' ARGUMENT_EXPR_SHIFT)
 * >>                         | ('<=' ARGUMENT_EXPR_SHIFT)
 * >>                         | ('>' ARGUMENT_EXPR_SHIFT)
 * >>                         | ('>=' ARGUMENT_EXPR_SHIFT)
 * >>                       )...];
 * >>
 * >> ARGUMENT_EXPR_CMPEQ ::= ARGUMENT_EXPR_CMP [(
 * >>                           ('==' ARGUMENT_EXPR_CMP)
 * >>                         | ('!=' ARGUMENT_EXPR_CMP)      (Not allowed at start of expr; "{foo != bar}" is malformed; use "{(foo != bar)}" instead!)
 * >>                         | ('===' ARGUMENT_EXPR_CMP)
 * >>                         | ('!==' ARGUMENT_EXPR_CMP)     (Not allowed at start of expr; "{foo !== bar}" is malformed; use "{(foo !== bar)}" instead!)
 * >>                         | ('??' ARGUMENT_EXPR_CMP)
 * >>                         | ('$is' ARGUMENT_EXPR_CMP)
 * >>                         | ('!' '$is' ARGUMENT_EXPR_CMP) (Not allowed at start of expr; "{foo !$is bar}" is malformed; use "{(foo !$is bar)}" instead!)
 * >>                         | ('$in' ARGUMENT_EXPR_CMP)
 * >>                         | ('!' '$in' ARGUMENT_EXPR_CMP) (Not allowed at start of expr; "{foo !$in bar}" is malformed; use "{(foo !$in bar)}" instead!)
 * >>                       )...];
 * >>
 * >> ARGUMENT_EXPR_AND ::= ARGUMENT_EXPR_CMPEQ [('&' ARGUMENT_EXPR_CMPEQ)...];
 * >> ARGUMENT_EXPR_XOR ::= ARGUMENT_EXPR_AND [('^' ARGUMENT_EXPR_AND)...];
 * >> ARGUMENT_EXPR_OR ::= ARGUMENT_EXPR_XOR [('|' ARGUMENT_EXPR_XOR)...];
 * >> ARGUMENT_EXPR_AS ::= ARGUMENT_EXPR_OR [('$as' ARGUMENT_EXPR_OR)...];
 * >> ARGUMENT_EXPR_LAND ::= ARGUMENT_EXPR_AS [('&&' ARGUMENT_EXPR_AS)...];
 * >> ARGUMENT_EXPR_LOR ::= ARGUMENT_EXPR_LAND [('||' ARGUMENT_EXPR_LAND)...];
 * >> ARGUMENT_EXPR_COND ::= ARGUMENT_EXPR_LOR ['?' ARGUMENT_EXPR_LOR : ARGUMENT_EXPR_COND]; // ff-branch is mandatory to prevent ambiguity in case of "{foo?bar:baz}" (is "baz" the argument for "(foo?bar).__format__", or is it the ff-branch?)
 * >>
 * >> ARGUMENT_EXPR ::= ARGUMENT_EXPR_COND; // Note that in here, whitespace outside of "strings" is ignored
 * >> ARGUMENT_EXPR_EXP ::= ARGUMENT_EXPR ['...'];
 * >>
 * >> NUMBER       ::= ['0x'|'0X'|'0b'|'0B'] ('0'...'9')...;
 * >> SYMBOL       ::= SYMBOL_START [SYMBOL_CONT...];
 * >> SYMBOL_START ::= (('a'...'z')|('A'...'Z')|'_'|'$');  // Anything matching `DeeUni_IsSymStrt()'
 * >> SYMBOL_CONT  ::= SYMBOL_START|('0'...'9');           // Anything matching `DeeUni_IsSymCont()'
 *
 */


PRIVATE ATTR_COLD int DCALL err_lbrace_in_format_in_simple(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unescaped '{' in {:format-string} in simple format "
	                       "pattern (only allowed in advanced patterns)");
}

PRIVATE ATTR_COLD int DCALL err_unmatched_lbrace_in_simple(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unmatched '{' in simple format pattern");
}

PRIVATE ATTR_COLD int DCALL err_unmatched_lbrace_in_advanced(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unmatched '{' in simple advanced pattern");
}

PRIVATE ATTR_COLD int DCALL err_unmatched_rbrace_in_simple(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unmatched '}' in simple format pattern");
}

PRIVATE ATTR_COLD int DCALL err_unmatched_rbrace_in_advanced(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unmatched '}' in advanced format pattern");
}

PRIVATE ATTR_COLD int DCALL err_unknown_repr_mode_in_simple(char const *mode) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unrecognized object repesentation mode '{!%.1q}' in simple format pattern",
	                       mode);
}

PRIVATE ATTR_COLD int DCALL err_unknown_repr_mode_in_advanced(char const *mode) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unrecognized object repesentation mode '{!%.1q}' in advanced format pattern",
	                       mode);
}

PRIVATE ATTR_COLD int DCALL err_invalid_char_after_lbrace_in_simple(char const *ptr) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid character %.1q following `{' in simple format pattern",
	                       ptr);

}

PRIVATE ATTR_COLD int DCALL err_invalid_char_after_expr_in_advanced(char const *ptr) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid character %.1q following `{<expr>' in advanced format pattern",
	                       ptr);

}

PRIVATE ATTR_COLD int DCALL err_invalid_char_after_lbrace_exclaim_spec_in_simple(char spec, char const *ptr) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Expected '}' after '{!%c' in simple format pattern, but got %.1q",
	                       spec, ptr);
}

PRIVATE ATTR_COLD int DCALL err_invalid_char_after_lbrace_exclaim_spec_in_advanced(char spec, char const *ptr) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Expected '}' after '{!%c' in advanced format pattern, but got %.1q",
	                       spec, ptr);
}

/* Return a pointer to the next '{' or '}' character.
 * If not found, return `NULL' instead. */
PRIVATE WUNUSED char const *DCALL
find_next_brace(char const *str, char const *end) {
	for (; str < end; ++str) {
		if (*str == '{' || *str == '}')
			return str;
	}
	return NULL;
}



enum {
	/* Special tokens. */
	SFA_TOK_EOF       = '\0', /* END-OF-FILE (will always be ZERO) */
	SFA_TOK_CHAR      = '\'', /* 'f'. */
	SFA_TOK_STRING    = '\"', /* "foobar" */
	SFA_TOK_INT       = '0',  /* 42 */

	/* Double (or longer) tokens. */
	SFA_TOK_TWOCHAR_BEGIN = 256,
	SFA_TOK_SHL = SFA_TOK_TWOCHAR_BEGIN, /* "<<". */
	SFA_TOK_SHR,           /* ">>". */
	SFA_TOK_EQUAL,         /* "==". */
	SFA_TOK_NOT_EQUAL,     /* "!=". */
	SFA_TOK_GREATER_EQUAL, /* ">=". */
	SFA_TOK_LOWER_EQUAL,   /* "<=". */
	SFA_TOK_DOTS,          /* "...". */
	SFA_TOK_LAND,          /* "&&". */
	SFA_TOK_LOR,           /* "||". */
	SFA_TOK_POW,           /* "**". */
	SFA_TOK_EQUAL3,        /* "===". */
	SFA_TOK_NOT_EQUAL3,    /* "!==". */
	SFA_TOK_QMARK_QMARK,   /* "??". */
	SFA_TOK_D_IS,          /* "$is". */
	SFA_TOK_D_IN,          /* "$in". */
	SFA_TOK_D_AS,          /* "$as". */
	SFA_TOK_D_TRUE,        /* "$true". */
	SFA_TOK_D_FALSE,       /* "$false". */
	SFA_TOK_D_NONE,        /* "$none". */
	SFA_TOK_D_STR,         /* "$str". */
	SFA_TOK_D_REPR,        /* "$repr". */
	SFA_TOK_D_COPY,        /* "$copy". */
	SFA_TOK_D_DEEPCOPY,    /* "$deepcopy". */
	SFA_TOK_D_TYPE,        /* "$type". */
	SFA_TOK_D_BOUND,       /* "$bound". */
	SFA_TOK_D_INT,         /* "$42". */
	SFA_TOK_KEYWORD,       /* KEEP THIS THE LAST TOKEN! */
};

struct string_format_parser {
	char const *sfp_iter; /* [1..1] Pointer just after the next '{' character. */
	char const *sfp_wend; /* [1..1] Input string end pointer. */
};

struct string_format_advanced {
	struct string_format_parser sfa_parser;  /* Underlying parser */
	DeeObject                  *sfa_args;    /* [1..1] Format template arguments */
	Dee_formatprinter_t         sfa_printer; /* [1..1] Output printer */
	void                       *sfa_arg;     /* [?..?] Cookie for `sfa_printer' */
	unsigned int                sfa_exprtok; /* Current expression token (`sfa_parser.sfp_iter' usually points after the
	                                          * token, expect for `SFA_TOK_CHAR', `SFA_TOK_STRING', `SFA_TOK_INT' and
	                                          * `SFA_TOK_KEYWORD', where it points to the start) */
	bool                        sfa_inparen; /* Are we within parenthesis? */
};


PRIVATE NONNULL((1)) unsigned int DFCALL
sfa_yield(struct string_format_advanced *__restrict self) {
#define RETURN0(x) do{ result = (x); goto done0; }__WHILE0
#define RETURN1(x) do{ result = (x); goto done1; }__WHILE0
#define RETURN2(x) do{ result = (x); goto done2; }__WHILE0
#define RETURN3(x) do{ result = (x); goto done3; }__WHILE0
#define RETURN5(x) do{ result = (x); goto done5; }__WHILE0
#define RETURN6(x) do{ result = (x); goto done6; }__WHILE0
#define RETURN9(x) do{ result = (x); goto done9; }__WHILE0
	unsigned int result;
	unsigned char ch;
	char const *ptr;
	ptr = self->sfa_parser.sfp_iter;
again:
	if unlikely(ptr >= self->sfa_parser.sfp_wend)
		RETURN0(SFA_TOK_EOF);
	ch = (unsigned char)*ptr;
	switch (ch) {

	case '<':
	case '>':
	case '=':
	case '!':
	case '.':
	case '&':
	case '|':
	case '*':
	case '?':
		if likely((ptr + 1) < self->sfa_parser.sfp_wend) {
			unsigned char nextch = (unsigned char)ptr[1];
			switch (ch) {
			case '<':
				if (nextch == '<')
					RETURN2(SFA_TOK_SHL);
				if (nextch == '=')
					RETURN2(SFA_TOK_LOWER_EQUAL);
				break;
			case '>':
				if (nextch == '>')
					RETURN2(SFA_TOK_SHR);
				if (nextch == '=')
					RETURN2(SFA_TOK_GREATER_EQUAL);
				break;
			case '=':
				if (nextch == '=') {
					if likely((ptr + 2) < self->sfa_parser.sfp_wend) {
						if (ptr[2] == '=')
							RETURN3(SFA_TOK_EQUAL3);
					}
					RETURN2(SFA_TOK_EQUAL);
				}
				break;
			case '!':
				if (nextch == '=') {
					if likely((ptr + 2) < self->sfa_parser.sfp_wend) {
						if (ptr[2] == '=')
							RETURN3(SFA_TOK_NOT_EQUAL3);
					}
					RETURN2(SFA_TOK_NOT_EQUAL);
				}
				break;
			case '.':
				if (nextch == '.') {
					if likely((ptr + 2) < self->sfa_parser.sfp_wend) {
						if (ptr[2] == '.')
							RETURN3(SFA_TOK_DOTS);
					}
				}
				break;
			case '&':
				if (nextch == '&')
					RETURN2(SFA_TOK_LAND);
				break;
			case '|':
				if (nextch == '|')
					RETURN2(SFA_TOK_LOR);
				break;
			case '*':
				if (nextch == '*')
					RETURN2(SFA_TOK_POW);
				break;
			case '?':
				if (nextch == '*')
					RETURN2(SFA_TOK_QMARK_QMARK);
				break;
			default: __builtin_unreachable();
			}
		}
		goto onechar;

	case '$':
		if likely((ptr + 2) < self->sfa_parser.sfp_wend) {
			unsigned char nextch1 = (unsigned char)ptr[1];
			unsigned char nextch2 = (unsigned char)ptr[2];
			size_t kwd_len = 2;
			while ((ptr + kwd_len + 1) < self->sfa_parser.sfp_wend &&
			       DeeUni_IsSymCont(ptr[kwd_len + 1]))
				++kwd_len;
			switch (nextch1) {
			case 'i':
				if (kwd_len == 2) {
					if (nextch2 == 's')
						RETURN3(SFA_TOK_D_IS);
					if (nextch2 == 'n')
						RETURN3(SFA_TOK_D_IN);
				}
				break;

			case 'a':
				if (kwd_len == 2) {
					if (nextch2 == 's')
						RETURN3(SFA_TOK_D_AS);
				}
				break;

			case 't':
				if (kwd_len == 4) {
					if (nextch2 == 'r' && ptr[3] == 'u' && ptr[4] == 'e')
						RETURN5(SFA_TOK_D_TRUE);
					if (nextch2 == 'y' && ptr[3] == 'p' && ptr[4] == 'e')
						RETURN5(SFA_TOK_D_TYPE);
				}
				break;

			case 'f':
				if (kwd_len == 5) {
					if (nextch2 == 'a' && ptr[3] == 'l' && ptr[4] == 's' && ptr[5] == 'e')
						RETURN5(SFA_TOK_D_FALSE);
				}
				break;

			case 'n':
				if (kwd_len == 4) {
					if (nextch2 == 'o' && ptr[3] == 'n' && ptr[4] == 'e')
						RETURN5(SFA_TOK_D_NONE);
				}
				break;

			case 's':
				if (kwd_len == 3) {
					if (nextch2 == 't' && ptr[3] == 'r')
						RETURN5(SFA_TOK_D_STR);
				}
				break;

			case 'r':
				if (kwd_len == 4) {
					if (nextch2 == 'e' && ptr[3] == 'p' && ptr[4] == 'r')
						RETURN5(SFA_TOK_D_REPR);
				}
				break;

			case 'c':
				if (kwd_len == 4) {
					if (nextch2 == 'o' && ptr[3] == 'p' && ptr[4] == 'y')
						RETURN5(SFA_TOK_D_COPY);
				}
				break;

			case 'd':
				if (kwd_len == 8) {
					if (nextch2 == 'e' && ptr[3] == 'e' && ptr[4] == 'p' &&
					    ptr[5] == 'c' && ptr[6] == 'o' && ptr[7] == 'p' && ptr[8] == 'y')
						RETURN9(SFA_TOK_D_DEEPCOPY);
				}
				break;

			case 'b':
				if (kwd_len == 5) {
					if (nextch2 == 'o' && ptr[3] == 'u' && ptr[4] == 'n' && ptr[5] == 'd')
						RETURN6(SFA_TOK_D_BOUND);
				}
				break;

			default: break;
			}
		}
		if likely((ptr + 1) < self->sfa_parser.sfp_wend) {
			unsigned char nextch1 = (unsigned char)ptr[1];
			if (DeeUni_IsDigit(nextch1))
				RETURN1(SFA_TOK_D_INT); /* Consume the '$', but leave the integer as-is */
		}
		goto onechar;

	default:
		if (ch >= 0x80) {
			char const *next_ptr = ptr;
			uint32_t uni = Dee_unicode_readutf8_n(&next_ptr, self->sfa_parser.sfp_wend);
			uniflag_t flags = DeeUni_Flags(uni);
			if (flags & Dee_UNICODE_ISSPACE) {
				ptr = next_ptr;
				goto again;
			} else if (flags & Dee_UNICODE_ISDIGIT) {
				RETURN0(SFA_TOK_INT);
			} else {
				RETURN0(SFA_TOK_KEYWORD);
			}
		} else {
			uniflag_t flags;
			flags = DeeUni_Flags(ch);
			if (flags & Dee_UNICODE_ISSPACE) {
	case ' ': case '\t': case '\r': case '\n':
				++ptr;
				goto again;
			} else if (flags & Dee_UNICODE_ISDIGIT) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
				RETURN0(SFA_TOK_INT);
			} else if (flags & Dee_UNICODE_ISSYMSTRT) {
	case 'a': case 'b': case 'c': case 'd': case 'e':
	case 'f': case 'g': case 'h': case 'i': case 'j':
	case 'k': case 'l': case 'm': case 'n': case 'o':
	case 'p': case 'q': case 'r': case 's': case 't':
	case 'u': case 'v': case 'w': case 'x': case 'y':
	case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E':
	case 'F': case 'G': case 'H': case 'I': case 'J':
	case 'K': case 'L': case 'M': case 'N': case 'O':
	case 'P': case 'Q': case 'R': case 'S': case 'T':
	case 'U': case 'V': case 'W': case 'X': case 'Y':
	case 'Z':
	case '_':
				RETURN0(SFA_TOK_KEYWORD);
			} else {
onechar:
				RETURN1(ch);
			}
		}
		break;
	}
done1:
	++ptr;
done0:
	self->sfa_parser.sfp_iter = ptr;
	self->sfa_exprtok         = result;
	return result;
done9:
	ptr += 3;
done6:
	++ptr;
done5:
	ptr += 2;
done3:
	++ptr;
done2:
	++ptr;
	goto done1;
#undef RETURN0
#undef RETURN1
#undef RETURN2
#undef RETURN3
#undef RETURN5
#undef RETURN6
#undef RETURN9
}

PRIVATE NONNULL((1)) size_t DFCALL
sfa_tok_int_getlen(struct string_format_advanced *__restrict self) {
	char const *int_end = self->sfa_parser.sfp_iter;
	while (int_end < self->sfa_parser.sfp_wend && DeeUni_IsAlnum(*int_end))
		++int_end;
	return (size_t)(int_end - self->sfa_parser.sfp_iter);
}

PRIVATE NONNULL((1)) size_t DFCALL
sfa_tok_keyword_getlen(struct string_format_advanced *__restrict self) {
	char const *kwd_end = self->sfa_parser.sfp_iter;
	while (kwd_end < self->sfa_parser.sfp_wend) {
		char const *nextptr = kwd_end;
		uint32_t ch = Dee_unicode_readutf8_n(&nextptr, self->sfa_parser.sfp_wend);
		if (!DeeUni_IsSymCont(ch))
			break;
		kwd_end = nextptr;
	}
	return (size_t)(kwd_end - self->sfa_parser.sfp_iter);
}

PRIVATE ATTR_COLD NONNULL((1)) size_t DFCALL
sfa_tok_getlen_for_rewind(struct string_format_advanced *__restrict self) {
	size_t result = 1;
	switch (self->sfa_exprtok) {
	case SFA_TOK_KEYWORD:
	case SFA_TOK_CHAR:
	case SFA_TOK_STRING:
	case SFA_TOK_INT:
	case SFA_TOK_D_INT:
		result = 0;
		break;
	case SFA_TOK_SHR:
	case SFA_TOK_EQUAL:
	case SFA_TOK_NOT_EQUAL:
	case SFA_TOK_GREATER_EQUAL:
	case SFA_TOK_LOWER_EQUAL:
	case SFA_TOK_LAND:
	case SFA_TOK_LOR:
	case SFA_TOK_POW:
	case SFA_TOK_QMARK_QMARK:
		result = 2;
		break;
	case SFA_TOK_DOTS:
	case SFA_TOK_EQUAL3:
	case SFA_TOK_NOT_EQUAL3:
	case SFA_TOK_D_IS:
	case SFA_TOK_D_IN:
	case SFA_TOK_D_AS:
		result = 3;
		break;
	case SFA_TOK_D_STR:
		result = 4;
		break;
	case SFA_TOK_D_TRUE:
	case SFA_TOK_D_NONE:
	case SFA_TOK_D_REPR:
	case SFA_TOK_D_COPY:
	case SFA_TOK_D_TYPE:
		result = 5;
		break;
	case SFA_TOK_D_FALSE:
	case SFA_TOK_D_BOUND:
		result = 6;
		break;
	case SFA_TOK_D_DEEPCOPY:
		result = 9;
		break;
	default: break;
	}
	return result;
}

PRIVATE ATTR_COLD NONNULL((1)) int DFCALL
sfa_err_bad_token(struct string_format_advanced *__restrict self,
                  char const *expected_token) {
	char const *actual_token_repr = NULL;
	size_t actual_token_length = 0;
	switch (self->sfa_exprtok) {
	case SFA_TOK_KEYWORD:
		actual_token_length = sfa_tok_keyword_getlen(self);
		break;
	case SFA_TOK_CHAR:
	case SFA_TOK_STRING:
		actual_token_repr = "<string>";
		break;
	case SFA_TOK_INT:
		actual_token_repr = "<int>";
		break;
	case SFA_TOK_D_INT:
		actual_token_repr = "<$int>";
		break;
	default:
		actual_token_length = sfa_tok_getlen_for_rewind(self);
		break;
	}
	if (actual_token_repr) {
		actual_token_length = strlen(actual_token_repr);
	} else {
		actual_token_repr = self->sfa_parser.sfp_iter;
	}
	return DeeError_Throwf(&DeeError_ValueError,
	                       *expected_token == '<'
	                       ? "Unexpected token %.?q in advanced format pattern expression when %s was expected"
	                       : "Unexpected token %.?q in advanced format pattern expression when %q was expected",
	                       actual_token_length, actual_token_repr, expected_token);
}


PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipunary_base(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipunary(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipprod(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipsum(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipshift(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipcmp(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipcmpeq(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipxor(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipor(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipas(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipland(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skiplor(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipcond(struct string_format_advanced *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalunary_base(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalunary(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalprod(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalsum(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalshift(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalcmp(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalcmpeq(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evaland(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalxor(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalor(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalas(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalland(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evallor(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalcond(struct string_format_advanced *__restrict self);

PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipunary_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipprod_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipsum_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipshift_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipcmp_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipcmpeq_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipand_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipxor_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipor_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipas_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipland_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skiplor_operand(struct string_format_advanced *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DFCALL sfa_skipcond_operand(struct string_format_advanced *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalunary_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalprod_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalsum_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalshift_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalcmp_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalcmpeq_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evaland_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalxor_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalor_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalas_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalland_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evallor_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL sfa_evalcond_operand(struct string_format_advanced *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs);

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) Dee_ssize_t DFCALL
string_format_advanced_do1(struct string_format_advanced *__restrict self);


/* Parse and print an advanced, extended format-string text element with escapes.
 * PATTERN: "foo = {foo:x<{width}} after"
 * IN: -----------------^ ^       ^
 * IN(next_brace): -------+       |
 * OUT: --------------------------+
 * @return: * : Dee_formatprinter_t-compatible return value */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DFCALL
string_format_advanced_do_format_ex_with_escapes(struct string_format_advanced *__restrict self,
                                                 DeeObject *__restrict value,
                                                 char const *next_brace) {
	Dee_ssize_t result;
	struct unicode_printer format;
	DREF DeeObject *format_str;
	unicode_printer_init(&format);
again_handle_next_brace:
	if unlikely(unicode_printer_printutf8(&format, self->sfa_parser.sfp_iter,
	                                      (size_t)(next_brace - self->sfa_parser.sfp_iter)) < 0)
		goto err_format_printer;
	ASSERT(*next_brace == '{' || *next_brace == '}');

	/* Check for escaped {{ or }} */
	if unlikely((next_brace + 1) < self->sfa_parser.sfp_wend &&
	            (next_brace[0] == next_brace[1])) {
		self->sfa_parser.sfp_iter = next_brace + 1;
		next_brace = find_next_brace(self->sfa_parser.sfp_iter + 1,
		                             self->sfa_parser.sfp_wend);
again_handle_next_brace_nullable:
		if unlikely(!next_brace) {
			err_unmatched_lbrace_in_advanced();
			goto err_format_printer;
		}
		goto again_handle_next_brace;
	}

	if (*next_brace == '{') {
		/* Recursively emplaced object format (within the "format"-printer itself) */
		Dee_formatprinter_t saved_sfa_printer;
		void *saved_sfa_arg;
		saved_sfa_printer = self->sfa_printer;
		saved_sfa_arg     = self->sfa_arg;
		self->sfa_printer = &unicode_printer_print;
		self->sfa_arg     = &format;

		/* Print object repr into the "format"-printer */
		self->sfa_parser.sfp_iter = next_brace + 1;
		sfa_yield(self); /* Yield first token */
		result = string_format_advanced_do1(self);
		if unlikely(result < 0)
			goto err_format_printer;

		/* Restore context */
		self->sfa_printer = saved_sfa_printer;
		self->sfa_arg     = saved_sfa_arg;

		/* Find, and handle the next brace */
		next_brace = find_next_brace(self->sfa_parser.sfp_iter,
		                             self->sfa_parser.sfp_wend);
		goto again_handle_next_brace_nullable;
	}

	ASSERT(*next_brace == '}'); /* Known to be unescaped -> end of argument */
	self->sfa_parser.sfp_iter = next_brace + 1;
	format_str = unicode_printer_pack(&format);
	if unlikely(!format_str)
		goto err;
	result = DeeObject_PrintFormat(value, self->sfa_printer, self->sfa_arg, format_str);
	Dee_Decref(format_str);
	return result;
err_format_printer:
	unicode_printer_fini(&format);
err:
	return -1;
}

/* Parse and print an advanced, extended format-string text element.
 * PATTERN: "foo = {foo:x<{width}} after"
 * IN: -----------------^         ^
 * OUT: --------------------------+
 * @return: * : Dee_formatprinter_t-compatible return value */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) Dee_ssize_t DFCALL
string_format_advanced_do_format_ex(struct string_format_advanced *__restrict self,
                                    DeeObject *__restrict value) {
	Dee_ssize_t result;
	char const *next_brace;
	next_brace = find_next_brace(self->sfa_parser.sfp_iter,
	                             self->sfa_parser.sfp_wend);
	if unlikely(!next_brace)
		return err_unmatched_lbrace_in_advanced();

	/* Check if the format string contains escapes or recursively emplaced arguments. */
	if unlikely(((next_brace + 1) < self->sfa_parser.sfp_wend && next_brace[0] == next_brace[1]) ||
	            (*next_brace == '{'))
		return string_format_advanced_do_format_ex_with_escapes(self, value, next_brace);

	/* Simple case: format string can be used as-is. */
	ASSERT(*next_brace == '}');
	result = DeeObject_PrintFormatString(value, self->sfa_printer, self->sfa_arg,
	                                     self->sfa_parser.sfp_iter,
	                                     (size_t)(next_brace - self->sfa_parser.sfp_iter));
	self->sfa_parser.sfp_iter = next_brace + 1;
	return result;
}

/* Parse and print an advanced format-string text element.
 * PATTERN: "foo = {foo + $1!r} after"
 * IN: -------------^          ^
 * OUT: -----------------------+
 * @return: * : Dee_formatprinter_t-compatible return value */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) Dee_ssize_t DFCALL
string_format_advanced_do1(struct string_format_advanced *__restrict self) {
	Dee_ssize_t result;
	DREF DeeObject *value;

	/* Evaluate expression */
	value = sfa_evalcond(self);
	if unlikely(!value)
		goto err;
	if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend) {
		err_unmatched_lbrace_in_advanced();
		goto err_value;
	}

	/* Check what sort of object representation mode is being used. */
	switch (__builtin_expect(self->sfa_exprtok, '}')) {
	case '}':
		result = DeeObject_Print(value, self->sfa_printer, self->sfa_arg);
		break;

	case '!':
		if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend) {
			err_unknown_repr_mode_in_advanced("");
			goto err_value;
		}
		switch (__builtin_expect(*self->sfa_parser.sfp_iter, 'r')) {
		case 'r':
			++self->sfa_parser.sfp_iter;
			if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend) {
				err_invalid_char_after_lbrace_exclaim_spec_in_advanced('r', "");
				goto err_value;
			}
			if unlikely(*self->sfa_parser.sfp_iter != '}') {
				err_invalid_char_after_lbrace_exclaim_spec_in_advanced('r', self->sfa_parser.sfp_iter);
				goto err_value;
			}
			++self->sfa_parser.sfp_iter;
			result = DeeObject_PrintRepr(value, self->sfa_printer, self->sfa_arg);
			break;
		case 's':
		case 'a':
			++self->sfa_parser.sfp_iter;
			if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend) {
				err_invalid_char_after_lbrace_exclaim_spec_in_advanced(self->sfa_parser.sfp_iter[-1], "");
				goto err_value;
			}
			if unlikely(*self->sfa_parser.sfp_iter != '}') {
				err_invalid_char_after_lbrace_exclaim_spec_in_advanced(self->sfa_parser.sfp_iter[-1],
				                                                       self->sfa_parser.sfp_iter);
				goto err_value;
			}
			++self->sfa_parser.sfp_iter;
			result = DeeObject_Print(value, self->sfa_printer, self->sfa_arg);
			break;
		default:
			return err_unknown_repr_mode_in_advanced(self->sfa_parser.sfp_iter);
		}
		break;

	case ':':
		result = string_format_advanced_do_format_ex(self, value);
		break;

	default:
		self->sfa_parser.sfp_iter -= sfa_tok_getlen_for_rewind(self);
		err_invalid_char_after_expr_in_advanced(self->sfa_parser.sfp_iter);
		goto err_value;
	}
	Dee_Decref(value);
	return result;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DFCALL
string_format_advanced_skip1(struct string_format_advanced *__restrict self) {
	if unlikely(sfa_skipcond(self))
		goto err;
	if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend)
		return err_unmatched_lbrace_in_advanced();
	switch (__builtin_expect(*self->sfa_parser.sfp_iter, '}')) {
	case '}':
		++self->sfa_parser.sfp_iter;
		break;
	case '!':
		++self->sfa_parser.sfp_iter;
		if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend)
			return err_unknown_repr_mode_in_advanced("");
		switch (__builtin_expect(*self->sfa_parser.sfp_iter, 'r')) {
		case 'r':
			++self->sfa_parser.sfp_iter;
			if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend)
				return err_invalid_char_after_lbrace_exclaim_spec_in_advanced('r', "");
			if unlikely(*self->sfa_parser.sfp_iter != '}')
				return err_invalid_char_after_lbrace_exclaim_spec_in_advanced('r', self->sfa_parser.sfp_iter);
			++self->sfa_parser.sfp_iter;
			break;
		case 's':
		case 'a':
			++self->sfa_parser.sfp_iter;
			if unlikely(self->sfa_parser.sfp_iter >= self->sfa_parser.sfp_wend)
				return err_invalid_char_after_lbrace_exclaim_spec_in_advanced(self->sfa_parser.sfp_iter[-1], "");
			if unlikely(*self->sfa_parser.sfp_iter != '}')
				return err_invalid_char_after_lbrace_exclaim_spec_in_advanced(self->sfa_parser.sfp_iter[-1],
				                                                              self->sfa_parser.sfp_iter);
			++self->sfa_parser.sfp_iter;
			break;
		default:
			return err_unknown_repr_mode_in_advanced(self->sfa_parser.sfp_iter);
		}
		break;

	case ':':
		++self->sfa_parser.sfp_iter;
		break;

	default:
		return err_invalid_char_after_expr_in_advanced(self->sfa_parser.sfp_iter);
	}
	return 0;
err:
	return -1;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
string_format_advanced_do1_intostr(struct string_format_advanced *__restrict self) {
	Dee_formatprinter_t saved_sfa_printer;
	void *saved_sfa_arg;
	struct unicode_printer printer;
	Dee_ssize_t temp;
	unicode_printer_init(&printer);
	saved_sfa_printer = self->sfa_printer;
	saved_sfa_arg     = self->sfa_arg;
	self->sfa_printer = &unicode_printer_print;
	self->sfa_arg     = &printer;
	temp = string_format_advanced_do1(self);
	self->sfa_printer = saved_sfa_printer;
	self->sfa_arg     = saved_sfa_arg;
	if unlikely(temp < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}





#ifndef CONFIG_EXPERIMENTAL_NEW_STRING_FORMAT
/* TODO: Re-write this file such that "string.format" uses `DeeObject_Foreach()'.
 *       For this purpose, scan ahead until the first '{'. If it is followed by
 *       '}' or '!', assume that the format string is "simple", meaning that
 *       every input parameter is only ever used once.
 *
 * TODO: Get rid of ':' and "DeeObject_PrintFormat()" (or at least re-work them)
 */

struct formatter {
	/* TODO: Unicode support */
	char           *f_iter;        /* [1..1] Current format string position. */
	char           *f_end;         /* [1..1] Format string end. */
	char           *f_flush_start; /* [1..1] Address where string flushing should start. */
	DREF DeeObject *f_seqiter;     /* [null_if(f_seqsize != DEE_FASTSEQ_NOTFAST_DEPRECATED)][0..1] The iterator used to get sequence-elements from `f_args' */
	DeeObject      *f_args;        /* [1..1][const] A user-given sequence object used to index format arguments. */
	size_t          f_seqsize;     /* [const] Fast sequence size of `f_args'. */
	size_t          f_seqindex;    /* [valid_if(f_seqsize != DEE_FASTSEQ_NOTFAST_DEPRECATED)] Fast sequence  */
};

PRIVATE NONNULL((1, 2)) int DCALL
error_unused_format_string(char *start, char *end) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unused/unrecognized format string %$q in string.format",
	                       (size_t)(end - start), start);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetUnaryArg(struct formatter *__restrict self,
                      char **__restrict p_fmt_start,
                      bool do_eval) {
	/* TODO: Unicode support */
	char *fmt_start = *p_fmt_start;
	char ch         = *fmt_start;
	DREF DeeObject *result;
	if (DeeUni_IsSymStrt(ch)) {
		/* Dict-style key-lookup */
		char *key_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		while (DeeUni_IsSymCont(*key_end))
			++key_end;
		if (!do_eval) {
			result = Dee_None;
			Dee_Incref(result);
		} else {
			size_t len = (size_t)(key_end - fmt_start);
			result = DeeObject_GetItemStringLenHash(self->f_args,
			                                    fmt_start,
			                                    len,
			                                    Dee_HashPtr(fmt_start, len));
		}
		fmt_start = key_end;
	} else if (DeeUni_IsDigit(ch)) {
		/* list-style index lookup */
		DREF DeeObject *key;
		char *index_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		if (!do_eval) {
			while (DeeUni_IsSymCont(*index_end))
				++index_end;
			result = Dee_None;
			Dee_Incref(result);
		} else if (ch >= '0' && ch <= '9') {
			/* Optimization for ~normal~ indices. */
			unsigned int value, radix = 10;
			dssize_t new_index, index = ch - '0';
			ch = *index_end;
			if (ch == '0') {
				if (ch == 'b' || ch == 'B') {
					radix = 2;
					++index_end;
				} else if (ch == 'x' || ch == 'X') {
					radix = 16;
					++index_end;
				} else {
					radix = 8;
				}
			}
			for (;;) {
				ch = *index_end;
				if (!DeeUni_AsDigit(ch, 16, &value)) {
					/* Check for symbol characters not recognized in numbers. */
					if unlikely(DeeUni_IsSymCont(ch))
						goto do_variable_length_index;
					break;
				}
				/* Check for values illegal for the selected radix. */
				if unlikely(value >= radix)
					goto do_variable_length_index;
				/* Add values to the new index. */
				new_index = index * radix;
				new_index += value;
				/* Check for overflow (including the sign bit). */
				if unlikely(new_index <= index)
					goto do_variable_length_index;
				/* Use the new index from now on. */
				index = new_index;
				++index_end;
			}
			/* Do an integer-index lookup. */
			result = DeeObject_GetItemIndex(self->f_args, index);
		} else {
do_variable_length_index:
			index_end = fmt_start + 1;
			while (DeeUni_IsSymCont(*index_end))
				++index_end;
			key = DeeInt_FromString(fmt_start, index_end - fmt_start,
			                        DEEINT_STRING(0, DEEINT_STRING_FNORMAL));
			if unlikely(!key)
				goto err;
			result = DeeObject_GetItem(self->f_args, key);
			Dee_Decref(key);
		}
		fmt_start = index_end;
#if 0 /* TODO: Quoted strings? */
	} else if (ch == '\'') {
#endif
	} else if (!do_eval) {
		result = Dee_None;
		Dee_Incref(result);
	} else {
		/* Support for fast-sequence index access (primarily for Tuple and SharedVector,
		 * which are the most likely types of argument sequences used in format strings,
		 * including the case of template strings) */
		if (self->f_seqsize != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
			ASSERT(!self->f_seqiter);
			if unlikely(self->f_seqindex >= self->f_seqsize)
				goto err_not_enough_args;
			result = DeeFastSeq_GetItem_deprecated(self->f_args, self->f_seqindex);
			++self->f_seqindex;
		} else {
			/* Fallback: No key or index -> Yield the next iterator-item. */
			if (!self->f_seqiter &&
			    (self->f_seqiter = DeeObject_Iter(self->f_args)) == NULL)
				goto err;
			result = DeeObject_IterNext(self->f_seqiter);
			/* Check for iter-done */
			if unlikely(result == ITER_DONE)
				goto err_not_enough_args;
		}
	}
	*p_fmt_start = fmt_start;
	return result;
err_not_enough_args:
	DeeError_Throwf(&DeeError_UnpackError,
	                "Insufficient number of arguments");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Formatter_GetUnaryIndex(char **__restrict p_fmt_start) {
	char *fmt_start = *p_fmt_start;
	char ch         = *fmt_start;
	DREF DeeObject *result;
	if (DeeUni_IsSymStrt(ch)) {
		/* Dict-style key-lookup */
		char *key_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		while (DeeUni_IsSymCont(*key_end))
			++key_end;
		result = DeeString_NewSized(fmt_start,
		                            (size_t)(key_end - fmt_start));
		fmt_start = key_end;
	} else if (DeeUni_IsDigit(ch)) {
		/* list-style index lookup */
		char *index_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		index_end = fmt_start + 1;
		while (DeeUni_IsSymCont(*index_end))
			++index_end;
		result = DeeInt_FromString(fmt_start, index_end - fmt_start,
		                           DEEINT_STRING(0, DEEINT_STRING_FNORMAL));
		fmt_start = index_end;
	} else {
		char *end = fmt_start;
		while (*end && *end != '}')
			++end;
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid expression: %$q",
		                (size_t)(end - fmt_start), fmt_start);
		goto err;
	}
	*p_fmt_start = fmt_start;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Formatter_GetUnaryKey(char **__restrict p_fmt_start) {
	char *fmt_start = *p_fmt_start;
	char ch         = *fmt_start;
	DREF DeeObject *result;
	if (DeeUni_IsSymStrt(ch)) {
		/* Dict-style key-lookup */
		char *key_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		while (DeeUni_IsSymCont(*key_end))
			++key_end;
		result = DeeString_NewSized(fmt_start,
		                            (size_t)(key_end - fmt_start));
		fmt_start = key_end;
#if 0 /* TODO: Quoted strings? */
	} else if (ch == '\'') {
#endif
	} else {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid key expression: %:1q",
		                fmt_start);
		goto err;
	}
	*p_fmt_start = fmt_start;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetOne(struct formatter *__restrict self,
                 char **__restrict p_fmt_start,
                 bool do_eval);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetValue(struct formatter *__restrict self,
                   char **__restrict p_fmt_start,
                   bool do_eval) {
	char *fmt_end, *fmt_start = *p_fmt_start;
	DREF DeeObject *result;
	unsigned int recursion;
	ASSERT(*fmt_start == '{');
	/* Figure out where the format ends (the next, recursive `}') */
	++fmt_start;
	fmt_end   = fmt_start;
	recursion = 1;
	for (; fmt_end < self->f_end; ++fmt_end) {
		if (*fmt_end == '{') {
			++recursion;
		} else {
			if (*fmt_end == '}' && !--recursion)
				break;
		}
	}
	/* Load the format expression. */
	result = Formatter_GetOne(self, &fmt_start, do_eval);
	if unlikely(!result)
		goto err;
	if (fmt_start < fmt_end) {
		error_unused_format_string(fmt_start, fmt_end);
		goto err;
	}

	if (*fmt_end == '}')
		++fmt_end;
	*p_fmt_start = fmt_end;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetExpr(struct formatter *__restrict self,
                  char **__restrict p_fmt_start,
                  bool do_eval) {
	return **p_fmt_start == '{'
	       ? Formatter_GetValue(self, p_fmt_start, do_eval)
	       : Formatter_GetUnaryIndex(p_fmt_start);
}

#undef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
#ifndef CONFIG_NO_ALLOW_SPACE_IN_FORMAT_EXPRESSION
#define CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
#endif /* !CONFIG_NO_ALLOW_SPACE_IN_FORMAT_EXPRESSION */


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetOne(struct formatter *__restrict self,
                 char **__restrict p_fmt_start,
                 bool do_eval) {
	char *fmt_start;
	DREF DeeObject *result, *new_result;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
	ASSERT(!DeeUni_IsSpace('\0'));
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
	if (**p_fmt_start == '(') {
		/* Parenthesis around main argument. */
		fmt_start = *p_fmt_start;
		++fmt_start; /* Skip `(' */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		result = Formatter_GetOne(self, &fmt_start, do_eval);
		if unlikely(!result)
			goto err;
		if (*fmt_start != ')') {
			DeeError_Throwf(&DeeError_ValueError,
			                "Expected `)' after `(' in format expression, but got %:1q",
			                fmt_start);
			goto err_r;
		}
		++fmt_start; /* Skip `)' */
	} else {
		result = Formatter_GetUnaryArg(self, p_fmt_start, do_eval);
		if unlikely(!result)
			goto err;
		fmt_start = *p_fmt_start;
	}
next_suffix:
	/* Deal with item suffix modifiers (`foo[bar]', `foo(bar)', `foo.bar') */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
	while (DeeUni_IsSpace(*fmt_start))
		++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
	switch (*fmt_start) {

		case '[': {
		DREF DeeObject *index;
		/* Key / Index lookup. */
		++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		index = Formatter_GetExpr(self, &fmt_start, do_eval);
		if unlikely(!index)
			goto err_r;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == ':') {
			DREF DeeObject *index2;
			++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			index2 = Formatter_GetExpr(self, &fmt_start, do_eval);
			if unlikely(!index2) {
				Dee_Decref(index);
				goto err_r;
			}
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			if unlikely(*fmt_start != ']') {
				Dee_Decref(index2);
err_bad_index_expression:
				DeeError_Throwf(&DeeError_ValueError,
				                "Expected `]' after `[' in format expression, but got %:1q",
				                fmt_start);
				Dee_Decref(index);
				goto err_r;
			}
			new_result = DeeObject_GetRange(result, index, index2);
			Dee_Decref(index2);
		} else {
			if unlikely(*fmt_start != ']')
				goto err_bad_index_expression;
			new_result = DeeObject_GetItem(result, index);
		}
		Dee_Decref(index);
		if unlikely(!new_result)
			goto err_r;
		Dee_Decref(result);
		result = new_result;
		++fmt_start; /* Skip the trailing `]' character. */
		goto next_suffix;
	}	break;

	case '.': {
		DREF DeeObject *attr;
		/* Attribute lookup */
		++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == '{') {
			attr = Formatter_GetValue(self, &fmt_start, do_eval);
			if (attr && DeeObject_AssertTypeExact(attr, &DeeString_Type))
				Dee_Clear(attr);
		} else {
			attr = Formatter_GetUnaryKey(&fmt_start);
		}
		if unlikely(!attr)
			goto err_r;
		/* Do the attribute lookup. */
		new_result = DeeObject_GetAttr(result, attr);
		Dee_Decref(attr);
		if unlikely(!new_result)
			goto err_r;
		Dee_Decref(result);
		result = new_result;
		goto next_suffix;
	}	break;

	case '(': {
		DREF DeeObject *arg;
		struct objectlist args;
		/* Call the currently set result expression. */
		++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == ')') {
			/* Simple case: 0-argument call */
			new_result = DeeObject_Call(result, 0, NULL);
			if unlikely(!new_result)
				goto err_r;
			Dee_Decref(result);
			result = new_result;
			goto next_suffix;
		}
		arg = Formatter_GetExpr(self, &fmt_start, do_eval);
		if unlikely(!arg)
			goto err_r;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		objectlist_init(&args);
		if (*fmt_start != ',') {
			/* Another simple case: 1-argument call */
			if (fmt_start[0] == '.' &&
			    fmt_start[1] == '.' &&
			    fmt_start[2] == '.') {
				/* Varargs call with single argument. */
				DREF DeeObject *new_arg;
				char *after_dots = fmt_start + 3;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
				while (DeeUni_IsSpace(*after_dots))
					++after_dots;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
				if (after_dots[0] == ',') {
					size_t error;
					/* There are more arguments after this first one. */
					error = objectlist_extendseq(&args, arg);
					Dee_Decref(arg);
					if unlikely(error == (size_t)-1)
						goto err_r;
					fmt_start = after_dots + 1; /* Skip `,' */
					goto parse_second_argument;
				}
				if unlikely(after_dots[0] != ')') {
					fmt_start = after_dots;
					goto err_expected_rparen_arg;
				}
				new_arg = DeeTuple_FromSequence(arg);
				Dee_Decref(arg);
				if unlikely(!new_arg)
					goto err_call_argv;
				new_result = DeeObject_Call(result,
				                            DeeTuple_SIZE(new_arg),
				                            DeeTuple_ELEM(new_arg));
				Dee_Decref(new_arg);
				fmt_start = after_dots; /* Skip `...' */
			} else if (fmt_start[0] != ')') {
err_expected_rparen_arg:
				Dee_Decref(arg);
err_expected_rparen:
				DeeError_Throwf(&DeeError_ValueError,
				                "Expected `)' after `(' to complete call expression, but got %:1q",
				                fmt_start);
err_call_argv:
				objectlist_fini(&args);
				goto err_r;
			} else {
				new_result = DeeObject_Call(result, 1, &arg);
			}
			++fmt_start; /* Skip `)' */
			if unlikely(!new_result)
				goto err_r;
			Dee_Decref(result);
			result = new_result;
			goto next_suffix;
		}
		if (objectlist_append(&args, arg))
			goto err_call_argv;
		++fmt_start; /* Skip `,' */
parse_second_argument:
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		/* Parse additional arguments. */
		for (;;) {
			arg = Formatter_GetExpr(self, &fmt_start, do_eval);
			if unlikely(!arg)
				goto err_call_argv;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			if (fmt_start[0] == '.' &&
			    fmt_start[1] == '.' &&
			    fmt_start[2] == '.') {
				size_t extend_error;
				/* Expand argument list. */
				extend_error = objectlist_extendseq(&args, arg);
				Dee_Decref(arg);
				if unlikely(extend_error == (size_t)-1)
					goto err_call_argv;
				fmt_start += 3;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
				while (DeeUni_IsSpace(*fmt_start))
					++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			} else {
				/* Append to the argument list. */
				if (objectlist_append(&args, arg))
					goto err_call_argv;
			}
			if (*fmt_start != ',')
				break;
			++fmt_start; /* Skip `,' */
		}
		if (*fmt_start != ')')
			goto err_expected_rparen;
		++fmt_start; /* Skip `)' */
		/* Actually do the call. */
		new_result = DeeObject_Call(result, args.ol_elemc, args.ol_elemv);
		objectlist_fini(&args);
		/* Set the returned value as new result. */
		if unlikely(!new_result)
			goto err_r;
		Dee_Decref(result);
		result = new_result;
		goto next_suffix;
	}	break;

	case '?': {
		DREF DeeObject *tt, *ff;
		int is_true;
		/* Conditional expression:
		 * >> print "Hello {cond ? world : universe}".format({ .cond = true });  // Hello world
		 * >> print "Hello {cond ? world : universe}".format({ .cond = false }); // Hello universe
		 * NOTE: Also supports the `?:' extension that re-uses
		 *       the condition expression as the tt-, or ff-value
		 * NOTE: Also supports the missing-: extension that uses `none' for `ff' */
		/* Evaluate the condition. */
		is_true = DeeObject_Bool(result);
		if unlikely(is_true < 0)
			goto err_r;
		++fmt_start; /* Skip `?' */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == ':') {
			tt = result;
			Dee_Incref(tt);
		} else {
			tt = Formatter_GetExpr(self, &fmt_start, do_eval && is_true != 0);
			if unlikely(!tt)
				goto err_r;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		}
		if (*fmt_start == ':') {
			++fmt_start; /* Skip `?' */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			if (!*fmt_start || *fmt_start == '}') {
				/* Re-use the condition for `ff' */
				ff = result;
				Dee_Incref(ff);
			} else {
				ff = Formatter_GetExpr(self, &fmt_start, do_eval && is_true == 0);
				if unlikely(!ff) {
					Dee_Decref(tt);
					goto err_r;
				}
			}
		} else {
			/* No ff-branch given. */
			ff = Dee_None;
			Dee_Incref(ff);
		}
		/* Set the new result value. */
		Dee_Decref(result);
		if (is_true) {
			result = tt; /* Inherit reference */
			Dee_Decref(ff);
		} else {
			result = ff; /* Inherit reference */
			Dee_Decref(tt);
		}
		goto next_suffix;
	}	break;

	default: break;
	}

	*p_fmt_start = fmt_start;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}




PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
format_impl(struct formatter *__restrict self,
            dformatprinter printer, void *arg) {
	DREF DeeObject *in_arg;
#define print(p, s)                            \
	do {                                       \
		if unlikely((*printer)(arg, p, s) < 0) \
			goto err;                          \
	}	__WHILE0
	ASSERT(!*self->f_end);
	while (self->f_iter < self->f_end) {
		char *format_start;
		char *format_end;
		char ch = *self->f_iter++;
		unsigned int recursion;
		dssize_t print_error;
		if (ch == '}' && *self->f_iter == '}') {
			/* Replace `}}' with `}' */
			ASSERT(self->f_iter != self->f_end);
			print(self->f_flush_start, (size_t)((self->f_iter - 1) - self->f_flush_start));
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		if (ch != '{')
			continue;
		/* Flush everything up to this point. */
		print(self->f_flush_start, (size_t)((self->f_iter - 1) - self->f_flush_start));
		if (*self->f_iter == '{') {
			/* Replace `{{' with `{' */
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		format_start = self->f_iter;
		/* Figure out where the format ends (the next, recursive `}') */
		format_end = self->f_iter;
		recursion  = 1;
		for (; format_end < self->f_end; ++format_end) {
			if (*format_end == '{') {
				++recursion;
			} else {
				if (*format_end == '}' && !--recursion)
					break;
			}
		}
		/* Process the format string to extract an argument. */
		in_arg = Formatter_GetOne(self, &format_start, true);
		if unlikely(!in_arg)
			goto err;
		if (*format_start == '!') {
			/* Explicit format mode. */
			char mode = *++format_start;
			if (mode == 'a' || mode == 's')
				goto print_normal;
			if (mode == 'r') {
				print_error = DeeObject_PrintRepr(in_arg, printer, arg);
			} else {
				/* TODO: This error message doesn't handle unicode! */
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid character %.1q following `!' in string.format",
				                format_start);
				goto err_arg;
			}
			++format_start;
		} else if (*format_start == ':') {
			/* Format according to the format string. */
			char *content_start = format_start + 1;
			char *content_end   = format_end;
			char *content_iter;
			/* Check if the format string contains additional format commands. */
			content_iter = content_start;
			for (; content_iter < content_end; ++content_iter) {
				if (*content_iter == '{' || *content_iter == '}') {
					/* Special format-string pre-processing is required. */
					struct formatter inner_formatter;
					struct unicode_printer format_string_printer = UNICODE_PRINTER_INIT;
					DREF DeeObject *format_string;
					inner_formatter.f_iter        = content_iter;
					inner_formatter.f_end         = content_end;
					inner_formatter.f_flush_start = content_start;
					inner_formatter.f_seqiter     = self->f_seqiter;
					inner_formatter.f_args        = self->f_args;
					inner_formatter.f_seqsize     = self->f_seqsize;
					inner_formatter.f_seqindex    = self->f_seqindex;

					/* Format the format string, thus allowing it
					 * to contain text from input arguments. */
					print_error = format_impl(&inner_formatter,
					                          &unicode_printer_print,
					                          &format_string_printer);
					self->f_seqiter  = inner_formatter.f_seqiter;
					self->f_seqindex = inner_formatter.f_seqindex;
					if unlikely(print_error < 0) {
						unicode_printer_fini(&format_string_printer);
						goto err_arg;
					}
					format_string = unicode_printer_pack(&format_string_printer);
					if unlikely(!format_string)
						goto err_arg;
					/* Now use the generated format-string to format the input argument. */
					print_error = DeeObject_PrintFormat(in_arg, printer, arg, format_string);
					Dee_Decref(format_string);
					goto check_print_error;
				}
			}
			/* No pre-processing required -> Just format the object as it is right now! */
			print_error = DeeObject_PrintFormatString(in_arg, printer, arg, content_start,
			                                          (size_t)(content_end - content_start));
		} else {
print_normal:
			print_error = DeeObject_Print(in_arg, printer, arg);
		}
check_print_error:
		Dee_Decref(in_arg);
		if unlikely(print_error < 0)
			goto err;
		ASSERT(format_start <= format_end);
		if (format_start < format_end) {
			error_unused_format_string(format_start, format_end);
			goto err;
		}
		if (format_end >= self->f_end)
			break; /* The format string ends here. */
		self->f_iter        = format_end + 1;
		self->f_flush_start = self->f_iter;
	}

	/* Flush the remainder. */
	return (int)(*printer)(arg, self->f_flush_start,
	                       (size_t)(self->f_iter -
	                                self->f_flush_start));
#undef print
err_arg:
	Dee_Decref(in_arg);
err:
	return -1;
}


/* Format a given `format' string subject to {}-style formatting rules.
 * NOTE: This is the function called by `.format' for strings.
 * @param: args: A sequence (usually a Dict, or a List) used for
 *               providing input values to the format string. */
INTERN WUNUSED NONNULL((1, 3, 5)) dssize_t DCALL
DeeString_Format_old(dformatprinter printer, void *arg,
                     /*utf-8*/ char const *__restrict format,
                     size_t format_len, DeeObject *__restrict args) {
	struct formatter self;
	dssize_t result;
	self.f_flush_start = (char *)format;
	self.f_iter        = (char *)format;
	self.f_end         = (char *)format + format_len;
	self.f_seqiter     = NULL;
	self.f_args        = args;
	self.f_seqsize     = DeeFastSeq_GetSize_deprecated(args);
	self.f_seqindex    = 0;
	result             = format_impl(&self, printer, arg);
	Dee_XDecref(self.f_seqiter);
	return result;
}



PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
format_bytes_impl(struct formatter *__restrict self,
                  dformatprinter printer,
                  dformatprinter format_printer,
                  void *arg) {
	dssize_t temp, result = 0;
	DREF DeeObject *in_arg;
#define print_format(p, s)                                     \
	do {                                                       \
		if unlikely((temp = (*format_printer)(arg, p, s)) < 0) \
			goto err;                                          \
		result += temp;                                        \
	}	__WHILE0
	while (self->f_iter < self->f_end) {
		char *format_start;
		char *format_end;
		char ch = *self->f_iter++;
		unsigned int recursion;
		dssize_t print_error;
		if (ch == '}' &&
		    self->f_iter < self->f_end &&
		    *self->f_iter == '}') {
			/* Replace `}}' with `}' */
			print_format(self->f_flush_start,
			             (size_t)((self->f_iter - 1) - self->f_flush_start));
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		if (ch != '{')
			continue;
		if (self->f_iter == self->f_end)
			break;
		/* Flush everything up to this point. */
		print_format(self->f_flush_start,
		             (size_t)((self->f_iter - 1) - self->f_flush_start));
		if (*self->f_iter == '{') {
			/* Replace `{{' with `{' */
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		format_start = self->f_iter;
		/* Figure out where the format ends (the next, recursive `}') */
		format_end = self->f_iter;
		recursion  = 1;
		for (; format_end < self->f_end; ++format_end) {
			if (*format_end == '{') {
				++recursion;
			} else {
				if (*format_end == '}' && !--recursion)
					break;
			}
		}
		/* Process the format string to extract an argument. */
		in_arg = Formatter_GetOne(self, &format_start, true);
		if unlikely(!in_arg)
			goto err;
		if (*format_start == '!') {
			/* Explicit format mode. */
			char mode = *++format_start;
			if (mode == 'a' || mode == 's')
				goto print_normal;
			if (mode == 'r') {
				print_error = DeeObject_PrintRepr(in_arg, printer, arg);
			} else {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid character %.1q following `!' in string.format",
				                format_start);
				goto err_arg;
			}
			++format_start;
		} else if (*format_start == ':') {
			/* Format according to the format string. */
			char *content_start = format_start + 1;
			char *content_end   = format_end;
			char *content_iter;
			/* Check if the format string contains additional format commands. */
			content_iter = content_start;
			for (; content_iter < content_end; ++content_iter) {
				if (*content_iter == '{' || *content_iter == '}') {
					/* Special format-string pre-processing is required. */
					struct formatter inner_formatter;
					struct unicode_printer format_string_printer = UNICODE_PRINTER_INIT;
					DREF DeeObject *format_string;
					inner_formatter.f_iter        = content_iter;
					inner_formatter.f_end         = content_end;
					inner_formatter.f_flush_start = content_start;
					inner_formatter.f_seqiter     = self->f_seqiter;
					inner_formatter.f_args        = self->f_args;
					inner_formatter.f_seqindex    = self->f_seqindex;
					inner_formatter.f_seqsize     = self->f_seqsize;

					/* Format the format string, thus allowing it
					 * to contain text from input arguments. */
					print_error = format_bytes_impl(&inner_formatter,
					                                &unicode_printer_print,
					                                &unicode_printer_print,
					                                &format_string_printer);
					self->f_seqiter  = inner_formatter.f_seqiter;
					self->f_seqindex = inner_formatter.f_seqindex;
					if unlikely(print_error < 0) {
						unicode_printer_fini(&format_string_printer);
						goto err_arg;
					}
					format_string = unicode_printer_pack(&format_string_printer);
					if unlikely(!format_string)
						goto err_arg;
					/* Now use the generated format-string to format the input argument. */
					print_error = DeeObject_PrintFormat(in_arg, printer, arg, format_string);
					Dee_Decref(format_string);
					goto check_print_error;
				}
			}
			/* No pre-processing required -> Just format the object as it is right now! */
			print_error = DeeObject_PrintFormatString(in_arg, printer, arg, content_start,
			                                          (size_t)(content_end - content_start));
		} else {
print_normal:
			print_error = DeeObject_Print(in_arg, printer, arg);
		}
check_print_error:
		Dee_Decref(in_arg);
		if unlikely(print_error < 0)
			goto err;
		ASSERT(format_start <= format_end);
		if (format_start < format_end) {
			error_unused_format_string(format_start, format_end);
			goto err;
		}
		if (format_end >= self->f_end)
			break; /* The format string ends here. */
		self->f_iter        = format_end + 1;
		self->f_flush_start = self->f_iter;
	}
	/* Flush the remainder. */
	print_format(self->f_flush_start,
	             (size_t)(self->f_iter -
	                      self->f_flush_start));
	return result;
#undef print_format
err_arg:
	Dee_Decref(in_arg);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 4, 6)) dssize_t DCALL
DeeBytes_Format_old(dformatprinter printer,
                    dformatprinter format_printer, void *arg,
                    char const *__restrict format,
                    size_t format_len, DeeObject *__restrict args) {
	struct formatter self;
	dssize_t result;
	self.f_flush_start = (char *)format;
	self.f_iter        = (char *)format;
	self.f_end         = (char *)format + format_len;
	self.f_seqiter     = NULL;
	self.f_args        = args;
	self.f_seqsize     = DeeFastSeq_GetSize_deprecated(args);
	self.f_seqindex    = 0;
	result             = format_bytes_impl(&self, printer, format_printer, arg);
	Dee_XDecref(self.f_seqiter);
	return result;
}
#endif /* !CONFIG_EXPERIMENTAL_NEW_STRING_FORMAT */

DECL_END


#ifndef __INTELLISENSE__
#define DEFINE_DeeString_FormatPrinter
#include "format-impl.c.inl"
#define DEFINE_DeeString_FormatWStr
#include "format-impl.c.inl"
#define DEFINE_DeeString_Format
#include "format-impl.c.inl"

#define DEFINE_sfa_skipexpr
#include "format-expr.c.inl"
#define DEFINE_sfa_evalexpr
#include "format-expr.c.inl"
#endif /* !__INTELLISENSE__ */


#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_FORMAT_C */
