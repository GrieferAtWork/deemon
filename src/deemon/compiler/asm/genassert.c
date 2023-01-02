/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_GENASSERT_C
#define GUARD_DEEMON_COMPILER_ASM_GENASSERT_C 1

#include <deemon/api.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/module.h>
#include <deemon/object.h>

#include "../../runtime/builtin.h"

DECL_BEGIN

/* Quick translation table for most basic operator instruction codes. */
INTDEF instruction_t const operator_instr_table[];

#define OPCOUNT_OPCOUNTMASK 0x0f
#define OPCOUNT_RESULTMASK  0xf0
#define OPCOUNT_INSTRIN     0x00 /* The instruction intrinsically pushes a result. */
#define OPCOUNT_PUSHFIRST   0x10 /* You must re-return the first operand. */
#define OPCOUNT_PUSHSECOND  0x20 /* You must re-return the second operand. */
#define OPCOUNT_PUSHTHIRD   0x30 /* You must re-return the third operand. */
#define OPCOUNT_PUSHFOURTH  0x40 /* You must re-return the fourth operand. */
#define OPCOUNT_POPPUSHNONE 0x70 /* You must pop one object, the push `none'. */
#define OPCOUNT_PUSHNONE    0x80 /* You must re-return none. */
INTDEF uint8_t const operator_opcount_table[OPERATOR_USERCOUNT];

INTERN WUNUSED NONNULL((1, 3)) int
(DCALL asm_genassert)(struct ast *__restrict expr,
                      struct ast *message,
                      struct ast *ddi_ast,
                      unsigned int gflags) {
	struct asm_sym *assert_enter;
	struct asm_sym *assert_leave;
	struct asm_sec *old_section;
	uint8_t argc;
	int32_t deemon_modid;
	uint16_t operator_name;
	if (current_assembler.a_flag & ASM_FNOASSERT) {
		/* Discard the assert-expression and message and emit a constant true. */
		if (gflags & ASM_G_FPUSHRES) {
			if (ast_genasm(expr, gflags))
				goto err;
		}
		goto done;
	}
	/* Assertions have their own action due to their very special encoding:
	 * >> assert foo() == bar(), "This is bad";
	 * This generates the following assembly:
	 * >>    push  <foo>
	 * >>    call  top, #0
	 * >>    push  <bar>
	 * >>    call  top, #0
	 * >>    dup   #1             // Copy <foo>
	 * >>    dup   #1             // Copy <bar>
	 * >>    cmp   eq top, pop    // Do the comparison
	 * >>   [dup]                 // When the result is used
	 * >>    jf    .cold.1f
	 * >>   [rrot  #3]            // When the result is used
	 * >>    pop
	 * >>    pop
	 * >>2:
	 * >>
	 * >>.cold.1:
	 * >>   [pop]                 // When the result is used
	 * >>                         // Stack: foo, bar
	 * >>    push  @"This is bad" // Stack: foo, bar, message  --- When no message is given, push `none' instead
	 * >>    rrot  #3             // Stack: message, foo, bar
	 * >>    push  $__eq__        // Stack: message, foo, bar, id  --- Push the operator id as an integer onto the stack.
	 * >>    rrot  #3             // Stack: message, id, foo, bar
	 * >>    call  @deemon:@__assert, #4 // Call the assertion handler (implementation-specific)
	 * >>   [pop]                 // When the result isn't used.
	 * >>    jmp   2b             // This may seem redundant, but without this, peephole breaks.
	 * >>                         // Also: this is required to no confuse debuggers, and in the
	 * >>                         //       event that the assertion-failure function was overwritten,
	 * >>                         //       it may actually return once again!
	 */
	if unlikely((assert_enter = asm_newsym()) == NULL)
		goto err; /* .cold.1: */
	if unlikely((assert_leave = asm_newsym()) == NULL)
		goto err; /* 2: */
	if (expr->a_type == AST_OPERATOR &&
	    /* NOTE: Don't handle varargs operators. */
	    !(expr->a_operator.o_exflag & AST_OPERATOR_FVARARGS) &&
	    /* NOTE: Don't handle invalid operators. */
	    (operator_name = expr->a_flag,
	     operator_name < OPERATOR_INC || operator_name > OPERATOR_INPLACE_POW)) {
		instruction_t op_instr;
		uint8_t operand_mode;
		/* Figure out how many operands there are while generating code for the operator itself. */
		argc = 1;
		if (ast_genasm(expr->a_operator.o_op0, ASM_G_FPUSHRES))
			goto err;
		if (!expr->a_operator.o_op1)
			goto emit_instruction;
		++argc;
		if (ast_genasm_one(expr->a_operator.o_op1, ASM_G_FPUSHRES))
			goto err;
		if (!expr->a_operator.o_op2)
			goto emit_instruction;
		++argc;
		if (ast_genasm_one(expr->a_operator.o_op2, ASM_G_FPUSHRES))
			goto err;
		if (!expr->a_operator.o_op3)
			goto emit_instruction;
		++argc;
		if (ast_genasm_one(expr->a_operator.o_op3, ASM_G_FPUSHRES))
			goto err;
emit_instruction:
		/* Duplicate all operands. */
		if (asm_putddi(ddi_ast))
			goto err;
		/* TODO: Some operands don't need to be duplicated, as re-loading them
		 *       from their base-expression doesn't have any side-effects.
		 *       Such operands include constants, or symbols that couldn't be
		 *       modified in the mean time (such as locals, refs, or args). */
		switch (argc) {

		case 4:
			if (asm_gdup_n(2))
				goto err;
			if (asm_gdup_n(2))
				goto err;
			if (asm_gdup_n(2))
				goto err;
			if (asm_gdup_n(2))
				goto err;
			break;

		case 3:
			if (asm_gdup_n(1))
				goto err;
			if (asm_gdup_n(1))
				goto err;
			if (asm_gdup_n(1))
				goto err;
			break;

		case 2:
			if (asm_gdup_n(0))
				goto err;
			if (asm_gdup_n(0))
				goto err;
			break;

		case 1:
			if (asm_gdup())
				goto err;
			break;

		default: break;
		}

		if (asm_putddi(expr))
			goto err;
		/* With all the operands on-stack, as well as duplicated,
		 * it's time to perform the operation that's to-be asserted. */
		if (operator_name > OPERATOR_USERCOUNT ||
		    (op_instr = operator_instr_table[operator_name]) == 0 ||
		    (operand_mode = operator_opcount_table[operator_name],
		     (operand_mode & OPCOUNT_OPCOUNTMASK) != argc)) {
			/* Invoke the operator using a general-purpose instruction. */
			if (asm_goperator(operator_name, argc - 1))
				goto err;
		} else {
			/* The operator has its own dedicated instruction, which we can use. */
			if (asm_put(op_instr))
				goto err;
			asm_subsp(argc);
			/* STACK: a, [b, [c, [d]]], [check_cond] (remember the duplicates we created above) */
			/* Must unify the instruction behavior by fixing the
			 * stack (`check_cond' must always be present). */
			switch (operand_mode & OPCOUNT_RESULTMASK) {

			case OPCOUNT_PUSHFIRST:
				if (argc > 1 ? asm_gdup_n(argc - 2) : asm_gdup())
					goto err; /* `dup #argc-1' */
				break;

			case OPCOUNT_PUSHSECOND:
				if (argc > 2 ? asm_gdup_n(argc - 3) : asm_gdup())
					goto err; /* `dup #argc-2' */
				break;

			case OPCOUNT_PUSHTHIRD:
				if (argc > 3 ? asm_gdup_n(argc - 4) : asm_gdup())
					goto err; /* `dup #argc-3' */
				break;

			case OPCOUNT_PUSHFOURTH:
				ASSERT(!(argc > 4));
				if (/*argc > 4 ? asm_gdup_n(argc-5) : */ asm_gdup())
					goto err; /* `dup #argc-4' */
				break;

			case OPCOUNT_POPPUSHNONE:
				asm_incsp();
				if (asm_gpop())
					goto err;
				ATTR_FALLTHROUGH
			case OPCOUNT_PUSHNONE:
				if (asm_gpush_none())
					goto err; /* Kind-of pointless, but still a case that can happen... */
				break;

			default: asm_incsp(); /* The instruction leaves behind the result. */
			}
		}
		if (asm_putddi(ddi_ast))
			goto err;
		/* Duplicate `condition' to-be re-used as result of the assert expression. */
		if ((gflags & ASM_G_FPUSHRES) && asm_gdup())
			goto err;
			/* STACK: a, [b, [c, [d]]] condition [condition] */
#define assert_cleanup assert_enter
		old_section = current_assembler.a_curr;
		if (old_section == &current_assembler.a_sect[SECTION_COLD]) {
			if (asm_gjmp(ASM_JT, assert_cleanup))
				goto err;
			asm_decsp(); /* Adjust for `ASM_JT' */
		} else {
			if (asm_gjmp(ASM_JF, assert_enter))
				goto err;
			asm_decsp(); /* Adjust for `ASM_JF' */
			current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
			if (asm_putddi(ddi_ast))
				goto err;
			asm_defsym(assert_enter);
		}
		/* STACK: a, [b, [c, [d]]], [condition] */
		if ((gflags & ASM_G_FPUSHRES) && asm_gpop())
			goto err; /* Pop the duplicated `condition' */
		/* STACK: a, [b, [c, [d]]] */
		if (message) {
			if (ast_genasm_one(message, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
		} else {
			if (asm_gpush_none())
				goto err;
		}
		/* STACK: a, [b, [c, [d]]], message */
		if (asm_grrot(argc + 1))
			goto err;
		/* STACK: message, a, [b, [c, [d]]] */
		if (asm_gpush_u16(operator_name))
			goto err; /* Push the instruction id */
		/* STACK: message, a, [b, [c, [d]]], operator_name */
		if (asm_grrot(argc + 1))
			goto err;
		/* STACK: message, operator_name, a, [b, [c, [d]]] */
		/* Add `deemon' to the import list. */
		deemon_modid = asm_newmodule(DeeModule_GetDeemon());
		if unlikely(deemon_modid < 0)
			goto err;
		/* Generate the call to the builtin assertion function. */
		if (asm_gcall_extern((uint16_t)deemon_modid, id___assert, argc + 2))
			goto err;
		/* Pop the result when it isn't being used. */
		if (!(gflags & ASM_G_FPUSHRES) && asm_gpop())
			goto err;
		/* Jump back to regular code. */
		if (asm_gjmp(ASM_JMP, assert_leave))
			goto err;
		current_assembler.a_curr = old_section;
		/* Generate the stack-cleanup for when the assertion didn't fail. */
		asm_addsp(argc);
		if (old_section == &current_assembler.a_sect[SECTION_COLD])
			asm_defsym(assert_cleanup);
		else if (asm_putddi(ddi_ast))
			goto err;
		/* STACK: a, [b, [c, [d]]] [condition] */
		if ((gflags & ASM_G_FPUSHRES) && asm_grrot(argc + 1))
			goto err;
		/* STACK: [condition] a, [b, [c, [d]]] */
		if (asm_gadjstack(-(int16_t)argc))
			goto err;
		/* STACK: [condition] */
		asm_defsym(assert_leave);
		goto done;
#undef assert_cleanup
	}

	if (expr->a_type == AST_ACTION &&
	    expr->a_flag == AST_FACTION_IN) {
		/* Special case: in-expressions. */
		operator_name = OPERATOR_CONTAINS;
		argc          = 2;
		if (ast_genasm(expr->a_action.a_act0, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(expr->a_action.a_act1, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_gswap())
			goto err;
		goto emit_instruction;
	}
#if 0
	if (expr->a_type == AST_ACTION &&
	    expr->a_flag == AST_FACTION_IS) {
		/* TODO: Special case: is-expressions. */
		goto emit_instruction;
	}
	if (expr->a_type == AST_ACTION &&
	    (expr->a_flag == AST_FACTION_SAMEOBJ ||
	     expr->a_flag == AST_FACTION_DIFFOBJ)) {
		/* TODO: Special case: same/diff-object expressions. */
		goto emit_instruction;
	}
#endif
	/* Fallback: Do not include extended information in the failure.
	 * In this kind of message, information about operands is omit. */
	if (ast_genasm(expr, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	if ((gflags & ASM_G_FPUSHRES) && asm_gdup())
		goto err;
	old_section = current_assembler.a_curr;
	if (old_section == &current_assembler.a_sect[SECTION_COLD]) {
		if (asm_gjmp(ASM_JT, assert_leave))
			goto err;
		asm_decsp(); /* Adjust for `ASM_JT' / `ASM_JF' */
	} else {
		if (asm_gjmp(ASM_JF, assert_enter))
			goto err;
		asm_decsp(); /* Adjust for `ASM_JT' / `ASM_JF' */
		current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
		if (asm_putddi(ddi_ast))
			goto err;
	}
	asm_defsym(assert_enter);
	/* Now we're inside the assertion handler. */
	if ((gflags & ASM_G_FPUSHRES) && asm_gpop())
		goto err; /* The asserted expression. */
	argc = 0;
	/* Generate code for the assertion message. */
	if (message) {
		if (ast_genasm_one(message, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		++argc;
	}
	/* Add `deemon' to the import list. */
	deemon_modid = asm_newmodule(DeeModule_GetDeemon());
	if unlikely(deemon_modid < 0)
		goto err;
	/* Generate the call to the builtin assertion function. */
	if (asm_gcall_extern((uint16_t)deemon_modid, id___assert, argc))
		goto err;
	/* Pop the result when it isn't being used. */
	if (!(gflags & ASM_G_FPUSHRES) && asm_gpop())
		goto err;
	/* Jump back to regular code if we went into the cold section. */
	if (old_section != &current_assembler.a_sect[SECTION_COLD] &&
	    asm_gjmp(ASM_JMP, assert_leave))
		goto err;
	current_assembler.a_curr = old_section;
	asm_defsym(assert_leave);
done:
	return 0;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENASSERT_C */
