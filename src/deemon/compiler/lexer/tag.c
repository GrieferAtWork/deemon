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
#ifndef GUARD_DEEMON_COMPILER_LEXER_TAG_C
#define GUARD_DEEMON_COMPILER_LEXER_TAG_C 1

#include <deemon/api.h>
#include <deemon/none.h>
#include <deemon/class.h>
#include <deemon/string.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/tuple.h>
#include <deemon/error.h>

DECL_BEGIN

INTERN struct ast_tags current_tags;
INTERN void DCALL clear_current_tags(void) {
 unicode_printer_fini(&current_tags.at_doc);
 memset(&current_tags,0,sizeof(struct ast_tags));
}

PRIVATE int DCALL append_doc_string(void) {
 ASSERT(tok == TOK_STRING);
 do {
  if unlikely(ast_decode_unicode_string(&current_tags.at_doc)) goto err;
  if unlikely(yield() < 0) goto err;
 } while (tok == TOK_STRING);
 /* Append a line-feed at the end. */
 if unlikely(unicode_printer_putascii(&current_tags.at_doc,'\n'))
    goto err;
 return 0;
err:
 return -1;
}

struct tag_flag {
    char     name[16]; /* Name of the flag tag. */
    uint16_t addr;     /* The offset into `struct ast_tags' where this flag is located at. */
    uint16_t flag;     /* The flag set by this tag. */
};

PRIVATE struct tag_flag tag_flags[] = {
    { "copyable",    COMPILER_OFFSETOF(struct ast_tags,at_code_flags),   CODE_FCOPYABLE },
    { "thiscall",    COMPILER_OFFSETOF(struct ast_tags,at_code_flags),   CODE_FTHISCALL },
    { "assembly",    COMPILER_OFFSETOF(struct ast_tags,at_code_flags),   CODE_FASSEMBLY },
    { "nobase",      COMPILER_OFFSETOF(struct ast_tags,at_tagflags),     AST_TAG_FNOBASE },
    { "lenient",     COMPILER_OFFSETOF(struct ast_tags,at_code_flags),   CODE_FLENIENT },
    { "heapframe",   COMPILER_OFFSETOF(struct ast_tags,at_code_flags),   CODE_FHEAPFRAME },
    { "constructor", COMPILER_OFFSETOF(struct ast_tags,at_code_flags),   CODE_FCONSTRUCTOR },
    { "final",       COMPILER_OFFSETOF(struct ast_tags,at_class_flags),  TP_FFINAL },
    { "truncate",    COMPILER_OFFSETOF(struct ast_tags,at_class_flags),  TP_FTRUNCATE },
    { "interrupt",   COMPILER_OFFSETOF(struct ast_tags,at_class_flags),  TP_FINTERRUPT },
    { "private",     COMPILER_OFFSETOF(struct ast_tags,at_member_flags), CLASS_ATTRIBUTE_FPRIVATE },
    { "readonly",    COMPILER_OFFSETOF(struct ast_tags,at_member_flags), CLASS_ATTRIBUTE_FREADONLY },
    { "method",      COMPILER_OFFSETOF(struct ast_tags,at_member_flags), CLASS_ATTRIBUTE_FMETHOD },
    { "likely",      COMPILER_OFFSETOF(struct ast_tags,at_expect),       AST_FCOND_LIKELY },
    { "unlikely",    COMPILER_OFFSETOF(struct ast_tags,at_expect),       AST_FCOND_UNLIKELY }
};



INTERN int (DCALL parse_tags)(void) {
 DREF struct ast *args_ast;
 DREF DeeObject *tag_args;
 size_t i; uint32_t old_flags;
again:
 if (tok == TOK_STRING) {
  if unlikely(append_doc_string())
     goto err;
 } else if (TPP_ISKEYWORD(tok)) {
  tok_t tag_id = tok;
  char const *tag_name_str = token.t_kwd->k_name;
  size_t      tag_name_len = token.t_kwd->k_size;
  /* Trim leading/trailing underscores from tag names (prevent ambiguity when macros are used). */
  while (tag_name_len && *tag_name_str == '_') ++tag_name_str,--tag_name_len;
  while (tag_name_len && tag_name_str[tag_name_len-1] == '_') --tag_name_len;
  if unlikely(tag_name_str != token.t_kwd->k_name ||
              tag_name_len != token.t_kwd->k_size) {
   struct TPPKeyword *kwd;
   kwd = TPPLexer_LookupKeyword(tag_name_str,tag_name_len,1);
   if unlikely(!kwd) goto err;
   tag_id = kwd->k_id;
  }
  if unlikely(yield() < 0) goto err;
  args_ast = NULL;
  tag_args = NULL;
  if (tok == '(') {
   /* Parse a tag argument list. */
   old_flags = TPPLexer_Current->l_flags;
   TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
   if unlikely(yield() < 0) goto err_flags;
   if (tok == KWD_auto && tag_id == KWD_doc) {
    /* Special case: `@doc(auto)' */
    if unlikely(unicode_printer_putascii(&current_tags.at_doc,'\n'))
       goto err_flags;
    if unlikely(yield() < 0) goto err_flags;
    TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
    if unlikely(likely(tok == ')') ? (yield() < 0) :
                WARN(W_EXPECTED_RPAREN_AFTER_LPAREN_IN_TAG))
       goto err;
    goto done_tag;
   }
   if (tok == ')') {
    args_ast = ast_sethere(ast_constexpr(Dee_EmptyTuple));
   } else {
    args_ast = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
                               AST_FMULTIPLE_TUPLE,
                               NULL);
   }
   if unlikely(!args_ast) goto err_flags;
   TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
   if unlikely(likely(tok == ')') ? (yield() < 0) :
               WARN(W_EXPECTED_RPAREN_AFTER_LPAREN_IN_TAG))
      goto err_args;
   /* Optimize the arguments AST to reduce the argument tuple. */
   if (ast_optimize_all(args_ast,true)) goto err_args;
  }
  __IF0 {
need_constexpr:
   ASSERT(!tag_args);
   if (args_ast->a_type == AST_CONSTEXPR) {
    tag_args = args_ast->a_constexpr;
    ASSERT(DeeTuple_Check(tag_args));
    Dee_Incref(tag_args);
   } else {
    if unlikely(WARN(W_EXPECTED_CONSTANT_EXPRESSION_FOR_TAG))
       goto err_args;
   }
  }
  switch (tag_id) {

   /* Define the class/super symbols. */
  case KWD_class:
  case KWD_super:
   if (args_ast->a_type != AST_MULTIPLE ||
       args_ast->a_multiple.m_astc != 1 ||
       args_ast->a_multiple.m_astv[0]->a_type != AST_SYM) {
    if unlikely(WARN(W_EXPECTED_SYMBOL_FOR_CLASS_SUPER_TAG))
       goto err;
   } else {
    struct symbol *sym;
    sym = args_ast->a_multiple.m_astv[0]->a_sym;
    ASSERT(sym);
    /* TODO: Check if `sym->sym_scope' is reachable from `current_scope' */
    if (tag_id == KWD_class) {
     current_tags.at_class = sym;
    } else {
     current_tags.at_super = sym;
    }
   }
   break;

  case KWD_doc:
   if (!tag_args) goto need_constexpr;
   if unlikely(!tag_args) {
    if unlikely(WARN(W_EXPECTED_ARGUMENTS_FOR_TAG))
       goto err_args;
    break;
   }
   /* Append documentation strings. */
   for (i = 0; i < DeeTuple_SIZE(tag_args); ++i) {
    DeeObject *arg = DeeTuple_GET(tag_args,i);
    if (DeeString_Check(arg)) {
     if unlikely(unicode_printer_printstring(&current_tags.at_doc,arg) < 0)
        goto err_args;
     continue;
    }
    if (WARN(W_EXPECTED_STRING_FOR_DOC_TAG))
        goto err_args;
   }
   break;

  default:
#define IS_TAG(x) \
  (tag_name_len == COMPILER_STRLEN(x) && \
   !memcmp(tag_name_str,x,COMPILER_STRLEN(x)))
   if unlikely(tag_args) {
    if unlikely(WARN(W_UNEXPECTED_ARGUMENTS_FOR_TAG))
       goto err_args;
    break;
   }
   /* Misc tags without their own token ID. */
   {
    struct tag_flag *iter;
    for (iter  = tag_flags;
         iter != COMPILER_ENDOF(tag_flags); ++iter) {
     if (strlen(iter->name) == tag_name_len &&
         memcmp(iter->name,tag_name_str,tag_name_len*sizeof(char)) == 0) {
      /* Set the flag specified by the entry. */
      *(uint16_t *)((uint8_t *)&current_tags+iter->addr) |= iter->flag;
      goto found_tag;
     }
    }
   }
   if unlikely(WARN(W_UNKNOWN_TAG,tag_name_len,tag_name_str))
      goto err_args;
#undef IS_TAG
   break;
  }
found_tag:
  ast_xdecref(args_ast);
  Dee_XDecref(tag_args);
 } else {
  goto done;
 }
done_tag:
 if (tok == ',') {
  /* Parse more tags. */
  if unlikely(yield() < 0) goto err;
  goto again;
 }
done:
 return 0;
err_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err;
err_args:
 ast_xdecref(args_ast);
 Dee_XDecref(tag_args);
err:
 return -1;
}

INTERN int DCALL parse_tags_block(void) {
 while (tok == '@') {
  if unlikely(yield() < 0) goto err;
  if unlikely(parse_tags()) goto err;
 }
 return 0;
err:
 return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_TAG_C */
