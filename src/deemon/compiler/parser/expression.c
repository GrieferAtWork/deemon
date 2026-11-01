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
#ifndef GUARD_DEEMON_COMPILER_PARSER_EXPRESSION_C
#define GUARD_DEEMON_COMPILER_PARSER_EXPRESSION_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* Dee_Free, Dee_Mallocc, Dee_Reallocc */
#include <deemon/bool.h>              /* Dee_False, Dee_True */
#include <deemon/compiler/ast.h>      /* AST_*, ast, ast_* */
#include <deemon/compiler/compiler.h> /* DeeCompiler_Current */
#include <deemon/compiler/lexer.h>    /* DeeLexer, DeeParser_*, TPPCALL, TPP_*, advance_wraplf, ast_loc, peek_keyword, peek_next_advance, peek_next_token, tpp_* */
#include <deemon/compiler/optimize.h> /* ast_optimize_all */
#include <deemon/compiler/parser.h>   /* AST_COMMA_*, AST_PARSE_WASEXPR_NO, CASE_TOKEN_IS_*, PARSE_UNARY_DISALLOW_CASTS, P_OPERATOR_FNORMAL, TOKEN_IS_*, ast_*, current_tags, parse_tags_block */
#include <deemon/compiler/symbol.h>   /* LOOKUP_SYM_*, SYMBOL_TYPE_*, current_basescope, current_scope, decl_ast_skip, get_current_this, lookup_nth, lookup_symbol, new_unnamed_symbol, symbol */
#include <deemon/error.h>             /* DeeError_Handled, ERROR_HANDLED_RESTORE */
#include <deemon/float.h>             /* DeeFloat_New */
#include <deemon/int.h>               /* DeeIntObject, DeeInt_*, Dee_INT_STRING* */
#include <deemon/module.h>            /* DeeModule*, Dee_module_symbol */
#include <deemon/none.h>              /* Dee_None */
#include <deemon/object.h>            /* DREF, DeeObject, DeeObject_AsUInt, Dee_AsObject, Dee_Decref, Dee_DecrefNokill, Dee_Incref, ITER_DONE */
#include <deemon/string.h>            /* DeeString_NewSized */
#include <deemon/stringutils.h>       /* Dee_unicode_readutf8_n */
#include <deemon/system-features.h>   /* DeeSystem_DEFINE_memrchr, memmoveupc */
#include <deemon/thread.h>            /* DeeThread_Self, Dee_import_frame */
#include <deemon/tuple.h>             /* Dee_EmptyTuple */
#include <deemon/type.h>              /* Dee_operator_t, OPERATOR_*, TP_FFINAL, TP_FNORMAL */

#include "../../runtime/strings.h"
#include "../../runtime/builtin.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* int32_t, uint8_t, uint16_t, uint32_t */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
struct fixed_length_suffix_type {
	char flst_suffix[6];
	char flst_ctype[10];
};

PRIVATE struct fixed_length_suffix_type const fixed_length_suffix_types[] = {
	{ "u", /*    */ "uint" },
	{ "l", /*    */ "long" },
	{ "ul", /*   */ "ulong" },
	{ "ll", /*   */ "llong" },
	{ "ull", /*  */ "ullong" },
	{ "i8", /*   */ "int8_t" },
	{ "ui8", /*  */ "uint8_t" },
	{ "i16", /*  */ "int16_t" },
	{ "ui16", /* */ "uint16_t" },
	{ "i32", /*  */ "int32_t" },
	{ "ui32", /* */ "uint32_t" },
	{ "i64", /*  */ "int64_t" },
	{ "ui64", /* */ "uint64_t" },
	{ "i128", /* */ "int128_t" },
	{ "ui128", /**/ "uint128_t" },
};

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) tpp_char const *DCALL
bse_trylskip(tpp_lexer *__restrict lexer,
             tpp_char const *start, tpp_char const *end,
             char const *match_text) {
	for (;;) {
		tpp_char ch = (tpp_char)*match_text++;
		if (ch == '\0')
			return start;
		if (start >= end)
			break;
		if (*start != ch)
			break;
		start = tpp_preparse_skipbse_fwd(lexer, start + 1, end);
	}
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) bool DCALL
bse_equals(tpp_lexer *__restrict lexer,
           tpp_char const *text, tpp_size len,
           char const *match_text) {
	tpp_char const *end = text + len;
	text = bse_trylskip(lexer, text, end, match_text);
	return text == end;
}
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */


#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER

PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_DecodeUnaryInt(DeeParser *__restrict self) {
	DeeAst *result;
	tpp_errno error;
	DREF DeeIntObject *resval;
	tpp_char const *suffix_start;
	ASSERT(TPP_TOK_ISINT(DeeParser_GetTok(self)));
#if 0 /* TODO: Implement as part of TPP3's `tpp_lexer_decodeint_ex()` */
	/* Verify that thousands-separators (if present) are used consistently. */
	if unlikely(verify_consistent_grouping(self))
		goto err;
#endif
	error = tpp_lexer_decodeint_ex(&self->p_lexer.dl_lexer, &resval, &suffix_start);
	if (TPP_ISERR(error))
		goto err;
	result = DeeAst_NewConst(Dee_AsObject(resval));
	if unlikely(!result)
		goto err;
	if (DeeAst_SetLocHere(result, &self->p_lexer))
		goto err_r;
	if (suffix_start < DeeParser_GetTokenEnd(self)) {
		if (tpp_lexer_getextension(&self->p_lexer.dl_lexer, TPP_EXT_FIXED_LENGTH_INTEGRALS)) {
			/* "-ffixed-length-integrals":
			 * >> 123ull
			 * Compile as:
			 * >> (ullong from ctypes)(123) */
			size_t i;
			DeeAst *ctypes_function;
			DeeAstLoc ctypes_loc;
			tpp_size suffix_len = (tpp_size)(DeeParser_GetTokenEnd(self) - suffix_start);
			for (i = 0;; ++i) {
				if (i >= COMPILER_LENOF(fixed_length_suffix_types))
					goto unknown_suffix;
				if (bse_equals(&self->p_lexer.dl_lexer, suffix_start, suffix_len,
				               fixed_length_suffix_types[i].flst_suffix))
					break; /* Found it! */
			}
			ctypes_function = DeeAst_NewSymRefExternByName("ctypes",
			                                               fixed_length_suffix_types[i].flst_ctype,
			                                               &self->p_lexer);
			if unlikely(!ctypes_function)
				goto err_r;
			ctypes_loc.l_name = result->a_loc.l_name;
			ctypes_loc.l_lc   = tpp_lexer_getlcinfoat(&self->p_lexer.dl_lexer, suffix_start);
			DeeAst_SetLoc(ctypes_function, &ctypes_loc);
			result = DeeAst_NewOperator2(OPERATOR_CALL, ctypes_function, result);
			if unlikely(!result)
				goto err;
			DeeAst_SetLoc(result, &ctypes_loc);
		} else {
unknown_suffix:
			if (DeeParser_Warnf(self, TPP_W_INVALID_INTEGER))
				goto err_r;
		}
	}
	return result;
err_r:
	DeeAst_Destroy(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL /* TPP_CASE_TPP_TOK_INT */
DeeParser_ParseUnaryInt(DeeParser *__restrict self) {
	DeeAst *result = DeeParser_DecodeUnaryInt(self);
	if (result && TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err_r;
	return result;
err_r:
	DeeAst_Destroy(result);
/*err:*/
	return NULL;
}

/* TPP_CASE_TPP_TOK_STRING (DeeParser_Has(self, CHARACTER_LITERALS)) */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryCharacterLiteral(DeeParser *__restrict self) {
	tpp_errno error;
	tpp_uintmax literal_value;
	DREF DeeObject *literal_value_ob;
	DeeAst *result;
	DeeAstLoc loc;
	ASSERT(TPP_TOK_ISSTRING(DeeParser_GetTok(self)));
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	if (DeeParser_Warnf(self, TPP_W_DEPRECATED_CHARACTER_INT))
		goto err;
	error = tpp_lexer_parsecharacter_literal(&self->p_lexer.dl_lexer, &literal_value,
	                                         TPP_LEXER_PARSESTRING_FLAG_STOPONLF);
	if (TPP_ISERR(error))
		goto err;
	literal_value_ob = DeeInt_NEWU(literal_value);
	if unlikely(!literal_value_ob)
		goto err;
	result = DeeAst_NewConst(literal_value_ob);
	result = DeeAst_TrySetLoc(result, &loc);
	return result;
err:
	return NULL;
}



/************************************************************************/
/* STRING                                                               */
/************************************************************************/
struct string_parser {
	struct Dee_unicode_printer sp_printer; /* String printer */
	size_t                     sp_paramc;  /* Allocated / used # of parameters in `sp_paramv` */
	DeeAst                    *sp_paramv;  /* [0..sp_paramc][owned] Format-string params (or "NULL" if `sp_printer` is a string constant).
	                                        * When `sp_paramc > 0`, this is the `Dee_AST_MULTI` that will be used ast the first argument
	                                        * in the call to `string.format`. It also means that `{` and `}` must be escaped in `sp_printer`. */
	DeeAstLoc                  sp_loc;     /* String start location */
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
	if (me->sp_paramc <= 0)
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

PRIVATE tpp_ssize DPRINTER_CC
string_parser_printdata(void *arg, char const *__restrict data, size_t len) {
	/* Same as `string_parser_printutf8()`, but use `Dee_unicode_printer_print8()` */
	char const *iter, *end;
	struct string_parser *me = (struct string_parser *)arg;
	if (me->sp_paramc <= 0)
		return Dee_unicode_printer_print8(&me->sp_printer, (uint8_t const *)data, len);
	end = (iter = data) + len;
	for (; iter < end; ++iter) {
		char ch = *iter;
		if (ch == '{' || ch == '}') {
			if (Dee_unicode_printer_print8(&me->sp_printer, (uint8_t const *)data, (size_t)((iter + 1) - data)) < 0)
				goto err;
			data = iter;
		}
	}
	if (Dee_unicode_printer_print8(&me->sp_printer, (uint8_t const *)data, (size_t)(end - data)) < 0)
		goto err;
	return 0;
err:
	return -1;
}

/* parse the actual expression that is embedded within a template string. */
PRIVATE WUNUSED NONNULL((1)) DeeAst *TPPCALL
string_parser_printexpr_parse(DeeParser *self) {
	/* TPP doesn't yield the first token for us (so we can decide how
	 * to yield that token) As such, this is where we yield said token. */
	if (TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err;
	return DeeParser_ParseExpr(self);
err:
	return NULL;
}

PRIVATE tpp_ssize TPPCALL
string_parser_printexpr(void *arg, tpp_lexer *tpp_restrict lexer) {
	struct string_parser *me = (struct string_parser *)arg;
	DeeAst *params;
	DeeAst *expr;
	DeeParser *self = DeeParser_OfTPP(lexer);

	/* Parse expression */
	expr = string_parser_printexpr_parse(self);
	if unlikely(!expr)
		goto err;

	/* Check for special case: first time we get
	 * here, we must escape already-printed text,
	 * as well as allocate the params-sequence! */
	if (me->sp_paramc <= 0) {
		if unlikely(string_parser_escape_braces(me))
			goto err_expr;
	}
	params = DeeAst_ReallocUninitializedMulti(me->sp_paramv, me->sp_paramc + 1);
	if unlikely(!params)
		goto err_expr;
	DeeAst_GetMultiElem(params)[me->sp_paramc] = expr; /* Inherit */
	me->sp_paramv = params;
	++me->sp_paramc;

	/* Print the template parameter marker
	 * HINT: When `expr` is something like `repr`,
	 *       this will be optimized to `{!r}` later! */
	switch (DeeParser_GetTok(self)) {

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
		tpp_file *const file = DeeParser_GetFile(self);
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
			if (!tpp_lexer_canpopfile(&self->p_lexer.dl_lexer))
				break;
			tpp_lexer_popfile(&self->p_lexer.dl_lexer);
		}

		/* Consume all input */
		DeeLexer_SetTokenRange(&self->p_lexer, params_end, params_end);
		DeeLexer_SetTokenId(&self->p_lexer, TPP_TOK_EOF);
		return Dee_unicode_printer_putascii(&me->sp_printer, '}');
	}	break;

	default: break;
	}
	return Dee_UNICODE_PRINTER_PRINT(&me->sp_printer, "{}");
err_expr:
	DeeAst_Destroy(expr);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DeeAst *TPPCALL
string_parser_pack(/*inherit(always)*/ struct string_parser *__restrict self) {
	DeeAst *params_ast;
	DeeAst *template_str_ast;
	DeeAst *result;
	DREF DeeObject *template_str;

	/* Pack string template (or constant string if there aren't any params) */
	template_str = Dee_unicode_printer_pack(&self->sp_printer);
	if unlikely(!template_str)
		goto err__expr;

	/* Pack string template into an AST */
	template_str_ast = DeeAst_NewConst(template_str);
	if unlikely(!template_str_ast)
		goto err__expr;
	DeeAst_SetLoc(template_str_ast, &self->sp_loc);

	/* Check for simple case: without any format-params, we're already done! */
	if likely(self->sp_paramc <= 0)
		return template_str_ast;
	params_ast = DeeAst_PackMulti(self->sp_paramv, Dee_AST_MULTI_KIND_GENERIC_SEQ, self->sp_paramc);
	DeeAst_SetLoc(params_ast, &self->sp_loc);
	result = DeeAst_NewCallAttrConst(template_str_ast, Dee_AsObject(&str_format), 1, &params_ast);
	if unlikely(!result)
		goto err;
	DeeAst_SetLoc(result, &self->sp_loc);
	return result;
err__expr:
	if (self->sp_paramc > 0) {
		DeeAst_Destroyv(DeeAst_GetMultiElem(self->sp_paramv), self->sp_paramc);
		DeeAst_FreeUninitializedMulti(self->sp_paramv);
	}
err:
	return NULL;
}


/* Parse a TPP_TOK_ISSTRING()-like token into an AST (includes template string handling) */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryString(DeeParser *__restrict self) {
	tpp_ssize status;
	tpp_lexer_decodestring_config config;
	struct string_parser parser;
	ASSERT(TPP_TOK_ISSTRING(DeeParser_GetTok(self)));

	/* Setup string parser. */
	if (DeeParser_GetLoc(self, &parser.sp_loc))
		goto err;
	Dee_unicode_printer_init(&parser.sp_printer);
	parser.sp_paramc = 0;
	parser.sp_paramv = NULL;

	/* Setup decode config. */
	config.tldsc_dataprinter = &string_parser_printdata;
	config.tldsc_utf8printer = &string_parser_printutf8;
	config.tldsc_formatexpr  = &string_parser_printexpr;
	config.tldsc_arg         = &parser;
#if TPP_HAVE_STRING_ESCAPE_BIGCHAR
#error "Deemon doesn't support `tldsc_bigprinter`"
#endif /* TPP_HAVE_STRING_ESCAPE_BIGCHAR */

	/* Parse string...
	 * NOTE: Stop if an LF token is encountered (if they're enabled, they have special meaning) */
	ASSERT(DeeParser_GetTokenStart(self) >= tpp_string_str(tpp_file_getchunk(DeeParser_GetFile(self))));
	ASSERT(DeeParser_GetTokenEnd(self) <= tpp_string_end(tpp_file_getchunk(DeeParser_GetFile(self))));
	status = tpp_lexer_parsestring_ex(&self->p_lexer.dl_lexer, &config, TPP_LEXER_PARSESTRING_FLAG_STOPONLF);
	if (TPP_SSIZE_ISERR(status))
		goto err_parser;

	/* Pack parser into a string AST */
	return string_parser_pack(&parser);
err_parser:
	Dee_unicode_printer_fini(&parser.sp_printer);
	if (parser.sp_paramc > 0) {
		DeeAst_Destroyv(DeeAst_GetMultiElem(parser.sp_paramv), parser.sp_paramc);
		DeeAst_FreeUninitializedMulti(parser.sp_paramv);
	}
err:
	return NULL;
}


/* TPP_CASE_TPP_TOK_FLOAT */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_DecodeUnaryFloat(DeeParser *__restrict self) {
	tpp_float value;
	DREF DeeFloatObject *value_obj;
	DeeAst *value_ast;
	tpp_errno error;
	tpp_char const *suffix_start;
	error = tpp_lexer_decodefloat_ex(&self->p_lexer.dl_lexer, &value, &suffix_start);
	if (TPP_ISERR(error))
		goto err;
	if (suffix_start < DeeParser_GetTokenEnd(self)) {
		if (DeeParser_Warnf(self, TPP_W_INVALID_FLOAT))
			goto err;
	}
	value_obj = (DREF DeeFloatObject *)DeeFloat_New((double)value);
	if unlikely(!value_obj)
		goto err;
	value_ast = DeeAst_NewConst(Dee_AsObject(value_obj));
	if unlikely(!value_ast)
		goto err;
	if (DeeAst_SetLocHere(value_ast, &self->p_lexer))
		goto err_value_ast;
	return value_ast;
err_value_ast:
	DeeAst_Destroy(value_ast);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryFloat(DeeParser *__restrict self) {
	DeeAst *result = DeeParser_DecodeUnaryFloat(self);
	if (result && TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err_r;
	return result;
err_r:
	DeeAst_Destroy(result);
/*err:*/
	return NULL;
}


/* TPP_TOK_IS_UNARY_BUILTIN_CONSTANT */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_DecodeUnaryBuiltinConstant(DeeParser *__restrict self) {
	DeeAst *result;
	DeeObject *const_value;
	ASSERT(TPP_TOK_IS_UNARY_BUILTIN_CONSTANT(DeeParser_GetTok(self)));
	switch (DeeParser_GetTok(self)) {
	case TPP_KWD_none:
		const_value = Dee_None;
		break;
	case TPP_KWD_true:
		const_value = Dee_True;
		break;
	case TPP_KWD_false:
		const_value = Dee_False;
		break;
	default: __builtin_unreachable();
	}
	Dee_Incref(const_value);
	result = DeeAst_NewConst(const_value);
	result = DeeAst_TrySetLocHere(result, &self->p_lexer);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryBuiltinConstant(DeeParser *__restrict self) {
	DeeAst *result = DeeParser_DecodeUnaryBuiltinConstant(self);
	if (result && TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err_r;
	return result;
err_r:
	DeeAst_Destroy(result);
/*err:*/
	return NULL;
}


/* TPP_KWD_bound `bound(expr)` */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryBound(DeeParser *__restrict self) {
	DeeAstLoc loc;
	bool has_paren;
	DeeAst *bound_expr;
	ASSERT(DeeParser_GetTok(self) == TPP_KWD_bound);
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	DeeParser_NoLf_Push(self);
	if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_nolf_break:
		DeeParser_NoLf_Break(self);
		goto err;
	}
	if (DeeParser_ParenBegin(self, &has_paren))
		goto err_nolf_break;
	bound_expr = DeeParser_ParseExpr(self);
	if unlikely(!bound_expr)
		goto err_nolf_break;
	bound_expr = DeeAst_NewAct1(Dee_AST_ACT1_BOUND, bound_expr, &self->p_lexer, &loc);
	if unlikely(!bound_expr)
		goto err_nolf_break;
	DeeParser_NoLf_Pop(self);
	if (DeeParser_ParenEnd(self, has_paren))
		goto err;
	return bound_expr;
err:
	return NULL;
}


/* TPP_TOK_IS_UNARY_BUILTIN_FUNCTION */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryBuiltinFunction(DeeParser *__restrict self) {
	bool has_paren;
	DeeAstLoc loc;
	DeeAst *result;
	tpp_token_id tok;
	tpp_token_id const mode = DeeParser_GetTok(self);
	ASSERT(TPP_TOK_IS_UNARY_BUILTIN_FUNCTION(mode));
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	DeeParser_NoLf_Push(self);
	tok = DeeParser_Yield(self);
	if (TPP_TOK_ISERR(tok)) {
err_nolf_break:
		DeeParser_NoLf_Break(self);
		goto err;
	}
	if (tok == '(' || tok == TPP_KWD_pack) {
		if (DeeParser_ParenBegin(self, &has_paren))
			goto err_nolf_break;
		result = DeeParser_ParseExpr(self);
	} else {
		has_paren = false;
		result = DeeParser_ParseUnary(self);
	}
	if unlikely(!result)
		goto err_nolf_break;
	switch (mode) {
	case TPP_KWD_str: result = DeeAst_NewOperator1(OPERATOR_STR, result); break;
	case TPP_KWD_repr: result = DeeAst_NewOperator1(OPERATOR_REPR, result); break;
	case TPP_KWD_copy: result = DeeAst_NewOperator1(OPERATOR_COPY, result); break;
	case TPP_KWD_type: result = DeeAst_NewAct1(Dee_AST_ACT1_TYPEOF, result); break;
	case TPP_KWD_deepcopy: result = DeeAst_NewAct1(Dee_AST_ACT1_DEEPCOPY, result); break;
	default: __builtin_unreachable();
	}
	if unlikely(!result)
		goto err_nolf_break;
	DeeAst_SetLoc(result, &loc);
	DeeParser_NoLf_Pop(self);
	if (DeeParser_ParenEnd(self, has_paren))
		goto err;
	return result;
err:
	return NULL;
}


/* TPP_TOK_IS_UNARY_OPERATOR */
#define TPP_CASE_TPP_TOK_UNARY_OPERATOR \
	case '#':                           \
	case '~':                           \
	case '+':                           \
	case '-':                           \
	case '!':                           \
	case TPP_TOK_PLUS_PLUS:             \
	case TPP_TOK_MINUS_MINUS:
#define TPP_TOK_IS_UNARY_OPERATOR(tok)               \
	((tok) == '#' || (tok) == '~' || (tok) == '+' || \
	 (tok) == '-' || (tok) == '!' ||                 \
	 (tok) == TPP_TOK_PLUS_PLUS ||                   \
	 (tok) == TPP_TOK_MINUS_MINUS)
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryOperator(DeeParser *__restrict self) {
	DeeAst *result;
	DeeAstLoc loc;
	tpp_token_id tok;
	tpp_token_id const mode = DeeParser_GetTok(self);
	ASSERT(TPP_TOK_IS_UNARY_BUILTIN_FUNCTION(mode));
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	if (TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err;
	result = DeeParser_ParseUnary(self);
	if unlikely(!result)
		goto err;
	switch (mode) {
	case '#': result = DeeAst_NewOperator1(OPERATOR_SIZE, result); break;
	case '~': result = DeeAst_NewOperator1(OPERATOR_INV, result); break;
	case '+': result = DeeAst_NewOperator1(OPERATOR_POS, result); break;
	case '-': result = DeeAst_NewOperator1(OPERATOR_NEG, result); break;
	case '!': result = DeeAst_NewAct1(Dee_AST_ACT1_NOT, result); break;
	case TPP_TOK_PLUS_PLUS: result = DeeAst_NewOperator1(OPERATOR_INC, result); break;
	case TPP_TOK_MINUS_MINUS: result = DeeAst_NewOperator1(OPERATOR_DEC, result); break;
	default: __builtin_unreachable();
	}
	if unlikely(!result)
		goto err;
	DeeAst_SetLoc(result, &loc);
	return result;
err:
	return NULL;
}

/* '<' + TPP_TOK_MC_STARTSWITH_LANGLE */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) DeeAst *DFCALL /* "ATTR_NOINLINE" because shouldn't be used anymore */
DeeParser_ParseUnaryCell(DeeParser *__restrict self) {
	DeeAstLoc loc;
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
	DeeAst *result;
	tpp_token_id tok;
	ASSERT(DeeParser_GetTok(self) == '<' ||
	       TPP_TOK_MC_STARTSWITH_LANGLE(DeeParser_GetTok(self)));
	DeeLexer_SetTokenEnd(&self->p_lexer, DeeLexer_GetTokenStart(&self->p_lexer) + 1);
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	if (DeeParser_Warnf(self, TPP_W_DEPRECATED_CELL_SYNTAX))
		goto err;
	tok = DeeParser_Yield(self);
	if (TPP_TOK_ISERR(tok))
		goto err;
	if (tok == '>' || TPP_TOK_MC_STARTSWITH_RANGLE(tok)) {
		/* empty Cell. */
		result = DeeAst_NewCallSymRefExtern(DeeModule_NewDeemon(),
		                                    &deemon_modsym_Cell,
		                                    0, NULL);
	} else {
		result = DeeParser_ParseUnary(self);
		if unlikely(!result)
			goto err;
		result = DeeAst_NewCallSymRefExtern(DeeModule_NewDeemon(),
		                                    &deemon_modsym_Cell,
		                                    1, &result);
	}
	if unlikely(!result)
		goto err;
	DeeAst_SetLoc(result, &loc);
	if (TPP_TOK_MC_STARTSWITH_RANGLE(DeeParser_GetTok(self))) {
		DeeLexer_SetTokenEnd(&self->p_lexer, DeeLexer_GetTokenStart(&self->p_lexer) + 1);
		DeeLexer_SetTokenId(&self->p_lexer, TPP_TOK_OFCHAR('>'));
	}
	if (TPP_TOK_ISERR(DeeParser_Skip(self, TPP_TOK_OFCHAR('>'))))
		goto err_r;
	return result;
err_r:
	DeeAst_Destroy(result);
err:
	return NULL;
}


/* TPP_KWD_if */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryIf(DeeParser *__restrict self) {
	bool has_paren;
	DeeAst *result;
	DeeAst *if_cond;
	DeeAst *if_tt;
	DeeAst *if_ff;
	DeeAstLoc loc;
	Dee_ast_if_flag flag = Dee_AST_IF_FLAG_NORMAL;
	ASSERT(DeeParser_GetTok(self) == TPP_KWD_if);
	if (DeeParser_ConsumeAnnotation_Likely(self))
		flag |= Dee_AST_IF_FLAG_LIKELY;
	if (DeeParser_ConsumeAnnotation_Unlikely(self))
		flag |= Dee_AST_IF_FLAG_UNLIKELY;
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	DeeParser_NoLf_Push(self);
	if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_nolf_break:
		DeeParser_NoLf_Break(self);
		goto err;
	}
	if (DeeParser_ParenBegin(self, &has_paren))
		goto err_nolf_break;
	if_cond = DeeParser_ParseExpr(self);
	if unlikely(!if_cond)
		goto err_nolf_break;
	DeeParser_NoLf_Pop(self);
	if (DeeParser_ParenEnd(self, has_paren)) {
err_if_cond:
		DeeAst_Destroy(if_cond);
		goto err;
	}
	if_tt = NULL;
	if (DeeParser_GetTok(self) != TPP_KWD_else &&
	    DeeParser_GetTok(self) != TPP_KWD_elif) {
		if_tt = DeeParser_ParseExpr(self);
		if unlikely(!if_tt)
			goto err_if_cond;
	}
	if_ff = NULL;
	if (DeeParser_GetTok(self) == TPP_KWD_else ||
	    DeeParser_GetTok(self) == TPP_KWD_elif) {
		if (DeeParser_GetTok(self) == TPP_KWD_elif) {
			DeeLexer_SetTokenId(&self->p_lexer, TPP_KWD_if); /* Cheat a bit... */
		} else if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_if_cond_tt:
			DeeAst_XDestroy(if_tt);
			goto err_if_cond;
		}
		if_ff = DeeParser_ParseExpr(self);
		if unlikely(!if_ff)
			goto err_if_cond_tt;
	}
	result = DeeAst_NewIf(flag, if_cond, if_tt, if_ff);
	result = DeeAst_TrySetLoc(result, &loc);
	return result;
err:
	return NULL;
}


/* TPP_KWD_assert */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryAssert(DeeParser *__restrict self) {
	DeeAst *result;
	DeeAst *assert_expr;
	DeeAst *assert_message;
	bool has_paren;
	DeeAstLoc loc;
	ASSERT(DeeParser_GetTok(self) == TPP_KWD_assert);
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	DeeParser_NoLf_Push(self);
	if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_nolf_break:
		DeeParser_NoLf_Break(self);
		goto err;
	}
	if (DeeParser_ParenBegin(self, &has_paren))
		goto err_nolf_break;
	assert_expr = DeeParser_ParseExpr(self);
	if unlikely(!assert_expr)
		goto err_nolf_break;
	assert_message = NULL;
	if (DeeParser_GetTok(self) == ',') {
		tpp_token_id tok = DeeParser_Yield(self);
		if (TPP_TOK_ISERR(tok)) {
err_nolf_break_assert_expr:
			DeeAst_Destroy(assert_expr);
			goto err_nolf_break;
		}
		if (has_paren || TPP_TOK_MAYBE_EXPR_START(tok)) {
			assert_message = DeeParser_ParseExpr(self);
			if unlikely(!assert_message)
				goto err_nolf_break_assert_expr;
		}
	}
	DeeParser_NoLf_Pop(self);
	if (DeeParser_ParenEnd(self, has_paren)) {
/*err_assert_expr_message:*/
		DeeAst_XDestroy(assert_message);
		DeeAst_Destroy(assert_expr);
		goto err;
	}
	result = assert_message
	         ? DeeAst_NewAct2(Dee_AST_ACT2_ASSERT, assert_expr, assert_message)
	         : DeeAst_NewAct1(Dee_AST_ACT1_ASSERT, assert_expr);
	result = DeeAst_TrySetLoc(result, &loc);
	return result;
err:
	return NULL;
}


/* TPP_KWD_class */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryClass(DeeParser *__restrict self, bool is_final) {
	ASSERT(DeeParser_GetTok(self) == TPP_KWD_class);
	/* TODO */
}


/* With the current token being `->`, parse that token,
 * as well as the following function-body. This function
 * returns the **BODY** of the function, meaning that the
 * caller must still package as `Dee_AST_FUNCTION` */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseFunctionBodyAtArrow(DeeParser *__restrict self) {
	DeeAst *result;
	tpp_token_id tok;
	DeeParser_NoLf_Push(self);
	tok = DeeParser_Yield(self);
	DeeParser_NoLf_Pop(self);
	if (TPP_TOK_ISERR(tok))
		goto err;
	if (tok == '{')
		return DeeParser_ParseStatementsOrBraces(self);
	return DeeParser_ParseExpr(self);
err:
	return NULL;
}

/* Keep the token that comes after `: <typedef>` and return it.
 * Must be called while the current token is `:`. Also note that
 * this function always returns with the current token (still/
 * once again) being that same `:`
 *
 * @return: * :               The token that comes after the <typedef>-part
 * @return: TPP_TOK_EOF:      EOF reached before end-of-typedef
 * @return: TPP_TOK_ISERR(*): Error was thrown */
PRIVATE WUNUSED NONNULL((1)) tpp_token_id DFCALL
DeeParser_PeekTokenAfterColonAndTypedef(DeeParser *__restrict self) {
	ASSERT(DeeParser_GetTok(self) == ':');
	/* TODO */
	return TPP_TOK_EOF;
}


/* Parse a java-lambda, starting at the first parameter:
 * >> (final x) -> { ... }
 *     ^     ^            ^
 *     start start        end
 * (either "start" is fine)
 *
 * Always returns a `Dee_AST_FUNCTION` with at least 1 parameter
 * NOTE: The returned AST doesn't have its location initialized.
 *       That should be done by the caller. */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseJavaLambdaAtFirstArgument(DeeParser *__restrict self,
                                         Dee_parser_functionparam_flag firstparam_flag,
                                         bool has_paren) {
	DeeAst *function_body;
	DeeAstLoc arrow_loc;
	tpp_token_id arrow_tok;
	struct Dee_ast_function_builder builder;
	Dee_ast_function_builder_init(&builder);
	Dee_ast_function_builder_consume_attr(&builder, self);

	/* Parse parameter list */
	DeeParser_NoLf_Push(self);
	for (;;) {
		if (DeeParser_ParseFunctionParam(self, &builder, firstparam_flag)) {
err_builder_nolf_break:
			DeeParser_NoLf_Break(self);
			goto err_builder;
		}
		if (DeeParser_GetTok(self) != ',')
			break;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_builder_nolf_break;
		firstparam_flag = Dee_PARSER_FUNCTIONPARAM_FLAG_NORMAL;
	}
	DeeParser_NoLf_Pop(self);
	if (has_paren) {
		if (TPP_TOK_ISERR(DeeParser_Skip(self, TPP_TOK_OFCHAR(')'))))
			goto err_builder;
	}

	/* Initialize return typedef */
	if (DeeParser_MaybeParseFunctionReturnType(self, &builder))
		goto err_builder;
	arrow_tok = DeeParser_Require(self, TPP_TOK_MINUS_RANGLE);
	if (TPP_TOK_ISERR(arrow_tok))
		goto err_builder;
	if (DeeParser_GetLoc(self, &arrow_loc))
		goto err_builder;
	if (arrow_tok == TPP_TOK_MINUS_RANGLE) {
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_builder;
	}
	if (DeeParser_GetTok(self) == '{') {
		function_body = DeeParser_ParseStatementsOrBraces(self);
	} else {
		function_body = DeeParser_ParseExpr(self);
		if unlikely(!function_body)
			goto err_builder;
		function_body = DeeAst_NewReturn(function_body);
		function_body = DeeAst_TrySetLoc(function_body, &arrow_loc);
	}
	if unlikely(!function_body)
		goto err_builder;
	return Dee_ast_function_builder_pack(&builder, NULL, function_body);
err_builder:
	Dee_ast_function_builder_fini(&builder);
err:
	return NULL;
}


PRIVATE tpp_errno TPPCALL
tpp_lexer_peek_accept_non_exclaim(void *arg, tpp_lexer *self) {
	(void)arg;
	if (tpp_lexer_gettok(self) == '!' ||
	    TPP_TOK_MC_STARTSWITH_EXCLAIM(tpp_lexer_gettok(self)))
		return TPP_ENOENT;
	return TPP_EOK;
}

PRIVATE WUNUSED NONNULL((1)) tpp_token_id TPPCALL
tpp_peek_next_non_exclaim(tpp_lexer *__restrict self) {
	return tpp_lexer_peek_raw(self, TPP_LEXER_PEEK_RAW_FLAG_NORMAL,
	                          NULL, &tpp_lexer_peek_accept_non_exclaim, NULL);
}


/* Maybe parse a CAST_SUFFIX expression in `(TYPE_EXPR)CAST_SUFFIX` */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DeeAst *DFCALL
DeeParser_MaybeParseUnaryCastSuffix(DeeParser *__restrict self,
                                    /*inherit(always)*/ DeeAst *__restrict type_expr,
                                    DeeAstLoc const *__restrict loc) {
	DeeAst *cast_suffix;
	if (!TPP_TOK_MAYBE_EXPR_START(DeeParser_GetTok(self)))
		return type_expr;
	switch (DeeParser_GetTok(self)) {

	case '!': {
		/* Special handling required:
		 * >> (int)!!!42;         // This...
		 * >> (int)!!!in my_list; // ... vs. this
		 * After parsing any number of additional `!` tokens, if the token
		 * thereafter is the keyword `is` or `in`, then this isn't a cast
		 * expression. However if it isn't, then it is a cast expression. */
		tpp_token_id next_token_id = tpp_peek_next_non_exclaim(&self->p_lexer.dl_lexer);
		if (TPP_TOK_ISERR(next_token_id))
			goto err_type_expr;
		if (next_token_id == TPP_KWD_is ||
		    next_token_id == TPP_KWD_in ||
		    next_token_id == TPP_KWD_as)
			return type_expr; /* This isn't a cast expression. */
	}	break;

	case '+': /* `(typexpr).operator add(castexpr)` vs. `(typexpr)castexpr.operator pos()` */
	case '-': /* `(typexpr).operator sub(castexpr)` vs. `(typexpr)castexpr.operator neg()` */
	case '<': /* `(typexpr).operator lo(castexpr)` vs. `(typexpr)(Cell(castexpr))` */
	case '[': /* `(typexpr).operator [](castexpr)` vs. `(typexpr)(List(castexpr))` */
	case TPP_TOK_DOT_DOT_DOT:
	case '(': /* Come `(func)(a, b, c)` as `func(a, b, c)`, rather than `func((a, b, c))` */
		return type_expr; /* Not a cast expression. */
	default: break;
	}

	/* Compile as a cast-expression */
	cast_suffix = DeeParser_ParseUnary(self);
	if unlikely(!cast_suffix)
		goto err_type_expr;
	type_expr = DeeAst_NewCall(type_expr, 1, &cast_suffix);
	type_expr = DeeAst_TrySetLoc(type_expr, loc);
	return type_expr;
err_type_expr:
	DeeAst_Destroy(type_expr);
err:
	return NULL;
}

/* Parenthesis + pack */
PRIVATE WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryParen(DeeParser *__restrict self) {
	DeeAst *result;
	DeeAstLoc loc;
	bool allow_tuple;
	bool has_paren = DeeParser_GetTok(self) == '(';
	bool allow_cast = has_paren;
	ASSERT(has_paren || DeeParser_GetTok(self) == TPP_KWD_pack);
	if (DeeParser_GetLoc(self, &loc))
		goto err;
	DeeParser_NoLf_Push(self);
	if (!has_paren) {
		bool inside_macro = tpp_file_ismacro(DeeParser_GetFile(self));
		tpp_token_id tok = DeeParser_Yield(self);
		if (TPP_TOK_ISERR(tok)) {
err_nolf_break:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (tok == '(') {
			has_paren = true;
		} else if (!inside_macro) {
			if (DeeParser_WarnfLoc(self, &loc, TPP_W_PACK_USED_OUTSIDE_OF_MACRO))
				goto err_nolf_break;
		}
	}

	switch (DeeParser_GetTok(self)) {

	case ')': {
		/* Empty tuple, or parameter-less java lambda */
		DeeParser_NoLf_Break(self);
		if (!has_paren) /* Something like `local x = (pack)` -> same as `local x = (())` */
			goto return_empty_tuple;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
			DeeAst *function_body;
parse_noarg_java_lambda:
			result = DeeAst_MallocUninitializedFunction(0);
			if unlikely(!result)
				goto err;
			/* Initialize return typedef */
			if (DeeParser_GetTok(self) == ':') {
				if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_result_function:
					DeeAst_FreeUninitializedFunction(result);
					goto err;
				}
				if (DeeParser_ParseTypeDef(self, DeeAst_GetFunctionReturnTypedef(result)))
					goto err_result_function;
			} else {
				Dee_ast_typedef_init(DeeAst_GetFunctionReturnTypedef(result));
			}

			function_body = DeeParser_ParseFunctionBodyAtArrow(self);
			if unlikely(!function_body) {
err_result_function_return_typedef:
				Dee_ast_typedef_fini(DeeAst_GetFunctionReturnTypedef(result));
				goto err_result_function;
			}
			result = DeeAst_PackFunction(result, Dee_AST_FUNCTION_FLAG_NORMAL, NULL,
			                             function_body, NULL, NULL, 0, 0);
			DeeAst_SetLoc(result, &loc);
			return result;
		}

		/* Deal with special case: `(): int -> 42` vs. `x ? (): int;`
		 * Only the first case is a java-lambda, while in the second
		 * case, we mustn't consume the `:`-token as it is part of an
		 * ?-style if-else expression. */
		if (DeeParser_GetTok(self) == ':') {
			tpp_token_id peek = DeeParser_PeekTokenAfterColonAndTypedef(self);
			if (TPP_TOK_ISERR(peek))
				goto err;
			if (peek == TPP_TOK_MINUS_RANGLE)
				goto parse_noarg_java_lambda;
		}

		/* Just an empty tuple... */
return_empty_tuple:
		result = DeeAst_NewConst(DeeTuple_NewEmpty());
		result = DeeAst_TrySetLoc(result, &loc);
		return result;
	}	break;

	case TPP_KWD_final: {
		/* Can be: `(final class { ... })`  (expression-in-parenthesis)
		 * Can be: `(final a) -> { ... }`   (java-lambda) */
		DeeAstLoc final_loc;
		tpp_token_id tok;
		if (DeeParser_GetLoc(self, &final_loc))
			goto err;
		tok = DeeParser_Yield(self);
		if (TPP_TOK_ISERR(tok))
			goto err;
		DeeParser_NoLf_Break(self);
		if (tok == TPP_KWD_class) {
			result = DeeParser_ParseUnaryClass(self, true);
			if unlikely(!result)
				goto err;
			result = DeeParser_ParseExprOperand(self, result);
			if unlikely(!result)
				goto err;
			allow_cast  = true;
			allow_tuple = true;
			break;
		}
		result = DeeParser_ParseJavaLambdaAtFirstArgument(self, true, has_paren);
		result = DeeAst_TrySetLoc(result, &loc);
		return result;
	}	break;

	case TPP_KWD_local:
	case TPP_KWD_varying:
	case TPP_TOK_STAR_STAR: {
		/* Guarantied to be a java-lambda */
		DeeParser_NoLf_Break(self);
		result = DeeParser_ParseJavaLambdaAtFirstArgument(self, false, has_paren);
		result = DeeAst_TrySetLoc(result, &loc);
		return result;
	}	break;

	case '{':
		result = DeeParser_ParseStatementsOrBraces(self);
		if unlikely(!result)
			goto err_nolf_break;
		allow_cast  = false;
		allow_tuple = DeeParser_ParseStatementsOrBraces_WASSTMT(result);
		break;

	case '(':
	case TPP_KWD_pack:
		result = DeeParser_ParseUnaryParen(self);
		if unlikely(!result)
			goto err_nolf_break;
		allow_cast  = false;
		allow_tuple = true;
		break;

	case ':': {
		tpp_token_id peek;
		if (has_paren)
			goto expression_in_paren;

		/* >> pack: int -> 42;
		 * Same as:
		 * >> pack(): int -> 42;
		 * Same as:
		 * >> (): int -> 42; */
		DeeParser_NoLf_Break(self);
		peek = DeeParser_PeekTokenAfterColonAndTypedef(self);
		if (TPP_TOK_ISERR(peek))
			goto err;
		if (peek == TPP_TOK_MINUS_RANGLE)
			goto parse_noarg_java_lambda;
		goto return_empty_tuple;
	}	break;

	case TPP_TOK_MINUS_RANGLE: {
		if (has_paren)
			goto expression_in_paren;
		/* >> pack -> 42;
		 * Same as:
		 * >> pack() -> 42;
		 * Same as:
		 * >> () -> 42; */
		DeeParser_NoLf_Break(self);
		goto parse_noarg_java_lambda;
	}	break;

	default:
		if (DeeParser_HasTokenKwd(self)) {
			/* Can be: `(a)`                             a                                (parenthesis)
			 * Can be: `(a) -> { ... }`                  function(a) {...}                (java-lambda)
			 * Can be: `(a?:b)`                          a?a:b                            (parenthesis -- elvis-operator)
			 * Can be: `(a?:b) -> { ... }`               function(a?: b) {...}            (java-lambda: `a` is param-name, `b` is type-expression, `?` means optional parameter)
			 * Can be: `(a,)`                            Tuple{a}                         (tuple)
			 * Can be: `(a,) -> { ... }`                 function(a) {...}                (java-lambda)
			 * Can be: `(a = init)`                      a=init                           (parenthesis)
			 * Can be: `(a = init,)`                     Tuple{Assign{a=init}}            (tuple)
			 * Can be: `(a = init) -> { ... }`           function(a=init) {...}           (java-lambda)
			 * Can be: `(a, b, c)`                       Tuple{a,b,c}                     (tuple)
			 * Can be: `(a, b, c) -> { ... }`            function(a,b,c) {...}            (java-lambda)
			 * Can be: `(a, b, c = init)`                Tuple{a,b,(c=init)}              (tuple)
			 * Can be: `(a, b, c = init) -> { ... }`     function(a,b,c=init) {...}       (java-lambda)
			 * Can be: `(a, b, c = init...)`             (a,b,c)=init                     (parenthesis containing Dee_AST_UNPACK_WR)
			 * Can be: `(a, b, c = init...) -> { ... }`  function(a,b,c=init...) {...}    (java-lambda; `init...` is default value for `c`)
			 * Can be: `(a, b, c = init...,)`            Tuple{(a,b,c)=init}              (tuple containing Dee_AST_UNPACK_WR)
			 * Can be: `(a, b, c = init...,) -> { ... }` function(a,b,c=init...) {...}    (java-lambda; `init...` is default value for `c`)
			 *
			 * And everything above can also exist when `pack` is used instead of `(...)`:
			 * Can be: `pack a`                             a                                (parenthesis)
			 * Can be: `pack a -> { ... }`                  function(a) {...}                (java-lambda)
			 * Can be: `pack a?:b`                          a?a:b                            (parenthesis -- elvis-operator)
			 * Can be: `pack a?:b -> { ... }`               function(a?: b) {...}            (java-lambda: `a` is param-name, `b` is type-expression, `?` means optional parameter)
			 * Can be: `pack a,`                            Tuple{a}                         (tuple)
			 * Can be: `pack a, -> { ... }`                 function(a) {...}                (java-lambda)
			 * Can be: `pack a = init`                      a=init                           (parenthesis)
			 * Can be: `pack a = init,`                     Tuple{Assign{a=init}}            (tuple)
			 * Can be: `pack a = init -> { ... }`           function(a=init) {...}           (java-lambda)
			 * Can be: `pack a, b, c`                       Tuple{a,b,c}                     (tuple)
			 * Can be: `pack a, b, c -> { ... }`            function(a,b,c) {...}            (java-lambda)
			 * Can be: `pack a, b, c = init`                Tuple{a,b,(c=init)}              (tuple)
			 * Can be: `pack a, b, c = init -> { ... }`     function(a,b,c=init) {...}       (java-lambda)
			 * Can be: `pack a, b, c = init...`             (a,b,c)=init                     (parenthesis containing Dee_AST_UNPACK_WR)
			 * Can be: `pack a, b, c = init... -> { ... }`  function(a,b,c=init...) {...}    (java-lambda; `init...` is default value for `c`)
			 * Can be: `pack a, b, c = init...,`            Tuple{(a,b,c)=init}              (tuple containing Dee_AST_UNPACK_WR)
			 * Can be: `pack a, b, c = init..., -> { ... }` function(a,b,c=init...) {...}    (java-lambda; `init...` is default value for `c`)
			 */
			/* TODO */
		} else if (!has_paren && !TPP_TOK_MAYBE_EXPR_START(DeeParser_GetTok(self))) {
			/* Something like `{ pack }` -> compile as `{ () }` */
			DeeParser_NoLf_Break(self);
			goto return_empty_tuple;
		}
		ATTR_FALLTHROUGH
		/* Keywords that can never indicate use of a java-lambda */
	TPP_CASE_TPP_TOK_UNARY_BUILTIN_CONSTANT
	case TPP_KWD_bound:
	TPP_CASE_TPP_TOK_UNARY_BUILTIN_FUNCTION
	case TPP_KWD_if:
	case TPP_KWD_assert:
	case TPP_KWD_function:
	case TPP_KWD_class:
expression_in_paren:
		result = DeeParser_ParseExpr(self);
		if unlikely(!result)
			goto err_nolf_break;
		allow_cast  = true;
		allow_tuple = true;
		break;
	}

	if (allow_tuple) {
		if (DeeParser_GetTok(self) == ',') {
			result = DeeParser_ParseCommaOperand(self, result, Dee_AST_MULTI_KIND_TUPLE);
			if unlikely(!result)
				goto err_nolf_break;
			DeeAst_SetLoc(result, &loc);
		} else if (result->a_kind == Dee_AST_ACT1_UNPACK) {
			/* Special case: `(a...)` must also be packed as a tuple */
			result = DeeAst_NewMulti(Dee_AST_MULTI_KIND_TUPLE, 1, &result);
			if unlikely(!result)
				goto err_nolf_break;
			DeeAst_SetLoc(result, &loc);
		}
		allow_cast = false; /* If it's a tuple, it can no longer be a cast! */
	}
	DeeParser_NoLf_Pop(self);
	if (has_paren) {
		if (TPP_TOK_ISERR(DeeParser_Skip(self, TPP_TOK_OFCHAR(')'))))
			goto err_r;
	}

	/* If allowed, also parse a cast-suffix expression */
	if (allow_cast)
		result = DeeParser_MaybeParseUnaryCastSuffix(self, result, &loc);
	return result;
err_r:
	DeeAst_Destroy(result);
err:
	return NULL;
}


INTERN ATTR_CONST WUNUSED bool DFCALL
TPP_TOK_MAYBE_EXPR_START(tpp_token_id tok) {
	switch (tok) {
	TPP_CASE_TPP_TOK_INT
	TPP_CASE_TPP_TOK_STRING
	TPP_CASE_TPP_TOK_FLOAT
	case '#':
	case '~':
	case '+':
	case '-':
	case '!':
	case '<': /* Cell (deprecated syntax) */
	TPP_CASE_TPP_TOK_MC_STARTSWITH_LANGLE
	case '(':
	case '[':
	case '{':
	case TPP_TOK_COLON_COLON:
	case TPP_TOK_DOT_DOT_DOT:
		return true;
	default: break;
	}
	return TPP_TOK_ISKEYWORD(tok);
}


INTERN WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnaryHead(DeeParser *__restrict self) {
	switch (DeeParser_GetTok(self)) {

	TPP_CASE_TPP_TOK_INT
		return DeeParser_ParseUnaryInt(self);
	TPP_CASE_TPP_TOK_STRING_SQUOTE
		if (DeeParser_Has(self, BUILTIN_EXPR_CHARACTER_LITERALS))
			return DeeParser_ParseUnaryCharacterLiteral(self);
		ATTR_FALLTHROUGH
	TPP_CASE_TPP_TOK_STRING_DQUOTE
		return DeeParser_ParseUnaryString(self);
	TPP_CASE_TPP_TOK_FLOAT
		return DeeParser_ParseUnaryFloat(self);
	TPP_CASE_TPP_TOK_UNARY_BUILTIN_CONSTANT
		return DeeParser_ParseUnaryBuiltinConstant(self);
	case TPP_KWD_bound:
		return DeeParser_ParseUnaryBound(self);
	TPP_CASE_TPP_TOK_UNARY_BUILTIN_FUNCTION
		return DeeParser_ParseUnaryBuiltinFunction(self);
	TPP_CASE_TPP_TOK_UNARY_OPERATOR
		return DeeParser_ParseUnaryOperator(self);
	case '<': /* Cell (deprecated syntax) */
	TPP_CASE_TPP_TOK_MC_STARTSWITH_LANGLE
		return DeeParser_ParseUnaryCell(self);
	case TPP_KWD_if:
		return DeeParser_ParseUnaryIf(self);
	case TPP_KWD_assert:
		return DeeParser_ParseUnaryAssert(self);
	case '(':
	case TPP_KWD_pack:
		return DeeParser_ParseUnaryParen(self);

	}
#if 0
	switch (DeeParser_GetTok(self)) {

	case TPP_KWD_function: {
		tpp_keyword const *function_name;
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_FUNCTION_IN_EXPRESSION))
			goto err;
		/* Create a new function */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		function_name = NULL;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_HasTokenKwd(self)) {
			function_name = DeeParser_GetTokenKwd(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
		}
		return ast_parse_function(self, function_name, NULL, false, &loc, NULL, NULL);
	}	break;

	case TPP_KWD_final: {
		tpp_keyword const *class_name;
		uint16_t class_flags;
		class_flags = TP_FFINAL;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Require(self, TPP_KWD_class)))
			goto err;
		goto do_create_class;
	case TPP_KWD_class:
		class_flags = TP_FNORMAL;
do_create_class:
		/* Create a new function */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		class_name = NULL;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_HasTokenKwd(self)) {
			if (DeeParser_GetTok(self) == TPP_KWD_final && !(class_flags & TP_FFINAL)) {
				/* allow `class final` as an alias for `final class` */
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
					goto err;
				class_flags |= TP_FFINAL;
			}
			if (DeeParser_HasTokenKwd(self)) {
				class_name = DeeParser_GetTokenKwd(self);
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
					goto err;
			}
		}
		/* Actually do parse the class now. */
		result = ast_parse_class(self, class_flags, class_name,
		                         false, LOOKUP_SYM_NORMAL);
		return ast_setddi(result, &loc);
	}	break;

	case TPP_KWD_import:
		return ast_parse_import_expression(self);

	case TPP_KWD_do:
	case TPP_KWD_while:
	case TPP_KWD_for:
	case TPP_KWD_foreach:
		/* Loop expressions. */
		return ast_parse_loopexpr(self);

	case '{':
		/* Brace-style sequence or mapping expression. */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
/*err_lbrace_flags:*/
			DeeParser_NoLf_Break(self);
			goto err;
		}
		result = ast_setddi(ast_parse_brace_items(self), &loc);
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeParser_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_BRACEINIT))
			goto err_r;
		return result;

	case TPP_KWD_del: {
		bool has_paren;

		/* Delete expression. */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_del_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (DeeParser_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_DEL))
			goto err_del_flags;
		result = ast_putddi(ast_parse_del(self, lookup_mode & ~PARSE_UNARY_DISALLOW_CASTS), &loc);
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeParser_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_DEL))
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
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		name = ast_parse_operator_name(self, P_OPERATOR_FNORMAL);
		if unlikely(name < 0)
			goto err;
		if (DeeParser_GetTok(self) != '(') {
			/* Bound-operator expression. */
			merge = ast_operator_func((Dee_operator_t)name, NULL);
		} else {
			DeeParser_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
/*err_operator_flags:*/
				DeeParser_NoLf_Break(self);
				goto err;
			}
			if (DeeParser_GetTok(self) == ')') {
				result = ast_constexpr(Dee_EmptyTuple);
			} else {
				result = ast_parse_comma(self,
				                        AST_COMMA_FORCEMULTIPLE,
				                        AST_FMULTIPLE_TUPLE,
				                        NULL);
			}
			DeeParser_NoLf_Pop(self);
			if unlikely(!result)
				goto err;
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN))
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_lbracket_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (DeeParser_GetTok(self) == '&' ||
		    DeeParser_GetTok(self) == '=') {
			if (DeeParser_Warnf(self, TPP_W_DEPRECATED_LAMBDA_MODE))
				goto err_lbracket_flags;
			if (DeeParser_GetLoc(self, &loc))
				goto err_lbracket_flags;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_lbracket_flags;
			DeeParser_NoLf_Break(self);
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(']'), W_EXPECTED_RBRACKET_AFTER_LAMBDA))
				goto err;
			goto do_lambda;
		}
		if (DeeParser_GetTok(self) == ']') {
			DeeParser_NoLf_Break(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
			if (DeeParser_GetTok(self) == ':') {
				/* If the current token is ':', try to skip over the return type
				 * annotation and check if the next token thereafter is '->' or '{'. */
				tpp_token_id tok_after_decl;
				tok_after_decl = peek_token_after_decl_ast_skip(self);
				if (TPP_TOK_ISERR(tok_after_decl))
					goto err;
				if (tok_after_decl == '{' || tok_after_decl == TPP_TOK_MINUS_RANGLE)
					goto do_lambda; /* Yup: it's a lambda! */
			}
			if (DeeParser_GetTok(self) == '(' ||
			    DeeParser_GetTok(self) == '{' ||
			    DeeParser_GetTok(self) == TPP_TOK_MINUS_RANGLE)
				goto do_lambda;
			result = ast_multiple(AST_FMULTIPLE_LIST, 0, NULL);
			goto set_list_loc;
		}
		if (DeeParser_GetTok(self) == ':') {
			DREF struct ast *start_expression;
			DREF struct ast *step_expression;
			result = ast_constexpr(Dee_None);
			if unlikely(!result)
				goto err_lbracket_flags;
			/* Range without begin index. */
do_range_expression:
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_lbracket_flags_r:
				ast_decref(result);
				goto err_lbracket_flags;
			}
			start_expression = result; /* Inherit reference. */
			/* Parse the end index. */
			if (DeeParser_GetTok(self) == ',') {
				/* No end index given. */
				result = ast_constexpr(Dee_None);
			} else {
				result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			}
			if unlikely(!result) {
				ast_decref(start_expression);
				goto err_lbracket_flags;
			}
			if (DeeParser_GetTok(self) == ',') {
				if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
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
			if (DeeParser_GetTok(self) == ':' &&
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
		DeeParser_NoLf_Pop(self);
		if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(']'), W_EXPECTED_RBRACKET_AFTER_LIST))
			goto err_r;
set_list_loc:
		return ast_setddi(result, &loc);
do_lambda:
		DeeParser_NoLf_Push(self);
		if unlikely(parse_tags_block(self)) {
/*err_lambda_flags:*/
			DeeParser_NoLf_Break(self);
			goto err;
		}
		result = ast_parse_function(self, NULL, NULL, true, &loc, NULL, NULL);
		DeeParser_NoLf_Pop(self);
		return result;

	case TPP_KWD_this: {
		struct symbol *this_sym;
		this_sym = get_current_this();
		if (!this_sym)
			goto default_case;
		result = ast_sethere(self, ast_sym(this_sym));
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		return result;
	}	break;

	case TPP_KWD___nth: {
		bool has_paren;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_nth_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (DeeParser_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_NTH))
			goto err_nth_flags;
		result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		/* Optimize the ast-expression to propagate constant, thus
		 * allowing the use of `__nth(2+3)` instead of forcing the
		 * user to write `__nth(5)` or `__nth(__TPP_EVAL(2+3))` */
		if (ast_optimize_all(result, true))
			goto err_r;
		if (result->a_type != AST_CONSTEXPR) {
			if (DeeParser_Warnf(self, TPP_W_EXPECTED_CONSTANT_AFTER_NTH))
				goto err_r;
		}
		if (DeeParser_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_NTH))
			goto err_r;
		if (DeeParser_HasTokenKwd(self)) {
			unsigned int nth_symbol = 0;
			struct symbol *sym;
			if (result->a_type == AST_CONSTEXPR &&
			    DeeObject_AsUInt(result->a_constexpr, &nth_symbol)) {
				DeeError_Handled(ERROR_HANDLED_RESTORE);
				if (DeeParser_Warnf(self, TPP_W_EXPECTED_CONSTANT_AFTER_NTH))
					goto err_r;
			}
			ast_decref(result);
			sym = lookup_nth(self, nth_symbol, DeeParser_GetTokenKwd(self));
			if likely(sym) {
				result = ast_sym(sym);
			} else {
				if (DeeParser_Warnf(self, TPP_W_UNKNOWN_NTH_SYMBOL, nth_symbol))
					goto err;
				result = ast_constexpr(Dee_None);
			}
			if unlikely(!result)
				goto err;
			result = ast_sethere(self, result);
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
		} else {
			if (DeeParser_Warnf(self, TPP_W_EXPECTED_KEYWORD_AFTER_NTH))
				goto err_r;
		}
		return result;
	}	break;

	case TPP_TOK_COLON_COLON:
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_GLOBAL_PREFIX))
			goto err;
		ATTR_FALLTHROUGH
	case TPP_KWD_global:
		lookup_mode |= LOOKUP_SYM_VGLOBAL;
		goto do_warn_deprecated_modifier;
	case TPP_KWD_local:
		lookup_mode |= LOOKUP_SYM_VLOCAL;
do_warn_deprecated_modifier:
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_PREFIX_IN_EXPRESSION))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
			return result;
		}
		ATTR_FALLTHROUGH
	default:
default_case:
		if (DeeParser_HasTokenKwd(self)) {
			/* Perform a regular symbol lookup. */
			tpp_keyword const *name;
			name = DeeParser_GetTokenKwd(self);
			if (DeeParser_GetLoc(self, &loc))
				goto err;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
			if (DeeParser_GetTok(self) == TPP_KWD_from) {
				/* `Error from deemon` - Short form of `import Error from deemon` */
				if (DeeParser_GetLoc(self, &loc))
					goto err;
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
					goto err;
				result = ast_parse_import_single(self, name);
				if (result && (DeeParser_GetTok(self) != ';' &&
				               DeeParser_GetTok(self) != ',' &&
				               DeeParser_GetTok(self) != ')' &&
				               DeeParser_GetTok(self) != '}' &&
				               DeeParser_GetTok(self) != ']' &&
				               DeeParser_GetTok(self) != '\n' &&
				               /* Don't emit this warning from macros! */
				               !tpp_file_ismacro(DeeParser_GetFile(self)))) {
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
					if (DeeParser_WarnfLoc(self, &loc, TPP_W_UNCLEAR_SYMBOL_FROM_MODULE,
					                      symname, modname))
						goto err;
				}
			} else if (DeeParser_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
				/* Support for java-style lambda with singular argument. */
				result = ast_parse_function_java_lambda(self, name, &loc);
			} else {
				DREF struct symbol *sym;
				sym = lookup_symbol(self, lookup_mode & ~PARSE_UNARY_DISALLOW_CASTS, name, &loc);
				if unlikely(!sym)
					goto err;
				result = ast_sym(sym);
			}
		} else {
			if (DeeParser_Warnf(self, TPP_W_UNEXPECTED_TOKEN_IN_EXPRESSION))
				goto err;
			result = ast_constexpr(Dee_None);
		}
		return ast_setddi(result, &loc);
	}
	__builtin_unreachable();
#endif
err_r:
	ast_decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeAst *DFCALL
DeeParser_ParseUnaryOperandDotOperator(DeeParser *__restrict self,
                                    /*inherit(always)*/ DeeAst *__restrict lhs) {
	/* Named & bound operator invocation. */
	DeeAstLoc loc;
	DeeAst *operator_args;
	struct Dee_ast_operator_id operator_name;
	if (DeeParser_GetLoc(self, &loc))
		goto err_lhs;
	if (TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err_lhs;
	if (DeeParser_ParseOperatorName(self, &operator_name))
		goto err_lhs;
	if (DeeParser_GetTok(self) != '(') {
		/* Bound-operator expression. */
		DeeAst *operator_function = DeeAst_NewOperatorFunction(&operator_name, &self->p_lexer);
		if unlikely(!operator_function)
			goto err_lhs;
		return DeeAst_NewInstanceMethod(operator_function, lhs, &self->p_lexer);
	}
	DeeParser_NoLf_Push(self);
	if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
		DeeParser_NoLf_Break(self);
err_lhs_operator_name:
		Dee_ast_operator_id_fini(&operator_name);
		goto err_lhs;
	}
	if (DeeParser_GetTok(self) == ')') {
		operator_args = DeeAst_NewConst(DeeTuple_NewEmpty());
	} else {
		operator_args = DeeParser_ParseExprComma();
	}
	DeeParser_NoLf_Pop(self);
	if unlikely(!operator_args)
		goto err_lhs_operator_name;
	lhs = DeeAst_NewOperatorEx2(&operator_name, Dee_AST_OPERATOR_FLAG_VARARG,
	                            lhs, operator_args);
	if unlikely(!lhs)
		goto err;
	DeeAst_SetLoc(lhs, &loc);
	if (TPP_TOK_ISERR(DeeParser_Skip(self, TPP_TOK_OFCHAR(')'))))
		goto err_lhs;
	return lhs;
err_lhs:
	DeeAst_Destroy(lhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL
DeeParser_ParseUnaryOperand(DeeParser *__restrict self,
                            /*inherit(always)*/ DeeAst *__restrict lhs) {
	DeeAstLoc loc;
again:
	switch (DeeParser_GetTok(self)) {

	case TPP_TOK_COLON_COLON:
		/* Backwards compatibility with deemon 100+ */
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_ATTRIBUTE_SYNTAX))
			goto err_lhs;
		ATTR_FALLTHROUGH
	case '.': /* Attribute lookup */
		if (DeeParser_GetLoc(self, &loc))
			goto err_lhs;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_lhs;
		switch (DeeParser_GetTok(self)) {

		case TPP_KWD_this:
			/* This is really just a no-op. */
yield_and_again:
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_lhs;
			goto again;

		case TPP_KWD_class:
			/* Return the real class of a type, properly unwinding super. */
			lhs = DeeAst_NewAct1(Dee_AST_ACT1_CLASSOF, lhs);
			if unlikely(!lhs)
				goto err;
			DeeAst_SetLoc(lhs, &loc);
			goto yield_and_again;

		case TPP_KWD_super:
			/* Return the real class of a type, properly unwinding super. */
			lhs = DeeAst_NewAct1(Dee_AST_ACT1_SUPEROF, lhs);
			if unlikely(!lhs)
				goto err;
			DeeAst_SetLoc(lhs, &loc);
			goto yield_and_again;

		case TPP_KWD_operator:
			lhs = DeeParser_ParseUnaryOperandDotOperator(self, lhs);
			if unlikely(!lhs)
				goto err;
			goto again;

		default:
			break;
		}

		if (DeeParser_HasTokenKwd(self)) {
			DREF DeeObject *attr_name;
			DeeAst *attr_name_ast;
			if (DeeParser_IsIdentifier(self, DeeParser_GetTokenKwd(self))) {
				if (DeeParser_Warnf(self, TPP_W_RESERVED_ATTRIBUTE_NAME,
				                   DeeParser_GetTokenKwdCStr(self)))
					goto err_lhs;
			}
			attr_name = DeeString_NewSized(DeeParser_GetTokenKwdCStr(self),
			                               DeeParser_GetTokenKwdLen(self));
			if unlikely(!attr_name)
				goto err_lhs;
			attr_name_ast = DeeAst_NewConst(attr_name);
			if unlikely(!attr_name_ast)
				goto err_lhs;
			if (DeeAst_SetLocHere(attr_name_ast, &self->p_lexer)) {
				DeeAst_Destroy(attr_name_ast);
				goto err_lhs;
			}
			lhs = DeeAst_NewOperator2(OPERATOR_GETATTR, lhs, attr_name_ast);
			if unlikely(!lhs)
				goto err;
			DeeAst_SetLoc(lhs, &loc);
			goto yield_and_again;
		}

		if (DeeParser_Warnf(self, TPP_W_EXPECTED_KEYWORD_AFTER_DOT))
			goto err_lhs;
		goto again;

	case '[': {
		DeeAst *range_start;

		/* sequence operator */
		if (DeeParser_GetLoc(self, &loc))
			goto err_lhs;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_lhs_lbracket_flags:
			DeeParser_NoLf_Break(self);
			goto err_lhs;
		}
		if (DeeParser_GetTok(self) == ':') {
			range_start = DeeAst_NewConst(DeeNone_NewRef());
		} else {
			range_start = DeeParser_ParseExpr(self);
		}
		if unlikely(!range_start)
			goto err_lhs_lbracket_flags;
		if (DeeParser_GetTok(self) == ':') {
			DeeAst *range_end;
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_lhs_lbracket_flags_range_start:
				DeeAst_Destroy(range_start);
				goto err_lhs_lbracket_flags;
			}
			if (DeeParser_GetTok(self) == ']') {
				range_end = DeeAst_NewConst(DeeNone_NewRef());
			} else {
				range_end = DeeParser_ParseExpr(self);
			}
			if unlikely(!range_end)
				goto err_lhs_lbracket_flags_range_start;
			lhs = DeeAst_NewOperator3(OPERATOR_GETRANGE, lhs, range_start, range_end);
		} else {
			lhs = DeeAst_NewOperator2(OPERATOR_GETITEM, lhs, range_start);
		}
		DeeParser_NoLf_Pop(self);
		if unlikely(!lhs)
			goto err;
		DeeAst_SetLoc(lhs, &loc);
		if (TPP_TOK_ISERR(DeeParser_Skip(self, TPP_TOK_OFCHAR(']'))))
			goto err_lhs;
		goto again;
	}	break;

	case '{': { /* Brace initializers. */
		if (DeeParser_GetLoc(self, &loc))
			goto err_lhs;
		/* TODO */
	}	break;

#if 0
		case '{':
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
			bool inside_macro = tpp_file_ismacro(DeeParser_GetFile(self));
			/* Call expression without parenthesis. */
			if (DeeParser_GetLoc(self, &loc))
				goto err_r;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
			if (DeeParser_GetTok(self) == '(')
				goto do_normal_call_with_loc;
			if (!inside_macro) {
				if (DeeParser_WarnfLoc(self, &loc, TPP_W_PACK_USED_OUTSIDE_OF_MACRO))
					goto err_r;
			}
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
			if (DeeParser_GetLoc(self, &loc))
				goto err_r;
do_normal_call_with_loc:
			DeeParser_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_r_lparen_flags:
				DeeParser_NoLf_Break(self);
				goto err_r;
			}
			if (DeeParser_GetTok(self) == ')') {
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
			DeeParser_NoLf_Pop(self);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_CALL))
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
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
		}	break;

		case TPP_TOK_DOT_DOT_DOT: /* Expand expression. */
			merge = ast_sethere(self, ast_expand(result));
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
#endif

	default: break;
	}
	return lhs;
err_lhs:
	DeeAst_Destroy(lhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DFCALL
DeeParser_ParseUnary(DeeParser *__restrict self) {
	DeeAst *result = DeeParser_ParseUnaryHead(self);
	if likely(result)
		result = DeeParser_ParseUnaryOperand(self, result);
	return result;
}


INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL
DeeParser_ParseProdOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs) {

}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseSumOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseShiftOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseCmpOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseCmpeqOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseAndOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseXorOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseOrOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseAsOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseLandOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseLorOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseCondOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DFCALL DeeParser_ParseAssignOperand(DeeParser *__restrict self, /*inherit(always)*/ DeeAst *__restrict lhs);


#else /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */
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
INTERN uint16_t parser_flags = PARSE_FNORMAL;


#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
INTDEF WUNUSED NONNULL((1)) tpp_token_id TPPCALL
tpp_peek_next_non_exclaim(tpp_lexer *__restrict self);
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */

/* Return 1 if the current token may be the begin of an expression.
 * @return: 1:  Yes
 * @return: 0:  No
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DFCALL
maybe_expression_begin(DeeLexer *self) {
	switch (DeeParser_GetTok(self)) {
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
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		tpp_token_id next_non_exclaim;
		/* Peek the next token that doesn't start with `!` */
		next_non_exclaim = tpp_peek_next_non_exclaim(&self->dl_lexer);
		if (TPP_TOK_ISERR(next_non_exclaim))
			goto err;
		if (next_non_exclaim == TPP_KWD_is ||
		    next_non_exclaim == TPP_KWD_in ||
		    next_non_exclaim == TPP_KWD_as)
			goto no; /* This isn't an expression. */
		goto yes;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		struct TPPFile *tok_file;
		tpp_keyword const *kwd;
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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	}	break;

	default:
		/* Any kind of keyword can appear in expressions, either
		 * a native expression keyword, or as a variable name. */
		if (DeeParser_HasTokenKwd(self))
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
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	tpp_token_id peek;
	peek = tpp_lexer_peek_raw(&self->dl_lexer, TPP_LEXER_PEEK_RAW_FLAG_NORMAL,
	                          NULL, NULL, NULL);
	if (TPP_TOK_ISERR(peek))
		goto err;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	char const *tok_begin;
	char peek;
	struct TPPFile *tok_file;
	(void)self;
	tok_begin = peek_next_token(&tok_file);
	if unlikely(!tok_begin)
		goto err;
	peek = *tok_begin;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	switch (peek) {

	case '~':
	case '(':
	case '#':
	case '[': /* For lists. */
	case '{': /* Brace initializers. */
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
	case '_': /* For identifiers */
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	case '$': /* For identifiers */
		goto yes;

	case '+':
	case '-':
	case '<': /* For cells. (deprecated syntax) */
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		goto yes;

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	TPP_CASE_TPP_TOK_NUMBER
	TPP_CASE_TPP_TOK_STRING
		goto yes;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		/* Integer / float */
	case '\'':
	case '\"':
		/* String constants */
		goto yes;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	case TPP_TOK_DOT_DOT_DOT:
		goto yes;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	case '.': /* TPP_TOK_DOT_DOT_DOT */
		tok_begin = advance_wraplf(tok_begin);
		if (*tok_begin != '.')
			goto no;
		tok_begin = advance_wraplf(tok_begin);
		if (*tok_begin != '.')
			goto no;
		goto yes; /* ... */
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	case TPP_TOK_COLON_COLON: /* Deprecated global-accessor syntax. */
		goto yes;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	case ':': /* Deprecated global-accessor syntax. */
		tok_begin = advance_wraplf(tok_begin);
		if (*tok_begin != ':')
			goto no;
		goto yes; /* :: */
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

	case '!': {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		tpp_token_id next_non_exclaim;
		/* Peek the next token that doesn't start with `!` */
		next_non_exclaim = tpp_peek_next_non_exclaim(&self->dl_lexer);
		if (TPP_TOK_ISERR(next_non_exclaim))
			goto err;
		if (next_non_exclaim == TPP_KWD_is ||
		    next_non_exclaim == TPP_KWD_in ||
		    next_non_exclaim == TPP_KWD_as)
			goto no; /* This isn't an expression. */
		goto yes;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		tpp_keyword const *kwd;
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
			    tpp_keyword_getid(kwd) == TPP_KWD_in ||
			    tpp_keyword_getid(kwd) == TPP_KWD_as)
				goto no;
		}
		goto yes;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	}	break;


#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
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
		if (TPP_TOK_ISKEYWORD(peek))
			goto yes;
		break;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	default: {
		tpp_keyword const *kwd;
		if (!tpp_is_keyword_start(peek))
			goto no;
		kwd = peek_keyword(tok_file, tok_begin, 0);
		if unlikely(!kwd) {
			if unlikely(TPPLexer_Current->l_token.t_id == TOK_ERR)
				goto err;
			goto yes; /* First-time-used user-defined keyword token. */
		}
		switch (tpp_keyword_getid(kwd)) {

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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

	}
no:
	return 0;
yes:
	return 1;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF struct ast *DFCALL
make_bound_expression(DeeLexer *self,
                      struct ast *__restrict base_expr,
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
	} else {
		if (DeeParser_WarnfAst(self, base_expr, TPP_W_CANNOT_TEST_EXPRESSION_BINDING))
			goto err;
		/* Fallback-after-warning: Return `true` */
		result = ast_constexpr(Dee_True);
	}
	return ast_putddi(result, loc);
err:
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
	DeeParser_NoLf_Push(self);
	if (DeeParser_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_IMPORT)) {
err_r_flags:
		DeeParser_NoLf_Break(self);
		goto err_r;
	}
	if (DeeParser_GetLoc(self, &loc))
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
	DeeParser_NoLf_Pop(self);
	if unlikely(!merge)
		goto err;
	result = merge;
	if (DeeParser_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_IMPORT))
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
	if (DeeParser_GetLoc(self, &import_loc))
		goto err;
	if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	/* TODO: Have this warning be done inside of TPP3 */
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	char const *tok_start, *tok_end, *iter, *lastsep;
	size_t interval, wanted_interval;
	tok_start = (char const *)DeeParser_GetTokenStart(self);
	tok_end   = (char const *)DeeParser_GetTokenEnd(self);
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
			if (DeeParser_Warnf(self, TPP_W_INCONSISTENT_THOUSANDS_SEPERATORS,
			                   (unsigned int)interval,
			                   (unsigned int)offset))
				goto err;
			return 0;
		}
		lastsep = iter;
	}

	/* Decimals should use a thousands-interval of `3`.
	 * For every other radix, the interval should be `4`. */
	wanted_interval = *tok_start == '0' ? 4 : 3;
	if (wanted_interval != interval) {
		if (DeeParser_Warnf(self, TPP_W_INCORRECT_THOUSANDS_SEPERATORS,
		                   (unsigned int)wanted_interval,
		                   (unsigned int)interval))
			goto err;
	}

	return 0;
err:
	return -1;
}

/* Peek the token that comes after `decl_ast_skip()` */
PRIVATE WUNUSED NONNULL((1)) tpp_token_id DFCALL
peek_token_after_decl_ast_skip(DeeLexer *self) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	tpp_token_id result;
	tpp_char const *colon_ptr;
	tpp_file *const file = DeeParser_GetFile(self);
	tpp_size rel_colon_ptr;

	/* Prevent going out of the current file */
	tpp_lexer_autopopfile_pushoff(&self->dl_lexer);

	/* Prevent macro expansion and directives (essentially
	 * turning `tpp_lexer_yield()` into `tpp_lexer_yieldraw()`) */
	tpp_lexer_pushfeatures(&self->dl_lexer);
	tpp_lexer_disablefeature(&self->dl_lexer, TPP_FEAT_CPP_DIRECTIVES);
	tpp_lexer_disablefeature(&self->dl_lexer, TPP_FEAT_CPP_MACROS);

	/* Prevent warnings being emitted */
	tpp_lexer_nowarnings_pushon(&self->dl_lexer);

	/* Save the current token number. */
	tpp_lexer_pushtokennum(&self->dl_lexer);

	/* Ensure that text doesn't get unloaded (so we can always roll back)
	 * At the same time, configure the file to keep everything that makes
	 * up the currently loaded token (which is a `:`-token) in-memory. */
	colon_ptr = DeeParser_GetTokenStart(self);
	ASSERT(colon_ptr == tpp_file_getlastpos(file));
	ASSERT(*colon_ptr == ':');
	ASSERT(DeeParser_GetTok(self) == ':');
	ASSERT(DeeParser_GetTokenLen(self) == 1);
	tpp_file_pushkeep(file, colon_ptr);

	/* Get a persistent descriptor for the in-memory offset of the `:`-token,
	 * which we can later use to restore the current token once all is said
	 * and done. */
	rel_colon_ptr = tpp_file_keep_ptr2rel(file, colon_ptr);

	/* Yield to whatever comes after the `:`-token.
	 * Still use `DeeParser_Yield()` since we need its (or rather:  */
	result = DeeParser_Yield(self);
	if (!TPP_TOK_ISERR(result)) {
		/* Actually do the job of skipping a decl-ast token-sequence. */
		if unlikely(decl_ast_skip(self)) {
			result = TPP_TOK_EDEEMON;
		} else {
			/* decl-ast sequence skipped -> see what the current token is! */
			result = DeeParser_GetTok(self);
		}
	}

	/* At the end, restore the current token as a 1-byte `:`-token
	 * located at `tpp_file_keep_rel2ptr(rel_collon_ptr)` (do this
	 * using `tpp_token_setrange()`)
	 *
	 * With all of this, we're able to temporarily pre-parse the contents
	 * of the current file to check what's up ahead. However, this doesn't
	 * support the case where (syntactically relevant) parts of the decl
	 * expression are hidden inside of macros :(  (i.e. you could still
	 * use macros where keywords are also accepted in decl expressions,
	 * but not for stuff like substituting unmatched `)` or-the-like) */
	colon_ptr = tpp_file_keep_rel2ptr(file, rel_colon_ptr);
	DeeParser_SetTokenRange(self, colon_ptr, colon_ptr + 1);
	DeeParser_SetTokenId(self, TPP_TOK_OFCHAR(':'));

	/* Pop context... */
	tpp_file_popkeep(file);
	tpp_lexer_poptokennum(&self->dl_lexer);
	tpp_lexer_nowarnings_pop(&self->dl_lexer);
	tpp_lexer_popfeatures(&self->dl_lexer);
	tpp_lexer_autopopfile_pop(&self->dl_lexer);
	return result;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	tpp_token_id result;
	struct TPPLexerPosition pos;
	if unlikely(!TPPLexer_SavePosition(&pos))
		goto err;
	if (TPP_TOK_ISERR(DeeParser_Yield(self)))
		goto err_restore_pos;
	if unlikely(decl_ast_skip(self))
		goto err_restore_pos;
	result = DeeParser_GetTok(self);
	TPPLexer_LoadPosition(&pos);
	return result;
err_restore_pos:
	TPPLexer_LoadPosition(&pos);
err:
	return TPP_TOK_EDEEMON;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
}

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DFCALL
get_ctypes_module(DeeLexer *__restrict self, struct ast_loc *loc) {
	DREF DeeModuleObject *result;
	static char const ctypes_name[] = "ctypes";
	result = DeeModule_OpenEx(ctypes_name, COMPILER_STRLEN(ctypes_name), NULL, 0,
	                          DeeModule_IMPORT_F_NORMAL |
	                          DeeModule_IMPORT_F_ENOENT |
	                          DeeModule_IMPORT_F_ERECUR,
	                          DeeCompiler_Current->cp_imports);
	if unlikely(!DeeModule_IMPORT_ISOK(result)) {
		if unlikely(result == DeeModule_IMPORT_ERROR)
			goto err;
		if (result == DeeModule_IMPORT_ENOENT) {
			if (DeeParser_WarnfLoc(self, loc, TPP_W_MODULE_NOT_FOUND,
			                      ctypes_name))
				goto err;
		} else if (result == DeeModule_IMPORT_ERECUR) {
			struct Dee_import_frame *current = DeeThread_Self()->t_import_curr;
			char const *current_name = current ? current->if_absfile : "?";
			if (DeeParser_WarnfLoc(self, loc, TPP_W_RECURSIVE_MODULE_DEPENDENCY,
			                      ctypes_name, current_name))
				goto err;
		}
		result = &DeeModule_Empty;
		Dee_Incref(result);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct symbol *DFCALL
get_ctypes_symbol(DeeLexer *self, struct ast_loc *loc, char const *symbol_name) {
	DREF DeeModuleObject *ctypes;
	struct symbol *extern_symbol;
	struct Dee_module_symbol *ctypes_symbol;
	ctypes = get_ctypes_module(self, loc);
	if unlikely(!ctypes)
		goto err;
	extern_symbol = new_unnamed_symbol();
	if unlikely(!extern_symbol)
		goto err_ctypes;
	ctypes_symbol = DeeModule_GetSymbolString(ctypes, symbol_name);
	if unlikely(!ctypes_symbol) {
		if (DeeParser_Warnf(self, TPP_W_MODULE_IMPORT_NOT_FOUND,
		                   symbol_name, DeeModule_GetShortName(ctypes)))
			goto err_ctypes;
		extern_symbol->s_type   = SYMBOL_TYPE_MODULE;
		extern_symbol->s_module = ctypes; /* Inherit reference. */
	} else {
		/* Setup this symbol as an external reference. */
		extern_symbol->s_type            = SYMBOL_TYPE_EXTERN;
		extern_symbol->s_extern.e_module = ctypes; /* Inherit reference. */
		extern_symbol->s_extern.e_symbol = ctypes_symbol;
	}
	return extern_symbol;
err_ctypes:
	Dee_Decref(ctypes);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF struct ast *DFCALL
get_ctypes_symbol_ast(DeeLexer *self, struct ast_loc *loc, char const *symbol_name) {
	DREF struct ast *result;
	struct symbol *sym;
	sym = get_ctypes_symbol(self, loc, symbol_name);
	if unlikely(!sym)
		goto err;
	result = ast_sym(sym);
	result = ast_setddi(result, loc);
	return result;
err:
	return NULL;
}

/* @return: ITER_DONE: Unknown suffix */
INTERN ATTR_NOINLINE WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_wrap_fixed_length_integer(DeeLexer *self, DeeIntObject *value,
                              tpp_char const *suffix_start, tpp_size suffix_len) {
	size_t i;
	struct ast_loc ctype_loc;
	DREF struct ast *value_ast;
	DREF struct ast *ctype_ast;
	DREF struct ast *args_ast;
	DREF struct ast *call_ast;
	DREF struct ast **argv;
	for (i = 0;; ++i) {
		if (i >= COMPILER_LENOF(fixed_length_suffix_types))
			return (DREF struct ast *)ITER_DONE;
		if (bse_equals(&self->dl_lexer, suffix_start, suffix_len,
		               fixed_length_suffix_types[i].flst_suffix))
			break; /* Found it! */
	}
	value_ast = ast_sethere(self, ast_constexpr(Dee_AsObject(value)));
	if unlikely(!value_ast)
		goto err;
	ctype_loc = value_ast->a_ddi;
	ctype_loc.l_lc = tpp_lexer_getlcinfoat(&self->dl_lexer, suffix_start);
	ctype_ast = get_ctypes_symbol_ast(self, &ctype_loc, fixed_length_suffix_types[i].flst_ctype);
	if unlikely(!ctype_ast)
		goto err_value_ast;
	argv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
	if unlikely(!argv)
		goto err_value_ast_ctype_ast;
	argv[0] = value_ast; /* Inherit reference */
	args_ast = ast_multiple(AST_FMULTIPLE_TUPLE, 1, argv);
	args_ast = ast_setddi(args_ast, &ctype_loc);
	if unlikely(!args_ast)
		goto err_value_ast_ctype_ast_argv;
	call_ast = ast_operator2(OPERATOR_CALL, 0, ctype_ast, args_ast);
	ast_decref(args_ast);
	ast_decref(ctype_ast);
	return ast_setddi(call_ast, &ctype_loc);
err_value_ast_ctype_ast_argv:
	Dee_Free(argv);
err_value_ast_ctype_ast:
	ast_decref(ctype_ast);
err_value_ast:
	ast_decref(value_ast);
err:
	return NULL;
}
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */


INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_unaryhead(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	DREF struct ast *merge;
	struct ast_loc loc;
	switch (DeeParser_GetTok(self)) {

	TPP_CASE_TPP_TOK_INT {
		DREF DeeObject *resval;

		/* Verify that thousands-separators (if present) are used consistently. */
		if unlikely(verify_consistent_grouping(self))
			goto err;

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		{
			tpp_errno error;
			tpp_char const *suffix_start;
			error = tpp_lexer_decodeint_ex(&self->dl_lexer, (DeeIntObject **)&resval, &suffix_start);
			if (TPP_ISERR(error))
				goto err;
			if (suffix_start < DeeParser_GetTokenEnd(self)) {
				/* "-ffixed-length-integrals":
				 * >> 123ull
				 * Compile as:
				 * >> (ullong from ctypes)(123)
				 */
				if (tpp_lexer_getextension(&self->dl_lexer, TPP_EXT_FIXED_LENGTH_INTEGRALS)) {
					tpp_size suffix_len = (tpp_size)(DeeParser_GetTokenEnd(self) - suffix_start);
					result = ast_wrap_fixed_length_integer(self, (DeeIntObject *)resval,
					                                       suffix_start, suffix_len);
					if (result != (DREF struct ast *)ITER_DONE) {
						Dee_Decref(resval);
						if unlikely(!result)
							goto err;
						if (TPP_TOK_ISERR(DeeParser_Yield(self)))
							goto err_r;
						return result;
					}
				}
				if (DeeParser_Warnf(self, TPP_W_INVALID_INTEGER)) {
					Dee_Decref(resval);
					goto err;
				}
			}
		}
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		/* Use our own integer parser, so we can process arbitrary-precision integers. */
		{
			size_t toklen = DeeParser_GetTokenLen(self);
			resval = DeeInt_FromString((char const *)DeeParser_GetTokenStart(self), toklen,
			                           Dee_INT_STRING(0, Dee_INT_STRING_FESCAPED |
			                                             Dee_INT_STRING_FTRY));
		}
		/* Check if the integer failed to be parsed. */
		if unlikely(resval == ITER_DONE) {
			if (DeeParser_Warnf(self, TPP_W_INVALID_INTEGER))
				goto err;
			goto create_none;
		}
create_constexpr:
		if unlikely(!resval)
			goto err;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		result = ast_sethere(self, ast_constexpr(resval));
		Dee_Decref(resval);
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		return result;

#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
	TPP_CASE_TPP_TOK_STRING_SQUOTE
		if (!DeeParser_Has(self, CHARACTER_LITERALS)) {
			goto decode_string;
		} else {
			tint_t value;
			if unlikely(TPP_Atoi(&value) == TPP_ATOI_ERR)
				goto err;
			if (DeeParser_Warnf(self, TPP_W_DEPRECATED_CHARACTER_INT))
				goto err;
			resval = DeeInt_NewInt64(value);
		}
		goto create_constexpr;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	}	break;


#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	TPP_CASE_TPP_TOK_STRING_SQUOTE {
		tpp_errno error;
		tpp_uintmax value;
		DREF DeeObject *resval;
		if (!DeeParser_Has(self, BUILTIN_EXPR_CHARACTER_LITERALS))
			goto decode_string;
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_CHARACTER_INT))
			goto err;
		error = tpp_lexer_parsecharacter_literal(&self->dl_lexer, &value,
		                                         TPP_LEXER_PARSESTRING_FLAG_STOPONLF);
		if (TPP_ISERR(error))
			goto err;
		resval = DeeInt_NEWU(value);
		if unlikely(!resval)
			goto err;
		result = ast_constexpr(resval);
		Dee_Decref(resval);
		return ast_setddi(result, &loc);
	}	break;
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */


	TPP_CASE_TPP_TOK_STRING_DQUOTE {
decode_string:
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		return ast_parse_string_ast(self);
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		{
			DREF DeeObject *resval = ast_parse_string_const(self);
			if unlikely(!resval)
				goto err;
			result = ast_setddi(ast_constexpr(resval), &loc);
			Dee_Decref(resval);
		}
		return result;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	}	break;

#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
	case TPP_KWD_f:
	case TPP_KWD_F:
		/* Check if this might be a template string. */
		if ((*TPPLexer_Current->l_token.t_end == '\"') ||
		    (*TPPLexer_Current->l_token.t_end == '\'' && !DeeParser_Has(self, CHARACTER_LITERALS))) {
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
			ASSERT(TPP_TOK_ISSTRING(DeeParser_GetTok(self)));
			return ast_parse_template_string(self);
		}
		goto do_keyword;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

	TPP_CASE_TPP_TOK_FLOAT {
		tpp_float value;
		DREF DeeObject *resval;
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		tpp_errno error;
		tpp_char const *suffix_start;
		error = tpp_lexer_decodefloat_ex(&self->dl_lexer, &value, &suffix_start);
		if (TPP_ISERR(error))
			goto err;
		if (suffix_start < DeeParser_GetTokenEnd(self)) {
			if (DeeParser_Warnf(self, TPP_W_INVALID_FLOAT))
				goto err;
		}
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		if (TPP_Atof(&value) == TPP_ATOF_ERR)
			goto err;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		resval = DeeFloat_New((double)value);
		if unlikely(!resval)
			goto err;
		/* Construct a new branch for the constant float value. */
		result = ast_sethere(self, ast_constexpr(resval));
		Dee_Decref(resval);
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		return result;
	}	break;

	case TPP_KWD_none: {
		DeeObject *constval;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
create_none:
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		constval = Dee_None;
mkconst:
		result = ast_sethere(self, ast_constexpr(constval));
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetTok(self) == '(') {
			result = ast_parse_unaryhead(self,
			                             LOOKUP_SYM_SECONDARY |
			                             PARSE_UNARY_DISALLOW_CASTS);
		} else {
			result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
		}
		if unlikely(!result)
			goto err;
		merge = make_bound_expression(self, result, &loc);
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetTok(self) == '(') {
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetTok(self) == '(') {
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
	case TPP_TOK_LANGLE_RANGLE:
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_CELL_SYNTAX))
			goto err;
		goto do_empty_cell;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

	case '<': /* Cell (deprecated syntax) */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_CELL_SYNTAX))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetTok(self) == '>') {
			/* empty Cell. */
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
do_empty_cell:
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
			result = ast_action0(AST_FACTION_CELL0);
			if unlikely(!result)
				goto err;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
		} else {
			result = ast_parse_unary(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!result)
				goto err;
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR('>'), TPP_W_EXPECTED_RANGLE_AFTER_LANGLE))
				goto err_r;
			merge = ast_action1(AST_FACTION_CELL1, result);
			ast_decref(result);
			result = merge;
		}
		return ast_setddi(result, &loc);

	case TPP_KWD_if: {
		/* if-in-expressions. */
		DREF struct ast *tt_branch;
		DREF struct ast *ff_branch;
		uint16_t expect;
		bool has_paren;
		expect = current_tags.at_expect;
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_if_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (DeeParser_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_IF))
			goto err_if_flags;
		result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeParser_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_IF))
			goto err;
		tt_branch = NULL;
		if (DeeParser_GetTok(self) != TPP_KWD_else && DeeParser_GetTok(self) != TPP_KWD_elif) {
			tt_branch = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!tt_branch)
				goto err_r;
		}
		ff_branch = NULL;
		if (DeeParser_GetTok(self) == TPP_KWD_elif) {
			DeeParser_SetTokenId(self, TPP_KWD_if); /* Cheat a bit... */
			goto do_else_branch;
		}
		if (DeeParser_GetTok(self) == TPP_KWD_else) {
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
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
		tpp_keyword const *function_name;
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_FUNCTION_IN_EXPRESSION))
			goto err;
		/* Create a new function */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		function_name = NULL;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_HasTokenKwd(self)) {
			function_name = DeeParser_GetTokenKwd(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
		}
		return ast_parse_function(self, function_name, NULL, false, &loc, NULL, NULL);
	}	break;

	case TPP_KWD_final: {
		tpp_keyword const *class_name;
		uint16_t class_flags;
		class_flags = TP_FFINAL;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		if (TPP_TOK_ISERR(DeeParser_Require(self, TPP_KWD_class)))
			goto err;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		if (unlikely(DeeParser_GetTok(self) != TPP_KWD_class)) {
			if (DeeParser_Warnf(self, TPP_W_EXPECTED_CLASS_AFTER_FINAL))
				goto err;
		}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		goto do_create_class;
	case TPP_KWD_class:
		class_flags = TP_FNORMAL;
do_create_class:
		/* Create a new function */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		class_name = NULL;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_HasTokenKwd(self)) {
			if (DeeParser_GetTok(self) == TPP_KWD_final && !(class_flags & TP_FFINAL)) {
				/* allow `class final` as an alias for `final class` */
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
					goto err;
				class_flags |= TP_FFINAL;
			}
			if (DeeParser_HasTokenKwd(self)) {
				class_name = DeeParser_GetTokenKwd(self);
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		bool inside_macro = tpp_file_ismacro(DeeParser_GetFile(self));
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		has_paren = 0;
		DeeParser_NoLf_Push(self);
		if (DeeParser_GetTok(self) == '(') {
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_pack_flags:
				DeeParser_NoLf_Break(self);
				goto err;
			}
			has_paren = DeeParser_GetTok(self) == '(' ? 2 : 1;
		} else if (!inside_macro) {
			if (DeeParser_WarnfLoc(self, &loc, TPP_W_PACK_USED_OUTSIDE_OF_MACRO))
				goto err_pack_flags;
		}
		if (DeeParser_GetTok(self) == '{') {
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
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (has_paren) {
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_PACK))
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_lparen_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		allow_cast = DeeParser_GetTok(self) != '(' && !(lookup_mode & PARSE_UNARY_DISALLOW_CASTS);
		if (DeeParser_GetTok(self) != '{') {
			if (DeeParser_GetTok(self) == ')') {
				DeeParser_NoLf_Break(self);
				if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_LPAREN))
					goto err;

				/* Support for java-style lambda with empty argument list. */
				if (DeeParser_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
					result = ast_parse_function_java_lambda(self, NULL, NULL);
					result = ast_setddi(result, &loc);
					return result;
				}
				if (DeeParser_GetTok(self) == ':') {
					tpp_token_id tok_after_decl;
					tok_after_decl = peek_token_after_decl_ast_skip(self);
					if (TPP_TOK_ISERR(tok_after_decl))
						goto err;
					if (tok_after_decl == TPP_TOK_MINUS_RANGLE) {
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
					DeeParser_NoLf_Break(self);
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
				DeeParser_NoLf_Break(self);
				if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_LPAREN))
					goto err_r;
			}
			if (DeeParser_GetTok(self) == '{' && !allow_cast) {
				if (DeeParser_Warnf(self, TPP_W_PROBABLY_MISSING_ARROW))
					goto err_r;
			}
			goto set_lparen_ddi;
		}

		/* Statements in expressions. */
		result = ast_parse_statement_or_braces(self, &was_expression);
		if unlikely(!result)
			goto err_lparen_flags;
		allow_cast = false; /* Don't allow braces, or statements as cast expressions. */
		if (DeeParser_GetTok(self) == ',' && was_expression != AST_PARSE_WASEXPR_NO) {
			DREF struct ast **tuple_branchv;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_lparen_flags_r;
			if (DeeParser_GetTok(self) == ')') {
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
		if (DeeParser_GetTok(self) != ')' && was_expression != AST_PARSE_WASEXPR_NO) {
			result = ast_parse_postexpr(self, result);
			if unlikely(!result)
				goto err_lparen_flags;
			/*was_expression = AST_PARSE_WASEXPR_YES;*/
		}
		DeeParser_NoLf_Pop(self);
		if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_LPAREN))
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
/*err_lbrace_flags:*/
			DeeParser_NoLf_Break(self);
			goto err;
		}
		result = ast_setddi(ast_parse_brace_items(self), &loc);
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeParser_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_BRACEINIT))
			goto err_r;
		return result;

	case TPP_KWD_del: {
		bool has_paren;

		/* Delete expression. */
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_del_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (DeeParser_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_DEL))
			goto err_del_flags;
		result = ast_putddi(ast_parse_del(self, lookup_mode & ~PARSE_UNARY_DISALLOW_CASTS), &loc);
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeParser_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_DEL))
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
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err;
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		name = ast_parse_operator_name(self, P_OPERATOR_FNORMAL);
		if unlikely(name < 0)
			goto err;
		if (DeeParser_GetTok(self) != '(') {
			/* Bound-operator expression. */
			merge = ast_operator_func((Dee_operator_t)name, NULL);
		} else {
			DeeParser_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
/*err_operator_flags:*/
				DeeParser_NoLf_Break(self);
				goto err;
			}
			if (DeeParser_GetTok(self) == ')') {
				result = ast_constexpr(Dee_EmptyTuple);
			} else {
				result = ast_parse_comma(self,
				                        AST_COMMA_FORCEMULTIPLE,
				                        AST_FMULTIPLE_TUPLE,
				                        NULL);
			}
			DeeParser_NoLf_Pop(self);
			if unlikely(!result)
				goto err;
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN))
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
		if (DeeParser_GetLoc(self, &loc))
			goto err;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_lbracket_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (DeeParser_GetTok(self) == '&' ||
		    DeeParser_GetTok(self) == '=') {
			if (DeeParser_Warnf(self, TPP_W_DEPRECATED_LAMBDA_MODE))
				goto err_lbracket_flags;
			if (DeeParser_GetLoc(self, &loc))
				goto err_lbracket_flags;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_lbracket_flags;
			DeeParser_NoLf_Break(self);
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(']'), W_EXPECTED_RBRACKET_AFTER_LAMBDA))
				goto err;
			goto do_lambda;
		}
		if (DeeParser_GetTok(self) == ']') {
			DeeParser_NoLf_Break(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
			if (DeeParser_GetTok(self) == ':') {
				/* If the current token is ':', try to skip over the return type
				 * annotation and check if the next token thereafter is '->' or '{'. */
				tpp_token_id tok_after_decl;
				tok_after_decl = peek_token_after_decl_ast_skip(self);
				if (TPP_TOK_ISERR(tok_after_decl))
					goto err;
				if (tok_after_decl == '{' || tok_after_decl == TPP_TOK_MINUS_RANGLE)
					goto do_lambda; /* Yup: it's a lambda! */
			}
			if (DeeParser_GetTok(self) == '(' ||
			    DeeParser_GetTok(self) == '{' ||
			    DeeParser_GetTok(self) == TPP_TOK_MINUS_RANGLE)
				goto do_lambda;
			result = ast_multiple(AST_FMULTIPLE_LIST, 0, NULL);
			goto set_list_loc;
		}
		if (DeeParser_GetTok(self) == ':') {
			DREF struct ast *start_expression;
			DREF struct ast *step_expression;
			result = ast_constexpr(Dee_None);
			if unlikely(!result)
				goto err_lbracket_flags;
			/* Range without begin index. */
do_range_expression:
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_lbracket_flags_r:
				ast_decref(result);
				goto err_lbracket_flags;
			}
			start_expression = result; /* Inherit reference. */
			/* Parse the end index. */
			if (DeeParser_GetTok(self) == ',') {
				/* No end index given. */
				result = ast_constexpr(Dee_None);
			} else {
				result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			}
			if unlikely(!result) {
				ast_decref(start_expression);
				goto err_lbracket_flags;
			}
			if (DeeParser_GetTok(self) == ',') {
				if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
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
			if (DeeParser_GetTok(self) == ':' &&
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
		DeeParser_NoLf_Pop(self);
		if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(']'), W_EXPECTED_RBRACKET_AFTER_LIST))
			goto err_r;
set_list_loc:
		return ast_setddi(result, &loc);
do_lambda:
		DeeParser_NoLf_Push(self);
		if unlikely(parse_tags_block(self)) {
/*err_lambda_flags:*/
			DeeParser_NoLf_Break(self);
			goto err;
		}
		result = ast_parse_function(self, NULL, NULL, true, &loc, NULL, NULL);
		DeeParser_NoLf_Pop(self);
		return result;

	case TPP_KWD_this: {
		struct symbol *this_sym;
		this_sym = get_current_this();
		if (!this_sym)
			goto default_case;
		result = ast_sethere(self, ast_sym(this_sym));
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		return result;
	}	break;

	case TPP_KWD___nth: {
		bool has_paren;
		DeeParser_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_nth_flags:
			DeeParser_NoLf_Break(self);
			goto err;
		}
		if (DeeParser_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_NTH))
			goto err_nth_flags;
		result = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
		DeeParser_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		/* Optimize the ast-expression to propagate constant, thus
		 * allowing the use of `__nth(2+3)` instead of forcing the
		 * user to write `__nth(5)` or `__nth(__TPP_EVAL(2+3))` */
		if (ast_optimize_all(result, true))
			goto err_r;
		if (result->a_type != AST_CONSTEXPR) {
			if (DeeParser_Warnf(self, TPP_W_EXPECTED_CONSTANT_AFTER_NTH))
				goto err_r;
		}
		if (DeeParser_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_NTH))
			goto err_r;
		if (DeeParser_HasTokenKwd(self)) {
			unsigned int nth_symbol = 0;
			struct symbol *sym;
			if (result->a_type == AST_CONSTEXPR &&
			    DeeObject_AsUInt(result->a_constexpr, &nth_symbol)) {
				DeeError_Handled(ERROR_HANDLED_RESTORE);
				if (DeeParser_Warnf(self, TPP_W_EXPECTED_CONSTANT_AFTER_NTH))
					goto err_r;
			}
			ast_decref(result);
			sym = lookup_nth(self, nth_symbol, DeeParser_GetTokenKwd(self));
			if likely(sym) {
				result = ast_sym(sym);
			} else {
				if (DeeParser_Warnf(self, TPP_W_UNKNOWN_NTH_SYMBOL, nth_symbol))
					goto err;
				result = ast_constexpr(Dee_None);
			}
			if unlikely(!result)
				goto err;
			result = ast_sethere(self, result);
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
		} else {
			if (DeeParser_Warnf(self, TPP_W_EXPECTED_KEYWORD_AFTER_NTH))
				goto err_r;
		}
		return result;
	}	break;

	case TPP_TOK_COLON_COLON:
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_GLOBAL_PREFIX))
			goto err;
		ATTR_FALLTHROUGH
	case TPP_KWD_global:
		lookup_mode |= LOOKUP_SYM_VGLOBAL;
		goto do_warn_deprecated_modifier;
	case TPP_KWD_local:
		lookup_mode |= LOOKUP_SYM_VLOCAL;
do_warn_deprecated_modifier:
		if (DeeParser_Warnf(self, TPP_W_DEPRECATED_PREFIX_IN_EXPRESSION))
			goto err;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
			return result;
		}
		ATTR_FALLTHROUGH
	default:
default_case:
		if (DeeParser_HasTokenKwd(self)) {
			/* Perform a regular symbol lookup. */
			tpp_keyword const *name;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
do_keyword:
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
			name = DeeParser_GetTokenKwd(self);
			if (DeeParser_GetLoc(self, &loc))
				goto err;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err;
			if (DeeParser_GetTok(self) == TPP_KWD_from) {
				/* `Error from deemon` - Short form of `import Error from deemon` */
				if (DeeParser_GetLoc(self, &loc))
					goto err;
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
					goto err;
				result = ast_parse_import_single(self, name);
				if (result && (DeeParser_GetTok(self) != ';' &&
				               DeeParser_GetTok(self) != ',' &&
				               DeeParser_GetTok(self) != ')' &&
				               DeeParser_GetTok(self) != '}' &&
				               DeeParser_GetTok(self) != ']' &&
				               DeeParser_GetTok(self) != '\n' &&
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
				               TPPLexer_Current->l_token.t_id != TOK_ERR &&
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
				               /* Don't emit this warning from macros! */
				               !tpp_file_ismacro(DeeParser_GetFile(self)))) {
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
					if (DeeParser_WarnfLoc(self, &loc, TPP_W_UNCLEAR_SYMBOL_FROM_MODULE,
					                      symname, modname))
						goto err;
				}
			} else if (DeeParser_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
				/* Support for java-style lambda with singular argument. */
				result = ast_parse_function_java_lambda(self, name, &loc);
			} else {
				DREF struct symbol *sym;
				sym = lookup_symbol(self, lookup_mode & ~PARSE_UNARY_DISALLOW_CASTS, name, &loc);
				if unlikely(!sym)
					goto err;
				result = ast_sym(sym);
			}
		} else {
			if (DeeParser_Warnf(self, TPP_W_UNEXPECTED_TOKEN_IN_EXPRESSION))
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
		switch (DeeParser_GetTok(self)) {

		case TPP_TOK_COLON_COLON:
			/* Backwards compatibility with deemon 100+ */
			if (DeeParser_Warnf(self, TPP_W_DEPRECATED_ATTRIBUTE_SYNTAX))
				goto err_r;
			ATTR_FALLTHROUGH
		case '.': /* Attribute lookup */
			if (DeeParser_GetLoc(self, &loc))
				goto err_r;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
			if (DeeParser_HasTokenKwd(self)) {
				DREF DeeObject *attr_name;
				if (DeeParser_GetTok(self) == TPP_KWD_this)
					goto got_attr; /* This is really just a no-op. */
				if (DeeParser_GetTok(self) == TPP_KWD_class) {
					/* Return the real class of a type, properly unwinding super. */
					merge = ast_sethere(self, ast_action1(AST_FACTION_CLASSOF, result));
				} else if (DeeParser_GetTok(self) == TPP_KWD_super) {
					/* Return the real class of a type, properly unwinding super. */
					merge = ast_sethere(self, ast_action1(AST_FACTION_SUPEROF, result));
				} else if (DeeParser_GetTok(self) == TPP_KWD_operator) {
					/* Named & bound operator invocation. */
					int32_t name;
					if (TPP_TOK_ISERR(DeeParser_Yield(self)))
						goto err_r;
					if (DeeParser_GetLoc(self, &loc))
						goto err_r;
					name = ast_parse_operator_name(self, P_OPERATOR_FNORMAL);
					if unlikely(name < 0)
						goto err_r;
					if (DeeParser_GetTok(self) != '(') {
						/* Bound-operator expression. */
						merge = ast_operator_func((Dee_operator_t)name, result);
					} else {
						DeeParser_NoLf_Push(self);
						if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
/*err_r_operator_lparen_flags:*/
							DeeParser_NoLf_Break(self);
							goto err_r;
						}
						if (DeeParser_GetTok(self) == ')') {
							other = ast_constexpr(Dee_EmptyTuple);
						} else {
							other = ast_parse_comma(self,
							                        AST_COMMA_FORCEMULTIPLE,
							                        AST_FMULTIPLE_TUPLE,
							                        NULL);
						}
						DeeParser_NoLf_Pop(self);
						if unlikely(!other)
							goto err_r;
						if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN))
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
					if (DeeParser_IsIdentifier(self, DeeParser_GetTokenKwd(self))) {
						if (DeeParser_Warnf(self, TPP_W_RESERVED_ATTRIBUTE_NAME,
						                   DeeParser_GetTokenKwdCStr(self)))
							goto err;
					}
					attr_name = DeeString_NewSized(DeeParser_GetTokenKwdCStr(self),
					                               DeeParser_GetTokenKwdLen(self));
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
					if (TPP_TOK_ISERR(DeeParser_Yield(self)))
						goto err_r;
					goto got_attr2;
				}
				ast_decref(result);
				if unlikely(!merge)
					goto err;
				result = merge;
got_attr:
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
					goto err_r;
got_attr2:;
			} else {
				if (DeeParser_Warnf(self, TPP_W_EXPECTED_KEYWORD_AFTER_DOT))
					goto err_r;
			}
			break;

		case '[': {
			/* sequence operator */
			if (DeeParser_GetLoc(self, &loc))
				goto err_r;
			DeeParser_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_r_lbracket_flags:
				DeeParser_NoLf_Break(self);
				goto err_r;
			}
			if (DeeParser_GetTok(self) == ':') {
				other = ast_constexpr(Dee_None);
				if unlikely(!other)
					goto err_r_lbracket_flags;
				goto do_range;
			}
			other = ast_parse_expr(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!other)
				goto err_r_lbracket_flags;
			if (DeeParser_GetTok(self) == ':') {
				DREF struct ast *third;
do_range:
				/* range operator. */
				if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_r_lbracket_flags_other:
					ast_decref(other);
					goto err_r_lbracket_flags;
				}
				if (DeeParser_GetTok(self) == ']') {
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
			DeeParser_NoLf_Pop(self);
			ast_decref(other);
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(']'), W_EXPECTED_RBRACKET_AFTER_GETITEM))
				goto err_r;
			ast_setddi(result, &loc);
		}	break;

		case '{': /* Brace initializers. */
			if (DeeParser_GetLoc(self, &loc))
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
			bool inside_macro = tpp_file_ismacro(DeeParser_GetFile(self));
			/* Call expression without parenthesis. */
			if (DeeParser_GetLoc(self, &loc))
				goto err_r;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
			if (DeeParser_GetTok(self) == '(')
				goto do_normal_call_with_loc;
			if (!inside_macro) {
				if (DeeParser_WarnfLoc(self, &loc, TPP_W_PACK_USED_OUTSIDE_OF_MACRO))
					goto err_r;
			}
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
			if (DeeParser_GetLoc(self, &loc))
				goto err_r;
do_normal_call_with_loc:
			DeeParser_NoLf_Push(self);
			if (TPP_TOK_ISERR(DeeParser_Yield(self))) {
err_r_lparen_flags:
				DeeParser_NoLf_Break(self);
				goto err_r;
			}
			if (DeeParser_GetTok(self) == ')') {
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
			DeeParser_NoLf_Pop(self);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (DeeParser_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_CALL))
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
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
		}	break;

		case TPP_TOK_DOT_DOT_DOT: /* Expand expression. */
			merge = ast_sethere(self, ast_expand(result));
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
	tpp_token_id cmd = DeeParser_GetTok(self);
	ASSERT(TOKEN_IS_PROD(cmd));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		cmd = DeeParser_GetTok(self);
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
	tpp_token_id cmd = DeeParser_GetTok(self);
	ASSERT(TOKEN_IS_SUM(cmd));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		if (DeeParser_GetTok(self) == TPP_TOK_DOT_DOT_DOT && cmd == '+') { /* sum */
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		cmd = DeeParser_GetTok(self);
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
	tpp_token_id cmd = DeeParser_GetTok(self);
	ASSERT(TOKEN_IS_SHIFT(cmd));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		cmd = DeeParser_GetTok(self);
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
	tpp_token_id cmd = DeeParser_GetTok(self);
	ASSERT(TOKEN_IS_CMP(cmd));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		if (DeeParser_GetTok(self) == TPP_TOK_DOT_DOT_DOT && (cmd == '<' || cmd == '>')) {
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		cmd = DeeParser_GetTok(self);
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
	tpp_token_id cmd = DeeParser_GetTok(self);
	ASSERT(TOKEN_IS_CMPEQ(cmd));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (cmd == TPP_TOK_QMARK_QMARK) {
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
			if (DeeParser_GetTok(self) == '!') {
				invert ^= 1;
				goto yield_again;
			}
			if (cmd == '!') {
				if (DeeParser_GetTok(self) == TPP_KWD_is || DeeParser_GetTok(self) == TPP_KWD_in) {
					cmd = DeeParser_GetTok(self);
					if (TPP_TOK_ISERR(DeeParser_Yield(self)))
						goto err_r;
				} else {
					/* TODO: Must rewind to start of '!'-sequence in this case and
					 *       not try to parse any additional in/is expressions:
					 * >> local foo = "value";
					 * >> local bar = f"foo = {foo!r}"; // << we mustn't parse the '!' here!
					 *
					 * iow: the `W_EXPECTED_IS_OR_IN_AFTER_EXCLAIM` warning needs to go away
					 */
					if (DeeParser_Warnf(self, TPP_W_EXPECTED_IS_OR_IN_AFTER_EXCLAIM))
						goto err_r;
					cmd = TPP_KWD_is;
				}
			}
			if (DeeParser_GetTok(self) == TPP_KWD_bound && cmd == TPP_KWD_is) {
				/* Special cast: `foo is bound` --> `bound(foo)` */
				if (TPP_TOK_ISERR(DeeParser_Yield(self)))
					goto err_r;
				merge = make_bound_expression(self, lhs, &loc);
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
		cmd = DeeParser_GetTok(self);
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
	ASSERT(TOKEN_IS_AND(DeeParser_GetTok(self)));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (!TOKEN_IS_AND(DeeParser_GetTok(self)))
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
	ASSERT(TOKEN_IS_XOR(DeeParser_GetTok(self)));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (!TOKEN_IS_XOR(DeeParser_GetTok(self)))
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
	ASSERT(TOKEN_IS_OR(DeeParser_GetTok(self)));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (!TOKEN_IS_OR(DeeParser_GetTok(self)))
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
	ASSERT(TOKEN_IS_AS(DeeParser_GetTok(self)));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (!TOKEN_IS_AS(DeeParser_GetTok(self)))
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
	ASSERT(TOKEN_IS_LAND(DeeParser_GetTok(self)));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		if (DeeParser_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (!TOKEN_IS_LAND(DeeParser_GetTok(self)))
			break;
	}
	if (TOKEN_IS_LOR(DeeParser_GetTok(self))) {
		if (DeeParser_Warnf(self, TPP_W_PAREN_AROUND_LAND))
			goto err_r;
	}
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
	ASSERT(TOKEN_IS_LOR(DeeParser_GetTok(self)));
	for (;;) {
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		if (DeeParser_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
				goto err_r;
			merge = ast_action1(AST_FACTION_ANY, lhs);
		} else {
			rhs = ast_parse_lor(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!rhs)
				goto err_r;
			if (TOKEN_IS_LAND(DeeParser_GetTok(self))) {
				/* Suggest parenthesis around logical-and. */
				if (DeeParser_Warnf(self, TPP_W_PAREN_AROUND_LAND))
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
		if (!TOKEN_IS_LOR(DeeParser_GetTok(self)))
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
	ASSERT(TOKEN_IS_COND(DeeParser_GetTok(self)));
	for (;;) {
		uint16_t expect;
		expect = current_tags.at_expect;
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		if (DeeParser_GetTok(self) == ':') {
			/* Missing true-branch. (Reuse the condition branch!) */
			tt = lhs;
			ast_incref(lhs);
		} else {
			tt = ast_parse_cond(self, LOOKUP_SYM_SECONDARY);
			if unlikely(!tt)
				goto err_r;
		}
		if (DeeParser_GetTok(self) == ':') {
			/* Parse the false-branch. */
			if (TPP_TOK_ISERR(DeeParser_Yield(self)))
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
		if (!TOKEN_IS_COND(DeeParser_GetTok(self)))
			break;
	}
	return lhs;
err_tt:
	ast_decref(tt);
err_r:
	ast_decref(lhs);
	return NULL;
}



#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DFCALL
ast_parse_assign_operand(DeeLexer *self, /*inherit(always)*/ DREF struct ast *__restrict lhs) {
	DREF struct ast *rhs, *merge;
	struct ast_loc loc;
	tpp_token_id cmd = DeeParser_GetTok(self);
	ASSERT(TOKEN_IS_ASSIGN(cmd));
	for (;;) {
		Dee_operator_t operator_name;
		if (DeeParser_GetLoc(self, &loc))
			goto err_r;
		if (TPP_TOK_ISERR(DeeParser_Yield(self)))
			goto err_r;
		rhs = ast_parse_cond(self, LOOKUP_SYM_SECONDARY);
		if unlikely(!rhs)
			goto err_r;
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		switch (cmd) {
		case TPP_TOK_COLON_EQUAL:
			operator_name = AST_SHOULD_MOVEASSIGN(rhs)
			                ? OPERATOR_MOVEASSIGN
			                : OPERATOR_ASSIGN;
			break;
		case TPP_TOK_PLUS_EQUAL: operator_name = OPERATOR_INPLACE_ADD; break;
		case TPP_TOK_MINUS_EQUAL: operator_name = OPERATOR_INPLACE_SUB; break;
		case TPP_TOK_STAR_EQUAL: operator_name = OPERATOR_INPLACE_MUL; break;
		case TPP_TOK_SLASH_EQUAL: operator_name = OPERATOR_INPLACE_DIV; break;
		case TPP_TOK_PERCENT_EQUAL: operator_name = OPERATOR_INPLACE_MOD; break;
		case TPP_TOK_LANGLE_LANGLE_EQUAL: operator_name = OPERATOR_INPLACE_SHL; break;
		case TPP_TOK_RANGLE_RANGLE_EQUAL: operator_name = OPERATOR_INPLACE_SHR; break;
		case TPP_TOK_AMP_EQUAL: operator_name = OPERATOR_INPLACE_AND; break;
		case TPP_TOK_PIPE_EQUAL: operator_name = OPERATOR_INPLACE_OR; break;
		case TPP_TOK_HAT_EQUAL: operator_name = OPERATOR_INPLACE_XOR; break;
		case TPP_TOK_STAR_STAR_EQUAL: operator_name = OPERATOR_INPLACE_POW; break;
		default: __builtin_unreachable();
		}
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		if (cmd == TPP_TOK_COLON_EQUAL) {
			/* Special case: move-assign. */
			operator_name = AST_SHOULD_MOVEASSIGN(rhs)
			                ? OPERATOR_MOVEASSIGN
			                : OPERATOR_ASSIGN;
		} else {
			/* Inplace operation. */
			operator_name = inplace_fops[cmd - TOK_INPLACE_MIN];
		}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		merge = ast_operator2(operator_name, 0, lhs, rhs);
		ast_decref(rhs);
		ast_decref(lhs);
		lhs = ast_setddi(merge, &loc);
		if unlikely(!lhs)
			break;
		cmd = DeeParser_GetTok(self);
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
	if (likely(result) && TOKEN_IS_PROD(DeeParser_GetTok(self)))
		result = ast_parse_prod_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_sum(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_prod(self, lookup_mode);
	if (likely(result) && TOKEN_IS_SUM(DeeParser_GetTok(self)))
		result = ast_parse_sum_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_shift(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_sum(self, lookup_mode);
	if (likely(result) && TOKEN_IS_SHIFT(DeeParser_GetTok(self)))
		result = ast_parse_shift_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_cmp(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_shift(self, lookup_mode);
	if (likely(result) && TOKEN_IS_CMP(DeeParser_GetTok(self)))
		result = ast_parse_cmp_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_cmpeq(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_cmp(self, lookup_mode);
	if (likely(result) && TOKEN_IS_CMPEQ(DeeParser_GetTok(self)))
		result = ast_parse_cmpeq_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_and(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_cmpeq(self, lookup_mode);
	if (likely(result) && TOKEN_IS_AND(DeeParser_GetTok(self)))
		result = ast_parse_and_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_xor(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_and(self, lookup_mode);
	if (likely(result) && TOKEN_IS_XOR(DeeParser_GetTok(self)))
		result = ast_parse_xor_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_or(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_xor(self, lookup_mode);
	if (likely(result) && TOKEN_IS_OR(DeeParser_GetTok(self)))
		result = ast_parse_or_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_as(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_or(self, lookup_mode);
	if (likely(result) && TOKEN_IS_AS(DeeParser_GetTok(self)))
		result = ast_parse_as_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_land(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_as(self, lookup_mode);
	if (likely(result) && TOKEN_IS_LAND(DeeParser_GetTok(self)))
		result = ast_parse_land_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_lor(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_land(self, lookup_mode);
	if (likely(result) && TOKEN_IS_LOR(DeeParser_GetTok(self)))
		result = ast_parse_lor_operand(self, result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_cond(DeeLexer *self, unsigned int lookup_mode) {
	DREF struct ast *result;
	result = ast_parse_lor(self, lookup_mode);
	if (likely(result) && TOKEN_IS_COND(DeeParser_GetTok(self)))
		result = ast_parse_cond_operand(self, result);
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_assign(DeeLexer *self, unsigned int lookup_mode) {
#ifdef __OPTIMIZE_SIZE__
	DREF struct ast *result;
	result = ast_parse_cond(self, lookup_mode);
	if (likely(result) && TOKEN_IS_ASSIGN(DeeParser_GetTok(self)))
		result = ast_parse_assign_operand(self, result);
	return result;
#elif 1
	DREF struct ast *result;
	result = ast_parse_unary(self, lookup_mode);
	if unlikely(!result)
		goto done;
	switch (DeeParser_GetTok(self)) {
	CASE_TOKEN_IS_PROD:
		result = ast_parse_prod_operand(self, result);
		if unlikely(!result)
			goto done;
		if (TOKEN_IS_SUM(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_SUM:
			result = ast_parse_sum_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_SHIFT(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_SHIFT:
			result = ast_parse_shift_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_CMP(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_CMP:
			result = ast_parse_cmp_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_CMPEQ(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_CMPEQ:
			result = ast_parse_cmpeq_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_AND(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_AND:
			result = ast_parse_and_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_XOR(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_XOR:
			result = ast_parse_xor_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_OR(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_OR:
			result = ast_parse_or_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_AS(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_AS:
			result = ast_parse_as_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_LAND(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_LAND:
			result = ast_parse_land_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_LOR(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_LOR:
			result = ast_parse_lor_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_COND(DeeParser_GetTok(self))) {
	CASE_TOKEN_IS_COND:
			result = ast_parse_cond_operand(self, result);
			if unlikely(!result)
				goto done;
		}
		if (TOKEN_IS_ASSIGN(DeeParser_GetTok(self))) {
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
	if (TOKEN_IS_PROD(DeeParser_GetTok(self))) {
		result = ast_parse_prod_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_SUM(DeeParser_GetTok(self))) {
		result = ast_parse_sum_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_SHIFT(DeeParser_GetTok(self))) {
		result = ast_parse_shift_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_CMP(DeeParser_GetTok(self))) {
		result = ast_parse_cmp_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_CMPEQ(DeeParser_GetTok(self))) {
		result = ast_parse_cmpeq_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_AND(DeeParser_GetTok(self))) {
		result = ast_parse_and_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_XOR(DeeParser_GetTok(self))) {
		result = ast_parse_xor_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_OR(DeeParser_GetTok(self))) {
		result = ast_parse_or_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_AS(DeeParser_GetTok(self))) {
		result = ast_parse_as_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_LAND(DeeParser_GetTok(self))) {
		result = ast_parse_land_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_LOR(DeeParser_GetTok(self))) {
		result = ast_parse_lor_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_COND(DeeParser_GetTok(self))) {
		result = ast_parse_cond_operand(self, result);
		if unlikely(!result)
			goto done;
	}
	if (TOKEN_IS_ASSIGN(DeeParser_GetTok(self))) {
		result = ast_parse_assign_operand(self, result);
		if unlikely(!result)
			goto done;
	}
done:
	return result;
#else
	DREF struct ast *result;
	result = ast_parse_cond(self, lookup_mode);
	if (likely(result) && TOKEN_IS_ASSIGN(DeeParser_GetTok(self)))
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
	if (TOKEN_IS_PROD(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_prod_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_SUM(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_sum_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_SHIFT(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_shift_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_CMP(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_cmp_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_CMPEQ(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_cmpeq_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_AND(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_and_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_XOR(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_xor_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_OR(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_or_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_AS(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_as_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_LAND(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_land_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_LOR(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_lor_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_COND(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_cond_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
	if (TOKEN_IS_ASSIGN(DeeParser_GetTok(self))) {
		baseexpr = ast_parse_assign_operand(self, baseexpr);
		if unlikely(!baseexpr)
			goto done;
	}
done:
	return baseexpr;
}
#endif /* !CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_PARSER_EXPRESSION_C */
