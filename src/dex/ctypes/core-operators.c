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
#ifndef GUARD_DEX_CTYPES_CORE_OPERATORS_C
#define GUARD_DEX_CTYPES_CORE_OPERATORS_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#include <hybrid/limitcore.h> /* __INT32_MAX__, __INT32_MIN__, __INT64_MAX__, __INT64_MIN__, __UINT32_MAX__, __UINT64_MAX__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* INT32_MAX, INT32_MIN, INT64_MAX, INT64_MIN, UINT32_MAX, UINT64_MAX, int32_t, int64_t, uint32_t, uint64_t */

#ifndef INT32_MIN
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */
#ifndef INT32_MAX
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */
#ifndef UINT32_MAX
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */
#ifndef INT64_MIN
#define INT64_MIN __INT64_MIN__
#endif /* !INT64_MIN */
#ifndef INT64_MAX
#define INT64_MAX __INT64_MAX__
#endif /* !INT64_MAX */
#ifndef UINT64_MAX
#define UINT64_MAX __UINT64_MAX__
#endif /* !UINT64_MAX */

/* Trace self-optimizing operator inheritance. */
#if 1
#define LOG_INHERIT(base, self, what) \
	Dee_DPRINTF("[RT] Inherit `" what "' from %s into %s\n", DeeType_GetName(base), DeeType_GetName(self))
#else
#define LOG_INHERIT(base, self, what) (void)0
#endif

DECL_BEGIN

#define OPNAME(opname) "operator " opname

#define DEFINE_OPERATOR_INVOKE(name, instance_name, inherit_name)                             \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                     \
	invoke_##name(DeeSTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self, \
	              size_t argc, DeeObject *const *argv, Dee_operator_t opname);                \
	PRIVATE struct Dee_operator_invoke tpconst name =                                         \
	Dee_OPERATOR_INVOKE_INIT(&invoke_##name, instance_name, inherit_name);                    \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                     \
	invoke_##name(DeeSTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self, \
	              size_t argc, DeeObject *const *argv, Dee_operator_t opname)

/* >> operator cinit(args: Tuple); */
DEFINE_OPERATOR_INVOKE(stype_operator_init, NULL, NULL) {
	DeeObject *args;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cinit"), &args);
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if unlikely((*tp_self->st_init)(tp_self, DeeStruct_Data(self),
	                                DeeTuple_SIZE(args),
	                                DeeTuple_ELEM(args)))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator cassign(value: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_assign, NULL, NULL) {
	DeeObject *value;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cassign"), &value);
	if unlikely((*tp_self->st_assign)(tp_self, DeeStruct_Data(self), value))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator cstr(): string; */
DEFINE_OPERATOR_INVOKE(stype_operator_str, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cstr"));
	return (*tp_self->st_cast.st_str)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator crepr(): string; */
DEFINE_OPERATOR_INVOKE(stype_operator_repr, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("crepr"));
	return (*tp_self->st_cast.st_repr)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator cbool(): bool; */
DEFINE_OPERATOR_INVOKE(stype_operator_bool, NULL, NULL) {
	int result;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cbool"));
	result = (*tp_self->st_cast.st_bool)(tp_self, DeeStruct_Data(self));
	if unlikely(result < 0)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}

/* >> operator ccall(args: Tuple): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_call, NULL, NULL) {
	DeeObject *args;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ccall"), &args);
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	return (*tp_self->st_call)(tp_self, DeeStruct_Data(self),
	                           DeeTuple_SIZE(args),
	                           DeeTuple_ELEM(args));
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_int32_with_int(DeeSTypeObject *tp_self, void *self, int32_t *result) {
	int status;
	DREF DeeObject *temp;
	temp = (*tp_self->st_math->st_int)(tp_self, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get32Bit(temp, result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_int64_with_int(DeeSTypeObject *tp_self, void *self, int64_t *result) {
	int status;
	DREF DeeObject *temp;
	temp = (*tp_self->st_math->st_int)(tp_self, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get64Bit(temp, result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_double_with_int(DeeSTypeObject *tp_self, void *self, double *result) {
	int status;
	DREF DeeObject *temp;
	temp = (*tp_self->st_math->st_int)(tp_self, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_AsDouble(temp, result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_int_with_int32(DeeSTypeObject *tp_self, void *self) {
	int32_t value;
	int status = (*tp_self->st_math->st_int32)(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt32((uint32_t)value);
	return DeeInt_NewInt32(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_int64_with_int32(DeeSTypeObject *tp_self, void *self, int64_t *result) {
	int32_t value;
	int status = (*tp_self->st_math->st_int32)(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*result = (int64_t)(uint64_t)(uint32_t)value;
	} else {
		*result = (int64_t)value;
	}
	return status;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_double_with_int32(DeeSTypeObject *tp_self, void *self, double *result) {
	int32_t value;
	int status = (*tp_self->st_math->st_int32)(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*result = (double)(uint32_t)value;
	} else {
		*result = (double)value;
	}
	return status;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_int_with_int64(DeeSTypeObject *tp_self, void *self) {
	int64_t value;
	int status = (*tp_self->st_math->st_int64)(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt64((uint64_t)value);
	return DeeInt_NewInt64(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_int32_with_int64(DeeSTypeObject *tp_self, void *self, int32_t *result) {
	int64_t value;
	int status = (*tp_self->st_math->st_int64)(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		if ((uint64_t)value > UINT32_MAX)
			goto err_overflow;
		*result = (int32_t)(uint32_t)(uint64_t)value;
	} else {
		if ((value < INT32_MIN || value > INT32_MAX) && status == Dee_INT_SIGNED)
			goto err_overflow;
		*result = (int32_t)value;
	}
	return status;
err_overflow:
	return DeeError_Throwf(&DeeError_IntegerOverflow,
	                       "%s integer overflow after 32 bits",
	                       status == Dee_INT_SIGNED ? "positive" : "negative");
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_double_with_int64(DeeSTypeObject *tp_self, void *self, double *result) {
	int64_t value;
	int status = (*tp_self->st_math->st_int64)(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*result = (double)(uint64_t)value;
	} else {
		*result = (double)value;
	}
	return status;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_int_with_double(DeeSTypeObject *tp_self, void *self) {
	double value;
	int status = (*tp_self->st_math->st_double)(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	return DeeInt_NewDouble(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_int32_with_double(DeeSTypeObject *tp_self, void *self, int32_t *result) {
	double value;
	int status = (*tp_self->st_math->st_double)(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT32_MIN)
		goto err_overflow;
	if (value > INT32_MAX) {
		if (value <= UINT32_MAX) {
			*result = (int32_t)(uint32_t)value;
			return Dee_INT_UNSIGNED;
		}
		goto err_overflow;
	}
	*result = (int32_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	DeeError_Throwf(&DeeError_IntegerOverflow,
	                "%s integer overflow after 32 bits",
	                value >= 0 ? "positive" : "negative");
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
stype_int64_with_double(DeeSTypeObject *tp_self, void *self, int64_t *result) {
	double value;
	int status = (*tp_self->st_math->st_double)(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT64_MIN)
		goto err_overflow;
	if (value > INT64_MAX) {
		if (value <= UINT64_MAX) {
			*result = (int64_t)(uint64_t)value;
			return Dee_INT_UNSIGNED;
		}
		goto err_overflow;
	}
	*result = (int64_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	DeeError_Throwf(&DeeError_IntegerOverflow,
	                "%s integer overflow after 64 bits",
	                value >= 0 ? "positive" : "negative");
err:
	return -1;
}

PRIVATE NONNULL((1)) bool DCALL
DeeSType_InheritCInt(DeeSTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	struct stype_math *math = self->st_math;
	if (math) {
		if (math->st_int) {
			if (math->st_int32 == NULL)
				math->st_int32 = &stype_int32_with_int;
			if (math->st_int64 == NULL)
				math->st_int64 = &stype_int64_with_int;
			if (math->st_double == NULL)
				math->st_double = &stype_double_with_int;
			return true;
		} else if (math->st_double) {
			if (math->st_int32 == NULL)
				math->st_int32 = &stype_int32_with_double;
			if (math->st_int64 == NULL)
				math->st_int64 = &stype_int64_with_double;
			if (math->st_int == NULL)
				math->st_int = &stype_int_with_double;
			return true;
		} else if (math->st_int64) {
			if (math->st_int32 == NULL)
				math->st_int32 = &stype_int32_with_int64;
			if (math->st_double == NULL)
				math->st_double = &stype_double_with_int64;
			if (math->st_int == NULL)
				math->st_int = &stype_int_with_int64;
			return true;
		} else if (math->st_int32) {
			if (math->st_int64 == NULL)
				math->st_int64 = &stype_int64_with_int32;
			if (math->st_double == NULL)
				math->st_double = &stype_double_with_int32;
			if (math->st_int == NULL)
				math->st_int = &stype_int_with_int32;
			return true;
		}
	}

	base = DeeTypeMRO_Init(&mro, DeeSType_AsType(self));
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		struct stype_math *base_math;
		DeeSTypeObject *sbase;
		if (!DeeSType_Check(base))
			continue;
		sbase = DeeType_AsSType(base);
		base_math = sbase->st_math;
		if (base_math == NULL ||
		    (!base_math->st_int && !base_math->st_int32 &&
		     !base_math->st_int64 && !base_math->st_double)) {
			if (!DeeSType_InheritCInt(sbase))
				continue;
			base_math = sbase->st_math;
		}
		LOG_INHERIT(DeeSType_AsType(sbase), DeeSType_AsType(self), "operator cint");
		if (self->st_math != NULL) {
			self->st_math->st_int32  = base_math->st_int32;
			self->st_math->st_int64  = base_math->st_int64;
			self->st_math->st_int    = base_math->st_int;
			self->st_math->st_double = base_math->st_double;
		} else {
			self->st_math = base_math;
		}
		return true;
	}
	return false;
}

PRIVATE NONNULL((1)) void DCALL
do_DeeSType_InheritCInt(DeeSTypeObject *__restrict self,
                        DeeTypeObject *_unused_type_type,
                        struct Dee_opinfo const *_unused_info) {
	(void)_unused_type_type;
	(void)_unused_info;
	DeeSType_InheritCInt(self);
}

/* >> operator cint(): int; */
DEFINE_OPERATOR_INVOKE(stype_operator_int, NULL, &do_DeeSType_InheritCInt) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cbool"));
	return (*tp_self->st_math->st_int)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator cdouble(): float; */
DEFINE_OPERATOR_INVOKE(stype_operator_double, NULL, &do_DeeSType_InheritCInt) {
	double value;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cdouble"));
	if unlikely((*tp_self->st_math->st_double)(tp_self, DeeStruct_Data(self), &value))
		goto err;
	return DeeFloat_New(value);
err:
	return NULL;
}

/* >> operator cinv(): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_inv, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cinv"));
	return (*tp_self->st_math->st_inv)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator cpos(): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_pos, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cpos"));
	return (*tp_self->st_math->st_pos)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator cneg(): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_neg, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cneg"));
	return (*tp_self->st_math->st_neg)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator cadd(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_add, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cadd"), &other);
	return (*tp_self->st_math->st_add)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator csub(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_sub, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("csub"), &other);
	return (*tp_self->st_math->st_sub)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cmul(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_mul, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cmul"), &other);
	return (*tp_self->st_math->st_mul)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cdiv(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_div, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cdiv"), &other);
	return (*tp_self->st_math->st_div)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cmod(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_mod, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cmod"), &other);
	return (*tp_self->st_math->st_mod)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cshl(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_shl, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cshl"), &other);
	return (*tp_self->st_math->st_shl)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cshr(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_shr, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cshr"), &other);
	return (*tp_self->st_math->st_shr)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cand(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_and, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cand"), &other);
	return (*tp_self->st_math->st_and)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cor(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_or, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cor"), &other);
	return (*tp_self->st_math->st_or)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cxor(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_xor, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cxor"), &other);
	return (*tp_self->st_math->st_xor)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cpow(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_pow, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cpow"), &other);
	return (*tp_self->st_math->st_pow)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cinc(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inc, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cinc"));
	if unlikely((*tp_self->st_math->st_inc)(tp_self, DeeStruct_Data(self)))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cdec(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_dec, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cdec"));
	if unlikely((*tp_self->st_math->st_dec)(tp_self, DeeStruct_Data(self)))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator ciadd(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_add, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ciadd"), &other);
	if unlikely((*tp_self->st_math->st_add)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cisub(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_sub, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cisub"), &other);
	if unlikely((*tp_self->st_math->st_sub)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cimul(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_mul, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cimul"), &other);
	if unlikely((*tp_self->st_math->st_mul)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cidiv(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_div, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cidiv"), &other);
	if unlikely((*tp_self->st_math->st_div)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cimod(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_mod, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cimod"), &other);
	if unlikely((*tp_self->st_math->st_mod)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cishl(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_shl, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cishl"), &other);
	if unlikely((*tp_self->st_math->st_shl)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cishr(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_shr, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cishr"), &other);
	if unlikely((*tp_self->st_math->st_shr)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator ciand(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_and, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ciand"), &other);
	if unlikely((*tp_self->st_math->st_and)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cior(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_or, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cior"), &other);
	if unlikely((*tp_self->st_math->st_or)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cixor(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_xor, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cixor"), &other);
	if unlikely((*tp_self->st_math->st_xor)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator cipow(other: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_inplace_pow, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cipow"), &other);
	if unlikely((*tp_self->st_math->st_pow)(tp_self, DeeStruct_Data(self), other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

/* >> operator ceq(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_eq, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ceq"), &other);
	return (*tp_self->st_cmp->st_eq)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cne(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_ne, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cne"), &other);
	return (*tp_self->st_cmp->st_ne)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator clo(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_lo, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("clo"), &other);
	return (*tp_self->st_cmp->st_lo)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cle(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_le, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cle"), &other);
	return (*tp_self->st_cmp->st_le)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cgr(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_gr, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cgr"), &other);
	return (*tp_self->st_cmp->st_gr)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cge(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_ge, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cge"), &other);
	return (*tp_self->st_cmp->st_ge)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator citer(): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_iter, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("citer"));
	return (*tp_self->st_seq->st_iter_self)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator csize(): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_size, NULL, NULL) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("csize"));
	return (*tp_self->st_seq->st_size)(tp_self, DeeStruct_Data(self));
err:
	return NULL;
}

/* >> operator ccontains(other: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_contains, NULL, NULL) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ccontains"), &other);
	return (*tp_self->st_seq->st_contains)(tp_self, DeeStruct_Data(self), other);
err:
	return NULL;
}

/* >> operator cgetitem(index: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_getitem, NULL, NULL) {
	DeeObject *index;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cgetitem"), &index);
	return (*tp_self->st_seq->st_get)(tp_self, DeeStruct_Data(self), index);
err:
	return NULL;
}

/* >> operator cdelitem(index: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_delitem, NULL, NULL) {
	DeeObject *index;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cdelitem"), &index);
	if unlikely((*tp_self->st_seq->st_del)(tp_self, DeeStruct_Data(self), index))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator csetitem(index: Object, value: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_setitem, NULL, NULL) {
	DeeObject *index, *value;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("csetitem"), &index, &value);
	if unlikely((*tp_self->st_seq->st_set)(tp_self, DeeStruct_Data(self), index, value))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator cgetrange(start: Object, end: Object): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_getrange, NULL, NULL) {
	DeeObject *start, *end;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("cgetrange"), &start, &end);
	return (*tp_self->st_seq->st_range_get)(tp_self, DeeStruct_Data(self), start, end);
err:
	return NULL;
}

/* >> operator cdelrange(start: Object, end: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_delrange, NULL, NULL) {
	DeeObject *start, *end;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("cdelrange"), &start, &end);
	if unlikely((*tp_self->st_seq->st_range_del)(tp_self, DeeStruct_Data(self), start, end))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator csetrange(start: Object, end: Object, value: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_setrange, NULL, NULL) {
	DeeObject *start, *end, *value;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack3(err, argc, argv, OPNAME("csetrange"), &start, &end, &value);
	if unlikely((*tp_self->st_seq->st_range_set)(tp_self, DeeStruct_Data(self), start, end, value))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator cgetattr(attr: string): Object; */
DEFINE_OPERATOR_INVOKE(stype_operator_getattr, NULL, NULL) {
	DREF DeeObject *result;
	DeeObject *attr;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cgetattr"), &attr);
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	result = (*tp_self->st_attr->st_getattr)(tp_self, DeeStruct_Data(self), attr);
	if unlikely(result == ITER_DONE)
		result = DeeObject_TGenericGetAttr(DeeSType_AsType(tp_self), self, attr);
	return result;
err:
	return NULL;
}

/* >> operator cdelattr(attr: string); */
DEFINE_OPERATOR_INVOKE(stype_operator_delattr, NULL, NULL) {
	DeeObject *attr;
	int error;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("cdelattr"), &attr);
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	error = (*tp_self->st_attr->st_delattr)(tp_self, DeeStruct_Data(self), attr);
	if unlikely(error) {
		if likely(error == -2)
			error = DeeObject_TGenericDelAttr(DeeSType_AsType(tp_self), self, attr);
		if unlikely(error)
			goto err;
	}
	return_none;
err:
	return NULL;
}

/* >> operator csetattr(attr: string, value: Object); */
DEFINE_OPERATOR_INVOKE(stype_operator_setattr, NULL, NULL) {
	DeeObject *attr, *value;
	int error;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("csetattr"), &attr, &value);
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	error = (*tp_self->st_attr->st_setattr)(tp_self, DeeStruct_Data(self), attr, value);
	if unlikely(error) {
		if likely(error == -2)
			error = DeeObject_TGenericSetAttr(DeeSType_AsType(tp_self), self, attr, value);
		if unlikely(error)
			goto err;
	}
	return_none;
err:
	return NULL;
}

/* >> operator cenumattr(): rt.EnumAttr; */
DEFINE_OPERATOR_INVOKE(stype_operator_enumattr, NULL, NULL) {
	DeeObject *args[2];
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("cenumattr"));
	args[0] = (DeeObject *)tp_self;
	args[1] = self;
	return DeeObject_New(&DeeEnumAttr_Type, 2, args);
err:
	return NULL;
}


/* Define meta-data for new operators provided by structured types. */
INTDEF struct type_operator tpconst stype_operator_decls[];
INTERN_TPCONST struct type_operator tpconst stype_operator_decls[] = {
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0000_INIT /*       */, OPCLASS_TYPE, offsetof(DeeSTypeObject, st_init), /*                               */ OPCC_SPECIAL, "cinit", /*    */ "sinit", /*    */ "st_init", /*       */ &stype_operator_init),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0001_ASSIGN /*     */, OPCLASS_TYPE, offsetof(DeeSTypeObject, st_assign), /*                             */ OPCC_SPECIAL, "cassign", /*  */ "sassign", /*  */ "st_assign", /*     */ &stype_operator_assign),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0002_STR /*        */, OPCLASS_TYPE, offsetof(DeeSTypeObject, st_cast.st_str), /*                        */ OPCC_SPECIAL, "cstr", /*     */ "sstr", /*     */ "st_str", /*        */ &stype_operator_str),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0003_REPR /*       */, OPCLASS_TYPE, offsetof(DeeSTypeObject, st_cast.st_repr), /*                       */ OPCC_SPECIAL, "crepr", /*    */ "srepr", /*    */ "st_repr", /*       */ &stype_operator_repr),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0004_BOOL /*       */, OPCLASS_TYPE, offsetof(DeeSTypeObject, st_cast.st_bool), /*                       */ OPCC_SPECIAL, "cbool", /*    */ "sbool", /*    */ "st_bool", /*       */ &stype_operator_bool),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0005_CALL /*       */, OPCLASS_TYPE, offsetof(DeeSTypeObject, st_call), /*                               */ OPCC_SPECIAL, "ccall", /*    */ "scall", /*    */ "st_call", /*       */ &stype_operator_call),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0006_INT /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_int), /*        */ OPCC_SPECIAL, "cint", /*     */ "sint", /*     */ "st_int", /*        */ &stype_operator_int),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0007_DOUBLE /*     */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_double), /*     */ OPCC_SPECIAL, "cdouble", /*  */ "sdouble", /*  */ "st_double", /*     */ &stype_operator_double),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0008_INV /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inv), /*        */ OPCC_SPECIAL, "cinv", /*     */ "sinv", /*     */ "st_inv", /*        */ &stype_operator_inv),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0009_POS /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_pos), /*        */ OPCC_SPECIAL, "cpos", /*     */ "spos", /*     */ "st_pos", /*        */ &stype_operator_pos),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_000A_NEG /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_neg), /*        */ OPCC_SPECIAL, "cneg", /*     */ "sneg", /*     */ "st_neg", /*        */ &stype_operator_neg),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_000B_ADD /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_add), /*        */ OPCC_SPECIAL, "cadd", /*     */ "sadd", /*     */ "st_add", /*        */ &stype_operator_add),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_000C_SUB /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_sub), /*        */ OPCC_SPECIAL, "csub", /*     */ "ssub", /*     */ "st_sub", /*        */ &stype_operator_sub),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_000D_MUL /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_mul), /*        */ OPCC_SPECIAL, "cmul", /*     */ "smul", /*     */ "st_mul", /*        */ &stype_operator_mul),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_000E_DIV /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_div), /*        */ OPCC_SPECIAL, "cdiv", /*     */ "sdiv", /*     */ "st_div", /*        */ &stype_operator_div),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_000F_MOD /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_mod), /*        */ OPCC_SPECIAL, "cmod", /*     */ "smod", /*     */ "st_mod", /*        */ &stype_operator_mod),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0010_SHL /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_shl), /*        */ OPCC_SPECIAL, "cshl", /*     */ "sshl", /*     */ "st_shl", /*        */ &stype_operator_shl),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0011_SHR /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_shr), /*        */ OPCC_SPECIAL, "cshr", /*     */ "sshr", /*     */ "st_shr", /*        */ &stype_operator_shr),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0012_AND /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_and), /*        */ OPCC_SPECIAL, "cand", /*     */ "sand", /*     */ "st_and", /*        */ &stype_operator_and),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0013_OR /*         */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_or), /*         */ OPCC_SPECIAL, "cor", /*      */ "sor", /*      */ "st_or", /*         */ &stype_operator_or),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0014_XOR /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_xor), /*        */ OPCC_SPECIAL, "cxor", /*     */ "sxor", /*     */ "st_xor", /*        */ &stype_operator_xor),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0015_POW /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_pow), /*        */ OPCC_SPECIAL, "cpow", /*     */ "spow", /*     */ "st_pow", /*        */ &stype_operator_pow),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0016_INC /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inc), /*        */ OPCC_SPECIAL, "cinc", /*     */ "sinc", /*     */ "st_inc", /*        */ &stype_operator_inc),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0017_DEC /*        */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_dec), /*        */ OPCC_SPECIAL, "cdec", /*     */ "sdec", /*     */ "st_dec", /*        */ &stype_operator_dec),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0018_INPLACE_ADD /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_add), /**/ OPCC_SPECIAL, "ciadd", /*    */ "siadd", /*    */ "st_inplace_add", /**/ &stype_operator_inplace_add),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0019_INPLACE_SUB /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_sub), /**/ OPCC_SPECIAL, "cisub", /*    */ "sisub", /*    */ "st_inplace_sub", /**/ &stype_operator_inplace_sub),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_001A_INPLACE_MUL /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_mul), /**/ OPCC_SPECIAL, "cimul", /*    */ "simul", /*    */ "st_inplace_mul", /**/ &stype_operator_inplace_mul),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_001B_INPLACE_DIV /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_div), /**/ OPCC_SPECIAL, "cidiv", /*    */ "sidiv", /*    */ "st_inplace_div", /**/ &stype_operator_inplace_div),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_001C_INPLACE_MOD /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_mod), /**/ OPCC_SPECIAL, "cimod", /*    */ "simod", /*    */ "st_inplace_mod", /**/ &stype_operator_inplace_mod),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_001D_INPLACE_SHL /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_shl), /**/ OPCC_SPECIAL, "cishl", /*    */ "sishl", /*    */ "st_inplace_shl", /**/ &stype_operator_inplace_shl),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_001E_INPLACE_SHR /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_shr), /**/ OPCC_SPECIAL, "cishr", /*    */ "sishr", /*    */ "st_inplace_shr", /**/ &stype_operator_inplace_shr),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_001F_INPLACE_AND /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_and), /**/ OPCC_SPECIAL, "ciand", /*    */ "siand", /*    */ "st_inplace_and", /**/ &stype_operator_inplace_and),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0020_INPLACE_OR /* */, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_or), /* */ OPCC_SPECIAL, "cior", /*     */ "sior", /*     */ "st_inplace_or", /* */ &stype_operator_inplace_or),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0021_INPLACE_XOR /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_xor), /**/ OPCC_SPECIAL, "cixor", /*    */ "sixor", /*    */ "st_inplace_xor", /**/ &stype_operator_inplace_xor),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0022_INPLACE_POW /**/, offsetof(DeeSTypeObject, st_math), offsetof(struct stype_math, st_inplace_pow), /**/ OPCC_SPECIAL, "cipow", /*    */ "sipow", /*    */ "st_inplace_pow", /**/ &stype_operator_inplace_pow),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0023_EQ /*         */, offsetof(DeeSTypeObject, st_cmp), offsetof(struct stype_cmp, st_eq), /*           */ OPCC_SPECIAL, "ceq", /*      */ "seq", /*      */ "st_eq", /*         */ &stype_operator_eq),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0024_NE /*         */, offsetof(DeeSTypeObject, st_cmp), offsetof(struct stype_cmp, st_ne), /*           */ OPCC_SPECIAL, "cne", /*      */ "sne", /*      */ "st_ne", /*         */ &stype_operator_ne),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0025_LO /*         */, offsetof(DeeSTypeObject, st_cmp), offsetof(struct stype_cmp, st_lo), /*           */ OPCC_SPECIAL, "clo", /*      */ "slo", /*      */ "st_lo", /*         */ &stype_operator_lo),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0026_LE /*         */, offsetof(DeeSTypeObject, st_cmp), offsetof(struct stype_cmp, st_le), /*           */ OPCC_SPECIAL, "cle", /*      */ "sle", /*      */ "st_le", /*         */ &stype_operator_le),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0027_GR /*         */, offsetof(DeeSTypeObject, st_cmp), offsetof(struct stype_cmp, st_gr), /*           */ OPCC_SPECIAL, "cgr", /*      */ "sgr", /*      */ "st_gr", /*         */ &stype_operator_gr),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0028_GE /*         */, offsetof(DeeSTypeObject, st_cmp), offsetof(struct stype_cmp, st_ge), /*           */ OPCC_SPECIAL, "cge", /*      */ "sge", /*      */ "st_ge", /*         */ &stype_operator_ge),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0029_ITER /*       */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_iter_self), /*    */ OPCC_SPECIAL, "citer", /*    */ "siter", /*    */ "st_iter", /*       */ &stype_operator_iter),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_002A_SIZE /*       */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_size), /*         */ OPCC_SPECIAL, "csize", /*    */ "ssize", /*    */ "st_size", /*       */ &stype_operator_size),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_002B_CONTAINS /*   */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_contains), /*     */ OPCC_SPECIAL, "ccontains", /**/ "scontains", /**/ "st_contains", /*   */ &stype_operator_contains),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_002C_GETITEM /*    */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_get), /*          */ OPCC_SPECIAL, "cgetitem", /* */ "sgetitem", /* */ "st_getitem", /*    */ &stype_operator_getitem),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_002D_DELITEM /*    */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_del), /*          */ OPCC_SPECIAL, "cdelitem", /* */ "sdelitem", /* */ "st_delitem", /*    */ &stype_operator_delitem),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_002E_SETITEM /*    */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_set), /*          */ OPCC_SPECIAL, "csetitem", /* */ "ssetitem", /* */ "st_setitem", /*    */ &stype_operator_setitem),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_002F_GETRANGE /*   */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_range_get), /*    */ OPCC_SPECIAL, "cgetrange", /**/ "sgetrange", /**/ "st_getrange", /*   */ &stype_operator_getrange),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0030_DELRANGE /*   */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_range_del), /*    */ OPCC_SPECIAL, "cdelrange", /**/ "sdelrange", /**/ "st_delrange", /*   */ &stype_operator_delrange),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0031_SETRANGE /*   */, offsetof(DeeSTypeObject, st_seq), offsetof(struct stype_seq, st_range_set), /*    */ OPCC_SPECIAL, "csetrange", /**/ "ssetrange", /**/ "st_setrange", /*   */ &stype_operator_setrange),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0032_GETATTR /*    */, offsetof(DeeSTypeObject, st_attr), offsetof(struct stype_attr, st_getattr), /*    */ OPCC_SPECIAL, "cgetattr", /* */ "sgetattr", /* */ "st_getattr", /*    */ &stype_operator_getattr),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0033_DELATTR /*    */, offsetof(DeeSTypeObject, st_attr), offsetof(struct stype_attr, st_delattr), /*    */ OPCC_SPECIAL, "cdelattr", /* */ "sdelattr", /* */ "st_delattr", /*    */ &stype_operator_delattr),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0034_SETATTR /*    */, offsetof(DeeSTypeObject, st_attr), offsetof(struct stype_attr, st_setattr), /*    */ OPCC_SPECIAL, "csetattr", /* */ "ssetattr", /* */ "st_setattr", /*    */ &stype_operator_setattr),
	TYPE_OPERATOR_DECL(OPERATOR_STYPE_0035_ENUMATTR /*   */, offsetof(DeeSTypeObject, st_attr), offsetof(struct stype_attr, st_iterattr), /*   */ OPCC_SPECIAL, "cenumattr", /**/ "senumattr", /**/ "st_iterattr", /*   */ &stype_operator_enumattr),
};

DECL_END

#endif /* !GUARD_DEX_CTYPES_CORE_OPERATORS_C */
