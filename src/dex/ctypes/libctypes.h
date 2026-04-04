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

#include <deemon/alloc.h>     /* DeeObject_*, DeeSlab_GetFree, DeeSlab_GetMalloc */
#include <deemon/mro.h>       /* Dee_attrhint, Dee_attriter */
#include <deemon/object.h>    /* DREF, DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD, Dee_TYPE, Dee_hash_t, Dee_int128_t, Dee_uint128_t, OBJECT_HEAD */
#include <deemon/string.h>    /* Dee_wchar_t */
#include <deemon/type.h>      /* DeeType_Base, OPERATOR_EXTENDED */
#include <deemon/util/lock.h> /* Dee_atomic_rwlock_* */

#include <hybrid/sequence/list.h> /* LIST_ENTRY */
#include <hybrid/typecore.h>      /* __ALIGNOF_INTn__, __ALIGNOF_POINTER__, __BYTE_TYPE__, __CHAR_UNSIGNED__, __HYBRID_ALIGNOF, __SIZEOF_*__, __WCHAR_UNSIGNED__ */
#include <hybrid/byteorder.h>      /* __ALIGNOF_INTn__, __ALIGNOF_POINTER__, __BYTE_TYPE__, __CHAR_UNSIGNED__, __HYBRID_ALIGNOF, __SIZEOF_*__, __WCHAR_UNSIGNED__ */
#include <hybrid/unaligned.h>      /* __ALIGNOF_INTn__, __ALIGNOF_POINTER__, __BYTE_TYPE__, __CHAR_UNSIGNED__, __HYBRID_ALIGNOF, __SIZEOF_*__, __WCHAR_UNSIGNED__ */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* intN_t, intptr_t, uintN_t, uintptr_t */

#ifndef CONFIG_NO_CFUNCTION
#include <ffi.h>
#include <ffitarget.h>
#endif /* !CONFIG_NO_CFUNCTION */

/*
 * Experimental feature that fully re-written ctypes to fix
 * various design-flaws of the original implementation:
 *
 * - Rename "StructuredType" to "CType"
 * - Raw "CType"s are always read-only  (except that the fields of "StructType"s
 *   return LValues, which in turn allows modifications of the underlying struct)
 * - Read-write operations are exposed by "LValueType", but there is no way to
 *   turn (e.g.) "local x = ctypes.uint32_t(42);" into an l-value to itself, or
 *   form a pointer to the value of "x".
 * - iow: raw "CType"s are now true R-Values
 * - Both "LValue" and "Pointer" objects (when existing outside of a "StructType")
 *   have an optional, hidden reference to the object into which they point (this
 *   reference is "NULL" when the object is unknown).
 *   This then makes lots of things much safer:
 *   >> local p = (struct { .x = int })(x: 42).x.ptr;
 *   >> print repr p.ind;       // 42  -- (because the pointer still knowns+references
 *   >>                         //         the underlying struct; in the old ctypes impl,
 *   >>                         //         this dereference used to be undefined and could
 *   >>                         //         even cause deemon to crash)
 *   >> print repr p.__owner__; // "(struct { .x = int })(x: 42)"
 */
#if (!defined(CONFIG_EXPERIMENTAL_REWORKED_CTYPES) && \
     !defined(CONFIG_NO_EXPERIMENTAL_REWORKED_CTYPES))
#if 0 /* TODO: Incomplete */
#define CONFIG_EXPERIMENTAL_REWORKED_CTYPES
#else
#define CONFIG_NO_EXPERIMENTAL_REWORKED_CTYPES
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_REWORKED_CTYPES */

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





#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
typedef struct ctype_object CType; /* Also doubles as the RValue type */
typedef struct carray_type_object CArrayType;
typedef struct cstruct_type_object CStructType;
typedef struct cfunction_type_object CFunctionType;
typedef struct cpointer_type_object CPointerType;
typedef struct clvalue_type_object CLValueType;

typedef struct cobject_object CObject;
typedef struct carray_object CArray;
typedef struct cstruct_object CStruct;
/*typedef struct cfunction_object CFunction;*/
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

	/* Initialize "self" by casting- and assigning "value" to it
	 * @return: 0 : Success
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1)) int
	(DCALL *co_initfrom)(CType *tp_self, void *self, DeeObject *value);

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
	                                      * As hash for indexing this map, use `ft_hash'. */
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
#define CType_Sizeof(self)          (self)->ct_sizeof
#define CType_Alignof(self)         (self)->ct_alignof
#define CType_AllocInstance(self)   ((CObject *)DeeType_AllocInstance(CType_AsType(self)))
#define CType_FreeInstance(self, p) DeeType_FreeInstance(CType_AsType(self), p)
#define CType_Operators(self)       (self)->ct_operators

INTDEF DeeTypeObject CType_Type;   /* == type(ctypes.int) */
INTDEF CType AbstractCObject_Type; /* == Type.__base__(ctypes.int) */

#define CType_AsType(self)   (&(self)->ct_base)
#define CType_AsObject(self) ((DeeObject *)&(self)->ct_base)
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
	OBJECT_HEAD_EX(CType)
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
	CType                          cat_base;  /* The underlying C-type descriptor. */
	DREF CType                    *cat_item;  /* [1..1][const] The array's element type. */
	LIST_ENTRY(carray_type_object) cat_chain; /* [lock(cat_item->st_cachelock)] Hash-map entry of this array. */
	size_t                         cat_count; /* [const] The total number of items. */
	size_t                         cat_isize; /* [const][== CType_Sizeof(cat_item)]. */
};

/* Query properties of a given Array-Type */
#define CArrayType_AllocInstance(self)       ((CStruct *)DeeType_AllocInstance(CArrayType_AsType(self)))
#define CArrayType_FreeInstance(self, p)     DeeType_FreeInstance(CArrayType_AsType(self), p)
#define CArrayType_Count(self)               (self)->cat_count
#define CArrayType_PointedToType(self)       (self)->cat_item
#define CArrayType_SizeofPointedToType(self) (self)->cat_isize

/* Construct a new array-type `item_type[item_count]' */
INTDEF WUNUSED NONNULL((1)) DREF CArrayType *DCALL
CArrayType_Of(CType *__restrict item_type, size_t item_count);

INTDEF DeeTypeObject CArrayType_Type;  /* == type(ctypes.int[42]) */
INTDEF CArrayType AbstractCArray_Type; /* == Type.__base__(ctypes.int[42]) */

#define CArrayType_AsCType(self)  (&(self)->cat_base)
#define CArrayType_AsType(self)   (&(self)->cat_base.ct_base)
#define CArrayType_AsObject(self) ((DeeObject *)&(self)->cat_base.ct_base)
#define CType_AsCArrayType(self)  COMPILER_CONTAINER_OF(self, CArrayType, cat_base)
#define CType_IsCArrayType(self)  (Dee_TYPE(&(self)->ct_base) == &CArrayType_Type)
#define Type_AsCArrayType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CArrayType, cat_base.ct_base)
#define Type_IsCArrayType(self)   (Dee_TYPE(self) == &CArrayType_Type)
#define Object_AsCArrayType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CArrayType, cat_base.ct_base)
#define Object_IsCArrayType(self) (Dee_TYPE(self) == &CArrayType_Type)

struct carray_object {
	OBJECT_HEAD_EX(CArrayType)
	COMPILER_FLEXIBLE_ARRAY(__BYTE_TYPE__, ca_data); /* Array data... */
};

#define Object_AsCArray(self) Dee_REQUIRES_OBJECT(CArray, self)
#define Object_IsCArray(self) DeeType_Extends(Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)), &CArrayType_Type)

/* Interact with Array objects */
#define CArray_Alloc(tp)      CArrayType_AllocInstance(tp)
#define CArray_Free(tp, p)    CArrayType_FreeInstance(tp, p)
#define CArray_Init(self, tp) DeeObject_InitHeapEx(self, tp, CArrayType_AsType) /* All array-types are heap-allocated */
#define CArray_ItemAddr(self, i) ((self)->ca_data + ((i) * CArrayType_SizeofPointedToType(Dee_TYPE(self))))
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
	ptrdiff_t                      csf_offset; /* [valid_if(csf_name)] Offset of the field (from `DeeStruct_Data()') */
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
	 *    cst_first = "x"
	 *    cst_size  = 2
	 *    cst_fmsk  = 1
	 *    cst_fvec  = { {"x", 0, NULL, <int>}, DUMMY }
	 *
	 * >> local x = struct { ("x", int), union { .s = int, .u = uint } };
	 * >> print str(x());  // "{ .x = 0, { .s = 0, .u = 0 } }"
	 * >> print repr(x()); // "struct...(x: 0, union...(s: 0))"
	 *    cst_first = "x"
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

/* Construct a new struct-type from `initializer', which
 * should be `{((string, CType) | CStructType)...}'
 * @param: flags: Set of `CSTRUCTTYPE_F_*' */
INTDEF WUNUSED NONNULL((1)) DREF CStructType *DCALL
CStructType_Of(DeeObject *__restrict initializer, unsigned int flags);
#define CSTRUCTTYPE_F_NORMAL  0x0000 /* No special flags. */
#define CSTRUCTTYPE_F_PACKED  0x0001 /* Create a packed structure. */
#define CSTRUCTTYPE_F_UNION   0x0002 /* Create a union. */


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
#define CStructType_AsObject(self) ((DeeObject *)&(self)->cst_base.ct_base)
#define CType_AsCStructType(self)  COMPILER_CONTAINER_OF(self, CStructType, cst_base)
#define CType_IsCStructType(self)  (Dee_TYPE(&(self)->ct_base) == &CStructType_Type)
#define Type_AsCStructType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CStructType, cst_base.ct_base)
#define Type_IsCStructType(self)   (Dee_TYPE(self) == &CStructType_Type)
#define Object_AsCStructType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CStructType, cst_base.ct_base)
#define Object_IsCStructType(self) (Dee_TYPE(self) == &CStructType_Type)

struct cstruct_object {
	OBJECT_HEAD_EX(CStructType)
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
#define CFunctionType_AsObject(self) ((DeeObject *)&(self)->cft_base.ct_base)
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

#if 0 /* Wouldn't make sense... */
struct cfunction_object {
	OBJECT_HEAD_EX(CFunctionType)
	COMPILER_FLEXIBLE_ARRAY(__BYTE_TYPE__, cf_instruction); /* Host instruction... */
};

#define Object_AsCFunction(self) Dee_REQUIRES_OBJECT(CFunction, self)
#define Object_IsCFunction(self) DeeType_Extends(Dee_TYPE((DeeTypeObject *)Dee_TYPE(self)), &CFunctionType_Type)
#endif

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
	size_t      cpt_size; /* [const][== CType_Sizeof(pt_orig)] The size of the pointed-to type. */
};

/* Query properties of a given Pointer-Type */
#define CPointerType_PointedToType(self)       (self)->cpt_orig
#define CPointerType_SizeofPointedToType(self) (self)->cpt_size

INTDEF DeeTypeObject CPointerType_Type;    /* == type(ctypes.int.ptr) */
INTDEF CPointerType AbstractCPointer_Type; /* == Type.__base__(ctypes.int.ptr) */

#define CPointerType_AsCType(self)  (&(self)->cpt_base)
#define CPointerType_AsType(self)   (&(self)->cpt_base.ct_base)
#define CPointerType_AsObject(self) ((DeeObject *)&(self)->cpt_base.ct_base)
#define CType_AsCPointerType(self)  COMPILER_CONTAINER_OF(self, CPointerType, cpt_base)
#define CType_IsCPointerType(self)  (Dee_TYPE(&(self)->ct_base) == &CPointerType_Type)
#define Type_AsCPointerType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CPointerType, cpt_base.ct_base)
#define Type_IsCPointerType(self)   (Dee_TYPE(self) == &CPointerType_Type)
#define Object_AsCPointerType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CPointerType, cpt_base.ct_base)
#define Object_IsCPointerType(self) (Dee_TYPE(self) == &CPointerType_Type)

INTDEF WUNUSED NONNULL((1)) DREF CPointerType *DCALL CPointerType_Of(CType *__restrict self);

struct cpointer_object {
	OBJECT_HEAD_EX(CPointerType)
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
                     union pointer **__restrict p_result);

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
};

/* Query properties of a given LValue-Type */
#define CLValueType_PointedToType(self)      (self)->clt_orig
#define CLValueType_PointedToOperators(self) CType_Operators(CLValueType_PointedToType(self))

INTDEF DeeTypeObject CLValueType_Type;   /* == type(ctypes.int.lvalue) */
INTDEF CLValueType AbstractCLValue_Type; /* == Type.__base__(ctypes.int.lvalue) */

#define CLValueType_AsCType(self)  (&(self)->clt_base)
#define CLValueType_AsType(self)   (&(self)->clt_base.ct_base)
#define CLValueType_AsObject(self) ((DeeObject *)&(self)->clt_base.ct_base)
#define CType_AsCLValueType(self)  COMPILER_CONTAINER_OF(self, CLValueType, clt_base)
#define CType_IsCLValueType(self)  (Dee_TYPE(&(self)->ct_base) == &CLValueType_Type)
#define Type_AsCLValueType(self)   COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CLValueType, clt_base.ct_base)
#define Type_IsCLValueType(self)   (Dee_TYPE(self) == &CLValueType_Type)
#define Object_AsCLValueType(self) COMPILER_CONTAINER_OF(Dee_REQUIRES_OBJECT(DeeTypeObject, self), CLValueType, clt_base.ct_base)
#define Object_IsCLValueType(self) (Dee_TYPE(self) == &CLValueType_Type)

INTDEF WUNUSED NONNULL((1)) DREF CLValueType *DCALL CLValueType_Of(CType *__restrict self);

struct clvalue_object {
	OBJECT_HEAD_EX(CLValueType)
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






/* Builtin C types */
/* clang-format off */
typedef struct { OBJECT_HEAD_EX(CType) } CVoid;
typedef struct { OBJECT_HEAD_EX(CType)  c_value; } CChar;
typedef struct { OBJECT_HEAD_EX(CType) Dee_wchar_t c_value; } CWChar;
typedef struct { OBJECT_HEAD_EX(CType)  c_value; } CWChar;
/* clang-format on */

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


INTDEF WUNUSED DREF CObject *DCALL CChar_New(char val);
INTDEF WUNUSED DREF CObject *DCALL CWChar_New(Dee_wchar_t val);
INTDEF WUNUSED DREF CObject *DCALL CChar16_New(__CHAR16_TYPE__ val);
INTDEF WUNUSED DREF CObject *DCALL CChar32_New(__CHAR16_TYPE__ val);
INTDEF WUNUSED DREF CObject *DCALL CBool_New(bool value);
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
#else /* ... */
#define CONFIG_SUCHAR_NEEDS_OWN_TYPE
INTDEF CType CSChar_Type;
INTDEF CType CUChar_Type;
INTDEF WUNUSED DREF CObject *DCALL CSChar_New(HOST_INTFOR(CTYPES_sizeof_char) val);
INTDEF WUNUSED DREF CObject *DCALL CUChar_New(HOST_UINTFOR(CTYPES_sizeof_char) val);
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
#else /* ... */
#define CONFIG_SHORT_NEEDS_OWN_TYPE
INTDEF CType CShort_Type;
INTDEF CType CUShort_Type;
INTDEF WUNUSED DREF CObject *DCALL CShort_New(HOST_INTFOR(CTYPES_sizeof_short) val);
INTDEF WUNUSED DREF CObject *DCALL CUShort_New(HOST_UINTFOR(CTYPES_sizeof_short) val);
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
#else /* ... */
#define CONFIG_INT_NEEDS_OWN_TYPE
INTDEF CType CInt_Type;
INTDEF CType CUInt_Type;
INTDEF WUNUSED DREF CObject *DCALL CInt_New(HOST_INTFOR(CTYPES_sizeof_int) val);
INTDEF WUNUSED DREF CObject *DCALL CUInt_New(HOST_UINTFOR(CTYPES_sizeof_int) val);
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
#else /* ... */
#define CONFIG_LONG_NEEDS_OWN_TYPE
INTDEF CType CLong_Type;
INTDEF CType CULong_Type;
INTDEF WUNUSED DREF CObject *DCALL CLong_New(HOST_INTFOR(CTYPES_sizeof_long) val);
INTDEF WUNUSED DREF CObject *DCALL CULong_New(HOST_UINTFOR(CTYPES_sizeof_long) val);
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
#else /* ... */
#define CONFIG_LLONG_NEEDS_OWN_TYPE
INTDEF CType CLLong_Type;
INTDEF CType CULLong_Type;
INTDEF WUNUSED DREF CObject *DCALL CLLong_New(HOST_INTFOR(CTYPES_sizeof_llong) val);
INTDEF WUNUSED DREF CObject *DCALL CULLong_New(HOST_UINTFOR(CTYPES_sizeof_llong) val);
#endif /* !... */

#if (!defined(CONFIG_LONG_NEEDS_OWN_TYPE) &&                   \
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
INTDEF WUNUSED DREF CObject *DCALL CLong_New(HOST_INTFOR(CTYPES_sizeof_long) val);
INTDEF WUNUSED DREF CObject *DCALL CULong_New(HOST_UINTFOR(CTYPES_sizeof_long) val);
#endif /* ... */






/* Backwards compat... */
#define DeeSTypeObject               CType
#define DeeSType_AsType              CType_AsType
#define DeeStruct_Data               CObject_Data
#define st_sizeof                    ct_sizeof
#define DeeLValue_Check              Object_IsCLValue
#define DeeType_AsLValueType         Type_AsCLValueType
#define lt_orig                      clt_orig
#define lvalue_object                clvalue_object
#define l_ptr                        cl_value
#define DeeStructObject              CObject
#define DeeCVoid_Type                CVoid_Type
#define pointer_object               cpointer_object
#define p_ptr                        cp_value
#define DeeSType_GetTypeOf           CType_TypeOf
#define DeeLValueType_Check          Object_IsCLValueType
#define DeeSType_AsLValueType        CType_AsCLValueType
#define DeeSType_Sizeof              CType_Sizeof
#define DeeSType_Alignof             CType_Alignof
#define DeeSType_AsObject            CType_AsObject
#define DeeCInt8_Type                CInt8_Type
#define DeeCInt16_Type               CInt16_Type
#define DeeCInt32_Type               CInt32_Type
#define DeeCInt64_Type               CInt64_Type
#define DeeCInt128_Type              CInt128_Type
#define DeeCUInt8_Type               CUInt8_Type
#define DeeCUInt16_Type              CUInt16_Type
#define DeeCUInt32_Type              CUInt32_Type
#define DeeCUInt64_Type              CUInt64_Type
#define DeeCUInt128_Type             CUInt128_Type
#define DeeCSChar_Type               CSChar_Type
#define DeeCUChar_Type               CUChar_Type
#define DeeCShort_Type               CShort_Type
#define DeeCUShort_Type              CUShort_Type
#define DeeCInt_Type                 CInt_Type
#define DeeCUInt_Type                CUInt_Type
#define DeeCLong_Type                CLong_Type
#define DeeCULong_Type               CULong_Type
#define DeeCLLong_Type               CLLong_Type
#define DeeCULLong_Type              CULLong_Type
#define DeeCChar_Type                CChar_Type
#define DeeCWChar_Type               CWChar_Type
#define DeeCChar16_Type              CChar16_Type
#define DeeCChar32_Type              CChar32_Type
#define DeeCBool_Type                CBool_Type
#define DeeStructTypeObject          CStructType
#define DeeStructType_Type           CStructType_Type
#define int_newint(v)                Dee_AsObject(CIntN_New(CTYPES_sizeof_int)(v))
#define int_news8(v)                 Dee_AsObject(CInt8_New(v))
#define int_news16(v)                Dee_AsObject(CInt16_New(v))
#define int_news32(v)                Dee_AsObject(CInt32_New(v))
#define int_news64(v)                Dee_AsObject(CInt64_New(v))
#define int_news128(v)               Dee_AsObject(CInt128_New(v))
#define int_newu8(v)                 Dee_AsObject(CUInt8_New(v))
#define int_newu16(v)                Dee_AsObject(CUInt16_New(v))
#define int_newu32(v)                Dee_AsObject(CUInt32_New(v))
#define int_newu64(v)                Dee_AsObject(CUInt64_New(v))
#define int_newu128(v)               Dee_AsObject(CUInt128_New(v))
#define st_array                     ct_arrays
#define st_cfunction                 ct_functions
#define DeeCFloat_Type               CFloat_Type
#define DeeCDouble_Type              CDouble_Type
#define DeeCLDouble_Type             CLDouble_Type
#define DeeStructured_Type           AbstractCObject_Type
#define DeePointer_Type              AbstractCPointer_Type
#define DeeLValue_Type               AbstractCLValue_Type
#define DeeStruct_Type               AbstractCStruct_Type
#define DeeArray_Type                AbstractCArray_Type
#define DeeCFunction_Type            AbstractCFunction_Type
#define DeePointerType_AsSType       CPointerType_AsCType
#define DeeLValueType_AsSType        CLValueType_AsCType
#define DeeArrayType_AsSType         CArrayType_AsCType
#define DeeCFunctionType_AsSType     CFunctionType_AsCType
#define DeeSType_Type                CType_Type
#define DeePointerType_Type          CPointerType_Type
#define DeeLValueType_Type           CLValueType_Type
#define DeeArrayType_Type            CArrayType_Type
#define DeeCFunctionType_Type        CFunctionType_Type
#define DeePointerType_AsObject      CPointerType_AsObject
#define DeeLValueType_AsObject       CLValueType_AsObject
#define DeeArrayType_AsObject        CArrayType_AsObject
#define DeeStructType_AsObject       CStructType_AsObject
#define DeeCFunctionType_AsObject    CFunctionType_AsObject
#define CINT_SIZED                   CIntN_Type
#define CUINT_SIZED                  CUIntN_Type
#define st_base                      ct_base
#define DeePointer_NewChar(p)        Dee_AsObject(CPointer_NewChar(p))
#define DeePointer_NewVoid(p)        Dee_AsObject(CPointer_NewVoid(p))
#define DeeCFunctionTypeObject       CFunctionType
#define DeeSType_CacheLockReading    CType_CacheLockReading
#define DeeSType_CacheLockWriting    CType_CacheLockWriting
#define DeeSType_CacheLockTryRead    CType_CacheLockTryRead
#define DeeSType_CacheLockTryWrite   CType_CacheLockTryWrite
#define DeeSType_CacheLockCanRead    CType_CacheLockCanRead
#define DeeSType_CacheLockCanWrite   CType_CacheLockCanWrite
#define DeeSType_CacheLockWaitRead   CType_CacheLockWaitRead
#define DeeSType_CacheLockWaitWrite  CType_CacheLockWaitWrite
#define DeeSType_CacheLockRead       CType_CacheLockRead
#define DeeSType_CacheLockWrite      CType_CacheLockWrite
#define DeeSType_CacheLockTryUpgrade CType_CacheLockTryUpgrade
#define DeeSType_CacheLockUpgrade    CType_CacheLockUpgrade
#define DeeSType_CacheLockDowngrade  CType_CacheLockDowngrade
#define DeeSType_CacheLockEndWrite   CType_CacheLockEndWrite
#define DeeSType_CacheLockEndRead    CType_CacheLockEndRead
#define DeeSType_CacheLockEnd        CType_CacheLockEnd
#define DeeCFunctionType_AsType      CFunctionType_AsType
#define DeeType_AsCFunctionType      Type_AsCFunctionType
#define DeePointerType_Check         Object_IsCPointerType
#define DeeType_AsSType              Type_AsCType
#define DeePointerTypeObject         CPointerType
#define DeeLValueTypeObject          CLValueType
#define st_pointer                   ct_pointer
#define st_lvalue                    ct_lvalue
#define DeePointerType_AsType        CPointerType_AsType
#define DeeLValueType_AsType         CLValueType_AsType
#define pt_orig                      cpt_orig
#define pt_base                      cpt_base
#define pt_size                      cpt_size
#define st_align                     ct_alignof
#define DeeType_AsPointerType        Type_AsCPointerType
#define lt_base                      clt_base
#ifndef CONFIG_NO_CFUNCTION
#define st_ffitype              ct_ffitype
#define ft_base                 cft_base
#define ft_orig                 cft_return
#define ft_chain                cft_chain
#define ft_hash                 cft_hash
#define ft_argc                 cft_argc
#define ft_argv                 cft_argv
#define ft_cc                   cft_cc
#define ft_ffi_return_type      cft_ffi_return_type
#define ft_ffi_arg_type_v       cft_ffi_arg_type_v
#define ft_ffi_cif              cft_ffi_cif
#define ft_wsize                cft_wsize
#define ft_woff_argmem          cft_woff_argmem
#define ft_woff_argptr          cft_woff_argptr
#define ft_woff_variadic_argmem cft_woff_variadic_argmem
#define stype_ffitype           CType_GetFFIType
#endif /* !CONFIG_NO_CFUNCTION */
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
typedef struct stype_object DeeSTypeObject;
typedef struct pointer_type_object DeePointerTypeObject;
typedef struct lvalue_type_object DeeLValueTypeObject;
typedef struct array_type_object DeeArrayTypeObject;
typedef struct cfunction_type_object DeeCFunctionTypeObject;

struct stype_cast {
	/* Structured casting operators. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *st_str)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *st_repr)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1)) int             (DCALL *st_bool)(DeeSTypeObject *tp_self, void *self);
};
struct stype_math {
	/* Structured math operators. */
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_int32)(DeeSTypeObject *tp_self, void *self, int32_t *result);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_int64)(DeeSTypeObject *tp_self, void *self, int64_t *result);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_double)(DeeSTypeObject *tp_self, void *self, double *result);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *st_int)(DeeSTypeObject *tp_self, void *self); /* Cast to `int' */
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *st_inv)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *st_pos)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *st_neg)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_add)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_sub)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_mul)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_div)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_mod)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_shl)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_shr)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_and)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_or)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_xor)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_pow)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1))    int             (DCALL *st_inc)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1))    int             (DCALL *st_dec)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_add)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_sub)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_mul)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_div)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_mod)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_shl)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_shr)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_and)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_or)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_xor)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *st_inplace_pow)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
};

struct stype_cmp {
	/* Structured compare operators. */
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_eq)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_ne)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_lo)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_le)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_gr)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *st_ge)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
};

struct stype_seq {
	/* Structured sequence operators. */
	WUNUSED_T NONNULL_T((1))          DREF DeeObject *(DCALL *st_iter_self)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1))          DREF DeeObject *(DCALL *st_size)(DeeSTypeObject *tp_self, void *self);
	WUNUSED_T NONNULL_T((1, 3))       DREF DeeObject *(DCALL *st_contains)(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 3))       DREF DeeObject *(DCALL *st_get)(DeeSTypeObject *tp_self, void *self, DeeObject *index);
	WUNUSED_T NONNULL_T((1, 3))       int             (DCALL *st_del)(DeeSTypeObject *tp_self, void *self, DeeObject *index);
	WUNUSED_T NONNULL_T((1, 3, 4))    int             (DCALL *st_set)(DeeSTypeObject *tp_self, void *self, DeeObject *index, DeeObject *value);
	WUNUSED_T NONNULL_T((1, 3, 4))    DREF DeeObject *(DCALL *st_range_get)(DeeSTypeObject *tp_self, void *self, DeeObject *begin, DeeObject *end);
	WUNUSED_T NONNULL_T((1, 3, 4))    int             (DCALL *st_range_del)(DeeSTypeObject *tp_self, void *self, DeeObject *begin, DeeObject *end);
	WUNUSED_T NONNULL_T((1, 3, 4, 5)) int             (DCALL *st_range_set)(DeeSTypeObject *tp_self, void *self, DeeObject *begin, DeeObject *end, DeeObject *value);
};

struct stype_attr {
	/* Structured attribute operators. */

	/* Get a struct attribute (excluding generic attributes)
	 * @return: * :        Attribute value
	 * @return: NULL:      Error
	 * @return: ITER_DONE: No such attribute */
	WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *
	(DCALL *st_getattr)(DeeSTypeObject *tp_self, void *self, /*String*/ DeeObject *name);

	/* Delete a struct attribute (excluding generic attributes)
	 * @return: 0 : Success
	 * @return: -1: Error
	 * @return: -2: No such attribute */
	WUNUSED_T NONNULL_T((1, 3)) int
	(DCALL *st_delattr)(DeeSTypeObject *tp_self, void *self, /*String*/ DeeObject *name);

	/* Set a struct attribute (excluding generic attributes)
	 * @return: 0 : Success
	 * @return: -1: Error
	 * @return: -2: No such attribute */
	WUNUSED_T NONNULL_T((1, 3, 4)) int
	(DCALL *st_setattr)(DeeSTypeObject *tp_self, void *self,
	                    /*String*/ DeeObject *name, DeeObject *value);

	/* Enumerate struct attributes (excluding generic attributes) */
	WUNUSED_T NONNULL_T((1, 4)) size_t
	(DCALL *st_iterattr)(DeeSTypeObject *__restrict tp_self,
	                     struct Dee_attriter *iterbuf, size_t bufsize,
	                     struct Dee_attrhint const *__restrict hint);
};


LIST_HEAD(array_type_list, array_type_object);
struct stype_array {
	size_t                  sa_size; /* Amount of cached array types. */
	Dee_hash_t              sa_mask; /* Allocated map mask. */
	struct array_type_list *sa_list; /* [0..1][0..sa_mask+1][owned] Hash-map of array types.
	                                  * As hash for indexing this map, use `at_count'. */
};
#define STYPE_ARRAY_INIT { 0, 0, NULL }

#ifndef CONFIG_NO_CFUNCTION
LIST_HEAD(cfunction_type_list, cfunction_type_object);
struct stype_cfunction {
	size_t                      sf_size; /* Amount of cached function types. */
	Dee_hash_t                  sf_mask; /* Allocated map mask. */
	struct cfunction_type_list *sf_list; /* [0..1][0..sf_mask+1][owned] Hash-map of array types.
	                                      * As hash for indexing this map, use `ft_hash'. */
};
#define STYPE_CFUNCTION_INIT { 0, 0, NULL }
#endif /* !CONFIG_NO_CFUNCTION */



struct stype_object {
	DeeTypeObject           st_base;      /* The underlying type object. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t     st_cachelock; /* Lock for cached derived types. */
#endif /* !CONFIG_NO_THREADS */
	DeePointerTypeObject   *st_pointer;   /* [0..1][lock(st_cachelock)] A weak pointer to the pointer-type of this structured type. */
	DeeLValueTypeObject    *st_lvalue;    /* [0..1][lock(st_cachelock)] A weak pointer to the lvalue-type of this structured type. */
	struct stype_array      st_array;     /* [lock(st_cachelock)] Derived array sub-types. */
#ifndef CONFIG_NO_CFUNCTION
	struct stype_cfunction  st_cfunction; /* [lock(st_cachelock)] Derived function sub-types. */
	ffi_type               *st_ffitype;   /* [0..1][lock(WRITE_ONCE)] The type used by FFI to represent this type. */
#endif /* !CONFIG_NO_CFUNCTION */
	size_t                  st_sizeof;    /* [const] # of bytes required by instances of this type. */
	size_t                  st_align;     /* [const] Alignment required by this type. */
	WUNUSED_T NONNULL_T((1))
	int             (DCALL *st_init)(DeeSTypeObject *tp_self, void *self,
	                                 size_t argc, DeeObject *const *argv);
	WUNUSED_T NONNULL_T((1, 3))
	int             (DCALL *st_assign)(DeeSTypeObject *tp_self, void *self, DeeObject *value);
	struct stype_cast       st_cast;      /* Type casting operators. */
	WUNUSED_T NONNULL_T((1))
	DREF DeeObject *(DCALL *st_call)(DeeSTypeObject *tp_self, void *self,
	                                 size_t argc, DeeObject *const *argv);
	struct stype_math       *st_math;      /* [0..1] Math related operators. */
	struct stype_cmp        *st_cmp;       /* [0..1] Compare operators. */
	struct stype_seq        *st_seq;       /* [0..1] Sequence operators. */
	struct stype_attr const *st_attr;      /* [0..1] Attribute access operators. */
};

#define DeeSType_CacheLockReading(self)    Dee_atomic_rwlock_reading(&(self)->st_cachelock)
#define DeeSType_CacheLockWriting(self)    Dee_atomic_rwlock_writing(&(self)->st_cachelock)
#define DeeSType_CacheLockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->st_cachelock)
#define DeeSType_CacheLockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->st_cachelock)
#define DeeSType_CacheLockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->st_cachelock)
#define DeeSType_CacheLockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->st_cachelock)
#define DeeSType_CacheLockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->st_cachelock)
#define DeeSType_CacheLockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->st_cachelock)
#define DeeSType_CacheLockRead(self)       Dee_atomic_rwlock_read(&(self)->st_cachelock)
#define DeeSType_CacheLockWrite(self)      Dee_atomic_rwlock_write(&(self)->st_cachelock)
#define DeeSType_CacheLockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->st_cachelock)
#define DeeSType_CacheLockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->st_cachelock)
#define DeeSType_CacheLockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->st_cachelock)
#define DeeSType_CacheLockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->st_cachelock)
#define DeeSType_CacheLockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->st_cachelock)
#define DeeSType_CacheLockEnd(self)        Dee_atomic_rwlock_end(&(self)->st_cachelock)

struct pointer_type_object {
	DeeSTypeObject          pt_base;      /* The underlying type object. */
	DREF DeeSTypeObject    *pt_orig;      /* [1..1][const] The dereferenced type of a pointer. */
	size_t                  pt_size;      /* [== DeeSType_Sizeof(pt_orig)] The size of the pointed-to type. */
};
struct lvalue_type_object {
	DeeSTypeObject          lt_base;      /* The underlying type object. */
	DREF DeeSTypeObject    *lt_orig;      /* [1..1][const] The dereferenced type of an l-value. */
};

/* Codes for new operators available to types implementing "DeeSType_Type" */
#define STYPE_OPERATOR_INIT        OPERATOR_EXTENDED(0x0000)
#define STYPE_OPERATOR_ASSIGN      OPERATOR_EXTENDED(0x0001)
#define STYPE_OPERATOR_STR         OPERATOR_EXTENDED(0x0002)
#define STYPE_OPERATOR_REPR        OPERATOR_EXTENDED(0x0003)
#define STYPE_OPERATOR_BOOL        OPERATOR_EXTENDED(0x0004)
#define STYPE_OPERATOR_CALL        OPERATOR_EXTENDED(0x0005)
#define STYPE_OPERATOR_INT         OPERATOR_EXTENDED(0x0006)
#define STYPE_OPERATOR_DOUBLE      OPERATOR_EXTENDED(0x0007)
#define STYPE_OPERATOR_INV         OPERATOR_EXTENDED(0x0008)
#define STYPE_OPERATOR_POS         OPERATOR_EXTENDED(0x0009)
#define STYPE_OPERATOR_NEG         OPERATOR_EXTENDED(0x000a)
#define STYPE_OPERATOR_ADD         OPERATOR_EXTENDED(0x000b)
#define STYPE_OPERATOR_SUB         OPERATOR_EXTENDED(0x000c)
#define STYPE_OPERATOR_MUL         OPERATOR_EXTENDED(0x000d)
#define STYPE_OPERATOR_DIV         OPERATOR_EXTENDED(0x000e)
#define STYPE_OPERATOR_MOD         OPERATOR_EXTENDED(0x000f)
#define STYPE_OPERATOR_SHL         OPERATOR_EXTENDED(0x0010)
#define STYPE_OPERATOR_SHR         OPERATOR_EXTENDED(0x0011)
#define STYPE_OPERATOR_AND         OPERATOR_EXTENDED(0x0012)
#define STYPE_OPERATOR_OR          OPERATOR_EXTENDED(0x0013)
#define STYPE_OPERATOR_XOR         OPERATOR_EXTENDED(0x0014)
#define STYPE_OPERATOR_POW         OPERATOR_EXTENDED(0x0015)
#define STYPE_OPERATOR_INC         OPERATOR_EXTENDED(0x0016)
#define STYPE_OPERATOR_DEC         OPERATOR_EXTENDED(0x0017)
#define STYPE_OPERATOR_INPLACE_ADD OPERATOR_EXTENDED(0x0018)
#define STYPE_OPERATOR_INPLACE_SUB OPERATOR_EXTENDED(0x0019)
#define STYPE_OPERATOR_INPLACE_MUL OPERATOR_EXTENDED(0x001a)
#define STYPE_OPERATOR_INPLACE_DIV OPERATOR_EXTENDED(0x001b)
#define STYPE_OPERATOR_INPLACE_MOD OPERATOR_EXTENDED(0x001c)
#define STYPE_OPERATOR_INPLACE_SHL OPERATOR_EXTENDED(0x001d)
#define STYPE_OPERATOR_INPLACE_SHR OPERATOR_EXTENDED(0x001e)
#define STYPE_OPERATOR_INPLACE_AND OPERATOR_EXTENDED(0x001f)
#define STYPE_OPERATOR_INPLACE_OR  OPERATOR_EXTENDED(0x0020)
#define STYPE_OPERATOR_INPLACE_XOR OPERATOR_EXTENDED(0x0021)
#define STYPE_OPERATOR_INPLACE_POW OPERATOR_EXTENDED(0x0022)
#define STYPE_OPERATOR_EQ          OPERATOR_EXTENDED(0x0023)
#define STYPE_OPERATOR_NE          OPERATOR_EXTENDED(0x0024)
#define STYPE_OPERATOR_LO          OPERATOR_EXTENDED(0x0025)
#define STYPE_OPERATOR_LE          OPERATOR_EXTENDED(0x0026)
#define STYPE_OPERATOR_GR          OPERATOR_EXTENDED(0x0027)
#define STYPE_OPERATOR_GE          OPERATOR_EXTENDED(0x0028)
#define STYPE_OPERATOR_ITER        OPERATOR_EXTENDED(0x0029)
#define STYPE_OPERATOR_SIZE        OPERATOR_EXTENDED(0x002a)
#define STYPE_OPERATOR_CONTAINS    OPERATOR_EXTENDED(0x002b)
#define STYPE_OPERATOR_GETITEM     OPERATOR_EXTENDED(0x002c)
#define STYPE_OPERATOR_DELITEM     OPERATOR_EXTENDED(0x002d)
#define STYPE_OPERATOR_SETITEM     OPERATOR_EXTENDED(0x002e)
#define STYPE_OPERATOR_GETRANGE    OPERATOR_EXTENDED(0x002f)
#define STYPE_OPERATOR_DELRANGE    OPERATOR_EXTENDED(0x0030)
#define STYPE_OPERATOR_SETRANGE    OPERATOR_EXTENDED(0x0031)
#define STYPE_OPERATOR_GETATTR     OPERATOR_EXTENDED(0x0032)
#define STYPE_OPERATOR_DELATTR     OPERATOR_EXTENDED(0x0033)
#define STYPE_OPERATOR_SETATTR     OPERATOR_EXTENDED(0x0034)
#define STYPE_OPERATOR_ENUMATTR    OPERATOR_EXTENDED(0x0035)

#define OPERATOR_STYPE_0000_INIT        STYPE_OPERATOR_INIT
#define OPERATOR_STYPE_0001_ASSIGN      STYPE_OPERATOR_ASSIGN
#define OPERATOR_STYPE_0002_STR         STYPE_OPERATOR_STR
#define OPERATOR_STYPE_0003_REPR        STYPE_OPERATOR_REPR
#define OPERATOR_STYPE_0004_BOOL        STYPE_OPERATOR_BOOL
#define OPERATOR_STYPE_0005_CALL        STYPE_OPERATOR_CALL
#define OPERATOR_STYPE_0006_INT         STYPE_OPERATOR_INT
#define OPERATOR_STYPE_0007_DOUBLE      STYPE_OPERATOR_DOUBLE
#define OPERATOR_STYPE_0008_INV         STYPE_OPERATOR_INV
#define OPERATOR_STYPE_0009_POS         STYPE_OPERATOR_POS
#define OPERATOR_STYPE_000A_NEG         STYPE_OPERATOR_NEG
#define OPERATOR_STYPE_000B_ADD         STYPE_OPERATOR_ADD
#define OPERATOR_STYPE_000C_SUB         STYPE_OPERATOR_SUB
#define OPERATOR_STYPE_000D_MUL         STYPE_OPERATOR_MUL
#define OPERATOR_STYPE_000E_DIV         STYPE_OPERATOR_DIV
#define OPERATOR_STYPE_000F_MOD         STYPE_OPERATOR_MOD
#define OPERATOR_STYPE_0010_SHL         STYPE_OPERATOR_SHL
#define OPERATOR_STYPE_0011_SHR         STYPE_OPERATOR_SHR
#define OPERATOR_STYPE_0012_AND         STYPE_OPERATOR_AND
#define OPERATOR_STYPE_0013_OR          STYPE_OPERATOR_OR
#define OPERATOR_STYPE_0014_XOR         STYPE_OPERATOR_XOR
#define OPERATOR_STYPE_0015_POW         STYPE_OPERATOR_POW
#define OPERATOR_STYPE_0016_INC         STYPE_OPERATOR_INC
#define OPERATOR_STYPE_0017_DEC         STYPE_OPERATOR_DEC
#define OPERATOR_STYPE_0018_INPLACE_ADD STYPE_OPERATOR_INPLACE_ADD
#define OPERATOR_STYPE_0019_INPLACE_SUB STYPE_OPERATOR_INPLACE_SUB
#define OPERATOR_STYPE_001A_INPLACE_MUL STYPE_OPERATOR_INPLACE_MUL
#define OPERATOR_STYPE_001B_INPLACE_DIV STYPE_OPERATOR_INPLACE_DIV
#define OPERATOR_STYPE_001C_INPLACE_MOD STYPE_OPERATOR_INPLACE_MOD
#define OPERATOR_STYPE_001D_INPLACE_SHL STYPE_OPERATOR_INPLACE_SHL
#define OPERATOR_STYPE_001E_INPLACE_SHR STYPE_OPERATOR_INPLACE_SHR
#define OPERATOR_STYPE_001F_INPLACE_AND STYPE_OPERATOR_INPLACE_AND
#define OPERATOR_STYPE_0020_INPLACE_OR  STYPE_OPERATOR_INPLACE_OR
#define OPERATOR_STYPE_0021_INPLACE_XOR STYPE_OPERATOR_INPLACE_XOR
#define OPERATOR_STYPE_0022_INPLACE_POW STYPE_OPERATOR_INPLACE_POW
#define OPERATOR_STYPE_0023_EQ          STYPE_OPERATOR_EQ
#define OPERATOR_STYPE_0024_NE          STYPE_OPERATOR_NE
#define OPERATOR_STYPE_0025_LO          STYPE_OPERATOR_LO
#define OPERATOR_STYPE_0026_LE          STYPE_OPERATOR_LE
#define OPERATOR_STYPE_0027_GR          STYPE_OPERATOR_GR
#define OPERATOR_STYPE_0028_GE          STYPE_OPERATOR_GE
#define OPERATOR_STYPE_0029_ITER        STYPE_OPERATOR_ITER
#define OPERATOR_STYPE_002A_SIZE        STYPE_OPERATOR_SIZE
#define OPERATOR_STYPE_002B_CONTAINS    STYPE_OPERATOR_CONTAINS
#define OPERATOR_STYPE_002C_GETITEM     STYPE_OPERATOR_GETITEM
#define OPERATOR_STYPE_002D_DELITEM     STYPE_OPERATOR_DELITEM
#define OPERATOR_STYPE_002E_SETITEM     STYPE_OPERATOR_SETITEM
#define OPERATOR_STYPE_002F_GETRANGE    STYPE_OPERATOR_GETRANGE
#define OPERATOR_STYPE_0030_DELRANGE    STYPE_OPERATOR_DELRANGE
#define OPERATOR_STYPE_0031_SETRANGE    STYPE_OPERATOR_SETRANGE
#define OPERATOR_STYPE_0032_GETATTR     STYPE_OPERATOR_GETATTR
#define OPERATOR_STYPE_0033_DELATTR     STYPE_OPERATOR_DELATTR
#define OPERATOR_STYPE_0034_SETATTR     STYPE_OPERATOR_SETATTR
#define OPERATOR_STYPE_0035_ENUMATTR    STYPE_OPERATOR_ENUMATTR

#define OPERATOR_STYPE_MIN OPERATOR_STYPE_0000_INIT
#define OPERATOR_STYPE_MAX OPERATOR_STYPE_0035_ENUMATTR


/* The type for all structured types (aka. `DeeSTypeObject' objects)
 * (such as C-integer types, or C-style struct/union-declarations). */
INTDEF DeeTypeObject DeeSType_Type;
#define DeeSType_Check(ob)         DeeObject_InstanceOf((DeeObject *)(ob), &DeeSType_Type)
#define DeeSType_AsType(x)         (&(x)->st_base)
#define DeeSType_Sizeof(x)         ((DeeSTypeObject *)(x))->st_sizeof
#define DeeSType_Alignof(x)        ((DeeSTypeObject *)(x))->st_align
#define DeeSType_Base(x)           DeeType_AsSType(DeeType_Base(DeeSType_AsType(x)))
#define DeeSType_AsObject(x)       Dee_AsObject(&(x)->st_base)
#define DeeType_AsSType(x)         COMPILER_CONTAINER_OF(x, DeeSTypeObject, st_base)
#define DeeType_AsStructType(x)    COMPILER_CONTAINER_OF(x, DeeStructTypeObject, st_base.st_base)
#define DeeType_AsCFunctionType(x) COMPILER_CONTAINER_OF(x, DeeCFunctionTypeObject, ft_base.st_base)
#define DeeType_AsArrayType(x)     COMPILER_CONTAINER_OF(x, DeeArrayTypeObject, at_base.st_base)
#define DeeType_AsPointerType(x)   COMPILER_CONTAINER_OF(x, DeePointerTypeObject, pt_base.st_base)
#define DeeType_AsLValueType(x)    COMPILER_CONTAINER_OF(x, DeeLValueTypeObject, lt_base.st_base)
#define DeeSType_AsPointerType(x)  COMPILER_CONTAINER_OF(x, DeePointerTypeObject, pt_base)
#define DeeSType_AsLValueType(x)   COMPILER_CONTAINER_OF(x, DeeLValueTypeObject, lt_base)
#define DeePointerType_AsObject(x) DeeSType_AsObject(DeePointerType_AsSType(x))
#define DeePointerType_AsType(x)   (&(x)->pt_base.st_base)
#define DeePointerType_AsSType(x)  (&(x)->pt_base)
#define DeeLValueType_AsObject(x)  DeeSType_AsObject(DeeLValueType_AsSType(x))
#define DeeLValueType_AsType(x)    (&(x)->lt_base.st_base)
#define DeeLValueType_AsSType(x)   (&(x)->lt_base)

/* The types for all pointer/l-value types (aka. `DeePointerTypeObject' / `DeeLValueTypeObject' objects)
 * NOTE: These types are derived from `DeeSType_Type' */
INTDEF DeeTypeObject DeePointerType_Type;
INTDEF DeeTypeObject DeeLValueType_Type;
#define DeePointerType_Check(ob) DeeObject_InstanceOfExact((DeeObject *)(ob), &DeePointerType_Type) /* `pointer_type' is final. */
#define DeeLValueType_Check(ob)  DeeObject_InstanceOfExact((DeeObject *)(ob), &DeeLValueType_Type)  /* `lvalue_type' is final. */

/* Return (and create if missing) the pointer/l-value
 * type associated with a given structured type. */
INTDEF WUNUSED NONNULL((1)) DREF DeePointerTypeObject *DCALL DeeSType_Pointer(DeeSTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeLValueTypeObject *DCALL DeeSType_LValue(DeeSTypeObject *__restrict self);

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
INTDEF WUNUSED NONNULL((1)) DeeSTypeObject *DCALL
DeeSType_Get(DeeObject *__restrict self);

/* Same as `DeeSType_Get()', but also able to handle the
 * case where "self" is an *instance*, rather a some type. */
INTDEF WUNUSED NONNULL((1)) DeeSTypeObject *DCALL
DeeSType_GetTypeOf(DeeObject *__restrict self);

typedef struct {
	Dee_OBJECT_HEAD
	COMPILER_FLEXIBLE_ARRAY(__BYTE_TYPE__, so_data);
} DeeStructObject;

#define DeeStructObject_Malloc(sizeof_data) \
	((DREF DeeStructObject *)DeeObject_Malloc(sizeof(DeeObject) + (sizeof_data)))
#define DeeStructObject_Free(ptr) DeeObject_Free(ptr)

/* Check of `ob' is a structured object. */
#define DeeStruct_Check(ob)  DeeSType_Check(Dee_TYPE(ob))
/* Return a pointer to the structured data-block of `ob' */
#define DeeStruct_Data(ob)   (void *)((__BYTE_TYPE__ *)(ob) + sizeof(DeeObject))
/* Return the size of the structured data-block of `ob' */
#define DeeStruct_Size(ob)   DeeSType_Sizeof(Dee_TYPE(ob))
#define DeeStruct_Align(ob)  DeeSType_Alignof(Dee_TYPE(ob))


/* The base class of all structured objects.
 * This is the one that implements the wrapper operators for all the
 * regular operators being forwarded to their structured counterparts. */
INTDEF DeeSTypeObject DeeStructured_Type;


struct pointer_object {
	OBJECT_HEAD
	union pointer p_ptr;
};

struct lvalue_object {
	OBJECT_HEAD
	union pointer l_ptr;
};

#define pointer_object_malloc() DeeObject_MALLOC(struct pointer_object)
#define pointer_object_free(p)  DeeObject_FREE(Dee_REQUIRES_TYPE(struct pointer_object *, p))
#define lvalue_object_malloc()  DeeObject_MALLOC(struct lvalue_object)
#define lvalue_object_free(p)   DeeObject_FREE(Dee_REQUIRES_TYPE(struct lvalue_object *, p))

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define PTR_pointer_tp_alloc DeeSlab_GetMalloc(sizeof(struct pointer_object), (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(struct pointer_object))
#define PTR_pointer_tp_free  DeeSlab_GetFree(sizeof(struct pointer_object), NULL)
#define PTR_lvalue_tp_free   DeeSlab_GetFree(sizeof(struct lvalue_object), NULL)
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#define PTR_pointer_tp_alloc (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(struct pointer_object)
#define PTR_pointer_tp_free  NULL
#define PTR_lvalue_tp_free   NULL
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */



/* Derived from `DeeStructured_Type', these types are the base
 * classes of all pointer/lvalue types that are be generated
 * on-the-fly for structured types.
 * WARNING: The copy-constructor of `DeeLValue_Type' does not return
 *          another l-value object, but rather a copy of the pointed-to object. */
INTDEF DeePointerTypeObject DeePointer_Type;
INTDEF DeeLValueTypeObject DeeLValue_Type;

#if 1 /* Both would work, but this one returns in O(1) */
#define DeePointer_Check(ob) DeePointerType_Check(Dee_TYPE(ob))
#define DeeLValue_Check(ob)  DeeLValueType_Check(Dee_TYPE(ob))
#else
#define DeePointer_Check(ob) DeeObject_InstanceOf(ob, DeePointerType_AsType(&DeePointer_Type))
#define DeeLValue_Check(ob)  DeeObject_InstanceOf(ob, DeeLValueType_AsType(&DeeLValue_Type))
#endif

/* Interpret `self' as a pointer and store the result in `*result'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPointer(DeeObject *self,
                    DeeSTypeObject *pointer_base,
                    union pointer *__restrict result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPPointer(DeeObject *self,
                     DeeSTypeObject *pointer_base_base,
                     void ***p_result);

/* Same as `DeeObject_AsPointer()', but only ~try~ to interpret it.
 * @return:  1: The conversion failed.
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsPointer(DeeObject *self,
                       DeeSTypeObject *pointer_base,
                       union pointer *__restrict result);

/* Similar to `DeeObject_TryAsPointer()', but fills in `*p_pointer_base' with the
 * pointer-base type. For use with type-generic functions (such as the `atomic_*' api)
 * @return:  1: The conversion failed.
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsGenericPointer(DeeObject *self,
                              DeeSTypeObject **__restrict p_pointer_base,
                              union pointer *__restrict result);

/* S.a. `DeeObject_TryAsGenericPointer()'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsGenericPointer(DeeObject *self,
                           DeeSTypeObject **__restrict p_pointer_base,
                           union pointer *__restrict result);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeePointer_New(DeePointerTypeObject *pointer_type, void *pointer_value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeePointer_NewInherited(/*inherit(always)*/ DREF DeePointerTypeObject *pointer_type, void *pointer_value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeePointer_NewFor(DeeSTypeObject *pointer_base_type, void *pointer_value);
#define DeePointer_NewVoid(pointer_value) DeePointer_NewFor(&DeeCVoid_Type, pointer_value)
#define DeePointer_NewChar(pointer_value) DeePointer_NewFor(&DeeCChar_Type, pointer_value)

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeLValue_NewInherited(/*inherit(always)*/ DREF DeeLValueTypeObject *lvalue_type, void *pointer_value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeLValue_NewFor(DeeSTypeObject *lvalue_base_type, void *pointer_value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeLValue_NewForInherited(/*inherit(always)*/ DREF DeeSTypeObject *lvalue_base_type, void *pointer_value);


/* The main functions for the new `ref' (`&self') and `ind' (`*self')
 * operators made available through the ctypes module. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Ref(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Deref(DeeObject *__restrict self);


INTDEF struct stype_math pointer_math1; /* Math functions for pointer types with a base-size of `0' or `1' */
INTDEF struct stype_math pointer_mathn; /* Math functions for pointer types with a base-size of anything else. */
INTDEF struct stype_seq pointer_seq1;   /* Sequence functions for pointer types with a base-size of `0' or `1' */
INTDEF struct stype_seq pointer_seqn;   /* Sequence functions for pointer types with a base-size of anything else. */


/* Structured operator invocation functions */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_Assign(DeeSTypeObject *tp_self, void *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Str(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Repr(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeStruct_Bool(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Call(DeeSTypeObject *tp_self, void *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_Int32(DeeSTypeObject *tp_self, void *self, int32_t *result);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_Int64(DeeSTypeObject *tp_self, void *self, int64_t *result);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_Double(DeeSTypeObject *tp_self, void *self, double *result);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Int(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Inv(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Pos(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Neg(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Add(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Sub(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Mul(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Div(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Mod(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Shl(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Shr(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_And(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Or(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Xor(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Pow(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeStruct_Inc(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeStruct_Dec(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceAdd(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceSub(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceMul(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceDiv(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceMod(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceShl(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceShr(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceAnd(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceOr(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplaceXor(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_InplacePow(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeStruct_Hash(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Eq(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Ne(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Lo(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Le(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Gr(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Ge(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_Iter(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeStruct_GetSize(DeeSTypeObject *tp_self, void *self);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_Contains(DeeSTypeObject *tp_self, void *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeStruct_GetItem(DeeSTypeObject *tp_self, void *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeStruct_DelItem(DeeSTypeObject *tp_self, void *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL DeeStruct_SetItem(DeeSTypeObject *tp_self, void *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3, 4)) DREF DeeObject *DCALL DeeStruct_GetRange(DeeSTypeObject *tp_self, void *self, DeeObject *begin, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL DeeStruct_DelRange(DeeSTypeObject *tp_self, void *self, DeeObject *begin, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 3, 4, 5)) int DCALL DeeStruct_SetRange(DeeSTypeObject *tp_self, void *self, DeeObject *begin, DeeObject *end, DeeObject *value);

/* Get a struct attribute (excluding generic attributes)
 * @return: * :        Attribute value
 * @return: ITER_DONE: No such attribute
 * @return: NULL:      Error */
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_GetAttr(DeeSTypeObject *tp_self, void *self, DeeObject *name);

/* Delete a struct attribute (excluding generic attributes)
 * @return: 0 : Success
 * @return: -1: Error
 * @return: -2: No such attribute */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL
DeeStruct_DelAttr(DeeSTypeObject *tp_self, void *self, DeeObject *name);

/* Set a struct attribute (excluding generic attributes)
 * @return: 0 : Success
 * @return: -1: Error
 * @return: -2: No such attribute */
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL
DeeStruct_SetAttr(DeeSTypeObject *tp_self, void *self,
                  DeeObject *name, DeeObject *value);

/* Enumerate struct attributes (excluding generic attributes) */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL
DeeStruct_IterAttr(DeeSTypeObject *__restrict tp_self,
                   struct Dee_attriter *iterbuf, size_t bufsize,
                   struct Dee_attrhint const *__restrict hint);


/* Builtin standard C types. */
INTDEF DeeSTypeObject DeeCVoid_Type;
INTDEF DeeSTypeObject DeeCChar_Type;
INTDEF DeeSTypeObject DeeCWChar_Type;
INTDEF DeeSTypeObject DeeCChar16_Type;
INTDEF DeeSTypeObject DeeCChar32_Type;
INTDEF DeeSTypeObject DeeCBool_Type;
INTDEF DeeSTypeObject DeeCInt8_Type;
INTDEF DeeSTypeObject DeeCInt16_Type;
INTDEF DeeSTypeObject DeeCInt32_Type;
INTDEF DeeSTypeObject DeeCInt64_Type;
INTDEF DeeSTypeObject DeeCInt128_Type;
INTDEF DeeSTypeObject DeeCUInt8_Type;
INTDEF DeeSTypeObject DeeCUInt16_Type;
INTDEF DeeSTypeObject DeeCUInt32_Type;
INTDEF DeeSTypeObject DeeCUInt64_Type;
INTDEF DeeSTypeObject DeeCUInt128_Type;


#define PRIVATE_CINT_SIZED_1   DeeCInt8_Type
#define PRIVATE_CINT_SIZED_2   DeeCInt16_Type
#define PRIVATE_CINT_SIZED_4   DeeCInt32_Type
#define PRIVATE_CINT_SIZED_8   DeeCInt64_Type
#define PRIVATE_CINT_SIZED_16  DeeCInt128_Type
#define PRIVATE_CUINT_SIZED_1  DeeCUInt8_Type
#define PRIVATE_CUINT_SIZED_2  DeeCUInt16_Type
#define PRIVATE_CUINT_SIZED_4  DeeCUInt32_Type
#define PRIVATE_CUINT_SIZED_8  DeeCUInt64_Type
#define PRIVATE_CUINT_SIZED_16 DeeCUInt128_Type
#define CINT_SIZED(sizeof)     PP_PRIVATE_CAT2(PRIVATE_CINT_SIZED_, sizeof)
#define CUINT_SIZED(sizeof)    PP_PRIVATE_CAT2(PRIVATE_CUINT_SIZED_, sizeof)


INTDEF DeeSTypeObject  DeeCFloat_Type;
INTDEF DeeSTypeObject  DeeCDouble_Type;
INTDEF DeeSTypeObject  DeeCLDouble_Type;



#if CTYPES_sizeof_char == 1 && CTYPES_alignof_char == __ALIGNOF_INT8__
#define DeeCSChar_Type  DeeCInt8_Type
#define DeeCUChar_Type  DeeCUInt8_Type
#elif CTYPES_sizeof_char == 2 && CTYPES_alignof_char == __ALIGNOF_INT16__
#define DeeCSChar_Type  DeeCInt16_Type
#define DeeCUChar_Type  DeeCUInt16_Type
#elif CTYPES_sizeof_char == 4 && CTYPES_alignof_char == __ALIGNOF_INT32__
#define DeeCSChar_Type  DeeCInt32_Type
#define DeeCUChar_Type  DeeCUInt32_Type
#elif CTYPES_sizeof_char == 8 && CTYPES_alignof_char == __ALIGNOF_INT64__
#define DeeCSChar_Type  DeeCInt64_Type
#define DeeCUChar_Type  DeeCUInt64_Type
#else /* ... */
#define CONFIG_SUCHAR_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCSChar_Type;
INTDEF DeeSTypeObject   DeeCUChar_Type;
#endif /* !... */

#if CTYPES_sizeof_short == 1 && CTYPES_alignof_short == __ALIGNOF_INT8__
#define DeeCShort_Type  DeeCInt8_Type
#define DeeCUShort_Type DeeCUInt8_Type
#elif CTYPES_sizeof_short == 2 && CTYPES_alignof_short == __ALIGNOF_INT16__
#define DeeCShort_Type  DeeCInt16_Type
#define DeeCUShort_Type DeeCUInt16_Type
#elif CTYPES_sizeof_short == 4 && CTYPES_alignof_short == __ALIGNOF_INT32__
#define DeeCShort_Type  DeeCInt32_Type
#define DeeCUShort_Type DeeCUInt32_Type
#elif CTYPES_sizeof_short == 8 && CTYPES_alignof_short == __ALIGNOF_INT64__
#define DeeCShort_Type  DeeCInt64_Type
#define DeeCUShort_Type DeeCUInt64_Type
#else /* ... */
#define CONFIG_SHORT_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCShort_Type;
INTDEF DeeSTypeObject   DeeCUShort_Type;
#endif /* !... */

#if CTYPES_sizeof_int == 1 && CTYPES_alignof_int == __ALIGNOF_INT8__
#define DeeCInt_Type    DeeCInt8_Type
#define DeeCUInt_Type   DeeCUInt8_Type
#elif CTYPES_sizeof_int == 2 && CTYPES_alignof_int == __ALIGNOF_INT16__
#define DeeCInt_Type    DeeCInt16_Type
#define DeeCUInt_Type   DeeCUInt16_Type
#elif CTYPES_sizeof_int == 4 && CTYPES_alignof_int == __ALIGNOF_INT32__
#define DeeCInt_Type    DeeCInt32_Type
#define DeeCUInt_Type   DeeCUInt32_Type
#elif CTYPES_sizeof_int == 8 && CTYPES_alignof_int == __ALIGNOF_INT64__
#define DeeCInt_Type    DeeCInt64_Type
#define DeeCUInt_Type   DeeCUInt64_Type
#else /* ... */
#define CONFIG_INT_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCInt_Type;
INTDEF DeeSTypeObject   DeeCUInt_Type;
#endif /* !... */

#if CTYPES_sizeof_long == 1 && CTYPES_alignof_long == __ALIGNOF_INT8__
#define DeeCLong_Type   DeeCInt8_Type
#define DeeCULong_Type  DeeCUInt8_Type
#elif CTYPES_sizeof_long == 2 && CTYPES_alignof_long == __ALIGNOF_INT16__
#define DeeCLong_Type   DeeCInt16_Type
#define DeeCULong_Type  DeeCUInt16_Type
#elif CTYPES_sizeof_long == 4 && CTYPES_alignof_long == __ALIGNOF_INT32__
#define DeeCLong_Type   DeeCInt32_Type
#define DeeCULong_Type  DeeCUInt32_Type
#elif CTYPES_sizeof_long == 8 && CTYPES_alignof_long == __ALIGNOF_INT64__
#define DeeCLong_Type   DeeCInt64_Type
#define DeeCULong_Type  DeeCUInt64_Type
#else /* ... */
#define CONFIG_LONG_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCLong_Type;
INTDEF DeeSTypeObject   DeeCULong_Type;
#endif /* !... */

#if CTYPES_sizeof_llong == 1 && CTYPES_alignof_llong == __ALIGNOF_INT8__
#define DeeCLLong_Type  DeeCInt8_Type
#define DeeCULLong_Type DeeCUInt8_Type
#elif CTYPES_sizeof_llong == 2 && CTYPES_alignof_llong == __ALIGNOF_INT16__
#define DeeCLLong_Type  DeeCInt16_Type
#define DeeCULLong_Type DeeCUInt16_Type
#elif CTYPES_sizeof_llong == 4 && CTYPES_alignof_llong == __ALIGNOF_INT32__
#define DeeCLLong_Type  DeeCInt32_Type
#define DeeCULLong_Type DeeCUInt32_Type
#elif CTYPES_sizeof_llong == 8 && CTYPES_alignof_llong == __ALIGNOF_INT64__
#define DeeCLLong_Type  DeeCInt64_Type
#define DeeCULLong_Type DeeCUInt64_Type
#else /* ... */
#define CONFIG_LLONG_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCLLong_Type;
INTDEF DeeSTypeObject   DeeCULLong_Type;
#endif /* !... */

#if (!defined(CONFIG_LONG_NEEDS_OWN_TYPE) &&                   \
     (CTYPES_sizeof_long == CTYPES_sizeof_int && \
      CTYPES_alignof_long == CTYPES_alignof_int))
/* Make `long' its own distinct type. */
#define CONFIG_LONG_NEEDS_OWN_TYPE
#undef DeeCLong_Type
#undef DeeCULong_Type
INTDEF DeeSTypeObject DeeCLong_Type;
INTDEF DeeSTypeObject DeeCULong_Type;
#endif /* ... */


/* Array types, and foreign-function types. */
struct array_type_object {
	DeeSTypeObject                at_base;  /* The underlying structured type descriptor. */
	DREF DeeSTypeObject          *at_orig;  /* [1..1][const] The array's element type. */
	LIST_ENTRY(array_type_object) at_chain; /* [lock(at_orig->st_cachelock)] Hash-map entry of this array. */
	size_t                        at_count; /* The total number of items. */
};


INTDEF DeeTypeObject DeeArrayType_Type;
#define DeeArrayType_Check(ob) \
	DeeObject_InstanceOfExact((DeeObject *)(ob), &DeeArrayType_Type) /* `array_type' is final */
#define DeeArrayType_AsObject(x) DeeSType_AsObject(DeeArrayType_AsSType(x))
#define DeeArrayType_AsType(x)   (&(x)->at_base.st_base)
#define DeeArrayType_AsSType(x)  (&(x)->at_base)

/* Base classes for all C array types. */
INTDEF DeeArrayTypeObject DeeArray_Type;
#if 1 /* Both would work, but this one returns in O(1) */
#define DeeArray_Check(ob) DeeArrayType_Check(Dee_TYPE(ob))
#else
#define DeeArray_Check(ob) DeeObject_InstanceOf(ob, &DeeArray_Type)
#endif

/* Construct an array structured type that
 * consists of `num_items' instances of `self'. */
INTDEF WUNUSED NONNULL((1)) DREF DeeArrayTypeObject *DCALL
DeeSType_Array(DeeSTypeObject *__restrict self, size_t num_items);





struct cfunction_type_object {
	DeeSTypeObject                     ft_base;            /* The underlying structured type descriptor. */
#ifndef CONFIG_NO_CFUNCTION
	DREF DeeSTypeObject               *ft_orig;            /* [1..1][const] The function's return type. */
	LIST_ENTRY(cfunction_type_object)  ft_chain;           /* [lock(ft_orig->st_cachelock)] Hash-map entry of this c-function. */
	Dee_hash_t                         ft_hash;            /* [const] A pre-calculated hash used by `struct stype_cfunction' */
	size_t                             ft_argc;            /* [const] Amount of function argument types. */
	DREF DeeSTypeObject              **ft_argv;            /* [1..1][0..ft_argc][owned][const] Vector of function argument types. */
	ctypes_cc_t                        ft_cc;              /* [const] The calling convention used by this function. */
	ffi_type                          *ft_ffi_return_type; /* [1..1] Raw return type. */
	ffi_type                         **ft_ffi_arg_type_v;  /* [1..1][0..ob_argc][owned] Raw argument types. */
	ffi_cif                            ft_ffi_cif;         /* cif object to call the function. */
	/* WBuffer layout:
	 *  1. return value memory
	 *  2. argument memory...
	 *  3. argument pointers... */
	size_t                             ft_wsize;
	size_t                             ft_woff_argmem;
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
	union {
		size_t                         ft_woff_argptr;
		size_t                         ft_woff_variadic_argmem;
	};
#else /* __COMPILER_HAVE_TRANSPARENT_UNION */
	size_t                             ft_woff_argptr;
#define ft_woff_variadic_argmem        ft_woff_argptr
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
#endif /* !CONFIG_NO_CFUNCTION */
};

#define DeeCFunctionType_AsObject(x) DeeSType_AsObject(DeeCFunctionType_AsSType(x))
#define DeeCFunctionType_AsType(x)   (&(x)->ft_base.st_base)
#define DeeCFunctionType_AsSType(x)  (&(x)->ft_base)

INTDEF DeeTypeObject DeeCFunctionType_Type;
#define DeeCFunctionType_Check(ob) \
	DeeObject_InstanceOfExact((DeeObject *)(ob), &DeeCFunctionType_Type) /* `CFunctionType' is final */

INTDEF DeeCFunctionTypeObject DeeCFunction_Type;

#if 1 /* Both would work, but this one returns in O(1) */
#define DeeCFunction_Check(ob) DeeCFunctionType_Check(Dee_TYPE(ob))
#else
#define DeeCFunction_Check(ob) DeeObject_InstanceOf(ob, &DeeCFunction_Type)
#endif

/* Construct a C-function structured type that returns
 * an instance of `return_type' while taking `argc' arguments,
 * each of type `argv' when called using `calling_convention'
 * @param: calling_convention: One of `FFI_*' (Defaults to `CC_DEFAULT')
 * @param: inherit_argv: When `true', _always_ inherit the `argv' vector (even upon error)
 *                       Note however, that vector elements are not
 *                       inherited (as denoted by the lack of a DREF tag).
 * When `ctypes' has been built with `CONFIG_NO_CFUNCTION',
 * this function throws a NotImplemented error. */
INTDEF WUNUSED NONNULL((1)) DREF DeeCFunctionTypeObject *DCALL
DeeSType_CFunction(DeeSTypeObject *__restrict return_type,
                   ctypes_cc_t calling_convention, size_t argc,
                   DeeSTypeObject **argv, bool inherit_argv);


#ifdef CONFIG_NO_CFUNCTION
/* Throw a NotImplemented error explaining that cfunctions have been disabled. */
INTDEF ATTR_COLD void DCALL err_no_cfunction(void);
#endif /* CONFIG_NO_CFUNCTION */









struct Dee_string_object;
struct struct_field {
	DREF struct Dee_string_object *sf_name;   /* [0..1] The name of this field (NULL is used as sentinel) */
	Dee_hash_t                     sf_hash;   /* [valid_if(sf_name)][const][== DeeString_Hash(sf_name)] */
	uintptr_t                      sf_offset; /* [valid_if(sf_name)] Offset of the field (from `DeeStruct_Data()') */
	DREF DeeLValueTypeObject      *sf_type;   /* [1..1][valid_if(sf_name)] The l-value variant of this field's type. */
};

typedef struct struct_type_object {
	DeeSTypeObject                               st_base;  /* The underlying type object. */
	Dee_hash_t                                   st_fmsk;  /* [const] Field-vector mask. */
	COMPILER_FLEXIBLE_ARRAY(struct struct_field, st_fvec); /* [1..st_fmsk+1][const] Hash-vector of field names. */
} DeeStructTypeObject;

struct empty_struct_type_object {
	DeeSTypeObject      st_base;    /* The underlying type object. */
	Dee_hash_t          st_fmsk;    /* [== 0][const] Field-vector mask. */
	struct struct_field st_fvec[1]; /* [1..st_fmsk+1][const] Hash-vector of field names. */
};

#define DeeStructType_AsObject(x) DeeSType_AsObject(DeeStructType_AsSType(x))
#define DeeStructType_AsType(x)   (&(x)->st_base.st_base)
#define DeeStructType_AsSType(x)  (&(x)->st_base)

#define STRUCT_TYPE_HASHST(self, hash)  ((hash) & ((DeeStructTypeObject *)(self))->st_fmsk)
#define STRUCT_TYPE_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define STRUCT_TYPE_HASHIT(self, i)     (((DeeStructTypeObject *)(self))->st_fvec + ((i) & ((DeeStructTypeObject *)(self))->st_fmsk))


INTDEF DeeTypeObject DeeStructType_Type;
INTDEF struct empty_struct_type_object DeeStruct_Type;
#define DeeStructType_Check(ob) \
	DeeObject_InstanceOfExact((DeeObject *)(ob), &DeeStructType_Type) /* `struct_type' is final */

/* Construct a new struct-type from `fields', which is a `{(string, StructuredType)...}' */
INTDEF WUNUSED NONNULL((2)) DREF DeeStructTypeObject *DCALL
DeeStructType_FromSequence(DeeObject *name,
                           DeeObject *__restrict fields,
                           unsigned int flags);
#define STRUCT_TYPE_FNORMAL  0x0000 /* No special flags. */
#define STRUCT_TYPE_FPACKED  0x0001 /* Create a packed structure. */
#define STRUCT_TYPE_FUNION   0x0002 /* Create a union. */

/* Helper functions for constructing new integer objects. */
#if CTYPES_sizeof_int == 1
#define int_news8 int_newint
#elif CTYPES_sizeof_int == 2
#define int_news16 int_newint
#elif CTYPES_sizeof_int == 4
#define int_news32 int_newint
#elif CTYPES_sizeof_int == 8
#define int_news64 int_newint
#endif /* CTYPES_sizeof_int == ... */
INTDEF WUNUSED DREF DeeObject *DCALL int_news8(int8_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_news16(int16_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_news32(int32_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_news64(int64_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_news128(Dee_int128_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_newu8(uint8_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_newu16(uint16_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_newu32(uint32_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_newu64(uint64_t val);
INTDEF WUNUSED DREF DeeObject *DCALL int_newu128(Dee_uint128_t val);
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */


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
INTDEF DeeTypeObject DeeShLib_Type;

DECL_END

#endif /* !GUARD_DEX_CTYPES_LIBCTYPES_H */
