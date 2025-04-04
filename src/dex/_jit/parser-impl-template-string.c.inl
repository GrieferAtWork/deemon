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
#ifdef __INTELLISENSE__
#include "parser-impl.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>

DECL_BEGIN

/* Parse a template string */
#ifdef JIT_EVAL
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalTemplateString(JITLexer *__restrict self)
#else /* JIT_EVAL */
INTERN WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipTemplateString(JITLexer *__restrict self)
#endif /* !JIT_EVAL */
{
	unsigned char quote;
	unsigned char const *text_iter;
	IF_EVAL(unsigned char const *flush_start);
	IF_EVAL(struct unicode_printer printer = UNICODE_PRINTER_INIT);
parse_current_token_as_template_string:
	ASSERT(self->jl_tok == JIT_KEYWORD);
	ASSERT(self->jl_tokstart[0] == 'f' || self->jl_tokstart[0] == 'F');
#if 0 /* Fun fact: this right here breaks the MSVC compiler (lol; what a piece of junk...) */
	ASSERT(self->jl_tokstart[1] == '\'' || self->jl_tokstart[1] == '\"');
#else
	ASSERT(self->jl_tokstart[1] == '\"' || self->jl_tokstart[1] == '\'');
#endif
	ASSERT(self->jl_tokstart[0] == self->jl_tokend[-1]);
	quote     = self->jl_tokstart[1];
	text_iter = self->jl_tokstart + 2;
	IF_EVAL(flush_start = text_iter);

	/* Parse format string */
	for (;;) {
		unsigned char ch;
		ch = *text_iter++;
		switch (ch) {

		case '\"':
		case '\'':
			if (ch == quote)
				goto done_string;
			break;

		case '\0':
			if (text_iter >= self->jl_end) {
				syn_template_string_unterminated(self);
				goto err;
			}
			break;

		case '{': {
			RETURN_TYPE expr;

			/* Flush template text until before the '{' */
#ifdef JIT_EVAL
			if unlikely(unicode_printer_print(&printer, (char const *)flush_start,
			                                  (size_t)((text_iter - 1) - flush_start)) < 0)
				goto err;
#endif /* JIT_EVAL */

			/* Check for escaped '{' */
			if (*text_iter == '{') {
				IF_EVAL(flush_start = text_iter); /* Include 1 of the '{' in the next flush */
				++text_iter;
				break;
			}

			/* Parse an expression at this position. */
			JITLexer_YieldAt(self, text_iter);
			expr = CALL_PRIMARYF(Expression, LOOKUP_SYM_NORMAL);
			if unlikely(ISERR(expr))
				goto err;
			LOAD_LVALUE(expr, err);
			if (self->jl_tok == '!' || self->jl_tok == ':') {
				unsigned char const *spec;
				spec = self->jl_tokstart;
#ifdef JIT_EVAL
				/* Mirror what is done in `/src/deemon/objects/unicode/format.c:format_impl' */
				if (*spec == '!') {
					unsigned char mode;
					mode = *++spec;
					if (mode == 'a' || mode == 's') {
						if unlikely(unicode_printer_printobject(&printer, expr) < 0)
							goto err_expr;
					} else if (mode == 'r') {
						if unlikely(unicode_printer_printobjectrepr(&printer, expr) < 0)
							goto err_expr;
					} else {
						DeeError_Throwf(&DeeError_ValueError,
						                "Invalid character %.1q following `!' in string.format",
						                spec);
err_expr:
						Dee_Decref(expr);
						goto err;
					}
					++spec;
				} else {
					unsigned char const *args_start;
					ASSERT(*spec == ':');
					args_start = ++spec;
					/* TODO: This needs to support recursive '{' + '}' pairs! */
					/* TODO: This needs to support \-escape sequences! */
					spec = (unsigned char const *)memchr(spec, '}', (size_t)(self->jl_end - spec));
					if unlikely(!spec)
						goto err_expr_unmatched_lbrace;
					if (DeeObject_PrintFormatString(expr, &unicode_printer_print, &printer,
					                                (char const *)args_start,
					                                (size_t)(spec - args_start)) < 0)
						goto err_expr;
				}
				if unlikely(*spec != '}') {
err_expr_unmatched_lbrace:
					self->jl_tokstart = text_iter - 1;
					syn_template_string_unmatched_lbrace(self);
					goto err_expr;
				}
#else /* JIT_EVAL */
				/* TODO: This needs to support recursive '{' + '}' pairs! */
				spec = (unsigned char const *)memchr(spec, '}', (size_t)(self->jl_end - spec));
				if unlikely(!spec) {
					self->jl_tokstart = text_iter - 1;
					syn_template_string_unmatched_lbrace(self);
					goto err;
				}
#endif /* !JIT_EVAL */

				++spec; /* Include the trailing '}' */
				DECREF(expr);
				text_iter = spec;
			} else if (self->jl_tok == '}') {
#ifdef JIT_EVAL
				/* Simple case: no format pattern addend, so can simply print as-is */
				if unlikely(unicode_printer_printobject(&printer, expr) < 0)
					goto err_expr;
#endif /* JIT_EVAL */
				DECREF(expr);
				text_iter = self->jl_tokend;
			} else {
				DECREF(expr);
				self->jl_tokstart = text_iter - 1;
				syn_template_string_unmatched_lbrace(self);
				goto err;
			}
			IF_EVAL(flush_start = text_iter);
		}	break;

		case '}':
			if (*text_iter == '}') {
				/* Escaped '}' */
#ifdef JIT_EVAL
				if unlikely(unicode_printer_print(&printer, (char const *)flush_start,
				                                  (size_t)(text_iter - flush_start)) < 0)
					goto err;
#endif /* JIT_EVAL */
				++text_iter;
				IF_EVAL(flush_start = text_iter);
				break;
			}
			/* Error: unmatched '}' */
			self->jl_tokstart = text_iter - 1;
			syn_template_string_unmatched_rbrace(self);
			goto err;

		case '\\': {
#ifdef JIT_EVAL
			if unlikely(unicode_printer_print(&printer, (char const *)flush_start,
			                                  (size_t)((text_iter - 1) - flush_start)) < 0)
				goto err;
#endif /* JIT_EVAL */

			/* Do normal backslash escaping (including \{ --> { and \} --> }) */
			ch = *text_iter++;
			switch (ch) {

			case '\\': /* Escaped the following character itself. */
			case '\'':
			case '\"':
			case '?':
			case '{':
			case '}':
				break;

			case '\r':
				if (*text_iter == '\n')
					++text_iter;
				ATTR_FALLTHROUGH
			case '\n':
				goto after_escaped_putc; /* Escaped line-feed */

			case 'a': ch = (char)(unsigned char)0x07; break;
			case 'b': ch = (char)(unsigned char)0x08; break;
			case 'f': ch = (char)(unsigned char)0x0c; break;
			case 'n': ch = (char)(unsigned char)0x0a; break;
			case 'r': ch = (char)(unsigned char)0x0d; break;
			case 't': ch = (char)(unsigned char)0x09; break;
			case 'v': ch = (char)(unsigned char)0x0b; break;
			case 'e': ch = (char)(unsigned char)0x1b; break;

			case 'U': {
				unsigned int count;
				unsigned int max_digits;
				IF_EVAL(uint32_t digit_value);
				max_digits = 8;
				goto parse_hex_integer;
			case 'u':
				max_digits = 4;
				goto parse_hex_integer;
			case 'x':
			case 'X':
				max_digits = (unsigned int)-1; /* Unlimited. (TODO: This is incorrect -- \x should encode actual bytes!) */
parse_hex_integer:
				count = 0;
				IF_EVAL(digit_value = 0);
				while (count < max_digits) {
					uint32_t ch32;
					uint8_t val;
					unsigned char const *old_iter;
					old_iter = text_iter;
					ch32     = unicode_readutf8_n(&text_iter, self->jl_end);
					if (!DeeUni_AsDigit(ch32, 16, &val)) {
						text_iter = old_iter;
						break;
					}
					(void)val;
					IF_EVAL(digit_value <<= 4);
					IF_EVAL(digit_value |= val);
					++count;
				}
				if (!count) {
					/* Error: "No digits, or hex-chars found after \\x, \\u or \\U" */
					self->jl_tokstart = text_iter - 1;
					syn_template_string_no_digit_or_hex_after_backslash_x_u_U(self);
					goto err;
				}
#ifdef JIT_EVAL
				if (unicode_printer_putc(&printer, digit_value))
					goto err;
#endif /* JIT_EVAL */
				goto after_escaped_putc;
			}	break;

			default: {
				IF_EVAL(uint32_t digit_value);
				if (ch >= '0' && ch <= '7') {
					unsigned int count;
					IF_EVAL(digit_value = (uint32_t)(ch - '0'));
parse_oct_integer:
					/* Octal-encoded integer. */
					count = 1;
					while (count < 3) {
						uint32_t ch32;
						uint8_t digit;
						unsigned char const *old_iter;
						old_iter = text_iter;
						ch32     = unicode_readutf8_n(&text_iter, self->jl_end);
						if (!DeeUni_AsDigit(ch32, 8, &digit)) {
							text_iter = old_iter;
							break;
						}
						IF_EVAL(digit_value <<= 3);
						IF_EVAL(digit_value |= digit);
						++count;
					}
#ifdef JIT_EVAL
					if (unicode_printer_putc(&printer, digit_value))
						goto err;
#endif /* JIT_EVAL */
					goto after_escaped_putc;
				}
				if ((unsigned char)ch >= 0xc0) {
					uint32_t ch32;
					struct unitraits const *desc;
					uint8_t digit;
					--text_iter;
					ch32 = unicode_readutf8_n(&text_iter, self->jl_end);
					desc = DeeUni_Descriptor(ch32);
					if (desc->ut_flags & UNICODE_ISLF)
						goto after_escaped_putc; /* Escaped line-feed */
					if (DeeUniTrait_AsDigit(desc, 8, &digit)) {
						/* Unicode digit character. */
						IF_EVAL(digit_value = digit);
						goto parse_oct_integer;
					}
				}

				/* Error: Unknown escape character "%c" % ch */
				self->jl_tokstart = text_iter - 1;
				syn_template_string_undefined_escape(self, (int)(unsigned int)ch);
				goto err;
			}	break;

			}
#ifdef JIT_EVAL
			if unlikely(unicode_printer_put8(&printer, ch))
				goto err;
#endif /* JIT_EVAL */
after_escaped_putc:
			IF_EVAL(flush_start = text_iter);
		}	break;

		default:
			break;
		}
	}

	/* Flush the remainder. */
done_string:
#ifdef JIT_EVAL
	if unlikely(unicode_printer_print(&printer, (char const *)flush_start,
	                                  (size_t)((text_iter - 1) - flush_start)) < 0)
		goto err;
#endif /* JIT_EVAL */

	/* Yield the next token */
	JITLexer_YieldAt(self, text_iter);

	/* Check if the next token is another template string. - If so, join the two! */
	if ((self->jl_tok == JIT_KEYWORD && (*self->jl_tokstart == 'f' || *self->jl_tokstart == 'F')) &&
	    (*self->jl_tokend == '\"' || *self->jl_tokend == '\''))
		goto parse_current_token_as_template_string; /* Join adjacent template strings */

	/* Pack everything together on success. */
	return IFELSE(unicode_printer_pack(&printer), 0);
err:
	IF_EVAL(unicode_printer_fini(&printer));
	return ERROR;
}

DECL_END
