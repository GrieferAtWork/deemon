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
#ifndef GUARD_DEEMON_COMPILER_SYMBOL_C
#define GUARD_DEEMON_COMPILER_SYMBOL_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/lexer.h>
#include <deemon/util/cache.h>
#include <deemon/code.h>
#include <deemon/arg.h>
#include <deemon/module.h>

#include <hybrid/minmax.h>
#include <stdint.h>

DECL_BEGIN

#ifndef asserte
#ifdef NDEBUG
#define asserte(x)        x
#else
#define asserte(x) ASSERT(x)
#endif
#endif

INTDEF struct module_symbol empty_module_buckets[];
DEFINE_STRUCT_CACHE_EX(sym,struct symbol,
                       MAX(sizeof(struct symbol),
                           sizeof(struct text_label)),
                       64)
#ifndef NDEBUG
#define sym_alloc()  sym_dbgalloc(__FILE__,__LINE__)
#endif

/* Re-use the symbol cache for labels. (As rare as they are, this is the best way to allocate them) */
#define lbl_alloc()  ((struct text_label *)sym_alloc())
#define lbl_free(p)      sym_free((struct symbol *)(p))


INTERN DREF DeeScopeObject *current_scope     = NULL;
INTERN DeeBaseScopeObject  *current_basescope = NULL;
INTERN DeeRootScopeObject  *current_rootscope = NULL;
INTERN DeeModuleObject     *current_module    = NULL;


INTERN char const symclass_names[0x1f + 1][8] = {
    /* [SYMBOL_TYPE_NONE  ] = */"none",
    /* [SYMBOL_TYPE_GLOBAL] = */"global",
    /* [SYMBOL_TYPE_EXTERN] = */"extern",
    /* [SYMBOL_TYPE_MODULE] = */"module",
    /* [SYMBOL_TYPE_MYMOD ] = */"mymod",
    /* [SYMBOL_TYPE_GETSET] = */"getset",
    /* [SYMBOL_TYPE_CATTR ] = */"cattr",
    /* [SYMBOL_TYPE_ALIAS ] = */"alias",
    /* [SYMBOL_TYPE_ARG   ] = */"arg",
    /* [SYMBOL_TYPE_LOCAL ] = */"local",
    /* [SYMBOL_TYPE_STACK ] = */"stack",
    /* [SYMBOL_TYPE_STATIC] = */"static",
    /* [SYMBOL_TYPE_EXCEPT] = */"except",
    /* [SYMBOL_TYPE_MYFUNC] = */"myfunc",
    /* [SYMBOL_TYPE_THIS  ] = */"this",
    /* [SYMBOL_TYPE_AMBIG ] = */"ambig",
    /* [SYMBOL_TYPE_FWD   ] = */"fwd",
    /* [SYMBOL_TYPE_CONST ] = */"const",
    "?","?","?","?","?","?","?",
    "?","?","?","?","?","?"
};


INTERN void DCALL
symbol_addambig(struct symbol *__restrict self,
                struct ast_loc *loc) {
 struct ast_loc *new_vec;
 ASSERT(self);
 ASSERT(self->s_type == SYMBOL_TYPE_AMBIG);
 new_vec = (struct ast_loc *)Dee_TryRealloc(self->s_ambig.a_declv,
                                           (self->s_ambig.a_declc + 1) *
                                            sizeof(struct ast_loc));
 if unlikely(!new_vec) return;
 self->s_ambig.a_declv = new_vec;
 new_vec += self->s_ambig.a_declc++;
 if (loc) {
  memcpy(new_vec,loc,sizeof(struct ast_loc));
 } else {
  loc_here(new_vec);
 }
 if (new_vec->l_file)
     TPPFile_Incref(new_vec->l_file);
}

INTERN void DCALL symbol_fini(struct symbol *__restrict self) {
 switch (self->s_type) {
 case SYMBOL_TYPE_EXTERN:
 case SYMBOL_TYPE_MODULE:
  Dee_Decref(self->s_module);
  break;
 case SYMBOL_TYPE_GLOBAL:
  Dee_XDecref(self->s_global.g_doc);
  break;
 case SYMBOL_TYPE_ALIAS:
  SYMBOL_SUB_NREAD(self->s_alias,self->s_nread);
  SYMBOL_SUB_NWRITE(self->s_alias,self->s_nwrite);
  SYMBOL_SUB_NBOUND(self->s_alias,self->s_nbound);
  break;
 case SYMBOL_TYPE_CATTR:
  SYMBOL_DEC_NREAD(self->s_attr.a_class);
  //if (self->s_attr.a_this) /* TODO: This fails when `self->s_attr.a_this' was already deleted. */
  //    SYMBOL_DEC_NREAD(self->s_attr.a_this);
  break;
 case SYMBOL_TYPE_GETSET:
  if (self->s_getset.gs_get)
      SYMBOL_DEC_NREAD(self->s_getset.gs_get);
  if (self->s_getset.gs_del)
      SYMBOL_DEC_NREAD(self->s_getset.gs_del);
  if (self->s_getset.gs_set)
      SYMBOL_DEC_NREAD(self->s_getset.gs_set);
  break;
 {
  size_t i;
 case SYMBOL_TYPE_AMBIG:
  if (self->s_ambig.a_decl2.l_file)
      TPPFile_Decref(self->s_ambig.a_decl2.l_file);
  for (i = 0; i < self->s_ambig.a_declc; ++i) {
   if (self->s_ambig.a_declv[i].l_file)
       TPPFile_Decref(self->s_ambig.a_declv[i].l_file);
  }
  Dee_Free(self->s_ambig.a_declv);
 } break;
 case SYMBOL_TYPE_CONST:
  Dee_Decref(self->s_const);
  break;

 default: break;
 }
}



/* -------- DeeScopeObject Implementation -------- */
PRIVATE void DCALL
delsym(struct symbol *__restrict self) {
 DeeCompiler_DelItem(self);
 if (self->s_decl.l_file)
     TPPFile_Decref(self->s_decl.l_file);
 symbol_fini(self);
 sym_free(self);
}
PRIVATE void DCALL
visitsym(struct symbol *__restrict self, dvisit_t proc, void *arg) {
 switch (self->s_type) {
 case SYMBOL_TYPE_EXTERN:
 case SYMBOL_TYPE_MODULE:
  Dee_Visit(self->s_module);
  break;
#if 0
 case SYMBOL_TYPE_GLOBAL:
  Dee_XVisit(self->s_global.g_doc);
  break;
#endif
 default: break;
 }
}

PRIVATE void DCALL
scope_fini(DeeScopeObject *__restrict self) {
 struct symbol **biter,**bend,*iter,*next;
 DeeCompiler_DelItem(self);
 bend = (biter = self->s_map)+self->s_mapa;
 for (; biter != bend; ++biter) {
  iter = *biter;
  while (iter) {
   next = iter->s_next;
   delsym(iter);
   iter = next;
  }
 }
 Dee_Free(self->s_map);
 iter = self->s_del;
 while (iter) {
  next = iter->s_next;
  delsym(iter);
  iter = next;
 }
 Dee_XDecref(self->s_prev);
}

PRIVATE void DCALL
scope_visit(DeeScopeObject *__restrict self, dvisit_t proc, void *arg) {
 struct symbol **biter,**bend,*iter;
 recursive_rwlock_read(&DeeCompiler_Lock);
 bend = (biter = self->s_map)+self->s_mapa;
 for (; biter != bend; ++biter) {
  iter = *biter;
  while (iter) {
   visitsym(iter,proc,arg);
   iter = iter->s_next;
  }
 }
 iter = self->s_del;
 while (iter) {
  visitsym(iter,proc,arg);
  iter = iter->s_next;
 }
 Dee_XVisit(self->s_prev);
 recursive_rwlock_endread(&DeeCompiler_Lock);
}

INTERN DeeTypeObject DeeScope_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"scope",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */NULL, /*&DeeObject_Type,*/
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeScopeObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&scope_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&scope_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
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









INTDEF void DCALL
cleanup_switch_cases(struct text_label *switch_cases,
                     struct text_label *switch_default);
INTDEF void DCALL
visit_switch_cases(struct text_label *switch_cases,
                   dvisit_t proc, void *arg);


/* -------- DeeBaseScopeObject Implementation -------- */
PRIVATE void DCALL
base_scope_fini(DeeBaseScopeObject *__restrict self) {
 ASSERT(self->bs_argc_max >= self->bs_argc_min);
 {
  struct text_label **biter,**bend,*iter,*next;
  bend = (biter = self->bs_lbl)+self->bs_lbla;
  for (; biter != bend; ++biter) {
   iter = *biter;
   while (iter) {
    next = iter->tl_next;
    DeeCompiler_DelItem(iter);
    lbl_free(iter);
    iter = next;
   }
  }
  Dee_Free(self->bs_lbl);
 }
 cleanup_switch_cases(self->bs_swcase,
                      self->bs_swdefl);
 {
  DREF DeeObject **iter,**end;
  end = (iter = self->bs_default)+(self->bs_argc_max-
                                   self->bs_argc_min);
  for (; iter != end; ++iter) Dee_Decref(*iter);
 }
 Dee_Free(self->bs_default);
 Dee_Free(self->bs_argv);
}

PRIVATE void DCALL
base_scope_visit(DeeBaseScopeObject *__restrict self,
                 dvisit_t proc, void *arg) {
 recursive_rwlock_read(&DeeCompiler_Lock);
 ASSERT(self->bs_argc_max >= self->bs_argc_min);
 visit_switch_cases(self->bs_swcase,proc,arg);
 {
  DREF DeeObject **iter,**end;
  end = (iter = self->bs_default)+(self->bs_argc_max-
                                   self->bs_argc_min);
  for (; iter != end; ++iter) Dee_Visit(*iter);
 }
 recursive_rwlock_endread(&DeeCompiler_Lock);
}

INTERN DeeTypeObject DeeBaseScope_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"base_scope",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeScope_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeBaseScopeObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&base_scope_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&base_scope_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
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










/* -------- DeeRootScopeObject Implementation -------- */
PRIVATE int DCALL
root_scope_ctor(DeeRootScopeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeModuleObject *module;
 if (DeeArg_Unpack(argc,argv,"o:root_scope",&module) ||
     DeeObject_AssertType((DeeObject *)module,&DeeModule_Type))
     return -1;
 memset((uint8_t *)self + offsetof(DeeScopeObject,s_prev),0,
         sizeof(DeeRootScopeObject)-offsetof(DeeScopeObject,s_prev));
 Dee_Incref(module);
 self->rs_scope.bs_scope.s_base = &self->rs_scope;
 self->rs_scope.bs_root         = self;
 self->rs_module                = module;
 self->rs_bucketv               = empty_module_buckets;
#if CODE_FNORMAL != 0
 self->rs_scope.bs_flags = CODE_FNORMAL;
#endif
#if MODULE_FNORMAL != 0
 self->rs_flags          = MODULE_FNORMAL;
#endif
 return 0;
}
PRIVATE void DCALL
root_scope_fini(DeeRootScopeObject *__restrict self) {
 Dee_Decref(self->rs_module);
 Dee_XDecref(self->rs_code);
 { DREF DeeModuleObject **iter,**end;
   end = (iter = self->rs_importv)+self->rs_importc;
   for (; iter != end; ++iter) Dee_Decref(*iter);
   Dee_Free(self->rs_importv);
 }
 if (self->rs_bucketv != empty_module_buckets) {
  struct module_symbol *iter,*end;
  end = (iter = self->rs_bucketv)+(self->rs_bucketm+1);
  for (; iter != end; ++iter) {
   Dee_XDecref(iter->ss_name);
   Dee_XDecref(iter->ss_doc);
  }
  Dee_Free(self->rs_bucketv);
 }
}

PRIVATE DREF DeeObject *DCALL
root_scope_str(DeeRootScopeObject *__restrict self) {
 return_reference_((DeeObject *)self->rs_module->mo_name);
}
PRIVATE void DCALL
root_scope_visit(DeeRootScopeObject *__restrict self,
                 dvisit_t proc, void *arg) {
 size_t i;
 recursive_rwlock_read(&DeeCompiler_Lock);
 Dee_Visit(self->rs_module);
 Dee_XVisit(self->rs_code);
 for (i = 0; i < self->rs_importc; ++i)
     Dee_Visit(self->rs_importv[i]);
 recursive_rwlock_endread(&DeeCompiler_Lock);
}

INTERN DeeTypeObject DeeRootScope_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"root_scope",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeBaseScope_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&root_scope_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeRootScopeObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&root_scope_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&root_scope_str,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&root_scope_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
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



INTERN int (DCALL scope_push)(void) {
 DREF DeeScopeObject *new_scope;
 new_scope = (DeeScopeObject *)DeeObject_Calloc(sizeof(DeeScopeObject));
 if unlikely(!new_scope) return -1;
 DeeObject_Init(new_scope,&DeeScope_Type);
 new_scope->s_prev = current_scope; /* Inherit reference */
 new_scope->s_base = current_basescope;
 current_scope     = new_scope; /* Inherit reference */
 return 0;
}
INTERN void DCALL scope_pop(void) {
 DeeScopeObject *pop_scope;
 ASSERT(current_scope);
 ASSERT(current_scope != (DeeScopeObject *)current_basescope);
 ASSERT(current_scope != (DeeScopeObject *)current_rootscope);
 ASSERT(current_scope->s_base == current_basescope);
 ASSERT_OBJECT_TYPE(current_scope->s_prev,&DeeScope_Type);
 pop_scope     = current_scope;
 current_scope = pop_scope->s_prev;
 Dee_Incref(current_scope); /* Keep a reference in `current_scope' */
 Dee_Decref(pop_scope); /* Drop the reference previously stored in `current_scope' */
}

INTERN DREF DeeBaseScopeObject *DCALL basescope_new(void) {
 DREF DeeBaseScopeObject *result;
 result = (DeeBaseScopeObject *)DeeObject_Calloc(sizeof(DeeBaseScopeObject));
 if unlikely(!result) return NULL;
 DeeObject_Init((DeeObject *)result,&DeeBaseScope_Type);
 ASSERT(current_rootscope == current_basescope->bs_root);
 result->bs_scope.s_base = result;
 result->bs_root         = current_rootscope;
 return result;
}
INTERN void DCALL
basescope_push_ob(DeeBaseScopeObject *__restrict scope) {
 ASSERT((current_scope != NULL) == (current_basescope != NULL));
 ASSERT(scope->bs_scope.s_prev == current_scope);
 ASSERT(scope->bs_prev == current_basescope);
 Dee_Incref((DeeObject *)scope);
 Dee_Decref(current_scope);
 current_scope = (DREF DeeScopeObject *)scope;
 current_basescope = scope;
}
INTERN int (DCALL basescope_push)(void) {
 DREF DeeBaseScopeObject *new_scope;
 new_scope = (DeeBaseScopeObject *)DeeObject_Calloc(sizeof(DeeBaseScopeObject));
 if unlikely(!new_scope) return -1;
 DeeObject_Init((DeeObject *)new_scope,&DeeBaseScope_Type);
 ASSERT(current_scope);
 ASSERT(current_rootscope == current_basescope->bs_root);
 new_scope->bs_scope.s_prev = current_scope; /* Inherit reference */
 new_scope->bs_scope.s_base = new_scope;
 new_scope->bs_prev         = current_basescope;
 new_scope->bs_root         = current_rootscope;
 current_basescope = new_scope;
 current_scope     = (DREF DeeScopeObject *)new_scope; /* Inherit reference */
 return 0;
}

INTERN void DCALL basescope_pop(void) {
 DeeBaseScopeObject *pop_scope;
 ASSERT(current_scope);
 ASSERT(current_basescope);
 if (current_scope != (DeeScopeObject *)current_basescope)
     return; /* Can happen during error-cleanup. */
 ASSERT(current_scope != (DeeScopeObject *)current_rootscope);
 ASSERT(current_scope->s_base == current_basescope);
 pop_scope         = current_basescope;
 current_scope     = pop_scope->bs_scope.s_prev;
 current_basescope = pop_scope->bs_prev;
 Dee_Incref(current_scope);
 Dee_Decref((DeeObject *)pop_scope); /* Drop the reference previously stored in `current_scope' */
}


INTERN int DCALL
copy_argument_symbols(DeeBaseScopeObject *__restrict other) {
 unsigned int i,count;
 ASSERT(current_basescope != other);
 ASSERT(!current_basescope->bs_argc);
 ASSERT(!current_basescope->bs_argc_min);
 ASSERT(!current_basescope->bs_argc_max);
 ASSERT(!current_basescope->bs_argc_opt);
 ASSERT(!current_basescope->bs_argv);
 ASSERT(!current_basescope->bs_default);
 ASSERT(!current_basescope->bs_varargs);
 ASSERT(!(current_basescope->bs_flags&CODE_FVARARGS));
 /* Copy basic flags and counters. */
 current_basescope->bs_flags   |= other->bs_flags&CODE_FVARARGS;
 current_basescope->bs_argc_opt = other->bs_argc_opt;
 current_basescope->bs_argc_max = other->bs_argc_max;
 current_basescope->bs_argc_min = other->bs_argc_min;
 current_basescope->bs_argc     = other->bs_argc;
 /* Copy default arguments. */
 count = other->bs_argc_max - other->bs_argc_min;
 if (count) {
  current_basescope->bs_default = (DREF DeeObject **)Dee_Malloc(count*sizeof(DREF DeeObject *));
  if unlikely(!current_basescope->bs_default) goto err;
  for (i = 0; i < count; ++i) {
   current_basescope->bs_default[i] = other->bs_default[i];
   Dee_Incref(other->bs_default[i]);
  }
 }
 count = other->bs_argc;
 if (count) {
  current_basescope->bs_argv = (struct symbol **)Dee_Malloc(count*sizeof(struct symbol *));
  if unlikely(!current_basescope->bs_argv) goto err;
  /* Copy the actual argument symbols. */
  for (i = 0; i < count; ++i) {
   struct symbol *sym,*other_sym;
   other_sym = other->bs_argv[i];
   ASSERT(other_sym);
   sym = other_sym->s_name == &TPPKeyword_Empty
       ? new_unnamed_symbol()
       : new_local_symbol(other_sym->s_name,&other_sym->s_decl);
   if unlikely(!sym) goto err;
   SYMBOL_TYPE(sym) = SYMBOL_TYPE_ARG;
   ASSERT(other_sym->s_symid == (uint16_t)i);
   sym->s_flag  = SYMBOL_FALLOC;
   sym->s_symid = (uint16_t)i;
   /* Save the symbol in our vector. */
   current_basescope->bs_argv[i] = sym;
  }
 }
 if (other->bs_varargs) {
  ASSERT(current_basescope->bs_argv != NULL);
  ASSERT(current_basescope->bs_argc != 0);
  current_basescope->bs_varargs = current_basescope->bs_argv[current_basescope->bs_argc-1];
 }
 return 0;
err:
 count = other->bs_argc_max - other->bs_argc_min;
 for (i = 0; i < count; ++i)
      Dee_Decref(current_basescope->bs_default[i]);
 Dee_Free(current_basescope->bs_default);
 current_basescope->bs_argc     = 0;
 current_basescope->bs_argc_min = 0;
 current_basescope->bs_argc_max = 0;
 current_basescope->bs_flags   &= ~CODE_FVARARGS;
 return -1;
}


PRIVATE int DCALL
link_forward_symbol(struct symbol *__restrict self) {
 DeeScopeObject *iter;
 if (SYMBOL_TYPE(self) != SYMBOL_TYPE_FWD)
     return 0;
 iter = current_scope->s_prev;
 for (; iter; iter = iter->s_prev) {
  struct symbol *outer_match;
  outer_match = scope_lookup(iter,self->s_name);
  if (!outer_match) continue;
  /* Setup the symbol as an alias for another, outer symbol. */
  ASSERT(!(self->s_flag & (SYMBOL_FALLOCREF|SYMBOL_FALLOC)));
  self->s_type  = SYMBOL_TYPE_ALIAS;
  self->s_alias = outer_match;
  SYMBOL_ADD_NREAD(outer_match,self->s_nread);
  SYMBOL_ADD_NWRITE(outer_match,self->s_nwrite);
  SYMBOL_ADD_NBOUND(outer_match,self->s_nbound);
  return 0;
 }
 if (WARNAT(&self->s_decl,W_UNKNOWN_VARIABLE,SYMBOL_NAME(self)))
     goto err;
 self->s_type = SYMBOL_TYPE_NONE; /* Prevent the assembler from crashing later... */
 return 0;
err:
 return -1;
}

INTERN int DCALL link_forward_symbols(void) {
 struct symbol **bucket_iter,**bucket_end,*iter;
 ASSERT(current_scope->s_flags & SCOPE_FCLASS);
 bucket_end = (bucket_iter = current_scope->s_map) + current_scope->s_mapa;
 for (; bucket_iter < bucket_end; ++bucket_iter) {
  iter = *bucket_iter;
  for (; iter; iter = iter->s_next)
     if unlikely(link_forward_symbol(iter)) goto err;
 }
 /* Must also perform final linking on symbols that have been deleted. */
 iter = current_scope->s_del;
 for (; iter; iter = iter->s_next)
    if unlikely(link_forward_symbol(iter)) goto err;
 return 0;
err:
 return -1;
}



INTERN int DCALL
rehash_scope(DeeScopeObject *__restrict iter) {
 struct symbol **new_map,**biter,**bend;
 struct symbol *sym_iter,*s_next,**bucket;
 size_t old_size = iter->s_mapa;
 size_t new_size = old_size;
 if (!new_size) new_size = 1;
 new_size *= 2;
rehash_realloc:
 new_map = (struct symbol **)Dee_TryCalloc(new_size*sizeof(struct symbol *));
 if unlikely(!new_map) {
  if (new_size != 1) { new_size = 1; goto rehash_realloc; }
  if (old_size) {
   if (Dee_TryCollectMemory(sizeof(struct symbol *)))
       goto rehash_realloc;
   return 0;
  }
  if (Dee_CollectMemory(sizeof(struct symbol *)))
      goto rehash_realloc;
  return -1;
 }
 /* Rehash all symbols. */
 bend = (biter = iter->s_map)+old_size;
 for (; biter != bend; ++biter) {
  sym_iter = *biter;
  while (sym_iter) {
   s_next = sym_iter->s_next;
   bucket = &new_map[sym_iter->s_name->k_id % new_size];
   sym_iter->s_next = *bucket;
   *bucket = sym_iter;
   sym_iter = s_next;
  }
 }
 Dee_Free(iter->s_map);
 iter->s_map  = new_map;
 iter->s_mapa = new_size;
 return 0;
}

INTERN bool DCALL
is_reserved_symbol_name(struct TPPKeyword *__restrict name) {
 /* Quick check: any keywords not registered as builtin are allowed. */
 if (TPP_ISUSERKEYWORD(name->k_id))
     return false;
 /* White-list of non-reserved builtin keywords. */
 switch (name->k_id) {

 case KWD_ifdef:
 case KWD_ifndef:
 case KWD_endif:
 case KWD_undef:
 case KWD_include:
 case KWD_include_next:
 case KWD_line:
 case KWD_error:
 case KWD_warning:
#if !defined(TPP_CONFIG_EXTENSION_IDENT_SCCS) || TPP_CONFIG_EXTENSION_IDENT_SCCS
 case KWD_ident:
 case KWD_sccs:
#endif
#if !defined(TPP_CONFIG_EXTENSION_ASSERTIONS) || TPP_CONFIG_EXTENSION_ASSERTIONS
 case KWD_unassert:
#endif
 case KWD_pragma:
#if !TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS
#if !defined(TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA) || TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA
 case KWD_tpp_dollar_is_alpha:
#endif
#if !defined(TPP_CONFIG_EXTENSION_VA_ARGS) || TPP_CONFIG_EXTENSION_VA_ARGS
 case KWD_tpp_va_args:
#endif
#if !defined(TPP_CONFIG_EXTENSION_GCC_VA_ARGS) || TPP_CONFIG_EXTENSION_GCC_VA_ARGS
 case KWD_tpp_named_va_args:
#endif
#if !defined(TPP_CONFIG_EXTENSION_VA_COMMA) || TPP_CONFIG_EXTENSION_VA_COMMA
 case KWD_tpp_va_comma:
#endif
 case KWD_tpp_reemit_unknown_pragmas:
#if !defined(TPP_CONFIG_EXTENSION_MSVC_FIXED_INT) || TPP_CONFIG_EXTENSION_MSVC_FIXED_INT
 case KWD_tpp_msvc_integer_suffix:
#endif
#if !defined(TPP_CONFIG_EXTENSION_HASH_AT) || TPP_CONFIG_EXTENSION_HASH_AT
 case KWD_tpp_charize_operator:
#endif
#if !defined(TPP_CONFIG_FEATURE_TRIGRAPHS) || TPP_CONFIG_FEATURE_TRIGRAPHS
 case KWD_tpp_trigraphs:
#endif
#if !defined(TPP_CONFIG_FEATURE_DIGRAPHS) || TPP_CONFIG_FEATURE_DIGRAPHS
 case KWD_tpp_digraphs:
#endif
 case KWD_tpp_pragma_push_macro:
 case KWD_tpp_pragma_pop_macro:
 case KWD_tpp_pragma_region:
 case KWD_tpp_pragma_endregion:
 case KWD_tpp_pragma_warning:
 case KWD_tpp_pragma_message:
 case KWD_tpp_pragma_error:
 case KWD_tpp_pragma_once:
 case KWD_tpp_pragma_tpp_exec:
 case KWD_tpp_pragma_deprecated:
 case KWD_tpp_pragma_tpp_set_keyword_flags:
#if !defined(TPP_CONFIG_EXTENSION_INCLUDE_NEXT) || TPP_CONFIG_EXTENSION_INCLUDE_NEXT
 case KWD_tpp_directive_include_next:
#endif
#if !defined(TPP_CONFIG_EXTENSION_IMPORT) || TPP_CONFIG_EXTENSION_IMPORT
 case KWD_tpp_directive_import:
#endif
#if !defined(TPP_CONFIG_EXTENSION_WARNING) || TPP_CONFIG_EXTENSION_WARNING
 case KWD_tpp_directive_warning:
#endif
#if !defined(TPP_CONFIG_EXTENSION_LXOR) || TPP_CONFIG_EXTENSION_LXOR
 case KWD_tpp_lxor:
#endif
 case KWD_tpp_token_tilde_tilde:
 case KWD_tpp_token_pow:
 case KWD_tpp_token_lxor:
 case KWD_tpp_token_arrow:
 case KWD_tpp_token_collon_assign:
 case KWD_tpp_token_collon_collon:
#if !defined(TPP_CONFIG_EXTENSION_ALTMAC) || TPP_CONFIG_EXTENSION_ALTMAC
 case KWD_tpp_macro_calling_conventions:
#endif
 case KWD_tpp_strict_whitespace:
 case KWD_tpp_strict_integer_overflow:
 case KWD_tpp_support_ansi_characters:
 case KWD_tpp_emit_lf_after_directive:
#if !defined(TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR) || TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR
 case KWD_tpp_if_cond_expression:
#endif
 case KWD_tpp_debug:
#endif /* !TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS */

 case KWD_this:
 case KWD_doc:
 case KWD_auto:
  return false;

 default: break;
 }

 /* Default case: the builtin keyword is reserved. */
 return true;
}


INTERN struct symbol *DCALL
lookup_symbol(unsigned int mode, struct TPPKeyword *__restrict name,
              struct ast_loc *warn_loc) {
 struct symbol *result,**bucket;
 DeeScopeObject *iter = current_scope;
 ASSERT(iter);
 /* Warn if a reserved name is used for a symbol. */
 if (is_reserved_symbol_name(name) &&
     WARNAT(warn_loc,W_RESERVED_SYMBOL_NAME,name))
     goto err;
 if ((mode&LOOKUP_SYM_VMASK) == LOOKUP_SYM_VLOCAL) {
  /* Only lookup variables in the current scope. */
seach_single:
  ASSERT(iter->s_mapc <= iter->s_mapa);
  result = NULL;
  if (iter->s_mapa) {
   result = iter->s_map[name->k_id % iter->s_mapa];
   while (result && result->s_name != name)
          result = result->s_next;
   /* Simple case: If the variable was found, return it. */
   if (result) {
    if (mode & LOOKUP_SYM_STACK) {
     if (result->s_type != SYMBOL_TYPE_STACK &&
         WARNAT(warn_loc,W_EXPECTED_STACK_VARIABLE,result))
         goto err;
    }
    if (mode & LOOKUP_SYM_STATIC) {
     if (result->s_type != SYMBOL_TYPE_STATIC &&
         WARNAT(warn_loc,W_EXPECTED_STATIC_VARIABLE,result))
         goto err;
    }
    if ((mode&LOOKUP_SYM_ALLOWDECL) && SYMBOL_IS_WEAK(result)) {
     /* Re-declare this symbol. */
     SYMBOL_CLEAR_WEAK(result);
     if ((mode&LOOKUP_SYM_VGLOBAL) ||
        ((mode&LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT &&
          /* Default variable lookup in the root scope creates global variables. */
          current_scope == (DeeScopeObject *)current_rootscope)) {
      result->s_type = SYMBOL_TYPE_GLOBAL;
     } else if (mode & LOOKUP_SYM_STACK) {
      result->s_type = SYMBOL_TYPE_STACK;
     } else if (mode & LOOKUP_SYM_STATIC) {
      result->s_type = SYMBOL_TYPE_STATIC;
     } else {
      result->s_type = SYMBOL_TYPE_LOCAL;
     }
     return result;
    }
    SYMBOL_MARK_USED(result);
    return result;
   }
  }
  goto create_variable;
 } else if ((mode&LOOKUP_SYM_VMASK) == LOOKUP_SYM_VGLOBAL) {
  /* Only lookup variables in the root scope. */
  iter = (DeeScopeObject *)current_rootscope;
  goto seach_single;
 }
 do {
  /* Look through all scopes. */
  ASSERT(iter->s_mapc <= iter->s_mapa);
  if (iter->s_mapa) {
   result = iter->s_map[name->k_id % iter->s_mapa];
   while (result && result->s_name != name)
          result = result->s_next;
   if (result) {
    SYMBOL_MARK_USED(result);
    return result;
   }
  }
  if (iter->s_flags & SCOPE_FCLASS) {
   /* Reached a class scope.
    * In order to allow for forward symbol declarations in class declarations,
    * we must remember that this is where the symbol was first used, and perform
    * the final linking when the class has been fully declared, at which point
    * the symbol has either been re-declared as a member of the class, or will
    * be linked to other declarations that may be visible outside of the class. */
   if unlikely((result = sym_alloc()) == NULL)
      goto err;
   memset(result,0,sizeof(*result));
   result->s_name  = name;
   result->s_type  = SYMBOL_TYPE_FWD;
   goto add_result_to_iter;
  }
 } while ((iter = iter->s_prev) != NULL);
create_variable:
 if (!(mode&LOOKUP_SYM_ALLOWDECL) &&
       WARNAT(warn_loc,W_UNKNOWN_VARIABLE,name->k_name))
       goto err;
 if ((mode&LOOKUP_SYM_VGLOBAL) &&
      current_scope != (DeeScopeObject *)current_rootscope &&
      WARNAT(warn_loc,W_DECLARING_GLOBAL_IN_NONROOT,name))
      goto err;
 /* Warn if a new variable is declared implicitly outside the global scope. */
 if (!(mode & LOOKUP_SYM_VMASK) &&
       current_scope != (DeeScopeObject *)current_rootscope &&
       WARNAT(warn_loc,W_DECLARING_IMPLICIT_VARIABLE,name))
       goto err;

 /* Create a new symbol. */
 if unlikely((result = sym_alloc()) == NULL)
    goto err;
 memset(result,0,sizeof(*result));
 result->s_name  = name;
 result->s_scope = current_scope;
 result->s_type  = SYMBOL_TYPE_FWD;
 if ((mode&LOOKUP_SYM_VGLOBAL) ||
    ((mode&LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT &&
      /* Default variable lookup in the root scope creates global variables. */
      current_scope == (DeeScopeObject *)current_rootscope)) {
  result->s_type = SYMBOL_TYPE_GLOBAL;
  iter = (DeeScopeObject *)current_rootscope;
 } else {
  iter = current_scope;
  if (mode&LOOKUP_SYM_STACK) {
   result->s_type = SYMBOL_TYPE_STACK;
  } else if (mode&LOOKUP_SYM_STATIC) {
   result->s_type = SYMBOL_TYPE_STATIC;
  } else {
   result->s_type = SYMBOL_TYPE_LOCAL;
  }
 }
add_result_to_iter:
 if (++iter->s_mapc > iter->s_mapa) {
  /* Must rehash this scope. */
  if unlikely(rehash_scope(iter)) goto err_r;
 }
 /* Insert the new symbol. */
 ASSERT(iter->s_mapa != 0);
 bucket = &iter->s_map[name->k_id % iter->s_mapa];
 result->s_next = *bucket;
 *bucket = result;
 result->s_scope = iter;
 if (warn_loc)
  memcpy(&result->s_decl,
          warn_loc,
          sizeof(struct ast_loc));
 else {
  loc_here(&result->s_decl);
 }
 if (result->s_decl.l_file)
     TPPFile_Incref(result->s_decl.l_file);
 return result;
err_r:
 --iter->s_mapc;
 sym_free(result);
err:
 return NULL;
}

INTERN struct symbol *DCALL
lookup_nth(unsigned int nth, struct TPPKeyword *__restrict name) {
 DeeScopeObject *iter;
 /* Make sure to return `NULL' when `nth' was zero. */
 if unlikely(!nth--) return NULL;
 iter = current_scope;
 for (; iter; iter = iter->s_prev) {
  struct symbol *result;
  if (!iter->s_mapa) continue;
  result = iter->s_map[name->k_id % iter->s_mapa];
  while (result) {
   if (result->s_name == name) {
    /* Return this instance if it is the one that was requested. */
    if (!nth--) {
     SYMBOL_MARK_USED(result);
     return result;
    }
    break;
   }
   result = result->s_next;
  }
 }
 return NULL;
}


INTERN struct symbol *DCALL
new_local_symbol(struct TPPKeyword *__restrict name, struct ast_loc *loc) {
 struct symbol *result,**bucket;
 ASSERT(name);
 if unlikely((result = sym_alloc()) == NULL) return NULL;
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct symbol));
#endif
 result->s_name = name;
 if (++current_scope->s_mapc > current_scope->s_mapa) {
  if unlikely(rehash_scope(current_scope))
     goto err_r;
 }
 ASSERT(current_scope->s_mapa != 0);
 bucket            = &current_scope->s_map[name->k_id % current_scope->s_mapa];
 result->s_next  = *bucket;
 *bucket           = result;
 result->s_flag    = SYMBOL_FNORMAL;
 result->s_nread   = 0;
 result->s_nwrite  = 0;
 result->s_nbound  = 0;
 result->s_scope   = current_scope;
 if (loc) {
  memcpy(&result->s_decl,loc,sizeof(struct ast_loc));
 } else {
  loc_here(&result->s_decl);
 }
 if (result->s_decl.l_file)
     TPPFile_Incref(result->s_decl.l_file);
 return result;
err_r:
 --current_scope->s_mapc;
 sym_free(result);
 return NULL;
}
INTERN struct symbol *DCALL new_unnamed_symbol(void) {
 struct symbol *result;
 if unlikely((result = sym_alloc()) == NULL) return NULL;
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct symbol));
#endif
 result->s_name        = &TPPKeyword_Empty;
 result->s_next        = current_scope->s_del;
 current_scope->s_del  = result;
 result->s_flag        = SYMBOL_FNORMAL;
 result->s_nread       = 0;
 result->s_nwrite      = 0;
 result->s_nbound      = 0;
 result->s_scope       = current_scope;
 result->s_decl.l_file = NULL;
 return result;
}
INTERN struct symbol *DCALL
new_unnamed_symbol_in_scope(DeeScopeObject *__restrict scope){
 struct symbol *result;
 if unlikely((result = sym_alloc()) == NULL) return NULL;
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct symbol));
#endif
 result->s_name        = &TPPKeyword_Empty;
 result->s_next        = scope->s_del;
 scope->s_del          = result;
 result->s_flag        = SYMBOL_FNORMAL;
 result->s_nread       = 0;
 result->s_nwrite      = 0;
 result->s_nbound      = 0;
 result->s_scope       = scope;
 result->s_decl.l_file = NULL;
 return result;
}
INTERN struct symbol *DCALL
new_local_symbol_in_scope(DeeScopeObject *__restrict scope,
                          struct TPPKeyword *__restrict name,
                          struct ast_loc *loc) {
 struct symbol *result,**bucket;
 ASSERT(name);
 if unlikely((result = sym_alloc()) == NULL) return NULL;
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct symbol));
#endif
 result->s_name = name;
 if (++scope->s_mapc > scope->s_mapa) {
  if unlikely(rehash_scope(scope))
     goto err_r;
 }
 ASSERT(scope->s_mapa != 0);
 bucket            = &scope->s_map[name->k_id % scope->s_mapa];
 result->s_next  = *bucket;
 *bucket           = result;
 result->s_flag    = SYMBOL_FNORMAL;
 result->s_nread   = 0;
 result->s_nwrite  = 0;
 result->s_nbound  = 0;
 result->s_scope   = scope;
 if (loc) {
  memcpy(&result->s_decl,loc,sizeof(struct ast_loc));
 } else {
  loc_here(&result->s_decl);
 }
 if (result->s_decl.l_file)
     TPPFile_Incref(result->s_decl.l_file);
 return result;
err_r:
 --scope->s_mapc;
 sym_free(result);
 return NULL;
}
INTERN struct symbol *DCALL
get_local_symbol_in_scope(DeeScopeObject *__restrict scope,
                          struct TPPKeyword *__restrict name) {
 struct symbol *bucket;
 if (!scope->s_mapc) return false;
 ASSERT(scope->s_mapa != 0);
 bucket = scope->s_map[name->k_id % scope->s_mapa];
 while (bucket && bucket->s_name != name) bucket = bucket->s_next;
 return bucket;
}

INTERN struct symbol *DCALL
get_local_symbol(struct TPPKeyword *__restrict name) {
 struct symbol *bucket;
 if (!current_scope->s_mapc) return false;
 ASSERT(current_scope->s_mapa != 0);
 bucket = current_scope->s_map[name->k_id % current_scope->s_mapa];
 while (bucket && bucket->s_name != name) bucket = bucket->s_next;
 return bucket;
}
INTERN void DCALL
del_local_symbol(struct symbol *__restrict sym) {
 struct symbol **pbucket,*bucket;
 ASSERT(sym);
 ASSERT(sym->s_name != &TPPKeyword_Empty);
 ASSERT(sym->s_scope->s_mapa != 0);
 pbucket = &sym->s_scope->s_map[sym->s_name->k_id % sym->s_scope->s_mapa];
 while ((bucket = *pbucket,bucket && bucket != sym))
         pbucket = &bucket->s_next;
 if (!bucket) return; /* Shouldn't happen, but ignore (could happen if the symbol is deleted twice) */
 /* Unlink the symbol from the bucket list. */
 *pbucket = sym->s_next;
 /* Add the symbol to the chain of deleted (anonymous) symbols. */
 sym->s_next = sym->s_scope->s_del;
 sym->s_scope->s_del = sym;
}

INTERN struct symbol *DCALL
scope_lookup(DeeScopeObject *__restrict scope,
             struct TPPKeyword *__restrict name) {
 struct symbol *result = NULL;
 if (!scope->s_mapa) goto done;
 result = scope->s_map[name->k_id % scope->s_mapa];
 while (result && result->s_name != name) result = result->s_next;
done:
 return result;
}
INTERN struct symbol *DCALL
scope_lookup_str(DeeScopeObject *__restrict scope,
                 char const *__restrict name,
                 size_t name_length) {
 struct symbol *result = NULL;
 struct TPPKeyword *keyword;
 if (!scope->s_mapa) goto done;
 keyword = TPPLexer_LookupKeyword(name,name_length,0);
 if (!keyword) goto done;
 result = scope->s_map[keyword->k_id % scope->s_mapa];
 while (result && result->s_name != keyword) result = result->s_next;
done:
 return result;
}

PRIVATE int DCALL
rehash_labels(void) {
 struct text_label **new_map,**biter,**bend;
 struct text_label *lbl_iter,*s_next,**bucket;
 size_t old_size = current_basescope->bs_lbla;
 size_t new_size = old_size;
 if (!new_size) new_size = 1;
 new_size *= 2;
rehash_realloc:
 new_map = (struct text_label **)Dee_TryCalloc(new_size*sizeof(struct text_label *));
 if unlikely(!new_map) {
  if (new_size != 1) { new_size = 1; goto rehash_realloc; }
  if (Dee_CollectMemory(sizeof(struct text_label *))) goto rehash_realloc;
  return -1;
 }
 /* Rehash all text_labels. */
 bend = (biter = current_basescope->bs_lbl)+old_size;
 for (; biter != bend; ++biter) {
  lbl_iter = *biter;
  while (lbl_iter) {
   s_next = lbl_iter->tl_next;
   bucket = &new_map[lbl_iter->tl_name->k_id % new_size];
   lbl_iter->tl_next = *bucket;
   *bucket = lbl_iter;
   lbl_iter = s_next;
  }
 }
 Dee_Free(current_basescope->bs_lbl);
 current_basescope->bs_lbl  = new_map;
 current_basescope->bs_lbla = new_size;
 return 0;
}

INTERN struct text_label *DCALL
lookup_label(struct TPPKeyword *__restrict name) {
 struct text_label *result,**presult;
 if likely(current_basescope->bs_lbla) {
  result = current_basescope->bs_lbl[name->k_id % current_basescope->bs_lbla];
  while (result) {
   if (result->tl_name == name) return result;
   result = result->tl_next;
  }
 }
 if (current_basescope->bs_lblc >= current_basescope->bs_lbla &&
     rehash_labels()) return NULL;
 ASSERT(current_basescope->bs_lbla);
 presult = &current_basescope->bs_lbl[name->k_id % current_basescope->bs_lbla];
 result = lbl_alloc();
 if unlikely(!result) return NULL;
 result->tl_next = *presult;
 result->tl_name = name;
 result->tl_asym = NULL;
 result->tl_goto = 0;
 *presult = result;
 return result;
}
INTERN struct text_label *DCALL
new_case_label(struct ast *__restrict expr) {
 struct text_label *result;
 ASSERTF(current_basescope->bs_cflags&BASESCOPE_FSWITCH,
         "Not inside a switch statement");
 ASSERT_AST(expr);
 result = lbl_alloc();
 if unlikely(!result) goto done;
 /* Initialize the new label. */
 ast_incref(expr);
 result->tl_expr = expr;
 result->tl_asym = NULL;
 result->tl_goto = 0;
 /* Add the case to the currently active linked list of them. */
 result->tl_next = current_basescope->bs_swcase;
 current_basescope->bs_swcase = result;
done:
 return result;
}
INTERN struct text_label *DCALL
new_default_label(void) {
 struct text_label *result;
 ASSERTF(current_basescope->bs_cflags&BASESCOPE_FSWITCH,
         "Not inside a switch statement");
 result = current_basescope->bs_swdefl;
 if unlikely(result) goto done; /* Unlikely, considering actually valid use cases. */
 result = lbl_alloc();
 if unlikely(!result) goto done;
 /* Initialize the new label. */
 result->tl_next = NULL;
 result->tl_expr = NULL;
 result->tl_asym = NULL;
 result->tl_goto = 0;
 /* Set the new label as the default label. */
 current_basescope->bs_swdefl = result;
done:
 return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_SYMBOL_H */
