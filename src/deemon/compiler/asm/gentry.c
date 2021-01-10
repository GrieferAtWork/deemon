/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_GENTRY_C
#define GUARD_DEEMON_COMPILER_ASM_GENTRY_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/object.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

DECL_BEGIN

INTERN WUNUSED NONNULL((1)) int
(DCALL asm_gentry)(struct ast *__restrict try_ast,
                   unsigned int gflags) {
	/* Guarding addresses of all sections. */
	code_addr_t guard_begin[SECTION_TEXTCOUNT];
	code_addr_t guard_end[SECTION_TEXTCOUNT];
	/* [0..1][(b != NULL) == (e != NULL)] begin/end of the guarded section
	 *  NOTE: When NULL, the protected code did not write to the section. */
	struct {
		struct asm_sym *b, *e;
	} guard[SECTION_TEXTCOUNT];
	struct catch_expr *iter, *end;
	uint16_t i, old_finflag;
	bool is_guarding;
	struct asm_sym *next_handler;
	struct asm_sym *my_first_finally, *existing_finally;
	uint16_t guard_finflags, except_index;
	struct asm_sym *after_catch;
	struct handler_frame hand_frame;
	size_t catch_mask_c;
	DREF DeeTypeObject *catch_mask, **catch_mask_v; /* [owned_if(!= catch_mask)] */
	ASSERT(try_ast->a_type == AST_TRY);
	after_catch = NULL;

	/* TODO: Optimize:
	 * >> try {
	 * >>     newitem = it.operator iter();
	 * >> } catch (Signal.StopIteration) {
	 * >>     print "Iterator exhausted";
	 * >>     return;
	 * >> }
	 * Into:
	 * >>     foreach (newitem: it) goto gotit;
	 * >>     print "Iterator exhausted";
	 * >>     return;
	 * >> gotit:
	 * ASSEMBLY:
	 * >>     push    @it
	 * >>     foreach top, 1f
	 * >>     pop     @newitem                   // Pushed by `foreach'
	 * >>     pop                                // The iterator itself
	 * >>     jmp     2f
	 * >> 1:  print   @"Iterator exhausted", nl
	 * >>     ret
	 * >> 2:
	 * The reason this works is because `foreach' as an instruction basically does this:
	 * >> ITEM = TOP.NEXT()
	 * >> IF ITEM !IS BOUND THEN
	 * >>     POP()
	 * >>     JUMP PC+IMM
	 * >> FI
	 * >> PUSH(ITEM)
	 * Meaning it's more of a push_next_or_pop_and_jmp
	 *
	 * This way, we're not required to introduce a new exception guard
	 * (in case we can prove that no other expression could possible throw
	 * a StopIteration signal), and we can keep the runtime from needing
	 * to unnecessarily throw an exception (thus introducing the overhead
	 * to needing to keep track of the traceback), when we're just going
	 * to catch it immediately afterwards irregardless.
	 *
	 * -> Implement this by setting some flag to encode `iternext' branches as
	 *   `foreach' instructions that will jump to the exception handler `HND':
	 * DEEMON:
	 * >> x.operator iter();
	 * ASM:
	 * >>     push    @x
	 * >> 1:  foreach top, 2f
	 * >>     pop     #SP - 2 // `swap; pop' (write the yielded item into the slot still containing `x')
	 * >>
	 * >>.if $$SECTION == ".cold"
	 * >>     jmp     3f
	 * >>.endif
	 * >>
	 * >>.pushsection .cold
	 * >>.adjstack    1b.SP
	 * >> 2:  adjstack #HND.SP
	 * >>     jmp     HND
	 * >>.popsection
	 * >>
	 * >>.if $$SECTION == ".cold"
	 * >> 3:
	 * >>.endif
	 * >>.adjstack    4b.SP
	 * Peephole optimization will then be able to remove the cold-section adjustment
	 * code, if that code turns out to be unnecessary.
	 */


	/* Keep track of where different sections are currently at,
	 * so we can safely determine what changed afterwards and
	 * therewith generate appropriate exception handlers for
	 * everything that is located within.
	 * >> We must do it this way because the guarded code
	 *    may be writing text to more than one section, in
	 *    which case we must still guard everything it wrote! */
	for (i = 0; i < SECTION_TEXTCOUNT; ++i)
		guard_begin[i] = asm_secip(i);

	/* Save the priority index of the handlers that we're going to generate.
	 * This is the vector index where we're going to insert our handlers below. */
	except_index = current_assembler.a_exceptc;

	my_first_finally = NULL;
	existing_finally = current_assembler.a_finsym;
	/* If this we're currently inside of a loop, we much check
	 * for finally-handlers, because if there are some, then
	 * any break/continue statements inside must first jump
	 * to the nearest finally-block, which must then be executed
	 * before continuing on its path to execute more handler,
	 * until eventually jumping to where the break was meant to go. */
	end         = (iter = try_ast->a_try.t_catchv) + try_ast->a_try.t_catchc;
	old_finflag = current_assembler.a_finflag;
	for (; iter != end; ++iter) {
		if (!(iter->ce_flags & EXCEPTION_HANDLER_FFINALLY))
			continue;
		my_first_finally = asm_newsym();
		if unlikely(!my_first_finally)
			goto err;
		current_assembler.a_finsym  = my_first_finally;
		current_assembler.a_finflag = ASM_FINFLAG_NORMAL;
		goto gen_guard;
	}

gen_guard:
	/* HINT: We propagate the expression result of the guarded expression outwards.
	 *       I'm not quite sure, and I'm too lazy to look it up right now, but if
	 *       I remember correctly, this is something I wanted to do in the old
	 *       deemon, but never could because of how convoluted its assembly
	 *       generator was...
	 *       I mean: I didn't even understood how symbols, or relocations are
	 *               meant to work, or knew what they were at all.
	 *               Looking back, it's amazing that I managed to create something
	 *               that worked, knowing so little about how it's done correctly. */
	if (ast_genasm(try_ast->a_try.t_guard, gflags))
		goto err;

	/* Check if a loop control statement was used within the guarded block.
	 * Because if one was, then we must do some special handling to compile
	 * all of our finally-handlers. */
	guard_finflags              = current_assembler.a_finflag;
	current_assembler.a_finsym  = existing_finally;
	current_assembler.a_finflag = old_finflag;

	/* This is where the guarded section ends. */
	for (i = 0; i < SECTION_TEXTCOUNT; ++i)
		guard_end[i] = asm_secip(i);

	/* Figure out what has changed and generate appropriate symbols. */
	bzero(guard, sizeof(guard));
	is_guarding = false;
	for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
		if (guard_begin[i] != guard_end[i]) {
			guard[i].b = asm_newsym();
			if unlikely(!guard[i].b)
				goto err;
			guard[i].e = asm_newsym();
			if unlikely(!guard[i].e)
				goto err;
			guard[i].b->as_sect = i;
			guard[i].e->as_sect = i;
#if 1
			/* All right. This is kind-of lazy.
			 * The problem (that could only be fixed properly by
			 * introducing some mechanism to define symbols at the
			 * current location when the current section is changed)
			 * originates from code like this:
			 * >> try {
			 * >>     __stack local inner_stack = 42;
			 * >>     try {
			 * >>         print "Try",inner_stack;
			 * >>     } catch (...) {
			 * >>         print "Inner",inner_stack;
			 * >>     }
			 * >> } catch (...) {
			 * >>     print "Outer";
			 * >> }
			 * When the Outer exception handler gets assembled, it will
			 * create a total of 4 symbols for the begin/end of its
			 * protection are in all text sections covered by the
			 * try-block. (.text and .cold)
			 * It then simply defines all the begin-symbols at the
			 * associated text positions of the affected section,
			 * later to check if the section's text point changed,
			 * in which case an exception handler entry is generated
			 * to cover the affected area.
			 * However, these symbols (like all others) also include
			 * tracking information for the respective stack-depth.
			 * And while that information isn't actually required by
			 * the exception handler (it only needs to know the stack-depth
			 * at the exception handler entry point, but not at the coverage
			 * entry/exit points in all sections), those symbols still
			 * include that information.
			 * However, looking back at the code, the inner catch-handler
			 * will be placed in the .cold section, yet the outer handler
			 * would have otherwise created a symbol at its entry point,
			 * not realizing that the stack address at that location is
			 * invalid.
			 * So to go the easy route, simply define the symbols as using
			 * an invalid stack address which makes them illegal for use
			 * as jump targets, but still possible for use as exception
			 * handler begin/end addresses. */
			guard[i].b->as_stck = ASM_SYM_STCK_INVALID;
			guard[i].e->as_stck = ASM_SYM_STCK_INVALID;
#else
			guard[i].b->as_stck = current_assembler.a_stackcur;
			guard[i].e->as_stck = current_assembler.a_stackcur;
			if (PUSH_RESULT)
				--guard[i].b->as_stck;
#endif
			guard[i].b->as_hand = current_assembler.a_handlerc;
			guard[i].e->as_hand = current_assembler.a_handlerc;
			guard[i].b->as_addr = guard_begin[i];
			guard[i].e->as_addr = guard_end[i];
			is_guarding         = true;
		}
	}

	/* Now that we know exactly what is being protected,
	 * let's generate the actual exception handlers. */
	end                         = (iter = try_ast->a_try.t_catchv) + try_ast->a_try.t_catchc;
	next_handler                = NULL;
	hand_frame.hf_prev          = current_assembler.a_handler;
	current_assembler.a_handler = &hand_frame;
	++current_assembler.a_handlerc;
	for (; iter != end; ++iter) {
#define IS_LAST_HANDLER() (iter == end - 1)
		struct asm_sym *handler_entry;
		struct asm_exc *descriptor;
		uint16_t handler_stack_exit = current_assembler.a_stackcur;
		uint16_t handler_stack      = handler_stack_exit;
		catch_mask_v                = &catch_mask;
		catch_mask_c                = 1;
		catch_mask                  = NULL;
		/* Keep track of what kind of handler this is. */
		hand_frame.hf_flags = iter->ce_flags;
		if (hand_frame.hf_flags & EXCEPTION_HANDLER_FFINALLY) {
			struct asm_sym *finally_exit = NULL;
			/* Finally handlers are written in-line directly after the protected code.
			 * Additionally, their stack alignment matches that of the guarded code,
			 * so that we can easily hide the result value of the protected part in the stack. */
			if (next_handler) {
				handler_entry = next_handler, next_handler = NULL;
			} else if unlikely((handler_entry = asm_newsym()) == NULL) {
				goto err_hand_frame;
			}
			if (guard_finflags & ASM_FINFLAG_USED) {
				if unlikely((finally_exit = asm_newsym()) == NULL)
					goto err_hand_frame;
				/* Push the stack & address where finally is meant to return to normally. */
				if unlikely(asm_gpush_abs(finally_exit))
					goto err_hand_frame;
				if unlikely(asm_gpush_stk(finally_exit))
					goto err_hand_frame;
				handler_stack += 2;
			}
			if (my_first_finally) {
				asm_defsym(my_first_finally);
				my_first_finally = NULL;
			}
			/* Note how we didn't switch sections. - Finally handlers are written in-line. */
			asm_defsym(handler_entry);
			/* If this is the first finally, define the entry label as such. */
			if (iter->ce_mask && /* We never defined what should happen to a mask here.
			                      * So to not cause any problems and stick to the documentation
			                      * stating that this code is executed before the handler itself,
			                      * simple evaluate it as an expression and move on. */
			    ast_genasm(iter->ce_mask, ASM_G_FNORMAL))
				goto err_hand_frame;
			if (ast_genasm(iter->ce_code, ASM_G_FNORMAL))
				goto err_hand_frame;
			if (is_guarding) {
				struct handler_frame *catch_iter;
				uint16_t num_catch = 0;
				catch_iter         = current_assembler.a_handler;
				for (; catch_iter; catch_iter = catch_iter->hf_prev) {
					if (!(catch_iter->hf_flags & EXCEPTION_HANDLER_FFINALLY))
						++num_catch;
				}
				if (asm_gendfinally_n(num_catch))
					goto err_hand_frame;
			}
			if (guard_finflags & ASM_FINFLAG_USED) {
				/* Search for the next finally-handler. */
				struct catch_expr *iter2     = iter + 1;
				struct asm_sym *next_finally = existing_finally;
				for (; iter2 != end; ++iter2) {
					if (!(iter2->ce_flags & EXCEPTION_HANDLER_FFINALLY))
						continue;
					next_finally = asm_newsym();
					if unlikely(!next_finally)
						goto err_hand_frame;
					/* Define as the entry point of the next finally-block. */
					my_first_finally = next_finally;
					break;
				}
				if (next_finally) {
					/* This is where it gets a bit convoluted,
					 * because we need to check something at runtime:
					 * We already know that the finally-handler was not executed
					 * following a return instruction (because in that case
					 * execution would not have passed `asm_gendfinally'), but
					 * what we don't know is how the finally block was entered.
					 * Was it:
					 *   - Entered through normal code flow
					 *   - Or entered because a loop control expression, or goto-statement
					 *     was executed that needed to jump across the finally handler.
					 * Considering the fact that we know of more finally handlers
					 * that need to be executed before the loop control expression
					 * can be served, we must either jump to the next finally-handler
					 * in case we got here because of a loop-control expression, or
					 * we must continue execution after the finally block itself, in
					 * case it was entered through normal means:
					 * >> for (;;) {
					 * >>     try {
					 * >>         try {
					 * >>             if (should_break())
					 * >>                 break; // push addrof(Loop_end); jmp addrof(Inner_finally)
					 * >>             
					 * >>             print "Entering inner finally normally";
					 * >>         } finally { // push addrof(Leaving_inner_finally);
					 * >>             print "Inner_finally";
					 * >>             // This is where we are right now.
					 * >>             //   - If we got here from `break', we must hold
					 * >>             //     off from jumping to `Loop_end' because of
					 * >>             //     the outer finally block, but we can not
					 * >>             //     blindly jump there all the time, because
					 * >>             //     then we'd always skip `Leaving_inner_finally'
					 * >>             //   - To fix this, we must compare the address that is
					 * >>             //     currently located ontop of the stack and contains the
					 * >>             //     finally-return-address (either `Leaving_inner_finally' or `Loop_end'),
					 * >>             //     and only jump to `Outter_finally' when it isn't equal
					 * >>             //     to `Leaving_inner_finally'.
					 * >>             // ASM:
					 * >>             //  >> # TOP == finally_return_address
					 * >>             //  >>     dup
					 * >>             //  >>     push $addrof(Leaving_inner_finally)
					 * >>             //  >>     cmp  eq
					 * >>             //  >>     jf   addrof(Outter_finally)
					 * >>             //  >>     jmp  pop
					 * >>             // HINT: `Leaving_inner_finally' is named `finally_exit'
					 * >>             // HINT: `Outter_finally' is named `next_finally'
					 * >>         }
					 * >>         print "Leaving_inner_finally";
					 * >>         print "Entering outer finally normally";
					 * >>     } finally { // push addrof(Loop_last)
					 * >>         print "Outter_finally";
					 * >>         // jmp pop (Can be served blindly in this case)
					 * >>     }
					 * >>     print "Loop_last";
					 * >> }
					 * >> print "Loop_end";
					 */
					/* Generate the assembly documented above. */
					if (asm_gdup())
						goto err_hand_frame;
					if (asm_gpush_abs(finally_exit))
						goto err_hand_frame;
					if (asm_gcmp_eq())
						goto err_hand_frame;
					/* TODO: Must clean up catch-handlers between here and the next finally! */
					if (asm_gjmp(ASM_JF, next_finally))
						goto err_hand_frame;
					asm_decsp(); /* Popped by `ASM_JF' */
				}
				ASSERT((current_assembler.a_flag & ASM_FSTACKDISP) ||
				       (current_assembler.a_stackcur == handler_stack));
				/* Ensure that the stack is properly adjusted to where
				 * the handler return address and stack depth are stored.
				 * This must be done because in STACKDISP mode, the stack
				 * may need to be re-aligned after stack variables were
				 * initialized. */
				if (asm_gsetstack(handler_stack))
					goto err_hand_frame;
				/* When there are no further finally-blocks left,
				 * simply pop the loop-exit address and jump there. */
				if (asm_gjmp_pop_pop())
					goto err_hand_frame;
			}
			/* Define the address that is pushed to cause the
			 * `jmp pop' above to become a no-op when the handler
			 * is entered through regular code-flow. */
			if (finally_exit)
				asm_defsym(finally_exit);
			/* Set the FFINALLY flag in the resulting code object, thus
			 * ensuring that finally-blocks are executed after return. */
			if (is_guarding)
				current_basescope->bs_flags |= CODE_FFINALLY;
		} else {
			struct asm_sec *prev_section;
			struct asm_sym *jump_across = NULL;
			code_addr_t cleanup_begin[SECTION_TEXTCOUNT];
			code_addr_t cleanup_end[SECTION_TEXTCOUNT];
			bool needs_cleanup, is_empty_handler;
			struct {
				struct asm_sym *b, *e;
			} cleanup[SECTION_TEXTCOUNT];
			/* Don't generate catch-blocks if they're not guarding anything.
			 * This can happen when the guarded code is a no-op that might
			 * have been optimized away during the AST optimization pass. */
			if (!is_guarding)
				continue;
			/* Switch the current section to the cold one.
			 * >> Exception handlers are assumed to be executed only
			 *    rarely, so instead of generating jumps around them
			 *    in regular text, we later put them at the end, so-as
			 *    to prevent them from influencing code performance
			 *    all-together! */
			prev_section             = current_assembler.a_curr;
			current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
			if (prev_section == current_assembler.a_curr) {
				jump_across = asm_newsym();
				if unlikely(!jump_across)
					goto err_hand_frame;
				if (asm_gjmp(ASM_JMP, jump_across))
					goto err_hand_frame;
			}
			/* Allow catch-handlers to define their own return value. */
			if (gflags & ASM_G_FPUSHRES) {
				--handler_stack;
				asm_decsp();
			}

			/* This is where the handler's entry point is located at! */
			if (next_handler)
				handler_entry = next_handler, next_handler = NULL;
			else if unlikely((handler_entry = asm_newsym()) == NULL)
			goto err_hand_frame;
			asm_defsym(handler_entry);

			/* Must include the catch-mask expression in the cleanup guard! */
			needs_cleanup = false;
			for (i = 0; i < SECTION_TEXTCOUNT; ++i)
				cleanup_begin[i] = asm_secip(i);
			if (iter->ce_mask) {
				struct ast *mask_ast = iter->ce_mask;
				struct asm_sym *enter_handler;
				/* Deal with an explicit exception handling mask.
				 * NOTE: The runtime has special hooks in place to quickly deal
				 *       with a mask that was already known at compile-time.
				 *       Though when the catch-expression wasn't known then,
				 *       we must generate some work-around code:
				 *       >> try {
				 *       >>     ....
				 *       >> } catch (get_mask()) {
				 *       >>     print "Handler";
				 *       >> }
				 *       Compile as:
				 *       >> try {
				 *       >>     ....
				 *       >> } catch (<except>...) {
				 *       >>     if (<except> !is get_mask()) {
				 *       >>         if (<is_last_handler>) {
				 *       >>             throw;
				 *       >>         } else {
				 *       >>             <goto_next_handler>
				 *       >>         }
				 *       >>     }
				 *       >>     print "Handler";
				 *       >> }
				 */
handle_mask_ast:
				if (mask_ast->a_type == AST_CONSTEXPR &&
				    DeeType_Check(mask_ast->a_constexpr) &&
				    asm_allowconst(mask_ast->a_constexpr)) {
					catch_mask = (DREF DeeTypeObject *)mask_ast->a_constexpr;
					Dee_Incref(catch_mask);
				} else if (mask_ast->a_type == AST_CONSTEXPR &&
				           DeeTuple_Check(mask_ast->a_constexpr)) {
					/* More than one exception mask. */
					DeeObject **maskv;
					maskv         = DeeTuple_ELEM(mask_ast->a_constexpr);
					catch_mask_c  = DeeTuple_SIZE(mask_ast->a_constexpr);
					enter_handler = NULL;
					if unlikely(!catch_mask_c) /* No mask? */
					catch_mask_v = NULL;
					else {
						size_t catch_mask_i;
						catch_mask_v = (DREF DeeTypeObject **)Dee_Calloc(catch_mask_c *
						                                                 sizeof(DREF DeeTypeObject *));
						if unlikely(!catch_mask_v) {
							catch_mask_c = 0;
							goto err_hand_frame;
						}
						for (catch_mask_i = 0; catch_mask_i < catch_mask_c; ++catch_mask_i) {
							DeeObject *mask_object;
							mask_object = maskv[catch_mask_i];
							ASSERT_OBJECT(mask_object);
							if (DeeType_Check(mask_object) &&
							    asm_allowconst(mask_object)) {
								catch_mask_v[catch_mask_i] = (DREF DeeTypeObject *)mask_object;
								Dee_Incref(catch_mask_v[catch_mask_i]);
							} else {
								/* Runtime-evaluated sub-mask. */
								if (!enter_handler && (enter_handler = asm_newsym()) == NULL)
									goto err_hand_frame;
								/* Generate code of this mask. */
								if (asm_gpush_except())
									goto err_hand_frame; /* except */
								if (asm_gpush_constexpr(mask_object))
									goto err_hand_frame; /* except, mask */
								if (asm_ginstanceof())
									goto err_hand_frame; /* except instanceof mask */
								/* Jump to the handler entry point if the mask matches. */
								if (asm_gjmp(ASM_JT, enter_handler))
									goto err_hand_frame;
								asm_decsp(); /* Popped by `ASM_JF' */
							}
						}
					}
					goto do_multimask_rethrow;
				} else if (mask_ast->a_type == AST_MULTIPLE &&
				           mask_ast->a_flag != AST_FMULTIPLE_KEEPLAST) {
					/* More than one exception mask. */
					struct ast **maskv;
					enter_handler = NULL;
					maskv         = mask_ast->a_multiple.m_astv;
					if (mask_ast->a_multiple.m_astc == 1) {
						/* Special handling when only a single type is being masked. */
						mask_ast = maskv[0];
						goto handle_mask_ast;
					}
					catch_mask_c = mask_ast->a_multiple.m_astc;
					if unlikely(!catch_mask_c) /* No mask? */
					catch_mask_v = NULL;
					else {
						size_t catch_mask_i;
						catch_mask_v = (DREF DeeTypeObject **)Dee_Calloc(catch_mask_c *
						                                                 sizeof(DREF DeeTypeObject *));
						if unlikely(!catch_mask_v) {
							catch_mask_c = 0;
							goto err_hand_frame;
						}
						for (catch_mask_i = 0; catch_mask_i < catch_mask_c; ++catch_mask_i) {
							mask_ast = maskv[catch_mask_i];
							ASSERT_AST(mask_ast);
							if (mask_ast->a_type == AST_CONSTEXPR &&
							    DeeType_Check(mask_ast->a_constexpr) &&
							    asm_allowconst(mask_ast->a_constexpr)) {
								catch_mask_v[catch_mask_i] = (DREF DeeTypeObject *)mask_ast->a_constexpr;
								Dee_Incref(catch_mask_v[catch_mask_i]);
							} else {
								/* Runtime-evaluated sub-mask. */
								if (!enter_handler && (enter_handler = asm_newsym()) == NULL)
									goto err_hand_frame;
								/* Generate code of this mask. */
								if (asm_gpush_except())
									goto err_hand_frame; /* except */
								if (ast_genasm_one(mask_ast, ASM_G_FPUSHRES))
									goto err_hand_frame; /* except, mask */
								if (asm_ginstanceof())
									goto err_hand_frame; /* except is mask */
								/* Jump to the handler entry point if the mask matches. */
								if (asm_gjmp(ASM_JT, enter_handler))
									goto err_hand_frame;
								asm_decsp(); /* Popped by `ASM_JF' */
							}
						}
					}
do_multimask_rethrow:
					if (enter_handler) {
						/* If the enter-handler symbol was allocated, that means
						 * that at least one of the catch-masks has to be evaluated
						 * at runtime, meaning we must generate a bit more code now. */
						if (!IS_LAST_HANDLER()) {
							next_handler = asm_newsym();
							if unlikely(!next_handler)
								goto err_hand_frame;
							if (asm_gjmp(ASM_JMP, next_handler))
								goto err_hand_frame;
						} else {
							if (asm_grethrow())
								goto err_hand_frame;
						}
						asm_defsym(enter_handler);
					}
				} else {
					/* Fallback: generate a check at runtime. */
					if (asm_gpush_except())
						goto err_hand_frame; /* except */
					if (ast_genasm_one(mask_ast, ASM_G_FPUSHRES))
						goto err_hand_frame; /* except, mask */
					if (asm_ginstanceof())
						goto err_hand_frame; /* except is mask */

					if (!IS_LAST_HANDLER()) {
						next_handler = asm_newsym();
						if unlikely(!next_handler)
							goto err_hand_frame;
						/* Jump to the next handler when it's not a match. */
						if (asm_gjmp(ASM_JF, next_handler))
							goto err_hand_frame;
						asm_decsp(); /* Popped by `ASM_JF' */
					} else {
						struct asm_sym *is_a_match;
						is_a_match = asm_newsym();
						if unlikely(!is_a_match)
							goto err_hand_frame;
						/* Execute the handler when it's a match. */
						if (asm_gjmp(ASM_JT, is_a_match))
							goto err_hand_frame;
						asm_decsp(); /* Popped by `ASM_JT' */

						/* Since there is no next handler, it's up to us to re-throw the exception. */
						if (asm_grethrow())
							goto err_hand_frame;
						/* This is where our match-check jumps to when there was no match. */
						asm_defsym(is_a_match);
					}
				}
			}
			/* Allow the handler to save a result onto the stack. */
			if (ast_genasm(iter->ce_code, gflags))
				goto err_hand_frame;
			for (i = 0; i < SECTION_TEXTCOUNT; ++i)
				cleanup_end[i] = asm_secip(i);
			/* If the FSECOND flag is set, the caller is OK with secondary
			 * exceptions not being discarded in the event of a new primary
			 * exception. */
			if (!(iter->ce_mode & CATCH_EXPR_FSECOND)) {
				/* Must generate cleanup code to use always make use of the primary exception. */
				/* Check which sections need to be protected by cleanup code. */
				bzero(cleanup, sizeof(cleanup));
				for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
					if (cleanup_begin[i] == cleanup_end[i])
						continue;
					needs_cleanup = true;
					cleanup[i].b  = asm_newsym();
					if unlikely(!cleanup[i].b)
						goto err_hand_frame;
					cleanup[i].e = asm_newsym();
					if unlikely(!cleanup[i].e)
						goto err_hand_frame;
					cleanup[i].b->as_stck = ASM_SYM_STCK_INVALID;
					cleanup[i].e->as_stck = ASM_SYM_STCK_INVALID;
					cleanup[i].b->as_sect = i;
					cleanup[i].e->as_sect = i;
					cleanup[i].b->as_hand = current_assembler.a_handlerc;
					cleanup[i].e->as_hand = current_assembler.a_handlerc;
					cleanup[i].b->as_addr = cleanup_begin[i];
					cleanup[i].e->as_addr = cleanup_end[i];
				}
			}
			/* Check if the handler is a so-called ~empty~ handler, that is a
			 * handler that would not contain any code other than `throw except'
			 * to re-throw the last error. (or rather continue handling it)
			 * Such a handler can be optimized by making use of the
			 * `EXCEPTION_HANDLER_FHANDLED' flag to let the runtime handle
			 * its associated exception before using the handler's guard
			 * end address as its entry point, continuing execution after
			 * the error has been discarded.
			 * XXX: Why don't we just set the flag for any handler that doesn't
			 *      make use of the active exception? Then we wouldn't have to
			 *      generate another cleanup handler if the handler itself causes
			 *      another error to be thrown?
			 *      For this purpose, we should add a new __asm__-clobber
			 *      field `"except"', that should be specified to disable
			 *      this optimization when user-assembly appears in exception
			 *      handlers.
			 *      Other than that: Check if the handler contains any use
			 *      of `SYMBOL_TYPE_EXCEPT' symbols that don't originate from
			 *      other catch-handlers that may be reachable from inside.
			 */
			is_empty_handler = !needs_cleanup && !(gflags & ASM_G_FPUSHRES);
			if (is_empty_handler) {
				for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
					if (cleanup_begin[i] == cleanup_end[i])
						continue;
					is_empty_handler = false;
					break;
				}
			}
			if (is_empty_handler) {
				/* Cheat a bit by re-defining the handle-entry
				 * symbol to point to its coverage exit address.
				 * This is to optimize empty catch-handlers:
				 * >> try {
				 * >>     return get_value_1();
				 * >> } catch (...) {
				 * >> }
				 * >> return get_value_2();
				 * ASM:
				 * >>.begin except_1
				 * >>    call global @get_value_1, #0
				 * >>    ret  pop
				 * >>.end except_1
				 * >>.entry except_1, @except, @handled
				 * >>    call global @get_value_2, #0
				 * >>    ret  pop
				 */
				handler_entry->as_sect = (uint16_t)(prev_section - current_assembler.a_sect);
				handler_entry->as_addr = (code_addr_t)(prev_section->sec_iter - prev_section->sec_begin);
				/* Use the FHANDLED flag to discard the error,
				 * rather than generating text to do the same. */
				hand_frame.hf_flags |= EXCEPTION_HANDLER_FHANDLED;
			} else {
				/* Before regular exit from a catch-block, handle the current exception. */
				if (asm_gendcatch())
					goto err_hand_frame;
				if (!IS_LAST_HANDLER() || current_assembler.a_curr != prev_section) {
					/* Just generate a jmp after all catch handlers. */
					if (!after_catch && (after_catch = asm_newsym()) == NULL)
						goto err_hand_frame;
					if (asm_gjmp(ASM_JMP, after_catch))
						goto err_hand_frame;
				}
				/* If there are more handlers and this one just pushed its result,
				 * then we must re-adjust the stack to discard the value. */
				if ((gflags & ASM_G_FPUSHRES) && !IS_LAST_HANDLER())
					asm_decsp();
				if (needs_cleanup) {
					/* So do _do_ need to generate cleanup code! */
					struct asm_sym *cleanup_entry;
					if (IS_LAST_HANDLER() && current_assembler.a_curr == prev_section) {
						/* But generate code to jump across the cleanup text. */
						if (!jump_across && (jump_across = asm_newsym()) == NULL)
							goto err_hand_frame;
						if (asm_gjmp(ASM_JMP, jump_across))
							goto err_hand_frame;
					}
					/* Determine the distance to the highest-order exception handler. */
					cleanup_entry = asm_newsym();
					if unlikely(!cleanup_entry)
						goto err_hand_frame;

					/* Create an exception descriptor for the cleanup. */
					for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
						if (!cleanup[i].b)
							continue;
						/* NOTE: Increase the priority for following handlers, so that
						 *       cleanup handlers are considered after the real handlers. */
						descriptor = asm_newexc_at(except_index++);
						if unlikely(!descriptor)
							goto err_hand_frame;
						/* Track usage of exception handler symbols. */
						++cleanup[i].b->as_used;
						++cleanup[i].e->as_used;
						++cleanup_entry->as_used;
						descriptor->ex_mask  = NULL;
						descriptor->ex_start = cleanup[i].b;
						descriptor->ex_end   = cleanup[i].e;
						descriptor->ex_addr  = cleanup_entry;
#if 0
						descriptor->ex_stack = handler_stack;
#endif
						descriptor->ex_flags = EXCEPTION_HANDLER_FNORMAL;
					}

					/* This is the cleanup code. */
					asm_defsym(cleanup_entry);
					if (asm_gendcatch_n(1))
						goto err_hand_frame; /* Discard the first secondary exception */
					if (asm_grethrow())
						goto err_hand_frame;
				}
			}
			if (jump_across)
				asm_defsym(jump_across);
			current_assembler.a_curr = prev_section;
		}

		/* Get rid of duplicate exception masks that can occur when multi-catch expressions are used. */
		if (catch_mask_c > 1) {
			size_t mask_i, mask_j;
			for (mask_i = 0; mask_i < catch_mask_c - 1; ++mask_i) {
				DeeTypeObject *my_mask = catch_mask_v[mask_i];
				for (mask_j = mask_i + 1; mask_j < catch_mask_c;) {
					if (catch_mask_v[mask_j] == my_mask) {
						/* Get rid of this mask (which is already in use) */
						--catch_mask_c;
						memmovedownc(catch_mask_v + mask_i,
						             catch_mask_v + mask_i + 1,
						             (catch_mask_c - mask_i),
						             sizeof(DREF DeeTypeObject *));
						Dee_XDecref(my_mask);
						continue;
					}
					++mask_j;
				}
			}
		}

		/* Create an exception descriptor for every affected section. */
		for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
			size_t mask_i;
			if (!guard[i].b)
				continue;
			/* Create new descriptors at the effective priority index.
			 * Since we're not incrementing `except_index' following this,
			 * the effective priority remains the same, meaning that later
			 * exception handlers have a lower priority than previous ones,
			 * just as is intended by the deemon specs. */
			for (mask_i = 0; mask_i < catch_mask_c; ++mask_i) {
				descriptor = asm_newexc_at(except_index);
				if unlikely(!descriptor)
					goto err_hand_frame;
				descriptor->ex_mask = catch_mask_v[mask_i];
				Dee_XIncref(catch_mask_v[mask_i]);
				/* Track usage of exception handler symbols. */
				++guard[i].b->as_used;
				++guard[i].e->as_used;
				++handler_entry->as_used;
				descriptor->ex_start = guard[i].b;
				descriptor->ex_end   = guard[i].e;
				descriptor->ex_addr  = handler_entry;
#if 0
				/* No special magic here. - Simply use the stack depth as it is after the guarded expression.
				 * With that in mind, the result value of a catch()-expression is the same object that the
				 * runtime uses when aligning the stack. */
				descriptor->ex_stack = handler_stack;
#endif
				descriptor->ex_flags = hand_frame.hf_flags;
			}
		}

		/* Reset the stack depth to its original value.
		 * NOTE: The current value may be distorted and invalid due to
		 *       stack variables having been initialized inside the loop. */
		ASSERT(current_assembler.a_flag & ASM_FSTACKDISP ||
		       current_assembler.a_stackcur == handler_stack_exit);
		current_assembler.a_stackcur = handler_stack_exit;

		/* Drop the reference to a compile-time catch mask. */
		if (catch_mask_v != &catch_mask) {
			while (catch_mask_c--)
				Dee_XDecref(catch_mask_v[catch_mask_c]);
			Dee_Free(catch_mask_v);
		}
		Dee_XDecref(catch_mask);
#undef IS_LAST_HANDLER
	}
	current_assembler.a_handler = hand_frame.hf_prev;
	--current_assembler.a_handlerc;
	/* This is where catch-handles jump to. */
	if (after_catch)
		asm_defsym(after_catch);
	return 0;
err_hand_frame:
	if (catch_mask_v != &catch_mask) {
		while (catch_mask_c--)
			Dee_Decref(catch_mask_v[catch_mask_c]);
		Dee_Free(catch_mask_v);
	}
	Dee_XDecref(catch_mask);
	current_assembler.a_handler = hand_frame.hf_prev;
	--current_assembler.a_handlerc;
	goto err;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENTRY_C */
