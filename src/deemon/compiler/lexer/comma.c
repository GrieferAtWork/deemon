/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_COMPILER_LEXER_COMMA_C
#define GUARD_DEEMON_COMPILER_LEXER_COMMA_C 1

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/util/string.h>
#include <deemon/seq.h>
#include <deemon/map.h>
#include <deemon/dict.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/string.h>
#include <deemon/list.h>
#include <deemon/tuple.h>

#include "../../runtime/strings.h"

DECL_BEGIN

#define IS_SYMBOL_NAME(tok)  \
   (TPP_ISKEYWORD(tok) && (!KWD_ISUNARY(tok) || (tok) == KWD_none))


INTERN void DCALL
astlist_fini(struct astlist *__restrict self) {
 size_t i;
 for (i = 0; i < self->ast_c; ++i)
     ast_decref(self->ast_v[i]);
 Dee_Free(self->ast_v);
}
INTERN int DCALL
astlist_upsize(struct astlist *__restrict self,
               size_t min_add) {
 DREF struct ast **new_vector;
 size_t new_alloc = self->ast_a*2;
 ASSERT(min_add != 0);
 if (!new_alloc) new_alloc = min_add;
 while ((new_alloc-self->ast_c) < min_add) new_alloc *= 2;
do_realloc:
 new_vector = (DREF struct ast **)Dee_TryRealloc(self->ast_v,new_alloc*
                                                   sizeof(DREF struct ast *));
 if unlikely(!new_vector) {
  if (new_alloc != self->ast_c+min_add) {
   new_alloc = self->ast_c+min_add;
   goto do_realloc;
  }
  if (Dee_CollectMemory(new_alloc*sizeof(DREF struct ast *)))
      goto do_realloc;
  return -1;
 }
 self->ast_v = new_vector;
 self->ast_a = new_alloc;
 ASSERT(self->ast_a > self->ast_c);
 ASSERT((self->ast_a-self->ast_c) >= min_add);
 return 0;
}

INTERN void DCALL
astlist_trunc(struct astlist *__restrict self) {
 DREF struct ast **new_vector;
 if (self->ast_c == self->ast_a) return;
 new_vector = (DREF struct ast **)Dee_TryRealloc(self->ast_v,self->ast_c*
                                                   sizeof(DREF struct ast *));
 if likely(new_vector) self->ast_v = new_vector;
}
INTERN int DCALL
astlist_append(struct astlist *__restrict self,
               struct ast *__restrict branch) {
 if (self->ast_c == self->ast_a &&
     astlist_upsize(self,1)) return -1;
 self->ast_v[self->ast_c++] = branch;
 ast_incref(branch);
 return 0;
}
INTERN int DCALL
astlist_appendall(struct astlist *__restrict self,
                  struct astlist *__restrict other) {
 size_t avail = self->ast_a-self->ast_c;
 if (avail < other->ast_c) {
  if (!self->ast_c) {
   /* Even more simple (Steal everything) */
   Dee_Free(self->ast_v);
   self->ast_c  = other->ast_c;
   self->ast_a  = other->ast_a;
   self->ast_v  = other->ast_v;
   other->ast_c = 0;
   other->ast_a = 0;
   other->ast_v = NULL;
   return 0;
  }
  if (astlist_upsize(self,other->ast_c-avail))
      return -1;
 }
 ASSERT(self->ast_c+other->ast_c <= self->ast_a);
 MEMCPY_PTR(self->ast_v+self->ast_c,other->ast_v,other->ast_c);
 self->ast_c += other->ast_c;
 other->ast_c = 0; /* Steal all of these references. */
 return 0;
}

INTERN int DCALL
ast_parse_lookup_mode(unsigned int *__restrict pmode) {
next_modifier:
 switch (tok) {

 case KWD_final:
  if (*pmode & LOOKUP_SYM_FINAL &&
      WARN(W_VARIABLE_MODIFIER_DUPLICATED))
      goto err;
  *pmode |= LOOKUP_SYM_FINAL;
  goto continue_modifier;

 case KWD_varying:
  if (*pmode & LOOKUP_SYM_VARYING &&
      WARN(W_VARIABLE_MODIFIER_DUPLICATED))
      goto err;
  *pmode |= LOOKUP_SYM_VARYING;
  goto continue_modifier;

 case KWD_local:
  if (*pmode & LOOKUP_SYM_VLOCAL &&
      WARN(W_VARIABLE_MODIFIER_DUPLICATED))
      goto err;
  if (*pmode & LOOKUP_SYM_VGLOBAL) {
   if (WARN(W_VARIABLE_MODIFIER_INCOMPATIBLE,
            DeeString_STR(&str_global)))
       goto err;
   *pmode &= ~LOOKUP_SYM_VGLOBAL;
  }
  *pmode |= LOOKUP_SYM_VLOCAL;
continue_modifier:
  if unlikely(yield() < 0) goto err;
  goto next_modifier;

 case TOK_COLLON_COLLON:
  /* Backwards compatibility with deemon 100+ */
  if (WARN(W_DEPRECATED_GLOBAL_PREFIX))
      goto err;
  ATTR_FALLTHROUGH
 case KWD_global:
  if (*pmode & LOOKUP_SYM_VGLOBAL &&
      WARN(W_VARIABLE_MODIFIER_DUPLICATED))
      goto err;
  if (*pmode & LOOKUP_SYM_VLOCAL) {
   if (WARN(W_VARIABLE_MODIFIER_INCOMPATIBLE,
            DeeString_STR(&str_local)))
       goto err;
   *pmode &= ~LOOKUP_SYM_VLOCAL;
  }
  *pmode |= LOOKUP_SYM_VGLOBAL;
  goto continue_modifier;

 case KWD_static:
  if (*pmode & LOOKUP_SYM_STATIC &&
      WARN(W_VARIABLE_MODIFIER_DUPLICATED))
      goto err;
  if (*pmode & LOOKUP_SYM_STACK) {
   if (WARN(W_VARIABLE_MODIFIER_INCOMPATIBLE,"__stack"))
       goto err;
   *pmode &= ~LOOKUP_SYM_STACK;
  }
  *pmode |= LOOKUP_SYM_STATIC;
  goto continue_modifier;

 case KWD___stack:
  if (*pmode & LOOKUP_SYM_STACK &&
      WARN(W_VARIABLE_MODIFIER_DUPLICATED))
      goto err;
  if (*pmode & LOOKUP_SYM_STATIC) {
   if (WARN(W_VARIABLE_MODIFIER_INCOMPATIBLE,
            DeeString_STR(&str_static)))
       goto err;
   *pmode &= ~LOOKUP_SYM_STATIC;
  }
  *pmode |= LOOKUP_SYM_STACK;
  goto continue_modifier;

 default: break;
 }
 if ((*pmode & (LOOKUP_SYM_VARYING | LOOKUP_SYM_FINAL)) == LOOKUP_SYM_VARYING) {
  if (WARN(W_VARYING_WITHOUT_FINAL))
      goto err;
 }
 return 0;
err:
 return -1;
}


INTERN DREF struct ast *DCALL
ast_parse_comma(uint16_t mode, uint16_t flags, uint16_t *pout_mode) {
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
 struct decl_ast decl;
#endif
 DREF struct ast *current; bool need_semi; int error;
 unsigned int lookup_mode; struct ast_loc loc;
 /* In: "foo,bar = 10,x,y = getvalue()...,7"
  *              |     |                 |
  *              +-------------------------  expr_batch = { }
  *                    |                 |   expr_comma = { foo, bar }
  *                    |                 |
  *                    +-------------------  expr_batch = { foo, (bar = 10) }
  *                                      |   expr_comma = { x }
  *                                      |
  *                                      +-  expr_batch = { foo, (bar = 10), (x,y = getvalue()...) }
  *                                          expr_comma = { }
  */
 struct astlist expr_batch = ASTLIST_INIT; /* Expressions that have already been written to. */
 struct astlist expr_comma = ASTLIST_INIT; /* Expressions that are pending getting assigned. */
#if AST_COMMA_ALLOWVARDECLS == LOOKUP_SYM_ALLOWDECL
 lookup_mode = mode&AST_COMMA_ALLOWVARDECLS;
#else
 lookup_mode = ((mode&AST_COMMA_ALLOWVARDECLS)
                ? LOOKUP_SYM_ALLOWDECL
                : LOOKUP_SYM_NORMAL);
#endif
 /* Allow explicit visibility modifiers when variable can be declared. */
 if (mode&AST_COMMA_ALLOWVARDECLS &&
     ast_parse_lookup_mode(&lookup_mode))
     goto err;
 /* Allow `final' variable declarations.
  * >> final global foo = 42;
  * A variable declared as `final' can only be assigned once during its life-time,
  * with any attempt to assign another value after that point resulting in an Error
  * being thrown.
  * Semantically, this behavior would be identical to java, while finally adding a
  * way for user-code to make use of module-scope constants which the compiler is
  * allowed to inline at the use-site whenever used.
  * Problems:
  *  - User-defined classes already use `final' where it would appear if it was
  *    a variable/storage modifier, with it appearing in that location having a
  *    different meaning that specifying a write-once variable.
  *    Solution #1: All user-defined classes are stored in write-once variables (not a good idea)
  *    Solution #2: User-defined classes are not stored in write-once variables, and the
  *                `final' prefix is not applied to class itself (disallowing sub-classing)
  *              -> This way, the user could still write `final MyClass = class { ... };'
  *                 or `final MyClass = final class { ... };', thus solving the problem.
  *  - Not everything that's final should also be constant.
  *    By default, any `final' global should be, however another variable modifier
  *   `varying' should be introduced which may be combined with `final' to declare
  *    a write-once variable whose value may not be assumed to be constant at compile
  *    time, as it may not be consistent across multiple runs.
  *    As far as runtime support goes, that's already there:
  *     - __stack/local variables don't need runtime support, as they would be handled by the compiler
  *     - global variables already provide and implement 2 flags:
  *       - MODSYM_FREADONLY   (enables write-once semantics)
  *       - MODSYM_FCONSTEXPR  (allows the optimizer to inline the symbol's value)
  *    foo.dee:
  *    >> global normal_global = 42;
  *    >> global final fixed_value = 42;
  *    >> global final changing_rand = rand(); // This shouldn't be done, but can't be prevented...
  *    >> global final varying changing_value = rand();
  *    bar.dee:
  *    >> import * from foo;
  *    >> print normal_global;  // print extern @bar:@normal_global, nl  // Cannot be inlined
  *    >> print fixed_value;    // print @42, nl                         // Can be inlined
  *    >> print changing_rand;  // print @<UNDEFINED>, nl                // Can be inlined
  *    >> print changing_value; // print extern @bar:@changing_value, nl // Cannot be inlined
  */

next_expr:
 need_semi = !!(mode&AST_COMMA_PARSESEMI);
 /* Parse an expression (special handling for functions/classes) */
 if (tok == KWD_class) {
  /* Declare a new class */
  uint16_t class_flags = current_tags.at_class_flags & 0xf; /* From tags. */
  struct TPPKeyword *class_name = NULL;
  unsigned int symbol_mode = lookup_mode;
  if (symbol_mode & LOOKUP_SYM_FINAL)
      class_flags |= TP_FFINAL;
  symbol_mode &= ~(LOOKUP_SYM_FINAL | LOOKUP_SYM_VARYING);
  loc_here(&loc);
  if unlikely(yield() < 0) goto err;
  if (tok == KWD_final && !(class_flags & TP_FFINAL)) {
   class_flags |= TP_FFINAL;
   if unlikely(yield() < 0) goto err;
  }
  if (TPP_ISKEYWORD(tok)) {
   class_name = token.t_kwd;
   if unlikely(yield() < 0) goto err;
   if ((symbol_mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT) {
    /* Use the default mode appropriate for the current scope. */
    if (current_scope == &current_rootscope->rs_scope.bs_scope)
         symbol_mode |= LOOKUP_SYM_VGLOBAL;
    else symbol_mode |= LOOKUP_SYM_VLOCAL;
   }
  }
  current = ast_setddi(ast_parse_class(class_flags,class_name,
                                       class_name != NULL,
                                       symbol_mode),
                      &loc);
  if unlikely(!current) goto err;
  need_semi = false; /* Classes always have braces and don't need semicolons. */
#if 0 /* TODO: property variable */
 } else if (tok == KWD_property) {
#endif
 } else if (tok == KWD_function) {
  /* Declare a new function */
  struct TPPKeyword *function_name = NULL;
  struct symbol *function_symbol = NULL;
  unsigned int symbol_mode = lookup_mode;
  struct ast_loc function_name_loc;
  loc_here(&loc);
  if unlikely(yield() < 0) goto err;
  if (TPP_ISKEYWORD(tok)) {
   loc_here(&function_name_loc);
   function_name = token.t_kwd;
   if unlikely(yield() < 0) goto err;
   if ((symbol_mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT) {
    /* Use the default mode appropriate for the current scope. */
    if (current_scope == &current_rootscope->rs_scope.bs_scope)
         symbol_mode |= LOOKUP_SYM_VGLOBAL;
    else symbol_mode |= LOOKUP_SYM_VLOCAL;
   }
   /* Create the symbol that will be used by the function. */
   function_symbol = lookup_symbol(symbol_mode,function_name,&loc);
   if unlikely(!function_symbol) goto err;
  }
  {
   struct ast_tags_printers temp;
   AST_TAGS_BACKUP_PRINTERS(temp);
   current = ast_parse_function(function_name,&need_semi,false,&loc
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
                                ,&decl
#endif
                                );
   AST_TAGS_RESTORE_PRINTERS(temp);
  }
  if unlikely(!current) goto err;
  /* Pack together the documentation string for the function. */
  if (function_symbol) {
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
   if (function_symbol->s_decltype.da_type != DAST_NONE) {
    if (!decl_ast_equal(&function_symbol->s_decltype,&decl)) {
     if (WARN(W_SYMBOL_TYPE_DECLARATION_CHANGED,function_symbol))
         goto err_decl;
    }
    decl_ast_fini(&decl);
   } else {
    decl_ast_move(&function_symbol->s_decltype,&decl);
   }
#endif
   if (function_symbol->s_type == SYMBOL_TYPE_GLOBAL &&
      !function_symbol->s_global.g_doc) {
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
    function_symbol->s_global.g_doc = (DREF DeeStringObject *)ast_tags_doc(&function_symbol->s_decltype);
#else
    function_symbol->s_global.g_doc = (DREF DeeStringObject *)ast_tags_doc();
#endif
    if unlikely(!function_symbol->s_global.g_doc) goto err;
   }
  } else {
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
   decl_ast_fini(&decl);
#endif
  }
  if unlikely(ast_tags_clear()) goto err_current;
  if (function_symbol) {
   DREF struct ast *function_name_ast,*merge;
   /* Store the function in the parsed symbol. */
   function_name_ast = ast_setddi(ast_sym(function_symbol),&function_name_loc);
   if unlikely(!function_name_ast) goto err_current;
   merge = ast_setddi(ast_action2(AST_FACTION_STORE,function_name_ast,current),&loc);
   ast_decref(function_name_ast);
   if unlikely(!merge) goto err_current;
   ast_decref(current);
   current = merge;
  }
 } else {
  if (mode & AST_COMMA_ALLOWKWDLIST) {
   if (TPP_ISKEYWORD(tok)) {
    /* If the next token is a `:', then we're currently at a keyword list label,
     * in which case we're supposed to stop and let the caller deal with this. */
    char *next = peek_next_token(NULL);
    if unlikely(!next) goto err;
    if (*next == ':') {
     /* Make sure it isn't a `::' or `:=' token. */
     ++next;
     while (SKIP_WRAPLF(next,token.t_file->f_end));
     if (*next != ':' && *next != '=')
          goto done_expression_nocurrent;
    }
   }
   if (tok == TOK_POW) /* foo(**bar) --> Invoke using `bar' for keyword arguments. */
       goto done_expression_nocurrent;
  }
  if (!IS_SYMBOL_NAME(tok) &&
      (lookup_mode&LOOKUP_SYM_VMASK) != LOOKUP_SYM_VDEFAULT) {
   /* Warn if an explicit visibility modifier isn't followed by a symbol name. */
   if (WARN(W_EXPECTED_VARIABLE_AFTER_VISIBILITY))
       goto err;
  }
  current = ast_parse_expr(lookup_mode);
  /* Check for errors. */
  if unlikely(!current)
     goto err;
  if ((lookup_mode & LOOKUP_SYM_ALLOWDECL) && TPP_ISKEYWORD(tok) &&
    (!(mode & AST_COMMA_NOSUFFIXKWD) || !is_reserved_symbol_name(token.t_kwd))) {
   /* C-style variable declarations. */
   struct symbol *var_symbol;
   DREF struct ast *args,*merge;
   struct ast_loc symbol_name_loc;
   if (KWD_IS_D100_VARIABLE_MODIFIER(tok)) {
    /* Deemon 100+ used to allow `int local x;' as alias for `local x = int()'.
     * While this isn't support anymore, still try to emulate it... */
    if (WARN(W_DEPRECATED_LOOKUP_MODE_AFTER_VAR_TYPE))
        goto err_current;
    if (ast_parse_lookup_mode(&lookup_mode))
        goto err;
   }
   loc_here(&symbol_name_loc);
   var_symbol = get_local_symbol(token.t_kwd);
   if unlikely(var_symbol) {
    if (WARN(W_VARIABLE_ALREADY_EXISTS,token.t_kwd))
        goto err_current;
   } else {
    /* Create a new symbol for the initialized variable. */
    var_symbol = new_local_symbol(token.t_kwd,NULL);
    if unlikely(!var_symbol) goto err_current;
    if (lookup_mode & LOOKUP_SYM_FINAL) {
     var_symbol->s_flag |= SYMBOL_FFINAL;
     if (lookup_mode & LOOKUP_SYM_VARYING)
         var_symbol->s_flag |= SYMBOL_FVARYING;
    }
    if (lookup_mode & LOOKUP_SYM_STATIC) {
     var_symbol->s_type = SYMBOL_TYPE_STATIC;
    } else if (lookup_mode & LOOKUP_SYM_STACK) {
     var_symbol->s_type = SYMBOL_TYPE_STACK;
    } else if ((lookup_mode & LOOKUP_SYM_VGLOBAL) ||
             (!(lookup_mode & LOOKUP_SYM_VLOCAL) &&
                current_scope == (DeeScopeObject *)current_rootscope)) {
     var_symbol->s_type = SYMBOL_TYPE_GLOBAL;
     var_symbol->s_global.g_doc = NULL;
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
     if (var_symbol->s_decltype.da_type == DAST_NONE) {
      /* Try to extract documentation information from the C-declaration's type specifier. */
      switch (current->a_type) {
      case AST_SYM:
       var_symbol->s_decltype.da_type   = DAST_SYMBOL;
       var_symbol->s_decltype.da_symbol = current->a_sym;
       symbol_incref(current->a_sym);
       break;
      case AST_CONSTEXPR:
       var_symbol->s_decltype.da_type  = DAST_CONST;
       var_symbol->s_decltype.da_const = current->a_constexpr;
       Dee_Incref(current->a_constexpr);
       break;
      default: break;
      }
     }
     /* Package together documentation tags for this variable symbol. */
     var_symbol->s_global.g_doc = (DREF DeeStringObject *)ast_tags_doc(&var_symbol->s_decltype);
     if unlikely(!var_symbol->s_global.g_doc) goto err_current;
#else /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
     /* Package together documentation tags for this variable symbol. */
     var_symbol->s_global.g_doc = (DREF DeeStringObject *)ast_tags_doc();
     if unlikely(!var_symbol->s_global.g_doc) goto err_current;
#endif /* !CONFIG_HAVE_DECLARATION_DOCUMENTATION */
    } else {
     var_symbol->s_type = SYMBOL_TYPE_LOCAL;
    }
   }
   if unlikely(ast_tags_clear()) goto err_current;
   if unlikely(yield() < 0) goto err_current;

   /* Allow syntax like this:
    * >> list my_list;
    * >> int my_int = 42;
    * >> MyClass my_instance(10,20,30);
    * >> thread my_thread = []{
    * >>     print "Thread execution...";
    * >> };
    * >> my_thread.start();
    * Same as:
    * >> local my_list = list();
    * >> local my_int = int(42);
    * >> local my_instance = MyClass(10,20,30);
    * >> local my_thread = thread([]{
    * >>     print "Thread execution...";
    * >> });
    * >> my_thread.start();
    * Now that C compatibility is gone, there's no ambiguity to this!
    */
   if (tok == '=' || tok == '{') {
    /* Single-operand argument list. */
    DREF struct ast **exprv;
    struct ast_loc equal_loc;
    loc_here(&equal_loc);
    if (tok == '=' && unlikely(yield() < 0)) goto err_current;
    /* Parse a preferred-type brace expression. */
    args = ast_parse_expr(LOOKUP_SYM_NORMAL);
    if unlikely(!args) goto err_current;
    /* Wrap the returned ast in a 1-element tuple (for the argument list) */
    exprv = (DREF struct ast **)Dee_Malloc(1*sizeof(DREF struct ast *));
    if unlikely(!exprv) goto err_args;
    exprv[0] = args; /* Inherit */
    /* Create a multi-branch AST for the assigned expression. */
    /* TODO: Add support for applying annotations here! */
    args = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE,1,exprv),&equal_loc);
    if unlikely(!args) { ast_decref(exprv[0]); Dee_Free(exprv); goto err_current; }
   } else if (tok == '(' || tok == KWD_pack) {
    /* Comma-separated argument list. */
    uint32_t old_flags;
    bool has_paren = tok == '(';
    old_flags = TPPLexer_Current->l_flags;
    TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
    if unlikely(yield() < 0) {
     TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
     goto err_current;
    }
    /* TODO: Add support for applying annotations here! */
    if (tok == ')' || (!has_paren && !maybe_expression_begin())) {
     /* Empty argument list (Same as none at all). */
     args = ast_sethere(ast_constexpr(Dee_EmptyTuple));
    } else {
     args = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
                            AST_FMULTIPLE_TUPLE,
                            NULL);
    }
    TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
    if unlikely(!args) goto err_current;
    if (has_paren) {
     if unlikely(likely(tok == ')') ? (yield() < 0) :
                 WARN(W_EXPECTED_RPAREN_AFTER_CALL)) {
err_args:
      ast_decref(args);
      goto err_current;
     }
    }
   } else {
    /* No argument list. */
    args = ast_setddi(ast_constexpr(Dee_EmptyTuple),&symbol_name_loc);
    if unlikely(!args) goto err_current;
   }
   merge = ast_setddi(ast_operator2(OPERATOR_CALL,AST_OPERATOR_FNORMAL,
                                    current,args),
                     &symbol_name_loc);
   ast_decref(args);
   ast_decref(current);
   if unlikely(!merge) goto err;
   current = merge;
   /* Now generate a branch to store the expression in the branch. */
   args = ast_setddi(ast_sym(var_symbol),&symbol_name_loc);
   if unlikely(!args) goto err_current;
   merge = ast_setddi(ast_action2(AST_FACTION_STORE,
                                  args,current),
                     &symbol_name_loc);
   ast_decref(args);
   ast_decref(current);
   if unlikely(!merge) goto err;
   current = merge;
  } else {
   if (current->a_type == AST_SYM &&
      (mode & AST_COMMA_ALLOWTYPEDECL)) {
    struct symbol *var_symbol = current->a_sym;
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
    if (tok == ':') {
     struct decl_ast decl;
     if unlikely(yield() < 0) goto err_current;
     if unlikely(decl_ast_parse(&decl)) goto err_current;
     if (var_symbol->s_decltype.da_type != DAST_NONE &&
        !decl_ast_equal(&var_symbol->s_decltype,&decl)) {
      decl_ast_fini(&decl);
      if (WARN(W_SYMBOL_TYPE_DECLARATION_CHANGED,var_symbol))
          goto err_current;
     } else {
      decl_ast_move(&var_symbol->s_decltype,&decl);
     }
    }
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
    if (var_symbol->s_type == SYMBOL_TYPE_GLOBAL &&
       !var_symbol->s_global.g_doc) {
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
     var_symbol->s_global.g_doc = (DREF DeeStringObject *)ast_tags_doc(&var_symbol->s_decltype);
#else /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
     var_symbol->s_global.g_doc = (DREF DeeStringObject *)ast_tags_doc();
#endif /* !CONFIG_HAVE_DECLARATION_DOCUMENTATION */
     if unlikely(!var_symbol->s_global.g_doc)
        goto err_current;
    }
   }
  }
 }

 if (tok == ',' && !(mode&AST_COMMA_PARSESINGLE)) {
  if (mode & AST_COMMA_STRICTCOMMA) {
   /* Peek the next token to check if it might be an expression. */
   char *next = peek_next_token(NULL);
   if unlikely(!next) goto err;
   if (!maybe_expression_begin_c(*next))
        goto done_expression;
  }
  /* Append to the current comma-sequence. */
  error = astlist_append(&expr_comma,current);
  if unlikely(error) goto err_current;
  ast_decref(current);
  /* Yield the ',' token. */
continue_at_comma:
  if unlikely(yield() < 0) goto err;
  if (!maybe_expression_begin()) {
   /* Special case: `x = (10,)'
    * Same as `x = pack(10)', in that a single-element tuple is created. */
   /* Flush any remaining entries from the comma-list. */
   error = astlist_appendall(&expr_batch,&expr_comma);
   if unlikely(error) goto err;
   Dee_Free(expr_comma.ast_v);
   /* Pack the branch together to form a multi-branch AST. */
   astlist_trunc(&expr_batch);
   current = ast_multiple(flags,expr_batch.ast_c,expr_batch.ast_v);
   if unlikely(!current) goto err_nocomma;
   /* Free an remaining buffers. */
   /*Dee_Free(expr_batch.ast_v);*/ /* This one was inherited. */
   /* WARNING: At this point, both `expr_batch' and `expr_comma' are
    *          in an undefined state, but don't hold any real data. */
   goto done_expression_nomerge;
  }
  goto next_expr;
 }
 if (tok == '=') {
  DREF struct ast *store_source;
  /* This is where the magic happens and where we
   * assign to expression in the active comma-list. */
  loc_here(&loc);
  if unlikely(yield() < 0) goto err_current;
  /* TODO: Add support for applying annotations here! */
  store_source = ast_parse_comma(AST_COMMA_PARSESINGLE |
                                (mode & AST_COMMA_STRICTCOMMA),
                                 AST_FMULTIPLE_KEEPLAST,
                                 NULL);
  if unlikely(!store_source) goto err_current;

  need_semi = true;
  /* Now everything depends on whether or not what
   * we've just parsed is an expand-expression:
   * >> a,b,c = get_value()...; // >> (((a,b,c) = get_value())...);
   * >> a,b,c = get_value();    // >> (a,b,(c = get_value())); */
  if (store_source->a_type == AST_EXPAND) {
   DREF struct ast *store_target,*store_branch;
   /* Append the last expression (in the example above, that is `c') */
   error = astlist_append(&expr_comma,current);
   ast_decref(current);
   if unlikely(error) {
err_store_source:
    ast_decref(store_source);
    goto err;
   }
   astlist_trunc(&expr_comma);
   store_target = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE,
                                          expr_comma.ast_c,
                                          expr_comma.ast_v),
                            &loc);
   if unlikely(!store_target) goto err_store_source;
   /* Steal all of these. */
   expr_comma.ast_a = 0;
   expr_comma.ast_c = 0;
   expr_comma.ast_v = NULL;
   /* Now store the expand's underlying expression within this tuple. */
   store_branch = ast_setddi(ast_action2(AST_FACTION_STORE,
                                         store_target,
                                         store_source->a_expand),
                            &loc);
   ast_decref(store_target);
   ast_decref(store_source);
   if unlikely(!store_branch) goto err;
   /* Now wrap the store-branch in another expand expression. */
   current = ast_setddi(ast_expand(store_branch),&loc);
   ast_decref(store_branch);
   if unlikely(!current) goto err;
  } else {
   DREF struct ast *store_branch;
   /* Second case: assign `store_source' to `current' after
    *              flushing everything from the comma-list. */
   error = astlist_appendall(&expr_batch,&expr_comma);
   if unlikely(error) { ast_decref(store_source); goto err_current; }
   /* With the comma-list now empty, generate the
    * store as described in the previous comment. */
   store_branch = ast_setddi(ast_action2(AST_FACTION_STORE,
                                         current,
                                         store_source),
                            &loc);
   ast_decref(store_source);
   ast_decref(current);
   if unlikely(!store_branch) goto err;
   current = store_branch;
  }

  /* Check for further comma or store expressions. */
  if (tok == ',' && !(mode&AST_COMMA_PARSESINGLE)) {
   if (!(mode&AST_COMMA_STRICTCOMMA)) {
do_append_gen_to_batch:
    /* Append the generated expression to the batch. */
    error = astlist_append(&expr_batch,current);
    if unlikely(error) goto err_current;
    ast_decref(current);
    goto continue_at_comma;
   } else {
    char *next_token;
    next_token = peek_next_token(NULL);
    if unlikely(!next_token) goto err_current;
    if (maybe_expression_begin_c(*next_token))
        goto do_append_gen_to_batch;
   }
  }
 }
done_expression:

 /* Merge the current expression with the batch and comma lists. */
 if (expr_batch.ast_c || expr_comma.ast_c) {
  /* Flush any remaining entries from the comma-list. */
  error = astlist_appendall(&expr_batch,&expr_comma);
  if unlikely(error) goto err_current;
  /* Append the remaining expression to the batch. */
  error = astlist_append(&expr_batch,current);
  if unlikely(error) goto err_current;
  ast_decref(current);
  /* Pack the branch together to form a multi-branch AST. */
  astlist_trunc(&expr_batch);
  current = ast_multiple(flags,expr_batch.ast_c,expr_batch.ast_v);
  if unlikely(!current) goto err;

  /* Free an remaining buffers. */
  /*Dee_Free(expr_batch.ast_v);*/ /* This one was inherited. */
  Dee_Free(expr_comma.ast_v);
  /* WARNING: At this point, both `expr_batch' and `expr_comma' are
   *          in an undefined state, but don't hold any real data. */
 } else {
  ASSERT(!expr_batch.ast_v);
  ASSERT(!expr_comma.ast_v);
  if (mode&AST_COMMA_FORCEMULTIPLE) {
   /* If the caller wants to force us to package
    * everything in a multi-branch, grant that wish. */
   DREF struct ast **astv,*result;
   astv = (DREF struct ast **)Dee_Malloc(1*sizeof(DREF struct ast *));
   if unlikely(!astv) goto err_current;
   astv[0] = current;
   result = ast_multiple(flags,1,astv);
   if unlikely(!result) { Dee_Free(astv); goto err_current; }
   current = result;
  }
 }
done_expression_nomerge:
 if (pout_mode) {
  if (need_semi)
     *pout_mode |= AST_COMMA_OUT_FNEEDSEMI;
 } else if (need_semi && (mode & AST_COMMA_PARSESEMI)) {
  /* Consume a `;' token as part of the expression. */
  if likely(tok == ';' || tok == '\n') {
   do {
    if (yieldnbif(mode & AST_COMMA_ALLOWNONBLOCK) < 0)
        goto err_clear_current_only;
   } while (tok == '\n');
  } else {
   if unlikely(WARN(W_EXPECTED_SEMICOLLON_AFTER_EXPRESSION)) {
err_clear_current_only:
    ast_decref(current);
    current = NULL;
   }
  }
 }
 return current;
done_expression_nocurrent:
 /* Merge the current expression with the batch and comma lists. */
 if (expr_batch.ast_c || expr_comma.ast_c) {
  /* Flush any remaining entries from the comma-list. */
  error = astlist_appendall(&expr_batch,&expr_comma);
  if unlikely(error) goto err;
  /* Pack the branch together to form a multi-branch AST. */
  astlist_trunc(&expr_batch);
  current = ast_multiple(flags,expr_batch.ast_c,expr_batch.ast_v);
  if unlikely(!current) goto err;
  /* Free an remaining buffers. */
  /*Dee_Free(expr_batch.ast_v);*/ /* This one was inherited. */
  Dee_Free(expr_comma.ast_v);
  /* WARNING: At this point, both `expr_batch' and `expr_comma' are
   *          in an undefined state, but don't hold any real data. */
 } else {
  ASSERT(!expr_batch.ast_v);
  ASSERT(!expr_comma.ast_v);
  /* If the caller wants to force us to package
   * everything in a multi-branch, grant that wish. */
  current = ast_multiple(flags,0,NULL);
  if unlikely(!current) goto err;
 }
 goto done_expression_nomerge;
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
err_decl:
 decl_ast_fini(&decl);
 goto err;
#endif
err_current:
 ast_decref(current);
#ifndef CONFIG_HAVE_DECLARATION_DOCUMENTATION
err_decl:
#endif
err:
 astlist_fini(&expr_comma);
err_nocomma:
 astlist_fini(&expr_batch);
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_COMMA_C */
