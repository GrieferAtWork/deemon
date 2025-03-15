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
#ifndef GUARD_DEEMON_OBJECTS_ATTRIBUTE_C
#define GUARD_DEEMON_OBJECTS_ATTRIBUTE_C 1

#include <deemon/api.h>

/* Without our use of SJLJ, gcc emits the following warning (on some platforms):
 * >> /src/deemon/objects/attribute.c: In function ‘enumattriter_next’:
 * >> /include/deemon/system-sjlj.h:90:37: warning: ‘({anonymous})’ may be used uninitialized in this function [-Wmaybe-uninitialized]
 * >>    90 | #define DeeSystem_SetJmp(env)       _setjmp(env)
 * >>       |                                     ^~~~~~~
 * >> /include/deemon/system-sjlj.h:90:37: note: ‘({anonymous})’ was declared here
 * >>    90 | #define DeeSystem_SetJmp(env)       _setjmp(env)
 * >>       |                                     ^~~~~~~~~~~~
 * >> /src/deemon/objects/attribute.c:1183:15: note: in expansion of macro ‘DeeSystem_SetJmp’
 * >>  1183 |  if ((error = DeeSystem_SetJmp(self->ei_break)) == 0)
 * >>       |               ^~~~~~~~~~~~~~~~
 * >> /include/deemon/system-sjlj.h:90:37: warning: ‘({anonymous})’ may be used uninitialized in this function [-Wmaybe-uninitialized]
 * >>    90 | #define DeeSystem_SetJmp(env)       _setjmp(env)
 * >>       |                                     ^~~~~~~
 * >> /include/deemon/system-sjlj.h:90:37: note: ‘({anonymous})’ was declared here
 * >>    90 | #define DeeSystem_SetJmp(env)       _setjmp(env)
 * >>       |                                     ^~~~~~~~~~~~
 * >> /src/deemon/objects/attribute.c:1166:8: note: in expansion of macro ‘DeeSystem_SetJmp’
 * >>  1166 |    if (DeeSystem_SetJmp(self->ei_break) == 0) {
 *
 * Obviously this is total bogus, so just suppress it. */
__pragma_GCC_diagnostic_ignored(Wmaybe_uninitialized)

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/system-sjlj.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/spcall.h>
/**/

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
/**/

#include <stddef.h> /* offsetof */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */


typedef DeeAttributeObject /*       */ Attr;
typedef DeeEnumAttrObject /*        */ EnumAttr;
typedef DeeEnumAttrIteratorObject /**/ EnumAttrIter;


PRIVATE NONNULL((1)) void DCALL
attr_fini(Attr *__restrict self) {
	if (self->a_info.a_perm & ATTR_NAMEOBJ)
		Dee_Decref(COMPILER_CONTAINER_OF(self->a_name, DeeStringObject, s_str));
	attribute_info_fini(&self->a_info);
}

PRIVATE NONNULL((1, 2)) void DCALL
attr_visit(Attr *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->a_info.a_decl);
	Dee_XVisit(self->a_info.a_attrtype);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
attr_hash(Attr *__restrict self) {
	Dee_hash_t result;
	result = self->a_info.a_perm;
	result = Dee_HashCombine(result, DeeObject_Hash(self->a_info.a_decl));
	result = Dee_HashCombine(result, Dee_HashPointer(self->a_info.a_attrtype));
	if (self->a_info.a_perm & ATTR_NAMEOBJ) {
		DeeStringObject *string;
		string = COMPILER_CONTAINER_OF(self->a_name, DeeStringObject, s_str);
		result = Dee_HashCombine(result, DeeString_Hash((DeeObject *)string));
	} else {
		result = Dee_HashCombine(result, Dee_HashStr(self->a_name));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
attr_compare_eq(Attr *self, Attr *other) {
	if (DeeObject_AssertType(other, &DeeAttribute_Type))
		goto err;
	if (self->a_info.a_attrtype != other->a_info.a_attrtype)
		goto nope;
	if (self->a_info.a_decl != other->a_info.a_decl)
		Dee_return_DeeObject_TryCompareEq_if_ne(self->a_info.a_decl, other->a_info.a_decl);
	if ((self->a_info.a_perm & ~(ATTR_NAMEOBJ | ATTR_DOCOBJ)) !=
	    (other->a_info.a_perm & ~(ATTR_NAMEOBJ | ATTR_DOCOBJ)))
		goto nope;
	if (self->a_name == other->a_name)
		goto yup;
	if ((self->a_info.a_perm & ATTR_NAMEOBJ) &&
	    (other->a_info.a_perm & ATTR_NAMEOBJ)) {
		DeeStringObject *my_name = COMPILER_CONTAINER_OF(self->a_name, DeeStringObject, s_str);
		DeeStringObject *ot_name = COMPILER_CONTAINER_OF(other->a_name, DeeStringObject, s_str);
		if (DeeString_Hash((DeeObject *)my_name) != DeeString_Hash((DeeObject *)ot_name))
			goto nope;
		if (!DeeString_EqualsSTR(my_name, ot_name))
			goto nope;
	} else {
		if (strcmp(self->a_name, other->a_name) != 0)
			goto nope;
	}
yup:
	return 0;
nope:
	return 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp attr_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&attr_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&attr_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE struct type_member tpconst attr_members[] = {
	TYPE_MEMBER_FIELD_DOC("decl", STRUCT_OBJECT, offsetof(Attr, a_info.a_decl),
	                      "The type or object that is declaring this ?."),
	TYPE_MEMBER_FIELD_DOC("attrtype", STRUCT_OBJECT_OPT, offsetof(Attr, a_info.a_attrtype),
	                      "->?X2?DType?N\n"
	                      "The type of this ?., or ?N if not known"),
	TYPE_MEMBER_BITFIELD_DOC("canget", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMGET,
	                         "Check if the ?. has a way of being read from"),
	TYPE_MEMBER_BITFIELD_DOC("candel", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMDEL,
	                         "Check if the ?. has a way of being deleted"),
	TYPE_MEMBER_BITFIELD_DOC("canset", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMSET,
	                         "Check if the ?. has a way of being written to"),
	TYPE_MEMBER_BITFIELD_DOC("cancall", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMCALL,
	                         "Returns ?t if the ?. is intended to be called as a function. "
	                         /**/ "Note that this feature alone does not meant that the ?. really can, or "
	                         /**/ "cannot be called, only that calling it as a function might be the intended use."),
	TYPE_MEMBER_BITFIELD_DOC("isprivate", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PRIVATE,
	                         "Check if the ?. is considered to be private\n"
	                         "Private attributes only appear in user-classes, prohibiting access to only thiscall "
	                         /**/ "functions with a this-argument that is an instance of the declaring class."),
	TYPE_MEMBER_BITFIELD_DOC("isproperty", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PROPERTY,
	                         "Check if the ?. is property-like, meaning that access by "
	                         /**/ "reading, deletion, or writing causes unpredictable side-effects"),
	TYPE_MEMBER_BITFIELD_DOC("iswrapper", STRUCT_CONST, Attr, a_info.a_perm, ATTR_WRAPPER,
	                         "Check if the ?. is accessed from the implementing type, which "
	                         /**/ "exposes it as a wrapper for an instance member (e.g. ${string.find} is an unbound "
	                         /**/ "wrapper (aka. ${Attribute(string, \"find\").iswrapper == true}) for the instance function, "
	                         /**/ "member or property that would be bound in ${\"foo\".find} (aka. "
	                         /**/ "${Attribute(\"foo\", \"find\").iswrapper == false}))"),
	TYPE_MEMBER_BITFIELD_DOC("isinstance", STRUCT_CONST, Attr, a_info.a_perm, ATTR_IMEMBER,
	                         "Check if accessing this ?. requires an instance of the declaring object "
	                         /**/ "?#decl, rather than being an ?. of the declaring object ?#decl itself.\n"
	                         "Note that practically all attributes, such as member functions, are available as both "
	                         /**/ "instance and class attributes, while in other cases an ?. will evaluate to different "
	                         /**/ "objects depending on being invoked on a class or an instance (such as ?Aisreg?Eposix:stat)"),
	TYPE_MEMBER_BITFIELD_DOC("isclass", STRUCT_CONST, Attr, a_info.a_perm, ATTR_CMEMBER,
	                         "Check if access to this ?. must be made though the declaring type ?#decl.\n"
	                         "To test if an ?. can only be accessed through an instance, use ?#isinstance instead"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
attr_get_name(Attr *__restrict self) {
	DREF DeeStringObject *result;
	char const *name_str;
	uint16_t perm;
again:
	name_str = self->a_name;
	atomic_thread_fence(Dee_ATOMIC_ACQUIRE);
	perm = self->a_info.a_perm;
	if (perm & ATTR_NAMEOBJ) {
		result = COMPILER_CONTAINER_OF(name_str,
		                               DeeStringObject,
		                               s_str);
		Dee_Incref(result);
	} else {
		if (!name_str) {
			result = (DREF DeeStringObject *)Dee_EmptyString;
			Dee_Incref(result);
		} else {
			/* Wrap the name string into a string object. */
			result = (DREF DeeStringObject *)DeeString_New(name_str);
			if unlikely(!result)
				goto done;
			/* Cache the name-string as part of the attribute structure. */
			if unlikely(!atomic_cmpxch_weak(&self->a_name, name_str, DeeString_STR(result))) {
				Dee_Decref(result);
				goto again;
			}
			atomic_or(&self->a_info.a_perm, ATTR_NAMEOBJ);
			Dee_Incref(result);
		}
	}
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
attr_get_doc(Attr *__restrict self) {
	DREF DeeStringObject *result;
	char const *doc_str;
	uint16_t perm;
again:
	doc_str = self->a_info.a_doc;
	atomic_thread_fence(Dee_ATOMIC_ACQUIRE);
	perm = self->a_info.a_perm;
	if (perm & ATTR_DOCOBJ) {
		result = COMPILER_CONTAINER_OF(doc_str,
		                               DeeStringObject,
		                               s_str);
		Dee_Incref(result);
	} else {
		if (!doc_str) {
			result = (DREF DeeStringObject *)Dee_None;
			Dee_Incref(result);
		} else {
			/* Wrap the doc string into a string object. */
			result = (DREF DeeStringObject *)DeeString_NewUtf8(doc_str,
			                                                   strlen(doc_str),
			                                                   STRING_ERROR_FIGNORE);
			if unlikely(!result)
				goto done;
			/* Cache the doc-string as part of the attribute structure. */
			if unlikely(!atomic_cmpxch_weak(&self->a_info.a_doc, doc_str, DeeString_STR(result))) {
				Dee_Decref(result);
				goto again;
			}
			atomic_or(&self->a_info.a_perm, ATTR_DOCOBJ);
			Dee_Incref(result);
		}
	}
done:
	return result;
}


PRIVATE char attr_flags[] = {
	/* [FFS(ATTR_PERMGET) - 1]  = */ 'g',
	/* [FFS(ATTR_PERMDEL) - 1]  = */ 'd',
	/* [FFS(ATTR_PERMSET) - 1]  = */ 's',
	/* [FFS(ATTR_PERMCALL) - 1] = */ 'f',
	/* [FFS(ATTR_IMEMBER) - 1]  = */ 'i',
	/* [FFS(ATTR_CMEMBER) - 1]  = */ 'c',
	/* [FFS(ATTR_PRIVATE) - 1]  = */ 'h',
	/* [FFS(ATTR_PROPERTY) - 1] = */ 'p',
	/* [FFS(ATTR_WRAPPER) - 1]  = */ 'w',
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_getflags(Attr *__restrict self) {
	DREF DeeObject *result;
	uint16_t mask = self->a_info.a_perm & 0x1ff;
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
	dst       = DeeString_STR(result);
	mask      = self->a_info.a_perm & 0x1ff;
	num_flags = 0;
	while (mask) {
		if (mask & 1) {
			*dst++ = attr_flags[num_flags];
		}
		mask >>= 1;
		++num_flags;
	}
	ASSERT(dst == DeeString_END(result));
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
attr_print(Attr *__restrict self,
           dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%k.%s",
	                        self->a_info.a_decl,
	                        self->a_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
attr_printrepr(Attr *__restrict self,
               dformatprinter printer, void *arg) {
	char flags_str[COMPILER_LENOF(attr_flags) + 1];
	char *flags_ptr = flags_str;
	uint16_t flags_index, flags_mask;
	flags_mask  = self->a_info.a_perm & 0x1ff;
	flags_index = 0;
	while (flags_mask) {
		if (flags_mask & 1)
			*flags_ptr++ = attr_flags[flags_index];
		flags_mask >>= 1;
		++flags_index;
	}
	*flags_ptr = '\0';
	return DeeFormat_Printf(printer, arg,
	                        "Attribute(%r, %q, %q)",
	                        self->a_info.a_decl,
	                        self->a_name, flags_str);
}



PRIVATE struct type_getset tpconst attr_getsets[] = {
	TYPE_GETTER_AB_F("name", &attr_get_name, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "The name of this ?."),
	TYPE_GETTER_AB_F("doc", &attr_get_doc, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "The documentation string of this ?., or ?N when no documentation is present"),
	TYPE_GETTER_AB_F("flags", &attr_getflags, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Return a set of characters descripting the flags of @this ?.:\n"
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
	TYPE_GETSET_END
};


LOCAL WUNUSED NONNULL((1, 2)) int DCALL
string_to_attrflags(char const *__restrict str,
                    uint16_t *__restrict p_result) {
	while (*str) {
		char ch = *str++;
		unsigned int i;
		for (i = 0; attr_flags[i] != ch; ++i) {
			if unlikely(i >= COMPILER_LENOF(attr_flags)) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Unknown attribute flag %:1q",
				                str - 1);
				goto err;
			}
		}
		*p_result |= (uint16_t)1 << i;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
attribute_init(DeeAttributeObject *__restrict self,
               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int lookup_error;
	DeeObject *search_self, *search_name;
	DeeObject *flagmask = NULL, *flagval = NULL;
	struct attribute_lookup_rules rules;
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = 0;
	rules.alr_perm_value = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__ob_name_flagmask_flagval_decl, "oo|ooo:Attribute",
	                    &search_self, &search_name, &flagmask,
	                    &flagval, &rules.alr_decl))
		goto err;
	if (DeeObject_AssertTypeExact(search_name, &DeeString_Type))
		goto err;
	rules.alr_name = DeeString_STR(search_name);
	rules.alr_hash = DeeString_Hash(search_name);
	if (flagmask) {
		if (DeeString_Check(flagmask)) {
			if unlikely(string_to_attrflags(DeeString_STR(flagmask), &rules.alr_perm_mask))
				goto err;
		} else {
			if (DeeObject_AsUInt16(flagmask, &rules.alr_perm_mask))
				goto err;
		}
		if (flagval) {
			if (DeeString_Check(flagval)) {
				if unlikely(string_to_attrflags(DeeString_STR(flagval), &rules.alr_perm_value))
					goto err;
			} else {
				if (DeeObject_AsUInt16(flagval, &rules.alr_perm_value))
					goto err;
			}
		} else {
			rules.alr_perm_value = rules.alr_perm_mask;
		}
	} else if (flagval) {
		rules.alr_perm_mask = (uint16_t)-1;
		if (DeeString_Check(flagval)) {
			if unlikely(string_to_attrflags(DeeString_STR(flagval), &rules.alr_perm_value))
				goto err;
		} else {
			if (DeeObject_AsUInt16(flagval, &rules.alr_perm_value))
				goto err;
		}
	}
	lookup_error = DeeObject_FindAttr(Dee_TYPE(search_self),
	                                  search_self,
	                                  &self->a_info,
	                                  &rules);
	if (lookup_error > 0) {
		/* Attribute wasn't found... */
		err_unknown_attribute_lookup_string(Dee_TYPE(search_self), rules.alr_name);
		goto err;
	}
	if likely(!lookup_error) {
		self->a_name = DeeString_STR(argv[1]);
		self->a_info.a_perm |= ATTR_NAMEOBJ;
		Dee_Incref(argv[1]);
	}
	return lookup_error;
err:
	return -1;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
attribute_exists(DeeTypeObject *__restrict UNUSED(self), size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	int lookup_error;
	struct attribute_info info;
	DeeObject *search_self, *search_name;
	DeeObject *flagmask = NULL, *flagval = NULL;
	struct attribute_lookup_rules rules;
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = 0;
	rules.alr_perm_value = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__ob_name_flagmask_flagval_decl, "oo|ooo:exists",
	                    &search_self, &search_name, &flagmask,
	                    &flagval, &rules.alr_decl))
		goto err;
	if (DeeObject_AssertTypeExact(search_name, &DeeString_Type))
		goto err;
	rules.alr_name = DeeString_STR(search_name);
	rules.alr_hash = DeeString_Hash(search_name);
	if (flagmask) {
		if (DeeString_Check(flagmask)) {
			if unlikely(string_to_attrflags(DeeString_STR(flagmask), &rules.alr_perm_mask))
				goto err;
		} else {
			if (DeeObject_AsUInt16(flagmask, &rules.alr_perm_mask))
				goto err;
		}
		if (flagval) {
			if (DeeString_Check(flagval)) {
				if unlikely(string_to_attrflags(DeeString_STR(flagval), &rules.alr_perm_value))
					goto err;
			} else {
				if (DeeObject_AsUInt16(flagval, &rules.alr_perm_value))
					goto err;
			}
		} else {
			rules.alr_perm_value = rules.alr_perm_mask;
		}
	} else if (flagval) {
		rules.alr_perm_mask = (uint16_t)-1;
		if (DeeString_Check(flagval)) {
			if unlikely(string_to_attrflags(DeeString_STR(flagval), &rules.alr_perm_value))
				goto err;
		} else {
			if (DeeObject_AsUInt16(flagval, &rules.alr_perm_value))
				goto err;
		}
	}
	lookup_error = DeeObject_FindAttr(Dee_TYPE(search_self),
	                                  search_self,
	                                  &info,
	                                  &rules);
	if (lookup_error != 0) {
		if likely(lookup_error > 0)
			return_false;
		goto err;
	}
	attribute_info_fini(&info);
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
attribute_lookup(DeeTypeObject *__restrict UNUSED(self), size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeAttributeObject *result;
	int lookup_error;
	struct attribute_info info;
	DeeObject *search_self, *search_name;
	DeeObject *flagmask = NULL, *flagval = NULL;
	struct attribute_lookup_rules rules;
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = 0;
	rules.alr_perm_value = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__ob_name_flagmask_flagval_decl, "oo|ooo:lookup",
	                    &search_self, &search_name, &flagmask,
	                    &flagval, &rules.alr_decl))
		goto err;
	if (DeeObject_AssertTypeExact(search_name, &DeeString_Type))
		goto err;
	rules.alr_name = DeeString_STR(search_name);
	rules.alr_hash = DeeString_Hash(search_name);
	if (flagmask) {
		if (DeeString_Check(flagmask)) {
			if unlikely(string_to_attrflags(DeeString_STR(flagmask), &rules.alr_perm_mask))
				goto err;
		} else {
			if (DeeObject_AsUInt16(flagmask, &rules.alr_perm_mask))
				goto err;
		}
		if (flagval) {
			if (DeeString_Check(flagval)) {
				if unlikely(string_to_attrflags(DeeString_STR(flagval), &rules.alr_perm_value))
					goto err;
			} else {
				if (DeeObject_AsUInt16(flagval, &rules.alr_perm_value))
					goto err;
			}
		} else {
			rules.alr_perm_value = rules.alr_perm_mask;
		}
	} else if (flagval) {
		rules.alr_perm_mask = (uint16_t)-1;
		if (DeeString_Check(flagval)) {
			if unlikely(string_to_attrflags(DeeString_STR(flagval), &rules.alr_perm_value))
				goto err;
		} else {
			if (DeeObject_AsUInt16(flagval, &rules.alr_perm_value))
				goto err;
		}
	}
	lookup_error = DeeObject_FindAttr(Dee_TYPE(search_self),
	                                  search_self,
	                                  &info,
	                                  &rules);
	if (lookup_error != 0) {
		if likely(lookup_error > 0)
			return_none;
		goto err;
	}
	result = DeeObject_MALLOC(DeeAttributeObject);
	if unlikely(!result)
		goto err_info;
	DeeObject_Init(result, &DeeAttribute_Type);
	info.a_perm |= ATTR_NAMEOBJ;
	memcpy(&result->a_info, &info, sizeof(struct attribute_info)); /* Inherit references */
	result->a_name = DeeString_STR(search_name);
	Dee_Incref(search_name);
	return (DREF DeeObject *)result;
err_info:
	attribute_info_fini(&info);
err:
	return NULL;
}

PRIVATE struct type_method tpconst attr_class_methods[] = {
	TYPE_KWMETHOD("exists", &attribute_exists,
	              "(ob,name:?Dstring,flagmask:?X2?Dint?Dstring=!P{},flagval:?X2?Dint?Dstring=!Aflagmask,decl?)->?Dbool\n"
	              "#tValueError{The given @flagmask or @flagval contains an unrecognized flag character}"
	              "Taking the same arguments as ?#{op:constructor}, check if the an attribute matching "
	              /**/ "the given arguments exists, returning ?t/?f indicative of this\n"
	              "${"
	              /**/ "static function exists(ob, name, flagmask = \"\", flagval = \"\", decl?) {\n"
	              /**/ "	import Error from deemon;\n"
	              /**/ "	try {\n"
	              /**/ "		attribute(ob, name, flagmask, flagval, decl);\n"
	              /**/ "	} catch (Error.AttributeError) {\n"
	              /**/ "		return false;\n"
	              /**/ "	}\n"
	              /**/ "	return true;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD("lookup", &attribute_lookup,
	              "(ob,name:?Dstring,flagmask:?X2?Dint?Dstring=!P{},flagval:?X2?Dint?Dstring=!Aflagmask,decl?)->?X2?.?N\n"
	              "#tValueError{The given @flagmask or @flagval contains an unrecognized flag character}"
	              "Same as ?#{op:constructor}, but return ?N if the attribute doesn't exist\n"
	              "${"
	              /**/ "static function lookup(ob, name, flagmask = \"\", flagval = \"\", decl?) {\n"
	              /**/ "	import Error from deemon;\n"
	              /**/ "	try {\n"
	              /**/ "		return attribute(ob, name, flagmask, flagval, decl);\n"
	              /**/ "	} catch (Error.AttributeError) {\n"
	              /**/ "		return none;\n"
	              /**/ "	}\n"
	              /**/ "}"
	              "}"),
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

	                         "(ob,name:?Dstring,flagmask:?X2?Dint?Dstring=!P{},flagval:?X2?Dint?Dstring=!Aflagmask,decl?)\n"
	                         "#pflagmask{Set of attribute flags to mask when searching for matches (s.a. ?#flags)}"
	                         "#pflagval{Set of attribute flags required when searching for matches (s.a. ?#flags) "
	                         /*          */ "(When only this is given, and @flagmask is omit (as possible when "
	                         /*          */ "using keyword arguments), flagmask is set to @flagval)}"
	                         "#tAttributeError{No attribute matching the specified restrictions could be found}"
	                         "#tValueError{The given @flagmask or @flagval contains an unrecognized flag character}"
	                         "Lookup an ?. enumerated by ${enumattr(ob)} or ${enumattr(tp)}, matching "
	                         /**/ "the given @name, as well as having its set of flags match @flagval, when masked by @flagmask\n"
	                         "Additionally, @decl may be specified to narrow down valid matches to only those declared by it\n"
	                         "${"
	                         /**/ "function findattr(ob: Object, name: string, flagmask: int | string,\n"
	                         /**/ "                  flagval: int | string, decl?: Object): Attribute {\n"
	                         /**/ "	import enumattr, Error, HashSet from deemon;\n"
	                         /**/ "	flagmask = HashSet(flagmask);\n"
	                         /**/ "	for (local attr: enumattr(ob)) {\n"
	                         /**/ "		if (attr.name != name)\n"
	                         /**/ "			continue;\n"
	                         /**/ "		if (decl is bound && attr.decl !== decl)\n"
	                         /**/ "			continue;\n"
	                         /**/ "		if ((flagmask & attr.flags) != flagval)\n"
	                         /**/ "			continue;\n"
	                         /**/ "		return attr;\n"
	                         /**/ "	}\n"
	                         /**/ "	throw Error.AttributeError(...);\n"
	                         /**/ "}"
	                         "}\n"
	                         "Using @flagmask and @flagval, you can easily restrict a search to only "
	                         /**/ "class-, or instance-attributes:\n"
	                         "${"
	                         /**/ "import Attribute from deemon;\n"
	                         /**/ "import stat from posix;\n"
	                         /**/ "/* The class-variant (Attribute cannot be accessed from an instance) */\n"
	                         /**/ "print repr Attribute(stat, \"isreg\", \"i\", \"\");\n"
	                         /**/ "/* The class-variant (Attribute is a wrapper) */\n"
	                         /**/ "print repr Attribute(stat, \"isreg\", \"w\");\n"
	                         /**/ "/* The instance-variant (Attribute can be accessed from an instance) */\n"
	                         /**/ "print repr Attribute(stat, \"isreg\", \"i\");\n"
	                         /**/ "/* The instance-variant (Attribute isn't a wrapper) */\n"
	                         /**/ "print repr Attribute(stat, \"isreg\", \"w\", \"\");"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor   = */ (dfunptr_t)NULL,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(Attr),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&attribute_init
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&attr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&attr_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&attr_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&attr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &attr_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ attr_getsets,
	/* .tp_members       = */ attr_members,
	/* .tp_class_methods = */ attr_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ attr_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(attr_operators),
};



#ifndef CONFIG_LONGJMP_ENUMATTR
struct attr_list {
	size_t      al_c;
	size_t      al_a;
	DREF Attr **al_v;
};

PRIVATE dssize_t DCALL
save_attr(DeeObject *__restrict declarator,
          char const *__restrict attr_name, char const *attr_doc,
          uint16_t perm, DeeTypeObject *attr_type,
          struct attr_list *__restrict self) {
	DREF Attr *new_attr;
	/* Make sure that the collection vector has sufficient space. */
	if (self->al_c == self->al_a) {
		DREF Attr **new_vector;
		size_t new_alloc = self->al_a * 2;
		if (!new_alloc)
			new_alloc = 8;
do_realloc:
		new_vector = (DREF Attr **)Dee_TryReallocc(self->al_v, new_alloc,
		                                           sizeof(DREF Attr *));
		if unlikely(!new_vector) {
			if (new_alloc != self->al_c + 1) {
				new_alloc = self->al_c + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemory(new_alloc * sizeof(DREF Attr *)))
				goto do_realloc;
			goto err;
		}
		self->al_v = new_vector;
		self->al_a = new_alloc;
	}
	/* Allocate a new attribute descriptor. */
	new_attr = DeeObject_MALLOC(Attr);
	if unlikely(!new_attr)
		goto err;
	new_attr->a_name = attr_name;
	if (perm & ATTR_NAMEOBJ)
		Dee_Incref(COMPILER_CONTAINER_OF(attr_name, DeeStringObject, s_str));
	if (!attr_doc) {
		ASSERT(!(perm & ATTR_DOCOBJ));
		new_attr->a_info.a_doc = NULL;
	} else if (perm & ATTR_DOCOBJ) {
		new_attr->a_info.a_doc = attr_doc;
		Dee_Incref(COMPILER_CONTAINER_OF(attr_doc, DeeStringObject, s_str));
	} else {
		new_attr->a_info.a_doc = attr_doc;
	}
	new_attr->a_info.a_decl     = declarator;
	new_attr->a_info.a_perm     = perm;
	new_attr->a_info.a_attrtype = attr_type;
	Dee_Incref(declarator);
	Dee_XIncref(attr_type);
	DeeObject_Init(new_attr, &DeeAttribute_Type);
	self->al_v[self->al_c++] = new_attr; /* Inherit reference. */
	return 0;
err:
	return -1;
}
#endif /* !CONFIG_LONGJMP_ENUMATTR */


PRIVATE WUNUSED NONNULL((1)) int DCALL
enumattr_init(EnumAttr *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeObject *a, *b = NULL;
	_DeeArg_Unpack1Or2(err, argc, argv, "enumattr", &a, &b);
	if (b) {
		if (DeeObject_AssertType(a, &DeeType_Type))
			goto err;
		if (DeeObject_AssertTypeOrAbstract(b, (DeeTypeObject *)a))
			goto err;
		self->ea_type = (DREF DeeTypeObject *)a;
		self->ea_obj  = b;
	} else if (DeeSuper_Check(a)) {
		self->ea_type = DeeSuper_TYPE(a);
		self->ea_obj  = DeeSuper_SELF(a);
	} else {
		self->ea_type = Dee_TYPE(a);
		self->ea_obj  = a;
	}
	Dee_Incref(self->ea_type);
	Dee_Incref(self->ea_obj);

#ifndef CONFIG_LONGJMP_ENUMATTR
	/* Collect all attributes */
	{
		struct attr_list list;
		list.al_a = list.al_c = 0;
		list.al_v             = NULL;

		/* Enumerate all the attributes. */
		if (DeeObject_EnumAttr(self->ea_type, self->ea_obj, (denum_t)&save_attr, &list) < 0) {
			Dee_Decrefv(list.al_v, list.al_c);
			Dee_Free(list.al_v);
			Dee_Decref(self->ea_type);
			Dee_XDecref(self->ea_obj);
			goto err;
		}

		/* Truncate the collection vector. */
		if (list.al_c != list.al_a) {
			DREF Attr **new_vector;
			new_vector = (DREF Attr **)Dee_TryReallocc(list.al_v, list.al_c,
			                                           sizeof(DREF Attr *));
			if likely(new_vector)
				list.al_v = new_vector;
		}

		/* Assign the attribute vector. */
		self->ea_attrc = list.al_c;
		self->ea_attrv = list.al_v; /* Inherit. */
	}
#endif /* !CONFIG_LONGJMP_ENUMATTR */
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
enumattr_fini(EnumAttr *__restrict self) {
	Dee_Decref(self->ea_type);
	Dee_XDecref(self->ea_obj);
#ifndef CONFIG_LONGJMP_ENUMATTR
	Dee_Decrefv(self->ea_attrv, self->ea_attrc);
	Dee_Free(self->ea_attrv);
#endif /* !CONFIG_LONGJMP_ENUMATTR */
}

PRIVATE NONNULL((1, 2)) void DCALL
enumattr_visit(EnumAttr *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ea_type);
	Dee_XVisit(self->ea_obj);
#ifndef CONFIG_LONGJMP_ENUMATTR
	Dee_Visitv(self->ea_attrv, self->ea_attrc);
#endif /* !CONFIG_LONGJMP_ENUMATTR */
}

PRIVATE NONNULL((1, 2)) void DCALL
enumattriter_setup(EnumAttrIter *__restrict self,
                   EnumAttr *__restrict seq) {
	self->ei_seq = seq;
	Dee_Incref(seq);
#ifdef CONFIG_LONGJMP_ENUMATTR
	Dee_atomic_lock_init(&self->ei_lock);
	/* Set the buffer pointer to its initial state. */
	self->ei_bufpos = NULL;
#else /* CONFIG_LONGJMP_ENUMATTR */
	self->ei_iter = seq->ea_attrv;
	self->ei_end  = seq->ea_attrv + seq->ea_attrc;
#endif /* !CONFIG_LONGJMP_ENUMATTR */
}

PRIVATE WUNUSED NONNULL((1)) DREF EnumAttrIter *DCALL
enumattr_iter(EnumAttr *__restrict self) {
	DREF EnumAttrIter *result;
	result = DeeObject_MALLOC(EnumAttrIter);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeEnumAttrIterator_Type);
	enumattriter_setup(result, self);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
enumattr_hash(EnumAttr *__restrict self) {
	return ((self->ea_obj ? DeeObject_Hash(self->ea_obj) : 0) ^
	        Dee_HashPointer(self->ea_type));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
enumattr_compare_eq(EnumAttr *self, EnumAttr *other) {
	if (DeeObject_AssertTypeExact(other, &DeeEnumAttr_Type))
		goto err;
	if (self->ea_type != other->ea_type)
		return 1;
	return DeeObject_TryCompareEq(self->ea_obj, other->ea_obj);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp enumattr_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&enumattr_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&enumattr_compare_eq,
	/* .tp_compare       = */ DEFIMPL(&default__seq_operator_compare__with__seq_operator_foreach),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__seq_operator_lo__with__seq_operator_compare),
	/* .tp_le            = */ DEFIMPL(&default__seq_operator_le__with__seq_operator_compare),
	/* .tp_gr            = */ DEFIMPL(&default__seq_operator_gr__with__seq_operator_compare),
	/* .tp_ge            = */ DEFIMPL(&default__seq_operator_ge__with__seq_operator_compare),
};

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
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
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

PRIVATE struct type_member tpconst enumattr_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeEnumAttrIterator_Type),
	TYPE_MEMBER_END
};

/* `enumattr from deemon' */
PUBLIC DeeTypeObject DeeEnumAttr_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_enumattr),
	/* .tp_doc      = */ DOC("(tp:?DType)\n"
	                         "Enumerate attributes of the ?DType @tp and its bases\n"
	                         "\n"

	                         "(ob)\n"
	                         "Same as ${enumattr(type(ob), ob)}\n"
	                         "\n"

	                         "(tp:?DType,ob)\n"
	                         "Create a new sequence for enumerating the ?D{Attribute}s of a given object.\n"
	                         "When @tp is given, only enumerate objects implemented by @tp or "
	                         /**/ "one of its bases and those accessible through a superview of @ob using @tp.\n"
	                         "Note that iterating this object may be expensive, and that conversion to "
	                         /**/ "a different sequence before iterating multiple times may be desirable"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&enumattr_init,
				TYPE_FIXED_ALLOCATOR(EnumAttr)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&enumattr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_compare_eq),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&enumattr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ &enumattr_cmp,
	/* .tp_seq           = */ &enumattr_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ enumattr_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
};

#ifdef CONFIG_LONGJMP_ENUMATTR
/* Signal values with which `ei_continue' can be invoked. */
#define CNTSIG_CONTINUE  1 /* Continue execution. */
#define CNTSIG_STOP      2 /* Tear down execution. */

/* Signal values with which `ei_break' can be invoked. */
#define BRKSIG_YIELD     1 /* Yield more items. */
#define BRKSIG_ERROR     2 /* An error occurred. */
#define BRKSIG_STOP      3 /* Stop yielding items. */
#define BRKSIG_COLLECT   4 /* Collect memory and try again. */
#endif /* CONFIG_LONGJMP_ENUMATTR */


PRIVATE WUNUSED NONNULL((1)) int DCALL
enumattriter_init(EnumAttrIter *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	EnumAttr *seq;
	_DeeArg_Unpack1(err, argc, argv, "_EnumAttrIterator", &seq);
	if (DeeObject_AssertTypeExact(seq, &DeeEnumAttr_Type)) /* DeeEnumAttr_Type is final, so *Exact is OK */
		goto err;
	enumattriter_setup(self, seq);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
enumattriter_fini(EnumAttrIter *__restrict self) {
#ifdef CONFIG_LONGJMP_ENUMATTR
	DREF Attr **iter = self->ei_bufpos;
	if (iter && iter != (DREF Attr **)ITER_DONE) {
		int error;
		ASSERT(iter >= self->ei_buffer);
		ASSERT(iter <= COMPILER_ENDOF(self->ei_buffer));

		/* Discard all remaining items. */
		for (; iter < COMPILER_ENDOF(self->ei_buffer); ++iter)
			Dee_Decref(*iter);
		self->ei_bufpos = (DREF Attr **)ITER_DONE; /* Indicate that we want to stop iteration. */

		/* Resolve execution of the iterator normally. */
		error = DeeSystem_SetJmp(self->ei_break);
		if (error == 0)
			DeeSystem_LongJmp(self->ei_continue, CNTSIG_STOP);
		if (error == BRKSIG_ERROR)
			DeeError_Handled(ERROR_HANDLED_RESTORE);
	}
#endif /* CONFIG_LONGJMP_ENUMATTR */
	Dee_Decref(self->ei_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
enumattriter_visit(EnumAttrIter *__restrict self, dvisit_t proc, void *arg) {
#ifdef CONFIG_LONGJMP_ENUMATTR
	DREF Attr **iter = self->ei_bufpos;
	if (iter && iter != (DREF Attr **)ITER_DONE) {
		ASSERT(iter >= self->ei_buffer);
		ASSERT(iter <= COMPILER_ENDOF(self->ei_buffer));
		for (; iter < COMPILER_ENDOF(self->ei_buffer); ++iter)
			Dee_Visit(*iter);
	}
#endif /* CONFIG_LONGJMP_ENUMATTR */
	Dee_Visit(self->ei_seq);
}


#ifdef CONFIG_LONGJMP_ENUMATTR
PRIVATE WUNUSED NONNULL((1, 2, 6)) dssize_t DCALL
enumattr_longjmp(DeeObject *__restrict declarator,
                 char const *__restrict attr_name, char const *attr_doc,
                 uint16_t perm, DeeTypeObject *attr_type,
                 EnumAttrIter *__restrict iterator) {
	int error;
	DREF Attr *new_attribute;
	ASSERT(iterator->ei_bufpos != COMPILER_ENDOF(iterator->ei_buffer));
again:
	/* Create a new descriptor for the information passed. */
	new_attribute = DeeObject_TRYMALLOC(Attr);
	if unlikely(!new_attribute)
		goto err_collect; /* Error. */
	new_attribute->a_info.a_decl     = declarator;
	new_attribute->a_info.a_perm     = perm;
	new_attribute->a_info.a_attrtype = attr_type;
	new_attribute->a_name            = attr_name;
	if (perm & ATTR_NAMEOBJ)
		Dee_Incref(COMPILER_CONTAINER_OF(attr_name, DeeStringObject, s_str));
	if (!attr_doc) {
		ASSERT(!(perm & ATTR_DOCOBJ));
		new_attribute->a_info.a_doc = NULL;
	} else if (perm & ATTR_DOCOBJ) {
		new_attribute->a_info.a_doc = attr_doc;
		Dee_Incref(COMPILER_CONTAINER_OF(attr_doc, DeeStringObject, s_str));
	} else {
		new_attribute->a_info.a_doc = attr_doc;
	}
	Dee_Incref(declarator);
	Dee_XIncref(attr_type);
	DeeObject_Init(new_attribute, &DeeAttribute_Type);

	/* Done! Now save the attribute in the collection buffer. */
	*iterator->ei_bufpos++ = new_attribute;

	/* If the buffer is not full, jump over to the
	 * caller and let them yield what we've collected. */
	if (iterator->ei_bufpos == COMPILER_ENDOF(iterator->ei_buffer)) {
		iterator->ei_bufpos = iterator->ei_buffer;
		if ((error = DeeSystem_SetJmp(iterator->ei_continue)) == 0)
			DeeSystem_LongJmp(iterator->ei_break, BRKSIG_YIELD);
		/* Stop iteration if the other end requested this. */
		if (error == CNTSIG_STOP)
			return -2;
	}
	return 0;

err_collect:
	/* Let the other end collect memory for us.
	 * With how small our stack is, we'd probably not be able to
	 * safely execute user-code that may be invoked from gc-callbacks
	 * associated with the memory collection sub-system. */
	if ((error = DeeSystem_SetJmp(iterator->ei_continue)) == 0)
		DeeSystem_LongJmp(iterator->ei_break, BRKSIG_COLLECT);

	/* Stop iteration if the other end requested this. */
	if (error == CNTSIG_STOP)
		return -2;
	goto again;
}


PRIVATE ATTR_NOINLINE ATTR_NORETURN ATTR_USED void SPCALL_CC
enumattr_start(void *arg) {
	EnumAttrIter *self = (EnumAttrIter *)arg;
	dssize_t enum_error;

	/* This is where execution on the fake stack starts. */
	self->ei_bufpos = self->ei_buffer;
	enum_error = DeeObject_EnumAttr(self->ei_seq->ea_type,
	                                self->ei_seq->ea_obj,
	                                (denum_t)&enumattr_longjmp, self);

	/* -1 indicates an internal error, rather than stop-enumeration (with is -2). */
	if unlikely(enum_error == -1) {
		/* Discard all unyielded attributes and enter an error state. */
		while (self->ei_bufpos > self->ei_buffer) {
			--self->ei_bufpos;
			Dee_Decref(*self->ei_bufpos);
		}
		self->ei_bufpos = (DREF Attr **)ITER_DONE;
		DeeSystem_LongJmp(self->ei_break, BRKSIG_ERROR);
		__builtin_unreachable();
	}

	/* Notify of the last remaining attributes (if there are any). */
	if (self->ei_bufpos != (DREF Attr **)ITER_DONE &&
	    self->ei_bufpos != self->ei_buffer) {
		size_t count   = (size_t)(self->ei_bufpos - self->ei_buffer);
		Attr **new_pos = COMPILER_ENDOF(self->ei_buffer) - count;
		ASSERT(count < CONFIG_LONGJMP_ENUMATTR_CLUSTER);
		memmoveupc(new_pos, self->ei_buffer,
		           count, sizeof(Attr *));
		self->ei_bufpos = new_pos;
		if (DeeSystem_SetJmp(self->ei_continue) == 0)
			DeeSystem_LongJmp(self->ei_break, BRKSIG_YIELD);
		ASSERT(self->ei_bufpos == self->ei_buffer ||
		       self->ei_bufpos == (DREF Attr **)ITER_DONE);
	}

	/* Mark the buffer as exhausted. */
	self->ei_bufpos = (DREF Attr **)ITER_DONE;
	DeeSystem_LongJmp(self->ei_break, BRKSIG_STOP);
	__builtin_unreachable();
}
#endif /* CONFIG_LONGJMP_ENUMATTR */


PRIVATE WUNUSED NONNULL((1)) DREF Attr *DCALL
enumattriter_next(EnumAttrIter *__restrict self) {
#ifdef CONFIG_LONGJMP_ENUMATTR
	DREF Attr *result;
	int error;
	/* Quick check: is the iterator exhausted. */
again_locked:
	DeeEnumAttrIterator_LockAcquire(self);
again:
	/* Check for case: Iterator exhausted. */
	if (self->ei_bufpos == (DREF Attr **)ITER_DONE)
		goto iter_done;
	if (self->ei_bufpos != COMPILER_ENDOF(self->ei_buffer)) {
		if (!self->ei_bufpos) {
			if (DeeSystem_SetJmp(self->ei_break) == 0) {
				/* Special case: initial call. */
				SPCALL_NORETURN(enumattr_start, self,
				                self->ei_stack,
				                sizeof(self->ei_stack));
				__builtin_unreachable();
			}
			goto again;
		}
		ASSERT(self->ei_bufpos >= self->ei_buffer);
		ASSERT(self->ei_bufpos < COMPILER_ENDOF(self->ei_buffer));

		/* Take away one of the generated attributes. */
		result = *self->ei_bufpos++;
		goto done;
	}

	/* Continue execution of the iterator. */
	self->ei_bufpos = self->ei_buffer;
	if ((error = DeeSystem_SetJmp(self->ei_break)) == 0)
		DeeSystem_LongJmp(self->ei_continue, CNTSIG_CONTINUE);

	/* Handle signal return signals from the enumeration sub-routine. */
	if (error == BRKSIG_COLLECT) {
		DeeEnumAttrIterator_LockRelease(self);
		if (Dee_CollectMemory(sizeof(Attr)))
			goto again_locked;
		return NULL;
	}
	if (error == BRKSIG_STOP)
		goto iter_done;
	if (error == BRKSIG_ERROR) {
		result = NULL;
		goto done;
	}

	/* Jump back and handle the attributes that got enumerated. */
	goto again;
iter_done:
	result = (Attr *)ITER_DONE;
done:
	DeeEnumAttrIterator_LockRelease(self);
	return result;
#else /* CONFIG_LONGJMP_ENUMATTR */
	DREF Attr **p_result;
	do {
		p_result = atomic_read(&self->ei_iter);
		if (p_result == self->ei_end)
			return (DREF Attr *)ITER_DONE;
	} while unlikely(!atomic_cmpxch_weak(&self->ei_iter, p_result, p_result + 1));
	return_reference_(*p_result);
#endif /* !CONFIG_LONGJMP_ENUMATTR */
}

PRIVATE struct type_member tpconst enumattriter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(EnumAttrIter, ei_seq), "->?Ert:EnumAttr"),
	TYPE_MEMBER_END
};

/* `(enumattr from deemon).Iterator' */
PUBLIC DeeTypeObject DeeEnumAttrIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_EnumAttrIterator",
	/* .tp_doc      = */ DOC("next->?DAttribute"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&enumattriter_init,
				TYPE_FIXED_ALLOCATOR(EnumAttrIter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&enumattriter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&enumattriter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
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
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__E31EBEB26CC72F83),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ATTRIBUTE_C */
