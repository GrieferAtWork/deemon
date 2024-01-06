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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_CCALL_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_CCALL_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/cell.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/weakref.h>

DECL_BEGIN

/************************************************************************/
/* TYPE METHOD/GETSET SPECIFIC OPTIMIZATIONS                            */
/************************************************************************/

#define CCA_OPTIMIZATION(attr, func, argc)   { { attr }, func, argc }
#define CCO_OPTIMIZATION(opname, func, argc) { { (char const *)(uintptr_t)(opname) }, func, argc }

#define OPERATOR_0000_CONSTRUCTOR  OPERATOR_CONSTRUCTOR
#define OPERATOR_0001_COPY         OPERATOR_COPY
#define OPERATOR_0002_DEEPCOPY     OPERATOR_DEEPCOPY
#define OPERATOR_0003_DESTRUCTOR   OPERATOR_DESTRUCTOR
#define OPERATOR_0004_ASSIGN       OPERATOR_ASSIGN
#define OPERATOR_0005_MOVEASSIGN   OPERATOR_MOVEASSIGN
#define OPERATOR_0006_STR          OPERATOR_STR
#define OPERATOR_0007_REPR         OPERATOR_REPR
#define OPERATOR_0008_BOOL         OPERATOR_BOOL
#define OPERATOR_0009_ITERNEXT     OPERATOR_ITERNEXT
#define OPERATOR_000A_CALL         OPERATOR_CALL
#define OPERATOR_000B_INT          OPERATOR_INT
#define OPERATOR_000C_FLOAT        OPERATOR_FLOAT
#define OPERATOR_000D_INV          OPERATOR_INV
#define OPERATOR_000E_POS          OPERATOR_POS
#define OPERATOR_000F_NEG          OPERATOR_NEG
#define OPERATOR_0010_ADD          OPERATOR_ADD
#define OPERATOR_0011_SUB          OPERATOR_SUB
#define OPERATOR_0012_MUL          OPERATOR_MUL
#define OPERATOR_0013_DIV          OPERATOR_DIV
#define OPERATOR_0014_MOD          OPERATOR_MOD
#define OPERATOR_0015_SHL          OPERATOR_SHL
#define OPERATOR_0016_SHR          OPERATOR_SHR
#define OPERATOR_0017_AND          OPERATOR_AND
#define OPERATOR_0018_OR           OPERATOR_OR
#define OPERATOR_0019_XOR          OPERATOR_XOR
#define OPERATOR_001A_POW          OPERATOR_POW
#define OPERATOR_001B_INC          OPERATOR_INC
#define OPERATOR_001C_DEC          OPERATOR_DEC
#define OPERATOR_001D_INPLACE_ADD  OPERATOR_INPLACE_ADD
#define OPERATOR_001E_INPLACE_SUB  OPERATOR_INPLACE_SUB
#define OPERATOR_001F_INPLACE_MUL  OPERATOR_INPLACE_MUL
#define OPERATOR_0020_INPLACE_DIV  OPERATOR_INPLACE_DIV
#define OPERATOR_0021_INPLACE_MOD  OPERATOR_INPLACE_MOD
#define OPERATOR_0022_INPLACE_SHL  OPERATOR_INPLACE_SHL
#define OPERATOR_0023_INPLACE_SHR  OPERATOR_INPLACE_SHR
#define OPERATOR_0024_INPLACE_AND  OPERATOR_INPLACE_AND
#define OPERATOR_0025_INPLACE_OR   OPERATOR_INPLACE_OR
#define OPERATOR_0026_INPLACE_XOR  OPERATOR_INPLACE_XOR
#define OPERATOR_0027_INPLACE_POW  OPERATOR_INPLACE_POW
#define OPERATOR_0028_HASH         OPERATOR_HASH
#define OPERATOR_0029_EQ           OPERATOR_EQ
#define OPERATOR_002A_NE           OPERATOR_NE
#define OPERATOR_002B_LO           OPERATOR_LO
#define OPERATOR_002C_LE           OPERATOR_LE
#define OPERATOR_002D_GR           OPERATOR_GR
#define OPERATOR_002E_GE           OPERATOR_GE
#define OPERATOR_002F_ITERSELF     OPERATOR_ITERSELF
#define OPERATOR_0030_SIZE         OPERATOR_SIZE
#define OPERATOR_0031_CONTAINS     OPERATOR_CONTAINS
#define OPERATOR_0032_GETITEM      OPERATOR_GETITEM
#define OPERATOR_0033_DELITEM      OPERATOR_DELITEM
#define OPERATOR_0034_SETITEM      OPERATOR_SETITEM
#define OPERATOR_0035_GETRANGE     OPERATOR_GETRANGE
#define OPERATOR_0036_DELRANGE     OPERATOR_DELRANGE
#define OPERATOR_0037_SETRANGE     OPERATOR_SETRANGE
#define OPERATOR_0038_GETATTR      OPERATOR_GETATTR
#define OPERATOR_0039_DELATTR      OPERATOR_DELATTR
#define OPERATOR_003A_SETATTR      OPERATOR_SETATTR
#define OPERATOR_003B_ENUMATTR     OPERATOR_ENUMATTR
#define OPERATOR_003C_ENTER        OPERATOR_ENTER
#define OPERATOR_003D_LEAVE        OPERATOR_LEAVE


#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err

#define DEFINE_UNCHECKED_OPTIMIZATION(name, impl)                                 \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                                        \
	name(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) { \
		(void)argc;                                                               \
		return impl;                                                              \
	}
#define DEFINE_CHECKED_OPTIMIZATION(name, impl)                                   \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                                        \
	name(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) { \
		int result;                                                               \
		(void)argc;                                                               \
		result = impl;                                                            \
		if likely(result == 0)                                                    \
			Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;   \
		return result;                                                            \
	}
#define DEFINE_CHECKED_OPERATOR(name, opname, argc) \
	DEFINE_CHECKED_OPTIMIZATION(name, Dee_function_generator_vop(self, opname, argc, VOP_F_PUSHRES))


/* value -> bool
 * Implement a fast "operator bool" by checking if `*(void **)((byte_t *)value + offsetof_field) != NULL' */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vbool_field_nonzero(struct Dee_function_generator *__restrict self,
                    ptrdiff_t offsetof_field) {
	struct Dee_memloc *vtop;
	DO(Dee_function_generator_vdup(self));                 /* value, value */
	DO(Dee_function_generator_vind(self, offsetof_field)); /* value, value->xx_field */
	DO(Dee_function_generator_vreg(self, NULL));           /* value, reg:value->xx_field */
	DO(Dee_function_generator_vdirect(self, 1));           /* value, reg:value->xx_field */
	DO(Dee_function_generator_vswap(self));                /* reg:value->xx_field, value */
	DO(Dee_function_generator_vpop(self));                 /* reg:value->xx_field */
	vtop = Dee_function_generator_vtop(self);
	ASSERT(MEMLOC_VMORPH_ISDIRECT(vtop->ml_vmorph));
	vtop->ml_vmorph = MEMLOC_VMORPH_BOOL_NZ;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
vsize_field_uint(struct Dee_function_generator *__restrict self,
                 ptrdiff_t offsetof_size_field) {
	struct Dee_memloc *loc;
	DO(Dee_function_generator_vdup(self));                      /* value, value */
	DO(Dee_function_generator_vind(self, offsetof_size_field)); /* value, value->xx_size */
	DO(Dee_function_generator_vreg(self, NULL));                /* value, reg:value->xx_size */
	DO(Dee_function_generator_vdirect(self, 1));                /* value, reg:value->xx_size */
	DO(Dee_function_generator_vswap(self));                     /* reg:value->xx_size, value */
	DO(Dee_function_generator_vpop(self));                      /* reg:value->xx_size */
	loc = Dee_function_generator_vtop(self);
	ASSERT(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph));
	loc->ml_vmorph = MEMLOC_VMORPH_UINT;
	return 0;
err:
	return -1;
}




/************************************************************************/
/* Object                                                               */
/************************************************************************/
DEFINE_CHECKED_OPERATOR(cca_Object___copy__, OPERATOR_COPY, 1)
DEFINE_CHECKED_OPERATOR(cca_Object___deepcopy__, OPERATOR_DEEPCOPY, 1)
DEFINE_CHECKED_OPERATOR(cca_Object___assign__, OPERATOR_ASSIGN, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___moveassign__, OPERATOR_MOVEASSIGN, 2)
DEFINE_CHECKED_OPTIMIZATION(cca_Object___str__, Dee_function_generator_vopstr(self)) /* 1 */
DEFINE_CHECKED_OPERATOR(cca_Object___repr__, OPERATOR_REPR, 1)
DEFINE_CHECKED_OPTIMIZATION(cca_Object___bool__, Dee_function_generator_vopbool(self, VOPBOOL_F_NORMAL)) /* 1 */
DEFINE_UNCHECKED_OPTIMIZATION(cca_Object___call__,
                              Dee_function_generator_vassert_type_exact_c(self, &DeeTuple_Type) ||
                              Dee_function_generator_vopcalltuple_unchecked(self)) /* 2 */
DEFINE_UNCHECKED_OPTIMIZATION(cca_Object___thiscall__,
                              Dee_function_generator_vassert_type_exact_c(self, &DeeTuple_Type) ||
                              Dee_function_generator_vopthiscalltuple_unchecked(self)) /* 3 */
DEFINE_CHECKED_OPERATOR(cca_Object___hash__, OPERATOR_HASH, 1)
DEFINE_CHECKED_OPTIMIZATION(cca_Object___int__, Dee_function_generator_vopint(self)) /* 1 */
DEFINE_CHECKED_OPERATOR(cca_Object___inv__, OPERATOR_INV, 1)
DEFINE_CHECKED_OPERATOR(cca_Object___pos__, OPERATOR_POS, 1)
DEFINE_CHECKED_OPERATOR(cca_Object___neg__, OPERATOR_NEG, 1)
DEFINE_CHECKED_OPERATOR(cca_Object___add__, OPERATOR_ADD, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___sub__, OPERATOR_SUB, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___mul__, OPERATOR_MUL, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___div__, OPERATOR_DIV, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___mod__, OPERATOR_MOD, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___shl__, OPERATOR_SHL, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___shr__, OPERATOR_SHR, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___and__, OPERATOR_AND, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___or__, OPERATOR_OR, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___xor__, OPERATOR_XOR, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___pow__, OPERATOR_POW, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___eq__, OPERATOR_EQ, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___ne__, OPERATOR_NE, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___lo__, OPERATOR_LO, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___le__, OPERATOR_LE, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___gr__, OPERATOR_GR, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___ge__, OPERATOR_GE, 2)
DEFINE_CHECKED_OPTIMIZATION(cca_Object___size__, Dee_function_generator_vopsize(self)) /* 1 */
DEFINE_CHECKED_OPERATOR(cca_Object___contains__, OPERATOR_CONTAINS, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___getitem__, OPERATOR_GETITEM, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___delitem__, OPERATOR_DELITEM, 2)
DEFINE_CHECKED_OPERATOR(cca_Object___setitem__, OPERATOR_SETITEM, 3)
DEFINE_CHECKED_OPERATOR(cca_Object___getrange__, OPERATOR_GETRANGE, 3)
DEFINE_CHECKED_OPERATOR(cca_Object___delrange__, OPERATOR_DELRANGE, 3)
DEFINE_CHECKED_OPERATOR(cca_Object___setrange__, OPERATOR_SETRANGE, 4)
DEFINE_CHECKED_OPERATOR(cca_Object___iterself__, OPERATOR_ITERSELF, 1)
DEFINE_CHECKED_OPERATOR(cca_Object___iternext__, OPERATOR_ITERNEXT, 1)
DEFINE_CHECKED_OPTIMIZATION(cca_Object___getattr__, Dee_function_generator_vopgetattr(self)) /* 2 */
DEFINE_UNCHECKED_OPTIMIZATION(cca_Object___callattr__,
                              Dee_function_generator_vassert_type_exact_c(self, &DeeTuple_Type) ||
                              Dee_function_generator_vopcallattrtuple_unchecked(self))       /* 3 */
DEFINE_CHECKED_OPTIMIZATION(cca_Object___hasattr__, Dee_function_generator_vophasattr(self)) /* 2 */
DEFINE_CHECKED_OPTIMIZATION(cca_Object___delattr__, Dee_function_generator_vopdelattr(self)) /* 2 */
DEFINE_CHECKED_OPTIMIZATION(cca_Object___setattr__, Dee_function_generator_vopsetattr(self)) /* 3 */

PRIVATE struct Dee_ccall_optimization tpconst cca_Object[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("__add__", &cca_Object___add__, 1),
	CCA_OPTIMIZATION("__and__", &cca_Object___and__, 1),
	CCA_OPTIMIZATION("__assign__", &cca_Object___assign__, 1),
	CCA_OPTIMIZATION("__bool__", &cca_Object___bool__, 0),
	CCA_OPTIMIZATION("__call__", &cca_Object___call__, 1),
	CCA_OPTIMIZATION("__callattr__", &cca_Object___callattr__, 2),
	CCA_OPTIMIZATION("__contains__", &cca_Object___contains__, 1),
	CCA_OPTIMIZATION("__copy__", &cca_Object___copy__, 0),
	CCA_OPTIMIZATION("__deepcopy__", &cca_Object___deepcopy__, 0),
	CCA_OPTIMIZATION("__delattr__", &cca_Object___delattr__, 1),
	CCA_OPTIMIZATION("__delitem__", &cca_Object___delitem__, 1),
	CCA_OPTIMIZATION("__delrange__", &cca_Object___delrange__, 2),
	CCA_OPTIMIZATION("__div__", &cca_Object___div__, 1),
	CCA_OPTIMIZATION("__eq__", &cca_Object___eq__, 1),
	CCA_OPTIMIZATION("__ge__", &cca_Object___ge__, 1),
	CCA_OPTIMIZATION("__getattr__", &cca_Object___getattr__, 1),
	CCA_OPTIMIZATION("__getitem__", &cca_Object___getitem__, 1),
	CCA_OPTIMIZATION("__getrange__", &cca_Object___getrange__, 2),
	CCA_OPTIMIZATION("__gr__", &cca_Object___gr__, 1),
	CCA_OPTIMIZATION("__hasattr__", &cca_Object___hasattr__, 1),
	CCA_OPTIMIZATION("__hash__", &cca_Object___hash__, 0),
	CCA_OPTIMIZATION("__int__", &cca_Object___int__, 0),
	CCA_OPTIMIZATION("__inv__", &cca_Object___inv__, 0),
	CCA_OPTIMIZATION("__iternext__", &cca_Object___iternext__, 0),
	CCA_OPTIMIZATION("__iterself__", &cca_Object___iterself__, 0),
	CCA_OPTIMIZATION("__le__", &cca_Object___le__, 1),
	CCA_OPTIMIZATION("__lo__", &cca_Object___lo__, 1),
	CCA_OPTIMIZATION("__mod__", &cca_Object___mod__, 1),
	CCA_OPTIMIZATION("__moveassign__", &cca_Object___moveassign__, 1),
	CCA_OPTIMIZATION("__mul__", &cca_Object___mul__, 1),
	CCA_OPTIMIZATION("__ne__", &cca_Object___ne__, 1),
	CCA_OPTIMIZATION("__neg__", &cca_Object___neg__, 0),
	CCA_OPTIMIZATION("__or__", &cca_Object___or__, 1),
	CCA_OPTIMIZATION("__pos__", &cca_Object___pos__, 0),
	CCA_OPTIMIZATION("__pow__", &cca_Object___pow__, 1),
	CCA_OPTIMIZATION("__repr__", &cca_Object___repr__, 0),
	CCA_OPTIMIZATION("__setattr__", &cca_Object___setattr__, 2),
	CCA_OPTIMIZATION("__setitem__", &cca_Object___setitem__, 2),
	CCA_OPTIMIZATION("__setrange__", &cca_Object___setrange__, 3),
	CCA_OPTIMIZATION("__shl__", &cca_Object___shl__, 1),
	CCA_OPTIMIZATION("__shr__", &cca_Object___shr__, 1),
	CCA_OPTIMIZATION("__size__", &cca_Object___size__, 0),
	CCA_OPTIMIZATION("__str__", &cca_Object___str__, 0),
	CCA_OPTIMIZATION("__sub__", &cca_Object___sub__, 1),
	CCA_OPTIMIZATION("__thiscall__", &cca_Object___thiscall__, 2),
	CCA_OPTIMIZATION("__xor__", &cca_Object___xor__, 1),
};




/************************************************************************/
/* Sequence                                                             */
/************************************************************************/

/* seq -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_sum(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SEQ_ENUMERATE, 1));
	return Dee_function_generator_vcallapi(self, &DeeSeq_Sum, VCALL_CC_RAWINTPTR, 1); /* result */
err:
	return -1;
}

/* seq -> CHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
impl_cca_Sequence_anyall(struct Dee_function_generator *__restrict self,
                         void const *api_function) {
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SEQ_ENUMERATE, 1));
	DO(Dee_function_generator_vcallapi(self, api_function, VCALL_CC_NEGINT, 1));
	DO(Dee_function_generator_vdirect(self, 1));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
	Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* seq -> CHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_any(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return impl_cca_Sequence_anyall(self, (void const *)&DeeSeq_Any);
}

/* seq -> CHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_all(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return impl_cca_Sequence_anyall(self, (void const *)&DeeSeq_All);
}


/* seq, [key] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
impl_cca_Sequence_minmax(struct Dee_function_generator *__restrict self,
                         Dee_vstackaddr_t argc, void const *api_function) {
	if (argc >= 1) {
		DO(Dee_function_generator_vcoalesce_c(self, Dee_None, NULL)); /* seq, key */
	} else {
		DO(Dee_function_generator_vpush_NULL(self)); /* seq, key */
	}
	DO(Dee_function_generator_vnotoneref_at(self, 1));
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SEQ_ENUMERATE, 2));
	return Dee_function_generator_vcallapi(self, api_function, VCALL_CC_RAWINTPTR, 2);
err:
	return -1;
}

/* seq, [key] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_min(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	return impl_cca_Sequence_minmax(self, argc, (void const *)&DeeSeq_Min);
}

/* seq, [key] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_max(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	return impl_cca_Sequence_minmax(self, argc, (void const *)&DeeSeq_Max);
}

PRIVATE struct Dee_ccall_optimization tpconst cca_Sequence[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("all", &cca_Sequence_all, 0),
	CCA_OPTIMIZATION("any", &cca_Sequence_any, 0),
	CCA_OPTIMIZATION("max", &cca_Sequence_max, 0),
	CCA_OPTIMIZATION("max", &cca_Sequence_max, 1),
	CCA_OPTIMIZATION("min", &cca_Sequence_min, 0),
	CCA_OPTIMIZATION("min", &cca_Sequence_min, 1),
	CCA_OPTIMIZATION("sum", &cca_Sequence_sum, 0),
	/* TODO: When types are known, we can pretty much fully inline stuff like "Sequence.insert()" */
};




/************************************************************************/
/* Mapping                                                             */
/************************************************************************/

/* map, key, [def] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Mapping_get(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	if (argc < 2)
		DO(Dee_function_generator_vpush_const(self, Dee_None));
	DO(Dee_function_generator_vopgetitemdef(self));
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* map, key, [value] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Mapping_setdefault(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	struct Dee_memloc l_ITER_DONE;
	DREF struct Dee_memstate *common_state;
	struct Dee_host_symbol *Lnot_ITER_DONE;
	struct type_nsi const *nsi;
	DeeTypeObject *map_type = Dee_memloc_typeof(Dee_function_generator_vtop(self) - argc);
	if unlikely(!map_type)
		return 1; /* Shouldn't happen since we only get called when types are known... */
	if (argc < 2)
		DO(Dee_function_generator_vpush_const(self, Dee_None)); /* map, key, value */
	nsi = DeeType_NSI(map_type);
	if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP && nsi->nsi_maplike.nsi_setdefault) {
		DO(Dee_function_generator_vnotoneref(self, 2));                                  /* this, key, value */
		DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SETITEM, 2)); /* this, key, value */
		DO(Dee_function_generator_vcallapi(self, nsi->nsi_maplike.nsi_setdefault, VCALL_CC_OBJECT, 3));
		Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
		return 0;
	}
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		return 1; /* Optimized version would be too long. */

	/* Fallback: lookup key and override if not already present (thread-unsafe) */
	DO(Dee_function_generator_vnotoneref(self, 2));         /* this, key, value */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SETITEM, 2)); /* this, key, value */
	DO(Dee_function_generator_vdup_n(self, 3));             /* this, key, value, this */
	DO(Dee_function_generator_vdup_n(self, 3));             /* this, key, value, this, key */
	DO(Dee_function_generator_vpush_addr(self, ITER_DONE)); /* this, key, value, this, key, ITER_DONE */
	DO(Dee_function_generator_vopgetitemdef(self));         /* this, key, value, current_value */

	/* >> if (current_value == ITER_DONE) {
	 * >>     if (DeeObject_SetItem(this, key, value))
	 * >>         HANDLE_EXCEPT();
	 * >>     result = value;
	 * >>     Dee_Incref(value);
	 * >> } */
	Lnot_ITER_DONE = Dee_function_generator_newsym(self);
	if unlikely(!Lnot_ITER_DONE)
		goto err;
	l_ITER_DONE.ml_type = MEMLOC_TYPE_CONST;
	l_ITER_DONE.ml_value.v_const = ITER_DONE;
	DO(_Dee_function_generator_gjcmp(self, Dee_function_generator_vtop(self), &l_ITER_DONE,
	                                 false, Lnot_ITER_DONE, NULL, Lnot_ITER_DONE));
	DO(Dee_function_generator_state_unshare(self));
	common_state = self->fg_state;
	Dee_memstate_vtop(common_state)->ml_flags &= ~MEMLOC_F_NOREF;
	Dee_memstate_incref(common_state);                                                /* this, key, value, ref:current_value */
	EDO(err_common_state, Dee_function_generator_state_unshare(self));                /* this, key, value, ref:current_value */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF;                    /* this, key, value, current_value */
	EDO(err_common_state, Dee_function_generator_vpop(self));                         /* this, key, value */
	EDO(err_common_state, Dee_function_generator_vdup_n(self, 3));                    /* this, key, value, this */
	EDO(err_common_state, Dee_function_generator_vdup_n(self, 3));                    /* this, key, value, this, key */
	EDO(err_common_state, Dee_function_generator_vdup_n(self, 3));                    /* this, key, value, this, key, value */
	EDO(err_common_state, Dee_function_generator_vop(self, OPERATOR_SETITEM, 3, VOP_F_NORMAL)); /* this, key, value */
	EDO(err_common_state, Dee_function_generator_vdup(self));                         /* this, key, value, value */
	EDO(err_common_state, Dee_function_generator_gincref(self, Dee_function_generator_vtop(self), 1)); /* this, key, value, value */
	Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;                   /* this, key, value, ref:value */
	EDO(err_common_state, Dee_function_generator_vmorph(self, common_state));         /* this, key, value, ref:current_value */
	Dee_memstate_decref(common_state);
	Dee_host_symbol_setsect(Lnot_ITER_DONE, self->fg_sect);
	HA_printf("Lnot_ITER_DONE:\n");            /* this, key, value, ref:current_value */
	DO(Dee_function_generator_vrrot(self, 4)); /* ref:current_value, this, key, value */
	DO(Dee_function_generator_vpop(self));     /* ref:current_value, this, key */
	DO(Dee_function_generator_vpop(self));     /* ref:current_value, this */
	DO(Dee_function_generator_vpop(self));     /* ref:current_value */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}

PRIVATE struct Dee_ccall_optimization tpconst cca_Mapping[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("get", &cca_Mapping_get, 1),
	CCA_OPTIMIZATION("get", &cca_Mapping_get, 2),
	CCA_OPTIMIZATION("setdefault", &cca_Mapping_setdefault, 1),
	CCA_OPTIMIZATION("setdefault", &cca_Mapping_setdefault, 2),
	/* TODO: setold() */
	/* TODO: setnew() */
	/* TODO: setold_ex() */
	/* TODO: setnew_ex() */
};




/************************************************************************/
/* List                                                                 */
/************************************************************************/

/* this, [args...] -> CHECKED(none) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_append(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	DO(Dee_function_generator_vnotoneref(self, argc)); /* this, [args...] */
	while (argc) {
		/* XXX: Pre-reserve memory when multiple arguments are given? */
		/* XXX: Use a different function `DeeList_AppendInherted()', so we don't have to decref the appended object? */
		DO(Dee_function_generator_vdup_n(self, argc + 1));                              /* this, [args...], this */
		DO(Dee_function_generator_vlrot(self, argc + 1));                               /* this, [moreargs...], this, arg */
		DO(Dee_function_generator_vcallapi(self, &DeeList_Append, VCALL_CC_INT, 2)); /* this, [moreargs...] */
		--argc;
	}
	DO(Dee_function_generator_vpop(self));                  /* N/A */
	DO(Dee_function_generator_vpush_const(self, Dee_None)); /* none */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this, seq -> CHECKED(none) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_extend(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	DeeTypeObject *seqtype;
	(void)argc;
	seqtype = Dee_function_generator_vtoptype(self);
	if (seqtype == &DeeTuple_Type) {
		DO(Dee_function_generator_vdup(self));                                     /* this, seq, seq */
		DO(Dee_function_generator_vrrot(self, 3));                                 /* seq, this, seq */
		DO(Dee_function_generator_vdup(self));                                     /* seq, this, seq, seq */
		DO(Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size)));   /* seq, this, seq, seq->t_size */
		DO(Dee_function_generator_vswap(self));                                    /* seq, this, seq->t_size, seq */
		DO(Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* seq, this, seq->t_size, seq->t_elem */
		DO(Dee_function_generator_vcallapi(self, &DeeList_AppendVector, VCALL_CC_INT, 3)); /* seq */
		DO(Dee_function_generator_vpop(self));                                     /* N/A */
	} else {
		DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SEQ_ENUMERATE, 1)); /* this, seq */
		DO(Dee_function_generator_vcallapi(self, &DeeList_AppendSequence, VCALL_CC_INT, 2)); /* N/A */
	}
	DO(Dee_function_generator_vpush_const(self, Dee_None)); /* none */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this, index, item -> CHECKED(none) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_insert(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	DO(Dee_function_generator_vswap(self));            /* this, item, index */
	DO(Dee_function_generator_vmorph_uint(self));      /* this, item, uint:index */
	DO(Dee_function_generator_vswap(self));            /* this, uint:index, item */
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* this, uint:index, item */
	DO(Dee_function_generator_vcallapi(self, &DeeList_Insert, VCALL_CC_INT, 3)); /* N/A */
	DO(Dee_function_generator_vpush_const(self, Dee_None)); /* none */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this, index, seq -> CHECKED(none) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_insertall(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	DeeTypeObject *seqtype = Dee_function_generator_vtoptype(self);
	(void)argc;
	DO(Dee_function_generator_vswap(self));       /* this, seq, index */
	DO(Dee_function_generator_vmorph_uint(self)); /* this, seq, uint:index */
	DO(Dee_function_generator_vswap(self));       /* this, uint:index, seq */
	if (seqtype == &DeeTuple_Type) {
		DO(Dee_function_generator_vdup(self));                                     /* this, uint:index, seq, seq */
		DO(Dee_function_generator_vrrot(self, 4));                                 /* seq, this, uint:index, seq */
		DO(Dee_function_generator_vdup(self));                                     /* seq, this, uint:index, seq, seq */
		DO(Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size)));   /* seq, this, uint:index, seq, seq->t_size */
		DO(Dee_function_generator_vswap(self));                                    /* seq, this, uint:index, seq->t_size, seq */
		DO(Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* seq, this, uint:index, seq->t_size, seq->t_elem */
		DO(Dee_function_generator_vcallapi(self, &DeeList_InsertVector, VCALL_CC_INT, 4)); /* seq */
		DO(Dee_function_generator_vpop(self));                                     /* N/A */
	} else {
		DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SEQ_ENUMERATE, 1)); /* this, uint:index, seq */
		DO(Dee_function_generator_vcallapi(self, &DeeList_InsertSequence, VCALL_CC_INT, 3)); /* N/A */
	}
	DO(Dee_function_generator_vpush_const(self, Dee_None)); /* none */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this, should, [start, [end]] -> CHECKED(count) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_removeif(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	if (argc >= 3) {
		DO(Dee_function_generator_vmorph_uint(self)); /* this, should, start, uint:end */
		DO(Dee_function_generator_vswap(self));       /* this, should, uint:end, start */
		DO(Dee_function_generator_vmorph_uint(self)); /* this, should, uint:end, uint:start */
		DO(Dee_function_generator_vswap(self));       /* this, should, uint:start, uint:end */
	} else if (argc >= 2) {
		DO(Dee_function_generator_vmorph_uint(self));              /* this, should, uint:start */
		DO(Dee_function_generator_vpush_immSIZ(self, (size_t)-1)); /* this, should, uint:start, uint:end */
	} else {
		DO(Dee_function_generator_vpush_immSIZ(self, (size_t)-1)); /* this, should, uint:start */
		DO(Dee_function_generator_vpush_immSIZ(self, (size_t)-1)); /* this, should, uint:start, uint:end */
	}
	DO(Dee_function_generator_vlrot(self, 3));      /* this, uint:start, uint:end, should */
	DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_CALL, 1)); /* this, uint:start, uint:end, should */
	DO(Dee_function_generator_vcallapi(self, &DeeList_RemoveIf, VCALL_CC_M1INTPTR, 4)); /* result */
	Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_UINT;
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}


/* this, index, [count] -> CHECKED(count) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_erase(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	if (argc >= 2) {
		DO(Dee_function_generator_vmorph_uint(self)); /* this, index, uint:count */
		DO(Dee_function_generator_vswap(self));       /* this, uint:count, index */
		DO(Dee_function_generator_vmorph_uint(self)); /* this, uint:count, uint:index */
		DO(Dee_function_generator_vswap(self));       /* this, uint:index, uint:count */
	} else {
		DO(Dee_function_generator_vmorph_uint(self));     /* this, uint:index */
		DO(Dee_function_generator_vpush_immSIZ(self, 1)); /* this, uint:index, uint:count */
	}
	DO(Dee_function_generator_vcallapi(self, &DeeList_Erase, VCALL_CC_M1INTPTR, 3)); /* result */
	Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_UINT;
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this -> CHECKED(none) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_clear(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	DO(Dee_function_generator_vcallapi(self, &DeeList_Clear, VCALL_CC_VOID, 1)); /* N/A */
	DO(Dee_function_generator_vpush_const(self, Dee_None)); /* none */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this, [index] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_pop(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	if (argc >= 1) {
		DO(Dee_function_generator_vmorph_int(self)); /* this, int:index */
	} else {
		DO(Dee_function_generator_vpush_immSIZ(self, -1)); /* this, int:index */
	}
	return Dee_function_generator_vcallapi(self, &DeeList_Pop, VCALL_CC_RAWINTPTR, 2); /* UNCHECKED(result) */
err:
	return -1;
}

/* this, [key] -> CHECKED(none) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_sort(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	if (argc >= 1) {
		DeeTypeObject *known_type;
		known_type = Dee_function_generator_vtoptype(self);
		if (known_type == &DeeNone_Type) {
			DO(Dee_function_generator_vpop(self)); /* this */
			goto use_null_key;
		} else if (known_type != NULL) {
			/* ... */
		} else {
			DO(Dee_function_generator_vcoalesce_c(self, Dee_None, NULL)); /* this, key */
		}
		DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_CALL, 1)); /* this, key */
	} else {
use_null_key:
		DO(Dee_function_generator_vpush_addr(self, NULL)); /* this, key */
	}
	DO(Dee_function_generator_vcallapi(self, &DeeList_Sort, VCALL_CC_INT, 2)); /* N/A */
	DO(Dee_function_generator_vpush_const(self, Dee_None)); /* none */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this, newsize, [filler] -> CHECKED(none) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_resize(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	if (argc >= 2) {
		DO(Dee_function_generator_vswap(self));       /* this, filler, newsize */
		DO(Dee_function_generator_vmorph_uint(self)); /* this, filler, uint:newsize */
		DO(Dee_function_generator_vswap(self));       /* this, uint:newsize, filler */
	} else {
		DO(Dee_function_generator_vmorph_uint(self));           /* this, uint:newsize */
		DO(Dee_function_generator_vpush_const(self, Dee_None)); /* this, uint:newsize, fillter */
	}
	DO(Dee_function_generator_vcallapi(self, &DeeList_Resize, VCALL_CC_INT, 3)); /* N/A */
	DO(Dee_function_generator_vpush_const(self, Dee_None)); /* none */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

PRIVATE struct Dee_ccall_optimization tpconst cca_List[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("append", &cca_List_append, Dee_CCALL_ARGC_ANY),
	CCA_OPTIMIZATION("clear", &cca_List_clear, 0),
	CCA_OPTIMIZATION("erase", &cca_List_erase, 1),
	CCA_OPTIMIZATION("erase", &cca_List_erase, 2),
	CCA_OPTIMIZATION("extend", &cca_List_extend, 1),
	CCA_OPTIMIZATION("insert", &cca_List_insert, 2),
	CCA_OPTIMIZATION("insertall", &cca_List_insertall, 2),
	CCA_OPTIMIZATION("pop", &cca_List_pop, 0),
	CCA_OPTIMIZATION("pop", &cca_List_pop, 1),
	CCA_OPTIMIZATION("removeif", &cca_List_removeif, 1),
	CCA_OPTIMIZATION("removeif", &cca_List_removeif, 2),
	CCA_OPTIMIZATION("removeif", &cca_List_removeif, 3),
	CCA_OPTIMIZATION("resize", &cca_List_resize, 1),
	CCA_OPTIMIZATION("resize", &cca_List_resize, 2),
	CCA_OPTIMIZATION("sort", &cca_List_sort, 0),
	CCA_OPTIMIZATION("sort", &cca_List_sort, 1),
};

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_List_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeListObject, l_list.ol_elemc));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_List_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeListObject, l_list.ol_elemc));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_List[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_List_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_List_size, 0),
};




/************************************************************************/
/* String                                                               */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_String_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeStringObject, s_len));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_String_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeStringObject, s_len));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_String[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_String_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_String_size, 0),
};




/************************************************************************/
/* Bytes                                                                */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Bytes_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeBytesObject, b_size));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Bytes_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeBytesObject, b_size));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_Bytes[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Bytes_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_Bytes_size, 0),
};




/************************************************************************/
/* Tuple                                                                */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Tuple_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeTupleObject, t_size));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Tuple_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeTupleObject, t_size));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_Tuple[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Tuple_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_Tuple_size, 0),
};




/************************************************************************/
/* HashSet                                                              */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_HashSet_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeHashSetObject, hs_used));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_HashSet_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeHashSetObject, hs_used));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_HashSet[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_HashSet_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_HashSet_size, 0),
};




/************************************************************************/
/* RoSet                                                                */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoSet_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeRoSetObject, rs_size));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoSet_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeRoSetObject, rs_size));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_RoSet[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_RoSet_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_RoSet_size, 0),
};




/************************************************************************/
/* Dict                                                                 */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Dict_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeDictObject, d_used));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Dict_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeDictObject, d_used));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_Dict[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Dict_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_Dict_size, 0),
};




/************************************************************************/
/* RoDict                                                               */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoDict_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeRoDictObject, rd_size));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoDict_size(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeRoDictObject, rd_size));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_RoDict[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_RoDict_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_RoDict_size, 0),
};




/************************************************************************/
/* Int                                                                  */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Int_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeIntObject, ob_size));
}

PRIVATE struct Dee_ccall_optimization tpconst cco_Int[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Int_bool, 0),
};




/************************************************************************/
/* WeakRef                                                              */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_WeakRef_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	DO(Dee_function_generator_vdelta(self, offsetof(DeeWeakRefObject, wr_ref)));        /* &wr->wr_ref */
	return Dee_function_generator_vcallapi(self, &Dee_weakref_bound, VCALL_CC_BOOL, 1); /* result */
err:
	return -1;
}

PRIVATE struct Dee_ccall_optimization tpconst cco_WeakRef[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_WeakRef_bool, 0),
};




/************************************************************************/
/* Cell                                                                 */
/************************************************************************/

/* cell -> value */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_DeeCell_Get_or_Error(struct Dee_function_generator *__restrict self,
                           DeeTypeObject *error_type) {
	DREF struct Dee_memstate *saved_state;
	struct Dee_host_section *text, *cold;
	void const *except_api;
	ASSERT(error_type == &DeeError_ValueError ||
	       error_type == &DeeError_UnboundAttribute);
	except_api = (void const *)&libhostasm_rt_err_cell_empty_ValueError;
	if (error_type == &DeeError_UnboundAttribute)
		except_api = (void const *)&libhostasm_rt_err_cell_empty_UnboundAttribute;
	DO(Dee_function_generator_vdup(self));                                    /* cell, cell */
	DO(Dee_function_generator_vdelta(self, offsetof(DeeCellObject, c_lock))); /* cell, &cell->c_lock */
	DO(Dee_function_generator_vrwlock_read(self));                            /* cell */
	DO(Dee_function_generator_vdup(self));                                    /* cell, cell */
	DO(Dee_function_generator_vind(self, offsetof(DeeCellObject, c_item)));   /* cell, cell->c_item */
	DO(Dee_function_generator_vreg(self, NULL));                              /* cell, reg:cell->c_item */
	text = self->fg_sect;
	cold = &self->fg_block->bb_hcold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if (text == cold) {
		struct Dee_host_symbol *Lcell_not_empty;
		Lcell_not_empty = Dee_function_generator_newsym(self);
		if unlikely(!Lcell_not_empty)
			goto err;
		Dee_host_symbol_setname(Lcell_not_empty, ".Lcell_not_empty");
		DO(_Dee_function_generator_gjnz(self, Dee_function_generator_vtop(self), Lcell_not_empty)); /* cell, reg:cell->c_item */
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		EDO(err_saved_state, Dee_function_generator_vcallapi(self, except_api, VCALL_CC_EXCEPT, 0));
		HA_printf(".Lcell_not_empty:\n");
		Dee_host_symbol_setsect(Lcell_not_empty, self->fg_sect);
	} else {
		struct Dee_host_symbol *Lcell_empty;
		Lcell_empty = Dee_function_generator_newsym(self);
		if unlikely(!Lcell_empty)
			goto err;
		Dee_host_symbol_setname(Lcell_empty, ".Lcell_empty");
		DO(_Dee_function_generator_gjz(self, Dee_function_generator_vtop(self), Lcell_empty)); /* cell, reg:cell->c_item */
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		HA_printf(".section .cold\n");
		self->fg_sect = cold;
		HA_printf(".Lcell_empty:\n");
		Dee_host_symbol_setsect(Lcell_empty, self->fg_sect);
		EDO(err_saved_state, Dee_function_generator_vcallapi(self, except_api, VCALL_CC_EXCEPT, 0));
		HA_printf(".section .text\n");
		self->fg_sect = text;
	}
	Dee_memstate_decref(self->fg_state);
	self->fg_state = saved_state;                                                   /* cell, reg:cell->c_item */
	DO(Dee_function_generator_gincref(self, Dee_function_generator_vtop(self), 1)); /* cell, ref:cell->c_item */
	Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;                 /* cell, ref:cell->c_item */
	DO(Dee_function_generator_vswap(self));                                         /* ref:cell->c_item, cell */
	DO(Dee_function_generator_vdup(self));                                          /* ref:cell->c_item, cell, cell */
	DO(Dee_function_generator_vdelta(self, offsetof(DeeCellObject, c_lock)));       /* ref:cell->c_item, cell, &cell->c_lock */
	DO(Dee_function_generator_vrwlock_endread(self));                               /* ref:cell->c_item, cell */
	return Dee_function_generator_vpop(self);                                       /* ref:cell->c_item */
err_saved_state:
	Dee_memstate_decref(saved_state);
err:
	return -1;
}

/* cell, ref:value -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vcall_DeeCell_DelOrSet(struct Dee_function_generator *__restrict self) {
	DO(Dee_function_generator_vdup_n(self, 2));                                      /* cell, ref:value, cell */
	DO(Dee_function_generator_vdelta(self, offsetof(DeeCellObject, c_lock)));        /* cell, ref:value, &cell->c_lock */
	DO(Dee_function_generator_vrwlock_write(self));                                  /* cell, ref:value */
	DO(Dee_function_generator_vdup_n(self, 2));                                      /* cell, ref:value, cell */
	DO(Dee_function_generator_vswap(self));                                          /* cell, cell, ref:value */
	DO(Dee_function_generator_vswapind(self, offsetof(DeeCellObject, c_item)));      /* cell, ref:old_value */
	DO(Dee_function_generator_vdup_n(self, 2));                                      /* cell, ref:old_value, cell */
	DO(Dee_function_generator_vdelta(self, offsetof(DeeCellObject, c_lock)));        /* cell, ref:old_value, &cell->c_lock */
	DO(Dee_function_generator_vrwlock_endwrite(self));                               /* cell, ref:old_value */
	DO(Dee_function_generator_vswap(self));                                          /* ref:old_value, cell */
	DO(Dee_function_generator_vpop(self));                                           /* ref:old_value */
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);            /* ref:old_value */
	DO(Dee_function_generator_gxdecref(self, Dee_function_generator_vtop(self), 1)); /* old_value */
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

/* this, [def] -> UNCHECKED(value) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_get(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	DREF struct Dee_memstate *common_state;
	if (argc == 0) {
		DO(vcall_DeeCell_Get_or_Error(self, &DeeError_ValueError));
	} else {
		struct Dee_host_symbol *Lpresent;
		DO(Dee_function_generator_vswap(self));                                            /* def, this */
		DO(Dee_function_generator_vcallapi(self, &DeeCell_TryGet, VCALL_CC_RAWINTPTR, 1)); /* def, UNCHECKED(result) */
		Lpresent = Dee_function_generator_newsym(self);
		if unlikely(!Lpresent)
			goto err;
		Dee_host_symbol_setname(Lpresent, ".Lpresent");
		DO(_Dee_function_generator_gjnz(self, Dee_function_generator_vtop(self), Lpresent)); /* def, result */
		DO(Dee_function_generator_state_unshare(self));
		common_state = self->fg_state;
		Dee_memstate_vtop(common_state)->ml_flags &= ~MEMLOC_F_NOREF;
		Dee_memstate_incref(common_state);
		EDO(err_common_state, Dee_function_generator_state_unshare(self));
		Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF;
		EDO(err_common_state, Dee_function_generator_vpop(self));                 /* def */
		EDO(err_common_state, Dee_function_generator_vdup(self));                 /* def, def */
		EDO(err_common_state, Dee_function_generator_gincref(self, Dee_function_generator_vtop(self), 1)); /* def, def */
		Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;           /* def, ref:def */
		EDO(err_common_state, Dee_function_generator_vmorph(self, common_state)); /* def, result */
		Dee_memstate_decref(common_state);
		HA_printf(".Lpresent:\n");
		Dee_host_symbol_setsect(Lpresent, self->fg_sect);
		DO(Dee_function_generator_vswap(self)); /* result, def */
		DO(Dee_function_generator_vpop(self));  /* result */
	}
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}

/* this -> CHECKED(value) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_value_getter(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		return Dee_function_generator_vcallapi(self, &DeeCell_Get, VCALL_CC_OBJECT, 1);
	DO(vcall_DeeCell_Get_or_Error(self, &DeeError_UnboundAttribute));
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_OBJCHECKED;
	return 0;
err:
	return -1;
}

/* this -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_value_delete(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		return Dee_function_generator_vcallapi(self, &DeeCell_Del, VCALL_CC_VOID, 1);
	DO(Dee_function_generator_vpush_NULL(self)); /* cell, NULL */
	return vcall_DeeCell_DelOrSet(self);
err:
	return -1;
}

/* this, value -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_value_setter(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		return Dee_function_generator_vcallapi(self, &DeeCell_Set, VCALL_CC_VOID, 2);
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* cell, value */
	DO(Dee_function_generator_vref2(self));            /* cell, ref:value */
	return vcall_DeeCell_DelOrSet(self);
err:
	return -1;
}

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Cell_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeCellObject, c_item));
}

PRIVATE struct Dee_ccall_optimization tpconst cca_Cell[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("get", &cca_Cell_get, 0),
	CCA_OPTIMIZATION("get", &cca_Cell_get, 1),
	CCA_OPTIMIZATION("value", &cca_Cell_value_delete, Dee_CCALL_ARGC_DELETE),
	CCA_OPTIMIZATION("value", &cca_Cell_value_getter, Dee_CCALL_ARGC_GETTER),
	CCA_OPTIMIZATION("value", &cca_Cell_value_setter, Dee_CCALL_ARGC_SETTER),
	CCA_OPTIMIZATION("value", &cco_Cell_bool, Dee_CCALL_ARGC_BOUND),
};

PRIVATE struct Dee_ccall_optimization tpconst cco_Cell[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Cell_bool, 0),
};




/************************************************************************/
/* None                                                                 */
/************************************************************************/

/* this, [args...] -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_none(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = Dee_function_generator_vpush_const(self, Dee_None);
	return result;
}

/* this, [args...] -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_popall(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = Dee_function_generator_vpush_const(self, Dee_None);
	return result;
}

#define str_none (*COMPILER_CONTAINER_OF(DeeNone_Type.tp_name, DeeStringObject, s_str))
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_str_none(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = Dee_function_generator_vpush_const(self, &str_none);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_false(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = Dee_function_generator_vpush_const(self, Dee_False);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_zero(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = Dee_function_generator_vpush_const(self, DeeInt_Zero);
	return result;
}

/* none, other -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_cmp_eq(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	DO(Dee_function_generator_vswap(self));                     /* other, none */
	DO(Dee_function_generator_vpop(self));                      /* other */
	return Dee_function_generator_veqconstaddr(self, Dee_None); /* other==Dee_None */
err:
	return -1;
}

/* none, other -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_cmp_ne(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	int result = cco_None_cmp_eq(self, argc);
	if likely(result == 0)
		result = Dee_function_generator_vopnot(self);
	return result;
}

PRIVATE struct Dee_ccall_optimization tpconst cco_None[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0001_COPY, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0002_DEEPCOPY, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0004_ASSIGN, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0005_MOVEASSIGN, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0006_STR, &cco_None_return_str_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0007_REPR, &cco_None_return_str_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_None_return_false, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000A_CALL, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000B_INT, &cco_None_return_zero, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000D_INV, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000E_POS, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000F_NEG, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0010_ADD, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0011_SUB, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0012_MUL, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0013_DIV, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0014_MOD, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0015_SHL, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0016_SHR, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0017_AND, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0018_OR, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0019_XOR, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001A_POW, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001B_INC, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001C_DEC, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001D_INPLACE_ADD, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001E_INPLACE_SUB, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001F_INPLACE_MUL, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0020_INPLACE_DIV, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0021_INPLACE_MOD, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0022_INPLACE_SHL, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0023_INPLACE_SHR, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0024_INPLACE_AND, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0025_INPLACE_OR, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0026_INPLACE_XOR, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0027_INPLACE_POW, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0028_HASH, &cco_None_return_zero, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0029_EQ, &cco_None_cmp_eq, 1),
	CCO_OPTIMIZATION(OPERATOR_002A_NE, &cco_None_cmp_ne, 1),
	CCO_OPTIMIZATION(OPERATOR_002B_LO, &cco_None_cmp_ne, 1),
	CCO_OPTIMIZATION(OPERATOR_002C_LE, &cco_None_cmp_eq, 1),
	CCO_OPTIMIZATION(OPERATOR_002D_GR, &cco_None_cmp_ne, 1),
	CCO_OPTIMIZATION(OPERATOR_002E_GE, &cco_None_cmp_eq, 1),
	CCO_OPTIMIZATION(OPERATOR_002F_ITERSELF, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0031_CONTAINS, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0032_GETITEM, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0033_DELITEM, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0034_SETITEM, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0035_GETRANGE, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0036_DELRANGE, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0037_SETRANGE, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0038_GETATTR, &cco_None_return_none, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0039_DELATTR, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_003A_SETATTR, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_003C_ENTER, &cco_None_popall, Dee_CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_003D_LEAVE, &cco_None_popall, Dee_CCALL_ARGC_ANY),
};




/************************************************************************/
/* Bool                                                                 */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Bool_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	/* Already a boolean, so nothing to do here! */
	(void)self;
	(void)argc;
	return 0;
}

PRIVATE struct Dee_ccall_optimization tpconst cco_Bool[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Bool_bool, 0),
};




#ifdef CONFIG_HAVE_FPU
/************************************************************************/
/* Float                                                                 */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Float_bool(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc) {
	(void)argc;
	return Dee_function_generator_vcallapi(self, DeeFloat_Type.tp_cast.tp_bool, VCALL_CC_BOOL, 1); /* result */
}

PRIVATE struct Dee_ccall_optimization tpconst cco_Float[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Float_bool, 0),
};
#endif /* CONFIG_HAVE_FPU */




#undef DEFINE_UNCHECKED_OPTIMIZATION
#undef DEFINE_CHECKED_OPTIMIZATION
#undef DEFINE_CHECKED_OPERATOR

struct ccall_optimizations_struct {
	DeeTypeObject                 const *tccos_type; /* [1..1] The type to which these optimizations apply. */
	struct Dee_ccall_optimization const *tccos_opts; /* [0..tccos_size] Vector of optimization handlers (sorted lexicographically by `tcco_name'). */
	size_t                               tccos_size; /* [1..1][const] # of optimization handlers. */
};

/* Attribute optimizations for known types. */
PRIVATE struct ccall_optimizations_struct tpconst cca_optimizations[] = {
	/* TODO: Optimize known functions from deemon.Sequence into their NSI equivalents (if available) */
#define DEFINE_OBJMETHOD_OPTIMIZATIIONS(type, vec) { type, vec, COMPILER_LENOF(vec) }
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeObject_Type, cca_Object),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeSeq_Type, cca_Sequence),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeList_Type, cca_List),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeCell_Type, cca_Cell),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeMapping_Type, cca_Mapping),
#undef DEFINE_OBJMETHOD_OPTIMIZATIIONS
};


/* Operator optimizations for known types. */
PRIVATE struct ccall_optimizations_struct tpconst cco_optimizations[] = {
#define DEFINE_OBJMETHOD_OPTIMIZATIIONS(type, vec) { type, vec, COMPILER_LENOF(vec) }
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeInt_Type, cco_Int),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeString_Type, cco_String),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeBytes_Type, cco_Bytes),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeTuple_Type, cco_Tuple),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeList_Type, cco_List),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeHashSet_Type, cco_HashSet),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeRoSet_Type, cco_RoSet),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeDict_Type, cco_Dict),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeRoDict_Type, cco_RoDict),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeCell_Type, cco_Cell),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeWeakRef_Type, cco_WeakRef),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeNone_Type, cco_None),
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeBool_Type, cco_Bool),
#ifdef CONFIG_HAVE_FPU
	DEFINE_OBJMETHOD_OPTIMIZATIIONS(&DeeFloat_Type, cco_Float),
#endif /* CONFIG_HAVE_FPU */
#undef DEFINE_OBJMETHOD_OPTIMIZATIIONS
};


/* Try to find a dedicated optimization for `INSTANCEOF(<type>).<name>(argc...)' */
INTERN WUNUSED NONNULL((1, 2)) struct Dee_ccall_optimization const *DCALL
Dee_ccall_find_attr_optimization(DeeTypeObject *__restrict type,
                                 char const *name, Dee_vstackaddr_t argc) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(cca_optimizations); ++i) {
		struct Dee_ccall_optimization const *opts;
		size_t lo, hi;
		if (cca_optimizations[i].tccos_type != type)
			continue;
		lo   = 0;
		hi   = cca_optimizations[i].tccos_size;
		opts = cca_optimizations[i].tccos_opts;
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct Dee_ccall_optimization const *ent = &opts[mid];
			int cmp = strcmp(name, ent->tcco_name.n_attr);
			if (cmp < 0) {
				hi = mid;
			} else if (cmp > 0) {
				lo = mid + 1;
			} else {
				if (ent->tcco_argc != Dee_CCALL_ARGC_ANY && ent->tcco_argc != argc) {
					struct Dee_ccall_optimization const *end = opts + hi;
					while (ent > opts && strcmp(ent[-1].tcco_name.n_attr, name) == 0)
						--ent;
					for (;;) {
						if (ent >= end)
							goto no_dedicated_optimization;
						if (ent->tcco_argc == Dee_CCALL_ARGC_ANY)
							break;
						if (ent->tcco_argc == argc)
							break;
						++ent;
						if (strcmp(ent->tcco_name.n_attr, name) != 0)
							goto no_dedicated_optimization;
					}
				}
				return ent; /* There is an optimization for this case! */
			}
		}
		goto no_dedicated_optimization;
	}
no_dedicated_optimization:
	return NULL;
}

/* Try to find a dedicated optimization for `INSTANCEOF(<type>).operator <operator_name> (argc...)'
 * NOTE: Optimizations returned type this one may or may not push a result onto the stack,
 *       depending on the operator in question (`operator_name')! Because of this, if the
 *       operator is generic, the caller needs to check how the vstack depth is altered.
 *       For inplace operators, the same applies, but the "this" argument always remains
 *       on-stack as well! */
INTERN WUNUSED NONNULL((1)) struct Dee_ccall_optimization const *DCALL
Dee_ccall_find_operator_optimization(DeeTypeObject *__restrict type,
                                     uint16_t operator_name,
                                     Dee_vstackaddr_t argc) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(cco_optimizations); ++i) {
		struct Dee_ccall_optimization const *opts;
		size_t lo, hi;
		if (cco_optimizations[i].tccos_type != type)
			continue;
		lo   = 0;
		hi   = cco_optimizations[i].tccos_size;
		opts = cco_optimizations[i].tccos_opts;
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct Dee_ccall_optimization const *ent = &opts[mid];
			uint16_t ent_opname = (uint16_t)ent->tcco_name.n_opname;
			if (operator_name < ent_opname) {
				hi = mid;
			} else if (operator_name > ent_opname) {
				lo = mid + 1;
			} else {
				if (ent->tcco_argc != Dee_CCALL_ARGC_ANY && ent->tcco_argc != argc) {
					struct Dee_ccall_optimization const *end = opts + hi;
					while (ent > opts && ((uint16_t)ent[-1].tcco_name.n_opname == operator_name))
						--ent;
					for (;;) {
						if (ent >= end)
							goto no_dedicated_optimization;
						if (ent->tcco_argc == Dee_CCALL_ARGC_ANY)
							break;
						if (ent->tcco_argc == argc)
							break;
						++ent;
						if ((uint16_t)ent->tcco_name.n_opname != operator_name)
							goto no_dedicated_optimization;
					}
				}
				return ent; /* There is an optimization for this case! */
			}
		}
		goto no_dedicated_optimization;
	}
no_dedicated_optimization:
	return NULL;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_CCALL_C */
