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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_IPARSER_C
#define GUARD_DEEMON_COMPILER_INTERFACE_IPARSER_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/arg.h>
#include <deemon/none.h>
#include <deemon/thread.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/util/cache.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/lexer.h>

DECL_BEGIN

DECLARE_OBJECT_CACHE(compiler_wrap,DeeCompilerWrapperObject)




INTDEF int DCALL
get_scope_lookupmode(DeeObject *__restrict value,
                     unsigned int *__restrict presult);


PRIVATE struct keyword suffix_kwlist[] = { K(head), KEND };
PRIVATE struct keyword lookupmode_kwlist[] = { K(lookupmode), KEND };
#define DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(name,func,IF_SUFFIX,is_suffix) \
PRIVATE DREF DeeObject *DCALL \
parser_##name(DeeCompilerWrapperObject *__restrict self, \
              size_t argc, DeeObject **__restrict argv, \
              DeeObject *kw) \
{ \
 DREF DeeObject *result = NULL; \
 DeeCompilerAstObject *head; DREF struct ast *result_ast; \
 COMPILER_BEGIN(self->cw_compiler); \
 if (DeeArg_UnpackKw(argc,argv,kw,suffix_kwlist,"o:" #name,&head)) \
     goto done; \
 if (DeeObject_AssertTypeExact((DeeObject *)head,&DeeCompilerAst_Type)) \
     goto done; \
 if unlikely(head->ci_compiler != self->cw_compiler) { err_invalid_ast_compiler(head); goto done; } \
 if unlikely(head->ci_value->a_scope->s_base != current_basescope) { err_invalid_ast_basescope(head,current_basescope); goto done; } \
 ast_incref(head->ci_value); \
 IF_SUFFIX(if (!(is_suffix)) { \
  result_ast = head->ci_value; \
 } else) \
 { \
  uint16_t old_exceptsz = DeeThread_Self()->t_exceptsz; \
  result_ast = func(head->ci_value); \
  if unlikely(!result_ast) { \
   if (old_exceptsz == DeeThread_Self()->t_exceptsz) { \
    result = Dee_None; \
    Dee_Incref(result); \
   } \
   goto done; \
  } \
 } \
 result = DeeCompiler_GetAst(result_ast); \
 ast_decref_unlikely(result_ast); \
done: \
 COMPILER_END(); \
 return result; \
}
#define DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(name,func) \
PRIVATE DREF DeeObject *DCALL \
parser_##name(DeeCompilerWrapperObject *__restrict self, \
              size_t argc, DeeObject **__restrict argv, \
              DeeObject *kw) \
{ \
 DREF DeeObject *result = NULL; \
 DREF struct ast *result_ast; unsigned int lookup_mode; \
 DeeObject *lookup_mode_ob = Dee_EmptyString; \
 uint16_t old_exceptsz; \
 COMPILER_BEGIN(self->cw_compiler); \
 if (DeeArg_UnpackKw(argc,argv,kw,lookupmode_kwlist,"|o:" #name,&lookup_mode_ob)) \
     goto done; \
 if unlikely(get_scope_lookupmode(lookup_mode_ob,&lookup_mode)) \
    goto done; \
 result_ast = func(lookup_mode); \
 old_exceptsz = DeeThread_Self()->t_exceptsz; \
 if unlikely(!result_ast) { \
  if (old_exceptsz == DeeThread_Self()->t_exceptsz) { \
   result = Dee_None; \
   Dee_Incref(result); \
  } \
  goto done; \
 } \
 result = DeeCompiler_GetAst(result_ast); \
 ast_decref_unlikely(result_ast); \
done: \
 COMPILER_END(); \
 return result; \
}
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_unaryhead,ast_parse_unaryhead)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_unary,ast_parse_unary)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_prod,ast_parse_prod)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_sum,ast_parse_sum)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_shift,ast_parse_shift)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_cmp,ast_parse_cmp)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_cmpeq,ast_parse_cmpeq)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_and,ast_parse_and)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_xor,ast_parse_xor)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_or,ast_parse_or)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_as,ast_parse_as)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_land,ast_parse_land)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_lor,ast_parse_lor)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_cond,ast_parse_cond)
DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION(parse_assign,ast_parse_assign)
#define IF_TRUE(x)  x
#define IF_FALSE(x)
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_unarytail,ast_parse_unary_operand,IF_FALSE,)
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_prodtail,ast_parse_prod_operand,IF_TRUE,TOKEN_IS_PROD(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_sumtail,ast_parse_sum_operand,IF_TRUE,TOKEN_IS_SUM(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_shifttail,ast_parse_shift_operand,IF_TRUE,TOKEN_IS_SHIFT(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_cmptail,ast_parse_cmp_operand,IF_TRUE,TOKEN_IS_CMP(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_cmpeqtail,ast_parse_cmpeq_operand,IF_TRUE,TOKEN_IS_CMPEQ(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_andtail,ast_parse_and_operand,IF_TRUE,TOKEN_IS_AND(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_xortail,ast_parse_xor_operand,IF_TRUE,TOKEN_IS_XOR(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_ortail,ast_parse_or_operand,IF_TRUE,TOKEN_IS_OR(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_astail,ast_parse_as_operand,IF_TRUE,TOKEN_IS_AS(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_landtail,ast_parse_land_operand,IF_TRUE,TOKEN_IS_LAND(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_lortail,ast_parse_lor_operand,IF_TRUE,TOKEN_IS_LOR(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_condtail,ast_parse_cond_operand,IF_TRUE,TOKEN_IS_COND(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_assigntail,ast_parse_assign_operand,IF_TRUE,TOKEN_IS_ASSIGN(tok))
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_exprtail,ast_parse_postexpr,IF_FALSE,)
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_maptail,ast_parse_mapping,IF_TRUE,tok == (tok_t)':')
DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION(parse_seqtail,ast_parse_brace_list,IF_TRUE,tok == (tok_t)',')
#undef IF_FALSE
#undef IF_TRUE
#undef DEFINE_SIMPLE_LOOKUPMODE_PARSER_FUNCTION
#undef DEFINE_SIMPLE_SUFFIX_PARSER_FUNCTION


PRIVATE DREF DeeObject *DCALL
parser_parse_stmt(DeeCompilerWrapperObject *__restrict self,
                  size_t argc, DeeObject **__restrict argv,
                  DeeObject *kw) {
 DREF DeeObject *result = NULL; bool nonblocking = false;
 DREF struct ast *result_ast;
 uint16_t old_exceptsz;
 PRIVATE struct keyword kwlist[] = { K(nonblocking), KEND };
 COMPILER_BEGIN(self->cw_compiler);
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"|b:parse_stmt",&nonblocking))
     goto done;
 old_exceptsz = DeeThread_Self()->t_exceptsz;
 result_ast = ast_parse_statement(nonblocking);
 if unlikely(!result_ast) {
  if (old_exceptsz == DeeThread_Self()->t_exceptsz) {
   result = Dee_None;
   Dee_Incref(result);
  }
  goto done;
 }
 result = DeeCompiler_GetAst(result_ast);
 ast_decref_unlikely(result_ast);
done:
 COMPILER_END();
 return result;
}

PRIVATE DREF DeeObject *DCALL
parser_parse_allstmt(DeeCompilerWrapperObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
 DREF DeeObject *result = NULL;
 DeeObject *end = Dee_EmptyString;
 DREF struct ast *result_ast;
 tok_t end_token = TOK_EOF;
 uint16_t old_exceptsz;
 PRIVATE struct keyword kwlist[] = { K(end), KEND };
 COMPILER_BEGIN(self->cw_compiler);
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"|o:parse_allstmt",&end))
     goto done;
 if (end != Dee_EmptyString) {
  end_token = get_token_from_obj(end,true);
  if unlikely(end_token == TOK_ERR) goto done;
 }
 old_exceptsz = DeeThread_Self()->t_exceptsz;
 result_ast = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST,
                                         end_token);
 if unlikely(!result_ast) {
  if (old_exceptsz == DeeThread_Self()->t_exceptsz) {
   result = Dee_None;
   Dee_Incref(result);
  }
  goto done;
 }
 result = DeeCompiler_GetAst(result_ast);
 ast_decref_unlikely(result_ast);
done:
 COMPILER_END();
 return result;
}


PRIVATE struct type_method parser_methods[] = {
    { "parse_unaryhead",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_unaryhead,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a unary head-expression, that is a unary expression without any tail- or binary suffix"),
      TYPE_METHOD_FKWDS },
    { "parse_unary",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_unary,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a whole unary expression"),
      TYPE_METHOD_FKWDS },
    { "parse_prod",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_prod,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a product expression"),
      TYPE_METHOD_FKWDS },
    { "parse_sum",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_sum,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a sum expression"),
      TYPE_METHOD_FKWDS },
    { "parse_shift",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_shift,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a shift expression"),
      TYPE_METHOD_FKWDS },
    { "parse_cmp",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_cmp,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a compare expression (non-equal)"),
      TYPE_METHOD_FKWDS },
    { "parse_cmpeq",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_cmpeq,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a compare expression (equal)"),
      TYPE_METHOD_FKWDS },
    { "parse_and",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_and,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a bit-wise and expression"),
      TYPE_METHOD_FKWDS },
    { "parse_xor",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_xor,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a bit-wise xor expression"),
      TYPE_METHOD_FKWDS },
    { "parse_or",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_or,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a bit-wise or expression"),
      TYPE_METHOD_FKWDS },
    { "parse_as",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_as,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse an as expression"),
      TYPE_METHOD_FKWDS },
    { "parse_land",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_land,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a logical and expression"),
      TYPE_METHOD_FKWDS },
    { "parse_lor",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_lor,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a logical or expression"),
      TYPE_METHOD_FKWDS },
    { "parse_cond",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_cond,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse a conditional expression"),
      TYPE_METHOD_FKWDS },
    { "parse_assign",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_assign,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Parse an assignment (not store), or inplace expression"),
      TYPE_METHOD_FKWDS },
    { "parse_expr",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_assign,
      DOC("(lookupmode=!P{})->?AAst?Ert:Compiler\n"
          "(lookupmode=!0)->?AAst?Ert:Compiler\n"
          "Alias for #parse_assign"),
      TYPE_METHOD_FKWDS },
    { "parse_unarytail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_unarytail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a unary expression (which includes attribute lookups, and call operations)"),
      TYPE_METHOD_FKWDS },
    { "parse_prodtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_prodtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a product expression"),
      TYPE_METHOD_FKWDS },
    { "parse_sumtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_sumtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a sum expression"),
      TYPE_METHOD_FKWDS },
    { "parse_shifttail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_shifttail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a shift expression"),
      TYPE_METHOD_FKWDS },
    { "parse_cmptail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_cmptail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a cmp expression"),
      TYPE_METHOD_FKWDS },
    { "parse_cmpeqtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_cmpeqtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a cmpeq expression"),
      TYPE_METHOD_FKWDS },
    { "parse_andtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_andtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a and expression"),
      TYPE_METHOD_FKWDS },
    { "parse_xortail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_xortail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a xor expression"),
      TYPE_METHOD_FKWDS },
    { "parse_ortail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_ortail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a or expression"),
      TYPE_METHOD_FKWDS },
    { "parse_astail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_astail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a as expression"),
      TYPE_METHOD_FKWDS },
    { "parse_landtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_landtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a land expression"),
      TYPE_METHOD_FKWDS },
    { "parse_lortail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_lortail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a lor expression"),
      TYPE_METHOD_FKWDS },
    { "parse_condtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_condtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a cond expression"),
      TYPE_METHOD_FKWDS },
    { "parse_assigntail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_assigntail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a assign expression"),
      TYPE_METHOD_FKWDS },
    { "parse_exprtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_exprtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a expr expression"),
      TYPE_METHOD_FKWDS },
    { "parse_maptail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_maptail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a generic mapping expression"),
      TYPE_METHOD_FKWDS },
    { "parse_seqtail",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_seqtail,
      DOC("(head:?AAst?Ert:Compiler)->?AAst?Ert:Compiler\n"
          "Parse the tail of a generic sequence expression"),
      TYPE_METHOD_FKWDS },
    { "parse_stmt",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_stmt,
      DOC("(bool nonblocking=false)->?AAst?Ert:Compiler\n"
          "Parse a statement or #parse_comma expression"),
      TYPE_METHOD_FKWDS },
    { "parse_allstmt",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&parser_parse_allstmt,
      DOC("(end=!P{})->?AAst?Ert:Compiler\n"
          "(end=!0)->?AAst?Ert:Compiler\n"
          "Parse statements (#parse_stmt) and pack them togerther into "
          "a multiple/keep-last branch, until the input file ends, or "
          "a token equal to @end (s.a. :Compiler.lexer.token.op:eq) is "
          "encountered at the start of a statement"),
      TYPE_METHOD_FKWDS },
    { NULL }
};


PRIVATE DREF DeeObject *DCALL
parser_getlfstmt(DeeCompilerWrapperObject *__restrict self) {
 DREF DeeObject *result;
 COMPILER_BEGIN(self->cw_compiler);
 result = DeeBool_For(parser_flags & PARSE_FLFSTMT);
 Dee_Incref(result);
 COMPILER_END();
 return result;
}

PRIVATE int DCALL
parser_setlfstmt(DeeCompilerWrapperObject *__restrict self,
                 DeeObject *__restrict value) {
 int newval = DeeObject_Bool(value);
 if unlikely(newval < 0) return -1;
 COMPILER_BEGIN(self->cw_compiler);
 if (newval) {
  parser_flags |= PARSE_FLFSTMT;
 } else {
  parser_flags &= ~PARSE_FLFSTMT;
 }
 COMPILER_END();
 return 0;
}

PRIVATE struct type_getset parser_getsets[] = {
    { "allowlfstatements",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&parser_getlfstmt, NULL,
      (int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&parser_setlfstmt,
      DOC("->?Dbool\n"
          "Get or set if line-feeds should be recognized as statement terminators "
          "(you must also enable line-feed token created in :Compiler.lexer.wantlf)\n"
          "When enabled, most statements can be termined by line-feeds, in addition to "
          "semicolon tokens.") },
    { NULL }
};

PRIVATE struct type_member parser_class_members[] = {
    TYPE_MEMBER_CONST("Ast",&DeeCompilerAst_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeCompilerParser_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_Parser",
    /* .tp_doc      = */DOC("Parse data from the input token stream (:Compiler.lexer) of the associated #compiler\n"
                            "For more information on what exactly is parsed by individual `parse_*' functions, see the "
                            "following table, or consult the `/lib/LANGUAGE.txt' file within the deemon source tree\n"
                            "%{table Function|Example|Description\n"
                            "#parse_unaryhead|${[foo].bar}, ${[42] + 7}, ${[(10 + 20)]}, ${[[]{ return 42; }]}|Parse the head of a unary expression, including unary prefix operators, and the unary operand itself\n"
                            "#parse_unary|${[foo.bar]}, ${[getitems(a,b,c)]}|Parse a whole unary expression\n"
                            "#parse_prod|${[x * y / z % v ** w]}|Parse product expressions (using #parse_unary for operands)\n"
                            "#parse_sum|${[x + y - z]}|Parse sum expressions (using #parse_prod for operands)\n"
                            "#parse_shift|${[x << y >> z]}|Parse shift expressions (using #parse_sum for operands)\n"
                            "#parse_cmp|${[x < y <= z > v >= w]}|Parse compare expressions (non-equal) (using #parse_shift for operands)\n"
                            "#parse_cmpeq|${[x == y != z === a !== b is c in d !is e !in f]}|Parse compare expressions (equal) and in/is-expressions (using #parse_cmp for operands)\n"
                            "#parse_and|${[x & y]}|Parse bit-wise and expressions (using #parse_cmpeq for operands)\n"
                            "#parse_xor|${[x ^ y]}|Parse bit-wise xor expressions (using #parse_and for operands)\n"
                            "#parse_or|${[x | y]}|Parse bit-wise or expressions (using #parse_xor for operands)\n"
                            "#parse_as|${[x as y]}|Parse bit-wise or expressions (using #parse_or for operands)\n"
                            "#parse_land|${[x && y]}|Parse logical and expressions (using #parse_as for operands)\n"
                            "#parse_lor|${[x || y]}|Parse logical or expressions (using #parse_land for operands)\n"
                            "#parse_cond|${[x ? y : z]}, ${[x ? : z]}, ${[x ? y]}, ${[x ? y : ]}|Parse conditional expressions (using #parse_lor for `x', and #parse_cond for `y' and `z')\n"
                            "#parse_assign|${[x := y += z -= v += w /= a %= b <<= c >>= d &= e |= f ^= g **= h]}|Parse assignment expressions (using #parse_cond for operands)\n"
                            "#parse_expr|-|Alias for #parse_assign\n"
                            "#parse_unarytail|${foo[.bar]}, ${getitems[(a,b,c)]}|Parse the tail of a unary expression, including all tightly bound suffix expressions\n"
                            "#parse_prodtail|${x [* y / z % v ** w]}|Given a #parse_unary operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_sumtail|${x [+ y - z]}|Given a #parse_prod operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_shifttail|${x [<< y >> z]}|Given a #parse_sum operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_cmptail|${x [< y <= z > v >= w]}|Given a #parse_shift operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_cmpeqtail|${x [== y != z === a !== b is c in d !is e !in f]}|Given a #parse_cmp operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_andtail|${x [& y]}|Given a #parse_cmpeq operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_xortail|${x [^ y]}|Given a #parse_and operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_ortail|${x [| y]}|Given a #parse_xor operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_astail|${x [as y]}|Given a #parse_or operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_landtail|${x [&& y]}|Given a #parse_as operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_lortail|${x [|| y]}|Given a #parse_land operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_condtail|${x [? y : z]}, ${x [? : z]}, ${x [? y]}, ${x [? y : ]}|Given a #parse_lor operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_assigntail|${x [:= y += z -= v += w /= a %= b <<= c >>= d &= e |= f ^= g **= h]}|Given a #parse_cond operand `x', parse the suffix also parsed by #parse_prod\n"
                            "#parse_exprtail|${x [<any-non-unary-tail> y]}|Does the work of all the *-tail functions, excluding #{parse_unarytail}\n"
                            "#parse_maptail|${{x [: y, v: w]}}|Process the tail in a generic mapping expression. - No-op when the current token isn't a $\":\"\n"
                            "#parse_seqtail|${{x [, y, z]}}|Process the tail in a generic sequence expression. - No-op when the current token isn't a $\",\"\n"
                            "}\n"
                            "Note the processed portion of an expression is written in []-brackets\n"
                            "In order to explain what tail parsers do, look at this example:\n"
                            ">/* input: \"a.foo + 14 * 7 ? x : y-7\" */\n"
                            ">x = parser.parse_unaryhead();  /* x = `a' */\n"
                            ">x = parser.parse_unarytail(x); /* x = `a' + `.foo' (same as an initial call to `parse_unary') */\n"
                            ">x = parser.parse_exprtail(x);  /* x = `a.foo' + `+ 14 * 7 ? x : y-7' (same as an initial call to `parse_expr') */\n"
                            ),
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeCompilerWrapper_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                TYPE_ALLOCATOR(compiler_wrap_tp_alloc,compiler_wrap_tp_free)
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
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
    /* .tp_methods       = */parser_methods,
    /* .tp_getsets       = */parser_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */parser_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_IPARSER_C */
