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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_FORMAT_C
#define GUARD_DEEMON_OBJECTS_UNICODE_FORMAT_C 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/tuple.h>
/**/

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint32_t */

DECL_BEGIN

/*
 * =========================== Format strings ===========================
 *
 * These format patterns are used to drive template strings, as well
 * as provide for more advanced patterns. They come in 2 variants:
 * - Simple
 * - Advanced
 *
 * These patterns are mutually exclusive, allowing detection of which
 * one we're dealing with. Additionally, even though you might think
 * that you're allowed to mix these 2 variants, you can't. You must
 * always use either one, or the other for the *entire* format pattern.
 *
 *
 * Examples:
 * - Simple:
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
 * >>                             // The format string used CANNOT include other arguments.
 * >>                             (':' [TEXT...])
 * >>                         ]
 * >>                         '}'
 * >>                     ]
 * >>                     [SIMPLE_TEMPLATE];
 * >>
 * >> ADVANCED_TEMPLATE ::= [TEXT...]
 * >>                       [   // Start of template argument
 * >>                           '{'
 * >>                           // NOTE: Text making up "ARGUMENT_EXPR" here is always interpreted as utf-8,
 * >>                           //       even when the rest of the template string originates from Bytes.
 * >>                           ARGUMENT_EXPR
 * >>                           [
 * >>                               ('!' (
 * >>                                   'r' | // Insert `DeeObject_Repr()' of "ARGUMENT_EXPR"
 * >>                                   's' | // Insert `DeeObject_Str()' of "ARGUMENT_EXPR"
 * >>                                   'a'   // Insert `DeeObject_Str()' of "ARGUMENT_EXPR"
 * >>                               )) |
 * >>                               // Insert `DeeObject_PrintFormatString()' of "ARGUMENT_EXPR"
 * >>                               // The format string used CAN include other arguments (it is parsed as a recursive template string)
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
 * >> ARGUMENT_EXPR_CALL_ARGS ::= [',' ~~ (ARGUMENT_EXPR_EXP...)] [(
 * >>                                 [',']  // If unlabelled arguments were also used
 * >>                                 (',' ~~ (
 * >>                                     SYMBOL ':' ARGUMENT_EXPR
 * >>                                 )...)
 * >>                             ) | ('**' ARGUMENT_EXPR)];
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
	                       "Unmatched '{' in advanced format pattern");
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
PRIVATE WUNUSED NONNULL((1, 2)) char const *DCALL
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
#define RETURN4(x) do{ result = (x); goto done4; }__WHILE0
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
				if (nextch == '?')
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
						RETURN6(SFA_TOK_D_FALSE);
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
						RETURN4(SFA_TOK_D_STR);
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
	++ptr;
done4:
	++ptr;
done3:
	++ptr;
done2:
	++ptr;
	goto done1;
#undef RETURN0
#undef RETURN1
#undef RETURN2
#undef RETURN3
#undef RETURN4
#undef RETURN5
#undef RETURN6
#undef RETURN9
}

PRIVATE NONNULL((1)) size_t DFCALL
sfa_tok_string_getlen(struct string_format_advanced *__restrict self) {
	char const *iter = self->sfa_parser.sfp_iter;
	char quote = iter[-1];
	ASSERT(quote == '"' || quote == '\'');
	while (iter < self->sfa_parser.sfp_wend) {
		char ch = *iter;
		if (ch == '\\') {
			++iter;
			if (iter >= self->sfa_parser.sfp_wend)
				break;
			++iter;
		} else if (ch == quote) {
			break;
		} else {
			++iter;
		}
	}
	return (size_t)(iter - self->sfa_parser.sfp_iter);
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

PRIVATE NONNULL((1)) size_t DFCALL
sfa_tok_getlen_for_rewind(struct string_format_advanced *__restrict self) {
	size_t result = 1;
	switch (self->sfa_exprtok) {
	case SFA_TOK_KEYWORD:
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

PRIVATE NONNULL((1)) char const *DFCALL
sfa_tok_getbounds(struct string_format_advanced *__restrict self,
                  size_t *__restrict p_token_length) {
	char const *result;
	size_t length;
	switch (self->sfa_exprtok) {
	case SFA_TOK_KEYWORD:
		length = sfa_tok_keyword_getlen(self);
		result = self->sfa_parser.sfp_iter;
		break;
	case SFA_TOK_CHAR:
	case SFA_TOK_STRING:
		result = self->sfa_parser.sfp_iter - 1;
		length = sfa_tok_string_getlen(self) + 2;
		break;
	case SFA_TOK_INT:
		result = self->sfa_parser.sfp_iter;
		length = sfa_tok_int_getlen(self);
		break;
	case SFA_TOK_D_INT:
		result = self->sfa_parser.sfp_iter - 1;
		length = sfa_tok_int_getlen(self) + 1;
		break;
	default:
		length = sfa_tok_getlen_for_rewind(self);
		result = self->sfa_parser.sfp_iter - length;
		break;
	}
	if ((result + length) > self->sfa_parser.sfp_wend)
		length = (size_t)(self->sfa_parser.sfp_wend - result);
	*p_token_length = length;
	return result;
}

PRIVATE ATTR_COLD NONNULL((1)) int DFCALL
sfa_err_bad_token(struct string_format_advanced *__restrict self,
                  char const *expected_token) {
	size_t len;
	char const *tok = sfa_tok_getbounds(self, &len);
	return DeeError_Throwf(&DeeError_ValueError,
	                       *expected_token == '<'
	                       ? "Unexpected token %.?q in advanced format pattern expression when %s was expected"
	                       : "Unexpected token %.?q in advanced format pattern expression when %q was expected",
	                       len, tok, expected_token);
}

/* Parse arguments for a function call.
 * PATTERN: "foo = {foo.upper($2, end: $5).length} after"
 * IN: -----------------------^           ^
 * OUT: ----------------------------------+ */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DFCALL
sfa_evalcallargs(struct string_format_advanced *__restrict self, /*out*/ DREF DeeObject **p_kw);
PRIVATE WUNUSED NONNULL((1)) int DFCALL
sfa_skipcallargs(struct string_format_advanced *__restrict self);


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
	self->sfa_inparen = false;
	value = sfa_evalcond(self);
	if unlikely(!value)
		goto err;

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
	int temp;
	bool saved_sfa_inparen;
	saved_sfa_inparen = self->sfa_inparen;
	temp = sfa_skipcond(self);
	self->sfa_inparen = saved_sfa_inparen;
	if unlikely(temp)
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
	bool saved_sfa_inparen;
	struct unicode_printer printer;
	Dee_ssize_t temp;
	unicode_printer_init(&printer);
	saved_sfa_printer = self->sfa_printer;
	saved_sfa_arg     = self->sfa_arg;
	saved_sfa_inparen = self->sfa_inparen;
	self->sfa_printer = &unicode_printer_print;
	self->sfa_arg     = &printer;
	temp = string_format_advanced_do1(self);
	self->sfa_printer = saved_sfa_printer;
	self->sfa_arg     = saved_sfa_arg;
	self->sfa_inparen = saved_sfa_inparen;
	if unlikely(temp < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}

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
