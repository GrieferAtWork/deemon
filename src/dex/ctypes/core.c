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
#ifndef GUARD_DEX_CTYPES_CORE_C
#define GUARD_DEX_CTYPES_CORE_C 1
#define _KOS_SOURCE 1

#include "libctypes.h"
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/arg.h>
#include <deemon/gc.h>
#include <deemon/error.h>
#include <deemon/seq.h>
#include <deemon/string.h>

DECL_BEGIN

PRIVATE char const ispointer_name[] = ":ispointer";
PRIVATE char const islvalue_name[] = ":islvalue";
PRIVATE char const isarray_name[] = ":islvalue";
PRIVATE char const isfunction_name[] = ":isfunction";
PRIVATE char const isstruct_name[] = ":isstruct";
DOC_DEF(ispointer_doc,"()->bool\nReturns :true if @this :structured_type is a :pointer_type");
DOC_DEF(islvalue_doc,"()->bool\nReturns :true if @this :structured_type is an :lvalue_type");
DOC_DEF(isarray_doc,"()->bool\nReturns :true if @this :structured_type is an :array_type");
DOC_DEF(isfunction_doc,"()->bool\nReturns :true if @this :structured_type is a :function_type");
DOC_DEF(isstruct_doc,"()->bool\nReturns :true if @this :structured_type is a :struct_type");


/* Interpret `self' as a pointer and store the result in `*result'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTERN int DCALL
DeeObject_AsPointer(DeeObject *__restrict self,
                    DeeSTypeObject *__restrict pointer_base,
                    union pointer *__restrict result) {
 int error;
 DREF DeePointerTypeObject *pointer_type;
 error = DeeObject_TryAsPointer(self,pointer_base,result);
 if (error <= 0) return error;
 /* Emit a type-assertion error. */
 pointer_type = DeeSType_Pointer(pointer_base);
 if (pointer_type) {
  DeeObject_TypeAssertFailed(self,(DeeTypeObject *)pointer_type);
  Dee_Decref((DeeObject *)pointer_type);
 }
 return -1;
}

/* Same as `DeeObject_AsPointer()', but only ~try~ to interpret it. */
INTERN int DCALL
DeeObject_TryAsPointer(DeeObject *__restrict self,
                       DeeSTypeObject *__restrict pointer_base,
                       union pointer *__restrict result) {
 if (DeeNone_Check(self)) {
  /* none is the NULL pointer. */
null_pointer:
  result->ptr = NULL;
  return 0;
 }
 if (DeePointer_Check(self)) {
  DeeSTypeObject *base;
  base = ((DeePointerTypeObject *)self)->pt_orig;
  /* A pointer of the same type, or a void-pointer. */
  if (base == pointer_base ||
      base == &DeeCVoid_Type ||
      pointer_base == &DeeCVoid_Type) {
   result->ptr = ((struct pointer_object *)self)->p_ptr.ptr;
   return 0;
  }
 }
 /* int(0) also counts as a NULL-pointer. */
 if (DeeInt_Check(self)) {
  uint32_t val;
  if (DeeInt_TryGetU32(self,&val) && val == 0)
      goto null_pointer;
 }
 /* Special handling for strings (which can be cast to `char *') */
 if (DeeString_Check(self)) {
  if (pointer_base == &DeeCVoid_Type) {
   result->ptr = DeeString_STR(self);
   return 0;
  }
  if (pointer_base == &DeeCChar_Type) {
   result->ptr = DeeString_AsUtf8(self);
   if unlikely(!result->ptr) goto err;
   return 0;
  }
  if (pointer_base == &DeeCWChar_Type) {
   result->ptr = DeeString_AsWide(self);
   if unlikely(!result->ptr) goto err;
   return 0;
  }
  if (pointer_base == &DeeCChar16_Type) {
   result->ptr = DeeString_AsUtf16(self,STRING_ERROR_FREPLAC);
   if unlikely(!result->ptr) goto err;
   return 0;
  }
  if (pointer_base == &DeeCChar32_Type) {
   result->ptr = DeeString_AsUtf32(self);
   if unlikely(!result->ptr) goto err;
   return 0;
  }
 }
 /* Conversion failed. */
 return 1;
err:
 return -1;
}



PRIVATE int DCALL
stype_init(DeeSTypeObject *__restrict self) {
 /* Clear all new fields added by structured types. */
 memset((&self->st_base)+1,0,sizeof(DeeSTypeObject)-
          COMPILER_OFFSETAFTER(DeeSTypeObject,st_base));
 return (*DeeType_Type.tp_init.tp_alloc.tp_ctor)((DeeObject *)&self->st_base);
}
PRIVATE void DCALL
stype_fini(DeeSTypeObject *__restrict self) {
 /* Free array/function type caches. */
 Dee_Free(self->st_array.sa_list);
#ifndef CONFIG_NO_CFUNCTION
 if (self->st_ffitype != &ffi_type_pointer)
     Dee_Free(self->st_ffitype);
 Dee_Free(self->st_cfunction.sf_list);
#endif /* !CONFIG_NO_CFUNCTION */
}

PRIVATE DREF DeeObject *DCALL
stype_ispointer(DeePointerTypeObject *__restrict UNUSED(self),
                size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,ispointer_name))
     return NULL;
 return_false;
}
PRIVATE DREF DeeObject *DCALL
stype_islvalue(DeePointerTypeObject *__restrict UNUSED(self),
               size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,islvalue_name))
     return NULL;
 return_false;
}
PRIVATE DREF DeeObject *DCALL
stype_isarray(DeePointerTypeObject *__restrict UNUSED(self),
              size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,isarray_name))
     return NULL;
 return_false;
}
PRIVATE DREF DeeObject *DCALL
stype_isfunction(DeePointerTypeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,isfunction_name))
     return NULL;
 return_false;
}
PRIVATE DREF DeeObject *DCALL
stype_isstruct(DeePointerTypeObject *__restrict UNUSED(self),
               size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,isstruct_name))
     return NULL;
 return_false;
}

PRIVATE DREF DeeObject *DCALL
stype_dofunc(DeeSTypeObject *__restrict self, size_t argc,
             DeeObject **__restrict argv, cc_t cc_flags) {
 DeeSTypeObject **argv_types; size_t i;
 cc_t cc = (cc_t)((unsigned int)CC_DEFAULT|
                  (unsigned int)cc_flags);
 if (argc && DeeString_Check(argv[0])) {
  --argc,++argv;
  cc = cc_lookup(DeeString_STR(argv[-1]));
  if unlikely(cc == CC_INVALID) goto err;
  cc = (cc_t)((unsigned int)cc|
              (unsigned int)cc_flags);
 }
 argv_types = (DeeSTypeObject **)Dee_Malloc(argc*sizeof(DeeSTypeObject *));
 if unlikely(!argv_types) goto err;
 /* Translate argument types. */
 for (i = 0; i < argc; ++i) {
  argv_types[i] = DeeSType_Get(argv[i]);
  if unlikely(!argv_types[i]) goto err_argv;
 }
 /* Lookup the associated C-function type while inheriting the argument vector. */
 return (DREF DeeObject *)DeeSType_CFunction(self,cc,argc,argv_types,true);
err_argv:
 Dee_Free(argv_types);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
stype_func(DeeSTypeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 return stype_dofunc(self,argc,argv,(cc_t)0);
}
PRIVATE DREF DeeObject *DCALL
stype_vfunc(DeeSTypeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 return stype_dofunc(self,argc,argv,CC_FVARARGS);
}


PRIVATE struct type_method stype_methods[] = {
    { ispointer_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_ispointer, DOC_GET(ispointer_doc) },
    { islvalue_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_islvalue, DOC_GET(islvalue_doc) },
    { isarray_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_isarray, DOC_GET(isarray_doc) },
    { isfunction_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_isfunction, DOC_GET(isfunction_doc) },
    { isstruct_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_isstruct, DOC_GET(isstruct_doc) },
    { "func", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_func,
      DOC("(type types...)->function_type\n"
          "(string calling_convention, type types...)->function_type\n"
          "@throw ValueError The given @calling_convention is unknown, or not supported by the host\n"
          "@param calling_convention The name of the calling convention\n"
          "Construct a new function prototype, using @types as argument, @this "
          "as return type, and @calling_convention as calling convention\n"
          "Note that unlike #{operator ()}, certain types from the deemon core are "
          "also accepted as argument types, such as :deemon:bool inplace of :bool") },
    { "vfunc", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_vfunc,
      DOC("(type types...)->function_type\n"
          "(string calling_convention, type types...)->function_type\n"
          "@throw ValueError The given @calling_convention is unknown, or not supported by the host\n"
          "@param calling_convention The name of the calling convention\n"
          "Same as #func, but enable support for varrgs") },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
stype_sizeof(DeeSTypeObject *__restrict self) {
 size_t result = DeeSType_Sizeof(self);
 return DeeInt_NewSize(result);
}
PRIVATE DREF DeeObject *DCALL
stype_alignof(DeeSTypeObject *__restrict self) {
 size_t result = DeeSType_Alignof(self);
 return DeeInt_NewSize(result);
}

PRIVATE struct type_getset stype_getsets[] = {
    { "ptr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeSType_Pointer, NULL, NULL,
      DOC("->pointer_type\nReturns the pointer type associated with @this :structured_type") },
    { islvalue_name+3, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeSType_LValue, NULL, NULL,
      DOC("->lvalue_type\nReturns the l-value type associated with @this :structured_type") },
    { "sizeof", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stype_sizeof, NULL, NULL,
      DOC("->int\nReturns the size of @this :structured_type in bytes") },
    { "alignof", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stype_alignof, NULL, NULL,
      DOC("->int\nReturns the alignment of @this :structured_type in bytes") },
    { ispointer_name+3, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeSType_Pointer, NULL, NULL,
      DOC("->pointer_type\nAlias for #ptr") },
    { NULL }
};

PRIVATE DREF DeeArrayTypeObject *DCALL
stype_getitem(DeeSTypeObject *__restrict self,
              DeeObject *__restrict array_size_ob) {
 /* Use `operator []' to construct array types. */
 size_t array_size;
 if (DeeObject_AsSize(array_size_ob,&array_size))
     return NULL;
 return DeeSType_Array(self,array_size);
}

PRIVATE struct type_seq stype_seq = {
    /* .tp_iter_self = */NULL,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict index))&stype_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
};


PRIVATE DREF DeeObject *DCALL
stype_call(DeeSTypeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 size_t i;
 /* Create a new instance, or create a new function-type. */
 if (!argc) goto create_inst;
 for (i = 0; i < argc; ++i) {
  if (!DeeSType_Check(argv[i]))
       goto create_inst;
 }
 /* Special case: `xxx(void)' constructs a function prototype with no arguments. */
 if (argc == 1 && argv[0] == (DeeObject *)&DeeCVoid_Type)
     argc = 0;
 /* Use the default calling convention for constructing this function type. */
 return (DREF DeeObject *)DeeSType_CFunction(self,CC_DEFAULT,argc,
                                            (DeeSTypeObject **)argv,
                                             false);
create_inst:
 /* Construct a new instance. */
 return DeeObject_New((DeeTypeObject *)self,argc,argv);
}


INTERN DeeTypeObject DeeSType_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"structured_type",
    /* .tp_doc      = */DOC("operator getitem\n"
                            "Construct new array types using this type as item type\n"
                            "\n"
                            "operator()()->structured\n"
                            "operator()(object args...)->structured\n"
                            "operator()(structured_type types...)->function\n"
                            "Construct a new function type using this type as return type, "
                            "or construct a new instance of @this :structured_type"),
    /* .tp_flags    = */TP_FNORMAL|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeType_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(void *)&stype_init,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeSTypeObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&stype_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stype_call,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&stype_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */stype_methods,
    /* .tp_getsets       = */stype_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


PRIVATE void DCALL
ptype_fini(DeePointerTypeObject *__restrict self) {
 DeeSTypeObject *orig = self->pt_orig;
 /* Delete the weak-link to the original type. */
#ifndef CONFIG_NO_THREADS
 rwlock_write(&orig->st_cachelock);
#endif
 if (orig->st_pointer == self)
     orig->st_pointer = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&orig->st_cachelock);
#endif
 Dee_Decref((DeeObject *)orig);
}
PRIVATE void DCALL
ptype_visit(DeePointerTypeObject *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit((DeeObject *)self->pt_orig);
}

PRIVATE struct type_member ptype_members[] = {
    TYPE_MEMBER_FIELD("base",STRUCT_OBJECT,offsetof(DeePointerTypeObject,pt_orig)),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
ptype_ispointer(DeePointerTypeObject *__restrict UNUSED(self),
                size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,ispointer_name))
     return NULL;
 return_true;
}

PRIVATE struct type_method ptype_methods[] = {
    { ispointer_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&ptype_ispointer, DOC_GET(ispointer_doc) },
    { NULL }
};

INTERN DeeTypeObject DeePointerType_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"pointer_type",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSType_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeePointerTypeObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ptype_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&ptype_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */ptype_methods,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */ptype_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


/* Assert that we can re-use some operators from `pointer_type'. */
STATIC_ASSERT(COMPILER_OFFSETOF(DeePointerTypeObject,pt_orig) ==
              COMPILER_OFFSETOF(DeeLValueTypeObject,lt_orig));

PRIVATE DREF DeeObject *DCALL
ltype_islvalue(DeePointerTypeObject *__restrict UNUSED(self),
               size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,islvalue_name))
     return NULL;
 return_true;
}

PRIVATE struct type_method ltype_methods[] = {
    { islvalue_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&ltype_islvalue, DOC_GET(islvalue_doc) },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
ltype_sizeof(DeeLValueTypeObject *__restrict self) {
 size_t result = DeeSType_Sizeof(self->lt_orig);
 return DeeInt_NewSize(result);
}

PRIVATE struct type_getset ltype_getsets[] = {
    { "sizeof", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ltype_sizeof, NULL, NULL,
      DOC("->int\nReturns the size of the base of @this :lvalue_type in bytes") },
    { NULL }
};

PRIVATE void DCALL
ltype_fini(DeeLValueTypeObject *__restrict self) {
 DeeSTypeObject *orig = self->lt_orig;
 /* Delete the weak-link to the original type. */
#ifndef CONFIG_NO_THREADS
 rwlock_write(&orig->st_cachelock);
#endif
 if (orig->st_lvalue == self)
     orig->st_lvalue = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&orig->st_cachelock);
#endif
 Dee_Decref((DeeObject *)orig);
}


INTERN DeeTypeObject DeeLValueType_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"lvalue_type",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSType_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeLValueTypeObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ltype_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&ptype_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */ltype_methods,
    /* .tp_getsets       = */ltype_getsets,
    /* .tp_members       = */ptype_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE DREF DeeObject *DCALL
make_structured_name(DeeSTypeObject *__restrict self, char class_ch) {
 /* TODO: Special handling for function-pointers. */
 return DeeString_Newf("%s%c",self->st_base.tp_name,class_ch);
}


PRIVATE DREF DeePointerTypeObject *DCALL
pointertype_new(DeeSTypeObject *__restrict self) {
 DREF DeePointerTypeObject *result;
 DREF DeeStringObject *name;
 result = DeeGCObject_CALLOC(DeePointerTypeObject);
 if unlikely(!result) goto done;
 /* Create the name of the resulting type. */
 name = (DREF DeeStringObject *)make_structured_name(self,'*');
 if unlikely(!name) goto err_r;
 /* Store a reference to the pointed-to type. */
 Dee_Incref((DeeObject *)self);
 Dee_Incref((DeeObject *)&DeePointer_Type);
 /* Initialize fields. */
 result->pt_orig                  = self; /* Inherit reference. */
 result->pt_base.st_align         = CONFIG_CTYPES_ALIGNOF_POINTER;
 result->pt_base.st_base.tp_name  = DeeString_STR(name); /* Inherit reference. */
 result->pt_base.st_base.tp_init.tp_alloc.tp_instance_size = sizeof(struct pointer_object);
 result->pt_base.st_base.tp_flags = TP_FTRUNCATE|TP_FINHERITCTOR|TP_FNAMEOBJECT|TP_FHEAP|TP_FMOVEANY;
 result->pt_base.st_base.tp_base  = &DeePointer_Type.pt_base.st_base; /* Inherit reference. */
 result->pt_size                  = DeeSType_Sizeof(self);
#ifndef CONFIG_NO_CFUNCTION
 result->pt_base.st_ffitype = &ffi_type_pointer;
#endif

 /* Setup use of the proper math operators. */
 if (result->pt_size == 0)
  result->pt_base.st_math = &pointer_math0;
 else if (result->pt_size == 1)
  result->pt_base.st_math = &pointer_math1;
 else {
  result->pt_base.st_math = &pointer_mathn;
 }

 /* Finalize the pointer type. */
 DeeObject_Init((DeeTypeObject *)result,&DeePointerType_Type);
 DeeGC_Track((DeeObject *)result);
done:
 return result;
err_r:
 DeeObject_Free(result);
 return NULL;
}
PRIVATE DREF DeeLValueTypeObject *DCALL
lvaluetype_new(DeeSTypeObject *__restrict self) {
 DREF DeeLValueTypeObject *result;
 DREF DeeStringObject *name;
 result = DeeGCObject_CALLOC(DeeLValueTypeObject);
 if unlikely(!result) goto done;
 /* Create the name of the resulting type. */
 name = (DREF DeeStringObject *)make_structured_name(self,'&');
 if unlikely(!name) goto err_r;
 /* Store a reference to the pointed-to type. */
 Dee_Incref((DeeObject *)self);
 Dee_Incref((DeeObject *)&DeeLValue_Type);
 /* Initialize fields. */
 result->lt_orig                  = self; /* Inherit reference. */
 result->lt_base.st_align         = CONFIG_CTYPES_ALIGNOF_LVALUE;
 result->lt_base.st_base.tp_name  = DeeString_STR(name); /* Inherit reference. */
 result->lt_base.st_base.tp_init.tp_alloc.tp_instance_size = sizeof(struct lvalue_object);
 result->lt_base.st_base.tp_flags = TP_FTRUNCATE|TP_FVARIABLE|TP_FINHERITCTOR|TP_FNAMEOBJECT|TP_FHEAP|TP_FMOVEANY;
 result->lt_base.st_base.tp_base  = &DeeLValue_Type.lt_base.st_base; /* Inherit reference. */
#ifndef CONFIG_NO_CFUNCTION
 result->lt_base.st_ffitype = &ffi_type_pointer;
#endif

 /* Initialize the lvalue-pointer of the resulting l-value type to point
  * to itself, thus preventing weird types like l-value-to-l-value. */
 result->lt_base.st_lvalue = result;
 /* Finalize the lvalue type. */
 DeeObject_Init((DeeTypeObject *)result,&DeeLValueType_Type);
 DeeGC_Track((DeeObject *)result);
done:
 return result;
err_r:
 DeeObject_Free(result);
 return NULL;
}

INTERN DREF DeePointerTypeObject *DCALL
DeeSType_Pointer(DeeSTypeObject *__restrict self) {
 DREF DeePointerTypeObject *result;
 ASSERT_OBJECT_TYPE((DeeObject *)self,&DeeSType_Type);
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->st_cachelock);
#endif
 result = self->st_pointer;
 if (result && !Dee_IncrefIfNotZero((DeeObject *)result))
     result = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->st_cachelock);
#endif
 if (!result) {
  /* Lazily construct missing types. */
  result = pointertype_new(self);
  if likely(result) {
#ifndef CONFIG_NO_THREADS
   rwlock_write(&self->st_cachelock);
#endif
   /* Check if the type was created due to race conditions. */
   if unlikely(self->st_pointer &&
               Dee_IncrefIfNotZero((DeeObject *)self->st_pointer)) {
    DREF DeePointerTypeObject *new_result;
    new_result = self->st_pointer;
#ifndef CONFIG_NO_THREADS
    rwlock_endwrite(&self->st_cachelock);
#endif
    Dee_DecrefDokill((DeeObject *)result);
    return new_result;
   }
   self->st_pointer = result; /* Weakly referenced. */
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->st_cachelock);
#endif
  }
 }
 return result;
}
INTERN DREF DeeLValueTypeObject *DCALL
DeeSType_LValue(DeeSTypeObject *__restrict self) {
 DREF DeeLValueTypeObject *result;
 ASSERT_OBJECT_TYPE((DeeObject *)self,&DeeSType_Type);
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->st_cachelock);
#endif
 result = self->st_lvalue;
 if (result && !Dee_IncrefIfNotZero((DeeObject *)result))
     result = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->st_cachelock);
#endif
 if (!result) {
  /* Lazily construct missing types. */
  result = lvaluetype_new(self);
  if likely(result) {
#ifndef CONFIG_NO_THREADS
   rwlock_write(&self->st_cachelock);
#endif
   /* Check if the type was created due to race conditions. */
   if unlikely(self->st_lvalue &&
               Dee_IncrefIfNotZero((DeeObject *)self->st_lvalue)) {
    DREF DeeLValueTypeObject *new_result;
    new_result = self->st_lvalue;
#ifndef CONFIG_NO_THREADS
    rwlock_endwrite(&self->st_cachelock);
#endif
    Dee_DecrefDokill((DeeObject *)result);
    return new_result;
   }
   self->st_lvalue = result; /* Weakly referenced. */
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->st_cachelock);
#endif
  }
 }
 return result;
}

INTERN DeeSTypeObject *DCALL
DeeSType_Get(DeeObject *__restrict self) {
 /* Quick check: is it a structured type. */
 if (DeeSType_Check(self))
     return (DeeSTypeObject *)self;

 /* Map some builtin types to their structured counterparts. */
 if (DeeNone_Check(self) ||
     self == (DeeObject *)&DeeNone_Type)
     return &DeeCVoid_Type;
 if (self == (DeeObject *)&DeeBool_Type)
     return &DeeCBool_Type;
 if (self == (DeeObject *)&DeeInt_Type)
     return &DeeCInt_Type;
 if (self == (DeeObject *)&DeeFloat_Type)
     return &DeeCDouble_Type;

 /* Throw a type-assertion failure error. */
 DeeObject_TypeAssertFailed(self,&DeeSType_Type);
 return NULL;
}


INTERN ATTR_COLD int DCALL
err_unimplemented_operator(DeeSTypeObject *__restrict tp, uint16_t operator_name) {
 struct opinfo *info;
 info = Dee_OperatorInfo(Dee_TYPE((DeeObject *)tp),operator_name);
 ASSERT_OBJECT((DeeTypeObject *)tp);
 ASSERT(DeeType_Check((DeeObject *)tp));
 return DeeError_Throwf(&DeeError_NotImplemented,
                        "Operator `%k.__%s__' is not implemented",
                        tp,info ? info->oi_sname : "??" "?");
}

PRIVATE int DCALL
struct_ctor(DeeObject *__restrict self) {
 DeeSTypeObject *tp_self = (DeeSTypeObject *)Dee_TYPE(self);
 while (!DeeSType_Check((DeeObject *)tp_self))
     tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self);
 /* ZERO-initialize by default. */
 memset(DeeStruct_Data(self),0,DeeSType_Sizeof(tp_self));
 return 0;
}
PRIVATE int DCALL
struct_copy(DeeObject *__restrict self,
            DeeObject *__restrict other) {
 DeeSTypeObject *tp_self = (DeeSTypeObject *)Dee_TYPE(self);
 while (!DeeSType_Check((DeeObject *)tp_self))
     tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self);
 /* Copy memory. */
 memcpy(DeeStruct_Data(self),
        DeeStruct_Data(other),
        DeeSType_Sizeof(tp_self));
 return 0;
}
PRIVATE int DCALL
struct_init(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeSTypeObject *orig_type,*tp_self;
 orig_type = tp_self = (DeeSTypeObject *)Dee_TYPE(self);
 do {
  if (tp_self->st_init)
      return (*tp_self->st_init)(orig_type,DeeStruct_Data(self),argc,argv);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_CONSTRUCTOR);
 return 0;
}
PRIVATE int DCALL
struct_assign(DeeObject *__restrict self,
              DeeObject *__restrict some_object) {
 return DeeStruct_Assign((DeeSTypeObject *)Dee_TYPE(self),
                          DeeStruct_Data(self),some_object);
}

#define DEFINE_UNARY_STRUCT_OPERATOR(Treturn,struct_xxx,DeeStruct_Xxx) \
PRIVATE Treturn DCALL \
struct_xxx(DeeObject *__restrict self) { \
 return DeeStruct_Xxx((DeeSTypeObject *)Dee_TYPE(self), \
                       DeeStruct_Data(self)); \
}
#define DEFINE_BINARY_STRUCT_OPERATOR(Treturn,struct_xxx,DeeStruct_Xxx) \
PRIVATE Treturn DCALL \
struct_xxx(DeeObject *__restrict self, DeeObject *__restrict other) { \
 return DeeStruct_Xxx((DeeSTypeObject *)Dee_TYPE(self), \
                       DeeStruct_Data(self),other); \
}
#define DEFINE_TRINARY_STRUCT_OPERATOR(Treturn,struct_xxx,DeeStruct_Xxx) \
PRIVATE Treturn DCALL \
struct_xxx(DeeObject *__restrict self, DeeObject *__restrict a, DeeObject *__restrict b) { \
 return DeeStruct_Xxx((DeeSTypeObject *)Dee_TYPE(self), \
                       DeeStruct_Data(self),a,b); \
}
#define DEFINE_UNARY_INPLACE_STRUCT_OPERATOR(Treturn,struct_xxx,DeeStruct_Xxx) \
PRIVATE Treturn DCALL \
struct_xxx(DeeObject **__restrict pself) { \
 DeeObject *self = *pself; \
 return DeeStruct_Xxx((DeeSTypeObject *)Dee_TYPE(self), \
                       DeeStruct_Data(self)); \
}
#define DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(Treturn,struct_xxx,DeeStruct_Xxx) \
PRIVATE Treturn DCALL \
struct_xxx(DeeObject **__restrict pself, DeeObject *__restrict other) { \
 DeeObject *self = *pself; \
 return DeeStruct_Xxx((DeeSTypeObject *)Dee_TYPE(self), \
                       DeeStruct_Data(self),other); \
}
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_str,DeeStruct_Str)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_repr,DeeStruct_Repr)
DEFINE_UNARY_STRUCT_OPERATOR(int,struct_bool,DeeStruct_Bool)
PRIVATE DREF DeeObject *DCALL
struct_call(DeeObject *__restrict self, size_t argc,
            DeeObject **__restrict argv) {
 return DeeStruct_Call((DeeSTypeObject *)Dee_TYPE(self),
                         DeeStruct_Data(self),argc,argv);
}
PRIVATE int DCALL
struct_int32(DeeObject *__restrict self, int32_t *__restrict result) {
 return DeeStruct_Int32((DeeSTypeObject *)Dee_TYPE(self),
                         DeeStruct_Data(self),result);
}
PRIVATE int DCALL
struct_int64(DeeObject *__restrict self, int64_t *__restrict result) {
 return DeeStruct_Int64((DeeSTypeObject *)Dee_TYPE(self),
                         DeeStruct_Data(self),result);
}
PRIVATE int DCALL
struct_double(DeeObject *__restrict self, double *__restrict result) {
 return DeeStruct_Double((DeeSTypeObject *)Dee_TYPE(self),
                          DeeStruct_Data(self),result);
}
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_int,DeeStruct_Int)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_inv,DeeStruct_Inv)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_pos,DeeStruct_Pos)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_neg,DeeStruct_Neg)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_add,DeeStruct_Add)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_sub,DeeStruct_Sub)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_mul,DeeStruct_Mul)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_div,DeeStruct_Div)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_mod,DeeStruct_Mod)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_shl,DeeStruct_Shl)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_shr,DeeStruct_Shr)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_and,DeeStruct_And)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_or,DeeStruct_Or)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_xor,DeeStruct_Xor)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_pow,DeeStruct_Pow)
DEFINE_UNARY_INPLACE_STRUCT_OPERATOR(int,struct_inc,DeeStruct_Inc)
DEFINE_UNARY_INPLACE_STRUCT_OPERATOR(int,struct_dec,DeeStruct_Dec)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_add,DeeStruct_InplaceAdd)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_sub,DeeStruct_InplaceSub)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_mul,DeeStruct_InplaceMul)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_div,DeeStruct_InplaceDiv)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_mod,DeeStruct_InplaceMod)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_shl,DeeStruct_InplaceShl)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_shr,DeeStruct_InplaceShr)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_and,DeeStruct_InplaceAnd)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_or, DeeStruct_InplaceOr)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_xor,DeeStruct_InplaceXor)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int,struct_inplace_pow,DeeStruct_InplacePow)
DEFINE_UNARY_STRUCT_OPERATOR(dhash_t,struct_hash,DeeStruct_Hash)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_eq,DeeStruct_Eq)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_ne,DeeStruct_Ne)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_lo,DeeStruct_Lo)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_le,DeeStruct_Le)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_gr,DeeStruct_Gr)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_ge,DeeStruct_Ge)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_iter,DeeStruct_IterSelf)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *,struct_size,DeeStruct_GetSize)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_contains,DeeStruct_Contains)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_getitem,DeeStruct_GetItem)
DEFINE_BINARY_STRUCT_OPERATOR(int,struct_delitem,DeeStruct_DelItem)
DEFINE_TRINARY_STRUCT_OPERATOR(int,struct_setitem,DeeStruct_SetItem)
DEFINE_TRINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_getrange,DeeStruct_GetRange)
DEFINE_TRINARY_STRUCT_OPERATOR(int,struct_delrange,DeeStruct_DelRange)
PRIVATE int DCALL
struct_setrange(DeeObject *__restrict self, DeeObject *__restrict begin,
                DeeObject *__restrict end, DeeObject *__restrict value) {
 return DeeStruct_SetRange((DeeSTypeObject *)Dee_TYPE(self),
                            DeeStruct_Data(self),begin,end,value);
}
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *,struct_getattr,DeeStruct_GetAttr)
DEFINE_BINARY_STRUCT_OPERATOR(int,struct_delattr,DeeStruct_DelAttr)
DEFINE_TRINARY_STRUCT_OPERATOR(int,struct_setattr,DeeStruct_SetAttr)
PRIVATE dssize_t DCALL
struct_enumattr(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict UNUSED(self),
                denum_t proc, void *arg) {
 return DeeStruct_EnumAttr((DeeSTypeObject *)tp_self,proc,arg);
}
#undef DEFINE_BINARY_INPLACE_STRUCT_OPERATOR
#undef DEFINE_UNARY_INPLACE_STRUCT_OPERATOR
#undef DEFINE_TRINARY_STRUCT_OPERATOR
#undef DEFINE_BINARY_STRUCT_OPERATOR
#undef DEFINE_UNARY_STRUCT_OPERATOR

PRIVATE struct type_math struct_math = {
    /* .tp_int32       = */&struct_int32,
    /* .tp_int64       = */&struct_int64,
    /* .tp_double      = */&struct_double,
    /* .tp_int         = */&struct_int,
    /* .tp_inv         = */&struct_inv,
    /* .tp_pos         = */&struct_pos,
    /* .tp_neg         = */&struct_neg,
    /* .tp_add         = */&struct_add,
    /* .tp_sub         = */&struct_sub,
    /* .tp_mul         = */&struct_mul,
    /* .tp_div         = */&struct_div,
    /* .tp_mod         = */&struct_mod,
    /* .tp_shl         = */&struct_shl,
    /* .tp_shr         = */&struct_shr,
    /* .tp_and         = */&struct_and,
    /* .tp_or          = */&struct_or,
    /* .tp_xor         = */&struct_xor,
    /* .tp_pow         = */&struct_pow,
    /* .tp_inc         = */&struct_inc,
    /* .tp_dec         = */&struct_dec,
    /* .tp_inplace_add = */&struct_inplace_add,
    /* .tp_inplace_sub = */&struct_inplace_sub,
    /* .tp_inplace_mul = */&struct_inplace_mul,
    /* .tp_inplace_div = */&struct_inplace_div,
    /* .tp_inplace_mod = */&struct_inplace_mod,
    /* .tp_inplace_shl = */&struct_inplace_shl,
    /* .tp_inplace_shr = */&struct_inplace_shr,
    /* .tp_inplace_and = */&struct_inplace_and,
    /* .tp_inplace_or  = */&struct_inplace_or,
    /* .tp_inplace_xor = */&struct_inplace_xor,
    /* .tp_inplace_pow = */&struct_inplace_pow
};

PRIVATE struct type_cmp struct_cmp = {
    /* .tp_hash = */&struct_hash,
    /* .tp_eq   = */&struct_eq,
    /* .tp_ne   = */&struct_ne,
    /* .tp_lo   = */&struct_lo,
    /* .tp_le   = */&struct_le,
    /* .tp_gr   = */&struct_gr,
    /* .tp_ge   = */&struct_ge
};

PRIVATE struct type_seq struct_seq = {
    /* .tp_iter_self = */&struct_iter,
    /* .tp_size      = */&struct_size,
    /* .tp_contains  = */&struct_contains,
    /* .tp_get       = */&struct_getitem,
    /* .tp_del       = */&struct_delitem,
    /* .tp_set       = */&struct_setitem,
    /* .tp_range_get = */&struct_getrange,
    /* .tp_range_del = */&struct_delrange,
    /* .tp_range_set = */&struct_setrange
};

PRIVATE struct type_attr struct_attr = {
    /* .tp_getattr  = */&struct_getattr,
    /* .tp_delattr  = */&struct_delattr,
    /* .tp_setattr  = */&struct_setattr,
    /* .tp_enumattr = */&struct_enumattr
};


PRIVATE int DCALL
struct_getbuf(DeeObject *__restrict self,
              DeeBuffer *__restrict info,
              unsigned int UNUSED(flags)) {
 info->bb_base = DeeStruct_Data(self);
 info->bb_size = DeeStruct_Size(self);
 return 0;
}

PRIVATE struct type_buffer struct_buffer = {
    /* .tp_getbuf       = */&struct_getbuf,
    /* .tp_putbuf       = */NULL,
    /* .tp_buffer_flags = */DEE_BUFFER_TYPE_FNORMAL
};

PRIVATE DREF DeeObject *DCALL
struct_sizeof(DeeObject *__restrict self) {
 size_t result = DeeStruct_Size(self);
 return DeeInt_NewSize(result);
}
PRIVATE DREF DeeObject *DCALL
struct_alignof(DeeObject *__restrict self) {
 size_t result = DeeStruct_Align(self);
 return DeeInt_NewSize(result);
}
PRIVATE DREF DeeObject *DCALL
struct_ref(DeeObject *__restrict self) {
 DREF struct pointer_object *result;
 DREF DeePointerTypeObject *pointer_type;
 pointer_type = DeeSType_Pointer((DeeSTypeObject *)Dee_TYPE(self));
 if unlikely(!pointer_type) goto err;
 result = DeeObject_MALLOC(struct pointer_object);
 if unlikely(!result) goto err;
 /* Construct a new pointer directed at the data of this structured object. */
 DeeObject_InitNoref(result,(DREF DeeTypeObject *)pointer_type); /* Inherit reference: pointer_type */
 result->p_ptr.ptr = DeeStruct_Data(self);
 return (DREF DeeObject *)result;
err:
 return NULL;
}

PRIVATE struct type_getset struct_getsets[] = {
    { "sizeof", &struct_sizeof, NULL, NULL,
      DOC("->int\nReturns the size of @this :structured object") },
    { "alignof", &struct_alignof, NULL, NULL,
      DOC("->int\nReturns the alignment of @this :structured object") },
    { "ref", &struct_ref, NULL, NULL,
      DOC("->pointer\nReturns a pointer to @this :structured object") },
    { NULL }
};



/* This type needs to implement _all_ operators
 * to forward them to their structured counterparts! */
INTERN DeeSTypeObject DeeStructured_Type = {
    /* .st_base = */{
        OBJECT_HEAD_INIT((DeeTypeObject *)&DeeSType_Type),
        /* .tp_name     = */"structured",
        /* .tp_doc      = */NULL,
        /* .tp_flags    = */TP_FNORMAL|TP_FTRUNCATE|TP_FMOVEANY,
        /* .tp_weakrefs = */0,
        /* .tp_features = */TF_NONE,
        /* .tp_base     = */&DeeObject_Type,
        /* .tp_init = */{
            {
                /* .tp_alloc = */{
                    /* .tp_ctor      = */(void *)&struct_ctor,
                    /* .tp_copy_ctor = */(void *)&struct_copy,
                    /* .tp_deep_ctor = */(void *)&struct_copy,
                    /* .tp_any_ctor  = */(void *)&struct_init,
                    /* TODO: Use a custom allocator that takes alignment into account. */
                    /* .tp_free      = */NULL,
                    {
                        /* .tp_instance_size = */sizeof(DeeObject)
                    }
                }
            },
            /* .tp_dtor        = */NULL,
            /* .tp_assign      = */&struct_assign,
            /* .tp_move_assign = */&struct_assign
        },
        /* .tp_cast = */{
            /* .tp_str  = */&struct_str,
            /* .tp_repr = */&struct_repr,
            /* .tp_bool = */&struct_bool
        },
        /* .tp_call          = */&struct_call,
        /* .tp_visit         = */NULL,
        /* .tp_gc            = */NULL,
        /* .tp_math          = */&struct_math,
        /* .tp_cmp           = */&struct_cmp,
        /* .tp_seq           = */&struct_seq,
        /* .tp_iter_next     = */NULL,
        /* .tp_attr          = */&struct_attr,
        /* .tp_with          = */NULL,
        /* .tp_buffer        = */&struct_buffer,
        /* .tp_methods       = */NULL,
        /* .tp_getsets       = */struct_getsets,
        /* .tp_members       = */NULL,
        /* .tp_class_methods = */NULL,
        /* .tp_class_getsets = */NULL,
        /* .tp_class_members = */NULL
    },
#ifndef CONFIG_NO_THREADS
    /* .st_cachelock = */RWLOCK_INIT,
#endif
    /* .st_pointer  = */&DeePointer_Type,
    /* .st_lvalue   = */&DeeLValue_Type,
    /* .st_array    = */STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
    /* .st_cfunction= */STYPE_CFUNCTION_INIT,
    /* .st_ffitype  = */&ffi_type_void,
#endif /* !CONFIG_NO_CFUNCTION */
    /* .st_align    = */0,
    /* .st_init     = */NULL,
    /* .st_assign   = */NULL,
    /* .st_cast     = */{
        /* .st_str  = */NULL,
        /* .st_repr = */NULL,
        /* .st_bool = */NULL
    },
    /* .st_call     = */NULL,
    /* .st_math     = */NULL,
    /* .st_cmp      = */NULL,
    /* .st_seq      = */NULL,
    /* .st_attr     = */NULL
};


INTERN DREF DeeObject *DCALL
DeeObject_Ref(DeeObject *__restrict self) {
 DREF struct pointer_object *result;
 DREF DeePointerTypeObject *tp_result;
 if (DeeObject_AssertType(self,(DeeTypeObject *)&DeeStructured_Type))
     goto err;
 if (DeeLValue_Check(self)) {
  /* Special case: Must reference the data originally designated through the l-value. */
  /* Lookup the required pointer type. */
  tp_result = DeeSType_Pointer(((DeeLValueTypeObject *)Dee_TYPE(self))->lt_orig);
  if unlikely(!tp_result)
      goto err;
  /* Create the new pointer object. */
  result = (DREF struct pointer_object *)DeeObject_MALLOC(struct pointer_object);
  if unlikely(!result) goto err_tpres;
  DeeObject_InitNoref(result,(DeeTypeObject *)tp_result); /* Inherit reference: result_type */
  /* Copy the l-value pointer into the regular pointer. */
  result->p_ptr.ptr = ((struct lvalue_object *)self)->l_ptr.ptr;
  goto done;
 }
 /* Lookup the required pointer type. */
 tp_result = DeeSType_Pointer((DeeSTypeObject *)Dee_TYPE(self));
 if unlikely(!tp_result)
     goto err;
 /* Create the new pointer object. */
 result = (DREF struct pointer_object *)DeeObject_MALLOC(struct pointer_object);
 if unlikely(!result) goto err_tpres;
 DeeObject_InitNoref(result,(DeeTypeObject *)tp_result); /* Inherit reference: result_type */
 result->p_ptr.ptr = DeeStruct_Data(self);
done:
 return (DREF DeeObject *)result;
err_tpres:
 Dee_Decref((DeeObject *)tp_result);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeObject_Deref(DeeObject *__restrict self) {
 struct lvalue_object *result;
 DREF DeeLValueTypeObject *tp_result;
 if (DeePointer_Check(self)) {
  /* Regular pointer. */
  tp_result = DeeSType_LValue(((DeePointerTypeObject *)Dee_TYPE(self))->pt_orig);
  if unlikely(!tp_result) goto err;
  result = DeeObject_MALLOC(struct lvalue_object);
  if unlikely(!result) goto err_tpres;
  DeeObject_InitNoref(result,(DeeTypeObject *)tp_result); /* Inherit reference: result_type */
  result->l_ptr.ptr = ((struct pointer_object *)self)->p_ptr.ptr;
  return (DREF DeeObject *)result;
 }
 if (DeeLValue_Check(self)) {
  DREF DeePointerTypeObject *tp_base;
  tp_base = (DeePointerTypeObject *)((DeeLValueTypeObject *)Dee_TYPE(self))->lt_orig;
  if (DeePointerType_Check(tp_base)) {
   /* LValue-to-pointer. */
   tp_result = DeeSType_LValue(tp_base->pt_orig);
   if unlikely(!tp_result) goto err;
   result = DeeObject_MALLOC(struct lvalue_object);
   if unlikely(!result) goto err_tpres;
   DeeObject_InitNoref(result,(DeeTypeObject *)tp_result); /* Inherit reference: result_type */
   /* Dereference this pointer. */
   CTYPES_FAULTPROTECT(result->l_ptr.ptr = *(void **)((struct lvalue_object *)self)->l_ptr.ptr,
                       goto err_tpres_r);
   return (DREF DeeObject *)result;
  }
 }
 /* Throw an error. */
 DeeObject_TypeAssertFailed(self,(DeeTypeObject *)&DeePointer_Type);
err:
 return NULL;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
err_tpres_r:
 DeeObject_Free(result);
#endif /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
err_tpres:
 Dee_Decref((DeeObject *)tp_result);
 goto err;
}

INTERN DREF DeeObject *DCALL
DeePointer_New(DeePointerTypeObject *__restrict pointer_type,
               void *pointer_value) {
 struct pointer_object *result;
 ASSERT(DeePointerType_Check(pointer_type));
 /* Allocate a new pointer object. */
 result = DeeObject_MALLOC(struct pointer_object);
 if unlikely(!result) goto done;
 /* Initialize the new pointer object. */
 DeeObject_Init(result,(DeeTypeObject *)pointer_type);
 result->p_ptr.ptr = pointer_value;
done:
 return (DREF DeeObject *)result;
}
INTERN DREF DeeObject *DCALL
DeePointer_NewFor(DeeSTypeObject *__restrict pointer_type,
                  void *pointer_value) {
 DREF DeeObject *result;
 DREF DeePointerTypeObject *ptr_type;
 ptr_type = DeeSType_Pointer(pointer_type);
 if unlikely(!ptr_type) goto err;
 result = DeePointer_New(ptr_type,pointer_value);
 Dee_Decref((DeeObject *)ptr_type);
 return result;
err:
 return NULL;
}



INTERN int DCALL
DeeStruct_Assign(DeeSTypeObject *__restrict tp_self,
                 void *self, DeeObject *__restrict value) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_assign)
      return (*tp_self->st_assign)(orig_type,self,value);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 if (DeeObject_InstanceOf(value,(DeeTypeObject *)orig_type)) {
  uint8_t *dst,*src; size_t size; /* Copy-assign. */
  dst  = (uint8_t *)self;
  src  = (uint8_t *)DeeStruct_Data(value);
  size = DeeSType_Sizeof(orig_type);
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
  CTYPES_FAULTPROTECT(memcpy(dst,src,size),return -1);
#else
  CTYPES_FAULTPROTECT({
      while (size--) *dst++ = *src++;
  },return -1);
#endif
#else
  memcpy(dst,src,size);
#endif
  return 0;
 }
 if (DeeNone_Check(value)) {
  uint8_t *dst; size_t size; /* Clear memory. */
  dst  = (uint8_t *)self;
  size = DeeSType_Sizeof(orig_type);
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
  CTYPES_FAULTPROTECT(memset(dst,0,size),return -1);
#else
  CTYPES_FAULTPROTECT({
      while (size--) *dst++ = 0;
  },return -1);
#endif
#else
  memset(dst,0,size);
#endif
  return 0;
 }
 return err_unimplemented_operator(orig_type,OPERATOR_ASSIGN);
}
INTERN DREF DeeObject *DCALL
DeeStruct_Str(DeeSTypeObject *__restrict tp_self,
              void *self) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cast.st_str)
      return (*tp_self->st_cast.st_str)(orig_type,self);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_STR);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_Repr(DeeSTypeObject *__restrict tp_self,
               void *self) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cast.st_repr)
      return (*tp_self->st_cast.st_repr)(orig_type,self);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_REPR);
 return NULL;
}
INTERN int DCALL
DeeStruct_Bool(DeeSTypeObject *__restrict tp_self,
               void *self) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cast.st_bool)
      return (*tp_self->st_cast.st_bool)(orig_type,self);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_BOOL);
}
INTERN DREF DeeObject *DCALL
DeeStruct_Call(DeeSTypeObject *__restrict tp_self,
               void *self, size_t argc,
               DeeObject **__restrict argv) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_call)
      return (*tp_self->st_call)(orig_type,self,argc,argv);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_CALL);
 return NULL;
}
INTERN int DCALL
DeeStruct_Int32(DeeSTypeObject *__restrict tp_self,
                void *self,
                int32_t *__restrict result) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_math && tp_self->st_math->st_int32)
      return (*tp_self->st_math->st_int32)(orig_type,self,result);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_INT);
}
INTERN int DCALL
DeeStruct_Int64(DeeSTypeObject *__restrict tp_self,
                void *self, int64_t *__restrict result) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_math && tp_self->st_math->st_int64)
      return (*tp_self->st_math->st_int64)(orig_type,self,result);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_INT);
}
INTERN int DCALL
DeeStruct_Double(DeeSTypeObject *__restrict tp_self,
                 void *self, double *__restrict result) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_math && tp_self->st_math->st_double)
      return (*tp_self->st_math->st_double)(orig_type,self,result);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_FLOAT);
}
INTERN DREF DeeObject *DCALL
DeeStruct_Int(DeeSTypeObject *__restrict tp_self, void *self) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_math && tp_self->st_math->st_int)
      return (*tp_self->st_math->st_int)(orig_type,self);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_INT);
 return NULL;
}
#define DEFINE_UNARY_MATH_OPERATOR(Treturn,error_result,DeeStruct_Xxx,st_xxx,OPERATOR_XXX) \
INTERN Treturn DCALL \
DeeStruct_Xxx(DeeSTypeObject *__restrict tp_self, void *self) { \
 DeeSTypeObject *orig_type = tp_self; \
 do { \
  if (tp_self->st_math && tp_self->st_math->st_xxx) \
      return (*tp_self->st_math->st_xxx)(orig_type,self); \
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL && \
           DeeSType_Check(tp_self)); \
 err_unimplemented_operator(orig_type,OPERATOR_XXX); \
 return error_result; \
}
#define DEFINE_BINARY_MATH_OPERATOR(Treturn,error_result,DeeStruct_Xxx,st_xxx,OPERATOR_XXX) \
INTERN Treturn DCALL \
DeeStruct_Xxx(DeeSTypeObject *__restrict tp_self, void *self, \
              DeeObject *__restrict other) { \
 DeeSTypeObject *orig_type = tp_self; \
 do { \
  if (tp_self->st_math && tp_self->st_math->st_xxx) \
      return (*tp_self->st_math->st_xxx)(orig_type,self,other); \
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL && \
           DeeSType_Check(tp_self)); \
 err_unimplemented_operator(orig_type,OPERATOR_XXX); \
 return error_result; \
}
DEFINE_UNARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Inv,st_inv,OPERATOR_INV)
DEFINE_UNARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Pos,st_pos,OPERATOR_POS)
DEFINE_UNARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Neg,st_neg,OPERATOR_NEG)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Add,st_add,OPERATOR_ADD)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Sub,st_sub,OPERATOR_SUB)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Mul,st_mul,OPERATOR_MUL)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Div,st_div,OPERATOR_DIV)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Mod,st_mod,OPERATOR_MOD)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Shl,st_shl,OPERATOR_SHL)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Shr,st_shr,OPERATOR_SHR)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_And,st_and,OPERATOR_AND)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Or,st_or,OPERATOR_OR)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Xor,st_xor,OPERATOR_XOR)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *,NULL,DeeStruct_Pow,st_pow,OPERATOR_POW)
DEFINE_UNARY_MATH_OPERATOR(int,-1,DeeStruct_Inc,st_inc,OPERATOR_INC)
DEFINE_UNARY_MATH_OPERATOR(int,-1,DeeStruct_Dec,st_dec,OPERATOR_DEC)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceAdd,st_inplace_add,OPERATOR_INPLACE_ADD)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceSub,st_inplace_sub,OPERATOR_INPLACE_SUB)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceMul,st_inplace_mul,OPERATOR_INPLACE_MUL)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceDiv,st_inplace_div,OPERATOR_INPLACE_DIV)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceMod,st_inplace_mod,OPERATOR_INPLACE_MOD)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceShl,st_inplace_shl,OPERATOR_INPLACE_SHL)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceShr,st_inplace_shr,OPERATOR_INPLACE_SHR)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceAnd,st_inplace_and,OPERATOR_INPLACE_AND)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceOr,st_inplace_or,OPERATOR_INPLACE_OR)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplaceXor,st_inplace_xor,OPERATOR_INPLACE_XOR)
DEFINE_BINARY_MATH_OPERATOR(int,-1,DeeStruct_InplacePow,st_inplace_pow,OPERATOR_INPLACE_POW)
#undef DEFINE_BINARY_MATH_OPERATOR
#undef DEFINE_UNARY_MATH_OPERATOR
INTERN dhash_t DCALL
DeeStruct_Hash(DeeSTypeObject *__restrict tp_self,
               void *self) {
 return hash_ptr(self,DeeSType_Sizeof(tp_self));
}


INTERN DREF DeeObject *DCALL
DeeStruct_Eq(DeeSTypeObject *__restrict tp_self,
             void *self,
             DeeObject *__restrict some_object) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cmp && tp_self->st_cmp->st_eq)
      return (*tp_self->st_cmp->st_eq)(orig_type,self,some_object);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Compare object data. */
 if (orig_type == (DeeSTypeObject *)Dee_TYPE(some_object))
     return_bool(memcmp(self,DeeStruct_Data(some_object),DeeSType_Sizeof(orig_type)) == 0);
 err_unimplemented_operator(orig_type,OPERATOR_EQ);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_Ne(DeeSTypeObject *__restrict tp_self,
             void *self,
             DeeObject *__restrict some_object) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cmp && tp_self->st_cmp->st_ne)
      return (*tp_self->st_cmp->st_ne)(orig_type,self,some_object);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Compare object data. */
 if (orig_type == (DeeSTypeObject *)Dee_TYPE(some_object))
     return_bool(memcmp(self,DeeStruct_Data(some_object),DeeSType_Sizeof(orig_type)) != 0);
 err_unimplemented_operator(orig_type,OPERATOR_NE);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_Lo(DeeSTypeObject *__restrict tp_self,
             void *self,
             DeeObject *__restrict some_object) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cmp && tp_self->st_cmp->st_lo)
      return (*tp_self->st_cmp->st_lo)(orig_type,self,some_object);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Compare object data. */
 if (orig_type == (DeeSTypeObject *)Dee_TYPE(some_object))
     return_bool(memcmp(self,DeeStruct_Data(some_object),DeeSType_Sizeof(orig_type)) < 0);
 err_unimplemented_operator(orig_type,OPERATOR_LO);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_Le(DeeSTypeObject *__restrict tp_self,
             void *self,
             DeeObject *__restrict some_object) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cmp && tp_self->st_cmp->st_le)
      return (*tp_self->st_cmp->st_le)(orig_type,self,some_object);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Compare object data. */
 if (orig_type == (DeeSTypeObject *)Dee_TYPE(some_object))
     return_bool(memcmp(self,DeeStruct_Data(some_object),DeeSType_Sizeof(orig_type)) <= 0);
 err_unimplemented_operator(orig_type,OPERATOR_LE);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_Gr(DeeSTypeObject *__restrict tp_self,
             void *self,
             DeeObject *__restrict some_object) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cmp && tp_self->st_cmp->st_gr)
      return (*tp_self->st_cmp->st_gr)(orig_type,self,some_object);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Compare object data. */
 if (orig_type == (DeeSTypeObject *)Dee_TYPE(some_object))
     return_bool(memcmp(self,DeeStruct_Data(some_object),DeeSType_Sizeof(orig_type)) > 0);
 err_unimplemented_operator(orig_type,OPERATOR_GR);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_Ge(DeeSTypeObject *__restrict tp_self,
             void *self,
             DeeObject *__restrict some_object) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_cmp && tp_self->st_cmp->st_ge)
      return (*tp_self->st_cmp->st_ge)(orig_type,self,some_object);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Compare object data. */
 if (orig_type == (DeeSTypeObject *)Dee_TYPE(some_object))
     return_bool(memcmp(self,DeeStruct_Data(some_object),DeeSType_Sizeof(orig_type)) >= 0);
 err_unimplemented_operator(orig_type,OPERATOR_GE);
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeStruct_IterSelf(DeeSTypeObject *__restrict tp_self, void *self) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_iter_self)
      return (*tp_self->st_seq->stp_iter_self)(orig_type,self);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_ITERSELF);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_GetSize(DeeSTypeObject *__restrict tp_self, void *self) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_size)
      return (*tp_self->st_seq->stp_size)(orig_type,self);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_SIZE);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_Contains(DeeSTypeObject *__restrict tp_self,
                   void *self, DeeObject *__restrict some_object) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_contains)
      return (*tp_self->st_seq->stp_contains)(orig_type,self,some_object);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_CONTAINS);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeStruct_GetItem(DeeSTypeObject *__restrict tp_self, void *self,
                  DeeObject *__restrict index) {
 DeeSTypeObject *orig_type = tp_self;
 DREF DeeObject *result,*new_result;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_get)
      return (*tp_self->st_seq->stp_get)(orig_type,self,index);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Fallback: Implement getitem as `ind(add)' --> `foo[2]' same as `*(foo + 2)' */
 result = DeeStruct_Add(tp_self,self,index);
 if unlikely(!result) return NULL;
 new_result = DeeObject_Deref(result);
 Dee_Decref(result);
 return new_result;
}
INTERN int DCALL
DeeStruct_DelItem(DeeSTypeObject *__restrict tp_self, void *self,
                  DeeObject *__restrict index) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_del)
      return (*tp_self->st_seq->stp_del)(orig_type,self,index);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Fallback: Do a setitem operation with `none' */
 return DeeStruct_SetItem(tp_self,self,index,Dee_None);
}
INTERN int DCALL
DeeStruct_SetItem(DeeSTypeObject *__restrict tp_self, void *self,
                  DeeObject *__restrict index, DeeObject *__restrict value) {
 DeeSTypeObject *orig_type = tp_self;
 DREF DeeObject *temp,*temp2;
 int result;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_set)
      return (*tp_self->st_seq->stp_set)(orig_type,self,index,value);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Fallback: Implement setitem as
  * `ind(add) := value' --> `foo[2] = value'
  *              same as `*(foo + 2) := value' */
 temp = DeeStruct_Add(tp_self,self,index);
 if unlikely(!temp) goto err;
 temp2 = DeeObject_Deref(temp);
 Dee_Decref(temp);
 if unlikely(!temp2) goto err;
 result = DeeObject_Assign(temp2,value);
 Dee_Decref(temp2);
 return result;
err:
 return -1;
}
INTERN DREF DeeObject *DCALL
DeeStruct_GetRange(DeeSTypeObject *__restrict tp_self, void *self,
                   DeeObject *__restrict begin,
                   DeeObject *__restrict end) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_range_get)
      return (*tp_self->st_seq->stp_range_get)(orig_type,self,begin,end);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 err_unimplemented_operator(orig_type,OPERATOR_GETRANGE);
 return NULL;
}
INTERN int DCALL
DeeStruct_DelRange(DeeSTypeObject *__restrict tp_self, void *self,
                   DeeObject *__restrict begin,
                   DeeObject *__restrict end) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_range_del)
      return (*tp_self->st_seq->stp_range_del)(orig_type,self,begin,end);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_DELRANGE);
}
INTERN int DCALL
DeeStruct_SetRange(DeeSTypeObject *__restrict tp_self, void *self,
                   DeeObject *__restrict begin,
                   DeeObject *__restrict end,
                   DeeObject *__restrict value) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_seq && tp_self->st_seq->stp_range_set)
      return (*tp_self->st_seq->stp_range_set)(orig_type,self,begin,end,value);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_SETRANGE);
}

INTERN DREF DeeObject *DCALL
DeeStruct_GetAttr(DeeSTypeObject *__restrict tp_self,
                  void *self,
                  DeeObject *__restrict name) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_attr && tp_self->st_attr->st_getattr)
      return (*tp_self->st_attr->st_getattr)(orig_type,self,name);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 /* Special attributes: `sizeof' and `alignof' */

 err_unimplemented_operator(orig_type,OPERATOR_GETATTR);
 return NULL;
}

INTERN int DCALL
DeeStruct_DelAttr(DeeSTypeObject *__restrict tp_self,
                  void *self,
                  DeeObject *__restrict name) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_attr && tp_self->st_attr->st_delattr)
      return (*tp_self->st_attr->st_delattr)(orig_type,self,name);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_DELATTR);
}
INTERN int DCALL
DeeStruct_SetAttr(DeeSTypeObject *__restrict tp_self,
                  void *self,
                  DeeObject *__restrict name,
                  DeeObject *__restrict value) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_attr && tp_self->st_attr->st_setattr)
      return (*tp_self->st_attr->st_setattr)(orig_type,self,name,value);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return err_unimplemented_operator(orig_type,OPERATOR_SETATTR);
}
INTERN dssize_t DCALL
DeeStruct_EnumAttr(DeeSTypeObject *__restrict tp_self,
                   denum_t proc, void *arg) {
 DeeSTypeObject *orig_type = tp_self;
 do {
  if (tp_self->st_attr && tp_self->st_attr->st_enumattr)
      return (*tp_self->st_attr->st_enumattr)(orig_type,proc,arg);
 } while ((tp_self = (DeeSTypeObject *)DeeType_Base((DeeTypeObject *)tp_self)) != NULL &&
           DeeSType_Check(tp_self));
 return 0;
}





PRIVATE void DCALL
atype_fini(DeeArrayTypeObject *__restrict self) {
 DeeSTypeObject *orig = self->at_orig;
 /* Delete the weak-link to the original type. */
#ifndef CONFIG_NO_THREADS
 rwlock_write(&orig->st_cachelock);
#endif
 ASSERT(!LIST_ISUNBOUND(self,at_chain));
 LIST_REMOVE(self,at_chain);
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&orig->st_cachelock);
#endif
 Dee_Decref((DeeObject *)orig);
}

PRIVATE struct type_member atype_members[] = {
    TYPE_MEMBER_FIELD("base",STRUCT_OBJECT,offsetof(DeeArrayTypeObject,at_orig)),
    TYPE_MEMBER_FIELD("size",STRUCT_CONST|STRUCT_SIZE_T,offsetof(DeeArrayTypeObject,at_count)),
    TYPE_MEMBER_END
};


PRIVATE DREF DeeObject *DCALL
atype_isarray(DeeArrayTypeObject *__restrict UNUSED(self),
              size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,isarray_name))
     return NULL;
 return_true;
}

PRIVATE struct type_method atype_methods[] = {
    { isarray_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&atype_isarray, DOC_GET(isarray_doc) },
    { NULL }
};

/* Assert that we can re-use some operators from `pointer_type'. */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeArrayTypeObject,at_orig) ==
              COMPILER_OFFSETOF(DeeLValueTypeObject,lt_orig));

INTERN DeeTypeObject DeeArrayType_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"array_type",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSType_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeArrayTypeObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&atype_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&ptype_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */atype_methods,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */atype_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


#ifdef CONFIG_NO_CFUNCTION
/* Throw a NotImplemented error explaining that cfunctions have been disabled. */
INTERN ATTR_COLD void DCALL err_no_cfunction(void) {
 DeeError_Throwf(&DeeError_NotImplemented,
                 "ctypes was built without C-functions being enabled");
}
#endif


#ifndef CONFIG_NO_CFUNCTION
PRIVATE void DCALL
ftype_fini(DeeCFunctionTypeObject *__restrict self) {
 size_t i;
 DeeSTypeObject *orig = self->ft_orig;
 /* Delete the weak-link to the original type. */
#ifndef CONFIG_NO_THREADS
 rwlock_write(&orig->st_cachelock);
#endif
 ASSERT(!LIST_ISUNBOUND(self,ft_chain));
 LIST_REMOVE(self,ft_chain);
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&orig->st_cachelock);
#endif
 Dee_Decref((DeeObject *)orig);
 for (i = 0; i < self->ft_argc; ++i)
     Dee_Decref((DeeObject *)self->ft_argv[i]);
 Dee_Free(self->ft_argv);
 Dee_Free(self->ft_ffi_arg_type_v);
}

PRIVATE void DCALL
ftype_visit(DeeCFunctionTypeObject *__restrict self, dvisit_t proc, void *arg) {
 size_t i;
 Dee_Visit((DeeTypeObject *)self->ft_orig);
 for (i = 0; i < self->ft_argc; ++i)
     Dee_Visit((DeeObject *)self->ft_argv[i]);
}

/* Assert that we can re-use some operators from `pointer_type'. */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeArrayTypeObject,at_orig) ==
              COMPILER_OFFSETOF(DeeLValueTypeObject,lt_orig));
#endif /* !CONFIG_NO_CFUNCTION */

PRIVATE DREF DeeObject *DCALL
ftype_isfunction(DeeCFunctionTypeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,isfunction_name))
     return NULL;
 return_true;
}

PRIVATE struct type_method ftype_methods[] = {
    { isfunction_name+1, (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&ftype_isfunction, DOC_GET(isfunction_name) },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
ftype_args(DeeCFunctionTypeObject *__restrict self) {
#ifdef CONFIG_NO_CFUNCTION
 (void)self;
 err_no_cfunction();
 return NULL;
#else
 /* Use a read-only shared-reference vector to grant the user access to arguments. */
 return DeeRefVector_NewReadonly((DeeObject  *)self,
                                               self->ft_argc,
                                 (DeeObject **)self->ft_argv);

#endif
}

PRIVATE struct type_getset ftype_getsets[] = {
    { "args", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ftype_args, NULL, NULL,
      DOC("->sequence\n"
          "Returns an immutable sequence describing the argument types used by this function") },
    { NULL }
};

INTERN DeeTypeObject DeeCFunctionType_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"function_type",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSType_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeCFunctionTypeObject)
                }
            }
        },
#ifdef CONFIG_NO_CFUNCTION
        /* .tp_dtor        = */NULL,
#else
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ftype_fini,
#endif
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
#ifdef CONFIG_NO_CFUNCTION
    /* .tp_visit         = */NULL,
#else
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&ftype_visit,
#endif
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */ftype_methods,
    /* .tp_getsets       = */ftype_getsets,
    /* .tp_members       = */ptype_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


DECL_END


#endif /* !GUARD_DEX_CTYPES_CORE_C */
