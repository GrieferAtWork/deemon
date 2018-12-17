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
#include <deemon/alloc.h>
#include <deemon/none.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>

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

PRIVATE DREF DeeModuleObject *DCALL
import_module_by_name(DeeStringObject *__restrict module_name,
                      struct ast_loc *loc) {
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
 result = (DREF DeeModuleObject *)DeeModule_OpenGlobal((DeeObject *)module_name,
                                                        inner_compiler_options,
                                                        false);
module_opened:
 if unlikely(!ITER_ISOK(result)) {
  if unlikely(!result) goto err;
  if (WARNAT(loc,W_MODULE_NOT_FOUND,module_name))
      goto err;
  result = &empty_module;
  Dee_Incref(result);
 } else {
  /* Check for recursive dependency. */
  if (!(result->mo_flags&MODULE_FDIDLOAD) &&
        result != current_rootscope->rs_module) {
   PERRAT(loc,W_RECURSIVE_MODULE_DEPENDENCY,
          result->mo_name,current_rootscope->rs_module->mo_name);
   Dee_Decref(result);
   goto err;
  }
 }
 return result;
err:
 return NULL;
}

INTERN struct module_symbol *DCALL
import_module_symbol(DeeModuleObject *__restrict module,
                     struct TPPKeyword *__restrict name) {
 dhash_t i,perturb;
 dhash_t hash = Dee_HashUtf8(name->k_name,name->k_size);
 perturb = i = MODULE_HASHST(module,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(module,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (!MODULE_SYMBOL_EQUALS(item,name->k_name,name->k_size)) continue; /* Differing strings. */
  return item; /* Found it! */
 }
 return NULL;
}

/* @return:  1: OK (the parsed object most certainly was a module)
 * @return:  0: OK
 * @return: -1: Error */
PRIVATE int DCALL
ast_parse_module_name(struct unicode_printer *__restrict printer,
                      bool for_alias) {
 int result = 0;
 for (;;) {
  if (TOK_ISDOT(tok)) {
   if (unicode_printer_printascii(printer,"...",dot_count(tok)) < 0)
       goto err;
   result = 1;
   if unlikely(yield() < 0)
      goto err;
   if (UNICODE_PRINTER_LENGTH(printer) == 1 &&
      (!TPP_ISKEYWORD(tok) && tok != TOK_STRING &&
       (tok != TOK_CHAR || HAS(EXT_CHARACTER_LITERALS)) &&
       !TOK_ISDOT(tok)))
       break; /* Special case: `.' is a valid name for the current module. */
  } else if (TPP_ISKEYWORD(tok)) {
   /* Warn about reserved identifiers.
    * -> Reserved identifiers should be written as strings. */
   if (is_reserved_symbol_name(token.t_kwd) &&
       WARN(for_alias ? W_RESERVED_IDENTIFIER_IN_MODULE_NAME
                      : W_RESERVED_IDENTIFIER_IN_MODULE_NAME_NOALIAS,
            token.t_kwd))
       goto err;
   if (unicode_printer_print(printer,
                             token.t_kwd->k_name,
                             token.t_kwd->k_size) < 0)
       goto err;
   if unlikely(yield() < 0)
      goto err;
   if (!TOK_ISDOT(tok)) break;
  } else if (tok == TOK_STRING ||
            (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
   if (ast_decode_unicode_string(printer) < 0)
       goto err;
   if unlikely(yield() < 0)
      goto err;
   if (!TOK_ISDOT(tok) && tok != TOK_STRING &&
       (tok != TOK_CHAR || HAS(EXT_CHARACTER_LITERALS)))
        break;
  } else {
   if (WARN(W_EXPECTED_DOTS_KEYWORD_OR_STRING_IN_IMPORT_LIST))
       goto err;
   break;
  }
 }
 return result;
err:
 return -1;
}

PRIVATE int DCALL
ast_parse_symbol_name(struct unicode_printer *__restrict printer,
                      bool for_alias) {
 int result = 0;
 if (TPP_ISKEYWORD(tok)) {
  /* Warn about reserved identifiers.
   * -> Reserved identifiers should be written as string imports. */
  if (is_reserved_symbol_name(token.t_kwd) &&
      WARN(for_alias ? W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME
                     : W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME_NOALIAS,
           token.t_kwd))
      goto err;
  if (unicode_printer_print(printer,
                            token.t_kwd->k_name,
                            token.t_kwd->k_size) < 0)
      goto err;
  if unlikely(yield() < 0)
     goto err;
 } else if (tok == TOK_STRING ||
           (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
  do {
   if (ast_decode_unicode_string(printer) < 0)
       goto err;
   if unlikely(yield() < 0)
      goto err;
  } while (tok == TOK_STRING ||
          (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS)));
 } else {
  if (WARN(W_EXPECTED_KEYWORD_OR_STRING_IN_IMPORT_LIST))
      goto err;
 }
 return result;
err:
 return -1;
}


INTERN DREF DeeModuleObject *DCALL parse_module_byname(bool for_alias) {
 DREF DeeModuleObject *result;
 DREF DeeStringObject *module_name;
 struct unicode_printer name = UNICODE_PRINTER_INIT;
 struct ast_loc loc; loc_here(&loc);
 if unlikely(ast_parse_module_name(&name,for_alias) < 0)
    goto err_printer;
 module_name = (DREF DeeStringObject *)unicode_printer_pack(&name);
 if unlikely(!module_name) goto err;
 result = import_module_by_name(module_name,&loc);
 Dee_Decref(module_name);
 return result;
err_printer:
 unicode_printer_fini(&name);
err:
 return NULL;
}

INTERN struct symbol *FCALL
ast_parse_import_single_sym(struct TPPKeyword *__restrict import_name) {
 DREF DeeModuleObject *module;
 struct symbol *extern_symbol;
 struct module_symbol *modsym;
 /* Parse the name of the module from which to import a symbol. */
 module = parse_module_byname(true);
 if unlikely(!module) goto err;
 /* We've got the module. - Now just create an anonymous symbol. */
 extern_symbol = new_unnamed_symbol();
 if unlikely(!extern_symbol) goto err_module;
 /* Lookup the symbol which we're importing. */
 modsym = import_module_symbol(module,import_name);
 if unlikely(!modsym) {
  if (WARN(W_MODULE_IMPORT_NOT_FOUND,
           import_name->k_name,
           DeeString_STR(module->mo_name)))
      goto err_module;
  if (module == current_rootscope->rs_module) {
   extern_symbol->s_type = SYMBOL_TYPE_MYMOD;
   Dee_Decref(module);
  } else {
   extern_symbol->s_type = SYMBOL_TYPE_MODULE;
   extern_symbol->s_module = module; /* Inherit reference. */
  }
 } else {
  /* Setup this symbol as an external reference. */
  extern_symbol->s_type            = SYMBOL_TYPE_EXTERN;
  extern_symbol->s_extern.e_module = module; /* Inherit reference. */
  extern_symbol->s_extern.e_symbol = modsym;
 }
 return extern_symbol;
err_module:
 Dee_Decref(module);
 /*goto err;*/
err:
 return NULL;
}
INTERN DREF struct ast *FCALL
ast_parse_import_single(struct TPPKeyword *__restrict import_name) {
 struct symbol *extern_symbol;
 extern_symbol = ast_parse_import_single_sym(import_name);
 if unlikely(!extern_symbol)
    goto err;
 /* Return the whole thing as a symbol-ast. */
 return ast_sym(extern_symbol);
err:
 return NULL;
}



struct import_item {
    struct ast_loc        ii_import_loc;  /* Parser location of `ii_import_name' */
    struct TPPKeyword    *ii_symbol_name; /* [1..1] The name by which the item should be imported. */
    DREF DeeStringObject *ii_import_name; /* [0..1] The name of the object being imported.
                                           * When NULL, `ii_snam' is used instead. */
};

/* Return `bar' for a module name `.foo.bar', etc. */
PRIVATE struct TPPKeyword *DCALL
get_module_symbol_name(DeeStringObject *__restrict module_name, bool is_module) {
 char *utf8_repr,*symbol_start; size_t symbol_length;
 utf8_repr = DeeString_AsUtf8((DeeObject *)module_name);
 if unlikely(!utf8_repr) goto err;
 symbol_start = (char *)memrchr(utf8_repr,'.',WSTR_LENGTH(utf8_repr));
 if (!symbol_start)
      symbol_start = utf8_repr;
 else ++symbol_start;
 symbol_length = (size_t)((utf8_repr + WSTR_LENGTH(utf8_repr)) - symbol_start);
 /* Make sure that the symbol name is valid. */
 {
  char *iter,*end; uint32_t ch;
  end = (iter = symbol_start) + symbol_length;
  if unlikely(!symbol_length) goto bad_symbol_name;
  for (; iter < end; ++iter) {
   uniflag_t flags;
   ch = utf8_readchar((char const **)&iter,end);
   flags = DeeUni_Flags(ch);
   if (iter == symbol_start ? !(flags & UNICODE_FSYMSTRT)
                            : !(flags & UNICODE_FSYMCONT)) {
bad_symbol_name:
    if (is_module) {
     if (WARN(W_INVALID_NAME_FOR_MODULE_SYMBOL,module_name,symbol_length,symbol_start))
         goto err;
    } else {
     if (WARN(W_INVALID_NAME_FOR_IMPORT_SYMBOL,module_name))
         goto err;
    }
    break;
   }
  }
 }
 /* Lookup/create a keyword for the module's symbol name. */
 return TPPLexer_LookupKeyword(symbol_start,symbol_length,1);
err:
 return NULL;
}


/* @return:  2: Failed
 * @return:  1: OK (when `allow_module_name' is true, a module import was parsed)
 * @return:  0: OK
 * @return: -1: Error */
PRIVATE int DCALL
parse_import_symbol(struct import_item *__restrict result,
                    bool allow_module_name) {
 struct unicode_printer printer;
 int return_value = 0;
 loc_here(&result->ii_import_loc);
 if (TPP_ISKEYWORD(tok)) {
  /* - `foo'
   * - `foo = bar'
   * - `foo = .foo.bar'
   * - `foo = "bar"'
   * - `foo as bar'
   * - `foo.bar'
   * - `foo.bar as foobar' */
  result->ii_symbol_name = token.t_kwd;
  if unlikely(yield() < 0) goto err;
  if (tok == '=') {
   /* - `foo = bar'
    * - `foo = .foo.bar'
    * - `foo = "bar"' */
   if (is_reserved_symbol_name(result->ii_symbol_name) &&
       WARNAT(&result->ii_import_loc,W_RESERVED_IDENTIFIER_IN_ALIAS_NAME,result->ii_symbol_name))
       goto err;
   if unlikely(yield() < 0) goto err;
   unicode_printer_init(&printer);
   loc_here(&result->ii_import_loc);
   return_value = allow_module_name
                ? ast_parse_module_name(&printer,true)
                : ast_parse_symbol_name(&printer,true)
                ;
   if unlikely(return_value < 0) goto err_printer;
   result->ii_import_name = (DREF DeeStringObject *)unicode_printer_pack(&printer);
   if unlikely(!result->ii_import_name) goto err;
  } else if (tok == KWD_as) {
   if (is_reserved_symbol_name(result->ii_symbol_name) &&
       WARNAT(&result->ii_import_loc,W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME,result->ii_symbol_name))
       goto err;
   if unlikely(yield() < 0) goto err;
   /* - `foo as bar' */
   if (TPP_ISKEYWORD(tok)) {
    result->ii_import_name = (DREF DeeStringObject *)DeeString_NewUtf8(result->ii_symbol_name->k_name,
                                                                       result->ii_symbol_name->k_size,
                                                                       STRING_ERROR_FSTRICT);
    if unlikely(!result->ii_import_name) goto err;
    result->ii_symbol_name = token.t_kwd;
    if (is_reserved_symbol_name(token.t_kwd) &&
        WARN(W_RESERVED_IDENTIFIER_IN_ALIAS_NAME,token.t_kwd))
        goto err;
    if unlikely(yield() < 0) goto err;
   } else {
    if (WARN(W_EXPECTED_KEYWORD_AFTER_AS))
        goto err;
    result->ii_import_name = NULL;
   }
  } else if (TOK_ISDOT(tok) && allow_module_name) {
   /* - `foo.bar'
    * - `foo.bar as foobar' */
   if (is_reserved_symbol_name(result->ii_symbol_name) &&
       WARNAT(&result->ii_import_loc,W_RESERVED_IDENTIFIER_IN_MODULE_NAME,result->ii_symbol_name))
       goto err;
   unicode_printer_init(&printer);
   if unlikely(unicode_printer_print(&printer,
                                      result->ii_symbol_name->k_name,
                                      result->ii_symbol_name->k_size) < 0)
      goto err_printer;
   goto complete_module_name;
  } else {
   if (is_reserved_symbol_name(result->ii_symbol_name) &&
       WARNAT(&result->ii_import_loc,
               allow_module_name ? W_RESERVED_IDENTIFIER_IN_SYMBOL_OR_MODULE_NAME
                                 : W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME,
               result->ii_symbol_name))
       goto err;
   result->ii_import_name = NULL;
  }
 } else if (TOK_ISDOT(tok) && allow_module_name) {
  /* - `.foo.bar'
   * - `.foo.bar as foobar' */
  unicode_printer_init(&printer);
complete_module_name:
  return_value = 1;
  if unlikely(unicode_printer_printascii(&printer,"...",dot_count(tok)) < 0)
     goto err_printer;
  if unlikely(yield() < 0)
     goto err_printer;
  /* Make sure to properly parse `import . as me' */
  if ((TPP_ISKEYWORD(tok) && tok != KWD_as) ||
       TOK_ISDOT(tok) || tok == TOK_STRING ||
      (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS)) ||
      (UNICODE_PRINTER_LENGTH(&printer) != 1)) {
   if unlikely(ast_parse_module_name(&printer,true) < 0)
      goto err_printer;
  }
  result->ii_import_name = (DREF DeeStringObject *)unicode_printer_pack(&printer);
  if unlikely(!result->ii_import_name) goto err;
  if (tok == KWD_as) {
   /* - `.foo.bar as foobar' */
   if unlikely(yield() < 0) goto err_name;
   if unlikely(!TPP_ISKEYWORD(tok)) {
    if (WARN(W_EXPECTED_KEYWORD_AFTER_AS))
        goto err_name;
    goto autogenerate_symbol_name;
   }
   result->ii_symbol_name = token.t_kwd;
   /* Warn about reserved identifiers */
   if (is_reserved_symbol_name(token.t_kwd) &&
       WARN(W_RESERVED_IDENTIFIER_IN_ALIAS_NAME,token.t_kwd))
       goto err_name;
   if unlikely(yield() < 0) goto err_name;
  } else {
   /* - `.foo.bar' */
autogenerate_symbol_name:
   /* Autogenerate the module import symbol name. */
   result->ii_symbol_name = get_module_symbol_name(result->ii_import_name,
                                                   return_value != 0);
   if unlikely(!result->ii_symbol_name) goto err_name;
   /* Warn about the auto-generated name being a reserved identifiers */
   if (is_reserved_symbol_name(result->ii_symbol_name) &&
       WARNAT(&result->ii_import_loc,
               allow_module_name ? W_RESERVED_IDENTIFIER_IN_AUTOGENERATED_SYMBOL_OR_MODULE_NAME
                                 : W_RESERVED_IDENTIFIER_IN_AUTOGENERATED_SYMBOL_NAME,
               result->ii_symbol_name))
       goto err_name;
  }
 } else if (tok == TOK_STRING ||
           (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
  /* - `"foo"'
   * - `"foo" as foobar'
   * - `"foo.bar"'
   * - `"foo.bar" as foobar' */
  unicode_printer_init(&printer);
  return_value = allow_module_name
               ? ast_parse_module_name(&printer,true)
               : ast_parse_symbol_name(&printer,true)
               ;
  if unlikely(return_value < 0) goto err_printer;
  result->ii_import_name = (DREF DeeStringObject *)unicode_printer_pack(&printer);
  if unlikely(!result->ii_import_name) goto err;
  if (tok != KWD_as)
      goto autogenerate_symbol_name;
  /* An import alias was given. */
  if unlikely(yield() < 0) goto err_name;
  if (!TPP_ISKEYWORD(tok)) {
   if (WARN(W_EXPECTED_KEYWORD_AFTER_AS))
       goto err_name;
   goto autogenerate_symbol_name;
  }
  result->ii_symbol_name = token.t_kwd;
  /* Warn about reserved identifiers in alias names. */
  if (is_reserved_symbol_name(token.t_kwd) &&
      WARN(W_RESERVED_IDENTIFIER_IN_ALIAS_NAME,token.t_kwd))
      goto err_name;
  if unlikely(yield() < 0) goto err_name;
 } else {
  if (WARN(W_EXPECTED_KEYWORD_OR_STRING_IN_IMPORT_LIST))
      goto err;
  return_value = 2;
 }
 return return_value;
err_printer:
 unicode_printer_fini(&printer);
 goto err;
err_name:
 Dee_Decref(result->ii_import_name);
err:
 return -1;
}

PRIVATE int DCALL
ast_import_all_from_module(DeeModuleObject *__restrict module,
                           struct ast_loc *loc) {
 struct module_symbol *iter,*end;
 ASSERT_OBJECT_TYPE(module,&DeeModule_Type);
 if (module == current_rootscope->rs_module) {
  if (WARNAT(loc,W_IMPORT_GLOBAL_FROM_OWN_MODULE))
      goto err;
  goto done;
 }
 end = (iter = module->mo_bucketv) + (module->mo_bucketm + 1);
 for (; iter != end; ++iter) {
  struct symbol *sym;
  struct TPPKeyword *name;
  if (!MODULE_SYMBOL_GETNAMESTR(iter)) continue; /* Empty slot. */
  if (iter->ss_flags&MODSYM_FHIDDEN) continue; /* Hidden symbol. */
  name = TPPLexer_LookupKeyword(MODULE_SYMBOL_GETNAMESTR(iter),
                                MODULE_SYMBOL_GETNAMELEN(iter),
                                1);
  if unlikely(!name) goto err;
  sym = get_local_symbol(name);
  if unlikely(sym) {
   /* Re-importing a different symbol that would collide with another
    * weak symbol will create an ambiguity when that symbol is used.
    * Instead of producing an error here and now, only do so if the
    * symbol ever turns out to be used.
    * That way, modules exporting symbols with the same name can still
    * both be used in the same namespace, with all symbols from both
    * imported using `import *'
    * Additionally, `SYMBOL_TYPE_AMBIG' symbols are weak, meaning that
    * in a situation where 2 modules `a' and `b' both define `foo', you
    * can write:
    * >> import * from a;
    * >> import * from b;
    * >> import foo from b;  // Explicitly link `foo' to be import from the desired module.
    */
   if (!SYMBOL_IS_WEAK(sym))
        continue; /* Not a weakly declared symbol. */
   /* Special handling for re-imports. */
   if (sym->s_type == SYMBOL_TYPE_EXTERN) {
    if (sym->s_extern.e_module == module) {
     if (sym->s_extern.e_symbol == iter)
         continue; /* Same declaration. */
    } else {
     /* TODO: Special handling when aliasing `MODSYM_FEXTERN'-symbols.
      *       Importing an external symbol that has been aliased should
      *       not cause ambiguity if it is the original symbol with which
      *       the new one would collide (this goes if at least either the
      *       old, or the new symbol has the `MODSYM_FEXTERN' flag)
      */

     /* Special case: When both the old and new symbol refers to an external `final global'
      * variable (with neither being `varying'), then ensure that the modules have been
      * initialized, and check if the values bound for the symbols differ.
      * If they are identical, we're allowed to assume that they simply alias each other, in
      * which case it doesn't really matter which one we use for binding. However in the interest
      * of minimizing dependencies, if either module is the builtin `deemon' module, bind against
      * the symbol as found in `deemon', otherwise check if either module uses the other, in which
      * case: bind against the symbol of the module being used (aka. further down in the dependency
      * tree) */
     if ((sym->s_extern.e_symbol->ss_flags & (MODSYM_FREADONLY|MODSYM_FCONSTEXPR|MODSYM_FPROPERTY|MODSYM_FEXTERN)) == (MODSYM_FREADONLY|MODSYM_FCONSTEXPR) &&
         (iter->ss_flags                   & (MODSYM_FREADONLY|MODSYM_FCONSTEXPR|MODSYM_FPROPERTY|MODSYM_FEXTERN)) == (MODSYM_FREADONLY|MODSYM_FCONSTEXPR)) {
      /* Both symbols are non-varying (allowing value inlining).
       * -> Make sure both modules have been loaded, and compare the values that have been bound.
       * NOTE: For this purpose, we must perform an exact comparison (i.e. `a === b') */
      int error;
      error = DeeModule_RunInit((DeeObject *)module);
      if unlikely(error < 0) goto err;
      if (error == 0) {
       error = DeeModule_RunInit((DeeObject *)sym->s_extern.e_module);
       if unlikely(error < 0) goto err;
       if (error == 0) {
        /* Both modules are now initialized. */
        DeeObject *old_val,*new_val;
        rwlock_read(&module->mo_lock);
        old_val = module->mo_globalv[iter->ss_index];
        rwlock_endread(&module->mo_lock);
        rwlock_read(&sym->s_extern.e_module->mo_lock);
        new_val = sym->s_extern.e_module->mo_globalv[sym->s_extern.e_symbol->ss_index];
        rwlock_endread(&sym->s_extern.e_module->mo_lock);
        if (old_val == new_val) {
         /* One of the modules contains a copy-alias (i.e. a secondary memory location)
          * for the other symbol. - Try to figure out which one is aliasing which, and
          * potentially re-assign the import such that the new module has less dependencies. */
         if (sym->s_extern.e_module != &deemon_module) {
          if (module == &deemon_module) {
do_reassign_new_alias:
           sym->s_extern.e_module = module;
           sym->s_extern.e_symbol = iter;
          } else {
           uint16_t i;
           /* Neither module is the builtin `deemon' module.
            * Check if one of the modules is importing the other,
            * or bind the new module if it doesn't have any imports. */
           if (!module->mo_importc) goto do_reassign_new_alias;
           for (i = 0; i < sym->s_extern.e_module->mo_importc; ++i) {
            if (module == sym->s_extern.e_module->mo_importv[i])
                goto do_reassign_new_alias;
           }
          }
         }
         continue;
        }
       }
      }
     }
    }
   }
   if (sym->s_type != SYMBOL_TYPE_AMBIG) {
    /* Turn the symbol into one that is ambiguous. */
    symbol_fini(sym);
    sym->s_type = SYMBOL_TYPE_AMBIG;
    if (loc) {
     memcpy(&sym->s_ambig.a_decl2,loc,
             sizeof(struct ast_loc));
    } else {
     loc_here(&sym->s_ambig.a_decl2);
    }
    if (sym->s_ambig.a_decl2.l_file)
        TPPFile_Incref(sym->s_ambig.a_decl2.l_file);
    sym->s_ambig.a_declc = 0;
    sym->s_ambig.a_declv = NULL;
   } else {
    /* Add another ambiguous symbol declaration location. */
    symbol_addambig(sym,loc);
   }
  } else {
   sym = new_local_symbol(name,loc);
   if unlikely(!sym) goto err;
   /* Define this symbol as an import from this module. */
   sym->s_type  = SYMBOL_TYPE_EXTERN;
   sym->s_flag |= SYMBOL_FWEAK; /* Symbols imported by `*' are defined weakly. */
   sym->s_extern.e_module = module;
   sym->s_extern.e_symbol = iter;
   Dee_Incref(module);
  }
 }
done:
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
ast_import_single_from_module(DeeModuleObject *__restrict module,
                              struct import_item *__restrict item) {
 struct module_symbol *sym;
 struct symbol *import_symbol;
 if (module == current_rootscope->rs_module) {
  if (WARNAT(&item->ii_import_loc,W_IMPORT_GLOBAL_FROM_OWN_MODULE))
      goto err;
  goto done;
 }
 if (item->ii_import_name) {
  sym = DeeModule_GetSymbolString(module,
                                  DeeString_STR(item->ii_import_name),
                                  DeeString_Hash((DeeObject *)item->ii_import_name));
  if (!sym) {
   if (WARNAT(&item->ii_import_loc,W_MODULE_IMPORT_NOT_FOUND,
              DeeString_STR(item->ii_import_name),
              DeeString_STR(module->mo_name)))
       goto err;
   goto done;
  }
 } else {
  sym = import_module_symbol(module,item->ii_symbol_name);
  if (!sym) {
   if (WARNAT(&item->ii_import_loc,W_MODULE_IMPORT_NOT_FOUND,
              item->ii_symbol_name->k_name,
              DeeString_STR(module->mo_name)))
       goto err;
   goto done;
  }
 }
 import_symbol = get_local_symbol(item->ii_symbol_name);
 if unlikely(import_symbol) {
  if (SYMBOL_IS_WEAK(import_symbol)) {
   SYMBOL_CLEAR_WEAK(import_symbol);
   goto init_import_symbol;
  }
  /* Another symbol with the same name had already been imported. */
  if (SYMBOL_TYPE(import_symbol) == SYMBOL_TYPE_EXTERN &&
      SYMBOL_EXTERN_MODULE(import_symbol) == module &&
      SYMBOL_EXTERN_SYMBOL(import_symbol) == sym) {
   /* It was the same symbol that had been imported before.
    * -> Ignore the secondary import! */
  } else {
   if (WARNAT(&item->ii_import_loc,W_IMPORT_ALIAS_IS_ALREADY_DEFINED,item->ii_symbol_name))
       goto err;
  }
 } else {
  import_symbol = new_local_symbol(item->ii_symbol_name,
                                  &item->ii_import_loc);
  if unlikely(!import_symbol) goto err;
init_import_symbol:
  import_symbol->s_type = SYMBOL_TYPE_EXTERN;
  SYMBOL_EXTERN_MODULE(import_symbol) = module;
  SYMBOL_EXTERN_SYMBOL(import_symbol) = sym;
  Dee_Incref(module);
 }
done:
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
ast_import_module(struct import_item *__restrict item) {
 DREF DeeModuleObject *module;
 struct symbol *import_symbol;
 if (item->ii_import_name) {
  module = import_module_by_name(item->ii_import_name,
                                &item->ii_import_loc);
 } else {
  DREF DeeStringObject *module_name;
  module_name = (DREF DeeStringObject *)DeeString_NewUtf8(item->ii_symbol_name->k_name,
                                                          item->ii_symbol_name->k_size,
                                                          STRING_ERROR_FSTRICT);
  if unlikely(!module_name) goto err;
  module = import_module_by_name(module_name,
                                &item->ii_import_loc);
  Dee_Decref(module_name);
 }
 if unlikely(!module) goto err;
 import_symbol = get_local_symbol(item->ii_symbol_name);
 if unlikely(import_symbol) {
  if (SYMBOL_IS_WEAK(import_symbol)) {
   SYMBOL_CLEAR_WEAK(import_symbol);
   goto init_import_symbol;
  }
  /* Another symbol with the same name had already been imported. */
  if ((SYMBOL_TYPE(import_symbol) == SYMBOL_TYPE_MODULE &&
       SYMBOL_MODULE_MODULE(import_symbol) == module) ||
      (module == current_rootscope->rs_module &&
       SYMBOL_TYPE(import_symbol) == SYMBOL_TYPE_MYMOD)) {
   /* The same module has already been imported under this name! */
  } else {
   if (WARNAT(&item->ii_import_loc,W_IMPORT_ALIAS_IS_ALREADY_DEFINED,item->ii_symbol_name))
       goto err_module;
  }
  Dee_Decref(module);
 } else {
  import_symbol = new_local_symbol(item->ii_symbol_name,
                                  &item->ii_import_loc);
  if unlikely(!import_symbol) goto err_module;
init_import_symbol:
  if (module == current_rootscope->rs_module) {
   SYMBOL_TYPE(import_symbol) = SYMBOL_TYPE_MYMOD;
   Dee_Decref(module);
  } else {
   SYMBOL_TYPE(import_symbol)             = SYMBOL_TYPE_MODULE;
   SYMBOL_MODULE_MODULE(import_symbol) = module; /* Inherit reference */
  }
 }
 return 0;
err_module:
 Dee_Decref(module);
err:
 return -1;
}


INTDEF DREF struct ast *FCALL ast_sym_import_from_deemon(void);

INTERN int FCALL ast_parse_post_import(void) {
/* - import deemon;
 * - import deemon, util;
 * - import my_deemon = deemon;
 * - import my_deemon = "deemon";
 * - import my_deemon = deemon, my_util = util;
 * - import deemon as my_deemon;
 * - import "deemon" as my_deemon;
 * - import deemon as my_deemon, util as my_util;
 * - import * from deemon;
 * - import object from deemon;
 * - import object, list from deemon;
 * - import my_object = object from deemon;
 * - import my_object = "object" from deemon;
 * - import my_object = object, my_list = list from deemon;
 * - import object as my_object from deemon;
 * - import "object" as my_object from deemon;
 * - import object as my_object, list as my_list from deemon; */
 int error; struct import_item item;
 bool allow_modules = true;
 struct ast_loc star_loc;
 struct import_item *item_v;
 size_t item_a,item_c;
 DREF DeeModuleObject *module;
 star_loc.l_file = NULL; /* When non-NULL, import all */
 if (tok == '*') {
  loc_here(&star_loc);
  if unlikely(yield() < 0) goto err;
  if (tok == KWD_from) {
   if unlikely(yield() < 0) goto err;
   module = parse_module_byname(true);
   if unlikely(!module) goto err;
   error = ast_import_all_from_module(module,&star_loc);
   Dee_Decref(module);
   goto done;
  } else if (tok == ',') {
   item_a = item_c = 0;
   item_v = NULL;
   allow_modules = false;
   goto import_parse_list;
  }
  if (WARN(W_EXPECTED_COMMA_OR_FROM_AFTER_START_IN_IMPORT_LIST))
      goto err;
  goto done;
 }
 error = parse_import_symbol(&item,true);
 if unlikely(error < 0) goto err;
 if unlikely(error == 2) goto done;
 if (error) {
  /* Module import list */
  for (;;) {
import_item_as_module:
   error = ast_import_module(&item);
   Dee_XDecref(item.ii_import_name);
   if unlikely(error) goto err;
parse_module_import_list:
   if (tok != ',') break;
   if unlikely(yield() < 0) goto err;
   error = parse_import_symbol(&item,true);
   if unlikely(error < 0) goto err;
   if unlikely(error == 2) break;
  }
  /* Warn if the module import list is followed by a `from' */
  if (tok == KWD_from &&
      WARN(W_UNEXPECTED_FROM_AFTER_MODULE_IMPORT_LIST))
      goto err;
 } else if (tok == KWD_from) {
  /*  - `import foo from bar' */
  if unlikely(yield() < 0) goto err_item;
  module = parse_module_byname(true);
  if unlikely(!module) goto err_item;
  error = ast_import_single_from_module(module,&item);
  Dee_XDecref(item.ii_import_name);
  Dee_Decref(module);
  if unlikely(error) goto err;
 } else if (tok == ',') {
  item_a = 4;
  item_v = (struct import_item *)Dee_TryMalloc(4*sizeof(struct import_item));
  if unlikely(!item_v) {
   item_a = 2;
   item_v = (struct import_item *)Dee_Malloc(2*sizeof(struct import_item));
   if unlikely(!item_v) goto err_item;
  }
  item_v[0] = item;
  item_c = 1;
import_parse_list:
  do {
   ASSERT(tok == ',');
   if unlikely(yield() < 0)
      goto err_item_v;
   if (tok == '*') {
    if (star_loc.l_file) {
     if (WARN(W_UNEXPECTED_STAR_DUPLICATION_IN_IMPORT_LIST))
         goto err_item_v;
    } else {
     loc_here(&star_loc);
    }
    if unlikely(yield() < 0)
       goto err_item_v;
    /* Don't allow modules after `*' confirmed that symbols are being imported.
     * -> There is no such thing as import-all-modules. */
    allow_modules = false;
   } else {
    /* Parse the next import item. */
    ASSERT(item_c <= item_a);
    if (item_c >= item_a) {
     /* Allocate more space. */
     struct import_item *new_item_v;
     size_t new_item_a = item_a * 2;
     if unlikely(!new_item_a) new_item_a = 2;
     new_item_v = (struct import_item *)Dee_TryRealloc(item_v,new_item_a*
                                                       sizeof(struct import_item));
     if unlikely(!new_item_v) {
      new_item_a = item_c + 1;
      new_item_v = (struct import_item *)Dee_Realloc(item_v,new_item_a*
                                                     sizeof(struct import_item));
      if unlikely(!new_item_v) goto err_item_v;
     }
     item_v = new_item_v;
     item_a = new_item_a;
    }
    error = parse_import_symbol(&item_v[item_c],allow_modules);
    if unlikely(error < 0) goto err_item_v;
    if unlikely(error == 2) break;
    ++item_c; /* Import parsing confirmed. */
    if (error) {
     /* We're dealing with a module import list!
      * -> Import all items already parsed as modules. */
     size_t i;
     ASSERT(allow_modules);
     for (i = 0; i < item_c; ++i) {
      if unlikely(ast_import_module(&item_v[i]))
         goto err_item_v;
      Dee_XClear(item_v[i].ii_import_name);
     }
     Dee_Free(item_v);
     /* Then continue by parsing the remainder as a module import list. */
     goto parse_module_import_list;
    }
   }
  } while (tok == ',');
  /* A multi-item, comma-separated import list has now been parsed. */
  if (tok == KWD_from) {
   size_t i;
   /* import foo, bar, foobar from foobarfoo;  (symbol import) */
   if unlikely(yield() < 0) goto err_item_v;
   module = parse_module_byname(true);
   if unlikely(!module) goto err_item_v;
   /* If `*' was apart of the symbol import list,
    * start by importing all symbols from the module. */
   if (star_loc.l_file) {
    if unlikely(ast_import_all_from_module(module,&star_loc))
       goto err_item_v_module;
   }
   /* Now import all the explicitly defined symbols. */
   for (i = 0; i < item_c; ++i) {
    if unlikely(ast_import_single_from_module(module,&item_v[i]))
       goto err_item_v_module;
    Dee_XClear(item_v[i].ii_import_name);
   }
   Dee_Decref(module);
  } else {
   size_t i;
   if unlikely(!allow_modules) {
    /* Warn if there is a `from' missing following a symbol import list. */
    if (WARN(W_EXPECTED_FROM_AFTER_SYMBOL_IMPORT_LIST))
        goto err_item_v;
   }
   /* import foo, bar, foobar;  (module import) */
   for (i = 0; i < item_c; ++i) {
    if unlikely(ast_import_module(&item_v[i]))
       goto err_item_v;
    Dee_XClear(item_v[i].ii_import_name);
   }
  }
  Dee_Free(item_v);
  goto done;
err_item_v_module:
  Dee_Decref(module);
err_item_v:
  while (item_c--)
      Dee_XDecref(item_v[item_c].ii_import_name);
  Dee_Free(item_v);
  goto err;
err_item:
  Dee_XDecref(item.ii_import_name);
  goto err;
 } else {
  /* This is simply a single-module, stand-along import statement. */
  goto import_item_as_module;
 }
done:
 return 0;
err:
 return -1;
}

INTERN DREF struct ast *FCALL
ast_parse_import_hybrid(unsigned int *pwas_expression) {
 DREF struct ast *result;
 struct ast_loc import_loc;
 ASSERT(tok == KWD_import);
 loc_here(&import_loc);
 if unlikely(yield() < 0) goto err;
 if (tok == '(' || tok == KWD_pack) {
  /* `import', as seen in expressions. */
  result = ast_setddi(ast_sym_import_from_deemon(),&import_loc);
  if unlikely(!result) goto err;
  result = ast_parse_unary_operand(result);
  if unlikely(!result) goto err;
  result = ast_parse_postexpr(result);
  if (pwas_expression)
     *pwas_expression = AST_PARSE_WASEXPR_YES;
 } else {
  result = ast_setddi(ast_constexpr(Dee_None),&import_loc);
  if unlikely(!result) goto err;
  if unlikely(ast_parse_post_import())
     goto err_r;
  if (pwas_expression)
     *pwas_expression = AST_PARSE_WASEXPR_NO;
 }
 return result;
err_r:
 ast_decref(result);
err:
 return NULL;
}


INTERN DREF struct ast *FCALL ast_parse_import(void) {
 DREF DeeModuleObject *module;
 DREF struct ast *result;
 struct ast_loc import_loc;
 /* There are many valid ways of writing import statements:
  *  - import("deemon");
  *  - import pack "deemon";
  *  - import deemon;
  *  - import deemon, util;
  *  - import my_deemon = deemon;
  *  - import my_deemon = "deemon";
  *  - import my_deemon = deemon, my_util = util;
  *  - import deemon as my_deemon;
  *  - import "deemon" as my_deemon;
  *  - import deemon as my_deemon, util as my_util;
  *  - import * from deemon;
  *  - import object from deemon;
  *  - import object, list from deemon;
  *  - import my_object = object from deemon;
  *  - import my_object = "object" from deemon;
  *  - import my_object = object, my_list = list from deemon;
  *  - import object as my_object from deemon;
  *  - import "object" as my_object from deemon;
  *  - import object as my_object, list as my_list from deemon;
  *  - from deemon import *;
  *  - from deemon import object;
  *  - from deemon import object, list;
  *  - from deemon import my_object = object;
  *  - from deemon import my_object = "object";
  *  - from deemon import my_object = object, my_list = list;
  *  - from deemon import object as my_object;
  *  - from deemon import "object" as my_object;
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
 loc_here(&import_loc);
 if (tok == KWD_from) {
  /* - from deemon import *;
   * - from deemon import object;
   * - from deemon import object, list;
   * - from deemon import my_object = object;
   * - from deemon import my_object = "object";
   * - from deemon import my_object = object, my_list = list;
   * - from deemon import object as my_object;
   * - from deemon import "object" as my_object;
   * - from deemon import object as my_object, list as my_list; */
  bool did_import_all = false;
  result = ast_setddi(ast_constexpr(Dee_None),&import_loc);
  if unlikely(!result) goto err;
  if unlikely(yield() < 0) goto err_r;
  module = parse_module_byname(true);
  if unlikely(!module) goto err_r;
  /* All right! we've got the module. */
  if unlikely(likely(tok == KWD_import) ? (yield() < 0) :
              WARN(W_EXPECTED_IMPORT_AFTER_FROM))
     goto err_r_module;
  for (;;) {
   /* Parse an entire import list. */
   if (tok == '*') {
    if (did_import_all &&
        WARN(W_UNEXPECTED_STAR_DUPLICATION_IN_IMPORT_LIST))
        goto err_r_module;
    if unlikely(ast_import_all_from_module(module,NULL))
       goto err_r_module;
    if unlikely(yield() < 0)
       goto err_r_module;
    did_import_all = true;
   } else {
    int error; struct import_item item;
    error = parse_import_symbol(&item,false);
    if unlikely(error < 0) goto err_r_module;
    if unlikely(error == 2) break; /* failed */
    error = ast_import_single_from_module(module,&item);
    Dee_XDecref(item.ii_import_name);
    if unlikely(error) goto err_r_module;
   }
   if (tok != ',') break;
   if unlikely(yield() < 0) goto err_r_module;
  }
  Dee_Decref(module);
 } else {
  /* - import("deemon");
   * - import pack "deemon";
   * - import deemon;
   * - import deemon, util;
   * - import my_deemon = deemon;
   * - import my_deemon = "deemon";
   * - import my_deemon = deemon, my_util = util;
   * - import deemon as my_deemon;
   * - import "deemon" as my_deemon;
   * - import deemon as my_deemon, util as my_util;
   * - import * from deemon;
   * - import object from deemon;
   * - import object, list from deemon;
   * - import my_object = object from deemon;
   * - import my_object = "object" from deemon;
   * - import my_object = object, my_list = list from deemon;
   * - import object as my_object from deemon;
   * - import "object" as my_object from deemon;
   * - import object as my_object, list as my_list from deemon; */
  ASSERT(tok == KWD_import);
  if unlikely(yield() < 0) goto err;
  if (tok == '(' || tok == KWD_pack) {
   /* `import', as seen in expressions. */
   result = ast_setddi(ast_sym_import_from_deemon(),&import_loc);
   if unlikely(!result) goto err;
   result = ast_parse_unary_operand(result);
   if unlikely(!result) goto err;
   result = ast_parse_postexpr(result);
   goto done;
  }
  result = ast_setddi(ast_constexpr(Dee_None),&import_loc);
  if unlikely(!result) goto err;
  if unlikely(ast_parse_post_import())
     goto err_r;
 }
done:
 return result;
err_r_module:
 Dee_Decref(module);
err_r:
 ast_decref(result);
err:
 return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_IMPORT_C */
