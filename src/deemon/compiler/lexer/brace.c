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
#ifndef GUARD_DEEMON_COMPILER_LEXER_BRACE_C
#define GUARD_DEEMON_COMPILER_LEXER_BRACE_C 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/none.h>
#include <deemon/list.h>
#include <deemon/hashset.h>
#include <deemon/dict.h>
#include <deemon/tuple.h>
#include <deemon/seq.h>
#include <deemon/map.h>

DECL_BEGIN

struct type_mapping {
    DeeTypeObject *tm_type; /* Sequence type. */
    uint16_t       tm_flag; /* Flags for AST_MULTIPLE */
};

PRIVATE struct type_mapping seq_flags[] = {
    { &DeeList_Type, AST_FMULTIPLE_LIST },
    { &DeeTuple_Type, AST_FMULTIPLE_TUPLE },
    { &DeeHashSet_Type, AST_FMULTIPLE_SET },
    { &DeeDict_Type, AST_FMULTIPLE_DICT },
    { &DeeSeq_Type, AST_FMULTIPLE_GENERIC },
    { &DeeMapping_Type, AST_FMULTIPLE_GENERIC_KEYS },
    { NULL, AST_FMULTIPLE_GENERIC },
};

PRIVATE uint16_t DCALL get_seq_flags(DeeTypeObject *type) {
 struct type_mapping *iter = seq_flags;
 for (;; ++iter) {
  ASSERT(iter != COMPILER_ENDOF(seq_flags));
  if (iter->tm_type == type)
      return iter->tm_flag;
 }
}



INTERN DREF DeeAstObject *FCALL
ast_parse_brace_items(DeeTypeObject *preferred_type) {
 DREF DeeAstObject *result;
 uint16_t ast_flags = get_seq_flags(preferred_type);
 /* Parse the initial item. */
 if (tok == '.') {
  if unlikely(yield() < 0) goto err;
  if (TPP_ISKEYWORD(tok)) {
   DREF DeeObject *key = DeeString_NewSized(token.t_kwd->k_name,
                                            token.t_kwd->k_size);
   if unlikely(!key) goto err;
   result = ast_sethere(ast_constexpr(key));
   Dee_Decref(key);
   if unlikely(!result) goto err;
   if unlikely(yield() < 0) goto err_r;
  } else {
   if (WARN(W_EXPECTED_KEYWORD_AFTER_BRACE_DOT))
       goto err;
   result = ast_constexpr(Dee_None);
   if unlikely(!result) goto err;
  }
  if unlikely(likely(tok == '=') ? (yield() < 0) :
              WARN(W_EXPECTED_EQUAL_AFTER_BRACE_DOT))
     goto err_r;
  goto parse_dict;
 }
 /* Check for special case: Empty brace initializer. */
 if (!maybe_expression_begin())
      return ast_multiple(ast_flags,0,NULL);
 result = ast_parse_brace(LOOKUP_SYM_NORMAL,preferred_type);
 if unlikely(!result) goto err;
 if (tok == ':') {
  size_t elema,elemc;
  DREF DeeAstObject **elemv,*item;
  /* Dict-style mappings. */
  if unlikely(yield() < 0) goto err_r;
parse_dict:
  /* Parse the associated item. */
  item = ast_parse_brace(LOOKUP_SYM_NORMAL,preferred_type);
  if unlikely(!item) goto err_r;
  elema = 1,elemc = 0;
  elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
  if unlikely(!elemv) goto err_dict_keyitem;
  elemv[0] = result; /* Inherit */
  elemv[1] = item;   /* Inherit */
  ++elemc;
  /* Parse the remainder of a dict initializer. */
  while (tok == ',') {
   if unlikely(yield() < 0) goto err_dict_elemv;
   /* Parse the key expression. */
   if (tok == '.') {
    if unlikely(yield() < 0) goto err_dict_elemv;
    if (TPP_ISKEYWORD(tok)) {
     DREF DeeObject *key = DeeString_NewSized(token.t_kwd->k_name,
                                              token.t_kwd->k_size);
     if unlikely(!key) goto err_dict_elemv;
     result = ast_sethere(ast_constexpr(key));
     Dee_Decref(key);
     if unlikely(!result) goto err_dict_elemv;
     if unlikely(yield() < 0) goto err_r;
    } else {
     if (WARN(W_EXPECTED_KEYWORD_AFTER_BRACE_DOT))
         goto err_dict_elemv_result;
     result = ast_constexpr(Dee_None);
     if unlikely(!result) goto err_dict_elemv;
    }
    if unlikely(likely(tok == '=') ? (yield() < 0) :
                WARN(W_EXPECTED_EQUAL_AFTER_BRACE_DOT))
       goto err_dict_elemv_result;
   } else {
    if (!maybe_expression_begin()) break; /* Allow (and ignore) trailing comma. */
    result = ast_parse_brace(LOOKUP_SYM_NORMAL,preferred_type);
    if unlikely(!result) goto err_dict_elemv;
    if unlikely(likely(tok == ':') ? (yield() < 0) :
                WARN(W_EXPECTED_COLLON_AFTER_DICT_KEY))
       goto err_dict_elemv_result;
   }
   /* Now parse the associated item. */
   item = ast_parse_brace(LOOKUP_SYM_NORMAL,preferred_type);
   if unlikely(!item) goto err_dict_elemv_result;
   /* Extend the element vector if needed. */
   if (elemc == elema) {
    DREF DeeAstObject **new_elemv;
    size_t new_elema = elema*2;
    ASSERT(new_elema);
do_realloc_dict:
    new_elemv = (DREF DeeAstObject **)Dee_TryRealloc(elemv,(new_elema*2)*
                                                     sizeof(DREF DeeAstObject *));
    if unlikely(!new_elemv) {
     if (new_elema != elemc+1) { new_elema = elemc+1; goto do_realloc_dict; }
     if (Dee_CollectMemory((new_elema*2)*sizeof(DREF DeeAstObject *))) goto do_realloc_dict;
     goto err_dict_keyitem;
    }
    elemv = new_elemv;
    elema = new_elema;
   }
   elemv[(elemc * 2) + 0] = result; /* Inherit */
   elemv[(elemc * 2) + 1] = item;   /* Inherit */
   ++elemc;
  }
  if (elemc != elema) {
   DREF DeeAstObject **new_elemv;
   new_elemv = (DREF DeeAstObject **)Dee_TryRealloc(elemv,(elemc*2)*
                                                    sizeof(DREF DeeAstObject *));
   if likely(new_elemv) elemv = new_elemv;
  }
  result = ast_multiple(ast_flags == AST_FMULTIPLE_GENERIC
                      ? AST_FMULTIPLE_GENERIC_KEYS
                      : AST_FMULTIPLE_DICT,elemc*2,elemv);
  if unlikely(!result) goto err_dict_elemv;
  /* NOTE: On success, all items have been inherited by the branch. */
  goto done;
err_dict_keyitem:
  Dee_Decref(item);
err_dict_elemv_result:
  Dee_Decref(result);
err_dict_elemv:
  while (elemc--) {
   Dee_Decref(elemv[(elemc/2) + 0]);
   Dee_Decref(elemv[(elemc/2) + 1]);
  }
  Dee_Free(elemv);
  result = NULL;
  goto done;
 }
 /* Parse a list initializer. */
 {
  DREF DeeAstObject **elemv;
  size_t elema = 1,elemc = 1;
  elemv = (DREF DeeAstObject **)Dee_Malloc(1*sizeof(DREF DeeAstObject *));
  if unlikely(!elemv) goto err_r;
  elemv[0] = result; /* Inherit */
  for (;;) {
   if (tok != ',') {
    if (tok == ':') {
     if (WARN(W_EXPECTED_COMMA_IN_LIST_INITIALIZER))
         goto err_list_elemv;
     if unlikely(yield() < 0) goto err_list_elemv;
     goto parse_list_item;
    }
    break;
   }
parse_list_item:
   if unlikely(yield() < 0) goto err_list_elemv;
   if (!maybe_expression_begin()) break; /* Allow (and ignore) trailing comma. */
   result = ast_parse_brace(LOOKUP_SYM_NORMAL,preferred_type);
   if unlikely(!result) goto err_list_elemv;
   if (elemc == elema) {
    DREF DeeAstObject **new_elemv;
    size_t new_elema = elema*2;
    ASSERT(new_elema);
do_realloc_list:
    new_elemv = (DREF DeeAstObject **)Dee_TryRealloc(elemv,new_elema*
                                                     sizeof(DREF DeeAstObject *));
    if unlikely(!new_elemv) {
     if (new_elema != elemc+1) { new_elema = elemc+1; goto do_realloc_list; }
     if (Dee_CollectMemory(new_elema*sizeof(DREF DeeAstObject *))) goto do_realloc_list;
     goto err_list_elemv_result;
    }
    elemv = new_elemv;
    elema = new_elema;
   }
   elemv[elemc++] = result; /* Inherit. */
  }
  if (elemc != elema) {
   DREF DeeAstObject **new_elemv;
   new_elemv = (DREF DeeAstObject **)Dee_TryRealloc(elemv,elemc*
                                                    sizeof(DREF DeeAstObject *));
   if likely(new_elemv) elemv = new_elemv;
  }
  result = ast_multiple(ast_flags,elemc,elemv);
  if unlikely(!result) goto err_list_elemv;
  /* Upon success, `ast_multiple' inherits the element vector. */
done:
  return result;
err_list_elemv_result:
  Dee_Decref(result);
err_list_elemv:
  while (elemc--) Dee_Decref(elemv[elemc]);
  Dee_Free(elemv);
  goto err;
 }
err_r:
 Dee_Decref(result);
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_BRACE_C */
