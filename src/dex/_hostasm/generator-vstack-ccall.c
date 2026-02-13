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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_CCALL_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_CCALL_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/api.h>

#include <deemon/bool.h>            /* DeeBool_Type, Dee_False */
#include <deemon/bytes.h>           /* DeeBytesObject, DeeBytes_Type */
#include <deemon/cell.h>            /* DeeCell* */
#include <deemon/dict.h>            /* DeeDictObject, DeeDict_Type */
#include <deemon/error.h>           /* DeeError_UnboundAttribute, DeeError_ValueError */
#include <deemon/float.h>           /* CONFIG_HAVE_FPU, DeeFloat_Type */
#include <deemon/hashset.h>         /* DeeHashSetObject, DeeHashSet_Type */
#include <deemon/int.h>             /* DeeIntObject, DeeInt_Type, DeeInt_Zero */
#include <deemon/list.h>            /* DeeListObject, DeeList_* */
#include <deemon/map.h>             /* DeeMapping_Type */
#include <deemon/method-hints.h>    /* DeeMH_map_setdefault_t, DeeType_RequireMethodHint */
#include <deemon/none.h>            /* DeeNone_Type, Dee_None */
#include <deemon/object.h>          /* DeeObject_Type, DeeTypeObject */
#include <deemon/rodict.h>          /* DeeRoDictObject, DeeRoDict_Type */
#include <deemon/roset.h>           /* DeeRoSetObject, DeeRoSet_Type */
#include <deemon/seq.h>             /* DeeSeq_* */
#include <deemon/string.h>          /* DeeString* */
#include <deemon/system-features.h> /* CONFIG_HAVE_FPU, strcmp */
#include <deemon/tuple.h>           /* DeeTupleObject, DeeTuple_Type */
#include <deemon/type.h>            /* Dee_operator_t, OPERATOR_* */
#include <deemon/util/weakref.h>    /* Dee_weakref_bound, Dee_weakref_lock */
#include <deemon/weakref.h>         /* DeeWeakRefObject, DeeWeakRef_Type */

#include <stddef.h> /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h> /* uintptr_t */

DECL_BEGIN

/************************************************************************/
/* TYPE METHOD/GETSET SPECIFIC OPTIMIZATIONS                            */
/************************************************************************/

#define CCA_OPTIMIZATION(attr, func, argc)   { { attr }, func, argc }
#define CCO_OPTIMIZATION(opname, func, argc) { { (char const *)(uintptr_t)(opname) }, func, argc }

#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err

#define DEFINE_CCALL_OPTIMIZATION(name, impl)                 \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                    \
	name(struct fungen *__restrict self, vstackaddr_t argc) { \
		(void)argc;                                           \
		return impl;                                          \
	}
#define DEFINE_CCALL_OPERATOR(name, opname, argc) \
	DEFINE_CCALL_OPTIMIZATION(name, fg_vop(self, opname, argc, VOP_F_PUSHRES))


/* value -> bool
 * Implement a fast "operator bool" by checking if `*(void **)((byte_t *)value + offsetof_field) != NULL' */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vbool_field_nonzero(struct fungen *__restrict self,
                    ptrdiff_t offsetof_field) {
	DO(fg_vdup(self));                 /* value, value */
	DO(fg_vind(self, offsetof_field)); /* value, value->xx_field */
	DO(fg_vreg(self, NULL));           /* value, reg:value->xx_field */
	DO(fg_vdirect1(self));             /* value, reg:value->xx_field */
	DO(fg_vpop_at(self, 2));           /* reg:value->xx_field */
	return fg_vcall_DeeBool_For(self); /* result */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
vsize_field_uint(struct fungen *__restrict self,
                 ptrdiff_t offsetof_size_field) {
	DO(fg_vdup(self));                       /* value, value */
	DO(fg_vind(self, offsetof_size_field));  /* value, value->xx_size */
	DO(fg_vreg(self, NULL));                 /* value, reg:value->xx_size */
	DO(fg_vdirect1(self));                   /* value, reg:value->xx_size */
	DO(fg_vpop_at(self, 2));                 /* reg:value->xx_size */
	return fg_vcall_DeeInt_NewUIntptr(self); /* result */
err:
	return -1;
}




/************************************************************************/
/* Object                                                               */
/************************************************************************/
DEFINE_CCALL_OPERATOR(cca_Object___copy__, OPERATOR_COPY, 1)
DEFINE_CCALL_OPERATOR(cca_Object___assign__, OPERATOR_ASSIGN, 2)
DEFINE_CCALL_OPERATOR(cca_Object___moveassign__, OPERATOR_MOVEASSIGN, 2)
DEFINE_CCALL_OPTIMIZATION(cca_Object___str__, fg_vopstr(self)) /* 1 */
DEFINE_CCALL_OPERATOR(cca_Object___repr__, OPERATOR_REPR, 1)
DEFINE_CCALL_OPTIMIZATION(cca_Object___bool__, fg_vopbool(self, VOPBOOL_F_NORMAL)) /* 1 */
DEFINE_CCALL_OPTIMIZATION(cca_Object___call__,
                          fg_vcall_DeeObject_AssertTypeExact_c(self, &DeeTuple_Type) ||
                          fg_vopcalltuple(self)) /* 2 */
DEFINE_CCALL_OPTIMIZATION(cca_Object___thiscall__,
                          fg_vcall_DeeObject_AssertTypeExact_c(self, &DeeTuple_Type) ||
                          fg_vopthiscalltuple(self)) /* 3 */
DEFINE_CCALL_OPERATOR(cca_Object___hash__, OPERATOR_HASH, 1)
DEFINE_CCALL_OPTIMIZATION(cca_Object___int__, fg_vopint(self)) /* 1 */
DEFINE_CCALL_OPERATOR(cca_Object___inv__, OPERATOR_INV, 1)
DEFINE_CCALL_OPERATOR(cca_Object___pos__, OPERATOR_POS, 1)
DEFINE_CCALL_OPERATOR(cca_Object___neg__, OPERATOR_NEG, 1)
DEFINE_CCALL_OPERATOR(cca_Object___add__, OPERATOR_ADD, 2)
DEFINE_CCALL_OPERATOR(cca_Object___sub__, OPERATOR_SUB, 2)
DEFINE_CCALL_OPERATOR(cca_Object___mul__, OPERATOR_MUL, 2)
DEFINE_CCALL_OPERATOR(cca_Object___div__, OPERATOR_DIV, 2)
DEFINE_CCALL_OPERATOR(cca_Object___mod__, OPERATOR_MOD, 2)
DEFINE_CCALL_OPERATOR(cca_Object___shl__, OPERATOR_SHL, 2)
DEFINE_CCALL_OPERATOR(cca_Object___shr__, OPERATOR_SHR, 2)
DEFINE_CCALL_OPERATOR(cca_Object___and__, OPERATOR_AND, 2)
DEFINE_CCALL_OPERATOR(cca_Object___or__, OPERATOR_OR, 2)
DEFINE_CCALL_OPERATOR(cca_Object___xor__, OPERATOR_XOR, 2)
DEFINE_CCALL_OPERATOR(cca_Object___pow__, OPERATOR_POW, 2)
DEFINE_CCALL_OPERATOR(cca_Object___eq__, OPERATOR_EQ, 2)
DEFINE_CCALL_OPERATOR(cca_Object___ne__, OPERATOR_NE, 2)
DEFINE_CCALL_OPERATOR(cca_Object___lo__, OPERATOR_LO, 2)
DEFINE_CCALL_OPERATOR(cca_Object___le__, OPERATOR_LE, 2)
DEFINE_CCALL_OPERATOR(cca_Object___gr__, OPERATOR_GR, 2)
DEFINE_CCALL_OPERATOR(cca_Object___ge__, OPERATOR_GE, 2)
DEFINE_CCALL_OPTIMIZATION(cca_Object___size__, fg_vopsize(self)) /* 1 */
DEFINE_CCALL_OPERATOR(cca_Object___contains__, OPERATOR_CONTAINS, 2)
DEFINE_CCALL_OPERATOR(cca_Object___getitem__, OPERATOR_GETITEM, 2)
DEFINE_CCALL_OPERATOR(cca_Object___delitem__, OPERATOR_DELITEM, 2)
DEFINE_CCALL_OPERATOR(cca_Object___setitem__, OPERATOR_SETITEM, 3)
DEFINE_CCALL_OPERATOR(cca_Object___getrange__, OPERATOR_GETRANGE, 3)
DEFINE_CCALL_OPERATOR(cca_Object___delrange__, OPERATOR_DELRANGE, 3)
DEFINE_CCALL_OPERATOR(cca_Object___setrange__, OPERATOR_SETRANGE, 4)
DEFINE_CCALL_OPERATOR(cca_Object___iterself__, OPERATOR_ITER, 1)
DEFINE_CCALL_OPERATOR(cca_Object___iternext__, OPERATOR_ITERNEXT, 1)
DEFINE_CCALL_OPTIMIZATION(cca_Object___getattr__, fg_vopgetattr(self)) /* 2 */
DEFINE_CCALL_OPTIMIZATION(cca_Object___callattr__,
                          fg_vcall_DeeObject_AssertTypeExact_c(self, &DeeTuple_Type) ||
                          fg_vopcallattrtuple(self))                   /* 3 */
DEFINE_CCALL_OPTIMIZATION(cca_Object___hasattr__, fg_vophasattr(self)) /* 2 */
DEFINE_CCALL_OPTIMIZATION(cca_Object___delattr__, fg_vopdelattr(self)) /* 2 */
DEFINE_CCALL_OPTIMIZATION(cca_Object___setattr__, fg_vopsetattr(self)) /* 3 */

PRIVATE struct ccall_optimization tpconst cca_Object[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("__add__", &cca_Object___add__, 1),
	CCA_OPTIMIZATION("__and__", &cca_Object___and__, 1),
	CCA_OPTIMIZATION("__assign__", &cca_Object___assign__, 1),
	CCA_OPTIMIZATION("__bool__", &cca_Object___bool__, 0),
	CCA_OPTIMIZATION("__call__", &cca_Object___call__, 1),
	CCA_OPTIMIZATION("__callattr__", &cca_Object___callattr__, 2),
	CCA_OPTIMIZATION("__contains__", &cca_Object___contains__, 1),
	CCA_OPTIMIZATION("__copy__", &cca_Object___copy__, 0),
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

/* seq -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_sum(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1));
	return fg_vcallapi(self, &DeeSeq_Sum, VCALL_CC_OBJECT, 1); /* result */
err:
	return -1;
}

/* seq -> result */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
impl_cca_Sequence_anyall(struct fungen *__restrict self,
                         void const *api_function) {
	DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1));
	DO(fg_vcallapi(self, api_function, VCALL_CC_NEGINT, 1));
	DO(fg_vdirect1(self));
	ASSERT(fg_vtop_isdirect(self));
	fg_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* seq -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_any(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return impl_cca_Sequence_anyall(self, (void const *)&DeeSeq_Any);
}

/* seq -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_all(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return impl_cca_Sequence_anyall(self, (void const *)&DeeSeq_All);
}


/* seq, [key] -> result */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
impl_cca_Sequence_minmax(struct fungen *__restrict self,
                         vstackaddr_t argc, void const *api_function) {
	if (argc >= 1) {
		DO(fg_vcoalesce_c(self, Dee_None, NULL)); /* seq, key */
	} else {
		DO(fg_vpush_NULL(self)); /* seq, key */
	}
	DO(fg_vnotoneref_at(self, 1));
	DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 2));
	return fg_vcallapi(self, api_function, VCALL_CC_OBJECT, 2);
err:
	return -1;
}

/* seq -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_min(struct fungen *__restrict self, vstackaddr_t argc) {
	return impl_cca_Sequence_minmax(self, argc, (void const *)&DeeSeq_Min);
}

/* seq -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Sequence_max(struct fungen *__restrict self, vstackaddr_t argc) {
	return impl_cca_Sequence_minmax(self, argc, (void const *)&DeeSeq_Max);
}

PRIVATE struct ccall_optimization tpconst cca_Sequence[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("all", &cca_Sequence_all, 0),
	CCA_OPTIMIZATION("any", &cca_Sequence_any, 0),
	CCA_OPTIMIZATION("max", &cca_Sequence_max, 0),
	CCA_OPTIMIZATION("min", &cca_Sequence_min, 0),
	CCA_OPTIMIZATION("sum", &cca_Sequence_sum, 0),
	/* TODO: When types are known, we can pretty much fully inline stuff like "Sequence.insert()",
	 *       assuming that NSI operators have been defined. */
};




/************************************************************************/
/* Mapping                                                             */
/************************************************************************/

/* map, key, [def] -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Mapping_get(struct fungen *__restrict self, vstackaddr_t argc) {
	if (argc < 2)
		DO(fg_vpush_none(self));
	return fg_vopgetitemdef(self);
err:
	return -1;
}

/* map, key, [value] -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Mapping_setdefault(struct fungen *__restrict self, vstackaddr_t argc) {
	DeeMH_map_setdefault_t mh_map_setdefault;
	DeeTypeObject *map_type = memval_typeof(fg_vtop(self) - argc);
	if unlikely(!map_type)
		return 1; /* Shouldn't happen since we only get called when types are known... */
	if (argc < 2)
		DO(fg_vpush_none(self)); /* map, key, value */
	mh_map_setdefault = DeeType_RequireMethodHint(map_type, map_setdefault);
	DO(fg_vnotoneref(self, 2));                                  /* this, key, value */
	DO(fg_vnotoneref_if_operator_at(self, OPERATOR_SETITEM, 2)); /* this, key, value */
	return fg_vcallapi(self, mh_map_setdefault, VCALL_CC_OBJECT, 3);
err:
	return -1;
}

PRIVATE struct ccall_optimization tpconst cca_Mapping[] = {
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

/* this, [args...] -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_append(struct fungen *__restrict self, vstackaddr_t argc) {
	DO(fg_vnotoneref(self, argc)); /* this, [args...] */
	while (argc) {
		/* XXX: Pre-reserve memory when multiple arguments are given? */
		/* XXX: Use a different function `DeeList_AppendInherted()', so we don't have to decref the appended object? */
		DO(fg_vdup_at(self, argc + 1));                          /* this, [args...], this */
		DO(fg_vlrot(self, argc + 1));                            /* this, [moreargs...], this, arg */
		DO(fg_vcallapi(self, &DeeList_Append, VCALL_CC_INT, 2)); /* this, [moreargs...] */
		--argc;
	}
	DO(fg_vpop(self));                     /* N/A */
	return fg_vpush_none(self); /* none */
err:
	return -1;
}

/* this, seq -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_extend(struct fungen *__restrict self, vstackaddr_t argc) {
	DeeTypeObject *seqtype;
	(void)argc;
	seqtype = fg_vtoptype(self);
	if (seqtype == &DeeTuple_Type) {
		DO(fg_vdup(self));                                     /* this, seq, seq */
		DO(fg_vrrot(self, 3));                                 /* seq, this, seq */
		DO(fg_vdup(self));                                     /* seq, this, seq, seq */
		DO(fg_vind(self, offsetof(DeeTupleObject, t_size)));   /* seq, this, seq, seq->t_size */
		DO(fg_vswap(self));                                    /* seq, this, seq->t_size, seq */
		DO(fg_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* seq, this, seq->t_size, seq->t_elem */
		DO(fg_vcallapi(self, &DeeList_AppendVector, VCALL_CC_INT, 3)); /* seq */
		DO(fg_vpop(self));                                     /* N/A */
	} else {
		DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1)); /* this, seq */
		DO(fg_vcallapi(self, &DeeList_AppendSequence, VCALL_CC_INT, 2)); /* N/A */
	}
	return fg_vpush_none(self); /* none */
err:
	return -1;
}

/* this, index, item -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_insert(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	DO(fg_vswap(self));            /* this, item, index */
	DO(fg_vmorph_uint(self));      /* this, item, uint:index */
	DO(fg_vswap(self));            /* this, uint:index, item */
	DO(fg_vnotoneref_at(self, 1)); /* this, uint:index, item */
	DO(fg_vcallapi(self, &DeeList_Insert, VCALL_CC_INT, 3)); /* N/A */
	return fg_vpush_none(self); /* none */
err:
	return -1;
}

/* this, index, seq -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_insertall(struct fungen *__restrict self, vstackaddr_t argc) {
	DeeTypeObject *seqtype = fg_vtoptype(self);
	(void)argc;
	DO(fg_vswap(self));       /* this, seq, index */
	DO(fg_vmorph_uint(self)); /* this, seq, uint:index */
	DO(fg_vswap(self));       /* this, uint:index, seq */
	if (seqtype == &DeeTuple_Type) {
		DO(fg_vdup(self));                                     /* this, uint:index, seq, seq */
		DO(fg_vrrot(self, 4));                                 /* seq, this, uint:index, seq */
		DO(fg_vdup(self));                                     /* seq, this, uint:index, seq, seq */
		DO(fg_vind(self, offsetof(DeeTupleObject, t_size)));   /* seq, this, uint:index, seq, seq->t_size */
		DO(fg_vswap(self));                                    /* seq, this, uint:index, seq->t_size, seq */
		DO(fg_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* seq, this, uint:index, seq->t_size, seq->t_elem */
		DO(fg_vcallapi(self, &DeeList_InsertVector, VCALL_CC_INT, 4)); /* seq */
		DO(fg_vpop(self));                                     /* N/A */
	} else {
		DO(fg_vnotoneref_if_operator_at(self, OPERATOR_ITER, 1)); /* this, uint:index, seq */
		DO(fg_vcallapi(self, &DeeList_InsertSequence, VCALL_CC_INT, 3)); /* N/A */
	}
	return fg_vpush_none(self); /* none */
err:
	return -1;
}

/* this, should, [start, [end]] -> count */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_removeif(struct fungen *__restrict self, vstackaddr_t argc) {
	if (argc >= 3) {
		DO(fg_vmorph_uint(self)); /* this, should, start, uint:end */
		DO(fg_vswap(self));       /* this, should, uint:end, start */
		DO(fg_vmorph_uint(self)); /* this, should, uint:end, uint:start */
		DO(fg_vswap(self));       /* this, should, uint:start, uint:end */
	} else if (argc >= 2) {
		DO(fg_vmorph_uint(self));              /* this, should, uint:start */
		DO(fg_vpush_immSIZ(self, (size_t)-1)); /* this, should, uint:start, uint:end */
	} else {
		DO(fg_vpush_immSIZ(self, (size_t)-1)); /* this, should, uint:start */
		DO(fg_vpush_immSIZ(self, (size_t)-1)); /* this, should, uint:start, uint:end */
	}
	DO(fg_vlrot(self, 3));      /* this, uint:start, uint:end, should */
	DO(fg_vnotoneref_if_operator(self, OPERATOR_CALL, 1)); /* this, uint:start, uint:end, should */
	DO(fg_vcallapi(self, &DeeList_RemoveIf, VCALL_CC_M1INTPTR, 4)); /* result */
	fg_vtop(self)->mv_vmorph = MEMVAL_VMORPH_UINT;
	return 0;
err:
	return -1;
}


/* this, index, [count] -> count */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_erase(struct fungen *__restrict self, vstackaddr_t argc) {
	if (argc >= 2) {
		DO(fg_vmorph_uint(self)); /* this, index, uint:count */
		DO(fg_vswap(self));       /* this, uint:count, index */
		DO(fg_vmorph_uint(self)); /* this, uint:count, uint:index */
		DO(fg_vswap(self));       /* this, uint:index, uint:count */
	} else {
		DO(fg_vmorph_uint(self));     /* this, uint:index */
		DO(fg_vpush_immSIZ(self, 1)); /* this, uint:index, uint:count */
	}
	DO(fg_vcallapi(self, &DeeList_Erase, VCALL_CC_M1INTPTR, 3)); /* result */
	fg_vtop(self)->mv_vmorph = MEMVAL_VMORPH_UINT;
	return 0;
err:
	return -1;
}

/* this -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_clear(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	DO(fg_vcallapi(self, &DeeList_Clear, VCALL_CC_VOID, 1)); /* N/A */
	return fg_vpush_none(self); /* none */
err:
	return -1;
}

/* this, [index] -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_pop(struct fungen *__restrict self, vstackaddr_t argc) {
	if (argc >= 1) {
		DO(fg_vmorph_int(self)); /* this, int:index */
	} else {
		DO(fg_vpush_immSIZ(self, -1)); /* this, int:index */
	}
	return fg_vcallapi(self, &DeeList_Pop, VCALL_CC_OBJECT, 2); /* result */
err:
	return -1;
}

/* this, [key] -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_sort(struct fungen *__restrict self, vstackaddr_t argc) {
	if (argc >= 1) {
		DeeTypeObject *known_type;
		known_type = fg_vtoptype(self);
		if (known_type == &DeeNone_Type) {
			DO(fg_vpop(self)); /* this */
			goto use_null_key;
		} else if (known_type != NULL) {
			/* ... */
		} else {
			DO(fg_vcoalesce_c(self, Dee_None, NULL)); /* this, key */
		}
		DO(fg_vnotoneref_if_operator(self, OPERATOR_CALL, 1)); /* this, key */
	} else {
use_null_key:
		DO(fg_vpush_addr(self, NULL)); /* this, key */
	}
	DO(fg_vcallapi(self, &DeeList_Sort, VCALL_CC_INT, 2)); /* N/A */
	return fg_vpush_none(self); /* none */
err:
	return -1;
}

/* this, newsize, [filler] -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_List_resize(struct fungen *__restrict self, vstackaddr_t argc) {
	if (argc >= 2) {
		DO(fg_vswap(self));       /* this, filler, newsize */
		DO(fg_vmorph_uint(self)); /* this, filler, uint:newsize */
		DO(fg_vswap(self));       /* this, uint:newsize, filler */
	} else {
		DO(fg_vmorph_uint(self)); /* this, uint:newsize */
		DO(fg_vpush_none(self));  /* this, uint:newsize, filler */
	}
	DO(fg_vcallapi(self, &DeeList_Resize, VCALL_CC_INT, 3)); /* N/A */
	return fg_vpush_none(self); /* none */
err:
	return -1;
}

PRIVATE struct ccall_optimization tpconst cca_List[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("append", &cca_List_append, CCALL_ARGC_ANY),
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
cco_List_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeListObject, l_list.ol_elemc));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_List_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeListObject, l_list.ol_elemc));
}

PRIVATE struct ccall_optimization tpconst cco_List[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_List_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_List_size, 0),
};




/************************************************************************/
/* String                                                               */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_String_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeStringObject, s_len));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_String_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeStringObject, s_len));
}

/* this, other -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_String_add(struct fungen *__restrict self, vstackaddr_t argc) {
	struct memval *v_this = fg_vtop(self) - 1;
	(void)argc;
	if (memval_isconst(v_this) && DeeString_IsEmpty(memval_const_getobj(v_this))) {
		/* Special case: `"" + foo' same as `str foo' */
		DO(fg_vpop_at(self, 2)); /* other */
		return fg_vopstr(self);  /* result */
	}
	return 1;
err:
	return -1;
}

PRIVATE struct ccall_optimization tpconst cco_String[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_String_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0010_ADD, &cco_String_add, 1),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_String_size, 0),
};




/************************************************************************/
/* Bytes                                                                */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Bytes_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeBytesObject, b_size));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Bytes_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeBytesObject, b_size));
}

PRIVATE struct ccall_optimization tpconst cco_Bytes[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Bytes_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_Bytes_size, 0),
};




/************************************************************************/
/* Tuple                                                                */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Tuple_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeTupleObject, t_size));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Tuple_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeTupleObject, t_size));
}

PRIVATE struct ccall_optimization tpconst cco_Tuple[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Tuple_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_Tuple_size, 0),
};




/************************************************************************/
/* HashSet                                                              */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_HashSet_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeHashSetObject, hs_used));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_HashSet_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeHashSetObject, hs_used));
}

PRIVATE struct ccall_optimization tpconst cco_HashSet[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_HashSet_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_HashSet_size, 0),
};




/************************************************************************/
/* RoSet                                                                */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoSet_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeRoSetObject, rs_size));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoSet_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeRoSetObject, rs_size));
}

PRIVATE struct ccall_optimization tpconst cco_RoSet[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_RoSet_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_RoSet_size, 0),
};




/************************************************************************/
/* Dict                                                                 */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Dict_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeDictObject, d_vused));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Dict_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeDictObject, d_vused));
}

PRIVATE struct ccall_optimization tpconst cco_Dict[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Dict_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_Dict_size, 0),
};




/************************************************************************/
/* RoDict                                                               */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoDict_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeRoDictObject, rd_vsize));
}

/* this -> size */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_RoDict_size(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vsize_field_uint(self, offsetof(DeeRoDictObject, rd_vsize));
}

PRIVATE struct ccall_optimization tpconst cco_RoDict[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_RoDict_bool, 0),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_RoDict_size, 0),
};




/************************************************************************/
/* Int                                                                  */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Int_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeIntObject, ob_size));
}

PRIVATE struct ccall_optimization tpconst cco_Int[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Int_bool, 0),
};




/************************************************************************/
/* WeakRef                                                              */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_WeakRef_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	DO(fg_vdup(self));                                              /* wr, &wr->wr_ref */
	DO(fg_vdelta(self, offsetof(DeeWeakRefObject, wr_ref)));        /* wr, &wr->wr_ref */
	DO(fg_vcallapi(self, &Dee_weakref_bound, VCALL_CC_BOOL_NX, 1)); /* wr, result */
	return fg_vpop_at(self, 2);                                     /* result */
err:
	return -1;
}

/* this -> value */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_WeakRef_get(struct fungen *__restrict self, vstackaddr_t argc) {
	struct fg_branch branch;
	(void)argc;
	DO(fg_vdup(self));                              /* wr, &wr->wr_ref */
	DO(fg_vdelta(self, offsetof(DeeWeakRefObject, wr_ref)));         /* wr, &wr->wr_ref */
	DO(fg_vcallapi(self, &Dee_weakref_lock, VCALL_CC_RAWINTPTR, 1)); /* wr, nullable:result */
	DO(fg_vpop_at(self, 2));                        /* nullable:result */
	DO(fg_vdup(self));                              /* nullable:result, nullable:result */
	DO(fg_vjz_enter_unlikely(self, &branch));       /* nullable:result */
	EDO(err_branch, fg_vcallapi(self, &libhostasm_rt_err_cannot_lock_weakref, VCALL_CC_EXCEPT, 0)); /* nullable:result */
	DO(fg_vjx_leave_noreturn(self, &branch));       /* nullable:result */
	DO(fg_gincref_loc(self, fg_vtopdloc(self), 1)); /* result */
	fg_vtop_direct_setref(self);                    /* ref:result */
	return 0;
err_branch:
	fg_branch_fini(&branch);
err:
	return -1;
}

PRIVATE struct ccall_optimization tpconst cca_WeakRef[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("alive", &cco_WeakRef_bool, CCALL_ARGC_GETTER),
	CCA_OPTIMIZATION("value", &cco_WeakRef_get, CCALL_ARGC_GETTER),
	CCA_OPTIMIZATION("value", &cco_WeakRef_bool, CCALL_ARGC_BOUND),
};

PRIVATE struct ccall_optimization tpconst cco_WeakRef[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_WeakRef_bool, 0),
};




/************************************************************************/
/* Cell                                                                 */
/************************************************************************/

/* cell -> value */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_DeeCell_Get_or_Error(struct fungen *__restrict self,
                           DeeTypeObject *error_type) {
	struct fg_branch branch;
	void const *except_api;
	ASSERT(error_type == &DeeError_ValueError ||
	       error_type == &DeeError_UnboundAttribute);
	except_api = (void const *)&libhostasm_rt_err_cell_empty_ValueError;
	if (error_type == &DeeError_UnboundAttribute)
		except_api = (void const *)&libhostasm_rt_err_cell_empty_UnboundAttribute;
	DO(fg_vrwlock_read_field(self, offsetof(DeeCellObject, c_lock))); /* cell */
	DO(fg_vdup(self));                              /* cell, cell */
	DO(fg_vind(self, offsetof(DeeCellObject, c_item))); /* cell, cell->c_item */
	DO(fg_vreg(self, NULL));                        /* cell, reg:cell->c_item */
	DO(fg_vdirect1(self));                          /* cell, reg:cell->c_item */
	DO(fg_vdup(self));                              /* cell, reg:cell->c_item, reg:cell->c_item */
	DO(fg_vjz_enter_unlikely(self, &branch));       /* cell, reg:cell->c_item */
	EDO(err_branch, fg_vswap(self));                /* cell->c_item, cell */
#ifndef CONFIG_NO_THREADS
	EDO(err_branch, fg_vdelta(self, offsetof(DeeCellObject, c_lock))); /* cell->c_item, cell, &cell->c_lock */
	EDO(err_branch, fg_vrwlock_endread(self));      /* cell->c_item */
#endif /* !CONFIG_NO_THREADS */
	EDO(err_branch, fg_vcallapi(self, except_api, VCALL_CC_EXCEPT, 0)); /* cell->c_item, [cell] */
	DO(fg_vjx_leave_noreturn(self, &branch));       /* cell, reg:cell->c_item */
	DO(fg_gincref_loc(self, fg_vtopdloc(self), 1)); /* cell, ref:cell->c_item */
	fg_vtop_direct_setref(self);                    /* cell, ref:cell->c_item */
	DO(fg_vswap(self));                             /* ref:cell->c_item, cell */
	DO(fg_vrwlock_endread_field(self, offsetof(DeeCellObject, c_lock))); /* ref:cell->c_item, cell */
	return fg_vpop(self);                           /* ref:cell->c_item */
err_branch:
	fg_branch_fini(&branch);
err:
	return -1;
}

/* cell, ref:value -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vcall_DeeCell_DelOrSet(struct fungen *__restrict self) {
#ifndef CONFIG_NO_THREADS
	DO(fg_vdup_at(self, 2));             /* cell, ref:value, cell */
	DO(fg_vdelta(self, offsetof(DeeCellObject, c_lock))); /* cell, ref:value, &cell->c_lock */
	DO(fg_vrwlock_write(self));          /* cell, ref:value */
#endif /* !CONFIG_NO_THREADS */
	DO(fg_vdup_at(self, 2));             /* cell, ref:value, cell */
	DO(fg_vswap(self));                  /* cell, cell, ref:value */
	DO(fg_vswapind(self, offsetof(DeeCellObject, c_item))); /* cell, ref:old_value */
#ifndef CONFIG_NO_THREADS
	DO(fg_vdup_at(self, 2));             /* cell, ref:old_value, cell */
	DO(fg_vdelta(self, offsetof(DeeCellObject, c_lock))); /* cell, ref:old_value, &cell->c_lock */
	DO(fg_vrwlock_endwrite(self));       /* cell, ref:old_value */
#endif /* !CONFIG_NO_THREADS */
	DO(fg_vpop_at(self, 2));             /* ref:old_value */
	ASSERT(!fg_vtop_direct_isref(self)); /* ref:old_value */
	DO(fg_gxdecref_loc(self, fg_vtopdloc(self), 1)); /* old_value */
	return fg_vpop(self);
err:
	return -1;
}

/* this, [def] -> value */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_get(struct fungen *__restrict self, vstackaddr_t argc) {
	struct fg_branch branch;
	if (argc == 0)
		return vcall_DeeCell_Get_or_Error(self, &DeeError_ValueError);
	/**/                                                           /* this, def */
	DO(fg_vswap(self));                                            /* def, this */
	/* TODO: Inline this call to DeeCell_TryGet() */
	DO(fg_vcallapi(self, &DeeCell_TryGet, VCALL_CC_RAWINTPTR, 1)); /* def, result */
	fg_vtop_direct_setref(self);                                   /* def, ref:result */
	DO(fg_vdup(self));                                             /* def, ref:result, result */
	DO(fg_vjz_enter(self, &branch));                               /* def, ref:result */
	EDO(err_branch, fg_state_unshare(self));                       /* def, ref:result */
	fg_vtop_direct_clearref(self);                                 /* def, result */
	EDO(err_branch, fg_vpop(self));                                /* def */
	EDO(err_branch, fg_vdup(self));                                /* def, def */
	EDO(err_branch, fg_vdirect1(self));                            /* def, def */
	EDO(err_branch, fg_vref_noalias(self));                        /* def, ref:def */
	DO(fg_vjx_leave(self, &branch));                               /* def, result */
	return 0;
err_branch:
	fg_branch_fini(&branch);
err:
	return -1;
}

/* this -> value */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_value_getter(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)
		return fg_vcallapi(self, &DeeCell_Get, VCALL_CC_OBJECT, 1);
	return vcall_DeeCell_Get_or_Error(self, &DeeError_UnboundAttribute);
}

/* this -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_value_delete(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)
		return fg_vcallapi(self, &DeeCell_Del, VCALL_CC_VOID, 1);
	DO(fg_vpush_NULL(self)); /* cell, NULL */
	return vcall_DeeCell_DelOrSet(self);
err:
	return -1;
}

/* this, value -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cca_Cell_value_setter(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)
		return fg_vcallapi(self, &DeeCell_Set, VCALL_CC_VOID, 2);
	DO(fg_vnotoneref_at(self, 1)); /* cell, value */
	DO(fg_vref2(self, 2));         /* cell, ref:value */
	return vcall_DeeCell_DelOrSet(self);
err:
	return -1;
}

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Cell_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return vbool_field_nonzero(self, offsetof(DeeCellObject, c_item));
}

PRIVATE struct ccall_optimization tpconst cca_Cell[] = {
	/* IMPORTANT: Keep sorted! */
	CCA_OPTIMIZATION("get", &cca_Cell_get, 0),
	CCA_OPTIMIZATION("get", &cca_Cell_get, 1),
	CCA_OPTIMIZATION("value", &cca_Cell_value_delete, CCALL_ARGC_DELETE),
	CCA_OPTIMIZATION("value", &cca_Cell_value_getter, CCALL_ARGC_GETTER),
	CCA_OPTIMIZATION("value", &cca_Cell_value_setter, CCALL_ARGC_SETTER),
	CCA_OPTIMIZATION("value", &cco_Cell_bool, CCALL_ARGC_BOUND),
};

PRIVATE struct ccall_optimization tpconst cco_Cell[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Cell_bool, 0),
};




/************************************************************************/
/* None                                                                 */
/************************************************************************/

/* this, [args...] -> none */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_none(struct fungen *__restrict self, vstackaddr_t argc) {
	int result = fg_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = fg_vpush_none(self);
	return result;
}

/* this, [args...] -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_popall(struct fungen *__restrict self, vstackaddr_t argc) {
	int result = fg_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = fg_vpush_none(self);
	return result;
}

#define str_none (*COMPILER_CONTAINER_OF(DeeNone_Type.tp_name, DeeStringObject, s_str))
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_str_none(struct fungen *__restrict self, vstackaddr_t argc) {
	int result = fg_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = fg_vpush_const(self, &str_none);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_false(struct fungen *__restrict self, vstackaddr_t argc) {
	int result = fg_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = fg_vpush_const(self, Dee_False);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_return_zero(struct fungen *__restrict self, vstackaddr_t argc) {
	int result = fg_vpopmany(self, argc + 1);
	if likely(result == 0)
		result = fg_vpush_const(self, DeeInt_Zero);
	return result;
}

/* none, other -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_cmp_eq(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;                                                 /* none, other */
	DO(fg_vpop_at(self, 2));                /* other */
	return fg_veqconstaddr(self, Dee_None); /* other==Dee_None */
err:
	return -1;
}

/* none, other -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_None_cmp_ne(struct fungen *__restrict self, vstackaddr_t argc) {
	int result = cco_None_cmp_eq(self, argc);
	if likely(result == 0)
		result = fg_vopnot(self);
	return result;
}

PRIVATE struct ccall_optimization tpconst cco_None[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0001_COPY, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0004_ASSIGN, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0005_MOVEASSIGN, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0006_STR, &cco_None_return_str_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0007_REPR, &cco_None_return_str_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_None_return_false, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000A_CALL, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000B_INT, &cco_None_return_zero, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000D_INV, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000E_POS, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_000F_NEG, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0010_ADD, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0011_SUB, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0012_MUL, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0013_DIV, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0014_MOD, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0015_SHL, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0016_SHR, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0017_AND, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0018_OR, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0019_XOR, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001A_POW, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001B_INC, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001C_DEC, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001D_INPLACE_ADD, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001E_INPLACE_SUB, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_001F_INPLACE_MUL, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0020_INPLACE_DIV, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0021_INPLACE_MOD, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0022_INPLACE_SHL, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0023_INPLACE_SHR, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0024_INPLACE_AND, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0025_INPLACE_OR, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0026_INPLACE_XOR, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0027_INPLACE_POW, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0028_HASH, &cco_None_return_zero, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0029_EQ, &cco_None_cmp_eq, 1),
	CCO_OPTIMIZATION(OPERATOR_002A_NE, &cco_None_cmp_ne, 1),
	CCO_OPTIMIZATION(OPERATOR_002B_LO, &cco_None_cmp_ne, 1),
	CCO_OPTIMIZATION(OPERATOR_002C_LE, &cco_None_cmp_eq, 1),
	CCO_OPTIMIZATION(OPERATOR_002D_GR, &cco_None_cmp_ne, 1),
	CCO_OPTIMIZATION(OPERATOR_002E_GE, &cco_None_cmp_eq, 1),
	CCO_OPTIMIZATION(OPERATOR_002F_ITER, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0030_SIZE, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0031_CONTAINS, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0032_GETITEM, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0033_DELITEM, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0034_SETITEM, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0035_GETRANGE, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0036_DELRANGE, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0037_SETRANGE, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0038_GETATTR, &cco_None_return_none, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_0039_DELATTR, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_003A_SETATTR, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_003C_ENTER, &cco_None_popall, CCALL_ARGC_ANY),
	CCO_OPTIMIZATION(OPERATOR_003D_LEAVE, &cco_None_popall, CCALL_ARGC_ANY),
};




/************************************************************************/
/* Bool                                                                 */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Bool_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	/* Already a boolean, so nothing to do here! */
	(void)self;
	(void)argc;
	return 0;
}

PRIVATE struct ccall_optimization tpconst cco_Bool[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Bool_bool, 0),
};




#ifdef CONFIG_HAVE_FPU
/************************************************************************/
/* Float                                                                 */
/************************************************************************/

/* this -> bool */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cco_Float_bool(struct fungen *__restrict self, vstackaddr_t argc) {
	(void)argc;
	return fg_vcallapi(self, DeeFloat_Type.tp_cast.tp_bool, VCALL_CC_BOOL_NX, 1); /* result */
}

PRIVATE struct ccall_optimization tpconst cco_Float[] = {
	/* IMPORTANT: Keep sorted! */
	CCO_OPTIMIZATION(OPERATOR_0008_BOOL, &cco_Float_bool, 0),
};
#endif /* CONFIG_HAVE_FPU */




#undef DEFINE_CCALL_OPTIMIZATION
#undef DEFINE_CCALL_OPTIMIZATION
#undef DEFINE_CCALL_OPERATOR

struct ccall_optimizations_struct {
	DeeTypeObject             const *tccos_type; /* [1..1] The type to which these optimizations apply. */
	struct ccall_optimization const *tccos_opts; /* [0..tccos_size] Vector of optimization handlers (sorted lexicographically by `tcco_name'). */
	size_t                           tccos_size; /* [1..1][const] # of optimization handlers. */
};

/* Attribute optimizations for known types. */
PRIVATE struct ccall_optimizations_struct tpconst cca_optimizations[] = {
#define DEFINE_CCALL_OPTIMIZATIIONS(type, vec) { type, vec, COMPILER_LENOF(vec) }
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeObject_Type, cca_Object),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeSeq_Type, cca_Sequence),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeList_Type, cca_List),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeCell_Type, cca_Cell),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeMapping_Type, cca_Mapping),
#undef DEFINE_CCALL_OPTIMIZATIIONS
};


/* Operator optimizations for known types. */
PRIVATE struct ccall_optimizations_struct tpconst cco_optimizations[] = {
#define DEFINE_CCALL_OPTIMIZATIIONS(type, vec) { type, vec, COMPILER_LENOF(vec) }
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeInt_Type, cco_Int),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeString_Type, cco_String),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeBytes_Type, cco_Bytes),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeTuple_Type, cco_Tuple),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeList_Type, cco_List),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeHashSet_Type, cco_HashSet),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeRoSet_Type, cco_RoSet),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeDict_Type, cco_Dict),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeRoDict_Type, cco_RoDict),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeCell_Type, cco_Cell),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeWeakRef_Type, cco_WeakRef),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeNone_Type, cco_None),
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeBool_Type, cco_Bool),
#ifdef CONFIG_HAVE_FPU
	DEFINE_CCALL_OPTIMIZATIIONS(&DeeFloat_Type, cco_Float),
#endif /* CONFIG_HAVE_FPU */
#undef DEFINE_CCALL_OPTIMIZATIIONS
};


/* Try to find a dedicated optimization for `INSTANCEOF(<type>).<name>(argc...)' */
INTERN WUNUSED NONNULL((1, 2)) struct ccall_optimization const *DCALL
ccall_find_attr_optimization(DeeTypeObject *__restrict type,
                             char const *name, vstackaddr_t argc) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(cca_optimizations); ++i) {
		struct ccall_optimization const *opts;
		size_t lo, hi;
		if (cca_optimizations[i].tccos_type != type)
			continue;
		lo   = 0;
		hi   = cca_optimizations[i].tccos_size;
		opts = cca_optimizations[i].tccos_opts;
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct ccall_optimization const *ent = &opts[mid];
			int cmp = strcmp(name, ent->tcco_name.n_attr);
			if (cmp < 0) {
				hi = mid;
			} else if (cmp > 0) {
				lo = mid + 1;
			} else {
				if (ent->tcco_argc != CCALL_ARGC_ANY && ent->tcco_argc != argc) {
					struct ccall_optimization const *end = opts + hi;
					while (ent > opts && strcmp(ent[-1].tcco_name.n_attr, name) == 0)
						--ent;
					for (;;) {
						if (ent >= end)
							goto no_dedicated_optimization;
						if (ent->tcco_argc == CCALL_ARGC_ANY)
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
INTERN WUNUSED NONNULL((1)) struct ccall_optimization const *DCALL
ccall_find_operator_optimization(DeeTypeObject *__restrict type,
                                 Dee_operator_t operator_name,
                                 vstackaddr_t argc) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(cco_optimizations); ++i) {
		struct ccall_optimization const *opts;
		size_t lo, hi;
		if (cco_optimizations[i].tccos_type != type)
			continue;
		lo   = 0;
		hi   = cco_optimizations[i].tccos_size;
		opts = cco_optimizations[i].tccos_opts;
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct ccall_optimization const *ent = &opts[mid];
			Dee_operator_t ent_opname = (Dee_operator_t)ent->tcco_name.n_opname;
			if (operator_name < ent_opname) {
				hi = mid;
			} else if (operator_name > ent_opname) {
				lo = mid + 1;
			} else {
				if (ent->tcco_argc != CCALL_ARGC_ANY && ent->tcco_argc != argc) {
					struct ccall_optimization const *end = opts + hi;
					while (ent > opts && ((Dee_operator_t)ent[-1].tcco_name.n_opname == operator_name))
						--ent;
					for (;;) {
						if (ent >= end)
							goto no_dedicated_optimization;
						if (ent->tcco_argc == CCALL_ARGC_ANY)
							break;
						if (ent->tcco_argc == argc)
							break;
						++ent;
						if ((Dee_operator_t)ent->tcco_name.n_opname != operator_name)
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
