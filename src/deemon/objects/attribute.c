/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_ATTRIBUTE_C
#define GUARD_DEEMON_OBJECTS_ATTRIBUTE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h>

#include <hybrid/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifdef _MSC_VER
#pragma warning(disable: 4611) /* Some nonsensical warning about how setjmp() is evil... */
#endif /* _MSC_VER */

DECL_BEGIN

typedef DeeAttributeObject        Attr;
typedef DeeEnumAttrObject         EnumAttr;
typedef DeeEnumAttrIteratorObject EnumAttrIter;


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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_str(Attr *__restrict self) {
	return DeeString_Newf("%k.%s",
	                      self->a_info.a_decl,
	                      self->a_name);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
attr_hash(Attr *__restrict self) {
	dhash_t result;
	result = (DeeObject_Hash(self->a_info.a_decl) ^
	          Dee_HashPointer(self->a_info.a_attrtype) ^
	          self->a_info.a_perm);
	if (self->a_info.a_perm & ATTR_NAMEOBJ) {
		result ^= DeeString_Hash((DeeObject *)COMPILER_CONTAINER_OF(self->a_name,
		                                                            DeeStringObject,
		                                                            s_str));
	} else {
		result ^= Dee_HashStr(self->a_name);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
attr_eq(Attr *self, Attr *other) {
	int result;
	if (DeeObject_AssertType((DeeObject *)other, &DeeAttribute_Type))
		goto err;
	if (self->a_info.a_attrtype != other->a_info.a_attrtype)
		goto nope;
	if (self->a_info.a_decl != other->a_info.a_decl) {
		result = DeeObject_CompareEq(self->a_info.a_decl,
		                             other->a_info.a_decl);
		if unlikely(result < 0)
			goto err;
		if (!result)
			goto nope;
	}
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
		if (DeeString_SIZE(my_name) != DeeString_SIZE(ot_name))
			goto nope;
		if (memcmp(DeeString_STR(my_name), DeeString_STR(ot_name), DeeString_SIZE(ot_name) * sizeof(char)) != 0)
			goto nope;
	} else {
		if (strcmp(self->a_name, other->a_name) != 0)
			goto nope;
	}
yup:
	return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE struct type_cmp attr_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&attr_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&attr_eq
};

PRIVATE struct type_member attr_members[] = {
	TYPE_MEMBER_FIELD_DOC("decl", STRUCT_OBJECT, offsetof(Attr, a_info.a_decl),
	                      "The type or object that is declaring this Attribute"),
	TYPE_MEMBER_FIELD_DOC("attrtype", STRUCT_OBJECT_OPT, offsetof(Attr, a_info.a_attrtype),
	                      "->?X2?DType?N\n"
	                      "The type of this Attribute, or ?N if not known"),
	TYPE_MEMBER_BITFIELD_DOC("canget", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMGET,
	                         "Check if the Attribute has a way of being read from"),
	TYPE_MEMBER_BITFIELD_DOC("candel", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMDEL,
	                         "Check if the Attribute has a way of being deleted"),
	TYPE_MEMBER_BITFIELD_DOC("canset", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMSET,
	                         "Check if the Attribute has a way of being written to"),
	TYPE_MEMBER_BITFIELD_DOC("cancall", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PERMCALL,
	                         "Returns ?t if the Attribute is intended to be called as a function. "
	                         "Note that this feature alone does not meant that the Attribute really can, or "
	                         "cannot be called, only that calling it as a function might be the inteded use."),
	TYPE_MEMBER_BITFIELD_DOC("isprivate", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PRIVATE,
	                         "Check if the Attribute is considered to be private\n"
	                         "Private attributes only appear in user-classes, prohibiting access to only thiscall "
	                         "functions with a this-argument that is an instance of the declaring class."),
	TYPE_MEMBER_BITFIELD_DOC("isproperty", STRUCT_CONST, Attr, a_info.a_perm, ATTR_PROPERTY,
	                         "Check if the Attribute is property-like, meaning that access by "
	                         "reading, deletion, or writing causes unpredictable side-effects"),
	TYPE_MEMBER_BITFIELD_DOC("iswrapper", STRUCT_CONST, Attr, a_info.a_perm, ATTR_WRAPPER,
	                         "Check if the Attribute is accessed from the implementing type, which "
	                         "exposes it as a wrapper for an instance member (e.g. ${string.find} is an unbound "
	                         "wrapper (aka. ${Attribute(string,\"find\").iswrapper == true}) for the instance function, "
	                         "member or property that would be bound in ${\"foo\".find} (aka. "
	                         "${Attribute(\"foo\", \"find\").iswrapper == false}))"),
	TYPE_MEMBER_BITFIELD_DOC("isinstance", STRUCT_CONST, Attr, a_info.a_perm, ATTR_IMEMBER,
	                         "Check if accessing this Attribute requires an instance of the declaring object "
	                         "?#decl, rather than being an Attribute of the declaring object ?#decl itself.\n"
	                         "Note that practically all attributes, such as member functions, are available as both "
	                         "instance and class Attribute, while in other cases an Attribute will evaluate to different "
	                         "objects depending on being invoked on a class or an instance (such as :Dict.keys)"),
	TYPE_MEMBER_BITFIELD_DOC("isclass", STRUCT_CONST, Attr, a_info.a_perm, ATTR_CMEMBER,
	                         "Check if access to this Attribute must be made though the declaring type ?#decl.\n"
	                         "To test if an Attribute can only be accessed through an instance, use ?#isinstance instead"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
attr_get_name(Attr *__restrict self) {
	DREF DeeStringObject *result;
	char const *name_str;
	uint16_t perm;
again:
	name_str = self->a_name;
	__hybrid_atomic_thread_fence(__ATOMIC_ACQUIRE);
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
			if (!ATOMIC_CMPXCH(self->a_name, name_str, DeeString_STR(result))) {
				Dee_Decref(result);
				goto again;
			}
			ATOMIC_FETCHOR(self->a_info.a_perm, ATTR_NAMEOBJ);
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
	__hybrid_atomic_thread_fence(__ATOMIC_ACQUIRE);
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
			if (!ATOMIC_CMPXCH(self->a_info.a_doc, doc_str, DeeString_STR(result))) {
				Dee_Decref(result);
				goto again;
			}
			ATOMIC_FETCHOR(self->a_info.a_perm, ATTR_DOCOBJ);
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
	uint16_t mask          = self->a_info.a_perm & 0x1ff;
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
attr_repr(Attr *__restrict self) {
	DREF DeeObject *flags_str, *result;
	flags_str = attr_getflags(self);
	result = DeeString_Newf("Attribute(%r,%q,%r,%r,%r)",
	                        self->a_info.a_decl,
	                        self->a_name,
	                        flags_str,
	                        flags_str,
	                        self->a_info.a_decl);
	Dee_Decref(flags_str);
	return result;
}



PRIVATE struct type_getset attr_getsets[] = {
	{ "name", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_get_name, NULL, NULL,
	  DOC("->?Dstring\n"
	      "The name of this Attribute") },
	{ "doc", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_get_doc, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "The documentation string of this Attribute, or ?N when no documentation is present") },
	{ "flags", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_getflags, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Return a set of characters descripting the flags of @this Attribute:\n"
	      "#T{Character|Mnemonic|Field|Flag description~"
	      "$\"g\"|get|?#canget|The Attribute has a way of being read from&"
	      "$\"d\"|del|?#candel|The Attribute has a way of being deleted&"
	      "$\"s\"|set|?#canset|The Attribute has a way of being written to&"
	      "$\"f\"|function|?#cancall|The Attribute is intended to be called as a function&"
	      "$\"i\"|instance|?#isinstance|The Attribute requires an instance of the declaring object&"
	      "$\"c\"|class|?#isclass|The Attribute is accessed though the declaring type ?#decl&"
	      "$\"h\"|hidden|?#isprivate|The Attribute is considered to be private&"
	      "$\"p\"|property|?#isproperty|The Attribute is property-like&"
	      "$\"w\"|wrapper|?#iswrapper|The Attribute is provided by the type as a class member that wraps around an instance member}") },
	{ NULL }
};


LOCAL int DCALL
string_to_attrflags(char const *__restrict str,
                    uint16_t *__restrict presult) {
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
		*presult |= (uint16_t)1 << i;
	}
	return 0;
err:
	return -1;
}

PRIVATE DEFINE_KWLIST(attrinit_kwlist, { K(ob), K(name), K(flagmask), K(flagval), K(decl), KEND });

PRIVATE int DCALL
attribute_init(DeeAttributeObject *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	int lookup_error;
	DeeObject *search_self, *search_name;
	DeeObject *flagmask = NULL, *flagval = NULL;
	struct attribute_lookup_rules rules;
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = 0;
	rules.alr_perm_value = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, attrinit_kwlist, "oo|ooo:attribute",
	                    &search_self,
	                    &search_name,
	                    &flagmask,
	                    &flagval,
	                    &rules.alr_decl))
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
	lookup_error = DeeAttribute_Lookup(Dee_TYPE(search_self),
	                                   search_self,
	                                   &self->a_info,
	                                   &rules);
	if (lookup_error > 0) {
		/* Attribute wasn't found... */
		err_unknown_attribute_lookup(Dee_TYPE(search_self), rules.alr_name);
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
	if (DeeArg_UnpackKw(argc, argv, kw, attrinit_kwlist, "oo|ooo:exists",
	                    &search_self,
	                    &search_name,
	                    &flagmask,
	                    &flagval,
	                    &rules.alr_decl))
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
	lookup_error = DeeAttribute_Lookup(Dee_TYPE(search_self),
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
	if (DeeArg_UnpackKw(argc, argv, kw, attrinit_kwlist, "oo|ooo:lookup",
	                    &search_self,
	                    &search_name,
	                    &flagmask,
	                    &flagval,
	                    &rules.alr_decl))
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
	lookup_error = DeeAttribute_Lookup(Dee_TYPE(search_self),
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

PRIVATE struct type_method attr_class_methods[] = {
	{ "exists", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&attribute_exists,
	  DOC("(ob,name:?Dstring,flagmask:?X2?Dint?Dstring=!P{},flagval:?X2?Dint?Dstring=!VAflagmask,decl?)->?Dbool\n"
	      "@throw ValueError The given @flagmask or @flagval contains an unrecognized flag character\n"
	      "Taking the same arguments as ?#{op:constructor}, check if the an attribute matching "
	      "the given arguments exists, returning ?t/?f indicative of this\n"
	      "${"
	      "static function exists(ob, name, flagmask = \"\", flagval = \"\", decl?) {\n"
	      "	import Error from deemon;\n"
	      "	try {\n"
	      "		attribute(ob, name, flagmask, flagval, decl);\n"
	      "	} catch (Error.AttributeError) {\n"
	      "		return false;\n"
	      "	}\n"
	      "	return true;\n"
	      "}}"),
	  TYPE_METHOD_FKWDS },
	{ "lookup", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&attribute_lookup,
	  DOC("(ob,name:?Dstring,flagmask:?X2?Dint?Dstring=!P{},flagval:?X2?Dint?Dstring=!VAflagmask,decl?)->?X2?.?N\n"
	      "@throw ValueError The given @flagmask or @flagval contains an unrecognized flag character\n"
	      "Same as ?#{op:constructor}, but return ?N if the attribute doesn't exist\n"
	      "${"
	      "static function lookup(ob, name, flagmask = \"\", flagval = \"\", decl?) {\n"
	      "	import Error from deemon;\n"
	      "	try {\n"
	      "		return attribute(ob, name, flagmask, flagval, decl);\n"
	      "	} catch (Error.AttributeError) {\n"
	      "		return none;\n"
	      "	}\n"
	      "}}"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};



PUBLIC DeeTypeObject DeeAttribute_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Attribute),
	/* .tp_doc      = */ DOC("The descriptor object for abstract object attributes\n"

	                         "\n"
	                         "(ob,name:?Dstring,flagmask:?X2?Dint?Dstring=!P{},flagval:?X2?Dint?Dstring=!VAflagmask,decl?)\n"
	                         "@param flagmask Set of attribute flags to mask when searching for matches (s.a. ?#flags)\n"
	                         "@param flagval Set of attribute flags required when searching for matches (s.a. ?#flags) "
	                                        "(When only this is given, and @flagmask is omit (as possible when "
	                                        "using keyword arguments), flagmask is set to @flagval)\n"
	                         "@throw AttributeError No attribute matching the specified restrictions could be found\n"
	                         "@throw ValueError The given @flagmask or @flagval contains an unrecognized flag character\n"
	                         "Lookup an Attribute enumerated by ${enumattr(ob)} or ${enumattr(tp)}, matching "
	                         "the given @name, as well as having its set of flags match @flagval, when masked by @flagmask\n"
	                         "Additionally, @decl may be specified to narrow down valid matches to only those declared by it\n"
	                         "${"
	                         "function findattr(ob: Object, name: string, flagmask: int | string,\n"
	                         "                  flagval: int | string, decl?: Object): Attribute {\n"
	                         "	import enumattr, Error, HashSet from deemon;\n"
	                         "	flagmask = HashSet(flagmask);\n"
	                         "	for (local attr: enumattr(ob)) {\n"
	                         "		if (attr.name != name)\n"
	                         "			continue;\n"
	                         "		if (decl is bound && attr.decl !== decl)\n"
	                         "			continue;\n"
	                         "		if ((flagmask & attr.flags) != flagval)\n"
	                         "			continue;\n"
	                         "		return attr;\n"
	                         "	}\n"
	                         "	throw Error.AttributeError(...);\n"
	                         "}}\n"
	                         "Using @flagmask and @flagval, you can easily restrict a search to only class-, or instance-attributes:\n"
	                         "${"
	                         "import Attribute, Dict from deemon;\n"
	                         "/* The class-variant (Attribute cannot be accessed from an instance) */\n"
	                         "print repr Attribute(Dict, \"keys\", \"i\", \"\");\n"
	                         "/* The class-variant (Attribute is a wrapper) */\n"
	                         "print repr Attribute(Dict, \"keys\", \"w\");\n"
	                         "/* The instance-variant (Attribute can be accessed from an instance) */\n"
	                         "print repr Attribute(Dict, \"keys\", \"i\");\n"
	                         "/* The instance-variant (Attribute isn't a wrapper) */\n"
	                         "print repr Attribute(Dict, \"keys\", \"w\", \"\");"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ NULL,
				/* .tp_copy_ctor   = */ NULL,
				/* .tp_deep_ctor   = */ NULL,
				/* .tp_any_ctor    = */ NULL,
				TYPE_FIXED_ALLOCATOR(Attr),
				/* .tp_any_ctor_kw = */ &attribute_init
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&attr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&attr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &attr_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ attr_getsets,
	/* .tp_members       = */ attr_members,
	/* .tp_class_methods = */ attr_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		new_vector = (DREF Attr **)Dee_TryRealloc(self->al_v, new_alloc *
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
#endif


PRIVATE WUNUSED NONNULL((1)) int DCALL
enumattr_init(EnumAttr *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeObject *a, *b = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:enumattr", &a, &b))
		return -1;
	if (b) {
		if (DeeObject_AssertType(a, &DeeType_Type) ||
		    DeeObject_AssertType(b, (DeeTypeObject *)a))
			return -1;
		self->ea_type = (DREF DeeTypeObject *)a;
		self->ea_obj  = b;
		Dee_Incref(a);
		Dee_Incref(b);
	} else {
		self->ea_type = Dee_TYPE(a);
		self->ea_obj  = a;
		Dee_Incref(Dee_TYPE(a));
		Dee_Incref(a);
	}
#ifndef CONFIG_LONGJMP_ENUMATTR
	/* Collect all attributes */
	{
		struct attr_list list;
		list.al_a = list.al_c = 0;
		list.al_v             = NULL;
		/* Enumerate all the attributes. */
		if (DeeObject_EnumAttr(self->ea_type, self->ea_obj, (denum_t)&save_attr, &list) < 0) {
			while (list.al_c--)
				Dee_Decref(list.al_v[list.al_c]);
			Dee_Free(list.al_v);
			Dee_Decref(self->ea_type);
			Dee_XDecref(self->ea_obj);
			return -1;
		}
		/* Truncate the collection vector. */
		if (list.al_c != list.al_a) {
			DREF Attr **new_vector;
			new_vector = (DREF Attr **)Dee_TryRealloc(list.al_v,
			                                          list.al_c * sizeof(DREF Attr *));
			if likely(new_vector)
				list.al_v = new_vector;
		}
		/* Assign the attribute vector. */
		self->ea_attrc = list.al_c;
		self->ea_attrv = list.al_v; /* Inherit. */
	}
#endif
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
enumattr_fini(EnumAttr *__restrict self) {
	Dee_Decref(self->ea_type);
	Dee_XDecref(self->ea_obj);
#ifndef CONFIG_LONGJMP_ENUMATTR
	{
		size_t i;
		for (i = 0; i < self->ea_attrc; ++i)
			Dee_Decref(self->ea_attrv[i]);
	}
	Dee_Free(self->ea_attrv);
#endif
}

PRIVATE NONNULL((1, 2)) void DCALL
enumattr_visit(EnumAttr *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ea_type);
	Dee_XVisit(self->ea_obj);
#ifndef CONFIG_LONGJMP_ENUMATTR
	{
		size_t i;
		for (i = 0; i < self->ea_attrc; ++i)
			Dee_Visit(self->ea_attrv[i]);
	}
#endif
}

PRIVATE NONNULL((1, 2)) void DCALL
enumattriter_setup(EnumAttrIter *__restrict self,
                   EnumAttr *__restrict seq) {
	self->ei_seq = seq;
	Dee_Incref(seq);
#ifdef CONFIG_LONGJMP_ENUMATTR
	rwlock_init(&self->ei_lock);
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

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
enumattr_hash(EnumAttr *__restrict self) {
	return ((self->ea_obj ? DeeObject_Hash(self->ea_obj) : 0) ^
	        Dee_HashPointer(self->ea_type));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
enumattr_eq(EnumAttr *self,
            EnumAttr *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &DeeEnumAttr_Type))
		return NULL;
	if (self->ea_type != other->ea_type)
		return_false;
	return DeeObject_CompareEqObject(self->ea_obj, other->ea_obj);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
enumattr_ne(EnumAttr *self,
            EnumAttr *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &DeeEnumAttr_Type))
		return NULL;
	if (self->ea_type != other->ea_type)
		return_true;
	return DeeObject_CompareNeObject(self->ea_obj, other->ea_obj);
}

PRIVATE struct type_cmp enumattr_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&enumattr_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&enumattr_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&enumattr_ne
};

PRIVATE struct type_seq enumattr_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&enumattr_iter
};

PRIVATE struct type_member enumattr_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeEnumAttrIterator_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeEnumAttr_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_enumattr),
	/* .tp_doc      = */ DOC("(tp:?DType)\n"
	                         "Enumerate attributes of the :Type @tp and its bases\n"

	                         "\n"
	                         "(ob)\n"
	                         "Same as ${enumattr(type(ob), ob)}\n"

	                         "\n"
	                         "(tp:?DType,ob)\n"
	                         "Create a new sequence for enumerating the :{attribute}s of a given object.\n"
	                         "When @tp is given, only enumerate objects implemented by @tp or "
	                         "one of its bases and those accessible through a superview of @ob using @tp.\n"
	                         "Note that iterating this object may be expensive, and that conversion to "
	                         "a different sequence before iterating multiple times may be desirable"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT|TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &enumattr_init,
				TYPE_FIXED_ALLOCATOR(EnumAttr)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&enumattr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&enumattr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &enumattr_cmp,
	/* .tp_seq           = */ &enumattr_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */enumattr_class_members
};

#ifdef CONFIG_LONGJMP_ENUMATTR
/* Signal values with which `ei_continue' can be invoked. */
#define CNTSIG_CONTINUE  1 /* Continue execution. */
#define CNTSIG_STOP      2 /* Tear down execution. */
#define BRKSIG_YIELD     1 /* Yield more items. */
#define BRKSIG_ERROR     2 /* An error occurred. */
#define BRKSIG_STOP      3 /* Stop yielding items. */
#define BRKSIG_COLLECT   4 /* Collect memory and try again. */
#endif /* CONFIG_LONGJMP_ENUMATTR */


PRIVATE WUNUSED NONNULL((1)) int DCALL
enumattriter_init(EnumAttrIter *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	EnumAttr *seq;
	if (DeeArg_Unpack(argc, argv, "o:_EnumAttrIterator", &seq))
		goto err;
	if (DeeObject_AssertType((DeeObject *)seq, &DeeEnumAttr_Type))
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
		error = setjmp(self->ei_break);
		if (error == 0)
			longjmp(self->ei_continue, CNTSIG_STOP);
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

PRIVATE dssize_t DCALL
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
	Dee_XIncref(declarator);
	Dee_XIncref(attr_type);
	DeeObject_Init(new_attribute, &DeeAttribute_Type);
	/* Done! Now save the attribute in the collection buffer. */
	*iterator->ei_bufpos++ = new_attribute;
	/* If the buffer is not full, jump over to the
	 * caller and let them yield what we've collected. */
	if (iterator->ei_bufpos == COMPILER_ENDOF(iterator->ei_buffer)) {
		iterator->ei_bufpos = iterator->ei_buffer;
		if ((error = setjmp(iterator->ei_continue)) == 0)
			longjmp(iterator->ei_break, BRKSIG_YIELD);
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
	if ((error = setjmp(iterator->ei_continue)) == 0)
		longjmp(iterator->ei_break, BRKSIG_COLLECT);
	/* Stop iteration if the other end requested this. */
	if (error == CNTSIG_STOP)
		return -2;
	goto again;
}


#if defined(CONFIG_HOST_WINDOWS) && defined(__x86_64__)
ATTR_MSABI
#endif
PRIVATE ATTR_NOINLINE ATTR_NORETURN ATTR_USED void
enumattr_start(EnumAttrIter *__restrict self) {
	dssize_t enum_error;
	/* This is where execution on the fake stack starts. */
	self->ei_bufpos = self->ei_buffer;
	enum_error = DeeObject_EnumAttr(self->ei_seq->ea_type,
	                                self->ei_seq->ea_obj,
	                                (denum_t)&enumattr_longjmp, self);
	/* -1 indicates an internal error, rather than stop-enumeration (with is -2). */
	if unlikely(enum_error == -1) {
		/* Discard all unyielded attributes and enter an error state. */
		while (self->ei_bufpos != self->ei_buffer) {
			--self->ei_bufpos;
			Dee_Decref(*self->ei_bufpos);
		}
		self->ei_bufpos = (DREF Attr **)ITER_DONE;
		longjmp(self->ei_break, BRKSIG_ERROR);
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
		if (setjmp(self->ei_continue) == 0)
			longjmp(self->ei_break, BRKSIG_YIELD);
		ASSERT(self->ei_bufpos == self->ei_buffer);
	}
	/* Mark the buffer as exhausted. */
	self->ei_bufpos = (DREF Attr **)ITER_DONE;
	longjmp(self->ei_break, BRKSIG_STOP);
	__builtin_unreachable();
}
#endif

PRIVATE WUNUSED NONNULL((1)) DREF Attr *DCALL
enumattriter_next(EnumAttrIter *__restrict self) {
#ifdef CONFIG_LONGJMP_ENUMATTR
	DREF Attr *result;
	int error;
	/* Quick check: is the iterator exhausted. */
again_locked:
#ifndef CONFIG_NO_THREADS
	rwlock_write(&self->ei_lock);
#endif /* !CONFIG_NO_THREADS */
again:
	/* Check for case: Iterator exhausted. */
	if (self->ei_bufpos == (DREF Attr **)ITER_DONE)
		goto iter_done;
	if (self->ei_bufpos != COMPILER_ENDOF(self->ei_buffer)) {
		if (!self->ei_bufpos) {
			/* Special case: initial call. */
			uintptr_t *new_sp;
			/* TODO: Special handling for user-defined enumattr operators. */
			new_sp = COMPILER_ENDOF(self->ei_stack);
#ifndef __x86_64__
			*--new_sp = (uintptr_t)self; /* First argument to `enumattr_start' */
#endif /* __x86_64__ */
			if (setjmp(self->ei_break) == 0) {
				/* Set the  */
#ifdef __COMPILER_HAVE_GCC_ASM
				__asm__ __volatile__(""
#ifdef __x86_64__
				                     "movq %0, %%rsp\n\t"
#ifdef CONFIG_HOST_WINDOWS
				                     "subq $32, %%rsp\n\t" /* 32: Register scratch area... */
#endif /* CONFIG_HOST_WINDOWS */
#else /* __x86_64__ */
				                     "movl %0, %%esp\n\t"
#endif /* !__x86_64__ */
				                     "call "
#ifdef __USER_LABEL_PREFIX__
				                     PP_STR(__USER_LABEL_PREFIX__)
#endif /* __USER_LABEL_PREFIX__ */
				                     "enumattr_start\n\t"
				                     :
				                     : "g" (new_sp)
#ifdef __x86_64__
#ifdef CONFIG_HOST_WINDOWS
				                     , "c" (self)
#else /* CONFIG_HOST_WINDOWS */
				                     , "D" (self)
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* __x86_64__ */
				                     );
#else /* __COMPILER_HAVE_GCC_ASM */
				__asm {
#ifdef __x86_64__
#ifdef CONFIG_HOST_WINDOWS
					MOV  RCX, self
#else /* CONFIG_HOST_WINDOWS */
					MOV  RDI, self
#endif /* !CONFIG_HOST_WINDOWS */
#ifdef CONFIG_HOST_WINDOWS
					LEA  RSP, [new_sp - 32] /* 32: Register scratch area... */
#else /* CONFIG_HOST_WINDOWS */
					MOV  RSP, new_sp
#endif /* !CONFIG_HOST_WINDOWS */
#else /* __x86_64__ */
					MOV  ESP, new_sp
#endif /* !__x86_64__ */
					CALL enumattr_start
				}
#endif /* !__COMPILER_HAVE_GCC_ASM */
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
	if ((error = setjmp(self->ei_break)) == 0)
		longjmp(self->ei_continue, CNTSIG_CONTINUE);
	/* Handle signal return signals from the enumeration sub-routine. */
	if (error == BRKSIG_COLLECT) {
#ifndef CONFIG_NO_THREADS
		rwlock_endwrite(&self->ei_lock);
#endif /* !CONFIG_NO_THREADS */
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
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(&self->ei_lock);
#endif /* !CONFIG_NO_THREADS */
	return result;
#else /* CONFIG_LONGJMP_ENUMATTR */
	DREF Attr **presult;
	do {
		presult = ATOMIC_READ(self->ei_iter);
		if (presult == self->ei_end)
			return (DREF Attr *)ITER_DONE;
	} while (!ATOMIC_CMPXCH(self->ei_iter, presult, presult + 1));
	return_reference_(*presult);
#endif /* !CONFIG_LONGJMP_ENUMATTR */
}


PRIVATE struct type_member enumattriter_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(EnumAttrIter, ei_seq)),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeEnumAttrIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_EnumAttrIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)NULL,
				/* .tp_copy_ctor = */ (void *)NULL,
				/* .tp_deep_ctor = */ (void *)NULL,
				/* .tp_any_ctor  = */ (void *)&enumattriter_init,
				TYPE_FIXED_ALLOCATOR(EnumAttrIter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&enumattriter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&enumattriter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&enumattriter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ enumattriter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




struct attribute_lookup_data {
	struct attribute_info               *ald_info;    /* [1..1] The result info. */
	struct attribute_lookup_rules const *ald_rules;   /* [1..1] Lookup rules */
	bool                                 ald_fnddecl; /* [valid_if(ald_rules->alr_decl != NULL)]
	                                                   * Set to true after `alr_decl' had been encountered. */
};

PRIVATE dssize_t DCALL
attribute_lookup_enum(DeeObject *__restrict declarator,
                      char const *__restrict attr_name, char const *attr_doc,
                      uint16_t perm, DeeTypeObject *attr_type,
                      struct attribute_lookup_data *__restrict arg) {
	dhash_t attr_hash;
	struct attribute_info *result;
	struct attribute_lookup_rules const *rules = arg->ald_rules;
	if (rules->alr_decl) {
		if (declarator != rules->alr_decl) {
			if (arg->ald_fnddecl)
				return -3; /* The requested declarator came and went without a match... */
			return 0;
		}
		arg->ald_fnddecl = true;
	}
	if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
		return 0;
	if (perm & ATTR_NAMEOBJ)
		attr_hash = DeeString_Hash((DeeObject *)COMPILER_CONTAINER_OF(attr_name, DeeStringObject, s_str));
	else {
		attr_hash = Dee_HashStr(attr_name);
	}
	if (attr_hash != rules->alr_hash)
		return 0;
	if (strcmp(attr_name, arg->ald_rules->alr_name) != 0)
		return 0;
	/* This is the one! */
	result = arg->ald_info;
	if (!attr_doc) {
		ASSERT(!(perm & ATTR_DOCOBJ));
		result->a_doc = NULL;
	} else if (perm & ATTR_DOCOBJ) {
		result->a_doc = attr_doc;
		Dee_Incref(COMPILER_CONTAINER_OF(attr_doc, DeeStringObject, s_str));
	} else {
		result->a_doc = attr_doc;
	}
	result->a_decl = declarator;
	Dee_Incref(declarator);
	result->a_perm     = perm;
	result->a_attrtype = attr_type;
	Dee_XIncref(attr_type);
	return -2; /* Stop enumeration! */
}


INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
type_enumattr(DeeTypeObject *UNUSED(tp_self),
              DeeObject *self, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
module_enumattr(DeeTypeObject *UNUSED(tp_self),
                DeeObject *self, denum_t proc, void *arg);


PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeAttribute_Lookup(DeeTypeObject *tp_self, DeeObject *self,
                    struct attribute_info *__restrict result,
                    struct attribute_lookup_rules const *__restrict rules) {
	int error;
	DeeTypeObject *iter = tp_self;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	ASSERT(result);
	ASSERT(rules);
	ASSERT(rules->alr_name);
	ASSERT_OBJECT_OPT(rules->alr_decl);
	if (tp_self->tp_attr)
		goto do_iter_attr;
	/* Search through the cache for the requested attribute. */
	if ((error = DeeType_FindCachedAttr(tp_self, self, result, rules)) <= 0)
		goto done;
	for (;;) {
		if (rules->alr_decl && iter != (DeeTypeObject *)rules->alr_decl)
			goto next_iter;
		if (DeeType_IsClass(iter)) {
			if ((error = DeeClass_FindInstanceAttribute(tp_self, iter, self, result, rules)) <= 0)
				goto done;
		} else {
			if (iter->tp_methods &&
			    (error = DeeType_FindMethodAttr(tp_self, iter, result, rules)) <= 0)
				goto done;
			if (iter->tp_getsets &&
			    (error = DeeType_FindGetSetAttr(tp_self, iter, result, rules)) <= 0)
				goto done;
			if (iter->tp_members &&
			    (error = DeeType_FindMemberAttr(tp_self, iter, result, rules)) <= 0)
				goto done;
		}
next_iter:
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			dssize_t enum_error;
			struct attribute_lookup_data data;
			dssize_t (DCALL *enumattr)(DeeTypeObject *, DeeObject *, denum_t, void *);
do_iter_attr:
			enumattr = iter->tp_attr->tp_enumattr;
			if (!enumattr)
				break;
			if (enumattr == &type_enumattr)
				return DeeType_FindAttrString((DeeTypeObject *)self, result, rules);
			if (enumattr == &module_enumattr)
				return DeeModule_FindAttrString((DeeModuleObject *)self, result, rules);
			data.ald_info    = result;
			data.ald_rules   = rules;
			data.ald_fnddecl = false;
			enum_error       = (*enumattr)(iter, self, (denum_t)&attribute_lookup_enum, &data);
			if (enum_error == 0 || enum_error == -3) /* Not found */
				break; /* Don't consider attributes from lower levels for custom member access. */
			if (enum_error == -1)
				return -1;            /* Error... */
			ASSERT(enum_error == -2); /* Found it! */
			return 0;
		}
	}
	return 1; /* Not found */
done:
	return error;
}




DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ATTRIBUTE_C */
