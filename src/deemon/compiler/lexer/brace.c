/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_LEXER_BRACE_C
#define GUARD_DEEMON_COMPILER_LEXER_BRACE_C 1

#include <deemon/HashSet.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>

DECL_BEGIN

INTERN DREF struct ast *FCALL
ast_parse_mapping(struct ast *__restrict initial_key) {
	size_t elema, elemc;
	DREF struct ast *result;
	DREF struct ast **elemv, *item;
	/* Parse the associated item. */
	item = ast_parse_expr(LOOKUP_SYM_NORMAL);
	if unlikely(!item)
		goto err;
	elema = 1, elemc = 0;
	elemv = (DREF struct ast **)Dee_Malloc(2 * sizeof(DREF struct ast *));
	if unlikely(!elemv) {
		ast_decref(item);
		goto err;
	}
	ast_incref(initial_key);
	elemv[0] = initial_key;
	elemv[1] = item; /* Inherit */
	++elemc;
	/* Parse the remainder of a Dict initializer. */
	while (tok == ',') {
		if unlikely(yield() < 0)
			goto err_dict_elemv;
		/* Parse the key expression. */
		if (tok == '.') {
			if unlikely(yield() < 0)
				goto err_dict_elemv;
			if (TPP_ISKEYWORD(tok)) {
				DREF DeeObject *key = DeeString_NewSized(token.t_kwd->k_name,
				                                         token.t_kwd->k_size);
				if unlikely(!key)
					goto err_dict_elemv;
				result = ast_sethere(ast_constexpr(key));
				Dee_Decref(key);
				if unlikely(!result)
					goto err_dict_elemv;
				if unlikely(yield() < 0)
					goto err_dict_elemv_r;
			} else {
				if (WARN(W_EXPECTED_KEYWORD_AFTER_BRACE_DOT))
					goto err_dict_elemv;
				result = ast_constexpr(Dee_None);
				if unlikely(!result)
					goto err_dict_elemv;
			}
			if unlikely(likely(tok == '=') ? (yield() < 0) : WARN(W_EXPECTED_EQUAL_AFTER_BRACE_DOT))
				goto err_dict_elemv_r;
		} else {
			if (!maybe_expression_begin())
				break; /* Allow (and ignore) trailing comma. */
			result = ast_parse_expr(LOOKUP_SYM_NORMAL);
			if unlikely(!result)
				goto err_dict_elemv;
			if unlikely(likely(tok == ':') ? (yield() < 0) : WARN(W_EXPECTED_COLLON_AFTER_DICT_KEY))
				goto err_dict_elemv_r;
		}
		/* Now parse the associated item. */
		item = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!item)
			goto err_dict_elemv_r;
		/* Extend the element vector if needed. */
		if (elemc == elema) {
			DREF struct ast **new_elemv;
			size_t new_elema = elema * 2;
			ASSERT(new_elema);
do_realloc_dict:
			new_elemv = (DREF struct ast **)Dee_TryRealloc(elemv, (new_elema * 2) *
			                                                      sizeof(DREF struct ast *));
			if unlikely(!new_elemv) {
				if (new_elema != elemc + 1) {
					new_elema = elemc + 1;
					goto do_realloc_dict;
				}
				if (Dee_CollectMemory((new_elema * 2) * sizeof(DREF struct ast *)))
					goto do_realloc_dict;
				goto err_dict_keyitem;
			}
			elemv = new_elemv;
			elema = new_elema;
		}
		elemv[(elemc * 2) + 0] = result; /* Inherit */
		elemv[(elemc * 2) + 1] = item;   /* Inherit */
		++elemc;
	}
	if (elemc != elema) {
		DREF struct ast **new_elemv;
		new_elemv = (DREF struct ast **)Dee_TryRealloc(elemv, (elemc * 2) *
		                                                      sizeof(DREF struct ast *));
		if likely(new_elemv)
			elemv = new_elemv;
	}
	result = ast_multiple(AST_FMULTIPLE_GENERIC_KEYS, elemc * 2, elemv);
	if unlikely(!result)
		goto err_dict_elemv;
	/* NOTE: On success, all items have been inherited by the branch. */
done:
	return result;
err_dict_keyitem:
	ast_decref(item);
err_dict_elemv_r:
	ast_decref(result);
err_dict_elemv:
	while (elemc--) {
		ast_decref(elemv[(elemc / 2) + 0]);
		ast_decref(elemv[(elemc / 2) + 1]);
	}
	Dee_Free(elemv);
err:
	result = NULL;
	goto done;
}

INTERN DREF struct ast *FCALL
ast_parse_brace_list(struct ast *__restrict initial_item) {
	DREF struct ast *result;
	DREF struct ast **elemv;
	size_t elema = 1, elemc = 1;
	elemv = (DREF struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
	if unlikely(!elemv)
		goto err;
	ast_incref(initial_item);
	elemv[0] = initial_item;
	for (;;) {
		if (tok != ',') {
			if (tok == ':') {
				if (WARN(W_EXPECTED_COMMA_IN_LIST_INITIALIZER))
					goto err_list_elemv;
				if unlikely(yield() < 0)
					goto err_list_elemv;
				goto parse_list_item;
			}
			break;
		}
parse_list_item:
		if unlikely(yield() < 0)
			goto err_list_elemv;
		if (!maybe_expression_begin())
			break; /* Allow (and ignore) trailing comma. */
		result = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!result)
			goto err_list_elemv;
		if (elemc == elema) {
			DREF struct ast **new_elemv;
			size_t new_elema = elema * 2;
			ASSERT(new_elema);
do_realloc_list:
			new_elemv = (DREF struct ast **)Dee_TryRealloc(elemv, new_elema *
			                                                      sizeof(DREF struct ast *));
			if unlikely(!new_elemv) {
				if (new_elema != elemc + 1) {
					new_elema = elemc + 1;
					goto do_realloc_list;
				}
				if (Dee_CollectMemory(new_elema * sizeof(DREF struct ast *)))
					goto do_realloc_list;
				goto err_list_elemv_result;
			}
			elemv = new_elemv;
			elema = new_elema;
		}
		elemv[elemc++] = result; /* Inherit. */
	}
	if (elemc != elema) {
		DREF struct ast **new_elemv;
		new_elemv = (DREF struct ast **)Dee_TryRealloc(elemv, elemc *
		                                                      sizeof(DREF struct ast *));
		if likely(new_elemv)
			elemv = new_elemv;
	}
	result = ast_multiple(AST_FMULTIPLE_GENERIC, elemc, elemv);
	if unlikely(!result)
		goto err_list_elemv;
	/* Upon success, `ast_multiple' inherits the element vector. */
	return result;
err_list_elemv_result:
	ast_decref(result);
err_list_elemv:
	while (elemc--)
		ast_decref(elemv[elemc]);
	Dee_Free(elemv);
	goto err;
err:
	return NULL;
}


INTERN DREF struct ast *FCALL ast_parse_brace_items(void) {
	DREF struct ast *result, *new_result;
	/* Parse the initial item. */
	if (tok == '.') {
		if unlikely(yield() < 0)
			goto err;
		if (TPP_ISKEYWORD(tok)) {
			DREF DeeObject *key = DeeString_NewSized(token.t_kwd->k_name,
			                                         token.t_kwd->k_size);
			if unlikely(!key)
				goto err;
			result = ast_sethere(ast_constexpr(key));
			Dee_Decref(key);
			if unlikely(!result)
				goto err;
			if unlikely(yield() < 0)
				goto err_r;
		} else {
			if (WARN(W_EXPECTED_KEYWORD_AFTER_BRACE_DOT))
				goto err;
			result = ast_constexpr(Dee_None);
			if unlikely(!result)
				goto err;
		}
		if unlikely(likely(tok == '=') ? (yield() < 0) : WARN(W_EXPECTED_EQUAL_AFTER_BRACE_DOT))
			goto err_r;
		goto parse_dict;
	}
	/* Check for special case: Empty brace initializer. */
	if (!maybe_expression_begin())
		return ast_multiple(AST_FMULTIPLE_GENERIC, 0, NULL);
	result = ast_parse_expr(LOOKUP_SYM_NORMAL);
	if unlikely(!result)
		goto err;
	if (tok == ':') {
		if unlikely(yield() < 0)
			goto err_r;
parse_dict:
		new_result = ast_parse_mapping(result);
		ast_decref(result);
		result = new_result;
	} else {
		/* Parse an list initializer. */
		new_result = ast_parse_brace_list(result);
		ast_decref(result);
		result = new_result;
	}
	return result;
err_r:
	ast_decref(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_BRACE_C */
