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
struct ast_loc;

struct asm_sym;
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

struct symbol {
    struct symbol       *sym_next;      /* [0..1][owned] Next symbol with the same modulated `sym_name->k_id' */
    struct TPPKeyword   *sym_name;      /* [1..1][const] Name of this symbol. */
    DeeScopeObject      *sym_scope;     /* [1..1] The scope declaring this symbol. */
    uint16_t             sym_class;     /* Symbol class. (One of `SYM_CLASS_*')
                                         * This describes how is the variable addressed, and where does it live.
                                         * Sorry, deemon has a lot of these... */
#define SYM_FNORMAL      0x0000         /* Normal symbol flags. */
    uint16_t             sym_flag;      /* Symbol flags (Set of `SYM_F*' dependent on `sym_class') */
#if __SIZEOF_POINTER__ > 4
    uint16_t             sym_pad[2];    /* ... */
#endif
    uint32_t             sym_read;      /* The amount of times that this symbol is read from. */
    uint32_t             sym_write;     /* The amount of times that this symbol is written to. (Excluding an initial assignment)
                                         * HINT: Using this field, the code generator performs constant optimization. */
    union {

        struct {
#define SYM_CLASS_EXTERN               0x0000      /* External symbol. (NOTE: Doesn't use `SYM_FALLOC') */
#   define SYM_FEXTERN_ALLOC           0x8000      /* FLAG: The module index for the variable has been allocated. */
            DREF struct module_object *sym_module; /* [1..1][const] The module exporting this symbol. */
            struct module_symbol      *sym_modsym; /* [1..1] The symbol that is being imported from `sym_module'. */
            uint16_t                   sym_modid;  /* [valid_if(SYM_FEXTERN_ALLOC)] The module's index in the `rs_importv' vector of the current root-scope. */
        }                              sym_extern; /* External variable. */

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

        struct {
#define SYM_CLASS_ARG                  0x0003      /* Argument variable. */
            uint16_t                   sym_index;  /* [const] Virtual argument index. */
        }                              sym_arg;    /* Argument variable. */

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

        struct {
#define SYM_CLASS_MODULE               0x0006      /* Import module reference. */
#   define SYM_FMODULE_ALLOC           0x8000      /* FLAG: The import module index has been assigned. */
            DREF struct module_object *sym_module; /* [1..1] The referenced module. */
            uint16_t                   sym_modid;  /* [valid_if(SYM_FMODULE_ALLOC)] The module's index in the `rs_importv' vector of the current root-scope. */
        }                              sym_module; /* Import module reference. */

        struct {
#define SYM_CLASS_PROPERTY             0x0007      /* Property symbol (invokes callbacks for get/del/set). */
            struct symbol             *sym_get;    /* [0..1] The getter callback. */
            struct symbol             *sym_del;    /* [0..1] The delete callback. */
            struct symbol             *sym_set;    /* [0..1] the setter callback. */
        }                              sym_property; /* Property symbol. */

#define SYM_CLASS_ALIAS                0x0008      /* Alias for another symbol. */
        struct {
            struct symbol             *sym_alias;  /* [1..1][->sym_scope == sym_scope]
                                                    * The symbol being aliased.
                                                    * NOTE: In class scopes, this type of symbol is used
                                                    *       for forward references and as placeholder. */
        }                              sym_alias;

        /* Other symbol classes that reference special symbols. */
#define SYM_CLASS_EXCEPT        0x1009 /* The current exception. */
#define SYM_CLASS_THIS_MODULE   0x900a /* The current module. */
#define SYM_CLASS_THIS_FUNCTION 0x900b /* The function object identified by `s_scope->s_base'. */
#define SYM_CLASS_THIS          0x900c /* The `this' variable in class-member functions (thiscall functions).
                                        * NOTE: This class requires that `s_scope->s_base'
                                        *       have the `SCOPE_FTHISCALL' flag set. */
    };
};



/* Check if a given symbol class allows the symbols to be written. */
#define SYM_WRITABLE(x)       (!((x)->sym_class&0x8000))
/* Symbol classes that must be referenced when not part of the current base-scope. */
#define SYM_MUST_REFERENCE(x) \
  ((x)->sym_class != SYM_CLASS_EXTERN && \
   (x)->sym_class != SYM_CLASS_MODULE && \
  ((x)->sym_class != SYM_CLASS_VAR || \
   (x)->sym_flag  != SYM_FVAR_GLOBAL))


#ifdef CONFIG_BUILDING_DEEMON
INTDEF char const symclass_names[16][14];
#define SYMCLASS_NAME(cls) symclass_names[(cls)&0xf]

/* Return the real symbol given a symbol that may be a reference. */
INTDEF struct symbol *DCALL sym_realsym(struct symbol *__restrict self);
#endif


struct scope_object {
    OBJECT_HEAD
#ifdef __INTELLISENSE__
    DeeScopeObject          *s_prev; /* [0..1][const] Previous scope. */
#else
    DREF DeeScopeObject     *s_prev; /* [0..1][const] Previous scope. */
#endif
    DeeBaseScopeObject      *s_base; /* [1..1][const] The base scope of the surrounding function.
                                      *   HINT: If this is a self-pointer, this scope is actually a `DeeBaseScopeObject'  */
    struct symbol          **s_map;  /* [0..1][owned][0..s_mapa][owned] Hash-map of symbols defined in this scope.
                                      *   HINT: Use the TPP keyword id modulated by `s_mapa' as index. */
    size_t                   s_mapc; /* Amount of symbols defined within the hash-map `s_map'. */
    size_t                   s_mapa; /* Allocated vector size of the symbol hash-map `s_map'. */
    struct symbol           *s_del;  /* [0..1][owned] Chain of symbols that have been deleted. (And thereby made invisible) */
    struct symbol           *s_stk;  /* [0..1][CHAIN(->sym_stack.sym_nstck)] Chain of stack-based symbols. */
};
#define SCOPE_ADD_STACKSYM(self,x) \
 (void)(ASSERT((x)->sym_class == SYM_CLASS_STACK), \
               (x)->sym_flag = SYM_FNORMAL, \
               (x)->sym_stack.sym_nstck = (self)->s_stk, \
               (self)->s_stk = (x))


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
    struct symbol      *bs_refs;       /* [0..1] Chain of references defined for this scope. */
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
#define BASESCOPE_FNORMAL 0x0000
#define BASESCOPE_FRETURN 0x0001    /* A non-empty return statement has been encountered. */
#define BASESCOPE_FSWITCH 0x0002    /* The parser is currently allowed to generate switch-labels. */
    uint16_t            bs_cflags;     /* Compile-time scope flags (Set of `BASESCOPE_F*'). */
#if __SIZEOF_POINTER__ > 4
    uint16_t            bs_padding[(sizeof(void *)/2)-2];
#endif
};


/* TODO: class_scope_object */

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


/* Safely create a reference to a given symbol from `current_basescope'. */
INTDEF struct symbol *DCALL
symbol_reference(DeeBaseScopeObject *__restrict current_basescope,
                 struct symbol *__restrict sym);

/* Lookup a symbol in the given scope. */
INTDEF struct symbol *DCALL
scope_lookup(DeeScopeObject *__restrict scope,
             struct TPPKeyword *__restrict name);


/* Copy argument symbols from the given `other' base-scope into the current,
 * alongside defining them as symbols while duplicating default-values and the
 * var-args flag. - Basically, everything that may be inferred from an argument list.
 * This is done when creating superargs class operators. */
INTDEF int DCALL copy_argument_symbols(DeeBaseScopeObject *__restrict other);

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
