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
#ifdef __INTELLISENSE__
#include "parser.c"
//#define JIT_SKIP  1
#define JIT_EVAL  1
#endif

#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/none.h>
#include <deemon/objmethod.h>
#include <deemon/instancemethod.h>
#include <deemon/super.h>
#include <deemon/dict.h>
#include <deemon/list.h>
#include <deemon/tuple.h>
#include <deemon/float.h>
#include <deemon/string.h>
#include <deemon/int.h>
#include <deemon/bool.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/error.h>

#include <hybrid/unaligned.h>

#if !defined(JIT_SKIP) && !defined(JIT_EVAL)
#error "Must either #define JIT_SKIP or JIT_EVAL before #including this file"
#endif

#undef ERROR
#undef F
#undef ISOK
#undef ISERR

#if 1
#define JITLEXER_EVAL_FSECONDARY   JITLEXER_EVAL_FNORMAL
#else
#define JITLEXER_EVAL_FSECONDARY  (flags & ~(JITLEXER_EVAL_FPRIMARY))
#endif

#ifdef JIT_SKIP
#define LOAD_LVALUE(obj,err) (void)0
#define LOAD_LVALUE_OR_RETURN_ERROR(obj) (void)0
#define IF_SKIP(...)      __VA_ARGS__
#define IF_EVAL(...)      /* nothing */
#define SYNTAXERROR(...) (DeeError_Throwf(&DeeError_SyntaxError,__VA_ARGS__), \
                          JITLexer_ErrorTrace(self,self->jl_tokstart),-1)
#define DECREF(x)        (void)0
#define DECREF_MAYBE_LVALUE(x) (void)0
#define ERROR             -1
#define RETURN_TYPE       int
#define LHS_OR_OK         0
#define ISOK(x)           likely(!(x))
#define ISERR(x)          unlikely(x)
#define CALL_PRIMARY(name)          JITLexer_Skip##name(self,flags)
#define CALL_PRIMARYF(name,flags)   JITLexer_Skip##name(self,flags)
#define CALL_SECONDARY(name,result) JITLexer_Skip##name(self,JITLEXER_EVAL_FSECONDARY)
#define DEFINE_PRIMARY(name) \
  INTERN int FCALL JITLexer_Skip##name(JITLexer *__restrict self, unsigned int flags)
#define DEFINE_SECONDARY(name) \
  INTERN int FCALL JITLexer_Skip##name(JITLexer *__restrict self, unsigned int flags)
#else
#define LOAD_LVALUE(obj,err)  \
     do{if((obj) == JIT_LVALUE && ((obj) = JITLexer_PackLValue(self)) == NULL) goto err;}__WHILE0
#define LOAD_LVALUE_OR_RETURN_ERROR(obj)  \
     do{if((obj) == JIT_LVALUE && ((obj) = JITLexer_PackLValue(self)) == NULL) return NULL;}__WHILE0
#define IF_SKIP(...)      /* nothing */
#define IF_EVAL(...)      __VA_ARGS__
#define SYNTAXERROR(...) (DeeError_Throwf(&DeeError_SyntaxError,__VA_ARGS__), \
                          JITLexer_ErrorTrace(self,self->jl_tokstart),NULL)
#define ERROR             NULL
#define RETURN_TYPE       DREF DeeObject *
#define DECREF(x)         Dee_Decref(x)
#define DECREF_MAYBE_LVALUE(x) ((x) == JIT_LVALUE || Dee_Decref(x))
#define LHS_OR_OK         lhs
#define ISOK(x)           likely(x)
#define ISERR(x)          unlikely(!(x))
#define CALL_PRIMARY(name)          JITLexer_Eval##name(self,flags)
#define CALL_PRIMARYF(name,flags)   JITLexer_Eval##name(self,flags)
#define CALL_SECONDARY(name,result) JITLexer_Eval##name(self,result,JITLEXER_EVAL_FSECONDARY)
#define DEFINE_PRIMARY(name) \
   INTERN DREF DeeObject *FCALL JITLexer_Eval##name(JITLexer *__restrict self, unsigned int flags)
#define DEFINE_SECONDARY(name) \
   INTERN DREF DeeObject *FCALL JITLexer_Eval##name(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags)
#endif




DECL_BEGIN

#ifndef ERROR_CANNOT_TEST_BINDING_DEFINED
#define ERROR_CANNOT_TEST_BINDING_DEFINED 1
INTERN ATTR_COLD int DCALL err_cannot_test_binding(void) {
 return DeeError_Throwf(&DeeError_SyntaxError,
                        "Cannot test binding of expression. "
                        "Expected a symbol, or attribute expression");
}
PRIVATE ATTR_COLD int DCALL
err_cannot_invoke_inplace(DeeObject *base, uint16_t opname) {
 DeeTypeObject *typetype;
 struct opinfo *info;
 if (!base)
  typetype = NULL;
 else {
  typetype = Dee_TYPE(base);
  if (typetype == &DeeSuper_Type)
      typetype = DeeSuper_TYPE(base);
  typetype = Dee_TYPE(typetype);
 }
 info = Dee_OperatorInfo(typetype,opname);
 if likely(info) {
  return DeeError_Throwf(&DeeError_TypeError,
                         "Cannot invoke inplace `operator %s' (`__%s__') without l-value",
                         info->oi_uname,info->oi_sname);
 } else {
  return DeeError_Throwf(&DeeError_TypeError,
                         "Cannot invoke inplace operator #%X without l-value",
                         opname);
 }
}
#endif

#ifndef TPPLIKE_STRING_TO_FLOAT_DEFINED
#define TPPLIKE_STRING_TO_FLOAT_DEFINED 1
PRIVATE DREF DeeObject *DCALL
JIT_atof(char const *__restrict start, size_t length) {
 double fltval = 0; char *iter,*end,ch;
 int numsys,more,float_extension_mult;
 iter = (char *)start;
 end  = (char *)start + length;
 if (*iter == '0') {
  if (++iter == end) goto done_zero;
  if (*iter == 'x' || *iter == 'X') {
   if (++iter == end) goto done_zero;
   numsys = 16;
  } else if (*iter == 'b' || *iter == 'B') {
   if (++iter == end) goto done_zero;
   numsys = 2;
  } else if (*iter == '.') {
   numsys = 10;
  } else {
   numsys = 8;
  }
 } else {
  numsys = 10;
 }
 float_extension_mult = 0;
 do {
  ch = *iter++;
  switch (ch) {
  case 'p': case 'P': goto flt_ext;
  case '0': case '1': case '2': case '3':
  case '4': case '5': case '6': case '7':
  case '8': case '9': more = ch-'0'; break;
  case 'e': if (numsys < (10+'e'-'a')) goto flt_ext; ATTR_FALLTHROUGH
  case 'a': case 'b': case 'c': case 'd': more = 10+ch-'a'; break;
  case 'E': if (numsys < (10+'E'-'A')) goto flt_ext; ATTR_FALLTHROUGH
  case 'A': case 'B': case 'C': case 'D': more = 10+ch-'A'; break;
  case 'f': case 'F': if (numsys < (10+'f'-'a')) goto sfx; more = 0xf; break;
  case '.': float_extension_mult = numsys; goto next;
  default: goto sfx;
  }
  if unlikely(more >= numsys)
     goto sfx;
  if (float_extension_mult != 0) {
   fltval += (double)more / (double)float_extension_mult;
   float_extension_mult *= numsys;
  } else {
   fltval = fltval*numsys+more;
  }
next:;
 } while (iter != end);
done:
 return DeeFloat_New(fltval);
flt_ext:
 /* Read the Float extension: E[+/-][int] */
#define float_extension_pos numsys
#define float_extension_off more
 float_extension_pos = 1;
 float_extension_off = 0;
 while (SKIP_WRAPLF(iter,end));
 if (iter == end) { --iter; goto sfx; }
 ch = *iter++;
 while (SKIP_WRAPLF(iter,end));
 if (ch == '-' || ch == '+') {
  float_extension_pos = (ch == '+');
  if (iter == end) { --iter; goto sfx; }
  ch = *iter++;
  while (SKIP_WRAPLF(iter,end));
 }
 while (ch >= '0' && ch <= '9') {
  float_extension_off = float_extension_off*10+(ch-'0');
  if (iter == end) break;
  ch = *iter++;
  while (SKIP_WRAPLF(iter,end));
 }
 float_extension_mult = 1;
 while (float_extension_off != 0) {
  float_extension_mult *= 10;
  --float_extension_off;
 }
 if (float_extension_pos) {
  fltval *= float_extension_mult;
 } else {
  fltval /= float_extension_mult;
 }
#undef float_extension_pos
#undef float_extension_off
sfx:
 if (iter != end)
     goto err_invalid_suffix;
 goto done;
err_invalid_suffix:
 --iter;
 DeeError_Throwf(&DeeError_SyntaxError,
                 "Invalid floating point number %$q",
                 length,start);
 return NULL;
done_zero:
 return DeeFloat_New(0.0);
}
#endif

#ifndef BUILTIN_IMPORT_DEFINED
#define BUILTIN_IMPORT_DEFINED 1
INTDEF DeeCMethodObject builtin_import;
#endif

DEFINE_PRIMARY(YieldAndParseUnaryKeywordOperand) {
 RETURN_TYPE result;
 (void)flags;
 JITLexer_Yield(self);
 if (self->jl_tok == '(') {
  result = CALL_PRIMARYF(UnaryHead,
                         JITLEXER_EVAL_FSECONDARY |
                         JITLEXER_EVAL_FDISALLOWCAST);
 } else {
  result = CALL_PRIMARYF(Unary,JITLEXER_EVAL_FSECONDARY);
 }
 return result;
}

DEFINE_SECONDARY(CastOperand) {
 RETURN_TYPE result;
 RETURN_TYPE merge;
 IF_EVAL(unsigned char *pos;)
 (void)flags;
 IF_EVAL(pos = self->jl_tokstart;)
 switch (self->jl_tok) {

 {
  JITSmallLexer smlex;
 case '!':
  /* Special handling required:
   * >> (int)!!!42;         // This...
   * >> (int)!!!in my_list; // ... vs. this
   * After parsing any number of additional `!' tokens, if the token
   * thereafter is the keyword `is' or `in', then this isn't a cast
   * expression. However if it isn't, then it is a cast expression. */
  memcpy(&smlex,self,sizeof(JITSmallLexer));
  for (;;) {
   JITLexer_Yield((JITLexer *)&smlex);
   if (smlex.jl_tok != '!') break;
  }
  if (!JITLexer_ISKWD(&smlex,"is") &&
      !JITLexer_ISKWD(&smlex,"in"))
       goto do_a_cast;
  /* This isn't a cast expression. */
  goto not_a_cast;
 }

 case '+': /* `(typexpr).operator add(castexpr)' vs. `(typexpr)castexpr.operator pos()' */
 case '-': /* `(typexpr).operator sub(castexpr)' vs. `(typexpr)castexpr.operator neg()' */
 case '<': /* `(typexpr).operator lo(castexpr)' vs. `(typexpr)(cell(castexpr))' */
 case '[': /* `(typexpr).operator [](castexpr)' vs. `(typexpr)(list(castexpr))' */
#if 0
  if (WARN(W_UNCLEAR_CAST_INTENT))
      goto err;
#endif
not_a_cast:
  /* Not a cast expression. */
  result = LHS_OR_OK; /* Inherit reference */
  break;

#if 1
 {
  bool second_paren;
  bool is_multiple;
 case '(':
  /* Special handling for the following cases:
   * >> (float)();              // Call with 0 arguments
   * >> (float)(42);            // Call with 1 argument `42'
   * >> (float)((42),);         // Call with 1 argument `42'
   * >> (float)(10,20,30);      // Call with 3 arguments `10,20,30'
   * >> (float)(pack 10,20,30); // Call with 1 argument `(10,20,30)'
   * Without this handling, the 4th line would be compiled as
   * `float(pack(10,20,30))', when we want it to be `float(10,20,30)' */
  LOAD_LVALUE(lhs,err);
  JITLexer_Yield(self);
  if (self->jl_tok == ')') {
   /* Handle case #0 */
   JITLexer_Yield(self);
#ifdef JIT_EVAL
   result = DeeObject_Call(lhs,0,NULL);
   if unlikely(!result)
      JITLexer_ErrorTrace(self,pos);
   Dee_Decref(lhs);
#else /* JIT_EVAL */
   result = 0;
#endif /* JIT_EVAL */
   goto done;
  }
  second_paren = self->jl_tok == '(';
  /* Parse the cast-expression / argument list. */
  merge = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
  if (ISERR(merge)) goto err_lhs;
  LOAD_LVALUE(merge,err_lhs);
  is_multiple = false;
  if (self->jl_tok == TOK_DOTS || self->jl_tok == ',') {
   is_multiple = true;
   merge = CALL_SECONDARY(CommaTupleOperand,merge);
   if (ISERR(merge)) goto err_lhs;
   LOAD_LVALUE(merge,err_lhs);
  }
#ifdef JIT_EVAL
  ASSERT(DeeTuple_Check(merge));
#endif
  if likely(self->jl_tok == ')') {
   JITLexer_Yield(self);
  } else {
   DeeError_Throwf(&DeeError_SyntaxError,
                   "Expected `)' after `(', but got `%$s'",
                  (size_t)(self->jl_tokend - self->jl_tokstart),
                   self->jl_tokstart);
   goto err_merge;
  }
  if (!second_paren && !is_multiple) {
   /* Recursively parse cast suffix expressions:
    * >> (int)(float)get_value(); 
    * Parse as:
    * >> int(float(get_value()));
    * Without this, it would be parsed as:
    * >> int(float)(get_value());
    */
#ifdef JIT_EVAL
   result = JITLexer_EvalCastOperand(self,
                                     DeeTuple_GET(merge,0),
                                     JITLEXER_EVAL_FSECONDARY);
#else /* JIT_EVAL */
   result = JITLexer_SkipCastOperand(self,
                                     JITLEXER_EVAL_FSECONDARY);
#endif /* JIT_EVAL */
   if (ISERR(result)) goto err_merge;
   LOAD_LVALUE(result,err_merge);
#ifdef JIT_EVAL
   if (result == DeeTuple_GET(merge,0)) {
    /* Same argument expression. */
    Dee_Decref(result);
   } else {
    /* New argument expression. */
    Dee_Decref(merge);
    merge = DeeTuple_Pack(1,result);
    if unlikely(!merge) { Dee_Decref(result); goto err_lhs; }
   }
#endif
  }
#ifdef JIT_EVAL
  result = DeeObject_CallTuple(lhs,merge);
  Dee_Decref(merge);
  Dee_Decref(lhs);
#else /* JIT_EVAL */
  result = 0;
#endif /* JIT_EVAL */
 } break;
#endif


 default:
  /* If what follows still isn't the start of
   * an expression, then this isn't a cast. */
  if (!maybe_expression_begin())
       goto not_a_cast;
do_a_cast:
  /* Actually do a cast. */
  LOAD_LVALUE(lhs,err);
  merge = CALL_PRIMARY(Unary);
  LOAD_LVALUE(merge,err_lhs);
#ifdef JIT_EVAL
  if (ISERR(merge)) goto err_lhs;
  result = DeeObject_Call(lhs,1,(DeeObject **)&merge);
  Dee_Decref(merge);
  Dee_Decref(lhs);
#else /* JIT_EVAL */
  result = 0;
#endif /* !JIT_EVAL */
  break;
 }
done:
 return result;
err_merge:
 DECREF(merge);
err_lhs:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
 return ERROR;
}



#ifndef DOT_KEY_ERRORS_DEFINED
#define DOT_KEY_ERRORS_DEFINED 1
PRIVATE ATTR_COLD int DCALL
err_expected_keyword_after_dot(JITLexer *__restrict self) {
 SYNTAXERROR("Expected keyword after `.' in brace initializer, but got `%$s'",
            (size_t)(self->jl_tokend - self->jl_tokstart),
             self->jl_tokstart);
 return -1;
}
PRIVATE ATTR_COLD int DCALL
err_expected_eq_after_dot(JITLexer *__restrict self) {
 SYNTAXERROR("Expected `=' after `.' in mapping-like brace initializer, but got `%$s'",
            (size_t)(self->jl_tokend - self->jl_tokstart),
                     self->jl_tokstart);
 return -1;
}
#endif


DEFINE_PRIMARY(UnaryHead) {
 RETURN_TYPE result;
 IF_EVAL(unsigned char *pos;)
 (void)flags;
 IF_EVAL(pos = self->jl_tokstart;)
#ifdef JIT_EVAL
 ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
#endif /* JIT_EVAL */
 switch (self->jl_tok) {

 case TOK_INT:
  /* Integer constant. */
#ifdef JIT_EVAL
  result = DeeInt_FromString((char const *)self->jl_tokstart,
                            (size_t)(self->jl_tokend - self->jl_tokstart),
                             DEEINT_STRING(0,DEEINT_STRING_FNORMAL));
#else /* JIT_EVAL */
  result = 0;
#endif /* !JIT_EVAL */
done_y1:
  JITLexer_Yield(self);
  goto done;
 case JIT_STRING:
  /* String literal */
#ifdef JIT_EVAL
  result = DeeString_FromBackslashEscaped((char const *)self->jl_tokstart + 1,
                                          (size_t)(self->jl_tokend - self->jl_tokstart) - 2,
                                           STRING_ERROR_FSTRICT);
#else /* JIT_EVAL */
  result = 0;
#endif /* !JIT_EVAL */
  goto done_y1;
 case JIT_RAWSTRING:
  /* Raw string literal */
#ifdef JIT_EVAL
  result = DeeString_NewUtf8((char const *)self->jl_tokstart + 2,
                             (size_t)(self->jl_tokend - self->jl_tokstart) - 3,
                              STRING_ERROR_FSTRICT);
#else /* JIT_EVAL */
  result = 0;
#endif /* !JIT_EVAL */
  goto done_y1;
 case TOK_FLOAT:
  /* Floating point constant */
#ifdef JIT_EVAL
  result = JIT_atof((char const *)self->jl_tokstart,
                    (size_t)(self->jl_tokend - self->jl_tokstart));
#else /* JIT_EVAL */
  result = 0;
#endif /* !JIT_EVAL */
  goto done_y1;


 case '#':
  JITLexer_Yield(self);
  result = CALL_PRIMARYF(Unary,JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
  if (ISERR(result)) goto done;
  LOAD_LVALUE(result,err);
  {
   DREF DeeObject *new_result;
   new_result = DeeObject_SizeObject(result);
   if unlikely(!new_result)
      goto err_r_invoke;
   Dee_Decref(result);
   result = new_result;
  }
#endif
  goto done;

 case '~':
  JITLexer_Yield(self);
  result = CALL_PRIMARYF(Unary,JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
  if (ISERR(result)) goto done;
  LOAD_LVALUE(result,err);
  {
   DREF DeeObject *new_result;
   new_result = DeeObject_Inv(result);
   if unlikely(!new_result)
      goto err_r_invoke;
   Dee_Decref(result);
   result = new_result;
  }
#endif
  goto done;

 case '+':
  JITLexer_Yield(self);
  result = CALL_PRIMARYF(Unary,JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
  if (ISERR(result)) goto done;
  LOAD_LVALUE(result,err);
  {
   DREF DeeObject *new_result;
   new_result = DeeObject_Pos(result);
   if unlikely(!new_result)
      goto err_r_invoke;
   Dee_Decref(result);
   result = new_result;
  }
#endif
  goto done;

 case '-':
  JITLexer_Yield(self);
  result = CALL_PRIMARYF(Unary,JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
  if (ISERR(result)) goto done;
  LOAD_LVALUE(result,err);
  {
   DREF DeeObject *new_result;
   new_result = DeeObject_Neg(result);
   if unlikely(!new_result)
      goto err_r_invoke;
   Dee_Decref(result);
   result = new_result;
  }
#endif
  goto done;

#if 0
 case TOK_INC:
 case TOK_DEC:
  /* TODO: Need a custom parser protocol! */
  goto done;
#endif

 case '!': /* not */
  JITLexer_Yield(self);
  result = CALL_PRIMARYF(Unary,JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
  if (ISERR(result)) goto done;
  LOAD_LVALUE(result,err);
  {
   int new_result;
   new_result = DeeObject_Bool(result);
   if unlikely(new_result < 0)
      goto err_r_invoke;
   Dee_Decref(result);
   result = DeeBool_For(!new_result);
   Dee_Incref(result);
  }
#endif
  goto done;


 {
  bool allow_cast;
 case '(':
  /* Parenthesis. */
  JITLexer_Yield(self);
  allow_cast = self->jl_tok != '(' && !(flags & JITLEXER_EVAL_FDISALLOWCAST);
#if 0
  if (self->jl_tok == '{') {
   unsigned int was_expression;
   /* Statements in expressions. */
   result = ast_parse_statement_or_braces(&was_expression);
   allow_cast = false; /* Don't allow braces, or statements as cast expressions. */
   if (self->jl_tok == ',' && was_expression != AST_PARSE_WASEXPR_NO) {
    DREF struct ast **tuple_branchv;
    if unlikely(!result) goto err;
    if unlikely(yield() < 0) goto err_r;
    if (self->jl_tok == ')') {
     /* single-element tuple expression, where the single element is a sequence. */
     tuple_branchv = (struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
     if unlikely(!tuple_branchv) goto err_r;
     tuple_branchv[0] = result; /* Inherit reference. */
     merge = ast_multiple(AST_FMULTIPLE_TUPLE,1,tuple_branchv);
     if unlikely(!merge) { Dee_Free(tuple_branchv); goto err_r; }
     result = merge;
    } else {
     /* There are more items! */
     merge = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
                             AST_FMULTIPLE_TUPLE,
                             NULL);
     if unlikely(!merge) goto err_r;
     ASSERT(merge->a_type == AST_MULTIPLE ||
           (merge->a_type == AST_CONSTEXPR && 
            merge->a_constexpr == Dee_EmptyTuple));
     if (merge->a_constexpr == Dee_EmptyTuple) {
      tuple_branchv = (struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
      if unlikely(!tuple_branchv) goto err_merge_r;
      Dee_DecrefNokill(merge->a_constexpr);
      merge->a_type = AST_MULTIPLE;
      merge->a_flag = AST_FMULTIPLE_TUPLE;
      merge->a_multiple.m_astc = 1;
     } else {
      tuple_branchv = (struct ast **)Dee_Realloc(merge->a_multiple.m_astv,
                                                (merge->a_multiple.m_astc + 1) *
                                                 sizeof(DREF struct ast *));
      if unlikely(!tuple_branchv) goto err_merge_r;
      MEMMOVE_PTR(tuple_branchv + 1,
                  tuple_branchv,
                  merge->a_multiple.m_astc);
      ++merge->a_multiple.m_astc;
     }
     tuple_branchv[0] = result; /* Inherit reference. */
     merge->a_multiple.m_astv = tuple_branchv;
     result = merge; /* Inherit reference. */
    }
   }
  } else
#endif
  if (self->jl_tok == ')') {
   /* Empty tuple. */
#ifdef JIT_EVAL
   result = Dee_EmptyTuple;
   Dee_Incref(Dee_EmptyTuple);
#else
   result = 0;
#endif
   allow_cast = false; /* Don't allow empty tuples for cast expressions. */
  } else {
   /* Parenthesis / tuple expression. */
   result = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
   LOAD_LVALUE(result,err);
   if (ISOK(result) && self->jl_tok == ',' || self->jl_tok == TOK_DOTS) {
    result = CALL_SECONDARY(CommaTupleOperand,result);
    allow_cast = false; /* Don't allow comma-lists for cast expressions. */
   }
  }
  if (ISERR(result))
      goto err;
  if likely(self->jl_tok == ')') {
   JITLexer_Yield(self);
  } else {
   SYNTAXERROR("Expected `)' after `(', but got `%$s'",
              (size_t)(self->jl_tokend - self->jl_tokstart),
                       self->jl_tokstart);
   goto err_r;
  }
  if (allow_cast) {
   /* C-style cast expression (only for single-parenthesis expressions) */
   result = CALL_SECONDARY(CastOperand,result);
  }
 } break;

 case '[':
  JITLexer_Yield(self);
  if (self->jl_tok == ']') {
   JITLexer_Yield(self);
   /* Empty list. */
#ifdef JIT_EVAL
   result = DeeList_New();
#else
   result = 0;
#endif
   break;
  }
  result = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
  if (ISERR(result)) goto err;
  LOAD_LVALUE(result,err);
  result = CALL_SECONDARY(CommaListOperand,result);
  if (ISERR(result)) goto err;
  LOAD_LVALUE(result,err);
  if likely(self->jl_tok == ']') {
   JITLexer_Yield(self);
  } else {
   SYNTAXERROR("Expected `]' after `[', but got `%$s'",
              (size_t)(self->jl_tokend - self->jl_tokstart),
                       self->jl_tokstart);
   goto err_r;
  }
  break;

 case '{':
  JITLexer_Yield(self);
  if (self->jl_tok == '}') {
   JITLexer_Yield(self);
   /* Empty sequence. */
#ifdef JIT_EVAL
   result = Dee_EmptySeq;
   Dee_Incref(Dee_EmptySeq);
#else
   result = 0;
#endif
   break;
  }
  if (self->jl_tok == '.') {
   IF_EVAL(DREF DeeObject *first_key;)
   JITLexer_Yield(self);
   if (self->jl_tok != JIT_KEYWORD) {
    err_expected_keyword_after_dot(self);
    goto err;
   }
#ifdef JIT_EVAL
   first_key = DeeString_NewUtf8((char const *)self->jl_tokstart,
                                 (size_t)(self->jl_tokend - self->jl_tokstart),
                                  STRING_ERROR_FSTRICT);
   if unlikely(!first_key) goto err;
#endif
   if (self->jl_tok != '=') {
    err_expected_eq_after_dot(self);
    goto err;
   }
   result = CALL_SECONDARY(CommaDictOperand,first_key);
  } else {
   /* Parse the initial key / sequence item. */
   result = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
   if (ISERR(result)) goto err;
   LOAD_LVALUE(result,err);
   if (self->jl_tok == ':') {
    /* Mapping-like dict expression. */
    result = CALL_SECONDARY(CommaDictOperand,result);
   } else {
    result = CALL_SECONDARY(CommaListOperand,result);
   }
  }
  if (ISERR(result)) goto err;
  if likely(self->jl_tok == '}') {
   JITLexer_Yield(self);
  } else {
   SYNTAXERROR("Expected `}' after `{', but got `%$s'",
              (size_t)(self->jl_tokend - self->jl_tokstart),
                       self->jl_tokstart);
   goto err_r;
  }
  break;

 {
  char const *tok_begin;
  size_t tok_length;
  uint32_t name;
 case JIT_KEYWORD:
  tok_begin  = (char *)self->jl_tokstart;
  tok_length = (size_t)((char *)self->jl_tokend - tok_begin);
  switch (tok_length) {

  case 2:
#if 0
   if (tok_begin[0] == 'i' &&
       tok_begin[1] == 'f') {
    /* TODO: if-in-expression */
   }
#endif
   break;


  case 3:
   if (tok_begin[0] == 's' &&
       tok_begin[1] == 't' &&
       tok_begin[2] == 'r') {
    result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
                           JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
    if (ISERR(result)) goto done;
    LOAD_LVALUE(result,err);
    {
     DREF DeeObject *new_result;
     new_result = DeeObject_Str(result);
     if unlikely(!new_result)
        goto err_r_invoke;
     Dee_Decref(result);
     result = new_result;
    }
#endif
    goto done;
   }
   break;

  case 4:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('n','o','n','e')) {
#ifdef JIT_EVAL
    result = Dee_None;
    Dee_Incref(Dee_None);
#else /* JIT_EVAL */
    result = 0;
#endif /* !JIT_EVAL */
    goto done_y1;
   }
   if (name == ENCODE4('t','r','u','e')) {
#ifdef JIT_EVAL
    result = Dee_True;
    Dee_Incref(Dee_True);
#else /* JIT_EVAL */
    result = 0;
#endif /* !JIT_EVAL */
    goto done_y1;
   }
   if (name == ENCODE4('r','e','p','r')) {
    result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
                           JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
    if (ISERR(result)) goto done;
    LOAD_LVALUE(result,err);
    {
     DREF DeeObject *new_result;
     new_result = DeeObject_Repr(result);
     if unlikely(!new_result)
        goto err_r_invoke;
     Dee_Decref(result);
     result = new_result;
    }
#endif
    goto done;
   }
   if (name == ENCODE4('c','o','p','y')) {
    result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
                           JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
    if (ISERR(result)) goto done;
    LOAD_LVALUE(result,err);
    {
     DREF DeeObject *new_result;
     new_result = DeeObject_Copy(result);
     if unlikely(!new_result)
        goto err_r_invoke;
     Dee_Decref(result);
     result = new_result;
    }
#endif
    goto done;
   }
   if (name == ENCODE4('t','y','p','e')) {
    result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
                           JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
    if (ISERR(result)) goto done;
    LOAD_LVALUE(result,err);
    {
     DREF DeeTypeObject *new_result;
     new_result = Dee_TYPE(result);
     Dee_Incref(new_result);
     Dee_Decref(result);
     result = (DREF DeeObject *)new_result;
    }
#endif
    goto done;
   }
#if 0
   if (name == ENCODE4('p','a','c','k')) {
    /* TODO: pack() expressions */
   }
#endif
   break;

  case 5:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('f','a','l','s') &&
       *(uint8_t *)(tok_begin + 4) == 'e') {
#ifdef JIT_EVAL
    result = Dee_False;
    Dee_Incref(Dee_False);
#else /* JIT_EVAL */
    result = 0;
#endif /* !JIT_EVAL */
    goto done_y1;
   }
#if 0
   if (name == ENCODE4('b','o','u','n') &&
       *(uint8_t *)(tok_begin + 4) == 'd') {
    /* TODO: Need a custom parser protocol! */
   }
#endif
   break;

  case 6:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('i','m','p','o') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('r','t')) {
#ifdef JIT_EVAL
    result = (DREF DeeObject *)&builtin_import;
    Dee_Incref(&builtin_import);
#else /* JIT_EVAL */
    result = 0;
#endif /* !JIT_EVAL */
    goto done_y1;
   }
#if 0
   if (name == ENCODE4('a','s','s','e') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('r','t')) {
    /* TODO: Assert expressions */
   }
#endif
   break;


  case 8:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('d','e','e','p') &&
       UNALIGNED_GET32((uint32_t *)(tok_begin + 4)) == ENCODE4('c','o','p','y')) {
    result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
                           JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
    if (ISERR(result)) goto done;
    LOAD_LVALUE(result,err);
    {
     DREF DeeObject *new_result;
     new_result = DeeObject_DeepCopy(result);
     if unlikely(!new_result)
        goto err_r_invoke;
     Dee_Decref(result);
     result = new_result;
    }
#endif
    goto done;
   }
   break;
  default: break;
  }
  /* Fallback: identifier lookup / <x from y> expression. */
  {
#ifdef JIT_EVAL
   char const *symbol_name = (char const *)self->jl_tokstart;
   size_t symbol_size = (size_t)(self->jl_tokend - self->jl_tokstart);
   JITLexer_Yield(self);
   if (JITLexer_ISKWD(self,"from")) {
    DREF DeeModuleObject *module;
    struct module_symbol *symbol;
    JITLexer_Yield(self);
    module = (DREF DeeModuleObject *)JITLexer_EvalModule(self);
    if unlikely(!module)
       goto err;
    symbol = DeeModule_GetSymbolStringLen(module,
                                          symbol_name,
                                          symbol_size,
                                          hash_ptr(symbol_name,symbol_size));
    if unlikely(!symbol) {
     DeeError_Throwf(&DeeError_SymbolError,
                     "Symbol `%$s' could not be found in module `%k'",
                     symbol_size,symbol_name,module);
     Dee_Decref(module);
     goto err;
    }
    self->jl_lvalue.lv_kind = JIT_LVALUE_EXTERN;
    self->jl_lvalue.lv_extern.lx_mod = module; /* Inherit reference. */
    self->jl_lvalue.lv_extern.lx_sym = symbol;
   } else {
    if (!JITContext_Lookup(self->jl_context,
                          (JITSymbol *)&self->jl_lvalue,
                           symbol_name,
                           symbol_size)) {
     DeeError_Throwf(&DeeError_SymbolError,
                     "Unknown variable `%$s'",
                     symbol_size,symbol_name);
     goto err;
    }
   }
   result = JIT_LVALUE;
#else /* JIT_EVAL */
   result = 0;
   JITLexer_Yield(self);
   if (JITLexer_ISKWD(self,"from")) {
    JITLexer_Yield(self);
    if unlikely(JITLexer_SkipModule(self))
       goto err;
   }
#endif /* !JIT_EVAL */
  }
 } break;

 default:
  result = SYNTAXERROR("Unexpected token `%$s' in expression",
                      (size_t)(self->jl_tokend - self->jl_tokstart),
                       self->jl_tokstart);
  break;
 }
done:
 return result;
#ifdef JIT_EVAL
err_r_invoke:
 JITLexer_ErrorTrace(self,pos);
#endif
err_r:
 DECREF_MAYBE_LVALUE(result);
err:
 return ERROR;
}








DEFINE_PRIMARY(Unary) {
 RETURN_TYPE result;
 RETURN_TYPE rhs;
 IF_EVAL(unsigned char *pos;)
 result = CALL_PRIMARY(UnaryHead);
 if (ISOK(result)) for (;;) {
  switch (self->jl_tok) {

  case '.':
   IF_EVAL(pos = self->jl_tokstart;)
   /* Attribute lookup. */
   JITLexer_Yield(self);
   if (self->jl_tok != JIT_KEYWORD) {
    SYNTAXERROR("Expected a keyword after `.', but got `%$s'",
               (size_t)(self->jl_tokend - self->jl_tokstart),
                        self->jl_tokstart);
    goto err_r;
   }
   if (JITLexer_ISTOK(self,"operator")) {
    int32_t opno;
    JITLexer_Yield(self);
    opno = JITLexer_ParseOperatorName(self,P_OPERATOR_FNORMAL);
    if unlikely(opno < 0) goto err_r;
#ifdef JIT_EVAL
    {
     DREF DeeObject *opfun;
     LOAD_LVALUE(result,err);
     opfun = JIT_GetOperatorFunction((uint16_t)opno);
     if unlikely(!opfun) goto err_r;
     rhs = DeeInstanceMethod_New(opfun,result);
     Dee_Decref(opfun);
     if unlikely(!rhs) goto err_r_invoke;
     Dee_Decref(result);
     result = rhs;
    }
#endif
   } else if (JITLexer_ISTOK(self,"this")) {
    JITLexer_Yield(self);
   } else if (JITLexer_ISTOK(self,"class")) {
    JITLexer_Yield(self);
#ifdef JIT_EVAL
    LOAD_LVALUE(result,err);
    rhs = (DeeObject *)DeeObject_Class(result);
    Dee_Incref(rhs);
    Dee_Decref(result);
    result = rhs;
#endif /* JIT_EVAL */
   } else if (JITLexer_ISTOK(self,"super")) {
    JITLexer_Yield(self);
#ifdef JIT_EVAL
    LOAD_LVALUE(result,err);
    rhs = DeeSuper_Of(result);
    if unlikely(!rhs) goto err_r_invoke;
    Dee_Decref(result);
    result = rhs;
#endif /* JIT_EVAL */
   } else {
#ifdef JIT_EVAL
    char const *attr_name;
    size_t attr_size;
    LOAD_LVALUE(result,err);
    /* Generic attribute lookup. */
    attr_name = (char const *)self->jl_tokstart;
    attr_size = (size_t)(self->jl_tokend - self->jl_tokstart);
#endif /* JIT_EVAL */
    JITLexer_Yield(self);
#ifdef JIT_EVAL
#ifndef __OPTIMIZE_SIZE__
    /* Optimization for callattr expressions. */
    if (self->jl_tok == '(') {
     DREF DeeObject *attr_name_ob;
     DREF DeeObject *args;
     JITLexer_Yield(self);
     if (self->jl_tok == ')') {
      JITLexer_Yield(self);
      args = Dee_EmptyTuple;
      Dee_Incref(Dee_EmptyTuple);
     } else {
      /* TODO: Keyword arguments! */
      args = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
      if (ISERR(args)) goto err_r;
      LOAD_LVALUE(args,err_r);
      args = CALL_SECONDARY(CommaTupleOperand,args);
      if (ISERR(args)) goto err_r;
      LOAD_LVALUE(args,err_r);
      if (self->jl_tok == ')') {
       JITLexer_Yield(self);
      } else {
       SYNTAXERROR("Expected `)' to end call operation, but got `%$s'",
                  (size_t)(self->jl_tokend - self->jl_tokstart),
                           self->jl_tokstart);
       Dee_Decref(args);
       goto err_r;
      }
     }
     attr_name_ob = DeeString_NewUtf8(attr_name,attr_size,STRING_ERROR_FSTRICT);
     if unlikely(!attr_name_ob) goto err_r_invoke;
     rhs = DeeObject_CallAttrTuple(result,
                                   attr_name_ob,
                                   args);
     Dee_Decref(attr_name_ob);
     Dee_Decref(args);
     if unlikely(!rhs)
        goto err_r_invoke;
     Dee_Decref(result);
     result = rhs;
    } else
#endif
    {
     /* Generic attribute lookup */
     ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
     self->jl_lvalue.lv_kind = JIT_LVALUE_ATTR_STR;
     self->jl_lvalue.lv_attr_str.la_base = result; /* Inherit reference. */
     self->jl_lvalue.lv_attr_str.la_name = attr_name;
     self->jl_lvalue.lv_attr_str.la_nsiz = attr_size;
     result = JIT_LVALUE;
    }
#endif /* JIT_EVAL */
   }
   break;

#if 0 /* TODO */
  case '[': /* sequence operator */
   IF_EVAL(pos = self->jl_tokstart;)
   JITLexer_Yield();
   break;
#endif

  /* TODO: pack */


  {
  case '(': /* Call operator */
   IF_EVAL(pos = self->jl_tokstart;)
   JITLexer_Yield(self);
   LOAD_LVALUE(result,err);
   if (self->jl_tok == ')') {
    JITLexer_Yield(self);
#ifdef JIT_EVAL
    rhs = Dee_EmptyTuple;
    Dee_Incref(Dee_EmptyTuple);
#endif
   } else {
    /* TODO: Keyword arguments! */
    rhs = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
    if (ISERR(rhs)) goto err_r;
    LOAD_LVALUE(rhs,err_r);
    rhs = CALL_SECONDARY(CommaTupleOperand,rhs);
    if (ISERR(rhs)) goto err_r;
    LOAD_LVALUE(rhs,err_r);
    if (self->jl_tok == ')') {
     JITLexer_Yield(self);
    } else {
     SYNTAXERROR("Expected `)' to end call operation, but got `%$s'",
                (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
     DECREF(rhs);
     goto err_r;
    }
   }
#ifdef JIT_EVAL
   {
    DREF DeeObject *call_result;
    call_result = DeeObject_CallTuple(result,rhs);
    Dee_Decref(rhs);
    if unlikely(!call_result)
       goto err_r_invoke;
    Dee_Decref(result);
    result = call_result;
   }
#endif
  } break;

  default:
   goto done;
  }
 }
done:
 return result;
#ifdef JIT_EVAL
err_r_invoke:
 JITLexer_ErrorTrace(self,pos);
#endif
err_r:
 DECREF_MAYBE_LVALUE(result);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_PRIMARY(Prod) {
 RETURN_TYPE result = CALL_PRIMARY(Unary);
 if (ISOK(result) && TOKEN_IS_PROD(self->jl_tok))
     result = CALL_SECONDARY(ProdOperand,result);
 return result;
}

DEFINE_PRIMARY(Sum) {
 RETURN_TYPE result = CALL_PRIMARY(Prod);
 if (ISOK(result) && TOKEN_IS_SUM(self->jl_tok))
     result = CALL_SECONDARY(SumOperand,result);
 return result;
}

DEFINE_PRIMARY(Shift) {
 RETURN_TYPE result = CALL_PRIMARY(Sum);
 if (ISOK(result) && TOKEN_IS_SHIFT(self->jl_tok))
     result = CALL_SECONDARY(ShiftOperand,result);
 return result;
}

DEFINE_PRIMARY(Cmp) {
 RETURN_TYPE result = CALL_PRIMARY(Shift);
 if (ISOK(result) && TOKEN_IS_CMP(self->jl_tok))
     result = CALL_SECONDARY(CmpOperand,result);
 return result;
}

DEFINE_PRIMARY(CmpEQ) {
 RETURN_TYPE result = CALL_PRIMARYF(Cmp,flags | JITLEXER_EVAL_FALLOWISBOUND);
 if (ISOK(result) && TOKEN_IS_CMPEQ(self))
     result = CALL_SECONDARY(CmpEQOperand,result);
 return result;
}

DEFINE_PRIMARY(And) {
 RETURN_TYPE result = CALL_PRIMARY(CmpEQ);
 if (ISOK(result) && TOKEN_IS_AND(self->jl_tok))
     result = CALL_SECONDARY(AndOperand,result);
 return result;
}

DEFINE_PRIMARY(Xor) {
 RETURN_TYPE result = CALL_PRIMARY(And);
 if (ISOK(result) && TOKEN_IS_XOR(self->jl_tok))
     result = CALL_SECONDARY(XorOperand,result);
 return result;
}

DEFINE_PRIMARY(Or) {
 RETURN_TYPE result = CALL_PRIMARY(Xor);
 if (ISOK(result) && TOKEN_IS_OR(self->jl_tok))
     result = CALL_SECONDARY(OrOperand,result);
 return result;
}

DEFINE_PRIMARY(As) {
 RETURN_TYPE result = CALL_PRIMARY(Or);
 if (ISOK(result) && TOKEN_IS_AS(self))
     result = CALL_SECONDARY(AsOperand,result);
 return result;
}

DEFINE_PRIMARY(Land) {
 RETURN_TYPE result = CALL_PRIMARY(As);
 if (ISOK(result) && TOKEN_IS_LAND(self->jl_tok))
     result = CALL_SECONDARY(LandOperand,result);
 return result;
}

DEFINE_PRIMARY(Lor) {
 RETURN_TYPE result = CALL_PRIMARY(Land);
 if (ISOK(result) && TOKEN_IS_LOR(self->jl_tok))
     result = CALL_SECONDARY(LorOperand,result);
 return result;
}

DEFINE_PRIMARY(Cond) {
 RETURN_TYPE result = CALL_PRIMARY(Lor);
 if (ISOK(result) && TOKEN_IS_COND(self->jl_tok))
     result = CALL_SECONDARY(CondOperand,result);
 return result;
}

DEFINE_PRIMARY(Assign) {
#ifdef __OPTIMIZE_SIZE__
 RETURN_TYPE result;
 result = CALL_PRIMARYF(Cond,flags | JITLEXER_EVAL_FALLOWINPLACE);
 if (ISOK(result) && TOKEN_IS_ASSIGN(self->jl_tok))
     result = CALL_SECONDARY(AssignOperand,result);
 return result;
#elif 1
 RETURN_TYPE result;
 result = CALL_PRIMARYF(Unary,flags |
                        JITLEXER_EVAL_FALLOWINPLACE |
                        JITLEXER_EVAL_FALLOWISBOUND);
 if (ISERR(result)) goto done;
 switch (self->jl_tok) {

 case JIT_KEYWORD:
  if (JITLexer_ISTOK(self,"is"))
      goto case_cmpeq;
  if (JITLexer_ISTOK(self,"in"))
      goto case_cmpeq;
  if (JITLexer_ISTOK(self,"as"))
      goto case_as;
  break;

 CASE_TOKEN_IS_PROD:
  result = CALL_SECONDARY(ProdOperand,result);
  if (ISERR(result)) goto done;
  if (TOKEN_IS_SUM(self->jl_tok)) {
 CASE_TOKEN_IS_SUM:
   result = CALL_SECONDARY(SumOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_SHIFT(self->jl_tok)) {
 CASE_TOKEN_IS_SHIFT:
   result = CALL_SECONDARY(ShiftOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_CMP(self->jl_tok)) {
 CASE_TOKEN_IS_CMP:
   result = CALL_SECONDARY(CmpOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_CMPEQ(self)) {
 CASE_TOKEN_IS_CMPEQ:
case_cmpeq:
   result = CALL_SECONDARY(CmpEQOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_AND(self->jl_tok)) {
 CASE_TOKEN_IS_AND:
   result = CALL_SECONDARY(AndOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_XOR(self->jl_tok)) {
 CASE_TOKEN_IS_XOR:
   result = CALL_SECONDARY(XorOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_OR(self->jl_tok)) {
 CASE_TOKEN_IS_OR:
   result = CALL_SECONDARY(OrOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_AS(self)) {
 CASE_TOKEN_IS_AS:
case_as:
   result = CALL_SECONDARY(AsOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_LAND(self->jl_tok)) {
 CASE_TOKEN_IS_LAND:
   result = CALL_SECONDARY(LandOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_LOR(self->jl_tok)) {
 CASE_TOKEN_IS_LOR:
   result = CALL_SECONDARY(LorOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_COND(self->jl_tok)) {
 CASE_TOKEN_IS_COND:
   result = CALL_SECONDARY(CondOperand,result);
   if (ISERR(result)) goto done;
  }
  if (TOKEN_IS_ASSIGN(self->jl_tok)) {
 CASE_TOKEN_IS_ASSIGN:
   result = CALL_SECONDARY(AssignOperand,result);
   if (ISERR(result)) goto done;
  }
  break;
 default: break;
 }
done:
 return result;
#endif
}

/* With the current token one of the unary operator symbols, consume
* it and parse the second operand before returning the combination */
DEFINE_SECONDARY(ProdOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 IF_EVAL(unsigned int cmd = self->jl_tok;)
 ASSERT(TOKEN_IS_PROD(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  rhs = CALL_PRIMARY(Unary);
  if (ISERR(rhs)) goto err_r;
  LOAD_LVALUE(rhs,err_r);
#ifdef JIT_EVAL
  switch (cmd) {
  case '*':
   merge = DeeObject_Mul(lhs,rhs);
   break;
  case '/':
   merge = DeeObject_Div(lhs,rhs);
   break;
  case '%':
   merge = DeeObject_Mod(lhs,rhs);
   break;
  case TOK_POW:
   merge = DeeObject_Pow(lhs,rhs);
   break;
  default: __builtin_unreachable();
  }
  if unlikely(!merge)
     goto err_invoke;
  Dee_Decref(rhs);
  Dee_Decref(lhs);
  lhs = merge;
#endif
  if (!TOKEN_IS_PROD(self->jl_tok))
      break;
  IF_EVAL(cmd = self->jl_tok;)
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 JITLexer_ErrorTrace(self,pos);
 DECREF(rhs);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(SumOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 unsigned int cmd = self->jl_tok;
 ASSERT(TOKEN_IS_SUM(cmd));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  if (self->jl_tok == TOK_DOTS && cmd == '+') {
   JITLexer_Yield(self);
#ifdef JIT_EVAL
   merge = DeeSeq_Sum(lhs);
   if unlikely(!merge)
      goto err_invoke_norhs;
   Dee_Decref(lhs);
   lhs = merge;
#endif
  } else {
   rhs = CALL_PRIMARY(Prod);
   if (ISERR(rhs)) goto err_r;
   LOAD_LVALUE(rhs,err_r);
#ifdef JIT_EVAL
   switch (cmd) {
   case '+':
    merge = DeeObject_Add(lhs,rhs);
    break;
   case '-':
    merge = DeeObject_Sub(lhs,rhs);
    break;
   default: __builtin_unreachable();
   }
   if unlikely(!merge)
      goto err_invoke;
   Dee_Decref(rhs);
   Dee_Decref(lhs);
   lhs = merge;
#endif
  }
  cmd = self->jl_tok;
  if (!TOKEN_IS_SUM(cmd))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 DECREF(rhs);
err_invoke_norhs:
 JITLexer_ErrorTrace(self,pos);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(ShiftOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 IF_EVAL(unsigned int cmd = self->jl_tok;)
 ASSERT(TOKEN_IS_SHIFT(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  rhs = CALL_PRIMARY(Sum);
  if (ISERR(rhs)) goto err_r;
  LOAD_LVALUE(rhs,err_r);
#ifdef JIT_EVAL
  switch (cmd) {
  case TOK_SHL:
   merge = DeeObject_Shl(lhs,rhs);
   break;
  case TOK_SHR:
   merge = DeeObject_Shr(lhs,rhs);
   break;
  default: __builtin_unreachable();
  }
  if unlikely(!merge)
     goto err_invoke;
  Dee_Decref(rhs);
  Dee_Decref(lhs);
  lhs = merge;
#endif
  if (!TOKEN_IS_SHIFT(self->jl_tok))
      break;
  IF_EVAL(cmd = self->jl_tok;)
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 JITLexer_ErrorTrace(self,pos);
 DECREF(rhs);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(CmpOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 unsigned int cmd = self->jl_tok;
 ASSERT(TOKEN_IS_CMP(cmd));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  if (self->jl_tok == TOK_DOTS && (cmd == '<' || cmd == '>')) {
   JITLexer_Yield(self);
#ifdef JIT_EVAL
   merge = cmd == '<'
         ? DeeSeq_Min(lhs,NULL)
         : DeeSeq_Max(lhs,NULL);
   if unlikely(!merge)
      goto err_invoke_norhs;
   Dee_Decref(lhs);
   lhs = merge;
#endif
  } else {
   rhs = CALL_PRIMARY(Shift);
   if (ISERR(rhs)) goto err_r;
   LOAD_LVALUE(rhs,err_r);
#ifdef JIT_EVAL
   switch (cmd) {
   case TOK_LOWER:
    merge = DeeObject_CompareLoObject(lhs,rhs);
    break;
   case TOK_LOWER_EQUAL:
    merge = DeeObject_CompareLeObject(lhs,rhs);
    break;
   case TOK_GREATER:
    merge = DeeObject_CompareGrObject(lhs,rhs);
    break;
   case TOK_GREATER_EQUAL:
    merge = DeeObject_CompareGeObject(lhs,rhs);
    break;
   default: __builtin_unreachable();
   }
   if unlikely(!merge)
      goto err_invoke;
   Dee_Decref(rhs);
   Dee_Decref(lhs);
   lhs = merge;
#endif
  }
  cmd = self->jl_tok;
  if (!TOKEN_IS_CMP(cmd))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 DECREF(rhs);
err_invoke_norhs:
 JITLexer_ErrorTrace(self,pos);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(CmpEQOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 IF_EVAL(unsigned int cmd = self->jl_tok;)
 ASSERT(TOKEN_IS_CMPEQ(self));
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  if (self->jl_tok == '!') {
   bool inverted = false;
   do JITLexer_Yield(self),
      inverted = !inverted;
   while (self->jl_tok == '!');
   if (JITLexer_ISKWD(self,"is")) {
    JITLexer_Yield(self);
    if (JITLexer_ISKWD(self,"bound")) {
     /* Suffix expression */
#ifdef JIT_EVAL
     int error;
     if (lhs != JIT_LVALUE) {
      err_cannot_test_binding();
      goto err_r;
     }
     error = JITLValue_IsBound(&self->jl_lvalue,self->jl_context);
     JITLValue_Fini(&self->jl_lvalue);
     self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
     if unlikely(error < 0) goto err;
     lhs = DeeBool_For((error != 0) ^ inverted);
     Dee_Incref(lhs);
#endif
     JITLexer_Yield(self);
     goto continue_expr;
    }
    LOAD_LVALUE(lhs,err);
    rhs = CALL_PRIMARYF(Cmp,flags | JITLEXER_EVAL_FALLOWISBOUND);
    if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
    LOAD_LVALUE(rhs,err_r);
    {
     bool is_instance;
     if (DeeNone_Check(rhs)) {
      is_instance = DeeNone_Check(lhs);
     } else if (DeeSuper_Check(lhs)) {
      is_instance = DeeType_IsInherited(DeeSuper_TYPE(lhs),(DeeTypeObject *)rhs);
     } else {
      is_instance = DeeObject_InstanceOf(lhs,(DeeTypeObject *)rhs);
     }
     Dee_Decref(rhs);
     Dee_Decref(lhs);
     lhs = DeeBool_For(inverted ^ is_instance);
     Dee_Incref(lhs);
    }
#endif
    goto continue_expr;
   }
   LOAD_LVALUE(lhs,err);
   if (JITLexer_ISKWD(self,"in")) {
    JITLexer_Yield(self);
    rhs = CALL_PRIMARYF(Cmp,flags | JITLEXER_EVAL_FALLOWISBOUND);
    if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
    LOAD_LVALUE(rhs,err_r);
    {
     merge = DeeObject_ContainsObject(rhs,lhs);
     if unlikely(!merge)
        goto err_rhs;
     Dee_Decref(rhs);
     Dee_Decref(lhs);
     lhs = merge;
     if (inverted) {
      int b = DeeObject_Bool(lhs);
      if unlikely(b < 0)
         goto err_r;
      Dee_Decref(lhs);
      lhs = DeeBool_For(!b);
      Dee_Incref(lhs);
     }
    }
#endif
    goto continue_expr;
   }
   SYNTAXERROR("Expected `is' or `in' after `!', but got `%$s'",
              (size_t)(self->jl_tokend - self->jl_tokstart),
                       self->jl_tokstart);
   goto err_r;
  }
  if (JITLexer_ISKWD(self,"is")) {
   JITLexer_Yield(self);
   if (JITLexer_ISKWD(self,"bound")) {
    JITLexer_Yield(self);
#ifdef JIT_EVAL
    {
     int error;
     if (lhs != JIT_LVALUE) {
      err_cannot_test_binding();
      goto err_r;
     }
     error = JITLValue_IsBound(&self->jl_lvalue,self->jl_context);
     JITLValue_Fini(&self->jl_lvalue);
     self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
     if unlikely(error < 0) goto err;
     lhs = DeeBool_For(error);
     Dee_Incref(lhs);
    }
#endif
   } else {
    LOAD_LVALUE(lhs,err);
    rhs = CALL_PRIMARYF(Cmp,flags | JITLEXER_EVAL_FALLOWISBOUND);
    if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
    LOAD_LVALUE(rhs,err_r);
    {
     bool is_instance;
     if (DeeNone_Check(rhs)) {
      is_instance = DeeNone_Check(lhs);
     } else if (DeeSuper_Check(lhs)) {
      is_instance = DeeType_IsInherited(DeeSuper_TYPE(lhs),(DeeTypeObject *)rhs);
     } else {
      is_instance = DeeObject_InstanceOf(lhs,(DeeTypeObject *)rhs);
     }
     merge = DeeBool_For(is_instance);
     Dee_Incref(merge);
    }
#endif /* JIT_EVAL */
   }
   goto continue_expr;
  } else {
   JITLexer_Yield(self);
  }
  LOAD_LVALUE(lhs,err);
  rhs = CALL_PRIMARYF(Cmp,flags | JITLEXER_EVAL_FALLOWISBOUND);
  if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
  LOAD_LVALUE(rhs,err_r);
  switch (cmd) {
  case TOK_EQUAL:
   merge = DeeObject_CompareEqObject(lhs,rhs);
   break;
  case TOK_NOT_EQUAL:
   merge = DeeObject_CompareNeObject(lhs,rhs);
   break;
  case TOK_EQUAL3:
   merge = DeeBool_For(lhs == rhs);
   Dee_Incref(merge);
   break;
  case TOK_NOT_EQUAL3:
   merge = DeeBool_For(lhs != rhs);
   Dee_Incref(merge);
   break;
  case JIT_KEYWORD:
   merge = DeeObject_ContainsObject(rhs,lhs);
   break;
  default: __builtin_unreachable();
  }
  if unlikely(!merge)
     goto err_invoke;
  Dee_Decref(rhs);
  Dee_Decref(lhs);
  lhs = merge;
#endif
continue_expr:
  if (!TOKEN_IS_CMPEQ(self))
      break;
  IF_EVAL(cmd = self->jl_tok;)
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 JITLexer_ErrorTrace(self,pos);
err_rhs:
 DECREF(rhs);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
 return ERROR;
}

DEFINE_SECONDARY(AndOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 ASSERT(TOKEN_IS_AND(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  rhs = CALL_PRIMARY(CmpEQ);
  if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
  LOAD_LVALUE(rhs,err_r);
  merge = DeeObject_And(lhs,rhs);
  if unlikely(!merge)
     goto err_invoke;
  Dee_Decref(rhs);
  Dee_Decref(lhs);
  lhs = merge;
#endif
  if (!TOKEN_IS_AND(self->jl_tok))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 JITLexer_ErrorTrace(self,pos);
 DECREF(rhs);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(XorOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 ASSERT(TOKEN_IS_XOR(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  rhs = CALL_PRIMARY(And);
  if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
  LOAD_LVALUE(rhs,err_r);
  merge = DeeObject_Xor(lhs,rhs);
  if unlikely(!merge)
     goto err_invoke;
  Dee_Decref(rhs);
  Dee_Decref(lhs);
  lhs = merge;
#endif
  if (!TOKEN_IS_XOR(self->jl_tok))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 JITLexer_ErrorTrace(self,pos);
 DECREF(rhs);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(OrOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 ASSERT(TOKEN_IS_OR(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  rhs = CALL_PRIMARY(Xor);
  if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
  LOAD_LVALUE(rhs,err_r);
  merge = DeeObject_Or(lhs,rhs);
  if unlikely(!merge)
     goto err_invoke;
  Dee_Decref(rhs);
  Dee_Decref(lhs);
  lhs = merge;
#endif
  if (!TOKEN_IS_OR(self->jl_tok))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 JITLexer_ErrorTrace(self,pos);
 DECREF(rhs);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(AsOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(DREF DeeObject *merge;)
 IF_EVAL(unsigned char *pos;)
 ASSERT(TOKEN_IS_AS(self));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  rhs = CALL_PRIMARY(Or);
  if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
  LOAD_LVALUE(rhs,err_r);
  merge = DeeSuper_New((DeeTypeObject *)rhs,lhs);
  if unlikely(!merge)
     goto err_invoke;
  Dee_Decref(rhs);
  Dee_Decref(lhs);
  lhs = merge;
#endif
  if (!TOKEN_IS_AS(self))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 JITLexer_ErrorTrace(self,pos);
 DECREF(rhs);
#endif
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(LandOperand) {
#ifndef JIT_EVAL
 RETURN_TYPE rhs;
#endif
 IF_EVAL(unsigned char *pos;)
 ASSERT(TOKEN_IS_LAND(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  if (self->jl_tok == TOK_DOTS) {
   JITLexer_Yield(self);
#ifdef JIT_EVAL
   {
    int b = DeeSeq_All(lhs);
    if (b < 0) goto err_invoke_norhs;
    Dee_Decref(lhs);
    lhs = DeeBool_For(b);
    Dee_Incref(lhs);
   }
#endif
   goto continue_expr;
  }
#ifdef JIT_EVAL
  {
   int b = DeeObject_Bool(lhs);
   if (b < 0) goto err_invoke_norhs;
   Dee_Decref(lhs);
   if (b) {
    lhs = CALL_PRIMARY(As);
    if (ISERR(lhs)) goto err;
    LOAD_LVALUE(lhs,err);
    b = DeeObject_Bool(lhs);
    if (b < 0) goto err_invoke_norhs;
    Dee_Decref(lhs);
    lhs = DeeBool_For(b);
    Dee_Incref(lhs);
   } else {
    if unlikely(JITLexer_SkipAs(self,flags))
       goto err;
    lhs = Dee_False;
    Dee_Incref(Dee_False);
   }
  }
#else
  rhs = CALL_PRIMARY(As);
  if (ISERR(rhs)) goto err_r;
#endif
continue_expr:
  if (!TOKEN_IS_LAND(self->jl_tok))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
/*err_invoke:*/
 /*DECREF(rhs);*/
err_invoke_norhs:
 JITLexer_ErrorTrace(self,pos);
#else
err_r:
#endif
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(LorOperand) {
#ifndef JIT_EVAL
 RETURN_TYPE rhs;
#endif
 IF_EVAL(unsigned char *pos;)
 ASSERT(TOKEN_IS_LOR(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
  if (self->jl_tok == TOK_DOTS) {
   JITLexer_Yield(self);
#ifdef JIT_EVAL
   {
    int b = DeeSeq_Any(lhs);
    if (b < 0) goto err_invoke_norhs;
    Dee_Decref(lhs);
    lhs = DeeBool_For(b);
    Dee_Incref(lhs);
   }
#endif
   goto continue_expr;
  }
#ifdef JIT_EVAL
  {
   int b = DeeObject_Bool(lhs);
   if (b < 0) goto err_invoke_norhs;
   Dee_Decref(lhs);
   if (b) {
    if unlikely(JITLexer_SkipLand(self,flags))
       goto err;
    lhs = Dee_True;
    Dee_Incref(Dee_True);
   } else {
    lhs = CALL_PRIMARY(As);
    if (ISERR(lhs)) goto err;
    LOAD_LVALUE(lhs,err);
    b = DeeObject_Bool(lhs);
    if (b < 0) goto err_invoke_norhs;
    Dee_Decref(lhs);
    lhs = DeeBool_For(b);
    Dee_Incref(lhs);
   }
  }
#else
  rhs = CALL_PRIMARY(As);
  if (ISERR(rhs)) goto err_r;
#endif
continue_expr:
  if (!TOKEN_IS_LOR(self->jl_tok))
      break;
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
/*err_invoke:*/
 /*DECREF(rhs);*/
err_invoke_norhs:
 JITLexer_ErrorTrace(self,pos);
#else
err_r:
#endif
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}

DEFINE_SECONDARY(CondOperand) {
 IF_EVAL(unsigned char *pos;)
 /* >>  x ? y : z // >> x ? y : z
  * >>  x ?: z    // >> x ? x : z
  * >> (x ? y : ) // >> x ? y : x
  * >> (x ? y)    // >> x ? y : none
  */
 ASSERT(TOKEN_IS_COND(self->jl_tok));
 LOAD_LVALUE(lhs,err);
 for (;;) {
  IF_EVAL(pos = self->jl_tokstart;)
  JITLexer_Yield(self);
#ifdef JIT_EVAL
  {
   int b = DeeObject_Bool(lhs);
   if unlikely(b < 0)
      goto err_r;
   if (self->jl_tok == ':') {
    /* Missing true-branch. */
    JITLexer_Yield(self);
    if (b) {
     /* true?:<SKIP> */
     if unlikely(JITLexer_SkipCond(self,flags))
        goto err_r;
    } else {
     /* false?:<EVAL> */
     Dee_Decref(lhs);
     lhs = JITLexer_EvalCond(self,flags);
     if unlikely(!lhs)
        goto err;
     LOAD_LVALUE(lhs,err);
    }
    goto continue_expr;
   }
   if (b) {
    /* true?<EXPR>[:[<SKIP>]] */
    Dee_Decref(lhs);
    lhs = JITLexer_EvalCond(self,flags);
    if unlikely(!lhs)
       goto err;
    LOAD_LVALUE(lhs,err);
    if (self->jl_tok == ':') {
     JITLexer_Yield(self);
     if (maybe_expression_begin_c(*self->jl_tokstart)) {
      if unlikely(JITLexer_SkipCond(self,flags))
         goto err_r;
     }
    }
   } else {
    /* false?<SKIP>[:[<EXPR>]] */
    if unlikely(JITLexer_SkipCond(self,flags))
       goto err_r;
    if (self->jl_tok != ':') {
     Dee_Decref(lhs);
     lhs = Dee_None;
     Dee_Incref(Dee_None);
    } else {
     JITLexer_Yield(self);
     if (maybe_expression_begin_c(*self->jl_tokstart)) {
      Dee_Decref(lhs);
      lhs = JITLexer_EvalCond(self,flags);
      if unlikely(!lhs)
         goto err;
      LOAD_LVALUE(lhs,err);
     }
    }
   }
  }
#else
  if (self->jl_tok == ':') {
   /* Missing true-branch. */
   JITLexer_Yield(self);
   if unlikely(JITLexer_SkipCond(self,flags))
      goto err_r;
   goto continue_expr;
  }
  if unlikely(JITLexer_SkipCond(self,flags))
     goto err_r;
  if (self->jl_tok == ':') {
   /* Parse the false-branch. */
   JITLexer_Yield(self);
   if (maybe_expression_begin_c(*self->jl_tokstart)) {
    if unlikely(JITLexer_SkipCond(self,flags))
       goto err_r;
   } else {
    /* Missing false-branch. */
   }
  }
#endif
continue_expr:
  if (!TOKEN_IS_COND(self->jl_tok))
       break;
 }
 return LHS_OR_OK;
err_r:
 DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif
 return ERROR;
}


#ifndef INPLACE_FOPS_DEFINED
#define INPLACE_FOPS_DEFINED 1
#define TOK_INPLACE_MIN       TOK_ADD_EQUAL
#define OPERATOR_INPLACE_MIN  OPERATOR_INPLACE_ADD
STATIC_ASSERT((TOK_ADD_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_ADD-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SUB_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SUB-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_MUL_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MUL-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_DIV_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_DIV-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_MOD_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MOD-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SHL_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHL-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SHR_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHR-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_AND_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_AND-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_OR_EQUAL -TOK_INPLACE_MIN) == (OPERATOR_INPLACE_OR-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_XOR_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_XOR-OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_POW_EQUAL-TOK_INPLACE_MIN) == (OPERATOR_INPLACE_POW-OPERATOR_INPLACE_MIN));
PRIVATE uint16_t const inplace_fops[] = {
    /* [TOK_ADD_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_ADD,
    /* [TOK_SUB_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_SUB,
    /* [TOK_MUL_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_MUL,
    /* [TOK_DIV_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_DIV,
    /* [TOK_MOD_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_MOD,
    /* [TOK_SHL_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_SHL,
    /* [TOK_SHR_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_SHR,
    /* [TOK_AND_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_AND,
    /* [TOK_OR_EQUAL  - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_OR,
    /* [TOK_XOR_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_XOR,
    /* [TOK_POW_EQUAL - TOK_INPLACE_MIN] = */OPERATOR_INPLACE_POW
};
#endif /* !INPLACE_FOPS_DEFINED */


DEFINE_SECONDARY(AssignOperand) {
 RETURN_TYPE rhs;
 IF_EVAL(int error;)
 unsigned char *pos;
 IF_EVAL(unsigned int cmd = self->jl_tok;)
 ASSERT(TOKEN_IS_ASSIGN(self->jl_tok));
 for (;;) {
  pos = self->jl_tokstart;
  if (self->jl_tok == TOK_COLLON_EQUAL) {
   LOAD_LVALUE(lhs,err);
   JITLexer_Yield(self);
   rhs = CALL_PRIMARYF(Cond,flags | JITLEXER_EVAL_FALLOWINPLACE);
   if (ISERR(rhs)) goto err_r;
#ifdef JIT_EVAL
   LOAD_LVALUE(rhs,err_r);
   error = DeeObject_Assign(lhs,rhs);
   Dee_Decref(rhs);
   if unlikely(error)
      goto err_invoke;
#endif /* JIT_EVAL */
  } else {
#ifdef JIT_EVAL
   JITLValue lhs_lvalue;
   if (lhs != JIT_LVALUE) {
    err_cannot_invoke_inplace(lhs,inplace_fops[self->jl_tok - TOK_INPLACE_MIN]);
    goto err_invoke_norhs;
   }
   /* Store the L-value expression for LHS locally, so we can load another expression for RHS*/
   memcpy(&lhs_lvalue,&self->jl_lvalue,sizeof(JITLValue));
   self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
   JITLexer_Yield(self);
   rhs = CALL_PRIMARYF(Cond,flags | JITLEXER_EVAL_FALLOWINPLACE);
   if (ISERR(rhs)) { err_lvalue: JITLValue_Fini(&lhs_lvalue); goto err; }
   LOAD_LVALUE(rhs,err_lvalue);
   /* Load the L-value expression target object. */
   lhs = JITLValue_GetValue(&lhs_lvalue,self->jl_context);
   if unlikely(!lhs) {
    Dee_Decref(rhs);
    self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
    goto err_lvalue;
   }
   switch (cmd) {
   case TOK_ADD_EQUAL: error = DeeObject_InplaceAdd(&lhs,rhs); break;
   case TOK_SUB_EQUAL: error = DeeObject_InplaceSub(&lhs,rhs); break;
   case TOK_MUL_EQUAL: error = DeeObject_InplaceMul(&lhs,rhs); break;
   case TOK_DIV_EQUAL: error = DeeObject_InplaceDiv(&lhs,rhs); break;
   case TOK_MOD_EQUAL: error = DeeObject_InplaceMod(&lhs,rhs); break;
   case TOK_SHL_EQUAL: error = DeeObject_InplaceShl(&lhs,rhs); break;
   case TOK_SHR_EQUAL: error = DeeObject_InplaceShr(&lhs,rhs); break;
   case TOK_AND_EQUAL: error = DeeObject_InplaceAnd(&lhs,rhs); break;
   case TOK_OR_EQUAL: error = DeeObject_InplaceOr(&lhs,rhs); break;
   case TOK_XOR_EQUAL: error = DeeObject_InplaceXor(&lhs,rhs); break;
   case TOK_POW_EQUAL: error = DeeObject_InplacePow(&lhs,rhs); break;
   default: __builtin_unreachable();
   }
   if unlikely(error) { JITLValue_Fini(&lhs_lvalue); goto err_invoke; }
   Dee_Decref(rhs);
   /* Save back the l-value expression result object. */
   error = JITLValue_SetValue(&lhs_lvalue,self->jl_context,lhs);
   JITLValue_Fini(&lhs_lvalue);
   if unlikely(error) goto err_r;
#else
   JITLexer_Yield(self);
   rhs = CALL_PRIMARYF(Cond,flags | JITLEXER_EVAL_FALLOWINPLACE);
   if (ISERR(rhs)) goto err_r;
#endif
  }
  if (!TOKEN_IS_ASSIGN(self->jl_tok))
      break;
  IF_EVAL(cmd = self->jl_tok;)
 }
 return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
 DECREF(rhs);
err_invoke_norhs:
 JITLexer_ErrorTrace(self,pos);
#endif
err_r:
 DECREF_MAYBE_LVALUE(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
 return ERROR;
}

DEFINE_SECONDARY(CommaTupleOperand) {
 RETURN_TYPE result;
#ifdef JIT_EVAL
 DREF DeeObject *new_result;
 result = Dee_EmptyTuple;
 Dee_Incref(Dee_EmptyTuple);
#else
 int lhs;
 result = 0;
#endif
 (void)flags;
 LOAD_LVALUE(lhs,err_r);
again:
 switch (self->jl_tok) {

 case TOK_DOTS:
  /* Expand expression */
#ifdef JIT_EVAL
  new_result = DeeTuple_ConcatInherited(result,lhs);
  if unlikely(!new_result) goto err_lhs;
  result = new_result;
  Dee_Decref(lhs);
#endif
  if (self->jl_tok != ',')
      break;
#ifdef JIT_EVAL
  __IF0 {
 case ',':
   new_result = DeeTuple_Append(result,lhs);
   if unlikely(!new_result) goto err_lhs;
   Dee_Decref(lhs);
   result = new_result;
  }
#else
 case ',':
#endif
  JITLexer_Yield(self);
  /* Allow  */
  if (!maybe_expression_begin_c(*self->jl_tokstart))
      break;
  /* Load the next expression. */
  lhs = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
  if (ISERR(lhs))
      goto err_r;
  LOAD_LVALUE(lhs,err_r);
  goto again;

 default:
#ifdef JIT_EVAL
  new_result = DeeTuple_Append(result,lhs);
  if unlikely(!new_result) goto err_lhs;
  Dee_Decref(lhs);
  result = new_result;
#endif
  break;
 }
 return result;
#ifdef JIT_EVAL
err_lhs:
 DECREF(lhs);
#endif
err_r:
 DECREF(result);
 return ERROR;
}

DEFINE_SECONDARY(CommaListOperand) {
 RETURN_TYPE result;
#ifdef JIT_EVAL
 result = DeeList_New();
 if unlikely(!result) {
  Dee_Decref(lhs);
  return NULL;
 }
#else
 int lhs;
 result = 0;
#endif
 (void)flags;
 LOAD_LVALUE(lhs,err_r);
again:
 switch (self->jl_tok) {

 case TOK_DOTS:
  /* Expand expression */
#ifdef JIT_EVAL
  if (DeeList_AppendSequence(result,lhs))
      goto err_lhs;
  Dee_Decref(lhs);
#endif
  if (self->jl_tok != ',')
      break;
#ifdef JIT_EVAL
  __IF0 {
 case ',':
   if (DeeList_Append(result,lhs))
       goto err_lhs;
   Dee_Decref(lhs);
  }
#else
 case ',':
#endif
  JITLexer_Yield(self);
  /* Allow  */
  if (!maybe_expression_begin_c(*self->jl_tokstart))
      break;
  /* Load the next expression. */
  lhs = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
  if (ISERR(lhs))
      goto err_r;
  LOAD_LVALUE(lhs,err_r);
  goto again;

 default:
#ifdef JIT_EVAL
  if (DeeList_Append(result,lhs))
      goto err_lhs;
  Dee_Decref(lhs);
#endif
  break;
 }
 return result;
#ifdef JIT_EVAL
err_lhs:
 DECREF(lhs);
#endif
err_r:
 DECREF(result);
 return ERROR;
}


DEFINE_SECONDARY(CommaDictOperand) {
 RETURN_TYPE result;
 RETURN_TYPE value;
#ifdef JIT_EVAL
 result = DeeDict_New();
 if unlikely(!result) {
  Dee_Decref(lhs);
  return NULL;
 }
#else
 int lhs;
 result = 0;
#endif
 (void)flags;
 LOAD_LVALUE(lhs,err_r);
again:
 JITLexer_Yield(self);
 value = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
 if (ISERR(value))
     goto err_lhs;
 LOAD_LVALUE(value,err_lhs);
#ifdef JIT_EVAL
 {
  int error = DeeObject_SetItem(result,lhs,value);
  Dee_Decref(value);
  if unlikely(error)
     goto err_lhs;
  Dee_Decref(lhs);
 }
#endif
 if (self->jl_tok != ',')
     goto done;
 JITLexer_Yield(self);
 if (!maybe_expression_begin_c(*self->jl_tokstart))
     goto done;
 /* Parse the next key. */
 if (self->jl_tok == '.') {
  JITLexer_Yield(self);
  if (self->jl_tok != JIT_KEYWORD) {
   err_expected_keyword_after_dot(self);
   goto err_r;
  }
#ifdef JIT_EVAL
  lhs = DeeString_NewUtf8((char const *)self->jl_tokstart,
                          (size_t)(self->jl_tokend - self->jl_tokstart),
                           STRING_ERROR_FSTRICT);
  if unlikely(!lhs) goto err_r;
#endif
  if (self->jl_tok != '=') {
   err_expected_eq_after_dot(self);
   goto err_lhs;
  }
 } else {
  lhs = CALL_PRIMARYF(Expression,JITLEXER_EVAL_FSECONDARY);
  if (ISERR(lhs)) goto err_r;
  LOAD_LVALUE(lhs,err_r);
  if (self->jl_tok != ':') {
   SYNTAXERROR("Expected `:' after key in mapping-like brace initializer, but got `%$s'",
              (size_t)(self->jl_tokend - self->jl_tokstart),
                       self->jl_tokstart);
   goto err_lhs;
  }
 }
 goto again;
done:
 return result;
err_lhs:
#ifdef JIT_EVAL
 DECREF(lhs);
#endif
err_r:
 DECREF(result);
 return ERROR;
}

DECL_END


#undef LOAD_LVALUE
#undef LOAD_LVALUE_OR_RETURN_ERROR
#undef IF_SKIP
#undef IF_EVAL
#undef DECREF
#undef DECREF_MAYBE_LVALUE
#undef SYNTAXERROR
#undef ERROR
#undef RETURN_TYPE
#undef LHS_OR_OK
#undef ISOK
#undef ISERR
#undef DEFINE_PRIMARY
#undef DEFINE_SECONDARY
#undef CALL_PRIMARY
#undef CALL_PRIMARYF
#undef CALL_SECONDARY
#undef JITLEXER_EVAL_FSECONDARY

#undef JIT_EVAL
#undef JIT_SKIP
