/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_ASM_H
#define GUARD_DEEMON_ASM_H 1

/* NOTE: Apply modifications using:
 * $ deemon -F src/deemon/compiler/instrlen.c
 * $ deemon -F src/deemon/execute/code-exec-targets.c.inl
 * $ deemon -F src/deemon/execute/asm/exec-386.S
 * $ deemon -F src/dex/disassembler/printinstr.c
 * $ deemon -F lib/rt/bytecode.dee
 * Or the 1-line variant (read for copy-n-pasting into your terminal):
 * $ deemon -F src/deemon/compiler/instrlen.c src/deemon/execute/code-exec-targets.c.inl src/deemon/execute/asm/exec-386.S src/dex/disassembler/printinstr.c lib/rt/bytecode.dee
 * Also: Don't forget to add new instructions to
 *      `/src/deemon/compiler/asm/userdb.def'
 */


/* Deemon assembly definitions and instruction codes.
 * Starting in v200, deemon uses an entirely new assembly design that is
 * no longer restricted by rules on the length of individual instructions.
 * Instead, a CISC-style instruction set is used that allows for variable-length
 * opcodes, as well as prefixes and immediate operands. */

/* NOTE: All immediate values are encoded in little-endian.
 * NOTE: Unless otherwise documented, multiple pop-s in
 *       a single instruction always happen right-to-left.
 * NOTE: Unless otherwise documented, immediate operands
 *       are encoded left-to-right.
 * Additional checks performed when `CODE_FASSEMBLY' is set:
 *  >> Push when the stack is already full:
 *       If `CODE_FLENIENT' is set, extend the stack.
 *       If the stack has grown too large, throw an `Error.RuntimeError.SegFault'
 *       NOTE: We do not throw an `Error.RuntimeError.StackOverflow' to prevent
 *             confusion with recursing functions.
 *  >> Pop/dup/etc. when the stack is empty or not large enough:
 *       throw an `Error.RuntimeError.SegFault'
 *  >> Attempting to access an out-of-bounds variable:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to access an out-of-bounds constant:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to access an out-of-bounds argument:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to jump outside the active code object:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to yield from a non-yielding function:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to execute `ASM_PUSH_THIS' without `CODE_FTHISCALL' being set:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to use `ASM_CALL_TUPLE' (& related instruction) without a Tuple:
 *       throw an `Error.TypeError'
 *  >> Attempting to use `ASM_PUSH_EXCEPT' when no exception has been thrown:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to execute an undefined instruction:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 *  >> Attempting to do a fast attribute lookup (`ASM_GETATTR_C', etc.) using a constant that isn't a string:
 *       throw an `Error.TypeError'
 *  >> Attempting to handle an exception thrown by a caller:
 *       throw an `Error.RuntimeError.IllegalInstruction'
 * 
 * Additional changes in behavior when `CODE_FASSEMBLY' is set:
 *  >> `ASM_PUSH_CONST' and `ASM_PUSH_CONST16' behave identical to
 *     `ASM_PUSH_STATIC' and `ASM_PUSH_STATIC16' in that they always
 *     acquire the static variable lock before accessing the static
 *     variable vector.
 *     Similarly, all other instructions that use constant also acquire
 *     this lock (i.e.: `ASM_GETITEM_C', `ASM_GETATTR_C', etc...)
 */

/* Interpreter registers:
 *   - pointer REG_PC;              // Instruction pointer
 *   - pointer REG_SP;              // Stack pointer (Describes a potentially infinite amount of memory); may be limited by code requirements.
 *   - pointer REG_START_PC;        // Instruction pointer directed at the start of the current instruction.
 *   - Object  REG_RESULT;          // Result value
 *   - bool    REG_RESULT_ITERDONE; // When true, the function shall return to indicate iterator exhaustion. Only available in yielding function.
 *   - Integer REG_EXCEPTION_START; // Value of `thread->t_exceptsz' when the frame got created
 *   - Object  REG_LOCALS[];        // A variable number of object slots for local variables, limited by code requirements.
 *   - Object  REG_CONSTANTS[];     // A variable number of constant object slots, stored alongside code. Also contained are static variables.
 *   - Object  REG_REFS[];          // A variable number of object slots for referenced variables, limited by code requirements.
 *   - Object  REG_ARGS[];          // A variable number of object slots for function arguments, limited by code requirements.
 *   - Object  REG_GLOBALS[];       // A variable number of object slots for global variables, limited by module requirements.
 *   - Object  REG_EXTERN[][];      // A variable number of object slots, accessible through a variable number of module slots.
 *
 * Interpreter constants:
 *   - pointer MAX_VALID_INSTRUCTION_INDEX; // Greatest valid program counter (program size - 1)
 *   - pointer MIN_VALID_STACK_POINTER;     // Lowest valid stack pointer (GET_SP_BASE() + 0)
 *   - pointer MAX_VALID_STACK_POINTER;     // Greatest valid stack pointer (GET_SP_BASE() + (STACK_SIZE - 1) * STACK_ENTRY_SIZE)
 *   - Integer CODE_FLAGS;                  // Code flags (set of `CODE_F*')
 *   - bool    IS_CODE_YIELDING = CODE_FLAGS & CODE_FYIELDING;
 *
 * Frame setup:
 * >>     IF GET_THREADLOCAL_FRAME_COUNT() >= MAX_FRAME_COUNT THEN
 * >>         THROW(StackOverflow());
 * >>         PROPAGATE_EXCEPTIONS_TO_CALLER();
 * >>     FI
 * >>     INCREMENT_THREADLOCAL_FRAME_COUNT();
 * >>     REG_PC              = GET_IP_BASE() + 0;
 * >>     REG_SP              = GET_SP_BASE() + 0;
 * >>     REG_RESULT          = UNBOUND;
 * >>     REG_RESULT_ITERDONE = FALSE;
 * >>     REG_EXCEPTION_START = thread->t_exceptsz;
 *
 * Interpretation loop:
 * >>     WHILE TRUE DO
 * >> DISPATCH:
 * >>         REG_START_PC = REG_PC;
 * >>         REG_PC, OPCODE = DECODE_INSTRUCTION(WITH pc = REG_PC)...;
 * >>         EXECUTE_INSTRUCTION(WITH instr = OPCODE);
 * >>     DONE
 *
 * Frame cleanup:
 * >> RETURN:
 * >>     // Serve finally handles affecting the last-executed instruction.
 * >>     IF HAS_FINALLY_HANDLERS(REG_START_PC) THEN
 * >>         EXECUTE_FINALLY_HANDLERS();
 * >>     FI
 * >>     GOTO RETURN_WITHOUT_FINALLY;
 * >>
 * >> RETURN_WITHOUT_FINALLY:
 * >>     DECREMENT_THREADLOCAL_FRAME_COUNT();
 * >>     
 * >>     // Deal with dangling exceptions
 * >>     IF thread->t_exceptsz > REG_EXCEPTION_START THEN
 * >>         IF IS_BOUND(REG_RESULT) THEN
 * >>             REG_RESULT = UNBOUND;
 * >>         FI
 * >>         WHILE thread->t_exceptsz > REG_EXCEPTION_START + 1 DO
 * >>             PRINT_EXCEPT(WITH reason = "Discarding secondary error\n");
 * >>             POP_EXCEPT(WITH handle_interrupt = FALSE); // i.e. interrupts are re-scheduled
 * >>         DONE
 * >>         PROPAGATE_EXCEPTIONS_TO_CALLER();
 * >>     FI
 * >>     
 * >>     // NOTE: Implementations may implement some other means of indicating iter-done.
 * >>     //       For example, Griefer@Work's deemon returns `ITER_DONE' from `DeeObject_IterNext()'
 * >>     IF REG_RESULT_ITERDONE == true THEN
 * >>         THROW(StopIteration());
 * >>         PROPAGATE_EXCEPTIONS_TO_CALLER();
 * >>     FI
 * >>     PROPAGATE_RESULT_TO_CALLER(REG_RESULT);
 *
 * Misc. documentation functions:
 * >> EXCEPT:
 * >>     IF HAS_EXCEPTION_HANDLERS(REG_START_PC) THEN
 * >>         LOAD_LAST_EXCEPTION_HANDLER();
 * >>         GOTO DISPATCH;
 * >>     FI
 * >>     PROPAGATE_EXCEPTIONS_TO_CALLER();
 * >>
 * >>
 * >> void THROW(Object obj);
 * >>
 * >> // Causes undefined behavior in fast-mode code.
 * >> // Else, throws an exception `obj' in safe-mode code
 * >> void THROW_OR_UNDEFINED_BEHAVIOR(Object obj);
 * >>
 * >> // Access to stack items.
 * >> void PUSH(Object obj) BEGIN
 * >>     IF REG_SP >= MAX_VALID_STACK_POINTER THEN
 * >>         IF CODE_FLAGS & CODE_FLENIENT THEN
 * >>             EXTEND_STACK();
 * >>         ELSE
 * >>             THROW_OR_UNDEFINED_BEHAVIOR(SegFault());
 * >>         FI
 * >>     FI
 * >>     *REG_SP = obj;
 * >>     REG_SP += 1;
 * >> END
 * >>
 * >> Object POP() BEGIN
 * >>     IF REG_SP <= MIN_VALID_STACK_POINTER THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(SegFault());
 * >>     FI
 * >>     REG_SP -= 1;
 * >>     RETURN *REG_SP;
 * >> END
 * >>
 * >> Tuple POP(Integer num_items) BEGIN
 * >>     local result = List();
 * >>     for (none: [:num_items])
 * >>         result.insert(0, POP());
 * >>     RETURN Tuple(result);
 * >> END
 * >>
 * >> Object &NTH(Integer nth_item) BEGIN
 * >>     RETURN *(REG_SP - (nth_item + 1));
 * >> END
 * >>
 * >> Object &EXTERN(Integer module_index, Integer symbol_index) BEGIN
 * >>     IF module_index >= LENGTH(REG_EXTERN) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     IF symbol_index >= LENGTH(REG_EXTERN[module_index]) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     RETURN REG_EXTERN[module_index][symbol_index];
 * >> END
 * >>
 * >> Object &GLOBAL(Integer symbol_index) BEGIN
 * >>     IF symbol_index >= LENGTH(REG_GLOBALS) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     RETURN REG_GLOBALS[symbol_index];
 * >> END
 * >>
 * >> Object &LOCAL(Integer local_index) BEGIN
 * >>     IF symbol_index >= LENGTH(REG_LOCALS) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     RETURN REG_LOCALS[symbol_index];
 * >> END
 * >>
 * >> Object &STATIC(Integer static_index) BEGIN
 * >>     IF symbol_index >= LENGTH(REG_CONSTANTS) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     RETURN REG_CONSTANTS[symbol_index];
 * >> END
 * >>
 * >> Object  CONSTANT(Integer static_index) BEGIN
 * >>     RETURN STATIC(symbol_index); // Constant and static symbols use the same indices
 * >> END
 * >>
 * >> Object MODULE(Integer module_index) BEGIN
 * >>     IF module_index >= LENGTH(REG_EXTERN) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     RETURN REG_EXTERN[module_index];
 * >> END
 * >>
 * >> Object REF(Integer reference_index) BEGIN
 * >>     IF reference_index >= LENGTH(REG_REFS) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     RETURN REG_REFS[reference_index];
 * >> END
 * >>
 * >> Object ARG(Integer argument_index) BEGIN
 * >>     IF argument_index >= LENGTH(REG_ARGS) THEN
 * >>         THROW_OR_UNDEFINED_BEHAVIOR(IllegalInstruction());
 * >>     FI
 * >>     RETURN REG_ARGS[argument_index];
 * >> END
 * >>
 * >> Object INVOKE_OPERATOR(Integer operator_id, Object self, Object args...) BEGIN
 * >>     // `operator_id' is one of `OPERATOR_*'
 * >>     RETURN DeeObject_InvokeOperator(self, operator_id, #args, [args...]);
 * >> END
 * >>
 * >> Object GET_PREFIX_OBJECT() BEGIN
 * >>     // Return the `Object &' referenced by the instruction prefix (s.a. `ASM_ISPREFIX()')
 * >>     ...
 * >> END
 * >>
 * >> void SET_PREFIX_OBJECT(Object obj) BEGIN
 * >>     // Set the `Object &' referenced by the instruction prefix (s.a. `ASM_ISPREFIX()')
 * >>     ...
 * >> END
 * >>
 * >> Object &TOP    = NTH(0);
 * >> Object &FIRST  = NTH(0);
 * >> Object &SECOND = NTH(1);
 * >> Object &THIRD  = NTH(2);
 * >> Object &FOURTH = NTH(3);
 * >>
 * >> // Check if a given Object @ob is bound.
 * >> bool IS_BOUND(Object &obj) BEGIN
 * >>     RETURN obj != UNBOUND;
 * >> END
 * >>
 * >> // When `handle_interrupt' is FALSE, `@[interrupt]'
 * >> // exceptions are re-scheduled as pending interrupts
 * >> void    POP_EXCEPT(bool handle_interrupt);
 * >> void    PRINT_EXCEPT(string reason);
 * >>
 * >> // Propagate active exceptions to the caller after ensuring that no
 * >> // more than a single exception has been set since `REG_EXCEPTION_START'
 * >> NORETURN void PROPAGATE_EXCEPTIONS_TO_CALLER();
 */

/* Functional programming/exception-related instructions. */
#define ASM_RET_NONE          0x00 /* [1][-0,+0]   `ret'                                - Return `none' to the caller. This instruction does not return.
                                    *                                                     NOTE: In a yield-function, this instruction does not
                                    *                                                           return none, but signals `ITER_DONE' to the caller.
                                    *                                                           In documentation this is often referenced as throwing
                                    *                                                           a StopIteration exception, however an implementation
                                    *                                                           is allowed to use a custom signaling mechanism for this,
                                    *                                                           and is not required to translate `StopIteration' exceptions
                                    *                                                           into that same signal.
                                    *                                                           This is also the case for Griefer@Work's implementation.
                                    * >> IF IS_BOUND(REG_RESULT) THEN
                                    * >>     REG_RESULT = UNBOUND;
                                    * >> FI
                                    * >> IF IS_CODE_YIELDING THEN
                                    * >>     REG_PC              = REG_START_PC;
                                    * >>     REG_RESULT_ITERDONE = TRUE;
                                    * >> ELSE
                                    * >>     REG_RESULT          = none;
                                    * >> FI
                                    * >> GOTO RETURN; */
#define ASM_RET               0x01 /* [1][-1,+0]   `ret pop'                            - Return an Object to the caller. This instruction does not return.
                                    * [1][-0,+0]   `ret PREFIX'                         - `PREFIX: ret'
                                    * >> IF IS_BOUND(REG_RESULT) THEN
                                    * >>     REG_RESULT = UNBOUND;
                                    * >> FI
                                    * >> REG_RESULT = POP();
                                    * >> IF IS_CODE_YIELDING THEN
                                    * >>     GOTO RETURN_WITHOUT_FINALLY;
                                    * >> ELSE
                                    * >>     GOTO RETURN;
                                    * >> FI */
#define ASM_YIELD             0x01 /* [1][-1,+0]   `yield pop'                          - Pop one Object and yield it to the caller.
                                    * [1][-0,+0]   `yield PREFIX'                       - `PREFIX: yield'
                                    *                                                     This instruction may not return, and if it does, it doesn't immediately.
                                    *                                                     NOTE: Same opcode as `ASM_RET_POP'. - Behavior is selected by `CODE_FYIELDING' */
#define ASM_YIELDALL          0x02 /* [1][-1,+0]   `yield foreach, pop'                 - Pop one Object and iterate it, yielding all contained objects, one at a time.
                                    * [1][-0,+0]   `yield foreach, PREFIX'              - `PREFIX: yield foreach'
                                    *                                                     NOTE: Only available in code of yield functions.
                                    *                                                     HINT: During execution, this opcode will replace stack-top with its own iterator and
                                    *                                                           keep resetting `REG_PC' to re-execute itself until the iterator is finished, at which
                                    *                                                           point it will finally pop one value and resume execution at the next instruction.
                                    *                                                     WARNING: This instruction takes an iterator, not a sequence.
                                    * >> IF EXEC_SAFE && !IS_CODE_YIELDING THEN
                                    * >>     THROW(IllegalInstruction());
                                    * >> FI
                                    * >> IF IS_BOUND(REG_RESULT) THEN
                                    * >>     REG_RESULT = UNBOUND;
                                    * >> FI
                                    * >> TRY
                                    * >>    REG_RESULT = DeeObject_IterNext(TOP);
                                    * >> CATCH (StopIteration)
                                    * >>    POP();
                                    * >>    GOTO DISPATCH;
                                    * >> DONE
                                    * >> REG_PC = REG_START_PC;
                                    * >> GOTO RETURN_WITHOUT_FINALLY; */
#define ASM_THROW             0x03 /* [1][-1,+0]   `throw pop'                          - Pop one Object and throw it as an exception. This instruction does not return.
                                    * [1][-0,+0]   `throw PREFIX'                       - `PREFIX: throw'
                                    * >> THROW(POP());
                                    * >> EXCEPT(); */
#define ASM_RETHROW           0x04 /* [1][-0,+0]   `throw except'                       - Rethrow the current exception (Used to terminate catch-blocks, and generated by `throw' without an argument).
                                    * >> EXCEPT(); */
#define ASM_SETRET            0x05 /* [1][-1,+0]   `setret pop'                         - Set the contents of the return register.
                                    * [1][-0,+0]   `setret PREFIX'                      - `PREFIX: setret'
                                    * >> IF IS_BOUND(REG_RESULT) THEN
                                    * >>     REG_RESULT = UNBOUND;
                                    * >> FI
                                    * >> REG_RESULT = POP();
                                    * The intended use is when a return statement is used that is
                                    * guarded by a finally handler, in which case the result register
                                    * can simply be assigned before jumping into the finally handler
                                    * and proceeding to execute it, before `end finally' is encountered
                                    * at its end, which will check for the result register to be bound,
                                    * and return if it is. */
#define ASM_ENDCATCH          0x06 /* [1][-0,+0]   `end catch'                          - Unconditionally handle the current exception (Usually used when returning from catch-blocks).
                                    *                                                     If no exceptions are left to handle (or all that are left were thrown by a caller), this instruction acts as a no-op.
                                    * >> IF thread->t_exceptsz > REG_EXCEPTION_START THEN
                                    * >>     POP_EXCEPT(WITH handle_interrupt = TRUE);
                                    * >> FI */
#define ASM_ENDFINALLY        0x07 /* [1][-0,+0]   `end finally'                        - When running a finally-block after return, continue executing of the next surrounding finally-block, or actually return to the caller.
                                    *                                                     If this instruction is executed before a return instruction has been encountered (aka. when no return value has been set), it acts as a no-op.
                                    *                                                     If unhandled exceptions exist when this instruction is being executed, exception handling will recommence.
                                    *                                                     In order to support recursive catch/finally handling, an additional overload exists that accepts the min. number of errors
                                    *                                                     that must have been thrown since the start of the function in order for the runtime to rethrow the last of them.
                                    * >> IF IS_BOUND(REG_RESULT) THEN
                                    * >>     RETURN();
                                    * >> FI
                                    * >> IF (THREAD->t_exceptsz - REG_EXCEPTION_START) > 0 THEN
                                    * >>     EXCEPT();
                                    * >> FI */
#define ASM_CALL_KW           0x08 /* [3][-1-n,+1] `call top, #<imm8>, const <imm8>'    - Similar to `ASM_CALL', but also pass a keywords mapping from `<imm8>' */
#define ASM_CALL_TUPLE_KW     0x09 /* [2][-2,+1]   `call top, pop..., const <imm8>'     - Similar to `ASM_CALL_TUPLE', but also pass a keywords mapping from `<imm8>' */
/*      ASM_                  0x0a  *               --------                            - ------------------ */
/*      ASM_                  0x0b  *               --------                            - ------------------ */
#define ASM_PUSH_BND_ARG      0x0c /* [2][-0,+1]   `push bound arg <imm8>'              - Check if the argument variable indexed by `<imm8>' is bound, pushing true/false indicative of that state.
                                    * WARNING: This mnemonic only looks at positional and keyword arguments
                                    *          passed by the caller, but doesn't account for default arguments.
                                    * >> PUSH(bool(IS_BOUND(ARG(IMM8)))); */
#define ASM_PUSH_BND_EXTERN   0x0d /* [3][-0,+1]   `push bound extern <imm8>:<imm8>'    - Check if the extern variable indexed by `<imm8>:<imm8>' is bound, pushing true/false indicative of that state.
                                    * >> PUSH(bool(IS_BOUND(EXTERN(IMM8,IMM8)))); */
#define ASM_PUSH_BND_GLOBAL   0x0e /* [2][-0,+1]   `push bound global <imm8>'           - Check if the global variable indexed by `<imm8>' is bound, pushing true/false indicative of that state.
                                    * >> PUSH(bool(IS_BOUND(GLOBAL(IMM8)))); */
#define ASM_PUSH_BND_LOCAL    0x0f /* [2][-0,+1]   `push bound local <imm8>'            - Check if the local variable indexed by `<imm8>' is bound, pushing true/false indicative of that state.
                                    * >> PUSH(bool(IS_BOUND(LOCAL(IMM8)))); */

/* Control-flow-related instructions. */
#define ASM_JF                0x10 /* [2][-1,+0]   `jf pop, <Sdisp8>'                   - Pop one Object. If it evaluates to `false', add `<Sdisp8>' to the `REG_PC' of the next instruction.
                                    * [2][-0,+0]   `jf PREFIX, <Sdisp8>'                - `PREFIX: jf <Sdisp8>'
                                    * NOTE: This instruction unconditionally invokes the `bool' operator. */
#define ASM_JF16              0x11 /* [3][-1,+0]   `jf pop, <Sdisp16>'                  - Pop one Object. If it evaluates to `false', add `<Sdisp16>' (little endian) to the `REG_PC' of the next instruction.
                                    * [3][-0,+0]   `jf PREFIX, <Sdisp16>'               - `PREFIX: jf <Sdisp16>'
                                    * NOTE: This instruction unconditionally invokes the `bool' operator.
                                    * >> IF !POP() THEN REG_PC += SDISP16; FI */
#define ASM_JT                0x12 /* [2][-1,+0]   `jt pop, <Sdisp8>'                   - Pop one Object. If it evaluates to `true', add `<Sdisp8>' to the `REG_PC' of the next instruction.
                                    * [2][-0,+0]   `jt PREFIX, <Sdisp8>'                - `PREFIX: jt <Sdisp8>'
                                    * NOTE: This instruction unconditionally invokes the `bool' operator. */
#define ASM_JT16              0x13 /* [3][-1,+0]   `jt pop, <Sdisp16>'                  - Pop one Object. If it evaluates to `true', add `<Sdisp16>' (little endian) to the `REG_PC' of the next instruction.
                                    * [3][-0,+0]   `jt PREFIX, <Sdisp16>'               - `PREFIX: jt <Sdisp16>'
                                    * NOTE: This instruction unconditionally invokes the `bool' operator.
                                    * >> IF POP() THEN REG_PC += SDISP16; FI */
#define ASM_JMP               0x14 /* [2][-0,+0]   `jmp <Sdisp8>'                       - Unconditionally add `<Sdisp8>' to the `REG_PC' of the next instruction. */
#define ASM_JMP16             0x15 /* [3][-0,+0]   `jmp <Sdisp16>'                      - Unconditionally add `<Sdisp16>' (little endian) to the `REG_PC' of the next instruction.
                                    * >> REG_PC += SDISP16; */
#define ASM_FOREACH           0x16 /* [2][-1,+2|0] `foreach top, <Sdisp8>'              - Invoke the __iternext__ operator on stack-top, popping the iterator and performing a `jmp' using <Sdisp8> when it is empty, or leaving the iterator and pushing the loaded value onto the stack otherwise.
                                    * [2][-0,+1|0] `foreach PREFIX, <Sdisp8>'           - `PREFIX: foreach <Sdisp8>' */
#define ASM_FOREACH16         0x17 /* [3][-1,+2|0] `foreach top, <Sdisp16>'             - Invoke the __iternext__ operator on stack-top, popping the iterator and performing a `jmp' using <Sdisp16> when it is empty, or leaving the iterator and pushing the loaded value onto the stack otherwise.
                                    * [3][-0,+1|0] `foreach PREFIX, <Sdisp16>'          - `PREFIX: foreach <Sdisp16>' 
                                    * >> ob: Object = TOP.operator next();
                                    * >> IF IS_BOUND(ob) THEN
                                    * >>     PUSH(ob);
                                    * >> ELSE
                                    * >>     POP();
                                    * >>     REG_PC += Sdisp16;
                                    * >> FI */
#define ASM_JMP_POP           0x18 /* [1][-1,+0]   `jmp pop'                            - Pop one value from the stack and convert it to an int, using that value as new instruction pointer. NOTE: In safe-exec code, an out-of-bounds REG_PC causes an `Error.RuntimeError.SegFault'
                                    * >> int new_ip = int(POP());
                                    * >> IF new_ip < 0 || new_ip > MAX_VALID_INSTRUCTION_INDEX THEN
                                    * >>     THROW_OR_UNDEFINED_BEHAVIOR(Error.RuntimeError.SegFault());
                                    * >>     EXCEPT();
                                    * >> FI
                                    * >> REG_PC = new_ip; */
#define ASM_OPERATOR          0x19 /* [3][-1-n,+1] `op top, $<imm8>, #<imm8>'           - Invoke an operator `$<imm8>' on `top' after popping `#<imm8>' values used as arguments.
                                    * [3][-n,+1]   `PREFIX: push op $<imm8>, #<imm8>'
                                    * >> int id = IMM8;
                                    * >> Object args = POP(IMM8);
                                    * >> IF CURRENT_INSTRUCTION_HAS_PREFIX THEN
                                    * >>     Object obj = GET_PREFIX_OBJECT();
                                    * >>     obj = INVOKE_OPERATOR(id, obj, args...);
                                    * >>     SET_PREFIX_OBJECT(obj);
                                    * >> ELSE
                                    * >>     Object obj = POP();
                                    * >>     PUSH(INVOKE_OPERATOR(id, obj, args...));
                                    * >> FI */
#define ASM_OPERATOR_TUPLE    0x1a /* [2][-2,+1]   `op top, $<imm8>, pop...'            - Similar to `ASM_CALL_OP', but directly pop the argument Tuple.
                                    * [2][-1,+1]   `PREFIX: push op $<imm8>, pop...'
                                    * >> Object args = POP();
                                    * >> IF args !is Tuple THEN
                                    * >>     THROW_OR_UNDEFINED_BEHAVIOR(Error.TypeError());
                                    * >>     EXCEPT();
                                    * >> FI
                                    * >> IF CURRENT_INSTRUCTION_HAS_PREFIX THEN
                                    * >>     Object obj = GET_PREFIX_OBJECT();
                                    * >>     obj = INVOKE_OPERATOR(IMM8, obj, args...);
                                    * >>     SET_PREFIX_OBJECT(obj);
                                    * >> ELSE
                                    * >>     Object obj = POP();
                                    * >>     PUSH(INVOKE_OPERATOR(IMM8, obj, args...));
                                    * >> FI */
#define ASM_CALL              0x1b /* [2][-1-n,+1] `call top, #<imm8>'                  - Pop <imm8> values and pack then into a Tuple then used to call a function popped thereafter. - Push the result onto the stack.
                                    * >> Object args = POP(IMM8);
                                    * >> Object func = POP();
                                    * >> PUSH(func(args...)); */
#define ASM_CALL_TUPLE        0x1c /* [1][-2,+1]   `call top, pop...'                   - Similar to `ASM_CALL', but call the function using a Tuple found in stack-top. NOTE: Undefined behavior ensues if something other than a Tuple is found.
                                    * >> Object args = POP();
                                    * >> Object func = POP();
                                    * >> IF args !is Tuple THEN
                                    * >>     THROW_OR_UNDEFINED_BEHAVIOR(Error.TypeError());
                                    * >>     EXCEPT();
                                    * >> FI
                                    * >> PUSH(func(args...)); */
/*      ASM_                  0x1d  *               --------                            - ------------------ */
#define ASM_DEL_GLOBAL        0x1e /* [2][-0,+0]   `del global <imm8>'                  - Unlink the global variable indexed by `<imm8>'. Throws an `UnboundLocal' error if the variable wasn't assigned to begin with.
                                    * >> GLOBAL(IMM8) = UNBOUND; */
#define ASM_DEL_LOCAL         0x1f /* [2][-0,+0]   `del local <imm8>'                   - Unlink the local variable indexed by `<imm8>'. Throws an `UnboundLocal' error if the variable wasn't assigned to begin with.
                                    * >> LOCAL(IMM8) = UNBOUND; */

/* Stack control instructions. */
#define ASM_SWAP              0x20 /* [1][-2,+2]   `swap'                               - Swap the 2 top-most stack entries.
                                    * [1][-1,+1]   `swap top, PREFIX', `swap PREFIX, top' - `PREFIX: swap top'
                                    * >> ob: Object = TOP;
                                    * >> TOP = SECOND;
                                    * >> SECOND = ob; */
#define ASM_LROT              0x21 /* [2][-n,+n]   `lrot #<imm8>+3'                     - Rotate the top <imm8>+3 stack objects left by one; away from SP.
                                    * [2][-n,+n]   `lrot #<imm8>+2, PREFIX'             - `PREFIX: lrot #<imm8>+3'
                                    * NOTE: `lrot #2' should be encoded as `swap', `lrot #1' should be discarded, `lrot #0' is illegal.
                                    * HINT: The operand details the number of affected stack slots.
                                    * >> push $5  // 5
                                    * >> push $10 // 5, 10
                                    * >> push $20 // 5, 10, 20
                                    * >> push $30 // 5, 10, 20, 30
                                    * >>                 |  /   /
                                    * >>                 +-/---/-+
                                    * >>                  /   /  |
                                    * >> lrot #3  // 5, 20, 30, 10 // The top 3 entries were rotated left once
                                    * >>              \   \   \  |
                                    * >>             +-\---\---\-+
                                    * >>             |  \   \   \
                                    * >> rrot #4  // 10, 5, 20, 30 // The reverse of `lrot' */
#define ASM_RROT              0x22 /* [2][-n,+n]   `rrot #<imm8>+3'                     - Rotate the top <imm8>+3 stack objects right by one; towards SP.
                                    * [2][-n,+n]   `rrot #<imm8>+2, PREFIX'             - `PREFIX: rrot #<imm8>+3'
                                    * NOTE: `rrot #2' should be encoded as `swap', `rrot #1' should be discarded, `rrot #0' is illegal.
                                    * HINT: The operand details the number of affected stack slots. */
#define ASM_DUP               0x23 /* [1][-1,+2]   `dup', `dup #SP - 1', `push top'     - Duplicate the top-most stack Object.
                                    *              `mov PREFIX, top', `mov PREFIX, #SP - 1'
                                    * >> TARGET = TOP; */
#define ASM_DUP_N             0x24 /* [2][-n,+n+1] `dup #SP - <imm8> - 2', `push #SP - <imm8> - 2' - Push the <imm8>+2'th stack entry (before adjustment).
                                    *              `mov PREFIX, #SP - <imm8> - 2'
                                    *                                                     HINT: The operand details the number of slots passed before encountering the one that should be copied.
                                    * >> push $5      // 5
                                    * >> push $10     // 5,  10
                                    * >> push $20     // 5,  10, 20
                                    * >> push $30     // 5,  10, 20, 30
                                    * >> dup  #SP - 2 // 5,  10, 20, 30, 20
                                    * >> dup  #SP - 4 // 5,  10, 20, 30, 20, 10
                                    * >> pop  #SP - 6 // 10, 10, 20, 30, 20
                                    * >> pop  #SP - 4 // 10, 20, 20, 30
                                    * >> TARGET = NTH(IMM8 + 2); */
#define ASM_POP               0x25 /* [1][-1,+0]   `pop', `pop top', `pop #SP - 1'      - Pop and discard the top-most stack entry.
                                    * [2][-0,+0]   `mov top, PREFIX'                    - `PREFIX: pop'
                                    * >> POP(); */
#define ASM_POP_N             0x26 /* [2][-n-1,+n] `pop #SP - <imm8> - 2',              - Pop one stack and overwrite the `#<imm8>+1'th stack entry (before adjustment) with the value.
                                    * [2][-0,+0]   `mov #SP - <imm8> - 2, PREFIX'       - `PREFIX: pop #SP - <imm8> - 2'
                                    * >> Object &dst = NTH(IMM8 + 2);
                                    * >> dst = POP(); */
#define ASM_ADJSTACK          0x27 /* [2][-?,+?]   `adjstack #SP +/- <Simm8>'           - Adjust the stack-pointer by adding <Simm8> to it, popping values when negative, or pushing `none' when positive.
                                    *                                                     NOTE: The user-code assembler, always knowing the proper stack-depth,
                                    *                                                           interprets the operand as an absolute stack address, meaning
                                    *                                                           that while the data-stream encoding is SP-relative, the assembler
                                    *                                                           itself encodes the operand as SP-absolute, similar to how the
                                    *                                                           operands of ASM_JMP and friends are written absolute, despite
                                    *                                                           actually being absolute.
                                    *                                                     NOTE: Additionally, the user-code assembler will encode the following operands specially:
                                    *                                                         - `adjstack #SP + 1' --> `push none'
                                    *                                                         - `adjstack #SP + 0' --> `---'
                                    *                                                         - `adjstack #SP - 1' --> `pop'
                                    * >> int offset = SIMM8;
                                    * >> WHILE offset > 0 DO
                                    * >>     PUSH(none);
                                    * >>     offset = offset - 1;
                                    * >> DONE
                                    * >> WHILE offset < 0 DO
                                    * >>     POP();
                                    * >>     offset = offset + 1;
                                    * >> DONE */
#define ASM_SUPER             0x28 /* [1][-2,+1]   `super top, pop'                     - Create a new super-wrapper using a type from `pop' and the associated Object from `top'.
                                    * >> typ = POP();
                                    * >> PUSH(POP() as typ); */
#define ASM_SUPER_THIS_R      0x29 /* [2][-0,+1]   `push super this, ref <imm8>'        - Similar to `ASM_SUPER', but use a referenced variable `<imm8>' as type and the this-argument as Object.
                                    * >> PUSH(THIS as REF(IMM8)); */
/*      ASM_                  0x2a  *               --------                            - ------------------ */
/*      ASM_                  0x2b  *               --------                            - ------------------ */
#define ASM_POP_STATIC        0x2c /* [2][-1,+0]   `pop static <imm8>'                  - Pop the top stack value into the static variable indexed by `<imm8>', overwriting the previous value.
                                    * [2][-0,+0]   `mov static <imm8>, PREFIX'          - `PREFIX: pop static <imm8>'
                                    * >> STATIC(IMM8) = SOURCE; */
#define ASM_POP_EXTERN        0x2d /* [3][-1,+0]   `pop extern <imm8>:<imm8>'           - Pop the top stack value into the extern variable indexed by `<imm8>:<imm8>', overwriting the previous value.
                                    * [3][-0,+0]   `mov extern <imm8>:<imm8>, PREFIX'   - `PREFIX: pop extern <imm8>:<imm8>'
                                    * >> EXTERN(IMM8,IMM8) = SOURCE; */
#define ASM_POP_GLOBAL        0x2e /* [2][-1,+0]   `pop global <imm8>'                  - Pop the top stack value into the global variable indexed by `<imm8>', overwriting the previous value.
                                    * [2][-0,+0]   `mov global <imm8>, PREFIX'          - `PREFIX: pop global <imm8>'
                                    * >> GLOBAL(IMM8) = SOURCE; */
#define ASM_POP_LOCAL         0x2f /* [2][-1,+0]   `pop local <imm8>'                   - Pop the top stack value into the local variable indexed by `<imm8>', overwriting the previous value.
                                    * [2][-0,+0]   `mov local <imm8>, PREFIX'           - `PREFIX: pop local <imm8>'
                                    * >> LOCAL(IMM8) = SOURCE; */

/* Push builtin constant expressions, immediate values or local/static values. */

/* TODO: Add an option to the assembler to automatically unwind finally-blocks
 *       when a return statement is encountered inside of one.
 *       Related to this, change the `CODE_FFINALLY' flag to not be mandatory
 *       when there are finally-blocks defined, but _ONLY_ enable the automatic
 *       unwinding of them as part of returning from a function.
 *       That way we can get rid of the slow O(N) cleanup of finally-handlers,
 *       while still maintaining the option of having the runtime deal with them
 *       in smaller functions! */
/*      ASM_                  0x30  *               --------                            - ------------------ */
/*      ASM_                  0x31  *               --------                            - ------------------ */
/*      ASM_                  0x32  *               --------                            - ------------------ */
#define ASM_PUSH_NONE         0x33 /* [1][-0,+1]   `push none'                          - Push `none' onto the stack.
                                    * [1][-0,+0]   `mov  PREFIX, none'                  - `PREFIX: push none'
                                    * >> DESTINATION = none; */
/*      ASM_                  0x34  *               --------                            - ------------------ */
/*      ASM_                  0x35  *               --------                            - ------------------ */
#define ASM_PUSH_VARARGS      0x36 /* [1][-0,+1]   `push varargs'                       - Push variable arguments. (Illegal instruction if the code doesn't have the `CODE_FVARARGS' flag set)
                                    * [1][-0,+0]   `mov  PREFIX, varargs'               - `PREFIX: push varargs'
                                    * >> DESTINATION = VARARGS; */
#define ASM_PUSH_VARKWDS      0x37 /* [1][-0,+1]   `push varkwds'                       - Push variable keyword arguments. (Illegal instruction if the code doesn't have the `CODE_FVARKWDS' flag set)
                                    * [1][-0,+0]   `mov  PREFIX, varkwds'               - `PREFIX: push varkwds'
                                    * >> DESTINATION = VARKWDS; */
#define ASM_PUSH_MODULE       0x38 /* [2][-0,+1]   `push module <imm8>'                 - Push an imported module indexed by <imm8> from the current module.
                                    * [2][-0,+0]   `mov  PREFIX, module <imm8>'         - `PREFIX: push module <imm8>'
                                    * >> DESTINATION = MODULE(IMM8); */
#define ASM_PUSH_ARG          0x39 /* [2][-0,+1]   `push arg <imm8>'                    - Push the argument indexed by `<imm8>'.
                                    * [2][-0,+0]   `mov  PREFIX, arg <imm8>'            - `PREFIX: push arg <imm8>'
                                    * >> DESTINATION = ARG(IMM8); */
#define ASM_PUSH_CONST        0x3a /* [2][-0,+1]   `push const <imm8>'                  - Same as `ASM_PUSH_STATIC', but doesn't require a lock as the slot should acts as an immutable constant.
                                    * [2][-0,+0]   `mov  PREFIX, const <imm8>'          - `PREFIX: push const <imm8>'
                                    * >> DESTINATION = CONSTANT(IMM8); */
#define ASM_PUSH_REF          0x3b /* [2][-0,+1]   `push ref <imm8>'                    - Push a referenced variable indexed by `<imm8>'.
                                    * [2][-0,+0]   `mov  PREFIX, ref <imm8>'            - `PREFIX: push ref <imm8>'
                                    * >> DESTINATION = REF(IMM8); */
#define ASM_PUSH_STATIC       0x3c /* [2][-0,+1]   `push static <imm8>'                 - Push the static variable indexed by `<imm8>'.
                                    * [2][-0,+0]   `mov  PREFIX, static <imm8>'         - `PREFIX: push static <imm8>'
                                    * >> DESTINATION = STATIC(IMM8); */
#define ASM_PUSH_EXTERN       0x3d /* [3][-0,+1]   `push extern <imm8>:<imm8>'          - Push the extern variable indexed by `<imm8>:<imm8>'. If it isn't bound, throw an `Error.RuntimeError.UnboundLocal'
                                    * [3][-0,+0]   `mov  PREFIX, extern <imm8>'         - `PREFIX: push extern <imm8>'
                                    * >> DESTINATION = EXTERN(IMM8,IMM8); */
#define ASM_PUSH_GLOBAL       0x3e /* [2][-0,+1]   `push global <imm8>'                 - Push the global variable indexed by `<imm8>'. If it isn't bound, throw an `Error.RuntimeError.UnboundLocal'
                                    * [2][-0,+0]   `mov  PREFIX, global <imm8>'         - `PREFIX: push global <imm8>'
                                    * >> DESTINATION = GLOBAL(IMM8); */
#define ASM_PUSH_LOCAL        0x3f /* [2][-0,+1]   `push local <imm8>'                  - Push the local variable indexed by `<imm8>'. If it isn't bound, throw an `Error.RuntimeError.UnboundLocal'
                                    * [2][-0,+0]   `mov  PREFIX, local <imm8>'          - `PREFIX: push local <imm8>'
                                    * >> DESTINATION = LOCAL(IMM8); */

/* Sequence control instructions. */
#define ASM_CAST_TUPLE        0x40 /* [1][-1,+1]   `cast top, Tuple'                    - Convert a sequence in stack-top into a Tuple.
                                    * >> PUSH(Tuple(POP())); */
#define ASM_CAST_LIST         0x41 /* [1][-1,+1]   `cast top, List'                     - Convert a sequence in stack-top into a List.
                                    * >> PUSH(List(POP())); */
#define ASM_PACK_TUPLE        0x42 /* [2][-n,+1]   `push pack Tuple, #<imm8>'           - Pop <imm8> elements and pack them into a Tuple.
                                    * >> PUSH(Tuple(POP(IMM8))); */
#define ASM_PACK_LIST         0x43 /* [2][-n,+1]   `push pack List, #<imm8>'            - Pop <imm8> elements and pack them into a List.
                                    * >> PUSH(List(POP(IMM8))); */
/*      ASM_                  0x44  *               --------                            - ------------------ */
/*      ASM_                  0x45  *               --------                            - ------------------ */
#define ASM_UNPACK            0x46 /* [2][-1,+n]   `unpack pop, #<imm8>'                - Pop a sequence and unpack it into <imm8> elements then pushed onto the stack.
                                    *              `unpack PREFIX, #<imm8>', `PREFIX: unpack #<imm8>'
                                    * >> int n = IMM8;
                                    * >> Object seq = SOURCE;
                                    * >> FOR Object item IN seq DO
                                    * >>     IF N == 0 THEN
                                    * >>         THROW(Error.ValueError.UnpackError());
                                    * >>         EXCEPT();
                                    * >>     FI
                                    * >>     N = N - 1;
                                    * >>     PUSH(item);
                                    * >> DONE
                                    * >> IF N != 0 THEN
                                    * >>     THROW(Error.ValueError.UnpackError());
                                    * >>     EXCEPT();
                                    * >> FI */
#define ASM_CONCAT            0x47 /* [1][-2,+1]   `concat top, pop'                    - Concat a generic sequence in stack-top with a sequence below that, replacing it with the result (The original left sequence is not modified!).
                                    * >> PUSH(POP() + POP());  // For sequences (Same as ASM_ADD, but the left-sequence (top) may be modified in-place) */
#define ASM_EXTEND            0x48 /* [2][-n-1,+1] `extend top, #<imm8>'                - Same as `pack Tuple, #<imm8>; concat top, pop;'.
                                    * >> TOP.extend(POP(IMM8)); */
#define ASM_TYPEOF            0x49 /* [1][-1,+1]   `typeof top'                         - Replace the top stack-entry with its own class.
                                    * >> PUSH(type POP()); */
#define ASM_CLASSOF           0x4a /* [1][-1,+1]   `classof top'                        - Replace the top stack-entry with its own type.
                                    * >> PUSH(POP().class); */
#define ASM_SUPEROF           0x4b /* [1][-1,+1]   `superof top'                        - Replace the top stack-entry with a wrapper for the associated super-Object.
                                    * >> PUSH(POP().super); */
#define ASM_INSTANCEOF        0x4c /* [1][-2,+1]   `instanceof top, pop'                - Pop one Object (type), then another (Object) and check if `Object' is an instance of `type', pushing `true' or `false' indicative of this.
                                    * >> Object tp = POP();
                                    * >> IF tp === none THEN
                                    * >>     PUSH(POP() === none);
                                    * >> ELSE
                                    * >>     PUSH(tp.baseof(type POP()));
                                    * >> FI */
#define ASM_STR               0x4d /* [1][-1,+1]   `str top'                            - Apply the __str__ operator to the top-most stack entry.
                                    * >> PUSH(str POP()); */
#define ASM_REPR              0x4e /* [1][-1,+1]   `repr top'                           - Apply the __repr__ operator to the top-most stack entry.
                                    * >> PUSH(repr POP()); */
/*      ASM_                  0x4f  *               --------                            - ------------------ */
#define ASM_BOOL              0x50 /* [1][-1,+1]   `bool top' - `cast top, bool'        - Apply the __bool__ operator to the top-most stack entry and store the result as a bool.
                                    * >> PUSH(!!POP()); */
#define ASM_NOT               0x51 /* [1][-1,+1]   `not top'                            - Apply the __bool__ operator to the top-most stack entry and store the result as an inverted bool.
                                    * >> PUSH(!POP()); */
#define ASM_ASSIGN            0x52 /* [1][-2,+0]   `assign pop, pop'                    - Assign the value in stack-top to that below.
                                    * >> POP() := POP(); */
#define ASM_MOVE_ASSIGN       0x53 /* [1][-2,+0]   `assign move, pop, pop'              - Move-assign the value in stack-top to that below.
                                    * >> POP().operator move := (POP()); */
#define ASM_COPY              0x54 /* [1][-1,+1]   `copy top'                           - Replace stack-top with a copy of itself.
                                    * >> PUSH(copy POP()); */
#define ASM_DEEPCOPY          0x55 /* [1][-1,+1]   `deepcopy top'                       - Replace stack-top with a deep-copy of itself.
                                    * >> PUSH(deepcopy POP()); */
#define ASM_GETATTR           0x56 /* [1][-2,+1]   `getattr top, pop'                   - Pop a string, then use it to lookup an attribute in stack-top. @throws: Error.TypeError: The attribute Object isn't a string.
                                    * >> PUSH(POP().operator . (POP())); */
#define ASM_DELATTR           0x57 /* [1][-2,+0]   `delattr pop, pop'                   - Pop a string, then use it to delete an attribute in stack-top. @throws: Error.TypeError: The attribute Object isn't a string.
                                    * >> POP().operator del. (POP()); */
#define ASM_SETATTR           0x58 /* [1][-3,+0]   `setattr pop, pop, pop'              - Pop a value and a string then use them to set an attribute in stack-top. @throws: Error.TypeError: The attribute Object isn't a string.
                                    * >> POP().operator . (POP(), POP()); */
#define ASM_BOUNDATTR         0x59 /* [1][-2,+1]   `boundattr top, pop'                 - Pop a string, then use it to check if an attribute is bound in stack-top. @throws: Error.TypeError: The attribute Object isn't a string.
                                    * >> PUSH(bound(POP().operator . (POP())); */
#define ASM_GETATTR_C         0x5a /* [2][-1,+1]   `getattr top, const <imm8>'          - Perform a fast attribute lookup on stack-top using a string in constant slot `<imm8>'.
                                    * >> PUSH(POP().operator . (CONST(IMM8))); */
#define ASM_DELATTR_C         0x5b /* [2][-1,+0]   `delattr pop, const <imm8>'          - Delete an attribute of stack-top named by a string in constant slot `<imm8>'.
                                    * >> POP().operator del. (CONST(IMM8)); */
#define ASM_SETATTR_C         0x5c /* [2][-2,+0]   `setattr pop, const <imm8>, pop'     - Pop a value and set an attribute of (then) stack-top named by a string in constant slot `<imm8>'.
                                    * >> POP().operator . (CONST(IMM8), POP()); */
#define ASM_GETATTR_THIS_C    0x5d /* [2][-0,+1]   `push getattr this, const <imm8>'    - Lookup and push an attribute of `this', using a string in constant slot `<imm8>' (Only valid for code with the `CODE_FTHISCALL' flag set)
                                    * >> PUSH(THIS.operator . (CONST(IMM8))); */
#define ASM_DELATTR_THIS_C    0x5e /* [2][-0,+0]   `delattr this, const <imm8>'         - Delete an attribute of `this', named by a string in constant slot `<imm8>' (Only valid for code with the `CODE_FTHISCALL' flag set)
                                    * >> THIS.operator del. (CONST(IMM8)); */
#define ASM_SETATTR_THIS_C    0x5f /* [2][-1,+0]   `setattr this, const <imm8>, pop'    - Pop a value and set an attribute of `this', named by a string in constant slot `<imm8>' (Only valid for code with the `CODE_FTHISCALL' flag set)
                                    * >> THIS.operator . (CONST(IMM8), POP()); */

/* Compare instructions. */
#define ASM_CMP_EQ            0x60 /* [1][-2,+1]   `cmp eq, top, pop'                   - Compare the two top-most objects on the stack and push the result.
                                    * >> PUSH(POP() == POP()); */
#define ASM_CMP_NE            0x61 /* [1][-2,+1]   `cmp ne, top, pop'                   - Compare the two top-most objects on the stack and push the result.
                                    * >> PUSH(POP() != POP()); */
#define ASM_CMP_LO            0x62 /* [1][-2,+1]   `cmp lo, top, pop'                   - Compare the two top-most objects on the stack and push the result.
                                    * >> PUSH(POP() < POP()); */
#define ASM_CMP_LE            0x63 /* [1][-2,+1]   `cmp le, top, pop'                   - Compare the two top-most objects on the stack and push the result.
                                    * >> PUSH(POP() <= POP()); */
#define ASM_CMP_GR            0x64 /* [1][-2,+1]   `cmp gr, top, pop'                   - Compare the two top-most objects on the stack and push the result.
                                    * >> PUSH(POP() > POP()); */
#define ASM_CMP_GE            0x65 /* [1][-2,+1]   `cmp ge, top, pop'                   - Compare the two top-most objects on the stack and push the result.
                                    * >> PUSH(POP() >= POP()); */
#define ASM_CLASS_C           0x66 /* [2][-1,+1]   `class top, const <imm8>'            - Construct a new class type, using `pop' as base, and `const <imm8>' as class's descriptor. */
#define ASM_CLASS_GC          0x67 /* [3][-0,+1]   `push class global <imm8>, const <imm8>' - Same as `ASM_CLASS_C', however use `global <imm8>' as base. */
#define ASM_CLASS_EC          0x68 /* [4][-0,+1]   `push class extern <imm8>:<imm8>, const <imm8>' - Same as `ASM_CLASS_C', however use `extern <imm8>:<imm8>' as base. */
#define ASM_DEFCMEMBER        0x69 /* [2][-2,+1]   `defcmember top, $<imm8>, pop'       - Initialize a class member variable `<imm8>', using a value from the stack. */
#define ASM_GETCMEMBER_R      0x6a /* [3][-0,+1]   `push getcmember ref <imm8>, $<imm8>'- Lookup a class member of `ref <imm8>', given its index. */
#define ASM_CALLCMEMBER_THIS_R 0x6b/* [4][-n,+1]   `push callcmember this, ref <imm8>, $<imm8>, #<imm8>'- Lookup a class member of `ref <imm8>', given its index, then call that attribute as a this-call by popping #<imm8> arguments. */
/*      ASM_                  0x6c  *               --------                            - ------------------ */
/*      ASM_                  0x6d  *               --------                            - ------------------ */
#define ASM_FUNCTION_C        0x6e /* [3][-n,+1]   `push function const <imm8>, #<imm8>+1' - Create a new function Object, using constant slot <imm8> as code and popping $<imm8>+1 objects for use as references.
                                    * [3][-n,+0]   `PREFIX: function const <imm8>, #<imm8>+1' */
#define ASM_FUNCTION_C_16     0x6f /* [4][-n,+1]   `push function const <imm8>, #<imm16>+1' - Create a new function Object, using constant slot <imm8> as code and popping $<imm16>+1 objects for use as references.
                                    * [4][-n,+0]   `PREFIX: function const <imm8>, #<imm16>+1' */

/* Arithmetic instructions (NOTE: Some of these may be prefixed to perform an inplace operation). */
#define ASM_CAST_INT          0x70 /* [1][-1,+1]   `cast top, int'                      - Convert the top-most stack entry into an integer. */
#define ASM_INV               0x71 /* [1][-1,+1]   `inv top'                            - Apply the __inv__ operator to the top-most stack Object. */
#define ASM_POS               0x72 /* [1][-1,+1]   `pos top'                            - Apply the __pos__ operator to the top-most stack Object. */
#define ASM_NEG               0x73 /* [1][-1,+1]   `neg top'                            - Apply the __neg__ operator to the top-most stack Object. */
#define ASM_ADD               0x74 /* [1][-2,+1]   `add top, pop'                       - Apply the __add__ operator to the top-most stack Object.
                                    * [1][-1,+0]   `add PREFIX, pop' - `PREFIX: add pop'- Perform an inplace __add__ operation on `prefix', using `pop' as second operand. */
#define ASM_SUB               0x75 /* [1][-2,+1]   `sub top, pop'                       - Apply the __sub__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `sub PREFIX, pop' - `PREFIX: sub pop'- Perform an inplace __sub__ operation on `prefix', using `pop' as second operand. */
#define ASM_MUL               0x76 /* [1][-2,+1]   `mul top, pop'                       - Apply the __mul__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `mul PREFIX, pop' - `PREFIX: mul pop'- Perform an inplace __mul__ operation on `prefix', using `pop' as second operand. */
#define ASM_DIV               0x77 /* [1][-2,+1]   `div top, pop'                       - Apply the __div__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `div PREFIX, pop' - `PREFIX: div pop'- Perform an inplace __div__ operation on `prefix', using `pop' as second operand. */
#define ASM_MOD               0x78 /* [1][-2,+1]   `mod top, pop'                       - Apply the __mod__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `mod PREFIX, pop' - `PREFIX: mod pop'- Perform an inplace __mod__ operation on `prefix', using `pop' as second operand. */
#define ASM_SHL               0x79 /* [1][-2,+1]   `shl top, pop'                       - Apply the __shl__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `shl PREFIX, pop' - `PREFIX: shl pop'- Perform an inplace __shl__ operation on `prefix', using `pop' as second operand. */
#define ASM_SHR               0x7a /* [1][-2,+1]   `shr top, pop'                       - Apply the __shr__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `shr PREFIX, pop' - `PREFIX: shr pop'- Perform an inplace __shr__ operation on `prefix', using `pop' as second operand. */
#define ASM_AND               0x7b /* [1][-2,+1]   `and top, pop'                       - Apply the __and__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `and PREFIX, pop' - `PREFIX: and pop'- Perform an inplace __and__ operation on `prefix', using `pop' as second operand. */
#define ASM_OR                0x7c /* [1][-2,+1]   `or top, pop'                        - Apply the __or__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `or PREFIX, pop'  - `PREFIX: or pop' - Perform an inplace __or__ operation on `prefix', using `pop' as second operand. */
#define ASM_XOR               0x7d /* [1][-2,+1]   `xor top, pop'                       - Apply the __xor__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `xor PREFIX, pop' - `PREFIX: xor pop'- Perform an inplace __xor__ operation on `prefix', using `pop' as second operand. */
#define ASM_POW               0x7e /* [1][-2,+1]   `pow top, pop'                       - Apply the __pow__ operator to the two top-most objects on the stack.
                                    * [1][-1,+0]   `pow PREFIX, pop' - `PREFIX: pow pop'- Perform an inplace __pow__ operation on `prefix', using `pop' as second operand. */
#define ASM_INC               0x7f /* [1][-0,+0]   `inc'                                - Increment a variable by one. WARNING: Only available as inplace op with storage prefix.
                                    * [1][-0,+0]   `inc PREFIX' - `PREFIX: inc'         - Increment the prefixed variable. */
#define ASM_DEC               0x80 /* [1][-0,+0]   `dec'                                - Decrement a variable by one. WARNING: Only available as inplace op with storage prefix.
                                    * [1][-0,+0]   `dec PREFIX' - `PREFIX: dec'         - Decrement the prefixed variable. */
#define ASM_ADD_SIMM8         0x81 /* [2][-1,+1]   `add top, $<Simm8>'                  - Same as `ASM_ADD', but use <Simm8> as second operand.
                                    * [2][-0,+0]   `add PREFIX, $<Simm8>'               - `PREFIX: add $<Simm8>' */
#define ASM_ADD_IMM32         0x82 /* [5][-1,+1]   `add top, $<imm32>'                  - Same as `ASM_ADD', but use <imm32> as second operand.
                                    * [5][-0,+0]   `add $<imm32>, PREFIX'               - `PREFIX: add $<Simm8>' */
#define ASM_SUB_SIMM8         0x83 /* [2][-1,+1]   `sub top, $<Simm8>'                  - Same as `ASM_SUB', but use <Simm8> as second operand.
                                    * [2][-0,+0]   `sub PREFIX, $<Simm8>'               - `PREFIX: sub $<Simm8>' */
#define ASM_SUB_IMM32         0x84 /* [5][-1,+1]   `sub top, $<imm32>'                  - Same as `ASM_SUB', but use <imm32> as second operand.
                                    * [5][-0,+0]   `sub $<imm32>, PREFIX'               - `PREFIX: sub $<Simm8>' */
#define ASM_MUL_SIMM8         0x85 /* [2][-1,+1]   `mul top, $<Simm8>'                  - Same as `ASM_MUL', but use <Simm8> as second operand.
                                    * [2][-0,+0]   `mul PREFIX, $<Simm8>'               - `PREFIX: mul $<Simm8>' */
#define ASM_DIV_SIMM8         0x86 /* [2][-1,+1]   `div top, $<Simm8>'                  - Same as `ASM_DIV', but use <Simm8> as second operand.
                                    * [2][-0,+0]   `div PREFIX, $<Simm8>'               - `PREFIX: div $<Simm8>' */
#define ASM_MOD_SIMM8         0x87 /* [2][-1,+1]   `mod top, $<Simm8>'                  - Same as `ASM_MOD', but use <Simm8> as second operand.
                                    * [2][-0,+0]   `mod PREFIX, $<Simm8>'               - `PREFIX: mod $<Simm8>' */
#define ASM_SHL_IMM8          0x88 /* [2][-1,+1]   `shl top, $<imm8>'                   - Same as `ASM_SHL', but use <imm8> as second operand.
                                    * [2][-0,+0]   `shl PREFIX, $<Simm8>'               - `PREFIX: shl $<Simm8>' */
#define ASM_SHR_IMM8          0x89 /* [2][-1,+1]   `shr top, $<imm8>'                   - Same as `ASM_SHR', but use <imm8> as second operand.
                                    * [2][-0,+0]   `shr PREFIX, $<Simm8>'               - `PREFIX: shr $<Simm8>' */
#define ASM_AND_IMM32         0x8a /* [5][-1,+1]   `and top, $<imm32>'                  - Same as `ASM_AND', but use <imm32> as second operand.
                                    * [5][-0,+0]   `and PREFIX, $<imm32>'               - `PREFIX: and $<Simm8>' */
#define ASM_OR_IMM32          0x8b /* [5][-1,+1]   `or top, $<imm32>'                   - Same as `ASM_OR', but use <imm32> as second operand.
                                    * [5][-0,+0]   `or PREFIX, $<imm32>'                - `PREFIX: or $<Simm8>' */
#define ASM_XOR_IMM32         0x8c /* [5][-1,+1]   `xor top, $<imm32>'                  - Same as `ASM_XOR', but use <imm32> as second operand.
                                    * [5][-0,+0]   `xor PREFIX, $<imm32>'               - `PREFIX: xor $<Simm8>' */
#define ASM_ISNONE            0x8d /* [1][-1,+1]   `instanceof top, none'               - Check if `top' is the `none' Object and push true/false indicative of this. */
/*      ASM_                  0x8e  *               --------                            - ------------------ */
#define ASM_DELOP             0x8f /* [1][-0,+0]    --------                            - Same as `nop', but without an assigned mnemonic.
                                    *                                                     This instruction is meant for the compiler to be used
                                    *                                                     for marking instructions that should be deleted during
                                    *                                                     later optimization passes.
                                    *                                                     Note that such passes are optional and when the runtime
                                    *                                                     encounters this instruction, it behaves identical to a `nop'. */

/* No-operation */
#define ASM_NOP               0x90 /* [1][-0,+0]   `nop'                                - Does nothing (intentionally)
                                    * [1][-0,+0]   `nop PREFIX'                         - `PREFIX: nop'
                                    * HINT: This instruction can be used with the 0F, as well as any storage prefix. */

/* Print instructions. */
#define ASM_PRINT             0x91 /* [1][-1,+0]   `print pop'                          - Pop one Object and print it to stdout. */
#define ASM_PRINT_SP          0x92 /* [1][-1,+0]   `print pop, sp'                      - Same as `ASM_PRINT', but follow up by printing a space character. */
#define ASM_PRINT_NL          0x93 /* [1][-1,+0]   `print pop, nl'                      - Same as `ASM_PRINT', but follow up by printing a new-line character. */
#define ASM_PRINTNL           0x94 /* [1][-0,+0]   `print nl'                           - Print a new-line character to stdout. */
#define ASM_PRINTALL          0x95 /* [1][-1,+0]   `print pop...'                       - Pop one Object and iterate it as a sequence, printing all elements separated by space characters to stdout. */
#define ASM_PRINTALL_SP       0x96 /* [1][-1,+0]   `print pop..., sp'                   - Same as `ASM_PRINTALL', but follow up by printing a space character. */
#define ASM_PRINTALL_NL       0x97 /* [1][-1,+0]   `print pop..., nl'                   - Same as `ASM_PRINTALL', but follow up by printing a new-line character. */
/*      ASM_                  0x98  *               --------                            - ------------------ */
#define ASM_FPRINT            0x99 /* [1][-2,+1]   `print top, pop'                     - Pop one Object and print it to a file in stack-top. */
#define ASM_FPRINT_SP         0x9a /* [1][-2,+1]   `print top, pop, sp'                 - Same as `ASM_FPRINT', but follow up by printing a space character. */
#define ASM_FPRINT_NL         0x9b /* [1][-2,+1]   `print top, pop, nl'                 - Same as `ASM_FPRINT', but follow up by printing a new-line character. */
#define ASM_FPRINTNL          0x9c /* [1][-1,+1]   `print top, nl'                      - Print a new-line character to a file in stack-top. */
#define ASM_FPRINTALL         0x9d /* [1][-2,+1]   `print top, pop...'                  - Pop one Object and iterate it as a sequence, printing all elements separated by space characters a file in stack-top. */
#define ASM_FPRINTALL_SP      0x9e /* [1][-2,+1]   `print top, pop..., sp'              - Same as `ASM_FPRINTALL', but follow up by printing a space character. */
#define ASM_FPRINTALL_NL      0x9f /* [1][-2,+1]   `print top, pop..., nl'              - Same as `ASM_FPRINTALL', but follow up by printing a new-line character. */
/*      ASM_                  0xa0  *               --------                            - ------------------ */
#define ASM_PRINT_C           0xa1 /* [2][-0,+0]   `print const <imm8>'                 - Print a constant from `<imm8>' to stdout. */
#define ASM_PRINT_C_SP        0xa2 /* [2][-0,+0]   `print const <imm8>, sp'             - Same as `ASM_PRINT_C', but follow up by printing a space character. */
#define ASM_PRINT_C_NL        0xa3 /* [2][-0,+0]   `print const <imm8>, nl'             - Same as `ASM_PRINT_C', but follow up by printing a new-line character. */
#define ASM_RANGE_0_I16       0xa4 /* [3][-0,+1]   `push range $0, $<imm16>'            - Create a new range from using `int(0)' as `begin' and `int(<imm16>)' as `end'. */
/*      ASM_                  0xa5  *               --------                            - ------------------ */
#define ASM_ENTER             0xa6 /* [1][-1,+1]   `enter top'                          - Invoke `operator enter()' on `top', but don't pop or replace it with another value. */
#define ASM_LEAVE             0xa7 /* [1][-1,+0]   `leave pop'                          - Pop one Object and invoke `operator leave()' on it. */
/*      ASM_                  0xa8  *               --------                            - ------------------ */
#define ASM_FPRINT_C          0xa9 /* [2][-1,+1]   `print top, const <imm8>'            - Print a constant from `<imm8>' to a file in stack-top. */
#define ASM_FPRINT_C_SP       0xaa /* [2][-1,+1]   `print top, const <imm8>, sp'        - Same as `ASM_FPRINT_C', but follow up by printing a space character. */
#define ASM_FPRINT_C_NL       0xab /* [2][-1,+1]   `print top, const <imm8>, nl'        - Same as `ASM_FPRINT_C', but follow up by printing a new-line character. */
#define ASM_RANGE             0xac /* [1][-2,+1]   `range top, pop'                     - Create a new range from using `top' as `begin' and `pop' as `end'. */
#define ASM_RANGE_DEF         0xad /* [1][-1,+1]   `push range default, pop'            - Create a new range from using `type(end)()' as `begin' and `top' as `end'. */
#define ASM_RANGE_STEP        0xae /* [1][-3,+1]   `range top, pop, pop'                - Create a new range from using `top' as `begin', `pop' (first) as `end' and `pop' (second) as step. */
#define ASM_RANGE_STEP_DEF    0xaf /* [1][-2,+1]   `push range default, pop, pop'       - Create a new range from using `type(end)()' as `begin', `top' as `end' and `pop' (second) as step. */

/* Sequence operators. */
#define ASM_CONTAINS          0xb0 /* [1][-2,+1]   `contains top, pop'                  - Pop an Object and invoke the __contains__ operator on stack-top. */
#define ASM_CONTAINS_C        0xb1 /* [2][-1,+1]   `push contains const <imm8>, pop'    - Pop an Object and check if it is contained within a sequence found int the given constant (which is usually a read-only HashSet). */
#define ASM_GETITEM           0xb2 /* [1][-2,+1]   `getitem top, pop'                   - Pop a key/index and invoke the __getitem__ operator on stack-top. */
#define ASM_GETITEM_I         0xb3 /* [3][-1,+1]   `getitem top, $<Simm16>'             - Invoke the __getitem__ operator on stack-top, using an int <Simm16> (little-endian) as index. */
#define ASM_GETITEM_C         0xb4 /* [2][-1,+1]   `getitem top, const <imm8>'          - Invoke the __getitem__ operator on stack-top, using constant slot `<imm8>' as key. */
#define ASM_GETSIZE           0xb5 /* [1][-1,+1]   `getsize top'                        - Invoke the __size__ operator on stack-top. */
#define ASM_SETITEM           0xb6 /* [1][-3,+0]   `setitem pop, pop, pop'              - Pop a value, a key/index and invoke the __setitem__ operator on stack-top. */
#define ASM_SETITEM_I         0xb7 /* [3][-2,+0]   `setitem pop, $<Simm16>, pop'        - Pop a value and invoke the __setitem__ operator on stack-top, using an int <Simm16> (little-endian) as index. */
#define ASM_SETITEM_C         0xb8 /* [2][-2,+0]   `setitem pop, const <imm8>, pop'     - Pop a value and invoke the __setitem__ operator on stack-top, using constant slot `<imm8>' as key. */
#define ASM_ITERSELF          0xb9 /* [1][-1,+1]   `iterself top'                       - Replace stack-top with an iterator of itself. */
#define ASM_DELITEM           0xba /* [1][-2,+0]   `delitem pop, pop'                   - Pop a key/index and invoke the __delitem__ operator on stack-top. */
#define ASM_GETRANGE          0xbb /* [1][-3,+1]   `getrange top, pop, pop'             - Pop two keys (`begin', `end') and use them to invoke the __getrange__ operator on stack-top. */
#define ASM_GETRANGE_PN       0xbc /* [1][-2,+1]   `getrange top, pop, none'            - Same as `ASM_GETRANGE', but use `none' for the `end' key. */
#define ASM_GETRANGE_NP       0xbd /* [1][-2,+1]   `getrange top, none, pop'            - Same as `ASM_GETRANGE', but use `none' for the `begin' key. */
#define ASM_GETRANGE_PI       0xbe /* [3][-2,+1]   `getrange top, pop, $<Simm16>'       - Same as `ASM_GETRANGE', but use an int <Simm16> for the `end' key. */
#define ASM_GETRANGE_IP       0xbf /* [3][-2,+1]   `getrange top, $<Simm16>, pop'       - Same as `ASM_GETRANGE', but use an int <Simm16> for the `begin' key. */
#define ASM_GETRANGE_NI       0xc0 /* [3][-1,+1]   `getrange top, none, $<Simm16>'      - Same as `ASM_GETRANGE', but use `none' for the `begin' key and an int <Simm16> for the `end' key. */
#define ASM_GETRANGE_IN       0xc1 /* [3][-1,+1]   `getrange top, $<Simm16>, none'      - Same as `ASM_GETRANGE', but use an int <Simm16> for the `begin' key and `none' for the `end' key. */
#define ASM_GETRANGE_II       0xc2 /* [5][-1,+1]   `getrange top, $<Simm16>, $<Simm16>' - Same as `ASM_GETRANGE', but use an int <Simm16> (first) for the `begin' key and <Simm16> (second) for the `end' key. */
#define ASM_DELRANGE          0xc3 /* [1][-3,+0]   `delrange pop, pop, pop'             - Pop two keys (`begin', `end') and use them to invoke the __delrange__ operator on stack-top. */
#define ASM_SETRANGE          0xc4 /* [1][-4,+0]   `setrange pop, pop, pop, pop'        - Pop a value, then two keys (`begin', `end') and use them to invoke the __setrange__ operator on stack-top. */
#define ASM_SETRANGE_PN       0xc5 /* [1][-3,+0]   `setrange pop, pop, none, pop'       - Same as `ASM_SETRANGE', but use `none' for the `end' key. */
#define ASM_SETRANGE_NP       0xc6 /* [1][-3,+0]   `setrange pop, none, pop, pop'       - Same as `ASM_SETRANGE', but use `none' for the `begin' key. */
#define ASM_SETRANGE_PI       0xc7 /* [3][-3,+0]   `setrange pop, pop, $<Simm16>, pop'  - Same as `ASM_SETRANGE', but use an int <Simm16> for the `end' key. */
#define ASM_SETRANGE_IP       0xc8 /* [3][-3,+0]   `setrange pop, $<Simm16>, pop, pop'  - Same as `ASM_SETRANGE', but use an int <Simm16> for the `begin' key. */
#define ASM_SETRANGE_NI       0xc9 /* [3][-2,+0]   `setrange pop, none, $<Simm16>, pop' - Same as `ASM_SETRANGE', but use `none' for the `begin' key and an int <Simm16> for the `end' key. */
#define ASM_SETRANGE_IN       0xca /* [3][-2,+0]   `setrange pop, $<Simm16>, none, pop' - Same as `ASM_SETRANGE', but use an int <Simm16> for the `begin' key and `none' for the `end' key. */
#define ASM_SETRANGE_II       0xcb /* [5][-2,+0]   `setrange pop, $<Simm16>, $<Simm16>, pop' - Same as `ASM_SETRANGE', but use an int <Simm16> (first) for the `begin' key and <Simm16> (second) for the `end' key. */

/* Breakpoint. */
#define ASM_BREAKPOINT        0xcc /* [1][-0,+0]   `debug break'                        - Break into an attached debugger.
                                    * NOTE: An opcode of CCh has been intentionally assigned for this
                                    *       opcode, so-as to mirror x86's int3 instruction, as well as
                                    *       give it the recognizable bit-layout `10101010'. */
#define ASM_UD                0xcd /* [1][-0,+0]   `ud'                                 - Undefined instruction (always raises an `Error.RuntimeError.IllegalInstruction'). */
/* Call attribute. */
#define ASM_CALLATTR_C_KW     0xce /* [4][-1-n,+1] `callattr top, const <imm8>, #<imm8>, const <imm8>' - Similar to `ASM_CALLATTR_C', but also pass a keywords mapping from `<imm8>' */
#define ASM_CALLATTR_C_TUPLE_KW 0xcf/*[3][-2,  +1] `callattr top, const <imm8>, pop..., const <imm8>'  - Similar to `ASM_CALLATTR_C_TUPLE', but also pass a keywords mapping from `<imm8>' */
#define ASM_CALLATTR          0xd0 /* [2][-2-n,+1] `callattr top, pop, #<imm8>'         - Pop #<imm8> arguments into a Tuple and a string, then use them to call an attribute in stack-top. @throws: Error.TypeError: The attribute Object isn't a string. */
#define ASM_CALLATTR_TUPLE    0xd1 /* [1][-3,  +1] `callattr top, pop, pop...'          - Pop a Tuple and a string, then use them to call an attribute in stack-top. @throws: Error.TypeError: The attribute Object isn't a string. */
#define ASM_CALLATTR_C        0xd2 /* [3][-1-n,+1] `callattr top, const <imm8>, #<imm8>' - Pop #<imm8> arguments into a Tuple and perform a fast attribute call on stack-top using a string in constant slot `<imm8>'. */
#define ASM_CALLATTR_C_TUPLE  0xd3 /* [2][-2,  +1] `callattr top, const <imm8>, pop...' - Pop a Tuple and perform a fast attribute call on stack-top using a string in constant slot `<imm8>'. */
#define ASM_CALLATTR_THIS_C   0xd4 /* [3][-n,  +1] `push callattr this, const <imm8>, #<imm8>' - Pop #<imm8> arguments into a Tuple and lookup and call an attribute of `this', using a string in constant slot `<imm8>' (Only valid for code with the `CODE_FTHISCALL' flag set) */
#define ASM_CALLATTR_THIS_C_TUPLE 0xd5 /* [2][-1,+1] `push callattr this, const <imm8>, pop...' - Pop a Tuple and lookup and call an attribute of `this', using a string in constant slot `<imm8>' (Only valid for code with the `CODE_FTHISCALL' flag set) */
#define ASM_CALLATTR_C_SEQ    0xd6 /* [3][-1-n,+1] `callattr top, const <imm8>, [#<imm8>]' - Call an attribute <imm8> with a single sequence-like argument packed from the top #<imm8> stack-items. */
#define ASM_CALLATTR_C_MAP    0xd7 /* [3][-1-n,+1] `callattr top, const <imm8>, {#<imm8>*2}' - Call an attribute <imm8> with a single mapping-like argument packed from the top #<imm8>*2 stack-items. */
/*      ASM_                  0xd8  *               --------                            - ------------------ */
#define ASM_GETMEMBER_THIS_R  0xd9 /* [3][-0,+1]   `push getmember this, ref <imm8>, $<imm8>' - Same as `ASM_GETMEMBER_THIS', but use a referenced variable `<imm8>' as class type. */
#define ASM_DELMEMBER_THIS_R  0xda /* [3][-0,+0]   `delmember this, ref <imm8>, $<imm8>' - Same as `ASM_DELMEMBER_THIS', but use a referenced variable `<imm8>' as class type. */
#define ASM_SETMEMBER_THIS_R  0xdb /* [3][-1,+0]   `setmember this, ref <imm8>, $<imm8>, pop' - Same as `ASM_SETMEMBER_THIS', but use a referenced variable `<imm8>' as class type. */
#define ASM_BOUNDMEMBER_THIS_R 0xdc/* [3][-0,+1]   `push boundmember this, ref <imm8>, $<imm8>' - Same as `ASM_BOUNDMEMBER_THIS', but use a referenced variable `<imm8>' as class type. */
#define ASM_CALL_EXTERN       0xdd /* [4][-n,+1]   `push call extern <imm8>:<imm8>, #<imm8>' - Pop #<imm8> (second) values from the stack, pack then into a Tuple, then call an external function referenced by <imm8>:<imm8> (first). */
#define ASM_CALL_GLOBAL       0xde /* [3][-n,+1]   `push call global <imm8>, #<imm8>'   - Pop #<imm8> (second) values from the stack, pack then into a Tuple, then call a function in global slot <imm8> (first). */
#define ASM_CALL_LOCAL        0xdf /* [3][-n,+1]   `push call local <imm8>, #<imm8>'    - Pop #<imm8> (second) values from the stack, pack then into a Tuple, then call a function in local slot <imm8> (first). */



/* Reserved. */
/*      ASM_                  0xe0  *               --------                            - ------------------ */
/*      ASM_                  0xe1  *               --------                            - ------------------ */
/*      ASM_                  0xe2  *               --------                            - ------------------ */
/*      ASM_                  0xe3  *               --------                            - ------------------ */
/*      ASM_                  0xe4  *               --------                            - ------------------ */
/*      ASM_                  0xe5  *               --------                            - ------------------ */
/*      ASM_                  0xe6  *               --------                            - ------------------ */
/*      ASM_                  0xe7  *               --------                            - ------------------ */
/*      ASM_                  0xe8  *               --------                            - ------------------ */
/*      ASM_                  0xe9  *               --------                            - ------------------ */
/*      ASM_                  0xea  *               --------                            - ------------------ */
/*      ASM_                  0xeb  *               --------                            - ------------------ */
/*      ASM_                  0xec  *               --------                            - ------------------ */
/*      ASM_                  0xed  *               --------                            - ------------------ */
/*      ASM_                  0xee  *               --------                            - ------------------ */
/*      ASM_                  0xef  *               --------                            - ------------------ */

/* Reserved. */
#define ASM_EXTENDED1         0xf0 /* Extended opcode prefix #1. */
#define ASM_RESERVED1         0xf1 /* Reserved for future expansion. */
#define ASM_RESERVED2         0xf2 /* Reserved for future expansion. */
#define ASM_RESERVED3         0xf3 /* Reserved for future expansion. */
#define ASM_RESERVED4         0xf4 /* Reserved for future expansion. */
#define ASM_RESERVED5         0xf5 /* Reserved for future expansion. */
#define ASM_RESERVED6         0xf6 /* Reserved for future expansion. */
#define ASM_RESERVED7         0xf7 /* Reserved for future expansion. */
#define ASM_EXTENDEDMIN       0xf0
#define ASM_EXTENDEDMAX       0xf7
#if 1 /* Only one extension table exists. */
#define ASM_ISEXTENDED(x)   ((x) == ASM_EXTENDED1)
#define CASE_ASM_EXTENDED    case ASM_EXTENDED1
#else
#if ASM_EXTENDEDMIN + 7 == ASM_EXTENDEDMAX
#define CASE_ASM_EXTENDED     \
	case ASM_EXTENDEDMIN + 0: \
	case ASM_EXTENDEDMIN + 1: \
	case ASM_EXTENDEDMIN + 2: \
	case ASM_EXTENDEDMIN + 3: \
	case ASM_EXTENDEDMIN + 4: \
	case ASM_EXTENDEDMIN + 5: \
	case ASM_EXTENDEDMIN + 6: \
	case ASM_EXTENDEDMIN + 7
#else /* ASM_EXTENDEDMIN + 7 == ASM_EXTENDEDMAX */
#define CASE_ASM_EXTENDED \
	case ASM_EXTENDEDMIN ... ASM_EXTENDEDMAX
#endif /* ASM_EXTENDEDMIN + 7 != ASM_EXTENDEDMAX */
#if ((ASM_EXTENDEDMIN & 0xf8) == (ASM_EXTENDEDMAX & 0xf8)) && \
    ((ASM_EXTENDEDMIN & 7) == 0 && (ASM_EXTENDEDMAX & 7) == 7)
#define ASM_ISEXTENDED(x) (((x)&0xf8) == ASM_EXTENDEDMIN)
#else /* ... */
#define ASM_ISEXTENDED(x) ((x) >= ASM_EXTENDEDMIN && (x) <= ASM_EXTENDEDMAX)
#endif /* !... */
#endif

/* Working storage class modifiers. */
/*      ASM_                  0xf8  *  --------                            - ------------------ */
/*      ASM_                  0xf9  *  --------                            - ------------------ */
/* XXX: Prefix: ASM_MEMBER `member this, ref <imm8>, <imm8>' - Use a member of `this' */
/*      ASM_                  0xfa  *  --------                            - ------------------ */
#define ASM_STACK             0xfb /* `stack #<imm8>'                      - Use an Object located on the stack as storage class.
                                    *                                        The associated operand is an absolute offset from the stack's base. */
#define ASM_STATIC            0xfc /* `static <imm8>'                      - Same as `ASM_LOCAL', but used to write to static variables. */
#define ASM_EXTERN            0xfd /* `extern <imm8>:<imm8>'               - Same as `ASM_LOCAL', but used to write to extern variables. */
#define ASM_GLOBAL            0xfe /* `global <imm8>'                      - Same as `ASM_LOCAL', but used to write to global variables. */
#define ASM_LOCAL             0xff /* `local <imm8>'
                                    * PREFX: Changes the target of the following instruction normally
                                    * writing its results back onto the stack, to perform an inplace
                                    * operation on a local variable who's index is encoded in an 8-bit
                                    * unsigned integer following this prefix instruction.
                                    * When used as prefix, this instruction changes the assembly
                                    * representation of the following instruction.
                                    * ASM_INC:        `local <imm8>: inc'
                                    * ASM_ADD:        `local <imm8>: add pop'
                                    * ASM_INCPOST:    `local <imm8>: push inc'
                                    * ASM_FUNCTION_C: `local <imm8>: function const <imm8>, #<imm8>+1'
                                    * ASM_OPERATOR:   `local <imm8>: op $<imm8>, #<imm8>+1'
                                    * @throws: Error.RuntimeError.IllegalInstruction: 
                                    *          The following instruction doesn't support this prefix, and
                                    *          the associated code object is marked as `CODE_FASSEMBLY'.
                                    *          WARNING: If the code object isn't marked as `CODE_FASSEMBLY',
                                    *                   an unsupported instruction causes undefined behavior.
                                    * @throws: Error.RuntimeError.UnboundLocal:
                                    *          The specified local variable is not assigned. */
#define ASM_PREFIXMIN         0xf8
#define ASM_PREFIXMAX         0xff
#define ASM_ISPREFIX(x)     ((x) >= ASM_PREFIXMIN)


/* Opcodes for misc/rarely used operators, as well as 16-bit
 * extended-operand instruction (Prefixed by `ASM_EXTENDED1'). */
/*      ASM_                  0xf000  *               --------                            - ------------------ */
/*      ASM_                  0xf001  *               --------                            - ------------------ */
/*      ASM_                  0xf002  *               --------                            - ------------------ */
/*      ASM_                  0xf003  *               --------                            - ------------------ */
/*      ASM_                  0xf004  *               --------                            - ------------------ */
/*      ASM_                  0xf005  *               --------                            - ------------------ */
#define ASM_ENDCATCH_N        0xf006 /* [3][-0,+0]   `end catch, #<imm8>+1'               - Unconditionally handle the <imm8>+1'th exception.
                                      *                                                    `end catch, #0' should be encoded the same as `end catch'
                                      * >> IF (thread->t_exceptsz - REG_EXCEPTION_START) > IMM8 THEN
                                      * >>     POP_NTH_EXCEPT(WITH handle_interrupt = TRUE, WITH nth = IMM8 + 1);
                                      * >> FI
                                      * This instruction is required to properly discard the primary
                                      * exceptions in catch-handlers that may throw further exceptions:
                                      * >> try {
                                      * >>     result = get_value_1();
                                      * >> } catch (...) {
                                      * >>     result = get_value_2(); // When this throws another exception, the one thrown by
                                      * >>                             // `get_value_1' must is discarded using `end catch, #1'
                                      * >> }
                                      * >> return result;
                                      * ASM:
                                      * >> .Lbegin_except_1:
                                      * >>     call  global @get_value_1, #0
                                      * >>     pop   local @result
                                      * >> .Lend_except_1:
                                      * >> 1:  ret   local @result
                                      * >> .except .Lbegin_except_1, .Lend_except_1, .Lentry_except_1
                                      * >> .Lentry_except_1:
                                      * >> .Lbegin_except_1_cleanup:
                                      * >>     call  global @get_value_2, #0
                                      * >>     pop   local @result
                                      * >>     jmp   1b
                                      * >> .Lend_except_1_cleanup:
                                      * >> .except .Lbegin_except_1_cleanup, .Lend_except_1_cleanup, .Lentry_except_1_cleanup
                                      * >> .Lentry_except_1_cleanup:
                                      * >>     // Handle the primary exception that already
                                      * >>     // existed before `get_value_2' was called
                                      * >>     end   catch, #1
                                      * >>     throw except
                                      * Note that exception cleanup code like this only appears
                                      * in catch-handlers, but not in finally-handlers.
                                      * If a finally handler raises a secondary exception, that
                                      * exception will be printed and discarded when the function
                                      * returns, causing it to still fail with the primary exception. */
#define ASM_ENDFINALLY_N      0xf007 /* [3][-0,+0] `end finally, #<imm8>+1'               - Same as `end finally', but only do so if the number of raised exceptions is `> #<imm8>+1'
                                      *                                                    `end finally, #0' should be encoded the same as `end finally'
                                      * >> IF IS_BOUND(REG_RESULT) THEN
                                      * >>     RETURN();
                                      * >> FI
                                      * >> IF (THREAD->t_exceptsz - REG_EXCEPTION_START) > IMM8 + 1 THEN
                                      * >>     EXCEPT();
                                      * >> FI */
#define ASM16_CALL_KW         0xf008 /* [5][-1-n,+1] `call top, #<imm8>, const <imm16>'   - Similar to `ASM_CALL', but also pass a keywords mapping from `<imm16>' */
#define ASM16_CALL_TUPLE_KW   0xf009 /* [4][-2,+1]   `call top, pop..., const <imm16>'    - Similar to `ASM_CALL_TUPLE', but also pass a keywords mapping from `<imm16>' */
/*      ASM_                  0xf00a  *               --------                            - ------------------ */
/*      ASM_                  0xf00b  *               --------                            - ------------------ */
#define ASM16_PUSH_BND_ARG    0xf00c /* [4][-0,+1]   `push bound arg <imm8>'              - Check if the argument variable indexed by `<imm16>' is bound, pushing true/false indicative of that state. */
#define ASM16_PUSH_BND_EXTERN 0xf00d /* [6][-0,+1]   `push bound extern <imm16>:<imm16>'  - Check if the extern variable indexed by `<imm16>:<imm16>' is bound, pushing true/false indicative of that state. */
#define ASM16_PUSH_BND_GLOBAL 0xf00e /* [4][-0,+1]   `push bound global <imm16>'          - Check if the global variable indexed by `<imm16>' is bound, pushing true/false indicative of that state. */
#define ASM16_PUSH_BND_LOCAL  0xf00f /* [4][-0,+1]   `push bound local <imm16>'           - Check if the local variable indexed by `<imm16>' is bound, pushing true/false indicative of that state. */
/*      ASM_                  0xf010  *               --------                            - ------------------ */
/*      ASM_                  0xf011  *               --------                            - ------------------ */
/*      ASM_                  0xf012  *               --------                            - ------------------ */
/*      ASM_                  0xf013  *               --------                            - ------------------ */
#define ASM32_JMP             0xf014 /* [6][-0,+0]   `jmp <Sdisp32>'                      - Unconditionally add `<Sdisp32>' (little endian) to the `REG_PC' of the next instruction. */
/*      ASM_                  0xf015  *               --------                            - ------------------ */
/*      ASM_                  0xf016  *               --------                            - ------------------ */
/*      ASM_                  0xf017  *               --------                            - ------------------ */
#define ASM_JMP_POP_POP       0xf018 /* [2][-2,+0]   `jmp pop, #pop'                      - Similar to `ASM_JMP_POP', but pop a second integer that describes the absolute stack-depth that should be adjusted for before jumping.
                                      * >> int new_sp = int(POP());
                                      * >> int new_ip = int(POP());
                                      * >> IF new_sp < 0 THEN
                                      * >>     THROW_OR_UNDEFINED_BEHAVIOR(Error.RuntimeError.SegFault());
                                      * >>     EXCEPT();
                                      * >> FI
                                      * >> IF new_sp > MAX_VALID_STACK_SIZE THEN
                                      * >>     IF STACK_DEPTH_IS_VARIABLE THEN
                                      * >>         INCREASE_STACK_DEPTH(new_sp);
                                      * >>     ELSE
                                      * >>         THROW_OR_UNDEFINED_BEHAVIOR(Error.RuntimeError.SegFault());
                                      * >>         EXCEPT();
                                      * >>     FI
                                      * >> FI
                                      * >> IF new_ip < 0 || new_ip > MAX_VALID_INSTRUCTION_INDEX THEN
                                      * >>     THROW_OR_UNDEFINED_BEHAVIOR(Error.RuntimeError.SegFault());
                                      * >>     EXCEPT();
                                      * >> FI
                                      * >> WHILE new_sp != SP DO
                                      * >>     IF new_sp < SP THEN
                                      * >>         POP();
                                      * >>     ELSE
                                      * >>         PUSH(none);
                                      * >>     FI
                                      * >> DONE
                                      * >> REG_PC = new_ip; */
#define ASM16_OPERATOR        0xf019 /* [5][-1-n,+1] `op top, $<imm16>, #<imm8>'          - Same as `ASM_OPERATOR', but can be used to invoke extended operator codes.
                                      * [5][-n,+1]   `PREFIX: push op $<imm16>, #<imm8>' */
#define ASM16_OPERATOR_TUPLE  0xf01a /* [4][-2,+1]   `op top, $<imm16>, pop'              - Same as `ASM_OPERATOR_TUPLE', but can be used to invoke extended operator codes.
                                      * [4][-1,+1]   `PREFIX: push op $<imm16>, pop' */
#define ASM_CALL_SEQ          0xf01b /* [3][-1-n,+1] `call top, [#<imm8>]'                - Similar to `ASM_CALL', but pass arguments packaged in some implementation-specific sequence type as a single argument. Used to implement range-initializers. */
#define ASM_CALL_MAP          0xf01c /* [3][-1-n,+1] `call top, {#<imm8>*2}'              - Similar to `ASM_CALL', but pass arguments packaged in some implementation-specific Dict-style sequence type as a single argument. Used to implement range-initializers. */
#define ASM_THISCALL_TUPLE    0xf01d /* [2][-3,+1]   `call top, pop, pop...'              - Perform a this-call (which is the equivalent of inserting `pop' before `pop...', then using the result as argument list).
                                      * >> Object args    = POP();
                                      * >> Object thisarg = POP();
                                      * >> Object func    = POP();
                                      * >> IF args !is Tuple THEN
                                      * >>     THROW_OR_UNDEFINED_BEHAVIOR(Error.TypeError());
                                      * >>     EXCEPT();
                                      * >> FI
                                      * >> PUSH(func(thisarg, args...)); */
#define ASM16_DEL_GLOBAL      0xf01e /* [4][-0,+0]   `del global <imm16>'                 - Unlink the global variable indexed by `<imm16>'. Throws an `UnboundLocal' error if the variable wasn't assigned to begin with. */
#define ASM16_DEL_LOCAL       0xf01f /* [4][-0,+0]   `del local <imm16>'                  - Unlink the local variable indexed by `<imm16>'. Throws an `UnboundLocal' error if the variable wasn't assigned to begin with. */
#define ASM_CALL_TUPLE_KWDS   0xf020 /* [2][-3,+1]   `call top, pop..., pop'              - The universal call-with-keywords instruction that also takes keywords from the stack. */
#define ASM16_LROT            0xf021 /* [4][-n,+n]   `lrot #<imm16>+3'                    - Rotate the top #<imm16>+2 stack objects left by one; away from SP.
                                      * [4][-n,+n]   `lrot #<imm16>+2, PREFIX'            - `PREFIX: lrot #<imm16>+3' */
#define ASM16_RROT            0xf022 /* [4][-n,+n]   `rrot #<imm16>+3'                    - Rotate the top #<imm16>+2 stack objects right by one; towards SP.
                                      * [4][-n,+n]   `rrot #<imm16>+2, PREFIX'            - `PREFIX: rrot #<imm16>+3' */
/*      ASM_                  0xf023  *               --------                            - ------------------ */
#define ASM16_DUP_N           0xf024 /* [4][-n,+n+1] `dup #SP - <imm16> - 2', `push #SP   - <imm16> - 2' - Push the #<imm16>+1'th stack entry (before adjustment).
                                      * [4][-n-1,+n+1] `mov PREFIX, #SP - <imm16> - 2'    - `PREFIX: dup #SP - <imm16> - 2', `PREFIX: push #SP - <imm16> - 2' */
/*      ASM_                  0xf025  *               --------                            - ------------------ */
#define ASM16_POP_N           0xf026 /* [4][-n-1,+n] `pop #SP - <imm16> - 2',             - Pop one stack and overwrite the `#<imm16>+1'th stack entry (before adjustment) with the value.
                                      * [4][-n-1,+n+1] `mov #SP - <imm16> - 2, PREFIX'    - `PREFIX: pop #SP - <imm16> - 2' */
#define ASM16_ADJSTACK        0xf027 /* [4][-?,+?]   `adjstack #SP +/- <Simm16>'          - Same as `ASM_ADJSTACK', but using a 16-bit stack-offset. */
/*      ASM_                  0xf028  *               --------                            - ------------------ */
#define ASM16_SUPER_THIS_R    0xf029 /* [4][-0,+1]   `push super this, ref <imm16>'       - Similar to `ASM_SUPER', but use a referenced variable `<imm16>' as type and the this-argument as Object. */
/*      ASM_                  0xf02a  *               --------                            - ------------------ */
/*      ASM_                  0xf02b  *               --------                            - ------------------ */
#define ASM16_POP_STATIC      0xf02c /* [4][-1,+0]   `pop static <imm16>'                 - Pop the top stack value into the static variable indexed by `<imm16>', overwriting the previous value.
                                      * [4][-0,+0]   `mov static <imm16>, PREFIX'         - `PREFIX: pop static <imm16>' */
#define ASM16_POP_EXTERN      0xf02d /* [6][-1,+0]   `pop extern <imm16>:<imm16>'         - Pop the top stack value into the extern variable indexed by `<imm16>:<imm16>', overwriting the previous value.
                                      * [6][-0,+0]   `mov PREFIX, extern <imm16>:<imm16>' - `PREFIX: pop extern <imm16>:<imm16>' */
#define ASM16_POP_GLOBAL      0xf02e /* [4][-1,+0]   `pop global <imm16>'                 - Pop the top stack value into the global variable indexed by `<imm16>', overwriting the previous value.
                                      * [4][-0,+0]   `mov PREFIX, global <imm16>'         - `PREFIX: pop global <imm16>' */
#define ASM16_POP_LOCAL       0xf02f /* [4][-1,+0]   `pop local <imm16>'                  - Pop the top stack value into the local variable indexed by `<imm16>', overwriting the previous value.
                                      * [4][-0,+0]   `mov PREFIX, local <imm16>'          - `PREFIX: pop local <imm16>' */
/*      ASM_                  0xf030  *               --------                            - ------------------ */
/*      ASM_                  0xf031  *               --------                            - ------------------ */
/*      ASM_                  0xf032  *               --------                            - ------------------ */
/*      ASM_                  0xf033  *               --------                            - ------------------ */
#define ASM_PUSH_EXCEPT       0xf034 /* [2][-0,+1]   `push except'                        - Push the current exception onto the stack. (Used by `catch' statements to access throw's error Object)
                                      * [2][-0,+0]   `mov  PREFIX, except'                - `PREFIX: push except' */
#define ASM_PUSH_THIS         0xf035 /* [2][-0,+1]   `push this'                          - Push the `this' argument onto the stack (Only valid for code with the `CODE_FTHISCALL' flag set)
                                      * [2][-0,+0]   `mov  PREFIX, this'                  - `PREFIX: push this' */
#define ASM_PUSH_THIS_MODULE  0xf036 /* [2][-0,+1]   `push this_module'                   - Push the current module.
                                      * [2][-0,+0]   `mov  PREFIX, this_module'           - `PREFIX: push this_module' */
#define ASM_PUSH_THIS_FUNCTION 0xf037/* [2][-0,+1]   `push this_function'                 - Push the calling function onto the stack.
                                      * [2][-0,+0]   `mov  PREFIX, this_function'         - `PREFIX: push this_function' */
#define ASM16_PUSH_MODULE     0xf038 /* [4][-0,+1]   `push module <imm16>'                - Push an imported module indexed by <imm16> from the current module.
                                      * [4][-0,+0]   `mov  PREFIX, module <imm16>'        - `PREFIX: push module <imm16>' */
#define ASM16_PUSH_ARG        0xf039 /* [4][-0,+1]   `push arg <imm16>'                   - Push the argument indexed by `<imm16>'.
                                      * [4][-0,+0]   `mov  PREFIX, arg <imm16>'           - `PREFIX: push arg <imm16>' */
#define ASM16_PUSH_CONST      0xf03a /* [4][-0,+1]   `push const <imm16>'                 - Same as `ASM_PUSH_STATIC16', but doesn't require a lock as the slot should acts as an immutable constant.
                                      * [4][-0,+0]   `mov  PREFIX, const <imm16>'         - `PREFIX: push const <imm16>' */
#define ASM16_PUSH_REF        0xf03b /* [4][-0,+1]   `push ref <imm16>'                   - Push a referenced variable indexed by `<imm16>'.
                                      * [4][-0,+0]   `mov  PREFIX, ref <imm16>'           - `PREFIX: push ref <imm16>' */
#define ASM16_PUSH_STATIC     0xf03c /* [4][-0,+1]   `push static <imm16>'                - Push the static variable indexed by `<imm16>'.
                                      * [4][-0,+0]   `mov  PREFIX, static <imm16>'        - `PREFIX: push static <imm16>' */
#define ASM16_PUSH_EXTERN     0xf03d /* [6][-0,+0]   `push extern <imm16>:<imm16>'        - Push the extern variable indexed by `<imm16>:<imm16>'. If it isn't bound, throw an `Error.RuntimeError.UnboundLocal'
                                      * [4][-0,+0]   `mov  PREFIX, extern <imm16>:<imm16>'- `PREFIX: push extern <imm16>:<imm16>' */
#define ASM16_PUSH_GLOBAL     0xf03e /* [4][-0,+1]   `push global <imm16>'                - Push the global variable indexed by `<imm16>'. If it isn't bound, throw an `Error.RuntimeError.UnboundLocal'
                                      * [4][-0,+0]   `mov  PREFIX, global <imm16>'        - `PREFIX: push global <imm16>' */
#define ASM16_PUSH_LOCAL      0xf03f /* [4][-0,+1]   `push local <imm16>'                 - Push the local variable indexed by `<imm16>'. If it isn't bound, throw an `Error.RuntimeError.UnboundLocal'
                                      * [4][-0,+0]   `mov  PREFIX, local <imm16>'         - `PREFIX: push local <imm16>' */
#define ASM_CAST_HASHSET      0xf040 /* [2][-1,+1]   `cast top, HashSet'                  - Convert a sequence in stack-top into a HashSet.
                                      * >> PUSH(HashSet(POP())); */
#define ASM_CAST_DICT         0xf041 /* [2][-1,+1]   `cast top, Dict'                     - Convert a sequence in stack-top into a Dict.
                                      * >> PUSH(Dict(POP())); */
#define ASM16_PACK_TUPLE      0xf042 /* [4][-n,+1]   `push pack Tuple, #<imm16>'          - Pop <imm16> elements and pack them into a Tuple.
                                      * >> PUSH(Tuple({ POP(IMM16)... })); */
#define ASM16_PACK_LIST       0xf043 /* [4][-n,+1]   `push pack List, #<imm16>'           - Pop <imm16> elements and pack them into a List.
                                      * >> PUSH(List({ POP(IMM16)... })); */
/*      ASM_                  0xf044  *               --------                            - ------------------ */
/*      ASM_                  0xf045  *               --------                            - ------------------ */
#define ASM16_UNPACK          0xf046 /* [4][-1,+n]   `unpack pop, #<imm16>'                 - Pop a sequence and unpack it into <imm16> elements then pushed onto the stack.
                                      * >> PUSH(Tuple.unpack(POP(),IMM16)...); */
/*      ASM_                  0xf047  *               --------                            - ------------------ */
/*      ASM_                  0xf048  *               --------                            - ------------------ */
/*      ASM_                  0xf049  *               --------                            - ------------------ */
/*      ASM_                  0xf04a  *               --------                            - ------------------ */
/*      ASM_                  0xf04b  *               --------                            - ------------------ */
/*      ASM_                  0xf04c  *               --------                            - ------------------ */
/*      ASM_                  0xf04d  *               --------                            - ------------------ */
/*      ASM_                  0xf04e  *               --------                            - ------------------ */
/*      ASM_                  0xf04f  *               --------                            - ------------------ */
#define ASM_PUSH_TRUE         0xf050 /* [2][-0,+1]   `push true'                          - The the builtin `true' singleton onto the stack.
                                      * [4][-0,+0]   `mov  PREFIX, true'                  - `PREFIX: push true'
                                      * >> DESTINATION = true; */
#define ASM_PUSH_FALSE        0xf051 /* [2][-0,+1]   `push false'                         - The the builtin `false' singleton onto the stack.
                                      * [4][-0,+0]   `mov  PREFIX, false'                 - `PREFIX: push false'
                                      * >> DESTINATION = false; */
#define ASM_PACK_HASHSET      0xf052 /* [3][-n,+1]   `push pack HashSet, #<imm8>'         - Pop <imm8> elements and pack them into a HashSet.
                                      * >> PUSH(HashSet({ POP(IMM8)... })); */
#define ASM_PACK_DICT         0xf053 /* [3][-n,+1]   `push pack Dict, #<imm8>*2'          - Pop <imm8>*2 elements and pack them into a Dict, using every first as key and every second as item.
                                      * >> PUSH(Dict({ POP(IMM8 * 2)... }.segments(2))); */
/*      ASM_                  0xf054  *               --------                            - ------------------ */
/*      ASM_                  0xf055  *               --------                            - ------------------ */
/*      ASM_                  0xf056  *               --------                            - ------------------ */
/*      ASM_                  0xf057  *               --------                            - ------------------ */
/*      ASM_                  0xf058  *               --------                            - ------------------ */
#define ASM_BOUNDITEM         0xf059 /* [2][-2,+1]   `bounditem top, pop'                 - Pop an index/key, then use it to check if an item is bound in stack-top.
                                      * >> PUSH(bound(POP().operator [] (POP())); */
#define ASM16_GETATTR_C       0xf05a /* [4][-1,+1]   `getattr top, const <imm16>'         - Perform a fast attribute lookup on stack-top using a string in constant slot `<imm16>' (little-endian).
                                      * >> PUSH(POP().operator . (CONST(IMM16)); */
#define ASM16_DELATTR_C       0xf05b /* [4][-1,+0]   `delattr pop, const <imm16>'         - Delete an attribute of stack-top named by a string in constant slot `<imm16>' (little-endian).
                                      * >> POP().operator del. (CONST(IMM16)); */
#define ASM16_SETATTR_C       0xf05c /* [4][-2,+0]   `setattr pop, const <imm16>, pop'    - Pop a value and set an attribute of (then) stack-top named by a string in constant slot `<imm16>' (little-endian).
                                      * >> POP().operator .= (CONST(IMM16), POP()); */
#define ASM16_GETATTR_THIS_C  0xf05d /* [4][-0,+1]   `push getattr this, const <imm16>'   - Lookup and push an attribute of `this', using a string in constant slot `<imm16>' (Only valid for code with the `CODE_FTHISCALL' flag set)
                                      * >> PUSH(THIS.operator . (CONST(IMM16))); */
#define ASM16_DELATTR_THIS_C  0xf05e /* [4][-0,+0]   `delattr this, const <imm16>'        - Delete an attribute of `this', named by a string in constant slot `<imm16>' (Only valid for code with the `CODE_FTHISCALL' flag set)
                                      * >> THIS.operator del. (CONST(IMM16)); */
#define ASM16_SETATTR_THIS_C  0xf05f /* [4][-1,+0]   `setattr this, const <imm16>, pop'   - Pop a value and set an attribute of `this', named by a string in constant slot `<imm16>' (Only valid for code with the `CODE_FTHISCALL' flag set)
                                      * >> THIS.operator .= (CONST(IMM16), POP()); */
#define ASM_CMP_SO            0xf060 /* [2][-2,+1]   `cmp so, top, pop'                   - Compare for SameObject and push the result (`Object.id(a) == Object.id(b)' / `a === b').
                                      * >> PUSH(POP() === POP()); */
#define ASM_CMP_DO            0xf061 /* [2][-2,+1]   `cmp do, top, pop'                   - Compare for DifferentObject and push the result (`Object.id(a) != Object.id(b)' / `a !== b').
                                      * >> PUSH(POP() !== POP()); */
#define ASM16_PACK_HASHSET    0xf062 /* [4][-n,+1]   `push pack HashSet, #<imm16>'        - Pop <imm16> elements and pack them into a HashSet.
                                      * >> PUSH(HashSet({ POP(IMM16)... })); */
#define ASM16_PACK_DICT       0xf063 /* [4][-n,+1]   `push pack Dict, #<imm16>*2'         - Pop <imm16>*2 elements and pack them into a Dict, using every first as key and every second as item.
                                      * >> PUSH(Dict({ POP(IMM16 * 2)... }.segments(2))); */
#define ASM16_GETCMEMBER      0xf064 /* [4][-1,+1]   `getcmember top, $<imm16>'           - Lookup a class member of `top', given its index.
                                      * >> PUSH(type.__ctable__(POP())[IMM16]); */
#define ASM_CLASS             0xf065 /* [2][-2,+1]   `class top, pop'                     - Same as `ASM_CLASS_C', however the class descriptor is popped from the stack.
                                      * >> PUSH(rt.makeclass(POP(), POP())); */
#define ASM16_CLASS_C         0xf066 /* [4][-1,+1]   `class top, const <imm16>'           - Same as `ASM_CLASS_C', however operands are is extended to 16 bits.
                                      * >> PUSH(rt.makeclass(POP(),CONST(IMM16))); */
#define ASM16_CLASS_GC        0xf067 /* [6][-0,+1]   `push class global <imm16>, const <imm16>' - Same as `ASM_CLASS_GC', however operands are is extended to 16 bits.
                                      * >> PUSH(rt.makeclass(GLOBAL(IMM16),CONST(IMM16))); */
#define ASM16_CLASS_EC        0xf068 /* [8][-0,+1]   `push class extern <imm16>:<imm16>, const <imm16>' - Same as `ASM_CLASS_EC', however operands are extended to 16 bits.
                                      * >> PUSH(rt.makeclass(EXTERN(IMM16,IMM16),CONST(IMM16))); */
#define ASM16_DEFCMEMBER      0xf069 /* [4][-2,+1]   `defcmember top, $<imm16>, pop'      - Initialize a class member variable `<imm16>' using a value from the stack.
                                      * >> type.__ctable__(TOP)[IMM16] = POP(); */
#define ASM16_GETCMEMBER_R    0xf06a /* [6][-0,+1]   `push getcmember ref <imm16>, $<imm16>'- Lookup a class member of `ref <imm16>', given its index. */
#define ASM16_CALLCMEMBER_THIS_R 0xf06b/*[7][-n,+1]  `push callcmember this, ref <imm16>, $<imm16>, #<imm8>'- Lookup a class member of `ref <imm16>', given its index, then call that attribute as a this-call by popping #<imm8> arguments. */
/*      ASM_                  0xf06c  *               --------                            - ------------------ */
/*      ASM_                  0xf06d  *               --------                            - ------------------ */
#define ASM16_FUNCTION_C      0xf06e /* [5][-n,+1]   `push function const <imm16>, #<imm8>+1' - Create a new function Object, using constant slot <imm16> as code and popping $<imm8>+1 objects for use by references.
                                      * [5][-n,+0]   `PREFIX: function const <imm16>, #<imm8>+1' */
#define ASM16_FUNCTION_C_16   0xf06f /* [6][-n,+1]   `push function const <imm16>, #<imm16>+1' - Create a new function Object, using constant slot <imm16> as code and popping $<imm16>+1 objects for use by references.
                                      * [6][-n,+0]   `PREFIX: function const <imm16>, #<imm16>+1' */
#define ASM_SUPERGETATTR_THIS_RC 0xf070/*[4][-0,+1]  `push getattr this, ref <imm8>, const <imm8>' - Perform a super attribute lookup `(this as ref <imm8>).operator . (const <imm8>)'. */
#define ASM16_SUPERGETATTR_THIS_RC 0xf071/*[6][-0,+1]`push getattr this, ref <imm16>, const <imm16>' - Perform a super attribute lookup `(this as ref <imm16>).operator . (const <imm16>)'. */
#define ASM_SUPERCALLATTR_THIS_RC 0xf072/*[5][-n,+1] `push callattr this, ref <imm8>, const <imm8>, #<imm8>' - Perform a supercall on `(this as ref <imm8>).operator . (const <imm8>)', using `#<imm8>' arguments from the stack. */
#define ASM16_SUPERCALLATTR_THIS_RC 0xf073/*[7][-n,+1]`push callattr this, ref <imm16>, const <imm16>, #<imm8>' - Perform a supercall on `(this as ref <imm16>).operator . (const <imm16>)', using `#<imm8>' arguments from the stack. */
/*      ASM_                  0xf074  *               --------                            - ------------------ */
/*      ASM_                  0xf075  *               --------                            - ------------------ */
/*      ASM_                  0xf076  *               --------                            - ------------------ */
/*      ASM_                  0xf077  *               --------                            - ------------------ */
/*      ASM_                  0xf078  *               --------                            - ------------------ */
/*      ASM_                  0xf079  *               --------                            - ------------------ */
/*      ASM_                  0xf07a  *               --------                            - ------------------ */
/*      ASM_                  0xf07b  *               --------                            - ------------------ */
/*      ASM_                  0xf07c  *               --------                            - ------------------ */
/*      ASM_                  0xf07d  *               --------                            - ------------------ */
/*      ASM_                  0xf07e  *               --------                            - ------------------ */
#define ASM_INCPOST           0xf07f /* [2][-0,+1]   `push inc'                           - Push a copy, then increment a variable by one. WARNING: Only available as inplace op with storage prefix.
                                      * [2][-0,+1]   `push inc PREFIX' - `PREFIX: push inc' */
#define ASM_DECPOST           0xf080 /* [2][-0,+1]   `push dec'                           - Push a copy, then decrement a variable by one. WARNING: Only available as inplace op with storage prefix.
                                      * [2][-0,+1]   `push dec PREFIX' - `PREFIX: push dec' */
/*      ASM_                  0xf081  *               --------                            - ------------------ */
/*      ASM_                  0xf082  *               --------                            - ------------------ */
/*      ASM_                  0xf083  *               --------                            - ------------------ */
/*      ASM_                  0xf084  *               --------                            - ------------------ */
/*      ASM_                  0xf085  *               --------                            - ------------------ */
/*      ASM_                  0xf086  *               --------                            - ------------------ */
/*      ASM_                  0xf087  *               --------                            - ------------------ */
/*      ASM_                  0xf088  *               --------                            - ------------------ */
/*      ASM_                  0xf089  *               --------                            - ------------------ */
/*      ASM_                  0xf08a  *               --------                            - ------------------ */
/*      ASM_                  0xf08b  *               --------                            - ------------------ */
/*      ASM_                  0xf08c  *               --------                            - ------------------ */
/*      ASM_                  0xf08d  *               --------                            - ------------------ */
/*      ASM_                  0xf08e  *               --------                            - ------------------ */
#define ASM16_DELOP           0xf08f /* [2][-0,+0]    --------                            - Same as `ASM_DELOP' (we ignore the F0 prefix) */
#define ASM16_NOP             0xf090 /* [2][-0,+0]   `nop16'                              - Same as `ASM_NOP' (we ignore the F0 prefix)
                                      *              `nop16 PREFIX' - `PREFIX: nop16' */
#define ASM_REDUCE_MIN        0xf091 /* [2][-1,+1]   `reduce top, min'                    - Push the lowest element of a sequence `top' */
#define ASM_REDUCE_MAX        0xf092 /* [2][-1,+1]   `reduce top, max'                    - Push the greatest element of a sequence `top' */
#define ASM_REDUCE_SUM        0xf093 /* [2][-1,+1]   `reduce top, sum'                    - Push the sum of all element in sequence `top' */
#define ASM_REDUCE_ANY        0xf094 /* [2][-1,+1]   `reduce top, any'                    - Push `true' if any element of a sequence `top' is true. */
#define ASM_REDUCE_ALL        0xf095 /* [2][-1,+1]   `reduce top, all'                    - Push `true' if all elements of a sequence `top' are true. */
/*      ASM_                  0xf096  *               --------                            - ------------------ */
/*      ASM_                  0xf097  *               --------                            - ------------------ */
/*      ASM_                  0xf098  *               --------                            - ------------------ */
/*      ASM_                  0xf099  *               --------                            - ------------------ */
/*      ASM_                  0xf09a  *               --------                            - ------------------ */
/*      ASM_                  0xf09b  *               --------                            - ------------------ */
/*      ASM_                  0xf09c  *               --------                            - ------------------ */
/*      ASM_                  0xf09d  *               --------                            - ------------------ */
/*      ASM_                  0xf09e  *               --------                            - ------------------ */
/*      ASM_                  0xf09f  *               --------                            - ------------------ */
/*      ASM_                  0xf0a0  *               --------                            - ------------------ */
#define ASM16_PRINT_C         0xf0a1 /* [4][-0,+0]   `print const <imm16>'                - Print a constant from `<imm16>' to stdout. */
#define ASM16_PRINT_C_SP      0xf0a2 /* [4][-0,+0]   `print const <imm16>, sp'            - Same as `ASM_PRINT_C16', but follow up by printing a space character. */
#define ASM16_PRINT_C_NL      0xf0a3 /* [4][-0,+0]   `print const <imm16>, nl'            - Same as `ASM_PRINT_C16', but follow up by printing a new-line character. */
#define ASM_RANGE_0_I32       0xf0a4 /* [6][-0,+1]   `push range $0, $<imm32>'            - Create a new range from using `int(0)' as `begin' and `int(<imm32>)' as `end'. */
/*      ASM_                  0xf0a5  *               --------                            - ------------------ */
#define ASM_VARARGS_UNPACK    0xf0a6 /* [3][-0,+n]   `unpack varargs, #<imm8>'            - Unpack variable arguments and push `imm8' stack items. - Behaves the same as `push varargs; unpack pop, #<imm8>', except that the varargs Tuple doesn't need to be pushed. */
#define ASM_PUSH_VARKWDS_NE   0xf0a7 /* [2][-0,+1]   `push bool varkwds'                  - Push true/false indicative of variable arguments being present. (Illegal instruction if the code doesn't have the `CODE_FVARKWDS' flag set) */
/*      ASM_                  0xf0a7  *               --------                            - ------------------ */
/*      ASM_                  0xf0a8  *               --------                            - ------------------ */
#define ASM16_FPRINT_C        0xf0a9 /* [4][-1,+1]   `print top, const <imm16>'           - Print a constant from `<imm16>' to a file in stack-top. */
#define ASM16_FPRINT_C_SP     0xf0aa /* [4][-1,+1]   `print top, const <imm16>, sp'       - Same as `ASM_FPRINT_C16', but follow up by printing a space character. */
#define ASM16_FPRINT_C_NL     0xf0ab /* [4][-1,+1]   `print top, const <imm16>, nl'       - Same as `ASM_FPRINT_C16', but follow up by printing a new-line character. */
#define ASM_VARARGS_CMP_EQ_SZ 0xf0ac /* [3][-0,+1]   `push cmp eq, #varargs, $<imm8>'     - Compare the number of variable arguments with imm8, and push true if they are equal, or false if they differ. */
#define ASM_VARARGS_CMP_GR_SZ 0xf0ad /* [3][-0,+1]   `push cmp gr, #varargs, $<imm8>'     - Same as `ASM_VARARGS_CMP_EQ_SZ', but compare for greater-than */
/*      ASM_                  0xf0ae  *               --------                            - ------------------ */
/*      ASM_                  0xf0af  *               --------                            - ------------------ */
/*      ASM_                  0xf0b0  *               --------                            - ------------------ */
#define ASM16_CONTAINS_C      0xf0b1 /* [3][-1,+1]   `push contains const <imm16>, pop'   - Pop an Object and check if it is contained within a sequence found int the given constant (which is usually a read-only HashSet). */
#define ASM_VARARGS_GETITEM   0xf0b2 /* [2][-1,+1]   `getitem varargs, top'               - Pop an index `i', and use it to look up the i'th variable argument. If the index is negative, throw IntegerOverflow, If it is too large, throw IndexError. (Illegal instruction if the code doesn't have the `CODE_FVARARGS' flag set) */
#define ASM_VARARGS_GETITEM_I 0xf0b3 /* [3][-0,+1]   `push getitem varargs, $<imm8>'      - Same as `getitem varargs, top', however the index is encoded as an 8-bit, unsigned immediate operand. (Illegal instruction if the code doesn't have the `CODE_FVARARGS' flag set) */
#define ASM16_GETITEM_C       0xf0b4 /* [4][-1,+1]   `getitem top, const <imm16>'         - Invoke the __getitem__ operator on stack-top, using constant slot `<imm16>' (little-endian) as key. */
#define ASM_VARARGS_GETSIZE   0xf0b5 /* [2][-0,+1]   `push getsize varargs', `push #varargs' - Push the number of variable arguments onto the stack. (Illegal instruction if the code doesn't have the `CODE_FVARARGS' flag set) */
/*      ASM_                  0xf0b6  *               --------                            - ------------------ */
/*      ASM_                  0xf0b7  *               --------                            - ------------------ */
#define ASM16_SETITEM_C       0xf0b8 /* [4][-2,+0]   `setitem pop, const <imm16>, pop'    - Pop a value and invoke the __setitem__ operator on stack-top, using constant slot `<imm16>' (little-endian) as key. */
#define ASM_ITERNEXT          0xf0b9 /* [2][-1,+1]   `iternext top'                       - Replace stack-top with the result of `top.operator iter()' (Throws `Signal.StopIteration' if the iterator has been exhausted). */
/*      ASM_                  0xf0ba  *               --------                            - ------------------ */
/*      ASM_                  0xf0bb  *               --------                            - ------------------ */
/*      ASM_                  0xf0bc  *               --------                            - ------------------ */
/*      ASM_                  0xf0bd  *               --------                            - ------------------ */
#define ASM_GETMEMBER         0xf0be /* [3][-2,+1]   `getmember top, pop, $<imm8>'        - Pop a class type, then an instance describing of that class and push a member at index `$<imm8>' onto the stack. */
#define ASM16_GETMEMBER       0xf0bf /* [4][-2,+1]   `getmember top, pop, $<imm16>'       - Pop a class type, then an instance describing of that class and push a member at index `$<imm16>' onto the stack. */
#define ASM_DELMEMBER         0xf0c0 /* [3][-2,+0]   `delmember pop, pop, $<imm8>'        - Pop a class type, then an instance describing of that class and delete a member at index `$<imm8>'. */
#define ASM16_DELMEMBER       0xf0c1 /* [4][-2,+0]   `delmember pop, pop, $<imm16>'       - Pop a class type, then an instance describing of that class and delete a member at index `$<imm16>'. */
#define ASM_SETMEMBER         0xf0c2 /* [3][-3,+0]   `setmember pop, pop, $<imm8>, pop'   - Pop a value, a class type, and an instance, assigning the value to the member at instance index `$<imm8>' of the instance onto the stack. */
#define ASM16_SETMEMBER       0xf0c3 /* [4][-3,+0]   `setmember pop, pop, $<imm16>, pop'  - Pop a value, a class type, and an instance, assigning the value to the member at instance index `$<imm16>' of the instance onto the stack. */
#define ASM_BOUNDMEMBER       0xf0c4 /* [3][-2,+1]   `boundmember top, pop, $<imm8>'      - Pop an instance, a class type describing one of the bases of the instance  and push Dee_True/Dee_False if member at index `$<imm8>' is bound. */
#define ASM16_BOUNDMEMBER     0xf0c5 /* [4][-2,+1]   `boundmember top, pop, $<imm16>'     - Pop an instance, a class type describing one of the bases of the instance  and push Dee_True/Dee_False if member at index `$<imm16>' is bound. */
#define ASM_GETMEMBER_THIS    0xf0c6 /* [3][-1,+1]   `push getmember this, pop, $<imm8>'  - Pop a class type describing one of the bases of `this' and push a member at index `$<imm8>' onto the stack. */
#define ASM16_GETMEMBER_THIS  0xf0c7 /* [4][-1,+1]   `push getmember this, pop, $<imm16>' - Pop a class type describing one of the bases of `this' and push a member at index `$<imm16>' onto the stack. */
#define ASM_DELMEMBER_THIS    0xf0c8 /* [3][-1,+0]   `delmember this, pop, $<imm8>'       - Pop a class type describing one of the bases of `this' and delete a member at index `$<imm8>'. */
#define ASM16_DELMEMBER_THIS  0xf0c9 /* [4][-1,+0]   `delmember this, pop, $<imm16>'      - Pop a class type describing one of the bases of `this' and delete a member at index `$<imm16>'. */
#define ASM_SETMEMBER_THIS    0xf0ca /* [3][-2,+0]   `setmember this, pop, $<imm8>, pop'  - Pop a value and class type, assigning the value to the member at instance index `$<imm8>' onto the stack. */
#define ASM16_SETMEMBER_THIS  0xf0cb /* [4][-2,+0]   `setmember this, pop, $<imm16>, pop' - Pop a value and class type, assigning the value to the member at instance index `$<imm16>' onto the stack. */
#define ASM_BOUNDMEMBER_THIS  0xf0cc /* [3][-1,+1]   `push boundmember this, pop, $<imm8>' - Pop a class type describing one of the bases of `this' and push Dee_True/Dee_False if member at index `$<imm8>' is bound. */
#define ASM16_BOUNDMEMBER_THIS 0xf0cd/* [4][-1,+1]   `push boundmember this, pop, $<imm16>' - Pop a class type describing one of the bases of `this' and push Dee_True/Dee_False if member at index `$<imm16>' is bound. */
#define ASM16_CALLATTR_C_KW   0xf0ce /* [7][-1-n,+1] `callattr top, const <imm16>, #<imm8>, const <imm16>' - Similar to `ASM_CALLATTR_C', but also pass a keywords mapping from `<imm8>' */
#define ASM16_CALLATTR_C_TUPLE_KW 0xf0cf/*[6][-2,+1] `callattr top, const <imm16>, pop..., const <imm16>'  - Similar to `ASM_CALLATTR_C_TUPLE', but also pass a keywords mapping from `<imm8>' */
#define ASM_CALLATTR_KWDS     0xf0d0 /* [2][-3-n,+1] `callattr top, pop, #<imm8>, pop'    - Similar to `ASM_CALLATTR_TUPLE_KWDS', but take arguments from the stack */
#define ASM_CALLATTR_TUPLE_KWDS 0xf0d1 /*[2][-4,+1]  `callattr top, pop, pop..., pop'     - The universal call-attribute-with-keywords instruction that also takes keywords from the stack. */
#define ASM16_CALLATTR_C      0xf0d2 /* [5][-1-n,+1] `callattr top, const <imm16>, #<imm8>' - Pop #<imm8> arguments into a Tuple and perform a fast attribute call on stack-top using a string in constant slot `<imm16>' (little-endian). */
#define ASM16_CALLATTR_C_TUPLE 0xf0d3 /*[4][-2,+1]   `callattr top, const <imm16>, pop'   - Pop a Tuple and perform a fast attribute call on stack-top using a string in constant slot `<imm16>' (little-endian). */
#define ASM16_CALLATTR_THIS_C 0xf0d4 /* [5][-n,+1]   `callattr this, const <imm16>, #<imm8>' - Pop #<imm8> arguments into a Tuple and lookup and call an attribute of `this', using a string in constant slot `<imm16>' (Only valid for code with the `CODE_FTHISCALL' flag set) */
#define ASM16_CALLATTR_THIS_C_TUPLE 0xf0d5 /* [4][-1,+1] `callattr this, const <imm16>, pop' - Pop a Tuple and lookup and call an attribute of `this', using a string in constant slot `<imm16>' (Only valid for code with the `CODE_FTHISCALL' flag set) */
#define ASM16_CALLATTR_C_SEQ  0xf0d6 /* [4][-1-n,+1] `callattr top, const <imm16>, [#<imm8>]' - Call an attribute <imm16> with a single sequence-like argument packed from the top #<imm8> stack-items. */
#define ASM16_CALLATTR_C_MAP  0xf0d7 /* [4][-1-n,+1] `callattr top, const <imm16>, {#<imm8>*2}' - Call an attribute <imm16> with a single mapping-like argument packed from the top #<imm8>*2 stack-items. */
/*      ASM_                  0xf0d8  *               --------                            - ------------------ */
#define ASM16_GETMEMBER_THIS_R 0xf0d9/* [6][-0,+1]   `push getmember this, ref <imm16>, $<imm16>' - Same as `ASM16_GETMEMBER_THIS', but use a referenced variable `<imm16>' as class type. */
#define ASM16_DELMEMBER_THIS_R 0xf0da/* [6][-0,+0]   `delmember this, ref <imm16>, $<imm16>' - Same as `ASM16_DELMEMBER_THIS', but use a referenced variable `<imm16>' as class type. */
#define ASM16_SETMEMBER_THIS_R 0xf0db/* [6][-1,+0]   `setmember this, ref <imm16>, $<imm16>, pop' - Same as `ASM16_SETMEMBER_THIS', but use a referenced variable `<imm16>' as class type. */
#define ASM16_BOUNDMEMBER_THIS_R 0xf0dc/*[6][-0,+1]  `push boundmember this, ref <imm16>, $<imm16>' - Same as `ASM16_BOUNDMEMBER_THIS', but use a referenced variable `<imm16>' as class type. */
#define ASM16_CALL_EXTERN     0xf0dd /* [7][-n,+1]   `push call extern <imm16>:<imm16>, #<imm8>' - Pop #<imm8> values from the stack, pack then into a Tuple, then call an external function referenced by <imm16>:<imm16>. */
#define ASM16_CALL_GLOBAL     0xf0de /* [5][-n,+1]   `push call global <imm16>, #<imm8>'  - Pop #<imm8> values from the stack, pack then into a Tuple, then call a function in global slot <imm16>. */
#define ASM16_CALL_LOCAL      0xf0df /* [5][-n,+1]   `push call local <imm16>, #<imm8>'   - Pop #<imm8> values from the stack, pack then into a Tuple, then call a function in local slot <imm16>. */
/*      ASM_                  0xf0f0  *               --------                            - ------------------ */
/*      ASM_                  0xf0f1  *               --------                            - ------------------ */
/*      ASM_                  0xf0f2  *               --------                            - ------------------ */
/*      ASM_                  0xf0f3  *               --------                            - ------------------ */
/*      ASM_                  0xf0f4  *               --------                            - ------------------ */
/*      ASM_                  0xf0f5  *               --------                            - ------------------ */
/*      ASM_                  0xf0f6  *               --------                            - ------------------ */
/*      ASM_                  0xf0f7  *               --------                            - ------------------ */
/*      ASM_                  0xf0f8  *               --------                            - ------------------ */
/*      ASM_                  0xf0f9  *               --------                            - ------------------ */
/*      ASM_                  0xf0fa  *               --------                            - ------------------ */
#define ASM16_STACK           0xf0fb /* `stack #<imm16>'                                  - Use an Object located on the stack as storage class.
                                      *                                                     The associated operand is an absolute offset from the stack's base. */
#define ASM16_STATIC          0xf0fc /* `static <imm16>'                                  - Same as `ASM_LOCAL16', but used to write to static variables. */
#define ASM16_EXTERN          0xf0fd /* `extern <imm16>:<imm16>'                          - Same as `ASM_LOCAL16', but used to write to extern variables. */
#define ASM16_GLOBAL          0xf0fe /* `global <imm16>'                                  - Same as `ASM_LOCAL16', but used to write to global variables. */
#define ASM16_LOCAL           0xf0ff /* `local  <imm16>'                                  - Same as `ASM_LOCAL', but encodes a little-endian 16-bit local id. */


#define INSTRLEN_MAX   12 /* `extern <imm16>:<imm16>: function const <imm16>, #<imm16>+1'. */



/* A second, smaller bytecode interpreter exists who's sole purpose is
 * to translate text addresses to source file/line/col/etc. information.
 * During assembly, this information is generated similarly to code found in
 * text sections, though it actually uses its own instructions and interpreter.
 * This subsystem is referred to as DDI (DeemonDebugInformation)
 * Since the main purpose of DDI instrumentation is to add constants to its
 * main registers `uip' (UserIP) and `lno' (LiNeOffset), all single-byte
 * instructions `>= DDI_GENERIC_START' behave as follows:
 * >> uip += DDI_GENERIC_IP(*ip);
 * >> lno += DDI_GENERIC_LN(*ip);
 * >> ip  += 1;
 * >> goto continue_interpretation;
 * The interpreter has the following registers (`int' referring to any arbitrarily precise integer):
 *   - unsigned int      uip;        // The current user instruction. (`code_addr_t')
 *   - unsigned int      path;       // The current path number. (NOTE: ZERO indicates no path and all other values
 *                                   //                                 are used as index-1 in the `d_path_names'
 *                                   //                                 vector of the associated `DeeDDIObject')
 *   - unsigned int      file;       // The current file number.
 *   - unsigned int      name;       // The current name number. (function name)
 *   - unsigned int      usp;        // The current stack alignment/depth at `uip'.
 *   - int               col;        // The current column number within the active line (0-based).
 *   - int               lno;        // Line number (0-based)
 *   - bool              isstmt;     // The current assembly address is part of a statement.
 *                                   // This flag should be used as a hint for debuggers
 *                                   // when setting breakpoints on a given line number,
 *                                   // in that checkpoints with this register set should
 *                                   // be preferred over ones without.
 *   - register_state    st_stack[]; // Stack of saved register states (s.a. `DDI_X_PUSHSTATE')
 *   - opt<unsigned int> sp_names[]; // Array of stack-variable names (The (likely) max-length is identical to `DeeCode_StackDepth(code)')
 *   - opt<unsigned int> lc_names[]; // Array of local variable names (The length of this vector is `code->co_localc')
 * To translate an instruction address to debug information, one must
 * interpret the DDI bytecode stream and perform the following:
 * >> uint8_t *code = get_ddi_stream();
 * >> old_state = cur_state;
 * >> for (;;) {
 * >>     uint8_t op = *code++;
 * >>     switch (op) {
 * >>     case DDI_STOP:
 * >>         return is_enumerating_code_points ? enumeration_done() : ip_no_found();
 * >>     
 * >>         ... // Handle other instructions.
 * >>             // Whenever any of them try to modify REG_PC, the following must be done:
 * >>             // >>
 * >>             // >> ... // Do other work done by the instruction.
 * >>             // >>
 * >>             // >> cur_state.uip = get_new_uip();
 * >>             // >> if (is_enumerating_code_points) {
 * >>             // >>     yield;
 * >>             // >> } else {
 * >>             // >>     if (old_state.uip <= DESIRED_IP && DESIRED_IP < cur_state.uip)
 * >>             // >>         return old_state; // The old state describes information about `DESIRED_IP'
 * >>             // >> }
 * >>     
 * >>     default:
 * >>         cur_state.uip += DDI_GENERIC_IP(op);
 * >>         cur_state.lno += DDI_GENERIC_LN(op);
 * >>check_ip:
 * >>         // Check if the desired ip was found
 * >>         // NOTE: This check must be performed every time `cur_state.uip' is modified
 * >>         if (is_enumerating_code_points) {
 * >>             yield;
 * >>         } else {
 * >>             if (old_state.uip <= DESIRED_IP && DESIRED_IP < cur_state.uip)
 * >>                 return old_state; // The old state describes information about `DESIRED_IP'
 * >>         }
 * >>         break;
 * >>     }
 * >> }
 * READ_SLEB():
 *    A function for reading a signed, arbitrary-precision integer from
 *    the input stream. (NOTE: `result' must be pre-initialized to `0')
 *    The integer is encoded as follows:
 *    BYTE[0]     & 0x40: If this bit is set, the final return value
 *                        must be negated: `final_result = -result';
 *                        This bit can only appear in the first byte.
 *    BYTE[0]     & 0x3f: Set the result value: `result = BYTE[0] & 0x3f'
 *    BYTE[x > 0] & 0x7f: Or' this mask to the result value: `result = result | ((BYTE[x] & 0x7f) << (x*7)-1)'
 *    BYTE[x]     & 0x80: If this bit is set, also parse `BYTE[x+1]'
 *                        If this bit is not set, stop parsing.
 * READ_ULEB():
 *    A function for reading an unsigned, arbitrary-precision integer from
 *    the input stream. (NOTE: `result' must be pre-initialized to `0')
 *    The integer is encoded as follows:
 *    BYTE[x] & 0x7f: Or this mask to the result value: `result = result | ((BYTE[x] & 0x7f) << x*7)'
 *    BYTE[x] & 0x80: If this bit is set, also parse `BYTE[x+1]'
 *                    If this bit is not set, stop parsing.
 * HINT: When analyzing DDI information, you should realize that there is
 *       no way for the UIP register to be decremented, meaning that
 *       enumerating check points always yields information for ascending
 *       code addresses, while also guarantying that no checkpoint can ever
 *       be equal to a previous one.
 * NOTE: DDI also contains instructions to encode the life-times, names
 *       and locations of stack-based and local variables, allowing the
 *       compiler to use whatever storage class it deems most efficient,
 *       without having to fear debug information ever degrading. */
#define DDI_STOP          0x00 /* SPECIAL: End of information stream. */
#define DDI_ADDUIP        0x01 /* `cur_state.uip += READ_ULEB()+1;' */
#define DDI_ADDLNO        0x02 /* `cur_state.lno += READ_SLEB();' */
#define DDI_SETCOL        0x03 /* `cur_state.col  = READ_SLEB();' */
#define DDI_SETPATH       0x04 /* `cur_state.path = READ_ULEB();' */
#define DDI_SETFILE       0x05 /* `cur_state.file = READ_ULEB();' */
#define DDI_SETNAME       0x06 /* `cur_state.name = READ_ULEB();' */
#define DDI_INCUSP        0x07 /* `++usp;' */
#define DDI_DECUSP        0x08 /* `sp_names[usp] = UNBOUND; --usp;' */
#define DDI_ADDUSP        0x09 /* `usp += READ_SLEB();' NOTE: When the read offset is negative, also unbind the SP-names of all affected slots. */
/*      DDI_              0x0a  *               --------                            - ------------------ */
/*      DDI_              0x0b  *               --------                            - ------------------ */
#define DDI_DEFSPNAME     0x0c /* sp_names[usp-1] = READ_ULEB(); */
#define DDI_DELLCNAME     0x0d /* x = READ_ULEB(); if (x >= code->co_localc) sp_names[x - code->co_localc] = UNBOUND; else lc_names[x] = UNBOUND; */
#define DDI_DEFLCNAME     0x0e /* x = READ_ULEB(); if (x >= code->co_localc) sp_names[x - code->co_localc] = READ_ULEB(); else lc_names[x] = READ_ULEB(); */

#define DDI_EXTENDED      0x0f /* x = READ_ULEB(); switch (x & DDI_X_CMDMASK) { ... }' (Switches to one of `DDI_X_*') */
#define DDI_X_CMDMASK     0xff /* Mask for the extended opcode (the remainder of the ULEB is used as argument for that opcode) */
#define DDI_X_TOGGLESTMT  0x01 /* `isstmt = !isstmt;' */
#define DDI_X_PUSHSTATE   0x10 /* Push the state of the `path', `file', `name', `col', `lno',
                                * `isstmt', `sp_names' and `lc_names' registers, and remember
                                * their values as the return location of an inline function call.
                                * With that in mind, their values should be used to construct
                                * an additional stack-frame in tracebacks.
                                * The saved state is restored by the accompanying `DDI_X_POPSTATE' instruction.
                                * An implementation that does not wish to support this feature is allowed
                                * to set an internal flag that prevents modifications to the above registers
                                * when this imaginary stack is non-empty. */
#define DDI_X_POPSTATE    0x11 /* Pop (restore) the top-most register state saved by `DDI_X_PUSHSTATE' */

#define DDI_GENERIC_START 0x10 /* First instruction that is executed in a generic context. */
#define DDI_MAXSHIFT      0xff /* A generic opcode with the greatest shift of both UIP and LNO. */

#define DDI_GENERIC_IP(x)   (((x)&0xf0) >> 4) /* Intentionally designed to be >= 1 (because `0x0*' is used for misc. opcodes)
                                               * This way, any generic instruction is _always_ a checkpoint! */
#define DDI_GENERIC_LN(x)   ((x)&0x0f)
#define DDI_GENERIC(ip, ln) (((ip) << 4)|(ln))

#define DDI_INSTRLEN_MAX  3 /* Technically, instructions can be larger than this,
                             * however this is used for the size of padding-data
                             * that is appended at the end of DEC-based DDI text
                             * in order to ensure proper text termination.
                             * And 2 consecutive zero-bytes can always terminate
                             * any SLEB/ULEB, while also defining the `DDI_STOP'
                             * instruction.
                             * The 3rd byte is there for instruction such as
                             * `DDI_DEFLCNAME', which define 2 ULEBs (3 == 2*ULEB+1*STOP). */


#endif /* !GUARD_DEEMON_ASM_H */
