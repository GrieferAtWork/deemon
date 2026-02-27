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
#ifndef GUARD_DEX_CTYPES_BUILTINS_C
#define GUARD_DEX_CTYPES_BUILTINS_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/numeric.h>   /* DeeNumeric_Type */
#include <deemon/object.h>    /* DREF, DeeObject, DeeObject_Type, DeeTypeObject, Dee_AsObject, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/string.h>    /* DeeString_STR */
#include <deemon/type.h>      /* Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO, TF_NONE, TP_F* */
#include <deemon/util/lock.h> /* Dee_ATOMIC_RWLOCK_INIT */

#include <stddef.h> /* NULL, size_t */

#undef DEFINE_PRINTF_TYP
#undef DEFINE_PRINTF_FMT
#undef DEFINE_TYPE_NAME_SIGNED
#undef DEFINE_TYPE_NAME_UNSIGNED
#undef DEFINE_TYPE_NAME
#undef DEFINE_SIGNED
#undef DEFINE_ALIGNOF
#undef DEFINE_SIZEOF
#undef DEFINE_T

DECL_BEGIN

/* MRO vector for built-in C numeric types defined by `ctypes'.
 * Semantically equivalent to:
 * >> class SomeInteger: ctypes.Structured, deemon.Numeric {
 * >>     ...
 * >> }
 */
PRIVATE DeeTypeObject *tpconst ctypes_numeric_mro[] = {
	DeeSType_AsType(&DeeStructured_Type),
	&DeeNumeric_Type,
	&DeeObject_Type,
	NULL
};

DECL_END

#ifndef __INTELLISENSE__

/* Basic integer types. */
#define DEFINE_SIZEOF 1
#define DEFINE_SIGNED 1
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 2
#define DEFINE_SIGNED 1
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 4
#define DEFINE_SIGNED 1
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 8
#define DEFINE_SIGNED 1
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 16
#define DEFINE_SIGNED 1
#include "define-integer.c.inl"

#define DEFINE_SIZEOF 1
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 2
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 4
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 8
#include "define-integer.c.inl"
#define DEFINE_SIZEOF 16
#include "define-integer.c.inl"

/* `char' */
#ifndef CONFIG_CTYPES_CHAR_UNSIGNED
#define DEFINE_SIGNED             1
#endif  /* !CONFIG_CTYPES_CHAR_UNSIGNED */
#define DEFINE_NAME               "char"
#define DEFINE_PRINTF_FMT         "%c"
#define DEFINE_PRINTF_TYP         int
#ifdef CONFIG_CTYPES_CHAR_UNSIGNED
#define DEFINE_FFI_TYPE           ffi_type_uchar
#else /* CONFIG_CTYPES_CHAR_UNSIGNED */
#define DEFINE_FFI_TYPE           ffi_type_schar
#endif /* !CONFIG_CTYPES_CHAR_UNSIGNED */
#define DEFINE_SIZEOF             CONFIG_CTYPES_SIZEOF_CHAR
#define DEFINE_ALIGNOF            CONFIG_CTYPES_ALIGNOF_CHAR
#define DEFINE_TYPE_NAME          DeeCChar_Type
#define DEFINE_TYPE_NAME_SIGNED   DeeCSChar_Type
#define DEFINE_TYPE_NAME_UNSIGNED DeeCUChar_Type
#include "define-integer.c.inl"

/* `wchar_t' */
#ifndef CONFIG_CTYPES_WCHAR_UNSIGNED
#define DEFINE_SIGNED                1
#endif /* !CONFIG_CTYPES_WCHAR_UNSIGNED */
#define DEFINE_NAME                  "wchar_t"
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_WCHAR
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_WCHAR
#define DEFINE_TYPE_NAME             DeeCWChar_Type
#define DEFINE_NO_SIGNED_TYPE_NAME   1
#define DEFINE_NO_UNSIGNED_TYPE_NAME 1
#include "define-integer.c.inl"

/* `char16_t' */
#ifndef CONFIG_CTYPES_CHAR16_UNSIGNED
#define DEFINE_SIGNED                1
#endif /* !CONFIG_CTYPES_CHAR16_UNSIGNED */
#define DEFINE_NAME                  "char16_t"
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR16
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR16
#define DEFINE_TYPE_NAME             DeeCChar16_Type
#define DEFINE_NO_SIGNED_TYPE_NAME   1
#define DEFINE_NO_UNSIGNED_TYPE_NAME 1
#include "define-integer.c.inl"

/* `char32_t' */
#ifndef CONFIG_CTYPES_CHAR32_UNSIGNED
#define DEFINE_SIGNED                1
#endif /* !CONFIG_CTYPES_CHAR32_UNSIGNED */
#define DEFINE_NAME                  "char32_t"
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR32
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR32
#define DEFINE_TYPE_NAME             DeeCChar32_Type
#define DEFINE_NO_SIGNED_TYPE_NAME   1
#define DEFINE_NO_UNSIGNED_TYPE_NAME 1
#include "define-integer.c.inl"

/* `bool' */
#define DEFINE_NAME                  "bool"
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_BOOL
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_BOOL
#define DEFINE_TYPE_NAME             DeeCBool_Type
#define DEFINE_NO_SIGNED_TYPE_NAME   1
#define DEFINE_NO_UNSIGNED_TYPE_NAME 1
#define CONFIG_BOOL_STRING
#include "define-integer.c.inl"
#undef CONFIG_BOOL_STRING

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
/* `signed char' */
#define DEFINE_SIGNED                1
#define DEFINE_PRINTF_FMT            "%hhd"
#define DEFINE_PRINTF_TYP            signed char
#define DEFINE_T                     signed char
#define DEFINE_FFI_TYPE              ffi_type_schar
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR
#define DEFINE_TYPE_NAME             DeeCSChar_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCSChar_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCUChar_Type
#include "define-integer.c.inl"
/* `unsigned char' */
#define DEFINE_PRINTF_FMT            "%hhu"
#define DEFINE_PRINTF_TYP            unsigned char
#define DEFINE_T                     unsigned char
#define DEFINE_FFI_TYPE              ffi_type_uchar
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR
#define DEFINE_TYPE_NAME             DeeCUChar_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCSChar_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCUChar_Type
#include "define-integer.c.inl"
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
/* `signed short' */
#define DEFINE_SIGNED                1
#define DEFINE_PRINTF_FMT            "%hd"
#define DEFINE_PRINTF_TYP            signed short
#define DEFINE_T                     signed short
#define DEFINE_FFI_TYPE              ffi_type_sshort
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_SHORT
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_SHORT
#define DEFINE_TYPE_NAME             DeeCShort_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCShort_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCUShort_Type
#include "define-integer.c.inl"
/* `unsigned short' */
#define DEFINE_PRINTF_FMT            "%hu"
#define DEFINE_PRINTF_TYP            unsigned short
#define DEFINE_T                     unsigned short
#define DEFINE_FFI_TYPE              ffi_type_ushort
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_SHORT
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_SHORT
#define DEFINE_TYPE_NAME             DeeCUShort_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCShort_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCUShort_Type
#include "define-integer.c.inl"
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
/* `signed int' */
#define DEFINE_SIGNED                1
#define DEFINE_PRINTF_FMT            "%d"
#define DEFINE_PRINTF_TYP            signed int
#define DEFINE_T                     signed int
#define DEFINE_FFI_TYPE              ffi_type_sint
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_INT
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_INT
#define DEFINE_TYPE_NAME             DeeCInt_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCInt_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCUInt_Type
#include "define-integer.c.inl"
/* `unsigned int' */
#define DEFINE_PRINTF_FMT            "%u"
#define DEFINE_PRINTF_TYP            unsigned int
#define DEFINE_T                     unsigned int
#define DEFINE_FFI_TYPE              ffi_type_uint
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_INT
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_INT
#define DEFINE_TYPE_NAME             DeeCUInt_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCInt_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCUInt_Type
#include "define-integer.c.inl"
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#define CONFIG_DONT_PROMOTE_TO_INTEGER
#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
/* `signed long' */
#define DEFINE_SIGNED                1
#define DEFINE_PRINTF_FMT            "%ld"
#define DEFINE_PRINTF_TYP            signed long
#define DEFINE_T                     signed long
#define DEFINE_F(x)                  x##l
#define DEFINE_FFI_TYPE              ffi_type_slong
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_LONG
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_LONG
#define DEFINE_TYPE_NAME             DeeCLong_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCLong_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCULong_Type
#include "define-integer.c.inl"
/* `unsigned long' */
#define DEFINE_PRINTF_FMT            "%lu"
#define DEFINE_PRINTF_TYP            unsigned long
#define DEFINE_T                     unsigned long
#define DEFINE_F(x)                  x##ul
#define DEFINE_FFI_TYPE              ffi_type_ulong
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_LONG
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_LONG
#define DEFINE_TYPE_NAME             DeeCULong_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCLong_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCULong_Type
#include "define-integer.c.inl"
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
/* `signed long long' */
#define DEFINE_SIGNED                1
#define DEFINE_PRINTF_FMT            "%lld"
#define DEFINE_PRINTF_TYP            __LONGLONG
#define DEFINE_T                     __LONGLONG
#define DEFINE_F(x)                  x##ll
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_LLONG
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_LLONG
#define DEFINE_TYPE_NAME             DeeCLLong_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCLLong_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCULLong_Type
#include "define-integer.c.inl"
/* `unsigned long long' */
#define DEFINE_PRINTF_FMT            "%llu"
#define DEFINE_PRINTF_TYP            __ULONGLONG
#define DEFINE_T                     __ULONGLONG
#define DEFINE_F(x)                  x##ull
#define DEFINE_SIZEOF                CONFIG_CTYPES_SIZEOF_LLONG
#define DEFINE_ALIGNOF               CONFIG_CTYPES_ALIGNOF_LLONG
#define DEFINE_TYPE_NAME             DeeCULLong_Type
#define DEFINE_TYPE_NAME_SIGNED      DeeCLLong_Type
#define DEFINE_TYPE_NAME_UNSIGNED    DeeCULLong_Type
#include "define-integer.c.inl"
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */
#undef CONFIG_DONT_PROMOTE_TO_INTEGER

/* Floating point types. */
#define DEFINE_NAME       "float"
#define DEFINE_TYPE_NAME  DeeCFloat_Type
#define DEFINE_T          CONFIG_CTYPES_FLOAT_TYPE
#include "define-float.c.inl"

#define DEFINE_NAME       "double"
#define DEFINE_TYPE_NAME  DeeCDouble_Type
#define DEFINE_T          CONFIG_CTYPES_DOUBLE_TYPE
#include "define-float.c.inl"

#define DEFINE_NAME       "long_double"
#define DEFINE_TYPE_NAME  DeeCLDouble_Type
#define DEFINE_T          CONFIG_CTYPES_LDOUBLE_TYPE
#include "define-float.c.inl"

#endif /* !__INTELLISENSE__ */

DECL_BEGIN

PRIVATE DEFINE_STRING(str_void, "void");
PRIVATE DEFINE_STRING(repr_void, "void()");

PRIVATE WUNUSED DREF DeeObject *DCALL
void_str(DeeSTypeObject *__restrict UNUSED(tp_self), void *UNUSED(self)) {
	return_reference_(Dee_AsObject(&str_void));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
void_repr(DeeSTypeObject *__restrict UNUSED(tp_self), void *UNUSED(self)) {
	return_reference_(Dee_AsObject(&repr_void));
}

PRIVATE int DCALL
void_init(DeeSTypeObject *__restrict UNUSED(tp_self),
          void *UNUSED(self), size_t UNUSED(argc),
          DeeObject *const *UNUSED(argv)) {
	/* Emulate void-casting, which simply discards its operand. */
	return 0;
}

INTERN DeeSTypeObject DeeCVoid_Type = {
	/* .st_base = */ {
		OBJECT_HEAD_INIT(&DeeSType_Type),
		/* .tp_name     = */ DeeString_STR(&str_void),
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FNAMEOBJECT | TP_FINHERITCTOR,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ DeeSType_AsType(&DeeStructured_Type),
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO(
				/* DEFINE_T:              */ DeeObject,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ NULL,
				/* tp_serialize:   */ NULL /* TODO */
			),
			/* .tp_dtor        = */ NULL,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ NULL,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
#ifndef CONFIG_NO_THREADS
	/* .st_cachelock = */ Dee_ATOMIC_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .st_pointer  = */ NULL,
	/* .st_lvalue   = */ NULL,
	/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
	/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
	/* .st_ffitype  = */ &ffi_type_void,
#endif /* !CONFIG_NO_CFUNCTION */
	/* .st_sizeof   = */ 0,
	/* .st_align    = */ 0,
	/* .st_init     = */ &void_init,
	/* .st_assign   = */ NULL,
	/* .st_cast     = */ {
		/* .st_str  = */ &void_str,
		/* .st_repr = */ &void_repr,
		/* .st_bool = */ NULL
	},
	/* .st_call     = */ NULL,
	/* .st_math     = */ NULL,
	/* .st_cmp      = */ NULL,
	/* .st_seq      = */ NULL,
	/* .st_attr     = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_CTYPES_BUILTINS_C */
