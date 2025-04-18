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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENSWITCH_C
#define GUARD_DEEMON_COMPILER_ASM_GENSWITCH_C 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/tuple.h>

#include "../../runtime/strings.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#define DO(expr) if unlikely(expr) goto err

/* Assuming that the switch-expression is located ontop of the stack,
 * generate code to jump to `target' after popping said expression
 * when if equals `case_expr'. However if it doesn't, leave the stack
 * as it was upon entry. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
emit_runtime_check(struct ast *__restrict ddi_ast,
                   struct ast *__restrict case_expr,
                   struct asm_sym *__restrict target) {
	struct asm_sym *temp, *guard_begin, *guard_end;
	struct asm_exc *exc_hand;
	temp = asm_newsym();
	if unlikely(!temp)
		goto err;
	guard_begin = asm_newsym();
	if unlikely(!guard_begin)
		goto err;
	guard_end = asm_newsym();
	if unlikely(!guard_end)
		goto err;
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup()); /* expr, expr */
	DO(ast_genasm_one(case_expr, ASM_G_FPUSHRES)); /* expr, expr, case */

	/* This instruction must be protected by an exception
	 * handler for NOT_IMPLEMENTED and TYPE errors that
	 * are handled by jumping to the next check. */
	DO(asm_putddi(ddi_ast));
	asm_defsym(guard_begin);
	DO(asm_gcmp_eq()); /* expr, expr==case */
	asm_defsym(guard_end);
	DO(asm_gjmp(ASM_JF, temp)); /* if (!(expr == case)) goto temp; */
	asm_decsp();  /* Adjust for ASM_JF */
	DO(asm_gpop()); /* Pop `expr' before jumping to `target' */
	DO(asm_gjmp(ASM_JMP, target));
	asm_incsp();      /* Revert the popping of `expr' after the jump. */
	asm_defsym(temp); /* Jump here when the expressions didn't match. */

	/* Create a primary exception handler for NOT_IMPLEMENTED errors. */
	exc_hand = asm_newexc();
	if unlikely(!exc_hand)
		goto err;
	Dee_Incref(&DeeError_NotImplemented);
	exc_hand->ex_mask  = &DeeError_NotImplemented;
	exc_hand->ex_start = guard_begin;
	exc_hand->ex_end   = guard_end;
	exc_hand->ex_addr  = temp;
#if 0
	exc_hand->ex_stack = current_assembler.a_stackcur;
#endif
	exc_hand->ex_flags = EXCEPTION_HANDLER_FHANDLED;
	++guard_begin->as_used;
	++guard_end->as_used;
	++temp->as_used;

	/* Create a secondary exception handler for TYPE-errors. */
	exc_hand = asm_newexc();
	if unlikely(!exc_hand)
		goto err;
	Dee_Incref(&DeeError_TypeError);
	exc_hand->ex_mask  = &DeeError_TypeError;
	exc_hand->ex_start = guard_begin;
	exc_hand->ex_end   = guard_end;
	exc_hand->ex_addr  = temp;
#if 0
	exc_hand->ex_stack = current_assembler.a_stackcur;
#endif
	exc_hand->ex_flags = EXCEPTION_HANDLER_FHANDLED;
	++guard_begin->as_used;
	++guard_end->as_used;
	++temp->as_used;

	return 0;
err:
	return -1;
}


/* Construct a tuple `(sym.PC, sym.SP)' for the given `sym' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pack_target_tuple(struct asm_sym *__restrict sym) {
	DREF DeeObject *ri_ip;
	DREF DeeObject *ri_sp;
	DREF DeeObject *result;
	ri_ip = DeeRelInt_New(sym, 0, RELINT_MODE_FADDR);
	if unlikely(!ri_ip)
		goto err;
	ri_sp = DeeRelInt_New(sym, 0, RELINT_MODE_FSTCK);
	if unlikely(!ri_sp)
		goto err_ip;
	result = DeeTuple_PackSymbolic(2, ri_ip, ri_sp); /* Inherit references. */
	if unlikely(!result)
		goto err_ip_sp;
	return result;
err_ip_sp:
	Dee_DecrefDokill(ri_sp);
err_ip:
	Dee_DecrefDokill(ri_ip);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
try_get_integer_as_u16(DeeObject *__restrict self,
                       uint16_t *__restrict result) {
	if (DeeInt_Check(self)) {
		uint32_t val;
		if (!DeeInt_TryAsUInt32(self, &val))
			goto nope;
		if (val > UINT16_MAX)
			goto nope;
		*result = (uint16_t)val;
		return true;
	}
	if (DeeObject_InstanceOfExact(self, &DeeRelInt_Type)) {
		tint_t val;
		DeeRelIntObject *me = (DeeRelIntObject *)self;
		if (!ASM_SYM_DEFINED(me->ri_sym))
			goto nope;
		if (me->ri_mode == RELINT_MODE_FADDR)
			goto nope; /* Address values may change during relocation. */
		val = me->ri_add;
		val += me->ri_sym->as_stck;
		if unlikely(val < 0 || val > UINT16_MAX)
			goto nope;
		*result = (uint16_t)val;
		return true;
	}
nope:
	return false;
}



INTERN WUNUSED NONNULL((1)) int DCALL
ast_genasm_switch(struct ast *__restrict self) {
	struct asm_sym *old_break, *switch_break;
	struct text_label *constant_cases, **p_cases, *cases;
	struct asm_sym *default_sym;
	size_t num_constants;
	uint16_t old_finflag;
	int temp;
	int32_t default_cid = -1, jumptable_cid = -1;
	uint16_t stack_size_after_jump = 0;
	code_addr_t case_unpack_addr   = 0;
	bool has_expression            = false;
	ASSERT_AST(self);
	ASSERT(self->a_type == AST_SWITCH);

	/* Allocate a new symbol for `break' being used
	 * inside of a switch and set up contextual flags. */
	switch_break = asm_newsym();
	if unlikely(!switch_break)
		goto err;
	old_break = current_assembler.a_loopctl[ASM_LOOPCTL_BRK];
	current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = switch_break;
	old_finflag = current_assembler.a_finflag;
	current_assembler.a_finflag |= ASM_FINFLAG_NOLOOP;

	if (self->a_switch.s_default) {
		/* The default label shouldn't be allocated yet, but
		 * the specs don't state that a goto-AST can't jump
		 * to a case label, meaning that we must allow a
		 * pre-allocated symbol. */
		default_sym = self->a_switch.s_default->tl_asym;
		if likely(!default_sym) {
			default_sym = asm_newsym();
			/* Save the default symbol to that the switch-block may initialize it. */
			self->a_switch.s_default->tl_asym = default_sym;
		}
	} else {
		default_sym = asm_newsym();
	}
	if unlikely(!default_sym)
		goto err;

	/* Go through all the cases and collect those than can be considered
	 * suitable candidates for constant expressions, while generating
	 * text to compare cases that cannot be determined at compile-time
	 * at runtime. */
	constant_cases = NULL, num_constants = 0;
	p_cases = &self->a_switch.s_cases;
	temp    = 0;
	while ((cases = *p_cases) != NULL) {
		struct asm_sym *case_sym;
		ASSERT_AST(cases->tl_expr);
		/* Allocate the symbol for every case (We'll need them all
		 * eventually and this is a good spot to allocate them). */
		case_sym = cases->tl_asym;
		if likely(!case_sym) {
			case_sym = asm_newsym();
			if unlikely(!case_sym)
				goto err_cases;
			cases->tl_asym = case_sym;
		}

		/* NOTE: When the no-jmptab flag is set, act as though
		 *       no constants at all were being allowed. */
		if (cases->tl_expr->a_type == AST_CONSTEXPR &&
		    !(self->a_flag & AST_FSWITCH_NOJMPTAB) &&
		    asm_allowconst(cases->tl_expr->a_constexpr)) {
			/* Constant case (collect these and handle them individually).
			 * NOTE: Technically, we're reversing the order of constant expressions
			 *       here, but that's OK, considering they don't actually have any
			 *       side-effects. */
			*p_cases = cases->tl_next;
			++num_constants;
			cases->tl_next = constant_cases;
			constant_cases = cases;
			continue;
		}

		/* Generate a runtime check for this case. */
		if likely(!temp) {
			if (!has_expression) {
				/* Assemble text for the switch expression. */
				if (ast_genasm(self->a_switch.s_expr, ASM_G_FPUSHRES))
					goto err_cases;
				has_expression = true;
			}
			if unlikely(emit_runtime_check(self, cases->tl_expr, case_sym))
				goto err_cases;
		}
continue_next_case:
		p_cases = &cases->tl_next;
		continue;
err_cases:
		temp = -1;
		goto continue_next_case;
	}

	/* Re-append all of the constant cases at the end of the linked list of cases. */
	*p_cases = constant_cases;
	ASSERT((constant_cases != NULL) ==
	       (num_constants != 0));

	/* Check if something went wrong during creation of runtime cases. */
	if unlikely(temp)
		goto err;

	/* Now that all of the non-constant cases are out-of-the-way, we
	 * can simply move on to all of the actually constant cases. */

	/* Special case: If there are only 1 or less constants, generate runtime checks for them, too.
	 *               The additional overhead caused by generating a jump-table here would be
	 *               greater than the benefits it would give us. */
	if (num_constants <= 1) {
		size_t i;
/*use_runtime_checks:*/
		if (!has_expression) {
			/* Assemble text for the switch expression. */
			DO(ast_genasm(self->a_switch.s_expr, ASM_G_FPUSHRES));
			has_expression = true;
		}
		for (i = 0; i < num_constants; ++i) {
			DO(emit_runtime_check(self,
			                      constant_cases->tl_expr,
			                      constant_cases->tl_asym));
			constant_cases = constant_cases->tl_next;
		}
		ASSERT(!constant_cases);

		/* With all cases now in check, pop the expression and jump ahead to the default case. */
		DO(asm_putddi(self));
		DO(asm_gpop()); /* Pop the switch-expression. */

		/* Jump to the default case when no other was matched. */
		DO(asm_gjmp(ASM_JMP, default_sym));
		goto do_generate_block;
	}

#if 1
	/* Generate a jump table! */
	/* NOTE: Stack: ..., expr */

	{
		size_t i;
		DREF DeeRoDictObject *jump_table;
		struct Dee_rodict_builder jump_table_builder;
		DREF DeeObject *default_target;
		int32_t get_cid;
		Dee_rodict_builder_init_with_hint(&jump_table_builder, num_constants);
		for (i = 0; i < num_constants; ++i) {
			DeeObject *key;
			DREF DeeObject *case_target;
			ASSERT_AST(constant_cases->tl_expr);
			ASSERT(constant_cases->tl_expr->a_type == AST_CONSTEXPR);
			ASSERT(constant_cases->tl_asym);
			case_target = pack_target_tuple(constant_cases->tl_asym);
			if unlikely(!case_target) {
err_jump_table_builder:
				Dee_rodict_builder_fini(&jump_table_builder);
				goto err;
			}
			key = constant_cases->tl_expr->a_constexpr;
			Dee_Incref(key);
			if unlikely(Dee_rodict_builder_setitem_inherited(&jump_table_builder, key, case_target))
				goto err_jump_table_builder;
			constant_cases = constant_cases->tl_next;
		}

		/* With the jump table now generated, generate code looks like this:
		 * >>    push     const @<jump_table>             // expr, jump_table
		 * >>    swap                                     // jump_table, expr
		 * >>    push     const @(default.PC, default.SP) // jump_table, expr, default
		 * >>    callattr top, @"get", #2                 // target
		 * >>    unpack   pop, #2                         // target.PC, target.SP
		 * >>    jmp      pop, #pop                       // ... */
		jump_table = Dee_rodict_builder_pack(&jump_table_builder);
		if (asm_allowconst((DeeObject *)jump_table)) {
			jumptable_cid = asm_newconst((DeeObject *)jump_table);
			if unlikely(jumptable_cid < 0) {
err_jump_table:
				Dee_Decref_likely(jump_table);
				goto err;
			}
			if (asm_gpush_const((uint16_t)jumptable_cid))
				goto err_jump_table;
		} else {
			if (asm_gpush_constexpr((DeeObject *)jump_table))
				goto err_jump_table;
		}
		Dee_Decref_unlikely(jump_table);
		if (!has_expression) {
			/* Assemble text for the switch expression. */
			/* NOTE: Enforcing single-value mode here is _very_ important,
			 *       as failing to do so could allow user-code to construct
			 *       exploiting code that is capable of jumping to arbitrary
			 *       memory locations, whilst in exec-fast mode! */
			if (ast_genasm_one(self->a_switch.s_expr, ASM_G_FPUSHRES))
				goto err_cases;
			has_expression = true;
		} else {
			DO(asm_gswap());
		}
		/* jump_table, expr */
		default_target = pack_target_tuple(default_sym);
		if unlikely(!default_target)
			goto err;
		default_cid = asm_newconst(default_target);
		Dee_Decref_unlikely(default_target);
		if unlikely(default_cid < 0)
			goto err;
		DO(asm_gpush_const((uint16_t)default_cid)); /* jump_table, expr, default */
		get_cid = asm_newconst((DeeObject *)&str_get);
		if unlikely(get_cid < 0)
			goto err;
		DO(asm_gcallattr_const((uint16_t)get_cid, 2)); /* target */
		case_unpack_addr = asm_ip();
		DO(asm_gunpack(2)); /* target.PC, target.SP */
		DO(asm_gjmp_pop_pop());
		stack_size_after_jump = current_assembler.a_stackcur;
	}
#else
	goto use_runtime_checks;
#endif

do_generate_block:
	/* With all the jump-code out of the way, generate the switch block. */
	DO(ast_genasm(self->a_switch.s_block, ASM_G_FNORMAL));

	/* If the user didn't define a default symbol,
	 * it will jump where `break' is located at. */
	if (!ASM_SYM_DEFINED(default_sym))
		asm_defsym(default_sym);

	/* Define the switch break symbol and restore the assembler state. */
	asm_defsym(switch_break);
	ASSERT(current_assembler.a_finflag & ASM_FINFLAG_NOLOOP);
	current_assembler.a_finflag                  = old_finflag;
	current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = old_break;

	if (jumptable_cid >= 0 &&
	    (current_assembler.a_flag & (ASM_FOPTIMIZE | ASM_FOPTIMIZE_SIZE))) {
		/* Check if all jump targets share the same stack depth,
		 * and if they do, optimize the generated assembly:
		 * OLD:
		 * >>    push     const @<jump_table>             // expr, jump_table
		 * >>    swap                                     // jump_table, expr
		 * >>    push     const @(default.PC, default.SP) // jump_table, expr, default
		 * >>    callattr top, @"get", #2                 // target
		 * >>    unpack   pop, #2                         // target.PC, target.SP
		 * >>    jmp      pop, #pop                       // ...
		 * NEW:
		 * >>    push     const @<jump_table>             // expr, jump_table
		 * >>    swap                                     // jump_table, expr
		 * >>    push     const @default.PC               // jump_table, expr, default
		 * >>    callattr top, @"get", #2                 // target
		 * >>    jmp      pop                             // ...
		 */
		size_t i;
		uint16_t common_sp, alt_sp;
		DeeRoDictObject *jump_table;
		DeeObject *default_target;
		instruction_t *unpack;
		jump_table     = (DeeRoDictObject *)current_assembler.a_constv[jumptable_cid];
		default_target = current_assembler.a_constv[default_cid];
		if unlikely(!DeeRoDict_Check(jump_table))
			goto done; /* Shouldn't happen */
		if unlikely(!DeeTuple_Check(default_target))
			goto done; /* Shouldn't happen */
		if unlikely(DeeTuple_SIZE(default_target) != 2)
			goto done; /* Shouldn't happen */
		if (!try_get_integer_as_u16(DeeTuple_GET(default_target, 1), &common_sp))
			goto done;

		/* Make sure that all constant cases share the same common SP value. */
		for (i = 0; i < jump_table->rd_vsize; ++i) {
			DeeObject *case_target;
			case_target = _DeeRoDict_GetRealVTab(jump_table)[i].di_value;
			if unlikely(!DeeTuple_Check(case_target))
				goto done; /* Shouldn't happen */
			if unlikely(DeeTuple_SIZE(case_target) != 2)
				goto done; /* Shouldn't happen */
			if (!try_get_integer_as_u16(DeeTuple_GET(case_target, 1), &alt_sp))
				goto done;
			if (common_sp != alt_sp)
				goto done; /* Case-label has a different stack-indirection. */
		}
		unpack = &current_assembler.a_curr->sec_begin[case_unpack_addr + 0];
		ASSERT(unpack[0] == ASM_UNPACK);
		ASSERT(unpack[1] == 2);
		ASSERT(unpack[2] == (ASM_JMP_POP_POP & 0xff00) >> 8);
		ASSERT(unpack[3] == (ASM_JMP_POP_POP & 0xff));
		if (common_sp == stack_size_after_jump) {
			/* No adjustment is required! */
			unpack[0] = ASM_JMP_POP;
			unpack[1] = ASM_DELOP;
			unpack[2] = ASM_DELOP;
			unpack[3] = ASM_DELOP;
update_constants:
			/* Change all the constants.
			 * XXX: What if one of these constants got re-used in the mean time?
			 *      In that case we'd be changing somebody else constant, too.
			 *      I mean: It shouldn't be able to happen, but what if it could in the future? */
			current_assembler.a_constv[default_cid] = DeeTuple_GET(default_target, 0);
			Dee_Incref(DeeTuple_GET(default_target, 0));
			Dee_Decref_likely(default_target);
			for (i = 0; i < jump_table->rd_vsize; ++i) {
				DeeObject *case_target;
				struct Dee_dict_item *item;
				item = &_DeeRoDict_GetRealVTab(jump_table)[i];
				case_target = item->di_value;
				ASSERT(DeeTuple_Check(case_target));
				ASSERT(DeeTuple_SIZE(case_target) == 2);
				item->di_value = DeeTuple_GET(case_target, 0);
				Dee_Incref(DeeTuple_GET(case_target, 0));
				Dee_Decref_likely(case_target);
			}
		} else if (common_sp == stack_size_after_jump + 1) {
			/* >> callattr top, @"get", #2
			 * >> push     none
			 * >> swap
			 * >> jmp      pop
			 */
			unpack[0] = ASM_PUSH_NONE;
			unpack[1] = ASM_SWAP;
			unpack[2] = ASM_JMP_POP;
			unpack[3] = ASM_DELOP;
			goto update_constants;
		} else if (common_sp == stack_size_after_jump - 1) {
			/* >> callattr top, @"get", #2
			 * >> pop      #SP - 2
			 * >> jmp      pop */
			unpack[0] = ASM_POP_N;
			unpack[1] = 0;
			unpack[2] = ASM_JMP_POP;
			unpack[3] = ASM_DELOP;
			goto update_constants;
		}
	}
done:
	return 0;
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENSWITCH_C */
