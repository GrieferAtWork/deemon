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
#ifndef GUARD_DEEMON_COMPILER_JIT_PARSER_C
#define GUARD_DEEMON_COMPILER_JIT_PARSER_C 1

#include <deemon/api.h>
#include <deemon/compiler/jit.h>
#ifndef CONFIG_NO_JIT
#include <deemon/int.h>
#include <deemon/error.h>
#include <deemon/class.h>
#include <deemon/compiler/lexer.h>
#include <hybrid/unaligned.h>

#include "../../runtime/strings.h"

DECL_BEGIN

INTDEF DeeObject *rt_operator_names[1+(AST_OPERATOR_MAX-AST_OPERATOR_MIN)];

INTERN DREF DeeObject *FCALL
JIT_GetOperatorFunction(uint16_t opname) {
 DREF DeeObject *result;
 DREF DeeModuleObject *operators_module;
 char const *symbol_name = NULL;
 dhash_t hash = 0;
 if (opname >= AST_OPERATOR_MIN &&
     opname <= AST_OPERATOR_MAX) {
  /* Special, ambiguous operator. */
  DeeObject *name;
  name        = rt_operator_names[opname-AST_OPERATOR_MIN];
  symbol_name = DeeString_STR(name);
  hash        = DeeString_Hash(name);
 } else {
  struct opinfo *info;
  /* Default case: determine the operator symbol using generic-operator info. */
  info = Dee_OperatorInfo(NULL,opname);
  if (info) {
   symbol_name = info->oi_sname;
   hash = hash_str(symbol_name);
  }
 }
 operators_module = (DREF DeeModuleObject *)DeeModule_Open(&str_operators,NULL,true);
 if unlikely(!operators_module) goto err;
 if (symbol_name) {
  result = DeeModule_GetAttrString(operators_module,
                                   symbol_name,
                                   hash);
 } else {
  /* Fallback: Invoke `operator(id)' to generate the default callback. */
  result = DeeModule_GetAttrString(operators_module,"operator",
                                   hash_ptr("operator",COMPILER_STRLEN("operator")));
  if likely(result) {
   DREF DeeObject *callback_result;
   callback_result = DeeObject_Callf(result,"I16u",opname);
   Dee_Decref(result);
   result = callback_result;
  }
 }
 Dee_Decref(operators_module);
 return result;
err:
 return NULL;
}



#ifdef CONFIG_LITTLE_ENDIAN
#define ENCODE2(a,b)     ((b)<<8|(a))
#define ENCODE4(a,b,c,d) ((d)<<24|(c)<<16|(b)<<8|(a))
#else
#define ENCODE2(a,b)     ((b)|(a)<<8)
#define ENCODE4(a,b,c,d) ((d)|(c)<<8|(b)<<16|(a)<<24)
#endif

INTDEF struct opinfo basic_opinfo[OPERATOR_USERCOUNT];
INTDEF struct opinfo file_opinfo[FILE_OPERATOR_COUNT];

INTDEF struct opinfo *DCALL
find_opinfo(struct opinfo *__restrict v, unsigned int c,
            char const *__restrict str, size_t len);


INTERN int32_t FCALL
JITLexer_ParseOperatorName(JITLexer *__restrict self,
                           uint16_t features) {
 int32_t result;
 switch (self->jl_tok) {

 {
  uint16_t val;
 case TOK_INT:
  if (Dee_Atou16((char const *)self->jl_tokstart,
                 (size_t)(self->jl_tokend - self->jl_tokstart),
                  DEEINT_STRING(0,DEEINT_STRING_FNORMAL),
                 &val))
      goto err;
  result = (int32_t)(uint16_t)val;
  goto done_y1;
 }

 case '+':               result = AST_OPERATOR_POS_OR_ADD; goto done_y1;
 case '-':               result = AST_OPERATOR_NEG_OR_SUB; goto done_y1;
 case '*':               result = OPERATOR_MUL; goto done_y1;
 case '/':               result = OPERATOR_DIV; goto done_y1;
 case '%':               result = OPERATOR_MOD; goto done_y1;
 case '&':               result = OPERATOR_AND; goto done_y1;
 case '|':               result = OPERATOR_OR; goto done_y1;
 case '^':               result = OPERATOR_XOR; goto done_y1;
 case '~':               result = OPERATOR_INV; goto done_y1;
 case TOK_SHL:           result = OPERATOR_SHL; goto done_y1;
 case TOK_SHR:           result = OPERATOR_SHR; goto done_y1;
 case TOK_POW:           result = OPERATOR_POW; goto done_y1;
 case TOK_ADD_EQUAL:     result = OPERATOR_INPLACE_ADD; goto done_y1;
 case TOK_SUB_EQUAL:     result = OPERATOR_INPLACE_SUB; goto done_y1;
 case TOK_MUL_EQUAL:     result = OPERATOR_INPLACE_MUL; goto done_y1;
 case TOK_DIV_EQUAL:     result = OPERATOR_INPLACE_DIV; goto done_y1;
 case TOK_MOD_EQUAL:     result = OPERATOR_INPLACE_MOD; goto done_y1;
 case TOK_SHL_EQUAL:     result = OPERATOR_INPLACE_SHL; goto done_y1;
 case TOK_SHR_EQUAL:     result = OPERATOR_INPLACE_SHR; goto done_y1;
 case TOK_AND_EQUAL:     result = OPERATOR_INPLACE_AND; goto done_y1;
 case TOK_OR_EQUAL:      result = OPERATOR_INPLACE_OR; goto done_y1;
 case TOK_XOR_EQUAL:     result = OPERATOR_INPLACE_XOR; goto done_y1;
 case TOK_POW_EQUAL:     result = OPERATOR_INPLACE_POW; goto done_y1;
 case TOK_INC:           result = OPERATOR_INC; goto done_y1;
 case TOK_DEC:           result = OPERATOR_DEC; goto done_y1;
 case TOK_EQUAL:         result = OPERATOR_EQ; goto done_y1;
 case TOK_NOT_EQUAL:     result = OPERATOR_NE; goto done_y1;
 case TOK_LOWER:         do_operator_lo: result = OPERATOR_LO; goto done_y1;
 case TOK_LOWER_EQUAL:   result = OPERATOR_LE; goto done_y1;
 case TOK_GREATER:       do_operator_gr: result = OPERATOR_GR; goto done_y1;
 case TOK_GREATER_EQUAL: result = OPERATOR_GE; goto done_y1;
 case '#':               result = OPERATOR_SIZE; goto done_y1;
 case '=':
 case TOK_COLLON_EQUAL:
  result = OPERATOR_ASSIGN;
  JITLexer_Yield(self);
  if (JITLexer_ISKWD(self,"move")) {
   /* `= move' move-assign operator. */
   result = OPERATOR_MOVEASSIGN;
   goto done_y1;
  }
  goto done;

 case '(':
  JITLexer_Yield(self);
  if (self->jl_tok == ')') {
   result = OPERATOR_CALL;
   goto done_y1;
  }
  /* Parenthesis around operator name. */
  result = JITLexer_ParseOperatorName(self,features);
  if unlikely(result < 0) goto err;
  if unlikely(self->jl_tok != ')') {
   return DeeError_Throwf(&DeeError_SyntaxError,
                          "Expected `)' after `(', but got `%$s'",
                         (size_t)(self->jl_tokend - self->jl_tokstart),
                          self->jl_tokstart);
  }
  goto done_y1;

 case JIT_RAWSTRING:
  if (self->jl_tokend != self->jl_tokstart + 3)
      goto err_empty_string;
  result = OPERATOR_STR;
  break;
 case JIT_STRING:
  if (self->jl_tokend != self->jl_tokstart + 2) {
err_empty_string:
   return DeeError_Throwf(&DeeError_SyntaxError,
                          "Expected an empty string for `operator str'");
  }
  result = OPERATOR_STR;
  break;

 case '[':
  JITLexer_Yield(self);
  result = AST_OPERATOR_GETITEM_OR_SETITEM;
  if (self->jl_tok == ':') {
   result = AST_OPERATOR_GETRANGE_OR_SETRANGE;
   JITLexer_Yield(self);
  }
  if likely(self->jl_tok == ']') {
   JITLexer_Yield(self);
  } else {
err_rbrck_after_lbrck:
   return DeeError_Throwf(&DeeError_SyntaxError,
                          "Expected `]' after `[', but got `%$s'",
                         (size_t)(self->jl_tokend - self->jl_tokstart),
                          self->jl_tokstart);
  }
  if (self->jl_tok == '=') {
   JITLexer_Yield(self);
   result = (result == AST_OPERATOR_GETITEM_OR_SETITEM
           ? OPERATOR_SETITEM
           : OPERATOR_SETRANGE);
  }
  goto done;

 case '.':
  result = AST_OPERATOR_GETATTR_OR_SETATTR;
  JITLexer_Yield(self);
  if (self->jl_tok == '=') {
   result = OPERATOR_SETATTR;
   goto done_y1;
  }
  goto done;

 {
  char const *name_begin;
  size_t name_size; uint32_t name;
 default:
/*default_case:*/
  if (self->jl_tok != JIT_KEYWORD) goto unknown;
  name_begin = (char const *)self->jl_tokstart;
  name_size  = (size_t)((char const *)self->jl_tokend - name_begin);
  /* Other operator names that technically should have their own
   * keyword, but since this is the only place that keyword would
   * ever get used, the overhead of manually checking for them is
   * smaller, causing me to opt for this route instead. */
  switch (name_size) {
  case 3:
   if (name_begin[0] == 's' &&
       name_begin[1] == 't' &&
       name_begin[2] == 'r') {
    result = OPERATOR_STR;
    goto done_y1;
   }
   if (name_begin[0] == 'd' &&
       name_begin[1] == 'e' &&
       name_begin[2] == 'l') {
    JITLexer_Yield(self);
    if (self->jl_tok == '[') {
     JITLexer_Yield(self);
     result = OPERATOR_DELITEM;
     if (self->jl_tok == ':') {
      result = OPERATOR_DELRANGE;
      JITLexer_Yield(self);
     }
     if unlikely(self->jl_tok != ']')
        goto err_rbrck_after_lbrck;
     goto done_y1;
    }
    result = OPERATOR_DELATTR;
    if unlikely(self->jl_tok != '.') {
     return DeeError_Throwf(&DeeError_SyntaxError,
                            "Expected `[' or `.' after `del' in operator name, but got `%$s'",
                           (size_t)(self->jl_tokend - self->jl_tokstart),
                            self->jl_tokstart);
    }
    goto done_y1;
   }
   if (name_begin[0] == 'f' &&
       name_begin[1] == 'o' &&
       name_begin[2] == 'r' &&
      (features & P_OPERATOR_FCLASS)) {
    result = AST_OPERATOR_FOR;
    goto done_y1;
   }
   break;

  case 4:
   name = UNALIGNED_GET32((uint32_t *)name_begin);
#ifndef __OPTIMIZE_SIZE__
   if (name == ENCODE4('h','a','s','h')) { result = OPERATOR_HASH; goto done_y1; }
#endif /* !__OPTIMIZE_SIZE__ */
   if (name == ENCODE4('n','e','x','t')) { result = OPERATOR_ITERNEXT; goto done_y1; }
   if (name == ENCODE4('i','t','e','r')) { result = OPERATOR_ITERSELF; goto done_y1; }
   if (name == ENCODE4('r','e','p','r')) { result = OPERATOR_REPR; goto done_y1; }
   if (name == ENCODE4('c','o','p','y')) { result = OPERATOR_COPY; goto done_y1; }
   if (name == ENCODE4('m','o','v','e')) {
    JITLexer_Yield(self);
    result = OPERATOR_MOVEASSIGN;
    if unlikely(self->jl_tok != '=' &&
                self->jl_tok != TOK_COLLON_EQUAL) {
     return DeeError_Throwf(&DeeError_SyntaxError,
                            "Expected `:=' or `=' after `move' in operator name, but got `%$s'",
                           (size_t)(self->jl_tokend - self->jl_tokstart),
                            self->jl_tokstart);
    }
    goto done_y1;
   }
   break;

#ifndef __OPTIMIZE_SIZE__
  case 5:
   name = UNALIGNED_GET32((uint32_t *)name_begin);
   if (name == ENCODE4('e','n','t','e') && *((uint8_t *)(name_begin + 4)) == 'r') { result = OPERATOR_ENTER; goto done_y1; }
   if (name == ENCODE4('l','e','a','v') && *((uint8_t *)(name_begin + 4)) == 'e') { result = OPERATOR_LEAVE; goto done_y1; }
   if (name == ENCODE4('s','u','p','e') && *((uint8_t *)(name_begin + 4)) == 'r' && (features & P_OPERATOR_FCLASS)) { result = CLASS_OPERATOR_SUPERARGS; goto done_y1; }
   break;
  case 8:
   if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('c','o','n','t') &&
       UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE4('a','i','n','s'))
   { result = OPERATOR_CONTAINS; goto done_y1; }
   if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('d','e','e','p') &&
       UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE4('c','o','p','y'))
   { result = OPERATOR_DEEPCOPY; goto done_y1; }
   break;
  case 10:
   if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('d','e','s','t') &&
       UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE4('r','u','c','t') &&
       UNALIGNED_GET16((uint16_t *)(name_begin + 8)) == ENCODE2('o','r'))
   { result = OPERATOR_DESTRUCTOR; goto done_y1; }
   break;
  case 11:
   if (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('c','o','n','s') &&
       UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE4('t','r','u','c') &&
       UNALIGNED_GET32((uint32_t *)(name_begin + 8)) == ENCODE4('t','o','r',0))
   { result = OPERATOR_CONSTRUCTOR; goto done_y1; }
   break;
#endif /* !__OPTIMIZE_SIZE__ */
  default: break;
  }
  while (name_size && *name_begin == '_') ++name_begin,--name_size;
  while (name_size && name_begin[name_size-1] == '_') --name_size;

#if 0 /* Already handled by generic opinfo searches. */
  /* Some special operators that didn't merit their own keyword. */
  if (name_size == 4 &&
      UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('b','o','o','l'))
  { result = OPERATOR_BOOL; goto done_y1; }
  if (name_size == 3 &&
      UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('i','n','t',0))
  { result = OPERATOR_INT; goto done_y1; }
  if (name_size == 5 &&
      UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('f','l','o','a') &&
      *(uint8_t *)(name_begin + 4) == 't')
  { result = OPERATOR_FLOAT; goto done_y1; }
#endif

  /* Query an explicit operator by its name.
   * NOTE: This is also where a lot of backwards-compatibility lies, as
   *       the old deemon used to only accept e.g.: `operator __contains__'. */
  { struct opinfo *info;
    info = find_opinfo(basic_opinfo,COMPILER_LENOF(basic_opinfo),name_begin,name_size);
    if (info) { result = (uint16_t)(info-basic_opinfo); goto done_y1; }
    if (!(features & P_OPERATOR_FNOFILE)) {
     info = find_opinfo(file_opinfo,COMPILER_LENOF(file_opinfo),name_begin,name_size);
     if (info) { result = OPERATOR_EXTENDED(0)+(uint16_t)(info-file_opinfo); goto done_y1; }
    }
    /* Even more backwards compatibility. */
    if (name_size == 2) {
     if (UNALIGNED_GET16((uint16_t *)(name_begin + 0)) == ENCODE2('l','t'))
         goto do_operator_lo;
     if (UNALIGNED_GET16((uint16_t *)(name_begin + 0)) == ENCODE2('g','t'))
         goto do_operator_gr;
    }
    if (name_size == 6 &&
        UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('r','e','a','d') &&
        UNALIGNED_GET16((uint16_t *)(name_begin + 4)) == ENCODE2('n','p'))
    { result = FILE_OPERATOR_READ; goto done_y1; }
    if (name_size == 7 &&
        UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('w','r','i','t') &&
        UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE4('e','n','p',0))
    { result = FILE_OPERATOR_WRITE; goto done_y1; }
    if (name_size == 9 &&
        UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('s','u','p','e') &&
        UNALIGNED_GET32((uint32_t *)(name_begin + 4)) == ENCODE4('r','a','r','g') &&
                       *(uint8_t  *)(name_begin + 8) == 's' && (features&P_OPERATOR_FCLASS))
    { result = CLASS_OPERATOR_SUPERARGS; goto done_y1; }
    if (name_size == 6 &&
        UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('d','o','u','b') &&
        UNALIGNED_GET16((uint16_t *)(name_begin + 4)) == ENCODE2('l','e'))
    { result = OPERATOR_FLOAT; goto done_y1; }
    if (name_size == 5 &&
       ((UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('i','n','t','3') &&
                        *(uint8_t  *)(name_begin + 4) == '2') ||
        (UNALIGNED_GET32((uint32_t *)(name_begin + 0)) == ENCODE4('i','n','t','6') &&
                        *(uint8_t  *)(name_begin + 4) == '4')))
    { result = OPERATOR_INT; goto done_y1; }
  }
unknown:
  return DeeError_Throwf(&DeeError_SyntaxError,
                         "Unknown operator name `%$s'",
                        (size_t)(self->jl_tokend - self->jl_tokstart),
                         self->jl_tokstart);
 } break;
 }
done_y1:
 JITLexer_Yield(self);
done:
 return result;
err:
 return -1;
}

DECL_END

#ifndef __INTELLISENSE__
#define JIT_SKIP 1
#include "parser-impl.c.inl"
#define JIT_EVAL 1
#include "parser-impl.c.inl"
#endif

#endif /* !CONFIG_NO_JIT */

#endif /* !GUARD_DEEMON_COMPILER_JIT_PARSER_C */
