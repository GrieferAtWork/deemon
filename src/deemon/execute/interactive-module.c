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
#ifndef GUARD_DEEMON_EXECUTE_INTERACTIVE_MODULE_C
#define GUARD_DEEMON_EXECUTE_INTERACTIVE_MODULE_C 1
#define _GNU_SOURCE 1 /* memrchr() */

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/error.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/map.h>
#include <deemon/string.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/assembler.h>
#include <deemon/util/cache.h>
#include <deemon/util/recursive-rwlock.h>
#include <deemon/traceback.h>
#include <deemon/util/string.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#include <string.h>

DECL_BEGIN

#ifdef CONFIG_HOST_WINDOWS
#define CONFIG_NOCASE_FS 1
#define SEP              '\\'
#define SEP_S            "\\"
#define ISSEP(x) ((x) == '\\' || (x) == '/')
#define ISABS(x) ((x)[0] && (x)[1] == ':')
#else
#define SEP              '/'
#define SEP_S            "/"
#define ISSEP(x) ((x) == '/')
#define ISABS(x) ((x)[0] == '/')

#ifndef __USE_GNU
#define memrchr  dee_memrchr
LOCAL void *dee_memrchr(void const *__restrict p, int c, size_t n) {
 uint8_t *iter = (uint8_t *)p+n;
 while (iter != (uint8_t *)p) {
  if (*--iter == c) return iter;
 }
 return NULL;
}
#endif /* !__USE_GNU */

#endif



DECLARE_STRUCT_CACHE(sym,struct symbol)
#ifndef NDEBUG
#define sym_alloc()  sym_dbgalloc(__FILE__,__LINE__)
#endif


struct compiler_options_mapping {
    struct compiler_options  com_opt; /* The compiler options copy. */
    struct compiler_options *com_map; /* [1..1][const] The associated source-options mapping */
};

typedef struct {
    DeeModuleObject          im_module;   /* [OVERRIDE(.mo_path,[const])]
                                           * [OVERRIDE(.mo_pself,[0..0][const])]
                                           * [OVERRIDE(.mo_next,[0..0][const])]
                                           * [OVERRIDE(.mo_globpself,[0..0][const])]
                                           * [OVERRIDE(.mo_globnext,[0..0][const])]
                                           * [OVERRIDE(.mo_importc,[lock(:im_exec_lock)])]
                                           * [OVERRIDE(.mo_globalc,[lock(:im_exec_lock && :mo_lock)])]
                                           * [OVERRIDE(.mo_flags,[const][== MODULE_FLOADING|MODULE_FINITIALIZING])]
                                           * [OVERRIDE(.mo_bucketm,[lock(:im_exec_lock)])]
                                           * [OVERRIDE(.mo_bucketv,[lock(:im_exec_lock)])]
                                           * [OVERRIDE(.mo_importv,[lock(:im_exec_lock)])]
                                           * [OVERRIDE(.mo_globalv,[lock(:im_exec_lock && :mo_lock)])]
                                           * [OVERRIDE(.mo_root,[0..1][lock(READ(:im_exec_lock),WRITE(:im_lock))]
                                           *                    [COMMENT("The currently generated root-code object (when shared "
                                           *                             "prior to new code being generated, it is replaced with"
                                           *                             "an exact copy that is then used for modifcations; i.e. "
                                           *                             "it can be accessed when using `im_lock', and must be "
                                           *                             "copied before modifications can be made)\n"
                                           *                             "NOTE: A reference held by :im_frame.cf_func->fo_code does "
                                           *                             "not count towards the code object being shared, so-long as "
                                           *                             "that function isn't being shared, either")])]
                                           * [OVERRIDE(.mo_loader,[lock(:im_exec_lock)][0..1]
                                           *                      [COMMENT("Non-NULL, and pointing to the thread managing compilation "
                                           *                               "when new code is currently being compiled")])]
                                           * The underlying module. */
    unsigned int             im_mode;     /* [const] The module in which source code will be compiled.
                                           * Set of `MODULE_INTERACTIVE_MODE_F*' */
    DREF DeeObject          *im_stream;   /* [0..1][(!= NULL) == (im_module.mo_root != NULL)][lock(im_lock)]
                                           * The stream from which source code is read. */
    struct compiler_options  im_options;  /* [OVERRIDE(.[co_inner->*]->co_inner,[owned])]
                                           * [OVERRIDE(.[co_inner->*]->co_pathname,[TAG(DREF)])]
                                           * [OVERRIDE(.[co_inner->*]->co_filename,[TAG(DREF)])]
                                           * [OVERRIDE(.[co_inner->*]->co_rootname,[TAG(DREF)])]
                                           * [OVERRIDE(.co_inner->*->co_decoutput,[TAG(DREF)])]
                                           * [OVERRIDE(.co_decoutput,[0..0])]
                                           * [OVERRIDE(.co_decwriter,[valid_if(false)])]
                                           * [OVERRIDE(.co_assembler,[!(. & ASM_FPEEPHOLE)])]
                                           * [OVERRIDE(.co_assembler,[. & ASM_FNODEC])]
                                           * [OVERRIDE(.co_assembler,[. & ASM_FNOREUSECONST])]
                                           * [OVERRIDE(.co_assembler,[. & ASM_FBIGCODE])]
                                           * [const] Compiler options used for compiling source code. */
    DREF DeeCompilerObject  *im_compiler; /* [0..1][(!= NULL) == (im_module.mo_root != NULL)][lock(im_lock)]
                                           * The compiler used for compiling source code */
    /*ref*/struct TPPFile   *im_basefile; /* [0..1][(!= NULL) == (im_module.mo_root != NULL)][lock(im_lock)]
                                           * The TPP base file that is linked against `im_stream' */
    struct code_frame        im_frame;    /* [OVERRIDE(.cf_func,[0..1][(!= NULL) == (:im_module.mo_root != NULL)][TAG(DREF)]
                                           *                    [COMMENT("A function object referring to the code object used "
                                           *                             "to execute interactive assembly. This object is copied "
                                           *                             "prior to being modified the same way `im_module.mo_root' "
                                           *                             "is, meaning that accessing this function will yield a "
                                           *                             "sort-of snapshot that will only remain up to date until "
                                           *                             "it new code has to be compiled, after which point it "
                                           *                             "will refer to the old code")])]
                                           * [OVERRIDE(.cf_argc,[== DeeTuple_SIZE(cf_vargs)])]
                                           * [OVERRIDE(.cf_argv,[== DeeTuple_ELEM(cf_vargs)])]
                                           * [OVERRIDE(.cf_vargs,[1..1][const])]
                                           * [lock(im_exec_lock)]
                                           * Code execution frame used when items are yielded from the interactive module. */
#ifndef CONFIG_NO_THREADS
    recursive_rwlock_t       im_lock;     /* [ORDER(AFTER(im_exec_lock))] Lock used to synchronize compilation. */
    recursive_rwlock_t       im_exec_lock;/* Lock used to synchronize execution. */
#endif
} InteractiveModule;


#ifndef CONFIG_NO_THREADS
INTERN void DCALL
interactivemodule_lockread(InteractiveModule *__restrict self) {
 recursive_rwlock_read(&self->im_exec_lock);
}
INTERN void DCALL
interactivemodule_lockwrite(InteractiveModule *__restrict self) {
 recursive_rwlock_write(&self->im_exec_lock);
}
INTERN void DCALL
interactivemodule_lockendread(InteractiveModule *__restrict self) {
 recursive_rwlock_endread(&self->im_exec_lock);
}
INTERN void DCALL
interactivemodule_lockendwrite(InteractiveModule *__restrict self) {
 recursive_rwlock_endwrite(&self->im_exec_lock);
}

LOCAL bool DCALL is_an_imod(DeeObject *__restrict self) {
 DeeTypeObject *tp_self = Dee_TYPE(self);
 ASSERT_OBJECT(self);
 for (;;) {
  if (tp_self == &DeeInteractiveModule_Type)
      return true;
  if (tp_self == &DeeModule_Type)
      return false;
  tp_self = DeeType_Base(tp_self);
  ASSERTF(tp_self,"%p (%s) isn't a module",
          self,Dee_TYPE(self)->tp_name);
 }
}

PUBLIC void DCALL
DeeModule_LockSymbols(DeeObject *__restrict self) {
 if (is_an_imod(self))
     recursive_rwlock_read(&((InteractiveModule *)self)->im_exec_lock);
}
PUBLIC void DCALL
DeeModule_UnlockSymbols(DeeObject *__restrict self) {
 if (is_an_imod(self))
     recursive_rwlock_endread(&((InteractiveModule *)self)->im_exec_lock);
}
#endif /* !CONFIG_NO_THREADS */


typedef struct {
    OBJECT_HEAD
    DREF InteractiveModule *imi_mod; /* [1..1][const] The underlying, interactive module. */
} InteractiveModuleIterator;

PRIVATE bool DCALL is_statement(void) {
 switch (tok) {
  /* Check for statement-like code constructs (i.e. if, switch, goto, etc.)
   * While not technically reflected by our parser, we also count `class'
   * and `function', as well as symbol declarations. */


 case '@': /* Tags are designed to only appear in statement-like constructs. */
 case '{': /* A sub-scope is a pretty darn good indicator for a statement. */
 case KWD_if:
 case KWD_return:
 case KWD_yield:
 case KWD_from:
 case KWD_import: /* Even though `import' and `from' can appear in expressions,
                   * the preferred expression syntax is `foo from bar', which
                   * is why these count as statements. */
 case KWD_throw:
 case KWD_print:
 case KWD_for:
 case KWD_foreach:
 case KWD_assert:
 case KWD_do:
 case KWD_while:
 case KWD_break:
 case KWD_continue:
 case KWD_with:
 case KWD_try:
 case KWD_del:
#ifndef CONFIG_LANGUAGE_NO_ASM
 case KWD___asm:
 case KWD___asm__:
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 case KWD_goto:
 case KWD_switch:
  return true;

 default:
  /* XXX: What about labels? */
// case KWD_case:
// case KWD_default:
  break;
 }
 return false;
}


PRIVATE DREF DeeAstObject *DCALL
imod_parse_statement(InteractiveModule *__restrict self,
                     unsigned int mode) {
 DREF DeeAstObject *result,*merge;
 bool should_yield = false;
 if ((mode & MODULE_INTERACTIVE_MODE_FONLYBASEFILE) &&
      token.t_file != self->im_basefile);
 else if ((mode & (MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR|
                   MODULE_INTERACTIVE_MODE_FYIELDROOTSTMT)) ==
                  (MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR|
                   MODULE_INTERACTIVE_MODE_FYIELDROOTSTMT)) {
  should_yield = true;
 } else if (mode & MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR) {
  should_yield = !is_statement();
 } else if (mode & MODULE_INTERACTIVE_MODE_FYIELDROOTSTMT) {
  should_yield = is_statement();
 }

 merge = ast_parse_statement(true);
 if unlikely(!merge) goto err;
 if (should_yield) {
  result = ast_yield(merge);
  Dee_Decref(merge);
 } else {
  result = merge;
 }
 return result;
err:
 return NULL;
}

INTDEF int DCALL skip_lf(void);

PRIVATE DREF DeeObject *DCALL
imod_next(InteractiveModule *__restrict self) {
 DREF DeeObject *result; struct code_frame *top_frame;
 DREF DeeAstObject *statement_ast;
 DeeCodeObject *current_code;
 struct traceback_object *current_traceback;
 size_t preexisting_codesize;
 bool is_reusing_code_object;
 uint16_t old_co_flags;
 uint16_t old_co_localc;
 uint16_t old_co_staticc;
 uint16_t old_co_exceptc;
 uint32_t old_co_framesize;
 struct except_handler *old_co_exceptv;
 DeeDDIObject *old_co_ddi;
 int link_error;

 /* The heart piece of the interactive module execution model:
  * -> The function that compiles a piece of code, then feeds
  *    it to the interpreter as yielding code, before taking
  *    its return value and forwarding as an iterator-item. */
 recursive_rwlock_write(&self->im_exec_lock);
 /* Start by trying to execute code that has already been compiled. */
do_exec_code:
 /* TODO: Optimization: if (*self->im_frame.cf_ip == ASM_UD) { ... } */
 result = DeeCode_ExecFrameSafe(&self->im_frame);
 if (result != NULL) {
  /* value, or iter-done. */
  self->im_frame.cf_result = NULL;
  goto done_exec;
 }
 assert(DeeThread_Self()->t_exceptsz != 0);
 assert(DeeThread_Self()->t_except != NULL);
 if (!DeeObject_InstanceOf(DeeThread_Self()->t_except->ef_error,
                          &DeeError_IllegalInstruction))
      goto done_exec; /* Something other than the illegal-instruction error that we're looking for. */
 current_traceback = DeeThread_Self()->t_except->ef_trace;
 current_code = self->im_module.mo_root;
 if unlikely(!current_code) goto done_exec;
 preexisting_codesize = current_code->co_codebytes;
 if likely(current_traceback) {
  instruction_t *iter,*end;
  if unlikely(!current_traceback->tb_numframes) goto done_exec;
  /* Check if the error occurred past the written portion of interactive code. */
  top_frame = &current_traceback->tb_frames[current_traceback->tb_numframes-1];
  if (top_frame->cf_func != self->im_frame.cf_func) goto done_exec;
  /* If the error occurred somewhere within the loaded portion of code,
   * then this error isn't actually caused by the the absence of code. */
  ASSERT(top_frame->cf_ip >= current_code->co_code);
  ASSERT(top_frame->cf_ip <  current_code->co_code+
                             current_code->co_codebytes);
  iter = top_frame->cf_ip;
  end  = current_code->co_code+current_code->co_codebytes;
  preexisting_codesize = (size_t)(iter - current_code->co_code);
  for (; iter < end; ++iter) {
   if (*iter != ASM_UD)
        goto done_exec;
  }
 } else {
  /* Strip trailing ASM_UD instructions. */
  while (preexisting_codesize &&
         current_code->co_code[preexisting_codesize-1] == ASM_UD)
       --preexisting_codesize;
 }
 if (!DeeError_Handled(ERROR_HANDLED_RESTORE)) goto done_exec;

 recursive_rwlock_write(&self->im_lock);
 if unlikely(!self->im_compiler) {
  result = ITER_DONE;
  goto done_compiler;
 }
 ASSERT(!result);
 ASSERT(self->im_basefile);
 ASSERT(self->im_stream);
 ASSERT(self->im_module.mo_root);
 
 /* Active the context of the interactive module's compiler. */
 COMPILER_BEGIN(self->im_compiler);

 if (token.t_file == &TPPFile_Empty) {
  /* Push the source-stream file back onto the TPP include stack.
   * It got popped after the stream indicated EOF as the result of
   * available data having been consumed. */
  TPPLexer_PushFile(self->im_basefile);
 }

 /* Save the current exception context. */
 parser_start();

 /* Yield the initial token. */
 if unlikely((tok == TOK_EOF && yield() < 0) || skip_lf())
  statement_ast = NULL;
 else if (tok == TOK_EOF) {
  result = ITER_DONE; /* True EOF */
  if (parser_rethrow(false))
      result = NULL;
  COMPILER_END();
  /* TODO: clear compiler-related fields, thus releasing data no longer needed prematurely. */
  recursive_rwlock_endwrite(&self->im_lock);
  recursive_rwlock_endwrite(&self->im_exec_lock);
  return result;
 } else {
  /* Parse a statement in accordance to the interaction mode. */
  statement_ast = imod_parse_statement(self,self->im_mode);
 }

 /* Rethrow all errors that may have occurred during parsing. */
 if (parser_rethrow(statement_ast == NULL))
     Dee_XClear(statement_ast);

 if unlikely(!statement_ast)
    goto done_compiler_end;

 /* Merge code flags. */
 current_basescope->bs_flags |= current_code->co_flags;

 /* Run an additional optimization pass on the
  * AST before passing it off to the assembler. */
 if (optimizer_flags&OPTIMIZE_FENABLED) {
  int temp;
  temp = ast_optimize_all(statement_ast,false);
  /* Rethrow all errors that may have occurred during optimization. */
  if (parser_rethrow(temp != 0))
      temp = -1;
  if (temp)
      goto done_statement_ast;
 }

 /* Initialize the assembler context to create new code within our module.
  * NOTE: 2 references:
  *    - self->im_module.mo_root
  *    - self->im_frame.cf_func->fo_code
  */
 ASSERT(ATOMIC_READ(current_code->ob_refcnt) >= 2);
 ASSERT(self->im_module.mo_root == current_code);
 ASSERT(self->im_frame.cf_func->fo_code == current_code);
 is_reusing_code_object = (ATOMIC_READ(current_code->ob_refcnt) == 2 &&
                          !DeeObject_IsShared(self->im_frame.cf_func));
 if (is_reusing_code_object) {
  DeeGC_Untrack((DeeObject *)current_code);
  is_reusing_code_object = (ATOMIC_READ(current_code->ob_refcnt) == 2);
  if unlikely(!is_reusing_code_object)
     DeeGC_Track((DeeObject *)current_code);
 }
 ASSERT(current_code->co_refc == 0);
 ASSERT(current_code->co_argc_min == 0);
 ASSERT(current_code->co_argc_max == 0);
 ASSERT(current_code->co_defaultv == NULL);
 old_co_flags     = current_code->co_flags;
 old_co_localc    = current_code->co_localc;
 old_co_staticc   = current_code->co_staticc;
 old_co_exceptc   = current_code->co_exceptc;
 old_co_exceptv   = current_code->co_exceptv;
 old_co_framesize = current_code->co_framesize;
 old_co_ddi       = current_code->co_ddi;
 if (is_reusing_code_object) {
  ASSERT(current_code->co_module == (DREF DeeModuleObject *)self);
  DeeObject_FreeTracker((DeeObject *)current_code);
  assembler_init_reuse(current_code,
                       current_code->co_code+
                       preexisting_codesize);
  Dee_DecrefNokill(&DeeCode_Type);     /* current_code->ob_type */
  Dee_DecrefNokill((DeeObject *)self); /* current_code->co_module */
  current_assembler.a_constv = current_code->co_staticv;
  current_code->co_staticv   = NULL;
  current_code->co_staticc   = 0;
 } else {
  size_t i; instruction_t *text;
  assembler_init();
  current_assembler.a_constv = (DREF DeeObject **)Dee_Malloc(current_assembler.a_consta*
                                                             sizeof(DREF DeeObject *));
  if unlikely(!current_assembler.a_constv) goto done_assembler_fini;
  /* Copy over static variables. */
  rwlock_read(&current_code->co_static_lock);
  for (i = 0; i < current_assembler.a_constc; ++i) {
   current_assembler.a_constv[i] = current_code->co_staticv[i];
   Dee_Incref(current_assembler.a_constv[i]);
  }
  rwlock_endread(&current_code->co_static_lock);
  /* Copy over all the unwritten text. */
  text = asm_alloc(preexisting_codesize);
  if unlikely(!text) goto done_assembler_fini;
  memcpy(text,current_code->co_code,preexisting_codesize);
 }
 if (current_assembler.a_flag & ASM_FREUSELOC) {
  size_t alloc_size = (current_assembler.a_locala+7)/8;
  current_assembler.a_locala = current_assembler.a_localc;
  current_assembler.a_localuse = (uint8_t *)Dee_Malloc(alloc_size);
  if unlikely(!current_assembler.a_localuse) goto done_assembler_fini;
  memset(current_assembler.a_localuse,0xff,alloc_size);
 }

 /* Setup the assembler in accordance to the
  * pre-written code, and compiler options. */
 current_assembler.a_flag = self->im_options.co_assembler;
 ASSERT(!(current_assembler.a_flag & ASM_FPEEPHOLE));
 ASSERT(current_assembler.a_flag & ASM_FNODEC);
 ASSERT(current_assembler.a_flag & ASM_FNOREUSECONST);
 ASSERT(current_assembler.a_flag & ASM_FBIGCODE);
 current_assembler.a_localc = old_co_localc;
 current_assembler.a_constc = old_co_staticc;
 current_assembler.a_consta = old_co_staticc;

 /* Configure the currently active stack-depth. */
 current_assembler.a_stackcur = (uint16_t)(self->im_frame.cf_sp -
                                           self->im_frame.cf_stack);
 current_assembler.a_stackmax = (uint16_t)DeeCode_StackDepth(current_code);
 if (current_assembler.a_stackmax < current_assembler.a_stackcur)
     current_assembler.a_stackmax = current_assembler.a_stackcur;

 /* Now to actually assemble the statement. */
 if (ast_genasm(statement_ast,ASM_G_FNORMAL))
     goto done_assembler_fini;

 {
  /* Generate code for jumping access secondary assembly sections. */
  size_t i; struct asm_sym *code_end = NULL;
  for (i = 1; i < SECTION_COUNT; ++i) {
   if (current_assembler.a_sect[i].sec_iter ==
       current_assembler.a_sect[i].sec_begin)
       continue; /* Empty section. */
   code_end = asm_newsym();
   if unlikely(!code_end) goto done_assembler_fini;
   asm_setcur(SECTION_COUNT-1);
   break;
  }
  if (code_end) {
   for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
    asm_setcur(i);
    if (asm_gjmp(ASM_JMP,code_end))
        goto done_assembler_fini;
   }
  }
  asm_setcur(SECTION_COUNT-1);
  /* Append the trailing UD-instruction byte. */
  if (code_end)
      asm_defsym(code_end);
  if (asm_put(ASM_UD))
      goto done_assembler_fini;
 }

 /* Make sure that all used labels have been defined.
  * -> We don't allow unresolved forward-labels in
  *    interactive code (for now...) */
 if unlikely(asm_check_user_labels_defined())
    goto done_assembler_fini;

 /* Merge text sections. */
 if unlikely(asm_mergetext())
    goto done_assembler_fini;

 if (current_assembler.a_flag&(ASM_FPEEPHOLE|ASM_FOPTIMIZE)) {
  link_error = asm_linkstack();
  if unlikely(link_error != 0)
     goto err_link;
 }

 /* Merge static variables. */
 if unlikely(asm_mergestatic())
    goto done_assembler_fini;

 /* Apply constant relocations. */
 if unlikely(asm_applyconstrel())
    goto done_assembler_fini;

 /* Link together the text, resolving relocations. */
 link_error = asm_linktext();
 if unlikely(link_error != 0) {
err_link:
  if unlikely(link_error >= 0) {
   /* Already in bigcode mode? - That's not good... */
   DeeError_Throwf(&DeeError_CompilerError,
                   "Failed to link final code: Relocation target is out of bounds");
  }
  goto done_assembler_fini;
 }

 /* Indicate that we want to loop back and try to execute the newly generated code. */
 result = ITER_DONE;
done_assembler_fini:
 ASSERT(result == NULL || result == ITER_DONE);
 ASSERT(!current_assembler.a_refc);
 if (result != NULL) {
  /* TODO: Create new global variables & add new imports. */
  /*current_rootscope->rs_globalc;*/

  /* Update the frame storage for local variables. */
  if (current_assembler.a_localc > old_co_localc) {
   DREF DeeObject **new_localv;
   uint16_t req_localc = current_assembler.a_localc;
   new_localv = (DREF DeeObject **)Dee_Realloc(self->im_frame.cf_frame,
                                               req_localc*sizeof(DREF DeeObject *));
   if unlikely(!new_localv) goto err_result;
   MEMSET_PTR(new_localv+old_co_localc,0,req_localc-old_co_localc);
   /* Install the new stack. */
   self->im_frame.cf_frame = new_localv;
  }

  /* Update the frame storage for required stack memory. */
  if (current_assembler.a_stackmax > self->im_frame.cf_stacksz) {
   DREF DeeObject **new_stackv;
   uint16_t req_stacksz = current_assembler.a_stackmax;
   new_stackv = (DREF DeeObject **)Dee_Realloc(self->im_frame.cf_stack,
                                               req_stacksz*sizeof(DREF DeeObject *));
   if unlikely(!new_stackv) goto err_result;
   /* Install the new stack. */
   self->im_frame.cf_sp      = new_stackv + (self->im_frame.cf_sp - self->im_frame.cf_stack);
   self->im_frame.cf_stack   = new_stackv;
   self->im_frame.cf_stacksz = req_stacksz;
  }

  current_code = asm_gencode();
  if unlikely(!current_code) {
gencode_failed:
   result = NULL;
   if (is_reusing_code_object)
       goto recover_old_code_object;
  } else {
   ASSERT(current_rootscope->rs_code == current_code);
   current_rootscope->rs_code = current_code->co_next;
   Dee_DecrefNokill(current_code); /* Stolen from `current_rootscope->rs_code' */
   current_code->co_next = NULL;
   /* Fill in module-field of the new code object. */
   Dee_Incref((DeeObject *)self);
   current_code->co_module = (DREF DeeModuleObject *)self;

   ASSERT((current_code->co_exceptc != 0) ==
          (current_code->co_exceptv != 0));
   if (old_co_exceptc) {
    /* Pre-pend the old exception handler vector before the new one! */
    if (!current_code->co_exceptc) {
     current_code->co_exceptc = old_co_exceptc;
     current_code->co_exceptv = old_co_exceptv;
    } else if (is_reusing_code_object) {
     struct except_handler *full_exceptv;
     full_exceptv = (struct except_handler *)Dee_Realloc(old_co_exceptv,
                                                        (current_code->co_exceptc+old_co_exceptc)*
                                                         sizeof(struct except_handler));
     if unlikely(!full_exceptv) goto gencode_failed;
     memcpy(full_exceptv+old_co_exceptc,
            current_code->co_exceptv,
            current_code->co_exceptc*
            sizeof(struct except_handler));
     Dee_Free(current_code->co_exceptv);
     current_code->co_exceptc += old_co_exceptc;
     current_code->co_exceptv  = full_exceptv;
    } else {
     struct except_handler *full_exceptv; uint16_t i;
     full_exceptv = (struct except_handler *)Dee_Realloc(current_code->co_exceptv,
                                                        (current_code->co_exceptc+old_co_exceptc)*
                                                         sizeof(struct except_handler));
     if unlikely(!full_exceptv) goto gencode_failed;
     memmove(full_exceptv+old_co_exceptc,full_exceptv,
             current_code->co_exceptc*sizeof(struct except_handler));
     memcpy(full_exceptv,old_co_exceptv,old_co_exceptc*sizeof(struct except_handler));
     for (i = 0; i < old_co_exceptc; ++i)
         Dee_XIncref(full_exceptv[i].eh_mask);
     current_code->co_exceptc += old_co_exceptc;
     current_code->co_exceptv  = full_exceptv;
    }
   }

   /* TODO: Merge the old DDI descriptor with the new one.
    * NOTE: If something goes wrong here, `current_code->co_exceptv'
    *       must be truncated to a total length of `old_co_exceptc' */
   if (is_reusing_code_object)
       Dee_Decref(old_co_ddi);

   /* Set the new code object. */
   Dee_Incref(current_code);
   ASSERT(current_code->ob_refcnt == 2);

   if (DeeObject_IsShared(self->im_frame.cf_func)) {
    DeeFunctionObject *new_self_func;
    /* Must create a new function-object. */
    Dee_Incref(Dee_EmptyTuple);
    new_self_func = (DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)current_code);
    Dee_DecrefNokill(current_code);
    if unlikely(!new_self_func) {
     Dee_DecrefNokill(Dee_EmptyTuple);
     result = NULL;
     if (is_reusing_code_object)
         goto recover_old_code_object;
     goto do_assembler_fini;
    }
    Dee_Decref(self->im_frame.cf_func);
    self->im_frame.cf_func = new_self_func; /* Inherit reference. */
   } else {
    if (!is_reusing_code_object)
         Dee_Decref(self->im_frame.cf_func->fo_code);
    self->im_frame.cf_func->fo_code = current_code; /* Inherit reference. */
   }
   if (!is_reusing_code_object)
        Dee_Decref(self->im_module.mo_root);
   self->im_module.mo_root = current_code; /* Inherit reference. */
  }
  /* Set the code-execution frame to continue where it left off before. */
  self->im_frame.cf_ip = current_code->co_code + preexisting_codesize;
 } else {
err_result:
  result = NULL;
  if (is_reusing_code_object) {
   /* Recover the old code object. */
recover_old_code_object:
   ASSERT(is_reusing_code_object);
   current_code = current_assembler.a_sect[SECTION_TEXT].sec_code;
   current_code->co_code[preexisting_codesize] = ASM_UD;
   current_code->co_flags     = old_co_flags;
   current_code->co_localc    = old_co_localc;
   current_code->co_staticc   = old_co_staticc;
   current_code->co_staticv   = current_assembler.a_constv;
   current_code->co_refc      = 0;
   current_code->co_exceptc   = old_co_exceptc;
   current_code->co_exceptv   = old_co_exceptv;
   current_code->co_argc_min  = 0;
   current_code->co_argc_max  = 0;
   current_code->co_framesize = old_co_framesize;
   current_code->co_codebytes = (code_size_t)(preexisting_codesize+1);
#ifndef CONFIG_NO_THREADS
   rwlock_init(&current_code->co_static_lock);
#endif
   Dee_Incref((DeeObject *)self);
   current_code->co_module   = (DREF DeeModuleObject *)self;
   current_code->co_defaultv = NULL;
   current_code->co_ddi      = old_co_ddi;
   while (current_assembler.a_constc > old_co_staticc) {
    --current_assembler.a_constc;
    Dee_Decref(current_assembler.a_constv[current_assembler.a_constc]);
   }
   current_assembler.a_sect[SECTION_TEXT].sec_code  = NULL;
   current_assembler.a_sect[SECTION_TEXT].sec_begin = NULL;
   current_assembler.a_sect[SECTION_TEXT].sec_iter  = NULL;
   current_assembler.a_sect[SECTION_TEXT].sec_end   = NULL;
   current_assembler.a_constc                       = 0;
   current_assembler.a_consta                       = 0;
   current_assembler.a_constv                       = NULL;
  }
 }
do_assembler_fini:
 assembler_fini();
done_statement_ast:
 Dee_Decref(statement_ast);
done_compiler_end:
 COMPILER_END();
done_compiler:
 recursive_rwlock_endwrite(&self->im_lock);
done_exec:
 if (result == ITER_DONE)
     goto do_exec_code;
 recursive_rwlock_endwrite(&self->im_exec_lock);
 return result;
}


#define INTERACTIVE_MODULE_DEFAULT_GLOBAL_SYMBOL_MASK  15


PRIVATE void DCALL
incref_options(struct compiler_options *__restrict self) {
 Dee_XIncref(self->co_pathname);
 Dee_XIncref(self->co_filename);
 Dee_XIncref(self->co_rootname);
 Dee_XIncref(self->co_decoutput);
}
PRIVATE void DCALL
decref_options(struct compiler_options *__restrict self) {
 Dee_XDecref(self->co_pathname);
 Dee_XDecref(self->co_filename);
 Dee_XDecref(self->co_rootname);
 Dee_XDecref(self->co_decoutput);
}
PRIVATE void DCALL
visit_options(struct compiler_options *__restrict self, dvisit_t proc, void *arg) {
 Dee_XVisit(self->co_pathname);
 Dee_XVisit(self->co_filename);
 Dee_XVisit(self->co_rootname);
 Dee_XVisit(self->co_decoutput);
}


PRIVATE int DCALL
copy_options_chain(struct compiler_options_mapping **proot,
                   struct compiler_options_mapping **presult,
                   struct compiler_options *__restrict source) {
 struct compiler_options_mapping *iter;
 iter = *proot;
 /* Search for a pre-existing mapping for `source' */
 for (; iter; iter = (struct compiler_options_mapping *)iter->com_opt.co_inner) {
  if (iter->com_map == source) {
   *presult = iter;
   return 0;
  }
 }
 /* Create a copy of `source'. */
 iter = (struct compiler_options_mapping *)Dee_Malloc(sizeof(struct compiler_options_mapping));
 if unlikely(!iter) return -1;
 memcpy(iter,source,sizeof(struct compiler_options));
 iter->com_opt.co_inner = NULL;
 iter->com_map = source;
 /* Save the result. */
 *presult = iter;
 COMPILER_WRITE_BARRIER();
 if (source->co_inner) {
  /* Copy inner set of options. */
  if (copy_options_chain(proot,
                        (struct compiler_options_mapping **)&iter->com_opt.co_inner,
                         source->co_inner)) {
   /* Undo the copy */
   *presult = NULL;
   Dee_Free(iter);
   return -1;
  }
 }
 incref_options(&iter->com_opt);
 return 0;
}


PRIVATE void DCALL
free_options_chain(struct compiler_options *__restrict entry,
                   struct compiler_options *__restrict base,
                   unsigned int depth) {
 unsigned int i;
 struct compiler_options *iter = base;
 for (i = 0; i < depth; ++i) {
  if (iter == entry) return; /* Options loop detected */
  iter = iter->co_inner;
 }
 if (entry->co_inner)
     free_options_chain(entry->co_inner,base,depth+1);
 decref_options(entry);
 Dee_Free(entry);
}

PRIVATE void DCALL
visit_options_chain(struct compiler_options *__restrict entry,
                    struct compiler_options *__restrict base,
                    unsigned int depth, dvisit_t proc, void *arg) {
 unsigned int i;
 struct compiler_options *iter = base;
 for (i = 0; i < depth; ++i) {
  if (iter == entry) return; /* Options loop detected */
  iter = iter->co_inner;
 }
 if (entry->co_inner)
     visit_options_chain(entry->co_inner,base,depth+1,proc,arg);
 visit_options(entry,proc,arg);
}


PRIVATE int DCALL
module_rehash_globals(DeeModuleObject *__restrict self) {
 size_t i,new_mask = (self->mo_bucketm << 1) | 1;
 struct module_symbol *new_vec;
 ASSERT(!(new_mask & (new_mask+1)));
 new_vec = (struct module_symbol *)Dee_Calloc((new_mask+1)*
                                               sizeof(struct module_symbol));
 if unlikely(!new_vec) goto err;
 for (i = 0; i < self->mo_bucketm; ++i) {
  size_t j,perturb;
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) continue;
  perturb = j = item->ss_hash & new_mask;
  for (;; j = MODULE_HASHNX(j,perturb),MODULE_HASHPT(perturb)) {
   struct module_symbol *new_item = &new_vec[j & new_mask];
   if (new_item->ss_name) continue;
   /* Copy the old item into this new slot. */
   memcpy(new_item,item,sizeof(struct module_symbol));
   break;
  }
 }
 /* Free the old bucket vector and assign the new one */
 Dee_Free(self->mo_bucketv);
 self->mo_bucketm = (uint16_t)new_mask;
 self->mo_bucketv = new_vec;
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
module_import_symbol(DeeModuleObject *__restrict self,
                     DeeStringObject *__restrict name,
                     DeeObject *__restrict value) {
 dhash_t i,perturb,hash;
 DREF DeeObject **new_globalv;
 /* Rehash the global symbol table is need be. */
 if (self->mo_globalc/2 >= self->mo_bucketm &&
     module_rehash_globals(self))
     goto err;
 new_globalv = (DREF DeeObject **)Dee_Realloc(self->mo_globalv,
                                             (self->mo_globalc+1)*
                                              sizeof(DREF DeeObject *));
 if unlikely(!new_globalv) goto err;
 self->mo_globalv = new_globalv;

 /* Append the symbol initializer */
 new_globalv[self->mo_globalc++] = value;
 Dee_Incref(value);

 /* Insert the new object into the symbol table. */
 hash = DeeString_Hash((DeeObject *)name);
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (item->ss_name) continue;
  /* Use this item. */
  item->ss_name  = name;
  item->ss_doc   = NULL;
  item->ss_hash  = hash;
  item->ss_flags = MODSYM_FNORMAL;
  item->ss_index = self->mo_globalc - 1;
  Dee_Incref(name);
  break;
 }
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
module_import_symbol_pair(DeeModuleObject *__restrict self,
                          DeeObject *__restrict symbol_pair) {
 DREF DeeObject *key,*value; int result;
 if (DeeMapping_UnpackItemPair(symbol_pair,&key,&value))
     goto err;
 if (DeeObject_AssertTypeExact(key,&DeeString_Type))
     result = -1;
 else {
  result = module_import_symbol(self,(DeeStringObject *)key,value);
 }
 Dee_Decref(value);
 Dee_Decref(key);
 return result;
err:
 return -1;
}

PRIVATE int DCALL
module_import_symbols(DeeModuleObject *__restrict self,
                      DeeObject *__restrict default_symbols) {
 DREF DeeObject *iterator,*elem; int temp;
 iterator = DeeObject_IterSelf(default_symbols);
 if unlikely(!iterator) goto err;
 while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
  temp = module_import_symbol_pair(self,elem);
  Dee_Decref(elem);
  if unlikely(temp) goto err;
 }
 Dee_Decref(iterator);
 return 0;
err:
 return -1;
}




/* - Set the FASSEMBLY flag so we don't have to deal with fast-vs-safe execution,
 *   or any complication caused by having to change the execution model later.
 * - Set the FLENIENT stack flag so make it a little bit easier for us to
 *   automatically allocate more stack memory as new code is generated.
 * - Set the FVARARGS flag because arguments passed to the source code
 *   will be variadic and specified using `argv'
 * - Set the FHEAPFRAME flag because we always need to allocate local variable
 *   memory in the heap, considering how we operate as a yield-like function.
 */
#define INTERACTIVE_MODULE_CODE_FLAGS \
       (CODE_FASSEMBLY|CODE_FLENIENT|CODE_FVARARGS|CODE_FHEAPFRAME|CODE_FYIELDING)


INTDEF int DCALL
rehash_scope(DeeScopeObject *__restrict iter);
INTDEF int DCALL
TPPFile_SetStartingLineAndColumn(struct TPPFile *__restrict self,
                                 int start_line, int start_col);


PRIVATE int DCALL
imod_init(InteractiveModule *__restrict self,
          DeeObject *__restrict source_pathname,
          DeeObject *module_name,
          DeeObject *__restrict source_stream,
          int start_line, int start_col,
          struct compiler_options *options,
          unsigned int mode,
          DeeObject *argv,
          DeeObject *default_symbols) {
 size_t i;
 ASSERT_OBJECT_TYPE_EXACT(source_pathname,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT_OPT(module_name,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT_OPT(argv,&DeeTuple_Type);
 if (!argv) argv = Dee_EmptyTuple;
 /* First off: Copy compiler options */
 if (options) {
  memcpy(&self->im_options,options,sizeof(struct compiler_options));
  /* Recursively copy inner options from the given set of compiler options. */
  if (options->co_inner) {
   self->im_options.co_inner = NULL;
   if (copy_options_chain((struct compiler_options_mapping **)&self->im_options.co_inner,
                          (struct compiler_options_mapping **)&self->im_options.co_inner,
                           options->co_inner))
       goto err;
  }
 } else {
  memset(&self->im_options,0,sizeof(struct compiler_options));
  /* Enable the LFSTMT option by default. */
  self->im_options.co_parser |= PARSE_FLFSTMT;
 }

 /* Make sure to properly set some compiler
  * options mandatory for interactive compilation. */
 self->im_options.co_assembler &= ~ASM_FPEEPHOLE;
 self->im_options.co_assembler |= (ASM_FNODEC|ASM_FNOREUSECONST|ASM_FBIGCODE);
 self->im_options.co_decoutput  = NULL;
 incref_options(&self->im_options);

 /* Determine the module's name. */
 if (!module_name) {
  char  *name = DeeString_STR(source_pathname);
  size_t size = DeeString_SIZE(source_pathname);
  char *name_end,*name_start;
  name_end = name+size;
#ifdef CONFIG_HOST_WINDOWS
  name_start = name_end;
  while (name_start != name && !ISSEP(name_start[-1]))
       --name_start;
#else
  name_start = (char *)memrchr(name,SEP,size);
  if (!name_start) name_start = name-1;
  ++name_start;
#endif
  /* Get rid of a file extension in the module name. */
  while (name_end != name_start && name_end[-1] != '.') --name_end;
  while (name_end != name_start && name_end[-1] == '.') --name_end;
  if (name_end == name_start) name_end = name+size;
  module_name = DeeString_NewSized(name_start,(size_t)(name_end-name_start));
  if unlikely(!module_name) goto err_options;
  self->im_module.mo_name = (DREF struct string_object *)module_name; /* Inherit reference */
 } else {
  Dee_Incref(module_name);
  self->im_module.mo_name = (DREF struct string_object *)module_name;
 }
 /* Fill in the module's path name. */
 ASSERT_OBJECT_TYPE_EXACT(source_pathname,&DeeString_Type);
 Dee_Incref(source_pathname);
 self->im_module.mo_path = (DREF struct string_object *)source_pathname;

 /* Set global hook members as NULL pointers. */
 self->im_module.mo_pself     = NULL;
 self->im_module.mo_next      = NULL;
 self->im_module.mo_globpself = NULL;
 self->im_module.mo_globnext  = NULL;

 /* Reset imports, globals and flags. */
 self->im_module.mo_importc   = 0;
 self->im_module.mo_importv   = NULL;
 self->im_module.mo_globalc   = 0;
 self->im_module.mo_globalv   = NULL;
 self->im_module.mo_flags     = MODULE_FLOADING|MODULE_FINITIALIZING;
 self->im_module.mo_bucketm   = INTERACTIVE_MODULE_DEFAULT_GLOBAL_SYMBOL_MASK;
 self->im_module.mo_bucketv   = (struct module_symbol *)Dee_Calloc((INTERACTIVE_MODULE_DEFAULT_GLOBAL_SYMBOL_MASK+1)*
                                                                    sizeof(struct module_symbol));
 if unlikely(!self->im_module.mo_bucketv) goto err_name;

 /* If given, import default symbols as globals. */
 if (default_symbols &&
     module_import_symbols(&self->im_module,default_symbols))
     goto err_globals;


#ifndef CONFIG_NO_THREADS
 rwlock_init(&self->im_module.mo_lock);
 /* Setup the module to indicate that it isn't being loaded right now. */
 self->im_module.mo_loader = NULL;
#endif
 weakref_support_init(&self->im_module);

 /* With that, all module-level and some interactive
  * level components have already been initialized. */


 /* Save some of the given arguments in their proper locations. */
 self->im_stream = source_stream;
 Dee_Incref(source_stream);
 self->im_mode = mode;

#ifndef CONFIG_NO_THREADS
 recursive_rwlock_init(&self->im_lock);
 recursive_rwlock_init(&self->im_exec_lock);
#endif

 /* Setup the contents of the initial code frame. */
 self->im_frame.cf_prev    = CODE_FRAME_NOT_EXECUTING;
 self->im_frame.cf_argc    = DeeTuple_SIZE(argv);
 self->im_frame.cf_argv    = DeeTuple_ELEM(argv);
 self->im_frame.cf_frame   = NULL;
 self->im_frame.cf_stack   = NULL;
 self->im_frame.cf_sp      = NULL;
 //self->im_frame.cf_ip    = ...; /* Set below! */
 self->im_frame.cf_vargs   = (DREF DeeTupleObject *)argv;
 self->im_frame.cf_this    = NULL;
 self->im_frame.cf_result  = NULL;
 self->im_frame.cf_stacksz = 0;
 self->im_frame.cf_flags   = INTERACTIVE_MODULE_CODE_FLAGS;
 Dee_Incref(argv);

 /* Allocate the function object for the initial code frame. */
 self->im_frame.cf_func = (DeeFunctionObject *)DeeObject_Malloc(offsetof(DeeFunctionObject,fo_refv));
 if unlikely(!self->im_frame.cf_func) goto err_stream;
 DeeObject_Init(self->im_frame.cf_func,&DeeFunction_Type);
 //self->im_frame.cf_func->fo_code = ...; /* Set below! */

 /* Allocate the compiler that will be used
  * to process data from the source stream. */
 self->im_compiler = DeeCompiler_New((DeeObject *)self,self->im_options.co_compiler);
 if unlikely(!self->im_compiler) goto err_frame;

 /* Setup the initial configuration that will be used by the compiler,
  * as well as link in the source stream as the used TPP input file. */
 self->im_compiler->cp_options         = &self->im_options;
 self->im_compiler->cp_inner_options   = self->im_options.co_inner;
 self->im_compiler->cp_parser_flags    = self->im_options.co_parser;
 self->im_compiler->cp_optimizer_flags = self->im_options.co_optimizer;
 self->im_compiler->cp_unwind_limit    = self->im_options.co_unwind_limit;

 if (self->im_options.co_parser & PARSE_FLFSTMT)
     self->im_compiler->cp_lexer.l_flags |= TPPLEXER_FLAG_WANTLF;

 /* Allocate the varargs symbol for the root-scope. */
 {
  struct symbol *dots;
  DeeRootScopeObject *root_scope;
  root_scope = (DeeRootScopeObject *)self->im_compiler->cp_scope;
  if unlikely((dots = sym_alloc()) == NULL)
     goto err_compiler;
#ifndef NDEBUG
  memset(dots,0xcc,sizeof(struct symbol));
#endif
  dots->sym_name = &TPPKeyword_Empty;
  dots->sym_next = root_scope->rs_scope.bs_scope.s_del;
  root_scope->rs_scope.bs_scope.s_del = dots;
  dots->sym_flag  = SYM_FNORMAL;
  dots->sym_read  = 0;
  dots->sym_write = 0;
  dots->sym_scope = &root_scope->rs_scope.bs_scope;
  root_scope->rs_scope.bs_argv = (struct symbol **)Dee_Malloc(1*sizeof(struct symbol *));
  if unlikely(!root_scope->rs_scope.bs_argv) { sym_free(dots); goto err_compiler; }
  dots->sym_class                 = SYM_CLASS_ARG;
  dots->sym_arg.sym_index         = 0;
  root_scope->rs_scope.bs_argc    = 1;
  root_scope->rs_scope.bs_argv[0] = dots;
  root_scope->rs_scope.bs_varargs = dots;
  root_scope->rs_scope.bs_flags  |= INTERACTIVE_MODULE_CODE_FLAGS;
 }


 {
  DeeStringObject *source_path;
  /*ref*/struct TPPFile *base_file;
  source_path = self->im_options.co_pathname;
  if (!source_path) source_path = (DeeStringObject *)source_pathname;
  base_file = TPPFile_OpenStream((stream_t)source_stream,
                                  DeeString_STR(source_path));
  if unlikely(!base_file) goto err_compiler;
  /* Set the non-blocking I/O flag for the input file. */
  base_file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_NONBLOCK;

  self->im_basefile = base_file; /* Inherit reference. */
  /* Set the starting-line offset. */
  if (TPPFile_SetStartingLineAndColumn(base_file,start_line,start_col))
      goto err_basefile;

  /* Override the name that is used as the
   * effective display/DDI string of the file. */
  if (self->im_options.co_filename) {
   struct TPPString *used_name;
   ASSERT_OBJECT_TYPE_EXACT(self->im_options.co_filename,&DeeString_Type);
do_create_used_name:
   used_name = TPPString_New(DeeString_STR(self->im_options.co_filename),
                             DeeString_SIZE(self->im_options.co_filename));
   if unlikely(!used_name) {
    if (Dee_CollectMemory(offsetof(struct TPPString,s_text)+
                         (DeeString_SIZE(self->im_options.co_filename)+1)*sizeof(char)))
        goto do_create_used_name;
    goto err_basefile;
   }
   ASSERT(!base_file->f_textfile.f_usedname);
   base_file->f_textfile.f_usedname = used_name; /* Inherit */
  }

  /* Set the name of the current base-scope, which
   * describes the function of the module's root code. */
  if (self->im_options.co_rootname) {
   DREF DeeBaseScopeObject *module_base_scope;
   ASSERT_OBJECT_TYPE_EXACT(self->im_options.co_rootname,&DeeString_Type);
   module_base_scope = self->im_compiler->cp_scope->s_base;
   ASSERT(!module_base_scope->bs_name);
do_create_base_name:
   module_base_scope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(self->im_options.co_rootname),
                                                       DeeString_SIZE(self->im_options.co_rootname),1);
   if unlikely(!module_base_scope->bs_name) {
    if (Dee_CollectMemory(offsetof(struct TPPKeyword,k_name)+
                         (DeeString_SIZE(self->im_options.co_rootname)+1)*sizeof(char)))
        goto do_create_base_name;
    goto err_basefile;
   }
  }

  /* Create symbol bindings for all the global variables that had been pre-defined. */
  if (self->im_module.mo_globalc) {
   /* NOTE: Sadly we must switch compiler context here,
    *       just so we can use `TPPLexer_LookupKeyword()' */
   COMPILER_BEGIN(self->im_compiler);
   for (i = 0; i <= self->im_module.mo_bucketm; ++i) {
    struct symbol *sym,**bucket;
    struct module_symbol *modsym;
    modsym = &self->im_module.mo_bucketv[i];
    if (!modsym->ss_name) continue;
    if unlikely((sym = sym_alloc()) == NULL) {
err_compiler_basefile:
     COMPILER_END();
     goto err_basefile;
    }
    sym->sym_name  = TPPLexer_LookupKeyword(DeeString_STR(modsym->ss_name),
                                            DeeString_SIZE(modsym->ss_name),
                                            1);
    if unlikely(!sym->sym_name) goto err_compiler_basefile;
    sym->sym_class         = SYM_CLASS_VAR;
    sym->sym_flag          = SYM_FVAR_ALLOC|SYM_FVAR_GLOBAL;
    sym->sym_read          = 0;
    sym->sym_write         = 1; /* The initial write done by the pre-initialization. */
    sym->sym_scope         = current_scope;
    sym->sym_var.sym_index = modsym->ss_index; /* Bind to this global index. */
    sym->sym_var.sym_doc   = NULL;
    /* Register the symbol in the current scope. */
    if (++current_scope->s_mapc > current_scope->s_mapa) {
     /* Must rehash this scope. */
     if unlikely(rehash_scope(current_scope))
        goto err_compiler_basefile;
    }
    /* Insert the new symbol into the scope lookup map. */
    ASSERT(current_scope->s_mapa != 0);
    bucket = &current_scope->s_map[sym->sym_name->k_id % current_scope->s_mapa];
    sym->sym_next = *bucket;
    *bucket = sym;
   }
   COMPILER_END();
  }
 }
 /* And with _all_ of that, we've finally configured everything
  * to represent the initial state of the interactive compiler.
  * Only thing left is to create the initial code object, which
  * is done last due to the reference loop it creates. */

 /* Construct a small code object that contains a single `ASM_UD' instruction.
  * `ASM_UD' is used as a marker for code that hasn't been compiled yet, and
  * when encountered by the interpreter at the end of the code segment, an
  * IllegalInstruction error is thrown, which we can handle by checking if it
  * was thrown by an instruction beyond the end of compiled code, in which case
  * we know that interactive execution has reached the point where new assembly
  * has to be generated, and new source code must be compiled.
  * SO to start out: Create a single-instruction ASM_UD code-object. */
 {
  DREF DeeCodeObject *init_code;
  init_code = (DREF DeeCodeObject *)DeeGCObject_Malloc(offsetof(DeeCodeObject,co_code)+
                                                       sizeof(instruction_t));
  if unlikely(!init_code) goto err_globals;
  init_code->co_flags     = INTERACTIVE_MODULE_CODE_FLAGS;
  init_code->co_localc    = 0;
  init_code->co_staticc   = 0;
  init_code->co_refc      = 0;
  init_code->co_exceptc   = 0;
  init_code->co_argc_min  = 0;
  init_code->co_argc_max  = 0;
  init_code->co_framesize = 0;
  init_code->co_codebytes = sizeof(instruction_t);
#ifndef CONFIG_NO_THREADS
  rwlock_init(&init_code->co_static_lock);
#endif
  init_code->co_module    = (DREF struct module_object *)self;
  init_code->co_defaultv  = NULL;
  init_code->co_staticv   = NULL;
  init_code->co_exceptv   = NULL;
  init_code->co_ddi       = &empty_ddi;
  init_code->co_code[0]   = ASM_UD;
  Dee_Incref((DeeObject *)self);
  Dee_Incref(&empty_ddi);
  DeeObject_Init(init_code,&DeeCode_Type);
  DeeGC_Track((DeeObject *)init_code);
  self->im_module.mo_root = init_code; /* Inherit reference. */

  /* Set the initial instruction pointer. */
  self->im_frame.cf_ip = init_code->co_code;

  /* Set the code-pointer of the initial function object. */
  Dee_Incref(init_code);
  self->im_frame.cf_func->fo_code = init_code;
 }

 return 0;
err_basefile:
 TPPFile_Decref(self->im_basefile);
err_compiler:
 Dee_Decref(self->im_compiler);
err_frame:
 Dee_DecrefNokill(&DeeFunction_Type);
 DeeObject_FreeTracker((DeeObject *)self->im_frame.cf_func);
 DeeObject_Free((DeeObject *)self->im_frame.cf_func);
err_stream:
 Dee_Decref(self->im_stream);
err_globals:
 for (i = 0; i < self->im_module.mo_globalc; ++i)
     Dee_XDecref(self->im_module.mo_globalv[i]);
 for (i = 0; i <= self->im_module.mo_bucketm; ++i) {
  if (!self->im_module.mo_bucketv[i].ss_name)
       continue;
  Dee_Decref(self->im_module.mo_bucketv[i].ss_name);
  Dee_XDecref(self->im_module.mo_bucketv[i].ss_doc);
 }
 Dee_Free(self->im_module.mo_bucketv);
 Dee_Free(self->im_module.mo_globalv);
err_name:
 Dee_Decref(self->im_module.mo_name);
 Dee_Decref(self->im_module.mo_path);
err_options:
 free_options_chain(self->im_options.co_inner,
                    self->im_options.co_inner,
                    0);
err:
 return -1;
}


PRIVATE int DCALL
imod_ctor(InteractiveModule *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 DeeObject *imod_path = Dee_EmptyString;
 DeeObject *imod_name = NULL;
 DeeObject *imod_argv = NULL;
 DeeObject *imod_syms = NULL;
 switch (argc) {
 case 1:
  /* (file stream) */
  break;
 case 2:
  /* (file stream, string pathname) */
  /* (file stream, tuple argv) */
  /* (file stream, mapping default_symbols) */
  if (DeeTuple_Check(argv[1]))
      imod_argv = argv[1];
  else if (DeeString_Check(argv[1]))
      imod_path = argv[1];
  else imod_syms = argv[1];
  break;
 case 3:
  if (DeeTuple_Check(argv[1])) {
   /* (file stream, tuple argv, mapping default_symbols) */
   imod_argv = argv[1];
   imod_syms = argv[2];
  } else {
   /* (file stream, string pathname, mapping default_symbols) */
   /* (file stream, string pathname, tuple argv) */
   /* (file stream, string pathname, string name) */
   if (DeeObject_AssertTypeExact(argv[1],&DeeString_Type))
       goto err;
   imod_path = argv[1];
   if (DeeTuple_Check(argv[2]))
       imod_argv = argv[2];
   else if (DeeString_Check(argv[2]))
       imod_name = argv[2];
   else imod_syms = argv[2];
  }
  break;

 case 4:
  if (DeeObject_AssertTypeExact(argv[1],&DeeString_Type))
      goto err;
  imod_path = argv[1];
  if (DeeTuple_Check(argv[2])) {
   /* (file stream, string pathname, tuple argv, mapping default_symbols) */
   imod_argv = argv[2];
   imod_syms = argv[3];
  } else {
   /* (file stream, string pathname, string name, tuple argv) */
   /* (file stream, string pathname, string name, mapping default_symbols) */
   if (DeeObject_AssertTypeExact(argv[2],&DeeString_Type))
       goto err;
   imod_name = argv[2];
   if (DeeTuple_Check(argv[3])) {
    imod_argv = argv[3];
   } else {
    imod_syms = argv[3];
   }
  }
  break;

  /* (file stream, string pathname, string name, tuple argv, mapping default_symbols) */
 case 5:
  if (DeeObject_AssertTypeExact(argv[1],&DeeString_Type))
      goto err;
  if (DeeObject_AssertTypeExact(argv[2],&DeeString_Type))
      goto err;
  if (DeeObject_AssertTypeExact(argv[3],&DeeTuple_Type))
      goto err;
  imod_path = argv[1];
  imod_name = argv[2];
  imod_argv = argv[3];
  imod_syms = argv[4];
  break;

 default:
  err_invalid_argc("_interactivemodule",argc,1,5);
  goto err;
 }
 return imod_init(self,imod_path,imod_name,argv[0],0,0,NULL,
                  MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR|
                  MODULE_INTERACTIVE_MODE_FONLYBASEFILE,
                  imod_argv,imod_syms);
err:
 return -1;
}




PUBLIC DREF DeeObject *DCALL
DeeModule_OpenInteractive(DeeObject *__restrict source_pathname,
                          DeeObject *module_name,
                          DeeObject *__restrict source_stream,
                          int start_line, int start_col,
                          struct compiler_options *options,
                          unsigned int mode,
                          DeeObject *argv,
                          DeeObject *default_symbols) {
 DREF InteractiveModule *result;
 result = DeeGCObject_MALLOC(InteractiveModule);
 if unlikely(!result) goto done;
 DeeObject_Init((DeeObject *)result,&DeeInteractiveModule_Type);
 if (imod_init(result,source_pathname,
                      module_name,
                      source_stream,
                      start_line,
                      start_col,
                      options,
                      mode,
                      argv,
                      default_symbols))
     goto err_r;
 /* Start tracking the new module as a GC object. */
 DeeGC_Track((DeeObject *)result);
done:
 return (DREF DeeObject *)result;
err_r:
 Dee_DecrefNokill(&DeeInteractiveModule_Type);
 DeeObject_FreeTracker((DeeObject *)result);
 DeeObject_Free((DeeObject *)result);
 return NULL;
}

DFUNDEF DREF DeeObject *DCALL
DeeModule_OpenInteractiveString(char const *__restrict source_pathname,
                                DeeObject *module_name,
                                DeeObject *__restrict source_stream,
                                int start_line, int start_col,
                                struct compiler_options *options,
                                unsigned int mode,
                                DeeObject *argv,
                                DeeObject *default_symbols) {
 DREF DeeObject *result;
 DREF DeeObject *source_pathname_ob;
 source_pathname_ob = DeeString_New(source_pathname);
 if unlikely(!source_pathname_ob) return NULL;
 result = DeeModule_OpenInteractive(source_pathname_ob,
                                    module_name,
                                    source_stream,
                                    start_line,
                                    start_col,
                                    options,
                                    mode,
                                    argv,
                                    default_symbols);
 Dee_Decref(source_pathname_ob);
 return result;
}




PRIVATE void DCALL
imod_fini(InteractiveModule *__restrict self) {
 size_t i;
 Dee_XDecref(self->im_stream);
 Dee_XDecref(self->im_compiler);
 if (self->im_basefile)
     TPPFile_Decref(self->im_basefile);
 if (self->im_module.mo_root) {
  for (i = 0; i < self->im_module.mo_root->co_localc; ++i)
      Dee_XDecref(self->im_frame.cf_frame[i]);
 }
 Dee_Free(self->im_frame.cf_frame);
 i = (size_t)(self->im_frame.cf_sp - self->im_frame.cf_stack);
 while (i--) Dee_Decref(self->im_frame.cf_stack[i]);
 Dee_Free(self->im_frame.cf_stack);
 Dee_Decref(self->im_frame.cf_vargs);
 Dee_XDecref(self->im_frame.cf_func);
 if (self->im_options.co_inner) {
  free_options_chain(self->im_options.co_inner,
                     self->im_options.co_inner,
                     0);
 }
 decref_options(&self->im_options);
 ASSERT(!(self->im_module.mo_flags & MODULE_FDIDLOAD));
 for (i = 0; i < self->im_module.mo_globalc; ++i)
      Dee_XDecref(self->im_module.mo_globalv[i]);
 Dee_Free(self->im_module.mo_globalv);
}
PRIVATE void DCALL
imod_clear(InteractiveModule *__restrict self) {
 size_t i,localc,old_globalc;
 DREF DeeObject *old_stream;
 DREF DeeObject **old_globalv;
 struct compiler_options old_options;
 DREF DeeCompilerObject *old_compiler;
 /*ref*/struct TPPFile *old_basefile;
 struct code_frame old_frame;
 recursive_rwlock_write(&self->im_exec_lock);
 recursive_rwlock_write(&self->im_lock);
 old_stream        = self->im_stream;
 self->im_stream   = NULL;
 old_basefile      = self->im_basefile;
 self->im_basefile = NULL;
 old_compiler      = self->im_compiler;
 self->im_compiler = NULL;
 memcpy(&old_options,&self->im_options,sizeof(struct compiler_options));
 memset(&self->im_options,0,sizeof(struct compiler_options));
 self->im_options.co_assembler |= ASM_FNODEC;
 recursive_rwlock_endwrite(&self->im_lock);
 memcpy(&old_frame,&self->im_frame,sizeof(struct code_frame));
 self->im_frame.cf_func    = NULL;
 self->im_frame.cf_frame   = NULL;
 self->im_frame.cf_stack   = NULL;
 self->im_frame.cf_sp      = NULL;
 self->im_frame.cf_ip      = NULL;
 self->im_frame.cf_this    = NULL;
 self->im_frame.cf_result  = NULL;
 self->im_frame.cf_stacksz = 0;
 self->im_frame.cf_flags   = 0;
 localc = self->im_module.mo_root ? self->im_module.mo_root->co_localc : 0;
 ASSERT(!(self->im_module.mo_flags & MODULE_FDIDLOAD));
 old_globalc = self->im_module.mo_globalc;
 old_globalv = self->im_module.mo_globalv;
 self->im_module.mo_globalc = 0;
 self->im_module.mo_globalv = NULL;
 recursive_rwlock_endwrite(&self->im_exec_lock);
 Dee_XDecref(old_stream);
 Dee_XDecref(old_compiler);
 if (old_basefile)
     TPPFile_Decref(old_basefile);
 for (i = 0; i < localc; ++i)
     Dee_XDecref(old_frame.cf_frame[i]);
 Dee_Free(old_frame.cf_frame);
 for (i = 0; i < old_globalc; ++i)
      Dee_XDecref(old_globalv[i]);
 Dee_Free(old_globalv);
 i = (size_t)(old_frame.cf_sp - old_frame.cf_stack);
 while (i--) Dee_Decref(old_frame.cf_stack[i]);
 Dee_Free(old_frame.cf_stack);
 Dee_XDecref(old_frame.cf_func);
 if (old_options.co_inner) {
  free_options_chain(old_options.co_inner,
                     old_options.co_inner,
                     0);
 }
 decref_options(&old_options);
}

PRIVATE void DCALL
imod_visit(InteractiveModule *__restrict self, dvisit_t proc, void *arg) {
 size_t i;
 recursive_rwlock_read(&self->im_exec_lock);
 recursive_rwlock_read(&self->im_lock);
 Dee_Visit(self->im_stream);
 Dee_Visit(self->im_compiler);
 if (self->im_options.co_inner) {
  visit_options_chain(self->im_options.co_inner,
                      self->im_options.co_inner,
                      0,proc,arg);
 }
 visit_options(&self->im_options,proc,arg);
 recursive_rwlock_endread(&self->im_lock);
 /* TODO: `TPPFile_Visit(self->im_basefile)' */
 if (self->im_module.mo_root) {
  for (i = 0; i < self->im_module.mo_root->co_localc; ++i)
      Dee_XVisit(self->im_frame.cf_frame[i]);
 }
 i = (size_t)(self->im_frame.cf_sp - self->im_frame.cf_stack);
 while (i--) Dee_Visit(self->im_frame.cf_stack[i]);
 Dee_Visit(self->im_frame.cf_vargs);
 Dee_Visit(self->im_frame.cf_func);
 recursive_rwlock_endread(&self->im_exec_lock);
}


PRIVATE DREF InteractiveModule *DCALL
imod_iter(InteractiveModule *__restrict self) {
 Dee_Incref((DeeObject *)self);
 return self;
}

PRIVATE struct type_seq imod_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&imod_iter
};
PRIVATE struct type_gc imod_gc = {
    /* .tp_clear = */(void (DCALL *)(DeeObject *__restrict))&imod_clear
};


INTDEF struct type_gc module_gc;
PUBLIC DeeTypeObject DeeInteractiveModule_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_interactivemodule",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FGC,
    /* .tp_weakrefs = */WEAKREF_SUPPORT_ADDR(DeeModuleObject),
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeModule_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&imod_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(InteractiveModule)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&imod_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&imod_visit,
    /* .tp_gc            = */&imod_gc,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&imod_seq,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&imod_next,
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

#endif /* !GUARD_DEEMON_EXECUTE_INTERACTIVE_MODULE_C */
