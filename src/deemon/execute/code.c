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
#include <deemon/asm.h>
#include <deemon/util/string.h>

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

INTERN ATTR_NOINLINE DREF DeeObject *DCALL
object_call_vec2(DeeObject *__restrict func,
                 size_t argc1, DeeObject **__restrict vec1,
                 size_t argc2, DeeObject **__restrict vec2) {
 DREF DeeObject *result; DeeObject **argv;
 size_t argc = argc1 + argc2;
 ASSERT(argc1 != 0);
 ASSERT(argc2 != 0);
#ifdef Dee_Alloca
 if (argc < DEE_AMALLOC_MAX/sizeof(DREF DeeObject *)) {
  argv = (DREF DeeObject **)Dee_Alloca(argc*sizeof(DREF DeeObject *));
  if unlikely(!argv) return NULL;
  MEMCPY_PTR(argv,vec1,argc1);
  MEMCPY_PTR(argv+argc1,vec2,argc2);
  result = DeeObject_Call(func,argc,argv);
 } else
#endif
 {
  argv = (DREF DeeObject **)Dee_Malloc(argc*sizeof(DREF DeeObject *));
  if unlikely(!argv) return NULL;
  MEMCPY_PTR(argv,vec1,argc1);
  MEMCPY_PTR(argv+argc1,vec2,argc2);
  result = DeeObject_Call(func,argc,argv);
  Dee_Free(argv);
 }
 return result;
}



INTERN int DCALL
trigger_breakpoint(struct code_frame *__restrict frame) {
 /* TODO: Add some sort of hook that allows for debugging. */
 (void)frame;
 return TRIGGER_BREAKPOINT_CONTINUE;
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
 if (me->co_flags&CODE_FASSEMBLY)
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
 DeeObject *const *iter,*const *end;
 ASSERTF(!self->co_module ||
         !DeeModule_Check(self->co_module) ||
          self != self->co_module->mo_root ||
          self->co_module->ob_refcnt == 0,
         "Cannot destroy the root code object of a module");

 ASSERT(self->co_argc_max >= self->co_argc_min);
 /* Clear default argument objects. */
 if (self->co_argc_max != self->co_argc_min) {
  iter = self->co_defaultv;
  end  = iter+(self->co_argc_max-self->co_argc_min);
  for (; iter != end; ++iter) Dee_Decref(*iter);
 }
 /* Clear static variables/constants. */
 end = (iter = self->co_staticv)+self->co_staticc;
 for (; iter != end; ++iter) Dee_Decref(*iter);

 /* Clear exception handlers. */
 { struct except_handler *xiter,*xend;
   xend = (xiter = self->co_exceptv)+self->co_exceptc;
   for (; xiter != xend; ++xiter) Dee_XDecref(xiter->eh_mask);
 }
 /* Clear debug information. */
 Dee_Decref(self->co_ddi);

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
 DeeObject *const *iter,*const *end;
 struct except_handler *xiter,*xend;
 /* Visit the accompanying module.
  * NOTE: We must use `Dee_XVisit()' here because the pointer
  *       may still be NULL when it still represents the next
  *       element in the chain of code objects associted with
  *       the module currently being compiled. */
 Dee_XVisit(self->co_module);

 /* Visit default variables. */
 end = (iter = self->co_defaultv)+(self->co_argc_max-
                                   self->co_argc_min);
 for (; iter != end; ++iter) Dee_Visit(*iter);

 /* Visit static variables. */
 end = (iter = self->co_staticv)+self->co_staticc;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->co_static_lock);
#endif
 for (; iter != end; ++iter)
     Dee_Visit(*iter);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->co_static_lock);
#endif

 /* Visit exception information. */
 xend = (xiter = self->co_exceptv)+self->co_exceptc;
 for (; xiter != xend; ++xiter) Dee_XVisit(xiter->eh_mask);

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

PRIVATE struct type_member code_members[] = {
    TYPE_MEMBER_FIELD_DOC("ddi",STRUCT_OBJECT,offsetof(DeeCodeObject,co_ddi),"->ddi\nThe DDI (DeemonDebugInformation) data block"),
    TYPE_MEMBER_FIELD_DOC("__module__",STRUCT_OBJECT,offsetof(DeeCodeObject,co_module),"->module"),
    TYPE_MEMBER_FIELD_DOC("__argc_min__",STRUCT_CONST|STRUCT_UINT16_T,offsetof(DeeCodeObject,co_argc_min),"Min amount of arguments required to execute this code"),
    TYPE_MEMBER_FIELD_DOC("__argc_max__",STRUCT_CONST|STRUCT_UINT16_T,offsetof(DeeCodeObject,co_argc_max),"Max amount of arguments accepted by this code (excluding a varargs argument)"),
    TYPE_MEMBER_END
};

PRIVATE struct type_getset code_getsets[] = {
    { "__default__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_getdefault, NULL, NULL,
      DOC("->sequence\nAccess to the default values of arguments") },
    { "__static__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_getstatic, NULL, NULL,
      DOC("->sequence\nAccess to the static values of @this code object") },
    { "__isyielding__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_isyielding, NULL, NULL,
      DOC("->bool\nCheck if the code object is for a yield-function") },
    { "__iscopyable__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_iscopyable, NULL, NULL,
      DOC("->bool\nCheck if execution frames of the code object can be copied") },
    { "__hasassembly__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasassembly, NULL, NULL,
      DOC("->bool\nCheck if assembly of the code object is executed in safe-mode") },
    { "__islenient__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_islenient, NULL, NULL,
      DOC("->bool\nCheck if the runtime stack allocation allows for leniency") },
    { "__hasvarargs__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasvarargs, NULL, NULL,
      DOC("->bool\nCheck if the code object takes its last argument as a varargs-tuple") },
    { "__isthiscall__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_isthiscall, NULL, NULL,
      DOC("->bool\nCheck if the code object requires a hidden leading this-argument") },
    { "__hasheapframe__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasheapframe, NULL, NULL,
      DOC("->bool\nCheck if the runtime stack-frame must be allocated on the heap") },
    { "__hasfinally__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_hasfinally, NULL, NULL,
      DOC("->bool\n"
          "True if execution will jump to the nearest finally-block when a return instruction is encountered\n"
          "Note that this does not necessarily guaranty, or deny the presence of a try...finally statement in "
          "the user's source code, as the compiler may try to optimize this flag away to speed up runtime execution") },
    { "__isconstructor__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&code_isconstructor, NULL, NULL,
      DOC("->bool\n"
          "True for class constructor code objects. - When set, don't include the this-argument in "
          "tracebacks, thus preventing incomplete instances from being leaked when the constructor "
          "causes some sort of exception to be thrown") },
    { NULL }
};

PRIVATE struct type_member code_class_members[] = {
    TYPE_MEMBER_CONST("ddi",&DeeDDI_Type),
    TYPE_MEMBER_END
};


PRIVATE struct type_gc code_gc = {
    /* .tp_clear = */(void(DCALL *)(DeeObject *__restrict))&code_clear
};

PRIVATE DREF DeeObject *DCALL code_ctor(void) {
 return_reference_((DeeObject *)&empty_code);
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
                /* .tp_copy_ctor = */&DeeObject_NewRef,
                /* .tp_deep_ctor = */&DeeObject_NewRef,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&code_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&code_visit,
    /* .tp_gc            = */&code_gc,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
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
    /* .tp_class_members = */code_class_members
};


INTERN struct empty_code_struct empty_code_head = {
    {
        /* ... */
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

DECL_BEGIN

/* Invoke a user function object. */
INTERN DREF DeeObject *DCALL
function_call(DeeFunctionObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeCodeObject *code;
 ASSERT_OBJECT(self);
 ASSERT(DeeFunction_Check(self));
 code = self->fo_code;
 if unlikely(code->co_flags&CODE_FTHISCALL) {
  /* Special case: Invoke the function as a this-call. */
  if unlikely(!argc) {
   char *name = DeeDDI_NAME(code->co_ddi);
   err_invalid_argc(*name ? name : NULL,0,code->co_argc_min+1,
                     code->co_flags&CODE_FVARARGS ?
                   (size_t)-1 : ((size_t)code->co_argc_max+1));
   return NULL;
  }
  return DeeFunction_ThisCall(self,argv[0],argc-1,argv+1);
 }

 if unlikely(argc < code->co_argc_min ||
            (argc > code->co_argc_max &&
           !(code->co_flags&CODE_FVARARGS))) {
  /* ERROR: Invalid argument count! */
  char *name = DeeDDI_NAME(code->co_ddi);
  err_invalid_argc(*name ? name : NULL,argc,code->co_argc_min,
                    code->co_flags&CODE_FVARARGS ?
                  (size_t)-1 : ((size_t)code->co_argc_max));
  return NULL;
 }

 if (!(code->co_flags&CODE_FYIELDING)) {
  /* Default scenario: Perform a regular function call. */
  DeeObject *result;
  struct code_frame frame;
  frame.cf_func   = self;
  frame.cf_argc   = argc;
  frame.cf_argv   = argv;
  frame.cf_kw     = Dee_EmptyMapping;
  frame.cf_result = NULL;
#ifdef Dee_Alloca
  if (!(code->co_flags&CODE_FHEAPFRAME)) {
   frame.cf_frame = (DeeObject **)Dee_Alloca(code->co_framesize);
  } else
#endif /* Dee_Alloca */
  {
   frame.cf_frame = (DeeObject **)Dee_Malloc(code->co_framesize);
   if unlikely(!frame.cf_frame) return NULL;
  }
  /* Per-initialize local variable memory to ZERO. */
  MEMSET_PTR(frame.cf_frame,0,code->co_localc);
#ifndef NDEBUG
  frame.cf_prev    = CODE_FRAME_NOT_EXECUTING;
#endif
  frame.cf_stack   = frame.cf_frame+code->co_localc;
  frame.cf_sp      = frame.cf_stack;
  frame.cf_ip      = code->co_code;
  frame.cf_vargs   = NULL;
#ifdef NDEBUG
  /*frame.cf_this  = NULL;*/ /* Can be left uninitialized. */
#elif defined(UINT32_C) && defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
  frame.cf_this = (DeeObject *)UINT32_C(0xcccccccc);
#elif defined(UINT64_C) && defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
  frame.cf_this = (DeeObject *)UINT64_C(0xcccccccccccccccc);
#else
  memset(&frame.cf_this,0xcc,sizeof(DeeObject *));
#endif
  /* With the frame now set up, actually invoke the code. */
  if unlikely(code->co_flags&CODE_FASSEMBLY) {
   frame.cf_stacksz = 0;
   result = DeeCode_ExecFrameSafe(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
   /* Safe code execution allows for stack-space extension into heap memory.
    * >> Free that memory now that `DeeCode_ExecFrameSafe()' has finished. */
   if (frame.cf_stacksz) Dee_Free(frame.cf_stack);
   frame.cf_sp = frame.cf_frame+code->co_localc;
  } else {
   result = DeeCode_ExecFrameFast(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
  }
  /* Delete remaining local variables. */
  while (frame.cf_sp != frame.cf_frame)
       --frame.cf_sp,Dee_XDecref(*frame.cf_sp);

#ifdef Dee_Alloca
  if (code->co_flags&CODE_FHEAPFRAME)
#endif /* Dee_Alloca */
  {
   Dee_Free(frame.cf_frame);
  }
  Dee_XDecref(frame.cf_vargs);
  return result;
 }

 /* Special case: Create a yield-function callback. */
 {
  DREF DeeYieldFunctionObject *result;
  result = (DREF DeeYieldFunctionObject *)DeeGCObject_Malloc(sizeof(DeeYieldFunctionObject));
  if unlikely(!result) return NULL;
  result->yf_func = (DREF DeeFunctionObject *)self;
  /* Pack together an argument tuple for the yield-function. */
  result->yf_args = (DREF DeeTupleObject *)DeeTuple_NewVector(argc,argv);
  if unlikely(!result->yf_args) { DeeGCObject_Free(result); return NULL; }
  DeeObject_Init(result,&DeeYieldFunction_Type);
  result->yf_this = NULL;
#ifndef CONFIG_NO_THREADS
  rwlock_init(&result->yf_lock);
#endif
  Dee_Incref(self);
  DeeGC_Track((DeeObject *)result);
  return (DREF DeeObject *)result;
 }
}

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTERN DREF DeeObject *DCALL
DeeFunction_CallTuple(DeeFunctionObject *__restrict self,
                      DeeObject *__restrict args) {
 DeeCodeObject *code;
 ASSERT_OBJECT(self);
 ASSERT(DeeFunction_Check(self));
 code = self->fo_code;
 if unlikely(code->co_flags & CODE_FTHISCALL) {
  /* Special case: Invoke the function as a this-call. */
  if unlikely(!DeeTuple_SIZE(args)) {
   char *name = DeeDDI_NAME(code->co_ddi);
   err_invalid_argc(*name ? name : NULL,0,code->co_argc_min+1,
                     code->co_flags&CODE_FVARARGS ?
                   (size_t)-1 : ((size_t)code->co_argc_max+1));
   return NULL;
  }
  return DeeFunction_ThisCall(self,
                              DeeTuple_GET(args,0),
                              DeeTuple_SIZE(args) - 1,
                              DeeTuple_ELEM(args) + 1);
 }
 if unlikely(DeeTuple_SIZE(args) < code->co_argc_min ||
            (DeeTuple_SIZE(args) > code->co_argc_max &&
           !(code->co_flags&CODE_FVARARGS))) {
  /* ERROR: Invalid argument count! */
  char *name = DeeDDI_NAME(code->co_ddi);
  err_invalid_argc(*name ? name : NULL,
                    DeeTuple_SIZE(args),code->co_argc_min,
                    code->co_flags&CODE_FVARARGS ?
                  (size_t)-1 : ((size_t)code->co_argc_max));
  return NULL;
 }

 if (!(code->co_flags&CODE_FYIELDING)) {
  /* Default scenario: Perform a regular function call. */
  DeeObject *result;
  struct code_frame frame;
  frame.cf_func   = self;
  frame.cf_argc   = DeeTuple_SIZE(args);
  frame.cf_argv   = DeeTuple_ELEM(args);
  frame.cf_kw     = Dee_EmptyMapping;
  frame.cf_result = NULL;
#ifdef Dee_Alloca
  if (!(code->co_flags&CODE_FHEAPFRAME)) {
   frame.cf_frame = (DeeObject **)Dee_Alloca(code->co_framesize);
  } else
#endif /* Dee_Alloca */
  {
   frame.cf_frame = (DeeObject **)Dee_Malloc(code->co_framesize);
   if unlikely(!frame.cf_frame) return NULL;
  }
  /* Per-initialize local variable memory to ZERO. */
  MEMSET_PTR(frame.cf_frame,0,code->co_localc);
#ifndef NDEBUG
  frame.cf_prev    = CODE_FRAME_NOT_EXECUTING;
#endif
  frame.cf_stack   = frame.cf_frame+code->co_localc;
  frame.cf_sp      = frame.cf_stack;
  frame.cf_ip      = code->co_code;
  frame.cf_vargs   = NULL;
#ifdef NDEBUG
  /*frame.cf_this  = NULL;*/ /* Can be left uninitialized. */
#elif defined(UINT32_C) && defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
  frame.cf_this = (DeeObject *)UINT32_C(0xcccccccc);
#elif defined(UINT64_C) && defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
  frame.cf_this = (DeeObject *)UINT64_C(0xcccccccccccccccc);
#else
  memset(&frame.cf_this,0xcc,sizeof(DeeObject *));
#endif
  if (code->co_argc_min == 0)
      frame.cf_vargs = (DREF DeeTupleObject *)args;
  /* With the frame now set up, actually invoke the code. */
  if unlikely(code->co_flags&CODE_FASSEMBLY) {
   frame.cf_stacksz = 0;
   result = DeeCode_ExecFrameSafe(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
   /* Safe code execution allows for stack-space extension into heap memory.
    * >> Free that memory now that `DeeCode_ExecFrameSafe()' has finished. */
   if (frame.cf_stacksz) Dee_Free(frame.cf_stack);
   frame.cf_sp = frame.cf_frame+code->co_localc;
  } else {
   result = DeeCode_ExecFrameFast(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
  }
  /* Delete remaining local variables. */
  while (frame.cf_sp != frame.cf_frame)
       --frame.cf_sp,Dee_XDecref(*frame.cf_sp);

#ifdef Dee_Alloca
  if (code->co_flags&CODE_FHEAPFRAME)
#endif /* Dee_Alloca */
  {
   Dee_Free(frame.cf_frame);
  }
  if (code->co_argc_min != 0)
      Dee_XDecref(frame.cf_vargs);
  return result;
 }

 /* Special case: Create a yield-function callback. */
 {
  DREF DeeYieldFunctionObject *result;
  result = (DREF DeeYieldFunctionObject *)DeeGCObject_Malloc(sizeof(DeeYieldFunctionObject));
  if unlikely(!result) return NULL;
  result->yf_func = (DREF DeeFunctionObject *)self;
  result->yf_args = (DREF DeeTupleObject *)args;
  DeeObject_Init(result,&DeeYieldFunction_Type);
  result->yf_this = NULL;
#ifndef CONFIG_NO_THREADS
  rwlock_init(&result->yf_lock);
#endif
  Dee_Incref(self);
  Dee_Incref(args);
  DeeGC_Track((DeeObject *)result);
  return (DREF DeeObject *)result;
 }
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */


INTERN DREF DeeObject *DCALL
DeeFunction_ThisCallKw(DeeFunctionObject *__restrict self,
                       DeeObject *__restrict this_arg,
                       size_t argc, DeeObject **__restrict argv,
                       DeeObject *kw) {
 if (kw) {
  /* TODO: Keyword support for user-code functions */
  if (DeeKwds_Check(kw)) {
   if (DeeKwds_SIZE(kw) != 0)
       goto err_no_keywords;
  } else {
   size_t temp = DeeObject_Size(kw);
   if unlikely(temp == (size_t)-1) return NULL;
   if (temp != 0) goto err_no_keywords;
  }
 }
 return DeeFunction_ThisCall(self,this_arg,argc,argv);
err_no_keywords:
 err_keywords_func_not_accepted(DeeDDI_NAME(self->fo_code->co_ddi),kw);
 return NULL;
}


INTERN DREF DeeObject *DCALL
DeeFunction_ThisCall(DeeFunctionObject *__restrict self,
                     DeeObject *__restrict this_arg,
                     size_t argc, DeeObject **__restrict argv) {
 DeeCodeObject *code;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(this_arg);
 ASSERT(DeeFunction_Check(self));
 code = self->fo_code;
 if unlikely(!(code->co_flags&CODE_FTHISCALL)) {
  DREF DeeObject *result,*packed_args;
  /* Re-package the argument tuple and perform a regular call. */
  packed_args = DeeTuple_NewUninitialized(1+argc);
  if unlikely(!packed_args) return NULL;
  DeeTuple_SET(packed_args,0,this_arg);
  MEMCPY_PTR(DeeTuple_ELEM(packed_args)+1,argv,argc);
  /* Perform a regular callback. */
  result = function_call(self,
                         DeeTuple_SIZE(packed_args),
                         DeeTuple_ELEM(packed_args));
  /* The tuple we've created above only contained symbolic references. */
  DeeTuple_DecrefSymbolic(packed_args);
  return result;
 }
 if unlikely(argc < code->co_argc_min ||
            (argc > code->co_argc_max &&
           !(code->co_flags&CODE_FVARARGS))) {
  /* ERROR: Invalid argument count! */
  char *name = DeeDDI_NAME(code->co_ddi);
  err_invalid_argc(*name ? name : NULL,argc,code->co_argc_min,
                    code->co_flags&CODE_FVARARGS ? (size_t)-1 :
                  (size_t)code->co_argc_max);
  return NULL;
 }

 if (!(code->co_flags&CODE_FYIELDING)) {
  /* Default scenario: Perform a this-call. */
  DeeObject *result;
  struct code_frame frame;
  frame.cf_func = self;
  frame.cf_argc = argc;
  frame.cf_argv = argv;
  frame.cf_kw   = Dee_EmptyMapping;
  frame.cf_result = NULL;
#ifdef Dee_Alloca
  if (!(code->co_flags&CODE_FHEAPFRAME)) {
   frame.cf_frame = (DeeObject **)Dee_Alloca(code->co_framesize);
  } else
#endif /* Dee_Alloca */
  {
   frame.cf_frame = (DeeObject **)Dee_Malloc(code->co_framesize);
   if unlikely(!frame.cf_frame) return NULL;
  }
  /* Per-initialize local variable memory to ZERO. */
  MEMSET_PTR(frame.cf_frame,0,code->co_localc);
#ifndef NDEBUG
  frame.cf_prev   = CODE_FRAME_NOT_EXECUTING;
#endif
  frame.cf_stack  = frame.cf_frame+code->co_localc;
  frame.cf_sp     = frame.cf_stack;
  frame.cf_ip     = code->co_code;
  frame.cf_vargs  = NULL;
  frame.cf_this   = this_arg;
  /* With the frame now set up, actually invoke the code. */
  if unlikely(code->co_flags&CODE_FASSEMBLY) {
   frame.cf_stacksz = 0;
   result = DeeCode_ExecFrameSafe(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
   /* Safe code execution allows for stack-space extension into heap memory.
    * >> Free that memory now that `DeeCode_ExecFrameSafe()' has finished. */
   if (frame.cf_stacksz) Dee_Free(frame.cf_stack);
   frame.cf_sp = frame.cf_frame+code->co_localc;
  } else {
   result = DeeCode_ExecFrameFast(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
  }
  /* Delete remaining local variables. */
  while (frame.cf_sp != frame.cf_frame)
       --frame.cf_sp,Dee_XDecref(*frame.cf_sp);

#ifdef Dee_Alloca
  if (code->co_flags&CODE_FHEAPFRAME)
#endif /* Dee_Alloca */
  {
   Dee_Free(frame.cf_frame);
  }
  Dee_XDecref(frame.cf_vargs);
  return result;
 }
 /* Special case: Create a yield-function callback. */
 {
  DREF DeeYieldFunctionObject *result;
  result = (DREF DeeYieldFunctionObject *)DeeGCObject_Malloc(sizeof(DeeYieldFunctionObject));
  if unlikely(!result) return NULL;
  result->yf_func = self;
  /* Pack together an argument tuple for the yield-function. */
  result->yf_args = (DREF DeeTupleObject *)DeeTuple_NewVector(argc,argv);
  if unlikely(!result->yf_args) { DeeGCObject_Free(result); return NULL; }
  DeeObject_Init(result,&DeeYieldFunction_Type);
  result->yf_this = this_arg;
#ifndef CONFIG_NO_THREADS
  rwlock_init(&result->yf_lock);
#endif
  Dee_Incref(self);
  Dee_Incref(this_arg);
  DeeGC_Track((DeeObject *)result);
  return (DREF DeeObject *)result;
 }
}


#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTERN DREF DeeObject *DCALL
DeeFunction_ThisCallTuple(DeeFunctionObject *__restrict self,
                          DeeObject *__restrict this_arg,
                          DeeObject *__restrict args) {
 DeeCodeObject *code;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(this_arg);
 ASSERT(DeeFunction_Check(self));
 code = self->fo_code;
 if unlikely(!(code->co_flags&CODE_FTHISCALL)) {
  DREF DeeObject *result,*packed_args;
  /* Re-package the argument tuple and perform a regular call. */
  packed_args = DeeTuple_NewUninitialized(1+DeeTuple_SIZE(args));
  if unlikely(!packed_args) return NULL;
  DeeTuple_SET(packed_args,0,this_arg);
  MEMCPY_PTR(DeeTuple_ELEM(packed_args)+1,
             DeeTuple_ELEM(args),
             DeeTuple_SIZE(args));
  /* Perform a regular callback. */
  result = function_call(self,
                         DeeTuple_SIZE(packed_args),
                         DeeTuple_ELEM(packed_args));
  /* The tuple we've created above only contained symbolic references. */
  DeeTuple_DecrefSymbolic(packed_args);
  return result;
 }
 if unlikely(DeeTuple_SIZE(args) < code->co_argc_min ||
            (DeeTuple_SIZE(args) > code->co_argc_max &&
           !(code->co_flags&CODE_FVARARGS))) {
  /* ERROR: Invalid argument count! */
  char *name = DeeDDI_NAME(code->co_ddi);
  err_invalid_argc(*name ? name : NULL,
                    DeeTuple_SIZE(args),code->co_argc_min,
                    code->co_flags&CODE_FVARARGS ? (size_t)-1 :
                  (size_t)code->co_argc_max);
  return NULL;
 }

 if (!(code->co_flags&CODE_FYIELDING)) {
  /* Default scenario: Perform a this-call. */
  DeeObject *result;
  struct code_frame frame;
  frame.cf_func = self;
  frame.cf_argc = DeeTuple_SIZE(args);
  frame.cf_argv = DeeTuple_ELEM(args);
  frame.cf_kw   = Dee_EmptyMapping;
  frame.cf_result = NULL;
#ifdef Dee_Alloca
  if (!(code->co_flags&CODE_FHEAPFRAME)) {
   frame.cf_frame = (DeeObject **)Dee_Alloca(code->co_framesize);
  } else
#endif /* Dee_Alloca */
  {
   frame.cf_frame = (DeeObject **)Dee_Malloc(code->co_framesize);
   if unlikely(!frame.cf_frame) return NULL;
  }
  /* Per-initialize local variable memory to ZERO. */
  MEMSET_PTR(frame.cf_frame,0,code->co_localc);
#ifndef NDEBUG
  frame.cf_prev   = CODE_FRAME_NOT_EXECUTING;
#endif
  frame.cf_stack  = frame.cf_frame+code->co_localc;
  frame.cf_sp     = frame.cf_stack;
  frame.cf_ip     = code->co_code;
  frame.cf_vargs  = NULL;
  frame.cf_this   = this_arg;
  if (code->co_argc_min == 0)
      frame.cf_vargs = (DREF DeeTupleObject *)args;
  /* With the frame now set up, actually invoke the code. */
  if unlikely(code->co_flags&CODE_FASSEMBLY) {
   frame.cf_stacksz = 0;
   result = DeeCode_ExecFrameSafe(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
   /* Safe code execution allows for stack-space extension into heap memory.
    * >> Free that memory now that `DeeCode_ExecFrameSafe()' has finished. */
   if (frame.cf_stacksz) Dee_Free(frame.cf_stack);
   frame.cf_sp = frame.cf_frame+code->co_localc;
  } else {
   result = DeeCode_ExecFrameFast(&frame);
   /* Delete remaining stack objects. */
   while (frame.cf_sp != frame.cf_stack)
        --frame.cf_sp,Dee_Decref(*frame.cf_sp);
  }
  /* Delete remaining local variables. */
  while (frame.cf_sp != frame.cf_frame)
       --frame.cf_sp,Dee_XDecref(*frame.cf_sp);

#ifdef Dee_Alloca
  if (code->co_flags&CODE_FHEAPFRAME)
#endif /* Dee_Alloca */
  {
   Dee_Free(frame.cf_frame);
  }
  if (code->co_argc_min != 0)
      Dee_XDecref(frame.cf_vargs);
  return result;
 }
 /* Special case: Create a yield-function callback. */
 {
  DREF DeeYieldFunctionObject *result;
  result = (DREF DeeYieldFunctionObject *)DeeGCObject_Malloc(sizeof(DeeYieldFunctionObject));
  if unlikely(!result) return NULL;
  result->yf_func = self;
  /* Pack together an argument tuple for the yield-function. */
  result->yf_args = (DREF DeeTupleObject *)args;
  DeeObject_Init(result,&DeeYieldFunction_Type);
  result->yf_this = this_arg;
#ifndef CONFIG_NO_THREADS
  rwlock_init(&result->yf_lock);
#endif
  Dee_Incref(self);
  Dee_Incref(this_arg);
  Dee_Incref(args);
  DeeGC_Track((DeeObject *)result);
  return (DREF DeeObject *)result;
 }
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */


#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTERN DREF DeeObject *DCALL
DeeFunction_CallTupleKw(DeeFunctionObject *__restrict self,
                        DeeObject *__restrict args,
                        DeeObject *kw) {
 if (kw) {
  /* TODO: Keyword support for user-code functions */
  if (DeeKwds_Check(kw)) {
   if (DeeKwds_SIZE(kw) != 0)
       goto err_no_keywords;
  } else {
   size_t temp = DeeObject_Size(kw);
   if unlikely(temp == (size_t)-1) return NULL;
   if (temp != 0) goto err_no_keywords;
  }
 }
 return DeeFunction_CallTuple(self,args);
err_no_keywords:
 err_keywords_func_not_accepted(DeeDDI_NAME(self->fo_code->co_ddi),kw);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeFunction_ThisCallTupleKw(DeeFunctionObject *__restrict self,
                            DeeObject *__restrict this_arg,
                            DeeObject *__restrict args,
                            DeeObject *kw) {
 if (kw) {
  /* TODO: Keyword support for user-code functions */
  if (DeeKwds_Check(kw)) {
   if (DeeKwds_SIZE(kw) != 0)
       goto err_no_keywords;
  } else {
   size_t temp = DeeObject_Size(kw);
   if unlikely(temp == (size_t)-1) return NULL;
   if (temp != 0) goto err_no_keywords;
  }
 }
 return DeeFunction_ThisCallTuple(self,this_arg,args);
err_no_keywords:
 err_keywords_func_not_accepted(DeeDDI_NAME(self->fo_code->co_ddi),kw);
 return NULL;
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */

DECL_END


#endif /* !GUARD_DEEMON_EXECUTE_CODE_C */
