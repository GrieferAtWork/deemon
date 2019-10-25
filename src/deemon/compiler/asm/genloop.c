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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENLOOP_C
#define GUARD_DEEMON_COMPILER_ASM_GENLOOP_C 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/compiler/assembler.h>

DECL_BEGIN

/* @param: loop_flags:   Set of `AST_FLOOP_*'
 * @param: elem_or_cond: The loop element target ([0..1] in a foreach loop),
 *                       or the loop-continue condition ([0..1] in other loop types).
 * @param: iter_or_next: The loop iterator ([1..1] in a foreach loop),
 *                       or an optional expression executed at the end
 *                       of each iteration, and jumped to by `continue' ([0..1])
 * @param: block:        The main loop block executed in each iteration ([0..1])
 * @param: ddi_ast:      A branch used for debug information.
 * @return: * :         `loop_break' -- This symbol must be defined immediately
 *                       after the loop, however after variables allocated by
 *                       the scope have been disposed of.
 */
INTERN WUNUSED NONNULL((5)) struct asm_sym *
(DCALL asm_genloop)(uint16_t loop_flags,
                    struct ast *elem_or_cond,
                    struct ast *iter_or_next,
                    struct ast *block,
                    struct ast *ddi_ast) {
	struct asm_sym *old_break;
	struct asm_sym *old_continue;
	struct asm_sym *loop_break, *loop_continue;
	uint16_t old_finflag;
	/* Save old loop control symbols. */
	old_break    = current_assembler.a_loopctl[ASM_LOOPCTL_BRK];
	old_continue = current_assembler.a_loopctl[ASM_LOOPCTL_CON];
	/* Create new symbols for loop control. */
	loop_break = asm_newsym();
	if unlikely(!loop_break)
		goto err;
	loop_continue = asm_newsym();
	if unlikely(!loop_continue)
		goto err;
	/* Setup the new loop control symbols. */
	current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = loop_break;
	current_assembler.a_loopctl[ASM_LOOPCTL_CON] = loop_continue;
	/* Set the out-of-loop flag for execution of existing finally handler. */
	old_finflag = current_assembler.a_finflag;
	current_assembler.a_finflag |= ASM_FINFLAG_NOLOOP;

	/* Now let's get to work! */
	if (loop_flags & AST_FLOOP_FOREACH) {
		struct asm_sec *prev_section;
		/* >> foreach()-style loop. */
		/* Start out by evaluating the loop iterator. */
		ASSERT_AST(iter_or_next);
		if (iter_or_next->a_type == AST_OPERATOR &&
		    iter_or_next->a_flag == OPERATOR_ITERSELF &&
		    !(iter_or_next->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS)) &&
		    iter_or_next->a_operator.o_op0) {
			/* Generate a sequence as an ASP, thus optimizing away unnecessary casts. */
			if (ast_genasm_asp(iter_or_next->a_operator.o_op0, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(iter_or_next))
				goto err;
			if (asm_giterself())
				goto err;
		} else {
			if (ast_genasm(iter_or_next, ASM_G_FPUSHRES))
				goto err;
		}
		/* This is where the loop starts! (and where `continue' jump to) */
		asm_defsym(loop_continue);
		/* The foreach instruction will jump to the break-address
		 * when the iterator has been exhausted. */
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_gjmp(ASM_FOREACH, loop_break))
			goto err;
		asm_diicsp(); /* -2, +1: loop-branch of a foreach instruction. */
		/* HINT: Right now, the stack looks like this:
		 *       ..., iterator, elem */
		prev_section = current_assembler.a_curr;
		/* Put the loop block into the cold section if it's unlikely to be executed. */
		if (loop_flags & AST_FLOOP_UNLIKELY &&
		    prev_section != &current_assembler.a_sect[SECTION_COLD]) {
			struct asm_sym *loop_begin;
			loop_begin = asm_newsym();
			if unlikely(!loop_begin)
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gjmp(ASM_JMP, loop_begin))
				goto err; /* Jump into cold text. */
			current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
		}

		/* Store the foreach iterator into the loop variable(s) */
		if (elem_or_cond) {
			if (asm_gpop_expr(elem_or_cond))
				goto err;
		} else {
			if (asm_gpop())
				goto err;
		}

		/* Generate the loop block itself. */
		if (block &&
		    ast_genasm(block, ASM_G_FNORMAL))
			goto err;

		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_gjmp(ASM_JMP, loop_continue))
			goto err; /* Jump back to yield the next item. */
		current_assembler.a_curr = prev_section;

		/* -1: Adjust for the `ASM_FOREACH' instruction popping the iterator once it's empty. */
		asm_decsp();
	} else if (loop_flags & AST_FLOOP_POSTCOND) {
		struct asm_sym *loop_block = asm_newsym();
		if unlikely(!loop_block)
			goto err;
		asm_defsym(loop_block);
		/* NOTE: There's no point in trying to put some if this stuff into
		 *       the cold section when the `AST_FLOOP_UNLIKELY' flag is set.
		 *       Since the loop-block is always executed, we'd always have
		 *       to jump into cold text, which would kind-of defeat the purpose
		 *       considering that it's meant to contain code that's unlikely
		 *       to be called. */
		/* Generate the loop itself. */
		if (block &&
		    ast_genasm(block, ASM_G_FNORMAL))
			goto err;

		/* Evaluate the condition after the loop (and after the continue-symbol). */
		asm_defsym(loop_continue);
		if (iter_or_next &&
		    ast_genasm(iter_or_next, ASM_G_FNORMAL))
			goto err;
		if (elem_or_cond) {
			if (asm_gjcc(elem_or_cond, ASM_JT, loop_block, ddi_ast))
				goto err; /* if (cond) goto loop_block; */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gjmp(ASM_JMP, loop_block))
				goto err; /* if (cond) goto loop_block; */
		}
	} else {
		struct asm_sym *loop_block;
		if (!iter_or_next) {
			loop_block = loop_continue;
		} else {
			loop_block = asm_newsym();
			if unlikely(!loop_block)
				goto err;
		}
		asm_defsym(loop_block);
		/* Evaluate the condition before the loop. */
		if (loop_flags & AST_FLOOP_UNLIKELY &&
		    current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) {
			struct asm_sec *prev_section;
			struct asm_sym *loop_enter;
			loop_enter = asm_newsym();
			if unlikely(!loop_enter)
				goto err;
			if (asm_gjcc(elem_or_cond, ASM_JT, loop_enter, ddi_ast))
				goto err; /* if (cond) goto enter_loop; */

			prev_section             = current_assembler.a_curr;
			current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
			asm_defsym(loop_enter);

			/* Generate the loop itself. */
			if (block &&
			    ast_genasm(block, ASM_G_FNORMAL))
				goto err;

			if (iter_or_next) {
				asm_defsym(loop_continue);
				if (ast_genasm(iter_or_next, ASM_G_FNORMAL))
					goto err;
			}
			/* Jump back to re-evaluate the condition. */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gjmp(ASM_JMP, loop_block))
				goto err;
			current_assembler.a_curr = prev_section;
		} else {
			if (elem_or_cond &&
			    asm_gjcc(elem_or_cond, ASM_JF, loop_break, ddi_ast))
				goto err; /* if (!(cond)) break; */

			/* Generate the loop itself. */
			if (block &&
			    ast_genasm(block, ASM_G_FNORMAL))
				goto err;

			if (iter_or_next) {
				asm_defsym(loop_continue);
				if (ast_genasm(iter_or_next, ASM_G_FNORMAL))
					goto err;
			}
			/* Jump back to re-evaluate the condition. */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gjmp(ASM_JMP, loop_block))
				goto err;
		}
	}

	ASSERT(current_assembler.a_finflag & ASM_FINFLAG_NOLOOP);
	current_assembler.a_finflag = old_finflag;

	/* Restore old loop control symbols. */
	current_assembler.a_loopctl[ASM_LOOPCTL_CON] = old_continue;
	current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = old_break;

	/* Return the break-symbol, leaving it up to the caller to define it. */
	return loop_break;
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENLOOP_C */
