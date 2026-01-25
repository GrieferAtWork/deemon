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
#ifndef GUARD_DEEMON_RUNTIME_TYPE_MEMBER_C
#define GUARD_DEEMON_RUNTIME_TYPE_MEMBER_C 1

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_Unpack1, DeeArg_UnpackKw */
#include <deemon/bool.h>            /* DeeBool_Type, return_bool */
#include <deemon/error-rt.h>        /* DeeRT_ATTRIBUTE_ACCESS_*, DeeRT_Err* */
#include <deemon/error.h>           /* DeeError_Throwf, DeeError_TypeError */
#include <deemon/float.h>           /* DeeFloat_New, DeeFloat_Type */
#include <deemon/format.h>          /* Dee_VPPackf, Dee_VPPackf_Cleanup, Dee_VPackf, Dee_va_list_struct */
#include <deemon/int.h>             /* DeeInt_* */
#include <deemon/kwds.h>            /* DeeKwds_Check, DeeKwds_SIZE */
#include <deemon/mro.h>             /* Dee_ATTRINFO_*, Dee_ATTRITER_HEAD, Dee_ATTRPERM_F_*, Dee_attrdesc, Dee_attriter, Dee_attriter_init, Dee_attriter_type, Dee_attrperm_t, Dee_type_member_*, type_getset_*, type_member_get, type_member_iterattr, type_method_*, type_obmemb_*, type_obmeth_*, type_obprop_* */
#include <deemon/none.h>            /* DeeNone_Check, DeeNone_Type, Dee_None, return_none */
#include <deemon/object.h>
#include <deemon/string.h>          /* DeeString*, STRING_ERROR_FIGNORE */
#include <deemon/system-features.h> /* CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY, strlen */
#include <deemon/util/atomic.h>     /* atomic_* */
#include <deemon/variant.h>         /* Dee_variant, Dee_variant_* */

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* intN_t, uintN_t, uintptr_t */

/* Type member access. */

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
#define IF_THREADS(...) __VA_ARGS__
#else /* !CONFIG_NO_THREADS */
#define IF_THREADS(...) /* nothing */
#endif /* CONFIG_NO_THREADS */


/* Ensure that all struct-type-codes have their least significant bits set. */
STATIC_ASSERT((STRUCT_NONE & 1) == 1);
STATIC_ASSERT((STRUCT_OBJECT & 1) == 1);
STATIC_ASSERT((STRUCT_WOBJECT & 1) == 1);
STATIC_ASSERT((STRUCT_OBJECT_OPT & 1) == 1);
STATIC_ASSERT((STRUCT_WOBJECT_OPT & 1) == 1);
STATIC_ASSERT((STRUCT_CSTR & 1) == 1);
STATIC_ASSERT((STRUCT_CSTR_OPT & 1) == 1);
STATIC_ASSERT((STRUCT_CSTR_EMPTY & 1) == 1);
STATIC_ASSERT((STRUCT_STRING & 1) == 1);
STATIC_ASSERT((STRUCT_CHAR & 1) == 1);
STATIC_ASSERT((STRUCT_VARIANT & 1) == 1);
STATIC_ASSERT((STRUCT_BOOL8 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOL16 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOL32 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOL64 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT0 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT1 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT2 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT3 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT4 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT5 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT6 & 1) == 1);
STATIC_ASSERT((STRUCT_BOOLBIT7 & 1) == 1);
STATIC_ASSERT((STRUCT_FLOAT & 1) == 1);
STATIC_ASSERT((STRUCT_DOUBLE & 1) == 1);
STATIC_ASSERT((STRUCT_LDOUBLE & 1) == 1);
STATIC_ASSERT((STRUCT_INT8 & 1) == 1);
STATIC_ASSERT((STRUCT_INT16 & 1) == 1);
STATIC_ASSERT((STRUCT_INT32 & 1) == 1);
STATIC_ASSERT((STRUCT_INT64 & 1) == 1);
STATIC_ASSERT((STRUCT_INT128 & 1) == 1);

STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT0) == 0x01);
STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT1) == 0x02);
STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT2) == 0x04);
STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT3) == 0x08);
STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT4) == 0x10);
STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT5) == 0x20);
STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT6) == 0x40);
STATIC_ASSERT(STRUCT_BOOLBITMASK(STRUCT_BOOLBIT7) == 0x80);
STATIC_ASSERT(STRUCT_BOOLBIT(0x01) == STRUCT_BOOLBIT0);
STATIC_ASSERT(STRUCT_BOOLBIT(0x02) == STRUCT_BOOLBIT1);
STATIC_ASSERT(STRUCT_BOOLBIT(0x04) == STRUCT_BOOLBIT2);
STATIC_ASSERT(STRUCT_BOOLBIT(0x08) == STRUCT_BOOLBIT3);
STATIC_ASSERT(STRUCT_BOOLBIT(0x10) == STRUCT_BOOLBIT4);
STATIC_ASSERT(STRUCT_BOOLBIT(0x20) == STRUCT_BOOLBIT5);
STATIC_ASSERT(STRUCT_BOOLBIT(0x40) == STRUCT_BOOLBIT6);
STATIC_ASSERT(STRUCT_BOOLBIT(0x80) == STRUCT_BOOLBIT7);


/* Returns the object-type used to represent a given type-member. */
INTERN WUNUSED NONNULL((1)) DeeTypeObject *DCALL
type_member_typefor(struct type_member const *__restrict self) {
	if (TYPE_MEMBER_ISCONST(self))
		return Dee_TYPE(self->m_desc.md_const);
	switch (self->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

	case STRUCT_NONE & ~(STRUCT_CONST):
		return &DeeNone_Type;

	case STRUCT_CSTR & ~(STRUCT_CONST):
	case STRUCT_CSTR_EMPTY & ~(STRUCT_CONST):
	case STRUCT_STRING & ~(STRUCT_CONST):
	case STRUCT_CHAR & ~(STRUCT_CONST):
		return &DeeString_Type;

	case STRUCT_BOOL8 & ~(STRUCT_CONST):
	case STRUCT_BOOL16 & ~(STRUCT_CONST):
	case STRUCT_BOOL32 & ~(STRUCT_CONST):
	case STRUCT_BOOL64 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT0 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT1 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT2 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT3 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT4 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT5 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT6 & ~(STRUCT_CONST):
	case STRUCT_BOOLBIT7 & ~(STRUCT_CONST):
		return &DeeBool_Type;

	case STRUCT_FLOAT & ~(STRUCT_CONST):
	case STRUCT_DOUBLE & ~(STRUCT_CONST):
	case STRUCT_LDOUBLE & ~(STRUCT_CONST):
		return &DeeFloat_Type;

	case STRUCT_INT8 & ~(STRUCT_CONST):
	case STRUCT_INT16 & ~(STRUCT_CONST):
	case STRUCT_INT32 & ~(STRUCT_CONST):
	case STRUCT_INT64 & ~(STRUCT_CONST):
	case STRUCT_INT128 & ~(STRUCT_CONST):
	case (STRUCT_UNSIGNED | STRUCT_INT8) & ~(STRUCT_CONST):
	case (STRUCT_UNSIGNED | STRUCT_INT16) & ~(STRUCT_CONST):
	case (STRUCT_UNSIGNED | STRUCT_INT32) & ~(STRUCT_CONST):
	case (STRUCT_UNSIGNED | STRUCT_INT64) & ~(STRUCT_CONST):
	case (STRUCT_UNSIGNED | STRUCT_INT128) & ~(STRUCT_CONST):
		return &DeeInt_Type;

	default: break;
	}
	return NULL;
}


struct type_method_attriter {
	Dee_ATTRITER_HEAD
	DeeTypeObject            *tmai_tpself; /* [1..1][const] The type declaring "tmai_chain" (either via its "tp_methods" or "tp_class_methods") */
	struct type_method const *tmai_chain;  /* [1..1][lock(ATOMIC)] Next method to yield. */
	Dee_attrperm_t            tmai_perm;   /* [const] Chain access base perms. */
	uint16_t                  tmai_type;   /* [const] Either `Dee_ATTRINFO_INSTANCE_METHOD' or `Dee_ATTRINFO_METHOD' */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_method_attriter_next(struct type_method_attriter *__restrict self,
                          /*out*/ struct Dee_attrdesc *__restrict desc) {
	struct type_method const *item;
	do {
		item = atomic_read(&self->tmai_chain);
		if (!item->m_name)
			return 1;
	} while (!atomic_cmpxch_or_write(&self->tmai_chain, item, item + 1));
	desc->ad_name = item->m_name;
	desc->ad_doc  = item->m_doc;
	desc->ad_perm = self->tmai_perm;
	desc->ad_info.ai_type = self->tmai_type;
	desc->ad_info.ai_decl = Dee_AsObject(self->tmai_tpself);
	desc->ad_info.ai_value.v_method = item;
	desc->ad_type = NULL;
	return 0;
}


PRIVATE struct Dee_attriter_type tpconst type_method_attriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&type_method_attriter_next,
};



struct type_getset_attriter {
	Dee_ATTRITER_HEAD
	DeeTypeObject            *tgsai_tpself; /* [1..1][const] The type declaring "tgsai_chain" (either via its "tp_getsets" or "tp_class_getsets") */
	struct type_getset const *tgsai_chain;  /* [1..1][lock(ATOMIC)] Next getset to yield. */
	Dee_attrperm_t            tgsai_perm;   /* [const] Chain access base perms. */
	uint16_t                  tgsai_type;   /* [const] Either `Dee_ATTRINFO_INSTANCE_GETSET' or `Dee_ATTRINFO_GETSET' */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_getset_attriter_next(struct type_getset_attriter *__restrict self,
                          /*out*/ struct Dee_attrdesc *__restrict desc) {
	struct type_getset const *item;
	do {
		item = atomic_read(&self->tgsai_chain);
		if (!item->gs_name)
			return 1;
	} while (!atomic_cmpxch_or_write(&self->tgsai_chain, item, item + 1));
	desc->ad_name = item->gs_name;
	desc->ad_doc  = item->gs_doc;
	desc->ad_perm = self->tgsai_perm;
	if (item->gs_get)
		desc->ad_perm |= Dee_ATTRPERM_F_CANGET;
	if (item->gs_del)
		desc->ad_perm |= Dee_ATTRPERM_F_CANDEL;
	if (item->gs_set)
		desc->ad_perm |= Dee_ATTRPERM_F_CANSET;
	desc->ad_info.ai_type = self->tgsai_type;
	desc->ad_info.ai_decl = Dee_AsObject(self->tgsai_tpself);
	desc->ad_info.ai_value.v_getset = item;
	desc->ad_type = NULL;
	return 0;
}


PRIVATE struct Dee_attriter_type tpconst type_getset_attriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&type_getset_attriter_next,
};



struct type_member_attriter {
	Dee_ATTRITER_HEAD
	DeeTypeObject            *tmai_tpself; /* [1..1][const] The type declaring "tmai_chain" (either via its "tp_members" or "tp_class_members") */
	struct type_member const *tmai_chain;  /* [1..1][lock(ATOMIC)] Next member to yield. */
	Dee_attrperm_t            tmai_perm;   /* [const] Chain access base perms. */
	uint16_t                  tmai_type;   /* [const] Either `Dee_ATTRINFO_INSTANCE_MEMBER' or `Dee_ATTRINFO_MEMBER' */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_member_attriter_next(struct type_member_attriter *__restrict self,
                          /*out*/ struct Dee_attrdesc *__restrict desc) {
	struct type_member const *item;
	do {
		item = atomic_read(&self->tmai_chain);
		if (!item->m_name)
			return 1;
	} while (!atomic_cmpxch_or_write(&self->tmai_chain, item, item + 1));
	desc->ad_name = item->m_name;
	desc->ad_doc  = item->m_doc;
	desc->ad_perm = self->tmai_perm;
	if (!TYPE_MEMBER_ISCONST(item)) {
		if (!(item->m_desc.md_field.mdf_type & STRUCT_CONST))
			desc->ad_perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
	}
	desc->ad_info.ai_type = self->tmai_type;
	desc->ad_info.ai_decl = Dee_AsObject(self->tmai_tpself);
	desc->ad_info.ai_value.v_member = item;
	desc->ad_type = NULL;
	return 0;
}


PRIVATE struct Dee_attriter_type tpconst type_member_attriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&type_member_attriter_next,
};



INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
type_method_iterattr(DeeTypeObject *__restrict tp_self,
                     struct type_method const *chain, Dee_attrperm_t chain_perm,
                     struct Dee_attriter *iterbuf, size_t bufsize) {
	struct type_method_attriter *iter = (struct type_method_attriter *)iterbuf;
	ASSERT(chain_perm == (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL) ||
	       chain_perm == (Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL));
	if (bufsize >= sizeof(struct type_method_attriter)) {
		iter->tmai_tpself = tp_self;
		iter->tmai_chain  = chain;
		iter->tmai_perm   = chain_perm;
		iter->tmai_type   = Dee_ATTRINFO_METHOD;
		Dee_attriter_init(iter, &type_method_attriter_type);
	}
	return sizeof(struct type_method_attriter);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
type_getset_iterattr(DeeTypeObject *__restrict tp_self,
                     struct type_getset const *chain, Dee_attrperm_t chain_perm,
                     struct Dee_attriter *iterbuf, size_t bufsize) {
	struct type_getset_attriter *iter = (struct type_getset_attriter *)iterbuf;
	ASSERT(chain_perm == (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_PROPERTY) ||
	       chain_perm == (Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_PROPERTY));
	if (bufsize >= sizeof(struct type_getset_attriter)) {
		iter->tgsai_tpself = tp_self;
		iter->tgsai_chain  = chain;
		iter->tgsai_perm   = chain_perm;
		iter->tgsai_type   = Dee_ATTRINFO_GETSET;
		Dee_attriter_init(iter, &type_getset_attriter_type);
	}
	return sizeof(struct type_getset_attriter);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
type_member_iterattr(DeeTypeObject *__restrict tp_self,
                     struct type_member const *chain, Dee_attrperm_t chain_perm,
                     struct Dee_attriter *iterbuf, size_t bufsize) {
	struct type_member_attriter *iter = (struct type_member_attriter *)iterbuf;
	ASSERT(chain_perm == (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET) ||
	       chain_perm == (Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_CANGET));
	if (bufsize >= sizeof(struct type_member_attriter)) {
		iter->tmai_tpself = tp_self;
		iter->tmai_chain  = chain;
		iter->tmai_perm   = chain_perm;
		iter->tmai_type   = Dee_ATTRINFO_MEMBER;
		Dee_attriter_init(iter, &type_member_attriter_type);
	}
	return sizeof(struct type_member_attriter);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
type_obmeth_iterattr(DeeTypeObject *__restrict tp_self,
                     struct Dee_attriter *iterbuf, size_t bufsize) {
	struct type_method_attriter *iter = (struct type_method_attriter *)iterbuf;
	if (bufsize >= sizeof(struct type_method_attriter)) {
		iter->tmai_tpself = tp_self;
		iter->tmai_chain  = tp_self->tp_methods;
		iter->tmai_perm   = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL | Dee_ATTRPERM_F_WRAPPER;
		iter->tmai_type   = Dee_ATTRINFO_INSTANCE_METHOD;
		Dee_attriter_init(iter, &type_method_attriter_type);
	}
	return sizeof(struct type_method_attriter);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
type_obprop_iterattr(DeeTypeObject *__restrict tp_self,
                     struct Dee_attriter *iterbuf, size_t bufsize) {
	struct type_getset_attriter *iter = (struct type_getset_attriter *)iterbuf;
	if (bufsize >= sizeof(struct type_getset_attriter)) {
		iter->tgsai_tpself = tp_self;
		iter->tgsai_chain  = tp_self->tp_getsets;
		iter->tgsai_perm   = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_WRAPPER;
		iter->tgsai_type   = Dee_ATTRINFO_INSTANCE_GETSET;
		Dee_attriter_init(iter, &type_getset_attriter_type);
	}
	return sizeof(struct type_getset_attriter);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
type_obmemb_iterattr(DeeTypeObject *__restrict tp_self,
                     struct Dee_attriter *iterbuf, size_t bufsize) {
	struct type_member_attriter *iter = (struct type_member_attriter *)iterbuf;
	if (bufsize >= sizeof(struct type_member_attriter)) {
		iter->tmai_tpself = tp_self;
		iter->tmai_chain  = tp_self->tp_members;
		iter->tmai_perm   = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_WRAPPER;
		iter->tmai_type   = Dee_ATTRINFO_INSTANCE_MEMBER;
		Dee_attriter_init(iter, &type_member_attriter_type);
	}
	return sizeof(struct type_member_attriter);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obmeth_call(DeeTypeObject *cls_type,
                 struct type_method const *desc,
                 size_t argc, DeeObject *const *argv) {
	if unlikely(!argc) {
		DeeError_Throwf(&DeeError_TypeError,
		                "classmethod `%s' must be called with at least 1 argument",
		                desc->m_name);
		goto err;
	}
	if (DeeObject_AssertTypeOrAbstract(argv[0], cls_type))
		goto err;

	/* Use the first argument as the this-argument. */
	if (desc->m_flag & TYPE_METHOD_FKWDS)
		return (*(Dee_kwobjmethod_t)desc->m_func)(argv[0], argc - 1, argv + 1, NULL);
	return (*desc->m_func)(argv[0], argc - 1, argv + 1);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obmeth_call_kw(DeeTypeObject *cls_type,
                    struct type_method const *desc,
                    size_t argc, DeeObject *const *argv,
                    DeeObject *kw) {
	if unlikely(!argc) {
		DeeError_Throwf(&DeeError_TypeError,
		                "classmethod `%s' must be called with at least 1 argument",
		                desc->m_name);
		goto err;
	}
	if (DeeObject_AssertTypeOrAbstract(argv[0], cls_type))
		goto err;

	/* Use the first argument as the this-argument. */
	if (desc->m_flag & TYPE_METHOD_FKWDS)
		return (*(Dee_kwobjmethod_t)desc->m_func)(argv[0], argc - 1, argv + 1, kw);
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t temp = DeeObject_Size(kw);
			if unlikely(temp == (size_t)-1)
				goto err;
			if (temp != 0)
				goto err_no_keywords;
		}
	}
	return (*desc->m_func)(argv[0], argc - 1, argv + 1);
err_no_keywords:
	err_keywords_func_not_accepted_string(cls_type, desc->m_name, kw);
err:
	return NULL;
}

#ifdef CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY
#define VALIST_ADDR(x) (&(x))
#else /* CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */
#define VALIST_ADDR(x) (&(x)[0])
#endif /* !CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */


INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_obmeth_vcallf(DeeTypeObject *cls_type,
                   struct type_method const *desc,
                   char const *__restrict format,
                   va_list args) {
	DREF DeeObject *thisarg, *result;
	if unlikely(*format == '\0') {
		DeeError_Throwf(&DeeError_TypeError,
		                "classmethod `%s' must be called with at least 1 argument",
		                desc->m_name);
		goto err;
	}

	/* Use the first argument as the this-argument. */
	thisarg = Dee_VPPackf((char const **)&format, (struct Dee_va_list_struct *)VALIST_ADDR(args));
	if unlikely(!thisarg) {
		Dee_VPPackf_Cleanup(format, ((struct Dee_va_list_struct *)VALIST_ADDR(args))->vl_ap);
		goto err;
	} 
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err_thisarg;

	/* Invoke the function. */
	result = type_method_vcallf(desc, thisarg, format, args);
	Dee_Decref(thisarg);
	return result;
err_thisarg:
	Dee_Decref(thisarg);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_method_call_kw_normal(struct type_method const *desc,
                           DeeObject *self, size_t argc,
                           DeeObject *const *argv, DeeObject *kw) {
	ASSERT(!(desc->m_flag & TYPE_METHOD_FKWDS));
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t temp = DeeObject_Size(kw);
			if unlikely(temp == (size_t)-1)
				goto err;
			if (temp != 0)
				goto err_no_keywords;
		}
	}
	return (*desc->m_func)(self, argc, argv);
err_no_keywords:
	err_keywords_func_not_accepted_string(Dee_TYPE(self), desc->m_name, kw);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_getset_get(struct type_getset const *desc,
                DeeObject *__restrict self) {
	if likely(desc->gs_get)
		return (*desc->gs_get)(self);
	DeeRT_ErrRestrictedGetSet(self, desc, DeeRT_ATTRIBUTE_ACCESS_GET);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
type_getset_del(struct type_getset const *desc,
                DeeObject *__restrict self) {
	if likely(desc->gs_del)
		return (*desc->gs_del)(self);
	return DeeRT_ErrRestrictedGetSet(self, desc, DeeRT_ATTRIBUTE_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
type_getset_set(struct type_getset const *desc,
                DeeObject *self, DeeObject *value) {
	if likely(desc->gs_set)
		return (*desc->gs_set)(self, value);
	return DeeRT_ErrRestrictedGetSet(self, desc, DeeRT_ATTRIBUTE_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obprop_call(DeeTypeObject *cls_type,
                 struct type_getset const *desc,
                 size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	if unlikely(!desc->gs_get)
		goto err_unbound;
	DeeArg_Unpack1(err, argc, argv, "get", &thisarg);
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err;
	return (*desc->gs_get)(thisarg);
err_unbound:
	DeeRT_ErrRestrictedGetSet(cls_type, desc, DeeRT_ATTRIBUTE_ACCESS_GET);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obprop_call_kw(DeeTypeObject *cls_type,
                    struct type_getset const *desc,
                    size_t argc, DeeObject *const *argv,
                    DeeObject *kw) {
	DeeObject *thisarg;
	if unlikely(!desc->gs_get)
		goto err_unbound;
	if unlikely(DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:get", &thisarg))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err;
	return (*desc->gs_get)(thisarg);
err_unbound:
	DeeRT_ErrRestrictedGetSet(cls_type, desc, DeeRT_ATTRIBUTE_ACCESS_GET);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_obprop_vcallf(DeeTypeObject *cls_type,
                   struct type_getset const *desc,
                   char const *__restrict format,
                   va_list args) {
	DREF DeeObject *thisarg, *result;
	if unlikely(!desc->gs_get)
		goto err_unbound;
	thisarg = Dee_VPackf(format, args);
	if unlikely(!thisarg)
		goto err;
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err_thisarg;
	result = (*desc->gs_get)(thisarg);
	Dee_Decref(thisarg);
	return result;
err_thisarg:
	Dee_Decref(thisarg);
err:
	return NULL;
err_unbound:
	DeeRT_ErrRestrictedGetSet(cls_type, desc, DeeRT_ATTRIBUTE_ACCESS_GET);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obmemb_call(DeeTypeObject *cls_type,
                 struct type_member const *desc,
                 size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	DeeArg_Unpack1(err, argc, argv, "get", &thisarg);
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err;
	return type_member_get(desc, thisarg);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obmemb_call_kw(DeeTypeObject *cls_type,
                    struct type_member const *desc,
                    size_t argc, DeeObject *const *argv,
                    DeeObject *kw) {
	DeeObject *thisarg;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err;
	return type_member_get(desc, thisarg);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_obmemb_vcallf(DeeTypeObject *cls_type,
                   struct type_member const *desc,
                   char const *__restrict format, va_list args) {
	DREF DeeObject *thisarg, *result;
	thisarg = Dee_VPackf(format, args);
	if unlikely(!thisarg)
		goto err;
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err_thisarg;
	result = type_member_get(desc, thisarg);
	Dee_Decref(thisarg);
	return result;
err_thisarg:
	Dee_Decref(thisarg);
err:
	return NULL;
}


#define FIELD(T)  (*(T *)((uintptr_t)self + desc->m_desc.md_field.mdf_offset))

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_type_member_get(struct type_member const *desc,
                    DeeObject *__restrict self) {
	if (TYPE_MEMBER_ISCONST(desc)) {
		ASSERT_OBJECT(desc->m_desc.md_const);
		return_reference_(desc->m_desc.md_const);
	}
	switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)

	CASE(STRUCT_NONE):
ret_none:
		return_none;

	CASE(STRUCT_WOBJECT):
	CASE(STRUCT_WOBJECT_OPT): {
		DeeObject *ob;
		ob = Dee_weakref_lock(&FIELD(struct Dee_weakref));
		if unlikely(!ob)
			goto handle_null_ob;
		return ob;
	}

	CASE(STRUCT_OBJECT):
	CASE(STRUCT_OBJECT_OPT): {
		DeeObject *ob;
		ob = FIELD(DeeObject *);
		if unlikely(!ob) {
handle_null_ob:
			if (!(desc->m_desc.md_field.mdf_type & (STRUCT_OBJECT_OPT & ~(STRUCT_OBJECT))))
				goto is_unbound;
			ob = Dee_None;
		}
		Dee_Incref(ob);
		return ob;
	}	break;

	CASE(STRUCT_CSTR):
	CASE(STRUCT_CSTR_OPT):
	CASE(STRUCT_CSTR_EMPTY): {
		char const *cstr = FIELD(char const *);
		if unlikely(!cstr) {
			switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

			CASE(STRUCT_CSTR_OPT):
				goto ret_none;

			CASE(STRUCT_CSTR_EMPTY):
				cstr = "";
				break;

			default:
				goto is_unbound;
			}
		}
		return DeeString_NewAutoUtf8(cstr);
	}	break;

	CASE(STRUCT_STRING): {
		char const *str = &FIELD(char);
		return DeeString_NewUtf8(str, strlen(str), STRING_ERROR_FIGNORE);
	}

	CASE(STRUCT_CHAR):
		return DeeString_Chr(FIELD(unsigned char));

	CASE(STRUCT_VARIANT): {
		DREF DeeObject *result = Dee_variant_getobject(&FIELD(struct Dee_variant));
		if unlikely(result == ITER_DONE)
			goto is_unbound;
		return result;
	}	break;

	CASE(STRUCT_BOOL8):
		return_bool(FIELD(uint8_t) != 0);

	CASE(STRUCT_BOOL16):
		return_bool(FIELD(uint16_t) != 0);

	CASE(STRUCT_BOOL32):
		return_bool(FIELD(uint32_t) != 0);

	CASE(STRUCT_BOOL64):
		return_bool(FIELD(uint64_t) != 0);

#ifdef __OPTIMIZE_SIZE__
	CASE(STRUCT_BOOLBIT0): CASE(STRUCT_BOOLBIT1):
	CASE(STRUCT_BOOLBIT2): CASE(STRUCT_BOOLBIT3):
	CASE(STRUCT_BOOLBIT4): CASE(STRUCT_BOOLBIT5):
	CASE(STRUCT_BOOLBIT6): CASE(STRUCT_BOOLBIT7):
		return_bool((FIELD(uint8_t) & STRUCT_BOOLBITMASK(desc->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC))) != 0);
#else /* __OPTIMIZE_SIZE__ */
	CASE(STRUCT_BOOLBIT0): return_bool((FIELD(uint8_t) & 0x01) != 0);
	CASE(STRUCT_BOOLBIT1): return_bool((FIELD(uint8_t) & 0x02) != 0);
	CASE(STRUCT_BOOLBIT2): return_bool((FIELD(uint8_t) & 0x04) != 0);
	CASE(STRUCT_BOOLBIT3): return_bool((FIELD(uint8_t) & 0x08) != 0);
	CASE(STRUCT_BOOLBIT4): return_bool((FIELD(uint8_t) & 0x10) != 0);
	CASE(STRUCT_BOOLBIT5): return_bool((FIELD(uint8_t) & 0x20) != 0);
	CASE(STRUCT_BOOLBIT6): return_bool((FIELD(uint8_t) & 0x40) != 0);
	CASE(STRUCT_BOOLBIT7): return_bool((FIELD(uint8_t) & 0x80) != 0);
#endif /* !__OPTIMIZE_SIZE__ */

	CASE(STRUCT_FLOAT):
		return DeeFloat_New((double)FIELD(float));

	CASE(STRUCT_DOUBLE):
		return DeeFloat_New((double)FIELD(double));

	CASE(STRUCT_LDOUBLE):
		return DeeFloat_New((double)FIELD(long double));

	CASE(STRUCT_INT8):
		return DeeInt_NewInt8(FIELD(int8_t));

	CASE(STRUCT_UNSIGNED | STRUCT_INT8):
		return DeeInt_NewUInt8(FIELD(uint8_t));

	CASE(STRUCT_INT16):
		return DeeInt_NewInt16(FIELD(int16_t));

	CASE(STRUCT_UNSIGNED | STRUCT_INT16):
		return DeeInt_NewUInt16(FIELD(uint16_t));

	CASE(STRUCT_INT32):
		return DeeInt_NewInt32(FIELD(int32_t));

	CASE(STRUCT_UNSIGNED | STRUCT_INT32):
		return DeeInt_NewUInt32(FIELD(uint32_t));

	CASE(STRUCT_INT64):
		return DeeInt_NewInt64(FIELD(int64_t));

	CASE(STRUCT_UNSIGNED | STRUCT_INT64):
		return DeeInt_NewUInt64(FIELD(uint64_t));

	CASE(STRUCT_INT128):
		return DeeInt_NewInt128(FIELD(Dee_int128_t));

	CASE(STRUCT_UNSIGNED | STRUCT_INT128):
		return DeeInt_NewUInt128(FIELD(Dee_uint128_t));

#undef CASE
	default: break;
	}
is_unbound:
	return DeeRT_ErrUnboundMember(self, desc);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_type_member_tryget(struct type_member const *desc,
                       DeeObject *__restrict self) {
	if (TYPE_MEMBER_ISCONST(desc)) {
		ASSERT_OBJECT(desc->m_desc.md_const);
		return_reference_(desc->m_desc.md_const);
	}
	switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)

	CASE(STRUCT_WOBJECT): {
		DeeObject *ob;
		ob = Dee_weakref_lock(&FIELD(struct Dee_weakref));
		if unlikely(!ob)
			goto handle_null_ob;
		return ob;
	}	break;

	CASE(STRUCT_OBJECT): {
		DeeObject *ob;
		ob = FIELD(DeeObject *);
		if unlikely(!ob) {
handle_null_ob:
			ob = Dee_None;
		}
		Dee_Incref(ob);
		return ob;
	}	break;

	CASE(STRUCT_CSTR): {
		char const *cstr = FIELD(char const *);
		if unlikely(!cstr)
			goto is_unbound;
		return DeeString_NewAutoUtf8(cstr);
	}	break;

	CASE(STRUCT_VARIANT): {
		DREF DeeObject *result = Dee_variant_getobject(&FIELD(struct Dee_variant));
		if unlikely(result == ITER_DONE)
			goto is_unbound;
		return result;
	}	break;

	CASE(STRUCT_NONE):
	CASE(STRUCT_OBJECT_OPT):
	CASE(STRUCT_WOBJECT_OPT):
	CASE(STRUCT_CSTR_OPT):
	CASE(STRUCT_CSTR_EMPTY):
	CASE(STRUCT_STRING):
	CASE(STRUCT_CHAR):
	CASE(STRUCT_BOOL8):
	CASE(STRUCT_BOOL16):
	CASE(STRUCT_BOOL32):
	CASE(STRUCT_BOOL64):
	CASE(STRUCT_BOOLBIT0):
	CASE(STRUCT_BOOLBIT1):
	CASE(STRUCT_BOOLBIT2):
	CASE(STRUCT_BOOLBIT3):
	CASE(STRUCT_BOOLBIT4):
	CASE(STRUCT_BOOLBIT5):
	CASE(STRUCT_BOOLBIT6):
	CASE(STRUCT_BOOLBIT7):
	CASE(STRUCT_FLOAT):
	CASE(STRUCT_DOUBLE):
	CASE(STRUCT_LDOUBLE):
	CASE(STRUCT_INT8):
	CASE(STRUCT_UNSIGNED | STRUCT_INT8):
	CASE(STRUCT_INT16):
	CASE(STRUCT_UNSIGNED | STRUCT_INT16):
	CASE(STRUCT_INT32):
	CASE(STRUCT_UNSIGNED | STRUCT_INT32):
	CASE(STRUCT_INT64):
	CASE(STRUCT_UNSIGNED | STRUCT_INT64):
	CASE(STRUCT_INT128):
	CASE(STRUCT_UNSIGNED | STRUCT_INT128):
		return Dee_type_member_get(desc, self);

#undef CASE
	default: break;
	}
is_unbound:
	return ITER_DONE;
}

PUBLIC WUNUSED NONNULL((1, 2)) bool DCALL
Dee_type_member_bound(struct type_member const *desc,
                      DeeObject *__restrict self) {
	if (TYPE_MEMBER_ISCONST(desc)) {
		ASSERT_OBJECT(desc->m_desc.md_const);
		return true;
	}
	switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)
	CASE(STRUCT_NONE):
	CASE(STRUCT_OBJECT_OPT): /* Always bound (because it is `none' when NULL) */
	CASE(STRUCT_CSTR_OPT):
	CASE(STRUCT_CSTR_EMPTY):
	CASE(STRUCT_STRING):
	CASE(STRUCT_CHAR):
	CASE(STRUCT_BOOL8):
	CASE(STRUCT_BOOL16):
	CASE(STRUCT_BOOL32):
	CASE(STRUCT_BOOL64):
	CASE(STRUCT_BOOLBIT0):
	CASE(STRUCT_BOOLBIT1):
	CASE(STRUCT_BOOLBIT2):
	CASE(STRUCT_BOOLBIT3):
	CASE(STRUCT_BOOLBIT4):
	CASE(STRUCT_BOOLBIT5):
	CASE(STRUCT_BOOLBIT6):
	CASE(STRUCT_BOOLBIT7):
	CASE(STRUCT_FLOAT):
	CASE(STRUCT_DOUBLE):
	CASE(STRUCT_LDOUBLE):
	CASE(STRUCT_UNSIGNED|STRUCT_INT8):
	CASE(STRUCT_INT8):
	CASE(STRUCT_UNSIGNED|STRUCT_INT16):
	CASE(STRUCT_INT16):
	CASE(STRUCT_UNSIGNED|STRUCT_INT32):
	CASE(STRUCT_INT32):
	CASE(STRUCT_UNSIGNED|STRUCT_INT64):
	CASE(STRUCT_INT64):
	CASE(STRUCT_UNSIGNED|STRUCT_INT128):
	CASE(STRUCT_INT128):
	CASE(STRUCT_WOBJECT_OPT):
		return true;

	CASE(STRUCT_VARIANT):
		return Dee_variant_isbound(&FIELD(struct Dee_variant));

	CASE(STRUCT_WOBJECT):
		return Dee_weakref_bound(&FIELD(struct Dee_weakref));

	CASE(STRUCT_OBJECT):
	CASE(STRUCT_CSTR):
		return FIELD(void *) != NULL;

#undef CASE
	default: break;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_type_member_set_impl(struct type_member const *desc,
                         DeeObject *self, DeeObject *value) {
	switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_ATOMIC | STRUCT_CONST)) {
#define WRITE(dst, src) atomic_write(&dst, src)

	case STRUCT_WOBJECT_OPT:
		if (DeeNone_Check(value)) {
			Dee_weakref_clear(&FIELD(struct Dee_weakref));
			return 0;
		}
		ATTR_FALLTHROUGH
	case STRUCT_WOBJECT:
		if unlikely(!Dee_weakref_set(&FIELD(struct Dee_weakref), value))
			return err_cannot_weak_reference(value);
		return 0;

	case STRUCT_CHAR: {
		char chr_value;
		if (DeeString_Check(value)) {
			if unlikely(DeeString_WLEN(self) != 1)
				return err_expected_single_character_string(Dee_AsObject(self));
			chr_value = (char)DeeString_WSTR(value)[0];
		} else {
			if (DeeObject_AsChar(value, &chr_value))
				goto err;
		}
		WRITE(FIELD(char), chr_value);
	}	return 0;

	case STRUCT_VARIANT:
		Dee_variant_setobject(&FIELD(struct Dee_variant), value);
		return 0;

	case STRUCT_BOOL8:
	case STRUCT_BOOL16:
	case STRUCT_BOOL32:
	case STRUCT_BOOL64: {
		int boolval = DeeObject_Bool(value);
		if unlikely(boolval < 0)
			goto err;
		boolval = !!boolval;
		switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_ATOMIC | STRUCT_CONST)) {
		case STRUCT_BOOL8:  FIELD(uint8_t)  = (uint8_t )(unsigned int)boolval; break;
		case STRUCT_BOOL16: FIELD(uint16_t) = (uint16_t)(unsigned int)boolval; break;
		case STRUCT_BOOL32: FIELD(uint32_t) = (uint32_t)(unsigned int)boolval; break;
		case STRUCT_BOOL64: FIELD(uint64_t) = (uint64_t)(unsigned int)boolval; break;
		default: __builtin_unreachable();
		}
		return 0;
	}

	case STRUCT_BOOLBIT0:
	case STRUCT_BOOLBIT1:
	case STRUCT_BOOLBIT2:
	case STRUCT_BOOLBIT3:
	case STRUCT_BOOLBIT4:
	case STRUCT_BOOLBIT5:
	case STRUCT_BOOLBIT6:
	case STRUCT_BOOLBIT7: {
		int boolval;
		uint8_t mask, *pfield;
		boolval = DeeObject_Bool(value);
		if unlikely(boolval < 0)
			goto err;
		mask = STRUCT_BOOLBITMASK(desc->m_desc.md_field.mdf_type & ~(STRUCT_ATOMIC | STRUCT_CONST));
		pfield = &FIELD(uint8_t);
		if (boolval) {
			IF_THREADS(if (desc->m_desc.md_field.mdf_type & STRUCT_ATOMIC) {
				atomic_or(pfield, mask);
			} else) {
				*pfield |= mask;
			}
		} else {
			IF_THREADS(if (desc->m_desc.md_field.mdf_type & STRUCT_ATOMIC) {
				atomic_and(pfield, ~mask);
			} else) {
				*pfield &= ~mask;
			}
		}
		return 0;
	}	break;

	case STRUCT_FLOAT:
	case STRUCT_DOUBLE:
	case STRUCT_LDOUBLE: {
		double data;
		if (DeeObject_AsDouble(value, &data))
			goto err;
		switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_ATOMIC | STRUCT_CONST)) {
		case STRUCT_FLOAT: FIELD(float) = (float)data; break;
		case STRUCT_DOUBLE: FIELD(double) = data; break;
		case STRUCT_LDOUBLE:  FIELD(long double) = (long double)data; break;
		default: __builtin_unreachable();
		}
		return 0;
	}	break;

	{
		union {
			int8_t s8;
			int16_t s16;
			int32_t s32;
			int64_t s64;
			Dee_int128_t s128;
			uint8_t u8;
			uint16_t u16;
			uint32_t u32;
			uint64_t u64;
			Dee_uint128_t u128;
		} data;
	case STRUCT_UNSIGNED | STRUCT_INT8:
		if (DeeObject_AsUInt8(value, &data.u8))
			goto err;
		WRITE(FIELD(uint8_t), data.u8);
		break;

	case STRUCT_INT8:
		if (DeeObject_AsInt8(value, &data.s8))
			goto err;
		WRITE(FIELD(int8_t), data.s8);
		break;

	case STRUCT_UNSIGNED | STRUCT_INT16:
		if (DeeObject_AsUInt16(value, &data.u16))
			goto err;
		WRITE(FIELD(uint16_t), data.u16);
		break;

	case STRUCT_INT16:
		if (DeeObject_AsInt16(value, &data.s16))
			goto err;
		WRITE(FIELD(int16_t), data.s16);
		break;

	case STRUCT_UNSIGNED | STRUCT_INT32:
		if (DeeObject_AsUInt32(value, &data.u32))
			goto err;
		WRITE(FIELD(uint32_t), data.u32);
		break;

	case STRUCT_INT32:
		if (DeeObject_AsInt32(value, &data.s32))
			goto err;
		WRITE(FIELD(int32_t), data.s32);
		break;

	case STRUCT_UNSIGNED | STRUCT_INT64:
		if (DeeObject_AsUInt64(value, &data.u64))
			goto err;
		WRITE(FIELD(uint64_t), data.u64);
		break;

	case STRUCT_INT64:
		if (DeeObject_AsInt64(value, &data.s64))
			goto err;
		WRITE(FIELD(int64_t), data.s64);
		break;

	case STRUCT_UNSIGNED | STRUCT_INT128:
		if (DeeObject_AsUInt128(value, &data.u128))
			goto err;
		IF_THREADS(COMPILER_WRITE_BARRIER());
		FIELD(Dee_uint128_t) = data.u128;
		IF_THREADS(COMPILER_WRITE_BARRIER());
		break;

	case STRUCT_INT128:
		if (DeeObject_AsInt128(value, &data.s128))
			goto err;
		IF_THREADS(COMPILER_WRITE_BARRIER());
		FIELD(Dee_int128_t) = data.s128;
		IF_THREADS(COMPILER_WRITE_BARRIER());
		break;
	}

#undef WRITE
	default: break;
	}
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_type_member_set(struct type_member const *desc,
                    DeeObject *self, DeeObject *value) {
	if (TYPE_MEMBER_ISCONST(desc))
		goto err_cant_access;
	if (desc->m_desc.md_field.mdf_type & STRUCT_CONST)
		goto err_cant_access;
	return Dee_type_member_set_impl(desc, self, value);
err_cant_access:
	return DeeRT_ErrRestrictedMember(self, desc, DeeRT_ATTRIBUTE_ACCESS_SET);
}


PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_type_member_del(struct type_member const *desc, DeeObject *self) {
	if unlikely(TYPE_MEMBER_ISCONST(desc))
		goto cant_access;
	if unlikely(desc->m_desc.md_field.mdf_type & STRUCT_CONST)
		goto cant_access;
	switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_ATOMIC)) {
	case STRUCT_VARIANT:
		Dee_variant_setunbound(&FIELD(struct Dee_variant));
		return 0;
	default: break;
	}
	return Dee_type_member_set_impl(desc, self, Dee_None);
cant_access:
	return DeeRT_ErrRestrictedMember(self, desc, DeeRT_ATTRIBUTE_ACCESS_DEL);
}

#undef IF_THREADS

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_TYPE_MEMBER_C */
