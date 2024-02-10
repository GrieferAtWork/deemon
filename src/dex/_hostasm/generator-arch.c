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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_ARCH_C
#define GUARD_DEX_HOSTASM_GENERATOR_ARCH_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include <hybrid/sched/__yield.h>
#include <hybrid/sequence/bitset.h>
#include <hybrid/unaligned.h>

#ifdef HOSTASM_X86
#include "libgen86/gen.h"
#include "libgen86/register.h"
#endif /* HOSTASM_X86 */

DECL_BEGIN

#define OFFSETOF_ob_refcnt (ptrdiff_t)offsetof(DeeObject, ob_refcnt)

#ifdef HOSTASM_X86_64
#define HOST_REGISTER_PAX HOST_REGISTER_RAX
#define HOST_REGISTER_PCX HOST_REGISTER_RCX
#define HOST_REGISTER_PDX HOST_REGISTER_RDX
#else /* HOSTASM_X86_64 */
#define HOST_REGISTER_PAX HOST_REGISTER_EAX
#define HOST_REGISTER_PCX HOST_REGISTER_ECX
#define HOST_REGISTER_PDX HOST_REGISTER_EDX
#endif /* !HOSTASM_X86_64 */

#define gen86_printf(...) HA_printf("gen86:" __VA_ARGS__)


#ifdef HOSTASM_X86_64
#define fit32(v) likely((intptr_t)(uintptr_t)(v) == (intptr_t)(int32_t)(intptr_t)(uintptr_t)(v))
#else /* HOSTASM_X86_64 */
#define fit32_IS_1
#define fit32(v) 1
#endif /* !HOSTASM_X86_64 */


#define Dee_host_section_reqx86(self, n_instructions) \
	Dee_host_section_reqhost(self, GEN86_INSTRLEN_MAX * (n_instructions))

#ifdef HOST_REGISTER_R_ARG0
PRIVATE Dee_host_register_t const host_arg_regs[] = {
	HOST_REGISTER_R_ARG0,
#ifdef HOST_REGISTER_R_ARG1
	HOST_REGISTER_R_ARG1,
#ifdef HOST_REGISTER_R_ARG2
	HOST_REGISTER_R_ARG2,
#ifdef HOST_REGISTER_R_ARG3
	HOST_REGISTER_R_ARG3,
#ifdef HOST_REGISTER_R_ARG4
	HOST_REGISTER_R_ARG4,
#ifdef HOST_REGISTER_R_ARG5
	HOST_REGISTER_R_ARG5,
#ifdef HOST_REGISTER_R_ARG6
#error "Too many argument registers"
#endif /* HOST_REGISTER_R_ARG6 */
#endif /* HOST_REGISTER_R_ARG5 */
#endif /* HOST_REGISTER_R_ARG4 */
#endif /* HOST_REGISTER_R_ARG3 */
#endif /* HOST_REGISTER_R_ARG2 */
#endif /* HOST_REGISTER_R_ARG1 */
};
#endif /* HOST_REGISTER_R_ARG0 */

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
	CASE(DeeObject_CompareEq, 8);
	CASE(DeeObject_CompareNe, 8);
	CASE(DeeObject_CompareLo, 8);
	CASE(DeeObject_CompareLe, 8);
	CASE(DeeObject_CompareGr, 8);
	CASE(DeeObject_CompareGe, 8);
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
	CASE(DeeObject_Unpack, 12);
	CASE(DeeObject_NewDefault, 4);
	CASE(DeeError_Throw, 4);
	CASE(DeeObject_ConcatInherited, 8);
	CASE(DeeObject_ExtendInherited, 12);
	CASE(DeeTuple_FromSequence, 4);
	CASE(DeeList_FromSequence, 4);
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
gen86_symname(struct Dee_host_symbol const *sym) {
	static char buf[sizeof("[sect:+0x]") + 2 * (sizeof(void *) * 2)];
	if (sym->hs_name != NULL)
		return sym->hs_name;
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

INTERN NONNULL((1)) void DCALL
_Dee_memloc_debug_print(struct Dee_memloc const *__restrict self) {
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
_Dee_memobj_debug_print(struct Dee_memobj const *__restrict self, bool is_local, bool noref) {
	if (is_local && Dee_memobj_local_neverbound(self)) {
		Dee_DPRINT("-");
		return;
	}
	/* XXX: Print mv_obj.mvo_0.mo_typeof? */
	if (Dee_memobj_isref(self) && !noref)
		Dee_DPRINT("r");
	_Dee_memloc_debug_print(Dee_memobj_getloc(self));
	if (is_local) {
		if (Dee_memobj_local_alwaysbound(self)) {
			Dee_DPRINT("!");
		} else {
			Dee_DPRINT("?");
		}
	}
}

PRIVATE NONNULL((1)) void DCALL
_Dee_memval_debug_print_objects(struct Dee_memval const *__restrict self,
                                bool is_local) {
	if (MEMVAL_VMORPH_HASOBJ0(self->mv_vmorph)) {
		_Dee_memobj_debug_print(&self->mv_obj.mvo_0, is_local, false);
	} else {
		size_t i;
		struct Dee_memobjs *objs = Dee_memval_getobjn(self);
		for (i = 0; i < objs->mos_objc; ++i) {
			if (i != 0)
				Dee_DPRINT(",");
			_Dee_memobj_debug_print(&objs->mos_objv[i], false,
			                        (self->mv_flags & MEMVAL_F_NOREF) != 0);
		}
	}
}

INTERN NONNULL((1)) void DCALL
_Dee_memval_debug_print(struct Dee_memval const *__restrict self,
                        bool is_local) {
	switch (self->mv_vmorph) {
	case MEMVAL_VMORPH_DIRECT:
	case MEMVAL_VMORPH_DIRECT_01:
		_Dee_memval_debug_print_objects(self, is_local);
		break;
	case MEMVAL_VMORPH_BOOL_Z:
	case MEMVAL_VMORPH_BOOL_Z_01:
		_Dee_memval_debug_print_objects(self, is_local);
		Dee_DPRINT("==0");
		break;
	case MEMVAL_VMORPH_BOOL_NZ:
	case MEMVAL_VMORPH_BOOL_NZ_01:
		_Dee_memval_debug_print_objects(self, is_local);
		Dee_DPRINT("!=0");
		break;
	case MEMVAL_VMORPH_BOOL_LZ:
		_Dee_memval_debug_print_objects(self, is_local);
		Dee_DPRINT("<0");
		break;
	case MEMVAL_VMORPH_BOOL_GZ:
		_Dee_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(">0");
		break;
	case MEMVAL_VMORPH_INT:
		Dee_DPRINT("int(");
		_Dee_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(")");
		break;
	case MEMVAL_VMORPH_UINT:
		Dee_DPRINT("uint(");
		_Dee_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(")");
		break;
	case MEMVAL_VMORPH_NULLABLE:
		Dee_DPRINT("nullable(");
		_Dee_memval_debug_print_objects(self, is_local);
		Dee_DPRINT(")");
		break;
	default:
		Dee_DPRINTF("<%I8u:", self->mv_vmorph);
		_Dee_memval_debug_print_objects(self, is_local);
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
	ptr = DeeCode_FindDDI((DeeObject *)self, &st, NULL,
	                      code_addr, DDI_STATE_FNOTHROW);
	if (!DDI_ISOK(ptr))
		return NULL;
	result = NULL;
	DDI_STATE_DO(iter, &st) {
		if (lid < iter->dx_lcnamc) {
			result = DeeCode_GetDDIString((DeeObject *)self, iter->dx_lcnamv[lid]);
			if (result)
				goto done;
		}
	}
	DDI_STATE_WHILE(iter, &st);
done:
	Dee_ddi_state_fini(&st);
	return result;
}

INTERN NONNULL((1)) void DCALL
_Dee_memstate_debug_print(struct Dee_memstate const *__restrict self,
                          struct Dee_function_assembler *assembler,
                          Dee_instruction_t const *instr) {
	Dee_DPRINTF("\tCFA:   #%Id\n", (uintptr_t)self->ms_host_cfa_offset);
	if (self->ms_stackc > 0) {
		uint16_t i;
		Dee_DPRINT("\tstack: ");
		for (i = 0; i < self->ms_stackc; ++i) {
			if (i != 0)
				Dee_DPRINT(", ");
			_Dee_memval_debug_print(&self->ms_stackv[i], false);
		}
		Dee_DPRINT("\n");
	}
	if (self->ms_localc > 0) {
		Dee_lid_t i;
		bool is_first = true;
		for (i = 0; i < self->ms_localc; ++i) {
			char const *lid_name;
			if (Dee_memval_isdirect(&self->ms_localv[i]) &&
			    Dee_memval_direct_local_neverbound(&self->ms_localv[i]) &&
			    !Dee_memval_direct_isref(&self->ms_localv[i]))
				continue;
			Dee_DPRINT(is_first ? "\tlocal: " : ", ");
			lid_name = NULL;
			if (assembler && i < assembler->fa_localc && instr) {
				/* Lookup the name of the local in DDI */
				lid_name = DeeCode_LidNameAtAddr(assembler->fa_code, (uint16_t)i,
				                                 Dee_function_assembler_addrof(assembler, instr));
			} else if (assembler && i >= assembler->fa_localc) {
				Dee_lid_t xlid = i - assembler->fa_localc;
				switch (xlid) {
				case MEMSTATE_XLOCAL_A_THIS:
					lid_name = "@this";
					break;
				case MEMSTATE_XLOCAL_A_ARGC:
					lid_name = "@argc";
					break;
				case MEMSTATE_XLOCAL_A_ARGV:
					lid_name = "@argv";
					if (assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)
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
						Dee_aid_t aid = (Dee_aid_t)(xlid - MEMSTATE_XLOCAL_DEFARG_MIN);
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
			_Dee_memval_debug_print(&self->ms_localv[i], true);
			is_first = false;
		}
		if (!is_first)
			Dee_DPRINT("\n");
	}
}

INTERN NONNULL((1)) void DCALL
_Dee_memequivs_debug_print(struct Dee_memequivs const *__restrict self) {
	size_t i;
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct Dee_memequiv *iter, *eq = &self->meqs_list[i];
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
			_Dee_memequiv_debug_print(iter);
			iter->meq_loc.ml_adr.ma_typ |= 0x8000;
		} while ((iter = Dee_memequiv_next(iter)) != eq);
		Dee_DPRINT("}\n");
	}
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct Dee_memequiv *eq = &self->meqs_list[i];
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
Dee_function_generator_gadjust_reg_delta_impl(struct Dee_host_section *sect,
                                              struct Dee_function_generator *self, /* when NULL, don't adjust memory locations using `regno' */
                                              Dee_host_register_t regno, ptrdiff_t val_delta
#if defined(NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY) && !defined(NO_HOSTASM_DEBUG_PRINT)
                                              , bool log_instructions
#define LOCAL_HA_printf(...) (log_instructions ? gen86_printf(__VA_ARGS__) : (void)0)
#define Dee_function_generator_gadjust_reg_delta(sect, self, regno, val_delta, log_instructions) \
	Dee_function_generator_gadjust_reg_delta_impl(sect, self, regno, val_delta, log_instructions)
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY && !NO_HOSTASM_DEBUG_PRINT */
#define LOCAL_HA_printf(...) gen86_printf(__VA_ARGS__)
#define Dee_function_generator_gadjust_reg_delta(sect, self, regno, val_delta, log_instructions) \
	Dee_function_generator_gadjust_reg_delta_impl(sect, self, regno, val_delta)
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY || NO_HOSTASM_DEBUG_PRINT */
                                              ) {
	if unlikely(val_delta == 0)
		return 0;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	switch (val_delta) {
	case -1:
		LOCAL_HA_printf("dec" Plq "\t%s\n", gen86_regname(regno));
		gen86_decP_r(p_pc(sect), gen86_registers[regno]);
		break;
	case 1:
		LOCAL_HA_printf("inc" Plq "\t%s\n", gen86_regname(regno));
		gen86_incP_r(p_pc(sect), gen86_registers[regno]);
		break;
	default:
		/* TODO: Special handling on x86_64 when !fit32(val_delta) */
		LOCAL_HA_printf("lea" Plq "\t%Id(%s), %s\n", val_delta, gen86_regname(regno), gen86_regname(regno));
		gen86_leaP_db_r(p_pc(sect), val_delta, gen86_registers[regno], gen86_registers[regno]);
		break;
	}
	if (self != NULL)
		Dee_memstate_hregs_adjust_delta(self->fg_state, regno, val_delta);
	return 0;
err:
	return -1;
#undef LOCAL_HA_printf
}

#ifdef fit32_IS_1
#define Dee_function_generator_gadjust_reg_fit32(self, p_regno, p_val_delta)  0
#else /* fit32_IS_1 */
#define Dee_function_generator_gadjust_reg_fit32(self, p_regno, p_val_delta) \
	(fit32(*(p_val_delta)) ? 0 : _Dee_function_generator_gadjust_reg_fit32(self, p_regno, p_val_delta))
PRIVATE WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gadjust_reg_fit32(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t *__restrict p_regno,
                                          ptrdiff_t *__restrict p_val_delta) {
	int result;
	Dee_host_register_t regno = *p_regno;
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
	result = Dee_function_generator_gadjust_reg_delta(self->fg_sect, self, regno, val_delta, true);
	if likely(result == 0) {
		*p_val_delta = val_delta + adj_delta;
		ASSERT(fit32(*p_val_delta));
	}
	return result;
}
#endif /* !fit32_IS_1 */


PRIVATE WUNUSED NONNULL((1)) int DCALL
gcall86_impl(struct Dee_function_generator *__restrict self,
             void const *api_function
#if defined(NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY) && !defined(NO_HOSTASM_DEBUG_PRINT)
             , bool log_instructions
#define LOCAL_HA_printf(...) (log_instructions ? gen86_printf(__VA_ARGS__) : (void)0)
#define gcall86(self, api_function, log_instructions) \
	gcall86_impl(self, api_function, log_instructions)
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY && !NO_HOSTASM_DEBUG_PRINT */
#define LOCAL_HA_printf(...) gen86_printf(__VA_ARGS__)
#define gcall86(self, api_function, log_instructions) \
	gcall86_impl(self, api_function)
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY || NO_HOSTASM_DEBUG_PRINT */
             ) {
	struct Dee_host_section *sect = self->fg_sect;
	struct Dee_host_reloc *rel;

#ifdef DEE_FUNCTION_ASSEMBLER_F_MCLARGE
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_MCLARGE) {
		/* TODO: Instead of this:
		 * >> movabs $addr, %Pax
		 * >> callP  *%Pax
		 *
		 * Do this:
		 * >> callP  *.Laddr(%Pip)
		 * >> .pushsection .rodata
		 * >> .Laddr: .qword addr
		 * >> .popsection
		 *
		 * Where ".rodata" gets appended at the end of text.
		 * Or even better: add a special relocation for this
		 * case that looks at the delta during encode-time,
		 * and determines if a placement in .rodata is needed
		 */
		if unlikely(Dee_host_section_reqx86(sect, 2))
			goto err;
		LOCAL_HA_printf("movabs\t$%s, %%" Per "ax\n", gen86_addrname(api_function));
		gen86_movabs_imm_r(p_pc(sect), api_function, GEN86_R_PAX);
		LOCAL_HA_printf("call" Plq "\t*%%" Per "ax\n");
		gen86_callP_mod(p_pc(sect), gen86_modrm_r, GEN86_R_PAX);
		return 0;
	}
#endif /* DEE_FUNCTION_ASSEMBLER_F_MCLARGE */

	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	rel = Dee_host_section_newhostrel(sect);
	if unlikely(!rel)
		goto err;
	LOCAL_HA_printf("calll\t%s\n", gen86_addrname(api_function));
	gen86_calll_offset(p_pc(sect), -4);
	rel->hr_offset = (uint32_t)(p_off(sect) - 4);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
	rel->hr_vtype  = DEE_HOST_RELOCVALUE_ABS;
	rel->hr_value.rv_abs = (void *)api_function;
	return 0;
err:
	return -1;
#undef LOCAL_HA_printf
}



/* Object reference count incref/decref */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gincref_regx(struct Dee_function_generator *__restrict self,
                                     Dee_host_register_t regno,
                                     ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct Dee_host_section *sect;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf("incref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf("incref\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock inc" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf("lock inc" Plq "\tob_refcnt+%Id(%s)\n",
			             reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock add" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf("lock add" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n,
			             reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	sect = self->fg_sect;
	if unlikely(Dee_host_section_reqx86(sect, 2))
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
_Dee_function_generator_gdecref_nokill_regx(struct Dee_function_generator *__restrict self,
                                            Dee_host_register_t regno,
                                            ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct Dee_host_section *sect = self->fg_sect;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf("decref_nokill\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf("decref_nokill\t%Id(%s)",
		             reg_offset - OFFSETOF_ob_refcnt,
		             gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf("lock dec" Plq "\tob_refcnt+%Id(%s)\n", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock sub" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf("lock sub" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n, reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	if unlikely(Dee_host_section_reqx86(sect, 2))
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
_Dee_function_generator_gdestroy_regx(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t regno, ptrdiff_t reg_offset,
                                      bool do_kill) {
	Dee_host_register_t used_regno = regno;
	struct Dee_host_section *sect = self->fg_sect;
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
	if (used_regno != HOST_REGISTER_R_ARG0) {
		ptrdiff_t adj_delta;
		if unlikely(Dee_host_section_reqx86(sect, 1))
			goto err;
		adj_delta = reg_offset;
		if (adj_delta < INT32_MIN) {
			adj_delta = INT32_MIN;
		} else if (adj_delta > INT32_MAX) {
			adj_delta = INT32_MAX;
		}
		IF_VERBOSE_REFCNT_LOGGING(
		gen86_printf("lea" Plq "\t%Id(%s), %s\n",
		             adj_delta, gen86_regname(used_regno),
		             gen86_regname(HOST_REGISTER_R_ARG0)));
		gen86_leaP_db_r(p_pc(sect),
		                adj_delta, gen86_registers[used_regno],
		                gen86_registers[HOST_REGISTER_R_ARG0]);
		reg_offset -= adj_delta;
		used_regno = HOST_REGISTER_R_ARG0;
	}
#endif /* HOST_REGISTER_R_ARG0 */
	if unlikely(Dee_function_generator_gadjust_reg_delta(self->fg_sect, NULL, used_regno, reg_offset,
	                                                     !IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY))
		goto err;

	{
		size_t req_instructions = 0;
		if (do_kill)
			req_instructions += 1; /* movP $0, ob_refcnt(%used_regno) */
#ifndef HOST_REGISTER_R_ARG0
		req_instructions += 1; /* pushP %used_regno */
#endif /* !HOST_REGISTER_R_ARG0 */
#ifdef HOSTASM_X86_64_MSABI
		req_instructions += 1; /* subP $32, %Psp */
#endif /* HOSTASM_X86_64_MSABI */
#ifdef DEE_FUNCTION_ASSEMBLER_F_MCLARGE
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_MCLARGE)
			req_instructions += 1; /* movabs $DeeObject_Destroy, %Pax */
#endif /* DEE_FUNCTION_ASSEMBLER_F_MCLARGE */
		req_instructions += 1; /* call DeeObject_Destroy */
#ifdef HOSTASM_X86_64_MSABI
		req_instructions += 1; /* addP $32, %Psp */
#endif /* HOSTASM_X86_64_MSABI */
		if unlikely(Dee_host_section_reqx86(sect, req_instructions))
			goto err;
	}

	if (do_kill) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("mov" Plq "\t$0, ob_refcnt%+Id(%s)\n",
		                                       reg_offset - OFFSETOF_ob_refcnt,
		                                       gen86_regname(used_regno)));
		gen86_movP_imm_mod(p_pc(self->fg_sect), gen86_modrm_db, 0,
		                   reg_offset + OFFSETOF_ob_refcnt,
		                   gen86_registers[used_regno]);
	}

#ifndef HOST_REGISTER_R_ARG0
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("push" Plq "\t%s\n", gen86_regname(used_regno)));
	gen86_pushP_r(p_pc(sect), gen86_registers[used_regno]);
	Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
#endif /* !HOST_REGISTER_R_ARG0 */

#ifdef HOSTASM_X86_64_MSABI
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("sub" Plq "\t$32, %%" Per "sp\n"));
	gen86_subP_imm_r(p_pc(sect), 32, GEN86_R_PSP); /* TODO: Do this better */
	Dee_function_generator_gadjust_cfa_offset(self, 32);
#endif /* HOSTASM_X86_64_MSABI */

	/* Make the call to `DeeObject_Destroy()' */
	if unlikely(gcall86(self, (void const *)&DeeObject_Destroy,
	                    !IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY))
		goto err;
#ifndef HOST_REGISTER_R_ARG0
	Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
#endif /* !HOST_REGISTER_R_ARG0 */

#ifdef HOSTASM_X86_64_MSABI
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("add" Plq "\t$32, %%" Per "sp\n"));
	gen86_addP_imm_r(p_pc(sect), 32, GEN86_R_PSP); /* TODO: Do this better */
	Dee_function_generator_gadjust_cfa_offset(self, -32);
#endif /* !HOSTASM_X86_64_MSABI */

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

	/* Unused registers are now undefined (used registers were saved and restored) */
	Dee_function_generator_remember_undefined_unusedregs(self);
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
		if (!Dee_memstate_hregs_isused(state, save_regno))
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
_Dee_function_generator_gdecref_regx_impl(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t regno,
                                          ptrdiff_t ob_refcnt_offset, Dee_refcnt_t n) {
	struct Dee_host_section *text = self->fg_sect;
	struct Dee_host_section *cold = &self->fg_block->bb_hcold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if unlikely(Dee_function_generator_gadjust_reg_delta(self->fg_sect, self, regno, ob_refcnt_offset,
	                                                     !IS_DEFINED_NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY))
		goto err;
	if unlikely(Dee_host_section_reqx86(text, 3))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (n == 1) {
		if (ob_refcnt_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf("lock dec" Plq "\tob_refcnt+%Id(%s)\n", ob_refcnt_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (ob_refcnt_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock sub" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf("lock sub" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n, ob_refcnt_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
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
		struct Dee_host_symbol *sym_1f;
		struct Dee_host_reloc *rel;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jnz8\t1f\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_NZ, -1);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		sym_1f = Dee_function_generator_newsym(self);
		if unlikely(!sym_1f)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = sym_1f;
		ob_refcnt_offset -= OFFSETOF_ob_refcnt;
		if unlikely(_Dee_function_generator_gdestroy_regx(self, regno, ob_refcnt_offset, false))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
		_Dee_host_symbol_setsect(sym_1f, text);
	} else {
		struct Dee_host_reloc *enter_rel, *leave_rel;
		struct Dee_host_symbol *enter_sym, *leave_sym;

		/* Generate code that jumps into the cold section when the reference counter became `0'. */
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jzl\t1f\n"));
		gen86_jccl_offset(p_pc(text), GEN86_CC_Z, -4);
		enter_rel = Dee_host_section_newhostrel(text);
		if unlikely(!enter_rel)
			goto err;
		enter_sym = Dee_function_generator_newsym(self);
		if unlikely(!enter_sym)
			goto err;
		enter_rel->hr_offset = (uint32_t)(p_off(text) - 4);
		enter_rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		enter_rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		enter_rel->hr_value.rv_sym = enter_sym;

		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(".section .cold\n"));
		self->fg_sect = cold;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
		_Dee_host_symbol_setsect(enter_sym, cold);

		/* Generate the destroy-code *within* the cold section. */
		ob_refcnt_offset -= OFFSETOF_ob_refcnt;
		if unlikely(_Dee_function_generator_gdestroy_regx(self, regno, ob_refcnt_offset, false))
			goto err;

		/* Generate code to jump from the cold section back to the normal section. */
		if unlikely(Dee_host_section_reqx86(cold, 1))
			goto err;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jmpl\t1f\n"));
		gen86_jmpl_offset(p_pc(cold), -4);
		leave_rel = Dee_host_section_newhostrel(cold);
		if unlikely(!leave_rel)
			goto err;
		leave_sym = Dee_function_generator_newsym(self);
		if unlikely(!leave_sym)
			goto err;
		leave_rel->hr_offset = (uint32_t)(p_off(cold) - 4);
		leave_rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		leave_rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		leave_rel->hr_value.rv_sym = leave_sym;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf(".section .text\n"));
		self->fg_sect = text;
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
		Dee_host_symbol_setsect(leave_sym, text);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gdecref_regx(struct Dee_function_generator *__restrict self,
                                     Dee_host_register_t regno, ptrdiff_t reg_offset,
                                     Dee_refcnt_t n) {
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf("decref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf("decref\t%Id(%s)", reg_offset, gen86_regname(regno));
	}
	log_compact_decref_register_preserve_list(self, regno);
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		if (reg_offset == 0) {
			gen86_printf("lock dec" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf("lock dec" Plq "\tob_refcnt+%Id(%s)\n", reg_offset, gen86_regname(regno));
		}
	} else {
		if (reg_offset == 0) {
			gen86_printf("lock sub" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf("lock sub" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n, reg_offset, gen86_regname(regno));
		}
	}
#endif /* !NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	return _Dee_function_generator_gdecref_regx_impl(self, regno, reg_offset + OFFSETOF_ob_refcnt, n);
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gdecref_regx_dokill(struct Dee_function_generator *__restrict self,
                                            Dee_host_register_t regno, ptrdiff_t reg_offset) {
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == 0) {
		gen86_printf("decref_dokill\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf("decref_dokill\t%Id(%s)", reg_offset, gen86_regname(regno));
	}
	log_compact_decref_register_preserve_list(self, regno);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	return _Dee_function_generator_gdestroy_regx(self, regno, reg_offset, true);
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gxincref_regx(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t regno,
                                      ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct Dee_host_section *sect = self->fg_sect;
	uint8_t reg86;
	uintptr_t jcc8_offset;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 4))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf("xincref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf("xincref\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	reg86 = gen86_registers[regno];
	if (reg_offset == OFFSETOF_ob_refcnt) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(sect), reg86, reg86);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("cmp" Plq "\t$%Id, %s\n", -reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(sect), -reg_offset - OFFSETOF_ob_refcnt, reg86);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jz8\t1f\n"));
	gen86_jcc8_offset(p_pc(sect), GEN86_CC_Z, -1);
	jcc8_offset = p_off(sect) - 1;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (n == 1) {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock inc" Plq "\tob_refcnt(%s)\n", gen86_regname(regno));
		} else {
			gen86_printf("lock inc" Plq "\tob_refcnt+%Id(%s)\n",
			             reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
		}
	} else {
		if (reg_offset == OFFSETOF_ob_refcnt) {
			gen86_printf("lock add" Plq "\t$%Iu, ob_refcnt(%s)\n", n, gen86_regname(regno));
		} else {
			gen86_printf("lock add" Plq "\t$%Iu, ob_refcnt+%Id(%s)\n", n,
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
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
	return 0;
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gxdecref_nokill_regx(struct Dee_function_generator *__restrict self,
                                             Dee_host_register_t regno,
                                             ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct Dee_host_section *sect = self->fg_sect;
	uintptr_t jcc8_offset;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 4))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf("xdecref_nokill\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf("xdecref_nokill\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	if (reg_offset == OFFSETOF_ob_refcnt) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(sect), gen86_registers[regno], gen86_registers[regno]);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("cmp" Plq "\t$%Id, %s\n", -reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(sect), -reg_offset - OFFSETOF_ob_refcnt, gen86_registers[regno]);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jz8\t1f\n"));
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
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gxdecref_regx(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t regno,
                                      ptrdiff_t reg_offset, Dee_refcnt_t n) {
	struct Dee_host_section *sect = self->fg_sect;
	struct Dee_host_symbol *sym_1f;
	struct Dee_host_reloc *rel;
	uint8_t reg86;
	reg_offset += OFFSETOF_ob_refcnt;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &regno, &reg_offset))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	if (reg_offset == OFFSETOF_ob_refcnt) {
		gen86_printf("xdecref\t(%s)", gen86_regname(regno));
	} else {
		gen86_printf("xdecref\t%Id(%s)", reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno));
	}
	log_compact_decref_register_preserve_list(self, regno);
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	reg86 = gen86_registers[regno];
	if (reg_offset == OFFSETOF_ob_refcnt) {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("test" Plq "\t%s, %s\n", gen86_regname(regno), gen86_regname(regno)));
		gen86_testP_r_r(p_pc(sect), reg86, reg86);
	} else {
		IF_VERBOSE_REFCNT_LOGGING(gen86_printf("cmp" Plq "\t$%Id, %s\n", -reg_offset - OFFSETOF_ob_refcnt, gen86_regname(regno)));
		gen86_cmpP_imm_r(p_pc(sect), -reg_offset - OFFSETOF_ob_refcnt, reg86);
	}
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("jz8\t1f\n"));
	gen86_jcc8_offset(p_pc(sect), GEN86_CC_Z, -1);
	rel = Dee_host_section_newhostrel(sect);
	if unlikely(!rel)
		goto err;
	sym_1f = Dee_function_generator_newsym(self);
	if unlikely(!sym_1f)
		goto err;
	rel->hr_offset = (uint32_t)(p_off(sect) - 1);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
	rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
	rel->hr_value.rv_sym = sym_1f;
	if unlikely(_Dee_function_generator_gdecref_regx_impl(self, regno, reg_offset, n))
		goto err;
	IF_VERBOSE_REFCNT_LOGGING(gen86_printf("1:\n"));
	_Dee_host_symbol_setsect(sym_1f, sect);
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gincref_const(struct Dee_host_section *__restrict self,
                                DeeObject *value, Dee_refcnt_t n) {
	ptrdiff_t caddr = (ptrdiff_t)(uintptr_t)&value->ob_refcnt;
#ifdef _Dee_host_section_gincref_const_MAYFAIL
	if (!fit32(caddr))
		return 1;
#endif /* _Dee_host_section_gincref_const_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 2))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	gen86_printf("incref\t%s", gen86_addrname(value));
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINTF("\t# %r\n", value);
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		gen86_printf("lock inc" Plq "\t%#Ix\n", caddr);
	} else {
		gen86_printf("lock add" Plq "\t$%Iu, %#Ix\n", n, caddr);
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
_Dee_host_section_gdecref_const(struct Dee_host_section *__restrict self,
                                DeeObject *value, Dee_refcnt_t n) {
	ptrdiff_t caddr = (ptrdiff_t)(uintptr_t)&value->ob_refcnt;
#ifdef _Dee_host_section_gdecref_const_MAYFAIL
	if (!fit32(caddr))
		return 1;
#endif /* _Dee_host_section_gdecref_const_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 2))
		goto err;
	/* Constants can never be destroyed, so decref'ing one is
	 * like `Dee_DecrefNoKill()' (iow: doesn't need a zero-check) */
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
	gen86_printf("decref_nokill\t%s", gen86_addrname(value));
	if (n != 1)
		Dee_DPRINTF(", $%Iu", n);
	Dee_DPRINTF("\t# %r\n", value);
#else /* NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY */
	if (n == 1) {
		gen86_printf("lock dec" Plq "\t%#Ix\n", caddr);
	} else {
		gen86_printf("lock sub" Plq "\t$%Iu, %#Ix\n", n, caddr);
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




#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
#define IF_VERBOSE_LOCK_LOGGING(x) /* nothing */
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#define IF_VERBOSE_LOCK_LOGGING(x) x
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
_Dee_function_generator_gpause_or_yield(struct Dee_function_generator *__restrict self,
                                        Dee_host_register_t always_preserve_regno) {
#ifdef __NO_hybrid_yield
	if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
		goto err;
	gen86_pause(p_pc(self->fg_sect));
#else /* __NO_hybrid_yield */
	Dee_host_register_t regno;
	struct Dee_memstate *state = self->fg_state;
	struct Dee_host_section *sect;

	/* Push registers that need to be preserved. */
	for (regno = 0; regno < HOST_REGISTER_COUNT; ++regno) {
		if (regno == HOST_REGISTER_PAX)
			continue;
		if (state->ms_rinuse[regno] == 0 && regno != always_preserve_regno)
			continue;
		if unlikely(Dee_function_generator_ghstack_pushreg(self, regno))
			goto err;
	}
	sect = self->fg_sect;

	/* Make the call to the host's `sched_yield(2)' function */
#ifdef rt_sched_yield_IS_SleepEx
	if unlikely(Dee_host_section_reqx86(sect, 2))
		goto err;
#ifdef HOSTASM_X86_64
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(HOST_REGISTER_R_ARG0), gen86_regname(HOST_REGISTER_R_ARG0)));
	gen86_xorP_r_r(p_pc(sect), gen86_registers[HOST_REGISTER_R_ARG0], gen86_registers[HOST_REGISTER_R_ARG0]);
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(HOST_REGISTER_R_ARG1), gen86_regname(HOST_REGISTER_R_ARG1)));
	gen86_xorP_r_r(p_pc(sect), gen86_registers[HOST_REGISTER_R_ARG1], gen86_registers[HOST_REGISTER_R_ARG1]);
#else /* HOSTASM_X86_64 */
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("push" Plq "\t$0\n"));
	gen86_pushP_imm(p_pc(sect), 0);
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("push" Plq "\t$0\n"));
	gen86_pushP_imm(p_pc(sect), 0);
#endif /* !HOSTASM_X86_64 */

#ifdef HOSTASM_X86_64_MSABI
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("sub" Plq "\t$32, %%" Per "sp\n"));
	gen86_subP_imm_r(p_pc(sect), 32, GEN86_R_PSP); /* TODO: Do this better */
	Dee_function_generator_gadjust_cfa_offset(self, 32);
#endif /* HOSTASM_X86_64_MSABI */
	if unlikely(gcall86(self, (void const *)&SleepEx, true))
		goto err;
#ifdef HOSTASM_X86_64_MSABI
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("add" Plq "\t$32, %%" Per "sp\n"));
	gen86_subP_imm_r(p_pc(sect), 32, GEN86_R_PSP); /* TODO: Do this better */
	Dee_function_generator_gadjust_cfa_offset(self, -32);
#endif /* HOSTASM_X86_64_MSABI */
#else /* rt_sched_yield_IS_SleepEx */
	if unlikely(gcall86(self, (void const *)&rt_sched_yield, true))
		goto err;
#endif /* !rt_sched_yield_IS_SleepEx */

	/* Pop saved registers. */
	regno = HOST_REGISTER_COUNT;
	while (regno) {
		--regno;
		if (regno == HOST_REGISTER_PAX)
			continue;
		if (state->ms_rinuse[regno] == 0 && regno != always_preserve_regno)
			continue;
		if unlikely(Dee_function_generator_ghstack_popreg(self, regno))
			goto err;
	}
#endif /* !__NO_hybrid_yield */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_read_impl2(struct Dee_function_generator *__restrict self,
                                           struct Dee_memloc *__restrict loc,
                                           Dee_host_register_t tempreg) {
#define LOCAL_loc_isreg()  (loc->ml_adr.ma_typ == MEMADR_TYPE_HREG)
#define LOCAL_loc_regno()  (loc->ml_adr.ma_reg)
#define LOCAL_loc_reg86()  (gen86_registers[loc->ml_adr.ma_reg])
#define LOCAL_loc_regnam() gen86_regname(loc->ml_adr.ma_reg)
#define LOCAL_loc_regbas() Dee_memloc_getoff(loc)
#define LOCAL_loc_regoff() (Dee_memloc_getoff(loc) + (ptrdiff_t)offsetof(Dee_atomic_rwlock_t, arw_lock))
#define LOCAL_loc_const()  ((intptr_t)(uintptr_t)&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)
	struct Dee_host_symbol *text_Lfull_retry;
	struct Dee_host_symbol *text_LPax_retry;
	struct Dee_host_reloc *rel;
	struct Dee_host_symbol *cold_Lpause_and_full_retry;
	struct Dee_host_symbol *text_Ldone;
	struct Dee_host_section *text = self->fg_sect;
	struct Dee_host_section *cold = &self->fg_block->bb_hcold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if unlikely(Dee_host_section_reqx86(text, 6))
		goto err;
	cold_Lpause_and_full_retry = NULL;
	text_Ldone                 = NULL;
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		cold_Lpause_and_full_retry = Dee_function_generator_newsym(self);
		if unlikely(!cold_Lpause_and_full_retry)
			goto err;
	} else if (text == cold) {
		text_Ldone = Dee_function_generator_newsym(self);
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
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(".Lfull_retry:\n"));
	text_Lfull_retry = Dee_function_generator_newsym(self);
	if unlikely(!text_Lfull_retry)
		goto err;
	_Dee_host_symbol_setsect(text_Lfull_retry, text);
	if (LOCAL_loc_isreg()) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("mov" Plq "\tarw_lock%+Id(%s), %%" Per "ax\n", LOCAL_loc_regbas(), LOCAL_loc_regnam()));
		gen86_movP_db_r(p_pc(text), LOCAL_loc_regoff(), LOCAL_loc_reg86(), GEN86_R_PAX);
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("mov" Plq "\t%#Ix, %%" Per "ax\n", LOCAL_loc_const()));
		gen86_movP_d_r(p_pc(text), LOCAL_loc_const(), GEN86_R_PAX);
	}
	text_LPax_retry = NULL;
	if (text != cold && (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		text_LPax_retry = Dee_function_generator_newsym(self);
		if unlikely(!text_LPax_retry)
			goto err;
		_Dee_host_symbol_setsect(text_LPax_retry, text);
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
		gen86_printf(".LPax_retry:");
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	}
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("cmp" Plq "\t$-1, %%" Per "ax\n"));
	gen86_cmpP_imm_Pax(p_pc(text), -1);
	if (cold_Lpause_and_full_retry && text != cold) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("jel\t.Lpause_and_full_retry\n"));
		gen86_jccl_offset(p_pc(text), GEN86_CC_E, -4);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 4);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
	} else if (cold_Lpause_and_full_retry) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("je8\t.Lpause_and_full_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_E, -1);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("je8\t.Lfull_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_E, -1);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		ASSERT(text_Lfull_retry);
		rel->hr_value.rv_sym = text_Lfull_retry;
	}

	IF_VERBOSE_LOCK_LOGGING(gen86_printf("lea" Plq "\t1(%%" Per "ax), %s\n", gen86_regname(tempreg)));
	gen86_leaP_db_r(p_pc(text), 1, GEN86_R_PAX, gen86_registers[tempreg]);

	if (LOCAL_loc_isreg()) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("lock cmpxchg" Plq "\t%s, arw_lock%+Id(%s)\n",
		                                     gen86_regname(tempreg), LOCAL_loc_regbas(), LOCAL_loc_regnam()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_db, gen86_registers[tempreg],
		                     LOCAL_loc_regoff(), LOCAL_loc_reg86());
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("lock cmpxchg" Plq "\t%s, %#Ix\n", gen86_regname(tempreg), LOCAL_loc_const()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_d, gen86_registers[tempreg], LOCAL_loc_const());
	}

	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("jnz8\t.LPax_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_NZ, -1);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		ASSERT(text_LPax_retry);
		rel->hr_value.rv_sym = text_LPax_retry;
	} else if (text == cold) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("jz8\t.Ldone\n"));
		ASSERT(text_Ldone != NULL);
		gen86_jcc8_offset(p_pc(text), GEN86_CC_Z, -1);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = text_Ldone;
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("jnzl\t.Lpause_and_full_retry\n"));
		ASSERT(cold_Lpause_and_full_retry != NULL);
		gen86_jccl_offset(p_pc(text), GEN86_CC_NZ, -4);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 4);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
	}

	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		ASSERT(cold_Lpause_and_full_retry);
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifndef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
		if (text != cold)
			gen86_printf(".section .cold\n");
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		self->fg_sect = cold;
		_Dee_host_symbol_setsect(cold_Lpause_and_full_retry, cold);
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("Lpause_and_full_retry:\n"));
		if unlikely(_Dee_function_generator_gpause_or_yield(self,
		                                                    LOCAL_loc_isreg()
		                                                    ? LOCAL_loc_regno()
		                                                    : HOST_REGISTER_COUNT))
			goto err;
		if unlikely(Dee_host_section_reqx86(cold, 1))
			goto err;
		if (text == cold) {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf("jmp8\t.Lfull_retry\n"));
			gen86_jmp8_offset(p_pc(text), -1);
			rel = Dee_host_section_newhostrel(text);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(text) - 1);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			ASSERT(text_Lfull_retry);
			rel->hr_value.rv_sym = text_Lfull_retry;
		} else {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf("jmpl\t.Lfull_retry\n"));
			gen86_jmpl_offset(p_pc(cold), -4);
			rel = Dee_host_section_newhostrel(cold);
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
			gen86_printf(".section .text\n");
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		self->fg_sect = text;
	}
	if (text_Ldone != NULL) {
		_Dee_host_symbol_setsect(text_Ldone, text);
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("Ldone:\n"));
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
_Dee_function_generator_grwlock_write_impl2(struct Dee_function_generator *__restrict self,
                                            struct Dee_memloc const *__restrict loc,
                                            Dee_host_register_t tempreg) {
#define LOCAL_loc_isreg()  (loc->ml_adr.ma_typ == MEMADR_TYPE_HREG)
#define LOCAL_loc_regno()  (loc->ml_adr.ma_reg)
#define LOCAL_loc_reg86()  (gen86_registers[loc->ml_adr.ma_reg])
#define LOCAL_loc_regnam() gen86_regname(loc->ml_adr.ma_reg)
#define LOCAL_loc_regbas() Dee_memloc_getoff(loc)
#define LOCAL_loc_regoff() (Dee_memloc_getoff(loc) + (ptrdiff_t)offsetof(Dee_atomic_rwlock_t, arw_lock))
#define LOCAL_loc_const()  ((intptr_t)(uintptr_t)&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)
	struct Dee_host_symbol *text_Lfull_retry;
	struct Dee_host_section *text = self->fg_sect;
	if unlikely(Dee_host_section_reqx86(text, 4))
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
	IF_VERBOSE_LOCK_LOGGING(gen86_printf(".Lfull_retry:\n"));
	text_Lfull_retry = Dee_function_generator_newsym(self);
	if unlikely(!text_Lfull_retry)
		goto err;
	_Dee_host_symbol_setsect(text_Lfull_retry, text);

	IF_VERBOSE_LOCK_LOGGING(gen86_printf("xor" Plq "\t%%" Per "ax, %%" Per "ax\n"));
	gen86_xorP_r_r(p_pc(text), GEN86_R_PAX, GEN86_R_PAX);
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("mov" Plq "\t$-1, %s\n", gen86_regname(tempreg)));
	gen86_movP_imm_r(p_pc(text), -1, gen86_registers[tempreg]);
	if (LOCAL_loc_isreg()) {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("lock cmpxchg" Plq "\t%s, arw_lock%+Id(%s)\n",
		                                     gen86_regname(tempreg), LOCAL_loc_regbas(), LOCAL_loc_regnam()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_db, gen86_registers[tempreg],
		                     LOCAL_loc_regoff(), LOCAL_loc_reg86());
	} else {
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("lock cmpxchg" Plq "\t%s, %#Ix\n", gen86_regname(tempreg), LOCAL_loc_const()));
		gen86_lock(p_pc(text));
		gen86_cmpxchgP_r_mod(p_pc(text), gen86_modrm_d, gen86_registers[tempreg], LOCAL_loc_const());
	}

	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE) {
		struct Dee_host_reloc *rel;
		/* jnz8  .Lfull_retry */
		IF_VERBOSE_LOCK_LOGGING(gen86_printf("jnz8\t.Lfull_retry\n"));
		gen86_jcc8_offset(p_pc(text), GEN86_CC_NZ, -1);
		rel = Dee_host_section_newhostrel(text);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = (uint32_t)(p_off(text) - 1);
		rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
		rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
		ASSERT(text_Lfull_retry);
		rel->hr_value.rv_sym = text_Lfull_retry;
	} else {
		struct Dee_host_symbol *text_Ldone;
		struct Dee_host_reloc *rel;
		struct Dee_host_section *cold = &self->fg_block->bb_hcold;
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
			cold = text;
		text_Ldone = NULL;
		if (cold == text) {
			text_Ldone = Dee_function_generator_newsym(self);
			if unlikely(!text_Ldone)
				goto err;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf("jz8\t.Ldone\n"));
			gen86_jcc8_offset(p_pc(text), GEN86_CC_Z, -1);
			rel = Dee_host_section_newhostrel(text);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(text) - 1);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = text_Ldone;
		} else {
			struct Dee_host_symbol *cold_Lpause_and_full_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf("jnzl\t.Lpause_and_full_retry\n"));
			cold_Lpause_and_full_retry = Dee_function_generator_newsym(self);
			if unlikely(!cold_Lpause_and_full_retry)
				goto err;
			gen86_jccl_offset(p_pc(text), GEN86_CC_NZ, -4);
			rel = Dee_host_section_newhostrel(text);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(text) - 4);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = cold_Lpause_and_full_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(".section .cold\n"));
			self->fg_sect = cold;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(".Lpause_and_full_retry:\n"));
			_Dee_host_symbol_setsect(cold_Lpause_and_full_retry, cold);
		}
		if unlikely(_Dee_function_generator_gpause_or_yield(self,
		                                                    LOCAL_loc_isreg()
		                                                    ? LOCAL_loc_regno()
		                                                    : HOST_REGISTER_COUNT))
			goto err;
		if unlikely(Dee_host_section_reqx86(cold, 1))
			goto err;
		if (text == cold) {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf("jmp8\t.Lfull_retry\n"));
			gen86_jmp8_offset(p_pc(cold), -1);
			rel = Dee_host_section_newhostrel(cold);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(cold) - 1);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL8;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = text_Lfull_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(".Ldone:\n"));
			ASSERT(text_Ldone);
			_Dee_host_symbol_setsect(text_Ldone, text);
		} else {
			IF_VERBOSE_LOCK_LOGGING(gen86_printf("jmpl\t.Lfull_retry\n"));
			gen86_jmpl_offset(p_pc(cold), -4);
			rel = Dee_host_section_newhostrel(cold);
			if unlikely(!rel)
				goto err;
			rel->hr_offset = (uint32_t)(p_off(cold) - 4);
			rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
			rel->hr_vtype  = DEE_HOST_RELOCVALUE_SYM;
			rel->hr_value.rv_sym = text_Lfull_retry;
			IF_VERBOSE_LOCK_LOGGING(gen86_printf(".section .text\n"));
			self->fg_sect = text;
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
_Dee_function_generator_gpush_Pax_if_used(struct Dee_function_generator *__restrict self) {
	struct Dee_memstate *state = self->fg_state;
	if (state->ms_rinuse[HOST_REGISTER_PAX] == 0)
		return 1; /* Not in use */
	return Dee_function_generator_ghstack_pushreg(self, HOST_REGISTER_PAX);
}
#define _Dee_function_generator_gpop_Pax(self) \
	Dee_function_generator_ghstack_popreg(self, HOST_REGISTER_PAX)

#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_DECREF_ASSEMBLY
PRIVATE NONNULL((1)) void DCALL
log_compact_rwlock_register_preserve_list(struct Dee_function_generator *__restrict self) {
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t save_regno;
	bool is_first = true;

	/* Save registers. */
	for (save_regno = 0; save_regno < HOST_REGISTER_COUNT; ++save_regno) {
		if (!Dee_memstate_hregs_isused(state, save_regno))
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
_Dee_function_generator_gloc_no_Pax(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *__restrict loc) {
	if (loc->ml_adr.ma_typ == MEMADR_TYPE_HREG &&
	    loc->ml_adr.ma_reg == HOST_REGISTER_PAX) {
		/* We explicity *need* to use a register other than `%Pax' here! */
		Dee_host_register_t not_these[2], lockreg;
		struct Dee_memstate *state;
		not_these[0] = HOST_REGISTER_PAX;
		not_these[1] = HOST_REGISTER_COUNT;
		lockreg = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(lockreg >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_reg2reg(self, HOST_REGISTER_PAX, lockreg))
			goto err;
		state = self->fg_state;
		loc->ml_adr.ma_reg = lockreg;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_read_impl(struct Dee_function_generator *__restrict self,
                                          struct Dee_memloc const *__restrict loc) {
	Dee_host_register_t tempreg, not_these[3];
	struct Dee_memloc usedloc = *loc;
	int Pax_pushed;
	ASSERT(usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG ||
	       usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST);
	if unlikely(_Dee_function_generator_gloc_no_Pax(self, &usedloc))
		goto err;
	not_these[0] = HOST_REGISTER_PAX;
	not_these[1] = HOST_REGISTER_COUNT;
	not_these[2] = HOST_REGISTER_COUNT;
	if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG)
		not_these[1] = usedloc.ml_adr.ma_reg;
	tempreg = Dee_function_generator_gallocreg(self, not_these);
	if unlikely(tempreg >= HOST_REGISTER_COUNT)
		goto err;
	Pax_pushed = _Dee_function_generator_gpush_Pax_if_used(self);
	if unlikely(Pax_pushed < 0)
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST) {
		gen86_printf("rwlock_read\t%#Ix", usedloc.ml_adr.ma_val.v_const);
	} else {
		gen86_printf("rwlock_read\t%Id(%s)", Dee_memloc_getoff(&usedloc), gen86_regname(usedloc.ml_adr.ma_reg));
	}
	log_compact_rwlock_register_preserve_list(self);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	if unlikely(_Dee_function_generator_grwlock_read_impl2(self, &usedloc, tempreg))
		goto err;
	if (Pax_pushed == 0)
		return _Dee_function_generator_gpop_Pax(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_write_impl(struct Dee_function_generator *__restrict self,
                                           struct Dee_memloc const *__restrict loc) {
	Dee_host_register_t tempreg;
	struct Dee_memloc usedloc = *loc;
	int Pax_pushed;
	ASSERT(usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG ||
	       usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST);
	if unlikely(_Dee_function_generator_gloc_no_Pax(self, &usedloc))
		goto err;
	{
		Dee_host_register_t not_these[3];
		not_these[0] = HOST_REGISTER_PAX;
		not_these[1] = HOST_REGISTER_COUNT;
		not_these[2] = HOST_REGISTER_COUNT;
		if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_HREG)
			not_these[1] = usedloc.ml_adr.ma_reg;
		tempreg = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			goto err;
	}
	Pax_pushed = _Dee_function_generator_gpush_Pax_if_used(self);
	if unlikely(Pax_pushed < 0)
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	if (usedloc.ml_adr.ma_typ == MEMADR_TYPE_CONST) {
		gen86_printf("rwlock_write\t%#Ix", usedloc.ml_adr.ma_val.v_const);
	} else {
		gen86_printf("rwlock_write\t%Id(%s)", Dee_memloc_getoff(&usedloc), gen86_regname(usedloc.ml_adr.ma_reg));
	}
	log_compact_rwlock_register_preserve_list(self);
	Dee_DPRINT("\n");
#endif /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	if unlikely(_Dee_function_generator_grwlock_write_impl2(self, &usedloc, tempreg))
		goto err;
	if (Pax_pushed == 0)
		return _Dee_function_generator_gpop_Pax(self);
	return 0;
err:
	return -1;
}

/* Controls for operating with R/W-locks (as needed for accessing global/extern variables) */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endread_const(struct Dee_function_generator *__restrict self,
                                              Dee_atomic_rwlock_t *__restrict lock) {
	/* >> lock decP <lock->arw_lock> */
	struct Dee_host_section *sect = self->fg_sect;
	ASSERT(fit32(&lock->arw_lock));
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf("rwlock_endread\t%#Ix\n", lock);
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	gen86_printf("lock dec" Plq "\t%#Ix\n", &lock->arw_lock);
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	gen86_lock(p_pc(sect));
	gen86_decP_mod(p_pc(sect), gen86_modrm_d, (intptr_t)(uintptr_t)&lock->arw_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endwrite_const(struct Dee_function_generator *__restrict self,
                                               Dee_atomic_rwlock_t *__restrict lock) {
	/* >> movP  $0, <lock->arw_lock> */
	struct Dee_host_section *sect = self->fg_sect;
	ASSERT(fit32(&lock->arw_lock));
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf("rwlock_endwrite\t%#Ix\n", lock);
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	IF_VERBOSE_LOCK_LOGGING(gen86_printf("mov" Plq "\t$0, %#Ix\n", &lock->arw_lock));
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	gen86_movP_imm_d(p_pc(sect), 0, (intptr_t)(uintptr_t)&lock->arw_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_grwlock_endread_regx(struct Dee_function_generator *__restrict self,
                                             Dee_host_register_t regno, ptrdiff_t val_delta) {
	/* >> lock decP <lock->arw_lock> */
	struct Dee_host_section *sect = self->fg_sect;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf("rwlock_endread\t%Id(%s)\n", val_delta, gen86_regname(regno));
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	gen86_printf("lock dec" Plq "\tarw_lock%+Id(%s)\n", val_delta, gen86_regname(regno));
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	val_delta += offsetof(Dee_atomic_rwlock_t, arw_lock);
	gen86_decP_mod(p_pc(sect), gen86_modrm_db, val_delta, gen86_registers[regno]);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_grwlock_endwrite_regx(struct Dee_function_generator *__restrict self,
                                              Dee_host_register_t regno, ptrdiff_t val_delta) {
	/* >> movP  $0, <lock->arw_lock> */
	struct Dee_host_section *sect = self->fg_sect;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
#ifndef NO_HOSTASM_DEBUG_PRINT
#ifdef NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY
	gen86_printf("rwlock_endwrite\t%Id(%s)\n", val_delta, gen86_regname(regno));
#else /* NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
	gen86_printf("mov" Plq "\t$0, arw_lock%+Id(%s)\n", val_delta, gen86_regname(regno));
#endif /* !NO_HOSTASM_VERBOSE_LOCK_ASSEMBLY */
#endif /* NO_HOSTASM_DEBUG_PRINT */
	val_delta += offsetof(Dee_atomic_rwlock_t, arw_lock);
	gen86_movP_imm_db(p_pc(sect), 0, val_delta, gen86_registers[regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_read(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc const *__restrict loc) {
	struct Dee_memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (!fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)) {
	default:
			if unlikely(Dee_function_generator_gasreg(self, loc, &regloc, NULL))
				goto err;
			loc = &regloc;
		}
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _Dee_function_generator_grwlock_read_impl(self, loc);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_write(struct Dee_function_generator *__restrict self,
                                      struct Dee_memloc const *__restrict loc) {
	struct Dee_memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (!fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock)) {
	default:
			if unlikely(Dee_function_generator_gasreg(self, loc, &regloc, NULL))
				goto err;
			loc = &regloc;
		}
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _Dee_function_generator_grwlock_write_impl(self, loc);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endread(struct Dee_function_generator *__restrict self,
                                        struct Dee_memloc const *__restrict loc) {
	struct Dee_memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock))
			return _Dee_function_generator_grwlock_endread_const(self, (Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const);
		ATTR_FALLTHROUGH
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &regloc, NULL))
			goto err;
		loc = &regloc;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _Dee_function_generator_grwlock_endread_regx(self,
		                                                    loc->ml_adr.ma_reg,
		                                                    Dee_memloc_getoff(loc));
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endwrite(struct Dee_function_generator *__restrict self,
                                         struct Dee_memloc const *__restrict loc) {
	struct Dee_memloc regloc;
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_CONST:
		if (fit32(&((Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const)->arw_lock))
			return _Dee_function_generator_grwlock_endwrite_const(self, (Dee_atomic_rwlock_t *)loc->ml_adr.ma_val.v_const);
		ATTR_FALLTHROUGH
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &regloc, NULL))
			goto err;
		loc = &regloc;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return _Dee_function_generator_grwlock_endwrite_regx(self,
		                                                     loc->ml_adr.ma_reg,
		                                                     Dee_memloc_getoff(loc));
	}
	__builtin_unreachable();
err:
	return -1;
}




/* Allocate/deallocate memory from the host stack.
 * If stack memory gets allocated, zero-initialize it. */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_adjust(struct Dee_host_section *__restrict self,
                                 ptrdiff_t sp_delta) {
	ASSERTF(fit32(sp_delta),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if (sp_delta > 0) {
		/* Release stack memory. */
		ASSERT(IS_ALIGNED((uintptr_t)sp_delta, HOST_SIZEOF_POINTER));
		if unlikely(Dee_host_section_reqx86(self, 1))
			goto err;
		gen86_printf("add" Plq "\t$%Id, %%" Per "sp\n", sp_delta);
		gen86_addP_imm_r(p_pc(self), sp_delta, GEN86_R_PSP);
	} else if (sp_delta < 0) {
		/* Acquire stack memory. */
		ASSERT(IS_ALIGNED((uintptr_t)(-sp_delta), HOST_SIZEOF_POINTER));
		if unlikely(Dee_host_section_reqx86(self, 1))
			goto err;
		gen86_printf("sub" Plq "\t$%Id, %%" Per "sp\n", -sp_delta);
		gen86_subP_imm_r(p_pc(self), -sp_delta, GEN86_R_PSP);
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
_Dee_function_generator_ghstack_pushregind(struct Dee_function_generator *__restrict self,
                                           Dee_host_register_t src_regno, ptrdiff_t src_delta) {
	struct Dee_host_section *sect = self->fg_sect;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &src_regno, &src_delta))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	gen86_printf("push" Plq "\t%Id(%s)\n", src_delta, gen86_regname(src_regno));
	gen86_pushP_mod(p_pc(sect), gen86_modrm_db, src_delta, gen86_registers[src_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_pushconst(struct Dee_host_section *__restrict self,
                                    void const *value) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("push" Plq "\t$%#I32x\n", (uint32_t)(uintptr_t)value);
	gen86_pushP_imm(p_pc(self), (int32_t)(uint32_t)(uintptr_t)value);
#ifndef fit32_IS_1
	if (!fit32(value)) {
		/* Must manually write the high 32-bit of "value" to their proper place. */
		uint32_t hi32 = (uint32_t)((uint64_t)value >> 32);
		if unlikely(Dee_host_section_reqx86(self, 1))
			goto err;
		gen86_printf("movl\t$%#I32x, 4(%%" Per "sp)\n", hi32);
		gen86_movl_imm_db(p_pc(self), hi32, 4, GEN86_R_PSP);
	}
#endif /* fit32_IS_1 */
	return 0;
err:
	return -1;
}

/* Pushes the address of `(self)->fg_state->ms_host_cfa_offset' (as it was before the push) */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_pushhstack_at_cfa_boundary_np(struct Dee_host_section *__restrict self) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("push" Plq "\t%%" Per "sp\n");
	gen86_pushP_r(p_pc(self), GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_ghstack_pushhstackind(struct Dee_host_section *__restrict self,
                                        ptrdiff_t sp_offset) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
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
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
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
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
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
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
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
                                 void const *value, Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	if (value == NULL) {
		gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	} else {
		gen86_printf("mov" Plq "\t$%s, %s\n", gen86_addrname(value), gen86_regname(dst_regno));
		/* NOTE: This automatically does `movabs' on x86_64! */
		gen86_movP_imm_r(p_pc(self), (intptr_t)(uintptr_t)value, gen86_registers[dst_regno]);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmov_const2regind(struct Dee_function_generator *__restrict self, void const *value,
                                          Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	struct Dee_host_section *sect = self->fg_sect;
#ifdef _Dee_function_generator_gmov_const2regind_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _Dee_function_generator_gmov_const2regind_MAYFAIL */
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &dst_regno, &dst_delta))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	gen86_printf("mov" Plq "\t$%s, %Id(%s)\n", gen86_addrname(value), dst_delta, gen86_regname(dst_regno));
	gen86_movP_imm_db(p_pc(sect), (intptr_t)(uintptr_t)value, dst_delta, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_const2hstackind(struct Dee_host_section *__restrict self,
                                       void const *value, ptrdiff_t sp_offset) {
#ifdef _Dee_host_section_gmov_const2hstackind_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _Dee_host_section_gmov_const2hstackind_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t$%s, %Id(%%" Per "sp)\n", gen86_addrname(value), sp_offset);
	gen86_movP_imm_db(p_pc(self), (intptr_t)(uintptr_t)value, sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_const2constind(struct Dee_host_section *__restrict self,
                                      void const *value, void const **p_value) {
#ifdef _Dee_host_section_gmov_const2constind_MAYFAIL
	if (!fit32(value))
		return 1;
	if (!fit32(p_value))
		return 2;
#endif /* _Dee_host_section_gmov_const2constind_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t$%s, %#Ix\n", gen86_addrname(value), (intptr_t)(uintptr_t)p_value);
	gen86_movP_imm_d(p_pc(self), (intptr_t)(uintptr_t)value, (intptr_t)(uintptr_t)p_value);
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
	if (src_regno == dst_regno)
		return Dee_function_generator_gadjust_reg_delta(self, NULL, src_regno, src_delta, true);
#ifndef fit32_IS_1
	if (!fit32(src_delta)) {
		if unlikely(Dee_host_section_reqx86(self, 2))
			goto err;
		gen86_printf("movabs" Plq "\t%Id, %s\n", src_delta, gen86_regname(dst_regno));
		gen86_movabs_imm_r(p_pc(self), src_delta, gen86_registers[dst_regno]);
		gen86_printf("add" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_addP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
	} else
#endif /* !fit32_IS_1 */
	{
		if unlikely(Dee_host_section_reqx86(self, 1))
			goto err;
		gen86_printf("lea" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_db_r(p_pc(self), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmov_constind2reg(struct Dee_host_section *__restrict self,
                                    void const **p_value, Dee_host_register_t dst_regno) {
#ifdef _Dee_host_section_gmov_constind2reg_MAYFAIL
	if (!fit32(p_value))
		return 1;
#endif /* _Dee_host_section_gmov_constind2reg_MAYFAIL */
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
                                    Dee_host_register_t src_regno, void const **p_value) {
#ifdef _Dee_host_section_gmov_reg2constind_MAYFAIL
	if (!fit32(p_value))
		return 1;
#endif /* _Dee_host_section_gmov_reg2constind_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("mov" Plq "\t%s, %#Ix\n", gen86_regname(src_regno), (intptr_t)(uintptr_t)p_value);
	gen86_movP_r_d(p_pc(self), gen86_registers[src_regno], (intptr_t)(uintptr_t)p_value);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmov_regind2reg(struct Dee_function_generator *__restrict self,
                                        Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                        Dee_host_register_t dst_regno) {
	struct Dee_host_section *sect = self->fg_sect;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &src_regno, &src_delta))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	gen86_printf("mov" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
	gen86_movP_db_r(p_pc(sect), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmov_reg2regind(struct Dee_function_generator *__restrict self,
                                        Dee_host_register_t src_regno,
                                        Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	struct Dee_host_section *sect = self->fg_sect;
#ifndef fit32_IS_1
	if unlikely(src_regno == dst_regno && !fit32(dst_delta)) {
		/* Needs special handling when !fit32 */
		ptrdiff_t adj_delta;
		Dee_host_register_t new_dst_regno;
		Dee_host_register_t not_these[2];
		/* TODO: Check if there is a known register equivalence of `dst_regno'
		 *       with a value-delta that causes the final `val_delta' to
		 *       fit into 32 bit */
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		new_dst_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(new_dst_regno >= HOST_REGISTER_COUNT)
			goto err;
		if (dst_delta < INT32_MIN) {
			adj_delta = dst_delta - INT32_MIN;
		} else if (dst_delta > INT32_MAX) {
			adj_delta = dst_delta - INT32_MAX;
		} else {
			adj_delta = 0;
		}
		if unlikely(Dee_host_section_reqx86(sect, 1))
			goto err;
		HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(dst_regno), gen86_regname(new_dst_regno));
		gen86_leaP_db_r(p_pc(sect), adj_delta, gen86_registers[dst_regno], gen86_registers[new_dst_regno]);
		dst_regno = new_dst_regno;
	}
#endif /* !fit32_IS_1 */
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &dst_regno, &dst_delta))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	gen86_printf("mov" Plq "\t%s, %Id(%s)\n", gen86_regname(src_regno), dst_delta, gen86_regname(dst_regno));
	gen86_movP_r_db(p_pc(sect), gen86_registers[src_regno], dst_delta, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
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


/* Arithmetic helpers. */

/* dst_regno = src_regno1 + src_regno2; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gadd_regreg2reg(struct Dee_host_section *__restrict self,
                                  Dee_host_register_t src_regno1, Dee_host_register_t src_regno2,
                                  Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("lea" Plq "\t(%s,%s), %s\n", gen86_regname(src_regno1), gen86_regname(src_regno2), gen86_regname(dst_regno));
	gen86_leaP_bi_r(p_pc(self), gen86_registers[src_regno1], gen86_registers[src_regno2], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = src_regno * n; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gumul_regconst2reg(struct Dee_host_section *__restrict self,
                                     Dee_host_register_t src_regno, uintptr_t n,
                                     Dee_host_register_t dst_regno) {
	if (n == 1 && src_regno == dst_regno)
		return 0; /* Special case: nothing to do here. */
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	switch (n) {
	case 0:
		gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
		break;
	case 1:
		gen86_printf("mov" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_movP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case 2:
		gen86_printf("lea" Plq "\t(%s,%s), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bi_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], gen86_registers[dst_regno]);
		break;
	case 3:
		gen86_printf("lea" Plq "\t(%s,%s,2), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bis_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], 2, gen86_registers[dst_regno]);
		break;
	case 4:
		gen86_printf("lea" Plq "\t(,%s,4), %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_is_r(p_pc(self), gen86_registers[src_regno], 4, gen86_registers[dst_regno]);
		break;
	case 5:
		gen86_printf("lea" Plq "\t(%s,%s,4), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bis_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], 4, gen86_registers[dst_regno]);
		break;
	case 8:
		gen86_printf("lea" Plq "\t(%s,8), %s\n", gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_is_r(p_pc(self), gen86_registers[src_regno], 8, gen86_registers[dst_regno]);
		break;
	case 9:
		gen86_printf("lea" Plq "\t(%s,%s,8), %s\n", gen86_regname(src_regno), gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_leaP_bis_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno], 8, gen86_registers[dst_regno]);
		break;
	default:
		gen86_printf("imul" Plq "\t%$Id, %s, %s\n", (intptr_t)n, gen86_regname(src_regno), gen86_regname(dst_regno));
		gen86_imulP_imm_mod_r(p_pc(self), gen86_modrm_r, (intptr_t)n, gen86_registers[dst_regno], gen86_registers[src_regno]);
		break;
	}
	return 0;
err:
	return -1;
}


/* *(SP + sp_offset) = *(SP + sp_offset) + <value>; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gadd_const2hstackind(struct Dee_host_section *__restrict self,
                                       void const *value, ptrdiff_t sp_offset) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely((uintptr_t)value == 0)
		return 0;
#ifdef _Dee_host_section_gadd_const2hstackind_MAYFAIL
	if (!fit32(value))
		return 1;
#endif /* _Dee_host_section_gadd_const2hstackind_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("add" Plq "\t$%Id, %Id(%%" Per "sp)\n", (intptr_t)(uintptr_t)value, sp_offset);
	gen86_addP_imm_mod(p_pc(self), gen86_modrm_db, (intptr_t)(uintptr_t)value, sp_offset, GEN86_R_PSP);
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
_Dee_function_generator_gmorph_regx2reg01(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                          unsigned int cmp, Dee_host_register_t dst_regno) {
	struct Dee_host_section *sect = self->fg_sect;
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &src_regno, &src_delta))
		goto err;
	if unlikely(Dee_host_section_reqx86(sect, 3))
		goto err;
	if (src_regno != dst_regno) {
		gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(sect), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	}
	if (src_delta == 0 && (cmp == GMORPHBOOL_CC_EQ || cmp == GMORPHBOOL_CC_NE)) {
		gen86_printf("test" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(src_regno));
		gen86_testP_r_r(p_pc(sect), gen86_registers[src_regno], gen86_registers[src_regno]);
	} else {
		gen86_printf("cmp" Plq "\t$%Id, %s\n", -src_delta, gen86_regname(src_regno));
		gen86_cmpP_imm_r(p_pc(sect), -src_delta, gen86_registers[src_regno]);
	}
	if (src_regno == dst_regno) {
		gen86_printf("mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(sect), 0, gen86_registers[dst_regno]);
	}
	gen86_printf("set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(sect), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (src_regno + src_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmorph_regxCreg2reg01(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno,
                                        ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t rhs_regno,
                                        Dee_host_register_t dst_regno) {
	if (src_regno == rhs_regno) {
		uintptr_t value;
		switch (cmp) {
		case GMORPHBOOL_CC_EQ: value = src_delta == 0; break;
		case GMORPHBOOL_CC_NE: value = src_delta != 0; break;
		case GMORPHBOOL_CC_LO: value = src_delta < 0; break;
		case GMORPHBOOL_CC_GR: value = src_delta > 0; break;
		default: __builtin_unreachable();
		}
		return _Dee_host_section_gmov_const2reg(self, (void const *)(uintptr_t)(value ? 1 : 0), dst_regno);
	} else if (src_regno != dst_regno && rhs_regno != dst_regno && src_delta == 0) {
		if unlikely(Dee_host_section_reqx86(self, 3))
			goto err;
		gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
		gen86_printf("cmp" Plq "\t%s, %s\n", gen86_regname(rhs_regno), gen86_regname(src_regno));
		gen86_cmpP_r_r(p_pc(self), gen86_registers[rhs_regno], gen86_registers[src_regno]);
	} else {
		if unlikely(Dee_host_section_reqx86(self, (size_t)(src_delta ? 4 : 3)))
			goto err;
		if (src_delta != 0) {
			gen86_printf("lea" Plq "\t%Id(%s), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
			gen86_leaP_db_r(p_pc(self), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
			src_regno = dst_regno;
		}
		gen86_printf("cmp" Plq "\t%s, %s\n", gen86_regname(rhs_regno), gen86_regname(src_regno));
		gen86_cmpP_r_r(p_pc(self), gen86_registers[rhs_regno], gen86_registers[src_regno]);
		gen86_printf("mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(self), 0, gen86_registers[dst_regno]);
	}
	gen86_printf("set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(self), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (*(src_regno86 + ind_delta) + val_delta) <CMP> 0 ? 1 : 0; */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_gmorph_reg86ind2reg01(struct Dee_host_section *__restrict self,
                           uint8_t src_regno86, ptrdiff_t ind_delta,
                           ptrdiff_t val_delta, unsigned int cmp,
                           Dee_host_register_t dst_regno) {
	if unlikely(Dee_host_section_reqx86(self, 3))
		goto err;
	if (src_regno86 != gen86_registers[dst_regno]) {
		gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	}
	gen86_printf("cmp" Plq "\t$%Id, %Id(%s)\n", -val_delta, ind_delta, gen86_regnames[src_regno86]);
	gen86_cmpP_imm_mod(p_pc(self), gen86_modrm_db, -val_delta, ind_delta, src_regno86);
	if (src_regno86 == gen86_registers[dst_regno]) {
		gen86_printf("mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(self), 0, gen86_registers[dst_regno]);
	}
	gen86_printf("set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(self), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (*(src_regno86 + ind_delta) + val_delta) <CMP> rhs_regno ? 1 : 0; */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_gmorph_reg86indCreg2reg01(struct Dee_function_generator *__restrict self,
                               uint8_t src_regno86, ptrdiff_t ind_delta,
                               ptrdiff_t val_delta, unsigned int cmp,
                               Dee_host_register_t rhs_regno,
                               Dee_host_register_t dst_regno) {
	struct Dee_host_section *sect = self->fg_sect;
	ASSERT(fit32(ind_delta));
	ASSERT(fit32(-val_delta));
	if unlikely(Dee_host_section_reqx86(sect, 4))
		goto err;
	if (src_regno86 != gen86_registers[dst_regno] && rhs_regno != dst_regno && val_delta == 0) {
		gen86_printf("xor" Plq "\t%s, %s\n", gen86_regname(dst_regno), gen86_regname(dst_regno));
		gen86_xorP_r_r(p_pc(sect), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	}
	if (val_delta != 0) {
		if (rhs_regno == dst_regno && gen86_registers[dst_regno] == src_regno86) {
			/* Special case: %reg = (*(%reg + ind_delta) <CMP> (%reg - val_delta)) ? 1 : 0
			 * For this case, we actually need a temporary register. */
			Dee_host_register_t not_these[2];
			Dee_host_register_t tempreg;
			bool must_pop_tempreg = false;
			not_these[0] = dst_regno;
			not_these[1] = HOST_REGISTER_COUNT;
			tempreg = Dee_memstate_hregs_find_unused_ex(self->fg_state, not_these);
			if (tempreg >= HOST_REGISTER_COUNT) {
				if unlikely(Dee_host_section_reqx86(sect, 6))
					goto err;
				tempreg = 0;
				if (tempreg == rhs_regno)
					++tempreg;
				must_pop_tempreg = true;
				gen86_printf("push" Plq "\t%s\n", gen86_regname(tempreg));
				gen86_pushP_r(p_pc(sect), gen86_registers[tempreg]);
			}
			gen86_printf("lea" Plq "\t%Id(%s), %s\n", -val_delta, gen86_regname(rhs_regno), gen86_regname(tempreg));
			gen86_leaP_db_r(p_pc(sect), -val_delta, gen86_registers[rhs_regno], gen86_registers[tempreg]);
			gen86_printf("cmp" Plq "\t%s, %Id(%s)\n", gen86_regname(tempreg), ind_delta, gen86_regnames[src_regno86]);
			gen86_cmpP_r_mod(p_pc(sect), gen86_modrm_db, tempreg, ind_delta, src_regno86);
			if (must_pop_tempreg) {
				gen86_printf("pop" Plq "\t%s\n", gen86_regname(tempreg));
				gen86_popP_r(p_pc(sect), gen86_registers[tempreg]);
			}
			goto mov0_to_dst_and_setcc;
		}
		gen86_printf("lea" Plq "\t%Id(%s), %s\n", -val_delta, gen86_regname(rhs_regno), gen86_regname(dst_regno));
		gen86_leaP_db_r(p_pc(sect), -val_delta, gen86_registers[rhs_regno], gen86_registers[dst_regno]);
		rhs_regno = dst_regno;
	}
	gen86_printf("cmp" Plq "\t%s, %Id(%s)\n", gen86_regname(rhs_regno), ind_delta, gen86_regnames[src_regno86]);
	gen86_cmpP_r_mod(p_pc(sect), gen86_modrm_db, rhs_regno, ind_delta, src_regno86);
	if (src_regno86 == gen86_registers[dst_regno] || rhs_regno == dst_regno || val_delta != 0) {
mov0_to_dst_and_setcc:
		gen86_printf("mov" Plq "\t$0, %s\n", gen86_regname(dst_regno));
		gen86_movP_imm_r(p_pc(sect), 0, gen86_registers[dst_regno]);
	}
	gen86_printf("set%s\t%s\n", gen86_ccnames[gmorphbool_cc_86[cmp]], gen86_regname(dst_regno));
	gen86_setcc_r(p_pc(sect), gmorphbool_cc_86[cmp], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

/* dst_regno = (*(src_regno + ind_delta) + val_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmorph_regind2reg01(struct Dee_function_generator *__restrict self,
                                            Dee_host_register_t src_regno, ptrdiff_t ind_delta,
                                            ptrdiff_t val_delta, unsigned int cmp,
                                            Dee_host_register_t dst_regno) {
#ifdef _Dee_function_generator_gmorph_regind2reg01_MAYFAIL
	if (!fit32(-val_delta))
		return 1;
#endif /* _Dee_function_generator_gmorph_regind2reg01_MAYFAIL */
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &src_regno, &ind_delta))
		return -1;
	return impl_gmorph_reg86ind2reg01(self->fg_sect, gen86_registers[src_regno], ind_delta, val_delta, cmp, dst_regno);
}

/* dst_regno = (*(src_regno + ind_delta) + val_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmorph_regindCreg2reg01(struct Dee_function_generator *__restrict self,
                                                Dee_host_register_t src_regno, ptrdiff_t ind_delta,
                                                ptrdiff_t val_delta, unsigned int cmp,
                                                Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno) {
#ifndef fit32_IS_1
	if (!fit32(ind_delta)) {
		ptrdiff_t new_ind_delta = ind_delta;
		if unlikely(_Dee_function_generator_gadjust_reg_fit32(self, &src_regno, &new_ind_delta))
			return -1;
		if (src_regno == rhs_regno) /* Propagate adjustment delta to val_delta */
			val_delta += ind_delta - new_ind_delta;
		ind_delta = new_ind_delta;
	}
	if (!fit32(-val_delta)) {
		val_delta = -val_delta;
		if (src_regno == rhs_regno) {
			/* Must use a temporary register so adjustment doesn't affect "src_regno" */
			Dee_host_register_t new_rhs_regno;
			Dee_host_register_t not_these[2];
			ptrdiff_t adj_delta;
			not_these[0] = rhs_regno;
			not_these[1] = HOST_REGISTER_COUNT;
			new_rhs_regno = Dee_function_generator_gallocreg(self, not_these);
			if unlikely(new_rhs_regno >= HOST_REGISTER_COUNT)
				return -1;
			if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
				return -1;
			adj_delta = val_delta;
			if (adj_delta < INT32_MIN) {
				adj_delta = INT32_MIN;
			} else if (adj_delta > INT32_MAX) {
				adj_delta = INT32_MAX;
			}
			HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(rhs_regno), gen86_regname(new_rhs_regno));
			gen86_leaP_db_r(p_pc(self->fg_sect), val_delta, gen86_registers[rhs_regno], gen86_registers[new_rhs_regno]);
			val_delta += adj_delta;
			rhs_regno = new_rhs_regno;
		}
		if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &rhs_regno, &val_delta))
			return -1;
		val_delta = -val_delta;
	}
#endif /* !fit32_IS_1 */
	return impl_gmorph_reg86indCreg2reg01(self, gen86_registers[src_regno], ind_delta, val_delta, cmp, rhs_regno, dst_regno);
}

/* dst_regno = (*(SP + sp_offset) + val_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmorph_hstackind2reg01(struct Dee_host_section *__restrict self,
                                         ptrdiff_t sp_offset, ptrdiff_t val_delta,
                                         unsigned int cmp, Dee_host_register_t dst_regno) {
#ifdef _Dee_host_section_gmorph_hstackind2reg01_MAYFAIL
	if (!fit32(-val_delta))
		return 1;
#endif /* _Dee_host_section_gmorph_hstackind2reg01_MAYFAIL */
	return impl_gmorph_reg86ind2reg01(self, GEN86_R_PSP, sp_offset, val_delta, cmp, dst_regno);
}

/* dst_regno = (*(SP + sp_offset) + val_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmorph_hstackindCreg2reg01(struct Dee_function_generator *__restrict self,
                                                   ptrdiff_t sp_offset, ptrdiff_t val_delta,
                                                   unsigned int cmp, Dee_host_register_t rhs_regno,
                                                   Dee_host_register_t dst_regno) {
#ifndef fit32_IS_1
	if (!fit32(-val_delta)) {
		val_delta = -val_delta;
		if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &rhs_regno, &val_delta))
			return -1;
		val_delta = -val_delta;
	}
#endif /* !fit32_IS_1 */
	return impl_gmorph_reg86indCreg2reg01(self, GEN86_R_PSP, sp_offset, val_delta, cmp, rhs_regno, dst_regno);
}


#ifdef HAVE__Dee_host_section_gmorph_reg012regbool
/* dst_regno = &Dee_FalseTrue[src_regno + src_delta]; */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gmorph_reg012regbool(struct Dee_host_section *__restrict self,
                                       Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                       Dee_host_register_t dst_regno) {
	STATIC_ASSERT(sizeof(DeeBoolObject) == 8);
	src_delta *= 8;
	src_delta += (uintptr_t)Dee_FalseTrue;
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("lea" Plq "\t%Id(,%s,8), %s\n", src_delta, gen86_regname(src_regno), gen86_regname(dst_regno));
	gen86_leaP_dis_r(p_pc(self), src_delta, gen86_registers[src_regno], 8, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}
#endif /* HAVE__Dee_host_section_gmorph_reg012regbool */


PRIVATE ATTR_PURE WUNUSED Dee_host_register_t DCALL
alloc_unused_reg_for_call_args(struct Dee_memloc const *locv, size_t argc) {
	size_t i;
	Dee_host_register_t result;
	BITSET(HOST_REGISTER_COUNT) inuse_bitset;
	bzero(&inuse_bitset, sizeof(inuse_bitset));
	for (i = 0; i <= argc; ++i) { /* <= because +1 extra api_function arg */
		struct Dee_memloc const *arg = &locv[i];
		if (Dee_memloc_hasreg(arg))
			BITSET_TURNON(&inuse_bitset, Dee_memloc_getreg(arg));
	}
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&inuse_bitset, result))
			break;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Psp0_add_val_offset(struct Dee_function_generator *__restrict self,
                    ptrdiff_t val_offset) {
	struct Dee_memadr adr;
	if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
		goto err;
	gen86_printf("add" Plq "\t$%Id, (%%" Per "sp)\n", val_offset);
	gen86_addP_imm_mod(p_pc(self->fg_sect), gen86_modrm_b, val_offset, GEN86_R_PSP);
	Dee_memadr_init_hstackind(&adr, self->fg_state->ms_host_cfa_offset);
	Dee_function_generator_remember_deltavalue(self, &adr, val_offset);
	return 0;
err:
	return -1;
}

/* Push "loc" onto the HSTACK, and maybe adjust registers */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
push_memloc_maybe_adjust_regs(struct Dee_function_generator *__restrict self,
                              struct Dee_memloc *arg,
                              struct Dee_memloc *locv, size_t argc) {
	/* Look at equivalence classes to pick the best fit for the argument value
	 * #1: HSTACKIND (already at the correct CFA, and with rel_valoff==0)
	 * #2: HREG[rel_valoff==0]/HSTACK[sp_offset==0]
	 * #3: CONST (on x86_64: if it fits)
	 * #4: HREGIND[rel_valoff==0]/HSTACKIND[rel_valoff==0]
	 * #5: HREG
	 * #6: Fallback (use `arg' as-is)
	 */
	struct Dee_memequiv *eq;
	eq = Dee_memequivs_getclassof(&self->fg_state->ms_memequiv, Dee_memloc_getadr(arg));
	if (eq != NULL) {
		struct Dee_memequiv *iter;
		ptrdiff_t base_off;
		base_off = Dee_memloc_getoff(arg);
		base_off -= Dee_memloc_getoff(&eq->meq_loc);
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
		iter = eq;
		do {
			if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACKIND &&
			    Dee_memloc_hstackind_getcfa(&iter->meq_loc) == (self->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER) &&
			    (Dee_memloc_hstackind_getvaloff(&iter->meq_loc) + base_off) == 0)
				goto use_memequiv_iter_as_arg;
		} while ((iter = Dee_memequiv_next(iter)) != eq);
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
		iter = eq;
		do {
			if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG &&
			    (Dee_memloc_hreg_getvaloff(&iter->meq_loc) + base_off) == 0) {
use_memequiv_iter_as_arg:
				*arg = iter->meq_loc;
				Dee_memloc_adjoff(arg, base_off);
				goto use_arg;
			}
		} while ((iter = Dee_memequiv_next(iter)) != eq);
		iter = eq;
		do {
			if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_CONST &&
			    fit32((uintptr_t)Dee_memloc_const_getaddr(&iter->meq_loc) + base_off))
				goto use_memequiv_iter_as_arg;
		} while ((iter = Dee_memequiv_next(iter)) != eq);
		iter = eq;
		do {
			if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREGIND &&
			    (Dee_memloc_hregind_getvaloff(&iter->meq_loc) + base_off) == 0)
				goto use_memequiv_iter_as_arg;
			if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACKIND &&
			    (Dee_memloc_hstackind_getvaloff(&iter->meq_loc) + base_off) == 0)
				goto use_memequiv_iter_as_arg;
		} while ((iter = Dee_memequiv_next(iter)) != eq);
		iter = eq;
		do {
			if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG)
				goto use_memequiv_iter_as_arg;
		} while ((iter = Dee_memequiv_next(iter)) != eq);
	}
use_arg:

	/* NOTE: It is important that this function doesn't call `Dee_function_generator_gallocreg()'!
	 *       That is because `alloc_unused_reg_for_call_args()' needs to be used for temporary
	 *       registers */
	switch (Dee_memloc_gettyp(arg)) {

	case MEMADR_TYPE_HSTACKIND:
	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t ind_offset;
		Dee_host_register_t tempreg;
		uint8_t gen86_src_regno;
		if (Dee_memloc_getoff(arg) == 0) {
			if (Dee_memloc_gettyp(arg)== MEMADR_TYPE_HSTACKIND)
				return Dee_function_generator_ghstack_pushhstackind(self, Dee_memloc_hstackind_getcfa(arg));
			return Dee_function_generator_ghstack_pushregind(self,
			                                                 Dee_memloc_hregind_getreg(arg),
			                                                 Dee_memloc_hregind_getindoff(arg));
		}
		if (Dee_memloc_gettyp(arg)== MEMADR_TYPE_HSTACKIND) {
			uintptr_t cfa_offset = Dee_memloc_hstackind_getcfa(arg);
			ind_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
			gen86_src_regno = GEN86_R_PSP;
		} else {
			ind_offset = arg->ml_adr.ma_val.v_indoff;
			gen86_src_regno = gen86_registers[arg->ml_adr.ma_reg];
		}

		/* Try to use a temporary register. */
		tempreg = alloc_unused_reg_for_call_args(locv, argc);
		if (tempreg < HOST_REGISTER_COUNT) {
			if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
				goto err;
			if unlikely(Dee_function_generator_gmov_loc2reg(self, arg, tempreg))
				goto err;
			if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
				goto err;
			return Dee_function_generator_ghstack_pushreg(self, tempreg);
		}
		if unlikely(Dee_function_generator_ghstack_pushhstackind(self, arg->ml_adr.ma_val.v_cfa))
			goto err;
		return Psp0_add_val_offset(self, Dee_memloc_getoff(arg));
	}	break;

	case MEMADR_TYPE_HSTACK: {
		Dee_host_register_t tempreg;
		Dee_cfa_t cfa_offset = Dee_memloc_hstackind_getcfa(arg);
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		if (sp_offset == 0) {
			/* Special case: can use `pushP %Psp', which pushes the current %Psp value (as it was before the push) */
			if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
				goto err;
			gen86_printf("push" Plq "\t%%" Per "sp\n");
			gen86_pushP_r(p_pc(self->fg_sect), GEN86_R_PSP);
			Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
			return 0;
		}

		/* Check if there is a temp register we can use. */
		tempreg = alloc_unused_reg_for_call_args(locv, argc);
		if (tempreg < HOST_REGISTER_COUNT) {
			if unlikely(Dee_function_generator_gmov_hstack2reg(self, cfa_offset, tempreg))
				goto err;
			return Dee_function_generator_ghstack_pushreg(self, tempreg);
		}

		/* All GP registers need to remain preserved -> load address in 2 steps:
		 * >> pushP %Psp
		 * >> addP  $..., 0(%Psp) */
		if unlikely(Dee_host_section_reqx86(self->fg_sect, 2))
			goto err;
		gen86_printf("push" Plq "\t%%" Per "sp\n");
		gen86_pushP_r(p_pc(self->fg_sect), GEN86_R_PSP);
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		gen86_printf("add" Plq "\t$%Id, (%%" Per "sp)\n");
		gen86_addP_imm_mod(p_pc(self->fg_sect), gen86_modrm_b, sp_offset, GEN86_R_PSP);
	}	break;

	case MEMADR_TYPE_HREG:
		if (Dee_memloc_getoff(arg) != 0) {
			size_t i;
			ptrdiff_t val_delta = Dee_memloc_getoff(arg);
			Dee_host_register_t regno = Dee_memloc_hreg_getreg(arg);
			if unlikely(Dee_function_generator_gadjust_reg_delta(self->fg_sect, self, regno, val_delta, true))
				goto err;
			for (i = 0; i <= argc; ++i) {
				struct Dee_memloc *nextarg = &locv[i];
				switch (Dee_memloc_gettyp(nextarg)) {
				case MEMADR_TYPE_HREG:
					if (Dee_memloc_hreg_getreg(nextarg) == regno)
						nextarg->ml_off -= val_delta;
					break;
				case MEMADR_TYPE_HREGIND:
					if (Dee_memloc_hregind_getreg(nextarg) == regno)
						nextarg->ml_adr.ma_val.v_indoff -= val_delta;
					break;
				default: break;
				}
			}
		}
		return Dee_function_generator_ghstack_pushreg(self, Dee_memloc_hreg_getreg(arg));

	case MEMADR_TYPE_CONST:
		return Dee_function_generator_ghstack_pushconst(self, Dee_memloc_const_getaddr(arg));

	case MEMADR_TYPE_UNDEFINED:
		return Dee_function_generator_ghstack_adjust(self, HOST_SIZEOF_POINTER);

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot push memory location with type %#" PRFx16,
		                       Dee_memloc_gettyp(arg));
		break;
	}
	return 0;
err:
	return -1;
}

#ifdef HOST_REGISTER_R_ARG0
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
setreg_memloc_maybe_adjust_regs(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *arg, Dee_host_register_t dst_regno,
                                struct Dee_memloc *locv, size_t argc,
                                size_t already_pushed_arg_regs) {
	STATIC_ASSERT_MSG(COMPILER_LENOF(host_arg_regs) + 2 <= HOST_REGISTER_COUNT,
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
		struct Dee_memloc *nextarg = &locv[i];
		Dee_host_register_t free_regno;
		ptrdiff_t free_regno_val_offset_from_dst_regno;
		bool free_regno_initialized;
		uint16_t register_use_count[HOST_REGISTER_COUNT];
		if (!Dee_memloc_hasreg(nextarg))
			continue;
		if (Dee_memloc_getreg(nextarg) != dst_regno)
			continue;
		bzero(register_use_count, sizeof(register_use_count));
		for (i = 0; i < already_pushed_arg_regs; ++i) /* Already loaded arguments must not be clobbered */
			register_use_count[host_arg_regs[argc + i]] = (uint16_t)-1;
		for (i = 0; i <= argc; ++i) {
			if (Dee_memloc_hasreg(nextarg)) {
				Dee_host_register_t regno = Dee_memloc_getreg(nextarg);
				++register_use_count[regno];
			}
		}
		for (free_regno = 0;; ++free_regno) {
			ASSERTF(free_regno < HOST_REGISTER_COUNT, "At least 1 register should have been available!");
			if (register_use_count[free_regno] == 0)
				break;
		}
		free_regno_initialized = false;
		free_regno_val_offset_from_dst_regno = 0; /* %free_regno = %dst_regno + free_regno_val_offset_from_dst_regno */
		for (i = 0; i <= argc; ++i) {
			Dee_host_register_t used_regno;
			ptrdiff_t nextarg_valoff;
			nextarg = &locv[i];
			if (!Dee_memloc_hasreg(nextarg))
				continue;
			if (Dee_memloc_getreg(nextarg) != dst_regno)
				continue;
			used_regno = i == 0 ? HOST_REGISTER_PAX : host_arg_regs[i - 1]; /* Try to use PAX for the function pointer */
			if (register_use_count[used_regno] != 0)
				used_regno = free_regno;
			switch (Dee_memloc_gettyp(nextarg)) {
			case MEMADR_TYPE_HREG:
				nextarg_valoff = Dee_memloc_hreg_getvaloff(nextarg);
				break;
			case MEMADR_TYPE_HREGIND:
				nextarg_valoff = Dee_memloc_hregind_getindoff(nextarg);
				break;
			default: __builtin_unreachable();
			}
			if (used_regno == free_regno && free_regno_initialized) {
				nextarg_valoff -= free_regno_val_offset_from_dst_regno;
			} else {
				/* TODO: If "nextarg" is `MEMADR_TYPE_HREGIND' and the register isn't used
				 *       by any other argument, directly load the value into the register. */
				if unlikely(Dee_function_generator_gmov_regx2reg(self, dst_regno, nextarg_valoff, used_regno))
					goto err;
				if (used_regno == free_regno) {
					free_regno_val_offset_from_dst_regno = nextarg_valoff;
					free_regno_initialized = true;
				}
				nextarg_valoff = 0;
			}
			switch (Dee_memloc_gettyp(nextarg)) {
			case MEMADR_TYPE_HREG:
				Dee_memloc_init_hreg(nextarg, used_regno, nextarg_valoff);
				break;
			case MEMADR_TYPE_HREGIND:
				Dee_memloc_init_hregind(nextarg, used_regno, nextarg_valoff,
				                        Dee_memloc_hregind_getvaloff(nextarg));
				break;
			default: __builtin_unreachable();
			}
		}
		break;
	}

	/* At this point, we've successfully made it so that "dst_regno" isn't used by future arguments. */

	/* NOTE: It is important that this function doesn't call `Dee_function_generator_gallocreg()'!
	 *       That is because `alloc_unused_reg_for_call_args()' needs to be used for temporary
	 *       registers */
	switch (Dee_memloc_gettyp(arg)) {

	case MEMADR_TYPE_HSTACKIND:
		if unlikely(Dee_function_generator_gmov_hstackind2reg(self, Dee_memloc_hstackind_getcfa(arg), dst_regno))
			goto err;
		return Dee_function_generator_gmov_regx2reg(self, dst_regno, Dee_memloc_hregind_getvaloff(arg), dst_regno);

	case MEMADR_TYPE_HREGIND:
		if unlikely(Dee_function_generator_gmov_regind2reg(self,
		                                                   Dee_memloc_hregind_getreg(arg),
		                                                   Dee_memloc_hregind_getindoff(arg),
		                                                   dst_regno))
			goto err;
		return Dee_function_generator_gmov_regx2reg(self, dst_regno, Dee_memloc_hregind_getvaloff(arg), dst_regno);

	case MEMADR_TYPE_HSTACK:
		return Dee_function_generator_gmov_hstack2reg(self, Dee_memloc_hstack_getcfa(arg), dst_regno);

	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gmov_regx2reg(self,
		                                            Dee_memloc_hreg_getreg(arg),
		                                            Dee_memloc_hreg_getvaloff(arg),
		                                            dst_regno);

	case MEMADR_TYPE_CONST:
		return Dee_function_generator_gmov_const2reg(self, Dee_memloc_const_getaddr(arg), dst_regno);

	case MEMADR_TYPE_UNDEFINED:
		break;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move-to-reg memory location with type %#" PRFx16,
		                       Dee_memloc_gettyp(arg));
		break;
	}
	return 0;
err:
	return -1;
}
#endif /* HOST_REGISTER_R_ARG0 */

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gcallapi(struct Dee_function_generator *__restrict self,
                                 struct Dee_memloc *locv, size_t argc) {
	struct Dee_host_section *sect = self->fg_sect;
	uintptr_t hstackaddr_regs[HOST_REGISTER_COUNT];
	size_t argi;
	bzero(hstackaddr_regs, sizeof(hstackaddr_regs));

	/* Push arguments onto the host stack in reverse order. */
	argi = argc;
#ifdef HOST_REGISTER_R_ARG0
	while (argi > COMPILER_LENOF(host_arg_regs))
#else /* HOST_REGISTER_R_ARG0 */
	while (argi)
#endif /* !HOST_REGISTER_R_ARG0 */
	{
		struct Dee_memloc *arg;
		--argi;
		arg = &locv[argi + 1]; /* +1 because of the function being called. */

		/* All remaining arguments must be pushed via the stack. */
		if unlikely(push_memloc_maybe_adjust_regs(self, arg, locv, argi))
			goto err;
	}

#ifdef HOST_REGISTER_R_ARG0
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
			struct Dee_memloc *arg = &locv[i];
			struct Dee_memequiv *eq = Dee_memequivs_getclassof(&self->fg_state->ms_memequiv, Dee_memloc_getadr(arg));
			if (eq != NULL) {
				Dee_host_register_t best_fit = i == 0 ? HOST_REGISTER_PAX : host_arg_regs[i - 1];
				struct Dee_memequiv *iter;
				ptrdiff_t base_off;
				base_off = Dee_memloc_getoff(arg);
				base_off -= Dee_memloc_getoff(&eq->meq_loc);
				iter = eq;
				do {
					if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG &&
					    (Dee_memloc_hreg_getreg(&iter->meq_loc) == best_fit || best_fit == HOST_REGISTER_PAX) &&
					    (Dee_memloc_hreg_getvaloff(&iter->meq_loc) + base_off) == 0) {
use_memequiv_iter_as_arg:
						*arg = iter->meq_loc;
						Dee_memloc_adjoff(arg, base_off);
						goto use_arg;
					}
				} while ((iter = Dee_memequiv_next(iter)) != eq);
				iter = eq;
				do {
					if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREG ||
					    Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACK)
						goto use_memequiv_iter_as_arg;
				} while ((iter = Dee_memequiv_next(iter)) != eq);
				iter = eq;
				do {
					if (Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_CONST)
						goto use_memequiv_iter_as_arg;
				} while ((iter = Dee_memequiv_next(iter)) != eq);
				iter = eq;
				do {
					if ((Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HREGIND ||
					     Dee_memloc_gettyp(&iter->meq_loc) == MEMADR_TYPE_HSTACKIND) &&
					    (Dee_memloc_getoff(&iter->meq_loc) + base_off) == 0)
						goto use_memequiv_iter_as_arg;
				} while ((iter = Dee_memequiv_next(iter)) != eq);
			}
		}
use_arg:;
	}

	/* Fix-up argument registers. */
	while (argi) {
		Dee_host_register_t arg_regno;
		struct Dee_memloc *arg;
		--argi;
		arg = &locv[argi + 1]; /* +1 because of the function being called. */
		arg_regno = host_arg_regs[argi];
		if unlikely(setreg_memloc_maybe_adjust_regs(self, arg, arg_regno, locv, argi,
		                                            (COMPILER_LENOF(host_arg_regs) - 1) - argi))
			goto err;
	}
#endif /* HOST_REGISTER_R_ARG0 */



	/* With everything pushed onto the stack and in the correct position, generate the call.
	 * Note that because deemon API functions use DCALL (STDCALL), the called function does
	 * the stack cleanup. */
#ifdef HOSTASM_X86_64_MSABI
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	gen86_printf("sub" Plq "\t$32, %%" Per "sp\n");
	gen86_subP_imm_r(p_pc(sect), 32, GEN86_R_PSP); /* TODO: Do this better */
	Dee_function_generator_gadjust_cfa_offset(self, 32);
#endif /* HOSTASM_X86_64_MSABI */
	switch (__builtin_expect(Dee_memloc_gettyp(locv), MEMADR_TYPE_CONST)) {

	case MEMADR_TYPE_CONST: {
		void const *api_function = Dee_memloc_const_getaddr(locv);
		if unlikely(gcall86(self, api_function, true))
			goto err;
	}	break;

	default:
invoke_api_function_fallback:
		if unlikely(Dee_function_generator_gasreg(self, locv, locv, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG: {
		Dee_host_register_t regno = Dee_memloc_hreg_getreg(locv);
		ptrdiff_t val_offset = Dee_memloc_hreg_getvaloff(locv);
		if unlikely(val_offset != 0) {
			if unlikely(Dee_function_generator_gadjust_reg_delta_impl(self->fg_sect, NULL, regno, val_offset, true))
				goto err;
			if unlikely(Dee_host_section_reqx86(sect, 1))
				goto err;
		}
		gen86_printf("calll\t*%s\n", gen86_regname(regno));
		gen86_callP_mod(p_pc(sect), gen86_modrm_r, gen86_registers[regno]);
	}	break;

	case MEMADR_TYPE_HSTACKIND: {
		Dee_cfa_t cfa_offset = Dee_memloc_hstackind_getcfa(locv);
		ptrdiff_t val_offset = Dee_memloc_hstackind_getvaloff(locv);
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		if unlikely(val_offset != 0)
			goto invoke_api_function_fallback;
		gen86_printf("calll\t%Id(%%" Per "sp)\n", sp_offset);
		gen86_callP_mod(p_pc(sect), gen86_modrm_db, sp_offset, GEN86_R_PSP);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		Dee_host_register_t regno = Dee_memloc_hregind_getreg(locv);
		ptrdiff_t ind_offset = Dee_memloc_hregind_getindoff(locv);
		ptrdiff_t val_offset = Dee_memloc_hregind_getvaloff(locv);
		if unlikely(val_offset != 0)
			goto invoke_api_function_fallback;
		gen86_printf("calll\t%Id(%s)\n", ind_offset, gen86_regname(regno));
		gen86_callP_mod(p_pc(sect), gen86_modrm_db, ind_offset, gen86_registers[regno]);
	}	break;

	}

	/* Adjust the CFA offset to account for the called function having cleaned up stack arguments. */
	{
		size_t stack_dealloc;
#ifdef HOST_REGISTER_R_ARG0
		if (argc <= COMPILER_LENOF(host_arg_regs)) {
			stack_dealloc = 0;
		} else
#endif /* HOST_REGISTER_R_ARG0 */
		{
			stack_dealloc = argc;
#ifdef HOST_REGISTER_R_ARG0
			stack_dealloc -= COMPILER_LENOF(host_arg_regs);
#endif /* HOST_REGISTER_R_ARG0 */
		}
		if (stack_dealloc) {
			stack_dealloc *= HOST_SIZEOF_POINTER;
			Dee_function_generator_gadjust_cfa_offset(self, -(ptrdiff_t)stack_dealloc);
			/* TODO: Forget assumptions about deallocated stack locations */
		}
	}

#ifdef HOSTASM_X86_64_MSABI
	if unlikely(Dee_host_section_reqx86(sect, 1))
		goto err;
	gen86_printf("add" Plq "\t$32, %%" Per "sp\n");
	gen86_addP_imm_r(p_pc(sect), 32, GEN86_R_PSP); /* TODO: Do this better */
	Dee_function_generator_gadjust_cfa_offset(self, -32);
#endif /* !HOSTASM_X86_64_MSABI */

	return 0;
err:
	return -1;
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
	rel->hr_offset = (uint32_t)(p_off(self) - 4);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
	Dee_host_reloc_setsym(rel, dst);
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
	rel->hr_offset = (uint32_t)(p_off(self) - 4);
	rel->hr_rtype  = DEE_HOST_RELOC_PCREL32;
	Dee_host_reloc_setsym(rel, dst);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
_Dee_host_section_gjcc_reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno,
                           struct Dee_host_symbol *__restrict dst, uint8_t cc) {
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("test" Plq "\t%s, %s\n", gen86_regname(src_regno), gen86_regname(src_regno));
	gen86_testP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[src_regno]);
	return _Dee_host_section_gjcc(self, dst, cc);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
_Dee_function_generator_gjcc_regind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno,
                                    ptrdiff_t ind_delta, struct Dee_host_symbol *__restrict dst, uint8_t cc) {
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &src_regno, &ind_delta))
		goto err;
	if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
		goto err;
	gen86_printf("cmp" Plq "\t$0, %Id(%s)\n", ind_delta, gen86_regname(src_regno));
	gen86_cmpP_imm_mod(p_pc(self->fg_sect), gen86_modrm_db, 0, ind_delta, gen86_registers[src_regno]);
	return _Dee_host_section_gjcc(self->fg_sect, dst, cc);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
_Dee_host_section_gjcc_hstackind(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset,
                                 struct Dee_host_symbol *__restrict dst, uint8_t cc) {
	ASSERTF(fit32(sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("cmp" Plq "\t$0, %Id(%%" Per "sp)\n", sp_offset);
	gen86_cmpP_imm_mod(p_pc(self), gen86_modrm_db, 0, sp_offset, GEN86_R_PSP);
	return _Dee_host_section_gjcc(self, dst, cc);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 3)) int DCALL
_Dee_host_section_gjz_reg(struct Dee_host_section *__restrict self,
                          Dee_host_register_t src_regno, struct Dee_host_symbol *__restrict dst) {
	return _Dee_host_section_gjcc_reg(self, src_regno, dst, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
_Dee_function_generator_gjz_regind(struct Dee_function_generator *__restrict self,
                                   Dee_host_register_t src_regno, ptrdiff_t ind_delta,
                                   struct Dee_host_symbol *__restrict dst) {
	return _Dee_function_generator_gjcc_regind(self, src_regno, ind_delta, dst, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
_Dee_host_section_gjz_hstackind(struct Dee_host_section *__restrict self,
                                ptrdiff_t sp_offset, struct Dee_host_symbol *__restrict dst) {
	return _Dee_host_section_gjcc_hstackind(self, sp_offset, dst, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
_Dee_host_section_gjnz_reg(struct Dee_host_section *__restrict self,
                           Dee_host_register_t src_regno, struct Dee_host_symbol *__restrict dst) {
	return _Dee_host_section_gjcc_reg(self, src_regno, dst, GEN86_CC_NZ);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
_Dee_function_generator_gjnz_regind(struct Dee_function_generator *__restrict self,
                                    Dee_host_register_t src_regno, ptrdiff_t ind_delta,
                                    struct Dee_host_symbol *__restrict dst) {
	return _Dee_function_generator_gjcc_regind(self, src_regno, ind_delta, dst, GEN86_CC_NZ);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
_Dee_host_section_gjnz_hstackind(struct Dee_host_section *__restrict self,
                                 ptrdiff_t sp_offset, struct Dee_host_symbol *__restrict dst) {
	return _Dee_host_section_gjcc_hstackind(self, sp_offset, dst, GEN86_CC_NZ);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gjcc3(struct Dee_host_section *__restrict self, bool signed_cmp,
                        struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                        struct Dee_host_symbol *dst_gr) {
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
		if unlikely(_Dee_host_section_gjcc(self, dst_lo, cc_lo))
			goto err;
		if (cc_lo == GEN86_CC_NE)
			return dst_eq ? _Dee_host_section_gjmp(self, dst_eq) : 0;
	}
	if (dst_gr) {
		uint8_t cc_gr = signed_cmp ? GEN86_CC_G : GEN86_CC_A;
		if (dst_eq == dst_gr) {
			dst_eq = NULL;
			cc_gr  = signed_cmp ? GEN86_CC_GE : GEN86_CC_AE;
		}
		if unlikely(_Dee_host_section_gjcc(self, dst_gr, cc_gr))
			goto err;
	}
	if (dst_eq) {
		if (dst_lo && dst_gr)
			return _Dee_host_section_gjmp(self, dst_gr);
		return _Dee_host_section_gjcc(self, dst_eq, GEN86_CC_E);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gjcc_regCreg(struct Dee_host_section *__restrict self,
                               Dee_host_register_t lhs_regno, Dee_host_register_t rhs_regno, bool signed_cmp,
                               struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                               struct Dee_host_symbol *dst_gr) {
	if unlikely(lhs_regno == rhs_regno) /* Same register -> compare is always equal */
		return dst_eq ? _Dee_host_section_gjmp(self, dst_eq) : 0;
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("cmp" Plq "\t%s, %s\n", gen86_regname(rhs_regno), gen86_regname(lhs_regno));
	gen86_cmpP_r_r(p_pc(self), gen86_registers[rhs_regno], gen86_registers[lhs_regno]);
	return _Dee_host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gjcc_regCconst(struct Dee_host_section *__restrict self,
                                 Dee_host_register_t lhs_regno, void const *rhs_value, bool signed_cmp,
                                 struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                 struct Dee_host_symbol *dst_gr) {
	if unlikely(rhs_value == (void const *)0 && (dst_lo == dst_gr)) {
		/* >> if (%Pax == 0) { ... }
		 * Same as:
		 * >> if (!%Pax) { ... } */
		if (dst_lo && dst_eq) {
			if unlikely(_Dee_host_section_gjz_reg(self, lhs_regno, dst_eq))
				goto err;
			return _Dee_host_section_gjmp(self, dst_lo);
		} else if (dst_eq) {
			return _Dee_host_section_gjz_reg(self, lhs_regno, dst_eq);
		} else if (dst_lo) {
			return _Dee_host_section_gjnz_reg(self, lhs_regno, dst_lo);
		}
		return 0;
	}
#ifdef _Dee_host_section_gjcc_regCconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _Dee_host_section_gjcc_regCconst_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("cmp" Plq "\t$%#Ix, %s\n", rhs_value, gen86_regname(lhs_regno));
	gen86_cmpP_imm_r(p_pc(self), (int32_t)(intptr_t)(uintptr_t)rhs_value, gen86_registers[lhs_regno]);
	return _Dee_host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gjcc_regindCreg(struct Dee_function_generator *__restrict self,
                                        Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta,
                                        Dee_host_register_t rhs_regno, bool signed_cmp,
                                        struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                        struct Dee_host_symbol *dst_gr) {
#ifndef fit32_IS_1
	if (!fit32(lhs_ind_delta)) {
		if (lhs_regno == rhs_regno) {
			Dee_host_register_t not_these[2];
			ptrdiff_t adj_delta;
			not_these[0] = rhs_regno;
			not_these[1] = HOST_REGISTER_COUNT;
			lhs_regno = Dee_function_generator_gallocreg(self, NULL);
			if unlikely(lhs_regno >= HOST_REGISTER_COUNT)
				goto err;
			adj_delta = lhs_ind_delta;
			if (adj_delta < INT32_MIN) {
				adj_delta = INT32_MIN;
			} else if (adj_delta > INT32_MAX) {
				adj_delta = INT32_MAX;
			}
			if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
				goto err;
			HA_printf("lea" Plq "\t%Id(%s), %s\n", adj_delta, gen86_regname(rhs_regno), gen86_regname(lhs_regno));
			gen86_leaP_db_r(p_pc(self->fg_sect), adj_delta, gen86_registers[rhs_regno], gen86_registers[lhs_regno]);
			lhs_ind_delta -= adj_delta;
		}
		if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &lhs_regno, &lhs_ind_delta))
			goto err;
	}
#endif /* !fit32_IS_1 */
	if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
		goto err;
	gen86_printf("cmp" Plq "\t%s, %Id(%s)\n", gen86_regname(rhs_regno), lhs_ind_delta, gen86_regname(lhs_regno));
	gen86_cmpP_r_mod(p_pc(self->fg_sect), gen86_modrm_db, gen86_registers[rhs_regno],
	                 (int32_t)lhs_ind_delta, gen86_registers[lhs_regno]);
	return _Dee_host_section_gjcc3(self->fg_sect, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gjcc_regindCconst(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta,
                                          void const *rhs_value, bool signed_cmp,
                                          struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                          struct Dee_host_symbol *dst_gr) {
#ifdef _Dee_host_section_gjcc_regCconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _Dee_host_section_gjcc_regCconst_MAYFAIL */
	if unlikely(Dee_function_generator_gadjust_reg_fit32(self, &lhs_regno, &lhs_ind_delta))
		goto err;
	if unlikely(Dee_host_section_reqx86(self->fg_sect, 1))
		goto err;
	gen86_printf("cmp" Plq "\t$%#Ix, %Id(%s)\n", rhs_value, lhs_ind_delta, gen86_regname(lhs_regno));
	gen86_cmpP_imm_mod(p_pc(self->fg_sect), gen86_modrm_db,
	                   (int32_t)(intptr_t)(uintptr_t)rhs_value,
	                   lhs_ind_delta, gen86_registers[lhs_regno]);
	return _Dee_host_section_gjcc3(self->fg_sect, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gjcc_hstackindCreg(struct Dee_host_section *__restrict self,
                                     ptrdiff_t lhs_sp_offset, Dee_host_register_t rhs_regno, bool signed_cmp,
                                     struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                     struct Dee_host_symbol *dst_gr) {
	ASSERTF(fit32(lhs_sp_offset),
	        "What are you doing? An SP delta that large *can't* be "
	        "correct. The resulting code would just SEGFAULT!");
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("cmp" Plq "\t%s, %Id(%%" Per "sp)\n", gen86_regname(rhs_regno), lhs_sp_offset);
	gen86_cmpP_r_mod(p_pc(self), gen86_modrm_db, gen86_registers[rhs_regno],
	                 (int32_t)lhs_sp_offset, GEN86_R_PSP);
	return _Dee_host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_gjcc_hstackindCconst(struct Dee_host_section *__restrict self,
                                       ptrdiff_t lhs_sp_offset, void const *rhs_value, bool signed_cmp,
                                       struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                       struct Dee_host_symbol *dst_gr) {
#ifdef _Dee_host_section_gjcc_hstackindCconst_MAYFAIL
	if (!fit32(rhs_value))
		return 1;
#endif /* _Dee_host_section_gjcc_hstackindCconst_MAYFAIL */
	if unlikely(Dee_host_section_reqx86(self, 1))
		goto err;
	gen86_printf("cmp" Plq "\t$%#Ix, %Id(%%" Per "sp)\n", rhs_value, lhs_sp_offset);
	gen86_cmpP_imm_mod(p_pc(self), gen86_modrm_db, (int32_t)(intptr_t)(uintptr_t)rhs_value,
	                   (int32_t)lhs_sp_offset, GEN86_R_PSP);
	return _Dee_host_section_gjcc3(self, signed_cmp, dst_lo, dst_eq, dst_gr);
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_ARCH_C */
