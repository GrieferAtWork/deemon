/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#define EXEC_SAFE 1
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/cell.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>
#include <hybrid/unaligned.h>

#include "../objects/seq/varkwds.h"
#include "../runtime/runtime_error.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

__pragma_GCC_diagnostic_push
__pragma_GCC_diagnostic_ignored(Wunused_label)
__pragma_GCC_diagnostic_ignored(Wsign_compare)
__pragma_GCC_diagnostic_ignored(Wint_in_bool_context)
__pragma_GCC_diagnostic_ignored(MSconditional_expression_is_constant)

#ifdef _MSC_VER
/* Make sure to enable optimizations for the interpreter, as MSVC will otherwise
 * put every local variable defined below into a non-aliased stack-address, which
 * will drastically increase the required frame-size to the point where deemon
 * could crash from otherwise valid constructs, such as an execution depth of
 * only ~100 function calls, which is something that could legitimately happen
 * in valid code. */
#pragma optimize("ts", on)
#endif /* _MSC_VER */

#if ((defined(EXEC_SAFE) && defined(EXEC_FAST)) || \
     (!defined(EXEC_SAFE) && !defined(EXEC_FAST)))
#error "Invalid configuration. - Must either define `EXEC_SAFE' or `EXEC_FAST'"
#endif

#ifndef CONFIG_COMPILER_HAVE_ADDRESSIBLE_LABELS
#define USE_SWITCH
#endif /* !CONFIG_COMPILER_HAVE_ADDRESSIBLE_LABELS */

DECL_BEGIN

#ifndef FILE_SHL_DECLARED
#define FILE_SHL_DECLARED 1
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
file_shl(DeeObject *self,
         DeeObject *some_object);
#endif /* !FILE_SHL_DECLARED */

#ifndef OBJECT_TATTR_DECLARED
#define OBJECT_TATTR_DECLARED 1
INTDEF WUNUSED DREF DeeObject *(DCALL DeeObject_TGetAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, /*String*/ DeeObject *__restrict attr_name);
INTDEF int (DCALL DeeObject_TDelAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, /*String*/ DeeObject *__restrict attr_name);
INTDEF int (DCALL DeeObject_TSetAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, /*String*/ DeeObject *__restrict attr_name, DeeObject *__restrict value);
INTDEF WUNUSED DREF DeeObject *(DCALL DeeObject_TCallAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, /*String*/ DeeObject *__restrict attr_name, size_t argc, DeeObject *const *argv);
#endif /* !OBJECT_TATTR_DECLARED */


#define construct_varkwds_mapping() \
	construct_varkwds_mapping_impl(code, frame)
#ifndef CONSTRUCT_VARKWDS_MAPPING_IMPL_DEFINED
#define CONSTRUCT_VARKWDS_MAPPING_IMPL_DEFINED
PRIVATE WUNUSED DREF DeeObject *ATTR_FASTCALL
construct_varkwds_mapping_impl(DeeCodeObject *__restrict code,
                               struct code_frame *__restrict frame) {
	struct code_frame_kwds *kwds;
	DREF DeeObject *result;
	DeeObject *kw;
	kwds = frame->cf_kw;
	ASSERT(kwds != NULL);
	kw = kwds->fk_kw;
	if likely(DeeKwds_Check(kw)) {
		/* Most common case: Must create a wrapper around the kwds/argv hybrid descriptor,
		 *                   but exclude any keyword also found as part of our code's
		 *                   keyword list.
		 * >> function foo(x, y, **kw) {
		 * >> 	print repr kw;
		 * >> }
		 * >> foo(x: 10, y: 20, z: 30); // { "z": 30 }
		 * Semantically comparable to:
		 * >> return rt.RoDict(
		 * >> 	for (local key, id: kw)
		 * >> 		if (key !in __code__.__kwds__)
		 * >> 			(key, __argv__[(#__argv__ - #__code__.__kwds__) + id])
		 * >> );
		 */
		result = BlackListVarkwds_New(code,
		                              frame->cf_argc,
		                              (DeeKwdsObject *)kw,
		                              frame->cf_argv + frame->cf_argc);
	} else {
		/* Special case: create a proxy-mapping object for `kw' that get rids
		 *               of all keys that are equal to one of the strings found
		 *               within our keyword list.
		 * Semantically comparable to:
		 * >> return rt.RoDict(
		 * >> 	for (local key, item: kw)
		 * >> 		if (key !in __code__.__kwds__)
		 * >> 			(key, item)
		 * >> );
		 */
		result = BlackListMapping_New(code, frame->cf_argc, kw);
	}
	return result;
}
#endif /* !CONSTRUCT_VARKWDS_MAPPING_IMPL_DEFINED */



/* @return: * :        Prefixed object pointer (dereferences to non-NULL)
 * @return: NULL:      An error occurred
 * @return: ITER_DONE: The used prefix does not support pointer addressing. */
#ifdef EXEC_FAST
#define get_prefix_object_ptr() get_prefix_object_ptr_fast(frame, code, sp)
PRIVATE WUNUSED DREF DeeObject **ATTR_FASTCALL
get_prefix_object_ptr_fast(struct code_frame *__restrict frame,
                           DeeCodeObject *__restrict code,
                           DeeObject **__restrict sp)
#else /* EXEC_FAST */
#define get_prefix_object_ptr() get_prefix_object_ptr_safe(frame, code, sp)
PRIVATE WUNUSED DREF DeeObject **ATTR_FASTCALL
get_prefix_object_ptr_safe(struct code_frame *__restrict frame,
                           DeeCodeObject *__restrict code,
                           DeeObject **__restrict sp)
#endif /* !EXEC_FAST */
{
	DREF DeeObject **result;
	instruction_t *ip = frame->cf_ip;
	uint16_t imm_val;
	switch (*ip++) {

	case ASM_STACK:
		imm_val = *(uint8_t *)ip;
do_get_stack:
#ifdef EXEC_SAFE
		if unlikely((frame->cf_stack + imm_val) >= sp) {
			frame->cf_sp = sp;
			err_srt_invalid_sp(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT((frame->cf_stack + imm_val) < sp);
#endif /* !EXEC_SAFE */
		result = &frame->cf_stack[imm_val];
		break;

	case ASM_LOCAL:
		imm_val = UNALIGNED_GETLE8(ip + 0);
do_get_local:
#ifdef EXEC_SAFE
		if unlikely(imm_val >= code->co_localc) {
			frame->cf_sp = sp;
			err_srt_invalid_locale(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < code->co_localc);
#endif /* !EXEC_SAFE */
		result = &frame->cf_frame[imm_val];
		if unlikely(!*result) {
			err_unbound_local(code, frame->cf_ip, imm_val);
			return NULL;
		}
		break;

	case ASM_EXTENDED1:
		switch (*ip++) {

		case ASM16_STACK & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_stack;
		case ASM16_LOCAL & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_local;

		default:
			return (DREF DeeObject **)ITER_DONE;
		}
		break;

	default:
		return (DREF DeeObject **)ITER_DONE;
	}
	return result;
}

#ifdef EXEC_FAST
#define get_prefix_object() get_prefix_object_fast(frame, code, sp)
PRIVATE WUNUSED DREF DeeObject *ATTR_FASTCALL
get_prefix_object_fast(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp)
#else /* EXEC_SAFE */
#define get_prefix_object() get_prefix_object_safe(frame, code, sp)
PRIVATE WUNUSED DREF DeeObject *ATTR_FASTCALL
get_prefix_object_safe(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp)
#endif /* !EXEC_SAFE */
{
	DREF DeeObject *result;
	instruction_t *ip = frame->cf_ip;
	DeeModuleObject *mod;
	uint16_t imm_val;
	switch (*ip++) {

	case ASM_STACK:
		imm_val = *(uint8_t *)ip;
do_get_stack:
#ifdef EXEC_SAFE
		if unlikely((frame->cf_stack + imm_val) >= sp) {
			frame->cf_sp = sp;
			err_srt_invalid_sp(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT((frame->cf_stack + imm_val) < sp);
#endif /* !EXEC_SAFE */
		result = frame->cf_stack[imm_val];
		ASSERT_OBJECT(result);
		Dee_Incref(result);
		break;

	case ASM_STATIC:
		imm_val = *(uint8_t *)ip;
do_get_static:
#ifdef EXEC_SAFE
		if unlikely(imm_val >= code->co_staticc) {
			frame->cf_sp = sp;
			err_srt_invalid_static(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < code->co_staticc);
#endif /* !EXEC_SAFE */
		DeeCode_StaticLockRead(code);
		result = code->co_staticv[imm_val];
		ASSERT_OBJECT(result);
		Dee_Incref(result);
		DeeCode_StaticLockEndRead(code);
		break;

	case ASM_EXTERN:
#ifdef EXEC_SAFE
		imm_val = UNALIGNED_GETLE8(ip + 1);
		if unlikely(UNALIGNED_GETLE8(ip + 0) >= code->co_module->mo_importc) {
err_invalid_extern:
			frame->cf_sp = sp;
			err_srt_invalid_extern(frame, UNALIGNED_GETLE8(ip + 0), imm_val);
			return NULL;
		}
		mod = code->co_module->mo_importv[UNALIGNED_GETLE8(ip + 0)];
		if unlikely(imm_val >= mod->mo_globalc)
			goto err_invalid_extern;
#else /* EXEC_SAFE */
		ASSERT(UNALIGNED_GETLE8(ip + 0) < code->co_module->mo_importc);
		mod  = code->co_module->mo_importv[UNALIGNED_GETLE8(ip + 0)];
		imm_val = UNALIGNED_GETLE8(ip + 1);
		ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
		goto do_get_module_object;
	case ASM_GLOBAL:
		imm_val = UNALIGNED_GETLE8(ip + 0);
do_get_global:
		mod = code->co_module;
#ifdef EXEC_SAFE
		if unlikely(imm_val >= mod->mo_globalc) {
			frame->cf_sp = sp;
			err_srt_invalid_global(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
do_get_module_object:
		DeeModule_LockRead(mod);
		result = mod->mo_globalv[imm_val];
		Dee_XIncref(result);
		DeeModule_LockEndRead(mod);
		if unlikely(!result)
			err_unbound_global(mod, imm_val);
		ASSERT_OBJECT_OPT(result);
		break;

	case ASM_LOCAL:
		imm_val = UNALIGNED_GETLE8(ip + 0);
do_get_local:
#ifdef EXEC_SAFE
		if unlikely(imm_val >= code->co_localc) {
			frame->cf_sp = sp;
			err_srt_invalid_locale(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < code->co_localc);
#endif /* !EXEC_SAFE */
		result = frame->cf_frame[imm_val];
		if likely(result) {
			Dee_Incref(result);
		} else {
			err_unbound_local(code, frame->cf_ip, imm_val);
		}
		break;

	case ASM_EXTENDED1:
		switch (*ip++) {

		case ASM16_STACK & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_stack;

		case ASM16_STATIC & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_static;

		case ASM16_EXTERN & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
#ifdef EXEC_SAFE
			if unlikely(imm_val >= code->co_module->mo_importc) {
err_invalid_extern16:
				frame->cf_sp = sp;
				err_srt_invalid_extern(frame, UNALIGNED_GETLE16(ip + 0), imm_val);
				return NULL;
			}
			mod  = code->co_module->mo_importv[imm_val];
			imm_val = UNALIGNED_GETLE16(ip + 2);
			if unlikely(imm_val >= mod->mo_globalc)
				goto err_invalid_extern16;
#else /* EXEC_SAFE */
			ASSERT(imm_val < code->co_module->mo_importc);
			mod  = code->co_module->mo_importv[imm_val];
			imm_val = UNALIGNED_GETLE16(ip + 2);
			ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
			goto do_get_module_object;

		case ASM16_GLOBAL & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_global;

		case ASM16_LOCAL & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_local;

		default:
#ifdef EXEC_SAFE
			goto ill_instr;
#else /* EXEC_SAFE */
			__builtin_unreachable();
#endif /* !EXEC_SAFE */
		}
		break;

	default:
#ifdef EXEC_SAFE
ill_instr:
		err_illegal_instruction(code, frame->cf_ip);
		return NULL;
#else /* EXEC_SAFE */
		__builtin_unreachable();
#endif /* !EXEC_SAFE */
	}
	return result;
}


/* NOTE: A reference to `value' is only inherited upon success (return != NULL) */
#ifdef EXEC_FAST
#define xch_prefix_object(v) xch_prefix_object_fast(frame, code, sp, v)
PRIVATE WUNUSED DREF DeeObject *ATTR_FASTCALL
xch_prefix_object_fast(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#else /* EXEC_FAST */
#define xch_prefix_object(v) xch_prefix_object_safe(frame, code, sp, v)
PRIVATE WUNUSED DREF DeeObject *ATTR_FASTCALL
xch_prefix_object_safe(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#endif /* !EXEC_FAST */
{
	DREF DeeObject *result;
	instruction_t *ip = frame->cf_ip;
	DeeModuleObject *mod;
	uint16_t imm_val;
	switch (*ip++) {

	case ASM_STACK:
		imm_val = *(uint8_t *)ip;
do_get_stack:
#ifdef EXEC_SAFE
		if unlikely((frame->cf_stack + imm_val) >= sp) {
			frame->cf_sp = sp;
			err_srt_invalid_sp(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT((frame->cf_stack + imm_val) < sp);
#endif /* !EXEC_SAFE */
		result                   = frame->cf_stack[imm_val]; /* Inherit reference. */
		frame->cf_stack[imm_val] = value;                    /* Inherit reference. */
		ASSERT_OBJECT(result);
		break;

	case ASM_STATIC:
		imm_val = *(uint8_t *)ip;
do_get_static:
#ifdef EXEC_SAFE
		if unlikely(imm_val >= code->co_staticc) {
			frame->cf_sp = sp;
			err_srt_invalid_static(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < code->co_staticc);
#endif /* !EXEC_SAFE */
		DeeCode_StaticLockWrite(code);
		result = code->co_staticv[imm_val]; /* Inherit reference. */
		code->co_staticv[imm_val] = value;  /* Inherit reference. */
		DeeCode_StaticLockEndWrite(code);
		ASSERT_OBJECT(result);
		break;

	case ASM_EXTERN:
#ifdef EXEC_SAFE
		imm_val = UNALIGNED_GETLE8(ip + 1);
		if unlikely(UNALIGNED_GETLE8(ip + 0) >= code->co_module->mo_importc) {
err_invalid_extern:
			frame->cf_sp = sp;
			err_srt_invalid_extern(frame, UNALIGNED_GETLE8(ip + 0), imm_val);
			return NULL;
		}
		mod = code->co_module->mo_importv[UNALIGNED_GETLE8(ip + 0)];
		if unlikely(imm_val >= mod->mo_globalc)
			goto err_invalid_extern;
#else /* EXEC_SAFE */
		ASSERT(UNALIGNED_GETLE8(ip + 0) < code->co_module->mo_importc);
		mod  = code->co_module->mo_importv[UNALIGNED_GETLE8(ip + 0)];
		imm_val = UNALIGNED_GETLE8(ip + 1);
		ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
		goto do_get_module_object;

	case ASM_GLOBAL:
		imm_val = UNALIGNED_GETLE8(ip + 0);
do_get_global:
		mod = code->co_module;
#ifdef EXEC_SAFE
		if unlikely(imm_val >= mod->mo_globalc) {
			frame->cf_sp = sp;
			err_srt_invalid_global(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
do_get_module_object:
		DeeModule_LockWrite(mod);
		result = mod->mo_globalv[imm_val]; /* Inherit reference. */
		if unlikely(!result) {
			DeeModule_LockEndWrite(mod);
			err_unbound_global(mod, imm_val);
			return NULL;
		}
		mod->mo_globalv[imm_val] = value; /* Inherit reference. */
		DeeModule_LockEndWrite(mod);
		ASSERT_OBJECT(result);
		break;

	case ASM_LOCAL:
		imm_val = UNALIGNED_GETLE8(ip + 0);
do_get_local:
#ifdef EXEC_SAFE
		if unlikely(imm_val >= code->co_localc) {
			frame->cf_sp = sp;
			err_srt_invalid_locale(frame, imm_val);
			return NULL;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < code->co_localc);
#endif /* !EXEC_SAFE */
		result = frame->cf_frame[imm_val]; /* Inherit reference. */
		if likely(result) {
			frame->cf_frame[imm_val] = value; /* Inherit reference. */
		} else {
			err_unbound_local(code, frame->cf_ip, imm_val);
		}
		break;

	case ASM_EXTENDED1:
		switch (*ip++) {

		case ASM16_STACK & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_stack;

		case ASM16_STATIC & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_static;

		case ASM16_EXTERN & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
#ifdef EXEC_SAFE
			if unlikely(imm_val >= code->co_module->mo_importc) {
err_invalid_extern16:
				frame->cf_sp = sp;
				err_srt_invalid_extern(frame, UNALIGNED_GETLE16(ip + 0), imm_val);
				return NULL;
			}
			mod  = code->co_module->mo_importv[imm_val];
			imm_val = UNALIGNED_GETLE16(ip + 2);
			if unlikely(imm_val >= mod->mo_globalc)
				goto err_invalid_extern16;
#else /* EXEC_SAFE */
			ASSERT(imm_val < code->co_module->mo_importc);
			mod  = code->co_module->mo_importv[imm_val];
			imm_val = UNALIGNED_GETLE16(ip + 2);
			ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
			goto do_get_module_object;

		case ASM16_GLOBAL & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_global;

		case ASM16_LOCAL & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_get_local;

		default:
#ifdef EXEC_SAFE
			goto ill_instr;
#else /* EXEC_SAFE */
			__builtin_unreachable();
#endif /* !EXEC_SAFE */
		}
		break;

	default:
#ifdef EXEC_SAFE
ill_instr:
		err_illegal_instruction(code, frame->cf_ip);
		return NULL;
#else /* EXEC_SAFE */
		__builtin_unreachable();
#endif /* !EXEC_SAFE */
	}
	return result;
}


/* NOTE: A reference to `value' is _always_ inherited */
#ifdef EXEC_FAST
#define set_prefix_object(v) unlikely(set_prefix_object_fast(frame, code, sp, v))
PRIVATE int ATTR_FASTCALL
set_prefix_object_fast(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#else /* EXEC_FAST */
#define set_prefix_object(v) unlikely(set_prefix_object_safe(frame, code, sp, v))
PRIVATE int ATTR_FASTCALL
set_prefix_object_safe(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#endif /* !EXEC_FAST */
{
	DREF DeeObject *old_value;
	instruction_t *ip = frame->cf_ip;
	DeeModuleObject *mod;
	uint16_t imm_val;
	ASSERT_OBJECT(value);
	switch (*ip++) {

	case ASM_STACK:
		imm_val = *(uint8_t *)ip;
do_set_stack:
#ifdef EXEC_SAFE
		if unlikely((frame->cf_stack + imm_val) >= sp) {
			frame->cf_sp = sp;
			err_srt_invalid_sp(frame, imm_val);
			Dee_Decref(value);
			return -1;
		}
#else /* EXEC_SAFE */
		ASSERT((frame->cf_stack + imm_val) < sp);
#endif /* !EXEC_SAFE */
		old_value                = frame->cf_stack[imm_val];
		frame->cf_stack[imm_val] = value;
		Dee_Decref(old_value);
		break;

	case ASM_STATIC:
		imm_val = *(uint8_t *)ip;
do_set_static:
#ifdef EXEC_SAFE
		if unlikely(imm_val >= code->co_staticc) {
			frame->cf_sp = sp;
			err_srt_invalid_static(frame, imm_val);
			Dee_Decref(value);
			return -1;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < code->co_staticc);
#endif /* !EXEC_SAFE */
		DeeCode_StaticLockWrite(code);
		old_value                 = code->co_staticv[imm_val];
		code->co_staticv[imm_val] = value;
		ASSERT_OBJECT(old_value);
		DeeCode_StaticLockEndWrite(code);
		Dee_Decref(old_value);
		break;

	case ASM_EXTERN:
#ifdef EXEC_SAFE
		imm_val = UNALIGNED_GETLE8(ip + 1);
		if unlikely(UNALIGNED_GETLE8(ip + 0) >= code->co_module->mo_importc) {
err_invalid_extern:
			frame->cf_sp = sp;
			err_srt_invalid_extern(frame, UNALIGNED_GETLE8(ip + 0), imm_val);
			Dee_Decref(value);
			return -1;
		}
		mod = code->co_module->mo_importv[UNALIGNED_GETLE8(ip + 0)];
		if unlikely(imm_val >= mod->mo_globalc)
			goto err_invalid_extern;
#else /* EXEC_SAFE */
		ASSERT(UNALIGNED_GETLE8(ip + 0) < code->co_module->mo_importc);
		mod  = code->co_module->mo_importv[UNALIGNED_GETLE8(ip + 0)];
		imm_val = UNALIGNED_GETLE8(ip + 1);
		ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
		goto do_set_module_object;

	case ASM_GLOBAL:
		imm_val = UNALIGNED_GETLE8(ip + 0);
do_set_global:
		mod = code->co_module;
#ifdef EXEC_SAFE
		if unlikely(imm_val >= mod->mo_globalc) {
			frame->cf_sp = sp;
			err_srt_invalid_global(frame, imm_val);
			Dee_Decref(value);
			return -1;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
do_set_module_object:
		DeeModule_LockWrite(mod);
		old_value = mod->mo_globalv[imm_val];
		mod->mo_globalv[imm_val] = value;
		DeeModule_LockEndWrite(mod);
		Dee_XDecref(old_value);
		break;

	case ASM_LOCAL:
		imm_val = UNALIGNED_GETLE8(ip + 0);
do_set_local:
#ifdef EXEC_SAFE
		if unlikely(imm_val >= code->co_localc) {
			frame->cf_sp = sp;
			err_srt_invalid_locale(frame, imm_val);
			Dee_Decref(value);
			return -1;
		}
#else /* EXEC_SAFE */
		ASSERT(imm_val < code->co_localc);
#endif /* !EXEC_SAFE */
		old_value                = frame->cf_frame[imm_val];
		frame->cf_frame[imm_val] = value;
		Dee_XDecref(old_value);
		break;

	case ASM_EXTENDED1:
		switch (*ip++) {

		case ASM16_STACK & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_set_stack;

		case ASM16_STATIC & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_set_static;

		case ASM16_EXTERN & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
#ifdef EXEC_SAFE
			if unlikely(imm_val >= code->co_module->mo_importc) {
err_invalid_extern16:
				frame->cf_sp = sp;
				err_srt_invalid_extern(frame, UNALIGNED_GETLE16(ip + 0), imm_val);
				Dee_Decref(value);
				return -1;
			}
			mod  = code->co_module->mo_importv[imm_val];
			imm_val = UNALIGNED_GETLE16(ip + 2);
			if unlikely(imm_val >= mod->mo_globalc)
				goto err_invalid_extern16;
#else /* EXEC_SAFE */
			ASSERT(imm_val < code->co_module->mo_importc);
			mod  = code->co_module->mo_importv[imm_val];
			imm_val = UNALIGNED_GETLE16(ip + 2);
			ASSERT(imm_val < mod->mo_globalc);
#endif /* !EXEC_SAFE */
			goto do_set_module_object;

		case ASM16_GLOBAL & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_set_global;

		case ASM16_LOCAL & 0xff:
			imm_val = UNALIGNED_GETLE16(ip + 0);
			goto do_set_local;

#ifdef EXEC_SAFE
		default:
			goto ill_instr;
#else /* EXEC_SAFE */
		default:
			__builtin_unreachable();
#endif /* !EXEC_SAFE */
		}
		break;

	default:
#ifdef EXEC_SAFE
ill_instr:
		Dee_Decref(value);
		err_illegal_instruction(code, frame->cf_ip);
		return -1;
#else /* EXEC_SAFE */
		__builtin_unreachable();
#endif /* !EXEC_SAFE */
	}
	return 0;
}

#ifdef EXEC_FAST
PUBLIC NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameFast(struct code_frame *__restrict frame)
#else /* EXEC_FAST */
PUBLIC NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameSafe(struct code_frame *__restrict frame)
#endif /* !EXEC_FAST */
{
#ifndef USE_SWITCH
#ifndef __INTELLISENSE__
#include "code-exec-targets.c.inl"
#endif /* !__INTELLISENSE__ */
#endif /* !USE_SWITCH */
	register union {
		instruction_t *ptr;
		uint8_t       *u8;
		int8_t        *s8;
		uint16_t      *u16;
		uint32_t      *u32;
		int16_t       *s16;
		int32_t       *s32;
	} ip;
#define READ_imm8()   (*ip.u8++)
#define READ_Simm8()  (*ip.s8++)
#define READ_imm16()            UNALIGNED_GETLE16(ip.u16++)
#define READ_Simm16() ((int16_t)UNALIGNED_GETLE16(ip.u16++))
#define READ_imm32()            UNALIGNED_GETLE32(ip.u32++)
#define READ_Simm32() ((int32_t)UNALIGNED_GETLE32(ip.u32++))
	register DeeObject **sp;
	register DeeCodeObject *code;
	register uint16_t imm_val;
	uint16_t imm_val2;
	DeeThreadObject *this_thread = DeeThread_Self();
	uint16_t except_recursion    = this_thread->t_exceptsz;
#ifdef _MSC_VER
	/* MSVC is too dumb to take advantage of the C standard
	 * and optimize away inner-scope variables that take the
	 * address of a local variable, and have multiple such
	 * scopes share the same memory location for said variable:
	 * >> {
	 * >>     int x = 4;
	 * >>     printf("%p\n", &x);
	 * >> }
	 * >> {
	 * >>     int y = 5;
	 * >>     printf("%p\n", &y); // Regardless of optimization level, MSVC refuses to have
	 * >>                         // `y' share the same memory location with `x', even though
	 * >>                         // The C standard 100% allows a compiler to do this.
	 * >> }
	 *
	 * From what I understand, Microsoft actually has to do this due to some
	 * crappy, but expensive and powerful programs that get compiled using
	 * their compiler (*cough* NT Kernel *cough*), which actually need this
	 * behavior, presumably due to code like this (which any C compiler with
	 * proper optimization wouldn't allow you to do):
	 * >> PCHAR pMessage;
	 * >> pMessage = LookupErrorMessage(dwErrorCode);
	 * >> if (pMessage == NULL) {
	 * >>     CHAR cBuffer[1024];
	 * >>     FormatErrorMessage(cBuffer, "Unknown Error %X", dwErrorCode);
	 * >>     pMessage = cBuffer; // WRONG!!! WRONG!!! WRONG!!! If you do this, you're an idiot
	 * >> }
	 * >> LogSystemError(pMessage);
	 *
	 * Anyways: since I can't change what has already been decided, and since this
	 * ERROR (Yes, I'd call this an actual compiler Error) probably will never get
	 * fixed because MS is way too stuck in its ways, the only thing we can do is
	 * try to use the same variable for all the usage instances of `prefix_ob'
	 */
	DREF DeeObject *prefix_ob;
#define USING_PREFIX_OBJECT /* nothing */
#define NEED_UNIVERSAL_PREFIX_OB_WORKAROUND
#else /* _MSC_VER */
#define USING_PREFIX_OBJECT DREF DeeObject *prefix_ob;
#endif /* !_MSC_VER */


	code = frame->cf_func->fo_code;
	ASSERT((this_thread->t_execsz != 0) ==
	       (this_thread->t_exec != NULL));

#ifdef CONFIG_HAVE_EXEC_ALTSTACK
#if (DEE_EXEC_ALTSTACK_PERIOD & (DEE_EXEC_ALTSTACK_PERIOD - 1)) == 0
#define IS_ALTSTACK_PERIOD(x) (((x) & (DEE_EXEC_ALTSTACK_PERIOD - 1)) == (DEE_EXEC_ALTSTACK_PERIOD - 1))
#else /* (DEE_EXEC_ALTSTACK_PERIOD & (DEE_EXEC_ALTSTACK_PERIOD - 1)) == 0 */
#define IS_ALTSTACK_PERIOD(x) (((x) % DEE_EXEC_ALTSTACK_PERIOD) == (DEE_EXEC_ALTSTACK_PERIOD - 1))
#endif /* (DEE_EXEC_ALTSTACK_PERIOD & (DEE_EXEC_ALTSTACK_PERIOD - 1)) != 0 */
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */

	/* Limit `this_thread->t_execsz' and throw an
	 * Error.RuntimeError.StackOverflow' if it exceeds that limit. */
	if unlikely(this_thread->t_execsz >= DeeExec_StackLimit) {
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
		if (IS_ALTSTACK_PERIOD(this_thread->t_execsz) &&
		    this_thread->t_exec == frame)
			this_thread->t_exec = frame->cf_prev;
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */
		DeeError_Throwf(&DeeError_StackOverflow, "Stack overflow");
		return NULL;
	}

#ifdef CONFIG_HAVE_EXEC_ALTSTACK
	if unlikely(IS_ALTSTACK_PERIOD(this_thread->t_execsz)) {
		if (this_thread->t_exec == frame)
			goto inc_execsz_start;
		frame->cf_prev      = this_thread->t_exec;
		this_thread->t_exec = frame;
		/* Execute on to an alternate stack. */
#ifdef EXEC_SAFE
		return DeeCode_ExecFrameSafeAltStack(frame);
#else /* EXEC_SAFE */
		return DeeCode_ExecFrameFastAltStack(frame);
#endif /* !EXEC_SAFE */
	} else
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */
	{
#ifndef NDEBUG
		ASSERTF(frame->cf_prev == CODE_FRAME_NOT_EXECUTING,
		        "Frame is already being executed");
#endif /* !NDEBUG */
		ASSERT(this_thread->t_exec != frame);
		/* Hook frame into the thread-local execution stack. */
		frame->cf_prev      = this_thread->t_exec;
		this_thread->t_exec = frame;
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
inc_execsz_start:
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */
		++this_thread->t_execsz;
	}

	ip.ptr = frame->cf_ip;
	sp     = frame->cf_sp;
	ASSERT(ip.ptr >= code->co_code &&
	       ip.ptr < code->co_code + code->co_codebytes);

#define REFimm                frame->cf_func->fo_refv[imm_val]
#define LOCALimm              frame->cf_frame[imm_val]
#define EXTERNimm             code->co_module->mo_importv[imm_val]->mo_globalv[imm_val2]
#define GLOBALimm             code->co_module->mo_globalv[imm_val]
#define STATICimm             code->co_staticv[imm_val]
#define CONSTimm              code->co_staticv[imm_val]
#define CONSTimm2             code->co_staticv[imm_val2]
#define EXTERN_LOCKREAD()     DeeModule_LockRead(code->co_module->mo_importv[imm_val])
#define EXTERN_LOCKENDREAD()  DeeModule_LockEndRead(code->co_module->mo_importv[imm_val])
#define EXTERN_LOCKWRITE()    DeeModule_LockWrite(code->co_module->mo_importv[imm_val])
#define EXTERN_LOCKENDWRITE() DeeModule_LockEndWrite(code->co_module->mo_importv[imm_val])
#define GLOBAL_LOCKREAD()     DeeModule_LockRead(code->co_module)
#define GLOBAL_LOCKENDREAD()  DeeModule_LockEndRead(code->co_module)
#define GLOBAL_LOCKWRITE()    DeeModule_LockWrite(code->co_module)
#define GLOBAL_LOCKENDWRITE() DeeModule_LockEndWrite(code->co_module)
#define STATIC_LOCKREAD()     DeeCode_StaticLockRead(code)
#define STATIC_LOCKENDREAD()  DeeCode_StaticLockEndRead(code)
#define STATIC_LOCKWRITE()    DeeCode_StaticLockWrite(code)
#define STATIC_LOCKENDWRITE() DeeCode_StaticLockEndWrite(code)
#define THIS                  (frame->cf_this)
#define TOP                   sp[-1]
#define FIRST                 sp[-1]
#define SECOND                sp[-2]
#define THIRD                 sp[-3]
#define FOURTH                sp[-4]
#define POP()                 (*--sp)
#define POPREF()              (--sp, Dee_Decref(*sp))
#define PUSH(ob)              (*sp = (ob), ++sp)
#define PUSHREF(ob)           (*sp = (ob), Dee_Incref(*sp), ++sp)
#define STACK_BEGIN           frame->cf_stack
#define STACK_END             (frame->cf_stack + code->co_framesize)
#define STACKUSED             (sp - frame->cf_stack)
#define STACKPREALLOC         ((uint16_t)((code->co_framesize / sizeof(DeeObject *)) - code->co_localc))
#ifdef USE_SWITCH
#define RAW_TARGET2(op, _op) case op&0xff: target##_op:
#else /* USE_SWITCH */
#define RAW_TARGET2(op, _op)               target##_op:
#endif /* !USE_SWITCH */
#define RAW_TARGET(op)       RAW_TARGET2(op, _##op)
#define EXCEPTION_CLEANUP    /* nothing */
#ifdef EXEC_SAFE
#define ASSERT_TUPLE(ob)     do{ if unlikely(!DeeTuple_CheckExact(ob)) { EXCEPTION_CLEANUP goto err_requires_tuple;} }__WHILE0
#define ASSERT_STRING(ob)    do{ if unlikely(!DeeString_CheckExact(ob)) { EXCEPTION_CLEANUP goto err_requires_string;} }__WHILE0
#define ASSERT_THISCALL()    do{ if unlikely(!(code->co_flags & CODE_FTHISCALL)) { EXCEPTION_CLEANUP goto err_requires_thiscall_code; } }__WHILE0
#define CONST_LOCKREAD()     STATIC_LOCKREAD()
#define CONST_LOCKENDREAD()  STATIC_LOCKENDREAD()
#define CONST_LOCKWRITE()    STATIC_LOCKWRITE()
#define CONST_LOCKENDWRITE() STATIC_LOCKENDWRITE()
#define ASSERT_ARGimm()      do{ if unlikely(imm_val >= code->co_argc_max) { EXCEPTION_CLEANUP goto err_invalid_argument_index; } }__WHILE0
#define ASSERT_REFimm()      do{ if unlikely(imm_val >= code->co_refc) { EXCEPTION_CLEANUP goto err_invalid_ref; } }__WHILE0
#define ASSERT_EXTERNimm()   do{ if unlikely(imm_val >= code->co_module->mo_importc || imm_val2 >= code->co_module->mo_importv[imm_val]->mo_globalc) { EXCEPTION_CLEANUP goto err_invalid_extern; } }__WHILE0
#define ASSERT_GLOBALimm()   do{ if unlikely(imm_val >= code->co_module->mo_globalc) { EXCEPTION_CLEANUP goto err_invalid_global; } }__WHILE0
#define ASSERT_LOCALimm()    do{ if unlikely(imm_val >= code->co_localc) { EXCEPTION_CLEANUP goto err_invalid_locale; } }__WHILE0
#define ASSERT_STATICimm()   do{ if unlikely(imm_val >= code->co_staticc) { EXCEPTION_CLEANUP goto err_invalid_static; } }__WHILE0
#define ASSERT_CONSTimm()    do{ if unlikely(imm_val >= code->co_staticc) { EXCEPTION_CLEANUP goto err_invalid_const; } }__WHILE0
#define ASSERT_CONSTimm2()   do{ if unlikely(imm_val2 >= code->co_staticc) { EXCEPTION_CLEANUP imm_val2 = imm_val; goto err_invalid_const; } }__WHILE0
#define ASSERT_YIELDING()    do{ if unlikely(!(code->co_flags & CODE_FYIELDING)) { EXCEPTION_CLEANUP goto err_requires_yield_code; } }__WHILE0
#define STACKFREE            ((frame->cf_stack+(frame->cf_stacksz ? frame->cf_stacksz : STACKPREALLOC))-sp)
#define STACKSIZE            (frame->cf_stacksz ? frame->cf_stacksz : STACKPREALLOC)
#define ASSERT_USAGE(sp_sub, sp_add)                                                 \
	if unlikely((sp_sub) != 0 && ((-(sp_sub)) > STACKUSED)) {                        \
		EXCEPTION_CLEANUP                                                            \
		goto err_invalid_stack_affect;                                               \
	}                                                                                \
	if unlikely(((sp_sub) + (sp_add)) != 0 && (((sp_sub) + (sp_add)) > STACKFREE)) { \
		EXCEPTION_CLEANUP                                                            \
		goto increase_stacksize;                                                     \
	}
#else /* EXEC_SAFE */
#define ASSERT_TUPLE(ob)     ASSERT(DeeTuple_CheckExact(ob))
#define ASSERT_STRING(ob)    ASSERT(DeeString_CheckExact(ob))
#define ASSERT_THISCALL()    ASSERT(code->co_flags & CODE_FTHISCALL)
#define CONST_LOCKREAD()     (void)0
#define CONST_LOCKENDREAD()  (void)0
#define CONST_LOCKWRITE()    (void)0
#define CONST_LOCKENDWRITE() (void)0
#define ASSERT_ARGimm()      ASSERT(imm_val < code->co_argc_max)
#define ASSERT_REFimm()      ASSERT(imm_val < code->co_refc)
#define ASSERT_EXTERNimm()   ASSERT(imm_val < code->co_module->mo_importc && imm_val2 < code->co_module->mo_importv[imm_val]->mo_globalc)
#define ASSERT_GLOBALimm()   ASSERT(imm_val < code->co_module->mo_globalc)
#define ASSERT_LOCALimm()    ASSERT(imm_val < code->co_localc)
#define ASSERT_STATICimm()   ASSERT(imm_val < code->co_staticc)
#define ASSERT_CONSTimm()    ASSERT(imm_val < code->co_staticc)
#define ASSERT_CONSTimm2()   ASSERT(imm_val2 < code->co_staticc)
#define ASSERT_YIELDING()    ASSERT(code->co_flags & CODE_FYIELDING)
#define STACKFREE            ((frame->cf_stack+STACKPREALLOC)-sp)
#define STACKSIZE            (STACKPREALLOC)
#define ASSERT_USAGE(sp_sub, sp_add)               \
	ASSERT(!(sp_sub) || (-(sp_sub)) <= STACKUSED); \
	ASSERT(!((sp_sub) + (sp_add)) || ((sp_sub) + (sp_add)) <= STACKFREE);
#endif /* !EXEC_SAFE */
#define TARGET(op, sp_sub, sp_add) RAW_TARGET2(op, _##op) ASSERT_USAGE(sp_sub, sp_add) __IF0; else
#define TARGETSimm16(op, sp_sub, sp_add)                                        \
	        RAW_TARGET(op##16)     imm_val = (uint16_t)READ_Simm16();           \
	__IF0 { RAW_TARGET2(op, _##op) imm_val = (uint16_t)(int16_t)READ_Simm8(); } \
	        ASSERT_USAGE(sp_sub, sp_add)                                        \
	__IF0; else

#define REPEAT_INSTRUCTION() (ip.ptr = frame->cf_ip)
#define HANDLE_EXCEPT()      goto handle_except
#define DISPATCH()           goto next_instr
#define YIELD_RESULT()       do{ ASSERT(code->co_flags & CODE_FYIELDING); goto end_without_finally; }__WHILE0
#define RETURN_RESULT()      do{ ASSERT(!(code->co_flags & CODE_FYIELDING)); goto end_return; }__WHILE0
#define YIELD(val)           do{ frame->cf_result = (val); YIELD_RESULT(); }__WHILE0
#define RETURN(val)          do{ frame->cf_result = (val); RETURN_RESULT(); }__WHILE0
#ifndef __OPTIMIZE_SIZE__
#define PREDICT(opcode)      do{ if (*ip.ptr == (opcode)) { ++ip.ptr; goto target_##opcode; } }__WHILE0
#else /* !__OPTIMIZE_SIZE__ */
#define PREDICT(opcode)      do{}__WHILE0
#endif /* __OPTIMIZE_SIZE__ */


next_instr:
#if 0
	Dee_CHECKMEMORY();
#endif
#if 0
	if (_Dee_dprint_enabled) {
		struct ddi_state state;
		code_addr_t ip_addr = ip.ptr - code->co_code;
		if (!DeeCode_FindDDI((DeeObject *)code, &state, NULL, ip_addr, DDI_STATE_FNOTHROW | DDI_STATE_FNONAMES)) {
			Dee_DPRINTF("%s+%.4I32X [trace]\n", DeeCode_NAME(code), ip_addr);
		} else {
			struct ddi_xregs *iter;
			char const *path, *file, *name;
			char const *base_name = DeeCode_NAME(code);
			DDI_STATE_DO(iter, &state) {
				file = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_file);
				name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_name);
				if (!state.rs_regs.dr_path--) {
					path = NULL;
				} else {
					path = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_path);
				}
				Dee_DPRINTF("%s%s%s(%d,%d) : %s+%.4I32X",
				            path ? path : "",
				            path ? "/" : "",
				            file ? file : "",
				            iter->dx_base.dr_lno + 1,
				            iter->dx_base.dr_col + 1,
				            name ? name
				                 : (code->co_flags & CODE_FCONSTRUCTOR
				                    ? "<anonymous_ctor>"
				                    : "<anonymous>"),
				            ip_addr);
				if (name != base_name && *base_name) {
					/* Also print the name of the base-function */
					Dee_DPRINTF(" (%s)", base_name);
				}
				Dee_DPRINTF(" [sp=%" PRFu16 "]\n", (uint16_t)STACKUSED);
			}
			DDI_STATE_WHILE(iter, &state);
			Dee_ddi_state_fini(&state);
		}
	}
#endif
	frame->cf_ip = ip.ptr;
#ifdef USE_SWITCH
	switch (*ip.ptr++)
#else /* USE_SWITCH */
	goto *basic_targets[*ip.ptr++];
#endif /* !USE_SWITCH */
	{

		TARGET(ASM_RET_NONE, -0, +0) {
			if (ITER_ISOK(frame->cf_result))
				Dee_Decref(frame->cf_result);
			if (code->co_flags & CODE_FYIELDING) {
				/* Rewind the instruction pointer to potentially re-execute
				 * `ASM_RET_NONE' and return `ITER_DONE' once again, should
				 * the caller attempt to invoke us again. */
				REPEAT_INSTRUCTION();
				frame->cf_result = ITER_DONE;
			} else {
				/* Non-yielding `ASM_RET_NONE': Simply return `none' to the caller. */
				frame->cf_result = Dee_None;
				Dee_Incref(Dee_None);
			}
			goto end_return;
		}

		TARGET(ASM_RET, -1, +0) {
			/* Check if we're overwriting a previous return value
			 * (which can happen when `return' appears in a finally-block) */
			if (ITER_ISOK(frame->cf_result))
				Dee_Decref(frame->cf_result);
			frame->cf_result = POP();
			if (code->co_flags & CODE_FYIELDING)
				goto end_without_finally;
			goto end_return;
		}

		TARGET(ASM_SETRET, -1, +0) {
			if (ITER_ISOK(frame->cf_result))
				Dee_Decref(frame->cf_result);
			frame->cf_result = POP();
			DISPATCH();
		}

		TARGET(ASM_YIELDALL, -1, +0) {
			ASSERT_YIELDING();
			if (ITER_ISOK(frame->cf_result))
				Dee_Decref(frame->cf_result);
			frame->cf_result = DeeObject_IterNext(TOP);
			if unlikely(!frame->cf_result)
				HANDLE_EXCEPT();
			if (frame->cf_result != ITER_DONE) {
				/* Repeat this instruction and forward the value we've just read. */
				REPEAT_INSTRUCTION();
				YIELD_RESULT();
			}
			/* Pop the iterator that was enumerated. */
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_THROW, -1, +0) {
			DeeError_Throw(TOP);
			POPREF();
			HANDLE_EXCEPT();
		}

		TARGET(ASM_RETHROW, -0, +0) {
			if (except_recursion < this_thread->t_exceptsz) {
				/* We've already thrown at least one exception, meaning
				 * we can simply go back to handling it again! */
			} else if (this_thread->t_except) {
				/* Rethrow an exception of the caller. */
				DeeError_Throw(this_thread->t_except->ef_error);
			} else {
				/* Throw an exception because none had been thrown, yet. */
except_no_active_exception:
				err_no_active_exception();
			}
			HANDLE_EXCEPT();
		}

		TARGET(ASM_ENDCATCH, -0, +0) {
			ASSERT(except_recursion <= this_thread->t_exceptsz);
			/* Handle errors if we've caused any.
			 * NOTE: We do allow the handling of interrupt signal here,
			 *       so-as to comply with the intended usage-case within
			 *       interrupt exception handlers. */
			if (except_recursion != this_thread->t_exceptsz)
				DeeError_Handled(ERROR_HANDLED_INTERRUPT);
			DISPATCH();
		}

		TARGET(ASM_ENDFINALLY, -0, +0) {
			/* If a return value has been assigned, stop execution. */
			if (frame->cf_result != NULL)
				goto end_return;
			/* Check for errors. */
			if (except_recursion != this_thread->t_exceptsz)
				HANDLE_EXCEPT();
			DISPATCH();
		}

		TARGET(ASM_PUSH_BND_ARG, -0, +1) {
			imm_val = READ_imm8();
do_push_bnd_arg:
			ASSERT_ARGimm();
			PUSHREF(DeeBool_For(imm_val < frame->cf_argc ||
			                    (frame->cf_kw &&
			                     frame->cf_kw->fk_kargv[imm_val - frame->cf_argc])));
			DISPATCH();
		}

		TARGET(ASM_PUSH_BND_EXTERN, -0, +1) {
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_push_bnd_extern:
			ASSERT_EXTERNimm();
			/*EXTERN_LOCKREAD();*/
			PUSHREF(DeeBool_For(EXTERNimm != NULL));
			/*EXTERN_LOCKENDREAD();*/
			DISPATCH();
		}

		TARGET(ASM_PUSH_BND_GLOBAL, -0, +1) {
			imm_val = READ_imm8();
do_push_bnd_global:
			ASSERT_GLOBALimm();
			/*GLOBAL_LOCKREAD();*/
			PUSHREF(DeeBool_For(GLOBALimm != NULL));
			/*GLOBAL_LOCKENDREAD();*/
			DISPATCH();
		}

		TARGET(ASM_PUSH_BND_LOCAL, -0, +1) {
			imm_val = READ_imm8();
do_push_bnd_local:
			ASSERT_LOCALimm();
			PUSHREF(DeeBool_For(LOCALimm != NULL));
			DISPATCH();
		}

		TARGETSimm16(ASM_JF, -1, +0) {
			/* Conditionally jump if true. */
			int temp = DeeObject_Bool(TOP);
			if unlikely(temp < 0)
				HANDLE_EXCEPT();
			POPREF();
			if (!temp) {
jump_16:
				if ((int16_t)imm_val < 0) {
					if (DeeThread_CheckInterruptSelf(this_thread))
						HANDLE_EXCEPT();
				}
				ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
				goto assert_ip_bounds;
#else /* EXEC_SAFE */
				ASSERT(ip.ptr >= code->co_code);
				ASSERT(ip.ptr < code->co_code + code->co_codebytes);
#endif /* !EXEC_SAFE */
			}
			DISPATCH();
		}

		TARGETSimm16(ASM_JT, -1, +0) {
			/* Conditionally jump if false. */
			int temp = DeeObject_Bool(TOP);
			if unlikely(temp < 0)
				HANDLE_EXCEPT();
			POPREF();
			if (temp)
				goto jump_16;
			DISPATCH();
		}

		TARGETSimm16(ASM_JMP, -0, +0) {
			if ((int16_t)imm_val < 0) {
				if (DeeThread_CheckInterruptSelf(this_thread))
					HANDLE_EXCEPT();
			}

			/* Adjust the instruction pointer accordingly. */
			ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
assert_ip_bounds:
			/* Raise an error if the new PC has been displaced out-of-bounds. */
			if unlikely(ip.ptr < code->co_code ||
			            ip.ptr >= code->co_code + code->co_codebytes)
				goto err_invalid_ip;
#else /* EXEC_SAFE */
			ASSERT(ip.ptr >= code->co_code);
			ASSERT(ip.ptr < code->co_code + code->co_codebytes);
#endif /* !EXEC_SAFE */
			DISPATCH();
		}

		TARGETSimm16(ASM_FOREACH, -1, +2) {
			DREF DeeObject *elem;
			elem = DeeObject_IterNext(TOP);
			if unlikely(!elem)
				HANDLE_EXCEPT();
			if (elem == ITER_DONE) {
				/* Pop the iterator and Jump if it finished. */
				POPREF();
				goto jump_16;
			}
			/* Leave the iterator and push the element. */
			PUSH(elem);
			DISPATCH();
		}

		TARGET(ASM_JMP_POP, -1, +0) {
			code_addr_t absip;
			instruction_t *new_ip;
			if (DeeObject_AsUInt32(TOP, &absip))
				HANDLE_EXCEPT();
			POPREF();
#ifdef EXEC_SAFE
			if (absip >= code->co_codebytes) {
				DeeError_Throwf(&DeeError_SegFault,
				                "Invalid PC %.4I32X in absolute jmp",
				                absip);
				HANDLE_EXCEPT();
			}
#else /* EXEC_SAFE */
			ASSERTF(absip < code->co_codebytes, "Invalid PC: %X",
			        (unsigned int)absip);
#endif /* !EXEC_SAFE */
			new_ip = code->co_code + absip;
			if (new_ip < ip.ptr) {
				if (DeeThread_CheckInterruptSelf(this_thread))
					HANDLE_EXCEPT();
			}
			ip.ptr = new_ip;
			DISPATCH();
		}

		RAW_TARGET(ASM_CALL) {
			uint8_t n_args = READ_imm8();
			DREF DeeObject *call_result, **new_sp;
			ASSERT_USAGE(-1 - (int)n_args, +1);
			/* NOTE: Inherit references. */
			new_sp      = sp - n_args;
			call_result = DeeObject_Call(new_sp[-1], n_args, new_sp);
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			while (n_args--)
				POPREF();
			Dee_Decref(TOP);   /* Drop a reference from the called function. */
			TOP = call_result; /* Save the frame->cf_result of the call back on the stack. */
			DISPATCH();
		}

		RAW_TARGET(ASM_CALL_KW) {
			DREF DeeObject *call_result, **new_sp;
			imm_val2 = READ_imm8();
			imm_val  = READ_imm8();
do_call_kw:
			ASSERT_USAGE(-1 - (int)imm_val2, +1);
			/* NOTE: Inherit references. */
			new_sp = sp - imm_val2;
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *kwds;
				CONST_LOCKREAD();
				kwds = CONSTimm;
				Dee_Incref(kwds);
				CONST_LOCKENDREAD();
				call_result = DeeObject_CallKw(new_sp[-1], imm_val2, new_sp, kwds);
				Dee_Decref(kwds);
			}
#else /* EXEC_SAFE */
			call_result = DeeObject_CallKw(new_sp[-1], imm_val2, new_sp, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			while (imm_val2--)
				POPREF();
			Dee_Decref(TOP);   /* Drop a reference from the called function. */
			TOP = call_result; /* Save the frame->cf_result of the call back on the stack. */
			DISPATCH();
		}

		RAW_TARGET(ASM_CALL_TUPLE_KW) {
			DREF DeeObject *call_result;
			imm_val = READ_imm8();
do_call_tuple_kw:
			ASSERT_USAGE(-2, +1);
			/* NOTE: Inherit references. */
			ASSERT_CONSTimm();
			ASSERT_TUPLE(FIRST);
#ifdef EXEC_SAFE
			{
				DREF DeeObject *kwds;
				CONST_LOCKREAD();
				kwds = CONSTimm;
				Dee_Incref(kwds);
				CONST_LOCKENDREAD();
				call_result = DeeObject_CallTupleKw(SECOND, FIRST, kwds);
				Dee_Decref(kwds);
			}
#else /* EXEC_SAFE */
			call_result = DeeObject_CallTupleKw(SECOND, FIRST, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			POPREF();          /* Pop the argument tuple. */
			Dee_Decref(TOP);   /* Drop a reference from the called function. */
			TOP = call_result; /* Save the frame->cf_result of the call back on the stack. */
			DISPATCH();
		}

		TARGET(ASM_CALL_TUPLE, -2, +1) {
			DREF DeeObject *temp;
			ASSERT_TUPLE(FIRST);
			temp = DeeObject_CallTuple(SECOND, FIRST);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = temp; /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_OPERATOR) {
			DREF DeeObject *call_result;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_operator:
			ASSERT_USAGE(-1 - (int)imm_val2, +1);
			/* NOTE: Inherit references. */
			call_result = DeeObject_InvokeOperator((sp - imm_val2)[-1], imm_val,
			                                       (size_t)imm_val2,
			                                       sp - imm_val2);
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			while (imm_val2--)
				POPREF();
			Dee_Decref(TOP);   /* Drop a reference from the operator self-argument. */
			TOP = call_result; /* Save the result of the call back on the stack. */
			DISPATCH();
		}

		TARGET(ASM_OPERATOR_TUPLE, -2, +1) {
			DREF DeeObject *call_result;
			imm_val = READ_imm8();
do_operator_tuple:
			ASSERT_TUPLE(FIRST);
			call_result = DeeObject_InvokeOperator(SECOND, imm_val,
			                                       DeeTuple_SIZE(FIRST),
			                                       DeeTuple_ELEM(FIRST));
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			POPREF();          /* Pop the argument tuple. */
			Dee_Decref(TOP);   /* Drop a reference from operator self-argument. */
			TOP = call_result; /* Save the result of the call back on the stack. */
			DISPATCH();
		}


		TARGET(ASM_DEL_GLOBAL, -0, +0) {
			DeeObject **p_object, *del_object;
			imm_val = READ_imm8();
do_del_global:
			ASSERT_GLOBALimm();
			GLOBAL_LOCKWRITE();
			p_object   = &GLOBALimm;
			del_object = *p_object;
			*p_object  = NULL;
			GLOBAL_LOCKENDWRITE();
			if unlikely(!del_object)
				goto err_unbound_global;
			Dee_Decref(del_object);
			DISPATCH();
		}

		TARGET(ASM_DEL_LOCAL, -0, +0) {
			DeeObject **p_local;
			imm_val = READ_imm8();
do_del_local:
			ASSERT_LOCALimm();
			p_local = &LOCALimm;
			if unlikely(!*p_local)
				goto err_unbound_local;
			Dee_Clear(*p_local);
			DISPATCH();
		}

		TARGET(ASM_SWAP, -2, +2) {
			DREF DeeObject *temp;
			temp   = SECOND;
			SECOND = FIRST;
			FIRST  = temp;
			DISPATCH();
		}

		RAW_TARGET(ASM_LROT) {
			DREF DeeObject *temp;
			uint16_t shift = (uint16_t)(READ_imm8() + 3);
			ASSERT_USAGE(-(int)shift, +(int)shift);
			temp = *(sp - shift);
			memmovedownc(sp - shift,
			             sp - (shift - 1),
			             shift - 1,
			             sizeof(DREF DeeObject *));
			sp[-1] = temp;
			DISPATCH();
		}

		RAW_TARGET(ASM_RROT) {
			DREF DeeObject *temp;
			uint16_t shift = (uint16_t)(READ_imm8() + 3);
			ASSERT_USAGE(-(int)shift, +(int)shift);
			temp = sp[-1];
			memmoveupc(sp - (shift - 1),
			           sp - shift,
			           shift - 1,
			           sizeof(DREF DeeObject *));
			*(sp - shift) = temp;
			DISPATCH();
		}

		TARGET(ASM_DUP, -1, +2) {
			PUSHREF(TOP);
			DISPATCH();
		}

		RAW_TARGET(ASM_DUP_N) {
			uint8_t offset = READ_imm8();
			DREF DeeObject **p_slot;
			ASSERT_USAGE(-((int)offset + 2), +((int)offset + 3));
			p_slot = sp - (offset + 2);
			PUSHREF(*p_slot);
			DISPATCH();
		}

		TARGET(ASM_POP, -1, +0) {
			POPREF();
			DISPATCH();
		}

		RAW_TARGET(ASM_POP_N) {
			DREF DeeObject *old_object;
			DREF DeeObject **p_slot;
			uint8_t offset = READ_imm8();
			ASSERT_USAGE(-((int)offset + 2), +((int)offset + 1));
			p_slot     = sp - (offset + 2);
			old_object = *p_slot;
			*p_slot    = POP();
			Dee_Decref(old_object);
			DISPATCH();
		}

		{
			RAW_TARGET(ASM_ADJSTACK)
			imm_val = (uint16_t)(int16_t)READ_Simm8();
do_stack_adjust:
			if ((int16_t)imm_val < 0) {
#ifdef EXEC_SAFE
				if unlikely(-(int16_t)imm_val > STACKUSED)
					goto err_invalid_stack_affect;
#else /* EXEC_SAFE */
				ASSERT(-(int16_t)imm_val <= STACKUSED);
#endif /* !EXEC_SAFE */
				while (imm_val++)
					POPREF();
			} else {
#ifdef EXEC_SAFE
				if unlikely((int16_t)imm_val > STACKFREE)
					goto increase_stacksize;
#else /* EXEC_SAFE */
				ASSERT((int16_t)imm_val <= STACKFREE);
#endif /* !EXEC_SAFE */
				while (imm_val--)
					PUSHREF(Dee_None);
			}
			/* A stack adjustment is often followed by a
			 * decently-sized (though usually still 8-bit) jump.
			 * Therefor, we predict that a jump instruction follows. */
			PREDICT(ASM_JMP);
			DISPATCH();
		}

		TARGET(ASM_SUPER, -2, +1) {
			DREF DeeObject *super_wrapper;
			super_wrapper = DeeSuper_New((DeeTypeObject *)FIRST, SECOND);
			if unlikely(!super_wrapper)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = super_wrapper; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_SUPER_THIS_R, -0, +1) {
			DREF DeeObject *super_wrapper;
			imm_val = READ_imm8();
do_super_this_r:
			ASSERT_THISCALL();
			ASSERT_REFimm();
			super_wrapper = DeeSuper_New((DeeTypeObject *)REFimm, THIS);
			if unlikely(!super_wrapper)
				HANDLE_EXCEPT();
			PUSH(super_wrapper); /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_ISNONE, -1, +1) {
			Dee_Decref(TOP); /* Can already decref() because we only need to check the pointer. */
			TOP = DeeBool_For(TOP == Dee_None);
			Dee_Incref(TOP);
			DISPATCH();
		}

		TARGET(ASM_POP_STATIC, -1, +0) {
			DeeObject *old_value;
			imm_val = READ_imm8();
do_pop_static:
			ASSERT_STATICimm();
			STATIC_LOCKWRITE();
			old_value = STATICimm;
			STATICimm = POP();
			STATIC_LOCKENDWRITE();
			ASSERT_OBJECT(old_value);
			Dee_Decref(old_value);
			DISPATCH();
		}

		TARGET(ASM_POP_EXTERN, -1, +0) {
			DeeObject *old_value, **p_extern;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_pop_extern:
			ASSERT_EXTERNimm();
			EXTERN_LOCKWRITE();
			p_extern  = &EXTERNimm;
			old_value = *p_extern;
			*p_extern = POP();
			EXTERN_LOCKENDWRITE();
			Dee_XDecref(old_value);
			DISPATCH();
		}

		TARGET(ASM_POP_GLOBAL, -1, +0) {
			DeeObject *old_value, **p_global;
			imm_val = READ_imm8();
do_pop_global:
			ASSERT_GLOBALimm();
			GLOBAL_LOCKWRITE();
			p_global    = &GLOBALimm;
			old_value = *p_global;
			*p_global   = POP();
			GLOBAL_LOCKENDWRITE();
			Dee_XDecref(old_value);
			DISPATCH();
		}

		TARGET(ASM_POP_LOCAL, -1, +0) {
			DeeObject *old_value;
			imm_val = READ_imm8();
do_pop_local:
			ASSERT_LOCALimm();
			old_value = LOCALimm;
			LOCALimm  = POP();
			Dee_XDecref(old_value);
			DISPATCH();
		}

		TARGET(ASM_PUSH_REF, -0, +1) {
			imm_val = READ_imm8();
do_push_ref:
			ASSERT_REFimm();
			PUSHREF(REFimm);
			DISPATCH();
		}

		TARGET(ASM_PUSH_ARG, -0, +1) {
			DeeObject *value;
			imm_val = READ_imm8();
do_push_arg:
			ASSERT_ARGimm();
			/* Simple case: Direct argument/default reference. */
			if (imm_val < frame->cf_argc) {
				value = frame->cf_argv[imm_val];
			} else if (frame->cf_kw) {
				value = frame->cf_kw->fk_kargv[imm_val - frame->cf_argc];
				if (!value) {
					value = code->co_defaultv[imm_val - code->co_argc_min];
					if (!value)
						goto err_unbound_arg;
				}
			} else {
				value = code->co_defaultv[imm_val - code->co_argc_min];
				if (!value)
					goto err_unbound_arg;
			}
			PUSHREF(value);
			DISPATCH();
		}

		TARGET(ASM_PUSH_VARARGS, -0, +1) {
#ifdef EXEC_SAFE
			if (!(code->co_flags & CODE_FVARARGS))
				goto err_requires_varargs_code;
#else /* EXEC_SAFE */
			ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
			if (!frame->cf_vargs) {
				if (frame->cf_argc <= code->co_argc_max) {
					frame->cf_vargs = (DREF DeeTupleObject *)Dee_EmptyTuple;
					Dee_Incref(Dee_EmptyTuple);
				} else {
					frame->cf_vargs = (DREF DeeTupleObject *)DeeTuple_NewVector((size_t)(frame->cf_argc - code->co_argc_max),
					                                                            frame->cf_argv + code->co_argc_max);
					if unlikely(!frame->cf_vargs)
						HANDLE_EXCEPT();
				}
			}
			PUSHREF((DeeObject *)frame->cf_vargs);
			DISPATCH();
		}

		TARGET(ASM_PUSH_VARKWDS, -0, +1) {
			DeeObject *varkwds;
#ifdef EXEC_SAFE
			if (!(code->co_flags & CODE_FVARKWDS))
				goto err_requires_varkwds_code;
#else /* EXEC_SAFE */
			ASSERT(code->co_flags & CODE_FVARKWDS);
#endif /* !EXEC_SAFE */
			if (frame->cf_kw) {
				varkwds = frame->cf_kw->fk_varkwds;
				if (!varkwds) {
					DeeObject *oldval;
					varkwds = construct_varkwds_mapping();
					if unlikely(!varkwds)
						HANDLE_EXCEPT();
					oldval = atomic_cmpxch_val(&frame->cf_kw->fk_varkwds, NULL, varkwds);
					if unlikely(oldval) {
						VARKWDS_DECREF(varkwds);
						varkwds = oldval;
					}
				}
			} else {
				varkwds = Dee_EmptyMapping;
			}
			PUSHREF(varkwds);
			DISPATCH();
		}

		TARGET(ASM_PUSH_CONST, -0, +1) {
			imm_val = READ_imm8();
do_push_const:
			ASSERT_CONSTimm();
			CONST_LOCKREAD();
			PUSHREF(CONSTimm);
			CONST_LOCKENDREAD();
			DISPATCH();
		}

		TARGET(ASM_PUSH_STATIC, -0, +1) {
			imm_val = READ_imm8();
do_push_static:
			ASSERT_STATICimm();
			STATIC_LOCKREAD();
			PUSHREF(STATICimm);
			STATIC_LOCKENDREAD();
			DISPATCH();
		}

		TARGET(ASM_PUSH_EXTERN, -0, +1) {
			DeeObject *value;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_push_extern:
			ASSERT_EXTERNimm();
			EXTERN_LOCKREAD();
			value = EXTERNimm;
			if unlikely(!value) {
				EXTERN_LOCKENDREAD();
				goto err_unbound_extern;
			}
			PUSHREF(value);
			EXTERN_LOCKENDREAD();
			DISPATCH();
		}

		TARGET(ASM_PUSH_GLOBAL, -0, +1) {
			DeeObject *value;
			imm_val = READ_imm8();
do_push_global:
			ASSERT_GLOBALimm();
			GLOBAL_LOCKREAD();
			value = GLOBALimm;
			if unlikely(!value) {
				GLOBAL_LOCKENDREAD();
				goto err_unbound_global;
			}
			PUSHREF(value);
			GLOBAL_LOCKENDREAD();
			DISPATCH();
		}

		TARGET(ASM_PUSH_LOCAL, -0, +1) {
			DeeObject *value;
			imm_val = READ_imm8();
do_push_local:
			ASSERT_LOCALimm();
			value = LOCALimm;
			if unlikely(!value)
				goto err_unbound_local;
			PUSHREF(value);
			DISPATCH();
		}

		RAW_TARGET(ASM_PACK_TUPLE) {
			DREF DeeObject *temp;
			imm_val = READ_imm8();
do_pack_tuple:
			ASSERT_USAGE(-(int)imm_val, +1);
			temp = DeeTuple_NewVectorSymbolic(imm_val, sp - imm_val); /* Inherit references. */
			if unlikely(!temp)
				HANDLE_EXCEPT();
			sp -= imm_val;
			PUSH(temp);
			DISPATCH();
		}

		RAW_TARGET(ASM_PACK_LIST) {
			DREF DeeObject *temp;
			imm_val = READ_imm8();
do_pack_list:
			ASSERT_USAGE(-(int)imm_val, +1);
			temp = DeeList_NewVectorInherited(imm_val, sp - imm_val); /* Inherit references. */
			if unlikely(!temp)
				HANDLE_EXCEPT();
			sp -= imm_val;
			PUSH(temp);
			DISPATCH();
		}

		RAW_TARGET(ASM_UNPACK) {
			int error;
			DREF DeeObject *sequence;
			imm_val = READ_imm8();
do_unpack:
			ASSERT_USAGE(-1, +(int)imm_val);
			sequence = POP();
			error    = DeeObject_Unpack(sequence, imm_val, sp);
			Dee_Decref(sequence);
			if unlikely(error)
				HANDLE_EXCEPT();
			sp += imm_val;
			DISPATCH();
		}

		TARGET(ASM_CAST_TUPLE, -1, +1) {
			DREF DeeObject *temp;
			temp = DeeTuple_FromSequence(TOP);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = temp; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_CAST_LIST, -1, +1) {
			DREF DeeObject *temp;
			temp = DeeList_FromSequence(TOP);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = temp; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_PUSH_NONE, -0, +1) {
			Dee_Incref(Dee_None);
			PUSH(Dee_None);
			DISPATCH();
		}

		TARGET(ASM_PUSH_MODULE, -0, +1) {
			DeeModuleObject *mod;
			imm_val = READ_imm8();
do_push_module:
			mod = code->co_module;
			ASSERT_OBJECT(mod);
#ifdef EXEC_SAFE
			if (imm_val >= mod->mo_importc)
				goto err_invalid_module;
#else /* EXEC_SAFE */
			ASSERT(imm_val < mod->mo_importc);
#endif /* !EXEC_SAFE */
			mod = mod->mo_importv[imm_val];
			ASSERT_OBJECT(mod);
			PUSHREF((DeeObject *)mod);
			DISPATCH();
		}

		TARGET(ASM_CONCAT, -2, +1) {
			DREF DeeObject *temp;
			temp = DeeObject_ConcatInherited(SECOND, FIRST);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			SECOND = temp;
			POPREF();
			DISPATCH();
		}

		RAW_TARGET(ASM_EXTEND) {
			DREF DeeObject **new_sp;
			uint8_t n_args = READ_imm8();
			DREF DeeObject *temp;
			ASSERT_USAGE(-((int)n_args + 1), +1);
			new_sp = sp - n_args;
			temp   = DeeObject_ExtendInherited(new_sp[-1], n_args, new_sp);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			sp  = new_sp;
			TOP = temp;
			DISPATCH();
		}

		TARGET(ASM_TYPEOF, -1, +1) {
			DeeTypeObject *typ;
			typ = Dee_TYPE(TOP);
			Dee_Incref(typ);
			Dee_Decref(TOP);
			TOP = (DREF DeeObject *)typ; /* Inherit object. */
			DISPATCH();
		}

		TARGET(ASM_CLASSOF, -1, +1) {
			DeeTypeObject *typ;
			typ = DeeObject_Class(TOP);
			Dee_Incref(typ);
			Dee_Decref(TOP);
			TOP = (DREF DeeObject *)typ; /* Inherit object. */
			DISPATCH();
		}

		TARGET(ASM_SUPEROF, -1, +1) {
			DREF DeeObject *temp;
			temp = DeeSuper_Of(TOP);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = temp; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_INSTANCEOF, -2, +1) {
			bool is_instance;
			/* Special case: The deemon specs allow `none' to be
			 *               written as the second argument to `is' */
			if (DeeNone_Check(FIRST)) {
				is_instance = DeeNone_Check(SECOND);
			} else if (DeeSuper_Check(SECOND)) {
				is_instance = DeeType_IsInherited(DeeSuper_TYPE(SECOND), (DeeTypeObject *)FIRST);
			} else {
				is_instance = DeeObject_InstanceOf(SECOND, (DeeTypeObject *)FIRST);
			}
			POPREF();
			Dee_Decref(TOP);
			TOP = DeeBool_For(is_instance);
			Dee_Incref(TOP);
			DISPATCH();
		}

		TARGET(ASM_STR, -1, +1) {
			DREF DeeObject *temp;
			temp = DeeObject_Str(TOP);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = temp; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_REPR, -1, +1) {
			DREF DeeObject *temp;
			switch (*ip.u8) {

			/* Required handling for object repr streaming into a file via `<<' */
			case ASM_SHL: {
				DeeTypeObject *tp_temp = Dee_TYPE(SECOND);
				for (;;) {
					DREF DeeObject *other;
					DREF DeeObject *(DCALL *tp_shl)(DeeObject *, DeeObject *);
					if (!tp_temp->tp_math ||
					    (tp_shl = tp_temp->tp_math->tp_shl) == NULL) {
						tp_temp = tp_temp->tp_base;
						if (!tp_temp)
							break;
						continue;
					}
					++ip.u8;
					if (tp_shl == &file_shl) {
						/* Special case: `fp << repr foo'
						 * In this case, we can do a special optimization
						 * to directly print the repr to the file. */
						if (DeeObject_PrintRepr(TOP, (dformatprinter)&DeeFile_WriteAll, SECOND) < 0)
							HANDLE_EXCEPT();
						POPREF();
						DISPATCH();
					}
					temp = DeeObject_Repr(TOP);
					if unlikely(!temp)
						HANDLE_EXCEPT();
					POPREF();
					other = (*tp_shl)(TOP, temp);
					Dee_Decref(temp);
					if unlikely(!other)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = other;
					DISPATCH();
				}
			}	break;

			/* Required handling for object repr streaming to stdout */
			case ASM_PRINT:
			case ASM_PRINT_SP:
			case ASM_PRINT_NL: {
				DREF DeeObject *stream;
				int error;
				stream = DeeFile_GetStd(DEE_STDOUT);
				if unlikely(!stream)
					HANDLE_EXCEPT();
				switch (*ip.u8++) {
				case ASM_PRINT:
					error = DeeFile_PrintObjectRepr(stream, TOP);
					break;
				case ASM_PRINT_SP:
					error = DeeFile_PrintObjectReprSp(stream, TOP);
					break;
				case ASM_PRINT_NL:
					error = DeeFile_PrintObjectReprNl(stream, TOP);
					break;
				default: __builtin_unreachable();
				}
				Dee_Decref(stream);
				if unlikely(error)
					HANDLE_EXCEPT();
				POPREF();
				DISPATCH();
			}

			/* Required handling for object repr streaming to a custom file */
			case ASM_FPRINT:
			case ASM_FPRINT_SP:
			case ASM_FPRINT_NL: {
				int error;
				switch (*ip.u8++) {
				case ASM_PRINT:
					error = DeeFile_PrintObjectRepr(SECOND, TOP);
					break;
				case ASM_PRINT_SP:
					error = DeeFile_PrintObjectReprSp(SECOND, TOP);
					break;
				case ASM_PRINT_NL:
					error = DeeFile_PrintObjectReprNl(SECOND, TOP);
					break;
				default: __builtin_unreachable();
				}
				if unlikely(error)
					HANDLE_EXCEPT();
				POPREF();
				DISPATCH();
			}

			default: break;
			}
			temp = DeeObject_Repr(TOP);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = temp; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_BOOL, -1, +1) {
			int boolval = DeeObject_Bool(TOP);
			if unlikely(boolval < 0)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = DeeBool_For(boolval);
			Dee_Incref(TOP);
			DISPATCH();
		}

		TARGET(ASM_NOT, -1, +1) {
			int boolval = DeeObject_Bool(TOP);
			if unlikely(boolval < 0)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = DeeBool_For(!boolval);
			Dee_Incref(TOP);
			DISPATCH();
		}

		TARGET(ASM_ASSIGN, -2, +1) {
			if unlikely(DeeObject_Assign(SECOND, TOP))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_MOVE_ASSIGN, -2, +1) {
			if unlikely(DeeObject_MoveAssign(SECOND, TOP))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_COPY, -1, +1) {
			DREF DeeObject *temp;
			temp = DeeObject_Copy(TOP);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = temp;
			DISPATCH();
		}

		TARGET(ASM_DEEPCOPY, -1, +1) {
			DREF DeeObject *temp;
			temp = DeeObject_DeepCopy(TOP);
			if unlikely(!temp)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = temp;
			DISPATCH();
		}

#define DEFINE_COMPARE_INSTR(EQ, Eq)                             \
		TARGET(ASM_CMP_##EQ, -2, +1) {                           \
			DREF DeeObject *temp;                                \
			temp = DeeObject_Compare##Eq##Object(SECOND, FIRST); \
			if unlikely(!temp)                                   \
				HANDLE_EXCEPT();                                 \
			POPREF();                                            \
			Dee_Decref(TOP);                                     \
			TOP = temp; /* Inherit reference. */                 \
			DISPATCH();                                          \
		}
		DEFINE_COMPARE_INSTR(EQ, Eq)
		DEFINE_COMPARE_INSTR(NE, Ne)
		DEFINE_COMPARE_INSTR(LO, Lo)
		DEFINE_COMPARE_INSTR(LE, Le)
		DEFINE_COMPARE_INSTR(GR, Gr)
		DEFINE_COMPARE_INSTR(GE, Ge)
#undef DEFINE_COMPARE_INSTR

		TARGET(ASM_CLASS_C, -1, +1) {
			DREF DeeTypeObject *new_class;
			imm_val = READ_imm8();
do_class_c:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DeeObject *descriptor;
				CONST_LOCKREAD();
				descriptor = CONSTimm;
				Dee_Incref(descriptor);
				CONST_LOCKENDREAD();
				if unlikely(DeeObject_AssertTypeExact(descriptor, &DeeClassDescriptor_Type)) {
					new_class = NULL;
				} else {
					new_class = DeeClass_New((DeeTypeObject *)TOP, descriptor);
				}
				Dee_Decref(descriptor);
			}
#else /* EXEC_SAFE */
			new_class = DeeClass_New((DeeTypeObject *)TOP, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(!new_class)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = (DREF DeeObject *)new_class; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_CLASS_GC, -0, +1) {
			DREF DeeTypeObject *new_class;
			DREF DeeObject *base;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_class_gc:
			ASSERT_GLOBALimm();
			ASSERT_CONSTimm2();
			GLOBAL_LOCKREAD();
			base = GLOBALimm;
			Dee_XIncref(base);
			GLOBAL_LOCKENDREAD();
			if unlikely(!base)
				goto err_unbound_global;
#ifdef EXEC_SAFE
			{
				DeeObject *descriptor;
				CONST_LOCKREAD();
				descriptor = CONSTimm2;
				Dee_Incref(descriptor);
				CONST_LOCKENDREAD();
				if unlikely(DeeObject_AssertTypeExact(descriptor, &DeeClassDescriptor_Type)) {
					new_class = NULL;
				} else {
					new_class = DeeClass_New((DeeTypeObject *)base, descriptor);
				}
				Dee_Decref(descriptor);
			}
#else /* EXEC_SAFE */
			new_class = DeeClass_New((DeeTypeObject *)base, CONSTimm);
#endif /* !EXEC_SAFE */
			Dee_Decref(base);
			if unlikely(!new_class)
				HANDLE_EXCEPT();
			PUSH((DREF DeeObject *)new_class); /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_CLASS_EC, -0, +1) {
			DREF DeeTypeObject *new_class;
			DREF DeeObject *base;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
			ASSERT_EXTERNimm();
			EXTERN_LOCKREAD();
			base = EXTERNimm;
			Dee_XIncref(base);
			EXTERN_LOCKENDREAD();
			if unlikely(!base)
				goto err_unbound_extern;
#undef EXCEPTION_CLEANUP
#define EXCEPTION_CLEANUP Dee_Decref(base);
			imm_val = READ_imm8();
			ASSERT_CONSTimm();
#undef EXCEPTION_CLEANUP
#define EXCEPTION_CLEANUP /* nothing */
#ifdef EXEC_SAFE
			{
				DeeObject *descriptor;
				CONST_LOCKREAD();
				descriptor = CONSTimm;
				Dee_Incref(descriptor);
				CONST_LOCKENDREAD();
				if unlikely(DeeObject_AssertTypeExact(descriptor, &DeeClassDescriptor_Type)) {
					new_class = NULL;
				} else {
					new_class = DeeClass_New((DeeTypeObject *)base, descriptor);
				}
				Dee_Decref(descriptor);
			}
#else /* EXEC_SAFE */
			new_class = DeeClass_New((DeeTypeObject *)base, CONSTimm);
#endif /* !EXEC_SAFE */
			Dee_Decref(base);
			if unlikely(!new_class)
				HANDLE_EXCEPT();
			PUSH((DREF DeeObject *)new_class); /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_DEFCMEMBER, -2, +1) {
			imm_val = READ_imm8();
do_defcmember:
#ifdef EXEC_SAFE
			if (DeeClass_SetMemberSafe((DeeTypeObject *)SECOND, imm_val, FIRST))
				HANDLE_EXCEPT();
#else /* EXEC_SAFE */
			DeeClass_SetMember((DeeTypeObject *)SECOND, imm_val, FIRST);
#endif /* !EXEC_SAFE */
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_GETCMEMBER_R, -0, +1) {
			DREF DeeObject *member_value;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_getcmember_r:
			ASSERT_REFimm();
#ifdef EXEC_SAFE
			member_value = DeeClass_GetMemberSafe((DeeTypeObject *)REFimm, imm_val2);
#else /* EXEC_SAFE */
			member_value = DeeClass_GetMember((DeeTypeObject *)REFimm, imm_val2);
#endif /* !EXEC_SAFE */
			if unlikely(!member_value)
				HANDLE_EXCEPT();
			PUSH(member_value); /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_CALLCMEMBER_THIS_R) {
			DREF DeeObject *callback, *result;
			uint8_t argc;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_callcmember_this_r:
			argc = READ_imm8();
			ASSERT_REFimm();
			ASSERT_THISCALL();
			ASSERT_USAGE(-(int)argc, +1);
#ifdef EXEC_SAFE
			callback = DeeClass_GetMemberSafe((DeeTypeObject *)REFimm, imm_val2);
#else /* EXEC_SAFE */
			callback = DeeClass_GetMember((DeeTypeObject *)REFimm, imm_val2);
#endif /* !EXEC_SAFE */
			if unlikely(!callback)
				HANDLE_EXCEPT();
			result = DeeObject_ThisCall(callback, THIS, argc, sp - argc);
			Dee_Decref(callback);
			if unlikely(!result)
				HANDLE_EXCEPT();
			while (argc--)
				POPREF();
			PUSH(result); /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_FUNCTION_C_16)
		imm_val  = READ_imm8();
		imm_val2 = READ_imm16();
		goto do_function_c;
		RAW_TARGET(ASM_FUNCTION_C) {
			DREF DeeObject *function;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_function_c:
			ASSERT_USAGE(-(int)(imm_val2 + 1), +1);
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *code_object;
				CONST_LOCKREAD();
				code_object = CONSTimm;
				Dee_Incref(code_object);
				CONST_LOCKENDREAD();
				if (DeeObject_AssertTypeExact(code_object, &DeeCode_Type)) {
					Dee_Decref(code_object);
					HANDLE_EXCEPT();
				}
				if (((DeeCodeObject *)code_object)->co_refc != imm_val2 + 1) {
					err_invalid_refs_size(code_object, imm_val2 + 1);
					Dee_Decref(code_object);
					HANDLE_EXCEPT();
				}
				function = DeeFunction_NewInherited(code_object,
				                                    imm_val2 + 1,
				                                    sp - (imm_val2 + 1));
				Dee_Decref(code_object);
			}
#else /* EXEC_SAFE */
			function = DeeFunction_NewInherited(CONSTimm,
			                                    imm_val2 + 1,
			                                    sp - (imm_val2 + 1));
#endif /* !EXEC_SAFE */
			if unlikely(!function)
				HANDLE_EXCEPT();
			sp -= imm_val2 + 1;
			PUSH(function);
			DISPATCH();
		}

		TARGET(ASM_CAST_INT, -1, +1) {
			DeeObject *cast_result;
			cast_result = DeeObject_Int(TOP);
			if unlikely(!cast_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = cast_result; /* Inherit reference. */
			DISPATCH();
		}

#define DEFINE_UNARY_MATH_OPERATOR(INV, Inv)            \
		TARGET(ASM_##INV, -1, +1) {                     \
			DREF DeeObject *math_result;                \
			math_result = DeeObject_##Inv(TOP);         \
			if unlikely(!math_result)                   \
				HANDLE_EXCEPT();                        \
			Dee_Decref(TOP);                            \
			TOP = math_result; /* Inherit reference. */ \
			DISPATCH();                                 \
		}
		DEFINE_UNARY_MATH_OPERATOR(INV, Inv)
		DEFINE_UNARY_MATH_OPERATOR(POS, Pos)
		DEFINE_UNARY_MATH_OPERATOR(NEG, Neg)
#undef DEFINE_UNARY_MATH_OPERATOR

#define DEFINE_BINARY_MATH_OPERATOR(ADD, Add)             \
		TARGET(ASM_##ADD, -2, +1) {                       \
			DREF DeeObject *math_result;                  \
			math_result = DeeObject_##Add(SECOND, FIRST); \
			if unlikely(!math_result)                     \
				HANDLE_EXCEPT();                          \
			POPREF();                                     \
			Dee_Decref(TOP);                              \
			TOP = math_result; /* Inherit reference. */   \
			DISPATCH();                                   \
		}
		DEFINE_BINARY_MATH_OPERATOR(ADD, Add)
		DEFINE_BINARY_MATH_OPERATOR(SUB, Sub)
		DEFINE_BINARY_MATH_OPERATOR(MUL, Mul)
		DEFINE_BINARY_MATH_OPERATOR(DIV, Div)
		DEFINE_BINARY_MATH_OPERATOR(MOD, Mod)
		DEFINE_BINARY_MATH_OPERATOR(SHL, Shl)
		DEFINE_BINARY_MATH_OPERATOR(SHR, Shr)
		DEFINE_BINARY_MATH_OPERATOR(AND, And)
		DEFINE_BINARY_MATH_OPERATOR(OR, Or)
		DEFINE_BINARY_MATH_OPERATOR(XOR, Xor)
		DEFINE_BINARY_MATH_OPERATOR(POW, Pow)
#undef DEFINE_BINARY_MATH_OPERATOR

		TARGET(ASM_ADD_SIMM8, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_AddS8(TOP, READ_Simm8());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}
		TARGET(ASM_ADD_IMM32, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_AddInt(TOP, READ_imm32());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_SUB_SIMM8, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_SubS8(TOP, READ_Simm8());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}
		TARGET(ASM_SUB_IMM32, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_SubInt(TOP, READ_imm32());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_MUL_SIMM8, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_MulInt(TOP, READ_Simm8());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_DIV_SIMM8, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_DivInt(TOP, READ_Simm8());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_MOD_SIMM8, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_ModInt(TOP, READ_Simm8());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_AND_IMM32, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_AndInt(TOP, READ_imm32());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_OR_IMM32, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_OrInt(TOP, READ_imm32());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_XOR_IMM32, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_XorInt(TOP, READ_imm32());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_SHL_IMM8, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_ShlInt(TOP, READ_imm8());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}

		TARGET(ASM_SHR_IMM8, -1, +1) {
			DREF DeeObject *math_result;
			math_result = DeeObject_ShrInt(TOP, READ_imm8());
			if unlikely(!math_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = math_result;
			DISPATCH();
		}


		RAW_TARGET(ASM_DELOP)
		RAW_TARGET(ASM_NOP) {
			/* Literally do nothing. */
			DISPATCH();
		}

		/* Print instructions. */
		TARGET(ASM_PRINT, -1, +0) {
			DREF DeeObject *stream;
			int error;
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
			error = DeeFile_PrintObject(stream, TOP);
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_PRINT_SP, -1, +0) {
			DREF DeeObject *stream;
			int error;
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
			error = DeeFile_PrintObjectSp(stream, TOP);
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_PRINT_NL, -1, +0) {
			DREF DeeObject *stream;
			int error;
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
			error = DeeFile_PrintObjectNl(stream, TOP);
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_PRINTALL, -1, +0) {
			DREF DeeObject *stream;
			int error;
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
			error = DeeFile_PrintAll(stream, TOP);
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_PRINTALL_SP, -1, +0) {
			DREF DeeObject *stream;
			int error;
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
			error = DeeFile_PrintAllSp(stream, TOP);
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_PRINTALL_NL, -1, +0) {
			DREF DeeObject *stream;
			int error;
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
			error = DeeFile_PrintAllNl(stream, TOP);
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_PRINTNL, -0, +0) {
			DREF DeeObject *stream;
			int error;
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
			error = DeeFile_PrintNl(stream);
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			DISPATCH();
		}

		TARGET(ASM_FPRINT, -2, +1) {
			if unlikely(DeeFile_PrintObject(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_FPRINT_SP, -2, +1) {
			if unlikely(DeeFile_PrintObjectSp(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_FPRINT_NL, -2, +1) {
			if unlikely(DeeFile_PrintObjectNl(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_FPRINTALL, -2, +1) {
			if unlikely(DeeFile_PrintAll(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_FPRINTALL_SP, -2, +1) {
			if unlikely(DeeFile_PrintAllSp(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_FPRINTALL_NL, -2, +1) {
			if unlikely(DeeFile_PrintAllNl(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_FPRINTNL, -1, +1) {
			if unlikely(DeeFile_PrintNl(FIRST))
				HANDLE_EXCEPT();
			DISPATCH();
		}

		TARGET(ASM_PRINT_C, -0, +0) {
			DREF DeeObject *stream;
			int error;
			imm_val = READ_imm8();
do_print_c:
			ASSERT_CONSTimm();
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *print_object;
				CONST_LOCKREAD();
				print_object = CONSTimm;
				Dee_Incref(print_object);
				CONST_LOCKENDREAD();
				error = DeeFile_PrintObject(stream, print_object);
				Dee_Decref(print_object);
			}
#else /* EXEC_SAFE */
			error = DeeFile_PrintObject(stream, CONSTimm);
#endif /* !EXEC_SAFE */
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			DISPATCH();
		}

		TARGET(ASM_PRINT_C_SP, -0, +0) {
			DREF DeeObject *stream;
			int error;
			imm_val = READ_imm8();
do_print_c_sp:
			ASSERT_CONSTimm();
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *print_object;
				CONST_LOCKREAD();
				print_object = CONSTimm;
				Dee_Incref(print_object);
				CONST_LOCKENDREAD();
				error = DeeFile_PrintObjectSp(stream, print_object);
				Dee_Decref(print_object);
			}
#else /* EXEC_SAFE */
			error = DeeFile_PrintObjectSp(stream, CONSTimm);
#endif /* !EXEC_SAFE */
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			DISPATCH();
		}

		TARGET(ASM_PRINT_C_NL, -0, +0) {
			DREF DeeObject *stream;
			int error;
			imm_val = READ_imm8();
do_print_c_nl:
			ASSERT_CONSTimm();
			stream = DeeFile_GetStd(DEE_STDOUT);
			if unlikely(!stream)
				HANDLE_EXCEPT();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *print_object;
				CONST_LOCKREAD();
				print_object = CONSTimm;
				Dee_Incref(print_object);
				CONST_LOCKENDREAD();
				error = DeeFile_PrintObjectNl(stream, print_object);
				Dee_Decref(print_object);
			}
#else /* EXEC_SAFE */
			error = DeeFile_PrintObjectNl(stream, CONSTimm);
#endif /* !EXEC_SAFE */
			Dee_Decref(stream);
			if unlikely(error)
				HANDLE_EXCEPT();
			DISPATCH();
		}

		TARGET(ASM_FPRINT_C, -1, +1) {
			imm_val = READ_imm8();
do_fprint_c:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *print_object;
				int error;
				CONST_LOCKREAD();
				print_object = CONSTimm;
				Dee_Incref(print_object);
				CONST_LOCKENDREAD();
				error = DeeFile_PrintObject(TOP, print_object);
				Dee_Decref(print_object);
				if unlikely(error)
					HANDLE_EXCEPT();
			}
#else /* EXEC_SAFE */
			if unlikely(DeeFile_PrintObject(TOP, CONSTimm))
				HANDLE_EXCEPT();
#endif /* !EXEC_SAFE */
			DISPATCH();
		}

		TARGET(ASM_FPRINT_C_SP, -1, +1) {
			imm_val = READ_imm8();
do_fprint_c_sp:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *print_object;
				int error;
				CONST_LOCKREAD();
				print_object = CONSTimm;
				Dee_Incref(print_object);
				CONST_LOCKENDREAD();
				error = DeeFile_PrintObjectSp(TOP, print_object);
				Dee_Decref(print_object);
				if unlikely(error)
					HANDLE_EXCEPT();
			}
#else /* EXEC_SAFE */
			if unlikely(DeeFile_PrintObjectSp(TOP, CONSTimm))
				HANDLE_EXCEPT();
#endif /* !EXEC_SAFE */
			DISPATCH();
		}

		TARGET(ASM_FPRINT_C_NL, -1, +1) {
			imm_val = READ_imm8();
do_fprint_c_nl:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *print_object;
				int error;
				CONST_LOCKREAD();
				print_object = CONSTimm;
				Dee_Incref(print_object);
				CONST_LOCKENDREAD();
				error = DeeFile_PrintObjectNl(TOP, print_object);
				Dee_Decref(print_object);
				if unlikely(error)
					HANDLE_EXCEPT();
			}
#else /* EXEC_SAFE */
			if unlikely(DeeFile_PrintObjectNl(TOP, CONSTimm))
				HANDLE_EXCEPT();
#endif /* !EXEC_SAFE */
			DISPATCH();
		}

		TARGET(ASM_RANGE_0_I16, -0, +1) {
			DREF DeeObject *range_object;
#if __SIZEOF_SIZE_T__ <= 2
#error "sizeof(size_t) is too small and may cause an overflow for range object (WTF? a 16-bit machine?)"
#endif
			range_object = DeeRange_NewInt(0, READ_imm16(), 1);
			if unlikely(!range_object)
				HANDLE_EXCEPT();
			PUSH(range_object);
			DISPATCH();
		}

		TARGET(ASM_RANGE, -2, +1) {
			DREF DeeObject *range_object;
			range_object = DeeRange_New(DeeNone_Check(SECOND) ? DeeInt_Zero : SECOND,
			                            FIRST, NULL);
			if unlikely(!range_object)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_object; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_RANGE_DEF, -1, +1) {
			DREF DeeObject *range_object, *begin;
			begin = DeeObject_NewDefault(Dee_TYPE(TOP));
			if unlikely(!begin)
				HANDLE_EXCEPT();
			range_object = DeeRange_New(begin, TOP, NULL);
			Dee_Decref(begin);
			if unlikely(!range_object)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = range_object; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_RANGE_STEP, -3, +1) {
			DREF DeeObject *range_object;
			range_object = DeeRange_New(DeeNone_Check(THIRD) ? DeeInt_Zero : THIRD, SECOND,
			                            DeeNone_Check(FIRST) ? NULL : FIRST);
			if unlikely(!range_object)
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_object; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_RANGE_STEP_DEF, -2, +1) {
			DREF DeeObject *range_object, *begin;
			begin = DeeObject_NewDefault(Dee_TYPE(SECOND));
			if unlikely(!begin)
				HANDLE_EXCEPT();
			range_object = DeeRange_New(begin, SECOND,
			                            DeeNone_Check(FIRST) ? NULL : FIRST);
			Dee_Decref(begin);
			if unlikely(!range_object)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_object; /* Inherit reference. */
			DISPATCH();
		}

		/* With-operators. */
		TARGET(ASM_ENTER, -1, +1) {
			if (DeeObject_Enter(TOP))
				HANDLE_EXCEPT();
			DISPATCH();
		}
		TARGET(ASM_LEAVE, -1, +0) {
			if (DeeObject_Leave(TOP))
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}


		/* Sequence operators. */
		TARGET(ASM_GETSIZE, -1, +1) {
			DREF DeeObject *object_size;
			object_size = DeeObject_SizeObject(TOP);
			if unlikely(!object_size)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = object_size; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_CONTAINS, -2, +1) {
			DREF DeeObject *does_contain;
			does_contain = DeeObject_ContainsObject(SECOND, FIRST);
			if unlikely(!does_contain)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = does_contain; /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_CONTAINS_C) {
			DREF DeeObject *value;
			imm_val = READ_imm8();
do_contains_c:
			ASSERT_USAGE(-1, +1);
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *constant_set;
				CONST_LOCKREAD();
				constant_set = CONSTimm;
				Dee_Incref(constant_set);
				CONST_LOCKENDREAD();
				value = DeeObject_ContainsObject(constant_set, TOP);
				Dee_Decref(constant_set);
			}
#else /* EXEC_SAFE */
			value = DeeObject_ContainsObject(CONSTimm, TOP);
#endif /* !EXEC_SAFE */
			if unlikely(!value)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETITEM, -2, +1) {
			DREF DeeObject *value;
			value = DeeObject_GetItem(SECOND, FIRST);
			if unlikely(!value)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETITEM_I, -1, +1) {
			DREF DeeObject *value;
			value = DeeObject_GetItemIndex(TOP, READ_Simm16());
			if unlikely(!value)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = value; /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_GETITEM_C) {
			DREF DeeObject *value;
			imm_val = READ_imm8();
do_getitem_c:
			ASSERT_USAGE(-1, +1);
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *index_object;
				CONST_LOCKREAD();
				index_object = CONSTimm;
				Dee_Incref(index_object);
				CONST_LOCKENDREAD();
				value = DeeObject_GetItem(TOP, index_object);
				Dee_Decref(index_object);
			}
#else /* EXEC_SAFE */
			value = DeeObject_GetItem(TOP, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(!value)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_SETITEM, -3, +0) {
			if (DeeObject_SetItem(THIRD, SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETITEM_I, -2, +0) {
			if (DeeObject_SetItemIndex(SECOND, READ_Simm16(), FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETITEM_C, -2, +0) {
			imm_val = READ_imm8();
do_setitem_c:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *index_object;
				int error;
				CONST_LOCKREAD();
				index_object = CONSTimm;
				Dee_Incref(index_object);
				CONST_LOCKENDREAD();
				error = DeeObject_SetItem(SECOND, index_object, FIRST);
				Dee_Decref(index_object);
				if unlikely(error)
					HANDLE_EXCEPT();
			}
#else /* EXEC_SAFE */
			if (DeeObject_SetItem(SECOND, CONSTimm, FIRST))
				HANDLE_EXCEPT();
#endif /* !EXEC_SAFE */
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_DELITEM, -2, +0) {
			if (DeeObject_DelItem(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_GETRANGE, -3, +1) {
			DREF DeeObject *range_value;
			range_value = DeeObject_GetRange(THIRD, SECOND, FIRST);
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETRANGE_PN, -2, +1) {
			DREF DeeObject *range_value;
			range_value = DeeObject_GetRange(SECOND, FIRST, Dee_None);
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETRANGE_NP, -2, +1) {
			DREF DeeObject *range_value;
			range_value = DeeObject_GetRange(SECOND, Dee_None, FIRST);
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETRANGE_PI, -2, +1) {
			DREF DeeObject *range_value;
			range_value = DeeObject_GetRangeEndIndex(SECOND, FIRST, READ_Simm16());
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETRANGE_IP, -2, +1) {
			DREF DeeObject *range_value;
			range_value = DeeObject_GetRangeBeginIndex(SECOND, READ_Simm16(), FIRST);
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETRANGE_NI, -1, +1) {
			DREF DeeObject *range_value;
			range_value = DeeObject_GetRangeEndIndex(TOP, Dee_None, READ_Simm16());
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETRANGE_IN, -1, +1) {
			DREF DeeObject *range_value;
			range_value = DeeObject_GetRangeBeginIndex(TOP, READ_Simm16(), Dee_None);
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETRANGE_II, -1, +1) {
			DREF DeeObject *range_value;
			int16_t begin = READ_Simm16();
			range_value   = DeeObject_GetRangeIndex(TOP, begin, READ_Simm16());
			if unlikely(!range_value)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = range_value; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_DELRANGE, -3, +0) {
			if (DeeObject_DelRange(THIRD, SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE, -4, +0) {
			if (DeeObject_SetRange(FOURTH, THIRD, SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE_PN, -3, +0) {
			if (DeeObject_SetRange(THIRD, SECOND, Dee_None, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE_NP, -3, +0) {
			if (DeeObject_SetRange(THIRD, Dee_None, SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE_PI, -3, +0) {
			if unlikely(DeeObject_SetRangeEndIndex(THIRD, SECOND, READ_Simm16(), FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE_IP, -3, +0) {
			if unlikely(DeeObject_SetRangeBeginIndex(THIRD, READ_Simm16(), SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE_NI, -2, +0) {
			if unlikely(DeeObject_SetRangeEndIndex(SECOND, Dee_None, READ_Simm16(), FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE_IN, -2, +0) {
			if unlikely(DeeObject_SetRangeBeginIndex(SECOND, READ_Simm16(), Dee_None, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETRANGE_II, -2, +0) {
			int16_t begin = READ_Simm16();
			if unlikely(DeeObject_SetRangeIndex(SECOND, begin, READ_Simm16(), FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		/* Breakpoint. */
		TARGET(ASM_BREAKPOINT, -0, +0) {
			int error;

			/* Safe the instruction + stack-pointer. */
			frame->cf_ip = ip.ptr;
			frame->cf_sp = sp;
#ifdef EXEC_FAST
			frame->cf_stacksz = STACKPREALLOC;
#endif /* EXEC_FAST */

			/* Trigger a breakpoint. */
			error = trigger_breakpoint(frame);

			/* _always_ load new PC/SP from the frame to ensure that we are in a
			 * consistent state before handling potential exception, or moving on.
			 * After all: The purpose of this instruction is to allow some external
			 *            utility to inspect and/or modify our frame before either
			 *            raising an exception, or passing control back to running code.
			 * HINT: The idea for breakpoints is for some utility to replace opcodes
			 *       that the runtime should pause at with `ASM_BREAKPOINT' instructions,
			 *       causing it to halt and pass control over to said utility which
			 *       will then be able to restore the original byte that was replaced
			 *       with the breakpoint instruction.
			 *    >> Using this fairly simple trick, it is even possible to allow for 
			 *       step-by-step execution of code, simply by always replacing the next
			 *       instruction with a breakpoint (or in the case of a branch: the next
			 *       instruction, as well as the branch target), allowing code to be
			 *       executed an-instruction-at-a-time.
			 *       Even unpredictable instruction like `ASM_JMP_POP' or `ASM_JMP_POP_POP'
			 *       become predictable with this, as the debugger can simply evaluate
			 *       the stack to see where they will branch to! */
			sp     = frame->cf_sp;
			ip.ptr = frame->cf_ip;

			/* Re-load the effective code object, allowing a
			 * breakpoint handler to exchange the running code.
			 * This can be happen if the breakpoint handler is supposed
			 * to overwrite certain code instructions when doing so could
			 * harm other threads also running the same code, which would
			 * then also run into the breakpoint.
			 * In such cases, a breakpoint library may wish to replace
			 * the running code object with a duplicate, or some other
			 * code, essentially giving even more freedom by literally
			 * exchanging the assembly that should be executing. */
			code = frame->cf_func->fo_code;

			/* Check if we're supposed to handle an exception now. */
			switch (error) {

			case TRIGGER_BREAKPOINT_EXCEPT_EXIT:
				if (ITER_ISOK(frame->cf_result))
					Dee_Decref(frame->cf_result);
				frame->cf_result = NULL;
				goto end_without_finally;

			case TRIGGER_BREAKPOINT_EXIT:
			case TRIGGER_BREAKPOINT_EXIT_NOFIN:
				if (code->co_flags & CODE_FYIELDING) {
					if (ITER_ISOK(frame->cf_result))
						Dee_Decref(frame->cf_result);
					frame->cf_result = ITER_DONE;
				} else {
					ASSERT(frame->cf_result != ITER_DONE);
					if (!frame->cf_result) {
						/* Return `none' when no return value has been set. */
						frame->cf_result = Dee_None;
						Dee_Incref(Dee_None);
					}
				}
				if (error == TRIGGER_BREAKPOINT_EXIT_NOFIN)
					goto end_without_finally;
				goto end_return;

			case TRIGGER_BREAKPOINT_RETURN:
				if (code->co_flags & CODE_FYIELDING) {
					if (frame->cf_result == NULL)
						frame->cf_result = ITER_DONE;
					goto end_without_finally;
				}
				if (frame->cf_result == NULL) {
					frame->cf_result = Dee_None;
					Dee_Incref(Dee_None);
				}
				goto end_return;

#ifdef EXEC_FAST
			case TRIGGER_BREAKPOINT_CONTSAFE:
				/* Unhook frame from the thread-local execution stack.
				 *  - As we're about to re-enter the same frame, we've got no
				 *    way of skipping the frame setup, meaning that this switch
				 *    may result in the frame missing for a tiny moment.
				 * >> But that should be ok... */
				ASSERT(this_thread->t_execsz != 0);
				ASSERT(this_thread->t_exec == frame);
				ASSERT(frame->cf_prev != CODE_FRAME_NOT_EXECUTING);
				--this_thread->t_execsz;
				this_thread->t_exec = frame->cf_prev;
				frame->cf_prev      = CODE_FRAME_NOT_EXECUTING;

				/* Indicate that the stack hasn't been allocated dynamically. */
				frame->cf_stacksz = 0;

				/* Continue execution in safe-mode. */
				DeeCode_ExecFrameSafe(frame);

				/* Once safe-mode execution finishes, propagate it's return value
				 * and clean up any additional exception that we've raised before. */
				goto end_nounhook;
#endif /* EXEC_FAST */

			default:
				if (error < 0)
					HANDLE_EXCEPT();
				break;
			}
			DISPATCH();
		}

		/* Sequence iterator creation. */
		TARGET(ASM_ITERSELF, -1, +1) {
			DREF DeeObject *iterator;
			iterator = DeeObject_IterSelf(TOP);
			if unlikely(!iterator)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = iterator;
			/* Predict that this will be a foreach loop. */
			PREDICT(ASM_FOREACH);
			/*PREDICT(ASM_FOREACH16);*/
			DISPATCH();
		}

		/* Call attribute. */
		RAW_TARGET(ASM_CALLATTR) {
			DREF DeeObject *resval, **new_sp;
			uint8_t n_args = READ_imm8();
			ASSERT_USAGE(-(2 + (int)n_args), +1);
			new_sp = sp - n_args;
			if unlikely(!DeeString_Check(new_sp[-1])) {
				err_expected_string_for_attribute(new_sp[-1]);
				HANDLE_EXCEPT();
			}
			resval = DeeObject_CallAttr(new_sp[-2], new_sp[-1], n_args, new_sp);
			if unlikely(!resval)
				HANDLE_EXCEPT();
			while (n_args--)
				POPREF();
			POPREF();
			Dee_Decref(TOP);
			TOP = resval; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_CALLATTR_TUPLE, -3, +1) {
			DREF DeeObject *call_result;
			ASSERT_TUPLE(FIRST);
			if unlikely(!DeeString_Check(SECOND)) {
				err_expected_string_for_attribute(SECOND);
				HANDLE_EXCEPT();
			}
			call_result = DeeObject_CallAttrTuple(THIRD, SECOND, FIRST);
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			Dee_Decref(TOP);
			TOP = call_result; /* Inherit reference. */
			DISPATCH();
		}


		RAW_TARGET(ASM_CALLATTR_C_KW) {
			uint8_t argc;
			DREF DeeObject *call_result;
			DeeObject **new_sp;
			imm_val  = READ_imm8();
			argc     = READ_imm8();
			imm_val2 = READ_imm8();
			ASSERT_USAGE(-((int)argc + 1), +1);
			ASSERT_CONSTimm();
			ASSERT_CONSTimm2();
			new_sp = sp - argc;
#ifdef EXEC_SAFE
			{
				DREF DeeObject *attr_name;
				DREF DeeObject *kwds_map;
				CONST_LOCKREAD();
				attr_name = CONSTimm;
				kwds_map  = CONSTimm2;
				Dee_Incref(attr_name);
				Dee_Incref(kwds_map);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(attr_name)) {
					Dee_Decref_unlikely(attr_name);
					Dee_Decref_unlikely(kwds_map);
					goto err_requires_string;
				}
				call_result = DeeObject_CallAttrKw(new_sp[-1],
				                                   CONSTimm,
				                                   argc,
				                                   new_sp,
				                                   CONSTimm2);
				Dee_Decref_unlikely(attr_name);
				Dee_Decref_unlikely(kwds_map);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			call_result = DeeObject_CallAttrKw(new_sp[-1],
			                                   CONSTimm,
			                                   argc,
			                                   new_sp,
			                                   CONSTimm2);
#endif /* !EXEC_SAFE */
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			while (sp > new_sp)
				POPREF();
			Dee_Decref(TOP);
			TOP = call_result; /* Inherit reference. */
			DISPATCH();
		}
		RAW_TARGET(ASM_CALLATTR_C_TUPLE_KW) {
			DREF DeeObject *call_result;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_callattr_tuple_c_kw:
			ASSERT_USAGE(-2, +1);
			ASSERT_CONSTimm();
			ASSERT_CONSTimm2();
			ASSERT_TUPLE(TOP);
#ifdef EXEC_SAFE
			{
				DREF DeeObject *attr_name;
				DREF DeeObject *kwds_map;
				CONST_LOCKREAD();
				attr_name = CONSTimm;
				kwds_map  = CONSTimm2;
				Dee_Incref(attr_name);
				Dee_Incref(kwds_map);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(attr_name)) {
					Dee_Decref_unlikely(attr_name);
					Dee_Decref_unlikely(kwds_map);
					goto err_requires_string;
				}
				call_result = DeeObject_CallAttrTupleKw(SECOND, attr_name, FIRST, kwds_map);
				Dee_Decref_unlikely(attr_name);
				Dee_Decref_unlikely(kwds_map);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			call_result = DeeObject_CallAttrTupleKw(SECOND, CONSTimm, FIRST, CONSTimm2);
#endif /* !EXEC_SAFE */
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = call_result; /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_CALLATTR_C) {
			DREF DeeObject *callback_result, **new_sp;
			imm_val = READ_imm8();
do_callattr_c:
			imm_val2 = READ_imm8();
			ASSERT_USAGE(-((int)imm_val2 + 1), +1);
			ASSERT_CONSTimm();
			new_sp = sp - imm_val2;
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref_unlikely(imm_name);
					goto err_requires_string;
				}
				callback_result = DeeObject_CallAttr(new_sp[-1], imm_name, imm_val2, new_sp);
				Dee_Decref_unlikely(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			callback_result = DeeObject_CallAttr(new_sp[-1], CONSTimm, imm_val2, new_sp);
#endif /* !EXEC_SAFE */
			if unlikely(!callback_result)
				HANDLE_EXCEPT();
			while (imm_val2--)
				POPREF();
			Dee_Decref(TOP);
			TOP = callback_result; /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_CALLATTR_C_SEQ) {
			DREF DeeObject *callback_result, **new_sp;
			DREF DeeObject *shared_vector;
			imm_val = READ_imm8();
do_callattr_c_seq:
			imm_val2 = READ_imm8();
			ASSERT_USAGE(-((int)imm_val2 + 1), +1);
			ASSERT_CONSTimm();
			new_sp = sp - imm_val2;
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				shared_vector = DeeSharedVector_NewShared(imm_val2, new_sp);
				if unlikely(!shared_vector) {
					Dee_Decref(imm_name);
					HANDLE_EXCEPT();
				}
				sp              = new_sp;
				callback_result = DeeObject_CallAttr(new_sp[-1], imm_name, 1,
				                                     (DeeObject **)&shared_vector);
				DeeSharedVector_Decref(shared_vector);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			shared_vector = DeeSharedVector_NewShared(imm_val2, new_sp);
			if unlikely(!shared_vector)
				HANDLE_EXCEPT();
			sp              = new_sp;
			callback_result = DeeObject_CallAttr(new_sp[-1], CONSTimm, 1,
			                                     (DeeObject **)&shared_vector);
			DeeSharedVector_Decref(shared_vector);
#endif /* !EXEC_SAFE */
			if unlikely(!callback_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = callback_result; /* Inherit reference. */
			DISPATCH();
		}

		RAW_TARGET(ASM_CALLATTR_C_MAP) {
			USING_PREFIX_OBJECT
			DREF DeeObject *callback_result, **new_sp;
			imm_val = READ_imm8();
do_callattr_c_map:
			imm_val2 = READ_imm8();
			ASSERT_USAGE(-((int)(imm_val2 * 2) + 1), +1);
			ASSERT_CONSTimm();
			new_sp = sp - imm_val2 * 2;
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				prefix_ob = DeeSharedMap_NewShared(imm_val2, (DREF DeeSharedItem *)new_sp);
				if unlikely(!prefix_ob) {
					Dee_Decref(imm_name);
					HANDLE_EXCEPT();
				}
				sp              = new_sp;
				callback_result = DeeObject_CallAttr(new_sp[-1], imm_name, 1,
				                                     (DeeObject **)&prefix_ob);
				DeeSharedMap_Decref(prefix_ob);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			prefix_ob = DeeSharedMap_NewShared(imm_val2, (DREF DeeSharedItem *)new_sp);
			if unlikely(!prefix_ob)
				HANDLE_EXCEPT();
			sp              = new_sp;
			callback_result = DeeObject_CallAttr(new_sp[-1], CONSTimm, 1,
			                                     (DeeObject **)&prefix_ob);
			DeeSharedMap_Decref(prefix_ob);
#endif /* !EXEC_SAFE */
			if unlikely(!callback_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = callback_result; /* Inherit reference. */
			DISPATCH();
		}


		RAW_TARGET(ASM_CALLATTR_THIS_C) {
			DREF DeeObject *callback_result;
			imm_val = READ_imm8();
do_callattr_this_c:
			imm_val2 = READ_imm8();
			ASSERT_THISCALL();
			ASSERT_USAGE(-(int)imm_val2, +1);
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				callback_result = DeeObject_CallAttr(THIS, imm_name, imm_val2, sp - imm_val2);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			callback_result = DeeObject_CallAttr(THIS, CONSTimm, imm_val2, sp - imm_val2);
#endif /* !EXEC_SAFE */
			if unlikely(!callback_result)
				HANDLE_EXCEPT();
			while (imm_val2--)
				POPREF();
			PUSH(callback_result); /* Push the result onto the stack. */
			DISPATCH();
		}

		TARGET(ASM_CALLATTR_C_TUPLE, -2, +1) {
			DREF DeeObject *callback_result;
			imm_val = READ_imm8();
do_callattr_tuple_c:
			ASSERT_TUPLE(FIRST);
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				callback_result = DeeObject_CallAttrTuple(SECOND, imm_name, FIRST);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			callback_result = DeeObject_CallAttrTuple(SECOND, CONSTimm, FIRST);
#endif /* !EXEC_SAFE */
			if unlikely(!callback_result)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = callback_result; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_CALLATTR_THIS_C_TUPLE, -1, +1) {
			DREF DeeObject *callback_result;
			imm_val = READ_imm8();
do_callattr_this_tuple_c:
			ASSERT_THISCALL();
			ASSERT_TUPLE(TOP);
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				callback_result = DeeObject_CallAttrTuple(THIS, imm_name, TOP);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			callback_result = DeeObject_CallAttrTuple(THIS, CONSTimm, TOP);
#endif /* !EXEC_SAFE */
			if unlikely(!callback_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = callback_result; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_GETMEMBER_THIS_R, -0, +1) {
			DREF DeeObject *result;
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_getmember_r:
			ASSERT_THISCALL();
#ifdef EXEC_SAFE
			result = DeeInstance_GetMemberSafe((DeeTypeObject *)REFimm, THIS, imm_val2);
#else /* EXEC_SAFE */
			result = DeeInstance_GetMember((DeeTypeObject *)REFimm, THIS, imm_val2);
#endif /* !EXEC_SAFE */
			if unlikely(!result)
				HANDLE_EXCEPT();
			PUSH(result);
			DISPATCH();
		}

		TARGET(ASM_BOUNDMEMBER_THIS_R, -0, +1) {
#ifdef EXEC_SAFE
			int temp;
#else /* EXEC_SAFE */
			bool temp;
#endif /* !EXEC_SAFE */
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_boundmember_r:
			ASSERT_THISCALL();
			ASSERT_REFimm();
#ifdef EXEC_SAFE
			temp = DeeInstance_BoundMemberSafe((DeeTypeObject *)REFimm, THIS, imm_val2);
			if unlikely(temp < 0)
				HANDLE_EXCEPT();
			PUSHREF(DeeBool_For(temp != 0));
#else /* EXEC_SAFE */
			temp = DeeInstance_BoundMember((DeeTypeObject *)REFimm, THIS, imm_val2);
			PUSHREF(DeeBool_For(temp));
#endif /* !EXEC_SAFE */
			DISPATCH();
		}

		TARGET(ASM_DELMEMBER_THIS_R, -0, +0) {
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_delmember_r:
			ASSERT_THISCALL();
			ASSERT_REFimm();
#ifdef EXEC_SAFE
			if (DeeInstance_DelMemberSafe((DeeTypeObject *)REFimm, THIS, imm_val2))
				HANDLE_EXCEPT();
#else /* EXEC_SAFE */
			if (DeeInstance_DelMember((DeeTypeObject *)REFimm, THIS, imm_val2))
				HANDLE_EXCEPT();
#endif /* !EXEC_SAFE */
			DISPATCH();
		}

		TARGET(ASM_SETMEMBER_THIS_R, -1, +0) {
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_setmember_r:
			ASSERT_THISCALL();
			ASSERT_REFimm();
#ifdef EXEC_SAFE
			if unlikely(DeeInstance_SetMemberSafe((DeeTypeObject *)REFimm, THIS, imm_val2, TOP))
				HANDLE_EXCEPT();
#else /* EXEC_SAFE */
			DeeInstance_SetMember((DeeTypeObject *)REFimm, THIS,
			                      imm_val2, TOP);
#endif /* !EXEC_SAFE */
			POPREF();
			DISPATCH();
		}


		{
			DREF DeeObject *call_object, *call_result;
			RAW_TARGET(ASM_CALL_EXTERN)
			imm_val  = READ_imm8();
			imm_val2 = READ_imm8();
do_call_extern:
			ASSERT_EXTERNimm();
			EXTERN_LOCKREAD();
			call_object = EXTERNimm;
			Dee_XIncref(call_object);
			EXTERN_LOCKENDREAD();
			if unlikely(!call_object)
				goto err_unbound_extern;
do_object_call_imm:
			imm_val = READ_imm8();
#ifdef EXEC_SAFE
			/* Assert stack usage. */
			if unlikely(imm_val > STACKUSED || (!imm_val && !STACKFREE)) {
				Dee_Decref(call_object);
				if (imm_val > STACKUSED)
					goto err_invalid_stack_affect;
				goto increase_stacksize;
			}
#else /* EXEC_SAFE */
			ASSERT_USAGE(-(int)imm_val, +1);
#endif /* !EXEC_SAFE */
			call_result = DeeObject_Call(call_object, imm_val, sp - imm_val);
			Dee_Decref(call_object);
			if unlikely(!call_result)
				HANDLE_EXCEPT();
			while (imm_val--)
				POPREF();
			PUSH(call_result);
			DISPATCH();
			RAW_TARGET(ASM_CALL_GLOBAL)
			imm_val = READ_imm8();
do_call_global:
			ASSERT_GLOBALimm();
			GLOBAL_LOCKREAD();
			call_object = GLOBALimm;
			Dee_XIncref(call_object);
			GLOBAL_LOCKENDREAD();
			if unlikely(!call_object)
				goto err_unbound_global;
			goto do_object_call_imm;
			RAW_TARGET(ASM_CALL_LOCAL)
			imm_val = READ_imm8();
do_call_local:
			ASSERT_LOCALimm();
			call_object = LOCALimm;
			if unlikely(!call_object)
				goto err_unbound_local;
			Dee_Incref(call_object);
			goto do_object_call_imm;
		}

		TARGET(ASM_GETATTR, -2, +1) {
			DeeObject *attr_result;
			if unlikely(!DeeString_Check(FIRST)) {
				err_expected_string_for_attribute(FIRST);
				HANDLE_EXCEPT();
			}
			attr_result = DeeObject_GetAttr(SECOND, FIRST);
			if unlikely(!attr_result)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = attr_result; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_DELATTR, -2, +0) {
			if unlikely(!DeeString_Check(FIRST)) {
				err_expected_string_for_attribute(FIRST);
				HANDLE_EXCEPT();
			}
			if (DeeObject_DelAttr(SECOND, FIRST))
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETATTR, -3, +0) {
			int error;
			if unlikely(!DeeString_Check(SECOND)) {
				err_expected_string_for_attribute(SECOND);
				HANDLE_EXCEPT();
			}
			error = DeeObject_SetAttr(THIRD, SECOND, FIRST);
			if unlikely(error < 0)
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_BOUNDATTR, -2, +1) {
			int error;
			if unlikely(!DeeString_Check(FIRST)) {
				err_expected_string_for_attribute(FIRST);
				HANDLE_EXCEPT();
			}
			error = DeeObject_BoundAttr(SECOND, FIRST);
			if unlikely(error == -1)
				HANDLE_EXCEPT();
			POPREF();
			Dee_Decref(TOP);
			TOP = DeeBool_For(error > 0);
			Dee_Incref(TOP);
			DISPATCH();
		}

		TARGET(ASM_GETATTR_C, -1, +1) {
			DREF DeeObject *getattr_result;
			imm_val = READ_imm8();
do_getattr_c:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				getattr_result = DeeObject_GetAttr(TOP, imm_name);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			getattr_result = DeeObject_GetAttr(TOP, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(!getattr_result)
				HANDLE_EXCEPT();
			Dee_Decref(TOP);
			TOP = getattr_result; /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_DELATTR_C, -1, +0) {
			int error;
			imm_val = READ_imm8();
do_delattr_c:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				error = DeeObject_DelAttr(TOP, imm_name);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			error = DeeObject_DelAttr(TOP, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(error < 0)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_SETATTR_C, -2, +0) {
			int error;
			imm_val = READ_imm8();
do_setattr_c:
			ASSERT_CONSTimm();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				error = DeeObject_SetAttr(SECOND, imm_name, FIRST);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			error = DeeObject_SetAttr(SECOND, CONSTimm, FIRST);
#endif /* !EXEC_SAFE */
			if unlikely(error < 0)
				HANDLE_EXCEPT();
			POPREF();
			POPREF();
			DISPATCH();
		}

		TARGET(ASM_GETATTR_THIS_C, -0, +1) {
			DREF DeeObject *getattr_result;
			imm_val = READ_imm8();
do_getattr_this_c:
			ASSERT_CONSTimm();
			ASSERT_THISCALL();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				getattr_result = DeeObject_GetAttr(THIS, imm_name);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			getattr_result = DeeObject_GetAttr(THIS, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(!getattr_result)
				HANDLE_EXCEPT();
			PUSH(getattr_result); /* Inherit reference. */
			DISPATCH();
		}

		TARGET(ASM_DELATTR_THIS_C, -0, +0) {
			int error;
			imm_val = READ_imm8();
do_delattr_this_c:
			ASSERT_CONSTimm();
			ASSERT_THISCALL();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				error = DeeObject_DelAttr(THIS, imm_name);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			error = DeeObject_DelAttr(THIS, CONSTimm);
#endif /* !EXEC_SAFE */
			if unlikely(error < 0)
				HANDLE_EXCEPT();
			DISPATCH();
		}

		TARGET(ASM_SETATTR_THIS_C, -1, +0) {
			int error;
			imm_val = READ_imm8();
do_setattr_this_c:
			ASSERT_CONSTimm();
			ASSERT_THISCALL();
#ifdef EXEC_SAFE
			{
				DREF DeeObject *imm_name;
				CONST_LOCKREAD();
				imm_name = CONSTimm;
				Dee_Incref(imm_name);
				CONST_LOCKENDREAD();
				if unlikely(!DeeString_CheckExact(imm_name)) {
					Dee_Decref(imm_name);
					goto err_requires_string;
				}
				error = DeeObject_SetAttr(THIS, imm_name, TOP);
				Dee_Decref(imm_name);
			}
#else /* EXEC_SAFE */
			ASSERT_STRING(CONSTimm);
			error = DeeObject_SetAttr(THIS, CONSTimm, TOP);
#endif /* !EXEC_SAFE */
			if unlikely(error < 0)
				HANDLE_EXCEPT();
			POPREF();
			DISPATCH();
		}



		/* Opcodes for misc/rarely used operators (Prefixed by `ASM_EXTENDED1'). */
		RAW_TARGET(ASM_EXTENDED1) {

#ifdef USE_SWITCH
			switch (*ip.ptr++)
#else /* USE_SWITCH */
			goto *f0_targets[*ip.ptr++];
#endif /* !USE_SWITCH */
			{

				TARGET(ASM_ENDCATCH_N, -0, +0) {
					uint8_t nth_except = READ_imm8();
					if (this_thread->t_exceptsz > except_recursion + nth_except + 1) {
						struct except_frame **p_except_frame, *except_frame;

						/* We're allowed to handle the `nth_except' exception. */
						p_except_frame = &this_thread->t_except;
						do {
							except_frame = *p_except_frame;
							ASSERT(except_frame != NULL);
							p_except_frame = &except_frame->ef_prev;
						} while (nth_except--);

						/* Load the except_frame that we're supposed to get rid of. */
						except_frame = *p_except_frame;
						ASSERT(except_frame != NULL);

						/* Remove the exception frame from its chain. */
						*p_except_frame = except_frame->ef_prev;
						--this_thread->t_exceptsz;

						/* Destroy the except_frame in question. */
						if (ITER_ISOK(except_frame->ef_trace))
							Dee_Decref(except_frame->ef_trace);
						Dee_Decref(except_frame->ef_error);
						except_frame_free(except_frame);
					}
					DISPATCH();
				}

				TARGET(ASM_ENDFINALLY_N, -0, +0) {
					uint8_t min_except = READ_imm8();

					/* If a return value has been assigned, stop execution. */
					if (frame->cf_result != NULL)
						goto end_return;

					/* Check for errors, but only handle them if there are more than `min_except+1'. */
					if (this_thread->t_exceptsz > except_recursion + min_except + 1)
						HANDLE_EXCEPT();
					DISPATCH();
				}

				TARGET(ASM16_PUSH_BND_ARG, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_bnd_arg;
				}


				TARGET(ASM16_PUSH_BND_EXTERN, -0, +1) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_push_bnd_extern;
				}

				TARGET(ASM16_PUSH_BND_GLOBAL, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_bnd_global;
				}

				TARGET(ASM16_PUSH_BND_LOCAL, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_bnd_local;
				}

				RAW_TARGET(ASM32_JMP) {
					int32_t disp32;
					disp32 = READ_Simm32();
					if (disp32 < 0) {
						if (DeeThread_CheckInterruptSelf(this_thread))
							HANDLE_EXCEPT();
					}
					ip.ptr += disp32;
#ifdef EXEC_SAFE
					goto assert_ip_bounds;
#else /* EXEC_SAFE */
					ASSERT(ip.ptr >= code->co_code);
					ASSERT(ip.ptr < code->co_code + code->co_codebytes);
					DISPATCH();
#endif /* !EXEC_SAFE */
				}

				TARGET(ASM_JMP_POP_POP, -2, +0) {
					code_addr_t absip;
					uint16_t absstk, stksz;
					instruction_t *new_ip;
					if (DeeObject_AsUInt16(TOP, &absstk))
						HANDLE_EXCEPT();
					if (DeeObject_AsUInt32(SECOND, &absip))
						HANDLE_EXCEPT();
#ifdef EXEC_SAFE
					if (absip >= code->co_codebytes) {
						DeeError_Throwf(&DeeError_SegFault,
						                "Invalid PC %.4I32X in absolute jmp",
						                absip);
						HANDLE_EXCEPT();
					}
					if (absstk > STACKSIZE)
						goto increase_stacksize;
#else /* EXEC_SAFE */
					ASSERTF(absip < code->co_codebytes, "Invalid PC: %X",
					        (unsigned int)absip);
					ASSERT(absstk <= STACKSIZE);
#endif /* !EXEC_SAFE */
					new_ip = code->co_code + absip;
					if (new_ip < ip.ptr) {
						if (DeeThread_CheckInterruptSelf(this_thread))
							HANDLE_EXCEPT();
					}
					stksz = (uint16_t)(sp - frame->cf_stack);
					if (absstk > stksz) {
						absstk -= stksz;
						while (absstk--)
							PUSHREF(Dee_None);
					} else {
						stksz -= absstk;
						while (stksz--)
							POPREF();
					}
					ip.ptr = new_ip;
					DISPATCH();
				}

				RAW_TARGET(ASM16_CALL_KW) {
					imm_val2 = READ_imm8();
					imm_val  = READ_imm16();
					goto do_call_kw;
				}

				RAW_TARGET(ASM16_CALL_TUPLE_KW) {
					imm_val = READ_imm16();
					goto do_call_tuple_kw;
				}

				RAW_TARGET(ASM_CALL_TUPLE_KWDS) {
					DREF DeeObject *call_result;
					ASSERT_USAGE(-3, +1);
					ASSERT_TUPLE(SECOND);
					call_result = DeeObject_CallTupleKw(THIRD, SECOND, FIRST);
					if unlikely(!call_result)
						HANDLE_EXCEPT();
					POPREF();
					POPREF();
					Dee_Decref(TOP);
					TOP = call_result; /* Save the callback result. */
					DISPATCH();
				}

				RAW_TARGET(ASM_CALL_SEQ) {
					/* Sequence constructor invocation (Implemented using `_SharedVector'). */
					USING_PREFIX_OBJECT
					uint8_t n_args = READ_imm8();
					DREF DeeObject *callback_result;
					ASSERT_USAGE(-((int)n_args + 1), +1);
					prefix_ob = DeeSharedVector_NewShared(n_args, sp - n_args);
					if unlikely(!prefix_ob)
						HANDLE_EXCEPT();
					sp -= n_args; /* These operands have been inherited `DeeSharedVector_NewShared' */
					/* Invoke the object that is now located in TOP
					 * For this invocation, we pass only a single argument `prefix_ob' */
					callback_result = DeeObject_Call(TOP, 1, (DeeObject **)&prefix_ob);
					DeeSharedVector_Decref(prefix_ob);
					if unlikely(!callback_result)
						HANDLE_EXCEPT();
					/* Replace the function that was called with its return value. */
					Dee_Decref(TOP);
					TOP = callback_result;
					DISPATCH();
				}

				RAW_TARGET(ASM_CALL_MAP) {
					/* Dict-style sequence constructor invocation (Implemented using `_sharedkeyvector'). */
					USING_PREFIX_OBJECT
					uint8_t n_args = READ_imm8();
					DREF DeeObject *callback_result;
					ASSERT_USAGE(-(((int)n_args * 2) + 1), +1);

					/* NOTE: The vector of DeeSharedItem structures has
					 *       previously been constructed on the stack. */
					prefix_ob = DeeSharedMap_NewShared(n_args, (DeeSharedItem *)(sp - n_args * 2));
					if unlikely(!prefix_ob)
						HANDLE_EXCEPT();
					sp -= n_args * 2; /* These operands have been inherited `DeeSharedVector_NewShared' */

					/* Invoke the object that is now located in TOP
					 * For this invocation, we pass only a single argument `prefix_ob' */
					callback_result = DeeObject_Call(TOP, 1, (DeeObject **)&prefix_ob);
					DeeSharedMap_Decref(prefix_ob);
					if unlikely(!callback_result)
						HANDLE_EXCEPT();
					/* Replace the function that was called with its return value. */
					Dee_Decref(TOP);
					TOP = callback_result;
					DISPATCH();
				}

				TARGET(ASM_THISCALL_TUPLE, -3, +1) {
					DREF DeeObject *callback_result;
					ASSERT_TUPLE(FIRST);
					callback_result = DeeObject_ThisCallTuple(THIRD, SECOND, FIRST);
					if unlikely(!callback_result)
						HANDLE_EXCEPT();
					POPREF();
					POPREF();
					Dee_Decref(TOP);
					TOP = callback_result; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_PUSH_TRUE, -0, +1) {
					PUSHREF(Dee_True);
					
					DISPATCH();
				}
				TARGET(ASM_PUSH_FALSE, -0, +1) {
					PUSHREF(Dee_False);
					DISPATCH();
				}

				TARGET(ASM_PUSH_EXCEPT, -0, +1) {
					DeeObject *temp;
					/* Check if an exception has been set. */
					if unlikely(!this_thread->t_except)
						goto except_no_active_exception;
					temp = this_thread->t_except->ef_error;
					PUSHREF(temp);
					DISPATCH();
				}

				TARGET(ASM_PUSH_THIS, -0, +1) {
					ASSERT_THISCALL();
					PUSHREF(THIS);
					DISPATCH();
				}

				TARGET(ASM_PUSH_THIS_FUNCTION, -0, +1) {
					PUSHREF((DeeObject *)frame->cf_func);
					DISPATCH();
				}

				TARGET(ASM_PUSH_THIS_MODULE, -0, +1) {
					PUSHREF((DeeObject *)code->co_module);
					DISPATCH();
				}

				TARGET(ASM_CMP_SO, -2, +1) {
					bool is_same;
					is_same = SECOND == FIRST;
					POPREF();
					Dee_Decref(TOP);
					TOP = DeeBool_For(is_same);
					Dee_Incref(TOP);
					DISPATCH();
				}

				TARGET(ASM_CMP_DO, -2, +1) {
					bool is_diff;
					is_diff = SECOND != FIRST;
					POPREF();
					Dee_Decref(TOP);
					TOP = DeeBool_For(is_diff);
					Dee_Incref(TOP);
					DISPATCH();
				}

				RAW_TARGET(ASM16_DELOP)
				RAW_TARGET(ASM16_NOP) {
					/* Allow an ignore delop/nop being used with the F0 prefix. */
					DISPATCH();
				}

				RAW_TARGET(ASM16_OPERATOR) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm8();
					goto do_operator;
				}

				TARGET(ASM16_OPERATOR_TUPLE, -2, +1) {
					imm_val = READ_imm16();
					goto do_operator_tuple;
				}

				TARGET(ASM16_DEL_GLOBAL, -0, +0) {
					imm_val = READ_imm16();
					goto do_del_global;
				}

				TARGET(ASM16_DEL_LOCAL, -0, +0) {
					imm_val = READ_imm16();
					goto do_del_local;
				}

				RAW_TARGET(ASM16_LROT) {
					DREF DeeObject *temp;
					uint32_t shift;
					shift = (uint32_t)READ_imm16() + 3;
					ASSERT_USAGE(-(int)shift, +(int)shift);
					--sp;
					temp = *(sp - shift);
					memmovedownc(sp - (shift + 1),
					             sp - shift,
					             shift + 1,
					             sizeof(DREF DeeObject *));
					*sp = temp;
					++sp;
					DISPATCH();
				}

				RAW_TARGET(ASM16_RROT) {
					DREF DeeObject *temp;
					uint32_t shift;
					shift = (uint32_t)READ_imm16() + 3;
					ASSERT_USAGE(-(int)shift, +(int)shift);
					--sp;
					temp = *sp;
					memmoveupc(sp - shift,
					           sp - (shift + 1),
					           shift + 1,
					           sizeof(DREF DeeObject *));
					*(sp - shift) = temp;
					++sp;
					DISPATCH();
				}

				RAW_TARGET(ASM16_DUP_N) {
					uint16_t offset = READ_imm16();
					DREF DeeObject **p_slot;
					ASSERT_USAGE(-((int)offset + 2), +((int)offset + 3));
					p_slot = sp - (offset + 2);
					PUSHREF(*p_slot);
					DISPATCH();
					DISPATCH();
				}

				RAW_TARGET(ASM16_POP_N) {
					DREF DeeObject *old_object;
					uint16_t offset = READ_imm16();
					DREF DeeObject **p_slot;
					ASSERT_USAGE(-((int)offset + 2), +((int)offset + 1));
					p_slot     = sp - (offset + 2);
					old_object = *p_slot;
					*p_slot    = TOP;
					Dee_Decref(old_object);
					(void)POP();
					DISPATCH();
				}

				RAW_TARGET(ASM16_ADJSTACK) {
					imm_val = (uint16_t)(int16_t)READ_Simm16();
					goto do_stack_adjust;
				}

				TARGET(ASM16_SUPER_THIS_R, -0, +1) {
					imm_val = READ_imm16();
					goto do_super_this_r;
				}

				TARGET(ASM16_POP_STATIC, -1, +0) {
					imm_val = READ_imm16();
					goto do_pop_static;
				}

				TARGET(ASM16_POP_EXTERN, -1, +0) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_pop_extern;
				}

				TARGET(ASM16_POP_GLOBAL, -1, +0) {
					imm_val = READ_imm16();
					goto do_pop_global;
				}

				TARGET(ASM16_POP_LOCAL, -1, +0) {
					imm_val = READ_imm16();
					goto do_pop_local;
				}

				TARGET(ASM16_PUSH_REF, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_ref;
				}

				TARGET(ASM16_PUSH_ARG, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_arg;
				}

				TARGET(ASM16_PUSH_CONST, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_const;
				}

				TARGET(ASM16_PUSH_STATIC, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_static;
				}

				TARGET(ASM16_PUSH_EXTERN, -0, +1) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_push_extern;
				}

				TARGET(ASM16_PUSH_GLOBAL, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_global;
				}

				TARGET(ASM16_PUSH_LOCAL, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_local;
				}

				RAW_TARGET(ASM16_PACK_TUPLE) {
					imm_val = READ_imm16();
					goto do_pack_tuple;
				}

				RAW_TARGET(ASM16_PACK_LIST) {
					imm_val = READ_imm16();
					goto do_pack_list;
				}

				RAW_TARGET(ASM16_UNPACK) {
					imm_val = READ_imm16();
					goto do_unpack;
				}

				TARGET(ASM16_PUSH_MODULE, -0, +1) {
					imm_val = READ_imm16();
					goto do_push_module;
				}

				TARGET(ASM_CLASS, -2, +1) {
					DREF DeeTypeObject *new_class;
					if (DeeObject_AssertTypeExact(TOP, &DeeClassDescriptor_Type))
						HANDLE_EXCEPT();
					new_class = DeeClass_New((DeeTypeObject *)SECOND, TOP);
					if unlikely(!new_class)
						HANDLE_EXCEPT();
					POPREF();
					Dee_Decref(TOP);
					TOP = (DREF DeeObject *)new_class; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM16_CLASS_C, -1, +1) {
					imm_val = READ_imm16();
					goto do_class_c;
				}

				TARGET(ASM16_CLASS_GC, -0, +1) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_class_gc;
				}

				TARGET(ASM16_CLASS_EC, -0, +1) {
					DREF DeeTypeObject *new_class;
					DREF DeeObject *base;
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					ASSERT_EXTERNimm();
					EXTERN_LOCKREAD();
					base = EXTERNimm;
					Dee_XIncref(base);
					EXTERN_LOCKENDREAD();
					if unlikely(!base)
						goto err_unbound_extern;
#undef EXCEPTION_CLEANUP
#define EXCEPTION_CLEANUP Dee_Decref(base);
					imm_val = READ_imm16();
					ASSERT_CONSTimm();
#undef EXCEPTION_CLEANUP
#define EXCEPTION_CLEANUP /* nothing */
#ifdef EXEC_SAFE
					{
						DeeObject *descriptor;
						CONST_LOCKREAD();
						descriptor = CONSTimm;
						Dee_Incref(descriptor);
						CONST_LOCKENDREAD();
						if unlikely(DeeObject_AssertTypeExact(descriptor, &DeeClassDescriptor_Type)) {
							new_class = NULL;
						} else {
							new_class = DeeClass_New((DeeTypeObject *)base, descriptor);
						}
						Dee_Decref(descriptor);
					}
#else /* EXEC_SAFE */
					new_class = DeeClass_New((DeeTypeObject *)base, CONSTimm);
#endif /* !EXEC_SAFE */
					Dee_Decref(base);
					if unlikely(!new_class)
						HANDLE_EXCEPT();
					PUSH((DREF DeeObject *)new_class); /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM16_DEFCMEMBER, -2, +1) {
					imm_val = READ_imm16();
					goto do_defcmember;
				}

				TARGET(ASM16_GETCMEMBER, -1, +1) {
					DREF DeeObject *member_value;
					imm_val      = READ_imm16();
					member_value = DeeClass_GetMemberSafe((DeeTypeObject *)TOP, imm_val);
					if unlikely(!member_value)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = member_value; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM16_GETCMEMBER_R, -0, +1) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_getcmember_r;
				}

				RAW_TARGET(ASM16_CALLCMEMBER_THIS_R) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_callcmember_this_r;
				}

				RAW_TARGET(ASM16_FUNCTION_C) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm8();
					goto do_function_c;
				}

				RAW_TARGET(ASM16_FUNCTION_C_16) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_function_c;
				}

				TARGET(ASM_REDUCE_MIN, -1, +1) {
					DREF DeeObject *result;
					result = DeeSeq_Min(TOP, NULL);
					if unlikely(!result)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = result; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_REDUCE_MAX, -1, +1) {
					DREF DeeObject *result;
					result = DeeSeq_Max(TOP, NULL);
					if unlikely(!result)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = result; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_REDUCE_SUM, -1, +1) {
					DREF DeeObject *result;
					result = DeeSeq_Sum(TOP);
					if unlikely(!result)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = result; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_REDUCE_ANY, -1, +1) {
					int result = DeeSeq_Any(TOP);
					if unlikely(result < 0)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = DeeBool_For(result);
					Dee_Incref(TOP);
					DISPATCH();
				}

				TARGET(ASM_REDUCE_ALL, -1, +1) {
					int result = DeeSeq_All(TOP);
					if unlikely(result < 0)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = DeeBool_For(result);
					Dee_Incref(TOP);
					DISPATCH();
				}

				RAW_TARGET(ASM16_PRINT_C) {
					imm_val = READ_imm16();
					goto do_print_c;
				}

				RAW_TARGET(ASM16_PRINT_C_SP) {
					imm_val = READ_imm16();
					goto do_print_c_sp;
				}

				RAW_TARGET(ASM16_PRINT_C_NL) {
					imm_val = READ_imm16();
					goto do_print_c_nl;
				}

				RAW_TARGET(ASM16_FPRINT_C) {
					imm_val = READ_imm16();
					goto do_fprint_c;
				}

				RAW_TARGET(ASM16_FPRINT_C_SP) {
					imm_val = READ_imm16();
					goto do_fprint_c_sp;
				}

				RAW_TARGET(ASM16_FPRINT_C_NL) {
					imm_val = READ_imm16();
					goto do_fprint_c_nl;
				}

				RAW_TARGET(ASM16_CONTAINS_C) {
					imm_val = READ_imm16();
					goto do_contains_c;
				}

				RAW_TARGET(ASM16_GETITEM_C) {
					imm_val = READ_imm16();
					goto do_getitem_c;
				}

				RAW_TARGET(ASM16_SETITEM_C) {
					imm_val = READ_imm16();
					goto do_setitem_c;
				}

				RAW_TARGET(ASM_RANGE_0_I32) {
					DREF DeeObject *range_object;
					uint32_t range_end = READ_imm32();
					ASSERT_USAGE(-0, +1);
#if __SIZEOF_SIZE_T__ <= 4
					if (range_end > SSIZE_MAX) {
						/* Special case: We must create an unsigned range, but
						 *               the given target length doesn't fit.
						 *               Therefor, we must create an object-style
						 *               range in order to prevent the potential 
						 *               overflow. */
						DREF DeeObject *ob_end = DeeInt_NewUInt32(range_end);
						if unlikely(!ob_end)
							HANDLE_EXCEPT();
#if 1 /* Both versions will result in the same behavior, but this        \
       * one is faster in the current implementation because `int'       \
       * actually doesn't implement inplace operations, meaning          \
       * that `tp_inc()' would otherwise have to be substitued with      \
       * a call to `tp_add()' using `DeeInt_One' during every iteration. \
       * By passing `DeeInt_One' now, we can skip those checks later! */
						range_object = DeeRange_New(DeeInt_Zero, ob_end, DeeInt_One);
#else
						range_object = DeeRange_New(DeeInt_Zero, ob_end, NULL);
#endif
						Dee_Decref(ob_end);
					} else
#endif /* __SIZEOF_SIZE_T__ <= 4 */
					{
						range_object = DeeRange_NewInt(0, (dssize_t)range_end, 1);
					}
					if unlikely(!range_object)
						HANDLE_EXCEPT();
					PUSH(range_object);
					DISPATCH();
				}


				RAW_TARGET(ASM16_CALLATTR_C_KW) {
					uint8_t argc;
					DREF DeeObject *call_result;
					DeeObject **new_sp;
					imm_val  = READ_imm16();
					argc     = READ_imm8();
					imm_val2 = READ_imm16();
					ASSERT_USAGE(-((int)argc + 1), +1);
					ASSERT_CONSTimm();
					ASSERT_CONSTimm2();
					new_sp = sp - argc;
#ifdef EXEC_SAFE
					{
						DREF DeeObject *attr_name;
						DREF DeeObject *kwds_map;
						CONST_LOCKREAD();
						attr_name = CONSTimm;
						kwds_map  = CONSTimm2;
						Dee_Incref(attr_name);
						Dee_Incref(kwds_map);
						CONST_LOCKENDREAD();
						if unlikely(!DeeString_CheckExact(attr_name)) {
							Dee_Decref_unlikely(attr_name);
							Dee_Decref_unlikely(kwds_map);
							goto err_requires_string;
						}
						call_result = DeeObject_CallAttrKw(new_sp[-1],
						                                   CONSTimm,
						                                   argc,
						                                   new_sp,
						                                   CONSTimm2);
						Dee_Decref_unlikely(attr_name);
						Dee_Decref_unlikely(kwds_map);
					}
#else /* EXEC_SAFE */
					ASSERT_STRING(CONSTimm);
					call_result = DeeObject_CallAttrKw(new_sp[-1],
					                                   CONSTimm,
					                                   argc,
					                                   new_sp,
					                                   CONSTimm2);
#endif /* !EXEC_SAFE */
					if unlikely(!call_result)
						HANDLE_EXCEPT();
					while (sp > new_sp)
						POPREF();
					Dee_Decref(TOP);
					TOP = call_result; /* Inherit reference. */
					DISPATCH();
				}

				RAW_TARGET(ASM16_CALLATTR_C_TUPLE_KW) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_callattr_tuple_c_kw;
				}

				RAW_TARGET(ASM_CALLATTR_KWDS) {
					DeeObject **new_sp;
					DREF DeeObject *call_result;
					imm_val = READ_imm8();
					ASSERT_USAGE(-(int)(imm_val + 3), +1);
					new_sp = sp - (imm_val + 1);
					if unlikely(!DeeString_Check(new_sp[-1])) {
						err_expected_string_for_attribute(new_sp[-1]);
						HANDLE_EXCEPT();
					}
					call_result = DeeObject_CallAttrKw(new_sp[-2],
					                                   new_sp[-1],
					                                   imm_val,
					                                   new_sp,
					                                   TOP);
					if unlikely(!call_result)
						HANDLE_EXCEPT();
					while (sp > new_sp)
						POPREF();
					POPREF(); /* name */
					Dee_Decref(TOP);
					TOP = call_result;
					DISPATCH();
				}

				RAW_TARGET(ASM_CALLATTR_TUPLE_KWDS) {
					DREF DeeObject *call_result;
					ASSERT_USAGE(-4, +1);
					if unlikely(!DeeString_Check(THIRD)) {
						err_expected_string_for_attribute(THIRD);
						HANDLE_EXCEPT();
					}
					ASSERT_TUPLE(SECOND);
					call_result = DeeObject_CallAttrTupleKw(FOURTH, THIRD, SECOND, FIRST);
					POPREF(); /* kwds */
					POPREF(); /* args */
					POPREF(); /* name */
					Dee_Decref(TOP);
					TOP = call_result;
					DISPATCH();
				}

				RAW_TARGET(ASM16_CALLATTR_C) {
					imm_val = READ_imm16();
					goto do_callattr_c;
				}

				TARGET(ASM16_CALLATTR_C_TUPLE, -2, +1) {
					imm_val = READ_imm16();
					goto do_callattr_tuple_c;
				}

				RAW_TARGET(ASM16_CALLATTR_THIS_C) {
					imm_val = READ_imm16();
					goto do_callattr_this_c;
				}

				TARGET(ASM16_CALLATTR_THIS_C_TUPLE, -1, +1) {
					imm_val = READ_imm16();
					goto do_callattr_this_tuple_c;
				}

				RAW_TARGET(ASM16_CALLATTR_C_SEQ) {
					imm_val = READ_imm16();
					goto do_callattr_c_seq;
				}

				RAW_TARGET(ASM16_CALLATTR_C_MAP) {
					imm_val = READ_imm16();
					goto do_callattr_c_map;
				}

				TARGET(ASM_GETMEMBER, -2, +1) {
					DREF DeeObject *result;
					imm_val = READ_imm8();
do_getmember:
					result = DeeInstance_GetMemberSafe((DeeTypeObject *)FIRST, SECOND, imm_val);
					if unlikely(!result)
						HANDLE_EXCEPT();
					POPREF();
					Dee_Decref(TOP);
					TOP = result; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_BOUNDMEMBER, -2, +1) {
					int temp;
					imm_val = READ_imm8();
do_hasmember:
					temp = DeeInstance_BoundMemberSafe((DeeTypeObject *)FIRST, SECOND, imm_val);
					if unlikely(temp < 0)
						HANDLE_EXCEPT();
					POPREF();
					Dee_Decref(TOP);
					TOP = DeeBool_For(temp);
					Dee_Incref(TOP);
					DISPATCH();
				}

				TARGET(ASM_DELMEMBER, -2, +0) {
					imm_val = READ_imm8();
do_delmember:
					if (DeeInstance_DelMemberSafe((DeeTypeObject *)FIRST, SECOND, imm_val))
						HANDLE_EXCEPT();
					POPREF();
					POPREF();
					DISPATCH();
				}

				TARGET(ASM_SETMEMBER, -3, +0) {
					imm_val = READ_imm8();
do_setmember:
					if (DeeInstance_SetMemberSafe((DeeTypeObject *)SECOND, THIRD, imm_val, FIRST))
						HANDLE_EXCEPT();
					POPREF();
					POPREF();
					POPREF();
					DISPATCH();
				}

				TARGET(ASM16_GETMEMBER, -2, +1) {
					imm_val = READ_imm16();
					goto do_getmember;
				}

				TARGET(ASM16_BOUNDMEMBER, -2, +1) {
					imm_val = READ_imm16();
					goto do_hasmember;
				}

				TARGET(ASM16_DELMEMBER, -2, +0) {
					imm_val = READ_imm16();
					goto do_delmember;
				}

				TARGET(ASM16_SETMEMBER, -3, +0) {
					imm_val = READ_imm16();
					goto do_setmember;
				}

				TARGET(ASM_GETMEMBER_THIS, -1, +1) {
					DREF DeeObject *result;
					imm_val = READ_imm8();
do_getmember_this:
					ASSERT_THISCALL();
					result = DeeInstance_GetMemberSafe((DeeTypeObject *)TOP, THIS, imm_val);
					if unlikely(!result)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = result; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_BOUNDMEMBER_THIS, -1, +1) {
					int temp;
					imm_val = READ_imm8();
do_hasmember_this:
					ASSERT_THISCALL();
					temp = DeeInstance_BoundMemberSafe((DeeTypeObject *)TOP, THIS, imm_val);
					if unlikely(temp < 0)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = DeeBool_For(temp);
					Dee_Incref(TOP);
					DISPATCH();
				}

				TARGET(ASM_DELMEMBER_THIS, -1, +0) {
					imm_val = READ_imm8();
do_delmember_this:
					ASSERT_THISCALL();
					if (DeeInstance_DelMemberSafe((DeeTypeObject *)TOP, THIS, imm_val))
						HANDLE_EXCEPT();
					POPREF();
					DISPATCH();
				}

				TARGET(ASM_SETMEMBER_THIS, -2, +0) {
					imm_val = READ_imm8();
do_setmember_this:
					ASSERT_THISCALL();
					if (DeeInstance_SetMemberSafe((DeeTypeObject *)SECOND, THIS, imm_val, FIRST))
						HANDLE_EXCEPT();
					POPREF();
					POPREF();
					DISPATCH();
				}

				TARGET(ASM16_GETMEMBER_THIS, -1, +1) {
					imm_val = READ_imm16();
					goto do_getmember_this;
				}

				TARGET(ASM16_BOUNDMEMBER_THIS, -1, +1) {
					imm_val = READ_imm16();
					goto do_hasmember_this;
				}

				TARGET(ASM16_DELMEMBER_THIS, -1, +0) {
					imm_val = READ_imm16();
					goto do_delmember_this;
				}

				TARGET(ASM16_SETMEMBER_THIS, -2, +0) {
					imm_val = READ_imm16();
					goto do_setmember_this;
				}

				TARGET(ASM16_GETMEMBER_THIS_R, -0, +1) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_getmember_r;
				}

				TARGET(ASM16_DELMEMBER_THIS_R, -0, +0) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_delmember_r;
				}

				TARGET(ASM16_SETMEMBER_THIS_R, -1, +0) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_setmember_r;
				}

				TARGET(ASM16_BOUNDMEMBER_THIS_R, -1, +0) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_boundmember_r;
				}

				RAW_TARGET(ASM16_CALL_EXTERN) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_call_extern;
				}

				RAW_TARGET(ASM16_CALL_GLOBAL) {
					imm_val = READ_imm16();
					goto do_call_global;
				}

				RAW_TARGET(ASM16_CALL_LOCAL) {
					imm_val = READ_imm16();
					goto do_call_local;
				}

				TARGET(ASM_BOUNDITEM, -2, +1) {
					int error;
					error = DeeObject_BoundItem(SECOND, FIRST, true);
					if unlikely(error == -1)
						HANDLE_EXCEPT();
					POPREF();
					Dee_Decref(TOP);
					TOP = DeeBool_For(error > 0);
					Dee_Incref(TOP);
					DISPATCH();
				}

				TARGET(ASM16_GETATTR_C, -1, +1) {
					imm_val = READ_imm16();
					goto do_getattr_c;
				}

				TARGET(ASM16_DELATTR_C, -1, +0) {
					imm_val = READ_imm16();
					goto do_delattr_c;
				}

				TARGET(ASM16_SETATTR_C, -2, +0) {
					imm_val = READ_imm16();
					goto do_setattr_c;
				}

				TARGET(ASM16_GETATTR_THIS_C, -0, +1) {
					imm_val = READ_imm16();
					goto do_getattr_this_c;
				}

				TARGET(ASM16_DELATTR_THIS_C, -0, +0) {
					imm_val = READ_imm16();
					goto do_delattr_this_c;
				}

				TARGET(ASM16_SETATTR_THIS_C, -1, +0) {
					imm_val = READ_imm16();
					goto do_setattr_this_c;
				}


				{
					DREF DeeObject *hashset_object;
				RAW_TARGET(ASM16_PACK_HASHSET)
					imm_val = READ_imm16();
					goto do_pack_set;
				RAW_TARGET(ASM_PACK_HASHSET)
					imm_val = READ_imm8();
do_pack_set:
					hashset_object = DeeHashSet_NewItemsInherited(imm_val, sp - imm_val);
					if unlikely(!hashset_object)
						HANDLE_EXCEPT();
					sp -= imm_val;
					PUSH(hashset_object);
					DISPATCH();
				}

				{
					DREF DeeObject *dict_object;
				RAW_TARGET(ASM16_PACK_DICT)
					imm_val = READ_imm16();
					goto do_pack_dict;
				RAW_TARGET(ASM_PACK_DICT)
					imm_val = READ_imm8();
do_pack_dict:
					ASSERT_USAGE(-(int)(imm_val * 2), +1);
					dict_object = DeeDict_NewKeyItemsInherited(imm_val, sp - (imm_val * 2));
					if unlikely(!dict_object)
						HANDLE_EXCEPT();
					sp -= (imm_val * 2); /* Adjust SP to pop items. */
					PUSH(dict_object);   /* Inherit reference. */
					DISPATCH();
				}

				RAW_TARGET(ASM_CAST_DICT) {
					DREF DeeObject *dict_cast;
					ASSERT_USAGE(-1, +1);
					dict_cast = DeeDict_FromSequence(TOP);
					if unlikely(!dict_cast)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = dict_cast; /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_CAST_HASHSET, -1, +1) {
					DREF DeeObject *set_cast;
					set_cast = DeeHashSet_FromSequence(TOP);
					if unlikely(!set_cast)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = set_cast; /* Inherit reference */
					DISPATCH();
				}

				RAW_TARGET(ASM_ITERNEXT) {
					DREF DeeObject *iter_res;
					ASSERT_USAGE(-1, +1);
					iter_res = DeeObject_IterNext(TOP);
					if unlikely(!iter_res)
						HANDLE_EXCEPT();
					if (iter_res == ITER_DONE) {
						/* Throw the stop-iterator error object. */
						DeeError_Throw(&DeeError_StopIteration_instance);
						HANDLE_EXCEPT();
					}
					Dee_Decref(TOP);
					TOP = iter_res; /* Inherit reference. */
					DISPATCH();
				}

				RAW_TARGET(ASM16_EXTERN)
					ip.ptr += 4;
					goto do_prefix_instr;

				RAW_TARGET(ASM16_STACK)
				RAW_TARGET(ASM16_STATIC)
				RAW_TARGET(ASM16_GLOBAL)
				RAW_TARGET(ASM16_LOCAL)
					ip.ptr += 2;
					goto do_prefix_instr;

				RAW_TARGET(ASM_VARARGS_UNPACK) {
					size_t va_size;
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARARGS))
						goto err_requires_varargs_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
					imm_val = READ_imm8();
					ASSERT_USAGE(-0, +imm_val);
					va_size = likely(frame->cf_argc > code->co_argc_max)
					          ? frame->cf_argc - code->co_argc_max
					          : 0;
					if (imm_val != va_size) {
						err_invalid_va_unpack_size(imm_val, va_size);
						HANDLE_EXCEPT();
					}
					for (va_size = 0; va_size < imm_val; ++va_size)
						PUSHREF(frame->cf_argv[code->co_argc_max + va_size]);
					DISPATCH();
				}

				TARGET(ASM_PUSH_VARKWDS_NE, -0, +1) {
					DeeObject *value = Dee_False;
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARKWDS))
						goto err_requires_varkwds_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARKWDS);
#endif /*  !EXEC_SAFE */
					if (frame->cf_kw) {
						int temp;
						value = frame->cf_kw->fk_varkwds;
						if (!value) {
							DeeObject *oldval;
							value = construct_varkwds_mapping();
							if unlikely(!value)
								HANDLE_EXCEPT();
							oldval = atomic_cmpxch_val(&frame->cf_kw->fk_varkwds, NULL, value);
							if unlikely(oldval) {
								VARKWDS_DECREF(value);
								value = oldval;
							}
						}
						temp = DeeObject_Bool(value);
						if unlikely(temp < 0)
							HANDLE_EXCEPT();
						value = DeeBool_For(temp);
					}
					PUSHREF(value);
					DISPATCH();
				}

				/* Variable argument API. */
				TARGET(ASM_VARARGS_GETSIZE, -0, +1) {
					DREF DeeObject *varsize;
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARARGS))
						goto err_requires_varargs_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
					if (frame->cf_argc <= code->co_argc_max) {
						varsize = DeeInt_Zero;
						Dee_Incref(varsize);
					} else {
						varsize = DeeInt_NewSize((size_t)(frame->cf_argc - code->co_argc_max));
						if unlikely(!varsize)
							HANDLE_EXCEPT();
					}
					PUSH(varsize); /* Inherit reference. */
					DISPATCH();
				}

				TARGET(ASM_VARARGS_CMP_EQ_SZ, -0, +1) {
					size_t va_size;
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARARGS))
						goto err_requires_varargs_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
					if (frame->cf_argc <= code->co_argc_max) {
						va_size = 0;
					} else {
						va_size = (size_t)(frame->cf_argc - code->co_argc_max);
					}
					PUSHREF(DeeBool_For(va_size == READ_imm8()));
					DISPATCH();
				}

				TARGET(ASM_VARARGS_CMP_GR_SZ, -0, +1) {
					size_t va_size;
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARARGS))
						goto err_requires_varargs_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
					if (frame->cf_argc <= code->co_argc_max) {
						va_size = 0;
					} else {
						va_size = (size_t)(frame->cf_argc - code->co_argc_max);
					}
					PUSHREF(DeeBool_For(va_size > READ_imm8()));
					DISPATCH();
				}


				TARGET(ASM_VARARGS_GETITEM, -1, +1) {
					size_t index;
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARARGS))
						goto err_requires_varargs_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
					if (DeeObject_AsSize(TOP, &index))
						HANDLE_EXCEPT();
					if (OVERFLOW_UADD(index, code->co_argc_max, &index) ||
					    index >= frame->cf_argc) {
						err_va_index_out_of_bounds((size_t)(index - code->co_argc_max),
						                           (size_t)(frame->cf_argc - code->co_argc_max));
						HANDLE_EXCEPT();
					}
					/* Exchange the stack-top object */
					Dee_Decref(TOP);
					TOP = frame->cf_argv[index];
					Dee_Incref(TOP);
					DISPATCH();
				}

				TARGET(ASM_VARARGS_GETITEM_I, -0, +1) {
					size_t index;
					DeeObject *argobj;
					index = READ_imm8();
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARARGS))
						goto err_requires_varargs_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
					index += code->co_argc_max;
					if (index >= frame->cf_argc) {
						size_t va_size = 0;
						if (frame->cf_argc > code->co_argc_max)
							va_size = (size_t)(frame->cf_argc - code->co_argc_max);
						err_va_index_out_of_bounds((size_t)(index - code->co_argc_max), va_size);
						HANDLE_EXCEPT();
					}
					/* Exchange the stack-top object */
					argobj = frame->cf_argv[index];
					PUSHREF(argobj);
					DISPATCH();
				}

				RAW_TARGET(ASM_SUPERGETATTR_THIS_RC) {
					DREF DeeObject *attr_value;
					imm_val  = READ_imm8();
					imm_val2 = READ_imm8();
do_supergetattr_rc:
					ASSERT_USAGE(-0, +1);
					ASSERT_REFimm();
					ASSERT_CONSTimm2();
					ASSERT_THISCALL();
					if (DeeObject_AssertType(THIS, (DeeTypeObject *)REFimm))
						HANDLE_EXCEPT();
#ifdef EXEC_SAFE
					{
						DREF DeeObject *attr_name;
						CONST_LOCKREAD();
						attr_name = CONSTimm2;
						Dee_Incref(attr_name);
						CONST_LOCKENDREAD();
						if (!DeeString_Check(attr_name)) {
							err_expected_string_for_attribute(attr_name);
							Dee_Decref(attr_name);
							HANDLE_EXCEPT();
						}
						attr_value = DeeObject_TGetAttr((DeeTypeObject *)REFimm, THIS, attr_name);
						Dee_Decref(attr_name);
					}
#else /* EXEC_SAFE */
					attr_value = DeeObject_TGetAttr((DeeTypeObject *)REFimm, THIS, CONSTimm2);
#endif /* !EXEC_SAFE */
					if unlikely(!attr_value)
						HANDLE_EXCEPT();
					PUSH(attr_value);
					DISPATCH();
				}

				RAW_TARGET(ASM_SUPERCALLATTR_THIS_RC) {
					DREF DeeObject *callback_result;
					uint8_t argc;
					imm_val  = READ_imm8();
					imm_val2 = READ_imm8();
do_supercallattr_rc:
					ASSERT_REFimm();
					ASSERT_CONSTimm2();
					ASSERT_THISCALL();
					argc = READ_imm8();
					ASSERT_USAGE(-(int)argc, +1);
					if (DeeObject_AssertType(THIS, (DeeTypeObject *)REFimm))
						HANDLE_EXCEPT();
#ifdef EXEC_SAFE
					{
						DREF DeeObject *attr_name;
						CONST_LOCKREAD();
						attr_name = CONSTimm2;
						Dee_Incref(attr_name);
						CONST_LOCKENDREAD();
						if (!DeeString_Check(attr_name)) {
							err_expected_string_for_attribute(attr_name);
							Dee_Decref(attr_name);
							HANDLE_EXCEPT();
						}
						callback_result = DeeObject_TCallAttr((DeeTypeObject *)REFimm, THIS, attr_name, argc, sp - argc);
						Dee_Decref(attr_name);
					}
#else /* EXEC_SAFE */
					callback_result = DeeObject_TCallAttr((DeeTypeObject *)REFimm, THIS, CONSTimm2, argc, sp - argc);
#endif /* !EXEC_SAFE */
					if unlikely(!callback_result)
						HANDLE_EXCEPT();
					while (argc--)
						POPREF();
					PUSH(callback_result);
					DISPATCH();
				}

				RAW_TARGET(ASM16_SUPERGETATTR_THIS_RC) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_supergetattr_rc;
				}

				RAW_TARGET(ASM16_SUPERCALLATTR_THIS_RC) {
					imm_val  = READ_imm16();
					imm_val2 = READ_imm16();
					goto do_supercallattr_rc;
				}

#ifdef USE_SWITCH
			default:
				goto unknown_instruction;
#endif /* USE_SWITCH */
			}
			__builtin_unreachable();
		}


		{ /* Prefixed instruction handling. */
		RAW_TARGET(ASM_EXTERN)
			ip.ptr += 2;
			goto do_prefix_instr;
		RAW_TARGET(ASM_STACK)
		RAW_TARGET(ASM_STATIC)
		RAW_TARGET(ASM_GLOBAL)
		RAW_TARGET(ASM_LOCAL)
			++ip.ptr;
do_prefix_instr:
			/* Execute a prefixed instruction. */
			switch (*ip.ptr++) {
#define PREFIX_TARGET(opcode) case (opcode)&0xff:

				PREFIX_TARGET(ASM_JF16) {
					imm_val = (uint16_t)READ_Simm16();
					goto prefix_jf_16;
				}

				PREFIX_TARGET(ASM_JF) {
					/* Conditionally jump if true. */
					USING_PREFIX_OBJECT
					int temp;
					imm_val = (uint16_t)(int16_t)READ_Simm8();
prefix_jf_16:
					prefix_ob = get_prefix_object();
					if unlikely(!prefix_ob)
						HANDLE_EXCEPT();
					temp = DeeObject_Bool(prefix_ob);
					Dee_Decref(prefix_ob);
					if unlikely(temp < 0)
						HANDLE_EXCEPT();
					if (!temp) {
						if ((int16_t)imm_val < 0) {
							if (DeeThread_CheckInterruptSelf(this_thread))
								HANDLE_EXCEPT();
						}
						ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
						goto assert_ip_bounds;
#else /* EXEC_SAFE */
						ASSERT(ip.ptr >= code->co_code);
						ASSERT(ip.ptr < code->co_code + code->co_codebytes);
#endif /* !EXEC_SAFE */
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_JT16) {
					imm_val = (uint16_t)READ_Simm16();
					goto prefix_jt_16;
				}

				PREFIX_TARGET(ASM_JT) {
					/* Conditionally jump if true. */
					USING_PREFIX_OBJECT
					int temp;
					imm_val = (uint16_t)(int16_t)READ_Simm8();
prefix_jt_16:
					prefix_ob = get_prefix_object();
					if unlikely(!prefix_ob)
						HANDLE_EXCEPT();
					temp = DeeObject_Bool(prefix_ob);
					Dee_Decref(prefix_ob);
					if unlikely(temp < 0)
						HANDLE_EXCEPT();
					if (temp) {
						if ((int16_t)imm_val < 0) {
							if (DeeThread_CheckInterruptSelf(this_thread))
								HANDLE_EXCEPT();
						}
						ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
						goto assert_ip_bounds;
#else /* EXEC_SAFE */
						ASSERT(ip.ptr >= code->co_code);
						ASSERT(ip.ptr < code->co_code + code->co_codebytes);
#endif /* !EXEC_SAFE */
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_FOREACH16) {
					imm_val = (uint16_t)READ_Simm16();
					goto prefix_foreach_16;
				}

				PREFIX_TARGET(ASM_FOREACH) {
					DREF DeeObject *elem;
					USING_PREFIX_OBJECT
					imm_val = (uint16_t)(int16_t)READ_Simm8();
prefix_foreach_16:
					ASSERT_USAGE(-0, +1);
					prefix_ob = get_prefix_object();
					if unlikely(!prefix_ob)
						HANDLE_EXCEPT();
					elem = DeeObject_IterNext(prefix_ob);
					Dee_Decref(prefix_ob);
					if unlikely(!elem)
						HANDLE_EXCEPT();
					if (elem == ITER_DONE)
						goto jump_16;
					/* Push the element. */
					PUSH(elem);
					DISPATCH();
				}

#define DEFINE_INPLACE_MATH_OPERATOR(ADD, Add)                               \
				PREFIX_TARGET(ASM_##ADD) {                                   \
					int error;                                               \
					DREF DeeObject **prefix_pointer;                         \
					ASSERT_USAGE(-1, +0);                                    \
					prefix_pointer = get_prefix_object_ptr();                \
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {    \
						if unlikely(!prefix_pointer)                         \
							HANDLE_EXCEPT();                                 \
						error = DeeObject_Inplace##Add(prefix_pointer, TOP); \
						if unlikely(error)                                   \
							HANDLE_EXCEPT();                                 \
					} else {                                                 \
						USING_PREFIX_OBJECT                                  \
						prefix_ob = get_prefix_object();                     \
						if unlikely(!prefix_ob)                              \
							HANDLE_EXCEPT();                                 \
						error = DeeObject_Inplace##Add(&prefix_ob, TOP);     \
						if unlikely(error) {                                 \
							Dee_Decref(prefix_ob);                           \
							HANDLE_EXCEPT();                                 \
						}                                                    \
						if unlikely(set_prefix_object(prefix_ob))            \
							HANDLE_EXCEPT();                                 \
					}                                                        \
					POPREF();                                                \
					DISPATCH();                                              \
				}
				DEFINE_INPLACE_MATH_OPERATOR(ADD, Add)
				DEFINE_INPLACE_MATH_OPERATOR(SUB, Sub)
				DEFINE_INPLACE_MATH_OPERATOR(MUL, Mul)
				DEFINE_INPLACE_MATH_OPERATOR(DIV, Div)
				DEFINE_INPLACE_MATH_OPERATOR(MOD, Mod)
				DEFINE_INPLACE_MATH_OPERATOR(SHL, Shl)
				DEFINE_INPLACE_MATH_OPERATOR(SHR, Shr)
				DEFINE_INPLACE_MATH_OPERATOR(AND, And)
				DEFINE_INPLACE_MATH_OPERATOR(OR, Or)
				DEFINE_INPLACE_MATH_OPERATOR(XOR, Xor)
				DEFINE_INPLACE_MATH_OPERATOR(POW, Pow)
#undef DEFINE_INPLACE_MATH_OPERATOR

				PREFIX_TARGET(ASM_ADD_SIMM8) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceAddS8(prefix_pointer, READ_Simm8());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceAddS8(&prefix_ob, READ_Simm8());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_ADD_IMM32) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceAddInt(prefix_pointer, READ_imm32());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceAddInt(&prefix_ob, READ_imm32());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_SUB_SIMM8) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceSubS8(prefix_pointer, READ_Simm8());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceSubS8(&prefix_ob, READ_Simm8());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_SUB_IMM32) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceSubInt(prefix_pointer, READ_imm32());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceSubInt(&prefix_ob, READ_imm32());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_MUL_SIMM8) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceMulInt(prefix_pointer, READ_Simm8());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceMulInt(&prefix_ob, READ_Simm8());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_DIV_SIMM8) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceDivInt(prefix_pointer, READ_Simm8());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceDivInt(&prefix_ob, READ_Simm8());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_MOD_SIMM8) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceModInt(prefix_pointer, READ_Simm8());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceModInt(&prefix_ob, READ_Simm8());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_AND_IMM32) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceAndInt(prefix_pointer, READ_imm32());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceAndInt(&prefix_ob, READ_imm32());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_OR_IMM32) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceOrInt(prefix_pointer, READ_imm32());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceOrInt(&prefix_ob, READ_imm32());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_XOR_IMM32) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceXorInt(prefix_pointer, READ_imm32());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceXorInt(&prefix_ob, READ_imm32());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_SHL_IMM8) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceShlInt(prefix_pointer, READ_imm8());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceShlInt(&prefix_ob, READ_imm8());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_SHR_IMM8) {
					int error;
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceShrInt(prefix_pointer, READ_imm8());
						if unlikely(error)
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						error = DeeObject_InplaceShrInt(&prefix_ob, READ_imm8());
						if unlikely(error) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}


				PREFIX_TARGET(ASM_FUNCTION_C_16)
				imm_val  = READ_imm8();
				imm_val2 = READ_imm16();
				goto prefix_do_function_c;
				PREFIX_TARGET(ASM_FUNCTION_C) {
					DREF DeeObject *function;
					imm_val  = READ_imm8();
					imm_val2 = READ_imm8();
prefix_do_function_c:
					ASSERT_USAGE(-(int)(imm_val2 + 1), +0);
					ASSERT_CONSTimm();
#ifdef EXEC_SAFE
					{
						DREF DeeObject *code_object;
						CONST_LOCKREAD();
						code_object = CONSTimm;
						Dee_Incref(code_object);
						CONST_LOCKENDREAD();
						if (DeeObject_AssertTypeExact(code_object, &DeeCode_Type)) {
							Dee_Decref(code_object);
							HANDLE_EXCEPT();
						}
						if (((DeeCodeObject *)code_object)->co_refc != imm_val2 + 1) {
							err_invalid_refs_size(code_object, imm_val2 + 1);
							Dee_Decref(code_object);
							HANDLE_EXCEPT();
						}
						function = DeeFunction_NewInherited(code_object,
						                                    imm_val2 + 1,
						                                    sp - (imm_val2 + 1));
						Dee_Decref(code_object);
					}
#else /* EXEC_SAFE */
					function = DeeFunction_NewInherited(CONSTimm,
					                                    imm_val2 + 1,
					                                    sp - (imm_val2 + 1));
#endif /* !EXEC_SAFE */
					if unlikely(!function)
						HANDLE_EXCEPT();
					sp -= imm_val2 + 1;
					if unlikely(set_prefix_object(function))
						HANDLE_EXCEPT();
					DISPATCH();
				}


				PREFIX_TARGET(ASM_INC) {
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						if unlikely(DeeObject_Inc(prefix_pointer))
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						if unlikely(DeeObject_Inc(&prefix_ob)) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_DEC) {
					DREF DeeObject **prefix_pointer;
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						if unlikely(DeeObject_Dec(prefix_pointer))
							HANDLE_EXCEPT();
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						if unlikely(DeeObject_Dec(&prefix_ob)) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_OPERATOR) {
					DREF DeeObject *call_result;
					DREF DeeObject **prefix_pointer;
					imm_val  = READ_imm8();
					imm_val2 = READ_imm8();
do_prefix_operator:
					ASSERT_USAGE(-(int)imm_val2, +1);
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						call_result = DeeObject_PInvokeOperator(prefix_pointer, imm_val,
						                                        imm_val2, sp - imm_val2);
						if unlikely(!call_result)
							HANDLE_EXCEPT();
						while (imm_val2--)
							POPREF();
						PUSH(call_result);
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						call_result = DeeObject_PInvokeOperator(&prefix_ob, imm_val,
						                                        imm_val2, sp - imm_val2);
						if unlikely(!call_result) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						while (imm_val2--)
							POPREF();
						PUSH(call_result);
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_OPERATOR_TUPLE) {
					DREF DeeObject *call_result;
					DREF DeeObject **prefix_pointer;
					imm_val = READ_imm8();
do_prefix_operator_tuple:
					ASSERT_USAGE(-1, +1);
					ASSERT_TUPLE(TOP);
					prefix_pointer = get_prefix_object_ptr();
					if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
						if unlikely(!prefix_pointer)
							HANDLE_EXCEPT();
						call_result = DeeObject_PInvokeOperator(prefix_pointer,
						                                        imm_val,
						                                        DeeTuple_SIZE(TOP),
						                                        DeeTuple_ELEM(TOP));
						if unlikely(!call_result)
							HANDLE_EXCEPT();
						Dee_Decref(TOP);
						TOP = call_result; /* Inherit reference. */
					} else {
						USING_PREFIX_OBJECT
						prefix_ob = get_prefix_object();
						if unlikely(!prefix_ob)
							HANDLE_EXCEPT();
						call_result = DeeObject_PInvokeOperator(&prefix_ob,
						                                        imm_val,
						                                        DeeTuple_SIZE(TOP),
						                                        DeeTuple_ELEM(TOP));
						if unlikely(!call_result) {
							Dee_Decref(prefix_ob);
							HANDLE_EXCEPT();
						}
						Dee_Decref(TOP);
						TOP = call_result; /* Inherit reference. */
						if unlikely(set_prefix_object(prefix_ob))
							HANDLE_EXCEPT();
					}
					DISPATCH();
				}

				PREFIX_TARGET(ASM_UNPACK) {
					int error;
					DREF DeeObject *sequence;
					imm_val = READ_imm8();
prefix_do_unpack:
					ASSERT_USAGE(-0, +(int)imm_val);
					sequence = get_prefix_object();
					if unlikely(!sequence)
						HANDLE_EXCEPT();
					error = DeeObject_Unpack(sequence, imm_val, sp);
					Dee_Decref(sequence);
					if unlikely(error)
						HANDLE_EXCEPT();
					sp += imm_val;
					DISPATCH();
				}

				PREFIX_TARGET(ASM_EXTENDED1) {
					switch (*ip.ptr++) {

						PREFIX_TARGET(ASM16_NOP)
						PREFIX_TARGET(ASM16_DELOP) {
							DISPATCH();
						}

						PREFIX_TARGET(ASM16_OPERATOR) {
							imm_val  = READ_imm16();
							imm_val2 = READ_imm8();
							goto do_prefix_operator;
						}

						PREFIX_TARGET(ASM16_OPERATOR_TUPLE) {
							imm_val = READ_imm16();
							goto do_prefix_operator_tuple;
						}

						PREFIX_TARGET(ASM16_FUNCTION_C) {
							imm_val  = READ_imm16();
							imm_val2 = READ_imm8();
							goto prefix_do_function_c;
						}

						PREFIX_TARGET(ASM16_FUNCTION_C_16) {
							imm_val  = READ_imm16();
							imm_val2 = READ_imm16();
							goto prefix_do_function_c;
						}

						PREFIX_TARGET(ASM16_UNPACK) {
							imm_val = READ_imm16();
							goto prefix_do_unpack;
						}

						PREFIX_TARGET(ASM_INCPOST) { /* incpost */
							DREF DeeObject **prefix_pointer;
							DREF DeeObject *obcopy;
							ASSERT_USAGE(-0, +1);
							prefix_pointer = get_prefix_object_ptr();
							if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
								if unlikely(!prefix_pointer)
									HANDLE_EXCEPT();
								obcopy = DeeObject_Copy(*prefix_pointer);
								if unlikely(!obcopy)
									HANDLE_EXCEPT();
								PUSH(obcopy); /* Inherit reference. */
								if unlikely(!*prefix_pointer) {
#ifdef NDEBUG
									get_prefix_object_ptr();
#else /* NDEBUG */
									prefix_pointer = get_prefix_object_ptr();
									ASSERT(!prefix_pointer);
#endif /* !NDEBUG */
									HANDLE_EXCEPT();
								}
								if (DeeObject_Inc(prefix_pointer))
									HANDLE_EXCEPT();
							} else {
								USING_PREFIX_OBJECT
								prefix_ob = get_prefix_object();
								if unlikely(!prefix_ob)
									HANDLE_EXCEPT();
								obcopy = DeeObject_Copy(prefix_ob);
								if unlikely(!obcopy) {
									Dee_Decref(prefix_ob);
									HANDLE_EXCEPT();
								}
								PUSH(obcopy); /* Inherit reference. */
								if (DeeObject_Inc(&prefix_ob)) {
									Dee_Decref(prefix_ob);
									HANDLE_EXCEPT();
								}
								if unlikely(set_prefix_object(prefix_ob))
									HANDLE_EXCEPT();
							}
							DISPATCH();
						}


						PREFIX_TARGET(ASM_DECPOST) { /* decpost */
							DREF DeeObject **prefix_pointer;
							DREF DeeObject *obcopy;
							ASSERT_USAGE(-0, +1);
							prefix_pointer = get_prefix_object_ptr();
							if (prefix_pointer != (DREF DeeObject **)ITER_DONE) {
								if unlikely(!prefix_pointer)
									HANDLE_EXCEPT();
								obcopy = DeeObject_Copy(*prefix_pointer);
								if unlikely(!obcopy)
									HANDLE_EXCEPT();
								PUSH(obcopy); /* Inherit reference. */
								if unlikely(!*prefix_pointer) {
#ifdef NDEBUG
									get_prefix_object_ptr();
#else /* NDEBUG */
									prefix_pointer = get_prefix_object_ptr();
									ASSERT(!prefix_pointer);
#endif /* !NDEBUG */
									HANDLE_EXCEPT();
								}
								if (DeeObject_Dec(prefix_pointer))
									HANDLE_EXCEPT();
							} else {
								USING_PREFIX_OBJECT
								prefix_ob = get_prefix_object();
								if unlikely(!prefix_ob)
									HANDLE_EXCEPT();
								obcopy = DeeObject_Copy(prefix_ob);
								if unlikely(!obcopy) {
									Dee_Decref(prefix_ob);
									HANDLE_EXCEPT();
								}
								PUSH(obcopy); /* Inherit reference. */
								if (DeeObject_Dec(&prefix_ob)) {
									Dee_Decref(prefix_ob);
									HANDLE_EXCEPT();
								}
								if unlikely(set_prefix_object(prefix_ob))
									HANDLE_EXCEPT();
							}
							DISPATCH();
						}

						PREFIX_TARGET(ASM16_LROT) {
							DREF DeeObject *drop_object;
							DREF DeeObject *append_object;
							uint32_t shift = (uint32_t)READ_imm16() + 2;
							/* local @foo: lrot #3  (shift == 2)
							* >> temp   = PREFIX;
							* >> PREFIX = SECOND;
							* >> SECOND = FIRST;
							* >> FIRST  = temp;
							* Essentially, operate the same as (without the +1 stack usage):
							* >> push PREFIX
							* >> lrot #3
							* >> pop  PREFIX
							*/
							ASSERT_USAGE(-(int)shift, +(int)shift);
							drop_object = *(sp - shift);
							memmovedownc(sp - shift,
							             sp - (shift - 1),
							             shift - 1,
							             sizeof(DREF DeeObject *));
							append_object = xch_prefix_object(drop_object);
							if unlikely(!append_object) {
								TOP = drop_object;
								HANDLE_EXCEPT();
							}
							TOP = append_object;
							DISPATCH();
						}

						PREFIX_TARGET(ASM16_RROT) {
							DREF DeeObject *drop_object;
							DREF DeeObject *append_object;
							uint32_t shift = (uint32_t)READ_imm16() + 2;
							/* local @foo: rrot #3  (shift == 2)
							 * >> temp   = PREFIX;
							 * >> PREFIX = FIRST;
							 * >> FIRST  = SECOND;
							 * >> SECOND = temp;
							 * Essentially, operate the same as (without the +1 stack usage):
							 * >> push PREFIX
							 * >> rrot #3
							 * >> pop  PREFIX
							 */
							ASSERT_USAGE(-(int)shift, +(int)shift);
							drop_object = TOP;
							memmoveupc(sp - (shift - 1),
							           sp - shift,
							           shift - 1,
							           sizeof(DREF DeeObject *));
							append_object = xch_prefix_object(drop_object);
							if unlikely(!append_object) {
								*(sp - shift) = drop_object;
								HANDLE_EXCEPT();
							}
							*(sp - shift) = append_object;
							DISPATCH();
						}

						PREFIX_TARGET(ASM_PUSH_EXCEPT) {
							DREF DeeObject *temp;
							/* Check if an exception has been set. */
							if unlikely(!this_thread->t_except)
								goto except_no_active_exception;
							temp = this_thread->t_except->ef_error;
							Dee_Incref(temp);
							if (set_prefix_object(temp))
								HANDLE_EXCEPT();
							DISPATCH();
						}

						PREFIX_TARGET(ASM_PUSH_THIS) {
							ASSERT_THISCALL();
							Dee_Incref(THIS);
							if (set_prefix_object(THIS))
								HANDLE_EXCEPT();
							DISPATCH();
						}

						PREFIX_TARGET(ASM_PUSH_THIS_MODULE) {
							Dee_Incref((DeeObject *)code->co_module);
							if (set_prefix_object((DeeObject *)code->co_module))
								HANDLE_EXCEPT();
							DISPATCH();
						}

						PREFIX_TARGET(ASM_PUSH_THIS_FUNCTION) {
							Dee_Incref((DeeObject *)frame->cf_func);
							if (set_prefix_object((DeeObject *)frame->cf_func))
								HANDLE_EXCEPT();
							DISPATCH();
						}

						PREFIX_TARGET(ASM_PUSH_TRUE) {
							Dee_Incref(Dee_True);
							if (set_prefix_object(Dee_True))
								HANDLE_EXCEPT();
							DISPATCH();
						}

						PREFIX_TARGET(ASM_PUSH_FALSE) {
							Dee_Incref(Dee_False);
							if (set_prefix_object(Dee_False))
								HANDLE_EXCEPT();
							DISPATCH();
						}

						PREFIX_TARGET(ASM16_POP_STATIC) {
							imm_val = READ_imm16();
							goto do_prefix_pop_static;
						}

						PREFIX_TARGET(ASM16_POP_EXTERN) {
							imm_val  = READ_imm16();
							imm_val2 = READ_imm16();
							goto do_prefix_pop_extern;
						}

						PREFIX_TARGET(ASM16_POP_GLOBAL) {
							imm_val = READ_imm16();
							goto do_prefix_pop_global;
						}

						PREFIX_TARGET(ASM16_POP_LOCAL) {
							imm_val = READ_imm16();
							goto do_prefix_pop_local;
						}

						PREFIX_TARGET(ASM16_PUSH_MODULE) {
							imm_val = READ_imm16();
							goto do_prefix_push_module;
						}

						PREFIX_TARGET(ASM16_PUSH_REF) {
							imm_val = READ_imm16();
							goto do_prefix_push_ref;
						}

						PREFIX_TARGET(ASM16_PUSH_ARG) {
							imm_val = READ_imm16();
							goto do_prefix_push_arg;
						}

						PREFIX_TARGET(ASM16_PUSH_CONST) {
							imm_val = READ_imm16();
							goto do_prefix_push_const;
						}

						PREFIX_TARGET(ASM16_PUSH_STATIC) {
							imm_val = READ_imm16();
							goto do_prefix_push_static;
						}

						PREFIX_TARGET(ASM16_PUSH_EXTERN) {
							imm_val  = READ_imm16();
							imm_val2 = READ_imm16();
							goto do_prefix_push_extern;
						}

						PREFIX_TARGET(ASM16_PUSH_GLOBAL) {
							imm_val = READ_imm16();
							goto do_prefix_push_global;
						}

						PREFIX_TARGET(ASM16_PUSH_LOCAL) {
							imm_val = READ_imm16();
							goto do_prefix_push_local;
						}

						PREFIX_TARGET(ASM16_DUP_N) {
							uint16_t offset = READ_imm16();
							DREF DeeObject *slot;
							ASSERT_USAGE(-((int)offset + 2), +((int)offset + 2));
							slot = *(sp - (offset + 2));
							Dee_Incref(slot);
							if (set_prefix_object(slot))
								HANDLE_EXCEPT();
							DISPATCH();
						}

						PREFIX_TARGET(ASM16_POP_N) {
							DREF DeeObject *value, *old_value;
							DREF DeeObject **p_slot;
							uint16_t offset = READ_imm16();
							ASSERT_USAGE(-((int)offset + 2), +((int)offset + 2));
							value = get_prefix_object();
							if unlikely(!value)
								HANDLE_EXCEPT();
							p_slot    = sp - (offset + 2);
							old_value = *p_slot;
							*p_slot   = value; /* Inherit reference. */
							Dee_Decref(old_value);
							DISPATCH();
						}

					default:
						goto prefix_unknown_instruction;
					}
				}

				/* Always allow `noop' instructions to be used with a prefix. */
				PREFIX_TARGET(ASM_DELOP)
				PREFIX_TARGET(ASM_NOP)
					DISPATCH();

				PREFIX_TARGET(ASM_SWAP) {
					/* >> local @foo: swap
					 * Same as:
					 * >> xch   top, local @foo */
					DREF DeeObject *new_top;
					ASSERT_USAGE(-1, +1);
					new_top = xch_prefix_object(TOP);
					if unlikely(!new_top)
						HANDLE_EXCEPT();
					TOP = new_top; /* Inherit reference. */
					DISPATCH();
				}

				PREFIX_TARGET(ASM_LROT) {
					DREF DeeObject *drop_object;
					DREF DeeObject *append_object;
					uint16_t shift = (uint16_t)READ_imm8() + 2;
					/* local @foo: lrot #3  (shift == 2)
					 * >> temp   = PREFIX;
					 * >> PREFIX = SECOND;
					 * >> SECOND = FIRST;
					 * >> FIRST  = temp;
					 * Essentially, operate the same as (without the +1 stack usage):
					 * >> push PREFIX
					 * >> lrot #3
					 * >> pop  PREFIX
					 */
					ASSERT_USAGE(-(int)shift, +(int)shift);
					drop_object = *(sp - shift);
					memmovedownc(sp - shift,
					             sp - (shift - 1),
					             shift - 1,
					             sizeof(DREF DeeObject *));
					append_object = xch_prefix_object(drop_object);
					if unlikely(!append_object) {
						TOP = drop_object;
						HANDLE_EXCEPT();
					}
					TOP = append_object;
					DISPATCH();
				}

				PREFIX_TARGET(ASM_RROT) {
					DREF DeeObject *drop_object;
					DREF DeeObject *append_object;
					uint16_t shift = (uint16_t)READ_imm8() + 2;
					/* local @foo: rrot #3  (shift == 2)
					 * >> temp   = PREFIX;
					 * >> PREFIX = FIRST;
					 * >> FIRST  = SECOND;
					 * >> SECOND = temp;
					 * Essentially, operate the same as (without the +1 stack usage):
					 * >> push PREFIX
					 * >> rrot #3
					 * >> pop  PREFIX
					 */
					ASSERT_USAGE(-(int)shift, +(int)shift);
					drop_object = TOP;
					memmoveupc(sp - (shift - 1),
					           sp - shift,
					           shift - 1,
					           sizeof(DREF DeeObject *));
					append_object = xch_prefix_object(drop_object);
					if unlikely(!append_object) {
						*(sp - shift) = drop_object;
						HANDLE_EXCEPT();
					}
					*(sp - shift) = append_object;
					DISPATCH();
				}

				PREFIX_TARGET(ASM_POP_STATIC) {
					DeeObject *old_value;
					DREF DeeObject *value;
					imm_val = READ_imm8();
do_prefix_pop_static:
					ASSERT_STATICimm();
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					STATIC_LOCKWRITE();
					old_value = STATICimm;
					STATICimm = value; /* Inherit reference. */
					STATIC_LOCKENDWRITE();
					ASSERT_OBJECT(old_value);
					Dee_Decref(old_value);
					DISPATCH();
				}

				PREFIX_TARGET(ASM_POP_EXTERN) {
					DeeObject *old_value, **p_extern;
					DREF DeeObject *value;
					imm_val  = READ_imm8();
					imm_val2 = READ_imm8();
do_prefix_pop_extern:
					ASSERT_EXTERNimm();
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					EXTERN_LOCKWRITE();
					p_extern  = &EXTERNimm;
					old_value = *p_extern;
					*p_extern = value; /* Inherit reference. */
					EXTERN_LOCKENDWRITE();
					Dee_XDecref(old_value);
					DISPATCH();
				}

				PREFIX_TARGET(ASM_POP_GLOBAL) {
					DeeObject *old_value, **p_global;
					DREF DeeObject *value;
					imm_val = READ_imm8();
do_prefix_pop_global:
					ASSERT_GLOBALimm();
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					GLOBAL_LOCKWRITE();
					p_global    = &GLOBALimm;
					old_value = *p_global;
					*p_global   = value; /* Inherit reference. */
					GLOBAL_LOCKENDWRITE();
					Dee_XDecref(old_value);
					DISPATCH();
				}

				PREFIX_TARGET(ASM_POP_LOCAL) {
					DeeObject *old_value;
					DREF DeeObject *value;
					imm_val = READ_imm8();
do_prefix_pop_local:
					ASSERT_LOCALimm();
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					old_value = LOCALimm;
					LOCALimm  = value; /* Inherit reference. */
					Dee_XDecref(old_value);
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_MODULE) {
					DeeModuleObject *mod;
					imm_val = READ_imm8();
do_prefix_push_module:
					mod = code->co_module;
					ASSERT_OBJECT(mod);
#ifdef EXEC_SAFE
					if (imm_val >= mod->mo_importc)
						goto err_invalid_module;
#else /* EXEC_SAFE */
					ASSERT(imm_val < mod->mo_importc);
#endif /* !EXEC_SAFE */
					mod = mod->mo_importv[imm_val];
					ASSERT_OBJECT(mod);
					Dee_Incref(mod);
					if (set_prefix_object((DeeObject *)mod))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_NONE) {
					Dee_Incref(Dee_None);
					if (set_prefix_object(Dee_None))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_REF) {
					imm_val = READ_imm8();
do_prefix_push_ref:
					ASSERT_REFimm();
					Dee_Incref(REFimm);
					if (set_prefix_object(REFimm))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_ARG) {
					DREF DeeObject *value;
					imm_val = READ_imm8();
do_prefix_push_arg:
					ASSERT_ARGimm();
					/* Simple case: Direct argument/default reference. */
					if (imm_val < frame->cf_argc) {
						value = frame->cf_argv[imm_val];
					} else if (frame->cf_kw) {
						value = frame->cf_kw->fk_kargv[imm_val - frame->cf_argc];
						if (!value) {
							value = code->co_defaultv[imm_val - code->co_argc_min];
							if (!value)
								goto err_unbound_arg;
						}
					} else {
						value = code->co_defaultv[imm_val - code->co_argc_min];
						if (!value)
							goto err_unbound_arg;
					}
					Dee_Incref(value);
					if (set_prefix_object(value))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_VARARGS) {
					/* Special case: Varargs. */
#ifdef EXEC_SAFE
					if unlikely(!(code->co_flags & CODE_FVARARGS))
						goto err_requires_varargs_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARARGS);
#endif /* !EXEC_SAFE */
					if (!frame->cf_vargs) {
						if (frame->cf_argc <= code->co_argc_max) {
							frame->cf_vargs = (DREF DeeTupleObject *)Dee_EmptyTuple;
							Dee_Incref(Dee_EmptyTuple);
						} else {
							frame->cf_vargs = (DREF DeeTupleObject *)DeeTuple_NewVector((size_t)(frame->cf_argc - code->co_argc_max),
							                                                            frame->cf_argv + code->co_argc_max);
							if unlikely(!frame->cf_vargs)
								HANDLE_EXCEPT();
						}
					}
					Dee_Incref(frame->cf_vargs);
					if (set_prefix_object((DeeObject *)frame->cf_vargs))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_VARKWDS) {
					DREF DeeObject *varkwds;
					/* Special case: Varargs. */
#ifdef EXEC_SAFE
					if (!(code->co_flags & CODE_FVARKWDS))
						goto err_requires_varkwds_code;
#else /* EXEC_SAFE */
					ASSERT(code->co_flags & CODE_FVARKWDS);
#endif /* !EXEC_SAFE */
					if (frame->cf_kw) {
						varkwds = frame->cf_kw->fk_varkwds;
						if (!varkwds) {
							DeeObject *oldval;
							varkwds = construct_varkwds_mapping();
							if unlikely(!varkwds)
								HANDLE_EXCEPT();
							oldval = atomic_cmpxch_val(&frame->cf_kw->fk_varkwds, NULL, varkwds);
							if unlikely(oldval) {
								VARKWDS_DECREF(varkwds);
								varkwds = oldval;
							}
						}
					} else {
						varkwds = Dee_EmptyMapping;
					}
					Dee_Incref(varkwds);
					if (set_prefix_object((DeeObject *)varkwds))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_CONST) {
					DREF DeeObject *value;
					imm_val = READ_imm8();
do_prefix_push_const:
					ASSERT_CONSTimm();
					CONST_LOCKREAD();
					value = CONSTimm;
					Dee_Incref(value);
					CONST_LOCKENDREAD();
					if (set_prefix_object(value))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_STATIC) {
					DREF DeeObject *value;
					imm_val = READ_imm8();
do_prefix_push_static:
					ASSERT_STATICimm();
					STATIC_LOCKREAD();
					value = CONSTimm;
					Dee_Incref(value);
					STATIC_LOCKENDREAD();
					if (set_prefix_object(value))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_EXTERN) {
					DeeObject *value;
					imm_val  = READ_imm8();
					imm_val2 = READ_imm8();
do_prefix_push_extern:
					ASSERT_EXTERNimm();
					EXTERN_LOCKREAD();
					value = EXTERNimm;
					if unlikely(!value) {
						EXTERN_LOCKENDREAD();
						goto err_unbound_extern;
					}
					Dee_Incref(value);
					EXTERN_LOCKENDREAD();
					if (set_prefix_object(value))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_GLOBAL) {
					DeeObject *value;
					imm_val = READ_imm8();
do_prefix_push_global:
					ASSERT_GLOBALimm();
					GLOBAL_LOCKREAD();
					value = GLOBALimm;
					if unlikely(!value) {
						GLOBAL_LOCKENDREAD();
						goto err_unbound_global;
					}
					Dee_Incref(value);
					GLOBAL_LOCKENDREAD();
					if (set_prefix_object(value))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_PUSH_LOCAL) {
					DeeObject *value;
					imm_val = READ_imm8();
do_prefix_push_local:
					ASSERT_LOCALimm();
					value = LOCALimm;
					if unlikely(!value)
						goto err_unbound_local;
					Dee_Incref(value);
					if (set_prefix_object(value))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_DUP) {
					Dee_Incref(TOP);
					if (set_prefix_object(TOP))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_DUP_N) {
					uint8_t offset = READ_imm8();
					DREF DeeObject *slot;
					ASSERT_USAGE(-((int)offset + 2), +((int)offset + 2));
					slot = *(sp - (offset + 2));
					Dee_Incref(slot);
					if (set_prefix_object(slot))
						HANDLE_EXCEPT();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_POP) {
					DREF DeeObject *value;
					ASSERT_USAGE(-1, +1);
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					Dee_Decref(TOP);
					TOP = value; /* Inherit reference. */
					DISPATCH();
				}

				PREFIX_TARGET(ASM_POP_N) {
					DREF DeeObject *value, *old_value;
					DREF DeeObject **p_slot;
					uint8_t offset = READ_imm8();
					ASSERT_USAGE(-((int)offset + 2), +((int)offset + 2));
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					p_slot    = sp - (offset + 2);
					old_value = *p_slot;
					*p_slot   = value; /* Inherit reference. */
					Dee_Decref(old_value);
					DISPATCH();
				}

				PREFIX_TARGET(ASM_RET) {
					DREF DeeObject *value;
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					/* Check if we're overwriting a previous return value
					 * (which can happen when `return' appears in a finally-block) */
					if (ITER_ISOK(frame->cf_result))
						Dee_Decref(frame->cf_result);
					frame->cf_result = value;
					if (code->co_flags & CODE_FYIELDING)
						goto end_without_finally;
					goto end_return;
				}

				PREFIX_TARGET(ASM_YIELDALL) {
					DREF DeeObject *value;
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					ASSERT_YIELDING();
					if (ITER_ISOK(frame->cf_result))
						Dee_Decref(frame->cf_result);
					frame->cf_result = DeeObject_IterNext(value);
					Dee_Decref(value);
					if unlikely(!frame->cf_result)
						HANDLE_EXCEPT();
					if (frame->cf_result != ITER_DONE) {
						/* Repeat this instruction and forward the value we've just read. */
						REPEAT_INSTRUCTION();
						YIELD_RESULT();
					}
					/* Pop the iterator that was enumerated. */
					POPREF();
					DISPATCH();
				}

				PREFIX_TARGET(ASM_THROW) {
					DREF DeeObject *value;
					value = get_prefix_object();
					if unlikely(!value)
						HANDLE_EXCEPT();
					DeeError_Throw(value);
					Dee_Decref(value);
					HANDLE_EXCEPT();
				}

			default:
prefix_unknown_instruction:
				err_illegal_instruction(code, frame->cf_ip);
				HANDLE_EXCEPT();
			}

			/* Shouldn't get here. */
			__builtin_unreachable();
		} /* End of prefixed instruction handling. */

#ifdef USE_SWITCH
	default:
#endif /* USE_SWITCH */
#ifndef USE_SWITCH
target_ASM_UD:
target_ASM_RESERVED1:
target_ASM_RESERVED2:
target_ASM_RESERVED3:
target_ASM_RESERVED4:
target_ASM_RESERVED5:
target_ASM_RESERVED6:
target_ASM_RESERVED7:
#endif /* !USE_SWITCH */
unknown_instruction:
		err_illegal_instruction(code, frame->cf_ip);
		HANDLE_EXCEPT();
	}

	/* Opcode handlers must continue execution using
	 * `DISPATCH()', so we must not be able to get here. */
	__builtin_unreachable();

	{
		struct except_handler *current_except;
end_return:
		if (code->co_flags & CODE_FFINALLY) {
			code_addr_t ip_addr = (code_addr_t)(frame->cf_ip - code->co_code);
			ASSERT(code->co_exceptc != 0);
			/* Execute finally handlers. */
			current_except = code->co_exceptv + code->co_exceptc;
			while (current_except > code->co_exceptv) {
				--current_except;
				if (current_except->eh_flags & EXCEPTION_HANDLER_FFINALLY &&
				    ip_addr >= current_except->eh_start &&
				    ip_addr < current_except->eh_end) {
					/* Found a handler that must be executed. */
					goto exec_except;
				}
			}
		}
end_without_finally:

		/* Store exit PC/SP */
		frame->cf_ip = ip.ptr;
		frame->cf_sp = sp;

		/* Unhook frame from the thread-local execution stack. */
		ASSERT(this_thread->t_execsz != 0);
		ASSERT(this_thread->t_exec == frame);
		ASSERT(frame->cf_prev != CODE_FRAME_NOT_EXECUTING);
		--this_thread->t_execsz;
		this_thread->t_exec = frame->cf_prev;
		frame->cf_prev      = CODE_FRAME_NOT_EXECUTING;
#ifdef EXEC_FAST
end_nounhook:
#endif /* EXEC_FAST */
		/* Special case: Prevent return/yield from catch:
		 * Although the old deemon allowed users to use return/yield
		 * from catch/finally blocks, the new deemon doesn't, instead
		 * opting to discard such attempts and immediately propagate
		 * the exception. */
		if unlikely(this_thread->t_exceptsz > except_recursion) {
			/* Clear the return value. */
			if (ITER_ISOK(frame->cf_result))
				Dee_Decref(frame->cf_result);
			frame->cf_result = NULL;
			/* Additionally (to keep things consistent), despite
			 * allowing multiple exceptions to be raised at once,
			 * deemon now only allows functions to return with a
			 * single new exception set.
			 * With that in mind, we simply discard any additional
			 * exceptions that were raised after the first one was,
			 * greatly simplifying things when compared against the
			 * mess that the old deemon did, with multiple catch blocks
			 * stacked on top of each other, while still not providing
			 * a way of _truely_ catching all exception.
			 * This time around though, we make things much more obvious
			 * in allowing only a single exception to be returned by
			 * any function. */
			if unlikely(this_thread->t_exceptsz > except_recursion + 1) {
				uint16_t num_discard = (uint16_t)(this_thread->t_exceptsz - (except_recursion + 1));
				/* XXX: If we got here because of an interrupt exception,
				 *      having that exception be re-scheduled as a pending
				 *      interrupt may trigger `DeeThread_CheckInterrupt()'
				 *      of the I/O code used to print the error message,
				 *      thus causing an infinite loop? */
				do {
					DeeError_Print("Discarding secondary error\n", ERROR_PRINT_DOHANDLE);
				} while (--num_discard);
			}
		}

		return frame->cf_result;

		/* Exception handling. */
handle_except:
		{
			DeeObject *current_exception;
			/* Search for exception handlers covering
			 * the base of the current instruction. */
			code_addr_t ip_addr = (code_addr_t)(frame->cf_ip - code->co_code);
			ASSERTF(except_recursion < this_thread->t_exceptsz,
			        "No new exceptions have been thrown\n"
			        "ip_addr = +%.4I32X\n"
			        "name    = %s\n",
			        (uint32_t)ip_addr, DeeCode_NAME(code));
			ASSERTF(this_thread->t_except, "No error has been set");
			/* Lazily allocate a missing traceback.
			 * TODO: Only include information that would become lost _now_ in the traceback.
			 *       All other information should be added as the stack continues being
			 *       unwound, thus allowing for O(1) exceptions in small helper functions,
			 *       regardless of how deep the current execution stack actually is! */
			if (this_thread->t_except->ef_trace == (DREF DeeTracebackObject *)ITER_DONE)
				this_thread->t_except->ef_trace = DeeTraceback_New(this_thread);
			current_exception = this_thread->t_except->ef_error;
			current_except    = code->co_exceptv + code->co_exceptc;
			while (current_except > code->co_exceptv) {
				--current_except;
				if (!(ip_addr >= current_except->eh_start &&
				    ip_addr < current_except->eh_end))
					continue;
				/* Special case: interrupt objects can only be caught by interrupt-handlers. */
				if (DeeObject_IsInterrupt(current_exception) &&
				    !(current_except->eh_flags & EXCEPTION_HANDLER_FINTERPT))
					continue;
				/* Check the exception mask. */
				if (current_except->eh_mask) {
					DeeTypeObject *thrown_object_type;
					thrown_object_type = Dee_TYPE(current_exception);
					if (thrown_object_type == &DeeSuper_Type) /* Special case for super-views */
						thrown_object_type = DeeSuper_TYPE(current_exception);
					if (!DeeType_IsInherited(thrown_object_type, current_except->eh_mask))
						continue;
				}
				goto exec_except_maybe_handle; /* Execute this one! */
			}
		}
		if (ITER_ISOK(frame->cf_result))
			Dee_Decref(frame->cf_result);
		frame->cf_result = NULL;
		ip.ptr           = frame->cf_ip;
		goto end_return;
exec_except_maybe_handle:
		/* If the exception handler requests it,
		 * already handle the error beforehand. */
		if (current_except->eh_flags & EXCEPTION_HANDLER_FHANDLED)
			DeeError_Handled(ERROR_HANDLED_INTERRUPT);
exec_except:
		ASSERTF(current_except->eh_addr < current_except->eh_start ||
		        current_except->eh_addr >= current_except->eh_end,
		        "An exception handler must not be used to protect itself. "
		        "Such constructs would lead to infinite recursion.");
		frame->cf_sp = sp;
		{
			struct except_frame *iter;
			/* Use this point to extend upon tracebacks!
			 * Considering the lazy updating of code-frame PC/SP, any code
			 * that causes an exception can no longer rely upon the validity
			 * of the stack/instruction pointers it encounters on the stack.
			 * (Well... It can read the start-ip register safely, as this
			 * implementation stores that one within the frame)
			 * Instead, we dynamically add upon tracebacks here, filling in
			 * anything left undefined until now as the stack is unwound! */
			ASSERT(this_thread->t_execsz);
			iter = this_thread->t_except;
			for (; iter; iter = iter->ef_prev) {
				if (!ITER_ISOK(iter->ef_trace))
					continue;
				if unlikely(iter->ef_trace->tb_thread != this_thread)
					continue;
				DeeTraceback_AddFrame(iter->ef_trace, frame,
				                      this_thread->t_execsz - 1);
			}
		}
		ip.ptr = code->co_code + current_except->eh_addr;
#ifdef EXEC_SAFE
		if unlikely(ip.ptr < code->co_code ||
		            ip.ptr >= code->co_code + code->co_codebytes)
			goto err_invalid_ip;
#else /* EXEC_SAFE */
		ASSERT(ip.ptr >= code->co_code &&
		       ip.ptr < code->co_code + code->co_codebytes);
#endif /* !EXEC_SAFE */
		/* Adjust the stack according to requirements imposed by this exception handler. */
		{
			uint16_t cur_depth = (uint16_t)(sp - frame->cf_stack);
			uint16_t new_depth = current_except->eh_stack;
			if (cur_depth > new_depth) {
				cur_depth -= new_depth;
				while (cur_depth--)
					POPREF();
			} else /*if (cur_depth < new_depth)*/ {
				/* Push `none' to adjust the stack. */
				new_depth -= cur_depth;
				while (new_depth--)
					PUSHREF(Dee_None);
			}
			ASSERT(current_except->eh_stack ==
			       (uint16_t)(sp - frame->cf_stack));
		}

		/* With the new PC address active, check for interrupts to prevent
		 * potential loop constructs using exceptions of all things... */
		if (DeeThread_CheckInterruptSelf(this_thread))
			goto handle_except;

		/* Continue execution within this exception handler. */
		goto next_instr;
	} /* End of local variables scope. */

	/* --- Exception creation --- */

	/* Invalid use errors. */
#ifdef EXEC_SAFE
increase_stacksize:
	ip.ptr = frame->cf_ip;
	/* If lenient mode isn't enabled, then we can't dynamically grow the stack. */
	if (!(code->co_flags & CODE_FLENIENT))
		goto stack_fault;
	{
		DeeObject **new_stack;
		uint16_t new_size = frame->cf_stacksz;
		/* Determine the new stack size. */
		if (!new_size)
			new_size = STACKPREALLOC;
		new_size *= 2;
		if unlikely(new_size < frame->cf_stacksz)
			new_size = 0xffff; /* Overflow -> Try the max value */
		/* TODO:
		 * >> if unlikely(new_size > RUNTIME_OPTION_MAX_STACKSIZE)
		 * >>             new_size = RUNTIME_OPTION_MAX_STACKSIZE;
		 */
		if unlikely(new_size == frame->cf_stacksz) {
			/* The stack has already grown to its maximum. - Raise an error. */
stack_fault:
			DeeError_Throwf(&DeeError_SegFault, "Stack segment overflow");
			HANDLE_EXCEPT();
		}
		ASSERT(new_size != 0);

		/* Allocate/Re-allocate the stack on the heap. */
		if (frame->cf_stacksz) {
			new_stack = (DeeObject **)Dee_Reallocc(frame->cf_stack,
			                                       (size_t)new_size,
			                                       sizeof(DeeObject *));
		} else {
			new_stack = (DeeObject **)Dee_Mallocc((size_t)new_size, sizeof(DeeObject *));
		}
		if unlikely(!new_stack)
			HANDLE_EXCEPT();

		/* Install the new stack. */
		if (!frame->cf_stacksz) {
			/* Copy the old contents of the stack onto the new one. */
			memcpyc(new_stack,
			        frame->cf_stack,
			        sp - frame->cf_stack,
			        sizeof(DeeObject *));
		}

		/* Hook the new stack. */
		frame->cf_stacksz = new_size; /* A non-zero `cf_stacksz' value indicates a heap stack! */
		sp                = new_stack + (sp - frame->cf_stack);
		frame->cf_stack   = new_stack;
		frame->cf_sp      = sp; /* Set a new frame-sp that is part of the actual stack. */

		/* Try execute the current instruction again. */
		goto next_instr;
	}
err_invalid_stack_affect:
	DeeError_Throwf(&DeeError_SegFault, "Invalid stack effect at +%.4I32X",
	                (code_addr_t)(frame->cf_ip - code->co_code));
	HANDLE_EXCEPT();
	{
		DeeTypeObject *required_type;
err_requires_tuple:
		required_type = &DeeTuple_Type;
		goto illegal_type;
err_requires_string:
		required_type = &DeeString_Type;
illegal_type:
		DeeError_Throwf(&DeeError_TypeError,
		                "Instruction requires an instance of `%k'",
		                required_type);
		HANDLE_EXCEPT();
	}
err_invalid_ref:
	frame->cf_sp = sp;
	err_srt_invalid_ref(frame, imm_val);
	HANDLE_EXCEPT();
err_invalid_module:
	frame->cf_sp = sp;
	err_srt_invalid_module(frame, imm_val);
	HANDLE_EXCEPT();
err_invalid_extern:
	frame->cf_sp = sp;
	err_srt_invalid_extern(frame, imm_val, imm_val2);
	HANDLE_EXCEPT();
err_invalid_global:
	frame->cf_sp = sp;
	err_srt_invalid_global(frame, imm_val);
	HANDLE_EXCEPT();
err_invalid_locale:
	frame->cf_sp = sp;
	err_srt_invalid_locale(frame, imm_val);
	HANDLE_EXCEPT();
err_invalid_const:
	frame->cf_sp = sp;
	err_srt_invalid_const(frame, imm_val);
	HANDLE_EXCEPT();
err_invalid_static:
	frame->cf_sp = sp;
	err_srt_invalid_static(frame, imm_val);
	HANDLE_EXCEPT();
err_invalid_instance_addr:
err_invalid_ip:
err_invalid_operands:
err_requires_varargs_code:
err_requires_varkwds_code:
err_requires_yield_code:
err_requires_thiscall_code:
err_invalid_argument_index:
err_cannot_push_exception:
	err_illegal_instruction(code, frame->cf_ip);
	HANDLE_EXCEPT();
#endif /* EXEC_SAFE */
err_unbound_arg:
	ASSERT(imm_val <= code->co_argc_max);
	err_unbound_arg(code, frame->cf_ip, imm_val);
	HANDLE_EXCEPT();
err_unbound_extern:
	ASSERT(imm_val <= code->co_module->mo_importc);
	ASSERT(imm_val2 <= code->co_module->mo_importv[imm_val]->mo_globalc);
	err_unbound_global(code->co_module->mo_importv[imm_val], imm_val2);
	HANDLE_EXCEPT();
err_unbound_global:
	err_unbound_global(code->co_module, imm_val);
	HANDLE_EXCEPT();
err_unbound_local:
	err_unbound_local(code, frame->cf_ip, imm_val);
	HANDLE_EXCEPT();
}

DECL_END

#ifdef _MSC_VER
#pragma optimize("", on)
#endif /* _MSC_VER */

__pragma_GCC_diagnostic_pop

#undef NEED_UNIVERSAL_PREFIX_OB_WORKAROUND
#undef USING_PREFIX_OBJECT

#undef REFimm
#undef LOCALimm
#undef EXTERNimm
#undef GLOBALimm
#undef STATICimm
#undef CONSTimm
#undef EXTERN_LOCKREAD
#undef EXTERN_LOCKENDREAD
#undef EXTERN_LOCKWRITE
#undef EXTERN_LOCKENDWRITE
#undef GLOBAL_LOCKREAD
#undef GLOBAL_LOCKENDREAD
#undef GLOBAL_LOCKWRITE
#undef GLOBAL_LOCKENDWRITE
#undef STATIC_LOCKREAD
#undef STATIC_LOCKENDREAD
#undef STATIC_LOCKWRITE
#undef STATIC_LOCKENDWRITE
#undef THIS
#undef TOP
#undef FIRST
#undef SECOND
#undef THIRD
#undef FOURTH
#undef POP
#undef POPREF
#undef PUSH
#undef PUSHREF
#undef STACK_BEGIN
#undef STACK_END
#undef STACKUSED
#undef STACKPREALLOC
#undef RAW_TARGET
#undef ASSERT_CONSTimm
#undef ASSERT_CONSTimm2
#undef ASSERT_TUPLE
#undef ASSERT_STRING
#undef ASSERT_THISCALL
#undef CONST_LOCKREAD
#undef CONST_LOCKENDREAD
#undef CONST_LOCKWRITE
#undef CONST_LOCKENDWRITE
#undef ASSERT_ARGimm
#undef ASSERT_REFimm
#undef ASSERT_EXTERNimm
#undef ASSERT_GLOBALimm
#undef ASSERT_LOCALimm
#undef ASSERT_STATICimm
#undef ASSERT_YIELDING
#undef EXCEPTION_CLEANUP
#undef STACKSIZE
#undef STACKFREE
#undef ASSERT_USAGE
#undef TARGET
#undef READ_imm8
#undef READ_imm16
#undef READ_imm32
#undef READ_Simm8
#undef READ_Simm16
#undef READ_Simm32
#undef TARGETSimm16
#undef REPEAT_INSTRUCTION
#undef HANDLE_EXCEPT
#undef DISPATCH
#undef YIELD_RESULT
#undef RETURN_RESULT
#undef YIELD
#undef RETURN
#undef USE_SWITCH
#undef construct_varkwds_mapping
#undef set_prefix_object
#undef xch_prefix_object
#undef get_prefix_object_ptr
#undef get_prefix_object
