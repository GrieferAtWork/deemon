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
#ifndef GUARD_DEEMON_EXECUTE_MODULE_C
#define GUARD_DEEMON_EXECUTE_MODULE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/bool.h>
#include <deemon/string.h>
#include <deemon/exec.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/error.h>
#include <deemon/thread.h>
#include <deemon/seq.h>
#ifndef CONFIG_NO_DEX
#include <deemon/dex.h>
#endif

#include <string.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

PUBLIC DREF DeeObject *DCALL
DeeModule_GetRoot(DeeObject *__restrict self) {
 DREF DeeFunctionObject *result;
 DeeModuleObject *me = (DeeModuleObject *)self;
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 /* Check if this module has been loaded. */
 if unlikely(DeeModule_InitImports(self))
    return NULL;
 result = (DREF DeeFunctionObject *)DeeObject_Malloc(offsetof(DeeFunctionObject,fo_refv));
 if unlikely(!result) return NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&me->mo_lock);
#endif
 result->fo_code = me->mo_root;
 Dee_XIncref(result->fo_code);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&me->mo_lock);
#endif
 if (!result->fo_code) {
  result->fo_code = &empty_code;
  Dee_Incref(&empty_code);
 }
 DeeObject_Init(result,&DeeFunction_Type);
 return (DREF DeeObject *)result;
}


PRIVATE char const access_names[3][4] = {
   /* [ATTR_ACCESS_GET] = */"get",
   /* [ATTR_ACCESS_DEL] = */"del",
   /* [ATTR_ACCESS_SET] = */"set"
};
INTERN ATTR_COLD void DCALL
err_module_not_loaded_attr(DeeModuleObject *__restrict self,
                           char const *__restrict name, int access) {
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot %s global variable `%s' of module `%k' that hasn't been loaded yet",
                 access_names[access],name,self->mo_name);
}
INTERN ATTR_COLD void DCALL
err_no_such_global(DeeModuleObject *__restrict self,
                   char const *__restrict name, int access) {
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot %s unknown global variable: `%k.%s'",
                 access_names[access],self->mo_name,name);
}
INTERN ATTR_COLD void DCALL
err_readonly_global(DeeModuleObject *__restrict self,
                    char const *__restrict name) {
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot modify read-only global variable: `%k.%s'",
                 self->mo_name,name);
}

INTERN ATTR_COLD void DCALL
err_cannot_read_property(DeeModuleObject *__restrict self,
                         char const *__restrict name) {
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot read global property: `%k.%s'",
                 self->mo_name,name);
}
INTERN ATTR_COLD void DCALL
err_cannot_delete_property(DeeModuleObject *__restrict self,
                           char const *__restrict name) {
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot write global property: `%k.%s'",
                 self->mo_name,name);
}
INTERN ATTR_COLD void DCALL
err_cannot_write_property(DeeModuleObject *__restrict self,
                          char const *__restrict name) {
 DeeError_Throwf(&DeeError_AttributeError,
                 "Cannot write global property: `%k.%s'",
                 self->mo_name,name);
}

INTERN struct module_symbol empty_module_buckets[] = {
    { NULL, 0, 0 }
};



INTERN struct module_symbol *DCALL
DeeModule_GetSymbolID(DeeModuleObject *__restrict self, uint16_t gid) {
 struct module_symbol *iter,*end;
 struct module_symbol *result = NULL;
 /* Check if the module has actually been loaded yet.
  * This needs to be done to prevent a race condition
  * when reading the bucket fields below, as they only
  * become immutable once this flag has been set. */
 if unlikely(!(self->mo_flags&MODULE_FDIDLOAD))
    return NULL;
 end = (iter = self->mo_bucketv)+(self->mo_bucketm+1);
 for (; iter != end; ++iter) {
  if (!iter->ss_name) continue; /* Skip empty entries. */
  if (iter->ss_index == gid) {
   result = iter;
   /* If it's a symbol, we still stored it's name as
    * the result value, meaning we won't return NULL
    * but still keep on searching for another symbol
    * that isn't an alias. */
   if (!(iter->ss_flags&MODSYM_FALIAS))
         break;
  }
 }
 return result;
}
INTERN struct module_symbol *DCALL
DeeModule_GetSymbolString(DeeModuleObject *__restrict self,
                          char const *__restrict attr_name,
                          dhash_t hash) {
 dhash_t i,perturb;
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (strcmp(DeeString_STR(item->ss_name),attr_name)) continue;
  return item;
 }
 return NULL;
}
INTERN DREF DeeObject *DCALL
module_getattr_symbol(DeeModuleObject *__restrict self,
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
     err_unbound_global(self,symbol->ss_index);
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
   err_cannot_read_property(self,DeeString_STR(symbol->ss_name));
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
INTERN int DCALL
module_boundattr_symbol(DeeModuleObject *__restrict self,
                        struct module_symbol *__restrict symbol) {
 if likely(!(symbol->ss_flags & (MODSYM_FEXTERN|MODSYM_FPROPERTY))) {
  bool result;
read_symbol:
  ASSERT(symbol->ss_index < self->mo_globalc);
#ifndef CONFIG_NO_THREADS
  rwlock_read(&self->mo_lock);
#endif
  result = self->mo_globalv[symbol->ss_index] != NULL;
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->mo_lock);
#endif
  return result;
 }
 /* External symbol, or property. */
 if (symbol->ss_flags & MODSYM_FPROPERTY) {
  DREF DeeObject *callback,*callback_result;
#ifndef CONFIG_NO_THREADS
  rwlock_read(&self->mo_lock);
#endif
  callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
  Dee_XIncref(callback);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->mo_lock);
#endif
  if unlikely(!callback) return 0;
  /* Invoke the property callback. */
  callback_result = DeeObject_Call(callback,0,NULL);
  Dee_Decref(callback);
  if likely(callback_result) {
   Dee_Decref(callback_result);
   return 1;
  }
  if (CATCH_ATTRIBUTE_ERROR())
      return -3;
  if (DeeError_Catch(&DeeError_UnboundAttribute))
      return 0;
  return -1;
 }
 /* External symbol. */
 ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
 self = self->mo_importv[symbol->ss_extern.ss_impid];
 goto read_symbol;
}

LOCAL DREF DeeObject *DCALL
module_getattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 dhash_t i,perturb; DREF DeeObject *result;
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (!strcmp(DeeString_STR(item->ss_name),attr_name))
       return module_getattr_symbol(self,item);
 }
 /* Fallback: Do a generic attribute lookup on the module. */
 result = DeeObject_GenericGetAttrString((DeeObject *)self,attr_name,hash);
 if (result != ITER_DONE) return result;
 err_no_such_global(self,attr_name,ATTR_ACCESS_GET);
 return NULL;
}

LOCAL int DCALL
module_boundattr_impl(DeeModuleObject *__restrict self,
                      char const *__restrict attr_name, dhash_t hash) {
 dhash_t i,perturb;
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (!strcmp(DeeString_STR(item->ss_name),attr_name))
       return module_boundattr_symbol(self,item);
 }
 /* Fallback: Do a generic attribute lookup on the module. */
 return DeeObject_GenericBoundAttrString((DeeObject *)self,attr_name,hash);
}

LOCAL bool DCALL
module_hasattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 dhash_t i,perturb;
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (!strcmp(DeeString_STR(item->ss_name),attr_name))
       return true;
 }
 /* Fallback: Do a generic attribute lookup on the module. */
 return DeeObject_GenericHasAttrString((DeeObject *)self,attr_name,hash);
}

INTERN int DCALL
module_delattr_symbol(DeeModuleObject *__restrict self,
                      struct module_symbol *__restrict symbol) {
 DREF DeeObject *old_value;
 if unlikely(symbol->ss_flags&(MODSYM_FREADONLY|MODSYM_FEXTERN|MODSYM_FPROPERTY)) {
  if (symbol->ss_flags & MODSYM_FREADONLY) {
   err_readonly_global(self,DeeString_STR(symbol->ss_name));
   return -1;
  }
  if (symbol->ss_flags & MODSYM_FPROPERTY) {
   DREF DeeObject *callback,*temp;
#ifndef CONFIG_NO_THREADS
   rwlock_read(&self->mo_lock);
#endif
   ASSERT(symbol->ss_index+MODULE_PROPERTY_DEL < self->mo_globalc);
   callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_DEL];
   Dee_XIncref(callback);
#ifndef CONFIG_NO_THREADS
   rwlock_endread(&self->mo_lock);
#endif
   if unlikely(!callback) {
    err_cannot_delete_property(self,DeeString_STR(symbol->ss_name));
    return -1;
   }
   /* Invoke the property callback. */
   temp = DeeObject_Call(callback,0,NULL);
   Dee_Decref(callback);
   Dee_XDecref(temp);
   return temp ? 0 : -1;
  }
  /* External symbol. */
  ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
  self = self->mo_importv[symbol->ss_extern.ss_impid];
 }
 ASSERT(symbol->ss_index < self->mo_globalc);
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->mo_lock);
#endif
 old_value = self->mo_globalv[symbol->ss_index];
 self->mo_globalv[symbol->ss_index] = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->mo_lock);
#endif
#ifdef CONFIG_ERROR_DELETE_UNBOUND
 if unlikely(!old_value) {
  err_unbound_global(self,symbol->ss_index);
  return -1;
 }
 Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
 Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
 return 0;
}

LOCAL int DCALL
module_delattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 int error; dhash_t i,perturb;
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (!strcmp(DeeString_STR(item->ss_name),attr_name))
       return module_delattr_symbol(self,item);
 }
 /* Fallback: Do a generic attribute lookup on the module. */
 error = DeeObject_GenericDelAttrString((DeeObject *)self,attr_name,hash);
 if unlikely(error <= 0) return error;
 err_no_such_global(self,attr_name,ATTR_ACCESS_DEL);
 return -1;
}

INTERN int DCALL
module_setattr_symbol(DeeModuleObject *__restrict self,
                      struct module_symbol *__restrict symbol,
                      DeeObject *__restrict value) {
 DREF DeeObject *temp;
 if unlikely(symbol->ss_flags&(MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FEXTERN)) {
  if unlikely(symbol->ss_flags & MODSYM_FEXTERN) {
   ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
   self = self->mo_importv[symbol->ss_extern.ss_impid];
  }
  if (symbol->ss_flags & MODSYM_FREADONLY) {
   if (symbol->ss_flags & MODSYM_FPROPERTY)
       goto err_is_readonly;
   ASSERT(symbol->ss_index < self->mo_globalc);
#ifndef CONFIG_NO_THREADS
   rwlock_write(&self->mo_lock);
#endif
   /* Make sure not to allow write-access to global variables that
    * have already been assigned, but are marked as read-only. */
   if unlikely(self->mo_globalv[symbol->ss_index] != NULL) {
#ifndef CONFIG_NO_THREADS
    rwlock_endwrite(&self->mo_lock);
#endif
err_is_readonly:
    err_readonly_global(self,DeeString_STR(symbol->ss_name));
    return -1;
   }
   Dee_Incref(value);
   self->mo_globalv[symbol->ss_index] = value;
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->mo_lock);
#endif
   return 0;
  }
  if (symbol->ss_flags & MODSYM_FPROPERTY) {
   DREF DeeObject *callback;
#ifndef CONFIG_NO_THREADS
   rwlock_write(&self->mo_lock);
#endif
   callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_SET];
   Dee_XIncref(callback);
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->mo_lock);
#endif
   if (!callback) {
    err_cannot_write_property(self,DeeString_STR(symbol->ss_name));
    return -1;
   }
   temp = DeeObject_Call(callback,1,(DeeObject **)&value);
   Dee_Decref(callback);
   Dee_XDecref(temp);
   return temp ? 0 : -1;
  }
  /* External symbol. */
  ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
  self = self->mo_importv[symbol->ss_extern.ss_impid];
 }
 ASSERT(symbol->ss_index < self->mo_globalc);
 Dee_Incref(value);
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->mo_lock);
#endif
 temp = self->mo_globalv[symbol->ss_index];
 self->mo_globalv[symbol->ss_index] = value;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->mo_lock);
#endif
 Dee_XDecref(temp);
 return 0;
}

LOCAL int DCALL
module_setattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash,
                    DeeObject *__restrict value) {
 int error; dhash_t i,perturb;
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (!strcmp(DeeString_STR(item->ss_name),attr_name))
       return module_setattr_symbol(self,item,value);
 }
 /* Fallback: Do a generic attribute lookup on the module. */
 error = DeeObject_GenericSetAttrString((DeeObject *)self,attr_name,hash,value);
 if unlikely(error <= 0) return error;
 err_no_such_global(self,attr_name,ATTR_ACCESS_SET);
 return -1;
}


#ifndef CONFIG_NO_THREADS
INTDEF void DCALL interactivemodule_lockread(DeeModuleObject *__restrict self);
INTDEF void DCALL interactivemodule_lockwrite(DeeModuleObject *__restrict self);
INTDEF void DCALL interactivemodule_lockendread(DeeModuleObject *__restrict self);
INTDEF void DCALL interactivemodule_lockendwrite(DeeModuleObject *__restrict self);
#else
#deifne interactivemodule_lockread(self)     (void)0
#deifne interactivemodule_lockwrite(self)    (void)0
#deifne interactivemodule_lockendread(self)  (void)0
#deifne interactivemodule_lockendwrite(self) (void)0
#endif


INTERN DREF DeeObject *DCALL
DeeModule_GetAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash) {
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  if (DeeInteractiveModule_Check(self)) {
   DREF DeeObject *result;
   interactivemodule_lockread(self);
   result = module_getattr_impl(self,attr_name,hash);
   interactivemodule_lockendread(self);
   return result;
  }
  err_module_not_loaded_attr(self,attr_name,ATTR_ACCESS_GET);
  return NULL;
 }
 return module_getattr_impl(self,attr_name,hash);
}
INTERN int DCALL
DeeModule_BoundAttrString(DeeModuleObject *__restrict self,
                          char const *__restrict attr_name, dhash_t hash) {
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  if (DeeInteractiveModule_Check(self)) {
   int result;
   interactivemodule_lockread(self);
   result = module_boundattr_impl(self,attr_name,hash);
   interactivemodule_lockendread(self);
   return result;
  }
  return -2;
 }
 return module_boundattr_impl(self,attr_name,hash);
}
INTERN bool DCALL
DeeModule_HasAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash) {
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  if (DeeInteractiveModule_Check(self)) {
   bool result;
   interactivemodule_lockread(self);
   result = module_hasattr_impl(self,attr_name,hash);
   interactivemodule_lockendread(self);
   return result;
  }
  return false;
 }
 return module_hasattr_impl(self,attr_name,hash);
}
INTERN int DCALL
DeeModule_DelAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash) {
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  if (DeeInteractiveModule_Check(self)) {
   int result;
   interactivemodule_lockwrite(self);
   result = module_delattr_impl(self,attr_name,hash);
   interactivemodule_lockendwrite(self);
   return result;
  }
  err_module_not_loaded_attr(self,attr_name,ATTR_ACCESS_DEL);
  return -1;
 }
 return module_delattr_impl(self,attr_name,hash);
}
INTERN int DCALL
DeeModule_SetAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash,
                        DeeObject *__restrict value) {
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  if (DeeInteractiveModule_Check(self)) {
   int result;
   interactivemodule_lockwrite(self);
   result = module_setattr_impl(self,attr_name,hash,value);
   interactivemodule_lockendwrite(self);
   return result;
  }
  err_module_not_loaded_attr(self,attr_name,ATTR_ACCESS_SET);
  return -1;
 }
 return module_setattr_impl(self,attr_name,hash,value);
}


PRIVATE void DCALL
err_module_not_loaded(DeeModuleObject *__restrict self) {
 DeeError_Throwf(&DeeError_RuntimeError,
                 "Cannot initialized module `%k' that hasn't been loaded yet",
                 self->mo_name);
}


PUBLIC int DCALL
DeeModule_RunInit(DeeObject *__restrict self) {
 uint16_t flags; DeeThreadObject *caller;
 DeeModuleObject *me = (DeeModuleObject *)self;
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 /* Quick check: Don't do anything else if the module has already been loaded. */
 if (me->mo_flags&MODULE_FDIDINIT)
     return 0;
 /* Make sure not to tinker with an interactive module's root code object. */
 if (me->mo_flags&MODULE_FINITIALIZING &&
     DeeInteractiveModule_Check(self))
     return 0;

 COMPILER_READ_BARRIER();
 caller = DeeThread_Self();
begin_init:
 do {
  flags = ATOMIC_READ(me->mo_flags);
  if (flags&MODULE_FDIDINIT) return 0;
  /* Check if the module has been loaded yet. */
  if (!(flags&MODULE_FDIDLOAD)) {
   if (flags&MODULE_FLOADING) {
#ifdef CONFIG_HOST_WINDOWS
    /* Sleep a bit longer than usually. */
    __NAMESPACE_INT_SYM SleepEx(1000,0);
#else
    SCHED_YIELD();
#endif
    goto begin_init;
   }
   err_module_not_loaded(me);
   return -1; /* Not loaded yet. */
  }
 } while (!ATOMIC_CMPXCH(me->mo_flags,flags,flags|MODULE_FINITIALIZING));
 if (flags&MODULE_FINITIALIZING) {
  /* Module is already being loaded. */
  while ((flags = ATOMIC_READ(me->mo_flags),
         (flags&(MODULE_FINITIALIZING|MODULE_FDIDINIT)) ==
                 MODULE_FINITIALIZING)) {
   /* Check if the module is being loaded in the current thread. */
   if (me->mo_loader == caller)
       return 1;
#ifdef CONFIG_HOST_WINDOWS
   /* Sleep a bit longer than usually. */
   __NAMESPACE_INT_SYM SleepEx(1000,0);
#else
   SCHED_YIELD();
#endif
  }
  /* If the module has now been marked as having finished loading,
   * then simply act as though it was us that did it. */
  if (flags&MODULE_FDIDINIT)
      return 0;
  goto begin_init;
 }
 /* Setup the module to indicate that we're the ones loading it. */
 me->mo_loader = caller;

 /* FIXME: Technically, we'd need to acquire a global lock at this point
  *        and only call `init_function' while hold it in a non-sharing
  *        manner. Otherwise, there is a slim chance for a deadlock:
  *        thread_1:
  *           initialize_module("module_a"):
  *              >> import("module_b"); // Dynamic import to prevent compiler error due to recursion.
  *        thread_2:
  *           initialize_module("module_b"):
  *              >> import("module_a"); // see above...
  *        #1: `thread_1' gets here first and starts to
  *             run the initialization code of `module_a'
  *        #2: `thread_2' gets here second and start to 
  *             run the initialization code of `module_b'
  *        #3: `thread_1' executing `module_a' starts to import `module_b'
  *             It succeeds as the module is already in-cache and calls
  *            `DeeModule_RunInit()' to make sure that the module has been
  *             initialized, entering the idle-wait loop above that is
  *             designed to wait when another thread is already initializing
  *             the same module.
  *        #4: `thread_2' does the same as `thread_1', and enters the wait
  *             loop, idly waiting for `thread_1' to finish initializing
  *            `module_a', which it never will because of the obvious DEADLOCK!
  * NOTE:  This problem can also happen when loading a module, but is just much
  *        more difficult to invoke as it requires execution of multi-threaded
  *        user-code while the compiler is running, but is possible through
  *        GC-callbacks that may be invoked when memory allocation fails during
  *        the compilation process.
  *  >> One sollution would be to put a global, re-en lock around this part,
  *     but I really don't like the idea of that, especially considering how
  *     it would make it impossible to load different modules concurrently
  *     just on the off-chance that they might be importing each other.
  *     However, I also don't wish get rid of dynamic import mechanics, not
  *     only because any exposure to them at any point would lead back to
  *     the problem of this deadlock, but also because it would greatly
  *     hinder efficiency when there was no way of dynamically importing
  *     modules, allowing for runtime checks on the presence of symbols.
  * ... Python could get away with something like that thanks to
  *     its GIL, but I would never stoop so low as to simply say:
  *        "Yeah. We've got thread... Only one of them can ever be
  *         executed at the same time, but it's still multi-threading..." 
  *     me: "NO!!! THIS IS MADNESS AND DEFEATS THE POINT!!!"
  * T0D0: Stop going off-topic...
  */
#ifndef CONFIG_NO_DEX
 if (DeeDex_Check(self)) {
  if (dex_initialize((DeeDexObject *)self))
      goto err;
 } else
#endif
 {
  /* Create and run the module's initializer function. */
  DREF DeeObject *init_function,*init_result;
  DREF DeeObject *argv;
  init_function = DeeModule_GetRoot(self);
  if unlikely(!init_function) goto err;
  /* Call the module's main function with globally registered argv. */
  argv = Dee_GetArgv();
  init_result = DeeObject_Call(init_function,
                               DeeTuple_SIZE(argv),
                               DeeTuple_ELEM(argv));
  Dee_Decref(argv);
  Dee_Decref(init_function);
  if unlikely(!init_result) goto err;
  Dee_Decref(init_result);
 }

 /* Set the did-init flag when we're done now. */
 ATOMIC_FETCHOR(me->mo_flags,MODULE_FDIDINIT);
 return 0;
err:
 ATOMIC_FETCHAND(me->mo_flags,~(MODULE_FINITIALIZING));
 return -1;
}


PUBLIC int (DCALL DeeModule_InitImports)(DeeObject *__restrict self) {
 DeeModuleObject *me = (DeeModuleObject *)self;
 size_t i; uint16_t flags;
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 flags = ATOMIC_READ(me->mo_flags);
 /* Quick check: When the did-init flag has been set, we can
  *              assume that all other modules imported by
  *              this one have also been initialized. */
 if (flags&MODULE_FDIDINIT) return 0;
 /* Make sure not to tinker with the imports of an interactive module. */
 if (flags&MODULE_FINITIALIZING &&
     DeeInteractiveModule_Check(self))
     return 0;
 /* Make sure the module has been loaded. */
 if unlikely(!(flags&MODULE_FDIDLOAD)) {
  /* The module hasn't been loaded yet. */
  err_module_not_loaded(me);
  return -1;
 }
 /* Go though and run initializers for all imported modules. */
 for (i = 0; i < me->mo_importc; ++i) {
  DeeModuleObject *import = me->mo_importv[i];
  ASSERT_OBJECT_TYPE(import,&DeeModule_Type);
  if unlikely(DeeModule_RunInit((DeeObject *)import) < 0)
     return -1;
 }
 return 0;
}


PUBLIC /*String*/DeeObject *DCALL
DeeModule_GlobalName(DeeObject *__restrict self, uint16_t gid) {
 struct module_symbol *sym;
 sym = DeeModule_GetSymbolID((DeeModuleObject *)self,gid);
 return sym ? (DeeObject *)sym->ss_name : NULL;
}


PRIVATE DREF DeeObject *DCALL
module_str(DeeModuleObject *__restrict self) {
 return_reference_((DeeObject *)self->mo_name);
}
PRIVATE DREF DeeObject *DCALL
module_repr(DeeModuleObject *__restrict self) {
 return DeeString_Newf("import(%r)",self->mo_name);
}


INTERN DREF DeeObject *DCALL
module_getattr(DeeModuleObject *__restrict self,
               /*String*/DeeObject *__restrict name) {
 return DeeModule_GetAttrString(self,
                                DeeString_STR(name),
                                DeeString_Hash(name));
}
INTERN int DCALL
module_delattr(DeeModuleObject *__restrict self,
               /*String*/DeeObject *__restrict name) {
 return DeeModule_DelAttrString(self,
                                DeeString_STR(name),
                                DeeString_Hash(name));
}
INTERN int DCALL
module_setattr(DeeModuleObject *__restrict self,
               /*String*/DeeObject *__restrict name,
               DeeObject *__restrict value) {
 return DeeModule_SetAttrString(self,
                                DeeString_STR(name),
                                DeeString_Hash(name),value);
}
PRIVATE dssize_t DCALL
module_enumattr(DeeTypeObject *__restrict UNUSED(tp_self),
                DeeModuleObject *__restrict self, denum_t proc, void *arg) {
 struct module_symbol *iter,*end,*doc_iter;
 dssize_t temp,result = 0;
 ASSERT_OBJECT(self);
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  if (DeeInteractiveModule_Check(self)) {
   /* TODO: Special handling for enumerating the globals of an interactive module. */
  }
  return 0;
 }
 end = (iter = self->mo_bucketv)+(self->mo_bucketm+1);
 for (; iter != end; ++iter) {
  uint16_t perm; DREF DeeTypeObject *attr_type;
  /* Skip empty and hidden entries. */
  if (!iter->ss_name || (iter->ss_flags&MODSYM_FHIDDEN)) continue;
  perm = ATTR_IMEMBER | ATTR_ACCESS_GET | ATTR_NAMEOBJ;
  ASSERT(iter->ss_index < self->mo_globalc);
  if (!(iter->ss_flags&MODSYM_FREADONLY))
        perm |= (ATTR_ACCESS_DEL|ATTR_ACCESS_SET);
  if (iter->ss_flags&MODSYM_FPROPERTY) {
   perm &= ~(ATTR_ACCESS_GET|ATTR_ACCESS_DEL|ATTR_ACCESS_SET);
   perm |= ATTR_PROPERTY;
  }
  attr_type = NULL;
#if 0 /* Always allow this! (we allow it for user-classes, as well!) */
  /* For constant-expression symbols, we can predict
   * their type (as well as their value)... */
  if (iter->ss_flags&MODSYM_FCONSTEXPR)
#endif
  {
   DeeObject *symbol_object;
   if unlikely(DeeModule_RunInit((DeeObject *)self) < 0) return -1;
   if (self->mo_flags&MODULE_FDIDINIT) {
#ifndef CONFIG_NO_THREADS
    rwlock_read(&self->mo_lock);
#endif
    if (perm & ATTR_PROPERTY) {
     /* Check which property operations have been bound. */
     if (self->mo_globalv[iter->ss_index + MODULE_PROPERTY_GET])
         perm |= ATTR_ACCESS_GET;
     if (!(iter->ss_flags&MODSYM_FREADONLY)) {
      /* These callbacks are only allocated if the READONLY flag isn't set. */
      if (self->mo_globalv[iter->ss_index + MODULE_PROPERTY_DEL])
          perm |= ATTR_ACCESS_DEL;
      if (self->mo_globalv[iter->ss_index + MODULE_PROPERTY_SET])
          perm |= ATTR_ACCESS_SET;
     }
    } else {
     symbol_object = self->mo_globalv[iter->ss_index];
     if (symbol_object) {
      attr_type = Dee_TYPE(symbol_object);
      Dee_Incref(attr_type);
     }
    }
#ifndef CONFIG_NO_THREADS
    rwlock_endread(&self->mo_lock);
#endif
   }
  }
  doc_iter = iter;
  if (!doc_iter->ss_doc && (doc_iter->ss_flags & MODSYM_FALIAS)) {
   doc_iter = DeeModule_GetSymbolID(self,doc_iter->ss_index);
   ASSERT(doc_iter);
  }
  /* NOTE: Pass the module instance as declarator! */
  if (doc_iter->ss_doc) perm |= ATTR_DOCOBJ;
  temp = (*proc)((DeeObject *)self,iter->ss_name->s_str,
                  doc_iter->ss_doc ? doc_iter->ss_doc->s_str : NULL,
                  perm,attr_type,arg);
  Dee_XDecref(attr_type);
  if unlikely(temp < 0) goto err;
  result += temp;
 }
 temp = DeeObject_GenericEnumAttr(&DeeModule_Type,proc,arg);
 if unlikely(temp < 0) goto err;
 return result + temp;
err:
 return temp;
}

PRIVATE struct type_attr module_attr = {
    /* .tp_getattr  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&module_getattr,
    /* .tp_delattr  = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&module_delattr,
    /* .tp_setattr  = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&module_setattr,
    /* .tp_enumattr = */(dssize_t(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict,denum_t,void*))&module_enumattr
};

PRIVATE int DCALL
err_module_not_fully_loaded(DeeModuleObject *__restrict self) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "Module %k has not been fully loaded, yet",
                        self->mo_name);
}


PRIVATE DREF DeeObject *DCALL
module_get_code(DeeModuleObject *__restrict self) {
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  err_module_not_fully_loaded(self);
  return NULL;
 }
 return_reference_((DREF DeeObject *)self->mo_root);
}

PRIVATE DREF DeeObject *DCALL
module_get_path(DeeModuleObject *__restrict self) {
 if (!(self->mo_flags & MODULE_FDIDLOAD)) {
  err_module_not_fully_loaded(self);
  return NULL;
 }
 if (!self->mo_path) {
  err_unbound_attribute(&DeeModule_Type,"__path__");
  return NULL;
 }
 return_reference_((DREF DeeObject *)self->mo_path);
}

PRIVATE DREF DeeObject *DCALL
module_get_imports(DeeModuleObject *__restrict self) {
 return DeeRefVector_NewReadonly((DeeObject *)self,self->mo_importc,
                                 (DeeObject **)self->mo_importv);
}

PRIVATE struct type_member module_members[] = {
    TYPE_MEMBER_FIELD_DOC("__name__",STRUCT_OBJECT,
                          offsetof(DeeModuleObject,mo_name),
                          "->string\nThe name of @this module"),
    TYPE_MEMBER_END
};

INTDEF DREF DeeObject *DCALL
DeeModule_ViewExports(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL
DeeModule_ViewGlobals(DeeObject *__restrict self);

PRIVATE struct type_getset module_getsets[] = {
    { "__exports__", &DeeModule_ViewExports, NULL, NULL,
      DOC("->{(string,object)...}\n"
          "Returns a modifyable mapping-like object containing @this "
          "module's global variables accessible by name (and enumerable)\n"
          "Note that only existing exports can be modified, however no new symbols can be added:\n"
          ">import util;\n"
          ">print util.min;                // function\n"
          ">print util.__exports__[\"min\"]; // function\n"
          ">del util.min;\n"
          ">assert \"min\" !in util.__exports__;\n"
          ">util.__exports__[\"min\"] = 42;\n"
          ">print util.min;                // 42") },
    { "__imports__", (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_imports, NULL, NULL,
      DOC("->{module...}\n"
          "Returns an immutable sequence of all other modules imported by this one") },
    { "__globals__", &DeeModule_ViewGlobals, NULL, NULL,
      DOC("->sequence\n"
          "Similar to #__exports__, however global variables are addressed using their "
          "internal index. Using this, anonymous global variables (such as property callbacks) "
          "can be accessed and modified") },
    { "__code__",
     (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_code, NULL, NULL,
      DOC("->code\n"
          "@throw ValueError The module hasn't been fully loaded\n"
          "Returns the code object for the module's root initializer") },
    { "__path__",
     (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_path, NULL, NULL,
      DOC("->string\n"
          "@throw ValueError The module hasn't been fully loaded\n"
          "@throw AttributeError The module wasn't accessed through the filesystem\n"
          "Returns the absolute filesystem path of the module's source file") },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
module_class_getpath(DeeObject *__restrict UNUSED(self)) {
 return_reference(DeeModule_GetPath());
}
PRIVATE int DCALL
module_class_setpath(DeeObject *__restrict UNUSED(self),
                     DeeObject *__restrict value) {
 return DeeObject_Assign(DeeModule_GetPath(),value);
}

PRIVATE struct type_getset module_class_getsets[] = {
    { "path", &module_class_getpath, NULL, &module_class_setpath,
      DOC("->list\n"
          "A list of strings describing the search path for system libraries") },
    /* Deprecated aliases to emulate the old `dexmodule' builtin type. */
    { "search_path", &module_class_getpath, NULL, &module_class_setpath,
      DOC("->list\n"
          "Deprecated alias for #path") },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
module_class_open(DeeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject **__restrict argv) {
 /* This is pretty much the same as the builtin `import()' function.
  * The only reason it exist is to be a deprecated alias for backwards
  * compatibility with the old deemon. */
 DeeObject *module_name;
 if (DeeArg_Unpack(argc,argv,"o:open",&module_name) ||
     DeeObject_AssertTypeExact(module_name,&DeeString_Type))
     return NULL;
 return DeeModule_Import(module_name,NULL,true);
}


PRIVATE struct type_method module_class_methods[] = {
    /* Deprecated aliases to emulate the old `dexmodule' builtin type. */
    { "open", &module_class_open, DOC("(string name)->module\nDeprecated alias for :import") },
    { NULL }
};


INTDEF void DCALL module_unbind(DeeModuleObject *__restrict self);

PRIVATE void DCALL
module_fini(DeeModuleObject *__restrict self) {
 struct module_symbol *iter,*end;
 DREF DeeModuleObject **miter,**mend; size_t i;
 module_unbind(self);
 Dee_Decref(self->mo_name);
 Dee_XDecref(self->mo_root);
 Dee_XDecref(self->mo_path);
 if (self->mo_flags&MODULE_FDIDLOAD) {
  for (i = 0; i < self->mo_globalc; ++i)
       Dee_XDecref(self->mo_globalv[i]);
  Dee_Free(self->mo_globalv);
 }
 if (self->mo_bucketv != empty_module_buckets) {
  end = (iter = self->mo_bucketv)+self->mo_bucketm+1;
  for (; iter != end; ++iter) {
   Dee_XDecref(iter->ss_name);
   Dee_XDecref(iter->ss_doc);
  }
  Dee_Free((void *)self->mo_bucketv);
 }

 mend = (miter = (DREF DeeModuleObject **)self->mo_importv)+self->mo_importc;
 for (; miter != mend; ++miter) Dee_Decref(*miter);
 Dee_Free((void *)self->mo_importv);
}

PRIVATE void DCALL
module_visit(DeeModuleObject *__restrict self,
             dvisit_t proc, void *arg) {
 size_t i;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->mo_lock);
#endif

 /* Visit the root-code object. */
 Dee_XVisit(self->mo_root);

 /* Visit global variables. */
 if (self->mo_flags&MODULE_FDIDLOAD) {
  for (i = 0; i < self->mo_globalc; ++i)
       Dee_XVisit(self->mo_globalv[i]);
 }

#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->mo_lock);
#endif
 /* Visit imported modules. */
 for (i = 0; i < self->mo_importc; ++i)
      Dee_XVisit(self->mo_importv[i]);
}

PRIVATE void DCALL
module_clear(DeeModuleObject *__restrict self) {
 DREF DeeObject *buffer[16],**iter = buffer,**iter2;
 DREF DeeCodeObject *root_code; size_t i;
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->mo_lock);
#endif
 root_code = self->mo_root;
 self->mo_root = NULL;
restart:
 i = self->mo_globalc;
 while (i--) {
  /* Operate in reverse order, better mirrors the
   * (likely) order in which stuff was initialized in. */
  DREF DeeObject *ob = self->mo_globalv[i];
  if (ob == NULL) continue;
  self->mo_globalv[i] = NULL;
  if (!Dee_DecrefIfNotOne(ob)) {
   *iter++ = ob;
   if (iter == COMPILER_ENDOF(buffer)) {
#ifndef CONFIG_NO_THREADS
    rwlock_endwrite(&self->mo_lock);
#endif
    for (iter2 = buffer; iter2 < iter; ++iter2)
        Dee_Decref(*iter2);
    iter = buffer;
#ifndef CONFIG_NO_THREADS
    rwlock_write(&self->mo_lock);
#endif
    goto restart;
   }
  }
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->mo_lock);
#endif
 for (iter2 = buffer; iter2 < iter; ++iter2)
     Dee_Decref(*iter2);
 Dee_XDecref(root_code);
}

PRIVATE void DCALL
module_pclear(DeeModuleObject *__restrict self,
              unsigned int priority) {
 size_t i;
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->mo_lock);
#endif
 i = self->mo_globalc;
 while (i--) {
  /* Operate in reverse order, better mirrors the
   * (likely) order in which stuff was initialized in. */
  DREF DeeObject *ob = self->mo_globalv[i];
  if (ob == NULL) continue;
  if (DeeObject_GCPriority(ob) < priority)
      continue; /* Clear this object in a later pass. */
  self->mo_globalv[i] = NULL;
  /* Temporarily unlock the module to immediately
   * destroy a global variable when the priority
   * level isn't encompassing _all_ objects, yet. */
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->mo_lock);
#endif
  Dee_Decref(ob);
#ifndef CONFIG_NO_THREADS
  rwlock_write(&self->mo_lock);
#endif
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->mo_lock);
#endif
}

PRIVATE int DCALL
module_init(DeeModuleObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 /* Clear out module-specific data. */
 memset(DeeObject_DATA(self),0,
        sizeof(DeeModuleObject)-
        sizeof(DeeObject));
 if (DeeArg_Unpack(argc,argv,"o:module",&self->mo_name) ||
     DeeObject_AssertTypeExact((DeeObject *)self->mo_name,&DeeString_Type))
     return -1;
 self->mo_bucketv = empty_module_buckets;
 Dee_Incref(self->mo_name);
 weakref_support_init(self);
 return 0;
}

INTERN struct type_gc module_gc = {
    /* .tp_clear  = */(void(DCALL *)(DeeObject *__restrict))&module_clear,
    /* .tp_pclear = */(void(DCALL *)(DeeObject *__restrict,unsigned int))&module_pclear,
    /* .tp_gcprio = */GC_PRIORITY_MODULE
};



/* Single module objects are unique, comparing/hashing
 * them is as single as comparing their memory locations. */
PRIVATE dhash_t DCALL
module_hash(DeeModuleObject *__restrict self) {
 return DeeObject_HashGeneric(self);
}
PRIVATE DREF DeeObject *DCALL
module_eq(DeeModuleObject *__restrict self,
          DeeModuleObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,&DeeModule_Type))
     return NULL;
 return_bool_(self == other);
}
PRIVATE DREF DeeObject *DCALL
module_ne(DeeModuleObject *__restrict self,
          DeeModuleObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,&DeeModule_Type))
     return NULL;
 return_bool_(self != other);
}

PRIVATE struct type_cmp module_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&module_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&module_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&module_ne
};

PUBLIC DeeTypeObject DeeModule_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_module),
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FGC|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */WEAKREF_SUPPORT_ADDR(DeeModuleObject),
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&module_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeModuleObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&module_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&module_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&module_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&module_visit,
    /* .tp_gc            = */&module_gc,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&module_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */&module_attr,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */module_getsets,
    /* .tp_members       = */module_members,
    /* .tp_class_methods = */module_class_methods,
    /* .tp_class_getsets = */module_class_getsets,
    /* .tp_class_members = */NULL
};

INTERN struct static_module_struct empty_module_head = {
    {
        /* ... */
        NULL,
        NULL
    },{
        OBJECT_HEAD_INIT(&DeeModule_Type),
        /* .mo_name      = */(DeeStringObject *)Dee_EmptyString,
        /* .mo_pself     = */NULL,
        /* .mo_next      = */NULL,
        /* .mo_path      = */NULL,
        /* .mo_globpself = */NULL,
        /* .mo_globnext  = */NULL,
        /* .mo_importc   = */0,
        /* .mo_globalc   = */0,
#ifndef CONFIG_NO_DEC
        /* .mo_flags     = */MODULE_FDIDINIT|MODULE_FDIDLOAD|MODULE_FHASCTIME,
#else
        /* .mo_flags     = */MODULE_FDIDINIT|MODULE_FDIDLOAD,
#endif
        /* .mo_bucketm   = */0,
        /* .mo_bucketv   = */empty_module_buckets,
        /* .mo_importv   = */NULL,
        /* .mo_globalv   = */NULL,
        /* .mo_root      = */&empty_code,
#ifndef CONFIG_NO_THREADS
        /* .mo_lock      = */RWLOCK_INIT,
        /* .mo_loader    = */NULL,
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEC
        /* .mo_ctime     = */0,
#endif /* !CONFIG_NO_DEC */
        WEAKREF_SUPPORT_INIT
    }
};


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_C */
