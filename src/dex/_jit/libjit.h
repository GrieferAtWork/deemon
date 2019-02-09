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
#ifndef GUARD_DEX_JIT_LIBJIT_H
#define GUARD_DEX_JIT_LIBJIT_H 1

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/object.h>
#include <deemon/util/cache.h>
#include <hybrid/typecore.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/recursive-rwlock.h>
#endif

DECL_BEGIN

/* JIT Token ID overrides. */
#define JIT_KEYWORD    TOK_KEYWORD_BEGIN
#define JIT_STRING     TOK_STRING
#define JIT_RAWSTRING  TOK_CHAR

enum {
    /* Special tokens. */
    TOK_EOF       = '\0', /* END-OF-FILE (will always be ZERO) */
    TOK_CHAR      = '\'', /* 'f'. */
    TOK_STRING    = '\"', /* "foobar". (also includes `r"foobar"' when `TPP_CONFIG_RAW_STRING_LITERALS' is enabled) */
    TOK_INT       = '0',  /* 42 */
    TOK_FLOAT     = 'f',  /* 42.0 */
/*  TOK_LF        = '\n', */
/*  TOK_SPACE     = ' ', */
/*  TOK_COMMENT   = 'c',  /* like this one! */

    /* Single-character tokens (always equal to that character's ordinal). */
    TOK_ADD       = '+',
    TOK_AND       = '&',
    TOK_ASSIGN    = '=',
    TOK_AT        = '@',
    TOK_BACKSLASH = '\\',
    TOK_COLLON    = ':',
    TOK_COMMA     = ',',
    TOK_DIV       = '/',
    TOK_DOT       = '.',
    TOK_HASH      = '#',
    TOK_LANGLE    = '<',
    TOK_LBRACE    = '{',
    TOK_LBRACKET  = '[',
    TOK_LPAREN    = '(',
    TOK_MOD       = '%',
    TOK_MUL       = '*',
    TOK_NOT       = '!',
    TOK_OR        = '|',
    TOK_QUESTION  = '?',
    TOK_RANGLE    = '>',
    TOK_RBRACE    = '}',
    TOK_RBRACKET  = ']',
    TOK_RPAREN    = ')',
    TOK_SEMICOLON = ';',
    TOK_SUB       = '-',
    TOK_TILDE     = '~',
    TOK_XOR       = '^',

    /* Double (or longer) tokens. */
    TOK_TWOCHAR_BEGIN = 256,
    TOK_SHL = TOK_TWOCHAR_BEGIN, /* "<<". */
    TOK_SHR,           /* ">>". */
    TOK_EQUAL,         /* "==". */
    TOK_NOT_EQUAL,     /* "!=". */
    TOK_GREATER_EQUAL, /* ">=". */
    TOK_LOWER_EQUAL,   /* "<=". */
    TOK_DOTS,          /* "...". */
    TOK_ADD_EQUAL,     /* "+=". */
    TOK_SUB_EQUAL,     /* "-=". */
    TOK_MUL_EQUAL,     /* "*=". */
    TOK_DIV_EQUAL,     /* "/=". */
    TOK_MOD_EQUAL,     /* "%=". */
    TOK_SHL_EQUAL,     /* "<<=". */
    TOK_SHR_EQUAL,     /* ">>=". */
    TOK_AND_EQUAL,     /* "&=". */
    TOK_OR_EQUAL,      /* "|=". */
    TOK_XOR_EQUAL,     /* "^=". */
    TOK_POW_EQUAL,     /* "**=". */
/*  TOK_AT_EQUAL,      /* "@=". */
/*  TOK_GLUE,          /* "##". */
    TOK_LAND,          /* "&&". */
    TOK_LOR,           /* "||". */
/*  TOK_LXOR,          /* "^^". */
    TOK_INC,           /* "++". */
    TOK_DEC,           /* "--". */
    TOK_POW,           /* "**". */
/*  TOK_TILDE_TILDE,   /* "~~". */
    TOK_ARROW,         /* "->". */
    TOK_COLLON_EQUAL,  /* ":=". */
/*  TOK_NAMESPACE,     /* "::". */
/*  TOK_ARROW_STAR,    /* "->*". */
/*  TOK_DOT_STAR,      /* ".*". */
/*  TOK_DOTDOT,        /* "..". */
/*  TOK_LOGT,          /* "<>". */
/*  TOK_LANGLE3,       /* "<<<". */
/*  TOK_RANGLE3,       /* ">>>". */
/*  TOK_LANGLE3_EQUAL, /* "<<<=". */
/*  TOK_RANGLE3_EQUAL, /* ">>>=". */
    TOK_EQUAL3,        /* "===". */
    TOK_NOT_EQUAL3,    /* "!==". */
    TOK_KEYWORD_BEGIN, /* KEEP THIS THE LAST TOKEN! */

    TOK_TWOCHAR_END = TOK_KEYWORD_BEGIN,

    /* Name aliases */
    TOK_POS           = TOK_ADD,
    TOK_NEG           = TOK_SUB,
    TOK_LOWER         = TOK_LANGLE,
    TOK_GREATER       = TOK_RANGLE,
/*  TOK_COLLON_COLLON = TOK_NAMESPACE, */
/*  TOK_LOWER_GREATER = TOK_LOGT, */
/*  TOK_LANGLE_RANGLE = TOK_LOGT, */
    TOK_LANGLE1       = TOK_LANGLE,
    TOK_LANGLE2       = TOK_SHL,
    TOK_LANGLE_EQUAL  = TOK_LOWER_EQUAL,
    TOK_LANGLE1_EQUAL = TOK_LOWER_EQUAL,
    TOK_LANGLE2_EQUAL = TOK_SHL_EQUAL,
    TOK_RANGLE1       = TOK_RANGLE,
    TOK_RANGLE2       = TOK_SHR,
    TOK_RANGLE_EQUAL  = TOK_GREATER_EQUAL,
    TOK_RANGLE1_EQUAL = TOK_GREATER_EQUAL,
    TOK_RANGLE2_EQUAL = TOK_SHR_EQUAL,
};


typedef struct jit_small_lexer JITSmallLexer;
typedef struct jit_symbol JITSymbol;
typedef struct jit_lvalue JITLValue;
typedef struct jit_lvalue_list JITLValueList;
typedef struct jit_lexer JITLexer;
typedef struct jit_context JITContext;
typedef struct jit_object_table JITObjectTable;
typedef struct jit_function_object JITFunctionObject;
typedef struct jit_yield_function_object JITYieldFunctionObject;
typedef struct jit_yield_function_iterator_object JITYieldFunctionIteratorObject;


/* Check if the given token qualifies for the associated operation parser function. */
#define TOKEN_IS_UNARY(self)  \
      ((self)->jl_tok == '.' || (self)->jl_tok == '(' || \
       (self)->jl_tok == '[' || (self)->jl_tok == '{' || \
       (self)->jl_tok == TOK_INC || (self)->jl_tok == TOK_DEC || \
        JITLexer_ISKWD(self,"pack"))
#define TOKEN_IS_PROD(self)   ((self)->jl_tok == '*' || (self)->jl_tok == '/' || (self)->jl_tok == '%' || (self)->jl_tok == TOK_POW)
#define TOKEN_IS_SUM(self)    ((self)->jl_tok == '+' || (self)->jl_tok == '-')
#define TOKEN_IS_SHIFT(self)  ((self)->jl_tok == TOK_SHL || (self)->jl_tok == TOK_SHR)
#define TOKEN_IS_CMP(self)    ((self)->jl_tok == TOK_LOWER || (self)->jl_tok == TOK_LOWER_EQUAL || (self)->jl_tok == TOK_GREATER || (self)->jl_tok == TOK_GREATER_EQUAL)
#define TOKEN_IS_CMPEQ(self)  \
      ((self)->jl_tok == TOK_EQUAL || (self)->jl_tok == TOK_NOT_EQUAL || \
       (self)->jl_tok == TOK_EQUAL3 || (self)->jl_tok == TOK_NOT_EQUAL3 || \
       (self)->jl_tok == '!' || \
      ((self)->jl_tok == JIT_KEYWORD && \
       (JITLexer_ISTOK(self,"is") || JITLexer_ISTOK(self,"in"))))
#define TOKEN_IS_AND(self)    ((self)->jl_tok == '&')
#define TOKEN_IS_XOR(self)    ((self)->jl_tok == '^')
#define TOKEN_IS_OR(self)     ((self)->jl_tok == '|')
#define TOKEN_IS_AS(self)     ((self)->jl_tok == JIT_KEYWORD && JITLexer_ISTOK(self,"as"))
#define TOKEN_IS_LAND(self)   ((self)->jl_tok == TOK_LAND)
#define TOKEN_IS_LOR(self)    ((self)->jl_tok == TOK_LOR)
#define TOKEN_IS_COND(self)   ((self)->jl_tok == '?')
#define TOKEN_IS_ASSIGN(self) ((self)->jl_tok == TOK_COLLON_EQUAL || ((self)->jl_tok >= TOK_ADD_EQUAL && (self)->jl_tok <= TOK_POW_EQUAL))

#define CASE_TOKEN_IS_UNARY  case '.': case '(': case '[': case '{': case TOK_INC: case TOK_DEC
#define CASE_TOKEN_IS_PROD   case '*': case '/': case '%': case TOK_POW
#define CASE_TOKEN_IS_SUM    case '+': case '-'
#define CASE_TOKEN_IS_SHIFT  case TOK_SHL: case TOK_SHR
#define CASE_TOKEN_IS_CMP    case TOK_LOWER: case TOK_LOWER_EQUAL: case TOK_GREATER: case TOK_GREATER_EQUAL
#define CASE_TOKEN_IS_CMPEQ  case TOK_EQUAL: case TOK_NOT_EQUAL: case TOK_EQUAL3: case TOK_NOT_EQUAL3: case '!'
#define CASE_TOKEN_IS_AND    case '&'
#define CASE_TOKEN_IS_XOR    case '^'
#define CASE_TOKEN_IS_OR     case '|'
#define CASE_TOKEN_IS_LAND   case TOK_LAND
#define CASE_TOKEN_IS_LOR    case TOK_LOR
#define CASE_TOKEN_IS_COND   case '?'
#define CASE_TOKEN_IS_ASSIGN case TOK_COLLON_EQUAL: case TOK_ADD_EQUAL: case TOK_SUB_EQUAL: case TOK_MUL_EQUAL: case TOK_DIV_EQUAL: case TOK_MOD_EQUAL: \
                             case TOK_SHL_EQUAL: case TOK_SHR_EQUAL: case TOK_AND_EQUAL: case TOK_OR_EQUAL: case TOK_XOR_EQUAL: case TOK_POW_EQUAL




struct jit_object_entry;

#define JIT_SYMBOL_NONE      0x0000 /* No symbol (unused) */
#define JIT_SYMBOL_POINTER   0x0001 /* Pointer to an object reference (used to describe local & inherited variables) */
#define JIT_SYMBOL_OBJENT    0x0002 /* Object table entry. */
#define JIT_SYMBOL_EXTERN    0x0003 /* External symbol reference */
#define JIT_SYMBOL_GLOBAL    0x0004 /* Try to access an entry inside of `jc_globals' */
#define JIT_SYMBOL_GLOBALSTR 0x0005 /* Same as `JIT_SYMBOL_GLOBAL', but use a string as operand. */
struct jit_symbol {
    uint16_t js_kind;  /* Symbol kind (One of JIT_SYMBOL_*) */
    uint16_t js_pad[(sizeof(void *)-2)/2];
    union {
        DREF DeeObject **js_ptr; /* [0..1][1..1] JIT_SYMBOL_POINTER -- Object pointer. */
        struct {
            struct jit_object_entry *jo_ent;     /* [1..1] The entry, the last time it was loaded. */
            struct jit_object_table *jo_tab;     /* [1..1] The object table in question. */
            /*utf-8*/char const     *jo_namestr; /* [0..jo_namelen] The object name. */
            size_t                   jo_namelen; /* Length of the object name. */
        } js_objent; /* JIT_SYMBOL_OBJENT -- Object table entry. */
        struct {
            DREF DeeModuleObject    *jx_mod; /* [1..1] The module that is being referenced.
                                              * NOTE: Guarantied to be a regular, or a DEX module. */
            struct module_symbol    *jx_sym; /* The symbol that is being accessed. */
        } js_extern; /* JIT_SYMBOL_EXTERN */
        DREF struct string_object   *js_global; /* [1..1] JIT_SYMBOL_GLOBAL user-level global symbol name. */
        struct {
            /*utf-8*/char const     *jg_namestr; /* [0..jg_namelen] The global symbol name. */
            size_t                   jg_namelen; /* Length of the global symbol name. */
            dhash_t                  jg_namehsh; /* Hash for the global symbol name. */
        } js_globalstr; /* JIT_SYMBOL_GLOBALSTR */
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


#define JIT_LVALUE_NONE       JIT_SYMBOL_NONE      /* No l-value */
#define JIT_LVALUE_POINTER    JIT_SYMBOL_POINTER   /* Pointer to an object reference (used to describe local & inherited variables) */
#define JIT_LVALUE_OBJENT     JIT_SYMBOL_OBJENT    /* Object table entry. */
#define JIT_LVALUE_EXTERN     JIT_SYMBOL_EXTERN    /* External symbol reference */
#define JIT_LVALUE_GLOBAL     JIT_SYMBOL_GLOBAL    /* Try to access an entry inside of `jc_globals' */
#define JIT_LVALUE_GLOBALSTR  JIT_SYMBOL_GLOBALSTR /* Same as `JIT_SYMBOL_GLOBAL', but use a string as operand. */
#define JIT_LVALUE_ATTR       0x0100               /* Attribute expression. */
#define JIT_LVALUE_ATTRSTR    0x0101               /* Attribute string expression. */
#define JIT_LVALUE_ITEM       0x0102               /* Item expression. */
#define JIT_LVALUE_RANGE      0x0103               /* Range expression. */
#define JIT_LVALUE_RVALUE     0x0200               /* R-value expression (just a regular, read-only expression, but stored inside of an L-Value descriptor). */
struct jit_lvalue {
    uint16_t lv_kind;  /* L-value kind (One of JIT_LVALUE_*) */
    uint16_t lv_pad[(sizeof(void *)-2)/2];
    union {
        DREF DeeObject **lv_ptr; /* [0..1][1..1] JIT_LVALUE_POINTER -- Object pointer. */
        struct {
            struct jit_object_entry *lo_ent;     /* [1..1] The entry, the last time it was loaded. */
            struct jit_object_table *lo_tab;     /* [1..1] The object table in question. */
            /*utf-8*/char const     *lo_namestr; /* [0..lo_namelen] The object name. */
            size_t                   lo_namesiz; /* Length of the object name. */
        } lv_objent; /* JIT_LVALUE_OBJENT | JIT_LVALUE_ROOBJENT -- Object table entry. */
        struct {
            DREF DeeModuleObject *lx_mod; /* [1..1] The module that is being referenced.
                                           * NOTE: Guarantied to be a regular, or a DEX module. */
            struct module_symbol *lx_sym; /* The symbol that is being accessed. */
        } lv_extern; /* JIT_LVALUE_EXTERN */
        DREF struct string_object   *lv_global; /* [1..1] JIT_LVALUE_GLOBAL user-level global symbol name. */
        struct {
            /*utf-8*/char const     *lg_namestr; /* [0..jg_namelen] The global symbol name. */
            size_t                   lg_namelen; /* Length of the global symbol name. */
            dhash_t                  lg_namehsh; /* Hash of the global symbol name. */
        } lv_globalstr; /* JIT_LVALUE_GLOBALSTR */
        struct {
            DREF DeeObject            *la_base; /* [1..1] Expression base object */
            DREF struct string_object *la_name; /* [1..1] Attribute name object */
        } lv_attr; /* JIT_LVALUE_ATTR */
        struct {
            DREF DeeObject      *la_base; /* [1..1] Expression base object */
            /*utf-8*/char const *la_name; /* [0..la_size] Attribute name. */
            size_t               la_size; /* Length of the attribute name. */
            dhash_t              la_hash; /* Hash of the attribute name. */
        } lv_attrstr; /* JIT_LVALUE_ATTRSTR */
        struct {
            DREF DeeObject *li_base;  /* [1..1] Expression base object */
            DREF DeeObject *li_index; /* [1..1] Item index/key object */
        } lv_item; /* JIT_LVALUE_ITEM */
        struct {
            DREF DeeObject *lr_base;  /* [1..1] Expression base object */
            DREF DeeObject *lr_start; /* [1..1] Range start index object */
            DREF DeeObject *lr_end;   /* [1..1] Range end index object */
        } lv_range; /* JIT_LVALUE_RANGE */
        DREF DeeObject *lv_rvalue;    /* [1..1] JIT_LVALUE_RVALUE */
    };
};

#define JITLValue_Init(self) ((self)->lv_kind = JIT_LVALUE_NONE)

/* Finalize a given L-Value object. */
INTDEF void FCALL JITLValue_Fini(JITLValue *__restrict self);
INTDEF void FCALL JITLValue_Visit(JITLValue *__restrict self, dvisit_t proc, void *arg);

/* Interact with an L-Value
 * NOTE: For all of these, the caller must ensure that `self->lv_kind != JIT_LVALUE_NONE' */
INTDEF int FCALL JITLValue_IsBound(JITLValue *__restrict self, JITContext *__restrict context); /* -1: error; 0: no; 1: yes */
INTDEF DREF DeeObject *FCALL JITLValue_GetValue(JITLValue *__restrict self, JITContext *__restrict context);
INTDEF int FCALL JITLValue_DelValue(JITLValue *__restrict self, JITContext *__restrict context);
INTDEF int FCALL JITLValue_SetValue(JITLValue *__restrict self, JITContext *__restrict context, DeeObject *__restrict value);


struct jit_lvalue_list {
    size_t     ll_size;  /* Number of used entires. */
    size_t     ll_alloc; /* Number of allocated entires. */
    JITLValue *ll_list;  /* [0..ll_size|ALLOC(ll_alloc)][owned] Vector of L-values. */
};

#define JITLVALUELIST_INIT       { 0, 0, NULL }
#define JITLValueList_Init(self) ((self)->ll_size = (self)->ll_alloc = 0,(self)->ll_list = NULL)
#define JITLValueList_CInit(self) (ASSERT((self)->ll_size == 0), \
                                   ASSERT((self)->ll_alloc == 0), \
                                   ASSERT((self)->ll_list == NULL))
/* Finalize the given L-value list. */
INTDEF void DCALL JITLValueList_Fini(JITLValueList *__restrict self);

/* Append the given @value onto @self, returning -1 on error and 0 on success. */
INTDEF int DCALL
JITLValueList_Append(JITLValueList *__restrict self,
                     /*inherit(on_success)*/JITLValue *__restrict value);
/* Append an R-value expression. */
INTDEF int DCALL
JITLValueList_AppendRValue(JITLValueList *__restrict self,
                           DeeObject *__restrict value);

struct objectlist;
/* Copy `self' and append all of the referenced objects to the given object list.
 * NOTE: `self' remains valid after this operation! */
INTDEF int DCALL
JITLValueList_CopyObjects(JITLValueList *__restrict self,
                          struct objectlist *__restrict dst,
                          JITContext *__restrict context);

/* Unpack `values' and assign each of the unpacked values to
 * the proper LValue of at the same position within `self'
 * @return:  0: Success.
 * @return: -1: An error occurred. */
INTDEF int DCALL
JITLValueList_UnpackAssign(JITLValueList *__restrict self,
                           JITContext *__restrict context,
                           DeeObject *__restrict values);


struct jit_small_lexer {
    unsigned int            jl_tok;      /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
    /*utf-8*/unsigned char *jl_tokstart; /* [1..1] Token starting pointer. */
    /*utf-8*/unsigned char *jl_tokend;   /* [1..1] Token end pointer. */
    /*utf-8*/unsigned char *jl_end;      /* [1..1] Input end pointer. */
};

struct jit_lexer {
    unsigned int            jl_tok;       /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
    /*utf-8*/unsigned char *jl_tokstart;  /* [1..1] Token starting pointer. */
    /*utf-8*/unsigned char *jl_tokend;    /* [1..1] Token end pointer. */
    /*utf-8*/unsigned char *jl_end;       /* [1..1] Input end pointer. */
    /*utf-8*/unsigned char *jl_errpos;    /* [0..1] Lexer error position. */
    union {
        struct {
            JITFunctionObject
                           *jl_function;  /* [1..1] The function who's text is being scanned. */
            JITObjectTable *jl_parobtab;  /* [0..1] Pointer to the parent scope's object table. */
#define JIT_SCANDATA_FNORMAL  0x0000      /* Normal scan data flags. */
#define JIT_SCANDATA_FINCHILD 0x0001      /* The scanner is currently processing text of a recursively defined child function. */
#define JIT_SCANDATA_FERROR   0x0002      /* An error occurred. */
            unsigned int    jl_flags;     /* Set of `JIT_SCANDATA_F*' */
        }                   jl_scandata;  /* Data fields used by `JITLexer_Scan*' functions. */
        struct {
            DeeObject      *jl_text;      /* [1..1] The object that owns input text (Usually a string or bytes object)
                                           * For expressions such as ones used to create lambda functions, a reference
                                           * to this object is stored to ensure that child code will not be deallocated. */
            JITContext     *jl_context;   /* [1..1][const] The associated JIT context. */
            JITLValue       jl_lvalue;    /* L-value expression. */
        };
    };
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
    /*utf-8*/char const *oe_namestr; /* [0..oe_namelen] Name of the object
                                      * NOTE: `NULL' indicates an unused/sentinal entry;
                                      *       `ITER_DONE' indicates a deleted entry. */
    size_t               oe_namelen; /* Length of the object name. */
    dhash_t              oe_namehsh; /* Hash of the object name. */
    DREF DeeObject      *oe_value;   /* [0..1] Value associated with this entry (NULL if unbound). */
};
struct jit_object_table_pointer {
    JITObjectTable    *otp_tab; /* [0..1] The table that is being referenced. */
    size_t             otp_ind; /* The number of scopes for which `otp_tab' table is shared.
                                 * `otp_tab' is required to be `NULL' when this field is ZERO(0),
                                 * and modifications may only be made to `otp_tab' when this
                                 * field is ONE(1). */
};
struct jit_object_table {
    /* A small, light-weight table for mapping objects to strings
     * -> Used to represent local variables in functions. */
    size_t                   ot_mask;  /* Allocated hash-mask. */
    size_t                   ot_size;  /* Number of used + deleted entries. */
    size_t                   ot_used;  /* Number of used entries. */
    struct jit_object_entry *ot_list;  /* [1..ot_mask + 1][owned_if(!= jit_empty_object_list)]
                                        * Object table hash-vector. */
    struct jit_object_table_pointer
                             ot_prev;  /* Previous object table (does not affect utility functions, and
                                        * is only used to link between different scoped object tables) */
};

INTDEF struct jit_object_entry jit_empty_object_list[1];

#define JITObjectTable_NEXT(i,perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1),(perturb) >>= 5)

/* Initialize/finalize a given JIT object table. */
#define JITOBJECTTABLE_INIT         { 0, 0, 0, jit_empty_object_list }
#define JITObjectTable_CInit(self)  (ASSERT((self)->ot_mask == 0),ASSERT((self)->ot_size == 0),ASSERT((self)->ot_used == 0),(self)->ot_list = jit_empty_object_list)
#define JITObjectTable_Init(self)   ((self)->ot_mask = (self)->ot_size = (self)->ot_used = 0,(self)->ot_list = jit_empty_object_list)
INTDEF void DCALL JITObjectTable_Fini(JITObjectTable *__restrict self);
INTDEF void DCALL JITObjectTable_Visit(JITObjectTable *__restrict self, dvisit_t proc, void *arg);

/* Allocate/free a JIT object table from cache. */
DECLARE_STRUCT_CACHE(jit_object_table,struct jit_object_table);
#ifndef NDEBUG
#define jit_object_table_alloc() jit_object_table_dbgalloc(__FILE__,__LINE__)
#endif

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
                      /*utf-8*/char const *namestr,
                      size_t namelen, dhash_t namehsh,
                      DeeObject *value,
                      bool override_existing);

/* Delete an existing entry for an object with the given name
 * @return: true:  Successfully deleted the entry, after potentially unbinding an associated object.
 * @return: false: The object table didn't include an entry matching the given name. */
INTDEF bool DCALL
JITObjectTable_Delete(JITObjectTable *__restrict self,
                      /*utf-8*/char const *namestr,
                      size_t namelen, dhash_t namehsh);

/* Lookup a given object within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Could not find an object matching the specified name. (no error was thrown) */
INTDEF struct jit_object_entry *DCALL
JITObjectTable_Lookup(JITObjectTable *__restrict self,
                      /*utf-8*/char const *namestr,
                      size_t namelen, dhash_t namehsh);

/* Lookup or create an entry for a given name within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Failed to create a new entry. (an error _WAS_ thrown) */
INTDEF struct jit_object_entry *DCALL
JITObjectTable_Create(JITObjectTable *__restrict self,
                      /*utf-8*/char const *namestr,
                      size_t namelen, dhash_t namehsh);



/* Special values for `jc_retval' */
#define JITCONTEXT_RETVAL_UNSET       NULL /* unset return value */
#define JITCONTEXT_RETVAL_BREAK     ((DREF DeeObject *)-1) /* Unwind for loop break */
#define JITCONTEXT_RETVAL_CONTINUE  ((DREF DeeObject *)-2) /* Unwind for loop continue */
#define JITCONTEXT_RETVAL_ISSET(x)  ((uintptr_t)(x) < (uintptr_t)-0xf)

#define JITCONTEXT_FNORMAL 0x0000 /* Normal flags. */
#define JITCONTEXT_FSYNERR 0x0001 /* A syntax error occurred that may not be caught. */

struct jit_context {
    DeeModuleObject *jc_impbase;  /* [0..1] Base module used for relative, static imports (such as `foo from .baz.bar')
                                   * When `NULL', code isn't allowed to perform relative imports.
                                   * NOTE: If this isn't a module, JIT itself will throw an error. */
    struct jit_object_table_pointer
                     jc_locals;   /* Local variable table (forms a chain all the way to the previous base-scope) */
    DeeObject       *jc_globals;  /* [0..1] A pre-defined, mapping-like object containing pre-defined globals.
                                   * This object can be passed via the `globals' argument to `exec from deemon'
                                   * When not user-defined, a dict object is created the first time a write happens. */
#ifdef __INTELLISENSE__
    DeeObject       *jc_retval;   /* [0..1] Function return value.
                                   * When this is set to be non-NULL, and one of the `JITLexer_Eval*' functions
                                   * returns `NULL', then the there wasn't actually an error, but an alive return
                                   * statement was encountered, with the pre-existing error-unwind path being re-used
                                   * for propagation of that return value out of the JIT parser. */
#else
    DREF DeeObject  *jc_retval;   /* [0..1] Function return value.
                                   * When this is set to be non-NULL, and one of the `JITLexer_Eval*' functions
                                   * returns `NULL', then the there wasn't actually an error, but an alive return
                                   * statement was encountered, with the pre-existing error-unwind path being re-used
                                   * for propagation of that return value out of the JIT parser. */
#endif
    uint16_t         jc_except;   /* [const] Exception indirection at the start of code. */
    uint16_t         jc_flags;    /* Context flags (Set of `JITCONTEXT_F*') */
};
#define JITCONTEXT_INIT    { NULL, { NULL, 0 }, NULL, NULL, 0, JITCONTEXT_FNORMAL }
#define JITContext_Init(x)   memset(x,0,sizeof(JITContext))
#define JITContext_Fini(x)  (void)0


/* Check if the current scope is the global scope. */
#define JITContext_IsGlobalScope(x) \
      ((x)->jc_locals.otp_ind == 0 || \
      ((x)->jc_locals.otp_ind == 1 && (x)->jc_locals.otp_tab && !(x)->jc_locals.otp_tab->ot_prev.otp_tab))

/* Enter a new locals sub-scope. */
#define JITContext_PushScope(x)     \
 (void)(++(x)->jc_locals.otp_ind)

/* Leave the current locals sub-scope. */
#define JITContext_PopScope(x)      \
 (void)(ASSERT((x)->jc_locals.otp_ind != 0), \
       (--(x)->jc_locals.otp_ind == 0) ? _JITContext_PopLocals(x) : (void)0)
INTDEF void DCALL _JITContext_PopLocals(JITContext *__restrict self);

/* Get a pointer to the first locals object-table for the current scope,
 * either for reading (in which case `NULL' is indicative of an empty scope),
 * or for writing (in which case `NULL' indicates an error) */
#ifdef __INTELLISENSE__
INTDEF JITObjectTable *DCALL JITContext_GetROLocals(JITContext *__restrict self);
INTDEF JITObjectTable *DCALL JITContext_GetRWLocals(JITContext *__restrict self);
#else
#define JITContext_GetROLocals(self) ((self)->jc_locals.otp_tab)
INTDEF JITObjectTable *DCALL JITContext_GetRWLocals(JITContext *__restrict self);
#endif



/* Lookup a given symbol within a specific JIT context
 * @param: mode: Set of `LOOKUP_SYM_*'
 * @return: 0:  The specified symbol was found, and `result' was filled
 * @return: -1: An error occurred. */
INTDEF int FCALL
JITContext_Lookup(JITContext *__restrict self,
                  struct jit_symbol *__restrict result,
                  /*utf-8*/char const *__restrict name,
                  size_t namelen, unsigned int mode);
INTDEF int FCALL
JITContext_LookupNth(JITContext *__restrict self,
                     struct jit_symbol *__restrict result,
                     /*utf-8*/char const *__restrict name,
                     size_t namelen, size_t nth);

#define LOOKUP_SYM_NORMAL    0x0000
#define LOOKUP_SYM_VDEFAULT  0x0000 /* Default visibility. */
#define LOOKUP_SYM_VLOCAL    0x0001 /* Lookup rules when `local' is prefixed. */
#define LOOKUP_SYM_VGLOBAL   0x0002 /* Lookup rules when `global' is prefixed. */
#define LOOKUP_SYM_VMASK     0x0003 /* Mask for visibility options. */
#define LOOKUP_SYM_STATIC    0x0100 /* Create static variables / warn about non-static, existing variables. */
#define LOOKUP_SYM_STACK     0x0200 /* Create stack variables / warn about non-stack, existing variables. */
#define LOOKUP_SYM_ALLOWDECL 0x8000 /* Allow declaration of new variables (HINT: Unless set, warn when new variables are created). */



/* Parse an operator name, as can be found in an `x.operator <NAME>' expression
 * @param: features: Set of `P_OPERATOR_F*'
 * @return: * : One of `OPERATOR_*' or `AST_OPERATOR_*'
 * @return: -1: An error occurred. */
INTDEF int32_t FCALL JITLexer_ParseOperatorName(JITLexer *__restrict self, uint16_t features);
#define P_OPERATOR_FNORMAL 0x0000
#define P_OPERATOR_FCLASS  0x0001 /* Allow class-specific operator names. */
#define P_OPERATOR_FNOFILE 0x0002 /* Don't allow file-specific operator names. */


/* Ambiguous operator codes.
 * The caller should resolved these based on operand count. */
#define AST_OPERATOR_POS_OR_ADD           0xf000 /* `+' */
#define AST_OPERATOR_NEG_OR_SUB           0xf001 /* `-' */
#define AST_OPERATOR_GETITEM_OR_SETITEM   0xf002 /* `[]' */
#define AST_OPERATOR_GETRANGE_OR_SETRANGE 0xf003 /* `[:]' */
#define AST_OPERATOR_GETATTR_OR_SETATTR   0xf004 /* `.' */
#define AST_OPERATOR_MIN                  0xf000
#define AST_OPERATOR_MAX                  0xf004

/* Special class operators. */
#define AST_OPERATOR_FOR                  0xf005 /* `for' */


/* Check if a token `tok_id' may refer to the start of an expression. */
INTDEF bool FCALL JIT_MaybeExpressionBegin(unsigned int tok_id);


/* Return the operator function for `opname', as exported from the `operators' module. */
INTDEF DREF DeeObject *FCALL JIT_GetOperatorFunction(uint16_t opname);

/* JIT-specific evaluation flags (may be optionally or'd with `LOOKUP_SYM_*'). */
#define JITLEXER_EVAL_FNORMAL       0x0000 /* Normal evaluation flags. */
#define JITLEXER_EVAL_FALLOWINPLACE 0x0010 /* Allow inplace operations */
#define JITLEXER_EVAL_FALLOWISBOUND 0x0020 /* Allow  */
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
INTDEF DREF DeeObject *FCALL JITLexer_EvalUnaryOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
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

/* Evaluate/skip an argument list, including keyword labels */
INTDEF DREF DeeObject *FCALL JITLexer_EvalArgumentList(JITLexer *__restrict self, DREF DeeObject **__restrict pkwds);
INTDEF int FCALL JITLexer_SkipArgumentList(JITLexer *__restrict self);

/* Evaluate a keyword label list and return a valid mapping for keywords to values.
 * >> foo(10,20,x: 30, y: 40);
 *                 ^        ^
 *              ^
 *              +--- first_label_name == "x: 30, y...."
 *                   first_label_size == 1 */
INTDEF DREF DeeObject *FCALL
JITLexer_EvalKeywordLabelList(JITLexer *__restrict self,
                              char const *__restrict first_label_name,
                              size_t first_label_size);
INTDEF int FCALL
JITLexer_SkipKeywordLabelList(JITLexer *__restrict self);


INTDEF DREF /*Module*/DeeObject *FCALL JITLexer_EvalModule(JITLexer *__restrict self);
#define JITLexer_SkipModule(self) (JITLexer_ParseModuleName(self,NULL,NULL,NULL) < 0 ? -1 : 0)


/* Parse a comma-separated list of expressions,
 * as well as assignment/inplace expressions.
 * >> foo = 42;             // (foo = (42));
 * >> foo += 42;            // (foo += (42));
 * >> foo,bar = (10,20)...; // (foo,bar = (10,20)...);
 * >> foo,bar = 10;         // (foo,(bar = 10));
 * >> { 10 }                // (list { 10 }); // When `AST_COMMA_ALLOWBRACE' is set
 * >> { "foo": 10 }         // (dict { "foo": 10 }); // When `AST_COMMA_ALLOWBRACE' is set
 * @param: mode:      Set of `AST_COMMA_*'     - What is allowed and when should we pack values.
 * @param: seq_type:  The type of sequence to generate (one of `DeeTuple_Type' or `DeeList_Type')
 *                    When `NULL', evaluate to the last comma-expression.
 * @param: pout_mode: When non-NULL, instead of parsing a `;' when required,
 *                    set to `AST_COMMA_OUT_FNEEDSEMI' indicative of this. */
INTDEF DREF DeeObject *DCALL JITLexer_EvalComma(JITLexer *__restrict self, uint16_t mode, DeeTypeObject *seq_type, uint16_t *pout_mode);
INTDEF int DCALL JITLexer_SkipComma(JITLexer *__restrict self, uint16_t mode, uint16_t *pout_mode);

#define AST_COMMA_NORMAL        0x0000
#define AST_COMMA_FORCEMULTIPLE 0x0001 /* Always pack objects according to `flags' */
#define AST_COMMA_STRICTCOMMA   0x0002 /* Strictly enforce the rule of a `,' being followed by another expression.
                                        * NOTE: When this flag is set, trailing `,' are not parsed, but remain as the active token upon exit. */
#define AST_COMMA_NOSUFFIXKWD   0x0080 /* Don't parse c-style variable declarations for reserved keywords.
                                        * This is required for `else', `catch', `finally', etc.
                                        * >> `try foo catch (...)' (don't interpret as `local catch = foo(...)' when starting with `foo') */
#define AST_COMMA_ALLOWTYPEDECL 0x0800 /* Allow type declaration to be appended to variables, as well as documentation strings to be consumed. */
#define AST_COMMA_ALLOWKWDLIST  0x1000 /* Stop if what a keyword list label is encountered. */
#define AST_COMMA_PARSESINGLE   0x2000 /* Only parse a single expression. */
#define AST_COMMA_PARSESEMI     0x4000 /* Parse a `;' as part of the expression (if a `;' is required). */
#define AST_COMMA_ALLOWVARDECLS 0x8000 /* Allow new variables to be declared. */

#define AST_COMMA_OUT_FNORMAL   0x0000 /* Normal comma output flags. */
#define AST_COMMA_OUT_FNEEDSEMI 0x0001 /* Set if a semicolon is required. */
/* Additional flags only used by the JIT compiler. */
#define AST_COMMA_OUT_FMULTIPLE 0x0010 /* Multiple expressions were parsed. */


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

/* Parse lookup mode modifiers:
 * >> local x = 42;
 *    ^     ^
 */
INTDEF int DCALL JITLexer_ParseLookupMode(JITLexer *__restrict self,
                                          unsigned int *__restrict pmode);




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
INTDEF int FCALL JITLexer_SkipAssign(JITLexer *__restrict self, unsigned int flags); /* NOTE: Also handles inplace operators. */
INTDEF int FCALL JITLexer_SkipUnaryOperand(JITLexer *__restrict self, unsigned int flags);
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

/* Parse any kind of post-expression operand. */
INTDEF DREF DeeObject *FCALL JITLexer_EvalOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF int FCALL JITLexer_SkipOperand(JITLexer *__restrict self, unsigned int flags);

/* Recursively skip a pair of tokens, such as `{' and `}' or `(' and `)'
 * NOTE: Entry is expected to be after the initial instance of `pair_open' */
INTDEF int FCALL
JITLexer_SkipPair(JITLexer *__restrict self,
                  unsigned int pair_open,
                  unsigned int pair_close);

/* Parse, evaluate & execute an expression using JIT
 * @param: flags: Set of `JITLEXER_EVAL_F*' */
#define JITLexer_EvalExpression          JITLexer_EvalAssign
#define JITLexer_SkipExpression          JITLexer_SkipAssign
#define JITLexer_SkipGeneratorExpression JITLexer_SkipAssign


/* Parse a whole statement. */
INTDEF DREF DeeObject *FCALL JITLexer_EvalStatement(JITLexer *__restrict self);
INTDEF int FCALL JITLexer_SkipStatement(JITLexer *__restrict self);

INTDEF DREF DeeObject *FCALL JITLexer_EvalStatementBlock(JITLexer *__restrict self);
INTDEF int FCALL JITLexer_SkipStatementBlock(JITLexer *__restrict self);


/* Parse the mask portion of a catch statement:
 * >> try {
 * >>     throw "Foo";
 * >> } catch (string as s) {
 *             ^            ^
 * >> }
 */
INTDEF int FCALL
JITLexer_ParseCatchMask(JITLexer *__restrict self,
                        DREF DeeTypeObject **__restrict ptypemask,
                        char const **__restrict psymbol_name,
                        size_t *__restrict psymbol_size);

/* Hybrid parsing functions. */
INTDEF DREF DeeObject *FCALL JITLexer_EvalStatementOrBraces(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipStatementOrBraces(JITLexer *__restrict self, unsigned int *pwas_expression);

/* Starting immediatly after a `{' token, parse the items of the brace
 * initializer / expression, before returning ontop of the `}' token. */
INTDEF DREF DeeObject *FCALL JITLexer_EvalBraceItems(JITLexer *__restrict self);
INTDEF int FCALL JITLexer_SkipBraceItems(JITLexer *__restrict self);

/* Parse a statement/expression or automatically parse either.
 * @param: kind:            One of `AST_PARSE_WASEXPR_*'
 * @param: pwas_expression: [OUT] One of `AST_PARSE_WASEXPR_*' */
INTDEF DREF DeeObject *FCALL JITLexer_EvalTry(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalTryHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipTry(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipTryHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalDel(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalDelHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipDel(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipDelHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalIf(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalIfHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipIf(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipIfHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalFor(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalForHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipFor(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipForHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalForeach(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalForeachHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipForeach(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipForeachHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalWhile(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalWhileHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipWhile(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipWhileHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalDo(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalDoHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipDo(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipDoHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalWith(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalWithHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipWith(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipWithHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAssert(JITLexer *__restrict self, bool is_statement);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAssertHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipAssert(JITLexer *__restrict self, bool is_statement);
INTDEF int FCALL JITLexer_SkipAssertHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF DREF DeeObject *FCALL JITLexer_EvalImport(JITLexer *__restrict self, bool is_from_import);
INTDEF DREF DeeObject *FCALL JITLexer_EvalImportHybrid(JITLexer *__restrict self, bool is_from_import, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipImport(JITLexer *__restrict self, bool is_from_import);
INTDEF int FCALL JITLexer_SkipImportHybrid(JITLexer *__restrict self, bool is_from_import, unsigned int *pwas_expression);


/* @param: pwas_expression: When non-NULL, set to one of `AST_PARSE_WASEXPR_*' */
INTDEF DREF DeeObject *FCALL JITLexer_EvalHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);
INTDEF int FCALL JITLexer_SkipHybrid(JITLexer *__restrict self, unsigned int *pwas_expression);

#define AST_PARSE_WASEXPR_NO     0 /* It's a statement. */
#define AST_PARSE_WASEXPR_YES    1 /* It's an expression for sure. */
#define AST_PARSE_WASEXPR_MAYBE  2 /* It could either be an expression, or a statement. */


LOCAL DREF DeeObject *FCALL
JITLexer_EvalHybridSecondary(JITLexer *__restrict self,
                             unsigned int *pwas_expression) {
 DREF DeeObject *result;
 if unlikely(!pwas_expression) {
  result = JITLexer_EvalHybrid(self,pwas_expression);
 } else switch (*pwas_expression) {
 case AST_PARSE_WASEXPR_NO:
  result = JITLexer_EvalStatement(self);
  break;
 case AST_PARSE_WASEXPR_YES:
  result = JITLexer_EvalExpression(self,JITLEXER_EVAL_FNORMAL);
  break;
 case AST_PARSE_WASEXPR_MAYBE:
  result = JITLexer_EvalHybrid(self,pwas_expression);
  break;
 default: __builtin_unreachable();
 }
 return result;
}
LOCAL int FCALL
JITLexer_SkipHybridSecondary(JITLexer *__restrict self,
                             unsigned int *pwas_expression) {
 int result;
 if unlikely(!pwas_expression) {
  result = JITLexer_SkipHybrid(self,pwas_expression);
 } else switch (*pwas_expression) {
 case AST_PARSE_WASEXPR_NO:
  result = JITLexer_SkipStatement(self);
  break;
 case AST_PARSE_WASEXPR_YES:
  result = JITLexer_SkipExpression(self,JITLEXER_EVAL_FNORMAL);
  break;
 case AST_PARSE_WASEXPR_MAYBE:
  result = JITLexer_SkipHybrid(self,pwas_expression);
  break;
 default: __builtin_unreachable();
 }
 return result;
}


/* Wrapper for `JITLexer_EvalExpression()' which
 * automatically unwinds L-value expressions. */
#define JITLexer_SkipRValue(self) \
        JITLexer_SkipExpression(self,JITLEXER_EVAL_FNORMAL)
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


#define JITLexer_SkipRValueDecl(self) \
        JITLexer_SkipComma(self,AST_COMMA_NORMAL | AST_COMMA_ALLOWVARDECLS,NULL)
LOCAL DREF DeeObject *FCALL
JITLexer_EvalRValueDecl(JITLexer *__restrict self) {
 DREF DeeObject *result;
 result = JITLexer_EvalComma(self,
                             AST_COMMA_NORMAL |
                             AST_COMMA_ALLOWVARDECLS,
                             NULL,
                             NULL);
 ASSERT((result == JIT_LVALUE) ==
        (self->jl_lvalue.lv_kind != JIT_LVALUE_NONE));
 if (result == JIT_LVALUE)
     result = JITLexer_PackLValue(self);
 return result;
}






#define JIT_FUNCTION_FNORMAL   0x0000 /* Normal function flags. */
#define JIT_FUNCTION_FRETEXPR  0x0001 /* The function's source text describes an arrow-like return expression */
#define JIT_FUNCTION_FYIELDING 0x0002 /* The function's contains a yield statement. */
struct jit_function_object {
    OBJECT_HEAD
    /*utf-8*/char const    *jf_source_start; /* [1..1][const] Source start pointer. */
    /*utf-8*/char const    *jf_source_end;   /* [1..1][const] Source end pointer. */
    DREF DeeObject         *jf_source;       /* [1..1][const] The object that owns input text. */
    DREF DeeModuleObject   *jf_impbase;      /* [0..1][const] Base module used for relative, static imports (such as `foo from .baz.bar')
                                              * When `NULL', code isn't allowed to perform relative imports. */
    DREF DeeObject         *jf_globals;      /* [0..1] Mapping-like object for global variables. */
    JITObjectTable          jf_args;         /* [const] A template for the arguments accepted by this function.
                                              *  - NULL-values identify optional arguments
                                              * NOTE: The back-pointer of this object table is pointed
                                              *       at `jf_refs', with a use-counter that is `>= 2' */
    JITObjectTable          jf_refs;         /* [const] An object table containing all of the symbols potentially
                                              * referenced by the function at the time of its creation. */
    size_t                 *jf_argv;         /* [0..jf_argc_max][owned][const] Vector of indices into the `jf_args'
                                              * hash-vector, referring to the slots associated with positional
                                              * arguments. */
    size_t                  jf_selfarg;      /* [const] Index for `jf_args', to the self-argument, or (size_t)-1 if the function is anonymous. */
    size_t                  jf_varargs;      /* [const] Index for `jf_args', to the varargs argument, or (size_t)-1 if the function doesn't take a variable amount of arguments. */
    size_t                  jf_varkwds;      /* [const] Index for `jf_args', to the varkwds argument, or (size_t)-1 if the function doesn't take a keyword arguments. */
    uint16_t                jf_argc_min;     /* [const] Minimum amount of required positional arguments. */
    uint16_t                jf_argc_max;     /* [const] Maximum amount of required positional arguments. */
    uint16_t                jf_flags;        /* [const] Function flags (Set of `JIT_FUNCTION_F*'). */
};

INTDEF DeeTypeObject JITFunction_Type;

/* Create a new JIT function object by parsing the specified
 * parameter list, and executing the given source region. */
INTDEF DREF DeeObject *DCALL
JITFunction_New(/*utf-8*/char const *name_start,
                /*utf-8*/char const *name_end,
                /*utf-8*/char const *params_start,
                /*utf-8*/char const *params_end,
                /*utf-8*/char const *__restrict source_start,
                /*utf-8*/char const *__restrict source_end,
                JITObjectTable *parent_object_table,
                DeeObject *__restrict source,
                DeeModuleObject *impbase,
                DeeObject *globals,
                uint16_t flags);

/* Add a new symbol entry for an argument to `self->jf_args'
 * This is similar to using `JITObjectTable_Create()', however
 * when re-hashing, this function will also update indices contained
 * within the `self->jf_argv' vector, as well as the `self->jf_selfarg',
 * `self->jf_varargs' and `self->jf_varkwds' fields. */
INTDEF struct jit_object_entry *DCALL
JITFunction_CreateArgument(JITFunctionObject *__restrict self,
                           /*utf-8*/char const *namestr,
                           size_t namelen);

/* Analyze the contents of an expression/statement for possible references
 * to symbols from surrounding scopes, or the use of `yield'. */
INTDEF void FCALL JITLexer_ScanExpression(JITLexer *__restrict self, bool allow_casts);
INTDEF void FCALL JITLexer_ScanStatement(JITLexer *__restrict self);
INTDEF void DCALL JITLexer_ReferenceKeyword(JITLexer *__restrict self, char const *__restrict name, size_t size);



/* Yield-function support */
struct jit_yield_function_object {
    OBJECT_HEAD
    DREF JITFunctionObject *jy_func;       /* [1..1][const] The underlying regular function object. */
    DREF DeeObject         *jy_kw;         /* [0..1][const] Keyword arguments. */
    size_t                  jy_argc;       /* [const] Number of positional arguments passed. */
    DREF DeeObject         *jy_argv[1024]; /* [1..1][const][jy_argc] Vector of positional arguments. */
};




#define JIT_STATE_KIND_HASSCOPE(x) ((x) >= JIT_STATE_KIND_SCOPE2) /* Check if the given state kind has created a locals-scope */
#define JIT_STATE_KIND_SCOPE    0x0000 /* Simple scope */
#define JIT_STATE_KIND_DOWHILE  0x0001 /* Do-while loop (`do ... while (cond);') */
#define JIT_STATE_KIND_TRY      0x0002 /* try-statement (`try { ... } finally { ... } catch (...) { ... }') */
#define JIT_STATE_KIND_SCOPE2   0x0003 /* [SCOPE] Simple scope (including an associated `JITContext_PopScope()') */
#define JIT_STATE_KIND_FOR      0x0004 /* [SCOPE] For-statement (`for (local i = 0; i < 10; ++i) { ... }') */
#define JIT_STATE_KIND_WHILE    0x0005 /* [SCOPE] While-statement (`while (local item = getitem()) { ... }') */
#define JIT_STATE_KIND_FOREACH  0x0006 /* [SCOPE] Foreach-statement (`for (local x: items) { ... }') */
#define JIT_STATE_KIND_FOREACH2 0x0007 /* [SCOPE] Foreach-statement with multiple targets `for (local x,y: pairs) { ... }') */
#define JIT_STATE_KIND_WITH     0x0008 /* [SCOPE] with-statement (`with (local x = get_value()) { ... }') */
#define JIT_STATE_KIND_SKIPELSE 0x0009 /* [SCOPE] Skip of an else-block if `else' or `elif' is encountered after this block ends.
                                        *         -> This type of state is pushed when an if-expression evalutes to follow the true-branch,
                                        *            in which case the false-branch must be skipped (should it exist) */
/* TODO: States for: `switch' */



#define JIT_STATE_FLAG_BLOCK  0x0000 /* Block-state (popped when the block is terminated by a `}') */
#define JIT_STATE_FLAG_SINGLE 0x0001 /* Single-statement block (popped once the next statement has completed) */

struct jit_state {
    struct jit_state *js_prev; /* [0..1] Previous state. */
    uint16_t          js_kind; /* The state kind (One of `JIT_STATE_KIND_*') */
    uint16_t          js_flag; /* State flags (Set of `JIT_STATE_FLAG_*') */
#if __SIZEOF_POINTER__ > 4
    uint8_t           js_pad[sizeof(void *) - 4]; /* ... */
#endif
    union {
        struct {
            unsigned char *f_loop;  /* [1..1] Pointer to the statement's loop-statement. */
            unsigned char *f_cond;  /* [0..1] Pointer to the statement's cond-expression.
                                     * NOTE: This pointer is lazily initialized! */
        }             js_dowhile;   /* JIT_STATE_KIND_DOWHILE */
        struct {
            unsigned char *f_cond;  /* [0..1] Pointer to the for statement's cond-expression. */
            unsigned char *f_next;  /* [0..1] Pointer to the for statement's next-expression. */
            unsigned char *f_loop;  /* [1..1] Pointer to the for statement's loop-statement. */
        }             js_for;       /* JIT_STATE_KIND_FOR */
        struct {
            unsigned char *f_cond;  /* [1..1] Pointer to the for statement's cond-expression. */
            unsigned char *f_loop;  /* [1..1] Pointer to the for statement's loop-statement. */
        }             js_while;     /* JIT_STATE_KIND_WHILE */
        struct {
            DREF DeeObject *f_iter; /* [1..1] The iterator object. */
            JITLValue       f_elem; /* The iterator target expression lvalue (this is
                                     * where the elements enumerated from `f_iter' go). */
            unsigned char  *f_loop; /* [1..1] Pointer to the foreach statement's loop-statement. */
        }             js_foreach;   /* JIT_STATE_KIND_FOREACH */
        struct {
            DREF DeeObject *f_iter; /* [1..1] The iterator object. */
            JITLValueList   f_elem; /* The iterator target expression lvalues (this is
                                     * where the elements enumerated from `f_iter' go). */
            unsigned char  *f_loop; /* [1..1] Pointer to the foreach statement's loop-statement. */
        }             js_foreach2;  /* JIT_STATE_KIND_FOREACH */
        struct {
            DREF DeeObject *w_obj;  /* [1..1] The with-object on which `operator leave()' is invoked when the scope is left. */
        }             js_with;      /* JIT_STATE_KIND_WITH */
        struct {
            unsigned char *t_guard; /* [1..1] Pointer to the start of the guarded statement block. */
        }             js_try;       /* JIT_STATE_KIND_TRY */
    };
};

INTDEF void DCALL jit_state_fini(struct jit_state *__restrict self);
#define jit_state_alloc()        DeeSlab_MALLOC(struct jit_state)
#define jit_state_free(self)     DeeSlab_FREE(self)
#define jit_state_destroy(self) (jit_state_fini(self),jit_state_free(self))



struct jit_yield_function_iterator_object {
    OBJECT_HEAD  /* GC OBJECT */
#ifndef CONFIG_NO_THREADS
    recursive_rwlock_t           ji_lock;  /* Lock held while executing code of this iterator. */
#endif
    DREF JITYieldFunctionObject *ji_func;  /* [1..1][const] The underlying yield-function. */
    JITLexer                     ji_lex;   /* [OVERRIDE(.jl_text,[const][== ji_func->jy_func->jf_source])]
                                            * [OVERRIDE(.jl_context,[const][== &ji_ctx])]
                                            * [lock(ji_lock)] The associated lexer. */
    JITContext                   ji_ctx;   /* [OVERRIDE(.jc_impbase,[const][== ji_func->jy_func->jf_impbase])]
                                            * [OVERRIDE(.jc_locals.otp_ind,[>= 1])]
                                            * [OVERRIDE(.jc_locals.otp_tab,[owned_if(!= &ji_loc)])]
                                            * [OVERRIDE(.jc_globals,[lock(WRITE_ONCE)]
                                            *                       [if(ji_func->jy_func->jf_globals),[== ji_func->jy_func->jf_globals])]
                                            *           )]
                                            * [lock(ji_lock)] The associated JIT context. */
    JITObjectTable               ji_loc;   /* [OVERRIDE(.ot_prev.otp_ind,[>= 2])]
                                            * [OVERRIDE(.ot_prev.otp_tab,[const][== &ji_func->jy_func->jf_refs])]
                                            * [lock(ji_lock)] The base-level local variable table. */
    struct jit_state            *ji_state; /* [lock(ji_lock)][1..1][owned_if(!= &ji_bstat)]
                                            * The current execution/syntax state. */
    struct jit_state             ji_bstat; /* [const][.js_kind == JIT_STATE_KIND_SCOPE] The base-level execution state. */
};


INTDEF DeeTypeObject JITYieldFunction_Type;
INTDEF DeeTypeObject JITYieldFunctionIterator_Type;



/* RT Exception handlers. */
INTDEF ATTR_COLD int DCALL err_no_active_exception(void);
INTDEF ATTR_COLD int DCALL err_invalid_argc_len(char const *function_name, size_t function_size, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD int DCALL err_unknown_global(DeeObject *__restrict key);
INTDEF ATTR_COLD int DCALL err_unknown_global_str_len(char const *__restrict key, size_t keylen);
INTDEF ATTR_COLD int DCALL err_invalid_unpack_size(DeeObject *__restrict unpack_object, size_t need_size, size_t real_size);
INTDEF ATTR_COLD int DCALL err_invalid_unpack_iter_size(DeeObject *__restrict unpack_object, DeeObject *__restrict unpack_iterator, size_t need_size);

/* Syntax Exception handlers. */
INTDEF ATTR_COLD int FCALL syn_if_expected_lparen_after_if(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_if_expected_rparen_after_if(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_with_expected_lparen_after_with(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_with_expected_rparen_after_with(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_for_expected_lparen_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_for_expected_rparen_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_for_expected_semi1_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_for_expected_semi2_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_for_expected_rparen_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_throw_expected_semi_after_throw(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_yield_expected_semi_after_yield(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_break_expected_semi_after_break(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_continue_expected_semi_after_continue(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_return_expected_semi_after_return(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_foreach_expected_lparen_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_foreach_expected_collon_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_foreach_expected_rparen_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_while_expected_lparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_while_expected_rparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_dowhile_expected_while_after_do(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_dowhile_expected_lparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_dowhile_expected_rparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_dowhile_expected_semi_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_nonempty_string(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_lparen_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_rparen_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_string_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_semi_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_keyword_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_rbracket_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_string_before_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_lparen_before_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_rparen_after_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_asm_expected_keyword_for_label_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_try_expected_lparen_after_catch(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_try_expected_rparen_after_catch(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_brace_expected_rbrace(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_brace_expected_keyword_after_dot(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_brace_expected_equals_after_dot(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_brace_expected_collon_after_key(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_expr_expected_semi_after_expr(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_expr_unexpected_token(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_function_expected_lparen_after_function(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_function_expected_arrow_or_lbrace_after_function(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_call_expected_rparen_after_call(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_bound_cannot_test(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_pack_expected_rparen_after_lparen(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_paren_expected_rparen_after_lparen(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_bracket_expected_rbracket_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_attr_expected_keyword(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_item_expected_rbracket_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_isin_expected_is_or_in_after_exclaim(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_operator_expected_empty_string(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_operator_expected_lbracket_or_dot_after_del(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_operator_unknown_name(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_module_expected_dot_keyword_or_string(JITLexer *__restrict self);
INTDEF ATTR_COLD int FCALL syn_anno_expected_rbracket(JITLexer *__restrict self);


DECL_END

#endif /* !GUARD_DEX_JIT_LIBJIT_H */
