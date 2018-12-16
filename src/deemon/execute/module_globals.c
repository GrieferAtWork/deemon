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
#ifndef GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C
#define GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/none.h>
#include <deemon/map.h>
#include <deemon/string.h>
#include <deemon/module.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/bool.h>
#include <deemon/int.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN


typedef struct {
    OBJECT_HEAD
    DREF DeeModuleObject *mei_module; /* [1..1][const] The module who's exports are being iterated. */
    ATOMIC_DATA uint16_t  mei_index;  /* The current global variable index. */
} ModuleExportsIterator;

typedef struct {
    OBJECT_HEAD
    DREF DeeModuleObject *me_module;  /* [1..1][const] The module who's exports are being viewed. */
} ModuleExports;

INTDEF DeeTypeObject ModuleExports_Type;
INTDEF DeeTypeObject ModuleExportsIterator_Type;
INTDEF DeeTypeObject ModuleGlobals_Type;
INTDEF DeeTypeObject ModuleGlobalsIterator_Type;
INTDEF DREF ModuleExports *DCALL DeeModule_ViewExports(DeeModuleObject *__restrict self);


#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)             (x)->mei_index
#else
#define READ_INDEX(x) ATOMIC_READ((x)->mei_index)
#endif


PRIVATE int DCALL
mei_ctor(ModuleExportsIterator *__restrict self) {
 self->mei_index = 0;
 self->mei_module = &empty_module;
 Dee_Incref(&empty_module);
 return 0;
}

PRIVATE int DCALL
mei_copy(ModuleExportsIterator *__restrict self,
         ModuleExportsIterator *__restrict other) {
 self->mei_index = READ_INDEX(other);
 self->mei_module = other->mei_module;
 Dee_Incref(self->mei_module);
 return 0;
}

PRIVATE int DCALL
mei_init(ModuleExportsIterator *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 ModuleExports *exports_map;
 if (DeeArg_Unpack(argc,argv,"o:_ModuleExportsIterator",&exports_map) ||
     DeeObject_AssertTypeExact((DeeObject *)exports_map,&ModuleExports_Type))
     return -1;
 self->mei_index = 0;
 self->mei_module = exports_map->me_module;
 Dee_Incref(self->mei_module);
 return 0;
}


PRIVATE void DCALL
mei_fini(ModuleExportsIterator *__restrict self) {
 Dee_Decref_unlikely(self->mei_module);
}

PRIVATE void DCALL
mei_visit(ModuleExportsIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->mei_module);
}


#define DEFINE_MEI_COMPARE(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(ModuleExportsIterator *__restrict self, \
     ModuleExportsIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,Dee_TYPE(self))) \
     return NULL; \
 return_bool_(READ_INDEX(self) op READ_INDEX(other)); \
}
DEFINE_MEI_COMPARE(mei_eq,==)
DEFINE_MEI_COMPARE(mei_ne,!=)
DEFINE_MEI_COMPARE(mei_lo,<)
DEFINE_MEI_COMPARE(mei_le,<=)
DEFINE_MEI_COMPARE(mei_gr,>)
DEFINE_MEI_COMPARE(mei_ge,>=)
#undef DEFINE_MEI_COMPARE

PRIVATE struct type_cmp mei_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mei_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mei_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mei_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mei_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mei_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mei_ge,
};

LOCAL DREF DeeObject *DCALL
module_it_getattr_symbol(DeeModuleObject *__restrict self,
                         struct module_symbol *__restrict symbol) {
 DREF DeeObject *result;
 if likely(!(symbol->ss_flags & (MODSYM_FEXTERN|MODSYM_FPROPERTY))) {
read_symbol:
  ASSERT(symbol->ss_index < self->mo_globalc);
#ifndef CONFIG_NO_THREADS
  rwlock_read(&self->mo_lock);
#endif
  result = self->mo_globalv[symbol->ss_index];
  Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->mo_lock);
#endif
  if unlikely(!result)
     result = ITER_DONE; /* Unbound */
  return result;
 }
 /* External symbol, or property. */
 if (symbol->ss_flags & MODSYM_FPROPERTY) {
  DREF DeeObject *callback;
#ifndef CONFIG_NO_THREADS
  rwlock_read(&self->mo_lock);
#endif
  callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
  Dee_XIncref(callback);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->mo_lock);
#endif
  if unlikely(!callback)
     return ITER_DONE;
  /* Invoke the property callback. */
  result = DeeObject_Call(callback,0,NULL);
  Dee_Decref(callback);
  return result;
 }
 /* External symbol. */
 ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
 self = self->mo_importv[symbol->ss_extern.ss_impid];
 goto read_symbol;
}

PRIVATE DREF DeeObject *DCALL
mei_next(ModuleExportsIterator *__restrict self) {
 DREF DeeObject *result,*result_name,*result_value;
 DeeModuleObject *module = self->mei_module;
 uint16_t old_index,new_index;
 DeeModule_LockSymbols(module);
again:
 for (;;) {
  struct module_symbol *symbol;
  old_index = READ_INDEX(self);
  if (old_index > module->mo_bucketm) {
   DeeModule_UnlockSymbols(module);
   return ITER_DONE;
  }
  new_index = old_index;
  for (;;) {
   symbol = &module->mo_bucketv[new_index];
   if (symbol->ss_name) {
    result_name = (DREF DeeObject *)module_symbol_getnameobj(symbol);
    if unlikely(!result_name) {
     DeeModule_UnlockSymbols(module);
     return NULL;
    }
    break;
   }
continue_symbol_search:
   ++new_index;
   if (new_index > module->mo_bucketm) {
    if (!ATOMIC_CMPXCH(self->mei_index,old_index,new_index))
        goto again;
    DeeModule_UnlockSymbols(module);
    return ITER_DONE;
   }
  }
  /* Read the current value of this symbol. */
  result_value = module_it_getattr_symbol(module,symbol);
  if (!ITER_ISOK(result_value)) {
   if unlikely(!result_value) return NULL;
   goto continue_symbol_search;
  }
  if (ATOMIC_CMPXCH(self->mei_index,old_index,new_index+1))
      break;
  Dee_Decref_unlikely(result_value);
 }
 Dee_Incref(result_name);
 DeeModule_UnlockSymbols(module);
 result = DeeTuple_PackSymbolic(2,result_name,result_value);
 if unlikely(!result) {
  Dee_Decref_unlikely(result_name);
  Dee_Decref_unlikely(result_value);
 }
 return result;
}

PRIVATE DREF ModuleExports *DCALL
mei_getseq(ModuleExportsIterator *__restrict self) {
 return DeeModule_ViewExports(self->mei_module);
}

PRIVATE struct type_getset mei_getsets[] = {
    { DeeString_STR(&str_seq), (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mei_getseq },
    { NULL }
};

#undef READ_INDEX
INTERN DeeTypeObject ModuleExportsIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_ModuleExportsIterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&mei_ctor,
                /* .tp_copy_ctor = */&mei_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&mei_init,
                TYPE_FIXED_ALLOCATOR(ModuleExportsIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&mei_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&mei_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&mei_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mei_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */mei_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



PRIVATE int DCALL
me_ctor(ModuleExports *__restrict self) {
 self->me_module = &empty_module;
 Dee_Incref(&empty_module);
 return 0;
}

PRIVATE int DCALL
me_init(ModuleExports *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:_ModuleExports",&self->me_module) ||
     DeeObject_AssertType((DeeObject *)self->me_module,&DeeModule_Type))
     return -1;
 Dee_Incref(&empty_module);
 return 0;
}

STATIC_ASSERT(COMPILER_OFFSETOF(ModuleExportsIterator,mei_module) ==
              COMPILER_OFFSETOF(ModuleExports,me_module));
#define me_fini  mei_fini
#define me_visit mei_visit

PRIVATE int DCALL
me_bool(ModuleExports *__restrict self) {
 return self->me_module->mo_globalc;
}

PRIVATE DREF ModuleExportsIterator *DCALL
me_iter(ModuleExports *__restrict self) {
 DREF ModuleExportsIterator *result;
 result = DeeObject_MALLOC(ModuleExportsIterator);
 if unlikely(!result) goto done;
 result->mei_module = self->me_module;
 result->mei_index  = 0;
 Dee_Incref(result->mei_module);
 DeeObject_Init(result,&ModuleExportsIterator_Type);
done:
 return result;
}

PRIVATE DREF DeeObject *DCALL
me_size(ModuleExports *__restrict self) {
 size_t i,total_symbols = 0;
 DeeModuleObject *module = self->me_module;
 DeeModule_LockSymbols(module);
 for (i = 0; i <= module->mo_bucketm; ++i)
    if (module->mo_bucketv[i].ss_name)
        ++total_symbols;
 DeeModule_UnlockSymbols(module);
 return DeeInt_NewSize(total_symbols);
}

PRIVATE DREF DeeObject *DCALL
me_contains(ModuleExports *__restrict self, DeeObject *__restrict key) {
 bool result;
 DeeModuleObject *module = self->me_module;
 if (!DeeString_Check(key))
      return_false;
 DeeModule_LockSymbols(module);
 result = DeeModule_GetSymbolString(module,
                                    DeeString_STR(key),
                                    DeeString_Hash(key)) != NULL;
 DeeModule_UnlockSymbols(module);
 return_bool_(result);
}

LOCAL DREF DeeObject *DCALL
module_my_getattr_symbol(ModuleExports *__restrict exports_map,
                         DeeModuleObject *__restrict self,
                         struct module_symbol *__restrict symbol) {
 DREF DeeObject *result;
 if likely(!(symbol->ss_flags & (MODSYM_FEXTERN|MODSYM_FPROPERTY))) {
read_symbol:
  ASSERT(symbol->ss_index < self->mo_globalc);
#ifndef CONFIG_NO_THREADS
  rwlock_read(&self->mo_lock);
#endif
  result = self->mo_globalv[symbol->ss_index];
  Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->mo_lock);
#endif
  if unlikely(!result) {
   if (symbol->ss_flags & MODSYM_FNAMEOBJ) {
    err_unknown_key((DeeObject *)exports_map,
                    (DeeObject *)COMPILER_CONTAINER_OF(symbol->ss_name,DeeStringObject,s_str));
   } else {
    err_unknown_key_str((DeeObject *)exports_map,symbol->ss_name);
   }
  }
  return result;
 }
 /* External symbol, or property. */
 if (symbol->ss_flags & MODSYM_FPROPERTY) {
  DREF DeeObject *callback;
#ifndef CONFIG_NO_THREADS
  rwlock_read(&self->mo_lock);
#endif
  callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
  Dee_XIncref(callback);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->mo_lock);
#endif
  if unlikely(!callback) {
   if (symbol->ss_flags & MODSYM_FNAMEOBJ) {
    err_unknown_key((DeeObject *)exports_map,
                    (DeeObject *)COMPILER_CONTAINER_OF(symbol->ss_name,DeeStringObject,s_str));
   } else {
    err_unknown_key_str((DeeObject *)exports_map,symbol->ss_name);
   }
   return NULL;
  }
  /* Invoke the property callback. */
  result = DeeObject_Call(callback,0,NULL);
  Dee_Decref(callback);
  return result;
 }
 /* External symbol. */
 ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
 self = self->mo_importv[symbol->ss_extern.ss_impid];
 goto read_symbol;
}


PRIVATE DREF DeeObject *DCALL
me_get(ModuleExports *__restrict self, DeeObject *__restrict key) {
 DREF DeeObject *result;
 DeeModuleObject *module = self->me_module;
 struct module_symbol *symbol;
 if (!DeeString_Check(key))
      goto unknown_key;
 DeeModule_LockSymbols(module);
 symbol = DeeModule_GetSymbolString(module,
                                    DeeString_STR(key),
                                    DeeString_Hash(key));
 if unlikely(!symbol) {
  DeeModule_UnlockSymbols(module);
unknown_key:
  err_unknown_key((DeeObject *)self,key);
  return NULL;
 }
 result = module_my_getattr_symbol(self,module,symbol);
 DeeModule_UnlockSymbols(module);
 return result;
}

PRIVATE DREF DeeObject *DCALL
me_get_f(ModuleExports *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result; DeeObject *key;
 DeeModuleObject *module = self->me_module;
 struct module_symbol *symbol; DeeObject *defl = Dee_None;
 if (DeeArg_Unpack(argc,argv,"o|o:get",&key,&defl))
     goto err;
 if (!DeeString_Check(key))
      goto unknown_key;
 DeeModule_LockSymbols(module);
 symbol = DeeModule_GetSymbolString(module,
                                    DeeString_STR(key),
                                    DeeString_Hash(key));
 if unlikely(!symbol) {
  DeeModule_UnlockSymbols(module);
unknown_key:
  err_unknown_key((DeeObject *)self,key);
err:
  return NULL;
 }
 result = module_it_getattr_symbol(module,symbol);
 DeeModule_UnlockSymbols(module);
 if (result == ITER_DONE) {
  result = defl;
  Dee_Incref(defl);
 }
 return result;
}

PRIVATE int DCALL
me_del(ModuleExports *__restrict self, DeeObject *__restrict key) {
 int result;
 DeeModuleObject *module = self->me_module;
 struct module_symbol *symbol;
 if (!DeeString_Check(key))
      goto unknown_key;
 DeeModule_LockSymbols(module);
 symbol = DeeModule_GetSymbolString(module,
                                    DeeString_STR(key),
                                    DeeString_Hash(key));
 if unlikely(!symbol) {
  DeeModule_UnlockSymbols(module);
unknown_key:
  err_unknown_key((DeeObject *)self,key);
  return -1;
 }
 result = DeeModule_DelAttrSymbol(module,symbol);
 DeeModule_UnlockSymbols(module);
 return result;
}

PRIVATE int DCALL
me_set(ModuleExports *__restrict self,
       DeeObject *__restrict key,
       DeeObject *__restrict value) {
 int result;
 DeeModuleObject *module = self->me_module;
 struct module_symbol *symbol;
 if (!DeeString_Check(key))
      goto unknown_key;
 DeeModule_LockSymbols(module);
 symbol = DeeModule_GetSymbolString(module,
                                    DeeString_STR(key),
                                    DeeString_Hash(key));
 if unlikely(!symbol) {
  DeeModule_UnlockSymbols(module);
unknown_key:
  err_unknown_key((DeeObject *)self,key);
  return -1;
 }
 result = DeeModule_SetAttrSymbol(module,symbol,value);
 DeeModule_UnlockSymbols(module);
 return result;
}

PRIVATE struct type_seq me_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&me_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&me_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&me_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&me_get,
    /* .tp_del       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&me_del,
    /* .tp_set       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&me_set
};



PRIVATE struct type_method me_methods[] = {
    { DeeString_STR(&str_get), (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&me_get_f,
       DOC("(key,def=!N)\n"
           "@return The value associated with @key or @def when @key has no value associated") },
    { NULL }
};
PRIVATE struct type_member me_members[] = {
    TYPE_MEMBER_FIELD("__module__",STRUCT_OBJECT,
                      COMPILER_OFFSETOF(ModuleExports,me_module)),
    TYPE_MEMBER_END
};
PRIVATE struct type_member me_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&ModuleExportsIterator_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject ModuleExports_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_ModuleExports",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapping_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&me_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&me_init,
                TYPE_FIXED_ALLOCATOR(ModuleExports)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&me_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&me_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&me_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&me_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */me_methods,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */me_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */me_class_members
};


INTERN DREF ModuleExports *DCALL
DeeModule_ViewExports(DeeModuleObject *__restrict self) {
 DREF ModuleExports *result;
 result = DeeObject_MALLOC(ModuleExports);
 if unlikely(!result) goto done;
 result->me_module = self;
 Dee_Incref(self);
 DeeObject_Init(result,&ModuleExports_Type);
done:
 return result;
}





typedef struct {
    OBJECT_HEAD
    DREF DeeModuleObject *mgi_module; /* [1..1] The module who's exports are being viewed. */
    ATOMIC_DATA uint16_t  mgi_index;  /* The current global variable index. */
} ModuleGlobalsIterator;

STATIC_ASSERT(COMPILER_OFFSETOF(ModuleExportsIterator,mei_module) ==
              COMPILER_OFFSETOF(ModuleGlobalsIterator,mgi_module));
STATIC_ASSERT(COMPILER_OFFSETOF(ModuleExportsIterator,mei_index) ==
              COMPILER_OFFSETOF(ModuleGlobalsIterator,mgi_index));
#define mgi_ctor  mei_ctor
#define mgi_copy  mei_copy
#define mgi_fini  mei_fini
#define mgi_visit mei_visit
#define mgi_cmp   mei_cmp


#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)             (x)->mgi_index
#else
#define READ_INDEX(x) ATOMIC_READ((x)->mgi_index)
#endif

typedef struct {
    OBJECT_HEAD
    DREF DeeModuleObject *mg_module; /* [1..1] The module who's exports are being viewed. */
} ModuleGlobals;

INTDEF DREF ModuleGlobals *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self);


PRIVATE int DCALL
mgi_init(ModuleGlobalsIterator *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 ModuleGlobals *globals;
 if (DeeArg_Unpack(argc,argv,"o:_ModuleGlobalsIterator",&globals) ||
     DeeObject_AssertTypeExact((DeeObject *)globals,&ModuleGlobals_Type))
     return -1;
 self->mgi_index = 0;
 self->mgi_module = globals->mg_module;
 Dee_Incref(self->mgi_module);
 return 0;
}


PRIVATE DREF DeeObject *DCALL
mgi_next(ModuleGlobalsIterator *__restrict self) {
 DeeModuleObject *module = self->mgi_module;
 DREF DeeObject *result;
#ifdef CONFIG_NO_THREADS
 uint16_t new_index = self->mgi_index;
 if (new_index >= module->mo_globalc)
     return ITER_DONE;
 while ((result = module->mo_globalv[new_index]) == NULL) {
  ++new_index;
  if (new_index >= module->mo_globalc) {
   self->mgi_index = new_index;
   return ITER_DONE;
  }
 }
 Dee_Incref(result);
 self->mgi_index = new_index;
 return result;
#else
 uint16_t old_index,new_index;
 DeeModule_LockSymbols(module);
again:
 for (;;) {
  old_index = ATOMIC_READ(self->mgi_index);
  if (old_index >= module->mo_globalc) {
   DeeModule_UnlockSymbols(module);
   return ITER_DONE;
  }
  new_index = old_index;
  rwlock_read(&module->mo_lock);
  while ((result = module->mo_globalv[new_index]) == NULL) {
   ++new_index;
   if (new_index >= module->mo_globalc) {
    rwlock_endread(&module->mo_lock);
    if (!ATOMIC_CMPXCH(self->mgi_index,old_index,new_index))
         goto again;
    return ITER_DONE;
   }
  }
  Dee_Incref(result);
  rwlock_endread(&module->mo_lock);
  if (ATOMIC_CMPXCH(self->mgi_index,old_index,new_index+1))
      break;
  Dee_Decref(result);
 }
 DeeModule_UnlockSymbols(module);
 return result;
#endif
}

PRIVATE DREF ModuleGlobals *DCALL
mgi_getseq(ModuleGlobalsIterator *__restrict self) {
 return DeeModule_ViewGlobals(self->mgi_module);
}

PRIVATE struct type_getset mgi_getsets[] = {
    { DeeString_STR(&str_seq), (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mgi_getseq },
    { NULL }
};
#undef READ_INDEX


INTERN DeeTypeObject ModuleGlobalsIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_ModuleGlobalsIterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&mgi_ctor,
                /* .tp_copy_ctor = */&mgi_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&mgi_init,
                TYPE_FIXED_ALLOCATOR(ModuleGlobalsIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&mgi_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&mgi_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&mgi_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mgi_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */mgi_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



PRIVATE int DCALL
mg_init(ModuleGlobals *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:_ModuleGlobals",&self->mg_module) ||
     DeeObject_AssertType((DeeObject *)self->mg_module,&DeeModule_Type))
     return -1;
 Dee_Incref(&empty_module);
 return 0;
}

STATIC_ASSERT(COMPILER_OFFSETOF(ModuleExports,me_module) ==
              COMPILER_OFFSETOF(ModuleGlobals,mg_module));
STATIC_ASSERT(COMPILER_OFFSETOF(ModuleExportsIterator,mei_module) ==
              COMPILER_OFFSETOF(ModuleGlobals,mg_module));
#define mg_ctor    me_ctor
#define mg_fini    mei_fini
#define mg_bool    me_bool
#define mg_visit   mei_visit
#define mg_members me_members


PRIVATE DREF ModuleGlobalsIterator *DCALL
mg_iter(ModuleGlobals *__restrict self) {
 DREF ModuleGlobalsIterator *result;
 result = DeeObject_MALLOC(ModuleGlobalsIterator);
 if unlikely(!result) goto done;
 result->mgi_index  = 0;
 result->mgi_module = self->mg_module;
 Dee_Incref(self->mg_module);
 DeeObject_Init(result,&ModuleGlobalsIterator_Type);
done:
 return result;
}

PRIVATE DREF DeeObject *DCALL
mg_size(ModuleGlobals *__restrict self) {
#ifdef CONFIG_NO_THREADS
 return DeeInt_NewU16(self->mg_module->mo_globalc);
#else
 return DeeInt_NewU16(ATOMIC_READ(self->mg_module->mo_globalc));
#endif
}

PRIVATE DREF DeeObject *DCALL
mg_get(ModuleGlobals *__restrict self,
       DeeObject *__restrict index_ob) {
 size_t index; DREF DeeObject *result;
 DeeModuleObject *module = self->mg_module;
 if (DeeObject_AsSize(index_ob,&index))
     return NULL;
 DeeModule_LockSymbols(module);
 if (index >= module->mo_globalc) {
  size_t num_globals = module->mo_globalc;
  DeeModule_UnlockSymbols(module);
  err_index_out_of_bounds((DeeObject *)self,index,num_globals);
  return NULL;
 }
 rwlock_read(&module->mo_lock);
 result = module->mo_globalv[index];
 if unlikely(!result) {
  rwlock_endread(&module->mo_lock);
  DeeModule_UnlockSymbols(module);
  err_unbound_index((DeeObject *)self,index);
  return NULL;
 }
 Dee_Incref(result);
 rwlock_endread(&module->mo_lock);
 DeeModule_UnlockSymbols(module);
 return result;
}

PRIVATE int DCALL
mg_set(ModuleGlobals *__restrict self,
       DeeObject *__restrict index_ob,
       DeeObject *value) {
 size_t index; DREF DeeObject *result;
 DeeModuleObject *module = self->mg_module;
 if (DeeObject_AsSize(index_ob,&index))
     return -1;
 DeeModule_LockSymbols(module);
 if (index >= module->mo_globalc) {
  size_t num_globals = module->mo_globalc;
  DeeModule_UnlockSymbols(module);
  return err_index_out_of_bounds((DeeObject *)self,index,num_globals);
 }
 rwlock_write(&module->mo_lock);
 Dee_XIncref(value);
 result = module->mo_globalv[index];
 module->mo_globalv[index] = value;
 rwlock_endwrite(&module->mo_lock);
 DeeModule_UnlockSymbols(module);
 Dee_XDecref(result);
 return 0;
}

PRIVATE int DCALL
mg_del(ModuleGlobals *__restrict self,
       DeeObject *__restrict index_ob) {
 return mg_set(self,index_ob,NULL);
}

PRIVATE struct type_seq mg_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mg_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mg_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mg_get,
    /* .tp_del       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&mg_del,
    /* .tp_set       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&mg_set
};

PRIVATE struct type_member mg_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&ModuleGlobalsIterator_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject ModuleGlobals_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_ModuleGlobals",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&mg_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&mg_init,
                TYPE_FIXED_ALLOCATOR(ModuleExports)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&mg_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&mg_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&mg_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&mg_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */mg_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */mg_class_members
};


INTERN DREF ModuleGlobals *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self) {
 DREF ModuleGlobals *result;
 result = DeeObject_MALLOC(ModuleGlobals);
 if unlikely(!result) goto done;
 result->mg_module = self;
 Dee_Incref(self);
 DeeObject_Init(result,&ModuleGlobals_Type);
done:
 return result;
}


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C */
