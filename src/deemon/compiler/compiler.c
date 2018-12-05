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
#ifndef GUARD_DEEMON_COMPILER_COMPILER_C
#define GUARD_DEEMON_COMPILER_COMPILER_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/util/rwlock.h>
#include <deemon/compiler/optimize.h>

#include <string.h>

#include "../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
PUBLIC recursive_rwlock_t DeeCompiler_Lock = RWLOCK_INIT;
#endif

PUBLIC struct weakref DeeCompiler_Active = WEAKREF_INIT;
PRIVATE DeeCompilerObject *compiler_loaded = NULL; /* == DeeCompiler_Active */

PRIVATE void *DCALL memxch(void *a, void *b, size_t num_bytes) {
 uint8_t *pa = (uint8_t *)a;
 uint8_t *pb = (uint8_t *)b;
 while (num_bytes >= sizeof(unsigned int)) {
  unsigned int temp;
  temp = *(unsigned int *)pa;
  *(unsigned int *)pa = *(unsigned int *)pb;
  *(unsigned int *)pb = temp;
  pa += sizeof(unsigned int);
  pb += sizeof(unsigned int);
  num_bytes -= sizeof(unsigned int);
 }
 while (num_bytes--) {
  uint8_t temp;
  temp = *pa;
  *pa = *pb;
  *pb = temp;
  ++pa;
  ++pb;
 }
 return a;
}


/* compiler --> GLOBAL */
PRIVATE void DCALL
load_compiler(DeeCompilerObject *__restrict compiler) {
#ifndef CONFIG_NO_THREADS
 ASSERT(recursive_rwlock_writing(&DeeCompiler_Lock));
#endif
 current_scope = compiler->cp_scope;
 ASSERT_OBJECT(current_scope);
 current_basescope = current_scope->s_base;
 ASSERT_OBJECT((DeeObject *)current_basescope);
 current_rootscope = current_basescope->bs_root;
 ASSERT_OBJECT((DeeObject *)current_rootscope);
 inner_compiler_options = compiler->cp_inner_options;
 memcpy(&current_tags,&compiler->cp_tags,sizeof(struct ast_tags));
 parser_flags = compiler->cp_parser_flags;
 optimizer_flags = compiler->cp_optimizer_flags;
 optimizer_unwind_limit = compiler->cp_unwind_limit;
 if (!(compiler->cp_flags & COMPILER_FKEEPLEXER))
       memxch(&TPPLexer_Global,&compiler->cp_lexer,sizeof(struct TPPLexer));
 if (!(compiler->cp_flags & COMPILER_FKEEPERROR))
       memxch(&current_parser_errors,&compiler->cp_errors,sizeof(struct parser_errors));
}


/* GLOBAL --> compiler */
PRIVATE void DCALL
save_compiler(DeeCompilerObject *__restrict compiler) {
#ifndef CONFIG_NO_THREADS
 ASSERT(recursive_rwlock_writing(&DeeCompiler_Lock));
#endif
 ASSERT_OBJECT(current_scope);
 ASSERT_OBJECT((DeeObject *)current_basescope);
 ASSERT_OBJECT((DeeObject *)current_rootscope);
 ASSERT(current_basescope == current_scope->s_base);
 ASSERT(current_rootscope == current_basescope->bs_root);
 compiler->cp_scope = current_scope;
 compiler->cp_inner_options = inner_compiler_options;
 memcpy(&compiler->cp_tags,&current_tags,sizeof(struct ast_tags));
#ifndef NDEBUG
 memset(&current_scope,0xcc,sizeof(current_scope));
 memset(&current_basescope,0xcc,sizeof(current_scope));
 memset(&current_rootscope,0xcc,sizeof(current_scope));
 memset(&inner_compiler_options,0xcc,sizeof(inner_compiler_options));
 memset(&current_tags,0xcc,sizeof(current_tags));
#endif

 compiler->cp_parser_flags = parser_flags;
 compiler->cp_optimizer_flags = optimizer_flags;
 compiler->cp_unwind_limit = optimizer_unwind_limit;
 if (!(compiler->cp_flags & COMPILER_FKEEPLEXER))
       memxch(&TPPLexer_Global,&compiler->cp_lexer,sizeof(struct TPPLexer));
 if (!(compiler->cp_flags & COMPILER_FKEEPERROR))
       memxch(&current_parser_errors,&compiler->cp_errors,sizeof(struct parser_errors));
}


PUBLIC DREF DeeCompilerObject *DeeCompiler_Current = NULL;
PUBLIC void DCALL
DeeCompiler_Begin(DREF DeeCompilerObject *__restrict compiler) {
#ifndef CONFIG_NO_THREADS
 ASSERT(recursive_rwlock_writing(&DeeCompiler_Lock));
#endif
 ASSERT(DeeCompiler_Check(compiler));
 if (DeeCompiler_Current != compiler) {
  ASSERTF(compiler->cp_recursion == 0,
          "Cannot use interweaved compiler recursion "
          "(`a -> b -> a' is illegal)!");
  if ((compiler->cp_prev = DeeCompiler_Current) != NULL) {
   ASSERT(compiler_loaded != compiler);
   ASSERT(compiler_loaded == DeeCompiler_Current);
   /* Safe the state of another compiler during recursion. */
   save_compiler(DeeCompiler_Current);
   goto do_load_compiler;
  }
  if (compiler_loaded != compiler) {
   if (compiler_loaded) {
    /* WARNING: The reference counter of `compiler_loaded'
     *          may already be ZERO(0) at this point, but that is OK. */
    /* Safe the state of a dangling compiler. */
    save_compiler(compiler_loaded);
   }
do_load_compiler:
   /* Load the new compiler. */
   load_compiler(compiler);
   compiler_loaded = compiler;
   Dee_weakref_set(&DeeCompiler_Active,(DeeObject *)compiler);
  }
  DeeCompiler_Current = compiler;
 }
 ASSERT(compiler_loaded           == compiler);
 ASSERT(DeeCompiler_Active.wr_obj == (DeeObject *)compiler);
 ++compiler->cp_recursion;
}

PUBLIC void DCALL
DeeCompiler_End(void) {
 DeeCompilerObject *curr;
#ifndef CONFIG_NO_THREADS
 ASSERT(recursive_rwlock_writing(&DeeCompiler_Lock));
#endif
 curr = DeeCompiler_Current;
 ASSERT(curr != NULL);
 ASSERT(compiler_loaded           == curr);
 ASSERT(DeeCompiler_Active.wr_obj == (DeeObject *)curr);
 if (!--curr->cp_recursion) {
  DeeCompiler_Current = curr->cp_prev;
  curr->cp_prev = NULL;
  if (DeeCompiler_Current) {
   /* Reload the previously active compiler. */
   save_compiler(curr);
   load_compiler(DeeCompiler_Current);
   compiler_loaded = DeeCompiler_Current;
   Dee_weakref_set(&DeeCompiler_Active,(DeeObject *)DeeCompiler_Current);
  } else {
   /* NOTE: We intentionally leave `compiler_loaded' dangling,
    *       so we can optimize for cases in which only one compiler
    *       exists, but it constantly starts and stops.
    *       In such cases, we leave its state loaded, so we don't
    *       have to re-load it every time a new operation begins. */
#if 0
   save_compiler(curr);
   compiler_loaded = NULL;
   Dee_weakref_clear(&DeeCompiler_Active);
#endif
  }
 }
}

PUBLIC void DCALL
DeeCompiler_Unload(DREF DeeCompilerObject *__restrict compiler) {
 ASSERT(DeeCompiler_Check(compiler));
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_write(&DeeCompiler_Lock);
#endif /* !CONFIG_NO_THREADS */
 ASSERT(compiler != DeeCompiler_Current);
 ASSERT(compiler->cp_recursion == 0);
 if (compiler_loaded == compiler) {
  save_compiler(compiler);
  compiler_loaded = NULL;
  /* NOTE: Depending on order of destruction, the runtime
   *       may have already cleared this reference. */
  Dee_weakref_clear(&DeeCompiler_Active);
 }
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_endwrite(&DeeCompiler_Lock);
#endif /* !CONFIG_NO_THREADS */
}












/* -------- Compiler Object Implementation -------- */
/* Construct a new compiler for generating the source for the given `module'.
 * @param: flags: Set of `COMPILER_F*' (see above) */
PUBLIC DREF DeeCompilerObject *DCALL
DeeCompiler_New(DeeObject *__restrict module,
                uint16_t flags) {
 DREF DeeCompilerObject *result;
 ASSERT_OBJECT_TYPE(module,&DeeModule_Type);
 ASSERTF(!(flags & ~COMPILER_FMASK),"Invalid compiler flags in %x",flags);
 result = DeeObject_MALLOC(DeeCompilerObject);
 if unlikely(!result) goto done;
 /* Create the new root scope object. */
 result->cp_scope = (DREF DeeScopeObject *)DeeObject_New(&DeeRootScope_Type,1,
                                                         (DeeObject **)&module);
 if unlikely(!result->cp_scope) goto err_r;
 weakref_support_init(result);
 memset(&result->cp_tags,0,sizeof(result->cp_tags));
 memset(&result->cp_items,0,sizeof(result->cp_items));
 result->cp_flags           = flags;
 result->cp_prev            = NULL;
 result->cp_recursion       = 0;
 result->cp_options         = NULL;
 result->cp_inner_options   = NULL;
 result->cp_parser_flags    = PARSE_FNORMAL;
 result->cp_optimizer_flags = OPTIMIZE_FNORMAL;
 result->cp_unwind_limit    = 0;
#ifndef CONFIG_LANGUAGE_NO_ASM
 result->cp_uasm_unique     = 0;
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 if (!(flags & COMPILER_FKEEPLEXER)) {
  if (!TPPLexer_Init(&result->cp_lexer))
       goto err_scope;
#ifdef _MSC_VER
  /* Mirror MSVC's file-and-line syntax. */
  result->cp_lexer.l_flags |= TPPLEXER_FLAG_MSVC_MESSAGEFORMAT;
#endif
  result->cp_lexer.l_extokens = TPPLEXER_TOKEN_LANG_DEEMON;
 }
 if (!(flags & COMPILER_FKEEPERROR))
       parser_errors_init(&result->cp_errors);
 DeeObject_Init(result,&DeeCompiler_Type);
done:
 return result;
err_scope:
 Dee_Decref(result->cp_scope);
err_r:
 DeeObject_FREE(result);
 return NULL;
}

PRIVATE void DCALL
compiler_fini(DeeCompilerObject *__restrict self) {
 weakref_support_fini(self);

 /* Make sure that the compiler is fully unloaded. */
 DeeCompiler_Unload(self);

 if (self->cp_tags.at_anno.an_annov) {
  if unlikely(self->cp_tags.at_anno.an_annoc) {
   recursive_rwlock_write(&DeeCompiler_Lock);
   while (self->cp_tags.at_anno.an_annoc--)
       ast_decref(self->cp_tags.at_anno.an_annov[self->cp_tags.at_anno.an_annoc].aa_func);
   recursive_rwlock_endwrite(&DeeCompiler_Lock);
  }
  Dee_Free(self->cp_tags.at_anno.an_annov);
  self->cp_tags.at_anno.an_annoa = 0;
  self->cp_tags.at_anno.an_annov = NULL;
 }
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
 unicode_printer_fini(&self->cp_tags.at_decl);
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
 unicode_printer_fini(&self->cp_tags.at_doc);

 /* Always set the error-flag to prevent TPP from attempting
  * to warn about stuff like unclosed if-blocks, because now
  * that the compiler has been unloaded, we are no longer
  * allowed to emit any warnings. */
 self->cp_lexer.l_flags |= TPPLEXER_FLAG_ERROR;

 /* Then: Destroy its components. */
 if (!(self->cp_flags & COMPILER_FKEEPERROR))
       parser_errors_fini(&self->cp_errors);
 if (!(self->cp_flags & COMPILER_FKEEPLEXER))
       TPPLexer_Quit(&self->cp_lexer);
 Dee_Decref(self->cp_scope);
 Dee_Free(self->cp_items.ci_list);
}

PRIVATE void DCALL
compiler_visit(DeeCompilerObject *__restrict self, dvisit_t proc, void *arg) {
 /* First: Make sure that the compiler is fully unloaded. */
 DeeCompiler_Unload(self);

 /* TODO: parser_errors_visit(&self->cp_errors,proc,arg); */
 /* TODO: TPPLexer_Visit(&self->cp_lexer,proc,arg); */ // TPP uses DeeObject for its streams, meaning we're holding reference to those!
 Dee_Visit(self->cp_scope);
}

INTDEF struct type_method compiler_methods[];
INTDEF struct type_getset compiler_getsets[];
INTDEF struct type_member compiler_class_members[];

INTDEF int DCALL compiler_init(DeeCompilerObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

PUBLIC DeeTypeObject DeeCompiler_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"compiler",
    /* .tp_doc      = */NULL,
    /* TODO: This must be a GC object, because user-code may create const-symbols
     *       that re-reference the compiler, creating a reference loop:
     * >> import Compiler from rt;
     * >> local com = Compiler();
     * >> local sym = com.rootscope.newlocal("foo",loc: none);
     * >> sym.setconst(com); // Reference loop
     */
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */WEAKREF_SUPPORT_ADDR(DeeCompilerObject),
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                TYPE_FIXED_ALLOCATOR(DeeCompilerObject),
                /* .tp_any_ctor_kw = */&compiler_init,
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&compiler_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&compiler_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */compiler_methods,
    /* .tp_getsets       = */compiler_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */compiler_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_COMPILER_C */
