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
#ifndef GUARD_DEEMON_COMPILER_ASM_PARSEASM_C
#define GUARD_DEEMON_COMPILER_ASM_PARSEASM_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/tuple.h>
#include <deemon/list.h>
#include <deemon/dict.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/string.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/optimize.h>
#include <deemon/module.h>

#include "../../runtime/strings.h"

#include <string.h>
#include <limits.h>

#ifndef CONFIG_LANGUAGE_NO_ASM
DECL_BEGIN

#ifdef __INTELLISENSE__
INTERN struct asm_symtab     symtab;
#else
#define symtab current_userasm.ua_symtab
#endif

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#if defined(__USE_KOS) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (memcasecmp(a,b,s) == 0)
#define STRCASEEQ(a,b)   (strcasecmp(a,b) == 0)
#elif defined(_MSC_VER) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (_memicmp(a,b,s) == 0)
#define STRCASEEQ(a,b)   (_stricmp(a,b) == 0)
#else
#define MEMCASEEQ(a,b,s)  dee_memcaseeq((uint8_t *)(a),(uint8_t *)(b),s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
 while (s--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a;
  ++b;
 }
 return true;
}
#define STRCASEEQ(a,b)  dee_strcaseeq((char *)(a),(char *)(b))
LOCAL bool dee_strcaseeq(char *a, char *b) {
 while (*a) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a,++b;
 }
 return true;
}
#endif

#define IS_KWD(str) \
 (COMPILER_STRLEN(str) == token.t_kwd->k_size && \
  memcmp(token.t_kwd->k_name,str,sizeof(str)-sizeof(char)) == 0)
#define IS_KWD_NOCASE(str) \
 (COMPILER_STRLEN(str) == token.t_kwd->k_size && \
  MEMCASEEQ(token.t_kwd->k_name,str,sizeof(str)-sizeof(char)))



INTERN int DCALL
userassembler_init(void) {
 /* HINT: The caller will have already zero-initialized
  *      `current_userasm' and `symtab' */
 current_userasm.ua_lasti  = ASM_DELOP;
 current_userasm.ua_asmuid = DeeCompiler_Current->cp_uasm_unique;
 return 0;
}
INTERN void DCALL
userassembler_fini(void) {
 DeeCompiler_Current->cp_uasm_unique = current_userasm.ua_asmuid;
 Dee_Free(symtab.st_map);
}

PRIVATE bool FCALL symtab_rehash(void) {
 struct asm_sym **new_map,**biter,**bend;
 struct asm_sym *sym_iter,*s_next,**bucket;
 size_t old_size = symtab.st_alloc;
 size_t new_size = old_size;
 if (!new_size) new_size = 1;
 new_size *= 2;
rehash_realloc:
 new_map = (struct asm_sym **)Dee_TryCalloc(new_size*sizeof(struct asm_sym *));
 if unlikely(!new_map) {
  if (new_size != 1) { new_size = 1; goto rehash_realloc; }
  if (old_size) {
   if (Dee_TryCollectMemory(sizeof(struct symbol *)))
       goto rehash_realloc;
   return true;
  }
  if (Dee_CollectMemory(sizeof(struct symbol *)))
      goto rehash_realloc;
  return false;
 }
 /* Rehash all symbols. */
 bend = (biter = symtab.st_map)+old_size;
 for (; biter != bend; ++biter) {
  sym_iter = *biter;
  while (sym_iter) {
   s_next = sym_iter->as_uhnxt;
   bucket = &new_map[sym_iter->as_uname->k_id % new_size];
   sym_iter->as_uhnxt = *bucket;
   *bucket = sym_iter;
   sym_iter = s_next;
  }
 }
 Dee_Free(symtab.st_map);
 symtab.st_map   = new_map;
 symtab.st_alloc = new_size;
 return true;
}

INTERN struct asm_sym *FCALL
uasm_label_symbol(struct TPPKeyword *__restrict name) {
 char const *text = name->k_name;
 size_t      size = name->k_size;
 size_t label_number;
 struct text_label *textlbl;
 /* Check if the size is sufficient. */
 if (size < COMPILER_STRLEN(USERLABEL_PREFIX)+1)
     goto not_a_label;
 if (memcmp(text,USERLABEL_PREFIX,sizeof(USERLABEL_PREFIX)-sizeof(char)) != 0)
     goto not_a_label;
 text += COMPILER_STRLEN(USERLABEL_PREFIX);
 size -= COMPILER_STRLEN(USERLABEL_PREFIX);
 label_number = 0;
 while (size) {
  if (!DeeUni_IsDecimal(*text))
       goto not_a_label;
  label_number *= 10;
  label_number += DeeUni_AsDigit(*text);
  ++text,--size;
 }
 /* Check if the given label index actually exists. */
 if (label_number >= current_userasm.ua_labelc)
     goto not_a_label;
 /* Return the associated symbol. */
 textlbl = current_userasm.ua_labelv[label_number].ao_label;
 ASSERT(textlbl);
 if (!textlbl->tl_asym)
      textlbl->tl_asym = asm_newsym();
 return textlbl->tl_asym;
not_a_label:
 return NULL;
}


INTERN struct asm_sym *FCALL
uasm_symbol(struct TPPKeyword *__restrict name) {
 struct asm_sym *result,**presult;
 if (symtab.st_alloc) {
  /* Search for the symbol. */
  result = symtab.st_map[name->k_id % symtab.st_alloc];
  for (; result; result = result->as_uhnxt)
     if (result->as_uname == name)
         goto done;
 }
 /* Add a new symbol. */
 if (symtab.st_size >= symtab.st_alloc &&
    !symtab_rehash()) goto err;
 result = asm_newsym();
 if unlikely(!result) goto err;
 presult = &symtab.st_map[name->k_id % symtab.st_alloc];
 result->as_uname = name;
 result->as_uhnxt = *presult;
 result->as_uprev = NULL;
 *presult = result;
done:
 return result;
err:
 return NULL;
}


INTERN struct asm_sym *FCALL
uasm_fbsymbol(struct TPPKeyword *__restrict name,
              bool return_back_symbol) {
 struct asm_sym *result,**presult;
 if (symtab.st_alloc) {
  /* Search for the symbol. */
  presult = &symtab.st_map[name->k_id % symtab.st_alloc];
  while ((result = *presult) != NULL) {
   if (result->as_uname == name) {
    if (!return_back_symbol) {
     /* Forward reference (if it was already defined, replace the symbol with a copy) */
     if (ASM_SYM_DEFINED(result)) {
      struct asm_sym *new_result;
      new_result = asm_newsym();
      if unlikely(!new_result) goto err;
      /* Override the old symbol with the new one. */
      new_result->as_uname = name;
      new_result->as_uprev = result;
      new_result->as_uhnxt = result->as_uhnxt;
      result->as_uhnxt     = NULL;
      *presult             = new_result;
      result               = new_result;
     }
    } else {
     /* If the symbol isn't defined and has a predecessor, return it instead. */
     if (!ASM_SYM_DEFINED(result) &&
          result->as_uprev)
          result = result->as_uprev;
    }
    goto done;
   }
   presult = &result->as_uhnxt;
  }
 }
 if unlikely(return_back_symbol) {
  /* If the symbol doesn't already exist,
   * backward referencing is illegal. */
  DeeError_Throwf(&DeeError_CompilerError,
                  "Cannot cast backward reference to undefined symbol `%$s'",
                  name->k_size,name->k_name);
  goto err;
 }

 /* Create a new symbol. */
 if (symtab.st_size >= symtab.st_alloc &&
    !symtab_rehash()) goto err;
 result = asm_newsym();
 if unlikely(!result) goto err;
 presult = &symtab.st_map[name->k_id % symtab.st_alloc];
 result->as_uname = name;
 result->as_uhnxt = *presult;
 result->as_uprev = NULL;
 *presult = result;
done:
 return result;
err:
 return NULL;
}
INTERN struct asm_sym *FCALL
uasm_fbsymbol_def(struct TPPKeyword *__restrict name) {
 struct asm_sym *result,**presult;
 if (symtab.st_alloc) {
  /* Search for the symbol. */
  presult = &symtab.st_map[name->k_id % symtab.st_alloc];
  while ((result = *presult) != NULL) {
   if (result->as_uname == name) {
    /* If the symbol has already been defined, create a new one. */
    if (ASM_SYM_DEFINED(result)) {
     struct asm_sym *new_result;
     new_result = asm_newsym();
     if unlikely(!new_result) goto err;
     /* Override the old symbol with the new one. */
     new_result->as_uname = name;
     new_result->as_uprev = result;
     new_result->as_uhnxt = result->as_uhnxt;
     result->as_uhnxt     = NULL;
     *presult             = new_result;
     result               = new_result;
    }
    goto done;
   }
   presult = &result->as_uhnxt;
  }
 }
 /* Create a new symbol. */
 if (symtab.st_size >= symtab.st_alloc &&
    !symtab_rehash()) goto err;
 result = asm_newsym();
 if unlikely(!result) goto err;
 presult = &symtab.st_map[name->k_id % symtab.st_alloc];
 result->as_uname = name;
 result->as_uhnxt = *presult;
 result->as_uprev = NULL;
 *presult = result;
done:
 return result;
err:
 return NULL;
}


#define TOK_IS_SYMBOL_NAME_CH(x) \
  ((x) == '$' || (x) == '.' || (x) == '@' || \
   /*(x) == ':' ||*/ (x) == '%' || (x) == '&')
#define TOK_IS_SYMBOL_NAME(x) \
  (TPP_ISKEYWORD(x) || TOK_IS_SYMBOL_NAME_CH(x))

INTERN struct TPPKeyword *FCALL
uasm_parse_symnam(void) {
 struct TPPKeyword *result;
 char *symbol_start;
 char *symbol_end;
 if (tok == TOK_STRING) {
  /* Special case: String symbol name. */
  struct TPPString *strval;
  strval = TPPLexer_ParseString();
  if unlikely(!strval)
     goto err;
  /* Re-interprete the parsed string as a keyword that is then used as
   * a symbol name (thus allowing _anything_ to appear in a symbol name). */
  result = TPPLexer_LookupKeyword(strval->s_text,
                                  strval->s_size,1);
  TPPString_Decref(strval);
  goto done;
 }
 if (TPP_ISKEYWORD(tok) &&
    !TOK_IS_SYMBOL_NAME_CH(*token.t_end) &&
    !DeeUni_IsSymCont(*token.t_end)) {
  /* Simple case: the following character doesn't continue the symbol's name.
   * In this case, we don't need to re-validate the symbol name. */
  result = token.t_kwd;
  if unlikely(yield() < 0) goto err;
  goto done;
 }
 symbol_start = token.t_begin;
 symbol_end   = token.t_end;
continue_without_inc:
 for (;; ++symbol_end) {
  while (SKIP_WRAPLF(symbol_end,token.t_file->f_end));
  if (symbol_end == token.t_file->f_end) {
   int chunk_state;
   /* Load more input text. */
   *(uintptr_t *)&symbol_start -= (uintptr_t)token.t_file->f_begin;
   *(uintptr_t *)&symbol_end   -= (uintptr_t)token.t_file->f_begin;
   chunk_state = TPPFile_NextChunk(token.t_file,TPPFILE_NEXTCHUNK_FLAG_EXTEND);
   *(uintptr_t *)&symbol_start += (uintptr_t)token.t_file->f_begin;
   *(uintptr_t *)&symbol_end   += (uintptr_t)token.t_file->f_begin;
   if (!chunk_state) break;
   goto continue_without_inc;
  }
  /* We allow unicode symbol characters, as well as
   * some special characters, but no whitespace! */
  if (TOK_IS_SYMBOL_NAME_CH(*symbol_end)) continue;
  if (DeeUni_IsSymCont(*symbol_end)) continue;
  break;
 }
 /* Lookup the keyword for the symbol's name. */
 result = TPPLexer_LookupEscapedKeyword(symbol_start,
                               (size_t)(symbol_end-symbol_start),
                                        1);
 if unlikely(!result) goto err;
 /* Set the file point to continue parsing after the symbol name. */
 token.t_file->f_pos = symbol_end;
 /* Parse the next token following the symbol name. */
 if unlikely(yield() < 0) goto err;
done:
 return result;
err:
 return NULL;
}


PRIVATE int FCALL
uasm_parse_intexpr_unary_base(struct asm_intexpr *result, uint16_t features) {
 switch (tok) {

 case TOK_INT:
 case TOK_FLOAT:
  /* Integer constant. */
  if (!result)
       goto yield_done;
  {
   /* Check for a token like this: `1f.SP' */
   char *int_end = (char *)memchr(token.t_begin,'.',
                         (size_t)(token.t_end-token.t_begin));
   if (int_end) {
    /* Truncate the integer token to not include the dot or anything thereafter. */
    while (SKIP_WRAPLF_REV(int_end,token.t_begin));
    token.t_end         = int_end;
    token.t_file->f_pos = int_end;
   }
  }
  if (token.t_begin[0] != '0' && /* Check leading ZERO for 0xbbff */
     (token.t_end[-1] == 'b' || token.t_end[-1] == 'f')) {
   /* Forward/backward symbol reference. */
   struct TPPKeyword *name;
   name = TPPLexer_LookupEscapedKeyword(token.t_begin,
                              (size_t)((token.t_end-token.t_begin)-1),
                                        1);
   if unlikely(!name) goto err;
   result->ie_sym = uasm_fbsymbol(name,token.t_end[-1] == 'b');
   if unlikely(!result->ie_sym) goto err;
   result->ie_val = 0;
   result->ie_rel = (uint16_t)-1;
   goto yield_done;
  }
  ATTR_FALLTHROUGH
 case TOK_CHAR:
  if (!result)
       goto yield_done;
  /* Character constant. */
  result->ie_sym = NULL;
  result->ie_rel = (uint16_t)-1;
  if unlikely(TPP_Atoi(&result->ie_val) == TPP_ATOF_ERR)
     goto err;
yield_done:
  if unlikely(yield() < 0)
     goto err;
  goto done;

 case TOK_STRING:
  if (!result) {
   do if unlikely(yield() < 0) goto err;
   while (tok == TOK_STRING);
  } else {
   struct TPPString *strval;
   struct TPPKeyword *name;
   strval = TPPLexer_ParseString();
   if unlikely(!strval)
      goto err;
   /* Re-interprete the parsed string as a keyword that is then used as
    * a symbol name (thus allowing _anything_ to appear in a symbol name). */
   name = TPPLexer_LookupKeyword(strval->s_text,
                                 strval->s_size,1);
   TPPString_Decref(strval);
   if unlikely(!name) goto err;
   result->ie_rel = (uint16_t)-1;
   result->ie_val = 0;
   result->ie_sym = uasm_symbol(name);
   if unlikely(!result->ie_sym) goto err;
  }
  goto done;

 case '(':
  /* Parenthesis recursion. */
  if unlikely(yield() < 0) goto err;
  if unlikely(uasm_parse_intexpr(result,features)) goto err;
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_LPAREN))
     goto err;
  break;

 {
  tok_t operation;
  /* Unary operators. */
 case '!': case '~': case '-':
  operation = tok;
  if unlikely(yield() < 0) goto err;
  if unlikely(uasm_parse_intexpr(result,features)) goto err;
  if (result->ie_sym && WARN(W_UASM_CANNOT_PERFORM_OPERATION_WITH_SYMBOL)) goto err;
  /**/ if (operation == '!') result->ie_val = !result->ie_val;
  else if (operation == '~') result->ie_val = ~result->ie_val;
  else /*                 */ result->ie_val = -result->ie_val;
 } break;

 default:
  if ((features&UASM_INTEXPR_FHASSP) &&
       TPP_ISKEYWORD(tok) && IS_KWD_NOCASE("sp")) {
   if unlikely(yield() < 0) goto err;
   if (!result) goto done;
   /* Check if the stack is currently undefined. */
   if (current_userasm.ua_mode&USER_ASM_FSTKINV) {
    DeeError_Throwf(&DeeError_CompilerError,
                    "Cannot retrieve SP while the stack is in an undefined state");
    goto err;
   }
   /* Fill in the current stack depth. */
   result->ie_val = current_assembler.a_stackcur;
   result->ie_sym = NULL;
   result->ie_rel = ASM_OVERLOAD_FSTKABS;
   goto done;
  }
  /* Lookup/defined user-symbols. */
  if (TOK_IS_SYMBOL_NAME(tok)) {
   struct TPPKeyword *name;
   name = uasm_parse_symnam();
   if unlikely(!name) goto err;
   if (!result) goto done;
   result->ie_rel = ASM_OVERLOAD_FRELABS;
   if (name->k_size == 1 && name->k_name[0] == '.') {
    /* Special symbol: The current text address. */
    result->ie_val = 0;
    if (current_assembler.a_syms &&
        current_assembler.a_syms->as_sect ==
       (current_assembler.a_curr - current_assembler.a_sect) &&
        current_assembler.a_syms->as_addr == asm_ip())
        result->ie_sym = current_assembler.a_syms;
    else {
     result->ie_sym = asm_newsym();
     asm_defsym(result->ie_sym);
    }
    goto done;
   }
   result->ie_val = 0;
   if (name->k_size > 3 && (features&UASM_INTEXPR_FHASSP) &&
       name->k_name[name->k_size-3] == '.' &&
      (name->k_name[name->k_size-2] == 's' || name->k_name[name->k_size-2] == 'S' ||
       name->k_name[name->k_size-2] == 'i' || name->k_name[name->k_size-2] == 'I') &&
      (name->k_name[name->k_size-1] == 'p' || name->k_name[name->k_size-1] == 'P')) {
    /* Special case: this expression actually refers to the stack-address of a given symbol. */
    if (name->k_name[name->k_size-2] == 'i' ||
        name->k_name[name->k_size-2] == 'I') {
     result->ie_rel = ASM_OVERLOAD_FRELABS;
    } else {
     result->ie_rel = ASM_OVERLOAD_FSTKABS;
    }
#if 0 /* ??? */
    if (ASM_SYM_DEFINED(result->ie_sym)) {
     result->ie_val += result->ie_sym->as_stck;
     result->ie_sym  = NULL;
    }
#endif
    name = TPPLexer_LookupKeyword(name->k_name,name->k_size-3,1);
    if unlikely(!name) goto err;
   }
   /* Check for user-input label symbols. */
   result->ie_sym = uasm_label_symbol(name);
   if (!result->ie_sym)
        result->ie_sym = uasm_symbol(name);
   if unlikely(!result->ie_sym) goto err;
   if (result->ie_rel == ASM_OVERLOAD_FSTKABS &&
       ASM_SYM_DEFINED(result->ie_sym)) {
    result->ie_val += result->ie_sym->as_stck;
    result->ie_sym  = NULL;
   }
   goto done;
  }
  if (WARN(W_UASM_EXPECTED_INTEXPR))
      goto err;
  if (!result) goto done;
  result->ie_rel = (uint16_t)-1;
  result->ie_sym = NULL;
  result->ie_val = 0;
  goto done;
 }
done:
 return 0;
err:
 return -1;
}
PRIVATE int FCALL
uasm_parse_intexpr_unary(struct asm_intexpr *result, uint16_t features) {
 if unlikely(uasm_parse_intexpr_unary_base(result,features))
    goto err;
again:
 switch (tok) {
 case '.':
  if unlikely(yield() < 0) goto err;
  if (TPP_ISKEYWORD(tok)) {
   /* Explicitly define the relocation mode:
    * >>    push $1f.IP    // Push the address
    * >>    push $1f.SP    // Push the stack-depth
    * >>    jmp  pop, #pop // Do a long-jump with an absolute stack-depth.
    * >>.setstack 42       // Set the absolute stack-depth prior to the following instruction.
    * >>1:
    */
   if (IS_KWD_NOCASE("IP")) {
    if unlikely(yield() < 0) goto err;
    if (!result) goto again;
    if (result->ie_rel != (uint16_t)-1)
        goto err_ipsp_already_defined;
    if (!result->ie_sym)
        goto err_ipsp_no_symbol;
    result->ie_rel = ASM_OVERLOAD_FRELABS;
    goto again;
   }
   if (IS_KWD_NOCASE("SP")) {
    if unlikely(yield() < 0) goto err;
    if (!result) goto again;
    if (result->ie_rel != (uint16_t)-1)
        goto err_ipsp_already_defined;
    if (!result->ie_sym)
        goto err_ipsp_no_symbol;
    result->ie_rel = ASM_OVERLOAD_FSTKABS;
    if (ASM_SYM_DEFINED(result->ie_sym)) {
     result->ie_val += result->ie_sym->as_stck;
     result->ie_sym  = NULL;
    }
    goto again;
   }
  }
  if unlikely(WARN(W_UASM_EXPECTED_IP_OR_SP_AFTER_DOT))
     goto err;
  break;
err_ipsp_no_symbol:
  if unlikely(WARN(W_UASM_NEED_SYMBOL_FOR_RELOCATION_MODEL))
     goto err;
  goto again;
err_ipsp_already_defined:
  if unlikely(WARN(W_UASM_RELOCATION_MODEL_ALREADY_DEFINED))
     goto err;
  goto again;
 default: break;
 }
 return 0;
err:
 return -1;
}

PRIVATE int FCALL
uasm_parse_intexpr_sum(struct asm_intexpr *result, uint16_t features) {
 if unlikely(uasm_parse_intexpr_unary(result,features))
    goto err;
 while (tok == '+' || tok == '-') {
  tok_t mode = tok;
  if unlikely(yield() < 0) goto err;
  if (!result) {
   if unlikely(uasm_parse_intexpr_unary(NULL,features))
      goto err;
  } else {
   struct asm_intexpr other;
   if unlikely(uasm_parse_intexpr_unary(&other,features))
      goto err;
   /* Combine the 2 operands. */
   if (mode == '+') {
    if (result->ie_sym ||
       (result->ie_rel != (uint16_t)-1 &&
        result->ie_rel != other.ie_rel)) {
     if (other.ie_sym && WARN(W_UASM_CANNOT_ADD_2_SYMBOLS))
         goto err;
    } else {
     result->ie_rel = other.ie_rel;
     result->ie_sym = other.ie_sym;
    }
    result->ie_val += other.ie_val;
    continue;
   }
   /* Special case: The same symbol on both sides cancels out itself. */
   if (result->ie_sym == other.ie_sym ||
      (result->ie_sym && other.ie_sym &&
       ASM_SYM_DEFINED(result->ie_sym) &&
       result->ie_sym->as_sect == other.ie_sym->as_sect &&
       result->ie_sym->as_addr == other.ie_sym->as_addr)) {
    result->ie_sym = NULL;
    other.ie_sym   = NULL;
   }
   if (!other.ie_sym && other.ie_rel == (uint16_t)-1) {
    /* `... - 42' */
   } else if (!result->ie_sym &&
               result->ie_rel == ASM_OVERLOAD_FSTKABS &&
               other.ie_rel == ASM_OVERLOAD_FSTKABS) {
    /* `SP - other.SP' */
    result->ie_rel = ASM_OVERLOAD_FSTKDSP;
    result->ie_sym = other.ie_sym;
   } else if ((!result->ie_sym || !other.ie_sym) &&
              ((result->ie_rel == ASM_OVERLOAD_FSTKDSP && other.ie_rel == ASM_OVERLOAD_FSTKABS) ||
               (result->ie_rel == ASM_OVERLOAD_FSTKABS && other.ie_rel == ASM_OVERLOAD_FSTKDSP) ||
               (result->ie_rel == ASM_OVERLOAD_FSTKDSP && other.ie_rel == ASM_OVERLOAD_FSTKDSP))) {
    if (other.ie_rel   == ASM_OVERLOAD_FSTKDSP &&
        result->ie_rel == ASM_OVERLOAD_FSTKDSP) {
     /* `(SP - other.SP) - (SP - other.SP)' */
     result->ie_rel = (uint16_t)-1;
    } else if (other.ie_rel == ASM_OVERLOAD_FSTKABS) {
     /* `SP - (SP - other.SP)' */
     result->ie_rel = ASM_OVERLOAD_FSTKABS;
    } else {
     /* `(SP - other.SP) - SP' */
     result->ie_rel = ASM_OVERLOAD_FSTKDSP;
    }
    if (other.ie_sym)
        result->ie_sym = other.ie_sym;
   } else if (result->ie_sym && other.ie_sym && ASM_SYM_DEFINED(other.ie_sym)) {
    /* IP - . */
#if ((ASM_OVERLOAD_FRELABS|1) == ASM_OVERLOAD_FRELDSP) && \
    ((ASM_OVERLOAD_FSTKABS|1) == ASM_OVERLOAD_FSTKDSP)
    result->ie_rel = other.ie_rel | 1;
#else
    result->ie_rel = other.ie_rel == ASM_OVERLOAD_FRELABS
                   ? ASM_OVERLOAD_FRELDSP
                   : ASM_OVERLOAD_FSTKDSP;
#endif
   } else {
    if (WARN(W_UASM_CANNOT_SUB_2_SYMBOLS))
        goto err;
   }
   result->ie_val -= other.ie_val;
   continue;
  }
 }
 return 0;
err:
 return -1;
}

INTERN int FCALL
uasm_parse_intexpr(struct asm_intexpr *result, uint16_t features) {
 /* TODO: All the other expression levels. */
 return uasm_parse_intexpr_sum(result,features);
}

INTERN int32_t FCALL
uasm_parse_imm16(uint16_t features) {
 struct asm_intexpr result;
 if unlikely(uasm_parse_intexpr(&result,features))
    goto err;
 /* Warn if the parsed value is out-of-bounds. */
 if unlikely((result.ie_sym || result.ie_val < 0 ||
              result.ie_val > UINT16_MAX ||
              result.ie_rel != (uint16_t)-1) &&
              WARN(W_UASM_EXPECTED_16BIT_IMMEDIATE_INTEGER))
    goto err;
 return (int32_t)(uint32_t)(uint16_t)result.ie_val;
err:
 return -1;
}



/* Helper functions for parsing the arguments of symbol class operands. */
PRIVATE int32_t FCALL
do_parse_module_operands(void) {
 int32_t result;
 /* Parse a module by name. */
 if (tok == '@') {
  DREF DeeModuleObject *mod;
  if unlikely(yield() < 0) goto err;
  mod = parse_module_byname(true);
  if unlikely(!mod) goto err;
  /* Add the module to the assembler's import list. */
  result = asm_newmodule(mod);
  Dee_Decref(mod);
 } else {
  result = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
 }
 return result;
err:
 return -1;
}
PRIVATE int FCALL
do_parse_extern_operands(uint16_t *__restrict pmid,
                         uint16_t *__restrict pgid) {
 DREF DeeModuleObject *module;
 int32_t temp;
 /* Parse a module by name. */
 if (tok == '@') {
  if unlikely(yield() < 0) goto err;
  module = parse_module_byname(true);
  if unlikely(!module) goto err;
  /* Add the module to the assembler's import list. */
  temp = asm_newmodule(module);
  if unlikely(temp < 0) goto err;
 } else {
  temp = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
  if unlikely(temp < 0) goto err;
  /* Check if we can locate the module in our import list. */
  module = NULL;
  if ((uint16_t)temp < current_rootscope->rs_importc) {
   module = current_rootscope->rs_importv[(uint16_t)temp];
   Dee_Incref(module);
  }
 }
 *pmid = (uint16_t)temp;
 /* Now parse the symbol that is imported from this module. */
 if unlikely(likely(tok == ':') ? (yield() < 0) :
             WARN(W_UASM_EXPECTED_COLLON_AFTER_EXTERN_PREFIX))
    goto err;
 /* If the module name was given, allow the associated symbol to be addressed by name. */
 if (tok == '@' && module) {
  struct TPPKeyword *symbol_name;
  struct module_symbol *modsym;
  if unlikely(yield() < 0) goto err_module;
  symbol_name = uasm_parse_symnam();
  if unlikely(!symbol_name) goto err_module;
  modsym = import_module_symbol(module,symbol_name);
  if unlikely(!modsym) {
   if (WARN(W_MODULE_IMPORT_NOT_FOUND,
            symbol_name->k_name,
            DeeString_STR(module->mo_name)))
       goto err_module;
   *pgid = 0;
  } else {
   *pgid = modsym->ss_index;
  }
 } else {
  temp = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
  if unlikely(temp < 0) goto err_module;
  *pgid = (uint16_t)temp;
 }
 Dee_XDecref(module);
 return 0;
err_module:
 Dee_XDecref(module);
err:
 return -1;
}

PRIVATE ATTR_COLD void FCALL
err_unknown_symbol(struct TPPKeyword *__restrict name) {
 DeeError_Throwf(&DeeError_CompilerError,
                 "Unknown symbol `%s'",
                 name->k_name);
}

PRIVATE int32_t FCALL do_parse_global_operands(void) {
 int32_t result;
 struct symbol *sym;
 if (tok == '@') {
  struct TPPKeyword *symbol_name;
  if unlikely(yield() < 0) goto err;
  symbol_name = uasm_parse_symnam();
  if unlikely(!symbol_name) goto err;
  /* Allow global variables to be addressed by name. */
  sym = scope_lookup(&current_rootscope->rs_scope.bs_scope,
                      symbol_name);
  if (!sym) {
   err_unknown_symbol(symbol_name);
   goto err;
  }
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  if (sym->s_type != SYMBOL_TYPE_GLOBAL) {
   DeeError_Throwf(&DeeError_CompilerError,
                   "Symbol `%s' is not a global symbol",
                   symbol_name->k_name);
   goto err;
  }
  /* Bind the given symbol as a global item. */
  result = asm_gsymid(sym);
 } else {
  result = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
 }
 return result;
err:
 return -1;
}
PRIVATE int32_t FCALL do_parse_stack_operands(void) {
 if unlikely(likely(tok == '#') ? (yield() < 0) :
             WARN(W_UASM_EXPECTED_HASH_AFTER_STACK_PREFIX))
    return -1;
 return uasm_parse_imm16(UASM_INTEXPR_FHASSP);
}
PRIVATE int32_t FCALL do_parse_local_operands(void) {
 int32_t result;
 struct symbol *sym;
 if (tok == '@') {
  struct TPPKeyword *symbol_name;
  DeeScopeObject *scope_iter;
  if unlikely(yield() < 0) goto err;
  symbol_name = uasm_parse_symnam();
  if unlikely(!symbol_name) goto err;
  /* Allow local variables to be addressed by name. */
  scope_iter = current_scope;
  /* Look for local variables in all active scopes of the current function. */
  for (;;) {
   sym = scope_lookup(scope_iter,symbol_name);
   if (sym) break;
   scope_iter = scope_iter->s_prev;
   if (!scope_iter) break;
   if (scope_iter->s_base != current_basescope) break;
  }
  if (!sym) {
   err_unknown_symbol(symbol_name);
   goto err;
  }
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  if (sym->s_type != SYMBOL_TYPE_LOCAL ||
      SYMBOL_MUST_REFERENCE_TYPEMAY(sym)) {
   DeeError_Throwf(&DeeError_CompilerError,
                   "Symbol `%s' is not a local symbol",
                   symbol_name->k_name);
   goto err;
  }
  /* Bind the given symbol as a local item. */
  result = asm_lsymid(sym);
 } else {
  result = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
 }
 return result;
err:
 return -1;
}
PRIVATE int32_t FCALL do_parse_constexpr(void) {
 DREF struct ast *imm_const;
 DREF DeeObject *const_val;
 int32_t cid;
 if unlikely(scope_push()) goto err;
 imm_const = ast_parse_expr(LOOKUP_SYM_NORMAL);
 scope_pop();
 if unlikely(!imm_const) goto err;
 /* Optimize the constant branch to allow for constant propagation. */
 if unlikely(ast_optimize_all(imm_const,true)) {
err_imm_const:
  ast_decref(imm_const);
  goto err;
 }
 if (imm_const->a_type == AST_CONSTEXPR &&
     asm_allowconst(imm_const->a_constexpr)) {
  const_val = imm_const->a_constexpr;
 } else {
  if (WARN(W_UASM_EXPECTED_CONSTANT_EXPRESSION_AFTER_AT_CONST))
      goto err_imm_const;
  const_val = Dee_None;
 }
 cid = asm_newconst(const_val);
 ast_decref(imm_const);
 return cid;
err:
 return -1;
}
PRIVATE int32_t FCALL do_parse_const_operands(void) {
 int32_t result;
 if (tok == '@') {
  if unlikely(yield() < 0) goto err;
  /* Parse a regular, constant expression. */
  result = do_parse_constexpr();
 } else {
  /* Parse the constant index itself. */
  result = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
 }
 return result;
err:
 return -1;
}
PRIVATE int32_t FCALL do_parse_arg_operands(void) {
 int32_t result;
 struct symbol *sym;
 if (tok == '@') {
  struct TPPKeyword *symbol_name;
  if unlikely(yield() < 0) goto err;
  symbol_name = uasm_parse_symnam();
  if unlikely(!symbol_name) goto err;
  /* Allow argument variables to be addressed by name. */
  sym = scope_lookup(&current_basescope->bs_scope,
                      symbol_name);
  if (!sym) {
   err_unknown_symbol(symbol_name);
   goto err;
  }
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  if (SYMBOL_TYPE(sym) != SYMBOL_TYPE_ARG) {
   DeeError_Throwf(&DeeError_CompilerError,
                   "Symbol `%s' is not an argument symbol",
                   symbol_name->k_name);
   goto err;
  }
  if (!DeeBaseScope_IsArgReqOrDefl(current_basescope,sym->s_symid) &&
     (!DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid) ||
       DeeBaseScope_HasOptional(current_basescope))) {
   DeeError_Throwf(&DeeError_CompilerError,
                   "Argument `%s' cannot be addressed as a regular argument",
                   symbol_name->k_name);
   goto err;
  }
  /* Link the symbol's argument index. */
  result = sym->s_symid;
 } else {
  result = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
 }
 return result;
err:
 return -1;
}
PRIVATE int32_t FCALL do_parse_ref_operands(void) {
 int32_t result;
 struct symbol *sym;
 if (tok == '@') {
  struct TPPKeyword *symbol_name;
  DeeScopeObject *scope_iter;
  if unlikely(yield() < 0) goto err;
  symbol_name = uasm_parse_symnam();
  if unlikely(!symbol_name) goto err;
  /* Look for the variable outside the current function. */
  sym = NULL;
  scope_iter = current_basescope->bs_scope.s_prev;
  while (scope_iter) {
   sym = scope_lookup(&current_basescope->bs_scope,
                       symbol_name);
   /* Only consider symbols that may be referenced. */
   if (sym && SYMBOL_MAY_REFERENCE(sym)) break;
   scope_iter = scope_iter->s_prev;
  }
  if (!sym || !SYMBOL_MAY_REFERENCE(sym)) {
   err_unknown_symbol(symbol_name);
   goto err;
  }
  /* Link and lookup the symbol's reference index. */
  result = asm_rsymid(sym);
 } else {
  result = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
 }
 return result;
err:
 return -1;
}
PRIVATE int32_t FCALL do_parse_static_operands(void) {
 int32_t result;
 struct symbol *sym;
 if (tok == '@') {
  struct TPPKeyword *symbol_name;
  DeeScopeObject *scope_iter;
  if unlikely(yield() < 0) goto err;
  symbol_name = uasm_parse_symnam();
  if unlikely(!symbol_name) goto err;
  /* Allow static variables to be addressed by name. */
  scope_iter = current_scope;
  /* Look for static variables in all active scopes of the current function. */
  for (;;) {
   sym = scope_lookup(scope_iter,symbol_name);
   if (sym) break;
   scope_iter = scope_iter->s_prev;
   if (!scope_iter) break;
   if (scope_iter->s_base != current_basescope) break;
  }
  if (!sym) {
   err_unknown_symbol(symbol_name);
   goto err;
  }
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  if (sym->s_type != SYMBOL_TYPE_STATIC ||
      SYMBOL_MUST_REFERENCE_TYPEMAY(sym)) {
   DeeError_Throwf(&DeeError_CompilerError,
                   "Symbol `%s' is not a static symbol",
                   symbol_name->k_name);
   goto err;
  }
  /* Bind the given symbol as a static item. */
  result = asm_ssymid(sym);
 } else {
  result = uasm_parse_imm16(UASM_INTEXPR_FNORMAL);
 }
 return result;
err:
 return -1;
}

PRIVATE void FCALL
asm_invoke_operand_determine_intclass(struct asm_invoke_operand *__restrict self) {
 if (self->io_intexpr.ie_val < 0) {
  if (self->io_intexpr.ie_val < INT16_MIN ||
     (self->io_intexpr.ie_sym &&
     (current_assembler.a_flag&ASM_FBIGCODE))) {
   self->io_class = OPERAND_CLASS_SDISP32;
  } else if (self->io_intexpr.ie_val < INT8_MIN ||
             self->io_intexpr.ie_sym != NULL) {
   self->io_class = OPERAND_CLASS_SDISP16;
  } else if (self->io_intexpr.ie_val == -2) {
   self->io_class = OPERAND_CLASS_DISP_EQ_N2;
  } else if (self->io_intexpr.ie_val == -1) {
   self->io_class = OPERAND_CLASS_DISP_EQ_N1;
  } else {
   self->io_class = OPERAND_CLASS_SDISP8;
  }
 } else {
  if (!(self->io_intexpr.ie_val&1) &&
       !self->io_intexpr.ie_sym &&
        self->io_intexpr.ie_val >= 3) {
   /* Check for special half-classes. */
   if ((self->io_intexpr.ie_val >> 1) <= UINT8_MAX)
   { self->io_class = OPERAND_CLASS_DISP8_HALF; return; }
   if ((self->io_intexpr.ie_val >> 1) <= UINT16_MAX)
   { self->io_class = OPERAND_CLASS_DISP16_HALF; return; }
  }
  if (self->io_intexpr.ie_val > UINT16_MAX ||
     (self->io_intexpr.ie_sym &&
     (current_assembler.a_flag&ASM_FBIGCODE))) {
   self->io_class = OPERAND_CLASS_DISP32;
  } else if (self->io_intexpr.ie_sym != NULL) {
   self->io_class = OPERAND_CLASS_DISP16;
  } else if (self->io_intexpr.ie_val > UINT8_MAX) {
   self->io_class = OPERAND_CLASS_DISP16;
  } else if (self->io_intexpr.ie_val == 2) {
   self->io_class = OPERAND_CLASS_DISP_EQ_2;
  } else if (self->io_intexpr.ie_val == 1) {
   self->io_class = OPERAND_CLASS_DISP_EQ_1;
  } else if (self->io_intexpr.ie_val == 0) {
   self->io_class = OPERAND_CLASS_DISP_EQ_0;
  } else {
   self->io_class = OPERAND_CLASS_DISP8;
  }
 }
}

PRIVATE int FCALL
do_translate_operand_ast(struct asm_invoke_operand *__restrict result,
                         struct ast *__restrict expr) {
 switch (expr->a_type) {

 case AST_MULTIPLE:
  /* Unwind single-expression multi-branch ASTs, regardless of scope visibility. */
  if (expr->a_multiple.m_astc == 1)
      return do_translate_operand_ast(result,expr->a_multiple.m_astv[0]);
  goto unsupported_expression;
 case AST_EXPAND:
  if unlikely(do_translate_operand_ast(result,expr->a_expand))
     goto err;
  if (result->io_class&OPERAND_CLASS_FDOTSFLAG &&
      WARN(W_UASM_DOTS_FLAG_ALREADY_SET_FOR_OPERAND))
      goto err;
  /* Set the dots-flag. */
  result->io_class |= OPERAND_CLASS_FDOTSFLAG;
  break;

 {
  DeeObject *constval;
  int32_t symid;
 case AST_CONSTEXPR:
  constval = expr->a_constexpr;
  if unlikely(!asm_allowconst(constval))
     goto unsupported_expression;
#if 0 /* Don't do this. - Otherwise something like `print @20, nl' wouldn't work because
       * it would get interpreted as `print $20, nl' rather than `print const @20, nl' */
  if (!DeeInt_Check(constval))
       goto allocate_constant;
  /* Special handling for integer constant expressions. */
  if (!DeeInt_TryAsS64(constval,&result->io_intexpr.ie_val))
       goto allocate_constant;
  result->io_intexpr.ie_sym = NULL;
  result->io_intexpr.ie_rel = (uint16_t)-1;
  asm_invoke_operand_determine_intclass(result);
  result->io_class |= OPERAND_CLASS_FIMMVAL;
  break;
allocate_constant:
#endif
  /* Fallback: Allocate the operand as a constant variable. */
  symid = asm_newconst(constval);
  if unlikely(symid < 0) goto err;
  result->io_class = OPERAND_CLASS_CONST;
  result->io_symid = (uint16_t)symid;
 } break;

 {
  struct symbol *sym;
  int32_t symid;
 case AST_SYM:
  /* Symbol expression. */
  sym = expr->a_sym;
check_sym_class:
  if (SYMBOL_MUST_REFERENCE(sym)) {
   symid = asm_rsymid(sym);
   if unlikely(symid < 0) goto err;
   result->io_class = OPERAND_CLASS_REF;
   result->io_symid = (uint16_t)symid;
  } else {
   switch (SYMBOL_TYPE(sym)) {
  
   case SYMBOL_TYPE_ALIAS:
    sym = SYMBOL_ALIAS(sym);
    goto check_sym_class;

   case SYMBOL_TYPE_EXTERN:
    symid = asm_esymid(sym);
    if unlikely(symid < 0) goto err;
    result->io_class           = OPERAND_CLASS_EXTERN;
    result->io_extern.io_modid = (uint16_t)symid;
    result->io_extern.io_symid = SYMBOL_EXTERN_SYMBOL(sym)->ss_index;
    break;

   case SYMBOL_TYPE_GLOBAL:
    symid = asm_gsymid(sym);
    if unlikely(symid < 0) goto err;
    result->io_class = OPERAND_CLASS_GLOBAL;
    result->io_symid = (uint16_t)symid;
    break;
   case SYMBOL_TYPE_LOCAL:
    symid = asm_lsymid(sym);
    if unlikely(symid < 0) goto err;
    result->io_class = OPERAND_CLASS_LOCAL;
    result->io_symid = (uint16_t)symid;
    break;
   case SYMBOL_TYPE_STATIC:
    symid = asm_ssymid(sym);
    if unlikely(symid < 0) goto err;
    result->io_class = OPERAND_CLASS_STATIC;
    result->io_symid = (uint16_t)symid;
    break;

   case SYMBOL_TYPE_STACK:
    result->io_intexpr.ie_rel = (uint16_t)-1;
    result->io_intexpr.ie_sym = NULL;
    result->io_intexpr.ie_val = SYMBOL_STACK_OFFSET(sym);
    if (!(sym->s_flag & SYMBOL_FALLOC)) {
     if (WARN(W_UASM_STACK_VARIABLE_NOT_ALLOCATED))
         goto err;
     result->io_intexpr.ie_val = 0;
    }
    asm_invoke_operand_determine_intclass(result);
    result->io_class |= OPERAND_CLASS_FSTACKFLAG;
    break;

   case SYMBOL_TYPE_ARG:
    if (!DeeBaseScope_IsArgReqOrDefl(current_basescope,sym->s_symid) &&
       (!DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid) ||
         DeeBaseScope_HasOptional(current_basescope)))
         goto unsupported_expression;
    result->io_class = OPERAND_CLASS_ARG;
    result->io_symid = sym->s_symid;
    break;

   case SYMBOL_TYPE_MODULE:
    symid = asm_msymid(sym);
    if unlikely(symid < 0) goto err;
    result->io_class = OPERAND_CLASS_MODULE;
    result->io_symid = (uint16_t)symid;
    break;

   case SYMBOL_TYPE_EXCEPT:
    result->io_class = OPERAND_CLASS_EXCEPT;
    break;

   case SYMBOL_TYPE_MYMOD:
    result->io_class = OPERAND_CLASS_THIS_MODULE;
    break;

   case SYMBOL_TYPE_MYFUNC:
    result->io_class = OPERAND_CLASS_THIS_FUNCTION;
    break;

   case SYMBOL_TYPE_THIS:
    result->io_class = OPERAND_CLASS_THIS;
    break;

   default:
    goto unsupported_expression;
   }
  }
  break;
 }

 default:
unsupported_expression:
  if (WARN(W_UASM_UNSUPPORTED_EXPRESSION_FOR_AT_OPERAND))
      goto err;
  break;
 }
 return 0;
err:
 return -1;
}

/* Parse a deemon-level expression following `@' and
 * try to convert it into an assembly invocation operand.
 * In order words: accept pretty much all symbols, as well as constants. */
PRIVATE int FCALL
do_parse_atoperand(struct asm_invoke_operand *__restrict result) {
 DREF struct ast *imm_expr;
 if unlikely(scope_push()) goto err;
 imm_expr = ast_parse_expr(LOOKUP_SYM_NORMAL);
 scope_pop();
 if unlikely(!imm_expr) goto err;
 /* Optimize the constant branch to allow for constant propagation. */
 if unlikely(ast_optimize_all(imm_expr,true))
    goto err_imm_expr;
 if unlikely(do_translate_operand_ast(result,imm_expr))
    goto err_imm_expr;
 ast_decref(imm_expr);
 return 0;
err_imm_expr:
 ast_decref(imm_expr);
err:
 return -1;
}


/* @param: recognize_sp: When true, recognize `sp', as seen as operand of `print'.
 *                       Otherwise, `sp' is recognized as representative of the
 *                       current stack depth. */
PRIVATE int FCALL
do_parse_operand(struct asm_invoke_operand *__restrict result,
                 bool recognize_sp) {
 /* Parse the actual operand. */
 switch (tok) {

 case '{':
  /* the value is surrounded by braces. */
  if unlikely(yield() < 0) goto err;
  if unlikely(do_parse_operand(result,false)) goto err;
  if unlikely(likely(tok == '}') ? (yield() < 0) :
              WARN(W_UASM_EXPECTED_RBRACE_AFTER_LBRACE_IN_OPERAND))
     goto err;
  /* Set the brace flag in the operand class. */
  result->io_class |= OPERAND_CLASS_FBRACEFLAG;
  break;

 case '[':
  /* the value is surrounded by braces. */
  if unlikely(yield() < 0) goto err;
  if unlikely(do_parse_operand(result,false)) goto err;
  if unlikely(likely(tok == ']') ? (yield() < 0) :
              WARN(W_UASM_EXPECTED_RBRACKET_AFTER_LBRACKET_IN_OPERAND))
     goto err;
  /* Set the brace flag in the operand class. */
  result->io_class |= OPERAND_CLASS_FBRACKETFLAG;
  break;

parse_stack_operand:
  if unlikely(yield() < 0) goto err;
  if (tok != '#') {
   if (WARN(W_UASM_EXPECTED_HASH_AFTER_STACK_OPERAND))
       goto err;
   goto parse_stack_operand_start;
  }
  ATTR_FALLTHROUGH
 case '#':
  if unlikely(yield() < 0) goto err;
parse_stack_operand_start:
  if unlikely(do_parse_operand(result,false)) goto err;
  /* Set the stack-prefix flag. */
  if (result->io_class&OPERAND_CLASS_FSTACKFLAG) {
   if (result->io_class&OPERAND_CLASS_FSTACKFLAG2 &&
       WARN(W_UASM_HASH_FLAG_ALREADY_SET_FOR_OPERAND))
       goto err;
   result->io_class |= OPERAND_CLASS_FSTACKFLAG2;
  }
  result->io_class |= OPERAND_CLASS_FSTACKFLAG;
  break;

 case '$':
  if unlikely(yield() < 0) goto err;
  if unlikely(do_parse_operand(result,false)) goto err;
  /* Set the immediate-prefix flag. */
  if (result->io_class&OPERAND_CLASS_FIMMVAL &&
      WARN(W_UASM_DOLLAR_FLAG_ALREADY_SET_FOR_OPERAND))
      goto err;
  result->io_class |= OPERAND_CLASS_FIMMVAL;
  break;

 case '@':
  /* Immediate constant expression (Very useful for strings). */
  if unlikely(yield() < 0) goto err;
  if unlikely(do_parse_atoperand(result)) goto err;
  break;



 {
  int32_t val;
parse_ref_operand:
  if unlikely(yield() < 0) goto err;
  val = do_parse_ref_operands();
  if unlikely(val < 0) goto err;
  result->io_class = OPERAND_CLASS_ARG;
  result->io_symid = (uint16_t)val;
 } break;

 {
  int32_t val;
parse_arg_operand:
  if unlikely(yield() < 0) goto err;
  val = do_parse_arg_operands();
  if unlikely(val < 0) goto err;
  result->io_class = OPERAND_CLASS_ARG;
  result->io_symid = (uint16_t)val;
 } break;

 {
  int32_t val;
parse_const_operand:
  if unlikely(yield() < 0) goto err;
  val = do_parse_const_operands();
  if unlikely(val < 0) goto err;
  result->io_class = OPERAND_CLASS_CONST;
  result->io_symid = (uint16_t)val;
 } break;

 {
  int32_t val;
 case KWD_static:
parse_static_operand:
  if unlikely(yield() < 0) goto err;
  val = do_parse_static_operands();
  if unlikely(val < 0) goto err;
  result->io_class = OPERAND_CLASS_STATIC;
  result->io_symid = (uint16_t)val;
 } break;

 {
  int32_t val;
parse_module_operand:
  if unlikely(yield() < 0) goto err;
  val = do_parse_module_operands();
  if unlikely(val < 0) goto err;
  result->io_class = OPERAND_CLASS_MODULE;
  result->io_symid = (uint16_t)val;
 } break;

 {
parse_extern_operand:
  /* Parse a module by name. */
  if unlikely(yield() < 0) goto err;
  if unlikely(do_parse_extern_operands(&result->io_extern.io_modid,
                                       &result->io_extern.io_symid))
     goto err;
  result->io_class = OPERAND_CLASS_EXTERN;
 } break;

 {
  int32_t val;
 case KWD_global:
parse_global_operand:
  if unlikely(yield() < 0) goto err;
  val = do_parse_global_operands();
  if unlikely(val < 0) goto err;
  result->io_class = OPERAND_CLASS_GLOBAL;
  result->io_symid = (uint16_t)val;
 } break;

 {
  int32_t val;
 case KWD_local:
parse_local_operand:
  if unlikely(yield() < 0) goto err;
  val = do_parse_local_operands();
  if unlikely(val < 0) goto err;
  result->io_class = OPERAND_CLASS_LOCAL;
  result->io_symid = (uint16_t)val;
 } break;

 default:
  if (TPP_ISKEYWORD(tok)) {
   if (IS_KWD_NOCASE("pop")) { result->io_class = OPERAND_CLASS_POP; goto done_yield_1; }
   if (IS_KWD_NOCASE("top")) { result->io_class = OPERAND_CLASS_TOP; goto done_yield_1; }
   if (IS_KWD_NOCASE("ref")) goto parse_ref_operand;
   if (IS_KWD_NOCASE("arg")) goto parse_arg_operand;
   if (IS_KWD_NOCASE("const")) goto parse_const_operand;
   if (IS_KWD_NOCASE("static")) goto parse_static_operand;
   if (IS_KWD_NOCASE("module")) goto parse_module_operand;
   if (IS_KWD_NOCASE("extern")) goto parse_extern_operand;
   if (IS_KWD_NOCASE("global")) goto parse_global_operand;
   if (IS_KWD_NOCASE("local")) goto parse_local_operand;
   if (IS_KWD_NOCASE("stack")) goto parse_stack_operand;
   if (IS_KWD_NOCASE("none")) { case KWD_none: result->io_class = OPERAND_CLASS_NONE; goto done_yield_1; }
   if (IS_KWD_NOCASE("foreach")) { result->io_class = OPERAND_CLASS_FOREACH; goto done_yield_1; }
   if (IS_KWD_NOCASE("except")) { result->io_class = OPERAND_CLASS_EXCEPT; goto done_yield_1; }
   if (IS_KWD_NOCASE("catch")) { case KWD_catch: result->io_class = OPERAND_CLASS_CATCH; goto done_yield_1; }
   if (IS_KWD_NOCASE("finally")) { case KWD_finally: result->io_class = OPERAND_CLASS_FINALLY; goto done_yield_1; }
   if (IS_KWD_NOCASE("this")) { case KWD_this: result->io_class = OPERAND_CLASS_THIS; goto done_yield_1; }
   if (IS_KWD_NOCASE("this_module")) { result->io_class = OPERAND_CLASS_THIS_MODULE; goto done_yield_1; }
   if (IS_KWD_NOCASE("this_function")) { result->io_class = OPERAND_CLASS_THIS_FUNCTION; goto done_yield_1; }
   if (IS_KWD_NOCASE("true")) { case KWD_true: result->io_class = OPERAND_CLASS_TRUE; goto done_yield_1; }
   if (IS_KWD_NOCASE("false")) { case KWD_false: result->io_class = OPERAND_CLASS_FALSE; goto done_yield_1; }
   if (IS_KWD_NOCASE("list")) { result->io_class = OPERAND_CLASS_LIST; goto done_yield_1; }
   if (IS_KWD_NOCASE("tuple")) { result->io_class = OPERAND_CLASS_TUPLE; goto done_yield_1; }
   if (IS_KWD_NOCASE("hashset")) { result->io_class = OPERAND_CLASS_HASHSET; goto done_yield_1; }
   if (IS_KWD_NOCASE("dict")) { result->io_class = OPERAND_CLASS_DICT; goto done_yield_1; }
   if (IS_KWD_NOCASE("int")) { result->io_class = OPERAND_CLASS_INT; goto done_yield_1; }
   if (IS_KWD_NOCASE("bool")) { result->io_class = OPERAND_CLASS_BOOL; goto done_yield_1; }
   if (IS_KWD_NOCASE("eq")) { result->io_class = OPERAND_CLASS_EQ; goto done_yield_1; }
   if (IS_KWD_NOCASE("ne")) { result->io_class = OPERAND_CLASS_NE; goto done_yield_1; }
   if (IS_KWD_NOCASE("lo")) { result->io_class = OPERAND_CLASS_LO; goto done_yield_1; }
   if (IS_KWD_NOCASE("le")) { result->io_class = OPERAND_CLASS_LE; goto done_yield_1; }
   if (IS_KWD_NOCASE("gr")) { result->io_class = OPERAND_CLASS_GR; goto done_yield_1; }
   if (IS_KWD_NOCASE("ge")) { result->io_class = OPERAND_CLASS_GE; goto done_yield_1; }
   if (IS_KWD_NOCASE("so")) { result->io_class = OPERAND_CLASS_SO; goto done_yield_1; }
   if (IS_KWD_NOCASE("do")) { case KWD_do: result->io_class = OPERAND_CLASS_DO; goto done_yield_1; }
   if (IS_KWD_NOCASE("break")) { case KWD_break: result->io_class = OPERAND_CLASS_BREAK; goto done_yield_1; }
   if (IS_KWD_NOCASE("min")) { result->io_class = OPERAND_CLASS_MIN; goto done_yield_1; }
   if (IS_KWD_NOCASE("max")) { result->io_class = OPERAND_CLASS_MAX; goto done_yield_1; }
   if (IS_KWD_NOCASE("sum")) { result->io_class = OPERAND_CLASS_SUM; goto done_yield_1; }
   if (IS_KWD_NOCASE("any")) { result->io_class = OPERAND_CLASS_ANY; goto done_yield_1; }
   if (IS_KWD_NOCASE("all")) { result->io_class = OPERAND_CLASS_ALL; goto done_yield_1; }
   if (IS_KWD_NOCASE("sp") && recognize_sp) { result->io_class = OPERAND_CLASS_SP; goto done_yield_1; }
   if (IS_KWD_NOCASE("nl")) { result->io_class = OPERAND_CLASS_NL; goto done_yield_1; }
   if (IS_KWD_NOCASE("move")) { result->io_class = OPERAND_CLASS_MOVE; goto done_yield_1; }
   if (IS_KWD_NOCASE("default")) { result->io_class = OPERAND_CLASS_DEFAULT; goto done_yield_1; }
   if (IS_KWD_NOCASE("varargs")) { result->io_class = OPERAND_CLASS_VARARGS; goto done_yield_1; }
  }
  /* Fallback: Parse an address expression. */
  if unlikely(uasm_parse_intexpr(&result->io_intexpr,
                                  recognize_sp ? UASM_INTEXPR_FNORMAL
                                               : UASM_INTEXPR_FHASSP))
     goto err;
  /* Determine the address width classification of the operand. */
  asm_invoke_operand_determine_intclass(result);
  break;
done_yield_1:
  if unlikely(yield() < 0)
     goto err;
  break;
 }
 return 0;
err:
 return -1;
}

INTERN int FCALL
uasm_parse_operand(struct asm_invoke_operand *__restrict result) {
 ASSERT(!result->io_class);
 /* Parse the actual operand. */
 if unlikely(do_parse_operand(result,true))
    goto err;
 /* Check for a dots-suffix. */
 if (tok == TOK_DOTS) {
  /* Set the dots-flag. */
  if (result->io_class&OPERAND_CLASS_FDOTSFLAG &&
      WARN(W_UASM_DOTS_FLAG_ALREADY_SET_FOR_OPERAND))
      goto err;
  result->io_class |= OPERAND_CLASS_FDOTSFLAG;
  if unlikely(yield() < 0) goto err;
 }
 return 0;
err:
 return -1;
}

INTERN int FCALL
uasm_parse_instruction(void) {
 struct TPPKeyword *name;
 struct asm_mnemonic *mnemonic;
 struct asm_invocation invoc;
 if (tok == TOK_INT) {
  struct asm_sym *fbsym;
  /* Integer symbol definition. */
  name = TPPLexer_LookupEscapedKeyword(token.t_begin,
                              (size_t)(token.t_end-token.t_begin),1);
  if unlikely(!name) goto err;
  if unlikely(yield() < 0) goto err;
  if unlikely(likely(tok == ':') ? (yield() < 0) :
              WARN(W_UASM_EXPECTED_COLLON_AFTER_INTEGER))
     goto err;
  fbsym = uasm_fbsymbol_def(name);
  if unlikely(!fbsym) goto err;
  /* Define the symbol here. */
  uasm_defsym(fbsym);
  goto done_continue;
 }
 /* Clear out the invocation. */
 memset(&invoc,0,sizeof(struct asm_invocation));

read_mnemonic_name:
 /* Parse the name of the instruction. */
 name = uasm_parse_symnam();
 if unlikely(!name) goto err;
 if (tok == ':') {
  /* This is actually a label definition. */
  struct asm_sym *sym = uasm_symbol(name);
  if unlikely(!sym) goto err;
  if unlikely(ASM_SYM_DEFINED(sym)) {
   if (WARN(W_UASM_SYMBOL_ALREADY_DEFINED,name->k_name))
       goto err;
  } else {
   /* Define the symbol here. */
   uasm_defsym(sym);
  }
  /* Yield the `:' token. */
  if (yield() < 0) goto err;
  goto done_continue;
 }

 switch (name->k_id) {
 {
  int32_t val;
 case KWD_static:
do_static_prefix:
  if (invoc.ai_flags&INVOKE_FPREFIX)
      break;
  val = do_parse_static_operands();
  if unlikely(val < 0) goto err;
  invoc.ai_prefix     = ASM_STATIC;
  invoc.ai_prefix_id1 = (uint16_t)val;
continue_after_prefix:
  invoc.ai_flags |= INVOKE_FPREFIX;
  if unlikely(likely(tok == ':') ? (yield() < 0) :
              WARN(W_UASM_EXPECTED_COLLON_AFTER_PREFIX))
     goto err;
  goto read_mnemonic_name;
 }

 {
  int32_t val;
 case KWD_global:
do_global_prefix:
  if (invoc.ai_flags&INVOKE_FPREFIX)
      break;
  val = do_parse_global_operands();
  if unlikely(val < 0) goto err;
  invoc.ai_prefix     = ASM_GLOBAL;
  invoc.ai_prefix_id1 = (uint16_t)val;
  goto continue_after_prefix;
 }

 {
  int32_t val;
 case KWD_local:
do_local_prefix:
  if (invoc.ai_flags&INVOKE_FPREFIX)
      break;
  val = do_parse_local_operands();
  if unlikely(val < 0) goto err;
  invoc.ai_prefix     = ASM_LOCAL;
  invoc.ai_prefix_id1 = (uint16_t)val;
  goto continue_after_prefix;
 }

 {
#if 0
 case KWD_push:
#endif
do_push_prefix:
  /* Special case: either the push prefix, or the push instruction itself. */
  if (TPP_ISKEYWORD(tok)) {
   mnemonic = asm_mnemonic_lookup(token.t_kwd);
   if (mnemonic != NULL) {
    /* It's the push prefix. */
    name = token.t_kwd;
    invoc.ai_flags |= INVOKE_FPUSH; /* Set the push-prefix flag. */
    if unlikely(yield() < 0) goto err;
    goto got_mnemonic;
   }
  }
  break;
 }

 {
do_extern_prefix:
  if (invoc.ai_flags&INVOKE_FPREFIX)
      break;
  if unlikely(do_parse_extern_operands(&invoc.ai_prefix_id1,
                                       &invoc.ai_prefix_id2))
     goto err;
  invoc.ai_prefix = ASM_EXTERN;
  goto continue_after_prefix;
 }

 {
  int32_t val;
do_stack_prefix:
  if (invoc.ai_flags&INVOKE_FPREFIX)
      break;
  val = do_parse_stack_operands();
  if unlikely(val < 0) goto err;
  invoc.ai_prefix     = ASM_STACK;
  invoc.ai_prefix_id1 = (uint16_t)val;
  goto continue_after_prefix;
 }


#define NAMEISKWD(x) \
  (name->k_size == COMPILER_STRLEN(x) && \
   MEMCASEEQ(name->k_name,x,sizeof(x)-sizeof(char)))
 default:
  if (NAMEISKWD("push"))   goto do_push_prefix;
  if (NAMEISKWD("stack"))  goto do_stack_prefix;
  if (NAMEISKWD("extern")) goto do_extern_prefix;
  /* NOTE: Since we must remain case-insensitive, we must re-check all prefixes. */
  if (NAMEISKWD("static")) goto do_static_prefix;
  if (NAMEISKWD("global")) goto do_global_prefix;
  if (NAMEISKWD("local"))  goto do_local_prefix;
  if (NAMEISKWD("const")) { invoc.ai_flags |= INVOKE_FPREFIX_RO; goto do_static_prefix; }
  break;
#undef NAMEISKWD
 }

 /* All right! got the name of the instruction.
  * Now use it to lookup a mnemonic. */
 mnemonic = asm_mnemonic_lookup(name);
 if unlikely(!mnemonic) {
  if (WARN(W_UASM_UNKNOWN_MNEMONIC,name))
      goto err;
  /* Unknown mnemonic... (Discard the remainder of the line) */
  while (tok > 0 && tok != ';' && tok != '\n')
     if (yield() < 0) goto err;
  goto done;
 }
got_mnemonic:

 /* Time to start building the invocation. */
 while (tok > 0 && tok != ';' && tok != '\n' &&
        invoc.ai_opcount < ASM_MAX_INSTRUCTION_OPERANDS) {
  /* Parse an operand. */
  if unlikely(uasm_parse_operand(&invoc.ai_ops[invoc.ai_opcount]))
     goto err;
  ++invoc.ai_opcount;
  if (tok != ',') break;
  if unlikely(yield() < 0) goto err;
 }
 /* All right! We've got everything we need. Now to do the actual invocation. */
 if unlikely(uasm_invoke(mnemonic,&invoc))
    goto err;
done:
 return 0;
done_continue:
 return 1;
err:
 return -1;
}



/* Parse and compile user-defined assembly until EOF is encountered. */
INTERN int FCALL uasm_parse(void) {
 int error;
continue_line:
 while (tok > 0) {
  unsigned long old_num;
  old_num = token.t_num;
  if (tok == ';' || tok == '\n') {
   /* Empty line. */
  } else {
   if (tok == '.') {
    if unlikely(yield() < 0) goto err;
    /* Parse an assembly directive. */
    error = uasm_parse_directive();
   } else {
    /* Parse an assembly instruction. */
    error = uasm_parse_instruction();
   }
   if unlikely(error < 0) goto err;
   if (error > 0) goto continue_line;
  }
  /* Discard any trailing tokens before a new-line or semicolon. */
  while (tok > 0 && tok != ';' && tok != '\n') {
   if (WARN(W_UASM_IGNORING_TRAILING_TOKENS))
       goto err;
   if unlikely(yield() < 0) goto err;
  }
  /* Consume the `;' or `\n' token. */
  if (likely(tok == ';' || tok == '\n') &&
      unlikely(yield() < 0)) goto err;
  /* Warn if this didn't go anywhere. */
  if unlikely(old_num == token.t_num) {
   if (WARN(W_UASM_PARSING_FAILED))
       goto err;
   if unlikely(yield() < 0) goto err;
  }
 }
 return 0;
err:
 return -1;
}


INTERN struct asm_mnemonic *DCALL
asm_mnemonic_lookup_str(char const *__restrict name) {
 struct asm_mnemonic *result;
 ASM_MNEMONIC_FOREACH(result) {
  if (STRCASEEQ(result->am_name,name))
      return result;
 }
 return NULL;
}
INTERN struct asm_mnemonic *DCALL
asm_mnemonic_lookup(struct TPPKeyword *__restrict name) {
 struct asm_mnemonic *result;
 if (name->k_rare) {
  /* Check if the mnemonic has already been cached. */
  result = (struct asm_mnemonic *)name->k_rare->kr_user;
  if (result >= asm_mnemonics &&
      result < (struct asm_mnemonic *)((uintptr_t)asm_mnemonics+asm_mnemonics_size))
      goto done;
 }
 /* Do a string lookup. */
 result = asm_mnemonic_lookup_str(name->k_name);
 if unlikely(!result) goto done;
 /* Try to cache the mnemonic in the keyword. */
#undef calloc
#define calloc(n,s) Dee_Calloc((n)*(s))
 if (TPPKeyword_MAKERARE(name) &&
    !name->k_rare->kr_user)
     name->k_rare->kr_user = (void *)result;
done:
 return result;
}


DECL_END
#endif /* !CONFIG_LANGUAGE_NO_ASM */

#endif /* !GUARD_DEEMON_COMPILER_ASM_PARSEASM_C */
