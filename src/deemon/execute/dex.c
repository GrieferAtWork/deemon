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
#ifndef GUARD_DEEMON_EXECUTE_DEX_C
#define GUARD_DEEMON_EXECUTE_DEX_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/dex.h>

#ifndef CONFIG_NO_DEX
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif
#ifndef CONFIG_NO_DEC
#include <deemon/dec.h>
#endif
#include <hybrid/limits.h>

#include <string.h>

#include "../runtime/runtime_error.h"

#if defined(CONFIG_HOST_WINDOWS) && \
   !defined(__CYGWIN__)
/* NOTE: Don't use LoadLibrary() on cygwin. It does some crazy hacking
 *       to get fork() working properly with dynamic linking, so better
 *       not interfere with it by bypassing its mechanisms. */
#define USE_LOADLIBRARY
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef _WIN32_WCE
#undef  GetProcAddress
#define GetProcAddress GetProcAddressA
#endif

DECL_BEGIN

INTDEF struct module_symbol empty_module_buckets[];

INTERN int DCALL
dex_load_file(DeeDexObject *__restrict self,
              DeeObject *__restrict input_file) {
 struct module_symbol *modsym;
 struct dex_symbol *symbols; struct dex *descriptor;
 DREF DeeObject **globals; DREF DeeModuleObject **imports;
 size_t symcount,impcount; uint16_t symi,bucket_mask;
#ifdef USE_LOADLIBRARY
 HMODULE handle;
#else
 void *handle;
#endif
#ifdef USE_LOADLIBRARY
 LPWSTR name = (LPWSTR)DeeString_AsWide(input_file);
 if unlikely(!name) return -1;
 DBG_ALIGNMENT_DISABLE();
 handle = LoadLibraryW((LPCWSTR)name);
#else
 DBG_ALIGNMENT_DISABLE();
 handle = dlopen(DeeString_STR(input_file),
                 RTLD_LOCAL|
#ifdef RTLD_LAZY
                 RTLD_LAZY
#else
                 RTLD_NOW
#endif
                 );
#endif
 if (!handle) {
#ifdef USE_LOADLIBRARY
  /* TODO: Check against GetLastError() for the reason why. */
  DWORD error;
  error = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  DEE_DPRINTF("error = %u (%r)\n",error,input_file);
#endif
  return 1;
 }
#ifdef USE_LOADLIBRARY
 descriptor = (struct dex *)GetProcAddress(handle,"DEX");
 if (!descriptor) descriptor = (struct dex *)GetProcAddress(handle,"_DEX");
#else
 descriptor = (struct dex *)dlsym(handle,"DEX");
 if (!descriptor) descriptor = (struct dex *)dlsym(handle,"_DEX");
#endif
 DBG_ALIGNMENT_ENABLE();
 if (!descriptor) {
  DeeError_Throwf(&DeeError_RuntimeError,
                  "Dex extension %r does not export a descriptor table",
                  input_file);
  goto err;
 }
 self->d_handle = handle;
 self->d_dex    = descriptor;
 /* Load the extension's import vector. */
 impcount = 0,imports = NULL;
 if (descriptor->d_import_names &&
    *descriptor->d_import_names) {
  size_t i;
  char **names = (char **)descriptor->d_import_names;
  while (*names) ++impcount,++names;
  if unlikely(impcount > UINT16_MAX) {
   DeeError_Throwf(&DeeError_RuntimeError,
                   "Dex extension %r has too many imports",
                   input_file);
   goto err;
  }
  imports = (DREF DeeModuleObject **)Dee_Malloc(impcount*
                                                sizeof(DREF DeeModuleObject *));
  if unlikely(!imports) goto err;
  names = (char **)descriptor->d_import_names;
  /* Load import modules, using the same index as the original name. */
  for (i = 0; i < impcount; ++i) {
   DREF DeeObject *import;
   import = DeeModule_OpenString(names[i],NULL,true);
   if unlikely(!import) {
    while (i--) Dee_Decref(imports[i]);
    goto err_imp;
   }
   imports[i] = (DREF DeeModuleObject *)import;
  }
 }

 /* Load the extension's symbol table. */
 symbols = descriptor->d_symbols;
 symcount = 0;
 if (symbols) while (symbols->ds_name) ++symbols,++symcount;
 if unlikely(symcount > UINT16_MAX) {
  DeeError_Throwf(&DeeError_RuntimeError,
                  "Dex extension %r is too large",
                  input_file);
  goto err_imp_elem;
 }
 /* Generate the global variable table. */
 symbols = descriptor->d_symbols;
 globals = (DREF DeeObject **)Dee_Malloc(symcount*sizeof(DREF DeeObject *));
 if unlikely(!globals) goto err_imp_elem;
 /* Figure out how large the hash-mask should be. */
 bucket_mask = 1;
 while (bucket_mask < symcount) bucket_mask <<= 1;
 if ((bucket_mask-symcount) < 16) bucket_mask <<= 1;
 --bucket_mask;
 modsym = (struct module_symbol *)Dee_Calloc((bucket_mask+1)*
                                              sizeof(struct module_symbol));
 if unlikely(!modsym) goto err_glob;
 /* Set the symbol table and global variable vector. */
 for (symi = 0; symi < (uint16_t)symcount; ++symi) {
  struct dex_symbol *sym = &symbols[symi];
  dhash_t i,perturb,hash;
  DREF DeeObject *name_ob,*doc_ob;
  ASSERT(sym->ds_name);
  ASSERTF(!sym->ds_obj || DeeObject_DoCheck(sym->ds_obj),
          "Invalid object %p exported: `%s' by `%s'",
          sym->ds_obj,sym->ds_name,DeeString_STR(input_file));
  /* Create objects for the symbol's name and documentation.
   * XXX: Create doc-strings lazily? */
  name_ob = DeeString_NewAuto(sym->ds_name);
  if unlikely(!name_ob) goto err_glob_elem;
  doc_ob = NULL;
  if (sym->ds_doc) {
   doc_ob = DeeString_NewUtf8(sym->ds_doc,
                              strlen(sym->ds_doc),
                              STRING_ERROR_FIGNORE);
   if unlikely(!doc_ob) { Dee_Decref(name_ob); goto err_glob_elem; }
  }
  hash    = DeeString_Hash(name_ob);
  perturb = i = hash & bucket_mask;
  for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
   struct module_symbol *target = &modsym[i & bucket_mask];
   if (target->ss_name) continue;
   target->ss_name  = (DREF DeeStringObject *)name_ob;
   target->ss_doc   = (DREF DeeStringObject *)doc_ob;
   target->ss_index = symi;
   target->ss_hash  = hash;
   target->ss_flags = (uint16_t)sym->ds_flags;
   break;
  }
  /* Safe the proper initialization object in the global table. */
  globals[symi] = sym->ds_obj;
  Dee_XIncref(sym->ds_obj);
 }
 /* Write the tables into the module descriptor. */
 self->d_module.mo_importc = (uint16_t)impcount;
 self->d_module.mo_importv = imports;
 self->d_module.mo_globalc = (uint16_t)symcount;
 self->d_module.mo_globalv = globals;
 self->d_module.mo_bucketm = bucket_mask;
 self->d_module.mo_bucketv = modsym;
 /* Save the import table in the descriptor. */
 descriptor->d_imports     = (DeeObject **)imports;
 return 0;
err_glob_elem:
 /* Cleanup global variables. */
 while (symi--)
     Dee_XDecrefNokill(globals[symi]);
 /* Cleanup symbol names. */
 for (symi = 0; symi <= bucket_mask; ++symi) {
  Dee_XDecref(modsym[symi].ss_doc);
  Dee_XDecref(modsym[symi].ss_name);
 }
 Dee_Free(modsym);
err_glob:
 Dee_Free(globals);
err_imp_elem:
 while (impcount--)
     Dee_Decref(imports[impcount]);
err_imp:
 Dee_Free(imports);
err:
 DBG_ALIGNMENT_DISABLE();
#ifdef USE_LOADLIBRARY
 FreeLibrary(handle);
#else
 dlclose(handle);
#endif
 DBG_ALIGNMENT_ENABLE();
 return -1;
}

PUBLIC void *DCALL
DeeModule_GetNativeSymbol(DeeObject *__restrict self,
                          char const *__restrict name) {
 void *result;
 DeeDexObject *me = (DeeDexObject *)self;
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 if (!DeeDex_Check(self) || !(me->d_module.mo_flags&MODULE_FDIDLOAD))
      return NULL;
#ifdef USE_LOADLIBRARY
 DBG_ALIGNMENT_DISABLE();
 result = GetProcAddress((HMODULE)me->d_handle,name);
 DBG_ALIGNMENT_ENABLE();
 if (!result) {
  /* Try again after prepending an underscore. */
  if (((uintptr_t)(name  )&~(PAGESIZE-1)) ==
      ((uintptr_t)(name-1)&~(PAGESIZE-1)) &&
        name[-1] == '_') {
   DBG_ALIGNMENT_DISABLE();
   result = GetProcAddress((HMODULE)me->d_handle,name-1);
   DBG_ALIGNMENT_ENABLE();
  } else {
   char *temp_name; size_t namelen = strlen(name);
#ifdef Dee_AMallocNoFail
   Dee_AMallocNoFail(temp_name,(namelen+2)*sizeof(char));
#else
   temp_name = (char *)Dee_AMalloc((namelen+2)*sizeof(char));
   if unlikely(!temp_name)
      return NULL; /* ... Technically not correct, but if memory has gotten
                    *     this low, that's the last or the user's problems. */
#endif
   memcpy(temp_name+1,name,(namelen+1)*sizeof(char));
   temp_name[0] = '_';
   DBG_ALIGNMENT_DISABLE();
   result = GetProcAddress((HMODULE)me->d_handle,temp_name);
   DBG_ALIGNMENT_ENABLE();
   Dee_AFree(temp_name);
  }
 }
#else
 DBG_ALIGNMENT_DISABLE();
 result = dlsym(me->d_handle,name);
 DBG_ALIGNMENT_ENABLE();
 if (!result) {
  /* Try again after prepending an underscore. */
  if (((uintptr_t)(name  )&~(PAGESIZE-1)) ==
      ((uintptr_t)(name-1)&~(PAGESIZE-1)) &&
        name[-1] == '_') {
   DBG_ALIGNMENT_DISABLE();
   result = dlsym(me->d_handle,name-1);
   DBG_ALIGNMENT_ENABLE();
  } else {
   char *temp_name; size_t namelen = strlen(name);
#ifdef Dee_AMallocNoFail
   Dee_AMallocNoFail(temp_name,(namelen+2)*sizeof(char));
#else
   temp_name = (char *)Dee_AMalloc((namelen+2)*sizeof(char));
   if unlikely(!temp_name)
      return NULL; /* ... Technically not correct, but if memory has gotten
                    *     this low, that's the last or the user's problems. */
#endif
   memcpy(temp_name+1,name,(namelen+1)*sizeof(char));
   temp_name[0] = '_';
   DBG_ALIGNMENT_DISABLE();
   result = dlsym(me->d_handle,temp_name);
   DBG_ALIGNMENT_ENABLE();
   Dee_AFree(temp_name);
  }
 }
#endif
 return result;
}


INTERN DREF DeeObject *DCALL
DeeDex_New(DeeObject *__restrict name) {
 DREF DeeDexObject *result;
 ASSERT_OBJECT_TYPE_EXACT(name,&DeeString_Type);
 result = DeeGCObject_CALLOC(DeeDexObject);
 if unlikely(!result) return NULL;
 DeeObject_Init(&result->d_module,&DeeDex_Type);
 result->d_module.mo_name    = (DeeStringObject *)name;
 result->d_module.mo_bucketv = empty_module_buckets;
 Dee_Incref(name);
 weakref_support_init(&result->d_module);
 DeeGC_Track((DREF DeeObject *)result);
 return (DREF DeeObject *)result;
}

#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(dex_lock);
#endif
/* [0..1][lock(dex_lock)] Global chain of loaded dex extensions. */
PRIVATE DREF DeeDexObject *dex_chain;

INTERN bool DCALL DeeDex_Cleanup(void) {
 bool result = false;
 DREF DeeDexObject *dex;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&dex_lock);
#endif
 /* NOTE: Since DEX modules are only actually removed
  *       at a later phase, we can still be sure that
  *       we can safely traverse the list and simply
  *       hold the DEX-lock while walking entries,
  *       knowing that our current entry won't just
  *       randomly disappear.
  */
 for (dex = dex_chain; dex;
      dex = dex->d_next) {
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&dex_lock);
#endif

  /* Invoke the clear-operator on the dex (If it implements it). */
  if (dex->d_dex->d_clear &&
    (*dex->d_dex->d_clear)(dex))
      result = true;

#ifndef CONFIG_NO_THREADS
  rwlock_read(&dex_lock);
#endif
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&dex_lock);
#endif
 return result;
}

INTERN void DCALL DeeDex_Finalize(void) {
 DREF DeeDexObject *dex;
again:
#ifndef CONFIG_NO_THREADS
 rwlock_write(&dex_lock);
#endif
 while ((dex = dex_chain) != NULL) {
  dex_chain    = dex->d_next;
  dex->d_pself = NULL;
  dex->d_next  = NULL;
  if (!Dee_DecrefIfNotOne((DeeObject *)dex)) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&dex_lock);
#endif
   Dee_Decref((DeeObject *)dex);
   goto again;
  }
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&dex_lock);
#endif
}


INTERN int DCALL
dex_initialize(DeeDexObject *__restrict self) {
 int (DCALL *func)(DeeDexObject *__restrict self);
 ASSERT(self->d_dex);
 func = self->d_dex->d_init;
 if (func && (*func)(self)) return -1;
 /* Register the dex in the global chain. */
#ifndef CONFIG_NO_THREADS
 rwlock_write(&dex_lock);
#endif
 if ((self->d_next = dex_chain) != NULL)
      dex_chain->d_pself = &self->d_next;
 self->d_pself = &dex_chain;
 dex_chain = self;
 Dee_Incref((DeeObject *)self); /* The reference stored in the dex chain. */
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&dex_lock);
#endif
 return 0;
}

INTDEF size_t DCALL membercache_clear(size_t max_clear);

PRIVATE void DCALL
dex_fini(DeeDexObject *__restrict self) {
 ASSERT(!self->d_pself);
 if (self->d_module.mo_flags&MODULE_FDIDLOAD) {
  uint16_t i;
  /* Clear global variables before we unload the module,
   * because most likely they're all still pointing inside. */
  for (i = 0; i < self->d_module.mo_globalc; ++i)
       Dee_XClear(self->d_module.mo_globalv[i]);
  ASSERT(self->d_dex);
  if (self->d_module.mo_flags&MODULE_FDIDINIT &&
      self->d_dex->d_fini)
    (*self->d_dex->d_fini)(self);
  /* Must clear membercaches that may have been loaded by
   * this extension before unloading the associated library.
   * If we don't do this before, dangling points may be left
   * in the global chain of active membercaches.
   * XXX: Only do this for caches apart of this module's static binary image? */
  membercache_clear((size_t)-1);
  DBG_ALIGNMENT_DISABLE();
#ifdef USE_LOADLIBRARY
  FreeLibrary((HMODULE)self->d_handle);
#else
  dlclose(self->d_handle);
#endif
  DBG_ALIGNMENT_ENABLE();
 }
}
PRIVATE void DCALL
dex_visit(DeeDexObject *__restrict self,
          dvisit_t proc, void *arg) {
 if (self->d_module.mo_flags&MODULE_FDIDLOAD) {
  struct dex_symbol *iter;
  ASSERT(self->d_dex);
  iter = self->d_dex->d_symbols;
  if (iter) {
   for (; iter->ds_name; ++iter)
       Dee_XVisit(iter->ds_obj);
  }
 }
}


PUBLIC DeeTypeObject DeeDex_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"dex",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeModule_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeDexObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&dex_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&dex_visit,
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
    /* .tp_class_members = */NULL
};


DECL_END
#endif /* !CONFIG_NO_DEX */

#endif /* !GUARD_DEEMON_EXECUTE_DEX_C */
