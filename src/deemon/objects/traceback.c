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
#ifndef GUARD_DEEMON_OBJECTS_TRACEBACK_C
#define GUARD_DEEMON_OBJECTS_TRACEBACK_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/traceback.h>
#include <deemon/seq.h>
#include <deemon/none.h>
#include <deemon/int.h>
#include <deemon/gc.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#include <stddef.h>
#include <stdarg.h>
#include <string.h>

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
INTDEF DeeThreadObject DeeThread_Main;
#else
DATDEF DeeThreadObject DeeThread_Main;
#endif

INTERN struct empty_traceback_object empty_traceback = {
    OBJECT_HEAD_INIT(&DeeTraceback_Type),
    /* .tb_thread    = */&DeeThread_Main,
#ifndef CONFIG_NO_THREADS
    /* .tb_lock      = */RWLOCK_INIT,
#endif
    /* .tb_numframes = */0,
    /* .tb_padding   = */{0,0,0}
};



INTERN DREF DeeTracebackObject *DCALL
DeeTraceback_New(struct thread_object *__restrict thread) {
 DREF DeeTracebackObject *result;
 ASSERT(thread == DeeThread_Self());
 result = (DREF DeeTracebackObject *)DeeGCObject_TryMalloc(offsetof(DeeTracebackObject,tb_frames)+
                                                           thread->t_execsz*sizeof(struct code_frame));
 if likely(result) {
  struct code_frame *dst,*src;
  DeeObject_Init(result,&DeeTraceback_Type);
  result->tb_numframes = thread->t_execsz;
  result->tb_thread = thread;
  Dee_Incref(thread);
  rwlock_init(&result->tb_lock);
  dst = result->tb_frames+thread->t_execsz;
  src = thread->t_exec;
  while (dst-- != result->tb_frames) {
   DeeCodeObject *code;
   DeeObject *dont_track_this = NULL;
   ASSERT(src != NULL);
   ASSERT(src != CODE_FRAME_NOT_EXECUTING);
   /* Do a shallow memcopy of the execution frame. */
   memcpy(dst,src,sizeof(struct code_frame));
   /* Create references and duplicate local variables. */
   ASSERT_OBJECT_TYPE(dst->cf_func,&DeeFunction_Type);
   ASSERT_OBJECT_TYPE(dst->cf_func->fo_code,&DeeCode_Type);
   Dee_Incref(dst->cf_func);
   code = dst->cf_func->fo_code;
   dst->cf_flags = code->co_flags;
   if (code->co_flags&CODE_FTHISCALL) {
    ASSERT_OBJECT(dst->cf_this);
    if (!(code->co_flags&CODE_FCONSTRUCTOR)) {
     Dee_Incref(dst->cf_this);
    } else {
     dont_track_this = dst->cf_this;
     dst->cf_this = NULL;
    }
   } else {
    dst->cf_this = NULL;
   }
   if (!dst->cf_argc)
    dst->cf_argv = NULL;
   else if ((dst->cf_argv = (DREF DeeObject **)Dee_TryMalloc(dst->cf_argc*
                                                             sizeof(DREF DeeObject *))) != NULL) {
    DREF DeeObject **iter,**end,**src_iter;
    end = (iter = dst->cf_argv)+dst->cf_argc;
    src_iter = src->cf_argv;
    for (; iter != end; ++iter,++src_iter) {
     if (*src_iter == dont_track_this) {
      *iter = Dee_None;
      Dee_Incref(Dee_None);
     } else {
      DeeObject *ob = *src_iter;
      Dee_Incref(ob);
      *iter = ob;
     }
    }
   }
   Dee_XIncref(dst->cf_vargs);
   if (ITER_ISOK(dst->cf_result))
       Dee_Incref(dst->cf_result);
   /* Duplicate local variables. */
   dst->cf_frame = (DREF DeeObject **)Dee_TryMalloc(code->co_localc*
                                                    sizeof(DREF DeeObject *));
   if (dst->cf_frame) {
    DREF DeeObject **iter,**end,**source_iter;
    source_iter = src->cf_frame;
    end = (iter = dst->cf_frame)+code->co_localc;
    for (; iter != end; ++iter,++source_iter) {
     if (*source_iter == dont_track_this) {
      *iter = Dee_None;
      Dee_Incref(Dee_None);
     } else {
      DeeObject *ob = *source_iter;
      Dee_XIncref(ob);
      *iter = ob;
     }
    }
   }
   /* At this point, the contents of the stack can't be trusted. */
   dst->cf_stack   = (DREF DeeObject **)dont_track_this; /* Save this here so that `DeeTraceback_AddFrame()' sees it. */
   dst->cf_sp      = NULL;
   dst->cf_stacksz = 0;
   /* Continue with the next frame. */
   src = src->cf_prev;
  }
  ASSERT(src == NULL);
  DeeGC_Track((DeeObject *)result);
 }
 return result;
}

INTERN void DCALL
DeeTraceback_AddFrame(DeeTracebackObject *__restrict self,
                      struct code_frame *__restrict frame,
                      uint16_t frame_id) {
 struct code_frame *dst; uint16_t stacksz;
 DeeObject *dont_track_this;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeTraceback_Type);
 if unlikely(frame_id >= self->tb_numframes) return; /* Untracked frame. */
 dst = &self->tb_frames[frame_id];
 if unlikely(dst->cf_prev != frame->cf_prev) return; /* Different frame. */
 if unlikely(self->tb_thread != DeeThread_Self()) return; /* Different thread. */
 rwlock_write(&self->tb_lock);
 if unlikely(dst->cf_stacksz)
    goto done; /* Frame already initialized */
 ASSERT(!dst->cf_sp);
 ASSERT(!dst->cf_stacksz);
 dont_track_this = (DeeObject *)dst->cf_stack;
 dst->cf_stack   = NULL;
 /* Since we can't report errors, only ~try~ to copy stacks. */
 stacksz = (uint16_t)(frame->cf_sp-frame->cf_stack);
 if unlikely(!stacksz) goto done;
 dst->cf_stack = (DREF DeeObject **)Dee_TryMalloc(stacksz*sizeof(DREF DeeObject *));
 if likely(dst->cf_stack) {
  dst->cf_stacksz = stacksz;
  dst->cf_sp      = dst->cf_stack+stacksz;
  /* Duplicate the stack. */
  while (stacksz--) {
   dst->cf_stack[stacksz] = frame->cf_stack[stacksz];
   if (dst->cf_stack[stacksz] == dont_track_this)
       dst->cf_stack[stacksz] = Dee_None;
   ASSERT_OBJECT(dst->cf_stack[stacksz]);
   Dee_Incref(dst->cf_stack[stacksz]);
  }
 }
done:
 rwlock_endwrite(&self->tb_lock);
}


typedef struct {
    OBJECT_HEAD
    DREF DeeTracebackObject *ti_trace; /* [1..1][const] The traceback that is being iterated. */
    struct code_frame       *ti_next;  /* [1..1][in(ti_trace->tb_frames)][atomic]
                                        * The next frame (yielded in reverse order) */
} TraceIterator;
INTDEF DeeTypeObject DeeTracebackIterator_Type;

PRIVATE void DCALL
traceiter_fini(TraceIterator *__restrict self) {
 Dee_Decref(self->ti_trace);
}
PRIVATE void DCALL
traceiter_visit(TraceIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->ti_trace);
}
PRIVATE DREF DeeObject *DCALL
traceiter_next(TraceIterator *__restrict self) {
 struct code_frame *result_frame;
#ifdef CONFIG_NO_THREADS
 result_frame = self->ti_next;
 if unlikely(result_frame < self->ti_trace->tb_frames)
    return ITER_DONE;
 self->ti_next = result_frame-1;
#else
 do {
  result_frame = ATOMIC_READ(self->ti_next);
  if unlikely(result_frame < self->ti_trace->tb_frames)
     return ITER_DONE;
 } while (!ATOMIC_CMPXCH(self->ti_next,result_frame,result_frame-1));
#endif
 /* Create a new frame wrapper for this entry. */
 return DeeFrame_NewReferenceWithLock((DeeObject *)self->ti_trace,
                                       result_frame,
                                       DEEFRAME_FREADONLY,
                                      &self->ti_trace->tb_lock);
}

PRIVATE struct type_member traceiter_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,
                       offsetof(TraceIterator,ti_trace)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeTracebackIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"traceback.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                TYPE_FIXED_ALLOCATOR(TraceIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&traceiter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&traceiter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */traceiter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


PRIVATE DREF DeeTracebackObject *DCALL traceback_new(void) {
 return DeeTraceback_New(DeeThread_Self());
}
PRIVATE void DCALL
traceback_fini(DeeTracebackObject *__restrict self) {
 struct code_frame *frame; size_t i,frame_index;
 Dee_Decref(self->tb_thread);
 frame_index = self->tb_numframes;
 while (frame_index--) {
  frame = &self->tb_frames[frame_index];
  ASSERT_OBJECT_TYPE(frame->cf_func,&DeeFunction_Type);
  ASSERT_OBJECT_TYPE(frame->cf_func->fo_code,&DeeCode_Type);
  /* Decref local variables. */
  i = frame->cf_func->fo_code->co_localc;
  while (i--) Dee_XDecref(frame->cf_frame[i]);
  Dee_Free(frame->cf_frame);
  /* Decref stack objects. */
  if (frame->cf_stacksz) {
   i = frame->cf_stacksz;
   while (i--) Dee_Decref(frame->cf_stack[i]);
   Dee_Free(frame->cf_stack);
  }
  /* Decref argument objects. */
  i = frame->cf_argc;
  while (i--) Dee_Decref(frame->cf_argv[i]);
  Dee_Free(frame->cf_argv);
  /* Decref misc. frame objects. */
  Dee_Decref(frame->cf_func);
  Dee_XDecref(frame->cf_this);
  Dee_XDecref(frame->cf_vargs);
  if (ITER_ISOK(frame->cf_result))
      Dee_Decref(frame->cf_result);
 }
}

PRIVATE void DCALL
traceback_visit(DeeTracebackObject *__restrict self,
                dvisit_t proc, void *arg) {
 struct code_frame *iter,*end;
 rwlock_read(&self->tb_lock);
 Dee_Visit(self->tb_thread);
 end = (iter = self->tb_frames)+self->tb_numframes;
 for (; iter != end; ++iter) {
  DREF DeeObject **oiter,**oend;
  ASSERT_OBJECT_TYPE(iter->cf_func,&DeeFunction_Type);
  ASSERT_OBJECT_TYPE(iter->cf_func->fo_code,&DeeCode_Type);
  /* Visit local variables. */
  oend = (oiter = iter->cf_frame)+iter->cf_func->fo_code->co_localc;
  if (oiter) for (; oiter != oend; ++oiter) Dee_XVisit(*oiter);
  /* Visit stack objects. */
  oend = (oiter = iter->cf_stack)+iter->cf_stacksz;
  for (; oiter != oend; ++oiter) Dee_Visit(*oiter);
  /* Visit argument objects. */
  oend = (oiter = iter->cf_argv)+iter->cf_argc;
  for (; oiter != oend; ++oiter) Dee_Visit(*oiter);
  Dee_Visit(iter->cf_func);
  Dee_XVisit(iter->cf_this);
  Dee_XVisit(iter->cf_vargs);
  if (ITER_ISOK(iter->cf_result))
      Dee_Visit(iter->cf_result);
 }
 rwlock_endread(&self->tb_lock);
}

PRIVATE void DCALL
traceback_clear(DeeTracebackObject *__restrict self) {
 DREF DeeObject *decref_later_buffer[64],**decref_later;
 struct code_frame *iter,*end;
 decref_later = decref_later_buffer;
 rwlock_write(&self->tb_lock);
 end = (iter = self->tb_frames)+self->tb_numframes;
 for (; iter != end; ++iter) {
  DREF DeeObject **oiter,**oend;
  ASSERT_OBJECT_TYPE(iter->cf_func,&DeeFunction_Type);
  ASSERT_OBJECT_TYPE(iter->cf_func->fo_code,&DeeCode_Type);
  /* Decref local variables. */
  oend = (oiter = iter->cf_frame)+iter->cf_func->fo_code->co_localc;
  if (oiter) for (; oiter != oend; ++oiter) {
   DeeObject *ob = *oiter;
   if (!ob) continue;
   *oiter = NULL;
   if (!Dee_DecrefIfNotOne(ob)) {
    *decref_later++ = ob;
    if (decref_later == COMPILER_ENDOF(decref_later_buffer))
        goto clear_buffer;
   }
  }
  /* Decref stack objects. */
  oend = (oiter = iter->cf_stack)+iter->cf_stacksz;
  for (; oiter != oend; ++oiter) {
   DeeObject *ob = *oiter;
   if (DeeNone_Check(ob)) continue;
   *oiter = Dee_None;
   Dee_Incref(Dee_None);
   if (!Dee_DecrefIfNotOne(ob)) {
    *decref_later++ = ob;
    if (decref_later == COMPILER_ENDOF(decref_later_buffer))
        goto clear_buffer;
   }
  }
  /* Decref argument objects. */
  oend = (oiter = iter->cf_argv)+iter->cf_argc;
  for (; oiter != oend; ++oiter) {
   DeeObject *ob = *oiter;
   if (DeeNone_Check(ob)) continue;
   *oiter = Dee_None;
   Dee_Incref(Dee_None);
   if (!Dee_DecrefIfNotOne(ob)) {
    *decref_later++ = ob;
    if (decref_later == COMPILER_ENDOF(decref_later_buffer))
        goto clear_buffer;
   }
  }
  if (iter->cf_this) {
   DeeObject *ob = iter->cf_this;
   iter->cf_this = NULL;
   if (!Dee_DecrefIfNotOne(ob)) {
    *decref_later++ = ob;
    if (decref_later == COMPILER_ENDOF(decref_later_buffer))
        goto clear_buffer;
   }
  }
  if (iter->cf_vargs) {
   DeeObject *ob = (DeeObject *)iter->cf_vargs;
   iter->cf_vargs = NULL;
   if (!Dee_DecrefIfNotOne(ob)) {
    *decref_later++ = ob;
    if (decref_later == COMPILER_ENDOF(decref_later_buffer))
        goto clear_buffer;
   }
  }
  if (ITER_ISOK(iter->cf_result)) {
   DeeObject *ob = iter->cf_result;
   iter->cf_result = NULL;
   if (!Dee_DecrefIfNotOne(ob)) {
    *decref_later++ = ob;
    if (decref_later == COMPILER_ENDOF(decref_later_buffer))
        goto clear_buffer;
   }
  }
 }
 rwlock_endwrite(&self->tb_lock);
 while (decref_later-- != decref_later_buffer)
        Dee_Decref(*decref_later);
 return;
clear_buffer:
 rwlock_endwrite(&self->tb_lock);
 while (decref_later-- != decref_later_buffer)
        Dee_Decref(*decref_later);
 decref_later = decref_later_buffer;
}

PRIVATE DREF TraceIterator *DCALL
traceback_iter(DeeTracebackObject *__restrict self) {
 TraceIterator *result;
 /* Create a new traceback iterator object. */
 result = DeeObject_MALLOC(TraceIterator);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&DeeTracebackIterator_Type);
 result->ti_next  = self->tb_frames+self->tb_numframes-1;
 result->ti_trace = self;
 Dee_Incref(self);
done:
 return result;
}

INTDEF dssize_t DCALL
print_ddi(struct ascii_printer *__restrict printer,
          DeeCodeObject *__restrict code, code_addr_t ip);

PRIVATE DREF DeeObject *DCALL
traceback_repr(DeeTracebackObject *__restrict self) {
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 uint16_t i = self->tb_numframes;
 while (i--) {
  DREF DeeCodeObject *code;
  code_addr_t ip; dssize_t error;
  rwlock_read(&self->tb_lock);
  code = self->tb_frames[i].cf_func->fo_code;
  Dee_Incref(code);
  ip = (code_addr_t)(self->tb_frames[i].cf_ip-code->co_code);
  rwlock_endread(&self->tb_lock);
  error = print_ddi(&printer,code,ip);
  Dee_Decref(code);
  if unlikely(error < 0) goto err;
 }
 return ascii_printer_pack(&printer);
err:
 ascii_printer_fini(&printer);
 return NULL;
}


PRIVATE struct type_seq traceback_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceback_iter,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};

PRIVATE DREF DeeObject *DCALL
traceback_current(DeeObject *__restrict UNUSED(self)) {
 DREF DeeThreadObject *thread;
 DREF DeeObject *result;
 thread = DeeThread_Self();
 if unlikely(!thread->t_except) {
  err_no_active_exception();
  return NULL;
 }
 result = (DREF DeeObject *)except_frame_gettb(thread->t_except);
 if unlikely(!result) result = (DREF DeeObject *)&empty_traceback;
 Dee_Incref(result);
 return result;
}

PRIVATE struct type_getset traceback_class_getsets[] = {
    { "current", &traceback_current, NULL, NULL,
      DOC("->?.\n"
          "@throw RuntimeError No exception was being handled"
          "Returns the traceback associated with the current exception") },
    { NULL }
};
PRIVATE struct type_member traceback_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeTracebackIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE struct type_gc traceback_gc = {
    /* .tp_clear = */(void(DCALL *)(DeeObject *__restrict))&traceback_clear
};

PUBLIC DeeTypeObject DeeTraceback_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_traceback),
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FVARIABLE|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */&traceback_new,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&traceback_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceback_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&traceback_visit,
    /* .tp_gc            = */&traceback_gc,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&traceback_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */traceback_class_getsets,
    /* .tp_class_members = */traceback_class_members
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TRACEBACK_C */
