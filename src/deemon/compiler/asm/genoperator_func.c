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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENOPERATOR_FUNC_C
#define GUARD_DEEMON_COMPILER_ASM_GENOPERATOR_FUNC_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/error.h>
#include <deemon/module.h>

#include "../../runtime/builtin.h"
#include "../../runtime/strings.h"

DECL_BEGIN

INTDEF DeeObject *rt_operator_names[1 + (AST_OPERATOR_MAX - AST_OPERATOR_MIN)];

/* @return:  1: Symbol doesn't exist.
 * @return:  0: Everything is OK.
 * @return: -1: An error occurred. */
PRIVATE int DCALL
bind_module_symbol(DeeModuleObject *__restrict module,
                   uint16_t *__restrict pmodid,
                   uint16_t *__restrict psymid,
                   char const *__restrict symbol_name) {
	struct module_symbol *symbol;
	int32_t temp;
	symbol = DeeModule_GetSymbolString(module,
	                                   symbol_name,
	                                   Dee_HashStr(symbol_name));
	if unlikely(!symbol)
		return 1; /* Doesn't exists */
	if (symbol->ss_flags & MODSYM_FEXTERN) {
		/* Follow external module symbols. */
		ASSERT(symbol->ss_extern.ss_impid < module->mo_importc);
		module = module->mo_importv[symbol->ss_extern.ss_impid];
	}
	/* XXX: What if the calling module is the `operators' module? */
	temp = asm_newmodule(module);
	if unlikely(temp < 0)
		return -1;
	*pmodid = (uint16_t)temp;
	*psymid = symbol->ss_index;
	return 0;
}


INTERN int DCALL
ast_gen_operator_func(struct ast *binding,
                      struct ast *__restrict ddi_ast,
                      uint16_t operator_name) {
	struct opinfo *info;
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
		info = Dee_OperatorInfo(NULL, operator_name);
		if (info)
			symbol_name = info->oi_sname;
	}
	/* Import the operators module. */
	operators_module = (DREF DeeModuleObject *)DeeModule_OpenGlobal(&str_operators,
	                                                                inner_compiler_options,
	                                                                false);
	if unlikely(!ITER_ISOK(operators_module))
		{
		if (operators_module) {
			if (WARNAST(ddi_ast, W_MODULE_NOT_FOUND, &str_operators))
				goto err;
			return asm_gpush_none();
		}
		goto err;
	}
	if (symbol_name) {
		/* Lookup a symbol matching the operator's name. */
		temp = bind_module_symbol(operators_module, &opmod_id, &opsym_id, symbol_name);
		if unlikely(temp != 0)
			{
			if (temp < 0)
				goto err;
			if (WARNAST(ddi_ast, W_NO_OPERATOR_SYMBOL, symbol_name))
				goto err_module;
			goto generic_operator;
		}
		if (asm_gpush_extern(opmod_id, opsym_id))
			goto err_module; /* <...> from operators */
		if (binding) {
			if (ast_genasm_one(binding, ASM_G_FPUSHRES))
				goto err;
			deemon_module_id = asm_newmodule(DeeModule_GetDeemon());
			if unlikely(deemon_module_id < 0)
				goto err_module;
			if (asm_gcall_extern((uint16_t)deemon_module_id, id_InstanceMethod, 2))
				goto err_module; /* InstanceMethod(<...> from operators,binding) */
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
		                          DeeString_STR(&str_operator));
		if unlikely(temp != 0)
			{
			if (temp < 0)
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
			goto err_module; /* InstanceMethod(operator from operators,opname) */
		if (binding) {
			if (ast_genasm_one(binding, ASM_G_FPUSHRES))
				goto err_module;
			if (asm_gcall_extern((uint16_t)deemon_module_id, id_InstanceMethod, 2))
				goto err_module; /* InstanceMethod(InstanceMethod(operator from operators,opname),binding) */
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
