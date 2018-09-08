/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#define EXEC_SAFE 1
#define _KOS_SOURCE 1
#endif

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/cell.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/hashset.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/string.h>
#include <deemon/util/cache.h>

#include <hybrid/atomic.h>
#include <hybrid/unaligned.h>
#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/sched/yield.h>

#include "../objects/seq/svec.h"
#include "../objects/seq/smap.h"
#include "../runtime/runtime_error.h"

#ifndef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX  __SSIZE_MAX__
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4102) /* Unused label. */
#pragma warning(push)
#pragma warning(disable: 4127)
#ifdef NDEBUG
#pragma warning(disable: 4701) /* Potentially uninitialized variable (See `DEBUG_SET_IMMVAL2()' below) */
#endif
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-Wsign-compare"
#if __GNUC__ >= 7
#pragma GCC diagnostic ignored "-Wint-in-bool-context" /* Produces some incorrect warnings. */
#endif
#endif

#ifdef _MSC_VER
/* Make sure to enable optimizations for the interpreter, as MSVC will otherwise
 * put every local variable defined below into a non-aliased stack-address, which
 * will drastically increase the required frame-size to the point where deemon
 * could crash from otherwise valid constructs, such as an execution depth of
 * only ~100 function calls, which is something that could legitimatly happen
 * in valid code.
 */
#pragma optimize("ts", on)
#endif

#if (defined(EXEC_SAFE) && defined(EXEC_FAST)) || \
    (!defined(EXEC_SAFE) && !defined(EXEC_FAST))
#error "Invalid configuration. - Must either define `EXEC_SAFE' or `EXEC_FAST'"
#endif

#ifndef CONFIG_COMPILER_HAVE_ADDRESSIBLE_LABELS
#define USE_SWITCH
#endif

DECL_BEGIN

#ifndef EXCEPT_FRAME_CACHE_DECLARED
#define EXCEPT_FRAME_CACHE_DECLARED 1
DECLARE_STRUCT_CACHE(ef,struct except_frame)
#endif

#ifndef FILE_SHL_DECLARED
#define FILE_SHL_DECLARED 1
INTDEF DREF DeeObject *DCALL
file_shl(DeeObject *__restrict self,
         DeeObject *__restrict some_object);
#endif

#ifdef EXEC_FAST
#define get_prefix_object()  get_prefix_object_fast(frame,code,sp)
PRIVATE DREF DeeObject *ATTR_FASTCALL
get_prefix_object_fast(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp)
#else
#define get_prefix_object()  get_prefix_object_safe(frame,code,sp)
PRIVATE DREF DeeObject *ATTR_FASTCALL
get_prefix_object_safe(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp)
#endif
{
 DREF DeeObject *result;
 instruction_t *ip = frame->cf_ip;
 DeeModuleObject *module;
 uint16_t imm_val;
 switch (*ip++) {

 case ASM_STACK:
  imm_val = *(uint8_t *)ip;
do_get_stack:
#ifdef EXEC_SAFE
  if unlikely((frame->cf_stack + imm_val) >= sp) {
   frame->cf_sp = sp;
   err_srt_invalid_sp(frame,imm_val);
   return NULL;
  }
#else
  ASSERT((frame->cf_stack + imm_val) < sp);
#endif
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
   err_srt_invalid_static(frame,imm_val);
   return NULL;
  }
#else
  ASSERT(imm_val < code->co_staticc);
#endif
#ifndef CONFIG_NO_THREADS
  rwlock_read(&code->co_static_lock);
#endif
  result = code->co_staticv[imm_val];
  ASSERT_OBJECT(result);
  Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&code->co_static_lock);
#endif
  break;

 case ASM_EXTERN:
#ifdef EXEC_SAFE
  imm_val = *(uint8_t *)(ip + 1);
  if unlikely(*(uint8_t *)(ip + 0) >= code->co_module->mo_importc) {
err_invalid_extern:
   frame->cf_sp = sp;
   err_srt_invalid_extern(frame,*(uint8_t *)(ip + 0),imm_val);
   return NULL;
  }
  module = code->co_module->mo_importv[*(uint8_t *)(ip + 0)];
  if unlikely(imm_val >= module->mo_globalc)
     goto err_invalid_extern;
#else
  ASSERT(*(uint8_t *)(ip + 0) < code->co_module->mo_importc);
  module = code->co_module->mo_importv[*(uint8_t *)(ip + 0)];
  imm_val = *(uint8_t *)(ip + 1);
  ASSERT(imm_val < module->mo_globalc);
#endif
  goto do_get_module_object;
 case ASM_GLOBAL:
  imm_val = *(uint8_t *)(ip + 0);
do_get_global:
  module = code->co_module;
#ifdef EXEC_SAFE
  if unlikely(imm_val >= module->mo_globalc) {
   frame->cf_sp = sp;
   err_srt_invalid_global(frame,imm_val);
   return NULL;
  }
#else
  ASSERT(imm_val < module->mo_globalc);
#endif
do_get_module_object:
#ifndef CONFIG_NO_THREADS
  rwlock_read(&module->mo_lock);
#endif
  result = module->mo_globalv[imm_val];
  Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&module->mo_lock);
#endif
  if unlikely(!result)
     err_unbound_global(module,imm_val);
  ASSERT_OBJECT_OPT(result);
  break;

 case ASM_LOCAL:
  imm_val = *(uint8_t *)(ip + 0);
do_get_local:
#ifdef EXEC_SAFE
  if unlikely(imm_val >= code->co_localc) {
   frame->cf_sp = sp;
   err_srt_invalid_locale(frame,imm_val);
   return NULL;
  }
#else
  ASSERT(imm_val < code->co_localc);
#endif
  result = frame->cf_frame[imm_val];
  if likely(result) {
   Dee_Incref(result);
  } else {
   err_unbound_local(code,frame->cf_ip,imm_val);
  }
  break;

 case ASM_EXTENDED1:
  switch (*ip++) {

  case ASM16_STACK & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)ip);
   goto do_get_stack;
  case ASM16_STATIC & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)ip);
   goto do_get_static;
  case ASM16_EXTERN & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
#ifdef EXEC_SAFE
   if unlikely(imm_val >= code->co_module->mo_importc) {
err_invalid_extern16:
    frame->cf_sp = sp;
    err_srt_invalid_extern(frame,UNALIGNED_GETLE16((uint16_t *)(ip + 0)),imm_val);
    return NULL;
   }
   module = code->co_module->mo_importv[imm_val];
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 2));
   if unlikely(imm_val >= module->mo_globalc)
      goto err_invalid_extern16;
#else
   ASSERT(imm_val < code->co_module->mo_importc);
   module = code->co_module->mo_importv[imm_val];
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 2));
   ASSERT(imm_val < module->mo_globalc);
#endif
   goto do_get_module_object;
  case ASM16_GLOBAL & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
   goto do_get_global;
  case ASM16_LOCAL & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
   goto do_get_local;

  default:
#ifdef EXEC_SAFE
   goto ill_instr;
#else
   __builtin_unreachable();
#endif
  }
  break;

 default:
#ifdef EXEC_SAFE
ill_instr:
  err_illegal_instruction(code,frame->cf_ip);
  return NULL;
#else
  __builtin_unreachable();
#endif
 }
 return result;
}


/* NOTE: A reference to `value' is only inherited upon success (return != NULL) */
#ifdef EXEC_FAST
#define xch_prefix_object(v)  xch_prefix_object_fast(frame,code,sp,v)
PRIVATE DREF DeeObject *ATTR_FASTCALL
xch_prefix_object_fast(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#else
#define xch_prefix_object(v)  xch_prefix_object_safe(frame,code,sp,v)
PRIVATE DREF DeeObject *ATTR_FASTCALL
xch_prefix_object_safe(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#endif
{
 DREF DeeObject *result;
 instruction_t *ip = frame->cf_ip;
 DeeModuleObject *module;
 uint16_t imm_val;
 switch (*ip++) {

 case ASM_STACK:
  imm_val = *(uint8_t *)ip;
do_get_stack:
#ifdef EXEC_SAFE
  if unlikely((frame->cf_stack + imm_val) >= sp) {
   frame->cf_sp = sp;
   err_srt_invalid_sp(frame,imm_val);
   return NULL;
  }
#else
  ASSERT((frame->cf_stack + imm_val) < sp);
#endif
  result = frame->cf_stack[imm_val]; /* Inherit reference. */
  frame->cf_stack[imm_val] = value;  /* Inherit reference. */
  ASSERT_OBJECT(result);
  break;

 case ASM_STATIC:
  imm_val = *(uint8_t *)ip;
do_get_static:
#ifdef EXEC_SAFE
  if unlikely(imm_val >= code->co_staticc) {
   frame->cf_sp = sp;
   err_srt_invalid_static(frame,imm_val);
   return NULL;
  }
#else
  ASSERT(imm_val < code->co_staticc);
#endif
#ifndef CONFIG_NO_THREADS
  rwlock_write(&code->co_static_lock);
#endif
  result = code->co_staticv[imm_val]; /* Inherit reference. */
  code->co_staticv[imm_val] = value;  /* Inherit reference. */
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&code->co_static_lock);
#endif
  ASSERT_OBJECT(result);
  break;

 case ASM_EXTERN:
#ifdef EXEC_SAFE
  imm_val = *(uint8_t *)(ip + 1);
  if unlikely(*(uint8_t *)(ip + 0) >= code->co_module->mo_importc) {
err_invalid_extern:
   frame->cf_sp = sp;
   err_srt_invalid_extern(frame,*(uint8_t *)(ip + 0),imm_val);
   return NULL;
  }
  module = code->co_module->mo_importv[*(uint8_t *)(ip + 0)];
  if unlikely(imm_val >= module->mo_globalc)
     goto err_invalid_extern;
#else
  ASSERT(*(uint8_t *)(ip + 0) < code->co_module->mo_importc);
  module = code->co_module->mo_importv[*(uint8_t *)(ip + 0)];
  imm_val = *(uint8_t *)(ip + 1);
  ASSERT(imm_val < module->mo_globalc);
#endif
  goto do_get_module_object;
 case ASM_GLOBAL:
  imm_val = *(uint8_t *)(ip + 0);
do_get_global:
  module = code->co_module;
#ifdef EXEC_SAFE
  if unlikely(imm_val >= module->mo_globalc) {
   frame->cf_sp = sp;
   err_srt_invalid_global(frame,imm_val);
   return NULL;
  }
#else
  ASSERT(imm_val < module->mo_globalc);
#endif
do_get_module_object:
#ifndef CONFIG_NO_THREADS
  rwlock_write(&module->mo_lock);
#endif
  result = module->mo_globalv[imm_val]; /* Inherit reference. */
  if unlikely(!result) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&module->mo_lock);
#endif
   err_unbound_global(module,imm_val);
   return NULL;
  }
  module->mo_globalv[imm_val] = value; /* Inherit reference. */
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&module->mo_lock);
#endif
  ASSERT_OBJECT(result);
  break;

 case ASM_LOCAL:
  imm_val = *(uint8_t *)(ip + 0);
do_get_local:
#ifdef EXEC_SAFE
  if unlikely(imm_val >= code->co_localc) {
   frame->cf_sp = sp;
   err_srt_invalid_locale(frame,imm_val);
   return NULL;
  }
#else
  ASSERT(imm_val < code->co_localc);
#endif
  result = frame->cf_frame[imm_val]; /* Inherit reference. */
  if likely(result) {
   frame->cf_frame[imm_val] = value; /* Inherit reference. */
  } else {
   err_unbound_local(code,frame->cf_ip,imm_val);
  }
  break;

 case ASM_EXTENDED1:
  switch (*ip++) {

  case ASM16_STACK & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)ip);
   goto do_get_stack;
  case ASM16_STATIC & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)ip);
   goto do_get_static;
  case ASM16_EXTERN & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
#ifdef EXEC_SAFE
   if unlikely(imm_val >= code->co_module->mo_importc) {
err_invalid_extern16:
    frame->cf_sp = sp;
    err_srt_invalid_extern(frame,UNALIGNED_GETLE16((uint16_t *)(ip + 0)),imm_val);
    return NULL;
   }
   module = code->co_module->mo_importv[imm_val];
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 2));
   if unlikely(imm_val >= module->mo_globalc)
      goto err_invalid_extern16;
#else
   ASSERT(imm_val < code->co_module->mo_importc);
   module = code->co_module->mo_importv[imm_val];
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 2));
   ASSERT(imm_val < module->mo_globalc);
#endif
   goto do_get_module_object;
  case ASM16_GLOBAL & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
   goto do_get_global;
  case ASM16_LOCAL & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
   goto do_get_local;

  default:
#ifdef EXEC_SAFE
   goto ill_instr;
#else
   __builtin_unreachable();
#endif
  }
  break;

 default:
#ifdef EXEC_SAFE
ill_instr:
  err_illegal_instruction(code,frame->cf_ip);
  return NULL;
#else
  __builtin_unreachable();
#endif
 }
 return result;
}


/* NOTE: A reference to `value' is _always_ inherited */
#ifdef EXEC_FAST
#define set_prefix_object(v)  unlikely(set_prefix_object_fast(frame,code,sp,v))
PRIVATE int ATTR_FASTCALL
set_prefix_object_fast(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#else
#define set_prefix_object(v)  unlikely(set_prefix_object_safe(frame,code,sp,v))
PRIVATE int ATTR_FASTCALL
set_prefix_object_safe(struct code_frame *__restrict frame,
                       DeeCodeObject *__restrict code,
                       DeeObject **__restrict sp,
                       DREF DeeObject *__restrict value)
#endif
{
 DREF DeeObject *old_value;
 instruction_t *ip = frame->cf_ip;
 DeeModuleObject *module;
 uint16_t imm_val;
 ASSERT_OBJECT(value);
 switch (*ip++) {

 case ASM_STACK:
  imm_val = *(uint8_t *)ip;
do_set_stack:
#ifdef EXEC_SAFE
  if unlikely((frame->cf_stack + imm_val) >= sp) {
   frame->cf_sp = sp;
   err_srt_invalid_sp(frame,imm_val);
   Dee_Decref(value);
   return -1;
  }
#else
  ASSERT((frame->cf_stack + imm_val) < sp);
#endif
  old_value = frame->cf_stack[imm_val];
  frame->cf_stack[imm_val] = value;
  Dee_Decref(old_value);
  break;

 case ASM_STATIC:
  imm_val = *(uint8_t *)ip;
do_set_static:
#ifdef EXEC_SAFE
  if unlikely(imm_val >= code->co_staticc) {
   frame->cf_sp = sp;
   err_srt_invalid_static(frame,imm_val);
   Dee_Decref(value);
   return -1;
  }
#else
  ASSERT(imm_val < code->co_staticc);
#endif
#ifndef CONFIG_NO_THREADS
  rwlock_write(&code->co_static_lock);
#endif
  old_value = code->co_staticv[imm_val];
  code->co_staticv[imm_val] = value;
  ASSERT_OBJECT(old_value);
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&code->co_static_lock);
#endif
  Dee_Decref(old_value);
  break;

 case ASM_EXTERN:
#ifdef EXEC_SAFE
  imm_val = *(uint8_t *)(ip + 1);
  if unlikely(*(uint8_t *)(ip + 0) >= code->co_module->mo_importc) {
err_invalid_extern:
   frame->cf_sp = sp;
   err_srt_invalid_extern(frame,*(uint8_t *)(ip + 0),imm_val);
   Dee_Decref(value);
   return -1;
  }
  module = code->co_module->mo_importv[*(uint8_t *)(ip + 0)];
  if unlikely(imm_val >= module->mo_globalc)
     goto err_invalid_extern;
#else
  ASSERT(*(uint8_t *)(ip + 0) < code->co_module->mo_importc);
  module = code->co_module->mo_importv[*(uint8_t *)(ip + 0)];
  imm_val = *(uint8_t *)(ip + 1);
  ASSERT(imm_val < module->mo_globalc);
#endif
  goto do_set_module_object;
 case ASM_GLOBAL:
  imm_val = *(uint8_t *)(ip + 0);
do_set_global:
  module = code->co_module;
#ifdef EXEC_SAFE
  if unlikely(imm_val >= module->mo_globalc) {
   frame->cf_sp = sp;
   err_srt_invalid_global(frame,imm_val);
   Dee_Decref(value);
   return -1;
  }
#else
  ASSERT(imm_val < module->mo_globalc);
#endif
do_set_module_object:
#ifndef CONFIG_NO_THREADS
  rwlock_write(&module->mo_lock);
#endif
  old_value = module->mo_globalv[imm_val];
  module->mo_globalv[imm_val] = value;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&module->mo_lock);
#endif
  Dee_XDecref(old_value);
  break;

 case ASM_LOCAL:
  imm_val = *(uint8_t *)(ip + 0);
do_set_local:
#ifdef EXEC_SAFE
  if unlikely(imm_val >= code->co_localc) {
   frame->cf_sp = sp;
   err_srt_invalid_locale(frame,imm_val);
   Dee_Decref(value);
   return -1;
  }
#else
  ASSERT(imm_val < code->co_localc);
#endif
  old_value = frame->cf_frame[imm_val];
  frame->cf_frame[imm_val] = value;
  Dee_XDecref(old_value);
  break;

 case ASM_EXTENDED1:
  switch (*ip++) {

  case ASM16_STACK & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)ip);
   goto do_set_stack;
  case ASM16_STATIC & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)ip);
   goto do_set_static;
  case ASM16_EXTERN & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
#ifdef EXEC_SAFE
   if unlikely(imm_val >= code->co_module->mo_importc) {
err_invalid_extern16:
    frame->cf_sp = sp;
    err_srt_invalid_extern(frame,UNALIGNED_GETLE16((uint16_t *)(ip + 0)),imm_val);
    Dee_Decref(value);
    return -1;
   }
   module = code->co_module->mo_importv[imm_val];
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 2));
   if unlikely(imm_val >= module->mo_globalc)
      goto err_invalid_extern16;
#else
   ASSERT(imm_val < code->co_module->mo_importc);
   module = code->co_module->mo_importv[imm_val];
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 2));
   ASSERT(imm_val < module->mo_globalc);
#endif
   goto do_set_module_object;
  case ASM16_GLOBAL & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
   goto do_set_global;
  case ASM16_LOCAL & 0xff:
   imm_val = UNALIGNED_GETLE16((uint16_t *)(ip + 0));
   goto do_set_local;

#ifdef EXEC_SAFE
  default: goto ill_instr;
#else
  default: __builtin_unreachable();
#endif
  }
  break;

 default:
#ifdef EXEC_SAFE
ill_instr:
  Dee_Decref(value);
  err_illegal_instruction(code,frame->cf_ip);
  return -1;
#else
  __builtin_unreachable();
#endif
 }
 return 0;
}

#ifndef PRIVATE_OBJECT_CALL_VEC2_DEFINED
#define PRIVATE_OBJECT_CALL_VEC2_DEFINED 1
INTDEF DREF DeeObject *DCALL
object_call_vec2(DeeObject *__restrict func,
                 size_t argc1, DeeObject **__restrict vec1,
                 size_t argc2, DeeObject **__restrict vec2);
#endif /* !PRIVATE_OBJECT_CALL_VEC2_DEFINED */

#ifdef EXEC_FAST
PUBLIC DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameFast(struct code_frame *__restrict frame)
#else
PUBLIC DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameSafe(struct code_frame *__restrict frame)
#endif
{
#ifndef USE_SWITCH
#ifndef __INTELLISENSE__
#include "code-exec-targets.c.inl"
#endif
#endif
 register union {
     instruction_t *ptr;
     uint8_t  *u8;
     int8_t   *s8;
     uint16_t *u16;
     uint32_t *u32;
     int16_t  *s16;
     int32_t  *s32;
 } ip;
#define READ_imm8()   (*ip.u8++)
#define READ_Simm8()  (*ip.s8++)
#define READ_imm16()            UNALIGNED_GETLE16(ip.u16++)
#define READ_Simm16() ((int16_t)UNALIGNED_GETLE16(ip.u16++))
#define READ_imm32()            UNALIGNED_GETLE32(ip.u32++)
#define READ_Simm32() ((int32_t)UNALIGNED_GETLE32(ip.u32++))
 register DeeObject    **sp;
 register DeeCodeObject *code;
 register uint16_t imm_val;
 uint16_t imm_val2;
 DeeThreadObject *this_thread = DeeThread_Self();
 uint16_t except_recursion = this_thread->t_exceptsz;
#ifdef _MSC_VER
 /* MSVC is too dumb to take advantage of the C standard
  * and optimize away inner-scope variables that take the
  * address of a local variable, and have multiple such
  * scopes share the same memory location for said variable:
  * >> {
  * >>     int x = 4;
  * >>     printf("%p\n",&x);
  * >> }
  * >> {
  * >>     int y = 5;
  * >>     printf("%p\n",&y); // Regardless of optimization level, MSVC refuses to have
  * >>                        // `y' share the same memory location with `x', even though
  * >>                        // The C standard 100% allows a compiler to do this.
  * >> }
  * From what I understand, Microsoft actually has to do this due to some
  * crappy, but expensive and powerful programs that get compiled using
  * their compiler (*cough* Windows Kernel *cough*), which actually need
  * this behavior, presumably due to code like this (which any C compiler
  * with proper optimization wouldn't allow you to do):
  * >> PCHAR pMessage;
  * >> pMessage = LookupErrorMessage(dwErrorCode);
  * >> if (pMessage == NULL) {
  * >>     CHAR cBuffer[1024];
  * >>     FormatErrorMessage(cBuffer,"Unknown Error %X",dwErrorCode);
  * >>     pMessage = cBuffer; // WRONG!!! WRONG!!! WRONG!!! If you do this, you're an idiot
  * >> }
  * >> LogSystemError(pMessage);
  * Anyways. Since I can't change what has already been decided, and since this
  * ERROR (Yes, I'd call this an actual compiler Error) probably will never get
  * fixed because MS is way too stuck in its ways, the only thing we can do is
  * try to use the same variable for all the usage instances of `prefix_ob'
  */
 DREF DeeObject *prefix_ob;
#define USING_PREFIX_OBJECT  /* nothing */
#define NEED_UNIVERSAL_PREFIX_OB_WORKAROUND 1
#else
#define USING_PREFIX_OBJECT  DREF DeeObject *prefix_ob;
#endif


 ASSERTF(frame,"Invalid frame");
 ASSERT_OBJECT(frame->cf_func);
 code = frame->cf_func->fo_code;
 ASSERT_OBJECT(code);
 ASSERT((this_thread->t_execsz != 0) ==
        (this_thread->t_exec != NULL));

#ifdef CONFIG_HAVE_EXEC_ALTSTACK
#if (DEE_EXEC_ALTSTACK_PERIOD & (DEE_EXEC_ALTSTACK_PERIOD-1)) == 0
#define IS_ALTSTACK_PERIOD(x) (((x) & (DEE_EXEC_ALTSTACK_PERIOD-1)) == (DEE_EXEC_ALTSTACK_PERIOD-1))
#else
#define IS_ALTSTACK_PERIOD(x) (((x) % DEE_EXEC_ALTSTACK_PERIOD) == (DEE_EXEC_ALTSTACK_PERIOD-1))
#endif
#endif

 /* Limit `this_thread->t_execsz' and throw an
  * Error.RuntimeError.StackOverflow' if it exceeds that limit. */
 if unlikely(this_thread->t_execsz >= DeeExec_StackLimit) {
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
  if (IS_ALTSTACK_PERIOD(this_thread->t_execsz) &&
      this_thread->t_exec == frame)
      this_thread->t_exec = frame->cf_prev;
#endif
  DeeError_Throwf(&DeeError_StackOverflow,"Stack overflow");
  return NULL;
 }

#ifdef CONFIG_HAVE_EXEC_ALTSTACK
 if (IS_ALTSTACK_PERIOD(this_thread->t_execsz)) {
  if (this_thread->t_exec == frame)
      goto inc_execsz_start;
  frame->cf_prev = this_thread->t_exec;
  this_thread->t_exec = frame;
  /* Execute on to an alternate stack. */
#ifdef EXEC_SAFE
  return DeeCode_ExecFrameSafeAltStack(frame);
#else
  return DeeCode_ExecFrameFastAltStack(frame);
#endif
 } else
#endif
 {
#ifndef NDEBUG
  ASSERTF(frame->cf_prev == CODE_FRAME_NOT_EXECUTING,
          "Frame is already being executed");
#endif
  ASSERT(this_thread->t_exec != frame);
  /* Hook frame into the thread-local execution stack. */
  frame->cf_prev = this_thread->t_exec;
  this_thread->t_exec = frame;
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
inc_execsz_start:
#endif
  ++this_thread->t_execsz;
 }



#ifndef CONFIG_NO_THREADS
#ifdef EXEC_FAST
 /* Don't allow the compiler to move the frame-linking code below this point. */
 COMPILER_WRITE_BARRIER();

 if unlikely(code->co_flags&CODE_FASSEMBLY) {
  /* Highly unlikely case:
   *   There is a chance that our execution got suspended when
   *   another thread was setting the assembly flag just now.
   *   Yet whilst it was doing this, suspending us in the process,
   *   our execution path had just tested the assembly flag and
   *   seen that it wasn't set, deciding to go ahead and execute
   *   the code object in fast-mode.
   *   However in the time it took for us to register our code-frame,
   *   another set went ahead and set the assembly flag after failing
   *   to find any other thread actively executing said code object.
   *  (as usually indicated by the code object being apart of the
   *   frame-stack of another thread, which it hadn't yet become
   *   apart of in our thread).
   *   Therefor, now that the assembly flag has been set, we must
   *   remove our frame and switch to safe-mode. */
  this_thread->t_exec = frame->cf_prev;
  --this_thread->t_execsz;
  frame->cf_prev = CODE_FRAME_NOT_EXECUTING;
  return DeeCode_ExecFrameSafe(frame);
 }
#endif
#endif

 ip.ptr = frame->cf_ip;
 sp     = frame->cf_sp;
 ASSERT(ip.ptr >= code->co_code &&
        ip.ptr <  code->co_code+code->co_codebytes);

#define REFimm                frame->cf_func->fo_refv[imm_val]
#define LOCALimm              frame->cf_frame[imm_val]
#define EXTERNimm             code->co_module->mo_importv[imm_val]->mo_globalv[imm_val2]
#define GLOBALimm             code->co_module->mo_globalv[imm_val]
#define STATICimm             code->co_staticv[imm_val]
#define CONSTimm              code->co_staticv[imm_val]
#define CONSTimm2             code->co_staticv[imm_val2]
#ifdef CONFIG_NO_THREADS
#define EXTERN_LOCKREAD()       (void)0
#define EXTERN_LOCKENDREAD()    (void)0
#define EXTERN_LOCKWRITE()      (void)0
#define EXTERN_LOCKENDWRITE()   (void)0
#define GLOBAL_LOCKREAD()       (void)0
#define GLOBAL_LOCKENDREAD()    (void)0
#define GLOBAL_LOCKWRITE()      (void)0
#define GLOBAL_LOCKENDWRITE()   (void)0
#define STATIC_LOCKREAD()       (void)0
#define STATIC_LOCKENDREAD()    (void)0
#define STATIC_LOCKWRITE()      (void)0
#define STATIC_LOCKENDWRITE()   (void)0
#else
#define EXTERN_LOCKREAD()       rwlock_read(&code->co_module->mo_importv[imm_val]->mo_lock)
#define EXTERN_LOCKENDREAD()    rwlock_endread(&code->co_module->mo_importv[imm_val]->mo_lock)
#define EXTERN_LOCKWRITE()      rwlock_write(&code->co_module->mo_importv[imm_val]->mo_lock)
#define EXTERN_LOCKENDWRITE()   rwlock_endwrite(&code->co_module->mo_importv[imm_val]->mo_lock)
#define GLOBAL_LOCKREAD()       rwlock_read(&code->co_module->mo_lock)
#define GLOBAL_LOCKENDREAD()    rwlock_endread(&code->co_module->mo_lock)
#define GLOBAL_LOCKWRITE()      rwlock_write(&code->co_module->mo_lock)
#define GLOBAL_LOCKENDWRITE()   rwlock_endwrite(&code->co_module->mo_lock)
#define STATIC_LOCKREAD()       rwlock_read(&code->co_static_lock)
#define STATIC_LOCKENDREAD()    rwlock_endread(&code->co_static_lock)
#define STATIC_LOCKWRITE()      rwlock_write(&code->co_static_lock)
#define STATIC_LOCKENDWRITE()   rwlock_endwrite(&code->co_static_lock)
#endif
#define THIS                 (frame->cf_this)
#define TOP                   sp[-1]
#define FIRST                 sp[-1]
#define SECOND                sp[-2]
#define THIRD                 sp[-3]
#define FOURTH                sp[-4]
#define POP()             (*--sp)
#define POPREF()           (--sp,Dee_Decref(*sp))
#define PUSH(ob)            (*sp = (ob),++sp)
#define PUSHREF(ob)         (*sp = (ob),Dee_Incref(*sp),++sp)
#define STACK_BEGIN           frame->cf_stack
#define STACK_END            (frame->cf_stack+code->co_framesize)
#define STACKUSED            (sp-frame->cf_stack)
#define STACKPREALLOC       ((uint16_t)((code->co_framesize/sizeof(DeeObject *))-code->co_localc))
#ifdef USE_SWITCH
#define RAW_TARGET2(op,_op)   case op&0xff: target##_op:
#else
#define RAW_TARGET2(op,_op)                 target##_op:
#endif
#define RAW_TARGET(op)        RAW_TARGET2(op,_##op)
#define EXCEPTION_CLEANUP     /* nothing */
#ifdef EXEC_SAFE
#define ASSERT_TUPLE(ob)     do{ if unlikely(!DeeTuple_CheckExact(ob)) { EXCEPTION_CLEANUP goto err_requires_tuple;} }__WHILE0
#define ASSERT_STRING(ob)    do{ if unlikely(!DeeString_CheckExact(ob)) { EXCEPTION_CLEANUP goto err_requires_string;} }__WHILE0
#define ASSERT_THISCALL()    do{ if unlikely(!(code->co_flags&CODE_FTHISCALL)) { EXCEPTION_CLEANUP goto err_requires_thiscall_code; } }__WHILE0
#define CONST_LOCKREAD()     STATIC_LOCKREAD()
#define CONST_LOCKENDREAD()  STATIC_LOCKENDREAD()
#define CONST_LOCKWRITE()    STATIC_LOCKWRITE()
#define CONST_LOCKENDWRITE() STATIC_LOCKENDWRITE()
#define ASSERT_REFimm()      do{ if unlikely(imm_val >= code->co_refc) { EXCEPTION_CLEANUP goto err_invalid_ref; } }__WHILE0
#define ASSERT_EXTERNimm()   do{ if unlikely(imm_val >= code->co_module->mo_importc || imm_val2 >= code->co_module->mo_importv[imm_val]->mo_globalc) { EXCEPTION_CLEANUP goto err_invalid_extern; } }__WHILE0
#define ASSERT_GLOBALimm()   do{ if unlikely(imm_val >= code->co_module->mo_globalc) { EXCEPTION_CLEANUP goto err_invalid_global; } }__WHILE0
#define ASSERT_LOCALimm()    do{ if unlikely(imm_val >= code->co_localc) { EXCEPTION_CLEANUP goto err_invalid_locale; } }__WHILE0
#define ASSERT_STATICimm()   do{ if unlikely(imm_val >= code->co_staticc) { EXCEPTION_CLEANUP goto err_invalid_static; } }__WHILE0
#define ASSERT_CONSTimm()    do{ if unlikely(imm_val >= code->co_staticc) { EXCEPTION_CLEANUP goto err_invalid_const; } }__WHILE0
#define ASSERT_CONSTimm2()   do{ if unlikely(imm_val2 >= code->co_staticc) { EXCEPTION_CLEANUP imm_val2 = imm_val; goto err_invalid_const; } }__WHILE0
#define ASSERT_YIELDING()    do{ if unlikely(!(code->co_flags&CODE_FYIELDING)) { EXCEPTION_CLEANUP goto err_requires_yield_code; } }__WHILE0
#define STACKFREE           ((frame->cf_stack+(frame->cf_stacksz ? frame->cf_stacksz : STACKPREALLOC))-sp)
#define STACKSIZE            (frame->cf_stacksz ? frame->cf_stacksz : STACKPREALLOC)
#define ASSERT_USAGE(sp_sub,sp_add) \
     if unlikely((sp_sub) != 0 && ((-(sp_sub)) > STACKUSED)) { EXCEPTION_CLEANUP goto err_invalid_stack_affect; } \
     if unlikely(((sp_sub)+(sp_add)) != 0 && (((sp_sub)+(sp_add)) > STACKFREE)) { EXCEPTION_CLEANUP goto increase_stacksize; }
#else
#define ASSERT_TUPLE(ob)      ASSERT(DeeTuple_CheckExact(ob))
#define ASSERT_STRING(ob)     ASSERT(DeeString_CheckExact(ob))
#define ASSERT_THISCALL()     ASSERT(code->co_flags&CODE_FTHISCALL)
#define CONST_LOCKREAD()     (void)0
#define CONST_LOCKENDREAD()  (void)0
#define CONST_LOCKWRITE()    (void)0
#define CONST_LOCKENDWRITE() (void)0
#define ASSERT_REFimm()       ASSERT(imm_val < code->co_refc)
#define ASSERT_EXTERNimm()    ASSERT(imm_val < code->co_module->mo_importc && imm_val2 < code->co_module->mo_importv[imm_val]->mo_globalc)
#define ASSERT_GLOBALimm()    ASSERT(imm_val < code->co_module->mo_globalc)
#define ASSERT_LOCALimm()     ASSERT(imm_val < code->co_localc)
#define ASSERT_STATICimm()    ASSERT(imm_val < code->co_staticc)
#define ASSERT_CONSTimm()     ASSERT(imm_val < code->co_staticc)
#define ASSERT_CONSTimm2()    ASSERT(imm_val2 < code->co_staticc)
#define ASSERT_YIELDING()     ASSERT(code->co_flags&CODE_FYIELDING)
#define STACKFREE           ((frame->cf_stack+STACKPREALLOC)-sp)
#define STACKSIZE            (STACKPREALLOC)
#define ASSERT_USAGE(sp_sub,sp_add) \
     ASSERT(!(sp_sub) || (-(sp_sub)) <= STACKUSED); \
     ASSERT(!((sp_sub)+(sp_add)) || ((sp_sub)+(sp_add)) <= STACKFREE);
#endif
#define TARGET(op,sp_sub,sp_add) RAW_TARGET2(op,_##op) ASSERT_USAGE(sp_sub,sp_add) __IF0; else
#define TARGETSimm16(op,sp_sub,sp_add)        \
         RAW_TARGET(op##16)    imm_val = (uint16_t)READ_Simm16(); \
 __IF0 { RAW_TARGET2(op,_##op) imm_val = (uint16_t)(int16_t)READ_Simm8(); } \
         ASSERT_USAGE(sp_sub,sp_add) \
 __IF0; else

#define REPEAT_INSTRUCTION() (ip.ptr = frame->cf_ip)
#define HANDLE_EXCEPT()       goto handle_except
#define DISPATCH()            goto next_instr
#define YIELD_RESULT()        do{ ASSERT(code->co_flags&CODE_FYIELDING); goto end_without_finally; }__WHILE0
#define RETURN_RESULT()       do{ ASSERT(!(code->co_flags&CODE_FYIELDING)); goto end_return; }__WHILE0
#define YIELD(val)            do{ frame->cf_result = (val); YIELD_RESULT(); }__WHILE0
#define RETURN(val)           do{ frame->cf_result = (val); RETURN_RESULT(); }__WHILE0
#ifndef __OPTIMIZE_SIZE__
#define PREDICT(opcode)       do{ if (*ip.ptr == (opcode)) { ++ip.ptr; goto target_##opcode; } }__WHILE0
#else
#define PREDICT(opcode)       do{}__WHILE0
#endif


next_instr:
#if 0
 DEE_CHECKMEMORY();
#endif
#if 0
 {
  struct ddi_regs start,stop;
  code_addr_t ip_addr = ip.ptr - code->co_code;
  printf("ip %.4X: ",(unsigned)ip_addr);
  if (!DeeDDI_FindIP((DeeObject *)code->co_ddi,&start,&stop,ip_addr))
       printf("No information\n");
  else {
   ASSERT(ip_addr < stop.dr_uip);
   ASSERT(ip_addr >= start.dr_uip);
   ASSERT(start.dr_uip < stop.dr_uip);
   printf("%.4X...%.4X - line %d, col %d, path `%s', file `%s' (SP: %u/%u)\n",
         (unsigned)start.dr_uip,(unsigned)stop.dr_uip,
          start.dr_lno+1,start.dr_col+1,
         (start.dr_path && start.dr_path-1 < code->co_ddi->d_paths) ?
          DeeDDI_PATH_NAME(code->co_ddi,start.dr_path-1) : "",
          start.dr_file < code->co_ddi->d_files ?
          DeeDDI_FILE_NAME(code->co_ddi,start.dr_file) : "",
         (unsigned)start.dr_usp,(unsigned)(sp - frame->cf_stack));
   if (start.dr_uip == ip_addr)
       ASSERT(start.dr_usp == (unsigned)(sp - frame->cf_stack));
  }
 }
#endif
 frame->cf_ip = ip.ptr;
#ifdef USE_SWITCH
 switch (*ip.ptr++)
#else
 goto *basic_targets[*ip.ptr++];
#endif
 {

 TARGET(ASM_RET_NONE,-0,+0) {
     if (ITER_ISOK(frame->cf_result))
         Dee_Decref(frame->cf_result);
     if (code->co_flags&CODE_FYIELDING) {
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

 TARGET(ASM_RET,-1,+0) {
     /* Check if we're overwriting a previous return value
      * (which can happen when `return' appears in a finally-block) */
     if (ITER_ISOK(frame->cf_result))
         Dee_Decref(frame->cf_result);
     frame->cf_result = POP();
     if (code->co_flags&CODE_FYIELDING)
         goto end_without_finally;
     goto end_return;
 }

 TARGET(ASM_YIELDALL,-1,+0) {
     ASSERT_YIELDING();
     if (ITER_ISOK(frame->cf_result))
         Dee_Decref(frame->cf_result);
     frame->cf_result = DeeObject_IterNext(TOP);
     if unlikely(!frame->cf_result) HANDLE_EXCEPT();
     if (frame->cf_result != ITER_DONE) {
      /* Repeat this instruction and forward the value we've just read. */
      REPEAT_INSTRUCTION();
      YIELD_RESULT();
     }
     /* Pop the iterator that was enumerated. */
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_THROW,-1,+0) {
     DeeError_Throw(TOP);
     POPREF();
     HANDLE_EXCEPT();
 }

 TARGET(ASM_RETHROW,-0,+0) {
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

 TARGET(ASM_ENDCATCH,-0,+0) {
     ASSERT(except_recursion <= this_thread->t_exceptsz);
     /* Handle errors if we've caused any.
      * NOTE: We do allow the handling of interrupt signal here,
      *       so-as to comply with the intended usage-case within
      *       interrupt exception handlers. */
     if (except_recursion != this_thread->t_exceptsz)
         DeeError_Handled(ERROR_HANDLED_INTERRUPT);
     DISPATCH();
 }

 TARGET(ASM_ENDFINALLY,-0,+0) {
     /* If a return value has been assigned, stop execution. */
     if (frame->cf_result != NULL)
         goto end_return;
     /* Check for errors. */
     if (except_recursion != this_thread->t_exceptsz)
         HANDLE_EXCEPT();
     DISPATCH();
 }

 TARGET(ASM_PUSH_BND_EXTERN,-0,+1) {
     imm_val  = READ_imm8();
     imm_val2 = READ_imm8();
do_push_bnd_extern:
     ASSERT_EXTERNimm();
     /*EXTERN_LOCKREAD();*/
     PUSHREF(DeeBool_For(EXTERNimm != NULL));
     /*EXTERN_LOCKENDREAD();*/
     DISPATCH();
 }

 TARGET(ASM_PUSH_BND_GLOBAL,-0,+1) {
     imm_val = READ_imm8();
do_push_bnd_global:
     ASSERT_GLOBALimm();
     /*GLOBAL_LOCKREAD();*/
     PUSHREF(DeeBool_For(GLOBALimm != NULL));
     /*GLOBAL_LOCKENDREAD();*/
     DISPATCH();
 }

 TARGET(ASM_PUSH_BND_LOCAL,-0,+1) {
     imm_val = READ_imm8();
do_push_bnd_local:
     ASSERT_LOCALimm();
     PUSHREF(DeeBool_For(LOCALimm != NULL));
     DISPATCH();
 }

 TARGETSimm16(ASM_JF,-1,+0) {
     /* Conditionally jump if true. */
     int temp = DeeObject_Bool(TOP);
     if unlikely(temp < 0) HANDLE_EXCEPT();
     POPREF();
     if (!temp) {
jump_16:
#ifndef CONFIG_NO_THREADS
      if ((int16_t)imm_val < 0 &&
           DeeThread_CheckInterruptSelf(this_thread))
           HANDLE_EXCEPT();
#endif
      ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
      goto assert_ip_bounds;
#else
      ASSERT(ip.ptr >= code->co_code);
      ASSERT(ip.ptr <  code->co_code+code->co_codebytes);
#endif
     }
     DISPATCH();
 }

 TARGETSimm16(ASM_JT,-1,+0) {
     /* Conditionally jump if false. */
     int temp = DeeObject_Bool(TOP);
     if unlikely(temp < 0) HANDLE_EXCEPT();
     POPREF();
     if (temp) goto jump_16;
     DISPATCH();
 }

 TARGETSimm16(ASM_JMP,-0,+0) {
#ifndef CONFIG_NO_THREADS
     if ((int16_t)imm_val < 0 &&
          DeeThread_CheckInterruptSelf(this_thread))
          HANDLE_EXCEPT();
#endif
     /* Adjust the instruction pointer accordingly. */
     ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
assert_ip_bounds:
     /* Raise an error if the new IP has been displaced out-of-bounds. */
     if unlikely(ip.ptr <  code->co_code ||
                 ip.ptr >= code->co_code+code->co_codebytes)
        goto err_invalid_ip;
#else
     ASSERT(ip.ptr >= code->co_code);
     ASSERT(ip.ptr <  code->co_code+code->co_codebytes);
#endif
     DISPATCH();
 }

 TARGETSimm16(ASM_FOREACH,-1,+2) {
     DREF DeeObject *elem;
     elem = DeeObject_IterNext(TOP);
     if unlikely(!elem) HANDLE_EXCEPT();
     if (elem == ITER_DONE) {
      /* Pop the iterator and Jump if it finished. */
      POPREF();
      goto jump_16;
     }
     /* Leave the iterator and push the element. */
     PUSH(elem);
     DISPATCH();
 }

 TARGET(ASM_JMP_POP,-1,+0) {
     code_addr_t absip;
     instruction_t *new_ip;
     if (DeeObject_AsUInt32(TOP,&absip)) HANDLE_EXCEPT();
     POPREF();
#ifdef EXEC_SAFE
     if (absip >= code->co_codebytes) {
      DeeError_Throwf(&DeeError_SegFault,
                      "Invalid IP %.4I32X in absolute jmp",
                      absip);
      HANDLE_EXCEPT();
     }
#else
     ASSERTF(absip < code->co_codebytes,"Invalid IP: %X",
            (unsigned int)absip);
#endif
     new_ip = code->co_code + absip;
#ifndef CONFIG_NO_THREADS
     if (new_ip < ip.ptr &&
         DeeThread_CheckInterruptSelf(this_thread))
         HANDLE_EXCEPT();
#endif
     ip.ptr = new_ip;
     DISPATCH();
 }

 RAW_TARGET(ASM_CALL) {
     uint8_t n_args = READ_imm8();
     DREF DeeObject *call_result,**new_sp;
     ASSERT_USAGE(-1-(int)n_args,+1);
     /* NOTE: Inherit references. */
     new_sp = sp-n_args;
     call_result = DeeObject_Call(new_sp[-1],n_args,new_sp);
     if unlikely(!call_result) HANDLE_EXCEPT();
     while (n_args--) POPREF();
     Dee_Decref(TOP); /* Drop a reference from the called function. */
     TOP = call_result; /* Save the frame->cf_result of the call back on the stack. */
     DISPATCH();
 }

 RAW_TARGET(ASM_CALL_KW) {
     DREF DeeObject *call_result,**new_sp;
     imm_val2 = READ_imm8();
     imm_val = READ_imm8();
do_call_kw:
     ASSERT_USAGE(-1-(int)imm_val2,+1);
     /* NOTE: Inherit references. */
     new_sp = sp-imm_val2;
     ASSERT_CONSTimm();
#ifdef EXEC_SAFE
     {
      DREF DeeObject *kwds;
      CONST_LOCKREAD();
      kwds = CONSTimm;
      Dee_Incref(kwds);
      CONST_LOCKENDREAD();
      call_result = DeeObject_CallKw(new_sp[-1],imm_val2,new_sp,kwds);
      Dee_Decref(kwds);
     }
#else
     call_result = DeeObject_CallKw(new_sp[-1],imm_val2,new_sp,CONSTimm);
#endif
     if unlikely(!call_result) HANDLE_EXCEPT();
     while (imm_val2--) POPREF();
     Dee_Decref(TOP); /* Drop a reference from the called function. */
     TOP = call_result; /* Save the frame->cf_result of the call back on the stack. */
     DISPATCH();
 }

 RAW_TARGET(ASM_CALL_TUPLE_KW) {
     DREF DeeObject *call_result;
     imm_val = READ_imm8();
do_call_tuple_kw:
     ASSERT_USAGE(-2,+1);
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
      call_result = DeeObject_CallTupleKw(SECOND,FIRST,kwds);
      Dee_Decref(kwds);
     }
#else
     call_result = DeeObject_CallTupleKw(SECOND,FIRST,CONSTimm);
#endif
     if unlikely(!call_result) HANDLE_EXCEPT();
     POPREF();          /* Pop the argument tuple. */
     Dee_Decref(TOP);   /* Drop a reference from the called function. */
     TOP = call_result; /* Save the frame->cf_result of the call back on the stack. */
     DISPATCH();
 }

 TARGET(ASM_CALL_TUPLE,-2,+1) {
     DREF DeeObject *temp;
     ASSERT_TUPLE(FIRST);
     temp = DeeObject_CallTuple(SECOND,FIRST);
     if unlikely(!temp) HANDLE_EXCEPT();
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
     ASSERT_USAGE(-1-(int)imm_val2,+1);
     /* NOTE: Inherit references. */
     call_result = DeeObject_InvokeOperator((sp - imm_val2)[-1],imm_val,
                                           (size_t)imm_val2,
                                            sp - imm_val2);
     if unlikely(!call_result) HANDLE_EXCEPT();
     while (imm_val2--) POPREF();
     Dee_Decref(TOP); /* Drop a reference from the operator self-argument. */
     TOP = call_result; /* Save the result of the call back on the stack. */
     DISPATCH();
 }

 TARGET(ASM_OPERATOR_TUPLE,-2,+1) {
     DREF DeeObject *call_result;
     imm_val = READ_imm8();
do_operator_tuple:
     ASSERT_TUPLE(FIRST);
     call_result = DeeObject_InvokeOperator(SECOND,(unsigned int)imm_val,
                                            DeeTuple_SIZE(FIRST),
                                            DeeTuple_ELEM(FIRST));
     if unlikely(!call_result) HANDLE_EXCEPT();
     POPREF(); /* Pop the argument tuple. */
     Dee_Decref(TOP); /* Drop a reference from operator self-argument. */
     TOP = call_result; /* Save the result of the call back on the stack. */
     DISPATCH();
 }


 TARGET(ASM_DEL_GLOBAL,-0,+0) {
     DeeObject **pobject,*del_object;
     imm_val = READ_imm8();
do_del_global:
     ASSERT_GLOBALimm();
     GLOBAL_LOCKWRITE();
     pobject = &GLOBALimm;
     del_object = *pobject;
     *pobject = NULL;
     GLOBAL_LOCKENDWRITE();
     if unlikely(!del_object)
        goto err_unbound_global;
     Dee_Decref(del_object);
     DISPATCH();
 }

 TARGET(ASM_DEL_LOCAL,-0,+0) {
     DeeObject **plocal;
     imm_val = READ_imm8();
do_del_local:
     ASSERT_LOCALimm();
     plocal = &LOCALimm;
     if unlikely(!*plocal)
        goto err_unbound_local;
     Dee_Clear(*plocal);
     DISPATCH();
 }

 TARGET(ASM_SWAP,-2,+2) {
     DREF DeeObject *temp;
     temp   = SECOND;
     SECOND = FIRST;
     FIRST  = temp;
     DISPATCH();
 }

 RAW_TARGET(ASM_LROT) {
     DREF DeeObject *temp;
     uint16_t shift = (uint16_t)(READ_imm8()+3);
     ASSERT_USAGE(-(int)shift,+(int)shift);
     temp = *(sp-shift);
     MEMMOVE_PTR(sp-shift,sp-(shift-1),shift-1);
     sp[-1] = temp;
     DISPATCH();
 }

 RAW_TARGET(ASM_RROT) {
     DREF DeeObject *temp;
     uint16_t shift = (uint16_t)(READ_imm8()+3);
     ASSERT_USAGE(-(int)shift,+(int)shift);
     temp = sp[-1];
     MEMMOVE_PTR(sp-(shift-1),sp-shift,shift-1);
     *(sp-shift) = temp;
     DISPATCH();
 }

 TARGET(ASM_DUP,-1,+2) {
     PUSHREF(TOP);
     DISPATCH();
 }

 RAW_TARGET(ASM_DUP_N) {
     uint8_t offset = READ_imm8();
     DREF DeeObject **pslot;
     ASSERT_USAGE(-((int)offset+2),+((int)offset+3));
     pslot = sp - (offset+2);
     PUSHREF(*pslot);
     DISPATCH();
 }

 TARGET(ASM_POP,-1,+0) {
     POPREF();
     DISPATCH();
 }

 RAW_TARGET(ASM_POP_N) {
     DREF DeeObject *old_object;
     DREF DeeObject **pslot;
     uint8_t offset = READ_imm8();
     ASSERT_USAGE(-((int)offset+2),+((int)offset+1));
     pslot = sp - (offset+2);
     old_object = *pslot;
     *pslot = POP();
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
#else
      ASSERT(-(int16_t)imm_val <= STACKUSED);
#endif
      while (imm_val++) POPREF();
     } else {
#ifdef EXEC_SAFE
      if unlikely((int16_t)imm_val > STACKFREE)
         goto increase_stacksize;
#else
      ASSERT((int16_t)imm_val <= STACKFREE);
#endif
      while (imm_val--) PUSHREF(Dee_None);
     }
     /* A stack adjustment is often followed by a
      * decently-sized (though usually still 8-bit) jump.
      * Therefor, we predict that a jump instruction follows. */
     PREDICT(ASM_JMP);
     DISPATCH();
 }

 TARGET(ASM_SUPER,-2,+1) {
     DREF DeeObject *super_wrapper;
     super_wrapper = DeeSuper_New((DeeTypeObject *)FIRST,SECOND);
     if unlikely(!super_wrapper)
        HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = super_wrapper; /* Inherit reference. */
     DISPATCH();
 }
 TARGET(ASM_SUPER_THIS_R,-0,+1) {
     DREF DeeObject *super_wrapper;
     imm_val = READ_imm8();
do_super_this_r:
     ASSERT_THISCALL();
     ASSERT_REFimm();
     super_wrapper = DeeSuper_New((DeeTypeObject *)REFimm,THIS);
     if unlikely(!super_wrapper)
        HANDLE_EXCEPT();
     PUSH(super_wrapper); /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_ISNONE,-1,+1) {
     Dee_Decref(TOP); /* Can already decref() because we only need to check the pointer. */
     TOP = DeeBool_For(TOP == Dee_None);
     Dee_Incref(TOP);
     DISPATCH();
 }

 TARGET(ASM_POP_STATIC,-1,+0) {
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

 TARGET(ASM_POP_EXTERN,-1,+0) {
     DeeObject *old_value,**pglobl;
     imm_val  = READ_imm8();
     imm_val2 = READ_imm8();
do_pop_extern:
     ASSERT_EXTERNimm();
     EXTERN_LOCKWRITE();
     pglobl    = &EXTERNimm;
     old_value = *pglobl;
     *pglobl   = POP();
     EXTERN_LOCKENDWRITE();
     Dee_XDecref(old_value);
     DISPATCH();
 }

 TARGET(ASM_POP_GLOBAL,-1,+0) {
     DeeObject *old_value,**pglobl;
     imm_val = READ_imm8();
do_pop_global:
     ASSERT_GLOBALimm();
     GLOBAL_LOCKWRITE();
     pglobl    = &GLOBALimm;
     old_value = *pglobl;
     *pglobl   = POP();
     GLOBAL_LOCKENDWRITE();
     Dee_XDecref(old_value);
     DISPATCH();
 }

 TARGET(ASM_POP_LOCAL,-1,+0) {
     DeeObject *old_value;
     imm_val = READ_imm8();
do_pop_local:
     ASSERT_LOCALimm();
     old_value = LOCALimm;
     LOCALimm  = POP();
     Dee_XDecref(old_value);
     DISPATCH();
 }

 TARGET(ASM_PUSH_REF,-0,+1) {
     imm_val = READ_imm8();
do_push_ref:
     ASSERT_REFimm();
     PUSHREF(REFimm);
     DISPATCH();
 }

 TARGET(ASM_PUSH_ARG,-0,+1) {
     imm_val = READ_imm8();
do_push_arg:
     ASSERT(code->co_argc_max >= code->co_argc_min);
     if (imm_val < code->co_argc_max) {
      /* Simple case: Direct argument/default reference. */
      if (imm_val < frame->cf_argc) {
       PUSHREF(frame->cf_argv[imm_val]);
      } else {
       /* TODO: Keyword argument support? */
       ASSERT(imm_val >= code->co_argc_min);
       PUSHREF(code->co_defaultv[imm_val-code->co_argc_min]);
      }
     } else {
      /* Special case: Varargs. */
#ifdef EXEC_SAFE
      if (!(code->co_flags&CODE_FVARARGS))
          goto err_invalid_argument_index;
      if (imm_val != code->co_argc_max)
          goto err_invalid_argument_index;
#else
      ASSERT(code->co_flags&CODE_FVARARGS);
      ASSERTF(imm_val == code->co_argc_max,
              "out-of-bounds argument index");
#endif
      ASSERT(frame->cf_argc >= code->co_argc_min);
      if (!frame->cf_vargs) {
       if (frame->cf_argc <= code->co_argc_max) {
        frame->cf_vargs = (DREF DeeTupleObject *)Dee_EmptyTuple;
        Dee_Incref(Dee_EmptyTuple);
       } else {
        frame->cf_vargs = (DREF DeeTupleObject *)
         DeeTuple_NewVector((size_t)(frame->cf_argc-code->co_argc_max),
                                     frame->cf_argv+code->co_argc_max);
        if unlikely(!frame->cf_vargs) HANDLE_EXCEPT();
       }
      }
      PUSHREF((DeeObject *)frame->cf_vargs);
     }
     DISPATCH();
 }

 TARGET(ASM_PUSH_CONST,-0,+1) {
     imm_val = READ_imm8();
do_push_const:
     ASSERT_CONSTimm();
     CONST_LOCKREAD();
     PUSHREF(CONSTimm);
     CONST_LOCKENDREAD();
     DISPATCH();
 }

 TARGET(ASM_PUSH_STATIC,-0,+1) {
     imm_val = READ_imm8();
do_push_static:
     ASSERT_STATICimm();
     STATIC_LOCKREAD();
     PUSHREF(STATICimm);
     STATIC_LOCKENDREAD();
     DISPATCH();
 }

 TARGET(ASM_PUSH_EXTERN,-0,+1) {
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

 TARGET(ASM_PUSH_GLOBAL,-0,+1) {
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

 TARGET(ASM_PUSH_LOCAL,-0,+1) {
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
     ASSERT_USAGE(-(int)imm_val,+1);
     temp = DeeTuple_NewVectorSymbolic(imm_val,sp-imm_val); /* Inherit references. */
     if unlikely(!temp) HANDLE_EXCEPT();
     sp -= imm_val;
     PUSH(temp);
     DISPATCH();
 }

 RAW_TARGET(ASM_PACK_LIST) {
     DREF DeeObject *temp;
     imm_val = READ_imm8();
do_pack_list:
     ASSERT_USAGE(-(int)imm_val,+1);
     temp = DeeList_NewVectorInherited(imm_val,sp-imm_val); /* Inherit references. */
     if unlikely(!temp) HANDLE_EXCEPT();
     sp -= imm_val;
     PUSH(temp);
     DISPATCH();
 }

 RAW_TARGET(ASM_UNPACK) {
     int error;
     DREF DeeObject *sequence;
     imm_val = READ_imm8();
do_unpack:
     ASSERT_USAGE(-1,+(int)imm_val);
     sequence = POP();
     error = DeeObject_Unpack(sequence,imm_val,sp);
     Dee_Decref(sequence);
     if unlikely(error) HANDLE_EXCEPT();
     sp += imm_val;
     DISPATCH();
 }

 TARGET(ASM_CAST_TUPLE,-1,+1) {
     DREF DeeObject *temp;
     temp = DeeTuple_FromSequence(TOP);
     if unlikely(!temp) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = temp; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_CAST_LIST,-1,+1) {
     DREF DeeObject *temp;
     temp = DeeList_FromSequence(TOP);
     if unlikely(!temp) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = temp; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_PUSH_NONE,-0,+1) {
     Dee_Incref(Dee_None);
     PUSH(Dee_None);
     DISPATCH();
 }

 TARGET(ASM_PUSH_MODULE,-0,+1) {
     DeeModuleObject *mod;
     imm_val = READ_imm8();
do_push_module:
     mod = code->co_module;
     ASSERT_OBJECT(mod);
#ifdef EXEC_SAFE
     if (imm_val >= mod->mo_importc)
         goto err_invalid_module;
#else
     ASSERT(imm_val < mod->mo_importc);
#endif
     mod = mod->mo_importv[imm_val];
     ASSERT_OBJECT(mod);
     PUSHREF((DeeObject *)mod);
     DISPATCH();
 }

 TARGET(ASM_CONCAT,-2,+1) {
     DREF DeeObject *temp;
     temp = DeeObject_ConcatInherited(SECOND,FIRST);
     if unlikely(!temp) HANDLE_EXCEPT();
     SECOND = temp;
     POPREF();
     DISPATCH();
 }

 RAW_TARGET(ASM_EXTEND) {
     DREF DeeObject **new_sp;
     uint8_t n_args = READ_imm8();
     DREF DeeObject *temp;
     ASSERT_USAGE(-((int)n_args+1),+1);
     new_sp = sp-n_args;
     temp = DeeObject_ExtendInherited(new_sp[-1],n_args,new_sp);
     if unlikely(!temp) HANDLE_EXCEPT();
     sp  = new_sp;
     TOP = temp;
     DISPATCH();
 }

 TARGET(ASM_TYPEOF,-1,+1) {
     DeeTypeObject *typ;
     typ = Dee_TYPE(TOP);
     Dee_Incref(typ);
     Dee_Decref(TOP);
     TOP = (DREF DeeObject *)typ; /* Inherit object. */
     DISPATCH();
 }

 TARGET(ASM_CLASSOF,-1,+1) {
     DeeTypeObject *typ;
     typ = DeeObject_Class(TOP);
     Dee_Incref(typ);
     Dee_Decref(TOP);
     TOP = (DREF DeeObject *)typ; /* Inherit object. */
     DISPATCH();
 }

 TARGET(ASM_SUPEROF,-1,+1) {
     DREF DeeObject *temp;
     temp = DeeSuper_Of(TOP);
     if unlikely(!temp) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = temp; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_INSTANCEOF,-2,+1) {
     bool is_instance;
     /* Special case: The deemon specs allow `none' to be
      *               written as the second argument to `is' */
     if (DeeNone_Check(FIRST)) {
      is_instance = DeeNone_Check(SECOND);
     } else if (DeeSuper_Check(SECOND)) {
      is_instance = DeeType_IsInherited(DeeSuper_TYPE(SECOND),(DeeTypeObject *)FIRST);
     } else {
      is_instance = DeeObject_InstanceOf(SECOND,(DeeTypeObject *)FIRST);
     }
     POPREF();
     Dee_Decref(TOP);
     TOP = DeeBool_For(is_instance);
     Dee_Incref(TOP);
     DISPATCH();
 }

 TARGET(ASM_STR,-1,+1) {
     DREF DeeObject *temp;
     temp = DeeObject_Str(TOP);
     if unlikely(!temp) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = temp; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_REPR,-1,+1) {
     DREF DeeObject *temp;
#if 1
     if (*ip.u8 == ASM_SHL) {
      DeeTypeObject *tp_temp = Dee_TYPE(SECOND);
      for (;;) {
       DREF DeeObject *other;
       DREF DeeObject *(DCALL *tp_shl)(DeeObject *__restrict,DeeObject *__restrict);
       if (!tp_temp->tp_math ||
           (tp_shl = tp_temp->tp_math->tp_shl) == NULL) {
        tp_temp = tp_temp->tp_base;
        if (!tp_temp) break;
        continue;
       }
       ++ip.u8;
       if (tp_shl == &file_shl) {
        /* Special case: `fp << repr foo'
         * In this case, we can a special optimization
         * to directly print the repr to the file. */
        if (DeeObject_PrintRepr(TOP,(dformatprinter)&DeeFile_WriteAll,SECOND) < 0)
            HANDLE_EXCEPT();
        POPREF();
        DISPATCH();
       }
       temp = DeeObject_Repr(TOP);
       if unlikely(!temp) HANDLE_EXCEPT();
       POPREF();
       other = (*tp_shl)(TOP,temp);
       Dee_Decref(temp);
       if unlikely(!other)
          HANDLE_EXCEPT();
       Dee_Decref(TOP);
       TOP = other;
       DISPATCH();
      }
     }
#endif
     temp = DeeObject_Repr(TOP);
     if unlikely(!temp) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = temp; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_BOOL,-1,+1) {
     int boolval = DeeObject_Bool(TOP);
     if unlikely(boolval < 0) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = DeeBool_For(boolval);
     Dee_Incref(TOP);
     DISPATCH();
 }

 TARGET(ASM_NOT,-1,+1) {
     int boolval = DeeObject_Bool(TOP);
     if unlikely(boolval < 0) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = DeeBool_For(!boolval);
     Dee_Incref(TOP);
     DISPATCH();
 }

 TARGET(ASM_ASSIGN,-2,+1) {
     if unlikely(DeeObject_Assign(SECOND,TOP))
        HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_MOVE_ASSIGN,-2,+1) {
     if unlikely(DeeObject_MoveAssign(SECOND,TOP))
        HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_COPY,-1,+1) {
     DREF DeeObject *temp;
     temp = DeeObject_Copy(TOP);
     if unlikely(!temp) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = temp;
     DISPATCH();
 }

 TARGET(ASM_DEEPCOPY,-1,+1) {
     DREF DeeObject *temp;
     temp = DeeObject_DeepCopy(TOP);
     if unlikely(!temp) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = temp;
     DISPATCH();
 }

#define DEFINE_COMPARE_INSTR(EQ,Eq) \
 TARGET(ASM_CMP_##EQ,-2,+1) \
 { \
     DREF DeeObject *temp; \
     temp = DeeObject_Compare##Eq##Object(SECOND,FIRST); \
     if unlikely(!temp) HANDLE_EXCEPT(); \
     POPREF(); \
     Dee_Decref(TOP); \
     TOP = temp; /* Inherit reference. */ \
     DISPATCH(); \
 }
 DEFINE_COMPARE_INSTR(EQ,Eq)
 DEFINE_COMPARE_INSTR(NE,Ne)
 DEFINE_COMPARE_INSTR(LO,Lo)
 DEFINE_COMPARE_INSTR(LE,Le)
 DEFINE_COMPARE_INSTR(GR,Gr)
 DEFINE_COMPARE_INSTR(GE,Ge)
#undef DEFINE_COMPARE_INSTR

 TARGET(ASM_CLASS_C,-1,+1) {
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
      if unlikely(DeeObject_AssertTypeExact(descriptor,&DeeClassDescriptor_Type))
         new_class = NULL;
      else {
       new_class = DeeClass_New((DeeTypeObject *)TOP,descriptor);
      }
      Dee_Decref(descriptor);
     }
#else
     new_class = DeeClass_New((DeeTypeObject *)TOP,CONSTimm);
#endif
     if unlikely(!new_class)
        HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = (DREF DeeObject *)new_class; /* Inherit reference. */
     DISPATCH();
 }
 TARGET(ASM_CLASS_GC,-0,+1) {
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
      if unlikely(DeeObject_AssertTypeExact(descriptor,&DeeClassDescriptor_Type))
         new_class = NULL;
      else {
       new_class = DeeClass_New((DeeTypeObject *)base,descriptor);
      }
      Dee_Decref(descriptor);
     }
#else
     new_class = DeeClass_New((DeeTypeObject *)base,CONSTimm);
#endif
     Dee_Decref(base);
     if unlikely(!new_class)
        HANDLE_EXCEPT();
     PUSH((DREF DeeObject *)new_class); /* Inherit reference. */
     DISPATCH();
 }
 TARGET(ASM_CLASS_EC,-0,+1) {
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
      if unlikely(DeeObject_AssertTypeExact(descriptor,&DeeClassDescriptor_Type))
         new_class = NULL;
      else {
       new_class = DeeClass_New((DeeTypeObject *)base,descriptor);
      }
      Dee_Decref(descriptor);
     }
#else
     new_class = DeeClass_New((DeeTypeObject *)base,CONSTimm);
#endif
     Dee_Decref(base);
     if unlikely(!new_class)
        HANDLE_EXCEPT();
     PUSH((DREF DeeObject *)new_class); /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_DEFMEMBER,-2,+1) {
     imm_val = READ_imm8();
do_defmember:
#ifdef EXEC_SAFE
     if (DeeClass_SetMemberSafe((DeeTypeObject *)SECOND,imm_val,FIRST))
         HANDLE_EXCEPT();
#else
     DeeClass_SetMember((DeeTypeObject *)SECOND,imm_val,FIRST);
#endif
     POPREF();
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
     ASSERT_USAGE(-(int)(imm_val2 + 1),+1);
     ASSERT_CONSTimm();
#ifdef EXEC_SAFE
     {
      DREF DeeObject *code_object;
      CONST_LOCKREAD();
      code_object = CONSTimm;
      Dee_Incref(code_object);
      CONST_LOCKENDREAD();
      if (DeeObject_AssertTypeExact(code_object,&DeeCode_Type)) {
       Dee_Decref(code_object);
       HANDLE_EXCEPT();
      }
      if (((DeeCodeObject *)code_object)->co_refc != imm_val2 + 1) {
       err_invalid_refs_size(code_object,imm_val2 + 1);
       Dee_Decref(code_object);
       HANDLE_EXCEPT();
      }
      function = DeeFunction_NewInherited(code_object,
                                          imm_val2+1,
                                          sp-(imm_val2+1));
      Dee_Decref(code_object);
     }
#else
     function = DeeFunction_NewInherited(CONSTimm,
                                         imm_val2+1,
                                         sp-(imm_val2+1));
#endif
     if unlikely(!function) HANDLE_EXCEPT();
     sp -= imm_val2+1;
     PUSH(function);
     DISPATCH();
 }

 TARGET(ASM_CAST_INT,-1,+1) {
     DeeObject *cast_result;
     cast_result = DeeObject_Int(TOP);
     if unlikely(!cast_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = cast_result; /* Inherit reference. */
     DISPATCH();
 }

#define DEFINE_UNARY_MATH_OPERATOR(INV,Inv) \
 TARGET(ASM_##INV,-1,+1) \
 { \
     DREF DeeObject *math_result; \
     math_result = DeeObject_##Inv(TOP); \
     if unlikely(!math_result) HANDLE_EXCEPT(); \
     Dee_Decref(TOP); \
     TOP = math_result; /* Inherit reference. */ \
     DISPATCH(); \
 }
 DEFINE_UNARY_MATH_OPERATOR(INV,Inv)
 DEFINE_UNARY_MATH_OPERATOR(POS,Pos)
 DEFINE_UNARY_MATH_OPERATOR(NEG,Neg)
#undef DEFINE_UNARY_MATH_OPERATOR

#define DEFINE_BINARY_MATH_OPERATOR(ADD,Add) \
 TARGET(ASM_##ADD,-2,+1) \
 { \
     DREF DeeObject *math_result; \
     math_result = DeeObject_##Add(SECOND,FIRST); \
     if unlikely(!math_result) HANDLE_EXCEPT(); \
     POPREF(); \
     Dee_Decref(TOP); \
     TOP = math_result; /* Inherit reference. */ \
     DISPATCH(); \
 }
 DEFINE_BINARY_MATH_OPERATOR(ADD,Add)
 DEFINE_BINARY_MATH_OPERATOR(SUB,Sub)
 DEFINE_BINARY_MATH_OPERATOR(MUL,Mul)
 DEFINE_BINARY_MATH_OPERATOR(DIV,Div)
 DEFINE_BINARY_MATH_OPERATOR(MOD,Mod)
 DEFINE_BINARY_MATH_OPERATOR(SHL,Shl)
 DEFINE_BINARY_MATH_OPERATOR(SHR,Shr)
 DEFINE_BINARY_MATH_OPERATOR(AND,And)
 DEFINE_BINARY_MATH_OPERATOR(OR, Or)
 DEFINE_BINARY_MATH_OPERATOR(XOR,Xor)
 DEFINE_BINARY_MATH_OPERATOR(POW,Pow)
#undef DEFINE_BINARY_MATH_OPERATOR

 TARGET(ASM_ADD_SIMM8,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_AddS8(TOP,READ_Simm8());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }
 TARGET(ASM_ADD_IMM32,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_AddInt(TOP,READ_imm32());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_SUB_SIMM8,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_SubS8(TOP,READ_Simm8());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }
 TARGET(ASM_SUB_IMM32,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_SubInt(TOP,READ_imm32());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_MUL_SIMM8,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_MulInt(TOP,READ_Simm8());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_DIV_SIMM8,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_DivInt(TOP,READ_Simm8());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_MOD_SIMM8,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_ModInt(TOP,READ_Simm8());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_AND_IMM32,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_AndInt(TOP,READ_imm32());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_OR_IMM32,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_OrInt(TOP,READ_imm32());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_XOR_IMM32,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_XorInt(TOP,READ_imm32());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_SHL_IMM8,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_ShlInt(TOP,READ_imm8());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }

 TARGET(ASM_SHR_IMM8,-1,+1) {
     DREF DeeObject *math_result;
     math_result = DeeObject_ShrInt(TOP,READ_imm8());
     if unlikely(!math_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = math_result;
     DISPATCH();
 }


 RAW_TARGET(ASM_DELOP)
 RAW_TARGET(ASM_NOP)
 {
     /* Literally do nothing. */
     DISPATCH();
 }

 /* Print instructions. */
 TARGET(ASM_PRINT,-1,+0) {
     DREF DeeObject *stream; int error;
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream)
        HANDLE_EXCEPT();
     error = DeeFile_PrintObject(stream,TOP);
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_PRINT_SP,-1,+0) {
     DREF DeeObject *stream; int error;
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream)
        HANDLE_EXCEPT();
     error = DeeFile_PrintObjectSp(stream,TOP);
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_PRINT_NL,-1,+0) {
     DREF DeeObject *stream; int error;
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream)
        HANDLE_EXCEPT();
     error = DeeFile_PrintObjectNl(stream,TOP);
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_PRINTALL,-1,+0) {
     DREF DeeObject *stream; int error;
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream)
        HANDLE_EXCEPT();
     error = DeeFile_PrintAll(stream,TOP);
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_PRINTALL_SP,-1,+0) {
     DREF DeeObject *stream; int error;
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream)
        HANDLE_EXCEPT();
     error = DeeFile_PrintAllSp(stream,TOP);
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_PRINTALL_NL,-1,+0) {
     DREF DeeObject *stream; int error;
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream)
        HANDLE_EXCEPT();
     error = DeeFile_PrintAllNl(stream,TOP);
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_PRINTNL,-0,+0) {
     DREF DeeObject *stream; int error;
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream)
        HANDLE_EXCEPT();
     error = DeeFile_PrintLn(stream);
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     DISPATCH();
 }

 TARGET(ASM_FPRINT,-2,+1) {
     if unlikely(DeeFile_PrintObject(SECOND,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_FPRINT_SP,-2,+1) {
     if unlikely(DeeFile_PrintObjectSp(SECOND,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_FPRINT_NL,-2,+1) {
     if unlikely(DeeFile_PrintObjectNl(SECOND,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_FPRINTALL,-2,+1) {
     if unlikely(DeeFile_PrintAll(SECOND,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_FPRINTALL_SP,-2,+1) {
     if unlikely(DeeFile_PrintAllSp(SECOND,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_FPRINTALL_NL,-2,+1) {
     if unlikely(DeeFile_PrintAllNl(SECOND,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_FPRINTNL,-1,+1) {
     if unlikely(DeeFile_PrintLn(FIRST))
        HANDLE_EXCEPT();
     DISPATCH();
 }

 TARGET(ASM_PRINT_C,-0,+0) {
     DREF DeeObject *stream; int error;
     imm_val = READ_imm8();
do_print_c:
     ASSERT_CONSTimm();
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream) HANDLE_EXCEPT();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *print_object;
       CONST_LOCKREAD();
       print_object = CONSTimm;
       Dee_Incref(print_object);
       CONST_LOCKENDREAD();
       error = DeeFile_PrintObject(stream,print_object);
       Dee_Decref(print_object);
     }
#else
     error = DeeFile_PrintObject(stream,CONSTimm);
#endif
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     DISPATCH();
 }

 TARGET(ASM_PRINT_C_SP,-0,+0) {
     DREF DeeObject *stream; int error;
     imm_val = READ_imm8();
do_print_c_sp:
     ASSERT_CONSTimm();
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream) HANDLE_EXCEPT();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *print_object;
       CONST_LOCKREAD();
       print_object = CONSTimm;
       Dee_Incref(print_object);
       CONST_LOCKENDREAD();
       error = DeeFile_PrintObjectSp(stream,print_object);
       Dee_Decref(print_object);
     }
#else
     error = DeeFile_PrintObjectSp(stream,CONSTimm);
#endif
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     DISPATCH();
 }

 TARGET(ASM_PRINT_C_NL,-0,+0) {
     DREF DeeObject *stream; int error;
     imm_val = READ_imm8();
do_print_c_nl:
     ASSERT_CONSTimm();
     stream = DeeFile_GetStd(DEE_STDOUT);
     if unlikely(!stream) HANDLE_EXCEPT();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *print_object;
       CONST_LOCKREAD();
       print_object = CONSTimm;
       Dee_Incref(print_object);
       CONST_LOCKENDREAD();
       error = DeeFile_PrintObjectNl(stream,print_object);
       Dee_Decref(print_object);
     }
#else
     error = DeeFile_PrintObjectNl(stream,CONSTimm);
#endif
     Dee_Decref(stream);
     if unlikely(error)
        HANDLE_EXCEPT();
     DISPATCH();
 }

 TARGET(ASM_FPRINT_C,-1,+1) {
     imm_val = READ_imm8();
do_fprint_c:
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *print_object;
       int error;
       CONST_LOCKREAD();
       print_object = CONSTimm;
       Dee_Incref(print_object);
       CONST_LOCKENDREAD();
       error = DeeFile_PrintObject(TOP,print_object);
       Dee_Decref(print_object);
       if unlikely(error)
          HANDLE_EXCEPT();
     }
#else
     if unlikely(DeeFile_PrintObject(TOP,CONSTimm))
        HANDLE_EXCEPT();
#endif
     DISPATCH();
 }

 TARGET(ASM_FPRINT_C_SP,-1,+1) {
     imm_val = READ_imm8();
do_fprint_c_sp:
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *print_object;
       int error;
       CONST_LOCKREAD();
       print_object = CONSTimm;
       Dee_Incref(print_object);
       CONST_LOCKENDREAD();
       error = DeeFile_PrintObjectSp(TOP,print_object);
       Dee_Decref(print_object);
       if unlikely(error)
          HANDLE_EXCEPT();
     }
#else
     if unlikely(DeeFile_PrintObjectSp(TOP,CONSTimm))
        HANDLE_EXCEPT();
#endif
     DISPATCH();
 }

 TARGET(ASM_FPRINT_C_NL,-1,+1) {
     imm_val = READ_imm8();
do_fprint_c_nl:
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *print_object;
       int error;
       CONST_LOCKREAD();
       print_object = CONSTimm;
       Dee_Incref(print_object);
       CONST_LOCKENDREAD();
       error = DeeFile_PrintObjectNl(TOP,print_object);
       Dee_Decref(print_object);
       if unlikely(error)
          HANDLE_EXCEPT();
     }
#else
     if unlikely(DeeFile_PrintObjectNl(TOP,CONSTimm))
        HANDLE_EXCEPT();
#endif
     DISPATCH();
 }

 TARGET(ASM_RANGE_0_I16,-0,+1) {
     DREF DeeObject *range_object;
#if __SIZEOF_SIZE_T__ <= 2
#error "sizeof(size_t) is too small and may cause an overflow for range object (WTF? a 16-bit machine?)"
#endif
     range_object = DeeRange_NewInt(0,READ_imm16(),1);
     if unlikely(!range_object) HANDLE_EXCEPT();
     PUSH(range_object);
     DISPATCH();
 }

 TARGET(ASM_RANGE,-2,+1) {
     DREF DeeObject *range_object;
     range_object = DeeRange_New(DeeNone_Check(SECOND) ? &DeeInt_Zero : SECOND,
                                 FIRST,NULL);
     if unlikely(!range_object) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_object; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_RANGE_DEF,-1,+1) {
     DREF DeeObject *range_object,*begin;
     begin = DeeObject_NewDefault(Dee_TYPE(TOP));
     if unlikely(!begin) HANDLE_EXCEPT();
     range_object = DeeRange_New(begin,TOP,NULL);
     Dee_Decref(begin);
     if unlikely(!range_object) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = range_object; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_RANGE_STEP,-3,+1) {
     DREF DeeObject *range_object;
     range_object = DeeRange_New(DeeNone_Check(THIRD) ? &DeeInt_Zero : THIRD,SECOND,
                                 DeeNone_Check(FIRST) ? NULL : FIRST);
     if unlikely(!range_object) HANDLE_EXCEPT();
     POPREF();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_object; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_RANGE_STEP_DEF,-2,+1) {
     DREF DeeObject *range_object,*begin;
     begin = DeeObject_NewDefault(Dee_TYPE(SECOND));
     if unlikely(!begin) HANDLE_EXCEPT();
     range_object = DeeRange_New(begin,SECOND,
                                 DeeNone_Check(FIRST) ? NULL : FIRST);
     Dee_Decref(begin);
     if unlikely(!range_object) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_object; /* Inherit reference. */
     DISPATCH();
 }

 /* With-operators. */
 TARGET(ASM_ENTER,-1,+1) {
     if (DeeObject_Enter(TOP))
         HANDLE_EXCEPT();
     DISPATCH();
 }
 TARGET(ASM_LEAVE,-1,+0) {
     if (DeeObject_Leave(TOP))
         HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }


 /* Sequence operators. */
 TARGET(ASM_GETSIZE,-1,+1) {
     DREF DeeObject *object_size;
     object_size = DeeObject_SizeObject(TOP);
     if unlikely(!object_size) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = object_size; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_CONTAINS,-2,+1) {
     DREF DeeObject *does_contain;
     does_contain = DeeObject_ContainsObject(SECOND,FIRST);
     if unlikely(!does_contain) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = does_contain; /* Inherit reference. */
     DISPATCH();
 }

 RAW_TARGET(ASM_CONTAINS_C) {
     DREF DeeObject *value;
     imm_val = READ_imm8();
do_contains_c:
     ASSERT_USAGE(-1,+1);
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *constant_set;
       CONST_LOCKREAD();
       constant_set = CONSTimm;
       Dee_Incref(constant_set);
       CONST_LOCKENDREAD();
       value = DeeObject_ContainsObject(constant_set,TOP);
       Dee_Decref(constant_set);
     }
#else
     value = DeeObject_ContainsObject(CONSTimm,TOP);
#endif
     if unlikely(!value) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETITEM,-2,+1) {
     DREF DeeObject *value;
     value = DeeObject_GetItem(SECOND,FIRST);
     if unlikely(!value) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETITEM_I,-1,+1) {
     DREF DeeObject *value;
     value = DeeObject_GetItemIndex(TOP,READ_Simm16());
     if unlikely(!value) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = value; /* Inherit reference. */
     DISPATCH();
 }

 RAW_TARGET(ASM_GETITEM_C) {
     DREF DeeObject *value;
     imm_val = READ_imm8();
do_getitem_c:
     ASSERT_USAGE(-1,+1);
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *index_object;
       CONST_LOCKREAD();
       index_object = CONSTimm;
       Dee_Incref(index_object);
       CONST_LOCKENDREAD();
       value = DeeObject_GetItem(TOP,index_object);
       Dee_Decref(index_object);
     }
#else
     value = DeeObject_GetItem(TOP,CONSTimm);
#endif
     if unlikely(!value) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_SETITEM,-3,+0) {
     if (DeeObject_SetItem(THIRD,SECOND,FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETITEM_I,-2,+0) {
     if (DeeObject_SetItemIndex(SECOND,READ_Simm16(),FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETITEM_C,-2,+0) {
     imm_val = READ_imm8();
do_setitem_c:
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *index_object;
       int error;
       CONST_LOCKREAD();
       index_object = CONSTimm;
       Dee_Incref(index_object);
       CONST_LOCKENDREAD();
       error = DeeObject_SetItem(SECOND,index_object,FIRST);
       Dee_Decref(index_object);
       if unlikely(error) HANDLE_EXCEPT();
     }
#else
     if (DeeObject_SetItem(SECOND,CONSTimm,FIRST))
         HANDLE_EXCEPT();
#endif
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_DELITEM,-2,+0) {
     if (DeeObject_DelItem(SECOND,FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_GETRANGE,-3,+1) {
     DREF DeeObject *range_value;
     range_value = DeeObject_GetRange(THIRD,SECOND,FIRST);
     if unlikely(!range_value) HANDLE_EXCEPT();
     POPREF();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETRANGE_PN,-2,+1) {
     DREF DeeObject *range_value;
     range_value = DeeObject_GetRange(SECOND,FIRST,Dee_None);
     if unlikely(!range_value) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETRANGE_NP,-2,+1) {
     DREF DeeObject *range_value;
     range_value = DeeObject_GetRange(SECOND,Dee_None,FIRST);
     if unlikely(!range_value) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETRANGE_PI,-2,+1) {
     DREF DeeObject *range_value;
     range_value = DeeObject_GetRangeEndIndex(SECOND,FIRST,READ_Simm16());
     if unlikely(!range_value) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETRANGE_IP,-2,+1) {
     DREF DeeObject *range_value;
     range_value = DeeObject_GetRangeBeginIndex(SECOND,READ_Simm16(),FIRST);
     if unlikely(!range_value) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETRANGE_NI,-1,+1) {
     DREF DeeObject *range_value;
     range_value = DeeObject_GetRangeEndIndex(TOP,Dee_None,READ_Simm16());
     if unlikely(!range_value) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETRANGE_IN,-1,+1) {
     DREF DeeObject *range_value;
     range_value = DeeObject_GetRangeBeginIndex(TOP,READ_Simm16(),Dee_None);
     if unlikely(!range_value) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETRANGE_II,-1,+1) {
     DREF DeeObject *range_value;
     int16_t begin = READ_Simm16();
     range_value = DeeObject_GetRangeIndex(TOP,begin,READ_Simm16());
     if unlikely(!range_value) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = range_value; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_DELRANGE,-3,+0) {
     if (DeeObject_DelRange(THIRD,SECOND,FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE,-4,+0) {
     if (DeeObject_SetRange(FOURTH,THIRD,SECOND,FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE_PN,-3,+0) {
     if (DeeObject_SetRange(THIRD,SECOND,Dee_None,FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE_NP,-3,+0) {
     if (DeeObject_SetRange(THIRD,Dee_None,SECOND,FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE_PI,-3,+0) {
     if unlikely(DeeObject_SetRangeEndIndex(THIRD,SECOND,READ_Simm16(),FIRST))
        HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE_IP,-3,+0) {
     if unlikely(DeeObject_SetRangeBeginIndex(THIRD,READ_Simm16(),SECOND,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE_NI,-2,+0) {
     if unlikely(DeeObject_SetRangeEndIndex(SECOND,Dee_None,READ_Simm16(),FIRST))
        HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE_IN,-2,+0) {
     if unlikely(DeeObject_SetRangeBeginIndex(SECOND,READ_Simm16(),Dee_None,FIRST))
        HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETRANGE_II,-2,+0) {
     int16_t begin = READ_Simm16();
     if unlikely(DeeObject_SetRangeIndex(SECOND,begin,READ_Simm16(),FIRST))
        HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

/* Breakpoint. */
 TARGET(ASM_BREAKPOINT,-0,+0) {
     int error;
     /* Safe the instruction + stack-pointer. */
     frame->cf_ip = ip.ptr;
     frame->cf_sp = sp;
#ifdef EXEC_FAST
     frame->cf_stacksz = STACKPREALLOC;
#endif
     /* Trigger a breakpoint. */
     error = trigger_breakpoint(frame);
     /* _always_ load new SP/IP from the frame to ensure that we are in a
      * consistent state before handling potential exception, or moving on.
      * After all: The purpose of this instruction is to allow some external
      *            utility to inspect and/or modify our frame before either
      *            raising an exception, or passing control back to running code.
      * HINT: The idea for breakpoints is for some utility to replace opcodes
      *       that the runtime should pause at with `ASM_BREAKPOINT' instructions,
      *       causing it to halt and pass control over to said utility with
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
      if (code->co_flags&CODE_FYIELDING) {
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
      if (code->co_flags&CODE_FYIELDING) {
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
      frame->cf_prev = CODE_FRAME_NOT_EXECUTING;
      /* Indicate that the stack hasn't been allocated dynamically. */
      frame->cf_stacksz = 0;
      /* Continue execution in safe-mode. */
      DeeCode_ExecFrameSafe(frame);
      /* Once safe-mode execution finishes, propagate it's return value
       * and clean up any additional exception that we've raised before. */
      goto end_nounhook;
#endif /* EXEC_FAST */

     default:
      if (error < 0) HANDLE_EXCEPT();
      break;
     }
     DISPATCH();
 }

 /* Sequence iterator creation. */
 TARGET(ASM_ITERSELF,-1,+1) {
     DREF DeeObject *iterator;
     iterator = DeeObject_IterSelf(TOP);
     if unlikely(!iterator) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = iterator;
     /* Predict that this will be a foreach loop. */
     PREDICT(ASM_FOREACH);
     /*PREDICT(ASM_FOREACH16);*/
     DISPATCH();
 }

 /* Call attribute. */
 RAW_TARGET(ASM_CALLATTR) {
     DREF DeeObject *resval,**new_sp;
     uint8_t n_args = READ_imm8();
     ASSERT_USAGE(-(2+(int)n_args),+1);
     new_sp = sp-n_args;
     if unlikely(!DeeString_Check(new_sp[-1])) {
      err_expected_string_for_attribute(new_sp[-1]);
      HANDLE_EXCEPT();
     }
     resval = DeeObject_CallAttr(new_sp[-2],new_sp[-1],n_args,new_sp);
     if unlikely(!resval) HANDLE_EXCEPT();
     while (n_args--) POPREF();
     POPREF();
     Dee_Decref(TOP);
     TOP = resval; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_CALLATTR_TUPLE,-3,+1) {
     DREF DeeObject *call_result;
     ASSERT_TUPLE(FIRST);
     if unlikely(!DeeString_Check(SECOND)) {
      err_expected_string_for_attribute(SECOND);
      HANDLE_EXCEPT();
     }
     call_result = DeeObject_CallAttrTuple(THIRD,SECOND,FIRST);
     if unlikely(!call_result) HANDLE_EXCEPT();
     POPREF();
     POPREF();
     Dee_Decref(TOP);
     TOP = call_result; /* Inherit reference. */
     DISPATCH();
 }

 
 RAW_TARGET(ASM_CALLATTR_C_KW) {
     uint8_t argc; DREF DeeObject *call_result;
     DeeObject **new_sp;
     imm_val  = READ_imm8();
     argc     = READ_imm8();
     imm_val2 = READ_imm8();
     ASSERT_USAGE(-((int)argc + 1),+1);
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
#else
     ASSERT_STRING(CONSTimm);
     call_result = DeeObject_CallAttrKw(new_sp[-1],
                                        CONSTimm,
                                        argc,
                                        new_sp,
                                        CONSTimm2);
#endif
     if unlikely(!call_result) HANDLE_EXCEPT();
     while (sp > new_sp) POPREF();
     Dee_Decref(TOP);
     TOP = call_result; /* Inherit reference. */
     DISPATCH();
 }
 RAW_TARGET(ASM_CALLATTR_TUPLE_C_KW) {
     DREF DeeObject *call_result;
     imm_val  = READ_imm8();
     imm_val2 = READ_imm8();
do_callattr_tuple_c_kw:
     ASSERT_USAGE(-2,+1);
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
      call_result = DeeObject_CallAttrTupleKw(SECOND,attr_name,FIRST,kwds_map);
      Dee_Decref_unlikely(attr_name);
      Dee_Decref_unlikely(kwds_map);
     }
#else
     ASSERT_STRING(CONSTimm);
     call_result = DeeObject_CallAttrTupleKw(SECOND,CONSTimm,FIRST,CONSTimm2);
#endif
     if unlikely(!call_result) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = call_result; /* Inherit reference. */
     DISPATCH();
 }

 RAW_TARGET(ASM_CALLATTR_C) {
     DREF DeeObject *callback_result,**new_sp;
     imm_val = READ_imm8();
do_callattr_c:
     imm_val2 = READ_imm8();
     ASSERT_USAGE(-((int)imm_val2+1),+1);
     ASSERT_CONSTimm();
     new_sp = sp-imm_val2;
#ifdef EXEC_SAFE
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref_unlikely(imm_name);
        goto err_requires_string;
       }
       callback_result = DeeObject_CallAttr(new_sp[-1],imm_name,imm_val2,new_sp);
       Dee_Decref_unlikely(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     callback_result = DeeObject_CallAttr(new_sp[-1],CONSTimm,imm_val2,new_sp);
#endif
     if unlikely(!callback_result) HANDLE_EXCEPT();
     while (imm_val2--) POPREF();
     Dee_Decref(TOP);
     TOP = callback_result; /* Inherit reference. */
     DISPATCH();
 }

 RAW_TARGET(ASM_CALLATTR_C_SEQ) {
     DREF DeeObject *callback_result,**new_sp;
     DREF SharedVector *shared_vector;
     imm_val = READ_imm8();
do_callattr_c_seq:
     imm_val2 = READ_imm8();
     ASSERT_USAGE(-((int)imm_val2+1),+1);
     ASSERT_CONSTimm();
     new_sp = sp-imm_val2;
#ifdef EXEC_SAFE
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       shared_vector = SharedVector_NewShared(imm_val2,new_sp);
       if unlikely(!shared_vector) { Dee_Decref(imm_name); HANDLE_EXCEPT(); }
       sp = new_sp;
       callback_result = DeeObject_CallAttr(new_sp[-1],imm_name,1,
                                           (DeeObject **)&shared_vector);
       SharedVector_Decref(shared_vector);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     shared_vector = SharedVector_NewShared(imm_val2,new_sp);
     if unlikely(!shared_vector) HANDLE_EXCEPT();
     sp = new_sp;
     callback_result = DeeObject_CallAttr(new_sp[-1],CONSTimm,1,
                                         (DeeObject **)&shared_vector);
     SharedVector_Decref(shared_vector);
#endif
     if unlikely(!callback_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = callback_result; /* Inherit reference. */
     DISPATCH();
 }

 RAW_TARGET(ASM_CALLATTR_C_MAP) {
     DREF DeeObject *callback_result,**new_sp;
     DREF SharedMap *shared_map;
     imm_val = READ_imm8();
do_callattr_c_map:
     imm_val2 = READ_imm8();
     ASSERT_USAGE(-((int)(imm_val2 * 2)+1),+1);
     ASSERT_CONSTimm();
     new_sp = sp - imm_val2 * 2;
#ifdef EXEC_SAFE
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       shared_map = SharedMap_NewShared(imm_val2,(DREF SharedKey *)new_sp);
       if unlikely(!shared_map) { Dee_Decref(imm_name); HANDLE_EXCEPT(); }
       sp = new_sp;
       callback_result = DeeObject_CallAttr(new_sp[-1],imm_name,1,
                                           (DeeObject **)&shared_map);
       SharedMap_Decref(shared_map);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     shared_map = SharedMap_NewShared(imm_val2,(DREF SharedKey *)new_sp);
     if unlikely(!shared_map) HANDLE_EXCEPT();
     sp = new_sp;
     callback_result = DeeObject_CallAttr(new_sp[-1],CONSTimm,1,
                                         (DeeObject **)&shared_map);
     SharedMap_Decref(shared_map);
#endif
     if unlikely(!callback_result) HANDLE_EXCEPT();
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
     ASSERT_USAGE(-(int)imm_val2,+1);
     ASSERT_CONSTimm();
#ifdef EXEC_SAFE
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       callback_result = DeeObject_CallAttr(THIS,imm_name,imm_val2,sp-imm_val2);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     callback_result = DeeObject_CallAttr(THIS,CONSTimm,imm_val2,sp-imm_val2);
#endif
     if unlikely(!callback_result) HANDLE_EXCEPT();
     while (imm_val2--) POPREF();
     PUSH(callback_result); /* Push the result onto the stack. */
     DISPATCH();
 }

 TARGET(ASM_CALLATTR_TUPLE_C,-2,+1) {
     DREF DeeObject *callback_result;
     imm_val = READ_imm8();
do_callattr_tuple_c:
     ASSERT_TUPLE(FIRST);
     ASSERT_CONSTimm();
#ifdef EXEC_SAFE
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       callback_result = DeeObject_CallAttrTuple(SECOND,imm_name,FIRST);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     callback_result = DeeObject_CallAttrTuple(SECOND,CONSTimm,FIRST);
#endif
     if unlikely(!callback_result) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = callback_result; /* Inherit reference. */
     DISPATCH();
 }
     
 TARGET(ASM_CALLATTR_THIS_TUPLE_C,-1,+1) {
     DREF DeeObject *callback_result;
     imm_val = READ_imm8();
do_callattr_this_tuple_c:
     ASSERT_THISCALL();
     ASSERT_TUPLE(TOP);
     ASSERT_CONSTimm();
#ifdef EXEC_SAFE
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       callback_result = DeeObject_CallAttrTuple(THIS,imm_name,TOP);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     callback_result = DeeObject_CallAttrTuple(THIS,CONSTimm,TOP);
#endif
     if unlikely(!callback_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = callback_result; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_GETMEMBER_R,-0,+1) {
     DREF DeeObject *result;
     imm_val  = READ_imm8();
     imm_val2 = READ_imm8();
do_getmember_r:
     ASSERT_THISCALL();
#ifdef EXEC_SAFE
     result = DeeInstance_GetMemberSafe((DeeTypeObject *)REFimm,THIS,imm_val2);
#else
     result = DeeInstance_GetMember((DeeTypeObject *)REFimm,THIS,imm_val2);
#endif
     if unlikely(!result) HANDLE_EXCEPT();
     PUSH(result);
     DISPATCH();
 }

 TARGET(ASM_BOUNDMEMBER_R,-0,+1) {
#ifdef EXEC_SAFE
     int temp;
#else
     bool temp;
#endif
     imm_val  = READ_imm8();
     imm_val2 = READ_imm8();
do_boundmember_r:
     ASSERT_THISCALL();
     ASSERT_REFimm();
#ifdef EXEC_SAFE
     temp = DeeInstance_BoundMemberSafe((DeeTypeObject *)REFimm,THIS,imm_val2);
     if unlikely(temp < 0)
        HANDLE_EXCEPT();
     PUSHREF(DeeBool_For(temp != 0));
#else
     temp = DeeInstance_BoundMember((DeeTypeObject *)REFimm,THIS,imm_val2);
     PUSHREF(DeeBool_For(temp));
#endif
     DISPATCH();
 }

 TARGET(ASM_DELMEMBER_R,-0,+0) {
     imm_val  = READ_imm8();
     imm_val2 = READ_imm8();
do_delmember_r:
     ASSERT_THISCALL();
     ASSERT_REFimm();
#ifdef EXEC_SAFE
     if (DeeInstance_DelMemberSafe((DeeTypeObject *)REFimm,THIS,imm_val2))
         HANDLE_EXCEPT();
#else
     if (DeeInstance_DelMember((DeeTypeObject *)REFimm,THIS,imm_val2))
         HANDLE_EXCEPT();
#endif
     DISPATCH();
 }

 TARGET(ASM_SETMEMBER_R,-1,+0) {
     imm_val  = READ_imm8();
     imm_val2 = READ_imm8();
do_setmember_r:
     ASSERT_THISCALL();
     ASSERT_REFimm();
#ifdef EXEC_SAFE
     if unlikely(DeeInstance_SetMemberSafe((DeeTypeObject *)REFimm,THIS,imm_val2,TOP))
        HANDLE_EXCEPT();
#else
     DeeInstance_SetMember((DeeTypeObject *)REFimm,THIS,
                           imm_val2,TOP);
#endif
     POPREF();
     DISPATCH();
 }


 {   DREF DeeObject *call_object,*call_result;
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
#else
     ASSERT_USAGE(-(int)imm_val,+1);
#endif
     call_result = DeeObject_Call(call_object,imm_val,sp-imm_val);
     Dee_Decref(call_object);
     if unlikely(!call_result) HANDLE_EXCEPT();
     while (imm_val--) POPREF();
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

 TARGET(ASM_GETATTR,-2,+1) {
     DeeObject *attr_result;
     if unlikely(!DeeString_Check(FIRST)) {
      err_expected_string_for_attribute(FIRST);
      HANDLE_EXCEPT();
     }
     attr_result = DeeObject_GetAttr(SECOND,FIRST);
     if unlikely(!attr_result) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = attr_result; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_DELATTR,-2,+0) {
     if unlikely(!DeeString_Check(FIRST)) {
      err_expected_string_for_attribute(FIRST);
      HANDLE_EXCEPT();
     }
     if (DeeObject_DelAttr(SECOND,FIRST))
         HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETATTR,-3,+0) {
     int error;
     if unlikely(!DeeString_Check(SECOND)) {
      err_expected_string_for_attribute(SECOND);
      HANDLE_EXCEPT();
     }
     error = DeeObject_SetAttr(THIRD,SECOND,FIRST);
     if unlikely(error < 0) HANDLE_EXCEPT();
     POPREF();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_BOUNDATTR,-2,+1) {
     int error;
     if unlikely(!DeeString_Check(FIRST)) {
      err_expected_string_for_attribute(FIRST);
      HANDLE_EXCEPT();
     }
     error = DeeObject_BoundAttr(SECOND,FIRST);
     if unlikely(error == -2) HANDLE_EXCEPT();
     POPREF();
     Dee_Decref(TOP);
     TOP = DeeBool_For(error > 0);
     Dee_Incref(TOP);
     DISPATCH();
 }

 TARGET(ASM_GETATTR_C,-1,+1) {
     DREF DeeObject *getattr_result;
     imm_val = READ_imm8();
do_getattr_c:
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       getattr_result = DeeObject_GetAttr(TOP,imm_name);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     getattr_result = DeeObject_GetAttr(TOP,CONSTimm);
#endif
     if unlikely(!getattr_result) HANDLE_EXCEPT();
     Dee_Decref(TOP);
     TOP = getattr_result; /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_DELATTR_C,-1,+0) {
     int error;
     imm_val = READ_imm8();
do_delattr_c:
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       error = DeeObject_DelAttr(TOP,imm_name);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     error = DeeObject_DelAttr(TOP,CONSTimm);
#endif
     if unlikely(error < 0) HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_SETATTR_C,-2,+0) {
     int error;
     imm_val = READ_imm8();
do_setattr_c:
     ASSERT_CONSTimm();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       error = DeeObject_SetAttr(SECOND,imm_name,FIRST);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     error = DeeObject_SetAttr(SECOND,CONSTimm,FIRST);
#endif
     if unlikely(error < 0) HANDLE_EXCEPT();
     POPREF();
     POPREF();
     DISPATCH();
 }

 TARGET(ASM_GETATTR_THIS_C,-0,+1) {
     DREF DeeObject *getattr_result;
     imm_val = READ_imm8();
do_getattr_this_c:
     ASSERT_CONSTimm();
     ASSERT_THISCALL();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       getattr_result = DeeObject_GetAttr(THIS,imm_name);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     getattr_result = DeeObject_GetAttr(THIS,CONSTimm);
#endif
     if unlikely(!getattr_result) HANDLE_EXCEPT();
     PUSH(getattr_result); /* Inherit reference. */
     DISPATCH();
 }

 TARGET(ASM_DELATTR_THIS_C,-0,+0) {
     int error;
     imm_val = READ_imm8();
do_delattr_this_c:
     ASSERT_CONSTimm();
     ASSERT_THISCALL();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       error = DeeObject_DelAttr(THIS,imm_name);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     error = DeeObject_DelAttr(THIS,CONSTimm);
#endif
     if unlikely(error < 0) HANDLE_EXCEPT();
     DISPATCH();
 }

 TARGET(ASM_SETATTR_THIS_C,-1,+0) {
     int error;
     imm_val = READ_imm8();
do_setattr_this_c:
     ASSERT_CONSTimm();
     ASSERT_THISCALL();
#if defined(EXEC_SAFE) && !defined(CONFIG_NO_THREADS)
     { DREF DeeObject *imm_name;
       CONST_LOCKREAD();
       imm_name = CONSTimm;
       Dee_Incref(imm_name);
       CONST_LOCKENDREAD();
       if unlikely(!DeeString_CheckExact(imm_name)) {
        Dee_Decref(imm_name);
        goto err_requires_string;
       }
       error = DeeObject_SetAttr(THIS,imm_name,TOP);
       Dee_Decref(imm_name);
     }
#else
     ASSERT_STRING(CONSTimm);
     error = DeeObject_SetAttr(THIS,CONSTimm,TOP);
#endif
     if unlikely(error < 0) HANDLE_EXCEPT();
     POPREF();
     DISPATCH();
 }



 /* Opcodes for misc/rarely used operators (Prefixed by `ASM_EXTENDED1'). */
 RAW_TARGET(ASM_EXTENDED1) {

#ifdef USE_SWITCH
     switch (*ip.ptr++)
#else
     goto *f0_targets[*ip.ptr++];
#endif
     {

     TARGET(ASM_ENDCATCH_N,-0,+0) {
         uint8_t nth_except = READ_imm8();
         if (this_thread->t_exceptsz > except_recursion+nth_except+1) {
          struct except_frame **pframe,*frame;
          /* We're allowed to handle the `nth_except' exception. */
          pframe = &this_thread->t_except;
          do frame = *pframe,
             ASSERT(frame),
             pframe = &frame->ef_prev;
          while (nth_except--);
          /* Load the frame that we're supposed to get rid of. */
          frame = *pframe;
          ASSERT(frame);
          /* Remove the exception frame from its chain. */
          *pframe = frame->ef_prev;
          --this_thread->t_exceptsz;
          /* Destroy the frame in question. */
          if (ITER_ISOK(frame->ef_trace))
              Dee_Decref(frame->ef_trace);
          Dee_Decref(frame->ef_error);
          ef_free(frame);
         }
         DISPATCH();
     }

     TARGET(ASM_ENDFINALLY_N,-0,+0) {
         uint8_t min_except = READ_imm8();
         /* If a return value has been assigned, stop execution. */
         if (frame->cf_result != NULL)
             goto end_return;
         /* Check for errors, but only handle them if there are more than `min_except+1'. */
         if (this_thread->t_exceptsz > except_recursion+min_except+1)
             HANDLE_EXCEPT();
         DISPATCH();
     }

     TARGET(ASM16_PUSH_BND_EXTERN,-0,+1) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_push_bnd_extern;
     }

     TARGET(ASM16_PUSH_BND_GLOBAL,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_bnd_global;
     }

     TARGET(ASM16_PUSH_BND_LOCAL,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_bnd_local;
     }

     RAW_TARGET(ASM32_JMP) {
         int32_t disp32;
         disp32 = READ_Simm32();
#ifndef CONFIG_NO_THREADS
         if (disp32 < 0 &&
             DeeThread_CheckInterruptSelf(this_thread))
             HANDLE_EXCEPT();
#endif
         ip.ptr += disp32;
#ifdef EXEC_SAFE
         goto assert_ip_bounds;
#else
         ASSERT(ip.ptr >= code->co_code);
         ASSERT(ip.ptr <  code->co_code+code->co_codebytes);
         DISPATCH();
#endif
     }

     TARGET(ASM_JMP_POP_POP,-2,+0) {
         code_addr_t absip; uint16_t absstk,stksz;
         instruction_t *new_ip;
         if (DeeObject_AsUInt16(TOP,&absstk)) HANDLE_EXCEPT();
         if (DeeObject_AsUInt32(SECOND,&absip)) HANDLE_EXCEPT();
#ifdef EXEC_SAFE
         if (absip >= code->co_codebytes) {
          DeeError_Throwf(&DeeError_SegFault,
                          "Invalid IP %.4I32X in absolute jmp",
                          absip);
          HANDLE_EXCEPT();
         }
         if (absstk > STACKSIZE)
             goto increase_stacksize;
#else
         ASSERTF(absip < code->co_codebytes,"Invalid IP: %X",
                (unsigned int)absip);
         ASSERT(absstk <= STACKSIZE);
#endif
         new_ip = code->co_code + absip;
#ifndef CONFIG_NO_THREADS
         if (new_ip < ip.ptr &&
             DeeThread_CheckInterruptSelf(this_thread))
             HANDLE_EXCEPT();
#endif
         stksz = (uint16_t)(sp - frame->cf_stack);
         if (absstk > stksz) {
          absstk -= stksz;
          while (absstk--) PUSHREF(Dee_None);
         } else {
          stksz -= absstk;
          while (stksz--) POPREF();
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
         ASSERT_USAGE(-3,+1);
         ASSERT_TUPLE(SECOND);
         call_result = DeeObject_CallTupleKw(THIRD,SECOND,FIRST);
         if unlikely(!call_result) HANDLE_EXCEPT();
         POPREF();
         POPREF();
         Dee_Decref(TOP);
         TOP = call_result; /* Save the callback result. */
         DISPATCH();
     }

     RAW_TARGET(ASM_CALL_SEQ) {
         /* Sequence constructor invocation (Implemented using `_sharedvector'). */
         uint8_t n_args = READ_imm8();
         DREF DeeObject *callback_result;
#ifdef NEED_UNIVERSAL_PREFIX_OB_WORKAROUND
#define shared_vector  (*(DREF SharedVector **)&prefix_ob)
#else /* NEED_UNIVERSAL_PREFIX_OB_WORKAROUND */
         DREF SharedVector *shared_vector;
#endif /* !NEED_UNIVERSAL_PREFIX_OB_WORKAROUND */
         ASSERT_USAGE(-((int)n_args+1),+1);
         shared_vector = SharedVector_NewShared(n_args,sp-n_args);
         if unlikely(!shared_vector)
            HANDLE_EXCEPT();
         sp -= n_args; /* These operands have been inherited `SharedVector_NewShared' */
         /* Invoke the object that is now located in TOP
          * For this invocation, we pass only a single argument `shared_vector' */
         callback_result = DeeObject_Call(TOP,1,(DeeObject **)&shared_vector);
         SharedVector_Decref(shared_vector);
         if unlikely(!callback_result)
            HANDLE_EXCEPT();
         /* Replace the function that was called with its return value. */
         Dee_Decref(TOP);
         TOP = callback_result;
         DISPATCH();
#ifdef NEED_UNIVERSAL_PREFIX_OB_WORKAROUND
#undef shared_vector
#endif /* !NEED_UNIVERSAL_PREFIX_OB_WORKAROUND */
     }

     RAW_TARGET(ASM_CALL_MAP) {
         /* dict-style sequence constructor invocation (Implemented using `_sharedkeyvector'). */
         uint8_t n_args = READ_imm8();
         DREF DeeObject *callback_result;
#ifdef NEED_UNIVERSAL_PREFIX_OB_WORKAROUND
#define shared_key_vector  (*(DREF SharedMap **)&prefix_ob)
#else /* NEED_UNIVERSAL_PREFIX_OB_WORKAROUND */
         DREF SharedMap *shared_key_vector;
#endif /* !NEED_UNIVERSAL_PREFIX_OB_WORKAROUND */
         ASSERT_USAGE(-(((int)n_args*2)+1),+1);
         /* NOTE: The vector of SharedKey structures has
          *       previously been constructed on the stack. */
         shared_key_vector = SharedMap_NewShared(n_args,(SharedKey *)(sp-n_args*2));
         if unlikely(!shared_key_vector)
            HANDLE_EXCEPT();
         sp -= n_args*2; /* These operands have been inherited `SharedVector_NewShared' */
         /* Invoke the object that is now located in TOP
          * For this invocation, we pass only a single argument `shared_key_vector' */
         callback_result = DeeObject_Call(TOP,1,(DeeObject **)&shared_key_vector);
         SharedMap_Decref(shared_key_vector);
         if unlikely(!callback_result)
            HANDLE_EXCEPT();
         /* Replace the function that was called with its return value. */
         Dee_Decref(TOP);
         TOP = callback_result;
         DISPATCH();
#ifdef NEED_UNIVERSAL_PREFIX_OB_WORKAROUND
#undef shared_key_vector
#endif /* !NEED_UNIVERSAL_PREFIX_OB_WORKAROUND */
     }

     TARGET(ASM_PUSH_TRUE,-0,+1) {
         PUSHREF(Dee_True);
         DISPATCH();
     }
     TARGET(ASM_PUSH_FALSE,-0,+1) {
         PUSHREF(Dee_False);
         DISPATCH();
     }
     TARGET(ASM_PUSH_EXCEPT,-0,+1) {
         DeeObject *temp;
         /* Check if an exception has been set. */
         if unlikely(!this_thread->t_except)
            goto except_no_active_exception;
         temp = this_thread->t_except->ef_error;
         PUSHREF(temp);
         DISPATCH();
     }

     TARGET(ASM_PUSH_THIS,-0,+1) {
         ASSERT_THISCALL();
         PUSHREF(THIS);
         DISPATCH();
     }

     TARGET(ASM_PUSH_THIS_FUNCTION,-0,+1) {
         PUSHREF((DeeObject *)frame->cf_func);
         DISPATCH();
     }

     TARGET(ASM_PUSH_THIS_MODULE,-0,+1) {
         PUSHREF((DeeObject *)code->co_module);
         DISPATCH();
     }


     TARGET(ASM_CMP_SO,-2,+1) {
         bool is_same = SECOND == FIRST;
         POPREF();
         Dee_Decref(TOP);
         TOP = DeeBool_For(is_same);
         Dee_Incref(TOP);
         DISPATCH();
     }
     TARGET(ASM_CMP_DO,-2,+1) {
         bool is_diff = SECOND != FIRST;
         POPREF();
         Dee_Decref(TOP);
         TOP = DeeBool_For(is_diff);
         Dee_Incref(TOP);
         DISPATCH();
     }


     RAW_TARGET(ASM16_DELOP)
     RAW_TARGET(ASM16_NOP)
         /* Allow an ignore delop/nop being used with the F0 prefix. */
         DISPATCH();

     RAW_TARGET(ASM16_OPERATOR) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm8();
         goto do_operator;
     }
     TARGET(ASM16_OPERATOR_TUPLE,-2,+1) {
         imm_val = READ_imm16();
         goto do_operator_tuple;
     }

     TARGET(ASM16_DEL_GLOBAL,-0,+0) {
         imm_val = READ_imm16();
         goto do_del_global;
     }
     TARGET(ASM16_DEL_LOCAL,-0,+0) {
         imm_val = READ_imm16();
         goto do_del_local;
     }

     RAW_TARGET(ASM16_LROT) {
         DREF DeeObject *temp;
         uint32_t shift;
         shift = (uint32_t)READ_imm16() + 3;
         ASSERT_USAGE(-(int)shift,+(int)shift);
         --sp;
         temp = *(sp-shift);
         MEMMOVE_PTR(sp-(shift+1),sp-shift,shift+1);
         *sp = temp;
         ++sp;
         DISPATCH();
     }

     RAW_TARGET(ASM16_RROT) {
         DREF DeeObject *temp;
         uint32_t shift;
         shift = (uint32_t)READ_imm16() + 3;
         ASSERT_USAGE(-(int)shift,+(int)shift);
         --sp;
         temp = *sp;
         MEMMOVE_PTR(sp-shift,sp-(shift+1),shift+1);
         *(sp-shift) = temp;
         ++sp;
         DISPATCH();
     }

     RAW_TARGET(ASM16_DUP_N) {
         uint16_t offset = READ_imm16();
         DREF DeeObject **pslot;
         ASSERT_USAGE(-((int)offset+2),+((int)offset+3));
         pslot = sp - (offset+2);
         PUSHREF(*pslot);
         DISPATCH();
         DISPATCH();
     }
     RAW_TARGET(ASM16_POP_N) {
         DREF DeeObject *old_object;
         uint16_t offset = READ_imm16();
         DREF DeeObject **pslot;
         ASSERT_USAGE(-((int)offset+2),+((int)offset+1));
         pslot = sp - (offset+2);
         old_object = *pslot;
         *pslot = TOP;
         Dee_Decref(old_object);
         POP();
         DISPATCH();
     }

     RAW_TARGET(ASM16_ADJSTACK) {
         imm_val = (uint16_t)(int16_t)READ_Simm16();
         goto do_stack_adjust;
     }
     TARGET(ASM16_SUPER_THIS_R,-0,+1) {
         imm_val = READ_imm16();
         goto do_super_this_r;
     }
     TARGET(ASM16_POP_STATIC,-1,+0) {
         imm_val = READ_imm16();
         goto do_pop_static;
     }
     TARGET(ASM16_POP_EXTERN,-1,+0) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_pop_extern;
     }
     TARGET(ASM16_POP_GLOBAL,-1,+0) {
         imm_val = READ_imm16();
         goto do_pop_global;
     }
     TARGET(ASM16_POP_LOCAL,-1,+0) {
         imm_val = READ_imm16();
         goto do_pop_local;
     }
     TARGET(ASM16_PUSH_REF,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_ref;
     }
     TARGET(ASM16_PUSH_ARG,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_arg;
     }
     TARGET(ASM16_PUSH_CONST,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_const;
     }
     TARGET(ASM16_PUSH_STATIC,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_static;
     }
     TARGET(ASM16_PUSH_EXTERN,-0,+1) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_push_extern;
     }
     TARGET(ASM16_PUSH_GLOBAL,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_global;
     }
     TARGET(ASM16_PUSH_LOCAL,-0,+1) {
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
     TARGET(ASM16_PUSH_MODULE,-0,+1) {
         imm_val = READ_imm16();
         goto do_push_module;
     }

     TARGET(ASM16_CLASS_C,-1,+1) {
         imm_val = READ_imm16();
         goto do_class_c;
     }
     TARGET(ASM16_CLASS_GC,-0,+1) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_class_gc;
     }
     TARGET(ASM16_CLASS_EC,-0,+1) {
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
          if unlikely(DeeObject_AssertTypeExact(descriptor,&DeeClassDescriptor_Type))
             new_class = NULL;
          else {
           new_class = DeeClass_New((DeeTypeObject *)base,descriptor);
          }
          Dee_Decref(descriptor);
         }
#else
         new_class = DeeClass_New((DeeTypeObject *)base,CONSTimm);
#endif
         Dee_Decref(base);
         if unlikely(!new_class)
            HANDLE_EXCEPT();
         PUSH((DREF DeeObject *)new_class); /* Inherit reference. */
         DISPATCH();
     }

     TARGET(ASM16_DEFMEMBER,-2,+1) {
         imm_val = READ_imm16();
         goto do_defmember;
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

     TARGET(ASM_REDUCE_MIN,-1,+1) {
         DREF DeeObject *result;
         result = DeeSeq_Min(TOP,NULL);
         if unlikely(!result) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = result; /* Inherit reference. */
         DISPATCH();
     }

     TARGET(ASM_REDUCE_MAX,-1,+1) {
         DREF DeeObject *result;
         result = DeeSeq_Max(TOP,NULL);
         if unlikely(!result) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = result; /* Inherit reference. */
         DISPATCH();
     }

     TARGET(ASM_REDUCE_SUM,-1,+1) {
         DREF DeeObject *result;
         result = DeeSeq_Sum(TOP);
         if unlikely(!result) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = result; /* Inherit reference. */
         DISPATCH();
     }

     TARGET(ASM_REDUCE_ANY,-1,+1) {
         int result = DeeSeq_Any(TOP);
         if unlikely(result < 0) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = DeeBool_For(result);
         Dee_Incref(TOP);
         DISPATCH();
     }

     TARGET(ASM_REDUCE_ALL,-1,+1) {
         int result = DeeSeq_All(TOP);
         if unlikely(result < 0) HANDLE_EXCEPT();
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
         ASSERT_USAGE(-0,+1);
#if __SIZEOF_SIZE_T__ <= 4
         if (range_end > SSIZE_MAX) {
          /* Special case: We must create an unsigned range, but
           *               the given target length doesn't fit.
           *               Therefor, we must create an object-style
           *               range in order to prevent the potential 
           *               overflow. */
          DREF DeeObject *ob_end = DeeInt_NewU32(range_end);
          if unlikely(!ob_end) HANDLE_EXCEPT();
#if 1     /* Both versions will result in the same behavior, but this
           * one is faster in the current implementation because `int'
           * actually doesn't implement inplace operations, meaning
           * that `tp_inc()' would otherwise have to be substitued with
           * a call to `tp_add()' using `DeeInt_One' during every iteration.
           * By passing `DeeInt_One' now, we can skip those checks later! */
          range_object = DeeRange_New(&DeeInt_Zero,ob_end,&DeeInt_One);
#else
          range_object = DeeRange_New(&DeeInt_Zero,ob_end,NULL);
#endif
          Dee_Decref(ob_end);
         } else
#endif
         {
          range_object = DeeRange_NewInt(0,(dssize_t)range_end,1);
         }
         if unlikely(!range_object) HANDLE_EXCEPT();
         PUSH(range_object);
         DISPATCH();
     }



     RAW_TARGET(ASM16_CALLATTR_C_KW) {
         uint8_t argc; DREF DeeObject *call_result;
         DeeObject **new_sp;
         imm_val  = READ_imm16();
         argc     = READ_imm8();
         imm_val2 = READ_imm16();
         ASSERT_USAGE(-((int)argc + 1),+1);
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
#else
         ASSERT_STRING(CONSTimm);
         call_result = DeeObject_CallAttrKw(new_sp[-1],
                                            CONSTimm,
                                            argc,
                                            new_sp,
                                            CONSTimm2);
#endif
         if unlikely(!call_result) HANDLE_EXCEPT();
         while (sp > new_sp) POPREF();
         Dee_Decref(TOP);
         TOP = call_result; /* Inherit reference. */
         DISPATCH();
     }
     RAW_TARGET(ASM16_CALLATTR_TUPLE_C_KW) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_callattr_tuple_c_kw;
     }
     RAW_TARGET(ASM_CALLATTR_KWDS) {
         DeeObject **new_sp;
         DREF DeeObject *call_result;
         imm_val = READ_imm8();
         ASSERT_USAGE(-(int)(imm_val + 3),+1);
         new_sp  = sp - (imm_val + 1);
         if unlikely(!DeeString_Check(new_sp[-1])) {
          err_expected_string_for_attribute(new_sp[-1]);
          HANDLE_EXCEPT();
         }
         call_result = DeeObject_CallAttrKw(new_sp[-2],
                                            new_sp[-1],
                                            imm_val,
                                            new_sp,
                                            TOP);
         if unlikely(!call_result) HANDLE_EXCEPT();
         while (sp > new_sp) POPREF();
         POPREF(); /* name */
         Dee_Decref(TOP);
         TOP = call_result;
         DISPATCH();
     }

     RAW_TARGET(ASM_CALLATTR_TUPLE_KWDS) {
         DREF DeeObject *call_result;
         ASSERT_USAGE(-4,+1);
         if unlikely(!DeeString_Check(THIRD)) {
          err_expected_string_for_attribute(THIRD);
          HANDLE_EXCEPT();
         }
         ASSERT_TUPLE(SECOND);
         call_result = DeeObject_CallAttrTupleKw(FOURTH,THIRD,SECOND,FIRST);
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
     TARGET(ASM16_CALLATTR_TUPLE_C,-2,+1) {
         imm_val = READ_imm16();
         goto do_callattr_tuple_c;
     }
     RAW_TARGET(ASM16_CALLATTR_THIS_C) {
         imm_val = READ_imm16();
         goto do_callattr_this_c;
     }
     TARGET(ASM16_CALLATTR_THIS_TUPLE_C,-1,+1) {
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
     TARGET(ASM_GETMEMBER,-1,+1) {
         DREF DeeObject *result;
         imm_val = READ_imm8();
do_getmember:
         ASSERT_THISCALL();
         result = DeeInstance_GetMemberSafe((DeeTypeObject *)TOP,THIS,
                                            (unsigned int)imm_val);
         if unlikely(!result) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = result; /* Inherit reference. */
         DISPATCH();
     }
     TARGET(ASM_BOUNDMEMBER,-1,+1) {
         int temp;
         imm_val = READ_imm8();
do_hasmember:
         ASSERT_THISCALL();
         temp = DeeInstance_BoundMemberSafe((DeeTypeObject *)TOP,THIS,
                                            (unsigned int)imm_val);
         if unlikely(temp < 0) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = DeeBool_For(temp);
         Dee_Incref(TOP);
         DISPATCH();
     }
     TARGET(ASM_DELMEMBER,-1,+0) {
         imm_val = READ_imm8();
do_delmember:
         ASSERT_THISCALL();
         if (DeeInstance_DelMemberSafe((DeeTypeObject *)TOP,THIS,
                                       (unsigned int)imm_val))
             HANDLE_EXCEPT();
         POPREF();
         DISPATCH();
     }
     TARGET(ASM_SETMEMBER,-2,+0) {
         imm_val = READ_imm8();
do_setmember:
         ASSERT_THISCALL();
         if (DeeInstance_SetMemberSafe((DeeTypeObject *)SECOND,THIS,
                                       (unsigned int)imm_val,FIRST))
             HANDLE_EXCEPT();
         POPREF();
         POPREF();
         DISPATCH();
     }
     TARGET(ASM16_GETMEMBER,-1,+1) {
         imm_val = READ_imm16();
         goto do_getmember;
     }
     TARGET(ASM16_BOUNDMEMBER,-1,+1) {
         imm_val = READ_imm16();
         goto do_hasmember;
     }
     TARGET(ASM16_DELMEMBER,-1,+0) {
         imm_val = READ_imm16();
         goto do_delmember;
     }
     TARGET(ASM16_SETMEMBER,-2,+0) {
         imm_val = READ_imm16();
         goto do_setmember;
     }

     TARGET(ASM16_GETMEMBER_R,-0,+1) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_getmember_r;
     }
     TARGET(ASM16_DELMEMBER_R,-0,+0) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_delmember_r;
     }
     TARGET(ASM16_SETMEMBER_R,-1,+0) {
         imm_val  = READ_imm16();
         imm_val2 = READ_imm16();
         goto do_setmember_r;
     }
     TARGET(ASM16_BOUNDMEMBER_R,-1,+0) {
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
     TARGET(ASM16_GETATTR_C,-1,+1) {
         imm_val = READ_imm16();
         goto do_getattr_c;
     }
     TARGET(ASM16_DELATTR_C,-1,+0) {
         imm_val = READ_imm16();
         goto do_delattr_c;
     }
     TARGET(ASM16_SETATTR_C,-2,+0) {
         imm_val = READ_imm16();
         goto do_setattr_c;
     }
     TARGET(ASM16_GETATTR_THIS_C,-0,+1) {
         imm_val = READ_imm16();
         goto do_getattr_this_c;
     }
     TARGET(ASM16_DELATTR_THIS_C,-0,+0) {
         imm_val = READ_imm16();
         goto do_delattr_this_c;
     }
     TARGET(ASM16_SETATTR_THIS_C,-1,+0) {
         imm_val = READ_imm16();
         goto do_setattr_this_c;
     }


     {   DREF DeeObject *hashset_object;
         RAW_TARGET(ASM16_PACK_HASHSET) imm_val = READ_imm16(); goto do_pack_set;
         RAW_TARGET(ASM_PACK_HASHSET) imm_val = READ_imm8();
do_pack_set:
         hashset_object = DeeHashSet_NewItemsInherited(imm_val,sp-imm_val);
         if unlikely(!hashset_object) HANDLE_EXCEPT();
         sp -= imm_val;
         PUSH(hashset_object);
         DISPATCH();
     }

     {   DREF DeeObject *dict_object;
     RAW_TARGET(ASM16_PACK_DICT) imm_val = READ_imm16(); goto do_pack_dict;
     RAW_TARGET(ASM_PACK_DICT)   imm_val = READ_imm8();
do_pack_dict:
         ASSERT_USAGE(-(int)(imm_val*2),+1);
         dict_object = DeeDict_NewKeyItemsInherited(imm_val,sp - (imm_val*2));
         if unlikely(!dict_object) HANDLE_EXCEPT();
         sp -= (imm_val*2); /* Adjust SP to pop items. */
         PUSH(dict_object); /* Inherit reference. */
         DISPATCH();
     }

     RAW_TARGET(ASM_CAST_DICT) {
         DREF DeeObject *dict_cast;
         ASSERT_USAGE(-1,+1);
         dict_cast = DeeDict_FromSequence(TOP);
         if unlikely(!dict_cast) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = dict_cast; /* Inherit reference. */
         DISPATCH();
     }

     TARGET(ASM_CAST_HASHSET,-1,+1) {
         DREF DeeObject *set_cast;
         set_cast = DeeHashSet_FromSequence(TOP);
         if unlikely(!set_cast) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = set_cast; /* Inherit reference */
         DISPATCH();
     }

     RAW_TARGET(ASM_ITERNEXT) {
         DREF DeeObject *iter_res;
         ASSERT_USAGE(-1,+1);
         iter_res = DeeObject_IterNext(TOP);
         if unlikely(!iter_res) HANDLE_EXCEPT();
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
         if (!(code->co_flags&CODE_FVARARGS))
             goto err_requires_varargs_code;
#else
         ASSERT(code->co_flags&CODE_FVARARGS);
#endif
         imm_val = READ_imm8();
         ASSERT_USAGE(-0,+imm_val);
         va_size = likely(frame->cf_argc > code->co_argc_max)
                 ? frame->cf_argc - code->co_argc_max
                 : 0;
         if (imm_val != va_size) {
          err_invalid_va_unpack_size(imm_val,va_size);
          HANDLE_EXCEPT();
         }
         for (va_size = 0; va_size < imm_val; ++va_size)
             PUSHREF(frame->cf_argv[code->co_argc_max + va_size]);
         DISPATCH();
     }

     TARGET(ASM_CALL_ARGSFWD,-1,+1) {
         DREF DeeObject *temp;
         uint8_t arg_lo = READ_imm8();
         uint8_t arg_hi = READ_imm8();
#ifdef EXEC_SAFE
         if (arg_lo > arg_hi)
             goto err_invalid_operands;
#else
         ASSERT(arg_lo <= arg_hi);
#endif
         if (arg_hi < code->co_argc_max) {
          if (arg_hi < frame->cf_argc) {
           /* Just a regular, old call-forward */
           temp = DeeObject_Call(TOP,
                                (size_t)(arg_hi-arg_lo)+1,
                                 frame->cf_argv+arg_lo);
          } else if (arg_lo >= frame->cf_argc) {
           /* Call using only default-arguments. */
           ASSERT(arg_lo >= code->co_argc_min);
           temp = DeeObject_Call(TOP,
                                (size_t)(arg_hi-arg_lo)+1,
                                (DeeObject **)code->co_defaultv+
                                (arg_lo - code->co_argc_min));
          } else {
           /* Partial call using both default, and given arguments. */
           size_t argc = (size_t)(arg_hi-arg_lo)+1;
           size_t fwd_argc = frame->cf_argc - arg_lo;
           temp = object_call_vec2(TOP,
                                   fwd_argc,
                                   frame->cf_argv + arg_lo,
                                   argc - fwd_argc,
                                  (DeeObject **)code->co_defaultv +
                                 ((code->co_argc_max-code->co_argc_min) - (argc-fwd_argc)));
          }
         } else {
          /* Special case: Varargs-inclusive. */
#ifdef EXEC_SAFE
          if (!(code->co_flags&CODE_FVARARGS))
              goto err_invalid_argument_index;
          if (arg_hi != code->co_argc_max)
              goto err_invalid_argument_index;
#else
          ASSERT(code->co_flags&CODE_FVARARGS);
          ASSERTF(arg_hi == code->co_argc_max,
                  "out-of-bounds argument index");
#endif
          temp = DeeObject_Call(TOP,
                                frame->cf_argc-arg_lo,
                                frame->cf_argv+arg_lo);
         }
         if unlikely(!temp) HANDLE_EXCEPT();
         Dee_Decref(TOP);
         TOP = temp; /* Inherit reference. */
         DISPATCH();
     }


     /* Variable argument API. */
     TARGET(ASM_VARARGS_GETSIZE,-0,+1) {
         DREF DeeObject *varsize;
#ifdef EXEC_SAFE
         if (!(code->co_flags&CODE_FVARARGS))
             goto err_invalid_argument_index;
#else
         ASSERT(code->co_flags&CODE_FVARARGS);
#endif
         ASSERT(frame->cf_argc >= code->co_argc_min);
         if (frame->cf_argc <= code->co_argc_max) {
          varsize = &DeeInt_Zero;
          Dee_Incref(varsize);
         } else {
          varsize = DeeInt_NewSize((size_t)(frame->cf_argc-code->co_argc_max));
          if unlikely(!varsize) HANDLE_EXCEPT();
         }
         PUSH(varsize); /* Inherit reference. */
         DISPATCH();
     }

     TARGET(ASM_VARARGS_CMP_EQ_SZ,-0,+1) {
         size_t va_size;
#ifdef EXEC_SAFE
         if (!(code->co_flags&CODE_FVARARGS))
             goto err_invalid_argument_index;
#else
         ASSERT(code->co_flags&CODE_FVARARGS);
#endif
         ASSERT(frame->cf_argc >= code->co_argc_min);
         if (frame->cf_argc <= code->co_argc_max) {
          va_size = 0;
         } else {
          va_size = (size_t)(frame->cf_argc-code->co_argc_max);
         }
         PUSHREF(DeeBool_For(va_size == READ_imm8()));
         DISPATCH();
     }

     TARGET(ASM_VARARGS_CMP_GR_SZ,-0,+1) {
         size_t va_size;
#ifdef EXEC_SAFE
         if (!(code->co_flags&CODE_FVARARGS))
             goto err_invalid_argument_index;
#else
         ASSERT(code->co_flags&CODE_FVARARGS);
#endif
         ASSERT(frame->cf_argc >= code->co_argc_min);
         if (frame->cf_argc <= code->co_argc_max) {
          va_size = 0;
         } else {
          va_size = (size_t)(frame->cf_argc-code->co_argc_max);
         }
         PUSHREF(DeeBool_For(va_size > READ_imm8()));
         DISPATCH();
     }


     TARGET(ASM_VARARGS_GETITEM,-1,+1) {
         size_t index;
#ifdef EXEC_SAFE
         if (!(code->co_flags&CODE_FVARARGS))
             goto err_invalid_argument_index;
#else
         ASSERT(code->co_flags&CODE_FVARARGS);
#endif
         if (DeeObject_AsSize(TOP,&index))
             HANDLE_EXCEPT();
         ASSERT(frame->cf_argc >= code->co_argc_min);
         index += code->co_argc_max;
         if (index >= frame->cf_argc) {
          err_va_index_out_of_bounds((size_t)(index-code->co_argc_max),
                                     (size_t)(frame->cf_argc-code->co_argc_max));
          HANDLE_EXCEPT();
         }
         /* Exchange the stack-top object */
         Dee_Decref(TOP);
         TOP = frame->cf_argv[index];
         Dee_Incref(TOP);
         DISPATCH();
     }

     TARGET(ASM_VARARGS_GETITEM_I,-0,+1) {
         size_t index;
         DeeObject *argobj;
         index = READ_imm8();
#ifdef EXEC_SAFE
         if (!(code->co_flags&CODE_FVARARGS))
             goto err_invalid_argument_index;
#else
         ASSERT(code->co_flags&CODE_FVARARGS);
#endif
         ASSERT(frame->cf_argc >= code->co_argc_min);
         index += code->co_argc_max;
         if (index >= frame->cf_argc) {
          size_t va_size = 0;
          if (frame->cf_argc > code->co_argc_max)
              va_size = (size_t)(frame->cf_argc-code->co_argc_max);
          err_va_index_out_of_bounds((size_t)(index-code->co_argc_max),va_size);
          HANDLE_EXCEPT();
         }
         /* Exchange the stack-top object */
         argobj = frame->cf_argv[index];
         PUSHREF(argobj);
         DISPATCH();
     }


#ifdef USE_SWITCH
     default:
         goto unknown_instruction;
#endif
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
#define PREFIX_TARGET(opcode) case (opcode) & 0xff:


 PREFIX_TARGET(ASM_JF16) {
     imm_val = (uint16_t)READ_Simm16();
     goto prefix_jf_16;
 }
 PREFIX_TARGET(ASM_JF) {
     /* Conditionally jump if true. */
     DREF DeeObject *prefix_ob; int temp;
     imm_val = (uint16_t)(int16_t)READ_Simm8();
prefix_jf_16:
     prefix_ob = get_prefix_object();
     if unlikely(!prefix_ob) HANDLE_EXCEPT();
     temp = DeeObject_Bool(prefix_ob);
     Dee_Decref(prefix_ob);
     if unlikely(temp < 0) HANDLE_EXCEPT();
     if (!temp) {
#ifndef CONFIG_NO_THREADS
      if ((int16_t)imm_val < 0 &&
           DeeThread_CheckInterruptSelf(this_thread))
           HANDLE_EXCEPT();
#endif
      ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
      goto assert_ip_bounds;
#else
      ASSERT(ip.ptr >= code->co_code);
      ASSERT(ip.ptr <  code->co_code+code->co_codebytes);
#endif
     }
     DISPATCH();
 }

 PREFIX_TARGET(ASM_JT16) {
     imm_val = (uint16_t)READ_Simm16();
     goto prefix_jt_16;
 }
 PREFIX_TARGET(ASM_JT) {
     /* Conditionally jump if true. */
     DREF DeeObject *prefix_ob; int temp;
     imm_val = (uint16_t)(int16_t)READ_Simm8();
prefix_jt_16:
     prefix_ob = get_prefix_object();
     if unlikely(!prefix_ob) HANDLE_EXCEPT();
     temp = DeeObject_Bool(prefix_ob);
     Dee_Decref(prefix_ob);
     if unlikely(temp < 0) HANDLE_EXCEPT();
     if (temp) {
#ifndef CONFIG_NO_THREADS
      if ((int16_t)imm_val < 0 &&
           DeeThread_CheckInterruptSelf(this_thread))
           HANDLE_EXCEPT();
#endif
      ip.ptr += (int16_t)imm_val;
#ifdef EXEC_SAFE
      goto assert_ip_bounds;
#else
      ASSERT(ip.ptr >= code->co_code);
      ASSERT(ip.ptr <  code->co_code+code->co_codebytes);
#endif
     }
     DISPATCH();
 }

 PREFIX_TARGET(ASM_FOREACH16) {
     imm_val = (uint16_t)READ_Simm16();
     goto prefix_foreach_16;
 }
 PREFIX_TARGET(ASM_FOREACH) {
     DREF DeeObject *elem,*prefix_ob;
     imm_val = (uint16_t)(int16_t)READ_Simm8();
prefix_foreach_16:
     ASSERT_USAGE(-0,+1);
     prefix_ob = get_prefix_object();
     if unlikely(!prefix_ob) HANDLE_EXCEPT();
     elem = DeeObject_IterNext(prefix_ob);
     Dee_Decref(prefix_ob);
     if unlikely(!elem) HANDLE_EXCEPT();
     if (elem == ITER_DONE)
         goto jump_16;
     /* Push the element. */
     PUSH(elem);
     DISPATCH();
 }

#define DEFINE_INPLACE_MATH_OPERATOR(ADD,Add) \
  PREFIX_TARGET(ASM_##ADD) \
  { \
      int error; \
      USING_PREFIX_OBJECT \
      ASSERT_USAGE(-1,+0); \
      prefix_ob = get_prefix_object(); \
      if unlikely(!prefix_ob) HANDLE_EXCEPT(); \
      error = DeeObject_Inplace##Add(&prefix_ob,TOP); \
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); } \
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT(); \
      POPREF(); \
      DISPATCH(); \
  }
  DEFINE_INPLACE_MATH_OPERATOR(ADD,Add)
  DEFINE_INPLACE_MATH_OPERATOR(SUB,Sub)
  DEFINE_INPLACE_MATH_OPERATOR(MUL,Mul)
  DEFINE_INPLACE_MATH_OPERATOR(DIV,Div)
  DEFINE_INPLACE_MATH_OPERATOR(MOD,Mod)
  DEFINE_INPLACE_MATH_OPERATOR(SHL,Shl)
  DEFINE_INPLACE_MATH_OPERATOR(SHR,Shr)
  DEFINE_INPLACE_MATH_OPERATOR(AND,And)
  DEFINE_INPLACE_MATH_OPERATOR(OR, Or)
  DEFINE_INPLACE_MATH_OPERATOR(XOR,Xor)
  DEFINE_INPLACE_MATH_OPERATOR(POW,Pow)
#undef DEFINE_INPLACE_MATH_OPERATOR

  PREFIX_TARGET(ASM_ADD_SIMM8) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceAddS8(&prefix_ob,READ_Simm8());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }
  PREFIX_TARGET(ASM_ADD_IMM32) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceAddInt(&prefix_ob,READ_imm32());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_SUB_SIMM8) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceSubS8(&prefix_ob,READ_Simm8());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }
  PREFIX_TARGET(ASM_SUB_IMM32) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceSubInt(&prefix_ob,READ_imm32());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_MUL_SIMM8) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceMulInt(&prefix_ob,READ_Simm8());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_DIV_SIMM8) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceDivInt(&prefix_ob,READ_Simm8());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_MOD_SIMM8) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceModInt(&prefix_ob,READ_Simm8());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_AND_IMM32) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceAndInt(&prefix_ob,READ_imm32());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_OR_IMM32) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceOrInt(&prefix_ob,READ_imm32());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_XOR_IMM32) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceXorInt(&prefix_ob,READ_imm32());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_SHL_IMM8) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceShlInt(&prefix_ob,READ_imm8());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_SHR_IMM8) {
      int error;
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      error = DeeObject_InplaceShrInt(&prefix_ob,READ_imm8());
      if unlikely(error) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
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
      ASSERT_USAGE(-(int)(imm_val2 + 1),+0);
      ASSERT_CONSTimm();
#ifdef EXEC_SAFE
      {
       DREF DeeObject *code_object;
       CONST_LOCKREAD();
       code_object = CONSTimm;
       Dee_Incref(code_object);
       CONST_LOCKENDREAD();
       if (DeeObject_AssertTypeExact(code_object,&DeeCode_Type)) {
        Dee_Decref(code_object);
        HANDLE_EXCEPT();
       }
       if (((DeeCodeObject *)code_object)->co_refc != imm_val2+1) {
        err_invalid_refs_size(code_object,imm_val2+1);
        Dee_Decref(code_object);
        HANDLE_EXCEPT();
       }
       function = DeeFunction_NewInherited(code_object,
                                           imm_val2+1,
                                           sp-(imm_val2+1));
       Dee_Decref(code_object);
      }
#else
      function = DeeFunction_NewInherited(CONSTimm,
                                          imm_val2+1,
                                          sp-(imm_val2+1));
#endif
      if unlikely(!function) HANDLE_EXCEPT();
      sp -= imm_val2+1;
      if unlikely(set_prefix_object(function))
         HANDLE_EXCEPT();
      DISPATCH();
  }


  PREFIX_TARGET(ASM_INC) {
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      if unlikely(DeeObject_Inc(&prefix_ob)) {
       Dee_Decref(prefix_ob);
       HANDLE_EXCEPT();
      }
      if unlikely(set_prefix_object(prefix_ob))
         HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_DEC) {
      USING_PREFIX_OBJECT
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      if unlikely(DeeObject_Dec(&prefix_ob)) {
       Dee_Decref(prefix_ob);
       HANDLE_EXCEPT();
      }
      if unlikely(set_prefix_object(prefix_ob))
         HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_OPERATOR) {
      DREF DeeObject *call_result;
      USING_PREFIX_OBJECT
      imm_val  = READ_imm8();
      imm_val2 = READ_imm8();
do_prefix_operator:
      ASSERT_USAGE(-(int)imm_val2,+1);
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      call_result = DeeObject_PInvokeOperator(&prefix_ob,imm_val,
                                               imm_val2,sp-imm_val2);
      if unlikely(!call_result) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      while (imm_val2--) POPREF();
      PUSH(call_result);
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_OPERATOR_TUPLE) {
      DREF DeeObject *call_result;
      USING_PREFIX_OBJECT
      imm_val = READ_imm8();
do_prefix_operator_tuple:
      ASSERT_USAGE(-1,+1);
      ASSERT_TUPLE(TOP);
      prefix_ob = get_prefix_object();
      if unlikely(!prefix_ob) HANDLE_EXCEPT();
      call_result = DeeObject_PInvokeOperator(&prefix_ob,
                                               imm_val,
                                               DeeTuple_SIZE(TOP),
                                               DeeTuple_ELEM(TOP));
      if unlikely(!call_result) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
      Dee_Decref(TOP);
      TOP = call_result; /* Inherit reference. */
      if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_UNPACK) {
      int error;
      DREF DeeObject *sequence;
      imm_val = READ_imm8();
prefix_do_unpack:
      ASSERT_USAGE(-0,+(int)imm_val);
      sequence = get_prefix_object();
      if unlikely(!sequence) HANDLE_EXCEPT();
      error = DeeObject_Unpack(sequence,imm_val,sp);
      Dee_Decref(sequence);
      if unlikely(error) HANDLE_EXCEPT();
      sp += imm_val;
      DISPATCH();
  }

  PREFIX_TARGET(ASM_EXTENDED1) {
      switch (*ip.ptr++) {

      PREFIX_TARGET(ASM16_NOP)
      PREFIX_TARGET(ASM16_DELOP)
          DISPATCH();

      PREFIX_TARGET(ASM16_OPERATOR)
          imm_val  = READ_imm16();
          imm_val2 = READ_imm8();
          goto do_prefix_operator;

      PREFIX_TARGET(ASM16_OPERATOR_TUPLE)
          imm_val  = READ_imm16();
          goto do_prefix_operator_tuple;

      PREFIX_TARGET(ASM16_FUNCTION_C)
          imm_val = READ_imm16();
          imm_val2 = READ_imm8();
          goto prefix_do_function_c;

      PREFIX_TARGET(ASM16_FUNCTION_C_16)
          imm_val = READ_imm16();
          imm_val2 = READ_imm16();
          goto prefix_do_function_c;

      PREFIX_TARGET(ASM16_UNPACK)
          imm_val = READ_imm16();
          goto prefix_do_unpack;

      PREFIX_TARGET(ASM_INCPOST) { /* incpost */
          DREF DeeObject *obcopy;
          USING_PREFIX_OBJECT
          ASSERT_USAGE(-0,+1);
          prefix_ob = get_prefix_object();
          if unlikely(!prefix_ob) HANDLE_EXCEPT();
          obcopy = DeeObject_Copy(prefix_ob);
          if unlikely(!obcopy) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
          PUSH(obcopy); /* Inherit reference. */
          if (DeeObject_Inc(&prefix_ob)) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
          if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
          DISPATCH();
      }


      PREFIX_TARGET(ASM_DECPOST) { /* incpost */
          DREF DeeObject *obcopy;
          USING_PREFIX_OBJECT
          ASSERT_USAGE(-0,+1);
          prefix_ob = get_prefix_object();
          if unlikely(!prefix_ob) HANDLE_EXCEPT();
          obcopy = DeeObject_Copy(prefix_ob);
          if unlikely(!obcopy) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
          PUSH(obcopy); /* Inherit reference. */
          if (DeeObject_Dec(&prefix_ob)) { Dee_Decref(prefix_ob); HANDLE_EXCEPT(); }
          if unlikely(set_prefix_object(prefix_ob)) HANDLE_EXCEPT();
          DISPATCH();
      }

      PREFIX_TARGET(ASM16_LROT) {
          DREF DeeObject *drop_object;
          DREF DeeObject *append_object;
          uint32_t shift = (uint32_t)READ_imm16()+2;
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
          ASSERT_USAGE(-(int)shift,+(int)shift);
          drop_object = *(sp - shift);
          MEMMOVE_PTR(sp-shift,sp-(shift-1),shift-1);
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
          uint32_t shift = (uint32_t)READ_imm16()+2;
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
          ASSERT_USAGE(-(int)shift,+(int)shift);
          drop_object = TOP;
          MEMMOVE_PTR(sp-(shift-1),sp-shift,shift-1);
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
          if (set_prefix_object(temp)) HANDLE_EXCEPT();
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
          ASSERT_USAGE(-((int)offset+2),+((int)offset+2));
          slot = *(sp - (offset+2));
          Dee_Incref(slot);
          if (set_prefix_object(slot))
              HANDLE_EXCEPT();
          DISPATCH();
      }
      PREFIX_TARGET(ASM16_POP_N) {
          DREF DeeObject *value,*old_value;
          DREF DeeObject **pslot;
          uint16_t offset = READ_imm16();
          ASSERT_USAGE(-((int)offset+2),+((int)offset+2));
          value = get_prefix_object();
          if unlikely(!value) HANDLE_EXCEPT();
          pslot = sp - (offset+2);
          old_value = *pslot;
          *pslot = value; /* Inherit reference. */
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
      ASSERT_USAGE(-1,+1);
      new_top = xch_prefix_object(TOP);
      if unlikely(!new_top)
         HANDLE_EXCEPT();
      TOP = new_top; /* Inherit reference. */
      DISPATCH();
  }

  PREFIX_TARGET(ASM_LROT) {
      DREF DeeObject *drop_object;
      DREF DeeObject *append_object;
      uint16_t shift = (uint16_t)READ_imm8()+2;
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
      ASSERT_USAGE(-(int)shift,+(int)shift);
      drop_object = *(sp - shift);
      MEMMOVE_PTR(sp-shift,sp-(shift-1),shift-1);
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
      uint16_t shift = (uint16_t)READ_imm8()+2;
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
      ASSERT_USAGE(-(int)shift,+(int)shift);
      drop_object = TOP;
      MEMMOVE_PTR(sp-(shift-1),sp-shift,shift-1);
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
      if unlikely(!value) HANDLE_EXCEPT();
      STATIC_LOCKWRITE();
      old_value = STATICimm;
      STATICimm = value; /* Inherit reference. */
      STATIC_LOCKENDWRITE();
      ASSERT_OBJECT(old_value);
      Dee_Decref(old_value);
      DISPATCH();
  }

  PREFIX_TARGET(ASM_POP_EXTERN) {
      DeeObject *old_value,**pglobl;
      DREF DeeObject *value;
      imm_val  = READ_imm8();
      imm_val2 = READ_imm8();
do_prefix_pop_extern:
      ASSERT_EXTERNimm();
      value = get_prefix_object();
      if unlikely(!value) HANDLE_EXCEPT();
      EXTERN_LOCKWRITE();
      pglobl    = &EXTERNimm;
      old_value = *pglobl;
      *pglobl   = value; /* Inherit reference. */
      EXTERN_LOCKENDWRITE();
      Dee_XDecref(old_value);
      DISPATCH();
  }

  PREFIX_TARGET(ASM_POP_GLOBAL) {
      DeeObject *old_value,**pglobl;
      DREF DeeObject *value;
      imm_val = READ_imm8();
do_prefix_pop_global:
      ASSERT_GLOBALimm();
      value = get_prefix_object();
      if unlikely(!value) HANDLE_EXCEPT();
      GLOBAL_LOCKWRITE();
      pglobl    = &GLOBALimm;
      old_value = *pglobl;
      *pglobl   = value; /* Inherit reference. */
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
      if unlikely(!value) HANDLE_EXCEPT();
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
#else
      ASSERT(imm_val < mod->mo_importc);
#endif
      mod = mod->mo_importv[imm_val];
      ASSERT_OBJECT(mod);
      Dee_Incref(mod);
      if (set_prefix_object((DeeObject *)mod))
          HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_PUSH_NONE) {
      Dee_Incref(Dee_None);
      if (set_prefix_object(Dee_None)) HANDLE_EXCEPT();
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
      ASSERT(code->co_argc_max >= code->co_argc_min);
      if (imm_val < code->co_argc_max) {
       /* Simple case: Direct argument/default reference. */
       if (imm_val < frame->cf_argc) {
        value = frame->cf_argv[imm_val];
       } else {
        /* TODO: Keyword argument support? */
        ASSERT(imm_val >= code->co_argc_min);
        value = code->co_defaultv[imm_val-code->co_argc_min];
       }
      } else {
       /* Special case: Varargs. */
#ifdef EXEC_SAFE
       if (!(code->co_flags&CODE_FVARARGS))
           goto err_invalid_argument_index;
       if (imm_val != code->co_argc_max)
           goto err_invalid_argument_index;
#else
       ASSERT(code->co_flags&CODE_FVARARGS);
       ASSERTF(imm_val == code->co_argc_max,
               "out-of-bounds argument index");
#endif
       ASSERT(frame->cf_argc >= code->co_argc_min);
       if (!frame->cf_vargs) {
        if (frame->cf_argc <= code->co_argc_max) {
         frame->cf_vargs = (DREF DeeTupleObject *)Dee_EmptyTuple;
         Dee_Incref(Dee_EmptyTuple);
        } else {
         frame->cf_vargs = (DREF DeeTupleObject *)
          DeeTuple_NewVector((size_t)(frame->cf_argc-code->co_argc_max),
                                      frame->cf_argv+code->co_argc_max);
         if unlikely(!frame->cf_vargs) HANDLE_EXCEPT();
        }
       }
       value = (DeeObject *)frame->cf_vargs;
      }
      Dee_Incref(value);
      if (set_prefix_object(value))
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
      ASSERT_USAGE(-((int)offset+2),+((int)offset+2));
      slot = *(sp - (offset+2));
      Dee_Incref(slot);
      if (set_prefix_object(slot))
          HANDLE_EXCEPT();
      DISPATCH();
  }

  PREFIX_TARGET(ASM_POP) {
      DREF DeeObject *value;
      ASSERT_USAGE(-1,+1);
      value = get_prefix_object();
      if unlikely(!value) HANDLE_EXCEPT();
      Dee_Decref(TOP);
      TOP = value; /* Inherit reference. */
      DISPATCH();
  }

  PREFIX_TARGET(ASM_POP_N) {
      DREF DeeObject *value,*old_value;
      DREF DeeObject **pslot;
      uint8_t offset = READ_imm8();
      ASSERT_USAGE(-((int)offset+2),+((int)offset+2));
      value = get_prefix_object();
      if unlikely(!value) HANDLE_EXCEPT();
      pslot = sp - (offset+2);
      old_value = *pslot;
      *pslot = value; /* Inherit reference. */
      Dee_Decref(old_value);
      DISPATCH();
  }

  PREFIX_TARGET(ASM_RET) {
      DREF DeeObject *value;
      value = get_prefix_object();
      if unlikely(!value) HANDLE_EXCEPT();
      /* Check if we're overwriting a previous return value
       * (which can happen when `return' appears in a finally-block) */
      if (ITER_ISOK(frame->cf_result))
          Dee_Decref(frame->cf_result);
      frame->cf_result = value;
      if (code->co_flags&CODE_FYIELDING)
          goto end_without_finally;
      goto end_return;
  }

  PREFIX_TARGET(ASM_YIELDALL) {
      DREF DeeObject *value;
      value = get_prefix_object();
      if unlikely(!value) HANDLE_EXCEPT();
      ASSERT_YIELDING();
      if (ITER_ISOK(frame->cf_result))
          Dee_Decref(frame->cf_result);
      frame->cf_result = DeeObject_IterNext(value);
      Dee_Decref(value);
      if unlikely(!frame->cf_result) HANDLE_EXCEPT();
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
      if unlikely(!value) HANDLE_EXCEPT();
      DeeError_Throw(value);
      Dee_Decref(value);
      HANDLE_EXCEPT();
  }



  default:
prefix_unknown_instruction:
      err_illegal_instruction(code,frame->cf_ip);
      HANDLE_EXCEPT();
  }

  /* Shouldn't get here. */
  __builtin_unreachable();
 } /* End of prefixed instruction handling. */


#ifdef USE_SWITCH
 default:
#endif
#ifndef USE_SWITCH
target_ASM_UD:
target_ASM_RESERVED1:
target_ASM_RESERVED2:
target_ASM_RESERVED3:
target_ASM_RESERVED4:
target_ASM_RESERVED5:
target_ASM_RESERVED6:
target_ASM_RESERVED7:
#endif
unknown_instruction:
  err_illegal_instruction(code,frame->cf_ip);
  HANDLE_EXCEPT();
 }

 /* Opcode handlers must continue execution using
  * `DISPATCH()', so we must not be able to get here. */
 __builtin_unreachable();

{
 struct except_handler *current_except;
end_return:
 if (code->co_flags&CODE_FFINALLY) {
  code_addr_t ip_addr = (code_addr_t)(frame->cf_ip - code->co_code);
  ASSERT(code->co_exceptc != 0);
  /* Execute finally handlers. */
  current_except = code->co_exceptv+code->co_exceptc;
  while (current_except-- != code->co_exceptv) {
   if (current_except->eh_flags&EXCEPTION_HANDLER_FFINALLY &&
       ip_addr >= current_except->eh_start &&
       ip_addr <  current_except->eh_end) {
    /* Found a handler that must be executed. */
    goto exec_except;
   }
  }
 }
end_without_finally:

 /* Store exit IP/SP */
 frame->cf_ip = ip.ptr;
 frame->cf_sp = sp;

 /* Unhook frame from the thread-local execution stack. */
 ASSERT(this_thread->t_execsz != 0);
 ASSERT(this_thread->t_exec == frame);
 ASSERT(frame->cf_prev != CODE_FRAME_NOT_EXECUTING);
 --this_thread->t_execsz;
 this_thread->t_exec = frame->cf_prev;
 frame->cf_prev = CODE_FRAME_NOT_EXECUTING;

#ifdef EXEC_FAST
end_nounhook:
#endif
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
  if unlikely(this_thread->t_exceptsz > except_recursion+1) {
   uint16_t num_discard = (uint16_t)(this_thread->t_exceptsz-(except_recursion+1));
   do DeeError_Print("Discarding secondary error\n",ERROR_PRINT_DOHANDLE);
   while (--num_discard);
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
          "No new exceptions have been thrown");
  ASSERTF(this_thread->t_except,"No error has been set");
  /* Lazily allocate a missing traceback.
   * TODO: Only include information that would become lost _now_ in the traceback.
   *       All other information should be added as the stack continues being
   *       unwound, thus allowing for O(1) exceptions in small helper functions,
   *       regardless of how deep the current execution stack actually is! */
  if (this_thread->t_except->ef_trace == (DREF DeeTracebackObject *)ITER_DONE)
      this_thread->t_except->ef_trace = DeeTraceback_New(this_thread);
  current_exception = this_thread->t_except->ef_error;
  current_except = code->co_exceptv+code->co_exceptc;
  while (current_except-- != code->co_exceptv) {
   if (ip_addr >= current_except->eh_start &&
       ip_addr <  current_except->eh_end &&
       /* Special case: interrupt objects can only be caught by interrupt-handlers. */
     (!DeeType_IsInterrupt(Dee_TYPE(current_exception)) ||
      (current_except->eh_flags&EXCEPTION_HANDLER_FINTERPT)) &&
     (!current_except->eh_mask ||
       DeeObject_InstanceOf(current_exception,
                            current_except->eh_mask)))
       goto exec_except_maybe_handle; /* Execute this one! */
  }
 }
 if (ITER_ISOK(frame->cf_result))
     Dee_Decref(frame->cf_result);
 frame->cf_result = NULL;
 ip.ptr = frame->cf_ip;
 goto end_return;
exec_except_maybe_handle:
 /* If the exception handler requests it,
  * already handle the error beforehand. */
 if (current_except->eh_flags & EXCEPTION_HANDLER_FHANDLED)
     DeeError_Handled(ERROR_HANDLED_INTERRUPT);
exec_except:
 ASSERTF(current_except->eh_addr <  current_except->eh_start ||
         current_except->eh_addr >= current_except->eh_end,
         "An exception handler must not be used to protect itself. "
         "Such constructs would lead to infinite recursion.");
 frame->cf_sp = sp;
 { struct except_frame *iter;
   /* Use this point to extend upon tracebacks!
    * Considering the lazy updating of code-frame SP/IP, any code
    * that causes an exception can no longer rely upon the validity
    * of the stack/instruction pointers it encounters on the stack.
    * (Well... It can read the start-ip register safely, as this
    *  implementation stores that one within the frame)
    * Instead, we dynamically add upon tracebacks here, filling in
    * anything left undefined until now as the stack is unwound! */
   ASSERT(this_thread->t_execsz);
   iter = this_thread->t_except;
   for (; iter; iter = iter->ef_prev) {
    if (!ITER_ISOK(iter->ef_trace))
         continue;
    if unlikely(iter->ef_trace->tb_thread != this_thread) continue;
    DeeTraceback_AddFrame(iter->ef_trace,frame,
                          this_thread->t_execsz-1);
   }
 }
 ip.ptr = code->co_code+current_except->eh_addr;
#ifdef EXEC_SAFE
 if unlikely(ip.ptr <  code->co_code ||
             ip.ptr >= code->co_code+code->co_codebytes)
    goto err_invalid_ip;
#else
 ASSERT(ip.ptr >= code->co_code &&
        ip.ptr <  code->co_code+code->co_codebytes);
#endif
 /* Adjust the stack according to requirements imposed by this exception handler. */
 {
  uint16_t cur_depth = (uint16_t)(sp - frame->cf_stack);
  uint16_t new_depth = current_except->eh_stack;
  if (cur_depth > new_depth) {
   cur_depth -= new_depth;
   while (cur_depth--) POPREF();
  } else /*if (cur_depth < new_depth)*/ {
   /* Push `none' to adjust the stack. */
   new_depth -= cur_depth;
   while (new_depth--) PUSHREF(Dee_None);
  }
  ASSERT(current_except->eh_stack == 
        (uint16_t)(sp - frame->cf_stack));
 }
#ifndef CONFIG_NO_THREADS
 /* With the new IP address active, check for interrupts to prevent
  * potential loop constructs using exceptions of all things... */
 if (DeeThread_CheckInterruptSelf(this_thread))
     goto handle_except;
#endif

 /* Continue execution within this exception handler. */
 goto next_instr;
} /* End of local variables scope. */

 /* --- Exception creation --- */

 /* Invalid use errors. */
#ifdef EXEC_SAFE
increase_stacksize:
 ip.ptr = frame->cf_ip;
 /* If lenient mode isn't enabled, then we can't dynamically grow the stack. */
 if (!(code->co_flags&CODE_FLENIENT)) goto stack_fault;
 { DeeObject **new_stack;
   uint16_t new_size = frame->cf_stacksz;
   /* Determine the new stack size. */
   if (!new_size) new_size = STACKPREALLOC;
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
    DeeError_Throwf(&DeeError_SegFault,"Stack segment overflow");
    HANDLE_EXCEPT();
   }
   ASSERT(new_size);
   /* Allocate/Re-allocate the stack on the heap. */
   if (frame->cf_stacksz) {
    new_stack = (DeeObject **)Dee_Realloc(frame->cf_stack,
                                         (size_t)new_size*sizeof(DeeObject *));
   } else {
    new_stack = (DeeObject **)Dee_Malloc((size_t)new_size*sizeof(DeeObject *));
   }
   if unlikely(!new_stack) HANDLE_EXCEPT();
   /* Install the new stack. */
   if (!frame->cf_stacksz) {
    /* Copy the old contents of the stack onto the new one. */
    MEMCPY_PTR(new_stack,frame->cf_stack,sp-frame->cf_stack);
   }
   /* Hook the new stack. */
   frame->cf_stacksz = new_size; /* A non-zero `cf_stacksz' value indicates a heap stack! */
   sp = new_stack+(sp-frame->cf_stack);
   frame->cf_stack = new_stack;
   frame->cf_sp    = sp; /* Set a new frame-sp that is part of the actual stack. */
   /* Try execute the current instruction again. */
   goto next_instr;
 }
err_invalid_stack_affect:
 DeeError_Throwf(&DeeError_SegFault,"Invalid stack effect at +%.4I32X",
                (code_addr_t)(frame->cf_ip-code->co_code));
 HANDLE_EXCEPT();
 { DeeTypeObject *required_type;
err_requires_tuple:  required_type = &DeeTuple_Type; goto illegal_type;
err_requires_string: required_type = &DeeString_Type;
illegal_type:
   DeeError_Throwf(&DeeError_TypeError,
                   "Instruction requires an instance of `%k'",
                   required_type);
   HANDLE_EXCEPT();
 }
err_invalid_ref:    frame->cf_sp = sp; err_srt_invalid_ref(frame,imm_val); HANDLE_EXCEPT();
err_invalid_module: frame->cf_sp = sp; err_srt_invalid_module(frame,imm_val); HANDLE_EXCEPT();
err_invalid_extern: frame->cf_sp = sp; err_srt_invalid_extern(frame,imm_val,imm_val2); HANDLE_EXCEPT();
err_invalid_global: frame->cf_sp = sp; err_srt_invalid_global(frame,imm_val); HANDLE_EXCEPT();
err_invalid_locale: frame->cf_sp = sp; err_srt_invalid_locale(frame,imm_val); HANDLE_EXCEPT();
err_invalid_const:  frame->cf_sp = sp; err_srt_invalid_const(frame,imm_val); HANDLE_EXCEPT();
err_invalid_static: frame->cf_sp = sp; err_srt_invalid_static(frame,imm_val); HANDLE_EXCEPT();
err_invalid_instance_addr:
err_invalid_ip:
err_invalid_operands:
err_requires_varargs_code:
err_requires_yield_code:
err_requires_thiscall_code:
err_invalid_argument_index:
err_cannot_push_exception:
 err_illegal_instruction(code,frame->cf_ip);
 HANDLE_EXCEPT();
#endif
err_unbound_extern:
 ASSERT(imm_val  <= code->co_module->mo_importc);
 ASSERT(imm_val2 <= code->co_module->mo_importv[imm_val]->mo_globalc);
 err_unbound_global(code->co_module->mo_importv[imm_val],imm_val2);
 HANDLE_EXCEPT();
err_unbound_global:
 err_unbound_global(code->co_module,imm_val);
 HANDLE_EXCEPT();
err_unbound_local:
 err_unbound_local(code,frame->cf_ip,imm_val);
 HANDLE_EXCEPT();
}

DECL_END

#ifdef _MSC_VER
#pragma optimize("", on)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
#undef set_prefix_object
#undef xch_prefix_object
#undef get_prefix_object
