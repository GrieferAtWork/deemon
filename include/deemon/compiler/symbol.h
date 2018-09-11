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

#define CONFIG_USE_NEW_CLASS_SCOPES 1

typedef struct scope_object       DeeScopeObject;
typedef struct class_scope_object DeeClassScopeObject;
typedef struct base_scope_object  DeeBaseScopeObject;
typedef struct root_scope_object  DeeRootScopeObject;

struct TPPKeyword;
struct ast;
struct string_object;
struct module_object;
struct asm_sym;
struct class_attribute;

struct ast_loc {
    struct TPPFile      *l_file; /* [0..1] Location file. */
#ifdef CONFIG_BUILDING_DEEMON
    union {
        struct TPPLCInfo l_lc;   /* [valid_if(l_file != NULL)] Line/column information. */
        struct {
            int          l_line; /* [valid_if(l_file != NULL)] Location line. */
            int          l_col;  /* [valid_if(l_file != NULL)] Location column. */
        }
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
        _l_linecol
#endif
        ;
    }
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
    _l_lc_select
#define l_lc       _l_lc_select.l_lc
#ifdef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define l_line     _l_lc_select.l_line
#define l_col      _l_lc_select.l_col
#else /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#define l_line     _l_lc_select._l_linecol.l_line
#define l_col      _l_lc_select._l_linecol.l_col
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#elif !defined(__COMPILER_HAVE_TRANSPARENT_STRUCT)
#define l_line     _l_linecol.l_line
#define l_col      _l_linecol.l_col
#endif
    ;
#else
    int                  l_line; /* [valid_if(l_file != NULL)] Location line. */
    int                  l_col;  /* [valid_if(l_file != NULL)] Location column. */
#endif
};




struct text_label {
    struct text_label          *tl_next; /* [0..1][owned] Next case-label, or the next symbol with
                                          *               the same modulated `s_name->k_id' */
    union {
#ifdef __INTELLISENSE__
             struct ast *tl_expr; /* [0..1][valid_if(CHAIN(bs_swcase|s_cases))][const]
                                   *  Expression of a case-label. NOTE: NULL for the default case.
                                   *  NOTE: Always NULL in `bs_swdefl|s_default' labels. */
#else
        DREF struct ast *tl_expr; /* [0..1][valid_if(CHAIN(bs_swcase|s_cases))][const]
                                   *  Expression of a case-label. NOTE: NULL for the default case.
                                   *  NOTE: Always NULL in `bs_swdefl|s_default' labels. */
#endif
        struct TPPKeyword      *tl_name; /* [1..1][valid_if(CHAIN(bs_lbl[*]))][const] Name of this label. */
    };
    struct asm_sym             *tl_asym; /* [0..1] Assembly symbol (lazily allocated) */
    unsigned int                tl_goto; /* The number of times this label is used as a goto target. */
};


struct symbol {
    struct symbol       *s_next;   /* [0..1][owned] Next symbol with the same modulated `s_name->k_id' */
    struct TPPKeyword   *s_name;   /* [1..1][const] Name of this symbol. */
    DeeScopeObject      *s_scope;  /* [1..1][const] The scope declaring this symbol. */
#define SYMBOL_TYPE_NONE   0x0000  /* Undefined symbol type. */
#define SYMBOL_TYPE_GLOBAL 0x0001  /* A global symbol. */
#define SYMBOL_TYPE_EXTERN 0x0002  /* An external symbol. */
#define SYMBOL_TYPE_MODULE 0x0003  /* An import module. */
#define SYMBOL_TYPE_MYMOD  0x0004  /* The current module. */
#define SYMBOL_TYPE_GETSET 0x0005  /* A get/set property symbol. */
#define SYMBOL_TYPE_CATTR  0x0006  /* Class attribute. */
#define SYMBOL_TYPE_ALIAS  0x0007  /* An alias for a different symbol. */
#define SYMBOL_TYPE_ARG    0x0008  /* An argument passed to a function. */
#define SYMBOL_TYPE_LOCAL  0x0009  /* A local symbol. */
#define SYMBOL_TYPE_STACK  0x000a  /* A stack symbol. */
#define SYMBOL_TYPE_STATIC 0x000b  /* A static symbol. */
#define SYMBOL_TYPE_EXCEPT 0x000c  /* The current exception. */
#define SYMBOL_TYPE_MYFUNC 0x000d  /* The current function. */
#define SYMBOL_TYPE_THIS   0x000e  /* The this-argument of a function. */
#define SYMBOL_TYPE_AMBIG  0x000f  /* An ambiguous symbol (caused by `import *' when an overlap occurrs). */
#define SYMBOL_TYPE_FWD    0x0010  /* A forward-defined symbol. */
#define SYMBOL_TYPE_CONST  0x0011  /* A symbol that evaluates to a constant expression. */
#define SYMBOL_TYPE_MAYREF(x)       ((x) >= SYMBOL_TYPE_ARG)
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
                                    * The effective reference ID (RID) of the symbol within the current assembler,
                                    * when the symbol is declared in a different base-scope, and must be
                                    * used in order to construct an inner function making use of it. */
    struct ast_loc       s_decl;   /* [OVERRIDE(.l_file,REF(TPPFile_Decref) [0..1])]
                                    * The source location first referencing where the symbol. */
    uint32_t             s_nread;  /* Number of times the symbol is read */
    uint32_t             s_nwrite; /* Number of times the symbol is written */
    uint32_t             s_nbound; /* Number of times the symbol is checking for being bound */
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
            struct class_attribute    *a_attr;   /* [1..1] The attribute that is being described. */
            struct symbol             *a_class;  /* [1..1][REF(SYMBOL_NREAD(.))] The class that is defining the symbol. */
            struct symbol             *a_this;   /* [0..1][REF(SYMBOL_NREAD(.))] The instance to which the attribute is bound (NULL when this is a class attribute). */
        }                s_attr;   /* [SYMBOL_TYPE_CATTR] Class / instance attribute */
        struct {
            struct symbol             *gs_get;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as getter. */
            struct symbol             *gs_del;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as delete. */
            struct symbol             *gs_set;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as setter. */
        }                s_getset; /* [SYMBOL_TYPE_GETSET] */
        struct symbol   *s_alias;  /* [SYMBOL_TYPE_ALIAS][1..1] The symbol being aliased.
                                    * NOTE: This symbol may be another alias, however is not allowed to produce a loop */
        struct {
            struct ast_loc             a_decl2;  /* [OVERRIDE(.l_file,REF(TPPFile_Decref) [0..1])]
                                                  * The second declaration location. */
            size_t                     a_declc;  /* Number of additional declaration locations. */
            struct ast_loc            *a_declv;  /* [OVERRIDE(.l_file,REF(TPPFile_Decref) [0..1])]
                                                  * [0..a_declc][owned] Additional declaration locations. */
        }                s_ambig;  /* [SYMBOL_TYPE_AMBIG] */
        DREF DeeObject  *s_const;  /* [SYMBOL_TYPE_CONST] The constant that the symbol evaluates to. */

    };
};


/* Return the alias of a `SYMBOL_TYPE_ALIAS'-typed symbol
 * `x', or `x' itself if it's some other kind of type. */
FORCELOCAL struct symbol *DCALL
SYMBOL_UNWIND_ALIAS(struct symbol *__restrict x) {
 while (x->s_type == SYMBOL_TYPE_ALIAS) {
  ASSERT(x != x->s_alias);
  x = x->s_alias;
 }
 return x;
}

/* Inplace-unwind alias symbol references.
 * -> Same as `x = SYMBOL_UNWIND_ALIAS(x)' */
#define SYMBOL_INPLACE_UNWIND_ALIAS(x) \
do{ \
 if ((x)->s_type == SYMBOL_TYPE_ALIAS) \
     (x) = _priv_symbol_dounwind_alias(x); \
}__WHILE0
FORCELOCAL struct symbol *DCALL
_priv_symbol_dounwind_alias(struct symbol *__restrict x) {
 do {
  ASSERT(x != x->s_alias);
  x = x->s_alias;
 } while (x->s_type == SYMBOL_TYPE_ALIAS);
 return x;
}
FORCELOCAL void DCALL _priv_symbol_incread(struct symbol *__restrict x) { for (;;) { ++x->s_nread; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_incwrite(struct symbol *__restrict x) { for (;;) { ++x->s_nwrite; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_incbound(struct symbol *__restrict x) { for (;;) { ++x->s_nbound; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_decread(struct symbol *__restrict x) { for (;;) { ASSERT(x->s_nread); --x->s_nread; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_decwrite(struct symbol *__restrict x) { for (;;) { ASSERT(x->s_nwrite); --x->s_nwrite; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_decbound(struct symbol *__restrict x) { for (;;) { ASSERT(x->s_nbound); --x->s_nbound; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_addread(struct symbol *__restrict x, uint32_t n) { if (n) for (;;) { x->s_nread += n; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_addwrite(struct symbol *__restrict x, uint32_t n) { if (n) for (;;) { x->s_nwrite += n; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_addbound(struct symbol *__restrict x, uint32_t n) { if (n) for (;;) { x->s_nbound += n; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_subread(struct symbol *__restrict x, uint32_t n) { if (n) for (;;) { ASSERT(x->s_nread >= n); x->s_nread -= n; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_subwrite(struct symbol *__restrict x, uint32_t n) { if (n) for (;;) { ASSERT(x->s_nwrite >= n); x->s_nwrite -= n; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }
FORCELOCAL void DCALL _priv_symbol_subbound(struct symbol *__restrict x, uint32_t n) { if (n) for (;;) { ASSERT(x->s_nbound >= n); x->s_nbound -= n; if (x->s_type != SYMBOL_TYPE_ALIAS) break; x = x->s_alias; } }


/* Return the name of a given symbol `x' as a `char *' pointer. */
#define SYMBOL_NAME(x)             ((x)->s_name->k_name)

/* Get/inc/dec the read-, write- and bound- access counters. */
#define SYMBOL_NREAD(x)            ((uint32_t const)(x)->s_nread)
#define SYMBOL_NWRITE(x)           ((uint32_t const)(x)->s_nwrite)
#define SYMBOL_NBOUND(x)           ((uint32_t const)(x)->s_nbound)
#define SYMBOL_INC_NREAD(x)         _priv_symbol_incread(x)
#define SYMBOL_INC_NWRITE(x)        _priv_symbol_incwrite(x)
#define SYMBOL_INC_NBOUND(x)        _priv_symbol_incbound(x)
#define SYMBOL_DEC_NREAD(x)         _priv_symbol_decread(x)
#define SYMBOL_DEC_NWRITE(x)        _priv_symbol_decwrite(x)
#define SYMBOL_DEC_NBOUND(x)        _priv_symbol_decbound(x)
#define SYMBOL_ADD_NREAD(x,n)       _priv_symbol_addread(x,n)
#define SYMBOL_ADD_NWRITE(x,n)      _priv_symbol_addwrite(x,n)
#define SYMBOL_ADD_NBOUND(x,n)      _priv_symbol_addbound(x,n)
#define SYMBOL_SUB_NREAD(x,n)       _priv_symbol_subread(x,n)
#define SYMBOL_SUB_NWRITE(x,n)      _priv_symbol_subwrite(x,n)
#define SYMBOL_SUB_NBOUND(x,n)      _priv_symbol_subbound(x,n)

/* Mark the given symbol `x' as in-use, turning a weakly
 * linked symbol into one that is strongly linked.
 * -> Weakly linked symbols can be re-declared retroactively, with
 *    the act of doing so not causing any compiler warnings. */
#define SYMBOL_MARK_USED(x)        (void)((x)->s_flag &= ~SYMBOL_FWEAK)

/* Check if a given symbol `x' has been declared as a weak symbol. */
#define SYMBOL_IS_WEAK(x)          ((x)->s_flag & SYMBOL_FWEAK)

/* Clear the linkage of a given symbol `x', leaving `x->s_type',
 * as well as all type-specific fields undefined. */
#define SYMBOL_CLEAR_WEAK(x) \
       (symbol_fini(x),(x)->s_flag &= ~SYMBOL_FWEAK)

/* Check if a given symbol `x' must be addressed as a reference */
#define SYMBOL_MUST_REFERENCE(x) \
   (SYMBOL_TYPE_MAYREF((x)->s_type) && \
   ((x)->s_type == SYMBOL_TYPE_THIS ? \
    (x) != current_basescope->bs_this : \
    (x)->s_scope->s_base != current_basescope))

#define SYMBOL_MUST_REFERENCE_THIS(x) \
   (ASSERT((x)->s_type == SYMBOL_TYPE_THIS), \
    (x) != current_basescope->bs_this)

/* Same as `SYMBOL_MUST_REFERENCE()', but the caller already knows
 * that the symbol's type may be referenced (`SYMBOL_TYPE_MAYREF(x->s_type) == true') */
#define SYMBOL_MUST_REFERENCE_TYPEMAY(x) \
   (ASSERT(SYMBOL_TYPE_MAYREF((x)->s_type)), \
   (x)->s_type == SYMBOL_TYPE_THIS ? \
   (x) != current_basescope->bs_this : \
   (x)->s_scope->s_base != current_basescope)

#define SYMBOL_MUST_REFERENCE_NOTTHIS(x) \
   (ASSERT(SYMBOL_TYPE_MAYREF((x)->s_type) && (x)->s_type != SYMBOL_TYPE_THIS), \
   (x)->s_scope->s_base != current_basescope)

/* Check if a given symbol `x' can be addressed as a reference */
#define SYMBOL_MAY_REFERENCE(x) \
   ((x)->s_type == SYMBOL_TYPE_THIS ? \
    (x) != current_basescope->bs_this : \
    (x)->s_scope->s_base != current_basescope)

#define SYMBOL_ALIAS(x)            ((x)->s_alias)           /* XXX: Remove me? */
#define SYMBOL_SCOPE(x)            ((x)->s_scope)           /* XXX: Remove me? */
#define SYMBOL_TYPE(x)             ((x)->s_type)            /* XXX: Remove me? */
#define SYMBOL_FIELD_CLASS(x)      ((x)->s_attr.a_class)    /* XXX: Remove me? */
#define SYMBOL_FIELD_ATTR(x)       ((x)->s_attr.a_attr)     /* XXX: Remove me? */
#define SYMBOL_EXTERN_MODULE(x)    ((x)->s_extern.e_module) /* XXX: Remove me? */
#define SYMBOL_EXTERN_SYMBOL(x)    ((x)->s_extern.e_symbol) /* XXX: Remove me? */
#define SYMBOL_MODULE_MODULE(x)    ((x)->s_module)          /* XXX: Remove me? */
#define SYMBOL_STACK_OFFSET(x)     ((x)->s_symid)           /* XXX: Remove me? */

/* Finalize the given symbol. */
INTDEF void DCALL symbol_fini(struct symbol *__restrict self);

/* Add a 3rd, 4th, etc. ambiguity location to a given symbol.
 * When `loc' is NULL, the current location is used. */
INTDEF void DCALL symbol_addambig(struct symbol *__restrict self,
                                  struct ast_loc *loc);

/* Check if `self' uses `other' when the specified operation is performed. */
INTDEF bool DCALL symbol_uses_symbol_on_get(struct symbol *__restrict self, struct symbol *__restrict other);
INTDEF bool DCALL symbol_uses_symbol_on_del(struct symbol *__restrict self, struct symbol *__restrict other);
INTDEF bool DCALL symbol_uses_symbol_on_set(struct symbol *__restrict self, struct symbol *__restrict other);
#define symbol_uses_symbol_on_bnd(self,other) symbol_uses_symbol_on_get(self,other)

/* Check if reading from, or checking the given symbol for being bound has side-effects.
 * Note that UnboundLocal errors (as thrown when accessing an unbound local symbol) are
 * not considered true side-effects for this purpose. */
INTDEF bool DCALL symbol_get_haseffect(struct symbol *__restrict self, DeeScopeObject *__restrict caller_scope);
#define symbol_bnd_haseffect(self,caller_scope) symbol_get_haseffect(self,caller_scope)
#define symbol_del_haseffect(self,caller_scope) symbol_get_haseffect(self,caller_scope)
#define symbol_set_haseffect(self,caller_scope) symbol_get_haseffect(self,caller_scope)
#define CONFIG_SYMBOL_BND_HASEFFECT_IS_SYMBOL_GET_HASEFFECT 1
#define CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT 1

/* Check if the given symbol `self' is reachable from the given `caller_scope' */
INTDEF bool DCALL symbol_reachable(struct symbol *__restrict self, DeeScopeObject *__restrict caller_scope);


#ifdef CONFIG_BUILDING_DEEMON
INTDEF char const symclass_names[0x1f + 1][8];
#define SYMBOL_TYPE_NAME(cls) symclass_names[(cls)&0x1f]
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
    DeeClassScopeObject     *s_class; /* [0..1][const] A pointer to the nearest class scope, or NULL if outside of any. */
    struct symbol          **s_map;   /* [0..1][owned][0..s_mapa][owned] Hash-map of symbols defined in this scope.
                                       * HINT: Use the TPP keyword id modulated by `s_mapa' as index. */
    size_t                   s_mapc;  /* Amount of symbols defined within the hash-map `s_map'. */
    size_t                   s_mapa;  /* Allocated vector size of the symbol hash-map `s_map'. */
    struct symbol           *s_del;   /* [0..1][owned] Chain of symbols that have been deleted. (And thereby made invisible) */
#define SCOPE_FNORMAL        0x0000   /* Normal scope flags. */
#define SCOPE_FCLASS         0x0001   /* Class scope. */
    uint16_t                 s_flags; /* Scope flags (Set of `SCOPE_F*'). */
#ifndef NDEBUG
    uint16_t                 s_old_stack; /* Used by stack alignment assertions during assembly: The stack depth when the scope was entered. */
#if __SIZEOF_POINTER__ > 4
    uint16_t                 s_pad[(sizeof(void *)/2)-2];
#endif
#else
    uint16_t                 s_pad[(sizeof(void *)/2)-1];
#endif
};


struct class_scope_object {
    DeeScopeObject      cs_scope; /* Underlying regular scope.
                                   * This is also where all of the class's members are defined. */
    struct symbol      *cs_class; /* [0..1] A symbol describing the class being described. */
    struct symbol      *cs_super; /* [0..1] A symbol describing the new class's base-class. */
    struct symbol      *cs_this;  /* [1..1][owned] A symbol describing the this-argument of instances of the class. */
};

/* Returns a pointer to the previous class scope, or `NULL' if no such scope exists. */
#define DeeClassScope_Prev(x) ((x)->cs_scope.s_prev ? (x)->cs_scope.s_prev->s_class : NULL)



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
    struct symbol      *bs_this;       /* [0..1] The `cs_this' of the class which this base-scope's function is implementing a member function for. */
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
                                        * [[*]->s_class == SYMBOL_TYPE_ARG] All symbols referenced _MUST_ be argument symbols.
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
#define DeeBaseScope_IsArgOptional(self,x)   ((x) >= (self)->bs_argc_max && (x) < (uint16_t)((self)->bs_argc_max+(self)->bs_argc_opt))
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

#ifdef CONFIG_BUILDING_DEEMON
INTDEF DeeTypeObject DeeScope_Type;
INTDEF DeeTypeObject DeeClassScope_Type;
INTDEF DeeTypeObject DeeBaseScope_Type;
INTDEF DeeTypeObject DeeRootScope_Type;

INTDEF DREF DeeScopeObject *current_scope;     /* [lock(DeeCompiler_Lock)][1..1] The current scope. */
INTDEF DeeBaseScopeObject  *current_basescope; /* [lock(DeeCompiler_Lock)][1..1][== current_scope->s_base] The current base scope. */
INTDEF DeeRootScopeObject  *current_rootscope; /* [lock(DeeCompiler_Lock)][1..1][== current_basescope->bs_root] The current root scope. */

/* Begin/end a new scope.
 * NOTE: The caller should then fill in special information in `current_scope'. */
INTDEF int (DCALL scope_push)(void);
INTDEF void DCALL scope_pop(void);

/* Enter a new class-scope. */
INTDEF int (DCALL classscope_push)(void);
INTDEF struct symbol *(DCALL get_current_this)(void);

/* Begin/end a new base-scope.
 * NOTE: The caller should then fill in special information in `current_basescope'. */
INTDEF int (DCALL basescope_push)(void);
INTDEF void DCALL basescope_pop(void);
INTDEF void DCALL basescope_push_ob(DeeBaseScopeObject *__restrict scope);

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
INTDEF struct text_label *DCALL new_case_label(struct ast *__restrict expr);
/* Ensure existence and return the default label of a switch-statement.
 * NOTE: The caller is responsible to ensure that the `BASESCOPE_FSWITCH' flag is set.
 *       Additionally (if this is desired), it is the caller's task to warn if the
 *       default case had already been allocated before. - This function's only
 *       purpose is to lazily allocate a missing default case and initialize it. */
INTDEF struct text_label *DCALL new_default_label(void);


/* Lookup a symbol in the given scope. */
INTDEF struct symbol *DCALL
scope_lookup(DeeScopeObject *__restrict scope,
             struct TPPKeyword *__restrict name);
INTDEF struct symbol *DCALL
scope_lookup_str(DeeScopeObject *__restrict scope,
                 char const *__restrict name,
                 size_t name_length);

/* Copy argument symbols from the given `other' base-scope into the current,
 * alongside defining them as symbols while duplicating default-values and the
 * var-args flag. - Basically, everything that may be inferred from an argument list.
 * This is done when creating superargs class operators. */
INTDEF int DCALL copy_argument_symbols(DeeBaseScopeObject *__restrict other);

/* Fully link all forward-defined symbols inside of a class scope. */
INTDEF int DCALL link_forward_symbols(void);

/* Allocate and return a new symbol private to the current scope.
 * NOTE: The caller must ensure that no variable with the given name already exist.
 *       Should the symbol already exist, then there will be two defined afterwards,
 *       only one of which will actually be addressable.
 * NOTE: The caller is also required to initialize the returned
 *       symbol, who's class is undefined up until that point. */
INTDEF struct symbol *DCALL new_local_symbol(struct TPPKeyword *__restrict name, struct ast_loc *loc);
INTDEF struct symbol *DCALL get_local_symbol(struct TPPKeyword *__restrict name);
#define has_local_symbol(name) (get_local_symbol(name) != NULL)
/* Delete a given local symbol, making it anonymous.
 * NOTE: The given `sym' doesn't necessarily need to be apart of the current scope. */
INTDEF void DCALL del_local_symbol(struct symbol *__restrict sym);
/* Create a new unnamed (aka. deleted) symbol. */
INTDEF struct symbol *DCALL new_unnamed_symbol(void);

INTDEF struct symbol *DCALL new_unnamed_symbol_in_scope(DeeScopeObject *__restrict scope);
INTDEF struct symbol *DCALL new_local_symbol_in_scope(DeeScopeObject *__restrict scope, struct TPPKeyword *__restrict name, struct ast_loc *loc);
INTDEF struct symbol *DCALL get_local_symbol_in_scope(DeeScopeObject *__restrict scope, struct TPPKeyword *__restrict name);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define scope_push()      __builtin_expect(scope_push(),0)
#define basescope_push()  __builtin_expect(basescope_push(),0)
#endif
#endif

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_SYMBOL_H */
