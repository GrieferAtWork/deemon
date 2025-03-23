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
#ifndef GUARD_DEX_CTYPES_LIBCTYPES_H
#define GUARD_DEX_CTYPES_LIBCTYPES_H 1

#include <deemon/api.h>
#include <deemon/mro.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/util/lock.h>

#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>

#ifndef CONFIG_NO_CFUNCTION
#include <ffi.h>
#include <ffitarget.h>
#endif /* !CONFIG_NO_CFUNCTION */
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintptr_t, intN_t, uintN_t */


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
#ifdef CONFIG_EXPERIMENTAL_ATTRITER
	WUNUSED_T NONNULL_T((1, 4)) size_t
	(DCALL *st_iterattr)(DeeSTypeObject *__restrict tp_self,
	                     struct Dee_attriter *iterbuf, size_t bufsize,
	                     struct Dee_attrhint const *__restrict hint);
#else /* CONFIG_EXPERIMENTAL_ATTRITER */
	WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t
	(DCALL *st_enumattr)(DeeSTypeObject *__restrict tp_self, Dee_enum_t proc, void *arg);
#endif /* !CONFIG_EXPERIMENTAL_ATTRITER */
};


LIST_HEAD(array_type_list, array_type_object);
struct stype_array {
	size_t                  sa_size; /* Amount of cached array types. */
	size_t                  sa_mask; /* Allocated map mask. */
	struct array_type_list *sa_list; /* [0..1][0..sa_mask+1][owned] Hash-map of array types.
	                                  * As hash for indexing this map, use `at_count'. */
};
#define STYPE_ARRAY_INIT { 0, 0, NULL }

#ifndef CONFIG_NO_CFUNCTION
LIST_HEAD(cfunction_type_list, cfunction_type_object);
struct stype_cfunction {
	size_t                      sf_size; /* Amount of cached function types. */
	size_t                      sf_mask; /* Allocated map mask. */
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
#define DeeSType_AsObject(x)       ((DeeObject *)&(x)->st_base)
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


union pointer {
	uintptr_t       uint;
	intptr_t        sint;
	void           *ptr;
	uint8_t        *p8;
	uint16_t       *p16;
	uint32_t       *p32;
	uint64_t       *p64;
	int8_t         *ps8;
	int16_t        *ps16;
	int32_t        *ps32;
	int64_t        *ps64;
	void           *pvoid;
	char           *pchar;
	Dee_wchar_t    *pwchar;
	signed char    *pschar;
	unsigned char  *puchar;
	short          *pshort;
	unsigned short *pushort;
	int            *pint;
	unsigned int   *puint;
	long           *plong;
	unsigned long  *pulong;
};

struct pointer_object {
	OBJECT_HEAD
	union pointer p_ptr;
};

struct lvalue_object {
	OBJECT_HEAD
	union pointer l_ptr;
};


/* Derived from `DeeStructured_Type', these types are the base
 * classes of all pointer/lvalue types that are be generated
 * on-the-fly for structured types.
 * WARNING: The copy-constructor of `DeeLValue_Type' does not return
 *          another l-value object, but rather a copy of the pointed-to object. */
INTDEF DeePointerTypeObject DeePointer_Type;
INTDEF DeeLValueTypeObject DeeLValue_Type;

#if 1 /* Both would work, but this one takes constant armored time. */
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
DeePointer_New(DeePointerTypeObject *pointer_type,
               void *pointer_value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeePointer_NewFor(DeeSTypeObject *pointer_type,
                  void *pointer_value);
#define DeePointer_NewVoid(pointer_value) \
	DeePointer_NewFor(&DeeCVoid_Type, pointer_value)
#define DeePointer_NewChar(pointer_value) \
	DeePointer_NewFor(&DeeCChar_Type, pointer_value)


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
#ifdef CONFIG_EXPERIMENTAL_ATTRITER
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL
DeeStruct_IterAttr(DeeSTypeObject *__restrict tp_self,
                   struct Dee_attriter *iterbuf, size_t bufsize,
                   struct Dee_attrhint const *__restrict hint);
#else /* CONFIG_EXPERIMENTAL_ATTRITER */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeStruct_EnumAttr(DeeSTypeObject *__restrict tp_self,
                   Dee_enum_t proc, void *arg);
#endif /* !CONFIG_EXPERIMENTAL_ATTRITER */


#ifdef __SIZEOF_BOOL__
#define CONFIG_CTYPES_SIZEOF_BOOL __SIZEOF_BOOL__
#else /* __SIZEOF_BOOL__ */
#define CONFIG_CTYPES_SIZEOF_BOOL 1
#endif /* !__SIZEOF_BOOL__ */

#ifdef __SIZEOF_WCHAR_T__
#define CONFIG_CTYPES_SIZEOF_WCHAR __SIZEOF_WCHAR_T__
#elif defined(CONFIG_HOST_WINDOWS)
#define CONFIG_CTYPES_SIZEOF_WCHAR 2
#else /* ... */
#define CONFIG_CTYPES_SIZEOF_WCHAR 4
#endif /* !... */

#define CONFIG_CTYPES_SIZEOF_CHAR16 2
#define CONFIG_CTYPES_SIZEOF_CHAR32 4

#ifdef __SIZEOF_CHAR__
#define CONFIG_CTYPES_SIZEOF_CHAR __SIZEOF_CHAR__
#else /* __SIZEOF_CHAR__ */
#define CONFIG_CTYPES_SIZEOF_CHAR 1
#endif /* !__SIZEOF_CHAR__ */

#ifdef __SIZEOF_SHORT__
#define CONFIG_CTYPES_SIZEOF_SHORT __SIZEOF_SHORT__
#else /* __SIZEOF_SHORT__ */
#define CONFIG_CTYPES_SIZEOF_SHORT 2
#endif /* !__SIZEOF_SHORT__ */

#ifdef __SIZEOF_INT__
#define CONFIG_CTYPES_SIZEOF_INT __SIZEOF_INT__
#else /* __SIZEOF_INT__ */
#define CONFIG_CTYPES_SIZEOF_INT 4
#endif /* !__SIZEOF_INT__ */

#ifdef __SIZEOF_LONG__
#define CONFIG_CTYPES_SIZEOF_LONG __SIZEOF_LONG__
#elif defined(CONFIG_HOST_WINDOWS)
#define CONFIG_CTYPES_SIZEOF_LONG 4
#else /* ... */
#define CONFIG_CTYPES_SIZEOF_LONG __SIZEOF_POINTER__
#endif /* !... */

#ifdef __SIZEOF_LONG_LONG__
#define CONFIG_CTYPES_SIZEOF_LLONG __SIZEOF_LONG_LONG__
#elif defined(__SIZEOF_LLONG__)
#define CONFIG_CTYPES_SIZEOF_LLONG __SIZEOF_LLONG__
#else /* ... */
#define CONFIG_CTYPES_SIZEOF_LLONG 8
#endif /* !... */

#ifdef __CHAR_UNSIGNED__
#define CONFIG_CTYPES_CHAR_UNSIGNED
#endif /* __CHAR_UNSIGNED__ */

#ifdef __WCHAR_UNSIGNED__
#define CONFIG_CTYPES_WCHAR_UNSIGNED
#endif /* __WCHAR_UNSIGNED__ */

#define CONFIG_CTYPES_CHAR16_UNSIGNED
#define CONFIG_CTYPES_CHAR32_UNSIGNED

#define CONFIG_CTYPES_ALIGNOF_POINTER __ALIGNOF_POINTER__
#define CONFIG_CTYPES_ALIGNOF_LVALUE  CONFIG_CTYPES_ALIGNOF_POINTER
#define CONFIG_CTYPES_ALIGNOF_BOOL    __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_BOOL)
#define CONFIG_CTYPES_ALIGNOF_WCHAR   __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_WCHAR)
#define CONFIG_CTYPES_ALIGNOF_CHAR16  __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_CHAR16)
#define CONFIG_CTYPES_ALIGNOF_CHAR32  __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_CHAR32)
#define CONFIG_CTYPES_ALIGNOF_CHAR    __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_CHAR)
#define CONFIG_CTYPES_ALIGNOF_SHORT   __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_SHORT)
#define CONFIG_CTYPES_ALIGNOF_INT     __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_INT)
#define CONFIG_CTYPES_ALIGNOF_LONG    __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_LONG)
#define CONFIG_CTYPES_ALIGNOF_LLONG   __HYBRID_ALIGNOF(CONFIG_CTYPES_SIZEOF_LLONG)

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


#ifdef __LONGDOUBLE
typedef __LONGDOUBLE long_double;
#else /* __LONGDOUBLE */
typedef long double long_double;
#endif /* !__LONGDOUBLE */
#define CONFIG_CTYPES_FLOAT_TYPE   float
#define CONFIG_CTYPES_DOUBLE_TYPE  double
#if (defined(FFI_TYPE_LONGDOUBLE) && \
     defined(FFI_TYPE_DOUBLE) &&     \
     FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE)
#define CONFIG_CTYPES_LDOUBLE_TYPE double
#else /* FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE */
#define CONFIG_CTYPES_LDOUBLE_TYPE long_double
#endif /* FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE */

INTDEF DeeSTypeObject  DeeCFloat_Type;
INTDEF DeeSTypeObject  DeeCDouble_Type;
INTDEF DeeSTypeObject  DeeCLDouble_Type;

/* Alignment requirements of the host's C floating point types. */
#ifndef CONFIG_ALIGNOF_FLOAT
#define CONFIG_ALIGNOF_FLOAT   COMPILER_ALIGNOF(float)
#endif /* !CONFIG_ALIGNOF_FLOAT */
#ifndef CONFIG_ALIGNOF_DOUBLE
#define CONFIG_ALIGNOF_DOUBLE  COMPILER_ALIGNOF(double)
#endif /* !CONFIG_ALIGNOF_DOUBLE */
#ifndef CONFIG_ALIGNOF_LDOUBLE
#define CONFIG_ALIGNOF_LDOUBLE COMPILER_ALIGNOF(long_double)
#endif /* !CONFIG_ALIGNOF_LDOUBLE */




#if CONFIG_CTYPES_SIZEOF_CHAR == 1 && CONFIG_CTYPES_ALIGNOF_CHAR == __ALIGNOF_INT8__
#define DeeCSChar_Type  DeeCInt8_Type
#define DeeCUChar_Type  DeeCUInt8_Type
#elif CONFIG_CTYPES_SIZEOF_CHAR == 2 && CONFIG_CTYPES_ALIGNOF_CHAR == __ALIGNOF_INT16__
#define DeeCSChar_Type  DeeCInt16_Type
#define DeeCUChar_Type  DeeCUInt16_Type
#elif CONFIG_CTYPES_SIZEOF_CHAR == 4 && CONFIG_CTYPES_ALIGNOF_CHAR == __ALIGNOF_INT32__
#define DeeCSChar_Type  DeeCInt32_Type
#define DeeCUChar_Type  DeeCUInt32_Type
#elif CONFIG_CTYPES_SIZEOF_CHAR == 8 && CONFIG_CTYPES_ALIGNOF_CHAR == __ALIGNOF_INT64__
#define DeeCSChar_Type  DeeCInt64_Type
#define DeeCUChar_Type  DeeCUInt64_Type
#else /* ... */
#define CONFIG_SUCHAR_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCSChar_Type;
INTDEF DeeSTypeObject   DeeCUChar_Type;
#endif /* !... */

#if CONFIG_CTYPES_SIZEOF_SHORT == 1 && CONFIG_CTYPES_ALIGNOF_SHORT == __ALIGNOF_INT8__
#define DeeCShort_Type  DeeCInt8_Type
#define DeeCUShort_Type DeeCUInt8_Type
#elif CONFIG_CTYPES_SIZEOF_SHORT == 2 && CONFIG_CTYPES_ALIGNOF_SHORT == __ALIGNOF_INT16__
#define DeeCShort_Type  DeeCInt16_Type
#define DeeCUShort_Type DeeCUInt16_Type
#elif CONFIG_CTYPES_SIZEOF_SHORT == 4 && CONFIG_CTYPES_ALIGNOF_SHORT == __ALIGNOF_INT32__
#define DeeCShort_Type  DeeCInt32_Type
#define DeeCUShort_Type DeeCUInt32_Type
#elif CONFIG_CTYPES_SIZEOF_SHORT == 8 && CONFIG_CTYPES_ALIGNOF_SHORT == __ALIGNOF_INT64__
#define DeeCShort_Type  DeeCInt64_Type
#define DeeCUShort_Type DeeCUInt64_Type
#else /* ... */
#define CONFIG_SHORT_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCShort_Type;
INTDEF DeeSTypeObject   DeeCUShort_Type;
#endif /* !... */

#if CONFIG_CTYPES_SIZEOF_INT == 1 && CONFIG_CTYPES_ALIGNOF_INT == __ALIGNOF_INT8__
#define DeeCInt_Type    DeeCInt8_Type
#define DeeCUInt_Type   DeeCUInt8_Type
#elif CONFIG_CTYPES_SIZEOF_INT == 2 && CONFIG_CTYPES_ALIGNOF_INT == __ALIGNOF_INT16__
#define DeeCInt_Type    DeeCInt16_Type
#define DeeCUInt_Type   DeeCUInt16_Type
#elif CONFIG_CTYPES_SIZEOF_INT == 4 && CONFIG_CTYPES_ALIGNOF_INT == __ALIGNOF_INT32__
#define DeeCInt_Type    DeeCInt32_Type
#define DeeCUInt_Type   DeeCUInt32_Type
#elif CONFIG_CTYPES_SIZEOF_INT == 8 && CONFIG_CTYPES_ALIGNOF_INT == __ALIGNOF_INT64__
#define DeeCInt_Type    DeeCInt64_Type
#define DeeCUInt_Type   DeeCUInt64_Type
#else /* ... */
#define CONFIG_INT_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCInt_Type;
INTDEF DeeSTypeObject   DeeCUInt_Type;
#endif /* !... */

#if CONFIG_CTYPES_SIZEOF_LONG == 1 && CONFIG_CTYPES_ALIGNOF_LONG == __ALIGNOF_INT8__
#define DeeCLong_Type   DeeCInt8_Type
#define DeeCULong_Type  DeeCUInt8_Type
#elif CONFIG_CTYPES_SIZEOF_LONG == 2 && CONFIG_CTYPES_ALIGNOF_LONG == __ALIGNOF_INT16__
#define DeeCLong_Type   DeeCInt16_Type
#define DeeCULong_Type  DeeCUInt16_Type
#elif CONFIG_CTYPES_SIZEOF_LONG == 4 && CONFIG_CTYPES_ALIGNOF_LONG == __ALIGNOF_INT32__
#define DeeCLong_Type   DeeCInt32_Type
#define DeeCULong_Type  DeeCUInt32_Type
#elif CONFIG_CTYPES_SIZEOF_LONG == 8 && CONFIG_CTYPES_ALIGNOF_LONG == __ALIGNOF_INT64__
#define DeeCLong_Type   DeeCInt64_Type
#define DeeCULong_Type  DeeCUInt64_Type
#else /* ... */
#define CONFIG_LONG_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCLong_Type;
INTDEF DeeSTypeObject   DeeCULong_Type;
#endif /* !... */

#if CONFIG_CTYPES_SIZEOF_LLONG == 1 && CONFIG_CTYPES_ALIGNOF_LLONG == __ALIGNOF_INT8__
#define DeeCLLong_Type  DeeCInt8_Type
#define DeeCULLong_Type DeeCUInt8_Type
#elif CONFIG_CTYPES_SIZEOF_LLONG == 2 && CONFIG_CTYPES_ALIGNOF_LLONG == __ALIGNOF_INT16__
#define DeeCLLong_Type  DeeCInt16_Type
#define DeeCULLong_Type DeeCUInt16_Type
#elif CONFIG_CTYPES_SIZEOF_LLONG == 4 && CONFIG_CTYPES_ALIGNOF_LLONG == __ALIGNOF_INT32__
#define DeeCLLong_Type  DeeCInt32_Type
#define DeeCULLong_Type DeeCUInt32_Type
#elif CONFIG_CTYPES_SIZEOF_LLONG == 8 && CONFIG_CTYPES_ALIGNOF_LLONG == __ALIGNOF_INT64__
#define DeeCLLong_Type  DeeCInt64_Type
#define DeeCULLong_Type DeeCUInt64_Type
#else /* ... */
#define CONFIG_LLONG_NEEDS_OWN_TYPE
INTDEF DeeSTypeObject   DeeCLLong_Type;
INTDEF DeeSTypeObject   DeeCULLong_Type;
#endif /* !... */

#if (!defined(CONFIG_LONG_NEEDS_OWN_TYPE) &&                   \
     (CONFIG_CTYPES_SIZEOF_LONG == CONFIG_CTYPES_SIZEOF_INT && \
      CONFIG_CTYPES_ALIGNOF_LONG == CONFIG_CTYPES_ALIGNOF_INT))
/* Make `long' its own distinct type. */
#define CONFIG_LONG_NEEDS_OWN_TYPE
#undef DeeCLong_Type
#undef DeeCULong_Type
INTDEF DeeSTypeObject DeeCLong_Type;
INTDEF DeeSTypeObject DeeCULong_Type;
#endif /* ... */


#if CONFIG_CTYPES_SIZEOF_INT == 1
#define CTYPES_INT  int8_t
#define CTYPES_UINT uint8_t
#elif CONFIG_CTYPES_SIZEOF_INT == 2
#define CTYPES_INT  int16_t
#define CTYPES_UINT uint16_t
#elif CONFIG_CTYPES_SIZEOF_INT == 4
#define CTYPES_INT  int32_t
#define CTYPES_UINT uint32_t
#elif CONFIG_CTYPES_SIZEOF_INT == 8
#define CTYPES_INT  int64_t
#define CTYPES_UINT uint64_t
#else /* CONFIG_CTYPES_SIZEOF_INT == ... */
#define CTYPES_INT  int
#define CTYPES_UINT unsigned int
#endif /* CONFIG_CTYPES_SIZEOF_INT != ... */


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
#if 1 /* Both would work, but this one takes constant armored time. */
#define DeeArray_Check(ob) DeeArrayType_Check(Dee_TYPE(ob))
#else
#define DeeArray_Check(ob) DeeObject_InstanceOf(ob, &DeeArray_Type)
#endif

/* Construct an array structured type that
 * consists of `num_items' instances of `self'. */
INTDEF WUNUSED NONNULL((1)) DREF DeeArrayTypeObject *DCALL
DeeSType_Array(DeeSTypeObject *__restrict self, size_t num_items);





#ifndef CONFIG_NO_CFUNCTION
typedef ffi_abi ctypes_cc_t;
#define CC_DEFAULT   FFI_DEFAULT_ABI
#define CC_MTYPE     ((ctypes_cc_t)0x7fff) /* MASK: The actual FFI type. */
#define CC_FVARARGS  ((ctypes_cc_t)0x8000) /* FLAG: Variable-length argument list. */
#define CC_INVALID   ((ctypes_cc_t)-1)
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

#if 1 /* Both would work, but this one takes constant armored time. */
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









typedef struct struct_type_object DeeStructTypeObject;
struct string_object;
struct struct_field {
	DREF struct string_object *sf_name;   /* [0..1] The name of this field (NULL is used as sentinel) */
	Dee_hash_t                 sf_hash;   /* [valid_if(sf_name)][const][== DeeString_Hash(sf_name)] */
	uintptr_t                  sf_offset; /* [valid_if(sf_name)] Offset of the field (from `DeeStruct_Data()') */
	DREF DeeLValueTypeObject  *sf_type;   /* [1..1][valid_if(sf_name)] The l-value variant of this field's type. */
};

struct struct_type_object {
	DeeSTypeObject                               st_base;  /* The underlying type object. */
	Dee_hash_t                                   st_fmsk;  /* [const] Field-vector mask. */
	COMPILER_FLEXIBLE_ARRAY(struct struct_field, st_fvec); /* [1..st_fmsk+1][const] Hash-vector of field names. */
};

struct empty_struct_type_object {
	DeeSTypeObject      st_base;    /* The underlying type object. */
	size_t              st_fmsk;    /* [== 0][const] Field-vector mask. */
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
INTDEF DeeTypeObject DeeShlib_Type;



/* Helper functions for constructing new integer objects. */
#if CONFIG_CTYPES_SIZEOF_INT == 1
#define int_news8 int_newint
#elif CONFIG_CTYPES_SIZEOF_INT == 2
#define int_news16 int_newint
#elif CONFIG_CTYPES_SIZEOF_INT == 4
#define int_news32 int_newint
#elif CONFIG_CTYPES_SIZEOF_INT == 8
#define int_news64 int_newint
#endif /* CONFIG_CTYPES_SIZEOF_INT == ... */
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


DECL_END

#endif /* !GUARD_DEX_CTYPES_LIBCTYPES_H */
