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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENFUNCTION_C
#define GUARD_DEEMON_COMPILER_ASM_GENFUNCTION_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/module.h>
#include <deemon/object.h>
/**/

#include <stdint.h> /* uint16_t */

DECL_BEGIN

#define DO(expr) if unlikely(expr) goto err

PRIVATE WUNUSED DREF DeeCodeObject *DCALL
ast_assemble_function(struct ast *__restrict function_ast,
                      uint16_t *__restrict p_refc,
                      struct asm_symbol_ref **__restrict p_refv) {
	DREF DeeCodeObject *result;
	DeeScopeObject *prev_scope;
	ASSERT(function_ast->a_type == AST_FUNCTION);
	/* This is where it gets interesting, because this will
	 * create a temporary sub-assembler, allowing for recursion! */
	ASSERT_OBJECT_TYPE((DeeObject *)current_scope, &DeeScope_Type);
	ASSERT_OBJECT_TYPE((DeeObject *)current_basescope, &DeeBaseScope_Type);
	ASSERT_OBJECT_TYPE((DeeObject *)function_ast->a_function.f_scope, &DeeBaseScope_Type);
	ASSERT(function_ast->a_function.f_scope->bs_root == current_rootscope);
	ASSERT(current_basescope == current_scope->s_base);

	prev_scope = current_scope;
	/* Temporarily override the active scope + base-scope. */
	current_scope     = (DREF DeeScopeObject *)function_ast->a_function.f_scope;
	current_basescope = function_ast->a_function.f_scope;

	/* HINT: `code_compile' will safe and restore our own assembler context. */
	result = code_compile(function_ast->a_function.f_code,
	                      /* Don't propagate `ASM_FBIGCODE' */
	                      (current_assembler.a_flag & ~ASM_FBIGCODE) |
	                      (DeeCompiler_Current->cp_options
	                       ? (DeeCompiler_Current->cp_options->co_assembler & ASM_FBIGCODE)
	                       : 0),
	                      false, p_refc, p_refv);
	/* Now that the code has been generated, it's time to
	 * register it as a constant variable of our own code. */
	current_basescope = prev_scope->s_base;
	current_scope     = prev_scope;
	if unlikely(!result)
		goto err;
	/* Save the restore-code-object for the event of the linker having to be reset. */
	function_ast->a_function.f_scope->bs_restore = result;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
asm_gpush_function(struct ast *__restrict function_ast) {
	DREF DeeCodeObject *code;
	int32_t cid;
	uint16_t i, refc;
	struct asm_symbol_ref *refv;
	code = ast_assemble_function(function_ast, &refc, &refv);
	if unlikely(!code)
		goto err;
	DO(asm_putddi(function_ast));
	ASSERT(code->co_refstaticc >= code->co_refc);
	if (!refc && code->co_refstaticc <= code->co_refc) {
		/* Special case: The function doesn't reference any code.
		 * -> In this case, we can construct the function object
		 *    itself as a constant. */
		DREF DeeFunctionObject *function;
		Dee_Free(refv);
		function = (DREF DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)code);
		Dee_Decref(code);
		if unlikely(!function)
			goto err;
		cid = asm_newconst((DeeObject *)function);
		Dee_Decref(function);
		if unlikely(cid < 0)
			goto err;
		return asm_gpush_const((uint16_t)cid);
	}
	cid = asm_newconst((DeeObject *)code);
	Dee_Decref(code);
	if unlikely(cid < 0)
		goto err_refv;
	/* Push referenced symbols. */
	for (i = 0; i < refc; ++i) {
		refv[i].sr_sym->s_flag &= ~(SYMBOL_FALLOC | SYMBOL_FALLOCREF);
		refv[i].sr_sym->s_flag |= refv[i].sr_orig_flag & (SYMBOL_FALLOC | SYMBOL_FALLOCREF);
		refv[i].sr_sym->s_refid = refv[i].sr_orig_refid;
		DO(asm_gpush_symbol(refv[i].sr_sym, function_ast));
	}
	Dee_Free(refv);
	/* Generate assembly to create (and push) the new function object. */
	return asm_gfunction_ii((uint16_t)cid, (uint16_t)refc);
err_refv:
	Dee_Free(refv);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
asm_gmov_function(struct symbol *__restrict dst,
                  struct ast *function_ast,
                  struct ast *dst_warn_ast) {
	DREF DeeCodeObject *code;
	int32_t cid;
	uint16_t i, refc;
	struct asm_symbol_ref *refv;
	code = ast_assemble_function(function_ast, &refc, &refv);
	if unlikely(!code)
		goto err;
	DO(asm_putddi(function_ast));
	if (!refc) {
		/* Special case: The function doesn't reference any code.
		 * -> In this case, we can construct the function object
		 *    itself as a constant. */
		DREF DeeFunctionObject *function;
		Dee_Free(refv);
		function = (DREF DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)code);
		Dee_Decref(code);
		if unlikely(!function)
			goto err;
		cid = asm_newconst((DeeObject *)function);
		Dee_Decref(function);
		if unlikely(cid < 0)
			goto err;
		DO(asm_gprefix_symbol(dst, dst_warn_ast));
		return asm_gpush_const_p((uint16_t)cid);
	}
	cid = asm_newconst((DeeObject *)code);
	Dee_Decref(code);
	if unlikely(cid < 0)
		goto err_refv;
	/* Push referenced symbols. */
	for (i = 0; i < refc; ++i) {
		refv[i].sr_sym->s_flag &= ~(SYMBOL_FALLOC | SYMBOL_FALLOCREF);
		refv[i].sr_sym->s_flag |= refv[i].sr_orig_flag & (SYMBOL_FALLOC | SYMBOL_FALLOCREF);
		refv[i].sr_sym->s_refid = refv[i].sr_orig_refid;
		DO(asm_gpush_symbol(refv[i].sr_sym, function_ast));
	}
	Dee_Free(refv);
	/* Generate assembly to create (and push) the new function object. */
	DO(asm_gprefix_symbol(dst, dst_warn_ast));
	ASSERT(refc != 0);
	return asm_gfunction_ii_prefixed((uint16_t)cid,
	                                 (uint16_t)refc);
err_refv:
	Dee_Free(refv);
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENFUNCTION_C */
