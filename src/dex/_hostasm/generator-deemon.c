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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_DEEMON_C
#define GUARD_DEX_HOSTASM_GENERATOR_DEEMON_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

/************************************************************************/
/* DEEMON TO VSTACK MICRO-OP TRANSFORMATION                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL /* this/addrof(this), opname, args */
Dee_function_generator_vcall_DeeObject_InvokeOperatorTuple(struct Dee_function_generator *__restrict self,
                                                           void const *api_function) {
	struct Dee_memloc args_tuple;
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err; /* this/addrof(this), opname, reg:args */
	args_tuple = *Dee_function_generator_vtop(self);
	ASSERT(args_tuple.ml_type == MEMLOC_TYPE_HREG);
	if unlikely(Dee_function_generator_vpush_regind(self,
	                                                args_tuple.ml_value.v_hreg.r_regno,
	                                                args_tuple.ml_value.v_hreg.r_off +
	                                                offsetof(DeeTupleObject, t_size),
	                                                0))
		goto err; /* this/addrof(this), opname, args, DeeTuple_SIZE(args) */
	if unlikely(Dee_function_generator_vpush_reg(self,
	                                             args_tuple.ml_value.v_hreg.r_regno,
	                                             args_tuple.ml_value.v_hreg.r_off + offsetof(DeeTupleObject, t_elem)))
		goto err; /* this/addrof(this), opname, args, DeeTuple_SIZE(args), DeeTuple_ELEM(args) */
	if unlikely(Dee_function_generator_vlrot(self, 3))
		goto err; /* this/addrof(this), opname, DeeTuple_SIZE(args), DeeTuple_ELEM(args), args */
	if unlikely(Dee_function_generator_vrrot(self, 5))
		goto err; /* args, this/addrof(this), opname, DeeTuple_SIZE(args), DeeTuple_ELEM(args) */
	if unlikely(Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_OBJECT, 4))
		goto err; /* args, result */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* result, args */
	return Dee_function_generator_vpop(self);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_prefix(struct Dee_function_generator *__restrict self,
                                    uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	switch (prefix_type) {
	case ASM_STACK:
		return Dee_function_generator_vdup_n(self, self->fg_state->ms_stackc - id1);
	case ASM_STATIC:
		return Dee_function_generator_vpush_static(self, id1);
	case ASM_EXTERN:
		return Dee_function_generator_vpush_extern(self, id1, id2);
	case ASM_GLOBAL:
		return Dee_function_generator_vpush_global(self, id1);
	case ASM_LOCAL:
		return Dee_function_generator_vpush_ulocal(self, id1);
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_prefix(struct Dee_function_generator *__restrict self,
                                   uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	switch (prefix_type) {
	case ASM_STACK:
		return Dee_function_generator_vpop_n(self, self->fg_state->ms_stackc - id1);
	case ASM_STATIC:
		return Dee_function_generator_vpop_static(self, id1);
	case ASM_EXTERN:
		return Dee_function_generator_vpop_extern(self, id1, id2);
	case ASM_GLOBAL:
		return Dee_function_generator_vpop_global(self, id1);
	case ASM_LOCAL:
		return Dee_function_generator_vpop_ulocal(self, id1);
	default: __builtin_unreachable();
	}
}

/* If possible, push a `DREF DeeObject **' for the prefix (only for STACK/LOCALS)
 * @return: 1 : Prefix doesn't support direct addressing
 * @return: 0 : Success 
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_prefix_addr(struct Dee_function_generator *__restrict self,
                                         uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	uintptr_t cfa_offset;
	struct Dee_memloc *src_loc;
	struct Dee_memstate *state;
	(void)id2;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	switch (prefix_type) {
	case ASM_STACK:
		ASSERTF(id1 < state->ms_stackc, "Should have been checked by the caller");
		src_loc = &state->ms_stackv[id1];
		break;
	case ASM_LOCAL:
		ASSERTF(id1 < state->ms_localc, "Should have been checked by the caller");
		src_loc = &state->ms_localv[id1];
		break;
	default:
		/* Direct addressing not support for this type of prefix. */
		return 1;
	}
	if (src_loc->ml_flags & MEMLOC_F_NOREF) {
		if unlikely(Dee_function_generator_gincref(self, src_loc))
			goto err;
		src_loc->ml_flags &= ~MEMLOC_F_NOREF;
	}
	Dee_memstate_verifyrinuse(self->fg_state);
	if unlikely(Dee_function_generator_gflush(self, src_loc))
		goto err;
	Dee_memstate_verifyrinuse(self->fg_state);
	ASSERT(!(src_loc->ml_flags & MEMLOC_F_NOREF));
	ASSERT(src_loc->ml_type == MEMLOC_TYPE_HSTACKIND);
	ASSERT(src_loc->ml_value.v_hstack.s_off == 0);
	cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
#ifndef HOSTASM_STACK_GROWS_DOWN
	cfa_offset -= HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	return Dee_memstate_vpush_hstack(state, cfa_offset);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
perform_inplace_math_operation_with_vtop_addr(struct Dee_function_generator *__restrict self,
                                              uint16_t prefix_opcode, Dee_instruction_t const *prefix_instr) {
	switch (prefix_opcode) {
	case ASM_ADD: /* add PREFIX, pop */
	case ASM_SUB: /* sub PREFIX, pop */
	case ASM_MUL: /* mul PREFIX, pop */
	case ASM_DIV: /* div PREFIX, pop */
	case ASM_MOD: /* mod PREFIX, pop */
	case ASM_SHL: /* shl PREFIX, pop */
	case ASM_SHR: /* shr PREFIX, pop */
	case ASM_AND: /* and PREFIX, pop */
	case ASM_OR:  /* or  PREFIX, pop */
	case ASM_XOR: /* xor PREFIX, pop */
	case ASM_POW: /* pow PREFIX, pop */
		switch (prefix_opcode) {
		case ASM_ADD: /* add PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceAdd, VCALLOP_CC_INT, 2);
		case ASM_SUB: /* sub PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceSub, VCALLOP_CC_INT, 2);
		case ASM_MUL: /* mul PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceMul, VCALLOP_CC_INT, 2);
		case ASM_DIV: /* div PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceDiv, VCALLOP_CC_INT, 2);
		case ASM_MOD: /* mod PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceMod, VCALLOP_CC_INT, 2);
		case ASM_SHL: /* shl PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceShl, VCALLOP_CC_INT, 2);
		case ASM_SHR: /* shr PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceShr, VCALLOP_CC_INT, 2);
		case ASM_AND: /* and PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceAnd, VCALLOP_CC_INT, 2);
		case ASM_OR:  /* or  PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceOr, VCALLOP_CC_INT, 2);
		case ASM_XOR: /* xor PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceXor, VCALLOP_CC_INT, 2);
		case ASM_POW: /* pow PREFIX, pop */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplacePow, VCALLOP_CC_INT, 2);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();

	case ASM_INC: /* inc PREFIX */
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_Inc, VCALLOP_CC_INT, 1);
	case ASM_DEC: /* dec PREFIX */
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_Dec, VCALLOP_CC_INT, 1);

	case ASM_ADD_SIMM8:   /* add PREFIX, $<Simm8> */
	case ASM_SUB_SIMM8:   /* sub PREFIX, $<Simm8> */
	case ASM_MUL_SIMM8:   /* mul PREFIX, $<Simm8> */
	case ASM_DIV_SIMM8:   /* div PREFIX, $<Simm8> */
	case ASM_MOD_SIMM8: { /* mod PREFIX, $<Simm8> */
		int8_t Simm8 = (int8_t)prefix_instr[1];
		if unlikely(Dee_function_generator_vpush_Simm8(self, Simm8))
			goto err;
		switch (prefix_opcode) {
		case ASM_ADD_SIMM8: /* add PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceAddInt8, VCALLOP_CC_INT, 2);
		case ASM_SUB_SIMM8: /* sub PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceSubInt8, VCALLOP_CC_INT, 2);
		case ASM_MUL_SIMM8: /* mul PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceMulInt8, VCALLOP_CC_INT, 2);
		case ASM_DIV_SIMM8: /* div PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceDivInt8, VCALLOP_CC_INT, 2);
		case ASM_MOD_SIMM8: /* mod PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceModInt8, VCALLOP_CC_INT, 2);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	}	break;

	case ASM_SHL_IMM8:   /* shl PREFIX, $<imm8> */
	case ASM_SHR_IMM8: { /* shr PREFIX, $<imm8> */
		uint8_t imm8 = (uint8_t)prefix_instr[1];
		if unlikely(Dee_function_generator_vpush_imm8(self, imm8))
			goto err;
		switch (prefix_opcode) {
		case ASM_SHL_IMM8: /* shl PREFIX, $<imm8> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceShlUInt8, VCALLOP_CC_INT, 2);
		case ASM_SHR_IMM8: /* shr PREFIX, $<imm8> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceShrUInt8, VCALLOP_CC_INT, 2);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	}	break;

	case ASM_ADD_IMM32:   /* add PREFIX, $<imm32> */
	case ASM_SUB_IMM32:   /* sub PREFIX, $<imm32> */
	case ASM_AND_IMM32:   /* and PREFIX, $<imm32> */
	case ASM_OR_IMM32:    /* or  PREFIX, $<imm32> */
	case ASM_XOR_IMM32: { /* xor PREFIX, $<imm32> */
		uint32_t imm32 = (uint32_t)UNALIGNED_GETLE32(prefix_instr + 1);
		if unlikely(Dee_function_generator_vpush_imm32(self, imm32))
			goto err;
		switch (prefix_opcode) {
		case ASM_ADD_IMM32: /* add PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceAddUInt32, VCALLOP_CC_INT, 2);
		case ASM_SUB_IMM32: /* sub PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceSubUInt32, VCALLOP_CC_INT, 2);
		case ASM_AND_IMM32: /* and PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceAndUInt32, VCALLOP_CC_INT, 2);
		case ASM_OR_IMM32:  /* or  PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceOrUInt32, VCALLOP_CC_INT, 2);
		case ASM_XOR_IMM32: /* xor PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_InplaceXorUInt32, VCALLOP_CC_INT, 2);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	}	break;

	default: __builtin_unreachable();
	}
	__builtin_unreachable();
err:
	return -1;
}



/* Max # of non-print-to-stdout instructions before the stdout cache is invalidated. */
#ifndef CONFIG_HOSTASM_STDOUT_CACHE_MAXINSTR_N
#define CONFIG_HOSTASM_STDOUT_CACHE_MAXINSTR_N 5
#endif /* !CONFIG_HOSTASM_STDOUT_CACHE_MAXINSTR_N */

#define CASE_ASM_PRINT     \
	case ASM_PRINT:        \
	case ASM_PRINT_SP:     \
	case ASM_PRINT_NL:     \
	case ASM_PRINTNL:      \
	case ASM_PRINTALL:     \
	case ASM_PRINTALL_SP:  \
	case ASM_PRINTALL_NL:  \
	case ASM_PRINT_C:      \
	case ASM16_PRINT_C:    \
	case ASM_PRINT_C_SP:   \
	case ASM16_PRINT_C_SP: \
	case ASM_PRINT_C_NL:   \
	case ASM16_PRINT_C_NL

#define CASE_ASM_PRINT_AFTER_REPR \
	case ASM_PRINT:               \
	case ASM_PRINT_SP:            \
	case ASM_PRINT_NL


/* Generate code for a print-to-stdout instruction. The caller has left stdout in vtop.
 * It is assumed that this function will pop the caller-pushed stdout slot. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gen_print_with_stdout_in_vtop(struct Dee_function_generator *__restrict self,
                              Dee_instruction_t const *instr, uint16_t opcode,
                              bool repr) {
	void const *api_function;
	switch (opcode) {

	case ASM_PRINTNL:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeFile_PrintNl, VCALLOP_CC_INT, 1);

	case ASM_PRINT:
	case ASM_PRINT_SP:
	case ASM_PRINT_NL:
	case ASM_PRINTALL:
	case ASM_PRINTALL_SP:
	case ASM_PRINTALL_NL:
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* File, value */
		switch (opcode) {
		case ASM_PRINT:
			api_function = repr ? (void const *)&DeeFile_PrintObjectRepr
			                    : (void const *)&DeeFile_PrintObject;
			break;
		case ASM_PRINT_SP:
			api_function = repr ? (void const *)&DeeFile_PrintObjectReprSp
			                    : (void const *)&DeeFile_PrintObjectSp;
			break;
		case ASM_PRINT_NL:
			api_function = repr ? (void const *)&DeeFile_PrintObjectReprNl
			                    : (void const *)&DeeFile_PrintObjectNl;
			break;
		case ASM_PRINTALL:
			api_function = (void const *)&DeeFile_PrintAll;
			break;
		case ASM_PRINTALL_SP:
			api_function = (void const *)&DeeFile_PrintAllSp;
			break;
		case ASM_PRINTALL_NL:
			api_function = (void const *)&DeeFile_PrintAllNl;
			break;
		default: __builtin_unreachable();
		}
		break;

	case ASM_PRINT_C:
	case ASM16_PRINT_C:
	case ASM_PRINT_C_SP:
	case ASM16_PRINT_C_SP:
	case ASM_PRINT_C_NL:
	case ASM16_PRINT_C_NL: {
		DeeObject *constant;
		uint16_t cid = instr[1];
		if (opcode > 0xff)
			cid = UNALIGNED_GETLE16(instr + 2);
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err; /* File, value */
		switch (opcode) {
		case ASM_PRINT_C:
		case ASM16_PRINT_C:
			api_function = (void const *)&DeeFile_PrintObject;
			break;
		case ASM_PRINT_C_SP:
		case ASM16_PRINT_C_SP:
			api_function = (void const *)&DeeFile_PrintObjectSp;
			break;
		case ASM_PRINT_C_NL:
		case ASM16_PRINT_C_NL:
			api_function = (void const *)&DeeFile_PrintObjectNl;
			break;
		default: __builtin_unreachable();
		}
	}	break;

	default: __builtin_unreachable();
	}
	return Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_INT, 2);
err:
	return -1;
}

/* For the non-file-print instructions, load the `stdout' file stream only the first
 * time a print-like instruction is reached. We then keep that object in a hidden
 * local `DEE_MEMSTATE_EXTRA_LOCALS_STDOUT' until the end of the basic block, or until
 * a chunk of at least `CONFIG_HOSTASM_STDOUT_CACHE_MAXINSTR_N' non-print-to-stdout
 * instructions are about to be compiled.
 *
 * NOTE: Technically, this alters the behavior of instructions, since the
 *       native version of these (re-)loads `deemon.File.stdout' every time
 *       they are evaluated, but that seems quite overkill if you ask me. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gen_stdout_print(struct Dee_function_generator *__restrict self,
                                        Dee_instruction_t const *instr,
                                        Dee_instruction_t const **p_next_instr,
                                        bool repr) {
	size_t stdout_lid;
	Dee_instruction_t const *iter, *print_block_end, *print_block_end_limit;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	stdout_lid = (size_t)self->fg_assembler->fa_code->co_localc + DEE_MEMSTATE_EXTRA_LOCALS_STDOUT;

	/* Figure out where we want to end the print-block. */
	print_block_end_limit = self->fg_block->bb_deemon_end;
	print_block_end       = *p_next_instr;
#if CONFIG_HOSTASM_STDOUT_CACHE_MAXINSTR_N > 0
	{
		size_t non_print_break_size = 0;
		iter = print_block_end;
		while (iter < print_block_end_limit &&
		       non_print_break_size < CONFIG_HOSTASM_STDOUT_CACHE_MAXINSTR_N) {
			Dee_instruction_t const *next = DeeAsm_NextInstr(iter);
			uint16_t opcode = iter[0];
			if (ASM_ISEXTENDED(opcode))
				opcode = (opcode << 8) | iter[1];
			switch (opcode) {
			CASE_ASM_PRINT:
				print_block_end = next;
				non_print_break_size = 0;
				break;
			default:
				++non_print_break_size;
				break;
			}
			iter = next;
		}
	}
#endif /* CONFIG_HOSTASM_STDOUT_CACHE_MAXINSTR_N > 0 */

	/* If the stdout lid hasn't been loaded yet, do so now.
	 * Note that it should always be not-yet-loaded! */
	ASSERT(stdout_lid < self->fg_state->ms_localc);
	if likely(!(self->fg_state->ms_localv[stdout_lid].ml_flags & MEMLOC_F_LOCAL_BOUND)) {
		if unlikely(Dee_function_generator_vpush_imm32(self, DEE_STDOUT))
			goto err;
		if unlikely(Dee_function_generator_vcallapi(self, (void const *)&DeeFile_GetStd, VCALLOP_CC_OBJECT, 1))
			goto err;
		if unlikely(Dee_function_generator_vpop_local(self, stdout_lid))
			goto err;
	}
	ASSERT(self->fg_state->ms_localv[stdout_lid].ml_type != MEMLOC_TYPE_UNALLOC);
	ASSERT(self->fg_state->ms_localv[stdout_lid].ml_flags & MEMLOC_F_LOCAL_BOUND);
	ASSERT(!(self->fg_state->ms_localv[stdout_lid].ml_flags & MEMLOC_F_NOREF));
	for (iter = instr; iter < print_block_end;) {
		int temp;
		Dee_instruction_t const *next = DeeAsm_NextInstr(iter);
		uint16_t opcode = iter[0];
		if (ASM_ISEXTENDED(opcode))
			opcode = (opcode << 8) | iter[1];
		switch (opcode) {
		CASE_ASM_PRINT:
do_handle_ASM_PRINT:
			if unlikely(Dee_function_generator_vpush_local(self, stdout_lid))
				goto err;
			temp = gen_print_with_stdout_in_vtop(self, iter, opcode, repr);
			break;
		case ASM_REPR: {
			/* Special case for when the ASM_REPR is followed by a print-opcode. */
			uint16_t next_opcode = iter[1];
			if (ASM_ISEXTENDED(next_opcode))
				next_opcode = (next_opcode << 8) | iter[2];
			switch (next_opcode) {
			CASE_ASM_PRINT_AFTER_REPR:
				repr = true;
				opcode = next_opcode;
				++iter;
				goto do_handle_ASM_PRINT;
			default:
				break;
			}
		}	ATTR_FALLTHROUGH
		default:
			temp = Dee_function_generator_geninstr(self, iter, &next);
			break;
		}
		if unlikely(temp)
			goto err;
		iter = next;
		repr = false;
	}
	*p_next_instr = iter;
	return Dee_function_generator_vdel_local(self, stdout_lid); /* Delete the stdout-cache */
err:
	return -1;
}

/* Convert a single deemon instruction `instr' to host assembly and adjust the host memory
 * state according to the instruction in question. This is the core function to parse deemon
 * code and convert it to host assembly.
 * @param: p_next_instr: [inout] Pointer to the next instruction (may be overwritten if the
 *                               generated instruction was merged with its successor, as is
 *                               the case for `ASM_REPR' when followed by print-instructions) */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_geninstr(struct Dee_function_generator *__restrict self,
                                Dee_instruction_t const *instr,
                                Dee_instruction_t const **p_next_instr) {
	uint16_t opcode = instr[0];
	if (ASM_ISEXTENDED(opcode))
		opcode = (opcode << 8) | instr[1];
	switch (opcode) {

	case ASM_DELOP:
	case ASM16_DELOP:
	case ASM_NOP:
	case ASM16_NOP:
		/* No-op instructions */
		break;

	case ASM_SWAP:
		return Dee_function_generator_vswap(self);
	case ASM_LROT:
		return Dee_function_generator_vlrot(self, instr[1] + 3);
	case ASM16_LROT:
		return Dee_function_generator_vlrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	case ASM_RROT:
		return Dee_function_generator_vrrot(self, instr[1] + 3);
	case ASM16_RROT:
		return Dee_function_generator_vrrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	case ASM_DUP:
		return Dee_function_generator_vdup(self);
	case ASM_DUP_N:
		return Dee_function_generator_vdup_n(self, instr[1] + 2);
	case ASM16_DUP_N:
		return Dee_function_generator_vdup_n(self, UNALIGNED_GETLE16(instr + 2) + 2);
	case ASM_POP:
		return Dee_function_generator_vpop(self);
	case ASM_POP_N: {
		uint32_t n;
		n = instr[1] + 2;
		__IF0 { case ASM16_POP_N: n = UNALIGNED_GETLE16(instr + 2) + 2; }
		/* pop #SP - <imm8> - 2    (N = <imm8> + 2)
		 * <==>
		 * >> DECREF(STACKV[STACKC - N]);
		 * >> STACKV[STACKC - N] = STACKV[STACKC - 1];
		 * >> --STACKC;
		 * <==>
		 * >> LROT(N);
		 * >> POP();
		 * >> --N;
		 * >> RROT(N);
		 */
		if unlikely(Dee_function_generator_vlrot(self, n))
			goto err;
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		return Dee_function_generator_vrrot(self, n - 1);
	}	break;

	case ASM_ADJSTACK: {
		int32_t delta;
		delta = *(int8_t const *)(instr + 1);
		__IF0 { case ASM16_ADJSTACK: delta = (int16_t)UNALIGNED_GETLE16(instr + 2); }
		if (delta > 0) {
			do {
				if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
					goto err;
			} while (--delta > 0);
		} else if (delta < 0) {
			do {
				if unlikely(Dee_function_generator_vpop(self))
					goto err;
			} while (++delta < 0);
		}
	}	break;

	case ASM_POP_LOCAL:
		return Dee_function_generator_vpop_ulocal(self, instr[1]);
	case ASM16_POP_LOCAL:
		return Dee_function_generator_vpop_ulocal(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_PUSH_NONE:
		return Dee_function_generator_vpush_const(self, Dee_None);
	case ASM_PUSH_THIS_MODULE:
		return Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code->co_module);
	case ASM_PUSH_THIS_FUNCTION:
		return Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_function);

	case ASM_PUSH_MODULE: {
		DeeModuleObject *code_module;
		DeeModuleObject *push_module;
		uint16_t mid;
		mid = instr[1];
		__IF0 { case ASM16_PUSH_MODULE: mid = UNALIGNED_GETLE16(instr + 2); }
		code_module = self->fg_assembler->fa_code->co_module;
		if unlikely(mid >= code_module->mo_importc)
			return err_illegal_mid(mid);
		push_module = code_module->mo_importv[mid];
		return Dee_function_generator_vpush_const(self, (DeeObject *)push_module);
	}	break;

	case ASM_PUSH_ARG:
		return Dee_function_generator_vpush_arg(self, instr[1]);
	case ASM16_PUSH_ARG:
		return Dee_function_generator_vpush_arg(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_PUSH_CONST: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_PUSH_CONST: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		return Dee_function_generator_vpush_const(self, constant);
	}	break;

	case ASM_PUSH_REF: {
		uint16_t rid;
		DeeObject *reference;
		rid = instr[1];
		__IF0 { case ASM16_PUSH_REF: rid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(rid >= self->fg_assembler->fa_code->co_refc)
			return err_illegal_rid(rid);
		reference = self->fg_assembler->fa_function->fo_refv[rid];
		return Dee_function_generator_vpush_const(self, reference);
	}	break;

	case ASM_PUSH_EXTERN:
		return Dee_function_generator_vpush_extern(self, instr[1], instr[2]);
	case ASM16_PUSH_EXTERN:
		return Dee_function_generator_vpush_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));
	case ASM_PUSH_GLOBAL:
		return Dee_function_generator_vpush_global(self, instr[1]);
	case ASM16_PUSH_GLOBAL:
		return Dee_function_generator_vpush_global(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_PUSH_LOCAL:
		return Dee_function_generator_vpush_ulocal(self, instr[1]);
	case ASM16_PUSH_LOCAL:
		return Dee_function_generator_vpush_ulocal(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_RET_NONE:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		ATTR_FALLTHROUGH
	case ASM_RET:
		return Dee_function_generator_vret(self);

	case ASM_THROW:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeError_Throw, VCALLOP_CC_EXCEPT, 1);

	//TODO: case ASM_SETRET:

	case ASM_DEL_LOCAL:
		return Dee_function_generator_vdel_ulocal(self, instr[1]);
	case ASM16_DEL_LOCAL:
		return Dee_function_generator_vdel_ulocal(self, UNALIGNED_GETLE16(instr + 2));

	//TODO: case ASM_CALL_KW:
	//TODO: case ASM16_CALL_KW:

	case ASM_CALL_TUPLE_KW: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CALL_TUPLE_KW: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_CallTupleKw, VCALLOP_CC_OBJECT, 3);
	}	break;

	//TODO: case ASM_PUSH_BND_ARG:
	//TODO: case ASM16_PUSH_BND_ARG:
	//TODO: case ASM_PUSH_BND_EXTERN:
	//TODO: case ASM16_PUSH_BND_EXTERN:
	//TODO: case ASM_PUSH_BND_GLOBAL:
	//TODO: case ASM16_PUSH_BND_GLOBAL:
	//TODO: case ASM_PUSH_BND_LOCAL:
	//TODO: case ASM16_PUSH_BND_LOCAL:

	case ASM_JF:
	case ASM_JF16:
	case ASM_JT:
	case ASM_JT16:
	case ASM_JMP:
	case ASM_JMP16:
	case ASM32_JMP: {
		bool jump_if_true;
		struct Dee_jump_descriptor *desc;
do_jcc:
		desc = Dee_jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
		ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
		        Dee_function_assembler_addrof(self->fg_assembler, instr));
		if ((opcode & 0xff) == ASM_JF) {
			jump_if_true = false;
		} else if ((opcode & 0xff) == ASM_JT) {
			jump_if_true = true;
		} else {
			if unlikely(Dee_function_generator_vpush_const(self, Dee_True))
				goto err;
			jump_if_true = true;
		}
		return Dee_function_generator_vjcc(self, desc, instr, jump_if_true);
	}	break;

	//TODO: case ASM_FOREACH:
	//TODO: case ASM_FOREACH16:
	//TODO: case ASM_JMP_POP:
	//TODO: case ASM_JMP_POP_POP:

	case ASM_OPERATOR:
	case ASM16_OPERATOR: {
		uint16_t opname;
		uint8_t argc;
		if (opcode == ASM_OPERATOR) {
			opname = instr[1];
			argc   = instr[2];
		} else {
			opname = UNALIGNED_GETLE16(instr + 2);
			argc   = instr[4];
		}
		return Dee_function_generator_vop(self, opname, argc);
	}	break;

	case ASM_OPERATOR_TUPLE: {
		uint16_t opname;
		opname = instr[1];
		__IF0 { case ASM16_OPERATOR_TUPLE: opname = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(Dee_function_generator_vpush_imm16(self, opname))
			goto err; /* this, args, opname */
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* this, opname, args */
		return Dee_function_generator_vcall_DeeObject_InvokeOperatorTuple(self, (void const *)&DeeObject_InvokeOperator);
	}	break;

	//TODO: case ASM_CALL:

	case ASM_CALL_TUPLE:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_CallTuple, VCALLOP_CC_OBJECT, 2);

	case ASM_DEL_GLOBAL:
		return Dee_function_generator_vdel_global(self, instr[1]);
	case ASM16_DEL_GLOBAL:
		return Dee_function_generator_vdel_global(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_SUPER:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeSuper_New, VCALLOP_CC_OBJECT, 2);

	case ASM_SUPER_THIS_R: {
		uint16_t rid;
		DeeObject *reference;
		rid = instr[1];
		__IF0 { case ASM16_SUPER_THIS_R: rid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(rid >= self->fg_assembler->fa_code->co_refc)
			return err_illegal_rid(rid);
		reference = self->fg_assembler->fa_function->fo_refv[rid];
		if unlikely(Dee_function_generator_vpush_const(self, reference))
			goto err;
		if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THIS))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeSuper_New, VCALLOP_CC_OBJECT, 2);
	}	break;

	case ASM_POP_STATIC:
		return Dee_function_generator_vpop_static(self, instr[1]);
	case ASM16_POP_STATIC:
		return Dee_function_generator_vpop_static(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_POP_EXTERN:
		return Dee_function_generator_vpop_extern(self, instr[1], instr[2]);
	case ASM16_POP_EXTERN:
		return Dee_function_generator_vpop_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));
	case ASM_POP_GLOBAL:
		return Dee_function_generator_vpop_global(self, instr[1]);
	case ASM16_POP_GLOBAL:
		return Dee_function_generator_vpop_global(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_PUSH_VARARGS:
		if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)) {
			if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_ARGC))
				goto err;
			if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_ARGV))
				goto err;
			return Dee_function_generator_vcallapi(self, (void const *)&DeeTuple_NewVector, VCALLOP_CC_OBJECT, 2);
		}
		return Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_ARGS);

	case ASM_PUSH_VARKWDS:
		return Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_KW);

	case ASM_PUSH_STATIC:
		return Dee_function_generator_vpush_static(self, instr[1]);
	case ASM16_PUSH_STATIC:
		return Dee_function_generator_vpush_static(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_CAST_TUPLE:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeTuple_FromSequence, VCALLOP_CC_OBJECT, 1);
	case ASM_CAST_LIST:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeList_FromSequence, VCALLOP_CC_OBJECT, 1);

		/* TODO: Implement these by creating the list/tuple raw, and then filling its items. */
	//TODO: case ASM_PACK_TUPLE:
	//TODO: case ASM16_PACK_TUPLE:
	//TODO: case ASM_PACK_LIST:
	//TODO: case ASM16_PACK_LIST:

	//TODO: case ASM_UNPACK:
	//TODO: case ASM16_UNPACK:
	
	case ASM_CONCAT:
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* rhs, lhs */
		if unlikely(Dee_function_generator_vref(self))
			goto err; /* rhs, REF:lhs */
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* REF:lhs, rhs */
		if unlikely(Dee_function_generator_vcallapi(self, (void const *)&DeeObject_ConcatInherited, VCALLOP_CC_RAWINT_KEEPARGS, 2))
			goto err; /* ([valid_if(!RESULT)] REF:lhs), rhs, RESULT */
		if unlikely(Dee_function_generator_gjz_except(self, Dee_function_generator_vtop(self)))
			goto err; /* ([valid_if(false)] REF:lhs), rhs, RESULT */
		if unlikely(Dee_function_generator_vlrot(self, 3))
			goto err; /* rhs, RESULT, ([valid_if(false)] REF:lhs) */
		ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));
		Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF; /* rhs, RESULT, ([valid_if(false)] lhs) */
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* rhs, RESULT */
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* RESULT, rhs */
		return Dee_function_generator_vpop(self); /* RESULT */

	//TODO: case ASM_EXTEND:
	//TODO: case ASM_TYPEOF:
	//TODO: case ASM_CLASSOF:

	case ASM_SUPEROF:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeSuper_Of, VCALLOP_CC_OBJECT, 1);

	//TODO: case ASM_INSTANCEOF:
	//TODO: case ASM_IMPLEMENTS:

	case ASM_STR:
		return Dee_function_generator_vop(self, OPERATOR_STR, 1);
	case ASM_BOOL:
		/* TODO: Handle sequences of ASM_BOOL/ASM_NOT followed by ASM_JT/ASM_JF */
		return Dee_function_generator_vop(self, OPERATOR_BOOL, 1);

	//TODO: case ASM_NOT:

	case ASM_REPR: {
		/* Special handling when the next opcode is one
		 * of the ASM_*PRINT instructions, or ASM_SHL */
		Dee_instruction_t const *next_instr = *p_next_instr;
		uint16_t next_opcode = next_instr[0];
		if (ASM_ISEXTENDED(next_opcode))
			next_opcode = (next_opcode << 8) | next_instr[1];
		switch (next_opcode) {

		CASE_ASM_PRINT_AFTER_REPR:
			*p_next_instr = DeeAsm_NextInstr(next_instr);
			return Dee_function_generator_gen_stdout_print(self, next_instr, p_next_instr, true);

		case ASM_FPRINT:
		case ASM_FPRINT_SP:
		case ASM_FPRINT_NL: {
			void const *api_function;
			switch (opcode) {
			case ASM_FPRINT:    /* print top, pop */
				api_function = (void const *)&DeeFile_PrintObjectRepr;
				break;
			case ASM_FPRINT_SP: /* print top, pop, sp */
				api_function = (void const *)&DeeFile_PrintObjectReprSp;
				break;
			case ASM_FPRINT_NL: /* print top, pop, nl */
				api_function = (void const *)&DeeFile_PrintObjectReprNl;
				break;
			default: __builtin_unreachable();
			}
			if unlikely(Dee_function_generator_vswap(self))
				goto err; /* value, File */
			if unlikely(Dee_function_generator_vdup(self))
				goto err; /* value, File, File */
			if unlikely(Dee_function_generator_vlrot(self, 3))
				goto err; /* File, File, value */
			return Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_OBJECT, 2);
		}	break;

		case ASM_SHL:
			/* TODO: Special handling needed here! */
			break;

		default: break;
		}
		return Dee_function_generator_vop(self, OPERATOR_REPR, 1);
	}	break;

	case ASM_ASSIGN:
		return Dee_function_generator_vopv(self, OPERATOR_ASSIGN, 2);
	case ASM_MOVE_ASSIGN:
		return Dee_function_generator_vopv(self, OPERATOR_MOVEASSIGN, 2);

	case ASM_COPY:
		return Dee_function_generator_vop(self, OPERATOR_COPY, 1);
	case ASM_DEEPCOPY:
		return Dee_function_generator_vop(self, OPERATOR_DEEPCOPY, 1);
	case ASM_GETATTR:
		return Dee_function_generator_vop(self, OPERATOR_GETATTR, 2);
	case ASM_DELATTR:
		return Dee_function_generator_vopv(self, OPERATOR_DELATTR, 2);
	case ASM_SETATTR:
		return Dee_function_generator_vopv(self, OPERATOR_SETATTR, 3);

	//TODO: case ASM_BOUNDATTR:
	
	case ASM_GETATTR_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_GETATTR_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETATTR, 2);
	}	break;

	case ASM_DELATTR_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_DELATTR_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_DELATTR, 2))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_SETATTR_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_SETATTR_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETATTR, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_GETATTR_THIS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_GETATTR_THIS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETATTR, 2);
	}	break;

	case ASM_DELATTR_THIS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_DELATTR_THIS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_DELATTR, 2))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_SETATTR_THIS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_SETATTR_THIS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vlrot(self, 3))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETATTR, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_CMP_EQ:
	case ASM_CMP_NE:
	case ASM_CMP_GE:
	case ASM_CMP_LO:
	case ASM_CMP_LE:
	case ASM_CMP_GR: {
		/* In the case of the cmp-instructions, check if the next instructions
		 * is `jf' or `jt', and if so: generate a call to `DeeObject_Compare*'
		 * without the *Object suffix. That way, we don't need DeeObject_Bool,
		 * and also don't have to decref the comparison result!
		 * 
		 * Similarly, we can do the same if `ASM_BOOL' appears next, in which
		 * case we don't have to make 2 calls, or have the result be a reference */
		Dee_instruction_t const *next_instr = instr + 1;
		if (next_instr < self->fg_block->bb_deemon_end) {
			uint16_t next_opcode = next_instr[0];
			if (ASM_ISEXTENDED(next_opcode))
				next_opcode = (next_opcode << 8) | next_instr[1];
			switch (next_opcode) {

			case ASM_JT:
			case ASM_JT16:
			case ASM_JF:
			case ASM_JF16:
				/* TODO */
				break;
			
			case ASM_NOT:
			case ASM_BOOL:
				/* TODO */
				break;
			
			default: break;
			}
		}
		switch (opcode) {
		case ASM_CMP_EQ: return Dee_function_generator_vop(self, OPERATOR_EQ, 2);
		case ASM_CMP_NE: return Dee_function_generator_vop(self, OPERATOR_NE, 2);
		case ASM_CMP_GE: return Dee_function_generator_vop(self, OPERATOR_GE, 2);
		case ASM_CMP_LO: return Dee_function_generator_vop(self, OPERATOR_LO, 2);
		case ASM_CMP_LE: return Dee_function_generator_vop(self, OPERATOR_LE, 2);
		case ASM_CMP_GR: return Dee_function_generator_vop(self, OPERATOR_GR, 2);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	}	break;

	case ASM_CLASS_C:      /* push class pop, const <imm8> */
	case ASM16_CLASS_C:    /* push class pop, const <imm8> */
	case ASM_CLASS_GC:     /* push class global <imm8>, const <imm8> */
	case ASM16_CLASS_GC:   /* push class global <imm8>, const <imm8> */
	case ASM_CLASS_EC:     /* push class extern <imm8>:<imm8>, const <imm8> */
	case ASM16_CLASS_EC: { /* push class extern <imm8>:<imm8>, const <imm8> */
		DeeObject *desc;
		uint16_t desc_cid;
		switch (opcode) {
		case ASM_CLASS_C:
			desc_cid = instr[1];
			break;
		case ASM16_CLASS_C:
			desc_cid = UNALIGNED_GETLE16(instr + 2);
			break;
		case ASM_CLASS_GC: {
			uint16_t base_gid;
			base_gid = instr[1];
			desc_cid = instr[2];
			__IF0 {
		case ASM16_CLASS_GC:
				base_gid = UNALIGNED_GETLE16(instr + 2);
				desc_cid = UNALIGNED_GETLE16(instr + 4);
			}
			if unlikely(Dee_function_generator_vpush_global(self, base_gid))
				goto err;
		}	break;
		case ASM_CLASS_EC: {
			uint16_t base_mid;
			uint16_t base_gid;
			base_mid = instr[1];
			base_gid = instr[2];
			desc_cid = instr[3];
			__IF0 {
		case ASM16_CLASS_EC:
				base_mid = UNALIGNED_GETLE16(instr + 2);
				base_gid = UNALIGNED_GETLE16(instr + 4);
				desc_cid = UNALIGNED_GETLE16(instr + 6);
			}
			if unlikely(Dee_function_generator_vpush_extern(self, base_mid, base_gid))
				goto err;
		}	break;
		default: __builtin_unreachable();
		}
		if unlikely(desc_cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(desc_cid);
		desc = self->fg_assembler->fa_code->co_staticv[desc_cid];
		if unlikely(Dee_function_generator_vpush_const(self, desc))
			goto err;
		ATTR_FALLTHROUGH
	case ASM_CLASS:
		if unlikely(Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code->co_module))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeClass_New, VCALLOP_CC_OBJECT, 3);
	}	break;

	//TODO: case ASM_DEFCMEMBER:
	//TODO: case ASM16_DEFCMEMBER:
	//TODO: case ASM_GETCMEMBER_R:
	//TODO: case ASM16_GETCMEMBER:
	//TODO: case ASM16_GETCMEMBER_R:
	//TODO: case ASM_CALLCMEMBER_THIS_R:
	//TODO: case ASM16_CALLCMEMBER_THIS_R:

		/* TODO: Implement these by creating the function raw, and then using mov-s to fill in `fo_refv'
		 *       That way, we don't even have to push the reference somewhere temporarily, or have to
		 *       do decref when we just want the function to inherit them. */
	//TODO: case ASM_FUNCTION_C:
	//TODO: case ASM16_FUNCTION_C:
	//TODO: case ASM_FUNCTION_C_16:
	//TODO: case ASM16_FUNCTION_C_16:

	case ASM_CAST_INT:
		return Dee_function_generator_vop(self, OPERATOR_INT, 1);
	case ASM_INV:
		return Dee_function_generator_vop(self, OPERATOR_INV, 1);
	case ASM_POS:
		return Dee_function_generator_vop(self, OPERATOR_POS, 1);
	case ASM_NEG:
		return Dee_function_generator_vop(self, OPERATOR_NEG, 1);
	case ASM_ADD:
		return Dee_function_generator_vop(self, OPERATOR_ADD, 2);
	case ASM_SUB:
		return Dee_function_generator_vop(self, OPERATOR_SUB, 2);
	case ASM_MUL:
		return Dee_function_generator_vop(self, OPERATOR_MUL, 2);
	case ASM_DIV:
		return Dee_function_generator_vop(self, OPERATOR_DIV, 2);
	case ASM_MOD:
		return Dee_function_generator_vop(self, OPERATOR_MOD, 2);
	case ASM_SHL:
		return Dee_function_generator_vop(self, OPERATOR_SHL, 2);
	case ASM_SHR:
		return Dee_function_generator_vop(self, OPERATOR_SHR, 2);
	case ASM_AND:
		return Dee_function_generator_vop(self, OPERATOR_AND, 2);
	case ASM_OR:
		return Dee_function_generator_vop(self, OPERATOR_OR, 2);
	case ASM_XOR:
		return Dee_function_generator_vop(self, OPERATOR_XOR, 2);
	case ASM_POW:
		return Dee_function_generator_vop(self, OPERATOR_POW, 2);

	case ASM_ADD_SIMM8:
	case ASM_SUB_SIMM8:
	case ASM_MUL_SIMM8:
	case ASM_DIV_SIMM8:
	case ASM_MOD_SIMM8: {
		int8_t Simm8 = (int8_t)instr[1];
		if unlikely(Dee_function_generator_vpush_Simm8(self, Simm8))
			goto err;
		switch (opcode) {
		case ASM_ADD_SIMM8:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_AddInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_SUB_SIMM8:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_SubInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_MUL_SIMM8:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_MulInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_DIV_SIMM8:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_DivInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_MOD_SIMM8:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_ModInt8, VCALLOP_CC_OBJECT, 2);
		default: __builtin_unreachable();
		}
	}	break;

	case ASM_SHL_IMM8:
	case ASM_SHR_IMM8: {
		uint8_t imm8 = (uint8_t)instr[1];
		if unlikely(Dee_function_generator_vpush_imm8(self, imm8))
			goto err;
		switch (opcode) {
		case ASM_SHL_IMM8:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_ShlUInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_SHR_IMM8:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_ShrUInt8, VCALLOP_CC_OBJECT, 2);
		default: __builtin_unreachable();
		}
	}	break;

	case ASM_ADD_IMM32:
	case ASM_SUB_IMM32:
	case ASM_AND_IMM32:
	case ASM_OR_IMM32:
	case ASM_XOR_IMM32: {
		uint32_t imm32 = (uint32_t)UNALIGNED_GETLE32(instr + 1);
		if unlikely(Dee_function_generator_vpush_imm32(self, imm32))
			goto err;
		switch (opcode) {
		case ASM_ADD_IMM32:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_AddUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_SUB_IMM32:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_SubUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_AND_IMM32:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_AndUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_OR_IMM32:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_OrUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_XOR_IMM32:
			return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_XorUInt32, VCALLOP_CC_OBJECT, 2);
		default: __builtin_unreachable();
		}
	}	break;

	//TODO: case ASM_ISNONE:

	CASE_ASM_PRINT:
		return Dee_function_generator_gen_stdout_print(self, instr, p_next_instr, false);

	case ASM_FPRINT:         /* print top, pop */
	case ASM_FPRINT_SP:      /* print top, pop, sp */
	case ASM_FPRINT_NL:      /* print top, pop, nl */
	case ASM_FPRINTALL:      /* print top, pop... */
	case ASM_FPRINTALL_SP:   /* print top, pop..., sp */
	case ASM_FPRINTALL_NL: { /* print top, pop..., nl */
		void const *api_function;
		switch (opcode) {
		case ASM_FPRINT: /* print top, pop */
			api_function = (void const *)&DeeFile_PrintObject;
			break;
		case ASM_FPRINT_SP: /* print top, pop, sp */
			api_function = (void const *)&DeeFile_PrintObjectSp;
			break;
		case ASM_FPRINT_NL: /* print top, pop, nl */
			api_function = (void const *)&DeeFile_PrintObjectNl;
			break;
		case ASM_FPRINTALL: /* print top, pop... */
			api_function = (void const *)&DeeFile_PrintAll;
			break;
		case ASM_FPRINTALL_SP: /* print top, pop..., sp */
			api_function = (void const *)&DeeFile_PrintAllSp;
			break;
		case ASM_FPRINTALL_NL: /* print top, pop..., nl */
			api_function = (void const *)&DeeFile_PrintAllNl;
			break;
		default: __builtin_unreachable();
		}
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* value, File */
		if unlikely(Dee_function_generator_vdup(self))
			goto err; /* value, File, File */
		if unlikely(Dee_function_generator_vlrot(self, 3))
			goto err; /* File, File, value */
		return Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_OBJECT, 2);
	}	break;

	case ASM_FPRINTNL: /* print top, nl */
		if unlikely(Dee_function_generator_vdup(self))
			goto err; /* File, File */
		return Dee_function_generator_vcallapi(self, (void const *)&DeeFile_PrintNl, VCALLOP_CC_OBJECT, 1);

	case ASM_FPRINT_C:        /* print top, const <imm8> */
	case ASM16_FPRINT_C:      /* print top, const <imm16> */
	case ASM_FPRINT_C_SP:     /* print top, const <imm8>, sp */
	case ASM16_FPRINT_C_SP:   /* print top, const <imm16>, sp */
	case ASM_FPRINT_C_NL:     /* print top, const <imm8>, nl */
	case ASM16_FPRINT_C_NL: { /* print top, const <imm16>, nl */
		void const *api_function;
		DeeObject *constant;
		uint16_t cid = instr[1];
		if (opcode > 0xff)
			cid = UNALIGNED_GETLE16(instr + 2);
		switch (opcode) {
		case ASM_FPRINT_C:   /* print top, const <imm8> */
		case ASM16_FPRINT_C: /* print top, const <imm16> */
			api_function = (void const *)&DeeFile_PrintObject;
			break;
		case ASM_FPRINT_C_SP:   /* print top, const <imm8>, sp */
		case ASM16_FPRINT_C_SP: /* print top, const <imm16>, sp */
			api_function = (void const *)&DeeFile_PrintObjectSp;
			break;
		case ASM_FPRINT_C_NL:   /* print top, const <imm8>, nl */
		case ASM16_FPRINT_C_NL: /* print top, const <imm16>, nl */
			api_function = (void const *)&DeeFile_PrintObjectNl;
			break;
		default: __builtin_unreachable();
		}
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vdup(self))
			goto err; /* File, File */
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err; /* File, File, value */
		return Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_OBJECT, 2);
	}	break;

	case ASM_ENTER:
		if unlikely(Dee_function_generator_vdup(self))
			goto err;
		return Dee_function_generator_vopv(self, OPERATOR_ENTER, 1);
	case ASM_LEAVE:
		return Dee_function_generator_vopv(self, OPERATOR_LEAVE, 1);

	//TODO: case ASM_RANGE:
	//TODO: case ASM_RANGE_DEF:
	//TODO: case ASM_RANGE_STEP:
	//TODO: case ASM_RANGE_STEP_DEF:

	case ASM_RANGE_0_I16: {
		uint32_t imm32;
		imm32 = UNALIGNED_GETLE16(instr + 1);
		__IF0 { case ASM_RANGE_0_I32: imm32 = UNALIGNED_GETLE32(instr + 2); }
		if unlikely(Dee_function_generator_vpush_imm32(self, 0))
			goto err;
		if unlikely(Dee_function_generator_vpush_imm32(self, imm32))
			goto err;
		if unlikely(Dee_function_generator_vpush_imm32(self, 1))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeRange_NewInt, VCALLOP_CC_OBJECT, 2);
	}	break;

	case ASM_CONTAINS:
		return Dee_function_generator_vop(self, OPERATOR_CONTAINS, 2);

	case ASM_CONTAINS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CONTAINS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_CONTAINS, 2);
	}	break;

	case ASM_GETITEM:
		return Dee_function_generator_vop(self, OPERATOR_GETITEM, 2);

	case ASM_GETITEM_I: {
		uint16_t imm16 = UNALIGNED_GETLE16(instr + 1);
		if unlikely(Dee_function_generator_vpush_imm16(self, imm16))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_GetItemIndex, VCALLOP_CC_OBJECT, 2);
	}	break;

	case ASM_GETITEM_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_GETITEM_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETITEM, 2);
	}	break;

	case ASM_GETSIZE:
		return Dee_function_generator_vop(self, OPERATOR_SIZE, 2);

	case ASM_SETITEM_I: {
		uint16_t imm16 = UNALIGNED_GETLE16(instr + 1);
		if unlikely(Dee_function_generator_vpush_imm16(self, imm16))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_SetItemIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	case ASM_SETITEM_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_SETITEM_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETITEM, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_ITERSELF:
		return Dee_function_generator_vop(self, OPERATOR_ITERSELF, 1);
	case ASM_DELITEM:
		return Dee_function_generator_vopv(self, OPERATOR_DELITEM, 2);
	case ASM_SETITEM:
		return Dee_function_generator_vopv(self, OPERATOR_SETITEM, 3);
	case ASM_GETRANGE:
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3);
	case ASM_GETRANGE_PN:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3);
	case ASM_GETRANGE_NP:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3);

	case ASM_GETRANGE_PI:
	case ASM_GETRANGE_NI: {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		if (opcode == ASM_GETRANGE_NI) {
			if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
				goto err;
		}
		if unlikely(Dee_function_generator_vpush_Simm16(self, Simm16))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_GetRangeEndIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	case ASM_GETRANGE_IP:
	case ASM_GETRANGE_IN: {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		if unlikely(Dee_function_generator_vpush_Simm16(self, Simm16))
			goto err;
		if (opcode == ASM_GETRANGE_IN) {
			if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
				goto err;
		} else {
			if unlikely(Dee_function_generator_vswap(self))
				goto err;
		}
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_GetRangeBeginIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	case ASM_GETRANGE_II: {
		int16_t begin_Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		int16_t end_Simm16   = (int16_t)UNALIGNED_GETLE16(instr + 3);
		if unlikely(Dee_function_generator_vpush_Simm16(self, begin_Simm16))
			goto err;
		if unlikely(Dee_function_generator_vpush_Simm16(self, end_Simm16))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_GetRangeIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	case ASM_DELRANGE:
		return Dee_function_generator_vopv(self, OPERATOR_DELRANGE, 3);
	case ASM_SETRANGE:
		return Dee_function_generator_vopv(self, OPERATOR_SETRANGE, 4);
	case ASM_SETRANGE_PN:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETRANGE, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	case ASM_SETRANGE_NP:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETRANGE, 3))
			goto err;
		return Dee_function_generator_vpop(self);

	case ASM_SETRANGE_PI:
	case ASM_SETRANGE_NI: {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		if (opcode == ASM_SETRANGE_NI) {
			if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
				goto err;
		}
		if unlikely(Dee_function_generator_vpush_Simm16(self, Simm16))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_SetRangeEndIndex, VCALLOP_CC_INT, 4);
	}	break;

	case ASM_SETRANGE_IP:
	case ASM_SETRANGE_IN: {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		if unlikely(Dee_function_generator_vpush_Simm16(self, Simm16))
			goto err;
		if (opcode == ASM_SETRANGE_IN) {
			if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
				goto err;
		} else {
			if unlikely(Dee_function_generator_vswap(self))
				goto err;
		}
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_SetRangeBeginIndex, VCALLOP_CC_INT, 4);
	}	break;

	case ASM_SETRANGE_II: {
		int16_t begin_Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		int16_t end_Simm16   = (int16_t)UNALIGNED_GETLE16(instr + 3);
		if unlikely(Dee_function_generator_vpush_Simm16(self, begin_Simm16))
			goto err;
		if unlikely(Dee_function_generator_vpush_Simm16(self, end_Simm16))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_SetRangeIndex, VCALLOP_CC_INT, 4);
	}	break;

	//TODO: case ASM_BREAKPOINT:
	//TODO: case ASM_UD:

	//TODO: case ASM_CALLATTR_C_KW:
	//TODO: case ASM16_CALLATTR_C_KW:

	case ASM_CALLATTR_C_TUPLE_KW: {
		uint16_t args_cid;
		uint16_t kwds_cid;
		DeeObject *constant;
		args_cid = instr[1];
		kwds_cid = instr[2];
		__IF0 {
	case ASM16_CALLATTR_C_TUPLE_KW:
			args_cid = UNALIGNED_GETLE16(instr + 2);
			kwds_cid = UNALIGNED_GETLE16(instr + 4);
		}
		if unlikely(args_cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(args_cid);
		if unlikely(kwds_cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(kwds_cid);
		constant = self->fg_assembler->fa_code->co_staticv[args_cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[kwds_cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		ATTR_FALLTHROUGH
	case ASM_CALLATTR_TUPLE_KWDS:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_CallAttrTupleKw, VCALLOP_CC_OBJECT, 4);
	}	break;

	//TODO: case ASM_CALLATTR:
	//TODO: case ASM_CALLATTR_C:
	//TODO: case ASM16_CALLATTR_C:

	case ASM_CALLATTR_C_TUPLE: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CALLATTR_C_TUPLE: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		ATTR_FALLTHROUGH
	case ASM_CALLATTR_TUPLE:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_CallAttrTuple, VCALLOP_CC_OBJECT, 3);
	}	break;

	//TODO: case ASM_CALLATTR_THIS_C:
	//TODO: case ASM16_CALLATTR_THIS_C:

	case ASM_CALLATTR_THIS_C_TUPLE: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CALLATTR_THIS_C_TUPLE: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vlrot(self, 3))
			goto err;
		ATTR_FALLTHROUGH
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_CallAttrTuple, VCALLOP_CC_OBJECT, 3);
	}	break;

	//TODO: case ASM_CALLATTR_C_SEQ:
	//TODO: case ASM16_CALLATTR_C_SEQ:
	//TODO: case ASM_CALLATTR_C_MAP:
	//TODO: case ASM16_CALLATTR_C_MAP:

	//TODO: case ASM_CALLATTR_KWDS:

	//TODO: case ASM_GETMEMBER_THIS_R:
	//TODO: case ASM16_GETMEMBER_THIS_R:
	//TODO: case ASM_DELMEMBER_THIS_R:
	//TODO: case ASM16_DELMEMBER_THIS_R:
	//TODO: case ASM_SETMEMBER_THIS_R:
	//TODO: case ASM16_SETMEMBER_THIS_R:
	//TODO: case ASM_BOUNDMEMBER_THIS_R:
	//TODO: case ASM16_BOUNDMEMBER_THIS_R:
	//TODO: case ASM_CALL_EXTERN:
	//TODO: case ASM16_CALL_EXTERN:
	//TODO: case ASM_CALL_GLOBAL:
	//TODO: case ASM16_CALL_GLOBAL:
	//TODO: case ASM_CALL_LOCAL:
	//TODO: case ASM16_CALL_LOCAL:

	//TODO: case ASM_CALL_SEQ:
	//TODO: case ASM_CALL_MAP:

	case ASM_THISCALL_TUPLE:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_ThisCallTuple, VCALLOP_CC_OBJECT, 3);
	case ASM_CALL_TUPLE_KWDS:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeObject_CallTupleKw, VCALLOP_CC_OBJECT, 3);

	//TODO: case ASM_PUSH_EXCEPT:

	case ASM_PUSH_THIS:
		return Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THIS);
	case ASM_CAST_HASHSET:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeHashSet_FromSequence, VCALLOP_CC_OBJECT, 1);
	case ASM_CAST_DICT:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeDict_FromSequence, VCALLOP_CC_OBJECT, 1);

	case ASM_PUSH_TRUE:
		return Dee_function_generator_vpush_const(self, Dee_True);
	case ASM_PUSH_FALSE:
		return Dee_function_generator_vpush_const(self, Dee_False);

	//TODO: case ASM_PACK_HASHSET:
	//TODO: case ASM16_PACK_HASHSET:
	//TODO: case ASM_PACK_DICT:
	//TODO: case ASM16_PACK_DICT:
	//TODO: case ASM_BOUNDITEM:
	//TODO: case ASM_CMP_SO:
	//TODO: case ASM_CMP_DO:

	//TODO: case ASM_SUPERGETATTR_THIS_RC:
	//TODO: case ASM16_SUPERGETATTR_THIS_RC:
	//TODO: case ASM_SUPERCALLATTR_THIS_RC:
	//TODO: case ASM16_SUPERCALLATTR_THIS_RC:
	//TODO: case ASM_INCPOST:
	//TODO: case ASM_DECPOST:

	case ASM_REDUCE_MIN:
		if unlikely(Dee_function_generator_vpush_addr(self, NULL))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeSeq_Min, VCALLOP_CC_OBJECT, 2);
	case ASM_REDUCE_MAX:
		if unlikely(Dee_function_generator_vpush_addr(self, NULL))
			goto err;
		return Dee_function_generator_vcallapi(self, (void const *)&DeeSeq_Max, VCALLOP_CC_OBJECT, 2);
	case ASM_REDUCE_SUM:
		return Dee_function_generator_vcallapi(self, (void const *)&DeeSeq_Sum, VCALLOP_CC_OBJECT, 1);

	//TODO: case ASM_REDUCE_ANY:
	//TODO: case ASM_REDUCE_ALL:

	//TODO: case ASM_VARARGS_UNPACK:
	//TODO: case ASM_PUSH_VARKWDS_NE:
	//TODO: case ASM_VARARGS_CMP_EQ_SZ:
	//TODO: case ASM_VARARGS_CMP_GR_SZ:
	//TODO: case ASM_VARARGS_GETITEM:
	//TODO: case ASM_VARARGS_GETITEM_I:
	//TODO: case ASM_VARARGS_GETSIZE:

	case ASM_ITERNEXT:
		return Dee_function_generator_vop(self, OPERATOR_ITERNEXT, 1);

	//TODO: case ASM_GETMEMBER:
	//TODO: case ASM16_GETMEMBER:
	//TODO: case ASM_DELMEMBER:
	//TODO: case ASM16_DELMEMBER:
	//TODO: case ASM_SETMEMBER:
	//TODO: case ASM16_SETMEMBER:
	//TODO: case ASM_BOUNDMEMBER:
	//TODO: case ASM16_BOUNDMEMBER:
	//TODO: case ASM_GETMEMBER_THIS:
	//TODO: case ASM16_GETMEMBER_THIS:
	//TODO: case ASM_DELMEMBER_THIS:
	//TODO: case ASM16_DELMEMBER_THIS:
	//TODO: case ASM_SETMEMBER_THIS:
	//TODO: case ASM16_SETMEMBER_THIS:
	//TODO: case ASM_BOUNDMEMBER_THIS:
	//TODO: case ASM16_BOUNDMEMBER_THIS:



		/* Instruction prefixes. */
	case ASM_STACK:
	case ASM16_STACK:
	case ASM_STATIC:
	case ASM16_STATIC:
	case ASM_EXTERN:
	case ASM16_EXTERN:
	case ASM_GLOBAL:
	case ASM16_GLOBAL:
	case ASM_LOCAL:
	case ASM16_LOCAL: {
		Dee_instruction_t const *prefix_instr;
		uint8_t prefix_type = opcode & 0xff;
		uint16_t prefix_opcode;
		uint16_t id1 = instr[1];
		uint16_t id2 = instr[2];
		if (opcode & 0xff00) {
			id1 = UNALIGNED_GETLE16(instr + 2);
			id2 = UNALIGNED_GETLE16(instr + 4);
		}
		switch (prefix_type) {
		case ASM_STACK:
			if unlikely(id1 >= self->fg_state->ms_stackc)
				return err_illegal_stack_effect();
			break;
		case ASM_STATIC:
			if unlikely(id1 >= self->fg_assembler->fa_code->co_staticc)
				return err_illegal_sid(id1);
			break;
		case ASM_EXTERN: {
			DeeModuleObject *mod;
			mod = self->fg_assembler->fa_code->co_module;
			if unlikely(id1 >= mod->mo_importc)
				return err_illegal_sid(id1);
			mod = mod->mo_importv[id1];
			if unlikely(id2 >= mod->mo_globalc)
				return err_illegal_gid(mod, id2);
		}	break;
		case ASM_GLOBAL: {
			DeeModuleObject *mod;
			mod = self->fg_assembler->fa_code->co_module;
			if unlikely(id1 >= mod->mo_globalc)
				return err_illegal_gid(mod, id1);
		}	break;
		case ASM_LOCAL:
			if unlikely(id1 >= self->fg_assembler->fa_code->co_localc)
				return err_illegal_lid(id1);
			break;
		default: __builtin_unreachable();
		}
		prefix_instr  = DeeAsm_SkipPrefix(instr);
		prefix_opcode = prefix_instr[0];
		if (ASM_ISEXTENDED(prefix_opcode))
			prefix_opcode = (prefix_opcode << 8) | prefix_instr[1];
		switch (prefix_opcode) {

		case ASM_JF:   /* jf PREFIX, <Sdisp8> */
		case ASM_JF16: /* jf PREFIX, <Sdisp16> */
		case ASM_JT:   /* jt PREFIX, <Sdisp8> */
		case ASM_JT16: /* jt PREFIX, <Sdisp16> */
			if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
				goto err;
			opcode = prefix_opcode;
			/* Need to do this in a special way because `instr' must not become `prefix_instr' here. */
			goto do_jcc;

		case ASM_RET:          /* ret PREFIX */
		case ASM_THROW:        /* throw PREFIX */
		case ASM_SETRET:       /* setret PREFIX */
		case ASM_POP_STATIC:   /* mov static <imm8>, PREFIX */
		case ASM16_POP_STATIC: /* mov static <imm16>, PREFIX */
		case ASM_POP_EXTERN:   /* mov extern <imm8>:<imm8>, PREFIX */
		case ASM16_POP_EXTERN: /* mov extern <imm16>:<imm16>, PREFIX */
		case ASM_POP_GLOBAL:   /* mov global <imm8>, PREFIX */
		case ASM16_POP_GLOBAL: /* mov global <imm16>, PREFIX */
		case ASM_POP_LOCAL:    /* mov local <imm8>, PREFIX */
		case ASM16_POP_LOCAL:  /* mov local <imm16>, PREFIX */
		case ASM_UNPACK:       /* unpack PREFIX, #<imm8> */
			if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
				goto err;
			return Dee_function_generator_geninstr(self, prefix_instr, p_next_instr);

		case ASM_FOREACH:            /* foreach PREFIX, <Sdisp8> */
		case ASM_FOREACH16:          /* foreach PREFIX, <Sdisp16> */
		case ASM_DUP:                /* mov PREFIX, top', `mov PREFIX, #SP - 1 */
		case ASM_DUP_N:              /* mov PREFIX, #SP - <imm8> - 2 */
		case ASM16_DUP_N:            /* mov PREFIX, #SP - <imm16> - 2 */
		case ASM_POP:                /* mov top, PREFIX */
		case ASM_PUSH_NONE:          /* mov  PREFIX, none */
		case ASM_PUSH_VARARGS:       /* mov  PREFIX, varargs */
		case ASM_PUSH_VARKWDS:       /* mov  PREFIX, varkwds */
		case ASM_PUSH_MODULE:        /* mov  PREFIX, module <imm8> */
		case ASM_PUSH_ARG:           /* mov  PREFIX, arg <imm8> */
		case ASM_PUSH_CONST:         /* mov  PREFIX, const <imm8> */
		case ASM_PUSH_REF:           /* mov  PREFIX, ref <imm8> */
		case ASM_PUSH_STATIC:        /* mov  PREFIX, static <imm8> */
		case ASM_PUSH_EXTERN:        /* mov  PREFIX, extern <imm8>:<imm8> */
		case ASM_PUSH_GLOBAL:        /* mov  PREFIX, global <imm8> */
		case ASM_PUSH_LOCAL:         /* mov  PREFIX, local <imm8> */
		case ASM_FUNCTION_C:         /* PREFIX: function const <imm8>, #<imm8>+1 */
		case ASM16_FUNCTION_C:       /* PREFIX: function const <imm16>, #<imm8>+1 */
		case ASM_FUNCTION_C_16:      /* PREFIX: function const <imm8>, #<imm16>+1 */
		case ASM16_FUNCTION_C_16:    /* PREFIX: function const <imm16>, #<imm16>+1 */
		case ASM_PUSH_EXCEPT:        /* mov  PREFIX, except */
		case ASM_PUSH_THIS:          /* mov  PREFIX, this */
		case ASM_PUSH_THIS_MODULE:   /* mov  PREFIX, this_module */
		case ASM_PUSH_THIS_FUNCTION: /* mov  PREFIX, this_function */
		case ASM16_PUSH_MODULE:      /* mov  PREFIX, module <imm16> */
		case ASM16_PUSH_ARG:         /* mov  PREFIX, arg <imm16> */
		case ASM16_PUSH_CONST:       /* mov  PREFIX, const <imm16> */
		case ASM16_PUSH_REF:         /* mov  PREFIX, ref <imm16> */
		case ASM16_PUSH_STATIC:      /* mov  PREFIX, static <imm16> */
		case ASM16_PUSH_EXTERN:      /* mov  PREFIX, extern <imm16>:<imm16> */
		case ASM16_PUSH_GLOBAL:      /* mov  PREFIX, global <imm16> */
		case ASM16_PUSH_LOCAL:       /* mov  PREFIX, local <imm16> */
		case ASM_PUSH_TRUE:          /* mov  PREFIX, true */
		case ASM_PUSH_FALSE:         /* mov  PREFIX, false */
			if unlikely(Dee_function_generator_geninstr(self, prefix_instr, p_next_instr))
				goto err;
			if unlikely(self->fg_block->bb_mem_end == (DREF struct Dee_memstate *)-1)
				return 0;
			return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);


		case ASM_POP_N: { /* mov #SP - <imm8> - 2, PREFIX */
			uint32_t n;
			n = instr[1] + 2; /* mov #SP - <imm16> - 2, PREFIX */
			__IF0 { case ASM16_POP_N: n = UNALIGNED_GETLE16(instr + 2) + 2; }
			if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
				goto err;
			if unlikely(Dee_function_generator_vlrot(self, n + 1))
				goto err;
			if unlikely(Dee_function_generator_vpop(self))
				goto err;
			return Dee_function_generator_vrrot(self, n);
		}	break;

			/* NOTE: These instructions are special because they operate atomically for GLOBAL/EXTERN prefixes! */
		//TODO: case ASM_SWAP: /* swap top, PREFIX */
		//TODO: case ASM_LROT: /* lrot #<imm8>+2, PREFIX */
		//TODO: case ASM_RROT: /* rrot #<imm8>+2, PREFIX */
		//TODO: case ASM16_LROT: /* lrot #<imm16>+2, PREFIX */
		//TODO: case ASM16_RROT: /* rrot #<imm16>+2, PREFIX */

		case ASM_ADD:         /* add PREFIX, pop */
		case ASM_SUB:         /* sub PREFIX, pop */
		case ASM_MUL:         /* mul PREFIX, pop */
		case ASM_DIV:         /* div PREFIX, pop */
		case ASM_MOD:         /* mod PREFIX, pop */
		case ASM_SHL:         /* shl PREFIX, pop */
		case ASM_SHR:         /* shr PREFIX, pop */
		case ASM_AND:         /* and PREFIX, pop */
		case ASM_OR:          /* or  PREFIX, pop */
		case ASM_XOR:         /* xor PREFIX, pop */
		case ASM_POW:         /* pow PREFIX, pop */
		case ASM_INC:         /* inc PREFIX */
		case ASM_DEC:         /* dec PREFIX */
		case ASM_ADD_SIMM8:   /* add PREFIX, $<Simm8> */
		case ASM_ADD_IMM32:   /* add PREFIX, $<imm32> */
		case ASM_SUB_SIMM8:   /* sub PREFIX, $<Simm8> */
		case ASM_SUB_IMM32:   /* sub PREFIX, $<imm32> */
		case ASM_MUL_SIMM8:   /* mul PREFIX, $<Simm8> */
		case ASM_DIV_SIMM8:   /* div PREFIX, $<Simm8> */
		case ASM_MOD_SIMM8:   /* mod PREFIX, $<Simm8> */
		case ASM_SHL_IMM8:    /* shl PREFIX, $<Simm8> */
		case ASM_SHR_IMM8:    /* shr PREFIX, $<Simm8> */
		case ASM_AND_IMM32:   /* and PREFIX, $<imm32> */
		case ASM_OR_IMM32:    /* or  PREFIX, $<imm32> */
		case ASM_XOR_IMM32: { /* xor PREFIX, $<imm32> */
			int temp = Dee_function_generator_vpush_prefix_addr(self, prefix_type, id1, id2);
			if unlikely(temp < 0)
				goto err;
			if (temp == 0) {
				/* Prefix supports direct addressing (and the `DREF DeeObject **' was already pushed) */
				switch (prefix_opcode) {
				case ASM_ADD: /* add PREFIX, pop */
				case ASM_SUB: /* sub PREFIX, pop */
				case ASM_MUL: /* mul PREFIX, pop */
				case ASM_DIV: /* div PREFIX, pop */
				case ASM_MOD: /* mod PREFIX, pop */
				case ASM_SHL: /* shl PREFIX, pop */
				case ASM_SHR: /* shr PREFIX, pop */
				case ASM_AND: /* and PREFIX, pop */
				case ASM_OR:  /* or  PREFIX, pop */
				case ASM_XOR: /* xor PREFIX, pop */
				case ASM_POW: /* pow PREFIX, pop */
					if unlikely(Dee_function_generator_vswap(self))
						goto err;
					break;
				default: break;
				}
				return perform_inplace_math_operation_with_vtop_addr(self, prefix_opcode, prefix_instr);
			} else {
				if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
					goto err; /* value */
				if unlikely(Dee_function_generator_vflush(self))
					goto err; /* onstack(value) */
				ASSERT(Dee_function_generator_vtop(self)->ml_type == MEMLOC_TYPE_HSTACKIND);
				ASSERT(Dee_function_generator_vtop(self)->ml_value.v_hstack.s_off == 0);
				if unlikely(Dee_function_generator_vpush_hstack(self, Dee_function_generator_vtop(self)->ml_value.v_hstack.s_cfa))
					goto err; /* onstack(value), addrof(onstack(value)) */
				switch (prefix_opcode) {
				case ASM_ADD: /* add PREFIX, pop */
				case ASM_SUB: /* sub PREFIX, pop */
				case ASM_MUL: /* mul PREFIX, pop */
				case ASM_DIV: /* div PREFIX, pop */
				case ASM_MOD: /* mod PREFIX, pop */
				case ASM_SHL: /* shl PREFIX, pop */
				case ASM_SHR: /* shr PREFIX, pop */
				case ASM_AND: /* and PREFIX, pop */
				case ASM_OR:  /* or  PREFIX, pop */
				case ASM_XOR: /* xor PREFIX, pop */
				case ASM_POW: /* pow PREFIX, pop */
					if unlikely(Dee_function_generator_vlrot(self, 3))
						goto err; /* onstack(value), addrof(onstack(value)), operand */
					break;
				default: break;
				}
				if unlikely(perform_inplace_math_operation_with_vtop_addr(self, prefix_opcode, prefix_instr))
					goto err; /* onstack(value) */
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);
			}
		}	break;

		case ASM_INCPOST:   /* push inc PREFIX' - `PREFIX: push inc */
		case ASM_DECPOST: { /* push dec PREFIX' - `PREFIX: push dec */
			int temp;
			if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
				goto err;
			temp = Dee_function_generator_vpush_prefix_addr(self, prefix_type, id1, id2);
			if unlikely(temp < 0)
				goto err;
			if (temp) {
				if unlikely(Dee_function_generator_vswap(self))
					goto err; /* addrof(value), value */
				if unlikely(Dee_function_generator_vop(self, OPERATOR_COPY, 1))
					goto err; /* addrof(value), copy */
				if unlikely(Dee_function_generator_vswap(self))
					goto err; /* copy, addrof(value) */
				return Dee_function_generator_vcallapi(self,
				                                       opcode == ASM_INCPOST ? (void const *)&DeeObject_Inc
				                                                             : (void const *)&DeeObject_Dec,
				                                       VCALLOP_CC_INT, 1);
			} else {
				if unlikely(Dee_function_generator_vdup(self))
					goto err; /* value, value */
				if unlikely(Dee_function_generator_vop(self, OPERATOR_COPY, 1))
					goto err; /* value, copy */
				if unlikely(Dee_function_generator_vswap(self))
					goto err; /* copy, value */
				if unlikely(Dee_function_generator_vflush(self))
					goto err; /* copy, onstack(value) */
				ASSERT(Dee_function_generator_vtop(self)->ml_type == MEMLOC_TYPE_HSTACKIND);
				ASSERT(Dee_function_generator_vtop(self)->ml_value.v_hstack.s_off == 0);
				if unlikely(Dee_function_generator_vpush_hstack(self, Dee_function_generator_vtop(self)->ml_value.v_hstack.s_cfa))
					goto err; /* copy, onstack(value), addrof(onstack(value)) */
				if unlikely(Dee_function_generator_vcallapi(self,
				                                            opcode == ASM_INCPOST ? (void const *)&DeeObject_Inc
				                                                                  : (void const *)&DeeObject_Dec,
				                                            VCALLOP_CC_INT, 1))
					goto err; /* copy, onstack(value) */
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);
			}
		}	break;

		case ASM_DELOP:
		case ASM16_DELOP:
		case ASM_NOP:   /* nop PREFIX */
		case ASM16_NOP: /* nop16 PREFIX' - `PREFIX: nop16 */
			break;

		//TODO: case ASM_OPERATOR:   /* PREFIX: push op $<imm8>, #<imm8> */
		//TODO: case ASM16_OPERATOR: /* PREFIX: push op $<imm16>, #<imm8> */

		case ASM_OPERATOR_TUPLE: { /* PREFIX: push op $<imm8>, pop... */
			int temp;
			uint16_t opname;
			opname = prefix_instr[1];  /* PREFIX: push op $<imm16>, pop */
			__IF0 { case ASM16_OPERATOR_TUPLE: opname = UNALIGNED_GETLE16(prefix_instr + 2); }
			if unlikely(Dee_function_generator_vpush_imm16(self, opname))
				goto err; /* args, opname */
			if unlikely(Dee_function_generator_vswap(self))
				goto err; /* opname, args */
			temp = Dee_function_generator_vpush_prefix_addr(self, prefix_type, id1, id2);
			if unlikely(temp < 0)
				goto err;
			if (temp) { /* opname, args, addrof(value) */
				if unlikely(Dee_function_generator_vrrot(self, 3))
					goto err; /* addrof(value), opname, args */
				return Dee_function_generator_vcall_DeeObject_InvokeOperatorTuple(self, (void const *)&DeeObject_PInvokeOperator);
			} else {
				if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
					goto err; /* opname, args, value */
				if unlikely(Dee_function_generator_vflush(self))
					goto err; /* opname, args, onstack(value) */
				ASSERT(Dee_function_generator_vtop(self)->ml_type == MEMLOC_TYPE_HSTACKIND);
				ASSERT(Dee_function_generator_vtop(self)->ml_value.v_hstack.s_off == 0);
				if unlikely(Dee_function_generator_vpush_hstack(self, Dee_function_generator_vtop(self)->ml_value.v_hstack.s_cfa))
					goto err; /* opname, args, onstack(value), addrof(onstack(value)) */
				if unlikely(Dee_function_generator_vrrot(self, 4))
					goto err; /* addrof(onstack(value)), opname, args, onstack(value) */
				if unlikely(Dee_function_generator_vrrot(self, 4))
					goto err; /* onstack(value), addrof(onstack(value)), opname, args */
				if unlikely(Dee_function_generator_vcall_DeeObject_InvokeOperatorTuple(self, (void const *)&DeeObject_PInvokeOperator))
					goto err; /* onstack(value), result */
				if unlikely(Dee_function_generator_vswap(self))
					goto err; /* result, onstack(value) */
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);
			}
		}	break;

		default:
			DeeError_Throwf(&DeeError_IllegalInstruction,
			                "Opcode not supported: %#.2" PRFx16 ":%#.2" PRFx16,
			                opcode, prefix_opcode);
			goto err;
		}
	}	break;

	default:
		DeeError_Throwf(&DeeError_IllegalInstruction,
		                "Opcode not supported: %#.2" PRFx16,
		                opcode);
		goto err;
	}
	return 0;
err:
	return -1;
}


/* Wrapper around `Dee_function_generator_geninstr()' to generate the entire basic block. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_genall(struct Dee_function_generator *__restrict self) {
	struct Dee_basic_block *block = self->fg_block;
	Dee_instruction_t const *instr;
	ASSERT(block->bb_mem_start != NULL);
	ASSERT(block->bb_mem_end == NULL);

	/* Generate text. */
	block->bb_deemon_end = block->bb_deemon_end_r;
	for (instr = block->bb_deemon_start; instr < block->bb_deemon_end;) {
		Dee_instruction_t const *next_instr = DeeAsm_NextInstr(instr);
		if unlikely(Dee_function_generator_geninstr(self, instr, &next_instr))
			goto err;
		ASSERT(block->bb_mem_end == NULL ||
		       block->bb_mem_end == (DREF struct Dee_memstate *)-1);
		if (block->bb_mem_end == (DREF struct Dee_memstate *)-1) {
			/* Special indicator meaning that this block needs to be re-compiled
			 * because its starting memory state had to be constrained. This can
			 * happen (e.g.) in cod like this:
			 * >> local x = "foo";
			 * >> do {
			 * >>     print x;    // In the first pass, "x" is the constant "foo"
			 * >>     x += "bar";
			 * >>     // This jumps to an already-compiled basic block with a more
			 * >>     // constraining memory state.
			 * >> } while (x != "foobarbar");
			 */
			ASSERT(block->bb_htext.hs_end == block->bb_htext.hs_start);
			ASSERT(block->bb_htext.hs_relc == 0);
			ASSERT(block->bb_hcold.hs_end == block->bb_hcold.hs_start);
			ASSERT(block->bb_hcold.hs_relc == 0);
			block->bb_mem_end = NULL;
			return 0;
		}
		instr = next_instr;
	}

	/* Assign the final memory state. */
	ASSERT(block->bb_mem_end == NULL);
	block->bb_mem_end = self->fg_state;
	Dee_memstate_incref(self->fg_state);
	if (block->bb_next != NULL) {
		/* Constrain the starting-state of the fallthru-block with the ending memory-state of `block' */
		struct Dee_basic_block *next_block = block->bb_next;
		Dee_code_addr_t addr = Dee_function_assembler_addrof(self->fg_assembler,
		                                                     next_block->bb_deemon_start);
		int error = Dee_basic_block_constrainwith(next_block, block->bb_mem_end, addr);
		if unlikely(error < 0)
			goto err;
	}
	return 0;
err:
	ASSERT(block->bb_mem_end == NULL);
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_DEEMON_C */
