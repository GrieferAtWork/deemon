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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENOPERATOR_FUNC_C
#define GUARD_DEEMON_COMPILER_ASM_GENOPERATOR_FUNC_C 1

#include <deemon/api.h>

#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include "../../runtime/builtin.h"
#include "../../runtime/strings.h"

#include <stdbool.h> /* false */
#include <stddef.h>  /* NULL */
#include <stdint.h>  /* int32_t, uint16_t */

DECL_BEGIN

#define DO(expr) if unlikely(expr) goto err

INTDEF DeeStringObject *tpconst rt_operator_names[1 + (AST_OPERATOR_MAX - AST_OPERATOR_MIN)];

/* @return:  1: Symbol doesn't exist.
 * @return:  0: Everything is OK.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
bind_module_symbol(DeeModuleObject *__restrict module,
                   uint16_t *__restrict p_modid,
                   uint16_t *__restrict p_symid,
                   char const *__restrict symbol_name) {
	struct Dee_module_symbol *symbol;
	int32_t temp;
	symbol = DeeModule_GetSymbolString(module, symbol_name);
	if unlikely(!symbol)
		return 1; /* Doesn't exists */
	if (symbol->ss_flags & Dee_MODSYM_FEXTERN) {
		/* Follow external module symbols. */
		ASSERT(symbol->ss_impid < module->mo_importc);
		module = module->mo_importv[symbol->ss_impid];
	}
	/* XXX: What if the calling module is the `operators' module? */
	temp = asm_newmodule(module);
	if unlikely(temp < 0)
		goto err;
	*p_modid = (uint16_t)temp;
	*p_symid = Dee_module_symbol_getindex(symbol);
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((2)) int DCALL
ast_gen_operator_func(struct ast *binding,
                      struct ast *ddi_ast,
                      Dee_operator_t operator_name) {
	struct Dee_opinfo const *info;
	int temp;
	char const *symbol_name = NULL;
	DREF DeeModuleObject *operators_module;
	uint16_t COMPILER_IGNORE_UNINITIALIZED(opmod_id);
	uint16_t COMPILER_IGNORE_UNINITIALIZED(opsym_id);
	int32_t deemon_module_id;
	if (operator_name >= AST_OPERATOR_MIN &&
	    operator_name <= AST_OPERATOR_MAX) {
		/* Special, ambiguous operator. */
		symbol_name = DeeString_STR(rt_operator_names[operator_name - AST_OPERATOR_MIN]);
	} else {
		/* Default case: determine the operator symbol using generic-operator info. */
		info = DeeTypeType_GetOperatorById(&DeeType_Type, operator_name);
		if (info) {
			symbol_name = info->oi_sname;
		} else {
			/* TODO: Lookup the operator by its name at runtime! */
		}
	}
	/* Import the operators module. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	operators_module = DeeModule_OpenEx("operators", COMPILER_STRLEN("operators"),
	                                    NULL, 0, DeeModule_IMPORT_F_ENOENT,
	                                    inner_compiler_options);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	operators_module = DeeModule_OpenGlobal(Dee_AsObject(&str_operators),
	                                        inner_compiler_options,
	                                        false);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if unlikely(!ITER_ISOK(operators_module)) {
		if (operators_module) {
			DO(WARNAST(ddi_ast, W_MODULE_NOT_FOUND, &str_operators));
			return asm_gpush_none();
		}
		goto err;
	}
	if (symbol_name) {
		/* Lookup a symbol matching the operator's name. */
		temp = bind_module_symbol(operators_module, &opmod_id, &opsym_id, symbol_name);
		if unlikely(temp != 0) {
			if unlikely(temp < 0)
				goto err;
			if (WARNAST(ddi_ast, W_NO_OPERATOR_SYMBOL, symbol_name))
				goto err_module;
			goto generic_operator;
		}
		if (asm_gpush_extern(opmod_id, opsym_id))
			goto err_module; /* <...> from operators */
		if (binding) {
			if (ast_genasm_one(binding, ASM_G_FPUSHRES))
				goto err_module;
			deemon_module_id = asm_newmodule(DeeModule_GetDeemon());
			if unlikely(deemon_module_id < 0)
				goto err_module;
			if (asm_gcall_extern((uint16_t)deemon_module_id, id_InstanceMethod, 2))
				goto err_module; /* InstanceMethod(<...> from operators, binding) */
		}
	} else {
generic_operator:
		/* Fallback: invoke the a function `operator from operators',
		 *           which takes the raw ID of the operator which we're
		 *           trying to generate a function for, such that the
		 *           user can override it to implement their custom
		 *           operator ID.
		 *        -> If that function doesn't exist either, that's a
		 *           compiler error. */
		temp = bind_module_symbol(operators_module,
		                          &opmod_id,
		                          &opsym_id,
		                          STR_operator);
		if unlikely(temp != 0) {
			if unlikely(temp < 0)
				goto err;
			if (WARNAST(ddi_ast, W_NO_OPERATOR_FALLBACK_FUNCTION))
				goto err_module;
		}
		deemon_module_id = asm_newmodule(DeeModule_GetDeemon());
		if unlikely(deemon_module_id < 0)
			goto err_module;
		if (asm_gpush_extern(opmod_id, opsym_id))
			goto err_module; /* operator from operators */
		if (asm_gpush_u16(operator_name))
			goto err_module; /* operator from operators, opname */
		if (asm_gcall_extern((uint16_t)deemon_module_id, id_InstanceMethod, 2))
			goto err_module; /* InstanceMethod(operator from operators, opname) */
		if (binding) {
			if (ast_genasm_one(binding, ASM_G_FPUSHRES))
				goto err_module;
			if (asm_gcall_extern((uint16_t)deemon_module_id, id_InstanceMethod, 2))
				goto err_module; /* InstanceMethod(InstanceMethod(operator from operators, opname), binding) */
		}
	}
	Dee_Decref(operators_module);
	return 0;
err_module:
	Dee_Decref(operators_module);
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENOPERATOR_FUNC_C */
