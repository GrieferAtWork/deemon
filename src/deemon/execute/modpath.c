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
#ifndef GUARD_DEEMON_EXECUTE_MODPATH_C
#define GUARD_DEEMON_EXECUTE_MODPATH_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/exec.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/thread.h>
#ifndef CONFIG_NO_DEX
#include <deemon/dex.h>
#endif
#ifndef CONFIG_NO_DEC
#include <deemon/dec.h>
#endif

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <deemon/compiler/compiler.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/optimize.h>
#include <deemon/string.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#endif

#include <string.h>
#include <stdlib.h>

#ifndef __USE_KOS
#define strend(x) ((x)+strlen(x))
#endif

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else
#   define PATH_MAX 260
#endif
#endif

DECL_BEGIN


INTDEF struct module_symbol empty_module_buckets[];

#if defined(__USE_KOS) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (memcasecmp(a,b,s) == 0)
#elif defined(_MSC_VER) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (_memicmp(a,b,s) == 0)
#else
#define MEMCASEEQ(a,b,s)  dee_memcaseeq((uint8_t *)(a),(uint8_t *)(b),s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
 while (s--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a;
  ++b;
 }
 return true;
}
#endif

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
#endif

PRIVATE dssize_t DCALL
print_pwd(struct unicode_printer *__restrict printer) {
#ifdef CONFIG_HOST_WINDOWS
 LPWSTR buffer; DWORD new_bufsize,bufsize = 256;
 buffer = unicode_printer_alloc_wchar(printer,bufsize);
 if unlikely(!buffer) goto err;
again:
 new_bufsize = GetCurrentDirectoryW(bufsize+1,buffer);
 if unlikely(!new_bufsize) { nt_ThrowLastError(); goto err_release; }
 if (new_bufsize > bufsize) {
  LPWSTR new_buffer;
  /* Increase the buffer and try again. */
  new_buffer = unicode_printer_resize_wchar(printer,buffer,new_bufsize);
  if unlikely(!new_buffer) goto err_release;
  bufsize = new_bufsize;
  goto again;
 }
 if unlikely(unicode_printer_confirm_wchar(printer,buffer,new_bufsize) < 0)
    goto err;
 if ((!printer->up_length ||
       UNICODE_PRINTER_GETCHAR(printer,printer->up_length-1) != SEP) &&
       unicode_printer_putascii(printer,SEP))
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,buffer);
err:
 return -1;
#else
 char *buffer,*new_buffer; size_t bufsize = 256;
 buffer = unicode_printer_alloc_utf8(printer,bufsize);
 if unlikely(!buffer) goto err;
 while (!getcwd(buffer,bufsize+1)) {
  /* Increase the buffer and try again. */
  if (errno != ERANGE) {
   DeeError_Throwf(&DeeError_SystemError,
                   "Failed to determine the current working directory");
   goto err_release;
  }
  bufsize *= 2;
  new_buffer = unicode_printer_resize_utf8(printer,buffer,bufsize);
  if unlikely(!new_buffer) goto err_release;
 }
 bufsize = strlen(buffer);
 if unlikely(unicode_printer_confirm_utf8(printer,buffer,bufsize) < 0)
    goto err;
 /* Make sure there is a trailing slash */
 if ((!printer->up_length ||
       UNICODE_PRINTER_GETCHAR(printer,printer->up_length-1) != SEP) &&
       unicode_printer_putascii(printer,SEP))
     goto err;
 return 0;
err_release:
 unicode_printer_free_utf8(printer,buffer);
err:
 return -1;
#endif

}

INTERN DREF DeeObject *DCALL
make_absolute(DeeObject *__restrict path) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 char *iter,*begin,*end,*flush_start,*flush_end,ch;
 begin = DeeString_AsUtf8(path);
 if unlikely(!begin) goto err;
 end = begin + WSTR_LENGTH(begin);
 /* Strip leading space. */
 begin = utf8_skipspace(begin,end);
 if (!ISABS(begin)) {
  /* Print the current working directory when the given path isn't absolute. */
  if unlikely(print_pwd(&printer) < 0) goto err;
#ifdef CONFIG_HOST_WINDOWS
  /* Handle drive-relative paths. */
  if (ISSEP(begin[0]) && UNICODE_PRINTER_LENGTH(&printer)) {
   size_t index = 0;
   /* This sep must exist because it was printed by `print_pwd()' */
   while ((++index,UNICODE_PRINTER_GETCHAR(&printer,index-1) != SEP));
   unicode_printer_truncate(&printer,index);
   /* Strip leading slashes. */
   for (;;) {
    begin = utf8_skipspace(begin,end);
    if (begin >= end) break;
    if (!ISSEP(*begin)) break;
    ++begin;
   }
  }
#endif
 }
 iter = flush_start = begin;
 ASSERT(*end == '\0');
next:
 ch = *iter++;
 switch (ch) {

 /* NOTE: The following part has been mirrored in `fs_pathexpand'
  *       that is apart of the `fs' DEX implementation file: `fs/path.c'
  *       If a bug is found in this code, it should be fixed here, as
  *       well as within the DEX source file. */
 {
  char const *sep_loc;
  bool did_print_sep;
#if SEP != '/'
 case '/':
#endif
 case SEP:
 case '\0':
  sep_loc = flush_end = iter-1;
  /* Skip multiple slashes and whitespace following a path separator. */
  for (;;) {
   iter = utf8_skipspace(iter,end);
   if (iter >= end) break;
   if (!ISSEP(*iter)) break;
   ++iter;
  }
  flush_end = utf8_skipspace_rev(flush_end,flush_start);
  /* Analyze the last path portion for being a special name (`.' or `..') */
  if (flush_end[-1] == '.') {
   if (flush_end[-2] == '.' && flush_end-2 == flush_start) {
    dssize_t new_end; size_t printer_length;
    /* Parent-directory-reference. */
    /* Delete the last directory that was written. */
    if (!printer.up_buffer) goto do_flush_after_sep;
    printer_length = printer.up_length;
    if (!printer_length) goto do_flush_after_sep;
    if (UNICODE_PRINTER_GETCHAR(&printer,printer_length-1) == SEP)
        --printer_length;
    new_end = unicode_printer_memrchr(&printer,SEP,0,printer_length);
    if (new_end < 0) goto do_flush_after_sep;
    ++new_end;
    /* Truncate the valid length of the printer to after the previous slash. */
    printer.up_length = (size_t)new_end;
    unicode_printer_truncate(&printer,(size_t)new_end);
    goto done_flush;
   } else if (flush_end[-3] == SEP && flush_end-3 >= flush_start) {
    /* Parent-directory-reference. */
    char *new_end;
    new_end = (char *)memrchr(flush_start,SEP,
                             (size_t)((flush_end-3)-flush_start));
    if (!new_end) goto done_flush;
    flush_end = new_end+1; /* Include the previous sep in this flush. */
    if (unicode_printer_print(&printer,flush_start,
                             (size_t)(flush_end-flush_start)) < 0)
        goto err;
    goto done_flush;
   } else if (flush_end-1 == flush_start) {
    /* Self-directory-reference. */
done_flush:
    flush_start = iter;
    goto done_flush_nostart;
   } else if (flush_end[-2] == SEP &&
              flush_end-2 >= flush_start) {
    /* Self-directory-reference. */
    flush_end -= 2;
   }
  }
do_flush_after_sep:
  /* Check if we need to fix anything */
  if (flush_end == iter-1
#ifdef CONFIG_HOST_WINDOWS
      && (*sep_loc == SEP || iter == end+1)
#endif
      )
      goto done_flush_nostart;
  /* If we can already include a slash in this part, do so. */
  did_print_sep = false;
  if (sep_loc == flush_end
#ifdef CONFIG_HOST_WINDOWS
      && (*sep_loc == SEP)
#endif
      )
      ++flush_end,did_print_sep = true;
  /* Flush everything prior to the path. */
  ASSERT(flush_end >= flush_start);
  if (unicode_printer_print(&printer,flush_start,
                           (size_t)(flush_end-flush_start)) < 0)
      goto err;
  flush_start = iter;
  if (did_print_sep)
   ; /* The slash has already been been printed: `foo/ bar' */
  else if (sep_loc == iter-1
#ifdef CONFIG_HOST_WINDOWS
           && (!*sep_loc || *sep_loc == SEP)
#endif
           )
   --flush_start; /* The slash will be printed as part of the next flush: `foo /bar' */
  else {
   /* The slash must be printed explicitly: `foo / bar' */
   if (unicode_printer_putascii(&printer,SEP) < 0)
       goto err;
  }
done_flush_nostart:
  if (iter == end+1)
      goto done;
  goto next;
 }
 default: goto next;
 }
done:
 --iter;
 /* Print the remainder. */
 if (iter > flush_start) {

  /* Check for special case: The printer was never used.
   * If this is the case, we can simply re-return the given path. */
  if (!UNICODE_PRINTER_LENGTH(&printer)) {
   unicode_printer_fini(&printer);
   return_reference_(path);
  }
  /* Actually print the remainder. */
  if (unicode_printer_print(&printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
      goto err;
 }
 /* Pack everything together. */
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}


/* Begin loading the given module.
 * @return: 0: You're now responsible to load the module.
 * @return: 1: The module has already been loaded.
 * @return: 2: You've already started loading this module. */
PRIVATE int DCALL
DeeModule_BeginLoading(DeeModuleObject *__restrict self) {
 uint16_t flags;
 DeeThreadObject *caller = DeeThread_Self();
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
begin_loading:
 flags = ATOMIC_FETCHOR(self->mo_flags,MODULE_FLOADING);
 if (flags&MODULE_FLOADING) {
  /* Module is already being loaded. */
  while ((flags = ATOMIC_READ(self->mo_flags),
         (flags&(MODULE_FLOADING|MODULE_FDIDLOAD)) ==
                 MODULE_FLOADING)) {
   /* Check if the module is being loaded in the current thread. */
   if (self->mo_loader == caller)
       return 2;
#ifdef CONFIG_HOST_WINDOWS
   /* Sleep a bit longer than usually. */
   __NAMESPACE_INT_SYM SleepEx(1000,0);
#else
   SCHED_YIELD();
#endif
  }
  /* If the module has now been marked as having finished loading,
   * then simply act as though it was us that did it. */
  if (flags&MODULE_FDIDLOAD) return 1;
  goto begin_loading;
 }
 /* Setup the module to indicate that we're the ones loading it. */
 self->mo_loader = caller;
 return 0;
}
PRIVATE void DCALL
DeeModule_FailLoading(DeeModuleObject *__restrict self) {
 ATOMIC_FETCHAND(self->mo_flags,~(MODULE_FLOADING));
}
PRIVATE void DCALL
DeeModule_DoneLoading(DeeModuleObject *__restrict self) {
 ATOMIC_FETCHOR(self->mo_flags,MODULE_FDIDLOAD);
}

INTERN int DCALL
TPPFile_SetStartingLineAndColumn(struct TPPFile *__restrict self,
                                 int start_line, int start_col) {
 /* Set the starting-line offset. */
 self->f_textfile.f_lineoff = start_line;
 if (start_col > 0) {
  struct TPPString *pad_text;
  /* Insert some padding white-space to make it look like the first line
   * starts with a whole bunch of whitespace, thereby adjusting the offset
   * of the starting column number in the first line. */
  pad_text = TPPString_NewSized((size_t)(unsigned int)start_col);
  if unlikely(!pad_text) return -1;
  /* Use space characters to pad text. */
  memset(pad_text->s_text,' ',pad_text->s_size);
  TPPString_Decref(self->f_text);
  self->f_text  = pad_text; /* Inherit reference */
  self->f_begin = pad_text->s_text;
  self->f_end   = pad_text->s_text+pad_text->s_size;
  /* Start scanning _after_ the padding text (don't produce white-space tokens before then!) */
  self->f_pos   = self->f_end;
 }
 return 0;
}


INTERN int DCALL
DeeModule_LoadSourceStreamEx(DeeModuleObject *__restrict self,
                             DeeObject *__restrict input_file,
                             struct compiler_options *options,
                             int start_line, int start_col,
                             struct string_object *input_pathname) {
 DREF DeeCompilerObject *compiler; struct TPPFile *base_file;
 DREF DeeAstObject *ast; DREF DeeCodeObject *root_code; int result;
 uint16_t assembler_flags; uint16_t compiler_flags;
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 ASSERT_OBJECT_TYPE(input_file,(DeeTypeObject *)&DeeFile_Type);
 ASSERT_OBJECT_TYPE_EXACT_OPT(input_pathname,&DeeString_Type);
#if 1 /* Always prefer the manual override */
 if (options && options->co_pathname)
     input_pathname = options->co_pathname;
#else
 if (!input_pathname && options)
      input_pathname = options->co_pathname;
#endif
 compiler_flags = COMPILER_FNORMAL;
 if (options)
     compiler_flags = options->co_compiler;
 /* Create a new compiler for the module. */
 compiler = DeeCompiler_New((DeeObject *)self,compiler_flags);
 if unlikely(!compiler) goto err;
 /* Start working with this compiler. */
 COMPILER_BEGIN(compiler);
again_openstream:
 base_file = TPPFile_OpenStream((stream_t)input_file,
                                 input_pathname ? DeeString_STR(input_pathname) : "");
 if unlikely(!base_file) {
  if (Dee_CollectMemory(TPPFILE_SIZEOF_TEXT))
      goto again_openstream;
  goto err_compiler;
 }
 /* Set the starting-line offset. */
 if (TPPFile_SetStartingLineAndColumn(base_file,start_line,start_col)) {
  TPPFile_Decref(base_file);
  goto err_compiler;
 }

 /* Push the initial source file onto the #include-stack,
  * and TPP inherit our reference to it. */
 TPPLexer_PushFileInherited(base_file);

 /* Override the name that is used as the
  * effective display/DDI string of the file. */
 if (options && options->co_filename) {
  struct TPPString *used_name;
  ASSERT_OBJECT_TYPE_EXACT(options->co_filename,&DeeString_Type);
do_create_used_name:
  used_name = TPPString_New(DeeString_STR(options->co_filename),
                            DeeString_SIZE(options->co_filename));
  if unlikely(!used_name) {
   if (Dee_CollectMemory(offsetof(struct TPPString,s_text)+
                        (DeeString_SIZE(options->co_filename)+1)*sizeof(char)))
       goto do_create_used_name;
   goto err_compiler;
  }
  ASSERT(!base_file->f_textfile.f_usedname);
  base_file->f_textfile.f_usedname = used_name; /* Inherit */
 }
 ASSERT(!current_basescope->bs_name);
 /* Set the name of the current base-scope, which
  * describes the function of the module's root code. */
 if (options && options->co_rootname) {
  ASSERT_OBJECT_TYPE_EXACT(options->co_rootname,&DeeString_Type);
do_create_base_name:
  current_basescope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(options->co_rootname),
                                                      DeeString_SIZE(options->co_rootname),1);
  if unlikely(!current_basescope->bs_name) {
   if (Dee_CollectMemory(offsetof(struct TPPKeyword,k_name)+
                        (DeeString_SIZE(options->co_rootname)+1)*sizeof(char)))
       goto do_create_base_name;
   goto err_compiler;
  }
 }

 assembler_flags = 0;
 inner_compiler_options = NULL;
 if (options) {
  /* Load custom parser/optimizer flags. */
  assembler_flags          = options->co_assembler;
  compiler->cp_options     = options;
  inner_compiler_options   = options->co_inner;
  parser_flags             = options->co_parser;
  optimizer_flags          = options->co_optimizer;
  optimizer_unwind_limit   = options->co_unwind_limit;
  if (options->co_tabwidth)
      TPPLexer_Current->l_tabsize = (size_t)options->co_tabwidth;
  if (parser_flags & PARSE_FLFSTMT)
      TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;

  if (options->co_setup) {
   /* Run a custom setup protocol. */
   result = (*options->co_setup)(options->co_setup_arg);
   if unlikely(result < 0) {
    DeeCompiler_End();
    Dee_Decref(compiler);
    recursive_rwlock_endwrite(&DeeCompiler_Lock);
    return result;
   }
  }
 }

 /* Allocate the varargs symbol for the root-scope. */
 {
  struct symbol *dots = new_unnamed_symbol();
  if unlikely(!dots) goto err_compiler;
  current_basescope->bs_argv    = (struct symbol **)Dee_Malloc(1*sizeof(struct symbol *));
  if unlikely(!current_basescope->bs_argv) goto err_compiler;
  dots->s_type  = SYMBOL_TYPE_ARG;
  dots->s_symid = 0;
  dots->s_flag |= SYMBOL_FALLOC;
  current_basescope->bs_argc    = 1;
  current_basescope->bs_argv[0] = dots;
  current_basescope->bs_varargs = dots;
  current_basescope->bs_flags  |= CODE_FVARARGS;
 }


 /* Save the current exception context. */
 parser_start();

 /* Yield the initial token. */
 if unlikely(yield() < 0)
  ast = NULL;
 else {
  /* Parse statements until the end of the source stream. */
  ast = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST,TOK_EOF);
 }

 if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
       TPPLexer_ClearIfdefStack();

 /* Rethrow all errors that may have occurred during parsing. */
 if (parser_rethrow(ast == NULL))
     Dee_XClear(ast);

 if unlikely(!ast)
    goto err_compiler;

 /* Run an additional optimization pass on the
  * AST before passing it off to the assembler. */
 if (optimizer_flags&OPTIMIZE_FENABLED) {
  result = ast_optimize_all(ast,false);
  /* Rethrow all errors that may have occurred during optimization. */
  if (parser_rethrow(result != 0))
      result = -1;
  if (result)
      goto err_compiler_ast;
 }

 {
  uint16_t refc; struct asm_symbol_ref *refv;
  root_code = code_compile(ast,assembler_flags,&refc,&refv);
  ASSERT(!root_code || !refc);
  ASSERT(!root_code || !refv);
 }
 Dee_Decref(ast);

 /* Rethrow all errors that may have occurred during text assembly. */
 if (parser_rethrow(root_code == NULL))
     Dee_XClear(root_code);

 /* Check for errors during assembly. */
 if unlikely(!root_code) goto err_compiler;

 /* Finally, put together the module itself. */
 result = module_compile(self,root_code,assembler_flags);
 Dee_Decref(root_code);

 /* Rethrow all errors that may have occurred during module linkage. */
 if (parser_rethrow(result != 0))
     result = -1;

 DeeCompiler_End();
 Dee_Decref(compiler);
 recursive_rwlock_endwrite(&DeeCompiler_Lock);
 return result;
err_compiler_ast:
 Dee_Decref(ast);
err_compiler:
 DeeCompiler_End();
 Dee_Decref(compiler);
 recursive_rwlock_endwrite(&DeeCompiler_Lock);
err:
 return -1;
}

PUBLIC int DCALL
DeeModule_LoadSourceStream(DeeObject *__restrict self,
                           DeeObject *__restrict input_file,
                           struct compiler_options *options,
                           int start_line, int start_col) {
 int result = DeeModule_BeginLoading((DeeModuleObject *)self);
 if (result == 0) {
  result = DeeModule_LoadSourceStreamEx((DeeModuleObject *)self,
                                         input_file,
                                         options,
                                         start_line,
                                         start_col,
                                         NULL);
  if unlikely(result)
       DeeModule_FailLoading((DeeModuleObject *)self);
  else DeeModule_DoneLoading((DeeModuleObject *)self);
 }
 return result;
}








struct module_bucket {
 DeeModuleObject *mb_list; /* [0..1][weak] Chain of modules in this bucket. */
};

/* Filesystem-based module hash table. */
PRIVATE size_t                modules_c = 0;    /* [lock(modules_lock)] Amount of modules in-cache. */
PRIVATE size_t                modules_a = 0;    /* [lock(modules_lock)] Allocated hash-map size. */
PRIVATE struct module_bucket *modules_v = NULL; /* [lock(modules_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(modules_lock);
#endif /* !CONFIG_NO_THREADS */

/* Name-based, global module hash table. */
PRIVATE size_t                modules_glob_c = 0;    /* [lock(modules_lock)] Amount of modules in-cache. */
PRIVATE size_t                modules_glob_a = 0;    /* [lock(modules_lock)] Allocated hash-map size. */
PRIVATE struct module_bucket *modules_glob_v = NULL; /* [lock(modules_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */

PRIVATE DeeModuleObject *DCALL
find_file_module(DeeStringObject *__restrict module_file) {
 DeeModuleObject *result = NULL;
#ifdef CONFIG_NOCASE_FS
 dhash_t hash = DeeString_HashCase((DeeObject *)module_file);
#else
 dhash_t hash = DeeString_Hash((DeeObject *)module_file);
#endif
#ifndef CONFIG_NO_THREADS
 ASSERT(rwlock_reading(&modules_lock));
#endif /* !CONFIG_NO_THREADS */
 if (modules_a) {
  result = modules_v[hash % modules_a].mb_list;
  while (result) {
   ASSERTF(result->mo_path,"All modules found in the file cache must have a path assigned");
   ASSERT_OBJECT_TYPE_EXACT(result->mo_path,&DeeString_Type);
   if (
#ifndef CONFIG_NOCASE_FS
       DeeString_HASH(result->mo_path) == hash &&
#endif
       DeeString_SIZE(result->mo_path) == DeeString_SIZE(module_file) &&
#ifdef CONFIG_NOCASE_FS
       MEMCASEEQ(DeeString_STR(result->mo_path),DeeString_STR(module_file),
                 DeeString_SIZE(module_file)*sizeof(char))
#else
       memcmp(DeeString_STR(result->mo_path),DeeString_STR(module_file),
              DeeString_SIZE(module_file)*sizeof(char)) == 0
#endif
       )
       break; /* Found it! */
   result = result->mo_next;
  }
 }
 return result;
}
PRIVATE DeeModuleObject *DCALL
find_glob_module(DeeStringObject *__restrict module_name) {
#ifdef CONFIG_NOCASE_FS
 dhash_t hash = DeeString_HashCase((DeeObject *)module_name);
#else
 dhash_t hash = DeeString_Hash((DeeObject *)module_name);
#endif
 DeeModuleObject *result = NULL;
#ifndef CONFIG_NO_THREADS
 ASSERT(rwlock_reading(&modules_glob_lock));
#endif /* !CONFIG_NO_THREADS */
 if (modules_glob_a) {
  result = modules_glob_v[hash % modules_glob_a].mb_list;
  while (result) {
   ASSERT_OBJECT_TYPE_EXACT(result->mo_name,&DeeString_Type);
   if (
#ifndef CONFIG_NOCASE_FS
       DeeString_HASH(result->mo_name) == hash &&
#endif
       DeeString_SIZE(result->mo_name) == DeeString_SIZE(module_name) &&
#ifdef CONFIG_NOCASE_FS
       MEMCASEEQ(DeeString_STR(result->mo_name),DeeString_STR(module_name),
                 DeeString_SIZE(module_name)*sizeof(char))
#else
       memcmp(DeeString_STR(result->mo_name),DeeString_STR(module_name),
              DeeString_SIZE(module_name)*sizeof(char)) == 0
#endif
       )
       break; /* Found it! */
   result = result->mo_next;
  }
 }
 return result;
}

PRIVATE bool DCALL rehash_file_modules(void) {
 struct module_bucket *new_vector,*biter,*bend,*dst;
 DeeModuleObject *iter,*next;
 size_t new_size = modules_a*2;
#ifndef CONFIG_NO_THREADS
 ASSERT(rwlock_writing(&modules_lock));
#endif /* !CONFIG_NO_THREADS */
 if unlikely(!new_size) new_size = 4;
do_alloc_new_vector:
 new_vector = (struct module_bucket *)Dee_TryCalloc(new_size*sizeof(struct module_bucket));
 if unlikely(!new_vector) {
  if (modules_a != 0) return true; /* Don't actually need to rehash. */
  if (new_size != 1) { new_size = 1; goto do_alloc_new_vector; }
  return false;
 }
 ASSERT(new_size);
 bend = (biter = modules_v)+modules_a;
 for (; biter != bend; ++biter) {
  iter = biter->mb_list;
  while (iter) {
   next = iter->mo_next;
   ASSERTF(iter->mo_path,"All modules found in the file cache must have a path assigned");
   ASSERT_OBJECT_TYPE_EXACT(iter->mo_path,&DeeString_Type);
   /* Re-hash this entry. */
#ifndef CONFIG_NOCASE_FS
   dst = &new_vector[DeeString_HASH((DeeObject *)iter->mo_path) % new_size];
#else
   dst = &new_vector[DeeString_HashCase((DeeObject *)iter->mo_path) % new_size];
#endif
   if ((iter->mo_next = dst->mb_list) != NULL)
        iter->mo_next->mo_pself = &iter->mo_next;
   iter->mo_pself = &dst->mb_list;
   dst->mb_list = iter;
   /* Continue with the next. */
   iter = next;
  }
 }
 Dee_Free(modules_v);
 modules_v = new_vector;
 modules_a = new_size;
 return true;
}
PRIVATE bool DCALL rehash_glob_modules(void) {
 struct module_bucket *new_vector,*biter,*bend,*dst;
 DeeModuleObject *iter,*next;
 size_t new_size = modules_glob_a*2;
#ifndef CONFIG_NO_THREADS
 ASSERT(rwlock_writing(&modules_glob_lock));
#endif /* !CONFIG_NO_THREADS */
 if unlikely(!new_size) new_size = 4;
do_alloc_new_vector:
 new_vector = (struct module_bucket *)Dee_TryCalloc(new_size*sizeof(struct module_bucket));
 if unlikely(!new_vector) {
  if (modules_glob_a != 0) return true; /* Don't actually need to rehash. */
  if (new_size != 1) { new_size = 1; goto do_alloc_new_vector; }
  return false;
 }
 ASSERT(new_size);
 bend = (biter = modules_glob_v)+modules_glob_a;
 for (; biter != bend; ++biter) {
  iter = biter->mb_list;
  while (iter) {
   next = iter->mo_globnext;
   ASSERT_OBJECT_TYPE_EXACT(iter->mo_name,&DeeString_Type);
   /* Re-hash this entry. */
#ifndef CONFIG_NOCASE_FS
   dst = &new_vector[DeeString_HASH((DeeObject *)iter->mo_name) % new_size];
#else
   dst = &new_vector[DeeString_HashCase((DeeObject *)iter->mo_name) % new_size];
#endif
   if ((iter->mo_globnext = dst->mb_list) != NULL)
        iter->mo_globnext->mo_globpself = &iter->mo_globnext;
   iter->mo_globpself = &dst->mb_list;
   dst->mb_list = iter;
   /* Continue with the next. */
   iter = next;
  }
 }
 Dee_Free(modules_glob_v);
 modules_glob_v = new_vector;
 modules_glob_a = new_size;
 return true;
}


PRIVATE bool DCALL
add_file_module(DeeModuleObject *__restrict self) {
 dhash_t hash; struct module_bucket *bucket;
 ASSERT(!self->mo_pself);
 ASSERT_OBJECT_TYPE_EXACT(self->mo_path,&DeeString_Type);
#ifndef CONFIG_NO_THREADS
 ASSERT(rwlock_writing(&modules_lock));
#endif /* !CONFIG_NO_THREADS */
 if (modules_c >= modules_a &&
    !rehash_file_modules()) return false;
 ASSERT(modules_a);
 /* Insert the module into the table. */
#ifdef CONFIG_NOCASE_FS
 hash = DeeString_HashCase((DeeObject *)self->mo_path);
#else
 hash = DeeString_Hash((DeeObject *)self->mo_path);
#endif
 bucket = &modules_v[hash % modules_a];
 if ((self->mo_next = bucket->mb_list) != NULL)
      self->mo_next->mo_pself = &self->mo_next;
 self->mo_pself = &bucket->mb_list;
 bucket->mb_list = self;
 ++modules_c;
 return true;
}

PRIVATE bool DCALL
add_glob_module(DeeModuleObject *__restrict self) {
 dhash_t hash; struct module_bucket *bucket;
 ASSERT(!self->mo_globpself);
 ASSERT(self->mo_name);
#ifndef CONFIG_NO_THREADS
 ASSERT(rwlock_writing(&modules_glob_lock));
#endif /* !CONFIG_NO_THREADS */
 if (modules_glob_c >= modules_glob_a &&
    !rehash_glob_modules()) return false;
 ASSERT(modules_glob_a);
 /* Insert the module into the table. */
#ifdef CONFIG_NOCASE_FS
 hash = DeeString_HashCase((DeeObject *)self->mo_name);
#else
 hash = DeeString_Hash((DeeObject *)self->mo_name);
#endif
 bucket = &modules_glob_v[hash % modules_glob_a];
 if ((self->mo_globnext = bucket->mb_list) != NULL)
      self->mo_globnext->mo_globpself = &self->mo_globnext;
 self->mo_globpself = &bucket->mb_list;
 bucket->mb_list = self;
 ++modules_glob_c;
 return true;
}



INTERN void DCALL
module_unbind(DeeModuleObject *__restrict self) {
 if (self->mo_pself) {
#ifndef CONFIG_NO_THREADS
  rwlock_write(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
  if ((*self->mo_pself = self->mo_next) != NULL)
        self->mo_next->mo_pself = self->mo_pself;
  if (!--modules_c) {
   Dee_Free(modules_v);
   modules_v = NULL;
   modules_a = 0;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
 }
 if (self->mo_globpself) {
#ifndef CONFIG_NO_THREADS
  rwlock_write(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
  if ((*self->mo_globpself = self->mo_globnext) != NULL)
        self->mo_globnext->mo_globpself = self->mo_globpself;
  if (!--modules_glob_c) {
   Dee_Free(modules_glob_v);
   modules_glob_v = NULL;
   modules_glob_a = 0;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
 }
}

PUBLIC DREF DeeObject *DCALL
DeeModule_OpenFile(DeeObject *__restrict source_pathname,
                   DeeObject *module_name, uint16_t file_class,
                   struct compiler_options *options) {
 DREF DeeModuleObject *result; int load_error;
 DREF DeeObject *input_stream = NULL;
 DREF DeeStringObject *abs_source_pathname;
#ifndef CONFIG_NO_DEC
 /* Dismiss attempt to load DEC files when the DEC loader has been disabled. */
 if ((file_class&MODULE_FILECLASS_MASK) == MODULE_FILECLASS_COMPILED &&
      options && (options->co_decloader&DEC_FDISABLE)) {
  /* Throw an error instead if the caller requested this. */
  if (file_class&MODULE_FILECLASS_THROWERROR) {
   err_file_not_found(DeeString_STR(source_pathname));
   return NULL;
  }
  return ITER_DONE;
 }
#endif

 abs_source_pathname = (DREF DeeStringObject *)make_absolute(source_pathname);
 if unlikely(!abs_source_pathname) return NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
 /* Search for an existing instance of this module. */
 result = find_file_module(abs_source_pathname);
 if (result) {
  Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
  goto found_existing_module;
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&modules_lock);
#endif /* !CONFIG_NO_THREADS */

#ifndef CONFIG_NO_DEX
 if ((file_class&MODULE_FILECLASS_MASK) != MODULE_FILECLASS_EXTENSION)
#endif
 {
  /* Open the module's source file stream. */
  input_stream = DeeFile_Open((DeeObject *)abs_source_pathname,OPEN_FRDONLY,0);
  if unlikely(!ITER_ISOK(input_stream)) {
   result = (DREF DeeModuleObject *)input_stream;
   if (input_stream == ITER_DONE &&
       file_class&MODULE_FILECLASS_THROWERROR)
       err_file_not_found(DeeString_STR(source_pathname)),
       result = NULL;
   goto done;
  }
 }

 /* Create a new module. */
 if (!module_name) {
  char  *name = DeeString_STR(source_pathname);
  size_t size = DeeString_SIZE(source_pathname);
  char *name_end,*name_start;
  DREF DeeObject *name_object;
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
#ifndef CONFIG_NO_DEC
  if (file_class == MODULE_FILECLASS_COMPILED &&
     *name_start == '.') ++name_start;
#endif

  /* Get rid of a file extension in the module name. */
  while (name_end != name_start && name_end[-1] != '.') --name_end;
  while (name_end != name_start && name_end[-1] == '.') --name_end;
  if (name_end == name_start) name_end = name+size;
  name_object = DeeString_NewSized(name_start,(size_t)(name_end-name_start));
  if unlikely(!name_object) goto err;
#ifndef CONFIG_NO_DEX
  if ((file_class&MODULE_FILECLASS_MASK) == MODULE_FILECLASS_EXTENSION)
   result = (DREF DeeModuleObject *)DeeDex_New(name_object);
  else
#endif
  {
   result = (DREF DeeModuleObject *)DeeModule_New(name_object);
  }
  Dee_Decref(name_object);
 } else {
  DeeModuleObject *existing_module;
  /* Check if the module is already loaded in the global cache. */
#ifndef CONFIG_NO_THREADS
  rwlock_read(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
  result = find_glob_module((DeeStringObject *)module_name);
  if (result) {
   Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
   rwlock_endread(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
   goto found_existing_module;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
  /* Create a new module. */
#ifndef CONFIG_NO_DEX
  if ((file_class&MODULE_FILECLASS_MASK) == MODULE_FILECLASS_EXTENSION)
   result = (DREF DeeModuleObject *)DeeDex_New(module_name);
  else
#endif
  {
   result = (DREF DeeModuleObject *)DeeModule_New(module_name);
  }
  /* Add the module to the global module cache. */
set_global_module:
#ifndef CONFIG_NO_THREADS
  rwlock_write(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
  existing_module = find_glob_module((DeeStringObject *)module_name);
  if unlikely(existing_module) {
   /* The module got created in the mean time. */
   Dee_Incref(existing_module);
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
   Dee_Decref(result);
   result = existing_module;
   goto found_existing_module;
  }
  /* Add the module to the global cache. */
  if unlikely(!add_glob_module(result)) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
   /* Try to collect some memory, then try again. */
   if (Dee_CollectMemory(1)) goto set_global_module;
   goto err_r;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
 }
 /* Set the module's name within its data structure. */
 ASSERT_OBJECT_TYPE_EXACT(abs_source_pathname,&DeeString_Type);
 result->mo_path = (DREF DeeStringObject *)abs_source_pathname;
 Dee_Incref(abs_source_pathname);
 /* Now add the module to the file-cache. */
set_file_module:
#ifndef CONFIG_NO_THREADS
 rwlock_write(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
 {
  /* Check if the file-system version of this module got added in the mean time. */
  DeeModuleObject *existing_module;
  existing_module = find_file_module(abs_source_pathname);
  if unlikely(existing_module) {
   Dee_Incref(existing_module);
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
   Dee_Decref(result);
   result = existing_module;
   goto found_existing_module;
  }
  /* Add the module to the file-cache. */
  if unlikely(!add_file_module(result)) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
   /* Try to collect some memory, then try again. */
   if (Dee_CollectMemory(1)) goto set_file_module;
   goto err_r;
  }
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&modules_lock);
#endif /* !CONFIG_NO_THREADS */
found_existing_module:
 /* Now to actually load the module. */
 load_error = DeeModule_BeginLoading(result);
 if (load_error == 0) {
#ifndef CONFIG_NO_DEX
  if (DeeDex_Check(result)) {
   load_error = dex_load_file((DeeDexObject *)result,
                              (DeeObject *)abs_source_pathname);
   if (load_error > 0) goto file_not_found;
  } else
#endif
  {
   /* If the input stream hasn't been opened yet, open it now. */
   if (!input_stream) {
    input_stream = DeeFile_Open((DeeObject *)abs_source_pathname,OPEN_FRDONLY,0);
    if (ITER_ISOK(input_stream)) goto do_load_input_stream;
    if (input_stream == ITER_DONE) {
     /* Propagate file-not-found by returning ITER_DONE. */
#if !defined(CONFIG_NO_DEX) || !defined(CONFIG_NO_DEC)
file_not_found:
#endif
     DeeModule_FailLoading(result);
     Dee_Decref(result);
     result = (DREF DeeModuleObject *)ITER_DONE;
     if (file_class&MODULE_FILECLASS_THROWERROR)
         err_file_not_found(DeeString_STR(source_pathname)),
         result = NULL;
     goto done;
    }
    load_error = -1;
   } else {
do_load_input_stream:
#ifndef CONFIG_NO_DEC
    if ((file_class&MODULE_FILECLASS_MASK) == MODULE_FILECLASS_COMPILED) {
     load_error = DeeModule_OpenDec(result,input_stream,options,
                                   (DeeStringObject *)abs_source_pathname);
     if (load_error > 0)
         goto file_not_found;
    } else
#endif /* !CONFIG_NO_DEC */
    {
     load_error = DeeModule_LoadSourceStreamEx(result,
                                               input_stream,
                                               options,
                                               0,
                                               0,
                                              (DeeStringObject *)source_pathname);
    }
   }
  }
  /* Depending on a load error having occurred, either signify
   * that the module has been loaded, or failed to be loaded. */
  if unlikely(load_error) {
   DeeModule_FailLoading(result);
   Dee_Clear(result);
  } else {
   DeeModule_DoneLoading(result);
  }
 }
done:
 if (input_stream != ITER_DONE)
     Dee_XDecref(input_stream);
 Dee_Decref(abs_source_pathname);
 return (DREF DeeObject *)result;
err_r:
 Dee_Decref(result);
err:
 result = NULL;
 goto done;
}
PUBLIC DREF DeeObject *DCALL
DeeModule_OpenFileString(char const *__restrict source_pathname,
                         DeeObject *module_name, uint16_t file_class,
                         struct compiler_options *options) {
 DREF DeeObject *pathname_string,*result;
 pathname_string = DeeString_New(source_pathname);
 if unlikely(!pathname_string) return NULL;
 result = DeeModule_OpenFile(pathname_string,module_name,file_class,options);
 Dee_Decref(pathname_string);
 return result;
}


/* Very similar to `DeeModule_OpenMemory()', and used to implement it,
 * however source data is made available using a stream object derived
 * from `file from deemon' */
PUBLIC DREF DeeObject *DCALL
DeeModule_OpenStream(DeeObject *__restrict source_pathname,
                     DeeObject *module_name,
                     DeeObject *__restrict source_stream,
                     int start_line, int start_col,
                     struct compiler_options *options) {
 DREF DeeModuleObject *result; int load_error;
 /* Create a new module. */
 if (!module_name) {
  char  *name = DeeString_STR(source_pathname);
  size_t size = DeeString_SIZE(source_pathname);
  char *name_end,*name_start;
  DREF DeeObject *name_object;
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
  name_object = DeeString_NewSized(name_start,(size_t)(name_end-name_start));
  if unlikely(!name_object) goto err;
  result = (DREF DeeModuleObject *)DeeModule_New(name_object);
  Dee_Decref(name_object);
 } else {
  DeeModuleObject *existing_module;
  /* Check if the module is already loaded in the global cache. */
#ifndef CONFIG_NO_THREADS
  rwlock_read(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
  result = find_glob_module((DeeStringObject *)module_name);
  if (result) {
   Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
   rwlock_endread(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
   goto found_existing_module;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
  /* Create a new module. */
  result = (DREF DeeModuleObject *)DeeModule_New(module_name);
  /* Add the module to the global module cache. */
set_global_module:
#ifndef CONFIG_NO_THREADS
  rwlock_write(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
  existing_module = find_glob_module((DeeStringObject *)module_name);
  if unlikely(existing_module) {
   /* The module got created in the mean time. */
   Dee_Incref(existing_module);
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
   Dee_Decref(result);
   result = existing_module;
   goto found_existing_module;
  }
  /* Add the module to the global cache. */
  if unlikely(!add_glob_module(result)) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
   /* Try to collect some memory, then try again. */
   if (Dee_CollectMemory(1)) goto set_global_module;
   goto err_r;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
 }
found_existing_module:
 load_error = DeeModule_BeginLoading(result);
 if (load_error != 0) goto done;
 /* If the input stream hasn't been opened yet, open it now. */
 load_error = DeeModule_LoadSourceStreamEx(result,
                                           source_stream,
                                           options,
                                           start_line,
                                           start_col,
                                          (DeeStringObject *)source_pathname);
 /* Depending on a load error having occurred, either signify
  * that the module has been loaded, or failed to be loaded. */
 if unlikely(load_error) {
  DeeModule_FailLoading(result);
  Dee_Clear(result);
 } else {
  DeeModule_DoneLoading(result);
 }
done:
 return (DREF DeeObject *)result;
err_r:
 Dee_Decref(result);
err:
 result = NULL;
 goto done;
}
PUBLIC DREF DeeObject *DCALL
DeeModule_OpenStreamString(char const *__restrict source_pathname,
                           DeeObject *module_name,
                           DeeObject *__restrict source_stream,
                           int start_line, int start_col,
                           struct compiler_options *options) {
 DREF DeeObject *source_pathname_object,*result;
 source_pathname_object = DeeString_New(source_pathname);
 if unlikely(!source_pathname_object) return NULL;
 result = DeeModule_OpenStream(source_pathname_object,
                               module_name,
                               source_stream,
                               start_line,
                               start_col,
                               options);
 Dee_Decref(source_pathname_object);
 return result;
}


/* Construct a module from a memory source-code blob.
 * NOTE: Unlike `DeeModule_OpenFile()', this function will not bind `source_pathname'
 *       to the returned module, meaning that the module object returned will be entirely
 *       anonymous, except for when `module_name' was passed as non-NULL, in which case
 *       the returned module will be made available as a global import with that same name,
 *       and be available for later addressing using `DeeModule_Open()'
 * @param: source_pathname: The filename of the source file from which data (supposedly) originates.
 *                          Used by `#include' directives, as well as `__FILE__' and ddi information.
 * @param: module_name:     When non-NULL, use this as the module's actual name.
 *                          Also: register the module as a global module.
 * @param: data:            A pointer to the raw source-code that should be parsed as
 *                          the deemon source for the module.
 * @param: data_size:       The size of the `data' blob (in characters)
 * @param: start_line:      The starting line number of the data blob (zero-based)
 * @param: start_col:       The starting column offset of the data blob (zero-based)
 * @param: options:         An optional set of extended compiler options. */
PUBLIC DREF DeeObject *DCALL
DeeModule_OpenMemory(DeeObject *__restrict source_pathname,
                     DeeObject *module_name, char const *__restrict data,
                     size_t data_size, int start_line, int start_col,
                     struct compiler_options *options) {
 DREF DeeObject *source_stream,*result;
 source_stream = DeeFile_OpenRoMemory(data,data_size);
 if unlikely(!source_stream) return NULL;
 result = DeeModule_OpenStream(source_pathname,
                               module_name,
                               source_stream,
                               start_line,
                               start_col,
                               options);
 DeeFile_ReleaseMemory(source_stream);
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeModule_OpenMemoryString(char const *__restrict source_pathname,
                           DeeObject *module_name, char const *__restrict data,
                           size_t data_size, int start_line, int start_col,
                           struct compiler_options *options) {
 DREF DeeObject *source_pathname_object,*result;
 source_pathname_object = DeeString_New(source_pathname);
 if unlikely(!source_pathname_object) return NULL;
 result = DeeModule_OpenMemory(source_pathname_object,
                               module_name,
                               data,
                               data_size,
                               start_line,
                               start_col,
                               options);
 Dee_Decref(source_pathname_object);
 return result;
}



PUBLIC DREF DeeObject *DCALL
DeeModule_NewString(char const *__restrict name) {
 DREF DeeObject *name_object,*result;
 name_object = DeeString_New(name);
 if unlikely(!name_object) return NULL;
 result = DeeModule_New(name_object);
 Dee_Decref(name_object);
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeModule_New(DeeObject *__restrict name) {
 DeeModuleObject *result;
 ASSERT_OBJECT_TYPE_EXACT(name,&DeeString_Type);
 result = DeeGCObject_CALLOC(DeeModuleObject);
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&DeeModule_Type);
 result->mo_name    = (DeeStringObject *)name;
 result->mo_bucketv = empty_module_buckets;
 Dee_Incref(name);
 weakref_support_init(result);
 DeeGC_Track((DREF DeeObject *)result);
 return (DREF DeeObject *)result;
}


#define SOURCE_EXTENSION_MAX  3
struct ext_def {
 char    source_ext[SOURCE_EXTENSION_MAX];
 uint8_t source_class;
};

PRIVATE struct ext_def const extensions[] = {
#ifndef CONFIG_NO_DEC
    { {'d','e','c'}, MODULE_FILECLASS_COMPILED },
#endif
    { {'d','e','e'}, MODULE_FILECLASS_SOURCE },
#ifndef CONFIG_NO_DEX
#ifdef CONFIG_HOST_WINDOWS
    { {'d','l','l'}, MODULE_FILECLASS_EXTENSION },
#else
    { {'s','o',0}, MODULE_FILECLASS_EXTENSION },
#endif
#endif
};

PRIVATE ATTR_COLD void DCALL
err_invalid_module_name(DeeObject *__restrict module_name) {
 DeeError_Throwf(&DeeError_ValueError,
                 "%r is not a valid module name",
                 module_name);
}

PRIVATE ATTR_COLD void DCALL
err_module_not_found(DeeObject *__restrict module_name) {
 DeeError_Throwf(&DeeError_FileNotFound,
                 "Module %r could not be found",
                 module_name);
}

#ifdef CONFIG_LITTLE_ENDIAN
#define ENCODE4(a,b,c,d) ((d)<<24|(c)<<16|(b)<<8|(a))
#else
#define ENCODE4(a,b,c,d) ((d)|(c)<<8|(b)<<16|(a)<<24)
#endif


PRIVATE DREF DeeObject *DCALL
DeeModule_DoGet(char const *__restrict name,
                size_t size, dhash_t hash) {
 DREF DeeModuleObject *result = NULL;
 /* Check if the caller requested the builtin deemon module. */
 if (size == DeeString_SIZE(&str_deemon) &&
     hash == DeeString_Hash(&str_deemon) &&
     memcmp(name,DeeString_STR(&str_deemon),
            DeeString_SIZE(&str_deemon)*sizeof(char)) == 0) {
  /* Yes, they did. */
  result = get_deemon_module();
  Dee_Incref(result);
  goto done;
 }
#ifndef CONFIG_NO_THREADS
 rwlock_read(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
 if (modules_glob_a) {
  result = modules_glob_v[hash % modules_glob_a].mb_list;
  while (result) {
   ASSERT_OBJECT_TYPE_EXACT(result->mo_name,&DeeString_Type);
   if (DeeString_SIZE(result->mo_name) == size &&
#ifdef CONFIG_NOCASE_FS
       MEMCASEEQ(DeeString_STR(result->mo_name),name,
                 size*sizeof(char))
#else
       memcmp(DeeString_STR(result->mo_name),name,
              size*sizeof(char)) == 0
#endif
       ) {
    Dee_Incref(result);
    break; /* Found it! */
   }
   result = result->mo_next;
  }
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
done:
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeModule_Get(DeeObject *__restrict module_name) {
 return DeeModule_DoGet(DeeString_STR(module_name),
                        DeeString_SIZE(module_name),
#ifdef CONFIG_NOCASE_FS
                        DeeString_HashCase(module_name)
#else
                        DeeString_Hash(module_name)
#endif
                        );
}
PUBLIC DREF DeeObject *DCALL
DeeModule_GetString(char const *__restrict module_name) {
 size_t length = strlen(module_name);
 return DeeModule_DoGet(module_name,length,
#ifdef CONFIG_NOCASE_FS
                        hash_caseptr(module_name,length)
#else
                        hash_ptr(module_name,length)
#endif
                        );
}


#if 0
#define IS_VALID_MODULE_CHARACTER(ch) \
     (!((ch) == '/' || (ch) == '\\' || (ch) == '|' || \
        (ch) == '&' || (ch) == '~' || (ch) == '%' || \
        (ch) == '$' || (ch) == '?' || (ch) == '!' || \
        (ch) == '*' || (ch) == '\'' || (ch) == '\"'))
#else
#define IS_VALID_MODULE_CHARACTER(ch) \
     ((DeeUni_Flags(ch)& \
      (UNICODE_FALPHA|UNICODE_FLOWER|UNICODE_FUPPER|UNICODE_FTITLE| \
       UNICODE_FDECIMAL|UNICODE_FSYMSTRT|UNICODE_FSYMCONT)) || \
     ((ch) == '-' || (ch) == '=' || (ch) == ',' || (ch) == '(' || \
      (ch) == ')' || (ch) == '[' || (ch) == ']' || (ch) == '{' || \
      (ch) == '}' || (ch) == '<' || (ch) == '>' || (ch) == '+'))
#endif

PUBLIC DREF DeeObject *DCALL
DeeModule_Open(DeeObject *__restrict module_name,
               struct compiler_options *options,
               bool throw_error) {
 DREF DeeObject *path;
 DREF DeeModuleObject *result;
 DeeListObject *paths; size_t i;
 ASSERT_OBJECT_TYPE_EXACT(module_name,&DeeString_Type);
 /* First off: Check if this is a request for the builtin `deemon' module.
  * NOTE: This check is always done in case-sensitive mode! */
 if (DeeString_SIZE(module_name) == DeeString_SIZE(&str_deemon) &&
     DeeString_Hash(module_name) == DeeString_Hash(&str_deemon) &&
     memcmp(DeeString_STR(module_name),DeeString_STR(&str_deemon),
            DeeString_SIZE(&str_deemon)*sizeof(char)) == 0) {
  /* Yes, it is. */
  result = get_deemon_module();
  Dee_Incref(result);
  goto done;
 }

 /* Search for a cache entry for this module in the global module cache. */
#ifndef CONFIG_NO_THREADS
 rwlock_read(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
 result = find_glob_module((DeeStringObject *)module_name);
 Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */
 if (result) {
  if likely(result->mo_flags & MODULE_FDIDLOAD)
     goto done; /* Found a cached module. */
  Dee_Decref(result);
 }

 /* Default case: Must load a new module. */
 paths = DeeModule_GetPath();
 DeeList_LockRead(paths);
 for (i = 0; i < DeeList_SIZE(paths); ++i) {
  path = DeeList_GET(paths,i);
  Dee_Incref(path);
  DeeList_LockEndRead(paths);
  if (DeeString_Check(path)) {
   size_t j,length,module_ext_start;
#ifndef CONFIG_NO_DEC
   size_t module_name_start,module_name_size;
#endif
   struct unicode_printer printer = UNICODE_PRINTER_INIT;
   DREF DeeStringObject *module_path;
   /* Try to speed this up by reserving some memory in the printer. */
   unicode_printer_allocate(&printer,
                            DeeString_SIZE(path) + 1 +
#ifndef CONFIG_NO_DEC
                            1 + /* The `.' prefixed before DEC files. */
#endif
                            DeeString_SIZE(module_name) +
                            1 + SOURCE_EXTENSION_MAX,
                            STRING_WIDTH_COMMON(DeeString_WIDTH(path),
                                                DeeString_WIDTH(module_name)));
   /* Start out by printing the given path. */
   if unlikely(unicode_printer_printstring(&printer,path) < 0)
      goto err_path_printer;
   /* Strip trailing slashes. */
   for (;;) {
    uint32_t ch;
    length = UNICODE_PRINTER_LENGTH(&printer);
    if (!length) break;
    ch = UNICODE_PRINTER_GETCHAR(&printer,length - 1);
    if (!ISSEP(ch)) break;
    unicode_printer_truncate(&printer,length - 1);
   }
   /* Append a single slash. */
   if unlikely(unicode_printer_putascii(&printer,SEP))
      goto err_path_printer;
   /* Append the module filename. */
   j = UNICODE_PRINTER_LENGTH(&printer);
#ifndef CONFIG_NO_DEC
   module_name_start = j;
#endif
   if unlikely(unicode_printer_printstring(&printer,module_name) < 0)
      goto err_path_printer;
   module_ext_start = UNICODE_PRINTER_LENGTH(&printer);
   /* Validate the path and convert `.' into `/' */
   for (; j < module_ext_start; ++j) {
    uint32_t ch;
    ch = UNICODE_PRINTER_GETCHAR(&printer,j);
    if (ch != '.') {
     if (!IS_VALID_MODULE_CHARACTER(ch)) {
err_invalid_name:
      err_invalid_module_name(module_name);
err_path_printer:
      unicode_printer_fini(&printer);
      goto err_path;
     }
     continue;
    }
    /* Make sure the next character isn't another `.'
     * (which isn't allowed in system module names) */
    if (j + 1 >= module_ext_start)
        goto err_invalid_name;
    if (UNICODE_PRINTER_GETCHAR(&printer,j+1) == '.')
        goto err_invalid_name;
    /* Convert to a slash. */
    UNICODE_PRINTER_SETCHAR(&printer,j,SEP);
#ifndef CONFIG_NO_DEC
    module_name_start = j + 1;
#endif
   }
   /* Reserve characters for the extension. */
   if (unicode_printer_reserve(&printer,
#ifndef CONFIG_NO_DEC
                               1 +
#endif
                               1 + SOURCE_EXTENSION_MAX) < 0)
       goto err_path_printer;
   UNICODE_PRINTER_SETCHAR(&printer,module_ext_start,'.');
   ++module_ext_start;
#ifndef CONFIG_NO_DEC
   module_name_size = module_ext_start - module_name_start;
#endif

   /* pack together the full module path. */
   module_path = (REF DeeStringObject *)unicode_printer_pack(&printer);
   if unlikely(!module_path) goto err_path;
   j = 0;
   do {
#ifndef CONFIG_NO_DEC
    if (extensions[j].source_class == MODULE_FILECLASS_COMPILED) {
     /* Shift the filename. */
     DeeString_Memmove(module_path,
                       module_name_start + 1,
                       module_name_start,
                       module_name_size);
     DeeString_SetChar(module_path,module_name_start,'.');
     DeeString_SetChar(module_path,module_ext_start + 1,'d');
     DeeString_SetChar(module_path,module_ext_start + 2,'e');
     DeeString_SetChar(module_path,module_ext_start + 3,'c');
     /* Open the module source file. */
     result = (DREF DeeModuleObject *)DeeModule_OpenFile((DeeObject *)module_path,module_name,
                                                          MODULE_FILECLASS_COMPILED,
                                                          options);
     if (result != (DREF DeeModuleObject *)ITER_DONE)
         goto done_path;
     /* Undo the path modifications done for the leading `.' of DEC files. */
     DeeString_Memmove(module_path,
                       module_name_start,
                       module_name_start + 1,
                       module_name_size);
     DeeString_PopbackAscii(module_path);
    } else
#endif
    {
     size_t k;
     for (k = 0; k < SOURCE_EXTENSION_MAX; ++k) {
      DeeString_SetChar(module_path,module_ext_start + k,
                        extensions[j].source_ext[k]);
     }
     result = (DREF DeeModuleObject *)DeeModule_OpenFile((DeeObject *)module_path,module_name,
                                                          extensions[j].source_class,
                                                          options);
     if (result != (DREF DeeModuleObject *)ITER_DONE) {
done_path:
      Dee_Decref(module_path);
      Dee_Decref(path);
      goto done;
     }
    }
   } while (++j < COMPILER_LENOF(extensions));
   Dee_Decref(module_path);
  } else {
   /* `path' isn't a string */
  }
  Dee_Decref(path);
  DeeList_LockRead(paths);
 }
 DeeList_LockEndRead(paths);
 if (!throw_error)
      return ITER_DONE;
 err_module_not_found(module_name);
err:
 return NULL;
err_path:
 Dee_Decref(path);
 goto err;
done:
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeModule_OpenString(char const *__restrict module_name,
                     struct compiler_options *options,
                     bool throw_error) {
 DREF DeeObject *name_object,*result;
 name_object = DeeString_New(module_name);
 if unlikely(!name_object) return NULL;
 result = DeeModule_Open(name_object,options,throw_error);
 Dee_Decref(name_object);
 return result;
}



PUBLIC DREF DeeObject *DCALL
DeeModule_OpenRelative(DeeObject *__restrict module_name,
                       char const *__restrict module_pathname,
                       size_t module_pathsize,
                       struct compiler_options *options,
                       bool throw_error) {
 DREF DeeModuleObject *result; size_t i;
 DREF DeeStringObject *module_path;
 char *iter,*begin,*end,ch,*flush_start;
#ifndef CONFIG_NO_DEC
 size_t module_name_start;
 size_t module_name_size;
 size_t module_ext_start;
#endif
 struct unicode_printer full_path = UNICODE_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(module_name,&DeeString_Type);
 flush_start = begin = DeeString_AsUtf8(module_name);
 if unlikely(!begin) goto err;
 end = (iter = begin)+WSTR_LENGTH(begin);
 /* Shouldn't happen: Not actually a relative module name. */
 if (begin == end || *begin != '.')
     return DeeModule_Open(module_name,options,throw_error);
 if (unicode_printer_print(&full_path,module_pathname,module_pathsize) < 0)
     goto err;
 /* Add a trailing slash is necessary. */
 if (module_pathsize && !ISSEP(module_pathname[module_pathsize-1]) &&
     unicode_printer_putascii(&full_path,SEP)) goto err;
 /* Interpret and process the given module name. */
#ifndef CONFIG_NO_DEC
 module_name_start = UNICODE_PRINTER_LENGTH(&full_path);
#endif
next:
 ch = *iter++;
 if (ch == '\0') {
  if (iter == end+1)
      goto done;
  goto next;
 }
 if (ch != '.') {
  /* Validate that this character is allowed in module names. */
  uint32_t ch32;
  --iter;
  ch32 = utf8_readchar((char const **)&iter,end);
  if (!IS_VALID_MODULE_CHARACTER(ch32)) {
   err_invalid_module_name(module_name);
   goto err;
  }
  goto next;
 }
 /* Flush the current part and append another slash. */
 if (flush_start != iter-1) {
  if (unicode_printer_print(&full_path,flush_start,
                           (size_t)(iter-flush_start)-1) < 0 ||
      unicode_printer_putascii(&full_path,SEP))
      goto err;
 }
 /* Handle parent directory references. */
 for (; *iter == '.'; ++iter) {
  if (UNICODE_PRINTER_LENGTH(&full_path) != 0) {
   size_t old_length,new_end;
   old_length = UNICODE_PRINTER_LENGTH(&full_path);
   while (old_length) {
    uint32_t ch;
    ch = UNICODE_PRINTER_GETCHAR(&full_path,old_length-1);
    if (!ISSEP(ch)) break;
    --old_length;
   }
   new_end = (size_t)unicode_printer_memchr(&full_path,'/',0,old_length);
#ifdef CONFIG_HOST_WINDOWS
   {
    size_t temp;
    temp = (size_t)unicode_printer_memchr(&full_path,'\\',0,old_length);
    if (new_end > temp)
        new_end = temp;
   }
#endif
   if (new_end != (size_t)-1) {
    /* Truncate the existing path. */
    ++new_end;
    unicode_printer_truncate(&full_path,new_end);
    continue;
   }
  }
  /* Append a host-specific parent directory reference. */
  if (unicode_printer_printascii(&full_path,".." SEP_S,3) < 0)
      goto err;
 }
#ifndef CONFIG_NO_DEC
 module_name_start = UNICODE_PRINTER_LENGTH(&full_path);
#endif
 flush_start = iter;
 goto next;
done:
 --iter;
 /* Print the remainder. */
 if (iter > flush_start) {
  if (unicode_printer_print(&full_path,flush_start,
                           (size_t)(iter-flush_start)) < 0)
      goto err;
 }
#ifndef CONFIG_NO_DEC
 module_name_size = (UNICODE_PRINTER_LENGTH(&full_path) -
                     module_name_start) + 1;
#endif
 /* With the full path now printed, reserve memory for the extension. */
 {
  dssize_t temp;
  temp = unicode_printer_reserve(&full_path,
#ifndef CONFIG_NO_DEC
                                 1 +
#endif
                                 1 + SOURCE_EXTENSION_MAX);
  if unlikely(temp == -1) goto err;
  /* NOTE: The remainder is written in the source-class loop below. */
  UNICODE_PRINTER_SETCHAR(&full_path,temp,'.');
 }
 module_path = (DREF DeeStringObject *)unicode_printer_pack(&full_path);
 if unlikely(!module_path) goto err_noprinter;
 /* Loop through known extensions and classes while trying to find what belongs. */
#ifndef CONFIG_NO_DEC
 module_ext_start = DeeString_WLEN(module_path);
 module_name_start = (module_ext_start -
                     (1+SOURCE_EXTENSION_MAX+module_name_size));
 module_ext_start -= SOURCE_EXTENSION_MAX + 1;
#else
 module_ext_start = DeeString_WLEN(module_path) - SOURCE_EXTENSION_MAX;
#endif
 i = 0;
 do {
#ifndef CONFIG_NO_DEC
  if (extensions[i].source_class == MODULE_FILECLASS_COMPILED) {
   DeeString_Memmove(module_path,
                     module_name_start+1,
                     module_name_start,
                     module_name_size);
   DeeString_SetChar(module_path,module_name_start,'.');
   DeeString_SetChar(module_path,module_ext_start + 1,'d');
   DeeString_SetChar(module_path,module_ext_start + 2,'e');
   DeeString_SetChar(module_path,module_ext_start + 3,'c');
   result = (DREF DeeModuleObject *)DeeModule_OpenFile((DeeObject *)module_path,NULL,
                                                        extensions[i].source_class,
                                                        options);
   if (result == (DREF DeeModuleObject *)ITER_DONE) {
    DeeString_Memmove(module_path,
                      module_name_start,
                      module_name_start + 1,
                      module_name_size);
    DeeString_PopbackAscii(module_path);
   }
  } else
#endif
  {
   /* Graft the current extension onto the path. */
   size_t j;
   for (j = 0; j < SOURCE_EXTENSION_MAX; ++j) {
    DeeString_SetChar(module_path,module_ext_start + j,
                      extensions[i].source_ext[j]);
   }
   /* NOTE: We pass NULL for `module_name' to this function, so it can figure
    *       out what the module's name is itself, while also not attempting to
    *       register it as a global module, yet still register it under its
    *       absolute filename. */
   result = (DREF DeeModuleObject *)DeeModule_OpenFile((DeeObject *)module_path,NULL,
                                                        extensions[i].source_class,
                                                        options);
  }
  if (result != (DREF DeeModuleObject *)ITER_DONE) break;
 } while (++i != COMPILER_LENOF(extensions));
 Dee_Decref(module_path);
 /* Throw an error if the module could not be found
  * and if we're not supposed to return `ITER_DONE'. */
 if (result == (DREF DeeModuleObject *)ITER_DONE &&
     throw_error) {
  err_module_not_found(module_name);
  result = NULL;
 }
 return (DREF DeeObject *)result;
err:
 unicode_printer_fini(&full_path);
err_noprinter:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeModule_OpenRelativeString(char const *__restrict module_name,
                             char const *__restrict module_pathname,
                             size_t module_pathsize,
                             struct compiler_options *options,
                             bool throw_error) {
 DREF DeeObject *name_object,*result;
 name_object = DeeString_New(module_name);
 if unlikely(!name_object) return NULL;
 result = DeeModule_OpenRelative(name_object,
                                 module_pathname,
                                 module_pathsize,
                                 options,throw_error);
 Dee_Decref(name_object);
 return result;
}

INTERN DREF DeeObject *DCALL
DeeModule_Import(DeeObject *__restrict module_name,
                 struct compiler_options *options,
                 bool throw_error) {
 DREF DeeObject *result;
 struct code_frame *frame = DeeThread_Self()->t_exec;
 if (frame) {
  DeeStringObject *path; char *begin,*end;
  /* Load the path of the currently executing code (for relative imports). */
  ASSERT_OBJECT_TYPE_EXACT(frame->cf_func,&DeeFunction_Type);
  ASSERT_OBJECT_TYPE_EXACT(frame->cf_func->fo_code,&DeeCode_Type);
  ASSERT_OBJECT_TYPE(frame->cf_func->fo_code->co_module,&DeeModule_Type);
  path = frame->cf_func->fo_code->co_module->mo_path;
  if unlikely(!path) {
   DeeError_Throwf(&DeeError_FileNotFound,
                   "The calling module %k has no associated filesystem location",
                   frame->cf_func->fo_code->co_module);
   return NULL;
  }
  ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
  end = (begin = DeeString_STR(path))+DeeString_SIZE(path);
  /* Find the end of the current path. */
  while (begin != end && !ISSEP(end[-1])) --end;
  result = DeeModule_OpenRelative(module_name,begin,(size_t)(end-begin),options,throw_error);
 } else {
  /* Without an execution frame, dismiss the relative import() code handling. */
  result = DeeModule_Open(module_name,options,throw_error);
 }
 return result;
}



PUBLIC DeeListObject DeeModule_Path = {
    OBJECT_HEAD_INIT(&DeeList_Type),
    /* .l_alloc = */0,
    /* .l_size  = */0,
    /* .l_elem  = */NULL
#ifndef CONFIG_NO_THREADS
    ,
    /* .l_lock  = */RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
};

#ifdef CONFIG_HOST_WINDOWS
#define DELIM ';'
#else
#define DELIM ':'
#endif


#ifdef CONFIG_DEEMON_HOME
PRIVATE DEFINE_STRING(default_deemon_home,CONFIG_DEEMON_HOME);
#endif


#ifdef CONFIG_HOST_UNIX
INTERN DREF DeeObject *DCALL
unix_readlink(char const *__restrict path) {
 DREF DeeObject *result; char *buffer; int error;
 size_t bufsize,new_size; dssize_t req_size;
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 bufsize = PATH_MAX;
 buffer  = ascii_printer_alloc(&printer,bufsize);
 if unlikely(!buffer) goto err;
 for (;;) {
  struct stat st;
  req_size = readlink(path,buffer,bufsize+1);
  if unlikely(req_size < 0) {
handle_error:
   error = errno;
   DeeError_SysThrowf(&DeeError_FSError,error,
                      "Failed to read symbolic link %q",
                      path);
   goto err;
  }
  if ((size_t)req_size <= bufsize) break;
  if (lstat(path,&st))
      goto handle_error;
  /* Ensure that this is still a symbolic link. */
  if (!S_ISLNK(st.st_mode)) { error = EINVAL; goto handle_error; }
  new_size = (size_t)st.st_size;
  if (new_size <= bufsize) break; /* Shouldn't happen, but might due to race conditions? */
  buffer = ascii_printer_alloc(&printer,new_size-bufsize);
  if unlikely(!buffer) goto err;
  buffer -= bufsize;
  bufsize = new_size;
 }
 /* Release unused data. */
 while ((size_t)req_size && printer.ap_string->s_str[(size_t)req_size-1] != '/') --req_size;
 while ((size_t)req_size && printer.ap_string->s_str[(size_t)req_size-1] == '/') --req_size;
 printer.ap_string->s_str[(size_t)(req_size++)] = '/';
 ascii_printer_release(&printer,(size_t)(bufsize-(size_t)req_size));
 return ascii_printer_pack(&printer);
err:
 ascii_printer_fini(&printer);
 return NULL;
}
#endif

PRIVATE DREF /*String*/DeeObject *
DCALL get_default_home(void) {
 DREF DeeObject *result,*new_result;
 char *env = getenv("DEEMON_HOME");
 if (env && *env) {
  result = DeeString_New(env);
  if unlikely(!result) goto err;
  new_result = make_absolute(result);
  Dee_Decref(result);
  if unlikely(!new_result) goto err;
  result = new_result;
  goto done;
 }
#ifdef CONFIG_DEEMON_HOME
 result = (DREF DeeObject *)&default_deemon_home;
 Dee_Incref(result);
#elif defined(CONFIG_HOST_WINDOWS)
 {
  /* TODO: Rewrite to directly use wide-string buffers (`DeeString_NewWideBuffer()'). */
  WCHAR stack_buffer[261],*buffer = stack_buffer;
  WCHAR *heap_buffer = NULL,*path_start;
  DWORD buffer_size = COMPILER_LENOF(stack_buffer)-1,size;
again:
  size = GetModuleFileNameW(NULL,buffer,buffer_size);
  if unlikely(!size) {
   err_system_error("GetModuleFileName");
err_heap_buffer:
   Dee_Free(heap_buffer);
   goto err;
  }
  if (size == buffer_size &&
      GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
   WCHAR *new_buffer;
   buffer_size = size*2;
   new_buffer  = (WCHAR *)Dee_Realloc(heap_buffer,(buffer_size+1)*sizeof(WCHAR));
   if unlikely(!new_buffer) goto err_heap_buffer;
   buffer      = new_buffer;
   heap_buffer = new_buffer;
   goto again;
  }
  path_start = buffer+size;
  while (path_start != buffer && path_start[-1] != '\\') --path_start;
  while (path_start != buffer && path_start[-1] == '\\') --path_start;
  if (path_start == buffer) path_start = buffer+size;
  *path_start++ = '\\'; /* Add the trailing slash now, so we don't have to do it later. */
  result = DeeString_NewWide(buffer,
                            (size_t)(path_start-buffer),
                             STRING_ERROR_FREPLAC);
  Dee_Free(heap_buffer);
  if unlikely(!result) goto err;
  new_result = make_absolute(result);
  Dee_Decref(result);
  result = new_result;
  ASSERT(!result || ISSEP(DeeString_END(result)[-1]));
  goto done_slash;
 }
#elif defined(CONFIG_HOST_UNIX)
 result = unix_readlink("/proc/self/exe");
 ASSERT(!result || DeeString_END(result)[-1] == '/');
 goto done_slash;
#else
 result = DeeString_New(".");
 if unlikely(!result) goto err;
#endif
done:
 if (!DeeString_SIZE(result) ||
     !ISSEP(DeeString_END(result)[-1])) {
  /* Must append a trailing slash. */
  new_result = (DREF DeeObject *)DeeString_NewSized(DeeString_STR(result),
                                                    DeeString_SIZE(result)+1);
  Dee_Decref(result);
  if unlikely(!new_result) goto err;
  DeeString_END(new_result)[-1] = SEP;
  result = new_result;
 }
done_slash:
 return result;
err:
 return NULL;
}


#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(deemon_home_lock);
#endif
PRIVATE DREF /*String*/DeeObject *deemon_home = NULL;
PUBLIC DREF /*String*/DeeObject *DCALL DeeExec_GetHome(void) {
 DREF DeeObject *result;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&deemon_home_lock);
#endif
 result = deemon_home;
 if (result) {
  Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&deemon_home_lock);
#endif
  return result;
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&deemon_home_lock);
#endif
 /* Re-create the default home path. */
 result = get_default_home();

 /* Save the generated path in the global variable. */
 if likely(result) {
  DREF DeeObject *other;
#ifndef CONFIG_NO_THREADS
  rwlock_write(&deemon_home_lock);
#endif
  other = deemon_home;
  if unlikely(other) {
   Dee_Incref(other);
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&deemon_home_lock);
#endif
   Dee_Decref(result);
   return other;
  }
  Dee_Incref(result);
  deemon_home = result;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&deemon_home_lock);
#endif
 }
 return result;
}

PUBLIC void DCALL
DeeExec_SetHome(/*String*/DeeObject *new_home) {
 DREF DeeObject *old_home;
 ASSERT_OBJECT_TYPE_EXACT_OPT(new_home,&DeeString_Type);
 Dee_XIncref(new_home);
#ifndef CONFIG_NO_THREADS
 rwlock_write(&deemon_home_lock);
#endif
 old_home = deemon_home;
 deemon_home = new_home;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&deemon_home_lock);
#endif
 Dee_XDecref(old_home);
}


PRIVATE void DCALL do_init_module_path(void) {
 DREF DeeObject *path_part; int error;
 char *path = getenv("DEEMON_PATH");
 if (path) while (*path) {
  /* Split the module path. */
  char *next_path = strchr(path,DELIM);
  if (next_path) {
   path_part = DeeString_NewSized(path,(size_t)(next_path-path));
   ++next_path;
  } else {
   next_path = strend(path);
   path_part = DeeString_NewSized(path,(size_t)(next_path-path));
  }
  if unlikely(!path_part) goto init_error;
  error = DeeList_Append((DeeObject *)&DeeModule_Path,path_part);
  Dee_Decref(path_part);
  if unlikely(error) goto init_error;
  path = next_path;
 }
 /* Add the default path based on deemon-home. */
 path_part = DeeString_Newf("%Klib",DeeExec_GetHome());
 if unlikely(!path_part) goto init_error;
 error = DeeList_Append((DeeObject *)&DeeModule_Path,path_part);
 Dee_Decref(path_part);
 if unlikely(error) goto init_error;
 return;
init_error:
 DeeError_Print("Failed to initialize module path\n",
                ERROR_PRINT_DOHANDLE);
}



#ifdef CONFIG_NO_THREADS
#define INIT_PENDING 0
#define INIT_COMPLET 1
#else
#define INIT_PENDING 0
#define INIT_PROGRES 1
#define INIT_COMPLET 2
#endif

PRIVATE int module_init_state = INIT_PENDING;
PUBLIC void DCALL DeeModule_InitPath(void) {
 /* Lazily calculate hashes of exported objects upon first access. */
 if unlikely(module_init_state != INIT_COMPLET) {
#ifdef CONFIG_NO_THREADS
  do_init_module_path();
  module_init_state = INIT_COMPLET;
#else
  COMPILER_READ_BARRIER();
  if (ATOMIC_CMPXCH(module_init_state,INIT_PENDING,INIT_PROGRES)) {
   do_init_module_path();
   ATOMIC_WRITE(module_init_state,INIT_COMPLET);
  } else {
   while (ATOMIC_READ(module_init_state) != INIT_COMPLET)
       SCHED_YIELD();
  }
#endif
 }
}



DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODPATH_C */
