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
#ifndef GUARD_DEEMON_COMPILER_SYMBOL_H
#define GUARD_DEEMON_COMPILER_SYMBOL_H 1

#include "../api.h"
#include "../object.h"
#include "../code.h"
#ifdef CONFIG_BUILDING_DEEMON
#include "tpp.h"
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

DECL_BEGIN

typedef struct scope_object      DeeScopeObject;
typedef struct base_scope_object DeeBaseScopeObject;
typedef struct root_scope_object DeeRootScopeObject;

struct TPPKeyword;
struct ast_object;
struct string_object;
struct module_object;
struct asm_sym;

struct ast_loc {
    struct TPPFile      *l_file; /* [0..1] Location file. */
#ifdef CONFIG_BUILDING_DEEMON
    union{
        struct TPPLCInfo l_lc;   /* [valid_if(l_file != NULL)] Line/column information. */
        struct{
            int          l_line; /* [valid_if(l_file != NULL)] Location line. */
            int          l_col;  /* [valid_if(l_file != NULL)] Location column. */
        };
    };
#else
     int                 l_line; /* [valid_if(l_file != NULL)] Location line. */
     int                 l_col;  /* [valid_if(l_file != NULL)] Location column. */
#endif
};




struct text_label {
    struct text_label          *tl_next; /* [0..1][owned] Next case-label, or the next symbol with
                                          *               the same modulated `sym_name->k_id' */
    union {
#ifdef __INTELLISENSE__
             struct ast_object *tl_expr; /* [0..1][valid_if(CHAIN(bs_swcase|ast_cases))][const]
                                          *  Expression of a case-label. NOTE: NULL for the default case.
                                          *  NOTE: Always NULL in `bs_swdefl|ast_default' labels. */
#else
        DREF struct ast_object *tl_expr; /* [0..1][valid_if(CHAIN(bs_swcase|ast_cases))][const]
                                          *  Expression of a case-label. NOTE: NULL for the default case.
                                          *  NOTE: Always NULL in `bs_swdefl|ast_default' labels. */
#endif
        struct TPPKeyword      *tl_name; /* [1..1][valid_if(CHAIN(bs_lbl[*]))][const] Name of this label. */
    };
    struct asm_sym             *tl_asym; /* [0..1] Assembly symbol (lazily allocated) */
    unsigned int                tl_goto; /* The number of times this label is used as a goto target. */
};


#undef CONFIG_USE_NEW_SYMBOL_TYPE
#define CONFIG_USE_NEW_SYMBOL_TYPE 1 /* TODO: Make this mandatory. */


#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
struct symbol {
    struct symbol       *s_next;   /* [0..1][owned] Next symbol with the same modulated `s_name->k_id' */
    struct TPPKeyword   *s_name;   /* [1..1][const] Name of this symbol. */
    DeeScopeObject      *s_scope;  /* [1..1] The scope declaring this symbol. */
#define SYMBOL_TYPE_NONE   0x0000  /* Undefined symbol type. */
#define SYMBOL_TYPE_GLOBAL 0x0001  /* A global symbol. */
#define SYMBOL_TYPE_EXTERN 0x0002  /* An external symbol. */
#define SYMBOL_TYPE_MODULE 0x0003  /* An import module. */
#define SYMBOL_TYPE_MYMOD  0x0004  /* The current module. */
#define SYMBOL_TYPE_GETSET 0x0005  /* A get/set property symbol. */
#define SYMBOL_TYPE_IFIELD 0x0006  /* An instance member symbol. */
#define SYMBOL_TYPE_CFIELD 0x0007  /* An class member symbol. */
#define SYMBOL_TYPE_ALIAS  0x0008  /* An alias for a different symbol. -- TODO: Go though all places where this is unwound, and replace while-loops with if-checks */
#define SYMBOL_TYPE_ARG    0x0009  /* An argument passed to a function. */
#define SYMBOL_TYPE_LOCAL  0x000a  /* A local symbol. */
#define SYMBOL_TYPE_STACK  0x000b  /* A stack symbol. */
#define SYMBOL_TYPE_STATIC 0x000c  /* A static symbol. */
#define SYMBOL_TYPE_EXCEPT 0x000d  /* The current exception. */
#define SYMBOL_TYPE_MYFUNC 0x000e  /* The current function. */
#define SYMBOL_TYPE_THIS   0x000f  /* The this-argument of a function. */
#define SYMBOL_TYPE_AMBIG  0x0010  /* An ambiguous symbol (caused by `import *' when an overlap occurrs). */
#define SYMBOL_TYPE_FWD    0x0011  /* A forward-defined symbol. */
#define SYMBOL_TYPE_MAYREF(x) ((x) >= SYMBOL_TYPE_ARG)
    uint16_t             s_type;   /* Symbol class. (One of `SYMBOL_TYPE_*')
                                    * This describes how is the variable addressed, and where does it live. */
    uint16_t             s_flag;   /* Symbol flags (Set of `SYMBOL_F*') */
#define SYMBOL_FNORMAL   0x0000    /* Normal symbol flags. */
#define SYMBOL_FWEAK     0x0001    /* The symbol is defined weakly and can be overwritten by explicit
                                    * declarations, or turned into a non-weak symbol if used. */
#define SYMBOL_FALLOC    0x0002    /* Used during assembly: the symbol has been allocated. */
#define SYMBOL_FALLOCREF 0x0004    /* A reference ID for the symbol has been allocated. */
#define SYMBOL_FSTACK_NOUNBIND_OK 0x0100 /* FLAG: If the symbol appears in a `del' expression, and `sym_bound' is non-ZERO,
                                          *       still don't warn about the fact that a stack variable isn't being unbound,
                                          *       but is only being overwritten. */
    uint16_t             s_symid;  /* [valid_if(SYMBOL_FALLOC)] The dynamic ID allocated for the symbol.
                                    *  - SYMBOL_TYPE_GLOBAL: GID of the exported symbol.
                                    *  - SYMBOL_TYPE_EXTERN: MID of the imported module.
                                    *  - SYMBOL_TYPE_MODULE: MID of the imported module.
                                    *  - SYMBOL_TYPE_LOCAL:  LID of the exported symbol.
                                    *  - SYMBOL_TYPE_STACK:  Absolute stack position of the symbol.
                                    *  - SYMBOL_TYPE_STATIC: SID of the symbol. */
    uint16_t             s_refid;  /* [valid_if(SYMBOL_FALLOCREF)]
                                    * The effective reference ID of the symbol within the current assembler,
                                    * when the symbol is declared in a different base-scope, and must be
                                    * used in order to construct an inner function making use of it. */
    struct ast_loc       s_decl;   /* [const] The source location first referencing where the symbol. */
    uint32_t             s_nread;  /* [valid_if(!= SYMBOL_TYPE_ALIAS)] Number of times the symbol is read */
    uint32_t             s_nwrite; /* [valid_if(!= SYMBOL_TYPE_ALIAS)] Number of times the symbol is written */
    uint32_t             s_nbound; /* [valid_if(!= SYMBOL_TYPE_ALIAS)] Number of times the symbol is checking for being bound */
    union {                        /* Type-specific symbol data. */
        struct {
            DREF struct module_object *e_module; /* [1..1] The module from which the symbol is imported. */
            struct module_symbol      *e_symbol; /* [1..1] The symbol imported from another module. */
        }                s_extern; /* [SYMBOL_TYPE_EXTERN] */
        DREF struct module_object     *s_module; /* [SYMBOL_TYPE_MODULE] */
        struct {
            DREF struct string_object *g_doc;    /* [0..1] An optional documentation string of this global symbol. */
        }                s_global; /* [SYMBOL_TYPE_GLOBAL] */
        struct {
            struct symbol             *f_class;  /* [1..1][REF(SYMBOL_NREAD(.))] The class that is defining the symbol. */
            struct member_entry       *f_member; /* [1..1] The member that is being described. */
        }                s_field;  /* [SYMBOL_TYPE_CFIELD | SYMBOL_TYPE_IFIELD] */
        struct {
            struct symbol             *gs_get;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as getter. */
            struct symbol             *gs_del;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as delete. */
            struct symbol             *gs_set;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as setter. */
        }                s_getset; /* [SYMBOL_TYPE_GETSET] */
        struct symbol   *s_alias;  /* [SYMBOL_TYPE_ALIAS][->s_type != SYMBOL_TYPE_ALIAS]
                                    * [1..1] The symbol being aliased. */
        struct {
            struct ast_loc             a_decl2;  /* [const] The second declaration location. */
            size_t                     a_declc;  /* Number of additional declaration locations. */
            struct ast_loc            *a_declv;  /* [0..a_declc][owned] Additional declaration locations. */
        }                s_ambig;  /* [SYMBOL_TYPE_AMBIG] */
    };
};


#define SYMBOL_NAME(x)             ((x)->s_name->k_name)
#define SYMBOL_NREAD(x)          (*((x)->s_type == SYMBOL_TYPE_ALIAS ? &(x)->s_alias->s_nread : &(x)->s_nread))
#define SYMBOL_NWRITE(x)         (*((x)->s_type == SYMBOL_TYPE_ALIAS ? &(x)->s_alias->s_nwrite : &(x)->s_nwrite))
#define SYMBOL_NBOUND(x)         (*((x)->s_type == SYMBOL_TYPE_ALIAS ? &(x)->s_alias->s_nbound : &(x)->s_nbound))
#define SYMBOL_INC_NREAD(x)        ((x)->s_type == SYMBOL_TYPE_ALIAS ? (void)++(x)->s_alias->s_nread : (void)++(x)->s_nread)
#define SYMBOL_INC_NWRITE(x)       ((x)->s_type == SYMBOL_TYPE_ALIAS ? (void)++(x)->s_alias->s_nwrite : (void)++(x)->s_nwrite)
#define SYMBOL_INC_NBOUND(x)       ((x)->s_type == SYMBOL_TYPE_ALIAS ? (void)++(x)->s_alias->s_nbound : (void)++(x)->s_nbound)
#define SYMBOL_DEC_NREAD(x)        ((x)->s_type == SYMBOL_TYPE_ALIAS ? (void)(ASSERT((x)->s_alias->s_nread != 0),--(x)->s_alias->s_nread) : (void)(ASSERT((x)->s_nread != 0),--(x)->s_nread))
#define SYMBOL_DEC_NWRITE(x)       ((x)->s_type == SYMBOL_TYPE_ALIAS ? (void)(ASSERT((x)->s_alias->s_nwrite != 0),--(x)->s_alias->s_nwrite) : (void)(ASSERT((x)->s_nwrite != 0),--(x)->s_nwrite))
#define SYMBOL_DEC_NBOUND(x)       ((x)->s_type == SYMBOL_TYPE_ALIAS ? (void)(ASSERT((x)->s_alias->s_nbound != 0),--(x)->s_alias->s_nbound) : (void)(ASSERT((x)->s_nbound != 0),--(x)->s_nbound))
#define SYMBOL_MARK_USED(x)        (void)((x)->s_flag &= ~SYMBOL_FWEAK)
#define SYMBOL_IS_WEAK(x)          ((x)->s_flag & SYMBOL_FWEAK)
#define SYMBOL_CLEAR_WEAK(x)       (symbol_fini(x),(x)->s_nread = (x)->s_nwrite = 0,(x)->s_flag &= ~SYMBOL_FWEAK)

/* Check if a given symbol `x' must be addressed as a reference */
#define SYMBOL_MUST_REFERENCE(x)   (SYMBOL_TYPE_MAYREF((x)->s_type) && (x)->s_scope->s_base != current_basescope)

/* Same as `SYMBOL_MUST_REFERENCE()', but the caller already knows
 * that the symbol's type may be referenced (`SYMBOL_TYPE_MAYREF(x->s_type) == true') */
#define SYMBOL_MUST_REFERENCE_TYPEMAY(x) (ASSERT(SYMBOL_TYPE_MAYREF((x)->s_type)),(x)->s_scope->s_base != current_basescope)

/* Check if a given symbol `x' can be addressed as a reference */
#define SYMBOL_MAY_REFERENCE(x)    ((x)->s_scope->s_base != current_basescope)

#define SYMBOL_ALIAS(x)            ((x)->s_alias)           /* TODO: Remove me */
#define SYMBOL_SCOPE(x)            ((x)->s_scope)           /* TODO: Remove me */
#define SYMBOL_TYPE(x)             ((x)->s_type)            /* TODO: Remove me */
#define SYMBOL_FIELD_CLASS(x)      ((x)->s_field.f_class)   /* TODO: Remove me */
#define SYMBOL_FIELD_MEMBER(x)     ((x)->s_field.f_member)  /* TODO: Remove me */
#define SYMBOL_EXTERN_MODULE(x)    ((x)->s_extern.e_module) /* TODO: Remove me */
#define SYMBOL_EXTERN_SYMBOL(x)    ((x)->s_extern.e_symbol) /* TODO: Remove me */
#define SYMBOL_MODULE_MODULE(x)    ((x)->s_module)          /* TODO: Remove me */
#define SYMBOL_STACK_OFFSET(x)     ((x)->s_symid)           /* TODO: Remove me */
#define SYMBOL_ARG_INDEX(x)        ((x)->s_symid)           /* TODO: Remove me */
#define SYMBOL_GETSET_GETTER(x)    ((x)->s_getset.gs_get)   /* TODO: Remove me */
#define SYMBOL_GETSET_DELETE(x)    ((x)->s_getset.gs_del)   /* TODO: Remove me */
#define SYMBOL_GETSET_SETTER(x)    ((x)->s_getset.gs_set)   /* TODO: Remove me */

#endif /* CONFIG_USE_NEW_SYMBOL_TYPE */



/* Finalize the given symbol. */
INTERN void DCALL symbol_fini(struct symbol *__restrict self);




#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
struct symbol {
    struct symbol       *sym_next;      /* [0..1][owned] Next symbol with the same modulated `sym_name->k_id' */
#define SYMBOL_NAME(x) ((x)->sym_name->k_name)
    struct TPPKeyword   *sym_name;      /* [1..1][const] Name of this symbol. */
#define SYMBOL_SCOPE(x)((x)->sym_scope)
    DeeScopeObject      *sym_scope;     /* [1..1] The scope declaring this symbol. */
#define SYMBOL_TYPE(x) ((x)->sym_class)
    uint16_t             sym_class;     /* Symbol class. (One of `SYM_CLASS_*')
                                         * This describes how is the variable addressed, and where does it live.
                                         * Sorry, deemon has a lot of these... */
#define SYM_FNORMAL      0x0000         /* Normal symbol flags. */
    uint16_t             sym_flag;      /* Symbol flags (Set of `SYM_F*' dependent on `sym_class') */
#if __SIZEOF_POINTER__ > 4
    uint16_t             sym_pad[(sizeof(void *)/2)-2]; /* ... */
#endif
#define SYMBOL_NREAD(x)      ((x)->sym_read)
#define SYMBOL_NWRITE(x)     ((x)->sym_write)
#define SYMBOL_INC_NREAD(x)  ((void)++(x)->sym_read)
#define SYMBOL_INC_NWRITE(x) ((void)++(x)->sym_write)
#define SYMBOL_DEC_NREAD(x)  (ASSERT((x)->sym_read),(void)--(x)->sym_read)
#define SYMBOL_DEC_NWRITE(x) (ASSERT((x)->sym_write),(void)--(x)->sym_write)
    uint32_t             sym_read;      /* The amount of times that this symbol is read from. */
    uint32_t             sym_write;     /* The amount of times that this symbol is written to. (Excluding an initial assignment)
                                         * HINT: Using this field, the code generator performs constant optimization. */
    union {

#define SYMBOL_MARK_USED(x)  \
   ((x)->sym_class == SYM_CLASS_EXTERN ? \
      (void)((x)->sym_flag &= ~SYM_FEXTERN_WEAK) : \
      (void)0)
#define SYMBOL_IS_WEAK(x)    \
  (((x)->sym_class == SYM_CLASS_EXTERN && ((x)->sym_flag & SYM_FEXTERN_WEAK)) || \
   ((x)->sym_class == SYM_CLASS_AMBIGUOUS))
#define SYMBOL_CLEAR_WEAK(x) \
   ((x)->sym_class == SYM_CLASS_EXTERN ? (void)Dee_Decref((x)->sym_extern.sym_module) : (void)0, \
    (x)->sym_read = (x)->sym_write = 0)

        struct {
#define SYM_CLASS_EXTERN               0x0000      /* External symbol. (NOTE: Doesn't use `SYM_FALLOC') */
#   define SYM_FEXTERN_WEAK            0x4000      /* FLAG: The variable is weakly imported, and is silently overwritten by an explicit import, or declaration. */
#   define SYM_FEXTERN_ALLOC           0x8000      /* FLAG: The module index for the variable has been allocated. */
            DREF struct module_object *sym_module; /* [1..1][const] The module exporting this symbol. */
            struct module_symbol      *sym_modsym; /* [1..1] The symbol that is being imported from `sym_module'. */
            uint16_t                   sym_modid;  /* [valid_if(SYM_FEXTERN_ALLOC)] The module's index in the `rs_importv' vector of the current root-scope. */
        }                              sym_extern; /* External variable. */
#define SYMBOL_EXTERN_MODULE(x)    ((x)->sym_extern.sym_module)
#define SYMBOL_EXTERN_SYMBOL(x)    ((x)->sym_extern.sym_modsym)

        struct {
#define SYM_CLASS_VAR                  0x0001      /* Variable. */
#   define SYM_FVAR_GLOBAL             0x0000      /* Global variable. */
#   define SYM_FVAR_LOCAL              0x0001      /* Local variable. */
#   define SYM_FVAR_STATIC             0x0002      /* Static variable. */
#   define SYM_FVAR_MASK               0x00ff      /* MASK: The mask for the variable sub-class. */
#   define SYM_FVAR_ALLOC              0x8000      /* FLAG: The variable has been allocated.
                                                    * HINT: Code can (and does) assume that this flag is only set during code generation. */
            uint16_t                   sym_index;  /* [valid_if(SYM_FVAR_ALLOC)] Variable index (either global, local or static). */
            DREF struct string_object *sym_doc;    /* [0..1] Optional documentation string for global variables. */
        }                              sym_var;    /* Variable. */

        struct {
#define SYM_CLASS_STACK                0x0002      /* Stack-based Variable. */
#   define SYM_FSTK_ALLOC              0x8000      /* FLAG: The variable has been allocated.
                                                    * HINT: Code can (and does) assume that this flag is only set during code generation. */
#   define SYM_FSTK_NOUNBIND_OK        0x0001      /* FLAG: If the symbol appears in a `del' expression, and `sym_bound' is non-ZERO,
                                                    *       still don't warn about the fact that a stack variable isn't being unbound,
                                                    *       but is only being overwritten. */
            uint16_t                   sym_offset; /* Absolute stack address. */
            struct symbol             *sym_nstck;  /* [0..1] Next stack-based variable apart of the current scope. */
            uint32_t                   sym_bound;  /* The number of times that the symbol appears as part of a `bound()' expression. */
        }                              sym_stack;  /* Stack-based Variable. */
#define SYMBOL_STACK_OFFSET(x)  ((x)->sym_stack.sym_offset)
#define SYMBOL_NBOUND(x)        ((x)->sym_class == SYM_CLASS_STACK ? (x)->sym_stack.sym_bound : 0)
#define SYMBOL_INC_NBOUND(x)    ((x)->sym_class == SYM_CLASS_STACK ? (void)(++(x)->sym_stack.sym_bound) : (void)0)
#define SYMBOL_DEC_NBOUND(x)    ((x)->sym_class == SYM_CLASS_STACK ? (void)(ASSERT((x)->sym_stack.sym_bound != 0),--(x)->sym_stack.sym_bound) : (void)0)

        struct {
#define SYM_CLASS_ARG                  0x0003      /* Argument variable. */
            uint16_t                   sym_index;  /* [const] Virtual argument index. */
        }                              sym_arg;    /* Argument variable. */
#define SYMBOL_ARG_INDEX(x)  ((x)->sym_arg.sym_index)

        struct {
#define SYM_CLASS_REF                  0x8004      /* Referenced variable. */
#   define SYM_FREF_ALLOC              0x8000      /* FLAG: The reference has been allocated. */
            struct symbol             *sym_ref;    /* [1..1][->sym_scope->s_base == sym_scope->s_base->bs_prev]
                                                    * A variable referenced from the previous base-scope.
                                                    * NOTE: For every symbol, only one reference symbol
                                                    *       must ever be created in each child scope.
                                                    *       When referenced variables are used more than
                                                    *       once, the same reference must be re-used, so-as
                                                    *       to prevent creation of multiple references for
                                                    *       the same symbol. */
            struct symbol             *sym_rnext;  /* [0..1][CHAIN(sym_scope->s_base->bs_refs)]
                                                    * Chain of referenced variables in the current base-scope.
                                                    * >> Used to track which symbols a reference has already been created for. */
            uint16_t                   sym_index;  /* [valid_if(SYM_FREF_ALLOC)] Reference index. */
        }                              sym_ref;    /* Referenced variable. */

        struct {
#define SYM_CLASS_MEMBER               0x0005      /* Instance member slot. */
#   define SYM_FMEMBER_INST            0x0000      /* VALUE: The symbol is part of the instance member table. */
#   define SYM_FMEMBER_CLASS           0x0001      /* FLAG: The symbol is part of the class member table. */
            struct symbol             *sym_class;  /* [1..1] A symbol describing the class that is defining this member. */
            struct member_entry       *sym_member; /* [1..1] The member that is being described. */
            struct symbol             *sym_ref;    /* [0..1] A chain of symbols that are referencing this member.
                                                    * NOTE: Every entry is another SYM_CLASS_MEMBER, who's `sym_member.sym_class'
                                                    *       field is a `SYM_CLASS_REF' to our `sym_member.sym_class', as well as
                                                    *       sharing the same `sym_member' pointer. */
        }                              sym_member;
#define SYMBOL_FIELD_CLASS(x)      ((x)->sym_member.sym_class)
#define SYMBOL_FIELD_MEMBER(x)     ((x)->sym_member.sym_member)

        struct {
#define SYM_CLASS_MODULE               0x0006      /* Import module reference. */
#   define SYM_FMODULE_ALLOC           0x8000      /* FLAG: The import module index has been assigned. */
            DREF struct module_object *sym_module; /* [1..1] The referenced module. */
            uint16_t                   sym_modid;  /* [valid_if(SYM_FMODULE_ALLOC)] The module's index in the `rs_importv' vector of the current root-scope. */
        }                              sym_module; /* Import module reference. */
#define SYMBOL_MODULE_MODULE(x)    ((x)->sym_module.sym_module)

        struct {
#define SYM_CLASS_PROPERTY             0x0007      /* Property symbol (invokes callbacks for get/del/set). */
            struct symbol             *sym_get;    /* [0..1] The getter callback. */
            struct symbol             *sym_del;    /* [0..1] The delete callback. */
            struct symbol             *sym_set;    /* [0..1] the setter callback. */
        }                              sym_property; /* Property symbol. */
#define SYMBOL_GETSET_GETTER(x)    ((x)->sym_property.sym_get)
#define SYMBOL_GETSET_DELETE(x)    ((x)->sym_property.sym_del)
#define SYMBOL_GETSET_SETTER(x)    ((x)->sym_property.sym_set)

#define SYM_CLASS_ALIAS                0x0008      /* Alias for another symbol. */
        struct {
            struct symbol             *sym_alias;  /* [1..1][->sym_scope == sym_scope]
                                                    * The symbol being aliased.
                                                    * NOTE: In class scopes, this type of symbol is used
                                                    *       for forward references and as placeholder. */
        }                              sym_alias;
#define SYMBOL_ALIAS(x)      ((x)->sym_alias.sym_alias)

        /* Other symbol classes that reference special symbols. */
#define SYM_CLASS_EXCEPT        0x1009 /* The current exception. */
#define SYM_CLASS_THIS_MODULE   0x900a /* The current module. */
#define SYM_CLASS_THIS_FUNCTION 0x900b /* The function object identified by `s_scope->s_base'. */
#define SYM_CLASS_THIS          0x900c /* The `this' variable in class-member functions (thiscall functions).
                                        * NOTE: This class requires that `s_scope->s_base'
                                        *       have the `SCOPE_FTHISCALL' flag set. */
#define SYM_CLASS_AMBIGUOUS     0x900d /* An ambiguous symbol. */
    };
};
/* Symbol classes that must be referenced when not part of the current base-scope. */
#define SYM_MUST_REFERENCE(x) \
  ((x)->sym_class != SYM_CLASS_EXTERN && \
   (x)->sym_class != SYM_CLASS_MODULE && \
   (x)->sym_class != SYM_CLASS_THIS_MODULE && \
  ((x)->sym_class != SYM_CLASS_VAR || \
   (x)->sym_flag  != SYM_FVAR_GLOBAL))
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */



#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
#define sym_next                s_next              /* TODO: Remove me */
#define sym_name                s_name              /* TODO: Remove me */
#define SYM_CLASS_EXTERN        SYMBOL_TYPE_EXTERN  /* TODO: Remove me */
#define SYM_CLASS_STACK         SYMBOL_TYPE_STACK   /* TODO: Remove me */
#define SYM_CLASS_ARG           SYMBOL_TYPE_ARG     /* TODO: Remove me */
#define SYM_CLASS_MODULE        SYMBOL_TYPE_MODULE  /* TODO: Remove me */
#define SYM_CLASS_PROPERTY      SYMBOL_TYPE_GETSET  /* TODO: Remove me */
#define SYM_CLASS_ALIAS         SYMBOL_TYPE_ALIAS   /* TODO: Remove me */
#define SYM_CLASS_EXCEPT        SYMBOL_TYPE_EXCEPT  /* TODO: Remove me */
#define SYM_CLASS_THIS_MODULE   SYMBOL_TYPE_MYMOD   /* TODO: Remove me */
#define SYM_CLASS_THIS_FUNCTION SYMBOL_TYPE_MYFUNC  /* TODO: Remove me */
#define SYM_CLASS_THIS          SYMBOL_TYPE_THIS    /* TODO: Remove me */
#define SYM_CLASS_AMBIGUOUS     SYMBOL_TYPE_AMBIG   /* TODO: Remove me */
#endif



#ifdef CONFIG_BUILDING_DEEMON
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
INTDEF char const symclass_names[0x1f + 1][8];
#define SYMCLASS_NAME(cls) symclass_names[(cls)&0x1f]
#else
INTDEF char const symclass_names[0xf + 1][14];
#define SYMCLASS_NAME(cls) symclass_names[(cls)&0xf]
#endif

#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
/* Return the real symbol given a symbol that may be a reference. */
INTDEF struct symbol *DCALL sym_realsym(struct symbol *__restrict self);
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */

#endif


struct scope_object {
    OBJECT_HEAD
#ifdef __INTELLISENSE__
    DeeScopeObject          *s_prev;  /* [0..1][const] Previous scope. */
#else                                 
    DREF DeeScopeObject     *s_prev;  /* [0..1][const] Previous scope. */
#endif                                
    DeeBaseScopeObject      *s_base;  /* [1..1][const] The base scope of the surrounding function.
                                       * HINT: If this is a self-pointer, this scope is actually a `DeeBaseScopeObject'  */
    struct symbol          **s_map;   /* [0..1][owned][0..s_mapa][owned] Hash-map of symbols defined in this scope.
                                       * HINT: Use the TPP keyword id modulated by `s_mapa' as index. */
    size_t                   s_mapc;  /* Amount of symbols defined within the hash-map `s_map'. */
    size_t                   s_mapa;  /* Allocated vector size of the symbol hash-map `s_map'. */
    struct symbol           *s_del;   /* [0..1][owned] Chain of symbols that have been deleted. (And thereby made invisible) */
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
    struct symbol           *s_stk;   /* [0..1][CHAIN(->sym_stack.sym_nstck)] Chain of stack-based symbols. */
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
#define SCOPE_FNORMAL        0x0000   /* Normal scope flags. */
#define SCOPE_FCLASS         0x0001   /* Class scope. */
    uint16_t                 s_flags; /* Scope flags (Set of `SCOPE_F*'). */
    uint16_t                 s_pad[(sizeof(void *)/2)-1];
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
};

#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
#define SCOPE_ADD_STACKSYM(self,x) \
 (void)(ASSERT((x)->sym_class == SYM_CLASS_STACK), \
               (x)->sym_flag = SYM_FNORMAL, \
               (x)->sym_stack.sym_nstck = (self)->s_stk, \
               (self)->s_stk = (x))
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */



struct base_scope_object {
    DeeScopeObject      bs_scope;      /* Underlying regular scope. */
    DeeBaseScopeObject *bs_prev;       /* [0..1] The previous base (function) scope.
                                        * NOTE: When `NULL', this scope is actually a `DeeRootScopeObject'. */
    DeeRootScopeObject *bs_root;       /* [1..1] The module-local root/global scope.
                                        * HINT: If this is a self-pointer, this scope is actually a `DeeRootScopeObject'. */
    struct TPPKeyword  *bs_name;       /* [0..1][const] Name of the function of this scope.
                                        * HINT: During creating of a base-scope, the creator is required
                                        *       to register a symbol for function self-referencing.
                                        *       With that in mind, unnamed or root-mode functions does have such a symbol. */
    struct text_label **bs_lbl;        /* [0..1][owned][0..bs_lbla][owned] Hash-map of labels defined in this scope.
                                        * HINT: Use the TPP keyword id modulated by `bs_lbla' as index. */
    size_t              bs_lblc;       /* Amount of labels defined within the hash-map `bs_lbl'. */
    size_t              bs_lbla;       /* Allocated vector size of the labels hash-map `bs_lbl'. */
    struct text_label  *bs_swcase;     /* [0..1][CHAIN(->tl_next)][owned] Chain of switch labels.
                                        * NOTE: This chain links cases in the reverse order of their appearance. */
    struct text_label  *bs_swdefl;     /* [0..1][owned] Default label in a switch statement. */
#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
    struct symbol      *bs_refs;       /* [0..1] Chain of references defined for this scope. */
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
    struct symbol      *bs_super;      /* [0..1] A symbol describing the super-identifier in thiscall functions.
                                        * NOTE:  If necessary, this symbol is replaced with a `SYM_CLASS_REF' variable
                                        *        upon first access (call to `get_current_super()'), following the usual
                                        *        rules for accessing out-of-scope variables. */
    struct symbol      *bs_class;      /* [0..1] Same as `bs_super', but instead used to refer to a class's own
                                        *        type, which is required for accessing instance members by index. */
    DeeCodeObject      *bs_restore;    /* [0..1] Pointer to the generated code object (once that code object has been generated)
                                        * In the event that assembler must be reset due to a linker truncation,
                                        * this code object will be used to restore inherited (stolen) data. */
    DREF DeeObject    **bs_default;    /* [1..1][0..(bs_argc_max - bs_argc_min)][owned] Vector of function default arguments. */
    struct symbol      *bs_varargs;    /* [0..1] A symbol for the varargs argument. */
    struct symbol     **bs_argv;       /* [1..1][0..bs_argc][owned] Vector of arguments taken by the function implemented by this scope.
                                        * HINT: This vector is also used to track which arguments was written to
                                        *      (deemon assembly doesn't allow modifications of arguments), as all
                                        *       those that was will have been converted into `SYM_CLASS_LOCAL'.
                                        *       Using this information, the assembly generate will emit code to copy
                                        *       all modified arguments into local variables at the start of the function:
                                        *   >> function foo(x) { print x; }
                                        *   >> function bar(x) { print x; x = 10; print x; }
                                        * [[*]->s_index == *] The index in this vector _MUST_ mirror the argument index.
                                        * [[*]->s_scope == self] All symbols referenced _MUST_ be associated with `bs_prev'.
                                        * [[*]->s_class == SYM_CLASS_ARG] All symbols referenced _MUST_ be argument symbols.
                                        * Only symbols of the following classes (should) appear as references (because other's don't make sense):
                                        * ASSEMBLY:
                                        *   >> foo:
                                        *   >>    push arg @x
                                        *   >>    print pop, nl
                                        *   >>    ret
                                        *   >> bar:
                                        *   >>    push arg @x
                                        *   >>    pop local @x
                                        *   >>    push local @x // May be optimized into a dup
                                        *   >>    print pop, nl
                                        *   >>    push $10
                                        *   >>    pop local @x
                                        *   >>    push local @x // May be optimized into a dup
                                        *   >>    print pop, nl
                                        *   >>    ret
                                        */
    /* Argument indices are ordered as follows:
     *  0 ... bs_argc_min-1                       --- `foo'        Mandatory arguments without default values
     *  bs_argc_min ... bs_argc_max-1             --- `foo = none' Optional arguments with default values
     *  bs_argc_max ... bs_argc_max+bs_argc_opt-1 --- `?foo'       Optional arguments without default values
     *  bs_argc_max+bs_argc_opt                   --- `foo...'     The varargs argument
     */
#define DeeBaseScope_IsArgRequired(self,x)   ((x) < (self)->bs_argc_min)
#define DeeBaseScope_IsArgDefault(self,x)    ((x) >= (self)->bs_argc_min && (x) < (self)->bs_argc_max)
#define DeeBaseScope_IsArgReqOrDefl(self,x)  ((x) < (self)->bs_argc_max)
#define DeeBaseScope_IsArgOptional(self,x)   ((x) >= (self)->bs_argc_max && (x) < (self)->bs_argc_max+(self)->bs_argc_opt)
#define DeeBaseScope_IsArgVarArgs(self,x)    ((self)->bs_varargs && (x) == (uint16_t)(self)->bs_argc-1)
#define DeeBaseScope_HasOptional(self)       ((self)->bs_argc_opt != 0)
    uint16_t            bs_argc_min;   /* Min amount of argument symbols defined for this scope. */
    uint16_t            bs_argc_max;   /* Max amount of argument symbols defined for this scope. */
    uint16_t            bs_argc_opt;   /* The number of optional argument symbols defined for this scope. */
    uint16_t            bs_argc;       /* The actual amount of argument symbols.
                                        * == (bs_argc_max+bs_argc_opt)+(bs_varargs ? 1 : 0); */
    uint16_t            bs_flags;      /* Scope flags (Set of `CODE_F*'). */
#define BASESCOPE_FNORMAL 0x0000       /* Normal base-scope flags. */
#define BASESCOPE_FRETURN 0x0001       /* A non-empty return statement has been encountered. */
#define BASESCOPE_FSWITCH 0x0002       /* The parser is currently allowed to generate switch-labels. */
    uint16_t            bs_cflags;     /* Compile-time scope flags (Set of `BASESCOPE_F*'). */
#if __SIZEOF_POINTER__ > 4
    uint16_t            bs_padding[(sizeof(void *)/2)-2];
#endif
};


struct root_scope_object {
    DeeBaseScopeObject         rs_scope;   /* Underlying base scope. */
    DREF struct module_object *rs_module;  /* [1..1][const] The module that is being compiled. */
    /* NOTE: Fields below are only modified during code generation (aka. by the assembler)
     *    >> Before assembly starts for the first time, they should all be ZERO/NULL-initialized. */
    DREF DeeCodeObject        *rs_code;    /* [0..1][LINK(->co_next)] Linked list of all code objects already generated for this module. */
    DREF struct module_object**rs_importv; /* [1..1][0..rs_importc|ALLOC(rs_importa)] Vector of other modules imported by this one. */
    struct module_symbol      *rs_bucketv; /* [0..rs_bucketm+1][owned_if(!= empty_module_buckets)]
                                            * Hash-vector for translating a string into a `uint16_t' index for a global variable.
                                            * This is where module symbol names are stored and also.
                                            * HINT: This vector is populated by the assembler during code generation. */
    uint16_t                   rs_flags;   /* Module flags (Set of `MODULE_F*') */
    uint16_t                   rs_globalc; /* The total number of global variables. */
    uint16_t                   rs_bucketm; /* Mask applied to symbol buckets. */
    uint16_t                   rs_importc; /* The total number of other modules imported by this one. */
    uint16_t                   rs_importa; /* The allocated amount of memory for the imported module vector. */
};

DDATDEF DeeTypeObject DeeScope_Type;
DDATDEF DeeTypeObject DeeBaseScope_Type;
DDATDEF DeeTypeObject DeeRootScope_Type;


#ifdef CONFIG_BUILDING_DEEMON
INTDEF DREF DeeScopeObject *current_scope;     /* [lock(DeeCompiler_Lock)][1..1] The current scope. */
INTDEF DeeBaseScopeObject  *current_basescope; /* [lock(DeeCompiler_Lock)][1..1][== current_scope->s_base] The current base scope. */
INTDEF DeeRootScopeObject  *current_rootscope; /* [lock(DeeCompiler_Lock)][1..1][== current_basescope->bs_root] The current root scope. */

/* Begin/end a new scope.
 * NOTE: The caller should then fill in special information in `current_scope'. */
INTDEF int (DCALL scope_push)(void);
INTDEF void DCALL scope_pop(void);

/* Begin/end a new base-scope.
 * NOTE: The caller should then fill in special information in `current_basescope'. */
INTDEF int (DCALL basescope_push)(void);
INTDEF void DCALL basescope_pop(void);
INTDEF void DCALL basescope_push_ob(DeeBaseScopeObject *__restrict scope);
INTDEF DREF DeeBaseScopeObject *DCALL basescope_new(void);

/* Lookup a symbol for the given `name', following constraints set by `mode'.
 * @param: mode:     Set of `LOOKUP_SYM_*'
 * @param: warn_loc: When non-NULL the location to reference in warnings.
 *                   When NULL, the current location is used instead.
 * @return: SYMBOL_IGNORE: The caller should ignore this symbol, replacing
 *                         it with a none-expression, or placeholder.
 *                         This is what is done to prevent unused reference
 *                         variables from popping up when they are only
 *                         used from dead branches (aka: `parser_flags & PARSE_DEAD')
 * @return: * :            A new reference to the symbol requested.
 * @return: NULL:          An error occurred.
 */
INTDEF struct symbol *DCALL
lookup_symbol(unsigned int mode, struct TPPKeyword *__restrict name,
              struct ast_loc *warn_loc);
#define LOOKUP_SYM_NORMAL    0x0000
#define LOOKUP_SYM_VDEFAULT  0x0000 /* Default visibility. */
#define LOOKUP_SYM_VLOCAL    0x0001 /* Lookup rules when `local' is prefixed. */
#define LOOKUP_SYM_VGLOBAL   0x0002 /* Lookup rules when `global' is prefixed. */
#define LOOKUP_SYM_VMASK     0x0003 /* Mask for visibility options. */
#define LOOKUP_SYM_STATIC    0x0100 /* Create static variables / warn about non-static, existing variables. */
#define LOOKUP_SYM_STACK     0x0200 /* Create stack variables / warn about non-stack, existing variables. */
#define LOOKUP_SYM_ALLOWDECL 0x8000 /* Allow declaration of new variables (HINT: Unless set, warn when new variables are created). */

/* Lookup the nth instance of `name' (starting at 1 for the first)
 * Return `NULL' if no such instance exists or if nth is 0 */
INTDEF struct symbol *DCALL
lookup_nth(unsigned int nth, struct TPPKeyword *__restrict name);

/* Check if `name' is a reserved symbol name. */
INTDEF bool DCALL
is_reserved_symbol_name(struct TPPKeyword *__restrict name);


/* Lookup or create a label, given its name in the current base-scope. */
INTDEF struct text_label *DCALL lookup_label(struct TPPKeyword *__restrict name);
/* Create a new case label for `expr'.
 * NOTE: The caller is responsible to ensure that the `BASESCOPE_FSWITCH' flag is set. */
INTDEF struct text_label *DCALL new_case_label(struct ast_object *__restrict expr);
/* Ensure existence and return the default label of a switch-statement.
 * NOTE: The caller is responsible to ensure that the `BASESCOPE_FSWITCH' flag is set.
 *       Additionally (if this is desired), it is the caller's task to warn if the
 *       default case had already been allocated before. - This function's only
 *       purpose is to lazily allocate a missing default case and initialize it. */
INTDEF struct text_label *DCALL new_default_label(void);


#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
/* Safely create a reference to a given symbol from `current_basescope'. */
INTDEF struct symbol *DCALL
symbol_reference(DeeBaseScopeObject *__restrict current_basescope,
                 struct symbol *__restrict sym);
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */

/* Lookup a symbol in the given scope. */
INTDEF struct symbol *DCALL
scope_lookup(DeeScopeObject *__restrict scope,
             struct TPPKeyword *__restrict name);


/* Copy argument symbols from the given `other' base-scope into the current,
 * alongside defining them as symbols while duplicating default-values and the
 * var-args flag. - Basically, everything that may be inferred from an argument list.
 * This is done when creating superargs class operators. */
INTDEF int DCALL copy_argument_symbols(DeeBaseScopeObject *__restrict other);

#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
/* Fully link all forward-defined symbols inside of a class scope. */
INTDEF int DCALL link_forward_symbols(void);
#endif /* CONFIG_USE_NEW_SYMBOL_TYPE */

/* Allocate and return a new symbol private to the current scope.
 * NOTE: The caller must ensure that no variable with the given name already exist.
 *       Should the symbol already exist, then there will be two defined afterwards,
 *       only one of which will actually be addressable.
 * NOTE: The caller is also required to initialize the returned
 *       symbol, who's class is undefined up until that point. */
INTDEF struct symbol *DCALL new_local_symbol(struct TPPKeyword *__restrict name);
INTDEF struct symbol *DCALL get_local_symbol(struct TPPKeyword *__restrict name);
#define has_local_symbol(name) (get_local_symbol(name) != NULL)
/* Delete a given local symbol, making it anonymous. */
INTDEF void DCALL del_local_symbol(struct symbol *__restrict sym);
/* Create a new unnamed (aka. deleted) symbol. */
INTDEF struct symbol *DCALL new_unnamed_symbol(void);


/* Get symbols describing the current class or its super type.
 * NOTE: These functions will lazily allocate references for
 *       out-of-scope symbols, only returning NULL when allocation
 *       of those symbols fails due to lack of memory.
 *       The caller on the other hand is required to check
 *       if a class/super type is available _BEFOREHAND_
 *       using the below `has_current_*()' macros. */
INTDEF struct symbol *DCALL get_current_class(void);
INTDEF struct symbol *DCALL get_current_super(void);
#define has_current_class() (current_basescope->bs_class != NULL)
#define has_current_super() (current_basescope->bs_super != NULL)


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define scope_push()      __builtin_expect(scope_push(),0)
#define basescope_push()  __builtin_expect(basescope_push(),0)
#endif
#endif

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_SYMBOL_H */
