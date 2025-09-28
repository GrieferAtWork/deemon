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
#ifndef GUARD_DEEMON_VARIANT_H
#define GUARD_DEEMON_VARIANT_H 1

#include "api.h"
/**/

#include <hybrid/__unaligned.h>

#include "object.h"
#include "types.h"
#include "util/atomic.h"
/**/

#include <stdint.h>

DECL_BEGIN

/*
 * >> struct Dee_variant;
 *
 * A general-purpose container for deemon objects, or native (C) types.
 * Variants may appear within native C struct representations of deemon
 * types, where they can be used to easily expose some value to deemon
 * user-code in such a way that native C values get turned into deemon
 * objects lazily.
 */
struct Dee_variant;

/* Possible values for "struct Dee_variant::var_type" */
enum Dee_variant_type {
	Dee_VARIANT_UNBOUND, /* Value is unbound (guarantied to be "0") */
	Dee_VARIANT_OBJECT,  /* Value is a deemon object (guarantied to be "1") */
	Dee_VARIANT_INT32,   /* ... */
	Dee_VARIANT_UINT32,  /* ... */
	Dee_VARIANT_INT64,   /* ... */
	Dee_VARIANT_UINT64,  /* ... */
	Dee_VARIANT_INT128,  /* ... */
	Dee_VARIANT_UINT128, /* ... */
#ifndef CONFIG_NO_FPU
	Dee_VARIANT_FLOAT,   /* ... */
#endif /* !CONFIG_NO_FPU */
#ifndef CONFIG_NO_THREADS
	Dee_VARIANT_LOCKED,  /* Value is locked -- yield, then try again */
#endif /* !CONFIG_NO_THREADS */
};

struct Dee_variant {
	enum Dee_variant_type var_type; /* [lock(ATOMIC)] Variant type */
#if __ALIGNOF_POINTER__ > __SIZEOF_INT__
	__BYTE_TYPE__ _var_pad[__ALIGNOF_POINTER__ - __SIZEOF_INT__];
#endif /* __ALIGNOF_POINTER__ > __SIZEOF_INT__ */
	union {
		DREF DeeObject *d_object;      /* [1..1][valid_if(var_type == Dee_VARIANT_OBJECT)] */
#if __ALIGNOF_INT32__ <= __ALIGNOF_POINTER__
		int32_t         d_int32;       /* [valid_if(var_type == Dee_VARIANT_INT32)] */
		uint32_t        d_uint32;      /* [valid_if(var_type == Dee_VARIANT_UINT32)] */
#else /* __ALIGNOF_INT32__ <= __ALIGNOF_POINTER__ */
		__BYTE_TYPE__  _d_int32[4];    /* [valid_if(var_type == Dee_VARIANT_INT32)] */
		__BYTE_TYPE__  _d_uint32[4];   /* [valid_if(var_type == Dee_VARIANT_UINT32)] */
#endif /* __ALIGNOF_INT32__ > __ALIGNOF_POINTER__ */
#if __ALIGNOF_INT64__ <= __ALIGNOF_POINTER__
		int64_t         d_int64;       /* [valid_if(var_type == Dee_VARIANT_INT64)] */
		uint64_t        d_uint64;      /* [valid_if(var_type == Dee_VARIANT_UINT64)] */
#else /* __ALIGNOF_INT64__ <= __ALIGNOF_POINTER__ */
		__BYTE_TYPE__  _d_int64[8];    /* [valid_if(var_type == Dee_VARIANT_INT64)] */
		__BYTE_TYPE__  _d_uint64[8];   /* [valid_if(var_type == Dee_VARIANT_UINT64)] */
#endif /* __ALIGNOF_INT64__ > __ALIGNOF_POINTER__ */
#if __ALIGNOF_INT128__ <= __ALIGNOF_POINTER__
		Dee_int128_t    d_int128;      /* [valid_if(var_type == Dee_VARIANT_INT128)] */
		Dee_uint128_t   d_uint128;     /* [valid_if(var_type == Dee_VARIANT_UINT128)] */
#else /* __ALIGNOF_INT128__ <= __ALIGNOF_POINTER__ */
		__BYTE_TYPE__  _d_int128[16];  /* [valid_if(var_type == Dee_VARIANT_INT128)] */
		__BYTE_TYPE__  _d_uint128[16]; /* [valid_if(var_type == Dee_VARIANT_UINT128)] */
#endif /* __ALIGNOF_INT128__ > __ALIGNOF_POINTER__ */
#ifndef CONFIG_NO_FPU
#if __ALIGNOF_DOUBLE__ <= __ALIGNOF_POINTER__
		double          d_float;       /* [valid_if(var_type == Dee_VARIANT_FLOAT)] */
#else /* __ALIGNOF_DOUBLE__ <= __ALIGNOF_POINTER__ */
		__BYTE_TYPE__  _d_float[__SIZEOF_DOUBLE__]; /* [valid_if(var_type == Dee_VARIANT_FLOAT)] */
#endif /* __ALIGNOF_DOUBLE__ > __ALIGNOF_POINTER__ */
#endif /* !CONFIG_NO_FPU */
	} var_data; /* [lock(var_type == Dee_VARIANT_LOCKED)] Variant data */
};

#if __ALIGNOF_INT32__ <= __ALIGNOF_POINTER__
#define _Dee_variant_set_int32(self, v)  (void)((self)->var_data.d_int32 = (v))
#define _Dee_variant_set_uint32(self, v) (void)((self)->var_data.d_uint32 = (v))
#define _Dee_variant_get_int32(self)     (self)->var_data.d_int32
#define _Dee_variant_get_uint32(self)    (self)->var_data.d_uint32
#else /* __ALIGNOF_INT32__ <= __ALIGNOF_POINTER__ */
#define _Dee_variant_set_int32(self, v)  __hybrid_unaligned_set32((self)->var_data._d_int32, (uint32_t)(v))
#define _Dee_variant_set_uint32(self, v) __hybrid_unaligned_set32((self)->var_data._d_uint32, v)
#define _Dee_variant_get_int32(self)     (int32_t)__hybrid_unaligned_get32((self)->var_data._d_int32)
#define _Dee_variant_get_uint32(self)    __hybrid_unaligned_get32((self)->var_data._d_uint32)
#endif /* __ALIGNOF_INT32__ > __ALIGNOF_POINTER__ */
#if __ALIGNOF_INT64__ <= __ALIGNOF_POINTER__
#define _Dee_variant_set_int64(self, v)  (void)((self)->var_data.d_int64 = (v))
#define _Dee_variant_set_uint64(self, v) (void)((self)->var_data.d_uint64 = (v))
#define _Dee_variant_get_int64(self)     (self)->var_data.d_int64
#define _Dee_variant_get_uint64(self)    (self)->var_data.d_uint64
#else /* __ALIGNOF_INT64__ <= __ALIGNOF_POINTER__ */
#define _Dee_variant_set_int64(self, v)  __hybrid_unaligned_set64((self)->var_data._d_int64, (uint64_t)(v))
#define _Dee_variant_set_uint64(self, v) __hybrid_unaligned_set64((self)->var_data._d_uint64, v)
#define _Dee_variant_get_int64(self)     (int64_t) __hybrid_unaligned_get64((self)->var_data._d_int64)
#define _Dee_variant_get_uint64(self)    __hybrid_unaligned_get64((self)->var_data._d_uint64)
#endif /* __ALIGNOF_INT64__ > __ALIGNOF_POINTER__ */
#if __ALIGNOF_INT128__ <= __ALIGNOF_POINTER__
#define _Dee_variant_set_int128(self, v)  (void)((self)->var_data.d_int128 = (v))
#define _Dee_variant_set_uint128(self, v) (void)((self)->var_data.d_uint128 = (v))
#define _Dee_variant_get_int128(self)     (self)->var_data.d_int128
#define _Dee_variant_get_uint128(self)    (self)->var_data.d_uint128
#else /* __ALIGNOF_INT128__ <= __ALIGNOF_POINTER__ */
#define _Dee_variant_set_int128(self, v)                                                                                                       \
	(void)((self)->var_data._d_int128[0] = __hybrid_int128_getword8(v, 0), (self)->var_data._d_int128[1] = __hybrid_int128_getword8(v, 1),     \
	       (self)->var_data._d_int128[2] = __hybrid_int128_getword8(v, 2), (self)->var_data._d_int128[3] = __hybrid_int128_getword8(v, 3),     \
	       (self)->var_data._d_int128[4] = __hybrid_int128_getword8(v, 4), (self)->var_data._d_int128[5] = __hybrid_int128_getword8(v, 5),     \
	       (self)->var_data._d_int128[6] = __hybrid_int128_getword8(v, 6), (self)->var_data._d_int128[7] = __hybrid_int128_getword8(v, 7),     \
	       (self)->var_data._d_int128[8] = __hybrid_int128_getword8(v, 8), (self)->var_data._d_int128[9] = __hybrid_int128_getword8(v, 9),     \
	       (self)->var_data._d_int128[10] = __hybrid_int128_getword8(v, 10), (self)->var_data._d_int128[11] = __hybrid_int128_getword8(v, 11), \
	       (self)->var_data._d_int128[12] = __hybrid_int128_getword8(v, 12), (self)->var_data._d_int128[13] = __hybrid_int128_getword8(v, 13), \
	       (self)->var_data._d_int128[14] = __hybrid_int128_getword8(v, 14), (self)->var_data._d_int128[15] = __hybrid_int128_getword8(v, 15))
#define _Dee_variant_set_uint128(self, v)                                                                                                          \
	(void)((self)->var_data._d_uint128[0] = __hybrid_uint128_getword8(v, 0), (self)->var_data._d_uint128[1] = __hybrid_uint128_getword8(v, 1),     \
	       (self)->var_data._d_uint128[2] = __hybrid_uint128_getword8(v, 2), (self)->var_data._d_uint128[3] = __hybrid_uint128_getword8(v, 3),     \
	       (self)->var_data._d_uint128[4] = __hybrid_uint128_getword8(v, 4), (self)->var_data._d_uint128[5] = __hybrid_uint128_getword8(v, 5),     \
	       (self)->var_data._d_uint128[6] = __hybrid_uint128_getword8(v, 6), (self)->var_data._d_uint128[7] = __hybrid_uint128_getword8(v, 7),     \
	       (self)->var_data._d_uint128[8] = __hybrid_uint128_getword8(v, 8), (self)->var_data._d_uint128[9] = __hybrid_uint128_getword8(v, 9),     \
	       (self)->var_data._d_uint128[10] = __hybrid_uint128_getword8(v, 10), (self)->var_data._d_uint128[11] = __hybrid_uint128_getword8(v, 11), \
	       (self)->var_data._d_uint128[12] = __hybrid_uint128_getword8(v, 12), (self)->var_data._d_uint128[13] = __hybrid_uint128_getword8(v, 13), \
	       (self)->var_data._d_uint128[14] = __hybrid_uint128_getword8(v, 14), (self)->var_data._d_uint128[15] = __hybrid_uint128_getword8(v, 15))
#ifdef __hybrid_unaligned_get128
#define _Dee_variant_get_int128(self)  (Dee_int128_t)__hybrid_unaligned_get128((self)->var_data._d_int128)
#define _Dee_variant_get_uint128(self) __hybrid_unaligned_get128((self)->var_data._d_uint128)
#else /* __hybrid_unaligned_get128 */
#define _Dee_variant_get_int128(self) _Dee_variant_get_int128(self)
LOCAL WUNUSED NONNULL((1)) Dee_int128_t
(DCALL _Dee_variant_get_int128)(struct Dee_variant const *__restrict self) {
	Dee_int128_t result;
	unsigned int i;
	for (i = 0; i < 16; ++i)
		__hybrid_int128_setword8(result, i, (self)->var_data._d_int128[i]);
	return result;
}
#define _Dee_variant_get_uint128(self) _Dee_variant_get_uint128(self)
LOCAL WUNUSED NONNULL((1)) Dee_uint128_t
(DCALL _Dee_variant_get_uint128)(struct Dee_variant const *__restrict self) {
	Dee_uint128_t result;
	unsigned int i;
	for (i = 0; i < 16; ++i)
		__hybrid_uint128_setword8(result, i, (self)->var_data._d_uint128[i]);
	return result;
}
#endif /* !__hybrid_unaligned_get128 */
#endif /* __ALIGNOF_INT128__ > __ALIGNOF_POINTER__ */
#ifndef CONFIG_NO_FPU
#if __ALIGNOF_DOUBLE__ <= __ALIGNOF_POINTER__
#define _Dee_variant_set_float(self, v) \
	(void)((self)->var_data.d_float = (v))
#define _Dee_variant_get_float(self) (self)->var_data.d_float
#else /* __ALIGNOF_DOUBLE__ <= __ALIGNOF_POINTER__ */
#define _Dee_variant_set_float(self, v) _Dee_variant_set_float(self, v)
LOCAL NONNULL((1)) void (DCALL _Dee_variant_set_float)(struct Dee_variant *__restrict self, double v) {
	union {
		double f;
		__BYTE_TYPE__ b[__SIZEOF_DOUBLE__];
	} data;
	size_t i;
	data.f = v;
	for (i = 0; i < COMPILER_LENOF(data.b); ++i)
		self->var_data._d_float[i] = data.b[i];
}
#define _Dee_variant_get_float(self) _Dee_variant_get_float(self)
LOCAL WUNUSED NONNULL((1)) double (DCALL _Dee_variant_get_float)(struct Dee_variant const *__restrict self) {
	union {
		double f;
		__BYTE_TYPE__ b[__SIZEOF_DOUBLE__];
	} data;
	size_t i;
	for (i = 0; i < COMPILER_LENOF(data.b); ++i)
		data.b[i] = self->var_data._d_float[i];
	return data.f;
}
#endif /* __ALIGNOF_DOUBLE__ > __ALIGNOF_POINTER__ */
#endif /* !CONFIG_NO_FPU */


/* Helpers for working with "struct Dee_variant" */
#define Dee_variant_init_unbound(self)    (void)((self)->var_type = Dee_VARIANT_UNBOUND)
#define Dee_variant_init_object(self, v)  ((self)->var_type = Dee_VARIANT_OBJECT, (self)->var_data.d_object = (v), Dee_Incref((self)->var_data.d_object))
#define Dee_variant_init_object_inherited(self, v)  ((self)->var_type = Dee_VARIANT_OBJECT, (self)->var_data.d_object = (v))
#define Dee_variant_init_int32(self, v)   (void)((self)->var_type = Dee_VARIANT_INT32, _Dee_variant_set_int32(self, v))
#define Dee_variant_init_uint32(self, v)  (void)((self)->var_type = Dee_VARIANT_UINT32, _Dee_variant_set_uint32(self, v))
#define Dee_variant_init_int64(self, v)   (void)((self)->var_type = Dee_VARIANT_INT64, _Dee_variant_set_int64(self, v))
#define Dee_variant_init_uint64(self, v)  (void)((self)->var_type = Dee_VARIANT_UINT64, _Dee_variant_set_uint64(self, v))
#define Dee_variant_init_int128(self, v)  (void)((self)->var_type = Dee_VARIANT_INT128, _Dee_variant_set_int128(self, v))
#define Dee_variant_init_uint128(self, v) (void)((self)->var_type = Dee_VARIANT_UINT128, _Dee_variant_set_uint128(self, v))
#ifndef CONFIG_NO_FPU
#define Dee_variant_init_float(self, v) (void)((self)->var_type = Dee_VARIANT_FLOAT, _Dee_variant_set_float(self, v))
#endif /* !CONFIG_NO_FPU */

#define Dee_variant_fini(self)                       \
	(void)((self)->var_type != Dee_VARIANT_OBJECT || \
	       (Dee_Decref((self)->var_data.d_object), 0))
#define Dee_variant_visit(self)                      \
	(void)((self)->var_type != Dee_VARIANT_OBJECT || \
	       (Dee_Visit((self)->var_data.d_object), 0))
#define Dee_variant_gettype(self)           ((enum Dee_variant_type)Dee_atomic_read((int *)&(self)->var_type))
#define Dee_variant_isbound(self)           (Dee_variant_gettype(self) != Dee_VARIANT_UNBOUND)
#define Dee_variant_gettype_nonatomic(self) (self)->var_type
#define Dee_variant_isbound_nonatomic(self) (Dee_variant_gettype_nonatomic(self) != Dee_VARIANT_UNBOUND)

/* Initialize "self" as a copy of "other" */
DFUNDEF NONNULL((1, 2)) void DCALL
Dee_variant_init_copy(struct Dee_variant *__restrict self,
                      struct Dee_variant *__restrict other);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_init_deepcopy(struct Dee_variant *__restrict self,
                          struct Dee_variant *__restrict other);


/* Get the value of a variant in the form of a deemon object.
 * If the variant's type isn't set to "Dee_VARIANT_OBJECT", the
 * linked object is lazily allocated and assigned
 * @return: ITER_DONE: [Dee_variant_trygetobject] Returned if the variant is unbound
 * @return: NULL:      An error was thrown (`Dee_variant_getobject()' throws an UnboundAttribute error if "self" is unbound) */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_variant_trygetobject(struct Dee_variant *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
Dee_variant_getobject(struct Dee_variant *__restrict self,
                      DeeObject *owner, char const *attr);

/* Set the value of a variant (these can never fail) */
DFUNDEF NONNULL((1)) void DCALL Dee_variant_setunbound(struct Dee_variant *__restrict self);
DFUNDEF NONNULL((1, 2)) void DCALL Dee_variant_setobject(struct Dee_variant *__restrict self, DeeObject *value);
DFUNDEF NONNULL((1)) void DCALL Dee_variant_setint32(struct Dee_variant *__restrict self, int32_t value);
DFUNDEF NONNULL((1)) void DCALL Dee_variant_setuint32(struct Dee_variant *__restrict self, uint32_t value);
DFUNDEF NONNULL((1)) void DCALL Dee_variant_setint64(struct Dee_variant *__restrict self, int64_t value);
DFUNDEF NONNULL((1)) void DCALL Dee_variant_setuint64(struct Dee_variant *__restrict self, uint64_t value);
DFUNDEF NONNULL((1)) void DCALL Dee_variant_setint128(struct Dee_variant *__restrict self, Dee_int128_t value);
DFUNDEF NONNULL((1)) void DCALL Dee_variant_setuint128(struct Dee_variant *__restrict self, Dee_uint128_t value);

/* Print the DeeObject_Str() or DeeObject_Repr() of the object linked to "self"
 * When the variant "self" is unbound, nothing is printed and "0" is returned.
 * If you want to (safely) implement custom handling for unbound variants, you
 * should wrap these calls like this:
 * >> struct Dee_variant copy;
 * >> Dee_variant_init_copy(&copy, VARIANT_TO_PRINT);
 * >> if (Dee_variant_isbound_nonatomic(&copy)) {
 * >>     result = Dee_variant_print(&copy, printer, arg);
 * >> } else {
 * >>     result = DeeFormat_PRINT(printer, arg, "<UNBOUND>");
 * >> }
 * >> Dee_variant_fini(&copy); */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_variant_print(struct Dee_variant *__restrict self,
                  Dee_formatprinter_t printer, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_variant_printrepr(struct Dee_variant *__restrict self,
                      Dee_formatprinter_t printer, void *arg);

/* Compare variants with each other. */
DFUNDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL Dee_variant_hash(struct Dee_variant *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_variant_compare(struct Dee_variant *lhs, struct Dee_variant *rhs);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_variant_compare_eq(struct Dee_variant *lhs, struct Dee_variant *rhs);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_variant_trycompare_eq(struct Dee_variant *lhs, struct Dee_variant *rhs);

DECL_END

#endif /* !GUARD_DEEMON_VARIANT_H */
