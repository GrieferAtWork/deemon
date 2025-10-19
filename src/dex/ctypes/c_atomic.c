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
#ifndef GUARD_DEX_CTYPES_C_ATOMIC_C
#define GUARD_DEX_CTYPES_C_ATOMIC_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* memcpy, ... */
#include <deemon/util/atomic.h>
#include <deemon/util/futex.h>

#include <hybrid/typecore.h>

/**/
#include "c_api.h" /* Prototypes... */

/**/
#include <stddef.h> /* size_t */
#include <stdint.h> /* intN_t, uintN_t */

DECL_BEGIN

PRIVATE ATTR_COLD int DCALL err_bad_atomic_size(size_t size) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "No support for %" PRFuSIZ "-bytes large atomic operations",
	                       size);
}

PRIVATE ATTR_COLD int DCALL err_bad_futex_size(size_t size) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "No support for %" PRFuSIZ "-bytes large futex operations",
	                       size);
}

PRIVATE ATTR_COLD int DCALL err_unaligned_futex_poiner(void *ptr, size_t size) {
	return DeeError_Throwf(&DeeError_SegFault,
	                       "Cannot do %" PRFuSIZ "-bytes large futex operation "
	                       "on %p (pointer is not properly aligned)",
	                       size, ptr);
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
		if (DeeObject_InstanceOf(value, DeeSType_AsType(ob_ptr_orig))) {
			/* structured-type-derived-from-<ob_ptr_orig> */
			memcpy(result, DeeStruct_Data(value), ob_ptr_orig->st_sizeof);
			return 0;
		}
		if (DeeLValue_Check(value)) {
			DeeSTypeObject *lv_base;
			lv_base = DeeType_AsLValueType(Dee_TYPE(value))->lt_orig;
			if (DeeType_Extends(DeeSType_AsType(lv_base),
			                    DeeSType_AsType(ob_ptr_orig))) {
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
		error = DeeObject_Get8Bit(value, &result->ao_s8);
		break;

	case 2:
		error = DeeObject_Get16Bit(value, &result->ao_s16);
		break;

	case 4:
		error = DeeObject_Get32Bit(value, &result->ao_s32);
		break;

	case 8:
		error = DeeObject_Get64Bit(value, &result->ao_s64);
		break;

	default:
		err_bad_atomic_size(ob_ptr_orig->st_sizeof);
		goto err;
	}
	if likely(error != -1)
		return 0;
	DeeObject_TypeAssertFailed(value, DeeSType_AsType(ob_ptr_orig));
err:
	return -1;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_atomic_cmpxch(size_t argc, DeeObject *const *argv) {
	union atomic_operand oldval, newval;
	union pointer ptr;
	DeeSTypeObject *basetype;
	bool result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("atomic_cmpxch", params: """
	DeeObject *ptr;
	DeeObject *oldval;
	DeeObject *newval;
	bool weak = false;
""");]]]*/
	struct {
		DeeObject *ptr;
		DeeObject *oldval;
		DeeObject *newval;
		bool weak;
	} args;
	args.weak = false;
	if (DeeArg_UnpackStruct(argc, argv, "ooo|b:atomic_cmpxch", &args))
		goto err;
/*[[[end]]]*/
	if unlikely(DeeObject_AsGenericPointer(args.ptr, &basetype, &ptr))
		goto err;
	if unlikely(get_atomic_operand(args.oldval, basetype, &oldval))
		goto err;
	if unlikely(get_atomic_operand(args.newval, basetype, &newval))
		goto err;
	CTYPES_FAULTPROTECT({
		if (args.weak) {
			switch (basetype->st_sizeof) {
			case 1: result = atomic_cmpxch_weak(ptr.p8, oldval.ao_u8, newval.ao_u8); break;
			case 2: result = atomic_cmpxch_weak(ptr.p16, oldval.ao_u16, newval.ao_u16); break;
			case 4: result = atomic_cmpxch_weak(ptr.p32, oldval.ao_u32, newval.ao_u32); break;
			case 8: result = atomic_cmpxch_weak(ptr.p64, oldval.ao_u64, newval.ao_u64); break;
			default: __builtin_unreachable();
			}
		} else {
			switch (basetype->st_sizeof) {
			case 1: result = atomic_cmpxch(ptr.p8, oldval.ao_u8, newval.ao_u8); break;
			case 2: result = atomic_cmpxch(ptr.p16, oldval.ao_u16, newval.ao_u16); break;
			case 4: result = atomic_cmpxch(ptr.p32, oldval.ao_u32, newval.ao_u32); break;
			case 8: result = atomic_cmpxch(ptr.p64, oldval.ao_u64, newval.ao_u64); break;
			default: __builtin_unreachable();
			}
		}
	}, goto err);
	return_bool(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_atomic_cmpxch_val(size_t argc, DeeObject *const *argv) {
	union atomic_operand oldval, newval;
	union pointer ptr;
	DeeSTypeObject *basetype;
	DREF DeeObject *result_obj;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("atomic_cmpxch_val", params: """
	DeeObject *ptr;
	DeeObject *oldval;
	DeeObject *newval;
""");]]]*/
	struct {
		DeeObject *ptr;
		DeeObject *oldval;
		DeeObject *newval;
	} args;
	DeeArg_UnpackStruct3(err, argc, argv, "atomic_cmpxch_val", &args, &args.ptr, &args.oldval, &args.newval);
/*[[[end]]]*/
	if unlikely(DeeObject_AsGenericPointer(args.ptr, &basetype, &ptr))
		goto err;
	if unlikely(get_atomic_operand(args.oldval, basetype, &oldval))
		goto err;
	if unlikely(get_atomic_operand(args.newval, basetype, &newval))
		goto err;
	/* Allocate a buffer for the result (which is the *real* old value) */
	result_obj = (DREF DeeObject *)DeeObject_Malloc(sizeof(DeeObject) +
	                                                basetype->st_sizeof);
	if unlikely(!result_obj)
		goto err;
	CTYPES_FAULTPROTECT({
		switch (basetype->st_sizeof) {
		case 1: *(uint8_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(ptr.p8, oldval.ao_u8, newval.ao_u8); break;
		case 2: *(uint16_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(ptr.p16, oldval.ao_u16, newval.ao_u16); break;
		case 4: *(uint32_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(ptr.p32, oldval.ao_u32, newval.ao_u32); break;
		case 8: *(uint64_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(ptr.p64, oldval.ao_u64, newval.ao_u64); break;
		default: __builtin_unreachable();
		}
	}, goto err_result_obj);
	DeeObject_Init(result_obj, DeeSType_AsType(basetype));
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

#define CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_name, atomic_name, atomic_func)                           \
	INTERN WUNUSED DREF DeeObject *DCALL                                                                  \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                                               \
		DeeObject *ob_ptr, *ob_addend;                                                                    \
		union atomic_operand addend;                                                                      \
		union pointer ptr;                                                                                \
		DeeSTypeObject *basetype;                                                                         \
		DREF DeeObject *result_obj;                                                                       \
		DeeArg_Unpack2(err, argc, argv, atomic_name, &ob_ptr, &ob_addend);                                \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))                                  \
			goto err;                                                                                     \
		if unlikely(get_atomic_operand(ob_addend, basetype, &addend))                                     \
			goto err;                                                                                     \
		/* Allocate a buffer for the result (which is the *real* old value) */                            \
		result_obj = (DREF DeeObject *)DeeObject_Malloc(sizeof(DeeObject) +                               \
		                                                basetype->st_sizeof);                             \
		if unlikely(!result_obj)                                                                          \
			goto err;                                                                                     \
		CTYPES_FAULTPROTECT({                                                                             \
			switch (basetype->st_sizeof) {                                                                \
			case 1: *(uint8_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p8, addend.ao_u8); break;    \
			case 2: *(uint16_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p16, addend.ao_u16); break; \
			case 4: *(uint32_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p32, addend.ao_u32); break; \
			case 8: *(uint64_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p64, addend.ao_u64); break; \
			default: __builtin_unreachable();                                                             \
			}                                                                                             \
		}, goto err_result_obj);                                                                          \
		DeeObject_Init(result_obj, DeeSType_AsType(basetype));                                            \
		return result_obj;                                                                                \
	IF_HAVE_FAULTPROTECT(err_result_obj:                                                                  \
		DeeObject_Free(result_obj);)                                                                      \
	err:                                                                                                  \
		return NULL;                                                                                      \
	}
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_fetchadd, "atomic_fetchadd", atomic_fetchadd)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_fetchsub, "atomic_fetchsub", atomic_fetchsub)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_fetchand, "atomic_fetchand", atomic_fetchand)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_fetchor, "atomic_fetchor", atomic_fetchor)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_fetchxor, "atomic_fetchxor", atomic_fetchxor)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_fetchnand, "atomic_fetchnand", atomic_fetchnand)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_addfetch, "atomic_addfetch", atomic_addfetch)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_subfetch, "atomic_subfetch", atomic_subfetch)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_andfetch, "atomic_andfetch", atomic_andfetch)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_orfetch, "atomic_orfetch", atomic_orfetch)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_xorfetch, "atomic_xorfetch", atomic_xorfetch)
CYTYPES_DEFINE_ATOMIC_BINOP(capi_atomic_nandfetch, "atomic_nandfetch", atomic_nandfetch)
#undef CYTYPES_DEFINE_ATOMIC_BINOP

#define CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_name, atomic_name, atomic_func) \
	INTERN WUNUSED DREF DeeObject *DCALL                                            \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                         \
		DeeObject *ob_ptr, *ob_addend;                                              \
		union atomic_operand addend;                                                \
		union pointer ptr;                                                          \
		DeeSTypeObject *basetype;                                                   \
		DeeArg_Unpack2(err, argc, argv, atomic_name, &ob_ptr, &ob_addend);          \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))            \
			goto err;                                                               \
		if unlikely(get_atomic_operand(ob_addend, basetype, &addend))               \
			goto err;                                                               \
		CTYPES_FAULTPROTECT({                                                       \
			switch (basetype->st_sizeof) {                                          \
			case 1: atomic_func(ptr.p8, addend.ao_u8); break;                       \
			case 2: atomic_func(ptr.p16, addend.ao_u16); break;                     \
			case 4: atomic_func(ptr.p32, addend.ao_u32); break;                     \
			case 8: atomic_func(ptr.p64, addend.ao_u64); break;                     \
			default: __builtin_unreachable();                                       \
			}                                                                       \
		}, goto err);                                                               \
		return_none;                                                                \
	err:                                                                            \
		return NULL;                                                                \
	}
CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_add, "atomic_add", atomic_add)
CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_sub, "atomic_sub", atomic_sub)
CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_and, "atomic_and", atomic_and)
CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_or, "atomic_or", atomic_or)
CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_xor, "atomic_xor", atomic_xor)
CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_nand, "atomic_nand", atomic_nand)
CTYPES_DEFINE_ATOMIC_BINOP_VOID(capi_atomic_write, "atomic_write", atomic_write)
#undef CTYPES_DEFINE_ATOMIC_BINOP_VOID

#define CTYPES_DEFINE_ATOMIC_UNOP(capi_atomic_name, atomic_name, atomic_func)              \
	INTERN WUNUSED DREF DeeObject *DCALL                                                   \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                                \
		DeeObject *ob_ptr;                                                                 \
		union pointer ptr;                                                                 \
		DeeSTypeObject *basetype;                                                          \
		DREF DeeObject *result_obj;                                                        \
		DeeArg_Unpack1(err, argc, argv, atomic_name, &ob_ptr);                             \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))                   \
			goto err;                                                                      \
		/* Allocate a buffer for the result (which is the *real* old value) */             \
		result_obj = (DREF DeeObject *)DeeObject_Malloc(sizeof(DeeObject) +                \
		                                                basetype->st_sizeof);              \
		if unlikely(!result_obj)                                                           \
			goto err;                                                                      \
		CTYPES_FAULTPROTECT({                                                              \
			switch (basetype->st_sizeof) {                                                 \
			case 1: *(uint8_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p8); break;   \
			case 2: *(uint16_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p16); break; \
			case 4: *(uint32_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p32); break; \
			case 8: *(uint64_t *)DeeStruct_Data(result_obj) = atomic_func(ptr.p64); break; \
			default: __builtin_unreachable();                                              \
			}                                                                              \
		}, goto err_result_obj);                                                           \
		DeeObject_Init(result_obj, DeeSType_AsType(basetype));                             \
		return result_obj;                                                                 \
	IF_HAVE_FAULTPROTECT(err_result_obj:                                                   \
		DeeObject_Free(result_obj);)                                                       \
	err:                                                                                   \
		return NULL;                                                                       \
	}
CTYPES_DEFINE_ATOMIC_UNOP(capi_atomic_fetchinc, "atomic_fetchinc", atomic_fetchinc)
CTYPES_DEFINE_ATOMIC_UNOP(capi_atomic_fetchdec, "atomic_fetchdec", atomic_fetchdec)
CTYPES_DEFINE_ATOMIC_UNOP(capi_atomic_incfetch, "atomic_incfetch", atomic_incfetch)
CTYPES_DEFINE_ATOMIC_UNOP(capi_atomic_decfetch, "atomic_decfetch", atomic_decfetch)
CTYPES_DEFINE_ATOMIC_UNOP(capi_atomic_read, "atomic_read", atomic_read)
#undef CTYPES_DEFINE_ATOMIC_UNOP

#define CTYPES_DEFINE_ATOMIC_UNOP_VOID(capi_atomic_name, atomic_name, atomic_func) \
	INTERN WUNUSED DREF DeeObject *DCALL                                           \
	capi_atomic_name(size_t argc, DeeObject *const *argv) {                        \
		DeeObject *ob_ptr;                                                         \
		union pointer ptr;                                                         \
		DeeSTypeObject *basetype;                                                  \
		DeeArg_Unpack1(err, argc, argv, atomic_name, &ob_ptr);                     \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))           \
			goto err;                                                              \
		CTYPES_FAULTPROTECT({                                                      \
			switch (basetype->st_sizeof) {                                         \
			case 1: atomic_func(ptr.p8); break;                                    \
			case 2: atomic_func(ptr.p16); break;                                   \
			case 4: atomic_func(ptr.p32); break;                                   \
			case 8: atomic_func(ptr.p64); break;                                   \
			default: __builtin_unreachable();                                      \
			}                                                                      \
		}, goto err);                                                              \
		return_none;                                                               \
	err:                                                                           \
		return NULL;                                                               \
	}
CTYPES_DEFINE_ATOMIC_UNOP_VOID(capi_atomic_inc, "atomic_inc", atomic_inc)
CTYPES_DEFINE_ATOMIC_UNOP_VOID(capi_atomic_dec, "atomic_dec", atomic_dec)
#undef CTYPES_DEFINE_ATOMIC_UNOP_VOID


/* Futex API */
INTERN WUNUSED DREF DeeObject *DCALL
capi_futex_wakeone(size_t argc, DeeObject *const *argv) {
	union pointer ptr;
	DeeSTypeObject *basetype;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("futex_wakeone", params: """
	DeeObject *ptr;
""");]]]*/
	struct {
		DeeObject *ptr;
	} args;
	DeeArg_Unpack1(err, argc, argv, "futex_wakeone", &args.ptr);
/*[[[end]]]*/
	if unlikely(DeeObject_AsGenericPointer(args.ptr, &basetype, &ptr))
		goto err;
	/* Because our `capi_futex_wait()' emulates 8-bit and 16-bit waits
	 * by waiting for the relevant 32-bit word, we have to match its
	 * alignment here! */
	ptr.uint &= ~3;
	DeeFutex_WakeOne(ptr.pvoid);
	return_none;
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_futex_wakeall(size_t argc, DeeObject *const *argv) {
	union pointer ptr;
	DeeSTypeObject *basetype;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("futex_wakeall", params: """
	DeeObject *ptr;
""");]]]*/
	struct {
		DeeObject *ptr;
	} args;
	DeeArg_Unpack1(err, argc, argv, "futex_wakeall", &args.ptr);
/*[[[end]]]*/
	if unlikely(DeeObject_AsGenericPointer(args.ptr, &basetype, &ptr))
		goto err;
	/* Because our `capi_futex_wait()' emulates 8-bit and 16-bit waits
	 * by waiting for the relevant 32-bit word, we have to match its
	 * alignment here! */
	ptr.uint &= ~3;
	DeeFutex_WakeAll(ptr.pvoid);
	return_none;
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_futex_wait(size_t argc, DeeObject *const *argv) {
	int wait_error;
	union atomic_operand expected;
	union pointer ptr;
	DeeSTypeObject *basetype;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("futex_wait", params: """
	DeeObject *ptr;
	DeeObject *expected;
""");]]]*/
	struct {
		DeeObject *ptr;
		DeeObject *expected;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "futex_wait", &args, &args.ptr, &args.expected);
/*[[[end]]]*/
	if unlikely(DeeObject_AsGenericPointer(args.ptr, &basetype, &ptr))
		goto err;
	if unlikely(get_atomic_operand(args.expected, basetype, &expected))
		goto err;

	/* Do the futex wait operation. */
	switch (basetype->st_sizeof) {

	case 1: {
		union {
			uint8_t v8[4];
			uint32_t v32;
		} expected_oldval;
		uint32_t *aligned_ptr;
		unsigned int v8_index;
		aligned_ptr = (uint32_t *)(ptr.uint & ~3);
		v8_index    = (ptr.uint & 3);
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v8[v8_index] == expected.ao_u8) {
			wait_error = DeeFutex_Wait32(aligned_ptr, expected_oldval.v32);
		} else {
			wait_error = 0;
		}
	}	break;

	case 2: {
		union {
			uint16_t v16[2];
			uint32_t v32;
		} expected_oldval;
		uint32_t *aligned_ptr;
		unsigned int v16_index;
		if unlikely(ptr.uint & 1)
			goto err_unaligned;
		aligned_ptr = (uint32_t *)(ptr.uint & ~3);
		v16_index   = (ptr.uint & 2) >> 1;
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v16[v16_index] == expected.ao_u16) {
			wait_error = DeeFutex_Wait32(aligned_ptr, expected_oldval.v32);
		} else {
			wait_error = 0;
		}
	}	break;

	case 4: {
		uint32_t true_oldval;
		if unlikely(ptr.uint & 3)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(ptr.p32), goto err);
		if (true_oldval == expected.ao_u32) {
			wait_error = DeeFutex_Wait32(ptr.pvoid, expected.ao_u32);
		} else {
			wait_error = 0;
		}
	}	break;

#if __SIZEOF_POINTER__ >= 8
	case 8: {
		uint64_t true_oldval;
		if unlikely(ptr.uint & 7)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(ptr.p64), goto err);
		if (true_oldval == expected.ao_u64) {
			wait_error = DeeFutex_Wait64(ptr.pvoid, expected.ao_u64);
		} else {
			wait_error = 0;
		}
	}	break;
#endif /* __SIZEOF_POINTER__ >= 8 */

	default:
		err_bad_futex_size(basetype->st_sizeof);
		goto err;
	}
	if unlikely(wait_error < 0)
		goto err;
	return_bool(wait_error == 0);
err_unaligned:
	err_unaligned_futex_poiner(ptr.pvoid, basetype->st_sizeof);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_futex_timedwait(size_t argc, DeeObject *const *argv) {
	int wait_error;
	union atomic_operand expected;
	union pointer ptr;
	DeeSTypeObject *basetype;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("futex_timedwait", params: """
	DeeObject *ptr;
	DeeObject *expected;
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		DeeObject *ptr;
		DeeObject *expected;
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "futex_timedwait", &args, &args.ptr, "o", _DeeArg_AsObject, &args.expected, "o", _DeeArg_AsObject, &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	if unlikely(DeeObject_AsGenericPointer(args.ptr, &basetype, &ptr))
		goto err;
	if unlikely(get_atomic_operand(args.expected, basetype, &expected))
		goto err;

	/* Do the futex wait operation. */
	switch (basetype->st_sizeof) {

	case 1: {
		union {
			uint8_t v8[4];
			uint32_t v32;
		} expected_oldval;
		uint32_t *aligned_ptr;
		unsigned int v8_index;
		aligned_ptr = (uint32_t *)(ptr.uint & ~3);
		v8_index    = (ptr.uint & 3);
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v8[v8_index] == expected.ao_u8) {
			wait_error = DeeFutex_Wait32Timed(aligned_ptr, expected_oldval.v32, args.timeout_nanoseconds);
		} else {
			wait_error = 0;
		}
	}	break;

	case 2: {
		union {
			uint16_t v16[2];
			uint32_t v32;
		} expected_oldval;
		uint32_t *aligned_ptr;
		unsigned int v16_index;
		if unlikely(ptr.uint & 1)
			goto err_unaligned;
		aligned_ptr = (uint32_t *)(ptr.uint & ~3);
		v16_index   = (ptr.uint & 2) >> 1;
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v16[v16_index] == expected.ao_u16) {
			wait_error = DeeFutex_Wait32Timed(aligned_ptr, expected_oldval.v32, args.timeout_nanoseconds);
		} else {
			wait_error = 0;
		}
	}	break;

	case 4: {
		uint32_t true_oldval;
		if unlikely(ptr.uint & 3)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(ptr.p32), goto err);
		if (true_oldval == expected.ao_u32) {
			wait_error = DeeFutex_Wait32Timed(ptr.pvoid, expected.ao_u32, args.timeout_nanoseconds);
		} else {
			wait_error = 0;
		}
	}	break;

#if __SIZEOF_POINTER__ >= 8
	case 8: {
		uint64_t true_oldval;
		if unlikely(ptr.uint & 7)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(ptr.p64), goto err);
		if (true_oldval == expected.ao_u64) {
			wait_error = DeeFutex_Wait64Timed(ptr.pvoid, expected.ao_u64, args.timeout_nanoseconds);
		} else {
			wait_error = 0;
		}
	}	break;
#endif /* __SIZEOF_POINTER__ >= 8 */

	default:
		err_bad_futex_size(basetype->st_sizeof);
		goto err;
	}
	if unlikely(wait_error < 0)
		goto err;
	return_bool(wait_error == 0);
err_unaligned:
	err_unaligned_futex_poiner(ptr.pvoid, basetype->st_sizeof);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_CTYPES_C_ATOMIC_C */
