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
#ifndef GUARD_DEEMON_COMPILER_ASM_MODGEN_C
#define GUARD_DEEMON_COMPILER_ASM_MODGEN_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/gc.h>
#include <deemon/string.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/symbol.h>
#ifndef CONFIG_NO_DEC
#include <deemon/error.h>
#include <deemon/compiler/dec.h>
#endif
#include <deemon/module.h>

#include <string.h>

DECL_BEGIN

INTDEF struct module_symbol empty_module_buckets[];
INTERN int DCALL
module_compile(DeeModuleObject *__restrict module,
               DeeCodeObject *__restrict root_code,
               uint16_t flags) {
#ifndef CONFIG_NO_THREADS
 ASSERT(recursive_rwlock_writing(&DeeCompiler_Lock));
#endif
 ASSERT_OBJECT(root_code);
 ASSERT(DeeCode_Check(root_code));
 ASSERT_OBJECT_TYPE((DeeObject *)current_rootscope,&DeeRootScope_Type);
 /* Truncate some vectors before we'll be inheriting them. */
 ASSERT(current_rootscope->rs_importc <= current_rootscope->rs_importa);
 if (current_rootscope->rs_importa != current_rootscope->rs_importc) {
  DREF DeeModuleObject **new_vector;
  new_vector = (DREF DeeModuleObject **)Dee_TryRealloc(current_rootscope->rs_importv,
                                                       current_rootscope->rs_importc*
                                                       sizeof(DREF DeeModuleObject *));
  if likely(new_vector) current_rootscope->rs_importv = new_vector;
 }

 /* Start filling in members of the module. */
 module->mo_globalv = (DREF DeeObject **)Dee_Calloc(current_rootscope->rs_globalc*
                                                    sizeof(DREF DeeObject *));
 if unlikely(!module->mo_globalv) goto err;
 module->mo_globalc = current_rootscope->rs_globalc;
 module->mo_importc = current_rootscope->rs_importc;
#ifdef CONFIG_NO_THREADS
 module->mo_flags  |= current_rootscope->rs_flags;
#else
 ATOMIC_FETCHOR(module->mo_flags,current_rootscope->rs_flags);
#endif
 module->mo_bucketm = current_rootscope->rs_bucketm;
 module->mo_bucketv = current_rootscope->rs_bucketv;
 module->mo_importv = current_rootscope->rs_importv;
 module->mo_root    = root_code;
 Dee_Incref(root_code);
 
 /* Yes, we're just stealing all of these. */
 current_rootscope->rs_importv = NULL;
 current_rootscope->rs_importc = 0;
 current_rootscope->rs_importa = 0;
 current_rootscope->rs_bucketv = empty_module_buckets;
 current_rootscope->rs_bucketm = 0;

 { DREF DeeCodeObject *iter,*next;
   iter = current_rootscope->rs_code;
   current_rootscope->rs_code = NULL;
   while (iter) {
    next = iter->co_next;
    iter->co_module = module;
    Dee_Incref(module); /* Create the new module-reference now stored in `iter->co_module'. */
    Dee_Decref(iter); /* This reference was owned by the chain before. */
    iter = next;
   }
 }

 /* Since we're now updated all code objects ever created with the
  * current module, the given root-code had to have been one of them. */
 ASSERTF(root_code->co_module == module,
         "The given root-code was not generated for this module");

#ifndef CONFIG_NO_DEC
 /* Save the time when compilation of the module started. */
 module->mo_ctime = DecTime_Now();
#ifdef CONFIG_NO_THREADS
 module->mo_flags |= MODULE_FHASCTIME;
#else
 ATOMIC_FETCHOR(module->mo_flags,MODULE_FHASCTIME);
#endif
#endif /* !CONFIG_NO_DEC */

#ifndef CONFIG_NO_DEC
 /* With the module fully initialized, package it into a compiled DEC file.
  * NOTE: if the `ASM_FNODEC' flag has been set, don't create a DEC file. */
 if (!(flags&ASM_FNODEC) && dec_create(module)) {
#if 1
  /* If creation of a DEC file failed because of an illegal
   * constant, don't fail the entire compilation process. */
  if (!DeeError_Catch(&DeeError_NotImplemented))
       goto err_module;
#else
  goto err_module;
#endif
 }
#endif

 return 0;
#ifndef CONFIG_NO_DEC
err_module:
 /* Transfer ownership back to the compiler. */
 current_rootscope->rs_importv = (DREF DeeModuleObject **)module->mo_importv;
 current_rootscope->rs_importc = module->mo_importc;
 current_rootscope->rs_importa = module->mo_importc;
 current_rootscope->rs_bucketv = module->mo_bucketv;
 current_rootscope->rs_bucketm = module->mo_bucketm;

 module->mo_importv = NULL;
 module->mo_importc = 0;
 module->mo_bucketv = empty_module_buckets;
 module->mo_bucketm = 0;

 /* Free the previously allocated global object vector. */
 module->mo_globalc = 0;
 Dee_Free((void *)module->mo_globalv);
 module->mo_globalv = NULL;
#endif
err:
 return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_MODGEN_C */
