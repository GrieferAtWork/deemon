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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CLASS_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CLASS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* Dee_*alloc*, Dee_Free, Dee_Freea */
#include <deemon/class.h>             /* DeeClassDescriptorObject, DeeClassDescriptor_CLSOPNEXT, DeeClassDescriptor_Check, Dee_CLASS_OPERATOR_PRINT, Dee_CLASS_OPERATOR_PRINTREPR, Dee_class_operator */
#include <deemon/code.h>              /* Dee_CODE_FTHISCALL, Dee_CODE_FYIELDING */
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/object.h>
#include <deemon/string.h>            /* DeeString_STR */
#include <deemon/system-features.h>   /* memcpy*, memset */

#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

/* Returns the address of a given operator `name' */
PRIVATE WUNUSED NONNULL((1)) struct Dee_class_operator *DCALL
DeeClassDescriptorObject_GetOperatorAddr(DeeClassDescriptorObject *__restrict self,
                                         Dee_operator_t name) {
	Dee_operator_t i, perturb;
	i = perturb = name & self->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct Dee_class_operator *entry;
		entry = &self->cd_clsop_list[i & self->cd_clsop_mask];
		if (entry->co_name != name) {
			if (entry->co_name == (Dee_operator_t)-1)
				break; /* Not implemented! */
			continue;
		}

		/* Found the entry! */
		ASSERT(entry->co_addr < self->cd_cmemb_size);
		return entry;
	}
	return NULL;
}

INTDEF struct Dee_class_operator empty_class_operators[];

/* Rename the name of `slot' to `new_name' */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
class_descriptor_rename_operator(DeeClassDescriptorObject *__restrict self,
                                 struct Dee_class_operator *slot,
                                 Dee_operator_t new_name) {
	Dee_operator_t mask, i, j, perturb;
	struct Dee_class_operator *new_table;
	ASSERT(slot >= self->cd_clsop_list &&
	       slot <= self->cd_clsop_list + self->cd_clsop_mask);
	ASSERT(slot->co_name != new_name);
	mask      = self->cd_clsop_mask;
	new_table = (struct Dee_class_operator *)Dee_Mallocac(mask + 1, sizeof(struct Dee_class_operator));
	if unlikely(!new_table)
		goto err;

	/* Rename the operator (so it'll get rehashed properly below) */
	slot->co_name = new_name;

	/* Fill the new table with all unused entries. */
	memset(new_table, 0xff, (mask + 1) * sizeof(struct Dee_class_operator));

	/* Rehash all pre-existing bindings. */
	for (i = 0; i <= self->cd_clsop_mask; ++i) {
		struct Dee_class_operator *op, *new_op;
		op = &self->cd_clsop_list[i];
		if (op->co_name == (Dee_operator_t)-1)
			continue; /* Unused entry. */

		/* Insert the entry into the new table. */
		j = perturb = op->co_name & mask;
		for (;; DeeClassDescriptor_CLSOPNEXT(j, perturb)) {
			new_op = &new_table[j & mask];
			if (new_op->co_name == (Dee_operator_t)-1)
				break;
		}
		memcpy(new_op, op, sizeof(struct Dee_class_operator));
	}

	/* Install the new table and mask. */
	ASSERT(self->cd_clsop_list != empty_class_operators);
	memcpyc(self->cd_clsop_list, new_table,
	        mask + 1, sizeof(struct Dee_class_operator));
	Dee_Freea(new_table);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
ast_convert_return_to_fprint_action(struct ast *__restrict self,
                                    struct symbol *file_symbol,
                                    struct ast *print_content) {
	DREF struct ast **elemv;
	DREF struct ast *content_ast;
	DREF struct ast *file_symbol_ast;
	elemv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
	if unlikely(!elemv)
		goto err;
	elemv[0] = print_content;
	ast_incref(print_content);
	content_ast = ast_multiple(AST_FMULTIPLE_TUPLE, 1, elemv);
	content_ast = ast_putddi(content_ast, &self->a_ddi);
	if unlikely(!content_ast)
		goto err_elemv;
	file_symbol_ast = ast_sym(file_symbol);
	file_symbol_ast = ast_putddi(file_symbol_ast, &self->a_ddi);
	if unlikely(!file_symbol_ast)
		goto err_content_ast;

	/* Re-assign a return-branch into an fprint action. */
	ASSERT(self->a_type == AST_RETURN);
	ast_decref_unlikely(self->a_return);
	self->a_type          = AST_ACTION;
	self->a_flag          = AST_FACTION_FPRINT;
	self->a_action.a_act0 = file_symbol_ast; /* Inherit reference */
	self->a_action.a_act1 = content_ast;     /* Inherit reference */
	self->a_action.a_act2 = NULL;

	return 0;
err_content_ast:
	ast_decref_likely(content_ast);
	goto err;
err_elemv:
	ast_decref_nokill(print_content);
	Dee_Free(elemv);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int
(DCALL ast_try_optimize_class_operator_str2print)(struct ast *__restrict self,
                                                  struct Dee_class_operator *operator_str,
                                                  Dee_operator_t new_operator) {
	size_t i;
	struct class_member *str_member;
	struct ast *str_func;
	struct ast *str_code;
	struct ast *str_return_expr;
	DeeBaseScopeObject *str_scope;
	DeeClassDescriptorObject *desc;
	struct symbol *fp_arg;
	ASSERT(self->a_type == AST_CLASS);
	ASSERT(self->a_class.c_desc->a_type == AST_CONSTEXPR);
	desc = (DeeClassDescriptorObject *)self->a_class.c_desc->a_constexpr;
	ASSERT(DeeClassDescriptor_Check(desc));
	ASSERT(operator_str >= desc->cd_clsop_list &&
	       operator_str <= desc->cd_clsop_list + desc->cd_clsop_mask);
	ASSERT(operator_str->co_name == OPERATOR_STR ||
	       operator_str->co_name == OPERATOR_REPR);
	ASSERT(new_operator == Dee_CLASS_OPERATOR_PRINT ||
	       new_operator == Dee_CLASS_OPERATOR_PRINTREPR);

	/* Find the class member used to initialize `str_addr' */
	for (i = 0;; ++i) {
		if (i >= self->a_class.c_memberc)
			goto done; /* Special case: operator is never assigned */
		str_member = &self->a_class.c_memberv[i];
		if (str_member->cm_index == operator_str->co_addr)
			break;
	}

	/* Check what kind of expression will be assigned to the str-operator.
	 * We can only optimize code like:
	 * >> operator str(): string {
	 * >>     ... // Any code that doesn't contain a `return' statement
	 * >>     return EXPRESSION;
	 * >> }
	 */
	str_func = str_member->cm_ast;
	if (str_func->a_type != AST_FUNCTION)
		goto done;
	str_scope = str_func->a_function.f_scope;
	if (!(str_scope->bs_cflags & BASESCOPE_FRETURN))
		goto done;
	if (str_scope->bs_flags & Dee_CODE_FYIELDING)
		goto done;
	if (!(str_scope->bs_flags & Dee_CODE_FTHISCALL))
		goto done;
	if (str_scope->bs_argc != 0)
		goto done;
	ASSERTF(str_scope->bs_argc_min == 0, "Then why is `str_scope->bs_argc == 0'?");
	ASSERTF(str_scope->bs_argc_max == 0, "Then why is `str_scope->bs_argc == 0'?");
	ASSERTF(!str_scope->bs_varargs, "Then why is `str_scope->bs_argc == 0'?");
	ASSERTF(!str_scope->bs_varkwds, "Then why is `str_scope->bs_argc == 0'?");
	ASSERTF(str_scope->bs_this, "Then why is `str_scope->bs_flags & Dee_CODE_FTHISCALL'?");

	/* TODO: Don't require there to be a single return statement.
	 * Just replace all return statements with prints:
	 * >> operator str() {
	 * >>     if (a)
	 * >>         return "A";
	 * >>     if (b)
	 * >>         return "B";
	 * >>     return "C";
	 * >> }
	 * into:
	 * >> operator str(fp) {
	 * >>     if (a)
	 * >>         { print fp: "A",; return; }
	 * >>     if (b)
	 * >>         { print fp: "B",; return; }
	 * >>     print fp: "C",;
	 * >> }
	 */
	str_code = str_func->a_function.f_code;
	while (str_code->a_type == AST_MULTIPLE) {
		if (str_code->a_multiple.m_astc == 0)
			goto done;
		for (i = 0; i < str_code->a_multiple.m_astc - 1; ++i) {
			if (ast_contains_return(str_code->a_multiple.m_astv[i]))
				goto done;
		}
		str_code = str_code->a_multiple.m_astv[str_code->a_multiple.m_astc - 1];
	}

	/* The last statement must be a return-statement. */
	if (str_code->a_type != AST_RETURN)
		goto done;
	str_return_expr = str_code->a_return;
	if (str_return_expr->a_type == AST_CONSTEXPR)
		goto done; /* Don't try to optimize away constant expression strings */
	if (ast_contains_return(str_return_expr))
		goto done;

	/* Get a symbol which is going to become the `fp' argument. */
	fp_arg = new_unnamed_symbol_in_scope(&str_scope->bs_scope);
	if unlikely(!fp_arg)
		goto err;

	/* Optimize:
	 * >> return <EXPRESSION>
	 * Into:
	 * >> print <fp_arg>: (<EXPRESSION>,)...,;
	 */
	if (ast_convert_return_to_fprint_action(str_code, fp_arg, str_return_expr))
		goto err;
	ASSERT(str_code->a_type == AST_ACTION);
	ASSERT(str_code->a_flag == AST_FACTION_FPRINT);

	/* Inject the `fp_arg' symbol as an argument. */
	{
		struct symbol **argv;
		argv = (struct symbol **)Dee_Reallocc(str_scope->bs_argv, 1, sizeof(struct symbol *));
		if unlikely(!argv)
			goto err;
		str_scope->bs_argv     = argv;
		str_scope->bs_argc_min = 1;
		str_scope->bs_argc_max = 1;
		str_scope->bs_argc     = 1;
		str_scope->bs_cflags &= ~BASESCOPE_FRETURN; /* There are no more return statements in this scope */
		argv[0] = fp_arg;
		fp_arg->s_type  = SYMBOL_TYPE_ARG;
		fp_arg->s_flag  = SYMBOL_FALLOC;
		fp_arg->s_symid = 0;
	}

	/* Rename the operator. */
	if (class_descriptor_rename_operator(desc, operator_str, new_operator))
		goto err;
	
#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
	{
		char const *cname = desc->cd_name ? DeeString_STR(desc->cd_name) : "<anonymous>";
		char const *oname = new_operator == Dee_CLASS_OPERATOR_PRINT ? "str" : "repr";
		OPTIMIZE_VERBOSEAT(str_func,
		                   "Optimize `%s.operator %s() { [...] return EXPR; }' "
		                   /* */ "-> `%s.operator %s(<fp>) { [...] print <fp>: (EXPR,)...,; }'\n",
		                   cname, oname, cname, oname);
	}
#endif /* CONFIG_HAVE_OPTIMIZE_VERBOSE */
	++optimizer_count;
done:
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_optimize_class)(struct ast_optimize_stack *__restrict stack,
                           struct ast *__restrict self, bool result_used) {
	size_t i;
	ASSERT(self->a_type == AST_CLASS);
	(void)result_used;

	/* Do generic optimizations */
	if (self->a_class.c_base &&
	    ast_optimize(stack, self->a_class.c_base, true))
		goto err;
	if (ast_optimize(stack, self->a_class.c_desc, true))
		goto err;
	for (i = 0; i < self->a_class.c_memberc; ++i) {
		struct class_member *member = &self->a_class.c_memberv[i];
		if (ast_optimize(stack, member->cm_ast, member->cm_index != (uint16_t)-1))
			goto err;
	}

	/* Try to optimize str/repr operators into print/printrepr:
	 * >> class Point {
	 * >>     this = default;
	 * >>     public final member x: int;
	 * >>     public final member y: int;
	 * >>     operator str(): string {
	 * >>         return f"<Point {x}, {y}>";
	 * >>     }
	 * >> }
	 *
	 * Optimize this into:
	 * >> class Point {
	 * >>     this = default;
	 * >>     public final member x: int;
	 * >>     public final member y: int;
	 * >>     operator str(fp: File) {
	 * >>         print fp: ("<Point ", x, ", ", y, ">"),;
	 * >>     }
	 * >> } */
	if (self->a_class.c_desc->a_type == AST_CONSTEXPR &&
	    DeeClassDescriptor_Check(self->a_class.c_desc->a_constexpr)) {
		struct Dee_class_operator *operator_str;
		DeeClassDescriptorObject *desc;
		desc = (DeeClassDescriptorObject *)self->a_class.c_desc->a_constexpr;
		if ((operator_str = DeeClassDescriptorObject_GetOperatorAddr(desc, OPERATOR_STR)) != NULL &&
		    (DeeClassDescriptorObject_GetOperatorAddr(desc, Dee_CLASS_OPERATOR_PRINT) == NULL)) {
			if (ast_try_optimize_class_operator_str2print(self, operator_str, Dee_CLASS_OPERATOR_PRINT))
				goto err;
		}
		if ((operator_str = DeeClassDescriptorObject_GetOperatorAddr(desc, OPERATOR_REPR)) != NULL &&
		    (DeeClassDescriptorObject_GetOperatorAddr(desc, Dee_CLASS_OPERATOR_PRINTREPR) == NULL)) {
			if (ast_try_optimize_class_operator_str2print(self, operator_str, Dee_CLASS_OPERATOR_PRINTREPR))
				goto err;
		}
	}

	return 0;
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CLASS_C */
