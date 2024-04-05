/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_INSTRLEN_C
#define GUARD_DEEMON_COMPILER_INSTRLEN_C 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

/* Define instruction length lookup tables. */
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          PRIVATE uint8_t const instrlen_##table_prefix[256] = {
#define DEE_ASM_END(table_prefix)            };
#define DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic) instr_len,
#define DEE_ASM_PREFIX(table_prefix, opcode_byte, instr_len, name)                           instr_len,
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)                              instr_len,
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name)                         0x00,
#include <deemon/asm-table.h>


/* How to encode/decode stack effects */
#define STACK_EFFECT(down, up)  ((down) << 8 | (up))
#define STACK_EFFECT_UP(x)      ((x)&0xff)
#define STACK_EFFECT_DOWN(x)    (((x)&0xff00) >> 8)
#define STACK_EFFECT_MINSPECIAL 0xfffe
#define STACK_EFFECT_EXTENDED   0xfffe
#define STACK_EFFECT_PREFIX     0xffff

/* Define stack effect lookup tables (without prefix) */
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          PRIVATE uint16_t const stackeffect_##table_prefix[256] = {
#define DEE_ASM_END(table_prefix)            };
#define DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic) STACK_EFFECT(sp_sub, sp_add),
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)                              STACK_EFFECT(SP_SUB0, SP_ADD0),
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name)                         STACK_EFFECT_EXTENDED,
#define DEE_ASM_PREFIX(table_prefix, opcode_byte, instr_len, name)                           STACK_EFFECT_PREFIX,
#include <deemon/asm-table.h>

/* Define stack effect lookup tables (with prefix) */
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          PRIVATE uint16_t const stackeffect_p_##table_prefix[256] = {
#define DEE_ASM_END(table_prefix)            };
#define DEE_ASM_OPCODE_P(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic, prefix_sp_sub, prefix_sp_add, prefix_mnemonic) \
	STACK_EFFECT(prefix_sp_sub, prefix_sp_add),
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)      STACK_EFFECT(SP_SUB0, SP_ADD0),
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) STACK_EFFECT_EXTENDED,
#define DEE_ASM_PREFIX(table_prefix, opcode_byte, instr_len, name)   STACK_EFFECT_PREFIX,
#include <deemon/asm-table.h>


/* Return a pointer to the instruction following `pc' */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) instruction_t *DCALL
DeeAsm_NextInstr(instruction_t const *__restrict pc) {
	instruction_t opcode;
	uint8_t length;
again:
	opcode = *pc;
	length = instrlen_0x00[opcode];
	if unlikely(!length) {
		/* Extended instruction. */
		uint8_t const *instrlen_x;
		switch (opcode) {
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
		case opcode_byte:                                            \
			instrlen_x = instrlen_##opcode_byte;                     \
			break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
		default: __builtin_unreachable();
		}
		opcode = pc[1];
		length = instrlen_x[opcode];
		ASSERTF(length, "Nested extended instruction shouldn't exist");
		ASSERTF(length >= 2, "Extended instructions should always be at least 2 bytes long");
	}
	pc += length;
	if (ASM_ISPREFIX(opcode))
		goto again;
	return (instruction_t *)pc;
}


/* Skip over any prefix that may be found before an instruction (e.g. `ASM_LOCAL')
 * The returned pointer points to the first actual instruction byte.
 * When no prefix is present, simply re-return `pc' */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_instruction_t *DCALL
DeeAsm_SkipPrefix(Dee_instruction_t const *__restrict pc) {
	instruction_t opcode;
again:
	opcode = *pc;
	if (ASM_ISPREFIX(opcode)) {
		pc += instrlen_0x00[opcode];
		goto again;
	}
	if (instrlen_0x00[opcode] == 0) {
		if (ASM_ISPREFIX(pc[1])) {
			uint8_t const *instrlen_x;
			switch (opcode) {
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
			case opcode_byte:                                        \
				instrlen_x = instrlen_##opcode_byte;                 \
				break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
			default: __builtin_unreachable();
			}
			opcode = pc[1];
			pc += instrlen_x[opcode];
			goto again;
		}
	}
	return (instruction_t *)pc;
}



/* Return if the given `instr' doesn't return normally (i.e. doesn't fall
 * through to its successor instruction) when executed in a context where
 * `code_flags' (which is the set of CODE_F* flags) is active. */
PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeAsm_IsNoreturn(uint16_t instr, uint16_t code_flags) {
	bool result = false;
	switch (instr) {

	case ASM_RET:
		/* In yielding code, the `ret' instruction does actually
		 * return, since it would actually be the `yield' instruction. */
		if (code_flags & CODE_FYIELDING)
			break;
		ATTR_FALLTHROUGH
	case ASM_RET_NONE:
	case ASM_THROW:
	case ASM_RETHROW:
	case ASM_JMP:
	case ASM_JMP16:
	case ASM32_JMP:
	case ASM_JMP_POP:
	case ASM_JMP_POP_POP:
		result = true;
		break;

	default: break;
	}
	return result;
}

/* Same as `DeeAsm_NextInstr()', but also keep track of the current stack depth.
 * NOTE:    The affect of branch instructions is evaluated as the
 *          fall-through path (aka. when the branch isn't taken).
 * WARNING: This also goes for instructions that always take a branch! */
PUBLIC ATTR_RETNONNULL NONNULL((1, 2)) Dee_instruction_t *DCALL
DeeAsm_NextInstrSp(Dee_instruction_t const *__restrict pc,
                   /*in|out*/ uint16_t *__restrict p_stacksz) {
	uint16_t sp_add, sp_sub;
	return DeeAsm_NextInstrEf(pc, p_stacksz, &sp_add, &sp_sub);
}

/* Same as `DeeAsm_NextInstr', but also returns the effective stack effect (sub/add) of the instruction.
 * This function is used by the peephole optimizer to trace usage of objects stored on the stack.
 * NOTES:
 *   - Because some instruction's stack effect depends on the current stack depth.
 *     That may sound weird, but think about how fixed-depth instructions behave,
 *     this function also keeps track of the current stack depth. (e.g.: ASM_STACK)
 *   - Since some instructions exist who's stack-effect depends on parameters on
 *     known at runtime (e.g.: ASM_JMP_POP_POP), those instructions have an effective
 *     stack-effect of 0, which sub/add effect addends that maximize the potential
 *     influence (e.g.: `ASM_JMP_POP_POP': `*p_sp_sub = (*p_sp_sub = *p_stacksz)+2, *p_stacksz -= 2;')
 *   - Before returning, `*p_stacksz' will be adjusted to `(OLD(*p_stacksz) - *p_sp_sub) + *p_sp_add' */
PUBLIC ATTR_RETNONNULL NONNULL((1, 2, 3, 4)) Dee_instruction_t *DCALL
DeeAsm_NextInstrEf(Dee_instruction_t const *__restrict pc,
                   /*in|out*/ uint16_t *__restrict p_stacksz,
                   /*out*/ uint16_t *__restrict p_sp_add,
                   /*out*/ uint16_t *__restrict p_sp_sub) {
	uint8_t sp_sub_action, sp_add_action;
	Dee_instruction_t opcode = *pc;
	uint16_t effect = stackeffect_0x00[opcode];
	if (effect >= STACK_EFFECT_MINSPECIAL) {
		if (effect == STACK_EFFECT_EXTENDED) {
			uint16_t const *stackeffect_x;
			switch (opcode) {
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
			case opcode_byte:                                        \
				stackeffect_x = stackeffect_##opcode_byte;           \
				break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
			default: __builtin_unreachable();
			}
			opcode = pc[1];
			effect = stackeffect_x[opcode];
			ASSERT(effect != STACK_EFFECT_EXTENDED);
		}
		if (effect == STACK_EFFECT_PREFIX) {
			if (opcode == ASM_STACK) {
				/* Special case: minimal stack effect delta. */
				uint16_t sp_addr, min_sp_sub;
				if (pc[0] == ASM_STACK) {
					sp_addr = UNALIGNED_GETLE8(pc + 1);
					pc += 2;
				} else {
					sp_addr = UNALIGNED_GETLE16(pc + 2);
					pc += 4;
				}
				min_sp_sub = *p_stacksz - sp_addr;
				pc = DeeAsm_NextInstrEf(pc, p_stacksz, p_sp_add, p_sp_sub);
				if (min_sp_sub > *p_sp_sub) {
					uint16_t extra = min_sp_sub - *p_sp_sub;
					*p_sp_sub += extra;
					*p_sp_add += extra;
				}
				return (instruction_t *)pc;
			}

			/* Skip prefix and inspect stack effect. */
			pc = DeeAsm_SkipPrefix(pc);
			opcode = *pc;
			effect = stackeffect_p_0x00[opcode];
			if (effect >= STACK_EFFECT_MINSPECIAL) {
				uint16_t const *stackeffect_p_x;
				ASSERT(effect == STACK_EFFECT_EXTENDED);
				switch (opcode) {
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
				case opcode_byte:                                    \
					stackeffect_p_x = stackeffect_p_##opcode_byte;   \
					break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
				default: __builtin_unreachable();
				}
				opcode = pc[1];
				effect = stackeffect_p_x[opcode];
				ASSERT(effect < STACK_EFFECT_MINSPECIAL);
			}
		}
	}

	sp_sub_action = STACK_EFFECT_DOWN(effect);
	sp_add_action = STACK_EFFECT_UP(effect);
	ASSERT(sp_sub_action < SP_SUB_COUNT);
	ASSERT(sp_add_action < SP_ADD_COUNT);
	switch (sp_sub_action) {
#define DEE_SP_SUB(code, expr) \
	case code:                 \
		*p_sp_sub = expr;      \
		break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
	default: __builtin_unreachable();
	}
	switch (sp_add_action) {
#define DEE_SP_ADD(code, expr) \
	case code:                 \
		*p_sp_add = expr;      \
		break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
	default: __builtin_unreachable();
	}
	*p_stacksz -= *p_sp_sub;
	*p_stacksz += *p_sp_add;
	return DeeAsm_NextInstr(pc);
}

PRIVATE unsigned int DCALL
prefix_symbol_usage(instruction_t const *__restrict pc) {
	unsigned int result;
	switch (pc[0]) {

	case ASM_ADD:
	case ASM_SUB:
	case ASM_MUL:
	case ASM_DIV:
	case ASM_MOD:
	case ASM_SHL:
	case ASM_SHR:
	case ASM_AND:
	case ASM_OR:
	case ASM_XOR:
	case ASM_POW:
	case ASM_INC:
	case ASM_DEC:
	case ASM_OPERATOR:
	case ASM_OPERATOR_TUPLE:
		/* Inplace operations read & write. */
		result = ASM_USING_READ | ASM_USING_WRITE;
		break;

	case ASM_FUNCTION_C:
	case ASM_FUNCTION_C_16:
		/* These only write to the symbol. */
		result = ASM_USING_WRITE;
		break;

		/* All the new mov-style prefix instruction */
	case ASM_DUP:
	case ASM_DUP_N:
	case ASM_PUSH_NONE:
	case ASM_PUSH_MODULE:
	case ASM_PUSH_REF:
	case ASM_PUSH_ARG:
	case ASM_PUSH_CONST:
	case ASM_PUSH_STATIC:
	case ASM_PUSH_EXTERN:
	case ASM_PUSH_GLOBAL:
	case ASM_PUSH_LOCAL:
		result = ASM_USING_WRITE;
		break;

	case ASM_POP:
	case ASM_POP_N:
	case ASM_POP_STATIC:
	case ASM_POP_EXTERN:
	case ASM_POP_GLOBAL:
	case ASM_POP_LOCAL:
	case ASM_RET:
	case ASM_YIELDALL:
	case ASM_THROW:
	case ASM_JT:
	case ASM_JT16:
	case ASM_JF:
	case ASM_JF16:
	case ASM_FOREACH:
	case ASM_FOREACH16:
		result = ASM_USING_READ;
		break;

	case ASM_EXTENDED1:
		switch (pc[1]) {

		case ASM_INCPOST & 0xff:
		case ASM_DECPOST & 0xff:
		case ASM16_OPERATOR & 0xff:
		case ASM16_OPERATOR_TUPLE & 0xff:
		case ASM_CMPXCH_UB_LOCK & 0xff:
		case ASM_CMPXCH_UB_POP & 0xff:
		case ASM_CMPXCH_POP_UB & 0xff:
		case ASM_CMPXCH_POP_POP & 0xff:
			result = ASM_USING_READ | ASM_USING_WRITE;
			break;

		case ASM16_FUNCTION_C & 0xff:
		case ASM16_FUNCTION_C_16 & 0xff:
			/* These only write to the symbol. */
			result = ASM_USING_WRITE;
			break;

			/* All the new mov-style prefix instruction */
		case ASM16_DUP_N & 0xff:
		case ASM16_PUSH_MODULE & 0xff:
		case ASM16_PUSH_REF & 0xff:
		case ASM16_PUSH_ARG & 0xff:
		case ASM16_PUSH_CONST & 0xff:
		case ASM16_PUSH_STATIC & 0xff:
		case ASM16_PUSH_EXTERN & 0xff:
		case ASM16_PUSH_GLOBAL & 0xff:
		case ASM16_PUSH_LOCAL & 0xff:
		case ASM_PUSH_EXCEPT & 0xff:
		case ASM_PUSH_THIS & 0xff:
		case ASM_PUSH_THIS_MODULE & 0xff:
		case ASM_PUSH_THIS_FUNCTION & 0xff:
		case ASM_PUSH_TRUE & 0xff:
		case ASM_PUSH_FALSE & 0xff:
			result = ASM_USING_WRITE;
			break;

		case ASM16_POP_N & 0xff:
		case ASM16_POP_STATIC & 0xff:
		case ASM16_POP_EXTERN & 0xff:
		case ASM16_POP_GLOBAL & 0xff:
		case ASM16_POP_LOCAL & 0xff:
			result = ASM_USING_READ;
			break;

		default:
			result = ASM_USING_READ | ASM_USING_WRITE;
			break;
		}
		break;

	default:
		result = ASM_USING_READ | ASM_USING_WRITE;
		break;
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) unsigned int DCALL
asm_uses_local(instruction_t const *__restrict pc, uint16_t lid) {
	unsigned int result = 0;
	switch (pc[0]) {

	case ASM_LOCAL:
		if (UNALIGNED_GETLE8(pc + 1) == lid)
			result = prefix_symbol_usage(pc + 2);
		pc += 2;
		goto check_prefix_core_usage;

	case ASM_PUSH_BND_LOCAL:
	case ASM_PUSH_LOCAL:
	case ASM_CALL_LOCAL:
		if (UNALIGNED_GETLE8(pc + 1) != lid)
			break;
		result = ASM_USING_READ;
		break;

	case ASM_DEL_LOCAL:
	case ASM_POP_LOCAL:
		if (UNALIGNED_GETLE8(pc + 1) != lid)
			break;
		result = ASM_USING_WRITE;
		break;

	case ASM_EXTENDED1:
		switch (pc[1]) {

		case ASM16_PUSH_BND_LOCAL & 0xff:
		case ASM16_PUSH_LOCAL & 0xff:
		case ASM16_CALL_LOCAL & 0xff:
			if (UNALIGNED_GETLE16(pc + 2) != lid)
				break;
			result = ASM_USING_READ;
			break;

		case ASM16_DEL_LOCAL & 0xff:
		case ASM16_POP_LOCAL & 0xff:
			if (UNALIGNED_GETLE16(pc + 2) != lid)
				break;
			result = ASM_USING_WRITE;
			break;

		case ASM16_LOCAL & 0xff:
			if (UNALIGNED_GETLE16(pc + 2) == lid)
				result = prefix_symbol_usage(pc + 4);
			pc += 4;
			goto check_prefix_core_usage;

		case ASM16_STACK & 0xff:
		case ASM16_STATIC & 0xff:
		case ASM16_EXTERN & 0xff:
		case ASM16_GLOBAL & 0xff:
			pc += 4;
			goto check_prefix_core_usage;

		default: break;
		}
		break;

	case ASM_STACK:
	case ASM_STATIC:
	case ASM_EXTERN:
	case ASM_GLOBAL:
check_prefix_core_usage:
		switch (*pc) {

		case ASM_PUSH_LOCAL: /* mov PREFIX, local */
			if (UNALIGNED_GETLE8(pc + 1) == lid)
				result |= ASM_USING_READ;
			break;

		case ASM_POP_LOCAL: /* mov local, PREFIX */
			if (UNALIGNED_GETLE8(pc + 1) == lid)
				result |= ASM_USING_WRITE;
			break;

		case ASM_EXTENDED1:
			switch (pc[1]) {

			case ASM16_PUSH_LOCAL & 0xff: /* mov PREFIX, local */
				if (UNALIGNED_GETLE16(pc + 2) == lid)
					result |= ASM_USING_READ;
				break;

			case ASM16_POP_LOCAL & 0xff: /* mov local, PREFIX */
				if (UNALIGNED_GETLE16(pc + 2) == lid)
					result |= ASM_USING_WRITE;
				break;

			default: break;
			}
			break;

		default: break;
		}
		break;

	default: break;
	}
	return result;
}



INTERN WUNUSED NONNULL((1)) unsigned int DCALL
asm_uses_static(instruction_t const *__restrict pc, uint16_t sid) {
	/* NOTE: This function also allows for tracking of constant variable usage. */
	unsigned int result = 0;
	switch (pc[0]) {

	case ASM_STATIC:
		if (UNALIGNED_GETLE8(pc + 1) != sid)
			break;
		result = prefix_symbol_usage(pc + 2);
		result |= asm_uses_static(pc + 2, sid);
		pc += 2;
		goto check_prefix_core_usage;

	case ASM_POP_STATIC:
		if (UNALIGNED_GETLE8(pc + 1) != sid)
			break;
		result = ASM_USING_WRITE;
		break;

	case ASM_PUSH_STATIC:
	case ASM_PUSH_CONST:
	case ASM_GETATTR_C:
	case ASM_DELATTR_C:
	case ASM_SETATTR_C:
	case ASM_GETATTR_THIS_C:
	case ASM_DELATTR_THIS_C:
	case ASM_SETATTR_THIS_C:
	case ASM_FUNCTION_C:
	case ASM_FUNCTION_C_16:
	case ASM_PRINT_C:
	case ASM_PRINT_C_SP:
	case ASM_PRINT_C_NL:
	case ASM_FPRINT_C:
	case ASM_FPRINT_C_SP:
	case ASM_FPRINT_C_NL:
	case ASM_GETITEM_C:
	case ASM_SETITEM_C:
	case ASM_CALLATTR_C:
	case ASM_CALLATTR_C_MAP:
	case ASM_CALLATTR_C_SEQ:
	case ASM_CALLATTR_C_TUPLE:
	case ASM_CALLATTR_THIS_C:
	case ASM_CALLATTR_THIS_C_TUPLE:
		if (UNALIGNED_GETLE8(pc + 1) != sid)
			break;
		result = ASM_USING_READ;
		break;

	case ASM_CLASS_C:
		if (UNALIGNED_GETLE8(pc + 1) == sid) {
			result = ASM_USING_READ;
			break;
		}
		break;

	case ASM_CLASS_GC:
		if (UNALIGNED_GETLE8(pc + 2) == sid) {
			result = ASM_USING_READ;
			break;
		}
		break;

	case ASM_CLASS_EC:
		if (UNALIGNED_GETLE8(pc + 3) == sid) {
			result = ASM_USING_READ;
			break;
		}
		break;

	case ASM_EXTENDED1:
		switch (pc[1]) {

		case ASM16_CLASS_C & 0xff:
			if (UNALIGNED_GETLE16(pc + 2) == sid) {
				result = ASM_USING_READ;
				break;
			}
			break;

		case ASM16_CLASS_GC & 0xff:
			if (UNALIGNED_GETLE16(pc + 4) == sid) {
				result = ASM_USING_READ;
				break;
			}
			break;

		case ASM16_CLASS_EC & 0xff:
			if (UNALIGNED_GETLE16(pc + 6) == sid) {
				result = ASM_USING_READ;
				break;
			}
			break;

		case ASM16_STATIC & 0xff:
			if (UNALIGNED_GETLE16(pc + 2) != sid)
				break;
			result = prefix_symbol_usage(pc + 4);
			result |= asm_uses_static(pc + 4, sid);
			pc += 4;
			goto check_prefix_core_usage;

		case ASM16_POP_STATIC & 0xff:
			if (UNALIGNED_GETLE16(pc + 2) != sid)
				break;
			result = ASM_USING_WRITE;
			break;

		case ASM16_PUSH_STATIC & 0xff:
		case ASM16_PUSH_CONST & 0xff:
		case ASM16_GETATTR_C & 0xff:
		case ASM16_DELATTR_C & 0xff:
		case ASM16_SETATTR_C & 0xff:
		case ASM16_GETATTR_THIS_C & 0xff:
		case ASM16_DELATTR_THIS_C & 0xff:
		case ASM16_SETATTR_THIS_C & 0xff:
		case ASM16_FUNCTION_C & 0xff:
		case ASM16_FUNCTION_C_16 & 0xff:
		case ASM16_PRINT_C & 0xff:
		case ASM16_PRINT_C_SP & 0xff:
		case ASM16_PRINT_C_NL & 0xff:
		case ASM16_FPRINT_C & 0xff:
		case ASM16_FPRINT_C_SP & 0xff:
		case ASM16_FPRINT_C_NL & 0xff:
		case ASM16_GETITEM_C & 0xff:
		case ASM16_SETITEM_C & 0xff:
		case ASM16_CALLATTR_C & 0xff:
		case ASM16_CALLATTR_C_MAP & 0xff:
		case ASM16_CALLATTR_C_SEQ & 0xff:
		case ASM16_CALLATTR_C_TUPLE & 0xff:
		case ASM16_CALLATTR_THIS_C & 0xff:
		case ASM16_CALLATTR_THIS_C_TUPLE & 0xff:
			if (UNALIGNED_GETLE16(pc + 2) != sid)
				break;
			result = ASM_USING_READ;
			break;

		case ASM16_STACK & 0xff:
		case ASM16_LOCAL & 0xff:
		case ASM16_EXTERN & 0xff:
		case ASM16_GLOBAL & 0xff:
			pc += 4;
			goto check_prefix_core_usage;

		default: break;
		}
		break;

	case ASM_STACK:
	case ASM_LOCAL:
	case ASM_EXTERN:
	case ASM_GLOBAL:
check_prefix_core_usage:
		switch (*pc) {

		case ASM_PUSH_STATIC: /* mov PREFIX, static */
		case ASM_PUSH_CONST:  /* mov PREFIX, const */
			if (UNALIGNED_GETLE8(pc + 1) == sid)
				result |= ASM_USING_READ;
			break;
		case ASM_POP_STATIC: /* mov static, PREFIX */
			if (UNALIGNED_GETLE8(pc + 1) == sid)
				result |= ASM_USING_WRITE;
			break;

		case ASM_EXTENDED1:
			switch (pc[1]) {

			case ASM16_PUSH_STATIC & 0xff: /* mov PREFIX, static */
			case ASM16_PUSH_CONST & 0xff:  /* mov PREFIX, const */
				if (UNALIGNED_GETLE16(pc + 2) == sid)
					result |= ASM_USING_READ;
				break;
			case ASM16_POP_STATIC & 0xff: /* mov local, PREFIX */
				if (UNALIGNED_GETLE16(pc + 2) == sid)
					result |= ASM_USING_WRITE;
				break;

			default: break;
			}
			break;

		default: break;
		}
		break;

	default: break;
	}
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INSTRLEN_C */
