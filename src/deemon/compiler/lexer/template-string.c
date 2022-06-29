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
#ifndef GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C
#define GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>

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
	args_argv = (DREF struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
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


/* Parse a template string. */
INTERN WUNUSED DREF struct ast *FCALL
ast_parse_template_string(void) {
	size_t format_argc            = 0;
	size_t format_arga            = 0;
	DREF struct ast **format_argv = NULL;
	struct ast_loc loc;
	struct ast *result;
	struct unicode_printer format_printer = UNICODE_PRINTER_INIT;
	char *flush_start, *text_iter, *text_end;
	loc_here(&loc);
parse_current_token_as_template_string:
	ASSERT(tok == TOK_STRING || tok == TOK_CHAR);
	ASSERT(token.t_begin < token.t_end);
	ASSERT(token.t_begin[0] == '\"' || token.t_begin[0] == '\'');
	ASSERT(token.t_begin[0] == token.t_end[-1]);
	text_iter   = token.t_begin + 1;
	text_end    = token.t_end - 1;
	flush_start = text_iter;

	/* Parse format string */
	while (text_iter < text_end) {
		char ch = *text_iter++;
		switch (ch) {

		case '{': {
			uint32_t old_lexer_flags;
			struct TPPFile *old_eob;
			struct TPPToken old_token;
			char *old_file_pos;
			char *old_file_end, old_file_end_ch;
			char *expr_start, *expr_end;
			unsigned int recursion;
			DREF struct ast *expr_ast;

			/* Flush */
			if unlikely(unicode_printer_print(&format_printer, flush_start,
			                                  (size_t)((text_iter - 1) - flush_start)) < 0)
				goto err;
			if (*text_iter == '{') {
				/* Escaped '}' */
handle_escaped_lbrace:
				flush_start = text_iter; /* Next flush starts on-top of the second '{' */
				++text_iter;
				break;
			}

			/* Scan ahead until matching '}' and push contained text as a TPP-file.
			 * Then, setup the lexer to indicate EOF once that file has reached its
			 * end, before finally parsing the contents of the file as an expression
			 *
			 * The resulting expression must then be added to `format_argv'.
			 * NOTE: No special escaping happens inside of the '{...}' area, but we still
			 *       have to take care to set-up the lexer's line/column offsets such that
			 *       they point to the correct locations in the containing file! */
			expr_start = text_iter;
			expr_end   = expr_start;
			recursion  = 1;
			for (;;) {
				if (expr_end >= text_end) {
					/* Error: Unmatched '{' */
					if (parser_warnatptrf(expr_start - 1, W_TEMPLATE_STRING_UNMATCHED_LBRACE))
						goto err;
					--text_iter;
					goto handle_escaped_lbrace;
				}
				if (*expr_end == '{') {
					++recursion;
				} else if (*expr_end == '}') {
					--recursion;
					if (!recursion)
						break;
				}
				++expr_end;
			}

			/* Re-configure the lexer to stop parsing at the end of the string. */
			old_token                    = token;
			old_eob                      = TPPLexer_Current->l_eob_file;
			TPPLexer_Current->l_eob_file = old_token.t_file;
			old_file_pos                 = old_token.t_file->f_pos;
			old_file_end                 = old_token.t_file->f_end;
			old_token.t_file->f_pos      = (char *)expr_start;
			old_token.t_file->f_end      = (char *)expr_end;
			old_file_end_ch              = *expr_end;
			*expr_end                    = '\0';

			/* Configure the lexer for template string expression mode. */
			old_lexer_flags  = TPPLexer_Current->l_flags;
			TPPLexer_Current->l_flags &= ~(TPPLEXER_FLAG_WANTSPACE |
			                               TPPLEXER_FLAG_WANTLF |
			                               TPPLEXER_FLAG_WANTCOMMENTS);
			TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_NO_SEEK_ON_EOB |
			                              TPPLEXER_FLAG_NO_POP_ON_EOF);

			/* Parse the actual expression. */
			if unlikely(yield() < 0) {
err_inner_expr:
				expr_ast = NULL;
			} else {
				/* Parse the expression. */
				expr_ast = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if unlikely(!expr_ast)
					goto done_inner_expr;
				if (tok == '!') {
					/* The remainder of the expression is the format-argument */
					if unlikely(unicode_printer_put8(&format_printer, '{'))
						goto err_inner_expr_expr_ast;
					if unlikely(unicode_printer_print(&format_printer, token.t_begin,
					                                  (size_t)(expr_end - token.t_begin)) < 0)
						goto err_inner_expr_expr_ast;
					if unlikely(unicode_printer_put8(&format_printer, '}'))
						goto err_inner_expr_expr_ast;
				} else {
					if unlikely(unicode_printer_print(&format_printer, "{}", 2) < 0)
						goto err_inner_expr_expr_ast;
					if unlikely(tok && WARN(W_TEMPLATE_STRING_UNEXPECTED_TOKEN))
						goto err_inner_expr_expr_ast;
				}
				/* Append `expr_ast' to `format_argv' */
				ASSERT(format_argc <= format_arga);
				if (format_argc >= format_arga) {
					size_t new_format_arga = (format_arga << 1) | 1;
					DREF struct ast **new_format_argv;
					new_format_argv = (DREF struct ast **)Dee_TryRealloc(format_argv,
					                                                     new_format_arga *
					                                                     sizeof(DREF struct ast *));
					if (!new_format_argv) {
						new_format_arga = format_arga + 1;
						new_format_argv = (DREF struct ast **)Dee_Realloc(format_argv,
						                                                  new_format_arga *
						                                                  sizeof(DREF struct ast *));
						if unlikely(!new_format_argv) {
err_inner_expr_expr_ast:
							ast_decref_likely(expr_ast);
							goto err_inner_expr;
						}
					}
					format_arga = new_format_arga;
					format_argv = new_format_argv;
				}
				format_argv[format_argc] = expr_ast;
				++format_argc;
			}

			/* Restore the old lexer configuration. */
done_inner_expr:
			*expr_end                    = old_file_end_ch;
			TPPLexer_Current->l_flags    = old_lexer_flags;
			TPPLexer_Current->l_eob_file = old_eob;
			old_token.t_file->f_pos      = old_file_pos;
			old_token.t_file->f_end      = old_file_end;
			token                        = old_token;
			if unlikely(!expr_ast)
				goto err;

			/* Resume parsing after the '}' */
			text_iter   = expr_end + 1; /* Skip trailing '}' */
			flush_start = text_iter;
		}	break;

		case '}':
			if (*text_iter == '}') {
				/* Escaped '}' (NOTE: This `unicode_printer_print()' includes the '}' itself!) */
				if unlikely(unicode_printer_print(&format_printer, flush_start,
				                                  (size_t)(text_iter - flush_start)) < 0)
					goto err;
				++text_iter;
				flush_start = text_iter; /* Start flushing after the '{' */
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
			case '?':
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
					struct unitraits *desc;
					uint32_t ch32;
					uint8_t val;
					char *old_iter;
					old_iter = text_iter;
					ch32     = utf8_readchar((char const **)&text_iter, text_end);
					if (ch32 >= 'a' && ch32 <= 'f') {
						val = 10 + ((uint8_t)ch32 - 'a');
					} else if (ch32 >= 'A' && ch32 <= 'F') {
						val = 10 + ((uint8_t)ch32 - 'A');
					} else {
						desc = DeeUni_Descriptor(ch32);
						if (!(desc->ut_flags & UNICODE_FDECIMAL) ||
						    desc->ut_digit >= 10) {
							text_iter = old_iter;
							break;
						}
						val = desc->ut_digit;
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
						struct unitraits *desc;
						uint32_t ch32;
						char *old_iter;
						old_iter = text_iter;
						ch32     = utf8_readchar((char const **)&text_iter, text_end);
						desc     = DeeUni_Descriptor(ch32);
						if (!(desc->ut_flags & UNICODE_FDECIMAL) ||
						    desc->ut_digit >= 8) {
							text_iter = old_iter;
							break;
						}
						digit_value <<= 3;
						digit_value |= desc->ut_digit;
						++count;
					}
					if (unicode_printer_putc(&format_printer, digit_value))
						goto err;
					goto after_escaped_putc;
				}
				if ((unsigned char)ch >= 0xc0) {
					uint32_t ch32;
					struct unitraits *desc;
					--text_iter;
					ch32 = utf8_readchar((char const **)&text_iter, text_end);
					desc = DeeUni_Descriptor(ch32);
					if (desc->ut_flags & UNICODE_FLF)
						goto after_escaped_putc; /* Escaped line-feed */
					if ((desc->ut_flags & UNICODE_FDECIMAL) &&
					    (desc->ut_digit < 8)) {
						/* Unicode digit character. */
						digit_value = desc->ut_digit;
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
	    (*token.t_end == '\"' || (*token.t_end == '\'' && HAS(EXT_CHARACTER_LITERALS)))) {
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
			new_format_argv = (DREF struct ast **)Dee_TryRealloc(format_argv,
			                                                     format_argc *
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
