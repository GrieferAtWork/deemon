/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_C_ATOMIC_C
#define GUARD_DEX_CTYPES_C_ATOMIC_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include "c_api.h"
/**/

#include <hybrid/atomic.h>
#include <hybrid/overflow.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/system-features.h>

DECL_BEGIN

PRIVATE ATTR_COLD int DCALL err_bad_atomic_size(size_t size) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "No support for %Iu-bytes large atomic operations",
	                       size);
}

union atomic_operand {
	int8_t ao_s8;
	int16_t ao_s16;
	int32_t ao_s32;
	int64_t ao_s64;
	uint8_t ao_u8;
	uint16_t ao_u16;
	uint32_t ao_u32;
	uint64_t ao_u64;
};

PRIVATE NONNULL((1, 2, 3)) int DCALL
get_atomic_operand(DeeObject *value, DeeSTypeObject *ob_ptr_orig,
                   union atomic_operand *__restrict result) {
	int error;
#ifndef __OPTIMIZE_SIZE__
	if (!DeeInt_Check(value))
#endif /* !__OPTIMIZE_SIZE__ */
	{
		if (DeeObject_InstanceOf(value, (DeeTypeObject *)ob_ptr_orig)) {
			/* structured-type-derived-from-<ob_ptr_orig> */
			memcpy(result, DeeStruct_Data(value), ob_ptr_orig->st_sizeof);
			return 0;
		}
		if (DeeLValue_Check(value)) {
			DeeSTypeObject *lv_base;
			lv_base = ((DeeLValueTypeObject *)Dee_TYPE(value))->lt_orig;
			if (DeeType_IsInherited((DeeTypeObject *)lv_base,
			                        (DeeTypeObject *)ob_ptr_orig)) {
				/* Lvalue -> structured-type-derived-from-<ob_ptr_orig> */
				CTYPES_FAULTPROTECT({
					union pointer result_ptr;
					result_ptr.ptr = *(void **)((struct lvalue_object *)value)->l_ptr.ptr;
					memcpy(result, result_ptr.ptr, ob_ptr_orig->st_sizeof);
				}, goto err);
				return 0;
			}
		}
	}

	/* Always accept signed/unsigned integers */
	switch (ob_ptr_orig->st_sizeof) {

	case 1:
		error = DeeObject_GetInt8(value, &result->ao_s8);
		break;

	case 2:
		error = DeeObject_GetInt16(value, &result->ao_s16);
		break;

	case 4:
		error = DeeObject_GetInt32(value, &result->ao_s32);
		break;

	case 8:
		error = DeeObject_GetInt64(value, &result->ao_s64);
		break;

	default:
		err_bad_atomic_size(ob_ptr_orig->st_sizeof);
		goto err;
	}
	if likely(error != -1)
		return 0;
	DeeObject_TypeAssertFailed(value, (DeeTypeObject *)ob_ptr_orig);
err:
	return -1;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_atomic_cmpxch(size_t argc, DeeObject *const *argv) {
	DeeObject *ob_ptr, *ob_oldval, *ob_newval;
	union atomic_operand oldval, newval;
	union pointer ptr;
	DeeSTypeObject *basetype;
	bool result, weak = false;
	if (DeeArg_Unpack(argc, argv, "ooo|b:atomic_cmpxch",
	                  &ob_ptr, &ob_oldval, &ob_newval, &weak))
		goto err;
	if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))
		goto err;
	if unlikely(get_atomic_operand(ob_oldval, basetype, &oldval))
		goto err;
	if unlikely(get_atomic_operand(ob_newval, basetype, &newval))
		goto err;
	CTYPES_FAULTPROTECT({
		if (weak) {
			switch (basetype->st_sizeof) {
			case 1: result = ATOMIC_CMPXCH_WEAK(*ptr.p8, oldval.ao_u8, newval.ao_u8); break;
			case 2: result = ATOMIC_CMPXCH_WEAK(*ptr.p16, oldval.ao_u16, newval.ao_u16); break;
			case 4: result = ATOMIC_CMPXCH_WEAK(*ptr.p32, oldval.ao_u32, newval.ao_u32); break;
			case 8: result = ATOMIC_CMPXCH_WEAK(*ptr.p64, oldval.ao_u64, newval.ao_u64); break;
			default: __builtin_unreachable();
			}
		} else {
			switch (basetype->st_sizeof) {
			case 1: result = ATOMIC_CMPXCH(*ptr.p8, oldval.ao_u8, newval.ao_u8); break;
			case 2: result = ATOMIC_CMPXCH(*ptr.p16, oldval.ao_u16, newval.ao_u16); break;
			case 4: result = ATOMIC_CMPXCH(*ptr.p32, oldval.ao_u32, newval.ao_u32); break;
			case 8: result = ATOMIC_CMPXCH(*ptr.p64, oldval.ao_u64, newval.ao_u64); break;
			default: __builtin_unreachable();
			}
		}
	}, goto err);
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_atomic_cmpxch_val(size_t argc, DeeObject *const *argv) {
	DeeObject *ob_ptr, *ob_oldval, *ob_newval;
	union atomic_operand oldval, newval;
	union pointer ptr;
	DeeSTypeObject *basetype;
	DREF DeeObject *result_obj;
	if (DeeArg_Unpack(argc, argv, "ooo:atomic_cmpxch_val",
	                  &ob_ptr, &ob_oldval, &ob_newval))
		goto err;
	if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))
		goto err;
	if unlikely(get_atomic_operand(ob_oldval, basetype, &oldval))
		goto err;
	if unlikely(get_atomic_operand(ob_newval, basetype, &newval))
		goto err;
	/* Allocate a buffer for the result (which is the *real* old value) */
	result_obj = (DREF DeeObject *)DeeObject_Malloc(sizeof(DeeObject) +
	                                                basetype->st_sizeof);
	if unlikely(!result_obj)
		goto err;
	CTYPES_FAULTPROTECT({
		switch (basetype->st_sizeof) {
		case 1: *(uint8_t *)DeeStruct_Data(result_obj) = ATOMIC_CMPXCH_VAL(*ptr.p8, oldval.ao_u8, newval.ao_u8); break;
		case 2: *(uint16_t *)DeeStruct_Data(result_obj) = ATOMIC_CMPXCH_VAL(*ptr.p16, oldval.ao_u16, newval.ao_u16); break;
		case 4: *(uint32_t *)DeeStruct_Data(result_obj) = ATOMIC_CMPXCH_VAL(*ptr.p32, oldval.ao_u32, newval.ao_u32); break;
		case 8: *(uint64_t *)DeeStruct_Data(result_obj) = ATOMIC_CMPXCH_VAL(*ptr.p64, oldval.ao_u64, newval.ao_u64); break;
		default: __builtin_unreachable();
		}
	}, goto err_result_obj);
	DeeObject_Init(result_obj, (DeeTypeObject *)basetype);
	return result_obj;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
err_result_obj:
	DeeObject_Free(result_obj);
#endif /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
err:
	return NULL;
}

#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#define IF_HAVE_FAULTPROTECT(x) x
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
#define IF_HAVE_FAULTPROTECT(x) /* nothing */
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */

#define DEFINE_ATOMIC_BINOP(capi_atomic_name, atomic_name, ATOMIC_NAME)                                    \
	INTERN WUNUSED DREF DeeObject *DCALL                                                                   \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                                                \
		DeeObject *ob_ptr, *ob_addend;                                                                     \
		union atomic_operand addend;                                                                       \
		union pointer ptr;                                                                                 \
		DeeSTypeObject *basetype;                                                                          \
		DREF DeeObject *result_obj;                                                                        \
		if (DeeArg_Unpack(argc, argv, "oo:" atomic_name,                                                   \
		                  &ob_ptr, &ob_addend))                                                            \
			goto err;                                                                                      \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))                                   \
			goto err;                                                                                      \
		if unlikely(get_atomic_operand(ob_addend, basetype, &addend))                                      \
			goto err;                                                                                      \
		/* Allocate a buffer for the result (which is the *real* old value) */                             \
		result_obj = (DREF DeeObject *)DeeObject_Malloc(sizeof(DeeObject) +                                \
		                                                basetype->st_sizeof);                              \
		if unlikely(!result_obj)                                                                           \
			goto err;                                                                                      \
		CTYPES_FAULTPROTECT({                                                                              \
			switch (basetype->st_sizeof) {                                                                 \
			case 1: *(uint8_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p8, addend.ao_u8); break;    \
			case 2: *(uint16_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p16, addend.ao_u16); break; \
			case 4: *(uint32_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p32, addend.ao_u32); break; \
			case 8: *(uint64_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p64, addend.ao_u64); break; \
			default: __builtin_unreachable();                                                              \
			}                                                                                              \
		}, goto err_result_obj);                                                                           \
		DeeObject_Init(result_obj, (DeeTypeObject *)basetype);                                             \
		return result_obj;                                                                                 \
IF_HAVE_FAULTPROTECT(err_result_obj:                                                                       \
		DeeObject_Free(result_obj);)                                                                       \
err:                                                                                                       \
		return NULL;                                                                                       \
	}
DEFINE_ATOMIC_BINOP(capi_atomic_fetchadd, "atomic_fetchadd", ATOMIC_FETCHADD)
DEFINE_ATOMIC_BINOP(capi_atomic_fetchsub, "atomic_fetchsub", ATOMIC_FETCHSUB)
DEFINE_ATOMIC_BINOP(capi_atomic_fetchand, "atomic_fetchand", ATOMIC_FETCHAND)
DEFINE_ATOMIC_BINOP(capi_atomic_fetchor, "atomic_fetchor", ATOMIC_FETCHOR)
DEFINE_ATOMIC_BINOP(capi_atomic_fetchxor, "atomic_fetchxor", ATOMIC_FETCHXOR)
DEFINE_ATOMIC_BINOP(capi_atomic_fetchnand, "atomic_fetchnand", ATOMIC_FETCHNAND)
DEFINE_ATOMIC_BINOP(capi_atomic_addfetch, "atomic_addfetch", ATOMIC_ADDFETCH)
DEFINE_ATOMIC_BINOP(capi_atomic_subfetch, "atomic_subfetch", ATOMIC_SUBFETCH)
DEFINE_ATOMIC_BINOP(capi_atomic_andfetch, "atomic_andfetch", ATOMIC_ANDFETCH)
DEFINE_ATOMIC_BINOP(capi_atomic_orfetch, "atomic_orfetch", ATOMIC_ORFETCH)
DEFINE_ATOMIC_BINOP(capi_atomic_xorfetch, "atomic_xorfetch", ATOMIC_XORFETCH)
DEFINE_ATOMIC_BINOP(capi_atomic_nandfetch, "atomic_nandfetch", ATOMIC_NANDFETCH)
#undef DEFINE_ATOMIC_BINOP

#define DEFINE_ATOMIC_BINOP_VOID(capi_atomic_name, atomic_name, ATOMIC_NAME) \
	INTERN WUNUSED DREF DeeObject *DCALL                                     \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                  \
		DeeObject *ob_ptr, *ob_addend;                                       \
		union atomic_operand addend;                                         \
		union pointer ptr;                                                   \
		DeeSTypeObject *basetype;                                            \
		if (DeeArg_Unpack(argc, argv, "oo:" atomic_name,                     \
		                  &ob_ptr, &ob_addend))                              \
			goto err;                                                        \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))     \
			goto err;                                                        \
		if unlikely(get_atomic_operand(ob_addend, basetype, &addend))        \
			goto err;                                                        \
		CTYPES_FAULTPROTECT({                                                \
			switch (basetype->st_sizeof) {                                   \
			case 1: ATOMIC_NAME(*ptr.p8, addend.ao_u8); break;               \
			case 2: ATOMIC_NAME(*ptr.p16, addend.ao_u16); break;             \
			case 4: ATOMIC_NAME(*ptr.p32, addend.ao_u32); break;             \
			case 8: ATOMIC_NAME(*ptr.p64, addend.ao_u64); break;             \
			default: __builtin_unreachable();                                \
			}                                                                \
		}, goto err);                                                        \
		return_none;                                                         \
err:                                                                         \
		return NULL;                                                         \
	}
DEFINE_ATOMIC_BINOP_VOID(capi_atomic_add, "atomic_add", ATOMIC_ADD)
DEFINE_ATOMIC_BINOP_VOID(capi_atomic_sub, "atomic_sub", ATOMIC_SUB)
DEFINE_ATOMIC_BINOP_VOID(capi_atomic_and, "atomic_and", ATOMIC_AND)
DEFINE_ATOMIC_BINOP_VOID(capi_atomic_or, "atomic_or", ATOMIC_OR)
DEFINE_ATOMIC_BINOP_VOID(capi_atomic_xor, "atomic_xor", ATOMIC_XOR)
DEFINE_ATOMIC_BINOP_VOID(capi_atomic_nand, "atomic_nand", ATOMIC_NAND)
DEFINE_ATOMIC_BINOP_VOID(capi_atomic_write, "atomic_write", ATOMIC_WRITE)
#undef DEFINE_ATOMIC_BINOP_VOID

#define DEFINE_ATOMIC_UNOP(capi_atomic_name, atomic_name, ATOMIC_NAME)                      \
	INTERN WUNUSED DREF DeeObject *DCALL                                                    \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                                 \
		DeeObject *ob_ptr;                                                                  \
		union pointer ptr;                                                                  \
		DeeSTypeObject *basetype;                                                           \
		DREF DeeObject *result_obj;                                                         \
		if (DeeArg_Unpack(argc, argv, "o:" atomic_name, &ob_ptr))                           \
			goto err;                                                                       \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))                    \
			goto err;                                                                       \
		/* Allocate a buffer for the result (which is the *real* old value) */              \
		result_obj = (DREF DeeObject *)DeeObject_Malloc(sizeof(DeeObject) +                 \
		                                                basetype->st_sizeof);               \
		if unlikely(!result_obj)                                                            \
			goto err;                                                                       \
		CTYPES_FAULTPROTECT({                                                               \
			switch (basetype->st_sizeof) {                                                  \
			case 1: *(uint8_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p8); break;   \
			case 2: *(uint16_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p16); break; \
			case 4: *(uint32_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p32); break; \
			case 8: *(uint64_t *)DeeStruct_Data(result_obj) = ATOMIC_NAME(*ptr.p64); break; \
			default: __builtin_unreachable();                                               \
			}                                                                               \
		}, goto err_result_obj);                                                            \
		DeeObject_Init(result_obj, (DeeTypeObject *)basetype);                              \
		return result_obj;                                                                  \
IF_HAVE_FAULTPROTECT(err_result_obj:                                                        \
		DeeObject_Free(result_obj);)                                                        \
err:                                                                                        \
		return NULL;                                                                        \
	}
DEFINE_ATOMIC_UNOP(capi_atomic_fetchinc, "atomic_fetchinc", ATOMIC_FETCHINC)
DEFINE_ATOMIC_UNOP(capi_atomic_fetchdec, "atomic_fetchdec", ATOMIC_FETCHDEC)
DEFINE_ATOMIC_UNOP(capi_atomic_incfetch, "atomic_incfetch", ATOMIC_INCFETCH)
DEFINE_ATOMIC_UNOP(capi_atomic_decfetch, "atomic_decfetch", ATOMIC_DECFETCH)
DEFINE_ATOMIC_UNOP(capi_atomic_read, "atomic_read", ATOMIC_READ)
#undef DEFINE_ATOMIC_UNOP

#define DEFINE_ATOMIC_UNOP_VOID(capi_atomic_name, atomic_name, ATOMIC_NAME) \
	INTERN WUNUSED DREF DeeObject *DCALL                                    \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                 \
		DeeObject *ob_ptr;                                                  \
		union pointer ptr;                                                  \
		DeeSTypeObject *basetype;                                           \
		if (DeeArg_Unpack(argc, argv, "o:" atomic_name, &ob_ptr))           \
			goto err;                                                       \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))    \
			goto err;                                                       \
		CTYPES_FAULTPROTECT({                                               \
			switch (basetype->st_sizeof) {                                  \
			case 1: ATOMIC_NAME(*ptr.p8); break;                            \
			case 2: ATOMIC_NAME(*ptr.p16); break;                           \
			case 4: ATOMIC_NAME(*ptr.p32); break;                           \
			case 8: ATOMIC_NAME(*ptr.p64); break;                           \
			default: __builtin_unreachable();                               \
			}                                                               \
		}, goto err);                                                       \
		return_none;                                                        \
err:                                                                        \
		return NULL;                                                        \
	}
DEFINE_ATOMIC_UNOP_VOID(capi_atomic_inc, "atomic_inc", ATOMIC_INC)
DEFINE_ATOMIC_UNOP_VOID(capi_atomic_dec, "atomic_dec", ATOMIC_DEC)
#undef DEFINE_ATOMIC_UNOP_VOID


DECL_END


#endif /* !GUARD_DEX_CTYPES_C_ATOMIC_C */