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
#ifndef GUARD_DEX_CTYPES_CTYPES_C
#define GUARD_DEX_CTYPES_CTYPES_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
#include <deemon/api.h>

#include <deemon/bool.h>            /* DeeBool_Type */
#include <deemon/bytes.h>           /* DeeBytes_Check, DeeBytes_DATA */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/float.h>           /* DeeFloat_Type */
#include <deemon/format.h>          /* DeeFormat_PRINT, DeeFormat_Printf, PRFuSIZ */
#include <deemon/int.h>             /* DeeInt_* */
#include <deemon/none.h>            /* DeeNone_Check, DeeNone_Type */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_TypeAssertFailed, DeeTypeObject, Dee_Decref, Dee_Decref_unlikely, Dee_TYPE, Dee_XDecref, Dee_XIncref, Dee_formatprinter_t, Dee_ssize_t */
#include <deemon/string.h>          /* DeeString*, STRING_ERROR_FREPLAC */
#include <deemon/system-features.h> /* DeeSystem_DEFINE_strcmp, strlen */
#include <deemon/type.h>            /* DeeType_GetName */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

#ifdef CONFIG_NO_CFUNCTION
INTERN WUNUSED NONNULL((1)) ctypes_cc_t DCALL
cc_trylookup(char const *__restrict UNUSED(name)) {
	return CC_INVALID;
}

INTERN WUNUSED char const *DCALL
cc_getname(ctypes_cc_t UNUSED(cc)) {
	return NULL;
}
#else /* CONFIG_NO_CFUNCTION */

struct cc_entry {
	char        name[12];
	ctypes_cc_t cc;
};

#ifndef CONFIG_HAVE_SYSTEM_FFI
#ifdef X86_WIN32
#define CONFIG_HAVE_FFI_SYSV
#define CONFIG_HAVE_FFI_STDCALL
#define CONFIG_HAVE_FFI_THISCALL
#define CONFIG_HAVE_FFI_FASTCALL
#define CONFIG_HAVE_FFI_MS_CDECL
#define CONFIG_HAVE_FFI_PASCAL
#define CONFIG_HAVE_FFI_REGISTER
#elif defined(X86_WIN64)
#define CONFIG_HAVE_FFI_WIN64
#else /* ... */
#define CONFIG_HAVE_FFI_SYSV
#define CONFIG_HAVE_FFI_UNIX64
#define CONFIG_HAVE_FFI_THISCALL
#define CONFIG_HAVE_FFI_FASTCALL
#define CONFIG_HAVE_FFI_STDCALL
#define CONFIG_HAVE_FFI_PASCAL
#define CONFIG_HAVE_FFI_REGISTER
#endif /* !... */
#endif /* !CONFIG_HAVE_SYSTEM_FFI */

PRIVATE struct cc_entry const cc_db[] = {
#ifdef CONFIG_HAVE_FFI_SYSV
	{ "sysv", FFI_SYSV },
#endif /* CONFIG_HAVE_FFI_SYSV */
#ifdef CONFIG_HAVE_FFI_STDCALL
	{ "stdcall", FFI_STDCALL },
#endif /* CONFIG_HAVE_FFI_STDCALL */
#ifdef CONFIG_HAVE_FFI_THISCALL
	{ "thiscall", FFI_THISCALL },
#endif /* CONFIG_HAVE_FFI_THISCALL */
#ifdef CONFIG_HAVE_FFI_FASTCALL
	{ "fastcall", FFI_FASTCALL },
#endif /* CONFIG_HAVE_FFI_FASTCALL */
#ifdef CONFIG_HAVE_FFI_MS_CDECL
	{ "ms_cdecl", FFI_MS_CDECL },
#endif /* CONFIG_HAVE_FFI_MS_CDECL */
#ifdef CONFIG_HAVE_FFI_PASCAL
	{ "pascal", FFI_PASCAL },
#endif /* CONFIG_HAVE_FFI_PASCAL */
#ifdef CONFIG_HAVE_FFI_REGISTER
	{ "register", FFI_REGISTER },
#endif /* CONFIG_HAVE_FFI_REGISTER */
#ifdef CONFIG_HAVE_FFI_WIN64
	{ "win64", FFI_WIN64 },
#endif /* CONFIG_HAVE_FFI_WIN64 */
#ifdef CONFIG_HAVE_FFI_UNIX64
	{ "unix64", FFI_UNIX64 },
#endif /* CONFIG_HAVE_FFI_UNIX64 */
#ifdef CONFIG_HAVE_FFI_GNUW64
	{ "gnuw64", FFI_GNUW64 },
#endif /* CONFIG_HAVE_FFI_GNUW64 */
#ifdef CONFIG_HAVE_FFI_EFI64
	{ "efi64", FFI_EFI64 },
#endif /* CONFIG_HAVE_FFI_EFI64 */
	{ "", FFI_DEFAULT_ABI },
	{ "default", FFI_DEFAULT_ABI },
};

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */


INTERN WUNUSED NONNULL((1)) ctypes_cc_t DCALL
cc_trylookup(char const *__restrict name) {
	size_t i;
	/* Search for a calling convention matching the given name. */
	for (i = 0; i < COMPILER_LENOF(cc_db); ++i) {
		if (strcmp(cc_db[i].name, name) == 0)
			return cc_db[i].cc;
	}
	return CC_INVALID;
}

INTERN WUNUSED char const *DCALL
cc_getname(ctypes_cc_t cc) {
	size_t i;
	/* Search for the database entry for the given CC. */
	for (i = 0; i < COMPILER_LENOF(cc_db); ++i) {
		if (cc_db[i].cc == cc)
			return cc_db[i].name;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1)) ffi_type *DCALL
CType_GetFFIType(CType *__restrict self) {
	if (self->ct_ffitype)
		return self->ct_ffitype;
	/* TODO: Lazily allocate struct descriptors. */

	DeeError_NOTIMPLEMENTED();
	return NULL;
}
#endif /* !CONFIG_NO_CFUNCTION */


INTERN WUNUSED NONNULL((1)) ctypes_cc_t DCALL
cc_lookup(char const *__restrict name) {
#ifdef CONFIG_NO_CFUNCTION
	DeeError_Throwf(&DeeError_ValueError,
	                "Unrecognized calling convention %q",
	                name);
	return CC_INVALID;
#else /* CONFIG_NO_CFUNCTION */
	ctypes_cc_t result = cc_trylookup(name);
	if (result == CC_INVALID) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Unrecognized calling convention %q",
		                name);
	}
	return result;
#endif /* !CONFIG_NO_CFUNCTION */
}



PRIVATE WUNUSED NONNULL((1)) CType *DCALL
CType_FromDeemonType(DeeTypeObject *__restrict self) {
	/* Map some builtin types to their structured counterparts. */
	if (DeeNone_Check(self) || self == &DeeNone_Type)
		return &CVoid_Type;
	if (self == &DeeBool_Type)
		return &CBool_Type;
	if (self == &DeeInt_Type)
		return &CInt_Type;
	if (self == &DeeFloat_Type)
		return &CDouble_Type;
	return NULL;
}

/* Return the structured type equivalent of `self', or
 * re-return `self' if it already is a structured type.
 * The following types found in the builtin `deemon' module are mapped:
 *   - `none from deemon'       --> `void from ctypes'
 *   - `type(none from deemon)' --> `void from ctypes'
 *   - `bool from deemon'       --> `bool from ctypes'
 *   - `int from deemon'        --> `int from ctypes'
 *   - `float from deemon'      --> `double from ctypes'
 * If `self' is not one of these mappings and also not
 * a c-type, a TypeError is thrown and NULL is returned.
 * WARNING: This function does not return a reference! */
INTERN WUNUSED NONNULL((1)) CType *DCALL
CType_Of(DeeObject *__restrict self) {
	CType *result;
	if (Object_IsCType(self))
		return Object_AsCType(self);

	/* Map some builtin types to their structured counterparts. */
	result = CType_FromDeemonType((DeeTypeObject *)self);
	if (result)
		return result;

	/* Throw a type-assertion failure error. */
	DeeObject_TypeAssertFailed(self, &CType_Type);
	return NULL;
}


/* Same as `DeeSType_Get()', but also able to handle the
 * case where "self" is an *instance*, rather a some type. */
INTERN WUNUSED NONNULL((1)) CType *DCALL
CType_TypeOf(DeeObject *__restrict self) {
	CType *result;
	if (Object_IsCType(self))
		return Object_AsCType(self);
	if (Object_IsCObject(self))
		return Dee_TYPE(Object_AsCObject(self));

	/* Map some builtin types to their structured counterparts. */
	result = CType_FromDeemonType((DeeTypeObject *)self);
	if (result)
		return result;
	result = CType_FromDeemonType(Dee_TYPE(self));
	if (result)
		return result;

	/* Throw a type-assertion failure error. */
	DeeObject_TypeAssertFailed(self, &CType_Type);
	return NULL;
}


#define EDO(err, expr)                   \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CStructType_PrintCRepr(CStructType *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	bool is_first = true;
	struct cstruct_field *field;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "struct {");
	if unlikely(result < 0)
		goto done;
	CStructType_ForeachField(field, self) {
		if (is_first) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
			is_first = false;
		}
		ASSERT(cstruct_field_getname(field));
		EDO(err_temp, CType_PrintCRepr(cstruct_field_gettype(field), printer, arg,
		                               DeeString_STR(cstruct_field_getname(field))));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "; "));
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
done:
	return result;
err_temp:
	return temp;
}

STATIC_ASSERT(offsetof(CPointerType, cpt_orig) == offsetof(CLValueType, clt_orig));
PRIVATE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
CType_PrintCRepr_Head(CType *__restrict self,
                      Dee_formatprinter_t printer, void *arg,
                      bool *p_need_space_before_name) {
	Dee_ssize_t temp, result;
	if (CType_IsCArrayType(self)) {
		CArrayType *me = CType_AsCArrayType(self);
		CType *inner = CArrayType_PointedToType(me);
		bool need_paren = CType_IsCFunctionType(inner);
		result = CType_PrintCRepr_Head(inner, printer, arg, p_need_space_before_name);
		if unlikely(result < 0)
			goto done;
		if (need_paren) {
			if (*p_need_space_before_name) {
				EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
				*p_need_space_before_name = false;
			}
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
		}
	} else if (CType_IsCStructType(self)) {
		result = CStructType_PrintCRepr(CType_AsCStructType(self), printer, arg);
		*p_need_space_before_name = true;
#ifndef CONFIG_NO_CFUNCTION
	} else if (CType_IsCFunctionType(self)) {
		CFunctionType *me = CType_AsCFunctionType(self);
		CType *return_type = CFunction_ReturnType(me);
		result = CType_PrintCRepr_Head(return_type, printer, arg, p_need_space_before_name);
		if unlikely(result < 0)
			goto done;
		if ((me->cft_cc & ~CC_FVARARGS) != CC_DEFAULT) {
			char const *cc_name;
			if (*p_need_space_before_name) {
				EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
			}
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
			cc_name = cc_getname((ctypes_cc_t)(me->cft_cc & ~CC_FVARARGS));
			if unlikely(!cc_name)
				cc_name = "?";
			EDO(err_temp, (*printer)(arg, cc_name, strlen(cc_name)));
			*p_need_space_before_name = true;
		}
#endif /* !CONFIG_NO_CFUNCTION */
	} else if (CType_IsCPointerType(self) || CType_IsCLValueType(self)) {
		char marker[2];
		CType *inner = ((CPointerType *)self)->cpt_orig;
		bool need_paren = CType_IsCArrayType(inner) ||
		                  CType_IsCFunctionType(inner);
		result = CType_PrintCRepr_Head(inner, printer, arg, p_need_space_before_name);
		if unlikely(result < 0)
			goto done;
		if (*p_need_space_before_name) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
		}
		marker[1] = CType_IsCPointerType(self) ? '*' : '&';
		if (need_paren) {
			marker[0] = '(';
			temp = (*printer)(arg, marker, 2);
		} else {
			temp = (*printer)(arg, &marker[1], 1);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		*p_need_space_before_name = false;
	} else {
		char const *name = DeeType_GetName(CType_AsType(self));
		result = (*printer)(arg, name, strlen(name));
	}
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CType_PrintCRepr_Tail(CType *__restrict self,
                      Dee_formatprinter_t printer,
                      void *arg) {
	Dee_ssize_t temp, result;
	CType *tail = NULL;
	if (CType_IsCArrayType(self)) {
		bool need_paren;
		CArrayType *me = CType_AsCArrayType(self);
		tail = CArrayType_PointedToType(me);
		need_paren = CType_IsCFunctionType(tail);
		result = DeeFormat_Printf(printer, arg, "%s[%" PRFuSIZ "]",
		                          need_paren ? ")" : "",
		                          CArrayType_Count(me));
		if unlikely(result < 0)
			goto done;
	} else if (CType_IsCStructType(self)) {
		result = 0;
#ifndef CONFIG_NO_CFUNCTION
	} else if (CType_IsCFunctionType(self)) {
		size_t i, argc;
		CFunctionType *me = CType_AsCFunctionType(self);
		tail = CFunction_ReturnType(me);
		result = 0;
		if ((me->cft_cc & ~CC_FVARARGS) != CC_DEFAULT)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
		argc = CFunction_ArgumentCount(me);
		if (argc == 0) {
			temp = DeeFormat_PRINT(printer, arg, "(void)");
		} else {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
			for (i = 0; i < argc; ++i) {
				CType *arg_type;
				if (i != 0)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
				arg_type = CFunction_ArgumentType(me, i);
				EDO(err_temp, CType_PrintCRepr(arg_type, printer, arg, NULL));
			}
			temp = DeeFormat_PRINT(printer, arg, ")");
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
#endif /* !CONFIG_NO_CFUNCTION */
	} else if (CType_IsCPointerType(self) || CType_IsCLValueType(self)) {
		bool need_paren;
		tail = ((CPointerType *)self)->cpt_orig;
		need_paren = CType_IsCArrayType(tail) ||
		             CType_IsCFunctionType(tail);
		result = 0;
		if (need_paren) {
			result = DeeFormat_PRINT(printer, arg, ")");
			if unlikely(result < 0)
				goto done;
		}
	} else {
		result = 0;
	}
	if (tail)
		EDO(err_temp, CType_PrintCRepr_Tail(tail, printer, arg));
done:
	return result;
err_temp:
	return temp;
}


/* Print the C-representation of "self" for the purposes of a variable "varname"
 * Note that "varname" may be `NULL' or an empty string, in which case the type
 * is printed anonymously. */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CType_PrintCRepr(CType *__restrict self, Dee_formatprinter_t printer,
                 void *arg, char const *varname) {
	Dee_ssize_t temp, result;
	bool need_space_before_name = false;
	result = CType_PrintCRepr_Head(self, printer, arg, &need_space_before_name);
	if unlikely(result < 0)
		goto done;
	if (varname) {
		if (need_space_before_name)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
		EDO(err_temp, (*printer)(arg, varname, strlen(varname)));
	}
	EDO(err_temp, CType_PrintCRepr_Tail(self, printer, arg));
done:
	return result;
err_temp:
	return temp;
}




PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CStructType_PrintDRepr(CStructType *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	bool is_first = true;
	struct cstruct_field *field;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "struct {");
	if unlikely(result < 0)
		goto done;
	CStructType_ForeachField(field, self) {
		if (!is_first)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ","));
		ASSERT(cstruct_field_getname(field));
		EDO(err_temp, DeeFormat_Printf(printer, arg, " %r: ", cstruct_field_getname(field)));
		EDO(err_temp, CType_PrintDRepr(cstruct_field_gettype(field), printer, arg));
		is_first = false;
	}
	EDO(err_temp, is_first ? DeeFormat_PRINT(printer, arg, "}")
	                       : DeeFormat_PRINT(printer, arg, " }"));
done:
	return result;
err_temp:
	return temp;
}


/* Print the deemon representation of "self" (iow: the deemon expression to form "self") */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CType_PrintDRepr(CType *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (CType_IsCArrayType(self)) {
		CArrayType *me = CType_AsCArrayType(self);
		result = CType_PrintDRepr(CArrayType_PointedToType(me), printer, arg);
		if unlikely(result < 0)
			goto done;
		EDO(err_temp, DeeFormat_Printf(printer, arg, "[%" PRFuSIZ "]",
		                               CArrayType_Count(me)));
	} else if (CType_IsCStructType(self)) {
		result = CStructType_PrintDRepr(CType_AsCStructType(self), printer, arg);
#ifndef CONFIG_NO_CFUNCTION
	} else if (CType_IsCFunctionType(self)) {
		CFunctionType *me = CType_AsCFunctionType(self);
		CType *return_type = CFunction_ReturnType(me);
		result = CType_PrintDRepr(return_type, printer, arg);
		if unlikely(result < 0)
			goto done;
		if (me->cft_cc != CC_DEFAULT) {
			size_t i;
			EDO(err_temp, ctypes_cc_isvarargs(me->cft_cc)
			              ? DeeFormat_PRINT(printer, arg, ".vfunc(")
			              : DeeFormat_PRINT(printer, arg, ".func("));
			if ((me->cft_cc & ~CC_FVARARGS) != CC_DEFAULT) {
				char const *cc_name = cc_getname((ctypes_cc_t)(me->cft_cc & ~CC_FVARARGS));
				if unlikely(!cc_name)
					cc_name = "?";
				EDO(err_temp, DeeFormat_Printf(printer, arg, "%q", cc_name));
			}
			for (i = 0; i < CFunction_ArgumentCount(me); ++i) {
				CType *arg_type = CFunction_ArgumentType(me, i);
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
				EDO(err_temp, CType_PrintDRepr(arg_type, printer, arg));
			}
		} else if (CFunction_ArgumentCount(me) == 0) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "(void"));
		} else {
			size_t i;
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
			for (i = 0; i < CFunction_ArgumentCount(me); ++i) {
				CType *arg_type = CFunction_ArgumentType(me, i);
				if (i != 0)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
				EDO(err_temp, CType_PrintDRepr(arg_type, printer, arg));
			}
		}
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
#endif /* !CONFIG_NO_CFUNCTION */
	} else if (CType_IsCPointerType(self)) {
		CPointerType *me = CType_AsCPointerType(self);
		result = CType_PrintDRepr(CPointerType_PointedToType(me), printer, arg);
		if unlikely(result < 0)
			goto done;
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ".ptr"));
	} else if (CType_IsCLValueType(self)) {
		CLValueType *me = CType_AsCLValueType(self);
		result = CType_PrintDRepr(CLValueType_PointedToType(me), printer, arg);
		if unlikely(result < 0)
			goto done;
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ".lvalue"));
	} else {
		char const *name = DeeType_GetName(CType_AsType(self));
		result = (*printer)(arg, name, strlen(name));
	}
done:
	return result;
err_temp:
	return temp;
}




/* Interpret `self' as a pointer and store the result in `*result'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPointer(DeeObject *self, CType *pointer_base,
                    union pointer *__restrict result) {
	int error;
	DREF CPointerType *pointer_type;
	error = DeeObject_TryAsPointer(self, pointer_base, result);
	if (error <= 0)
		return error;
	/* Emit a type-assertion error. */
	pointer_type = CPointerType_Of(pointer_base);
	if (pointer_type) {
		DeeObject_TypeAssertFailed(self, CPointerType_AsType(pointer_type));
		Dee_Decref(CPointerType_AsType(pointer_type));
	}
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPPointer(DeeObject *self, CType *pointer_base_base,
                     void ***__restrict p_result) {
	int error;
	DREF CPointerType *pointer_base;
	pointer_base = CPointerType_Of(pointer_base_base);
	if unlikely(!pointer_base)
		goto err;
	error = DeeObject_AsPointer(self, CPointerType_AsCType(pointer_base),
	                            (union pointer *)p_result);
	Dee_Decref(CPointerType_AsType(pointer_base));
	return error;
err:
	return -1;
}


/* Same as `DeeObject_AsPointer()', but only ~try~ to interpret it. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsPointer(DeeObject *self,
                       CType *pointer_base,
                       union pointer *__restrict result) {
	if (DeeNone_Check(self)) {
		/* none is the NULL pointer. */
null_pointer:
		result->ptr = NULL;
		return 0;
	}

	if (Object_IsCPointer(self)) {
		CPointer *me = Object_AsCPointer(self);
		CType *base = CPointerType_PointedToType(Dee_TYPE(me));
		/* A pointer of the same type, or a void-pointer. */
		if (base == pointer_base ||
		    base == &CVoid_Type ||
		    pointer_base == &CVoid_Type) {
			*result = me->cp_value;
			return 0;
		}
		goto nope;
	}

	/* int(0) also counts as a NULL-pointer. */
	if (DeeInt_Check(self)) {
		if (DeeInt_IsZero(self))
			goto null_pointer;
		goto nope;
	}

	/* Special handling for strings (which can be cast to `char *') */
	if (DeeString_Check(self)) {
		if (pointer_base == &CVoid_Type) {
			result->pcvoid = DeeString_STR(self);
			return 0;
		}
		if (pointer_base == &CChar_Type) {
			result->pcvoid = DeeString_AsUtf8(self);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		if (pointer_base == &CWChar_Type) {
			result->pcvoid = DeeString_AsWide(self);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		if (pointer_base == &CChar16_Type) {
			result->pcvoid = DeeString_AsUtf16(self, STRING_ERROR_FREPLAC);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		if (pointer_base == &CChar32_Type) {
			result->pcvoid = DeeString_AsUtf32(self);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		goto nope;
	}

	/* Taking the pointer of a Bytes object yield the buffer's base */
	if (DeeBytes_Check(self)) {
		if (pointer_base == &CVoid_Type ||
		    pointer_base == &CChar_Type ||
		    pointer_base == &CInt8_Type ||
		    pointer_base == &CUInt8_Type) {
			result->ptr = DeeBytes_DATA(self);
			return 0;
		}
		goto nope;
	}

	/* Special handling for lvalue->pointer */
	if (Object_IsCLValue(self)) {
		CLValue *me = Object_AsCLValue(self);
		CType *lv_base = CLValueType_PointedToType(Dee_TYPE(me));
		if (CType_IsCPointerType(lv_base)) {
			CType *base = CPointerType_PointedToType(CType_AsCPointerType(lv_base));
			/* A pointer of the same type, or a void-pointer. */
			if (base == pointer_base ||
			    base == &CVoid_Type ||
			    pointer_base == &CVoid_Type) {
				/* LValue -> Pointer (must deref) */
				CTYPES_FAULTPROTECT(result->ptr = *me->cl_value.pptr, goto err);
				return 0;
			}
		}
		goto nope;
	}

	/* Conversion failed. */
nope:
	return 1;
err:
	return -1;
}

/* S.a. `DeeObject_TryAsGenericPointer()'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsGenericPointer(DeeObject *self,
                           CType **__restrict p_pointer_base,
                           union pointer *__restrict result) {
	int error;
	error = DeeObject_TryAsGenericPointer(self, p_pointer_base, result);
	if (error <= 0)
		return error;
	return DeeObject_TypeAssertFailed(self, CPointerType_AsType(&DeePointer_Type));
}

/* Similar to `DeeObject_TryAsPointer()', but fills in `*p_pointer_base' with the
 * pointer-base type. For use with type-generic functions (such as the `atomic_*' api)
 * @return:  1: The conversion failed.
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsGenericPointer(DeeObject *self,
                              CType **__restrict p_pointer_base,
                              union pointer *__restrict result) {
	if (Object_IsCPointer(self)) {
		CPointer *me = Object_AsCPointer(self);
		CType *base = CPointerType_PointedToType(Dee_TYPE(me));
		*result = me->cp_value;
		*p_pointer_base = base;
		return 0;
	}

	if (DeeBytes_Check(self)) {
		result->ptr = DeeBytes_DATA(self);
		*p_pointer_base = &CUInt8_Type;
		return 0;
	}

	/* Special handling for lvalue->pointer */
	if (Object_IsCLValue(self)) {
		CLValue *me = Object_AsCLValue(self);
		CType *lv_base = CLValueType_PointedToType(Dee_TYPE(me));
		if (CType_IsCPointerType(lv_base)) {
			*p_pointer_base = CPointerType_PointedToType(CType_AsCPointerType(lv_base));
			/* LValue -> Pointer (must deref) */
			CTYPES_FAULTPROTECT(result->ptr = *me->cl_value.pptr, goto err);
			return 0;
		}
		goto nope;
	}

	/* Conversion failed. */
nope:
	return 1;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
err:
	return -1;
#endif /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
}



/* Creating pointer objects */
INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CPointer_New(CPointerType *__restrict type, void *value) {
	DREF CPointer *result;
	result = CPointer_Alloc();
	if unlikely(!result)
		goto err;
	result->cp_owner     = NULL;
	result->cp_value.ptr = value;
	CPointer_Init(result, type);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CPointer_NewInherited(/*inherit(always)*/ DREF CPointerType *__restrict type, void *value) {
	DREF CPointer *result;
	result = CPointer_Alloc();
	if unlikely(!result)
		goto err;
	result->cp_owner     = NULL;
	result->cp_value.ptr = value;
	CPointer_InitInherited(result, type);
	return result;
err:
	Dee_Decref_unlikely(CPointerType_AsType(type));
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CPointer_NewEx(CPointerType *__restrict type, void *value, /*[0..1]*/ DeeObject *owner) {
	DREF CPointer *result;
	result = CPointer_Alloc();
	if unlikely(!result)
		goto err;
	Dee_XIncref(owner);
	result->cp_owner     = owner;
	result->cp_value.ptr = value;
	CPointer_Init(result, type);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CPointer_NewExInherited(/*inherit(always)*/ DREF CPointerType *__restrict type, void *value,
                        /*inherit(always)*/ /*[0..1]*/ DREF DeeObject *owner) {
	DREF CPointer *result;
	result = CPointer_Alloc();
	if unlikely(!result)
		goto err;
	result->cp_owner     = owner; /* Inherited */
	result->cp_value.ptr = value;
	CPointer_InitInherited(result, type);
	return result;
err:
	Dee_Decref_unlikely(CPointerType_AsType(type));
	Dee_XDecref(owner);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CPointer_For(CType *__restrict pointed_to_type, void *value) {
	DREF CPointerType *pointer_type = CPointerType_Of(pointed_to_type);
	if unlikely(!pointer_type)
		goto err;
	return CPointer_NewInherited(pointer_type, value);
err:
	return NULL;
}

DECL_END
#endif /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */

#endif /* !GUARD_DEX_CTYPES_CTYPES_C */
