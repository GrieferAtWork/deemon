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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_DEEMON_C
#define GUARD_DEX_HOSTASM_GENERATOR_DEEMON_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
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

#include <hybrid/align.h>     /* CEILDIV */
#include <hybrid/bitset.h>    /* bitset_test */
#include <hybrid/byteswap.h>  /* UNALIGNED_GETLE16, UNALIGNED_GETLE32 */
#include <hybrid/unaligned.h> /* UNALIGNED_GETLE16, UNALIGNED_GETLE32 */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* int8_t, int16_t, uint8_t, uint16_t, uint32_t */

DECL_BEGIN

#ifndef CHAR_BIT
#include <hybrid/typecore.h> /* __CHAR_BIT__ */
#define CHAR_BIT __CHAR_BIT__
#endif /* !CHAR_BIT */

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
delete_unused_locals(struct fungen *__restrict self,
                     Dee_instruction_t const *next_instr) {
	if (self->fg_nextlastloc != NULL) {
		while (self->fg_nextlastloc->bbl_instr < next_instr) {
			/* Delete local after the last time it was read. */
			lid_t lid = self->fg_nextlastloc->bbl_lid;
			if unlikely(fg_vdel_local(self, lid))
				goto err;
			++self->fg_nextlastloc;
		}
	}
	return 0;
err:
	return -1;
}


#define ASM_MATHBLOCK_MIN ASM_CAST_INT
#define ASM_MATHBLOCK_MAX ASM_XOR_IMM32
#define ASM_MATHBLOCK_CONTAINS(opcode) ((opcode) >= ASM_MATHBLOCK_MIN && (opcode) <= ASM_MATHBLOCK_MAX)
struct opname_pair {
	Dee_operator_t op_normal;  /* Normal operator (or 0 if undefined) */
	Dee_operator_t op_inplace; /* Inplace operator (or 0 if undefined) */
};

#define ASM_MATHBLOCK_OPCODE2OPERATOR(opcode)  asm_mathblock_opnames[(opcode) - ASM_MATHBLOCK_MIN].op_normal
#define ASM_MATHBLOCK_OPCODE2IOPERATOR(opcode) asm_mathblock_opnames[(opcode) - ASM_MATHBLOCK_MIN].op_inplace
PRIVATE struct opname_pair const asm_mathblock_opnames[(ASM_MATHBLOCK_MAX - ASM_MATHBLOCK_MIN) + 1] = {
	/* [ASM_CAST_INT]  = */ { OPERATOR_INT, 0 },
	/* [ASM_INV]       = */ { OPERATOR_INV, 0 },
	/* [ASM_POS]       = */ { OPERATOR_POS, 0 },
	/* [ASM_NEG]       = */ { OPERATOR_NEG, 0 },
	/* [ASM_ADD]       = */ { OPERATOR_ADD, OPERATOR_INPLACE_ADD },
	/* [ASM_SUB]       = */ { OPERATOR_SUB, OPERATOR_INPLACE_SUB },
	/* [ASM_MUL]       = */ { OPERATOR_MUL, OPERATOR_INPLACE_MUL },
	/* [ASM_DIV]       = */ { OPERATOR_DIV, OPERATOR_INPLACE_DIV },
	/* [ASM_MOD]       = */ { OPERATOR_MOD, OPERATOR_INPLACE_MOD },
	/* [ASM_SHL]       = */ { OPERATOR_SHL, OPERATOR_INPLACE_SHL },
	/* [ASM_SHR]       = */ { OPERATOR_SHR, OPERATOR_INPLACE_SHR },
	/* [ASM_AND]       = */ { OPERATOR_AND, OPERATOR_INPLACE_AND },
	/* [ASM_OR]        = */ { OPERATOR_OR, OPERATOR_INPLACE_OR },
	/* [ASM_XOR]       = */ { OPERATOR_XOR, OPERATOR_INPLACE_XOR },
	/* [ASM_POW]       = */ { OPERATOR_POW, OPERATOR_INPLACE_POW },
	/* [ASM_INC]       = */ { 0, OPERATOR_INC },
	/* [ASM_DEC]       = */ { 0, OPERATOR_DEC },
	/* [ASM_ADD_SIMM8] = */ { OPERATOR_ADD, OPERATOR_INPLACE_ADD },
	/* [ASM_ADD_IMM32] = */ { OPERATOR_ADD, OPERATOR_INPLACE_ADD },
	/* [ASM_SUB_SIMM8] = */ { OPERATOR_SUB, OPERATOR_INPLACE_SUB },
	/* [ASM_SUB_IMM32] = */ { OPERATOR_SUB, OPERATOR_INPLACE_SUB },
	/* [ASM_MUL_SIMM8] = */ { OPERATOR_MUL, OPERATOR_INPLACE_MUL },
	/* [ASM_DIV_SIMM8] = */ { OPERATOR_DIV, OPERATOR_INPLACE_DIV },
	/* [ASM_MOD_SIMM8] = */ { OPERATOR_MOD, OPERATOR_INPLACE_MOD },
	/* [ASM_SHL_IMM8]  = */ { OPERATOR_SHL, OPERATOR_INPLACE_SHL },
	/* [ASM_SHR_IMM8]  = */ { OPERATOR_SHR, OPERATOR_INPLACE_SHR },
	/* [ASM_AND_IMM32] = */ { OPERATOR_AND, OPERATOR_INPLACE_AND },
	/* [ASM_OR_IMM32]  = */ { OPERATOR_OR, OPERATOR_INPLACE_OR },
	/* [ASM_XOR_IMM32] = */ { OPERATOR_XOR, OPERATOR_INPLACE_XOR },
};

STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_CAST_INT));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_INV) && (ASM_CAST_INT + 1 == ASM_INV));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_POS) && (ASM_INV + 1 == ASM_POS));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_NEG) && (ASM_POS + 1 == ASM_NEG));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_ADD) && (ASM_NEG + 1 == ASM_ADD));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_SUB) && (ASM_ADD + 1 == ASM_SUB));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_MUL) && (ASM_SUB + 1 == ASM_MUL));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_DIV) && (ASM_MUL + 1 == ASM_DIV));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_MOD) && (ASM_DIV + 1 == ASM_MOD));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_SHL) && (ASM_MOD + 1 == ASM_SHL));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_SHR) && (ASM_SHL + 1 == ASM_SHR));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_AND) && (ASM_SHR + 1 == ASM_AND));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_OR) && (ASM_AND + 1 == ASM_OR));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_XOR) && (ASM_OR + 1 == ASM_XOR));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_POW) && (ASM_XOR + 1 == ASM_POW));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_INC) && (ASM_POW + 1 == ASM_INC));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_DEC) && (ASM_INC + 1 == ASM_DEC));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_ADD_SIMM8) && (ASM_DEC + 1 == ASM_ADD_SIMM8));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_ADD_IMM32) && (ASM_ADD_SIMM8 + 1 == ASM_ADD_IMM32));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_SUB_SIMM8) && (ASM_ADD_IMM32 + 1 == ASM_SUB_SIMM8));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_SUB_IMM32) && (ASM_SUB_SIMM8 + 1 == ASM_SUB_IMM32));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_MUL_SIMM8) && (ASM_SUB_IMM32 + 1 == ASM_MUL_SIMM8));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_DIV_SIMM8) && (ASM_MUL_SIMM8 + 1 == ASM_DIV_SIMM8));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_MOD_SIMM8) && (ASM_DIV_SIMM8 + 1 == ASM_MOD_SIMM8));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_SHL_IMM8) && (ASM_MOD_SIMM8 + 1 == ASM_SHL_IMM8));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_SHR_IMM8) && (ASM_SHL_IMM8 + 1 == ASM_SHR_IMM8));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_AND_IMM32) && (ASM_SHR_IMM8 + 1 == ASM_AND_IMM32));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_OR_IMM32) && (ASM_AND_IMM32 + 1 == ASM_OR_IMM32));
STATIC_ASSERT(ASM_MATHBLOCK_CONTAINS(ASM_XOR_IMM32) && (ASM_OR_IMM32 + 1 == ASM_XOR_IMM32));


PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpush_prefix(struct fungen *__restrict self,
                Dee_instruction_t const *instr,
                uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	switch (prefix_type) {
	case ASM_STACK:
		return fg_vdup_at(self, self->fg_state->ms_stackc - id1);
	case ASM_STATIC:
		return fg_vpush_static(self, id1);
	case ASM_EXTERN:
		return fg_vpush_extern(self, id1, id2, true);
	case ASM_GLOBAL:
		return fg_vpush_global(self, id1, true);
	case ASM_LOCAL:
		return fg_vpush_ulocal(self, instr, id1);
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpop_prefix(struct fungen *__restrict self,
               uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	switch (prefix_type) {
	case ASM_STACK:
		return fg_vpop_at(self, self->fg_state->ms_stackc - id1);
	case ASM_STATIC:
		return fg_vpop_static(self, id1);
	case ASM_EXTERN:
		return fg_vpop_extern(self, id1, id2);
	case ASM_GLOBAL:
		return fg_vpop_global(self, id1);
	case ASM_LOCAL:
		return fg_vpop_ulocal(self, id1);
	default: __builtin_unreachable();
	}
}

/* Push the prefixed object as a `DREF DeeObject *'. In case of ASM_STACK/ASM_LOCAL,
 * make sure that no other memory location is aliasing the prefixed location.
 * @return: 0 : Success 
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpush_prefix_noalias(struct fungen *__restrict self,
                        Dee_instruction_t const *instr,
                        uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	host_cfa_t cfa_offset;
	struct memval *src_mval, *dst_mval;
	struct memstate *state;
again:
	DO(fg_state_unshare(self));
	state = self->fg_state;
	DO(memstate_reqvstack(state, state->ms_stackc + 1)); /* Pre-allocate for below */
	switch (prefix_type) {
	case ASM_STACK:
		ASSERTF(id1 < state->ms_stackc, "Should have been checked by the caller");
		src_mval = &state->ms_stackv[id1];
		break;
	case ASM_LOCAL:
		ASSERTF(id1 < state->ms_localc, "Should have been checked by the caller");
		src_mval = &state->ms_localv[id1];
		break;
	default:
		/* Fallback: other types of prefixes can never be aliased, so they can be pushed as-is */
		return fg_vpush_prefix(self, instr, prefix_type, id1, id2);
	}

	/* Make sure the prefixed value is a direct value. */
	if unlikely(!memval_isdirect(src_mval)) {
		DO(fg_vdirect_memval(self, src_mval));
		goto again;
	}

	/* In case of a local, assert that the local is currently bound. */
	if (prefix_type == ASM_LOCAL && !memval_direct_local_alwaysbound(src_mval)) {
		if (memval_direct_local_neverbound(src_mval))
			return 1; /* Cannot address always-unbound local */
		DO(fg_gassert_local_bound(self, instr, id1));
		memval_direct_local_setbound(src_mval);
	}

	/* Prefixed location must contain a reference and not be aliased. */
	if (!memval_direct_isconst(src_mval)) {
		struct memval *alias;
		struct memval *alias_with_reference = NULL;    /* Alias that has a reference */
		struct memval *alias_without_reference = NULL; /* Alias that needs a reference */
		bool got_alias = false; /* There *are* aliases. */
		memstate_foreach(alias, state) {
			if (alias == src_mval)
				continue;
			if (!memval_isdirect(alias))
				continue;
			if (!memval_direct_sameloc(alias, src_mval))
				continue;
			/* Got an alias! */
			got_alias = true;
			if (!memval_direct_isref(alias)) {
				alias_without_reference = alias;
			} else if (!memval_direct_isref(src_mval)) {
				/* Steal reference from alias */
				memval_direct_setref(src_mval);
				memval_direct_clearref(alias);
				alias_without_reference = alias;
			} else {
				alias_with_reference = alias;
			}
		}
		memstate_foreach_end;
		if (got_alias) {
			ASSERT(!alias_with_reference || memval_direct_isref(alias_with_reference));
			ASSERT(!alias_without_reference || !memval_direct_isref(alias_without_reference));
			if (!memval_direct_isref(src_mval)) {
				/* There are aliases, but no-one is holding a reference.
				 * This can happen if the location points to a constant
				 * that got flushed, in which case we only need a single
				 * reference. */
				ASSERT(alias_without_reference);
				ASSERT(!alias_with_reference);
				DO(fg_gincref_loc(self, memval_direct_getloc(src_mval), 1));
				memval_direct_setref(src_mval);
			} else if (alias_without_reference && !alias_with_reference) {
				/* There are aliases, but less that 2 references -> make sure there are at least 2 references */
				ASSERT(!memval_direct_isref(alias_without_reference));
				DO(fg_gincref_loc(self, memval_direct_getloc(alias_without_reference), 1));
				memval_direct_setref(alias_without_reference);
			}

			/* Must generate code to move the value into a second, distinct stack location. */
			cfa_offset = memstate_hstack_find(state, self->fg_state_hstack_res, HOST_SIZEOF_POINTER);
			if (cfa_offset != (host_cfa_t)-1) {
				if unlikely(fg_gmov_loc2hstackind(self, memval_direct_getloc(src_mval), cfa_offset))
					goto err;
			} else {
				if unlikely(fg_ghstack_pushloc(self, memval_direct_getloc(src_mval)))
					goto err;
#ifdef HOSTASM_STACK_GROWS_DOWN
				cfa_offset = state->ms_host_cfa_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
				cfa_offset = state->ms_host_cfa_offset - HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			}

			/* Note how we *don't* update any of the location's aliases here.
			 * From this point forth, the location doesn't have *any* aliases! */
			memloc_init_hstackind(memval_direct_getloc(src_mval), cfa_offset, 0);
		} else {
			/* No aliases exist, so there's no need to force a distinct location. */
			if (!memval_direct_isref(src_mval)) {
				DO(fg_gincref_loc(self, memval_direct_getloc(src_mval), 1));
				memval_direct_setref(src_mval);
			}
		}
	}

	/* Push the addressed location onto the stack, thus creating a singular alias.
	 * Said alias then gets to inherit the reference currently held in `src_loc'. */
	ASSERT(state->ms_stackc < state->ms_stacka);
	dst_mval = &state->ms_stackv[state->ms_stackc];
	memval_direct_initcopy(dst_mval, src_mval);
	memval_direct_clearref(src_mval); /* Reference was stolen (if there was one) */
	memstate_incrinuse_for_direct_memval(state, dst_mval);
	++state->ms_stackc;
	return 0;
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
gen_print_to_file(struct fungen *__restrict self,
                  void const *api_function, Dee_operator_t value_operator) {
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
		bool repr = false;
		char const *print_after = NULL; /* 1-character string to print *after* the object (or NULL) */
		DeeTypeObject *value_type;
		struct memval *value_mval;
		Dee_operator_t print_operator;
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
		value_mval = fg_vtop(self);
		print_operator = repr ? OPERATOR_REPR : OPERATOR_STR;
		if (memval_isconst(value_mval)) {
			DeeObject *constval = memval_const_getobj(value_mval);
			if (DeeType_GetOperatorFlags(Dee_TYPE(constval), print_operator) & METHOD_FCONSTCALL) {
				char const *utf8;
				if (repr || !DeeString_Check(constval)) {
					constval = repr ? DeeObject_Repr(constval)
					                : DeeObject_Str(constval);
					if unlikely(!constval)
						goto handle_error_and_do_fallback;
					constval = fg_inlineref(self, constval);
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
				if (print_after != NULL) {
					DO(fg_vdup_at(self, 2)); /* File, value, File */
					DO(fg_vswap(self));     /* File, File, value */
				}
				DO(fg_vpop(self));          /* [File], File */
				if (WSTR_LENGTH(utf8) == 0) {
					/* Special case: printing an empty string does nothing */
					DO(fg_vpop(self)); /* [File] */
				} else if (WSTR_LENGTH(utf8) == 1) {
					/* Special case: printing a single character allows us to use `DeeFile_Putc()' */
					DO(fg_vnotoneref_if_operator_at(self, FILE_OPERATOR_PUTC, 1)); /* [File], File */
					DO(fg_vpush_immINT(self, utf8[0]));                            /* [File], File, ch */
					DO(fg_vcallapi(self, &DeeFile_Putc, VCALL_CC_RAWINT, 2));      /* [File], status */
					DO(fg_vcheckerr(self, GETC_ERR));                              /* [File] */
				} else {
					DO(fg_vnotoneref_if_operator_at(self, FILE_OPERATOR_WRITE, 1)); /* [File], File */
					DO(fg_vpush_addr(self, utf8));                                  /* [File], File, utf8 */
					DO(fg_vpush_immSIZ(self, WSTR_LENGTH(utf8)));                   /* [File], File, utf8, length */
					DO(fg_vcallapi(self, &DeeFile_WriteAll, VCALL_CC_M1INTPTR, 3)); /* [File], print_status */
					DO(fg_vpop(self));                                              /* [File] */
				}
				goto handle_print_after_and_pop_file;
			}
		}

		/* If the type of object being printed is known, then we can inline the print-operator. */
		value_type = memval_typeof(value_mval);
		if (value_type != NULL && DeeType_InheritOperator(value_type, print_operator)) {
			api_function = repr ? value_type->tp_cast.tp_printrepr
			                    : value_type->tp_cast.tp_print;
		} else {
			api_function = repr ? (void const *)&DeeObject_PrintRepr
			                    : (void const *)&DeeObject_Print;
		}

		DO(fg_vswap(self));            /* value, File */
		DO(fg_vnotoneref_if_operator(self, repr ? OPERATOR_REPR : OPERATOR_STR, 1)); /* File, value */
		DO(fg_vnotoneref_at(self, 2)); /* File, value */
		if (print_after != NULL) {
			DO(fg_vdup(self));     /* value, File, File */
			DO(fg_vrrot(self, 3)); /* File, value, File */
		}
		DO(fg_vpush_addr(self, (void const *)&DeeFile_WriteAll)); /* [File], value, File, &DeeFile_WriteAll */
		DO(fg_vswap(self));                                       /* [File], value, &DeeFile_WriteAll, File */
		DO(fg_vcallapi(self, api_function, VCALL_CC_NEGINT, 3));  /* [File], print_status */
		DO(fg_vpop(self));                                        /* [File] */
handle_print_after_and_pop_file:
		if (print_after != NULL) {
			DO(fg_vnotoneref_if_operator_at(self, FILE_OPERATOR_PUTC, 1)); /* File */
			DO(fg_vpush_immINT(self, *print_after));                       /* File, ch */
			DO(fg_vcallapi(self, &DeeFile_Putc, VCALL_CC_RAWINT, 2));      /* status */
			return fg_vcheckerr(self, GETC_ERR);                           /* - */
		}
		return 0;
	}
fallback:
	DO(fg_vnotoneref_if_operator_at(self, value_operator, 1)); /* File, value */
	DO(fg_vnotoneref_at(self, 2));                             /* File, value */
	return fg_vcallapi(self, api_function, VCALL_CC_INT, 2);
handle_error_and_do_fallback:
	DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
	goto fallback;
err:
	return -1;
}

/* Generate code for a print-to-stdout instruction. The caller has left stdout in vtop.
 * It is assumed that this function will pop the caller-pushed stdout slot. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gen_print_with_stdout_in_vtop(struct fungen *__restrict self,
                              Dee_instruction_t const *instr, uint16_t opcode,
                              bool repr) {
	void const *api_function;
	Dee_operator_t value_operators;
	switch (opcode) {

	TARGET(ASM_PRINTNL)
		DO(fg_vnotoneref_if_operator(self, FILE_OPERATOR_PUTC, 1)); /* File */
		return fg_vcallapi(self, &DeeFile_PrintNl, VCALL_CC_INT, 1);

	TARGET(ASM_PRINT)
	TARGET(ASM_PRINT_SP)
	TARGET(ASM_PRINT_NL)
	TARGET(ASM_PRINTALL)
	TARGET(ASM_PRINTALL_SP)
	TARGET(ASM_PRINTALL_NL)
		DO(fg_vswap(self)); /* File, value */
		value_operators = repr ? OPERATOR_REPR : OPERATOR_STR;
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
			value_operators = OPERATOR_ITER;
			break;
		case ASM_PRINTALL_SP:
			api_function = (void const *)&DeeFile_PrintAllSp;
			value_operators = OPERATOR_ITER;
			break;
		case ASM_PRINTALL_NL:
			api_function = (void const *)&DeeFile_PrintAllNl;
			value_operators = OPERATOR_ITER;
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
		DO(fg_vpush_cid(self, cid)); /* File, value */
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
		value_operators = OPERATOR_STR;
	}	break;

	default: __builtin_unreachable();
	}
	return gen_print_to_file(self, api_function, value_operators);
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
fg_gen_stdout_print(struct fungen *__restrict self,
                    Dee_instruction_t const *instr,
                    Dee_instruction_t const **p_next_instr,
                    bool repr) {
	Dee_instruction_t const *iter, *print_block_end, *print_block_end_limit;
	DO(fg_state_unshare(self));

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
			DO(fg_vpush_xlocal(self, iter, MEMSTATE_XLOCAL_STDOUT));
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
			temp = fg_geninstr(self, iter, &next);
			break;
		}
		if unlikely(temp)
			goto err;
		if unlikely(self->fg_block->bb_mem_end == (DREF struct memstate *)-1)
			return 0;
		DO(delete_unused_locals(self, next));
		iter = next;
		repr = false;
	}
	*p_next_instr = iter;
	return fg_vdel_xlocal(self, MEMSTATE_XLOCAL_STDOUT); /* Delete the stdout-cache */
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
		case ASM_FOREACH_KEY:
		case ASM_FOREACH_KEY16:
		case ASM_FOREACH_VALUE:
		case ASM_FOREACH_VALUE16:
		case ASM_FOREACH_PAIR:
		case ASM_FOREACH_PAIR16:
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
					uint16_t n;
					n = instr[1] + 2;
					__IF0 { case ASM16_POP_N: n = UNALIGNED_GETLE16(instr + 2) + 2; }
					if (sptrack == stacksz - n)
						return false; /* Location will simply get overwritten! */
					sptrack = stacksz - n;
					goto continue_with_next;
				}	break;

				case ASM_DUP_N: {
					uint16_t n;
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
					uint16_t n;
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
					uint16_t n;
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
fg_geninstr(struct fungen *__restrict self,
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
		return fg_vswap(self);
	TARGET(ASM_LROT)
		return fg_vlrot(self, instr[1] + 3);
	TARGET(ASM16_LROT)
		return fg_vlrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	TARGET(ASM_RROT)
		return fg_vrrot(self, instr[1] + 3);
	TARGET(ASM16_RROT)
		return fg_vrrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	TARGET(ASM_DUP)
		return fg_vdup(self);
	TARGET(ASM_DUP_N)
		return fg_vdup_at(self, instr[1] + 2);
	TARGET(ASM16_DUP_N)
		return fg_vdup_at(self, UNALIGNED_GETLE16(instr + 2) + 2);
	TARGET(ASM_POP)
		return fg_vpop(self);
	TARGET(ASM_POP_N)
		return fg_vpop_at(self, instr[1] + 2);
	TARGET(ASM16_POP_N)
		return fg_vpop_at(self, UNALIGNED_GETLE16(instr + 2) + 2);

	TARGET(ASM_ADJSTACK) {
		vstackoff_t delta;
		delta = *(int8_t const *)(instr + 1);
		__IF0 { TARGET(ASM16_ADJSTACK) delta = (int16_t)UNALIGNED_GETLE16(instr + 2); }
		if (delta > 0) {
			do {
				DO(fg_vpush_none(self));
			} while (--delta > 0);
		} else if (delta < 0) {
			return fg_vpopmany(self, (vstackaddr_t)(-delta));
		}
	}	break;

	TARGET(ASM_POP_LOCAL)
		return fg_vpop_ulocal(self, instr[1]);
	TARGET(ASM16_POP_LOCAL)
		return fg_vpop_ulocal(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_NONE)
		return fg_vpush_none(self);
	TARGET(ASM_PUSH_THIS_MODULE)
		return fg_vpush_const(self, self->fg_assembler->fa_code->co_module);
	TARGET(ASM_PUSH_THIS_FUNCTION)
		return fg_vpush_this_function(self);

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
		return fg_vpush_const(self, push_module);
	}	break;

	TARGET(ASM_PUSH_ARG)
		return fg_vpush_arg(self, instr, instr[1]);
	TARGET(ASM16_PUSH_ARG)
		return fg_vpush_arg(self, instr, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_CONST)
		return fg_vpush_cid(self, instr[1]);
	TARGET(ASM16_PUSH_CONST) 
		return fg_vpush_cid(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_REF)
		return fg_vpush_rid(self, instr[1]);
	TARGET(ASM16_PUSH_REF)
		return fg_vpush_rid(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_EXTERN)
	TARGET(ASM16_PUSH_EXTERN)
	TARGET(ASM_PUSH_GLOBAL)
	TARGET(ASM16_PUSH_GLOBAL) {
		bool ref = matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                           (uint16_t)self->fg_state->ms_stackc + 1);
		switch (opcode) {
		case ASM_PUSH_EXTERN:
			return fg_vpush_extern(self, instr[1], instr[2], ref);
		case ASM16_PUSH_EXTERN:
			return fg_vpush_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4), ref);
		case ASM_PUSH_GLOBAL:
			return fg_vpush_global(self, instr[1], ref);
		case ASM16_PUSH_GLOBAL:
			return fg_vpush_global(self, UNALIGNED_GETLE16(instr + 2), ref);
		default: __builtin_unreachable();
		}
	}	break;

	TARGET(ASM_PUSH_LOCAL)
		return fg_vpush_ulocal(self, instr, instr[1]);
	TARGET(ASM16_PUSH_LOCAL)
		return fg_vpush_ulocal(self, instr, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_RET_NONE)
		DO(fg_vpush_none(self));
		ATTR_FALLTHROUGH
	TARGET(ASM_RET)
		return fg_vret(self);

	TARGET(ASM_THROW)
		DO(fg_vnotoneref_at(self, 1));
		return fg_vcallapi(self, &DeeError_Throw, VCALL_CC_EXCEPT, 1);

	//TODO: TARGET(ASM_SETRET)

	TARGET(ASM_DEL_LOCAL)
		return fg_vdel_ulocal(self, instr[1]);
	TARGET(ASM16_DEL_LOCAL)
		return fg_vdel_ulocal(self, UNALIGNED_GETLE16(instr + 2));

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
		DO(fg_vpush_cid(self, kw_cid));  /* func, [args...], kw */
		return fg_vopcallkw(self, argc); /* result */
	}	break;

	TARGET(ASM_CALL_TUPLE_KW) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_CALL_TUPLE_KW) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid));    /* func, args, kw */
		return fg_vopcalltuplekw(self); /* result */
	}	break;

	TARGET(ASM_PUSH_BND_ARG)
		return fg_vbound_arg(self, instr[1]);
	TARGET(ASM16_PUSH_BND_ARG)
		return fg_vbound_arg(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_BND_EXTERN)
		return fg_vbound_extern(self, instr[1], instr[2]);
	TARGET(ASM16_PUSH_BND_EXTERN)
		return fg_vbound_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));

	TARGET(ASM_PUSH_BND_GLOBAL)
		return fg_vbound_global(self, instr[1]);
	TARGET(ASM16_PUSH_BND_GLOBAL)
		return fg_vbound_global(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_BND_LOCAL)
		return fg_vbound_ulocal(self, instr, instr[1]);
	TARGET(ASM16_PUSH_BND_LOCAL)
		return fg_vbound_ulocal(self, instr, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_JF)
	TARGET(ASM_JF16)
	TARGET(ASM_JT)
	TARGET(ASM_JT16)
	TARGET(ASM_JMP)
	TARGET(ASM_JMP16)
	TARGET(ASM32_JMP) {
		bool jump_if_true;
		struct jump_descriptor *desc;
do_jcc:
		desc = jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
		ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
		        function_assembler_addrof(self->fg_assembler, instr));
		if (opcode == ASM_JF || opcode == ASM_JF16) {
			jump_if_true = false;
		} else if (opcode == ASM_JT || opcode == ASM_JT16) {
			jump_if_true = true;
		} else {
			DO(fg_vpush_const(self, Dee_True));
			jump_if_true = true;
		}
		return fg_vjcc(self, desc, instr, jump_if_true);
	}	break;

	TARGET(ASM_FOREACH)
	TARGET(ASM_FOREACH16) {
		struct jump_descriptor *desc;
		desc = jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
		ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
		        function_assembler_addrof(self->fg_assembler, instr));
		return fg_vforeach(self, desc, false);
	}	break;

	//TODO: TARGET(ASM_FOREACH_KEY)
	//TODO: TARGET(ASM_FOREACH_KEY16)
	//TODO: TARGET(ASM_FOREACH_VALUE)
	//TODO: TARGET(ASM_FOREACH_VALUE16)
	//TODO: TARGET(ASM_FOREACH_PAIR)
	//TODO: TARGET(ASM_FOREACH_PAIR16)
	//TODO: TARGET(ASM_JMP_POP)
	//TODO: TARGET(ASM_JMP_POP_POP)

	TARGET(ASM_OPERATOR)
	TARGET(ASM16_OPERATOR) {
		Dee_operator_t opname;
		uint8_t argc;
		if (opcode == ASM_OPERATOR) {
			opname = instr[1];
			argc   = instr[2];
		} else {
			opname = UNALIGNED_GETLE16(instr + 2);
			argc   = instr[4];
		}
		return fg_vop(self, opname, argc + 1, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_OPERATOR_TUPLE)
		return fg_voptuple(self, instr[1], VOP_F_PUSHRES);
	TARGET(ASM16_OPERATOR_TUPLE)
		return fg_voptuple(self, UNALIGNED_GETLE16(instr + 2), VOP_F_PUSHRES);

	TARGET(ASM_CALL)
		return fg_vopcall(self, instr[1]);

	TARGET(ASM_CALL_TUPLE)
		return fg_vopcalltuple(self);

	TARGET(ASM_DEL_GLOBAL)
		return fg_vdel_global(self, instr[1]);
	TARGET(ASM16_DEL_GLOBAL)
		return fg_vdel_global(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_SUPER)
		return fg_vopsuper(self);

	TARGET(ASM_SUPER_THIS_R) {
		uint16_t rid;
		rid = instr[1];
		__IF0 { TARGET(ASM16_SUPER_THIS_R) rid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr));
		DO(fg_vpush_rid(self, rid));
		return fg_vopsuper(self);
	}	break;

	TARGET(ASM_POP_STATIC)
		return fg_vpop_static(self, instr[1]);
	TARGET(ASM16_POP_STATIC)
		return fg_vpop_static(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_POP_EXTERN)
		return fg_vpop_extern(self, instr[1], instr[2]);
	TARGET(ASM16_POP_EXTERN)
		return fg_vpop_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));
	TARGET(ASM_POP_GLOBAL)
		return fg_vpop_global(self, instr[1]);
	TARGET(ASM16_POP_GLOBAL)
		return fg_vpop_global(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PUSH_VARARGS)
		return fg_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_VARARGS);
	TARGET(ASM_PUSH_VARKWDS)
		return fg_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_VARKWDS);

	TARGET(ASM_PUSH_STATIC)
		return fg_vpush_static(self, instr[1]);
	TARGET(ASM16_PUSH_STATIC)
		return fg_vpush_static(self, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_CAST_TUPLE)
		return fg_vopcast(self, &DeeTuple_Type);

	TARGET(ASM_CAST_LIST)
		return fg_vopcast(self, &DeeList_Type);

	TARGET(ASM_CAST_VARKWDS)
		return fg_vopcast_varkwds(self);

	TARGET(ASM_PACK_TUPLE)
	TARGET(ASM16_PACK_TUPLE)
	TARGET(ASM_PACK_LIST)
	TARGET(ASM16_PACK_LIST) {
		uint16_t elemc = instr[1];
		DeeTypeObject *seq_type = &DeeList_Type;
		if ((opcode & 0xff) != ASM_PACK_LIST)
			seq_type = &DeeTuple_Type;
		if (opcode > 0xff)
			elemc = UNALIGNED_GETLE16(instr + 2);
		return fg_vpackseq(self, seq_type, elemc);
	}	break;

	TARGET(ASM_PACK_ONE)
		/* TODO: Optimizations */
		return fg_vcallapi(self, &DeeSeq_PackOne, VCALL_CC_OBJECT, 1);

	TARGET(ASM_UNPACK)
		return fg_vopunpack(self, instr[1]);
	TARGET(ASM16_UNPACK)
		return fg_vopunpack(self, UNALIGNED_GETLE16(instr + 2));
	
	TARGET(ASM_CONCAT)
		return fg_vopconcat(self);

	TARGET(ASM_EXTEND)
		return fg_vopextend(self, instr[1]);

	TARGET(ASM_TYPEOF)
		return fg_voptypeof(self, true);

	TARGET(ASM_CLASSOF)
		return fg_vopclassof(self, true);

	TARGET(ASM_SUPEROF)
		return fg_vopsuperof(self);

	TARGET(ASM_INSTANCEOF)
		return fg_vopinstanceof(self);

	TARGET(ASM_IMPLEMENTS)
		return fg_vopimplements(self);

	TARGET(ASM_STR)
		return fg_vopstr(self);

	TARGET(ASM_BOOL)
		return fg_vopbool(self, false);

	TARGET(ASM_NOT)
		return fg_vopnot(self);

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
			return fg_gen_stdout_print(self, next_instr, p_next_instr, true);

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
			DO(fg_vnotoneref_if_operator(self, OPERATOR_REPR, 1)); /* File, value */
			DO(fg_vnotoneref_at(self, 2)); /* File, value */
			DO(fg_vswap(self));            /* value, File */
			DO(fg_vdup(self));             /* value, File, File */
			DO(fg_vlrot(self, 3));         /* File, File, value */
			return fg_vcallapi(self, api_function, VCALL_CC_OBJECT, 2); /* File */
		}	break;

		TARGET(ASM_SHL)
			/* Special handling needed here! */
			*p_next_instr = DeeAsm_NextInstr(next_instr);
			/* TODO: Optimizations when rhs is a constant. */
			/* TODO: Optimizations when the type of lhs is known. */
			DO(fg_vnotoneref_at(self, 1));                            /* lhs, rhs */
			DO(fg_vnotoneref_if_operator_at(self, OPERATOR_REPR, 2)); /* lhs, rhs */
			return fg_vcallapi(self, &libhostasm_rt_DeeObject_ShlRepr, VCALL_CC_OBJECT, 2);

		default: break;
		}
		return fg_vop(self, OPERATOR_REPR, 1, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_ASSIGN)
		return fg_vop(self, OPERATOR_ASSIGN, 2, VOP_F_NORMAL);
	TARGET(ASM_MOVE_ASSIGN)
		return fg_vop(self, OPERATOR_MOVEASSIGN, 2, VOP_F_NORMAL);

	TARGET(ASM_COPY)
		return fg_vop(self, OPERATOR_COPY, 1, VOP_F_PUSHRES);
	TARGET(ASM_DEEPCOPY)
		return fg_vop(self, OPERATOR_DEEPCOPY, 1, VOP_F_PUSHRES);
	TARGET(ASM_GETATTR)
		return fg_vopgetattr(self);
	TARGET(ASM_DELATTR)
		return fg_vopdelattr(self);
	TARGET(ASM_SETATTR)
		return fg_vopsetattr(self);
	TARGET(ASM_BOUNDATTR)
		return fg_vopboundattr(self);
	
	TARGET(ASM_GETATTR_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_GETATTR_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid));
		return fg_vopgetattr(self);
	}	break;

	TARGET(ASM_DELATTR_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_DELATTR_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid));
		return fg_vopdelattr(self);
	}	break;

	TARGET(ASM_SETATTR_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_SETATTR_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid));
		DO(fg_vswap(self));
		return fg_vopsetattr(self);
	}	break;

	TARGET(ASM_GETATTR_THIS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_GETATTR_THIS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr));
		DO(fg_vpush_cid(self, cid));
		return fg_vopgetattr(self);
	}	break;

	TARGET(ASM_DELATTR_THIS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_DELATTR_THIS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr));
		DO(fg_vpush_cid(self, cid));
		return fg_vopdelattr(self);
	}	break;

	TARGET(ASM_SETATTR_THIS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_SETATTR_THIS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr));
		DO(fg_vpush_cid(self, cid));
		DO(fg_vlrot(self, 3));
		return fg_vopsetattr(self);
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
		case ASM_CMP_EQ: return fg_vop(self, OPERATOR_EQ, 2, VOP_F_PUSHRES);
		case ASM_CMP_NE: return fg_vop(self, OPERATOR_NE, 2, VOP_F_PUSHRES);
		case ASM_CMP_GE: return fg_vop(self, OPERATOR_GE, 2, VOP_F_PUSHRES);
		case ASM_CMP_LO: return fg_vop(self, OPERATOR_LO, 2, VOP_F_PUSHRES);
		case ASM_CMP_LE: return fg_vop(self, OPERATOR_LE, 2, VOP_F_PUSHRES);
		case ASM_CMP_GR: return fg_vop(self, OPERATOR_GR, 2, VOP_F_PUSHRES);
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
		struct memobj_xinfo *xinfo;
		DeeClassDescriptorObject *cdesc;
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
			DO(fg_vpush_global(self, base_gid, true));
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
			DO(fg_vpush_extern(self, base_mid, base_gid, true));
		}	break;
		default: __builtin_unreachable();
		}
		DO(fg_vpush_cid_t(self, desc_cid, &DeeClassDescriptor_Type));
		__IF0 {
	TARGET(ASM_CLASS)
			DO(fg_vcall_DeeObject_AssertTypeExact_c_if_safe(self, &DeeClassDescriptor_Type)); /* base, desc */
		}
		cdesc = NULL;
		if (fg_vtop_isdirect(self) &&
		    memobj_isconst(fg_vtopdobj(self)))
			cdesc = (DeeClassDescriptorObject *)memobj_const_getobj(fg_vtopdobj(self));
		DO(fg_vnotoneref(self, 2));
		DO(fg_vpush_const(self, self->fg_assembler->fa_code->co_module));
		DO(fg_vcallapi(self, &DeeClass_New, VCALL_CC_OBJECT, 3));
		DO(fg_voneref_noalias(self));
		ASSERT(memval_hasobj0(fg_vtop(self)));
		if (cdesc) {
			struct memobj_xinfo_cdesc *xic;
			/* Remember the class descriptor */
			xinfo = memobj_reqxinfo(memval_getobj0(fg_vtop(self)));
			if unlikely(!xinfo)
				goto err;
			xic = (struct memobj_xinfo_cdesc *)Dee_Calloc(offsetof(struct memobj_xinfo_cdesc, moxc_init) +
				                                          CEILDIV(cdesc->cd_cmemb_size, CHAR_BIT));
			if unlikely(!xic)
				goto err;
			xic->moxc_desc = cdesc;
			Dee_Free(xinfo->mox_cdesc);
			xinfo->mox_cdesc = xic;
		}
		return 0;
	}	break;

	TARGET(ASM_DEFCMEMBER) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_DEFCMEMBER) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vdup_at(self, 2)); /* type, value, type */
		DO(fg_vswap(self));     /* type, type, value */
		return fg_vpop_cmember(self, addr, FG_CIMEMBER_F_NORMAL); /* type */
	}	break;

	TARGET(ASM16_GETCMEMBER) {
		uint16_t addr = UNALIGNED_GETLE16(instr + 2);
		unsigned int flags = FG_CIMEMBER_F_SAFE; /* Safe semantics are required here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc))
			flags |= FG_CIMEMBER_F_REF;
		return fg_vpush_cmember(self, addr, flags);
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
		flags = FG_CIMEMBER_F_NORMAL; /* Non-safe semantics are allowed here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc + 1))
			flags |= FG_CIMEMBER_F_REF;
		DO(fg_vpush_rid(self, type_rid)); /* this, type */
		return fg_vpush_cmember(self, addr, flags);
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
		DO(fg_vpush_rid(self, type_rid));  /* [args...], type */
		DO(fg_vpush_cmember(self, addr, FG_CIMEMBER_F_REF)); /* [args...], func */
		DO(fg_vrrot(self, argc + 1));      /* func, [args...] */
		DO(fg_vpush_this(self, instr));    /* func, [args...], this */
		DO(fg_vrrot(self, argc + 1));      /* func, this, [args...] */
		return fg_vopthiscall(self, argc); /* result */
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
		DeeCodeObject *code;
		bool calloc_used;
		switch (opcode) {
		case ASM_FUNCTION_C:
			code_cid = instr[1];
			refc     = instr[2];
			break;
		case ASM16_FUNCTION_C:
			code_cid = UNALIGNED_GETLE16(instr + 2);
			refc     = instr[4];
			break;
		case ASM_FUNCTION_C_16:
			code_cid = instr[1];
			refc     = UNALIGNED_GETLE16(instr + 2);
			break;
		case ASM16_FUNCTION_C_16:
			code_cid = UNALIGNED_GETLE16(instr + 2);
			refc     = UNALIGNED_GETLE16(instr + 4);
			break;
		default: __builtin_unreachable();
		}
		code = (DeeCodeObject *)fg_getconst(self, code_cid);
		if unlikely(!code)
			goto err;
		if unlikely(DeeObject_AssertTypeExact(code, &DeeCode_Type))
			goto err;
		if unlikely(code->co_refc != refc) {
			err_invalid_refs_size(code, refc);
			goto err;
		}
		calloc_used = false;
		sizeof_function = _Dee_MallococBufsize(offsetof(DeeFunctionObject, fo_refv),
			                                   (size_t)code->co_refstaticc,
			                                   sizeof(DREF DeeObject *));
		ASSERT(code->co_refstaticc >= refc);
		if likely(code->co_refstaticc <= refc) {
			DO(fg_vcall_DeeGCObject_Malloc(self, sizeof_function, false)); /* [refs...], ref:function */
		} else {
			DO(fg_vcall_DeeGCObject_Malloc(self, sizeof_function, true));  /* [refs...], ref:function */
			calloc_used = true;
		}
		ASSERT(fg_vtop_direct_isref(self));                             /* [refs...], ref:function */
		DO(fg_voneref_noalias(self));                                   /* [refs...], ref:function */
		DO(fg_vcall_DeeObject_Init_c(self, &DeeFunction_Type));         /* [refs...], ref:function */
		/* TODO: When "FUNCTION_ASSEMBLER_F_SAFE" isn't set, check if the
		 *       associated code object is used by any other ASM_FUNCTION instruction.
		 *       If it isn't (which should always be the case), then store information
		 *       about the types of objects stored as references within the code object,
		 *       so that said information can be re-used when compiling the lambda func:
		 * >> function outer() {
		 * >>     local l = [];
		 * >>     function collect(x) {
		 * >>         if (x !is none)
		 * >>             l.append(x); // Here, "l" is always DeeList_Type, so this can directly link to `list_append()'
		 * >>     }
		 * >>     collect(a());
		 * >>     collect(b());
		 * >>     collect(c());
		 * >>     return l;
		 * >> }
		 */
		DO(fg_vpush_const(self, code));               /* [refs...], ref:function, code */
		DO(fg_vref2(self, (vstackaddr_t)(refc + 2))); /* [refs...], ref:function, ref:code */
#ifdef CONFIG_HAVE_CODE_METRICS
		/* TODO: atomic_inc(&code->co_metrics.com_functions); */
#endif /* CONFIG_HAVE_CODE_METRICS */
		DO(fg_vpopind(self, offsetof(DeeFunctionObject, fo_code))); /* [refs...], ref:function */
		if (!calloc_used) {
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
			DO(fg_vpush_NULL(self) || fg_vpopind(self, offsetof(DeeFunctionObject, fo_hostasm.hafu_data)));
			DO(fg_vpush_NULL(self) || fg_vpopind(self, offsetof(DeeFunctionObject, fo_hostasm.hafu_call)));
			DO(fg_vpush_NULL(self) || fg_vpopind(self, offsetof(DeeFunctionObject, fo_hostasm.hafu_call_kw)));
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
			DO(fg_vpush_NULL(self) || fg_vpopind(self, offsetof(DeeFunctionObject, fo_hostasm.hafu_call_tuple)));
			DO(fg_vpush_NULL(self) || fg_vpopind(self, offsetof(DeeFunctionObject, fo_hostasm.hafu_call_tuple_kw)));
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#ifndef CONFIG_NO_THREADS
			DO(fg_vpush_ATOMIC_RWLOCK_INIT(self) || fg_vpopind(self, offsetof(DeeFunctionObject, fo_reflock)));
#endif /* !CONFIG_NO_THREADS */
		}
		while (refc) {
			--refc;
			DO(fg_vswap(self));                           /* [refs...], ref:function, ref */
			DO(fg_vref2(self, (vstackaddr_t)(refc + 2))); /* [refs...], ref:function, ref:ref */
			DO(fg_vpopind(self,                           /* [refs...], ref:function */
			              _Dee_MallococBufsize(offsetof(DeeFunctionObject, fo_refv),
			                                   refc, sizeof(DREF DeeObject *))));
		}
		ASSERT(fg_vtop_direct_isref(self)); /* ref:function */
		fg_vtop_direct_clearref(self);
		DO(fg_vcallapi(self, &DeeGC_Track, VCALL_CC_RAWINTPTR_NX, 1));
		ASSERT(!fg_vtop_direct_isref(self));
		fg_vtop_direct_setref(self); /* ref:function */
	}	break;

	TARGET(ASM_CAST_INT)
	TARGET(ASM_INV)
	TARGET(ASM_POS)
	TARGET(ASM_NEG)
		return fg_vop(self, ASM_MATHBLOCK_OPCODE2OPERATOR(opcode), 1, VOP_F_PUSHRES);

	TARGET(ASM_ADD)
	TARGET(ASM_SUB)
	TARGET(ASM_MUL)
	TARGET(ASM_DIV)
	TARGET(ASM_MOD)
	TARGET(ASM_SHL)
	TARGET(ASM_SHR)
	TARGET(ASM_AND)
	TARGET(ASM_OR)
	TARGET(ASM_XOR)
	TARGET(ASM_POW)
		return fg_vop(self, ASM_MATHBLOCK_OPCODE2OPERATOR(opcode), 2, VOP_F_PUSHRES);

	TARGET(ASM_ADD_SIMM8)
	TARGET(ASM_SUB_SIMM8)
	TARGET(ASM_MUL_SIMM8)
	TARGET(ASM_DIV_SIMM8)
	TARGET(ASM_MOD_SIMM8) {
		Dee_operator_t opname;
		DREF DeeObject *intval;
		intval = DeeInt_NewInt8((int8_t)instr[1]);
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		opname = ASM_MATHBLOCK_OPCODE2OPERATOR(opcode);
		DO(fg_vpush_const(self, intval)); /* lhs, rhs */
		return fg_vop(self, opname, 2, VOP_F_PUSHRES | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_SHL_IMM8)
	TARGET(ASM_SHR_IMM8) {
		Dee_operator_t opname;
		DREF DeeObject *intval;
		intval = DeeInt_NewUInt8((uint8_t)instr[1]);
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		opname = ASM_MATHBLOCK_OPCODE2OPERATOR(opcode);
		DO(fg_vpush_const(self, intval)); /* lhs, rhs */
		return fg_vop(self, opname, 2, VOP_F_PUSHRES | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_ADD_IMM32)
	TARGET(ASM_SUB_IMM32)
	TARGET(ASM_AND_IMM32)
	TARGET(ASM_OR_IMM32)
	TARGET(ASM_XOR_IMM32) {
		Dee_operator_t opname;
		DREF DeeObject *intval;
		intval = DeeInt_NewUInt32((uint32_t)UNALIGNED_GETLE32(instr + 1));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		opname = ASM_MATHBLOCK_OPCODE2OPERATOR(opcode);
		DO(fg_vpush_const(self, intval)); /* lhs, rhs */
		return fg_vop(self, opname, 2, VOP_F_PUSHRES | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_ISNONE)
		return fg_veqconstaddr(self, Dee_None);

	CASE_ASM_PRINT:
		return fg_gen_stdout_print(self, instr, p_next_instr, false);

	TARGET(ASM_FPRINT)         /* print top, pop */
	TARGET(ASM_FPRINT_SP)      /* print top, pop, sp */
	TARGET(ASM_FPRINT_NL)      /* print top, pop, nl */
	TARGET(ASM_FPRINTALL)      /* print top, pop... */
	TARGET(ASM_FPRINTALL_SP)   /* print top, pop..., sp */
	TARGET(ASM_FPRINTALL_NL) { /* print top, pop..., nl */
		void const *api_function;
		Dee_operator_t value_operators = OPERATOR_STR;
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
			value_operators = OPERATOR_ITER;
			break;
		case ASM_FPRINTALL_SP: /* print top, pop..., sp */
			api_function = (void const *)&DeeFile_PrintAllSp;
			value_operators = OPERATOR_ITER;
			break;
		case ASM_FPRINTALL_NL: /* print top, pop..., nl */
			api_function = (void const *)&DeeFile_PrintAllNl;
			value_operators = OPERATOR_ITER;
			break;
		default: __builtin_unreachable();
		}
		DO(fg_vswap(self));    /* value, File */
		DO(fg_vdup(self));     /* value, File, File */
		DO(fg_vlrot(self, 3)); /* File, File, value */
		return gen_print_to_file(self, api_function, value_operators);
	}	break;

	TARGET(ASM_FPRINTNL) /* print top, nl */
		DO(fg_vdup(self)); /* File, File */
		return fg_vcallapi(self, &DeeFile_PrintNl, VCALL_CC_INT, 1);

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
		DO(fg_vdup(self));           /* File, File */
		DO(fg_vpush_cid(self, cid)); /* File, File, value */
		return gen_print_to_file(self, api_function, OPERATOR_STR);
	}	break;

	TARGET(ASM_ENTER)
		DO(fg_vdup(self));
		return fg_vop(self, OPERATOR_ENTER, 1, VOP_F_NORMAL);
	TARGET(ASM_LEAVE)
		return fg_vop(self, OPERATOR_LEAVE, 1, VOP_F_NORMAL);

	TARGET(ASM_RANGE)                          /* start, end */
		DO(fg_vnotoneref(self, 2));            /* start, end */
		DO(fg_vswap(self));                    /* end, start */
		DO(fg_vpush_none(self));               /* end, start, Dee_None */
		DO(fg_vpush_const(self, DeeInt_Zero)); /* end, start, Dee_None, DeeInt_Zero */
		DO(fg_vcoalesce(self));                /* end, used_start */
		DO(fg_vswap(self));                    /* used_start, end */
		DO(fg_vpush_addr(self, NULL));         /* used_start, end, NULL */
		DO(fg_vcallapi(self, &DeeRange_New, VCALL_CC_OBJECT, 3)); /* result */
		return fg_voneref_noalias(self);       /* result */

	TARGET(ASM_RANGE_DEF)                                                 /* end */
		DO(fg_vnotoneref_at(self, 1));                                    /* end */
		DO(fg_vdup(self));                                                /* end, end */
		DO(fg_voptypeof(self, false));                                    /* end, type(end) */
		DO(fg_vcallapi(self, &DeeObject_NewDefault, VCALL_CC_OBJECT, 1)); /* end, type(end)() */
		DO(fg_vswap(self));                                               /* type(end)(), end */
		DO(fg_vpush_addr(self, NULL));                                    /* type(end)(), end, NULL */
		DO(fg_vcallapi(self, &DeeRange_New, VCALL_CC_OBJECT, 3));         /* result */
		return fg_voneref_noalias(self);                                  /* result */

	TARGET(ASM_RANGE_STEP)                     /* start, end, step */
		DO(fg_vnotoneref(self, 3));            /* start, end, step */
		DO(fg_vpush_none(self));               /* start, end, step, Dee_None */
		DO(fg_vpush_addr(self, NULL));         /* start, end, step, Dee_None, NULL */
		DO(fg_vcoalesce(self));                /* start, end, used_step */
		DO(fg_vlrot(self, 3));                 /* end, used_step, start */
		DO(fg_vpush_none(self));               /* end, used_step, start, Dee_None */
		DO(fg_vpush_const(self, DeeInt_Zero)); /* end, used_step, start, Dee_None, DeeInt_Zero */
		DO(fg_vcoalesce(self));                /* end, used_step, used_start */
		DO(fg_vrrot(self, 3));                 /* used_start, end, used_step */
		DO(fg_vcallapi(self, &DeeRange_New, VCALL_CC_OBJECT, 3)); /* result */
		return fg_voneref_noalias(self);       /* result */

	TARGET(ASM_RANGE_STEP_DEF)           /* end, step */
		DO(fg_vnotoneref(self, 2));      /* end, step */
		DO(fg_vpush_none(self));         /* end, step, Dee_None */
		DO(fg_vpush_addr(self, NULL));   /* end, step, Dee_None, NULL */
		DO(fg_vcoalesce(self));          /* end, used_step */
		DO(fg_vswap(self));              /* used_step, end */
		DO(fg_vdup(self));               /* used_step, end, end */
		DO(fg_voptypeof(self, false));   /* used_step, end, type(end) */
		DO(fg_vcallapi(self, &DeeObject_NewDefault, VCALL_CC_OBJECT, 1)); /* used_step, end, type(end)() */
		DO(fg_vswap(self));              /* used_step, type(end)(), end */
		DO(fg_vlrot(self, 3));           /* type(end)(), end, used_step */
		DO(fg_vcallapi(self, &DeeRange_New, VCALL_CC_OBJECT, 3)); /* result */
		return fg_voneref_noalias(self); /* result */

	TARGET(ASM_RANGE_0_I16) {
		uint32_t imm32;
		imm32 = UNALIGNED_GETLE16(instr + 1);
		__IF0 { TARGET(ASM_RANGE_0_I32) imm32 = UNALIGNED_GETLE32(instr + 2); }
		DO(fg_vpush_imm32(self, 0));
		DO(fg_vpush_imm32(self, imm32));
		DO(fg_vpush_imm32(self, 1));
		DO(fg_vcallapi(self, &DeeRange_NewInt, VCALL_CC_OBJECT, 3));
		return fg_voneref_noalias(self); /* result */
	}	break;

	TARGET(ASM_CONTAINS)
		return fg_vop(self, OPERATOR_CONTAINS, 2, VOP_F_PUSHRES);

	TARGET(ASM_CONTAINS_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_CONTAINS_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid)); /* item, seq */
		DO(fg_vswap(self));          /* seq, item */
		return fg_vop(self, OPERATOR_CONTAINS, 2, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_GETITEM)
		return fg_vop(self, OPERATOR_GETITEM, 2, VOP_F_PUSHRES);

	TARGET(ASM_GETITEM_I) {
		DREF DeeObject *intval;
		intval = DeeInt_NewUInt16(UNALIGNED_GETLE16(instr + 1));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		DO(fg_vpush_const(self, intval)); /* seq, index */
		return fg_vop(self, OPERATOR_GETITEM, 2, VOP_F_PUSHRES | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_GETITEM_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_GETITEM_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid)); /* seq, index */
		return fg_vop(self, OPERATOR_GETITEM, 2, VOP_F_PUSHRES);
	}	break;

	TARGET(ASM_GETSIZE)
		return fg_vopsize(self);

	TARGET(ASM_SETITEM_I) {
		DREF DeeObject *intval;
		intval = DeeInt_NewUInt16(UNALIGNED_GETLE16(instr + 1));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		DO(fg_vpush_const(self, intval)); /* seq, value, index */
		DO(fg_vswap(self));               /* seq, index, value */
		return fg_vop(self, OPERATOR_SETITEM, 3, VOP_F_PUSHRES | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_SETITEM_C) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_SETITEM_C) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid));
		DO(fg_vswap(self));
		DO(fg_vop(self, OPERATOR_SETITEM, 3, VOP_F_PUSHRES));
		return fg_vpop(self);
	}	break;

	TARGET(ASM_ITERSELF)
		return fg_vop(self, OPERATOR_ITER, 1, VOP_F_PUSHRES);

	TARGET(ASM_DELITEM)
		return fg_vop(self, OPERATOR_DELITEM, 2, VOP_F_NORMAL);

	TARGET(ASM_SETITEM)
		return fg_vop(self, OPERATOR_SETITEM, 3, VOP_F_NORMAL);

	TARGET(ASM_GETRANGE)
		return fg_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES);

	TARGET(ASM_GETRANGE_PN)      /* seq, start */
		DO(fg_vpush_none(self)); /* seq, start, end */
		return fg_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES);

	TARGET(ASM_GETRANGE_NP)      /* seq, end */
		DO(fg_vpush_none(self)); /* seq, end, begin */
		DO(fg_vswap(self));      /* seq, begin, end */
		return fg_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES);

	TARGET(ASM_GETRANGE_PI)
	TARGET(ASM_GETRANGE_NI)
	TARGET(ASM_GETRANGE_IP)
	TARGET(ASM_GETRANGE_IN) {
		DREF DeeObject *intval;
		intval = DeeInt_NewInt16((int16_t)UNALIGNED_GETLE16(instr + 1));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		switch (opcode) {
		case ASM_GETRANGE_PI:                 /* seq, begin */
			DO(fg_vpush_const(self, intval)); /* seq, begin, end */
			break;
		case ASM_GETRANGE_NI:                 /* seq */
			DO(fg_vpush_none(self));          /* seq, begin */
			DO(fg_vpush_const(self, intval)); /* seq, begin, end */
			break;
		case ASM_GETRANGE_IP:                 /* seq, end */
			DO(fg_vpush_const(self, intval)); /* seq, end, begin */
			DO(fg_vswap(self));               /* seq, begin, end */
			break;
		case ASM_GETRANGE_IN:                 /* seq */
			DO(fg_vpush_const(self, intval)); /* seq, begin */
			DO(fg_vpush_none(self));          /* seq, begin, end */
			break;
		default: __builtin_unreachable();
		}
		return fg_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_GETRANGE_II) {
		DREF DeeObject *intval;
		intval = DeeInt_NewInt16((int16_t)UNALIGNED_GETLE16(instr + 1));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		DO(fg_vpush_const(self, intval));
		intval = DeeInt_NewInt16((int16_t)UNALIGNED_GETLE16(instr + 3));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		DO(fg_vpush_const(self, intval));
		return fg_vop(self, OPERATOR_GETRANGE, 3, VOP_F_PUSHRES | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_DELRANGE)
		return fg_vop(self, OPERATOR_DELRANGE, 3, VOP_F_NORMAL);

	TARGET(ASM_SETRANGE)
		return fg_vop(self, OPERATOR_SETRANGE, 4, VOP_F_NORMAL);

	TARGET(ASM_SETRANGE_PN)
		DO(fg_vpush_none(self));
		return fg_vop(self, OPERATOR_SETRANGE, 3, VOP_F_NORMAL);

	TARGET(ASM_SETRANGE_NP)
		DO(fg_vpush_none(self));
		DO(fg_vswap(self));
		return fg_vop(self, OPERATOR_SETRANGE, 3, VOP_F_NORMAL);

	TARGET(ASM_SETRANGE_PI)   /* seq, begin, value */
	TARGET(ASM_SETRANGE_NI)   /* seq, value */
	TARGET(ASM_SETRANGE_IP)   /* seq, end, value */
	TARGET(ASM_SETRANGE_IN) { /* seq, value */
		DREF DeeObject *intval;
		intval = DeeInt_NewInt16((int16_t)UNALIGNED_GETLE16(instr + 1));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		switch (opcode) {
		case ASM_SETRANGE_PI:                 /* seq, begin, value */
			DO(fg_vpush_const(self, intval)); /* seq, begin, value, end */
			DO(fg_vswap(self));               /* seq, begin, end, value */
			break;
		case ASM_SETRANGE_NI:                 /* seq, value */
			DO(fg_vpush_none(self));          /* seq, value, begin */
			DO(fg_vpush_const(self, intval)); /* seq, value, begin, end */
			DO(fg_vlrot(self, 3));            /* seq, begin, end, value */
			break;
		case ASM_SETRANGE_IP:                 /* seq, end, value */
			DO(fg_vpush_const(self, intval)); /* seq, end, value, begin */
			DO(fg_vrrot(self, 3));            /* seq, begin, end, value */
			break;
		case ASM_SETRANGE_IN:                 /* seq, value */
			DO(fg_vpush_const(self, intval)); /* seq, value, begin */
			DO(fg_vpush_none(self));          /* seq, value, begin, end */
			DO(fg_vlrot(self, 3));            /* seq, begin, end, value */
			break;
		default: __builtin_unreachable();
		}
		return fg_vop(self, OPERATOR_SETRANGE, 4, VOP_F_NORMAL | VOP_F_ALLOWNATIVE);
	}	break;

	TARGET(ASM_SETRANGE_II) {
		DREF DeeObject *intval;
		intval = DeeInt_NewInt16((int16_t)UNALIGNED_GETLE16(instr + 1));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		DO(fg_vpush_const(self, intval));
		intval = DeeInt_NewInt16((int16_t)UNALIGNED_GETLE16(instr + 3));
		if unlikely(!intval)
			goto err;
		intval = fg_inlineref(self, intval);
		if unlikely(!intval)
			goto err;
		DO(fg_vpush_const(self, intval));
		return fg_vop(self, OPERATOR_SETRANGE, 4, VOP_F_NORMAL | VOP_F_ALLOWNATIVE);
	}	break;

	//TODO: TARGET(ASM_BREAKPOINT)

	TARGET(ASM_UD)
		DO(fg_vpush_addr(self, instr));
		DO(fg_vpush_const(self, self->fg_assembler->fa_code));
		return fg_vcallapi(self, &libhostasm_rt_err_illegal_instruction, VCALL_CC_EXCEPT, 2);

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
		DO(fg_vpush_cid(self, attr_cid)); /* this, [args...], attr */
		DO(fg_vrrot(self, argc + 1));     /* this, attr, [args...] */
		DO(fg_vpush_cid(self, kwds_cid)); /* this, attr, [args...], kw */
		return fg_vopcallattrkw(self, argc);
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
		DO(fg_vpush_cid(self, attr_cid));
		DO(fg_vswap(self));
		DO(fg_vpush_cid(self, kwds_cid));
		ATTR_FALLTHROUGH
	TARGET(ASM_CALLATTR_TUPLE_KWDS)
		return fg_vopcallattrtuplekw(self);
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
			DO(fg_vpush_cid(self, attr_cid)); /* this, [args...], attr */
			DO(fg_vrrot(self, argc + 1));     /* this, attr, [args...] */
		}
		return fg_vopcallattr(self, argc);
	}	break;

	TARGET(ASM_CALLATTR_C_TUPLE) {
		uint16_t cid;
		cid = instr[1];
		__IF0 { TARGET(ASM16_CALLATTR_C_TUPLE) cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_cid(self, cid));
		DO(fg_vswap(self));
		ATTR_FALLTHROUGH
	TARGET(ASM_CALLATTR_TUPLE)
		return fg_vopcallattrtuple(self);
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
		DO(fg_vpush_this(self, instr));    /* [args...], this */
		DO(fg_vrrot(self, argc + 1));      /* this, [args...] */
		DO(fg_vpush_cid(self, attr_cid));  /* this, [args...], attr */
		DO(fg_vrrot(self, argc + 1));      /* this, attr, [args...] */
		return fg_vopcallattr(self, argc); /* result */
	}	break;

	TARGET(ASM_CALLATTR_THIS_C_TUPLE) {
		uint16_t attr_cid;
		attr_cid = instr[1];
		__IF0 { TARGET(ASM16_CALLATTR_THIS_C_TUPLE) attr_cid = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr));   /* args, this */
		DO(fg_vpush_cid(self, attr_cid)); /* args, this, attr */
		DO(fg_vlrot(self, 3));            /* this, attr, args */
		return fg_vopcallattrtuple(self); /* result */
	}	break;

	TARGET(ASM_CALLATTR_C_SEQ)
	TARGET(ASM_CALLATTR_C_MAP) {
		uint16_t attr_cid;
		uint16_t itemc;
		vstackaddr_t elemc;
		attr_cid = instr[1];
		itemc    = instr[2];
		__IF0 {
	TARGET(ASM16_CALLATTR_C_SEQ)
	TARGET(ASM16_CALLATTR_C_MAP)
			attr_cid = UNALIGNED_GETLE16(instr + 2);
			itemc    = instr[4];
		}
		elemc = (opcode & 0xff) == ASM_CALLATTR_C_MAP ? itemc * 2 : itemc;
		DO(fg_vpush_cid(self, attr_cid)); /* this, [args...], attr */
		DO(fg_vrrot(self, elemc + 1));    /* this, attr, [args...] */
		return (opcode & 0xff) == ASM_CALLATTR_C_MAP
		       ? fg_vopcallattrmap(self, itemc)
		       : fg_vopcallattrseq(self, itemc);
	}	break;

	TARGET(ASM_CALLATTR_KWDS) {
		uint8_t argc = instr[1]; /* this, attr, [args...], kwds */
		return fg_vopcallattrkw(self, argc);
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
			DO(fg_vpush_extern(self, mid, gid, true)); /* [args...], func */
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
			DO(fg_vpush_global(self, gid, true)); /* [args...], func */
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
			DO(fg_vpush_ulocal(self, instr, lid)); /* [args...], func */
		}	break;
		default: __builtin_unreachable();
		}                              /* [args...], func */
		DO(fg_vrrot(self, argc + 1));  /* func, [args...] */
		return fg_vopcall(self, argc); /* result */
	}	break;

	TARGET(ASM_CALL_SEQ)
		return fg_vopcallseq(self, instr[2]);

	TARGET(ASM_CALL_MAP)
		return fg_vopcallmap(self, instr[2]);

	TARGET(ASM_THISCALL_TUPLE)
		return fg_vopthiscalltuple(self);

	TARGET(ASM_CALL_TUPLE_KWDS)
		return fg_vopcalltuplekw(self);

	TARGET(ASM_PUSH_EXCEPT)
		return fg_vpush_except(self);

	TARGET(ASM_PUSH_THIS)
		return fg_vpush_this(self, instr);

	TARGET(ASM_CAST_HASHSET)
		return fg_vopcast(self, &DeeHashSet_Type);

	TARGET(ASM_CAST_DICT)
		return fg_vopcast(self, &DeeDict_Type);

	TARGET(ASM_PUSH_TRUE)
		return fg_vpush_const(self, Dee_True);

	TARGET(ASM_PUSH_FALSE)
		return fg_vpush_const(self, Dee_False);

	TARGET(ASM_PACK_HASHSET)
		return fg_vpackseq(self, &DeeHashSet_Type, instr[2]);
	TARGET(ASM16_PACK_HASHSET)
		return fg_vpackseq(self, &DeeHashSet_Type, UNALIGNED_GETLE16(instr + 2));

	TARGET(ASM_PACK_DICT)
		return fg_vpackseq(self, &DeeDict_Type, instr[2] * 2);
	TARGET(ASM16_PACK_DICT)
		return fg_vpackseq(self, &DeeDict_Type, UNALIGNED_GETLE16(instr + 2) * 2);

	TARGET(ASM_BOUNDITEM)
		return fg_vopbounditem(self);

	TARGET(ASM_CMP_SO)
	TARGET(ASM_CMP_DO)
		/* TODO: Special handling for when the instruction that eventually pops the
		 *       value pushed here is ASM_JT/ASM_JF, in which case that jump needs
		 *       to be implemented using `fg_gjcc()' */
		DO(fg_veqaddr(self));
		if (opcode == ASM_CMP_DO)
			return fg_vopnot(self);
		break;


	TARGET(ASM_SUPERGETATTR_THIS_RC) {
		uint16_t type_rid, attr_cid;
		type_rid = instr[2];
		attr_cid = instr[3];
		__IF0 {
	TARGET(ASM16_SUPERGETATTR_THIS_RC)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			attr_cid = UNALIGNED_GETLE16(instr + 4);
		}
		DO(fg_vpush_this(self, instr));   /* this */
		DO(fg_vpush_rid(self, type_rid)); /* this, super */
		DO(fg_vopsuper(self));            /* this as super */
		DO(fg_vpush_cid(self, attr_cid)); /* this as super, attr */
		return fg_vopgetattr(self);       /* result */
	}	break;

	TARGET(ASM_SUPERCALLATTR_THIS_RC) {
		uint16_t type_rid, attr_cid;
		uint8_t argc;
		type_rid = instr[2];
		attr_cid = instr[3];
		argc     = instr[4];
		__IF0 {
	TARGET(ASM16_SUPERCALLATTR_THIS_RC)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			attr_cid = UNALIGNED_GETLE16(instr + 4);
			argc     = instr[6];
		}
		DO(fg_vpush_this(self, instr));    /* [args...], this */
		DO(fg_vpush_rid(self, type_rid));  /* [args...], this, super */
		DO(fg_vopsuper(self));             /* [args...], this as super */
		DO(fg_vrrot(self, argc + 1));      /* this as super, [args...] */
		DO(fg_vpush_cid(self, attr_cid));  /* this as super, [args...], attr */
		DO(fg_vrrot(self, argc + 1));      /* this as super, attr, [args...] */
		return fg_vopcallattr(self, argc); /* result */
	}	break;

	TARGET(ASM_REDUCE_MIN)
		DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1));
		return fg_vcallapi(self, &DeeSeq_Min, VCALL_CC_OBJECT, 1);

	TARGET(ASM_REDUCE_MAX)
		DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1));
		return fg_vcallapi(self, &DeeSeq_Max, VCALL_CC_OBJECT, 1);

	TARGET(ASM_REDUCE_SUM)
		DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1));
		return fg_vcallapi(self, &DeeSeq_Sum, VCALL_CC_OBJECT, 1);

	TARGET(ASM_REDUCE_ANY)
	TARGET(ASM_REDUCE_ALL)
		DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1));
		DO(fg_vcallapi(self,
		               opcode == ASM_REDUCE_ANY ? (void const *)&DeeSeq_Any
		                                        : (void const *)&DeeSeq_All,
		               VCALL_CC_NEGINT, 1));
		DO(fg_vdirect1(self));
		ASSERT(fg_vtop_isdirect(self));
		fg_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
		break;

	//TODO: TARGET(ASM_VARARGS_UNPACK)
	//TODO: TARGET(ASM_PUSH_VARKWDS_NE)
	//TODO: TARGET(ASM_VARARGS_CMP_EQ_SZ)
	//TODO: TARGET(ASM_VARARGS_CMP_GR_SZ)
	//TODO: TARGET(ASM_VARARGS_GETITEM)
	//TODO: TARGET(ASM_VARARGS_GETITEM_I)
	//TODO: TARGET(ASM_VARARGS_GETSIZE)

	TARGET(ASM_ITERNEXT)
		return fg_vop(self, OPERATOR_ITERNEXT, 1, VOP_F_PUSHRES);

	TARGET(ASM_GETMEMBER) {
		uint16_t addr;
		unsigned int flags;
		addr = instr[1];
		__IF0 { TARGET(ASM16_GETMEMBER) addr = UNALIGNED_GETLE16(instr + 2); }
		flags = FG_CIMEMBER_F_SAFE; /* Safe semantics are required here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc - 1))
			flags |= FG_CIMEMBER_F_REF;
		return fg_vpush_imember(self, addr, flags);
	}	break;

	TARGET(ASM_DELMEMBER)
		return fg_vdel_imember(self, instr[1], FG_CIMEMBER_F_SAFE);
	TARGET(ASM16_DELMEMBER)
		return fg_vdel_imember(self, UNALIGNED_GETLE16(instr + 2), FG_CIMEMBER_F_SAFE);
	TARGET(ASM_SETMEMBER)
		return fg_vpop_imember(self, instr[1], FG_CIMEMBER_F_SAFE);
	TARGET(ASM16_SETMEMBER)
		return fg_vpop_imember(self, UNALIGNED_GETLE16(instr + 2), FG_CIMEMBER_F_SAFE);
	TARGET(ASM_BOUNDMEMBER)
		return fg_vbound_imember(self, instr[1], FG_CIMEMBER_F_SAFE);
	TARGET(ASM16_BOUNDMEMBER)
		return fg_vbound_imember(self, UNALIGNED_GETLE16(instr + 2), FG_CIMEMBER_F_SAFE);

	TARGET(ASM_GETMEMBER_THIS) {
		uint16_t addr;
		unsigned int flags;
		addr = instr[1];
		__IF0 { TARGET(ASM16_GETMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		flags = FG_CIMEMBER_F_SAFE; /* Safe semantics are required here */
		if (matching_pop_requires_reference(*p_next_instr, self->fg_block->bb_deemon_end,
		                                    (uint16_t)self->fg_state->ms_stackc))
			flags |= FG_CIMEMBER_F_REF;
		DO(fg_vpush_this(self, instr)); /* type, this */
		DO(fg_vswap(self));             /* this, type */
		return fg_vpush_imember(self, addr, flags);
	}	break;

	TARGET(ASM_DELMEMBER_THIS) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_DELMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr)); /* type, this */
		DO(fg_vswap(self));             /* this, type */
		return fg_vdel_imember(self, addr, FG_CIMEMBER_F_SAFE);
	}	break;

	TARGET(ASM_SETMEMBER_THIS) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_SETMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr)); /* type, value, this */
		DO(fg_vrrot(self, 3));          /* this, type, value */
		return fg_vpop_imember(self, addr, FG_CIMEMBER_F_SAFE);
	}	break;

	TARGET(ASM_BOUNDMEMBER_THIS) {
		uint16_t addr;
		addr = instr[1];
		__IF0 { TARGET(ASM16_BOUNDMEMBER_THIS) addr = UNALIGNED_GETLE16(instr + 2); }
		DO(fg_vpush_this(self, instr)); /* type, this */
		DO(fg_vswap(self));             /* this, type */
		return fg_vbound_imember(self, addr, FG_CIMEMBER_F_SAFE);
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
		DO(fg_vpush_this(self, instr));           /* this */
		DO(fg_vpush_rid(self, type_rid));         /* this, type */
		return fg_vpush_imember(self, addr, ref); /* value */
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
		DO(fg_vpush_this(self, instr));   /* this */
		DO(fg_vpush_rid(self, type_rid)); /* this, type */
		return fg_vdel_imember(self, addr, FG_CIMEMBER_F_NORMAL); 
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
		DO(fg_vpush_this(self, instr));   /* value, this */
		DO(fg_vpush_rid(self, type_rid)); /* value, this, type */
		DO(fg_vlrot(self, 3));            /* this, type, value */
		return fg_vpop_imember(self, addr, FG_CIMEMBER_F_NORMAL);
	}	break;

	//TODO:TARGET(ASM_SETMEMBERI_THIS_R) {
	//TODO:	uint16_t type_rid, addr;
	//TODO:	type_rid = instr[1];
	//TODO:	addr     = instr[2];
	//TODO:	__IF0 {
	//TODO:TARGET(ASM16_SETMEMBERI_THIS_R)
	//TODO:		type_rid = UNALIGNED_GETLE16(instr + 2);
	//TODO:		addr     = UNALIGNED_GETLE16(instr + 4);
	//TODO:	}
	//TODO:	DO(fg_vpush_this(self, instr));   /* value, this */
	//TODO:	DO(fg_vpush_rid(self, type_rid)); /* value, this, type */
	//TODO:	DO(fg_vlrot(self, 3));            /* this, type, value */
	//TODO:	return fg_vpop_imember(self, addr, FG_CIMEMBER_F_NORMAL);
	//TODO:}	break;

	TARGET(ASM_BOUNDMEMBER_THIS_R) {
		uint16_t type_rid, addr;
		type_rid = instr[1];
		addr     = instr[2];
		__IF0 {
	TARGET(ASM16_BOUNDMEMBER_THIS_R)
			type_rid = UNALIGNED_GETLE16(instr + 2);
			addr     = UNALIGNED_GETLE16(instr + 4);
		}
		DO(fg_vpush_this(self, instr));   /* this */
		DO(fg_vpush_rid(self, type_rid)); /* this, type */
		return fg_vbound_imember(self, addr, FG_CIMEMBER_F_NORMAL); 
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
			if unlikely(id1 >= self->fg_assembler->fa_code->co_constc)
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
			DO(fg_vpush_prefix(self, instr, prefix_type, id1, id2));
			opcode = prefix_opcode;
			/* Need to do this in a special way because `instr' must not become `prefix_instr' here. */
			goto do_jcc;

		TARGET(ASM_FOREACH)     /* foreach PREFIX, <Sdisp8> */
		TARGET(ASM_FOREACH16) { /* foreach PREFIX, <Sdisp16> */
			struct jump_descriptor *desc;
			desc = jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
			ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
			        function_assembler_addrof(self->fg_assembler, instr));
			DO(fg_vpush_prefix(self, instr, prefix_type, id1, id2));
			return fg_vforeach(self, desc, true);
		}	break;

		//TODO: TARGET(ASM_FOREACH_KEY)
		//TODO: TARGET(ASM_FOREACH_KEY16)
		//TODO: TARGET(ASM_FOREACH_VALUE)
		//TODO: TARGET(ASM_FOREACH_VALUE16)
		//TODO: TARGET(ASM_FOREACH_PAIR)
		//TODO: TARGET(ASM_FOREACH_PAIR16)

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
			DO(fg_vpush_prefix(self, instr, prefix_type, id1, id2));
			return fg_geninstr(self, prefix_instr, p_next_instr);

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
		case ASM_FUNCTION_C:         /* PREFIX: function const <imm8>, #<imm8> */
		case ASM16_FUNCTION_C:       /* PREFIX: function const <imm16>, #<imm8> */
		case ASM_FUNCTION_C_16:      /* PREFIX: function const <imm8>, #<imm16> */
		case ASM16_FUNCTION_C_16:    /* PREFIX: function const <imm16>, #<imm16> */
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
			DO(fg_geninstr(self, prefix_instr, p_next_instr));
			if unlikely(self->fg_block->bb_mem_end == (DREF struct memstate *)-1)
				return 0;
			return fg_vpop_prefix(self, prefix_type, id1, id2);

		TARGET(ASM_POP_N) { /* mov #SP - <imm8> - 2, PREFIX */
			uint16_t n;
			n = prefix_instr[1] + 2; /* mov #SP - <imm16> - 2, PREFIX */
			__IF0 { TARGET(ASM16_POP_N) n = UNALIGNED_GETLE16(prefix_instr + 2) + 2; }
			DO(fg_vpush_prefix(self, instr, prefix_type, id1, id2));
			DO(fg_vlrot(self, n + 1));
			DO(fg_vpop(self));
			return fg_vrrot(self, n);
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
					DO(fg_vlrot(self, prefix_instr[1] + 2));
					break;
				case ASM16_LROT: /* lrot #<imm16>+2, PREFIX */
					DO(fg_vlrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 2));
					break;
				default: break;
				}

				DO(fg_vref2(self, 0));                            /* ..., ref:value */
				DO(fg_vpush_addr(self, &mod->mo_globalv[gid]));   /* ..., ref:value, p_global */
#ifndef CONFIG_NO_THREADS
				DO(fg_grwlock_write_const(self, &mod->mo_lock));  /* - */
#endif /* !CONFIG_NO_THREADS */
				DO(fg_vind(self, 0));                             /* ..., ref:value, *p_global */
				DO(fg_vreg(self, NULL));                          /* ..., ref:value, old_value */
				ASSERT(!fg_vtop_direct_isref(self));              /* - */
#ifndef CONFIG_NO_THREADS
				DO(fg_gassert_bound(self, fg_vtopdloc(self), instr, mod, gid, NULL, &mod->mo_lock));
#else /* !CONFIG_NO_THREADS */
				DO(fg_gassert_bound(self, fg_vtopdloc(self), instr, mod, gid));
#endif /* CONFIG_NO_THREADS */
				ASSERT(!fg_vtop_direct_isref(self));              /* - */
				fg_vtop_direct_setref(self);                      /* ..., ref:value, ref:old_value */
				DO(fg_vswap(self));                               /* ..., ref:old_value, ref:value */
				ASSERT(fg_vtop_direct_isref(self));               /* - */
				DO(fg_gmov_loc2constind(self, fg_vtopdloc(self), (void const **)&mod->mo_globalv[gid], 0)); /* - */
				fg_vtop_direct_clearref(self);                    /* ..., ref:old_value, value */
#ifndef CONFIG_NO_THREADS
				DO(fg_grwlock_endwrite_const(self, &mod->mo_lock)); /* - */
#endif /* !CONFIG_NO_THREADS */
				DO(fg_vpop(self));                                /* ..., ref:old_value */

				/* Do post-processing, now that the stack looks like: a, b, c, old_value */
				switch (prefix_opcode) {
				case ASM_RROT:   /* rrot #<imm8>+2, PREFIX */
					DO(fg_vrrot(self, prefix_instr[1] + 2));
					break;
				case ASM16_RROT: /* rrot #<imm16>+2, PREFIX */
					DO(fg_vrrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 2));
					break;
				default: break;
				}
			}	break;

			default: {
				int temp;
				DO(fg_vpush_prefix(self, instr, prefix_type, id1, id2));
				switch (prefix_opcode) {
				case ASM_SWAP:   /* swap top, PREFIX */
					temp = fg_vswap(self);
					break;
				case ASM_LROT:   /* lrot #<imm8>+2, PREFIX */
					temp = fg_vlrot(self, prefix_instr[1] + 3);
					break;
				case ASM_RROT:   /* rrot #<imm8>+2, PREFIX */
					temp = fg_vrrot(self, prefix_instr[1] + 3);
					break;
				case ASM16_LROT: /* lrot #<imm16>+2, PREFIX */
					temp = fg_vlrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 3);
					break;
				case ASM16_RROT: /* rrot #<imm16>+2, PREFIX */
					temp = fg_vrrot(self, UNALIGNED_GETLE16(prefix_instr + 2) + 3);
					break;
				default: __builtin_unreachable();
				}
				if unlikely(temp)
					goto err;
				return fg_vpop_prefix(self, prefix_type, id1, id2);
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
			Dee_operator_t opname;
			DREF DeeObject *const_operand;
			unsigned int vop_flags = VOP_F_NORMAL;
			DO(fg_vpush_prefix_noalias(self, instr, prefix_type, id1, id2));
			/* Prefix supports direct addressing (and the `DREF DeeObject **' was already pushed) */
			switch (prefix_opcode) {
			case ASM_ADD_SIMM8: /* add PREFIX, $<Simm8> */
			case ASM_SUB_SIMM8: /* sub PREFIX, $<Simm8> */
			case ASM_MUL_SIMM8: /* mul PREFIX, $<Simm8> */
			case ASM_DIV_SIMM8: /* div PREFIX, $<Simm8> */
			case ASM_MOD_SIMM8: /* mod PREFIX, $<Simm8> */
				const_operand = DeeInt_NewInt8((int8_t)prefix_instr[1]);
do_push_const_operand:
				if unlikely(!const_operand)
					goto err;
				const_operand = fg_inlineref(self, const_operand);
				if unlikely(!const_operand)
					goto err;
				DO(fg_vpush_const(self, const_operand)); /* ref:this, value */
				vop_flags |= VOP_F_ALLOWNATIVE;
				break;
			case ASM_SHL_IMM8:  /* shl PREFIX, $<Simm8> */
			case ASM_SHR_IMM8:  /* shr PREFIX, $<Simm8> */
				const_operand = DeeInt_NewUInt8((uint8_t)prefix_instr[1]);
				goto do_push_const_operand;
			case ASM_ADD_IMM32: /* add PREFIX, $<imm32> */
			case ASM_SUB_IMM32: /* sub PREFIX, $<imm32> */
			case ASM_AND_IMM32: /* and PREFIX, $<imm32> */
			case ASM_OR_IMM32:  /* or  PREFIX, $<imm32> */
			case ASM_XOR_IMM32: /* xor PREFIX, $<imm32> */
				const_operand = DeeInt_NewUInt32((uint32_t)UNALIGNED_GETLE32(prefix_instr + 1));
				goto do_push_const_operand;
			default:
				DO(fg_vswap(self)); /* ref:this, value */
				break;
			}
			opname = ASM_MATHBLOCK_OPCODE2IOPERATOR(prefix_opcode); /* ref:this, value */
			DO(fg_vinplaceop(self, opname, 1, vop_flags));          /* ref:this */
			return fg_vpop_prefix(self, prefix_type, id1, id2);     /* N/A */
		}	break;

		TARGET(ASM_INCPOST)   /* push inc PREFIX' - `PREFIX: push inc */
		TARGET(ASM_DECPOST) { /* push dec PREFIX' - `PREFIX: push dec */
			Dee_operator_t opname = opcode == ASM_INCPOST ? OPERATOR_INC : OPERATOR_DEC;
			DO(fg_vpush_prefix_noalias(self, instr, prefix_type, id1, id2)); /* ref:this */
			DO(fg_vdup(self));                                  /* ref:this, this */
			DO(fg_vop(self, OPERATOR_COPY, 1, VOP_F_PUSHRES));  /* ref:this, ref:copy */
			DO(fg_vswap(self));                                 /* ref:copy, ref:this */
			DO(fg_vinplaceop(self, opname, 0, VOP_F_NORMAL));   /* ref:copy, ref:this */
			return fg_vpop_prefix(self, prefix_type, id1, id2); /* ref:copy */
		}	break;

		TARGET(ASM_DELOP)
		TARGET(ASM16_DELOP)
		TARGET(ASM_NOP)   /* nop PREFIX */
		TARGET(ASM16_NOP) /* nop16 PREFIX' - `PREFIX: nop16 */
			break;

		TARGET(ASM_OPERATOR) { /* PREFIX: push op $<imm8>, #<imm8> */
			Dee_operator_t opname;
			uint8_t argc;
			opname = prefix_instr[1];
			argc   = prefix_instr[2];
			__IF0 {
		TARGET(ASM16_OPERATOR) /* PREFIX: push op $<imm16>, #<imm8> */
				opname = UNALIGNED_GETLE16(prefix_instr + 2);
				argc   = prefix_instr[4];
			}                                                                /* [args...] */
			DO(fg_vpush_prefix_noalias(self, instr, prefix_type, id1, id2)); /* [args...], ref:this */
			DO(fg_vrrot(self, argc + 1));                                    /* ref:this, [args...] */
			DO(fg_vinplaceop(self, opname, argc, VOP_F_PUSHRES));            /* ref:this, result */
			DO(fg_vswap(self));                                              /* result, ref:this */
			return fg_vpop_prefix(self, prefix_type, id1, id2);              /* result */
		}	break;

		TARGET(ASM_OPERATOR_TUPLE) { /* PREFIX: push op $<imm8>, pop... */
			Dee_operator_t opname;
			opname = prefix_instr[1]; /* PREFIX: push op $<imm16>, pop... */
			__IF0 {
		TARGET(ASM16_OPERATOR_TUPLE)
				opname = UNALIGNED_GETLE16(prefix_instr + 2);
			}                                                                /* args */
			DO(fg_vpush_prefix_noalias(self, instr, prefix_type, id1, id2)); /* args, ref:this */
			DO(fg_vswap(self));                                              /* ref:this, args */
			DO(fg_vinplaceoptuple(self, opname, VOP_F_PUSHRES));             /* ref:this, result */
			DO(fg_vswap(self));                                              /* result, ref:this */
			return fg_vpop_prefix(self, prefix_type, id1, id2);              /* result */
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


/* Wrapper around `fg_geninstr()' to generate the entire basic block.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_genall(struct fungen *__restrict self) {
	struct basic_block *block = self->fg_block;
	Dee_instruction_t const *instr;
	ASSERT(block->bb_mem_start != NULL);
	ASSERT(block->bb_mem_end == NULL);

	/* Generate code to delete all locals that are currently bound,
	 * but whose values don't matter as per `self->bb_locuse' */
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOEARLYDEL)) {
		struct memstate *state = self->fg_state;
		lid_t lid;
		for (lid = 0; lid < state->ms_localc; ++lid) {
			if (bitset_test(block->bb_locuse, lid))
				continue;
			if (memval_isdirect(&state->ms_localv[lid]) &&
			    memval_direct_local_neverbound(&state->ms_localv[lid]))
				continue;
			if (memstate_isshared(state)) {
				DO(fg_state_unshare(self));
				state = self->fg_state;
			}
			DO(fg_vdel_local(self, lid));
		}
	}

	/* Generate text. */
	block->bb_deemon_end = block->bb_deemon_end_r;
	block->bb_next       = block->bb_next_r;
	self->fg_nextlastloc = block->bb_locreadv;
	for (instr = block->bb_deemon_start; instr < block->bb_deemon_end;) {
		Dee_instruction_t const *next_instr = DeeAsm_NextInstr(instr);

		/* TODO: If this function throws an exception that isn't an interrupt or BadAlloc,
		 *       re-wrap that exception as an IllegalInstruction, whilst also appending
		 *       DDI debug information about the origin of `instr' */
		DO(fg_geninstr(self, instr, &next_instr));

		ASSERT(block->bb_mem_end == NULL ||
		       block->bb_mem_end == (DREF struct memstate *)-1);
		if (block->bb_mem_end == (DREF struct memstate *)-1) {
			/* Special indicator meaning that this block needs to be re-compiled
			 * because its starting memory state had to be constrained. This can
			 * happen (e.g.) in code like this:
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
	memstate_incref(self->fg_state);
	if (block->bb_next != NULL) {
		/* Constrain the starting-state of the fallthru-block with the ending memory-state of `block' */
		struct basic_block *next_block = block->bb_next;
		Dee_code_addr_t addr = function_assembler_addrof(self->fg_assembler,
		                                                     next_block->bb_deemon_start);
		int error = basic_block_constrainwith(next_block, block->bb_mem_end, addr);
		if unlikely(error < 0)
			goto err_mem_end;
		if (error > 0)
			return 0;
	}

	return 0;
err_mem_end:
	memstate_decref(block->bb_mem_end);
	block->bb_mem_end = NULL;
err:
	ASSERT(block->bb_mem_end == NULL);
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_DEEMON_C */
