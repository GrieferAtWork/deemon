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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_ARCH_C
#define GUARD_DEX_HOSTASM_GENERATOR_ARCH_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/list.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include <hybrid/sequence/bitset.h>
#include <hybrid/unaligned.h>

#ifdef HOSTASM_X86
#include "libgen86/gen.h"
#include "libgen86/register.h"
#endif /* HOSTASM_X86 */

DECL_BEGIN

#define Dee_host_section_reqx86(self, n_instructions) \
	Dee_host_section_reqhost(self, GEN86_INSTRLEN_MAX * (n_instructions))

PRIVATE uint8_t const gen86_registers[HOST_REGISTER_COUNT] = {
#ifdef HOSTASM_X86_64_MSABI
	GEN86_R_RAX,
	GEN86_R_RCX,
	GEN86_R_RDX,
	GEN86_R_R8,
	GEN86_R_R9,
	GEN86_R_R10,
	GEN86_R_R11,
#elif defined(HOSTASM_X86_64_SYSVABI)
	GEN86_R_RAX,
	GEN86_R_RCX,
	GEN86_R_RDX,
	GEN86_R_RDI,
	GEN86_R_RSI,
	GEN86_R_R8,
	GEN86_R_R9,
	GEN86_R_R10,
	GEN86_R_R11,
#elif defined(HOSTASM_X86)
	GEN86_R_EAX,
	GEN86_R_ECX,
	GEN86_R_EDX,
#endif /* ... */
};

#define p_pc(sect)  (&(sect)->hs_end)
#define p_off(sect) (uintptr_t)((sect)->hs_end - (sect)->hs_start)


#ifdef NO_HOSTASM_DEBUG_PRINT
#define NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
#define gen86_regname(regno) ""
#define gen86_printf(...)    (void)0
#else /* NO_HOSTASM_DEBUG_PRINT */
#define NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
#define gen86_regname(regno) gen86_regnames[gen86_registers[regno]]
#if 0
#define Per "P"
#define Plq "P"
#elif defined(HOSTASM_X86_64)
#define Per "r"
#define Plq "q"
#else /* HOSTASM_X86_64 */
#define Per "e"
#define Plq "l"
#endif /* !HOSTASM_X86_64 */
PRIVATE char const gen86_regnames[][5] = {
	/* [GEN86_R_PAX] = */ "%" Per "ax",
	/* [GEN86_R_PCX] = */ "%" Per "cx",
	/* [GEN86_R_PDX] = */ "%" Per "dx",
	/* [GEN86_R_PBX] = */ "%" Per "bx",
	/* [GEN86_R_PSP] = */ "%" Per "sp",
	/* [GEN86_R_PBP] = */ "%" Per "bp",
	/* [GEN86_R_PSI] = */ "%" Per "si",
	/* [GEN86_R_PDI] = */ "%" Per "di",
#ifdef HOSTASM_X86_64
	/* [GEN86_R_R8]  = */ "%r8",
	/* [GEN86_R_R9]  = */ "%r9",
	/* [GEN86_R_R10] = */ "%r10",
	/* [GEN86_R_R11] = */ "%r11",
	/* [GEN86_R_R12] = */ "%r12",
	/* [GEN86_R_R13] = */ "%r13",
	/* [GEN86_R_R14] = */ "%r14",
	/* [GEN86_R_R15] = */ "%r15",
#endif /* HOSTASM_X86_64 */
};
PRIVATE char const gen86_ccnames[][3] = {
	/* [GEN86_CC_O]  = */ "o",
	/* [GEN86_CC_NO] = */ "no",
	/* [GEN86_CC_B]  = */ "b",
	/* [GEN86_CC_AE] = */ "ae",
	/* [GEN86_CC_Z]  = */ "z",
	/* [GEN86_CC_NZ] = */ "nz",
	/* [GEN86_CC_BE] = */ "be",
	/* [GEN86_CC_A]  = */ "a",
	/* [GEN86_CC_S]  = */ "s",
	/* [GEN86_CC_NS] = */ "ns",
	/* [GEN86_CC_PE] = */ "pe",
	/* [GEN86_CC_PO] = */ "po",
	/* [GEN86_CC_L]  = */ "l",
	/* [GEN86_CC_GE] = */ "ge",
	/* [GEN86_CC_LE] = */ "le",
	/* [GEN86_CC_G]  = */ "g",
};
#define gen86_printf(...) Dee_DPRINTF("gen86:" __VA_ARGS__)
PRIVATE ATTR_RETNONNULL WUNUSED char const *DCALL
gen86_addrname(void const *addr) {
	static char buf[2 + sizeof(void *) * 2 + 1];
#ifdef HOSTASM_X86_64
#define CASE(func, n)                \
	if (addr == (void const *)&func) \
		return #func
#else /* HOSTASM_X86_64 */
#define CASE(func, n)                \
	if (addr == (void const *)&func) \
		return #func "@" #n
#endif /* !HOSTASM_X86_64 */
	CASE(DeeObject_Copy, 4);
	CASE(DeeObject_DeepCopy, 4);
	CASE(DeeObject_Assign, 8);
	CASE(DeeObject_MoveAssign, 8);
	CASE(DeeObject_Str, 4);
	CASE(DeeObject_Repr, 4);
	CASE(DeeObject_Bool, 4);
	CASE(DeeObject_IterNext, 4);
	CASE(DeeObject_Call, 12);
	CASE(DeeObject_CallKw, 16);
	CASE(DeeObject_CallTuple, 8);
	CASE(DeeObject_CallTupleKw, 12);
	CASE(DeeObject_CallAttr, 16);
	CASE(DeeObject_CallAttrKw, 20);
	CASE(DeeObject_CallAttrTuple, 12);
	CASE(DeeObject_CallAttrTupleKw, 16);
	CASE(DeeObject_Int, 4);
	CASE(DeeObject_Inv, 4);
	CASE(DeeObject_Pos, 4);
	CASE(DeeObject_Neg, 4);
	CASE(DeeObject_Add, 8);
	CASE(DeeObject_Sub, 8);
	CASE(DeeObject_Mul, 8);
	CASE(DeeObject_Div, 8);
	CASE(DeeObject_Mod, 8);
	CASE(DeeObject_Shl, 8);
	CASE(DeeObject_Shr, 8);
	CASE(DeeObject_And, 8);
	CASE(DeeObject_Or, 8);
	CASE(DeeObject_Xor, 8);
	CASE(DeeObject_Pow, 8);
	CASE(DeeObject_AddInt8, 8);
	CASE(DeeObject_SubInt8, 8);
	CASE(DeeObject_MulInt8, 8);
	CASE(DeeObject_DivInt8, 8);
	CASE(DeeObject_ModInt8, 8);
	CASE(DeeObject_ShlUInt8, 8);
	CASE(DeeObject_ShrUInt8, 8);
	CASE(DeeObject_AddUInt32, 8);
	CASE(DeeObject_SubUInt32, 8);
	CASE(DeeObject_AndUInt32, 8);
	CASE(DeeObject_OrUInt32, 8);
	CASE(DeeObject_XorUInt32, 8);
	CASE(DeeObject_Inc, 4);
	CASE(DeeObject_Dec, 4);
	CASE(DeeObject_InplaceAdd, 8);
	CASE(DeeObject_InplaceSub, 8);
	CASE(DeeObject_InplaceMul, 8);
	CASE(DeeObject_InplaceDiv, 8);
	CASE(DeeObject_InplaceMod, 8);
	CASE(DeeObject_InplaceShl, 8);
	CASE(DeeObject_InplaceShr, 8);
	CASE(DeeObject_InplaceAnd, 8);
	CASE(DeeObject_InplaceOr, 8);
	CASE(DeeObject_InplaceXor, 8);
	CASE(DeeObject_InplacePow, 8);
	CASE(DeeObject_Hash, 4);
	CASE(DeeObject_CompareEqObject, 8);
	CASE(DeeObject_CompareNeObject, 8);
	CASE(DeeObject_CompareLoObject, 8);
	CASE(DeeObject_CompareLeObject, 8);
	CASE(DeeObject_CompareGrObject, 8);
	CASE(DeeObject_CompareGeObject, 8);
	CASE(DeeObject_IterSelf, 4);
	CASE(DeeObject_SizeObject, 4);
	CASE(DeeObject_Contains, 8);
	CASE(DeeObject_GetItem, 8);
	CASE(DeeObject_DelItem, 8);
	CASE(DeeObject_SetItem, 12);
	CASE(DeeObject_GetRange, 12);
	CASE(DeeObject_GetRangeBeginIndex, 12);
	CASE(DeeObject_GetRangeEndIndex, 12);
	CASE(DeeObject_GetRangeIndex, 12);
	CASE(DeeObject_DelRange, 12);
	CASE(DeeObject_SetRange, 16);
	CASE(DeeObject_SetRangeBeginIndex, 16);
	CASE(DeeObject_SetRangeEndIndex, 16);
	CASE(DeeObject_SetRangeIndex, 16);
	CASE(DeeObject_GetAttr, 8);
	CASE(DeeObject_DelAttr, 8);
	CASE(DeeObject_SetAttr, 12);
	CASE(DeeObject_Enter, 4);
	CASE(DeeObject_Leave, 4);
	CASE(DeeError_Throw, 4);
	CASE(DeeObject_ConcatInherited, 8);
	CASE(DeeObject_ExtendInherited, 8);
	CASE(DeeTuple_FromSequence, 4);
	CASE(DeeList_FromSequence, 4);
	CASE(DeeSuper_Of, 4);
	CASE(DeeRange_NewInt, 4);
#undef CASE
	*Dee_sprintf(buf, "%#Ix", addr) = '\0';
	return buf;
}

PRIVATE ATTR_RETNONNULL WUNUSED char const *DCALL
gen86_symname(struct Dee_host_symbol const *sym) {
	static char buf[sizeof("[sect:+0x]") + 2 * (sizeof(void *) * 2)];
	switch (sym->hs_type) {
	case DEE_HOST_SYMBOL_ABS:
		return gen86_addrname(sym->hs_value.sv_abs);
	case DEE_HOST_SYMBOL_JUMP:
		*Dee_sprintf(buf, "[sect:%p]", &sym->hs_value.sv_jump->jd_to->bb_htext) = '\0';
		return buf;
	case DEE_HOST_SYMBOL_SECT:
		*Dee_sprintf(buf, "[sect:%p+%#Ix]", sym->hs_value.sv_sect.ss_sect, sym->hs_value.sv_sect.ss_off) = '\0';
		return buf;
	default: break;
	}
	return "[...]";
}

PRIVATE NONNULL((1)) void DCALL
_Dee_memloc_debug_print(struct Dee_memloc *__restrict self, bool is_local) {
	if (!(self->ml_flags & MEMLOC_F_NOREF))
		Dee_DPRINT("r");
	switch (self->ml_type) {
	case MEMLOC_TYPE_HSTACK:
		Dee_DPRINTF("#%Iu", self->ml_value.v_hstack.s_cfa);
		break;
	case MEMLOC_TYPE_HSTACKIND:
		Dee_DPRINTF("[#%Iu]", self->ml_value.v_hstack.s_cfa);
		if (self->ml_value.v_hstack.s_off != 0)
			Dee_DPRINTF("+%Id", self->ml_value.v_hstack.s_off);
		break;
	case MEMLOC_TYPE_HREG:
	case MEMLOC_TYPE_HREGIND:
		if (self->ml_type == MEMLOC_TYPE_HREGIND)
			Dee_DPRINT("[");
		Dee_DPRINT(gen86_regname(self->ml_value.v_hreg.r_regno));
		if (self->ml_value.v_hreg.r_off != 0)
			Dee_DPRINTF("+%Id", self->ml_value.v_hreg.r_off);
		if (self->ml_type == MEMLOC_TYPE_HREGIND) {
			Dee_DPRINT("]");
			if (self->ml_value.v_hreg.r_voff != 0)
				Dee_DPRINTF("+%Id", self->ml_value.v_hreg.r_voff);
		}
		break;
	case MEMLOC_TYPE_CONST:
		Dee_DPRINTF("$%p", self->ml_value.v_const);
		break;
	case MEMLOC_TYPE_UNALLOC:
		if (is_local) {
			Dee_DPRINT("-");
			break;
		}
		ATTR_FALLTHROUGH
	default:
		Dee_DPRINTF("{%I16u:%p,%p}", self->ml_type,
		            self->ml_value._v_data[0],
		            self->ml_value._v_data[1]);
		break;
	}
	if (is_local) {
		switch (self->ml_flags & MEMLOC_M_LOCAL_BSTATE) {
		case MEMLOC_F_LOCAL_BOUND:
			Dee_DPRINT("!");
			break;
		case MEMLOC_F_LOCAL_UNBOUND:
			Dee_DPRINT("?");
			break;
		default:
			break;
		}
	}
}

INTERN NONNULL((1)) void DCALL
_Dee_memstate_debug_print(struct Dee_memstate *__restrict self) {
	Dee_DPRINTF("\tCFA:   #%Iu\n", self->ms_host_cfa_offset);
	if (self->ms_stackc > 0) {
		uint16_t i;
		Dee_DPRINT("\tstack: ");
		for (i = 0; i < self->ms_stackc; ++i) {
			if (i != 0)
				Dee_DPRINT(", ");
			_Dee_memloc_debug_print(&self->ms_stackv[i], false);
		}
		Dee_DPRINT("\n");
	}
	if (self->ms_localc > 0) {
		size_t i;
		bool is_first = true;
		for (i = 0; i < self->ms_localc; ++i) {
			if (self->ms_localv[i].ml_type == MEMLOC_TYPE_UNALLOC &&
			    self->ms_localv[i].ml_flags == (MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND))
				continue;
			Dee_DPRINT(is_first ? "\tlocal: " : ", ");
			Dee_DPRINTF("%I16u=", i);
			_Dee_memloc_debug_print(&self->ms_localv[i], true);
			is_first = false;
		}
		if (!is_first)
			Dee_DPRINT("\n");
	}
}
#endif /* !NO_HOSTASM_DEBUG_PRINT */

#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
#define IF_VERBOSE_REFCNT_LOGGING(x) /* nothing */
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#define IF_VERBOSE_REFCNT_LOGGING(x) x
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */



/* Object reference count incref/decref */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gincref_regx(struct Dee_host_section *__restrict self,
                               Dee_host_register_t regno, ptrdiff_t reg_offset) {
	if unlikely(Dee_host_section_reqx86(self, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
	if (reg_offset == 0) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("lock inc" Plq "\tob_refcnt(%s)\n", gen86_regname(regno)));
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("lock inc" Plq "\tob_refcnt+%Id(%s)\n", reg_offset, gen86_regname(regno)));
	}
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf("incref\t%s\n", gen86_regname(regno));
	} else {
		gen86_printf("incref\t%s+%Id\n", gen86_regname(regno), reg_offset);
	}
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(self));
	gen86_incP_mod(p_pc(self), gen86_modrm_db,
	               reg_offset + (ptrdiff_t)offsetof(DeeObject, ob_refcnt),
	               gen86_registers[regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gdecref_regx_nokill(struct Dee_host_section *__restrict self,
                                      Dee_host_register_t regno, ptrdiff_t reg_offset) {
	if unlikely(Dee_host_section_reqx86(self, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
	if (reg_offset == 0) {
		gen86_printf("lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
	} else {
		gen86_printf("lock dec" Plq "\tob_refcnt+%Id(%s)\n", reg_offset, gen86_regname(regno));
	}
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf("decref_nokill\t%s\n", gen86_regname(regno));
	} else {
		gen86_printf("decref_nokill\t%s+%Id\n", gen86_regname(regno), reg_offset);
	}
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(self));
	gen86_decP_mod(p_pc(self), gen86_modrm_db,
	               reg_offset + (ptrdiff_t)offsetof(DeeObject, ob_refcnt),
	               gen86_registers[regno]);
	return 0;
err:
	return -1;
}

#if !defined(CONFIG_NO_BADREFCNT_CHECKS) || defined(CONFIG_TRACE_REFCHANGES)
DFUNDEF NONNULL((1)) void (DCALL DeeObject_Destroy)(DeeObject *__restrict self);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS || CONFIG_TRACE_REFCHANGES */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_gdestroy_regx(struct Dee_function_generator *__restrict self,
                                      struct Dee_host_section *sect,
                                      Dee_host_register_t regno, ptrdiff_t reg_offset) {
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t save_regno;

	/* Save registers. */
	for (save_regno = 0; save_regno < HOST_REGISTER_COUNT; ++save_regno) {
		if (state->ms_rinuse[save_regno] == 0)
			continue;
		if (save_regno == regno)
			continue;
		if unlikely(Dee_host_section_reqx86(sect, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("push" Plq "\t%s\n", gen86_regname(save_regno)));
		gen86_pushP_r(p_pc(sect), gen86_registers[save_regno]);
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
	}

	/* Load `regno' as argument for the call to `DeeObject_Destroy()' */
#ifdef HOST_REGISTER_R_ARG0
	if (regno != HOST_REGISTER_R_ARG0) {
		if unlikely(Dee_host_section_reqx86(sect, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(
		gen86_printf("lea" Plq "\t%Id(%s), %s\n",
		             reg_offset, gen86_regname(regno),
		             gen86_regname(HOST_REGISTER_R_ARG0)));
		gen86_leaP_db_r(p_pc(sect),
		                reg_offset, gen86_registers[regno],
		                gen86_registers[HOST_REGISTER_R_ARG0]);
	}
#else /* HOST_REGISTER_R_ARG0 */
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	if (reg_offset == 0) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("push" Plq "\t%s\n", gen86_regname(regno)));
		gen86_pushP_r(p_pc(sect), gen86_registers[regno]);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("lea" Plq "\t%Id(%s), %s\n", reg_offset,
		                                        gen86_regname(regno), gen86_regname(regno)));
		gen86_leaP_db_r(p_pc(sect), reg_offset, gen86_registers[regno], gen86_registers[regno]);
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("push" Plq "\t%s\n", gen86_regname(regno)));
		gen86_pushP_r(p_pc(sect), gen86_registers[regno]);
	}
	Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
#endif /* !HOST_REGISTER_R_ARG0 */

	/* Make the call to `DeeObject_Destroy()' */
	{
		struct Dee_host_reloc *rel;
#ifdef HOSTASM_X86_64_MSABI
		if unlikely(Dee_host_section_reqx86(sect, 3))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("sub" Plq "\t$32, %%" Per "sp\n"));
		gen86_subP_imm_r(p_pc(sect), 32, GEN86_R_PSP);
#else /* HOSTASM_X86_64_MSABI */
		if unlikely(Dee_host_section_reqx86(sect, 1))
			goto err;
#endif /* !HOSTASM_X86_64_MSABI */
		rel = Dee_host_section_newhostrel(sect);
		if unlikely(!rel)
			goto err;
#ifdef HOSTASM_X86_64
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("calll\tDeeObject_Destroy\n"));
#else /* HOSTASM_X86_64 */
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("calll\tDeeObject_Destroy@4\n"));
#endif /* !HOSTASM_X86_64 */
		gen86_calll_offset(p_pc(sect), -4);
		rel->hr_offset = p_off(sect) - 4;
		rel->hr_rtype  = DEE_HOST_RELOC_REL32;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_ABS;
		rel->hr_value.rv_abs = (void *)&DeeObject_Destroy;
#ifdef HOSTASM_X86_64_MSABI
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("add" Plq "\t$32, %%" Per "sp\n"));
		gen86_addP_imm_r(p_pc(sect), 32, GEN86_R_PSP);
#endif /* !HOSTASM_X86_64_MSABI */
	}

	/* Account for the fact that `DeeObject_Destroy()' pops its own arguments on i386 */
#ifndef HOST_REGISTER_R_ARG0
	Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
#endif /* !HOST_REGISTER_R_ARG0 */

	/* After calling an external function, usage registers may have gotten clobbered. */
	Dee_memstate_hregs_clear_usage(self->fg_state);

	/* Restore registers (in reverse order). */
	save_regno = HOST_REGISTER_COUNT;
	while (save_regno) {
		--save_regno;
		if (state->ms_rinuse[save_regno] == 0)
			continue;
		if (save_regno == regno)
			continue;
		if unlikely(Dee_host_section_reqx86(sect, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("pop" Plq "\t%s\n", gen86_regname(save_regno)));
		gen86_popP_r(p_pc(sect), gen86_registers[save_regno]);
		Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
	}

	return 0;
err:
	return -1;
}

#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
PRIVATE NONNULL((1)) void DCALL
log_compact_decref_register_preserve_list(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t regno) {
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t save_regno;
	bool is_first = true;

	/* Save registers. */
	for (save_regno = 0; save_regno < HOST_REGISTER_COUNT; ++save_regno) {
		if (state->ms_rinuse[save_regno] == 0)
			continue;
		if (save_regno == regno)
			continue;
		Dee_DPRINT(", ");
		if (is_first) {
			Dee_DPRINT("[");
			is_first = false;
		}
		Dee_DPRINT(gen86_regname(save_regno));
	}
	if (!is_first)
		Dee_DPRINT("]");
}
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */


INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gdecref_regx(struct Dee_function_generator *__restrict self,
                                     Dee_host_register_t regno, ptrdiff_t reg_offset) {
	struct Dee_host_section *sect = self->fg_sect;
	if unlikely(Dee_host_section_reqx86(sect, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
	if (reg_offset == 0) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno)));
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("lock dec" Plq "\tob_refcnt+%Id(%s)\n", reg_offset, gen86_regname(regno)));
	}
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf("decref\t%s", gen86_regname(regno));
	} else {
		gen86_printf("decref\t%s+%Id", gen86_regname(regno), reg_offset);
	}
	log_compact_decref_register_preserve_list(self, regno);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(sect));
	gen86_decP_mod(p_pc(sect), gen86_modrm_db,
	               reg_offset + (ptrdiff_t)offsetof(DeeObject, ob_refcnt),
	               gen86_registers[regno]);

	/* NOTE: decP sets FLAGS.Z=1 when the reference counter becomes `0'. */
	if ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE) ||
	    (sect != &self->fg_block->bb_htext)) {
		uintptr_t jcc8_offset;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jnz8\t1f\n"));
		gen86_jcc8_offset(p_pc(sect), GEN86_CC_NZ, -1);
		jcc8_offset = p_off(sect) - 1;
		if unlikely(_Dee_function_generator_gdestroy_regx(self, sect, regno, reg_offset))
			goto err;
		/* Fill in the delta for the non-zero-jump above. */
		*(int8_t *)(sect->hs_start + jcc8_offset) += (int8_t)(p_off(sect) - jcc8_offset);
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
	} else {
		struct Dee_host_section *cold = &self->fg_block->bb_hcold;
		struct Dee_host_reloc *enter_rel;
		struct Dee_host_reloc *leave_rel;
		int32_t delta;

		/* Generate code that jumps into the cold section when the reference counter became `0'. */
		delta = -4 + p_off(cold);
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jzl\t1f\n"));
		gen86_jccl_offset(p_pc(sect), GEN86_CC_Z, delta);
		enter_rel = Dee_host_section_newhostrel(sect);
		if unlikely(!enter_rel)
			goto err;
		enter_rel->hr_offset = p_off(sect);
		enter_rel->hr_rtype  = DEE_HOST_RELOC_REL32;
		enter_rel->hr_vtype  = DEE_HOST_RELOCVALUE_SECT;
		enter_rel->hr_value.rv_sect = cold;

		/* Generate the destroy-code *within* the cold section. */
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(".section .cold\n"));
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
		if unlikely(_Dee_function_generator_gdestroy_regx(self, cold, regno, reg_offset))
			goto err;

		/* Generate code to jump from the cold section back to the normal section. */
		if unlikely(Dee_host_section_reqx86(cold, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jmpl\t1f\n"));
		delta = -4 + p_off(sect);
		gen86_jmpl_offset(p_pc(cold), delta);
		leave_rel = Dee_host_section_newhostrel(cold);
		if unlikely(!leave_rel)
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(".section .text\n"));
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
		leave_rel->hr_offset = p_off(cold);
		enter_rel->hr_rtype  = DEE_HOST_RELOC_REL32;
		enter_rel->hr_vtype  = DEE_HOST_RELOCVALUE_SECT;
		enter_rel->hr_value.rv_sect = sect;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gincref_const(struct Dee_host_section *__restrict self,
                                DeeObject *value) {
	if unlikely(Dee_host_section_reqx86(self, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	gen86_printf("incref\t$%#Ix\n", value);
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("lock inc" Plq "\t%#Ix\n", (intptr_t)(uintptr_t)&value->ob_refcnt));
	gen86_lock(p_pc(self));
	gen86_incP_mod(p_pc(self), gen86_modrm_d, (intptr_t)(uintptr_t)&value->ob_refcnt); /* TODO: movabs */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gdecref_const(struct Dee_host_section *__restrict self,
                                DeeObject *value) {
	if unlikely(Dee_host_section_reqx86(self, 2))
		goto err;
	/* Constants can never be destroyed, so decref'ing one is
	 * like `Dee_DecrefNoKill()' (iow: doesn't need a zero-check) */
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	gen86_printf("decref_nokill\t$%#Ix\n", value);
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("lock dec" Plq "\t%#Ix\n", (intptr_t)(uintptr_t)&value->ob_refcnt));
	gen86_lock(p_pc(self));
	gen86_decP_mod(p_pc(self), gen86_modrm_d, (intptr_t)(uintptr_t)&value->ob_refcnt); /* TODO: movabs */
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gxincref_reg(struct Dee_host_section *__restrict self,
                               Dee_host_register_t regno, ptrdiff_t reg_offset) {
	uint8_t reg86;
	uintptr_t jcc8_offset;
	if unlikely(Dee_host_section_reqx86(self, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf("xincref\t%s\n", gen86_regname(regno));
	} else {
		gen86_printf("xincref\t%s+%Id\n", gen86_regname(regno), reg_offset);
	}
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	reg86 = gen86_registers[regno];
	if (reg_offset == 0) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(self), reg86, reg86);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("cmp" Plq "\t$%Id, %s\n", -reg_offset, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(self), -reg_offset, reg86);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jz8\t1f\n"));
	gen86_jcc8_offset(p_pc(self), GEN86_CC_Z, -1);
	jcc8_offset = p_off(self) - 1;
	if unlikely(_Dee_host_section_gincref_regx(self, regno, reg_offset))
		goto err;
	/* Fix-up the jump */
	*(int8_t *)(self->hs_start + jcc8_offset) += (int8_t)(p_off(self) - jcc8_offset);
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gxdecref_regx(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t regno, ptrdiff_t reg_offset) {
	struct Dee_host_section *sect = self->fg_sect;
	uint8_t reg86;
	uintptr_t jcc8_offset;
	if unlikely(Dee_host_section_reqx86(sect, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf("xdecref\t%s", gen86_regname(regno));
	} else {
		gen86_printf("xdecref\t%s+%Id", gen86_regname(regno), reg_offset);
	}
	log_compact_decref_register_preserve_list(self, regno);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	reg86 = gen86_registers[regno];
	if (reg_offset == 0) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(sect), reg86, reg86);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("cmp" Plq "\t$%Id, %s\n", -reg_offset, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(sect), -reg_offset, reg86);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jz8\t1f\n"));
	gen86_jcc8_offset(p_pc(sect), GEN86_CC_Z, -1);
	jcc8_offset = p_off(sect) - 1;
	if unlikely(_Dee_function_generator_gdecref_regx(self, regno, reg_offset))
		goto err;
	/* Fix-up the jump */
	*(int8_t *)(sect->hs_start + jcc8_offset) += (int8_t)(p_off(sect) - jcc8_offset);
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
	return 0;
err:
	return -1;
}



/* Controls for operating with R/W-locks (as needed for accessing global/extern variables) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_read(struct Dee_function_generator *__restrict self,
                                     Dee_atomic_rwlock_t *__restrict lock) {
	/* >> #if OSIZE
	 * >> .Lfull_retry:
	 * >>     movP  <&lock->arw_lock>, %Pax
	 * >> .LPax_retry:
	 * >>     testP $ATOMIC_RWLOCK_WFLAG, %Pax
	 * >>     jnz8  .Lfull_retry
	 * >>     leaP  1(%Pax), %temp
	 * >>     lock  cmpxchgP %temp, <&lock->arw_lock>
	 * >>     jnz8  .LPax_retry
	 * >> #else // OSIZE
	 * >> .Ldo_retry:
	 * >>     movP  <&lock->arw_lock>, %Pax
	 * >>     testP $ATOMIC_RWLOCK_WFLAG, %Pax
	 * >>     jnzl  .Lretry
	 * >>     leaP  1(%Pax), %temp
	 * >>     lock  cmpxchgP %temp, <&lock->arw_lock>
	 * >>     jnzl  .Lretry
	 * >> .section .cold
	 * >> .Lretry:
	 * >> #if CONFIG_HAVE_sched_yield
	 * >>     <PUSH_USED_REGS_EXCEPT(%Pax, %temp)>
	 * >>     call  sched_yield
	 * >>     <POP_USED_REGS(%Pax, %temp)>
	 * >> #else // CONFIG_HAVE_sched_yield
	 * >>     pause
	 * >> #endif // !CONFIG_HAVE_sched_yield
	 * >>     jmpl  .Ldo_retry
	 * >> .section .text
	 * >> #endif // !OSIZE */
	/* TODO */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_write(struct Dee_function_generator *__restrict self,
                                      Dee_atomic_rwlock_t *__restrict lock) {
	/* >> .Lretry:
	/* >>     xorP  %Pax, %Pax
	 * >>     movP  $ATOMIC_RWLOCK_WFLAG, %temp
	 * >>     lock  cmpxchgP %temp, <&lock->arw_lock>
	 * >> #if OSIZE
	 * >>     jnz8  .Lretry
	 * >> #else // OSIZE
	 * >>     jnzl  .Lyield_and_retry
	 * >> .section .cold
	 * >> .Lyield_and_retry:
	 * >> #if CONFIG_HAVE_sched_yield
	 * >>     <PUSH_USED_REGS_EXCEPT(%Pax, %temp)>
	 * >>     call  sched_yield
	 * >>     <POP_USED_REGS(%Pax, %temp)>
	 * >> #else // CONFIG_HAVE_sched_yield
	 * >>     pause
	 * >> #endif // !CONFIG_HAVE_sched_yield
	 * >>     jmpl  .Lretry
	 * >> .section .text
	 * >> #endif // !OSIZE */
	/* TODO */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endread(struct Dee_function_generator *__restrict self,
                                        Dee_atomic_rwlock_t *__restrict lock) {
	/* >> lock decP <&lock->arw_lock> */
	/* TODO */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endwrite(struct Dee_function_generator *__restrict self,
                                         Dee_atomic_rwlock_t *__restrict lock) {
	/* >> movP  $0, <&lock->arw_lock> */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}


/* Allocate/deallocate memory from the host stack.
 * If stack memory gets allocated, zero-initialize it. */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_adjust(struct Dee_host_section *__restrict self,
                                 ptrdiff_t alloc_delta) {
	if (alloc_delta < 0) {
		/* Release stack memory. */
		ASSERT(IS_ALIGNED((uintptr_t)(-alloc_delta), HOST_SIZEOF_POINTER));
		if unlikely(Dee_host_section_reqx86(self, 1))
			goto err;
		gen86_printf("add" Plq "\t$%Id, %%" Per "sp\n", -alloc_delta);
		gen86_addP_imm_r(p_pc(self), -alloc_delta, GEN86_R_PSP);
	} else if (alloc_delta > 0) {
		/* Acquire stack memory. */
		size_t n_pointers = (uintptr_t)alloc_delta / HOST_SIZEOF_POINTER;
		ASSERT(IS_ALIGNED((uintptr_t)alloc_delta, HOST_SIZEOF_POINTER));
		ASSERT(n_pointers >= 1);
		/* Generate a bunch of `pushP $0' instructions. */
		if unlikely(Dee_host_section_reqx86(self, n_pointers))
			goto err;
		do {
			gen86_printf("push" Plq "\t$0\n");
			gen86_pushP_imm(p_pc(self), 0);
		} while (--n_pointers);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_pushreg(struct Dee_host_section *__restrict self,
                                  Dee_host_register_t src_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("push" Plq "\t%s\n", gen86_regname(src_regno));
	gen86_pushP_r(p_pc(self), gen86_registers[src_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_pushregind(struct Dee_host_section *__restrict self,
                                     Dee_host_register_t src_regno, ptrdiff_t src_delta) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("push" Plq "\t%Id(%s)\n", src_delta, gen86_regname(src_regno));
	gen86_pushP_mod(p_pc(self), gen86_modrm_db, src_delta, gen86_registers[src_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_pushconst(struct Dee_host_section *__restrict self,
                                    DeeObject *value) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("push" Plq "\t$%#Ix\n", (intptr_t)(uintptr_t)value);
	gen86_pushP_imm(p_pc(self), (intptr_t)(uintptr_t)value); /* TODO: movabs */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_pushhstackind(struct Dee_host_section *__restrict self,
                                     ptrdiff_t sp_offset) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("push" Plq "\t%Id(%%" Per "sp)\n", sp_offset);
	gen86_pushP_mod(p_pc(self), gen86_modrm_db, sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_popreg(struct Dee_host_section *__restrict self,
                                 Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("pop" Plq "\t%s\n", gen86_regname(dst_regno));
	gen86_popP_r(p_pc(self), gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_reg2hstackind(struct Dee_host_section *__restrict self,
                                     Dee_host_register_t src_regno, ptrdiff_t sp_offset) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t%s, %Id(%%" Per "sp)\n", gen86_regname(src_regno), sp_offset);
	gen86_movP_r_db(p_pc(self), gen86_registers[src_regno], sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_hstack2reg(struct Dee_host_section *__restrict self,
                                  ptrdiff_t sp_offset, Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("lea" Plq "\t%Id(%%" Per "sp), %s\n", sp_offset, gen86_regname(dst_regno));
	gen86_leaP_db_r(p_pc(self), sp_offset, GEN86_R_PSP, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_hstackind2reg(struct Dee_host_section *__restrict self,
                                  ptrdiff_t sp_offset, Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t%Id(%%" Per "sp), %s\n", sp_offset, gen86_regname(dst_regno));
	gen86_movP_db_r(p_pc(self), sp_offset, GEN86_R_PSP, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_const2reg(struct Dee_host_section *__restrict self,
                                 DeeObject *value, Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	if (value == NULL) {
		gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	} else {
		gen86_printf("mov" Plq "\t$%#Ix, %s\n", (intptr_t)(uintptr_t)value, gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(self), (intptr_t)(uintptr_t)value, gen86_registers[dst_regno]); /* TODO: movabs */
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_const2regind(struct Dee_host_section *__restrict self,
                                    DeeObject *value, Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t$%#Ix, %Id(%s)\n", (intptr_t)(uintptr_t)value, dst_delta, gen86_regname(dst_regno));
	gen86_movP_imm_db(p_pc(self), (intptr_t)(uintptr_t)value, dst_delta, gen86_registers[dst_regno]); /* TODO: movabs */
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_regx2reg(struct Dee_host_section *__restrict self,
                                Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                Dee_host_register_t dst_regno) {
	if unlikely(src_regno == dst_regno && src_delta == 0)
		return 0;
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	if (src_regno == dst_regno) {
		switch (src_delta) {
		case -1:
			gen86_printf("dec" Plq "\t%s\n", gen86_regname(dst_regno));
			gen86_decP_r(p_pc(self), gen86_registers[dst_regno]);
			return 0;
		case 1:
			gen86_printf("inc" Plq "\t%s\n", gen86_regname(dst_regno));
			gen86_incP_r(p_pc(self), gen86_registers[dst_regno]);
			return 0;
		default:
			break;
		}
	}
	gen86_printf("lea" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
	gen86_leaP_db_r(p_pc(self), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_constind2reg(struct Dee_host_section *__restrict self,
                                    DeeObject **p_value, Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t%#Ix, %s\n", (intptr_t)(uintptr_t)p_value, gen86_regname(dst_regno));
	gen86_movP_d_r(p_pc(self), (intptr_t)(uintptr_t)p_value, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_reg2constind(struct Dee_host_section *__restrict self,
                                    Dee_host_register_t src_regno, DeeObject **p_value) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t%s, %#Ix\n", gen86_regname(src_regno), (intptr_t)(uintptr_t)p_value);
	gen86_movP_r_d(p_pc(self), gen86_registers[src_regno], (intptr_t)(uintptr_t)p_value);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_regind2reg(struct Dee_host_section *__restrict self,
                                  Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                  Dee_host_register_t dst_regno) {
	if unlikely(src_regno == dst_regno)
		return 0;
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
	gen86_movP_db_r(p_pc(self), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_reg2regind(struct Dee_host_section *__restrict self,
                                  Dee_host_register_t src_regno,
                                  Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	if unlikely(src_regno == dst_regno)
		return 0;
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t%s, %Id(%s)\n", gen86_regname(src_regno), dst_delta, gen86_regname(dst_regno));
	gen86_movP_r_db(p_pc(self), gen86_registers[src_regno], dst_delta, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmov_truearg2reg(struct Dee_function_generator *__restrict self,
                                         size_t argno, Dee_host_register_t dst_regno) {
	size_t sp_offset;
	sp_offset = self->fg_state->ms_host_cfa_offset; /* Offset to return-pc */
#if defined(HOSTASM_X86_64) && defined(HOSTASM_X86_64_SYSVABI)
	/* This matches the way registers are saved in the prolog. */
	{
		Dee_hostfunc_cc_t cc = self->fg_assembler->fa_cc;
		size_t trueargc = cc & HOSTFUNC_CC_F_TUPLE ? 1 : 2;
		if (cc & HOSTFUNC_CC_F_THIS)
			++trueargc;
		if (cc & HOSTFUNC_CC_F_KW)
			++trueargc;
		sp_offset -= trueargc * HOST_SIZEOF_POINTER; /* Base address of saved register arguments */
		sp_offset += argno * HOST_SIZEOF_POINTER;    /* Load the relevant argument */
	}
#else /* HOSTASM_X86_64 */
	sp_offset += HOST_SIZEOF_POINTER;               /* Skip over return-pc */
	sp_offset += argno * HOST_SIZEOF_POINTER;       /* Load the relevant argument */
#endif /* !HOSTASM_X86_64 */
	return _Dee_function_generator_gmov_hstackind2reg(self, sp_offset, dst_regno);
}

/* Load special runtime values into `dst_regno' */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmov_usage2reg(struct Dee_function_generator *__restrict self,
                                       Dee_host_regusage_t usage,
                                       Dee_host_register_t dst_regno) {
	Dee_hostfunc_cc_t cc = self->fg_assembler->fa_cc;
	switch (usage) {
	case DEE_HOST_REGUSAGE_THIS:
		if (!(cc & HOSTFUNC_CC_F_THIS))
			break;
		return _Dee_function_generator_gmov_truearg2reg(self, 0, dst_regno);

	case DEE_HOST_REGUSAGE_ARGC: {
		size_t offset = 0;
		if (cc & HOSTFUNC_CC_F_TUPLE)
			break;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	case DEE_HOST_REGUSAGE_ARGV: {
		size_t offset = 1;
		if (cc & HOSTFUNC_CC_F_TUPLE)
			break;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	case DEE_HOST_REGUSAGE_ARGS: {
		size_t offset = 0;
		if (!(cc & HOSTFUNC_CC_F_TUPLE))
			break;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	case DEE_HOST_REGUSAGE_KW: {
		size_t offset = 1;
		if (!(cc & HOSTFUNC_CC_F_KW))
			break;
		if (!(cc & HOSTFUNC_CC_F_TUPLE))
			++offset;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	default: break;
	}
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Unsupported register usage %" PRFu8,
	                       (uint8_t)usage);
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gret(struct Dee_function_generator *__restrict self) {
	struct Dee_host_section *sect = self->fg_sect;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
#ifdef HOSTASM_X86_64
	gen86_printf("ret" Plq "\n");
	gen86_retP(p_pc(sect));
#else /* HOSTASM_X86_64 */
	/* Special handling needed because DCALL is STDCALL on i386 */
	{
		Dee_hostfunc_cc_t cc = self->fg_assembler->fa_cc;
		size_t ret_imm = cc & HOSTFUNC_CC_F_TUPLE ? 4 : 8;
		if (cc & HOSTFUNC_CC_F_THIS)
			ret_imm += 4;
		if (cc & HOSTFUNC_CC_F_KW)
			ret_imm += 4;
		gen86_printf("ret" Plq "\t$%Iu\n", ret_imm);
		gen86_retP_imm(p_pc(sect), ret_imm);
	}
#endif /* !HOSTASM_X86_64 */
	return 0;
err:
	return -1;
}



/* Generate a call to a C-function `api_function' with `argc'
 * pointer-sized arguments whose values are taken from `argv'.
 * NOTE: The given `api_function' is assumed to use the `DCALL' calling convention. */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gcallapi(struct Dee_function_generator *__restrict self,
                                 void const *api_function, size_t argc,
                                 struct Dee_memloc *argv) {
#ifdef HOSTASM_X86_64
	/* TODO */
	(void)self;
	(void)api_function;
	(void)argc;
	(void)argv;
	return DeeError_NOTIMPLEMENTED();
#else /* HOSTASM_X86_64 */
	struct Dee_host_section *sect = self->fg_sect;
	uintptr_t hstackaddr_regs[HOST_REGISTER_COUNT];
	size_t argi;
	bzero(hstackaddr_regs, sizeof(hstackaddr_regs));

	/* Push arguments onto the host stack in reverse order. */
	argi = argc;
	while (argi) {
		struct Dee_memloc *arg;
		--argi;
		arg = &argv[argi];
		switch (arg->ml_type) {

		case MEMLOC_TYPE_HSTACKIND:
		case MEMLOC_TYPE_HREGIND: {
			ptrdiff_t val_offset;
			val_offset = arg->ml_value.v_hstack.s_off;
			if (val_offset == 0) {
				if (arg->ml_type == MEMLOC_TYPE_HSTACKIND) {
					if unlikely(Dee_function_generator_ghstack_pushhstackind(self,
					                                                         arg->ml_value.v_hstack.s_cfa))
						goto err;
				} else {
					if unlikely(Dee_function_generator_ghstack_pushregind(self,
					                                                      arg->ml_value.v_hreg.r_regno,
					                                                      arg->ml_value.v_hreg.r_off))
						goto err;
				}
			} else {
				ptrdiff_t ind_offset;
				Dee_host_register_t temp_regno;
				uint8_t gen86_src_regno;
				BITSET(HOST_REGISTER_COUNT) preserve_regs;
				size_t future_argi;
				if (arg->ml_type == MEMLOC_TYPE_HSTACKIND) {
					ind_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, arg->ml_value.v_hstack.s_cfa);
					gen86_src_regno = GEN86_R_PSP;
				} else {
					ind_offset = arg->ml_value.v_hreg.r_off;
					gen86_src_regno = gen86_registers[arg->ml_value.v_hreg.r_regno];
				}
				bzero(&preserve_regs, sizeof(preserve_regs));
				for (future_argi = 0; future_argi < argi; ++future_argi) {
					struct Dee_memloc *future_arg = &argv[future_argi];
					if (MEMLOC_TYPE_HASREG(future_arg->ml_type))
						BITSET_TURNON(&preserve_regs, future_arg->ml_value.v_hreg.r_regno);
				}
				for (temp_regno = 0; temp_regno < HOST_REGISTER_COUNT; ++temp_regno) {
					if (BITSET_GET(&preserve_regs, temp_regno))
						continue;
					if (hstackaddr_regs[temp_regno] != 0)
						continue;
do_push_hind_offset_with_temp_regno:
					if unlikely(Dee_host_section_reqx86(sect, 3))
						goto err;
					gen86_printf("mov" Plq "\t%Id(%s), %s\n", ind_offset, gen86_regnames[gen86_src_regno], gen86_regname(temp_regno));
					gen86_movP_db_r(p_pc(sect), ind_offset, gen86_src_regno, gen86_registers[temp_regno]);
					gen86_printf("lea" Plq "\t%Id(%s), %s\n", val_offset, gen86_regname(temp_regno), gen86_regname(temp_regno));
					gen86_leaP_db_r(p_pc(sect), val_offset, gen86_registers[temp_regno], gen86_registers[temp_regno]);
					gen86_printf("push" Plq "\t%s\n", gen86_regname(temp_regno));
					gen86_pushP_r(p_pc(sect), gen86_registers[temp_regno]);
					Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
					goto done_push_argi;
				}
				for (temp_regno = 0; temp_regno < HOST_REGISTER_COUNT; ++temp_regno) {
					if (BITSET_GET(&preserve_regs, temp_regno))
						continue;
					hstackaddr_regs[temp_regno] = 0;
					goto do_push_hind_offset_with_temp_regno;
				}
				if unlikely(Dee_function_generator_ghstack_pushhstackind(self, arg->ml_value.v_hstack.s_cfa))
					goto err;
				gen86_printf("add" Plq "\t$%Id, (%%" Per "sp)\n", val_offset);
				gen86_addP_imm_mod(p_pc(sect), gen86_modrm_b, val_offset, GEN86_R_PSP);
			}
		}	break;

		case MEMLOC_TYPE_HSTACK: {
			Dee_host_register_t addr_regno;
			uintptr_t cfa_offset = arg->ml_value.v_hstack.s_cfa;
			size_t future_argi;
			ptrdiff_t sp_offset;
			BITSET(HOST_REGISTER_COUNT) preserve_regs;
			ASSERT(cfa_offset > 0);
			ASSERT(cfa_offset <= self->fg_state->ms_host_cfa_offset);
			if (cfa_offset == self->fg_state->ms_host_cfa_offset) {
				/* Special case: can use `pushP %Psp', which pushes the current %Psp value (as it was before the push) */
				if unlikely(Dee_host_section_reqx86(sect, 1))
					goto err;
				gen86_printf("push" Plq "\t%%" Per "sp\n");
				gen86_pushP_r(p_pc(sect), GEN86_R_PSP);
				Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
				break;
			}

			/* Check if we already have a cache register for the same stack address. */
			for (addr_regno = 0; addr_regno < HOST_REGISTER_COUNT; ++addr_regno) {
				if (hstackaddr_regs[addr_regno] == cfa_offset) {
					if unlikely(Dee_function_generator_ghstack_pushreg(self, addr_regno))
						goto err;
					goto done_push_argi;
				}
			}

			/* Check if there is a temp register we can use. */
			bzero(&preserve_regs, sizeof(preserve_regs));
			for (future_argi = 0; future_argi < argi; ++future_argi) {
				struct Dee_memloc *future_arg = &argv[future_argi];
				if (MEMLOC_TYPE_HASREG(future_arg->ml_type))
					BITSET_TURNON(&preserve_regs, future_arg->ml_value.v_hreg.r_regno);
			}
			sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
			for (addr_regno = 0; addr_regno < HOST_REGISTER_COUNT; ++addr_regno) {
				if (!BITSET_GET(&preserve_regs, addr_regno)) {
					if unlikely(_Dee_host_section_gmov_hstack2reg(sect, sp_offset, addr_regno))
						goto err;
					hstackaddr_regs[cfa_offset] = addr_regno;
					if unlikely(Dee_function_generator_ghstack_pushreg(self, addr_regno))
						goto err;
					goto done_push_argi;
				}
			}

			/* All GP registers need to remain preserved -> load address in 2 steps:
			 * >> pushP %Psp
			 * >> addP  $..., 0(%Psp) */
			if unlikely(Dee_host_section_reqx86(sect, 1))
				goto err;
			gen86_printf("push" Plq "\t%%" Per "sp\n");
			gen86_pushP_r(p_pc(sect), GEN86_R_PSP);
			Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
			gen86_printf("add" Plq "\t$%Id, (%%" Per "sp)\n");
			gen86_addP_imm_mod(p_pc(sect), gen86_modrm_b, sp_offset, GEN86_R_PSP);
		}	break;

		case MEMLOC_TYPE_HREG:
			if (arg->ml_value.v_hreg.r_off != 0) {
				size_t future_argi;
				if unlikely(_Dee_function_generator_gmov_regx2reg(self,
				                                                  arg->ml_value.v_hreg.r_regno,
				                                                  arg->ml_value.v_hreg.r_off,
				                                                  arg->ml_value.v_hreg.r_regno))
					goto err;
				/* Fix-up offset for the same register used by arguments not yet pushed */
				for (future_argi = 0; future_argi < argi; ++future_argi) {
					struct Dee_memloc *future_arg = &argv[future_argi];
					if (MEMLOC_TYPE_HASREG(future_arg->ml_type) &&
					    future_arg->ml_value.v_hreg.r_regno == arg->ml_value.v_hreg.r_regno)
						future_arg->ml_value.v_hreg.r_off -= arg->ml_value.v_hreg.r_off;
				}
			}
			if unlikely(Dee_function_generator_ghstack_pushreg(self, arg->ml_value.v_hreg.r_regno))
				goto err;
			break;

		case MEMLOC_TYPE_CONST:
			if unlikely(Dee_function_generator_ghstack_pushconst(self, arg->ml_value.v_const))
				goto err;
			break;

		default:
			return DeeError_Throwf(&DeeError_IllegalInstruction,
			                       "Cannot push memory location with type %#" PRFx16,
			                       arg->ml_type);
			break;
		}
done_push_argi:
		;
	}

	/* With everything pushed onto the stack and in the correct position, generate the call.
	 * Note that because deemon API functions use DCALL (STDCALL), the called function does
	 * the stack cleanup. */
	{
		struct Dee_host_reloc *rel;
		if unlikely(Dee_host_section_reqx86(sect, 1))
			goto err;
		rel = Dee_host_section_newhostrel(sect);
		if unlikely(!rel)
			goto err;
		gen86_printf("calll\t%s\n", gen86_addrname(api_function));
		gen86_calll_offset(p_pc(sect), -4);
		rel->hr_offset = p_off(sect) - 4;
		rel->hr_rtype  = DEE_HOST_RELOC_REL32;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_ABS;
		rel->hr_value.rv_abs = api_function;
	}

	/* Adjust the CFA offset to account for the called function having cleaned up stack arguments. */
	Dee_function_generator_gadjust_cfa_offset(self, -(ptrdiff_t)(argc * HOST_SIZEOF_POINTER));

	/* After calling an external function, usage registers may have gotten clobbered. */
	Dee_memstate_hregs_clear_usage(self->fg_state);
	return 0;
err:
	return -1;
#endif /* !HOSTASM_X86_64 */
}




PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gtest_and_jcc(struct Dee_function_generator *__restrict self,
                                      struct Dee_memloc *test_loc,
                                      struct Dee_host_symbol *__restrict dst, uint8_t cc) {
	struct Dee_host_section *sect = self->fg_sect;
	struct Dee_host_reloc *rel;
	if unlikely(Dee_host_section_reqx86(sect, 2))
		goto err;
	switch (test_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, test_loc->ml_value.v_hstack.s_cfa);
		ptrdiff_t val_offset = test_loc->ml_value.v_hstack.s_off;
		gen86_printf("cmp" Plq "\t$%Id, %Id(%%" Per "sp)\n", -val_offset, sp_offset);
		gen86_cmpP_imm_mod(p_pc(sect), gen86_modrm_db, -val_offset, sp_offset, GEN86_R_PSP);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		ptrdiff_t off = test_loc->ml_value.v_hreg.r_off;
		ptrdiff_t val_offset = test_loc->ml_value.v_hreg.r_voff;
		gen86_printf("cmp" Plq "\t$%Id, %Id(%s)\n", -val_offset, off, gen86_regname(test_loc->ml_value.v_hreg.r_regno));
		gen86_cmpP_imm_mod(p_pc(sect), gen86_modrm_db, -val_offset, off, gen86_registers[test_loc->ml_value.v_hreg.r_regno]);
	}	break;

	case MEMLOC_TYPE_HREG: {
		uint8_t reg86 = gen86_registers[test_loc->ml_value.v_hreg.r_regno];
		ptrdiff_t off = test_loc->ml_value.v_hreg.r_off;
		if (off == 0) {
			gen86_printf("test" Plq "\t%s, %s\n",
			             gen86_regname(test_loc->ml_value.v_hreg.r_regno),
			             gen86_regname(test_loc->ml_value.v_hreg.r_regno));
			gen86_testP_r_r(p_pc(sect), reg86, reg86);
		} else {
			/* This cmp sets FLAGS based on the result of `%r_regno - $off' */
			gen86_printf("cmp" Plq "\t$%Id, %s\n", off, gen86_regname(test_loc->ml_value.v_hreg.r_regno));
			gen86_cmpP_imm_r(p_pc(sect), off, reg86);
		}
	}	break;

	case MEMLOC_TYPE_HSTACK:
		/* Stack addresses are always non-NULL */
		if (cc == GEN86_CC_Z)
			return 0; /* Don't jump */
		goto always_jump;
	case MEMLOC_TYPE_CONST:
		if ((test_loc->ml_value.v_const == NULL) != (cc == GEN86_CC_Z))
			return 0; /* Don't jump */
always_jump:
		return _Dee_host_section_gjmp(sect, dst);
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot test memory location with type %#" PRFx16,
		                       test_loc->ml_type);
	}
	gen86_printf("j%sl\t%s\n", gen86_ccnames[cc], gen86_symname(dst));
	gen86_jccl_offset(p_pc(sect), cc, -4);
	rel = Dee_host_section_newhostrel(sect);
	if unlikely(!rel)
		goto err;
	rel->hr_offset = p_off(sect) - 4;
	rel->hr_rtype  = DEE_HOST_RELOC_REL32;
	rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
	rel->hr_value.rv_sym = dst;
	return 0;
err:
	return -1;
}

/* Generate jumps. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gjz(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *test_loc, struct Dee_host_symbol *__restrict dst) {
	return _Dee_function_generator_gtest_and_jcc(self, test_loc, dst, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gjnz(struct Dee_function_generator *__restrict self,
                             struct Dee_memloc *test_loc, struct Dee_host_symbol *__restrict dst) {
	return _Dee_function_generator_gtest_and_jcc(self, test_loc, dst, GEN86_CC_NZ);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_host_section_gjmp(struct Dee_host_section *__restrict self,
                       struct Dee_host_symbol *__restrict dst) {
	struct Dee_host_reloc *rel;
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("jmpl\t%s\n", gen86_symname(dst));
	gen86_jmpl_offset(p_pc(self), -4);
	rel = Dee_host_section_newhostrel(self);
	if unlikely(!rel)
		goto err;
	rel->hr_offset = p_off(self) - 4;
	rel->hr_rtype  = DEE_HOST_RELOC_REL32;
	rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
	rel->hr_value.rv_sym = dst;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_host_section_gjcc(struct Dee_host_section *__restrict self,
                       struct Dee_host_symbol *__restrict dst, uint8_t cc) {
	struct Dee_host_reloc *rel;
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("j%sl\t%s\n", gen86_ccnames[cc], gen86_symname(dst));
	gen86_jccl_offset(p_pc(self), cc, -4);
	rel = Dee_host_section_newhostrel(self);
	if unlikely(!rel)
		goto err;
	rel->hr_offset = p_off(self) - 4;
	rel->hr_rtype  = DEE_HOST_RELOC_REL32;
	rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
	rel->hr_value.rv_sym = dst;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
Dee_memstate_adjust_register_delta(struct Dee_memstate *__restrict self,
                                   Dee_host_register_t regno, ptrdiff_t delta) {
	size_t i;
	for (i = 0; i < self->ms_stackc; ++i) {
		struct Dee_memloc *loc = &self->ms_stackv[i];
		if (MEMLOC_TYPE_HASREG(loc->ml_type) &&
		    loc->ml_value.v_hreg.r_regno == regno)
			loc->ml_value.v_hreg.r_off += delta;
	}
	for (i = 0; i < self->ms_localc; ++i) {
		struct Dee_memloc *loc = &self->ms_localv[i];
		if (MEMLOC_TYPE_HASREG(loc->ml_type) &&
		    loc->ml_value.v_hreg.r_regno == regno)
			loc->ml_value.v_hreg.r_off += delta;
	}
}

/* Emit conditional jump(s) based on `<lhs> <=> <rhs>'
 * NOTE: This function may clobber `lhs' and `rhs', and may flush/shift local/stack locations. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gjcmp(struct Dee_function_generator *__restrict self,
                              struct Dee_memloc *lhs, struct Dee_memloc *rhs, bool signed_cmp,
                              struct Dee_host_symbol *dst_lo,   /* Jump here if `<lhs> < <rhs>' */
                              struct Dee_host_symbol *dst_eq,   /* Jump here if `<lhs> == <rhs>' */
                              struct Dee_host_symbol *dst_gr) { /* Jump here if `<lhs> > <rhs>' */
	struct Dee_host_section *sect = self->fg_sect;
	if (Dee_memloc_sameloc(lhs, rhs) || (dst_eq == dst_lo && dst_eq == dst_gr))
		return dst_eq ? _Dee_host_section_gjmp(sect, dst_eq) : 0;
	if (lhs->ml_type == MEMLOC_TYPE_CONST && rhs->ml_type != MEMLOC_TYPE_CONST) {
		/* We always want constants in `rhs', because that's how we can best encode `cmpP' */
swap_operands:
#define Tswap(T, a, b) do { T _temp = a; a = b; b = _temp; } __WHILE0
		Tswap(struct Dee_memloc *, lhs, rhs);
		Tswap(struct Dee_host_symbol *, dst_lo, dst_gr);
#undef Tswap
	}
	switch (rhs->ml_type) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(lhs->ml_type))
			not_these[0] = lhs->ml_value.v_hreg.r_regno;
		if unlikely(Dee_function_generator_greg(self, rhs, not_these))
			goto err;
	}	ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
	case MEMLOC_TYPE_HSTACK: {
		uint8_t rhs_gen86_basereg;
		ptrdiff_t rhs_basereg_off;
		if (rhs->ml_type == MEMLOC_TYPE_HREG) {
			rhs_basereg_off   = rhs->ml_value.v_hreg.r_off;
			rhs_gen86_basereg = gen86_registers[rhs->ml_value.v_hreg.r_regno];
		} else {
			uintptr_t cfa_offset = rhs->ml_value.v_hstack.s_cfa;
			rhs_basereg_off   = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
			rhs_gen86_basereg = GEN86_R_PSP;
		}
		switch (lhs->ml_type) {
		default:
			if unlikely(Dee_function_generator_greg(self, lhs, NULL))
				goto err;
			ATTR_FALLTHROUGH
		case MEMLOC_TYPE_HREG:
		case MEMLOC_TYPE_HSTACK: {
			uint8_t lhs_gen86_basereg;
			if (lhs->ml_type == MEMLOC_TYPE_HREG) {
				rhs_basereg_off -= lhs->ml_value.v_hreg.r_off;
				lhs_gen86_basereg = gen86_registers[lhs->ml_value.v_hreg.r_regno];
			} else {
				uintptr_t cfa_offset = lhs->ml_value.v_hstack.s_cfa;
				rhs_basereg_off -= Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
				lhs_gen86_basereg = GEN86_R_PSP;
			}
			if (lhs_gen86_basereg == lhs_gen86_basereg) {
				/* Same base register -> only the offset matters (which is already known) */
				struct Dee_host_symbol *dst;
				if (signed_cmp ? (0 < rhs_basereg_off) : (0 < (uintptr_t)rhs_basereg_off)) {
					dst = dst_lo;
				} else if (signed_cmp && 0 > rhs_basereg_off) {
					dst = dst_gr;
				} else {
					dst = dst_eq;
				}
				return dst ? _Dee_host_section_gjmp(sect, dst) : 0;
			}

			/* >> %lhs_gen86_basereg <==> %rhs_gen86_basereg + rhs_basereg_off */
			if (rhs_basereg_off != 0) {
				/* Inline `rhs_basereg_off' into a register and adjust local/stack locations. */
				if unlikely(Dee_host_section_reqx86(sect, 1))
					goto err;
				if (lhs_gen86_basereg != GEN86_R_PSP) {
					Dee_host_register_t lhs_regno = lhs->ml_value.v_hreg.r_regno;
					ASSERT(lhs_gen86_basereg != GEN86_R_PSP);
					gen86_printf("lea" Plq "\t%Id(%s), %s\n", -rhs_basereg_off,
					             gen86_regnames[lhs_gen86_basereg], gen86_regnames[lhs_gen86_basereg]);
					gen86_leaP_db_r(p_pc(sect), -rhs_basereg_off, lhs_gen86_basereg, lhs_gen86_basereg);
					Dee_memstate_adjust_register_delta(self->fg_state, lhs_regno, -rhs_basereg_off);
					self->fg_state->ms_rusage[lhs_regno] = DEE_HOST_REGUSAGE_GENERIC;
				} else {
					Dee_host_register_t rhs_regno = rhs->ml_value.v_hreg.r_regno;
					ASSERT(rhs_gen86_basereg != GEN86_R_PSP);
					gen86_printf("lea" Plq "\t%Id(%s), %s\n", rhs_basereg_off,
					             gen86_regnames[rhs_gen86_basereg], gen86_regnames[rhs_gen86_basereg]);
					gen86_leaP_db_r(p_pc(sect), rhs_basereg_off, rhs_gen86_basereg, rhs_gen86_basereg);
					Dee_memstate_adjust_register_delta(self->fg_state, rhs_regno, rhs_basereg_off);
					self->fg_state->ms_rusage[rhs_regno] = DEE_HOST_REGUSAGE_GENERIC;
				}
			}

			/* >> %lhs_gen86_basereg <==> %rhs_gen86_basereg */
			if unlikely(Dee_host_section_reqx86(sect, 1))
				goto err;
			gen86_printf("cmpP" Plq "\t%s, %s\n",
			             gen86_regnames[rhs_gen86_basereg],
			             gen86_regnames[lhs_gen86_basereg]);
			gen86_cmpP_r_r(p_pc(sect), rhs_gen86_basereg, lhs_gen86_basereg);
		}	break;

		case MEMLOC_TYPE_HREGIND:
		case MEMLOC_TYPE_HSTACKIND:
			/* Swap operands so we can re-use the rhs=*IND case below */
			goto swap_operands;
		}
	}	break;

	case MEMLOC_TYPE_HREGIND:
	case MEMLOC_TYPE_HSTACKIND: {
		uint8_t rhs_gen86_basereg;
		ptrdiff_t rhs_basereg_off;
		ptrdiff_t rhs_value_off;
		if (rhs->ml_type == MEMLOC_TYPE_HREGIND) {
			rhs_basereg_off   = rhs->ml_value.v_hreg.r_off;
			rhs_gen86_basereg = gen86_registers[rhs->ml_value.v_hreg.r_regno];
		} else {
			uintptr_t cfa_offset = rhs->ml_value.v_hstack.s_cfa;
			rhs_basereg_off   = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
			rhs_gen86_basereg = GEN86_R_PSP;
		}
		rhs_value_off = rhs->ml_value.v_hreg.r_voff;
		switch (lhs->ml_type) {
		default:
			if unlikely(Dee_function_generator_greg(self, lhs, NULL))
				goto err;
			ATTR_FALLTHROUGH
		case MEMLOC_TYPE_HREG:
		case MEMLOC_TYPE_HSTACK: {
			uint8_t lhs_gen86_basereg;
			ptrdiff_t lhs_basereg_off;
			if (lhs->ml_type == MEMLOC_TYPE_HREG) {
				lhs_basereg_off   = lhs->ml_value.v_hreg.r_off;
				lhs_gen86_basereg = gen86_registers[lhs->ml_value.v_hreg.r_regno];
			} else {
				uintptr_t cfa_offset = lhs->ml_value.v_hstack.s_cfa;
				lhs_basereg_off   = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
				lhs_gen86_basereg = GEN86_R_PSP;
			}
			rhs_value_off -= lhs_basereg_off;
			if unlikely(Dee_host_section_reqx86(sect, 1))
				goto err;

			/* >> %lhs_gen86_basereg <==> (*(%rhs_gen86_basereg + rhs_basereg_off) + rhs_value_off) */
			if (rhs_value_off != 0) {
				Dee_host_register_t temp_regno;
				if (lhs_gen86_basereg != GEN86_R_PSP) {
					temp_regno = lhs->ml_value.v_hreg.r_regno;
				} else {
					Dee_host_register_t not_these[2];
					not_these[0] = HOST_REGISTER_COUNT;
					not_these[1] = HOST_REGISTER_COUNT;
					if (MEMLOC_TYPE_HASREG(rhs->ml_type))
						not_these[0] = rhs->ml_value.v_hreg.r_regno;
					temp_regno = Dee_memstate_hregs_find_unused_ex(self->fg_state, not_these);
					if (temp_regno >= HOST_REGISTER_COUNT) {
						for (temp_regno = 0; temp_regno < HOST_REGISTER_COUNT; ++temp_regno) {
							if (self->fg_state->ms_rusage[temp_regno] == DEE_HOST_REGUSAGE_GENERIC)
								break;
						}
						if (temp_regno >= HOST_REGISTER_COUNT)
							temp_regno = 0;
						if (MEMLOC_TYPE_HASREG(rhs->ml_type) &&
						    rhs->ml_value.v_hreg.r_regno == temp_regno)
							++temp_regno;
					}
				}
				/* Inline `rhs_value_off' into `temp_regno' and adjust local/stack locations. */
				if unlikely(Dee_host_section_reqx86(sect, 1))
					goto err;
				gen86_printf("lea" Plq "\t%Id(%s), %s\n", -rhs_value_off,
				             gen86_regnames[lhs_gen86_basereg], gen86_regname(temp_regno));
				gen86_leaP_db_r(p_pc(sect), -rhs_value_off, lhs_gen86_basereg, gen86_registers[temp_regno]);
				Dee_memstate_adjust_register_delta(self->fg_state, temp_regno, -rhs_value_off);
				self->fg_state->ms_rusage[temp_regno] = DEE_HOST_REGUSAGE_GENERIC;
				lhs_gen86_basereg = gen86_registers[temp_regno];
			}

			/* >> %lhs_gen86_basereg <==> *(%rhs_gen86_basereg + rhs_basereg_off) */
			if unlikely(Dee_host_section_reqx86(sect, 1))
				goto err;
			/* NOTE: rhs-first because of AT&T operand order! */
			gen86_printf("cmpP" Plq "\t%Id(%s), %s\n", rhs_basereg_off,
			             gen86_regnames[rhs_gen86_basereg],
			             gen86_regnames[lhs_gen86_basereg]);
			gen86_cmpP_mod_r(p_pc(sect), gen86_modrm_db, rhs_basereg_off, rhs_gen86_basereg, lhs_gen86_basereg);
		}	break;
		}
	}

	case MEMLOC_TYPE_CONST: {
		uintptr_t rhs_value = (uintptr_t)rhs->ml_value.v_const;
		switch (lhs->ml_type) {
		default:
			if unlikely(Dee_function_generator_greg(self, lhs, NULL))
				goto err;
			ATTR_FALLTHROUGH
		case MEMLOC_TYPE_HREG:
		case MEMLOC_TYPE_HSTACK: {
			uint8_t lhs_gen86_basereg;
			ptrdiff_t lhs_basereg_off;
			if (lhs->ml_type == MEMLOC_TYPE_HREG) {
				lhs_basereg_off   = lhs->ml_value.v_hreg.r_off;
				lhs_gen86_basereg = gen86_registers[lhs->ml_value.v_hreg.r_regno];
			} else {
				uintptr_t cfa_offset = lhs->ml_value.v_hstack.s_cfa;
				lhs_basereg_off   = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
				lhs_gen86_basereg = GEN86_R_PSP;
			}
			rhs_value -= lhs_basereg_off;
			if unlikely(Dee_host_section_reqx86(sect, 1))
				goto err;
			if (rhs_value == 0 && (dst_lo == dst_gr)) {
				gen86_printf("test" Plq "\t%s, %s\n", gen86_regnames[lhs_gen86_basereg], gen86_regnames[lhs_gen86_basereg]);
				gen86_testP_r_r(p_pc(sect), lhs_gen86_basereg, lhs_gen86_basereg);
			} else {
				/* NOTE: rhs-first because of AT&T operand order! */
				gen86_printf("cmp" Plq "\t$%Id, %s\n", (intptr_t)rhs_value, gen86_regnames[lhs_gen86_basereg]);
				gen86_cmpP_imm_r(p_pc(sect), (intptr_t)rhs_value, lhs_gen86_basereg);
			}
		}	break;

		case MEMLOC_TYPE_HREGIND:
		case MEMLOC_TYPE_HSTACKIND: {
			uint8_t lhs_gen86_basereg;
			ptrdiff_t lhs_basereg_off;
			if (lhs->ml_type == MEMLOC_TYPE_HREGIND) {
				lhs_basereg_off   = lhs->ml_value.v_hreg.r_off;
				lhs_gen86_basereg = gen86_registers[lhs->ml_value.v_hreg.r_regno];
			} else {
				uintptr_t cfa_offset = lhs->ml_value.v_hstack.s_cfa;
				lhs_basereg_off   = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
				lhs_gen86_basereg = GEN86_R_PSP;
			}
			rhs_value -= lhs->ml_value.v_hreg.r_voff;
			if unlikely(Dee_host_section_reqx86(sect, 1))
				goto err;
			/* NOTE: rhs-first because of AT&T operand order! */
			gen86_printf("cmp" Plq "\t$%Id, %Id(%s)\n",
			             (intptr_t)rhs_value, lhs_basereg_off,
			             gen86_regnames[lhs_gen86_basereg]);
			gen86_cmpP_imm_mod(p_pc(sect), gen86_modrm_db, (intptr_t)rhs_value, lhs_basereg_off, lhs_gen86_basereg);
		}	break;

		case MEMLOC_TYPE_CONST: {
			int diff;
			uintptr_t lhs_value = (uintptr_t)lhs->ml_value.v_const;
			struct Dee_host_symbol *dst;
			if (signed_cmp) {
				intptr_t slhs_value = (intptr_t)lhs_value;
				intptr_t srhs_value = (intptr_t)rhs_value;
				diff = slhs_value < srhs_value ? -1 : slhs_value > srhs_value ? 1 : 0;
			} else {
				diff = lhs_value < rhs_value ? -1 : lhs_value > rhs_value ? 1 : 0;
			}
			if (diff < 0) {
				dst = dst_lo;
			} else if (diff > 0) {
				dst = dst_gr;
			} else {
				dst = dst_eq;
			}
			return dst ? _Dee_host_section_gjmp(sect, dst) : 0;
		}	break;
		}
	}	break;
	}

	/* At this point, the compare itself has been generated.
	 * Now to emit the actual branch instructions. */
	if (dst_lo) {
		uint8_t cc_lo = signed_cmp ? GEN86_CC_L : GEN86_CC_B;
		if (dst_eq == dst_lo) {
			dst_eq = NULL;
			cc_lo  = signed_cmp ? GEN86_CC_LE : GEN86_CC_BE;
		} else if (dst_gr == dst_lo) {
			dst_gr = NULL;
			cc_lo  = GEN86_CC_NE;
		}
		if unlikely(_Dee_host_section_gjcc(sect, dst_lo, cc_lo))
			goto err;
		if (cc_lo == GEN86_CC_NE)
			return dst_eq ? _Dee_host_section_gjmp(sect, dst_eq) : 0;
	}
	if (dst_gr) {
		uint8_t cc_gr = signed_cmp ? GEN86_CC_G : GEN86_CC_A;
		if (dst_eq == dst_gr) {
			dst_eq = NULL;
			cc_gr  = signed_cmp ? GEN86_CC_GE : GEN86_CC_AE;
		}
		if unlikely(_Dee_host_section_gjcc(sect, dst_gr, cc_gr))
			goto err;
	}
	if (dst_eq) {
		if unlikely((dst_lo && dst_gr) ? _Dee_host_section_gjmp(sect, dst_gr)
		                               : _Dee_host_section_gjcc(sect, dst_gr, GEN86_CC_E))
			goto err;
	}
	return 0;
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_ARCH_C */
