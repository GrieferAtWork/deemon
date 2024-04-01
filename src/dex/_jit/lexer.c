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
#ifndef GUARD_DEX_JIT_LEXER_C
#define GUARD_DEX_JIT_LEXER_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>

DECL_BEGIN

INTERN void DFCALL
JITLexer_Yield(JITLexer *__restrict self) {
	uniflag_t flags;
	uint32_t ch32;
	unsigned char ch, *iter;
	iter = self->jl_tokend;
raw_again:
	self->jl_tokstart = iter;
	if unlikely(iter >= self->jl_end) {
		self->jl_tok    = TOK_EOF;
		self->jl_tokend = iter;
		return;
	}
	ch = *iter++;
	switch (ch) {

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		/* Scan an integer or floating point token. */
		self->jl_tok = TOK_INT;
		while (iter < self->jl_end) {
			unsigned char *start;
			ch = *iter++;
			if unlikely(ch > 0x7f) {
				/* UTF-8 sequence. */
				--iter;
				start = iter;
				ch32 = unicode_readutf8_n(&iter, self->jl_end);
handle_int_or_float_ch32:
				if (!(DeeUni_Flags(ch32) & (UNICODE_ISSYMCONT | UNICODE_ISDIGIT))) {
#ifdef CONFIG_HAVE_FPU
					if (ch32 == '.' && self->jl_tok == TOK_INT) {
						self->jl_tok = TOK_FLOAT;
						continue;
					}
#endif /* CONFIG_HAVE_FPU */
					iter = start;
					break;
				} else {
#ifdef CONFIG_HAVE_FPU
					if (((ch32 == 'p' || ch32 == 'P')) ||
					    ((ch32 == 'e' || ch32 == 'E') && self->jl_tok == TOK_FLOAT))
						goto handle_decimal_power_suffix;
#endif /* CONFIG_HAVE_FPU */
				}
			} else {
				if (!(DeeUni_Flags(ch) & (UNICODE_ISSYMCONT | UNICODE_ISDIGIT))) {
#ifdef CONFIG_HAVE_FPU
					if (ch == '.' && self->jl_tok == TOK_INT) {
						self->jl_tok = TOK_FLOAT;
						continue;
					}
#endif /* CONFIG_HAVE_FPU */
					--iter;
					break;
				} else {
#ifdef CONFIG_HAVE_FPU
					if (((ch == 'p' || ch == 'P')) ||
					    ((ch == 'e' || ch == 'E') && self->jl_tok == TOK_FLOAT)) {
handle_decimal_power_suffix:
						self->jl_tok = TOK_FLOAT;
						ch = *iter;
						if (ch == '+' || ch == '-') {
							++iter;
						} else if (ch > 0x7f) {
							start = iter;
							ch32 = unicode_readutf8_n(&iter, self->jl_end);
							if (ch32 == '+' || ch32 == '-') {
								/* Accept '+' or '-' after decimal power suffix. */
							} else {
								goto handle_int_or_float_ch32;
							}
						}
					}
#endif /* CONFIG_HAVE_FPU */
				}
			}
		}
		break;

	case '<':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_LOWER_EQUAL;
				break;
			}
			if (ch == '<') {
				++iter;
				if (likely(iter < self->jl_end) && *iter == '=') {
					self->jl_tok = TOK_SHL_EQUAL;
					++iter;
					break;
				}
				self->jl_tok = TOK_SHL;
				break;
			}
		}
		self->jl_tok = '<';
		break;

	case '>':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_GREATER_EQUAL;
				break;
			}
			if (ch == '>') {
				++iter;
				if (likely(iter < self->jl_end) && *iter == '=') {
					self->jl_tok = TOK_SHR_EQUAL;
					++iter;
					break;
				}
				self->jl_tok = TOK_SHR;
				break;
			}
		}
		self->jl_tok = '>';
		break;

	case '.':
		if (likely(iter + 1 < self->jl_end) &&
		    (iter[0] == '.' && iter[1] == '.')) {
			iter += 2;
			self->jl_tok = TOK_DOTS;
			break;
		}
		goto do_single;

	case '&':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '&') {
				++iter;
				self->jl_tok = TOK_LAND;
				break;
			}
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_AND_EQUAL;
				break;
			}
		}
		self->jl_tok = '&';
		break;

	case '|':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '|') {
				++iter;
				self->jl_tok = TOK_LOR;
				break;
			}
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_OR_EQUAL;
				break;
			}
		}
		self->jl_tok = '|';
		break;

	case '/':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_DIV_EQUAL;
				break;
			}
			if (ch == '*') {
				/* Skip a C-like comment. */
				++iter;
				while (iter < self->jl_end) {
					ch = *iter++;
					if (ch == '*') {
						if unlikely(iter >= self->jl_end)
							break;
						if (*iter == '/') {
							++iter;
							break;
						}
					}
				}
				goto again;
			}
			if (ch == '/') {
				/* Skip a C++-like comment. */
				++iter;
				while (iter < self->jl_end) {
					ch = *iter++;
					if (ch == '\n')
						break;
					if (ch == '\r') {
						if unlikely(iter >= self->jl_end)
							break;
						if (*iter == '\n')
							++iter;
						break;
					}
					if (ch > 0x7f) {
						--iter;
						ch32 = unicode_readutf8_n(&iter, self->jl_end);
						if (DeeUni_IsLF(ch32))
							break;
					}
				}
				goto again;
			}
		}
		self->jl_tok = '/';
		break;

	case '=':
		if (likely(iter < self->jl_end) && *iter == '=') {
			++iter;
			if (likely(iter < self->jl_end) && *iter == '=') {
				++iter;
				self->jl_tok = TOK_EQUAL3;
				break;
			}
			self->jl_tok = TOK_EQUAL;
			break;
		}
		goto do_single;
	case '!':
		if (likely(iter < self->jl_end) && *iter == '=') {
			++iter;
			if (likely(iter < self->jl_end) && *iter == '=') {
				++iter;
				self->jl_tok = TOK_NOT_EQUAL3;
				break;
			}
			self->jl_tok = TOK_NOT_EQUAL;
			break;
		}
		goto do_single;

	case '+':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '+') {
				++iter;
				self->jl_tok = TOK_INC;
				break;
			}
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_ADD_EQUAL;
				break;
			}
		}
		self->jl_tok = '+';
		break;
	case '-':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '>') {
				++iter;
				self->jl_tok = TOK_ARROW;
				break;
			}
			if (ch == '-') {
				++iter;
				self->jl_tok = TOK_DEC;
				break;
			}
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_SUB_EQUAL;
				break;
			}
		}
		self->jl_tok = '-';
		break;

	case '*':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_MUL_EQUAL;
				break;
			}
			if (ch == '*') {
				++iter;
				if (likely(iter < self->jl_end) && *iter == '=') {
					++iter;
					self->jl_tok = TOK_POW_EQUAL;
					break;
				}
				self->jl_tok = TOK_POW;
				break;
			}
		}
		self->jl_tok = '*';
		break;

	case '^':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_XOR_EQUAL;
				break;
			}
		}
		self->jl_tok = '^';
		break;

	case '%':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_MOD_EQUAL;
				break;
			}
		}
		self->jl_tok = '%';
		break;

	case ':':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '=') {
				++iter;
				self->jl_tok = TOK_COLON_EQUAL;
				break;
			}
		}
		self->jl_tok = ':';
		break;

	case '\'':
	case '\"':
		if unlikely(iter >= self->jl_end) {
			self->jl_tokstart = (unsigned char *)"\"\"";
			self->jl_tokend   = self->jl_tokstart + 2;
		} else {
			do {
				unsigned char ch2 = *iter++;
				if (ch2 == ch)
					break;
				if (ch2 == '\\' && iter < self->jl_end)
					++iter;
			} while likely(iter < self->jl_end);
		}
		self->jl_tok = JIT_STRING;
		break;

	case 'r':
		if likely(iter < self->jl_end) {
			ch = *iter;
			if (ch == '\'' || ch == '\"') {
				/* Raw string literal. */
				++iter;
				if unlikely(iter >= self->jl_end) {
					self->jl_tokstart = (unsigned char *)"r\"\"";
					self->jl_tokend   = self->jl_tokstart + 3;
				} else {
					while (iter < self->jl_end && *iter++ != ch)
						;
				}
				self->jl_tok = JIT_RAWSTRING;
				break;
			}
		}
		goto scan_keyword;

	case '?':
		if (*iter == '?') {
			++iter;
			self->jl_tok = TOK_QMARK_QMARK;
			break;
		}
		goto do_single;

	default:
		if unlikely(ch > 0x7f) {
			/* UTF-8 sequence. */
			--iter;
			ch32 = unicode_readutf8_n(&iter, self->jl_end);
			flags = DeeUni_Flags(ch32);
			if (flags & UNICODE_ISSPACE)
				goto again;
		}
		flags = DeeUni_Flags(ch);
		/* Skip whitespace. */
		if (flags & UNICODE_ISSPACE) {
	case ' ':
	case '\t':
	case '\r':
	case '\n':
again:
			self->jl_tokend = iter;
			goto raw_again;
		}
		if (flags & UNICODE_ISSYMSTRT) {
scan_keyword:
	case '_':
	case '$':
	case 'a':
	case 'A':
	case 'b':
	case 'B':
	case 'c':
	case 'C':
	case 'd':
	case 'D':
	case 'e':
	case 'E':
	case 'f':
	case 'F':
	case 'g':
	case 'G':
	case 'h':
	case 'H':
	case 'i':
	case 'I':
	case 'j':
	case 'J':
	case 'k':
	case 'K':
	case 'l':
	case 'L':
	case 'm':
	case 'M':
	case 'n':
	case 'N':
	case 'o':
	case 'O':
	case 'p':
	case 'P':
	case 'q':
	case 'Q': /*case 'r':*/
	case 'R':
	case 's':
	case 'S':
	case 't':
	case 'T':
	case 'u':
	case 'U':
	case 'v':
	case 'V':
	case 'w':
	case 'W':
	case 'x':
	case 'X':
	case 'y':
	case 'Y':
	case 'z':
	case 'Z':
			/* Scan a keyword. */
			while likely(iter < self->jl_end) {
				ch = *iter++;
				if unlikely(ch > 0x7f) {
					/* UTF-8 sequence. */
					unsigned char *start;
					--iter;
					start = iter;
					ch32 = unicode_readutf8_n(&iter, self->jl_end);
					if (!(DeeUni_Flags(ch32) & UNICODE_ISSYMCONT)) {
						iter = start;
						break;
					}
				} else {
					if (!(DeeUni_Flags(ch) & UNICODE_ISSYMCONT)) {
						--iter;
						break;
					}
				}
			}
			self->jl_tok = JIT_KEYWORD;
			break;
		}
		/* Fallback: single-character token. */
		ATTR_FALLTHROUGH
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
	case '@':
	case ',':
	case ';':
	case '~':
	case '#':
do_single:
		self->jl_tok = ch;
		break;
	}
	self->jl_tokend = iter;
}


DECL_END

#endif /* !GUARD_DEX_JIT_LEXER_C */
