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
#ifndef GUARD_DEEMON_OBJECTS_ERROR_TYPES_C
#define GUARD_DEEMON_OBJECTS_ERROR_TYPES_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/error.h>
#include <deemon/string.h>
#include <deemon/none.h>
#include <deemon/exec.h>
#include <deemon/format.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/error_types.h>
#include <deemon/compiler/tpp.h>
#include <deemon/module.h>

#include "../runtime/strings.h"

#ifndef CONFIG_NO_STDLIB
#include <stdlib.h> /* EXIT_FAILURE */
#endif

DECL_BEGIN


#define INIT_CUSTOM_ERROR(tp_name,tp_doc,tp_flags,tp_base, \
                          tp_ctor,tp_copy_ctor,tp_deep_ctor,tp_any_ctor, \
                          T,tp_dtor,tp_str,tp_visit, \
                          tp_methods,tp_getsets,tp_members, \
                          tp_class_members) \
{ \
    OBJECT_HEAD_INIT(&DeeType_Type), \
    /* .tp_name     = */tp_name, \
    /* .tp_doc      = */DOC(tp_doc), \
    /* .tp_flags    = */tp_flags, \
    /* .tp_weakrefs = */0, \
    /* .tp_features = */TF_NONE, \
    /* .tp_base     = */tp_base, \
    /* .tp_init = */{ \
        { \
            /* .tp_alloc = */{ \
                /* .tp_ctor      = */(void *)(tp_ctor), \
                /* .tp_copy_ctor = */(void *)(tp_copy_ctor), \
                /* .tp_deep_ctor = */(void *)(tp_deep_ctor), \
                /* .tp_any_ctor  = */(void *)(tp_any_ctor), \
                TYPE_FIXED_ALLOCATOR(T) \
            } \
        }, \
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))(tp_dtor), \
        /* .tp_assign      = */NULL, \
        /* .tp_move_assign = */NULL \
    }, \
    /* .tp_cast = */{ \
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
        /* .tp_bool = */NULL \
    }, \
    /* .tp_call          = */NULL, \
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))(tp_visit), \
    /* .tp_gc            = */NULL, \
    /* .tp_math          = */NULL, \
    /* .tp_cmp           = */NULL, \
    /* .tp_seq           = */NULL, \
    /* .tp_iter_next     = */NULL, \
    /* .tp_attr          = */NULL, \
    /* .tp_with          = */NULL, \
    /* .tp_buffer        = */NULL, \
    /* .tp_methods       = */tp_methods, \
    /* .tp_getsets       = */tp_getsets, \
    /* .tp_members       = */tp_members, \
    /* .tp_class_methods = */NULL, \
    /* .tp_class_getsets = */NULL, \
    /* .tp_class_members = */tp_class_members \
}


#ifndef CONFIG_NO_OBJECT_SLABS
LOCAL size_t DCALL get_slab_size(void (DCALL *tp_free)(void *__restrict ob)) {
#define CHECK_SIZE(x) \
 if (tp_free == &DeeObject_SlabFree##x || \
     tp_free == &DeeGCObject_SlabFree##x) \
     return x * __SIZEOF_POINTER__;
 DEE_ENUMERATE_SLAB_SIZES(CHECK_SIZE)
#undef CHECK_SIZE
 return 0;
}

#define GET_INSTANCE_SIZE(self) \
   (Dee_TYPE(self)->tp_init.tp_alloc.tp_free \
  ? get_slab_size(Dee_TYPE(self)->tp_init.tp_alloc.tp_free) \
  : Dee_TYPE(self)->tp_init.tp_alloc.tp_instance_size)
#else /* !CONFIG_NO_OBJECT_SLABS */
#define GET_INSTANCE_SIZE(self) \
   (Dee_TYPE(self)->tp_init.tp_alloc.tp_instance_size)
#endif /* CONFIG_NO_OBJECT_SLABS */


/* Try to construct the given error object using its legacy constructor.
 * Note that any additional memory are zero-initialized upon success (true),
 * and that no error will be thrown on failure (false).
 * NOTE: Upon failure, the message and inner fields are initialized to NULL. */
PRIVATE bool DCALL
error_try_init(DeeErrorObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 size_t instance_size;
 ASSERT(!(Dee_TYPE(self)->tp_flags&TP_FVARIABLE));
 instance_size = GET_INSTANCE_SIZE(self);
 ASSERT(instance_size >= sizeof(DeeErrorObject));
 switch (argc) {
 case 0:
  self->e_message = NULL;
  self->e_inner   = NULL;
  goto done_ok;
 case 1:
  if (!DeeString_Check(argv[0])) break;
  self->e_message = (DeeStringObject *)argv[0];
  Dee_Incref(self->e_message);
  self->e_inner   = NULL;
  goto done_ok;
 case 2:
  if (!DeeString_Check(argv[0])) break;
  self->e_message = (DeeStringObject *)argv[0];
  self->e_inner   = argv[1];
  Dee_Incref(self->e_message);
  Dee_Incref(self->e_inner);
  goto done_ok;
 default: break;
 }
 self->e_message = NULL;
 self->e_inner   = NULL;
 return false;
done_ok:
 /* Clear our any additional fields. */
 memset(self+1,0,instance_size - sizeof(DeeErrorObject));
 return true;
}




/* BEGIN::Error */
PRIVATE struct type_member error_class_members[] = {
    TYPE_MEMBER_CONST("AttributeError",&DeeError_AttributeError),
    TYPE_MEMBER_CONST("CompilerError",&DeeError_CompilerError),
    TYPE_MEMBER_CONST("ThreadCrash",&DeeError_ThreadCrash),
    TYPE_MEMBER_CONST("NoMemory",&DeeError_NoMemory),
    TYPE_MEMBER_CONST("RuntimeError",&DeeError_RuntimeError),
    TYPE_MEMBER_CONST("TypeError",&DeeError_TypeError),
    TYPE_MEMBER_CONST("ValueError",&DeeError_ValueError),
    TYPE_MEMBER_CONST("SystemError",&DeeError_SystemError),
    TYPE_MEMBER_CONST("AppExit",&DeeError_AppExit),
    TYPE_MEMBER_END
};
PRIVATE int DCALL
error_ctor(DeeErrorObject *__restrict self) {
 size_t instance_size;
 ASSERT(!(Dee_TYPE(self)->tp_flags&TP_FVARIABLE));
 instance_size = GET_INSTANCE_SIZE(self);
 ASSERT(instance_size >= sizeof(DeeErrorObject));
 memset(&self->e_message,0,instance_size - offsetof(DeeErrorObject,e_message));
 ASSERT(!self->e_inner);
 ASSERT(!self->e_message);
 return 0;
}
PRIVATE int DCALL
error_copy(DeeErrorObject *__restrict self,
           DeeErrorObject *__restrict other) {
 size_t instance_size;
 ASSERT(!(Dee_TYPE(self)->tp_flags&TP_FVARIABLE));
 instance_size = GET_INSTANCE_SIZE(self);
 ASSERT(instance_size >= sizeof(DeeErrorObject));
 self->e_inner   = other->e_inner;
 self->e_message = other->e_message;
 Dee_XIncref(self->e_inner);
 Dee_XIncref(self->e_message);
 memset(self+1,0,instance_size - sizeof(DeeErrorObject));
 return 0;
}
PRIVATE int DCALL
error_deep(DeeErrorObject *__restrict self,
           DeeErrorObject *__restrict other) {
 size_t instance_size;
 ASSERT(!(Dee_TYPE(self)->tp_flags&TP_FVARIABLE));
 instance_size = GET_INSTANCE_SIZE(self);
 ASSERT(instance_size >= sizeof(DeeErrorObject));
 self->e_inner = NULL;
 if (other->e_inner) {
  self->e_inner = DeeObject_DeepCopy(other->e_inner);
  if unlikely(!self->e_inner) return -1;
 }
 self->e_message = other->e_message;
 Dee_XIncref(self->e_message);
 memset(self+1,0,instance_size - sizeof(DeeErrorObject));
 return 0;
}

PRIVATE char const error_init_fmt[] = "|oo:Error";
PRIVATE struct keyword error_init_kwlist[] = { K(message), K(inner), KEND };

PRIVATE int DCALL
error_init(DeeErrorObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 size_t instance_size;
 ASSERT(!(Dee_TYPE(self)->tp_flags&TP_FVARIABLE));
 instance_size = GET_INSTANCE_SIZE(self);
 ASSERT(instance_size >= sizeof(DeeErrorObject));
 self->e_message = NULL;
 self->e_inner   = NULL;
 if (DeeArg_Unpack(argc,argv,error_init_fmt,&self->e_message,&self->e_inner))
     goto err;
 if (self->e_message &&
     DeeObject_AssertTypeExact(self->e_message,&DeeString_Type))
     goto err;
 Dee_XIncref(self->e_message);
 Dee_XIncref(self->e_inner);
 memset(self+1,0,instance_size - sizeof(DeeErrorObject));
 return 0;
err:
 return -1;
}
PRIVATE int DCALL
error_init_kw(DeeErrorObject *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
 size_t instance_size;
 ASSERT(!(Dee_TYPE(self)->tp_flags&TP_FVARIABLE));
 instance_size = GET_INSTANCE_SIZE(self);
 ASSERT(instance_size >= sizeof(DeeErrorObject));
 self->e_message = NULL;
 self->e_inner   = NULL;
 if (DeeArg_UnpackKw(argc,argv,kw,error_init_kwlist,error_init_fmt,&self->e_message,&self->e_inner))
     goto err;
 if (self->e_message &&
     DeeObject_AssertTypeExact(self->e_message,&DeeString_Type))
     goto err;
 Dee_XIncref(self->e_message);
 Dee_XIncref(self->e_inner);
 memset(self+1,0,instance_size - sizeof(DeeErrorObject));
 return 0;
err:
 return -1;
}
PRIVATE void DCALL
error_fini(DeeErrorObject *__restrict self) {
 Dee_XDecref(self->e_message);
 Dee_XDecref(self->e_inner);
}
PRIVATE void DCALL
error_visit(DeeErrorObject *__restrict self, dvisit_t proc, void *arg) {
 Dee_XVisit(self->e_message);
 Dee_XVisit(self->e_inner);
}
PRIVATE DREF DeeObject *DCALL
error_str(DeeErrorObject *__restrict self) {
 if (self->e_message)
     return_reference_((DeeObject *)self->e_message);
 if (self->e_inner)
     return DeeString_Newf("%k -> %k",Dee_TYPE(self),self->e_inner);
 return DeeObject_Str((DeeObject *)Dee_TYPE(self));
}
PRIVATE DREF DeeObject *DCALL
error_repr(DeeErrorObject *__restrict self) {
 if (self->e_inner) {
  if (self->e_message) {
   return DeeString_Newf("%k(%r,inner: %r)",
                         Dee_TYPE(self),
                         self->e_message,
                         self->e_inner);
  }
  return DeeString_Newf("%k(inner: %r)",
                        Dee_TYPE(self),
                        self->e_inner);
 }
 return DeeString_Newf("%k(%r)",
                       Dee_TYPE(self),
                       self->e_message);
}
PRIVATE struct type_member error_members[] = {
    TYPE_MEMBER_FIELD_DOC("inner",STRUCT_OBJECT_OPT,offsetof(DeeErrorObject,e_inner),
                          "->?X3?DError?O?N\n"
                          "An optional inner error object, or :none when not set"),
    TYPE_MEMBER_FIELD_DOC("message",STRUCT_OBJECT_OPT,offsetof(DeeErrorObject,e_message),
                          "->?X2?Dstring?N\n"
                          "The error message associated with this Error object, or :none when not set"),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_Error = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_Error),
    /* .tp_doc      = */DOC("Base class for all errors thrown by the runtime\n"
                            "\n"
                            "(message?:?Dstring,inner?:?X2?DError?O)\n"
                            "Create a new error object with the given @message and @inner error"
                            ),
    /* .tp_flags    = */TP_FNORMAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(void *)&error_ctor,
                /* .tp_copy_ctor = */(void *)&error_copy,
                /* .tp_deep_ctor = */(void *)&error_deep,
                /* .tp_any_ctor  = */(void *)&error_init,
                TYPE_FIXED_ALLOCATOR(DeeErrorObject),
                /* .tp_any_ctor_kw = */(void *)&error_init_kw,
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&error_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&error_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&error_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&error_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */error_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */error_class_members
};
/* END::Error */











/* BEGIN::AttributeError */
PRIVATE struct type_member attribute_error_class_members[] = {
    TYPE_MEMBER_CONST("UnboundAttribute",&DeeError_UnboundAttribute),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_AttributeError =
INIT_CUSTOM_ERROR("AttributeError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,attribute_error_class_members);
PUBLIC DeeTypeObject DeeError_UnboundAttribute =
INIT_CUSTOM_ERROR("UnboundAttribute",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_AttributeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::AttributeError */






/* BEGIN::CompilerError */
PRIVATE void DCALL
comerr_fini(DeeCompilerErrorObject *__restrict self) {
 size_t i,count = self->ce_errorc;
 ASSERTF(self->ce_locs.cl_prev == NULL ||
         self->ce_locs.cl_file != NULL,
         "More than one location requires a base file to be present");
 if (self->ce_locs.cl_file) {
  /* Cleanup saved file locations. */
  struct compiler_error_loc *next,*iter;
  TPPFile_Decref(self->ce_locs.cl_file);
  iter = self->ce_locs.cl_prev;
  while (iter) {
   next = iter->cl_prev;
   ASSERT(iter->cl_file);
   TPPFile_Decref(iter->cl_file);
   Dee_Free(iter);
   iter = next;
  }
 }
 for (i = 0; i < count; ++i) {
  if (self->ce_errorv[i] != self)
      Dee_Decref(self->ce_errorv[i]);
 }
 Dee_Free(self->ce_errorv);
 Dee_weakref_fini(&self->ce_master);
}
PRIVATE void DCALL
comerr_visit(DeeCompilerErrorObject *__restrict self,
                   dvisit_t proc, void *arg) {
 size_t i,count = self->ce_errorc;
 for (i = 0; i < count; ++i) {
  if (self->ce_errorv[i] != self)
      Dee_Visit(self->ce_errorv[i]);
 }
}

INTDEF dssize_t DCALL
comerr_print(DeeCompilerErrorObject *__restrict self,
             dformatprinter printer, void *arg);
PRIVATE DREF DeeObject *DCALL
comerr_str(DeeCompilerErrorObject *__restrict self) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if unlikely(comerr_print(self,(dformatprinter)&unicode_printer_print,&printer) < 0)
    goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}
PRIVATE struct type_member compiler_error_class_members[] = {
    TYPE_MEMBER_CONST("SyntaxError",&DeeError_SyntaxError),
    TYPE_MEMBER_CONST("SymbolError",&DeeError_SymbolError),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_CompilerError =
INIT_CUSTOM_ERROR("CompilerError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,NULL,NULL,NULL,DeeCompilerErrorObject,
                 &comerr_fini,&comerr_str,&comerr_visit,NULL,NULL,NULL,
                  compiler_error_class_members);
PUBLIC DeeTypeObject DeeError_SyntaxError =
INIT_CUSTOM_ERROR("SyntaxError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_CompilerError,
                  NULL,NULL,NULL,NULL,DeeCompilerErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_SymbolError =
INIT_CUSTOM_ERROR("SymbolError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_CompilerError,
                  NULL,NULL,NULL,NULL,DeeCompilerErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::CompilerError */






PUBLIC DeeTypeObject DeeError_ThreadCrash =
INIT_CUSTOM_ERROR("ThreadCrash",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);




/* BEGIN::NoMemory */
PRIVATE struct type_member nomemory_members[] = {
    TYPE_MEMBER_FIELD("bytes",STRUCT_SIZE_T,offsetof(DeeNoMemoryErrorObject,nm_allocsize)),
    TYPE_MEMBER_END
};
PRIVATE int DCALL
nomemory_ctor(DeeNoMemoryErrorObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 if (error_try_init((DeeErrorObject *)self,argc,argv)) goto done;
 self->nm_allocsize = 1;
 if (DeeArg_Unpack(argc,argv,"|Iu:NoMemory",&self->nm_allocsize))
     return -1;
done:
 return 0;
}
PRIVATE int DCALL
nomemory_copy(DeeNoMemoryErrorObject *__restrict self,
              DeeNoMemoryErrorObject *__restrict other) {
 self->nm_allocsize = other->nm_allocsize;
 return error_copy((DeeErrorObject *)self,(DeeErrorObject *)other);
}
PRIVATE int DCALL
nomemory_deep(DeeNoMemoryErrorObject *__restrict self,
              DeeNoMemoryErrorObject *__restrict other) {
 self->nm_allocsize = other->nm_allocsize;
 return error_deep((DeeErrorObject *)self,(DeeErrorObject *)other);
}
PRIVATE DREF DeeObject *DCALL
nomemory_str(DeeNoMemoryErrorObject *__restrict self) {
 if (self->nm_allocsize)
     return DeeString_Newf("Failed to allocated %Iu bytes",
                           self->nm_allocsize);
 return error_str((DeeErrorObject *)self);
}
PUBLIC DeeTypeObject DeeError_NoMemory =
INIT_CUSTOM_ERROR("NoMemory",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,&nomemory_copy,&nomemory_deep,&nomemory_ctor,
                  DeeNoMemoryErrorObject,NULL,&nomemory_str,NULL,
                  NULL,NULL,nomemory_members,NULL);
/* END::NoMemory */




/* BEGIN::RuntimeError */
PRIVATE struct type_member runtimeerror_class_members[] = {
    TYPE_MEMBER_CONST("NotImplemented",&DeeError_NotImplemented),
    TYPE_MEMBER_CONST("AssertionError",&DeeError_AssertionError),
    TYPE_MEMBER_CONST("UnboundLocal",&DeeError_UnboundLocal),
    TYPE_MEMBER_CONST("StackOverflow",&DeeError_StackOverflow),
    TYPE_MEMBER_CONST("SegFault",&DeeError_SegFault),
    TYPE_MEMBER_CONST("IllegalInstruction",&DeeError_IllegalInstruction),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_RuntimeError =
INIT_CUSTOM_ERROR("RuntimeError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,runtimeerror_class_members);
PUBLIC DeeTypeObject DeeError_NotImplemented =
INIT_CUSTOM_ERROR("NotImplemented",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_RuntimeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_AssertionError =
INIT_CUSTOM_ERROR("AssertionError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_RuntimeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_UnboundLocal =
INIT_CUSTOM_ERROR("UnboundLocal",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_RuntimeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_StackOverflow =
INIT_CUSTOM_ERROR("StackOverflow",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_RuntimeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_SegFault =
INIT_CUSTOM_ERROR("SegFault",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_RuntimeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_IllegalInstruction =
INIT_CUSTOM_ERROR("IllegalInstruction",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_RuntimeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::RuntimeError */




PUBLIC DeeTypeObject DeeError_TypeError =
INIT_CUSTOM_ERROR("TypeError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);




/* BEGIN::ValueError */
PRIVATE struct type_member valueerror_class_members[] = {
    TYPE_MEMBER_CONST("Arithmetic",&DeeError_Arithmetic),
    TYPE_MEMBER_CONST("KeyError",&DeeError_KeyError),
    TYPE_MEMBER_CONST("IndexError",&DeeError_IndexError),
    TYPE_MEMBER_CONST("SequenceError",&DeeError_SequenceError),
    TYPE_MEMBER_CONST("UnicodeError",&DeeError_UnicodeError),
    TYPE_MEMBER_CONST("ReferenceError",&DeeError_ReferenceError),
    TYPE_MEMBER_CONST("UnpackError",&DeeError_UnpackError),
    TYPE_MEMBER_CONST("BufferError",&DeeError_BufferError),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_ValueError =
INIT_CUSTOM_ERROR("ValueError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,valueerror_class_members);
/* BEGIN::ValueError.Arithmetic */
PRIVATE struct type_member arithmetic_class_members[] = {
    TYPE_MEMBER_CONST("IntegerOverflow",&DeeError_IntegerOverflow),
    TYPE_MEMBER_CONST("DivideByZero",&DeeError_DivideByZero),
    TYPE_MEMBER_CONST("NegativeShift",&DeeError_NegativeShift),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_Arithmetic =
INIT_CUSTOM_ERROR("Arithmetic",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,arithmetic_class_members);
PUBLIC DeeTypeObject DeeError_IntegerOverflow =
INIT_CUSTOM_ERROR("IntegerOverflow",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Arithmetic,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_DivideByZero =
INIT_CUSTOM_ERROR("DivideByZero",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Arithmetic,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_NegativeShift =
INIT_CUSTOM_ERROR("NegativeShift",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Arithmetic,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::ValueError.Arithmetic */
PUBLIC DeeTypeObject DeeError_KeyError =
INIT_CUSTOM_ERROR("KeyError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* BEGIN::ValueError.IndexError */
PRIVATE struct type_member indexerror_class_members[] = {
    TYPE_MEMBER_CONST("UnboundItem",&DeeError_UnboundItem),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_IndexError =
INIT_CUSTOM_ERROR("IndexError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,indexerror_class_members);
PUBLIC DeeTypeObject DeeError_UnboundItem =
INIT_CUSTOM_ERROR("UnboundItem",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_IndexError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::ValueError.IndexError */
PUBLIC DeeTypeObject DeeError_SequenceError =
INIT_CUSTOM_ERROR("SequenceError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* BEGIN::ValueError.UnicodeError */
PRIVATE struct type_member unicodeerror_class_members[] = {
    TYPE_MEMBER_CONST("UnicodeDecodeError",&DeeError_UnicodeDecodeError),
    TYPE_MEMBER_CONST("UnicodeEncodeError",&DeeError_UnicodeEncodeError),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_UnicodeError =
INIT_CUSTOM_ERROR("UnicodeError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,unicodeerror_class_members);
PUBLIC DeeTypeObject DeeError_UnicodeDecodeError =
INIT_CUSTOM_ERROR("UnicodeDecodeError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_UnicodeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_UnicodeEncodeError =
INIT_CUSTOM_ERROR("UnicodeEncodeError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_UnicodeError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::ValueError.UnicodeError */
PUBLIC DeeTypeObject DeeError_ReferenceError =
INIT_CUSTOM_ERROR("ReferenceError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_UnpackError =
INIT_CUSTOM_ERROR("UnpackError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_BufferError =
INIT_CUSTOM_ERROR("BufferError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_ValueError,
                  NULL,NULL,NULL,NULL,DeeErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::ValueError */





/* BEGIN::SystemError */
PRIVATE struct type_member systemerror_class_members[] = {
    TYPE_MEMBER_CONST("UnsupportedAPI",&DeeError_UnsupportedAPI),
    TYPE_MEMBER_CONST("FSError",&DeeError_FSError),
    TYPE_MEMBER_CONST_DOC("IOError",&DeeError_FSError,"Deprecated alias for #FSError"),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_SystemError =
INIT_CUSTOM_ERROR("SystemError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Error,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,systemerror_class_members);
PUBLIC DeeTypeObject DeeError_UnsupportedAPI =
INIT_CUSTOM_ERROR("UnsupportedAPI",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_SystemError,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PRIVATE struct type_member fserror_class_members[] = {
    TYPE_MEMBER_CONST("AccessError",&DeeError_AccessError),
    TYPE_MEMBER_CONST("FileNotFound",&DeeError_FileNotFound),
    TYPE_MEMBER_CONST("FileExists",&DeeError_FileExists),
    TYPE_MEMBER_CONST("HandleClosed",&DeeError_HandleClosed),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_FSError =
INIT_CUSTOM_ERROR("FSError",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_SystemError,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,fserror_class_members);
PRIVATE struct type_member accesserror_members[] = {
    TYPE_MEMBER_CONST("ReadOnly",&DeeError_ReadOnly),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_AccessError =
INIT_CUSTOM_ERROR("AccessError","An error derived from :FSError that is thrown when attempting "
                                "to access a file or directory without the necessary permissions",
                  TP_FNORMAL|TP_FINHERITCTOR,&DeeError_FSError,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,accesserror_members);
PUBLIC DeeTypeObject DeeError_ReadOnly =
INIT_CUSTOM_ERROR("ReadOnly","An error derived from :AccessError that is thrown when attempting "
                             "to modify a file or directory when it or the filesystem is read-only",
                  TP_FNORMAL|TP_FINHERITCTOR,&DeeError_AccessError,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_FileNotFound =
INIT_CUSTOM_ERROR("FileNotFound",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_FSError,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_FileExists =
INIT_CUSTOM_ERROR("FileExists","An error derived from :FSError that is thrown when attempting "
                               "to create a filesystem object when the target path already exists",
                  TP_FNORMAL|TP_FINHERITCTOR,&DeeError_FSError,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
PUBLIC DeeTypeObject DeeError_HandleClosed =
INIT_CUSTOM_ERROR("HandleClosed",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_FSError,
                  NULL,NULL,NULL,NULL,DeeSystemErrorObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
/* END::SystemError */






PRIVATE int DCALL
appexit_init(struct appexit_object *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
#ifdef EXIT_FAILURE
 self->ae_exitcode = EXIT_FAILURE;
#else
 self->ae_exitcode = 1;
#endif
 /* Read the exitcode from arguments. */
 return DeeArg_Unpack(argc,argv,"|d:appexit",&self->ae_exitcode);
}

PRIVATE int DCALL
appexit_copy(struct appexit_object *__restrict self,
             struct appexit_object *__restrict other) {
 self->ae_exitcode = other->ae_exitcode;
 return 0;
}

PRIVATE DREF DeeObject *DCALL
appexit_str(struct appexit_object *__restrict self) {
 return DeeString_Newf("exit:%d",self->ae_exitcode);
}

PRIVATE DREF DeeObject *DCALL
appexit_repr(struct appexit_object *__restrict self) {
 return DeeString_Newf("AppExit(%d)",self->ae_exitcode);
}

PRIVATE struct type_member appexit_members[] = {
    TYPE_MEMBER_FIELD("exitcode",STRUCT_CONST|STRUCT_INT,offsetof(struct appexit_object,ae_exitcode)),
    TYPE_MEMBER_END
};


PRIVATE DREF DeeObject *DCALL
appexit_class_atexit(DeeObject *__restrict UNUSED(self),
                     size_t argc, DeeObject **__restrict argv) {
 DeeObject *callback,*args = Dee_EmptyTuple;
 if (DeeArg_Unpack(argc,argv,"o|o:atexit",&callback,&args) ||
     DeeObject_AssertTypeExact(args,&DeeTuple_Type) ||
     Dee_AtExit(callback,args))
     return NULL;
 return_none;
}

PUBLIC int DCALL Dee_Exit(int exitcode, bool run_atexit) {
#if !defined(CONFIG_NO_STDLIB) && 1
 if (run_atexit)
     exit(exitcode);
#if !defined(CONFIG_NO__Exit) && \
    (defined(CONFIG_HAVE__Exit) || \
     defined(_Exit) || defined(__USE_ISOC99))
 _Exit(exitcode);
#elif !defined(CONFIG_NO__exit) && \
      (defined(_MSC_VER) || defined(CONFIG_HAVE__exit) || defined(_exit))
 _exit(exitcode);
#else
 /* Fallback: Discard all registered callbacks and use the regular exit() */
 Dee_RunAtExit(DEE_RUNATEXIT_FDONTRUN);
 exit(exitcode);
#endif
 for (;;) {}
#else
 /* If callbacks aren't supposed to be executed, discard
  * all of them and prevent the addition of new ones. */
 if (!run_atexit)
      Dee_RunAtExit(DEE_RUNATEXIT_FDONTRUN);
 /* No stdlib support. Instead, we must throw an AppExit error. */
 {
  struct appexit_object *error;
  error = DeeObject_MALLOC(struct appexit_object);
  if unlikely(!error) goto err;
  /* Initialize the appexit error. */
  error->ae_exitcode = exitcode;
  DeeObject_Init(error,&DeeError_AppExit);
  /* Throw the appexit error. */
  DeeError_Throw((DeeObject *)error);
  Dee_Decref(error);
err:
  return -1;
 }
#endif
}

PRIVATE DREF DeeObject *DCALL
appexit_class_exit(DeeObject *__restrict UNUSED(self),
                   size_t argc, DeeObject **__restrict argv) {
 int exitcode = EXIT_FAILURE; bool run_atexit = true;
 if (DeeArg_Unpack(argc,argv,"|db:exit",&exitcode,&run_atexit))
     goto err;
 Dee_Exit(exitcode,run_atexit);
err:
 return NULL;
}

PRIVATE struct type_method appexit_class_methods[] = {
    { "exit", &appexit_class_exit,
      DOC("()\n"
          "(exitcode:?Dint,invoke_atexit=!t)\n"
          "Terminate execution of deemon after invoking #atexit callbacks when @invoke_atexit is :true\n"
          "Termination is done using the C $exit or $_exit functions, if available. However if these "
          "functions are not provided by the host, an :AppExit error is thrown instead\n"
          "When no @exitcode is given, the host's default default value of EXIT_FAILURE, or $1 is used\n"
          "This function never returns normally") },
    { "atexit", &appexit_class_atexit,
      DOC("(callback:?Dcallable,args=!T0)\n"
          "@throw RuntimeError Additional atexit-callbacks can no longer be registered\n"
          "@throw NotImplemented Deemon was built without support for #atexit\n"
          "Register a given @callback to be executed before deemon terminates") },
    { NULL }
};

PUBLIC DeeTypeObject DeeError_AppExit = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"AppExit",
    /* .tp_doc      = */DOC("An AppExit object is a special kind of interrupt that "
                            "is not derived from :object, and has no base class at "
                            "all. It's purpose is to allow user-code to throw an "
                            "instance of it and have the stack unwind itself, alongside "
                            "all existing objects being destroyed normally before deemon "
                            "will be terminated with the given exitcode\n"
                            "\n"
                            "()\n"
                            "(exitcode:?Dint)\n"
                            "Construct a new AppExit object using the given @exitcode "
                            "or the host's default value for EXIT_FAILURE, or $1"),
    /* .tp_flags    = */TP_FFINAL|TP_FINTERRUPT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */NULL, /* No base type (to only allow AppExit-interrupt handlers,
                               * and all-interrupt handlers to catch this error) */
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */&appexit_copy,
                /* .tp_deep_ctor = */&appexit_copy,
                /* .tp_any_ctor  = */&appexit_init,
                TYPE_FIXED_ALLOCATOR(struct appexit_object)
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&appexit_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&appexit_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */appexit_members,
    /* .tp_class_methods = */appexit_class_methods,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};






/* Fallback instance when throwing no-memory errors with an unknown size. */
PUBLIC DeeNoMemoryErrorObject DeeError_NoMemory_instance = {
    OBJECT_HEAD_INIT(&DeeError_NoMemory),
    /* .e_message    = */(DREF DeeStringObject *)&str_nomemory,
    /* .e_inner      = */NULL,
    /* .nm_allocsize = */(size_t)-1
};








/* ==== Signal type subsystem ==== */
PUBLIC DeeTypeObject DeeError_StopIteration =
INIT_CUSTOM_ERROR("StopIteration",NULL,TP_FNORMAL|TP_FINHERITCTOR,&DeeError_Signal,
                  NULL,NULL,NULL,NULL,DeeSignalObject,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
PRIVATE struct type_member interrupt_class_members[] = {
    TYPE_MEMBER_CONST("KeyboardInterrupt",&DeeError_KeyboardInterrupt),
    TYPE_MEMBER_CONST("ThreadExit",&DeeError_ThreadExit),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_Interrupt =
INIT_CUSTOM_ERROR("Interrupt",NULL,TP_FNORMAL|TP_FINHERITCTOR|TP_FINTERRUPT /* Interrupt type! */,&DeeError_Signal,
                  NULL,NULL,NULL,NULL,DeeSignalObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,interrupt_class_members);
PRIVATE int DCALL
threadexit_init(struct threadexit_object *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 self->te_result = Dee_None;
 if (DeeArg_Unpack(argc,argv,"|o",&self->te_result))
     return -1;
 Dee_Incref(self->te_result);
 return 0;
}
PRIVATE void DCALL
threadexit_fini(struct threadexit_object *__restrict self) {
 Dee_Decref(self->te_result);
}
PRIVATE void DCALL
threadexit_visit(struct threadexit_object *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->te_result);
}
PRIVATE struct type_member threadexit_members[] = {
    TYPE_MEMBER_FIELD("__result__",STRUCT_OBJECT,offsetof(struct threadexit_object,te_result)),
    TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_ThreadExit =
INIT_CUSTOM_ERROR("ThreadExit",NULL,TP_FNORMAL|TP_FINTERRUPT /* Interrupt type! */,&DeeError_Signal,
                  NULL,NULL,NULL,&threadexit_init,struct threadexit_object,
                  &threadexit_fini,NULL,&threadexit_visit,
                  NULL,NULL,threadexit_members,interrupt_class_members);
PUBLIC DeeTypeObject DeeError_KeyboardInterrupt =
INIT_CUSTOM_ERROR("KeyboardInterrupt",NULL,TP_FNORMAL|TP_FINHERITCTOR|TP_FINTERRUPT /* Interrupt type! */,&DeeError_Interrupt,
                  NULL,NULL,NULL,NULL,DeeSignalObject,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL);
INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));
PRIVATE struct type_member signal_class_members[] = {
    TYPE_MEMBER_CONST("Interrupt",&DeeError_Interrupt),
    TYPE_MEMBER_CONST("StopIteration",&DeeError_StopIteration),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
signal_repr(DeeSignalObject *__restrict self) {
 return DeeString_Newf("%k()",Dee_TYPE(self));
}

PUBLIC DeeTypeObject DeeError_Signal = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_Signal),
    /* .tp_doc      = */DOC("Base class for signaling exceptions\n"
                            "\n"
                            "()"),
    /* .tp_flags    = */TP_FNORMAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(void *)&none_i1,
                /* .tp_copy_ctor = */(void *)&none_i2,
                /* .tp_deep_ctor = */(void *)&none_i2,
                /* .tp_any_ctor  = */NULL,
                TYPE_FIXED_ALLOCATOR(DeeSignalObject)
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&signal_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */signal_class_members
};
PUBLIC DeeSignalObject DeeError_StopIteration_instance = {
    OBJECT_HEAD_INIT(&DeeError_StopIteration)
};
PUBLIC DeeSignalObject DeeError_Interrupt_instance = {
    OBJECT_HEAD_INIT(&DeeError_Interrupt)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */
