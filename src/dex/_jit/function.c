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
#ifndef GUARD_DEX_JIT_FUNCTION_C
#define GUARD_DEX_JIT_FUNCTION_C 1

#include "libjit.h"
#include <deemon/arg.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/callable.h>
#include <deemon/string.h>
#include <deemon/thread.h>

DECL_BEGIN

typedef JITFunctionObject JITFunction;


INTERN DREF DeeObject *DCALL
JITFunction_New(/*utf-8*/char const *name_start,
                /*utf-8*/char const *name_end,
                /*utf-8*/char const *params_start,
                /*utf-8*/char const *params_end,
                /*utf-8*/char const *__restrict source_start,
                /*utf-8*/char const *__restrict source_end,
                JITObjectTable *__restrict parent_object_table,
                DeeObject *__restrict source,
                DeeModuleObject *impbase,
                DeeObject *globals,
                uint16_t flags) {
 DREF JITFunction *result;
 result = DeeObject_MALLOC(JITFunction);
 if unlikely(!result) goto done;
 result->jf_source_start = source_start;
 result->jf_source_end   = source_end;
 result->jf_source       = source;
 result->jf_impbase      = impbase;
 result->jc_globals      = globals;
 JITObjectTable_Init(&result->jf_args);
 JITObjectTable_Init(&result->jf_refs);
 result->jf_args.ot_prev.otp_ind = 2;
 result->jf_args.ot_prev.otp_tab = &result->jf_refs;
 result->jf_refs.ot_prev.otp_ind = 0;
 result->jf_refs.ot_prev.otp_tab = NULL;
 result->jf_argv     = NULL;
 result->jf_selfarg  = (size_t)-1;
 result->jf_varargs  = (size_t)-1;
 result->jf_kwargs   = (size_t)-1;
 result->jf_argc_min = 0;
 result->jf_argc_max = 0;
 result->jf_flags    = flags;
 Dee_Incref(source);
 Dee_XIncref(impbase);
 Dee_XIncref(globals);
 DeeObject_Init(result,&JITFunction_Type);

 /* TODO: Analyze & parse the parameter list. */
 (void)params_start;
 (void)params_end;

 if (name_start < name_end) {
  struct jit_object_entry *ent;
  size_t len = (size_t)(name_end - name_start);
  ent = JITObjectTable_Create(&result->jf_args,
                              name_start,
                              len,
                              hash_ptr(name_start,len));
  if unlikely(!ent)
     goto err_r;
  result->jf_selfarg = (size_t)(ent - result->jf_args.ot_list);
 }

 /* TODO: Scan the function body for keywords and search the chain of
  *       object tables provided by the parent for those keywords when
  *       interpreted as identifiers. - Any matching identifier should
  *       then be copied into our function's reference table. */
 (void)parent_object_table;


done:
 return (DREF DeeObject *)result;
err_r:
 Dee_DecrefDokill(result);
 return NULL;
}


PRIVATE void DCALL
jf_fini(JITFunction *__restrict self) {
 Dee_Decref(self->jf_source);
 Dee_XDecref(self->jf_impbase);
 Dee_XDecref(self->jc_globals);
 JITObjectTable_Fini(&self->jf_refs);
 Dee_Free(self->jf_argv);
}

PRIVATE void DCALL
jf_visit(JITFunction *__restrict self, dvisit_t proc, void *arg) {
 size_t i;
 Dee_Visit(self->jf_source);
 Dee_XVisit(self->jf_impbase);
 Dee_XVisit(self->jc_globals);
 if (self->jf_args.ot_list != jit_empty_object_list) {
  for (i = 0; i <= self->jf_args.ot_mask; ++i) {
   if (!ITER_ISOK(self->jf_args.ot_list[i].oe_namestr))
        continue;
   Dee_XVisit(self->jf_args.ot_list[i].oe_value);
  }
 }
 if (self->jf_refs.ot_list != jit_empty_object_list) {
  for (i = 0; i <= self->jf_refs.ot_mask; ++i) {
   if (!ITER_ISOK(self->jf_refs.ot_list[i].oe_namestr))
       continue;
   Dee_XVisit(self->jf_refs.ot_list[i].oe_value);
  }
 }
}


PRIVATE DREF DeeObject *DCALL
jf_str(JITFunction *__restrict self) {
 struct jit_object_entry *ent;
 if (self->jf_selfarg == (size_t)-1)
     return DeeString_New("<anonymous>");
 ent = &self->jf_args.ot_list[self->jf_selfarg];
 return DeeString_NewUtf8((char const *)ent->oe_namestr,
                           ent->oe_namelen,
                           STRING_ERROR_FSTRICT);
}


PRIVATE DREF DeeObject *DCALL
jf_repr(JITFunction *__restrict self) {
 size_t i;
 struct jit_object_entry *ent;
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if (self->jf_selfarg == (size_t)-1) {
  if unlikely(UNICODE_PRINTER_PRINT(&printer,"[](") < 0)
     goto err;
 } else {
  ent = &self->jf_args.ot_list[self->jf_selfarg];
  if unlikely(unicode_printer_printf(&printer,"function %$s(",
                                      ent->oe_namelen,
                                      ent->oe_namestr) < 0)
     goto err;
 }
 for (i = 0; i < self->jf_argc_max; ++i) {
  if (i != 0 && unicode_printer_putascii(&printer,','))
      goto err;
  ent = &self->jf_args.ot_list[self->jf_argv[i]];
  if unlikely(unicode_printer_print(&printer,
                                   (char const *)ent->oe_namestr,
                                    ent->oe_namelen) < 0)
     goto err;
  if (i > self->jf_argc_min) {
   if (!ent->oe_value) {
    if (unicode_printer_putascii(&printer,'?'))
        goto err;
   } else {
    if unlikely(unicode_printer_printf(&printer," = %r",ent->oe_value) < 0)
       goto err;
   }
  }
 }
 if (self->jf_varargs != (size_t)-1) {
  if (self->jf_argc_max != 0 && unicode_printer_putascii(&printer,','))
      goto err;
  ent = &self->jf_args.ot_list[self->jf_varargs];
  if unlikely(unicode_printer_print(&printer,
                                   (char const *)ent->oe_namestr,
                                    ent->oe_namelen) < 0)
     goto err;
  if unlikely(UNICODE_PRINTER_PRINT(&printer,"...") < 0)
     goto err;
 }
 if (self->jf_kwargs != (size_t)-1) {
  if ((self->jf_argc_max != 0 || self->jf_varargs != (size_t)-1) &&
       unicode_printer_putascii(&printer,','))
       goto err;
  ent = &self->jf_args.ot_list[self->jf_kwargs];
  if unlikely(UNICODE_PRINTER_PRINT(&printer,"**") < 0)
     goto err;
  if unlikely(unicode_printer_print(&printer,
                                   (char const *)ent->oe_namestr,
                                    ent->oe_namelen) < 0)
     goto err;
 }
 if (self->jf_flags & JIT_FUNCTION_FRETEXPR) {
  if unlikely(UNICODE_PRINTER_PRINT(&printer,") -> ") < 0)
     goto err;
  if unlikely(unicode_printer_print(&printer,self->jf_source_start,
                                   (size_t)(self->jf_source_end - self->jf_source_start)) < 0)
     goto err;
 } else {
  if unlikely(UNICODE_PRINTER_PRINT(&printer,") {\n\t") < 0)
     goto err;
  if unlikely(unicode_printer_print(&printer,self->jf_source_start,
                                   (size_t)(self->jf_source_end - self->jf_source_start)) < 0)
     goto err;
  if unlikely(UNICODE_PRINTER_PRINT(&printer,"\n}") < 0)
     goto err;
 }
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
jf_call_kw(JITFunction *__restrict self, size_t argc,
           DeeObject **__restrict argv, DeeObject *kw) {
 DREF DeeObject *result;
 JITLexer lexer;
 JITContext context;
 JITObjectTable base_locals;
 DeeThreadObject *ts = DeeThread_Self();
 size_t i;
 if (DeeThread_CheckInterrupt())
     goto err;
 ASSERT(self->jf_args.ot_prev.otp_ind >= 2);
 ASSERT(self->jf_args.ot_prev.otp_tab == &self->jf_refs);
 if (argc < self->jf_argc_min)
     goto err_argc;
 if (argc > self->jf_argc_max && self->jf_varargs == (size_t)-1)
     goto err_argc;
 memcpy(&base_locals,&self->jf_args,sizeof(JITObjectTable));
 base_locals.ot_list = (struct jit_object_entry *)Dee_Malloc((base_locals.ot_mask + 1) *
                                                              sizeof(struct jit_object_entry));
 if unlikely(!base_locals.ot_list) goto err;
 memcpy(base_locals.ot_list,self->jf_args.ot_list,
       (base_locals.ot_mask + 1) * sizeof(struct jit_object_entry));

 /* TODO: Load arguments! */
 (void)argc;
 (void)argv;
 (void)kw;

 /* Define the self-argument. */
 if (self->jf_selfarg != (size_t)-1) {
  ASSERT(base_locals.ot_list[self->jf_selfarg].oe_value == NULL);
  base_locals.ot_list[self->jf_selfarg].oe_value = (DREF DeeObject *)self;
 }

 /* Assign references to all objects from base-locals */
 for (i = 0; i <= base_locals.ot_mask; ++i) {
  if (!ITER_ISOK(base_locals.ot_list[i].oe_namestr))
       continue;
  Dee_XIncref(base_locals.ot_list[i].oe_value);
 }

 /* Initialize the lexer and context control structures. */
 lexer.jl_text    = self->jf_source;
 lexer.jl_context = &context;
 JITLValue_Init(&lexer.jl_lvalue);
 context.jc_impbase        = self->jf_impbase;
 context.jc_globals        = self->jc_globals;
 context.jc_retval         = JITCONTEXT_RETVAL_UNSET;
 context.jc_locals.otp_ind = 1;
 context.jc_locals.otp_tab = &base_locals;
 context.jc_except         = ts->t_exceptsz;
 context.jc_flags          = JITCONTEXT_FNORMAL;
 JITLexer_Start(&lexer,
               (unsigned char *)self->jf_source_start,
               (unsigned char *)self->jf_source_end);

 if (self->jf_flags & JIT_FUNCTION_FRETEXPR) {
  result = JITLexer_EvalExpression(&lexer,JITLEXER_EVAL_FNORMAL);
  if (result == JIT_LVALUE) {
   result = JITLValue_GetValue(&lexer.jl_lvalue,
                               &context);
   JITLValue_Fini(&lexer.jl_lvalue);
  }
  if likely(result) {
   ASSERT(context.jc_retval == JITCONTEXT_RETVAL_UNSET);
   if unlikely(lexer.jl_tok != TOK_EOF) {
    DeeError_Throwf(&DeeError_SyntaxError,
                    "Expected EOF but got `%$s'",
                   (size_t)(lexer.jl_end - lexer.jl_tokstart),
                    lexer.jl_tokstart);
    lexer.jl_errpos = lexer.jl_tokstart;
    Dee_Clear(result);
    goto handle_error;
   }
  } else load_return_value:
  if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
   if (JITCONTEXT_RETVAL_ISSET(context.jc_retval)) {
    result = context.jc_retval;
   } else {
    /* Exited code via unconventional means, such as `break' or `continue' */
    DeeError_Throwf(&DeeError_SyntaxError,
                    "Attempted to use `break' or `continue' used outside of a loop");
    lexer.jl_errpos = lexer.jl_tokstart;
    goto handle_error;
   }
  } else {
   if (!lexer.jl_errpos)
        lexer.jl_errpos = lexer.jl_tokstart;
handle_error:
   JITLValue_Fini(&lexer.jl_lvalue);
   result = NULL;
   /* TODO: Somehow remember that the error happened at `lexer.jl_errpos' */
   ;
  }
 } else if unlikely(lexer.jl_tok == TOK_EOF) {
  result = Dee_None;
  Dee_Incref(Dee_None);
 } else {
  do {
   result = JITLexer_EvalStatement(&lexer);
   if (ITER_ISOK(result)) { Dee_Decref_unlikely(result); continue; }
   if (result == JIT_LVALUE) {
    JITLValue_Fini(&lexer.jl_lvalue);
    JITLValue_Init(&lexer.jl_lvalue);
    continue;
   }
   /* Error, or return encountered. */
   goto load_return_value;
  } while (lexer.jl_tok != TOK_EOF);
 }
 if (ts->t_exceptsz > context.jc_except) {
  if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
   if (JITCONTEXT_RETVAL_ISSET(context.jc_retval))
       Dee_Decref(context.jc_retval);
   context.jc_retval = JITCONTEXT_RETVAL_UNSET;
  }
  Dee_XClear(result);
  while (ts->t_exceptsz > context.jc_except + 1) {
   DeeError_Print("Discarding secondary error\n",
                  ERROR_PRINT_DOHANDLE);
  }
 }
 ASSERT(context.jc_globals == self->jc_globals);
 JITObjectTable_Fini(&base_locals);
 return result;
/*
err_base_locals:
 JITObjectTable_Fini(&base_locals);*/
err:
 return NULL;
err_argc:
 if (self->jf_selfarg == (size_t)-1) {
  err_invalid_argc_len(NULL,
                       0,
                       argc,
                       self->jf_argc_min,
                       self->jf_argc_max);
 } else {
  struct jit_object_entry *ent;
  ent = &self->jf_args.ot_list[self->jf_selfarg];
  err_invalid_argc_len((char const *)ent->oe_namestr,
                        ent->oe_namelen,
                        argc,
                        self->jf_argc_min,
                        self->jf_argc_max);
 }
 goto err;
}



PRIVATE bool DCALL
jf_equal(JITFunction *__restrict a,
         JITFunction *__restrict b) {
 if (a == b) return true;
 /* TODO */
 return false;
}

PRIVATE dhash_t DCALL
jf_hash(JITFunction *__restrict self) {
 (void)self;
 /* TODO */
 return 0;
}

PRIVATE DREF DeeObject *DCALL
jf_eq(JITFunction *__restrict self,
      JITFunction *__restrict other) {
 if (DeeObject_AssertTypeExact(other,&JITFunction_Type))
     goto err;
 return_bool(jf_equal(self,other));
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
jf_ne(JITFunction *__restrict self,
      JITFunction *__restrict other) {
 if (DeeObject_AssertTypeExact(other,&JITFunction_Type))
     goto err;
 return_bool(!jf_equal(self,other));
err:
 return NULL;
}


PRIVATE struct type_cmp jf_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&jf_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&jf_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&jf_ne
};


PRIVATE struct type_getset jf_getsets[] = {
    /* TODO: __name__ */
    /* TODO: __default__ */
    /* TODO: __args__ */
    /* TODO: __refs__ */
    /* etc... */
    { NULL }
};

PRIVATE struct type_member jf_members[] = {
    TYPE_MEMBER_FIELD("__impbase__",STRUCT_OBJECT_OPT,offsetof(JITFunction,jf_impbase)),
    TYPE_MEMBER_FIELD_DOC("__globals__",STRUCT_OBJECT_OPT,offsetof(JITFunction,jc_globals),
                          "->?X2?S?T2?Dstring?O?N"),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject JITFunction_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_JitFunction",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeCallable_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&jf_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jf_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jf_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&jf_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&jf_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */jf_getsets,
    /* .tp_members       = */jf_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL,
    /* .tp_call_kw       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict,DeeObject *))&jf_call_kw,
};

DECL_END

#endif /* !GUARD_DEX_JIT_FUNCTION_C */
