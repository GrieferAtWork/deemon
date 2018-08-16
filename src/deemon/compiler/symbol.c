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


#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
INTERN char const symclass_names[0x1f + 1][8] = {
    /* [SYMBOL_TYPE_NONE  ] = */"none",
    /* [SYMBOL_TYPE_GLOBAL] = */"global",
    /* [SYMBOL_TYPE_EXTERN] = */"extern",
    /* [SYMBOL_TYPE_MODULE] = */"module",
    /* [SYMBOL_TYPE_MYMOD ] = */"mymod",
    /* [SYMBOL_TYPE_GETSET] = */"getset",
    /* [SYMBOL_TYPE_CFIELD] = */"cfield",
    /* [SYMBOL_TYPE_IFIELD] = */"ifield",
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
    "?","?","?","?","?","?","?",
    "?","?","?","?","?","?","?"
};
#else
INTERN char const symclass_names[0xf + 1][14] = {
    /* [SYM_CLASS_EXTERN        & 0xf] = */"extern",
    /* [SYM_CLASS_VAR           & 0xf] = */"var",
    /* [SYM_CLASS_STACK         & 0xf] = */"stack",
    /* [SYM_CLASS_ARG           & 0xf] = */"arg",
    /* [SYM_CLASS_REF           & 0xf] = */"ref",
    /* [SYM_CLASS_MEMBER        & 0xf] = */"member",
    /* [SYM_CLASS_MODULE        & 0xf] = */"module",
    /* [SYM_CLASS_PROPERTY      & 0xf] = */"property",
    /* [SYM_CLASS_ALIAS         & 0xf] = */"alias",
    /* [SYM_CLASS_EXCEPT        & 0xf] = */"except",
    /* [SYM_CLASS_THIS_MODULE   & 0xf] = */"this_module",
    /* [SYM_CLASS_THIS_FUNCTION & 0xf] = */"this_function",
    /* [SYM_CLASS_THIS          & 0xf] = */"this",
    "?","?","?" /* Padding... */
};
#endif


INTERN void DCALL symbol_fini(struct symbol *__restrict self) {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
 switch (self->s_type) {
 case SYMBOL_TYPE_EXTERN:
 case SYMBOL_TYPE_MODULE:
  Dee_Decref(self->s_module);
  break;
 case SYMBOL_TYPE_GLOBAL:
  Dee_XDecref(self->s_global.g_doc);
  break;
 case SYMBOL_TYPE_CFIELD:
 case SYMBOL_TYPE_IFIELD:
  SYMBOL_DEC_NREAD(self->s_field.f_class);
  break;
 case SYMBOL_TYPE_GETSET:
  if (self->s_getset.gs_get)
      SYMBOL_DEC_NREAD(self->s_getset.gs_get);
  if (self->s_getset.gs_del)
      SYMBOL_DEC_NREAD(self->s_getset.gs_del);
  if (self->s_getset.gs_set)
      SYMBOL_DEC_NREAD(self->s_getset.gs_set);
  break;
 case SYMBOL_TYPE_AMBIG:
  Dee_Free(self->s_ambig.a_declv);
  break;

 default: break;
 }
#else
 switch (self->sym_class) {
 case SYM_CLASS_EXTERN:
 case SYM_CLASS_MODULE:
  Dee_Decref(self->sym_module.sym_module);
  break;
 case SYM_CLASS_VAR:
  Dee_XDecref(self->sym_var.sym_doc);
  break;
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
 case SYM_CLASS_REF:
  ASSERT(self->sym_ref.sym_ref);
  ASSERT(self->sym_ref.sym_ref->sym_read);
  --self->sym_ref.sym_ref->sym_read;
  break;
#endif
 default: break;
 }
#endif
}



/* -------- DeeScopeObject Implementation -------- */
PRIVATE void DCALL
delsym(struct symbol *__restrict self) {
 symbol_fini(self);
 sym_free(self);
}
PRIVATE void DCALL
visitsym(struct symbol *__restrict self, dvisit_t proc, void *arg) {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
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
#else
 switch (self->sym_class) {
 case SYM_CLASS_EXTERN:
 case SYM_CLASS_MODULE:
  Dee_Visit(self->sym_module.sym_module);
  break;
 case SYM_CLASS_VAR:
  /*Dee_XVisit(self->sym_var.sym_doc);*/
  break;
 default: break;
 }
#endif
}

#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
#define sym_next  s_next  /* TODO: Remove me */
#define sym_name  s_name  /* TODO: Remove me */
#define sym_scope s_scope /* TODO: Remove me */
#endif

PRIVATE void DCALL
scope_fini(DeeScopeObject *__restrict self) {
 struct symbol **biter,**bend,*iter,*next;
 bend = (biter = self->s_map)+self->s_mapa;
 for (; biter != bend; ++biter) {
  iter = *biter;
  while (iter) {
   next = iter->sym_next;
   delsym(iter);
   iter = next;
  }
 }
 Dee_Free(self->s_map);
 iter = self->s_del;
 while (iter) {
  next = iter->sym_next;
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
   iter = iter->sym_next;
  }
 }
 iter = self->s_del;
 while (iter) {
  visitsym(iter,proc,arg);
  iter = iter->sym_next;
 }
 Dee_XVisit(self->s_prev);
 recursive_rwlock_endread(&DeeCompiler_Lock);
}

PRIVATE struct type_member scope_members[] = {
    TYPE_MEMBER_FIELD("base",STRUCT_OBJECT,offsetof(DeeScopeObject,s_base)),
    TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeScope_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"scope",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
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
    /* .tp_members       = */scope_members,
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
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
 {
  struct symbol *sym_iter,*sym_next;
  sym_iter = self->bs_refs;
  while (sym_iter) {
   ASSERT(sym_iter->sym_class == SYM_CLASS_REF);
   sym_next = sym_iter->sym_ref.sym_rnext;
   delsym(sym_iter);
   sym_iter = sym_next;
  }
 }
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
 Dee_Free(self->bs_default);
 Dee_Free(self->bs_argv);
}

PRIVATE struct type_member base_scope_members[] = {
    TYPE_MEMBER_FIELD("root",STRUCT_OBJECT,offsetof(DeeBaseScopeObject,bs_root)),
    TYPE_MEMBER_END
};

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
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
 {
  struct symbol *sym_iter;
  sym_iter = self->bs_refs;
  while (sym_iter) {
   ASSERT(sym_iter->sym_class == SYM_CLASS_REF);
   visitsym(sym_iter,proc,arg);
   sym_iter = sym_iter->sym_ref.sym_rnext;
  }
 }
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
 recursive_rwlock_endread(&DeeCompiler_Lock);
}

PUBLIC DeeTypeObject DeeBaseScope_Type = {
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
    /* .tp_members       = */base_scope_members,
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

PRIVATE struct type_member root_scope_members[] = {
    TYPE_MEMBER_FIELD("module",STRUCT_OBJECT,offsetof(DeeRootScopeObject,rs_module)),
    TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeRootScope_Type = {
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
    /* .tp_members       = */root_scope_members,
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

#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
PRIVATE struct symbol *DCALL
basescope_addref(DeeBaseScopeObject *__restrict self,
                 struct symbol *__restrict sym) {
 struct symbol *refsym;
 DeeBaseScopeObject *symbol_base;
 ASSERT_OBJECT_TYPE((DeeObject *)self,&DeeBaseScope_Type);
 ASSERT(sym);
 ASSERT_OBJECT_TYPE(sym->sym_scope,&DeeScope_Type);
 ASSERT(SYM_MUST_REFERENCE(sym));
 ASSERT(sym->sym_scope->s_base != self);
 if (SYMBOL_TYPE(sym) == SYM_CLASS_MEMBER) {
  /* Special case: For class members, only reference the
   * associated class symbol, not Not the member itself. */
  struct symbol *class_sym,*new_sym;
  class_sym = SYMBOL_FIELD_CLASS(sym);
  if (!SYM_MUST_REFERENCE(class_sym))
       return sym;
  class_sym = basescope_addref(self,class_sym);
  if unlikely(!class_sym) goto err;
#if 0
  new_sym = self->bs_scope.s_del;
  /* Search for an existing reference to this symbol. */
  for (; new_sym; new_sym = new_sym->sym_next) {
   if (new_sym->sym_class == SYM_CLASS_MEMBER &&
       new_sym->sym_member.sym_class == class_sym &&
       new_sym->sym_member.sym_member == SYMBOL_FIELD_MEMBER(sym))
       return new_sym;
  }
#endif
  /* Create a new member symbol referencing this class. */
  new_sym = sym_alloc();
  if unlikely(!new_sym) goto err;
#ifndef NDEBUG
  memset(new_sym,0xcc,sizeof(struct symbol));
#endif
  new_sym->sym_name              = &TPPKeyword_Empty;
  new_sym->sym_next              = self->bs_scope.s_del;
  self->bs_scope.s_del           = new_sym;
  new_sym->sym_flag              = sym->sym_flag; /* Copy symbol table id. */
  new_sym->sym_read              = 0;
  new_sym->sym_write             = 0;
  new_sym->sym_scope             = current_scope;
  new_sym->sym_class             = SYM_CLASS_MEMBER;
  new_sym->sym_member.sym_class  = class_sym;
  new_sym->sym_member.sym_member = SYMBOL_FIELD_MEMBER(sym);
  /* If the symbol table is relocated, this pointer becomes invalid.
   * Therefor, we must keep track of all referencing
   * symbols, so we can update them when that happens. */
  new_sym->sym_member.sym_ref    = sym->sym_member.sym_ref;
  sym->sym_member.sym_ref        = new_sym;
  return new_sym;
 }
 symbol_base = sym->sym_scope->s_base;
 ASSERT_OBJECT_TYPE((DeeObject *)symbol_base,&DeeBaseScope_Type);
 ASSERT(symbol_base != self);
 if (symbol_base != self->bs_prev) {
  /* Recursively add symbols down the chain. */
  sym = basescope_addref(self->bs_prev,sym);
  if unlikely(!sym) goto err;
 }
 /* Add a reference to `symbol' */
 ASSERT(SYM_MUST_REFERENCE(sym));
 ASSERT_OBJECT_TYPE((DeeObject *)sym->sym_scope,&DeeScope_Type);
 ASSERT_OBJECT_TYPE((DeeObject *)sym->sym_scope->s_base,&DeeBaseScope_Type);
 ASSERT(sym->sym_scope->s_base == self->bs_prev);
 /* Search if there already is a reference for this symbol. */
 for (refsym = self->bs_refs;
      refsym; refsym = refsym->sym_ref.sym_rnext) {
  ASSERT(refsym->sym_class == SYM_CLASS_REF);
  if (refsym->sym_ref.sym_ref == sym)
      return refsym;
 }
 /* Create a new reference symbol. */
 refsym = sym_alloc();
 if unlikely(!refsym) goto err;
 refsym->sym_scope         = (DeeScopeObject *)self;
 refsym->sym_flag          = SYM_FNORMAL;
 refsym->sym_class         = SYM_CLASS_REF;
 refsym->sym_ref.sym_ref   = sym;
 refsym->sym_ref.sym_rnext = self->bs_refs;
 refsym->sym_read          = 0;
 refsym->sym_write         = 0;
 self->bs_refs             = refsym;
 ++sym->sym_read; /* The reference reads from the base symbol! */
 return refsym;
err:
 return NULL;
}
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */

#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
INTERN struct symbol *DCALL
symbol_reference(DeeBaseScopeObject *__restrict current_basescope,
                 struct symbol *__restrict sym) {
 ASSERT(sym);
 ASSERT(sym->sym_scope);
 ASSERT(sym->sym_scope->s_base);
 /* Simple case: Symbol from same base-scope. */
 if (sym->sym_scope->s_base == current_basescope ||
    !SYM_MUST_REFERENCE(sym))
     return sym;
 return basescope_addref(current_basescope,sym);
}
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */

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
   sym = other_sym->sym_name == &TPPKeyword_Empty
       ? new_unnamed_symbol() : new_local_symbol(other_sym->sym_name);
   if unlikely(!sym) goto err;
   SYMBOL_TYPE(sym) = SYM_CLASS_ARG;
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
   ASSERT(other_sym->s_symid == (uint16_t)i);
   sym->s_flag  = SYMBOL_FALLOC;
   sym->s_symid = (uint16_t)i;
#else
   SYMBOL_ARG_INDEX(sym) = other_sym->sym_arg.sym_index;
#endif
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


#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
PRIVATE int DCALL
link_forward_symbol(struct symbol *__restrict self) {
 DeeScopeObject *iter;
 if (SYMBOL_TYPE(self) != SYMBOL_TYPE_FWD)
     return 0;
 iter = current_scope->s_prev;
 for (; iter; iter = iter->s_prev) {
  struct symbol *outer_match;
  outer_match = scope_lookup(iter,self->sym_name);
  if (!outer_match) continue;
  /* Setup the symbol as an alias for another, outer symbol. */
  ASSERT(!(self->s_flag & (SYMBOL_FALLOCREF|SYMBOL_FALLOC)));
  self->s_type  = SYMBOL_TYPE_ALIAS;
  self->s_alias = outer_match;
  outer_match->s_nread  += self->s_nread;
  outer_match->s_nwrite += self->s_nwrite;
  outer_match->s_nbound += self->s_nbound;
#ifndef NDEBUG
  self->s_nread  = 0xcccc;
  self->s_nwrite = 0xcccc;
  self->s_nbound = 0xcccc;
#endif
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
#endif /* CONFIG_USE_NEW_SYMBOL_TYPE */



#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
INTERN struct symbol *DCALL
sym_realsym(struct symbol *__restrict self) {
 while ((ASSERT(self),self->sym_class == SYM_CLASS_REF))
         self = self->sym_ref.sym_ref;
 return self;
}
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */

INTERN int DCALL
rehash_scope(DeeScopeObject *__restrict iter) {
 struct symbol **new_map,**biter,**bend;
 struct symbol *sym_iter,*sym_next,**bucket;
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
   sym_next = sym_iter->sym_next;
   bucket = &new_map[sym_iter->sym_name->k_id % new_size];
   sym_iter->sym_next = *bucket;
   *bucket = sym_iter;
   sym_iter = sym_next;
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
 case KWD_once:
 case KWD_push_macro:
 case KWD_pop_macro:
 case KWD_region:
 case KWD_endregion:
 case KWD_message:
 case KWD_deprecated:
 case KWD_tpp_exec:
 case KWD_tpp_set_keyword_flags:
 case KWD_push:
 case KWD_pop:
 case KWD_disable:
 case KWD_enable:
 case KWD_suppress:
 case KWD_TPP:
 case KWD_include_path:
 case KWD_GCC:
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
   while (result && result->sym_name != name)
          result = result->sym_next;
   /* Simple case: If the variable was found, return it. */
   if (result) {
    if (mode & LOOKUP_SYM_STACK) {
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
     struct symbol *realsym = sym_realsym(result);
     if (realsym->sym_class != SYM_CLASS_STACK &&
         WARNAT(warn_loc,W_EXPECTED_STACK_VARIABLE,name))
         goto err;
#else /* !CONFIG_USE_NEW_SYMBOL_TYPE */
     if (result->s_type != SYM_CLASS_STACK &&
         WARNAT(warn_loc,W_EXPECTED_STACK_VARIABLE,name))
         goto err;
#endif /* CONFIG_USE_NEW_SYMBOL_TYPE */
    }
    if (mode & LOOKUP_SYM_STATIC) {
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
     struct symbol *realsym = sym_realsym(result);
     if (realsym->sym_class != SYM_CLASS_VAR &&
         realsym->sym_flag  != SYM_FVAR_STATIC &&
         WARNAT(warn_loc,W_EXPECTED_STATIC_VARIABLE,name))
         goto err;
#else /* !CONFIG_USE_NEW_SYMBOL_TYPE */
     /* TODO: Add a reference to the declaration of the selected variable in the message */
     if (result->s_type != SYMBOL_TYPE_STATIC &&
         WARNAT(warn_loc,W_EXPECTED_STATIC_VARIABLE,name))
         goto err;
#endif /* CONFIG_USE_NEW_SYMBOL_TYPE */
    }
    if ((mode&LOOKUP_SYM_ALLOWDECL) && SYMBOL_IS_WEAK(result)) {
     /* Re-declare this symbol. */
     SYMBOL_CLEAR_WEAK(result);
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
     if ((mode&LOOKUP_SYM_VGLOBAL) ||
        ((mode&LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT &&
          /* Default variable lookup in the root scope creates global variables. */
          current_scope == (DeeScopeObject *)current_rootscope)) {
      result->s_type = SYMBOL_TYPE_GLOBAL;
     } else if (mode & LOOKUP_SYM_STACK) {
      result->s_type = SYM_CLASS_STACK;
     } else if (mode & LOOKUP_SYM_STATIC) {
      result->s_type = SYMBOL_TYPE_STATIC;
     } else {
      result->s_type = SYMBOL_TYPE_LOCAL;
     }
#else
     result->sym_class = SYM_CLASS_VAR;
     if ((mode&LOOKUP_SYM_VGLOBAL) ||
        ((mode&LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT &&
          /* Default variable lookup in the root scope creates global variables. */
          current_scope == (DeeScopeObject *)current_rootscope)) {
      result->sym_flag = SYM_FVAR_GLOBAL;
     } else {
      if (mode&LOOKUP_SYM_STACK) {
       result->sym_class = SYM_CLASS_STACK;
       result->sym_flag  = SYM_FNORMAL;
#ifndef NDEBUG
       result->sym_stack.sym_offset = 0xcccc;
#endif
       /* Add the symbol to the chain of stack variables in this scope. */
       result->sym_stack.sym_nstck = iter->s_stk;
       result->sym_stack.sym_bound = 0;
       iter->s_stk = result;
      } else {
       result->sym_flag = (mode & LOOKUP_SYM_STATIC)
                        ? SYM_FVAR_STATIC
                        : SYM_FVAR_LOCAL;
      }
     }
#endif
     return result;
    }
    SYMBOL_MARK_USED(result);
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
    /* Recursively add references for the scope path that leads to this symbol. */
    if (iter->s_base != current_basescope &&
        SYM_MUST_REFERENCE(result))
        result = basescope_addref(current_basescope,result);
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
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
   while (result && result->sym_name != name)
          result = result->sym_next;
   if (result) {
    SYMBOL_MARK_USED(result);
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
    /* Check if the symbol must be referenced. */
    if (iter->s_base != current_basescope &&
        SYM_MUST_REFERENCE(result)) {
     /* Recursively add references for the scope path that leads to this symbol. */
     result = basescope_addref(current_basescope,result);
    }
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
    return result;
   }
  }
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
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
#endif
 } while ((iter = iter->s_prev) != NULL);
create_variable:
 if (!(mode&LOOKUP_SYM_ALLOWDECL) &&
       WARNAT(warn_loc,W_UNKNOWN_VARIABLE,name->k_name))
       goto err;
 if ((mode&LOOKUP_SYM_VGLOBAL) &&
      current_scope != (DeeScopeObject *)current_rootscope &&
      WARNAT(warn_loc,W_DECLARING_GLOBAL_IN_NONROOT,name))
      goto err;
 /* Create a new symbol. */
 if unlikely((result = sym_alloc()) == NULL)
    goto err;
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
 memset(result,0,sizeof(*result));
 result->s_name  = name;
 result->s_scope = current_scope;
 result->s_type  = SYMBOL_TYPE_FWD;
#else
 result->sym_read  = 0;
 result->sym_write = 0;
 result->sym_name  = name;
 result->sym_class = SYM_CLASS_VAR;
 result->sym_var.sym_doc = NULL;
#endif
 if ((mode&LOOKUP_SYM_VGLOBAL) ||
    ((mode&LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT &&
      /* Default variable lookup in the root scope creates global variables. */
      current_scope == (DeeScopeObject *)current_rootscope)) {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
  result->s_type = SYMBOL_TYPE_GLOBAL;
#else
  result->sym_flag = SYM_FVAR_GLOBAL;
#endif
  iter = (DeeScopeObject *)current_rootscope;
 } else {
  iter = current_scope;
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
  if (mode&LOOKUP_SYM_STACK) {
   result->s_type = SYMBOL_TYPE_STACK;
  } else if (mode&LOOKUP_SYM_STATIC) {
   result->s_type = SYMBOL_TYPE_STATIC;
  } else {
   result->s_type = SYMBOL_TYPE_LOCAL;
  }
#else
  if (mode&LOOKUP_SYM_STACK) {
   result->sym_class = SYM_CLASS_STACK;
   result->sym_flag  = SYM_FNORMAL;
#ifndef NDEBUG
   result->sym_stack.sym_offset = 0xcccc;
#endif
   /* Add the symbol to the chain of stack variables in this scope. */
   result->sym_stack.sym_nstck = iter->s_stk;
   result->sym_stack.sym_bound = 0;
   iter->s_stk = result;
  } else {
   result->sym_flag = (mode&LOOKUP_SYM_STATIC ? SYM_FVAR_STATIC : SYM_FVAR_LOCAL);
  }
#endif
 }
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
add_result_to_iter:
#endif
 if (++iter->s_mapc > iter->s_mapa) {
  /* Must rehash this scope. */
  if unlikely(rehash_scope(iter)) goto err_r;
 }
 /* Insert the new symbol. */
 ASSERT(iter->s_mapa != 0);
 bucket = &iter->s_map[name->k_id % iter->s_mapa];
 result->sym_next = *bucket;
 *bucket = result;
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
 result->s_scope = current_scope;
 if (warn_loc)
  memcpy(&result->s_decl,
          warn_loc,
          sizeof(struct ast_loc));
 else {
  loc_here(&result->s_decl);
 }
#else
 result->sym_scope = current_scope;
#endif
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
   if (result->sym_name == name) {
    /* Return this instance if it is the one that was requested. */
    if (!nth--) {
     SYMBOL_MARK_USED(result);
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
     /* Check if the symbol must be referenced. */
     if (iter->s_base != current_basescope && SYM_MUST_REFERENCE(result)) {
      /* Recursively add references for the scope path that leads to this symbol. */
      result = basescope_addref(current_basescope,result);
     }
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
     return result;
    }
    break;
   }
   result = result->sym_next;
  }
 }
 return NULL;
}


INTERN struct symbol *DCALL
new_local_symbol(struct TPPKeyword *__restrict name) {
 struct symbol *result,**bucket;
 ASSERT(name);
 if unlikely((result = sym_alloc()) == NULL) return NULL;
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct symbol));
#endif
 result->sym_name = name;
 if (++current_scope->s_mapc > current_scope->s_mapa) {
  if (rehash_scope(current_scope)) goto err_r;
 }
 ASSERT(current_scope->s_mapa != 0);
 bucket            = &current_scope->s_map[name->k_id % current_scope->s_mapa];
 result->sym_next  = *bucket;
 *bucket           = result;
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
 result->s_flag    = SYMBOL_FNORMAL;
 result->s_nread   = 0;
 result->s_nwrite  = 0;
 result->s_nbound  = 0;
 result->s_scope   = current_scope;
 loc_here(&result->s_decl); /* TODO: Let the caller pass this one. */
#else
 result->sym_flag  = SYM_FNORMAL;
 result->sym_read  = 0;
 result->sym_write = 0;
 result->sym_scope = current_scope;
#endif
 return result;
err_r:
 --current_scope->s_mapc;
 sym_free(result);
 return NULL;
}
INTERN struct symbol *DCALL
new_unnamed_symbol(void) {
 struct symbol *result;
 if unlikely((result = sym_alloc()) == NULL) return NULL;
#ifndef NDEBUG
 memset(result,0xcc,sizeof(struct symbol));
#endif
 result->sym_name = &TPPKeyword_Empty;
 result->sym_next = current_scope->s_del;
 current_scope->s_del = result;
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
 result->s_flag    = SYMBOL_FNORMAL;
 result->s_nread   = 0;
 result->s_nwrite  = 0;
 result->s_nbound  = 0;
 result->s_scope   = current_scope;
 loc_here(&result->s_decl); /* TODO: Let the caller pass this one. */
#else
 result->sym_flag  = SYM_FNORMAL;
 result->sym_read  = 0;
 result->sym_write = 0;
 result->sym_scope = current_scope;
#endif
 return result;
}
INTERN struct symbol *DCALL
get_local_symbol(struct TPPKeyword *__restrict name) {
 struct symbol *bucket;
 if (!current_scope->s_mapa) return false;
 bucket = current_scope->s_map[name->k_id % current_scope->s_mapa];
 while (bucket && bucket->sym_name != name) bucket = bucket->sym_next;
 return bucket;
}
INTERN void DCALL
del_local_symbol(struct symbol *__restrict sym) {
 struct symbol **pbucket,*bucket;
 ASSERT(sym);
 ASSERT(sym->sym_scope == current_scope);
 ASSERT(sym->sym_name != &TPPKeyword_Empty);
 ASSERT(current_scope->s_mapa != 0);
 pbucket = &current_scope->s_map[sym->sym_name->k_id % current_scope->s_mapa];
 while ((bucket = *pbucket,bucket && bucket != sym))
         pbucket = &bucket->sym_next;
 if (!bucket) return; /* Shouldn't happen, but ignore (could happen if the symbol is deleted twice) */
 /* Unlink the symbol from the bucket list. */
 *pbucket = sym->sym_next;
 /* Add the symbol to the chain of deleted (anonymous) symbols. */
 sym->sym_next = current_scope->s_del;
 current_scope->s_del = sym;
}

INTERN struct symbol *DCALL
scope_lookup(DeeScopeObject *__restrict scope,
             struct TPPKeyword *__restrict name) {
 struct symbol *result = NULL;
 if (!scope->s_mapa) goto done;
 result = scope->s_map[name->k_id % scope->s_mapa];
 while (result && result->sym_name != name) result = result->sym_next;
done:
 return result;
}

PRIVATE int DCALL
rehash_labels(void) {
 struct text_label **new_map,**biter,**bend;
 struct text_label *lbl_iter,*sym_next,**bucket;
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
   sym_next = lbl_iter->tl_next;
   bucket = &new_map[lbl_iter->tl_name->k_id % new_size];
   lbl_iter->tl_next = *bucket;
   *bucket = lbl_iter;
   lbl_iter = sym_next;
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
new_case_label(DeeAstObject *__restrict expr) {
 struct text_label *result;
 ASSERTF(current_basescope->bs_cflags&BASESCOPE_FSWITCH,
         "Not inside a switch statement");
 ASSERT_OBJECT_TYPE(expr,&DeeAst_Type);
 result = lbl_alloc();
 if unlikely(!result) goto done;
 /* Initialize the new label. */
 Dee_Incref(expr);
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


INTERN struct symbol *DCALL get_current_class(void) {
 struct symbol *result;
 result = current_basescope->bs_class;
 ASSERT(result);
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
 if (result->sym_scope->s_base != current_basescope &&
     SYM_MUST_REFERENCE(result))
     result = basescope_addref(current_basescope,result);
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
 return result;
}
INTERN struct symbol *DCALL get_current_super(void) {
 struct symbol *result;
 result = current_basescope->bs_super;
 ASSERT(result);
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
 if (result->sym_scope->s_base != current_basescope &&
     SYM_MUST_REFERENCE(result))
     result = basescope_addref(current_basescope,result);
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
 return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_SYMBOL_H */
