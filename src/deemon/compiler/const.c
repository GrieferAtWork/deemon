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
#ifndef GUARD_DEEMON_COMPILER_CONST_C
#define GUARD_DEEMON_COMPILER_CONST_C 1

#include <deemon/api.h>

#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/compiler/const.h>
#include <deemon/float.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/none.h>
#include <deemon/objmethod.h>
#include <deemon/pair.h>
#include <deemon/property.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/string.h>
#include <deemon/super.h>

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* size_t */

DECL_BEGIN

#define DeeType_ConstEquals(a, b)  ((a) == (b))
#define DeeType_XConstEquals(a, b) ((a) == (b))

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeInt_ConstEqualsNotSame(DeeIntObject const *__restrict a,
                          DeeIntObject const *__restrict b) {
	size_t i, size;
	if (a->ob_size != b->ob_size)
		return false;
	size = (size_t)(a->ob_size < 0 ? -a->ob_size : a->ob_size);
	for (i = 0; i < size; ++i) {
		if (a->ob_digit[i] != b->ob_digit[i])
			return false;
	}
	return true;
}

#define DeeString_ConstEquals(a, b)        DeeString_EqualsSTR(a, b)
#define DeeString_ConstEqualsNotSame(a, b) DeeString_EqualsSTR(a, b)

#define DeeDDI_ConstEquals(a, b) ((a) == (b) || DeeDDI_ConstEqualsNotSame(a, b))
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeDDI_ConstEqualsNotSame(DeeDDIObject const *__restrict a,
                          DeeDDIObject const *__restrict b) {
	if (a->d_ddisize != b->d_ddisize)
		goto nope;
	if (a->d_nstring != b->d_nstring)
		goto nope;
	if (!DeeString_ConstEquals(a->d_strtab, b->d_strtab))
		goto nope;
	if ((a->d_exdat != NULL) != (b->d_exdat != NULL))
		goto nope;
	if (a->d_exdat &&
	    (a->d_exdat->dx_size != b->d_exdat->dx_size ||
	     bcmp(a->d_exdat->dx_data, b->d_exdat->dx_data,
	          a->d_exdat->dx_size) != 0))
		goto nope;
	if (bcmpc(a->d_strings, b->d_strings, a->d_nstring, sizeof(*a->d_strings)) != 0)
		goto nope;
	if (bcmp(&a->d_start, &b->d_start, a->d_ddisize + sizeof(struct Dee_ddi_regs)) != 0)
		goto nope;
	return true;
nope:
	return false;
}

#define DeeCode_ConstEquals(a, b) ((a) == (b) || DeeCode_ConstEqualsNotSame(a, b))
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeCode_ConstEqualsNotSame(DeeCodeObject const *__restrict a,
                           DeeCodeObject const *__restrict b) {
	(void)a;
	(void)b;
	if (a->co_flags != b->co_flags)
		goto nope;
	if (a->co_localc != b->co_localc)
		goto nope;
	if (a->co_constc != b->co_constc)
		goto nope;
	if (a->co_refc != b->co_refc)
		goto nope;
	if (a->co_refstaticc != b->co_refstaticc)
		goto nope;
	if (a->co_exceptc != b->co_exceptc)
		goto nope;
	if (a->co_argc_min != b->co_argc_min)
		goto nope;
	if (a->co_argc_max != b->co_argc_max)
		goto nope;
	if (a->co_framesize != b->co_framesize)
		goto nope;
	if (a->co_codebytes != b->co_codebytes)
		goto nope;
	if (a->co_module != b->co_module)
		goto nope;
	if (a->co_keywords) {
		uint16_t i;
		if (!b->co_keywords)
			goto nope;
		for (i = 0; i < a->co_argc_max; ++i) {
			if (!DeeString_ConstEquals(a->co_keywords[i], b->co_keywords[i]))
				goto nope;
		}
	} else if (b->co_keywords) {
		goto nope;
	}
	if (a->co_defaultv) {
		uint16_t i;
		for (i = 0; i < (a->co_argc_max - a->co_argc_min); ++i) {
			if (!DeeObject_XConstEquals(a->co_defaultv[i],
			                            b->co_defaultv[i]))
				goto nope;
		}
	}
	if (a->co_constv) {
		uint16_t i;
		for (i = 0; i < a->co_constc; ++i) {
			if (!DeeObject_ConstEquals(a->co_constv[i], b->co_constv[i]))
				goto nope;
		}
	}
	if (a->co_exceptv) {
		uint16_t i;
		for (i = 0; i < a->co_exceptc; ++i) {
			if (a->co_exceptv[i].eh_start != b->co_exceptv[i].eh_start)
				goto nope;
			if (a->co_exceptv[i].eh_end != b->co_exceptv[i].eh_end)
				goto nope;
			if (a->co_exceptv[i].eh_addr != b->co_exceptv[i].eh_addr)
				goto nope;
			if (a->co_exceptv[i].eh_stack != b->co_exceptv[i].eh_stack)
				goto nope;
			if (a->co_exceptv[i].eh_flags != b->co_exceptv[i].eh_flags)
				goto nope;
			if (!DeeType_XConstEquals(a->co_exceptv[i].eh_mask,
			                          b->co_exceptv[i].eh_mask))
				goto nope;
		}
	}
	if (bcmp(a->co_code, b->co_code, a->co_codebytes) != 0)
		goto nope;
	return DeeDDI_ConstEquals(a->co_ddi, b->co_ddi);
nope:
	return false;
}

#define DeeFunction_ConstEquals(a, b) ((a) == (b) || DeeFunction_ConstEqualsNotSame(a, b))
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeFunction_ConstEqualsNotSame(DeeFunctionObject const *__restrict a,
                               DeeFunctionObject const *__restrict b) {
	uint16_t i, refc;
	if (a->fo_code->co_refstaticc > a->fo_code->co_refc)
		return false; /* If there are static variables, we'd be mutable! */
	if (!DeeCode_ConstEquals(a->fo_code, b->fo_code))
		return false;
	refc = a->fo_code->co_refc;
	for (i = 0; i < refc; ++i) {
		if (!DeeObject_ConstEquals(a->fo_refv[i], b->fo_refv[i]))
			return false;
	}
	return true;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeYieldFunction_ConstEqualsNotSame(DeeYieldFunctionObject const *__restrict a,
                                    DeeYieldFunctionObject const *__restrict b) {
	size_t i;
	if (!DeeFunction_ConstEquals(a->yf_func, b->yf_func))
		goto nope;
	if (!DeeObject_XConstEquals(a->yf_this, b->yf_this))
		goto nope;
	if (a->yf_pargc != b->yf_pargc)
		goto nope;
	if (a->yf_argc != b->yf_argc)
		goto nope;
	if ((a->yf_kw != NULL) != (b->yf_kw != NULL))
		goto nope;
	if (a->yf_kw != NULL) {
		size_t count;
		struct Dee_code_frame_kwds const *akw = a->yf_kw;
		struct Dee_code_frame_kwds const *bkw = b->yf_kw;
		if (!DeeObject_ConstEquals(akw->fk_kw, bkw->fk_kw))
			goto nope;
		count = (size_t)a->yf_func->fo_code->co_argc_max - a->yf_argc;
		for (i = 0; i < count; ++i) {
			if (!DeeObject_XConstEquals(akw->fk_kargv[i], bkw->fk_kargv[i]))
				goto nope;
		}
	}
	for (i = 0; i < a->yf_argc; ++i) {
		if (!DeeObject_ConstEquals(a->yf_argv[i], b->yf_argv[i]))
			goto nope;
	}
	return true;
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeInstanceMethod_ConstEqualsNotSame(DeeInstanceMethodObject const *__restrict a,
                                     DeeInstanceMethodObject const *__restrict b) {
	return DeeObject_ConstEquals(a->im_func, b->im_func) &&
	       DeeObject_ConstEquals(a->im_this, b->im_this);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeKwds_ConstEqualsNotSame(DeeKwdsObject const *__restrict a,
                           DeeKwdsObject const *__restrict b) {
	size_t i;
	if (a->kw_size != b->kw_size)
		goto nope;
	/* Don't need to be binary identical (map order doesn't matter)
	 * Only need to be logically identical! */
	for (i = 0; i <= a->kw_mask; ++i) {
		DeeStringObject *name = a->kw_map[i].ke_name;
		if (name != NULL) {
			size_t b_index = DeeKwds_IndexOf(Dee_AsObject(b), Dee_AsObject(name));
			if (a->kw_map[i].ke_index != b_index)
				goto nope;
		}
	}
	/* Because we already compared sizes, we know
	 * that "b" can't contain any additional strings. */
	return true;
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeObjMethod_ConstEqualsNotSame(DeeObjMethodObject const *__restrict a,
                                DeeObjMethodObject const *__restrict b) {
	return a->om_func.omf_meth == b->om_func.omf_meth &&
	       DeeObject_ConstEquals(a->om_this, b->om_this);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeClsMethod_ConstEqualsNotSame(DeeClsMethodObject const *__restrict a,
                                DeeClsMethodObject const *__restrict b) {
	return DeeType_ConstEquals(a->clm_type, b->clm_type) &&
	       a->clm_func.clmf_meth == b->clm_func.clmf_meth;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeClsProperty_ConstEqualsNotSame(DeeClsPropertyObject const *__restrict a,
                                  DeeClsPropertyObject const *__restrict b) {
	return DeeType_ConstEquals(a->cp_type, b->cp_type) &&
	       a->cp_get == b->cp_get &&
	       a->cp_del == b->cp_del &&
	       a->cp_set == b->cp_set &&
	       a->cp_bound == b->cp_bound;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeClsMember_ConstEqualsNotSame(DeeClsMemberObject const *__restrict a,
                                DeeClsMemberObject const *__restrict b) {
	return DeeType_ConstEquals(a->cmb_type, b->cmb_type) &&
	       a->cmb_memb.m_name == b->cmb_memb.m_name &&
	       a->cmb_memb.m_desc.md_const == b->cmb_memb.m_desc.md_const &&
	       a->cmb_memb.m_doc == b->cmb_memb.m_doc;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeCMethod_ConstEqualsNotSame(DeeCMethodObject const *__restrict a,
                              DeeCMethodObject const *__restrict b) {
	return a->cm_flags == b->cm_flags &&
	       a->cm_func.cmf_meth == b->cm_func.cmf_meth;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeProperty_ConstEqualsNotSame(DeePropertyObject const *__restrict a,
                               DeePropertyObject const *__restrict b) {
	return DeeObject_XConstEquals(a->p_get, b->p_get) &&
	       DeeObject_XConstEquals(a->p_del, b->p_del) &&
	       DeeObject_XConstEquals(a->p_set, b->p_set);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeRoDict_ConstEqualsNotSame(DeeRoDictObject const *__restrict a,
                             DeeRoDictObject const *__restrict b) {
	size_t i;
	/* Because the hash-table should just be reflective cache
	 * of "rd_vtab", we only have to compare the later! */
	if (a->rd_vsize != b->rd_vsize)
		goto nope;
	for (i = 0; i < a->rd_vsize; ++i) {
		struct Dee_dict_item const *a_item = &a->rd_vtab[i];
		struct Dee_dict_item const *b_item = &b->rd_vtab[i];
#ifndef __OPTIMIZE_SIZE__
		if (a_item->di_hash != b_item->di_hash)
			goto nope; /* Not really necessary because we're also comparing "di_key" */
#endif /* !__OPTIMIZE_SIZE__ */
		if (!DeeObject_ConstEquals(a_item->di_key, b_item->di_key))
			goto nope;
		if (!DeeObject_ConstEquals(a_item->di_value, b_item->di_value))
			goto nope;
	}
	return true;
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeRoSet_ConstEqualsNotSame(DeeRoSetObject const *__restrict a,
                            DeeRoSetObject const *__restrict b) {
	size_t i;
	/* Because the hash-table should just be reflective cache
	 * of "rs_vtab", we only have to compare the later! */
	if (a->rs_vsize != b->rs_vsize)
		goto nope;
	for (i = 0; i < a->rs_vsize; ++i) {
		struct Dee_hashset_item const *a_item = &a->rs_vtab[i];
		struct Dee_hashset_item const *b_item = &b->rs_vtab[i];
#ifndef __OPTIMIZE_SIZE__
		if (a_item->hsi_hash != b_item->hsi_hash)
			goto nope; /* Not really necessary because we're also comparing "hsi_key" */
#endif /* !__OPTIMIZE_SIZE__ */
		if (!DeeObject_ConstEquals(a_item->hsi_key, b_item->hsi_key))
			goto nope;
	}
	return true;
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeSuper_ConstEqualsNotSame(DeeSuperObject const *__restrict a,
                            DeeSuperObject const *__restrict b) {
	return DeeType_ConstEquals(a->s_type, b->s_type) &&
	       DeeObject_ConstEquals(a->s_self, b->s_self);
}



/* Do a compile-time, constant check that "a" and "b" are *deeply* equal.
 * This means that:
 * - Both must have the same type
 * - The type must be immutable and known to the compiler
 * - In case of sequence types, element counts must match, and all
 *   elements must be equal according to this function also.
 *
 * NOTE:
 * - This function is mainly needed because of stuff like `0 == none`,
 *   which is the case because `int(none) == 0`, which would otherwise
 *   mean that `{0}` and `{none}` might (sometimes) compare equal. */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeObject_ConstEquals(DeeObject const *a,
                      DeeObject const *b) {
	DeeTypeObject *tp;
	if (a == b)
		return true;
	tp = Dee_TYPE(a);
	if (tp != Dee_TYPE(b))
		return false;

	/* Only certain types can be compared! */
	if (tp == &DeeString_Type)
		return DeeString_ConstEqualsNotSame((DeeStringObject *)a, (DeeStringObject *)b);
	if (tp == &DeeInt_Type)
		return DeeInt_ConstEqualsNotSame((DeeIntObject *)a, (DeeIntObject *)b);
#ifdef Dee_CONFIG_BOOL_TLS /* Without TLS booleans, the "a == b" check above is already enough */
	if (tp == &DeeBool_Type)
		return !!DeeBool_IsTrue(a) == !!DeeBool_IsTrue(b);
#endif /* Dee_CONFIG_BOOL_TLS */
	if (tp == &DeeRoDict_Type)
		return DeeRoDict_ConstEqualsNotSame((DeeRoDictObject *)a, (DeeRoDictObject *)b);
	if (tp == &DeeRoSet_Type)
		return DeeRoSet_ConstEqualsNotSame((DeeRoSetObject *)a, (DeeRoSetObject *)b);
#ifdef CONFIG_HAVE_FPU
	if (tp == &DeeFloat_Type)
		return DeeFloat_VALUE(a) == DeeFloat_VALUE(b);
#endif /* CONFIG_HAVE_FPU */
	if (tp == &DeeNone_Type)
		return true; /* "none" is anything typed as "type(none)" */

	if (DeeType_IsAbstract(tp))
		return true; /* Dummy instance of (constructable), abstract object: "Object(), Sequence(), Mapping(), ..." */
	if (tp == &DeeDDI_Type)
		return DeeDDI_ConstEqualsNotSame((DeeDDIObject *)a, (DeeDDIObject *)b);
	if (tp == &DeeCode_Type)
		return DeeCode_ConstEqualsNotSame((DeeCodeObject *)a, (DeeCodeObject *)b);
	if (tp == &DeeFunction_Type)
		return DeeFunction_ConstEqualsNotSame((DeeFunctionObject *)a, (DeeFunctionObject *)b);
	if (tp == &DeeYieldFunction_Type)
		return DeeYieldFunction_ConstEqualsNotSame((DeeYieldFunctionObject *)a, (DeeYieldFunctionObject *)b);
	if (tp == &DeeInstanceMethod_Type)
		return DeeInstanceMethod_ConstEqualsNotSame((DeeInstanceMethodObject *)a, (DeeInstanceMethodObject *)b);
	if (tp == &DeeKwds_Type)
		return DeeKwds_ConstEqualsNotSame((DeeKwdsObject *)a, (DeeKwdsObject *)b);
	if (tp == &DeeObjMethod_Type || tp == &DeeKwObjMethod_Type)
		return DeeObjMethod_ConstEqualsNotSame((DeeObjMethodObject *)a, (DeeObjMethodObject *)b);
	if (tp == &DeeClsMethod_Type || tp == &DeeKwClsMethod_Type)
		return DeeClsMethod_ConstEqualsNotSame((DeeClsMethodObject *)a, (DeeClsMethodObject *)b);
	if (tp == &DeeClsProperty_Type)
		return DeeClsProperty_ConstEqualsNotSame((DeeClsPropertyObject *)a, (DeeClsPropertyObject *)b);
	if (tp == &DeeClsMember_Type)
		return DeeClsMember_ConstEqualsNotSame((DeeClsMemberObject *)a, (DeeClsMemberObject *)b);
	if (tp == &DeeCMethod_Type || tp == &DeeKwCMethod_Type ||
	    tp == &DeeCMethod0_Type || tp == &DeeCMethod1_Type)
		return DeeCMethod_ConstEqualsNotSame((DeeCMethodObject *)a, (DeeCMethodObject *)b);
#ifdef CONFIG_ENABLE_SEQ_ONE_TYPE
	if (tp == &DeeSeqOne_Type) {
		return DeeObject_ConstEquals(((DeeSeqOneObject const *)a)->so_item,
		                             ((DeeSeqOneObject const *)b)->so_item);
	}
#endif /* CONFIG_ENABLE_SEQ_ONE_TYPE */
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	if (tp == &DeeSeqPair_Type) {
		return DeeObject_ConstEquals(((DeeSeqPairObject const *)a)->sp_items[0],
		                             ((DeeSeqPairObject const *)b)->sp_items[0]) &&
		       DeeObject_ConstEquals(((DeeSeqPairObject const *)a)->sp_items[1],
		                             ((DeeSeqPairObject const *)b)->sp_items[1]);
	}
#endif /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	if (tp == &DeeProperty_Type)
		return DeeProperty_ConstEqualsNotSame((DeePropertyObject *)a, (DeePropertyObject *)b);
	if (tp == &DeeSuper_Type)
		return DeeSuper_ConstEqualsNotSame((DeeSuperObject *)a, (DeeSuperObject *)b);

	/* TODO: Subclasses of "Error" */
	/* TODO: User-defined classes where all members are "final" */

	return false;
}

INTERN ATTR_PURE WUNUSED bool DCALL
DeeObject_XConstEquals(DeeObject const *a,
                       DeeObject const *b) {
	return a == b || (a && b && DeeObject_ConstEquals(a, b));
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_CONST_C */
