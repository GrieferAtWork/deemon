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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_RT_C
#define GUARD_DEEMON_RUNTIME_ERROR_RT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/variant.h>

#include <hybrid/int128.h>
/**/

#include "kwlist.h"
#include "runtime_error.h"
#include "strings.h"
/**/

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

#ifndef CONFIG_NO_OBJECT_SLABS
LOCAL ATTR_CONST WUNUSED NONNULL((1)) size_t DCALL
get_slab_size(void (DCALL *tp_free)(void *__restrict ob)) {
#define CHECK_SIZE(index, size)                 \
	if (tp_free == &DeeObject_SlabFree##size || \
	    tp_free == &DeeGCObject_SlabFree##size) \
		return size * __SIZEOF_POINTER__;
	DeeSlab_ENUMERATE(CHECK_SIZE)
#undef CHECK_SIZE
	return 0;
}

#define GET_INSTANCE_SIZE(self)                        \
	((self)->tp_init.tp_alloc.tp_free                  \
	 ? get_slab_size((self)->tp_init.tp_alloc.tp_free) \
	 : (self)->tp_init.tp_alloc.tp_instance_size)
#else /* !CONFIG_NO_OBJECT_SLABS */
#define GET_INSTANCE_SIZE(self) \
	((self)->tp_init.tp_alloc.tp_instance_size)
#endif /* CONFIG_NO_OBJECT_SLABS */

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
error_printrepr(DeeErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg);





PRIVATE DeeErrorObject RT_ErrNoActiveException = {
	OBJECT_HEAD_INIT(&DeeError_RuntimeError),
	/* .e_message = */ (DeeStringObject *)&str_No_active_exception,
	/* .e_inner   = */ NULL,
};

PUBLIC ATTR_COLD int (DCALL DeeRT_ErrNoActiveException)(void) {
	return DeeError_Throw((DeeObject *)&RT_ErrNoActiveException);
}




/* Throws a `DeeError_IntegerOverflow' indicating that some an integer
 * object or native (C) value cannot be used/processed because its value
 * exceeds the maximum supported value bounds within some context-of-use.
 *
 * The unsigned overflow throwing functions will only take the upper
 * bound (greatest) of valid values, and assume that the lower bound
 * is equal to `0'
 *
 * @param: positive: When true, assume "value > maxval".
 *                   Else, assume "value < maxval" */

/************************************************************************/
/* General-purpose, parameterized errors                                */
/************************************************************************/
typedef struct {
	ERROR_OBJECT_HEAD
	COMPILER_FLEXIBLE_ARRAY(struct Dee_variant, io_params); /* [...] Error parameters */
} ParameterizedError;
#define ParameterizedErrorType_GetParamCount(self) \
	((GET_INSTANCE_SIZE(self) - offsetof(ParameterizedError, io_params)) / sizeof(struct Dee_variant))
#define ParameterizedError_GetParamCount(self) \
	ParameterizedErrorType_GetParamCount(Dee_TYPE(self))

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ParameterizedError_Copy(ParameterizedError *__restrict self,
                        ParameterizedError *__restrict other) {
	size_t i, n_params = ParameterizedError_GetParamCount(self);
	for (i = 0; i < n_params; ++i) {
		Dee_variant_init_copy(&self->io_params[i],
		                      &other->io_params[i]);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ParameterizedError_Deep(ParameterizedError *__restrict self,
                        ParameterizedError *__restrict other) {
	size_t i, n_params = ParameterizedError_GetParamCount(self);
	for (i = 0; i < n_params; ++i) {
		if unlikely(Dee_variant_init_deepcopy(&self->io_params[i],
		                                      &other->io_params[i]))
			goto err_i;
	}
	return 0;
err_i:
	while (i--)
		Dee_variant_fini(&self->io_params[i]);
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
ParameterizedError_Fini(ParameterizedError *__restrict self) {
	size_t i, n_params = ParameterizedError_GetParamCount(self);
	for (i = 0; i < n_params; ++i)
		Dee_variant_fini(&self->io_params[i]);
}

PRIVATE NONNULL((1, 2)) void DCALL
ParameterizedError_Visit(ParameterizedError *__restrict self,
                         Dee_visit_t proc, void *arg) {
	size_t i, n_params = ParameterizedError_GetParamCount(self);
	for (i = 0; i < n_params; ++i)
		Dee_variant_visit(&self->io_params[i]);
}

#define PARAMETERIZED_ERROR_TYPE_INIT(tp_name, tp_base, T,                               \
                                      tp_print, tp_printrepr,                            \
                                      tp_methods, tp_getsets, tp_members,                \
                                      tp_class_methods, tp_class_members)                \
	{                                                                                    \
		OBJECT_HEAD_INIT(&DeeType_Type),                                                 \
		/* .tp_name     = */ tp_name,                                                    \
		/* .tp_doc      = */ NULL,                                                       \
		/* .tp_flags    = */ TP_FINHERITCTOR,                                            \
		/* .tp_weakrefs = */ 0,                                                          \
		/* .tp_features = */ TF_NONE,                                                    \
		/* .tp_base     = */ tp_base,                                                    \
		/* .tp_init = */ {                                                               \
			{                                                                            \
				/* .tp_alloc = */ {                                                      \
					/* .tp_ctor      = */ (Dee_funptr_t)NULL, /* Inherited */            \
					/* .tp_copy_ctor = */ (Dee_funptr_t)&ParameterizedError_Copy,        \
					/* .tp_deep_ctor = */ (Dee_funptr_t)&ParameterizedError_Deep,        \
					/* .tp_any_ctor  = */ (Dee_funptr_t)NULL, /* Inherited */            \
					TYPE_FIXED_ALLOCATOR(T)                                              \
				}                                                                        \
			},                                                                           \
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ParameterizedError_Fini, \
			/* .tp_assign      = */ NULL,                                                \
			/* .tp_move_assign = */ NULL                                                 \
		},                                                                               \
		/* .tp_cast = */ {                                                               \
			/* .tp_str       = */ NULL,                                                  \
			/* .tp_repr      = */ NULL,                                                  \
			/* .tp_bool      = */ NULL,                                                  \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_printrepr) \
		},                                                                               \
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ParameterizedError_Visit, \
		/* .tp_gc            = */ NULL,                                                  \
		/* .tp_math          = */ NULL,                                                  \
		/* .tp_cmp           = */ NULL,                                                  \
		/* .tp_seq           = */ NULL,                                                  \
		/* .tp_iter_next     = */ NULL,                                                  \
		/* .tp_iterator      = */ NULL,                                                  \
		/* .tp_attr          = */ NULL,                                                  \
		/* .tp_with          = */ NULL,                                                  \
		/* .tp_buffer        = */ NULL,                                                  \
		/* .tp_methods       = */ tp_methods,                                            \
		/* .tp_getsets       = */ tp_getsets,                                            \
		/* .tp_members       = */ tp_members,                                            \
		/* .tp_class_methods = */ tp_class_methods,                                      \
		/* .tp_class_getsets = */ NULL,                                                  \
		/* .tp_class_members = */ tp_class_members                                       \
	}





typedef struct {
	ERROR_OBJECT_HEAD
	struct Dee_variant io_value;    /* Value that's overflowing */
	struct Dee_variant io_minval;   /* Min valid value */
	struct Dee_variant io_maxval;   /* Max valid value */
	bool               io_positive; /* Is the overflow positive? */
} IntegerOverflow;

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
integer_overflow_print(IntegerOverflow *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	if (self->e_message)
		return DeeString_PrintUtf8((DeeObject *)self->e_message, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "%s integer overflow: %Vk exceeds range of valid values [%Vk,%Vk]",
	                        self->io_positive ? "positive" : "negative",
	                        &self->io_value, &self->io_minval, &self->io_maxval);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
integer_overflow_printrepr(IntegerOverflow *__restrict self,
                           Dee_formatprinter_t printer, void *arg) {
	if (!Dee_variant_isbound_nonatomic(&self->io_value))
		return error_printrepr((DeeErrorObject *)self, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "IntegerOverflow.of(value: %Vr, minval: %Vr, maxval: %Vr, positive: %s)",
	                        &self->io_value, &self->io_minval, &self->io_maxval,
	                        self->io_positive ? "true" : "false");
}

PRIVATE WUNUSED NONNULL((1)) DREF IntegerOverflow *DCALL
integer_overflow_of(DeeObject *UNUSED(error_type), size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	DREF IntegerOverflow *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("of", params: "
		value:?DNumeric,
		minval:?DNumeric,
		maxval:?DNumeric,
		bool positive,
", docStringPrefix: "integer_overflow");]]]*/
#define integer_overflow_of_params "value:?DNumeric,minval:?DNumeric,maxval:?DNumeric,positive:?Dbool"
	struct {
		DeeObject *value;
		DeeObject *minval;
		DeeObject *maxval;
		bool positive;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value_minval_maxval_positive, "ooob:of", &args))
		goto err;
/*[[[end]]]*/
	result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_variant_init_object(&result->io_value, args.value);
	Dee_variant_init_object(&result->io_minval, args.minval);
	Dee_variant_init_object(&result->io_maxval, args.maxval);
	result->io_positive = args.positive;
	DeeObject_Init(result, &DeeError_IntegerOverflow);
	return result;
err:
	return NULL;
}

PRIVATE struct type_method tpconst integer_overflow_class_methods[] = {
	TYPE_KWMETHOD("of", &integer_overflow_of, "(" integer_overflow_of_params ")->?."),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst integer_overflow_members[] = {
	TYPE_MEMBER_FIELD_DOC("value", STRUCT_VARIANT | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_value),
	                      "->?DNumeric\n"
	                      "Value that is overflowing"),
	TYPE_MEMBER_FIELD_DOC("minval", STRUCT_VARIANT | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_minval),
	                      "->?DNumeric\n"
	                      "Smallest acceptable value (?#value is either less than this, or greater than ?#maxval)"),
	TYPE_MEMBER_FIELD_DOC("maxval", STRUCT_VARIANT | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_maxval),
	                      "->?DNumeric\n"
	                      "Greatest acceptable value (?#value is either greater than this, or less than ?#minval)"),
	TYPE_MEMBER_FIELD_DOC("positive", STRUCT_BOOL(__SIZEOF_BOOL__) | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_positive),
	                      "If true, ?#value is greater than ?#maxval, else it is less than ?#minval"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_IntegerOverflow =
PARAMETERIZED_ERROR_TYPE_INIT("IntegerOverflow", &DeeError_ArithmeticError,
                              IntegerOverflow,
                              &integer_overflow_print,
                              &integer_overflow_printrepr,
                              NULL, NULL, integer_overflow_members,
                              integer_overflow_class_methods, NULL);

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrIntegerOverflow)(/*Numeric*/ DeeObject *value,
                                 /*Numeric*/ DeeObject *minval,
                                 /*Numeric*/ DeeObject *maxval,
                                 bool positive) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_variant_init_object(&result->io_value, value);
	Dee_variant_init_object(&result->io_minval, minval);
	Dee_variant_init_object(&result->io_maxval, maxval);
	result->io_positive = positive;
	DeeObject_Init(result, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

/* Same as "DeeRT_ErrIntegerOverflow", but minval/maxval are set as
 * >> minval   = (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) ? -(1 << (num_bits - 1)) : 0;
 * >> maxval   = (flags & DeeRT_ErrIntegerOverflowEx_F_SIGNED) ? ((1 << (num_bits - 1)) - 1) : ((1 << num_bits) - 1);
 * >> positive = (flags & DeeRT_ErrIntegerOverflowEx_F_POSITIVE) != 0; */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrIntegerOverflowEx)(/*Numeric*/ DeeObject *value,
                                   size_t num_bits,
                                   unsigned int flags) {
	size_t max_bits;
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->e_message = NULL;
	result->e_inner   = NULL;
	max_bits = num_bits;
	if (flags & DeeRT_ErrIntegerOverflowEx_F_SIGNED)
		--max_bits;
	Dee_variant_init_uint32(&result->io_minval, 0);
	if (num_bits <= 32) {
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN))
			Dee_variant_init_int32(&result->io_minval, -((int32_t)1 << (num_bits - 1)));
		Dee_variant_init_uint32(&result->io_maxval, ((uint32_t)1 << max_bits) - 1);
	} else if (num_bits <= 64) {
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN))
			Dee_variant_init_int64(&result->io_minval, -((int64_t)1 << (num_bits - 1)));
		Dee_variant_init_uint64(&result->io_maxval, ((uint64_t)1 << max_bits) - 1);
	} else if (num_bits <= 128) {
		Dee_uint128_t maxval;
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) {
			Dee_int128_t minval;
			__hybrid_int128_setone(minval);
			__hybrid_int128_shl(minval, num_bits - 1);
			__hybrid_int128_neg(minval);
			Dee_variant_init_int128(&result->io_minval, minval);
		}
		__hybrid_uint128_setone(maxval);
		__hybrid_uint128_shl(maxval, max_bits);
		__hybrid_uint128_dec(maxval);
		Dee_variant_init_uint128(&result->io_maxval, maxval);
	} else {
		DREF DeeObject *maxval, *shift;
		shift = DeeInt_NewSize(max_bits);
		if unlikely(!shift)
			goto err_r;
		maxval = DeeObject_Shl(DeeInt_One, shift);
		Dee_Decref(shift);
		if unlikely(!maxval)
			goto err_r;
		if unlikely(DeeObject_Dec(&maxval)) {
			Dee_Decref(maxval);
			goto err_r;
		}
		Dee_variant_init_object_inherited(&result->io_maxval, maxval);
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) {
			DREF DeeObject *temp, *minval;
			shift = DeeInt_NewSize(num_bits - 1);
			if unlikely(!shift) {
err_r_maxval:
				Dee_variant_fini(&result->io_maxval);
				goto err_r;
			}
			temp = DeeObject_Shl(DeeInt_One, shift);
			Dee_Decref(shift);
			if unlikely(!temp)
				goto err_r_maxval;
			minval = DeeObject_Neg(temp);
			Dee_Decref(temp);
			if unlikely(!minval)
				goto err_r_maxval;
			Dee_variant_init_object_inherited(&result->io_minval, minval);
		}
	}
	Dee_variant_init_object(&result->io_value, value);
	result->io_positive = (flags & DeeRT_ErrIntegerOverflowEx_F_POSITIVE) != 0;
	DeeObject_Init(result, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}


#if __SIZEOF_SIZE_T__ < 8
PRIVATE ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflow32_impl)(int32_t value,
                                        int32_t minval,
                                        int32_t maxval,
                                        bool is_signed) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_variant_init_int32(&result->io_value, value);
	Dee_variant_init_int32(&result->io_minval, minval);
	Dee_variant_init_int32(&result->io_maxval, maxval);
	result->io_positive = value > maxval;
	if (!is_signed) {
		result->io_value.var_type  = Dee_VARIANT_UINT32;
		result->io_minval.var_type = Dee_VARIANT_UINT32;
		result->io_maxval.var_type = Dee_VARIANT_UINT32;
		result->io_positive = true;
	}
	DeeObject_Init(result, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}
#endif /* __SIZEOF_SIZE_T__ < 8 */

PRIVATE ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflow64_impl)(int64_t value,
                                        int64_t minval,
                                        int64_t maxval,
                                        bool is_signed) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_variant_init_int64(&result->io_value, value);
	Dee_variant_init_int64(&result->io_minval, minval);
	Dee_variant_init_int64(&result->io_maxval, maxval);
	result->io_positive = value > maxval;
	if (!is_signed) {
		result->io_value.var_type  = Dee_VARIANT_UINT64;
		result->io_minval.var_type = Dee_VARIANT_UINT64;
		result->io_maxval.var_type = Dee_VARIANT_UINT64;
		result->io_positive = true;
	}
	DeeObject_Init(result, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PRIVATE ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflow128_impl)(Dee_int128_t value,
                                         Dee_int128_t minval,
                                         Dee_int128_t maxval,
                                         bool is_signed) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_variant_init_int128(&result->io_value, value);
	Dee_variant_init_int128(&result->io_minval, minval);
	Dee_variant_init_int128(&result->io_maxval, maxval);
	result->io_positive = __hybrid_int128_gr128(value, maxval);
	if (!is_signed) {
		result->io_value.var_type  = Dee_VARIANT_UINT128;
		result->io_minval.var_type = Dee_VARIANT_UINT128;
		result->io_maxval.var_type = Dee_VARIANT_UINT128;
		result->io_positive = true;
	}
	DeeObject_Init(result, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowS)(Dee_ssize_t value,
                                  Dee_ssize_t minval,
                                  Dee_ssize_t maxval) {
#if __SIZEOF_SIZE_T__ < 8
	return DeeRT_ErrIntegerOverflow32_impl(value, minval, maxval, true);
#else /* __SIZEOF_SIZE_T__ < 8 */
	return DeeRT_ErrIntegerOverflow64_impl(value, minval, maxval, true);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowU)(size_t value, size_t maxval) {
#if __SIZEOF_SIZE_T__ < 8
	return DeeRT_ErrIntegerOverflow32_impl((int32_t)value, 0, (int32_t)maxval, false);
#else /* __SIZEOF_SIZE_T__ < 8 */
	return DeeRT_ErrIntegerOverflow64_impl((int64_t)value, 0, (int64_t)maxval, false);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}

#if __SIZEOF_SIZE_T__ < 8
PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowS64)(int64_t value,
                                    int64_t minval,
                                    int64_t maxval) {
	return DeeRT_ErrIntegerOverflow64_impl(value, minval, maxval, true);
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowU64)(uint64_t value, uint64_t maxval) {
	return DeeRT_ErrIntegerOverflow64_impl((int64_t)value, 0, (int64_t)maxval, false);
}
#endif /* __SIZEOF_SIZE_T__ < 8 */

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowS128)(Dee_int128_t value,
                                     Dee_int128_t minval,
                                     Dee_int128_t maxval) {
	return DeeRT_ErrIntegerOverflow128_impl(value, minval, maxval, true);
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowU128)(Dee_uint128_t value,
                                     Dee_uint128_t maxval) {
	Dee_int128_t used_value;
	Dee_int128_t used_minval;
	Dee_int128_t used_maxval;
	memcpy(&used_value, &value, sizeof(used_value));
	__hybrid_int128_setzero(used_minval);
	memcpy(&used_maxval, &maxval, sizeof(used_maxval));
	return DeeRT_ErrIntegerOverflow128_impl(used_value,
	                                        used_minval,
	                                        used_maxval,
	                                        false);
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowUMul)(size_t lhs, size_t rhs) {
#if __SIZEOF_SIZE_T__ < 8
	uint64_t value = (uint64_t)lhs * (uint64_t)rhs;
	return DeeRT_ErrIntegerOverflowU64(value, SIZE_MAX);
#else /* __SIZEOF_SIZE_T__ < 8 */
	Dee_uint128_t value, maxval;
	__hybrid_uint128_set(value, lhs);
	__hybrid_uint128_mul(value, rhs);
	__hybrid_uint128_set(maxval, SIZE_MAX);
	return DeeRT_ErrIntegerOverflowU128(value, maxval);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowUAdd)(size_t lhs, size_t rhs) {
#if __SIZEOF_SIZE_T__ < 8
	uint64_t value = (uint64_t)lhs + (uint64_t)rhs;
	return DeeRT_ErrIntegerOverflowU64(value, SIZE_MAX);
#else /* __SIZEOF_SIZE_T__ < 8 */
	Dee_uint128_t value, maxval;
	__hybrid_uint128_set(value, lhs);
	__hybrid_uint128_add(value, rhs);
	__hybrid_uint128_set(maxval, SIZE_MAX);
	return DeeRT_ErrIntegerOverflowU128(value, maxval);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}



/* TODO: Custom handling for: DeeError_IndexError (include attributes for relevant sequence / index) */
/* TODO: Custom handling for: DeeError_UnboundItem */


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ERROR_RT_C */
