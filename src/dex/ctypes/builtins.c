/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_BUILTINS_C
#define GUARD_DEX_CTYPES_BUILTINS_C 1
#define DEE_SOURCE 1

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/super.h>

#undef FORMAT_TYP
#undef FORMAT_STR
#undef SIGNED_TYPE_NAME
#undef UNSIGNED_TYPE_NAME
#undef TYPE_NAME
#undef SIGNED
#undef ALIGNOF
#undef SIZEOF
#undef T
#undef X
#undef F

#ifndef __INTELLISENSE__

/* Basic integer types. */
#define SIZEOF 1
#define SIGNED 1
#include "define-integer.c.inl"
#define SIZEOF 2
#define SIGNED 1
#include "define-integer.c.inl"
#define SIZEOF 4
#define SIGNED 1
#include "define-integer.c.inl"
#define SIZEOF 8
#define SIGNED 1
#include "define-integer.c.inl"
#define SIZEOF 1
#include "define-integer.c.inl"
#define SIZEOF 2
#include "define-integer.c.inl"
#define SIZEOF 4
#include "define-integer.c.inl"
#define SIZEOF 8
#include "define-integer.c.inl"

/* `char' */
#ifndef CONFIG_CTYPES_CHAR_UNSIGNED
#define SIGNED             1
#endif  /* !CONFIG_CTYPES_CHAR_UNSIGNED */
#define NAME               "char"
#define FORMAT_STR         "%c"
#define FORMAT_TYP         int
#ifdef CONFIG_CTYPES_CHAR_UNSIGNED
#define FFI_TYPE           ffi_type_uchar
#else /* CONFIG_CTYPES_CHAR_UNSIGNED */
#define FFI_TYPE           ffi_type_schar
#endif /* !CONFIG_CTYPES_CHAR_UNSIGNED */
#define SIZEOF             CONFIG_CTYPES_SIZEOF_CHAR
#define ALIGNOF            CONFIG_CTYPES_ALIGNOF_CHAR
#define TYPE_NAME          DeeCChar_Type
#define SIGNED_TYPE_NAME   DeeCSChar_Type
#define UNSIGNED_TYPE_NAME DeeCUChar_Type
#include "define-integer.c.inl"

/* `wchar_t' */
#ifndef CONFIG_CTYPES_WCHAR_UNSIGNED
#define SIGNED                1
#endif /* !CONFIG_CTYPES_WCHAR_UNSIGNED */
#define NAME                  "wchar_t"
#define SIZEOF                CONFIG_CTYPES_SIZEOF_WCHAR
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_WCHAR
#define TYPE_NAME             DeeCWChar_Type
#define NO_SIGNED_TYPE_NAME   1
#define NO_UNSIGNED_TYPE_NAME 1
#include "define-integer.c.inl"

/* `char16_t' */
#ifndef CONFIG_CTYPES_CHAR16_UNSIGNED
#define SIGNED                1
#endif /* !CONFIG_CTYPES_CHAR16_UNSIGNED */
#define NAME                  "char16_t"
#define SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR16
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR16
#define TYPE_NAME             DeeCChar16_Type
#define NO_SIGNED_TYPE_NAME   1
#define NO_UNSIGNED_TYPE_NAME 1
#include "define-integer.c.inl"

/* `char32_t' */
#ifndef CONFIG_CTYPES_CHAR32_UNSIGNED
#define SIGNED                1
#endif /* !CONFIG_CTYPES_CHAR32_UNSIGNED */
#define NAME                  "char32_t"
#define SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR32
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR32
#define TYPE_NAME             DeeCChar32_Type
#define NO_SIGNED_TYPE_NAME   1
#define NO_UNSIGNED_TYPE_NAME 1
#include "define-integer.c.inl"

/* `bool' */
#define NAME                  "bool"
#define SIZEOF                CONFIG_CTYPES_SIZEOF_BOOL
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_BOOL
#define TYPE_NAME             DeeCBool_Type
#define NO_SIGNED_TYPE_NAME   1
#define NO_UNSIGNED_TYPE_NAME 1
#define CONFIG_BOOL_STRING 1
#include "define-integer.c.inl"
#undef CONFIG_BOOL_STRING

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
/* `signed char' */
#define SIGNED                1
#define FORMAT_STR            "%hhd"
#define FORMAT_TYP            signed char
#define T                     signed char
#define FFI_TYPE              ffi_type_schar
#define SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR
#define TYPE_NAME             DeeCSChar_Type
#define SIGNED_TYPE_NAME      DeeCSChar_Type
#define UNSIGNED_TYPE_NAME    DeeCUChar_Type
#include "define-integer.c.inl"
/* `unsigned char' */
#define FORMAT_STR            "%hhu"
#define FORMAT_TYP            unsigned char
#define T                     unsigned char
#define FFI_TYPE              ffi_type_uchar
#define SIZEOF                CONFIG_CTYPES_SIZEOF_CHAR
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_CHAR
#define TYPE_NAME             DeeCUChar_Type
#define SIGNED_TYPE_NAME      DeeCSChar_Type
#define UNSIGNED_TYPE_NAME    DeeCUChar_Type
#include "define-integer.c.inl"
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
/* `signed short' */
#define SIGNED                1
#define FORMAT_STR            "%hd"
#define FORMAT_TYP            signed short
#define T                     signed short
#define FFI_TYPE              ffi_type_sshort
#define SIZEOF                CONFIG_CTYPES_SIZEOF_SHORT
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_SHORT
#define TYPE_NAME             DeeCShort_Type
#define SIGNED_TYPE_NAME      DeeCShort_Type
#define UNSIGNED_TYPE_NAME    DeeCUShort_Type
#include "define-integer.c.inl"
/* `unsigned short' */
#define FORMAT_STR            "%hu"
#define FORMAT_TYP            unsigned short
#define T                     unsigned short
#define FFI_TYPE              ffi_type_ushort
#define SIZEOF                CONFIG_CTYPES_SIZEOF_SHORT
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_SHORT
#define TYPE_NAME             DeeCUShort_Type
#define SIGNED_TYPE_NAME      DeeCShort_Type
#define UNSIGNED_TYPE_NAME    DeeCUShort_Type
#include "define-integer.c.inl"
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
/* `signed int' */
#define SIGNED                1
#define FORMAT_STR            "%d"
#define FORMAT_TYP            signed int
#define T                     signed int
#define FFI_TYPE              ffi_type_sint
#define SIZEOF                CONFIG_CTYPES_SIZEOF_INT
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_INT
#define TYPE_NAME             DeeCInt_Type
#define SIGNED_TYPE_NAME      DeeCInt_Type
#define UNSIGNED_TYPE_NAME    DeeCUInt_Type
#include "define-integer.c.inl"
/* `unsigned int' */
#define FORMAT_STR            "%u"
#define FORMAT_TYP            unsigned int
#define T                     unsigned int
#define FFI_TYPE              ffi_type_uint
#define SIZEOF                CONFIG_CTYPES_SIZEOF_INT
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_INT
#define TYPE_NAME             DeeCUInt_Type
#define SIGNED_TYPE_NAME      DeeCInt_Type
#define UNSIGNED_TYPE_NAME    DeeCUInt_Type
#include "define-integer.c.inl"
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
/* `signed long' */
#define SIGNED                1
#define FORMAT_STR            "%ld"
#define FORMAT_TYP            signed long
#define T                     signed long
#define FFI_TYPE              ffi_type_slong
#define SIZEOF                CONFIG_CTYPES_SIZEOF_LONG
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_LONG
#define TYPE_NAME             DeeCLong_Type
#define SIGNED_TYPE_NAME      DeeCLong_Type
#define UNSIGNED_TYPE_NAME    DeeCULong_Type
#include "define-integer.c.inl"
/* `unsigned long' */
#define FORMAT_STR            "%lu"
#define FORMAT_TYP            unsigned long
#define T                     unsigned long
#define FFI_TYPE              ffi_type_ulong
#define SIZEOF                CONFIG_CTYPES_SIZEOF_LONG
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_LONG
#define TYPE_NAME             DeeCULong_Type
#define SIGNED_TYPE_NAME      DeeCLong_Type
#define UNSIGNED_TYPE_NAME    DeeCULong_Type
#include "define-integer.c.inl"
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
/* `signed long long' */
#define SIGNED                1
#define FORMAT_STR            "%lld"
#define FORMAT_TYP            signed long long
#define T                     signed long long
#define SIZEOF                CONFIG_CTYPES_SIZEOF_LLONG
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_LLONG
#define TYPE_NAME             DeeCLLong_Type
#define SIGNED_TYPE_NAME      DeeCLLong_Type
#define UNSIGNED_TYPE_NAME    DeeCULLong_Type
#include "define-integer.c.inl"
/* `unsigned long long' */
#define FORMAT_STR            "%llu"
#define FORMAT_TYP            unsigned long long
#define T                     unsigned long long
#define SIZEOF                CONFIG_CTYPES_SIZEOF_LLONG
#define ALIGNOF               CONFIG_CTYPES_ALIGNOF_LLONG
#define TYPE_NAME             DeeCULLong_Type
#define SIGNED_TYPE_NAME      DeeCLLong_Type
#define UNSIGNED_TYPE_NAME    DeeCULLong_Type
#include "define-integer.c.inl"
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */

/* Floating point types. */
#define NAME       "float"
#define TYPE_NAME  DeeCFloat_Type
#define T          CONFIG_CTYPES_FLOAT_TYPE
#include "define-float.c.inl"

#define NAME       "double"
#define TYPE_NAME  DeeCDouble_Type
#define T          CONFIG_CTYPES_DOUBLE_TYPE
#include "define-float.c.inl"

#define NAME       "long_double"
#define TYPE_NAME  DeeCLDouble_Type
#define T          CONFIG_CTYPES_LDOUBLE_TYPE
#include "define-float.c.inl"

#endif /* !__INTELLISENSE__ */

DECL_BEGIN

PRIVATE DEFINE_STRING(str_void, "void");

PRIVATE WUNUSED DREF DeeObject *DCALL
void_str(DeeSTypeObject *__restrict UNUSED(tp_self), void *UNUSED(self)) {
	return_reference_((DeeObject *)&str_void);
}

PRIVATE int DCALL
void_init(DeeSTypeObject *__restrict UNUSED(tp_self),
          void *UNUSED(self), size_t UNUSED(argc),
          DeeObject **UNUSED(argv)) {
	/* Emulate void-casting, which simply discards its operand. */
	return 0;
}

INTERN DeeSTypeObject DeeCVoid_Type = {
	/* .st_base = */ {
		OBJECT_HEAD_INIT((DeeTypeObject *)&DeeSType_Type),
		/* .tp_name     = */ DeeString_STR(&str_void),
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FNAMEOBJECT | TP_FINHERITCTOR,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeeStructured_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ NULL,
					/* .tp_copy_ctor = */ NULL,
					/* .tp_deep_ctor = */ NULL,
					/* .tp_any_ctor  = */ NULL,
					TYPE_FIXED_ALLOCATOR_S(DeeObject)
				}
			},
			/* .tp_dtor        = */ NULL,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
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
	/* .st_cachelock = */ RWLOCK_INIT,
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
		/* .st_repr = */ &void_str,
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
