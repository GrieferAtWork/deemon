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
#ifndef GUARD_DEEMON_COMPILER_LEXER_IMPORT_C
#define GUARD_DEEMON_COMPILER_LEXER_IMPORT_C 1
#define _GNU_SOURCE 1

#include <deemon/api.h>
#include <deemon/none.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/error.h>
#include <deemon/module.h>

#include "../../runtime/strings.h"

#include <string.h>

DECL_BEGIN

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


INTERN struct compiler_options *inner_compiler_options = NULL;

#define TOK_ISDOT(x) ((x) == '.' || (x) == TOK_DOTDOT || (x) == TOK_DOTS)
LOCAL unsigned int DCALL dot_count(tok_t tk) {
 if (tk == TOK_DOTS) return 3;
 if (tk == TOK_DOTDOT) return 2;
 return 1;
}

PRIVATE int DCALL
parse_module_name(struct ascii_printer *__restrict printer) {
 uint32_t old_flags = TPPLexer_Current->l_flags;
 /* Module name parsing is space- and linefeed-sensitive. */
 TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_WANTSPACE|
                               TPPLEXER_FLAG_WANTLF);
 for (;;) {
  dssize_t error; tok_t kind = tok;
  /* Print dots and keywords. */
  if (TOK_ISDOT(kind)) {
   error = ascii_printer_print(printer,"...",dot_count(kind));
   if unlikely(yield() < 0) goto err;
  } else if (TPP_ISKEYWORD(kind)) {
   struct TPPKeyword *kwd = token.t_kwd;
   error = ascii_printer_print(printer,kwd->k_name,kwd->k_size);
   if unlikely(yield() < 0) goto err;
  } else if (kind == TOK_STRING) {
   /* Decode and append a string.
    * The only reason this is allowed, is so that code may import
    * modules who's names contain unicode characters that must be
    * encoded as escape codes. */
   char *buffer;
   char *string_begin = token.t_begin;
   char *string_end = token.t_end;
   size_t unescape_size;
   if (string_begin != string_end && *string_begin == '\"') ++string_begin;
   if (string_begin != string_end && string_end[-1] == '\"') --string_end;
   unescape_size = TPP_SizeofUnescape(string_begin,(size_t)(string_end-string_begin),1);
   buffer = ascii_printer_alloc(printer,unescape_size);
   if unlikely(!buffer) goto err;
   TPP_Unescape(buffer,string_begin,(size_t)(string_end-string_begin),1);
   error = 0;
   /* Skip additional space-tokens after a string. */
   do if unlikely(yield() < 0) goto err;
   while (tok == ' ' || tok == '\n');
  } else break;
  if unlikely(error < 0) goto err;
  /* Dots must follow after a keyword component. Otherwise the name ends here. */
  if (TPP_ISKEYWORD(kind) && !TOK_ISDOT(tok)) break;
  /* Strings may only be followed by other strings, or dots. */
  if (kind == TOK_STRING && tok != TOK_STRING && !TOK_ISDOT(tok))
      break;
 }
 TPPLexer_Current->l_flags = old_flags;
 if ((tok == ' ' || (tok == '\n' && !(old_flags & TPPLEXER_FLAG_WANTLF))) &&
      yield() < 0) goto err;
 return 0;
err:
 TPPLexer_Current->l_flags = old_flags;
 return -1;
}

PRIVATE DREF DeeModuleObject *DCALL
import_module_by_name(DeeStringObject *__restrict module_name) {
 DREF DeeModuleObject *result;
 if (module_name->s_str[0] == '.') {
  char const *filename;
  size_t filename_length;
  if (module_name->s_len == 1) {
   /* Special case: Import your own module. */
   return_reference_(current_rootscope->rs_module);
  }
  filename = TPPFile_Filename(token.t_file,&filename_length);
  if likely(filename) {
   char *path_start; /* Relative module import. */
   path_start = (char *)memrchr(filename,'/',filename_length);
#ifdef CONFIG_HOST_WINDOWS
   {
    char *path_start2;
    path_start2 = (char *)memrchr(filename,'\\',filename_length);
    ASSERT(NULL == 0); /* The following expression assumes this... */
    if (!path_start || path_start2 > path_start)
         path_start = path_start2;
   }
#endif
   if (path_start) ++path_start;
   else path_start = (char *)filename;
   filename_length = (size_t)(path_start-filename);
   /* Interpret the module name relative to the path of the current source file. */
   result = (DREF DeeModuleObject *)DeeModule_OpenRelative((DeeObject *)module_name,
                                                            filename,filename_length,
                                                            inner_compiler_options,
                                                            false);
   goto module_opened;
  }
 }
 result = (DREF DeeModuleObject *)DeeModule_Open((DeeObject *)module_name,
                                                  inner_compiler_options,
                                                  false);
module_opened:
 if unlikely(!ITER_ISOK(result)) {
  if unlikely(!result) goto err;
  if (WARN(W_MODULE_NOT_FOUND,module_name))
      goto err;
  result = &empty_module;
  Dee_Incref(result);
 } else {
  /* Check for recursive dependency. */
  if (!(result->mo_flags&MODULE_FDIDLOAD) &&
        result != current_rootscope->rs_module) {
   PERR(W_RECURSIVE_MODULE_DEPENDENCY,
        result->mo_name,current_rootscope->rs_module->mo_name);
   Dee_Decref(result);
   goto err;
  }
 }
 return result;
err:
 return NULL;
}

INTERN DREF DeeModuleObject *DCALL
parse_module_byname(void) {
 DREF DeeModuleObject *result = NULL;
 struct ascii_printer name = ASCII_PRINTER_INIT;
 if unlikely(parse_module_name(&name)) goto done2;
 if unlikely(!name.ap_string) {
  result = import_module_by_name((DeeStringObject *)Dee_EmptyString);
done2:
  ascii_printer_fini(&name);
 } else {
  DREF DeeStringObject *module_name;
  module_name = (DeeStringObject *)ascii_printer_pack(&name);
  if unlikely(!module_name) goto done;
  result = import_module_by_name(module_name);
  Dee_Decref(module_name);
 }
done:
 return result;
}

INTERN struct module_symbol *DCALL
import_module_symbol(DeeModuleObject *__restrict module,
                     struct TPPKeyword *__restrict name) {
 dhash_t i,perturb;
 dhash_t hash = hash_ptr(name->k_name,name->k_size);
 perturb = i = MODULE_HASHST(module,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(module,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (DeeString_SIZE(item->ss_name) != name->k_size) continue; /* Non-matching length */
  if (memcmp(DeeString_STR(item->ss_name), /* Differing strings. */
             name->k_name,name->k_size*sizeof(char)) != 0) continue;
  return item; /* Found it! */
 }
 return NULL;
}

INTERN DREF DeeAstObject *FCALL
ast_parse_import_single(struct TPPKeyword *__restrict import_name) {
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 DREF DeeStringObject *module_name;
 DREF DeeModuleObject *module;
 struct symbol *extern_symbol;
 struct module_symbol *modsym;
 /* Parse the name of the module from which to import a symbol. */
 if unlikely(parse_module_name(&printer)) goto err_printer;
 module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
 if unlikely(!module_name) goto err;
 module = import_module_by_name(module_name);
 Dee_Decref(module_name);
 if unlikely(!module) goto err;
 /* We've got the module. - Now just create an anonymous symbol. */
 extern_symbol = new_unnamed_symbol();
 if unlikely(!extern_symbol) goto err_module;
 /* Lookup the symbol which we're importing. */
 modsym = import_module_symbol(module,import_name);
 if unlikely(!modsym) {
  if (WARN(W_MODULE_IMPORT_NOT_FOUND,import_name,
           DeeString_STR(module->mo_name)))
      goto err_module;
  if (module == current_rootscope->rs_module) {
   extern_symbol->sym_class = SYM_CLASS_THIS_MODULE;
   Dee_Decref(module);
  } else {
   extern_symbol->sym_class             = SYM_CLASS_MODULE;
   extern_symbol->sym_module.sym_module = module; /* Inherit reference. */
  }
 } else {
  /* Setup this symbol as an external reference. */
  extern_symbol->sym_class             = SYM_CLASS_EXTERN;
  extern_symbol->sym_extern.sym_module = module; /* Inherit reference. */
  extern_symbol->sym_extern.sym_modsym = modsym;
 }
 /* Return the whole thing as a symbol-ast. */
 return ast_sym(extern_symbol);
err_module:
 Dee_Decref(module);
 goto err;
err_printer:
 ascii_printer_fini(&printer);
err:
 return NULL;
}

#if 0 /* TODO: Re-write the import statement parser. */

struct import_item {
    struct TPPKeyword    *ii_snam; /* [1..1] The name by which the item should be imported. */
    DREF DeeStringObject *ii_name; /* [0..1] The name of the object being imported.
                                    *  When NULL, `ii_snam' is used instead. */
};
struct import_list {
    size_t                   il_cnt; /* The number of imported items. */
    size_t                   il_siz; /* The allocated number of items. */
    struct import_item      *il_vec; /* [0..il_cnt|alloc(il_siz)][owned] Vector of imported items. */
#define IMPORT_LIST_FNORMAL  0x0000  /* Normal import list flags. */
#define IMPORT_LIST_FALL     0x0001  /* All objects should be imported from a module (`*' was encountered). */
#define IMPORT_LIST_FMODULES 0x0002  /* The import list refers to modules, rather than symbols.
                                      * Set when `.' tokens are encountered within some import name. */
    unsigned int             il_flg; /* Import list flags (Set of `IMPORT_LIST_F*') */
};


#define PARSE_IMPORT_ITEM_ERROR   (-1) /* An error occurred. */
#define PARSE_IMPORT_ITEM_ADD2LIST  0  /* Add `*result' to the list. */
#define PARSE_IMPORT_ITEM_NEXTITEM  1  /* Do not add `*result' to the list. */
PRIVATE int DCALL
parse_import_item(struct import_list *__restrict list,
                  struct import_item *__restrict result) {
 if (tok == '*') {
  list->il_flg |= IMPORT_LIST_FALL;
  if (yield() < 0) goto err;
  return PARSE_IMPORT_ITEM_NEXTITEM;
 }
 if (TPP_ISKEYWORD(tok)) {
  result->ii_snam = token.t_kwd;
 } else {
  TPPLexer_ParseString();

 }



done:
 return PARSE_IMPORT_ITEM_ADD2LIST;
err:
 return PARSE_IMPORT_ITEM_ERROR;
}



INTERN DREF DeeAstObject *FCALL
ast_parse_import(bool allow_symbol_define) {
 DREF DeeAstObject *result;
 /* Valid ways of writing import statements:
  *  - import("deemon");
  *  - import deemon;
  *  - import deemon, util;
  *  - import my_deemon = deemon;
  *  - import my_deemon = deemon, my_util = util;
  *  - import deemon as my_deemon;
  *  - import deemon as my_deemon, util as my_util;
  *  - import * from deemon;
  *  - import object from deemon;
  *  - import object, list from deemon;
  *  - import my_object = object from deemon;
  *  - import my_object = object, my_list = list from deemon;
  *  - import object as my_object from deemon;
  *  - import object as my_object, list as my_list from deemon;
  *  - from deemon import *;
  *  - from deemon import object;
  *  - from deemon import object, list;
  *  - from deemon import my_object = object;
  *  - from deemon import my_object = object, my_list = list;
  *  - from deemon import object as my_object;
  *  - from deemon import object as my_object, list as my_list;
  * ----
  *  // NOTE: `keyword' may not be followed by `string', and
  *  //       `string' may not be followed by `keyword'
  *  module_name ::= (
  *      (string | '.' | keyword)...
  *  );
  *  symbol_name ::= (
  *       keyword |
  *       string...
  *  );
  *  symbol_import ::= (
  *      (['global' | 'local'] symbol_name) |
  *      (['global' | 'local'] keyword '=' symbol_name) |
  *      (symbol_name 'as' ['global' | 'local'] keyword)
  *  );
  *  rule_import ::= (
  *       ('import' '(' string ')') |
  *
  *       (['global' | 'local'] 'import' ',' ~~ (
  *          (['global' | 'local'] module_name) |
  *          (['global' | 'local'] keyword '=' module_name) |
  *          (module_name 'as' ['global' | 'local'] keyword)
  *       )...) |
  *
  *       (['global' | 'local'] 'import' ',' ~~ (
  *          '*' | symbol_import
  *       )... 'from' module_name) |
  *
  *       ('from' module_name 'import' ',' ~~ (
  *          '*' | symbol_import
  *       )...)
  *  );
  * ----
  * NOTES:
  *   - Importing the same symbol under the same
  *     name twice is allowed and behaves as a no-op.
  *   - A `global' prefix can be used to forward an external
  *     symbol as an export of the current module, which then
  *     refers to the same variable as was originally defined
  *     by the module from which symbols are being imported.
  *     The default is `local', and `global' can only be used
  *     which the root scope.
  */
 ASSERT(tok == KWD_import || tok == KWD_from);

}

#else

PRIVATE DREF DeeAstObject *DCALL
ast_parse_import_single_from(DeeModuleObject *__restrict module,
                             bool allow_symbol_define) {
 struct module_symbol *modsym;
 struct symbol *sym;
 struct TPPKeyword *name;
 struct ast_loc loc; loc_here(&loc);
 if unlikely(!TPP_ISKEYWORD(tok)) {
  if unlikely(WARN(W_EXPECTED_KEYWORD_AFTER_FROM_IMPORT))
     goto err;
  /* As error-handling fallback, return an AST describing the module itself. */
  sym = new_unnamed_symbol();
  if unlikely(!sym) goto err;
import_whole_module:
  if (module == current_rootscope->rs_module) {
   sym->sym_class = SYM_CLASS_THIS_MODULE;
  } else {
   sym->sym_class = SYM_CLASS_MODULE;
   sym->sym_module.sym_module = module;
   Dee_Incref(module);
  }
  return ast_setddi(ast_sym(sym),&loc);
 }
 name = token.t_kwd;
 if unlikely(yield() < 0) goto err;
 if (allow_symbol_define) {
  if unlikely(has_local_symbol(name)) {
   /* TODO: Allow re-import of the same symbol under the same name! */
   if (WARN(W_IMPORT_ALIAS_IS_ALREADY_DEFINED,name))
       goto err;
   goto create_from_anon;
  }
  sym = new_local_symbol(name);
 } else {
create_from_anon:
  sym = new_unnamed_symbol();
 }
 if unlikely(!sym) goto err;
 if (module == current_rootscope->rs_module) {
  Dee_Decref(module);
  if (WARN(W_IMPORT_GLOBAL_FROM_OWN_MODULE))
      goto err_sym;
  goto import_whole_module;
 }
 /* Import the symbol from the module. */
 modsym = import_module_symbol(module,name);
 if unlikely(!modsym) {
  /* Import doesn't exist. */
  if (WARN(W_MODULE_IMPORT_NOT_FOUND,name,
           DeeString_STR(module->mo_name)))
      goto err_sym;
  goto import_whole_module;
 }
 /* This symbol refers to an external variable now. */
 sym->sym_class             = SYM_CLASS_EXTERN;
 sym->sym_extern.sym_modsym = modsym;
 sym->sym_extern.sym_module = module;
 return ast_setddi(ast_sym(sym),&loc);
err_sym:
 sym->sym_class = SYM_CLASS_THIS_MODULE;
err:
 return NULL;
}

PRIVATE int DCALL
import_all(DeeModuleObject *__restrict module) {
 struct module_symbol *iter,*end;
 ASSERT_OBJECT_TYPE(module,&DeeModule_Type);
 if (module == current_rootscope->rs_module) {
  if (WARN(W_IMPORT_GLOBAL_FROM_OWN_MODULE))
      goto err;
  goto done;
 }
 end = (iter = module->mo_bucketv)+(module->mo_bucketm+1);
 for (; iter != end; ++iter) {
  struct symbol *sym;
  struct TPPKeyword *name;
  if (!iter->ss_name) continue; /* Empty slot. */
  if (iter->ss_flags&MODSYM_FHIDDEN) continue; /* Hidden symbol. */
  name = TPPLexer_LookupKeyword(iter->ss_name->s_str,
                                iter->ss_name->s_len,1);
  if unlikely(!name) goto err;
  if unlikely(has_local_symbol(name)) {
   /* TODO: Allow re-import of the same symbol under the same name! */
   if (WARN(W_IMPORT_ALIAS_IS_ALREADY_DEFINED,name))
       goto err;
   continue; /* Skip this one... */
  }
  sym = new_local_symbol(name);
  if unlikely(!sym) goto err;
  /* Define this symbol as an import from this module. */
  sym->sym_class             = SYM_CLASS_EXTERN;
  sym->sym_extern.sym_module = module;
  sym->sym_extern.sym_modsym = iter;
  Dee_Incref(module);
 }
done:
 return 0;
err:
 return -1;
}


INTERN DREF DeeAstObject *FCALL
ast_parse_import(bool allow_symbol_define) {
 DREF DeeAstObject *result;
 DREF DeeModuleObject *module;
 struct ast_loc loc; tok_t start;
 struct ascii_printer printer;
 ASSERT(tok == KWD_import || tok == KWD_from);
 start = tok;
 loc_here(&loc);
 if unlikely(yield() < 0) goto err;
 if (start == KWD_from) {
  DREF DeeStringObject *module_name;
  /* Syntax: `from foo import bar'
   * >> When symbols can be defined:   `import baz = foo.bar.baz'
   * >> When symbols can't be defined: `import("foo.bar.baz")' */
  ascii_printer_init(&printer);
  if unlikely(parse_module_name(&printer))
     goto err_printer;
  module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
  if unlikely(!module_name)
     goto err;
  module = import_module_by_name(module_name);
  Dee_Decref(module_name);
  if unlikely(!module)
     goto err;
  if unlikely(likely(tok == KWD_import) ? (yield() < 0) :
              WARN(W_EXPECTED_IMPORT_AFTER_FROM))
     goto err_module;
  if (allow_symbol_define && tok == '*') {
   /* Special case: import all symbols from a module into the current scope.
    *               Because returning a multiple-ast for every symbol in this
    *               case is not only unpredictable, but would also be waaay
    *               too expensive, the specs require us to return a non-ast
    *               instead. */
   if unlikely(import_all(module))
      goto err_module;
   Dee_Decref(module);
   if unlikely(yield() < 0)
      goto err;
   return ast_setddi(ast_constexpr(Dee_None),&loc);
  }
  /* Now that we've got the module, we can parse imports. */
  result = ast_parse_import_single_from(module,allow_symbol_define);
 } else if (tok == '*' && allow_symbol_define) {
  DREF DeeStringObject *module_name;
  /* Syntax: `import * from foo' */
  if unlikely(yield() < 0) goto err;
  if unlikely(likely(tok == KWD_from) ? (yield() < 0) :
              WARN(W_EXPECTED_FROM_AFTER_IMPORTALL))
     goto err;
  ascii_printer_init(&printer);
  if unlikely(parse_module_name(&printer))
     goto err_printer;
  module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
  if unlikely(!module_name)
     goto err;
  module = import_module_by_name(module_name);
  Dee_Decref(module_name);
  if unlikely(!module)
     goto err;
  if unlikely(import_all(module))
     goto err_module;
  Dee_Decref(module);
  return ast_setddi(ast_constexpr(Dee_None),&loc);
 } else if (tok == TOK_STRING || TOK_ISDOT(tok)) {
  ascii_printer_init(&printer);
  goto do_load_module;
 } else if (TPP_ISKEYWORD(tok)) {
  /* Syntax: `import deemon' / `import Error from deemon'
   * In this context, the import is added statically to the calling module. */
  struct TPPKeyword *import_name;
  loc_here(&loc);
  import_name = token.t_kwd;
  if unlikely(yield() < 0) goto err;
  if (TOK_ISDOT(tok)) {
   /* Syntax: `import foo.bar.baz'
    * >> When symbols can be defined:   `import baz = foo.bar.baz'
    * >> When symbols can't be defined: `import("foo.bar.baz")' */
   DREF DeeStringObject *module_name;
   struct symbol *result_symbol;
   ascii_printer_init(&printer);
   if unlikely(ascii_printer_print(&printer,
                                    import_name->k_name,
                                    import_name->k_size) < 0)
      goto err_printer;
do_load_module:
   if unlikely(parse_module_name(&printer))
      goto err_printer;
   module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
   if unlikely(!module_name)
      goto err;
   /* Load an inner module. */
   module = import_module_by_name(module_name);
   if unlikely(!module) { Dee_Decref(module_name); goto err; }
   /* `import foo.bar' is the same as `import bar = foo.bar' when we're allowed to define symbols.
    *  Otherwise, we encapsulate the module in an anonymous symbol. */
   if (allow_symbol_define) {
    char *import_name_begin;
    import_name_begin = (char *)memrchr(DeeString_STR(module_name),'.',
                                        DeeString_SIZE(module_name));
    if (import_name_begin) ++import_name_begin;
    else import_name_begin = DeeString_STR(module_name);
    import_name = TPPLexer_LookupKeyword(import_name_begin,
                                        (size_t)((DeeString_STR(module_name) +
                                                  DeeString_SIZE(module_name))-
                                                  import_name_begin),1);
    Dee_Decref(module_name);
    if unlikely(!import_name) goto err_module;
    if unlikely(has_local_symbol(import_name)) {
     /* TODO: Allow re-import of the same symbol under the same name! */
     if (WARN(W_IMPORT_ALIAS_IS_ALREADY_DEFINED,import_name))
         goto err_module;
     goto import_anon;
    }
    result_symbol = new_local_symbol(import_name);
   } else {
import_anon:
    Dee_Decref(module_name);
    result_symbol = new_unnamed_symbol();
   }
   if unlikely(!result_symbol) goto err_module;
   if (module == current_rootscope->rs_module) {
    /* Special case: this is an import to our own module. */
    result_symbol->sym_class = SYM_CLASS_THIS_MODULE;
    Dee_Decref(module);
   } else {
    ASSERT(module);
    result_symbol->sym_class             = SYM_CLASS_MODULE;
    result_symbol->sym_module.sym_module = module; /* Inherit reference. */
   }
   /* Now create the result AST for this symbol. */
   result = ast_setddi(ast_sym(result_symbol),&loc);
  } else if (tok == ',') {
   /* Syntax: `import foo,fiz from bar'
    * >> When symbols can be defined:   `import foo = foo from bar; import fiz = fiz from bar'
    * >> When symbols can't be defined: `(import("bar").foo,import("bar").fiz)'
    * Syntax: `import foo,fiz'
    * >> When symbols can be defined:   `import foo = foo; import fiz = fiz'
    * >> When symbols can't be defined: `(import("foo"),import("fiz"))' */
   DREF DeeAstObject **astv; size_t astc,asta;
   bool is_module_import_list;
   is_module_import_list = false;
   goto do_import_multiple;
do_import_multiple_modules:
   is_module_import_list = true;
do_import_multiple:
   astc = 0,asta = 2;
   astv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
   if unlikely(!astv) goto err;
   for (;;) {
    DREF DeeAstObject *new_ast;
    struct symbol *ast_symbol;
    if (allow_symbol_define) {
     char *symbol_name_start;
     struct TPPKeyword *symbol_name = import_name;
     symbol_name_start = (char *)memrchr(import_name->k_name,'.',
                                         import_name->k_size);
     if (symbol_name_start) {
      ++symbol_name_start;
      symbol_name = TPPLexer_LookupKeyword(symbol_name_start,
                                          (size_t)((import_name->k_name+
                                                    import_name->k_size)-
                                                    symbol_name_start),
                                           1);
      if unlikely(!symbol_name) goto err_astv;
     }
     if unlikely(has_local_symbol(symbol_name)) {
      /* TODO: Allow re-import of the same symbol under the same name! */
      if (WARN(W_IMPORT_ALIAS_IS_ALREADY_DEFINED,symbol_name))
          goto err_astv;
      goto import_anon_module;
     }
     ast_symbol = new_local_symbol(symbol_name);
     if unlikely(!ast_symbol) goto err_astv;
    } else {
import_anon_module:
     ast_symbol = new_unnamed_symbol();
     if unlikely(!ast_symbol) goto err_astv;
    }
    /* Temporary storage of names. - deleted later. */
    ast_symbol->sym_module.sym_module = (DREF DeeModuleObject *)import_name;
    /* Some set symbol class to keep the symbol in a consistent state.
     * We'll override this later. */
    ast_symbol->sym_class = SYM_CLASS_THIS_MODULE;
    /* Make sure there is enough available buffer. */
    if (astc == asta) {
     size_t new_alloc = asta*2;
     DREF DeeAstObject **new_vector;
     ASSERT(new_alloc);
do_realloc_astv:
     new_vector = (DREF DeeAstObject **)Dee_TryRealloc(astv,new_alloc*
                                                       sizeof(DREF DeeAstObject *));
     if unlikely(!new_vector) {
      if (new_alloc != astc+1) { new_alloc = astc+1; goto do_realloc_astv; }
      if (Dee_CollectMemory(new_alloc*sizeof(DREF DeeAstObject *))) goto do_realloc_astv;
      goto err_astv;
     }
     astv = new_vector;
     asta = new_alloc;
    }
    /* Create an add a new AST for this import symbol. */
    new_ast = ast_setddi(ast_sym(ast_symbol),&loc);
    if unlikely(!new_ast) goto err_astv;
    astv[astc++] = new_ast; /* Inherit reference. */
    if (tok != ',') break;
    if unlikely(yield() < 0) {
err_astv:
     while (astc--) Dee_Decref(astv[astc]);
     Dee_Free(astv);
     goto err;
    }
    if (TOK_ISDOT(tok)) {
     ascii_printer_init(&printer);
     goto module_name_in_asts;
    }
    if unlikely(!TPP_ISKEYWORD(tok)) {
     if (WARN(W_EXPECTED_KEYWORD_AFTER_COMMA_IN_IMPORT))
         goto err_astv;
     break;
    }
    loc_here(&loc);
    import_name = token.t_kwd;
    if unlikely(yield() < 0)
       goto err_astv;
    if (TOK_ISDOT(tok)) {
     /* Special case: `import foo,foo.bar' */
     ascii_printer_init(&printer);
     if unlikely(ascii_printer_print(&printer,import_name->k_name,import_name->k_size) < 0) {
err_printer_astv:
      ascii_printer_fini(&printer);
      goto err_astv;
     }
module_name_in_asts:
     if (parse_module_name(&printer))
         goto err_printer_astv;
     /* Re-work the import name into a keyword. */
     import_name = TPPLexer_LookupKeyword(ASCII_PRINTER_STR(&printer),
                                          ASCII_PRINTER_LEN(&printer),1);
     ascii_printer_fini(&printer);
     if unlikely(!import_name) goto err_astv;
     is_module_import_list = true;
    }
   }
   if (astc != asta) {
    DREF DeeAstObject **new_vector;
    new_vector = (DREF DeeAstObject **)Dee_TryRealloc(astv,astc*sizeof(DREF DeeAstObject *));
    if likely(new_vector) astv = new_vector;
   }
   /* Ast this point, we've parse the import list, which either consists of
    * names that are going to be imported from another module when the following
    * keyword is `from', or are module names themself when it isn't. */
   if (!is_module_import_list && tok == KWD_from) {
    DREF DeeStringObject *module_name;
    DREF DeeAstObject **iter,**end;
    /* Syntax: `import foo,fiz from bar'
     * >> When symbols can be defined:   `import foo = foo from bar; import fiz = fiz from bar'
     * >> When symbols can't be defined: `(import("bar").foo,import("bar").fiz)' */
    if unlikely(yield() < 0)
       goto err_astv;
    if unlikely(!TPP_ISKEYWORD(tok) && !TOK_ISDOT(tok)) {
     if (WARN(W_EXPECTED_KEYWORD_AFTER_COMMA_FROM_IN_IMPORT))
         goto err_astv;
     goto import_each_module;
    }
    ascii_printer_init(&printer);
    /* Parse the module's name. */
    if unlikely(parse_module_name(&printer))
       goto err_printer_astv;
    module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
    if unlikely(!module_name) goto err_astv;
    /* Import the module specified by this name. */
    module = import_module_by_name(module_name);
    Dee_Decref(module_name);
    if unlikely(!module) goto err_astv;
    if (module == current_rootscope->rs_module) {
     Dee_Decref(module);
     if (WARN(W_IMPORT_GLOBAL_FROM_OWN_MODULE))
         goto err_astv;
     goto import_each_module;
    }
    /* Now go all of our symbols and link them against the given module. */
    end = (iter = astv)+astc;
    for (; iter != end; ++iter) {
     struct symbol *sym;
     struct module_symbol *modsym;
     struct TPPKeyword *import_name;
     ASSERT(*iter);
     ASSERT((*iter)->ast_type == AST_SYM);
     sym = (*iter)->ast_sym;
     ASSERT(sym);
     /* The other end of the hack used above. */
     import_name = (struct TPPKeyword *)sym->sym_module.sym_module;
     modsym = import_module_symbol(module,import_name);
     if unlikely(!modsym) {
      /* Import doesn't exist. */
      if (WARN(W_MODULE_IMPORT_NOT_FOUND,import_name,
               DeeString_STR(module->mo_name)))
          goto err_astv;
      /* This symbol refers to a module now. */
      ASSERT(module);
      sym->sym_class             = SYM_CLASS_MODULE;
      sym->sym_module.sym_module = module;
     } else {
      /* This symbol refers to an external variable now. */
      sym->sym_class             = SYM_CLASS_EXTERN;
      sym->sym_extern.sym_modsym = modsym;
      sym->sym_extern.sym_module = module;
     }
     Dee_Incref(module);
    }
    Dee_Decref(module);
   } else {
    /* Syntax: `import foo,fiz'
     * >> When symbols can be defined:   `import foo = foo; import fiz = fiz'
     * >> When symbols can't be defined: `(import("foo"),import("fiz"))' */
    DREF DeeAstObject **iter,**end;
import_each_module:
    end = (iter = astv)+astc;
    for (; iter != end; ++iter) {
     struct symbol *sym;
     DREF DeeObject *module_name;
     struct TPPKeyword *import_name;
     ASSERT(*iter);
     ASSERT((*iter)->ast_type == AST_SYM);
     sym = (*iter)->ast_sym;
     ASSERT(sym);
     /* The other end of the hack used above. */
     import_name = (struct TPPKeyword *)sym->sym_module.sym_module;
     module_name = DeeString_NewSized(import_name->k_name,
                                      import_name->k_size);
     if unlikely(!module_name) goto err_astv;
     sym->sym_module.sym_module = import_module_by_name((DREF DeeStringObject *)module_name);
     Dee_Decref(module_name);
     if unlikely(!sym->sym_module.sym_module) goto err_astv;
     /* This symbol refers to a module now. */
     sym->sym_class = SYM_CLASS_MODULE;
    }
   }
   /* Create the multi-ast that will be returned. */
   result = ast_multiple(AST_FMULTIPLE_KEEPLAST,astc,astv);
   if unlikely(!result) goto err_astv;
  } else if (tok == '=' && allow_symbol_define) {
   DREF DeeStringObject *module_name;
   struct TPPKeyword *symbol_name;
   struct symbol *import_sym;
   /* Syntax: `import foo = baz'
    * Syntax: `import foo = bar from baz'
    * HINT: `import_name' == `foo' */
   symbol_name = import_name;
   if unlikely(has_local_symbol(symbol_name)) {
    /* TODO: Allow re-import of the same symbol under the same name! */
    if (WARN(W_IMPORT_ALIAS_IS_ALREADY_DEFINED,import_name))
        goto err;
    goto do_import_single_module_load;
   }
   import_sym = new_local_symbol(symbol_name);
   if unlikely(!import_sym) goto err;
   import_sym->sym_class = SYM_CLASS_THIS_MODULE;
   if unlikely(yield() < 0) goto err;
   if (tok == TOK_STRING || TOK_ISDOT(tok)) {
    /* Specially encoded module. */
    ascii_printer_init(&printer);
    goto do_import_alias_module;
   }
   if (!TPP_ISKEYWORD(tok)) {
    if (WARN(W_EXPECTED_KEYWORD_AFTER_IMPORT_ALIAS))
        goto err;
    goto do_import_single_module_load;
   }
   import_name = token.t_kwd;
   if unlikely(yield() < 0) goto err;
   if (tok != KWD_from) {
    /* Load a module. */
    ascii_printer_init(&printer);
    /* Process the module's name. */
    if unlikely(ascii_printer_print(&printer,
                                      import_name->k_name,
                                      import_name->k_size) < 0)
       goto err_printer;
do_import_alias_module:
    if unlikely(parse_module_name(&printer))
       goto err_printer;
    module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
    if unlikely(!module_name) goto err;
    module = import_module_by_name(module_name);
    Dee_Decref(module_name);
    if unlikely(!module) goto err;
do_import_alias_module_noparse:
    if (module == current_rootscope->rs_module) {
     import_sym->sym_class = SYM_CLASS_THIS_MODULE;
     Dee_Decref(module);
    } else {
     import_sym->sym_class             = SYM_CLASS_MODULE;
     import_sym->sym_module.sym_module = module; /* Inherit reference. */
    }
   } else {
    struct module_symbol *modsym;
    if unlikely(yield() < 0) goto err; /* `from' */
    /* Load a module. */
    ascii_printer_init(&printer);
    /* Process the module's name. */
    if unlikely(parse_module_name(&printer)) goto err_printer;
    if unlikely(ASCII_PRINTER_LEN(&printer) == 0) {
     /* Empty printer -> No module name was given. */
     ascii_printer_fini(&printer);
     if (WARN(W_EXPECTED_KEYWORD_AFTER_FROM_IN_IMPORT)) goto err;
     goto do_import_single_module_load;
    }
    module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
    if unlikely(!module_name) goto err;
    module = import_module_by_name(module_name);
    Dee_Decref(module_name);
    if unlikely(!module) goto err;
    if (module == current_rootscope->rs_module) {
     if (WARN(W_IMPORT_GLOBAL_FROM_OWN_MODULE))
         goto err_module;
     goto do_import_alias_module_noparse;
    }
    modsym = import_module_symbol(module,import_name);
    if unlikely(!modsym) {
     if (WARN(W_MODULE_IMPORT_NOT_FOUND,import_name,
              DeeString_STR(module->mo_name)))
         goto err_module;
     goto do_import_alias_module_noparse;
    }
    import_sym->sym_class             = SYM_CLASS_EXTERN;
    import_sym->sym_extern.sym_module = module; /* Inherit reference. */
    import_sym->sym_extern.sym_modsym = modsym;
   }
   result = ast_setddi(ast_sym(import_sym),&loc);
  } else if (tok == KWD_from) {
   DREF DeeStringObject *module_name;
   struct symbol *import_symbol;
   struct module_symbol *modsym;
   /* Syntax: `import foo from bar'
    * >> When symbols can be defined:   `import foo = foo from bar'
    * >> When symbols can't be defined: `import("bar").foo' */
   if unlikely(yield() < 0) goto err;
   ascii_printer_init(&printer);
   /* Process the module's name. */
   if unlikely(parse_module_name(&printer)) goto err_printer;
   if unlikely(ASCII_PRINTER_LEN(&printer) == 0) {
    /* Empty printer -> No module name was given. */
    ascii_printer_fini(&printer);
    if (WARN(W_EXPECTED_KEYWORD_AFTER_FROM_IN_IMPORT)) goto err;
    goto do_import_single_module_load;
   }
   module_name = (DREF DeeStringObject *)ascii_printer_pack(&printer);
   if unlikely(!module_name) goto err;
   module = import_module_by_name(module_name);
   Dee_Decref(module_name);
   if unlikely(!module) goto err;
   if (module == current_rootscope->rs_module) {
    if (WARN(W_IMPORT_GLOBAL_FROM_OWN_MODULE))
        goto err_module;
    goto do_import_single_module;
   }
   modsym = import_module_symbol(module,import_name);
   if unlikely(!modsym) {
    if (WARN(W_MODULE_IMPORT_NOT_FOUND,import_name,
             DeeString_STR(module->mo_name)))
        goto err_module;
    goto do_import_single_module;
   }
   /* Got the module from which we're importing. */
   if (allow_symbol_define) {
    if unlikely(has_local_symbol(import_name)) {
     /* TODO: Allow re-import of the same symbol under the same name! */
     if (WARN(W_IMPORT_ALIAS_IS_ALREADY_DEFINED,import_name))
         goto err_module;
     goto import_anon_symbol;
    }
    import_symbol = new_local_symbol(import_name);
   } else {
import_anon_symbol:
    import_symbol = new_unnamed_symbol();
   }
   if unlikely(!import_symbol) goto err_module;
   /* Link this symbol against the specified module import. */
   import_symbol->sym_class             = SYM_CLASS_EXTERN;
   import_symbol->sym_extern.sym_modsym = modsym;
   import_symbol->sym_extern.sym_module = module; /* Inherit reference. */
   /* Create a new branch for this symbol. */
   result = ast_setddi(ast_sym(import_symbol),&loc);
  } else {
   struct symbol *modsym;
   DREF DeeStringObject *module_name;
   if (TOK_ISDOT(tok)) {
    /* Syntax: `import foo.bar'
     * Syntax: `import foo.bar,bar.baz' */
    ascii_printer_init(&printer);
    if unlikely(ascii_printer_print(&printer,
                                     import_name->k_name,
                                     import_name->k_size) < 0 ||
                parse_module_name(&printer))
       goto err_printer;
    import_name = TPPLexer_LookupKeyword(ASCII_PRINTER_STR(&printer),
                                         ASCII_PRINTER_LEN(&printer),1);
    ascii_printer_fini(&printer);
    if unlikely(!import_name) goto err;
    if (tok == ',') goto do_import_multiple_modules;
   }
   /* Syntax: `import bar'
    * >> When symbols can be defined:   `import bar = bar'
    * >> When symbols can't be defined: `import("bar")' */
do_import_single_module_load:
   module_name = (DREF DeeStringObject *)DeeString_NewSized(import_name->k_name,
                                                            import_name->k_size);
   if unlikely(!module_name) goto err;
   module = import_module_by_name(module_name);
   Dee_Decref(module_name);
   if unlikely(!module) goto err;
do_import_single_module:
   if (allow_symbol_define) {
    struct TPPKeyword *symbol_name = import_name;
    char *symbol_name_start;
    symbol_name_start = (char *)memrchr(import_name->k_name,'.',
                                        import_name->k_size);
    if (symbol_name_start) {
     ++symbol_name_start;
     symbol_name = TPPLexer_LookupKeyword(symbol_name_start,
                                         (size_t)((import_name->k_name+
                                                   import_name->k_size)-
                                                   symbol_name_start),
                                          1);
     if unlikely(!symbol_name) goto err_module;
    }
    if unlikely(has_local_symbol(symbol_name)) {
     /* TODO: Allow re-import of the same symbol under the same name! */
     if (WARN(W_IMPORT_ALIAS_IS_ALREADY_DEFINED,symbol_name))
         goto err_module;
     goto import_single_anon_module;
    }
    modsym = new_local_symbol(symbol_name);
   } else {
import_single_anon_module:
    modsym = new_unnamed_symbol();
   }
   if unlikely(!modsym) goto err_module;
   if (module == current_rootscope->rs_module) {
    modsym->sym_class = SYM_CLASS_THIS_MODULE;
    Dee_Decref(module);
   } else {
    ASSERT(module);
    modsym->sym_class             = SYM_CLASS_MODULE;
    modsym->sym_module.sym_module = module; /* Inherit reference. */
   }
   /* Create the AST used to refer to this module. */
   result = ast_setddi(ast_sym(modsym),&loc);
  }
 } else {
  /* As a stand-alone, `import' links against the
   * `import' symbol found in the `deemon' module. */
  struct symbol *import_symbol;
  import_symbol = new_unnamed_symbol();
  if unlikely(!import_symbol) goto err;
  /* Setup an external symbol pointing at `import from deemon' */
  import_symbol->sym_class             = SYM_CLASS_EXTERN;
  import_symbol->sym_extern.sym_module = get_deemon_module();
  Dee_Incref(import_symbol->sym_extern.sym_module);
  import_symbol->sym_extern.sym_modsym = DeeModule_GetSymbolString(import_symbol->sym_extern.sym_module,
                                                                   DeeString_STR(&str_import),
                                                                   DeeString_Hash(&str_import));
  ASSERT(import_symbol->sym_extern.sym_modsym);
  result = ast_setddi(ast_sym(import_symbol),&loc);
 }
 return result;
err_printer: ascii_printer_fini(&printer); goto err;
err_module: Dee_Decref(module); /*goto err;
err_r: Dee_Decref(result);*/
err:   return NULL;
}

#endif

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_IMPORT_C */
