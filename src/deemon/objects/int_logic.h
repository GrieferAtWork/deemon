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
/* NOTE: Deemon's integer object implementation is
 *       heavily based on python's `long' data type.
 *       With that in mind, licensing of deemon's integer
 *       implementation must be GPL-compatible, GPL being
 *       the license that python is restricted by.
 *    >> So to simplify this whole deal: I make no claim of having invented the
 *       way that deemon's (phyton's) arbitrary-length integers are implemented,
 *       with all algorithms found in `int_logic.c' originating from phython
 *       before being adjusted to fit deemon's runtime. */
#ifndef GUARD_DEEMON_OBJECTS_INT_LOGIC_H
#define GUARD_DEEMON_OBJECTS_INT_LOGIC_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>            /* DeeObject_Free */
#include <deemon/int.h>              /* DeeIntObject, Dee_SIZEOF_DIGIT, Dee_sdigit_t */
#include <deemon/object.h>           /* DREF, DeeObject, Dee_OBJECT_OFFSETOF_DATA */
#include <deemon/system-features.h>  /* CONFIG_HAVE_*, memset, memsetl, memsetq, memsetw */
#include <deemon/util/slab-config.h> /* Dee_SLAB_CHUNKSIZE_MAX */

#include <hybrid/typecore.h> /* __SIZEOF_SIZE_T__ */

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t */

DECL_BEGIN

#if Dee_SIZEOF_DIGIT == 2 && defined(CONFIG_HAVE_memsetw)
#define Dee_digit_memset(p, v, n) memsetw(p, v, n)
#elif Dee_SIZEOF_DIGIT == 4 && defined(CONFIG_HAVE_memsetl)
#define Dee_digit_memset(p, v, n) memsetl(p, v, n)
#elif Dee_SIZEOF_DIGIT == 1 && defined(CONFIG_HAVE_memset)
#define Dee_digit_memset(p, v, n) memset(p, v, n)
#elif Dee_SIZEOF_DIGIT == 8 && defined(CONFIG_HAVE_memsetq)
#define Dee_digit_memset(p, v, n) memsetq(p, v, n)
#else /* Dee_SIZEOF_DIGIT == ... */
#define Dee_digit_memset(p, v, n)    \
	do {                             \
		size_t _i;                   \
		for (_i = 0; _i < (n); ++_i) \
			(p)[_i] = (v);           \
	}	__WHILE0
#endif /* Dee_SIZEOF_DIGIT != ... */

#define Dee_SIZEOF_INT_OBJECT(n_digits) \
	(Dee_OBJECT_OFFSETOF_DATA + __SIZEOF_SIZE_T__ + (Dee_SIZEOF_DIGIT * (n_digits)))

#undef Dee_INT_SLAB_MAXDIGITS
#ifdef Dee_SLAB_CHUNKSIZE_MAX
#define Dee_INT_SLAB_MAXDIGITS ((Dee_SLAB_CHUNKSIZE_MAX - Dee_SIZEOF_INT_OBJECT(0)) / Dee_SIZEOF_DIGIT)
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

/* Config: use slab allocators for (small) integers */
#undef CONFIG_USE_SLABS_FOR_INTEGERS
#if defined(Dee_INT_SLAB_MAXDIGITS) && Dee_INT_SLAB_MAXDIGITS >= 2 && 1
#define CONFIG_USE_SLABS_FOR_INTEGERS
#endif /* ... */


#ifdef CONFIG_USE_SLABS_FOR_INTEGERS
#ifdef NDEBUG
typedef ATTR_MALLOC_T WUNUSED_T void *(DCALL *Dee_int_slab_alloc_t)(void);
#else /* NDEBUG */
typedef ATTR_MALLOC_T WUNUSED_T void *(DCALL *Dee_int_slab_alloc_t)(char const *file, int line);
#endif /* !NDEBUG */
typedef NONNULL_T((1)) void (DCALL *Dee_int_slab_free_t)(void *__restrict p);

INTDEF size_t const int_slab_size[Dee_INT_SLAB_MAXDIGITS + 1];
INTDEF Dee_int_slab_alloc_t const int_slab_alloc[Dee_INT_SLAB_MAXDIGITS + 1];
INTDEF Dee_int_slab_free_t const int_slab_free[Dee_INT_SLAB_MAXDIGITS + 1];
#endif /* CONFIG_USE_SLABS_FOR_INTEGERS */


#ifdef NDEBUG
INTDEF WUNUSED DREF DeeIntObject *DCALL DeeInt_Alloc(size_t n_digits);
#else /* NDEBUG */
INTDEF WUNUSED DREF DeeIntObject *DCALL DeeInt_Alloc_d(size_t n_digits, char const *file, int line);
#define DeeInt_Alloc(n_digits) DeeInt_Alloc_d(n_digits, __FILE__, __LINE__)
#endif /* !NDEBUG */
#ifdef CONFIG_USE_SLABS_FOR_INTEGERS
INTDEF NONNULL((1)) void DCALL int_free(void *__restrict ob);
#define DeeInt_Free(self) int_free(Dee_REQUIRES_TYPE(DeeIntObject *, self))
#else /* CONFIG_USE_SLABS_FOR_INTEGERS */
#define DeeInt_Free(self) DeeObject_Free(self)
#endif /* !CONFIG_USE_SLABS_FOR_INTEGERS */

#define DeeInt_Destroy(self) DeeInt_Free(self)

/* Must be called **after** `(*p_self)->ob_size` may have been set to a value
 * that may be less than what was originally specified to `DeeInt_Alloc()`
 *
 * When small integers use slab allocations, this is needed to realloc
 * `*p_self` into a different slab class (or onto a slab in the first place)
 * when `old_n_digits` (value passed to `DeeInt_Alloc()`) has a different
 * allocator than `new_n_digits` (the ABS of the current `(*p_self)->ob_size`)
 *
 * @param: DeeIntObject **p_self: Self-pointer (may be set to NULL on error)
 * @param: size_t old_n_digits:   Original integer allocation
 * @param: size_t new_n_digits:   Final integer allocation */
#ifdef CONFIG_USE_SLABS_FOR_INTEGERS
#define DeeInt_PTruncate(p_self, old_n_digits, new_n_digits)                            \
	(ASSERT((new_n_digits) <= (old_n_digits)),                                          \
	 (new_n_digits) < (old_n_digits)                                                    \
	 ? (void)(*(p_self) = DeeInt_PTruncate_impl(*(p_self), old_n_digits, new_n_digits)) \
	 : (void)0)
#ifdef NDEBUG
INTDEF NONNULL((1)) DREF DeeIntObject *DCALL
DeeInt_PTruncate_impl(/*inherit(always)*/ DREF DeeIntObject *__restrict self,
                      size_t old_n_digits, size_t new_n_digits);
#else /* NDEBUG */
#define DeeInt_PTruncate_impl(self, old_n_digits, new_n_digits) \
	DeeDbgInt_PTruncate_impl(self, old_n_digits, new_n_digits, __FILE__, __LINE__)
INTDEF NONNULL((1)) DREF DeeIntObject *DCALL
DeeDbgInt_PTruncate_impl(/*inherit(always)*/ DREF DeeIntObject *__restrict self,
                         size_t old_n_digits, size_t new_n_digits,
                         char const *file, int line);
#endif /* !NDEBUG */
#else /* CONFIG_USE_SLABS_FOR_INTEGERS */
#define DeeInt_PTruncate(p_self, old_n_digits, new_n_digits) (void)0
#define DeeInt_PTruncate_IS_NOOP 1
#endif /* !CONFIG_USE_SLABS_FOR_INTEGERS */

INTDEF WUNUSED NONNULL((1, 2)) int DCALL int_divmod(DeeIntObject *a, DeeIntObject *b, DeeIntObject **p_div, DeeIntObject **p_rem);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_add(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_sub(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_mul(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_div(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_mod(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_inv(DeeIntObject *__restrict v);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_neg(DeeIntObject *__restrict v);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_shl(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_shr(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_and(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_xor(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_or(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_pow(DeeIntObject *a, DeeObject *b);

INTDEF WUNUSED NONNULL((1)) int DCALL int_inc(DREF DeeIntObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL int_dec(DREF DeeIntObject **__restrict p_self);

INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_AddSDigit(DeeIntObject *__restrict a, Dee_sdigit_t b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_SubSDigit(DeeIntObject *__restrict a, Dee_sdigit_t b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_AddUInt32(DeeIntObject *__restrict a, uint32_t b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_SubUInt32(DeeIntObject *__restrict a, uint32_t b);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_pext(DeeIntObject *self, DeeIntObject *mask);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_pdep(DeeIntObject *self, DeeIntObject *mask);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_LOGIC_H */
