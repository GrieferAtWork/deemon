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
#ifndef GUARD_DEX_CTYPES_C_ATOMIC_C
#define GUARD_DEX_CTYPES_C_ATOMIC_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>
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

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

#include "c_api.h" /* Prototypes... */

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* intN_t, uintN_t, uintptr_t */

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


/*[[[deemon (print_CMethod from rt.gen.unpack)("atomic_cmpxch", """
	DeeObject *ptr;
	DeeObject *oldval;
	DeeObject *newval;
	bool weak = false;
""", visi: "INTERN");]]]*/
#define c_atomic_atomic_cmpxch_params "ptr,oldval,newval,weak=!f"
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL c_atomic_atomic_cmpxch_f_impl(DeeObject *ptr, DeeObject *oldval, DeeObject *newval, bool weak);
PRIVATE WUNUSED DREF DeeObject *DCALL c_atomic_atomic_cmpxch_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ptr;
		DeeObject *oldval;
		DeeObject *newval;
		bool weak;
	} args;
	args.weak = false;
	if (DeeArg_UnpackStruct(argc, argv, "ooo|b:atomic_cmpxch", &args))
		goto err;
	return c_atomic_atomic_cmpxch_f_impl(args.ptr, args.oldval, args.newval, args.weak);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_atomic_atomic_cmpxch, &c_atomic_atomic_cmpxch_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL c_atomic_atomic_cmpxch_f_impl(DeeObject *ptr, DeeObject *oldval, DeeObject *newval, bool weak)
/*[[[end]]]*/
{
	union atomic_operand op_oldval, op_newval;
	union pointer op_ptr;
	DeeSTypeObject *basetype;
	bool result;
	if unlikely(DeeObject_AsGenericPointer(ptr, &basetype, &op_ptr))
		goto err;
	if unlikely(get_atomic_operand(oldval, basetype, &op_oldval))
		goto err;
	if unlikely(get_atomic_operand(newval, basetype, &op_newval))
		goto err;
	CTYPES_FAULTPROTECT({
		if (weak) {
			switch (basetype->st_sizeof) {
			case 1: result = atomic_cmpxch_weak(op_ptr.p8, op_oldval.ao_u8, op_newval.ao_u8); break;
			case 2: result = atomic_cmpxch_weak(op_ptr.p16, op_oldval.ao_u16, op_newval.ao_u16); break;
			case 4: result = atomic_cmpxch_weak(op_ptr.p32, op_oldval.ao_u32, op_newval.ao_u32); break;
			case 8: result = atomic_cmpxch_weak(op_ptr.p64, op_oldval.ao_u64, op_newval.ao_u64); break;
			default: __builtin_unreachable();
			}
		} else {
			switch (basetype->st_sizeof) {
			case 1: result = atomic_cmpxch(op_ptr.p8, op_oldval.ao_u8, op_newval.ao_u8); break;
			case 2: result = atomic_cmpxch(op_ptr.p16, op_oldval.ao_u16, op_newval.ao_u16); break;
			case 4: result = atomic_cmpxch(op_ptr.p32, op_oldval.ao_u32, op_newval.ao_u32); break;
			case 8: result = atomic_cmpxch(op_ptr.p64, op_oldval.ao_u64, op_newval.ao_u64); break;
			default: __builtin_unreachable();
			}
		}
	}, goto err);
	return_bool(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("atomic_cmpxch_val", """
	DeeObject *ptr;
	DeeObject *oldval;
	DeeObject *newval;
""", visi: "INTERN");]]]*/
#define c_atomic_atomic_cmpxch_val_params "ptr,oldval,newval"
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL c_atomic_atomic_cmpxch_val_f_impl(DeeObject *ptr, DeeObject *oldval, DeeObject *newval);
PRIVATE WUNUSED DREF DeeObject *DCALL c_atomic_atomic_cmpxch_val_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ptr;
		DeeObject *oldval;
		DeeObject *newval;
	} args;
	DeeArg_UnpackStruct3(err, argc, argv, "atomic_cmpxch_val", &args, &args.ptr, &args.oldval, &args.newval);
	return c_atomic_atomic_cmpxch_val_f_impl(args.ptr, args.oldval, args.newval);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_atomic_atomic_cmpxch_val, &c_atomic_atomic_cmpxch_val_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL c_atomic_atomic_cmpxch_val_f_impl(DeeObject *ptr, DeeObject *oldval, DeeObject *newval)
/*[[[end]]]*/
{
	union atomic_operand op_oldval, op_newval;
	union pointer op_ptr;
	DeeSTypeObject *basetype;
	DREF DeeStructObject *result_obj;
	if unlikely(DeeObject_AsGenericPointer(ptr, &basetype, &op_ptr))
		goto err;
	if unlikely(get_atomic_operand(oldval, basetype, &op_oldval))
		goto err;
	if unlikely(get_atomic_operand(newval, basetype, &op_newval))
		goto err;
	/* Allocate a buffer for the result (which is the *real* old value) */
	result_obj = DeeStructObject_Malloc(basetype->st_sizeof);
	if unlikely(!result_obj)
		goto err;
	CTYPES_FAULTPROTECT({
		switch (basetype->st_sizeof) {
		case 1: *(uint8_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(op_ptr.p8, op_oldval.ao_u8, op_newval.ao_u8); break;
		case 2: *(uint16_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(op_ptr.p16, op_oldval.ao_u16, op_newval.ao_u16); break;
		case 4: *(uint32_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(op_ptr.p32, op_oldval.ao_u32, op_newval.ao_u32); break;
		case 8: *(uint64_t *)DeeStruct_Data(result_obj) = atomic_cmpxch_val(op_ptr.p64, op_oldval.ao_u64, op_newval.ao_u64); break;
		default: __builtin_unreachable();
		}
	}, goto err_result_obj);
	DeeObject_Init(result_obj, DeeSType_AsType(basetype));
	return Dee_AsObject(result_obj);
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
err_result_obj:
	DeeStructObject_Free(result_obj);
#endif /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
err:
	return NULL;
}

#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#define IF_HAVE_FAULTPROTECT(x) x
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
#define IF_HAVE_FAULTPROTECT(x) /* nothing */
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */

#define CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_name, atomic_name, atomic_func)                       \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                                                 \
	c_atomic_atomic_name##_f(size_t argc, DeeObject *const *argv) {                                       \
		DeeObject *ob_ptr, *ob_addend;                                                                    \
		union atomic_operand addend;                                                                      \
		union pointer ptr;                                                                                \
		DeeSTypeObject *basetype;                                                                         \
		DREF DeeStructObject *result_obj;                                                                 \
		DeeArg_Unpack2(err, argc, argv, atomic_name, &ob_ptr, &ob_addend);                                \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))                                  \
			goto err;                                                                                     \
		if unlikely(get_atomic_operand(ob_addend, basetype, &addend))                                     \
			goto err;                                                                                     \
		/* Allocate a buffer for the result (which is the *real* old value) */                            \
		result_obj = DeeStructObject_Malloc(basetype->st_sizeof);                                         \
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
		return Dee_AsObject(result_obj);                                                                  \
	IF_HAVE_FAULTPROTECT(err_result_obj:                                                                  \
		DeeStructObject_Free(result_obj);)                                                                \
	err:                                                                                                  \
		return NULL;                                                                                      \
	}                                                                                                     \
	INTERN DEFINE_CMETHOD(c_atomic_atomic_name, &c_atomic_atomic_name##_f, METHOD_FNORMAL)
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_fetchadd, "atomic_fetchadd", atomic_fetchadd);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_fetchsub, "atomic_fetchsub", atomic_fetchsub);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_fetchand, "atomic_fetchand", atomic_fetchand);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_fetchor, "atomic_fetchor", atomic_fetchor);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_fetchxor, "atomic_fetchxor", atomic_fetchxor);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_fetchnand, "atomic_fetchnand", atomic_fetchnand);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_addfetch, "atomic_addfetch", atomic_addfetch);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_subfetch, "atomic_subfetch", atomic_subfetch);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_andfetch, "atomic_andfetch", atomic_andfetch);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_orfetch, "atomic_orfetch", atomic_orfetch);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_xorfetch, "atomic_xorfetch", atomic_xorfetch);
CYTYPES_DEFINE_ATOMIC_BINOP(c_atomic_atomic_nandfetch, "atomic_nandfetch", atomic_nandfetch);
#undef CYTYPES_DEFINE_ATOMIC_BINOP

#define CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_name, atomic_name, atomic_func) \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                               \
	c_atomic_atomic_name##_f(size_t argc, DeeObject *const *argv) {                     \
		DeeObject *ob_ptr, *ob_addend;                                                  \
		union atomic_operand addend;                                                    \
		union pointer ptr;                                                              \
		DeeSTypeObject *basetype;                                                       \
		DeeArg_Unpack2(err, argc, argv, atomic_name, &ob_ptr, &ob_addend);              \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))                \
			goto err;                                                                   \
		if unlikely(get_atomic_operand(ob_addend, basetype, &addend))                   \
			goto err;                                                                   \
		CTYPES_FAULTPROTECT({                                                           \
			switch (basetype->st_sizeof) {                                              \
			case 1: atomic_func(ptr.p8, addend.ao_u8); break;                           \
			case 2: atomic_func(ptr.p16, addend.ao_u16); break;                         \
			case 4: atomic_func(ptr.p32, addend.ao_u32); break;                         \
			case 8: atomic_func(ptr.p64, addend.ao_u64); break;                         \
			default: __builtin_unreachable();                                           \
			}                                                                           \
		}, goto err);                                                                   \
		return_none;                                                                    \
	err:                                                                                \
		return NULL;                                                                    \
	}                                                                                   \
	INTERN DEFINE_CMETHOD(c_atomic_atomic_name, &c_atomic_atomic_name##_f, METHOD_FNORMAL)
CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_add, "atomic_add", atomic_add);
CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_sub, "atomic_sub", atomic_sub);
CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_and, "atomic_and", atomic_and);
CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_or, "atomic_or", atomic_or);
CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_xor, "atomic_xor", atomic_xor);
CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_nand, "atomic_nand", atomic_nand);
CTYPES_DEFINE_ATOMIC_BINOP_VOID(c_atomic_atomic_write, "atomic_write", atomic_write);
#undef CTYPES_DEFINE_ATOMIC_BINOP_VOID

#define CTYPES_DEFINE_ATOMIC_UNOP(c_atomic_atomic_name, atomic_name, atomic_func)          \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                                  \
	c_atomic_atomic_name##_f(size_t argc, DeeObject *const *argv) {                        \
		DeeObject *ob_ptr;                                                                 \
		union pointer ptr;                                                                 \
		DeeSTypeObject *basetype;                                                          \
		DREF DeeStructObject *result_obj;                                                  \
		DeeArg_Unpack1(err, argc, argv, atomic_name, &ob_ptr);                             \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))                   \
			goto err;                                                                      \
		/* Allocate a buffer for the result (which is the *real* old value) */             \
		result_obj = DeeStructObject_Malloc(basetype->st_sizeof);                          \
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
		return Dee_AsObject(result_obj);                                                   \
	IF_HAVE_FAULTPROTECT(err_result_obj:                                                   \
		DeeStructObject_Free(result_obj);)                                                 \
	err:                                                                                   \
		return NULL;                                                                       \
	}                                                                                      \
	INTERN DEFINE_CMETHOD(c_atomic_atomic_name, &c_atomic_atomic_name##_f, METHOD_FNORMAL)
CTYPES_DEFINE_ATOMIC_UNOP(c_atomic_atomic_fetchinc, "atomic_fetchinc", atomic_fetchinc);
CTYPES_DEFINE_ATOMIC_UNOP(c_atomic_atomic_fetchdec, "atomic_fetchdec", atomic_fetchdec);
CTYPES_DEFINE_ATOMIC_UNOP(c_atomic_atomic_incfetch, "atomic_incfetch", atomic_incfetch);
CTYPES_DEFINE_ATOMIC_UNOP(c_atomic_atomic_decfetch, "atomic_decfetch", atomic_decfetch);
CTYPES_DEFINE_ATOMIC_UNOP(c_atomic_atomic_read, "atomic_read", atomic_read);
#undef CTYPES_DEFINE_ATOMIC_UNOP

#define CTYPES_DEFINE_ATOMIC_UNOP_VOID(c_atomic_atomic_name, atomic_name, atomic_func) \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                              \
	c_atomic_atomic_name##_f(size_t argc, DeeObject *const *argv) {                    \
		DeeObject *ob_ptr;                                                             \
		union pointer ptr;                                                             \
		DeeSTypeObject *basetype;                                                      \
		DeeArg_Unpack1(err, argc, argv, atomic_name, &ob_ptr);                         \
		if unlikely(DeeObject_AsGenericPointer(ob_ptr, &basetype, &ptr))               \
			goto err;                                                                  \
		CTYPES_FAULTPROTECT({                                                          \
			switch (basetype->st_sizeof) {                                             \
			case 1: atomic_func(ptr.p8); break;                                        \
			case 2: atomic_func(ptr.p16); break;                                       \
			case 4: atomic_func(ptr.p32); break;                                       \
			case 8: atomic_func(ptr.p64); break;                                       \
			default: __builtin_unreachable();                                          \
			}                                                                          \
		}, goto err);                                                                  \
		return_none;                                                                   \
	err:                                                                               \
		return NULL;                                                                   \
	}                                                                                  \
	INTERN DEFINE_CMETHOD(c_atomic_atomic_name, &c_atomic_atomic_name##_f, METHOD_FNORMAL)
CTYPES_DEFINE_ATOMIC_UNOP_VOID(c_atomic_atomic_inc, "atomic_inc", atomic_inc);
CTYPES_DEFINE_ATOMIC_UNOP_VOID(c_atomic_atomic_dec, "atomic_dec", atomic_dec);
#undef CTYPES_DEFINE_ATOMIC_UNOP_VOID


/* Futex API */

/*[[[deemon (print_CMethod from rt.gen.unpack)("futex_wakeone", "ptr:ctypes:void*", visi: "INTERN");]]]*/
#define c_atomic_futex_wakeone_params "ptr:?Aptr?Gvoid"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_atomic_futex_wakeone_f_impl(void *ptr);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_atomic_futex_wakeone_f(DeeObject *__restrict arg0) {
	union pointer ptr;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCVoid_Type, &ptr))
		goto err;
	return c_atomic_futex_wakeone_f_impl(ptr.pvoid);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_atomic_futex_wakeone, &c_atomic_futex_wakeone_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_atomic_futex_wakeone_f_impl(void *ptr)
/*[[[end]]]*/
{
	/* Because our `capi_futex_wait()' emulates 8-bit and 16-bit waits
	 * by waiting for the relevant 32-bit word, we have to match its
	 * alignment here! */
	ptr = (void *)((uintptr_t)ptr & ~3);
	DeeFutex_WakeOne(ptr);
	return_none;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("futex_wakeall", "ptr:ctypes:void*", visi: "INTERN");]]]*/
#define c_atomic_futex_wakeall_params "ptr:?Aptr?Gvoid"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_atomic_futex_wakeall_f_impl(void *ptr);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_atomic_futex_wakeall_f(DeeObject *__restrict arg0) {
	union pointer ptr;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCVoid_Type, &ptr))
		goto err;
	return c_atomic_futex_wakeall_f_impl(ptr.pvoid);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_atomic_futex_wakeall, &c_atomic_futex_wakeall_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_atomic_futex_wakeall_f_impl(void *ptr)
/*[[[end]]]*/
{
	/* Because our `capi_futex_wait()' emulates 8-bit and 16-bit waits
	 * by waiting for the relevant 32-bit word, we have to match its
	 * alignment here! */
	ptr = (void *)((uintptr_t)ptr & ~3);
	DeeFutex_WakeAll(ptr);
	return_none;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("futex_wait", "ptr,expected", visi: "INTERN");]]]*/
#define c_atomic_futex_wait_params "ptr,expected"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL c_atomic_futex_wait_f_impl(DeeObject *ptr, DeeObject *expected);
PRIVATE WUNUSED DREF DeeObject *DCALL c_atomic_futex_wait_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ptr;
		DeeObject *expected;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "futex_wait", &args, &args.ptr, &args.expected);
	return c_atomic_futex_wait_f_impl(args.ptr, args.expected);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_atomic_futex_wait, &c_atomic_futex_wait_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL c_atomic_futex_wait_f_impl(DeeObject *ptr, DeeObject *expected)
/*[[[end]]]*/
{
	int wait_error;
	union atomic_operand op_expected;
	union pointer op_ptr;
	DeeSTypeObject *basetype;
	if unlikely(DeeObject_AsGenericPointer(ptr, &basetype, &op_ptr))
		goto err;
	if unlikely(get_atomic_operand(expected, basetype, &op_expected))
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
		aligned_ptr = (uint32_t *)(op_ptr.uint & ~3);
		v8_index    = (op_ptr.uint & 3);
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v8[v8_index] == op_expected.ao_u8) {
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
		if unlikely(op_ptr.uint & 1)
			goto err_unaligned;
		aligned_ptr = (uint32_t *)(op_ptr.uint & ~3);
		v16_index   = (op_ptr.uint & 2) >> 1;
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v16[v16_index] == op_expected.ao_u16) {
			wait_error = DeeFutex_Wait32(aligned_ptr, expected_oldval.v32);
		} else {
			wait_error = 0;
		}
	}	break;

	case 4: {
		uint32_t true_oldval;
		if unlikely(op_ptr.uint & 3)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(op_ptr.p32), goto err);
		if (true_oldval == op_expected.ao_u32) {
			wait_error = DeeFutex_Wait32(op_ptr.pvoid, op_expected.ao_u32);
		} else {
			wait_error = 0;
		}
	}	break;

#if __SIZEOF_POINTER__ >= 8
	case 8: {
		uint64_t true_oldval;
		if unlikely(op_ptr.uint & 7)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(op_ptr.p64), goto err);
		if (true_oldval == op_expected.ao_u64) {
			wait_error = DeeFutex_Wait64(op_ptr.pvoid, op_expected.ao_u64);
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
	err_unaligned_futex_poiner(op_ptr.pvoid, basetype->st_sizeof);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("futex_timedwait", "ptr,expected, uint64_t timeout_nanoseconds", visi: "INTERN");]]]*/
#define c_atomic_futex_timedwait_params "ptr,expected,timeout_nanoseconds:?Dint"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL c_atomic_futex_timedwait_f_impl(DeeObject *ptr, DeeObject *expected, uint64_t timeout_nanoseconds);
PRIVATE WUNUSED DREF DeeObject *DCALL c_atomic_futex_timedwait_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ptr;
		DeeObject *expected;
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "futex_timedwait", &args, &args.ptr, "o", _DeeArg_AsObject, &args.expected, "o", _DeeArg_AsObject, &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
	return c_atomic_futex_timedwait_f_impl(args.ptr, args.expected, args.timeout_nanoseconds);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_atomic_futex_timedwait, &c_atomic_futex_timedwait_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL c_atomic_futex_timedwait_f_impl(DeeObject *ptr, DeeObject *expected, uint64_t timeout_nanoseconds)
/*[[[end]]]*/
{
	int wait_error;
	union atomic_operand op_expected;
	union pointer op_ptr;
	DeeSTypeObject *basetype;
	if unlikely(DeeObject_AsGenericPointer(ptr, &basetype, &op_ptr))
		goto err;
	if unlikely(get_atomic_operand(expected, basetype, &op_expected))
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
		aligned_ptr = (uint32_t *)(op_ptr.uint & ~3);
		v8_index    = (op_ptr.uint & 3);
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v8[v8_index] == op_expected.ao_u8) {
			wait_error = DeeFutex_Wait32Timed(aligned_ptr, expected_oldval.v32, timeout_nanoseconds);
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
		if unlikely(op_ptr.uint & 1)
			goto err_unaligned;
		aligned_ptr = (uint32_t *)(op_ptr.uint & ~3);
		v16_index   = (op_ptr.uint & 2) >> 1;
		CTYPES_FAULTPROTECT(expected_oldval.v32 = atomic_read(aligned_ptr), goto err);
		if (expected_oldval.v16[v16_index] == op_expected.ao_u16) {
			wait_error = DeeFutex_Wait32Timed(aligned_ptr, expected_oldval.v32, timeout_nanoseconds);
		} else {
			wait_error = 0;
		}
	}	break;

	case 4: {
		uint32_t true_oldval;
		if unlikely(op_ptr.uint & 3)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(op_ptr.p32), goto err);
		if (true_oldval == op_expected.ao_u32) {
			wait_error = DeeFutex_Wait32Timed(op_ptr.pvoid, op_expected.ao_u32, timeout_nanoseconds);
		} else {
			wait_error = 0;
		}
	}	break;

#if __SIZEOF_POINTER__ >= 8
	case 8: {
		uint64_t true_oldval;
		if unlikely(op_ptr.uint & 7)
			goto err_unaligned;
		CTYPES_FAULTPROTECT(true_oldval = atomic_read(op_ptr.p64), goto err);
		if (true_oldval == op_expected.ao_u64) {
			wait_error = DeeFutex_Wait64Timed(op_ptr.pvoid, op_expected.ao_u64, timeout_nanoseconds);
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
	err_unaligned_futex_poiner(op_ptr.pvoid, basetype->st_sizeof);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_CTYPES_C_ATOMIC_C */
