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
#ifndef GUARD_DEEMON_EXECUTE_CODE_C
#define GUARD_DEEMON_EXECUTE_CODE_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/arg.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/bool.h>
#include <deemon/gc.h>
#include <deemon/thread.h>
#include <deemon/none.h>
#include <deemon/error.h>
#include <deemon/code.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/int.h>
#include <deemon/asm.h>
#include <deemon/util/string.h>

#include <hybrid/unaligned.h>
#include <hybrid/byteswap.h>
#include <hybrid/byteorder.h>
#include <stdint.h>

#include "../objects/seq/svec.h"
#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"


#ifdef CONFIG_HAVE_EXEC_ALTSTACK
#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#undef THIS
#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif


#if defined(_MSC_VER) && defined(__x86_64__)
/* The x86_64,msvc version is implemented in `asm/altstack-x86_64.S' */
#define CONFIG_EXEC_ALTSTACK_USES_EXTERNAL_ASSEMBLY 1
#endif

DECL_BEGIN

#ifdef CONFIG_HOST_WINDOWS
#define ALTSTACK_ALLOC_FAILED   NULL
#elif defined(MAP_FAILED)
#define ALTSTACK_ALLOC_FAILED   MAP_FAILED
#else
#define ALTSTACK_ALLOC_FAILED ((void *)-1)
#endif

PRIVATE void *DCALL tryalloc_altstack(void) {
#ifdef CONFIG_HOST_WINDOWS
 return VirtualAlloc(NULL,
                     DEE_EXEC_ALTSTACK_SIZE,
                     MEM_COMMIT|MEM_RESERVE,
                     PAGE_READWRITE);
#else
#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif
#ifndef MAP_PRIVATE
#define MAP_PRIVATE   0
#endif
#ifndef MAP_GROWSDOWN
#define MAP_GROWSDOWN 0
#endif
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#ifndef MAP_STACK
#define MAP_STACK 0
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#ifdef MAP_ANONYMOUS
 return mmap(NULL,
             DEE_EXEC_ALTSTACK_SIZE,
             PROT_READ|PROT_WRITE,
             MAP_ANONYMOUS|MAP_PRIVATE|MAP_GROWSDOWN|MAP_STACK|MAP_UNINITIALIZED,
             -1,
             0);
#else
 void *result;
 int fd = open("/dev/null",O_RDONLY);
 if unlikely(fd < 0) return ALTSTACK_ALLOC_FAILED;
 result = mmap(NULL,
               DEE_EXEC_ALTSTACK_SIZE,
               PROT_READ|PROT_WRITE,
               MAP_FILE|MAP_PRIVATE|MAP_GROWSDOWN|MAP_STACK,
               fd,
               0);
 close(fd);
 return result;
#endif
#endif
}

#ifdef CONFIG_EXEC_ALTSTACK_USES_EXTERNAL_ASSEMBLY
INTERN void DCALL free_altstack(void *stack)
#else
PRIVATE void DCALL free_altstack(void *stack)
#endif
{
#ifdef CONFIG_HOST_WINDOWS
 VirtualFree(stack,DEE_EXEC_ALTSTACK_SIZE,MEM_RELEASE);
#else
 munmap(stack,DEE_EXEC_ALTSTACK_SIZE);
#endif
}



#ifdef CONFIG_EXEC_ALTSTACK_USES_EXTERNAL_ASSEMBLY
INTERN void *DCALL alloc_altstack(void)
#else
PRIVATE void *DCALL alloc_altstack(void)
#endif
{
 void *result;
again:
 result = tryalloc_altstack();
 if unlikely(result == ALTSTACK_ALLOC_FAILED) {
  if (DeeMem_ClearCaches((size_t)-1))
      goto again;
  Dee_BadAlloc(DEE_EXEC_ALTSTACK_SIZE);
 }
 return result;
}



#ifndef CONFIG_EXEC_ALTSTACK_USES_EXTERNAL_ASSEMBLY
#ifndef __USER_LABEL_PREFIX__
#ifdef CONFIG_HOST_WINDOWS
#define __USER_LABEL_PREFIX__ _
#else
#define __USER_LABEL_PREFIX__ /* nothing */
#endif
#endif


#ifdef __x86_64__
#define WRAP_SYMBOL(s) PP_STR(__USER_LABEL_PREFIX__) #s
#ifdef __COMPILER_HAVE_GCC_ASM
#define CALL_WITH_STACK(result,func,frame,new_stack) \
 __asm__("push{q} {%%rbp|rbp}\n\t" \
         "mov{q}  {%%rsp, %%rbp|rbp, rsp}\n\t" \
         "lea{q}  {" PP_STR(DEE_EXEC_ALTSTACK_SIZE) "(%2), %%rsp|" \
                  "rsp, [%2 + " PP_STR(DEE_EXEC_ALTSTACK_SIZE) "]}\n\t" \
         "call    " WRAP_SYMBOL(func) "\n\t" \
         "mov{q}  {%%rbp, %%rsp|rsp, rbp}\n\t" \
         "pop{q}  {%%rbp|rbp}\n\t" \
         : "=a" (result) \
         : "c" (frame) \
         , "r" (new_stack) \
         : "memory", "cc")
#else
#define CALL_WITH_STACK(result,func,frame,new_stack) \
 __asm { \
     __asm PUSH RBX \
     __asm MOV  RAX, new_stack \
     __asm MOV  RCX, frame \
     __asm MOV  RBX, RSP \
     __asm LEA  RSP, [RAX + DEE_EXEC_ALTSTACK_SIZE] \
     __asm CALL func \
     __asm MOV  RSP, RBX \
     __asm POP  RBX \
     __asm MOV  result, RAX \
 }
#endif
#elif defined(__i386__)
#ifdef CONFIG_HOST_WINDOWS
#define WRAP_SYMBOL(s) "@" #s "@4"
#else
#define WRAP_SYMBOL(s) PP_STR(__USER_LABEL_PREFIX__) #s
#endif
#ifdef __COMPILER_HAVE_GCC_ASM
#define CALL_WITH_STACK(result,func,frame,new_stack) \
 __asm__("push{l} {%%ebp|ebp}\n\t" \
         "mov{l}  {%%esp, %%ebp|ebp, esp}\n\t" \
         "lea{l}  {" PP_STR(DEE_EXEC_ALTSTACK_SIZE) "(%2), %%esp|" \
                  "esp, [%2 + " PP_STR(DEE_EXEC_ALTSTACK_SIZE) "]}\n\t" \
         "call    " WRAP_SYMBOL(func) "\n\t" \
         "mov{l}  {%%ebp, %%esp|esp, ebp}\n\t" \
         "pop{l}  {%%ebp|ebp}\n\t" \
         : "=a" (result) \
         : "c" (frame) \
         , "r" (new_stack) \
         : "memory", "cc")
#else
#define CALL_WITH_STACK(result,func,frame,new_stack) \
 __asm { \
     __asm PUSH EBX \
     __asm MOV  EAX, new_stack \
     __asm MOV  ECX, frame \
     __asm MOV  EBX, ESP \
     __asm LEA  ESP, [EAX + DEE_EXEC_ALTSTACK_SIZE] \
     __asm CALL func \
     __asm MOV  ESP, EBX \
     __asm POP  EBX \
     __asm MOV  result, EAX \
 }
#endif
#else
#error "Unsupported Architecture. - Please disable `CONFIG_HAVE_EXEC_ALTSTACK'"
#endif


DFUNDEF DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameFastAltStack(struct code_frame *__restrict frame) {
 DREF DeeObject *result;
 void *new_stack = alloc_altstack();
 if unlikely(new_stack == ALTSTACK_ALLOC_FAILED)
    return NULL;
 CALL_WITH_STACK(result,DeeCode_ExecFrameFast,frame,new_stack);
 free_altstack(new_stack);
 return result;
}

DFUNDEF DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameSafeAltStack(struct code_frame *__restrict frame) {
 DREF DeeObject *result;
 void *new_stack = alloc_altstack();
 if unlikely(new_stack == ALTSTACK_ALLOC_FAILED)
    return NULL;
 CALL_WITH_STACK(result,DeeCode_ExecFrameSafe,frame,new_stack);
 free_altstack(new_stack);
 return result;
}
#endif /* !CONFIG_EXEC_ALTSTACK_USES_EXTERNAL_ASSEMBLY */

DECL_END
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */


DECL_BEGIN



INTERN int DCALL
trigger_breakpoint(struct code_frame *__restrict frame) {
 /* TODO: Add some sort of hook that allows for debugging. */
 (void)frame;
 return TRIGGER_BREAKPOINT_CONTINUE;
}






PUBLIC char *DCALL
DeeCode_GetASymbolName(DeeObject *__restrict self, uint16_t aid) {
 /* Argument */
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeCode_Type);
 if (((DeeCodeObject *)self)->co_keywords &&
     aid < ((DeeCodeObject *)self)->co_argc_max)
     return DeeString_STR(((DeeCodeObject *)self)->co_keywords[aid]);
 return NULL;
}

PUBLIC char *DCALL
DeeCode_GetSSymbolName(DeeObject *__restrict self, uint16_t sid) {
 /* Static symbol name */
 DeeDDIObject *ddi; uint8_t *reader;
 uint32_t offset;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeCode_Type);
 ddi = ((DeeCodeObject *)self)->co_ddi;
 if (ddi->d_exdat) {
  reader = (uint8_t *)ddi->d_exdat->dx_data;
  for (;;) {
   uint8_t op = *reader++;
   switch (op) {
   case DDI_EXDAT_O_END:
    goto done_exdat;
   case DDI_EXDAT_O_SNAM | DDI_EXDAT_OP8:
    if (*(uint8_t *)(reader + 0) == sid) {
     offset = *(uint8_t *)(reader + 1);
     goto return_strtab_offset;
    }
    reader += 1 + 1;
    break;
   case DDI_EXDAT_O_SNAM | DDI_EXDAT_OP16:
    if (UNALIGNED_GETLE16((uint16_t *)(reader + 0)) == sid) {
     offset = UNALIGNED_GETLE16((uint16_t *)(reader + 2));
     goto return_strtab_offset;
    }
    reader += 2 + 2;
    break;
   case DDI_EXDAT_O_SNAM | DDI_EXDAT_OP32:
    if (UNALIGNED_GETLE16((uint16_t *)(reader + 0)) == sid) {
     offset = UNALIGNED_GETLE32((uint32_t *)(reader + 2));
     goto return_strtab_offset;
    }
    reader += 2 + 4;
    break;
   default:
    switch (op & DDI_EXDAT_OPMASK) {
    case DDI_EXDAT_OP8:  reader += 1 + 1; break;
    case DDI_EXDAT_OP16: reader += 2 + 2; break;
    case DDI_EXDAT_OP32: reader += 2 + 4; break;
    default: break;
    }
    break;
   }
  }
 }
done_exdat:
 return NULL;
return_strtab_offset:
 return DeeString_STR(ddi->d_strtab) + offset;
}

PUBLIC char *DCALL
DeeCode_GetRSymbolName(DeeObject *__restrict self, uint16_t rid) {
 /* Reference symbol name */
 DeeDDIObject *ddi; uint8_t *reader;
 uint32_t offset;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeCode_Type);
 ddi = ((DeeCodeObject *)self)->co_ddi;
 if (ddi->d_exdat) {
  reader = (uint8_t *)ddi->d_exdat->dx_data;
  for (;;) {
   uint8_t op = *reader++;
   switch (op) {
   case DDI_EXDAT_O_END:
    goto done_exdat;
   case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP8:
    if (*(uint8_t *)(reader + 0) == rid) {
     offset = *(uint8_t *)(reader + 1);
     goto return_strtab_offset;
    }
    reader += 1 + 1;
    break;
   case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP16:
    if (UNALIGNED_GETLE16((uint16_t *)(reader + 0)) == rid) {
     offset = UNALIGNED_GETLE16((uint16_t *)(reader + 2));
     goto return_strtab_offset;
    }
    reader += 2 + 2;
    break;
   case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP32:
    if (UNALIGNED_GETLE16((uint16_t *)(reader + 0)) == rid) {
     offset = UNALIGNED_GETLE32((uint32_t *)(reader + 2));
     goto return_strtab_offset;
    }
    reader += 2 + 4;
    break;
   default:
    switch (op & DDI_EXDAT_OPMASK) {
    case DDI_EXDAT_OP8:  reader += 1 + 1; break;
    case DDI_EXDAT_OP16: reader += 2 + 2; break;
    case DDI_EXDAT_OP32: reader += 2 + 4; break;
    default: break;
    }
    break;
   }
  }
 }
done_exdat:
 return NULL;
return_strtab_offset:
 return DeeString_STR(ddi->d_strtab) + offset;
}

PUBLIC char *DCALL
DeeCode_GetDDIString(DeeObject *__restrict self, uint16_t id) {
 /* DDI String */
 DeeDDIObject *ddi;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeCode_Type);
 ddi = ((DeeCodeObject *)self)->co_ddi;
 if (id < ddi->d_nstring)
     return DeeString_STR(ddi->d_strtab) + ddi->d_strings[id];
 return NULL;
}








/* @return: 0: The code is not contained.
 * @return: 1: The code is contained.
 * @return: 2: The frame chain is incomplete, but code wasn't found thus far. */
PRIVATE int DCALL
frame_chain_contains_code(struct code_frame *__restrict iter, uint16_t count,
                          DeeCodeObject *__restrict code) {
 while (count--) {
  if (!iter || iter == CODE_FRAME_NOT_EXECUTING)
       return 2;
  if (iter->cf_func->fo_code == code)
      return 1;
 }
 return 0;
}

PUBLIC int DCALL
DeeCode_SetAssembly(/*Code*/DeeObject *__restrict self) {
 DeeCodeObject *me = (DeeCodeObject *)self;
 DeeThreadObject *caller;
 ASSERT_OBJECT_TYPE(self,&DeeCode_Type);
 /* Simple case: the assembly flag is already set. */
 if (me->co_flags & CODE_FASSEMBLY)
     return 0;
 caller = DeeThread_Self();
 /* Assume that the calling thread's execution chain
  * is consistent (which it _really_ should be). */
 if (frame_chain_contains_code(caller->t_exec,caller->t_execsz,me))
     goto already_executing;
 
#ifndef CONFIG_NO_THREADS
 /* Here comes the dangerous part: Checking the other threads... */
 {
  DeeThreadObject *threads;
check_other_threads:
  /* Check for interrupts. */
  if (DeeThread_CheckInterruptSelf(caller))
      goto err;
  COMPILER_BARRIER();
  threads = DeeThread_SuspendAll();
  DeeThread_FOREACH(threads) {
   int temp;
   if (threads == caller) continue; /* Skip the calling thread. */
   temp = frame_chain_contains_code(threads->t_exec,
                                    threads->t_execsz,
                                    me);
   if (!temp) continue; /* Unused. */
   COMPILER_READ_BARRIER();
   /* Resume execution of all the other threads. */
   DeeThread_ResumeAll();
   /* Unclear, or is being used. */
   if (temp == 2) {
    /* Unclear. - Wait for the thread to turn its stack consistent, then try again. */
    if (DeeThread_CheckInterruptSelf(caller))
        goto err;
    DeeThread_SleepNoInterrupt(100);
    goto check_other_threads;
   }
   /* The code object is currently being executed (or at least was being...) */
   goto already_executing;
  }
  /* Having confirmed that the code object isn't running, set the assembly
   * flag before resuming all the other threads so we can still ensure that
   * it will become visible as soon as the other threads start running again. */
  me->co_flags |= CODE_FASSEMBLY;
  COMPILER_WRITE_BARRIER(); /* Don't move the flag modification before this point. */
  DeeThread_ResumeAll();
 }
#else
 /* Simple case: Without any other threads to worry about, as well as the
  *              fact that the caller isn't using the code object, we can
  *              simply set the assembly flag and indicate success. */
 me->co_flags |= CODE_FASSEMBLY;
 COMPILER_WRITE_BARRIER();
#endif
 return 0;
already_executing:
 DeeError_Throwf(&DeeError_ValueError,
                 "Cannot set assembly mode for code object %k while it is being executed",
                 self);
err:
 return -1;
}


PRIVATE void DCALL
code_fini(DeeCodeObject *__restrict self) {
 uint16_t i;
 ASSERTF(!self->co_module ||
         !DeeModule_Check(self->co_module) ||
          self != self->co_module->mo_root ||
          self->co_module->ob_refcnt == 0,
         "Cannot destroy the root code object of a module");

 ASSERT(self->co_argc_max >= self->co_argc_min);
 /* Clear default argument objects. */
 if (self->co_argc_max != self->co_argc_min) {
  uint16_t count = self->co_argc_max - self->co_argc_min;
  for (i = 0; i < count; ++i)
      Dee_XDecref(self->co_defaultv[i]);
 }
 /* Clear static variables/constants. */
 for (i = 0; i < self->co_staticc; ++i)
     Dee_Decref(self->co_staticv[i]);

 /* Clear exception handlers. */
 for (i = 0; i < self->co_exceptc; ++i)
     Dee_XDecref(self->co_exceptv[i].eh_mask);

 /* Clear debug information. */
 Dee_Decref(self->co_ddi);

 /* Clear keyword names. */
 if (self->co_keywords) {
  for (i = 0; i < self->co_argc_max; ++i)
      Dee_Decref(self->co_keywords[i]);
  Dee_Free((void *)self->co_keywords);
 }

 /* Clear module information. */
 Dee_XDecref(self->co_module);

 /* Free vectors. */
 Dee_Free((void *)self->co_defaultv);
 Dee_Free((void *)self->co_staticv);
 Dee_Free((void *)self->co_exceptv);
}

PRIVATE void DCALL
code_visit(DeeCodeObject *__restrict self,
           dvisit_t proc, void *arg) {
 size_t i;
 /* Visit the accompanying module.
  * NOTE: We must use `Dee_XVisit()' here because the pointer
  *       may still be NULL when it still represents the next
  *       element in the chain of code objects associated with
  *       the module currently being compiled. */
 Dee_XVisit(self->co_module);

 /* Visit default variables. */
 for (i = 0; i < (uint16_t)(self->co_argc_max - self->co_argc_min); ++i)
     Dee_XVisit(self->co_defaultv[i]);

 /* Visit static variables. */
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->co_static_lock);
#endif
 for (i = 0; i < self->co_staticc; ++i)
     Dee_Visit(self->co_staticv[i]);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->co_static_lock);
#endif

 /* Visit exception information. */
 for (i = 0; i < self->co_exceptc; ++i)
     Dee_XVisit(self->co_exceptv[i].eh_mask);

 /* Visit debug information. */
 Dee_Visit(self->co_ddi);
}

PRIVATE void DCALL
code_clear(DeeCodeObject *__restrict self) {
 DREF DeeObject *buffer[16],**iter = buffer;
 size_t i; /* Clear out static variables. */
restart:
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->co_static_lock);
#endif
 for (i = 0; i < self->co_staticc; ++i) {
  DREF DeeObject *ob = self->co_staticv[i];
  if (DeeNone_Check(ob) || DeeString_Check(ob)) continue;
  if (DeeCode_Check(ob)) {
   /* Assembly may rely on certain constants being code objects!
    * XXX: Speaking of which, what if this code is being executed right now?
    *      After all: accessing constants normally doesn't require locks,
    *      but us meddling with _all_ of them (and not just statics) breaks
    *      some of the assumptions such code makes... */
   self->co_staticv[i] = (DeeObject *)&empty_code;
  } else {
   self->co_staticv[i] = Dee_None;
  }
  Dee_Incref(self->co_staticv[i]);
  if (!Dee_DecrefIfNotOne(ob)) {
   *iter++ = ob;
   if (iter == COMPILER_ENDOF(buffer)) {
#ifndef CONFIG_NO_THREADS
    rwlock_endwrite(&self->co_static_lock);
#endif
    while (iter != buffer) { --iter; Dee_Decref(*iter); }
    goto restart;
   }
  }
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->co_static_lock);
#endif
 while (iter != buffer) { --iter; Dee_Decref(*iter); }
}

PRIVATE DREF DeeObject *DCALL
code_get_kwds(DeeCodeObject *__restrict self) {
 ASSERT(self->co_argc_max >= self->co_argc_min);
 if (!self->co_keywords)
      return_empty_seq;
 return DeeRefVector_NewReadonly((DeeObject *)self,
                                 (size_t)self->co_argc_max,
                                 (DeeObject *const *)self->co_keywords);
}

PRIVATE DREF DeeObject *DCALL
code_getdefault(DeeCodeObject *__restrict self) {
 ASSERT(self->co_argc_max >= self->co_argc_min);
 return DeeRefVector_NewReadonly((DeeObject *)self,
                                 (size_t)(self->co_argc_max-self->co_argc_min),
                                 self->co_defaultv);
}

PRIVATE DREF DeeObject *DCALL
code_getstatic(DeeCodeObject *__restrict self) {
 ASSERT(self->co_argc_max >= self->co_argc_min);
 return DeeRefVector_NewReadonly((DeeObject *)self,
                                  self->co_staticc,
                                  self->co_staticv);
}

PRIVATE DREF DeeObject *DCALL
code_isyielding(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FYIELDING);
}
PRIVATE DREF DeeObject *DCALL
code_iscopyable(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FCOPYABLE);
}
PRIVATE DREF DeeObject *DCALL
code_hasassembly(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FASSEMBLY);
}
PRIVATE DREF DeeObject *DCALL
code_islenient(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FLENIENT);
}
PRIVATE DREF DeeObject *DCALL
code_hasvarargs(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FVARARGS);
}
PRIVATE DREF DeeObject *DCALL
code_isthiscall(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FTHISCALL);
}
PRIVATE DREF DeeObject *DCALL
code_hasheapframe(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FHEAPFRAME);
}
PRIVATE DREF DeeObject *DCALL
code_hasfinally(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FFINALLY);
}
PRIVATE DREF DeeObject *DCALL
code_isconstructor(DeeCodeObject *__restrict self) {
 return_bool_(self->co_flags & CODE_FCONSTRUCTOR);
}

PRIVATE DREF DeeObject *DCALL
code_get_name(DeeCodeObject *__restrict self) {
 struct function_info info;
 if (DeeCode_GetInfo((DeeObject *)self,&info) < 0)
     goto err;
 Dee_XDecref(info.fi_type);
 Dee_XDecref(info.fi_doc);
 if (!info.fi_name) return_none;
 return (DREF DeeObject *)info.fi_name;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
code_get_doc(DeeCodeObject *__restrict self) {
 struct function_info info;
 if (DeeCode_GetInfo((DeeObject *)self,&info) < 0)
     goto err;
 Dee_XDecref(info.fi_type);
 Dee_XDecref(info.fi_name);
 if (!info.fi_doc) return_none;
 return (DREF DeeObject *)info.fi_doc;
err:
 return NULL;
}

PRIVATE DREF DeeTypeObject *DCALL
code_get_type(DeeCodeObject *__restrict self) {
 struct function_info info;
 if (DeeCode_GetInfo((DeeObject *)self,&info) < 0)
     goto err;
 Dee_XDecref(info.fi_name);
 Dee_XDecref(info.fi_doc);
 if (!info.fi_type) {
  info.fi_type = (DREF DeeTypeObject *)Dee_None;
  Dee_Incref(Dee_None);
 }
 return info.fi_type;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
code_get_operator(DeeCodeObject *__restrict self) {
 struct function_info info;
 if (DeeCode_GetInfo((DeeObject *)self,&info) < 0)
     goto err;
 Dee_XDecref(info.fi_type);
 Dee_XDecref(info.fi_name);
 Dee_XDecref(info.fi_doc);
 if (info.fi_opname == (uint16_t)-1) return_none;
 return DeeInt_NewU16(info.fi_opname);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
code_get_operatorname(DeeCodeObject *__restrict self) {
 struct function_info info;
 struct opinfo *op;
 if (DeeCode_GetInfo((DeeObject *)self,&info) < 0)
     goto err;
 Dee_XDecref(info.fi_name);
 Dee_XDecref(info.fi_doc);
 if (info.fi_opname == (uint16_t)-1) {
  Dee_XDecref(info.fi_type);
  return_none;
 }
 op = Dee_OperatorInfo(info.fi_type,info.fi_opname);
 Dee_XDecref(info.fi_type);
 if (!op) return DeeInt_NewU16(info.fi_opname);
 return DeeString_New(op->oi_sname);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
code_get_property(DeeCodeObject *__restrict self) {
 struct function_info info;
 if (DeeCode_GetInfo((DeeObject *)self,&info) < 0)
     goto err;
 Dee_XDecref(info.fi_name);
 Dee_XDecref(info.fi_doc);
 Dee_XDecref(info.fi_type);
 if (info.fi_getset == (uint16_t)-1) return_none;
 return DeeInt_NewU16(info.fi_getset);
err:
 return NULL;
}

PRIVATE struct type_member code_members[] = {
    TYPE_MEMBER_FIELD_DOC("__ddi__",STRUCT_OBJECT,offsetof(DeeCodeObject,co_ddi),
                          "->?Ert:Ddi\n"
                          "The DDI (DeemonDebugInformation) data block"),
    TYPE_MEMBER_FIELD_DOC("__module__",STRUCT_OBJECT,offsetof(DeeCodeObject,co_module),
                          "->?Dmodule"),
    TYPE_MEMBER_FIELD_DOC("__argc_min__",STRUCT_CONST|STRUCT_UINT16_T,offsetof(DeeCodeObject,co_argc_min),
                          "Min amount of arguments required to execute this code"),
    TYPE_MEMBER_FIELD_DOC("__argc_max__",STRUCT_CONST|STRUCT_UINT16_T,offsetof(DeeCodeObject,co_argc_max),
                          "Max amount of arguments accepted by this code (excluding a varargs argument)"),
    TYPE_MEMBER_END
};

PRIVATE struct type_getset code_getsets[] = {
    { DeeString_STR(&str___name__),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_get_name, NULL, NULL,
      DOC("->?X2?Dstring?N\n"
          "Returns the name of @this code object, or :none if unknown (s.a. :function.__name__)") },
    { DeeString_STR(&str___doc__),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_get_doc, NULL, NULL,
      DOC("->?X2?Dstring?N\n"
          "Returns the documentation string of @this code object, or :none if unknown (s.a. :function.__doc__)") },
    { DeeString_STR(&str___type__),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_get_type, NULL, NULL,
      DOC("->?X2?Dtype?N\n"
          "Try to determine if @this code object is defined as part of a user-defined class, "
          "and if it is, return that class type, or :none if that class couldn't be found, "
          "of if @this code object is defined as stand-alone (s.a. :function.__type__)") },
    { DeeString_STR(&str___kwds__),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_get_kwds, NULL, NULL,
      DOC("->?S?Dstring\n"
          "Returns a sequence of keyword argument names accepted by @this code object\n"
          "If @this code doesn't accept keyword arguments, an empty sequence is returned") },
    { "__operator__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_get_operator, NULL, NULL,
      DOC("->?X2?Dint?N\n"
          "Try to determine if @this code object is defined as part of a user-defined class, "
          "and if so, if it is used to define an operator callback. If that is the case, "
          "return the internal ID of the operator that @this code object provides, or :none "
          "if that class couldn't be found, @this code object is defined as stand-alone, or "
          "defined as a class- or instance-method (s.a. :function.__operator__)") },
    { "__operatorname__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_get_operatorname, NULL, NULL,
      DOC("->?X3?Dstring?Dint?N\n"
          "Same as #__operator__, but instead try to return the unambiguous name of the "
          "operator, though still return its ID if the operator isn't recognized as being "
          "part of the standard (s.a. :function.__operatorname__)") },
    { "__property__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_get_property, NULL, NULL,
      DOC("->?X2?Dint?N\n"
          "Returns an integer describing the kind if @this code is part of a property or getset, "
          "or returns :none if the function's property could not be found, or if the function isn't "
          "declared as a property callback (s.a. :function.__property__)") },
    { "__default__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_getdefault, NULL, NULL,
      DOC("->?S?O\n"
          "Access to the default values of arguments") },
    { "__static__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_getstatic, NULL, NULL,
      DOC("->?S?O\n"
          "Access to the static values of @this code object") },
    { "__isyielding__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_isyielding, NULL, NULL,
      DOC("->?Dbool\n"
          "Check if the code object is for a yield-function") },
    { "__iscopyable__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_iscopyable, NULL, NULL,
      DOC("->?Dbool\n"
          "Check if execution frames of the code object can be copied") },
    { "__hasassembly__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasassembly, NULL, NULL,
      DOC("->?Dbool\n"
          "Check if assembly of the code object is executed in safe-mode") },
    { "__islenient__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_islenient, NULL, NULL,
      DOC("->?Dbool\n"
          "Check if the runtime stack allocation allows for leniency") },
    { "__hasvarargs__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasvarargs, NULL, NULL,
      DOC("->?Dbool\n"
          "Check if the code object takes its last argument as a varargs-tuple") },
    { "__isthiscall__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_isthiscall, NULL, NULL,
      DOC("->?Dbool\n"
          "Check if the code object requires a hidden leading this-argument") },
    { "__hasheapframe__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasheapframe, NULL, NULL,
      DOC("->?Dbool\n"
          "Check if the runtime stack-frame must be allocated on the heap") },
    { "__hasfinally__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasfinally, NULL, NULL,
      DOC("->?Dbool\n"
          "True if execution will jump to the nearest finally-block when a return instruction is encountered\n"
          "Note that this does not necessarily guaranty, or deny the presence of a try...finally statement in "
          "the user's source code, as the compiler may try to optimize this flag away to speed up runtime execution") },
    { "__isconstructor__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_isconstructor, NULL, NULL,
      DOC("->?Dbool\n"
          "True for class constructor code objects. - When set, don't include the this-argument in "
          "tracebacks, thus preventing incomplete instances from being leaked when the constructor "
          "causes some sort of exception to be thrown") },
    { NULL }
};

PRIVATE struct type_gc code_gc = {
    /* .tp_clear = */(void(DCALL *)(DeeObject *__restrict))&code_clear
};

PRIVATE dhash_t DCALL
code_hash(DeeCodeObject *__restrict self) {
 dhash_t result;
 result  = self->co_flags;
 result ^= self->co_localc;
 result ^= self->co_staticc;
 result ^= self->co_refc;
 result ^= self->co_exceptc;
 result ^= self->co_argc_min;
 result ^= self->co_argc_max;
 result ^= self->co_framesize;
 result ^= self->co_codebytes;
 if (self->co_module)
     result ^= DeeObject_Hash((DeeObject *)self->co_module);
 if (self->co_keywords) {
  uint16_t i;
  for (i = 0; i < self->co_argc_max; ++i)
      result ^= DeeString_Hash((DeeObject *)self->co_keywords[i]);
 }
 if (self->co_defaultv) {
  uint16_t i;
  for (i = 0; i < (self->co_argc_max - self->co_argc_min); ++i) {
   if ((DeeObject *)self->co_defaultv[i])
       result ^= DeeObject_Hash((DeeObject *)self->co_defaultv[i]);
  }
 }
 if (self->co_staticv) {
  uint16_t i;
  for (i = 0; i < self->co_staticc; ++i) {
   DREF DeeObject *ob;
   rwlock_read(&self->co_static_lock);
   ob = self->co_staticv[i];
   Dee_Incref(ob);
   rwlock_endread(&self->co_static_lock);
   result ^= DeeObject_Hash(ob);
   Dee_Decref(ob);
  }
 }
 if (self->co_exceptv) {
  uint16_t i;
  for (i = 0; i < self->co_exceptc; ++i) {
   result ^= self->co_exceptv[i].eh_start;
   result ^= self->co_exceptv[i].eh_end;
   result ^= self->co_exceptv[i].eh_addr;
   result ^= self->co_exceptv[i].eh_stack;
   result ^= self->co_exceptv[i].eh_flags;
   if (self->co_exceptv[i].eh_mask)
       result ^= DeeObject_Hash((DeeObject *)self->co_exceptv[i].eh_mask);
  }
 }
 result ^= DeeObject_Hash((DeeObject *)self->co_ddi);
 result ^= Dee_HashPtr(self->co_code,self->co_codebytes);
 return result;
}

PRIVATE int DCALL
code_eq_impl(DeeCodeObject *__restrict self,
             DeeCodeObject *__restrict other) {
 int temp;
 if (DeeObject_AssertTypeExact(other,&DeeCode_Type))
     return -1;
 if (self == other) return 1;
 if (self->co_flags != other->co_flags) goto nope;
 if (self->co_localc != other->co_localc) goto nope;
 if (self->co_staticc != other->co_staticc) goto nope;
 if (self->co_refc != other->co_refc) goto nope;
 if (self->co_exceptc != other->co_exceptc) goto nope;
 if (self->co_argc_min != other->co_argc_min) goto nope;
 if (self->co_argc_max != other->co_argc_max) goto nope;
 if (self->co_framesize != other->co_framesize) goto nope;
 if (self->co_codebytes != other->co_codebytes) goto nope;
 if (self->co_module != other->co_module) goto nope;
 if (self->co_keywords) {
  uint16_t i;
  if (!other->co_keywords) goto nope;
  for (i = 0; i < self->co_argc_max; ++i) {
   if (DeeString_SIZE(self->co_keywords[i]) !=
       DeeString_SIZE(other->co_keywords[i]))
       goto nope;
   if (memcmp(DeeString_STR(self->co_keywords[i]),
              DeeString_STR(other->co_keywords[i]),
              DeeString_SIZE(self->co_keywords[i]) * sizeof(char)) != 0)
       goto nope;
  }
 } else if (other->co_keywords) {
  goto nope;
 }
 if (self->co_defaultv) {
  uint16_t i;
  for (i = 0; i < (self->co_argc_max - self->co_argc_min); ++i) {
   if (self->co_defaultv[i] == NULL) {
    if (other->co_defaultv[i] != NULL)
        goto nope;
   } else {
    if (other->co_defaultv[i] == NULL)
        goto nope;
    temp = DeeObject_CompareEq(self->co_defaultv[i],
                               other->co_defaultv[i]);
    if (temp <= 0) goto err_temp;
   }
  }
 }
 if (self->co_staticv) {
  uint16_t i;
  for (i = 0; i < self->co_staticc; ++i) {
   DREF DeeObject *lhs,*rhs;
   rwlock_read(&self->co_static_lock);
   lhs = self->co_staticv[i];
   Dee_Incref(lhs);
   rwlock_endread(&self->co_static_lock);
   rwlock_read(&other->co_static_lock);
   rhs = other->co_staticv[i];
   Dee_Incref(rhs);
   rwlock_endread(&other->co_static_lock);
   temp = DeeObject_CompareEq(lhs,rhs);
   Dee_Decref(rhs);
   Dee_Decref(lhs);
   if (temp <= 0) goto err_temp;
  }
 }
 if (self->co_exceptv) {
  uint16_t i;
  for (i = 0; i < self->co_exceptc; ++i) {
   if (self->co_exceptv[i].eh_start != other->co_exceptv[i].eh_start) goto nope;
   if (self->co_exceptv[i].eh_end   != other->co_exceptv[i].eh_end) goto nope;
   if (self->co_exceptv[i].eh_addr  != other->co_exceptv[i].eh_addr) goto nope;
   if (self->co_exceptv[i].eh_stack != other->co_exceptv[i].eh_stack) goto nope;
   if (self->co_exceptv[i].eh_flags != other->co_exceptv[i].eh_flags) goto nope;
   if (self->co_exceptv[i].eh_mask  != other->co_exceptv[i].eh_mask) goto nope;
  }
 }
 temp = DeeObject_CompareEq((DeeObject *)self->co_ddi,
                            (DeeObject *)other->co_ddi);
 if (temp <= 0) goto err_temp;
 if (memcmp(self->co_code,other->co_code,self->co_codebytes) != 0)
     goto nope;
 return 1;
err_temp:
 return temp;
nope:
 return 0;
}

PRIVATE DREF DeeObject *DCALL
code_eq(DeeCodeObject *__restrict self,
        DeeCodeObject *__restrict other) {
 int result;
 result = code_eq_impl(self,other);
 if unlikely(result < 0) return NULL;
 return_bool_(result != 0);
}

PRIVATE DREF DeeObject *DCALL
code_ne(DeeCodeObject *__restrict self,
        DeeCodeObject *__restrict other) {
 int result;
 result = code_eq_impl(self,other);
 if unlikely(result < 0) return NULL;
 return_bool_(result == 0);
}

PRIVATE struct type_cmp code_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&code_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&code_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&code_ne
};

PRIVATE DREF DeeObject *DCALL
code_str(DeeCodeObject *__restrict self) {
 DREF DeeObject *result;
 DREF DeeObject *name = code_get_name(self);
 if unlikely(!name) goto err;
 if (DeeNone_Check(name))
  result = DeeString_New("<code for <anonymous>>");
 else {
  result = DeeString_Newf("<code for %r>",name);
 }
 Dee_Decref(name);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL code_ctor(void) {
 return_reference_((DeeObject *)&empty_code);
}

PRIVATE DREF DeeCodeObject *DCALL
code_copy(DeeCodeObject *__restrict self) {
 DREF DeeCodeObject *result; uint16_t i;
 result = (DREF DeeCodeObject *)DeeGCObject_Malloc(offsetof(DeeCodeObject,co_code) +
                                                   self->co_codebytes);
 if unlikely(!result) goto done;
 memcpy(result,self,offsetof(DeeCodeObject,co_code) + self->co_codebytes);
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->co_static_lock);
#endif
 if (result->co_keywords) {
  if (!result->co_argc_max)
       result->co_keywords = NULL;
  else {
   result->co_keywords = (DREF DeeStringObject **)Dee_Malloc(result->co_argc_max *
                                                             sizeof(DREF DeeStringObject *));
   if unlikely(!result->co_keywords)
      goto err_r;
   memcpy((void *)result->co_keywords,self->co_keywords,
           result->co_argc_max * sizeof(DREF DeeStringObject *));
   for (i = 0; i < result->co_argc_max; ++i)
       Dee_Incref(result->co_keywords[i]);
  }
 }
 ASSERT(result->co_argc_max >= result->co_argc_min);
 ASSERT((result->co_defaultv == NULL) ==
        (result->co_argc_max == result->co_argc_min));
 if (result->co_defaultv) {
  uint16_t n = result->co_argc_max - result->co_argc_min;
  result->co_defaultv = (DREF DeeObject **)Dee_Malloc(n * sizeof(DREF DeeObject *));
  if unlikely(!result->co_defaultv)
     goto err_r_keywords;
  memcpy((void *)result->co_defaultv,self->co_defaultv,
          n * sizeof(DREF DeeObject *));
  for (i = 0; i < n; ++i)
      Dee_XIncref(result->co_defaultv[i]);
 }
 ASSERT((result->co_staticc != 0) ==
        (result->co_staticv != NULL));
 if (result->co_staticv) {
  result->co_staticv = (DREF DeeObject **)Dee_Malloc(result->co_staticc *
                                                     sizeof(DREF DeeObject *));
  if unlikely(!result->co_staticv)
     goto err_r_default;
  rwlock_read(&self->co_static_lock);
  memcpy((void *)result->co_staticv,self->co_staticv,
          result->co_staticc * sizeof(DREF DeeObject *));
  for (i = 0; i < result->co_staticc; ++i)
      Dee_Incref(result->co_staticv[i]);
  rwlock_endread(&self->co_static_lock);
 }
 if (!result->co_exceptc)
      result->co_exceptv = NULL;
 else {
  result->co_exceptv = (struct except_handler *)Dee_Malloc(result->co_exceptc *
                                                           sizeof(struct except_handler));
  if unlikely(!result->co_exceptv)
     goto err_r_static;
  memcpy(result->co_exceptv,self->co_exceptv,
         result->co_exceptc *
         sizeof(struct except_handler));
  for (i = 0; i < result->co_exceptc; ++i)
      Dee_XIncref(result->co_exceptv[i].eh_mask);
 }

 Dee_XIncref(result->co_ddi);
 Dee_XIncref(result->co_module);
 DeeObject_Init(result,&DeeCode_Type);
 DeeGC_Track((DeeObject *)result);
done:
 return result;
err_r_static:
 if (result->co_staticv) {
  for (i = 0; i < result->co_staticc; ++i)
      Dee_Decref(result->co_staticv[i]);
  Dee_Free(result->co_staticv);
 }
err_r_default:
 if (result->co_defaultv) {
  uint16_t n = result->co_argc_max - result->co_argc_min;
  for (i = 0; i < n; ++i)
      Dee_XDecref(result->co_defaultv[i]);
  Dee_Free((void *)result->co_defaultv);
 }
err_r_keywords:
 if (result->co_keywords) {
  for (i = 0; i < result->co_argc_max; ++i)
      Dee_Decref(result->co_keywords[i]);
  Dee_Free((void *)result->co_keywords);
 }
err_r:
 DeeGCObject_Free(result);
 return NULL;
}

PRIVATE DREF DeeCodeObject *DCALL
code_deepcopy(DeeCodeObject *__restrict self) {
 DREF DeeCodeObject *result; uint16_t i;
 result = code_copy(self);
 if unlikely(!result) goto done;
 if (result->co_defaultv) {
  uint16_t n = result->co_argc_max - result->co_argc_min;
  for (i = 0; i < n; ++i) {
   if (DeeObject_XInplaceDeepCopy((DeeObject **)&result->co_defaultv[i]))
       goto err_r;
  }
 }
 if (result->co_staticv) {
  for (i = 0; i < result->co_staticc; ++i) {
   if (DeeObject_InplaceDeepCopyWithLock((DeeObject **)&result->co_staticv[i],&result->co_static_lock))
       goto err_r;
  }
 }
#if 0 /* Default information wouldn't change... */
 if (DeeObject_InplaceDeepCopy((DeeObject **)&result->co_ddi))
     goto err_r;
#endif
#if 0 /* Types are singletons. */
 if (result->co_exceptv) {
  for (i = 0; i < result->co_exceptc; ++i) {
   if (DeeObject_XInplaceDeepCopy((DeeObject **)&result->co_exceptv[i].eh_mask))
       goto err_r;
  }
 }
#endif
#if 0 /* Modules are singletons! - If they weren't we'd be corrupting their filesystem namespace... */
 if (DeeObject_XInplaceDeepCopy((DeeObject **)&result->co_module))
     goto err_r;
#endif
done:
 return result;
err_r:
 Dee_Decref(result);
 return NULL;
}


PUBLIC DeeTypeObject DeeCode_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_code),
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FVARIABLE|TP_FFINAL|TP_FGC|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */&code_ctor,
                /* .tp_copy_ctor = */&code_copy,
                /* .tp_deep_ctor = */&code_deepcopy,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&code_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_str,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&code_visit,
    /* .tp_gc            = */&code_gc,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&code_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */code_getsets,
    /* .tp_members       = */code_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


INTERN struct empty_code_struct empty_code_head = {
    {
        /* ... */
        NULL,
        NULL
    },{
        OBJECT_HEAD_INIT(&DeeCode_Type),
        /* .co_flags       = */CODE_FCOPYABLE,
        /* .co_localc      = */0,
        /* .co_staticc     = */0,
        /* .co_refc        = */0,
        /* .co_exceptc     = */0,
        /* .co_argc_min    = */0,
        /* .co_argc_max    = */0,
        /* .co_padding     = */0,
        /* .co_framesize   = */0,
        /* .co_codebytes   = */sizeof(instruction_t),
#ifndef CONFIG_NO_THREADS
        /* .co_static_lock = */RWLOCK_INIT,
#endif
        /* .co_module      = */{ &empty_module },
        /* .co_keywords    = */NULL,
        /* .co_defaultv    = */NULL,
        /* .co_staticv     = */NULL,
        /* .co_exceptv     = */NULL,
        /* .co_ddi         = */&empty_ddi,
        /* .co_code        = */{ ASM_RET_NONE },
    }
};


DECL_END

/* Pull in the actual code execution runtime. */
#ifndef __INTELLISENSE__
#define EXEC_SAFE 1
#include "code-exec.c.inl"
#undef EXEC_SAFE
#ifndef CONFIG_HAVE_EXEC_ASM
#define EXEC_FAST 1
#include "code-exec.c.inl"
#undef EXEC_FAST
#endif /* !CONFIG_HAVE_EXEC_ASM */
#endif

#ifndef __INTELLISENSE__
#undef CALL_THIS
#undef CALL_TUPLE
#undef CALL_KW

#include "code-invoke.c.inl"
#define CALL_KW 1
#include "code-invoke.c.inl"
#define CALL_THIS 1
#include "code-invoke.c.inl"
#define CALL_THIS 1
#define CALL_KW 1
#include "code-invoke.c.inl"

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
#define CALL_TUPLE 1
#include "code-invoke.c.inl"
#define CALL_TUPLE 1
#define CALL_KW 1
#include "code-invoke.c.inl"
#define CALL_TUPLE 1
#define CALL_THIS 1
#include "code-invoke.c.inl"
#define CALL_TUPLE 1
#define CALL_THIS 1
#define CALL_KW 1
#include "code-invoke.c.inl"
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
#endif

#endif /* !GUARD_DEEMON_EXECUTE_CODE_C */
