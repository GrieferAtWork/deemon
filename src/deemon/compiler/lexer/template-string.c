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
#ifndef GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C
#define GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>

#include "../../runtime/strings.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *FCALL
add_getattr_format(struct ast *__restrict self) {
	DREF struct ast *attr, *result;
	attr = ast_constexpr((DeeObject *)&str_format);
	if unlikely(!attr)
		goto err;
	result = ast_operator2(OPERATOR_GETATTR,
	                       AST_OPERATOR_FNORMAL,
	                       self, attr);
	ast_decref_unlikely(attr);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *FCALL
add_call_sequence(struct ast *__restrict base, size_t argc,
                  /*inherit(on_success)*/ DREF struct ast **argv) {
	DREF struct ast *result, *args, *args_sequence;
	DREF struct ast **args_argv;
	args_sequence = ast_multiple(AST_FMULTIPLE_GENERIC, argc, argv);
	if unlikely(!args_sequence)
		goto err;
	args_argv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
	if unlikely(!args_argv)
		goto err_args_sequence;
	args_argv[0] = args_sequence; /* Inherit reference */
	args = ast_multiple(AST_FMULTIPLE_TUPLE, 1, args_argv);
	if unlikely(!args)
		goto err_args_sequence_args_argv;
	result = ast_operator2(OPERATOR_CALL, AST_OPERATOR_FNORMAL, base, args);
	ast_decref_unlikely(args);
	return result;
err_args_sequence_args_argv:
	Dee_Free(args_argv);
err_args_sequence:
	ast_decref_likely(args_sequence);
err:
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) char *FCALL
find_unescape_quote(char *text, char *end, char quote) {
	while (text < end) {
		char ch = *text++;
		if (ch == '\\') {
			/* Quotes can always be backslash-escaped! */
			if (text >= end)
				break;
			++text;
			continue;
		}
		if (ch == quote)
			return text - 1;
	}
	return NULL;
}


/* Parse a template string. */
INTERN WUNUSED DREF struct ast *FCALL
ast_parse_template_string(void) {
	STATIC_ASSERT(TOK_STRING == '"');
	STATIC_ASSERT(TOK_CHAR == '\'');
	size_t format_argc            = 0;
	size_t format_arga            = 0;
	DREF struct ast **format_argv = NULL;
	struct ast_loc loc;
	struct ast *result;
	struct unicode_printer format_printer = UNICODE_PRINTER_INIT;
	char *flush_start, *text_iter, *text_end, quote;
	loc_here(&loc);
parse_current_token_as_template_string:
	ASSERT(tok == TOK_STRING || tok == TOK_CHAR);
	ASSERT(token.t_begin < token.t_end);
	ASSERT(token.t_begin[0] == '\"' || token.t_begin[0] == '\'');
	ASSERT(token.t_begin[0] == token.t_end[-1]);
	quote       = token.t_begin[0];
	text_iter   = token.t_begin + 1;
	text_end    = token.t_end - 1;
	flush_start = text_iter;

	/* Parse format string */
	while (text_iter < text_end) {
		char ch = *text_iter++;
		switch (ch) {

		case '{': {
			DREF struct ast *expr_ast;
			uint32_t old_flags;
			if (*text_iter == '{') {
				/* Escaped '{' */
				++text_iter;
				break;
			}

			/* Flush template text until '{' */
			if unlikely(unicode_printer_print(&format_printer, flush_start,
			                                  (size_t)((text_iter - 1) - flush_start)) < 0)
				goto err;

			/* Parse an expression at this position. */
			token.t_file->f_pos = text_iter;

			/* Parse the expression. */
			old_flags = TPPLexer_Current->l_flags;
			text_iter = (char *)0 + (text_iter - token.t_file->f_begin); /* To deal with a buffer realloc() */
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_EXTENDFILE;       /* So we don't loose our file position */
			if unlikely(yield() < 0) {
err_old_flags:
				TPPLexer_Current->l_flags = old_flags;
				goto err;
			}
			expr_ast = ast_parse_expr(LOOKUP_SYM_NORMAL);
			if unlikely(!expr_ast)
				goto err_old_flags;
			TPPLexer_Current->l_flags = old_flags;
			text_iter = token.t_file->f_begin + (text_iter - (char *)0); /* To deal with a buffer realloc() */

			/* Ensure that TPP has loaded the file until the next unescaped quote
			 * This is required because tpp normally sees something like:
			 * >> f"foo: {a + '"' + b} -- more"
			 * as this:
			 * >> [f]["foo: {a + '"][' + b} -- more"...]
			 *                       ^-- ERROR: Unescaped quote
			 * 
			 * As such, it won't natively load the entire template-string token
			 * into its file buffer, meaning we have to trick it by forcing it
			 * to load file data until the next unescaped quotation mark matching
			 * the original quote:            v-- This one right here
			 * >> f"foo: {a + '"' + b} -- more"
			 *
			 * Note however that we might actually get a false positive here:
			 * >> f"foo: {a + '"' + b} -- {'"'} more"
			 *                              ^-- This counts as a hit, but isn't escaped
			 * But that's OK because it's part of another template argument expression,
			 * which will be parsed by `ast_parse_expr()', which in turn makes use of
			 * TPP's automatic file-chunk-extension function. The only thing we need to
			 * ensure at that point, is that TPP doesn't discard older parts of its file
			 * so our pointers don't get all out of wack, which we do by setting the
			 * lexer's `TPPLEXER_FLAG_EXTENDFILE' flag above, and converting our pointers
			 * into relative offsets above/after the parsing step (which is needed in
			 * case a file extension causes the file buffer's base address to change)
			 */

			for (;;) {
				int error;
				text_end = find_unescape_quote(token.t_file->f_pos,
				                               token.t_file->f_end,
				                               quote);
				if (text_end)
					break;
				text_iter     = (char *)0 + (text_iter - token.t_file->f_begin);
				token.t_begin = (char *)0 + (token.t_begin - token.t_file->f_begin);
				token.t_end   = (char *)0 + (token.t_end - token.t_file->f_begin);
				error         = TPPFile_NextChunk(token.t_file, TPPFILE_NEXTCHUNK_FLAG_EXTEND);
				text_iter     = token.t_file->f_begin + (text_iter - (char *)0);
				token.t_begin = token.t_file->f_begin + (token.t_begin - (char *)0);
				token.t_end   = token.t_file->f_begin + (token.t_end - (char *)0);
				if unlikely(error < 0)
					goto err_expr_ast;
				if unlikely(error == 0) {
					if (parser_warnatptrf(text_iter, W_STRING_TERMINATED_BY_EOF))
						goto err_expr_ast;
					text_end = token.t_file->f_end;
					break;
				}
			}

			if (tok == '!' || tok == ':') {
				char *rbrace;
				/* TODO: This needs to support recursive '{' + '}' pairs! */
				/* TODO: This needs to support \-escape sequences! */
				rbrace = (char *)memchr(token.t_begin, '}', (size_t)(text_end - token.t_begin));
				if unlikely(!rbrace) {
					if (parser_warnatptrf(text_iter - 1, W_TEMPLATE_STRING_UNMATCHED_LBRACE))
						goto err_expr_ast;
					rbrace = text_end;
				}

				/* The remainder of the expression is the format-argument */
				if unlikely(unicode_printer_put8(&format_printer, '{'))
					goto err_expr_ast;
				if unlikely(unicode_printer_print(&format_printer, token.t_begin,
				                                  (size_t)(rbrace - token.t_begin)) < 0)
					goto err_expr_ast;
				if unlikely(unicode_printer_put8(&format_printer, '}'))
					goto err_expr_ast;
				if (*rbrace == '}')
					++rbrace;
				token.t_begin = rbrace;
			} else if (tok == '}') {
				if unlikely(unicode_printer_print(&format_printer, "{}", 2) < 0)
					goto err_expr_ast;
				++token.t_begin;
			} else {
				char *rbrace;
				if unlikely(WARN(W_TEMPLATE_STRING_UNEXPECTED_TOKEN))
					goto err_expr_ast;
				rbrace = (char *)memchr(token.t_begin, '}', (size_t)(text_end - token.t_begin));
				if (!rbrace) {
					if (parser_warnatptrf(text_iter - 1, W_TEMPLATE_STRING_UNMATCHED_LBRACE))
						goto err_expr_ast;
					rbrace = text_end;
				} else {
					++rbrace;
				}
				token.t_begin = rbrace;
			}

			/* Trick the current token into becoming a string until the next unescaped quote. */
			token.t_id  = (tok_t)quote; /* TOK_STRING or TOK_CHAR */
			token.t_end = text_end;
			if (*text_end == quote)
				++token.t_end; /* Skip over unescaped quote */
			token.t_file->f_pos = token.t_end;

			/* Continue parsing the template-string after the closing '}' */
			text_iter   = token.t_begin;
			flush_start = token.t_begin;

			/* Append `expr_ast' to `format_argv' */
			ASSERT(format_argc <= format_arga);
			if (format_argc >= format_arga) {
				size_t new_format_arga = (format_arga << 1) | 1;
				DREF struct ast **new_format_argv;
				new_format_argv = (DREF struct ast **)Dee_TryReallocc(format_argv, new_format_arga,
				                                                      sizeof(DREF struct ast *));
				if (!new_format_argv) {
					new_format_arga = format_arga + 1;
					new_format_argv = (DREF struct ast **)Dee_Reallocc(format_argv, new_format_arga,
					                                                   sizeof(DREF struct ast *));
					if unlikely(!new_format_argv) {
err_expr_ast:
						ast_decref_likely(expr_ast);
						goto err;
					}
				}
				format_arga = new_format_arga;
				format_argv = new_format_argv;
			}
			format_argv[format_argc] = expr_ast;
			++format_argc;

		}	break;

		case '}':
			if (*text_iter == '}') {
				/* Escaped '}'
				 * No need to flush since '}' also needs to be escaped
				 * as '}}' in the template for `string.format'! */
				++text_iter;
			} else {
				/* Error: unmatched '}' */
				if (parser_warnatptrf(text_iter - 1, W_TEMPLATE_STRING_UNMATCHED_RBRACE))
					goto err;
			}
			break;

		case '\\': {
			/* Do normal backslash escaping (including \{ --> { and \} --> }) */
			ch = '\0';
			if unlikely(unicode_printer_print(&format_printer, flush_start,
			                                  (size_t)((text_iter - 1) - flush_start)) < 0)
				goto err;
			if (text_iter < text_end)
				ch = *text_iter++;
			switch (ch) {

			case '\\': /* Escaped the following character itself. */
			case '\'':
			case '\"':
				break;

			case '{':
			case '}':
				/* Special case: in template strings, these can also be escaped with a backslash.
				 * However, because we're using them with `string.format', we still have to escape
				 * them for use with it (by writing them twice)! */
				if unlikely(unicode_printer_put8(&format_printer, ch))
					goto err;
				break;

			case '\r':
				if (text_iter < text_end && *text_iter == '\n')
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
				uint32_t digit_value;
				max_digits = 8;
				goto parse_hex_integer;
			case 'u':
				max_digits = 4;
				goto parse_hex_integer;
			case 'x':
			case 'X':
				max_digits = (unsigned int)-1; /* Unlimited. (TODO: This is incorrect -- \x should encode actual bytes!) */
parse_hex_integer:
				count       = 0;
				digit_value = 0;
				while (count < max_digits) {
					uint32_t ch32;
					uint8_t val;
					char *old_iter;
					old_iter = text_iter;
					ch32     = utf8_readchar((char const **)&text_iter, text_end);
					if (!DeeUni_AsDigit(ch32, 16, &val)) {
						text_iter = old_iter;
						break;
					}
					digit_value <<= 4;
					digit_value |= val;
					++count;
				}
				if (!count) {
					/* Error: "No digits, or hex-chars found after \\x, \\u or \\U" */
					if (parser_warnatptrf(text_iter - 1,
					                      W_TEMPLATE_STRING_NO_DIGIT_OR_HEX_AFTER_BACKSLASH_X_u_U))
						goto err;
					break;
				}
				if (unicode_printer_putc(&format_printer, digit_value))
					goto err;
				goto after_escaped_putc;
			}	break;

			default: {
				uint32_t digit_value;
				if (ch >= '0' && ch <= '7') {
					unsigned int count;
					digit_value = (uint32_t)(ch - '0');
parse_oct_integer:
					/* Octal-encoded integer. */
					count = 1;
					while (count < 3) {
						uint32_t ch32;
						uint8_t digit;
						char *old_iter;
						old_iter = text_iter;
						ch32     = utf8_readchar((char const **)&text_iter, text_end);
						if (!DeeUni_AsDigit(ch32, 8, &digit)) {
							text_iter = old_iter;
							break;
						}
						digit_value <<= 3;
						digit_value |= digit;
						++count;
					}
					if (unicode_printer_putc(&format_printer, digit_value))
						goto err;
					goto after_escaped_putc;
				}
				if ((unsigned char)ch >= 0xc0) {
					uint32_t ch32;
					struct unitraits const *desc;
					uint8_t digit;
					--text_iter;
					ch32 = utf8_readchar((char const **)&text_iter, text_end);
					desc = DeeUni_Descriptor(ch32);
					if (desc->ut_flags & UNICODE_ISLF)
						goto after_escaped_putc; /* Escaped line-feed */
					if (DeeUniTrait_AsDigit(desc, 8, &digit)) {
						/* Unicode digit character. */
						digit_value = digit;
						goto parse_oct_integer;
					}
				}
				/* Error: Unknown escape character "%c" % ch */
				if (parser_warnatptrf(text_iter - 1, W_TEMPLATE_STRING_UNDEFINED_ESCAPE,
				                      (int)(unsigned int)(unsigned char)ch))
					goto err;
			}	break;

			}
			if unlikely(unicode_printer_put8(&format_printer, ch))
				goto err;
after_escaped_putc:
			flush_start = text_iter;
		}	break;

		default:
			break;
		}
	}

	/* Flush the remainder. */
	if unlikely(unicode_printer_print(&format_printer, flush_start,
	                                  (size_t)(text_end - flush_start)) < 0)
		goto err;
	if unlikely(yield() < 0)
		goto err; /* Consume the template string token. */

	/* Check if the next token is another template string. - If so, join the two! */
	if ((tok == KWD_f || tok == KWD_F) &&
	    (*token.t_end == '\"' || (*token.t_end == '\'' && !HAS(EXT_CHARACTER_LITERALS)))) {
		/* Join adjacent template strings */
		if unlikely(yield() < 0)
			goto err; /* Consume the template string token. */
		goto parse_current_token_as_template_string;
	}

	/* Pack everything together. */
	{
		DREF struct ast *temp;
		DREF DeeObject *format_str;
		format_str = unicode_printer_pack(&format_printer);
		if unlikely(!format_str)
			goto err_noprinter;
		result = ast_constexpr(format_str);
		result = ast_setddi(result, &loc);
		Dee_Decref_unlikely(format_str);
		if unlikely(!result)
			goto err_noprinter;
		temp = add_getattr_format(result);
		temp = ast_setddi(temp, &loc);
		ast_decref_unlikely(result);
		if unlikely(!temp)
			goto err_noprinter;
		ASSERT(format_arga >= format_argc);
		if (format_arga > format_argc) {
			DREF struct ast **new_format_argv;
			new_format_argv = (DREF struct ast **)Dee_TryReallocc(format_argv,
			                                                      format_argc,
			                                                      sizeof(DREF struct ast *));
			if likely(new_format_argv)
				format_argv = new_format_argv;
		}
		result = add_call_sequence(temp, format_argc, format_argv);
		result = ast_setddi(result, &loc);
		ast_decref_unlikely(temp);
		if unlikely(!result)
			goto err_noprinter;
	}
	return result;
err:
	unicode_printer_fini(&format_printer);
err_noprinter:
	while (format_argc) {
		--format_argc;
		ast_decref_likely(format_argv[format_argc]);
	}
	Dee_Free(format_argv);
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C */
