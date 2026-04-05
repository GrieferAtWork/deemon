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
#ifdef __INTELLISENSE__
#include "ctypes-core.c"
//#define DEFINE_CVoid_Type
//#define DEFINE_CChar_Type
//#define DEFINE_CWChar_Type
//#define DEFINE_CChar16_Type
//#define DEFINE_CChar32_Type
//#define DEFINE_CBool_Type
//#define DEFINE_CInt8_Type
//#define DEFINE_CInt16_Type
//#define DEFINE_CInt32_Type
//#define DEFINE_CInt64_Type
//#define DEFINE_CInt128_Type
//#define DEFINE_CUInt8_Type
//#define DEFINE_CUInt16_Type
//#define DEFINE_CUInt32_Type
//#define DEFINE_CUInt64_Type
//#define DEFINE_CUInt128_Type
//#define DEFINE_CBSwapInt16_Type
//#define DEFINE_CBSwapInt32_Type
//#define DEFINE_CBSwapInt64_Type
//#define DEFINE_CBSwapInt128_Type
//#define DEFINE_CBSwapUInt16_Type
//#define DEFINE_CBSwapUInt32_Type
//#define DEFINE_CBSwapUInt64_Type
//#define DEFINE_CBSwapUInt128_Type
//#define DEFINE_CFloat_Type
//#define DEFINE_CDouble_Type
//#define DEFINE_CLDouble_Type
//#define DEFINE_CSChar_Type
//#define DEFINE_CUChar_Type
//#define DEFINE_CShort_Type
//#define DEFINE_CUShort_Type
//#define DEFINE_CInt_Type
//#define DEFINE_CUInt_Type
//#define DEFINE_CLong_Type
//#define DEFINE_CULong_Type
//#define DEFINE_CLLong_Type
#define DEFINE_CULLong_Type
#endif /* __INTELLISENSE__ */

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>   /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/bool.h>    /* Dee_False, Dee_True */
#include <deemon/numeric.h> /* DeeNumeric_Type */
#include <deemon/object.h>  /* DREF, DeeObject, DeeObject_Type, DeeTypeObject, Dee_formatprinter_t, Dee_int128_t, Dee_ssize_t, Dee_uint128_t, OBJECT_HEAD_INIT */
#include <deemon/type.h>    /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED, TF_NONE, TP_F*, TYPE_MEMBER_CONST, TYPE_MEMBER_END, type_member */

#include <hybrid/int128.h>    /* __HYBRID_INT128_INIT8N, __HYBRID_UINT128_INIT8N */
#include <hybrid/limitcore.h> /* __PRIVATE_MAX_S, __PRIVATE_MAX_U, __PRIVATE_MIN_S */

#include <stddef.h> /* NULL */
#include <stdint.h> /* intN_t, uintN_t */

#if (defined(DEFINE_CVoid_Type) +         \
     defined(DEFINE_CChar_Type) +         \
     defined(DEFINE_CWChar_Type) +        \
     defined(DEFINE_CChar16_Type) +       \
     defined(DEFINE_CChar32_Type) +       \
     defined(DEFINE_CBool_Type) +         \
     defined(DEFINE_CInt8_Type) +         \
     defined(DEFINE_CInt16_Type) +        \
     defined(DEFINE_CInt32_Type) +        \
     defined(DEFINE_CInt64_Type) +        \
     defined(DEFINE_CInt128_Type) +       \
     defined(DEFINE_CUInt8_Type) +        \
     defined(DEFINE_CUInt16_Type) +       \
     defined(DEFINE_CUInt32_Type) +       \
     defined(DEFINE_CUInt64_Type) +       \
     defined(DEFINE_CUInt128_Type) +      \
     defined(DEFINE_CBSwapInt16_Type) +   \
     defined(DEFINE_CBSwapInt32_Type) +   \
     defined(DEFINE_CBSwapInt64_Type) +   \
     defined(DEFINE_CBSwapInt128_Type) +  \
     defined(DEFINE_CBSwapUInt16_Type) +  \
     defined(DEFINE_CBSwapUInt32_Type) +  \
     defined(DEFINE_CBSwapUInt64_Type) +  \
     defined(DEFINE_CBSwapUInt128_Type) + \
     defined(DEFINE_CFloat_Type) +        \
     defined(DEFINE_CDouble_Type) +       \
     defined(DEFINE_CLDouble_Type) +      \
     defined(DEFINE_CSChar_Type) +        \
     defined(DEFINE_CUChar_Type) +        \
     defined(DEFINE_CShort_Type) +        \
     defined(DEFINE_CUShort_Type) +       \
     defined(DEFINE_CInt_Type) +          \
     defined(DEFINE_CUInt_Type) +         \
     defined(DEFINE_CLong_Type) +         \
     defined(DEFINE_CULong_Type) +        \
     defined(DEFINE_CLLong_Type) +        \
     defined(DEFINE_CULLong_Type)) != 1
#error "Must #define exactly one of these"
#endif /* ... */


#ifdef DEFINE_CVoid_Type
#define LOCAL_CObject          CVoid
#define LOCAL_CTYPES_T         void
#define LOCAL_CTYPES_typename  "void"
#define LOCAL_CTYPES_sizeof_T  0
#define LOCAL_CTYPES_alignof_T 0
#ifdef CTYPES_DEFINE_STATIC_POINTER_TYPES
#define LOCAL_CPointer_Type_PTR &CVoidPtr_Type
#endif /* CTYPES_DEFINE_STATIC_POINTER_TYPES */
#elif defined(DEFINE_CChar_Type)
#define LOCAL_CObject          CChar
#define LOCAL_CTYPES_T         CTYPES_char
#define LOCAL_CTYPES_typename  "char"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_char
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_char
#ifdef CTYPES_char_UNSIGNED
#define LOCAL_IS_UNSIGNED
#endif /* CTYPES_char_UNSIGNED */
#ifdef CTYPES_DEFINE_STATIC_POINTER_TYPES
#define LOCAL_CPointer_Type_PTR &CCharPtr_Type
#endif /* CTYPES_DEFINE_STATIC_POINTER_TYPES */
#define LOCAL_MEMBER_signed    CSChar_Type
#define LOCAL_MEMBER_unsigned  CUChar_Type
#elif defined(DEFINE_CWChar_Type)
#define LOCAL_CObject          CWChar
#define LOCAL_CTYPES_T         CTYPES_wchar_t
#define LOCAL_CTYPES_typename  "wchar_t"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_wchar_t
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_wchar_t
#elif defined(DEFINE_CChar16_Type)
#define LOCAL_CObject          CChar16
#define LOCAL_CTYPES_T         CTYPES_char16_t
#define LOCAL_CTYPES_typename  "char16_t"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_char16_t
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_char16_t
#elif defined(DEFINE_CChar32_Type)
#define LOCAL_CObject          CChar32
#define LOCAL_CTYPES_T         CTYPES_char32_t
#define LOCAL_CTYPES_typename  "char32_t"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_char32_t
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_char32_t
#elif defined(DEFINE_CBool_Type)
#define LOCAL_CObject          CBool
#define LOCAL_CTYPES_T         CTYPES_bool
#define LOCAL_CTYPES_typename  "bool"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_bool
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_bool
#elif defined(DEFINE_CInt8_Type)
#define LOCAL_CObject          CInt8
#define LOCAL_CTYPES_T         int8_t
#define LOCAL_CTYPES_typename  "int8_t"
#define LOCAL_CTYPES_sizeof_T  1
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int8_t
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CInt16_Type)
#define LOCAL_CObject          CInt16
#define LOCAL_CTYPES_T         int16_t
#define LOCAL_CTYPES_typename  "int16_t"
#define LOCAL_CTYPES_sizeof_T  2
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int16_t
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CInt32_Type)
#define LOCAL_CObject          CInt32
#define LOCAL_CTYPES_T         int32_t
#define LOCAL_CTYPES_typename  "int32_t"
#define LOCAL_CTYPES_sizeof_T  4
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int32_t
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CInt64_Type)
#define LOCAL_CObject          CInt64
#define LOCAL_CTYPES_T         int64_t
#define LOCAL_CTYPES_typename  "int64_t"
#define LOCAL_CTYPES_sizeof_T  8
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int64_t
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CInt128_Type)
#define LOCAL_CObject          CInt128
#define LOCAL_CTYPES_T         Dee_int128_t
#define LOCAL_CTYPES_typename  "int128_t"
#define LOCAL_CTYPES_sizeof_T  16
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int128_t
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CUInt8_Type)
#define LOCAL_CObject          CUInt8
#define LOCAL_CTYPES_T         uint8_t
#define LOCAL_CTYPES_typename  "uint8_t"
#define LOCAL_CTYPES_sizeof_T  1
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int8_t
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CUInt16_Type)
#define LOCAL_CObject          CUInt16
#define LOCAL_CTYPES_T         uint16_t
#define LOCAL_CTYPES_typename  "uint16_t"
#define LOCAL_CTYPES_sizeof_T  2
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int16_t
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CUInt32_Type)
#define LOCAL_CObject          CUInt32
#define LOCAL_CTYPES_T         uint32_t
#define LOCAL_CTYPES_typename  "uint32_t"
#define LOCAL_CTYPES_sizeof_T  4
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int32_t
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CUInt64_Type)
#define LOCAL_CObject          CUInt64
#define LOCAL_CTYPES_T         uint64_t
#define LOCAL_CTYPES_typename  "uint64_t"
#define LOCAL_CTYPES_sizeof_T  8
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int64_t
#define LOCAL_IS_UNSIGNED
#define LOCAL_MEMBER_signed    CUInt64_Type
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CUInt128_Type)
#define LOCAL_CObject          CUInt128
#define LOCAL_CTYPES_T         Dee_uint128_t
#define LOCAL_CTYPES_typename  "uint128_t"
#define LOCAL_CTYPES_sizeof_T  16
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int128_t
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapInt16_Type)
#define LOCAL_CObject          CBSwapInt16
#define LOCAL_CTYPES_T         int16_t
#define LOCAL_CTYPES_typename  "bswap_int16_t"
#define LOCAL_CTYPES_sizeof_T  2
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int16_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapInt32_Type)
#define LOCAL_CObject          CBSwapInt32
#define LOCAL_CTYPES_T         int32_t
#define LOCAL_CTYPES_typename  "bswap_int32_t"
#define LOCAL_CTYPES_sizeof_T  4
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int32_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapInt64_Type)
#define LOCAL_CObject          CBSwapInt64
#define LOCAL_CTYPES_T         int64_t
#define LOCAL_CTYPES_typename  "bswap_int64_t"
#define LOCAL_CTYPES_sizeof_T  8
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int64_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapInt128_Type)
#define LOCAL_CObject          CBSwapInt128
#define LOCAL_CTYPES_T         Dee_int128_t
#define LOCAL_CTYPES_typename  "bswap_int128_t"
#define LOCAL_CTYPES_sizeof_T  16
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int128_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapUInt16_Type)
#define LOCAL_CObject          CBSwapUInt16
#define LOCAL_CTYPES_T         uint16_t
#define LOCAL_CTYPES_typename  "bswap_uint16_t"
#define LOCAL_CTYPES_sizeof_T  2
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int16_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapUInt32_Type)
#define LOCAL_CObject          CBSwapUInt32
#define LOCAL_CTYPES_T         uint32_t
#define LOCAL_CTYPES_typename  "bswap_uint32_t"
#define LOCAL_CTYPES_sizeof_T  4
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int32_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapUInt64_Type)
#define LOCAL_CObject          CBSwapUInt64
#define LOCAL_CTYPES_T         uint64_t
#define LOCAL_CTYPES_typename  "bswap_uint64_t"
#define LOCAL_CTYPES_sizeof_T  8
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int64_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CBSwapUInt128_Type)
#define LOCAL_CObject          CBSwapUInt128
#define LOCAL_CTYPES_T         Dee_uint128_t
#define LOCAL_CTYPES_typename  "bswap_uint128_t"
#define LOCAL_CTYPES_sizeof_T  16
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int128_t
#define LOCAL_IS_BSWAP
#define LOCAL_IS_UNSIGNED
#define LOCAL_IS_INTEGRAL
#elif defined(DEFINE_CFloat_Type)
#define LOCAL_CObject          CFloat
#define LOCAL_CTYPES_T         CTYPES_float
#define LOCAL_CTYPES_typename  "float"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_float
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_float
#define LOCAL_IS_FLOAT
#elif defined(DEFINE_CDouble_Type)
#define LOCAL_CObject          CDouble
#define LOCAL_CTYPES_T         CTYPES_double
#define LOCAL_CTYPES_typename  "double"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_double
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_double
#define LOCAL_IS_FLOAT
#elif defined(DEFINE_CLDouble_Type)
#define LOCAL_CObject          CLDouble
#define LOCAL_CTYPES_T         CTYPES_ldouble
#define LOCAL_CTYPES_typename  "ldouble"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_ldouble
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_ldouble
#define LOCAL_IS_FLOAT
#elif defined(DEFINE_CSChar_Type)
#define LOCAL_CObject          CSChar
#define LOCAL_CTYPES_T         CTYPES_schar
#define LOCAL_CTYPES_typename  "schar"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_schar
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_schar
#define LOCAL_MEMBER_unsigned  CUChar_Type
#elif defined(DEFINE_CUChar_Type)
#define LOCAL_CObject          CUChar
#define LOCAL_CTYPES_T         CTYPES_uchar
#define LOCAL_CTYPES_typename  "uchar"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_char
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_char
#define LOCAL_IS_UNSIGNED
#define LOCAL_MEMBER_signed    CSChar_Type
#elif defined(DEFINE_CShort_Type)
#define LOCAL_CObject          CShort
#define LOCAL_CTYPES_T         CTYPES_short
#define LOCAL_CTYPES_typename  "short"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_short
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_short
#define LOCAL_MEMBER_unsigned  CUShort_Type
#elif defined(DEFINE_CUShort_Type)
#define LOCAL_CObject          CUShort
#define LOCAL_CTYPES_T         CTYPES_ushort
#define LOCAL_CTYPES_typename  "ushort"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_short
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_short
#define LOCAL_IS_UNSIGNED
#define LOCAL_MEMBER_signed    CShort_Type
#elif defined(DEFINE_CInt_Type)
#define LOCAL_CObject          CInt
#define LOCAL_CTYPES_T         CTYPES_int
#define LOCAL_CTYPES_typename  "int"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_int
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int
#define LOCAL_MEMBER_unsigned  CUInt_Type
#elif defined(DEFINE_CUInt_Type)
#define LOCAL_CObject          CUInt
#define LOCAL_CTYPES_T         CTYPES_uint
#define LOCAL_CTYPES_typename  "uint"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_int
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_int
#define LOCAL_IS_UNSIGNED
#define LOCAL_MEMBER_signed    CInt_Type
#elif defined(DEFINE_CLong_Type)
#define LOCAL_CObject          CLong
#define LOCAL_CTYPES_T         CTYPES_long
#define LOCAL_CTYPES_typename  "long"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_long
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_long
#define LOCAL_MEMBER_unsigned  CULong_Type
#elif defined(DEFINE_CULong_Type)
#define LOCAL_CObject          CULong
#define LOCAL_CTYPES_T         CTYPES_ulong
#define LOCAL_CTYPES_typename  "ulong"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_long
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_long
#define LOCAL_IS_UNSIGNED
#define LOCAL_MEMBER_signed    CLong_Type
#elif defined(DEFINE_CLLong_Type)
#define LOCAL_CObject          CLLong
#define LOCAL_CTYPES_T         CTYPES_llong
#define LOCAL_CTYPES_typename  "llong"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_llong
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_llong
#define LOCAL_MEMBER_unsigned  CULLong_Type
#elif defined(DEFINE_CULLong_Type)
#define LOCAL_CObject          CULLong
#define LOCAL_CTYPES_T         CTYPES_ullong
#define LOCAL_CTYPES_typename  "ullong"
#define LOCAL_CTYPES_sizeof_T  CTYPES_sizeof_llong
#define LOCAL_CTYPES_alignof_T CTYPES_alignof_llong
#define LOCAL_IS_UNSIGNED
#define LOCAL_MEMBER_signed    CLLong_Type
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifndef LOCAL_CObject_Type
#define LOCAL_CObject_Type PP_CAT2(LOCAL_CObject, _Type)
#endif /* !LOCAL_CObject_Type */
#ifndef LOCAL_CObject_New
#define LOCAL_CObject_New PP_CAT2(LOCAL_CObject, _New)
#endif /* !LOCAL_CObject_New */
#ifndef LOCAL_cobject_class_members
#define LOCAL_cobject_class_members PP_CAT2(LOCAL_CObject, _class_members)
#endif /* !LOCAL_cobject_class_members */

#ifndef LOCAL_c_operators_PTR
#if LOCAL_CTYPES_sizeof_T == 0
#define LOCAL_c_operators_PTR &cvoid_operators
#elif defined(LOCAL_IS_FLOAT)
#if defined(DEFINE_CFloat_Type)
#define LOCAL_c_operators_PTR &cfloat_operators
#elif defined(DEFINE_CDouble_Type)
#define LOCAL_c_operators_PTR &cdouble_operators
#elif defined(DEFINE_CLDouble_Type)
#define LOCAL_c_operators_PTR &cldouble_operators
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#elif defined(LOCAL_IS_UNSIGNED) && defined(LOCAL_IS_BSWAP)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_c_operators_PTR &cuint8_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_c_operators_PTR &cuint16_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_c_operators_PTR &cuint32_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_c_operators_PTR &cuint64_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_c_operators_PTR &cuint128_bswap_operators
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#elif defined(LOCAL_IS_UNSIGNED)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_c_operators_PTR &cuint8_operators
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_c_operators_PTR &cuint16_operators
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_c_operators_PTR &cuint32_operators
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_c_operators_PTR &cuint64_operators
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_c_operators_PTR &cuint128_operators
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#elif defined(LOCAL_IS_BSWAP)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_c_operators_PTR &cint8_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_c_operators_PTR &cint16_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_c_operators_PTR &cint32_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_c_operators_PTR &cint64_bswap_operators
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_c_operators_PTR &cint128_bswap_operators
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#elif LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_c_operators_PTR &cint8_operators
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_c_operators_PTR &cint16_operators
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_c_operators_PTR &cint32_operators
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_c_operators_PTR &cint64_operators
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_c_operators_PTR &cint128_operators
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#endif /* !LOCAL_c_operators_PTR */









#ifndef LOCAL_ffi_type_PTR
#if LOCAL_CTYPES_sizeof_T == 0
#define LOCAL_ffi_type_PTR &ffi_type_void
#elif defined(LOCAL_IS_FLOAT)
#if defined(DEFINE_CFloat_Type)
#define LOCAL_ffi_type_PTR &ffi_type_float
#elif defined(DEFINE_CDouble_Type)
#define LOCAL_ffi_type_PTR &ffi_type_double
#elif defined(DEFINE_CLDouble_Type)
#define LOCAL_ffi_type_PTR &ffi_type_longdouble
#endif /* ... */
#elif defined(LOCAL_IS_UNSIGNED)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_ffi_type_PTR &ffi_type_uint8
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_ffi_type_PTR &ffi_type_uint16
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_ffi_type_PTR &ffi_type_uint32
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_ffi_type_PTR &ffi_type_uint64
#endif /* ... */
#elif LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_ffi_type_PTR &ffi_type_sint8
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_ffi_type_PTR &ffi_type_sint16
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_ffi_type_PTR &ffi_type_sint32
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_ffi_type_PTR &ffi_type_sint64
#endif /* ... */
#ifndef LOCAL_ffi_type_PTR
#define LOCAL_ffi_type_PTR NULL
#endif /* !LOCAL_ffi_type_PTR */
#endif /* !LOCAL_ffi_type_PTR */


#ifndef LOCAL_CPointer_Type_PTR
#define LOCAL_CPointer_Type_PTR NULL
#endif /* !LOCAL_CPointer_Type_PTR */
#ifndef LOCAL_CLValue_Type_PTR
#define LOCAL_CLValue_Type_PTR NULL
#endif /* !LOCAL_CLValue_Type_PTR */


#ifdef LOCAL_IS_INTEGRAL
#if !defined(LOCAL_MEMBER_signed) && (!defined(LOCAL_MEMBER_signed) && !defined(LOCAL_MEMBER_unsigned))
#if defined(LOCAL_IS_BSWAP) && defined(LOCAL_IS_UNSIGNED)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_signed CBSwapInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_signed CBSwapInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_signed CBSwapInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_signed CBSwapInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_signed CBSwapInt128_Type
#endif /* ... */
#elif defined(LOCAL_IS_UNSIGNED)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_signed CInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_signed CInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_signed CInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_signed CInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_signed CInt128_Type
#endif /* ... */
#elif defined(LOCAL_IS_BSWAP)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_unsigned CBSwapUInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_unsigned CBSwapUInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_unsigned CBSwapUInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_unsigned CBSwapUInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_unsigned CBSwapUInt128_Type
#endif /* ... */
#elif LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_unsigned CUInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_unsigned CUInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_unsigned CUInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_unsigned CUInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_unsigned CUInt128_Type
#endif /* ... */
#endif /* !LOCAL_MEMBER_signed && (!LOCAL_MEMBER_signed && !LOCAL_MEMBER_unsigned) */
#endif /* LOCAL_IS_INTEGRAL */

#ifndef LOCAL_MEMBER_bswap
#if defined(LOCAL_IS_BSWAP) && defined(LOCAL_IS_UNSIGNED)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_bswap CUInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_bswap CUInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_bswap CUInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_bswap CUInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_bswap CUInt128_Type
#endif /* ... */
#elif defined(LOCAL_IS_UNSIGNED)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_bswap CBSwapUInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_bswap CBSwapUInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_bswap CBSwapUInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_bswap CBSwapUInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_bswap CBSwapUInt128_Type
#endif /* ... */
#elif defined(LOCAL_IS_BSWAP)
#if LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_bswap CInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_bswap CInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_bswap CInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_bswap CInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_bswap CInt128_Type
#endif /* ... */
#elif LOCAL_CTYPES_sizeof_T == 1
#define LOCAL_MEMBER_bswap CBSwapInt8_Type
#elif LOCAL_CTYPES_sizeof_T == 2
#define LOCAL_MEMBER_bswap CBSwapInt16_Type
#elif LOCAL_CTYPES_sizeof_T == 4
#define LOCAL_MEMBER_bswap CBSwapInt32_Type
#elif LOCAL_CTYPES_sizeof_T == 8
#define LOCAL_MEMBER_bswap CBSwapInt64_Type
#elif LOCAL_CTYPES_sizeof_T == 16
#define LOCAL_MEMBER_bswap CBSwapInt128_Type
#endif /* ... */
#endif /* !LOCAL_MEMBER_bswap */

DECL_BEGIN

#if !defined(LOCAL_IS_FLOAT) && LOCAL_CTYPES_sizeof_T != 0
#define LOCAL_MEMBER_minval PP_CAT2(LOCAL_CObject, _minval)
PRIVATE LOCAL_CObject LOCAL_MEMBER_minval = {
	OBJECT_HEAD_INIT(&LOCAL_CObject_Type),
#ifdef LOCAL_IS_UNSIGNED
#if LOCAL_CTYPES_sizeof_T == 16
	__HYBRID_UINT128_INIT8N(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#else /* LOCAL_CTYPES_sizeof_T == 16 */
	(LOCAL_CTYPES_T)0
#endif /* LOCAL_CTYPES_sizeof_T != 16 */
#else /* LOCAL_IS_UNSIGNED */
#if LOCAL_CTYPES_sizeof_T == 16
	__HYBRID_INT128_INIT8N(0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#else /* LOCAL_CTYPES_sizeof_T == 16 */
	(LOCAL_CTYPES_T)__PRIVATE_MIN_S(LOCAL_CTYPES_sizeof_T)
#endif /* LOCAL_CTYPES_sizeof_T != 16 */
#endif /* !LOCAL_IS_UNSIGNED */
};

#define LOCAL_MEMBER_maxval PP_CAT2(LOCAL_CObject, _maxval)
PRIVATE LOCAL_CObject LOCAL_MEMBER_maxval = {
	OBJECT_HEAD_INIT(&LOCAL_CObject_Type),
#ifdef LOCAL_IS_UNSIGNED
#if LOCAL_CTYPES_sizeof_T == 16
	__HYBRID_UINT128_INIT8N(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff)
#else /* LOCAL_CTYPES_sizeof_T == 16 */
	(LOCAL_CTYPES_T)__PRIVATE_MAX_U(LOCAL_CTYPES_sizeof_T)
#endif /* LOCAL_CTYPES_sizeof_T != 16 */
#else /* LOCAL_IS_UNSIGNED */
#if LOCAL_CTYPES_sizeof_T == 16
	__HYBRID_INT128_INIT8N(0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff)
#else /* LOCAL_CTYPES_sizeof_T == 16 */
	(LOCAL_CTYPES_T)__PRIVATE_MAX_S(LOCAL_CTYPES_sizeof_T)
#endif /* LOCAL_CTYPES_sizeof_T != 16 */
#endif /* !LOCAL_IS_UNSIGNED */
};
#endif /* !LOCAL_IS_FLOAT && LOCAL_CTYPES_sizeof_T != 0 */

PRIVATE struct type_member tpconst LOCAL_cobject_class_members[] = {
#ifdef LOCAL_MEMBER_minval
	TYPE_MEMBER_CONST("MIN", &LOCAL_MEMBER_minval),
#endif /* LOCAL_MEMBER_minval */
#ifdef LOCAL_MEMBER_maxval
	TYPE_MEMBER_CONST("MAX", &LOCAL_MEMBER_maxval),
#endif /* LOCAL_MEMBER_maxval */
#ifdef LOCAL_MEMBER_signed
	TYPE_MEMBER_CONST("signed", CType_AsType(&LOCAL_MEMBER_signed)),
#endif /* LOCAL_MEMBER_signed */
#ifdef LOCAL_MEMBER_unsigned
	TYPE_MEMBER_CONST("unsigned", CType_AsType(&LOCAL_MEMBER_unsigned)),
#endif /* LOCAL_MEMBER_unsigned */
#ifdef LOCAL_MEMBER_bswap
	TYPE_MEMBER_CONST("bswap", CType_AsType(&LOCAL_MEMBER_bswap)),
#endif /* LOCAL_MEMBER_bswap */
#ifdef LOCAL_IS_FLOAT
	TYPE_MEMBER_CONST("isfloat", Dee_True),
#else /* LOCAL_IS_FLOAT */
	TYPE_MEMBER_CONST("isfloat", Dee_False),
#endif /* !LOCAL_IS_FLOAT */

#ifdef LOCAL_MEMBER_minval
	TYPE_MEMBER_CONST("min", &LOCAL_MEMBER_minval), /* Deprecated alias (try to remove after "CONFIG_EXPERIMENTAL_REWORKED_CTYPES") */
#endif /* LOCAL_MEMBER_minval */
#ifdef LOCAL_MEMBER_maxval
	TYPE_MEMBER_CONST("max", &LOCAL_MEMBER_maxval), /* Deprecated alias (try to remove after "CONFIG_EXPERIMENTAL_REWORKED_CTYPES") */
#endif /* LOCAL_MEMBER_maxval */
	TYPE_MEMBER_END
};

#undef LOCAL_MEMBER_minval
#undef LOCAL_MEMBER_maxval
#undef LOCAL_MEMBER_signed
#undef LOCAL_MEMBER_unsigned
#undef LOCAL_MEMBER_bswap

#undef LOCAL_cobject_mro
#ifndef DEFINE_CVoid_Type
#define LOCAL_cobject_mro generic_numeric_ctype_mro
#ifndef generic_numeric_ctype_mro_DEFINED
#define generic_numeric_ctype_mro_DEFINED
PRIVATE DeeTypeObject *tpconst generic_numeric_ctype_mro[] = {
	CType_AsType(&AbstractCObject_Type),
	&DeeObject_Type,
	&DeeNumeric_Type,
	NULL,
};
#endif /* !generic_numeric_ctype_mro_DEFINED */
#endif /* !DEFINE_CVoid_Type */

#ifndef LOCAL_cobject_mro
#define LOCAL_cobject_mro NULL
#endif /* !LOCAL_cobject_mro */

INTERN CType LOCAL_CObject_Type = {
	/* .ct_base = */ {
		OBJECT_HEAD_INIT(&CType_Type),
		/* .tp_name     = */ LOCAL_CTYPES_typename,
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FINHERITCTOR | TP_FMOVEANY,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ CType_AsType(&AbstractCObject_Type),
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ LOCAL_CObject,
				/* tp_ctor:        */ &cobject_ctor,
				/* tp_copy_ctor:   */ &cobject_copy,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &cobject_init_kw,
				/* tp_serialize:   */ NULL /* XXX */
			),
			/* .tp_dtor        = */ NULL,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
		},
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ &cobject_math,
		/* .tp_cmp           = */ &cobject_cmp,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ cobject_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ LOCAL_cobject_class_members,
		/* .tp_method_hints  = */ NULL,
		/* .tp_call          = */ NULL,
		/* .tp_callable      = */ NULL,
		/* .tp_mro           = */ LOCAL_cobject_mro
	},
	CTYPE_INIT_COMMON_EX(
		/* ct_sizeof:    */ LOCAL_CTYPES_sizeof_T,
		/* ct_alignof:   */ LOCAL_CTYPES_alignof_T,
		/* ct_operators: */ LOCAL_c_operators_PTR,
		/* ct_ffitype:   */ LOCAL_ffi_type_PTR,
		/* ct_pointer:   */ LOCAL_CPointer_Type_PTR,
		/* ct_lvalue:    */ LOCAL_CLValue_Type_PTR
	)
};

#undef LOCAL_cobject_mro


#ifndef DEFINE_CVoid_Type
INTERN WUNUSED DREF CObject *DCALL
LOCAL_CObject_New(LOCAL_CTYPES_T val) {
	LOCAL_CObject *result;
	result = DeeObject_MALLOC(LOCAL_CObject);
	if unlikely(!result)
		goto err;
	result->c_value = val;
	CObject_InitStatic(result, &LOCAL_CObject_Type);
	return (DREF CObject *)result;
err:
	return NULL;
}
#endif /* !DEFINE_CVoid_Type */


DECL_END

#undef LOCAL_CObject_Type
#undef LOCAL_CObject_New
#undef LOCAL_cobject_class_members
#undef LOCAL_c_operators_PTR
#undef LOCAL_ffi_type_PTR
#undef LOCAL_CPointer_Type_PTR
#undef LOCAL_CLValue_Type_PTR


#undef LOCAL_CObject
#undef LOCAL_CTYPES_T
#undef LOCAL_CTYPES_typename
#undef LOCAL_CTYPES_sizeof_T
#undef LOCAL_CTYPES_alignof_T
#undef LOCAL_IS_BSWAP
#undef LOCAL_IS_UNSIGNED
#undef LOCAL_IS_INTEGRAL
#undef LOCAL_IS_FLOAT


#undef DEFINE_CVoid_Type
#undef DEFINE_CChar_Type
#undef DEFINE_CWChar_Type
#undef DEFINE_CChar16_Type
#undef DEFINE_CChar32_Type
#undef DEFINE_CBool_Type
#undef DEFINE_CInt8_Type
#undef DEFINE_CInt16_Type
#undef DEFINE_CInt32_Type
#undef DEFINE_CInt64_Type
#undef DEFINE_CInt128_Type
#undef DEFINE_CUInt8_Type
#undef DEFINE_CUInt16_Type
#undef DEFINE_CUInt32_Type
#undef DEFINE_CUInt64_Type
#undef DEFINE_CUInt128_Type
#undef DEFINE_CBSwapInt16_Type
#undef DEFINE_CBSwapInt32_Type
#undef DEFINE_CBSwapInt64_Type
#undef DEFINE_CBSwapInt128_Type
#undef DEFINE_CBSwapUInt16_Type
#undef DEFINE_CBSwapUInt32_Type
#undef DEFINE_CBSwapUInt64_Type
#undef DEFINE_CBSwapUInt128_Type
#undef DEFINE_CFloat_Type
#undef DEFINE_CDouble_Type
#undef DEFINE_CLDouble_Type
#undef DEFINE_CSChar_Type
#undef DEFINE_CUChar_Type
#undef DEFINE_CShort_Type
#undef DEFINE_CUShort_Type
#undef DEFINE_CInt_Type
#undef DEFINE_CUInt_Type
#undef DEFINE_CLong_Type
#undef DEFINE_CULong_Type
#undef DEFINE_CLLong_Type
#undef DEFINE_CULLong_Type
