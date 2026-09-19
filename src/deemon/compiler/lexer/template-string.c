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
#ifndef GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C
#define GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_*alloc*, Dee_Free */
#include <deemon/compiler/ast.h>    /* AST_*, ast, ast_* */
#include <deemon/compiler/lexer.h>  /* ast_parse_expr */
#include <deemon/compiler/symbol.h> /* LOOKUP_SYM_NORMAL */
#include <deemon/compiler/tpp.h>
#include <deemon/object.h>          /* DREF, DeeObject, Dee_AsObject, Dee_Decref_unlikely */
#include <deemon/string.h>          /* DeeUniTrait_AsDigit, DeeUni_AsDigit, DeeUni_Descriptor, Dee_UNICODE_ISLF, Dee_UNICODE_PRINTER_INIT, Dee_unicode_printer*, Dee_unitraits */
#include <deemon/stringutils.h>     /* Dee_unicode_readutf8_n */
#include <deemon/system-features.h> /* memchr */
#include <deemon/type.h>            /* OPERATOR_CALL, OPERATOR_GETATTR */

#include "../../runtime/strings.h"

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* uint8_t, uint32_t */

DECL_BEGIN

#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

INTERN WUNUSED NONNULL((1, 2)) int DFCALL
ast_parse_string_const_printer(DeeLexer *self, struct Dee_unicode_printer *__restrict printer) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	tpp_ssize status;
	tpp_lexer_decodestring_config config;
	ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));
	tpp_lexer_decodestring_config_init_simple(&config, &Dee_unicode_printer_print, printer);
	status = tpp_lexer_parsestring_ex(&self->dl_lexer, &config, TPP_LEXER_PARSESTRING_FLAG_NORMAL);
	if (TPP_SSIZE_ISERR(status))
		goto err;
	return 0;
err:
	return -1;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
ast_parse_string_const(DeeLexer *self) {
	struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
	ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));
	do {
		if unlikely(ast_parse_string_const_printer(self, &printer))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
	} while (DeeLexer_IsStringToken(self));
	return Dee_unicode_printer_pack(&printer);
err:
	Dee_unicode_printer_fini(&printer);
	return NULL;
}

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
struct string_parser {
	struct Dee_unicode_printer sp_printer; /* String printer */
	struct ast                *sp_params;  /* [0..1] Format-string params (or "NULL" if `sp_printer` is a string constant).
	                                        * When non-NULL, this is the `AST_MULTIPLE` that will be used ast the first argument
	                                        * in the call to `string.format`. It also means that `{` and `}` must be escaped in
	                                        * `sp_printer`. */
	struct ast_loc             sp_loc;     /* String start location */
};

/* Escape existing `{` and `}` characters in `self` */
PRIVATE WUNUSED NONNULL((1)) int DCALL
string_parser_escape_braces(struct string_parser *__restrict self) {
	char const *current_str, *current_end, *current_iter;
	DREF DeeStringObject *current;
	current = (DREF DeeStringObject *)Dee_unicode_printer_pack(&self->sp_printer);
	if unlikely(!current)
		goto err_reinit_printer;

	/* Can use `DeeString_STR` because we're only looking for `{` and `}` */
	current_str = DeeString_STR(current);
	current_end = current_str + WSTR_LENGTH(current_str);

	/* Search for `{` and `}` */
	for (current_iter = current_str;; ++current_iter) {
		char ch;
		if (current_iter >= current_end)
			goto no_braces_found;
		ch = *current_iter;
		if (ch == '{')
			break;
		if (ch == '}')
			break;
	}

	/* String *does* contain (at least 1) brace character
	 * -> really have to escape. */
	Dee_unicode_printer_init(&self->sp_printer);
	current_str = DeeString_AsUtf8(current);
	if unlikely(!current_str)
		goto err_current;
	current_end = current_str + WSTR_LENGTH(current_str);

	for (current_iter = current_str;
	     current_iter < current_end; ++current_iter) {
		char ch = *current_iter;
		if (ch == '{' || ch == '}') {
			/* Flush up to (and including) the brace character */
			if (Dee_unicode_printer_printutf8(&self->sp_printer, current_str,
			                                  (size_t)((current_iter + 1) - current_str)) < 0)
				goto err_current;
			/* Reset flush base to brace character (thus causing
			 * it to be printed twice, which is how it needs to
			 * be escaped) */
			current_str = current_iter;
		}
	}

	/* Flush remainder */
	if (Dee_unicode_printer_printutf8(&self->sp_printer, current_str,
	                                  (size_t)(current_end - current_str)) < 0)
		goto err_current;
	Dee_Decref_likely(current);
	return 0;
no_braces_found:
	Dee_unicode_printer_init_string(&self->sp_printer, Dee_AsObject(current));
	return 0;
err_current:
	Dee_Decref_likely(current);
	return -1;
/*err_reinit_printer_with_current:
	Dee_unicode_printer_init_string(&self->sp_printer, Dee_AsObject(current));
	return -1;*/
err_reinit_printer:
	Dee_unicode_printer_init(&self->sp_printer);
	return -1;
}

PRIVATE tpp_ssize DPRINTER_CC
string_parser_printutf8(void *arg, char const *__restrict data, size_t len) {
	char const *iter, *end;
	struct string_parser *me = (struct string_parser *)arg;

	/* Check for simple case: no template params -> no need to do any escaping. */
	if (me->sp_params == NULL)
		return Dee_unicode_printer_printutf8(&me->sp_printer, data, len);

	/* Must double-escape `{` and `}` characters from `data` */
	end = (iter = data) + len;
	for (; iter < end; ++iter) {
		char ch = *iter;
		if (ch == '{' || ch == '}') {
			/* Must escape! */
			if (Dee_unicode_printer_printutf8(&me->sp_printer, data, (size_t)((iter + 1) - data)) < 0)
				goto err;

			/* Set flush start o print `ch` a second time. */
			data = iter;
		}
	}

	/* Flush remainder */
	if (Dee_unicode_printer_printutf8(&me->sp_printer, data, (size_t)(end - data)) < 0)
		goto err;
	return 0;
err:
	return -1;
}

/* parse the actual expression that is embedded within a template string. */
PRIVATE WUNUSED NONNULL((1)) DREF struct ast *TPPCALL
string_parser_printexpr_parse(DeeLexer *self) {
	return ast_parse_expr(self, LOOKUP_SYM_NORMAL);
}

PRIVATE tpp_ssize TPPCALL
string_parser_printexpr(void *arg, tpp_lexer *tpp_restrict lexer) {
	struct string_parser *me = (struct string_parser *)arg;
	struct ast *params;
	DREF struct ast **param_v;
	DREF struct ast *expr;
	DeeLexer *self = DeeLexer_OfTPP(lexer);

	/* Parse expression */
	expr = string_parser_printexpr_parse(self);
	if unlikely(!expr)
		goto err;

	/* Check for special case: first time we get
	 * here, we must escape already-printed text,
	 * as well as allocate the params-sequence! */
	params = me->sp_params;
	if (params == NULL) {
		if unlikely(string_parser_escape_braces(me))
			goto err_expr;
		param_v = (DREF struct ast **)Dee_Malloc(1, sizeof(DREF struct ast *));
		if unlikely(!param_v)
			goto err_expr;
		param_v[0] = expr; /* Inherit */
		params = ast_multiple(AST_FMULTIPLE_GENERIC, 1, param_v);
		params = ast_setddi(params, &me->sp_loc);
		if unlikely(!params)
			goto err_expr_param_v;
		me->sp_params = params;
	} else {
		/* Second parameter */
		size_t param_c;
		ASSERT(params->a_type == AST_MULTIPLE);
		ASSERT(params->a_flag == AST_FMULTIPLE_GENERIC);
		param_c = params->a_multiple.m_astc;
		param_v = params->a_multiple.m_astv;
		param_v = (DREF struct ast **)Dee_Realloc(param_v, param_c + 1,
		                                          sizeof(DREF struct ast *));
		if unlikely(!param_v)
			goto err_expr;
		params->a_multiple.m_astv = param_v;
		params->a_multiple.m_astc = param_c + 1;
		param_v[param_c] = expr; /* Inherit */
	}

	/* Print the template parameter marker
	 * HINT: When `expr` is something like `repr`,
	 *       this will be optimized to `{!r}` later! */
	switch (DeeLexer_GetTok(self)) {

	case '!':
	TPP_CASE_TPP_TOK_MC_STARTSWITH_EXCLAIM
	case ':':
	TPP_CASE_TPP_TOK_MC_STARTSWITH_COLON {
		/* Special case: remainder of current file (which is a special
		 * sub-file pushed by the TPP engine) must be used as-is as the
		 * template string format arguments:
		 * >> local x = f"foo = {foo!r}";
		 * >> local x = "foo = {!r}".format({foo}); // Same as this
		 *
		 * Because the template expression may also contain macros, we
		 * must also unwind the #include-stack until its very bottom
		 * (our caller will have made it so we can't pop beyond the
		 * fake file used to describe the template expression):
		 *
		 * >> #define EXCLAIM() !
		 * >> local x = f"foo = {foo EXCLAIM()r}";
		 * >> local x = "foo = {!r}".format({foo}); // Same as this
		 */
		tpp_file *const file = DeeLexer_GetFile(self);
		tpp_char const *params_start;
		tpp_char const *params_end;
		if (Dee_unicode_printer_putascii(&me->sp_printer, '{'))
			goto err;
		for (;;) {
			params_start = tpp_file_getlastpos(file);
			params_end   = tpp_file_getend(file);
			if (Dee_unicode_printer_printutf8(&me->sp_printer, (char const *)params_start,
				                              (size_t)(params_end - params_start)) < 0)
				goto err;
			if (!tpp_lexer_canpopfile(&self->dl_lexer))
				break;
			tpp_lexer_popfile(&self->dl_lexer);
		}

		/* Consume all input */
		DeeLexer_SetTokenRange(self, params_end, params_end);
		DeeLexer_SetTokenId(self, TPP_TOK_EOF);
		return Dee_unicode_printer_putascii(&me->sp_printer, '}');
	}	break;

	default: break;
	}
	return Dee_UNICODE_PRINTER_PRINT(&me->sp_printer, "{}");
err_expr_param_v:
	Dee_Free(param_v);
err_expr:
	ast_decref_likely(expr);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *TPPCALL
string_parser_pack(/*inherit(always)*/ struct string_parser *__restrict self) {
	DREF struct ast *ast__result;
	DREF struct ast *ast__template;
	DREF struct ast *ast__constexpr_format;
	DREF struct ast *ast__template_getattr_format;
	DREF struct ast *ast__params_tuple;
	DREF struct ast **ast__params_tuple__argv;
	DREF DeeObject *template_str;

	/* Pack string template (or constant string if there aren't any params) */
	template_str = Dee_unicode_printer_pack(&self->sp_printer);
	if unlikely(!template_str)
		goto err__xexpr;

	/* Pack string template into an AST */
	ast__template = ast_constexpr(template_str);
	ast__template = ast_setddi(ast__template, &self->sp_loc);
	Dee_Decref_unlikely(template_str);
	if unlikely(!ast__template)
		goto err__xexpr;

	/* Check for simple case: without any format-params, we're already done! */
	if likely(!self->sp_params)
		return ast__template;
	ASSERT(self->sp_params->a_type == AST_MULTIPLE);
	ASSERT(self->sp_params->a_flag == AST_FMULTIPLE_GENERIC);

	/* Form an AST tree like this (and set `self->sp_loc` as DDI for all):
	 * >> ast_operator2(                  // ast__result
	 * >>     OPERATOR_CALL,
	 * >>     ast_operator2(              // ast__template_getattr_format
	 * >>         OPERATOR_GETATTR,
	 * >>         {ast__template},
	 * >>         ast_constexpr("format") // ast__constexpr_format
	 * >>     ),
	 * >>     ast_multiple(               // ast__params_tuple
	 * >>         AST_FMULTIPLE_TUPLE,
	 * >>         1,
	 * >>         {self->sp_params}       // ast__params_tuple__argv
	 * >>     )
	 * >> ) */
	ast__constexpr_format = ast_constexpr(Dee_AsObject(&str_format));
	ast__constexpr_format = ast_setddi(ast__constexpr_format, &self->sp_loc);
	if unlikely(!ast__constexpr_format)
		goto err__expr__template;

	ast__template_getattr_format = ast_operator2(OPERATOR_GETATTR, AST_OPERATOR_FNORMAL,
	                                             ast__template, ast__constexpr_format);
	ast__template_getattr_format = ast_setddi(ast__template_getattr_format, &self->sp_loc);
	ast_decref_unlikely(ast__constexpr_format);
	ast_decref_unlikely(ast__template);
	if unlikely(!ast__constexpr_format)
		goto err__expr;

	ast__params_tuple__argv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
	if unlikely(!ast__params_tuple__argv)
		goto err__expr__template_getattr_format;
	ast__params_tuple__argv[0] = self->sp_params;
	ast__params_tuple = ast_multiple(AST_FMULTIPLE_TUPLE, 1, ast__params_tuple__argv);
	ast__params_tuple = ast_setddi(ast__params_tuple, &self->sp_loc);
	if unlikely(!ast__params_tuple)
		goto err__expr__template_getattr_format__params_tuple_argv;

	ast__result = ast_operator2(OPERATOR_CALL, AST_OPERATOR_FNORMAL,
	                            ast__template_getattr_format,
	                            ast__params_tuple);
	ast__result = ast_setddi(ast__result, &self->sp_loc);
	ast_decref_unlikely(ast__template_getattr_format);
	ast_decref_unlikely(ast__params_tuple);
	return ast__result;
err:
	return NULL;

err__template_getattr_format__params_tuple:
	ast_decref(ast__template_getattr_format);
	ast_decref(ast__params_tuple);
	goto err;
err__expr__template_getattr_format__params_tuple_argv:
	Dee_Free(ast__params_tuple__argv);
err__expr__template_getattr_format:
	ast_decref(ast__template_getattr_format);
	goto err__expr;
err__expr__template:
	ast_decref(ast__template);
err__expr:
err__xexpr:
	ast_xdecref(self->sp_params);
err:
	return NULL;
}


/* Parse a TPP_TOK_ISSTRING()-like token into an AST (includes template string handling) */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_string_ast(DeeLexer *self) {
	tpp_ssize status;
	tpp_lexer_decodestring_config config;
	struct string_parser parser;
	ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));

	/* Setup string parser. */
	if (DeeLexer_GetLoc(self, &parser.sp_loc))
		goto err;
	Dee_unicode_printer_init(&parser.sp_printer);
	parser.sp_params = NULL;

	/* Setup decode config. */
	config.tldsc_dataprinter = &string_parser_printutf8;
	config.tldsc_utf8printer = &string_parser_printutf8;
	config.tldsc_formatexpr  = &string_parser_printexpr;
	config.tldsc_arg         = &parser;
#if TPP_HAVE_STRING_ESCAPE_BIGCHAR
#error "Deemon doesn't support `tldsc_bigprinter`"
#endif /* TPP_HAVE_STRING_ESCAPE_BIGCHAR */

	/* Parse string... */
	status = tpp_lexer_parsestring_ex(&self->dl_lexer, &config, TPP_LEXER_PARSESTRING_FLAG_NORMAL);
	if (TPP_SSIZE_ISERR(status))
		goto err_parser;

	/* Pack parser into a string AST */
	return string_parser_pack(&parser);
err_parser:
	Dee_unicode_printer_fini(&parser.sp_printer);
	ast_xdecref(parser.sp_params);
err:
	return NULL;
}

#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *DFCALL
add_getattr_format(struct ast *__restrict self) {
	DREF struct ast *attr, *result;
	attr = ast_constexpr(Dee_AsObject(&str_format));
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

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *DFCALL
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

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) char *DFCALL
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
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_template_string(DeeLexer *self) {
	STATIC_ASSERT(TOK_STRING == '"');
	STATIC_ASSERT(TOK_CHAR == '\'');
	size_t format_argc            = 0;
	size_t format_arga            = 0;
	DREF struct ast **format_argv = NULL;
	struct ast_loc loc;
	struct ast *result;
	struct Dee_unicode_printer format_printer = Dee_UNICODE_PRINTER_INIT;
	char const *flush_start, *text_iter, *text_end;
	char quote;
	(void)self;
	if (DeeLexer_GetLoc(self, &loc))
		goto err;
parse_current_token_as_template_string:
	ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));
	ASSERT(DeeLexer_GetTokenStart(self) < DeeLexer_GetTokenEnd(self));
	ASSERT(DeeLexer_GetTokenStart(self)[0] == '\"' ||
	       DeeLexer_GetTokenStart(self)[0] == '\'');
	/*ASSERT(DeeLexer_GetTokenStart(self)[0] == DeeLexer_GetTokenEnd(self)[-1]);*/ /* Might not be the case if the user suppressed an EOF-in-string warning */
	quote       = DeeLexer_GetTokenStart(self)[0];
	text_iter   = (char const *)DeeLexer_GetTokenStart(self) + 1;
	text_end    = (char const *)DeeLexer_GetTokenEnd(self) - 1;
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
			if unlikely(Dee_unicode_printer_print(&format_printer, flush_start,
			                                      (size_t)((text_iter - 1) - flush_start)) < 0)
				goto err;

			/* Parse an expression at this position. */
			TPPLexer_Current->l_token.t_file->f_pos = (char *)text_iter;

			/* Parse the expression. */
			old_flags = TPPLexer_Current->l_flags;
			text_iter = (char *)0 + (text_iter - TPPLexer_Current->l_token.t_file->f_begin); /* To deal with a buffer realloc() */
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_EXTENDFILE;       /* So we don't loose our file position */
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_old_flags:
				TPPLexer_Current->l_flags = old_flags;
				goto err;
			}
			expr_ast = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!expr_ast)
				goto err_old_flags;
			TPPLexer_Current->l_flags = old_flags;
			text_iter = TPPLexer_Current->l_token.t_file->f_begin + (text_iter - (char *)0); /* To deal with a buffer realloc() */

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
			 * which will be parsed by `ast_parse_expr()`, which in turn makes use of
			 * TPP's automatic file-chunk-extension function. The only thing we need to
			 * ensure at that point, is that TPP doesn't discard older parts of its file
			 * so our pointers don't get all out of wack, which we do by setting the
			 * lexer's `TPPLEXER_FLAG_EXTENDFILE` flag above, and converting our pointers
			 * into relative offsets above/after the parsing step (which is needed in
			 * case a file extension causes the file buffer's base address to change)
			 */

			for (;;) {
				int error;
				text_end = find_unescape_quote(TPPLexer_Current->l_token.t_file->f_pos,
				                               TPPLexer_Current->l_token.t_file->f_end,
				                               quote);
				if (text_end)
					break;
				text_iter                         = (char *)0 + (text_iter - TPPLexer_Current->l_token.t_file->f_begin);
				TPPLexer_Current->l_token.t_begin = (char *)0 + (TPPLexer_Current->l_token.t_begin - TPPLexer_Current->l_token.t_file->f_begin);
				TPPLexer_Current->l_token.t_end   = (char *)0 + (TPPLexer_Current->l_token.t_end - TPPLexer_Current->l_token.t_file->f_begin);
				error                             = TPPFile_NextChunk(TPPLexer_Current->l_token.t_file, TPPFILE_NEXTCHUNK_FLAG_EXTEND);
				text_iter                         = TPPLexer_Current->l_token.t_file->f_begin + (text_iter - (char *)0);
				TPPLexer_Current->l_token.t_begin = TPPLexer_Current->l_token.t_file->f_begin + (TPPLexer_Current->l_token.t_begin - (char *)0);
				TPPLexer_Current->l_token.t_end   = TPPLexer_Current->l_token.t_file->f_begin + (TPPLexer_Current->l_token.t_end - (char *)0);
				if unlikely(error < 0)
					goto err_expr_ast;
				if unlikely(error == 0) {
					if (parser_warnatptrf(text_iter, W_STRING_TERMINATED_BY_EOF))
						goto err_expr_ast;
					text_end = TPPLexer_Current->l_token.t_file->f_end;
					break;
				}
			}

			if (DeeLexer_GetTok(self) == '!' ||
			    DeeLexer_GetTok(self) == ':') {
				char const *rbrace;
				/* TODO: This needs to support recursive '{' + '}' pairs! */
				/* TODO: This needs to support \-escape sequences! */
				rbrace = (char const *)memchr(TPPLexer_Current->l_token.t_begin, '}', (size_t)(text_end - TPPLexer_Current->l_token.t_begin));
				if unlikely(!rbrace) {
					if (parser_warnatptrf(text_iter - 1, W_TEMPLATE_STRING_UNMATCHED_LBRACE))
						goto err_expr_ast;
					rbrace = text_end;
				}

				/* The remainder of the expression is the format-argument */
				if unlikely(Dee_unicode_printer_put8(&format_printer, '{'))
					goto err_expr_ast;
				if unlikely(Dee_unicode_printer_print(&format_printer, TPPLexer_Current->l_token.t_begin,
				                                      (size_t)(rbrace - TPPLexer_Current->l_token.t_begin)) < 0)
					goto err_expr_ast;
				if unlikely(Dee_unicode_printer_put8(&format_printer, '}'))
					goto err_expr_ast;
				if (*rbrace == '}')
					++rbrace;
				TPPLexer_Current->l_token.t_begin = (char *)rbrace;
			} else if (DeeLexer_GetTok(self) == '}') {
				if unlikely(Dee_unicode_printer_print(&format_printer, "{}", 2) < 0)
					goto err_expr_ast;
				++TPPLexer_Current->l_token.t_begin;
			} else {
				char *rbrace;
				if (DeeLexer_Warnf(self, TPP_W_TEMPLATE_STRING_UNEXPECTED_TOKEN))
					goto err_expr_ast;
				rbrace = (char *)memchr(TPPLexer_Current->l_token.t_begin, '}', (size_t)(text_end - TPPLexer_Current->l_token.t_begin));
				if (!rbrace) {
					if (parser_warnatptrf(text_iter - 1, W_TEMPLATE_STRING_UNMATCHED_LBRACE))
						goto err_expr_ast;
					rbrace = (char *)text_end;
				} else {
					++rbrace;
				}
				TPPLexer_Current->l_token.t_begin = rbrace;
			}

			/* Trick the current token into becoming a string until the next unescaped quote. */
			TPPLexer_Current->l_token.t_id  = (tok_t)quote; /* TOK_STRING or TOK_CHAR */
			TPPLexer_Current->l_token.t_end = (char *)text_end;
			if (*text_end == quote)
				++TPPLexer_Current->l_token.t_end; /* Skip over unescaped quote */
			TPPLexer_Current->l_token.t_file->f_pos = TPPLexer_Current->l_token.t_end;

			/* Continue parsing the template-string after the closing '}' */
			text_iter   = TPPLexer_Current->l_token.t_begin;
			flush_start = TPPLexer_Current->l_token.t_begin;

			/* Append `expr_ast` to `format_argv` */
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
				 * as '}}' in the template for `string.format`! */
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
			if unlikely(Dee_unicode_printer_print(&format_printer, flush_start,
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
				 * However, because we're using them with `string.format`, we still have to escape
				 * them for use with it (by writing them twice)! */
				if unlikely(Dee_unicode_printer_put8(&format_printer, ch))
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
					char const *old_iter;
					old_iter = text_iter;
					ch32     = Dee_unicode_readutf8_n(&text_iter, text_end);
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
				if (Dee_unicode_printer_putc(&format_printer, digit_value))
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
						char const *old_iter;
						old_iter = text_iter;
						ch32     = Dee_unicode_readutf8_n(&text_iter, text_end);
						if (!DeeUni_AsDigit(ch32, 8, &digit)) {
							text_iter = old_iter;
							break;
						}
						digit_value <<= 3;
						digit_value |= digit;
						++count;
					}
					if (Dee_unicode_printer_putc(&format_printer, digit_value))
						goto err;
					goto after_escaped_putc;
				}
				if ((unsigned char)ch >= 0xc0) {
					uint32_t ch32;
					struct Dee_unitraits const *desc;
					uint8_t digit;
					--text_iter;
					ch32 = Dee_unicode_readutf8_n(&text_iter, text_end);
					desc = DeeUni_Descriptor(ch32);
					if (desc->ut_flags & Dee_UNICODE_ISLF)
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
			if unlikely(Dee_unicode_printer_put8(&format_printer, ch))
				goto err;
after_escaped_putc:
			flush_start = text_iter;
		}	break;

		default:
			break;
		}
	}

	/* Flush the remainder. */
	if unlikely(Dee_unicode_printer_print(&format_printer, flush_start,
	                                  (size_t)(text_end - flush_start)) < 0)
		goto err;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err; /* Consume the template string token. */

	/* Check if the next token is another template string. - If so, join the two! */
	if ((DeeLexer_GetTok(self) == TPP_KWD_f ||
	     DeeLexer_GetTok(self) == TPP_KWD_F) &&
	    (*DeeLexer_GetTokenEnd(self) == '\"' ||
	     (*DeeLexer_GetTokenEnd(self) == '\'' && !DeeLexer_Has(self, CHARACTER_LITERALS)))) {
		/* Join adjacent template strings */
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err; /* Consume the template string token. */
		goto parse_current_token_as_template_string;
	}

	/* Pack everything together. */
	{
		DREF struct ast *temp;
		DREF DeeObject *format_str;
		format_str = Dee_unicode_printer_pack(&format_printer);
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
	Dee_unicode_printer_fini(&format_printer);
err_noprinter:
	while (format_argc) {
		--format_argc;
		ast_decref_likely(format_argv[format_argc]);
	}
	Dee_Free(format_argv);
	return NULL;
}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_TEMPLATE_STRING_C */
