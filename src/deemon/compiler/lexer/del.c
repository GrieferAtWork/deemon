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
#ifndef GUARD_DEEMON_COMPILER_LEXER_DEL_C
#define GUARD_DEEMON_COMPILER_LEXER_DEL_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/object.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

PRIVATE WUNUSED DREF struct ast *DCALL
ast_parse_del_single(unsigned int lookup_mode) {
	DREF struct ast *result;
	DREF struct ast *new_result;
	result = ast_parse_unary(lookup_mode);
	if unlikely(!result)
		goto err;
	switch (result->a_type) {

	case AST_SYM:
		/* Create an unbind AST. */
		new_result = ast_setddi(ast_unbind(result->a_sym),
		                        &result->a_ddi);
		if unlikely(!new_result)
			goto err_r;
		ast_decref(result);
		result = new_result;
		if ((lookup_mode & LOOKUP_SYM_ALLOWDECL) &&
		    result->a_unbind->s_scope == current_scope) {
			/* Delete the actual symbol (don't just unbind it). */
			del_local_symbol(result->a_unbind);
		}
		break;

	case AST_OPERATOR: {
		Dee_operator_t new_operator;
		switch (result->a_flag) {

		case OPERATOR_GETATTR:
			new_operator = OPERATOR_DELATTR;
			break;

		case OPERATOR_GETITEM:
			new_operator = OPERATOR_DELITEM;
			break;

		case OPERATOR_GETRANGE:
			new_operator = OPERATOR_DELRANGE;
			break;

		default: goto default_case;
		}

		/* Create a new operator branch. */
		if unlikely(result->a_operator.o_exflag & AST_OPERATOR_FVARARGS)
			goto create_2;
		if unlikely(!result->a_operator.o_op1) {
			new_result = ast_operator1(new_operator,
			                           result->a_operator.o_exflag,
			                           result->a_operator.o_op0);
		} else if unlikely(result->a_operator.o_op3) {
			new_result = ast_operator4(new_operator,
			                           result->a_operator.o_exflag,
			                           result->a_operator.o_op0,
			                           result->a_operator.o_op1,
			                           result->a_operator.o_op2,
			                           result->a_operator.o_op3);
		} else if (result->a_operator.o_op2) {
			new_result = ast_operator3(new_operator,
			                           result->a_operator.o_exflag,
			                           result->a_operator.o_op0,
			                           result->a_operator.o_op1,
			                           result->a_operator.o_op2);
		} else {
create_2:
			new_result = ast_operator2(new_operator,
			                           result->a_operator.o_exflag,
			                           result->a_operator.o_op0,
			                           result->a_operator.o_op1);
		}
		if unlikely(!new_result)
			goto err_r;
		ast_setddi(new_result, &result->a_ddi);
		ast_decref(result);
		result = new_result;
	}	break;

	default:
default_case:
		if (WARN(W_UNEXPECTED_EXPRESSION_FOR_DEL))
			goto err_r;
		break;
	}
	return result;
err_r:
	ast_decref(result);
err:
	return NULL;
}


/* Parse everything following a `del' keyword in a statement, or expression:
 * >> foo = 7;
 * >> print foo;
 * >> del foo;        // Unbind + delete
 *        ^  ^
 * >> foo = 42;
 * >> print foo;
 * >> print del(foo); // Unbind only
 *              ^  ^
 * NOTE: When `LOOKUP_SYM_ALLOWDECL' is set in `lookup_mode',
 *       the function is allocated to delete locally defined
 *       variable symbols.
 *       However, in all cases is this function allowed to
 *       unbind variables (deleting only referring to the
 *       compile-time symbol becoming unknown and being added to
 *       the current scope's chain of deleted/anonymous symbols) */
INTERN WUNUSED DREF struct ast *DCALL
ast_parse_del(unsigned int lookup_mode) {
	DREF struct ast *result;
	size_t delc, dela;
	DREF struct ast **delv;

	/* Parse additional lookup modifiers. */
	if (ast_parse_lookup_mode(&lookup_mode))
		goto err;
	result = ast_parse_del_single(lookup_mode);
	if unlikely(!result)
		goto err;
	if (tok == ',') {
		/* Delete-multiple expression. */
		if unlikely(yield() < 0)
			goto err_r;
		/* Check for relaxed comma-rules. */
		{
			int temp;
			temp = maybe_expression_begin();
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err_r;
				goto done;
			}
		}
		delv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
		if unlikely(!delv)
			goto err_r;
		dela = 2, delc = 1;
		delv[0] = result; /* Inherit */
		for (;;) {
			result = ast_parse_del_single(lookup_mode);
			if unlikely(!result)
				goto err_delv;
			if (delc == dela) {
				DREF struct ast **new_delv;
				size_t new_dela = dela * 2;
do_realloc_delv:
				new_delv = (DREF struct ast **)Dee_TryReallocc(delv, new_dela,
				                                               sizeof(DREF struct ast *));
				if unlikely(!new_delv) {
					if (new_dela != delc + 1) {
						new_dela = delc + 1;
						goto do_realloc_delv;
					}
					if (Dee_CollectMemory(new_dela * sizeof(DREF struct ast *)))
						goto do_realloc_delv;
					goto err_delv_r;
				}
				delv = new_delv;
				dela = new_dela;
			}
			delv[delc++] = result; /* Inherit */
			if (tok != ',')
				break;
			if unlikely(yield() < 0)
				goto err_delv;
			{
				int temp;
				temp = maybe_expression_begin();
				if (temp <= 0) {
					if unlikely(temp < 0)
						goto err_delv;
					break;
				}
			}
		}

		/* Pack all delete expression together into a multiple-branch. */
		ASSERT(delc <= dela);
		if (delc != dela) {
			DREF struct ast **new_delv;
			new_delv = (DREF struct ast **)Dee_TryReallocc(delv, delc,
			                                               sizeof(DREF struct ast *));
			if likely(new_delv)
				delv = new_delv;
		}
		result = ast_multiple(AST_FMULTIPLE_KEEPLAST, delc, delv);
		if unlikely(!result)
			goto err_delv;

		/* Upon success, the multiple-branch `result' inherited all `delv' expressions. */
	}
done:
	return result;
err_delv_r:
	ast_decref(result);
err_delv:
	ast_decrefv(delv, delc);
	Dee_Free(delv);
	goto err;
err_r:
	ast_decref(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_DEL_C */
