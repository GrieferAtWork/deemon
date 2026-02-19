/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_STREXEC_LIBJIT_H
#define GUARD_DEX_STREXEC_LIBJIT_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_FFree, DeeObject_MALLOC, DeeSlab_FREE, DeeSlab_MALLOC */
#include <deemon/module.h>          /* DeeModuleObject, Dee_module_symbol */
#include <deemon/object.h>          /* DREF, DeeObject, DeeTypeObject, Dee_Decref, Dee_XDecref, Dee_hash_t, ITER_DONE, OBJECT_HEAD */
#include <deemon/string.h>          /* Dee_unicode_printer */
#include <deemon/system-features.h> /* CONFIG_HAVE_FPU, bcmpc, bzero */
#include <deemon/type.h>            /* Dee_XVisit, Dee_operator_t, Dee_visit_t */
#include <deemon/util/rlock.h>      /* Dee_rshared_lock_* */

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* int32_t, uint8_t, uint16_t, uintptr_t */

DECL_BEGIN

struct jit_object_entry;
struct Dee_instance_desc;
struct Dee_class_attribute;
struct Dee_string_object;

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
#ifdef CONFIG_HAVE_FPU
	TOK_FLOAT     = 'f',  /* 42.0 */
#endif /* CONFIG_HAVE_FPU */
/*	TOK_LF        = '\n', */
/*	TOK_SPACE     = ' ', */
/*	TOK_COMMENT   = 'c',  /* like this one! */

	/* Single-character tokens (always equal to that character's ordinal). */
	TOK_ADD       = '+',
	TOK_AND       = '&',
	TOK_ASSIGN    = '=',
	TOK_AT        = '@',
	TOK_BACKSLASH = '\\',
	TOK_COLON     = ':',
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
/*	TOK_AT_EQUAL,      /* "@=". */
/*	TOK_GLUE,          /* "##". */
	TOK_LAND,          /* "&&". */
	TOK_LOR,           /* "||". */
/*	TOK_LXOR,          /* "^^". */
	TOK_INC,           /* "++". */
	TOK_DEC,           /* "--". */
	TOK_POW,           /* "**". */
/*	TOK_TILDE_TILDE,   /* "~~". */
	TOK_ARROW,         /* "->". */
	TOK_COLON_EQUAL,   /* ":=". */
/*	TOK_NAMESPACE,     /* "::". */
/*	TOK_ARROW_STAR,    /* "->*". */
/*	TOK_DOT_STAR,      /* ".*". */
/*	TOK_DOTDOT,        /* "..". */
/*	TOK_LOGT,          /* "<>". */
/*	TOK_LANGLE3,       /* "<<<". */
/*	TOK_RANGLE3,       /* ">>>". */
/*	TOK_LANGLE3_EQUAL, /* "<<<=". */
/*	TOK_RANGLE3_EQUAL, /* ">>>=". */
	TOK_EQUAL3,        /* "===". */
	TOK_NOT_EQUAL3,    /* "!==". */
	TOK_QMARK_QMARK,   /* "??". */
	TOK_KEYWORD_BEGIN, /* KEEP THIS THE LAST TOKEN! */

	TOK_TWOCHAR_END = TOK_KEYWORD_BEGIN,

	/* Name aliases */
	TOK_POS           = TOK_ADD,
	TOK_NEG           = TOK_SUB,
	TOK_LOWER         = TOK_LANGLE,
	TOK_GREATER       = TOK_RANGLE,
/*	TOK_COLON_COLON   = TOK_NAMESPACE, */
/*	TOK_LOWER_GREATER = TOK_LOGT, */
/*	TOK_LANGLE_RANGLE = TOK_LOGT, */
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

/* Special variable names. */
#define JIT_RTSYM_THIS  "this"
#define JIT_RTSYM_CLASS "class"



typedef struct jit_small_lexer JITSmallLexer;
typedef struct jit_symbol JITSymbol;
typedef struct jit_lvalue JITLValue;
typedef struct jit_lvalue_list JITLValueList;
typedef struct jit_lexer JITLexer;
typedef struct jit_context JITContext;
typedef struct jit_object_table JITObjectTable;
typedef struct jit_function_object JITFunctionObject;


/* Check if the given token qualifies for the associated operation parser function. */
#define JIT_TOKEN_IS_UNARY(self)                               \
	((self)->jl_tok == '.' || (self)->jl_tok == '(' ||         \
	 (self)->jl_tok == '[' || (self)->jl_tok == '{' ||         \
	 (self)->jl_tok == TOK_INC || (self)->jl_tok == TOK_DEC || \
	 JITLexer_ISKWD(self, "pack"))
#define JIT_TOKEN_IS_PROD(self)                        \
	((self)->jl_tok == '*' || (self)->jl_tok == '/' || \
	 (self)->jl_tok == '%' || (self)->jl_tok == TOK_POW)
#define JIT_TOKEN_IS_SUM(self) \
	((self)->jl_tok == '+' || (self)->jl_tok == '-')
#define JIT_TOKEN_IS_SHIFT(self) \
	((self)->jl_tok == TOK_SHL || (self)->jl_tok == TOK_SHR)
#define JIT_TOKEN_IS_CMP(self)                                           \
	((self)->jl_tok == TOK_LOWER || (self)->jl_tok == TOK_LOWER_EQUAL || \
	 (self)->jl_tok == TOK_GREATER || (self)->jl_tok == TOK_GREATER_EQUAL)
#define JIT_TOKEN_IS_CMPEQ(self)                                         \
	((self)->jl_tok == TOK_EQUAL || (self)->jl_tok == TOK_NOT_EQUAL ||   \
	 (self)->jl_tok == TOK_EQUAL3 || (self)->jl_tok == TOK_NOT_EQUAL3 || \
	 (self)->jl_tok == TOK_QMARK_QMARK || (self)->jl_tok == '!' ||       \
	 ((self)->jl_tok == JIT_KEYWORD &&                                   \
	  (JITLexer_ISTOK(self, "is") || JITLexer_ISTOK(self, "in"))))
#define JIT_TOKEN_IS_AND(self) \
	((self)->jl_tok == '&')
#define JIT_TOKEN_IS_XOR(self) \
	((self)->jl_tok == '^')
#define JIT_TOKEN_IS_OR(self) \
	((self)->jl_tok == '|')
#define JIT_TOKEN_IS_AS(self) \
	((self)->jl_tok == JIT_KEYWORD && JITLexer_ISTOK(self, "as"))
#define JIT_TOKEN_IS_LAND(self) \
	((self)->jl_tok == TOK_LAND)
#define JIT_TOKEN_IS_LOR(self) \
	((self)->jl_tok == TOK_LOR)
#define JIT_TOKEN_IS_COND(self) \
	((self)->jl_tok == '?')
#define JIT_TOKEN_IS_ASSIGN(self)         \
	((self)->jl_tok == TOK_COLON_EQUAL || \
	 ((self)->jl_tok >= TOK_ADD_EQUAL && (self)->jl_tok <= TOK_POW_EQUAL))

#define JIT_CASE_TOKEN_IS_UNARY \
	case '.':                   \
	case '(':                   \
	case '[':                   \
	case '{':                   \
	case TOK_INC:               \
	case TOK_DEC
#define JIT_CASE_TOKEN_IS_PROD \
	case '*':                  \
	case '/':                  \
	case '%':                  \
	case TOK_POW
#define JIT_CASE_TOKEN_IS_SUM \
	case '+':                 \
	case '-'
#define JIT_CASE_TOKEN_IS_SHIFT \
	case TOK_SHL:               \
	case TOK_SHR
#define JIT_CASE_TOKEN_IS_CMP \
	case TOK_LOWER:           \
	case TOK_LOWER_EQUAL:     \
	case TOK_GREATER:         \
	case TOK_GREATER_EQUAL
#define JIT_CASE_TOKEN_IS_CMPEQ \
	case TOK_EQUAL:             \
	case TOK_NOT_EQUAL:         \
	case TOK_EQUAL3:            \
	case TOK_NOT_EQUAL3:        \
	case TOK_QMARK_QMARK:       \
	case '!'
#define JIT_CASE_TOKEN_IS_AND \
	case '&'
#define JIT_CASE_TOKEN_IS_XOR \
	case '^'
#define JIT_CASE_TOKEN_IS_OR \
	case '|'
#define JIT_CASE_TOKEN_IS_LAND \
	case TOK_LAND
#define JIT_CASE_TOKEN_IS_LOR \
	case TOK_LOR
#define JIT_CASE_TOKEN_IS_COND \
	case '?'
#define JIT_CASE_TOKEN_IS_ASSIGN \
	case TOK_COLON_EQUAL:        \
	case TOK_ADD_EQUAL:          \
	case TOK_SUB_EQUAL:          \
	case TOK_MUL_EQUAL:          \
	case TOK_DIV_EQUAL:          \
	case TOK_MOD_EQUAL:          \
	case TOK_SHL_EQUAL:          \
	case TOK_SHR_EQUAL:          \
	case TOK_AND_EQUAL:          \
	case TOK_OR_EQUAL:           \
	case TOK_XOR_EQUAL:          \
	case TOK_POW_EQUAL



/* Helpers for working with  */
#define JIT_TOKEN_IS_DOT(self)       ((self)->jl_tok == '.' || (self)->jl_tok == TOK_DOTS)
#define JIT_TOKEN_IS_DOT_count(self) ((self)->jl_tok == TOK_DOTS ? 3 : 1)


#define JIT_SYMBOL_NONE      0x0000 /* No symbol (unused) */
#define JIT_SYMBOL_POINTER   0x0001 /* Pointer to an object reference (used to describe local & inherited variables) */
#define JIT_SYMBOL_OBJENT    0x0002 /* Object table entry. */
#define JIT_SYMBOL_EXTERN    0x0003 /* External symbol reference */
#define JIT_SYMBOL_GLOBAL    0x0004 /* Try to access an entry inside of `jc_globals' */
#define JIT_SYMBOL_GLOBALSTR 0x0005 /* Same as `JIT_SYMBOL_GLOBAL', but use a string as operand. */
#define JIT_SYMBOL_CLSATTRIB 0x0006 /* class- or instance-attribute. */
#define JIT_SYMBOL_ATTR      0x0007 /* Attribute expression. */
#define JIT_SYMBOL_ATTRSTR   0x0008 /* Attribute string expression. */
struct jit_symbol {
	uint16_t js_kind;  /* Symbol kind (One of JIT_SYMBOL_*) */
	uint16_t js_pad[(sizeof(void *)-2)/2];
	union {
		DREF DeeObject               **js_ptr;     /* [0..1][1..1] JIT_SYMBOL_POINTER -- Object pointer. */

		struct {
			struct jit_object_entry   *jo_ent;     /* [1..1] The entry, the last time it was loaded. */
			struct jit_object_table   *jo_tab;     /* [1..1] The object table in question. */
			/*utf-8*/ char const      *jo_namestr; /* [0..jo_namelen] The object name. */
			size_t                     jo_namelen; /* Length of the object name. */
		} js_objent; /* JIT_SYMBOL_OBJENT -- Object table entry. */

		struct {
			DREF DeeModuleObject      *jx_mod;     /* [1..1] The module that is being referenced.
			                                        * NOTE: Guarantied to be a regular, or a DEX module. */
			struct Dee_module_symbol      *jx_sym;     /* The symbol that is being accessed. */
		} js_extern; /* JIT_SYMBOL_EXTERN */

		DREF struct Dee_string_object *js_global;  /* [1..1] JIT_SYMBOL_GLOBAL user-level global symbol name. */

		struct {
			/*utf-8*/ char const      *jg_namestr; /* [0..jg_namelen] The global symbol name. */
			size_t                     jg_namelen; /* Length of the global symbol name. */
			Dee_hash_t                 jg_namehsh; /* Hash for the global symbol name. */
		} js_globalstr; /* JIT_SYMBOL_GLOBALSTR */

		struct {
			DREF DeeObject             *jc_obj;    /* [1..1][const] The object who's members are being referenced. */
			struct Dee_class_attribute *jc_attr;   /* [1..1][const] Attribute descriptor. */
			struct Dee_instance_desc   *jc_desc;   /* [1..1][const] Instance descriptor. */
		} js_clsattrib; /* JIT_SYMBOL_CLSATTRIB */

		struct {
			DREF DeeObject                *ja_base;    /* [1..1] Expression base object */
			DREF struct Dee_string_object *ja_name;    /* [1..1] Attribute name object */
		} js_attr; /* JIT_SYMBOL_ATTR */

		struct {
			DREF DeeObject            *ja_base;    /* [1..1] Expression base object */
			/*utf-8*/ char const      *ja_name;    /* [0..ja_size] Attribute name. */
			size_t                     ja_size;    /* Length of the attribute name. */
			Dee_hash_t                 ja_hash;    /* Hash of the attribute name. */
		} js_attrstr; /* JIT_SYMBOL_ATTRSTR */

	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define js_ptr       _dee_aunion.js_ptr
#define js_objent    _dee_aunion.js_objent
#define js_extern    _dee_aunion.js_extern
#define js_globalstr _dee_aunion.js_globalstr
#define js_clsattrib _dee_aunion.js_clsattrib
#define js_getset    _dee_aunion.js_getset
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#ifdef __INTELLISENSE__
NONNULL((1)) void DFCALL JITSymbol_Fini(JITSymbol *__restrict self);
#else /* __INTELLISENSE__ */
#define JITSymbol_Fini(self) JITLValue_Fini((JITLValue *)(self))
#endif /* !__INTELLISENSE__ */


/* Special value which may be returned by the EVAL functions to indicate
 * that the parsed expression cannot losslessly be represented as an object.
 * When this value is returned, everything required to calculate the underlying
 * value is stored within the the associated lexer `->jl_lvalue', and
 * `JITLexer_GetLValue()' can be called to discard extended information and
 * acquire the actual expression value. */
#define JIT_LVALUE    ITER_DONE


#define JIT_LVALUE_NONE      JIT_SYMBOL_NONE      /* No l-value */
#define JIT_LVALUE_POINTER   JIT_SYMBOL_POINTER   /* Pointer to an object reference (used to describe local & inherited variables) */
#define JIT_LVALUE_OBJENT    JIT_SYMBOL_OBJENT    /* Object table entry. (JIT_OBJECT_ENTRY_TYPE_LOCAL) */
#define JIT_LVALUE_EXTERN    JIT_SYMBOL_EXTERN    /* External symbol reference */
#define JIT_LVALUE_GLOBAL    JIT_SYMBOL_GLOBAL    /* Try to access an entry inside of `jc_globals' */
#define JIT_LVALUE_GLOBALSTR JIT_SYMBOL_GLOBALSTR /* Same as `JIT_LVALUE_GLOBAL', but use a string as operand. */
#define JIT_LVALUE_CLSATTRIB JIT_SYMBOL_CLSATTRIB /* class- or instance-attribute. */
#define JIT_LVALUE_ATTR      JIT_SYMBOL_ATTR      /* Attribute expression. */
#define JIT_LVALUE_ATTRSTR   JIT_SYMBOL_ATTRSTR   /* Attribute string expression. */
#define JIT_LVALUE_ITEM      0x0100               /* Item expression. */
#define JIT_LVALUE_RANGE     0x0101               /* Range expression. */
#define JIT_LVALUE_RVALUE    0x0200               /* R-value expression (just a regular, read-only expression, but stored inside of an L-Value descriptor). */
#define JIT_LVALUE_THIS      0x0201               /* Reference to the `this'-symbol (behaves the same as `JIT_LVALUE_RVALUE') */
#define JIT_LVALUE_ISSYM(kind)   ((kind) < 0x0100)
#define JITLValue_IsSymbol(self) JIT_LVALUE_ISSYM((self)->lv_kind)
struct jit_lvalue {
	uint16_t lv_kind;  /* L-value kind (One of JIT_LVALUE_*) */
	uint16_t lv_pad[(sizeof(void *)-2)/2];
	union {

		DREF DeeObject               **lv_ptr;     /* [0..1][1..1] JIT_LVALUE_POINTER -- Object pointer. */

		struct {
			struct jit_object_entry   *lo_ent;     /* [1..1] The entry, the last time it was loaded. */
			struct jit_object_table   *lo_tab;     /* [1..1] The object table in question. */
			/*utf-8*/ char const      *lo_namestr; /* [0..lo_namelen] The object name. */
			size_t                     lo_namesiz; /* Length of the object name. */
		} lv_objent; /* JIT_LVALUE_OBJENT -- Object table entry. */

		struct {
			DREF DeeModuleObject      *lx_mod;     /* [1..1] The module that is being referenced.
			                                        * NOTE: Guarantied to be a regular, or a DEX module. */
			struct Dee_module_symbol      *lx_sym;     /* The symbol that is being accessed. */
		} lv_extern; /* JIT_LVALUE_EXTERN */

		DREF struct Dee_string_object *lv_global;  /* [1..1] JIT_LVALUE_GLOBAL user-level global symbol name. */

		struct {
			/*utf-8*/ char const      *lg_namestr; /* [0..jg_namelen] The global symbol name. */
			size_t                     lg_namelen; /* Length of the global symbol name. */
			Dee_hash_t                 lg_namehsh; /* Hash of the global symbol name. */
		} lv_globalstr; /* JIT_LVALUE_GLOBALSTR */

		struct {
			DREF DeeObject             *lc_obj;    /* [1..1][const] The object who's members are being referenced. */
			struct Dee_class_attribute *lc_attr;   /* [1..1][const] Attribute descriptor. */
			struct Dee_instance_desc   *lc_desc;   /* [1..1][const] Instance descriptor. */
		} lv_clsattrib; /* JIT_LVALUE_CLSATTRIB */

		struct {
			DREF DeeObject                *la_base;/* [1..1] Expression base object */
			DREF struct Dee_string_object *la_name;/* [1..1] Attribute name object */
		} lv_attr; /* JIT_LVALUE_ATTR */

		struct {
			DREF DeeObject            *la_base;    /* [1..1] Expression base object */
			/*utf-8*/ char const      *la_name;    /* [0..la_size] Attribute name. */
			size_t                     la_size;    /* Length of the attribute name. */
			Dee_hash_t                 la_hash;    /* Hash of the attribute name. */
		} lv_attrstr; /* JIT_LVALUE_ATTRSTR */

		struct {
			DREF DeeObject            *li_base;    /* [1..1] Expression base object */
			DREF DeeObject            *li_index;   /* [1..1] Item index/key object */
		} lv_item; /* JIT_LVALUE_ITEM */

		struct {
			DREF DeeObject            *lr_base;    /* [1..1] Expression base object */
			DREF DeeObject            *lr_start;   /* [1..1] Range start index object */
			DREF DeeObject            *lr_end;     /* [1..1] Range end index object */
		} lv_range; /* JIT_LVALUE_RANGE */

		DREF DeeObject *lv_rvalue;    /* [1..1] JIT_LVALUE_RVALUE or JIT_LVALUE_THIS */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define lv_ptr       _dee_aunion.lv_ptr
#define lv_objent    _dee_aunion.lv_objent
#define lv_extern    _dee_aunion.lv_extern
#define lv_global    _dee_aunion.lv_global
#define lv_globalstr _dee_aunion.lv_globalstr
#define lv_attr      _dee_aunion.lv_attr
#define lv_attrstr   _dee_aunion.lv_attrstr
#define lv_item      _dee_aunion.lv_item
#define lv_range     _dee_aunion.lv_range
#define lv_rvalue    _dee_aunion.lv_rvalue
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#define JITLValue_Init(self)     ((self)->lv_kind = JIT_LVALUE_NONE)

/* Finalize a given L-Value object. */
INTDEF NONNULL((1)) void DFCALL
JITLValue_Fini(JITLValue *__restrict self);
INTDEF NONNULL((1, 2)) void DFCALL
JITLValue_Visit(JITLValue *__restrict self, Dee_visit_t proc, void *arg);

/* Interact with an L-Value
 * NOTE: For all of these, the caller must ensure that `self->lv_kind != JIT_LVALUE_NONE' */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
JITLValue_IsBound(JITLValue *__restrict self,
                  JITContext *__restrict context); /* -1: error; 0: no; 1: yes */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL
JITLValue_GetValue(JITLValue *__restrict self,
                   JITContext *__restrict context);
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
JITLValue_DelValue(JITLValue *__restrict self,
                   JITContext *__restrict context);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DFCALL
JITLValue_SetValue(JITLValue *__restrict self,
                   JITContext *__restrict context,
                   DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DFCALL
JITLValue_CallValue(JITLValue *__restrict self,
                    JITContext *__restrict context,
                    DeeObject *args, DeeObject *kw);


struct jit_lvalue_list {
	size_t     ll_size;  /* Number of used entries. */
	size_t     ll_alloc; /* Number of allocated entries. */
	JITLValue *ll_list;  /* [0..ll_size|ALLOC(ll_alloc)][owned] Vector of L-values. */
};

#define JITLVALUELIST_INIT { 0, 0, NULL }
#define JITLValueList_Init(self) \
	((self)->ll_size = (self)->ll_alloc = 0, (self)->ll_list = NULL)
#define JITLValueList_CInit(self)   \
	(ASSERT((self)->ll_size == 0),  \
	 ASSERT((self)->ll_alloc == 0), \
	 ASSERT((self)->ll_list == NULL))

/* Finalize the given L-value list. */
INTDEF NONNULL((1)) void DCALL JITLValueList_Fini(JITLValueList *__restrict self);

/* Append the given @value onto @self, returning -1 on error and 0 on success. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
JITLValueList_Append(JITLValueList *__restrict self,
                     /*inherit(on_success)*/ JITLValue *__restrict value);
/* Append an R-value expression. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
JITLValueList_AppendRValue(JITLValueList *__restrict self,
                           DeeObject *__restrict value);

struct Dee_objectlist;

/* Copy `self' and append all of the referenced objects to the given object list.
 * NOTE: `self' remains valid after this operation! */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
JITLValueList_CopyObjects(JITLValueList *__restrict self,
                          struct Dee_objectlist *__restrict dst,
                          JITContext *__restrict context);

/* Unpack `values' and assign each of the unpacked values to
 * the proper LValue of at the same position within `self'
 * @return:  0: Success.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
JITLValueList_UnpackAssign(JITLValueList *__restrict self,
                           JITContext *__restrict context,
                           DeeObject *__restrict values);


struct jit_small_lexer {
	unsigned int                   jl_tok;      /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
	/*utf-8*/ unsigned char const *jl_tokstart; /* [1..1] Token starting pointer. */
	/*utf-8*/ unsigned char const *jl_tokend;   /* [1..1] Token end pointer. */
	/*utf-8*/ unsigned char const *jl_end;      /* [1..1] Input end pointer. */
};

struct jit_lexer {
	unsigned int                   jl_tok;       /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
	/*utf-8*/ unsigned char const *jl_tokstart;  /* [1..1] Token starting pointer. */
	/*utf-8*/ unsigned char const *jl_tokend;    /* [1..1] Token end pointer. */
	/*utf-8*/ unsigned char const *jl_end;       /* [1..1] Input end pointer. */
	/*utf-8*/ unsigned char const *jl_errpos;    /* [0..1] Lexer error position. */
	union {
		struct {
			JITFunctionObject *jl_function;  /* [1..1] The function who's text is being scanned. */
			JITObjectTable    *jl_parobtab;  /* [0..1] Pointer to the parent scope's object table. */
#define JIT_SCANDATA_FNORMAL   0x0000        /* Normal scan data flags. */
#define JIT_SCANDATA_FINCHILD  0x0001        /* The scanner is currently processing text of a recursively defined child function. */
#define JIT_SCANDATA_FERROR    0x0002        /* An error occurred. */
			unsigned int       jl_flags;     /* Set of `JIT_SCANDATA_F*' */
		}                      jl_scandata;  /* Data fields used by `JITLexer_Scan*' functions. */
		struct {
			DeeObject         *jl_text;      /* [1..1] The object that owns input text (Usually a string or Bytes object)
			                                  * For expressions such as ones used to create lambda functions, a reference
			                                  * to this object is stored to ensure that child code will not be deallocated. */
			JITContext        *jl_context;   /* [1..1][const] The associated JIT context. */
			JITLValue          jl_lvalue;    /* L-value expression. */
		}
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
		_dee_astruct
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
		;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define jl_scandata _dee_aunion.jl_scandata
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define jl_text     _dee_aunion._dee_astruct.jl_text
#define jl_context  _dee_aunion._dee_astruct.jl_context
#define jl_lvalue   _dee_aunion._dee_astruct.jl_lvalue
#else /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#define jl_text     _dee_aunion.jl_text
#define jl_context  _dee_aunion.jl_context
#define jl_lvalue   _dee_aunion.jl_lvalue
#endif /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#elif !defined(__COMPILER_HAVE_TRANSPARENT_STRUCT)
#define jl_text     _dee_astruct.jl_text
#define jl_context  _dee_astruct.jl_context
#define jl_lvalue   _dee_astruct.jl_lvalue
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
	;
};

#define JITLexer_TokPtr(self) ((char const *)(self)->jl_tokstart)
#define JITLexer_TokLen(self) (size_t)((self)->jl_tokend - (self)->jl_tokstart)


/* Similar to `JITLexer_GetLValue()', but also finalize
 * the stored L-value, and set it to describe nothing.
 * NOTE: The stored L-value is _always_ reset! */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
JITLexer_PackLValue(JITLexer *__restrict self);


/* Check if the current token is a keyword `x' */
#define JITLexer_ISKWD(self, x)                                         \
	((self)->jl_tok == JIT_KEYWORD &&                                   \
	 COMPILER_STRLEN(x) == ((self)->jl_tokend - (self)->jl_tokstart) && \
	 bcmpc((self)->jl_tokstart, x, COMPILER_STRLEN(x), sizeof(char)) == 0)
#define JITLexer_ISTOK(self, x)                                         \
	(COMPILER_STRLEN(x) == ((self)->jl_tokend - (self)->jl_tokstart) && \
	 bcmpc((self)->jl_tokstart, x, COMPILER_STRLEN(x), sizeof(char)) == 0)

/* Initialize a given JIT lexer for a given data block, and load the first token. */
#define JITLexer_Start(self, start, end) \
	((self)->jl_tokend = (start),        \
	 (self)->jl_end    = (end),          \
	 JITLexer_Yield(self))
#define JITSmallLexer_Start(self, start, end) \
	((self)->jl_tokend = (start),             \
	 (self)->jl_end    = (end),               \
	 JITLexer_Yield((JITLexer *)(self)))


/* Starting at `token->jl_tokend', scan for the next input token
 * NOTE: This function may also be used with `JITSmallLexer' */
INTDEF NONNULL((1)) void DFCALL JITLexer_Yield(JITLexer *__restrict self);
#define JITLexer_YieldAt(self, pos) ((self)->jl_tokend = (pos), JITLexer_Yield(self))

/* Remember the fact that an exception was thrown
 * when code at `pos' was being executed. */
#ifdef __INTELLISENSE__
NONNULL((1, 2)) void DFCALL
JITLexer_ErrorTrace(JITLexer *__restrict self,
                    unsigned char const *__restrict pos);
#else /* __INTELLISENSE__ */
#define JITLexer_ErrorTrace(self, pos) (void)((self)->jl_errpos = (pos))
#endif /* !__INTELLISENSE__ */



#define JIT_OBJECT_ENTRY_TYPE_LOCAL      0 /* Local variable. */
#define JIT_OBJECT_ENTRY_TYPE_ATTR       1 /* Unbound (non-static) class member */
#define JIT_OBJECT_ENTRY_TYPE_ATTR_FIXED 2 /* Bound (static or non-static) class member */
#define JIT_OBJECT_ENTRY_EXTERN_SYMBOL   3 /* External module symbol (JIT_SYMBOL_EXTERN) */
#define JIT_OBJECT_ENTRY_EXTERN_ATTR     4 /* External module attribute (JIT_SYMBOL_ATTR) */
#define JIT_OBJECT_ENTRY_EXTERN_ATTRSTR  5 /* External module attribute (JIT_SYMBOL_ATTRSTR) */
struct jit_object_entry {
#define jit_object_entry_eqname(self, rhs_str, rhs_len, rhs_hsh) \
	((self)->oe_namehsh == (rhs_hsh) &&                          \
	 (self)->oe_namelen == (rhs_len) &&                          \
	 bcmpc((self)->oe_namestr, rhs_str, rhs_len, sizeof(char)) == 0)

	/*utf-8*/ char const       *oe_namestr;    /* [0..oe_namelen] Name of the object
	                                            * NOTE: `NULL' indicates an unused/sentinel entry;
	                                            *       `ITER_DONE' indicates a deleted entry. */
	size_t                      oe_namelen;    /* Length of the object name. */
	Dee_hash_t                  oe_namehsh;    /* Hash of the object name. */
	uintptr_t                   oe_type;       /* Object type (one of `JIT_OBJECT_ENTRY_TYPE_*') */
	union {
		DREF DeeObject         *oe_value;      /* [0..1][JIT_OBJECT_ENTRY_TYPE_LOCAL]
		                                        * Value associated with this entry (NULL if unbound). */

		struct {
			DREF DeeTypeObject         *a_class;    /* [1..1][const] The class that is declaring the attribute */
			struct Dee_class_attribute *a_attr;     /* [1..1][const] Attribute descriptor. */
		} oe_attr;                                  /* JIT_OBJECT_ENTRY_TYPE_ATTR */

		struct {
			DREF DeeObject             *af_obj;     /* [1..1][const] The object who's members are being referenced. */
			struct Dee_class_attribute *af_attr;    /* [1..1][const] Attribute descriptor. */
			struct Dee_instance_desc   *af_desc;    /* [1..1][const] Instance descriptor. */
		} oe_attr_fixed;                            /* JIT_OBJECT_ENTRY_TYPE_ATTR_FIXED */

		struct {
			DREF DeeModuleObject      *es_mod;      /* [1..1] The module that is being referenced.
			                                         * NOTE: Guarantied to be a regular, or a DEX module. */
			struct Dee_module_symbol  *es_sym;      /* The symbol that is being accessed. */
		} oe_extern_symbol;                         /* JIT_OBJECT_ENTRY_EXTERN_SYMBOL */

		struct {
			DREF DeeObject                *ea_base; /* [1..1] Expression base object */
			DREF struct Dee_string_object *ea_name; /* [1..1] Attribute name object */
		} oe_extern_attr;                           /* JIT_OBJECT_ENTRY_EXTERN_ATTR */

		struct {
			DREF DeeObject            *eas_base;    /* [1..1] Expression base object */
			/*utf-8*/ char const      *eas_name;    /* [0..ja_size] Attribute name. */
			size_t                     eas_size;    /* Length of the attribute name. */
			Dee_hash_t                 eas_hash;    /* Hash of the attribute name. */
		} oe_extern_attrstr;                        /* JIT_OBJECT_ENTRY_EXTERN_ATTRSTR */
	};
};

#define jit_object_entry_fini(self)                     \
	(Dee_XDecref((self)->oe_value),                     \
	 (self)->oe_type == JIT_OBJECT_ENTRY_EXTERN_ATTR    \
	 ? (void)Dee_Decref((self)->oe_extern_attr.ea_name) \
	 : (void)0)
#define jit_object_entry_visit(self) \
	Dee_XVisit((self)->oe_value) /* No need to visit `oe_extern_attr.ea_name' (it's always a string) */


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
	size_t                          ot_mask; /* Allocated hash-mask. */
	size_t                          ot_size; /* Number of used + deleted entries. */
	size_t                          ot_used; /* Number of used entries. */
	struct jit_object_entry        *ot_list; /* [1..ot_mask + 1][owned_if(ot_list != jit_empty_object_list)]
	                                          * Object table hash-vector. */
	struct jit_object_table_pointer ot_prev; /* Previous object table (does not affect utility functions, and
	                                          * is only used to link between different scoped object tables) */
	size_t                          ot_star_importc; /* Number of `import * from ...' modules/objects. */
	DREF DeeObject                **ot_star_importv; /* [1..1][0..ot_star_importc][owned] Vector of modules/objects used in `import * from ...' statements. */
};

INTDEF struct jit_object_entry jit_empty_object_list[1];

#define JITObjectTable_NEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)

/* Initialize/finalize a given JIT object table. */
#define JITOBJECTTABLE_INIT { 0, 0, 0, jit_empty_object_list }
#define JITObjectTable_CInit(self)            \
	(ASSERT((self)->ot_mask == 0),            \
	 ASSERT((self)->ot_size == 0),            \
	 ASSERT((self)->ot_used == 0),            \
	 (self)->ot_list = jit_empty_object_list, \
	 ASSERT((self)->ot_star_importc == 0),    \
	 ASSERT((self)->ot_star_importv == NULL))
#define JITObjectTable_Init(self)                             \
	((self)->ot_mask = (self)->ot_size = (self)->ot_used = 0, \
	 (self)->ot_list = jit_empty_object_list,                 \
	 (self)->ot_star_importc = 0, (self)->ot_star_importv = NULL)
INTDEF NONNULL((1)) void DCALL JITObjectTable_Fini(JITObjectTable *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL JITObjectTable_Visit(JITObjectTable *__restrict self, Dee_visit_t proc, void *arg);

/* Allocate/free a JIT object table from cache. */
#define JITObjectTable_Alloc()   DeeObject_MALLOC(struct jit_object_table)
#define JITObjectTable_Free(ptr) DeeObject_FFree(ptr, sizeof(struct jit_object_table))

/* Initialize `dst' as a copy of `src' */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
JITObjectTable_Copy(JITObjectTable *__restrict dst,
                    JITObjectTable const *__restrict src);

/* Update an object within the given object table, potentially overwriting an
 * existing object, or creating a new entry if no existing object could be found.
 * @param: value: The value to assign to the entry.
 *                When `NULL', the entry is unbound.
 * @return: 1:  Successfully updated an existing entry when `override_existing' was `true'.
 * @return: 1:  An entry already existed for the given name when `override_existing' was `false'.
 * @return: 0:  Successfully created a new entry.
 * @return: -1: An error occurred (failed to increase the hash size of `self') */
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) int DCALL
JITObjectTable_Update(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh,
                      /*[0..1]*/ DeeObject *value,
                      bool override_existing);

/* Delete an existing entry for an object with the given name
 * @return: true:  Successfully deleted the entry, after potentially unbinding an associated object.
 * @return: false: The object table didn't include an entry matching the given name. */
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL
JITObjectTable_Delete(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh);

/* Lookup a given object within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Could not find an object matching the specified name. (no error was thrown) */
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) struct jit_object_entry *DCALL
JITObjectTable_Lookup(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh);

/* Lookup or create an entry for a given name within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Failed to create a new entry. (an error _WAS_ thrown) */
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) struct jit_object_entry *DCALL
JITObjectTable_Create(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh);

/* Add a *-import module or object to `self' (if not already persent)
 * @return: 0 : Success
 * @return: -1: Success */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
JITObjectTable_AddImportStar(JITObjectTable *__restrict self,
                             DeeObject *module_or_object);

/* Search the list of *-imports of `self' for the one (if it exists)
 * that has an attribute matching the given `namestr'. If found,
 * return a reference to it, and if not found, return ITER_DONE.
 * NOTE: This function searches `self->ot_star_importv' in reverse
 *       order, meaning that modules from which an import happened
 *       more recently (as per `JITObjectTable_AddImportStar()')
 *       will be hit first. Also note that once a hit is found, the
 *       search ends (this behavior differs from the core compiler,
 *       where multiple *-imports of the same symbol-name result
 *       in a compiler error stating that the symbol is ambiguous).
 *       However, this difference is acceptable, since it only
 *       affects code that would otherwise be malformed.
 * @param: p_mod_symbol: when non-NULL, store the module-symbol (in
 *                       case the *-import was made for a module)
 * @return: * :        The module/object defining `namestr'
 * @return: ITER_DONE: The *-imported module defines `namestr'
 * @return: NULL:      An error was thrown. */
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DREF DeeObject *DCALL
JITObjectTable_FindImportStar(JITObjectTable *__restrict self,
                              /*utf-8*/ char const *namestr,
                              size_t namelen, Dee_hash_t namehsh,
                              struct Dee_module_symbol **p_mod_symbol);



/* Special values for `jc_retval' */
#define JITCONTEXT_RETVAL_UNSET     NULL                              /* unset return value */
#define JITCONTEXT_RETVAL_BREAK     ((DREF DeeObject *)(uintptr_t)-1) /* Unwind for loop break */
#define JITCONTEXT_RETVAL_CONTINUE  ((DREF DeeObject *)(uintptr_t)-2) /* Unwind for loop continue */
#define JITCONTEXT_RETVAL_ISSET(x)  ((uintptr_t)(x) < (uintptr_t)-0xf)

#define JITCONTEXT_FNORMAL 0x0000 /* Normal flags. */
#define JITCONTEXT_FSYNERR 0x0001 /* A syntax error occurred that may not be caught. */

struct jit_context {
	DeeObject       *jc_import;   /* [0..1] `import' function override (when NULL, use `DeeModule_ImportRel(jc_impbase)' instead) */
	DeeModuleObject *jc_impbase;  /* [0..1] Base module used for relative, static imports (such as `foo from .baz.bar')
	                               * When `NULL', code isn't allowed to perform relative imports.
	                               * NOTE: If this isn't a module, JIT itself will throw an error. */
	struct jit_object_table_pointer
	                 jc_locals;   /* Local variable table (forms a chain all the way to the previous base-scope) */
	DeeObject       *jc_globals;  /* [0..1] A pre-defined, mapping-like object containing pre-defined globals.
	                               * This object can be passed via the `globals' argument to `exec from deemon'
	                               * When not user-defined, a Dict object is created the first time a write happens. */
#ifdef __INTELLISENSE__
	DeeObject       *jc_retval;   /* [0..1] Function return value.
	                               * When this is set to be non-NULL, and one of the `JITLexer_Eval*' functions
	                               * returns `NULL', then the there wasn't actually an error, but an alive return
	                               * statement was encountered, with the pre-existing error-unwind path being re-used
	                               * for propagation of that return value out of the JIT parser. */
#else /* __INTELLISENSE__ */
	DREF DeeObject  *jc_retval;   /* [0..1] Function return value.
	                               * When this is set to be non-NULL, and one of the `JITLexer_Eval*' functions
	                               * returns `NULL', then the there wasn't actually an error, but an alive return
	                               * statement was encountered, with the pre-existing error-unwind path being re-used
	                               * for propagation of that return value out of the JIT parser. */
#endif /* !__INTELLISENSE__ */
	uint16_t         jc_except;   /* [const] Exception indirection at the start of code. */
	uint16_t         jc_flags;    /* Context flags (Set of `JITCONTEXT_F*') */
};
#define JITCONTEXT_INIT       { NULL, NULL, { NULL, 0 }, NULL, NULL, 0, JITCONTEXT_FNORMAL }
#define JITContext_Init(self) bzero(self, sizeof(JITContext))
#define JITContext_Fini(self) (void)0

/* Return a reference to an object that implements attribute
 * operators such that it allows access to JIT globals. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
JITContext_GetCurrentModule(JITContext *__restrict self);


/* Check if the current scope is the global scope. */
#define JITContext_IsGlobalScope(x)  \
	((x)->jc_locals.otp_ind == 0 ||  \
	 ((x)->jc_locals.otp_ind == 1 && \
	  (x)->jc_locals.otp_tab &&      \
	  !(x)->jc_locals.otp_tab->ot_prev.otp_tab))

/* Enter a new locals sub-scope. */
#define JITContext_PushScope(x) \
	(void)(++(x)->jc_locals.otp_ind)

/* Leave the current locals sub-scope. */
#define JITContext_PopScope(x)                  \
	(void)(ASSERT((x)->jc_locals.otp_ind != 0), \
	       (--(x)->jc_locals.otp_ind == 0) ? _JITContext_PopLocals(x) : (void)0)
INTDEF NONNULL((1)) void DCALL _JITContext_PopLocals(JITContext *__restrict self);

/* Get a pointer to the first locals object-table for the current scope,
 * either for reading (in which case `NULL' is indicative of an empty scope),
 * or for writing (in which case `NULL' indicates an error) */
#ifdef __INTELLISENSE__
WUNUSED NONNULL((1)) JITObjectTable *DCALL JITContext_GetROLocals(JITContext *__restrict self);
#else /* __INTELLISENSE__ */
#define JITContext_GetROLocals(self) ((self)->jc_locals.otp_tab)
#endif /* !__INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1)) JITObjectTable *DCALL JITContext_GetRWLocals(JITContext *__restrict self);



/* Lookup a given symbol within a specific JIT context
 * @param: mode: Set of `JIT_LOOKUP_SYM_*'
 * @return: 0:  The specified symbol was found, and `result' was filled
 * @return: -1: An error occurred. */
INTDEF WUNUSED ATTR_INS(3, 4) NONNULL((1, 2)) int DFCALL
JITContext_Lookup(JITContext *__restrict self,
                  JITSymbol *__restrict result,
                  /*utf-8*/ char const *name,
                  size_t namelen, unsigned int mode);
INTDEF WUNUSED ATTR_INS(3, 4) NONNULL((1, 2)) int DFCALL
JITContext_LookupNth(JITContext *__restrict self,
                     JITSymbol *__restrict result,
                     /*utf-8*/ char const *name,
                     size_t namelen, size_t nth);

#define JIT_LOOKUP_SYM_NORMAL    0x0000
#define JIT_LOOKUP_SYM_VDEFAULT  0x0000 /* Default visibility. */
#define JIT_LOOKUP_SYM_VLOCAL    0x0001 /* Lookup rules when `local' is prefixed. */
#define JIT_LOOKUP_SYM_VGLOBAL   0x0002 /* Lookup rules when `global' is prefixed. */
#define JIT_LOOKUP_SYM_VMASK     0x0003 /* Mask for visibility options. */
#define JIT_LOOKUP_SYM_STATIC    0x0100 /* Create static variables / warn about non-static, existing variables. */
#define JIT_LOOKUP_SYM_STACK     0x0200 /* Create stack variables / warn about non-stack, existing variables. */
#define JIT_LOOKUP_SYM_FINAL     0x0400 /* Create final (write-once) variables. */
#define JIT_LOOKUP_SYM_ALLOWDECL 0x8000 /* Allow declaration of new variables (HINT: Unless set, warn when new variables are created). */



/* Parse an operator name, as can be found in an `x.operator <NAME>' expression
 * @param: features: Set of `P_OPERATOR_F*'
 * @return: * : One of `OPERATOR_*' or `AST_OPERATOR_*'
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2)) int32_t DFCALL
JITLexer_ParseOperatorName(JITLexer *__restrict self,
                           DeeTypeObject *typetype,
                           uint16_t features);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipOperatorName(JITLexer *__restrict self);
#define JIT_P_OPERATOR_FNORMAL 0x0000
#define JIT_P_OPERATOR_FCLASS  0x0001 /* Allow class-specific operator names. */


/* Ambiguous operator codes.
 * The caller should resolved these based on operand count. */
#define JIT_AST_OPERATOR_POS_OR_ADD           0xf000 /* `+' */
#define JIT_AST_OPERATOR_NEG_OR_SUB           0xf001 /* `-' */
#define JIT_AST_OPERATOR_GETITEM_OR_SETITEM   0xf002 /* `[]' */
#define JIT_AST_OPERATOR_GETRANGE_OR_SETRANGE 0xf003 /* `[:]' */
#define JIT_AST_OPERATOR_GETATTR_OR_SETATTR   0xf004 /* `.' */
#define JIT_AST_OPERATOR_MIN                  0xf000
#define JIT_AST_OPERATOR_MAX                  0xf004

/* Special class operators. */
#define JIT_AST_OPERATOR_FOR                  0xf005 /* `for' */


/* Check if the current token may refer to the start of an expression.
 * The currently selected token is not altered/is restored before this function returns.
 * NOTE: This function may also be used with `JITSmallLexer' */
INTDEF WUNUSED NONNULL((1)) bool DFCALL
JITLexer_MaybeExpressionBegin(JITLexer *__restrict self);


/* Return the operator function for `opname', as exported from the `operators' module. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JIT_GetOperatorFunction(DeeTypeObject *__restrict typetype, Dee_operator_t opname);

/* JIT-specific evaluation flags (may be optionally or'd with `JIT_LOOKUP_SYM_*'). */
#define JITLEXER_EVAL_FNORMAL       0x0000 /* Normal evaluation flags. */
#define JITLEXER_EVAL_FALLOWINPLACE 0x0000 /* (ignored) Allow inplace operations */
#define JITLEXER_EVAL_FALLOWISBOUND 0x0000 /* (ignored) Allow `foo is bound' expressions */
#define JITLEXER_EVAL_FDISALLOWCAST 0x0040 /* Disallow cast expressions. */
#define JITLEXER_EVAL_FPRIMARY      (JITLEXER_EVAL_FALLOWISBOUND | JITLEXER_EVAL_FALLOWINPLACE)

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalUnaryHead(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalUnary(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalProd(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalSum(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalShift(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalCmp(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalCmpEQ(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalAnd(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalXor(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalOr(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalAs(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalLand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalLor(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalCond(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalAssign(JITLexer *__restrict self, unsigned int flags); /* NOTE: Also handled inplace operators. */

/* With the current token one of the unary operator symbols, consume
 * it and parse the second operand before returning the combination */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalUnaryOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalProdOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalSumOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalShiftOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalCmpOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalCmpEQOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalAndOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalXorOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalOrOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalAsOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalLandOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalLorOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalCondOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalAssignOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalCommaTupleOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCommaTupleOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalCommaListOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCommaListOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL JITLexer_EvalCommaDictOperand(JITLexer *__restrict self, /*inherit(always)*/ DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCommaDictOperand(JITLexer *__restrict self, unsigned int flags);

/* Evaluate/skip an argument list, including keyword labels */
INTDEF WUNUSED ATTR_OUT(2) NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalArgumentList(JITLexer *__restrict self,
                          DREF DeeObject **__restrict p_kwds);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipArgumentList(JITLexer *__restrict self);

/* Evaluate a keyword label list and return a valid mapping for keywords to values.
 * >> foo(10, 20, x: 30, y: 40);
 *                   ^        ^
 *                ^
 *                +--- first_label_name == "x: 30, y...."
 *                     first_label_size == 1 */
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalKeywordLabelList(JITLexer *__restrict self,
                              char const *__restrict first_label_name,
                              size_t first_label_size);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipKeywordLabelList(JITLexer *__restrict self);


/* NOTE: This function might not necessarily return a module when a custom import override was defined.
 *       If that is the case, the caller must instead access the attributes of the returned object! */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalModule(JITLexer *__restrict self);
#define JITLexer_SkipModule(self) JITLexer_SkipModuleName(self)

/* Evaluate an import expression, starting on the token after "import".
 * >> import("foo");
 *          ^      ^ */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalImportExpression(JITLexer *__restrict self);


/* Parse a comma-separated list of expressions,
 * as well as assignment/inplace expressions.
 * >> foo = 42;               // (foo = (42));
 * >> foo += 42;              // (foo += (42));
 * >> foo, bar = (10, 20)...; // (foo, bar = (10, 20)...);
 * >> foo, bar = 10;          // (foo, (bar = 10));
 * >> { 10 }                  // (List { 10 }); // When `AST_COMMA_ALLOWBRACE' is set
 * >> { "foo": 10 }           // (Dict { "foo": 10 }); // When `AST_COMMA_ALLOWBRACE' is set
 * @param: mode:       Set of `AST_COMMA_*'     - What is allowed and when should we pack values.
 * @param: seq_type:   The type of sequence to generate (one of `DeeTuple_Type' or `DeeList_Type')
 *                     When `NULL', evaluate to the last comma-expression.
 * @param: p_out_mode: When non-NULL, instead of parsing a `;' when required,
 *                     set to `JIT_AST_COMMA_OUT_FNEEDSEMI' indicative of this. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
JITLexer_EvalComma(JITLexer *__restrict self, uint16_t mode,
                   DeeTypeObject *seq_type, uint16_t *p_out_mode);
INTDEF WUNUSED NONNULL((1)) int DCALL
JITLexer_SkipComma(JITLexer *__restrict self, uint16_t mode,
                   uint16_t *p_out_mode);

#define JIT_AST_COMMA_NORMAL        0x0000
#define JIT_AST_COMMA_FORCEMULTIPLE 0x0001 /* Always pack objects according to `flags' */
#define JIT_AST_COMMA_STRICTCOMMA   0x0002 /* Strictly enforce the rule of a `,' being followed by another expression.
                                            * NOTE: When this flag is set, trailing `,' are not parsed, but remain as the active token upon exit. */
#define JIT_AST_COMMA_NOSUFFIXKWD   0x0080 /* Don't parse c-style variable declarations for reserved keywords.
                                            * This is required for `else', `catch', `finally', etc.
                                            * >> `try foo catch (...)' (don't interpret as `local catch = foo(...)' when starting with `foo') */
#define JIT_AST_COMMA_ALLOWTYPEDECL 0x0800 /* Allow type declaration to be appended to variables, as well as documentation strings to be consumed. */
#define JIT_AST_COMMA_ALLOWKWDLIST  0x1000 /* Stop if what a keyword list label is encountered. */
#define JIT_AST_COMMA_PARSESINGLE   0x2000 /* Only parse a single expression. */
#define JIT_AST_COMMA_PARSESEMI     0x4000 /* Parse a `;' as part of the expression (if a `;' is required). */
#define JIT_AST_COMMA_ALLOWVARDECLS 0x8000 /* Allow new variables to be declared. */

#define JIT_AST_COMMA_OUT_FNORMAL   0x0000 /* Normal comma output flags. */
#define JIT_AST_COMMA_OUT_FNEEDSEMI 0x0001 /* Set if a semicolon is required. */
/* Additional flags only used by the JIT compiler. */
#define JIT_AST_COMMA_OUT_FMULTIPLE 0x0010 /* Multiple expressions were parsed. */


/* Parse a module name, either writing it to `*printer' (if non-NULL),
 * or storing the name's start and end pointers in `*p_name_start' and
 * `*p_name_end'
 * @return:  1: Successfully parsed the module name and written it to `*printer'
 * @return:  0: Successfully parsed the module name and stored it in `*p_name_start' / `*p_name_end'
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_EvalModuleName(JITLexer *__restrict self,
                        struct Dee_unicode_printer *printer,
                        /*utf-8*/ unsigned char const **p_name_start,
                        /*utf-8*/ unsigned char const **p_name_end);
#define JITLexer_SkipModuleName(self) \
	(JITLexer_EvalModuleName(self, NULL, NULL, NULL) < 0 ? -1 : 0)
#define JITLexer_SkipModuleNameIntoPrinter(self) \
	JITLexer_SkipModuleName(self)

/* Same as `JITLexer_EvalModuleName()', but always parse into a printer.
 * @return:  0: Successfully.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
JITLexer_EvalModuleNameIntoPrinter(JITLexer *__restrict self,
                                   struct Dee_unicode_printer *__restrict printer);

/* Evaluate a symbol name for an import statement and write it to `printer'
 * @return:  0: Successfully.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
JITLexer_EvalSymbolNameIntoPrinter(JITLexer *__restrict self,
                                   struct Dee_unicode_printer *__restrict printer);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipSymbolNameIntoPrinter(JITLexer *__restrict self);

/* Parse lookup mode modifiers:
 * >> local x = 42;
 *    ^     ^
 */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
JITLexer_ParseLookupMode(JITLexer *__restrict self,
                         unsigned int *__restrict p_mode);


/* Parse or skip a template string. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalTemplateString(JITLexer *__restrict self);
INTERN WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipTemplateString(JITLexer *__restrict self);


/* Skip evaluation functions. (same as the regular functions,
 * but expressions are skipped, rather than being evaluated)
 * However, syntax error are still thrown.
 * @param: flags: Set of `JITLEXER_EVAL_F*' */
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipUnaryHead(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipUnary(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipProd(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipSum(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipShift(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCmp(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCmpEQ(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAnd(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipXor(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipOr(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAs(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipLand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipLor(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCond(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAssign(JITLexer *__restrict self, unsigned int flags); /* NOTE: Also handles inplace operators. */
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipUnaryOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipProdOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipSumOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipShiftOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCmpOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCmpEQOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAndOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipXorOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipOrOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAsOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipLandOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipLorOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipCondOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAssignOperand(JITLexer *__restrict self, unsigned int flags);

/* Parse any kind of post-expression operand. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL
JITLexer_EvalOperand(JITLexer *__restrict self,
                     /*inherit(always)*/ DREF DeeObject *__restrict lhs,
                     unsigned int flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipOperand(JITLexer *__restrict self, unsigned int flags);

/* Recursively skip a pair of tokens, such as `{' and `}' or `(' and `)'
 * NOTE: Entry is expected to be after the initial instance of `pair_open' */
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipPair(JITLexer *__restrict self,
                  unsigned int pair_open,
                  unsigned int pair_close);

/* Parse, evaluate & execute an expression using JIT
 * @param: flags: Set of `JITLEXER_EVAL_F*' */
#define JITLexer_EvalExpression          JITLexer_EvalAssign
#define JITLexer_SkipExpression          JITLexer_SkipAssign
#define JITLexer_SkipGeneratorExpression JITLexer_SkipAssign


/* Parse a whole statement. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalStatement(JITLexer *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipStatement(JITLexer *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalStatementBlock(JITLexer *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipStatementBlock(JITLexer *__restrict self);


/************************************************************************/
/* Handling of type annotations.                                        */
/************************************************************************/
/* Skip type annotations:
 * >> local x: int from deemon | string from deemon = 42;
 *             ^                                    ^
 * @return: 0 : Success
 * @return: -1: Compiler error (only thrown when `throw_errors != false') */
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipTypeAnnotation(JITLexer *__restrict self, bool throw_errors);
/************************************************************************/



/* Parse the mask portion of a catch statement:
 * >> try {
 * >>     throw "Foo";
 * >> } catch (string as s) {
 *             ^            ^
 * >> }
 * @param: p_typemask: Filled with one of:
 *                      - NULL:           No mask was given (all objects can be caught)
 *                      - DeeTupleObject: A tuple of types that should be caught
 *                      - DeeTypeObject:  The single type that should be caught
 *                      - DeeObject:      Some other object (Dee)
 *                    You may use `JIT_IsCatchable()' to determine if the object can
 *                    be caught using this mask.
 */
INTDEF WUNUSED ATTR_OUT(2) ATTR_OUT(3) ATTR_OUT(4) NONNULL((1)) int DFCALL
JITLexer_ParseCatchMask(JITLexer *__restrict self,
                        DREF DeeObject **__restrict p_typemask,
                        char const **__restrict p_symbol_name,
                        size_t *__restrict p_symbol_size);

/* Check if `thrown_object' can be caught with `typemask'
 * NOTE: Assumes that interrupt catches are allowed.
 *       If such catches aren't allowed, the caller should
 *       call this function as:
 *       >> can_catch = (!mask || JIT_IsCatchable(obj, mask)) &&
 *       >>             (allow_interrupts || !DeeObject_IsInterrupt(obj)); */
INTDEF WUNUSED NONNULL((1, 2)) bool DFCALL
JIT_IsCatchable(DeeObject *thrown_object,
                DeeObject *typemask);

/* Hybrid parsing functions. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalStatementOrBraces(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipStatementOrBraces(JITLexer *__restrict self, unsigned int *p_was_expression);

/* Starting immediately after a `{' token, parse the items of the brace
 * initializer / expression, before returning ontop of the `}' token. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalBraceItems(JITLexer *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipBraceItems(JITLexer *__restrict self);

/* Parse a statement/expression or automatically parse either.
 * @param: kind:            One of `AST_PARSE_WASEXPR_*'
 * @param: p_was_expression: [OUT] One of `AST_PARSE_WASEXPR_*' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalTry(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalTryHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipTry(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipTryHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalDel(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalDelHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipDel(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipDelHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalIf(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalIfHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipIf(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipIfHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalFor(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalForHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipFor(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipForHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalForeach(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalForeachHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipForeach(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipForeachHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalWhile(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalWhileHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipWhile(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipWhileHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalDo(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalDoHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipDo(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipDoHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalWith(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalWithHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipWith(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipWithHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalAssert(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalAssertHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAssert(JITLexer *__restrict self, bool is_statement);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipAssertHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);

/* NOTE: Unlike other statements, the Import-statement parsers expect the
 *       lexer to point *after* the leading `import' or `from' keyword */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalImportHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipImportHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalImport(JITLexer *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipImport(JITLexer *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL JITLexer_EvalFromImport(JITLexer *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DFCALL JITLexer_SkipFromImport(JITLexer *__restrict self);

struct jit_import_item {
	char const                    *ii_symbol_name; /* [1..1] The name by which the item should be imported. */
	size_t                         ii_symbol_size; /* Length of `ii_symbol_name' (in characters) */
	DREF struct Dee_string_object *ii_import_name; /* [0..1] The name of the object being imported.
	                                       * When NULL, `ii_symbol_name' is used instead. */
};

/* Import a named module and bind it as a local variable. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
JITContext_DoImportModule(JITContext *__restrict self,
                          struct jit_import_item const *__restrict spec);

/* Import a named symbol from a module and binding it to a local symbol. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DFCALL
JITContext_DoImportSymbol(JITContext *__restrict self,
                          struct jit_import_item const *__restrict spec,
                          DeeObject *__restrict source_module);

/* Import a all symbols from a module and bind them to local symbols. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
JITContext_DoImportStar(JITContext *__restrict self,
                        DeeObject *__restrict source_module);


/* @return:  1: OK (when `allow_module_name' is true, a module import was parsed)
 * @return:  0: OK
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
JITLexer_EvalImportItem(JITLexer *__restrict self,
                        struct jit_import_item *__restrict result,
                        bool allow_module_name);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipImportItem(JITLexer *__restrict self,
                        bool allow_module_name);


/* Parse a class declaration, and return the produced class type.
 * Parsing starts after the `class' (or `class final'), meaning
 * that the current token is either:
 *   - `extends', `:' or `('     (followed by the class's base-type(s))
 *   - A keyword                 (the class name)
 *   - '{'                       (Start of the class body)
 * @param: tp_flags: Set of `0 | TP_FFINAL' */
INTDEF WUNUSED NONNULL((1)) DREF DeeTypeObject *DFCALL
JITLexer_EvalClass(JITLexer *__restrict self, uint16_t tp_flags);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipClass(JITLexer *__restrict self);


/* @param: p_was_expression: When non-NULL, set to one of `AST_PARSE_WASEXPR_*' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipHybrid(JITLexer *__restrict self, unsigned int *p_was_expression);

/* Same as `JITLexer_SkipHybrid()', but the current token is `{', and a trailing `;' should _NOT_ be consumed */
INTDEF WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipHybridAtBrace(JITLexer *__restrict self, unsigned int *p_was_expression);

#define JIT_AST_PARSE_WASEXPR_NO     0 /* It's a statement. */
#define JIT_AST_PARSE_WASEXPR_YES    1 /* It's an expression for sure. */
#define JIT_AST_PARSE_WASEXPR_MAYBE  2 /* It could either be an expression, or a statement. */


LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalHybridSecondary(JITLexer *__restrict self,
                             unsigned int *p_was_expression) {
	DREF DeeObject *result;
	if unlikely(!p_was_expression) {
		result = JITLexer_EvalHybrid(self, p_was_expression);
	} else {
		switch (*p_was_expression) {

		case JIT_AST_PARSE_WASEXPR_NO:
			result = JITLexer_EvalStatement(self);
			break;

		case JIT_AST_PARSE_WASEXPR_YES:
			result = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
			break;

		case JIT_AST_PARSE_WASEXPR_MAYBE:
			result = JITLexer_EvalHybrid(self, p_was_expression);
			break;

		default: __builtin_unreachable();
		}
	}
	return result;
}

LOCAL WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipHybridSecondary(JITLexer *__restrict self,
                             unsigned int *p_was_expression) {
	int result;
	if unlikely(!p_was_expression) {
		result = JITLexer_SkipHybrid(self, p_was_expression);
	} else {
		switch (*p_was_expression) {

		case JIT_AST_PARSE_WASEXPR_NO:
			result = JITLexer_SkipStatement(self);
			break;

		case JIT_AST_PARSE_WASEXPR_YES:
			result = JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL);
			break;

		case JIT_AST_PARSE_WASEXPR_MAYBE:
			result = JITLexer_SkipHybrid(self, p_was_expression);
			break;

		default: __builtin_unreachable();
		}
	}
	return result;
}


/* Wrapper for `JITLexer_EvalExpression()' which
 * automatically unwinds L-value expressions. */
#define JITLexer_SkipRValue(self) \
	JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL)
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalRValue(JITLexer *__restrict self) {
	DREF DeeObject *result;
	result = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
	ASSERT((result == JIT_LVALUE) ==
	       (self->jl_lvalue.lv_kind != JIT_LVALUE_NONE));
	if (result == JIT_LVALUE)
		result = JITLexer_PackLValue(self);
	return result;
}

#define JITLexer_SkipRValueDecl(self) \
	JITLexer_SkipComma(self, JIT_AST_COMMA_NORMAL | JIT_AST_COMMA_ALLOWVARDECLS, NULL)
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
JITLexer_EvalRValueDecl(JITLexer *__restrict self) {
	DREF DeeObject *result;
	result = JITLexer_EvalComma(self,
	                            JIT_AST_COMMA_NORMAL |
	                            JIT_AST_COMMA_ALLOWVARDECLS,
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
#define JIT_FUNCTION_FYIELDING 0x0002 /* The function contains a yield statement. */
struct jit_function_object {
	OBJECT_HEAD
	/*utf-8*/ char const   *jf_source_start; /* [1..1][const] Source start pointer. */
	/*utf-8*/ char const   *jf_source_end;   /* [1..1][const] Source end pointer. */
	DREF DeeObject         *jf_source;       /* [1..1][const] The object that owns input text. */
	DREF DeeObject         *jf_import;       /* [0..1][const] `import' function override (when NULL, use `deemon.operator . ("import")' instead) */
	DREF DeeModuleObject   *jf_impbase;      /* [0..1][const] Base module used for relative, static imports (such as `foo from .baz.bar')
	                                          * When `NULL', code isn't allowed to perform relative imports. */
	DREF DeeObject         *jf_globals;      /* [0..1][const] Mapping-like object for global variables. */
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
	/* TODO: Support for static variables. */
};

INTDEF DeeTypeObject JITFunction_Type;

/* Create a new JIT function object by parsing the specified
 * parameter list, and executing the given source region.
 * @param: context: The following fields are used:
 *                  - `jc_import'         (for `JITFunctionObject.jf_import')
 *                  - `jc_impbase'        (for `JITFunctionObject.jf_impbase')
 *                  - `jc_globals'        (for `JITFunctionObject.jf_globals')
 *                  - `jc_locals.otp_tab' (to scan for referenced variables)
 * @param: flags: Set of `JIT_FUNCTION_F*', optionally or'd with `JIT_FUNCTION_FTHISCALL' */
INTDEF WUNUSED NONNULL((5, 6, 7, 8)) DREF DeeObject *DCALL
JITFunction_New(/*utf-8*/ char const *name_start,
                /*utf-8*/ char const *name_end,
                /*utf-8*/ char const *params_start,
                /*utf-8*/ char const *params_end,
                /*utf-8*/ char const *source_start,
                /*utf-8*/ char const *source_end,
                JITContext const *__restrict context,
                DeeObject *__restrict source,
                uint16_t flags);

#define JIT_FUNCTION_FTHISCALL 0x8000 /* Special flag for `JITFunction_New()': Inject a hidden argument
                                       * at the start of the parameter-list with the name "this". */

/* Analyze the contents of an expression/statement for possible references
 * to symbols from surrounding scopes, or the use of `yield'. */
INTDEF NONNULL((1)) void DFCALL JITLexer_ScanExpression(JITLexer *__restrict self, bool allow_casts);
INTDEF NONNULL((1)) void DFCALL JITLexer_ScanStatement(JITLexer *__restrict self);

/* Assume that the given source text start/ends with `{' and `}'.
 * This function trims those characters, before also trimming any
 * additional whitespace next to them. */
INTDEF NONNULL((1, 2)) void DFCALL
JITFunction_TrimSurroundingBraces(/*utf-8*/ char const **__restrict p_source_start,
                                  /*utf-8*/ char const **__restrict p_source_end);



/* Yield-function support */
typedef struct jit_yield_function_object {
	OBJECT_HEAD
	DREF JITFunctionObject                   *jy_func;  /* [1..1][const] The underlying regular function object. */
	DREF DeeObject                           *jy_kw;    /* [0..1][const] Keyword arguments. */
	size_t                                    jy_argc;  /* [const] Number of positional arguments passed. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, jy_argv); /* [1..1][const][jy_argc] Vector of positional arguments. */
} JITYieldFunctionObject;




#define JIT_STATE_KIND_HASSCOPE(x) ((x) >= JIT_STATE_KIND_SCOPE2) /* Check if the given state kind has created a locals-scope */
#define JIT_STATE_KIND_SCOPE    0x0000 /* Simple scope */
#define JIT_STATE_KIND_DOWHILE  0x0001 /* Do-while loop (`do ... while (cond);') */
#define JIT_STATE_KIND_TRY      0x0002 /* try-statement (`try { ... } finally { ... } catch (...) { ... }') */
#define JIT_STATE_KIND_SCOPE2   0x0003 /* [SCOPE] Simple scope (including an associated `JITContext_PopScope()') */
#define JIT_STATE_KIND_FOR      0x0004 /* [SCOPE] For-statement (`for (local i = 0; i < 10; ++i) { ... }') */
#define JIT_STATE_KIND_WHILE    0x0005 /* [SCOPE] While-statement (`while (local item = getitem()) { ... }') */
#define JIT_STATE_KIND_FOREACH  0x0006 /* [SCOPE] Foreach-statement (`for (local x: items) { ... }') */
#define JIT_STATE_KIND_FOREACH2 0x0007 /* [SCOPE] Foreach-statement with multiple targets `for (local x, y: pairs) { ... }') */
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
#endif /* __SIZEOF_POINTER__ > 4 */
	union {
		struct {
			unsigned char const *f_loop;  /* [1..1] Pointer to the statement's loop-statement. */
			unsigned char const *f_cond;  /* [0..1] Pointer to the statement's cond-expression.
			                               * NOTE: This pointer is lazily initialized! */
		}             js_dowhile;         /* JIT_STATE_KIND_DOWHILE */
		struct {
			unsigned char const *f_cond;  /* [0..1] Pointer to the for statement's cond-expression. */
			unsigned char const *f_next;  /* [0..1] Pointer to the for statement's next-expression. */
			unsigned char const *f_loop;  /* [1..1] Pointer to the for statement's loop-statement. */
		}             js_for;             /* JIT_STATE_KIND_FOR */
		struct {
			unsigned char const *f_cond;  /* [1..1] Pointer to the for statement's cond-expression. */
			unsigned char const *f_loop;  /* [1..1] Pointer to the for statement's loop-statement. */
		}             js_while;           /* JIT_STATE_KIND_WHILE */
		struct {
			DREF DeeObject      *f_iter;  /* [1..1] The iterator object. */
			JITLValue            f_elem;  /* The iterator target expression lvalue (this is
			                               * where the elements enumerated from `f_iter' go). */
			unsigned char const *f_loop;  /* [1..1] Pointer to the foreach statement's loop-statement. */
		}             js_foreach;         /* JIT_STATE_KIND_FOREACH */
		struct {
			DREF DeeObject      *f_iter;  /* [1..1] The iterator object. */
			JITLValueList        f_elem;  /* The iterator target expression lvalues (this is
			                               * where the elements enumerated from `f_iter' go). */
			unsigned char const *f_loop;  /* [1..1] Pointer to the foreach statement's loop-statement. */
		}             js_foreach2;        /* JIT_STATE_KIND_FOREACH */
		struct {
			DREF DeeObject *w_obj;        /* [1..1] The with-object on which `operator leave()' is invoked when the scope is left. */
		}             js_with;            /* JIT_STATE_KIND_WITH */
		struct {
			unsigned char const *t_guard; /* [1..1] Pointer to the start of the guarded statement block. */
		}             js_try;             /* JIT_STATE_KIND_TRY */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define js_dowhile  _dee_aunion.js_dowhile
#define js_for      _dee_aunion.js_for
#define js_while    _dee_aunion.js_while
#define js_foreach  _dee_aunion.js_foreach
#define js_foreach2 _dee_aunion.js_foreach2
#define js_with     _dee_aunion.js_with
#define js_try      _dee_aunion.js_try
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

INTDEF NONNULL((1)) void DCALL jit_state_fini(struct jit_state *__restrict self);
#define jit_state_alloc()       DeeSlab_MALLOC(struct jit_state)
#define jit_state_free(self)    DeeSlab_FREE(self)
#define jit_state_destroy(self) (jit_state_fini(self), jit_state_free(self))



typedef struct jit_yield_function_iterator_object {
	OBJECT_HEAD  /* GC OBJECT */
#ifndef CONFIG_NO_THREADS
	Dee_rshared_lock_t           ji_lock;  /* Lock held while executing code of this iterator. */
#endif /* !CONFIG_NO_THREADS */
	DREF JITYieldFunctionObject *ji_func;  /* [1..1][const] The underlying yield-function. */
	JITLexer                     ji_lex;   /* [OVERRIDE(.jl_text, [const][== ji_func->jy_func->jf_source])]
	                                        * [OVERRIDE(.jl_context, [const][== &ji_ctx])]
	                                        * [lock(ji_lock)] The associated lexer. */
	JITContext                   ji_ctx;   /* [OVERRIDE(.jc_import, [const][== ji_func->jy_func->jf_import])]
	                                        * [OVERRIDE(.jc_impbase, [const][== ji_func->jy_func->jf_impbase])]
	                                        * [OVERRIDE(.jc_locals.otp_ind, [>= 1])]
	                                        * [OVERRIDE(.jc_locals.otp_tab, [owned_if(!= &ji_loc)])]
	                                        * [OVERRIDE(.jc_globals, [lock(WRITE_ONCE)]
	                                        *                        [if(ji_func->jy_func->jf_globals), [== ji_func->jy_func->jf_globals])]
	                                        *           )]
	                                        * [lock(ji_lock)] The associated JIT context. */
	JITObjectTable               ji_loc;   /* [OVERRIDE(.ot_prev.otp_ind, [>= 2])]
	                                        * [OVERRIDE(.ot_prev.otp_tab, [const][== &ji_func->jy_func->jf_refs])]
	                                        * [lock(ji_lock)] The base-level local variable table. */
	struct jit_state            *ji_state; /* [lock(ji_lock)][1..1][owned_if(!= &ji_bstat)]
	                                        * The current execution/syntax state. */
	struct jit_state             ji_bstat; /* [const][.js_kind == JIT_STATE_KIND_SCOPE] The base-level execution state. */
} JITYieldFunctionIteratorObject;

#define JITYieldFunctionIterator_Available(self)    Dee_rshared_lock_available(&(self)->ji_lock)
#define JITYieldFunctionIterator_Acquired(self)     Dee_rshared_lock_acquired(&(self)->ji_lock)
#define JITYieldFunctionIterator_TryAcquire(self)   Dee_rshared_lock_tryacquire(&(self)->ji_lock)
#define JITYieldFunctionIterator_Acquire(self)      Dee_rshared_lock_acquire(&(self)->ji_lock)
#define JITYieldFunctionIterator_AcquireNoInt(self) Dee_rshared_lock_acquire_noint(&(self)->ji_lock)
#define JITYieldFunctionIterator_WaitFor(self)      Dee_rshared_lock_waitfor(&(self)->ji_lock)
#define JITYieldFunctionIterator_Release(self)      Dee_rshared_lock_release(&(self)->ji_lock)

INTDEF DeeTypeObject JITYieldFunction_Type;
INTDEF DeeTypeObject JITYieldFunctionIterator_Type;



/* RT Exception handlers. */
INTDEF ATTR_COLD ATTR_INS(1, 2) int DCALL err_invalid_argc_len(char const *function_name, size_t function_size, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unknown_global(DeeObject *__restrict key);
INTDEF ATTR_COLD ATTR_INS(1, 2) int DCALL err_unknown_global_str_len(char const *__restrict key, size_t keylen);

/* Syntax Exception handlers. */
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_if_expected_lparen_after_if(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_if_expected_rparen_after_if(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_with_expected_lparen_after_with(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_with_expected_rparen_after_with(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_for_expected_lparen_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_for_expected_rparen_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_for_expected_semi1_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_for_expected_semi2_after_for(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_for_expected_rparen_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_throw_expected_semi_after_throw(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_yield_expected_semi_after_yield(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_break_expected_semi_after_break(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_continue_expected_semi_after_continue(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_return_expected_semi_after_return(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_foreach_expected_lparen_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_foreach_expected_colon_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_foreach_expected_rparen_after_foreach(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_while_expected_lparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_while_expected_rparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_dowhile_expected_while_after_do(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_dowhile_expected_lparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_dowhile_expected_rparen_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_dowhile_expected_semi_after_while(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_nonempty_string(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_lparen_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_rparen_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_string_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_semi_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_keyword_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_rbracket_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_string_before_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_lparen_before_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_rparen_after_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_asm_expected_keyword_for_label_operand(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_try_expected_lparen_after_catch(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_try_expected_rparen_after_catch(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_try_expected_keyword_after_as_in_catch(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_brace_expected_rbrace(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_brace_expected_keyword_after_dot(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_brace_expected_equals_after_dot(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_brace_expected_colon_after_key(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_expr_expected_semi_after_expr(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_expr_unexpected_token(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_function_expected_lparen_after_function(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_function_expected_arrow_or_lbrace_after_function(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_call_expected_rparen_after_call(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_bound_cannot_test(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_pack_expected_rparen_after_lparen(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_paren_expected_rparen_after_lparen(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_bracket_expected_rbracket_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_attr_expected_keyword(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_item_expected_rbracket_after_lbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_isin_expected_is_or_in_after_exclaim(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_operator_expected_empty_string(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_operator_expected_lbracket_or_dot_after_del(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_operator_unknown_name(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_anno_expected_rbracket(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_class_expected_class_after_final(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_class_expected_rparen_after_lparen_base(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_class_expected_lbrace_after_class(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_class_expected_rbrace_after_class(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_class_not_thiscall(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_nth_expected_lparen(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_nth_expected_rparen(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_type_annotation_unexpected_token(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_type_annotation_expected_dots_or_colon(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_type_annotation_expected_rbrace(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_type_annotation_expected_rparen(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_type_annotation_expected_string_after_asm(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_template_string_unterminated(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_template_string_unmatched_lbrace(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_template_string_unmatched_rbrace(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_template_string_no_digit_or_hex_after_backslash_x_u_U(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_template_string_undefined_escape(JITLexer *__restrict self, int ch);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_expected_dot_keyword_or_string(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_expected_keyword_after_as(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_expected_keyword_or_string_in_import_list(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1, 2)) int DFCALL syn_import_invalid_name_for_module_symbol(JITLexer *__restrict self, struct jit_import_item const *__restrict item);
INTDEF ATTR_COLD NONNULL((1, 2)) int DFCALL syn_import_invalid_name_for_import_symbol(JITLexer *__restrict self, struct jit_import_item const *__restrict item);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_expected_comma_or_from_after_star(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_expected_from_after_symbol_import_list(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_expected_import_after_from(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_unexpected_from_after_module_import_list(JITLexer *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DFCALL syn_import_unexpected_star_duplication_in_import_list(JITLexer *__restrict self);

INTDEF ATTR_COLD int (DFCALL err_cannot_import_relative)(char const *module_name, size_t module_namelen);

/* TODO: Dee_ASSUMED_VALUE-optimizations for all of the errors above! */

DECL_END

#endif /* !GUARD_DEX_STREXEC_LIBJIT_H */
