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
#ifndef GUARD_DEEMON_EXECUTE_FUNCTION_C
#define GUARD_DEEMON_EXECUTE_FUNCTION_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/bool.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/error.h>
#include <deemon/traceback.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/format.h>
#include <deemon/string.h>
#include <deemon/callable.h>
#include <deemon/code.h>
#include <deemon/gc.h>
#include <deemon/asm.h>
#include <deemon/thread.h>
#include <deemon/util/string.h>

#include <string.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeYieldFunctionIteratorObject YFIterator;
typedef DeeYieldFunctionObject         YFunction;



PUBLIC DREF DeeObject *DCALL
DeeFunction_New(DeeObject *__restrict code, size_t refc,
                DeeObject *const *__restrict refv) {
 DREF DeeFunctionObject *result; size_t i;
 ASSERT_OBJECT_TYPE_EXACT(code,&DeeCode_Type);
 ASSERT(((DeeCodeObject *)code)->co_refc == refc);
 result = (DREF DeeFunctionObject *)DeeObject_Malloc(offsetof(DeeFunctionObject,fo_refv)+
                                                    (refc*sizeof(DREF DeeObject *)));
 if unlikely(!result) goto done;
 result->fo_code = (DREF DeeCodeObject *)code;
 for (i = 0; i < refc; ++i) {
  DREF DeeObject *obj = refv[i];
  ASSERT_OBJECT(obj);
  result->fo_refv[i] = obj;
  Dee_Incref(obj);
 }
 Dee_Incref(code);
 DeeObject_Init(result,&DeeFunction_Type);
done:
 return (DREF DeeObject *)result;
}


INTERN DREF DeeObject *DCALL
DeeFunction_NewInherited(DeeObject *__restrict code, size_t refc,
                         DREF DeeObject *const *__restrict refv) {
 DREF DeeFunctionObject *result;
 ASSERT_OBJECT_TYPE_EXACT(code,&DeeCode_Type);
 ASSERTF(((DeeCodeObject *)code)->co_refc == refc,
         "((DeeCodeObject *)code)->co_refc = %I16u\n"
         "refc                             = %I16u\n"
         "name                             = %s\n",
         ((DeeCodeObject *)code)->co_refc,refc,
         DeeDDI_NAME(((DeeCodeObject *)code)->co_ddi));
 result = (DREF DeeFunctionObject *)DeeObject_Malloc(offsetof(DeeFunctionObject,fo_refv)+
                                                    (refc*sizeof(DREF DeeObject *)));
 if unlikely(!result) goto done;
 result->fo_code = (DREF DeeCodeObject *)code;
 MEMCPY_PTR(result->fo_refv,refv,refc);
 Dee_Incref(code);
 DeeObject_Init(result,&DeeFunction_Type);
done:
 return (DREF DeeObject *)result;
}
INTERN DREF DeeObject *DCALL
DeeFunction_NewNoRefs(DeeObject *__restrict code) {
 DREF DeeFunctionObject *result;
 ASSERT_OBJECT_TYPE_EXACT(code,&DeeCode_Type);
 ASSERT(((DeeCodeObject *)code)->co_refc == 0);
 result = (DREF DeeFunctionObject *)DeeObject_Malloc(offsetof(DeeFunctionObject,fo_refv));
 if unlikely(!result) goto done;
 result->fo_code = (DREF DeeCodeObject *)code;
 Dee_Incref(code);
 DeeObject_Init(result,&DeeFunction_Type);
done:
 return (DREF DeeObject *)result;
}


INTDEF DREF DeeObject *DCALL
function_call(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv);


PRIVATE DREF DeeFunctionObject *DCALL
function_init(size_t argc, DeeObject **__restrict argv) {
 DREF DeeFunctionObject *result;
 DeeCodeObject *code = &empty_code;
 DeeObject *refs = Dee_EmptyTuple;
 if (DeeArg_Unpack(argc,argv,"|oo:function",&code,&refs) ||
     DeeObject_AssertTypeExact((DeeObject *)code,&DeeCode_Type))
     goto err;
 result = (DREF DeeFunctionObject *)DeeObject_Malloc(offsetof(DeeFunctionObject,fo_refv)+
                                                    (code->co_refc*sizeof(DREF DeeObject *)));
 if unlikely(!result)
     goto err;
 if (DeeObject_Unpack(refs,code->co_refc,result->fo_refv))
     goto err_r;
 result->fo_code = code;
 Dee_Incref(code);
 DeeObject_Init(result,&DeeFunction_Type);
 return result;
err_r:
 DeeObject_Free(result);
err:
 return NULL;
}


PRIVATE struct type_member function_class_members[] = {
    TYPE_MEMBER_CONST("yieldfunction",&DeeYieldFunction_Type),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
function_getrefs(DeeFunctionObject *__restrict self) {
 return DeeRefVector_NewReadonly((DeeObject *)self,
                                  self->fo_code->co_refc,
                                  self->fo_refv);
}

PRIVATE struct type_getset function_getsets[] = {
    { "__refs__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&function_getrefs, NULL, NULL,
      DOC("->sequence") },
    { NULL }
};
PRIVATE struct type_member function_members[] = {
    TYPE_MEMBER_FIELD("__code__",STRUCT_OBJECT,offsetof(DeeFunctionObject,fo_code)),
    TYPE_MEMBER_END
};


PRIVATE void DCALL
function_fini(DeeFunctionObject *__restrict self) {
 size_t i;
 for (i = 0; i < self->fo_code->co_refc; ++i)
     Dee_Decref(self->fo_refv[i]);
 Dee_Decref(self->fo_code);
}

PRIVATE void DCALL
function_visit(DeeFunctionObject *__restrict self,
               dvisit_t proc, void *arg) {
 size_t i;
 for (i = 0; i < self->fo_code->co_refc; ++i)
     Dee_Visit(self->fo_refv[i]);
 Dee_Visit(self->fo_code);
}

PRIVATE DREF DeeObject *DCALL
function_str(DeeFunctionObject *__restrict self) {
 DeeDDIObject *ddi = self->fo_code->co_ddi;
 if (DeeDDI_HAS_NAME(ddi))
     return DeeString_New(DeeDDI_NAME(ddi));
 return_reference_(&str_function);
}
PRIVATE DREF DeeObject *DCALL
function_repr(DeeFunctionObject *__restrict self) {
#if 1
 uint16_t i;
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
#if 1
 if (UNICODE_PRINTER_PRINT(&printer,"function(code") < 0) goto err;
#else
 if (UNICODE_PRINTER_PRINT(&printer,"function(") < 0) goto err;
 if (unicode_printer_printobjectrepr(&printer,(DeeObject *)self->fo_code) < 0) goto err;
#endif
 if (self->fo_code->co_refc) {
  if (UNICODE_PRINTER_PRINT(&printer,",{ ") < 0) goto err;
  for (i = 0; i < self->fo_code->co_refc; ++i) {
   if (i != 0 && UNICODE_PRINTER_PRINT(&printer,", ") < 0) goto err;
   if (unicode_printer_printobjectrepr(&printer,self->fo_refv[i]) < 0) goto err;
  }
  if (UNICODE_PRINTER_PRINT(&printer," }")) goto err;
 }
 if (unicode_printer_putascii(&printer,')')) goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
#else
 uint16_t i,argc;
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 DeeCodeObject *code = self->fo_code;
 DeeDDIObject *ddi = code->co_ddi;
 char const *name = DeeDDI_NAME(ddi);
 /* Include attribute tags in the string representation. */
 if (code->co_flags&CODE_FCOPYABLE && UNICODE_PRINTER_PRINT(&printer,"@copyable ") < 0) goto err;
 if (code->co_flags&CODE_FASSEMBLY && UNICODE_PRINTER_PRINT(&printer,"@assembly ") < 0) goto err;
 if (code->co_flags&CODE_FLENIENT && UNICODE_PRINTER_PRINT(&printer,"@lenient ") < 0) goto err;
 if (code->co_flags&CODE_FTHISCALL && UNICODE_PRINTER_PRINT(&printer,"@thiscall ") < 0) goto err;
 if (code->co_flags&CODE_FHEAPFRAME && UNICODE_PRINTER_PRINT(&printer,"@heapframe ") < 0) goto err;
 if (code->co_flags&CODE_FCONSTRUCTOR && UNICODE_PRINTER_PRINT(&printer,"@constructor ") < 0) goto err;
 if (UNICODE_PRINTER_PRINT(&printer,"function") < 0) goto err;
 if (*name && unicode_printer_printf(&printer," %s",name) < 0) goto err;
 if (UNICODE_PRINTER_PRINT(&printer,"(") < 0) goto err;
 argc = code->co_argc_max;
 for (i = 0; i < argc; ++i) {
  if (i != 0 && UNICODE_PRINTER_PRINT(&printer,", ") < 0) goto err;
  if (DeeDDI_HAS_ARG(ddi,i)) {
   if (unicode_printer_printf(&printer,"%s",DeeDDI_ARG_NAME(ddi,i)) < 0)
       goto err;
  } else {
   if (unicode_printer_printf(&printer,"arg%I16u",i) < 0)
       goto err;
  }
  if (i >= code->co_argc_min) {
   if (unicode_printer_printf(&printer," = %r",
                              code->co_defaultv[i-code->co_argc_min]) < 0)
       goto err;
  }
 }
 if (code->co_flags&CODE_FVARARGS) {
  if (argc != 0 && UNICODE_PRINTER_PRINT(&printer,", ") < 0) goto err;
  if (DeeDDI_HAS_ARG(ddi,argc) &&
      unicode_printer_printf(&printer,"%s",DeeDDI_ARG_NAME(ddi,argc)) < 0)
      goto err;
  if (UNICODE_PRINTER_PRINT(&printer,"...") < 0) goto err;
 }
 if (UNICODE_PRINTER_PRINT(&printer,")") < 0) goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
#endif
}

PRIVATE dhash_t DCALL
function_hash(DeeFunctionObject *__restrict self) {
 uint16_t i; dhash_t result;
 result = DeeObject_Hash((DeeObject *)self->fo_code);
 for (i = 0; i < self->fo_code->co_refc; ++i)
     result ^= DeeObject_Hash(self->fo_refv[i]);
 return result;
}

PRIVATE DREF DeeObject *DCALL
function_eq(DeeFunctionObject *__restrict self,
            DeeFunctionObject *__restrict other) {
 uint16_t i; int result;
 if (DeeObject_AssertTypeExact(other,&DeeFunction_Type))
     goto err;
 result = DeeObject_CompareEq((DeeObject *)self->fo_code,
                              (DeeObject *)other->fo_code);
 if unlikely(result <= 0) goto err_or_false;
 ASSERT(self->fo_code->co_refc == other->fo_code->co_refc);
 for (i = 0; i < self->fo_code->co_refc; ++i) {
  result = DeeObject_CompareEq(self->fo_refv[i],
                               other->fo_refv[i]);
  if (result <= 0) goto err_or_false;
 }
 return_true;
err_or_false:
 if (!result)
     return_false;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
function_ne(DeeFunctionObject *__restrict self,
            DeeFunctionObject *__restrict other) {
 uint16_t i; int result;
 if (DeeObject_AssertTypeExact(other,&DeeFunction_Type))
     goto err;
 result = DeeObject_CompareNe((DeeObject *)self->fo_code,
                              (DeeObject *)other->fo_code);
 if unlikely(result != 0) goto err_or_true;
 ASSERT(self->fo_code->co_refc == other->fo_code->co_refc);
 for (i = 0; i < self->fo_code->co_refc; ++i) {
  result = DeeObject_CompareNe(self->fo_refv[i],
                               other->fo_refv[i]);
  if (result != 0) goto err_or_true;
 }
 return_false;
err_or_true:
 if (result > 0)
     return_true;
err:
 return NULL;
}

PRIVATE struct type_cmp function_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&function_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&function_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&function_ne
};


PUBLIC DeeTypeObject DeeFunction_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_function),
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL|TP_FNAMEOBJECT|TP_FVARIABLE,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeCallable_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&function_init,
                /* .tp_free      = */NULL, /* XXX: Use the tuple-allocator? */
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&function_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&function_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&function_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */&function_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&function_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&function_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */function_getsets,
    /* .tp_members       = */function_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */function_class_members
};

PRIVATE void DCALL
yf_fini(YFunction *__restrict self) {
 Dee_Decref(self->yf_func);
 Dee_Decref(self->yf_args);
 Dee_XDecref(self->yf_this);
}
PRIVATE void DCALL
yf_visit(YFunction *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->yf_func);
 Dee_Visit(self->yf_args);
 Dee_XVisit(self->yf_this);
}

PRIVATE int DCALL
yf_ctor(YFunction *__restrict self) {
 self->yf_func = function_init(0,NULL);
 if unlikely(!self->yf_func) goto err;
 self->yf_args = (DREF DeeTupleObject *)Dee_EmptyTuple;
 Dee_Incref(Dee_EmptyTuple);
 self->yf_this = NULL;
 return 0;
err:
 return -1;
}


PRIVATE int DCALL
yf_copy(YFunction *__restrict self,
        YFunction *__restrict other) {
 self->yf_func = other->yf_func;
 self->yf_args = other->yf_args;
 self->yf_this = other->yf_this;
 Dee_Incref(self->yf_func);
 Dee_Incref(self->yf_args);
 Dee_XIncref(self->yf_this);
 return 0;
}

PRIVATE int DCALL
yf_deepcopy(YFunction *__restrict self,
            YFunction *__restrict other) {
 self->yf_args = (DREF DeeTupleObject *)DeeObject_DeepCopy((DeeObject *)other->yf_args);
 if unlikely(!self->yf_args) goto err;
 self->yf_this = NULL;
 if (other->yf_this) {
  self->yf_this = DeeObject_DeepCopy(other->yf_this);
  if unlikely(!self->yf_this) goto err_args;
 }
 self->yf_func = other->yf_func;
 Dee_Incref(self->yf_func);
 return 0;
err_args:
 Dee_Decref(self->yf_args);
err:
 return -1;
}

PRIVATE int DCALL
yf_new(YFunction *__restrict self,
       size_t argc, DeeObject **__restrict argv) {
 DeeObject *func = NULL,*args = Dee_EmptyTuple,*this_ = NULL;
 if (DeeArg_Unpack(argc,argv,"|ooo:yieldfunction",&func,&args,&this_))
     goto err;
 /* The actually available overloads are:
  *   this();
  *   this(function func);
  *   this(function func, tuple args);
  *   this(function func, object this, tuple args); */
 if (this_) { DeeObject *temp = args; args = this_; this_ = temp; }
 if (DeeObject_AssertTypeExact(args,&DeeTuple_Type))
     goto err;
 if (func) {
  if (DeeObject_AssertTypeExact(func,&DeeFunction_Type))
      goto err;
  Dee_Incref(func);
 } else {
  func = (DeeObject *)function_init(0,NULL);
  if unlikely(!func) goto err;
 }
 if ((this_ != NULL) != (((DeeFunctionObject *)func)->fo_code->co_flags&CODE_FTHISCALL)) {
  Dee_Decref(func);
  DeeError_Throwf(&DeeError_TypeError,
                  "Invalid presence of this-argument");
  goto err;
 }
 self->yf_func = (DREF DeeFunctionObject *)func; /* Inherit reference. */
 self->yf_args = (DREF DeeTupleObject *)args;
 self->yf_this = this_;
 Dee_Incref(self->yf_args);
 Dee_XIncref(self->yf_this);
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
yfi_init(YFIterator *__restrict self,
         YFunction *__restrict yield_function) {
 DeeCodeObject *code;
#ifndef NDEBUG
 memset(&self->yi_frame,0xcc,sizeof(struct code_frame));
#endif
 /* Setup the frame for the iterator. */
 self->yi_func          = yield_function;
 self->yi_frame.cf_func = yield_function->yf_func;
 Dee_Incref(yield_function); /* Reference stored in `self->yi_func' */
 Dee_Incref(self->yi_frame.cf_func); /* Reference stored in here. */
 code = self->yi_frame.cf_func->fo_code;
 /* Allocate memory for frame data. */
 self->yi_frame.cf_prev  = CODE_FRAME_NOT_EXECUTING;
 self->yi_frame.cf_frame = (DREF DeeObject **)Dee_Calloc(code->co_framesize);
 if unlikely(!self->yi_frame.cf_frame) goto err;
 self->yi_frame.cf_stack = self->yi_frame.cf_frame+code->co_localc;
 self->yi_frame.cf_sp    = self->yi_frame.cf_stack;
 self->yi_frame.cf_ip    = code->co_code;
 self->yi_frame.cf_flags = code->co_flags;
 self->yi_frame.cf_vargs = NULL;
 self->yi_frame.cf_argc  = DeeTuple_SIZE(yield_function->yf_args);
 self->yi_frame.cf_argv  = DeeTuple_ELEM(yield_function->yf_args);
 self->yi_frame.cf_this  = yield_function->yf_this;
 Dee_XIncref(self->yi_frame.cf_this);
 self->yi_frame.cf_stacksz = 0;
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_init(&self->yi_lock);
#endif
 return 0;
err:
 return -1;
}

PRIVATE DREF YFIterator *DCALL
yf_iter_self(YFunction *__restrict self) {
 DREF YFIterator *result;
 result = DeeGCObject_MALLOC(YFIterator);
 if unlikely(!result) goto err;
 if unlikely(yfi_init(result,self)) goto err_r;
 DeeObject_Init(result,&DeeYieldFunctionIterator_Type);
 DeeGC_Track((DeeObject *)result);
 return result;
err_r:
 DeeGCObject_Free(result);
err:
 return NULL;
}

PRIVATE struct type_seq yf_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yf_iter_self
};

PRIVATE struct type_member yf_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeYieldFunctionIterator_Type),
    TYPE_MEMBER_END
};

/* Since yieldfunction objects are bound to a specific function, comparing
 * them won't compare the bound function, but rather that function's pointer! */
PRIVATE dhash_t DCALL
yf_hash(DeeYieldFunctionObject *__restrict self) {
 dhash_t result;
 result  = DeeObject_HashGeneric(self->yf_func);
 result ^= DeeObject_Hash((DeeObject *)self->yf_args);
 if (self->yf_this)
     result ^= DeeObject_Hash((DeeObject *)self->yf_this);
 return result;
}

PRIVATE int DCALL
yf_eq_impl(DeeYieldFunctionObject *__restrict self,
           DeeYieldFunctionObject *__restrict other) {
 int error;
 if (DeeObject_AssertTypeExact(other,&DeeYieldFunction_Type))
     goto err;
 if (self == other) goto yes;
 if (self->yf_func != other->yf_func) goto nope;
 error = DeeObject_CompareEq((DeeObject *)self->yf_args,
                             (DeeObject *)other->yf_args);
 if unlikely(error <= 0) goto do_return_error;
 ASSERTF((self->yf_this != NULL) == (other->yf_this != NULL),
         "If the functions are identical, they must also have "
         "identical requirements for the presence of a this-argument!");
 if (self->yf_this)
     return DeeObject_CompareEq(self->yf_this,other->yf_this);
yes:
 return 1;
do_return_error:
 return error;
nope:
 return 0;
err:
 return -1;
}

PRIVATE DREF DeeObject *DCALL
yf_eq(DeeYieldFunctionObject *__restrict self,
      DeeYieldFunctionObject *__restrict other) {
 int result = yf_eq_impl(self,other);
 if unlikely(result < 0) return NULL;
 return_bool_(result);
}
PRIVATE DREF DeeObject *DCALL
yf_ne(DeeYieldFunctionObject *__restrict self,
      DeeYieldFunctionObject *__restrict other) {
 int result = yf_eq_impl(self,other);
 if unlikely(result < 0) return NULL;
 return_bool_(!result);
}

PRIVATE struct type_cmp yf_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&yf_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&yf_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&yf_ne
};


PRIVATE struct type_member yf_members[] = {
    TYPE_MEMBER_FIELD_DOC("__func__",STRUCT_OBJECT,offsetof(YFunction,yf_func),"->function"),
    TYPE_MEMBER_FIELD_DOC("__args__",STRUCT_OBJECT,offsetof(YFunction,yf_args),"->sequence"),
    TYPE_MEMBER_FIELD("__this__",STRUCT_OBJECT,offsetof(YFunction,yf_this)),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeCodeObject *DCALL
yf_getcode(YFunction *__restrict self) {
 return_reference_(self->yf_func->fo_code);
}
PRIVATE DREF DeeObject *DCALL
yf_getrefs(YFunction *__restrict self) {
 return function_getrefs(self->yf_func);
}

PRIVATE struct type_getset yf_getsets[] = {
    { "__code__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yf_getcode, NULL, NULL,
      DOC("->code") },
    { "__refs__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yf_getrefs, NULL, NULL,
      DOC("->sequence") },
    { NULL }
};


PUBLIC DeeTypeObject DeeYieldFunction_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"yieldfunction",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL | TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&yf_ctor,
                /* .tp_copy_ctor = */&yf_copy,
                /* .tp_deep_ctor = */&yf_deepcopy,
                /* .tp_any_ctor  = */&yf_new,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(YFunction)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&yf_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL,
        /* .tp_deepload    = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&yf_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&yf_cmp,
    /* .tp_seq           = */&yf_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */yf_getsets,
    /* .tp_members       = */yf_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */yf_class_members
};

PRIVATE void DCALL
yfi_run_finally(YFIterator *__restrict self) {
 DeeCodeObject *code; code_addr_t ipaddr;
 struct except_handler *iter,*begin;
 if unlikely(!self->yi_func) return;
 /* Recursively execute all finally-handlers that
  * protect the current IP until none are left. */
 code = self->yi_frame.cf_func->fo_code;
 if unlikely(!code) return;
 ASSERT_OBJECT_TYPE(code,&DeeCode_Type);
 /* Simple case: without any finally handlers, we've got nothing to do. */
 if (!(code->co_flags&CODE_FFINALLY)) return;
exec_finally:
 iter = (begin = code->co_exceptv)+code->co_exceptc;
 /* NOTE: The frame-IP is allowed to equal the end of the
  *       associated code object, because it contains the
  *       address of the next instruction to-be executed.
  *       Similarly, range checks of handlers are adjusted, too.
  */
 ASSERT(self->yi_frame.cf_ip >= code->co_code &&
        self->yi_frame.cf_ip <= code->co_code+code->co_codebytes);
 ipaddr = (code_addr_t)(self->yi_frame.cf_ip - code->co_code);
 while (iter-- != begin) {
  DREF DeeObject *result;
  if (!(iter->eh_flags&EXCEPTION_HANDLER_FFINALLY)) continue;
  if (!(ipaddr > iter->eh_start && ipaddr <= iter->eh_end)) continue;
  /* Execute this finally-handler. */
  self->yi_frame.cf_ip = code->co_code+iter->eh_addr;
  /* We must somehow indicate to code-exec to stop when an
   * `ASM_ENDFINALLY' instruction is hit.
   * Normally, this is done when the return value has been
   * assigned, so we simply fake that by pre-assigning `none'. */
  self->yi_frame.cf_result = Dee_None;
  Dee_Incref(Dee_None);
  if unlikely(self->yi_frame.cf_flags&CODE_FASSEMBLY) {
   /* Special case: Execute the code using the safe runtime, rather than the fast. */
   result = DeeCode_ExecFrameSafe(&self->yi_frame);
  } else {
   /* Default case: Execute from a fast yield-function-iterator frame. */
   result = DeeCode_ExecFrameFast(&self->yi_frame);
  }
  if likely(result)
   Dee_Decref(result); /* Most likely, this is `none' */
  else {
   DeeError_Print("Unhandled exception in yieldfunction.iterator destructor\n",
                  ERROR_PRINT_DOHANDLE);
  }
  goto exec_finally;
 }

}

PRIVATE void DCALL
yfi_dtor(YFIterator *__restrict self) {
 size_t stacksize; size_t numlocals = 0;
 /* Execute established finally handlers. */
 yfi_run_finally(self);
 ASSERT(self->yi_frame.cf_prev == CODE_FRAME_NOT_EXECUTING);
 ASSERT_OBJECT_TYPE_OPT(self->yi_func,&DeeYieldFunction_Type);
 ASSERT_OBJECT_TYPE_OPT(self->yi_frame.cf_func,&DeeFunction_Type);
 ASSERT_OBJECT_TYPE_OPT(self->yi_frame.cf_vargs,&DeeTuple_Type);
 ASSERT_OBJECT_OPT(self->yi_frame.cf_this);
 if (self->yi_frame.cf_func)
     numlocals = self->yi_frame.cf_func->fo_code->co_localc;
 stacksize = self->yi_frame.cf_sp-self->yi_frame.cf_stack;
 Dee_XDecref(self->yi_func);
 Dee_XDecref(self->yi_frame.cf_func);
 Dee_XDecref(self->yi_frame.cf_this);
 Dee_XDecref(self->yi_frame.cf_vargs);
 /* Clear local objects. */
 while (numlocals--) Dee_XDecref(self->yi_frame.cf_frame[numlocals]);
 /* Clear objects from the stack. */
 while (stacksize--) Dee_Decref(self->yi_frame.cf_stack[stacksize]);
 if (self->yi_frame.cf_stacksz)
     Dee_Free(self->yi_frame.cf_stack);
 Dee_Free(self->yi_frame.cf_frame);
}


PRIVATE void DCALL
yfi_visit(YFIterator *__restrict self,
          dvisit_t proc, void *arg) {
 if (self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING)
     return; /* Can't visit a frame that is current executing. */
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_write(&self->yi_lock);
 COMPILER_READ_BARRIER();
 if (self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING) {
  recursive_rwlock_endwrite(&self->yi_lock);
  return; /* See above... */
 }
#endif
 Dee_XVisit(self->yi_func);
 Dee_XVisit(self->yi_frame.cf_this);
 Dee_XVisit(self->yi_frame.cf_vargs);
 /* Visit local variables. */
 if (self->yi_frame.cf_func) {
  DeeObject **locals = self->yi_frame.cf_frame;
  size_t numlocals = self->yi_frame.cf_func->fo_code->co_localc;
  Dee_Visit(self->yi_frame.cf_func);
  while (numlocals--) Dee_XVisit(locals[numlocals]);
 }
 /* Visit stack objects. */
 { size_t stacksize = (size_t)(self->yi_frame.cf_sp-
                               self->yi_frame.cf_stack);
   DeeObject **stack = self->yi_frame.cf_stack;
   while (stacksize--) Dee_Visit(stack[stacksize]);
 }

#ifndef CONFIG_NO_THREADS
 recursive_rwlock_endwrite(&self->yi_lock);
#endif
}

PRIVATE void DCALL
yfi_clear(YFIterator *__restrict self) {
 DeeObject *obj[4],**stack; size_t stacksize;
 DeeObject **locals; size_t numlocals = 0;
 bool heap_stack = false;
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_write(&self->yi_lock);
#endif
 /* Execute established finally handlers. */
 yfi_run_finally(self);
 if unlikely(self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING) {
  /* Can't clear a frame currently being executed. */
#ifndef CONFIG_NO_THREADS
  recursive_rwlock_endwrite(&self->yi_lock);
#endif
  return;
 }
 obj[0] = (DeeObject *)self->yi_func;
 obj[1] = (DeeObject *)self->yi_frame.cf_func;
 obj[2] = (DeeObject *)self->yi_frame.cf_this;
 obj[3] = (DeeObject *)self->yi_frame.cf_vargs;
 if (self->yi_frame.cf_func)
     numlocals = self->yi_frame.cf_func->fo_code->co_localc;
 stack     = self->yi_frame.cf_stack;
 stacksize = self->yi_frame.cf_sp-stack;
 if (self->yi_frame.cf_stacksz) {
  self->yi_frame.cf_stacksz = 0;
  heap_stack = true;
 }
 locals = self->yi_frame.cf_frame;
 self->yi_func           = NULL;
 self->yi_frame.cf_func  = NULL;
 self->yi_frame.cf_argc  = 0;
 self->yi_frame.cf_argv  = NULL;
 self->yi_frame.cf_this  = NULL;
 self->yi_frame.cf_vargs = NULL;
 self->yi_frame.cf_frame = NULL;
 self->yi_frame.cf_stack = NULL;
 self->yi_frame.cf_sp    = NULL;
 self->yi_frame.cf_ip    = NULL;
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_endwrite(&self->yi_lock);
#endif
 Dee_XDecref(obj[0]);
 Dee_XDecref(obj[1]);
 Dee_XDecref(obj[2]);
 Dee_XDecref(obj[3]);
 /* Clear local objects. */
 while (numlocals--) Dee_XDecref(locals[numlocals]);
 /* Clear objects from the stack. */
 while (stacksize--) Dee_Decref(stack[stacksize]);
 /* Free a heap-allocated stack, and local variable memory. */
 if (heap_stack) Dee_Free(stack);
 Dee_Free(locals);
}

PRIVATE DREF DeeObject *DCALL
yfi_iter_next(YFIterator *__restrict self) {
 DREF DeeObject *result;
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_write(&self->yi_lock);
#endif
 if unlikely(!self->yi_func) {
  /* Special case: Always be indicative of an exhausted iterator
   * when default-constructed, or after being cleared. */
  result = ITER_DONE;
 } else {
  ASSERT_OBJECT_TYPE(self->yi_func,&DeeYieldFunction_Type);
  ASSERT_OBJECT_TYPE(self->yi_frame.cf_func,&DeeFunction_Type);
  ASSERT_OBJECT_TYPE(self->yi_frame.cf_func->fo_code,&DeeCode_Type);
  ASSERTF(self->yi_frame.cf_func->fo_code->co_flags&CODE_FYIELDING,
          "Code is not assembled as a yield-function");
  ASSERTF(self->yi_frame.cf_ip >= self->yi_frame.cf_func->fo_code->co_code &&
          self->yi_frame.cf_ip <= self->yi_frame.cf_func->fo_code->co_code+
                                  self->yi_frame.cf_func->fo_code->co_codebytes,
          "Illegal IP: %p is not in %p...%p",
          self->yi_frame.cf_ip,
          self->yi_frame.cf_func->fo_code->co_code,
          self->yi_frame.cf_func->fo_code->co_code+
          self->yi_frame.cf_func->fo_code->co_codebytes);
  if unlikely(self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING) {
   DeeError_Throwf(&DeeError_SegFault,"Stack frame is already being executed");
   result = NULL;
   goto done;
  }
  self->yi_frame.cf_result = NULL;
  if unlikely(self->yi_frame.cf_flags&CODE_FASSEMBLY) {
   /* Special case: Execute the code using the safe runtime, rather than the fast. */
   result = DeeCode_ExecFrameSafe(&self->yi_frame);
  } else {
   /* Default case: Execute from a fast yield-function-iterator frame. */
   result = DeeCode_ExecFrameFast(&self->yi_frame);
  }
 }
done:
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_endwrite(&self->yi_lock);
#endif
 return result;
}

PRIVATE int DCALL
yfi_new(YFIterator *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
 YFunction *func;
 if (DeeArg_Unpack(argc,argv,"o:yieldfunction.iterator",&func))
     goto err;
 if (DeeObject_AssertType((DeeObject *)func,&DeeYieldFunction_Type))
     goto err;
 return yfi_init(self,func);
err:
 return -1;
}

LOCAL int DCALL
inplace_deepcopy_noarg(DREF DeeObject **__restrict pob,
                       size_t argc1, DREF DeeObject **__restrict argv1,
                       size_t args2_c, DeeObject **__restrict args2_v) {
 DREF DeeObject *ob = *pob,**iter,**end;
 /* Check if `*pob' is apart of the argument
  * tuple, and don't copy it if it is.
  * We take special care not to copy objects that were loaded
  * from arguments/references, as those are intended to be shared.
  * WARNING: This system isn't, and can never really be perfect.
  *          For example: should we copy an object loaded from
  *          the item of a list accessed through a reference/argument?
  *          The current implementation does, but the user
  *          may expect it not to do so.
  *          As far as logic goes, the sane thing would be to
  *          only copy objects when they are ever written to.
  *          But the again: how do we know when something will be written?
  *          Since everything can be dynamically altered, we have no
  *          way of predicting, or determining when _anything_ is going
  *          to change in the most dramatic way imaginable.
  *      ... Anyways. This is the best we can make out of a bad
  *          situation. - And luckily enough, the old deemon already
  *          knew that this could lead to troubles and established
  *          copyable stackframe as a whitelist-based system that
  *          a user must opt-in if they wish to use it.
  *          So in that sense: we've always got the user to blame if
  *                            they manage to break something or get
  *                            undefined behavior when using [[copyable]]!
  * NOTE: Don't get me wrong, through. I very much believe that copyable
  *       stack frames open up the door for _a_ _lot_ of awesome programming
  *       possibilities, while leaving any undefined behavior as pure-weak,
  *       in the sense that unless for some bug, the design is able to
  *       never crash no matter what the user might do.
  */
 end = (iter = argv1)+argc1;
 for (; iter != end; ++iter) if (*iter == ob) return 0;
 end = (iter = args2_v)+args2_c;
 for (; iter != end; ++iter) if (*iter == ob) return 0;
 /* Create an inplace deep-copy of this object. */
 return DeeObject_InplaceDeepCopy(pob);
}

PRIVATE int DCALL
yfi_copy(YFIterator *__restrict self,
         YFIterator *__restrict other) {
 DeeCodeObject *code; size_t stack_size;
 DREF DeeObject **iter,**end,**src;
again:
 recursive_rwlock_write(&other->yi_lock);
 /* Make sure that the function is actually copyable. */
 code = NULL;
 if (other->yi_frame.cf_func &&
  !((code = other->yi_frame.cf_func->fo_code)->co_flags&CODE_FCOPYABLE)) {
  DeeDDIObject *ddi = other->yi_frame.cf_func->fo_code->co_ddi;
  char *function_name = "?";
  if (ddi) {
   function_name = DeeDDI_NAME(ddi);
   Dee_Incref(ddi);
  }
  recursive_rwlock_endwrite(&other->yi_lock);
  DeeError_Throwf(&DeeError_ValueError,"Function `%s' is not copyable",function_name);
  Dee_XDecref(ddi);
  return -1;
 }
 self->yi_func = other->yi_func;
 /* Copy over frame data. */
 memcpy(&self->yi_frame,&other->yi_frame,sizeof(struct code_frame));
 /* In case the other frame is currently executing, mark ours as not. */
 self->yi_frame.cf_prev = CODE_FRAME_NOT_EXECUTING;
 if (code) {
  *(uintptr_t *)&self->yi_frame.cf_sp -= (uintptr_t)self->yi_frame.cf_stack;
  if (self->yi_frame.cf_stacksz) {
   /* Copy a heap-allocated, extended stack. */
   self->yi_frame.cf_stack = (DREF DeeObject **)Dee_TryMalloc(self->yi_frame.cf_stacksz*
                                                              sizeof(DREF DeeObject *));
   if unlikely(!self->yi_frame.cf_stack) goto nomem;
   src = other->yi_frame.cf_stack;
   end = (iter = self->yi_frame.cf_stack)+self->yi_frame.cf_stacksz;
   for (; iter != end; ++iter,++src) {
    DeeObject *ob = *src;
    Dee_Incref(ob);
    *iter = ob;
   }
  }
  self->yi_frame.cf_frame = (DREF DeeObject **)Dee_TryMalloc(code->co_framesize);
  if unlikely(!self->yi_frame.cf_frame) goto nomem_stack;
  /* Copy local variables. */
  src = other->yi_frame.cf_frame;
  end = (iter = self->yi_frame.cf_frame)+code->co_localc;
  for (; iter != end; ++iter,++src) {
   DeeObject *ob = *src;
   *iter = ob;
   Dee_Incref(ob);
  }
  if (!self->yi_frame.cf_stacksz) {
   /* Relocate + copy a frame-shared stack. */
   self->yi_frame.cf_stack = self->yi_frame.cf_frame+code->co_localc;
   *(uintptr_t *)&self->yi_frame.cf_sp += (uintptr_t)self->yi_frame.cf_stack;
   stack_size = (self->yi_frame.cf_sp-self->yi_frame.cf_stack);
   ASSERTF(stack_size*sizeof(DeeObject *) <=
          (code->co_framesize-code->co_localc*sizeof(DeeObject *)),
           "The stack is too large");
   /* Copy the stack. */
   src = other->yi_frame.cf_stack;
   end = (iter = self->yi_frame.cf_stack)+stack_size;
   for (; iter != end; ++iter,++src) {
    DeeObject *ob = *src;
    Dee_Incref(ob);
    *iter = ob;
   }
  } else {
   *(uintptr_t *)&self->yi_frame.cf_sp += (uintptr_t)self->yi_frame.cf_stack;
   stack_size = self->yi_frame.cf_stacksz;
  }
 } else {
  self->yi_frame.cf_frame   = NULL;
  self->yi_frame.cf_stack   = NULL;
  self->yi_frame.cf_sp      = NULL;
  self->yi_frame.cf_stacksz = 0;
  stack_size                = 0;
 }

 /* Create references. */
 Dee_XIncref(self->yi_func);
 Dee_XIncref(self->yi_frame.cf_func);
 Dee_XIncref(self->yi_frame.cf_this);
 Dee_XIncref(self->yi_frame.cf_vargs);

 recursive_rwlock_endwrite(&other->yi_lock);
 recursive_rwlock_init(&self->yi_lock);
 if (code) {
  DeeObject *this_arg = self->yi_frame.cf_this;
  DeeObject *varargs = (DeeObject *)self->yi_frame.cf_vargs;
  size_t      argc = self->yi_frame.cf_argc;
  DeeObject **argv = self->yi_frame.cf_argv;
  size_t      refc = self->yi_func->yf_func->fo_code->co_refc;
  DeeObject **refv = self->yi_func->yf_func->fo_refv;
  /* With all objects now referenced, we still have to replace
   * all locals and the stack with deep copies of themself. */
  end = (iter = self->yi_frame.cf_stack)+stack_size;
  for (; iter != end; ++iter) {
   if (*iter != this_arg && *iter != varargs &&
       inplace_deepcopy_noarg(iter,argc,argv,refc,refv)) goto err;
  }
  end = (iter = self->yi_frame.cf_frame)+code->co_localc;
  for (; iter != end; ++iter) {
   if (*iter != this_arg && *iter != varargs &&
       inplace_deepcopy_noarg(iter,argc,argv,refc,refv)) goto err;
  }
  /* WARNING: There are some thing that we don't copy, such as the this-argument.
   *          Similarly, we also don't copy function input arguments! */
 } else {
  ASSERT(!stack_size);
 }
 return 0;
err:
 yfi_dtor(self);
 return -1;
nomem_stack:
 if (self->yi_frame.cf_stacksz) {
  uint16_t n = self->yi_frame.cf_stacksz;
  while (n--) Dee_Decref(self->yi_frame.cf_stack[n]);
  Dee_Free(self->yi_frame.cf_stack);
 }
nomem:
 recursive_rwlock_endwrite(&other->yi_lock);
 if (Dee_CollectMemory(1))
     goto again;
 return -1;
}

#ifndef CONFIG_NO_THREADS
PRIVATE DREF YFunction *DCALL
yfi_getyfunc(YFIterator *__restrict self) {
 DREF YFunction *result;
 recursive_rwlock_read(&self->yi_lock);
 result = self->yi_func;
 Dee_XIncref(result);
 recursive_rwlock_endread(&self->yi_lock);
 if unlikely(!result)
    err_unbound_attribute(&DeeYieldFunctionIterator_Type,"seq");
 return result;
}
#endif /* !CONFIG_NO_THREADS */

PRIVATE DREF DeeObject *DCALL
yfi_getthis(YFIterator *__restrict self) {
 DREF DeeObject *result;
 recursive_rwlock_read(&self->yi_lock);
 result = self->yi_frame.cf_this;
 if (!(self->yi_frame.cf_flags&CODE_FTHISCALL))
       result = NULL;
 Dee_XIncref(result);
 recursive_rwlock_endread(&self->yi_lock);
 if unlikely(!result)
    err_unbound_attribute(&DeeYieldFunctionIterator_Type,"__this__");
 return result;
}

PRIVATE DREF DeeObject *DCALL
yfi_getframe(YFIterator *__restrict self) {
 return DeeFrame_NewReferenceWithLock((DeeObject *)self,
                                      &self->yi_frame,
                                       DEEFRAME_FREADONLY|
                                       DEEFRAME_FUNDEFSP|
                                       DEEFRAME_FRECLOCK,
                                      &self->yi_lock);
}

PRIVATE DREF DeeFunctionObject *DCALL
yfi_getfunc(YFIterator *__restrict self) {
 DREF DeeFunctionObject *result;
 recursive_rwlock_write(&self->yi_lock);
 if unlikely(!self->yi_func) {
  recursive_rwlock_endwrite(&self->yi_lock);
  err_unbound_attribute(&DeeYieldFunctionIterator_Type,"__func__");
  return NULL;
 }
 result = self->yi_func->yf_func;
 Dee_Incref(result);
 recursive_rwlock_endwrite(&self->yi_lock);
 return result;
}

PRIVATE DREF DeeCodeObject *DCALL
yfi_getcode(YFIterator *__restrict self) {
 DREF DeeCodeObject *result;
 recursive_rwlock_write(&self->yi_lock);
 if unlikely(!self->yi_func) {
  recursive_rwlock_endwrite(&self->yi_lock);
  err_unbound_attribute(&DeeYieldFunctionIterator_Type,"__code__");
  return NULL;
 }
 result = self->yi_func->yf_func->fo_code;
 Dee_Incref(result);
 recursive_rwlock_endwrite(&self->yi_lock);
 return result;
}

PRIVATE DREF DeeObject *DCALL
yfi_getrefs(YFIterator *__restrict self) {
 DREF DeeObject *result;
 DREF DeeFunctionObject *func;
 recursive_rwlock_write(&self->yi_lock);
 if unlikely(!self->yi_func) {
  recursive_rwlock_endwrite(&self->yi_lock);
  err_unbound_attribute(&DeeYieldFunctionIterator_Type,"__refs__");
  return NULL;
 }
 func = self->yi_func->yf_func;
 Dee_Incref(func);
 recursive_rwlock_endwrite(&self->yi_lock);
 result = function_getrefs(func);
 Dee_Decref(func);
 return result;
}

PRIVATE DREF DeeObject *DCALL
yfi_getargs(YFIterator *__restrict self) {
 DREF DeeObject *result;
 recursive_rwlock_write(&self->yi_lock);
 if unlikely(!self->yi_func) {
  recursive_rwlock_endwrite(&self->yi_lock);
  err_unbound_attribute(&DeeYieldFunctionIterator_Type,"__args__");
  return NULL;
 }
 result = (DeeObject *)self->yi_func->yf_args;
 Dee_Incref(result);
 recursive_rwlock_endwrite(&self->yi_lock);
 return result;
}

PRIVATE struct type_getset yfi_getsets[] = {
#ifndef CONFIG_NO_THREADS
    { DeeString_STR(&str_seq), (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getyfunc, NULL, NULL, DOC("->sequence\nAlias for #__yfunc__") },
#endif /* !CONFIG_NO_THREADS */
    { "__frame__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getframe, NULL, NULL, DOC("->frame") },
    { "__this__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getthis, NULL, NULL },
    { "__yfunc__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getyfunc, NULL, NULL, DOC("->yieldfunction") },
    { "__func__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getfunc, NULL, NULL, DOC("->function") },
    { "__code__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getcode, NULL, NULL, DOC("->code") },
    { "__refs__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getrefs, NULL, NULL, DOC("->sequence") },
    { "__args__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_getargs, NULL, NULL, DOC("->sequence") },
    { NULL }
};
#ifdef CONFIG_NO_THREADS
PRIVATE struct type_member yfi_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(YFIterator,yi_func)),
    TYPE_MEMBER_END
};
#endif /* CONFIG_NO_THREADS */

PRIVATE struct type_gc yfi_gc = {
    /* .tp_gc = */(void(DCALL *)(DeeObject *__restrict))&yfi_clear
};


PUBLIC DeeTypeObject DeeYieldFunctionIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"yieldfunction.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */&yfi_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&yfi_new,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(YFIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&yfi_dtor,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL,
        /* .tp_deepload    = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&yfi_visit,
    /* .tp_gc            = */&yfi_gc,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_iter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */yfi_getsets,
#ifdef CONFIG_NO_THREADS
    /* .tp_members       = */yfi_members,
#else
    /* .tp_members       = */NULL,
#endif
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_FUNCTION_C */
