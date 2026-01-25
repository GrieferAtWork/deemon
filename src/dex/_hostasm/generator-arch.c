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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_ARCH_C
#define GUARD_DEX_HOSTASM_GENERATOR_ARCH_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_Free, Dee_*alloc*, Dee_Free */
#include <deemon/bool.h>            /* DeeBoolObject, Dee_FalseTrue */
#include <deemon/class.h>           /* DeeClass_*, DeeInstance_* */
#include <deemon/code.h>            /* DeeCodeObject, DeeCode_*, DeeFunction_Type, Dee_DDI_ISOK, Dee_DDI_STATE_*, Dee_code_addr_t, Dee_ddi_*, Dee_instruction_t */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/file.h>            /* DeeFile_* */
#include <deemon/format.h>          /* Dee_sprintf, PRFx16 */
#include <deemon/gc.h>              /* DeeGCObject_*alloc*, DeeGCObject_Free, DeeGC_Track, DeeGC_Untrack */
#include <deemon/list.h>            /* DeeList_FromSequence, DeeList_NewUninitialized */
#include <deemon/map.h>             /* DeeSharedMap_Decref, DeeSharedMap_NewShared */
#include <deemon/object.h>
#include <deemon/seq.h>             /* DeeRange_New, DeeRange_NewInt, DeeSeq_Unpack, DeeSharedVector_Decref, DeeSharedVector_NewShared */
#include <deemon/super.h>           /* DeeSuper_New, DeeSuper_Of */
#include <deemon/system-features.h> /* bzero, stdout */
#include <deemon/tuple.h>           /* DeeTuple* */
#include <deemon/util/lock.h>       /* Dee_atomic_rwlock_t */

#include <hybrid/align.h>         /* CEIL_ALIGN, IS_ALIGNED */
#include <hybrid/bitset.h>        /* BITSET_LENGTHOF, bitset_* */
#include <hybrid/compiler.h>
#include <hybrid/sched/__yield.h> /* __NO_hybrid_yield, __hybrid_yield, __hybrid_yield_IS_* */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* INT32_MAX, INT32_MIN, int8_t, int32_t, intptr_t, uintN_t, uintptr_t */

#ifdef HOSTASM_X86
#include "libgen86/gen.h"
#include "libgen86/register.h"
#endif /* HOSTASM_X86 */

DECL_BEGIN

#define OFFSETOF_ob_refcnt (ptrdiff_t)offsetof(DeeObject, ob_refcnt)

#ifdef HOSTASM_X86_64
#define HOST_REGNO_PAX HOST_REGNO_RAX
#define HOST_REGNO_PCX HOST_REGNO_RCX
#define HOST_REGNO_PDX HOST_REGNO_RDX
#else /* HOSTASM_X86_64 */
#define HOST_REGNO_PAX HOST_REGNO_EAX
#define HOST_REGNO_PCX HOST_REGNO_ECX
#define HOST_REGNO_PDX HOST_REGNO_EDX
#endif /* !HOSTASM_X86_64 */

#define gen86_printf(sect, ...) (HA_printf("gen86:" __VA_ARGS__))


#ifdef HOSTASM_X86_64
#define fit32(v) likely((intptr_t)(uintptr_t)(v) == (intptr_t)(int32_t)(intptr_t)(uintptr_t)(v))
#else /* HOSTASM_X86_64 */
#define fit32_IS_1
#define fit32(v) 1
#endif /* !HOSTASM_X86_64 */


#define host_section_reqx86(self, n_instructions) \
	host_section_reqhost(self, GEN86_INSTRLEN_MAX * (n_instructions))

#ifdef HOST_REGNO_R_ARG0
PRIVATE host_regno_t const host_arg_regs[] = {
	HOST_REGNO_R_ARG0,
#ifdef HOST_REGNO_R_ARG1
	HOST_REGNO_R_ARG1,
#ifdef HOST_REGNO_R_ARG2
	HOST_REGNO_R_ARG2,
#ifdef HOST_REGNO_R_ARG3
	HOST_REGNO_R_ARG3,
#ifdef HOST_REGNO_R_ARG4
	HOST_REGNO_R_ARG4,
#ifdef HOST_REGNO_R_ARG5
	HOST_REGNO_R_ARG5,
#ifdef HOST_REGNO_R_ARG6
#error "Too many argument registers"
#endif /* HOST_REGNO_R_ARG6 */
#endif /* HOST_REGNO_R_ARG5 */
#endif /* HOST_REGNO_R_ARG4 */
#endif /* HOST_REGNO_R_ARG3 */
#endif /* HOST_REGNO_R_ARG2 */
#endif /* HOST_REGNO_R_ARG1 */
};
#endif /* HOST_REGNO_R_ARG0 */

PRIVATE uint8_t const gen86_registers[HOST_REGNO_COUNT] = {
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


#ifdef HOSTASM_X86_64
#define gen86_format_symname(name, n) name
#else /* HOSTASM_X86_64 */
#define gen86_format_symname(name, n) name "@" #n
#endif /* !HOSTASM_X86_64 */


#ifdef NO_HOSTASM_DEBUG_PRINT
#define NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
#define NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
#define gen86_regname(regno) ""
#else /* NO_HOSTASM_DEBUG_PRINT */
#define NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
#define NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
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
PRIVATE ATTR_RETNONNULL WUNUSED char const *DCALL
gen86_addrname(void const *addr) {
	static char buf[2 + sizeof(void *) * 2 + 1];
#define CASE(func, n)                \
	if (addr == (void const *)&func) \
		return gen86_format_symname(#func, n)
	CASE(DeeObject_Copy, 4);
	CASE(DeeObject_CopyInherited, 4);
	CASE(DeeObject_DeepCopy, 4);
	CASE(DeeObject_DeepCopyInherited, 4);
	CASE(DeeObject_Assign, 8);
	CASE(DeeObject_MoveAssign, 8);
	CASE(DeeObject_Str, 4);
	CASE(DeeObject_StrInherited, 4);
	CASE(DeeObject_Repr, 4);
	CASE(DeeObject_ReprInherited, 4);
	CASE(DeeObject_Bool, 4);
	CASE(DeeObject_BoolInherited, 4);
	CASE(DeeObject_IterNext, 4);
	CASE(DeeObject_Call, 12);
	CASE(DeeObject_CallInherited, 12);
	CASE(DeeObject_CallKw, 16);
	CASE(DeeObject_CallKwInherited, 16);
	CASE(DeeObject_CallTuple, 8);
	CASE(DeeObject_CallTupleInherited, 8);
	CASE(DeeObject_CallTupleKw, 12);
	CASE(DeeObject_CallTupleKwInherited, 12);
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
	CASE(DeeObject_InplaceAddInt8, 8);
	CASE(DeeObject_InplaceSubInt8, 8);
	CASE(DeeObject_InplaceMulInt8, 8);
	CASE(DeeObject_InplaceDivInt8, 8);
	CASE(DeeObject_InplaceModInt8, 8);
	CASE(DeeObject_InplaceShlUInt8, 8);
	CASE(DeeObject_InplaceShrUInt8, 8);
	CASE(DeeObject_InplaceAddUInt32, 8);
	CASE(DeeObject_InplaceSubUInt32, 8);
	CASE(DeeObject_InplaceAndUInt32, 8);
	CASE(DeeObject_InplaceOrUInt32, 8);
	CASE(DeeObject_InplaceXorUInt32, 8);
	CASE(DeeObject_Hash, 4);
	CASE(DeeObject_HashInherited, 4);
	CASE(DeeObject_Compare, 8);
	CASE(DeeObject_CompareEq, 8);
	CASE(DeeObject_TryCompareEq, 8);
	CASE(DeeObject_CmpEqAsBool, 8);
	CASE(DeeObject_CmpNeAsBool, 8);
	CASE(DeeObject_CmpLoAsBool, 8);
	CASE(DeeObject_CmpLeAsBool, 8);
	CASE(DeeObject_CmpGrAsBool, 8);
	CASE(DeeObject_CmpGeAsBool, 8);
	CASE(DeeObject_CmpEq, 8);
	CASE(DeeObject_CmpNe, 8);
	CASE(DeeObject_CmpLo, 8);
	CASE(DeeObject_CmpLe, 8);
	CASE(DeeObject_CmpGr, 8);
	CASE(DeeObject_CmpGe, 8);
	CASE(DeeObject_Iter, 4);
	CASE(DeeObject_SizeOb, 4);
	CASE(DeeObject_ContainsAsBool, 8);
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
	CASE(DeeSeq_Unpack, 12);
	CASE(DeeObject_NewDefault, 4);
	CASE(DeeObject_Class, 4);
	CASE(DeeType_Extends, 8);
	CASE(DeeType_Implements, 8);
	CASE(DeeError_Throw, 4);
	CASE(DeeError_ThrowInherited, 4);
	CASE(DeeObject_ConcatInherited, 8);
	CASE(DeeObject_ExtendInherited, 12);
	CASE(DeeTuple_FromSequence, 4);
	CASE(DeeList_FromSequence, 4);
	CASE(DeeSuper_New, 8);
	CASE(DeeSuper_Of, 4);
	CASE(DeeRange_New, 12);
	CASE(DeeRange_NewInt, 12);
	CASE(DeeFile_GetStd, 8);
	CASE(DeeFile_PrintNl, 4);
	CASE(DeeFile_PrintObject, 8);
	CASE(DeeFile_PrintObjectSp, 8);
	CASE(DeeFile_PrintObjectNl, 8);
	CASE(DeeFile_PrintObjectRepr, 8);
	CASE(DeeFile_PrintObjectReprSp, 8);
	CASE(DeeFile_PrintObjectReprNl, 8);
	CASE(DeeFile_PrintAll, 8);
	CASE(DeeFile_PrintAllSp, 8);
	CASE(DeeFile_PrintAllNl, 8);
	CASE(DeeFile_WriteAll, 12);
	CASE(DeeFile_Putc, 8);
	CASE(DeeSharedVector_NewShared, 8);
	CASE(DeeSharedVector_Decref, 4);
	CASE(DeeSharedMap_NewShared, 8);
	CASE(DeeSharedMap_Decref, 4);
	CASE(DeeCode_HandleBreakpoint, 4);
	CASE(DeeList_NewUninitialized, 4);
	CASE(DeeTuple_NewUninitialized, 4);
	CASE(DeeTuple_NewVector, 4);
	CASE(DeeTuple_NewVectorSymbolic, 4);
	CASE(DeeTuple_DecrefSymbolic, 4);
	CASE(DeeInstance_GetMember, 12);
	CASE(DeeInstance_BoundMember, 12);
	CASE(DeeInstance_DelMember, 12);
	CASE(DeeInstance_SetMember, 16);
	CASE(DeeInstance_GetMemberSafe, 12);
	CASE(DeeInstance_BoundMemberSafe, 12);
	CASE(DeeInstance_DelMemberSafe, 12);
	CASE(DeeInstance_SetMemberSafe, 16);
	CASE(DeeClass_SetMember, 12);
	CASE(DeeClass_SetMemberSafe, 12);
	CASE(DeeClass_GetMember, 8);
	CASE(DeeClass_GetMemberSafe, 8);
	CASE(DeeClass_New, 4);
	CASE(DeeGC_Track, 4);
	CASE(DeeGC_Untrack, 4);
	CASE(Dee_Malloc, 4);
	CASE(Dee_Calloc, 4);
	CASE(Dee_Realloc, 8);
	CASE(Dee_TryMalloc, 4);
	CASE(Dee_TryCalloc, 4);
	CASE(Dee_TryRealloc, 8);
	CASE(Dee_Free, 4);
	CASE(DeeGCObject_Malloc, 4);
	CASE(DeeGCObject_Calloc, 4);
	CASE(DeeGCObject_Realloc, 8);
	CASE(DeeGCObject_TryMalloc, 4);
	CASE(DeeGCObject_TryCalloc, 4);
	CASE(DeeGCObject_TryRealloc, 8);
	CASE(DeeGCObject_Free, 4);
	CASE(DeeObject_Free, 4);
	CASE(libhostasm_rt_err_unbound_global, 8);
	CASE(libhostasm_rt_err_unbound_local, 12);
	CASE(libhostasm_rt_err_unbound_arg, 12);
	CASE(libhostasm_rt_err_illegal_instruction, 8);
	CASE(libhostasm_rt_err_no_active_exception, 0);
	CASE(libhostasm_rt_err_unbound_attribute_string, 8);
	CASE(libhostasm_rt_err_unbound_class_member, 8);
	CASE(libhostasm_rt_err_unbound_instance_member, 8);
	CASE(libhostasm_rt_err_requires_class, 4);
	CASE(libhostasm_rt_err_invalid_class_addr, 8);
	CASE(libhostasm_rt_err_invalid_instance_addr, 8);
	CASE(libhostasm_rt_DeeObject_ShlRepr, 8);
#undef CASE
#define CASE(sym)                   \
	if (addr == (void const *)&sym) \
		return #sym
	CASE(DeeObject_Type);
	CASE(DeeType_Type);
	CASE(DeeFunction_Type);
	CASE(DeeCode_Type);
#undef CASE
	*Dee_sprintf(buf, "%#Ix", addr) = '\0';
	return buf;
}

PRIVATE ATTR_RETNONNULL WUNUSED char const *DCALL
gen86_symname(struct host_symbol const *sym) {
	static char buf[sizeof("[sect:+0x]") + 2 * (sizeof(void *) * 2)];
	if (sym->hs_name != NULL)
		return sym->hs_name;
	switch (sym->hs_type) {
	case HOST_SYMBOL_ABS:
		return gen86_addrname(sym->hs_value.sv_abs);
	case HOST_SYMBOL_JUMP:
		*Dee_sprintf(buf, "[sect:%p]", &sym->hs_value.sv_jump->jd_to->bb_htext) = '\0';
		return buf;
	case HOST_SYMBOL_SECT:
		*Dee_sprintf(buf, "[sect:%p+%#Ix]", sym->hs_value.sv_sect.ss_sect, sym->hs_value.sv_sect.ss_off) = '\0';
		return buf;
	default: break;
	}
	return "[...]";
}

INTERN NONNULL((1)) void DCALL
_memloc_debug_print(struct memloc const *__restrict self) {
	switch (self->ml_adr.ma_typ) {
	case MEMADR_TYPE_HSTACK:
		/* Signed, because we cheat when it comes to how caller-arguments */
		Dee_DPRINTF("#%Id", (ptrdiff_t)self->ml_adr.ma_val.v_cfa);
		break;
	case MEMADR_TYPE_HSTACKIND:
		/* Signed, because we cheat when it comes to how caller-arguments */
		Dee_DPRINTF("[#%Id]", (ptrdiff_t)self->ml_adr.ma_val.v_cfa);
		if (self->ml_off != 0)
			Dee_DPRINTF("%+Id", self->ml_off);
		break;
	case MEMADR_TYPE_HREG:
		Dee_DPRINT(gen86_regname(self->ml_adr.ma_reg));
		if (self->ml_off != 0)
			Dee_DPRINTF("%+Id", self->ml_off);
		break;
	case MEMADR_TYPE_HREGIND:
		Dee_DPRINTF("[%s", gen86_regname(self->ml_adr.ma_reg));
		if (self->ml_adr.ma_val.v_indoff != 0)
			Dee_DPRINTF("%+Id", self->ml_adr.ma_val.v_indoff);
		Dee_DPRINT("]");
		if (self->ml_off != 0)
			Dee_DPRINTF("%+Id", self->ml_off);
		break;
	case MEMADR_TYPE_CONST:
		Dee_DPRINTF("$%p", self->ml_adr.ma_val.v_const);
		break;
	case MEMADR_TYPE_UNDEFINED:
		Dee_DPRINT("<undefined>");
		break;
	default:
		Dee_DPRINTF("{%I8u:%s,%p}",
		            self->ml_adr.ma_typ,
		            gen86_regname(self->ml_adr.ma_reg),
		            self->ml_adr.ma_val.v_const);
		if (self->ml_off != 0)
			Dee_DPRINTF("%+Id", self->ml_off);
		break;
	}
}

INTERN NONNULL((1)) void DCALL
_memobj_debug_print(struct memobj const *__restrict self,
                    bool is_local, bool noref) {
	if (is_local && memobj_local_neverbound(self)) {
		Dee_DPRINT("-");
		return;
	}
	/* XXX: Print mv_obj.mvo_0.mo_typeof? */
	if (memobj_isref(self) && !noref)
		Dee_DPRINT("r");
	_memloc_debug_print(memobj_getloc(self));
	if (is_local) {
		if (memobj_local_alwaysbound(self)) {
			Dee_DPRINT("!");
		} else {
			Dee_DPRINT("?");
		}
	}
}

PRIVATE NONNULL((1)) void DCALL
_memval_debug_print_objects(struct memval const *__restrict self,
                            bool is_local) {
	if (MEMVAL_VMORPH_HASOBJ0(self->mv_vmorph)) {
		_memobj_debug_print(&self->mv_obj.mvo_0, is_local, false);
	} else {
		size_t i;
		struct memobjs *objs = memval_getobjn(self);
		for (i = 0; i < objs->mos_objc; ++i) {
			if (i != 0)
				Dee_DPRINT(",");
			_memobj_debug_print(&objs->mos_objv[i], false,
			                        (self->mv_flags & MEMVAL_F_NOREF) != 0);
		}
	}
}

INTERN NONNULL((1)) void DCALL
_memval_debug_print(struct memval const *__restrict self,
                    bool is_local) {
	switch (self->mv_vmorph) {
	case MEMVAL_VMORPH_DIRECT:
	case MEMVAL_VMORPH_DIRECT_01:
		_memval_debug_print_objects(self, is_local);
		break;
	case MEMVAL_VMORPH_BOOL_Z:
	case MEMVAL_VMORPH_BOOL_Z_01:
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT("==0");
		break;
	case MEMVAL_VMORPH_BOOL_NZ:
	case MEMVAL_VMORPH_BOOL_NZ_01:
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT("!=0");
		break;
	case MEMVAL_VMORPH_BOOL_LZ:
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT("<0");
		break;
	case MEMVAL_VMORPH_BOOL_GZ:
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(">0");
		break;
	case MEMVAL_VMORPH_INT:
		Dee_DPRINT("int(");
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(")");
		break;
	case MEMVAL_VMORPH_UINT:
		Dee_DPRINT("uint(");
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(")");
		break;
	case MEMVAL_VMORPH_NULLABLE:
		Dee_DPRINT("nullable(");
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(")");
		break;
	default:
		Dee_DPRINTF("<%I8u:", self->mv_vmorph);
		_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(">");
		break;
	}
}

/* Return the name of `lid' at `code_addr' in `self' */
PRIVATE WUNUSED NONNULL((1)) char const *DCALL
DeeCode_LidNameAtAddr(DeeCodeObject *__restrict self,
                      uint16_t lid, Dee_code_addr_t code_addr) {
	uint8_t *ptr;
	char const *result;
	struct Dee_ddi_state st;
	struct Dee_ddi_xregs *iter;
	ptr = DeeCode_FindDDI(Dee_AsObject(self), &st, NULL,
	                      code_addr, Dee_DDI_STATE_FNOTHROW);
	if (!Dee_DDI_ISOK(ptr))
		return NULL;
	result = NULL;
	Dee_DDI_STATE_DO(iter, &st) {
		if (lid < iter->dx_lcnamc) {
			result = DeeCode_GetDDIString(Dee_AsObject(self), iter->dx_lcnamv[lid]);
			if (result)
				goto done;
		}
	}
	Dee_DDI_STATE_WHILE(iter, &st);
done:
	Dee_ddi_state_fini(&st);
	return result;
}

INTERN NONNULL((1)) void DCALL
_memstate_debug_print(struct memstate const *__restrict self,
                      struct function_assembler *assembler,
                      Dee_instruction_t const *instr) {
	Dee_DPRINTF("\tCFA:   #%Id\n", (uintptr_t)self->ms_host_cfa_offset);
	if (self->ms_stackc > 0) {
		uint16_t i;
		Dee_DPRINT("\tstack: ");
		for (i = 0; i < self->ms_stackc; ++i) {
			if (i != 0)
				Dee_DPRINT(", ");
			_memval_debug_print(&self->ms_stackv[i], false);
		}
		Dee_DPRINT("\n");
	}
	if (self->ms_localc > 0) {
		lid_t i;
		bool is_first = true;
		for (i = 0; i < self->ms_localc; ++i) {
			char const *lid_name;
			if (memval_isdirect(&self->ms_localv[i]) &&
			    memval_direct_local_neverbound(&self->ms_localv[i]) &&
			    !memval_direct_isref(&self->ms_localv[i]))
				continue;
			Dee_DPRINT(is_first ? "\tlocal: " : ", ");
			lid_name = NULL;
			if (assembler && i < assembler->fa_localc && instr) {
				/* Lookup the name of the local in DDI */
				lid_name = DeeCode_LidNameAtAddr(assembler->fa_code, (uint16_t)i,
				                                 function_assembler_addrof(assembler, instr));
			} else if (assembler && i >= assembler->fa_localc) {
				lid_t xlid = i - assembler->fa_localc;
				switch (xlid) {
				case MEMSTATE_XLOCAL_A_THIS:
					lid_name = "@this";
					break;
				case MEMSTATE_XLOCAL_A_ARGC:
					lid_name = "@argc";
					break;
				case MEMSTATE_XLOCAL_A_ARGV:
					lid_name = "@argv";
					if (assembler->fa_cc & HOST_CC_F_TUPLE)
						lid_name = "@args";
					break;
				case MEMSTATE_XLOCAL_A_KW:
					lid_name = "@kw";
					break;
				case MEMSTATE_XLOCAL_VARARGS:
					lid_name = "@varargs";
					break;
				case MEMSTATE_XLOCAL_VARKWDS:
					lid_name = "@varkwds";
					break;
				case MEMSTATE_XLOCAL_STDOUT:
					lid_name = "@stdout";
					break;
				case MEMSTATE_XLOCAL_POPITER:
					lid_name = "@popiter";
					break;
				default:
					if (xlid >= MEMSTATE_XLOCAL_DEFARG_MIN) {
						/* Lookup argument name from keywords */
						aid_t aid = (aid_t)(xlid - MEMSTATE_XLOCAL_DEFARG_MIN);
						if (assembler->fa_code->co_keywords &&
						    assembler->fa_code->co_keywords[aid]) {
							Dee_DPRINTF("@arg(%k)=", assembler->fa_code->co_keywords[aid]);
							goto do_print_memloc_desc;
						}
					}
					break;
				}
			}
			if (lid_name) {
				Dee_DPRINTF("%s=", lid_name);
			} else {
				Dee_DPRINTF("%I16u=", i);
			}
do_print_memloc_desc:
			_memval_debug_print(&self->ms_localv[i], true);
			is_first = false;
		}
		if (!is_first)
			Dee_DPRINT("\n");
	}
}

INTERN NONNULL((1)) void DCALL
_memequivs_debug_print(struct memequivs const *__restrict self) {
	size_t i;
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv *iter, *eq = &self->meqs_list[i];
		if (eq->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_UNUSED)
			continue;
		if (eq->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_DUMMY)
			continue;
		if (eq->meq_loc.ml_adr.ma_typ & 0x8000)
			continue;
		iter = eq;
		do {
			if (iter == eq) {
				HA_print("\t{");
			} else {
				Dee_DPRINT(" <=> ");
			}
			_memequiv_debug_print(iter);
			iter->meq_loc.ml_adr.ma_typ |= 0x8000;
		} while ((iter = memequiv_next(iter)) != eq);
		Dee_DPRINT("}\n");
	}
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv *eq = &self->meqs_list[i];
		if (eq->meq_loc.ml_adr.ma_typ & 0x8000)
			eq->meq_loc.ml_adr.ma_typ &= ~0x8000;
	}
}
#endif /* !NO_HOSTASM_DEBUG_PRINT */

#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
#define IF_VERBOSE_REFCNT_LOGGING(x) /* nothing */
#define IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY true
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#define IF_VERBOSE_REFCNT_LOGGING(x) x
#define IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY false
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */


/* Adjust the delta of `regno' */
PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gadjust_reg_delta_impl(struct host_section *sect,
                               struct fungen *self, /* when NULL, don't adjust memory locations using `regno' */
                               host_regno_t regno, ptrdiff_t val_delta
#if defined(NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY) && !defined(NO_HOSTASM_DEBUG_PRINT)
                               , bool log_instructions
#define LOCAL_gen86_printf(sect, ...) (log_instructions ? gen86_printf(sect, __VA_ARGS__) : (void)0)
#define _fungen_gadjust_reg_delta(sect, self, regno, val_delta, log_instructions) \
	_fungen_gadjust_reg_delta_impl(sect, self, regno, val_delta, log_instructions)
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY && !NO_HOSTASM_DEBUG_PRINT */
#define LOCAL_gen86_printf(sect, ...) gen86_printf(sect, __VA_ARGS__)
#define _fungen_gadjust_reg_delta(sect, self, regno, val_delta, log_instructions) \
	_fungen_gadjust_reg_delta_impl(sect, self, regno, val_delta)
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY || NO_HOSTASM_DEBUG_PRINT */
                               ) {
	if unlikely(val_delta == 0)
		return 0;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	switch (val_delta) {
	case -1:
		LOCAL_gen86_printf(sect, "dec" Plq "\t%s\n", gen86_regname(regno));
		gen86_decP_r(p_pc(sect), gen86_registers[regno]);
		break;
	case 1:
		LOCAL_gen86_printf(sect, "inc" Plq "\t%s\n", gen86_regname(regno));
		gen86_incP_r(p_pc(sect), gen86_registers[regno]);
		break;
	default:
		/* TODO: Special handling on x86_64 when !fit32(val_delta) */
		LOCAL_gen86_printf(sect, "lea" Plq "\t%Id(%s), %s\n", val_delta, gen86_regname(regno), gen86_regname(regno));
		gen86_leaP_db_r(p_pc(sect), val_delta, gen86_registers[regno], gen86_registers[regno]);
		break;
	}
	if (self != NULL)
		memstate_hregs_adjust_delta(self->fg_state, regno, val_delta);
	return 0;
err:
	return -1;
#undef LOCAL_gen86_printf
}

#ifdef fit32_IS_1
#define _fungen_gadjust_reg_fit32(self, p_regno, p_val_delta) 0
#else /* fit32_IS_1 */
#define _fungen_gadjust_reg_fit32(self, p_regno, p_val_delta) \
	(fit32(*(p_val_delta)) ? 0 : _fungen_gadjust_reg_fit32_impl(self, p_regno, p_val_delta))
PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gadjust_reg_fit32_impl(struct fungen *__restrict self,
                               host_regno_t *__restrict p_regno,
                               ptrdiff_t *__restrict p_val_delta) {
	int result;
	struct host_section *sect = fg_gettext(self);
	host_regno_t regno = *p_regno;
	ptrdiff_t val_delta = *p_val_delta;
	ptrdiff_t adj_delta;
	/* TODO: Check if there is a known register equivalence of `regno'
	 *       with a value-delta that causes the final `val_delta' to
	 *       fit into 32 bit */
	if (val_delta < INT32_MIN) {
		adj_delta = val_delta - INT32_MIN;
	} else if (val_delta > INT32_MAX) {
		adj_delta = val_delta - INT32_MAX;
	} else {
		adj_delta = 0;
	}
	result = _fungen_gadjust_reg_delta(sect, self, regno, val_delta, true);
	if likely(result == 0) {
		*p_val_delta = val_delta + adj_delta;
		ASSERT(fit32(*p_val_delta));
	}
	return result;
}
#endif /* !fit32_IS_1 */


PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gcall86_impl(struct fungen *__restrict self,
                     void const *api_function
#if defined(NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY) && !defined(NO_HOSTASM_DEBUG_PRINT)
                     , bool log_instructions
#define LOCAL_gen86_printf(sect, ...) (log_instructions ? gen86_printf(sect, __VA_ARGS__) : (void)0)
#define _fungen_gcall86(self, api_function, log_instructions) \
	_fungen_gcall86_impl(self, api_function, log_instructions)
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY && !NO_HOSTASM_DEBUG_PRINT */
#define LOCAL_gen86_printf(sect, ...) gen86_printf(sect, __VA_ARGS__)
#define _fungen_gcall86(self, api_function, log_instructions) \
	_fungen_gcall86_impl(self, api_function)
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY || NO_HOSTASM_DEBUG_PRINT */
                     ) {
	struct host_section *sect = fg_gettext(self);
	struct host_reloc *rel;

#ifdef FUNCTION_ASSEMBLER_F_MCLARGE
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_MCLARGE) {
		/* TODO: Instead of this:
		 * >> movabs $addr, %Pax
		 * >> callP  *%Pax
		 *
		 * Do this:
		 * >> callP  *.Laddr(%Pip)   # .byte 0xff, 0x15; .long (.Laddr-.)-4
		 * >> .pushsection .rodata
		 * >> .Laddr: .qword addr
		 * >> .popsection
		 *
		 * Where ".rodata" gets appended at the end of text.
		 * Or even better: add a special relocation for this
		 * case that looks at the delta during encode-time,
		 * and determines if a placement in .rodata is needed
		 *
		 * Or even better:
		 * - Always encode as "callP *.Laddr(%Pip)", and replace
		 *   with "nop; callP addr" during linking if the delta
		 *   fits into 32 bits. But is this actually faster in
		 *   the general case than the movabs solution?
		 */
		if unlikely(host_section_reqx86(sect, 2))
			goto err;
		LOCAL_gen86_printf(sect, "movabs\t$%s, %%" Per "ax\n", gen86_addrname(api_function));
		gen86_movabs_imm_r(p_pc(sect), api_function, GEN86_R_PAX);
		LOCAL_gen86_printf(sect, "call" Plq "\t*%%" Per "ax\n");
		gen86_callP_mod(p_pc(sect), gen86_modrm_r, GEN86_R_PAX);
		return 0;
	}
#endif /* FUNCTION_ASSEMBLER_F_MCLARGE */

	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	rel = host_section_newhostrel(sect);
	if unlikely(!rel)
		goto err;
	LOCAL_gen86_printf(sect, "calll\t%s\n", gen86_addrname(api_function));
	gen86_calll_offset(p_pc(sect), -4);
	rel->hr_offset = (uint32_t)(p_off(sect) - 4);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
	rel->hr_vtype  = DEE_HOST_RELOCVALUE_ABS;
	rel->hr_value.rv_abs = (void *)api_function;
	return 0;
err:
	return -1;
#undef LOCAL_gen86_printf
}


#if HOSTASM_SCRACHAREA_SIZE > 0 || HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER
PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gcall86_prepare_stack_for_call_impl(struct fungen *__restrict self,
                                       bool is_stractch_area_always_in_use
#if defined(NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY) && !defined(NO_HOSTASM_DEBUG_PRINT)
                                       , bool log_instructions
#define LOCAL_gen86_printf(sect, ...) (log_instructions ? gen86_printf(sect, __VA_ARGS__) : (void)0)
#define _fungen_gcall86_prepare_stack_for_call(self, is_stractch_area_always_in_use, log_instructions) \
	_fungen_gcall86_prepare_stack_for_call_impl(self, is_stractch_area_always_in_use, log_instructions)
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY && !NO_HOSTASM_DEBUG_PRINT */
#define LOCAL_gen86_printf(sect, ...) gen86_printf(sect, __VA_ARGS__)
#define _fungen_gcall86_prepare_stack_for_call(self, is_stractch_area_always_in_use, log_instructions) \
	_fungen_gcall86_prepare_stack_for_call_impl(self, is_stractch_area_always_in_use)
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY || NO_HOSTASM_DEBUG_PRINT */
                                       ) {
	struct host_section *sect = fg_gettext(self);
	struct memstate *state = self->fg_state;
	host_cfa_t current_cfa_offset = state->ms_host_cfa_offset;
	host_cfa_t new_cfa_offset_for_alignment;
	host_cfa_t new_cfa_offset;
	ptrdiff_t sp_alloc;
	(void)is_stractch_area_always_in_use;
	if (is_stractch_area_always_in_use) {
		new_cfa_offset = current_cfa_offset + HOSTASM_SCRACHAREA_SIZE;
	} else {
		host_cfa_t greatest_inuse_cfa_offset;
		greatest_inuse_cfa_offset = memstate_hstack_greatest_inuse(state);
		new_cfa_offset = greatest_inuse_cfa_offset + HOSTASM_SCRACHAREA_SIZE;
	}

	new_cfa_offset_for_alignment = new_cfa_offset; /* due to return address */
#if HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER
	new_cfa_offset_for_alignment += HOST_SIZEOF_POINTER; /* due to return address */
	new_cfa_offset_for_alignment += (HOSTASM_STACK_ALIGNMENT - 1);
	new_cfa_offset_for_alignment &= ~(HOSTASM_STACK_ALIGNMENT - 1);
	new_cfa_offset_for_alignment -= HOST_SIZEOF_POINTER; /* due to return address */
#endif /* HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER */
	sp_alloc = new_cfa_offset_for_alignment - current_cfa_offset;
	if (sp_alloc < 0) {
		/* Nothing needs to be allocated.
		 * -> See if the current SP would already be properly aligned (in which case: don't do anything) */
#if HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER
		if (IS_ALIGNED(current_cfa_offset + HOST_SIZEOF_POINTER, HOSTASM_STACK_ALIGNMENT))
			return 0;
#else /* HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER */
		return 0;
#endif /* HOSTASM_STACK_ALIGNMENT <= HOST_SIZEOF_POINTER */
	}
	if (sp_alloc != 0) {
		/* Must allocate of free space on the stack! */
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		if (sp_alloc < 0) {
			LOCAL_gen86_printf(sect, "add" Plq "\t$%Iu, %%" Per "sp\n", -sp_alloc);
			gen86_addP_imm_r(p_pc(sect), -sp_alloc, GEN86_R_PSP);
		} else {
			LOCAL_gen86_printf(sect, "sub" Plq "\t$%Iu, %%" Per "sp\n", sp_alloc);
			gen86_subP_imm_r(p_pc(sect), sp_alloc, GEN86_R_PSP);
		}
		return fg_gadjust_cfa_offset(self, sp_alloc);
	}
	return 0;
err:
	return -1;
#undef LOCAL_gen86_printf
}
#else /* HOSTASM_SCRACHAREA_SIZE > 0 || HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER */
#define _fungen_gcall86_prepare_stack_for_call_IS_NOOP
#define _fungen_gcall86_prepare_stack_for_call(self, is_stractch_area_always_in_use, log_instructions) 0
#endif /* HOSTASM_SCRACHAREA_SIZE <= 0 && HOSTASM_STACK_ALIGNMENT <= HOST_SIZEOF_POINTER */


PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gcall86_with_preserved_stack_impl(struct fungen *__restrict self,
                                     void const *api_function, size_t pushed_sp_offset
#ifndef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
                                     , bool is_stractch_area_always_in_use
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */
#if defined(NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY) && !defined(NO_HOSTASM_DEBUG_PRINT)
                                     , bool log_instructions
#define LOCAL_gen86_printf(sect, ...) (log_instructions ? gen86_printf(sect, __VA_ARGS__) : (void)0)
#ifdef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
#define _fungen_gcall86_with_preserved_stack(self, api_function, pushed_sp_offset, is_stractch_area_always_in_use, log_instructions) \
	_fungen_gcall86_with_preserved_stack_impl(self, api_function, pushed_sp_offset, log_instructions)
#else /* _fungen_gcall86_prepare_stack_for_call_IS_NOOP */
#define _fungen_gcall86_with_preserved_stack(self, api_function, pushed_sp_offset, is_stractch_area_always_in_use, log_instructions) \
	_fungen_gcall86_with_preserved_stack_impl(self, api_function, pushed_sp_offset, is_stractch_area_always_in_use, log_instructions)
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY && !NO_HOSTASM_DEBUG_PRINT */
#define LOCAL_gen86_printf(sect, ...) gen86_printf(sect, __VA_ARGS__)
#ifdef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
#define _fungen_gcall86_with_preserved_stack(self, api_function, pushed_sp_offset, is_stractch_area_always_in_use, log_instructions) \
	_fungen_gcall86_with_preserved_stack_impl(self, api_function, pushed_sp_offset)
#else /* _fungen_gcall86_prepare_stack_for_call_IS_NOOP */
#define _fungen_gcall86_with_preserved_stack(self, api_function, pushed_sp_offset, is_stractch_area_always_in_use, log_instructions) \
	_fungen_gcall86_with_preserved_stack_impl(self, api_function, pushed_sp_offset, is_stractch_area_always_in_use)
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY || NO_HOSTASM_DEBUG_PRINT */
                                     ) {
	struct host_section *sect = fg_gettext(self);
	host_cfa_t new_cfa, old_cfa = self->fg_state->ms_host_cfa_offset;
	size_t sp_dealloc, sp_adjust;
#ifndef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
	if (pushed_sp_offset > 0)
		is_stractch_area_always_in_use = true;
	if unlikely(_fungen_gcall86_prepare_stack_for_call(self, is_stractch_area_always_in_use, log_instructions))
		goto err;
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */
	if unlikely(_fungen_gcall86(self, api_function, log_instructions))
		goto err;
	new_cfa = self->fg_state->ms_host_cfa_offset;
	sp_dealloc = (new_cfa - old_cfa);
	sp_adjust = sp_dealloc;
	sp_adjust += pushed_sp_offset;
#ifdef HOSTASM_X86_64 /* On i386, STDCALL means that the callee pops functions arguments */
	sp_dealloc = sp_adjust;
#endif /* HOSTASM_X86_64 */
	if (sp_dealloc > 0) {
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		LOCAL_gen86_printf(sect, "add" Plq "\t$%Iu, %%" Per "sp\n", sp_dealloc);
		gen86_addP_imm_r(p_pc(sect), (ptrdiff_t)sp_dealloc, GEN86_R_PSP);
	}
	return fg_gadjust_cfa_offset(self, -(ptrdiff_t)sp_adjust);
err:
	return -1;
#undef LOCAL_gen86_printf
}



/* Object reference count incref/decref */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gincref_regx(struct fungen *__restrict self,
                     host_regno_t regno,
                     ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct host_section *sect;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(_fungen_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
	sect = fg_gettext(self);
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf(sect, "incref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf(sect, "incref\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock inc" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock inc" Plq "\tob_refcnt+%Id(%s)\n",
			             reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock add" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock add" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n,
			             reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	if unlikely(host_section_reqx86(sect, 2))
		goto err;
	gen86_lock(p_pc(sect));
	if (n == 1) {
		gen86_incP_mod(p_pc(sect), gen86_modrm_db,
		               reg_offset, gen86_registers[regno]);
	} else {
		gen86_addP_imm_mod(p_pc(sect), gen86_modrm_db, (intptr_t)(uintptr_t)n,
		                   reg_offset, gen86_registers[regno]);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gdecref_nokill_regx(struct fungen *__restrict self,
                            host_regno_t regno,
                            ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct host_section *sect = fg_gettext(self);
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(_fungen_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf(sect, "decref_nokill\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf(sect, "decref_nokill\t%Id(%s)",
		             reg_offset - OFFSETOF_ob_refcnt,
		             gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock dec" Plq "\tob_refcnt+%Id(%s)\n", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock sub" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock sub" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n, reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	if unlikely(host_section_reqx86(sect, 2))
		goto err;
	gen86_lock(p_pc(sect));
	if (n == 1) {
		gen86_decP_mod(p_pc(sect), gen86_modrm_db,
		               reg_offset, gen86_registers[regno]);
	} else {
		gen86_subP_imm_mod(p_pc(sect), gen86_modrm_db, (intptr_t)(uintptr_t)n,
		                   reg_offset, gen86_registers[regno]);
	}
	return 0;
err:
	return -1;
}

#if !defined(CONFIG_NO_BADREFCNT_CHECKS) || defined(CONFIG_TRACE_REFCHANGES)
DFUNDEF NONNULL((1)) void (DCALL DeeObject_Destroy)(DeeObject *__restrict self);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS || CONFIG_TRACE_REFCHANGES */

PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gdestroy_regx(struct fungen *__restrict self,
                      host_regno_t regno, ptrdiff_t reg_offset,
                      bool do_kill) {
	host_regno_t used_regno = regno;
	struct host_section *sect = fg_gettext(self);
	struct memstate *state = self->fg_state;
	host_regno_t save_regno;
#ifndef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
	bool is_stractch_area_always_in_use = false;
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */

	/* Save registers. */
	for (save_regno = 0; save_regno < HOST_REGNO_COUNT; ++save_regno) {
		if (state->ms_rinuse[save_regno] == 0)
			continue;
		if (save_regno == regno)
			continue;
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "push" Plq "\t%s\n", gen86_regname(save_regno)));
		gen86_pushP_r(p_pc(sect), gen86_registers[save_regno]);
		if unlikely(fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER))
			goto err;
#ifndef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
		is_stractch_area_always_in_use = true;
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */
	}

	/* Load `regno' as argument for the call to `DeeObject_Destroy()' */
#ifdef HOST_REGNO_R_ARG0
	if (used_regno != HOST_REGNO_R_ARG0) {
		ptrdiff_t adj_delta;
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		adj_delta = reg_offset;
		if (adj_delta < INT32_MIN) {
			adj_delta = INT32_MIN;
		} else if (adj_delta > INT32_MAX) {
			adj_delta = INT32_MAX;
		}
		IF_VERBOSE_REFCNT_LOGGING(
		gen86_printf(sect, "lea" Plq "\t%Id(%s), %s\n",
		             adj_delta, gen86_regname(used_regno),
		             gen86_regname(HOST_REGNO_R_ARG0)));
		gen86_leaP_db_r(p_pc(sect),
		                adj_delta, gen86_registers[used_regno],
		                gen86_registers[HOST_REGNO_R_ARG0]);
		reg_offset -= adj_delta;
		used_regno = HOST_REGNO_R_ARG0;
	}
#endif /* HOST_REGNO_R_ARG0 */
	if unlikely(_fungen_gadjust_reg_delta(sect, NULL, used_regno, reg_offset,
	                                 !IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY))
		goto err;

	{
		size_t req_instructions = 0;
		if (do_kill)
			req_instructions += 1; /* movP $0, ob_refcnt(%used_regno) */
#ifndef HOST_REGNO_R_ARG0
		req_instructions += 1; /* pushP %used_regno */
#endif /* !HOST_REGNO_R_ARG0 */
#ifdef HOSTASM_X86_64_MSABI
		req_instructions += 1; /* subP $32, %Psp */
#endif /* HOSTASM_X86_64_MSABI */
#ifdef FUNCTION_ASSEMBLER_F_MCLARGE
		if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_MCLARGE)
			req_instructions += 1; /* movabs $DeeObject_Destroy, %Pax */
#endif /* FUNCTION_ASSEMBLER_F_MCLARGE */
		req_instructions += 1; /* call DeeObject_Destroy */
#ifdef HOSTASM_X86_64_MSABI
		req_instructions += 1; /* addP $32, %Psp */
#endif /* HOSTASM_X86_64_MSABI */
		if unlikely(host_section_reqx86(sect, req_instructions))
			goto err;
	}

	if (do_kill) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "mov" Plq "\t$0, ob_refcnt%+Id(%s)\n",
		                                       reg_offset - OFFSETOF_ob_refcnt,
		                                       gen86_regname(used_regno)));
		gen86_movP_imm_mod(p_pc(sect), gen86_modrm_db, 0,
		                   reg_offset + OFFSETOF_ob_refcnt,
		                   gen86_registers[used_regno]);
	}

	/* Make the call to `DeeObject_Destroy()' */
#ifndef HOST_REGNO_R_ARG0
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "push" Plq "\t%s\n", gen86_regname(used_regno)));
	gen86_pushP_r(p_pc(sect), gen86_registers[used_regno]);
	if unlikely(fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER))
		goto err;
	if unlikely(_fungen_gcall86_with_preserved_stack(self, (void const *)&DeeObject_Destroy, HOST_SIZEOF_POINTER,
	                                            is_stractch_area_always_in_use,
	                                            !IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY))
		goto err;
#else /* !HOST_REGNO_R_ARG0 */
	if unlikely(_fungen_gcall86_with_preserved_stack(self, (void const *)&DeeObject_Destroy, 0,
	                                            is_stractch_area_always_in_use,
	                                            !IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY))
		goto err;
#endif /* !HOST_REGNO_R_ARG0 */

	/* After calling an external function, usage registers may have gotten clobbered. */
	memstate_hregs_clear_usage(self->fg_state);

	/* Restore registers (in reverse order). */
	save_regno = HOST_REGNO_COUNT;
	while (save_regno) {
		--save_regno;
		if (state->ms_rinuse[save_regno] == 0)
			continue;
		if (save_regno == regno)
			continue;
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "pop" Plq "\t%s\n", gen86_regname(save_regno)));
		gen86_popP_r(p_pc(sect), gen86_registers[save_regno]);
		if unlikely(fg_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER))
			goto err;
	}

	/* Unused registers are now undefined (used registers were saved and restored) */
	fg_remember_undefined_unusedregs(self);
	return 0;
err:
	return -1;
}

#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
PRIVATE NONNULL((1)) void DCALL
log_compact_decref_register_preserve_list(struct fungen *__restrict self,
                                          host_regno_t regno) {
	struct memstate *state = self->fg_state;
	host_regno_t save_regno;
	bool is_first = true;

	/* Save registers. */
	for (save_regno = 0; save_regno < HOST_REGNO_COUNT; ++save_regno) {
		if (!memstate_hregs_isused(state, save_regno))
			continue;
		if (save_regno == regno)
			continue;
		Dee_DPRINT(", ");
		if (is_first) {
			Dee_DPRINT("{");
			is_first = false;
		}
		Dee_DPRINT(gen86_regname(save_regno));
	}
	if (!is_first)
		Dee_DPRINT("}");
}
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */


INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gdecref_regx_impl(struct fungen *__restrict self,
                          host_regno_t regno,
                          ptrdiff_t ob_refcnt_offset, Dee_refcnt_t n) {
	struct host_section *text = fg_gettext(self);
	struct host_section *cold = fg_getcold(self);
	if unlikely(!cold)
		goto err;
	if unlikely(_fungen_gadjust_reg_delta(text, self, regno, ob_refcnt_offset,
	                                 !IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY))
		goto err;
	if unlikely(host_section_reqx86(text, 3))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (n == 1) {
		if (ob_refcnt_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock dec" Plq "\tob_refcnt+%Id(%s)\n", ob_refcnt_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (ob_refcnt_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock sub" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock sub" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n, ob_refcnt_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(text));
	if (n == 1) {
		gen86_decP_mod(p_pc(text), gen86_modrm_db,
		               ob_refcnt_offset, gen86_registers[regno]);
	} else {
		gen86_subP_imm_mod(p_pc(text), gen86_modrm_db, (intptr_t)(uintptr_t)n,
		                   ob_refcnt_offset, gen86_registers[regno]);
	}

	/* NOTE: decP sets FLAGS.Z=1 when the reference counter becomes `0'. */
	if (text == cold) {
		struct host_symbol *sym_1f;
		struct host_reloc *rel;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "jnz8\t1f\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_NZ, -1);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		sym_1f = fg_newsym(self);
		if unlikely(!sym_1f)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = sym_1f;
		ob_refcnt_offset -= OFFSETOF_ob_refcnt;
		if unlikely(_fungen_gdestroy_regx(self, regno, ob_refcnt_offset, false))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "1:\n"));
		_host_symbol_setsect(sym_1f, text);
	} else {
		struct host_reloc *enter_rel, *leave_rel;
		struct host_symbol *enter_sym, *leave_sym;

		/* Generate code that jumps into the cold section when the reference counter became `0'. */
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "jzl\t1f\n"));
		gen86_jccl_offset(p_pc(text), GEN86_CC_Z, -4);
		enter_rel = host_section_newhostrel(text);
		if unlikely(!enter_rel)
			goto err;
		enter_sym = fg_newsym(self);
		if unlikely(!enter_sym)
			goto err;
		enter_rel->hr_offset = (uint32_t)(p_off(text) - 4);
		enter_rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		enter_rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		enter_rel->hr_value.rv_sym = enter_sym;

		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, ".section .cold\n"));
		if unlikely(fg_settext(self, cold))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "1:\n"));
		_host_symbol_setsect(enter_sym, cold);

		/* Generate the destroy-code *within* the cold section. */
		ob_refcnt_offset -= OFFSETOF_ob_refcnt;
		if unlikely(_fungen_gdestroy_regx(self, regno, ob_refcnt_offset, false))
			goto err;

		/* Generate code to jump from the cold section back to the normal section. */
		if unlikely(host_section_reqx86(cold, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "jmpl\t1f\n"));
		gen86_jmpl_offset(p_pc(cold), -4);
		leave_rel = host_section_newhostrel(cold);
		if unlikely(!leave_rel)
			goto err;
		leave_sym = fg_newsym(self);
		if unlikely(!leave_sym)
			goto err;
		leave_rel->hr_offset = (uint32_t)(p_off(cold) - 4);
		leave_rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		leave_rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		leave_rel->hr_value.rv_sym = leave_sym;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, ".section .text\n"));
		if unlikely(fg_settext(self, text))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "1:\n"));
		host_symbol_setsect(leave_sym, text);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gdecref_regx(struct fungen *__restrict self,
                     host_regno_t regno, ptrdiff_t reg_offset,
                     Dee_refcnt_t n) {
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf(fg_gettext(self), "decref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf(fg_gettext(self), "decref\t%Id(%s)", reg_offset, gen86_regname(regno));
	}
	log_compact_decref_register_preserve_list(self, regno);
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		if (reg_offset == 0) {
			gen86_printf(fg_gettext(self), "lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf(fg_gettext(self), "lock dec" Plq "\tob_refcnt+%Id(%s)\n", reg_offset, gen86_regname(regno));
		}
	} else {
		if (reg_offset == 0) {
			gen86_printf(fg_gettext(self), "lock sub" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf(fg_gettext(self), "lock sub" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n, reg_offset, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	return _fungen_gdecref_regx_impl(self, regno, reg_offset + OFFSETOF_ob_refcnt, n);
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gdecref_regx_dokill(struct fungen *__restrict self,
                            host_regno_t regno, ptrdiff_t reg_offset) {
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf(fg_gettext(self), "decref_dokill\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf(fg_gettext(self), "decref_dokill\t%Id(%s)", reg_offset, gen86_regname(regno));
	}
	log_compact_decref_register_preserve_list(self, regno);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	return _fungen_gdestroy_regx(self, regno, reg_offset, true);
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gxincref_regx(struct fungen *__restrict self,
                      host_regno_t regno,
                      ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct host_section *sect = fg_gettext(self);
	uint8_t reg86;
	uintptr_t jcc8_offset;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(_fungen_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
	if unlikely(host_section_reqx86(sect, 4))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf(sect, "xincref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf(sect, "xincref\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	reg86 = gen86_registers[regno];
	if (reg_offset == OFFSETOF_ob_refcnt) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(sect), reg86, reg86);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "cmp" Plq "\t$%Id, %s\n", -reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(sect), -reg_offset - OFFSETOF_ob_refcnt, reg86);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "jz8\t1f\n"));
	gen86_jcc8_offset(p_pc(sect), GEN86_CC_Z, -1);
	jcc8_offset = p_off(sect) - 1;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (n == 1) {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock inc" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock inc" Plq "\tob_refcnt+%Id(%s)\n",
			             reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf(sect, "lock add" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf(sect, "lock add" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n,
			             reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(sect));
	if (n == 1) {
		gen86_incP_mod(p_pc(sect), gen86_modrm_db,
		               reg_offset, gen86_registers[regno]);
	} else {
		gen86_addP_imm_mod(p_pc(sect), gen86_modrm_db, (intptr_t)(uintptr_t)n,
		                   reg_offset, gen86_registers[regno]);
	}
	*(int8_t *)(sect->hs_start + jcc8_offset) += (int8_t)(p_off(sect) - jcc8_offset);
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "1:\n"));
	return 0;
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1)) int DCALL
_fungen_gxdecref_nokill_regx(struct fungen *__restrict self,
                             host_regno_t regno,
                             ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct host_section *sect = fg_gettext(self);
	uintptr_t jcc8_offset;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(_fungen_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
	if unlikely(host_section_reqx86(sect, 4))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf(sect, "xdecref_nokill\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf(sect, "xdecref_nokill\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	if (reg_offset == OFFSETOF_ob_refcnt) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(sect), gen86_registers[regno], gen86_registers[regno]);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "cmp" Plq "\t$%Id, %s\n", -reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(sect), -reg_offset - OFFSETOF_ob_refcnt, gen86_registers[regno]);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "jz8\t1f\n"));
	gen86_jcc8_offset(p_pc(sect), GEN86_CC_Z, -1);
	jcc8_offset = p_off(sect) - 1;
	gen86_lock(p_pc(sect));
	if (n == 1) {
		gen86_decP_mod(p_pc(sect), gen86_modrm_db,
		               reg_offset, gen86_registers[regno]);
	} else {
		gen86_subP_imm_mod(p_pc(sect), gen86_modrm_db, (intptr_t)(uintptr_t)n,
		                   reg_offset, gen86_registers[regno]);
	}
	/* Fix-up the jump */
	*(int8_t *)(sect->hs_start + jcc8_offset) += (int8_t)(p_off(sect) - jcc8_offset);
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "1:\n"));
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gxdecref_regx(struct fungen *__restrict self,
                      host_regno_t regno,
                      ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct host_section *sect = fg_gettext(self);
	struct host_symbol *sym_1f;
	struct host_reloc *rel;
	uint8_t reg86;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(_fungen_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
	if unlikely(host_section_reqx86(sect, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf(sect, "xdecref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf(sect, "xdecref\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	log_compact_decref_register_preserve_list(self, regno);
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	reg86 = gen86_registers[regno];
	if (reg_offset == OFFSETOF_ob_refcnt) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(sect), reg86, reg86);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "cmp" Plq "\t$%Id, %s\n", -reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(sect), -reg_offset - OFFSETOF_ob_refcnt, reg86);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "jz8\t1f\n"));
	gen86_jcc8_offset(p_pc(sect), GEN86_CC_Z, -1);
	rel = host_section_newhostrel(sect);
	if unlikely(!rel)
		goto err;
	sym_1f = fg_newsym(self);
	if unlikely(!sym_1f)
		goto err;
	rel->hr_offset = (uint32_t)(p_off(sect) - 1);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
	rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
	rel->hr_value.rv_sym = sym_1f;
	if unlikely(_fungen_gdecref_regx_impl(self, regno, reg_offset, n))
		goto err;
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf(sect, "1:\n"));
	_host_symbol_setsect(sym_1f, sect);
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gincref_const(struct host_section *__restrict self,
                            DeeObject *value, Dee_refcnt_t n) {
	ptrdiff_t caddr = (ptrdiff_t)(uintptr_t)&value->ob_refcnt;
#ifdef _host_section_gincref_const_MAYFAIL
	if (!fit32(caddr))
		return 1;
#endif /* _host_section_gincref_const_MAYFAIL */
	if unlikely(host_section_reqx86(self, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	gen86_printf(self, "incref\t%s", gen86_addrname(value));
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINTF("\t# const @%r\n", value);
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		gen86_printf(self, "lock inc" Plq "\t%#Ix\n", caddr);
	} else {
		gen86_printf(self, "lock add" Plq "\t$%Iu, %#Ix\n", n, caddr);
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(self));
	if (n == 1) {
		gen86_incP_mod(p_pc(self), gen86_modrm_d, caddr);
	} else {
		gen86_addP_imm_mod(p_pc(self), gen86_modrm_d, (intptr_t)(uintptr_t)n, caddr);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gdecref_const(struct host_section *__restrict self,
                            DeeObject *value, Dee_refcnt_t n) {
	ptrdiff_t caddr = (ptrdiff_t)(uintptr_t)&value->ob_refcnt;
#ifdef _host_section_gdecref_const_MAYFAIL
	if (!fit32(caddr))
		return 1;
#endif /* _host_section_gdecref_const_MAYFAIL */
	if unlikely(host_section_reqx86(self, 2))
		goto err;
	/* Constants can never be destroyed, so decref'ing one is
	 * like `Dee_DecrefNoKill()' (iow: doesn't need a zero-check) */
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	gen86_printf(self, "decref_nokill\t%s", gen86_addrname(value));
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINTF("\t# const @%r\n", value);
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		gen86_printf(self, "lock dec" Plq "\t%#Ix\n", caddr);
	} else {
		gen86_printf(self, "lock sub" Plq "\t$%Iu, %#Ix\n", n, caddr);
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(self));
	if (n == 1) {
		gen86_decP_mod(p_pc(self), gen86_modrm_d, caddr);
	} else {
		gen86_subP_imm_mod(p_pc(self), gen86_modrm_d, (intptr_t)(uintptr_t)n, caddr);
	}
	return 0;
err:
	return -1;
}




#ifndef CONFIG_NO_THREADS
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
#define IF_VERBOSE_LOCK_LOGGING(x) /* nothing */
#define HAVE_VERBOSE_LOCK_LOGGING 0
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#define IF_VERBOSE_LOCK_LOGGING(x) x
#define HAVE_VERBOSE_LOCK_LOGGING 1
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */

#ifndef __NO_hybrid_yield
#ifdef __hybrid_yield_IS_SleepEx
#define rt_sched_yield_IS_SleepEx
#elif defined(__hybrid_yield_IS_sched_yield)
#define rt_sched_yield sched_yield
#elif defined(__hybrid_yield_IS_pthread_yield)
#define rt_sched_yield pthread_yield
#elif defined(__hybrid_yield_IS_thrd_yield)
#define rt_sched_yield thrd_yield
#else /* ... */
#define rt_sched_yield impl_rt_sched_yield
PRIVATE void impl_rt_sched_yield(void) {
	__hybrid_yield();
}
#endif /* !... */
#endif /* !__NO_hybrid_yield */

PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gpause_or_yield(struct fungen *__restrict self,
                        host_regno_t always_preserve_regno) {
#ifdef __NO_hybrid_yield
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_pause(p_pc(sect));
#else /* __NO_hybrid_yield */
	host_regno_t regno;
	struct memstate *state = self->fg_state;
	struct host_section *sect;
#ifndef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
	bool is_stractch_area_always_in_use = false;
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */

	/* Push registers that need to be preserved. */
	for (regno = 0; regno < HOST_REGNO_COUNT; ++regno) {
		if (regno == HOST_REGNO_PAX)
			continue;
		if (state->ms_rinuse[regno] == 0 && regno != always_preserve_regno)
			continue;
		if unlikely(fg_ghstack_pushreg(self, regno))
			goto err;
#ifndef _fungen_gcall86_prepare_stack_for_call_IS_NOOP
		is_stractch_area_always_in_use = true;
#endif /* !_fungen_gcall86_prepare_stack_for_call_IS_NOOP */
	}
	sect = fg_gettext(self);

	/* Make the call to the host's `sched_yield(2)' function */
#ifdef rt_sched_yield_IS_SleepEx
	if unlikely(host_section_reqx86(sect, 2))
		goto err;
#ifdef HOSTASM_X86_64
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "xor" Plq "\t%s, %s\n", gen86_regname(HOST_REGNO_R_ARG0), gen86_regname(HOST_REGNO_R_ARG0)));
	gen86_xorP_r_r(p_pc(sect), gen86_registers[HOST_REGNO_R_ARG0], gen86_registers[HOST_REGNO_R_ARG0]);
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "xor" Plq "\t%s, %s\n", gen86_regname(HOST_REGNO_R_ARG1), gen86_regname(HOST_REGNO_R_ARG1)));
	gen86_xorP_r_r(p_pc(sect), gen86_registers[HOST_REGNO_R_ARG1], gen86_registers[HOST_REGNO_R_ARG1]);
	if unlikely(_fungen_gcall86_with_preserved_stack(self, (void const *)&SleepEx, 0, is_stractch_area_always_in_use, HAVE_VERBOSE_LOCK_LOGGING))
		goto err;
#else /* HOSTASM_X86_64 */
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "push" Plq "\t$0\n"));
	gen86_pushP_imm(p_pc(sect), 0);
	if unlikely(fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER))
		goto err;
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "push" Plq "\t$0\n"));
	gen86_pushP_imm(p_pc(sect), 0);
	if unlikely(fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER))
		goto err;
	if unlikely(_fungen_gcall86_with_preserved_stack(self, (void const *)&SleepEx, HOST_SIZEOF_POINTER * 2, is_stractch_area_always_in_use, HAVE_VERBOSE_LOCK_LOGGING))
		goto err;
#endif /* !HOSTASM_X86_64 */

#else /* rt_sched_yield_IS_SleepEx */
	if unlikely(_fungen_gcall86_with_preserved_stack(self, (void const *)&rt_sched_yield, 0, is_stractch_area_always_in_use, HAVE_VERBOSE_LOCK_LOGGING))
		goto err;
#endif /* !rt_sched_yield_IS_SleepEx */

	/* Pop saved registers. */
	regno = HOST_REGNO_COUNT;
	while (regno) {
		--regno;
		if (regno == HOST_REGNO_PAX)
			continue;
		if (state->ms_rinuse[regno] == 0 && regno != always_preserve_regno)
			continue;
		if unlikely(fg_ghstack_popreg(self, regno))
			goto err;
	}
#endif /* !__NO_hybrid_yield */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_read_impl2(struct fungen *__restrict self,
                           struct memloc *__restrict loc,
                           host_regno_t tempreg) {
#define LOCAL_loc_isreg()  (loc->ml_adr.ma_typ == MEMADR_TYPE_HREG)
#define LOCAL_loc_regno()  (loc->ml_adr.ma_reg)
#define LOCAL_loc_reg86()  (gen86_registers[loc->ml_adr.ma_reg])
#define LOCAL_loc_regnam() gen86_regname(loc->ml_adr.ma_reg)
#define LOCAL_loc_regbas() memloc_getoff(loc)
#define LOCAL_loc_regoff() (memloc_getoff(loc) + (ptrdiff_t)offsetof(Dee_atomic_rwlock_t, arw_lock))
#define LOCAL_loc_const()  ((intptr_t)(uintptr_t)&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)
	struct host_symbol *text_Lfull_retry;
	struct host_symbol *text_LPax_retry;
	struct host_reloc *rel;
	struct host_symbol *cold_Lpause_and_full_retry;
	struct host_symbol *text_Ldone;
	struct host_section *text = fg_gettext(self);
	struct host_section *cold = fg_getcold(self);
	if unlikely(!cold)
		goto err;
	if unlikely(host_section_reqx86(text, 6))
		goto err;
	cold_Lpause_and_full_retry = NULL;
	text_Ldone                 = NULL;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
		cold_Lpause_and_full_retry = fg_newsym(self);
		if unlikely(!cold_Lpause_and_full_retry)
			goto err;
	} else if (text == cold) {
		text_Ldone = fg_newsym(self);
		if unlikely(!text_Ldone)
			goto err;
	}

	/* >> .Lfull_retry:
	 * >>     movP  <lock->arw_lock>, %Pax
	 * >> .LPax_retry:
	 * >>     cmpP  $-1, %Pax
	 * >>     je{8|l} OSIZE ? .Lfull_retry : .Lpause_and_full_retry
	 * >>     leaP  1(%Pax), %tempreg
	 * >>     lock  cmpxchgP %tempreg, <lock->arw_lock>
	 * >> #if OSIZE
	 * >>     jnz8  .LPax_retry
	 * >> #elif text == cold
	 * >>     jz8   .Ldone
	 * >> #else
	 * >>     jnzl  .Lpause_and_full_retry
	 * >> #endif
	 * >>
	 * >> #if !OSIZE
	 * >> #if text != cold
	 * >> .section .cold
	 * >> #endif
	 * >> .Lpause_and_full_retry:
	 * >>     PAUSE_OR_SCHED_YIELD
	 * >>     jmp{8|l}  .Lfull_retry
	 * >> #endif
	 * >> .Ldone:
	 */
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, ".Lfull_retry:\n"));
	text_Lfull_retry = fg_newsym(self);
	if unlikely(!text_Lfull_retry)
		goto err;
	_host_symbol_setsect(text_Lfull_retry, text);
	if (LOCAL_loc_isreg()) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "mov" Plq "\tarw_lock%+Id(%s), %%" Per "ax\n", LOCAL_loc_regbas(), LOCAL_loc_regnam()));
		gen86_movP_db_r(p_pc(text), LOCAL_loc_regoff(), LOCAL_loc_reg86(), GEN86_R_PAX);
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "mov" Plq "\t%#Ix, %%" Per "ax\n", LOCAL_loc_const()));
		gen86_movP_d_r(p_pc(text), LOCAL_loc_const(), GEN86_R_PAX);
	}
	text_LPax_retry = NULL;
	if (text != cold && (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
		text_LPax_retry = fg_newsym(self);
		if unlikely(!text_LPax_retry)
			goto err;
		_host_symbol_setsect(text_LPax_retry, text);
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
		gen86_printf(sect, ".LPax_retry:");
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	}
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "cmp" Plq "\t$-1, %%" Per "ax\n"));
	gen86_cmpP_imm_Pax(p_pc(text), -1);
	if (cold_Lpause_and_full_retry && text != cold) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jel\t.Lpause_and_full_retry\n"));
		gen86_jccl_offset(p_pc(text), GEN86_CC_E, -4);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 4);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
	} else if (cold_Lpause_and_full_retry) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "je8\t.Lpause_and_full_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_E, -1);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "je8\t.Lfull_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_E, -1);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		ASSERT(text_Lfull_retry);
		rel->hr_value.rv_sym = text_Lfull_retry;
	}

	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "lea" Plq "\t1(%%" Per "ax), %s\n", gen86_regname(tempreg)));
	gen86_leaP_db_r(p_pc(text), 1, GEN86_R_PAX, gen86_registers[tempreg]);

	if (LOCAL_loc_isreg()) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "lock cmpxchg" Plq "\t%s, arw_lock%+Id(%s)\n",
		                                     gen86_regname(tempreg), LOCAL_loc_regbas(), LOCAL_loc_regnam()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_db, gen86_registers[tempreg],
		                     LOCAL_loc_regoff(), LOCAL_loc_reg86());
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "lock cmpxchg" Plq "\t%s, %#Ix\n", gen86_regname(tempreg), LOCAL_loc_const()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_d, gen86_registers[tempreg], LOCAL_loc_const());
	}

	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jnz8\t.LPax_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_NZ, -1);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		ASSERT(text_LPax_retry);
		rel->hr_value.rv_sym = text_LPax_retry;
	} else if (text == cold) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jz8\t.Ldone\n"));
		ASSERT(text_Ldone != NULL);
		gen86_jcc8_offset(p_pc(text), GEN86_CC_Z, -1);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = text_Ldone;
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jnzl\t.Lpause_and_full_retry\n"));
		ASSERT(cold_Lpause_and_full_retry != NULL);
		gen86_jccl_offset(p_pc(text), GEN86_CC_NZ, -4);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 4);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
	}

	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
		ASSERT(cold_Lpause_and_full_retry);
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
		if (text != cold)
			gen86_printf(sect, ".section .cold\n");
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		if unlikely(fg_settext(self, cold))
			goto err;
		_host_symbol_setsect(cold_Lpause_and_full_retry, cold);
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "Lpause_and_full_retry:\n"));
		if unlikely(_fungen_gpause_or_yield(self,
		                                    LOCAL_loc_isreg()
		                                    ? LOCAL_loc_regno()
		                                    : HOST_REGNO_COUNT))
			goto err;
		if unlikely(host_section_reqx86(cold, 1))
			goto err;
		if (text == cold) {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jmp8\t.Lfull_retry\n"));
			gen86_jmp8_offset(p_pc(text), -1);
			rel = host_section_newhostrel(text);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(text) - 1);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			ASSERT(text_Lfull_retry);
			rel->hr_value.rv_sym = text_Lfull_retry;
		} else {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jmpl\t.Lfull_retry\n"));
			gen86_jmpl_offset(p_pc(cold), -4);
			rel = host_section_newhostrel(cold);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(cold) - 4);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = text_Lfull_retry;
		}
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
		if (text != cold)
			gen86_printf(sect, ".section .text\n");
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		if unlikely(fg_settext(self, text))
			goto err;
	}
	if (text_Ldone != NULL) {
		_host_symbol_setsect(text_Ldone, text);
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "Ldone:\n"));
	}
	return 0;
err:
	return -1;
#undef LOCAL_loc_isreg
#undef LOCAL_loc_regno
#undef LOCAL_loc_reg86
#undef LOCAL_loc_regnam
#undef LOCAL_loc_regbas
#undef LOCAL_loc_regoff
#undef LOCAL_loc_const
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_write_impl2(struct fungen *__restrict self,
                            struct memloc const *__restrict loc,
                            host_regno_t tempreg) {
#define LOCAL_loc_isreg()  (loc->ml_adr.ma_typ == MEMADR_TYPE_HREG)
#define LOCAL_loc_regno()  (loc->ml_adr.ma_reg)
#define LOCAL_loc_reg86()  (gen86_registers[loc->ml_adr.ma_reg])
#define LOCAL_loc_regnam() gen86_regname(loc->ml_adr.ma_reg)
#define LOCAL_loc_regbas() memloc_getoff(loc)
#define LOCAL_loc_regoff() (memloc_getoff(loc) + (ptrdiff_t)offsetof(Dee_atomic_rwlock_t, arw_lock))
#define LOCAL_loc_const()  ((intptr_t)(uintptr_t)&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)
	struct host_symbol *text_Lfull_retry;
	struct host_section *text = fg_gettext(self);
	if unlikely(host_section_reqx86(text, 4))
		goto err;

	/* >> .Lfull_retry:
	 * >>     xorP  %Pax, %Pax
	 * >>     movP  $-1, %tempreg
	 * >>     lock  cmpxchgP %tempreg, <lock->arw_lock>
	 * >> #if OSIZE
	 * >>     jnz8  .Lfull_retry
	 * >> #else // OSIZE
	 * >>
	 * >> #if text == cold
	 * >>     jz8   .Ldone
	 * >> #else
	 * >>     jnzl  .Lpause_and_full_retry
	 * >> .section .cold
	 * >> .Lpause_and_full_retry:
	 * >> #endif
	 * >>
	 * >>     PAUSE_OR_SCHED_YIELD
	 * >> #if text == cold
	 * >>     jmp8  .Lfull_retry
	 * >> .Ldone:
	 * >> #else
	 * >>     jmpl  .Lfull_retry
	 * >> .section .text
	 * >> #endif
	 * >>
	 * >> #endif // !OSIZE */
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, ".Lfull_retry:\n"));
	text_Lfull_retry = fg_newsym(self);
	if unlikely(!text_Lfull_retry)
		goto err;
	_host_symbol_setsect(text_Lfull_retry, text);

	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "xor" Plq "\t%%" Per "ax, %%" Per "ax\n"));
	gen86_xorP_r_r(p_pc(text), GEN86_R_PAX, GEN86_R_PAX);
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "mov" Plq "\t$-1, %s\n", gen86_regname(tempreg)));
	gen86_movP_imm_r(p_pc(text), -1, gen86_registers[tempreg]);
	if (LOCAL_loc_isreg()) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "lock cmpxchg" Plq "\t%s, arw_lock%+Id(%s)\n",
		                                     gen86_regname(tempreg), LOCAL_loc_regbas(), LOCAL_loc_regnam()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_db, gen86_registers[tempreg],
		                     LOCAL_loc_regoff(), LOCAL_loc_reg86());
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "lock cmpxchg" Plq "\t%s, %#Ix\n", gen86_regname(tempreg), LOCAL_loc_const()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_d, gen86_registers[tempreg], LOCAL_loc_const());
	}

	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) {
		struct host_reloc *rel;
		/* jnz8  .Lfull_retry */
		IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jnz8\t.Lfull_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_NZ, -1);
		rel = host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		ASSERT(text_Lfull_retry);
		rel->hr_value.rv_sym = text_Lfull_retry;
	} else {
		struct host_symbol *text_Ldone;
		struct host_reloc *rel;
		struct host_section *cold = fg_getcold(self);
		if unlikely(!cold)
			goto err;
		text_Ldone = NULL;
		if (cold == text) {
			text_Ldone = fg_newsym(self);
			if unlikely(!text_Ldone)
				goto err;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jz8\t.Ldone\n"));
			gen86_jcc8_offset(p_pc(text), GEN86_CC_Z, -1);
			rel = host_section_newhostrel(text);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(text) - 1);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = text_Ldone;
		} else {
			struct host_symbol *cold_Lpause_and_full_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jnzl\t.Lpause_and_full_retry\n"));
			cold_Lpause_and_full_retry = fg_newsym(self);
			if unlikely(!cold_Lpause_and_full_retry)
				goto err;
			gen86_jccl_offset(p_pc(text), GEN86_CC_NZ, -4);
			rel = host_section_newhostrel(text);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(text) - 4);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, ".section .cold\n"));
			if unlikely(fg_settext(self, cold))
				goto err;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, ".Lpause_and_full_retry:\n"));
			_host_symbol_setsect(cold_Lpause_and_full_retry, cold);
		}
		if unlikely(_fungen_gpause_or_yield(self,
		                                    LOCAL_loc_isreg()
		                                    ? LOCAL_loc_regno()
		                                    : HOST_REGNO_COUNT))
			goto err;
		if unlikely(host_section_reqx86(cold, 1))
			goto err;
		if (text == cold) {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jmp8\t.Lfull_retry\n"));
			gen86_jmp8_offset(p_pc(cold), -1);
			rel = host_section_newhostrel(cold);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(cold) - 1);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = text_Lfull_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, ".Ldone:\n"));
			ASSERT(text_Ldone);
			_host_symbol_setsect(text_Ldone, text);
		} else {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "jmpl\t.Lfull_retry\n"));
			gen86_jmpl_offset(p_pc(cold), -4);
			rel = host_section_newhostrel(cold);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(cold) - 4);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = text_Lfull_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, ".section .text\n"));
			if unlikely(fg_settext(self, text))
				goto err;
			ASSERT(!text_Ldone);
		}
	}
	return 0;
err:
	return -1;
#undef LOCAL_loc_isreg
#undef LOCAL_loc_regno
#undef LOCAL_loc_reg86
#undef LOCAL_loc_regnam
#undef LOCAL_loc_regbas
#undef LOCAL_loc_regoff
#undef LOCAL_loc_const
}



/* @return: 1 : %Pax was not pushed
 * @return: 0 : %Pax was pushed 
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_gpush_Pax_if_used(struct fungen *__restrict self) {
	struct memstate *state = self->fg_state;
	if (state->ms_rinuse[HOST_REGNO_PAX] == 0)
		return 1; /* Not in use */
	return fg_ghstack_pushreg(self, HOST_REGNO_PAX);
}
#define _fungen_gpop_Pax(self) \
	fg_ghstack_popreg(self, HOST_REGNO_PAX)

#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
PRIVATE NONNULL((1)) void DCALL
log_compact_rwlock_register_preserve_list(struct fungen *__restrict self) {
	struct memstate *state = self->fg_state;
	host_regno_t save_regno;
	bool is_first = true;

	/* Save registers. */
	for (save_regno = 0; save_regno < HOST_REGNO_COUNT; ++save_regno) {
		if (!memstate_hregs_isused(state, save_regno))
			continue;
		Dee_DPRINT(", ");
		if (is_first) {
			Dee_DPRINT("{");
			is_first = false;
		}
		Dee_DPRINT(gen86_regname(save_regno));
	}
	if (!is_first)
		Dee_DPRINT("}");
}
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */

/* Make sure that `loc' doesn't use %Pax */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_fungen_gloc_no_Pax(struct fungen *__restrict self,
                    struct memloc *__restrict loc) {
	if (loc->ml_adr.ma_typ == MEMADR_TYPE_HREG &&
	    loc->ml_adr.ma_reg == HOST_REGNO_PAX) {
		/* We explicity *need* to use a register other than `%Pax' here! */
		host_regno_t not_these[2], lockreg;
		struct memstate *state;
		not_these[0] = HOST_REGNO_PAX;
		not_these[1] = HOST_REGNO_COUNT;
		lockreg = fg_gallocreg(self, not_these);
		if unlikely(lockreg >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_reg2reg(self, HOST_REGNO_PAX, lockreg))
			goto err;
		state = self->fg_state;
		loc->ml_adr.ma_reg = lockreg;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_read_impl(struct fungen *__restrict self,
                          struct memloc const *__restrict loc) {
	host_regno_t tempreg, not_these[3];
	struct memloc usedloc = *loc;
	int Pax_pushed;
	ASSERT(usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG ||
	       usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST);
	if unlikely(_fungen_gloc_no_Pax(self, &usedloc))
		goto err;
	not_these[0] = HOST_REGNO_PAX;
	not_these[1] = HOST_REGNO_COUNT;
	not_these[2] = HOST_REGNO_COUNT;
	if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG)
		not_these[1] = usedloc.ml_adr.ma_reg;
	tempreg = fg_gallocreg(self, not_these);
	if unlikely(tempreg >= HOST_REGNO_COUNT)
		goto err;
	Pax_pushed = _fungen_gpush_Pax_if_used(self);
	if unlikely(Pax_pushed < 0)
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST) {
		gen86_printf(fg_gettext(self), "rwlock_read\t%#Ix", usedloc.ml_adr.ma_val.v_const);
	} else {
		gen86_printf(fg_gettext(self), "rwlock_read\t%Id(%s)", memloc_getoff(&usedloc), gen86_regname(usedloc.ml_adr.ma_reg));
	}
	log_compact_rwlock_register_preserve_list(self);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	if unlikely(_fungen_grwlock_read_impl2(self, &usedloc, tempreg))
		goto err;
	if (Pax_pushed == 0)
		return _fungen_gpop_Pax(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_write_impl(struct fungen *__restrict self,
                           struct memloc const *__restrict loc) {
	host_regno_t tempreg;
	struct memloc usedloc = *loc;
	int Pax_pushed;
	ASSERT(usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG ||
	       usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST);
	if unlikely(_fungen_gloc_no_Pax(self, &usedloc))
		goto err;
	{
		host_regno_t not_these[3];
		not_these[0] = HOST_REGNO_PAX;
		not_these[1] = HOST_REGNO_COUNT;
		not_these[2] = HOST_REGNO_COUNT;
		if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG)
			not_these[1] = usedloc.ml_adr.ma_reg;
		tempreg = fg_gallocreg(self, not_these);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			goto err;
	}
	Pax_pushed = _fungen_gpush_Pax_if_used(self);
	if unlikely(Pax_pushed < 0)
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST) {
		gen86_printf(fg_gettext(self), "rwlock_write\t%#Ix", usedloc.ml_adr.ma_val.v_const);
	} else {
		gen86_printf(fg_gettext(self), "rwlock_write\t%Id(%s)", memloc_getoff(&usedloc), gen86_regname(usedloc.ml_adr.ma_reg));
	}
	log_compact_rwlock_register_preserve_list(self);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	if unlikely(_fungen_grwlock_write_impl2(self, &usedloc, tempreg))
		goto err;
	if (Pax_pushed == 0)
		return _fungen_gpop_Pax(self);
	return 0;
err:
	return -1;
}

/* Controls for operating with R/W-locks (as needed for accessing global/extern variables) */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_endread_const(struct fungen *__restrict self,
                              Dee_atomic_rwlock_t *__restrict lock) {
	/* >> lock decP <lock->arw_lock> */
	struct host_section *sect = fg_gettext(self);
	ASSERT(fit32(&lock->arw_lock));
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf(sect, "rwlock_endread\t%#Ix\n", lock);
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	gen86_printf(sect, "lock dec" Plq "\t%#Ix\n", &lock->arw_lock);
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(sect));
	gen86_decP_mod(p_pc(sect), gen86_modrm_d, (intptr_t)(uintptr_t)&lock->arw_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_endwrite_const(struct fungen *__restrict self,
                               Dee_atomic_rwlock_t *__restrict lock) {
	/* >> movP  $0, <lock->arw_lock> */
	struct host_section *sect = fg_gettext(self);
	ASSERT(fit32(&lock->arw_lock));
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf(sect, "rwlock_endwrite\t%#Ix\n", lock);
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(sect, "mov" Plq "\t$0, %#Ix\n", &lock->arw_lock));
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	gen86_movP_imm_d(p_pc(sect), 0, (intptr_t)(uintptr_t)&lock->arw_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_grwlock_endread_regx(struct fungen *__restrict self,
                             host_regno_t regno, ptrdiff_t val_delta) {
	/* >> lock decP <lock->arw_lock> */
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf(sect, "rwlock_endread\t%Id(%s)\n", val_delta, gen86_regname(regno));
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	gen86_printf(sect, "lock dec" Plq "\tarw_lock%+Id(%s)\n", val_delta, gen86_regname(regno));
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	val_delta += offsetof(Dee_atomic_rwlock_t, arw_lock);
	gen86_decP_mod(p_pc(sect), gen86_modrm_db, val_delta, gen86_registers[regno]);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_fungen_grwlock_endwrite_regx(struct fungen *__restrict self,
                              host_regno_t regno, ptrdiff_t val_delta) {
	/* >> movP  $0, <lock->arw_lock> */
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf(sect, "rwlock_endwrite\t%Id(%s)\n", val_delta, gen86_regname(regno));
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	gen86_printf(sect, "mov" Plq "\t$0, arw_lock%+Id(%s)\n", val_delta, gen86_regname(regno));
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	val_delta += offsetof(Dee_atomic_rwlock_t, arw_lock);
	gen86_movP_imm_db(p_pc(sect), 0, val_delta, gen86_registers[regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_read(struct fungen *__restrict self,
                     struct memloc const *__restrict loc) {
	struct memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (!fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)) {
	default:
			if unlikely(fg_gasreg(self, loc, &regloc, NULL))
				goto err;
			loc = &regloc;
		}
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _fungen_grwlock_read_impl(self, loc);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_write(struct fungen *__restrict self,
                      struct memloc const *__restrict loc) {
	struct memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (!fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)) {
	default:
			if unlikely(fg_gasreg(self, loc, &regloc, NULL))
				goto err;
			loc = &regloc;
		}
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _fungen_grwlock_write_impl(self, loc);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_endread(struct fungen *__restrict self,
                        struct memloc const *__restrict loc) {
	struct memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock))
			return _fungen_grwlock_endread_const(self, (Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const);
		ATTR_FALLTHROUGH
	default:
		if unlikely(fg_gasreg(self, loc, &regloc, NULL))
			goto err;
		loc = &regloc;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _fungen_grwlock_endread_regx(self, loc->ml_adr.ma_reg, memloc_getoff(loc));
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_fungen_grwlock_endwrite(struct fungen *__restrict self,
                         struct memloc const *__restrict loc) {
	struct memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock))
			return _fungen_grwlock_endwrite_const(self, (Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const);
		ATTR_FALLTHROUGH
	default:
		if unlikely(fg_gasreg(self, loc, &regloc, NULL))
			goto err;
		loc = &regloc;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _fungen_grwlock_endwrite_regx(self, loc->ml_adr.ma_reg, memloc_getoff(loc));
	}
	__builtin_unreachable();
err:
	return -1;
}
#endif /* !CONFIG_NO_THREADS */




/* Allocate/deallocate memory from the host stack.
 * If stack memory gets allocated, zero-initialize it. */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_ghstack_adjust(struct fungen *__restrict self,
                       ptrdiff_t sp_delta) {
	struct host_section *sect = fg_gettext(self);
	ASSERTF(fit32(sp_delta),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if (sp_delta > 0) {
		/* Release stack memory. */
		ASSERT(IS_ALIGNED((uintptr_t)sp_delta, HOST_SIZEOF_POINTER));
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		gen86_printf(sect, "add" Plq "\t$%Id, %%" Per "sp\n", sp_delta);
		gen86_addP_imm_r(p_pc(sect), sp_delta, GEN86_R_PSP);
	} else if (sp_delta < 0) {
		/* Acquire stack memory. */
		ASSERT(IS_ALIGNED((uintptr_t)(-sp_delta), HOST_SIZEOF_POINTER));
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		gen86_printf(sect, "sub" Plq "\t$%Id, %%" Per "sp\n", -sp_delta);
		gen86_subP_imm_r(p_pc(sect), -sp_delta, GEN86_R_PSP);
	}
	return fg_gadjust_cfa_offset(self, -sp_delta);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_ghstack_pushreg(struct fungen *__restrict self,
                        host_regno_t src_regno) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "push" Plq "\t%s\n", gen86_regname(src_regno));
	gen86_pushP_r(p_pc(sect), gen86_registers[src_regno]);
	return fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_ghstack_pushregind(struct fungen *__restrict self,
                           host_regno_t src_regno, ptrdiff_t src_delta) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(_fungen_gadjust_reg_fit32(self, &src_regno, &src_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "push" Plq "\t%Id(%s)\n", src_delta, gen86_regname(src_regno));
	gen86_pushP_mod(p_pc(sect), gen86_modrm_db, src_delta, gen86_registers[src_regno]);
	return fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_ghstack_pushconst(struct fungen *__restrict self,
                          void const *value) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "push" Plq "\t$%#I32x\n", (uint32_t)(uintptr_t)value);
	gen86_pushP_imm(p_pc(sect), (int32_t)(uint32_t)(uintptr_t)value);
#ifndef fit32_IS_1
	if (!fit32(value)) {
		/* Must manually write the high 32-bit of "value" to their proper place. */
		uint32_t hi32 = (uint32_t)((uint64_t)value >> 32);
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		gen86_printf(sect, "movl\t$%#I32x, 4(%%" Per "sp)\n", hi32);
		gen86_movl_imm_db(p_pc(sect), hi32, 4, GEN86_R_PSP);
	}
#endif /* fit32_IS_1 */
	return fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
err:
	return -1;
}

/* Pushes the address of `(self)->fg_state->ms_host_cfa_offset' (as it was before the push) */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_ghstack_pushhstack_at_cfa_boundary_np(struct fungen *__restrict self) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "push" Plq "\t%%" Per "sp\n");
	gen86_pushP_r(p_pc(sect), GEN86_R_PSP);
	return fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_ghstack_pushhstackind(struct fungen *__restrict self,
                              ptrdiff_t sp_offset) {
	struct host_section *sect = fg_gettext(self);
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "push" Plq "\t%Id(%%" Per "sp)\n", sp_offset);
	gen86_pushP_mod(p_pc(sect), gen86_modrm_db, sp_offset, GEN86_R_PSP);
	return fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_ghstack_popreg(struct fungen *__restrict self,
                       host_regno_t dst_regno) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "pop" Plq "\t%s\n", gen86_regname(dst_regno));
	gen86_popP_r(p_pc(sect), gen86_registers[dst_regno]);
	return fg_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_reg2hstackind(struct host_section *__restrict self,
                                 host_regno_t src_regno, ptrdiff_t sp_offset) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "mov" Plq "\t%s, %Id(%%" Per "sp)\n", gen86_regname(src_regno), sp_offset);
	gen86_movP_r_db(p_pc(self), gen86_registers[src_regno], sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_hstack2reg(struct host_section *__restrict self,
                              ptrdiff_t sp_offset, host_regno_t dst_regno) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "lea" Plq "\t%Id(%%" Per "sp), %s\n", sp_offset, gen86_regname(dst_regno));
	gen86_leaP_db_r(p_pc(self), sp_offset, GEN86_R_PSP, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_hstackind2reg(struct host_section *__restrict self,
                                 ptrdiff_t sp_offset, host_regno_t dst_regno) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "mov" Plq "\t%Id(%%" Per "sp), %s\n", sp_offset, gen86_regname(dst_regno));
	gen86_movP_db_r(p_pc(self), sp_offset, GEN86_R_PSP, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_const2reg(struct host_section *__restrict self,
                             void const *value, host_regno_t dst_regno) {
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	if (value == NULL) {
		gen86_printf(self, "xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	} else {
		gen86_printf(self, "mov" Plq "\t$%s, %s\n", gen86_addrname(value), gen86_regname(dst_regno));
		/* NOTE: This automatically does `movabs' on x86_64! */
		gen86_movP_imm_r(p_pc(self), (intptr_t)(uintptr_t)value, gen86_registers[dst_regno]);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gmov_const2regind(struct fungen *__restrict self, void const *value,
                          host_regno_t dst_regno, ptrdiff_t dst_delta) {
	struct host_section *sect = fg_gettext(self);
#ifdef _fungen_gmov_const2regind_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _fungen_gmov_const2regind_MAYFAIL */
	if unlikely(_fungen_gadjust_reg_fit32(self, &dst_regno, &dst_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "mov" Plq "\t$%s, %Id(%s)\n", gen86_addrname(value), dst_delta, gen86_regname(dst_regno));
	gen86_movP_imm_db(p_pc(sect), (intptr_t)(uintptr_t)value, dst_delta, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_const2hstackind(struct host_section *__restrict self,
                                   void const *value, ptrdiff_t sp_offset) {
#ifdef _host_section_gmov_const2hstackind_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _host_section_gmov_const2hstackind_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "mov" Plq "\t$%s, %Id(%%" Per "sp)\n", gen86_addrname(value), sp_offset);
	gen86_movP_imm_db(p_pc(self), (intptr_t)(uintptr_t)value, sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_const2constind(struct host_section *__restrict self,
                                  void const *value, void const **p_value) {
#ifdef _host_section_gmov_const2constind_MAYFAIL
	if (!fit32(value))
		return 1;
	if (!fit32(p_value))
		return 2;
#endif /* _host_section_gmov_const2constind_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "mov" Plq "\t$%s, %#Ix\n", gen86_addrname(value), (intptr_t)(uintptr_t)p_value);
	gen86_movP_imm_d(p_pc(self), (intptr_t)(uintptr_t)value, (intptr_t)(uintptr_t)p_value);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_regx2reg(struct host_section *__restrict self,
                            host_regno_t src_regno, ptrdiff_t src_delta,
                            host_regno_t dst_regno) {
	if unlikely(src_regno == dst_regno && src_delta == 0)
		return 0;
	if (src_regno == dst_regno)
		return _fungen_gadjust_reg_delta(self, NULL, src_regno, src_delta, true);
#ifndef fit32_IS_1
	if (!fit32(src_delta)) {
		if unlikely(host_section_reqx86(self, 2))
			goto err;
		gen86_printf(self, "movabs" Plq "\t%Id, %s\n", src_delta, gen86_regname(dst_regno));
		gen86_movabs_imm_r(p_pc(self), src_delta, gen86_registers[dst_regno]);
		gen86_printf(self, "add" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_addP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
	} else
#endif /* !fit32_IS_1 */
	{
		if unlikely(host_section_reqx86(self, 1))
			goto err;
		gen86_printf(self, "lea" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_db_r(p_pc(self), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_constind2reg(struct host_section *__restrict self,
                                void const **p_value, host_regno_t dst_regno) {
#ifdef _host_section_gmov_constind2reg_MAYFAIL
	if (!fit32(p_value))
		return 1;
#endif /* _host_section_gmov_constind2reg_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "mov" Plq "\t%#Ix, %s\n", (intptr_t)(uintptr_t)p_value, gen86_regname(dst_regno));
	gen86_movP_d_r(p_pc(self), (intptr_t)(uintptr_t)p_value, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmov_reg2constind(struct host_section *__restrict self,
                                host_regno_t src_regno, void const **p_value) {
#ifdef _host_section_gmov_reg2constind_MAYFAIL
	if (!fit32(p_value))
		return 1;
#endif /* _host_section_gmov_reg2constind_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "mov" Plq "\t%s, %#Ix\n", gen86_regname(src_regno), (intptr_t)(uintptr_t)p_value);
	gen86_movP_r_d(p_pc(self), gen86_registers[src_regno], (intptr_t)(uintptr_t)p_value);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gmov_regind2reg(struct fungen *__restrict self,
                        host_regno_t src_regno, ptrdiff_t src_delta,
                        host_regno_t dst_regno) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(_fungen_gadjust_reg_fit32(self, &src_regno, &src_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "mov" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
	gen86_movP_db_r(p_pc(sect), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gmov_reg2regind(struct fungen *__restrict self,
                        host_regno_t src_regno,
                        host_regno_t dst_regno, ptrdiff_t dst_delta) {
	struct host_section *sect = fg_gettext(self);
#ifndef fit32_IS_1
	if unlikely(src_regno == dst_regno && !fit32(dst_delta)) {
		/* Needs special handling when !fit32 */
		ptrdiff_t adj_delta;
		host_regno_t new_dst_regno;
		host_regno_t not_these[2];
		/* TODO: Check if there is a known register equivalence of `dst_regno'
		 *       with a value-delta that causes the final `val_delta' to
		 *       fit into 32 bit */
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGNO_COUNT;
		new_dst_regno = fg_gallocreg(self, not_these);
		if unlikely(new_dst_regno >= HOST_REGNO_COUNT)
			goto err;
		if (dst_delta < INT32_MIN) {
			adj_delta = dst_delta - INT32_MIN;
		} else if (dst_delta > INT32_MAX) {
			adj_delta = dst_delta - INT32_MAX;
		} else {
			adj_delta = 0;
		}
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(dst_regno), gen86_regname(new_dst_regno));
		gen86_leaP_db_r(p_pc(sect), adj_delta, gen86_registers[dst_regno], gen86_registers[new_dst_regno]);
		dst_regno = new_dst_regno;
	}
#endif /* !fit32_IS_1 */
	if unlikely(_fungen_gadjust_reg_fit32(self, &dst_regno, &dst_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "mov" Plq "\t%s, %Id(%s)\n", gen86_regname(src_regno), dst_delta, gen86_regname(dst_regno));
	gen86_movP_r_db(p_pc(sect), gen86_registers[src_regno], dst_delta, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gret(struct fungen *__restrict self) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
#ifdef HOSTASM_X86_64
	gen86_printf(sect, "ret" Plq "\n");
	gen86_retP(p_pc(sect));
#else /* HOSTASM_X86_64 */
	/* Special handling needed because DCALL is STDCALL on i386 */
	{
		host_cc_t cc = self->fg_assembler->fa_cc;
		size_t ret_imm = cc & HOST_CC_F_TUPLE ? 4 : 8;
		if (cc & HOST_CC_F_THIS)
			ret_imm += 4;
		if (cc & HOST_CC_F_KW)
			ret_imm += 4;
		gen86_printf(sect, "ret" Plq "\t$%Iu\n", ret_imm);
		gen86_retP_imm(p_pc(sect), ret_imm);
	}
#endif /* !HOSTASM_X86_64 */
	return 0;
err:
	return -1;
}




/* Helpers for transforming locations into deemon boolean objects. */

STATIC_ASSERT(GMORPHBOOL_CC_EQ == 0); /* Compare: "==" */
STATIC_ASSERT(GMORPHBOOL_CC_NE == 1); /* Compare: "!=" */
STATIC_ASSERT(GMORPHBOOL_CC_LO == 2); /* Compare: "<" (signed) */
STATIC_ASSERT(GMORPHBOOL_CC_GR == 3); /* Compare: ">" (signed) */
PRIVATE uint8_t const gmorphbool_cc_86[] = {
	/* [GMORPHBOOL_CC_EQ] = */ GEN86_CC_E,  /* Compare: "==" */
	/* [GMORPHBOOL_CC_NE] = */ GEN86_CC_NE, /* Compare: "!=" */
	/* [GMORPHBOOL_CC_LO] = */ GEN86_CC_L,  /* Compare: "<" (signed) */
	/* [GMORPHBOOL_CC_GR] = */ GEN86_CC_G,  /* Compare: ">" (signed) */
};

/* dst_regno = (src_regno + src_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gmorph_regx2reg01(struct fungen *__restrict self,
                          host_regno_t src_regno, ptrdiff_t src_delta,
                          unsigned int cmp, host_regno_t dst_regno) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(_fungen_gadjust_reg_fit32(self, &src_regno, &src_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 3))
		goto err;
	if (src_regno != dst_regno) {
		gen86_printf(sect, "xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(sect), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	}
	if (src_delta == 0 && (cmp == GMORPHBOOL_CC_EQ || cmp == GMORPHBOOL_CC_NE)) {
		gen86_printf(sect, "test" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(src_regno));
		gen86_testP_r_r(p_pc(sect), gen86_registers[src_regno], gen86_registers[src_regno]);
	} else {
		gen86_printf(sect, "cmp" Plq "\t$%Id, %s\n", -src_delta, gen86_regname(src_regno));
		gen86_cmpP_imm_r(p_pc(sect), -src_delta, gen86_registers[src_regno]);
	}
	if (src_regno == dst_regno) {
		gen86_printf(sect, "mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(sect), 0, gen86_registers[dst_regno]);
	}
	gen86_printf(sect, "set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(sect), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (src_regno + src_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmorph_regxCreg2reg01(struct host_section *__restrict self, host_regno_t src_regno,
                                    ptrdiff_t src_delta, unsigned int cmp, host_regno_t rhs_regno,
                                    host_regno_t dst_regno) {
	if (src_regno == rhs_regno) {
		uintptr_t value;
		switch (cmp) {
		case GMORPHBOOL_CC_EQ: value = src_delta == 0; break;
		case GMORPHBOOL_CC_NE: value = src_delta != 0; break;
		case GMORPHBOOL_CC_LO: value = src_delta < 0; break;
		case GMORPHBOOL_CC_GR: value = src_delta > 0; break;
		default: __builtin_unreachable();
		}
		return _host_section_gmov_const2reg(self, (void const *)(uintptr_t)(value ? 1 : 0), dst_regno);
	} else if (src_regno != dst_regno && rhs_regno != dst_regno && src_delta == 0) {
		if unlikely(host_section_reqx86(self, 3))
			goto err;
		gen86_printf(self, "xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
		gen86_printf(self, "cmp" Plq "\t%s, %s\n", gen86_regname(rhs_regno), gen86_regname(src_regno));
		gen86_cmpP_r_r(p_pc(self), gen86_registers[rhs_regno], gen86_registers[src_regno]);
	} else {
		if unlikely(host_section_reqx86(self, (size_t)(src_delta ? 4 : 3)))
			goto err;
		if (src_delta != 0) {
			gen86_printf(self, "lea" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
			gen86_leaP_db_r(p_pc(self), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
			src_regno = dst_regno;
		}
		gen86_printf(self, "cmp" Plq "\t%s, %s\n", gen86_regname(rhs_regno), gen86_regname(src_regno));
		gen86_cmpP_r_r(p_pc(self), gen86_registers[rhs_regno], gen86_registers[src_regno]);
		gen86_printf(self, "mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(self), 0, gen86_registers[dst_regno]);
	}
	gen86_printf(self, "set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(self), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (*(src_regno86 + ind_delta) + val_delta) <CMP> 0 ? 1 : 0; */
PRIVATE WUNUSED NONNULL((1)) int DCALL
_host_section_gmorph_reg86ind2reg01_impl(struct host_section *__restrict self,
                                         uint8_t src_regno86, ptrdiff_t ind_delta,
                                         ptrdiff_t val_delta, unsigned int cmp,
                                         host_regno_t dst_regno) {
	if unlikely(host_section_reqx86(self, 3))
		goto err;
	if (src_regno86 != gen86_registers[dst_regno]) {
		gen86_printf(self, "xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	}
	gen86_printf(self, "cmp" Plq "\t$%Id, %Id(%s)\n", -val_delta, ind_delta, gen86_regnames[src_regno86]);
	gen86_cmpP_imm_mod(p_pc(self), gen86_modrm_db, -val_delta, ind_delta, src_regno86);
	if (src_regno86 == gen86_registers[dst_regno]) {
		gen86_printf(self, "mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(self), 0, gen86_registers[dst_regno]);
	}
	gen86_printf(self, "set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(self), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (*(src_regno86 + ind_delta) + val_delta) <CMP> rhs_regno ? 1 : 0; */
PRIVATE WUNUSED NONNULL((1)) int DCALL
_host_section_gmorph_reg86indCreg2reg01_impl(struct fungen *__restrict self,
                                             uint8_t src_regno86, ptrdiff_t ind_delta,
                                             ptrdiff_t val_delta, unsigned int cmp,
                                             host_regno_t rhs_regno,
                                             host_regno_t dst_regno) {
	struct host_section *sect = fg_gettext(self);
	ASSERT(fit32(ind_delta));
	ASSERT(fit32(-val_delta));
	if unlikely(host_section_reqx86(sect, 4))
		goto err;
	if (src_regno86 != gen86_registers[dst_regno] && rhs_regno != dst_regno && val_delta == 0) {
		gen86_printf(sect, "xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(sect), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	}
	if (val_delta != 0) {
		if (rhs_regno == dst_regno && gen86_registers[dst_regno] == src_regno86) {
			/* Special case: %reg = (*(%reg + ind_delta) <CMP> (%reg - val_delta)) ? 1 : 0
			 * For this case, we actually need a temporary register. */
			host_regno_t not_these[2];
			host_regno_t tempreg;
			bool must_pop_tempreg = false;
			not_these[0] = dst_regno;
			not_these[1] = HOST_REGNO_COUNT;
			tempreg = memstate_hregs_find_unused_ex(self->fg_state, not_these);
			if (tempreg >= HOST_REGNO_COUNT) {
				if unlikely(host_section_reqx86(sect, 6))
					goto err;
				tempreg = 0;
				if (tempreg == rhs_regno)
					++tempreg;
				must_pop_tempreg = true;
				gen86_printf(sect, "push" Plq "\t%s\n", gen86_regname(tempreg));
				gen86_pushP_r(p_pc(sect), gen86_registers[tempreg]);
			}
			gen86_printf(sect, "lea" Plq "\t%Id(%s), %s\n", -val_delta, gen86_regname(rhs_regno), gen86_regname(tempreg));
			gen86_leaP_db_r(p_pc(sect), -val_delta, gen86_registers[rhs_regno], gen86_registers[tempreg]);
			gen86_printf(sect, "cmp" Plq "\t%s, %Id(%s)\n", gen86_regname(tempreg), ind_delta, gen86_regnames[src_regno86]);
			gen86_cmpP_r_mod(p_pc(sect), gen86_modrm_db, tempreg, ind_delta, src_regno86);
			if (must_pop_tempreg) {
				gen86_printf(sect, "pop" Plq "\t%s\n", gen86_regname(tempreg));
				gen86_popP_r(p_pc(sect), gen86_registers[tempreg]);
			}
			goto mov0_to_dst_and_setcc;
		}
		gen86_printf(sect, "lea" Plq "\t%Id(%s), %s\n", -val_delta, gen86_regname(rhs_regno), gen86_regname(dst_regno));
		gen86_leaP_db_r(p_pc(sect), -val_delta, gen86_registers[rhs_regno], gen86_registers[dst_regno]);
		rhs_regno = dst_regno;
	}
	gen86_printf(sect, "cmp" Plq "\t%s, %Id(%s)\n", gen86_regname(rhs_regno), ind_delta, gen86_regnames[src_regno86]);
	gen86_cmpP_r_mod(p_pc(sect), gen86_modrm_db, rhs_regno, ind_delta, src_regno86);
	if (src_regno86 == gen86_registers[dst_regno] || rhs_regno == dst_regno || val_delta != 0) {
mov0_to_dst_and_setcc:
		gen86_printf(sect, "mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(sect), 0, gen86_registers[dst_regno]);
	}
	gen86_printf(sect, "set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(sect), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (*(src_regno + ind_delta) + val_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gmorph_regind2reg01(struct fungen *__restrict self,
                            host_regno_t src_regno, ptrdiff_t ind_delta,
                            ptrdiff_t val_delta, unsigned int cmp,
                            host_regno_t dst_regno) {
#ifdef _fungen_gmorph_regind2reg01_MAYFAIL
	if (!fit32(-val_delta))
		return 1;
#endif /* _fungen_gmorph_regind2reg01_MAYFAIL */
	if unlikely(_fungen_gadjust_reg_fit32(self, &src_regno, &ind_delta))
		return -1;
	return _host_section_gmorph_reg86ind2reg01_impl(fg_gettext(self), gen86_registers[src_regno],
	                                  ind_delta, val_delta, cmp, dst_regno);
}

/* dst_regno = (*(src_regno + ind_delta) + val_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gmorph_regindCreg2reg01(struct fungen *__restrict self,
                                host_regno_t src_regno, ptrdiff_t ind_delta,
                                ptrdiff_t val_delta, unsigned int cmp,
                                host_regno_t rhs_regno, host_regno_t dst_regno) {
#ifndef fit32_IS_1
	if (!fit32(ind_delta)) {
		ptrdiff_t new_ind_delta = ind_delta;
		if unlikely(_fungen_gadjust_reg_fit32_impl(self, &src_regno, &new_ind_delta))
			return -1;
		if (src_regno == rhs_regno) /* Propagate adjustment delta to val_delta */
			val_delta += ind_delta - new_ind_delta;
		ind_delta = new_ind_delta;
	}
	if (!fit32(-val_delta)) {
		val_delta = -val_delta;
		if (src_regno == rhs_regno) {
			/* Must use a temporary register so adjustment doesn't affect "src_regno" */
			struct host_section *sect = fg_gettext(self);
			host_regno_t new_rhs_regno;
			host_regno_t not_these[2];
			ptrdiff_t adj_delta;
			not_these[0] = rhs_regno;
			not_these[1] = HOST_REGNO_COUNT;
			new_rhs_regno = fg_gallocreg(self, not_these);
			if unlikely(new_rhs_regno >= HOST_REGNO_COUNT)
				return -1;
			if unlikely(host_section_reqx86(sect, 1))
				return -1;
			adj_delta = val_delta;
			if (adj_delta < INT32_MIN) {
				adj_delta = INT32_MIN;
			} else if (adj_delta > INT32_MAX) {
				adj_delta = INT32_MAX;
			}
			HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(rhs_regno), gen86_regname(new_rhs_regno));
			gen86_leaP_db_r(p_pc(sect), val_delta, gen86_registers[rhs_regno], gen86_registers[new_rhs_regno]);
			val_delta += adj_delta;
			rhs_regno = new_rhs_regno;
		}
		if unlikely(_fungen_gadjust_reg_fit32(self, &rhs_regno, &val_delta))
			return -1;
		val_delta = -val_delta;
	}
#endif /* !fit32_IS_1 */
	return _host_section_gmorph_reg86indCreg2reg01_impl(self, gen86_registers[src_regno], ind_delta,
	                                                    val_delta, cmp, rhs_regno, dst_regno);
}

/* dst_regno = (*(SP + sp_offset) + val_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmorph_hstackind2reg01(struct host_section *__restrict self,
                                     ptrdiff_t sp_offset, ptrdiff_t val_delta,
                                     unsigned int cmp, host_regno_t dst_regno) {
#ifdef _host_section_gmorph_hstackind2reg01_MAYFAIL
	if (!fit32(-val_delta))
		return 1;
#endif /* _host_section_gmorph_hstackind2reg01_MAYFAIL */
	return _host_section_gmorph_reg86ind2reg01_impl(self, GEN86_R_PSP, sp_offset, val_delta, cmp, dst_regno);
}

/* dst_regno = (*(SP + sp_offset) + val_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gmorph_hstackindCreg2reg01(struct fungen *__restrict self,
                                   ptrdiff_t sp_offset, ptrdiff_t val_delta,
                                   unsigned int cmp, host_regno_t rhs_regno,
                                   host_regno_t dst_regno) {
#ifndef fit32_IS_1
	if (!fit32(-val_delta)) {
		val_delta = -val_delta;
		if unlikely(_fungen_gadjust_reg_fit32(self, &rhs_regno, &val_delta))
			return -1;
		val_delta = -val_delta;
	}
#endif /* !fit32_IS_1 */
	return _host_section_gmorph_reg86indCreg2reg01_impl(self, GEN86_R_PSP, sp_offset, val_delta, cmp, rhs_regno, dst_regno);
}


#ifdef HAVE__host_section_gmorph_reg012regbool
/* dst_regno = &Dee_FalseTrue[src_regno + src_delta]; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gmorph_reg012regbool(struct host_section *__restrict self,
                                   host_regno_t src_regno, ptrdiff_t src_delta,
                                   host_regno_t dst_regno) {
	STATIC_ASSERT(sizeof(DeeBoolObject) == 8);
	src_delta *= 8;
	src_delta += (uintptr_t)Dee_FalseTrue;
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(sect, "lea" Plq "\t%Id(,%s,8), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
	gen86_leaP_dis_r(p_pc(self), src_delta, gen86_registers[src_regno], 8, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}
#endif /* HAVE__host_section_gmorph_reg012regbool */


PRIVATE ATTR_PURE WUNUSED host_regno_t DCALL
alloc_unused_reg_for_call_args(struct memloc const *locv, size_t argc) {
	size_t i;
	host_regno_t result;
	bitset_t inuse_bitset[BITSET_LENGTHOF(HOST_REGNO_COUNT)];
	bitset_clearall(inuse_bitset, HOST_REGNO_COUNT);
	for (i = 0; i <= argc; ++i) { /* <= because +1 extra api_function arg */
		struct memloc const *arg = &locv[i];
		if (memloc_hasreg(arg))
			bitset_set(inuse_bitset, memloc_getreg(arg));
	}
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (!bitset_test(inuse_bitset, result))
			break;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Psp0_add_val_offset(struct fungen *__restrict self,
                    ptrdiff_t val_offset) {
	struct host_section *sect = fg_gettext(self);
	struct memadr adr;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "add" Plq "\t$%Id, (%%" Per "sp)\n", val_offset);
	gen86_addP_imm_mod(p_pc(sect), gen86_modrm_b, val_offset, GEN86_R_PSP);
	memadr_init_hstackind(&adr, self->fg_state->ms_host_cfa_offset);
	fg_remember_deltavalue(self, &adr, val_offset);
	return 0;
err:
	return -1;
}

/* Push "loc" onto the HSTACK, and maybe adjust registers */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
push_memloc_maybe_adjust_regs(struct fungen *__restrict self,
                              struct memloc *arg,
                              struct memloc *locv, size_t argc) {
	/* Look at equivalence classes to pick the best fit for the argument value
	 * #1: HSTACKIND (already at the correct CFA, and with rel_valoff==0)
	 * #2: HREG[rel_valoff==0]/HSTACK[sp_offset==0]
	 * #3: CONST (on x86_64: if it fits)
	 * #4: HREGIND[rel_valoff==0]/HSTACKIND[rel_valoff==0]
	 * #5: HREG
	 * #6: Fallback (use `arg' as-is)
	 */
	struct memequiv *eq;
	eq = memequivs_getclassof(&self->fg_state->ms_memequiv, memloc_getadr(arg));
	if (eq != NULL) {
		struct memequiv *iter;
		ptrdiff_t base_off;
		base_off = memloc_getoff(arg);
		base_off -= memloc_getoff(&eq->meq_loc);
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
		iter = eq;
		do {
			if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACKIND &&
			    memloc_hstackind_getcfa(&iter->meq_loc) == (self->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER) &&
			    (memloc_hstackind_getvaloff(&iter->meq_loc) + base_off) == 0)
				goto use_memequiv_iter_as_arg;
		} while ((iter = memequiv_next(iter)) != eq);
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
		iter = eq;
		do {
			if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG &&
			    (memloc_hreg_getvaloff(&iter->meq_loc) + base_off) == 0) {
use_memequiv_iter_as_arg:
				*arg = iter->meq_loc;
				memloc_adjoff(arg, base_off);
				goto use_arg;
			}
		} while ((iter = memequiv_next(iter)) != eq);
		iter = eq;
		do {
			if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_CONST &&
			    fit32((uintptr_t)memloc_const_getaddr(&iter->meq_loc) + base_off))
				goto use_memequiv_iter_as_arg;
		} while ((iter = memequiv_next(iter)) != eq);
		iter = eq;
		do {
			if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREGIND &&
			    (memloc_hregind_getvaloff(&iter->meq_loc) + base_off) == 0)
				goto use_memequiv_iter_as_arg;
			if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACKIND &&
			    (memloc_hstackind_getvaloff(&iter->meq_loc) + base_off) == 0)
				goto use_memequiv_iter_as_arg;
		} while ((iter = memequiv_next(iter)) != eq);
		iter = eq;
		do {
			if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG)
				goto use_memequiv_iter_as_arg;
		} while ((iter = memequiv_next(iter)) != eq);
	}
use_arg:

	/* NOTE: It is important that this function doesn't call `fg_gallocreg()'!
	 *       That is because `alloc_unused_reg_for_call_args()' needs to be used for temporary
	 *       registers */
	switch (memloc_gettyp(arg)) {

	case MEMADR_TYPE_HSTACKIND:
	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t ind_offset;
		host_regno_t tempreg;
		uint8_t gen86_src_regno;
		if (memloc_getoff(arg) == 0) {
			if (memloc_gettyp(arg)== MEMADR_TYPE_HSTACKIND)
				return fg_ghstack_pushhstackind(self, memloc_hstackind_getcfa(arg));
			return fg_ghstack_pushregind(self,
			                                                 memloc_hregind_getreg(arg),
			                                                 memloc_hregind_getindoff(arg));
		}
		if (memloc_gettyp(arg)== MEMADR_TYPE_HSTACKIND) {
			uintptr_t cfa_offset = memloc_hstackind_getcfa(arg);
			ind_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
			gen86_src_regno = GEN86_R_PSP;
		} else {
			ind_offset = arg->ml_adr.ma_val.v_indoff;
			gen86_src_regno = gen86_registers[arg->ml_adr.ma_reg];
		}

		/* Try to use a temporary register. */
		tempreg = alloc_unused_reg_for_call_args(locv, argc);
		if (tempreg < HOST_REGNO_COUNT) {
			if unlikely(fg_gmov_loc2reg(self, arg, tempreg))
				goto err;
			return fg_ghstack_pushreg(self, tempreg);
		}
		if unlikely(fg_ghstack_pushhstackind(self, arg->ml_adr.ma_val.v_cfa))
			goto err;
		return Psp0_add_val_offset(self, memloc_getoff(arg));
	}	break;

	case MEMADR_TYPE_HSTACK: {
		host_regno_t tempreg;
		host_cfa_t cfa_offset = memloc_hstackind_getcfa(arg);
		ptrdiff_t sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		struct host_section *sect = fg_gettext(self);
		if (sp_offset == 0) {
			/* Special case: can use `pushP %Psp', which pushes the current %Psp value (as it was before the push) */
			if unlikely(host_section_reqx86(sect, 1))
				goto err;
			gen86_printf(sect, "push" Plq "\t%%" Per "sp\n");
			gen86_pushP_r(p_pc(sect), GEN86_R_PSP);
			return fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		}

		/* Check if there is a temp register we can use. */
		tempreg = alloc_unused_reg_for_call_args(locv, argc);
		if (tempreg < HOST_REGNO_COUNT) {
			if unlikely(fg_gmov_hstack2reg(self, cfa_offset, tempreg))
				goto err;
			return fg_ghstack_pushreg(self, tempreg);
		}

		/* All GP registers need to remain preserved -> load address in 2 steps:
		 * >> pushP %Psp
		 * >> addP  $..., 0(%Psp) */
		if unlikely(host_section_reqx86(sect, 2))
			goto err;
		gen86_printf(sect, "push" Plq "\t%%" Per "sp\n");
		gen86_pushP_r(p_pc(sect), GEN86_R_PSP);
		if unlikely(fg_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER))
			goto err;
		gen86_printf(sect, "add" Plq "\t$%Id, (%%" Per "sp)\n");
		gen86_addP_imm_mod(p_pc(sect), gen86_modrm_b, sp_offset, GEN86_R_PSP);
	}	break;

	case MEMADR_TYPE_HREG:
		if (memloc_getoff(arg) != 0) {
			size_t i;
			ptrdiff_t val_delta = memloc_getoff(arg);
			host_regno_t regno = memloc_hreg_getreg(arg);
			struct host_section *sect = fg_gettext(self);
			if unlikely(_fungen_gadjust_reg_delta(sect, self, regno, val_delta, true))
				goto err;
			for (i = 0; i <= argc; ++i) {
				struct memloc *nextarg = &locv[i];
				switch (memloc_gettyp(nextarg)) {
				case MEMADR_TYPE_HREG:
					if (memloc_hreg_getreg(nextarg) == regno)
						nextarg->ml_off -= val_delta;
					break;
				case MEMADR_TYPE_HREGIND:
					if (memloc_hregind_getreg(nextarg) == regno)
						nextarg->ml_adr.ma_val.v_indoff -= val_delta;
					break;
				default: break;
				}
			}
		}
		return fg_ghstack_pushreg(self, memloc_hreg_getreg(arg));

	case MEMADR_TYPE_CONST:
		return fg_ghstack_pushconst(self, memloc_const_getaddr(arg));

	case MEMADR_TYPE_UNDEFINED:
		return fg_ghstack_adjust(self, HOST_SIZEOF_POINTER);

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot push memory location with type %#" PRFx16,
		                       memloc_gettyp(arg));
		break;
	}
	return 0;
err:
	return -1;
}

#ifdef HOST_REGNO_R_ARG0
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
setreg_memloc_maybe_adjust_regs(struct fungen *__restrict self,
                                struct memloc *arg, host_regno_t dst_regno,
                                struct memloc *locv, size_t argc,
                                size_t already_pushed_arg_regs) {
	STATIC_ASSERT_MSG(COMPILER_LENOF(host_arg_regs) + 2 <= HOST_REGNO_COUNT,
	                  "Need at least 2 spare registers for shuffling:\n"
	                  "- 1 to store the function being called\n"
	                  "- 1 for temporary register moves\n");
	size_t i;

	/* Load argument into a register.
	 * - If the register is already used by an upcoming argument, then:
	 *   - try to load that upcoming argument into *its* proper register immediatly.
	 *   - If that doesn't work, then load the upcoming argument into some register
	 *     that isn't already in use (at this point, it is guarantied that such a
	 *     register exists, because %Pax isn't used for arguments, and the number of
	 *     GP registers is greater than the max number reg args +1 (+1 needed here
	 *     because 1 extra reg may be in use by the function getting called))
	 */

	/* Check if "dst_regno" is used by an upcoming argument (or the function pointer) */
	for (i = 0; i <= argc; ++i) {
		/* Registers used by upcoming arguments must not be clobbered */
		struct memloc *nextarg = &locv[i];
		host_regno_t free_regno;
		ptrdiff_t free_regno_val_offset_from_dst_regno;
		bool free_regno_initialized;
		uint16_t register_use_count[HOST_REGNO_COUNT];
		if (!memloc_hasreg(nextarg))
			continue;
		if (memloc_getreg(nextarg) != dst_regno)
			continue;
		bzero(register_use_count, sizeof(register_use_count));
		for (i = 0; i < already_pushed_arg_regs; ++i) /* Already loaded arguments must not be clobbered */
			register_use_count[host_arg_regs[argc + i]] = (uint16_t)-1;
		ASSERT(arg == &locv[argc + 1]);
		for (i = 0; i <= (argc + 1); ++i) { /* +1, because we also mustn't use a register used by the current argument! */
			struct memloc *checkarg = &locv[i];
			if (memloc_hasreg(checkarg)) {
				host_regno_t regno = memloc_getreg(checkarg);
				++register_use_count[regno];
			}
		}
		for (free_regno = 0;; ++free_regno) {
			ASSERTF(free_regno < HOST_REGNO_COUNT, "At least 1 register should have been available!");
			if (register_use_count[free_regno] == 0)
				break;
		}
		free_regno_initialized = false;
		free_regno_val_offset_from_dst_regno = 0; /* %free_regno = %dst_regno + free_regno_val_offset_from_dst_regno */
		for (i = 0; i <= argc; ++i) {
			host_regno_t used_regno;
			ptrdiff_t nextarg_valoff;
			nextarg = &locv[i];
			if (!memloc_hasreg(nextarg))
				continue;
			if (memloc_getreg(nextarg) != dst_regno)
				continue;
			used_regno = i == 0 ? HOST_REGNO_PAX : host_arg_regs[i - 1]; /* Try to use PAX for the function pointer */
			if (register_use_count[used_regno] != 0)
				used_regno = free_regno;
			switch (memloc_gettyp(nextarg)) {
			case MEMADR_TYPE_HREG:
				nextarg_valoff = memloc_hreg_getvaloff(nextarg);
				break;
			case MEMADR_TYPE_HREGIND:
				nextarg_valoff = memloc_hregind_getindoff(nextarg);
				break;
			default: __builtin_unreachable();
			}
			if (used_regno == free_regno && free_regno_initialized) {
				nextarg_valoff -= free_regno_val_offset_from_dst_regno;
			} else {
				/* TODO: If "nextarg" is `MEMADR_TYPE_HREGIND' and the register isn't used
				 *       by any other argument, directly load the value into the register. */
				if unlikely(fg_gmov_regx2reg(self, dst_regno, nextarg_valoff, used_regno))
					goto err;
				if (used_regno == free_regno) {
					free_regno_val_offset_from_dst_regno = nextarg_valoff;
					free_regno_initialized = true;
				}
				nextarg_valoff = 0;
			}
			switch (memloc_gettyp(nextarg)) {
			case MEMADR_TYPE_HREG:
				memloc_init_hreg(nextarg, used_regno, nextarg_valoff);
				break;
			case MEMADR_TYPE_HREGIND:
				memloc_init_hregind(nextarg, used_regno, nextarg_valoff,
				                        memloc_hregind_getvaloff(nextarg));
				break;
			default: __builtin_unreachable();
			}
		}
		break;
	}

	/* At this point, we've successfully made it so that "dst_regno" isn't used by future arguments. */

	/* NOTE: It is important that this function doesn't call `fg_gallocreg()'!
	 *       That is because `alloc_unused_reg_for_call_args()' needs to be used for temporary
	 *       registers */
	switch (memloc_gettyp(arg)) {

	case MEMADR_TYPE_HSTACKIND:
		if unlikely(fg_gmov_hstackind2reg_nopop(self, memloc_hstackind_getcfa(arg), dst_regno))
			goto err;
		return fg_gmov_regx2reg(self, dst_regno, memloc_hregind_getvaloff(arg), dst_regno);

	case MEMADR_TYPE_HREGIND:
		if unlikely(fg_gmov_regind2reg(self,
		                                                   memloc_hregind_getreg(arg),
		                                                   memloc_hregind_getindoff(arg),
		                                                   dst_regno))
			goto err;
		return fg_gmov_regx2reg(self, dst_regno, memloc_hregind_getvaloff(arg), dst_regno);

	case MEMADR_TYPE_HSTACK:
		return fg_gmov_hstack2reg(self, memloc_hstack_getcfa(arg), dst_regno);

	case MEMADR_TYPE_HREG:
		return fg_gmov_regx2reg(self,
		                                            memloc_hreg_getreg(arg),
		                                            memloc_hreg_getvaloff(arg),
		                                            dst_regno);

	case MEMADR_TYPE_CONST:
		return fg_gmov_const2reg(self, memloc_const_getaddr(arg), dst_regno);

	case MEMADR_TYPE_UNDEFINED:
		break;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move-to-reg memory location with type %#" PRFx16,
		                       memloc_gettyp(arg));
		break;
	}
	return 0;
err:
	return -1;
}
#endif /* HOST_REGNO_R_ARG0 */

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gcallapi(struct fungen *__restrict self,
                 struct memloc *locv, size_t argc) {
	struct host_section *sect = fg_gettext(self);
	uintptr_t hstackaddr_regs[HOST_REGNO_COUNT];
	size_t argi;
	bzero(hstackaddr_regs, sizeof(hstackaddr_regs));

#if HOSTASM_SCRACHAREA_SIZE > 0 || HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER
#ifdef HOST_REGNO_R_ARG0
	if (argc > COMPILER_LENOF(host_arg_regs))
#endif /* !HOST_REGNO_R_ARG0 */
	{
		/* Check if we need to do stack alignment early on.
		 * Since (some) arguments are passed via the stack, if we let alignment
		 * happen late, then those stack-based arguments will end up in the wrong
		 * locations. */
		struct memstate *state = self->fg_state;
		host_cfa_t current_cfa_offset = state->ms_host_cfa_offset;
		host_cfa_t greatest_inuse_cfa_offset;
#if HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER
		host_cfa_t cfa_at_callee, aligned_cfa_at_callee;
		size_t required_alignment; /* Minimal stack sub required for stack alignment if stack-arguments are pushed.
		                            * either: SP = SP - required_alignment
		                            * or:     SP = SP + HOSTASM_STACK_ALIGNMENT - required_alignment */
#endif /* HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER */
#ifdef HOST_REGNO_R_ARG0
		size_t stack_argc = argc - COMPILER_LENOF(host_arg_regs);
#else /* HOST_REGNO_R_ARG0 */
		size_t stack_argc = argc;
#endif /* !HOST_REGNO_R_ARG0 */
		size_t stack_needed, stack_free;
		stack_needed = stack_argc * HOST_SIZEOF_POINTER;
		stack_needed += HOSTASM_SCRACHAREA_SIZE;

		/* Take alignment requirements of the call-site into consideration */
#if HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER
		cfa_at_callee = current_cfa_offset;
		cfa_at_callee += stack_needed;
		cfa_at_callee += HOST_SIZEOF_POINTER; /* Because of the return address */
		aligned_cfa_at_callee = CEIL_ALIGN(cfa_at_callee, HOSTASM_STACK_ALIGNMENT);
		required_alignment = aligned_cfa_at_callee - cfa_at_callee;
		aligned_cfa_at_callee -= HOST_SIZEOF_POINTER; /* Because of the return address */
		stack_needed = aligned_cfa_at_callee - current_cfa_offset;
#endif /* HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER */
		greatest_inuse_cfa_offset = memstate_hstack_greatest_inuse(state);
		stack_free = current_cfa_offset - greatest_inuse_cfa_offset;
		if (stack_free >= stack_needed) {
			/* Under MSABI, don't just unconditionally push arguments onto
			 * the stack here, but see if we might have an easier time by
			 * allocating all stack space (including the call-site scratch
			 * area) from the get-go:
			 *
			 * Instead of this:
			 *     >> pushq g
			 *     >> pushq f
			 *     >> pushq e
			 *     >> movq  d, %r9
			 *     >> movq  c, %r8
			 *     >> movq  b, %rdx
			 *     >> movq  a, %rcx
			 *     >> subq  $32, %rsp
			 *     >> call  ...
			 *
			 * Consider do this instead:
			 *     >> subq  $56, %rsp
			 *     >> movq  g, 48(%rsp)
			 *     >> movq  f, 40(%rsp)
			 *     >> movq  e, 32(%rsp)
			 *     >> movq  d, %r9
			 *     >> movq  c, %r8
			 *     >> movq  b, %rdx
			 *     >> movq  a, %rcx
			 *     >> subq  $32, %rsp
			 *     >> call  ...
			 */

			/* TODO */
		}

#if HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER
		if (required_alignment) {
			size_t free_size = HOSTASM_STACK_ALIGNMENT - required_alignment;
			if (stack_free >= free_size) {
				/* Free "free_size" bytes of stack space in order to satisfy alignment. */
				if unlikely(host_section_reqx86(sect, 1))
					goto err;
				gen86_printf(sect, "add" Plq "\t$%Iu, %%" Per "sp\n", free_size);
				gen86_addP_imm_r(p_pc(sect), free_size, GEN86_R_PSP);
				if unlikely(fg_gadjust_cfa_offset(self, -(ptrdiff_t)free_size))
					goto err;
			} else {
				/* Allocate "required_alignment" bytes of stack space in order to satisfy alignment. */
				if unlikely(host_section_reqx86(sect, 1))
					goto err;
				gen86_printf(sect, "sub" Plq "\t$%Iu, %%" Per "sp\n", required_alignment);
				gen86_subP_imm_r(p_pc(sect), required_alignment, GEN86_R_PSP);
				if unlikely(fg_gadjust_cfa_offset(self, required_alignment))
					goto err;
			}
		}
#endif /* HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER */
	}
#endif /* HOSTASM_SCRACHAREA_SIZE > 0 || HOSTASM_STACK_ALIGNMENT > HOST_SIZEOF_POINTER */


	/* Push arguments onto the host stack in reverse order. */
	argi = argc;
#ifdef HOST_REGNO_R_ARG0
	while (argi > COMPILER_LENOF(host_arg_regs))
#else /* HOST_REGNO_R_ARG0 */
	while (argi)
#endif /* !HOST_REGNO_R_ARG0 */
	{
		struct memloc *arg;
		--argi;
		arg = &locv[argi + 1]; /* +1 because of the function being called. */

		/* All remaining arguments must be pushed via the stack. */
		if unlikely(push_memloc_maybe_adjust_regs(self, arg, locv, argi))
			goto err;
	}

#ifdef HOST_REGNO_R_ARG0
	/* Pick the best-fitting equivalences of register arguments. */
	{
		size_t i;
		for (i = 0; i <= argi; ++i) {
			/* Look at equivalences and pick:
			 * #1: MEMADR_TYPE_HREG[rel_valoff==0]  (The intended target register)
			 * #2: HREG/HSTACK
			 * #3: CONST
			 * #4: HREGIND[rel_valoff==0]/HSTACKIND[rel_valoff==0]
			 * #5: Fallback (use `arg' as-is) */
			struct memloc *arg = &locv[i];
			struct memequiv *eq = memequivs_getclassof(&self->fg_state->ms_memequiv, memloc_getadr(arg));
			if (eq != NULL) {
				host_regno_t best_fit = i == 0 ? HOST_REGNO_PAX : host_arg_regs[i - 1];
				struct memequiv *iter;
				ptrdiff_t base_off;
				base_off = memloc_getoff(arg);
				base_off -= memloc_getoff(&eq->meq_loc);
				iter = eq;
				do {
					if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG &&
					    (memloc_hreg_getreg(&iter->meq_loc) == best_fit || best_fit == HOST_REGNO_PAX) &&
					    (memloc_hreg_getvaloff(&iter->meq_loc) + base_off) == 0) {
use_memequiv_iter_as_arg:
						*arg = iter->meq_loc;
						memloc_adjoff(arg, base_off);
						goto use_arg;
					}
				} while ((iter = memequiv_next(iter)) != eq);
				iter = eq;
				do {
					if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG ||
					    memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACK)
						goto use_memequiv_iter_as_arg;
				} while ((iter = memequiv_next(iter)) != eq);
				iter = eq;
				do {
					if (memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_CONST)
						goto use_memequiv_iter_as_arg;
				} while ((iter = memequiv_next(iter)) != eq);
				iter = eq;
				do {
					if ((memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREGIND ||
					     memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACKIND) &&
					    (memloc_getoff(&iter->meq_loc) + base_off) == 0)
						goto use_memequiv_iter_as_arg;
				} while ((iter = memequiv_next(iter)) != eq);
			}
		}
use_arg:;
	}

	/* Fix-up argument registers. */
	while (argi) {
		host_regno_t arg_regno;
		struct memloc *arg;
		--argi;
		arg = &locv[argi + 1]; /* +1 because of the function being called. */
		arg_regno = host_arg_regs[argi];
		if unlikely(setreg_memloc_maybe_adjust_regs(self, arg, arg_regno, locv, argi,
		                                            (COMPILER_LENOF(host_arg_regs) - 1) - argi))
			goto err;
	}
#endif /* HOST_REGNO_R_ARG0 */

	/* Prepare the stack for the call that's about to be made. */
#ifdef HOST_REGNO_R_ARG0
	if unlikely(_fungen_gcall86_prepare_stack_for_call(self, argi > COMPILER_LENOF(host_arg_regs), true))
		goto err;
#else /* HOST_REGNO_R_ARG0 */
	if unlikely(_fungen_gcall86_prepare_stack_for_call(self, argi > 0, true))
		goto err;
#endif /* !HOST_REGNO_R_ARG0 */

	switch (__builtin_expect(memloc_gettyp(locv), MEMADR_TYPE_CONST)) {

	case MEMADR_TYPE_CONST: {
		void const *api_function = memloc_const_getaddr(locv);
		if unlikely(_fungen_gcall86(self, api_function, true))
			goto err;
	}	break;

	default:
invoke_api_function_fallback:
		if unlikely(fg_gasreg(self, locv, locv, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG: {
		host_regno_t regno = memloc_hreg_getreg(locv);
		ptrdiff_t val_offset = memloc_hreg_getvaloff(locv);
		if unlikely(val_offset != 0) {
			if unlikely(_fungen_gadjust_reg_delta(sect, NULL, regno, val_offset, true))
				goto err;
			if unlikely(host_section_reqx86(sect, 1))
				goto err;
		}
		gen86_printf(sect, "calll\t*%s\n", gen86_regname(regno));
		gen86_callP_mod(p_pc(sect), gen86_modrm_r, gen86_registers[regno]);
	}	break;

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t cfa_offset = memloc_hstackind_getcfa(locv);
		ptrdiff_t val_offset = memloc_hstackind_getvaloff(locv);
		ptrdiff_t sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		if unlikely(val_offset != 0)
			goto invoke_api_function_fallback;
		gen86_printf(sect, "calll\t%Id(%%" Per "sp)\n", sp_offset);
		gen86_callP_mod(p_pc(sect), gen86_modrm_db, sp_offset, GEN86_R_PSP);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		host_regno_t regno = memloc_hregind_getreg(locv);
		ptrdiff_t ind_offset = memloc_hregind_getindoff(locv);
		ptrdiff_t val_offset = memloc_hregind_getvaloff(locv);
		if unlikely(val_offset != 0)
			goto invoke_api_function_fallback;
		gen86_printf(sect, "calll\t%Id(%s)\n", ind_offset, gen86_regname(regno));
		gen86_callP_mod(p_pc(sect), gen86_modrm_db, ind_offset, gen86_registers[regno]);
	}	break;

	}

	/* On i386, STDCALL means that the callee removed
	 * arguments from the stack (adjust our CFA to match) */
#ifndef HOSTASM_X86_64
	return fg_gadjust_cfa_offset(self, -(ptrdiff_t)(argc * HOST_SIZEOF_POINTER));
#else /* !HOSTASM_X86_64 */
	return 0;
#endif /* HOSTASM_X86_64 */
err:
	return -1;
}




INTERN WUNUSED NONNULL((1, 2)) int DCALL
_host_section_gjmp(struct host_section *__restrict self,
                   struct host_symbol *__restrict dst) {
	struct host_reloc *rel;
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "jmpl\t%s\n", gen86_symname(dst));
	gen86_jmpl_offset(p_pc(self), -4);
	rel = host_section_newhostrel(self);
	if unlikely(!rel)
		goto err;
	rel->hr_offset = (uint32_t)(p_off(self) - 4);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
	host_reloc_setsym(rel, dst);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_host_section_gjcc(struct host_section *__restrict self,
                   struct host_symbol *__restrict dst, uint8_t cc) {
	struct host_reloc *rel;
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "j%sl\t%s\n", gen86_ccnames[cc], gen86_symname(dst));
	gen86_jccl_offset(p_pc(self), cc, -4);
	rel = host_section_newhostrel(self);
	if unlikely(!rel)
		goto err;
	rel->hr_offset = (uint32_t)(p_off(self) - 4);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
	host_reloc_setsym(rel, dst);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
_host_section_gjcc_reg(struct host_section *__restrict self, host_regno_t src_regno,
                       struct host_symbol *__restrict dst, uint8_t cc) {
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "test" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(src_regno));
	gen86_testP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno]);
	return _host_section_gjcc(self, dst, cc);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
_fungen_gjcc_regind(struct fungen *__restrict self, host_regno_t src_regno,
                    ptrdiff_t ind_delta, struct host_symbol *__restrict dst, uint8_t cc) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(_fungen_gadjust_reg_fit32(self, &src_regno, &ind_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "cmp" Plq "\t$0, %Id(%s)\n", ind_delta, gen86_regname(src_regno));
	gen86_cmpP_imm_mod(p_pc(sect), gen86_modrm_db, 0, ind_delta, gen86_registers[src_regno]);
	return _host_section_gjcc(sect, dst, cc);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
_host_section_gjcc_hstackind(struct host_section *__restrict self, ptrdiff_t sp_offset,
                             struct host_symbol *__restrict dst, uint8_t cc) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "cmp" Plq "\t$0, %Id(%%" Per "sp)\n", sp_offset);
	gen86_cmpP_imm_mod(p_pc(self), gen86_modrm_db, 0, sp_offset, GEN86_R_PSP);
	return _host_section_gjcc(self, dst, cc);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 3)) int DCALL
_host_section_gjz_reg(struct host_section *__restrict self,
                      host_regno_t src_regno, struct host_symbol *__restrict dst) {
	return _host_section_gjcc_reg(self, src_regno, dst, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
_fungen_gjz_regind(struct fungen *__restrict self,
                   host_regno_t src_regno, ptrdiff_t ind_delta,
                   struct host_symbol *__restrict dst) {
	return _fungen_gjcc_regind(self, src_regno, ind_delta, dst, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
_host_section_gjz_hstackind(struct host_section *__restrict self,
                            ptrdiff_t sp_offset, struct host_symbol *__restrict dst) {
	return _host_section_gjcc_hstackind(self, sp_offset, dst, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
_host_section_gjnz_reg(struct host_section *__restrict self,
                       host_regno_t src_regno, struct host_symbol *__restrict dst) {
	return _host_section_gjcc_reg(self, src_regno, dst, GEN86_CC_NZ);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
_fungen_gjnz_regind(struct fungen *__restrict self,
                    host_regno_t src_regno, ptrdiff_t ind_delta,
                    struct host_symbol *__restrict dst) {
	return _fungen_gjcc_regind(self, src_regno, ind_delta, dst, GEN86_CC_NZ);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
_host_section_gjnz_hstackind(struct host_section *__restrict self,
                             ptrdiff_t sp_offset, struct host_symbol *__restrict dst) {
	return _host_section_gjcc_hstackind(self, sp_offset, dst, GEN86_CC_NZ);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc3(struct host_section *__restrict self, bool signed_cmp,
                    struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                    struct host_symbol *dst_gr) {
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
		if unlikely(_host_section_gjcc(self, dst_lo, cc_lo))
			goto err;
		if (cc_lo == GEN86_CC_NE)
			return dst_eq ? _host_section_gjmp(self, dst_eq) : 0;
	}
	if (dst_gr) {
		uint8_t cc_gr = signed_cmp ? GEN86_CC_G : GEN86_CC_A;
		if (dst_eq == dst_gr) {
			dst_eq = NULL;
			cc_gr  = signed_cmp ? GEN86_CC_GE : GEN86_CC_AE;
		}
		if unlikely(_host_section_gjcc(self, dst_gr, cc_gr))
			goto err;
	}
	if (dst_eq) {
		if (dst_lo && dst_gr)
			return _host_section_gjmp(self, dst_gr);
		return _host_section_gjcc(self, dst_eq, GEN86_CC_E);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_regCreg(struct host_section *__restrict self,
                           host_regno_t lhs_regno, host_regno_t rhs_regno, bool signed_cmp,
                           struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                           struct host_symbol *dst_gr) {
	if unlikely(lhs_regno == rhs_regno) /* Same register -> compare is always equal */
		return dst_eq ? _host_section_gjmp(self, dst_eq) : 0;
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "cmp" Plq "\t%s, %s\n", gen86_regname(rhs_regno), gen86_regname(lhs_regno));
	gen86_cmpP_r_r(p_pc(self), gen86_registers[rhs_regno], gen86_registers[lhs_regno]);
	return _host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_regCconst(struct host_section *__restrict self,
                             host_regno_t lhs_regno, void const *rhs_value, bool signed_cmp,
                             struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                             struct host_symbol *dst_gr) {
	if unlikely(rhs_value == (void const *)0 && (dst_lo == dst_gr)) {
		/* >> if (%Pax == 0) { ... }
		 * Same as:
		 * >> if (!%Pax) { ... } */
		if (dst_lo && dst_eq) {
			if unlikely(_host_section_gjz_reg(self, lhs_regno, dst_eq))
				goto err;
			return _host_section_gjmp(self, dst_lo);
		} else if (dst_eq) {
			return _host_section_gjz_reg(self, lhs_regno, dst_eq);
		} else if (dst_lo) {
			return _host_section_gjnz_reg(self, lhs_regno, dst_lo);
		}
		return 0;
	}
#ifdef _host_section_gjcc_regCconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _host_section_gjcc_regCconst_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "cmp" Plq "\t$%#Ix, %s\n", rhs_value, gen86_regname(lhs_regno));
	gen86_cmpP_imm_r(p_pc(self), (int32_t)(intptr_t)(uintptr_t)rhs_value, gen86_registers[lhs_regno]);
	return _host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gjcc_regindCreg(struct fungen *__restrict self,
                        host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta,
                        host_regno_t rhs_regno, bool signed_cmp,
                        struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                        struct host_symbol *dst_gr) {
	struct host_section *sect = fg_gettext(self);
#ifndef fit32_IS_1
	if (!fit32(lhs_ind_delta)) {
		if (lhs_regno == rhs_regno) {
			host_regno_t not_these[2];
			ptrdiff_t adj_delta;
			not_these[0] = rhs_regno;
			not_these[1] = HOST_REGNO_COUNT;
			lhs_regno = fg_gallocreg(self, not_these);
			if unlikely(lhs_regno >= HOST_REGNO_COUNT)
				goto err;
			adj_delta = lhs_ind_delta;
			if (adj_delta < INT32_MIN) {
				adj_delta = INT32_MIN;
			} else if (adj_delta > INT32_MAX) {
				adj_delta = INT32_MAX;
			}
			if unlikely(host_section_reqx86(sect, 1))
				goto err;
			HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(rhs_regno), gen86_regname(lhs_regno));
			gen86_leaP_db_r(p_pc(sect), adj_delta, gen86_registers[rhs_regno], gen86_registers[lhs_regno]);
			lhs_ind_delta -= adj_delta;
		}
		if unlikely(_fungen_gadjust_reg_fit32(self, &lhs_regno, &lhs_ind_delta))
			goto err;
	}
#endif /* !fit32_IS_1 */
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "cmp" Plq "\t%s, %Id(%s)\n", gen86_regname(rhs_regno), lhs_ind_delta, gen86_regname(lhs_regno));
	gen86_cmpP_r_mod(p_pc(sect), gen86_modrm_db, gen86_registers[rhs_regno],
	                 (int32_t)lhs_ind_delta, gen86_registers[lhs_regno]);
	return _host_section_gjcc3(sect, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gjcc_regindCconst(struct fungen *__restrict self,
                          host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta,
                          void const *rhs_value, bool signed_cmp,
                          struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                          struct host_symbol *dst_gr) {
	struct host_section *sect = fg_gettext(self);
#ifdef _host_section_gjcc_regCconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _host_section_gjcc_regCconst_MAYFAIL */
	if unlikely(_fungen_gadjust_reg_fit32(self, &lhs_regno, &lhs_ind_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "cmp" Plq "\t$%#Ix, %Id(%s)\n", rhs_value, lhs_ind_delta, gen86_regname(lhs_regno));
	gen86_cmpP_imm_mod(p_pc(sect), gen86_modrm_db,
	                   (int32_t)(intptr_t)(uintptr_t)rhs_value,
	                   lhs_ind_delta, gen86_registers[lhs_regno]);
	return _host_section_gjcc3(sect, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_hstackindCreg(struct host_section *__restrict self,
                                 ptrdiff_t lhs_sp_offset, host_regno_t rhs_regno, bool signed_cmp,
                                 struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                                 struct host_symbol *dst_gr) {
	ASSERTF(fit32(lhs_sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "cmp" Plq "\t%s, %Id(%%" Per "sp)\n", gen86_regname(rhs_regno), lhs_sp_offset);
	gen86_cmpP_r_mod(p_pc(self), gen86_modrm_db, gen86_registers[rhs_regno],
	                 (int32_t)lhs_sp_offset, GEN86_R_PSP);
	return _host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_hstackindCconst(struct host_section *__restrict self,
                                   ptrdiff_t lhs_sp_offset, void const *rhs_value, bool signed_cmp,
                                   struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                                   struct host_symbol *dst_gr) {
#ifdef _host_section_gjcc_hstackindCconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _host_section_gjcc_hstackindCconst_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "cmp" Plq "\t$%#Ix, %Id(%%" Per "sp)\n", rhs_value, lhs_sp_offset);
	gen86_cmpP_imm_mod(p_pc(self), gen86_modrm_db, (int32_t)(intptr_t)(uintptr_t)rhs_value,
	                   (int32_t)lhs_sp_offset, GEN86_R_PSP);
	return _host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc2(struct host_section *__restrict self,
                    struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	/* At this point, the compare itself has been generated.
	 * Now to emit the actual branch instructions. */
	if (dst_nz) {
		if (dst_z) {
			if (dst_nz != dst_z) {
				if unlikely(_host_section_gjcc(self, dst_nz, GEN86_CC_NZ))
					goto err;
			}
			return _host_section_gjmp(self, dst_z);
		}
		return _host_section_gjcc(self, dst_nz, GEN86_CC_NZ);
	} else if (dst_z) {
		return _host_section_gjcc(self, dst_z, GEN86_CC_Z);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_regAreg(struct host_section *__restrict self,
                           host_regno_t lhs_regno, host_regno_t rhs_regno,
                           struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "test" Plq "\t%s, %s\n", gen86_regname(rhs_regno), gen86_regname(lhs_regno));
	gen86_testP_r_r(p_pc(self), gen86_registers[rhs_regno], gen86_registers[lhs_regno]);
	return _host_section_gjcc2(self, dst_nz, dst_z);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_regAconst(struct host_section *__restrict self,
                             host_regno_t lhs_regno, void const *rhs_value,
                             struct host_symbol *dst_nz, struct host_symbol *dst_z) {
#ifdef _host_section_gjcc_regAconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _host_section_gjcc_regAconst_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "test" Plq "\t$%#Ix, %s\n", rhs_value, gen86_regname(lhs_regno));
	gen86_testP_imm_r(p_pc(self), (int32_t)(intptr_t)(uintptr_t)rhs_value, gen86_registers[lhs_regno]);
	return _host_section_gjcc2(self, dst_nz, dst_z);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gjcc_regindAreg(struct fungen *__restrict self,
                        host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta,
                        host_regno_t rhs_regno,
                        struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	struct host_section *sect = fg_gettext(self);
#ifndef fit32_IS_1
	if (!fit32(lhs_ind_delta)) {
		if (lhs_regno == rhs_regno) {
			host_regno_t not_these[2];
			ptrdiff_t adj_delta;
			not_these[0] = rhs_regno;
			not_these[1] = HOST_REGNO_COUNT;
			lhs_regno = fg_gallocreg(self, not_these);
			if unlikely(lhs_regno >= HOST_REGNO_COUNT)
				goto err;
			adj_delta = lhs_ind_delta;
			if (adj_delta < INT32_MIN) {
				adj_delta = INT32_MIN;
			} else if (adj_delta > INT32_MAX) {
				adj_delta = INT32_MAX;
			}
			if unlikely(host_section_reqx86(sect, 1))
				goto err;
			HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(rhs_regno), gen86_regname(lhs_regno));
			gen86_leaP_db_r(p_pc(sect), adj_delta, gen86_registers[rhs_regno], gen86_registers[lhs_regno]);
			lhs_ind_delta -= adj_delta;
		}
		if unlikely(_fungen_gadjust_reg_fit32(self, &lhs_regno, &lhs_ind_delta))
			goto err;
	}
#endif /* !fit32_IS_1 */
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "test" Plq "\t%s, %Id(%s)\n", gen86_regname(rhs_regno), lhs_ind_delta, gen86_regname(lhs_regno));
	gen86_testP_r_mod(p_pc(sect), gen86_modrm_db, gen86_registers[rhs_regno],
	                  (int32_t)lhs_ind_delta, gen86_registers[lhs_regno]);
	return _host_section_gjcc2(sect, dst_nz, dst_z);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gjcc_regindAconst(struct fungen *__restrict self,
                          host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta,
                          void const *rhs_value,
                          struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	struct host_section *sect = fg_gettext(self);
#ifdef _host_section_gjcc_regAconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _host_section_gjcc_regAconst_MAYFAIL */
	if unlikely(_fungen_gadjust_reg_fit32(self, &lhs_regno, &lhs_ind_delta))
		goto err;
	if unlikely(host_section_reqx86(sect, 1))
		goto err;
	gen86_printf(sect, "test" Plq "\t$%#Ix, %Id(%s)\n", rhs_value, lhs_ind_delta, gen86_regname(lhs_regno));
	gen86_testP_imm_mod(p_pc(sect), gen86_modrm_db,
	                    (int32_t)(intptr_t)(uintptr_t)rhs_value,
	                    lhs_ind_delta, gen86_registers[lhs_regno]);
	return _host_section_gjcc2(sect, dst_nz, dst_z);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_hstackindAreg(struct host_section *__restrict self,
                                 ptrdiff_t lhs_sp_offset, host_regno_t rhs_regno,
                                 struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	ASSERTF(fit32(lhs_sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "test" Plq "\t%s, %Id(%%" Per "sp)\n", gen86_regname(rhs_regno), lhs_sp_offset);
	gen86_testP_r_mod(p_pc(self), gen86_modrm_db, gen86_registers[rhs_regno],
	                  (int32_t)lhs_sp_offset, GEN86_R_PSP);
	return _host_section_gjcc2(self, dst_nz, dst_z);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjcc_hstackindAconst(struct host_section *__restrict self,
                                   ptrdiff_t lhs_sp_offset, void const *rhs_value,
                                   struct host_symbol *dst_nz, struct host_symbol *dst_z) {
#ifdef _host_section_gjcc_hstackindAconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _host_section_gjcc_hstackindAconst_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "test" Plq "\t$%#Ix, %Id(%%" Per "sp)\n", rhs_value, lhs_sp_offset);
	gen86_testP_imm_mod(p_pc(self), gen86_modrm_db, (int32_t)(intptr_t)(uintptr_t)rhs_value,
	                    (int32_t)lhs_sp_offset, GEN86_R_PSP);
	return _host_section_gjcc2(self, dst_nz, dst_z);
err:
	return -1;
}


#ifndef NO_HOSTASM_DEBUG_PRINT
STATIC_ASSERT(BITOP_AND == 0);
STATIC_ASSERT(BITOP_OR == 1);
STATIC_ASSERT(BITOP_XOR == 2);
PRIVATE char const bitop_names[][4] = {
	/*[BITOP_AND] =*/ "and",
	/*[BITOP_OR]  =*/ "or",
	/*[BITOP_XOR] =*/ "xor",
};
#endif /* !NO_HOSTASM_DEBUG_PRINT */

PRIVATE NONNULL((1)) void DCALL
_host_section_gbitop_r_r(struct host_section *__restrict self, host_bitop_t op,
                         host_regno_t src_regno, host_regno_t dst_regno) {
	gen86_printf(self, "%s" Plq "\t%s, %s\n", bitop_names[op],
	             gen86_regname(src_regno), gen86_regname(dst_regno));
	switch (op) {
	case BITOP_AND:
		gen86_andP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case BITOP_OR:
		gen86_orP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case BITOP_XOR:
		gen86_xorP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	default: __builtin_unreachable();
	}
}

PRIVATE NONNULL((1)) void DCALL
_host_section_gbitop_imm_r(struct host_section *__restrict self, host_bitop_t op,
                           int32_t value, host_regno_t dst_regno) {
	gen86_printf(self, "%s" Plq "\t%s, %s\n", bitop_names[op],
	             gen86_addrname((void const *)(uintptr_t)(intptr_t)value),
	             gen86_regname(dst_regno));
	switch (op) {
	case BITOP_AND:
		gen86_andP_imm_r(p_pc(self), value, gen86_registers[dst_regno]);
		break;
	case BITOP_OR:
		gen86_orP_imm_r(p_pc(self), value, gen86_registers[dst_regno]);
		break;
	case BITOP_XOR:
		gen86_xorP_imm_r(p_pc(self), value, gen86_registers[dst_regno]);
		break;
	default: __builtin_unreachable();
	}
}

PRIVATE NONNULL((1)) void DCALL
_host_section_gbitop_db_r(struct host_section *__restrict self, host_bitop_t op,
                          int32_t src_offset, uint8_t src_regno86, host_regno_t dst_regno) {
	gen86_printf(self, "%s" Plq "\t%I32d(%s), %s\n", bitop_names[op], src_offset,
	             gen86_regnames[src_regno86], gen86_regname(dst_regno));
	switch (op) {
	case BITOP_AND:
		gen86_andP_mod_r(p_pc(self), gen86_modrm_db, src_offset, src_regno86, gen86_registers[dst_regno]);
		break;
	case BITOP_OR:
		gen86_orP_mod_r(p_pc(self), gen86_modrm_db, src_offset, src_regno86, gen86_registers[dst_regno]);
		break;
	case BITOP_XOR:
		gen86_xorP_mod_r(p_pc(self), gen86_modrm_db, src_offset, src_regno86, gen86_registers[dst_regno]);
		break;
	default: __builtin_unreachable();
	}
}

/* dst_regno = src1_regno <op> src2_regno; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gbitop_regreg2reg(struct host_section *__restrict self, host_bitop_t op,
                                host_regno_t src1_regno, host_regno_t src2_regno,
                                host_regno_t dst_regno) {
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	if (dst_regno == src2_regno) {
		_host_section_gbitop_r_r(self, op, src1_regno, dst_regno);
	} else {
		if (dst_regno != src1_regno) {
			gen86_printf(self, "mov" Plq "\t%s, %s\n", gen86_regname(src1_regno), gen86_regname(dst_regno));
			gen86_movP_r_r(p_pc(self), gen86_registers[src1_regno], gen86_registers[dst_regno]);
			if unlikely(host_section_reqx86(self, 1))
				goto err;
		}
		_host_section_gbitop_r_r(self, op, src2_regno, dst_regno);
	}
	return 0;
err:
	return -1;
}

/* dst_regno = src_regno <op> value; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gbitop_regconst2reg(struct host_section *__restrict self, host_bitop_t op,
                                  host_regno_t src_regno, void const *value,
                                  host_regno_t dst_regno) {
	if unlikely(host_section_reqx86(self, 1))
		goto err;
#ifdef _host_section_gbitop_regconst2reg_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _host_section_gbitop_regconst2reg_MAYFAIL */
	if (dst_regno != src_regno) {
		gen86_printf(self, "mov" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_movP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		if unlikely(host_section_reqx86(self, 1))
			goto err;
	}
	_host_section_gbitop_imm_r(self, op, (int32_t)(intptr_t)(uintptr_t)value, dst_regno);
	return 0;
err:
	return -1;
}

/* dst_regno = src1_regno <op> *(SP + src2_sp_offset); */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gbitop_reghstackind2reg(struct host_section *__restrict self, host_bitop_t op,
                                      host_regno_t src1_regno, ptrdiff_t src2_sp_offset,
                                      host_regno_t dst_regno) {
	ASSERTF(fit32(src2_sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if (src1_regno != dst_regno) {
		if unlikely(host_section_reqx86(self, 2))
			goto err;
		gen86_printf(self, "mov" Plq "\t%Id(%%" Per "sp), %s\n", src2_sp_offset, gen86_regname(dst_regno));
		gen86_movP_db_r(p_pc(self), src2_sp_offset, GEN86_R_PSP, gen86_registers[dst_regno]);
		_host_section_gbitop_r_r(self, op, src1_regno, dst_regno);
	} else {
		if unlikely(host_section_reqx86(self, 1))
			goto err;
		_host_section_gbitop_db_r(self, op, (int32_t)src2_sp_offset, GEN86_R_PSP, dst_regno);
	}
	return 0;
err:
	return -1;
}

/* dst_regno = src1_regno <op> *(src2_regno + src2_ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gbitop_regregind2reg(struct fungen *__restrict self, host_bitop_t op,
                             host_regno_t src1_regno, host_regno_t src2_regno,
                             ptrdiff_t src2_ind_delta, host_regno_t dst_regno) {
	struct host_section *sect = fg_gettext(self);
#ifndef fit32_IS_1
	if (!fit32(src2_ind_delta)) {
		if (src1_regno == src2_regno) {
			ptrdiff_t adj_delta;
			src1_regno = dst_regno;
			if (src1_regno == src2_regno) {
				host_regno_t not_these[2];
				not_these[0] = src2_regno;
				not_these[1] = HOST_REGNO_COUNT;
				src1_regno = fg_gallocreg(self, not_these);
				if unlikely(src1_regno >= HOST_REGNO_COUNT)
					goto err;
			}
			adj_delta = src2_ind_delta;
			if (adj_delta < INT32_MIN) {
				adj_delta = INT32_MIN;
			} else if (adj_delta > INT32_MAX) {
				adj_delta = INT32_MAX;
			}
			if unlikely(host_section_reqx86(sect, 1))
				goto err;
			HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(src2_regno), gen86_regname(src1_regno));
			gen86_leaP_db_r(p_pc(sect), adj_delta, gen86_registers[src2_regno], gen86_registers[src1_regno]);
			src2_ind_delta -= adj_delta;
		}
		if unlikely(_fungen_gadjust_reg_fit32(self, &src1_regno, &src2_ind_delta))
			goto err;
	}
#endif /* !fit32_IS_1 */
	if (src1_regno != dst_regno) {
		if unlikely(host_section_reqx86(sect, 2))
			goto err;
		gen86_printf(sect, "mov" Plq "\t%Id(%s), %s\n", src2_ind_delta, gen86_regname(src2_regno), gen86_regname(dst_regno));
		gen86_movP_db_r(p_pc(sect), src2_ind_delta, gen86_registers[src2_regno], gen86_registers[dst_regno]);
		_host_section_gbitop_r_r(sect, op, src1_regno, dst_regno);
	} else {
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		_host_section_gbitop_db_r(sect, op, (int32_t)src2_ind_delta, gen86_registers[src2_regno], dst_regno);
	}
	return 0;
err:
	return -1;
}





#ifndef NO_HOSTASM_DEBUG_PRINT
STATIC_ASSERT(ARITHOP_UADD == 0);
STATIC_ASSERT(ARITHOP_SADD == 1);
STATIC_ASSERT(ARITHOP_USUB == 2);
STATIC_ASSERT(ARITHOP_SSUB == 3);
STATIC_ASSERT(ARITHOP_UMUL == 4);
STATIC_ASSERT(ARITHOP_SMUL == 5);
PRIVATE char const arithop_names[][5] = {
	/*[ARITHOP_UADD] =*/ "add",
	/*[ARITHOP_SADD] =*/ "add",
	/*[ARITHOP_USUB] =*/ "sub",
	/*[ARITHOP_SSUB] =*/ "sub",
	/*[ARITHOP_UMUL] =*/ "mul",
	/*[ARITHOP_SMUL] =*/ "imul",
};
#endif /* !NO_HOSTASM_DEBUG_PRINT */

PRIVATE WUNUSED NONNULL((1)) int DCALL
_host_section_gjarith_jcc(struct host_section *__restrict self, host_arithop_t op,
                          struct host_symbol *dst_o, struct host_symbol *dst_no) {
	uint8_t cc_o = GEN86_CC_C;
	if (ARITHOP_ISSIGNED(op))
		cc_o = GEN86_CC_O;
	if (dst_no) {
		if (dst_o) {
			if (dst_no != dst_o) {
				if unlikely(_host_section_gjcc(self, dst_no, GEN86_CC_NOT(cc_o)))
					goto err;
			}
			return _host_section_gjmp(self, dst_o);
		}
		return _host_section_gjcc(self, dst_no, GEN86_CC_NOT(cc_o));
	} else if (dst_o) {
		return _host_section_gjcc(self, dst_o, cc_o);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gjarith_umul_prepare(struct host_section *__restrict self, host_regno_t dst_regno) {
	(void)self;
	(void)dst_regno;
	/* TODO: push %Pdx if used */
	if (dst_regno != HOST_REGNO_PAX) {
		/* TODO: push %Pax if used */
	}
	gen86_int3(p_pc(self));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gjarith_umul_finish(struct host_section *__restrict self, host_regno_t dst_regno) {
	(void)self;
	(void)dst_regno;
	if (dst_regno != HOST_REGNO_PAX) {
		/* TODO: mov %Pax into %dst_regno */
		/* TODO: pop %Pax if used */
	}
	/* TODO: pop %Pdx if used */
	gen86_int3(p_pc(self));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_host_section_gjarith_r_r(struct host_section *__restrict self, host_arithop_t op,
                              host_regno_t src_regno, host_regno_t dst_regno) {
	gen86_printf(self, "%s" Plq "\t%s, %s\n", arithop_names[op],
	             gen86_regname(src_regno), gen86_regname(dst_regno));
	switch (op) {
	case ARITHOP_UADD:
	case ARITHOP_SADD:
		gen86_addP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case ARITHOP_USUB:
	case ARITHOP_SSUB:
		gen86_subP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case ARITHOP_UMUL:
		if unlikely(gjarith_umul_prepare(self, dst_regno))
			goto err;
		gen86_mulP_r(p_pc(self), gen86_registers[src_regno]);
		if unlikely(gjarith_umul_finish(self, dst_regno))
			goto err;
		break;
	case ARITHOP_SMUL:
		gen86_imulP_mod_r(p_pc(self), gen86_modrm_r, gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	default: __builtin_unreachable();
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_host_section_gjarith_db_r(struct host_section *__restrict self, host_arithop_t op,
                           int32_t src_offset, uint8_t src_regno86, host_regno_t dst_regno) {
	gen86_printf(self, "%s" Plq "\t%I32d(%s), %s\n", arithop_names[op], src_offset,
	             gen86_regnames[src_regno86], gen86_regname(dst_regno));
	switch (op) {
	case ARITHOP_UADD:
	case ARITHOP_SADD:
		gen86_addP_mod_r(p_pc(self), gen86_modrm_db, src_offset, src_regno86, gen86_registers[dst_regno]);
		break;
	case ARITHOP_USUB:
	case ARITHOP_SSUB:
		gen86_subP_mod_r(p_pc(self), gen86_modrm_db, src_offset, src_regno86, gen86_registers[dst_regno]);
		break;
	case ARITHOP_UMUL:
	case ARITHOP_SMUL:
		/* TODO */
		gen86_int3(p_pc(self));
		break;
	default: __builtin_unreachable();
	}
	return 0;
/*
err:
	return -1;*/
}

/* dst_regno = src1_regno <op> src2_regno; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjarith_regreg2reg(struct host_section *__restrict self, host_arithop_t op,
                                 host_regno_t src1_regno, host_regno_t src2_regno,
                                 host_regno_t dst_regno,
                                 struct host_symbol *dst_o, struct host_symbol *dst_no) {
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	if (dst_regno == src2_regno) {
		if unlikely(_host_section_gjarith_r_r(self, op, src1_regno, dst_regno))
			goto err;
	} else {
		if (dst_regno != src1_regno) {
			gen86_printf(self, "mov" Plq "\t%s, %s\n", gen86_regname(src1_regno), gen86_regname(dst_regno));
			gen86_movP_r_r(p_pc(self), gen86_registers[src1_regno], gen86_registers[dst_regno]);
			if unlikely(host_section_reqx86(self, 1))
				goto err;
		}
		if unlikely(_host_section_gjarith_r_r(self, op, src2_regno, dst_regno))
			goto err;
	}
	return _host_section_gjarith_jcc(self, op, dst_o, dst_no);
err:
	return -1;
}

/* dst_regno = src_regno <op> value; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjarith_regconst2reg(struct host_section *__restrict self, host_arithop_t op,
                                   host_regno_t src_regno, void const *value,
                                   host_regno_t dst_regno,
                                   struct host_symbol *dst_o, struct host_symbol *dst_no) {
	if unlikely(host_section_reqx86(self, 1))
		goto err;
#ifdef _host_section_gjarith_regconst2reg_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _host_section_gjarith_regconst2reg_MAYFAIL */
	if (dst_regno != src_regno) {
		gen86_printf(self, "mov" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_movP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		if unlikely(host_section_reqx86(self, 1))
			goto err;
	}
	gen86_printf(self, "%s" Plq "\t%s, %s\n", arithop_names[op],
	             gen86_addrname((void const *)(uintptr_t)(intptr_t)value),
	             gen86_regname(dst_regno));
	switch (op) {
	case ARITHOP_UADD:
	case ARITHOP_SADD:
		gen86_addP_imm_r(p_pc(self), (int32_t)(intptr_t)(uintptr_t)value, gen86_registers[dst_regno]);
		break;
	case ARITHOP_USUB:
	case ARITHOP_SSUB:
		gen86_subP_imm_r(p_pc(self), (int32_t)(intptr_t)(uintptr_t)value, gen86_registers[dst_regno]);
		break;
	case ARITHOP_UMUL:
	case ARITHOP_SMUL:
		/* TODO */
		gen86_int3(p_pc(self));
		break;
	default: __builtin_unreachable();
	}
	return _host_section_gjarith_jcc(self, op, dst_o, dst_no);
err:
	return -1;
}

/* dst_regno = src1_regno <op> *(SP + src2_sp_offset); */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gjarith_reghstackind2reg(struct host_section *__restrict self, host_arithop_t op,
                                       host_regno_t src1_regno, ptrdiff_t src2_sp_offset,
                                       host_regno_t dst_regno,
                                       struct host_symbol *dst_o, struct host_symbol *dst_no) {
	ASSERTF(fit32(src2_sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if (src1_regno != dst_regno) {
		if unlikely(host_section_reqx86(self, 2))
			goto err;
		if (ARITHOP_MAYREORDER(op)) {
			gen86_printf(self, "mov" Plq "\t%Id(%%" Per "sp), %s\n", src2_sp_offset, gen86_regname(dst_regno));
			gen86_movP_db_r(p_pc(self), src2_sp_offset, GEN86_R_PSP, gen86_registers[dst_regno]);
			if unlikely(_host_section_gjarith_r_r(self, op, src1_regno, dst_regno))
				goto err;
		} else {
			gen86_printf(self, "mov" Plq "\t%s, %s\n", gen86_regname(src1_regno), gen86_regname(dst_regno));
			gen86_movP_r_r(p_pc(self), gen86_registers[src1_regno], gen86_registers[dst_regno]);
			if unlikely(_host_section_gjarith_db_r(self, op, (int32_t)src2_sp_offset, GEN86_R_PSP, dst_regno))
				goto err;
		}
	} else {
		if unlikely(host_section_reqx86(self, 1))
			goto err;
		if unlikely(_host_section_gjarith_db_r(self, op, (int32_t)src2_sp_offset, GEN86_R_PSP, dst_regno))
			goto err;
	}
	return _host_section_gjarith_jcc(self, op, dst_o, dst_no);
err:
	return -1;
}

/* dst_regno = src1_regno <op> *(src2_regno + src2_ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
_fungen_gjarith_regregind2reg(struct fungen *__restrict self, host_arithop_t op,
                              host_regno_t src1_regno, host_regno_t src2_regno,
                              ptrdiff_t src2_ind_delta, host_regno_t dst_regno,
                              struct host_symbol *dst_o, struct host_symbol *dst_no) {
	struct host_section *sect = fg_gettext(self);
#ifndef fit32_IS_1
	if (!fit32(src2_ind_delta)) {
		if (src1_regno == src2_regno) {
			ptrdiff_t adj_delta;
			src1_regno = dst_regno;
			if (src1_regno == src2_regno) {
				host_regno_t not_these[2];
				not_these[0] = src2_regno;
				not_these[1] = HOST_REGNO_COUNT;
				src1_regno = fg_gallocreg(self, not_these);
				if unlikely(src1_regno >= HOST_REGNO_COUNT)
					goto err;
			}
			adj_delta = src2_ind_delta;
			if (adj_delta < INT32_MIN) {
				adj_delta = INT32_MIN;
			} else if (adj_delta > INT32_MAX) {
				adj_delta = INT32_MAX;
			}
			if unlikely(host_section_reqx86(sect, 1))
				goto err;
			HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(src2_regno), gen86_regname(src1_regno));
			gen86_leaP_db_r(p_pc(sect), adj_delta, gen86_registers[src2_regno], gen86_registers[src1_regno]);
			src2_ind_delta -= adj_delta;
		}
		if unlikely(_fungen_gadjust_reg_fit32(self, &src1_regno, &src2_ind_delta))
			goto err;
	}
#endif /* !fit32_IS_1 */
	if (src1_regno != dst_regno) {
		if unlikely(host_section_reqx86(sect, 2))
			goto err;
		if (ARITHOP_MAYREORDER(op)) {
			gen86_printf(sect, "mov" Plq "\t%Id(%s), %s\n", src2_ind_delta, gen86_regname(src2_regno), gen86_regname(dst_regno));
			gen86_movP_db_r(p_pc(sect), src2_ind_delta, gen86_registers[src2_regno], gen86_registers[dst_regno]);
			if unlikely(_host_section_gjarith_r_r(sect, op, src1_regno, dst_regno))
				goto err;
		} else {
			gen86_printf(sect, "mov" Plq "\t%s, %s\n", gen86_regname(src1_regno), gen86_regname(dst_regno));
			gen86_movP_r_r(p_pc(sect), gen86_registers[src1_regno], gen86_registers[dst_regno]);
			if unlikely(_host_section_gjarith_db_r(sect, op, (int32_t)src2_ind_delta, gen86_registers[src2_regno], dst_regno))
				goto err;
		}
	} else {
		if unlikely(host_section_reqx86(sect, 1))
			goto err;
		if unlikely(_host_section_gjarith_db_r(sect, op, (int32_t)src2_ind_delta, gen86_registers[src2_regno], dst_regno))
			goto err;
	}
	return _host_section_gjarith_jcc(sect, op, dst_o, dst_no);
err:
	return -1;
}








/* dst_regno = src_regno * n; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gumul_regconst2reg(struct host_section *__restrict self,
                                 host_regno_t src_regno, uintptr_t n,
                                 host_regno_t dst_regno) {
	if (n == 1 && src_regno == dst_regno)
		return 0; /* Special case: nothing to do here. */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	switch (n) {
	case 0:
		gen86_printf(self, "xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
		break;
	case 1:
		gen86_printf(self, "mov" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_movP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case 2:
		gen86_printf(self, "lea" Plq "\t(%s,%s), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bi_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case 3:
		gen86_printf(self, "lea" Plq "\t(%s,%s,2), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bis_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], 2, gen86_registers[dst_regno]);
		break;
	case 4:
		gen86_printf(self, "lea" Plq "\t(,%s,4), %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_is_r(p_pc(self), gen86_registers[src_regno], 4, gen86_registers[dst_regno]);
		break;
	case 5:
		gen86_printf(self, "lea" Plq "\t(%s,%s,4), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bis_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], 4, gen86_registers[dst_regno]);
		break;
	case 8:
		gen86_printf(self, "lea" Plq "\t(%s,8), %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_is_r(p_pc(self), gen86_registers[src_regno], 8, gen86_registers[dst_regno]);
		break;
	case 9:
		gen86_printf(self, "lea" Plq "\t(%s,%s,8), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bis_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], 8, gen86_registers[dst_regno]);
		break;
	default:
		gen86_printf(self, "imul" Plq "\t%$Id, %s, %s\n", (intptr_t)n, gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_imulP_imm_mod_r(p_pc(self), gen86_modrm_r, (intptr_t)n, gen86_registers[dst_regno], gen86_registers[src_regno]);
		break;
	}
	return 0;
err:
	return -1;
}


/* *(SP + sp_offset) = *(SP + sp_offset) + <value>; */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_gadd_const2hstackind(struct host_section *__restrict self,
                                   void const *value, ptrdiff_t sp_offset) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely((uintptr_t)value == 0)
		return 0;
#ifdef _host_section_gadd_const2hstackind_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _host_section_gadd_const2hstackind_MAYFAIL */
	if unlikely(host_section_reqx86(self, 1))
		goto err;
	gen86_printf(self, "add" Plq "\t$%Id, %Id(%%" Per "sp)\n", (intptr_t)(uintptr_t)value, sp_offset);
	gen86_addP_imm_mod(p_pc(self), gen86_modrm_db, (intptr_t)(uintptr_t)value, sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_ARCH_C */
