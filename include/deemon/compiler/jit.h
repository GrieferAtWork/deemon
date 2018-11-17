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
#ifndef GUARD_DEEMON_COMPILER_JIT_H
#define GUARD_DEEMON_COMPILER_JIT_H 1

#include "../api.h"

/* Disable JIT when optimizing for size. */
#ifdef __OPTIMIZE_SIZE__
#define CONFIG_NO_JIT 1
#endif

/* The deemon JIT compiler is mainly intended for execution of
 * simple user expressions, with the aim of minimizing the amount
 * of time spent trying to preprocess, parse, assemble, link, etc.
 * user input, and instead do everything at once.
 * To maintain this goal, severe restrictions have to be imposed
 * on the JIT compiler, the most important of which is the absence
 * of the C preprocessor, removing directives, macros and escaped
 * line-feeds. */
#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_JIT
#include "../object.h"
#include "../module.h"
#include "../util/rwlock.h"
#include "lexer.h"
#include "tpp.h"

DECL_BEGIN

/* JIT Token ID overrides. */
#define JIT_KEYWORD    TOK_KEYWORD_BEGIN
#define JIT_STRING     TOK_STRING
#define JIT_RAWSTRING  TOK_CHAR

typedef struct jit_small_lexer JITSmallLexer;
typedef struct jit_symbol JITSymbol;
typedef struct jit_lvalue JITLValue;
typedef struct jit_lexer JITLexer;
typedef struct jit_module JITModule;
typedef struct jit_context JITContext;
typedef struct jit_object_table JITObjectTable;


#undef TOKEN_IS_CMPEQ
#define TOKEN_IS_CMPEQ(self)  \
      ((self)->jl_tok == TOK_EQUAL || (self)->jl_tok == TOK_NOT_EQUAL || \
       (self)->jl_tok == TOK_EQUAL3 || (self)->jl_tok == TOK_NOT_EQUAL3 || \
       (self)->jl_tok == '!' || \
      ((self)->jl_tok == JIT_KEYWORD && \
       (JITLexer_ISTOK(self,"is") || JITLexer_ISTOK(self,"in"))))
#undef CASE_TOKEN_IS_CMPEQ
#define CASE_TOKEN_IS_CMPEQ  \
      case TOK_EQUAL: case TOK_NOT_EQUAL: case TOK_EQUAL3: case TOK_NOT_EQUAL3: case '!'
#undef TOKEN_IS_AS
#define TOKEN_IS_AS(self) \
     ((self)->jl_tok == JIT_KEYWORD && JITLexer_ISTOK(self,"as"))


struct jit_object_entry;

#define JIT_SYMBOL_NONE     0x0000 /* No symbol (unused) */
#define JIT_SYMBOL_POINTER  0x0001 /* Pointer to an object reference (used to describe local & inherited variables) */
#define JIT_SYMBOL_OBJENT   0x0002 /* Object table entry. */
#define JIT_SYMBOL_EXTERN   0x0003 /* External symbol reference */
#define JIT_SYMBOL_USERGLOB 0x0004 /* Try to access an entry inside of `jc_uglobals'
                                    * If the JIT module has been loaded, access one of its elements instead. */
struct jit_symbol {
    uint16_t js_kind;  /* Symbol kind (One of JIT_SYMBOL_*) */
    uint16_t js_pad[(sizeof(void *)-2)/2];
    union {
        DREF DeeObject **js_ptr; /* [0..1][1..1] JIT_SYMBOL_POINTER -- Object pointer. */
        struct {
            struct jit_object_entry *jo_ent;     /* [1..1] The entry, the last time it was loaded. */
            struct jit_object_table *jo_tab;     /* [1..1] The object table in question. */
            unsigned char           *jo_namestr; /* [0..jo_namelen] The object name. */
            size_t                   jo_namelen; /* Length of the object name. */
        } js_objent; /* JIT_SYMBOL_OBJENT -- Object table entry. */
        struct {
            DREF DeeModuleObject *jx_mod; /* [1..1] The module that is being referenced.
                                           * NOTE: Guarantied to be a regular, or a DEX module. */
            struct module_symbol *jx_sym; /* The symbol that is being accessed. */
        } js_extern; /* JIT_SYMBOL_EXTERN | JIT_SYMBOL_ROEXTERN */
        DREF struct string_object *js_uglob_name; /* [1..1] JIT_SYMBOL_USERGLOB user-level global symbol name. */
    };
};
#ifdef __INTELLISENSE__
INTDEF void FCALL JITSymbol_Fini(JITSymbol *__restrict self);
#else
#define JITSymbol_Fini(self) JITLValue_Fini((JITLValue *)(self))
#endif


/* Special value which may be returned by the EVAL functions to indicate
 * that the parsed expression cannot losslessly be represented as an object.
 * When this value is returned, everything required to calculate the underlying
 * value is stored within the the associated length `->jl_lvalue', and
 * `JITLexer_GetLValue()' can be called to discard extended information and
 * acquire the actual expression value. */
#define JIT_LVALUE    ITER_DONE


#define JIT_LVALUE_NONE       JIT_SYMBOL_NONE     /* No l-value */
#define JIT_LVALUE_POINTER    JIT_SYMBOL_POINTER  /* Pointer to an object reference (used to describe local & inherited variables) */
#define JIT_LVALUE_OBJENT     JIT_SYMBOL_OBJENT   /* Object table entry. */
#define JIT_LVALUE_EXTERN     JIT_SYMBOL_EXTERN   /* External symbol reference */
#define JIT_LVALUE_USERGLOB   JIT_SYMBOL_USERGLOB /* Try to access an entry inside of `jc_uglobals'
                                                   * If the JIT module has been loaded, access one of its elements instead. */
#define JIT_LVALUE_ATTR       0x0100              /* Attribute expression. */
#define JIT_LVALUE_ATTR_STR   0x0101              /* Attribute string expression. */
#define JIT_LVALUE_ITEM       0x0102              /* Item expression. */
#define JIT_LVALUE_RANGE      0x0103              /* Range expression. */
#define JIT_LVALUE_EXPAND     0x0104              /* An expand expression (read-only) */
struct jit_lvalue {
    uint16_t lv_kind;  /* L-value kind (One of JIT_LVALUE_*) */
    uint16_t lv_pad[(sizeof(void *)-2)/2];
    union {
        DREF DeeObject **lv_ptr; /* [0..1][1..1] JIT_LVALUE_POINTER -- Object pointer. */
        struct {
            struct jit_object_entry *lo_ent;     /* [1..1] The entry, the last time it was loaded. */
            struct jit_object_table *lo_tab;     /* [1..1] The object table in question. */
            char const              *lo_namestr; /* [0..lo_namelen] The object name. */
            size_t                   lo_namesiz; /* Length of the object name. */
        } js_objent; /* JIT_SYMBOL_OBJENT -- Object table entry. */
        struct {
            DREF DeeModuleObject *lx_mod; /* [1..1] The module that is being referenced.
                                           * NOTE: Guarantied to be a regular, or a DEX module. */
            struct module_symbol *lx_sym; /* The symbol that is being accessed. */
        } lv_extern; /* JIT_LVALUE_EXTERN | JIT_LVALUE_ROEXTERN */
        DREF struct string_object *lv_uglob_name; /* [1..1] JIT_LVALUE_USERGLOB user-level global symbol name. */
        struct {
            DREF DeeObject            *la_base; /* [1..1] Expression base object */
            DREF struct string_object *la_name; /* [1..1] Attribute name object */
        } lv_attr; /* JIT_LVALUE_ATTR */
        struct {
            DREF DeeObject      *la_base; /* [1..1] Expression base object */
            /*utf-8*/char const *la_name; /* [0..la_nsiz] Attribute name. */
            size_t               la_nsiz; /* Length of the attribute name. */
        } lv_attr_str; /* JIT_LVALUE_ATTR_STR */
        struct {
            DREF DeeObject *li_base;  /* [1..1] Expression base object */
            DREF DeeObject *li_index; /* [1..1] Item index/key object */
        } lv_item; /* JIT_LVALUE_ITEM */
        struct {
            DREF DeeObject *lr_base;  /* [1..1] Expression base object */
            DREF DeeObject *lr_start; /* [1..1] Range start index object */
            DREF DeeObject *lr_end;   /* [1..1] Range end index object */
        } lv_range; /* JIT_LVALUE_RANGE */
        DREF DeeObject *lv_expand; /* [1..1] JIT_LVALUE_EXPAND */
    };
};

#define JITLValue_Init(self) ((self)->lv_kind = JIT_LVALUE_NONE)

/* Finalize a given L-Value object. */
INTDEF void FCALL JITLValue_Fini(JITLValue *__restrict self);

/* Interact with an L-Value
 * NOTE: For all of these, the caller must ensure that `self->lv_kind != JIT_LVALUE_NONE' */
INTDEF int FCALL JITLValue_IsBound(JITLValue *__restrict self, JITContext *__restrict context); /* -1: error; 0: no; 1: yes */
INTDEF DREF DeeObject *FCALL JITLValue_GetValue(JITLValue *__restrict self, JITContext *__restrict context);
INTDEF int FCALL JITLValue_DelValue(JITLValue *__restrict self, JITContext *__restrict context);
INTDEF int FCALL JITLValue_SetValue(JITLValue *__restrict self, JITContext *__restrict context, DeeObject *__restrict value);


struct jit_small_lexer {
    unsigned int            jl_tok;      /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
    /*utf-8*/unsigned char *jl_tokstart; /* [1..1] Token starting pointer. */
    /*utf-8*/unsigned char *jl_tokend;   /* [1..1] Token end pointer. */
    /*utf-8*/unsigned char *jl_end;      /* [1..1] Input end pointer. */
};

struct jit_lexer {
    unsigned int            jl_tok;      /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
    /*utf-8*/unsigned char *jl_tokstart; /* [1..1] Token starting pointer. */
    /*utf-8*/unsigned char *jl_tokend;   /* [1..1] Token end pointer. */
    /*utf-8*/unsigned char *jl_end;      /* [1..1] Input end pointer. */
    /*utf-8*/unsigned char *jl_errpos;   /* [0..1] Lexer error position. */
    DeeObject              *jl_text;     /* [1..1] The object that owns input text (Usually a string or bytes object)
                                          * For expressions such as ones used to create lambda functions, a reference
                                          * to this object is stored to ensure that child code will not be deallocated. */
    JITContext             *jl_context;  /* [1..1][const] The associated JIT context. */
    JITLValue               jl_lvalue;   /* L-value expression. */
};

/* Similar to `JITLexer_GetLValue()', but also finalize
 * the stored L-value, and set it to describe nothing.
 * NOTE: The stored L-value is _always_ reset! */
INTDEF DREF DeeObject *DCALL JITLexer_PackLValue(JITLexer *__restrict self);


/* Check if the current token is a keyword `x' */
#define JITLexer_ISKWD(self,x) \
   ((self)->jl_tok == JIT_KEYWORD && \
     COMPILER_STRLEN(x) == ((self)->jl_tokend - (self)->jl_tokstart) && \
     memcmp((self)->jl_tokstart,x,COMPILER_STRLEN(x) * sizeof(char)) == 0)
#define JITLexer_ISTOK(self,x) \
   (COMPILER_STRLEN(x) == ((self)->jl_tokend - (self)->jl_tokstart) && \
    memcmp((self)->jl_tokstart,x,COMPILER_STRLEN(x) * sizeof(char)) == 0)

/* Initialize a given JIT lexer for a given data block, and load the first token. */
#define JITLexer_Start(self,start,end) \
      ((self)->jl_tokend = (start), \
       (self)->jl_end    = (end), \
        JITLexer_Yield(self))


/* Starting at `token->jl_tokend', scan for the next input token
 * NOTE: This function may also be used with `JITSmallLexer' */
INTDEF void FCALL JITLexer_Yield(JITLexer *__restrict self);
#define JITLexer_YieldAt(self,pos) ((self)->jl_tokend = (pos),JITLexer_Yield(self))

/* Remember the fact that an exception was thrown
 * when code at `pos' was being executed. */
#ifdef __INTELLISENSE__
INTDEF void FCALL
JITLexer_ErrorTrace(JITLexer *__restrict self,
                    unsigned char *__restrict pos);
#else
#define JITLexer_ErrorTrace(self,pos) (void)((self)->jl_errpos = (pos))
#endif




struct jit_object_entry {
    DREF DeeObject         *oe_nameobj; /* [0..1] The object that owns the `oe_namestr' string (usually a string or bytes object)
                                         * NOTE: `NULL' indicates an unused/sentinal entry; `ITER_DONE' indicates a deleted entry. */
    /*utf-8*/unsigned char *oe_namestr; /* [0..oe_namelen] Name of the object */
    size_t                  oe_namelen; /* Length of the object name. */
    dhash_t                 oe_namehsh; /* Hash of the object name. */
    DREF DeeObject         *oe_value;   /* [0..1] Value associated with this entry (NULL if unbound). */
};
struct jit_object_table {
    /* A small, light-weight table for mapping objects to strings
     * -> Used to represent local variables in functions. */
    size_t                   ot_mask; /* Allocated hash-mask. */
    size_t                   ot_size; /* Number of used + deleted entries. */
    size_t                   ot_used; /* Number of used entries. */
    struct jit_object_entry *ot_list; /* [1..ot_mask + 1][owned_if(!= jit_empty_object_list)]
                                       * Object table hash-vector. */
    JITObjectTable          *ot_prev; /* [0..1] Previous object table (does not affect utility functions,
                                       * and is only used to link between different scoped object tables) */
};

INTDEF struct jit_object_entry jit_empty_object_list[1];

#define JITObjectTable_NEXT(i,perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1),(perturb) >>= 5)

/* Initialize/finalize a given JIT object table. */
#define JITOBJECTTABLE_INIT         { 0, 0, 0, jit_empty_object_list }
#define JITObjectTable_CInit(self)  (ASSERT((self)->ot_mask == 0),ASSERT((self)->ot_size == 0),ASSERT((self)->ot_used == 0),(self)->ot_list = jit_empty_object_list)
#define JITObjectTable_Init(self)   ((self)->ot_mask = (self)->ot_size = (self)->ot_used = 0,(self)->ot_list = jit_empty_object_list)
INTDEF void DCALL JITObjectTable_Fini(JITObjectTable *__restrict self);

/* Initialize `dst' as a copy of `src' */
INTDEF int DCALL
JITObjectTable_Copy(JITObjectTable *__restrict dst,
                    JITObjectTable const *__restrict src);

/* Insert all elements from `src' into `dst'
 * Existing entires will not be overwritten. */
INTDEF int DCALL
JITObjectTable_UpdateTable(JITObjectTable *__restrict dst,
                           JITObjectTable const *__restrict src);

/* Update an object within the given object table, potentially overwriting an
 * existing object, or creating a new entry if no existing object could be found.
 * @param: value: The value to assign to the entry.
 *                When `NULL', the entry is unbound.
 * @return: 1:  Successfully updated an existing entry when `override_existing' was `true'.
 * @return: 1:  An entry already existed for the given name when `override_existing' was `false'.
 * @return: 0:  Successfully created a new entry.
 * @return: -1: An error occurred (failed to increase the hash size of `self') */
INTDEF int DCALL
JITObjectTable_Update(JITObjectTable *__restrict self,
                      /*utf-8*/unsigned char *namestr,
                      size_t namelen, dhash_t namehsh,
                      DeeObject *__restrict nameobj,
                      DeeObject *value,
                      bool override_existing);

/* Delete an existing entry for an object with the given name
 * @return: true:  Successfully deleted the entry, after potentially unbinding an associated object.
 * @return: false: The object table didn't include an entry matching the given name. */
INTDEF bool DCALL
JITObjectTable_Delete(JITObjectTable *__restrict self,
                      /*utf-8*/unsigned char *namestr,
                      size_t namelen, dhash_t namehsh);

/* Lookup a given object within `self'
 * @return: * :        The value associated with the given name. (NOTE: Not a reference!)
 * @return: NULL:      The object matching the specified name has been unbound. (no error was thrown)
 * @return: ITER_DONE: Could not find an object matching the specified name. (no error was thrown) */
INTDEF DeeObject *DCALL
JITObjectTable_Lookup(JITObjectTable *__restrict self,
                      /*utf-8*/unsigned char *namestr,
                      size_t namelen, dhash_t namehsh);



struct jit_module {
    DeeModuleObject jm_module;     /* The underlying module. */
};

struct jit_context {
    DeeModuleObject *jc_impbase;    /* [0..1] Base module used for relative, static imports (such as `foo from .baz.bar')
                                     * When `NULL', code isn't allowed to perform relative imports. */
#ifdef __INTELLISENSE__
         JITModule  *jc_module;     /* [0..1] The JIT-module descriptor (lazily allocated). */
#else
    DREF JITModule  *jc_module;     /* [0..1] The JIT-module descriptor (lazily allocated). */
#endif
    JITObjectTable  *jc_locals;     /* [0..1] Local variable table (forms a chain all the way to the previous base-scope) */
    DeeObject       *jc_uglobals;   /* [0..1] A pre-defined, mapping-like object containing pre-defined globals.
                                     * This object can be passed via the `globals' argument to `exec from deemon' */

};
#define JITCONTEXT_INIT    { NULL, NULL, NULL, NULL }
#define JITContext_Init(x)   memset(x,0,sizeof(JITContext))
#define JITContext_Fini(x)   Dee_XDecref((DeeObject *)(x)->jc_module)



/* Lookup a given symbol within a specific JIT context
 * @return: true:  The specified symbol was found, and `result' was filled
 * @return: false: The symbol could not be found, and `result' was set to `JIT_SYMBOL_NONE' */
INTDEF bool FCALL
JITContext_Lookup(JITContext *__restrict self,
                  struct jit_symbol *__restrict result,
                  /*utf-8*/char const *__restrict name,
                  size_t namelen);
INTDEF bool FCALL
JITContext_LookupNth(JITContext *__restrict self,
                     struct jit_symbol *__restrict result,
                     /*utf-8*/char const *__restrict name,
                     size_t namelen, size_t nth);



/* Parse an operator name, as can be found in an `x.operator <NAME>' expression
 * @param: features: Set of `P_OPERATOR_F*'
 * @return: * : One of `OPERATOR_*' or `AST_OPERATOR_*'
 * @return: -1: An error occurred. */
INTDEF int32_t FCALL JITLexer_ParseOperatorName(JITLexer *__restrict self, uint16_t features);

/* Return the operator function for `opname', as exported from the `operators' module. */
INTDEF DREF DeeObject *FCALL JIT_GetOperatorFunction(uint16_t opname);

/* JIT-specific evaluation flags. */
#define JITLEXER_EVAL_FNORMAL       0x0000 /* Normal evaluation flags. */
#define JITLEXER_EVAL_FALLOWINPLACE 0x0010 /* Primary expression evaluation */
#define JITLEXER_EVAL_FALLOWISBOUND 0x0020 /* Primary expression evaluation */
#define JITLEXER_EVAL_FDISALLOWCAST 0x0040 /* Disallow cast expressions. */
#define JITLEXER_EVAL_FPRIMARY     (JITLEXER_EVAL_FALLOWISBOUND | JITLEXER_EVAL_FALLOWINPLACE)

INTDEF DREF DeeObject *FCALL JITLexer_EvalUnaryHead(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalUnary(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalProd(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalSum(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalShift(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmp(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmpEQ(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAnd(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalXor(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalOr(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAs(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLand(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLor(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCond(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAssign(JITLexer *__restrict self, unsigned int flags); /* NOTE: Also handled inplace operators. */

/* With the current token one of the unary operator symbols, consume
 * it and parse the second operand before returning the combination */
INTDEF DREF DeeObject *FCALL JITLexer_EvalProdOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalSumOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalShiftOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmpOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmpEQOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAndOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalXorOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalOrOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAsOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLandOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLorOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCondOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAssignOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);

INTDEF DREF DeeObject *FCALL JITLexer_EvalCommaTupleOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCommaTupleOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCommaListOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCommaListOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCommaDictOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCommaDictOperand(JITLexer *__restrict self, unsigned int flags);

INTDEF DREF /*Module*/DeeObject *FCALL JITLexer_EvalModule(JITLexer *__restrict self);
#define JITLexer_SkipModule(self) (JITLexer_ParseModuleName(self,NULL,NULL,NULL) < 0 ? -1 : 0)


/* Parse a module name, either writing it to `*printer' (if non-NULL),
 * or storing the name's start and end pointers in `*pname_start' and
 * `*pname_end'
 * @return:  1: Successfully parsed the module name and stored it in `*printer'
 *              In this case, this function will have also initialized `*printer'
 * @return:  0: Successfully parsed the module name and stored it in `*pname_start' / `*pname_end'
 * @return: -1: An error occurred. */
INTDEF int FCALL
JITLexer_ParseModuleName(JITLexer *__restrict self,
                         struct unicode_printer *printer,
                         /*utf-8*/unsigned char **pname_start,
                         /*utf-8*/unsigned char **pname_end);



/* Skip evaluation functions. (same as the regular functions,
 * but expressions are skipped, rather than being evaluated)
 * However, syntax error are still thrown. */
INTDEF int FCALL JITLexer_SkipUnaryHead(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipUnary(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipProd(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipSum(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipShift(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmp(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmpEQ(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAnd(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipXor(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipOr(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAs(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLor(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCond(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAssign(JITLexer *__restrict self, unsigned int flags); /* NOTE: Also handled inplace operators. */
INTDEF int FCALL JITLexer_SkipProdOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipSumOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipShiftOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmpOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmpEQOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAndOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipXorOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipOrOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAsOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLandOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLorOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCondOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAssignOperand(JITLexer *__restrict self, unsigned int flags);




/* Parse, evaluate & execute an expression using JIT
 * @param: flags: Set of `JITLEXER_EVAL_F*' */
#define JITLexer_EvalExpression JITLexer_EvalAssign
#define JITLexer_SkipExpression JITLexer_SkipAssign

/* Wrapper for `JITLexer_EvalExpression()' which
 * automatically unwinds L-value expressions. */
LOCAL DREF DeeObject *FCALL
JITLexer_EvalRValue(JITLexer *__restrict self) {
 DREF DeeObject *result;
 result = JITLexer_EvalExpression(self,JITLEXER_EVAL_FNORMAL);
 ASSERT((result == JIT_LVALUE) ==
        (self->jl_lvalue.lv_kind != JIT_LVALUE_NONE));
 if (result == JIT_LVALUE)
     result = JITLexer_PackLValue(self);
 return result;
}


DECL_END
#endif /* !CONFIG_NO_JIT */
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_JIT_H */
