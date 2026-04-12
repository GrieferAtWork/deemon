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
#ifndef GUARD_DEX_CTYPES_LIBCTYPES_H
#define GUARD_DEX_CTYPES_LIBCTYPES_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>     /* DeeObject_* */
#include <deemon/object.h>    /* DREF, DeeObject, DeeObject_AsDouble, DeeTypeObject, DeeType_Extends, Dee_AsObject, Dee_OBJECT_HEAD_EX, Dee_REQUIRES_OBJECT, Dee_TYPE, Dee_formatprinter_t, Dee_funptr_t, Dee_hash_t, Dee_int128_t, Dee_ssize_t, Dee_uint128_t */
#include <deemon/string.h>    /* Dee_wchar_t */
#include <deemon/type.h>      /* DeeObject_*, DeeType_AllocInstance, DeeType_FreeInstance */
#include <deemon/util/lock.h> /* Dee_ATOMIC_RWLOCK_INIT, Dee_atomic_rwlock_* */

#include <hybrid/byteorder.h>     /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */
#include <hybrid/sequence/list.h> /* LIST_ENTRY */
#include <hybrid/typecore.h>      /* __*_TYPE__, __ALIGNOF_*__, __CHAR_UNSIGNED__, __HYBRID_ALIGNOF, __SIZEOF_*__, __WCHAR_UNSIGNED__ */
#include <hybrid/unaligned.h>     /* UNALIGNED_GET*, UNALIGNED_SET* */

#include <stddef.h> /* NULL, ptrdiff_t, size_t */
#include <stdint.h> /* intN_t, intptr_t, uintN_t, uintptr_t */

#ifndef CONFIG_NO_CFUNCTION
#include <ffi.h>
#include <ffitarget.h>
#endif /* !CONFIG_NO_CFUNCTION */

#undef CONFIG_HAVE_CTYPES_DEPRECATED_ALIASES
#if 1
#define CONFIG_HAVE_CTYPES_DEPRECATED_ALIASES
#endif

DECL_BEGIN

#undef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef _MSC_VER
#define CONFIG_HAVE_CTYPES_FAULTPROTECT
#define CTYPES_FAULTPROTECT(expr, error)                                                 \
	do {                                                                                 \
		__try {                                                                          \
			expr;                                                                        \
		} __except (ctypes_seh_guard((struct _EXCEPTION_POINTERS *)_exception_info())) { \
			error;                                                                       \
		}                                                                                \
	}	__WHILE0

#define CONFIG_HAVE_CTYPES_SEH_GUARD
struct _EXCEPTION_POINTERS;
void *__cdecl _exception_info(void);
INTDEF int ctypes_seh_guard(struct _EXCEPTION_POINTERS *info);

#elif defined(__KOS__) && (__KOS_VERSION__ >= 400)
DECL_END
#include <kos/except.h>
DECL_BEGIN

/* XXX: Mark libctypes as dlexceptaware()? */

#if defined(TRY) && defined(EXCEPT)
#ifdef E_SEGFAULT
INTDEF void ctypes_kos_guard(void);

#define CONFIG_HAVE_CTYPES_KOS_GUARD
#define CONFIG_HAVE_CTYPES_FAULTPROTECT
#define CTYPES_FAULTPROTECT(expr, error) \
	do {                                 \
		TRY {                            \
			expr;                        \
		} EXCEPT {                       \
			ctypes_kos_guard();          \
			error;                       \
		}                                \
	}	__WHILE0
#endif /* E_SEGFAULT */
#endif /* TRY && EXCEPT */

#else /* ... */
/* TODO: handle #PF (PAGEFAULT) interrupts within `expr'
 * Do this by defining a handler for SIGSEGV, as well as
 * a (thread-local) jmp_buf to do SJLJ exceptions. */
#endif /* !... */


/* Fallback: without fault protection, we can still execute code directly */
#ifndef CTYPES_FAULTPROTECT
#define CTYPES_FAULTPROTECT(expr, error) expr
#endif /* !CTYPES_FAULTPROTECT */



#ifdef __SIZEOF_BOOL__
#define CTYPES_sizeof_bool __SIZEOF_BOOL__
#else /* __SIZEOF_BOOL__ */
#define CTYPES_sizeof_bool 1
#endif /* !__SIZEOF_BOOL__ */

#ifdef __SIZEOF_WCHAR_T__
#define CTYPES_sizeof_wchar_t __SIZEOF_WCHAR_T__
#elif defined(CONFIG_HOST_WINDOWS)
#define CTYPES_sizeof_wchar_t 2
#else /* ... */
#define CTYPES_sizeof_wchar_t 4
#endif /* !... */

#define CTYPES_sizeof_char16_t 2
#define CTYPES_sizeof_char32_t 4

#ifdef __SIZEOF_CHAR__
#define CTYPES_sizeof_char __SIZEOF_CHAR__
#else /* __SIZEOF_CHAR__ */
#define CTYPES_sizeof_char 1
#endif /* !__SIZEOF_CHAR__ */

#ifdef __SIZEOF_SHORT__
#define CTYPES_sizeof_short __SIZEOF_SHORT__
#else /* __SIZEOF_SHORT__ */
#define CTYPES_sizeof_short 2
#endif /* !__SIZEOF_SHORT__ */

#ifdef __SIZEOF_INT__
#define CTYPES_sizeof_int __SIZEOF_INT__
#else /* __SIZEOF_INT__ */
#define CTYPES_sizeof_int 4
#endif /* !__SIZEOF_INT__ */

#ifdef __SIZEOF_LONG__
#define CTYPES_sizeof_long __SIZEOF_LONG__
#elif defined(CONFIG_HOST_WINDOWS)
#define CTYPES_sizeof_long 4
#else /* ... */
#define CTYPES_sizeof_long __SIZEOF_POINTER__
#endif /* !... */

#ifdef __SIZEOF_LONG_LONG__
#define CTYPES_sizeof_llong __SIZEOF_LONG_LONG__
#elif defined(__SIZEOF_LLONG__)
#define CTYPES_sizeof_llong __SIZEOF_LLONG__
#else /* ... */
#define CTYPES_sizeof_llong 8
#endif /* !... */

#undef CTYPES_char_UNSIGNED
#ifdef __CHAR_UNSIGNED__
#define CTYPES_char_UNSIGNED
#endif /* __CHAR_UNSIGNED__ */

#undef CTYPES_wchar_t_UNSIGNED
#ifdef __WCHAR_UNSIGNED__
#define CTYPES_wchar_t_UNSIGNED
#endif /* __WCHAR_UNSIGNED__ */

#define CTYPES_char16_t_UNSIGNED
#define CTYPES_char32_t_UNSIGNED

#define CTYPES_sizeof_pointer  __SIZEOF_POINTER__
#define CTYPES_alignof_pointer __ALIGNOF_POINTER__
#define CTYPES_sizeof_lvalue   CTYPES_sizeof_pointer
#define CTYPES_alignof_lvalue  CTYPES_alignof_pointer

#define CTYPES_alignof_bool     __HYBRID_ALIGNOF(CTYPES_sizeof_bool)
#define CTYPES_alignof_wchar_t  __HYBRID_ALIGNOF(CTYPES_sizeof_wchar_t)
#define CTYPES_alignof_char16_t __HYBRID_ALIGNOF(CTYPES_sizeof_char16_t)
#define CTYPES_alignof_char32_t __HYBRID_ALIGNOF(CTYPES_sizeof_char32_t)
#define CTYPES_alignof_char     __HYBRID_ALIGNOF(CTYPES_sizeof_char)
#define CTYPES_alignof_short    __HYBRID_ALIGNOF(CTYPES_sizeof_short)
#define CTYPES_alignof_int      __HYBRID_ALIGNOF(CTYPES_sizeof_int)
#define CTYPES_alignof_long     __HYBRID_ALIGNOF(CTYPES_sizeof_long)
#define CTYPES_alignof_llong    __HYBRID_ALIGNOF(CTYPES_sizeof_llong)
#define CTYPES_alignof_int8_t   __ALIGNOF_INT8__
#define CTYPES_alignof_int16_t  __ALIGNOF_INT16__
#define CTYPES_alignof_int32_t  __ALIGNOF_INT32__
#define CTYPES_alignof_int64_t  __ALIGNOF_INT64__
#ifdef __ALIGNOF_INT128__
#define CTYPES_alignof_int128_t __ALIGNOF_INT128__
#else /* __ALIGNOF_INT128__ */
#define CTYPES_alignof_int128_t __ALIGNOF_INT64__
#endif /* !__ALIGNOF_INT128__ */


#define CTYPES_float float
#ifdef __SIZEOF_FLOAT__
#define CTYPES_sizeof_float __SIZEOF_FLOAT__
#else /* __SIZEOF_FLOAT__ */
#define CTYPES_sizeof_float sizeof(CTYPES_float)
#endif /* !__SIZEOF_FLOAT__ */
#ifdef __ALIGNOF_FLOAT__
#define CTYPES_alignof_float __ALIGNOF_FLOAT__
#else /* __ALIGNOF_FLOAT__ */
#define CTYPES_alignof_float COMPILER_ALIGNOF(CTYPES_float)
#endif /* !__ALIGNOF_FLOAT__ */

#define CTYPES_double double
#ifdef __SIZEOF_DOUBLE__
#define CTYPES_sizeof_double __SIZEOF_DOUBLE__
#else /* __SIZEOF_DOUBLE__ */
#define CTYPES_sizeof_double sizeof(CTYPES_double)
#endif /* !__SIZEOF_DOUBLE__ */
#ifdef __ALIGNOF_DOUBLE__
#define CTYPES_alignof_double __ALIGNOF_DOUBLE__
#else /* __ALIGNOF_DOUBLE__ */
#define CTYPES_alignof_double COMPILER_ALIGNOF(CTYPES_double)
#endif /* !__ALIGNOF_DOUBLE__ */

#if (defined(FFI_TYPE_LONGDOUBLE) && \
     defined(FFI_TYPE_DOUBLE) &&     \
     FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE)
#define CTYPES_ldouble         double
#define CTYPES_sizeof_ldouble  CTYPES_sizeof_double
#define CTYPES_alignof_ldouble CTYPES_alignof_double
#else /* FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE */
#ifdef __LONGDOUBLE
#define CTYPES_ldouble __LONGDOUBLE
#else /* __LONGDOUBLE */
#define CTYPES_ldouble long double
#endif /* !__LONGDOUBLE */
#ifdef __SIZEOF_LONG_DOUBLE__
#define CTYPES_sizeof_ldouble __SIZEOF_LONG_DOUBLE__
#else /* __SIZEOF_LONG_DOUBLE__ */
#define CTYPES_sizeof_ldouble sizeof(CTYPES_ldouble)
#endif /* !__SIZEOF_LONG_DOUBLE__ */
#ifdef __ALIGNOF_LONG_DOUBLE__
#define CTYPES_alignof_ldouble __ALIGNOF_LONG_DOUBLE__
#else /* __ALIGNOF_LONG_DOUBLE__ */
#define CTYPES_alignof_ldouble COMPILER_ALIGNOF(CTYPES_ldouble)
#endif /* !__ALIGNOF_LONG_DOUBLE__ */
#endif /* FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE */



#define PRIVATE_HOST_INTFOR_1   __INT8_TYPE__
#define PRIVATE_HOST_INTFOR_2   __INT16_TYPE__
#define PRIVATE_HOST_INTFOR_4   __INT32_TYPE__
#define PRIVATE_HOST_INTFOR_8   __INT64_TYPE__
#define PRIVATE_HOST_INTFOR_16  Dee_int128_t
#define PRIVATE_HOST_UINTFOR_1  __UINT8_TYPE__
#define PRIVATE_HOST_UINTFOR_2  __UINT16_TYPE__
#define PRIVATE_HOST_UINTFOR_4  __UINT32_TYPE__
#define PRIVATE_HOST_UINTFOR_8  __UINT64_TYPE__
#define PRIVATE_HOST_UINTFOR_16 Dee_uint128_t
#define HOST_INTFOR(sizeof)  PP_PRIVATE_CAT2(PRIVATE_HOST_INTFOR_, sizeof)
#define HOST_UINTFOR(sizeof) PP_PRIVATE_CAT2(PRIVATE_HOST_UINTFOR_, sizeof)

#define CTYPES_int      HOST_INTFOR(CTYPES_sizeof_int)
#define CTYPES_uint     HOST_UINTFOR(CTYPES_sizeof_int)
#define CTYPES_bool     HOST_UINTFOR(CTYPES_sizeof_bool)
#ifdef CTYPES_wchar_t_UNSIGNED
#define CTYPES_wchar_t  HOST_UINTFOR(CTYPES_sizeof_wchar_t)
#else /* CTYPES_wchar_t_UNSIGNED */
#define CTYPES_wchar_t  HOST_INTFOR(CTYPES_sizeof_wchar_t)
#endif /* !CTYPES_wchar_t_UNSIGNED */
#ifdef CTYPES_char16_t_UNSIGNED
#define CTYPES_char16_t HOST_UINTFOR(CTYPES_sizeof_char16_t)
#else /* CTYPES_char16_t_UNSIGNED */
#define CTYPES_char16_t HOST_INTFOR(CTYPES_sizeof_char16_t)
#endif /* !CTYPES_char16_t_UNSIGNED */
#ifdef CTYPES_char32_t_UNSIGNED
#define CTYPES_char32_t HOST_UINTFOR(CTYPES_sizeof_char32_t)
#else /* CTYPES_char32_t_UNSIGNED */
#define CTYPES_char32_t HOST_INTFOR(CTYPES_sizeof_char32_t)
#endif /* !CTYPES_char32_t_UNSIGNED */
#ifdef CTYPES_char_UNSIGNED
#define CTYPES_char     HOST_UINTFOR(CTYPES_sizeof_char)
#else /* CTYPES_char_UNSIGNED */
#define CTYPES_char     HOST_INTFOR(CTYPES_sizeof_char)
#endif /* !CTYPES_char_UNSIGNED */
#define CTYPES_schar    HOST_INTFOR(CTYPES_sizeof_char)
#define CTYPES_uchar    HOST_UINTFOR(CTYPES_sizeof_char)
#define CTYPES_short    HOST_INTFOR(CTYPES_sizeof_short)
#define CTYPES_ushort   HOST_UINTFOR(CTYPES_sizeof_short)
#define CTYPES_int      HOST_INTFOR(CTYPES_sizeof_int)
#define CTYPES_uint     HOST_UINTFOR(CTYPES_sizeof_int)
#define CTYPES_long     HOST_INTFOR(CTYPES_sizeof_long)
#define CTYPES_ulong    HOST_UINTFOR(CTYPES_sizeof_long)
#define CTYPES_llong    HOST_INTFOR(CTYPES_sizeof_llong)
#define CTYPES_ullong   HOST_UINTFOR(CTYPES_sizeof_llong)








union pointer {
	uintptr_t             uint;
	intptr_t              sint;
	void                 *ptr;
	uint8_t              *p8;
	uint16_t             *p16;
	uint32_t             *p32;
	uint64_t             *p64;
	int8_t               *ps8;
	int16_t              *ps16;
	int32_t              *ps32;
	int64_t              *ps64;
	void                 *pvoid;
	char                 *pchar;
	Dee_wchar_t          *pwchar;
	signed char          *pschar;
	unsigned char        *puchar;
	short                *pshort;
	unsigned short       *pushort;
	int                  *pint;
	unsigned int         *puint;
	long                 *plong;
	unsigned long        *pulong;
	void const           *pcvoid;
	char const           *pcchar;
	Dee_wchar_t const    *pcwchar;
	signed char const    *pcschar;
	unsigned char const  *pcuchar;
	short const          *pcshort;
	unsigned short const *pcushort;
	int const            *pcint;
	unsigned int const   *pcuint;
	long const           *pclong;
	unsigned long const  *pculong;
	__BYTE_TYPE__        *pbyte;
	void                **pptr;
	Dee_funptr_t          pfunc;
	Dee_funptr_t         *ppfunc;
};

#if __SIZEOF_POINTER__ == 4
#define pointer_get(self)    ((void *)UNALIGNED_GET32(&(self)->uint))
#define pointer_set(self, v) UNALIGNED_SET32(&(self)->uint, (uint32_t)(v))
#elif __SIZEOF_POINTER__ == 8
#define pointer_get(self)    ((void *)UNALIGNED_GET64(&(self)->uint))
#define pointer_set(self, v) UNALIGNED_SET64(&(self)->uint, (uint64_t)(v))
#elif __SIZEOF_POINTER__ == 2
#define pointer_get(self)    ((void *)UNALIGNED_GET16(&(self)->uint))
#define pointer_set(self, v) UNALIGNED_SET16(&(self)->uint, (uint16_t)(v))
#elif __SIZEOF_POINTER__ == 1
#define pointer_get(self)    ((void *)UNALIGNED_GET8(&(self)->uint))
#define pointer_set(self, v) UNALIGNED_SET8(&(self)->uint, (uint8_t)(v))
#else /* __SIZEOF_POINTER__ == ... */
#error "Unsupported '__SIZEOF_POINTER__'"
#endif /* __SIZEOF_POINTER__ != ... */


#ifndef CONFIG_NO_CFUNCTION
typedef ffi_abi ctypes_cc_t;
#define CC_DEFAULT   FFI_DEFAULT_ABI
#define CC_MTYPE     ((ctypes_cc_t)0x7fff) /* MASK: The actual FFI type. */
#define CC_FVARARGS  ((ctypes_cc_t)0x8000) /* FLAG: Variable-length argument list. */
#define CC_INVALID   ((ctypes_cc_t)-1)
#define ctypes_cc_isvarargs(x) ((x) & CC_FVARARGS)

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

#else /* !CONFIG_NO_CFUNCTION */
typedef int ctypes_cc_t;
#define CC_DEFAULT   0
#define CC_INVALID   (-1)
#define CC_FVARARGS  ((ctypes_cc_t)0x8000) /* FLAG: Variable-length argument list. */
#endif /* CONFIG_NO_CFUNCTION */

/* Convert a calling convention to/from its name.
 * @return: CC_INVALID: The given `name' was not recognized.
 * @return: NULL:       The given `cc' was not recognized. */
INTDEF WUNUSED NONNULL((1)) ctypes_cc_t DCALL cc_lookup(char const *__restrict name);
INTDEF WUNUSED NONNULL((1)) ctypes_cc_t DCALL cc_trylookup(char const *__restrict name);
INTDEF WUNUSED char const *DCALL cc_getname(ctypes_cc_t cc);





#undef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
#undef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES_REUSE_CFI
#ifndef CONFIG_NO_CFUNCTION
#define CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES /* TODO: Write unit test for this */
#define CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES_REUSE_CFI
#endif /* !CONFIG_NO_CFUNCTION */


typedef struct ctype_object CType; /* Also doubles as the RValue type */
typedef struct carray_type_object CArrayType;
typedef struct cstruct_type_object CStructType;
typedef struct cfunction_type_object CFunctionType;
typedef struct cpointer_type_object CPointerType;
typedef struct clvalue_type_object CLValueType;

typedef struct cobject_object CObject;
typedef struct carray_object CArray;
typedef struct cstruct_object CStruct;
#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
typedef struct cfunction_object CFunction;
#endif /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
typedef struct cpointer_object CPointer;
typedef struct clvalue_object CLValue;


#undef CTYPES_DEFINE_STATIC_POINTER_TYPES
#if 1
#define CTYPES_DEFINE_STATIC_POINTER_TYPES
#endif

struct ctype_operators {
	/* NOTE: *ALL* operators are [1..1][const]. Unsupported operators must
	 *       still be implemented, with the implementation then throwing
	 *       the appropriate exception. */

	/* Initialize a CObject instance using "src".
	 * For most types, this simply does:
	 * >> memcpy(CObject_Data(self), src, CType_Sizeof(Dee_TYPE(self)));
	 *
	 * However, some types of c-objects (like CPointer and CLValue) have
	 * some additional fields that are unrelated to the actual C-value,
	 * and also need to be initialized, which is then also done by this
	 * function!
	 *
	 * Note for this purpose that no special handling is required in
	 * regards to dealing with faults caused by reading from "src". If
	 * accessing that pointer can fault, it's the caller's job to call
	 * this operator from within "CTYPES_FAULTPROTECT".
	 *
	 * @return: * : Always re-returns "self" (for better register channeling) */
	ATTR_RETNONNULL_T NONNULL_T((1, 2)) CObject *
	(DCALL *co_initobject)(CObject *__restrict self, void const *src);

	/* Initialize "self" by casting- and assigning "value" to it
	 * @return: 0 : Success
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1, 3)) int
	(DCALL *co_initfrom)(CType *tp_self, void *self, DeeObject *value);

	/* Same as "co_initfrom", but used for secondary assignments
	 * (which have different semantics in case of lvalues, where
	 * secondary assignments re-assign the pointed-to value, rather
	 * than the lvalue location itself) */
	WUNUSED_T NONNULL_T((1, 3)) int
	(DCALL *co_assign)(CType *tp_self, void *self, DeeObject *value);

	/* Initialize a new instance of "self"
	 * @return: 0 : Success
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1)) int
	(DCALL *co_initwith)(CType *tp_self, void *self,
	                     size_t argc, DeeObject *const *argv,
	                     DeeObject *kw);

	/* Check if "self" should evaluate to true
	 * @return: 1 : Is true
	 * @return: 0 : Is false
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1)) int
	(DCALL *co_bool)(CType *tp_self, void const *self);

	/* Print the C-representation of "self" (for operator) */
	WUNUSED_T NONNULL_T((1, 3)) Dee_ssize_t
	(DCALL *co_printcrepr)(CType *tp_self, void const *self,
	                       Dee_formatprinter_t printer, void *arg);

	/* Print the deemon-representation of "self" */
	WUNUSED_T NONNULL_T((1, 3)) Dee_ssize_t
	(DCALL *co_printdrepr)(CType *tp_self, void const *self,
	                       Dee_formatprinter_t printer, void *arg);

	/* Compare "lhs" to "rhs" (returning Dee_COMPARE_*)
	 * @return: Dee_COMPARE_LO:  lhs < rhs
	 * @return: Dee_COMPARE_EQ:  lhs == rhs
	 * @return: Dee_COMPARE_GR:  lhs > rhs
	 * @return: Dee_COMPARE_ERR: Error */
	WUNUSED_T NONNULL_T((1, 3)) int
	(DCALL *co_compare)(CType *tp_self, void const *lhs, DeeObject *rhs);

	/* Math operators. */
	/* @return: Dee_INT_SIGNED:   The value stored in `*result' is signed.
	 * @return: Dee_INT_UNSIGNED: The value stored in `*result' is unsigned.
	 * @return: Dee_INT_ERROR:    An error occurred. */
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *co_int32)(CType *tp_self, void const *self, int32_t *result);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *co_int64)(CType *tp_self, void const *self, int64_t *result);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *co_double)(CType *tp_self, void const *self, double *result);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *co_int)(CType *tp_self, void const *self); /* Cast to `int' */
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *co_inv)(CType *tp_self, void const *self);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *co_pos)(CType *tp_self, void const *self);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *co_neg)(CType *tp_self, void const *self);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_add)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_sub)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_mul)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_div)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_mod)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_shl)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_shr)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_and)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_or)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_xor)(CType *tp_self, void const *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *co_pow)(CType *tp_self, void const *self, DeeObject *some_object);

	/* Inplace operators */
	WUNUSED_T NONNULL_T((1))    int (DCALL *co_inc)(CType *tp_self, void *self);
	WUNUSED_T NONNULL_T((1))    int (DCALL *co_dec)(CType *tp_self, void *self);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_add)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_sub)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_mul)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_div)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_mod)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_shl)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_shr)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_and)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_or)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_xor)(CType *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *co_inplace_pow)(CType *tp_self, void *self, DeeObject *some_object);
};

/* Void type */
INTDEF struct ctype_operators Dee_tpconst cvoid_operators;

/* Integer types */
INTDEF struct ctype_operators Dee_tpconst cint8_operators;
INTDEF struct ctype_operators Dee_tpconst cint16_operators;
INTDEF struct ctype_operators Dee_tpconst cint32_operators;
INTDEF struct ctype_operators Dee_tpconst cint64_operators;
INTDEF struct ctype_operators Dee_tpconst cint128_operators;
INTDEF struct ctype_operators Dee_tpconst cuint8_operators;
INTDEF struct ctype_operators Dee_tpconst cuint16_operators;
INTDEF struct ctype_operators Dee_tpconst cuint32_operators;
INTDEF struct ctype_operators Dee_tpconst cuint64_operators;
INTDEF struct ctype_operators Dee_tpconst cuint128_operators;
#define cint8_bswap_operators cint8_operators
INTDEF struct ctype_operators Dee_tpconst cint16_bswap_operators;
INTDEF struct ctype_operators Dee_tpconst cint32_bswap_operators;
INTDEF struct ctype_operators Dee_tpconst cint64_bswap_operators;
INTDEF struct ctype_operators Dee_tpconst cint128_bswap_operators;
#define cuint8_bswap_operators cuint8_operators
INTDEF struct ctype_operators Dee_tpconst cuint16_bswap_operators;
INTDEF struct ctype_operators Dee_tpconst cuint32_bswap_operators;
INTDEF struct ctype_operators Dee_tpconst cuint64_bswap_operators;
INTDEF struct ctype_operators Dee_tpconst cuint128_bswap_operators;

/* Floating point types */
INTDEF struct ctype_operators Dee_tpconst cfloat_operators;
INTDEF struct ctype_operators Dee_tpconst cdouble_operators;
INTDEF struct ctype_operators Dee_tpconst cldouble_operators;

/* Operators for pointer types */
INTDEF struct ctype_operators Dee_tpconst cpointer_operators;

/* "tp_self" is "CLValueType"; forward operator to "tp_self->clt_orig" and "*(void **)self"
 * This is only really needed when user-defined structure types contain lvalue fields:
 * >> local a = (union { .x = int })(x: 42);
 * >> local b = (union { .x = int.ptr, .y = int.lvalue })(y: a.x);
 * >> assert a.x == 42;
 * >> assert b.x == a.x.ptr;
 * >> assert b.y == 42;
 * >> b.y = 99;
 * >> assert a.x == 99; */
INTDEF struct ctype_operators Dee_tpconst clvalue_operators;

/* Operators for user-defined structure types */
INTDEF struct ctype_operators Dee_tpconst cstruct_operators;

/* Operators for array types */
INTDEF struct ctype_operators Dee_tpconst carray_operators;


INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsCFloat)(DeeObject *__restrict self, CTYPES_float *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsCLDouble)(DeeObject *__restrict self, CTYPES_ldouble *__restrict p_result);
#undef DeeObject_AsCDouble
#define DeeObject_AsCDouble DeeObject_AsDouble


LIST_HEAD(carray_type_list, carray_type_object);
struct ctype_array_types {
	size_t                   sa_size; /* Amount of cached array types. */
	Dee_hash_t               sa_mask; /* Allocated map mask. */
	struct carray_type_list *sa_list; /* [0..1][0..sa_mask+1][owned] Hash-map of array types.
	                                   * As hash for indexing this map, use `at_count'. */
};
#define CTYPE_ARRAY_TYPES_INIT { 0, 0, NULL }

#ifndef CONFIG_NO_CFUNCTION
LIST_HEAD(cfunction_type_list, cfunction_type_object);
struct ctype_function_types {
	size_t                      sf_size; /* Amount of cached function types. */
	Dee_hash_t                  sf_mask; /* Allocated map mask. */
	struct cfunction_type_list *sf_list; /* [0..1][0..sf_mask+1][owned] Hash-map of array types.
	                                      * As hash for indexing this map, use `cft_hash'. */
};
#define CTYPE_FUNCTION_TYPES_INIT { 0, 0, NULL }
#endif /* !CONFIG_NO_CFUNCTION */


/************************************************************************/
/* C TYPE                                                               */
/************************************************************************/
struct ctype_object {
	DeeTypeObject                 ct_base;      /* The underlying type object. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t           ct_cachelock; /* Lock for cached derived types. */
#endif /* !CONFIG_NO_THREADS    */
	CPointerType                 *ct_pointer;   /* [0..1][lock(ct_cachelock)] A weak pointer to the pointer-type of this C-type. */
	CLValueType                  *ct_lvalue;    /* [0..1][lock(ct_cachelock)] A weak pointer to the lvalue-type of this C-type. */
	struct ctype_array_types      ct_arrays;    /* [lock(ct_cachelock)] Derived array sub-types. */
#ifndef CONFIG_NO_CFUNCTION
	struct ctype_function_types   ct_functions; /* [lock(ct_cachelock)] Derived function sub-types. */
	ffi_type                     *ct_ffitype;   /* [owned_if(!= &ffi_type_pointer)][0..1][lock(WRITE_ONCE)] The type used by FFI to represent this type. */
#endif /* !CONFIG_NO_CFUNCTION */
	size_t                        ct_sizeof;    /* [const] # of bytes required by instances of this type. */
	size_t                        ct_alignof;   /* [const] Alignment required by this type. */
	struct ctype_operators const *ct_operators; /* [1..1][const] C operators */
};

#ifndef CONFIG_NO_THREADS
#define _CTYPE_INIT_COMMON__ct_cachelock Dee_ATOMIC_RWLOCK_INIT,
#else /* !CONFIG_NO_THREADS    */
#define _CTYPE_INIT_COMMON__ct_cachelock /* nothing */
#endif /* CONFIG_NO_THREADS    */
#ifndef CONFIG_NO_CFUNCTION
#define _CTYPE_INIT_COMMON__ct_functions CTYPE_FUNCTION_TYPES_INIT,
#define _CTYPE_INIT_COMMON__ct_ffitype(ct_ffitype_) ct_ffitype_,
#else /* !CONFIG_NO_CFUNCTION */
#define _CTYPE_INIT_COMMON__ct_functions            /* nothing */
#define _CTYPE_INIT_COMMON__ct_ffitype(ct_ffitype_) /* nothing */
#endif /* CONFIG_NO_CFUNCTION */

#define CTYPE_INIT_COMMON(ct_sizeof_, ct_alignof_, ct_operators_, ct_ffitype_) \
	CTYPE_INIT_COMMON_EX(ct_sizeof_, ct_alignof_, ct_operators_, ct_ffitype_, NULL, NULL)
#define CTYPE_INIT_COMMON_EX(ct_sizeof_, ct_alignof_, ct_operators_,   \
                             ct_ffitype_, ct_pointer_, ct_lvalue_)     \
	_CTYPE_INIT_COMMON__ct_cachelock                                   \
	/* .ct_pointer    = */ ct_pointer_,                                \
	/* .ct_lvalue     = */ ct_lvalue_,                                 \
	/* .ct_arrays     = */ CTYPE_ARRAY_TYPES_INIT,                     \
	/* .ct_functions  = */ _CTYPE_INIT_COMMON__ct_functions            \
	/* .ct_ffitype    = */ _CTYPE_INIT_COMMON__ct_ffitype(ct_ffitype_) \
	/* .ct_sizeof     = */ ct_sizeof_,                                 \
	/* .ct_alignof    = */ ct_alignof_,                                \
	/* .ct_operators  = */ ct_operators_

#define CType_CacheLockReading(self)    Dee_atomic_rwlock_reading(&(self)->ct_cachelock)
#define CType_CacheLockWriting(self)    Dee_atomic_rwlock_writing(&(self)->ct_cachelock)
#define CType_CacheLockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->ct_cachelock)
#define CType_CacheLockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->ct_cachelock)
#define CType_CacheLockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->ct_cachelock)
#define CType_CacheLockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->ct_cachelock)
#define CType_CacheLockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->ct_cachelock)
#define CType_CacheLockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->ct_cachelock)
#define CType_CacheLockRead(self)       Dee_atomic_rwlock_read(&(self)->ct_cachelock)
#define CType_CacheLockWrite(self)      Dee_atomic_rwlock_write(&(self)->ct_cachelock)
#define CType_CacheLockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->ct_cachelock)
#define CType_CacheLockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->ct_cachelock)
#define CType_CacheLockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->ct_cachelock)
#define CType_CacheLockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->ct_cachelock)
#define CType_CacheLockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->ct_cachelock)
#define CType_CacheLockEnd(self)        Dee_atomic_rwlock_end(&(self)->ct_cachelock)

/* Query properties of a given C-Type */
#define CType_Sizeof(self)  (self)->ct_sizeof
#define CType_Alignof(self) (self)->ct_alignof

#define CType_AllocInstance(self)                                     \
	(((self)->ct_base.tp_init.tp_alloc.tp_free)                       \
	 ? (DREF CObject *)(*(self)->ct_base.tp_init.tp_alloc.tp_alloc)() \
	 : (DREF CObject *)DeeObject_Malloc((self)->ct_base.tp_init.tp_alloc.tp_instance_size))
#define CType_FreeInstance(self, p)                   \
	(((self)->ct_base.tp_init.tp_alloc.tp_free)       \
	 ? (*(self)->ct_base.tp_init.tp_alloc.tp_free)(p) \
	 : DeeObject_Free(p))
#define CType_Operators(self) (self)->ct_operators

INTDEF DeeTypeObject CType_Type;   /* == type(ctypes.int) */
INTDEF CType AbstractCObject_Type; /* == Type.__base__(ctypes.int) */

#define CType_AsType(self)   (&(self)->ct_base)
#define Type_AsCType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CType, ct_base)
#define Type_IsCType(self)   DeeType_Extends(Dee_TYPE(self), &CType_Type)
#define Object_AsCType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CType, ct_base)
#define Object_IsCType(self) DeeType_Extends(Dee_TYPE(self), &CType_Type)

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
INTDEF WUNUSED NONNULL((1)) CType *DCALL CType_Of(DeeObject *__restrict self);

/* Same as `DeeSType_Get()', but also able to handle the
 * case where "self" is an *instance*, rather a some type. */
INTDEF WUNUSED NONNULL((1)) CType *DCALL CType_TypeOf(DeeObject *__restrict self);

/* Print the C-representation of "self" for the purposes of a variable "varname"
 * Note that "varname" may be `NULL' or an empty string, in which case the type
 * is printed anonymously. */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CType_PrintCRepr(CType *__restrict self, Dee_formatprinter_t printer, void *arg, char const *varname);

/* Print the deemon representation of "self" (iow: the deemon expression to form "self") */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CType_PrintDRepr(CType *__restrict self, Dee_formatprinter_t printer, void *arg);

#ifndef CONFIG_NO_CFUNCTION
/* Lazily allocated + return the FFI type descriptor of `self' */
INTDEF WUNUSED NONNULL((1)) ffi_type *DCALL
CType_GetFFIType(CType *__restrict self);
#endif /* !CONFIG_NO_CFUNCTION */


struct cobject_object {
	Dee_OBJECT_HEAD_EX(CType)
	COMPILER_FLEXIBLE_ARRAY(__BYTE_TYPE__, co_data); /* Object data... */
};

#define Object_AsCObject(self) Dee_REQUIRES_OBJECT(CObject, self)
#define Object_IsCObject(self) DeeType_Extends(Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)), &CType_Type)

/* Interact with C-Objects */
#define CObject_Alloc(tp)            CType_AllocInstance(tp)
#define CObject_Free(tp, p)          CType_FreeInstance(tp, p)
#define CObject_Init(self, tp)       DeeObject_InitEx(self, tp, CType_AsType) /* Some are static, some are heap-based */
#define CObject_InitStatic(self, tp) DeeObject_InitStaticEx(self, tp, CType_AsType)
#define CObject_InitHeap(self, tp)   DeeObject_InitHeapEx(self, tp, CType_AsType)
#define CObject_Data(self)           Object_AsCObject(self)->co_data






/************************************************************************/
/* ARRAY TYPE                                                           */
/************************************************************************/
struct carray_type_object {
	CType                          cat_base;   /* The underlying C-type descriptor. */
	DREF CType                    *cat_item;   /* [1..1][const] The array's element type. */
	LIST_ENTRY(carray_type_object) cat_chain;  /* [lock(cat_item->st_cachelock)] Hash-map entry of this array. */
	size_t                         cat_count;  /* [const] The total number of items. */
	size_t                         cat_stride; /* [const] Offset from start of one element, to start of next */
};

/* Query properties of a given Array-Type */
#define CArrayType_AllocInstance(self)       ((CStruct *)DeeType_AllocInstance(CArrayType_AsType(self)))
#define CArrayType_FreeInstance(self, p)     DeeType_FreeInstance(CArrayType_AsType(self), p)
#define CArrayType_Count(self)               (self)->cat_count
#define CArrayType_PointedToType(self)       (self)->cat_item
#define CArrayType_Stride(self)              (self)->cat_stride
#define CArrayType_SizeofPointedToType(self) (self)->cat_stride

/* Construct a new array-type `item_type[item_count]' */
INTDEF WUNUSED NONNULL((1)) DREF CArrayType *DCALL
CArrayType_Of(CType *__restrict item_type, size_t item_count);

INTDEF DeeTypeObject CArrayType_Type;  /* == type(ctypes.int[42]) */
INTDEF CArrayType AbstractCArray_Type; /* == Type.__base__(ctypes.int[42]) */

#define CArrayType_AsCType(self)  (&(self)->cat_base)
#define CArrayType_AsType(self)   (&(self)->cat_base.ct_base)
#define CType_AsCArrayType(self)  COMPILER_CONTAINER_OF(self, CArrayType, cat_base)
#define CType_IsCArrayType(self)  (Dee_TYPE(&(self)->ct_base) == &CArrayType_Type)
#define Type_AsCArrayType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CArrayType, cat_base.ct_base)
#define Type_IsCArrayType(self)   (Dee_TYPE(self) == &CArrayType_Type)
#define Object_AsCArrayType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CArrayType, cat_base.ct_base)
#define Object_IsCArrayType(self) (Dee_TYPE(self) == &CArrayType_Type)

struct carray_object {
	Dee_OBJECT_HEAD_EX(CArrayType)
	COMPILER_FLEXIBLE_ARRAY(__BYTE_TYPE__, ca_data); /* Array data... */
};

#define Object_AsCArray(self) Dee_REQUIRES_OBJECT(CArray, self)
#define Object_IsCArray(self) DeeType_Extends(Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)), &CArrayType_Type)

/* Interact with Array objects */
#define CArray_Alloc(tp)         CArrayType_AllocInstance(tp)
#define CArray_Free(tp, p)       CArrayType_FreeInstance(tp, p)
#define CArray_Init(self, tp)    DeeObject_InitHeapEx(self, tp, CArrayType_AsType) /* All array-types are heap-allocated */
#define CArray_ItemAddr(self, i) ((self)->ca_data + ((i) * CArrayType_Stride(Dee_TYPE(self))))
#define CArray_Items(self)       (self)->ca_data

/* Return a pointer to the `index' element (also asserts that "index" is in-bounds) */
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL
CArray_PlusOffset(CArray *__restrict self, ptrdiff_t index);

/* Return an LValue to the `index' element (also asserts that "index" is in-bounds) */
INTDEF WUNUSED NONNULL((1)) DREF CLValue *DCALL
CArray_GetItem(CArray *__restrict self, ptrdiff_t index);

/* Special case: this returns a pointer to "&ca_data[0]" (thus emulating C's array-to-pointer decay) */
#define CArray_AsPointer(self) CArray_PlusOffset(self, 0)






/************************************************************************/
/* STRUCT TYPE                                                          */
/************************************************************************/
struct Dee_string_object;
struct cstruct_field {
	DREF struct Dee_string_object *csf_name;   /* [0..1] The name of this field (NULL is used as sentinel; empty string for anonymous fields) */
	Dee_hash_t                     csf_hash;   /* [valid_if(csf_name)][const][== DeeString_Hash(csf_name)] */
	ptrdiff_t                      csf_offset; /* [valid_if(csf_name)] Offset of the field (from `CStruct_Data()') */
	struct cstruct_field          *csf_next;   /* [0..1] Next field for the purposes of anonymous initialization and field ordering */
	DREF CLValueType              *csf_lvtype; /* [1..1][valid_if(csf_name)] l-value variant of this field's type. */
};

#define cstruct_field_nextfield(self) (self)->csf_next
#define cstruct_field_getlvtype(self) (self)->csf_lvtype
#define cstruct_field_getoffset(self) (self)->csf_offset
#define cstruct_field_getname(self)   (self)->csf_name
#define cstruct_field_gettype(self)   CLValueType_PointedToType(cstruct_field_getlvtype(self))

struct cstruct_type_object {
	/* Examples:
	 * >> local x = struct { .x = int };
	 *    cst_first = &cst_fvec[0]  // "x"
	 *    cst_size  = 2
	 *    cst_fmsk  = 1
	 *    cst_fvec  = { {"x", 0, NULL, <int>}, DUMMY }
	 *
	 * >> local x = struct { ("x", int), union { .s = int, .u = uint } };
	 * >> print str(x());  // "{ .x = 0, { .s = 0, .u = 0 } }"
	 * >> print repr(x()); // "struct...(x: 0, union...(s: 0))"
	 *    cst_first = &cst_fvec[0]  // "x"
	 *    cst_size  = 9
	 *    cst_fmsk  = 7
	 *    cst_fvec  = {
	 *        [0] = {"x", 0, &cst_fvec[8], <int>},
	 *        [1] = {"s", 4, NULL, <int>},
	 *        [2] = {"u", 4, NULL, <uint>},
	 *        [3] = DUMMY,
	 *        [4] = DUMMY,
	 *        [5] = DUMMY,
	 *        [6] = DUMMY,
	 *        [7] = DUMMY,
	 *        // No longer part of "cst_fmsk" -- anonymous fields
	 *        [8] = {"", 4, NULL, <union{.s=int,.u=uint}> }
	 *    }
	 *
	 * NOTES:
	 * - When processing unnamed arguments in "co_initwith", those arguments are
	 *   used to initialize struct members enumerated via `cst_first->[...]->csf_next'
	 * - The same method is used to implement the object-printing functions
	 */
	CType                                         cst_base;  /* The underlying C-type object. */
	struct cstruct_field                         *cst_first; /* [0..1][const] First field in `cst_fvec' */
	Dee_hash_t                                    cst_fmsk;  /* [const] Field-vector mask. */
	size_t                                        cst_size;  /* [const][<= (cst_fmsk+1)] # of elements in `cst_fvec' */
	COMPILER_FLEXIBLE_ARRAY(struct cstruct_field, cst_fvec); /* [1..cst_size][const] Hash-vector of field names, followed by `cst_size - (cst_fmsk + 1)' anonymous fields (which use "Dee_EmptyString" as name) */
};

#define STRUCT_TYPE_HASHST(self, hash)  ((hash) & ((CStructType *)(self))->cst_fmsk)
#define STRUCT_TYPE_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define STRUCT_TYPE_HASHIT(self, i)     (((CStructType *)(self))->cst_fvec + ((i) & ((CStructType *)(self))->cst_fmsk))

/* Query properties of a given Struct-Type */
#define CStructType_AllocInstance(self)   ((CStruct *)DeeType_AllocInstance(CStructType_AsType(self)))
#define CStructType_FreeInstance(self, p) DeeType_FreeInstance(CStructType_AsType(self), p)
#define CStructType_FirstField(self) (self)->cst_first
#define CStructType_ForeachField(field, self)                     \
	for ((field) = CStructType_FirstField(self); (field) != NULL; \
	     (field) = cstruct_field_nextfield(field))
INTDEF WUNUSED NONNULL((1, 2)) struct cstruct_field *DCALL
CStructType_FieldByName(CStructType const *self, /*string*/DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) struct cstruct_field *DCALL
CStructType_FieldByNameStringHash(CStructType const *self, char const *name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) struct cstruct_field *DCALL
CStructType_FieldByNameStringLenHash(CStructType const *self, char const *name,
                                     size_t namelen, Dee_hash_t hash);
INTDEF ATTR_COLD NONNULL((1)) int DCALL
err_no_such_struct_field(CStructType *__restrict type,
                         char const *field_name,
                         size_t field_name_length);


/* Construct a new struct-type from `initializer', which
 * should be `{((string, CType) | CStructType)...}'
 * @param: flags: Set of `CSTRUCTTYPE_F_*' */
INTDEF WUNUSED NONNULL((1)) DREF CStructType *DCALL
CStructType_Of(DeeObject *__restrict initializer,
               unsigned int flags, size_t min_alignment);
#define CSTRUCTTYPE_F_NORMAL  0x0000 /* No special flags. */
#define CSTRUCTTYPE_F_PACKED  0x0001 /* Create a packed structure. */
#define CSTRUCTTYPE_F_UNION   0x0002 /* Create a union. */

/* Construct a new struct-type from `initializer', which
 * should be `{((int, string, CType) | (int, CStructType))...}'
 * @param: flags: Set of `CSTRUCTTYPE_F_*' */
INTDEF WUNUSED NONNULL((1)) DREF CStructType *DCALL
CStructType_OfExtended(DeeObject *__restrict initializer, size_t min_alignment);


INTDEF DeeTypeObject CStructType_Type;   /* == type(ctypes.struct { .x = ctypes.int }) */
INTDEF struct empty_cstruct_type_object {
	CType                 cst_base;    /* The underlying C-type object. */
	struct cstruct_field *cst_first;   /* [0..1][const] First field in `cst_fvec' */
	Dee_hash_t            cst_fmsk;    /* [const] Field-vector mask. */
	size_t                cst_size;    /* [const][<= (cst_fmsk+1)] # of elements in `cst_fvec' */
	struct cstruct_field  cst_fvec[1]; /* [1..cst_size][const] Hash-vector of field names, followed by `cst_size - (cst_fmsk + 1)' anonymous fields (which use "Dee_EmptyString" as name) */
} AbstractCStruct_Type; /* == Type.__base__(ctypes.struct { .x = ctypes.int }) */


#define CStructType_AsCType(self)  (&(self)->cst_base)
#define CStructType_AsType(self)   (&(self)->cst_base.ct_base)
#define CType_AsCStructType(self)  COMPILER_CONTAINER_OF(self, CStructType, cst_base)
#define CType_IsCStructType(self)  (Dee_TYPE(&(self)->ct_base) == &CStructType_Type)
#define Type_AsCStructType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CStructType, cst_base.ct_base)
#define Type_IsCStructType(self)   (Dee_TYPE(self) == &CStructType_Type)
#define Object_AsCStructType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CStructType, cst_base.ct_base)
#define Object_IsCStructType(self) (Dee_TYPE(self) == &CStructType_Type)

struct cstruct_object {
	Dee_OBJECT_HEAD_EX(CStructType)
	COMPILER_FLEXIBLE_ARRAY(__BYTE_TYPE__, cs_data); /* Struct data... */
};

#define Object_AsCStruct(self) Dee_REQUIRES_OBJECT(CStruct, self)
#define Object_IsCStruct(self) DeeType_Extends(Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)), &CStructType_Type)

/* Interact with Struct objects */
#define CStruct_Alloc(tp)      CStructType_AllocInstance(tp)
#define CStruct_Free(tp, p)    CStructType_FreeInstance(tp, p)
#define CStruct_Init(self, tp) DeeObject_InitHeapEx(self, tp, CStructType_AsType) /* All struct types are heap-allocated */
#define CStruct_Data(self)     (self)->cs_data
INTDEF WUNUSED NONNULL((1, 3)) DREF CLValue *DCALL CStruct_GetAttr_p(CStructType *__restrict tp_self, void *self, /*String*/ DeeObject *attr, DeeObject *owner);
INTDEF WUNUSED NONNULL((1, 3)) DREF CLValue *DCALL CStruct_GetAttrStringHash_p(CStructType *__restrict tp_self, void *self, char const *attr, Dee_hash_t hash, DeeObject *owner);
INTDEF WUNUSED NONNULL((1, 3)) DREF CLValue *DCALL CStruct_GetAttrStringLenHash_p(CStructType *__restrict tp_self, void *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *owner);
INTDEF WUNUSED NONNULL((2)) DREF CLValue *DCALL CStruct_GetAttrField_p(void *self, struct cstruct_field const *field, DeeObject *owner);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL CStruct_DelAttr_p(CStructType *__restrict tp_self, void *self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL CStruct_DelAttrStringHash_p(CStructType *__restrict tp_self, void *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL CStruct_DelAttrStringLenHash_p(CStructType *__restrict tp_self, void *self, char const *attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((2)) int DCALL CStruct_DelAttrField_p(void *self, struct cstruct_field const *field);
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL CStruct_SetAttr_p(CStructType *__restrict tp_self, void *self, /*String*/ DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3, 5)) int DCALL CStruct_SetAttrStringHash_p(CStructType *__restrict tp_self, void *self, char const *attr, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3, 6)) int DCALL CStruct_SetAttrStringLenHash_p(CStructType *__restrict tp_self, void *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((2, 3)) int DCALL CStruct_SetAttrField_p(void *self, struct cstruct_field const *field, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL CStruct_GetAttr(CStruct *self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL CStruct_GetAttrStringHash(CStruct *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL CStruct_GetAttrStringLenHash(CStruct *self, char const *attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL CStruct_DelAttr(CStruct *self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL CStruct_DelAttrStringHash(CStruct *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL CStruct_DelAttrStringLenHash(CStruct *self, char const *attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL CStruct_SetAttr(CStruct *self, /*String*/ DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL CStruct_SetAttrStringHash(CStruct *self, char const *attr, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL CStruct_SetAttrStringLenHash(CStruct *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);






/************************************************************************/
/* FUNCTION TYPE                                                        */
/************************************************************************/
struct cfunction_type_object {
	CType                             cft_base;            /* The underlying structured type descriptor. */
#ifndef CONFIG_NO_CFUNCTION
	/* [1..1][const] High-level wrapper for invoking function pointers matching this prototype. */
	DREF DeeObject           *(DCALL *cft_call)(CFunctionType *__restrict tp_self, Dee_funptr_t self, size_t argc, DeeObject *const *argv);
	DREF CType                       *cft_return;          /* [1..1][const] The function's return type. */
	LIST_ENTRY(cfunction_type_object) cft_chain;           /* [lock(cft_return->st_cachelock)] Hash-map entry of this c-function. */
	Dee_hash_t                        cft_hash;            /* [const] A pre-calculated hash used by `struct stype_cfunction' */
	size_t                            cft_argc;            /* [const] Amount of function argument types. */
	ctypes_cc_t                       cft_cc;              /* [const] The calling convention used by this function. */
	ffi_type                         *cft_ffi_return_type; /* [1..1] Raw return type. */
	ffi_type                  *const *cft_ffi_arg_type_v;  /* [1..1][0..ob_argc][owned] Raw argument types. */
	ffi_cif                           cft_ffi_cif;         /* cif object to call the function. */
	/* WBuffer layout:
	 *  1. return value memory
	 *  2. argument memory...
	 *  3. argument pointers... */
	size_t                            cft_wsize;
	size_t                            cft_woff_argmem;
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
	union {
		size_t                        cft_woff_argptr;
		size_t                        cft_woff_variadic_argmem;
	};
#else /* __COMPILER_HAVE_TRANSPARENT_UNION */
	size_t                            cft_woff_argptr;
#define cft_woff_variadic_argmem      cft_woff_argptr
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	COMPILER_FLEXIBLE_ARRAY(DREF CType *, cft_argv);       /* [1..1][0..cft_argc][const] Vector of function argument types. */
#endif /* !CONFIG_NO_CFUNCTION */
};

/* Query properties of a given Function-Type */
#ifndef CONFIG_NO_CFUNCTION
#define CFunction_ReturnType(self)      (self)->cft_return
#define CFunction_ArgumentCount(self)   (self)->cft_argc
#define CFunction_ArgumentTypes(self)   (self)->cft_argv
#define CFunction_ArgumentType(self, i) (self)->cft_argv[i]
#endif /* !CONFIG_NO_CFUNCTION */

INTDEF DeeTypeObject CFunctionType_Type; /* == type(ctypes.int.func(ctypes.int, ctypes.int)) */
INTDEF struct empty_cfunction_type_object {
	CType                             cft_base;            /* The underlying structured type descriptor. */
#ifndef CONFIG_NO_CFUNCTION
	DREF DeeObject           *(DCALL *cft_call)(CFunctionType *__restrict tp_self, Dee_funptr_t self, size_t argc, DeeObject *const *argv);
	DREF CType                       *cft_return;          /* [1..1][const] The function's return type. */
	LIST_ENTRY(cfunction_type_object) cft_chain;           /* [lock(cft_return->st_cachelock)] Hash-map entry of this c-function. */
	Dee_hash_t                        cft_hash;            /* [const] A pre-calculated hash used by `struct stype_cfunction' */
	size_t                            cft_argc;            /* [const] Amount of function argument types. */
	ctypes_cc_t                       cft_cc;              /* [const] The calling convention used by this function. */
	ffi_type                         *cft_ffi_return_type; /* [1..1] Raw return type. */
	ffi_type                  *const *cft_ffi_arg_type_v;  /* [1..1][0..ob_argc][owned] Raw argument types. */
	ffi_cif                           cft_ffi_cif;         /* cif object to call the function. */
	size_t                            cft_wsize;
	size_t                            cft_woff_argmem;
	size_t                            cft_woff_argptr;
#endif /* !CONFIG_NO_CFUNCTION */
} AbstractCFunction_Type; /* == Type.__base__(ctypes.int.func(ctypes.int, ctypes.int)) */

#define CFunctionType_AsCType(self)  (&(self)->cft_base)
#define CFunctionType_AsType(self)   (&(self)->cft_base.ct_base)
#define CType_AsCFunctionType(self)  COMPILER_CONTAINER_OF(self, CFunctionType, cft_base)
#define CType_IsCFunctionType(self)  (Dee_TYPE(&(self)->ct_base) == &CFunctionType_Type)
#define Type_AsCFunctionType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CFunctionType, cft_base.ct_base)
#define Type_IsCFunctionType(self)   (Dee_TYPE(self) == &CFunctionType_Type)
#define Object_AsCFunctionType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CFunctionType, cft_base.ct_base)
#define Object_IsCFunctionType(self) (Dee_TYPE(self) == &CFunctionType_Type)

/* Construct a C-Function type for the given arguments */
INTDEF WUNUSED NONNULL((1)) DREF CFunctionType *DCALL
CFunctionType_Of(CType *__restrict return_type,
                 ctypes_cc_t calling_convention,
                 size_t argc, CType *const *argv);

#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
struct cfunction_object {
	Dee_OBJECT_HEAD_EX(CFunctionType)
	union {
		Dee_funptr_t cff_func; /* [1..1][const] Function pointer (the prototype of this matches "CFunctionType") */
		void        *cff_vptr; /* [1..1][const] Used internally */
	} cf_func;
	DREF DeeObject  *cf_cb;    /* [1..1][const] Object that gets invoked by `cff_func' */
	ffi_closure     *cf_write; /* [1..1][owned] The writable callback function pointer. */
#ifndef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES_REUSE_CFI
	ffi_cif          cf_cif;   /* cif object to call the function. */
#endif /* !CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES_REUSE_CFI */
};

#define CFunction_Func(self) (self)->cf_func.cff_func

#define Object_AsCFunction(self) Dee_REQUIRES_OBJECT(CFunction, self)
#define Object_IsCFunction(self) DeeType_Extends(Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)), &CFunctionType_Type)

#define CFunction_Alloc()                 DeeObject_MALLOC(CFunction)
#define CFunction_Free(p)                 DeeObject_FREE(Dee_REQUIRES_TYPE(CFunction *, p))
#define CFunction_Init(self, tp)          DeeObject_InitHeapEx(self, tp, CFunctionType_AsType)          /* All function types are heap-types */
#define CFunction_InitInherited(self, tp) DeeObject_InitHeapInheritedEx(self, tp, CFunctionType_AsType) /* All function types are heap-types */

/* Construct a new C-function that invokes "cb" when called. */
INTDEF WUNUSED NONNULL((1, 2)) DREF CFunction *DCALL
CFunction_New(CFunctionType *prototype, DeeObject *cb);
#endif /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */

/* Interact with Function objects */
#ifndef CONFIG_NO_CFUNCTION
#ifdef __INTELLISENSE__
WUNUSED NONNULL((1, 3, 4)) DREF DeeObject *DCALL
CFunction_Call(CFunctionType *__restrict tp_self, Dee_funptr_t funcptr,
               size_t argc, DeeObject *const *argv);
#else /* __INTELLISENSE__ */
#define CFunction_Call(tp_self, funcptr, argc, argv) \
	(*(tp_self)->cft_call)(tp_self, funcptr, argc, argv)
#endif /* !__INTELLISENSE__ */
#endif /* !CONFIG_NO_CFUNCTION */




/************************************************************************/
/* POINTER TYPE                                                         */
/************************************************************************/
struct cpointer_type_object {
	CType       cpt_base; /* The underlying type object. */
	DREF CType *cpt_orig; /* [1..1][const] The dereferenced type of a pointer. */
	size_t      cpt_size; /* [const][== CType_Sizeof(cpt_orig)] The size of the pointed-to type. */
};

/* Query properties of a given Pointer-Type */
#define CPointerType_PointedToType(self)       (self)->cpt_orig
#define CPointerType_SizeofPointedToType(self) (self)->cpt_size

INTDEF DeeTypeObject CPointerType_Type;    /* == type(ctypes.int.ptr) */
INTDEF CPointerType AbstractCPointer_Type; /* == Type.__base__(ctypes.int.ptr) */

#define CPointerType_AsCType(self)  (&(self)->cpt_base)
#define CPointerType_AsType(self)   (&(self)->cpt_base.ct_base)
#define CType_AsCPointerType(self)  COMPILER_CONTAINER_OF(self, CPointerType, cpt_base)
#define CType_IsCPointerType(self)  (Dee_TYPE(&(self)->ct_base) == &CPointerType_Type)
#define Type_AsCPointerType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CPointerType, cpt_base.ct_base)
#define Type_IsCPointerType(self)   (Dee_TYPE(self) == &CPointerType_Type)
#define Object_AsCPointerType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CPointerType, cpt_base.ct_base)
#define Object_IsCPointerType(self) (Dee_TYPE(self) == &CPointerType_Type)

INTDEF WUNUSED NONNULL((1)) DREF CPointerType *DCALL CPointerType_Of(CType *__restrict self);

struct cpointer_object {
	Dee_OBJECT_HEAD_EX(CPointerType)
	union pointer   cp_value; /* [const] Pointer value */
	DREF DeeObject *cp_owner; /* [0..1][const] Owner of pointed-to value (to keep "cp_value" alive) */
};

#define Object_AsCPointer(self) Dee_REQUIRES_OBJECT(CPointer, self)
#define Object_IsCPointer(self) (Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)) == &CPointerType_Type)

/* Interact with Pointer objects */
INTDEF WUNUSED NONNULL((1)) DREF CLValue *DCALL CPointer_Ind(CPointer *__restrict self);

/* Construct a new pointer object */
#define CPointer_Alloc()        DeeObject_MALLOC(CPointer)
#define CPointer_Free(p)        DeeObject_FREE(Dee_REQUIRES_TYPE(CPointer *, p))
#ifdef CTYPES_DEFINE_STATIC_POINTER_TYPES
#define CPointer_Init(self, tp)          DeeObject_InitEx(self, tp, CPointerType_AsType)              /* Some pointer types are statically defined */
#define CPointer_InitInherited(self, tp) DeeObject_InitInheritedEx(self, tp, CPointerType_AsType)     /* Some pointer types are statically defined */
#else /* CTYPES_DEFINE_STATIC_POINTER_TYPES */
#define CPointer_Init(self, tp)          DeeObject_InitHeapEx(self, tp, CPointerType_AsType)          /* All pointer types are heap-allocated */
#define CPointer_InitInherited(self, tp) DeeObject_InitHeapInheritedEx(self, tp, CPointerType_AsType) /* All pointer types are heap-allocated */
#endif /* !CTYPES_DEFINE_STATIC_POINTER_TYPES */
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL CPointer_New(CPointerType *__restrict type, void *value);
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL CPointer_NewInherited(/*inherit(always)*/ DREF CPointerType *__restrict type, void *value);
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL CPointer_NewEx(CPointerType *__restrict type, void *value, /*[0..1]*/ DeeObject *owner);
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL CPointer_NewExInherited(/*inherit(always)*/ DREF CPointerType *__restrict type, void *value, /*inherit(always)*/ /*[0..1]*/ DREF DeeObject *owner);
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL CPointer_For(CType *__restrict pointed_to_type, void *value);
#ifdef CTYPES_DEFINE_STATIC_POINTER_TYPES
#define CPointer_NewVoid(pointer_value) CPointer_New(&CVoidPtr_Type, pointer_value)
#define CPointer_NewChar(pointer_value) CPointer_New(&CCharPtr_Type, pointer_value)
#else /* CTYPES_DEFINE_STATIC_POINTER_TYPES */
#define CPointer_NewVoid(pointer_value) CPointer_For(&CVoid_Type, pointer_value)
#define CPointer_NewChar(pointer_value) CPointer_For(&CChar_Type, pointer_value)
#endif /* !CTYPES_DEFINE_STATIC_POINTER_TYPES */

/* Return a new pointer offset by `index' */
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL
CPointer_PlusOffset(CPointer *__restrict self, ptrdiff_t index);

/* Return an L-Value after `index' as an offset to "self"  */
INTDEF WUNUSED NONNULL((1)) DREF CLValue *DCALL
CPointer_GetItem(CPointer *__restrict self, ptrdiff_t index);


/* Interpret `self' as a pointer and store the result in `*result'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPointer(DeeObject *self, CType *pointer_base,
                    union pointer *__restrict result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPPointer(DeeObject *self, CType *pointer_base_base,
                     void ***__restrict p_result);

/* Same as `DeeObject_AsPointer()', but only ~try~ to interpret it.
 * @return:  1: The conversion failed.
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsPointer(DeeObject *self, CType *pointer_base,
                       union pointer *__restrict result);

/* Similar to `DeeObject_TryAsPointer()', but fills in `*p_pointer_base' with the
 * pointer-base type. For use with type-generic functions (such as the `atomic_*' api)
 * @return:  1: The conversion failed.
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsGenericPointer(DeeObject *self,
                              CType **__restrict p_pointer_base,
                              union pointer *__restrict result);

/* S.a. `DeeObject_TryAsGenericPointer()'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsGenericPointer(DeeObject *self,
                           CType **__restrict p_pointer_base,
                           union pointer *__restrict result);







/************************************************************************/
/* LVALUE TYPE                                                          */
/************************************************************************/
struct clvalue_type_object {
	CType       clt_base; /* The underlying type object. */
	DREF CType *clt_orig; /* [1..1][const] The dereferenced type of an l-value. */

	/* Apply additional l-value dereferences that are necessary
	 * when working with lvalue-of-lvalue-[of-lvalue-[of-...]]
	 *
	 * WARNING: This operator must be called from within CTYPES_FAULTPROTECT */
	WUNUSED_T NONNULL_T((1))
	void *(DFCALL *clt_getlogicalptr)(CLValueType const *tp_self, void *ptr);
	DREF CType    *clt_logicalorig; /* [1..1][const] The dereferenced type of an l-value (after stripping away all recursive l-values) */
	size_t         clt_logicalind;  /* [const] Number of extra indirections necessary to reach "clt_logicalorig" */
};

/* Query properties of a given LValue-Type */
#define CLValueType_PointedToType(self)             (self)->clt_orig
#define CLValueType_PointedToOperators(self)        CType_Operators(CLValueType_PointedToType(self))
#define CLValueType_LogicalPointedToType(self)      (self)->clt_logicalorig
#define CLValueType_LogicalPointedToOperators(self) CType_Operators(CLValueType_LogicalPointedToType(self))
#define CLValueType_LogicalPtr(self, ptr)           ((*(self)->clt_getlogicalptr)(self, ptr))

INTDEF DeeTypeObject CLValueType_Type;   /* == type(ctypes.int.lvalue) */
INTDEF CLValueType AbstractCLValue_Type; /* == Type.__base__(ctypes.int.lvalue) */

#define CLValueType_AsCType(self)  (&(self)->clt_base)
#define CLValueType_AsType(self)   (&(self)->clt_base.ct_base)
#define CType_AsCLValueType(self)  COMPILER_CONTAINER_OF(self, CLValueType, clt_base)
#define CType_IsCLValueType(self)  (Dee_TYPE(&(self)->ct_base) == &CLValueType_Type)
#define Type_AsCLValueType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CLValueType, clt_base.ct_base)
#define Type_IsCLValueType(self)   (Dee_TYPE(self) == &CLValueType_Type)
#define Object_AsCLValueType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CLValueType, clt_base.ct_base)
#define Object_IsCLValueType(self) (Dee_TYPE(self) == &CLValueType_Type)

INTDEF WUNUSED NONNULL((1)) DREF CLValueType *DCALL CLValueType_Of(CType *__restrict self);

struct clvalue_object {
	Dee_OBJECT_HEAD_EX(CLValueType)
	union pointer   cl_value; /* [const] LValue pointer */
	DREF DeeObject *cl_owner; /* [0..1][const] Owner of pointed-to value (to keep "cl_value" alive) */
};

#define Object_AsCLValue(self) Dee_REQUIRES_OBJECT(CLValue, self)
#define Object_IsCLValue(self) (Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)) == &CLValueType_Type)

/* Interact with LValue objects */
#define CLValue_Alloc()                 DeeObject_MALLOC(CLValue)
#define CLValue_Free(p)                 DeeObject_FREE(Dee_REQUIRES_TYPE(CLValue *, p))
#define CLValue_Init(self, tp)          DeeObject_InitHeapEx(self, tp, CLValueType_AsType)          /* All L-Value types are heap-types */
#define CLValue_InitInherited(self, tp) DeeObject_InitHeapInheritedEx(self, tp, CLValueType_AsType) /* All L-Value types are heap-types */
INTDEF WUNUSED NONNULL((1)) DREF CPointer *DCALL CLValue_Ptr(CLValue *__restrict self);

#define CLValue_GetValue(self)        (self)->cl_value.ptr
#define CLValue_GetLogicalValue(self) CLValueType_LogicalPtr(Dee_TYPE(self), CLValue_GetValue(self))






/* Builtin C types */
/* clang-format off */
typedef struct { Dee_OBJECT_HEAD_EX(CType)                          } CVoid;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_char     c_value; } CChar;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_wchar_t  c_value; } CWChar;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_char16_t c_value; } CChar16;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_char32_t c_value; } CChar32;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_bool     c_value; } CBool;
typedef struct { Dee_OBJECT_HEAD_EX(CType) int8_t          c_value; } CInt8;
typedef struct { Dee_OBJECT_HEAD_EX(CType) int16_t         c_value; } CInt16;
typedef struct { Dee_OBJECT_HEAD_EX(CType) int32_t         c_value; } CInt32;
typedef struct { Dee_OBJECT_HEAD_EX(CType) int64_t         c_value; } CInt64;
typedef struct { Dee_OBJECT_HEAD_EX(CType) Dee_int128_t    c_value; } CInt128;
typedef struct { Dee_OBJECT_HEAD_EX(CType) uint8_t         c_value; } CUInt8;
typedef struct { Dee_OBJECT_HEAD_EX(CType) uint16_t        c_value; } CUInt16;
typedef struct { Dee_OBJECT_HEAD_EX(CType) uint32_t        c_value; } CUInt32;
typedef struct { Dee_OBJECT_HEAD_EX(CType) uint64_t        c_value; } CUInt64;
typedef struct { Dee_OBJECT_HEAD_EX(CType) Dee_uint128_t   c_value; } CUInt128;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_float    c_value; } CFloat;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_double   c_value; } CDouble;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_ldouble  c_value; } CLDouble;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_schar    c_value; } CSChar;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_uchar    c_value; } CUChar;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_short    c_value; } CShort;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_ushort   c_value; } CUShort;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_int      c_value; } CInt;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_uint     c_value; } CUInt;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_long     c_value; } CLong;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_ulong    c_value; } CULong;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_llong    c_value; } CLLong;
typedef struct { Dee_OBJECT_HEAD_EX(CType) CTYPES_ullong   c_value; } CULLong;
typedef CInt16 CBSwapInt16;
typedef CInt32 CBSwapInt32;
typedef CInt64 CBSwapInt64;
typedef CInt128 CBSwapInt128;
typedef CUInt16 CBSwapUInt16;
typedef CUInt32 CBSwapUInt32;
typedef CUInt64 CBSwapUInt64;
typedef CUInt128 CBSwapUInt128;
/* clang-format on */


/* Builtin C Types */
INTDEF CType CVoid_Type;

INTDEF CType CChar_Type;
INTDEF CType CWChar_Type;
INTDEF CType CChar16_Type;
INTDEF CType CChar32_Type;
INTDEF CType CBool_Type;

INTDEF CType CInt8_Type;
INTDEF CType CInt16_Type;
INTDEF CType CInt32_Type;
INTDEF CType CInt64_Type;
INTDEF CType CInt128_Type;
INTDEF CType CUInt8_Type;
INTDEF CType CUInt16_Type;
INTDEF CType CUInt32_Type;
INTDEF CType CUInt64_Type;
INTDEF CType CUInt128_Type;

#define CBSwapInt8_Type CInt8_Type
INTDEF CType CBSwapInt16_Type;
INTDEF CType CBSwapInt32_Type;
INTDEF CType CBSwapInt64_Type;
INTDEF CType CBSwapInt128_Type;
#define CBSwapUInt8_Type CUInt8_Type
INTDEF CType CBSwapUInt16_Type;
INTDEF CType CBSwapUInt32_Type;
INTDEF CType CBSwapUInt64_Type;
INTDEF CType CBSwapUInt128_Type;

INTDEF CType CFloat_Type;
INTDEF CType CDouble_Type;
INTDEF CType CLDouble_Type;


INTDEF WUNUSED DREF CObject *DCALL CChar_New(CTYPES_char val);
INTDEF WUNUSED DREF CObject *DCALL CWChar_New(CTYPES_wchar_t val);
INTDEF WUNUSED DREF CObject *DCALL CChar16_New(CTYPES_char16_t val);
INTDEF WUNUSED DREF CObject *DCALL CChar32_New(CTYPES_char32_t val);
INTDEF WUNUSED DREF CObject *DCALL CBool_New(CTYPES_bool value);
INTDEF WUNUSED DREF CObject *DCALL CInt8_New(int8_t val);
INTDEF WUNUSED DREF CObject *DCALL CUInt8_New(uint8_t val);
INTDEF WUNUSED DREF CObject *DCALL CInt16_New(int16_t val);
INTDEF WUNUSED DREF CObject *DCALL CUInt16_New(uint16_t val);
INTDEF WUNUSED DREF CObject *DCALL CInt32_New(int32_t val);
INTDEF WUNUSED DREF CObject *DCALL CUInt32_New(uint32_t val);
INTDEF WUNUSED DREF CObject *DCALL CInt64_New(int64_t val);
INTDEF WUNUSED DREF CObject *DCALL CUInt64_New(uint64_t val);
INTDEF WUNUSED DREF CObject *DCALL CInt128_New(Dee_int128_t val);
INTDEF WUNUSED DREF CObject *DCALL CUInt128_New(Dee_uint128_t val);
INTDEF WUNUSED DREF CObject *DCALL CFloat_New(CTYPES_float val);
INTDEF WUNUSED DREF CObject *DCALL CDouble_New(CTYPES_double val);
INTDEF WUNUSED DREF CObject *DCALL CLDouble_New(CTYPES_ldouble val);


/* Statically allocated pointer types (since these are needed very often) */
#ifdef CTYPES_DEFINE_STATIC_POINTER_TYPES
INTDEF CPointerType CVoidPtr_Type;
INTDEF CPointerType CCharPtr_Type;
#endif /* CTYPES_DEFINE_STATIC_POINTER_TYPES */

#define PRIVATE_CINT_TYPE_1   CInt8_Type
#define PRIVATE_CINT_TYPE_2   CInt16_Type
#define PRIVATE_CINT_TYPE_4   CInt32_Type
#define PRIVATE_CINT_TYPE_8   CInt64_Type
#define PRIVATE_CINT_TYPE_16  CInt128_Type
#define PRIVATE_CUINT_TYPE_1  CUInt8_Type
#define PRIVATE_CUINT_TYPE_2  CUInt16_Type
#define PRIVATE_CUINT_TYPE_4  CUInt32_Type
#define PRIVATE_CUINT_TYPE_8  CUInt64_Type
#define PRIVATE_CUINT_TYPE_16 CUInt128_Type
#define CIntN_Type(sizeof)    PP_PRIVATE_CAT2(PRIVATE_CINT_TYPE_, sizeof)
#define CUIntN_Type(sizeof)   PP_PRIVATE_CAT2(PRIVATE_CUINT_TYPE_, sizeof)

#define PRIVATE_CBSWAPINT_TYPE_1   CBSwapInt8_Type
#define PRIVATE_CBSWAPINT_TYPE_2   CBSwapInt16_Type
#define PRIVATE_CBSWAPINT_TYPE_4   CBSwapInt32_Type
#define PRIVATE_CBSWAPINT_TYPE_8   CBSwapInt64_Type
#define PRIVATE_CBSWAPINT_TYPE_16  CBSwapInt128_Type
#define PRIVATE_CBSWAPUINT_TYPE_1  CBSwapUInt8_Type
#define PRIVATE_CBSWAPUINT_TYPE_2  CBSwapUInt16_Type
#define PRIVATE_CBSWAPUINT_TYPE_4  CBSwapUInt32_Type
#define PRIVATE_CBSWAPUINT_TYPE_8  CBSwapUInt64_Type
#define PRIVATE_CBSWAPUINT_TYPE_16 CBSwapUInt128_Type
#define CBSwapIntN_Type(sizeof)    PP_PRIVATE_CAT2(PRIVATE_CBSWAPINT_TYPE_, sizeof)
#define CBSwapUIntN_Type(sizeof)   PP_PRIVATE_CAT2(PRIVATE_CBSWAPUINT_TYPE_, sizeof)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CLeIntN_Type  CIntN_Type
#define CLeUIntN_Type CUIntN_Type
#define CBeIntN_Type  CBSwapIntN_Type
#define CBeUIntN_Type CBSwapUIntN_Type
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define CLeIntN_Type  CBSwapIntN_Type
#define CLeUIntN_Type CBSwapUIntN_Type
#define CBeIntN_Type  CIntN_Type
#define CBeUIntN_Type CUIntN_Type
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */


#define PRIVATE_CINT_NEW_1   CInt8_New
#define PRIVATE_CINT_NEW_2   CInt16_New
#define PRIVATE_CINT_NEW_4   CInt32_New
#define PRIVATE_CINT_NEW_8   CInt64_New
#define PRIVATE_CINT_NEW_16  CInt128_New
#define PRIVATE_CUINT_NEW_1  CUInt8_New
#define PRIVATE_CUINT_NEW_2  CUInt16_New
#define PRIVATE_CUINT_NEW_4  CUInt32_New
#define PRIVATE_CUINT_NEW_8  CUInt64_New
#define PRIVATE_CUINT_NEW_16 CUInt128_New
#define CIntN_New(sizeof)    PP_PRIVATE_CAT2(PRIVATE_CINT_NEW_, sizeof)
#define CUIntN_New(sizeof)   PP_PRIVATE_CAT2(PRIVATE_CUINT_NEW_, sizeof)


#if CTYPES_sizeof_char == 1 && CTYPES_alignof_char == __ALIGNOF_INT8__
#define CSChar_Type CInt8_Type
#define CSChar_New  CInt8_New
#define CUChar_Type CUInt8_Type
#define CUChar_New  CUInt8_New
#elif CTYPES_sizeof_char == 2 && CTYPES_alignof_char == __ALIGNOF_INT16__
#define CSChar_Type CInt16_Type
#define CSChar_New  CInt16_New
#define CUChar_Type CUInt16_Type
#define CUChar_New  CUInt16_New
#elif CTYPES_sizeof_char == 4 && CTYPES_alignof_char == __ALIGNOF_INT32__
#define CSChar_Type CInt32_Type
#define CSChar_New  CInt32_New
#define CUChar_Type CUInt32_Type
#define CUChar_New  CUInt32_New
#elif CTYPES_sizeof_char == 8 && CTYPES_alignof_char == __ALIGNOF_INT64__
#define CSChar_Type CInt64_Type
#define CSChar_New  CInt64_New
#define CUChar_Type CUInt64_Type
#define CUChar_New  CUInt64_New
#elif CTYPES_sizeof_char == 16 && CTYPES_alignof_char == __ALIGNOF_INT128__
#define CSChar_Type CInt128_Type
#define CSChar_New  CInt128_New
#define CUChar_Type CUInt128_Type
#define CUChar_New  CUInt128_New
#else /* ... */
#define CONFIG_SUCHAR_NEEDS_OWN_TYPE
INTDEF CType CSChar_Type;
INTDEF CType CUChar_Type;
INTDEF WUNUSED DREF CObject *DCALL CSChar_New(CTYPES_schar val);
INTDEF WUNUSED DREF CObject *DCALL CUChar_New(CTYPES_uchar val);
#endif /* !... */

#if CTYPES_sizeof_short == 1 && CTYPES_alignof_short == __ALIGNOF_INT8__
#define CShort_Type  CInt8_Type
#define CShort_New   CInt8_New
#define CUShort_Type CUInt8_Type
#define CUShort_New  CUInt8_New
#elif CTYPES_sizeof_short == 2 && CTYPES_alignof_short == __ALIGNOF_INT16__
#define CShort_Type  CInt16_Type
#define CShort_New   CInt16_New
#define CUShort_Type CUInt16_Type
#define CUShort_New  CUInt16_New
#elif CTYPES_sizeof_short == 4 && CTYPES_alignof_short == __ALIGNOF_INT32__
#define CShort_Type  CInt32_Type
#define CShort_New   CInt32_New
#define CUShort_Type CUInt32_Type
#define CUShort_New  CUInt32_New
#elif CTYPES_sizeof_short == 8 && CTYPES_alignof_short == __ALIGNOF_INT64__
#define CShort_Type  CInt64_Type
#define CShort_New   CInt64_New
#define CUShort_Type CUInt64_Type
#define CUShort_New  CUInt64_New
#elif CTYPES_sizeof_short == 16 && CTYPES_alignof_short == __ALIGNOF_INT128__
#define CShort_Type  CInt128_Type
#define CShort_New   CInt128_New
#define CUShort_Type CUInt128_Type
#define CUShort_New  CUInt128_New
#else /* ... */
#define CONFIG_SHORT_NEEDS_OWN_TYPE
INTDEF CType CShort_Type;
INTDEF CType CUShort_Type;
INTDEF WUNUSED DREF CObject *DCALL CShort_New(CTYPES_short val);
INTDEF WUNUSED DREF CObject *DCALL CUShort_New(CTYPES_ushort val);
#endif /* !... */

#if CTYPES_sizeof_int == 1 && CTYPES_alignof_int == __ALIGNOF_INT8__
#define CInt_Type  CInt8_Type
#define CInt_New   CInt8_New
#define CUInt_Type CUInt8_Type
#define CUInt_New  CUInt8_New
#elif CTYPES_sizeof_int == 2 && CTYPES_alignof_int == __ALIGNOF_INT16__
#define CInt_Type  CInt16_Type
#define CInt_New   CInt16_New
#define CUInt_Type CUInt16_Type
#define CUInt_New  CUInt16_New
#elif CTYPES_sizeof_int == 4 && CTYPES_alignof_int == __ALIGNOF_INT32__
#define CInt_Type  CInt32_Type
#define CInt_New   CInt32_New
#define CUInt_Type CUInt32_Type
#define CUInt_New  CUInt32_New
#elif CTYPES_sizeof_int == 8 && CTYPES_alignof_int == __ALIGNOF_INT64__
#define CInt_Type  CInt64_Type
#define CInt_New   CInt64_New
#define CUInt_Type CUInt64_Type
#define CUInt_New  CUInt64_New
#elif CTYPES_sizeof_int == 16 && CTYPES_alignof_int == __ALIGNOF_INT128__
#define CInt_Type  CInt128_Type
#define CInt_New   CInt128_New
#define CUInt_Type CUInt128_Type
#define CUInt_New  CUInt128_New
#else /* ... */
#define CONFIG_INT_NEEDS_OWN_TYPE
INTDEF CType CInt_Type;
INTDEF CType CUInt_Type;
INTDEF WUNUSED DREF CObject *DCALL CInt_New(CTYPES_int val);
INTDEF WUNUSED DREF CObject *DCALL CUInt_New(CTYPES_uint val);
#endif /* !... */

#if CTYPES_sizeof_long == 1 && CTYPES_alignof_long == __ALIGNOF_INT8__
#define CLong_Type  CInt8_Type
#define CLong_New   CInt8_New
#define CULong_Type CUInt8_Type
#define CULong_New  CUInt8_New
#elif CTYPES_sizeof_long == 2 && CTYPES_alignof_long == __ALIGNOF_INT16__
#define CLong_Type  CInt16_Type
#define CLong_New   CInt16_New
#define CULong_Type CUInt16_Type
#define CULong_New  CUInt16_New
#elif CTYPES_sizeof_long == 4 && CTYPES_alignof_long == __ALIGNOF_INT32__
#define CLong_Type  CInt32_Type
#define CLong_New   CInt32_New
#define CULong_Type CUInt32_Type
#define CULong_New  CUInt32_New
#elif CTYPES_sizeof_long == 8 && CTYPES_alignof_long == __ALIGNOF_INT64__
#define CLong_Type  CInt64_Type
#define CLong_New   CInt64_New
#define CULong_Type CUInt64_Type
#define CULong_New  CUInt64_New
#elif CTYPES_sizeof_long == 16 && CTYPES_alignof_long == __ALIGNOF_INT128__
#define CLong_Type  CInt128_Type
#define CLong_New   CInt128_New
#define CULong_Type CUInt128_Type
#define CULong_New  CUInt128_New
#else /* ... */
#define CONFIG_LONG_NEEDS_OWN_TYPE
INTDEF CType CLong_Type;
INTDEF CType CULong_Type;
INTDEF WUNUSED DREF CObject *DCALL CLong_New(CTYPES_long val);
INTDEF WUNUSED DREF CObject *DCALL CULong_New(CTYPES_ulong val);
#endif /* !... */

#if CTYPES_sizeof_llong == 1 && CTYPES_alignof_llong == __ALIGNOF_INT8__
#define CLLong_Type  CInt8_Type
#define CLLong_New   CInt8_New
#define CULLong_Type CUInt8_Type
#define CULLong_New  CUInt8_New
#elif CTYPES_sizeof_llong == 2 && CTYPES_alignof_llong == __ALIGNOF_INT16__
#define CLLong_Type  CInt16_Type
#define CLLong_New   CInt16_New
#define CULLong_Type CUInt16_Type
#define CULLong_New  CUInt16_New
#elif CTYPES_sizeof_llong == 4 && CTYPES_alignof_llong == __ALIGNOF_INT32__
#define CLLong_Type  CInt32_Type
#define CLLong_New   CInt32_New
#define CULLong_Type CUInt32_Type
#define CULLong_New  CUInt32_New
#elif CTYPES_sizeof_llong == 8 && CTYPES_alignof_llong == __ALIGNOF_INT64__
#define CLLong_Type  CInt64_Type
#define CLLong_New   CInt64_New
#define CULLong_Type CUInt64_Type
#define CULLong_New  CUInt64_New
#elif CTYPES_sizeof_llong == 16 && CTYPES_alignof_llong == __ALIGNOF_INT128__
#define CLLong_Type  CInt128_Type
#define CLLong_New   CInt128_New
#define CULLong_Type CUInt128_Type
#define CULLong_New  CUInt128_New
#else /* ... */
#define CONFIG_LLONG_NEEDS_OWN_TYPE
INTDEF CType CLLong_Type;
INTDEF CType CULLong_Type;
INTDEF WUNUSED DREF CObject *DCALL CLLong_New(CTYPES_llong val);
INTDEF WUNUSED DREF CObject *DCALL CULLong_New(CTYPES_ullong val);
#endif /* !... */

#if (!defined(CONFIG_LONG_NEEDS_OWN_TYPE) &&     \
     (CTYPES_sizeof_long == CTYPES_sizeof_int && \
      CTYPES_alignof_long == CTYPES_alignof_int))
/* Make `long' its own distinct type. */
#define CONFIG_LONG_NEEDS_OWN_TYPE
#undef CLong_Type
#undef CULong_Type
#undef CLong_New
#undef CULong_New
INTDEF CType CLong_Type;
INTDEF CType CULong_Type;
INTDEF WUNUSED DREF CObject *DCALL CLong_New(CTYPES_long val);
INTDEF WUNUSED DREF CObject *DCALL CULong_New(CTYPES_ulong val);
#endif /* ... */






/* Backwards compat... */
#define DeeStruct_Data        CObject_Data                      /* TODO: REMOVE ME */
#define DeePointer_NewChar(p) Dee_AsObject(CPointer_NewChar(p)) /* TODO: REMOVE ME */
#define DeePointer_NewVoid(p) Dee_AsObject(CPointer_NewVoid(p)) /* TODO: REMOVE ME */

/* A special type to interface with native system libraries:
 *
 * >> @@@throw FileNotFound The given @name wasn't found
 * >> @@@throw ValueError The given @default_cc calling convention could not be found
 * >> this(name: string);
 * >> this(name: string, default_cc: string);
 * >>
 * >> @@Lookup exports
 * >> @@@throw KeyError: No export under that name
 * >> operator [] (name: string): void.ptr;
 * >> @@@throw AttributeError: No export under that name
 * >> operator . (name: string): int.vfunc(default_cc).ptr;
 * >>
 * >> @@Check export presence.
 * >> operator contains(name: string): bool;
 * >>
 * >> @@enumerate exported symbols.
 * >> @@Optional (if not supported, throw an UnsupportedAPI error)
 * >> operator iter(): Iterator;
 * >> operator enumattr();
 * >>
 * >> @@Returns the image load address.
 * >> @@NOTE: Only accessible via the class: `shlib.base(my_shlib)'
 * >> function base(): void.ptr;
 *
 * NOTE: This type is not derived from `deemon.Sequence', but simply `deemon.Object'
 */
INTDEF DeeTypeObject ShLib_Type;

#ifdef CONFIG_NO_CFUNCTION
/* Throw a NotImplemented error explaining that cfunctions have been disabled. */
INTDEF ATTR_COLD int DCALL err_no_cfunction(void);
#endif /* CONFIG_NO_CFUNCTION */

DECL_END

#endif /* !GUARD_DEX_CTYPES_LIBCTYPES_H */
