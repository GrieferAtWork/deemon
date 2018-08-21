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
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/interface.h>
#include <deemon/util/cache.h>

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


//PRIVATE int DCALL
//print_ast_code(struct ast *__restrict self,
//               dformatprinter printer, void *arg, bool is_expression,
//               DeeScopeObject *__restrict caller_scope) {
//}
//
//PRIVATE DREF DeeObject *DCALL
//ast_str(DeeCompilerAstObject *__restrict self) {
// struct unicode_printer printer = UNICODE_PRINTER_INIT;
//}

PRIVATE int DCALL
print_ast_repr(struct ast *__restrict self,
               struct unicode_printer *__restrict printer) {
#define DO(x)         do if ((x) < 0) goto err; __WHILE0
#define PRINT(str)    DO(UNICODE_PRINTER_PRINT(printer,str))
#define PRINTAST(ast) DO(print_ast_repr(ast,printer))
#define print(p,s)    DO(unicode_printer_print(printer,p,s))
#define printf(...)   DO(unicode_printer_printf(printer,__VA_ARGS__))
 switch (self->a_type) {

 case AST_CONSTEXPR:
  printf("makeconstexpr(%r)",self->a_constexpr);
  break;

 case AST_SYM:
  printf("makesym(symbol(%$q))",
         self->a_sym->s_name->k_size,
         self->a_sym->s_name->k_name);
  break;

 case AST_UNBIND:
  printf("makeunbind(symbol(%$q))",
         self->a_sym->s_name->k_size,
         self->a_sym->s_name->k_name);
  break;

 case AST_BOUND:
  printf("makebound(symbol(%$q))",
         self->a_sym->s_name->k_size,
         self->a_sym->s_name->k_name);
  break;

 {
  char *typing; size_t i;
 case AST_MULTIPLE:
  typing = "";
  /**/ if (self->a_flag == AST_FMULTIPLE_TUPLE) typing = "tuple";
  else if (self->a_flag == AST_FMULTIPLE_LIST) typing = "list";
  else if (self->a_flag == AST_FMULTIPLE_SET) typing = "set";
  else if (self->a_flag == AST_FMULTIPLE_DICT) typing = "dict";
  else if (self->a_flag == AST_FMULTIPLE_GENERIC) typing = "sequence";
  else if (self->a_flag == AST_FMULTIPLE_GENERIC_KEYS) typing = "mapping";
  printf("makemultiple(\"%s\",{ ",typing);
  for (i = 0; i < self->a_multiple.m_astc; ++i) {
   if (i != 0) PRINT(", ");
   PRINTAST(self->a_multiple.m_astv[i]);
  }
  PRINT(" })");
 } break;

 case AST_RETURN:
  PRINT("makereturn(");
  if (self->a_return)
      PRINTAST(self->a_return);
  PRINT(")");
  break;

 case AST_YIELD:
  PRINT("makeyield(");
  PRINTAST(self->a_yield);
  PRINT(")");
  break;

 case AST_THROW:
  PRINT("makethrow(");
  if (self->a_throw)
      PRINTAST(self->a_throw);
  PRINT(")");
  break;

 //case AST_TRY: /* TODO */
 //case AST_LOOP: /* TODO */
 //case AST_LOOPCTL: /* TODO */
 //case AST_CONDITIONAL: /* TODO */
 //case AST_BOOL: /* TODO */
 //case AST_EXPAND: /* TODO */
 //case AST_FUNCTION: /* TODO */
 //case AST_OPERATOR_FUNC: /* TODO */
 //case AST_OPERATOR: /* TODO */
 //case AST_ACTION: /* TODO */
 //case AST_CLASS: /* TODO */
 //case AST_LABEL: /* TODO */
 //case AST_GOTO: /* TODO */
 //case AST_SWITCH: /* TODO */
 //case AST_ASSEMBLY: /* TODO */

 default:
  printf("<ast(type: %I16u)>",self->a_type);
  break;
 }
 return 0;
err:
 return -1;
#undef printf
#undef PRINTAST
#undef PRINT
#undef print
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
          "@throw ReferenceError Attempted to set a scope not apart of the same base-scope (s.a. :compier.scope.base)\n"
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
        /* .tp_str  = */NULL,
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
