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
#ifndef GUARD_DEEMON_COMPILER_ASM_ASSEMBER_C
#define GUARD_DEEMON_COMPILER_ASM_ASSEMBER_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/bool.h>
#include <deemon/rodict.h>
#include <deemon/module.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/gc.h>
#include <deemon/none.h>
#include <deemon/asm.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/assembler.h>
#include <deemon/util/cache.h>

#include <string.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

#include "../../runtime/builtin.h"

#ifdef __KOS_SYSTEM_HEADERS__
#include <hybrid/minmax.h>
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((b) < (a) ? (a) : (b))
#endif

#ifndef PP_CAT2
#define PP_PRIVATE_CAT2(a,b)   a##b
#define PP_PRIVATE_CAT3(a,b,c) a##b##c
#define PP_CAT2(a,b)   PP_PRIVATE_CAT2(a,b)
#define PP_CAT3(a,b,c) PP_PRIVATE_CAT3(a,b,c)
#endif

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) \
   typedef int PP_CAT2(static_assert_,__LINE__)[(expr)?1:-1]
#endif


DECL_BEGIN

INTDEF struct module_symbol empty_module_buckets[];

/* Assert opcode relations assumed by the optimizer. */
STATIC_ASSERT((ASM16_CALL_KW&0xff) == ASM_CALL_KW);
STATIC_ASSERT((ASM16_CALL_TUPLE_KW&0xff) == ASM_CALL_TUPLE_KW);
STATIC_ASSERT((ASM16_PUSH_BND_EXTERN&0xff) == ASM_PUSH_BND_EXTERN);
STATIC_ASSERT((ASM16_PUSH_BND_GLOBAL&0xff) == ASM_PUSH_BND_GLOBAL);
STATIC_ASSERT((ASM16_PUSH_BND_LOCAL&0xff) == ASM_PUSH_BND_LOCAL);
STATIC_ASSERT((ASM32_JMP&0xff) == ASM_JMP);
STATIC_ASSERT((ASM16_OPERATOR&0xff) == ASM_OPERATOR);
STATIC_ASSERT((ASM16_OPERATOR_TUPLE&0xff) == ASM_OPERATOR_TUPLE);
STATIC_ASSERT((ASM16_DEL_GLOBAL&0xff) == ASM_DEL_GLOBAL);
STATIC_ASSERT((ASM16_DEL_LOCAL&0xff) == ASM_DEL_LOCAL);
STATIC_ASSERT((ASM16_LROT&0xff) == ASM_LROT);
STATIC_ASSERT((ASM16_RROT&0xff) == ASM_RROT);
STATIC_ASSERT((ASM16_DUP_N&0xff) == ASM_DUP_N);
STATIC_ASSERT((ASM16_POP_N&0xff) == ASM_POP_N);
STATIC_ASSERT((ASM16_ADJSTACK&0xff) == ASM_ADJSTACK);
STATIC_ASSERT((ASM16_POP_STATIC&0xff) == ASM_POP_STATIC);
STATIC_ASSERT((ASM16_POP_EXTERN&0xff) == ASM_POP_EXTERN);
STATIC_ASSERT((ASM16_POP_GLOBAL&0xff) == ASM_POP_GLOBAL);
STATIC_ASSERT((ASM16_POP_LOCAL&0xff) == ASM_POP_LOCAL);
STATIC_ASSERT((ASM16_PUSH_MODULE&0xff) == ASM_PUSH_MODULE);
STATIC_ASSERT((ASM16_PUSH_REF&0xff) == ASM_PUSH_REF);
STATIC_ASSERT((ASM16_PUSH_ARG&0xff) == ASM_PUSH_ARG);
STATIC_ASSERT((ASM16_PUSH_CONST&0xff) == ASM_PUSH_CONST);
STATIC_ASSERT((ASM16_PUSH_STATIC&0xff) == ASM_PUSH_STATIC);
STATIC_ASSERT((ASM16_PUSH_EXTERN&0xff) == ASM_PUSH_EXTERN);
STATIC_ASSERT((ASM16_PUSH_GLOBAL&0xff) == ASM_PUSH_GLOBAL);
STATIC_ASSERT((ASM16_PUSH_LOCAL&0xff) == ASM_PUSH_LOCAL);
STATIC_ASSERT((ASM16_PACK_TUPLE&0xff) == ASM_PACK_TUPLE);
STATIC_ASSERT((ASM16_PACK_LIST&0xff) == ASM_PACK_LIST);
STATIC_ASSERT((ASM16_UNPACK&0xff) == ASM_UNPACK);
STATIC_ASSERT((ASM16_GETATTR_C&0xff) == ASM_GETATTR_C);
STATIC_ASSERT((ASM16_DELATTR_C&0xff) == ASM_DELATTR_C);
STATIC_ASSERT((ASM16_SETATTR_C&0xff) == ASM_SETATTR_C);
STATIC_ASSERT((ASM16_GETATTR_THIS_C&0xff) == ASM_GETATTR_THIS_C);
STATIC_ASSERT((ASM16_DELATTR_THIS_C&0xff) == ASM_DELATTR_THIS_C);
STATIC_ASSERT((ASM16_SETATTR_THIS_C&0xff) == ASM_SETATTR_THIS_C);
STATIC_ASSERT((ASM16_FUNCTION_C&0xff) == ASM_FUNCTION_C);
STATIC_ASSERT((ASM16_FUNCTION_C_16&0xff) == ASM_FUNCTION_C_16);
STATIC_ASSERT((ASM_INCPOST&0xff) == ASM_INC);
STATIC_ASSERT((ASM_DECPOST&0xff) == ASM_DEC);
STATIC_ASSERT((ASM16_DELOP&0xff) == ASM_DELOP);
STATIC_ASSERT((ASM16_NOP&0xff) == ASM_NOP);
STATIC_ASSERT((ASM16_CONTAINS_C&0xff) == ASM_CONTAINS_C);
STATIC_ASSERT((ASM16_PRINT_C&0xff) == ASM_PRINT_C);
STATIC_ASSERT((ASM16_PRINT_C_SP&0xff) == ASM_PRINT_C_SP);
STATIC_ASSERT((ASM16_PRINT_C_NL&0xff) == ASM_PRINT_C_NL);
STATIC_ASSERT((ASM16_FPRINT_C&0xff) == ASM_FPRINT_C);
STATIC_ASSERT((ASM16_FPRINT_C_SP&0xff) == ASM_FPRINT_C_SP);
STATIC_ASSERT((ASM16_FPRINT_C_NL&0xff) == ASM_FPRINT_C_NL);
STATIC_ASSERT((ASM16_GETITEM_C&0xff) == ASM_GETITEM_C);
STATIC_ASSERT((ASM16_SETITEM_C&0xff) == ASM_SETITEM_C);
STATIC_ASSERT((ASM_ITERNEXT&0xff) == ASM_ITERSELF);
STATIC_ASSERT((ASM16_CALLATTR_C&0xff) == ASM_CALLATTR_C);
STATIC_ASSERT((ASM16_CALLATTR_C_TUPLE&0xff) == ASM_CALLATTR_C_TUPLE);
STATIC_ASSERT((ASM16_CALLATTR_THIS_C&0xff) == ASM_CALLATTR_THIS_C);
STATIC_ASSERT((ASM16_CALLATTR_THIS_C_TUPLE&0xff) == ASM_CALLATTR_THIS_C_TUPLE);
STATIC_ASSERT((ASM16_GETMEMBER_THIS_R&0xff) == ASM_GETMEMBER_THIS_R);
STATIC_ASSERT((ASM16_DELMEMBER_THIS_R&0xff) == ASM_DELMEMBER_THIS_R);
STATIC_ASSERT((ASM16_SETMEMBER_THIS_R&0xff) == ASM_SETMEMBER_THIS_R);
STATIC_ASSERT((ASM16_CALL_EXTERN&0xff) == ASM_CALL_EXTERN);
STATIC_ASSERT((ASM16_CALL_GLOBAL&0xff) == ASM_CALL_GLOBAL);
STATIC_ASSERT((ASM16_CALL_LOCAL&0xff) == ASM_CALL_LOCAL);
STATIC_ASSERT((ASM16_STATIC&0xff) == ASM_STATIC);
STATIC_ASSERT((ASM16_EXTERN&0xff) == ASM_EXTERN);
STATIC_ASSERT((ASM16_GLOBAL&0xff) == ASM_GLOBAL);
STATIC_ASSERT((ASM16_LOCAL&0xff) == ASM_LOCAL);
STATIC_ASSERT((ASM16_CALLATTR_C_SEQ&0xff) == ASM_CALLATTR_C_SEQ);
STATIC_ASSERT((ASM16_CALLATTR_C_MAP&0xff) == ASM_CALLATTR_C_MAP);

STATIC_ASSERT(__SIZEOF_POINTER__ == sizeof(void *));

#ifdef CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER
STATIC_ASSERT(sizeof(struct asm_exc) == sizeof(struct except_handler));
#else /* CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER */
STATIC_ASSERT(sizeof(struct asm_exc) != sizeof(struct except_handler));
#endif /* !CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER */

#define sc_text   current_assembler.a_sect[SECTION_TEXT]
#define sc_cold   current_assembler.a_sect[SECTION_COLD]
#define sc_main   current_assembler.a_sect[0]


INTERN struct assembler current_assembler;

#define INITIAL_TEXTALLOC 32
/* Cache for assembly symbols. */
DEFINE_STRUCT_CACHE(asym,struct asm_sym,64)
#ifndef NDEBUG
#define asym_alloc()  asym_dbgalloc(__FILE__,__LINE__)
#endif

INTERN struct ddi_checkpoint *DCALL asm_newddi(void) {
 struct ddi_checkpoint *result;
 ASSERT(current_assembler.a_ddi.da_checkc <=
        current_assembler.a_ddi.da_checka);
 result = current_assembler.a_ddi.da_checkv;
 if (current_assembler.a_ddi.da_checkc ==
     current_assembler.a_ddi.da_checka) {
  size_t new_alloc = current_assembler.a_ddi.da_checka*2;
  /* Preallocate a bunch because we use checkpoints quite excessively. */
  if (!new_alloc) new_alloc = 16;
do_realloc:
  result = (struct ddi_checkpoint *)Dee_TryRealloc(result,new_alloc*
                                                   sizeof(struct ddi_checkpoint));
  if unlikely(!result) {
   result = current_assembler.a_ddi.da_checkv;
   if (new_alloc != current_assembler.a_ddi.da_checkc+1) {
    new_alloc = current_assembler.a_ddi.da_checkc+1;
    goto do_realloc;
   }
   if (Dee_CollectMemory(new_alloc*sizeof(struct ddi_checkpoint)))
       goto do_realloc;
   goto done;
  }
  /* Save the reallocated pointer. */
  current_assembler.a_ddi.da_checkv = result;
  current_assembler.a_ddi.da_checka = new_alloc;
 }
 /* Adjust the result pointer and save that we've gotten an additional checkpoint. */
 result += current_assembler.a_ddi.da_checkc++;
 /* Collect (steal) pending symbol binding information. */
 ASSERT(current_assembler.a_ddi.da_bnda >=
        current_assembler.a_ddi.da_bndc);
 result->dc_bndc = current_assembler.a_ddi.da_bndc;
 result->dc_bndv = current_assembler.a_ddi.da_bndv;
 if (current_assembler.a_ddi.da_bndc != current_assembler.a_ddi.da_bnda) {
  /* Release the unused portion of the symbol-binding cache. */
  result->dc_bndv = (struct ddi_binding *)Dee_TryRealloc(result->dc_bndv,
                                                         result->dc_bndc*
                                                         sizeof(struct ddi_binding));
  if unlikely(!result->dc_bndv)
     result->dc_bndv = current_assembler.a_ddi.da_bndv;
 }
 current_assembler.a_ddi.da_bndc = 0;
 current_assembler.a_ddi.da_bnda = 0;
 current_assembler.a_ddi.da_bndv = NULL;
done:
 return result;
}

INTERN struct TPPFile *DCALL
ddi_newfile(char const *__restrict filename,
            size_t filename_length) {
 struct TPPFile *result;
 /* Search for older DDI checkpoints using this filename. */
 result = current_assembler.a_ddi.da_files;
 for (; result; result = result->f_prev) {
  if (result->f_namesize != filename_length) continue;
  if (memcmp(result->f_name,filename,filename_length*sizeof(char)) != 0) continue;
  /* This filename has already been used before! */
  goto done;
 }

 /* Construct a new fake TPP file. */
 result = (DREF struct TPPFile *)Dee_Calloc(sizeof(struct TPPFile));
 if unlikely(!result) goto err;
 result->f_refcnt   = 1;
#if TPPFILE_KIND_TEXT != 0
 result->f_kind     = TPPFILE_KIND_TEXT;
#endif
 result->f_name     = (char *)Dee_Malloc((filename_length+1)*sizeof(char));
 if unlikely(!result->f_name) { Dee_Free(result); goto err; }
 memcpy(result->f_name,filename,(filename_length+1)*sizeof(char));
 result->f_namesize = filename_length;
 result->f_namehash = TPP_Hashof(result->f_name,result->f_namesize);
 result->f_text     = TPPString_NewEmpty();
 result->f_begin    = result->f_text->s_text;
 result->f_end      = result->f_text->s_text;
 result->f_pos      = result->f_text->s_text;
 result->f_textfile.f_flags = TPP_TEXTFILE_FLAG_INTERNAL;
 /* Save the file reference in the global DDI fake-files chain */
 result->f_prev = current_assembler.a_ddi.da_files;
 current_assembler.a_ddi.da_files = result;
done:
 return result;
err:
 return NULL;
}



#ifdef NDEBUG
INTERN int (DCALL asm_putddi)(struct ast *__restrict self)
#else
INTERN int (DCALL asm_putddi_dbg)(struct ast *__restrict self,
                                  char const *file, int line)
#endif
{
 struct asm_sym *sym;
 struct ddi_checkpoint *ddi;
 ASSERT_AST(self);
 /* Check for simple case: DDI is disabled. */
 if (current_assembler.a_flag & ASM_FNODDI)
     goto done;
 /* Check if there even is DDI information to save. */
 if (!self->a_ddi.l_file)
     goto done;
 /* Discard redundant debug information early on to save on memory. */
 if (current_assembler.a_ddi.da_slast == current_assembler.a_curr &&
     current_assembler.a_ddi.da_last  == asm_ip())
     goto done;
 /* Allocate a symbol and DDI checkpoint. */
#ifdef NDEBUG
 if unlikely((sym = asm_newsym()) == NULL ||
             (ddi = asm_newddi()) == NULL)
    return -1;
#else
 if unlikely((sym = asm_newsym_dbg(file,line)) == NULL ||
             (ddi = asm_newddi()) == NULL)
    return -1;
#endif
 /* Simply define the symbol at the current text position.
  * NOTE: Its actual address may change during later assembly phases. */
 asm_defsym(sym);
 ddi->dc_sym = sym;
 ++sym->as_used; /* Track use of this symbol by DDI information. */
 ddi->dc_loc = self->a_ddi;
 ddi->dc_sp  = current_assembler.a_stackcur;
 /* Save the current text position to discard early uses of the same checkpoint. */
 current_assembler.a_ddi.da_last  = sym->as_used;
 current_assembler.a_ddi.da_slast = current_assembler.a_curr;
done:
 return 0;
}


PRIVATE struct ddi_binding *DCALL asm_alloc_ddi_binding(void) {
 ASSERT(current_assembler.a_ddi.da_bndc <=
        current_assembler.a_ddi.da_bnda);
 if (current_assembler.a_ddi.da_bndc ==
     current_assembler.a_ddi.da_bnda) {
  struct ddi_binding *new_vec;
  uint16_t new_alloc = current_assembler.a_ddi.da_bndc+1;
  if (new_alloc >= 3) {
   new_alloc *= 2;
   if unlikely(new_alloc <= current_assembler.a_ddi.da_bndc)
      new_alloc = current_assembler.a_ddi.da_bndc+1;
  }
  new_vec = (struct ddi_binding *)Dee_TryRealloc(current_assembler.a_ddi.da_bndv,
                                                 new_alloc*sizeof(struct ddi_binding));
  if unlikely(!new_vec) {
   new_alloc = current_assembler.a_ddi.da_bndc+1;
   new_vec = (struct ddi_binding *)Dee_Realloc(current_assembler.a_ddi.da_bndv,
                                               new_alloc*sizeof(struct ddi_binding));
   if unlikely(!new_vec) return NULL;
  }
  current_assembler.a_ddi.da_bndv = new_vec;
  current_assembler.a_ddi.da_bnda = new_alloc;
 }
 return &current_assembler.a_ddi.da_bndv[current_assembler.a_ddi.da_bndc++];
}

INTERN int DCALL
asm_putddi_bind(uint16_t ddi_class,
                uint16_t index,
                struct TPPKeyword *name) {
 struct ddi_binding *binding; size_t i;
 /* Check for simple case: DDI is disabled. */
 if (current_assembler.a_flag & ASM_FNODDI)
     goto done;
 if (name && current_assembler.a_ddi.da_checkc) {
  /* Add the new binding in the previous checkpoint:
   * When a new binding is created, the caller will have done
   * something similar to this:
   * >> asm_putddi(...)             // Emit DDI for the next instruction
   * >> asm_put(MY_OPCODE)          // Emit the instruction
   * >> asm_put(asm_lsymid(symbol)) // Emit a local-id as operand
   * Considering this, they expect that their instruction will
   * already be apart of DDI symbol-binding information for their
   * operand-symbol. However if we were to only include their binding
   * in the next checkpoint, a source disassembly would not contain
   * a name binding in the proper location (that is: as part of the
   * checkpoint just before the instruction that allocated the index)
   * If a symbol is being unbound, the behavior is the opposite, as
   * in that case we _really_ should only make the changes become
   * active at the next checkpoint, rather than the previous. */
  struct ddi_checkpoint *last_checkpoint;
  struct ddi_binding *new_vector;
  last_checkpoint = &current_assembler.a_ddi.da_checkv[current_assembler.a_ddi.da_checkc-1];
  for (i = 0; i < last_checkpoint->dc_bndc; ++i) {
   binding = &last_checkpoint->dc_bndv[i];
   if (binding->db_class != ddi_class) continue;
   if (binding->db_index != index) continue;
   /* Override this entry. */
   binding->db_name = name;
   goto done;
  }
  new_vector = (struct ddi_binding *)Dee_Realloc(last_checkpoint->dc_bndv,
                                                (last_checkpoint->dc_bndc+1)*
                                                 sizeof(struct ddi_binding));
  if unlikely(!new_vector) goto err;
  last_checkpoint->dc_bndv = new_vector;
  binding = &new_vector[last_checkpoint->dc_bndc++];
  binding->db_class = ddi_class;
  binding->db_index = index;
  binding->db_name  = name;
  goto done;
 }

 /* Search for an existing entry which we can override. */
 for (i = 0; i < current_assembler.a_ddi.da_bndc; ++i) {
  binding = &current_assembler.a_ddi.da_bndv[i];
  if (binding->db_class != ddi_class) continue;
  if (binding->db_index != index) continue;
  /* Override this entry. */
  binding->db_name = name;
  goto done;
 }
 /* Append a new binding. */
 binding = asm_alloc_ddi_binding();
 if unlikely(!binding) goto err;
 binding->db_class = ddi_class;
 binding->db_index = index;
 binding->db_name  = name;
done:
 return 0;
err:
 return -1;
}


INTERN int DCALL assembler_init(void) {
 memset(&current_assembler,0,sizeof(struct assembler));
 current_assembler.a_curr = &current_assembler.a_sect[SECTION_TEXT];
 /* Allocate an initial buffer for the text section. */
 current_assembler.a_sect[SECTION_TEXT].sec_code = (DeeCodeObject *)
  DeeGCObject_Malloc(offsetof(DeeCodeObject,co_code)+INITIAL_TEXTALLOC);
 current_assembler.a_sect[SECTION_TEXT].sec_begin = current_assembler.a_sect[SECTION_TEXT].sec_code->co_code;
 current_assembler.a_sect[SECTION_TEXT].sec_iter  = current_assembler.a_sect[SECTION_TEXT].sec_begin;
 current_assembler.a_sect[SECTION_TEXT].sec_end   = current_assembler.a_sect[SECTION_TEXT].sec_begin+INITIAL_TEXTALLOC;
 current_assembler.a_ddi.da_last = (code_addr_t)-1;
#ifndef CONFIG_LANGUAGE_NO_ASM
 return userassembler_init();
#else /* !CONFIG_LANGUAGE_NO_ASM */
 return 0;
#endif /* CONFIG_LANGUAGE_NO_ASM */
}
INTERN int DCALL
assembler_init_reuse(DeeCodeObject *__restrict code_obj,
                     instruction_t *__restrict text_end) {
 memset(&current_assembler,0,sizeof(struct assembler));
 current_assembler.a_curr = &current_assembler.a_sect[SECTION_TEXT];
 /* Allocate an initial buffer for the text section. */
 current_assembler.a_sect[SECTION_TEXT].sec_code  = code_obj;
 current_assembler.a_sect[SECTION_TEXT].sec_begin = code_obj->co_code;
 current_assembler.a_sect[SECTION_TEXT].sec_iter  = text_end;
 current_assembler.a_sect[SECTION_TEXT].sec_end   = text_end;
 current_assembler.a_ddi.da_last = (code_addr_t)-1;
#ifndef CONFIG_LANGUAGE_NO_ASM
 return userassembler_init();
#else /* !CONFIG_LANGUAGE_NO_ASM */
 return 0;
#endif /* CONFIG_LANGUAGE_NO_ASM */
}
INTERN void DCALL assembler_fini(void) {
 struct asm_sym *iter,*next;
 DREF DeeObject **ob_iter,**ob_end;
 struct asm_exc *xiter,*xend;
 unsigned int i;
#ifndef CONFIG_LANGUAGE_NO_ASM
 userassembler_fini();
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 for (i = 0; i < SECTION_COUNT; ++i) {
  if (current_assembler.a_sect[i].sec_code)
       DeeGCObject_Free(current_assembler.a_sect[i].sec_code);
  else Dee_Free(current_assembler.a_sect[i].sec_begin);
  Dee_Free(current_assembler.a_sect[i].sec_relv);
 }
 /* Free up symbols. */
 iter = current_assembler.a_syms;
 while (iter) {
  next = iter->as_next;
  asym_free(iter);
  iter = next;
 }
 /* Free up constant variables. */
 ob_end = (ob_iter = current_assembler.a_constv)+
                     current_assembler.a_constc;
 for (; ob_iter != ob_end; ++ob_iter) Dee_XDecref(*ob_iter);
 Dee_Free(current_assembler.a_constv);
 /* Free up static variables. */
 ob_end = (ob_iter = current_assembler.a_staticv)+
                     current_assembler.a_staticc;
 for (; ob_iter != ob_end; ++ob_iter) Dee_XDecref(*ob_iter);
 Dee_Free(current_assembler.a_staticv);
 /* Free up exception handlers. */
 xend = (xiter = current_assembler.a_exceptv)+
                 current_assembler.a_exceptc;
 for (; xiter != xend; ++xiter)
      Dee_XDecref(xiter->ex_mask);
 Dee_Free(current_assembler.a_exceptv);
 Dee_Free(current_assembler.a_localuse);
 Dee_Free(current_assembler.a_refv);
 Dee_Free(current_assembler.a_argrefv);

 /* Delete debug information. */
 for (i = 0; i < current_assembler.a_ddi.da_checkc; ++i)
     Dee_Free(current_assembler.a_ddi.da_checkv[i].dc_bndv);
 Dee_Free(current_assembler.a_ddi.da_checkv);
 Dee_Free(current_assembler.a_ddi.da_bndv);
 while (current_assembler.a_ddi.da_files) {
  struct TPPFile *next;
  next = current_assembler.a_ddi.da_files->f_prev;
  current_assembler.a_ddi.da_files->f_prev = NULL;
  TPPFile_Decref(current_assembler.a_ddi.da_files);
  current_assembler.a_ddi.da_files = next;
 }
}


INTERN bool DCALL asm_delunused_symbols(void) {
 bool result = false;
 struct asm_sym **piter,*iter;
 piter = &current_assembler.a_syms;
 while ((iter = *piter) != NULL) {
  if (!iter->as_used) {
   /* Unused symbol. */
   *piter = iter->as_next;
   asym_free(iter); /* Free this symbol. */
   continue;
  }
  piter = &iter->as_next;
 }
 return result;
}

#ifdef NDEBUG
INTDEF struct asm_sym *DCALL asm_newsym(void)
#else
INTDEF struct asm_sym *DCALL asm_newsym_dbg(char const *file, int line)
#endif
{
 struct asm_sym *result = asym_alloc();
 if unlikely(!result) goto done;
 result->as_next = current_assembler.a_syms;
 result->as_sect = SECTION_INVALID;
#ifndef NDEBUG
 result->as_addr = 0xcccccccc;
 result->as_stck = 0xcccc;
#endif
#ifndef CONFIG_LANGUAGE_NO_ASM
 result->as_uname = NULL;
 result->as_uhnxt = NULL;
 result->as_uprev = NULL;
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 current_assembler.a_syms = result;
 result->as_used = 0; /* Unused by default. */
#ifndef NDEBUG
 result->as_file = file;
 result->as_line = line;
#endif
done:
 return result;
}

PRIVATE int DCALL asm_realloc_exc(void) {
 struct asm_exc *new_vector;
 uint16_t new_alloc;
 new_vector = current_assembler.a_exceptv;
 new_alloc = current_assembler.a_excepta * 2;
 if (!new_alloc) new_alloc = 1;
 if unlikely(new_alloc < current_assembler.a_excepta) {
  if unlikely(current_assembler.a_exceptc == UINT16_MAX) {
   DeeError_Throwf(&DeeError_CompilerError,
                   "Too many exception handlers");
   return -1;
  }
  new_alloc = UINT16_MAX;
 }
 /* Must allocate more exception handlers. */
do_realloc:
 new_vector = (struct asm_exc *)Dee_TryRealloc(new_vector,new_alloc*sizeof(struct asm_exc));
 if unlikely(!new_vector) {
  if (new_alloc != current_assembler.a_exceptc+1) {
   new_alloc = current_assembler.a_exceptc+1;
   goto do_realloc;
  }
  if (Dee_CollectMemory(new_alloc*sizeof(struct asm_exc)))
      goto do_realloc;
 }
 current_assembler.a_excepta = new_alloc;
 current_assembler.a_exceptv = new_vector;
 return 0;
}
INTERN struct asm_exc *(DCALL asm_newexc)(void) {
 struct asm_exc *result;
 ASSERT(current_assembler.a_exceptc <=
        current_assembler.a_excepta);
 /* Allocate more exception vector memory if necessary. */
 if (current_assembler.a_exceptc == current_assembler.a_excepta &&
     asm_realloc_exc())
     return NULL;
 ASSERT(current_assembler.a_exceptc < current_assembler.a_excepta);
 result  = current_assembler.a_exceptv;
 result += current_assembler.a_exceptc++;
 /* The caller will be filling in the returned structure. */
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct asm_exc));
#endif
 return result;
}
INTERN struct asm_exc *(DCALL asm_newexc_at)(uint16_t priority) {
 struct asm_exc *result;
 ASSERT(priority <= current_assembler.a_exceptc);
 ASSERT(current_assembler.a_exceptc <= current_assembler.a_excepta);
 /* Allocate more exception vector memory if necessary. */
 if (current_assembler.a_exceptc == current_assembler.a_excepta &&
     asm_realloc_exc())
     return NULL;
 ASSERT(current_assembler.a_exceptc < current_assembler.a_excepta);
 result = current_assembler.a_exceptv;
 /* Shift all handlers with a greater priority. */
 memmove(result+priority+1,result+priority,
        (current_assembler.a_exceptc-priority)*
         sizeof(struct asm_exc));
 ++current_assembler.a_exceptc;
 /* Return the exception entry at the given priority (index). */
 result += priority;
 /* The caller will be filling in the returned structure. */
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct asm_exc));
#endif
 return result;
}

INTERN void DCALL
asm_defsym(struct asm_sym *__restrict self) {
 ASSERT(self);
 ASSERT(!ASM_SYM_DEFINED(self));
 self->as_sect = (uint16_t)(current_assembler.a_curr - current_assembler.a_sect);
 self->as_addr = ASM_SEC_ADDR(*current_assembler.a_curr);
 self->as_stck = current_assembler.a_stackcur;
 self->as_hand = current_assembler.a_handlerc;
}


INTERN bool DCALL asm_rmdelop(void) {
 struct asm_rel *rel_begin,*rel_end;
 instruction_t *iter,*end;
 bool result = false;
 if (!(current_assembler.a_flag&ASM_FOPTIMIZE))
       goto done;
 iter = sc_main.sec_begin;
 end  = sc_main.sec_iter;
#if 1
 if (!(current_assembler.a_flag&ASM_FPEEPHOLE)) {
  /* Without peephole around, still try to optimize the `adjstack' instruction!
   * NOTE: The caller will have already run `asm_linkstack()', so we can be
   *       save to assume that the operand of any `adjstack' instruction
   *       is immutable. */
  while (iter < end) {
   if (*(iter + 0) == ASM_ADJSTACK) {
    int8_t offset = *(int8_t *)(iter + 1);
    switch (offset) {
    case -2: *(iter + 0) = ASM_POP;       *(iter + 1) = ASM_POP;       break;
    case -1: *(iter + 0) = ASM_POP;       *(iter + 1) = ASM_DELOP;     break;
    case  0: *(iter + 0) = ASM_DELOP;     *(iter + 1) = ASM_DELOP;     break;
    case  1: *(iter + 0) = ASM_PUSH_NONE; *(iter + 1) = ASM_DELOP;     break;
    case  2: *(iter + 0) = ASM_PUSH_NONE; *(iter + 1) = ASM_PUSH_NONE; break;
    default: break;
    }
   } else if (*(iter + 0) == (ASM16_ADJSTACK & 0xff00) >> 8 &&
              *(iter + 1) == (ASM16_ADJSTACK & 0xff)) {
    int16_t offset = (int16_t)UNALIGNED_GETLE16((uint16_t *)(iter + 2));
    switch (offset) {
    case -2: *(iter + 0) = ASM_POP;       *(iter + 1) = ASM_POP;       goto common_adj16_delop;
    case -1: *(iter + 0) = ASM_POP;       *(iter + 1) = ASM_DELOP;     goto common_adj16_delop;
    case  0: *(iter + 0) = ASM_DELOP;     *(iter + 1) = ASM_DELOP;     goto common_adj16_delop;
    case  1: *(iter + 0) = ASM_PUSH_NONE; *(iter + 1) = ASM_DELOP;     goto common_adj16_delop;
    case  2: *(iter + 0) = ASM_PUSH_NONE; *(iter + 1) = ASM_PUSH_NONE;
common_adj16_delop:
     *(iter + 2) = ASM_DELOP;
     *(iter + 3) = ASM_DELOP;
     break;
    default:
     if (offset >= INT8_MIN && offset <= INT8_MAX) {
      *(iter + 0)           = ASM_ADJSTACK;
      *(int8_t *)(iter + 1) = (int8_t)offset;
      goto common_adj16_delop;
     }
     break;
    }
   }
   iter = asm_nextinstr(iter);
  }
 }
#endif

 rel_begin = sc_main.sec_relv;
 rel_end   = rel_begin+sc_main.sec_relc;
 while (iter < end) {
  /* Check if we should delete this instruction. */
  if (*iter == ASM_DELOP) {
   struct asm_rel *rel_iter;
   struct asm_sym *sym_iter;
   code_addr_t deladdr = (code_addr_t)(iter - sc_main.sec_begin);
   /* Modify code to get rid of the unused instruction. */
   memmove(iter,iter+1,(size_t)(--end-iter));
   /* Adjust the addresses of all relocations above. */
   while (rel_begin != rel_end &&
          rel_begin->ar_addr <= deladdr)
        ++rel_begin;
   for (rel_iter  = rel_begin;
        rel_iter != rel_end; ++rel_iter) {
    ASSERT(rel_iter->ar_addr > deladdr);
    --rel_iter->ar_addr; /* Adjust relocation address. */
   }
   /* Must also adjust the declaration addresses of all symbols. */
   for (sym_iter = current_assembler.a_syms;
        sym_iter; sym_iter = sym_iter->as_next) {
    /* NOTE: All symbols should be defined as part of the first section,
     *       but some unused symbols may still be hanging around... */
    if (sym_iter->as_sect == 0 &&
        sym_iter->as_addr > deladdr)
        --sym_iter->as_addr; /* Adjust the address of all symbols _ABOVE_ the deleted opcode. */
   }
   /* NOTE: We don't need to worry about adjusting exception handlers as
    *       they're using assembly symbols, meaning that we've already
    *       adjusted for them by updating symbol addresses. */
   result = true;
  } else {
   iter = asm_nextinstr(iter);
  }
  sc_main.sec_iter = end;
 }
done:
 return result;
}


/* Assert some special behavior of 16-bit vs. 8-bit jump instructions
 * that is assumed by the implementation of `asm_minjmp()' and peephole.
 *   - All 16-bit jump instructions have the least significant bit set.
 *   - All 8-bit jump instructions have the least significant bit clear.
 *   - Opcodes of all 16-bit variants of jump instructions are equal
 *     to 8-bit variants +1 (or |1 thanks to the 2 rules above)
 *   - All 32-bit jump instructions are prefixed with `ASM_EXTENDED1'
 */
STATIC_ASSERT((ASM_JMP == ASM_JMP16-1) && (ASM_JMP16&1));
STATIC_ASSERT((ASM_JT == ASM_JT16-1) && (ASM_JT16&1));
STATIC_ASSERT((ASM_JF == ASM_JF16-1) && (ASM_JF16&1));
STATIC_ASSERT((ASM_FOREACH == ASM_FOREACH16-1) && (ASM_FOREACH16&1));
STATIC_ASSERT((ASM32_JMP&0xff00) >> 8 == ASM_EXTENDED1);

/* Assert that our way of testing a conditional jump works.
 *   - All conditional jumps can be inverted by toggling the `ASM_JX_NOTBIT' (bit #2) bit.
 *   - The bool/not instructions can be exchanged by toggling the least significant bit.
 */
STATIC_ASSERT(ASM_JX_NOT(ASM_JT) == ASM_JF);
STATIC_ASSERT(ASM_JX_NOT(ASM_JF) == ASM_JT);
STATIC_ASSERT((ASM_BOOL^1) == ASM_NOT);

/* The following relations are assumed by the peephole optimizer for `pop ?; push ?' --> `dup; pop ?' */
STATIC_ASSERT(ASM_POP_STATIC+0x10 == ASM_PUSH_STATIC);
STATIC_ASSERT(ASM_POP_EXTERN+0x10 == ASM_PUSH_EXTERN);
STATIC_ASSERT(ASM_POP_GLOBAL+0x10 == ASM_PUSH_GLOBAL);
STATIC_ASSERT(ASM_POP_LOCAL+0x10  == ASM_PUSH_LOCAL);

PRIVATE bool DCALL
is_instruction_start(instruction_t *__restrict ptr) {
 instruction_t *iter = sc_main.sec_begin;
 while (iter < ptr) iter = asm_nextinstr(iter);
 return iter == ptr;
}
PRIVATE void DCALL
inc_reladdr_at(instruction_t *__restrict ptr) {
 code_addr_t addr = (code_addr_t)(ptr - sc_main.sec_begin);
 struct asm_rel *iter,*end;
 end = (iter = sc_main.sec_relv) + sc_main.sec_relc;
 for (; iter < end; ++iter) {
  if (iter->ar_addr < addr) continue;
  if (iter->ar_addr != addr) break;
  ++iter->ar_addr;
 }
}

PRIVATE void DCALL
asm_fix_jump_prefix(instruction_t *__restrict delop_instr) {
 instruction_t *prefix_loc;
 ASSERT(*(delop_instr + 0) == ASM_DELOP);
 ASSERT(*(delop_instr + 1) == ASM_JF ||  *(delop_instr + 1) == ASM_JT ||
        *(delop_instr + 1) == ASM_JMP || *(delop_instr + 1) == ASM_FOREACH);
 prefix_loc = delop_instr - 2;
 if (prefix_loc >= sc_main.sec_begin) {
  /* ASM_LOCAL */
  /* ASM_GLOBAL */
  /* ASM_STATIC */
  /* ASM_STACK */
  switch (prefix_loc[0]) {
  case ASM_LOCAL:
  case ASM_GLOBAL:
  case ASM_STATIC:
  case ASM_STACK:
   if (is_instruction_start(prefix_loc)) {
    prefix_loc[2] = prefix_loc[1];
    prefix_loc[1] = prefix_loc[0];
    prefix_loc[0] = ASM_DELOP;
    if (prefix_loc[1] == ASM_STACK)
        inc_reladdr_at(prefix_loc + 2);
    return;
   }
   break;
  default: break;
  }
 }
 prefix_loc = delop_instr - 3;
 if (prefix_loc >= sc_main.sec_begin &&
     prefix_loc[0] == ASM_EXTERN &&
     is_instruction_start(prefix_loc)) {
  /* ASM_EXTERN */
  prefix_loc[3] = prefix_loc[2];
  prefix_loc[2] = prefix_loc[1];
  prefix_loc[1] = prefix_loc[0];
  prefix_loc[0] = ASM_DELOP;
  return;
 }
 prefix_loc = delop_instr - 4;
 if (prefix_loc >= sc_main.sec_begin &&
     prefix_loc[0] == ASM_EXTENDED1) {
  /* ASM16_LOCAL */
  /* ASM16_GLOBAL */
  /* ASM16_STATIC */
  /* ASM16_STACK */
  switch (prefix_loc[0]) {
  case ASM16_LOCAL & 0xff:
  case ASM16_GLOBAL & 0xff:
  case ASM16_STATIC & 0xff:
  case ASM16_STACK & 0xff:
   if (is_instruction_start(prefix_loc)) {
    prefix_loc[4] = prefix_loc[3]; /* ID (low) */
    prefix_loc[3] = prefix_loc[2]; /* ID (high) */
    prefix_loc[2] = prefix_loc[1]; /* ASM16_XXX */
    prefix_loc[1] = prefix_loc[0]; /* ASM_EXTENDED1 */
    prefix_loc[0] = ASM_DELOP;
    if (prefix_loc[2] == (ASM16_STACK & 0xff))
        inc_reladdr_at(prefix_loc + 3);
    return;
   }
   break;
  default: break;
  }
 }
 prefix_loc = delop_instr - 6;
 if (prefix_loc >= sc_main.sec_begin &&
     prefix_loc[0] == ((ASM16_EXTERN & 0xff00) >> 8) &&
     prefix_loc[1] == (ASM16_EXTERN & 0xff) &&
     is_instruction_start(prefix_loc)) {
  /* ASM16_EXTERN */
  prefix_loc[6] = prefix_loc[5];
  prefix_loc[5] = prefix_loc[4];
  prefix_loc[4] = prefix_loc[3];
  prefix_loc[3] = prefix_loc[2];
  prefix_loc[2] = prefix_loc[1];
  prefix_loc[1] = prefix_loc[0];
  prefix_loc[0] = ASM_DELOP;
  return;
 }
}

INTERN bool DCALL asm_minjmp(void) {
 struct asm_rel *iter,*end;
 bool result = false;
 if (!(current_assembler.a_flag&ASM_FOPTIMIZE))
       goto done;
 end = (iter = sc_main.sec_relv)+sc_main.sec_relc;
 for (; iter != end; ++iter) {
  switch (iter->ar_type) {

  {
   int32_t target;
  case R_DMN_DISP16:
   ASSERT(iter->ar_sym);
   ASSERT(ASM_SYM_DEFINED(iter->ar_sym));
   target  = (int16_t)UNALIGNED_GETLE16((uint16_t *)(sc_main.sec_begin + iter->ar_addr));
   target += iter->ar_sym->as_addr;
   target -= iter->ar_addr;
   if (target >= INT8_MIN && target <= INT8_MAX) {
    uint8_t *instr;
    /* Yes, we can fit this one within an 8-bit target.
     * NOTE: Because 16-bit jump targets are encoded in little-endian,
     *       the proper address of an 8-bit target is already encoded
     *       within the lower (first) byte! */
    if (iter->ar_addr == 0) break;
    instr = (uint8_t *)(sc_main.sec_begin + iter->ar_addr - 1);
    if (*instr != ASM_JMP16 && *instr != ASM_JT16 &&
        *instr != ASM_JF16 && *instr != ASM_FOREACH16)
        break;
    /* Make sure that the upper 8 bits are truly sign-extensions. */
    if (instr[2] != ((instr[1]&0x80) ? 0xff : 0x00)) break;
#if 1 /* This variant is required because the other would overflow if `target == 0xff'
       * If we used the other variant, the assembler would have to loop over and use
       * BIGCODE mode (which would produce working code btw.), however doing so not only
       * is unnecessary, but also considerably slower because of wrapper assembly:
       * >>    jf    pop, 1f
       * Then becoming:
       * >>    jt    pop, 2f
       * >>    jmp   1f       // 32-bit
       * >>2:
       */
    *(instr + 2) = *((int8_t *)(instr + 1)) + 1; /* This +1 is negated by the `++iter->ar_addr' below. */
    *(instr + 1) = *(instr + 0) & ~1; /* Turn the instruction into its 8-bit counterpart. */
    *(instr + 0) = ASM_DELOP;   /* Mark the leading byte for deletion */
    ++iter->ar_addr;            /* Move the relocation up 1 byte, so it points to the new 8-bit offset. */
    asm_fix_jump_prefix(instr + 0);
#else
    *(instr + 0) &= ~1;         /* Turn the instruction into its 8-bit counterpart. */
    ++*((int8_t *)(instr + 1)); /* Increment the jump offset by one, because the
                                 * source address is now located one byte lower. */
    *(instr + 2)  = ASM_DELOP;  /* Mark the (now) unused high 8 bits as a DELOP. */
#endif
    iter->ar_type = R_DMN_DISP8;  /* Change the relocation into an 8-bit disposition. */
    result        = true;       /* Indicate that we managed to optimize something. */
   }
  } break;

  case R_DMN_STCKA16:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   if (iter->ar_sym->as_stck < INT8_MAX) {
    uint8_t *instr = (uint8_t *)(sc_main.sec_begin + iter->ar_addr - 1);
    /* Check if the second byte is a sign extension. */
    if (instr[2] == ((instr[1]&0x80) ? 0xff : 0x00)) {
     /* Convert into a `R_DMN_STCKAS8' */
     if (instr != sc_main.sec_begin && instr[-1] == ASM_EXTENDED1) {
      *(instr - 1) = ASM_DELOP; /* Mark the unused bytes as a DELOP. */
      *(instr + 2) = ASM_DELOP;
     } else {
      ASSERT(*(instr + 0) & 1);
      *(instr + 0) &= ~1;
      *(instr + 2)  = ASM_DELOP; /* Mark the unused byte as a DELOP. */
     }
     iter->ar_type = R_DMN_STCKAS8;
     result = true;
    }
   }
   break;

  {
   int32_t stack_offset;
   uint8_t *instr;
  case R_DMN_STCK16:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   stack_offset = (iter->ar_sym->as_stck -
                  (int16_t)UNALIGNED_GETLE16((uint16_t *)(sc_main.sec_begin + iter->ar_addr)));
   if (stack_offset < INT8_MIN || stack_offset > INT8_MAX) break;
   /* Convert this into a `R_DMN_STCK8' relocation. */
   instr = (uint8_t *)(sc_main.sec_begin + iter->ar_addr - 1);
   if (instr != sc_main.sec_begin && instr[-1] == ASM_EXTENDED1) {
    *(instr - 1)  = ASM_DELOP; /* Mark the unused bytes as a DELOP. */
    *(instr + 2)  = ASM_DELOP; /* ... */
   } else {
    if (!(*(instr + 0) & 1)) break;
    *(instr + 0) &= ~1;
    *(instr + 2) = ASM_DELOP; /* Mark the unused byte as a DELOP. */
   }
   iter->ar_type = R_DMN_STCK8;
   result = true;
   ATTR_FALLTHROUGH
  case R_DMN_STCK8:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   stack_offset = (iter->ar_sym->as_stck -
                 *(int8_t *)(sc_main.sec_begin + iter->ar_addr));
   if (stack_offset >= -2 && stack_offset <= +2) {
    /* Optimize to `pop; pop' or `push none; push none', and everything in-between. */
    instr = (uint8_t *)(sc_main.sec_begin + iter->ar_addr - 1);
    /* */if (stack_offset == -2) *(instr + 0) = ASM_POP,*(instr + 1) = ASM_POP;
    else if (stack_offset == -1) *(instr + 0) = ASM_POP,*(instr + 1) = ASM_DELOP;
    else if (stack_offset ==  0) *(instr + 0) = ASM_DELOP,*(instr + 1) = ASM_DELOP;
    else if (stack_offset ==  1) *(instr + 0) = ASM_PUSH_NONE,*(instr + 1) = ASM_DELOP;
    else /*                   */ *(instr + 0) = ASM_PUSH_NONE,*(instr + 1) = ASM_PUSH_NONE;
    asm_reldel(iter); /* Delete this relocation. */
    result = true;
   }
  } break;

  default:
   break;
  }
 }
done:
 return result;
}


INTERN int DCALL asm_mergetext(void) {
 unsigned int i;
 code_addr_t total_code;
 code_addr_t total_rel;
 code_addr_t code_offsets[SECTION_TEXTCOUNT];
 total_code = 0;
 total_rel  = 0;
 for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
  code_offsets[i] = total_code;
  total_code += (code_addr_t)(current_assembler.a_sect[i].sec_iter -
                              current_assembler.a_sect[i].sec_begin);
  total_rel  += (code_addr_t)current_assembler.a_sect[i].sec_relc;
 }
 /* Fix symbol addresses by moving them into the `sc_main'. */
 { struct asm_sym *iter = current_assembler.a_syms;
   for (; iter; iter = iter->as_next) {
    if (!ASM_SYM_DEFINED(iter)) continue;
    /* Adjust the symbol to point into the main section. */
    iter->as_addr += code_offsets[iter->as_sect];
    iter->as_sect  = 0;
   }
 }

 if (sc_main.sec_begin+total_code > sc_main.sec_end) {
  /* Make sure to allocate enough text. */
  DeeCodeObject *new_code;
  ASSERT(sc_main.sec_code != NULL);
  new_code = (DeeCodeObject *)DeeGCObject_Realloc(sc_main.sec_code,offsetof(DeeCodeObject,co_code)+
                                                  total_code*sizeof(instruction_t));
  if unlikely(!new_code) return -1;
  sc_main.sec_code  = new_code;
  sc_main.sec_iter  = new_code->co_code + (sc_main.sec_iter - sc_main.sec_begin);
  sc_main.sec_begin = new_code->co_code;
  sc_main.sec_end   = new_code->co_code+total_code;
 }
 if (sc_main.sec_rela < total_rel) {
  struct asm_rel *new_rel;
  /* Make sure to allocate enough relocations. */
  new_rel = (struct asm_rel *)Dee_Realloc(sc_main.sec_relv,total_rel*
                                          sizeof(struct asm_rel));
  if unlikely(!new_rel) return -1;
  sc_main.sec_relv = new_rel;
  sc_main.sec_rela = total_rel;
 }

 for (i = 1; i < SECTION_TEXTCOUNT; ++i) {
  size_t sect_size = (size_t)(current_assembler.a_sect[i].sec_iter-
                              current_assembler.a_sect[i].sec_begin);
  size_t j,count; struct asm_rel *dst;
  memcpy(sc_main.sec_iter,current_assembler.a_sect[i].sec_begin,
         sect_size*sizeof(instruction_t));
  sc_main.sec_iter += sect_size;
  /* Also copy relocations. */
  dst   = sc_main.sec_relv+sc_main.sec_relc;
  count = current_assembler.a_sect[i].sec_relc;
  memcpy(dst,current_assembler.a_sect[i].sec_relv,
         count*sizeof(struct asm_rel));
  /* Adjust relocation offsets. */
  for (j = 0; j < count; ++j)
       dst[j].ar_addr += code_offsets[i];
  sc_main.sec_relc += count;
  ASSERT(sc_main.sec_relc <= sc_main.sec_rela);
 }
 return 0;
}

INTERN int DCALL asm_mergestatic(void) {
 uint16_t static_offset,total_count;
 struct asm_rel *iter,*end;
 DREF DeeObject **total_vector;
 /* Simple case: Without any static variables, we've got nothing to do! */
 if (!current_assembler.a_staticc) return 0;
 /* We merge constant and static variables by
  * placing the static ones after the constant ones. */
 static_offset = current_assembler.a_constc;
 total_count   = static_offset+current_assembler.a_staticc;
 if unlikely(total_count < static_offset) {
  /* Too many variables. */
  DeeError_Throwf(&DeeError_CompilerError,
                  "Too many constant/static variables");
  return -1;
 }
 total_vector = (DREF DeeObject **)Dee_Realloc(current_assembler.a_constv,
                                               total_count*sizeof(DREF DeeObject *));
 if unlikely(!total_vector) return -1;
 /* Copy static variable initializers. */
 memcpy(total_vector+current_assembler.a_constc,
        current_assembler.a_staticv,
        current_assembler.a_staticc*
        sizeof(DREF DeeObject *));
 current_assembler.a_constv = total_vector;
 current_assembler.a_constc = total_count;
 current_assembler.a_consta = total_count;
 /* Delete the static variable vector. */
 Dee_Free(current_assembler.a_staticv);
 current_assembler.a_staticv = NULL;
 current_assembler.a_staticc = 0;
 current_assembler.a_statica = 0;
 /* Fix relocations for static variables. */
 end = (iter = sc_main.sec_relv)+sc_main.sec_relc;
 for (; iter != end; ++iter) {
  uint16_t *imm_addr;
  uint16_t static_id;
  if ((iter->ar_type & ~R_DMN_FUSER) != R_DMN_STATIC16) continue;
  imm_addr  = (uint16_t *)(sc_main.sec_begin + iter->ar_addr);
  static_id = (uint16_t)(UNALIGNED_GETLE16(imm_addr) + static_offset);
  /* Make sure that the addition did not overflow.
   * It shouldn't, because we've already checked if the total
   * number of constant variables could have overflowed. */
  ASSERT((uint32_t)static_id == (uint32_t)UNALIGNED_GETLE16(imm_addr) + (uint32_t)static_offset);
  if (static_id <= UINT8_MAX && !(iter->ar_type & R_DMN_FUSER)) {
   instruction_t *static_instr;
   /* We can encode this instruction using its 8-bit variant.
    * NOTE: This optimization assumes the (currently correct)
    *       fact that all static variable index operands are
    *       located one byte offset from the start of an instruction,
    *       as well as the fact that all 16-bit static instructions
    *       have their lowest bit set. */
   static_instr = (instruction_t *)imm_addr - 2;
   ASSERT(*(static_instr + 0) == ASM_EXTENDED1);
   if (*(static_instr + 1) == (ASM16_STATIC & 0xff)) {
    /* Delete the leading byte, rather than the trailing.
     * This is done so we don't have to deal with moving the
     * next instruction, too. (After all: this one's a prefix) */
    *(static_instr + 0) = ASM_DELOP;
    *(static_instr + 1) = ASM_DELOP;
    *(static_instr + 2) = ASM_STATIC;
    *(static_instr + 3) = (uint8_t)static_id;
   } else {
    /* Simple case: just transform it into an 8-bit instruction. */
    *(static_instr + 0) = ASM_DELOP; /* Mark unused bytes to-be deleted. */
    *(static_instr + 2) = (uint8_t)static_id;
    *(static_instr + 3) = ASM_DELOP;
   }
  } else {
   /* Simply override the operand. */
   UNALIGNED_SETLE16(imm_addr,static_id);
  }
  /* Delete this relocation. */
  asm_reldel(iter);
 }
 return 0;
}

INTERN int DCALL asm_linkstack(void) {
 struct asm_rel *iter,*end;
 instruction_t *code = sc_main.sec_begin;
 /* Link together text and resolve relocations. */
 end = (iter = sc_main.sec_relv)+sc_main.sec_relc;
 for (; iter != end; ++iter) {
  instruction_t *target; int32_t rel_value;
  if (!REL_HASSYM(iter->ar_type & ~(R_DMN_FUSER))) continue;
  target = code + iter->ar_addr;
  ASSERT(iter->ar_sym);
  ASSERT(ASM_SYM_DEFINED(iter->ar_sym));
  rel_value  = iter->ar_sym->as_addr;
  rel_value -= iter->ar_addr;
  switch (iter->ar_type & ~(R_DMN_FUSER)) {
  case R_DMN_STCK8:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = iter->ar_sym->as_stck - *(int8_t *)target;
   if unlikely(rel_value < INT8_MIN ||
               rel_value > INT8_MAX)
      goto trunc;
   *(int8_t *)target = (int8_t)rel_value;
   break;
  case R_DMN_STCK16:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = iter->ar_sym->as_stck - (int16_t)UNALIGNED_GETLE16((uint16_t *)target);
   if unlikely(rel_value < INT16_MIN ||
               rel_value > INT16_MAX)
      goto trunc;
   if (current_assembler.a_flag&ASM_FOPTIMIZE &&
      (int16_t)rel_value >= INT8_MIN &&
      (int16_t)rel_value <= INT8_MAX &&
       target >= code+2 && target[-2] == ASM_EXTENDED1 &&
     !(iter->ar_type & R_DMN_FUSER)) {
    *(int8_t *)target = (int8_t)rel_value;
    target[-2] = ASM_DELOP;
    target[1]  = ASM_DELOP;
   } else {
    UNALIGNED_SETLE16((uint16_t *)target,(uint16_t)(int16_t)rel_value);
   }
   break;
  case R_DMN_STCKAS8:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = *(int8_t *)target + iter->ar_sym->as_stck;
   if unlikely(rel_value < INT8_MIN || rel_value > INT8_MAX)
      goto trunc;
   *(int8_t *)target = (int8_t)rel_value;
   break;
  case R_DMN_STCKA16:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = UNALIGNED_GETLE16((uint16_t *)target) + iter->ar_sym->as_stck;
   if unlikely((uint16_t)rel_value > UINT16_MAX)
      goto trunc;
   if (current_assembler.a_flag&ASM_FOPTIMIZE &&
       rel_value < UINT8_MAX &&
       target >= code+2 && target[-2] == ASM_EXTENDED1 &&
     !(iter->ar_type & R_DMN_FUSER)) {
    *(uint8_t *)target = (uint8_t)rel_value;
    target[-2] = ASM_DELOP;
    target[1]  = ASM_DELOP;
   } else {
    UNALIGNED_SETLE16((uint16_t *)target,(uint16_t)rel_value);
   }
   break;

  {
   instruction_t *target_end;
   uint16_t num_keep,num_delete;
  case R_DMN_DELHAND:
   if (iter->ar_value < iter->ar_sym->as_hand) break;
   num_keep   = iter->ar_value - iter->ar_sym->as_hand;
   num_delete = iter->ar_value - num_keep;
   while (num_keep--) target = asm_nextinstr(target);
   target_end = target;
   while (num_delete--) target_end = asm_nextinstr(target_end);
   /* Delete affected instructions. */
   memset(target,ASM_DELOP,(size_t)(target_end-target));
  } break;

  default: continue;
  }
  /* Delete this relocation now that it's been handled. */
  asm_reldel(iter);
 }
 return 0;
trunc:
 return 1;
}

INTERN int DCALL asm_linktext(void) {
 struct asm_rel *iter,*end;
 instruction_t *code = sc_main.sec_begin;
 /* Link together text and resolve relocations. */
 end = (iter = sc_main.sec_relv)+sc_main.sec_relc;
 for (; iter != end; ++iter) {
  instruction_t *target; int32_t rel_value;
  if ((iter->ar_type & ~(R_DMN_FUSER)) == R_DMN_NONE) continue;
  target = code + iter->ar_addr;
  ASSERT(iter->ar_sym);
  ASSERT(ASM_SYM_DEFINED(iter->ar_sym));
  rel_value  = iter->ar_sym->as_addr;
  rel_value -= iter->ar_addr;
  switch (iter->ar_type & ~(R_DMN_FUSER)) {
  case R_DMN_ABS16:
   rel_value += iter->ar_addr;
   rel_value += UNALIGNED_GETLE16((uint16_t *)target);
   if unlikely((uint32_t)rel_value > UINT16_MAX)
      goto trunc;
   UNALIGNED_SETLE16((uint16_t *)target,(uint16_t)rel_value);
   break;
  case R_DMN_ABS32:
   rel_value += iter->ar_addr;
   rel_value += UNALIGNED_GETLE32((uint32_t *)target);
   UNALIGNED_SETLE32((uint32_t *)target,(uint32_t)rel_value);
   break;
  case R_DMN_ABSS8:
   rel_value += iter->ar_addr;
   ATTR_FALLTHROUGH
  case R_DMN_DISP8:
   rel_value += *(int8_t *)target;
   if unlikely(rel_value < INT8_MIN ||
               rel_value > INT8_MAX)
      goto trunc;
   *(int8_t *)target = (int8_t)rel_value;
   break;
  case R_DMN_DISP16:
   rel_value += (int16_t)UNALIGNED_GETLE16((uint16_t *)target);
   if unlikely(rel_value < INT16_MIN ||
               rel_value > INT16_MAX)
      goto trunc;
   UNALIGNED_SETLE16((uint16_t *)target,(uint16_t)(int16_t)rel_value);
   break;
  case R_DMN_DISP32:
   rel_value += (int32_t)UNALIGNED_GETLE32((uint32_t *)target);
   if unlikely(rel_value < INT32_MIN ||
               rel_value > INT32_MAX)
      goto trunc;
   UNALIGNED_SETLE32((uint32_t *)target,(uint32_t)(int32_t)rel_value);
   break;
  case R_DMN_STCK8:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = iter->ar_sym->as_stck - *(int8_t *)target;
   if unlikely(rel_value < INT8_MIN ||
               rel_value > INT8_MAX)
      goto trunc;
   *(int8_t *)target = (int8_t)rel_value;
   break;
  case R_DMN_STCK16:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = iter->ar_sym->as_stck - (int16_t)UNALIGNED_GETLE16((uint16_t *)target);
   if unlikely(rel_value < INT16_MIN ||
               rel_value > INT16_MAX)
      goto trunc;
   UNALIGNED_SETLE16((uint16_t *)target,(uint16_t)(int16_t)rel_value);
   break;
  case R_DMN_STCKAS8:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = *(int8_t *)target + iter->ar_sym->as_stck;
   if unlikely(rel_value < INT8_MIN || rel_value > INT8_MAX)
      goto trunc;
   *(int8_t *)target = (int8_t)rel_value;
   break;
  case R_DMN_STCKA16:
   ASSERT(iter->ar_sym->as_stck != ASM_SYM_STCK_INVALID);
   rel_value = UNALIGNED_GETLE16((uint16_t *)target) + iter->ar_sym->as_stck;
   if unlikely((uint16_t)rel_value > UINT16_MAX)
      goto trunc;
   UNALIGNED_SETLE16((uint16_t *)target,(uint16_t)rel_value);
   break;

  {
   instruction_t *target_end;
   uint16_t num_keep,num_delete;
  case R_DMN_DELHAND:
   if (iter->ar_value < iter->ar_sym->as_hand) break;
   num_keep   = iter->ar_value - iter->ar_sym->as_hand;
   num_delete = iter->ar_value - num_keep;
   while (num_keep--) target = asm_nextinstr(target);
   target_end = target;
   while (num_delete--) target_end = asm_nextinstr(target_end);
   /* Delete affected instructions. */
   memset(target,ASM_DELOP,(size_t)(target_end-target));
  } break;

  default: break;
  }
 }
 return 0;
trunc:
 return 1;
}


INTERN struct except_handler *DCALL asm_pack_exceptv(void) {
 struct except_handler *exceptv;
 struct asm_exc *begin,*iter;
 struct except_handler *dst;
#ifndef CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER
 exceptv = (struct except_handler *)Dee_Malloc(current_assembler.a_exceptc*
                                               sizeof(struct except_handler));
 if unlikely(!exceptv) return NULL; /* Well... $h1t. */
#else /* !CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER */
 exceptv = (struct except_handler *)current_assembler.a_exceptv;
 if (current_assembler.a_exceptc != current_assembler.a_excepta) {
  exceptv = (struct except_handler *)Dee_TryRealloc(exceptv,current_assembler.a_exceptc*
                                                    sizeof(struct except_handler));
  if (exceptv) current_assembler.a_exceptv = (struct asm_exc *)exceptv;
  else  exceptv = (struct except_handler *)current_assembler.a_exceptv;
 }
#endif /* CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER */
 dst  = exceptv+current_assembler.a_exceptc;
 iter = (begin = current_assembler.a_exceptv)+
                 current_assembler.a_exceptc;
 while ((--dst,iter-- != begin)) {
#ifndef CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER
  dst->eh_mask  = iter->ex_mask;
  dst->eh_flags = iter->ex_flags;
#endif
  ASSERT(iter->ex_start);
  ASSERT(iter->ex_end);
  ASSERT(iter->ex_addr);
  ASSERT(ASM_SYM_DEFINED(iter->ex_start));
  ASSERT(ASM_SYM_DEFINED(iter->ex_end));
  ASSERT(ASM_SYM_DEFINED(iter->ex_addr));
  ASSERT(iter->ex_start->as_addr <= (code_size_t)(sc_main.sec_iter - sc_main.sec_begin));
  ASSERT(iter->ex_end->as_addr   <= (code_size_t)(sc_main.sec_iter - sc_main.sec_begin));
  ASSERT(iter->ex_addr->as_addr  <= (code_size_t)(sc_main.sec_iter - sc_main.sec_begin));
  dst->eh_stack = iter->ex_addr->as_stck;
  dst->eh_start = iter->ex_start->as_addr;
  dst->eh_end   = iter->ex_end->as_addr;
  dst->eh_addr  = iter->ex_addr->as_addr;
#if __SIZEOF_POINTER__ > 4
  dst->eh_pad   = 0;
#endif
 }
#ifndef CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER
 Dee_Free(current_assembler.a_exceptv);
#endif
 return exceptv;
}


INTERN DREF DeeCodeObject *DCALL asm_gencode(void) {
 DREF DeeDDIObject *ddi;
 DREF DeeCodeObject *result;
 code_size_t total_codesize;
 struct except_handler *exceptv;
 ASSERT(sc_main.sec_code);
 ddi = ddi_compile();
 if unlikely(!ddi) return NULL;
 total_codesize = (code_size_t)(sc_main.sec_iter - sc_main.sec_begin);
 if (sc_main.sec_iter != sc_main.sec_end) {
  /* Try to release as much code memory as possible. - We won't be needing it anymore. */
  result = (DREF DeeCodeObject *)DeeGCObject_TryRealloc(sc_main.sec_code,offsetof(DeeCodeObject,co_code)+
                                                       (sc_main.sec_iter-sc_main.sec_begin)*sizeof(instruction_t));
  if (!result) result = sc_main.sec_code;
 } else {
  result = sc_main.sec_code;
 }
 if (current_assembler.a_staticc != current_assembler.a_statica) {
  DREF DeeObject **svec;
  svec = (DREF DeeObject **)Dee_TryRealloc(current_assembler.a_staticv,
                                           current_assembler.a_staticc*
                                           sizeof(DREF DeeObject *));
  if (svec) current_assembler.a_staticv = svec;
 }
 
 /* Make the exception handler vector become
  * compatible with `struct except_handler'. */
 if (current_assembler.a_exceptc) {
  exceptv = asm_pack_exceptv();
  if unlikely(!exceptv) {
   Dee_Decref(ddi);
   return NULL; /* Well... $h1t. */
  }
 } else {
  exceptv = NULL;
 }

 DeeObject_Init(result,&DeeCode_Type);
 /* Inherit data from the current base-scope. */
 ASSERT((current_basescope->bs_default  != NULL) ==
        (current_basescope->bs_argc_min != current_basescope->bs_argc_max));
 result->co_flags      = current_basescope->bs_flags;
 result->co_localc     = current_assembler.a_localc;
 result->co_staticc    = current_assembler.a_constc;
 result->co_refc       = current_assembler.a_refc;
 result->co_exceptc    = current_assembler.a_exceptc;
 result->co_argc_min   = current_basescope->bs_argc_min;
 result->co_argc_max   = current_basescope->bs_argc_max;
 result->co_padding    = 0;
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->co_static_lock);
#endif
 result->co_framesize  = (current_assembler.a_localc+
                          current_assembler.a_stackmax)*
                          sizeof(DeeObject *);
 result->co_codebytes  = total_codesize;
 result->co_keywords   = NULL; /* TODO */
 result->co_defaultv   = current_basescope->bs_default;
 result->co_staticv    = current_assembler.a_constv;
 result->co_exceptv    = exceptv;
 result->co_ddi        = ddi; /* Inherit reference. */
 result->co_next       = current_rootscope->rs_code;
 current_rootscope->rs_code = result;
 Dee_Incref(result); /* The reference that is stored in the root-scope. */

 /* Set the heapframe flag when the code's frame size is extremely large. */
 if unlikely(result->co_framesize >= CODE_LARGEFRAME_THRESHOLD)
    result->co_flags |= CODE_FHEAPFRAME;


 /* Yes, we steal all of this stuff! */
 sc_main.sec_code               = NULL; /* Prevent cleanup code from trying to free this. */
 sc_main.sec_begin              = NULL; /* *ditto* */
 sc_main.sec_iter               = NULL; /* ... */
 sc_main.sec_end                = NULL; 
 current_basescope->bs_default  = NULL; 
 current_basescope->bs_argc_max = current_basescope->bs_argc_min;
 current_assembler.a_consta     = 0;
 current_assembler.a_constc     = 0;
 current_assembler.a_constv     = NULL;
 current_assembler.a_exceptv    = NULL;
 current_assembler.a_exceptc    = 0;
 current_assembler.a_excepta    = 0;

 /* Now that it's fully initialized, start tracking the generated code object! */
 DeeGC_Track((DeeObject *)result);

 /* And we're done! */
 return result;
}


/* Assembly writing. */
INTERN struct asm_rel *(FCALL asm_allocrel)(void) {
 struct asm_rel *result;
 ASSERT(current_assembler.a_curr->sec_relc <=
        current_assembler.a_curr->sec_rela);
 result = current_assembler.a_curr->sec_relv;
 if (current_assembler.a_curr->sec_relc ==
     current_assembler.a_curr->sec_rela) {
  size_t new_rela = current_assembler.a_curr->sec_rela;
  if (!new_rela) new_rela = 1;
  while (new_rela <= current_assembler.a_curr->sec_relc) new_rela *= 2;
  /* Must allocate more relocations. */
do_realloc:
  result = (struct asm_rel *)Dee_TryRealloc(current_assembler.a_curr->sec_relv,
                                            new_rela*sizeof(struct asm_rel));
  if unlikely(!result) {
   if (new_rela != current_assembler.a_curr->sec_relc+1) {
    new_rela = current_assembler.a_curr->sec_relc+1;
    goto do_realloc;
   }
   if (Dee_CollectMemory(new_rela*sizeof(struct asm_rel)))
       goto do_realloc;
   return NULL;
  }
  current_assembler.a_curr->sec_relv = result;
  current_assembler.a_curr->sec_rela = new_rela;
 }
 result += current_assembler.a_curr->sec_relc++;
 return result;
}

INTERN instruction_t *(FCALL asm_alloc)(size_t n_bytes) {
 instruction_t *result; size_t min_size,new_size;
 ASSERT(current_assembler.a_curr);
 result = current_assembler.a_curr->sec_iter;
 if likely(result+n_bytes <= current_assembler.a_curr->sec_end) {
  /* Fast path: the section already has enough buffer memory allocated. */
  current_assembler.a_curr->sec_iter = result+n_bytes;
  goto end;
 }
 new_size = (current_assembler.a_curr->sec_iter -
             current_assembler.a_curr->sec_begin);
 min_size = new_size+n_bytes;
 if (!new_size) new_size = INITIAL_TEXTALLOC;
 while (new_size < min_size) new_size *= 2;
 if (current_assembler.a_curr->sec_code) {
  /* Must re-allocate the code object instead. */
  ASSERT(current_assembler.a_curr->sec_begin ==
         current_assembler.a_curr->sec_code->co_code);
  result = (instruction_t *)DeeGCObject_TryRealloc(current_assembler.a_curr->sec_code,
                                                   offsetof(DeeCodeObject,co_code)+
                                                   new_size*sizeof(instruction_t));
  if unlikely(!result) {
   if (new_size != min_size) { new_size = min_size; goto realloc_instr; }
   if (Dee_CollectMemory(offsetof(DeeCodeObject,co_code)+
                         new_size*sizeof(instruction_t)))
       goto realloc_instr;
   return NULL;
  }
  current_assembler.a_curr->sec_code = (DeeCodeObject *)result;
  result = ((DeeCodeObject *)result)->co_code;
 } else {
realloc_instr: /* Directly re-allocate code. */
  result = (instruction_t *)Dee_TryRealloc(current_assembler.a_curr->sec_begin,
                                           new_size*sizeof(instruction_t));
  if unlikely(!result) {
   if (new_size != min_size) { new_size = min_size; goto realloc_instr; }
   if (Dee_CollectMemory(new_size*sizeof(instruction_t))) goto realloc_instr;
   return NULL;
  }
 }
 current_assembler.a_curr->sec_iter  = result + (current_assembler.a_curr->sec_iter -
                                                 current_assembler.a_curr->sec_begin);
 current_assembler.a_curr->sec_begin = result;
 current_assembler.a_curr->sec_end   = result + new_size;
 result = current_assembler.a_curr->sec_iter;
 current_assembler.a_curr->sec_iter += n_bytes;
 ASSERT(current_assembler.a_curr->sec_iter <=
        current_assembler.a_curr->sec_end);
end:
#ifndef NDEBUG
 memset(result,0xcc,n_bytes);
#endif
 return result;
}
INTERN int (DCALL asm_putrel)(uint16_t type,
                              struct asm_sym *sym,
                              uint16_t value) {
 struct asm_rel *rel = asm_allocrel();
 if unlikely(!rel) goto err;
 rel->ar_addr  = asm_ip();
 rel->ar_sym   = sym;
 rel->ar_type  = type;
 rel->ar_value = value;
 if (sym) ++sym->as_used;
 return 0;
err:
 return -1;
}
INTERN int (DCALL asm_put)(instruction_t instr) {
 instruction_t *result = asm_alloc(sizeof(instruction_t));
 if unlikely(!result) goto err;
 *(result + 0) = instr;
 return 0;
err:
 return -1;
}
INTERN int (DCALL asm_put16)(uint16_t instr) {
 instruction_t *result;
 if (!(instr&0xff00))
       return asm_put((instruction_t)instr);
 result = asm_alloc(2*sizeof(instruction_t));
 if unlikely(!result) goto err;
 *(result + 0) = (uint8_t)(instr >> 8);
 *(result + 1) = (uint8_t)(instr);
 return 0;
err:
 return -1;
}
#ifndef CONFIG_BIG_ENDIAN
INTERN int (DCALL asm_put_data16)(uint16_t data) {
 uint16_t *result;
 result = (uint16_t *)asm_alloc(sizeof(uint16_t));
 if unlikely(!result) goto err;
 UNALIGNED_SETLE16(result,data);
 return 0;
err:
 return -1;
}
#endif
INTERN int (DCALL asm_put_data32)(uint32_t data) {
 uint32_t *result;
 result = (uint32_t *)asm_alloc(sizeof(uint32_t));
 if unlikely(!result) goto err;
 UNALIGNED_SETLE32(result,data);
 return 0;
err:
 return -1;
}
INTERN int (DCALL asm_put_data64)(uint64_t data) {
 uint64_t *result;
 result = (uint64_t *)asm_alloc(sizeof(uint64_t));
 if unlikely(!result) goto err;
 UNALIGNED_SETLE64(result,data);
 return 0;
err:
 return -1;
}


INTERN int (DCALL asm_putimm8)(instruction_t instr, uint8_t imm8) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+1);
 if likely(result) {
  *(result + 0) = instr;
  *(uint8_t *)(result + 1) = imm8;
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm8_8)(instruction_t instr, uint8_t imm8_1, uint8_t imm8_2) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+2);
 if likely(result) {
  *(result + 0) = instr;
  *(uint8_t *)(result + 1) = imm8_1;
  *(uint8_t *)(result + 2) = imm8_2;
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm8_8_8)(instruction_t instr, uint8_t imm8_1, uint8_t imm8_2, uint8_t imm8_3) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+3);
 if likely(result) {
  *(result + 0) = instr;
  *(uint8_t *)(result + 1) = imm8_1;
  *(uint8_t *)(result + 2) = imm8_2;
  *(uint8_t *)(result + 3) = imm8_3;
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm8_16)(instruction_t instr, uint8_t imm8_1, uint16_t imm16_2) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+3);
 if likely(result) {
  *(result + 0) = instr;
  *(uint8_t *)(result + 1) = imm8_1;
  UNALIGNED_SETLE16((uint16_t *)(result + 2),imm16_2);
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm16)(instruction_t instr, uint16_t imm16) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+2);
 if likely(result) {
  *(result + 0) = instr;
  UNALIGNED_SETLE16((uint16_t *)(result + 1),imm16);
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm16_8)(instruction_t instr, uint16_t imm16_1, uint8_t imm8_2) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+3);
 if likely(result) {
  *(result + 0) = instr;
  UNALIGNED_SETLE16((uint16_t *)(result + 1),imm16_1);
  *(uint8_t *)(result + 3) = imm8_2;
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm16_16)(instruction_t instr, uint16_t imm16_1, uint16_t imm16_2) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+4);
 if likely(result) {
  *(result + 0) = instr;
  UNALIGNED_SETLE16((uint16_t *)(result + 1),imm16_1);
  UNALIGNED_SETLE16((uint16_t *)(result + 3),imm16_2);
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm16_8_16)(instruction_t instr, uint16_t imm16_1, uint8_t imm8_2, uint16_t imm16_3) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+5);
 if likely(result) {
  *(result + 0) = instr;
  UNALIGNED_SETLE16((uint16_t *)(result + 1),imm16_1);
  *(uint8_t *)(result + 3) = imm8_2;
  UNALIGNED_SETLE16((uint16_t *)(result + 4),imm16_3);
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm16_16_8)(instruction_t instr, uint16_t imm16_1, uint16_t imm16_2, uint8_t imm8_3) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+5);
 if likely(result) {
  *(result + 0) = instr;
  UNALIGNED_SETLE16((uint16_t *)(result + 1),imm16_1);
  UNALIGNED_SETLE16((uint16_t *)(result + 3),imm16_2);
  *(uint8_t *)(result + 5) = imm8_3;
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm16_16_16)(instruction_t instr, uint16_t imm16_1, uint16_t imm16_2, uint16_t imm16_3) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+6);
 if likely(result) {
  *(result + 0) = instr;
  UNALIGNED_SETLE16((uint16_t *)(result + 1),imm16_1);
  UNALIGNED_SETLE16((uint16_t *)(result + 3),imm16_2);
  UNALIGNED_SETLE16((uint16_t *)(result + 5),imm16_3);
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putimm32)(instruction_t instr, uint32_t imm32) {
 instruction_t *result = asm_alloc(sizeof(instruction_t)+4);
 if likely(result) {
  *(result + 0) = instr;
  UNALIGNED_SETLE32((uint32_t *)(result + 1),imm32);
  return 0;
 }
 return -1;
}
INTERN int (DCALL asm_putsid16)(uint16_t instr, uint16_t sid) {
 instruction_t *result = asm_alloc(sizeof(uint16_t)+2);
 if likely(result) {
  struct asm_rel *rel = asm_allocrel();
  if unlikely(!rel) return -1;
  *(uint8_t *)(result + 0) = (uint8_t)((instr & 0xff00) >> 8);
  *(uint8_t *)(result + 1) = (uint8_t)(instr & 0xff);
  UNALIGNED_SETLE16((uint16_t *)(result + 2),sid);
  rel->ar_addr = asm_ip() - 2;
  rel->ar_type = R_DMN_STATIC16;
  return 0;
 }
 return -1;
}


PRIVATE void DCALL
relint_fini(DeeRelIntObject *__restrict self) {
 ASSERT(self->ri_sym);
 --self->ri_sym->as_used;
}

PRIVATE DREF DeeObject *DCALL
relint_eq(DeeRelIntObject *__restrict self,
          DeeRelIntObject *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&DeeRelInt_Type))
     return NULL;
 return_bool(self->ri_sym == other->ri_sym &&
             self->ri_add == other->ri_add &&
             self->ri_mode == other->ri_mode);
}
PRIVATE DREF DeeObject *DCALL
relint_ne(DeeRelIntObject *__restrict self,
          DeeRelIntObject *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&DeeRelInt_Type))
     return NULL;
 return_bool(self->ri_sym != other->ri_sym ||
             self->ri_add != other->ri_add ||
             self->ri_mode != other->ri_mode);
}

PRIVATE struct type_cmp relint_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&relint_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&relint_ne,
};

INTERN DeeTypeObject DeeRelInt_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_relint",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeRelIntObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&relint_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL,
        /* .tp_deepload    = */NULL
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
    /* .tp_cmp           = */&relint_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

INTERN DREF DeeObject *DCALL
DeeRelInt_New(struct asm_sym *__restrict sym,
              int_t addend, uint16_t mode) {
 DREF DeeRelIntObject *result;
 result = DeeObject_MALLOC(DeeRelIntObject);
 if unlikely(!result) goto err;
 ++sym->as_used;
 result->ri_sym  = sym;
 result->ri_add  = addend;
 result->ri_mode = mode;
 DeeObject_Init(result,&DeeRelInt_Type);
 return (DREF DeeObject *)result;
err:
 return NULL;
}

INTERN int32_t DCALL
asm_newrelint(struct asm_sym *sym,
              int_t addend, uint16_t mode) {
 DREF DeeObject *obj; int32_t result;
 if likely(sym) {
  obj = DeeRelInt_New(sym,addend,mode);
 } else {
  obj = DeeInt_NewS64(addend);
 }
 if unlikely(!obj) goto err;
 result = asm_newconst(obj);
 Dee_Decref_unlikely(obj);
 return result;
err:
 return -1;
}

PRIVATE int DCALL fix_relint(DeeObject **__restrict pobj) {
 int_t value; DREF DeeObject *intob;
 DeeRelIntObject *relint = (DeeRelIntObject *)*pobj;
 ASSERT(!DeeObject_IsShared(relint));
 ASSERT(relint->ri_sym);
 ASSERT(ASM_SYM_DEFINED(relint->ri_sym));
 value = relint->ri_add;
 if (relint->ri_mode == RELINT_MODE_FADDR)
      value += relint->ri_sym->as_addr;
 else value += relint->ri_sym->as_stck;
 intob = DeeInt_NewS64(value);
 if unlikely(!intob) return -1;
 /* Replace the constant slot with this value. */
 *pobj = (DREF DeeObject *)intob; /* Inherit reference (x2) */
 Dee_DecrefDokill(relint);
 return 0;
}

PRIVATE int DCALL fix_relint_r(DeeObject **__restrict pobj) {
 DeeObject *obj = *pobj;
 if (DeeObject_IsShared(obj)) {
  ASSERT(!DeeObject_InstanceOfExact(obj,&DeeRelInt_Type));
  return 0;
 }
 if (DeeObject_InstanceOfExact(obj,&DeeRelInt_Type))
     return fix_relint(pobj);
 if (DeeTuple_Check(obj)) {
  /* Tuples can also appear as part of jump-tables. */
  size_t i;
  if (DeeTuple_IsEmpty(obj)) goto done;
  for (i = 0; i < DeeTuple_SIZE(obj); ++i)
     if (fix_relint_r(&DeeTuple_ELEM(obj)[i])) goto err;
  goto done;
 }
 if (DeeRoDict_Check(obj)) {
  /* RO-dict objects can easily appear as part of jump-tables. */
  struct rodict_item *iter,*end;
  iter = ((DeeRoDictObject *)obj)->rd_elem;
  end = iter + ((DeeRoDictObject *)obj)->rd_mask+1;
  for (; iter < end; ++iter) {
   if (!iter->di_key) continue;
   if (fix_relint_r(&iter->di_value)) goto err;
  }
 }
done:
 return 0;
err:
 return -1;
}

INTERN int DCALL asm_applyconstrel(void) {
 uint16_t i;
 for (i = 0; i < current_assembler.a_constc; ++i) {
  if (fix_relint_r(&current_assembler.a_constv[i]))
      goto err;
 }
 return 0;
err:
 return -1;
}

INTERN int DCALL
asm_gpush_abs(struct asm_sym *__restrict sym) {
 int32_t cid = asm_newrelint(sym,0,RELINT_MODE_FADDR);
 if unlikely(cid < 0) return -1;
 return asm_gpush_const((uint16_t)cid);
}

INTERN int DCALL
asm_gpush_stk(struct asm_sym *__restrict sym) {
 int32_t cid = asm_newrelint(sym,0,RELINT_MODE_FSTCK);
 if unlikely(cid < 0) return -1;
 return asm_gpush_const((uint16_t)cid);
}


PRIVATE int DCALL
asm_do_gjmp(instruction_t instr,
            struct asm_sym *__restrict target) {
 instruction_t *data;
 struct asm_rel *rel;
 ASSERT(instr == ASM_JMP || instr == ASM_JT ||
        instr == ASM_JF || instr == ASM_FOREACH);
 if unlikely((rel = asm_allocrel()) == NULL) goto err;
 rel->ar_sym = target,++target->as_used;
 /* Let's get big-code assembly mode out of the way! */
 if unlikely(current_assembler.a_flag&ASM_FBIGCODE) {
  if (instr == ASM_JMP) {
   if unlikely((data = asm_alloc(6)) == NULL) goto err;
   *           (data + 0) = (instruction_t)((ASM32_JMP & 0xff00) >> 8);
   *           (data + 1) = instr;
   /* -4 to adjust for the ip offset of the immediate value itself. */
   UNALIGNED_SETLE32((uint32_t *)(data + 2),(uint32_t)(int32_t)-4);
  } else if (instr == ASM_FOREACH) {
   /* foreach top, <Simm32>:
    * >>     foreach top, 1f  (8-bit)
    * >>     jmp   2f         (8-bit)
    * >> 1:  jmp   <Simm32>   (32-bit)
    * >> 2:
    */
   if unlikely((data = asm_alloc(10)) == NULL) goto err;
   *           (data + 0) = ASM_FOREACH;
   *(int8_t  *)(data + 1) = 2; /* `sizeof(ASM_JMP) == 2' */
   *           (data + 2) = ASM_JMP;
   *(int8_t  *)(data + 3) = 6; /* `sizeof(ASM32_JMP) == 6' */
   *           (data + 4) = (instruction_t)((ASM32_JMP & 0xff00) >> 8);
   *           (data + 5) = (instruction_t)((ASM32_JMP & 0xff));
   /* -4 to adjust for the ip offset of the immediate value itself. */
   UNALIGNED_SETLE32((uint32_t *)(data + 6),(uint32_t)(int32_t)-4);
  } else {
   /* >>    jnX   1f     // 2
    * >>    jmp32 target // 6
    * >>1:
    */
   if unlikely((data = asm_alloc(8)) == NULL) goto err;
   *           (data + 0) = ASM_JX_NOT(instr);
   *(int8_t  *)(data + 1) = 6; /* sizeof(jmp32) */
   *           (data + 2) = (instruction_t)((ASM32_JMP & 0xff00) >> 8);
   *           (data + 3) = (instruction_t)((ASM32_JMP & 0xff));
   /* -4 to adjust for the ip offset of the immediate value itself. */
   UNALIGNED_SETLE32((uint32_t *)(data + 4),(uint32_t)(int32_t)-4);
  }
  rel->ar_type = R_DMN_DISP32;
  rel->ar_addr = asm_ip()-4;
  goto done;
 }
 /* Generate 16-bit instructions by default.
  * If possible, these will be optimized to 8-bit ones later. */
 if unlikely((data = asm_alloc(3)) == NULL) goto err;
 *(data + 0) = (instruction_t)(instr|1); /* |1 to indicate a 16-bit immediate value. */
 /* -2 to adjust for the ip offset of the immediate value itself. */
 UNALIGNED_SETLE16((uint16_t *)(data + 1),(uint16_t)(int16_t)-2);
 /* Setup the relocation. */
 rel->ar_type = R_DMN_DISP16;
 rel->ar_addr = asm_ip()-2;
done:
 return 0;
err:
 return -1;
}

#undef CONFIG_ASM_ENABLE_JCC_SYMBOLS
#define CONFIG_ASM_ENABLE_JCC_SYMBOLS 1

#ifdef CONFIG_ASM_ENABLE_JCC_SYMBOLS
PRIVATE int DCALL
asm_do_gjcc(struct ast *__restrict cond,
            instruction_t instr,
            struct asm_sym *__restrict target,
            struct ast *__restrict ddi_ast) {
 instruction_t *data;
 struct asm_rel *rel;
 ASSERT(instr == ASM_JT || instr == ASM_JF);
 if (cond->a_type != AST_SYM || !asm_can_prefix_symbol(cond->a_sym)) {
  if (ast_genasm(cond,ASM_G_FPUSHRES|ASM_G_FLAZYBOOL))
      goto err;
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_do_gjmp(instr,target)) goto err;
  asm_decsp(); /* Consumed by the JT/JF */
  goto done;
 }

 if unlikely((rel = asm_allocrel()) == NULL) goto err;
 /* Emit the symbol prefix. */
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gprefix_symbol(cond->a_sym,cond)) goto err;
 rel->ar_sym = target,++target->as_used;
 /* Let's get big-code assembly mode out of the way! */
 if unlikely(current_assembler.a_flag&ASM_FBIGCODE) {
  /* >>    jnX   1f     // 2
   * >>    jmp32 target // 6
   * >>1:
   */
  if unlikely((data = asm_alloc(8)) == NULL) goto err;
  *           (data + 0) = ASM_JX_NOT(instr);
  *(int8_t  *)(data + 1) = 6; /* sizeof(jmp32) */
  *           (data + 2) = (instruction_t)((ASM32_JMP & 0xff00) >> 8);
  *           (data + 3) = (instruction_t)((ASM32_JMP & 0xff));
  /* -4 to adjust for the ip offset of the immediate value itself. */
  UNALIGNED_SETLE32((uint32_t *)(data + 4),(uint32_t)(int32_t)-4);
  rel->ar_type = R_DMN_DISP32;
  rel->ar_addr = asm_ip()-4;
  goto done;
 }
 /* Generate 16-bit instructions by default.
  * If possible, these will be optimized to 8-bit ones later. */
 if unlikely((data = asm_alloc(3)) == NULL) goto err;
 *(data + 0) = (instruction_t)(instr|1); /* |1 to indicate a 16-bit immediate value. */
 /* -2 to adjust for the ip offset of the immediate value itself. */
 UNALIGNED_SETLE16((uint16_t *)(data + 1),(uint16_t)(int16_t)-2);
 /* Setup the relocation. */
 rel->ar_type = R_DMN_DISP16;
 rel->ar_addr = asm_ip()-2;
done:
 return 0;
err:
 return -1;
}
#endif /* CONFIG_ASM_ENABLE_JCC_SYMBOLS */



INTERN int DCALL
asm_gsetstack_s(struct asm_sym *__restrict target) {
 instruction_t *data; struct asm_rel *rel;
 if unlikely((rel = asm_allocrel()) == NULL) return -1;
 rel->ar_sym = target,++target->as_used;
 if unlikely((data = asm_alloc(4)) == NULL) return -1;
 *(data + 0) = (ASM16_ADJSTACK & 0xff00) >> 8;
 *(data + 1) = (ASM16_ADJSTACK & 0xff);
 UNALIGNED_SETLE16((uint16_t *)(data + 2),current_assembler.a_stackcur);
 rel->ar_addr = asm_ip()-2;
 rel->ar_type = R_DMN_STCK16;
 return 0;
}

INTERN int (DCALL asm_gadjhand)(struct asm_sym *__restrict target) {
 /* Generate code and a relocation to delete
  * unused text once `target' has been linked. */
 uint16_t depth = current_assembler.a_handlerc;
 struct handler_frame *iter; struct asm_rel *rel;
 ASSERT(target);
 iter = current_assembler.a_handler;
 ASSERT((iter != NULL) == (current_assembler.a_handlerc != 0));
 /* Simple case: without any handlers active right
  * now, there doesn't need to be any cleanup! */
 if (!iter) goto done;
 rel = asm_allocrel();
 if unlikely(!rel) goto err;
 rel->ar_type  = R_DMN_DELHAND;
 rel->ar_addr  = asm_ip();
 rel->ar_sym   = target;
 ++target->as_used;
 /* Save the number of instructions we're going to generate for cleanup. */
 rel->ar_value = current_assembler.a_handlerc;
 do {
  ASSERT(depth),--depth;
  /* Generate exception handler cleanup code.
   * This is literally the same as `asm_gunwind()' */
  if (iter->hf_flags&EXCEPTION_HANDLER_FFINALLY) {
   /* Due to the way that the `end finally' instruction is implemented,
    * we are allowed to merge adjacent finally handlers and only emit
    * an instruction for the lowest-order one:
    *  - To prove this to yourself, you may look at the psuedo
    *    code documented for the `ASM_ENDFINALLY' instruction. */
   if (current_assembler.a_flag&ASM_FOPTIMIZE) {
    while (iter->hf_prev &&
          (iter->hf_prev->hf_flags&EXCEPTION_HANDLER_FFINALLY))
           iter = iter->hf_prev,ASSERT(depth),--depth;
   }
   if (asm_gendfinally_n(depth)) goto err_rel;
  } else {
   if (asm_gendcatch()) goto err_rel;
  }
 } while ((iter = iter->hf_prev) != NULL);
 ASSERT(depth == 0);
done:
 return 0;
err_rel:
 rel->ar_type = R_DMN_NONE;
err:
 return -1;
}

INTERN int (DCALL asm_gunwind)(void) {
 uint16_t depth = current_assembler.a_handlerc;
 struct handler_frame *iter;
 iter = current_assembler.a_handler;
 ASSERT((iter != NULL) == (current_assembler.a_handlerc != 0));
 for (;iter; iter = iter->hf_prev) {
  ASSERT(depth),--depth;
  /* Generate exception handler cleanup code. */
  if (iter->hf_flags&EXCEPTION_HANDLER_FFINALLY) {
   if (asm_gendfinally_n(depth)) goto err;
  } else {
   if (asm_gendcatch()) goto err;
  }
 }
 ASSERT(depth == 0);
 return 0;
err:
 return -1;
}


INTERN int (DCALL asm_gjmps)(struct asm_sym *__restrict target) {
 if unlikely(asm_gadjhand(target)) goto err;
 if unlikely(asm_gsetstack_s(target)) goto err;
 return asm_do_gjmp(ASM_JMP,target);
err:
 return -1;
}
INTERN int (DCALL asm_gjcc)(struct ast *__restrict cond,
                            instruction_t instr,
                            struct asm_sym *__restrict target,
                            struct ast *__restrict ddi_ast) {
#ifdef CONFIG_ASM_ENABLE_JCC_SYMBOLS
 struct asm_sym *temp;
 ASSERT(instr == ASM_JT || instr == ASM_JF);
 while (cond->a_type == AST_BOOL) {
  if (cond->a_flag & AST_FBOOL_NEGATE)
      instr = ASM_JX_NOT(instr);
  cond = cond->a_bool;
 }
 if (!(current_assembler.a_flag&ASM_FSTACKDISP))
     return asm_do_gjcc(cond,instr,target,ddi_ast);
 /* Generate special code to adjust the stack before jumping to `target'. */
 /* >>    jnc       1f
  * >>    adjstack #target.sp
  * >>    jmp       target.ip
  * >>1:
  */
 if unlikely((temp = asm_newsym()) == NULL) goto err;
 if unlikely(asm_do_gjcc(cond,ASM_JX_NOT(instr),temp,ddi_ast)) goto err;
 if unlikely(asm_gsetstack_s(target)) goto err;
 if unlikely(asm_do_gjmp(ASM_JMP,target)) goto err;
 asm_defsym(temp);
 return 0;
err:
 return -1;
#else /* CONFIG_ASM_ENABLE_JCC_SYMBOLS */
 ASSERT(instr == ASM_JT || instr == ASM_JF);
 if (cond->a_type == AST_BOOL) {
  if (cond->a_flag&AST_FBOOL_NEGATE)
      instr = ASM_JX_NOT(instr);
  cond = cond->a_bool;
 }
 if (ast_genasm(cond,ASM_G_FPUSHRES|ASM_G_FLAZYBOOL)) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gjmp(instr,target)) goto err;
 asm_decsp(); /* Adjust for `ASM_JT' / `ASM_JF' popping a condition. */
 return 0;
err:
 return -1;
#endif /* !CONFIG_ASM_ENABLE_JCC_SYMBOLS */
}
INTERN int (DCALL asm_gjmp)(instruction_t instr,
                            struct asm_sym *__restrict target) {
 if (!(current_assembler.a_flag&ASM_FSTACKDISP))
     return asm_do_gjmp(instr,target);
 /* Generate special code to adjust the stack before jumping to `target'. */
 switch (instr) {
 /*case ASM_JMP:*/
 default:
  /* Simple case: Directly generate code to adjust the
   *              stack according to `target's wishes. */
  if unlikely(asm_gsetstack_s(target)) goto err;
  return asm_do_gjmp(ASM_JMP,target);
 {
  struct asm_sym *temp;
 case ASM_JT:
 case ASM_JF:
  /* >>    jnc       1f
   * >>    adjstack #target.sp
   * >>    jmp       target.ip
   * >>1:
   */
  if unlikely((temp = asm_newsym()) == NULL) goto err;
  if unlikely(asm_do_gjmp(ASM_JX_NOT(instr),temp)) goto err;
  /* Adjust the stack for `jt' / `jf' popping the argument. */
  --current_assembler.a_stackcur;
  if unlikely(asm_gsetstack_s(target)) goto err;
  if unlikely(asm_do_gjmp(ASM_JMP,target)) goto err;
  asm_defsym(temp);
  ++current_assembler.a_stackcur;
 } break;
 {
  struct asm_sym *temp1;
  struct asm_sym *temp2;
 case ASM_FOREACH:
  /* >>    foreach   1f
   * >>    jmp       2f
   * >>1:  adjstack #target.sp
   * >>    jmp       target.ip
   * >>2:
   * NOTE: If the adjstack offset turns out to be
   *       zero, peephole can fully optimize this! */
  if unlikely((temp1 = asm_newsym()) == NULL) goto err;
  if unlikely((temp2 = asm_newsym()) == NULL) goto err;
  if unlikely(asm_do_gjmp(ASM_FOREACH,temp1)) goto err;
  ++current_assembler.a_stackcur;
  /* We get here when `ASM_FOREACH' pushes a new value.. */
  if unlikely(asm_do_gjmp(ASM_JMP,temp2)) goto err;
  ASSERT(current_assembler.a_stackcur >= 2);
  /* Adjust the stack for `foreach' popping the iterator when done. */
  current_assembler.a_stackcur -= 2;
  asm_defsym(temp1); /* `ASM_FOREACH' will pop the iterator when its done and jump here.
                      *  Therefor, we must define this symbol while already having set
                      *  the proper stack alignment. */
  if unlikely(asm_gsetstack_s(target)) goto err;
  if unlikely(asm_do_gjmp(ASM_JMP,target)) goto err;
  current_assembler.a_stackcur += 2; /* Conversely to above, ASM_FOREACH pushes an element. */
  asm_defsym(temp2);
  --current_assembler.a_stackcur;
 } break;
 }
 return 0;
err:
 return -1;
}


INTERN int32_t DCALL
asm_newconst(DeeObject *__restrict constvalue) {
 int32_t result; DREF DeeObject **iter,**end,*elem;
 ASSERT_OBJECT(constvalue);
 if (!(current_assembler.a_flag & ASM_FNOREUSECONST)) {
  /* Check if we've already got this exact constant. */
  end = (iter = current_assembler.a_constv)+current_assembler.a_constc;
  for (; iter != end; ++iter) {
   elem = *iter;
   if (Dee_TYPE(elem) == Dee_TYPE(constvalue)) {
    int error = DeeObject_CompareEq(constvalue,elem);
    if (error != 0) {
     if unlikely(error < 0)
        return -1;
     /* Got a match for an existing instance! */
     return (int32_t)(iter - current_assembler.a_constv);
    }
   }
  }
 }

 /* Allocate more buffer memory when nothing is left. */
 ASSERT(current_assembler.a_constc <=
        current_assembler.a_consta);
 if (current_assembler.a_constc ==
     current_assembler.a_consta) {
  uint16_t new_consta = current_assembler.a_consta * 2;
  DREF DeeObject **new_vector;
  if (!new_consta) new_consta = 1;
  if unlikely(new_consta < current_assembler.a_constc) {
   new_consta = current_assembler.a_constc+1;
   if unlikely(new_consta < current_assembler.a_constc) {
    DeeError_Throwf(&DeeError_CompilerError,
                    "Too many constant variables");
    return -1;
   }
  }
do_realloc:
  new_vector = (DREF DeeObject **)Dee_TryRealloc(current_assembler.a_constv,
                                                 new_consta*sizeof(DREF DeeObject *));
  if unlikely(!new_vector) {
   if (new_consta != current_assembler.a_constc+1) {
    new_consta = current_assembler.a_constc+1;
    goto do_realloc;
   }
   if (Dee_CollectMemory(new_consta*sizeof(DREF DeeObject *)))
       goto do_realloc;
   return -1;
  }
  current_assembler.a_constv = new_vector;
  current_assembler.a_consta = new_consta;
 }
 result = current_assembler.a_constc;
 current_assembler.a_constv[current_assembler.a_constc++] = constvalue;
 Dee_Incref(constvalue);
 return result;
}

INTERN int32_t DCALL
asm_newstatic(DeeObject *initializer) {
 int32_t result;
 ASSERT_OBJECT_OPT(initializer);
 /* Allocate more buffer memory when nothing is left. */
 ASSERT(current_assembler.a_staticc <=
        current_assembler.a_statica);
 if (current_assembler.a_staticc ==
     current_assembler.a_statica) {
  uint16_t new_statica = current_assembler.a_statica * 2;
  DREF DeeObject **new_vector;
  if (!new_statica) new_statica = 1;
  if unlikely(new_statica < current_assembler.a_staticc) {
   new_statica = current_assembler.a_staticc+1;
   if unlikely(new_statica < current_assembler.a_staticc) {
    DeeError_Throwf(&DeeError_CompilerError,
                    "Too many static variables");
    return -1;
   }
  }
do_realloc:
  new_vector = (DREF DeeObject **)Dee_TryRealloc(current_assembler.a_staticv,
                                                 new_statica*sizeof(DREF DeeObject *));
  if unlikely(!new_vector) {
   if (new_statica != current_assembler.a_staticc+1) {
    new_statica = current_assembler.a_staticc+1;
    goto do_realloc;
   }
   if (Dee_CollectMemory(new_statica*sizeof(DREF DeeObject *)))
       goto do_realloc;
   return -1;
  }
  current_assembler.a_staticv = new_vector;
  current_assembler.a_statica = new_statica;
 }
 result = current_assembler.a_staticc;
 current_assembler.a_staticv[current_assembler.a_staticc++] = initializer;
 Dee_XIncref(initializer);
 return result;
}

INTERN int32_t DCALL asm_newlocal(void) {
 uint8_t *iter,*end,temp; uint16_t result;
 if (!(current_assembler.a_flag&ASM_FREUSELOC)) {
  if unlikely(current_assembler.a_localc == UINT16_MAX)
     goto err_too_many_locals;
  return current_assembler.a_localc++;
 }
 /* Search for unused local variable indices. */
 end = (iter = current_assembler.a_localuse)+
             ((current_assembler.a_localc+7)/8);
 for (; iter != end; ++iter) {
  if (*iter == 0xff) continue;
  /* We might have got something here... */
  temp   = *iter;
  result = (uint16_t)(iter-current_assembler.a_localuse)*8;
  while (temp & 1) ++result,temp >>= 1;
  if (result < current_assembler.a_localc) {
   /* Yes! reuse this one. */
   *iter |= (1 << (result % 8));
   goto end;
  }
  break;
 }
 /* Allocate a new local variable and mark is as in-use. */
 result = current_assembler.a_localc++;
 if unlikely(result == UINT16_MAX)
    goto err_too_many_locals;
 if (result/8 >= current_assembler.a_locala) {
  uint8_t *new_bitset; uint16_t new_size;
  /* Must extend the in-use bitset. */
  new_size = (current_assembler.a_locala*3)/2;
  if (!new_size) new_size = 1;
  if unlikely(new_size <= result) new_size = UINT16_MAX;
do_realloc:
  new_bitset = (uint8_t *)Dee_TryRealloc(current_assembler.a_localuse,new_size);
  if unlikely(!new_bitset) {
   uint16_t minsize = (result+1)/8;
   if (new_size != minsize) { new_size = minsize; goto do_realloc; }
   if (Dee_CollectMemory(new_size)) goto do_realloc;
   return -1;
  }
  current_assembler.a_localuse = new_bitset;
  current_assembler.a_locala   = new_size;
 }
 current_assembler.a_localuse[result / 8] |= 1 << (result % 8);
end:
 return (int32_t)result;
err_too_many_locals:
 DeeError_Throwf(&DeeError_CompilerError,
                 "Too many local variables");
 return -1;
}

INTERN void DCALL asm_dellocal(uint16_t index) {
 ASSERTF(index < current_assembler.a_localc,
         "Invalid variable index");
 /* Emit DDI information for the unbinding of this local variable.
  * If doing this fails, ignore that error. */
 if (asm_putddi_lunbind(index))
     DeeError_Handled(ERROR_HANDLED_RESTORE);
 if (current_assembler.a_flag&ASM_FREUSELOC) {
  ASSERTF(index < current_assembler.a_locala*8,
          "No convert is allocated for this variable");
  ASSERTF(current_assembler.a_localuse[index / 8] & (1 << (index % 8)),
          "The variable wasn't actually in use");
  /* Unset the in-use bit for this variable. */
  current_assembler.a_localuse[index / 8] &= ~(1 << (index % 8));
 }
}


PRIVATE bool DCALL rehash_globals(void) {
 uint16_t new_mask;
 struct module_symbol *new_vector;
 struct module_symbol *iter,*end;
 /* Try to rehash the global variable table. */
 new_mask = (current_rootscope->rs_bucketm << 1)|1;
 new_vector = (struct module_symbol *)Dee_TryCalloc((new_mask+1)*sizeof(struct module_symbol));
 if unlikely(!new_vector) return false;
 if (current_rootscope->rs_bucketv != empty_module_buckets) {
  /* Re-hash the table. */
  end = (iter = current_rootscope->rs_bucketv)+(current_rootscope->rs_bucketm+1);
  for (; iter != end; ++iter) {
   dhash_t i,perturb;
   i = perturb = (iter->ss_hash & new_mask);
   for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
    struct module_symbol *dst = &new_vector[i & new_mask];
    if (dst->ss_name) continue;
    *dst = *iter; /* Transfer this entry. */
    break;
   }
  }
  /* Setup the new table for use. */
  Dee_Free(current_rootscope->rs_bucketv);
 }
 current_rootscope->rs_bucketv = new_vector;
 current_rootscope->rs_bucketm = new_mask;
 return true;
}

INTERN int32_t DCALL
asm_gsymid(struct symbol *__restrict sym) {
 uint16_t result; dhash_t name_hash;
 struct TPPKeyword *name;
 struct module_symbol *iter; dhash_t perturb,i;
 ASSERT(sym);
 ASSERT(sym->s_type == SYMBOL_TYPE_GLOBAL);
 ASSERT_OBJECT_TYPE((DeeObject *)current_rootscope,&DeeRootScope_Type);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;

 /* Figure out the name and hash of this symbol's name.
  * NOTE: This is where we stop using TPP's indices for hashing
  *       and start relying on deemon's own string hashing algorithm,
  *       since the `TPPKeyword' still representing the name of this
  *       symbol won't be around anymore once the module itself has
  *       been compiled. */
 name      = sym->s_name;
 name_hash = hash_ptr(name->k_name,name->k_size);

 /* To prevent multiple-definition problems of the same global variable,
  * global variables are stored by name in the `current_rootscope'
  * (which will eventually be transformed into what will be a `DeeModuleObject').
  * Therefor, despite the fact that this symbol isn't linked against a global
  * variable, there is a chance that a global variable with the same name
  * already exists, in which case we must assign the _SAME_ index to this symbol,
  * creating a somewhat alias for a single, common global variable that can
  * only be expressed as a single, unsigned 16-bit integer known as the GID. */
 perturb = i = name_hash & current_rootscope->rs_bucketm;
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  iter = &current_rootscope->rs_bucketv[i & current_rootscope->rs_bucketm];
  if (!MODULE_SYMBOL_GETNAMESTR(iter)) break;
  if (iter->ss_hash == name_hash &&
      MODULE_SYMBOL_EQUALS(iter,name->k_name,name->k_size)) {
   /* Found a match! - This global variable had already been defined. */
   result = iter->ss_index;
   sym->s_symid = result;
   /* Set the allocated-flag, so we don't have to do this again for this symbol. */
   sym->s_flag |= SYMBOL_FALLOC;
   /* Better late than never... */
   if (!iter->ss_doc && sym->s_global.g_doc) {
    iter->ss_doc    = DeeString_STR(sym->s_global.g_doc);
    iter->ss_flags |= MODSYM_FDOCOBJ;
    Dee_Incref(sym->s_global.g_doc);
   }
   return result;
  }
 }
 /* All right! This is a new one, so we have to do all the
  * work of creating and adding a new `module_symbol'... */
 result = current_rootscope->rs_globalc;
 if unlikely(result == UINT16_MAX) {
  /* Make sure not to exceed what can actually be done. */
  DeeError_Throwf(&DeeError_CompilerError,
                  "Too many global variables");
  goto err;
 }

 /* Try to keep hash collisions to a minimum. */
 if (result >= current_rootscope->rs_bucketm/2)
     rehash_globals();
 /* Make sure that the hash relation remains valid. */
 if (result+1 >= current_rootscope->rs_bucketm &&
    !rehash_globals() && !Dee_CollectMemory(1))
     goto err;

 perturb = i = name_hash & current_rootscope->rs_bucketm;
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  DREF DeeObject *name_obj;
  iter = &current_rootscope->rs_bucketv[i & current_rootscope->rs_bucketm];
  if (MODULE_SYMBOL_GETNAMESTR(iter)) continue;
  name_obj = DeeString_NewSized(name->k_name,name->k_size);
  if unlikely(!name_obj) goto err;
  MODULE_SYMBOL_GETNAMESTR(iter) = DeeString_STR(name_obj);
  ((DeeStringObject *)name_obj)->s_hash = name_hash;
  iter->ss_flags = MODSYM_FNAMEOBJ;
  if (sym->s_global.g_doc) {
   /* Assign a documentation string. */
   iter->ss_doc = DeeString_STR(sym->s_global.g_doc);
   Dee_Incref(sym->s_global.g_doc);
   iter->ss_flags |= MODSYM_FDOCOBJ;
  }
  iter->ss_hash  = name_hash;
  iter->ss_index = result;
  break;
 }
 /* Increment to indicate that this index has now bee taken up. */
 ++current_rootscope->rs_globalc;
 /* Save the generated index in the given symbol. */
 sym->s_symid = result;
 /* Mark the given symbol as being allocated. */
 sym->s_flag |= SYMBOL_FALLOC;
 /* And we're done! */
 return result;
err:
 return -1;
}

INTERN int32_t DCALL
asm_lsymid(struct symbol *__restrict sym) {
 int32_t new_index;
 ASSERT(sym);
 ASSERT(!SYMBOL_MUST_REFERENCE(sym));
 ASSERT(sym->s_type == SYMBOL_TYPE_LOCAL);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;
 /* Allocate a new local variable index for the given symbol. */
 new_index = asm_newlocal();
 if unlikely(new_index < 0) goto end;
 ASSERT(new_index <= UINT16_MAX);
 sym->s_symid = (uint16_t)new_index;
 sym->s_flag |= SYMBOL_FALLOC;
 /* Generate DDI information for the local->symbol binding. */
 if (asm_putddi_lbind((uint16_t)new_index,sym->s_name))
     return -1;
end:
 return new_index;
}

INTERN int32_t DCALL
asm_ssymid(struct symbol *__restrict sym) {
 int32_t new_index;
 ASSERT(sym);
 ASSERT(!SYMBOL_MUST_REFERENCE(sym));
 ASSERT(SYMBOL_TYPE(sym) == SYMBOL_TYPE_STATIC);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;
 /* Allocate a new static variable index for the given symbol.
  * NOTE: By default, `Dee_None' is used for default-initialization of static variables. */
 new_index = asm_newstatic(Dee_None);
 if unlikely(new_index < 0) goto end;
 ASSERT(new_index <= UINT16_MAX);
 sym->s_symid = (uint16_t)new_index;
 sym->s_flag |= SYMBOL_FALLOC;
end:
 return new_index;
}

INTERN int32_t DCALL
asm_gsymid_for_read(struct symbol *__restrict sym,
                    struct ast *__restrict warn_ast) {
 ASSERT(SYMBOL_TYPE(sym) == SYMBOL_TYPE_GLOBAL);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;
 if (!sym->s_nwrite &&
      WARNAST(warn_ast,W_VARIABLE_READ_NEVER_WRITTEN,sym))
      goto err;
 return asm_gsymid(sym);
err:
 return -1;
}
INTERN int32_t DCALL
asm_lsymid_for_read(struct symbol *__restrict sym,
                    struct ast *__restrict warn_ast) {
 ASSERT(!SYMBOL_MUST_REFERENCE(sym));
 ASSERT(sym->s_type == SYMBOL_TYPE_LOCAL);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;
 if (!sym->s_nwrite &&
      WARNAST(warn_ast,W_VARIABLE_READ_NEVER_WRITTEN,sym))
      goto err;
 return asm_lsymid(sym);
err:
 return -1;
}
INTERN int32_t DCALL
asm_ssymid_for_read(struct symbol *__restrict sym,
                    struct ast *__restrict warn_ast) {
 ASSERT(!SYMBOL_MUST_REFERENCE(sym));
 ASSERT(SYMBOL_TYPE(sym) == SYMBOL_TYPE_STATIC);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;
 if (!sym->s_nwrite &&
      WARNAST(warn_ast,W_VARIABLE_READ_NEVER_WRITTEN,sym))
      goto err;
 return asm_ssymid(sym);
err:
 return -1;
}


INTERN int32_t DCALL
asm_rsymid(struct symbol *__restrict sym) {
 uint16_t result;
 ASSERT(SYMBOL_MAY_REFERENCE(sym));
 ASSERTF(asm_symbol_accessible(sym),
         "Unreachable symbol %s",
         sym->s_name->k_name);
 result = current_assembler.a_refc;
 if ((sym->s_flag & SYMBOL_FALLOCREF) &&
     (sym->s_refid < result) &&
      current_assembler.a_refv[sym->s_refid].sr_sym == sym)
      return sym->s_refid;
 ASSERT(result <= current_assembler.a_refa);
 if unlikely(result == UINT16_MAX) {
  return DeeError_Throwf(&DeeError_CompilerError,
                         "Too many reference variables");
 }
 if (result == current_assembler.a_refa) {
  /* Must allocate more references. */
  struct asm_symbol_ref *new_vector; uint16_t new_size;
  new_size = current_assembler.a_refa;
  if (!new_size) new_size = 1;
  new_size *= 2;
  if unlikely(new_size <= result) new_size = UINT16_MAX;
do_realloc:
  new_vector = (struct asm_symbol_ref *)Dee_TryRealloc(current_assembler.a_refv,new_size*
                                                       sizeof(struct asm_symbol_ref));
  if unlikely(!new_vector) {
   if (new_size != result+1) { new_size = result+1; goto do_realloc; }
   if (Dee_CollectMemory(new_size*sizeof(struct asm_symbol_ref))) goto do_realloc;
   return -1;
  }
  current_assembler.a_refv = new_vector;
  current_assembler.a_refa = new_size;
 }
 ++current_assembler.a_refc;
 current_assembler.a_refv[result].sr_sym = sym;
 current_assembler.a_refv[result].sr_orig_refid = sym->s_refid;
 current_assembler.a_refv[result].sr_orig_flag  = sym->s_flag;
 sym->s_refid = result;
 sym->s_flag |= SYMBOL_FALLOCREF;
 return result;
}


INTERN int32_t DCALL
asm_asymid_r(struct symbol *__restrict sym) {
 uint16_t result;
 ASSERT(SYMBOL_MAY_REFERENCE(sym));
 ASSERTF(current_assembler.a_flag & ASM_FARGREFS,
         "Not operating in ARGREF mode");
 ASSERTF(asm_symbol_accessible(sym),
         "Unreachable symbol %s",
         sym->s_name->k_name);
 /* Search for a pre-existing binding for `sym' */
 result = current_assembler.a_argrefc;
 while (result--) {
  if (current_assembler.a_argrefv[result] == sym)
      return current_basescope->bs_argc_max + result;
 }
 /* Allocate a new argref */
 result = current_assembler.a_argrefc;
 ASSERT(result <= current_assembler.a_argrefa);
 if unlikely(result == UINT16_MAX) {
  return DeeError_Throwf(&DeeError_CompilerError,
                         "Too many reference-through-argument variables");
 }
 if (result == current_assembler.a_argrefa) {
  /* Must allocate more references. */
  struct symbol **new_vector; uint16_t new_size;
  new_size = current_assembler.a_argrefa;
  if (!new_size) new_size = 1;
  new_size *= 2;
  if unlikely(new_size <= result) new_size = UINT16_MAX;
do_realloc:
  new_vector = (struct symbol **)Dee_TryRealloc(current_assembler.a_argrefv,
                                                new_size * sizeof(struct symbol *));
  if unlikely(!new_vector) {
   if (new_size != result+1) { new_size = result+1; goto do_realloc; }
   if (Dee_CollectMemory(new_size*sizeof(struct symbol *))) goto do_realloc;
   return -1;
  }
  current_assembler.a_argrefv = new_vector;
  current_assembler.a_argrefa = new_size;
 }
 ++current_assembler.a_argrefc;
 current_assembler.a_argrefv[result] = sym;
 return current_basescope->bs_argc_max + result;
}



INTERN int32_t DCALL
asm_newmodule(DeeModuleObject *__restrict mod) {
 DREF DeeModuleObject **iter,**end; uint16_t result;
 ASSERT_OBJECT_TYPE(mod,&DeeModule_Type);
 ASSERT_OBJECT_TYPE((DeeObject *)current_rootscope,&DeeRootScope_Type);
 /* Check if this module has already been imported by the one calling.
  * Must be checking to ensure that any module is only ever imported once.
  * NOTE: Since modules cannot be copied, we can simply compare pointers here! */
 end = (iter = current_rootscope->rs_importv)+
               current_rootscope->rs_importc;
 for (; iter != end; ++iter) {
  if (*iter == mod)
      return (int32_t)(iter-current_rootscope->rs_importv);
 }
 ASSERT(current_rootscope->rs_importc <=
        current_rootscope->rs_importa);
 result = current_rootscope->rs_importc;
 if unlikely(result == UINT16_MAX) {
  /* Make sure not to exceed what can actually be done. */
  DeeError_Throwf(&DeeError_CompilerError,
                  "Too many imported modules");
  return -1;
 }
 if (result == current_rootscope->rs_importa) {
  /* Must allocate more memory in the import vector. */
  DREF DeeModuleObject **new_vector; uint16_t new_size;
  new_size = current_rootscope->rs_importa;
  if (!new_size) new_size = 1;
  new_size *= 2;
  if unlikely(new_size <= result) new_size = UINT16_MAX;
do_realloc:
  new_vector = (DREF DeeModuleObject **)Dee_TryRealloc(current_rootscope->rs_importv,new_size*
                                                       sizeof(DREF DeeModuleObject *));
  if unlikely(!new_vector) {
   if (new_size != result+1) { new_size = result+1; goto do_realloc; }
   if (Dee_CollectMemory(new_size*sizeof(DREF DeeModuleObject *))) goto do_realloc;
   return -1;
  }
  current_rootscope->rs_importv = new_vector;
  current_rootscope->rs_importa = new_size;
 }
 /* Insert the given module into the import vector. */
 Dee_Incref(mod);
 current_rootscope->rs_importv[result] = mod;
 ++current_rootscope->rs_importc;
 return result;
}


INTERN int32_t DCALL
asm_esymid(struct symbol *__restrict sym) {
 int32_t result;
 DeeModuleObject *module;
 ASSERT(sym);
 ASSERT(SYMBOL_TYPE(sym) == SYMBOL_TYPE_EXTERN);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;
 module = SYMBOL_EXTERN_MODULE(sym);
 if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FEXTERN) {
  ASSERT(SYMBOL_EXTERN_SYMBOL(sym)->ss_extern.ss_impid < module->mo_importc);
  module = module->mo_importv[SYMBOL_EXTERN_SYMBOL(sym)->ss_extern.ss_impid];
 }
 result = asm_newmodule(module);
 if unlikely(result < 0) goto end;
 ASSERT(result <= UINT16_MAX);
 /* Cache the module ID within the symbol. */
 sym->s_symid = (uint16_t)result;
 sym->s_flag |= SYMBOL_FALLOC;
end:
 return result;
}

INTERN int32_t DCALL
asm_msymid(struct symbol *__restrict sym) {
 int32_t result;
 ASSERT(sym);
 ASSERT(SYMBOL_TYPE(sym) == SYMBOL_TYPE_MODULE);
 if (sym->s_flag & SYMBOL_FALLOC)
     return sym->s_symid;
 result = asm_newmodule(SYMBOL_EXTERN_MODULE(sym));
 if unlikely(result < 0) goto end;
 ASSERT(result <= UINT16_MAX);
 /* Cache the module ID within the symbol. */
 sym->s_symid = (uint16_t)result;
 sym->s_flag |= SYMBOL_FALLOC;
end:
 return result;
}

INTERN struct symbol *DCALL
asm_bind_deemon_export(DeeObject *__restrict constval) {
 uint16_t i;
 if (!DeeType_Check(constval)) {
  /* To speed this up, check what the user wants to seach for. */
  if (DeeNone_Check(constval)) { i = id_none; goto did_find_export; }
  if (constval == &DeeGCEnumTracked_Singleton) { i = id_gc; goto did_find_export; }
  if (DeeBool_Check(constval)) goto do_search;
  if (DeeCMethod_Check(constval)) goto do_search;
  goto done;
 }
do_search:
 rwlock_read(&deemon_module.mo_lock);
 for (i = 0; i < num_builtins_obj; ++i) {
  struct symbol *result;
  if (deemon_module.mo_globalv[i] != constval)
      continue;
  rwlock_endread(&deemon_module.mo_lock);
did_find_export:
  /* Check if the symbol has already been bound. */
  result = current_rootscope->rs_scope.bs_scope.s_del;
  for (; result; result = result->s_next) {
   if (result->s_type != SYMBOL_TYPE_EXTERN) continue;
   if (result->s_extern.e_module != &deemon_module) continue;
   if (result->s_extern.e_symbol->ss_index != i) continue;
   return result; /* Found it! */
  }
  result = new_unnamed_symbol_in_scope((DeeScopeObject *)current_rootscope);
  if unlikely(!result) goto done_result;
  result->s_type            = SYMBOL_TYPE_EXTERN;
  result->s_extern.e_module = get_deemon_module();
  result->s_extern.e_symbol = DeeModule_GetSymbolID(&deemon_module,i);
  ASSERT(result->s_extern.e_symbol != NULL);
done_result:
  return result;
 }
 rwlock_endread(&deemon_module.mo_lock);
done:
 return ASM_BIND_DEEMON_EXPORT_NOTFOUND;
}



INTERN int DCALL asm_check_user_labels_defined(void) {
 struct text_label **biter,**bend,*iter;
 bend = (biter = current_basescope->bs_lbl)+
                 current_basescope->bs_lbla;
 for (; biter != bend; ++biter) {
  for (iter = *biter; iter; iter = iter->tl_next) {
   struct asm_sym *sym = iter->tl_asym;
   if (!sym) continue; /* Label was never instantiated. */
   if (!sym->as_used) continue; /* Label isn't being used. */
   if unlikely(!ASM_SYM_DEFINED(sym)) {
    /* Error: User-defined label was never defined. */
    return DeeError_Throwf(&DeeError_CompilerError,
                           "Label `%s' has never been defined",
                           iter->tl_name->k_name);
   }
  }
 }
#ifndef CONFIG_LANGUAGE_NO_ASM
 {
  struct asm_sym *iter = current_assembler.a_syms;
  for (; iter; iter = iter->as_next) {
   if (ASM_SYM_DEFINED(iter)) continue; /* Skip symbols that are defined. */
   if (!iter->as_used) continue; /* Skip symbols that are unused. */
   ASSERTF(iter->as_uname != NULL,
           "Unnamed (aka. internal) symbol not defined.\n"
           "%s(%d) : Symbol was allocated here",
           iter->as_file,iter->as_line);
   return DeeError_Throwf(&DeeError_CompilerError,
                          "Assembly symbol `%s' has never been defined",
                          iter->as_uname->k_name);
  }
 }
#endif
 return 0;
}


INTERN DREF DeeCodeObject *DCALL
code_docompile(struct ast *__restrict code_ast) {
 int link_error; unsigned int i;
 ASSERT_AST(code_ast);
 ASSERT_OBJECT_TYPE((DeeObject *)current_basescope,&DeeBaseScope_Type);

restart:
 /* Generate code for copying modified arguments into locals. */
 {
  bool did_define_header_ddi = false;
  struct symbol **iter,**end,*sym;
  end = (iter = current_basescope->bs_argv)+current_basescope->bs_argc;
  for (; iter != end; ++iter) {
   sym = *iter;
   SYMBOL_INPLACE_UNWIND_ALIAS(sym);
   if (SYMBOL_TYPE(sym) != SYMBOL_TYPE_ARG) {
    uint16_t argid;
do_savearg:
    argid = (uint16_t)(iter - current_basescope->bs_argv);
    if (!did_define_header_ddi) {
     if (asm_putddi(code_ast)) goto err;
     did_define_header_ddi = true;
    }
    if (asm_gmov_varg(sym,argid,code_ast,true)) goto err;
   } else if (sym->s_nwrite != 0) {
    /* Must convert this one into a local variable. */
    sym->s_type  = SYMBOL_TYPE_LOCAL;
    sym->s_flag &= ~SYMBOL_FALLOC;
    goto do_savearg;
#if 0
   } else if (sym->sym_read == 0 &&
              sym->s_symid == current_basescope->bs_argc_max &&
              current_basescope->bs_argc_opt) {
    /* TODO: Option arguments that were never used can something be
     *       converted into default-arguments, at which point we may
     *       be able to omit the runtime check for the number of
     *       variable arguments. */
#endif
   }
  }
  if ((current_basescope->bs_flags & CODE_FVARARGS) &&
      !current_basescope->bs_varargs) {
   /* In a varargs-function without a varargs symbol (which can happen
    * in a function with optional arguments, and a missing varargs tail),
    * we must manually check if the actual argument count is within the
    * expected bounds, and throw a TypeError if they aren't. */
   uint16_t required_va_size; int32_t deemon_modid;
   instruction_t jmp_instruction = ASM_JT;
   struct asm_sym *error_sym; int32_t max_argc_cid;
   DREF DeeObject *max_argc_ob;
   if (!did_define_header_ddi) {
    if (asm_putddi(code_ast)) goto err;
    did_define_header_ddi = true;
   }
   required_va_size = current_basescope->bs_argc_opt;
   if likely(required_va_size) {
    /* >> push cmp gr, #varargs, $<required_va_size>
     * >> jt   .error */
    if (asm_gcmp_gr_varargs_sz(required_va_size)) goto err;
   } else {
    /* >> push cmp eq, #varargs, $0
     * >> jf   .error */
    jmp_instruction = ASM_JF;
    if (_asm_gcmp_eq_varargs_sz(0)) goto err;
   }
   error_sym = asm_newsym();
   if unlikely(!error_sym) goto err;
   if (asm_gjmp(jmp_instruction,error_sym)) goto err;
   asm_decsp();
   /* Generate the error processing code in .cold */
   asm_setcur(SECTION_COLD);
   if (asm_putddi(code_ast)) goto err;
   asm_defsym(error_sym);
   /* Throw a TypeError indicating an invalid argument count:
    * >>    push $<max_argc>
    * >>    call extern @deemon:@__badcall */
   deemon_modid = asm_newmodule(get_deemon_module());
   if unlikely(deemon_modid < 0) goto err;
   max_argc_ob = DeeInt_NewU16(current_basescope->bs_argc);
   if unlikely(!max_argc_ob) goto err;
   max_argc_cid = asm_newconst(max_argc_ob);
   Dee_Decref(max_argc_ob);
   if unlikely(max_argc_cid < 0) goto err;
   if (asm_gpush_const((uint16_t)max_argc_cid)) goto err;
   if (asm_gcall_extern((uint16_t)deemon_modid,id___badcall,1)) goto err;
   if (asm_gret_none()) goto err;
   asm_setcur(SECTION_TEXT);
   ASSERTF(current_assembler.a_stackcur == 1,
           "Only the return value of `__badcall from deemon' should still be there");
   current_assembler.a_stackcur = 0; /* The return value would only exists in .cold */
  }
 }

 /* Generate text assembly for the given code. */
 if unlikely(ast_genasm(code_ast,ASM_G_FNORMAL))
    goto err;

 /* For safety, pad all sections with a return instruction each.
  * NOTE: When not required, these instructions are later removed by peephole optimization. */
 for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
  if (i != SECTION_TEXT &&
      current_assembler.a_sect[i].sec_iter ==
      current_assembler.a_sect[i].sec_begin)
      continue; /* Skip empty seconds. */
  current_assembler.a_curr = &current_assembler.a_sect[i];
  if (asm_put(ASM_RET_NONE)) goto err;
 }

 /* Make sure that all in-use user-labels have been defined.
  * This has to be done manually because the assembler assumes
  * that all symbols that are in use have been defined at this
  * point, and that any symbol that isn't defined but used
  * indicates an internal error. */
 if unlikely(asm_check_user_labels_defined()) goto err;

 /* Merge text sections. */
 if unlikely(asm_mergetext()) goto err;

 /* When peephole optimizations are used, we need to pre-resolve
  * all relocations concerning stack alignments, as the peephole
  * optimizer must be able to keep track of the effective stack
  * depth at any given instruction.
  * NOTE: In order to allow `asm_rmdelop()' to get rid of
  *       unused instruction bytes caused by `R_DMN_DELHAND'
  *       instructions, we also pre-link the stack when
  *      `ASM_FOPTIMIZE' will allow `asm_rmdelop()' to do so. */
 if (current_assembler.a_flag&(ASM_FPEEPHOLE|ASM_FOPTIMIZE)) {
  link_error = asm_linkstack();
  if unlikely(link_error != 0)
     goto err_link;
  /* Remove unused assembly symbols to improve the
   * capabilities of `asm_peephole()'.
   * This function is not called as apart of the optimization
   * cycle below, because peephole optimization will automatically
   * remove unused symbols if it manages to get rid of some symbol. */
  if (current_assembler.a_flag&ASM_FPEEPHOLE)
      asm_delunused_symbols();
 }


 /* Keep shrinking `jmp', deleting DELOP instructions
  * and doing peephole optimizations while possible. */
 for (;;) {
  bool did_something;
  link_error = asm_peephole();
  if unlikely(link_error < 0) goto err;
  did_something = link_error != 0;
  if (asm_rmdelop()) did_something = true;
  /* Only minimize jumps if peephole and rmdelop didn't do anything.
   * This way, we keep just as big as possible to improve how peephole
   * optimizes jump forwarding. - Otherwise, peephole would have a hard
   * time following jumps, if targets are further than 256 bytes apart:
   * >>    jt    pop, 2f
   * >>    jmp   1f
   * >>2:  print @"foo", nl
   * >>1:  jmp   3f
   * >>    // more than 256 bytes of text here
   * >>3:
   * If the first `jmp' was optimized into an 8-bit jump before peephole
   * got around to optimize it into following `jmp 3f', then it wouldn't
   * be able to do that follow, and the final code would be less optimized. */
  if (!did_something) {
   if (!asm_minjmp())
        break;
  }

  /* TODO: Search for unused local/static/const symbols
   *       that are still registered as being used.
   *       This can easily happen when a constant is assigned to a stack variable
   *       that later turned out to never be used, or was used in a way that
   *       caused its value to become unused. */

  /* Check for interrupts in here.
   * -> This allows the user to interrupt compilation, since
   *    peephole and all the other optimizations may take
   *    O(LOG(N)) for very large assembly sizes. */
  if (DeeThread_CheckInterrupt()) goto err;
 }

 /* Merge const + static variables. */
 if unlikely(asm_mergestatic()) goto err;

 /* Apply constant relocations. */
 if unlikely(asm_applyconstrel()) goto err;

 /* Link together the text, resolving relocations. */
 link_error = asm_linktext();
 if unlikely(link_error != 0) {
err_link:
  if unlikely(link_error < 0) goto err;
  if unlikely(current_assembler.a_flag&ASM_FBIGCODE) {
   /* Already in bigcode mode? - That's not good... */
   DeeError_Throwf(&DeeError_CompilerError,
                   "Failed to link final code: Relocation target is out of bounds");
   goto err;
  }
  /* TODO: Restore the `...->bs_default' vectors and `bs_argc_min' and `bs_argc_max'
   *       values for all the base-scope objects reachable through DeeCodeObject objects
   *       found in our current constant-vector, as well as all those code object's
   *       static-variable vectors.
   *    -> This is required to properly allow re-compilation in large-code mode.
   *       Not doing this will cause assertion failures if functions with default-arguments
   *       are used anywhere within user-code.
   */

  /* TODO: Go through all reachable ASTs and for each AST_LABEL delete the associated
   *      `struct text_label::tl_asym' by setting it back to `NULL'.
   *    -> This is required to allow the assembler to define user-labels (including
   *       case-labels of switch-statements) a second time during the bigcode pass. */

  /* Restart the assembly process in bigcode mode. */
  assembler_fini();
  if (assembler_init()) goto err;
  current_assembler.a_flag |= ASM_FBIGCODE;
  goto restart;
 }

 /* Generate the code object. */
 return asm_gencode();
err:
 return NULL;
}

INTERN DREF DeeCodeObject *DCALL
code_compile(struct ast *__restrict code_ast, uint16_t flags,
             uint16_t *__restrict prefc,
             /*out:inherit*/struct asm_symbol_ref **__restrict prefv) {
 struct assembler old_assembler;
 DREF DeeCodeObject *result;
 ASSERT(prefc);
 ASSERT(prefv);

 /* Copy the current, and create a new assembler. */
 memcpy(&old_assembler,&current_assembler,sizeof(struct assembler));
 if unlikely(assembler_init()) goto err;

 /* Keep the old assembler scope so that asts know where their influence ends. */
 current_assembler.a_scope = old_assembler.a_scope;

 /* Set assembler flags. */
 current_assembler.a_flag = flags & ~ASM_FARGREFS;

 /* Actually code the given AST. */
 result = code_docompile(code_ast);

 if (result) {
  ASSERT(!current_assembler.a_argrefc);
  /* Return information about the required references to the caller. */
  *prefc = current_assembler.a_refc;
  *prefv = current_assembler.a_refv;
  /* Steal the vector. */
  current_assembler.a_refv = NULL;
  current_assembler.a_refc = 0;
  current_assembler.a_refa = 0;
 }

 /* Destroy the current assembler. */
 assembler_fini();
end:
 /* Restore the old assembler. */
 memcpy(&current_assembler,&old_assembler,sizeof(struct assembler));
 return result;
err: result = NULL; goto end;
}

INTERN DREF DeeCodeObject *DCALL
code_compile_argrefs(struct ast *__restrict code_ast, uint16_t flags,
                     uint16_t *__restrict prefc,
                     /*out:inherit*/struct asm_symbol_ref **__restrict prefv,
                     uint16_t *__restrict pargc,
                     /*out:inherit*/struct symbol ***__restrict pargv) {
 struct assembler old_assembler;
 DREF DeeCodeObject *result;
 ASSERT(prefc);
 ASSERT(prefv);
 /* Check if the function even qualifies for argrefs. */
 if unlikely(current_basescope->bs_argc_opt ||
             current_basescope->bs_argc_min != current_basescope->bs_argc_max ||
             current_basescope->bs_varargs != NULL ||
            (current_basescope->bs_flags & CODE_FVARARGS)) {
  *pargc = 0;
  *pargv = NULL;
  return code_compile(code_ast,flags,prefc,prefv);
 }

 /* Copy the current, and create a new assembler. */
 memcpy(&old_assembler,&current_assembler,sizeof(struct assembler));
 if unlikely(assembler_init()) goto err;

 /* Keep the old assembler scope so that asts know where their influence ends. */
 current_assembler.a_scope = old_assembler.a_scope;

 /* Set assembler flags. */
 current_assembler.a_flag = flags | ASM_FARGREFS;

 /* Actually code the given AST. */
 result = code_docompile(code_ast);

 if (result) {
  uint16_t i;
  if (current_assembler.a_argrefc) {
   /* Adjust the resulting code object to account for references-through-arguments. */
   if (result->co_keywords) {
    DREF DeeStringObject **new_keyword_vector;
    new_keyword_vector = (DREF DeeStringObject **)Dee_Realloc((void *)result->co_keywords,
                                                              result->co_argc_max *
                                                              sizeof(DREF DeeStringObject *));
    if unlikely(!new_keyword_vector) goto err_r;
    for (i = result->co_argc_max - current_assembler.a_argrefc;
         i < result->co_argc_max; ++i)
         new_keyword_vector[i] = (DeeStringObject *)Dee_EmptyString;
    result->co_keywords = new_keyword_vector;
   }
   if (result->co_argc_min == result->co_argc_max) {
    result->co_argc_min += current_assembler.a_argrefc;
    result->co_argc_max += current_assembler.a_argrefc;
   } else {
    /* We shouldn't get here, but we must still keep consistency! */
    if (result->co_defaultv) {
     DREF DeeObject **new_default_vector;
     new_default_vector = (DREF DeeObject **)Dee_Realloc((void *)result->co_defaultv,
                                                        ((result->co_argc_max - result->co_argc_min) +
                                                          current_assembler.a_argrefc) *
                                                          sizeof(DREF DeeObject *));
     if unlikely(!new_default_vector) goto err_r;
     for (i = result->co_argc_max;
          i < result->co_argc_max + current_assembler.a_argrefc; ++i)
          new_default_vector[i] = Dee_None;
     Dee_Incref_n(Dee_None,current_assembler.a_argrefc);
    }
    result->co_argc_max += current_assembler.a_argrefc;
   }
   if (result->co_keywords)
       Dee_Incref_n(Dee_EmptyString,current_assembler.a_argrefc);
  }
  /* Return information about the required references to the caller. */
  *prefc = current_assembler.a_refc;
  *prefv = current_assembler.a_refv;
  *pargc = current_assembler.a_argrefc;
  *pargv = current_assembler.a_argrefv;
  /* Steal the vector. */
  current_assembler.a_refv    = NULL;
  current_assembler.a_refc    = 0;
  current_assembler.a_refa    = 0;
  current_assembler.a_argrefv = NULL;
  current_assembler.a_argrefc = 0;
  current_assembler.a_argrefa = 0;
 }
end_fini:

 /* Destroy the current assembler. */
 assembler_fini();
end:
 /* Restore the old assembler. */
 memcpy(&current_assembler,&old_assembler,sizeof(struct assembler));
 return result;
err_r:
 Dee_Decref(result);
 goto end_fini;
err:
 result = NULL;
 goto end;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_ASSEMBER_C */
