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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_SYMBOL_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_SYMBOL_C 1

#include <deemon/api.h>

#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>

#include <stddef.h> /* NULL */

DECL_BEGIN

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_optimize_symbol)(struct ast_optimize_stack *__restrict stack,
                            struct ast *__restrict self, bool result_used) {
	struct symbol *sym;
	DREF DeeObject *symval;
	ASSERT(self->a_type == AST_SYM);
	(void)stack;
	sym = self->a_sym;
	/* If the symbol is being written to, then we can't optimize for external constants. */
#ifdef CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT
	if (symbol_get_haseffect(sym, self->a_scope))
		goto done;
#endif /* CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT */
	if (self->a_flag) {
#ifndef CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT
		if (symbol_set_haseffect(sym, self->a_scope))
			goto done;
#endif /* !CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT */
#ifdef OPTIMIZE_FASSUME
		/* Generic write with unknown value.
		 * -> Remove assumptions made on the symbol. */
		if (optimizer_flags & OPTIMIZE_FASSUME) {
			return ast_assumes_setsymval(stack->os_assume,
			                             self->a_sym,
			                             NULL);
		}
#endif /* OPTIMIZE_FASSUME */
		goto done;
	}
#ifndef CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT
	if (symbol_get_haseffect(sym, self->a_scope))
		goto done;
#endif /* !CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT */
	if (!result_used) {
		OPTIMIZE_VERBOSE("Remove unused read from symbol `%$s'\n",
		                 sym->s_name->k_size, sym->s_name->k_name);
		SYMBOL_DEC_NREAD(sym);
		self->a_type      = AST_CONSTEXPR;
		self->a_constexpr = DeeNone_NewRef();
		goto did_optimize;
	}
	SYMBOL_INPLACE_UNWIND_ALIAS(sym);
	ASSERT(sym->s_nread);
	/* Optimize constant, extern symbols. */
	if (sym->s_type == SYMBOL_TYPE_EXTERN &&
	    (sym->s_extern.e_symbol->ss_flags & (MODSYM_FPROPERTY | MODSYM_FCONSTEXPR)) == MODSYM_FCONSTEXPR) {
		/* The symbol is allowed to be expanded at compile-time. */
		int error;
		DeeModuleObject *symmod;
		symmod = SYMBOL_EXTERN_MODULE(sym);
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		error  = DeeModule_Initialize(symmod);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		error  = DeeModule_RunInit(symmod);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		if unlikely(error < 0)
			goto err;
		if (error == 0) {
			/* The module is not initialized. */
			ASSERT(Dee_module_symbol_getindex(sym->s_extern.e_symbol) < symmod->mo_globalc);
			DeeModule_LockRead(symmod);
			symval = symmod->mo_globalv[Dee_module_symbol_getindex(sym->s_extern.e_symbol)];
			Dee_XIncref(symval);
			DeeModule_LockEndRead(symmod);
			/* Make sure that the symbol value is allowed
			 * to be expanded in constant expression. */
			if likely(symval) {
				int allowed;
set_constant_expression:
				allowed = allow_constexpr(symval);
				if (allowed == CONSTEXPR_USECOPY) {
					if (DeeObject_InplaceDeepCopy(&symval)) {
						DeeError_Handled(ERROR_HANDLED_RESTORE);
						goto done_set_constexpr;
					}
				} else if (allowed != CONSTEXPR_ALLOWED) {
					goto done_set_constexpr;
				}
				/* Set the value as a constant expression. */
				SYMBOL_DEC_NREAD(self->a_sym); /* Trace read references. */
				self->a_constexpr = symval;    /* Inherit */
				self->a_type      = AST_CONSTEXPR;
				OPTIMIZE_VERBOSE("Inline constant symbol expression: `%r'\n", symval);
				goto did_optimize;
			}
done_set_constexpr:
			Dee_XDecref(symval);
		}
	} else if (sym->s_type == SYMBOL_TYPE_CONST) {
		/* Check for symbols that are actually constant expression. */
		symval = sym->s_const;
		Dee_Incref(symval);
		goto set_constant_expression;
	}

#ifdef OPTIMIZE_FASSUME
	/* Check on symbol assumptions to see if we can optimize
	 * this variable away, and into a constant expression. */
	if (optimizer_flags & OPTIMIZE_FASSUME) {
		symval = ast_assumes_getsymval(stack->os_assume, sym);
		if (symval)
			goto set_constant_expression;
	}
#endif /* OPTIMIZE_FASSUME */
done:
	return 0;
err:
	return -1;
did_optimize:
	++optimizer_count;
	goto done;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_SYMBOL_C */
