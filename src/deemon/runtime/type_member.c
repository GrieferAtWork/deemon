/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_TYPE_MEMBER_C
#define GUARD_DEEMON_RUNTIME_TYPE_MEMBER_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* strlen() */
#include <deemon/util/atomic.h>

#include <stdarg.h>
#include <stdbool.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"

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


/* Reurns the object-type used to represent a given type-member. */
INTERN WUNUSED NONNULL((1)) DeeTypeObject *DCALL
type_member_typefor(struct type_member const *__restrict self) {
	if (TYPE_MEMBER_ISCONST(self))
		return Dee_TYPE(self->m_const);
	switch (self->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

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


INTERN WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
type_method_enum(DeeTypeObject *__restrict tp_self,
                 struct type_method const *chain,
                 uint16_t flags, denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	flags |= ATTR_PERMGET | ATTR_PERMCALL;
	for (; chain->m_name; ++chain) {
		temp = (*proc)((DeeObject *)tp_self, chain->m_name, chain->m_doc,
		               flags, &DeeObjMethod_Type, arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
type_obmeth_enum(DeeTypeObject *__restrict tp_self,
                 denum_t proc, void *arg) {
	struct type_method const *chain;
	dssize_t temp, result = 0;
	chain = tp_self->tp_methods;
	ASSERT(chain != NULL);
	for (; chain->m_name; ++chain) {
		temp = (*proc)((DeeObject *)tp_self, chain->m_name, chain->m_doc,
		               ATTR_IMEMBER | ATTR_CMEMBER | ATTR_PERMGET | ATTR_PERMCALL | ATTR_WRAPPER,
		               &DeeObjMethod_Type /*&DeeClsMethod_Type*/, arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
type_obprop_enum(DeeTypeObject *__restrict tp_self,
                 denum_t proc, void *arg) {
	struct type_getset const *chain;
	dssize_t temp, result = 0;
	chain = tp_self->tp_getsets;
	ASSERT(chain != NULL);
	for (; chain->gs_name; ++chain) {
		uint16_t perm = ATTR_IMEMBER | ATTR_CMEMBER | ATTR_PROPERTY | ATTR_WRAPPER;
		if (chain->gs_get)
			perm |= ATTR_PERMGET;
		if (chain->gs_del)
			perm |= ATTR_PERMDEL;
		if (chain->gs_set)
			perm |= ATTR_PERMSET;
		temp = (*proc)((DeeObject *)tp_self, chain->gs_name, chain->gs_doc,
		               perm, NULL /*&DeeClsProperty_Type*/, arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
type_obmemb_enum(DeeTypeObject *__restrict tp_self,
                 denum_t proc, void *arg) {
	struct type_member const *chain;
	dssize_t temp, result = 0;
	chain = tp_self->tp_members;
	ASSERT(chain != NULL);
	for (; chain->m_name; ++chain) {
		uint16_t perm = ATTR_IMEMBER | ATTR_CMEMBER | ATTR_PERMGET | ATTR_WRAPPER;
		if (!(chain->m_field.m_type & STRUCT_CONST))
			perm |= ATTR_PERMDEL | ATTR_PERMSET;
		temp = (*proc)((DeeObject *)tp_self, chain->m_name, chain->m_doc,
		               perm, type_member_typefor(chain) /*&DeeClsMember_Type*/, arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
type_getset_enum(DeeTypeObject *__restrict tp_self,
                 struct type_getset const *chain,
                 uint16_t flags, denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	ASSERT(flags & ATTR_PROPERTY);
	for (; chain->gs_name; ++chain) {
		uint16_t perm = flags;
		if (chain->gs_get)
			perm |= ATTR_PERMGET;
		if (chain->gs_del)
			perm |= ATTR_PERMDEL;
		if (chain->gs_set)
			perm |= ATTR_PERMSET;
		temp = (*proc)((DeeObject *)tp_self, chain->gs_name,
		               chain->gs_doc, perm, NULL, arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
type_member_enum(DeeTypeObject *__restrict tp_self,
                 struct type_member const *chain,
                 uint16_t flags, denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	for (; chain->m_name; ++chain) {
		if (TYPE_MEMBER_ISCONST(chain)) {
			temp = (*proc)((DeeObject *)tp_self, chain->m_name, chain->m_doc,
			               flags | ATTR_PERMGET, Dee_TYPE(chain->m_const), arg);
		} else {
			uint16_t perm = flags | ATTR_PERMGET;
			if (!(chain->m_field.m_type & STRUCT_CONST))
				perm |= (ATTR_PERMDEL | ATTR_PERMSET);
			temp = (*proc)((DeeObject *)tp_self, chain->m_name, chain->m_doc,
			               perm, type_member_typefor(chain), arg);
		}
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) DeeTypeObject *DCALL
type_getset_typeof(struct type_getset const *chain,
                   DeeObject *__restrict self) {
	DeeTypeObject *result;
	DeeTypeMRO mro;
	result = DeeTypeMRO_Init(&mro, Dee_TYPE(self));
	do {
		if (result->tp_getsets == chain)
			return result;
	} while ((result = DeeTypeMRO_Next(&mro, result)) != NULL);
	if (DeeType_Check(self)) {
		result = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
		do {
			if (result->tp_class_getsets == chain)
				return result;
		} while ((result = DeeTypeMRO_Next(&mro, result)) != NULL);
	}
	return Dee_TYPE(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeTypeObject *DCALL
type_member_typeof(struct type_member const *chain,
                   DeeObject *__restrict self) {
	DeeTypeObject *result;
	DeeTypeMRO mro;
	result = DeeTypeMRO_Init(&mro, Dee_TYPE(self));
	do {
		if (result->tp_members == chain)
			return result;
	} while ((result = DeeTypeMRO_Next(&mro, result)) != NULL);
	if (DeeType_Check(self)) {
		result = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
		do {
			if (result->tp_class_members == chain)
				return result;
		} while ((result = DeeTypeMRO_Next(&mro, result)) != NULL);
	}
	return Dee_TYPE(self);
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
		return (*(dkwobjmethod_t)desc->m_func)(argv[0], argc - 1, argv + 1, NULL);
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
		return (*(dkwobjmethod_t)desc->m_func)(argv[0], argc - 1, argv + 1, kw);
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
	thisarg = Dee_VPPackf((char const **)&format, (struct va_list_struct *)VALIST_ADDR(args));
	if unlikely(!thisarg) {
		Dee_VPPackf_Cleanup(format, ((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
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
	err_cant_access_attribute_string(type_getset_typeof(desc, self),
	                                 desc->gs_name, ATTR_ACCESS_GET);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
type_getset_del(struct type_getset const *desc,
                DeeObject *__restrict self) {
	if likely(desc->gs_del)
		return (*desc->gs_del)(self);
	return err_cant_access_attribute_string(type_getset_typeof(desc, self),
	                                        desc->gs_name, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
type_getset_set(struct type_getset const *desc,
                DeeObject *self, DeeObject *value) {
	if likely(desc->gs_set)
		return (*desc->gs_set)(self, value);
	return err_cant_access_attribute_string(type_getset_typeof(desc, self),
	                                        desc->gs_name, ATTR_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obprop_call(DeeTypeObject *cls_type,
                 struct type_getset const *desc,
                 size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	if unlikely(!desc->gs_get)
		goto err_unbound;
	if unlikely(DeeArg_Unpack(argc, argv, "o:get", &thisarg))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err;
	return (*desc->gs_get)(thisarg);
err_unbound:
	err_cant_access_attribute_string(cls_type, desc->gs_name, ATTR_ACCESS_GET);
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
	err_cant_access_attribute_string(cls_type, desc->gs_name, ATTR_ACCESS_GET);
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
	err_cant_access_attribute_string(cls_type, desc->gs_name, ATTR_ACCESS_GET);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_obmemb_call(DeeTypeObject *cls_type,
                 struct type_member const *desc,
                 size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	if (DeeArg_Unpack(argc, argv, "o:get", &thisarg))
		goto err;
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


#define FIELD(T)  (*(T *)((uintptr_t)self + desc->m_field.m_offset))

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_type_member_get(struct type_member const *desc,
                    DeeObject *__restrict self) {
	if (TYPE_MEMBER_ISCONST(desc)) {
		ASSERT_OBJECT(desc->m_const);
		return_reference_(desc->m_const);
	}
	switch (desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)

	CASE(STRUCT_NONE):
ret_none:
		return_none;

	CASE(STRUCT_WOBJECT):
	CASE(STRUCT_WOBJECT_OPT): {
		DeeObject *ob;
		ob = Dee_weakref_lock(&FIELD(struct weakref));
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
			if (!(desc->m_field.m_type & (STRUCT_OBJECT_OPT & ~(STRUCT_OBJECT))))
				goto is_unbound;
			ob = Dee_None;
		}
		Dee_Incref(ob);
		return ob;
	}	break;

	CASE(STRUCT_CSTR):
	CASE(STRUCT_CSTR_OPT):
	CASE(STRUCT_CSTR_EMPTY): {
		char const *cstr;
		cstr = FIELD(char *);
		if unlikely(!cstr) {
			switch (desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

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
		return_bool((FIELD(uint8_t) & STRUCT_BOOLBITMASK(desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC))) != 0);
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
	err_unbound_attribute_string(type_member_typeof(desc, self),
	                             desc->m_name);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) bool DCALL
Dee_type_member_bound(struct type_member const *desc,
                      DeeObject *__restrict self) {
	if (TYPE_MEMBER_ISCONST(desc)) {
		ASSERT_OBJECT(desc->m_const);
		return true;
	}
	switch (desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

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

	CASE(STRUCT_WOBJECT):
		return Dee_weakref_bound(&FIELD(struct weakref));

	CASE(STRUCT_OBJECT):
	CASE(STRUCT_CSTR):
		return FIELD(void *) != NULL;

#undef CASE
	default: break;
	}
	return false;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_type_member_set(struct type_member const *desc,
                    DeeObject *self, DeeObject *value) {
	if (TYPE_MEMBER_ISCONST(desc))
		goto cant_access;
	if (desc->m_field.m_type & STRUCT_CONST)
		goto cant_access;
	switch (desc->m_field.m_type & ~(STRUCT_ATOMIC)) {
#define WRITE(dst, src) atomic_write(&dst, src)

	case STRUCT_WOBJECT_OPT:
		if (DeeNone_Check(value)) {
			Dee_weakref_clear(&FIELD(struct weakref));
			return 0;
		}
		ATTR_FALLTHROUGH
	case STRUCT_WOBJECT:
		if unlikely(!Dee_weakref_set(&FIELD(struct weakref), value))
			return err_cannot_weak_reference(value);
		return 0;

	case STRUCT_CHAR: {
		char chr_value;
		if (DeeString_Check(value)) {
			if unlikely(DeeString_WLEN(self) != 1)
				return err_expected_single_character_string((DeeObject *)self);
			chr_value = (char)DeeString_WSTR(value)[0];
		} else {
			if (DeeObject_AsChar(value, &chr_value))
				goto err;
		}
		WRITE(FIELD(char), chr_value);
	}	return 0;

	case STRUCT_BOOL8:
	case STRUCT_BOOL16:
	case STRUCT_BOOL32:
	case STRUCT_BOOL64: {
		int boolval;
		boolval = DeeObject_Bool(value);
		if unlikely(boolval < 0)
			goto err;
		boolval = !!boolval;
		switch (desc->m_field.m_type & ~(STRUCT_ATOMIC)) {
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
		mask = STRUCT_BOOLBITMASK(desc->m_field.m_type & ~STRUCT_ATOMIC);
		pfield = &FIELD(uint8_t);
		if (boolval) {
			IF_THREADS(if (desc->m_field.m_type & STRUCT_ATOMIC) {
				atomic_or(pfield, mask);
			} else) {
				*pfield |= mask;
			}
		} else {
			IF_THREADS(if (desc->m_field.m_type & STRUCT_ATOMIC) {
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
		switch (desc->m_field.m_type & ~(STRUCT_ATOMIC)) {

		case STRUCT_FLOAT:
			FIELD(float) = (float)data;
			break;

		case STRUCT_DOUBLE:
			FIELD(double) = data;
			break;

		default:
			FIELD(long double) = (long double)data;
			break;
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
cant_access:
	err_cant_access_attribute_string(type_member_typeof(desc, self),
	                                 desc->m_name, ATTR_ACCESS_SET);
err:
	return -1;
}

#undef IF_THREADS

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_TYPE_MEMBER_C */
