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
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

#include "utils.h"

DECL_BEGIN

/************************************************************************/
/* DEEMON TO VSTACK MICRO-OP TRANSFORMATION                             */
/************************************************************************/

#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */

#ifdef NO_HOSTASM_DEBUG_PRINT
#define TARGET(x) case x:
#else /* NO_HOSTASM_DEBUG_PRINT */
#define TARGET(x) __IF0 { case x: HA_print(#x "\n"); }
#endif /* !NO_HOSTASM_DEBUG_PRINT */


/* Delete all no-longer-used locals until `next_instr' */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
delete_unused_locals(struct Dee_function_generator *__restrict self,
                     Dee_instruction_t const *next_instr) {
	if (self->fg_nextlastloc != NULL) {
		while (self->fg_nextlastloc->bbl_instr < next_instr) {
			/* Delete local after the last time it was read. */
			size_t lid = self->fg_nextlastloc->bbl_lid;
			if unlikely(Dee_function_generator_vdel_local(self, lid))
				goto err;
			++self->fg_nextlastloc;
		}
	}
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_prefix(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr,
                                    uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	switch (prefix_type) {
	case ASM_STACK:
		return Dee_function_generator_vdup_n(self, self->fg_state->ms_stackc - id1);
	case ASM_STATIC:
		return Dee_function_generator_vpush_static(self, id1);
	case ASM_EXTERN:
		return Dee_function_generator_vpush_extern(self, id1, id2, true);
	case ASM_GLOBAL:
		return Dee_function_generator_vpush_global(self, id1, true);
	case ASM_LOCAL:
		return Dee_function_generator_vpush_ulocal(self, instr, id1);
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
                                         Dee_instruction_t const *instr,
                                         uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	uintptr_t cfa_offset;
	struct Dee_memloc *src_loc;
	struct Dee_memstate *state;
	(void)id2;
	DO(Dee_function_generator_state_unshare(self));
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

	/* In case of a local, assert that the local is currently bound. */
	if (prefix_type == ASM_LOCAL && !(src_loc->ml_flags & MEMLOC_F_LOCAL_BOUND)) {
		if (src_loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND)
			return 1; /* Cannot address always-unbound local */
		DO(Dee_function_generator_gassert_local_bound(self, instr, id1));
		/* After a bound assertion, the local variable is guarantied to be bound. */
		src_loc->ml_flags |= MEMLOC_F_LOCAL_BOUND;
	}

	/* Prefixed location must contain a reference */
	if (src_loc->ml_flags & MEMLOC_F_NOREF) {
		DO(Dee_function_generator_gincref(self, src_loc, 1));
		src_loc->ml_flags &= ~MEMLOC_F_NOREF;
	}
	DO(Dee_function_generator_gflush(self, src_loc));
	ASSERT(!(src_loc->ml_flags & MEMLOC_F_NOREF));
	ASSERT(src_loc->ml_type == MEMLOC_TYPE_HSTACKIND);
	ASSERT(src_loc->ml_value.v_hstack.s_off == 0);
	cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
	return Dee_memstate_vpush_hstack(state, cfa_offset);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
perform_inplace_math_operation_with_vtop_addr(struct Dee_function_generator *__restrict self,
                                              uint16_t prefix_opcode, Dee_instruction_t const *prefix_instr) {
	/* TODO: Inline operator optimizations, and having a dedicatd `Dee_function_generator_vinplaceop()' */
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
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceAdd, VCALLOP_CC_INT, 2);
		case ASM_SUB: /* sub PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceSub, VCALLOP_CC_INT, 2);
		case ASM_MUL: /* mul PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceMul, VCALLOP_CC_INT, 2);
		case ASM_DIV: /* div PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceDiv, VCALLOP_CC_INT, 2);
		case ASM_MOD: /* mod PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceMod, VCALLOP_CC_INT, 2);
		case ASM_SHL: /* shl PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceShl, VCALLOP_CC_INT, 2);
		case ASM_SHR: /* shr PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceShr, VCALLOP_CC_INT, 2);
		case ASM_AND: /* and PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceAnd, VCALLOP_CC_INT, 2);
		case ASM_OR:  /* or  PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceOr, VCALLOP_CC_INT, 2);
		case ASM_XOR: /* xor PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceXor, VCALLOP_CC_INT, 2);
		case ASM_POW: /* pow PREFIX, pop */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplacePow, VCALLOP_CC_INT, 2);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();

	case ASM_INC: /* inc PREFIX */
		return Dee_function_generator_vcallapi(self, &DeeObject_Inc, VCALLOP_CC_INT, 1);
	case ASM_DEC: /* dec PREFIX */
		return Dee_function_generator_vcallapi(self, &DeeObject_Dec, VCALLOP_CC_INT, 1);

	case ASM_ADD_SIMM8:   /* add PREFIX, $<Simm8> */
	case ASM_SUB_SIMM8:   /* sub PREFIX, $<Simm8> */
	case ASM_MUL_SIMM8:   /* mul PREFIX, $<Simm8> */
	case ASM_DIV_SIMM8:   /* div PREFIX, $<Simm8> */
	case ASM_MOD_SIMM8: { /* mod PREFIX, $<Simm8> */
		int8_t Simm8 = (int8_t)prefix_instr[1];
		DO(Dee_function_generator_vpush_Simm8(self, Simm8));
		switch (prefix_opcode) {
		case ASM_ADD_SIMM8: /* add PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceAddInt8, VCALLOP_CC_INT, 2);
		case ASM_SUB_SIMM8: /* sub PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceSubInt8, VCALLOP_CC_INT, 2);
		case ASM_MUL_SIMM8: /* mul PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceMulInt8, VCALLOP_CC_INT, 2);
		case ASM_DIV_SIMM8: /* div PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceDivInt8, VCALLOP_CC_INT, 2);
		case ASM_MOD_SIMM8: /* mod PREFIX, $<Simm8> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceModInt8, VCALLOP_CC_INT, 2);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	}	break;

	case ASM_SHL_IMM8:   /* shl PREFIX, $<imm8> */
	case ASM_SHR_IMM8: { /* shr PREFIX, $<imm8> */
		uint8_t imm8 = (uint8_t)prefix_instr[1];
		DO(Dee_function_generator_vpush_imm8(self, imm8));
		switch (prefix_opcode) {
		case ASM_SHL_IMM8: /* shl PREFIX, $<imm8> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceShlUInt8, VCALLOP_CC_INT, 2);
		case ASM_SHR_IMM8: /* shr PREFIX, $<imm8> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceShrUInt8, VCALLOP_CC_INT, 2);
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
		DO(Dee_function_generator_vpush_imm32(self, imm32));
		switch (prefix_opcode) {
		case ASM_ADD_IMM32: /* add PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceAddUInt32, VCALLOP_CC_INT, 2);
		case ASM_SUB_IMM32: /* sub PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceSubUInt32, VCALLOP_CC_INT, 2);
		case ASM_AND_IMM32: /* and PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceAndUInt32, VCALLOP_CC_INT, 2);
		case ASM_OR_IMM32:  /* or  PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceOrUInt32, VCALLOP_CC_INT, 2);
		case ASM_XOR_IMM32: /* xor PREFIX, $<imm32> */
			return Dee_function_generator_vcallapi(self, &DeeObject_InplaceXorUInt32, VCALLOP_CC_INT, 2);
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

/* File, value  ->  N/A */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gen_print_to_file(struct Dee_function_generator *__restrict self,
                  void const *api_function) {
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		bool repr = false;
		char const *print_after = NULL; /* 1-character string to print *after* the object (or NULL) */
		DeeTypeObject *value_type;
		struct Dee_memloc *value_loc;
		uint16_t print_operator;
		if (api_function == (void const *)&DeeFile_PrintObject) {
			/* ... */
		} else if (api_function == (void const *)&DeeFile_PrintObjectRepr) {
			repr = true;
		} else if (api_function == (void const *)&DeeFile_PrintObjectSp) {
			print_after = " ";
		} else if (api_function == (void const *)&DeeFile_PrintObjectReprSp) {
			repr        = true;
			print_after = " ";
		} else if (api_function == (void const *)&DeeFile_PrintObjectNl) {
			print_after = "\n";
		} else if (api_function == (void const *)&DeeFile_PrintObjectReprNl) {
			repr        = true;
			print_after = "\n";
		} else {
			goto fallback;
		}

		/* Check for special case: the object being printed is a constant.
		 * In this case, try to encode a call `DeeFile_WriteAll(file, VALUEOF(str constant))' */
		value_loc = Dee_function_generator_vtop(self);
		print_operator = repr ? OPERATOR_REPR : OPERATOR_STR;
		if (value_loc->ml_type == MEMLOC_TYPE_CONST) {
			DeeObject *constval = value_loc->ml_value.v_const;
			if (DeeType_IsOperatorConstexpr(Dee_TYPE(constval), print_operator)) {
				char const *utf8;
				if (repr || !DeeString_Check(constval)) {
					constval = repr ? DeeObject_Repr(constval)
					                : DeeObject_Str(constval);
					if unlikely(!constval)
						goto handle_error_and_do_fallback;
					constval = Dee_function_generator_inlineref(self, constval);
					if unlikely(!constval)
						goto err;
				}
				if (print_after != NULL) {
					/* XXX: Why not merge the constant string with `print_after'?
					 *      And on that note: why not try to merge multiple consecutive
					 *      constant strings being printed into 1 big one? */
				}

				/* Direct write the utf-8 representation of the string to the file. */
				utf8 = DeeString_AsUtf8(constval);
				if unlikely(!utf8)
					goto handle_error_and_do_fallback;
				if (print_after != NULL)
					DO(Dee_function_generator_vdup(self));
				DO(Dee_function_generator_vpop(self)); /* [File], File */
				if (WSTR_LENGTH(utf8) == 0) {
					/* Special case: printing an empty string does nothing */
					DO(Dee_function_generator_vpop(self)); /* [File] */
				} else if (WSTR_LENGTH(utf8) == 1) {
					/* Special case: printing a single character allows us to use `DeeFile_Putc()' */
					DO(Dee_function_generator_vpush_immINT(self, utf8[0]));                         /* [File], File, ch */
					DO(Dee_function_generator_vcallapi(self, &DeeFile_Putc, VCALLOP_CC_RAWINT, 2)); /* [File], status */
					DO(Dee_function_generator_vcheckerr(self, GETC_ERR));                           /* [File] */
				} else {
					DO(Dee_function_generator_vpush_addr(self, utf8));                                    /* [File], File, utf8 */
					DO(Dee_function_generator_vpush_immSIZ(self, WSTR_LENGTH(utf8)));                     /* [File], File, utf8, length */
					DO(Dee_function_generator_vcallapi(self, &DeeFile_WriteAll, VCALLOP_CC_M1INTPTR, 3)); /* [File], print_status */
					DO(Dee_function_generator_vpop(self));                                                /* [File] */
				}
				goto handle_print_after_and_pop_file;
			}
		}

		/* If the type of object being printed is known, then we can inline the print-operator. */
		value_type = Dee_memloc_typeof(value_loc);
		if (value_type != NULL && DeeType_InheritOperator(value_type, print_operator)) {
			api_function = repr ? value_type->tp_cast.tp_printrepr
			                    : value_type->tp_cast.tp_print;
		} else {
			api_function = repr ? (void const *)&DeeObject_PrintRepr
			                    : (void const *)&DeeObject_Print;
		}

		DO(Dee_function_generator_vswap(self)); /* value, File */
		if (print_after != NULL) {
			DO(Dee_function_generator_vdup(self));     /* value, File, File */
			DO(Dee_function_generator_vrrot(self, 3)); /* File, value, File */
		}
		DO(Dee_function_generator_vpush_addr(self, (void const *)&DeeFile_WriteAll));  /* [File], value, File, &DeeFile_WriteAll */
		DO(Dee_function_generator_vswap(self));                                        /* [File], value, &DeeFile_WriteAll, File */
		DO(Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_NEGINT, 3)); /* [File], print_status */
		DO(Dee_function_generator_vpop(self));                                         /* [File] */
handle_print_after_and_pop_file:
		if (print_after != NULL) {
			DO(Dee_function_generator_vpush_immINT(self, *print_after));                    /* File, ch */
			DO(Dee_function_generator_vcallapi(self, &DeeFile_Putc, VCALLOP_CC_RAWINT, 2)); /* status */
			return Dee_function_generator_vcheckerr(self, GETC_ERR);                        /* - */
		}
		return 0;
	}
fallback:
	return Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_INT, 2);
handle_error_and_do_fallback:
	DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
	goto fallback;
err:
	return -1;
}

/* Generate code for a print-to-stdout instruction. The caller has left stdout in vtop.
 * It is assumed that this function will pop the caller-pushed stdout slot. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gen_print_with_stdout_in_vtop(struct Dee_function_generator *__restrict self,
                              Dee_instruction_t const *instr, uint16_t opcode,
                              bool repr) {
	void const *api_function;
	switch (opcode) {

	TARGET(ASM_PRINTNL)
		return Dee_function_generator_vcallapi(self, &DeeFile_PrintNl, VCALLOP_CC_INT, 1);

	TARGET(ASM_PRINT)
	TARGET(ASM_PRINT_SP)
	TARGET(ASM_PRINT_NL)
	TARGET(ASM_PRINTALL)
	TARGET(ASM_PRINTALL_SP)
	TARGET(ASM_PRINTALL_NL)
		DO(Dee_function_generator_vswap(self)); /* File, value */
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

	TARGET(ASM_PRINT_C)
	TARGET(ASM16_PRINT_C)
	TARGET(ASM_PRINT_C_SP)
	TARGET(ASM16_PRINT_C_SP)
	TARGET(ASM_PRINT_C_NL)
	TARGET(ASM16_PRINT_C_NL) {
		uint16_t cid = instr[1];
		if (opcode > 0xff)
			cid = UNALIGNED_GETLE16(instr + 2);
		DO(Dee_function_generator_vpush_cid(self, cid)); /* File, value */
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
	return gen_print_to_file(self, api_function);
err:
	return -1;
}

/* For the non-file-print instructions, load the `stdout' file stream only the first
 * time a print-like instruction is reached. We then keep that object in a hidden
 * local `MEMSTATE_XLOCAL_STDOUT' until the end of the basic block, or until
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
	Dee_instruction_t const *iter, *print_block_end, *print_block_end_limit;
	DO(Dee_function_generator_state_unshare(self));

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
	for (iter = instr; iter < print_block_end;) {
		int temp;
		Dee_instruction_t const *next = DeeAsm_NextInstr(iter);
		uint16_t opcode = iter[0];
		if (ASM_ISEXTENDED(opcode))
			opcode = (opcode << 8) | iter[1];
		switch (opcode) {
		CASE_ASM_PRINT:
do_handle_ASM_PRINT:
			DO(Dee_function_generator_vpush_xlocal(self, iter, MEMSTATE_XLOCAL_STDOUT));
			temp = gen_print_with_stdout_in_vtop(self, iter, opcode, repr);
			break;
		TARGET(ASM_REPR) {
			/* Special case for when the ASM_REPR is followed by a print-opcode. */
			uint16_t next_opcode = iter[1];
			if (ASM_ISEXTENDED(next_opcode))
				next_opcode = (next_opcode << 8) | iter[2];
			switch (next_opcode) {
			CASE_ASM_PRINT_AFTER_REPR:
				repr = true;
				opcode = next_opcode;
				++iter;
				next = DeeAsm_NextInstr(iter);
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
		if unlikely(self->fg_block->bb_mem_end == (DREF struct Dee_memstate *)-1)
			return 0;
		DO(delete_unused_locals(self, next));
		iter = next;
		repr = false;
	}
	*p_next_instr = iter;
	return Dee_function_generator_vdel_xlocal(self, MEMSTATE_XLOCAL_STDOUT); /* Delete the stdout-cache */
err:
	return -1;
}

/* Given a vstack that looks like this:
 * >> [...], value
 * Check if whatever instruction eventually pops `value' requires `value' to
 * be a valid deemon object. Now this might sound like something that should
 * always be the case, and you'd be right, but there are situations where
 * it's OK if an object isn't valid (i.e. not safe to dereference), such as
 * in case of `ASM_ISNONE' or `ASM_CMP_SO' and `ASM_CMP_DO', all of which
 * only look at the address of the object in vtop.
 *
 * This special handling makes it so no extra reference is needed for code like:
 * >> global testGlobal = none;
 * >> function testFunction() {
 * >>     if (testGlobal is none)
 * >>         testGlobal = 42;
 * >> }
 * asm(testFunction):
 * >>     push global @testGlobal   # This push doesn't need to incref...
 * >>     instanceof top, none      # ... because this use of the value doesn't need a reference
 * >>     jf   pop, 1f
 * >>     mov  const @42, global @testGlobal
 * >> 1:  ret
 */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
matching_pop_requires_reference(Dee_instruction_t const *instr,
                                Dee_instruction_t const *end,
                                uint16_t stacksz) {
	/* TODO: This function should be its own step in the loader.
	 * - Have a sorted list of all instructions and their n'th pushed items where
	 *   the eventual consumer doesn't need a reference to the linked object.
	 * - Implement this scan by keeping a vstack that knows the pushing instruction
	 *   and n'th value pushed by said instruction. Whenever a value is popped in
	 *   such a way that its value isn't used, mark the instruction that pushed the
	 *   value to indicate that the resulting reference isn't used.
	 * - If this step is run *after* locuse, we can even integrate this with locals
	 *   being used to optimize code like this:
	 *   >> local x = globalValue;  // This doesn't need to acquire a reference, or a lock
	 *   >> local y = x === true;   // "x" is never used again after this point
	 * - At the same time, also detect stack slots that are always popped unconditionally,
	 *   which then become candidates for early deletion (by replacing their value with a
	 *   reference-less, constant `Dee_None'). This can then be used to optimize `__stack'
	 *   variables that aren't actually used until the end of their relevant scope.
	 */

	uint16_t sptrack = stacksz - 1; /* Absolute stack address which we care about. */
	while (instr < end) {
		uint16_t opcode;
		opcode = instr[0];
		if (ASM_ISEXTENDED(opcode))
			opcode = (opcode << 8) | instr[1];
		switch (opcode) {

		case ASM_JF:
		case ASM_JF16:
		case ASM_JT:
		case ASM_JT16:
		case ASM_FOREACH:
		case ASM_FOREACH16:
		case ASM_JMP:
		case ASM_JMP16:
		case ASM32_JMP:
			/* Technically: if stack address is used on at least 1 branch */
			goto yes;

		default: {
			Dee_instruction_t const *next;
			uint16_t new_stacksz = stacksz;
			uint16_t sp_add, sp_sub;
			next = DeeAsm_NextInstrEf(instr, &new_stacksz, &sp_add, &sp_sub);
			if (sptrack >= (stacksz - sp_sub) && sptrack < stacksz) {
				/* Found the instruction which eventually pops the value in question. */
				switch (opcode) {

				case ASM_ISNONE:
				case ASM_CMP_SO:
				case ASM_CMP_DO:
				case ASM_POP:
				case ASM_ADJSTACK:
				case ASM16_ADJSTACK:
					/* If a stack location is popped by these, it doesn't need to be a reference */
					return false;

				case ASM_POP_N: {
					Dee_vstackaddr_t n;
					n = instr[1] + 2;
					__IF0 { case ASM16_POP_N: n = UNALIGNED_GETLE16(instr + 2) + 2; }
					if (sptrack == stacksz - n)
						return false; /* Location will simply get overwritten! */
					sptrack = stacksz - n;
					goto continue_with_next;
				}	break;

				case ASM_DUP_N: {
					Dee_vstackaddr_t n;
					n = instr[1] + 2;
					__IF0 { case ASM16_DUP_N: n = UNALIGNED_GETLE16(instr + 2) + 2; }
					if (sptrack == stacksz - n)
						goto yes; /* Technically: if either original or dup'd location needs reference... */
					goto continue_with_next;
				}	break;

				case ASM_SWAP:
					if (sptrack == stacksz - 1) {
						--sptrack;
					} else {
						++sptrack;
					}
					goto continue_with_next;

				case ASM_LROT: {
					Dee_vstackaddr_t n;
					n = instr[1] + 3;
					__IF0 { case ASM16_LROT: n = UNALIGNED_GETLE16(instr + 2) + 3; }
					if (sptrack == stacksz - n) {
						sptrack += (n - 1);
					} else {
						--sptrack;
					}
					goto continue_with_next;
				}	break;

				case ASM_RROT: {
					Dee_vstackaddr_t n;
					n = instr[1] + 3;
					__IF0 { case ASM16_RROT: n = UNALIGNED_GETLE16(instr + 2) + 3; }
					if (sptrack == stacksz - 1) {
						sptrack -= (n - 1);
					} else {
						++sptrack;
					}
					goto continue_with_next;
				}	break;

				default: break;
				}
				goto yes;
			}
continue_with_next:
			stacksz = new_stacksz;
			instr   = next;
		}	break;

		}
	}
yes:
	return true;
}


/* Convert a single deemon instruction `instr' to host assembly and adjust the host memory
 * state according to the instruction in question. This is the core function to parse deemon
 * code and convert it to host assembly.
 * @param: p_next_instr: [inout] Pointer to the next instruction (may be overwritten if the
 *                               generated instruction was merged with its successor, as is
 *                               the case for `ASM_REPR' when followed by print-instructions)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_geninstr(struct Dee_function_generator *__restrict self,
                                Dee_instruction_t const *instr,
                                Dee_instruction_t const **p_next_instr) {
	uint16_t opcode = instr[0];
	if (ASM_ISEXTENDED(opcode))
		opcode = (opcode << 8) | instr[1];
	switch (opcode) {

	TARGET(ASM_DELOP)
	TARGET(ASM16_DELOP)
	TARGET(ASM_NOP)
	TARGET(ASM16_NOP)
		/* No-op instructions */
		break;

	TARGET(ASM_SWAP)
		return Dee_function_generator_vswap(self);
	TARGET(ASM_LROT)
		return Dee_function_generator_vlrot(self, instr[1] + 3);
	TARGET(ASM16_LROT)
		return Dee_function_generator_vlrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	TARGET(ASM_RROT)
		return Dee_function_generator_vrrot(self, instr[1] + 3);
	TARGET(ASM16_RROT)
		return Dee_function_generator_vrrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	TARGET(ASM_DUP)
		return Dee_function_generator_vdup(self);
	TARGET(ASM_DUP_N)
		return Dee_function_generator_vdup_n(self, instr[1] + 2);
	TARGET(ASM16_DUP_N)
		return Dee_function_generator_vdup_n(self, UNALIGNED_GETLE16(instr + 2) + 2);
	TARGET(ASM_POP)
		return Dee_function_generator_vpop(self);
	TARGET(ASM_POP_N)
		return Dee_function_generator_vpop_n(self, instr[1] + 2);
	TARGET(ASM16_POP_N)
		return Dee_function_generator_vpop_n(self, UNALIGNED_GETLE16(instr + 2) + 2);

	TARGET(ASM_ADJSTACK) {
		Dee_vstackoff_t delta;
		delta = *(int8_t const *)(instr + 1);
		__IF0 { TARGET(ASM16_ADJSTACK) delta = (int16_t)UNALIGNED_GETLE16(instr + 2); }
		if (delta > 0) {
			do {
				DO(Dee_function_generator_vpush_const(self, Dee_None));
			} while (--delta > 0);
		} else if (delta < 0) {
			return Dee_function_generator_vpopmany(self, (Dee_vstackaddr_t)(-delta));
		}
	}	break;

	TARGET(ASM_POP_LOCAL)
		return Dee_function_generator_vpop_ulocal(self, instr[1]);
	TARGET(ASM16_POP_LOCAL)
		return Dee_function_generator_vpop_ulocal(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_NONE)
		return Dee_function_generator_vpush_const(self, Dee_None);
	TARGET(ASM_PUSH_THIS_MODULE)
		return Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code->co_module);
	TARGET(ASM_PUSH_THIS_FUNCTION)
		return Dee_function_generator_vpush_this_function(self);

	TARGET(ASM_PUSH_MODULE) {
		DeeModuleObject *code_module;
		DeeModuleObject *push_module;
		uint16_t mid;
		mid = instr[1];
		__IF0 { TARGET(ASM16_PUSH_MODULE) mid = UNALIGNED_GETLE16(instr + 2); }
		code_module = self->fg_assembler->fa_code->co_module;
		if unlikely(mid >= code_module->mo_importc)
			return err_illegal_mid(mid);
		push_module = code_module->mo_importv[mid];
		return Dee_function_generator_vpush_const(self, (DeeObject *)push_module);
	}	break;

	TARGET(ASM_PUSH_ARG)
		return Dee_function_generator_vpush_arg(self, instr, instr[1]);
	TARGET(ASM16_PUSH_ARG)
		return Dee_function_generator_vpush_arg(self, instr, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_CONST)
		return Dee_function_generator_vpush_cid(self, instr[1]);
	TARGET(ASM16_PUSH_CONST) 
		return Dee_function_generator_vpush_cid(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_REF)
		return Dee_function_generator_vpush_rid(self, instr[1]);
	TARGET(ASM16_PUSH_REF)
		return Dee_function_generator_vpush_rid(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_EXTERN)
	TARGET(ASM16_PUSH_EXTERN)
	TARGET(ASM_PUSH_GLOBAL)
	TARGET(ASM16_PUSH_GLOBAL) {
		bool ref = matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                           (uint16_t)self->fg_state->ms_stackc + 1);
		switch (opcode) {
		case ASM_PUSH_EXTERN:
			return Dee_function_generator_vpush_extern(self, instr[1], instr[2], ref);
		case ASM16_PUSH_EXTERN:
			return Dee_function_generator_vpush_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4), ref);
		case ASM_PUSH_GLOBAL:
			return Dee_function_generator_vpush_global(self, instr[1], ref);
		case ASM16_PUSH_GLOBAL:
			return Dee_function_generator_vpush_global(self, UNALIGNED_GETLE16(instr + 2), ref);
		default: __builtin_unreachable();
		}
	}	break;

	TARGET(ASM_PUSH_LOCAL)
		return Dee_function_generator_vpush_ulocal(self, instr, instr[1]);
	TARGET(ASM16_PUSH_LOCAL)
		return Dee_function_generator_vpush_ulocal(self, instr, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_RET_NONE)
		DO(Dee_function_generator_vpush_const(self, Dee_None));
		ATTR_FALLTHROUGH
	TARGET(ASM_RET)
		return Dee_function_generator_vret(self);

	TARGET(ASM_THROW)
		return Dee_function_generator_vcallapi(self, &DeeError_Throw, VCALLOP_CC_EXCEPT, 1);

	//TODO: TARGET(ASM_SETRET)

	TARGET(ASM_DEL_LOCAL)
		return Dee_function_generator_vdel_ulocal(self, instr[1]);
	TARGET(ASM16_DEL_LOCAL)
		return Dee_function_generator_vdel_ulocal(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_CALL_KW) {
		uint8_t argc;
		uint16_t kw_cid;
		argc = instr[1];
		kw_cid  = instr[2];
		__IF0 {
	TARGET(ASM16_CALL_KW)
			argc   = instr[2];
			kw_cid = UNALIGNED_GETLE16(instr + 3);
		}
		DO(Dee_function_generator_vpush_cid(self, kw_cid));  /* func, [args...], kw */
		return Dee_function_generator_vopcallkw(self, argc); /* result */
	}	break;

	TARGET(ASM_CALL_TUPLE_KW) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_CALL_TUPLE_KW) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));    /* func, args, kw */
		return Dee_function_generator_vopcalltuplekw(self); /* result */
	}	break;

	TARGET(ASM_PUSH_BND_ARG)
		return Dee_function_generator_vbound_arg(self, instr[1]);
	TARGET(ASM16_PUSH_BND_ARG)
		return Dee_function_generator_vbound_arg(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_BND_EXTERN)
		return Dee_function_generator_vbound_extern(self, instr[1], instr[2]);
	TARGET(ASM16_PUSH_BND_EXTERN)
		return Dee_function_generator_vbound_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));

	TARGET(ASM_PUSH_BND_GLOBAL)
		return Dee_function_generator_vbound_global(self, instr[1]);
	TARGET(ASM16_PUSH_BND_GLOBAL)
		return Dee_function_generator_vbound_global(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_BND_LOCAL)
		return Dee_function_generator_vbound_ulocal(self, instr, instr[1]);
	TARGET(ASM16_PUSH_BND_LOCAL)
		return Dee_function_generator_vbound_ulocal(self, instr, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_JF)
	TARGET(ASM_JF16)
	TARGET(ASM_JT)
	TARGET(ASM_JT16)
	TARGET(ASM_JMP)
	TARGET(ASM_JMP16)
	TARGET(ASM32_JMP) {
		bool jump_if_true;
		struct Dee_jump_descriptor *desc;
do_jcc:
		desc = Dee_jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
		ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
		        Dee_function_assembler_addrof(self->fg_assembler, instr));
		if (opcode == ASM_JF || opcode == ASM_JF16) {
			jump_if_true = false;
		} else if (opcode == ASM_JT || opcode == ASM_JT16) {
			jump_if_true = true;
		} else {
			DO(Dee_function_generator_vpush_const(self, Dee_True));
			jump_if_true = true;
		}
		return Dee_function_generator_vjcc(self, desc, instr, jump_if_true);
	}	break;

	TARGET(ASM_FOREACH)
	TARGET(ASM_FOREACH16) {
		struct Dee_jump_descriptor *desc;
		desc = Dee_jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
		ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
		        Dee_function_assembler_addrof(self->fg_assembler, instr));
		return Dee_function_generator_vforeach(self, desc, false);
	}	break;

	//TODO: TARGET(ASM_JMP_POP)
	//TODO: TARGET(ASM_JMP_POP_POP)

	TARGET(ASM_OPERATOR)
	TARGET(ASM16_OPERATOR) {
		uint16_t opname;
		uint8_t argc;
		if (opcode == ASM_OPERATOR) {
			opname = instr[1];
			argc   = instr[2];
		} else {
			opname = UNALIGNED_GETLE16(instr + 2);
			argc   = instr[4];
		}
		return Dee_function_generator_vop(self, opname, argc + 1, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_OPERATOR_TUPLE)
		return Dee_function_generator_voptuple(self, instr[1], VOP_F_PUSHRES);
	TARGET(ASM16_OPERATOR_TUPLE)
		return Dee_function_generator_voptuple(self, UNALIGNED_GETLE16(instr + 2), VOP_F_PUSHRES);

	TARGET(ASM_CALL)
		return Dee_function_generator_vopcall(self, instr[1]);

	TARGET(ASM_CALL_TUPLE)
		return Dee_function_generator_vopcalltuple(self);

	TARGET(ASM_DEL_GLOBAL)
		return Dee_function_generator_vdel_global(self, instr[1]);
	TARGET(ASM16_DEL_GLOBAL)
		return Dee_function_generator_vdel_global(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_SUPER)
		return Dee_function_generator_vopsuper(self);

	TARGET(ASM_SUPER_THIS_R) {
		uint16_t rid;
		rid = instr[1];
		__IF0 { TARGET(ASM16_SUPER_THIS_R) rid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self));
		DO(Dee_function_generator_vpush_rid(self, rid));
		return Dee_function_generator_vopsuper(self);
	}	break;

	TARGET(ASM_POP_STATIC)
		return Dee_function_generator_vpop_static(self, instr[1]);
	TARGET(ASM16_POP_STATIC)
		return Dee_function_generator_vpop_static(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_POP_EXTERN)
		return Dee_function_generator_vpop_extern(self, instr[1], instr[2]);
	TARGET(ASM16_POP_EXTERN)
		return Dee_function_generator_vpop_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));
	TARGET(ASM_POP_GLOBAL)
		return Dee_function_generator_vpop_global(self, instr[1]);
	TARGET(ASM16_POP_GLOBAL)
		return Dee_function_generator_vpop_global(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_VARARGS)
		return Dee_function_generator_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_VARARGS);
	TARGET(ASM_PUSH_VARKWDS)
		return Dee_function_generator_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_VARKWDS);

	TARGET(ASM_PUSH_STATIC)
		return Dee_function_generator_vpush_static(self, instr[1]);
	TARGET(ASM16_PUSH_STATIC)
		return Dee_function_generator_vpush_static(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_CAST_TUPLE)
		DO(Dee_function_generator_vcallapi(self, &DeeTuple_FromSequence, VCALLOP_CC_OBJECT, 1));
		return Dee_function_generator_vsettyp(self, &DeeTuple_Type);

	TARGET(ASM_CAST_LIST)
		DO(Dee_function_generator_vcallapi(self, &DeeList_FromSequence, VCALLOP_CC_OBJECT, 1));
		return Dee_function_generator_vsettyp(self, &DeeList_Type);

	TARGET(ASM_PACK_TUPLE)
	TARGET(ASM16_PACK_TUPLE)
	TARGET(ASM_PACK_LIST)
	TARGET(ASM16_PACK_LIST) {
		/* Implement these by creating the list/tuple raw, and then filling its items. */
		uint16_t elemc = instr[1];
		bool is_list = (opcode & 0xff) == ASM_PACK_LIST;
		if (opcode > 0xff)
			elemc = UNALIGNED_GETLE16(instr + 2);
		DO(Dee_function_generator_vpush_immSIZ(self, elemc)); /* [elems...], elemc */
		DO(Dee_function_generator_vcallapi(self, is_list ? (void const *)&DeeList_NewUninitialized
		                                                 : (void const *)&DeeTuple_NewUninitialized,
		                                   VCALLOP_CC_OBJECT, 1)); /* [elems...], ref:seq */
		DO(Dee_function_generator_vdup(self));                     /* [elems...], ref:seq, seq */
		DO(is_list ? Dee_function_generator_vind(self, offsetof(DeeListObject, l_list.ol_elemv))
		           : Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* [elems...], ref:seq, elemv */
		DO(Dee_function_generator_vswap(self));                    /* [elems...], elemv, ref:seq */
		DO(Dee_function_generator_vrrot(self, elemc + 2));         /* ref:seq, [elems...], elemv */
		while (elemc) {
			--elemc;
			DO(Dee_function_generator_vswap(self));                                     /* ref:seq, [elems...], elemv, elem */
			DO(Dee_function_generator_vref(self));                                      /* ref:seq, [elems...], elemv, ref:elem */
			DO(Dee_function_generator_vpopind(self, elemc * sizeof(DREF DeeObject *))); /* ref:seq, [elems...], elemv */
		}
		DO(Dee_function_generator_vpop(self)); /* ref:seq */
		DO(Dee_function_generator_vsettyp(self, is_list ? &DeeList_Type : &DeeTuple_Type));
		if (is_list) {
			ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));
			Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF; /* Inherited by `DeeGC_Track()' */
			DO(Dee_function_generator_vcallapi(self, &DeeGC_Track, VCALLOP_CC_RAWINT, 1));
			ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
			Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF; /* Returned by `DeeGC_Track()' */
		}
	}	break;

	TARGET(ASM_UNPACK)
		return Dee_function_generator_vopunpack(self, instr[1]);
	TARGET(ASM16_UNPACK)
		return Dee_function_generator_vopunpack(self, UNALIGNED_GETLE16(instr + 2));
	
	TARGET(ASM_CONCAT)
		return Dee_function_generator_vopconcat(self);

	TARGET(ASM_EXTEND)
		return Dee_function_generator_vopextend(self, instr[1]);

	TARGET(ASM_TYPEOF)
		return Dee_function_generator_voptypeof(self, true);

	TARGET(ASM_CLASSOF)
		return Dee_function_generator_vopclassof(self, true);

	TARGET(ASM_SUPEROF)
		return Dee_function_generator_vopsuperof(self);

	//TODO: TARGET(ASM_INSTANCEOF)
	//TODO: TARGET(ASM_IMPLEMENTS)

	TARGET(ASM_STR)
		return Dee_function_generator_vopstr(self);
	TARGET(ASM_BOOL)
		return Dee_function_generator_vopbool(self);
	TARGET(ASM_NOT)
		return Dee_function_generator_vopnot(self);

	TARGET(ASM_REPR) {
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

		TARGET(ASM_FPRINT)
		TARGET(ASM_FPRINT_SP)
		TARGET(ASM_FPRINT_NL) {
			void const *api_function;
			*p_next_instr = DeeAsm_NextInstr(next_instr);
			switch (opcode) {
			TARGET(ASM_FPRINT)    /* print top, pop */
				api_function = (void const *)&DeeFile_PrintObjectRepr;
				break;
			TARGET(ASM_FPRINT_SP) /* print top, pop, sp */
				api_function = (void const *)&DeeFile_PrintObjectReprSp;
				break;
			TARGET(ASM_FPRINT_NL) /* print top, pop, nl */
				api_function = (void const *)&DeeFile_PrintObjectReprNl;
				break;
			default: __builtin_unreachable();
			}
			DO(Dee_function_generator_vswap(self));    /* value, File */
			DO(Dee_function_generator_vdup(self));     /* value, File, File */
			DO(Dee_function_generator_vlrot(self, 3)); /* File, File, value */
			return Dee_function_generator_vcallapi(self, api_function, VCALLOP_CC_OBJECT, 2);
		}	break;

		TARGET(ASM_SHL)
			/* Special handling needed here! */
			*p_next_instr = DeeAsm_NextInstr(next_instr);
			return Dee_function_generator_vcallapi(self, &libhostasm_rt_DeeObject_ShlRepr, VCALLOP_CC_OBJECT, 2);

		default: break;
		}
		return Dee_function_generator_vop(self, OPERATOR_REPR, 1, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_ASSIGN)
		return Dee_function_generator_vop(self, OPERATOR_ASSIGN, 2, VOP_F_NORMAL);
	TARGET(ASM_MOVE_ASSIGN)
		return Dee_function_generator_vop(self, OPERATOR_MOVEASSIGN, 2, VOP_F_NORMAL);

	TARGET(ASM_COPY)
		return Dee_function_generator_vop(self, OPERATOR_COPY, 1, VOP_F_PUSHRES);
	TARGET(ASM_DEEPCOPY)
		return Dee_function_generator_vop(self, OPERATOR_DEEPCOPY, 1, VOP_F_PUSHRES);
	TARGET(ASM_GETATTR)
		return Dee_function_generator_vopgetattr(self);
	TARGET(ASM_DELATTR)
		return Dee_function_generator_vopdelattr(self);
	TARGET(ASM_SETATTR)
		return Dee_function_generator_vopsetattr(self);
	TARGET(ASM_BOUNDATTR)
		return Dee_function_generator_vopboundattr(self);
	
	TARGET(ASM_GETATTR_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_GETATTR_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));
		return Dee_function_generator_vopgetattr(self);
	}	break;

	TARGET(ASM_DELATTR_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_DELATTR_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));
		return Dee_function_generator_vopdelattr(self);
	}	break;

	TARGET(ASM_SETATTR_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_SETATTR_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));
		DO(Dee_function_generator_vswap(self));
		return Dee_function_generator_vopsetattr(self);
	}	break;

	TARGET(ASM_GETATTR_THIS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_GETATTR_THIS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self));
		DO(Dee_function_generator_vpush_cid(self, cid));
		return Dee_function_generator_vopgetattr(self);
	}	break;

	TARGET(ASM_DELATTR_THIS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_DELATTR_THIS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self));
		DO(Dee_function_generator_vpush_cid(self, cid));
		return Dee_function_generator_vopdelattr(self);
	}	break;

	TARGET(ASM_SETATTR_THIS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_SETATTR_THIS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self));
		DO(Dee_function_generator_vpush_cid(self, cid));
		DO(Dee_function_generator_vlrot(self, 3));
		return Dee_function_generator_vopsetattr(self);
	}	break;

	TARGET(ASM_CMP_EQ)
	TARGET(ASM_CMP_NE)
	TARGET(ASM_CMP_GE)
	TARGET(ASM_CMP_LO)
	TARGET(ASM_CMP_LE)
	TARGET(ASM_CMP_GR) {
		/* In the case of the cmp-instructions, check if the next instructions
		 * is `jf' or `jt', and if so: generate a call to `DeeObject_Compare*'
		 * without the *Object suffix. That way, we don't need DeeObject_Bool,
		 * and also don't have to decref the comparison result!
		 * 
		 * Similarly, we can do the same if `ASM_BOOL' appears next, in which
		 * case we don't have to make 2 calls, or have the result be a reference */
		/* TODO: Figure out the instruction that eventually pops the result of the CMP */

		switch (opcode) {
		case ASM_CMP_EQ: return Dee_function_generator_vop(self, OPERATOR_EQ, 2, VOP_F_PUSHRES);
		case ASM_CMP_NE: return Dee_function_generator_vop(self, OPERATOR_NE, 2, VOP_F_PUSHRES);
		case ASM_CMP_GE: return Dee_function_generator_vop(self, OPERATOR_GE, 2, VOP_F_PUSHRES);
		case ASM_CMP_LO: return Dee_function_generator_vop(self, OPERATOR_LO, 2, VOP_F_PUSHRES);
		case ASM_CMP_LE: return Dee_function_generator_vop(self, OPERATOR_LE, 2, VOP_F_PUSHRES);
		case ASM_CMP_GR: return Dee_function_generator_vop(self, OPERATOR_GR, 2, VOP_F_PUSHRES);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	}	break;

	TARGET(ASM_CLASS_C)      /* push class pop, const <imm8> */
	TARGET(ASM16_CLASS_C)    /* push class pop, const <imm8> */
	TARGET(ASM_CLASS_GC)     /* push class global <imm8>, const <imm8> */
	TARGET(ASM16_CLASS_GC)   /* push class global <imm8>, const <imm8> */
	TARGET(ASM_CLASS_EC)     /* push class extern <imm8>:<imm8>, const <imm8> */
	TARGET(ASM16_CLASS_EC) { /* push class extern <imm8>:<imm8>, const <imm8> */
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
			DO(Dee_function_generator_vpush_global(self, base_gid, true));
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
			DO(Dee_function_generator_vpush_extern(self, base_mid, base_gid, true));
		}	break;
		default: __builtin_unreachable();
		}
		DO(Dee_function_generator_vpush_cid_t(self, desc_cid, &DeeClassDescriptor_Type));
		__IF0 {
	TARGET(ASM_CLASS)
			DO(Dee_function_generator_vassert_type_exact_if_safe_c(self, &DeeClassDescriptor_Type)); /* base, desc */
		}
		DO(Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code->co_module));
		/* TODO: Optimize succeeding ASM_DEFCMEMBER instructions by forgoing locking the
		 *       class, and not needing to call `DeeClass_SetMember*'. Since at this point,
		 *       we know that the class isn't being shared with anyone, we can track which
		 *       slots have already been initialized, and do direct writes if they aren't
		 *       yet. */
		return Dee_function_generator_vcallapi(self, &DeeClass_New, VCALLOP_CC_OBJECT, 3);
	}	break;

	TARGET(ASM_DEFCMEMBER) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_DEFCMEMBER) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_imm16(self, addr)); /* type, value, addr */
		DO(Dee_function_generator_vswap(self));             /* type, addr, value */
		return (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE)
		       ? Dee_function_generator_vcallapi(self, &DeeClass_SetMemberSafe, VCALLOP_CC_INT, 3)
		       : Dee_function_generator_vcallapi(self, &DeeClass_SetMember, VCALLOP_CC_VOID, 3);
	}	break;

	TARGET(ASM16_GETCMEMBER) {
		uint16_t addr = UNALIGNED_GETLE16(instr + 2);
		unsigned int flags = DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE; /* Safe semantics are required here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc))
			flags |= DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF;
		return Dee_function_generator_vpush_cmember(self, addr, flags);
	}	break;

	TARGET(ASM_GETCMEMBER_R) {
		uint16_t type_rid;
		uint16_t addr;
		unsigned int flags;
		type_rid = instr[1];
		addr     = instr[2];
		__IF0 {
	TARGET(ASM16_GETCMEMBER_R)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			addr     = UNALIGNED_GETLE16(instr + 4);
		}
		flags = DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL; /* Non-safe semantics are allowed here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc + 1))
			flags |= DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF;
		DO(Dee_function_generator_vpush_rid(self, type_rid)); /* this, type */
		return Dee_function_generator_vpush_cmember(self, addr, flags);
	}	break;

	TARGET(ASM_CALLCMEMBER_THIS_R) {
		uint16_t type_rid;
		uint16_t addr;
		uint8_t argc;
		type_rid = instr[1];
		addr     = instr[2];
		argc     = instr[3];
		__IF0 {
	TARGET(ASM16_CALLCMEMBER_THIS_R)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			addr     = UNALIGNED_GETLE16(instr + 4);
			argc     = instr[6];
		}
		DO(Dee_function_generator_vpush_rid(self, type_rid));  /* [args...], type */
		DO(Dee_function_generator_vpush_cmember(self, addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* [args...], func */
		DO(Dee_function_generator_vrrot(self, argc + 1));      /* func, [args...] */
		DO(Dee_function_generator_vpush_this(self));           /* func, [args...], this */
		DO(Dee_function_generator_vrrot(self, argc + 1));      /* func, this, [args...] */
		return Dee_function_generator_vopthiscall(self, argc); /* result */
	}	break;

	TARGET(ASM_FUNCTION_C)
	TARGET(ASM16_FUNCTION_C)
	TARGET(ASM_FUNCTION_C_16)
	TARGET(ASM16_FUNCTION_C_16) {
		/* Implement these by creating the function raw, and then using mov-s to fill in `fo_refv'
		 * That way, we don't even have to push the reference somewhere temporarily, or have to
		 * do decref when we just want the function to inherit them. */
		uint32_t refc;
		uint16_t code_cid;
		size_t sizeof_function;
		switch (opcode) {
		case ASM_FUNCTION_C:
			code_cid = instr[1];
			refc     = instr[2] + 1;
			break;
		case ASM16_FUNCTION_C:
			code_cid = UNALIGNED_GETLE16(instr + 2) + 1;
			refc     = instr[4];
			break;
		case ASM_FUNCTION_C_16:
			code_cid = instr[1];
			refc     = UNALIGNED_GETLE16(instr + 2) + 1;
			break;
		case ASM16_FUNCTION_C_16:
			code_cid = UNALIGNED_GETLE16(instr + 2);
			refc     = UNALIGNED_GETLE16(instr + 4) + 1;
			break;
		default: __builtin_unreachable();
		}
		sizeof_function = offsetof(DeeFunctionObject, fo_refv) +
		                  ((size_t)refc * sizeof(DREF DeeObject *));
		DO(Dee_function_generator_vpush_immSIZ(self, sizeof_function));                     /* [refs...], sizeof_function */
		DO(Dee_function_generator_vcallapi(self, &DeeObject_Malloc, VCALLOP_CC_OBJECT, 1)); /* [refs...], ref:function */
		ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));            /* - */
		DO(Dee_function_generator_vpush_immSIZ(self, 1));                                   /* [refs...], ref:function, 1 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeFunctionObject, ob_refcnt)));   /* [refs...], ref:function */
		DO(Dee_function_generator_vpush_const(self, (DeeObject *)&DeeFunction_Type));       /* [refs...], ref:function, DeeFunction_Type */
		DO(Dee_function_generator_vref(self));                                              /* [refs...], ref:function, ref:DeeFunction_Type */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeFunctionObject, ob_type)));     /* [refs...], ref:function */
		DO(Dee_function_generator_vpush_cid_t(self, code_cid, &DeeCode_Type));              /* [refs...], ref:function, code */
		DO(Dee_function_generator_vref(self));                                              /* [refs...], ref:function, ref:code */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeFunctionObject, fo_code)));     /* [refs...], ref:function */
		while (refc) {
			--refc;
			DO(Dee_function_generator_vswap(self)); /* [refs...], ref:function, ref */
			DO(Dee_function_generator_vref(self));  /* [refs...], ref:function, ref:ref */
			DO(Dee_function_generator_vpopind(self, /* [refs...], ref:function */
			                                  offsetof(DeeFunctionObject, fo_refv) +
			                                  (refc * sizeof(DREF DeeObject *))));
		}
		ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF)); /* ref:function */
	}	break;

	TARGET(ASM_CAST_INT)
		return Dee_function_generator_vopint(self);
	TARGET(ASM_INV)
		return Dee_function_generator_vop(self, OPERATOR_INV, 1, VOP_F_PUSHRES);
	TARGET(ASM_POS)
		return Dee_function_generator_vop(self, OPERATOR_POS, 1, VOP_F_PUSHRES);
	TARGET(ASM_NEG)
		return Dee_function_generator_vop(self, OPERATOR_NEG, 1, VOP_F_PUSHRES);
	TARGET(ASM_ADD)
		return Dee_function_generator_vop(self, OPERATOR_ADD, 2, VOP_F_PUSHRES);
	TARGET(ASM_SUB)
		return Dee_function_generator_vop(self, OPERATOR_SUB, 2, VOP_F_PUSHRES);
	TARGET(ASM_MUL)
		return Dee_function_generator_vop(self, OPERATOR_MUL, 2, VOP_F_PUSHRES);
	TARGET(ASM_DIV)
		return Dee_function_generator_vop(self, OPERATOR_DIV, 2, VOP_F_PUSHRES);
	TARGET(ASM_MOD)
		return Dee_function_generator_vop(self, OPERATOR_MOD, 2, VOP_F_PUSHRES);
	TARGET(ASM_SHL)
		return Dee_function_generator_vop(self, OPERATOR_SHL, 2, VOP_F_PUSHRES);
	TARGET(ASM_SHR)
		return Dee_function_generator_vop(self, OPERATOR_SHR, 2, VOP_F_PUSHRES);
	TARGET(ASM_AND)
		return Dee_function_generator_vop(self, OPERATOR_AND, 2, VOP_F_PUSHRES);
	TARGET(ASM_OR)
		return Dee_function_generator_vop(self, OPERATOR_OR, 2, VOP_F_PUSHRES);
	TARGET(ASM_XOR)
		return Dee_function_generator_vop(self, OPERATOR_XOR, 2, VOP_F_PUSHRES);
	TARGET(ASM_POW)
		return Dee_function_generator_vop(self, OPERATOR_POW, 2, VOP_F_PUSHRES);

	TARGET(ASM_ADD_SIMM8)
	TARGET(ASM_SUB_SIMM8)
	TARGET(ASM_MUL_SIMM8)
	TARGET(ASM_DIV_SIMM8)
	TARGET(ASM_MOD_SIMM8) {
		int8_t Simm8 = (int8_t)instr[1];
		DO(Dee_function_generator_vpush_Simm8(self, Simm8));
		/* TODO: Integration with `Dee_function_generator_vop()' */
		switch (opcode) {
		case ASM_ADD_SIMM8:
			return Dee_function_generator_vcallapi(self, &DeeObject_AddInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_SUB_SIMM8:
			return Dee_function_generator_vcallapi(self, &DeeObject_SubInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_MUL_SIMM8:
			return Dee_function_generator_vcallapi(self, &DeeObject_MulInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_DIV_SIMM8:
			return Dee_function_generator_vcallapi(self, &DeeObject_DivInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_MOD_SIMM8:
			return Dee_function_generator_vcallapi(self, &DeeObject_ModInt8, VCALLOP_CC_OBJECT, 2);
		default: __builtin_unreachable();
		}
	}	break;

	TARGET(ASM_SHL_IMM8)
	TARGET(ASM_SHR_IMM8) {
		uint8_t imm8 = (uint8_t)instr[1];
		DO(Dee_function_generator_vpush_imm8(self, imm8));
		/* TODO: Integration with `Dee_function_generator_vop()' */
		switch (opcode) {
		case ASM_SHL_IMM8:
			return Dee_function_generator_vcallapi(self, &DeeObject_ShlUInt8, VCALLOP_CC_OBJECT, 2);
		case ASM_SHR_IMM8:
			return Dee_function_generator_vcallapi(self, &DeeObject_ShrUInt8, VCALLOP_CC_OBJECT, 2);
		default: __builtin_unreachable();
		}
	}	break;

	TARGET(ASM_ADD_IMM32)
	TARGET(ASM_SUB_IMM32)
	TARGET(ASM_AND_IMM32)
	TARGET(ASM_OR_IMM32)
	TARGET(ASM_XOR_IMM32) {
		uint32_t imm32 = (uint32_t)UNALIGNED_GETLE32(instr + 1);
		DO(Dee_function_generator_vpush_imm32(self, imm32));
		/* TODO: Integration with `Dee_function_generator_vop()' */
		switch (opcode) {
		case ASM_ADD_IMM32:
			return Dee_function_generator_vcallapi(self, &DeeObject_AddUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_SUB_IMM32:
			return Dee_function_generator_vcallapi(self, &DeeObject_SubUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_AND_IMM32:
			return Dee_function_generator_vcallapi(self, &DeeObject_AndUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_OR_IMM32:
			return Dee_function_generator_vcallapi(self, &DeeObject_OrUInt32, VCALLOP_CC_OBJECT, 2);
		case ASM_XOR_IMM32:
			return Dee_function_generator_vcallapi(self, &DeeObject_XorUInt32, VCALLOP_CC_OBJECT, 2);
		default: __builtin_unreachable();
		}
	}	break;

	TARGET(ASM_ISNONE)
		return Dee_function_generator_veqconstaddr(self, Dee_None);

	CASE_ASM_PRINT:
		return Dee_function_generator_gen_stdout_print(self, instr, p_next_instr, false);

	TARGET(ASM_FPRINT)         /* print top, pop */
	TARGET(ASM_FPRINT_SP)      /* print top, pop, sp */
	TARGET(ASM_FPRINT_NL)      /* print top, pop, nl */
	TARGET(ASM_FPRINTALL)      /* print top, pop... */
	TARGET(ASM_FPRINTALL_SP)   /* print top, pop..., sp */
	TARGET(ASM_FPRINTALL_NL) { /* print top, pop..., nl */
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
		DO(Dee_function_generator_vswap(self));    /* value, File */
		DO(Dee_function_generator_vdup(self));     /* value, File, File */
		DO(Dee_function_generator_vlrot(self, 3)); /* File, File, value */
		return gen_print_to_file(self, api_function);
	}	break;

	TARGET(ASM_FPRINTNL) /* print top, nl */
		DO(Dee_function_generator_vdup(self)); /* File, File */
		return Dee_function_generator_vcallapi(self, &DeeFile_PrintNl, VCALLOP_CC_OBJECT, 1);

	TARGET(ASM_FPRINT_C)        /* print top, const <imm8> */
	TARGET(ASM16_FPRINT_C)      /* print top, const <imm16> */
	TARGET(ASM_FPRINT_C_SP)     /* print top, const <imm8>, sp */
	TARGET(ASM16_FPRINT_C_SP)   /* print top, const <imm16>, sp */
	TARGET(ASM_FPRINT_C_NL)     /* print top, const <imm8>, nl */
	TARGET(ASM16_FPRINT_C_NL) { /* print top, const <imm16>, nl */
		void const *api_function;
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
		DO(Dee_function_generator_vdup(self));           /* File, File */
		DO(Dee_function_generator_vpush_cid(self, cid)); /* File, File, value */
		return gen_print_to_file(self, api_function);
	}	break;

	TARGET(ASM_ENTER)
		DO(Dee_function_generator_vdup(self));
		return Dee_function_generator_vop(self, OPERATOR_ENTER, 1, VOP_F_NORMAL);
	TARGET(ASM_LEAVE)
		return Dee_function_generator_vop(self, OPERATOR_LEAVE, 1, VOP_F_NORMAL);

	TARGET(ASM_RANGE)                                           /* start, end */
		DO(Dee_function_generator_vswap(self));                 /* end, start */
		DO(Dee_function_generator_vpush_const(self, Dee_None)); /* end, start, Dee_None */
		DO(Dee_function_generator_vpush_addr(self, NULL));      /* end, start, Dee_None, NULL */
		DO(Dee_function_generator_vcoalesce(self));             /* end, used_start */
		DO(Dee_function_generator_vswap(self));                 /* used_start, end */
		DO(Dee_function_generator_vpush_addr(self, NULL));      /* used_start, end, NULL */
		return Dee_function_generator_vcallapi(self, &DeeRange_New, VCALLOP_CC_OBJECT, 3); /* result */

	TARGET(ASM_RANGE_DEF)                                                                       /* end */
		DO(Dee_function_generator_vdup(self));                                                  /* end, end */
		DO(Dee_function_generator_voptypeof(self, false));                                      /* end, type(end) */
		DO(Dee_function_generator_vcallapi(self, &DeeObject_NewDefault, VCALLOP_CC_OBJECT, 1)); /* end, type(end)() */
		DO(Dee_function_generator_vswap(self));                                                 /* type(end)(), end */
		DO(Dee_function_generator_vpush_addr(self, NULL));                                      /* type(end)(), end, NULL */
		return Dee_function_generator_vcallapi(self, &DeeRange_New, VCALLOP_CC_OBJECT, 3);      /* result */

	TARGET(ASM_RANGE_STEP)                                      /* start, end, step */
		DO(Dee_function_generator_vpush_const(self, Dee_None)); /* start, end, step, Dee_None */
		DO(Dee_function_generator_vpush_addr(self, NULL));      /* start, end, step, Dee_None, NULL */
		DO(Dee_function_generator_vcoalesce(self));             /* start, end, used_step */
		DO(Dee_function_generator_vlrot(self, 3));              /* end, used_step, start */
		DO(Dee_function_generator_vpush_const(self, Dee_None)); /* end, used_step, start, Dee_None */
		DO(Dee_function_generator_vpush_addr(self, NULL));      /* end, used_step, start, Dee_None, NULL */
		DO(Dee_function_generator_vcoalesce(self));             /* end, used_step, used_start */
		DO(Dee_function_generator_vrrot(self, 3));              /* used_start, end, used_step */
		return Dee_function_generator_vcallapi(self, &DeeRange_New, VCALLOP_CC_OBJECT, 3); /* result */

	TARGET(ASM_RANGE_STEP_DEF)                                                                  /* end, step */
		DO(Dee_function_generator_vpush_const(self, Dee_None));                                 /* end, step, Dee_None */
		DO(Dee_function_generator_vpush_addr(self, NULL));                                      /* end, step, Dee_None, NULL */
		DO(Dee_function_generator_vcoalesce(self));                                             /* end, used_step */
		DO(Dee_function_generator_vswap(self));                                                 /* used_step, end */
		DO(Dee_function_generator_vdup(self));                                                  /* used_step, end, end */
		DO(Dee_function_generator_voptypeof(self, false));                                      /* used_step, end, type(end) */
		DO(Dee_function_generator_vcallapi(self, &DeeObject_NewDefault, VCALLOP_CC_OBJECT, 1)); /* used_step, end, type(end)() */
		DO(Dee_function_generator_vswap(self));                                                 /* used_step, type(end)(), end */
		DO(Dee_function_generator_vlrot(self, 3));                                              /* type(end)(), end, used_step */
		return Dee_function_generator_vcallapi(self, &DeeRange_New, VCALLOP_CC_OBJECT, 3);      /* result */

	TARGET(ASM_RANGE_0_I16) {
		uint32_t imm32;
		imm32 = UNALIGNED_GETLE16(instr + 1);
		__IF0 { TARGET(ASM_RANGE_0_I32) imm32 = UNALIGNED_GETLE32(instr + 2); }
		DO(Dee_function_generator_vpush_imm32(self, 0));
		DO(Dee_function_generator_vpush_imm32(self, imm32));
		DO(Dee_function_generator_vpush_imm32(self, 1));
		return Dee_function_generator_vcallapi(self, &DeeRange_NewInt, VCALLOP_CC_OBJECT, 3);
	}	break;

	TARGET(ASM_CONTAINS)
		return Dee_function_generator_vop(self, OPERATOR_CONTAINS, 2, VOP_F_PUSHRES);

	TARGET(ASM_CONTAINS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_CONTAINS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));
		DO(Dee_function_generator_vswap(self));
		return Dee_function_generator_vop(self, OPERATOR_CONTAINS, 2, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_GETITEM)
		return Dee_function_generator_vop(self, OPERATOR_GETITEM, 2, VOP_F_PUSHRES);

	TARGET(ASM_GETITEM_I) {
		uint16_t imm16 = UNALIGNED_GETLE16(instr + 1);
		DO(Dee_function_generator_vpush_imm16(self, imm16));
		return Dee_function_generator_vcallapi(self, &DeeObject_GetItemIndex, VCALLOP_CC_OBJECT, 2);
	}	break;

	TARGET(ASM_GETITEM_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_GETITEM_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));
		return Dee_function_generator_vop(self, OPERATOR_GETITEM, 2, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_GETSIZE)
		return Dee_function_generator_vopsize(self);

	TARGET(ASM_SETITEM_I) {
		uint16_t imm16 = UNALIGNED_GETLE16(instr + 1);
		DO(Dee_function_generator_vpush_imm16(self, imm16));
		DO(Dee_function_generator_vswap(self));
		return Dee_function_generator_vcallapi(self, &DeeObject_SetItemIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	TARGET(ASM_SETITEM_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_SETITEM_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));
		DO(Dee_function_generator_vswap(self));
		DO(Dee_function_generator_vop(self, OPERATOR_SETITEM, 3, VOP_F_PUSHRES));
		return Dee_function_generator_vpop(self);
	}	break;

	TARGET(ASM_ITERSELF)
		return Dee_function_generator_vop(self, OPERATOR_ITERSELF, 1, VOP_F_PUSHRES);
	TARGET(ASM_DELITEM)
		return Dee_function_generator_vop(self, OPERATOR_DELITEM, 2, VOP_F_NORMAL);
	TARGET(ASM_SETITEM)
		return Dee_function_generator_vop(self, OPERATOR_SETITEM, 3, VOP_F_NORMAL);
	TARGET(ASM_GETRANGE)
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES);
	TARGET(ASM_GETRANGE_PN)
		DO(Dee_function_generator_vpush_const(self, Dee_None));
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES);
	TARGET(ASM_GETRANGE_NP)
		DO(Dee_function_generator_vpush_const(self, Dee_None));
		DO(Dee_function_generator_vswap(self));
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES);

	TARGET(ASM_GETRANGE_PI)
	TARGET(ASM_GETRANGE_NI) {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		if (opcode == ASM_GETRANGE_NI) {
			DO(Dee_function_generator_vpush_const(self, Dee_None));
		}
		DO(Dee_function_generator_vpush_Simm16(self, Simm16));
		return Dee_function_generator_vcallapi(self, &DeeObject_GetRangeEndIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	TARGET(ASM_GETRANGE_IP)
	TARGET(ASM_GETRANGE_IN) {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		DO(Dee_function_generator_vpush_Simm16(self, Simm16));
		if (opcode == ASM_GETRANGE_IN) {
			DO(Dee_function_generator_vpush_const(self, Dee_None));
		} else {
			DO(Dee_function_generator_vswap(self));
		}
		return Dee_function_generator_vcallapi(self, &DeeObject_GetRangeBeginIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	TARGET(ASM_GETRANGE_II) {
		int16_t begin_Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		int16_t end_Simm16   = (int16_t)UNALIGNED_GETLE16(instr + 3);
		DO(Dee_function_generator_vpush_Simm16(self, begin_Simm16));
		DO(Dee_function_generator_vpush_Simm16(self, end_Simm16));
		return Dee_function_generator_vcallapi(self, &DeeObject_GetRangeIndex, VCALLOP_CC_OBJECT, 3);
	}	break;

	TARGET(ASM_DELRANGE)
		return Dee_function_generator_vop(self, OPERATOR_DELRANGE, 3, VOP_F_NORMAL);
	TARGET(ASM_SETRANGE)
		return Dee_function_generator_vop(self, OPERATOR_SETRANGE, 4, VOP_F_NORMAL);
	TARGET(ASM_SETRANGE_PN)
		DO(Dee_function_generator_vpush_const(self, Dee_None));
		DO(Dee_function_generator_vop(self, OPERATOR_SETRANGE, 3, VOP_F_PUSHRES));
		return Dee_function_generator_vpop(self);
	TARGET(ASM_SETRANGE_NP)
		DO(Dee_function_generator_vpush_const(self, Dee_None));
		DO(Dee_function_generator_vswap(self));
		DO(Dee_function_generator_vop(self, OPERATOR_SETRANGE, 3, VOP_F_PUSHRES));
		return Dee_function_generator_vpop(self);

	TARGET(ASM_SETRANGE_PI)
	TARGET(ASM_SETRANGE_NI) {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		if (opcode == ASM_SETRANGE_NI) {
			DO(Dee_function_generator_vpush_const(self, Dee_None));
		}
		DO(Dee_function_generator_vpush_Simm16(self, Simm16));
		return Dee_function_generator_vcallapi(self, &DeeObject_SetRangeEndIndex, VCALLOP_CC_INT, 4);
	}	break;

	TARGET(ASM_SETRANGE_IP)
	TARGET(ASM_SETRANGE_IN) {
		int16_t Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		DO(Dee_function_generator_vpush_Simm16(self, Simm16));
		if (opcode == ASM_SETRANGE_IN) {
			DO(Dee_function_generator_vpush_const(self, Dee_None));
		} else {
			DO(Dee_function_generator_vswap(self));
		}
		return Dee_function_generator_vcallapi(self, &DeeObject_SetRangeBeginIndex, VCALLOP_CC_INT, 4);
	}	break;

	TARGET(ASM_SETRANGE_II) {
		int16_t begin_Simm16 = (int16_t)UNALIGNED_GETLE16(instr + 1);
		int16_t end_Simm16   = (int16_t)UNALIGNED_GETLE16(instr + 3);
		DO(Dee_function_generator_vpush_Simm16(self, begin_Simm16));
		DO(Dee_function_generator_vpush_Simm16(self, end_Simm16));
		return Dee_function_generator_vcallapi(self, &DeeObject_SetRangeIndex, VCALLOP_CC_INT, 4);
	}	break;

	//TODO: TARGET(ASM_BREAKPOINT)

	TARGET(ASM_UD) {
		DO(Dee_function_generator_vpush_addr(self, instr));
		DO(Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code));
		return Dee_function_generator_vcallapi(self, &libhostasm_rt_err_illegal_instruction, VCALLOP_CC_EXCEPT, 2);
	}	break;

	TARGET(ASM_CALLATTR_C_KW) {
		uint16_t attr_cid;
		uint8_t argc;
		uint16_t kwds_cid;
		attr_cid = instr[1];
		argc     = instr[2];
		kwds_cid = instr[3];
		__IF0 {
	TARGET(ASM16_CALLATTR_C_KW)
			attr_cid = UNALIGNED_GETLE16(instr + 2);
			argc     = instr[4];
			kwds_cid = UNALIGNED_GETLE16(instr + 5);
		}
		DO(Dee_function_generator_vpush_cid(self, attr_cid)); /* this, [args...], attr */
		DO(Dee_function_generator_vrrot(self, argc + 1));     /* this, attr, [args...] */
		DO(Dee_function_generator_vpush_cid(self, kwds_cid)); /* this, attr, [args...], kw */
		return Dee_function_generator_vopcallattrkw(self, argc);
	}	break;

	TARGET(ASM_CALLATTR_C_TUPLE_KW) {
		uint16_t attr_cid;
		uint16_t kwds_cid;
		attr_cid = instr[1];
		kwds_cid = instr[2];
		__IF0 {
	TARGET(ASM16_CALLATTR_C_TUPLE_KW)
			attr_cid = UNALIGNED_GETLE16(instr + 2);
			kwds_cid = UNALIGNED_GETLE16(instr + 4);
		}
		DO(Dee_function_generator_vpush_cid(self, attr_cid));
		DO(Dee_function_generator_vswap(self));
		DO(Dee_function_generator_vpush_cid(self, kwds_cid));
		ATTR_FALLTHROUGH
	TARGET(ASM_CALLATTR_TUPLE_KWDS)
		return Dee_function_generator_vopcallattrtuplekw(self);
	}	break;

	TARGET(ASM_CALLATTR)
	TARGET(ASM_CALLATTR_C) {
		uint8_t argc;
		argc = instr[1];
		if (opcode == ASM_CALLATTR_C) {
			uint16_t attr_cid;
			attr_cid = instr[1];
			argc     = instr[2];
			__IF0 {
	TARGET(ASM16_CALLATTR_C)
				attr_cid = UNALIGNED_GETLE16(instr + 2);
				argc     = instr[4];
			}
			DO(Dee_function_generator_vpush_cid(self, attr_cid)); /* this, [args...], attr */
			DO(Dee_function_generator_vrrot(self, argc + 1));     /* this, attr, [args...] */
		}
		return Dee_function_generator_vopcallattr(self, argc);
	}	break;

	TARGET(ASM_CALLATTR_C_TUPLE) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_CALLATTR_C_TUPLE) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_cid(self, cid));
		DO(Dee_function_generator_vswap(self));
		ATTR_FALLTHROUGH
	TARGET(ASM_CALLATTR_TUPLE)
		return Dee_function_generator_vopcallattrtuple(self);
	}	break;

	TARGET(ASM_CALLATTR_THIS_C) {
		uint16_t attr_cid;
		uint8_t argc;
		attr_cid = instr[1];
		argc     = instr[2];
		__IF0 {
	TARGET(ASM16_CALLATTR_THIS_C)
			attr_cid = UNALIGNED_GETLE16(instr + 2);
			argc     = instr[4];
		}
		DO(Dee_function_generator_vpush_this(self));           /* [args...], this */
		DO(Dee_function_generator_vrrot(self, argc + 1));      /* this, [args...] */
		DO(Dee_function_generator_vpush_cid(self, attr_cid));  /* this, [args...], attr */
		DO(Dee_function_generator_vrrot(self, argc + 1));      /* this, attr, [args...] */
		return Dee_function_generator_vopcallattr(self, argc); /* result */
	}	break;

	TARGET(ASM_CALLATTR_THIS_C_TUPLE) {
		uint16_t attr_cid;
		attr_cid = instr[1];
		__IF0 { TARGET(ASM16_CALLATTR_THIS_C_TUPLE) attr_cid = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self));          /* args, this */
		DO(Dee_function_generator_vpush_cid(self, attr_cid)); /* args, this, attr */
		DO(Dee_function_generator_vlrot(self, 3));            /* this, attr, args */
		return Dee_function_generator_vopcallattrtuple(self); /* result */
	}	break;

	TARGET(ASM_CALLATTR_C_SEQ)
	TARGET(ASM_CALLATTR_C_MAP) {
		uint16_t attr_cid;
		uint16_t argc;
		void const *api_create;
		void const *api_decref;
		attr_cid = instr[1];
		argc     = instr[2];
		__IF0 {
	TARGET(ASM16_CALLATTR_C_SEQ)
	TARGET(ASM16_CALLATTR_C_MAP)
			attr_cid = UNALIGNED_GETLE16(instr + 2);
			argc     = instr[4];
		}
		if ((opcode & 0xff) == ASM_CALLATTR_C_MAP) {
			argc *= 2;
			api_create = (void const *)&DeeSharedMap_NewShared;
			api_decref = (void const *)&DeeSharedMap_Decref;
		} else {
			api_create = (void const *)&DeeSharedVector_NewShared;
			api_decref = (void const *)&DeeSharedVector_Decref;
		}
		DO(Dee_function_generator_vlinear(self, argc, true));                        /* this, [args...], argv */
		DO(Dee_function_generator_vpush_immSIZ(self, argc));                         /* this, [args...], argv, argc */
		DO(Dee_function_generator_vswap(self));                                      /* this, [args...], argc, argv */
		DO(Dee_function_generator_vcallapi(self, api_create, VCALLOP_CC_OBJECT, 2)); /* this, [args...], shared_args */
		DO(Dee_function_generator_vlrot(self, argc + 1));                            /* [args...], shared_args, this */
		DO(Dee_function_generator_vpush_cid(self, attr_cid));                        /* [args...], shared_args, this, attr */
		DO(Dee_function_generator_vlrot(self, 3));                                   /* [args...], this, attr, shared_args */
		DO(Dee_function_generator_vopcallattr_unchecked(self, 1));                   /* [args...], shared_args, UNCHECKED(result) */
		DO(Dee_function_generator_vrrot(self, argc + 2));                            /* UNCHECKED(result), [args...], shared_args */
		DO(Dee_function_generator_vcallapi(self, api_decref, VCALLOP_CC_VOID, 1));   /* UNCHECKED(result), [args...] */
		DO(Dee_function_generator_vpopmany(self, argc));                             /* UNCHECKED(result) */
		return Dee_function_generator_vcheckobj(self);                               /* result */
	}	break;

	TARGET(ASM_CALLATTR_KWDS) {
		uint8_t argc = instr[1]; /* this, attr, [args...], kwds */
		return Dee_function_generator_vopcallattrkw(self, argc);
	}	break;

	TARGET(ASM_CALL_EXTERN)
	TARGET(ASM16_CALL_EXTERN)
	TARGET(ASM_CALL_GLOBAL)
	TARGET(ASM16_CALL_GLOBAL)
	TARGET(ASM_CALL_LOCAL)
	TARGET(ASM16_CALL_LOCAL) { /* [args...] */
		uint8_t argc;
		switch (opcode) {
		case ASM_CALL_EXTERN: {
			uint16_t mid, gid;
			mid  = instr[1];
			gid  = instr[2];
			argc = instr[3];
			__IF0 {
		case ASM16_CALL_EXTERN:
				mid  = UNALIGNED_GETLE16(instr + 2);
				gid  = UNALIGNED_GETLE16(instr + 4);
				argc = instr[6];
			}
			DO(Dee_function_generator_vpush_extern(self, mid, gid, true)); /* [args...], func */
		}	break;
		case ASM_CALL_GLOBAL: {
			uint16_t gid;
			gid  = instr[1];
			argc = instr[2];
			__IF0 {
		case ASM16_CALL_GLOBAL:
				gid  = UNALIGNED_GETLE16(instr + 2);
				argc = instr[4];
			}
			DO(Dee_function_generator_vpush_global(self, gid, true)); /* [args...], func */
		}	break;
		case ASM_CALL_LOCAL: {
			uint16_t lid;
			lid  = instr[1];
			argc = instr[2];
			__IF0 {
		case ASM16_CALL_LOCAL:
				lid  = UNALIGNED_GETLE16(instr + 2);
				argc = instr[4];
			}
			DO(Dee_function_generator_vpush_ulocal(self, instr, lid)); /* [args...], func */
		}	break;
		default: __builtin_unreachable();
		}                                                  /* [args...], func */
		DO(Dee_function_generator_vrrot(self, argc + 1));  /* func, [args...] */
		return Dee_function_generator_vopcall(self, argc); /* result */
	}	break;

	TARGET(ASM_CALL_SEQ) {
		uint16_t argc;
		void const *api_create;
		void const *api_decref;
		argc = instr[1];
		api_create = (void const *)&DeeSharedVector_NewShared;
		api_decref = (void const *)&DeeSharedVector_Decref;
		__IF0 {
	TARGET(ASM_CALL_MAP)
			argc = instr[1] * 2;
			api_create = (void const *)&DeeSharedMap_NewShared;
			api_decref = (void const *)&DeeSharedMap_Decref;
		}                                                                            /* func, [args...] */
		DO(Dee_function_generator_vlinear(self, argc, true));                        /* func, [args...], argv */
		DO(Dee_function_generator_vpush_immSIZ(self, argc));                         /* func, [args...], argv, argc */
		DO(Dee_function_generator_vswap(self));                                      /* func, [args...], argc, argv */
		DO(Dee_function_generator_vcallapi(self, api_create, VCALLOP_CC_OBJECT, 2)); /* func, [args...], shared_args */
		DO(Dee_function_generator_vlrot(self, argc + 2));                            /* [args...], shared_args, func */
		DO(Dee_function_generator_vswap(self));                                      /* [args...], func, shared_args */
		DO(Dee_function_generator_vopcall_unchecked(self, 1));                       /* [args...], shared_args, UNCHECKED(result) */
		DO(Dee_function_generator_vrrot(self, argc + 2));                            /* UNCHECKED(result), [args...], shared_args */
		DO(Dee_function_generator_vcallapi(self, api_decref, VCALLOP_CC_VOID, 1));   /* UNCHECKED(result), [args...] */
		DO(Dee_function_generator_vpopmany(self, argc));                             /* UNCHECKED(result) */
		return Dee_function_generator_vcheckobj(self);                               /* result */
	}	break;

	TARGET(ASM_THISCALL_TUPLE)
		return Dee_function_generator_vopthiscalltuple(self);
	TARGET(ASM_CALL_TUPLE_KWDS)
		return Dee_function_generator_vopcalltuplekw(self);

	TARGET(ASM_PUSH_EXCEPT)
		return Dee_function_generator_vpush_except(self);

	TARGET(ASM_PUSH_THIS)
		return Dee_function_generator_vpush_this(self);
	TARGET(ASM_CAST_HASHSET)
		return Dee_function_generator_vcallapi(self, &DeeHashSet_FromSequence, VCALLOP_CC_OBJECT, 1);
	TARGET(ASM_CAST_DICT)
		return Dee_function_generator_vcallapi(self, &DeeDict_FromSequence, VCALLOP_CC_OBJECT, 1);

	TARGET(ASM_PUSH_TRUE)
		return Dee_function_generator_vpush_const(self, Dee_True);
	TARGET(ASM_PUSH_FALSE)
		return Dee_function_generator_vpush_const(self, Dee_False);

	//TODO: TARGET(ASM_PACK_HASHSET)
	//TODO: TARGET(ASM16_PACK_HASHSET)
	//TODO: TARGET(ASM_PACK_DICT)
	//TODO: TARGET(ASM16_PACK_DICT)

	TARGET(ASM_BOUNDITEM) {                             /* seq, index */
		DO(Dee_function_generator_vpush_imm8(self, 1)); /* seq, index, true */
		DO(Dee_function_generator_vcallapi(self, &DeeObject_BoundItem, VCALLOP_CC_M1INT, 3)); /* seq, index, true */
		DO(Dee_function_generator_vdirect(self, 1));
		ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
		Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
	}	break;

	TARGET(ASM_CMP_SO)
	TARGET(ASM_CMP_DO)
		/* TODO: Special handling for when the instruction that eventually pops the
		 *       value pushed here is ASM_JT/ASM_JF, in which case that jump needs
		 *       to be implemented using `_Dee_function_generator_gjcmp()' */
		DO(Dee_function_generator_veqaddr(self));
		if (opcode == ASM_CMP_DO)
			return Dee_function_generator_vopnot(self);
		break;


	//TODO: TARGET(ASM_SUPERGETATTR_THIS_RC)
	//TODO: TARGET(ASM16_SUPERGETATTR_THIS_RC)
	//TODO: TARGET(ASM_SUPERCALLATTR_THIS_RC)
	//TODO: TARGET(ASM16_SUPERCALLATTR_THIS_RC)

	TARGET(ASM_REDUCE_MIN)
		DO(Dee_function_generator_vpush_addr(self, NULL));
		return Dee_function_generator_vcallapi(self, &DeeSeq_Min, VCALLOP_CC_OBJECT, 2);
	TARGET(ASM_REDUCE_MAX)
		DO(Dee_function_generator_vpush_addr(self, NULL));
		return Dee_function_generator_vcallapi(self, &DeeSeq_Max, VCALLOP_CC_OBJECT, 2);
	TARGET(ASM_REDUCE_SUM)
		return Dee_function_generator_vcallapi(self, &DeeSeq_Sum, VCALLOP_CC_OBJECT, 1);

	TARGET(ASM_REDUCE_ANY)
	TARGET(ASM_REDUCE_ALL)
		DO(Dee_function_generator_vcallapi(self,
		                                   opcode == ASM_REDUCE_ANY ? (void const *)&DeeSeq_Any
		                                                            : (void const *)&DeeSeq_All,
		                                   VCALLOP_CC_NEGINT, 1));
		DO(Dee_function_generator_vdirect(self, 1));
		ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
		Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
		break;

	//TODO: TARGET(ASM_VARARGS_UNPACK)
	//TODO: TARGET(ASM_PUSH_VARKWDS_NE)
	//TODO: TARGET(ASM_VARARGS_CMP_EQ_SZ)
	//TODO: TARGET(ASM_VARARGS_CMP_GR_SZ)
	//TODO: TARGET(ASM_VARARGS_GETITEM)
	//TODO: TARGET(ASM_VARARGS_GETITEM_I)
	//TODO: TARGET(ASM_VARARGS_GETSIZE)

	TARGET(ASM_ITERNEXT)
		return Dee_function_generator_vop(self, OPERATOR_ITERNEXT, 1, VOP_F_PUSHRES);

	TARGET(ASM_GETMEMBER) {
		uint16_t addr;
		unsigned int flags;
		addr = instr[1];
		__IF0 { TARGET(ASM16_GETMEMBER) addr = UNALIGNED_GETLE16(instr + 2); }
		flags = DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE; /* Safe semantics are required here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc - 1))
			flags |= DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF;
		return Dee_function_generator_vpush_imember(self, addr, flags);
	}	break;

	TARGET(ASM_DELMEMBER)
		return Dee_function_generator_vdel_imember(self, instr[1], DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	TARGET(ASM16_DELMEMBER)
		return Dee_function_generator_vdel_imember(self, UNALIGNED_GETLE16(instr + 2), DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	TARGET(ASM_SETMEMBER)
		return Dee_function_generator_vpop_imember(self, instr[1], DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	TARGET(ASM16_SETMEMBER)
		return Dee_function_generator_vpop_imember(self, UNALIGNED_GETLE16(instr + 2), DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	TARGET(ASM_BOUNDMEMBER)
		return Dee_function_generator_vbound_imember(self, instr[1], DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	TARGET(ASM16_BOUNDMEMBER)
		return Dee_function_generator_vbound_imember(self, UNALIGNED_GETLE16(instr + 2), DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);

	TARGET(ASM_GETMEMBER_THIS) {
		uint16_t addr;
		unsigned int flags;
		addr = instr[1];
		__IF0 { TARGET(ASM16_GETMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		flags = DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE; /* Safe semantics are required here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc))
			flags |= DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF;
		DO(Dee_function_generator_vpush_this(self)); /* type, this */
		DO(Dee_function_generator_vswap(self));      /* this, type */
		return Dee_function_generator_vpush_imember(self, addr, flags);
	}	break;

	TARGET(ASM_DELMEMBER_THIS) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_DELMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self)); /* type, this */
		DO(Dee_function_generator_vswap(self));      /* this, type */
		return Dee_function_generator_vdel_imember(self, addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	}	break;

	TARGET(ASM_SETMEMBER_THIS) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_SETMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self)); /* type, value, this */
		DO(Dee_function_generator_vrrot(self, 3));   /* this, type, value */
		return Dee_function_generator_vpop_imember(self, addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	}	break;

	TARGET(ASM_BOUNDMEMBER_THIS) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_BOUNDMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(Dee_function_generator_vpush_this(self)); /* type, this */
		DO(Dee_function_generator_vswap(self));      /* this, type */
		return Dee_function_generator_vbound_imember(self, addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE);
	}	break;

	TARGET(ASM_GETMEMBER_THIS_R) {
		uint16_t type_rid, addr;
		bool ref;
		type_rid = instr[1];
		addr     = instr[2];
		__IF0 {
	TARGET(ASM16_GETMEMBER_THIS_R)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			addr     = UNALIGNED_GETLE16(instr + 4);
		}
		ref = matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                      (uint16_t)self->fg_state->ms_stackc + 1);
		DO(Dee_function_generator_vpush_this(self));                  /* this */
		DO(Dee_function_generator_vpush_rid(self, type_rid));         /* this, type */
		return Dee_function_generator_vpush_imember(self, addr, ref); /* value */
	}	break;

	TARGET(ASM_DELMEMBER_THIS_R) {
		uint16_t type_rid, addr;
		type_rid = instr[1];
		addr     = instr[2];
		__IF0 {
	TARGET(ASM16_DELMEMBER_THIS_R)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			addr     = UNALIGNED_GETLE16(instr + 4);
		}
		DO(Dee_function_generator_vpush_this(self));          /* this */
		DO(Dee_function_generator_vpush_rid(self, type_rid)); /* this, type */
		return Dee_function_generator_vdel_imember(self, addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL); 
	}	break;

	TARGET(ASM_SETMEMBER_THIS_R) {
		uint16_t type_rid, addr;
		type_rid = instr[1];
		addr     = instr[2];
		__IF0 {
	TARGET(ASM16_SETMEMBER_THIS_R)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			addr     = UNALIGNED_GETLE16(instr + 4);
		}
		DO(Dee_function_generator_vpush_this(self));          /* value, this */
		DO(Dee_function_generator_vpush_rid(self, type_rid)); /* value, this, type */
		DO(Dee_function_generator_vlrot(self, 3));            /* this, type, value */
		return Dee_function_generator_vpop_imember(self, addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL);
	}	break;

	TARGET(ASM_BOUNDMEMBER_THIS_R) {
		uint16_t type_rid, addr;
		type_rid = instr[1];
		addr     = instr[2];
		__IF0 {
	TARGET(ASM16_BOUNDMEMBER_THIS_R)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			addr     = UNALIGNED_GETLE16(instr + 4);
		}
		DO(Dee_function_generator_vpush_this(self));          /* this */
		DO(Dee_function_generator_vpush_rid(self, type_rid)); /* this, type */
		return Dee_function_generator_vbound_imember(self, addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL); 
	}	break;






		/* Instruction prefixes. */
	TARGET(ASM_STACK)
	TARGET(ASM16_STACK)
	TARGET(ASM_STATIC)
	TARGET(ASM16_STATIC)
	TARGET(ASM_EXTERN)
	TARGET(ASM16_EXTERN)
	TARGET(ASM_GLOBAL)
	TARGET(ASM16_GLOBAL)
	TARGET(ASM_LOCAL)
	TARGET(ASM16_LOCAL) {
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
			if unlikely(id1 >= self->fg_assembler->fa_localc)
				return err_illegal_ulid(id1);
			break;
		default: __builtin_unreachable();
		}
		prefix_instr  = DeeAsm_SkipPrefix(instr);
		prefix_opcode = prefix_instr[0];
		if (ASM_ISEXTENDED(prefix_opcode))
			prefix_opcode = (prefix_opcode << 8) | prefix_instr[1];
		switch (prefix_opcode) {

		TARGET(ASM_JF)   /* jf PREFIX, <Sdisp8> */
		TARGET(ASM_JF16) /* jf PREFIX, <Sdisp16> */
		TARGET(ASM_JT)   /* jt PREFIX, <Sdisp8> */
		TARGET(ASM_JT16) /* jt PREFIX, <Sdisp16> */
			DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2));
			opcode = prefix_opcode;
			/* Need to do this in a special way because `instr' must not become `prefix_instr' here. */
			goto do_jcc;

		TARGET(ASM_FOREACH)     /* foreach PREFIX, <Sdisp8> */
		TARGET(ASM_FOREACH16) { /* foreach PREFIX, <Sdisp16> */
			struct Dee_jump_descriptor *desc;
			desc = Dee_jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
			ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
			        Dee_function_assembler_addrof(self->fg_assembler, instr));
			DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2));
			return Dee_function_generator_vforeach(self, desc, true);
		}	break;

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
			DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2));
			return Dee_function_generator_geninstr(self, prefix_instr, p_next_instr);

		case ASM_DUP:                /* mov  PREFIX, top', `mov PREFIX, #SP - 1 */
		case ASM_DUP_N:              /* mov  PREFIX, #SP - <imm8> - 2 */
		case ASM16_DUP_N:            /* mov  PREFIX, #SP - <imm16> - 2 */
		case ASM_POP:                /* mov  top, PREFIX */
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
			DO(Dee_function_generator_geninstr(self, prefix_instr, p_next_instr));
			if unlikely(self->fg_block->bb_mem_end == (DREF struct Dee_memstate *)-1)
				return 0;
			return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);

		TARGET(ASM_POP_N) { /* mov #SP - <imm8> - 2, PREFIX */
			Dee_vstackaddr_t n;
			n = prefix_instr[1] + 2; /* mov #SP - <imm16> - 2, PREFIX */
			__IF0 { TARGET(ASM16_POP_N) n = UNALIGNED_GETLE16(prefix_instr + 2) + 2; }
			DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2));
			DO(Dee_function_generator_vlrot(self, n + 1));
			DO(Dee_function_generator_vpop(self));
			return Dee_function_generator_vrrot(self, n);
		}	break;

		TARGET(ASM_SWAP)   /* swap top, PREFIX */
		TARGET(ASM_LROT)   /* lrot #<imm8>+2, PREFIX */
		TARGET(ASM_RROT)   /* rrot #<imm8>+2, PREFIX */
		TARGET(ASM16_LROT) /* lrot #<imm16>+2, PREFIX */
		TARGET(ASM16_RROT) /* rrot #<imm16>+2, PREFIX */
			/* These instructions are special because they operate atomically for GLOBAL/EXTERN prefixes! */
			switch (prefix_type) {

			case ASM_GLOBAL:
			case ASM_EXTERN: {
				uint16_t gid;
				DeeModuleObject *mod;
				mod = self->fg_assembler->fa_code->co_module;
				gid = id1;
				if (prefix_type == ASM_EXTERN) {
					mod = mod->mo_importv[id1];
					gid = id2;
				}

				/* Transform everything so the global/extern access becomes a swap.
				 * -> PREFIX: LROT N   <==>   LROT N-1, PREFIX   <==>   LROT N-1; SWAP TOP, PREFIX
				 * -> PREFIX: RROT N   <==>   RROT N-1, PREFIX   <==>   SWAP TOP, PREFIX; RROT N-1 */
				switch (prefix_opcode) {
				case ASM_LROT:   /* lrot #<imm8>+2, PREFIX */
					DO(Dee_function_generator_vlrot(self, prefix_instr[1] + 2));
					break;
				case ASM16_LROT: /* lrot #<imm16>+2, PREFIX */
					DO(Dee_function_generator_vlrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 2));
					break;
				default: break;
				}

				DO(Dee_function_generator_vref(self));                                /* ..., ref:value */
				DO(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]));   /* ..., ref:value, p_global */
				DO(Dee_function_generator_grwlock_write_const(self, &mod->mo_lock));  /* - */
				DO(Dee_function_generator_vind(self, 0));                             /* ..., ref:value, *p_global */
				DO(Dee_function_generator_vreg(self, NULL));                          /* ..., ref:value, old_value */
				ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF); /* - */
				DO(Dee_function_generator_gassert_bound(self, Dee_function_generator_vtop(self), instr,
				                                        mod, gid, NULL, &mod->mo_lock));
				ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF); /* - */
				Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;       /* ..., ref:value, ref:old_value */
				DO(Dee_function_generator_vswap(self));                               /* ..., ref:old_value, ref:value */
				ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF)); /* - */
				DO(Dee_function_generator_gmov_loc2constind(self, Dee_function_generator_vtop(self), &mod->mo_globalv[gid], 0)); /* - */
				Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF;        /* ..., ref:old_value, value */
				DO(Dee_function_generator_grwlock_endwrite_const(self, &mod->mo_lock)); /* - */
				DO(Dee_function_generator_vpop(self));                                /* ..., ref:old_value */

				/* Do post-processing, now that the stack looks like: a, b, c, old_value */
				switch (prefix_opcode) {
				case ASM_RROT:   /* rrot #<imm8>+2, PREFIX */
					DO(Dee_function_generator_vrrot(self, prefix_instr[1] + 2));
					break;
				case ASM16_RROT: /* rrot #<imm16>+2, PREFIX */
					DO(Dee_function_generator_vrrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 2));
					break;
				default: break;
				}
			}	break;

			default: {
				int temp;
				DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2));
				switch (prefix_opcode) {
				case ASM_SWAP:   /* swap top, PREFIX */
					temp = Dee_function_generator_vswap(self);
					break;
				case ASM_LROT:   /* lrot #<imm8>+2, PREFIX */
					temp = Dee_function_generator_vlrot(self, prefix_instr[1] + 3);
					break;
				case ASM_RROT:   /* rrot #<imm8>+2, PREFIX */
					temp = Dee_function_generator_vrrot(self, prefix_instr[1] + 3);
					break;
				case ASM16_LROT: /* lrot #<imm16>+2, PREFIX */
					temp = Dee_function_generator_vlrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 3);
					break;
				case ASM16_RROT: /* rrot #<imm16>+2, PREFIX */
					temp = Dee_function_generator_vrrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 3);
					break;
				default: __builtin_unreachable();
				}
				if unlikely(temp)
					goto err;
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);
			}	break;

			}
			break;

		TARGET(ASM_ADD)         /* add PREFIX, pop */
		TARGET(ASM_SUB)         /* sub PREFIX, pop */
		TARGET(ASM_MUL)         /* mul PREFIX, pop */
		TARGET(ASM_DIV)         /* div PREFIX, pop */
		TARGET(ASM_MOD)         /* mod PREFIX, pop */
		TARGET(ASM_SHL)         /* shl PREFIX, pop */
		TARGET(ASM_SHR)         /* shr PREFIX, pop */
		TARGET(ASM_AND)         /* and PREFIX, pop */
		TARGET(ASM_OR)          /* or  PREFIX, pop */
		TARGET(ASM_XOR)         /* xor PREFIX, pop */
		TARGET(ASM_POW)         /* pow PREFIX, pop */
		TARGET(ASM_INC)         /* inc PREFIX */
		TARGET(ASM_DEC)         /* dec PREFIX */
		TARGET(ASM_ADD_SIMM8)   /* add PREFIX, $<Simm8> */
		TARGET(ASM_ADD_IMM32)   /* add PREFIX, $<imm32> */
		TARGET(ASM_SUB_SIMM8)   /* sub PREFIX, $<Simm8> */
		TARGET(ASM_SUB_IMM32)   /* sub PREFIX, $<imm32> */
		TARGET(ASM_MUL_SIMM8)   /* mul PREFIX, $<Simm8> */
		TARGET(ASM_DIV_SIMM8)   /* div PREFIX, $<Simm8> */
		TARGET(ASM_MOD_SIMM8)   /* mod PREFIX, $<Simm8> */
		TARGET(ASM_SHL_IMM8)    /* shl PREFIX, $<Simm8> */
		TARGET(ASM_SHR_IMM8)    /* shr PREFIX, $<Simm8> */
		TARGET(ASM_AND_IMM32)   /* and PREFIX, $<imm32> */
		TARGET(ASM_OR_IMM32)    /* or  PREFIX, $<imm32> */
		TARGET(ASM_XOR_IMM32) { /* xor PREFIX, $<imm32> */
			int temp = Dee_function_generator_vpush_prefix_addr(self, instr, prefix_type, id1, id2);
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
					DO(Dee_function_generator_vswap(self));
					break;
				default: break;
				}
				return perform_inplace_math_operation_with_vtop_addr(self, prefix_opcode, prefix_instr);
			} else {
				DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2)); /* value */
				DO(Dee_function_generator_vref(self));                                       /* ref:value */
				DO(Dee_function_generator_vlinear(self, 1, false));                          /* ref:value, addrof(ref:value) */
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
					DO(Dee_function_generator_vlrot(self, 3)); /* ref:value, addrof(ref:value), operand */
					break;
				default: break;
				}
				DO(perform_inplace_math_operation_with_vtop_addr(self, prefix_opcode, prefix_instr)); /* ref:value */
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);               /* - */
			}
		}	break;

		TARGET(ASM_INCPOST)   /* push inc PREFIX' - `PREFIX: push inc */
		TARGET(ASM_DECPOST) { /* push dec PREFIX' - `PREFIX: push dec */
			uint16_t opname;
			int temp;
			DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2)); /* value */
			temp = Dee_function_generator_vpush_prefix_addr(self, instr, prefix_type, id1, id2);
			if unlikely(temp < 0)
				goto err;
			opname = opcode == ASM_INCPOST ? OPERATOR_INC : OPERATOR_DEC;
			if (temp == 0) {                                                           /* value, addrof(ref:value) */
				DO(Dee_function_generator_vswap(self));                                /* addrof(ref:value), value */
				DO(Dee_function_generator_vop(self, OPERATOR_COPY, 1, VOP_F_PUSHRES)); /* addrof(ref:value), copy */
				DO(Dee_function_generator_vswap(self));                                /* copy, addrof(ref:value) */
				return Dee_function_generator_vop(self, opname, 1, VOP_F_INPLACE);     /* copy */
			} else {
				DO(Dee_function_generator_vref(self));                                  /* ref:value */
				DO(Dee_function_generator_vdup(self));                                  /* ref:value, value */
				DO(Dee_function_generator_vop(self, OPERATOR_COPY, 1, VOP_F_PUSHRES));  /* ref:value, copy */
				DO(Dee_function_generator_vswap(self));                                 /* copy, ref:value */
				DO(Dee_function_generator_vlinear(self, 1, false));                     /* copy, ref:value, addrof(ref:value) */
				DO(Dee_function_generator_vop(self, opname, 1, VOP_F_INPLACE));         /* copy, ref:value */
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2); /* N/A */
			}
		}	break;

		TARGET(ASM_DELOP)
		TARGET(ASM16_DELOP)
		TARGET(ASM_NOP)   /* nop PREFIX */
		TARGET(ASM16_NOP) /* nop16 PREFIX' - `PREFIX: nop16 */
			break;

		TARGET(ASM_OPERATOR) { /* PREFIX: push op $<imm8>, #<imm8> */
			uint16_t opname;
			uint8_t argc;
			int temp;
			opname = prefix_instr[1];
			argc   = prefix_instr[2];
			__IF0 {
		TARGET(ASM16_OPERATOR) /* PREFIX: push op $<imm16>, #<imm8> */
				opname = UNALIGNED_GETLE16(prefix_instr + 2);
				argc   = prefix_instr[4];
			}                                                                            /* [args...] */
			DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2)); /* [args...], value */
			temp = Dee_function_generator_vpush_prefix_addr(self, instr, prefix_type, id1, id2);
			if unlikely(temp < 0)
				goto err;
			if (temp == 0) {                                        /* [args...], value, addrof(value) */
				DO(Dee_function_generator_vrrot(self, argc + 3));   /* addrof(value), [args...], value */
				DO(Dee_function_generator_vrrot(self, argc + 3));   /* value, addrof(value), [args...] */
				DO(Dee_function_generator_vop(self, opname, argc + 1, VOP_F_INPLACE | VOP_F_PUSHRES)); /* value, result */
				DO(Dee_function_generator_vswap(self));             /* result, value */
				return Dee_function_generator_vpop(self);           /* result */
			} else {                                                /* [args...], value */
				DO(Dee_function_generator_vref(self));              /* [args...], ref:value */
				DO(Dee_function_generator_vlinear(self, 1, false)); /* [args...], ref:value, addrof(value) */
				DO(Dee_function_generator_vrrot(self, argc + 3));   /* addrof(value), [args...], ref:value */
				DO(Dee_function_generator_vrrot(self, argc + 3));   /* ref:value, addrof(value), [args...] */
				DO(Dee_function_generator_vop(self, opname, argc + 1, VOP_F_INPLACE | VOP_F_PUSHRES)); /* ref:value, result */
				DO(Dee_function_generator_vswap(self));             /* result, ref:value */
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2); /* result */
			}
		}	break;

		TARGET(ASM_OPERATOR_TUPLE) { /* PREFIX: push op $<imm8>, pop... */
			int temp;
			uint16_t opname;
			opname = prefix_instr[1];  /* PREFIX: push op $<imm16>, pop */
			__IF0 { TARGET(ASM16_OPERATOR_TUPLE) opname = UNALIGNED_GETLE16(prefix_instr + 2); }
			DO(Dee_function_generator_vpush_prefix(self, instr, prefix_type, id1, id2)); /* args, value */
			temp = Dee_function_generator_vpush_prefix_addr(self, instr, prefix_type, id1, id2);
			if unlikely(temp < 0)
				goto err;
			if (temp == 0) {                                        /* args, value, addrof(value) */
				DO(Dee_function_generator_vlrot(self, 3));          /* value, addrof(value), args */
				DO(Dee_function_generator_voptuple(self, opname, VOP_F_INPLACE | VOP_F_PUSHRES)); /* value, result */
				DO(Dee_function_generator_vswap(self));             /* result, value */
				return Dee_function_generator_vpop(self);           /* result */
			} else {                                                /* args, value */
				DO(Dee_function_generator_vref(self));              /* args, ref:value */
				DO(Dee_function_generator_vlinear(self, 1, false)); /* args, ref:value, addrof(value) */
				DO(Dee_function_generator_vlrot(self, 3));          /* value, addrof(value), args */
				DO(Dee_function_generator_voptuple(self, opname, VOP_F_INPLACE | VOP_F_PUSHRES)); /* value, result */
				DO(Dee_function_generator_vswap(self));             /* result, value */
				return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2); /* result */
			}
		}	break;

		default:
			goto unsupported_opcode;
		}
	}	break;

	default:
unsupported_opcode:
		return err_unsupported_opcode(self->fg_assembler->fa_code, instr);
	}
	return 0;
err:
	return -1;
}


/* Wrapper around `Dee_function_generator_geninstr()' to generate the entire basic block.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_genall(struct Dee_function_generator *__restrict self) {
	struct Dee_basic_block *block = self->fg_block;
	Dee_instruction_t const *instr;
	ASSERT(block->bb_mem_start != NULL);
	ASSERT(block->bb_mem_end == NULL);

	/* Generate code to delete all locals that are currently bound,
	 * but whose values don't matter as per `self->bb_locuse' */
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYDEL)) {
		struct Dee_memstate *state = self->fg_state;
		size_t lid;
		for (lid = 0; lid < state->ms_localc; ++lid) {
			if (state->ms_localv[lid].ml_type == MEMLOC_TYPE_UNALLOC)
				continue;
			if (bitset_test(block->bb_locuse, lid))
				continue;
			if (Dee_memstate_isshared(state)) {
				DO(Dee_function_generator_state_unshare(self));
				state = self->fg_state;
			}
			DO(Dee_function_generator_vdel_local(self, lid));
		}
	}

	/* Generate text. */
	block->bb_deemon_end = block->bb_deemon_end_r;
	self->fg_nextlastloc = block->bb_locreadv;
	for (instr = block->bb_deemon_start; instr < block->bb_deemon_end;) {
		Dee_instruction_t const *next_instr = DeeAsm_NextInstr(instr);

		/* TODO: If this function throw an exception that isn't an interrupt or BadAlloc,
		 *       re-wrap that exception as an IllegalInstruction, whilst also appending
		 *       DDI debug information about the origin of `instr' */
		DO(Dee_function_generator_geninstr(self, instr, &next_instr));

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

		/* Look at `self->fg_block->bb_locreadv' to delete all locals
		 * for which there are entries for the range `[instr, next_instr)' */
		DO(delete_unused_locals(self, next_instr));
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
			goto err_mem_end;
		if (error > 0)
			return 0;
	}

	return 0;
err_mem_end:
	Dee_memstate_decref(block->bb_mem_end);
	block->bb_mem_end = NULL;
err:
	ASSERT(block->bb_mem_end == NULL);
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_DEEMON_C */
