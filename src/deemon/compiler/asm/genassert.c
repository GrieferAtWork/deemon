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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENASSERT_C
#define GUARD_DEEMON_COMPILER_ASM_GENASSERT_C 1

#include <deemon/api.h>

#include <deemon/asm.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/object.h>

#include "../../runtime/builtin.h"

#include <stddef.h> /* NULL */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#define DO(expr) if unlikely(expr) goto err

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
	Dee_operator_t operator_name;
	if (current_assembler.a_flag & ASM_FNOASSERT) {
		/* Discard the assert-expression and message and emit a constant true. */
		if (gflags & ASM_G_FPUSHRES)
			DO(ast_genasm(expr, gflags));
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
	assert_enter = asm_newsym();
	if unlikely(assert_enter == NULL)
		goto err; /* .cold.1: */
	assert_leave = asm_newsym();
	if unlikely(assert_leave == NULL)
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
		DO(ast_genasm(expr->a_operator.o_op0, ASM_G_FPUSHRES));
		if (!expr->a_operator.o_op1)
			goto emit_instruction;
		++argc;
		DO(ast_genasm_one(expr->a_operator.o_op1, ASM_G_FPUSHRES));
		if (!expr->a_operator.o_op2)
			goto emit_instruction;
		++argc;
		DO(ast_genasm_one(expr->a_operator.o_op2, ASM_G_FPUSHRES));
		if (!expr->a_operator.o_op3)
			goto emit_instruction;
		++argc;
		DO(ast_genasm_one(expr->a_operator.o_op3, ASM_G_FPUSHRES));

		/* Duplicate all operands. */
emit_instruction:
		DO(asm_putddi(ddi_ast));

		/* TODO: Some operands don't need to be duplicated, as re-loading them
		 *       from their base-expression doesn't have any side-effects.
		 *       Such operands include constants, or symbols that couldn't be
		 *       modified in the mean time (such as locals, refs, or args). */
		switch (argc) {

		case 4:
			DO(asm_gdup_n(2));
			DO(asm_gdup_n(2));
			DO(asm_gdup_n(2));
			DO(asm_gdup_n(2));
			break;

		case 3:
			DO(asm_gdup_n(1));
			DO(asm_gdup_n(1));
			DO(asm_gdup_n(1));
			break;

		case 2:
			DO(asm_gdup_n(0));
			DO(asm_gdup_n(0));
			break;

		case 1:
			DO(asm_gdup());
			break;

		default: break;
		}

		DO(asm_putddi(expr));

		/* With all the operands on-stack, as well as duplicated,
		 * it's time to perform the operation that's to-be asserted. */
		if (operator_name > OPERATOR_USERCOUNT ||
		    (op_instr = operator_instr_table[operator_name]) == 0 ||
		    (operand_mode = operator_opcount_table[operator_name],
		     (operand_mode & OPCOUNT_OPCOUNTMASK) != argc)) {
			int error;
			switch (operator_name) {

			case OPERATOR_CALL: {
				struct ast *kw_ast;
				DeeTypeObject *kw_type;
				if (argc != 2)
					goto fallback_generate_goperator;

				/* Special case: call-with-keywords (must also
				 * emit a cast-to-varkwds instruction if necessary) */
				kw_ast  = expr->a_operator.o_op2;
				kw_type = ast_predict_type_noanno(kw_ast);
				if (!kw_type || !DeeType_IsKw(kw_type)) {
					DO(asm_putddi(kw_ast));
					DO(asm_gcast_varkwds());
				}
				error = asm_gcall_tuple_kwds();
			}	break;

			case FAKE_OPERATOR_IS:
				if (argc != 2)
					goto fallback_generate_goperator;
				/* Special case: `assert a is b' */
				error = asm_gimplements();
				break;

			case FAKE_OPERATOR_SAME_OBJECT:
				if (argc != 2)
					goto fallback_generate_goperator;
				/* Special case: `assert a === b' */
				error = asm_gsameobj();
				break;

			case FAKE_OPERATOR_DIFF_OBJECT:
				if (argc != 2)
					goto fallback_generate_goperator;
				/* Special case: `assert a !== b' */
				error = asm_gdiffobj();
				break;

			default:
fallback_generate_goperator:
				/* Invoke the operator using a general-purpose instruction. */
				error = asm_goperator(operator_name, argc - 1);
				break;
			}
			if unlikely(error)
				goto err;
		} else {
			/* The operator has its own dedicated instruction, which we can use. */
			DO(asm_put(op_instr));
			asm_subsp(argc);

			/* STACK: a, [b, [c, [d]]], [check_cond] (remember the duplicates we created above) */

			/* Must unify the instruction behavior by fixing the
			 * stack (`check_cond' must always be present). */
			switch (operand_mode & OPCOUNT_RESULTMASK) {

			case OPCOUNT_PUSHFIRST:
				DO(argc > 1 ? asm_gdup_n(argc - 2) : asm_gdup()); /* `dup #argc-1' */
				break;

			case OPCOUNT_PUSHSECOND:
				DO(argc > 2 ? asm_gdup_n(argc - 3) : asm_gdup()); /* `dup #argc-2' */
				break;

			case OPCOUNT_PUSHTHIRD:
				DO(argc > 3 ? asm_gdup_n(argc - 4) : asm_gdup()); /* `dup #argc-3' */
				break;

			case OPCOUNT_PUSHFOURTH:
				ASSERT(!(argc > 4));
				DO(/*argc > 4 ? asm_gdup_n(argc-5) : */ asm_gdup()); /* `dup #argc-4' */
				break;

			case OPCOUNT_POPPUSHNONE:
				asm_incsp();
				DO(asm_gpop());
				ATTR_FALLTHROUGH
			case OPCOUNT_PUSHNONE:
				DO(asm_gpush_none()); /* Kind-of pointless, but still a case that can happen... */
				break;

			default: asm_incsp(); /* The instruction leaves behind the result. */
			}
		}
		DO(asm_putddi(ddi_ast));

		/* Duplicate `condition' to-be re-used as result of the assert expression. */
		if (gflags & ASM_G_FPUSHRES)
			DO(asm_gdup());

		/* STACK: a, [b, [c, [d]]] condition [condition] */
#define assert_cleanup assert_enter
		old_section = current_assembler.a_curr;
		if (old_section == &current_assembler.a_sect[SECTION_COLD]) {
			DO(asm_gjmp(ASM_JT, assert_cleanup));
			asm_decsp(); /* Adjust for `ASM_JT' */
		} else {
			DO(asm_gjmp(ASM_JF, assert_enter));
			asm_decsp(); /* Adjust for `ASM_JF' */
			current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
			DO(asm_putddi(ddi_ast));
			asm_defsym(assert_enter);
		}

		/* STACK: a, [b, [c, [d]]], [condition] */
		if (gflags & ASM_G_FPUSHRES)
			DO(asm_gpop()); /* Pop the duplicated `condition' */

		/* STACK: a, [b, [c, [d]]] */
		if (message) {
			DO(ast_genasm_one(message, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
		} else {
			DO(asm_gpush_none());
		}

		/* STACK: a, [b, [c, [d]]], message */
		DO(asm_grrot(argc + 1));

		/* STACK: message, a, [b, [c, [d]]] */
		DO(asm_gpush_u16(operator_name)); /* Push the instruction id */

		/* STACK: message, a, [b, [c, [d]]], operator_name */
		DO(asm_grrot(argc + 1));

		/* STACK: message, operator_name, a, [b, [c, [d]]] */

		/* Add `deemon' to the import list. */
		deemon_modid = asm_newmodule(DeeModule_GetDeemon());
		if unlikely(deemon_modid < 0)
			goto err;

		/* Generate the call to the builtin assertion function. */
		DO(asm_gcall_extern((uint16_t)deemon_modid, id___assert, argc + 2));

		/* Pop the result when it isn't being used. */
		if (!(gflags & ASM_G_FPUSHRES))
			DO(asm_gpop());

		/* Jump back to regular code. */
		DO(asm_gjmp(ASM_JMP, assert_leave));
		current_assembler.a_curr = old_section;

		/* Generate the stack-cleanup for when the assertion didn't fail. */
		asm_addsp(argc);
		if (old_section == &current_assembler.a_sect[SECTION_COLD]) {
			asm_defsym(assert_cleanup);
		} else {
			DO(asm_putddi(ddi_ast));
		}

		/* STACK: a, [b, [c, [d]]] [condition] */
		if (gflags & ASM_G_FPUSHRES)
			DO(asm_grrot(argc + 1));

		/* STACK: [condition] a, [b, [c, [d]]] */
		DO(asm_gadjstack(-(int16_t)argc));

		/* STACK: [condition] */
		asm_defsym(assert_leave);
		goto done;
#undef assert_cleanup
	}

	if (expr->a_type == AST_ACTION) {
		switch (expr->a_flag) {

		case AST_FACTION_IN:
			/* Special case: in-expressions. */
			operator_name = OPERATOR_CONTAINS;
			argc          = 2;
			DO(ast_genasm(expr->a_action.a_act0, ASM_G_FPUSHRES));
			DO(ast_genasm_one(expr->a_action.a_act1, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gswap());
			goto emit_instruction;

		case AST_FACTION_CALL_KW:
			/* Special case: call with keywords. */
			operator_name = OPERATOR_CALL;
			argc          = 3;
			DO(ast_genasm(expr->a_action.a_act0, ASM_G_FPUSHRES));
			DO(ast_genasm_one(expr->a_action.a_act1, ASM_G_FPUSHRES));
			DO(ast_genasm_one(expr->a_action.a_act2, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			goto emit_instruction;

		case AST_FACTION_IS:
		case AST_FACTION_SAMEOBJ:
		case AST_FACTION_DIFFOBJ:
			/* Special case: `a is b', `a === b' and `a !== b'. */
			switch (expr->a_flag) {
			case AST_FACTION_IS:
				operator_name = FAKE_OPERATOR_IS;
				break;
			case AST_FACTION_SAMEOBJ:
				operator_name = FAKE_OPERATOR_SAME_OBJECT;
				break;
			case AST_FACTION_DIFFOBJ:
				operator_name = FAKE_OPERATOR_DIFF_OBJECT;
				break;
			default: __builtin_unreachable();
			}
			DO(ast_genasm(expr->a_action.a_act0, ASM_G_FPUSHRES));
			DO(ast_genasm_one(expr->a_action.a_act1, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			argc = 2;
			goto emit_instruction;

		default: break;
		}
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
	DO(ast_genasm(expr, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	if (gflags & ASM_G_FPUSHRES)
		DO(asm_gdup());
	old_section = current_assembler.a_curr;
	if (old_section == &current_assembler.a_sect[SECTION_COLD]) {
		DO(asm_gjmp(ASM_JT, assert_leave));
		asm_decsp(); /* Adjust for `ASM_JT' / `ASM_JF' */
	} else {
		DO(asm_gjmp(ASM_JF, assert_enter));
		asm_decsp(); /* Adjust for `ASM_JT' / `ASM_JF' */
		current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
		DO(asm_putddi(ddi_ast));
	}
	asm_defsym(assert_enter);

	/* Now we're inside the assertion handler. */
	if (gflags & ASM_G_FPUSHRES)
		DO(asm_gpop()); /* The asserted expression. */
	argc = 0;

	/* Generate code for the assertion message. */
	if (message) {
		DO(ast_genasm_one(message, ASM_G_FPUSHRES));
		DO(asm_putddi(ddi_ast));
		++argc;
	}

	/* Add `deemon' to the import list. */
	deemon_modid = asm_newmodule(DeeModule_GetDeemon());
	if unlikely(deemon_modid < 0)
		goto err;

	/* Generate the call to the builtin assertion function. */
	DO(asm_gcall_extern((uint16_t)deemon_modid, id___assert, argc));

	/* Pop the result when it isn't being used. */
	if (!(gflags & ASM_G_FPUSHRES))
		DO(asm_gpop());

	/* Jump back to regular code if we went into the cold section. */
	if (old_section != &current_assembler.a_sect[SECTION_COLD])
		DO(asm_gjmp(ASM_JMP, assert_leave));
	current_assembler.a_curr = old_section;
	asm_defsym(assert_leave);
done:
	return 0;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENASSERT_C */
