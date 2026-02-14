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
#ifndef GUARD_DEEMON_OBJECTS_ATTRIBUTE_C
#define GUARD_DEEMON_OBJECTS_ATTRIBUTE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_*, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack* */
#include <deemon/attribute.h>          /* DeeAttributeObject, DeeEnumAttrIteratorObject, DeeEnumAttrObject */
#include <deemon/bool.h>               /* return_bool, return_false, return_true */
#include <deemon/computed-operators.h>
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_Printf */
#include <deemon/mro.h>                /* DeeObject_FindAttr, DeeObject_IterAttr, Dee_ATTRINFO_CUSTOM, Dee_ATTRPERM_F_*, Dee_ITERATTR_DEFAULT_BUFSIZE, Dee_SIZEOF_ATTRPERM_T, Dee_attrdesc, Dee_attrdesc_*, Dee_attrhint, Dee_attrhint_matches, Dee_attrinfo_typeof, Dee_attriter_*, Dee_attrperm_t, Dee_attrspec */
#include <deemon/none.h>               /* DeeNone_Check, DeeNone_NewRef, Dee_None, return_none */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_ISBOUND, Dee_BOUND_ISERR, Dee_COMPARE_*, Dee_Decref, Dee_Incref, Dee_TYPE, Dee_XDecref, Dee_XIncref, Dee_formatprinter_t, Dee_hash_t, Dee_return_DeeObject_TryCompareEq_if_ne, Dee_ssize_t, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/system-features.h>    /* DeeSystem_DEFINE_strcmp, memcpy, memset */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_STRUCT_INTEGER, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_Visit, Dee_XVisit, METHOD_FCONSTCALL, METHOD_FNOREFESCAPE, OPERATOR_*, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/hash.h>          /* Dee_HashCombine, Dee_HashStr */

#include <hybrid/typecore.h> /* __BYTE_TYPE__, __SHIFT_TYPE__ */

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#include <stddef.h> /* NULL, offsetof, ptrdiff_t, size_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef shift_t
#define shift_t __SHIFT_TYPE__
#undef container_of
#define container_of COMPILER_CONTAINER_OF

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */


typedef DeeAttributeObject /*       */ Attr;
typedef DeeEnumAttrObject /*        */ EnumAttr;
typedef DeeEnumAttrIteratorObject /**/ EnumAttrIter;

PRIVATE char const attr_permchars[] = {
	/* [FFS(Dee_ATTRPERM_F_CANGET) - 1]   = */ 'g',
	/* [FFS(Dee_ATTRPERM_F_CANDEL) - 1]   = */ 'd',
	/* [FFS(Dee_ATTRPERM_F_CANSET) - 1]   = */ 's',
	/* [FFS(Dee_ATTRPERM_F_CANCALL) - 1]  = */ 'f',
	/* [FFS(Dee_ATTRPERM_F_IMEMBER) - 1]  = */ 'i',
	/* [FFS(Dee_ATTRPERM_F_CMEMBER) - 1]  = */ 'c',
	/* [FFS(Dee_ATTRPERM_F_PRIVATE) - 1]  = */ 'h',
	/* [FFS(Dee_ATTRPERM_F_PROPERTY) - 1] = */ 'p',
	/* [FFS(Dee_ATTRPERM_F_WRAPPER) - 1]  = */ 'w',
};


PRIVATE NONNULL((1)) void DCALL
attr_fini(Attr *__restrict self) {
	Dee_Decref(self->a_desc.ad_info.ai_decl);
	Dee_attrdesc_fini(&self->a_desc);
}

PRIVATE NONNULL((1, 2)) void DCALL
attr_visit(Attr *__restrict self, Dee_visit_t proc, void *arg) {
	/* No need to visit the name/doc (strings don't need to be visited) */
	Dee_Visit(self->a_desc.ad_info.ai_decl);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
attr_serialize(Attr *__restrict self,
               DeeSerial *__restrict writer,
               Dee_seraddr_t addr) {
	Attr *out = DeeSerial_Addr2Mem(writer, addr, Attr);
	memcpy(&out->a_desc, &self->a_desc, sizeof(self->a_desc));
	if (DeeSerial_PutObject(writer, addr + offsetof(Attr, a_desc.ad_info.ai_decl), self->a_desc.ad_info.ai_decl))
		goto err;
	ASSERT(self->a_desc.ad_name);
	if (self->a_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *ob = container_of(self->a_desc.ad_name, DeeStringObject, s_str);
		if (DeeSerial_PutObjectEx(writer, addr + offsetof(Attr, a_desc.ad_name), ob, offsetof(DeeStringObject, s_str)))
			goto err;
	} else {
		char const *name = self->a_desc.ad_name;
		if (DeeSerial_PutPointer(writer, addr + offsetof(Attr, a_desc.ad_name), name))
			goto err;
	}
	if (self->a_desc.ad_doc) {
		if (self->a_desc.ad_perm & Dee_ATTRPERM_F_DOCOBJ) {
			DeeStringObject *ob = container_of(self->a_desc.ad_doc, DeeStringObject, s_str);
			if (DeeSerial_PutObjectEx(writer, addr + offsetof(Attr, a_desc.ad_doc), ob, offsetof(DeeStringObject, s_str)))
				goto err;
		} else {
			char const *doc = self->a_desc.ad_doc;
			if (DeeSerial_PutPointer(writer, addr + offsetof(Attr, a_desc.ad_doc), doc))
				goto err;
		}
	}
	return DeeSerial_XPutObject(writer, addr + offsetof(Attr, a_desc.ad_type),
	                            self->a_desc.ad_type);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
attr_hash(Attr *__restrict self) {
	Dee_hash_t result = self->a_desc.ad_perm;
	result = Dee_HashCombine(result, DeeObject_Hash(self->a_desc.ad_info.ai_decl));
	if (self->a_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *string;
		string = COMPILER_CONTAINER_OF(self->a_desc.ad_name, DeeStringObject, s_str);
		result = Dee_HashCombine(result, DeeString_Hash(string));
	} else {
		result = Dee_HashCombine(result, Dee_HashStr(self->a_desc.ad_name));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
attr_compare_eq_impl(Attr *lhs, Attr *rhs) {
	if ((lhs->a_desc.ad_perm & ~(Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_DOCOBJ)) !=
	    (rhs->a_desc.ad_perm & ~(Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_DOCOBJ)))
		goto nope;
	if (lhs->a_desc.ad_info.ai_decl != rhs->a_desc.ad_info.ai_decl)
		Dee_return_DeeObject_TryCompareEq_if_ne(lhs->a_desc.ad_info.ai_decl, rhs->a_desc.ad_info.ai_decl);
	if (lhs->a_desc.ad_name == rhs->a_desc.ad_name)
		goto yup;
	if ((lhs->a_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) &&
	    (rhs->a_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)) {
		DeeStringObject *lhs_name = COMPILER_CONTAINER_OF(lhs->a_desc.ad_name, DeeStringObject, s_str);
		DeeStringObject *rhs_name = COMPILER_CONTAINER_OF(rhs->a_desc.ad_name, DeeStringObject, s_str);
		if (DeeString_Hash(lhs_name) != DeeString_Hash(rhs_name))
			goto nope;
		if (!DeeString_EqualsSTR(lhs_name, rhs_name))
			goto nope;
	} else {
		if (strcmp(lhs->a_desc.ad_name, rhs->a_desc.ad_name) != 0)
			goto nope;
	}
yup:
	return Dee_COMPARE_EQ;
nope:
	return Dee_COMPARE_NE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
attr_compare_eq(Attr *lhs, Attr *rhs) {
	if (DeeObject_AssertType(rhs, &DeeAttribute_Type))
		goto err;
	return attr_compare_eq_impl(lhs, rhs);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
attr_trycompare_eq(Attr *lhs, Attr *rhs) {
	if (!DeeObject_InstanceOf(rhs, &DeeAttribute_Type))
		return Dee_COMPARE_NE;
	return attr_compare_eq_impl(lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
attr_getname(Attr *__restrict self) {
	char const *namestr = self->a_desc.ad_name;
	if (self->a_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *result = COMPILER_CONTAINER_OF(namestr, DeeStringObject, s_str);
		Dee_Incref(result);
		return result;
	}
	return (DREF DeeStringObject *)DeeString_New(namestr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
attr_getdoc(Attr *__restrict self) {
	char const *docstr = self->a_desc.ad_doc;
	if (!docstr)
		return (DREF DeeStringObject *)DeeNone_NewRef();
	if (self->a_desc.ad_perm & Dee_ATTRPERM_F_DOCOBJ) {
		DeeStringObject *result = COMPILER_CONTAINER_OF(docstr, DeeStringObject, s_str);
		Dee_Incref(result);
		return result;
	}
	return (DREF DeeStringObject *)DeeString_New(docstr);
}


#define ATTR_PERMMASK ((1 << COMPILER_LENOF(attr_permchars)) - 1)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_getperm(Attr *__restrict self) {
	DREF DeeObject *result;
	Dee_attrperm_t mask = self->a_desc.ad_perm & ATTR_PERMMASK;
	unsigned int num_flags = 0;
	char *dst;
	while (mask) {
		if (mask & 1)
			++num_flags;
		mask >>= 1;
	}
	result = DeeString_NewBuffer(num_flags);
	if unlikely(!result)
		goto done;
	dst  = DeeString_GetBuffer(result);
	mask = self->a_desc.ad_perm & ATTR_PERMMASK;
	num_flags = 0;
	while (mask) {
		if (mask & 1)
			*dst++ = attr_permchars[num_flags];
		mask >>= 1;
		++num_flags;
	}
	ASSERT(dst == DeeString_END(result));
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_getattrtype(Attr *__restrict self) {
	DREF DeeTypeObject *result;
	result = Dee_attrdesc_typeof(&self->a_desc);
	if (result)
		return Dee_AsObject(result);
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_getabiattrtype(Attr *__restrict self) {
	DREF DeeTypeObject *result;
	result = Dee_attrinfo_typeof(&self->a_desc.ad_info);
	if (result)
		return Dee_AsObject(result);
	return_none;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
attr_print(Attr *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Attribute %k.%s>",
	                        self->a_desc.ad_info.ai_decl,
	                        self->a_desc.ad_name);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
attr_printrepr_impl(struct Dee_attrdesc const *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	char perm_str[COMPILER_LENOF(attr_permchars) + 1];
	char *perm_ptr = perm_str;
	shift_t perm_index;
	Dee_attrperm_t perm_mask = self->ad_perm & ATTR_PERMMASK;
	for (perm_index = 0; perm_mask; perm_mask >>= 1, ++perm_index) {
		if (perm_mask & 1)
			*perm_ptr++ = attr_permchars[perm_index];
	}
	*perm_ptr = '\0';
	result = DeeFormat_Printf(printer, arg,
	                          "Attribute(%r, %q",
	                          self->ad_info.ai_decl,
	                          self->ad_name);
	if unlikely(result < 0)
		goto done;
	if (self->ad_doc) {
		temp = DeeFormat_Printf(printer, arg, ", doc: %q", self->ad_doc);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (*perm_str) {
		temp = DeeFormat_Printf(printer, arg, ", perm: %q)", perm_str);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
attr_printrepr(Attr *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return attr_printrepr_impl(&self->a_desc, printer, arg);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_to_attrperms(char const *__restrict str,
                    Dee_attrperm_t *__restrict p_result) {
	*p_result = 0;
	while (*str) {
		char ch = *str++;
		shift_t i;
		for (i = 0; attr_permchars[i] != ch; ++i) {
			if unlikely(i >= COMPILER_LENOF(attr_permchars)) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Unknown attribute flag %:1q",
				                str - 1);
				goto err;
			}
		}
		*p_result |= (Dee_attrperm_t)1 << i;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
attr_init_kw(DeeAttributeObject *__restrict self,
             size_t argc, DeeObject *const *argv,
             DeeObject *kw) {
	Dee_attrperm_t perm;
	struct {
		DeeTypeObject   *decl;
		DeeStringObject *name;
		DeeStringObject *doc;
		DeeObject       *perm;
		DeeTypeObject   *attrtype;
	} args;
	args.doc      = NULL;
	args.perm     = Dee_None;
	args.attrtype = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__decl_name_doc_perm_attrtype,
	                          "oo|ooo:Attribute", &args))
		goto err;
	if (DeeObject_AssertType(args.decl, &DeeType_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	if (args.doc && DeeObject_AssertTypeExact(args.doc, &DeeString_Type))
		goto err;
	if unlikely(!args.decl->tp_attr) {
		DeeError_Throwf(&DeeError_TypeError,
		                "Type %k does not implement attribute operators",
		                args.decl->tp_attr);
		goto err;
	}

	/* Load attribute permissions. */
	if (DeeNone_Check(args.perm)) {
		perm = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_PROPERTY;
		if (args.decl->tp_attr->tp_getattr)
			perm |= Dee_ATTRPERM_F_CANGET;
		if (args.decl->tp_attr->tp_delattr)
			perm |= Dee_ATTRPERM_F_CANDEL;
		if (args.decl->tp_attr->tp_setattr)
			perm |= Dee_ATTRPERM_F_CANSET;
	} else if (DeeString_Check(args.perm)) {
		if (string_to_attrperms(DeeString_STR(args.perm), &perm))
			goto err;
	} else {
		if (DeeObject_AsUIntX(args.perm, &perm))
			goto err;
	}

	self->a_desc.ad_type = NULL;
	if (args.attrtype) {
		if (DeeObject_AssertType(args.attrtype, &DeeType_Type))
			goto err;
		self->a_desc.ad_type = args.attrtype;
		Dee_Incref(args.attrtype);
	}
	Dee_Incref(args.name);
	self->a_desc.ad_perm = perm | Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_DOCOBJ;
	self->a_desc.ad_name = DeeString_STR(args.name);
	self->a_desc.ad_doc  = NULL;
	if (args.doc) {
		Dee_Incref(args.doc);
		self->a_desc.ad_doc = DeeString_STR(args.doc);
	}
	Dee_Incref(args.decl);
	self->a_desc.ad_info.ai_decl = (DeeObject *)args.decl;
	self->a_desc.ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
	self->a_desc.ad_info.ai_value.v_custom = args.decl->tp_attr;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
attribute_exists(DeeTypeObject *__restrict UNUSED(self), size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	int status;
	struct Dee_attrspec specs;
	struct Dee_attrdesc result;
	struct {
		DeeObject       *ob;
		DeeStringObject *name;
		DeeObject       *perm;
		DeeObject       *permset;
		DeeObject       *decl;
	} args;
	args.perm    = Dee_None;
	args.permset = Dee_None;
	args.decl    = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__ob_name_perm_permset_decl,
	                          "oo|ooo:exists", &args))
		goto err;
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	specs.as_name = DeeString_STR(args.name);
	specs.as_hash = DeeString_Hash(args.name);
	specs.as_decl = args.decl;
	if (DeeNone_Check(args.perm)) {
		specs.as_perm_mask = 0;
	} else if (DeeString_Check(args.perm)) {
		if (string_to_attrperms(DeeString_STR(args.perm), &specs.as_perm_mask))
			goto err;
	} else {
		if (DeeObject_AsUIntX(args.perm, &specs.as_perm_mask))
			goto err;
	}
	if (DeeNone_Check(args.permset)) {
		specs.as_perm_value = specs.as_perm_mask;
	} else if (DeeString_Check(args.permset)) {
		if (string_to_attrperms(DeeString_STR(args.permset), &specs.as_perm_value))
			goto err;
	} else {
		if (DeeObject_AsUIntX(args.permset, &specs.as_perm_value))
			goto err;
	}
	status = DeeObject_FindAttr(Dee_TYPE(args.ob), args.ob, &specs, &result);
	if unlikely(status < 0)
		goto err;
	if (status > 0)
		return_false;
	Dee_attrdesc_fini(&result);
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF Attr *DCALL
attribute_lookup(DeeTypeObject *__restrict UNUSED(self), size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	int status;
	DREF Attr *result;
	struct Dee_attrspec specs;
	struct {
		DeeObject       *ob;
		DeeStringObject *name;
		DeeObject       *perm;
		DeeObject       *permset;
		DeeObject       *decl;
	} args;
	args.perm    = Dee_None;
	args.permset = Dee_None;
	args.decl    = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__ob_name_perm_permset_decl,
	                          "oo|ooo:lookup", &args))
		goto err;
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	specs.as_name = DeeString_STR(args.name);
	specs.as_hash = DeeString_Hash(args.name);
	specs.as_decl = args.decl;
	if (DeeNone_Check(args.perm)) {
		specs.as_perm_mask = 0;
	} else if (DeeString_Check(args.perm)) {
		if (string_to_attrperms(DeeString_STR(args.perm), &specs.as_perm_mask))
			goto err;
	} else {
		if (DeeObject_AsUIntX(args.perm, &specs.as_perm_mask))
			goto err;
	}
	if (DeeNone_Check(args.permset)) {
		specs.as_perm_value = specs.as_perm_mask;
	} else if (DeeString_Check(args.permset)) {
		if (string_to_attrperms(DeeString_STR(args.permset), &specs.as_perm_value))
			goto err;
	} else {
		if (DeeObject_AsUIntX(args.permset, &specs.as_perm_value))
			goto err;
	}
	result = DeeObject_MALLOC(Attr);
	if unlikely(!result)
		goto err;
	status = DeeObject_FindAttr(Dee_TYPE(args.ob), args.ob, &specs, &result->a_desc);
	if unlikely(status < 0)
		goto err_r;
	if (status > 0) {
		DeeObject_FREE(result);
		return (DREF Attr *)DeeNone_NewRef();
	}
	Dee_Incref(result->a_desc.ad_info.ai_decl);
	DeeObject_Init(result, &DeeAttribute_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_callget(Attr *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	DeeArg_Unpack1(err, argc, argv, "callget", &thisarg);
	return Dee_attrdesc_callget(&self->a_desc, thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_callbound(Attr *__restrict self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *thisarg;
	DeeArg_Unpack1(err, argc, argv, "callbound", &thisarg);
	result = Dee_attrdesc_callbound(&self->a_desc, thisarg);
	if unlikely(Dee_BOUND_ISERR(result))
		goto err;
	return_bool(Dee_BOUND_ISBOUND(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_calldel(Attr *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	DeeArg_Unpack1(err, argc, argv, "calldel", &thisarg);
	if unlikely(Dee_attrdesc_calldel(&self->a_desc, thisarg))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_callset(Attr *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg, *value;
	DeeArg_Unpack2(err, argc, argv, "callset", &thisarg, &value);
	if unlikely(Dee_attrdesc_callset(&self->a_desc, thisarg, value))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_callcall(Attr *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if unlikely(argc == 0) {
		err_invalid_argc_va("callcall", 0, 1);
		return NULL;
	}
	return Dee_attrdesc_callcall(&self->a_desc, argv[0], argc - 1, argv + 1, kw);
}


PRIVATE struct type_cmp attr_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&attr_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&attr_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&attr_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE struct type_member tpconst attr_members[] = {
	TYPE_MEMBER_FIELD_DOC("decl", STRUCT_OBJECT_AB, offsetof(Attr, a_desc.ad_info.ai_decl),
	                      "->?X2?DModule?DType\n"
	                      "The ?DType or ?DModule that is declaring this ?.. "
	                      /**/ "In the case of custom attribute operators, this is the "
	                      /**/ "type that is declaring those custom attribute operators."),
	TYPE_MEMBER_BITFIELD_DOC("canget", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_CANGET,
	                         "Check if the ?. has a way of being read from"),
	TYPE_MEMBER_BITFIELD_DOC("candel", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_CANDEL,
	                         "Check if the ?. has a way of being deleted"),
	TYPE_MEMBER_BITFIELD_DOC("canset", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_CANSET,
	                         "Check if the ?. has a way of being written to"),
	TYPE_MEMBER_BITFIELD_DOC("cancall", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_CANCALL,
	                         "Returns ?t if the ?. is intended to be called as a function. "
	                         /**/ "Note that this feature alone does not meant that the ?. really can, or "
	                         /**/ "cannot be called, only that calling it as a function might be the intended use."),
	TYPE_MEMBER_BITFIELD_DOC("isprivate", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_PRIVATE,
	                         "Check if the ?. is considered to be private\n"
	                         "Private attributes only appear in user-classes, prohibiting access to only thiscall "
	                         /**/ "functions with a this-argument that is an instance of the declaring class."),
	TYPE_MEMBER_BITFIELD_DOC("isproperty", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_PROPERTY,
	                         "Check if the ?. is property-like, meaning that access by "
	                         /**/ "reading, deletion, or writing causes unpredictable side-effects"),
	TYPE_MEMBER_BITFIELD_DOC("iswrapper", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_WRAPPER,
	                         "Check if the ?. is accessed from the implementing type, which "
	                         /**/ "exposes it as a wrapper for an instance member (e.g. ${string.find} is an unbound "
	                         /**/ "wrapper (aka. ${Attribute(string, \"find\").iswrapper == true}) for the instance function, "
	                         /**/ "member or property that would be bound in ${\"foo\".find} (aka. "
	                         /**/ "${Attribute(\"foo\", \"find\").iswrapper == false}))"),
	TYPE_MEMBER_BITFIELD_DOC("isinstance", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_IMEMBER,
	                         "Check if accessing this ?. requires an instance of the declaring object "
	                         /**/ "?#decl, rather than being an ?. of the declaring object ?#decl itself.\n"
	                         "Note that practically all attributes, such as member functions, are available as both "
	                         /**/ "instance and class attributes, while in other cases an ?. will evaluate to different "
	                         /**/ "objects depending on being invoked on a class or an instance (such as ?Aisreg?Eposix:stat)"),
	TYPE_MEMBER_BITFIELD_DOC("isclass", STRUCT_CONST, Attr, a_desc.ad_perm, Dee_ATTRPERM_F_CMEMBER,
	                         "Check if access to this ?. must be made though the declaring type ?#decl.\n"
	                         "To test if an ?. can only be accessed through an instance, use ?#isinstance instead"),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst attr_methods[] = {
	TYPE_METHOD_F("callget", &attr_callget, METHOD_FNOREFESCAPE,
	              "(thisarg)->\n"
	              "#tTypeError{The given @thisarg does not implement @this attribute}\n"
	              "#tAttributeError{?#canget is !f}\n"
	              "Invoke the get-operator with @this attribute"),
	TYPE_METHOD_F("callbound", &attr_callbound, METHOD_FNOREFESCAPE,
	              "(thisarg)->?Dbool\n"
	              "#tTypeError{The given @thisarg does not implement @this attribute}\n"
	              "#tAttributeError{?#canget is !f}\n"
	              "Invoke the bound-operator with @this attribute"),
	TYPE_METHOD_F("calldel", &attr_calldel, METHOD_FNOREFESCAPE,
	              "(thisarg)\n"
	              "#tTypeError{The given @thisarg does not implement @this attribute}\n"
	              "#tAttributeError{?#candel is !f}\n"
	              "Invoke the del-operator with @this attribute"),
	TYPE_METHOD_F("callset", &attr_callset, METHOD_FNOREFESCAPE,
	              "(thisarg,value)\n"
	              "#tTypeError{The given @thisarg does not implement @this attribute}\n"
	              "#tAttributeError{?#canset is !f}\n"
	              "Invoke the set-operator with @this attribute"),
	TYPE_KWMETHOD_F("callcall", &attr_callcall, METHOD_FNOREFESCAPE,
	                "(thisarg,args!,kwds!!)\n"
	                "#tTypeError{The given @thisarg does not implement @this attribute}\n"
	                "#tAttributeError{?#cancall and ?#canget is !f}\n"
	                "Invoke the call-operator with @this attribute, "
	                /**/ "or call ?#callget and invoke the result"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst attr_getsets[] = {
	TYPE_GETTER_AB_F("name", &attr_getname, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "The name of this ?."),
	TYPE_GETTER_AB_F("doc", &attr_getdoc, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "The documentation string of this ?., or ?N when no documentation is present"),
	TYPE_GETTER_AB_F("perm", &attr_getperm, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Return a set of characters describing the flags of @this ?.:\n"
	                 "#T{Character|Mnemonic|Field|Flag description~"
	                 /**/ "$\"g\"|get|?#canget|The ?. has a way of being read from&"
	                 /**/ "$\"d\"|del|?#candel|The ?. has a way of being deleted&"
	                 /**/ "$\"s\"|set|?#canset|The ?. has a way of being written to&"
	                 /**/ "$\"f\"|function|?#cancall|The ?. is intended to be called as a function&"
	                 /**/ "$\"i\"|instance|?#isinstance|The ?. requires an instance of the declaring object&"
	                 /**/ "$\"c\"|class|?#isclass|The ?. is accessed though the declaring type ?#decl&"
	                 /**/ "$\"h\"|hidden|?#isprivate|The ?. is considered to be private&"
	                 /**/ "$\"p\"|property|?#isproperty|The ?. is property-like&"
	                 /**/ "$\"w\"|wrapper|?#iswrapper|The ?. is provided by the type as a class member that wraps around an instance member"
	                 "}"),
	TYPE_GETTER_AB_F("attrtype", &attr_getattrtype, METHOD_FNOREFESCAPE,
	                 "->?X2?DType?N\n"
	                 "The type of the attribute's value when accessed, if this "
	                 /**/ "can be determined via means unrelated to ?#doc.\n"
	                 "The attribute type returned here also has special handling "
	                 /**/ "to (in the case of a type attribute) return the attribute's "
	                 /**/ "instance-typing, rather than its class-typing (s.a. ?#abiattrtype):\n"
	                 "${"
	                 /**/ "class MyClass {\n"
	                 /**/ "	this = default;\n"
	                 /**/ "	public member myMember;\n"
	                 /**/ "}\n"
	                 /**/ "\\\n"
	                 /**/ "local inst = MyClass();\n"
	                 /**/ "local cAttr = Attribute.lookup(MyClass, \"myMember\");\n"
	                 /**/ "local iAttr = Attribute.lookup(inst, \"myMember\");\n"
	                 /**/ "assert cAttr.decl === MyClass;\n"
	                 /**/ "assert cAttr.attrtype is none;\n"
	                 /**/ "assert cAttr.abiattrtype === rt.InstanceMember;\n"
	                 /**/ "assert iAttr.decl === MyClass;\n"
	                 /**/ "assert iAttr.attrtype is none;\n"
	                 /**/ "assert iAttr.abiattrtype is none;"
	                 "}"),
	TYPE_GETTER_AB_F("abiattrtype", &attr_getabiattrtype, METHOD_FNOREFESCAPE,
	                 "->?X2?DType?N\n"
	                 "Similar to ?#attrtype, but ignores the #Cattrtype argument given to the constructor, "
	                 /**/ "and does #Bnot do special handling for instance attributes to return their instance "
	                 /**/ "typing when accessed via the associated type."),
	TYPE_GETTER_AB_F("flags", &attr_getperm, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Deprecated alias for ?#perm"),
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst attr_class_methods[] = {
	TYPE_KWMETHOD("exists", &attribute_exists,
	              "(ob,name:?Dstring,perm:?X2?Dint?Dstring=!P{},permset:?X2?Dint?Dstring=!Aperm,decl?)->?Dbool\n"
	              "#tValueError{The given @perm or @permset contains an unrecognized flag character}"
	              "Check if the an attribute matching the given arguments exists"),
	TYPE_KWMETHOD("lookup", &attribute_lookup,
	              "(ob,name:?Dstring,perm:?X2?Dint?Dstring=!P{},permset:?X2?Dint?Dstring=!Aperm,decl?)->?X2?.?N\n"
	              "#tValueError{The given @perm or @permset contains an unrecognized flag character}"
	              "Lookup an attribute matching the given arguments exists"),
	TYPE_METHOD_END
};

PRIVATE struct type_operator const attr_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
};

/* `Attribute from deemon' */
PUBLIC DeeTypeObject DeeAttribute_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Attribute),
	/* .tp_doc      = */ DOC("The descriptor object for abstract object attributes\n"
	                         "\n"

	                         "(decl:?DType,name:?Dstring,doc?:?Dstring,perm:?X3?Dint?Dstring?N=!N,attrtype?:?DType)\n"
	                         "#pattrtype{The type of object returned by the attribute (s.a. ?#attrtype; used by ?Edoc:Doc if not defined by @doc)}"
	                         "#tTypeError{The given @decl does not define custom attribute operators}"
	                         "#pperm{Attribute permissions (s.a. ?#perm)}"
	                         "Construct a descriptor for an attribute @name of type @decl"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ Attr,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &attr_init_kw,
			/* tp_serialize:   */ &attr_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&attr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&attr_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&attr_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&attr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &attr_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ attr_methods,
	/* .tp_getsets       = */ attr_getsets,
	/* .tp_members       = */ attr_members,
	/* .tp_class_methods = */ attr_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ attr_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(attr_operators),
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
enumattr_init_kw(EnumAttr *__restrict self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *ob;
		DeeObject *perm;
		DeeObject *permset;
		DeeObject *decl;
	} args;
	args.perm    = Dee_None;
	args.permset = Dee_None;
	args.decl    = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__ob_perm_permset_decl,
	                          "o|ooo:enumattr", &args))
		goto err;
	if (DeeNone_Check(args.perm)) {
		self->ea_hint.ah_perm_mask = 0;
	} else if (DeeString_Check(args.perm)) {
		if (string_to_attrperms(DeeString_STR(args.perm), &self->ea_hint.ah_perm_mask))
			goto err;
	} else {
		if (DeeObject_AsUIntX(args.perm, &self->ea_hint.ah_perm_mask))
			goto err;
	}
	if (DeeNone_Check(args.permset)) {
		self->ea_hint.ah_perm_value = self->ea_hint.ah_perm_mask;
	} else if (DeeString_Check(args.permset)) {
		if (string_to_attrperms(DeeString_STR(args.permset), &self->ea_hint.ah_perm_value))
			goto err;
	} else {
		if (DeeObject_AsUIntX(args.permset, &self->ea_hint.ah_perm_value))
			goto err;
	}
	Dee_XIncref(args.decl);
	self->ea_hint.ah_decl = args.decl;
	Dee_Incref(args.ob);
	self->ea_obj = args.ob;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
enumattr_fini(EnumAttr *__restrict self) {
	Dee_Decref(self->ea_obj);
	Dee_XDecref(self->ea_hint.ah_decl);
}

PRIVATE NONNULL((1, 2)) void DCALL
enumattr_visit(EnumAttr *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->ea_obj);
	Dee_XVisit(self->ea_hint.ah_decl);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
enumattr_serialize(EnumAttr *__restrict self,
                   DeeSerial *__restrict writer,
                   Dee_seraddr_t addr) {
	EnumAttr *out = DeeSerial_Addr2Mem(writer, addr, EnumAttr);
	out->ea_hint.ah_perm_mask  = self->ea_hint.ah_perm_mask;
	out->ea_hint.ah_perm_value = self->ea_hint.ah_perm_value;
	if (DeeSerial_PutObject(writer, addr + offsetof(EnumAttr, ea_obj), self->ea_obj))
		goto err;
	return DeeSerial_XPutObject(writer, addr + offsetof(EnumAttr, ea_hint.ah_decl), self->ea_hint.ah_decl);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF EnumAttrIter *DCALL
enumattr_iter(EnumAttr *__restrict self) {
	DREF EnumAttrIter *result;
	size_t req_bufsize;
	size_t cur_bufsize = Dee_ITERATTR_DEFAULT_BUFSIZE;
	result = (DREF EnumAttrIter *)DeeObject_Malloc(offsetof(EnumAttrIter, ei_iter) +
	                                               cur_bufsize);
	if unlikely(!result)
		goto err;
again_iterattr:
	req_bufsize = DeeObject_IterAttr(Dee_TYPE(self->ea_obj), self->ea_obj,
	                                 &result->ei_iter, cur_bufsize,
	                                 &self->ea_hint);
	if unlikely(req_bufsize == (size_t)-1)
		goto err_r;
	if (req_bufsize > cur_bufsize) {
		DREF EnumAttrIter *new_result;
		new_result = (DREF EnumAttrIter *)DeeObject_Realloc(result,
		                                                    offsetof(EnumAttrIter, ei_iter) +
		                                                    req_bufsize);
		if unlikely(!new_result)
			goto err_r;
		result      = new_result;
		cur_bufsize = req_bufsize;
		goto again_iterattr;
	} else if (req_bufsize < cur_bufsize) {
		/* Free unused memory */
		DREF EnumAttrIter *new_result;
		new_result = (DREF EnumAttrIter *)DeeObject_TryRealloc(result,
		                                                       offsetof(EnumAttrIter, ei_iter) +
		                                                       req_bufsize);
		if (likely(new_result) && unlikely(result != new_result)) {
			/* Special case: must update pointers within the iterator
			 *               to reflect the new memory location. */
			ptrdiff_t delta;
			delta  = (byte_t *)new_result - (byte_t *)result;
			result = new_result;
			Dee_attriter_moved(&result->ei_iter, delta);
		}
	}
	result->ei_itsz = req_bufsize;
	result->ei_seq  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeEnumAttrIterator_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE struct type_seq enumattr_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&enumattr_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst enumattr_members[] = {
	TYPE_MEMBER_FIELD("__ob__", STRUCT_OBJECT_AB, offsetof(EnumAttr, ea_obj)),
	TYPE_MEMBER_FIELD("__decl__", STRUCT_OBJECT, offsetof(EnumAttr, ea_hint.ah_decl)),
	TYPE_MEMBER_FIELD("__perm__", STRUCT_CONST | STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(Dee_SIZEOF_ATTRPERM_T),
	                  offsetof(EnumAttr, ea_hint.ah_perm_mask)),
	TYPE_MEMBER_FIELD("__permset__", STRUCT_CONST | STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(Dee_SIZEOF_ATTRPERM_T),
	                  offsetof(EnumAttr, ea_hint.ah_perm_value)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst enumattr_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeEnumAttrIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeAttribute_Type),
	TYPE_MEMBER_END
};

/* `enumattr from deemon' */
PUBLIC DeeTypeObject DeeEnumAttr_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_enumattr),
	/* .tp_doc      = */ DOC("(ob:?X3?DModule?DType?O,perm:?X2?Dint?Dstring=!P{},permset:?X2?Dint?Dstring=!Aperm,decl?:?X2?DModule?DType)\n"
	                         "#pob{The object whose attributes to enumerate}"
	                         "#pperm{Filter enumerated attributes based on permission flags (s.a. ?Aperm?DAttribute)}"
	                         "#ppermset{S.a. @perm}"
	                         "#pdecl{Filter enumerated attributes to only those declared by this object (s.a. ?Adecl?DAttribute)}"
	                         "Create a new sequence for enumerating the ?D{Attribute}s of a given object @ob.\n"
	                         "To enumerate the attributes of a specific type, simply do ${enumattr(ob as MyType)}\n"
	                         "User-defined classes can implement ${operator enumattr} as a yield-function returning "
	                         /**/ "either ?DAttribute or ?Dstring objects (the later will automatically be wrapped as "
	                         /**/ "${Attribute(<MyClass>, <value>)})"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ EnumAttr,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &enumattr_init_kw,
			/* tp_serialize:   */ &enumattr_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&enumattr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&enumattr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &enumattr_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ enumattr_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ enumattr_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



PRIVATE WUNUSED DREF EnumAttrIter *DCALL
enumattriter_init(size_t argc, DeeObject *const *argv) {
	EnumAttr *ea;
	DeeArg_Unpack1(err, argc, argv, "_EnumAttrIterator", &ea);
	if (DeeObject_AssertTypeExact(ea, &DeeEnumAttr_Type)) /* DeeEnumAttr_Type is final, so *Exact is OK */
		goto err;
	return enumattr_iter(ea);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF EnumAttrIter *DCALL
enumattriter_copy(EnumAttrIter *__restrict self) {
	DREF EnumAttrIter *result;
	result = (DREF EnumAttrIter *)DeeObject_Malloc(offsetof(EnumAttrIter, ei_iter) +
	                                               self->ei_itsz);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_attriter_copy(&result->ei_iter,
	                              &self->ei_iter, self->ei_itsz))
		goto err_r;
	Dee_Incref(self->ei_seq);
	result->ei_seq  = self->ei_seq;
	result->ei_itsz = self->ei_itsz;
	DeeObject_Init(result, &DeeEnumAttrIterator_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
enumattriter_fini(EnumAttrIter *__restrict self) {
	/* !!! Iterator must be finalized while still holding
	 *     (transitive) reference to `self->ei_seq->ea_obj' */
	Dee_attriter_fini(&self->ei_iter);
	Dee_Decref(self->ei_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
enumattriter_visit(EnumAttrIter *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_attriter_visit(&self->ei_iter);
	Dee_Visit(self->ei_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF Attr *DCALL
enumattriter_next(EnumAttrIter *__restrict self) {
	int status;
	DREF Attr *result;
	struct Dee_attrdesc desc;
	struct Dee_attrhint *hint = &self->ei_seq->ea_hint;
	for (;;) {
		DBG_memset(&desc, 0xcc, sizeof(desc));
		status = Dee_attriter_next(&self->ei_iter, &desc);
		if (status != 0) {
			if likely(status > 0)
				return (DREF Attr *)ITER_DONE;
			goto err;
		}
		if (!Dee_attrhint_matches(hint, &desc)) {
			/* Supposed to skip this attribute */
			Dee_attrdesc_fini(&desc);
			continue;
		}
		break;
	}
	result = DeeObject_MALLOC(Attr);
	if unlikely(!result)
		goto err;
	Dee_Incref(desc.ad_info.ai_decl);
	memcpy(&result->a_desc, &desc, sizeof(struct Dee_attrdesc));
	DeeObject_Init(result, &DeeAttribute_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
enumattriter_bool(EnumAttrIter *__restrict self) {
	return Dee_attriter_bool(&self->ei_iter, self->ei_itsz);
}

PRIVATE struct type_member tpconst enumattriter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(EnumAttrIter, ei_seq), "->?Ert:EnumAttr"),
	TYPE_MEMBER_FIELD("__itersz__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(EnumAttrIter, ei_itsz)),
	TYPE_MEMBER_END
};

/* `(enumattr from deemon).Iterator' */
PUBLIC DeeTypeObject DeeEnumAttrIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_EnumAttrIterator",
	/* .tp_doc      = */ DOC("next->?DAttribute"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &enumattriter_copy,
			/* tp_any_ctor:    */ &enumattriter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL, /* Not serializable (would require an extra operator in `struct Dee_attriter_type') */
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&enumattriter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&enumattriter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&enumattriter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&enumattriter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ enumattriter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ATTRIBUTE_C */
