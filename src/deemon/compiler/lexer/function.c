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
#ifndef GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C
#define GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C 1

#include <deemon/api.h>
#include <deemon/none.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/error.h>

DECL_BEGIN

PRIVATE struct symbol *DCALL parse_argument_name(void) {
 struct symbol *result;
 struct TPPKeyword *argument_name;
 if (tok == KWD_none) {
  /* Special case: Allow `none' to be used for placeholder/pending arguments. */
create_anon_argument:
  /* Create a new symbol for the argument. */
  result = new_unnamed_symbol();
 } else {
  argument_name = token.t_kwd;
  if (has_local_symbol(argument_name)) {
   if (WARN(W_ARGUMENT_NAME_ALREADY_IN_USE))
       goto err;
   goto create_anon_argument;
  }
  /* Check if the argument name is a reserved identifier. */
  if (is_reserved_symbol_name(argument_name)) {
   if (WARN(W_RESERVED_ARGUMENT_NAME,argument_name))
       goto err;
  }
  /* Create a new symbol for the argument. */
  result = new_local_symbol(argument_name);
 }
 return result;
err:
 return NULL;
}


INTERN int DCALL parse_arglist(void) {
 bool in_default_args = false;
 bool in_optional_args = false;
 struct symbol **new_symv; uint16_t defaulta,arga;
 DREF DeeObject **new_defaultv; struct symbol *arg;
 DREF DeeScopeObject *old_current_scope;
 ASSERT(!current_basescope->bs_argc_min);
 ASSERT(!current_basescope->bs_argc_max);
 ASSERT(!current_basescope->bs_argc_opt);
 ASSERT(!current_basescope->bs_argc);
 ASSERT(!current_basescope->bs_argv);
 ASSERT(!current_basescope->bs_varargs);
 old_current_scope = current_scope;
 current_scope = (DREF DeeScopeObject *)current_basescope;
 Dee_Incref(current_scope);
 defaulta = arga = 0;
 while (tok > 0 && tok != ')') {
  DREF DeeObject *defl_value;
  bool arg_is_optional = false;
  /* Special case: The old deemon allowed an ignored `local' before argument names.
   *               Since this doesn't harm anything, we allow doing so
   *               too (despite no longer documenting doing so). */
  if (tok == KWD_local && yield() < 0) goto err;
  if (tok == '?') {
   /* Found an optional argument! */
   arg_is_optional = true;
   if (yield() < 0) goto err;
  }

  ASSERT(current_basescope->bs_argc <= arga);
  if (current_basescope->bs_argc == arga) {
   uint16_t new_arga = arga * 2;
   if (!new_arga) new_arga = 2;
do_realloc_symv:
   new_symv = (struct symbol **)Dee_TryRealloc(current_basescope->bs_argv,
                                               new_arga*sizeof(struct symbol *));
   if unlikely(!new_symv) {
    if (new_arga != current_basescope->bs_argc+1) {
     new_arga = current_basescope->bs_argc+1;
     goto do_realloc_symv;
    }
    if (Dee_CollectMemory(new_arga*sizeof(struct symbol *)))
        goto do_realloc_symv;
   }
   current_basescope->bs_argv = new_symv;
   arga = new_arga;
  }
  if (tok == TOK_DOTS) {
   /* Special case: unnamed varargs. */
   arg = new_unnamed_symbol();
   if unlikely(!arg) goto err;
   /* Initialize the symbol as an argument. */
   arg->s_type  = SYMBOL_TYPE_ARG;
   arg->s_flag  = SYMBOL_FALLOC;
   arg->s_symid = current_basescope->bs_argc;
   /* Add the symbol to the argument symbol vector. */
   current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
   goto done_dots;
  }
  if unlikely(!TPP_ISKEYWORD(tok)) {
   if (WARN(W_EXPECTED_KEYWORD_FOR_ARGUMENT_NAME))
       goto err;
   break;
  }
  /* Parse the argument name. */
  arg = parse_argument_name();
  if unlikely(!arg) goto err;
  /* Initialize the symbol as an argument. */
  arg->s_type  = SYMBOL_TYPE_ARG;
  arg->s_flag  = SYMBOL_FALLOC;
  arg->s_symid = current_basescope->bs_argc;
  /* Add the symbol to the argument symbol vector. */
  current_basescope->bs_argv[current_basescope->bs_argc++] = arg;
  if unlikely(yield() < 0) goto err;
  if (tok == TOK_DOTS) {
   /* Varargs */
done_dots:
   if unlikely(yield() < 0) goto err;
   if (tok == ',' && unlikely(yield() < 0)) goto err;
   if unlikely(tok != ')' && WARN(W_EXPECTED_RPAREN_AFTER_VARARGS)) goto err;
   /* Set the varargs flag in the new base scope. */
   current_basescope->bs_flags  |= CODE_FVARARGS;
   current_basescope->bs_varargs = arg;
   goto done;
  }
  if (tok == '?' && !arg_is_optional) {
   arg_is_optional = true;
   if unlikely(yield() < 0) goto err;
  }
  ++current_basescope->bs_argc_max;
  if (tok == '=') {
   uint16_t defaultc;
   DREF DeeAstObject *initializer;
   if (in_optional_args) {
    if (WARN(W_DEFAULT_ARGUMENT_AFTER_NONDEFAULT_OPTIONAL))
        goto err;
   } else {
    /* Default initializer. */
    if (!in_default_args) {
     /* First default value. */
     current_basescope->bs_argc_min = current_basescope->bs_argc_max-1;
     in_default_args                = true;
    }
   }
   /* Yield the `=' token. */
   if unlikely(yield() < 0) goto err;
   /* Parse the initializer. */
   initializer = ast_parse_expression(LOOKUP_SYM_NORMAL);
   if unlikely(!initializer) goto err;
   if (in_optional_args) {
    Dee_Decref(initializer);
    goto got_optional_arg;
   }
   /* Do optimizations on the AST to allow for a bit for complex
    * expression that can still be resolved at compile-time.
    * (`ast_optimize()' is where constant propagation happens) */
   if (ast_optimize(initializer,true)) {
err_initializer:
    Dee_Decref(initializer);
    goto err;
   }
   /* Extract the value of a constant expression,
    * which is required for a default initializer. */
   if unlikely(initializer->ast_type != AST_CONSTEXPR) {
    if (WARN(W_EXPECTED_CONSTANT_EXPRESSION_FOR_ARGUMENT_DEFAULT))
        goto err_initializer;
    defl_value = Dee_None;
   } else {
    defl_value = initializer->ast_constexpr;
   }
   Dee_Incref(defl_value);
   Dee_Decref(initializer);
set_default_value:
   defaultc = (uint16_t)(current_basescope->bs_argc_max-
                         current_basescope->bs_argc_min);
   if (defaultc >= defaulta) {
    uint16_t new_defaulta = defaulta*2;
    if (!new_defaulta) new_defaulta = 1;
do_realloc_defaultv:
    new_defaultv = (DREF DeeObject **)Dee_TryRealloc(current_basescope->bs_default,
                                                     new_defaulta*sizeof(DREF DeeObject *));
    if unlikely(!new_defaultv) {
     if (new_defaulta != defaultc) { new_defaulta = defaultc; goto do_realloc_defaultv; }
     if (Dee_CollectMemory(new_defaulta*sizeof(DREF DeeObject *))) goto do_realloc_defaultv;
     Dee_Decref(defl_value);
     goto err;
    }
    current_basescope->bs_default = new_defaultv;
    defaulta = new_defaulta;
   }
   current_basescope->bs_default[defaultc-1] = defl_value; /* Inherit reference. */
  } else if (arg_is_optional) {
got_optional_arg:
   --current_basescope->bs_argc_max;
   ++current_basescope->bs_argc_opt;
   current_basescope->bs_flags |= CODE_FVARARGS;
   in_optional_args = true;
  } else if (in_optional_args) {
   if (WARN(W_MISSING_ARGUMENT_OPTIONAL_PREFIX))
       goto err;
   goto got_optional_arg;
  } else if (in_default_args) {
   /* Missing default initializer. */
   if (WARN(W_MISSING_ARGUMENT_DEFAULT_INITIAIZER))
       goto err;
   defl_value = Dee_None;
   Dee_Incref(Dee_None);
   goto set_default_value;
  }
  /* parse more arguments. */
  if (tok != ',') break;
  if unlikely(yield() < 0) goto err;
 }
done:
 ASSERT(current_basescope->bs_argc_min <=
        current_basescope->bs_argc_max);
 ASSERT(arga >= current_basescope->bs_argc);
 /* Truncate the argument vector. */
 if (arga != current_basescope->bs_argc) {
  new_symv = (struct symbol **)Dee_TryRealloc(current_basescope->bs_argv,
                                              current_basescope->bs_argc*
                                              sizeof(struct symbol *));
  if likely(new_symv) current_basescope->bs_argv = new_symv;
 }
 if (!in_default_args) {
  /* Without any default arguments, the function
   * takes an absolute number of parameters. */
  current_basescope->bs_argc_min = current_basescope->bs_argc_max;
 } else {
  /* Truncate the default argument vector. */
  uint16_t req_defaulta = (current_basescope->bs_argc_max-
                           current_basescope->bs_argc_min);
  if (defaulta != req_defaulta) {
   new_defaultv = (DREF DeeObject **)Dee_TryRealloc(current_basescope->bs_default,
                                                    req_defaulta*sizeof(DREF DeeObject *));
   if likely(new_defaultv) current_basescope->bs_default = new_defaultv;
  }
 }
 ASSERT(current_basescope->bs_argc ==
        current_basescope->bs_argc_max +
        current_basescope->bs_argc_opt +
       (current_basescope->bs_varargs ? 1 : 0));
 Dee_Decref(current_scope);
 current_scope = old_current_scope;
 return 0;
err:
 /* This needs to be done to ensure a consistent scope on exit. */
 if (!in_default_args)
      current_basescope->bs_argc_min = current_basescope->bs_argc_max;
 Dee_Decref(current_scope);
 current_scope = old_current_scope;
 return -1;
}

INTERN DREF DeeAstObject *DCALL
ast_parse_function(struct TPPKeyword *name, bool *pneed_semi,
                   bool allow_missing_params) {
 DREF DeeAstObject *result;
 if unlikely(basescope_push()) return NULL;
 current_basescope->bs_flags |= current_tags.at_code_flags;
 current_basescope->bs_class  = current_tags.at_class;
 current_basescope->bs_super  = current_tags.at_super;
 result = ast_parse_function_noscope(name,pneed_semi,allow_missing_params);
 basescope_pop();
 return result;
}
INTERN DREF DeeAstObject *DCALL
ast_parse_function_noscope(struct TPPKeyword *name,
                           bool *pneed_semi,
                           bool allow_missing_params) {
 uint32_t old_flags;
 DREF DeeAstObject *result,*code;
 /* Add information from tags. */
 if (name) {
  struct symbol *funcself_symbol;
  /* Save the function name in the base scope. */
  current_basescope->bs_name = name;
  /* Create a new symbol to allow for function-self-referencing. */
  funcself_symbol = new_local_symbol(name);
  if unlikely(!funcself_symbol) goto err;
  SYMBOL_TYPE(funcself_symbol) = SYMBOL_TYPE_MYFUNC;
 }
 if (tok == '(') {
  /* Argument list. */
  old_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(yield() < 0) goto err_flags;
  if unlikely(parse_arglist()) goto err_flags;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_ARGLIST))
     goto err;
 } else if (!allow_missing_params) {
  if (WARN(W_DEPRECATED_NO_PARAMETER_LIST))
      goto err;
 }
 if (tok == TOK_ARROW) {
  struct ast_loc arrow_loc;
  loc_here(&arrow_loc);
  if unlikely(yield() < 0) goto err;
  /* Expression function. */
  code = ast_parse_expression(LOOKUP_SYM_NORMAL);
  if unlikely(!code) goto err;
  result = code->ast_type == AST_EXPAND
         ? (current_basescope->bs_flags |= CODE_FYIELDING,ast_yield(code))
         : (ast_return(code));
  Dee_Decref(code);
  code = ast_setddi(result,&arrow_loc);
  if (pneed_semi) *pneed_semi = true;
 } else {
  old_flags = TPPLexer_Current->l_flags;
  if (parser_flags & PARSE_FLFSTMT)
      TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '{') ? (yield() < 0) :
              WARN(W_EXPECTED_LBRACE_AFTER_FUNCTION))
     goto err_flags;
  code = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST,'}');
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '}') ? (yield() < 0) :
              WARN(W_EXPECTED_RBRACE_AFTER_FUNCTION))
     goto err_xcode;
  if (pneed_semi) *pneed_semi = false;
 }
 if unlikely(!code) goto err;
 result = ast_function(code,current_basescope);
 Dee_Decref(code);
 if unlikely(!result) goto err;
 /* Hack: The function AST itself must be located in the caller's scope. */
 Dee_Decref(result->ast_scope);
 ASSERT(current_basescope);
 ASSERT(current_basescope->bs_scope.s_prev);
 result->ast_scope = current_basescope->bs_scope.s_prev;
 Dee_Incref(result->ast_scope);
 return result;
err_flags:
 TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err;
err_xcode:
 Dee_XDecref(code);
err:
 return NULL;
}
INTERN DREF DeeAstObject *DCALL
ast_parse_function_noscope_noargs(bool *pneed_semi) {
 uint32_t old_flags;
 DREF DeeAstObject *result,*code;
 if (tok == TOK_ARROW) {
  struct ast_loc arrow_loc;
  loc_here(&arrow_loc);
  if unlikely(yield() < 0) goto err;
  /* Expression function. */
  code = ast_parse_expression(LOOKUP_SYM_NORMAL);
  if unlikely(!code) goto err;
  result = code->ast_type == AST_EXPAND
         ? (current_basescope->bs_flags |= CODE_FYIELDING,ast_yield(code))
         : (ast_return(code));
  Dee_Decref(code);
  code = ast_setddi(result,&arrow_loc);
  if (pneed_semi) *pneed_semi = true;
 } else {
  old_flags = TPPLexer_Current->l_flags;
  if (parser_flags & PARSE_FLFSTMT)
      TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '{') ? (yield() < 0) :
              WARN(W_EXPECTED_LBRACE_AFTER_FUNCTION))
     goto err_flags;
  code = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST,'}');
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '}') ? (yield() < 0) :
              WARN(W_EXPECTED_RBRACE_AFTER_FUNCTION))
     goto err_xcode;
  if (pneed_semi) *pneed_semi = false;
 }
 if unlikely(!code) goto err;
 result = ast_function(code,current_basescope);
 Dee_Decref(code);
 if unlikely(!result) goto err;
 /* Hack: The function AST itself must be located in the caller's scope. */
 Dee_Decref(result->ast_scope);
 ASSERT(current_basescope);
 ASSERT(current_basescope->bs_scope.s_prev);
 result->ast_scope = current_basescope->bs_scope.s_prev;
 Dee_Incref(result->ast_scope);
 return result;
err_flags:
 TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err;
err_xcode:
 Dee_XDecref(code);
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_FUNCTION_C */
