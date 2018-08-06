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
#ifndef GUARD_DEEMON_COMPILER_LEXER_ASM_C
#define GUARD_DEEMON_COMPILER_LEXER_ASM_C 1

#include <deemon/api.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>

#ifndef CONFIG_LANGUAGE_NO_ASM
DECL_BEGIN

#define OPERAND_TYPE_OUTPUT 0
#define OPERAND_TYPE_INPUT  1
#define OPERAND_TYPE_LABEL  2
#define OPERAND_TYPE_COUNT  3
struct operand_list {
    size_t              ol_a;  /* Allocated amount of operands. */
    size_t              ol_c;  /* Amount of operands in use. */
    struct asm_operand *ol_v;  /* [0..ol_c|ALLOC(ol_a)][owned] Vector of operands. */
    size_t              ol_count[OPERAND_TYPE_COUNT]; /* Different operand counts. */
};

PRIVATE void DCALL
operand_list_fini(struct operand_list *__restrict self) {
 struct asm_operand *iter,*end;
 ASSERT(self->ol_c <= self->ol_a);
 ASSERT(self->ol_count[OPERAND_TYPE_OUTPUT]+
        self->ol_count[OPERAND_TYPE_INPUT]+
        self->ol_count[OPERAND_TYPE_LABEL] ==
        self->ol_c);
 end = (iter = self->ol_v)+
       (self->ol_count[OPERAND_TYPE_OUTPUT]+
        self->ol_count[OPERAND_TYPE_INPUT]);
 for (; iter != end; ++iter) {
  ASSERT(iter->ao_type);
  ASSERT(iter->ao_expr);
  TPPString_Decref(iter->ao_type);
  Dee_Decref(iter->ao_expr);
 }
 end = self->ol_v+self->ol_c;
 for (; iter != end; ++iter) {
  ASSERT(!iter->ao_type);
  ASSERT(iter->ao_label);
  ASSERT(iter->ao_label->tl_goto);
  --iter->ao_label->tl_goto;
 }
 Dee_Free(self->ol_v);
}

/* Allocate and return a new operand.
 * NOTE: The caller is responsible for safely initializing this operand. */
PRIVATE struct asm_operand *DCALL
operand_list_add(struct operand_list *__restrict self,
                 unsigned int type) {
 struct asm_operand *result = self->ol_v;
 ASSERT(self->ol_c <= self->ol_a);
 ASSERT(type < OPERAND_TYPE_COUNT);
 if (self->ol_c == self->ol_a) {
  size_t new_alloc = self->ol_a*2;
  if (!new_alloc) new_alloc = 2;
do_realloc:
  result = (struct asm_operand *)Dee_TryRealloc(self->ol_v,new_alloc*
                                                sizeof(struct asm_operand));
  if unlikely(!result) {
   if (new_alloc != self->ol_c+1) { new_alloc = self->ol_c+1; goto do_realloc; }
   if (Dee_CollectMemory(new_alloc*sizeof(struct asm_operand))) goto do_realloc;
   return NULL;
  }
  self->ol_v = result;
  self->ol_a = new_alloc;
 }
 /* Reserve the next operand number. */
 result += self->ol_c++;
 ++self->ol_count[type];
 return result;
}

PRIVATE int DCALL
asm_parse_operands(struct operand_list *__restrict list,
                   unsigned int type) {
 /*ref*/struct TPPString *operand_type;
 DREF DeeAstObject *operand_value;
 struct asm_operand *operand;
 while ((type == OPERAND_TYPE_LABEL ? TPP_ISKEYWORD(tok)
                                    : tok == TOK_STRING) ||
         tok == '[') {
  struct TPPKeyword *name = NULL;
  if (tok == '[') {
   if unlikely(yield() < 0) goto err;
   if (TPP_ISKEYWORD(tok)) {
    name = token.t_kwd;
    if unlikely(yield() < 0) goto err;
   } else {
    if (WARN(W_EXPECTED_KEYWORD_FOR_OPERAND_NAME))
        goto err;
   }
   if unlikely(likely(tok == ']') ? (yield() < 0) :
               WARN(W_EXPECTED_RBRACKET_AFTER_OPERAND_NAME))
      goto err;
  }
  if (type == OPERAND_TYPE_LABEL) {
   struct text_label *label_value;
   /* Label operand. */
   if (TPP_ISKEYWORD(tok)) {
    label_value = lookup_label(token.t_kwd);
    if unlikely(!label_value) goto err;
    if unlikely(yield() < 0) goto err;
   } else {
    if (WARN(W_EXPECTED_KEYWORD_FOR_LABEL_OPERAND))
        goto err;
    label_value = lookup_label(&TPPKeyword_Empty);
    if unlikely(!label_value) goto err;
   }
   operand = operand_list_add(list,type);
   if unlikely(!operand) goto err;
   /* Add the usage-reference to the label. */
   ++label_value->tl_goto;
   operand->ao_label = label_value;
   operand->ao_type  = NULL;
  } else {
   if (tok == TOK_STRING) {
    operand_type = TPPLexer_ParseString();
    if unlikely(!operand_type) goto err;
   } else {
    if (WARN(W_EXPECTED_STRING_BEFORE_OPERAND_VALUE))
        goto err;
    operand_type = TPPString_NewEmpty();
   }
   if (tok == KWD_pack) {
    if unlikely(yield() < 0) goto err_type;
    if (tok == '(') goto with_paren;
    operand_value = ast_parse_brace(LOOKUP_SYM_NORMAL,NULL);
    if unlikely(!operand_value) goto err_type;
   } else {
with_paren:
    if unlikely(likely(tok == '(') ? (yield() < 0) :
                WARN(W_EXPECTED_LPAREN_BEFORE_OPERAND_VALUE))
       goto err_type;
    operand_value = ast_parse_brace(LOOKUP_SYM_NORMAL,NULL);
    if unlikely(!operand_value) goto err_type;
    if unlikely(likely(tok == ')') ? (yield() < 0) :
                WARN(W_EXPECTED_RPAREN_AFTER_OPERAND_VALUE))
       goto err_value;
   }
   operand = operand_list_add(list,type);
   if unlikely(!operand) goto err_value;
   operand->ao_type = operand_type; /* Inherit */
   operand->ao_expr = operand_value; /* Inherit */
  }
  operand->ao_name = name;
  /* Yield the trailing comma. */
  if (tok != ',') break;
  if unlikely(yield() < 0) goto err;
 }
 return 0;
err_value: Dee_Decref(operand_value);
err_type:  TPPString_Decref(operand_type);
err:       return -1;
}




struct clobber_desc {
    char     cd_name[12]; /* Name of the clobber-operand. */
    uint16_t cd_flags;    /* Set of `AST_FASSEMBLY_*' defined for this descriptor. */
};

PRIVATE struct clobber_desc const clobber_descs[] = {
    { "memory",   AST_FASSEMBLY_MEMORY },   /* Memory barrier. */
    { "reach",    AST_FASSEMBLY_REACH },    /* User-assembly can be reached through non-conventional means. */
    { "noreturn", AST_FASSEMBLY_NORETURN }, /* User-assembly doesn't return normally (i.e. returns using `ret', or `throw'). */
    { "sp",       AST_FASSEMBLY_CLOBSP },   /* Don't warn about miss-aligned stack pointers. */
    { "cc", 0 },                            /* Ignored... */
};

/* Parse the clobber list and return a set of `AST_FASSEMBLY_*' */
PRIVATE int32_t DCALL asm_parse_clobber(void) {
 struct TPPString *name;
 uint16_t result = 0;
 while (tok == TOK_STRING) {
  name = TPPLexer_ParseString();
  if unlikely(!name) goto err;
  if (name->s_size < COMPILER_LENOF(clobber_descs[0].cd_name)) {
   struct clobber_desc const *iter = clobber_descs;
   for (; iter != COMPILER_ENDOF(clobber_descs); ++iter) {
    if (name->s_size == strlen(iter->cd_name) &&
        memcmp(name->s_text,iter->cd_name,name->s_size*sizeof(char)) == 0) {
     /* Found it! Set the proper flags and continue. */
     result |= iter->cd_flags;
     goto got_clobber;
    }
   }
  }
  if (WARN(W_UASM_UNKNOWN_CLOBBER_NAME,name->s_text))
      goto err_name;
got_clobber:
  TPPString_Decref(name);
  /* Yield the trailing comma. */
  if (tok != ',') break;
  if unlikely(yield() < 0) goto err;
 }
 return result;
err_name:
 TPPString_Decref(name);
err:
 return -1;
}

LOCAL bool DCALL is_collon(void) {
 if (tok == ':') return true;
 if (tok == TOK_COLLON_COLLON ||
     tok == TOK_COLLON_EQUAL) {
  /* Convert to a `:'-token and setup the lexer to re-parse
   * the remainder of the current token as part of the next. */
  token.t_id  = ':';
  token.t_end = token.t_begin+1;
  token.t_file->f_pos = token.t_end;
  return true;
 }
 return false;
}


INTERN DREF DeeAstObject *DCALL ast_parse_asm(void) {
 struct ast_loc loc; bool is_asm_goto = false;
 uint16_t ast_flags = AST_FASSEMBLY_NORMAL;
 /*REF*/struct TPPString *text;
 struct operand_list operands;
 DREF DeeAstObject *result;
 uint32_t old_flags; bool has_paren;
 memset(&operands,0,sizeof(struct operand_list));
 /*ASSERT(tok == KWD___asm__);*/
 if unlikely(yield() < 0) goto err;
 while (TPP_ISKEYWORD(tok)) {
  char const *name = token.t_kwd->k_name;
  size_t      size = token.t_kwd->k_size;
  while (size && *name == '_') ++name,--size;
  while (size && name[size-1] == '_') --size;
  if (size == COMPILER_STRLEN("volatile") &&
      memcmp(name,"volatile",size*sizeof(char)) == 0)
  { ast_flags |= AST_FASSEMBLY_VOLATILE; goto yield_prefix; }
  if (size == COMPILER_STRLEN("goto") &&
      memcmp(name,"goto",size*sizeof(char)) == 0)
  { is_asm_goto = true; goto yield_prefix; }
  break;
yield_prefix:
  if unlikely(yield() < 0) goto err;
 }
 old_flags = TPPLexer_Current->l_flags;
 TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
 if (tok == KWD_pack) {
  if unlikely(yield() < 0)
     goto err_flags;
  if (tok == '(') goto with_paren;
  has_paren = false;
 } else {
with_paren:
  if unlikely(likely(tok == '(') ? (yield() < 0) :
              WARN(W_EXPECTED_LPAREN_AFTER_ASM))
     goto err_flags;
  has_paren = true;
 }
 loc_here(&loc); /* Use the assembly text for DDI information. */
 if (tok == TOK_STRING) {
  text = TPPLexer_ParseString();
  if unlikely(!text) goto err_flags;
 } else {
  if (WARN(W_EXPECTED_STRING_AFTER_ASM))
      goto err_flags;
  text = TPPString_NewEmpty();
 }
 if (is_collon()) {
  if unlikely(yield() < 0) goto err_ops;
  /* Enable assembly formatting. */
  ast_flags |= AST_FASSEMBLY_FORMAT;
  /* Parse operands. */
  if unlikely(asm_parse_operands(&operands,OPERAND_TYPE_OUTPUT))
     goto err_ops;
  if (is_collon()) {
   if unlikely(yield() < 0) goto err_ops;
   if unlikely(asm_parse_operands(&operands,OPERAND_TYPE_INPUT))
      goto err_ops;
   if (is_collon()) {
    int32_t clobber;
    if unlikely(yield() < 0) goto err_ops;
    clobber = asm_parse_clobber();
    if unlikely(clobber < 0) goto err_ops;
    ast_flags |= (uint16_t)clobber;
    if (is_asm_goto && is_collon()) {
     if unlikely(yield() < 0) goto err_ops;
     if unlikely(asm_parse_operands(&operands,OPERAND_TYPE_LABEL))
        goto err_ops;
    }
   }
  }
 }
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 if (has_paren) {
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_ASM))
     goto err_text;
 }
 ASSERT(operands.ol_c ==
        operands.ol_count[OPERAND_TYPE_OUTPUT]+
        operands.ol_count[OPERAND_TYPE_INPUT]+
        operands.ol_count[OPERAND_TYPE_LABEL]);
 result = ast_assembly(ast_flags,text,
                       operands.ol_count[OPERAND_TYPE_OUTPUT],
                       operands.ol_count[OPERAND_TYPE_INPUT],
                       operands.ol_count[OPERAND_TYPE_LABEL],
                       operands.ol_v);
 if unlikely(!result) goto err_ops;
 /* NOTE: `ast_assembly' has inherited the operand vector upon success. */
 TPPString_Decref(text);
 return ast_setddi(result,&loc);
err_ops:
 operand_list_fini(&operands);
err_text:
 TPPString_Decref(text);
err_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err:
 return NULL;
}


DECL_END
#endif /* !CONFIG_LANGUAGE_NO_ASM */

#endif /* !GUARD_DEEMON_COMPILER_LEXER_ASM_C */
