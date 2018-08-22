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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_IAST_C
#define GUARD_DEEMON_COMPILER_INTERFACE_IAST_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/error.h>
#include <deemon/string.h>
#include <deemon/int.h>
#include <deemon/format.h>
#include <deemon/tuple.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/interface.h>
#include <deemon/util/cache.h>
#include <deemon/module.h>

#include "../../runtime/strings.h"

DECL_BEGIN

DECLARE_OBJECT_CACHE(compiler_item,DeeCompilerItemObject)
typedef DeeCompilerAstObject Ast;

INTERN ATTR_COLD int DCALL
err_invalid_ast_basescope(DeeCompilerAstObject *__restrict obj,
                          struct base_scope_object *__restrict base_scope) {
 (void)obj;
 (void)base_scope;
 return DeeError_Throwf(&DeeError_ValueError,
                        "base-scope of ast differs from the effective base-scope");
}
INTERN ATTR_COLD int DCALL
err_invalid_ast_compiler(DeeCompilerAstObject *__restrict obj) {
 (void)obj;
 return DeeError_Throwf(&DeeError_ValueError,
                        "Ast is associated with a different compiler");
}
INTERN ATTR_COLD int DCALL
err_invalid_scope_compiler(DeeCompilerScopeObject *__restrict obj) {
 (void)obj;
 return DeeError_Throwf(&DeeError_ValueError,
                        "Scope is associated with a different compiler");
}

INTERN ATTR_COLD int DCALL
err_invalid_symbol_compiler(DeeCompilerSymbolObject *__restrict obj) {
 (void)obj;
 return DeeError_Throwf(&DeeError_ValueError,
                        "Symbol is associated with a different compiler");
}

INTERN ATTR_COLD int DCALL
err_symbol_not_reachable(struct scope_object *__restrict scope,
                         struct symbol *__restrict sym) {
 (void)scope;
 return DeeError_Throwf(&DeeError_ReferenceError,
                        "Symbol %$q is not reachable from the specified scope",
                        sym->s_name->k_size,sym->s_name->k_name);
}

INTERN bool DCALL
scope_reaches_symbol(DeeScopeObject *__restrict scope,
                     struct symbol *__restrict sym) {
 DeeScopeObject *dst = sym->s_scope;
 for (; scope; scope = scope->s_prev) {
  if (scope == dst)
      return true;
 }
 return false;
}

INTERN ATTR_COLD int DCALL err_different_base_scope(void) {
 return DeeError_Throwf(&DeeError_ReferenceError,
                        "Cannot assign a new scope that isn't apart "
                        "of the same base-scope as the old one");
}
INTERN ATTR_COLD int DCALL err_different_root_scope(void) {
 return DeeError_Throwf(&DeeError_ReferenceError,
                        "Cannot assign a new scope that isn't apart "
                        "of the same root-scope as the old one");
}


PRIVATE DREF DeeObject *DCALL ast_getscope(Ast *__restrict self) {
 DREF DeeObject *result;
 COMPILER_BEGIN(self->ci_compiler);
 result = DeeCompiler_GetScope(self->ci_value->a_scope);
 COMPILER_END();
 return result;
}
PRIVATE int DCALL
ast_setscope(Ast *__restrict self,
             DeeCompilerScopeObject *__restrict value) {
 struct ast *branch = self->ci_value;
 if (DeeObject_AssertType((DeeObject *)value,&DeeCompilerScope_Type))
     return -1;
 if (value->ci_compiler != self->ci_compiler)
     return err_invalid_scope_compiler(value);
 COMPILER_BEGIN(self->ci_compiler);
 if unlikely(value->ci_value->s_base != branch->a_scope->s_base) {
  err_different_base_scope();
  goto err;
 }
 /* Make sure that referenced symbols is
  * still reachable from the new scope. */
 switch (branch->a_type) {

 case AST_SYM:
 case AST_UNBIND:
 case AST_BOUND:
  if (scope_reaches_symbol(value->ci_value,branch->a_sym))
      break;
err_unreachable_symbols:
  DeeError_Throwf(&DeeError_ReferenceError,
                  "Cannot assign new scope to branch containing "
                  "symbols that would no longer be reachable");
  goto err;

 case AST_CLASS:
  if (branch->a_class.c_classsym &&
     !scope_reaches_symbol(value->ci_value,branch->a_class.c_classsym))
      goto err_unreachable_symbols;
  if (branch->a_class.c_supersym &&
     !scope_reaches_symbol(value->ci_value,branch->a_class.c_supersym))
      goto err_unreachable_symbols;
  break;

 default: break;
 }
 /* Assign the new scope. */
 Dee_Incref(value->ci_value);
 Dee_Decref(branch->a_scope);
 branch->a_scope = value->ci_value;
 COMPILER_END();
 return 0;
err:
 COMPILER_END();
 return -1;
}


PRIVATE DeeObject *ast_names[] = {
    /* [AST_CONSTEXPR]     = */&str_constexpr,
    /* [AST_SYM]           = */&str_sym,
    /* [AST_UNBIND]        = */&str_unbind,
    /* [AST_BOUND]         = */&str_bound,
    /* [AST_MULTIPLE]      = */&str_multiple,
    /* [AST_RETURN]        = */&str_return,
    /* [AST_YIELD]         = */&str_yield,
    /* [AST_THROW]         = */&str_throw,
    /* [AST_TRY]           = */&str_try,
    /* [AST_LOOP]          = */&str_loop,
    /* [AST_LOOPCTL]       = */&str_loopctl,
    /* [AST_CONDITIONAL]   = */&str_conditional,
    /* [AST_BOOL]          = */&str_bool,
    /* [AST_EXPAND]        = */&str_expand,
    /* [AST_FUNCTION]      = */&str_function,
    /* [AST_OPERATOR_FUNC] = */&str_operatorfunc,
    /* [AST_OPERATOR]      = */&str_operator,
    /* [AST_ACTION]        = */&str_action,
    /* [AST_CLASS]         = */&str_class,
    /* [AST_LABEL]         = */&str_label,
    /* [AST_GOTO]          = */&str_goto,
    /* [AST_SWITCH]        = */&str_switch,
    /* [AST_ASSEMBLY]      = */&str_assembly,
};


PRIVATE DREF DeeObject *DCALL
ast_gettypeid(Ast *__restrict self) {
 uint16_t result;
 do result = ATOMIC_READ(self->ci_value->a_type);
 while unlikely(result >= COMPILER_LENOF(ast_names));
 return DeeInt_NewU16(result);
}

PRIVATE DREF DeeObject *DCALL
ast_gettype(Ast *__restrict self) {
 uint16_t result;
 do result = ATOMIC_READ(self->ci_value->a_type);
 while unlikely(result >= COMPILER_LENOF(ast_names));
 return_reference(ast_names[result]);
}


#define DO(x)         do if unlikely((x) < 0) goto err; __WHILE0
#define PRINT(str)    DO((*printer)(arg,str,COMPILER_STRLEN(str)))
#define print(p,s)    DO((*printer)(arg,p,s))
#define printf(...)   DO(Dee_FormatPrintf(printer,arg,__VA_ARGS__))

PRIVATE int DCALL
print_enter_scope(DeeScopeObject *__restrict caller_scope,
                  DeeScopeObject *__restrict child_scope,
                  dformatprinter printer, void *arg,
                  bool is_expression, size_t *__restrict pindent,
                  bool *__restrict pis_scope) {
 if (child_scope == caller_scope)
     return 0;
 if (is_expression) PRINT("(");
 PRINT("{\n");
 ++*pindent;
 DO(Dee_FormatRepeat(printer,arg,'\t',*pindent));
 *pis_scope = true;
 for (; child_scope && child_scope != caller_scope; child_scope = child_scope->s_prev) {
  size_t i; struct symbol *sym;
  if (!child_scope->s_mapc) continue;
  for (i = 0; i < child_scope->s_mapa; ++i) {
   sym = child_scope->s_map[i];
   for (; sym; sym = sym->s_next) {
    if (sym->s_type != SYMBOL_TYPE_GLOBAL &&
        sym->s_type != SYMBOL_TYPE_EXTERN &&
        sym->s_type != SYMBOL_TYPE_MODULE &&
        sym->s_type != SYMBOL_TYPE_MYMOD &&
        sym->s_type != SYMBOL_TYPE_LOCAL &&
        sym->s_type != SYMBOL_TYPE_STACK &&
        sym->s_type != SYMBOL_TYPE_STATIC)
        continue;
    switch (sym->s_type) {
    case SYMBOL_TYPE_EXTERN:
     printf("import %$s = %k from %k",
            sym->s_name->k_size,
            sym->s_name->k_name,
            sym->s_extern.e_symbol->ss_name,
            sym->s_extern.e_module->mo_name);
     break;
    case SYMBOL_TYPE_MODULE:
     printf("import %$s = %k",
            sym->s_name->k_size,
            sym->s_name->k_name,
            sym->s_module->mo_name);
     break;
    case SYMBOL_TYPE_MYMOD:
     printf("import %$s = .",
            sym->s_name->k_size,
            sym->s_name->k_name);
     break;
    case SYMBOL_TYPE_GLOBAL:
     PRINT("global ");
     goto print_symbol_name;
    case SYMBOL_TYPE_LOCAL:
     PRINT("local ");
     goto print_symbol_name;
    case SYMBOL_TYPE_STACK:
     PRINT("__stack local ");
     goto print_symbol_name;
    case SYMBOL_TYPE_STATIC:
     PRINT("static local ");
print_symbol_name:
     print(sym->s_name->k_name,sym->s_name->k_size);
     break;
    default: break;
    }
    PRINT(";\n");
    DO(Dee_FormatRepeat(printer,arg,'\t',*pindent));
   }
  }
 }
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
print_leave_scope(dformatprinter printer, void *arg,
                  bool is_expression, bool need_semicolon,
                  size_t indent, bool is_scope) {
 if (is_scope) {
  if (need_semicolon) {
   PRINT(";\n");
  } else {
   PRINT("\n");
  }
  DO(Dee_FormatRepeat(printer,arg,'\t',indent - 1));
  PRINT("}");
  if (is_expression) PRINT(")");
 }
 return 0;
err:
 return -1;
}

#define ENTER_SCOPE(caller_scope,child_scope,is_expression) \
do{ bool is_scope = false; \
    DO(print_enter_scope(caller_scope,child_scope,printer,arg,is_expression,&indent,&is_scope))
#define LEAVE_SCOPE(is_expression,need_semicolon) \
    DO(print_leave_scope(printer,arg,is_expression,need_semicolon,indent,is_scope)); \
}__WHILE0

INTDEF bool DCALL
DeeString_IsSymbol(DeeObject *__restrict self,
                   size_t start_index,
                   size_t end_index);
PRIVATE int DCALL
print_ast_code(struct ast *__restrict self,
               dformatprinter printer, void *arg, bool is_expression,
               DeeScopeObject *__restrict caller_scope, size_t indent);

PRIVATE int DCALL
print_asm_operator(struct asm_operand *__restrict operand,
                   dformatprinter printer, void *arg,
                   DeeScopeObject *__restrict caller_scope,
                   size_t indent) {
#ifndef CONFIG_LANGUAGE_NO_ASM
 if (operand->ao_name)
     printf("[%$s] ",operand->ao_name->k_size,operand->ao_name->k_name);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 ASSERT(operand->ao_type);
 printf("%$q (",
        operand->ao_type->s_size,
        operand->ao_type->s_text);
 DO(print_ast_code(operand->ao_expr,printer,arg,true,caller_scope,indent));
 PRINT(")");
 return 0;
err:
 return -1;
}
PRIVATE int DCALL
print_asm_label_operator(struct asm_operand *__restrict operand,
                         dformatprinter printer, void *arg) {
#ifndef CONFIG_LANGUAGE_NO_ASM
 if (operand->ao_name)
     printf("[%$s] ",operand->ao_name->k_size,operand->ao_name->k_name);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 ASSERT(!operand->ao_type);
 print(operand->ao_label->tl_name->k_name,
       operand->ao_label->tl_name->k_size);
 return 0;
err:
 return -1;
}


PRIVATE int DCALL
print_ast_code(struct ast *__restrict self,
               dformatprinter printer, void *arg, bool is_expression,
               DeeScopeObject *__restrict caller_scope, size_t indent) {
 bool need_semicolon = true;
 ENTER_SCOPE(caller_scope,self->a_scope,is_expression);
 __IF0 {
force_scope:
  if (is_expression) PRINT("(");
  PRINT("{\n");
  ++indent;
  DO(Dee_FormatRepeat(printer,arg,'\t',indent));
  is_scope = true;
 }
 switch (self->a_type) {

 case AST_CONSTEXPR:
  DO(DeeObject_PrintRepr(self->a_constexpr,printer,arg));
  break;

 case AST_SYM:
  print(self->a_sym->s_name->k_name,
        self->a_sym->s_name->k_size);
  break;

 case AST_UNBIND:
  printf("del(%$s)",
         self->a_unbind->s_name->k_size,
         self->a_unbind->s_name->k_name);
  break;

 case AST_BOUND:
  printf("%$s is bound",
         self->a_bound->s_name->k_size,
         self->a_bound->s_name->k_name);
  break;

 {
  size_t i;
 case AST_MULTIPLE:
  switch (self->a_flag) {
  case AST_FMULTIPLE_TUPLE:
   if (!self->a_multiple.m_astc)
    PRINT("pack()");
   else {
    PRINT("(");
    for (i = 0; i < self->a_multiple.m_astc; ++i) {
     DO(print_ast_code(self->a_multiple.m_astv[i],printer,arg,true,self->a_scope,indent + 1));
     if (i < self->a_multiple.m_astc-1)
         PRINT(",");
    }
    if (self->a_multiple.m_astc == 1) {
     PRINT(",)");
    } else {
     PRINT(")");
    }
   }
   break;
  case AST_FMULTIPLE_LIST:
   PRINT("[");
   for (i = 0; i < self->a_multiple.m_astc; ++i) {
    DO(print_ast_code(self->a_multiple.m_astv[i],printer,arg,true,self->a_scope,indent + 1));
    if (i < self->a_multiple.m_astc-1)
        PRINT(",");
   }
   PRINT("]");
   break;
  case AST_FMULTIPLE_SET:
  case AST_FMULTIPLE_GENERIC:
   PRINT("{ ");
   for (i = 0; i < self->a_multiple.m_astc; ++i) {
    DO(print_ast_code(self->a_multiple.m_astv[i],printer,arg,true,self->a_scope,indent + 1));
    if (i < self->a_multiple.m_astc-1)
        PRINT(", ");
   }
   PRINT(" }");
   break;
  case AST_FMULTIPLE_DICT:
  case AST_FMULTIPLE_GENERIC_KEYS:
   PRINT("{ ");
   for (i = 0; i < self->a_multiple.m_astc / 2; ++i) {
    DO(print_ast_code(self->a_multiple.m_astv[i * 2 + 0],printer,arg,true,self->a_scope,indent + 1));
    PRINT(": ");
    DO(print_ast_code(self->a_multiple.m_astv[i * 2 + 1],printer,arg,true,self->a_scope,indent + 1));
    if (i < (self->a_multiple.m_astc/2)-1)
        PRINT(", ");
   }
   PRINT(" }");
   break;
  default:
   if (!is_scope && is_expression) goto force_scope;
   if (!self->a_multiple.m_astc) {
    PRINT("none");
   } else for (i = 0; i < self->a_multiple.m_astc; ++i) {
    DO(print_ast_code(self->a_multiple.m_astv[i],printer,arg,false,self->a_scope,indent));
    if (i < self->a_multiple.m_astc-1) {
     PRINT(";\n");
     DO(Dee_FormatRepeat(printer,arg,'\t',indent));
    }
   }
   break;
  }
 } break;
 case AST_RETURN:
  if (!is_scope && is_expression) goto force_scope;
  if (self->a_return) {
   PRINT("return ");
   DO(print_ast_code(self->a_return,printer,arg,true,self->a_scope,indent));
  } else {
   PRINT("return");
  }
  break;
 case AST_YIELD:
  if (!is_scope && is_expression) goto force_scope;
  PRINT("yield ");
  DO(print_ast_code(self->a_yield,printer,arg,true,self->a_scope,indent));
  break;
 case AST_THROW:
  if (!is_scope && is_expression) goto force_scope;
  if (self->a_throw) {
   PRINT("throw ");
   DO(print_ast_code(self->a_throw,printer,arg,true,self->a_scope,indent));
  } else {
   PRINT("throw");
  }
  break;

 {
  size_t i;
 case AST_TRY:
  if (!self->a_try.t_catchc) {
   DO(print_ast_code(self->a_try.t_guard,printer,arg,
                     is_expression && !is_scope,
                     self->a_scope,indent));
   break;
  }
  PRINT("try ");
  DO(print_ast_code(self->a_try.t_guard,printer,arg,
                    is_expression && !is_scope,
                    self->a_scope,indent));
  need_semicolon = false;
  for (i = 0; i < self->a_try.t_catchc; ++i) {
   struct catch_expr *handler;
   handler = &self->a_try.t_catchv[i];
   if (handler->ce_flags & EXCEPTION_HANDLER_FFINALLY) {
    PRINT(" finally ");
    if (handler->ce_mask) {
     PRINT("{\n");
     DO(Dee_FormatRepeat(printer,arg,'\t',indent + 1));
     DO(print_ast_code(handler->ce_mask,printer,arg,false,
                       self->a_scope,indent + 1));
     PRINT(";\n");
     DO(Dee_FormatRepeat(printer,arg,'\t',indent + 1));
     DO(print_ast_code(handler->ce_code,printer,arg,
                       is_expression && !is_scope,
                       self->a_scope,indent + 1));
     PRINT(";\n");
     DO(Dee_FormatRepeat(printer,arg,'\t',indent));
     PRINT("}");
    } else {
     DO(print_ast_code(handler->ce_code,printer,arg,
                       is_expression && !is_scope,
                       self->a_scope,indent));
    }
   } else {
    struct symbol *except_symbol = NULL; size_t i;
    DeeScopeObject *handler_scope = handler->ce_code->a_scope;
    if (handler_scope->s_mapc) {
     for (i = 0; i < handler_scope->s_mapa; ++i) {
      except_symbol = handler_scope->s_map[i];
      for (; except_symbol; except_symbol = except_symbol->s_next) {
       if (except_symbol->s_type == SYMBOL_TYPE_EXCEPT)
           goto got_except_symbol;
      }
     }
     except_symbol = NULL;
    }
got_except_symbol:
    if (handler->ce_flags & EXCEPTION_HANDLER_FINTERPT)
        PRINT("@interrupt ");
    PRINT(" catch (");
    if (handler->ce_mask) {
     DO(print_ast_code(handler->ce_mask,printer,arg,true,self->a_scope,indent));
     if (except_symbol) {
      PRINT(" ");
      print(except_symbol->s_name->k_name,
            except_symbol->s_name->k_size);
     }
    } else {
     print(except_symbol->s_name->k_name,
           except_symbol->s_name->k_size);
     PRINT("...");
    }
    PRINT(") ");
    DO(print_ast_code(handler->ce_code,printer,arg,
                      is_expression && !is_scope,
                      self->a_scope,indent));
   }
  }
 } break;

 case AST_LOOP:
  if (!is_scope && is_expression) goto force_scope;
  if (self->a_flag & AST_FLOOP_UNLIKELY)
      PRINT("@unlikely ");
  if (self->a_flag & AST_FLOOP_FOREACH) {
   PRINT("foreach (");
   need_semicolon = false;
   if (self->a_loop.l_elem) {
    ENTER_SCOPE(self->a_scope,self->a_loop.l_elem->a_scope,false);
    DO(print_ast_code(self->a_loop.l_elem,printer,arg,true,self->a_loop.l_elem->a_scope,indent));
    PRINT(": ");
    DO(print_ast_code(self->a_loop.l_iter,printer,arg,true,self->a_loop.l_elem->a_scope,indent));
    PRINT(") ");
    if (self->a_loop.l_loop) {
     DO(print_ast_code(self->a_loop.l_loop,printer,arg,false,self->a_loop.l_elem->a_scope,indent));
    } else {
     PRINT("none;");
    }
    LEAVE_SCOPE(false,false);
   } else {
    PRINT("none: ");
    DO(print_ast_code(self->a_loop.l_iter,printer,arg,true,self->a_scope,indent));
    PRINT(") ");
    if (self->a_loop.l_loop) {
     DO(print_ast_code(self->a_loop.l_loop,printer,arg,false,self->a_scope,indent));
    } else {
     PRINT("none;");
    }
   }
  } else if (self->a_flag & AST_FLOOP_POSTCOND) {
   PRINT("do ");
   if (self->a_loop.l_next && self->a_loop.l_loop) {
    PRINT("{\n");
    DO(Dee_FormatRepeat(printer,arg,'\t',indent + 1));
    DO(print_ast_code(self->a_loop.l_loop,printer,arg,false,
                      self->a_scope,indent + 1));
    PRINT(";\n");
    DO(Dee_FormatRepeat(printer,arg,'\t',indent + 1));
    DO(print_ast_code(self->a_loop.l_next,printer,arg,false,
                      self->a_scope,indent + 1));
    PRINT(";\n");
    DO(Dee_FormatRepeat(printer,arg,'\t',indent));
    PRINT("}");
   } else if (self->a_loop.l_next) {
    DO(print_ast_code(self->a_loop.l_next,printer,arg,false,self->a_scope,indent));
   } else if (self->a_loop.l_loop) {
    DO(print_ast_code(self->a_loop.l_loop,printer,arg,false,self->a_scope,indent));
   } else {
    PRINT("{ }");
   }
   PRINT(" while (");
   if (self->a_loop.l_cond) {
    DO(print_ast_code(self->a_loop.l_cond,printer,arg,true,self->a_scope,indent));
   } else {
    PRINT("true");
   }
   PRINT(")");
  } else if (!self->a_loop.l_next) {
   need_semicolon = false;
   PRINT("while (");
   if (self->a_loop.l_cond) {
    DO(print_ast_code(self->a_loop.l_cond,printer,arg,true,self->a_scope,indent));
   } else {
    PRINT("true");
   }
   PRINT(") ");
   if (self->a_loop.l_loop) {
    DO(print_ast_code(self->a_loop.l_loop,printer,arg,false,self->a_scope,indent));
   } else {
    PRINT("{ }");
   }
  } else {
   need_semicolon = false;
   PRINT("for (; ");
   if (self->a_loop.l_cond) {
    DO(print_ast_code(self->a_loop.l_cond,printer,arg,true,self->a_scope,indent));
   }
   PRINT("; ");
   DO(print_ast_code(self->a_loop.l_next,printer,arg,true,self->a_scope,indent));
   PRINT(") ");
   if (self->a_loop.l_loop) {
    DO(print_ast_code(self->a_loop.l_loop,printer,arg,false,self->a_scope,indent));
   } else {
    PRINT("{ }");
   }
  }
  break;

 case AST_LOOPCTL:
  if (self->a_flag & AST_FLOOPCTL_CON) {
   PRINT("continue");
  } else {
   PRINT("break");
  }
  break;

 case AST_CONDITIONAL:
  if (is_expression ||
      self->a_conditional.c_tt == self->a_conditional.c_cond ||
      self->a_conditional.c_ff == self->a_conditional.c_cond) {
   PRINT("(");
   DO(print_ast_code(self->a_conditional.c_cond,printer,arg,true,self->a_scope,indent));
   PRINT(" ? ");
   if (self->a_conditional.c_tt == self->a_conditional.c_cond) {
   } else if (!self->a_conditional.c_tt) {
    PRINT("none");
   } else {
    DO(print_ast_code(self->a_conditional.c_tt,printer,arg,true,self->a_scope,indent));
   }
   if (self->a_conditional.c_ff == self->a_conditional.c_cond) {
    PRINT(" : ");
   } else if (!self->a_conditional.c_ff) {
    PRINT(" : none");
   } else {
    DO(print_ast_code(self->a_conditional.c_ff,printer,arg,true,self->a_scope,indent));
   }
   PRINT(")");
  } else {
   PRINT("if (");
   DO(print_ast_code(self->a_conditional.c_cond,printer,arg,true,self->a_scope,indent));
   PRINT(") ");
   if (self->a_conditional.c_tt) {
    DO(print_ast_code(self->a_conditional.c_tt,printer,arg,false,self->a_scope,indent));
   } else {
    PRINT("{ }");
   }
   if (self->a_conditional.c_ff) {
    PRINT(" else ");
    DO(print_ast_code(self->a_conditional.c_ff,printer,arg,false,self->a_scope,indent));
   }
  }
  break;

 case AST_BOOL:
  PRINT("(");
  print("!!",self->a_flag & AST_FBOOL_NEGATE ? 2 : 1);
  DO(print_ast_code(self->a_bool,printer,arg,true,self->a_scope,indent));
  PRINT(")");
  break;

 case AST_EXPAND:
  DO(print_ast_code(self->a_expand,printer,arg,true,self->a_scope,indent));
  PRINT("...");
  break;

 {
  size_t i;
  DeeBaseScopeObject *function_scope;
 case AST_FUNCTION:
  PRINT("[](");
  function_scope = self->a_function.f_scope;
  for (i = 0; i < function_scope->bs_argc; ++i) {
   if (i != 0) PRINT(", ");
   print(function_scope->bs_argv[i]->s_name->k_name,
         function_scope->bs_argv[i]->s_name->k_size);
   if (DeeBaseScope_IsArgOptional(function_scope,i)) {
    PRINT("?");
   } else if (DeeBaseScope_IsArgDefault(function_scope,i)) {
    printf(" = %r",function_scope->bs_default[i - function_scope->bs_argc_min]);
   }
  }
  if (function_scope->bs_flags & CODE_FVARARGS) {
   if (function_scope->bs_argc) PRINT(", ");
   if (function_scope->bs_varargs) {
    print(function_scope->bs_varargs->s_name->k_name,
          function_scope->bs_varargs->s_name->k_size);
   }
   PRINT("...");
  }
  PRINT(") ");
  DO(print_ast_code(self->a_function.f_code,printer,arg,false,self->a_scope,indent));
 } break;

 {
  struct opinfo *info;
 case AST_OPERATOR_FUNC:
  info = Dee_OperatorInfo(NULL,self->a_flag);
  if (!info) PRINT("(");
  if (self->a_operator_func.of_binding) {
   DO(print_ast_code(self->a_operator_func.of_binding,printer,arg,true,self->a_scope,indent));
   PRINT(".");
  }
  PRINT("operator ");
  if (info) {
   printf("%s",info->oi_sname);
  } else {
   printf("%I16u",self->a_flag);
   PRINT(")");
  }
 } break;

 {
  struct opinfo *info;
  char const *name;
 case AST_OPERATOR:
  if (self->a_operator.o_exflag & AST_OPERATOR_FVARARGS)
      goto operator_fallback;
  if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
   switch (self->a_flag) {
   case OPERATOR_INC:
    DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
    PRINT("++");
    goto done;
   case OPERATOR_DEC:
    DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
    PRINT("--");
    goto done;
   default: break;
   }
   goto operator_fallback;
  }
  switch (self->a_flag) {

  case OPERATOR_COPY:
   PRINT("copy(");
do_unary_operator:
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  case OPERATOR_DEEPCOPY:
   PRINT("deepcopy(");
   goto do_unary_operator;
  case OPERATOR_STR:
   PRINT("str(");
   goto do_unary_operator;
  case OPERATOR_REPR:
   PRINT("repr(");
   goto do_unary_operator;
  case OPERATOR_ASSIGN:
   if (!self->a_operator.o_op1) goto operator_fallback;
   PRINT("(");
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT(" := ");
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  case OPERATOR_CALL:
   if (!self->a_operator.o_op1) goto operator_fallback;
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   if ((self->a_operator.o_op1->a_type == AST_MULTIPLE &&
        self->a_operator.o_op1->a_flag == AST_FMULTIPLE_TUPLE) ||
       (self->a_operator.o_op1->a_type == AST_CONSTEXPR &&
        DeeTuple_Check(self->a_operator.o_op1->a_constexpr))) {
    DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   } else {
    PRINT("((");
    DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
    PRINT(")...)");
   }
   break;
  case OPERATOR_INV:
   PRINT("~(");
   goto do_unary_operator;
  case OPERATOR_POS:
   PRINT("+(");
   goto do_unary_operator;
  case OPERATOR_INC:
   PRINT("++(");
   goto do_unary_operator;
  case OPERATOR_DEC:
   PRINT("--(");
   goto do_unary_operator;
  case OPERATOR_NEG:
   PRINT("-(");
   goto do_unary_operator;
  case OPERATOR_SIZE:
   PRINT("#(");
   goto do_unary_operator;
  case OPERATOR_ADD:
   name = "+";
do_binary:
   if (!self->a_operator.o_op1) goto operator_fallback;
   PRINT("(");
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   printf(" %s ",name);
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  case OPERATOR_SUB:
   name = "-";
   goto do_binary;
  case OPERATOR_MUL:
   name = "*";
   goto do_binary;
  case OPERATOR_DIV:
   name = "/";
   goto do_binary;
  case OPERATOR_MOD:
   name = "%";
   goto do_binary;
  case OPERATOR_SHL:
   name = "<<";
   goto do_binary;
  case OPERATOR_SHR:
   name = ">>";
   goto do_binary;
  case OPERATOR_AND:
   name = "&";
   goto do_binary;
  case OPERATOR_OR:
   name = "|";
   goto do_binary;
  case OPERATOR_XOR:
   name = "^";
   goto do_binary;
  case OPERATOR_POW:
   name = "**";
   goto do_binary;
  case OPERATOR_INPLACE_ADD:
   name = "+=";
   goto do_binary;
   break;
  case OPERATOR_INPLACE_SUB:
   name = "-=";
   goto do_binary;
  case OPERATOR_INPLACE_MUL:
   name = "*=";
   goto do_binary;
  case OPERATOR_INPLACE_DIV:
   name = "/=";
   goto do_binary;
  case OPERATOR_INPLACE_MOD:
   name = "%=";
   goto do_binary;
  case OPERATOR_INPLACE_SHL:
   name = "<<=";
   goto do_binary;
  case OPERATOR_INPLACE_SHR:
   name = ">>=";
   goto do_binary;
  case OPERATOR_INPLACE_AND:
   name = "&=";
   goto do_binary;
  case OPERATOR_INPLACE_OR:
   name = "|=";
   goto do_binary;
  case OPERATOR_INPLACE_XOR:
   name = "^=";
   goto do_binary;
  case OPERATOR_INPLACE_POW:
   name = "**=";
   goto do_binary;
  case OPERATOR_EQ:
   name = "==";
   goto do_binary;
  case OPERATOR_NE:
   name = "!=";
   goto do_binary;
  case OPERATOR_LO:
   name = "<";
   goto do_binary;
  case OPERATOR_LE:
   name = "<=";
   goto do_binary;
  case OPERATOR_GR:
   name = ">";
   goto do_binary;
  case OPERATOR_GE:
   name = ">=";
   goto do_binary;
  case OPERATOR_GETITEM:
   if (!self->a_operator.o_op1) goto operator_fallback;
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT("[");
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT("]");
   break;
  case OPERATOR_DELITEM:
   if (!self->a_operator.o_op1) goto operator_fallback;
   PRINT("del(");
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT("[");
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT("])");
   break;
  case OPERATOR_SETITEM:
   if (!self->a_operator.o_op2) goto operator_fallback;
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT("[");
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT("] = ");
   DO(print_ast_code(self->a_operator.o_op2,printer,arg,true,self->a_scope,indent));
   break;
  case OPERATOR_GETRANGE:
   if (!self->a_operator.o_op2) goto operator_fallback;
   PRINT("[");
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT("[");
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT(":");
   DO(print_ast_code(self->a_operator.o_op2,printer,arg,true,self->a_scope,indent));
   PRINT("]");
   break;
  case OPERATOR_DELRANGE:
   if (!self->a_operator.o_op2) goto operator_fallback;
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT("del([");
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT(":");
   DO(print_ast_code(self->a_operator.o_op2,printer,arg,true,self->a_scope,indent));
   PRINT("])");
   break;
  case OPERATOR_SETRANGE:
   if (!self->a_operator.o_op3) goto operator_fallback;
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT("[");
   DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
   PRINT(":");
   DO(print_ast_code(self->a_operator.o_op2,printer,arg,true,self->a_scope,indent));
   PRINT("] = ");
   DO(print_ast_code(self->a_operator.o_op3,printer,arg,true,self->a_scope,indent));
   break;
  case OPERATOR_GETATTR:
   if (!self->a_operator.o_op1) goto operator_fallback;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR ||
      !DeeString_Check(self->a_operator.o_op1->a_constexpr) ||
      !DeeString_IsSymbol(self->a_operator.o_op1->a_constexpr,0,(size_t)-1))
       goto operator_fallback;
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT(".");
   DO(DeeObject_Print(self->a_operator.o_op1->a_constexpr,printer,arg));
   break;
  case OPERATOR_DELATTR:
   if (!self->a_operator.o_op1) goto operator_fallback;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR ||
      !DeeString_Check(self->a_operator.o_op1->a_constexpr) ||
      !DeeString_IsSymbol(self->a_operator.o_op1->a_constexpr,0,(size_t)-1))
       goto operator_fallback;
   PRINT("del(");
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT(".");
   DO(DeeObject_Print(self->a_operator.o_op1->a_constexpr,printer,arg));
   PRINT(")");
   break;
  case OPERATOR_SETATTR:
   if (!self->a_operator.o_op2) goto operator_fallback;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR ||
      !DeeString_Check(self->a_operator.o_op1->a_constexpr) ||
      !DeeString_IsSymbol(self->a_operator.o_op1->a_constexpr,0,(size_t)-1))
       goto operator_fallback;
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT(".");
   DO(DeeObject_Print(self->a_operator.o_op1->a_constexpr,printer,arg));
   PRINT(" = ");
   DO(print_ast_code(self->a_operator.o_op2,printer,arg,true,self->a_scope,indent));
   break;

  default:
operator_fallback:
   info = Dee_OperatorInfo(NULL,self->a_flag);
   /* TODO: if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP); */
   if (!info) PRINT("(");
   DO(print_ast_code(self->a_operator.o_op0,printer,arg,true,self->a_scope,indent));
   PRINT(".operator ");
   if (info) {
    printf("%s",info->oi_sname);
   } else {
    printf("%I16u ",self->a_flag);
   }
   PRINT("(");
   if (self->a_operator.o_op1) {
    DO(print_ast_code(self->a_operator.o_op1,printer,arg,true,self->a_scope,indent));
    if (self->a_operator.o_op2) {
     PRINT(",");
     DO(print_ast_code(self->a_operator.o_op2,printer,arg,true,self->a_scope,indent));
     if (self->a_operator.o_op3) {
      PRINT(",");
      DO(print_ast_code(self->a_operator.o_op3,printer,arg,true,self->a_scope,indent));
     }
    }
   }
   PRINT(")");
   if (!info) PRINT(")");
   break;
  }
 } break;

 case AST_ACTION:
  switch (self->a_flag & AST_FACTION_KINDMASK) {
#define ACTION(x) case ((x) & AST_FACTION_KINDMASK)
  ACTION(AST_FACTION_CELL0):
   PRINT("<>");
   break;
  ACTION(AST_FACTION_CELL1):
   PRINT("<");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(">");
   break;
  ACTION(AST_FACTION_TYPEOF):
   PRINT("type(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_CLASSOF):
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(".class");
   break;
  ACTION(AST_FACTION_SUPEROF):
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(".super");
   break;
  ACTION(AST_FACTION_PRINT):
   if (!is_scope && is_expression) goto force_scope;
   PRINT("print ");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT("...,");
   break;
  ACTION(AST_FACTION_PRINTLN):
   if (!is_scope && is_expression) goto force_scope;
   PRINT("print ");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT("...");
   break;
  ACTION(AST_FACTION_FPRINT):
   if (!is_scope && is_expression) goto force_scope;
   PRINT("print ");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(": ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT("...,");
   break;
  ACTION(AST_FACTION_FPRINTLN):
   if (!is_scope && is_expression) goto force_scope;
   PRINT("print ");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(": ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT("...");
   break;
  ACTION(AST_FACTION_RANGE):
   PRINT("[");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(":");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT(",");
   DO(print_ast_code(self->a_action.a_act2,printer,arg,true,self->a_scope,indent));
   PRINT("]");
   break;
  ACTION(AST_FACTION_IS):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" is ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_IN):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" in ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_AS):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" as ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_MIN):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" < ...)");
   break;
  ACTION(AST_FACTION_MAX):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" > ...)");
   break;
  ACTION(AST_FACTION_SUM):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" + ...)");
   break;
  ACTION(AST_FACTION_ANY):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" || ...)");
   break;
  ACTION(AST_FACTION_ALL):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" && ...)");
   break;
  ACTION(AST_FACTION_STORE):
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" = ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   break;
  ACTION(AST_FACTION_ASSERT):
   PRINT("assert(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_ASSERT_M):
   PRINT("assert(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(",");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_BOUNDATTR):
   if (self->a_action.a_act1->a_type != AST_CONSTEXPR ||
      !DeeString_Check(self->a_action.a_act1->a_constexpr) ||
      !DeeString_IsSymbol(self->a_action.a_act1->a_constexpr,0,(size_t)-1)) {
    PRINT("bound(");
    DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
    PRINT(".operator . (");
    DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
    PRINT("))");
   } else {
    PRINT("(");
    DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
    PRINT(".");
    DO(DeeObject_Print(self->a_action.a_act1->a_constexpr,printer,arg));
    PRINT(" is bound)");
   }
   break;
  ACTION(AST_FACTION_SAMEOBJ):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" === ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_DIFFOBJ):
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT(" !== ");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
  ACTION(AST_FACTION_CALL_KW):
   DO(print_ast_code(self->a_action.a_act0,printer,arg,true,self->a_scope,indent));
   PRINT("(");
   DO(print_ast_code(self->a_action.a_act1,printer,arg,true,self->a_scope,indent));
   PRINT("...,**");
   DO(print_ast_code(self->a_action.a_act2,printer,arg,true,self->a_scope,indent));
   PRINT(")");
   break;
#undef ACTION
  default:
   printf("/* unknown action %#I16x */",self->a_flag);
   break;
  }
  break;

 //case AST_CLASS: /* TODO */

 case AST_LABEL:
  if (self->a_flag & AST_FLABEL_CASE) {
   if (self->a_label.l_label->tl_expr) {
    PRINT("default");
   } else {
    printf("case %r",self->a_label.l_label->tl_expr);
   }
  } else {
   print(self->a_label.l_label->tl_name->k_name,
         self->a_label.l_label->tl_name->k_size);
  }
  PRINT(":");
  break;

 case AST_GOTO:
  printf("goto %$s",
         self->a_goto.g_label->tl_name->k_size,
         self->a_goto.g_label->tl_name->k_name);
  break;

 case AST_SWITCH:
  if (!is_scope && is_expression) goto force_scope;
  PRINT("switch (");
  DO(print_ast_code(self->a_switch.s_expr,printer,arg,true,self->a_scope,indent));
  PRINT(") ");
  DO(print_ast_code(self->a_switch.s_block,printer,arg,false,self->a_scope,indent));
  break;

 case AST_ASSEMBLY:
  if (!is_scope && is_expression) goto force_scope;
  PRINT("__asm__");
  if (self->a_flag & AST_FASSEMBLY_VOLATILE)
      PRINT(" __volatile__");
  if (self->a_assembly.as_num_l != 0)
      PRINT(" goto");
  PRINT("(");
#ifdef CONFIG_LANGUAGE_NO_ASM
  PRINT("\"\"");
#else
  printf("%$q",
         self->a_assembly.as_text.at_text->s_size,
         self->a_assembly.as_text.at_text->s_text);
#endif
  if (self->a_assembly.as_opc ||
     (self->a_flag & (AST_FASSEMBLY_FORMAT|AST_FASSEMBLY_MEMORY|
                      AST_FASSEMBLY_CLOBSP|AST_FASSEMBLY_REACH|
                      AST_FASSEMBLY_NORETURN))) {
   PRINT(" : ");
   if (self->a_assembly.as_num_o) {
    size_t i;
    for (i = 0; i < self->a_assembly.as_num_o; ++i) {
     if (i != 0) PRINT(", ");
     DO(print_asm_operator(&self->a_assembly.as_opv[i],printer,arg,self->a_scope,indent));
    }
   }
   if (self->a_assembly.as_num_i ||
       self->a_assembly.as_num_l ||
      (self->a_flag & (AST_FASSEMBLY_MEMORY|AST_FASSEMBLY_CLOBSP|
                       AST_FASSEMBLY_REACH|AST_FASSEMBLY_NORETURN))) {
    if (self->a_assembly.as_num_o) {
     PRINT(" : ");
    } else {
     PRINT(": ");
    }
    if (self->a_assembly.as_num_i) {
     size_t i;
     for (i = 0; i < self->a_assembly.as_num_i; ++i) {
      if (i != 0) PRINT(", ");
      DO(print_asm_operator(&self->a_assembly.as_opv[
                             self->a_assembly.as_num_o + i],
                             printer,arg,self->a_scope,indent));
     }
    }
    if (self->a_assembly.as_num_l ||
       (self->a_flag & (AST_FASSEMBLY_MEMORY|AST_FASSEMBLY_CLOBSP|
                        AST_FASSEMBLY_REACH|AST_FASSEMBLY_NORETURN))) {
     bool first_flag = true;
     if (self->a_assembly.as_num_i) {
      PRINT(" : ");
     } else {
      PRINT(": ");
     }
     if (self->a_flag & AST_FASSEMBLY_MEMORY) {
      PRINT("\"memory\"");
      first_flag = false;
     }
     if (self->a_flag & AST_FASSEMBLY_CLOBSP) {
      if (!first_flag) PRINT(", ");
      PRINT("\"sp\"");
      first_flag = false;
     }
     if (self->a_flag & AST_FASSEMBLY_REACH) {
      if (!first_flag) PRINT(", ");
      PRINT("\"reach\"");
      first_flag = false;
     }
     if (self->a_flag & AST_FASSEMBLY_NORETURN) {
      if (!first_flag) PRINT(", ");
      PRINT("\"noreturn\"");
      first_flag = false;
     }
     if (self->a_assembly.as_num_l) {
      if (self->a_flag & (AST_FASSEMBLY_MEMORY|AST_FASSEMBLY_CLOBSP|
                          AST_FASSEMBLY_REACH|AST_FASSEMBLY_NORETURN)) {
       PRINT(" : ");
      } else {
       PRINT(": ");
      }
      if (self->a_assembly.as_num_l) {
       size_t i;
       for (i = 0; i < self->a_assembly.as_num_l; ++i) {
        if (i != 0) PRINT(", ");
        DO(print_asm_label_operator(&self->a_assembly.as_opv[
                                     self->a_assembly.as_num_i +
                                     self->a_assembly.as_num_o +
                                     i],printer,arg));
       }
      }
     }
    }
   }
  }
  PRINT(")");
  break;


 default:
  printf("/* unknown ast: %I16u */",self->a_type);
  break;
 }
done:
 LEAVE_SCOPE(is_expression,need_semicolon);
 return 0;
err:
 return -1;
}

#undef printf
#undef PRINT
#undef print
#undef DO

PRIVATE DREF DeeObject *DCALL
ast_str(DeeCompilerAstObject *__restrict self) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 COMPILER_BEGIN(self->ci_compiler);
 if unlikely(print_ast_code(self->ci_value,
                           (dformatprinter)&unicode_printer_print,
                           &printer,true,self->ci_value->a_scope,0))
    goto err;
 COMPILER_END();
 return unicode_printer_pack(&printer);
err:
 COMPILER_END();
 unicode_printer_fini(&printer);
 return NULL;
}


INTDEF int DCALL
print_scope_repr(DeeScopeObject *__restrict self,
                 struct unicode_printer *__restrict printer);

PRIVATE dssize_t DCALL
print_operator_name(uint16_t opid,
                    struct unicode_printer *__restrict printer) {
 struct opinfo *info;
 switch (opid) {
 case AST_OPERATOR_POS_OR_ADD:
  return unicode_printer_print8(printer,(uint8_t *)"\"+\"",3);
 case AST_OPERATOR_NEG_OR_SUB:
  return unicode_printer_print8(printer,(uint8_t *)"\"-\"",3);
 case AST_OPERATOR_GETITEM_OR_SETITEM:
  return unicode_printer_print8(printer,(uint8_t *)"\"[]\"",4);
 case AST_OPERATOR_GETRANGE_OR_SETRANGE:
  return unicode_printer_print8(printer,(uint8_t *)"\"[:]\"",5);
 case AST_OPERATOR_GETATTR_OR_SETATTR:
  return unicode_printer_print8(printer,(uint8_t *)"\".\"",3);
 default: break;
 }
 info = Dee_OperatorInfo(NULL,opid);
 if unlikely(!info)
    return unicode_printer_printf(printer,"%I16u",opid);
 return unicode_printer_printf(printer,"%q",info->oi_sname);
}


PRIVATE char const action_names[][10] = {
    /* [AST_FACTION_CELL0    & AST_FACTION_KINDMASK] = */"cell",
    /* [AST_FACTION_CELL1    & AST_FACTION_KINDMASK] = */"cell",
    /* [AST_FACTION_TYPEOF   & AST_FACTION_KINDMASK] = */"typeof",
    /* [AST_FACTION_CLASSOF  & AST_FACTION_KINDMASK] = */"classof",
    /* [AST_FACTION_SUPEROF  & AST_FACTION_KINDMASK] = */"superof",
    /* [AST_FACTION_PRINT    & AST_FACTION_KINDMASK] = */"print",
    /* [AST_FACTION_PRINTLN  & AST_FACTION_KINDMASK] = */"println",
    /* [AST_FACTION_FPRINT   & AST_FACTION_KINDMASK] = */"fprint",
    /* [AST_FACTION_FPRINTLN & AST_FACTION_KINDMASK] = */"fprintln",
    /* [AST_FACTION_RANGE    & AST_FACTION_KINDMASK] = */"range",
    /* [AST_FACTION_IS       & AST_FACTION_KINDMASK] = */"is",
    /* [AST_FACTION_IN       & AST_FACTION_KINDMASK] = */"in",
    /* [AST_FACTION_AS       & AST_FACTION_KINDMASK] = */"as",
    /* [AST_FACTION_MIN      & AST_FACTION_KINDMASK] = */"min",
    /* [AST_FACTION_MAX      & AST_FACTION_KINDMASK] = */"max",
    /* [AST_FACTION_SUM      & AST_FACTION_KINDMASK] = */"sum",
    /* [AST_FACTION_ANY      & AST_FACTION_KINDMASK] = */"any",
    /* [AST_FACTION_ALL      & AST_FACTION_KINDMASK] = */"all",
    /* [AST_FACTION_STORE    & AST_FACTION_KINDMASK] = */"store",
    /* [AST_FACTION_ASSERT   & AST_FACTION_KINDMASK] = */"assert",
    /* [AST_FACTION_ASSERT_M & AST_FACTION_KINDMASK] = */"assert",
};


PRIVATE int DCALL
print_ast_repr(struct ast *__restrict self,
               struct unicode_printer *__restrict printer) {
#define DO(x)         do if ((x) < 0) goto err; __WHILE0
#define PUTC(c)       DO(unicode_printer_putascii(printer,c))
#define PRINT(str)    DO(UNICODE_PRINTER_PRINT(printer,str))
#define PRINTAST(ast) DO(print_ast_repr(ast,printer))
#define print(p,s)    DO(unicode_printer_print(printer,p,s))
#define printf(...)   DO(unicode_printer_printf(printer,__VA_ARGS__))
 switch (self->a_type) {

 case AST_CONSTEXPR:
  printf("makeconstexpr(value: %r",self->a_constexpr);
  break;

 case AST_SYM:
  printf("makesym(sym: symbol(%$q)",
         self->a_sym->s_name->k_size,
         self->a_sym->s_name->k_name);
  break;

 case AST_UNBIND:
  printf("makeunbind(sym: symbol(%$q)",
         self->a_sym->s_name->k_size,
         self->a_sym->s_name->k_name);
  break;

 case AST_BOUND:
  printf("makebound(sym: symbol(%$q)",
         self->a_sym->s_name->k_size,
         self->a_sym->s_name->k_name);
  break;

 {
  char *typing; size_t i;
 case AST_MULTIPLE:
  typing = NULL;
  /**/ if (self->a_flag == AST_FMULTIPLE_TUPLE) typing = "tuple";
  else if (self->a_flag == AST_FMULTIPLE_LIST) typing = "list";
  else if (self->a_flag == AST_FMULTIPLE_SET) typing = "set";
  else if (self->a_flag == AST_FMULTIPLE_DICT) typing = "dict";
  else if (self->a_flag == AST_FMULTIPLE_GENERIC) typing = "sequence";
  else if (self->a_flag == AST_FMULTIPLE_GENERIC_KEYS) typing = "mapping";
  printf("makemultiple(branches: { ");
  for (i = 0; i < self->a_multiple.m_astc; ++i) {
   if (i != 0) PRINT(", ");
   PRINTAST(self->a_multiple.m_astv[i]);
  }
  PRINT(" }, typing: ");
  if (typing) {
   printf("\"%s\"",typing);
  } else {
   PRINT("none");
  }
 } break;

 case AST_RETURN:
  PRINT("makereturn(expr: ");
print_single_expr:
  if (self->a_return) {
   PRINTAST(self->a_return);
  } else {
   PRINT("none");
  }
  break;

 case AST_YIELD:
  PRINT("makeyield(expr: ");
  ASSERT(self->a_yield);
  goto print_single_expr;

 case AST_THROW:
  PRINT("makethrow(expr: ");
  goto print_single_expr;

 {
  size_t i;
 case AST_TRY:
  PRINT("maketry(guard: ");
  PRINTAST(self->a_try.t_guard);
  PRINT(", handlers: {");
  for (i = 0; i < self->a_try.t_catchc; ++i) {
   bool first_flag = true;
   struct catch_expr *handler;
   handler = &self->a_try.t_catchv[i];
   if (i != 0) PUTC(',');
   PRINT("(\"");
   if (handler->ce_flags & EXCEPTION_HANDLER_FFINALLY) {
    PRINT("finally");
    first_flag = false;
   }
   if (handler->ce_flags & EXCEPTION_HANDLER_FINTERPT) {
    if (!first_flag)
         PUTC(',');
    PRINT("interrupt");
   }
   PRINT("\",");
   if (handler->ce_mask) {
    PRINTAST(handler->ce_mask);
    PUTC(',');
   } else {
    PRINT("none,");
   }
   PRINTAST(handler->ce_code);
   PRINT(")");
  }
  PUTC('}');
 } break;

 {
  bool first_flag;
 case AST_LOOP:
  PRINT("makeloop(flags: \"");
  first_flag = true;
  if (self->a_flag & AST_FLOOP_FOREACH) {
   PRINT("foreach");
   first_flag = false;
  }
  if (self->a_flag & AST_FLOOP_POSTCOND) {
   if (!first_flag)
        PUTC(',');
   PRINT("postcond");
   first_flag = false;
  }
  if (self->a_flag & AST_FLOOP_UNLIKELY) {
   if (!first_flag)
        PUTC(',');
   PRINT("unlikely");
  }
  PRINT("\", ");
  if (self->a_flag & AST_FLOOP_FOREACH) {
   PRINT("\", elem: ");
   if (self->a_loop.l_elem) {
    PRINTAST(self->a_loop.l_elem);
   } else {
    PRINT("none");
   }
   PRINT(", iter: ");
   PRINTAST(self->a_loop.l_iter);
  } else {
   PRINT("\", cond: ");
   if (self->a_loop.l_cond) {
    PRINTAST(self->a_loop.l_cond);
   } else {
    PRINT("none");
   }
   PRINT(", next: ");
   if (self->a_loop.l_next) {
    PRINTAST(self->a_loop.l_next);
   } else {
    PRINT("none");
   }
  }
  PRINT(", loop: ");
  if (self->a_loop.l_loop) {
   PRINTAST(self->a_loop.l_loop);
  } else {
   PRINT("none");
  }
 } break;

 case AST_LOOPCTL:
  PRINT("makeloopctl(isbreak: ");
  if (self->a_flag & AST_FLOOPCTL_CON) {
   PRINT("false");
  } else {
   PRINT("true");
  }
  break;
  
 {
  bool first_flag;
 case AST_CONDITIONAL:
  PRINT("makeconditional(cond: ");
  PRINTAST(self->a_conditional.c_cond);
  PRINT(", tt: ");
  if (!self->a_conditional.c_tt)
   PRINT("none");
  else if (self->a_conditional.c_tt == self->a_conditional.c_cond)
   PRINT("<cond>");
  else {
   PRINTAST(self->a_conditional.c_tt);
  }
  PRINT(", ff: ");
  if (!self->a_conditional.c_ff)
   PRINT("none");
  else if (self->a_conditional.c_ff == self->a_conditional.c_cond)
   PRINT("<cond>");
  else {
   PRINTAST(self->a_conditional.c_ff);
  }
  first_flag = true;
  PRINT(", flags: \"");
  if (self->a_flag & AST_FCOND_BOOL) {
   PRINT("bool");
   first_flag = false;
  }
  if (self->a_flag & AST_FCOND_LIKELY) {
   if (first_flag)
       PUTC(',');
   PRINT("likely");
   first_flag = false;
  }
  if (self->a_flag & AST_FCOND_UNLIKELY) {
   if (first_flag)
       PUTC(',');
   PRINT("unlikely");
   first_flag = false;
  }
  PUTC('\"');
 } break;

 case AST_BOOL:
  PRINT("makebool(expr: ");
  PRINTAST(self->a_bool);
  PRINT(", negate: ");
  if (self->a_flag & AST_FBOOL_NEGATE) {
   PRINT("true");
  } else {
   PRINT("false");
  }
  break;

 case AST_EXPAND:
  PRINT("makeexpand(expr: ");
  goto print_single_expr;

 case AST_FUNCTION:
  PRINT("makefunction(code: ");
  goto print_single_expr;

 case AST_OPERATOR_FUNC:
  PRINT("makeoperatorfunc(name: ");
  DO(print_operator_name(self->a_flag,printer));
  PRINT(", binding: ");
  if (self->a_operator_func.of_binding) {
   PRINTAST(self->a_operator_func.of_binding);
  } else {
   PRINT("none");
  }
  break;

 {
  bool first_flag;
 case AST_OPERATOR:
  PRINT("makeoperator(name: ");
  DO(print_operator_name(self->a_flag,printer));
  PRINT(", a: ");
  if (self->a_operator.o_op0) {
   PRINTAST(self->a_operator.o_op0);
  } else {
   PRINT("none");
  }
  PRINT(", b: ");
  if (self->a_operator.o_op1) {
   PRINTAST(self->a_operator.o_op1);
  } else {
   PRINT("none");
  }
  PRINT(", c: ");
  if (self->a_operator.o_op2) {
   PRINTAST(self->a_operator.o_op2);
  } else {
   PRINT("none");
  }
  PRINT(", d: ");
  if (self->a_operator.o_op3) {
   PRINTAST(self->a_operator.o_op3);
  } else {
   PRINT("none");
  }
  first_flag = true;
  PRINT(", flags: \"");
  if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
   PRINT("post");
   first_flag = false;
  }
  if (self->a_operator.o_exflag & AST_OPERATOR_FVARARGS) {
   PRINT("varargs");
   first_flag = false;
  }
  if (self->a_operator.o_exflag & AST_OPERATOR_FMAYBEPFX) {
   PRINT("maybeprefix");
   first_flag = false;
  }
  if (self->a_operator.o_exflag & AST_OPERATOR_FDONTOPT) {
   PRINT("dontoptimize");
   first_flag = false;
  }
  PUTC('\"');
 } break;

 case AST_ACTION:
  PRINT("makeaction(name: ");
  if ((self->a_flag & AST_FACTION_KINDMASK) < COMPILER_LENOF(action_names)) {
   printf("%q",action_names[self->a_flag & AST_FACTION_KINDMASK]);
  } else {
   printf("\"unknown:%I16u\"",self->a_flag & AST_FACTION_KINDMASK);
  }
  PRINT(", a: ");
  if (AST_FACTION_ARGC_GT(self->a_flag) >= 1) {
   PRINTAST(self->a_action.a_act0);
  } else {
   PRINT("none");
  }
  PRINT(", b: ");
  if (AST_FACTION_ARGC_GT(self->a_flag) >= 2) {
   PRINTAST(self->a_action.a_act1);
  } else {
   PRINT("none");
  }
  PRINT(", c: ");
  if (AST_FACTION_ARGC_GT(self->a_flag) >= 3) {
   PRINTAST(self->a_action.a_act2);
  } else {
   PRINT("none");
  }
  PRINT(", mustrun: ");
  if (self->a_flag & AST_FACTION_MAYBERUN) {
   PRINT("false");
  } else {
   PRINT("true");
  }
  break;

 //case AST_CLASS: /* TODO */
 //case AST_LABEL: /* TODO */
 //case AST_GOTO: /* TODO */
 //case AST_SWITCH: /* TODO */
 //case AST_ASSEMBLY: /* TODO */

 default:
  printf("<ast(type: %I16u)>",self->a_type);
  break;
 }
 PRINT(", scope: ");
 DO(print_scope_repr(self->a_scope,printer));
 PUTC(')');
 return 0;
err:
 return -1;
#undef printf
#undef PRINTAST
#undef PRINT
#undef print
#undef PUTC
#undef DO
}

PRIVATE DREF DeeObject *DCALL
ast_repr(DeeCompilerAstObject *__restrict self) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 COMPILER_BEGIN(self->ci_compiler);
 if unlikely(print_ast_repr(self->ci_value,&printer))
    goto err;
 COMPILER_END();
 return unicode_printer_pack(&printer);
err:
 COMPILER_END();
 unicode_printer_fini(&printer);
 return NULL;
}



PRIVATE struct type_getset ast_getsets[] = {
    { "scope",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ast_getscope, NULL,
      (int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ast_setscope,
      DOC("->:compiler.scope\n"
          "@throw ValueError Attempted to set a scope associated with a different compiler\n"
          "@throw ReferenceError Attempted to set a scope not apart of the same base-scope (s.a. :compiler.scope.base)\n"
          "@throw ReferenceError Attempted to set the scope of a branch containing symbols that would no longer be reachable\n"
          "Get or set the scope with which this branch is assocated") },
    { "type",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ast_gettype, NULL, NULL,
      DOC("->string\n"
          "Get the name of the ast type (same as the `make*' methods of :compiler)") },
    { "typeid",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ast_gettypeid, NULL, NULL,
      DOC("->int\n"
          "Get the internal type-id of ast") },
    /* TODO: Access to all the different ast fields. */
    { NULL }
};



INTERN DeeTypeObject DeeCompilerAst_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"ast",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeCompilerObjItem_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                TYPE_ALLOCATOR(compiler_item_tp_alloc,compiler_item_tp_free)
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ast_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ast_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */ast_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_IAST_C */
