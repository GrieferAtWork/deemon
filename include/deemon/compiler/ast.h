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
/*!export **/
/*!export AST_**/
/*!export AST_FACTION_**/
/*!export AST_FASSEMBLY_**/
/*!export AST_FBOOL_**/
/*!export AST_FCLASS_**/
/*!export AST_FCOND_**/
/*!export AST_FLABEL_**/
/*!export AST_FLOOPCTL_**/
/*!export AST_FLOOP_**/
/*!export AST_FMULTIPLE_**/
/*!export AST_FSWITCH_**/
/*!export AST_OPERATOR_**/
/*!export CATCH_EXPR_F**/
/*!export ast_**/
#ifndef GUARD_DEEMON_COMPILER_AST_H
#define GUARD_DEEMON_COMPILER_AST_H 1 /*!export-*/

#include "../api.h"

#include <hybrid/__minmax.h>
#include <hybrid/sequence/list.h>

#include "../alloc.h"  /* DeeDbgObject_MALLOC, DeeObject_FREE, DeeObject_MALLOC */
#include "../module.h" /* DeeModule_Deemon, Dee_module_symbol_getindex */
#include "../none.h"   /* DeeNone_Check */
#include "../object.h" /* ASSERT_OBJECT_TYPE_EXACT, ASSERT_OBJECT_TYPE_EXACT_OPT, Dee_Decref*, Dee_Incref, Dee_XDecref, Dee_XDecref_unlikely, Dee_XIncref */
#include "../type.h"   /* DeeObject_IsShared, Dee_operator_t, OPERATOR_COPY */
#include "../types.h"  /* DREF, DeeObject, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD, Dee_refcnt_t */
#include "lexer.h"     /* DeeLexer, TPP_REF, ast_loc, tpp_* */
#include "symbol.h"    /* DeeBaseScopeObject, DeeScopeObject, SYMBOL_TYPE_EXTERN, symbol, text_label */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* uint16_t */


DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER

/* Dummy array size for variable-length ASTs */
#define Dee_AST_FLEXARRAY 1024

typedef struct Dee_ast DeeAst;
typedef struct Dee_ast_rvalue DeeAstRValue;
struct Dee_link_symbol; /* Link-time symbol declaration */
#define Dee_link_symbol_incref(self)   (void)0 /* TODO */
#define Dee_link_symbol_decref(self)   (void)0 /* TODO */
#define Dee_link_symbol_xdecref(self)  (void)0 /* TODO */
#define Dee_link_symbol_rd_inc(self)   (void)0 /* Increment read-counter */
#define Dee_link_symbol_rd_dec(self)   (void)0 /* Decrement read-counter */
#define Dee_link_symbol_wr_inc(self)   (void)0 /* Increment write-counter */
#define Dee_link_symbol_wr_dec(self)   (void)0 /* Decrement write-counter */
#define Dee_link_symbol_rdwr_inc(self) (Dee_link_symbol_rd_inc(self), Dee_link_symbol_wr_inc(self))
#define Dee_link_symbol_rdwr_dec(self) (Dee_link_symbol_wr_dec(self), Dee_link_symbol_rd_dec(self))

/* AST allocators for fixed-size ASTs */
#define _DeeAst_SizeofFixed(field)                                 COMPILER_OFFSETAFTER(DeeAst, a_data.field)
#define _DeeAst_MallocUninitializedFixed(field)                    ((DeeAst *)DeeObject_FMalloc(_DeeAst_SizeofFixed(field)))
#define _DeeAst_TryMallocUninitializedFixed(field)                 ((DeeAst *)DeeObject_FTryMalloc(_DeeAst_SizeofFixed(field)))
#define _DeeAst_FreeUninitializedFixed(field, p)                   DeeObject_FFree(p, _DeeAst_SizeofFixed(field))
#define _DeeAst_Dbg_MallocUninitializedFixed(field, file, line)    ((DeeAst *)DeeDbgObject_FMalloc(_DeeAst_SizeofFixed(field), file, line))
#define _DeeAst_Dbg_TryMallocUninitializedFixed(field, file, line) ((DeeAst *)DeeDbgObject_FTryMalloc(_DeeAst_SizeofFixed(field), file, line))
#define _DeeAst_Dbg_FreeUninitializedFixed(field, p, file, line)   DeeDbgObject_FFree(p, _DeeAst_SizeofFixed(field), file, line)

/* AST allocators for variable-length ASTs */
#define _DeeAst_SizeofVar(field, count)                                     (offsetof(DeeAst, a_data.field) + ((count) * sizeof(((DeeAst *)0)->a_data.field[0])))
#define _DeeAst_MallocUninitializedVar(field, count)                        ((DeeAst *)Dee_Malloc(_DeeAst_SizeofVar(field, count)))
#define _DeeAst_TryMallocUninitializedVar(field, count)                     ((DeeAst *)Dee_TryMalloc(_DeeAst_SizeofVar(field, count)))
#define _DeeAst_ReallocUninitializedVar(field, p, count)                    ((DeeAst *)Dee_Realloc(p, _DeeAst_SizeofVar(field, count)))
#define _DeeAst_TryReallocUninitializedVar(field, p, count)                 ((DeeAst *)Dee_TryRealloc(p, _DeeAst_SizeofVar(field, count)))
#define _DeeAst_FreeUninitializedVar(p)                                     Dee_Free(p)
#define _DeeAst_Dbg_MallocUninitializedVar(field, count, file, line)        ((DeeAst *)DeeDbg_Malloc(_DeeAst_SizeofVar(field, count), file, line))
#define _DeeAst_Dbg_TryMallocUninitializedVar(field, count, file, line)     ((DeeAst *)DeeDbg_TryMalloc(_DeeAst_SizeofVar(field, count), file, line))
#define _DeeAst_Dbg_ReallocUninitializedVar(field, p, count, file, line)    ((DeeAst *)DeeDbg_Realloc(p, _DeeAst_SizeofVar(field, count), file, line))
#define _DeeAst_Dbg_TryReallocUninitializedVar(field, p, count, file, line) ((DeeAst *)DeeDbg_TryRealloc(p, _DeeAst_SizeofVar(field, count), file, line))
#define _DeeAst_Dbg_FreeUninitializedVar(p, file, line)                     DeeDbg_Free(p, file, line)

struct Dee_asm_symbol {
	int placeholder;
};
#define Dee_asm_symbol_init(self)    (void)0 /* Initialize as empty/unbound (populated later during assembly) */
#define Dee_asm_symbol_fini(self)    (void)0
#define Dee_asm_symbol_getname(self) ((char const *)NULL) /* User-defined name for symbol, or "NULL" if unnamed */

struct Dee_ast_typedef {
	/* Type annotation (as in: `local x: {int...}`) */
	/* TODO: This also needs to include the doc-string */
	int placeholder;
};
#define Dee_ast_typedef_init(self) (void)0 /* Initialize as empty */
#define Dee_ast_typedef_fini(self) (void)0

struct Dee_ast_operator_id {
	Dee_operator_t aoi_id; /* Operator ID */
	/* TODO: Operator name as a string (such that the ID can be looked up at runtime,
	 *       when the type of the associated this-argument is known, and the operator
	 *       can be queried using `DeeTypeType_GetOperatorByName()`) */
};
#define Dee_ast_operator_id_initid(self, id) (void)((self)->aoi_id = (id))
#define Dee_ast_operator_id_move(self, from) (void)(*(self) = *(from))
#define Dee_ast_operator_id_fini(self)       (void)0
#define Dee_ast_operator_id_hasid(self)      true
#define Dee_ast_operator_id_getid(self)      ((self)->aoi_id)
#define Dee_ast_operator_id_setid(self, v)   (void)((self)->aoi_id = (v)) /* Only use if `Dee_ast_operator_id_hasid(self) == true` */



enum Dee_ast_symref_kind {
	Dee_AST_SYMREF_KIND_BYNAME,  /* Simple named symbol reference */
	Dee_AST_SYMREF_KIND_NTH,     /* Simple named symbol reference, but link against Nth match */
	Dee_AST_SYMREF_KIND_EXTERN,  /* Reference to `symbol from module` */
	Dee_AST_SYMREF_KIND_THIS,    /* Reference to `this` (s.a. `struct Dee_ast_function::af_symbol_this`) */
#define Dee_AST_SYMREF_KIND_ISSIMPLE(x) ((unsigned int)(x) >= (unsigned int)Dee_AST_SYMREF_KIND_THIS)
	Dee_AST_SYMREF_KIND_TP_THIS, /* Reference to TYPE in `this as Type.__base__(TYPE)`, as used to implement `super` */
	Dee_AST_SYMREF_KIND_THISMOD, /* Reference to the current module */
	Dee_AST_SYMREF_KIND_VARARGS, /* Reference to the varargs argument of the current function */
};

enum Dee_ast_symref_byname_kind {
	Dee_AST_SYMREF_BYNAME_KIND_AUTO,   /* <no prefix>     (find first matching symbol) */
	Dee_AST_SYMREF_BYNAME_KIND_DECL,   /* <no prefix>     (declare as either `local` or `global` -- used for named function/class declaration) */
	Dee_AST_SYMREF_BYNAME_KIND_LOCAL,  /* `local`         (Create/use local- or stack-variable in current scope) */
	Dee_AST_SYMREF_BYNAME_KIND_STACK,  /* `__stack local` (Create/use stack-variable in current scope) */
	Dee_AST_SYMREF_BYNAME_KIND_GLOBAL, /* `global`        (Create/use global variable in current scope (if create, and current scope isn't global scope, emit a warning)) */
	Dee_AST_SYMREF_BYNAME_KIND_STATIC, /* `static local`  (Create/use static variable in current scope) */
};
typedef uint_fast8_t Dee_ast_symref_byname_flag;
#define Dee_AST_SYMREF_BYNAME_FLAG_NORMAL  0x00 /* Normal flags */
#define Dee_AST_SYMREF_BYNAME_FLAG_FINAL   0x01 /* `final` modifier used (ignored if `Dee_AST_SYMREF_BYNAME_KIND_AUTO` is used) */
#define Dee_AST_SYMREF_BYNAME_FLAG_VARYING 0x02 /* `varying` modifier used (ignored if `Dee_AST_SYMREF_BYNAME_KIND_AUTO` is used) */

struct Dee_ast_symref_byname {
	tpp_keyword const              *asrbn_name;    /* [1..1] Symbol name */
	enum Dee_ast_symref_byname_kind asrbn_kind;    /* Scope specifier */
	Dee_ast_symref_byname_flag      asrbn_flag;    /* Variable flags */
	struct Dee_ast_typedef          asrbn_typedef; /* Type annotation for symbol */
};
struct Dee_ast_symref_nth {
	tpp_keyword const *asrnt_name; /* [1..1] Symbol name */
	DeeAst            *asrnt_nth;  /* [1..1][owned] Expression describing which match to use (where `0` means the same as `Dee_AST_SYMREF_KIND_BYNAME` + `Dee_AST_SYMREF_BYNAME_KIND_AUTO`) */
};

struct Dee_module_symbol;
struct Dee_module_object;
struct Dee_ast_symref_extern {
	DREF struct Dee_module_object *asrex_mod; /* [1..1] Accessed module */
	struct Dee_module_symbol      *asrex_sym; /* [1..1] Accessed symbol */
};
struct Dee_ast_symref {
	enum Dee_ast_symref_kind asr_kind; /* Reference kind */
	union {
		struct Dee_ast_symref_byname asr_byname; /* [valid_if(asr_kind == Dee_AST_SYMREF_KIND_BYNAME)] */
		struct Dee_ast_symref_nth    asr_nth;    /* [valid_if(asr_kind == Dee_AST_SYMREF_KIND_NTH)] */
		struct Dee_ast_symref_extern asr_extern; /* [valid_if(asr_kind == Dee_AST_SYMREF_KIND_EXTERN)] */
	} asr_data;
};
#define Dee_ast_symref_init_byname(self, name, kind, flag)  \
	(void)((self)->asr_kind = Dee_AST_SYMREF_KIND_BYNAME,   \
	       (self)->asr_data.asr_byname.asrbn_name = (name), \
	       (self)->asr_data.asr_byname.asrbn_kind = (kind), \
	       (self)->asr_data.asr_byname.asrbn_flag = (flag), \
	       Dee_ast_typedef_init(&(self)->asr_data.asr_byname.asrbn_typedef))
#define Dee_ast_symref_init_nth(self, name, /*inherit(always)*/ nth) \
	(void)((self)->asr_kind = Dee_AST_SYMREF_KIND_NTH,               \
	       (self)->asr_data.asr_nth.asrnt_name = (name),             \
	       (self)->asr_data.asr_nth.asrnt_nth  = (nth))
#define Dee_ast_symref_init_extern(self, /*inherit(always)*/ mod, sym) \
	(void)((self)->asr_kind = Dee_AST_SYMREF_KIND_EXTERN,              \
	       (self)->asr_data.asr_extern.asrex_mod = (mod),              \
	       (self)->asr_data.asr_extern.asrex_sym = (sym))
#define Dee_ast_symref_init_simple(self, kind)             \
	(void)(Dee_ASSERT(Dee_AST_SYMREF_KIND_ISSIMPLE(kind)), \
	       (self)->asr_kind = (kind))
#define Dee_ast_symref_fini(self)                                             \
	do {                                                                      \
		switch ((self)->asr_kind) {                                           \
		case Dee_AST_SYMREF_KIND_BYNAME:                                      \
			Dee_ast_typedef_fini(&(self)->asr_data.asr_byname.asrbn_typedef); \
			break;                                                            \
		case Dee_AST_SYMREF_KIND_NTH:                                         \
			DeeAst_Destroy((self)->asr_data.asr_nth.asrnt_nth);               \
			break;                                                            \
		case Dee_AST_SYMREF_KIND_EXTERN:                                      \
			Dee_Decref((self)->asr_data.asr_extern.asrex_mod);                \
			break;                                                            \
		default: break;                                                       \
		}                                                                     \
	}	__WHILE0
#define Dee_ast_symref_move(self, from) (void)(*(self) = *(from))


enum Dee_ast_label_kind {
	Dee_AST_LABEL_KIND_GOTO, /* Goto-style label */
	Dee_AST_LABEL_KIND_CASE, /* switch-case/default label */
};

struct Dee_ast_label_goto {
	tpp_keyword const *alg_name; /* [1..1] Name of goto-label */
};

struct Dee_ast_label;
struct Dee_ast_label_case {
	DeeAst                   *alc_case;  /* [0..1][owned] Case-expr (or `NULL` if this is the default-case) */
	LIST_ENTRY(Dee_ast_label) alc_entry; /* [0..1][valid_if(WAS_LINKED)] Entry in list of case-labels of switch-statement */
};

struct Dee_ast_label {
	struct Dee_asm_symbol   al_symbol; /* Assembly symbol of label */
	enum Dee_ast_label_kind al_kind;   /* Label kind */
	union {
		struct Dee_ast_label_goto ald_goto; /* [valid_if(al_kind == Dee_AST_LABEL_KIND_GOTO)] */
		struct Dee_ast_label_case ald_case; /* [valid_if(al_kind == Dee_AST_LABEL_KIND_CASE)] */
	} al_data;
};

#define Dee_ast_label_fini(self)                                    \
	(Dee_asm_symbol_fini(&(self)->al_symbol),                       \
	 (self)->al_kind == Dee_AST_LABEL_KIND_CASE                     \
	 ? (void)(DeeAst_XDestroy((self)->al_data.ald_case.alc_case),   \
	          LIST_ISBOUND(self, al_data.ald_case.alc_entry)        \
	          ? (void)LIST_REMOVE(self, al_data.ald_case.alc_entry) \
	          : (void)0)                                            \
	 : (void)0)

struct Dee_ast_labels {
	size_t                                        al_count;   /* Label count */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_ast_label, al_labels); /* [al_count] Labels */
};

#define Dee_ast_labels_sizeof(count)        (offsetof(struct Dee_ast_labels, al_labels) + ((count) * sizeof(struct Dee_ast_label)))
#define Dee_ast_labels_malloc(count)        ((struct Dee_ast_labels *)Dee_Malloc(Dee_ast_labels_sizeof(count)))
#define Dee_ast_labels_trymalloc(count)     ((struct Dee_ast_labels *)Dee_TryMalloc(Dee_ast_labels_sizeof(count)))
#define Dee_ast_labels_realloc(p, count)    ((struct Dee_ast_labels *)Dee_Realloc(p, Dee_ast_labels_sizeof(count)))
#define Dee_ast_labels_tryrealloc(p, count) ((struct Dee_ast_labels *)Dee_TryRealloc(p, Dee_ast_labels_sizeof(count)))
#define Dee_ast_labels_free(self)           Dee_Free(self)
#define Dee_ast_labels_destroy(self)                                         \
	do {                                                                     \
		size_t _al_i;                                                        \
		for (_al_i = 0; _al_i < (self)->al_count; ++_al_i) {                 \
			struct Dee_ast_label *const _al_lbl = &(self)->al_labels[_al_i]; \
			Dee_ast_label_fini(_al_lbl);                                     \
		}                                                                    \
		Dee_ast_labels_free(self);                                           \
	}	__WHILE0


/* === For `Dee_AST_UNPACK_WR` */
enum Dee_ast_rvalue_kind {
	Dee_AST_RVALUE_KIND_NONE,     /* <Dee_AST_CONST+Dee_None> Discard */
	Dee_AST_RVALUE_KIND_SYMBOL,   /* <Dee_AST_SYMBOL_RD> Store into a linked symbol */
	Dee_AST_RVALUE_KIND_SYMREF,   /* <Dee_AST_SYMREF_RD> Store into a referenced symbol */
	Dee_AST_RVALUE_KIND_SETATTR,  /* <Dee_AST_[I]OPERATOR + OPERATOR_GETATTR> Store into a `obj.attr` expression */
	Dee_AST_RVALUE_KIND_SETITEM,  /* <Dee_AST_[I]OPERATOR + OPERATOR_GETITEM> Store into a `obj[item]` expression */
	Dee_AST_RVALUE_KIND_SETRANGE, /* <Dee_AST_[I]OPERATOR + OPERATOR_GETRANGE> Store into a `obj[start:end]` expression */
	Dee_AST_RVALUE_KIND_SETTER,   /* Call a setter-function */
	Dee_AST_RVALUE_KIND_ASSIGN,   /* <*> Store into a `obj := ...` expression (only produced when necessary) */
};
struct Dee_ast_rvalue_setattr {
	/* `<obj>.operator .= (<attr>, VALUE)` */
	DeeAst *arvsa_obj;  /* [1..1][owned] `obj` */
	DeeAst *arvsa_attr; /* [1..1][owned] `attr` */
};

struct Dee_ast_rvalue_setitem {
	/* `<obj>[<item>] = VALUE` */
	DeeAst *arvsi_obj;  /* [1..1][owned] `obj` */
	DeeAst *arvsi_item; /* [1..1][owned] `item` */
};

struct Dee_ast_rvalue_setrange {
	/* `<obj>[<start>:<end>] = VALUE` */
	DeeAst *arvsr_obj;   /* [1..1][owned] `obj` */
	DeeAst *arvsr_start; /* [1..1][owned] `start` */
	DeeAst *arvsr_end;   /* [1..1][owned] `end` */
};

struct Dee_ast_rvalue_assign {
	/* `<obj> := VALUE` */
	DeeAst *arva_obj; /* [1..1][owned] `obj` */
};

struct Dee_ast_rvalue_setter {
	/* `<setter>(VALUE)` */
	DeeAst *arvs_setter; /* [1..1][owned] `setter` */
};

struct Dee_ast_rvalue {
	enum Dee_ast_rvalue_kind           arv_kind;      /* Target kind */
	struct Dee_ast_labels             *arv_labels;    /* [0..n][owned] AST labels */
	DeeAstLoc                          arv_loc;       /* Location info */
	union {
		DREF struct Dee_link_symbol   *arvd_symbol;   /* [1..1][valid_if(arv_kind == Dee_AST_RVALUE_KIND_SYMBOL)] Linked symbol */
		struct Dee_ast_symref          arvd_symref;   /* [valid_if(arv_kind == Dee_AST_RVALUE_KIND_SYMREF)] Referenced symbol */
		struct Dee_ast_rvalue_setattr  arvd_setattr;  /* [valid_if(arv_kind == Dee_AST_RVALUE_KIND_SETATTR)] */
		struct Dee_ast_rvalue_setitem  arvd_setitem;  /* [valid_if(arv_kind == Dee_AST_RVALUE_KIND_SETITEM)] */
		struct Dee_ast_rvalue_setrange arvd_setrange; /* [valid_if(arv_kind == Dee_AST_RVALUE_KIND_SETRANGE)] */
		struct Dee_ast_rvalue_assign   arvd_assign;   /* [valid_if(arv_kind == Dee_AST_RVALUE_KIND_ASSIGN)] */
		struct Dee_ast_rvalue_setter   arvd_setter;   /* [valid_if(arv_kind == Dee_AST_RVALUE_KIND_SETTER)] */
	} arv_data;
};

enum Dee_ast_rvalue_mode {
	Dee_AST_RVALUE_MODE_RD   = 1, /* Read from symbol (should never be used on its own) */
	Dee_AST_RVALUE_MODE_WR   = 2, /* Write to symbol */
	Dee_AST_RVALUE_MODE_RDWR = 3, /* Read+write symbol */
};






enum Dee_ast_kind {
	Dee_AST_CONST,       /* Compile-time constant expression */
	Dee_AST_MULTI,       /* Multi-ast aggregation (statements, sequences, tuples, lists, dicts, ...) */
	Dee_AST_LOOP,        /* Loop-statement */
	Dee_AST_IF,          /* If-statement */
	Dee_AST_SWITCH_STMT, /* Switch-statement (`switch (expr) { case 10: ...; default: ...; }`) */
	Dee_AST_SWITCH_EXPR, /* Switch-expression (`switch (expr) { case 10 -> ...; default -> ...; }`) */
	Dee_AST_ASM_GOTO,    /* Assembly-level goto (used to implement `goto`, `break` and `continue`) */
	Dee_AST_CATCH,       /* Try-catch statement */
	Dee_AST_FINALLY,     /* Try-finally statement */
	Dee_AST_YIELD,       /* Yield-statement */
	Dee_AST_RETURN,      /* Return-statement */
	Dee_AST_FUNCTION,    /* Function-constructor */
	Dee_AST_CLASS,       /* Class-constructor */
	Dee_AST_UNPACK_WR,   /* Unpack-and-store */
	Dee_AST_IASM,        /* Undefined-defined inline assembly */
	Dee_AST_OPERATOR,    /* Invoke operator */
	Dee_AST_IOPERATOR,   /* Invoke inplace operator */
	/* HINT: AST_OPERATOR_FUNC is emulated as:
	 * - `operator <name>`         ->  `(<name> from operators)`
	 * - `<base>.operator <name>`  ->  `(InstanceMethod from deemon)(<name> from operators, <base>)` */

	/* Action AST type codes... */
#define Dee_AST_ISACT1(id)                                     \
	((unsigned int)(id) >= (unsigned int)Dee_AST_ACT1_SCOPE && \
	 (unsigned int)(id) <= (unsigned int)Dee_AST_ACT1_BOUND)
#define Dee_AST_CASE_ACT1       \
	case Dee_AST_ACT1_SCOPE:    \
	case Dee_AST_ACT1_NOT:      \
	case Dee_AST_ACT1_TYPEOF:   \
	case Dee_AST_ACT1_CLASSOF:  \
	case Dee_AST_ACT1_SUPEROF:  \
	case Dee_AST_ACT1_PRINT:    \
	case Dee_AST_ACT1_PRINTLN:  \
	case Dee_AST_ACT1_ASSERT:   \
	case Dee_AST_ACT1_DEEPCOPY: \
	case Dee_AST_ACT1_UNPACK:   \
	case Dee_AST_ACT1_DEL:      \
	case Dee_AST_ACT1_UNBIND:   \
	case Dee_AST_ACT1_BOUND:
	Dee_AST_ACT1_SCOPE,      /* `{ <act0>; }`               - Wrap `<act0>` in a child-scope (removed during linking) */
	Dee_AST_ACT1_NOT,        /* `!<act0>`                   - Convert `<act0>` into a boolean and negate the result */
	Dee_AST_ACT1_TYPEOF,     /* `type(<act0>)`              - Return the actual type of `<act0>` */
	Dee_AST_ACT1_CLASSOF,    /* `<act0>.class`              - Return the effective class of `<act0>` (Dereference super objects) */
	Dee_AST_ACT1_SUPEROF,    /* `<act0>.super`              - Return the super-object of `<act0>` */
	Dee_AST_ACT1_PRINT,      /* `print <act0>...,;`         - Print the elements of a sequence in `<act0>` (without linefeed) */
	Dee_AST_ACT1_PRINTLN,    /* `print <act0>...;`          - Print the elements of a sequence in `<act0>` */
	Dee_AST_ACT1_ASSERT,     /* `assert <act0>`             - Assert that <act0> evaluates to true at runtime. */
	Dee_AST_ACT1_DEEPCOPY,   /* `deepcopy <act0>`           - Create a deep copy of `act0` */
	Dee_AST_ACT1_UNPACK,     /* `<act0>...`                 - Unpack `act0` into a context-sensitive # of objects (without special handling: unpack to 1 object) */
	Dee_AST_ACT1_DEL,        /* `del <act0>`                - Unbind `<act0>` and return `none` and (if it's a symbol, and that symbol was declared in the current scope), delete said symbol (replaced with other kinds of ASTs during linking) */
	Dee_AST_ACT1_UNBIND,     /* `del(<act0>)`               - Unbind `<act0>` and return `none` (replaced with other kinds of ASTs during linking) */
	Dee_AST_ACT1_BOUND,      /* `<act0>...`                 - Check if `<act0>` is bound and return `true` / `false` (replaced with other kinds of ASTs during linking) */
#define Dee_AST_ISACT2(id)                                      \
	((unsigned int)(id) >= (unsigned int)Dee_AST_ACT2_FPRINT && \
	 (unsigned int)(id) <= (unsigned int)Dee_AST_ACT2_XCH)
#define Dee_AST_CASE_ACT2        \
	case Dee_AST_ACT2_FPRINT:    \
	case Dee_AST_ACT2_FPRINTLN:  \
	case Dee_AST_ACT2_IS:        \
	case Dee_AST_ACT2_IN:        \
	case Dee_AST_ACT2_AS:        \
	case Dee_AST_ACT2_ASSERT:    \
	case Dee_AST_ACT2_BOUNDATTR: \
	case Dee_AST_ACT2_BOUNDITEM: \
	case Dee_AST_ACT2_SAMEOBJ:   \
	case Dee_AST_ACT2_DIFFOBJ:   \
	case Dee_AST_ACT2_STORE:     \
	case Dee_AST_ACT2_XCH:

	Dee_AST_ACT2_FPRINT,    /* `print <act0>: <act1>...,;` - Print the elements of a sequence in `<act1>` to a file in `<act0>` (without linefeed) */
	Dee_AST_ACT2_FPRINTLN,  /* `print <act0>: <act1>...;`  - Print the elements of a sequence in `<act1>` to a file in `<act0>` */
	Dee_AST_ACT2_IS,        /* `<act0> is <act1>`          - Generate a test to check if `<act0>` ~is~ `<act1>` (instanceof / is none) */
	Dee_AST_ACT2_IN,        /* `<act0> in <act1>`          - A variant of `Dee_AST_OPERATOR:OPERATOR_CONTAINS` that evaluates arguments in reverse order. */
	Dee_AST_ACT2_AS,        /* `<act0> as <act1>`          - Create a super wrapper object for `<act0>` of type `<act1>` */
	Dee_AST_ACT2_ASSERT,    /* `assert <act0>, <act1>`     - Assert that <act0> evaluates to true at runtime,
	                         *                               <act1> is a message that is evaluated and added
	                         *                               to the error message when the assertion fails. */
	Dee_AST_ACT2_BOUNDATTR, /* `<act0>.operator . (<act1>) is bound` - Check if the attribute `<act1>` of `<act0>` is currently bound. */
	Dee_AST_ACT2_BOUNDITEM, /* `<act0>.operator [] (<act1>) is bound` - Check if the item `<act1>` of `<act0>` is currently bound. */
	Dee_AST_ACT2_SAMEOBJ,   /* `<act0> === <act1>`         - Check if <act0> and <act1> are the same object */
	Dee_AST_ACT2_DIFFOBJ,   /* `<act0> !== <act1>`         - Check if <act0> and <act1> are the different objects */
	Dee_AST_ACT2_STORE,     /* `<act0> = <act1>`           - Store `<act1>` in `<act0>` and return value of `<act1>` (replaced with other kinds of ASTs during linking) */
	Dee_AST_ACT2_INIT,      /* `if (<act0> !is bound) <act0> = <act1>` - Same as `Dee_AST_ACT2_STORE`, but only run initializer if `<act0>` was unbound (replaced with other kinds of ASTs during linking) */
	Dee_AST_ACT2_XCH,       /* `xch(<act0>, <act1>)`       - Store `<act1>` in `<act0>` and return old value of `<act0>` (replaced with other kinds of ASTs during linking) */
#define Dee_AST_ISACT3(id)                                     \
	((unsigned int)(id) >= (unsigned int)Dee_AST_ACT3_RANGE && \
	 (unsigned int)(id) <= (unsigned int)Dee_AST_ACT3_RANGE)
#define Dee_AST_CASE_ACT3 case Dee_AST_ACT3_RANGE:
	Dee_AST_ACT3_RANGE,     /* `[<act0>:<act1>, <act2>]`   - Create a new range object for enumerating indices from `<act0>` to `<act1>`, using `<act2>` as step. (constexpr(none) is used to indicate unset action operands) */

#define Dee_AST_ISSYM(x)                                     \
	((unsigned int)(x) >= (unsigned int)Dee_AST_SYMBOL_RD && \
	 (unsigned int)(x) <= (unsigned int)Dee_AST_SYMREF_XCH)
	Dee_AST_SYMBOL_RD,     /* Read from symbol */
	Dee_AST_SYMBOL_BND,    /* Check if symbol is bound */
	Dee_AST_SYMBOL_DEL,    /* Unbind symbol */
	Dee_AST_SYMBOL_WR,     /* Write to symbol */
	Dee_AST_SYMBOL_XCH,    /* Exchange value of symbol */
	Dee_AST_SYMBOL_INIT,   /* Initialize value of symbol (initialize is only executed if symbol was unbound;
	                        * for static symbols, initializer is additionally guarded by a lock to enforce
	                        * init-once semantics) */

	/* The following AST kinds only exist before linking */
	Dee_AST_SYMREF,        /* Access a symbol */
	Dee_AST_SYMREF_ALIAS,  /* Declare a symbol alias (alternate name for the same symbol -- used to implement `import`) */
	Dee_AST_IMPORT_STAR,   /* Declare symbols for all elements */

	Dee_AST_GOTO,     /* Goto label (replaced with `Dee_AST_ASM_GOTO` during linking) */
	Dee_AST_BREAK,    /* break-statement (replaced with `Dee_AST_ASM_GOTO` during linking) */
	Dee_AST_CONTINUE, /* continue-statement (replaced with `Dee_AST_ASM_GOTO` during linking) */
};

/* === For `Dee_AST_CONST` */
struct Dee_ast_const {
	DREF DeeObject *ace_value; /* [1..1] Constant expression value */
};
#define DeeAst_MallocUninitializedConst()                  _DeeAst_MallocUninitializedFixed(ad_const)
#define DeeAst_TryMallocUninitializedConst()               _DeeAst_TryMallocUninitializedFixed(ad_const)
#define DeeAst_FreeUninitializedConst(p)                   _DeeAst_FreeUninitializedFixed(ad_const, p)
#define DeeAst_Dbg_MallocUninitializedConst(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_const, file, line)
#define DeeAst_Dbg_TryMallocUninitializedConst(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_const, file, line)
#define DeeAst_Dbg_FreeUninitializedConst(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_const, p, file, line)
/* === */


/* === For `Dee_AST_MULTI` */
enum Dee_ast_multi_kind {
	Dee_AST_MULTI_KIND_LAST,        /* Evaluate all asts and return the value of the last one (if there are none, return `Dee_None`) */
	Dee_AST_MULTI_KIND_GENERIC_SEQ, /* Create some abstract object derived from `deemon.Sequence` */
	Dee_AST_MULTI_KIND_GENERIC_SET, /* Create some abstract object derived from `deemon.Set` */
	Dee_AST_MULTI_KIND_TUPLE,       /* Create a `deemon.Tuple` object */
	Dee_AST_MULTI_KIND_LIST,        /* Create a `deemon.List` object */
	Dee_AST_MULTI_KIND_HASHSET,     /* Create a `deemon.HashSet` object */
#define Dee_AST_MULTI_KIND_ISSEQ(x)                                       \
	((unsigned int)(x) >= (unsigned int)Dee_AST_MULTI_KIND_GENERIC_SEQ && \
	 (unsigned int)(x) <= (unsigned int)Dee_AST_MULTI_KIND_HASHSET)
#define Dee_AST_MULTI_KIND_ISMAP(x) ((unsigned int)(x) >= (unsigned int)Dee_AST_MULTI_KIND_GENERIC_MAP)
	Dee_AST_MULTI_KIND_GENERIC_MAP, /* Create some abstract object derived from `deemon.Mapping` (even elements are keys, odd ones are values) */
	Dee_AST_MULTI_KIND_DICT,        /* Create a `deemon.Dict` object (even elements are keys, odd ones are values) */
};
struct Dee_ast_multi {
	enum Dee_ast_multi_kind am_kind;  /* Multi-AST kind */
	size_t                  am_count; /* # of elements */
	DeeAst                 *am_elem[Dee_AST_FLEXARRAY]; /* [1..1][owned][am_count] AST elements */
};

/* HINT: Use `DeeAst_PackMulti()` to package. */
#define DeeAst_MallocUninitializedMulti(count)                        _DeeAst_MallocUninitializedVar(ad_multi.am_elem, count)
#define DeeAst_TryMallocUninitializedMulti(count)                     _DeeAst_TryMallocUninitializedVar(ad_multi.am_elem, count)
#define DeeAst_ReallocUninitializedMulti(p, count)                    _DeeAst_ReallocUninitializedVar(ad_multi.am_elem, p, count)
#define DeeAst_TryReallocUninitializedMulti(p, count)                 _DeeAst_TryReallocUninitializedVar(ad_multi.am_elem, p, count)
#define DeeAst_FreeUninitializedMulti(p)                              _DeeAst_FreeUninitializedVar(p)
#define DeeAst_Dbg_MallocUninitializedMulti(count, file, line)        _DeeAst_Dbg_MallocUninitializedVar(ad_multi.am_elem, count, file, line)
#define DeeAst_Dbg_TryMallocUninitializedMulti(count, file, line)     _DeeAst_Dbg_TryMallocUninitializedVar(ad_multi.am_elem, count, file, line)
#define DeeAst_Dbg_ReallocUninitializedMulti(p, count, file, line)    _DeeAst_Dbg_ReallocUninitializedVar(ad_multi.am_elem, p, count, file, line)
#define DeeAst_Dbg_TryReallocUninitializedMulti(p, count, file, line) _DeeAst_Dbg_TryReallocUninitializedVar(ad_multi.am_elem, p, count, file, line)
#define DeeAst_Dbg_FreeUninitializedMulti(p, file, line)              _DeeAst_Dbg_FreeUninitializedVar(p, file, line)
#define DeeAst_GetMultiElem(self)                                     ((self)->a_data.ad_multi.am_elem)
#define DeeAst_ReallocMulti(p, count)                                 DeeAst_ReallocUninitializedMulti(p, count)
#define DeeAst_TryReallocMulti(p, count)                              DeeAst_TryReallocUninitializedMulti(p, count)
#define DeeAst_Dbg_ReallocMulti(p, count, file, line)                 DeeAst_Dbg_ReallocUninitializedMulti(p, count, file, line)
#define DeeAst_Dbg_TryReallocMulti(p, count, file, line)              DeeAst_Dbg_TryReallocUninitializedMulti(p, count, file, line)
/* === */


/* === For `Dee_AST_LOOP` */
typedef uint_fast8_t Dee_ast_loop_flag;
#define Dee_AST_LOOP_FLAG_NORMAL   0x00
#define Dee_AST_LOOP_FLAG_POSTCOND 0x01 /* `al_cond` (when non-`NULL`) is evaluated after `al_body`, rather
                                         *  than before, essentially resulting in `do..while` behavior. */
#define Dee_AST_LOOP_FLAG_FOREACH  0x02 /* This is a foreach loop: `foreach ({alf_elem}: {alf_iter}) {al_body}` */
#define Dee_AST_LOOP_FLAG_UNLIKELY 0x04 /* It is unlikely for `al_body` to be executed (ignored if `Dee_AST_LOOP_FLAG_POSTCOND` is set) */

struct Dee_ast_loop {
	Dee_ast_loop_flag al_flag; /* Loop flags */
	union {
		struct {
			DeeAst *aln_cond; /* [0..1][owned] Loop condition (executed after `aln_next`; unless `Dee_AST_LOOP_FLAG_POSTCOND`, also executed once at start) */
			DeeAst *aln_next; /* [0..1][owned] Loop-next expression (executed after `al_body`; targeted by `continue`) */
		} ald_normal;
		struct {
			DeeAst *alf_elem; /* [0..1][owned] Foreach element expression (usually `Dee_AST_SYM*`) */
			DeeAst *alf_iter; /* [0..1][owned] Foreach iterator expression */
		} ald_foreach;
	} al_data;
	DeeAst               *al_body;      /* [1..1][owned] Loop body */
	struct Dee_asm_symbol al_break;     /* Symbol used for `break` inside of `al_body` */
	struct Dee_asm_symbol al_continue;  /* Symbol used for `continue` inside of `al_body` */
};
#define DeeAst_MallocUninitializedLoop()                  _DeeAst_MallocUninitializedFixed(ad_loop)
#define DeeAst_TryMallocUninitializedLoop()               _DeeAst_TryMallocUninitializedFixed(ad_loop)
#define DeeAst_FreeUninitializedLoop(p)                   _DeeAst_FreeUninitializedFixed(ad_loop, p)
#define DeeAst_Dbg_MallocUninitializedLoop(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_loop, file, line)
#define DeeAst_Dbg_TryMallocUninitializedLoop(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_loop, file, line)
#define DeeAst_Dbg_FreeUninitializedLoop(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_loop, p, file, line)
/* === */


/* === For `Dee_AST_IF` */
typedef uint_fast8_t Dee_ast_if_flag;
#define Dee_AST_IF_FLAG_NORMAL      0x00 /* Normal flags */
#define Dee_AST_IF_FLAG_COND_ASBOOL 0x00 /* Use `!!ai_cond` as condition */
#define Dee_AST_IF_FLAG_COND_ISNONE 0x01 /* Use `ai_cond is none` as condition */
#define Dee_AST_IF_FLAG_COND        0x0f /* Condition mask */
#define Dee_AST_IF_FLAG_LIKELY      0x10 /* The `ai_tt` branch is most more likely to be executed than `ai_ff` */
#define Dee_AST_IF_FLAG_UNLIKELY    0x20 /* The `ai_ff` branch is most more likely to be executed than `ai_tt` */

struct Dee_ast_if {
	/* HINT: This type of AST is used to implement the following:
	 * >> if (a) b; else c; // {Dee_AST_IF_FLAG_COND_ASBOOL, <a>, <b>, <c>}
	 * >> a ? b : c         // {Dee_AST_IF_FLAG_COND_ASBOOL, <a>, <b>, <c>}
	 * >> a ? : b           // {Dee_AST_IF_FLAG_COND_ASBOOL, <a>, <a>, <b>}
	 * >> a ? b :           // {Dee_AST_IF_FLAG_COND_ASBOOL, <a>, <b>, <a>}
	 * >> a && b            // {Dee_AST_IF_FLAG_COND_ASBOOL, <a>, <b>, <a>} (when `TPP_EXT_BOOL_LAND_LOR` is enabled, `a` and `b` are wrapped as `!!a` and `!!b`)
	 * >> a || b            // {Dee_AST_IF_FLAG_COND_ASBOOL, <a>, <a>, <b>} (when `TPP_EXT_BOOL_LAND_LOR` is enabled, `a` and `b` are wrapped as `!!a` and `!!b`)
	 * >> a ?? b            // {Dee_AST_IF_FLAG_COND_ISNONE, <a>, <b>, <a>} */
	Dee_ast_if_flag ai_flag; /* If-ast flags */
	DeeAst         *ai_cond; /* [1..1][owned] If-condition */
	DeeAst         *ai_tt;   /* [0..1][owned_if(!= ai_cond)] True-branch.
	                          * When set to the same pointer as `ai_cond`, re-use the
	                          * value of `ai_cond` as true-value (but only evaluate it once)
	                          * When `NULL`, same as if set to `Dee_AST_CONST:Dee_None` */
	DeeAst         *ai_ff;   /* [0..1][owned_if(!= ai_cond)] False-branch.
	                          * When set to the same pointer as `ai_cond`, re-use the
	                          * value of `ai_cond` as false-value (but only evaluate it once)
	                          * When `NULL`, same as if set to `Dee_AST_CONST:Dee_None` */
};
#define DeeAst_MallocUninitializedIf()                  _DeeAst_MallocUninitializedFixed(ad_if)
#define DeeAst_TryMallocUninitializedIf()               _DeeAst_TryMallocUninitializedFixed(ad_if)
#define DeeAst_FreeUninitializedIf(p)                   _DeeAst_FreeUninitializedFixed(ad_if, p)
#define DeeAst_Dbg_MallocUninitializedIf(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_if, file, line)
#define DeeAst_Dbg_TryMallocUninitializedIf(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_if, file, line)
#define DeeAst_Dbg_FreeUninitializedIf(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_if, p, file, line)
/* === */


/* === For `Dee_AST_ASM_GOTO` */
struct Dee_ast_asm_goto {
	struct Dee_asm_symbol *aag_symbol; /* [1..1] Symbol to jump to */
};
/* === */


/* === For `Dee_AST_SWITCH_STMT` */
struct Dee_ast_label;
LIST_HEAD(Dee_ast_label_list, Dee_ast_label);
struct Dee_ast_switch_stmt {
	DeeAst                   *asws_expr;    /* [1..1][owned] switch-expression */
	DeeAst                   *asws_body;    /* [1..1][owned] switch-body */
	struct Dee_ast_label_list asws_cases;   /* [0..n] Populated by linker: list of case-labels (excluding `default` case) */
	struct Dee_ast_label     *asws_default; /* [0..1] Populated by linker: default-case */
	struct Dee_asm_symbol     asws_break;   /* Symbol used for `break` inside of `asws_body` */
};
#define DeeAst_MallocUninitializedSwitchStmt()                  _DeeAst_MallocUninitializedFixed(ad_switch_stmt)
#define DeeAst_TryMallocUninitializedSwitchStmt()               _DeeAst_TryMallocUninitializedFixed(ad_switch_stmt)
#define DeeAst_FreeUninitializedSwitchStmt(p)                   _DeeAst_FreeUninitializedFixed(ad_switch_stmt, p)
#define DeeAst_Dbg_MallocUninitializedSwitchStmt(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_switch_stmt, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSwitchStmt(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_switch_stmt, file, line)
#define DeeAst_Dbg_FreeUninitializedSwitchStmt(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_switch_stmt, p, file, line)
/* === */


/* === For `Dee_AST_SWITCH_EXPR` */
struct Dee_ast_switch_expr_case {
	/* `case <asec_condv[0]>, <asec_condv[1]>, ... -> <asec_body>;` */
	DeeAst *asec_body;                     /* [1..1][owned] Case-body */
	size_t  asec_condc;                    /* # of conditions */
	DeeAst *asec_condv[Dee_AST_FLEXARRAY]; /* [1..1][owned][asec_condc] Case-conditions */
};

#define Dee_ast_switch_expr_case_sizeof(condc)        (offsetof(struct Dee_ast_switch_expr_case, asec_condv) + ((condc) * sizeof(DeeAst *)))
#define Dee_ast_switch_expr_case_malloc(condc)        ((struct Dee_ast_switch_expr_case *)Dee_Malloc(Dee_ast_switch_expr_case_sizeof(condc)))
#define Dee_ast_switch_expr_case_trymalloc(condc)     ((struct Dee_ast_switch_expr_case *)Dee_TryMalloc(Dee_ast_switch_expr_case_sizeof(condc)))
#define Dee_ast_switch_expr_case_realloc(p, condc)    ((struct Dee_ast_switch_expr_case *)Dee_Realloc(p, Dee_ast_switch_expr_case_sizeof(condc)))
#define Dee_ast_switch_expr_case_tryrealloc(p, condc) ((struct Dee_ast_switch_expr_case *)Dee_TryRealloc(p, Dee_ast_switch_expr_case_sizeof(condc)))
#define Dee_ast_switch_expr_case_free(p)              Dee_Free(p)
#define Dee_ast_switch_expr_case_destroy(self)                    \
	do {                                                          \
		size_t cond_i;                                            \
		DeeAst_Destroy((self)->asec_body);                        \
		for (cond_i = 0; cond_i < (self)->asec_condc; ++cond_i) { \
			DeeAst_Destroy((self)->asec_condv[cond_i]);           \
		}                                                         \
		Dee_ast_switch_expr_case_free(self);                      \
	}	__WHILE0

struct Dee_ast_switch_expr {
	/* `switch (<aswe_expr>) { <aswe_casev...> }` */
	DeeAst                          *aswe_expr;    /* [1..1][owned] switch-expression */
	size_t                           aswe_defcase; /* When `< aswe_casec`, index of `default` case.
	                                                * Else, there is no default case and an unhandled
	                                                * value results in `none` at runtime. */
	size_t                           aswe_casec;   /* # of cases */
	struct Dee_ast_switch_expr_case *aswe_casev[Dee_AST_FLEXARRAY]; /* [1..1][owned][aswe_casec] switch-cases */
};
/* HINT: Use `DeeAst_PackSwitchExpr()` to package. */
#define DeeAst_MallocUninitializedSwitchExpr(casec)                        _DeeAst_MallocUninitializedVar(ad_switch_expr.aswe_casev, casec)
#define DeeAst_TryMallocUninitializedSwitchExpr(casec)                     _DeeAst_TryMallocUninitializedVar(ad_switch_expr.aswe_casev, casec)
#define DeeAst_ReallocUninitializedSwitchExpr(p, casec)                    _DeeAst_ReallocUninitializedVar(ad_switch_expr.aswe_casev, p, casec)
#define DeeAst_TryReallocUninitializedSwitchExpr(p, casec)                 _DeeAst_TryReallocUninitializedVar(ad_switch_expr.aswe_casev, p, casec)
#define DeeAst_FreeUninitializedSwitchExpr(p)                              _DeeAst_FreeUninitializedVar(p)
#define DeeAst_Dbg_MallocUninitializedSwitchExpr(casec, file, line)        _DeeAst_Dbg_MallocUninitializedVar(ad_switch_expr.aswe_casev, casec, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSwitchExpr(casec, file, line)     _DeeAst_Dbg_TryMallocUninitializedVar(ad_switch_expr.aswe_casev, casec, file, line)
#define DeeAst_Dbg_ReallocUninitializedSwitchExpr(p, casec, file, line)    _DeeAst_Dbg_ReallocUninitializedVar(ad_switch_expr.aswe_casev, p, casec, file, line)
#define DeeAst_Dbg_TryReallocUninitializedSwitchExpr(p, casec, file, line) _DeeAst_Dbg_TryReallocUninitializedVar(ad_switch_expr.aswe_casev, p, casec, file, line)
#define DeeAst_Dbg_FreeUninitializedSwitchExpr(p, file, line)              _DeeAst_Dbg_FreeUninitializedVar(p, file, line)
#define DeeAst_GetSwitchExprCases(self)                                    ((self)->a_data.ad_switch_expr.aswe_casev)
/* === */


/* === For `Dee_AST_CATCH` */
typedef uint_fast8_t Dee_ast_catch_handler_flag;
#define Dee_AST_CATCH_HANDLER_FLAG_NORMAL    0x00 /* Normal flags */
#define Dee_AST_CATCH_HANDLER_FLAG_INTERRUPT 0x01 /* Treat as `@[interrupt]` handler (allow `DeeType_IsInterrupt()` to be caught) */

struct Dee_ast_catch_handler {
	Dee_ast_catch_handler_flag ach_flag; /* Catch-handler flags */
	DeeAst                    *ach_mask; /* [0..1][owned] Catch-handler mask (or `NULL` for catch-all) */
	DeeAst                    *ach_into; /* [0..1][owned] Catch-as expression (the current exception is stored in here; usually a `Dee_AST_SYMDEF`, which will be scoped alongside `ach_body`) */
	DeeAst                    *ach_body; /* [1..1][owned] Catch-handler body */
};
#define Dee_ast_catch_handler_fini(self) \
	(DeeAst_XDestroy((self)->ach_mask),  \
	 DeeAst_XDestroy((self)->ach_into),  \
	 DeeAst_Destroy((self)->ach_body))

struct Dee_ast_catch {
	DeeAst                      *ac_body;     /* [1..1][owned] Body of try-statement */
	size_t                       ac_handlerc; /* # of catch-handlers */
	struct Dee_ast_catch_handler ac_handlerv[Dee_AST_FLEXARRAY]; /* [ac_handlerc] Catch handlers */
};
/* HINT: Use `DeeAst_PackCatch()` to package. */
#define DeeAst_MallocUninitializedCatch(handlerc)                        _DeeAst_MallocUninitializedVar(ad_catch.ac_handlerv, handlerc)
#define DeeAst_TryMallocUninitializedCatch(handlerc)                     _DeeAst_TryMallocUninitializedVar(ad_catch.ac_handlerv, handlerc)
#define DeeAst_ReallocUninitializedCatch(p, handlerc)                    _DeeAst_ReallocUninitializedVar(ad_catch.ac_handlerv, p, handlerc)
#define DeeAst_TryReallocUninitializedCatch(p, handlerc)                 _DeeAst_TryReallocUninitializedVar(ad_catch.ac_handlerv, p, handlerc)
#define DeeAst_FreeUninitializedCatch(p)                                 _DeeAst_FreeUninitializedVar(p)
#define DeeAst_Dbg_MallocUninitializedCatch(handlerc, file, line)        _DeeAst_Dbg_MallocUninitializedVar(ad_catch.ac_handlerv, handlerc, file, line)
#define DeeAst_Dbg_TryMallocUninitializedCatch(handlerc, file, line)     _DeeAst_Dbg_TryMallocUninitializedVar(ad_catch.ac_handlerv, handlerc, file, line)
#define DeeAst_Dbg_ReallocUninitializedCatch(p, handlerc, file, line)    _DeeAst_Dbg_ReallocUninitializedVar(ad_catch.ac_handlerv, p, handlerc, file, line)
#define DeeAst_Dbg_TryReallocUninitializedCatch(p, handlerc, file, line) _DeeAst_Dbg_TryReallocUninitializedVar(ad_catch.ac_handlerv, p, handlerc, file, line)
#define DeeAst_Dbg_FreeUninitializedCatch(p, file, line)                 _DeeAst_Dbg_FreeUninitializedVar(p, file, line)
#define DeeAst_GetCatchHandlers(self)                                    ((self)->a_data.ad_catch.ac_handlerv)
/* === */


/* === For `Dee_AST_FINALLY` */
struct Dee_ast_finally {
	DeeAst *af_body;    /* [1..1][owned] Try-part */
	DeeAst *af_finally; /* [1..1][owned] Finally-part */
};
#define DeeAst_MallocUninitializedFinally()                  _DeeAst_MallocUninitializedFixed(ad_finally)
#define DeeAst_TryMallocUninitializedFinally()               _DeeAst_TryMallocUninitializedFixed(ad_finally)
#define DeeAst_FreeUninitializedFinally(p)                   _DeeAst_FreeUninitializedFixed(ad_finally, p)
#define DeeAst_Dbg_MallocUninitializedFinally(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_finally, file, line)
#define DeeAst_Dbg_TryMallocUninitializedFinally(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_finally, file, line)
#define DeeAst_Dbg_FreeUninitializedFinally(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_finally, p, file, line)
/* === */


/* === For `Dee_AST_YIELD` */
struct Dee_ast_yield {
	DeeAst *ay_expr; /* [1..1][owned] Expression to yield */
};
#define DeeAst_MallocUninitializedYield()                  _DeeAst_MallocUninitializedFixed(ad_yield)
#define DeeAst_TryMallocUninitializedYield()               _DeeAst_TryMallocUninitializedFixed(ad_yield)
#define DeeAst_FreeUninitializedYield(p)                   _DeeAst_FreeUninitializedFixed(ad_yield, p)
#define DeeAst_Dbg_MallocUninitializedYield(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_yield, file, line)
#define DeeAst_Dbg_TryMallocUninitializedYield(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_yield, file, line)
#define DeeAst_Dbg_FreeUninitializedYield(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_yield, p, file, line)
/* === */


/* === For `Dee_AST_RETURN` */
struct Dee_ast_return {
	DeeAst *ar_expr; /* [0..1][owned] Expression to return, or `NULL` to return `none` (or exit if inside of a yield-function) */
};
#define DeeAst_MallocUninitializedReturn()                  _DeeAst_MallocUninitializedFixed(ad_return)
#define DeeAst_TryMallocUninitializedReturn()               _DeeAst_TryMallocUninitializedFixed(ad_return)
#define DeeAst_FreeUninitializedReturn(p)                   _DeeAst_FreeUninitializedFixed(ad_return, p)
#define DeeAst_Dbg_MallocUninitializedReturn(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_return, file, line)
#define DeeAst_Dbg_TryMallocUninitializedReturn(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_return, file, line)
#define DeeAst_Dbg_FreeUninitializedReturn(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_return, p, file, line)
/* === */


/* === For `Dee_AST_FUNCTION` */
struct Dee_ast_function_param_base {
	tpp_keyword const           *afpb_name;    /* [0..1] Parameter name, or `NULL` if "none" was used as name */
	DeeAstLoc                    afpb_loc;     /* Parameter (name) location */
	Dee_ast_symref_byname_flag   afpb_flag;    /* Variable flags */
	struct Dee_ast_typedef       afpb_typedef; /* Type annotation for parameter */
	DREF struct Dee_link_symbol *afpb_sym;     /* [0..1] Set by linker: symbol for parameter */
};
#define Dee_ast_function_param_base_alloc()    DeeObject_MALLOC(struct Dee_ast_function_param_base)
#define Dee_ast_function_param_base_free(self) DeeObject_FREE(Dee_REQUIRES_TYPE(struct Dee_ast_function_param_base *, self))
#define Dee_ast_function_param_base_fini(self)    \
	(Dee_ast_typedef_fini(&(self)->afpb_typedef), \
	 Dee_link_symbol_xdecref((self)->afpb_sym))
#define Dee_ast_function_param_base_destroy(self) \
	(Dee_ast_function_param_base_fini(self),      \
	 Dee_ast_function_param_base_free(self))
#define Dee_ast_function_param_base_xdestroy(self) \
	(void)((self) && (Dee_ast_function_param_base_destroy(self), 1))

struct Dee_ast_function_param {
	struct Dee_ast_function_param_base afp_base;    /* Underlying parameter base */
	DeeAst                            *afp_default; /* [0..1][owned] Default-initializer for parameter (when not given by caller)
	                                                 * When `NULL`, the parameter is either mandatory (if its index is `< af_argc_min`),
	                                                 * or the parameter is optional (like in `function foo(param?)`) if its index is
	                                                 * `< af_argc_max`. Behavior is weak-undefined if non-NULL and index `< af_argc_min` */
};
#define Dee_ast_function_param_fini(self)                 \
	(Dee_ast_function_param_base_fini(&(self)->afp_base), \
	 DeeAst_XDestroy((self)->afp_default))

typedef uint_fast8_t Dee_ast_function_flag;
#define Dee_AST_FUNCTION_FLAG_NORMAL   0x00 /* Normal function flags */
#define Dee_AST_FUNCTION_FLAG_ISYIELD  0x01 /* Set by linker: `Dee_AST_YIELD`-ast encountered */
#define Dee_AST_FUNCTION_FLAG_COPYABLE 0x02 /* `@[copyable]`: Set `Dee_CODE_FCOPYABLE` flag on code */
#define Dee_AST_FUNCTION_FLAG_ASSEMBLY 0x04 /* Set by linker: `Dee_AST_IASM`-ast encountered */
#define Dee_AST_FUNCTION_FLAG_THISCALL 0x40 /* `@[thiscall]`: Set `Dee_CODE_FTHISCALL` flag on code */
#define Dee_AST_FUNCTION_FLAG_ISRETURN 0x80 /* Set by linker: `Dee_AST_RETURN`-ast with `Dee_ast_return::ar_expr != NULL` encountered */

struct Dee_ast_function {
	Dee_ast_function_flag               af_flag;           /* Function flags */
	tpp_keyword const                  *af_name;           /* [0..1] Function name (for debug information **only**) */
	DeeAst                             *af_body;           /* [1..1][owned] Function body */
	struct Dee_ast_function_param_base *af_varargs;        /* [0..1][owned] Parameter for `varargs...` parameter (or `NULL` if function doesn't take varargs) */
	struct Dee_ast_function_param_base *af_varkwds;        /* [0..1][owned] Parameter for `**varkwds` parameter (or `NULL` if function doesn't take varkwds) */
	size_t                              af_argc_min;       /* Min # of function parameters (== # of mandatory parameters) */
	size_t                              af_argc_max;       /* Max # of function parameters */
	DREF struct Dee_link_symbol        *af_symbol_this;    /* [0..1] Set/created by linker: symbol for `Dee_AST_SYMREF_KIND_THIS` (left `NULL` if `Dee_AST_FUNCTION_FLAG_THISCALL` isn't set) */
	DREF struct Dee_link_symbol        *af_symbol_tp_this; /* [0..1] Set by linker: symbol for `Dee_AST_SYMREF_KIND_TP_THIS` (left `NULL` if `Dee_AST_FUNCTION_FLAG_THISCALL` isn't set, or not inside of a class scope; s.a. `struct Dee_ast_class::ac_tp_this`) */
	struct Dee_ast_typedef              af_return_typedef; /* Type annotation for return value */
	struct Dee_ast_function_param       af_params[Dee_AST_FLEXARRAY]; /* [af_argc_max] Function parameters */
};

/* HINT: Use `DeeAst_PackFunction()` to package. */
#define DeeAst_MallocUninitializedFunction(argc_max)                        _DeeAst_MallocUninitializedVar(ad_function.af_params, argc_max)
#define DeeAst_TryMallocUninitializedFunction(argc_max)                     _DeeAst_TryMallocUninitializedVar(ad_function.af_params, argc_max)
#define DeeAst_ReallocUninitializedFunction(p, argc_max)                    _DeeAst_ReallocUninitializedVar(ad_function.af_params, p, argc_max)
#define DeeAst_TryReallocUninitializedFunction(p, argc_max)                 _DeeAst_TryReallocUninitializedVar(ad_function.af_params, p, argc_max)
#define DeeAst_FreeUninitializedFunction(p)                                 _DeeAst_FreeUninitializedVar(p)
#define DeeAst_Dbg_MallocUninitializedFunction(argc_max, file, line)        _DeeAst_Dbg_MallocUninitializedVar(ad_function.af_params, argc_max, file, line)
#define DeeAst_Dbg_TryMallocUninitializedFunction(argc_max, file, line)     _DeeAst_Dbg_TryMallocUninitializedVar(ad_function.af_params, argc_max, file, line)
#define DeeAst_Dbg_ReallocUninitializedFunction(p, argc_max, file, line)    _DeeAst_Dbg_ReallocUninitializedVar(ad_function.af_params, p, argc_max, file, line)
#define DeeAst_Dbg_TryReallocUninitializedFunction(p, argc_max, file, line) _DeeAst_Dbg_TryReallocUninitializedVar(ad_function.af_params, p, argc_max, file, line)
#define DeeAst_Dbg_FreeUninitializedFunction(p, file, line)                 _DeeAst_Dbg_FreeUninitializedVar(p, file, line)
#define DeeAst_GetFunctionParams(self)                                      ((self)->a_data.ad_function.af_params)
#define DeeAst_GetFunctionReturnTypedef(self)                               (&(self)->a_data.ad_function.af_return_typedef)
/* === */


/* === For `Dee_AST_CLASS` */
struct Dee_ast_class_member {
	uint16_t cm_addr; /* The member index/operator id of the member (operand for `ASM_DEFCMEMBER`).
	                   * HINT: When set to (uint16_t)-1, rather than initializing a
	                   *       class member, `cm_ast` is evaluated, with its result
	                   *       simply being discarded. */
	DeeAst  *cm_init; /* [1..1][owned] Initializer expression for `cm_index` */
};
#define Dee_ast_class_member_fini(self) DeeAst_Destroy((self)->cm_init)

struct Dee_ast_class {
	DeeAst                      *ac_base;    /* [0..1][owned] Class base/super type */
	DeeAst                      *ac_desc;    /* [1..1][owned] Class descriptor (usually `Dee_AST_CONST` pointing to `DeeClassDescriptor_Type`) */
	DREF struct Dee_link_symbol *ac_tp_this; /* [0..1] Set by linker: symbol for `Dee_AST_SYMREF_KIND_TP_THIS`
	                                          * HINT: This symbol is usually amended with info about `ac_desc`
	                                          *       and `ac_base` in order to allow for optimizations, such
	                                          *       that something like `this as Type.__base__(<ac_tp_this>)`
	                                          *       can be reduced to something simper.
	                                          * NOTE: This field also holds a `Dee_link_symbol_wr_inc`-reference */
	size_t                       ac_memberc; /* # of member initializers (s.a. `ASM_DEFCMEMBER`) */
	struct Dee_ast_class_member  ac_memberv[Dee_AST_FLEXARRAY]; /* [ac_memberc] Member initializers */
};

/* HINT: Use `DeeAst_PackClass()` to package. */
#define DeeAst_MallocUninitializedClass(memberc)                        _DeeAst_MallocUninitializedVar(ad_class.ac_memberv, memberc)
#define DeeAst_TryMallocUninitializedClass(memberc)                     _DeeAst_TryMallocUninitializedVar(ad_class.ac_memberv, memberc)
#define DeeAst_ReallocUninitializedClass(p, memberc)                    _DeeAst_ReallocUninitializedVar(ad_class.ac_memberv, p, memberc)
#define DeeAst_TryReallocUninitializedClass(p, memberc)                 _DeeAst_TryReallocUninitializedVar(ad_class.ac_memberv, p, memberc)
#define DeeAst_FreeUninitializedClass(p)                                _DeeAst_FreeUninitializedVar(p)
#define DeeAst_Dbg_MallocUninitializedClass(memberc, file, line)        _DeeAst_Dbg_MallocUninitializedVar(ad_class.ac_memberv, memberc, file, line)
#define DeeAst_Dbg_TryMallocUninitializedClass(memberc, file, line)     _DeeAst_Dbg_TryMallocUninitializedVar(ad_class.ac_memberv, memberc, file, line)
#define DeeAst_Dbg_ReallocUninitializedClass(p, memberc, file, line)    _DeeAst_Dbg_ReallocUninitializedVar(ad_class.ac_memberv, p, memberc, file, line)
#define DeeAst_Dbg_TryReallocUninitializedClass(p, memberc, file, line) _DeeAst_Dbg_TryReallocUninitializedVar(ad_class.ac_memberv, p, memberc, file, line)
#define DeeAst_Dbg_FreeUninitializedClass(p, file, line)                _DeeAst_Dbg_FreeUninitializedVar(p, file, line)
#define DeeAst_GetClassMembers(self)                                    ((self)->a_data.ad_class.ac_memberv)
/* === */


/* === For `Dee_AST_UNPACK_WR` */
struct Dee_ast_unpack_wr {
	DeeAst      *aupwr_expr;    /* Expression that should be unpacked */
	size_t       aupwr_targetc; /* # of unpack targets */
	DeeAstRValue aupwr_targetv[Dee_AST_FLEXARRAY]; /* [MODE(Dee_AST_RVALUE_MODE_WR)][aupwr_targetc] Unpack targets */
};
/* HINT: Use `DeeAst_PackUnpackWr()` to package. */
#define DeeAst_MallocUninitializedUnpackWr(targetc)                        _DeeAst_MallocUninitializedVar(ad_unpack_wr.aupwr_targetv, targetc)
#define DeeAst_TryMallocUninitializedUnpackWr(targetc)                     _DeeAst_TryMallocUninitializedVar(ad_unpack_wr.aupwr_targetv, targetc)
#define DeeAst_ReallocUninitializedUnpackWr(p, targetc)                    _DeeAst_ReallocUninitializedVar(ad_unpack_wr.aupwr_targetv, p, targetc)
#define DeeAst_TryReallocUninitializedUnpackWr(p, targetc)                 _DeeAst_TryReallocUninitializedVar(ad_unpack_wr.aupwr_targetv, p, targetc)
#define DeeAst_FreeUninitializedUnpackWr(p)                                _DeeAst_FreeUninitializedVar(p)
#define DeeAst_Dbg_MallocUninitializedUnpackWr(targetc, file, line)        _DeeAst_Dbg_MallocUninitializedVar(ad_unpack_wr.aupwr_targetv, targetc, file, line)
#define DeeAst_Dbg_TryMallocUninitializedUnpackWr(targetc, file, line)     _DeeAst_Dbg_TryMallocUninitializedVar(ad_unpack_wr.aupwr_targetv, targetc, file, line)
#define DeeAst_Dbg_ReallocUninitializedUnpackWr(p, targetc, file, line)    _DeeAst_Dbg_ReallocUninitializedVar(ad_unpack_wr.aupwr_targetv, p, targetc, file, line)
#define DeeAst_Dbg_TryReallocUninitializedUnpackWr(p, targetc, file, line) _DeeAst_Dbg_TryReallocUninitializedVar(ad_unpack_wr.aupwr_targetv, p, targetc, file, line)
#define DeeAst_Dbg_FreeUninitializedUnpackWr(p, file, line)                _DeeAst_Dbg_FreeUninitializedVar(p, file, line)
#define DeeAst_GetUnpackWrTargets(self)                                    ((self)->a_data.ad_unpack_wr.aupwr_targetv)
/* === */


/* === For `Dee_AST_IASM` */
enum Dee_ast_iasm_operand_kind {
	Dee_AST_IASM_OPERAND_KIND_AST,    /* AST */
	Dee_AST_IASM_OPERAND_KIND_RVALUE, /* R-Value */
	Dee_AST_IASM_OPERAND_KIND_LABEL,  /* Unlinked label name */
	Dee_AST_IASM_OPERAND_KIND_SYMBOL, /* Linked assembly symbol */
};
struct Dee_ast_iasm_operand {
	tpp_keyword const        *aio_name;       /* [0..1] User-defined name for this operand. */
	tpp_char const           *aio_type;       /* [0..1] Start pointer of operator type descriptor (or `NULL` for label-like operands) */
	tpp_size                  aio_type_size;  /* Length of `aio_type` (in bytes) */
	TPP_REF tpp_string       *aio_type_chunk; /* [0..1] Operand type descriptor string buffer */
	Dee_ast_iasm_operand_kind aio_kind;       /* Operand kind */
	union {
		DeeAst                *aiod_ast;    /* [1..1][owned][valid_if(aio_kind == Dee_AST_IASM_OPERAND_KIND_AST)] */
		DeeAstRValue           aiod_rvalue; /* [MODE(Dee_ast_iasm_operand_getrvaluemode(self))][valid_if(aio_kind == Dee_AST_IASM_OPERAND_KIND_RVALUE)] */
		tpp_keyword const     *aiod_label;  /* [1..1][valid_if(aio_kind == Dee_AST_IASM_OPERAND_KIND_LABEL)] */
		struct Dee_asm_symbol *aiod_symbol; /* [1..1][valid_if(aio_kind == Dee_AST_IASM_OPERAND_KIND_SYMBOL)] */
	} aio_data;
};
#define Dee_ast_iasm_operand_isinout(x) ((x)->aio_type_size >= 1 && (x)->aio_type[0] == '+')
#define Dee_ast_iasm_operand_getrvaluemode(x) \
	(Dee_ast_iasm_operand_isinout(x) ? Dee_AST_RVALUE_MODE_RDWR : Dee_AST_RVALUE_MODE_WR)
#define Dee_ast_iasm_operand_fini(self)                                  \
	do {                                                                 \
		if ((self)->aio_type_chunk)                                      \
			tpp_string_decref((self)->aio_type_chunk);                   \
		switch ((self)->aio_kind) {                                      \
		case Dee_AST_IASM_OPERAND_KIND_AST:                              \
			DeeAst_Destroy((self)->aio_data.aiod_ast);                   \
			break;                                                       \
		case Dee_AST_IASM_OPERAND_KIND_RVALUE:                           \
			DeeAstRValue_Fini(&(self)->aio_data.aiod_rvalue,             \
			                  Dee_ast_iasm_operand_getrvaluemode(self)); \
			break;                                                       \
		default:                                                         \
			break;                                                       \
		}                                                                \
	}	__WHILE0

typedef uint_fast8_t Dee_ast_iasm_flag;
#define Dee_AST_IASM_FLAG_NORMAL     0x00
#define Dee_AST_IASM_FLAG_FORMAT     0x01 /* Format the assembly text before parsing it. */
#define Dee_AST_IASM_FLAG_VOLATILE   0x02 /* Assembly is `volatile` (don't perform peephole optimizations on it) */
#define Dee_AST_IASM_FLAG_C_MEMORY   0x04 /* `memory` clobber was used: never reorder with other nearby branches (acts as a read/write barrier) */
#define Dee_AST_IASM_FLAG_C_STACK    0x08 /* `sp` clobber was used: don't warn if user-assembly leaves the stack in an unaligned state. */
#define Dee_AST_IASM_FLAG_C_REACH    0x10 /* `reach` clobber was used: never delete ast, even if it appears unreachable */
#define Dee_AST_IASM_FLAG_C_NORETURN 0x20 /* `noreturn` clobber was used: user-assembly does not return normally */
#define Dee_AST_IASM_FLAG_C_ALL                               \
	(Dee_AST_IASM_FLAG_C_MEMORY | Dee_AST_IASM_FLAG_C_STACK | \
	 Dee_AST_IASM_FLAG_C_REACH | Dee_AST_IASM_FLAG_C_NORETURN)

struct Dee_ast_iasm {
	Dee_ast_iasm_flag           aia_flag;       /* User-assembly flags */
	tpp_char const             *aia_text;       /* [1..1] Start pointer of user-assembly */
	tpp_size                    aia_text_size;  /* Length of `aia_text` (in bytes) */
	TPP_REF tpp_string         *aia_text_chunk; /* [0..1] User-assembly text buffer (usually pointed-into by `aia_text`) */
	tpp_lcinfo                  aia_text_lc;    /* [valid_if(aia_text != NULL)] Line/column information at start of `aia_text` */
	size_t                      aia_opc_o;      /* Amount of output operands. */
	size_t                      aia_opc_i;      /* Amount of input operands. */
	size_t                      aia_opc_l;      /* Amount of label operands. */
	size_t                      aia_opc;        /* [== as_num_o + as_num_i + as_num_l] Total number of operands. */
	struct Dee_ast_iasm_operand aia_opv[Dee_AST_FLEXARRAY]; /* [aia_opc] Assembly operands */
};

/* HINT: Use `DeeAst_PackIAsm()` to package. */
#define DeeAst_MallocUninitializedIAsm(opc)                        _DeeAst_MallocUninitializedVar(ad_iasm.aia_opv, opc)
#define DeeAst_TryMallocUninitializedIAsm(opc)                     _DeeAst_TryMallocUninitializedVar(ad_iasm.aia_opv, opc)
#define DeeAst_ReallocUninitializedIAsm(p, opc)                    _DeeAst_ReallocUninitializedVar(ad_iasm.aia_opv, p, opc)
#define DeeAst_TryReallocUninitializedIAsm(p, opc)                 _DeeAst_TryReallocUninitializedVar(ad_iasm.aia_opv, p, opc)
#define DeeAst_FreeUninitializedIAsm(p)                            _DeeAst_FreeUninitializedVar(p)
#define DeeAst_Dbg_MallocUninitializedIAsm(opc, file, line)        _DeeAst_Dbg_MallocUninitializedVar(ad_iasm.aia_opv, opc, file, line)
#define DeeAst_Dbg_TryMallocUninitializedIAsm(opc, file, line)     _DeeAst_Dbg_TryMallocUninitializedVar(ad_iasm.aia_opv, opc, file, line)
#define DeeAst_Dbg_ReallocUninitializedIAsm(p, opc, file, line)    _DeeAst_Dbg_ReallocUninitializedVar(ad_iasm.aia_opv, p, opc, file, line)
#define DeeAst_Dbg_TryReallocUninitializedIAsm(p, opc, file, line) _DeeAst_Dbg_TryReallocUninitializedVar(ad_iasm.aia_opv, p, opc, file, line)
#define DeeAst_Dbg_FreeUninitializedIAsm(p, file, line)            _DeeAst_Dbg_FreeUninitializedVar(p, file, line)
#define DeeAst_GetIAsmOperands(self)                               ((self)->a_data.ad_iasm.aia_opv)
/* === */


/* === For `Dee_AST_OPERATOR` */
#define AST_OPERATOR_FNORMAL    0x0000   /* Normal operator flags. */
#define AST_OPERATOR_FPOSTOP    0x0001   /* FLAG: Before the operation, create a copy of the self-operator
                                          *       and discard the return value of the operator, instead opting
                                          *       to exit with the self-copy ontop of the stack.
                                          * NOTE: No copy is created if the value of the expression itself isn't used.
                                          * HINT: This flag is used to implement `a++` as `({ __stack local temp = copy a; ++a; temp; })`  */
#define AST_OPERATOR_FVARARGS   0x0002   /* FLAG: The operator always has 2 arguments, the first of which
                                          *       is the self-operand and the second a tuple of arguments
                                          *       to pass to the operator callback. */
#define AST_OPERATOR_FMAYBEPFX  0x0004   /* FLAG: If the operator is an inplace operation, always generate code and don't
                                          *       cause a compiler error if the operation will cause an error at runtime.
                                          *       This flag is usually set for explicit operator invocations. */
#define AST_OPERATOR_FDONTOPT   0x8000   /* Don't perform constant propagation optimization on this branch.
                                          * Usually just set by the optimizer itself, so-as not to re-attempt
                                          * constant propagation after doing so failed before. */
typedef uint_fast8_t Dee_ast_operator_flag;
#define Dee_AST_OPERATOR_FLAG_NORMAL 0x00 /* Normal flags */
#define Dee_AST_OPERATOR_FLAG_POSTOP 0x01 /* Compile as `({ local _res = copy <ao_argv[0]>; <ao_argv[0]>.operator <ao_id>(...); _res; })` */
#define Dee_AST_OPERATOR_FLAG_VARARG 0x02 /* There must be exactly 2 arguments (else: undefined behavior), and the
                                           * operator is invoked as `<ao_argv[0]>.operator <ao_id>(<ao_argv[1]>...)` */

#define Dee_AST_OPERATOR_ARGC_MAX 4
struct Dee_ast_operator {
	struct Dee_ast_operator_id ao_id;   /* Operator ID */
	Dee_ast_operator_flag      ao_flag; /* Operator flags */
	uint_fast8_t               ao_argc; /* [>= 1 && <= Dee_AST_OPERATOR_ARGC_MAX] Argument count */
	DeeAst                    *ao_argv[Dee_AST_OPERATOR_ARGC_MAX]; /* [1..1][owned][ao_argc] Operator arguments (unused arguments are initialized to `NULL`) */
};
#define DeeAst_MallocUninitializedOperator()                  _DeeAst_MallocUninitializedFixed(ad_operator)
#define DeeAst_TryMallocUninitializedOperator()               _DeeAst_TryMallocUninitializedFixed(ad_operator)
#define DeeAst_FreeUninitializedOperator(p)                   _DeeAst_FreeUninitializedFixed(ad_operator, p)
#define DeeAst_Dbg_MallocUninitializedOperator(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_operator, file, line)
#define DeeAst_Dbg_TryMallocUninitializedOperator(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_operator, file, line)
#define DeeAst_Dbg_FreeUninitializedOperator(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_operator, p, file, line)
/* === */


/* === For `Dee_AST_IOPERATOR` */
struct Dee_ast_ioperator {
	struct Dee_ast_operator_id aio_id;   /* Operator ID */
	Dee_ast_operator_flag      aio_flag; /* Operator flags */
	uint_fast8_t               aio_argc; /* [>= 1 && <= Dee_AST_OPERATOR_ARGC_MAX] Argument count (including `aio_self`) */
	DeeAstRValue               aio_self; /* [MODE(Dee_AST_RVALUE_MODE_RDWR)] Self-argument */
	DeeAst                    *aio_xargv[Dee_AST_OPERATOR_ARGC_MAX - 1]; /* [1..1][owned][ao_argc-1] Extra operator arguments (unused arguments are initialized to `NULL`) */
};
#define DeeAst_MallocUninitializedIOperator()                  _DeeAst_MallocUninitializedFixed(ad_ioperator)
#define DeeAst_TryMallocUninitializedIOperator()               _DeeAst_TryMallocUninitializedFixed(ad_ioperator)
#define DeeAst_FreeUninitializedIOperator(p)                   _DeeAst_FreeUninitializedFixed(ad_ioperator, p)
#define DeeAst_Dbg_MallocUninitializedIOperator(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_ioperator, file, line)
#define DeeAst_Dbg_TryMallocUninitializedIOperator(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_ioperator, file, line)
#define DeeAst_Dbg_FreeUninitializedIOperator(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_ioperator, p, file, line)
/* === */


/* === For `Dee_AST_ACT1_*` */
struct Dee_ast_act1 {
	DeeAst *aa_act0; /* [1..1][owned] Operand #0 for action */
};
#define DeeAst_MallocUninitializedAct1()                  _DeeAst_MallocUninitializedFixed(ad_act1)
#define DeeAst_TryMallocUninitializedAct1()               _DeeAst_TryMallocUninitializedFixed(ad_act1)
#define DeeAst_FreeUninitializedAct1(p)                   _DeeAst_FreeUninitializedFixed(ad_act1, p)
#define DeeAst_Dbg_MallocUninitializedAct1(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_act1, file, line)
#define DeeAst_Dbg_TryMallocUninitializedAct1(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_act1, file, line)
#define DeeAst_Dbg_FreeUninitializedAct1(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_act1, p, file, line)
/* === */


/* === For `Dee_AST_ACT2_*` */
struct Dee_ast_act2 {
	DeeAst *aa_act0; /* [1..1][owned] Operand #0 for action */
	DeeAst *aa_act1; /* [1..1][owned] Operand #1 for action */
};
#define DeeAst_MallocUninitializedAct2()                  _DeeAst_MallocUninitializedFixed(ad_act2)
#define DeeAst_TryMallocUninitializedAct2()               _DeeAst_TryMallocUninitializedFixed(ad_act2)
#define DeeAst_FreeUninitializedAct2(p)                   _DeeAst_FreeUninitializedFixed(ad_act2, p)
#define DeeAst_Dbg_MallocUninitializedAct2(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_act2, file, line)
#define DeeAst_Dbg_TryMallocUninitializedAct2(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_act2, file, line)
#define DeeAst_Dbg_FreeUninitializedAct2(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_act2, p, file, line)
/* === */


/* === For `Dee_AST_ACT3_*` */
struct Dee_ast_act3 {
	DeeAst *aa_act0; /* [1..1][owned] Operand #0 for action */
	DeeAst *aa_act1; /* [1..1][owned] Operand #1 for action */
	DeeAst *aa_act2; /* [1..1][owned] Operand #2 for action */
};
#define DeeAst_MallocUninitializedAct3()                  _DeeAst_MallocUninitializedFixed(ad_act3)
#define DeeAst_TryMallocUninitializedAct3()               _DeeAst_TryMallocUninitializedFixed(ad_act3)
#define DeeAst_FreeUninitializedAct3(p)                   _DeeAst_FreeUninitializedFixed(ad_act3, p)
#define DeeAst_Dbg_MallocUninitializedAct3(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_act3, file, line)
#define DeeAst_Dbg_TryMallocUninitializedAct3(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_act3, file, line)
#define DeeAst_Dbg_FreeUninitializedAct3(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_act3, p, file, line)
/* === */


/* === For `Dee_AST_SYMBOL_RD` */
struct Dee_ast_symbol_rd {
	DREF struct Dee_link_symbol *asrd_symbol; /* [REFERENCE(Dee_link_symbol_rd_inc)][1..1] Symbol to read from */
};
#define DeeAst_MallocUninitializedSymbolRd()                  _DeeAst_MallocUninitializedFixed(ad_symbol_rd)
#define DeeAst_TryMallocUninitializedSymbolRd()               _DeeAst_TryMallocUninitializedFixed(ad_symbol_rd)
#define DeeAst_FreeUninitializedSymbolRd(p)                   _DeeAst_FreeUninitializedFixed(ad_symbol_rd, p)
#define DeeAst_Dbg_MallocUninitializedSymbolRd(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symbol_rd, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymbolRd(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symbol_rd, file, line)
#define DeeAst_Dbg_FreeUninitializedSymbolRd(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symbol_rd, p, file, line)
/* === */


/* === For `Dee_AST_SYMBOL_BND` */
struct Dee_ast_symbol_bnd {
	DREF struct Dee_link_symbol *asbn_symbol; /* [REFERENCE(Dee_link_symbol_rd_inc)][1..1] Symbol to check binding of */
};
#define DeeAst_MallocUninitializedSymbolBnd()                  _DeeAst_MallocUninitializedFixed(ad_symbol_bnd)
#define DeeAst_TryMallocUninitializedSymbolBnd()               _DeeAst_TryMallocUninitializedFixed(ad_symbol_bnd)
#define DeeAst_FreeUninitializedSymbolBnd(p)                   _DeeAst_FreeUninitializedFixed(ad_symbol_bnd, p)
#define DeeAst_Dbg_MallocUninitializedSymbolBnd(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symbol_bnd, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymbolBnd(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symbol_bnd, file, line)
#define DeeAst_Dbg_FreeUninitializedSymbolBnd(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symbol_bnd, p, file, line)
/* === */


/* === For `Dee_AST_SYMBOL_DEL` */
struct Dee_ast_symbol_del {
	DREF struct Dee_link_symbol *asub_symbol; /* [REFERENCE(Dee_link_symbol_wr_inc)][1..1] Symbol to unbind */
};
#define DeeAst_MallocUninitializedSymbolDel()                  _DeeAst_MallocUninitializedFixed(ad_symbol_del)
#define DeeAst_TryMallocUninitializedSymbolDel()               _DeeAst_TryMallocUninitializedFixed(ad_symbol_del)
#define DeeAst_FreeUninitializedSymbolDel(p)                   _DeeAst_FreeUninitializedFixed(ad_symbol_del, p)
#define DeeAst_Dbg_MallocUninitializedSymbolDel(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symbol_del, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymbolDel(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symbol_del, file, line)
#define DeeAst_Dbg_FreeUninitializedSymbolDel(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symbol_del, p, file, line)
/* === */


/* === For `Dee_AST_SYMBOL_WR` */
struct Dee_ast_symbol_wr {
	DREF struct Dee_link_symbol *aswr_symbol; /* [REFERENCE(Dee_link_symbol_wr_inc)][1..1] Symbol to write to */
	DeeAst                      *aswr_value;  /* [1..1][owned] Value to write (also result value of AST) */
};
#define DeeAst_MallocUninitializedSymbolWr()                  _DeeAst_MallocUninitializedFixed(ad_symbol_wr)
#define DeeAst_TryMallocUninitializedSymbolWr()               _DeeAst_TryMallocUninitializedFixed(ad_symbol_wr)
#define DeeAst_FreeUninitializedSymbolWr(p)                   _DeeAst_FreeUninitializedFixed(ad_symbol_wr, p)
#define DeeAst_Dbg_MallocUninitializedSymbolWr(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symbol_wr, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymbolWr(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symbol_wr, file, line)
#define DeeAst_Dbg_FreeUninitializedSymbolWr(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symbol_wr, p, file, line)
/* === */


/* === For `Dee_AST_SYMBOL_XCH` */
struct Dee_ast_symbol_xch {
	DREF struct Dee_link_symbol *asxc_symbol; /* [REFERENCE(Dee_link_symbol_rdwr_inc)][1..1] Symbol whose value to exchange */
	DeeAst                      *asxc_value;  /* [1..1][owned] Value to write */
};
#define DeeAst_MallocUninitializedSymbolXch()                  _DeeAst_MallocUninitializedFixed(ad_symbol_xch)
#define DeeAst_TryMallocUninitializedSymbolXch()               _DeeAst_TryMallocUninitializedFixed(ad_symbol_xch)
#define DeeAst_FreeUninitializedSymbolXch(p)                   _DeeAst_FreeUninitializedFixed(ad_symbol_xch, p)
#define DeeAst_Dbg_MallocUninitializedSymbolXch(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symbol_xch, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymbolXch(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symbol_xch, file, line)
#define DeeAst_Dbg_FreeUninitializedSymbolXch(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symbol_xch, p, file, line)
/* === */


/* === For `Dee_AST_SYMBOL_INIT` */
struct Dee_ast_symbol_init {
	DREF struct Dee_link_symbol *asin_symbol; /* [REFERENCE(Dee_link_symbol_rdwr_inc)][1..1] Symbol to initialize */
	DeeAst                      *asin_value;  /* [1..1][owned] Initializer value */
};
#define DeeAst_MallocUninitializedSymbolInit()                  _DeeAst_MallocUninitializedFixed(ad_symbol_init)
#define DeeAst_TryMallocUninitializedSymbolInit()               _DeeAst_TryMallocUninitializedFixed(ad_symbol_init)
#define DeeAst_FreeUninitializedSymbolInit(p)                   _DeeAst_FreeUninitializedFixed(ad_symbol_init, p)
#define DeeAst_Dbg_MallocUninitializedSymbolInit(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symbol_init, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymbolInit(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symbol_init, file, line)
#define DeeAst_Dbg_FreeUninitializedSymbolInit(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symbol_init, p, file, line)
/* === */


/* === For `Dee_AST_SYMREF` */
#define DeeAst_MallocUninitializedSymref()                  _DeeAst_MallocUninitializedFixed(ad_symref)
#define DeeAst_TryMallocUninitializedSymref()               _DeeAst_TryMallocUninitializedFixed(ad_symref)
#define DeeAst_FreeUninitializedSymref(p)                   _DeeAst_FreeUninitializedFixed(ad_symref, p)
#define DeeAst_Dbg_MallocUninitializedSymref(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symref, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymref(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symref, file, line)
#define DeeAst_Dbg_FreeUninitializedSymref(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symref, p, file, line)
/* === */


/* === For `Dee_AST_SYMREF_ALIAS` */
struct Dee_ast_symref_alias {
	struct Dee_ast_symref asra_symref; /* Underlying symbol reference */
	tpp_keyword const    *asra_alias;  /* New name under which `asra_symref` should become available locally */
};
#define DeeAst_MallocUninitializedSymrefAlias()                  _DeeAst_MallocUninitializedFixed(ad_symref_alias)
#define DeeAst_TryMallocUninitializedSymrefAlias()               _DeeAst_TryMallocUninitializedFixed(ad_symref_alias)
#define DeeAst_FreeUninitializedSymrefAlias(p)                   _DeeAst_FreeUninitializedFixed(ad_symref_alias, p)
#define DeeAst_Dbg_MallocUninitializedSymrefAlias(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_symref_alias, file, line)
#define DeeAst_Dbg_TryMallocUninitializedSymrefAlias(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_symref_alias, file, line)
#define DeeAst_Dbg_FreeUninitializedSymrefAlias(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_symref_alias, p, file, line)
/* === */


/* === For `Dee_AST_IMPORT_STAR` */
struct Dee_ast_import_star {
	DREF struct Dee_module_object *asra_mod; /* [1..1] Module whose symbols should be imported */
};
#define DeeAst_MallocUninitializedImportStar()                  _DeeAst_MallocUninitializedFixed(ad_import_star)
#define DeeAst_TryMallocUninitializedImportStar()               _DeeAst_TryMallocUninitializedFixed(ad_import_star)
#define DeeAst_FreeUninitializedImportStar(p)                   _DeeAst_FreeUninitializedFixed(ad_import_star, p)
#define DeeAst_Dbg_MallocUninitializedImportStar(file, line)    _DeeAst_Dbg_MallocUninitializedFixed(ad_import_star, file, line)
#define DeeAst_Dbg_TryMallocUninitializedImportStar(file, line) _DeeAst_Dbg_TryMallocUninitializedFixed(ad_import_star, file, line)
#define DeeAst_Dbg_FreeUninitializedImportStar(p, file, line)   _DeeAst_Dbg_FreeUninitializedFixed(ad_import_star, p, file, line)
/* === */


/* === For `Dee_AST_GOTO` */
struct Dee_ast_goto {
	tpp_keyword const *ach_symname; /* [1..1] Name of the label to jump to */
};
/* === */


/* === For `Dee_AST_BREAK` */
struct Dee_ast_break {
	tpp_keyword const *ab_symname; /* [0..1] Label-name of loop to break out of */
};
/* === */


/* === For `Dee_AST_CONTINUE` */
struct Dee_ast_continue {
	tpp_keyword const *ac_symname; /* [0..1] Label-name of loop to continue */
};
/* === */



struct Dee_ast {
	enum Dee_ast_kind      a_kind;   /* AST kind */
	struct Dee_ast_labels *a_labels; /* [0..n][owned] AST labels */
	DeeAstLoc              a_loc;    /* Location info */
#define DeeAst_SetLoc(self, aloc)      (void)((self)->a_loc = *(aloc))
#define DeeAst_SetLocHere(self, lexer) DeeLexer_GetLoc(lexer, &(self)->a_loc)

#define _DeeAst_InitCommon(self, kind) \
	(void)((self)->a_kind   = (kind),  \
	       (self)->a_labels = NULL,    \
	       ast_loc_init_empty(&(self)->a_loc))
#define _DeeAst_FiniCommon(self)                                   \
	do {                                                           \
		struct Dee_ast_labels *const _a_labels = (self)->a_labels; \
		if (_a_labels)                                             \
			Dee_ast_labels_destroy(_a_labels);                     \
	}	__WHILE0
#define _DeeAst_MoveCommon(self, kind, from) \
	(void)((self)->a_kind = (kind),          \
	       _DeeAst_MoveCommonNoKind(self, from))
#define _DeeAst_MoveCommonNoKind(self, from)    \
	(void)((self)->a_labels = (from)->a_labels, \
	       (self)->a_loc    = (from)->a_loc,    \
	       (from)->a_labels = NULL)
	union {
		struct Dee_ast_const       ad_const;       /* [valid_if(a_kind == Dee_AST_CONST)] */
		struct Dee_ast_multi       ad_multi;       /* [valid_if(a_kind == Dee_AST_MULTI)] */
		struct Dee_ast_loop        ad_loop;        /* [valid_if(a_kind == Dee_AST_LOOP)] */
		struct Dee_ast_if          ad_if;          /* [valid_if(a_kind == Dee_AST_IF)] */
		struct Dee_ast_asm_goto    ad_asm_goto;    /* [valid_if(a_kind == Dee_AST_ASM_GOTO)] */
		struct Dee_ast_switch_stmt ad_switch_stmt; /* [valid_if(a_kind == Dee_AST_SWITCH_STMT)] */
		struct Dee_ast_switch_expr ad_switch_expr; /* [valid_if(a_kind == Dee_AST_SWITCH_EXPR)] */
		struct Dee_ast_catch       ad_catch;       /* [valid_if(a_kind == Dee_AST_CATCH)] */
		struct Dee_ast_finally     ad_finally;     /* [valid_if(a_kind == Dee_AST_FINALLY)] */
		struct Dee_ast_yield       ad_yield;       /* [valid_if(a_kind == Dee_AST_YIELD)] */
		struct Dee_ast_return      ad_return;      /* [valid_if(a_kind == Dee_AST_RETURN)] */
		struct Dee_ast_function    ad_function;    /* [valid_if(a_kind == Dee_AST_FUNCTION)] */
		struct Dee_ast_class       ad_class;       /* [valid_if(a_kind == Dee_AST_CLASS)] */
		struct Dee_ast_unpack_wr   ad_unpack_wr;   /* [valid_if(a_kind == Dee_AST_UNPACK_WR)] */
		struct Dee_ast_iasm        ad_iasm;        /* [valid_if(a_kind == Dee_AST_IASM)] */
		struct Dee_ast_operator    ad_operator;    /* [valid_if(a_kind == Dee_AST_OPERATOR)] */
		struct Dee_ast_ioperator   ad_ioperator;   /* [valid_if(a_kind == Dee_AST_IOPERATOR)] */
		struct Dee_ast_act1        ad_act1;        /* [valid_if(Dee_AST_ISACT1(a_kind))] */
		struct Dee_ast_act2        ad_act2;        /* [valid_if(Dee_AST_ISACT2(a_kind))] */
		struct Dee_ast_act3        ad_act3;        /* [valid_if(Dee_AST_ISACT3(a_kind))] */
		struct Dee_ast_symbol_rd   ad_symbol_rd;   /* [valid_if(a_kind == Dee_AST_SYMBOL_RD)] */
		struct Dee_ast_symbol_bnd  ad_symbol_bnd;  /* [valid_if(a_kind == Dee_AST_SYMBOL_BND)] */
		struct Dee_ast_symbol_wr   ad_symbol_wr;   /* [valid_if(a_kind == Dee_AST_SYMBOL_WR)] */
		struct Dee_ast_symbol_del  ad_symbol_del;  /* [valid_if(a_kind == Dee_AST_SYMBOL_DEL)] */
		struct Dee_ast_symbol_xch  ad_symbol_xch;  /* [valid_if(a_kind == Dee_AST_SYMBOL_XCH)] */
		struct Dee_ast_symbol_init ad_symbol_init; /* [valid_if(a_kind == Dee_AST_SYMBOL_INIT)] */

		/* The following AST kinds only exist before linking */
		struct Dee_ast_symref       ad_symref;       /* [valid_if(a_kind == Dee_AST_SYMREF)] */
		struct Dee_ast_symref_alias ad_symref_alias; /* [valid_if(a_kind == Dee_AST_SYMREF_ALIAS)] */
		struct Dee_ast_import_star  ad_import_star;  /* [valid_if(a_kind == Dee_AST_IMPORT_STAR)] */
		struct Dee_ast_goto         ad_goto;         /* [valid_if(a_kind == Dee_AST_GOTO)] */
		struct Dee_ast_break        ad_break;        /* [valid_if(a_kind == Dee_AST_BREAK)] */
		struct Dee_ast_continue     ad_continue;     /* [valid_if(a_kind == Dee_AST_CONTINUE)] */
	} a_data; /* Kind-specific AST data */
};

LOCAL NONNULL((2)) DeeAst *DCALL
DeeAst_TrySetLoc(/*0..1*/DeeAst *self, DeeAstLoc const *__restrict aloc) {
	if likely(self)
		DeeAst_SetLoc(self, aloc);
	return self;
}

/* Allocator for: `Dee_AST_ASM_GOTO`, `Dee_AST_GOTO`, `Dee_AST_BREAK`, `Dee_AST_CONTINUE` */
#define _DeeAst_SIZEOF_GOTO_LIKE_IMPL                                 \
	__hybrid_max_c4(COMPILER_OFFSETAFTER(DeeAst, a_data.ad_asm_goto), \
	                COMPILER_OFFSETAFTER(DeeAst, a_data.ad_goto),     \
	                COMPILER_OFFSETAFTER(DeeAst, a_data.ad_break),    \
	                COMPILER_OFFSETAFTER(DeeAst, a_data.ad_continue))
enum { _DeeAst_SIZEOF_GOTO_LIKE = _DeeAst_SIZEOF_GOTO_LIKE_IMPL };
#undef _DeeAst_SIZEOF_GOTO_LIKE_IMPL
#define _DeeAst_MallocUninitializedGotoLike()                  ((DeeAst *)DeeObject_FMalloc(_DeeAst_SIZEOF_GOTO_LIKE))
#define _DeeAst_TryMallocUninitializedGotoLike()               ((DeeAst *)DeeObject_FTryMalloc(_DeeAst_SIZEOF_GOTO_LIKE))
#define _DeeAst_FreeUninitializedGotoLike(p)                   DeeObject_FFree(p, _DeeAst_SIZEOF_GOTO_LIKE)
#define _DeeAst_Dbg_MallocUninitializedGotoLike(file, line)    ((DeeAst *)DeeDbgObject_FMalloc(_DeeAst_SIZEOF_GOTO_LIKE, file, line))
#define _DeeAst_Dbg_TryMallocUninitializedGotoLike(file, line) ((DeeAst *)DeeDbgObject_FTryMalloc(_DeeAst_SIZEOF_GOTO_LIKE, file, line))
#define _DeeAst_Dbg_FreeUninitializedGotoLike(p, file, line)   DeeDbgObject_FFree(p, _DeeAst_SIZEOF_GOTO_LIKE, file, line)

#ifdef CONFIG_BUILDING_DEEMON
#undef DeeAst_HAVE_DBG
#if defined(NDEBUG) || defined(NDEBUG_AST)
#define DeeAst_DBG_PARAMS  /* nothing */
#define DeeAst_DBG_ARGS    /* nothing */
#define DeeAst__DBG_PARAMS /* nothing */
#define DeeAst__DBG_ARGS   /* nothing */
#else /* NDEBUG || NDEBUG_AST */
#define DeeAst_HAVE_DBG
#define DeeAst_DBG_PARAMS  char const *file, int line
#define DeeAst_DBG_ARGS    __FILE__, __LINE__
#define DeeAst__DBG_PARAMS , DeeAst_DBG_PARAMS
#define DeeAst__DBG_ARGS   , DeeAst_DBG_ARGS
#endif /* !NDEBUG && !NDEBUG_AST */

/* Initialize "self" from "value", and emit a warning `TPP_W_ASM_STORE_TO_RVALUE`
 * if `Dee_AST_RVALUE_KIND_ASSIGN` needs to be used and `lexer != NULL`. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeAstRValue_FromAst(DeeAstRValue *__restrict self,
                     /*inherit(always)*/ DeeAst *__restrict value,
                     DeeLexer *lexer, enum Dee_ast_rvalue_mode mode);
INTDEF NONNULL((1)) void DCALL
DeeAstRValue_Fini(DeeAstRValue *__restrict self,
                  enum Dee_ast_rvalue_mode mode);


/* Check if `self` can be used in `DeeAstRValue_FromAst()`
 * without triggering a `TPP_W_ASM_STORE_TO_RVALUE` warning. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeAst_IsRValue(DeeAst const *__restrict self);


/* Destroy any type of (fully initialized) AST */
INTDEF NONNULL((1)) void DCALL DeeAst_Destroy_(DeeAst *__restrict self DeeAst__DBG_PARAMS);
INTDEF void DCALL DeeAst_Destroyv(DeeAst *const *vector, size_t count);
#define DeeAst_Destroy(self)  DeeAst_Destroy_(self DeeAst__DBG_ARGS)
#define DeeAst_XDestroy(self) (void)((self) && (DeeAst_Destroy(self), 1))



LOCAL WUNUSED NONNULL((2)) DeeAst *DCALL
DeeAst_TrySetLocHere(/*0..1*/DeeAst *self, DeeLexer *__restrict lexer) {
	if (likely(self) && unlikely(DeeAst_SetLocHere(self, lexer))) {
		DeeAst_Destroy(self);
		self = NULL;
	}
	return self;
}


#undef DeeAst_HAVE_PRINT_REPR
#if !defined(NDEBUG) || 1
#define DeeAst_HAVE_PRINT_REPR
#endif /* !NDEBUG */

#ifdef DeeAst_HAVE_PRINT_REPR
typedef uint_fast8_t Dee_ast_printrepr_flag;
#define Dee_AST_PRINTREPR_FLAG_RAW    0x00 /* Allow brace-less multi-statements (but still: print statements) */
#define Dee_AST_PRINTREPR_FLAG_ASEXPR 0x01 /* Force-format as an expression (else: format as a statement) */
#define Dee_AST_PRINTREPR_FLAG_ASSTMT 0x02 /* Force-format as a statement */
#define Dee_AST_PRINTREPR_FLAG_ASBOTH 0x03 /* Allow either a statement- or an expression */
#define Dee_AST_PRINTREPR_FLAG_PAREN  0x04 /* Need parenthesis around expression. Set alongside `Dee_AST_PRINTREPR_FLAG_ASEXPR`
                                            * if something like `a + b` being inserted at your place would case problems, and
                                            * you'd need that something to be `(a + b)` instead in order to fix such problems. */

/* Print a user-code representation of `self` */
INTDEF WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr(DeeAst const *__restrict self,
                 Dee_ast_printrepr_flag flag, size_t indent_level,
                 Dee_formatprinter_t printer, void *arg);
#endif /* DeeAst_HAVE_PRINT_REPR */

/* AST API */
INTDEF WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_NewConst_(DREF /*inherit(always)*/ DeeObject *__restrict value DeeAst__DBG_PARAMS);
#define DeeAst_NewConst(/*inherit(always)*/ value) DeeAst_NewConst_(value DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_NewConstString_(/*utf-8*/ char const *__restrict value, size_t value_len DeeAst__DBG_PARAMS);
#define DeeAst_NewConstString(value, value_len) DeeAst_NewConstString_(value, value_len DeeAst__DBG_ARGS)
INTDEF WUNUSED DeeAst *DCALL DeeAst_NewMulti_(enum Dee_ast_multi_kind kind, size_t count, /*inherit(always)*/ DeeAst *const *asts DeeAst__DBG_PARAMS);
#define DeeAst_NewMulti(kind, count, asts) DeeAst_NewMulti_(kind, count, asts DeeAst__DBG_ARGS)
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_PackMulti_(DeeAst *__restrict self, enum Dee_ast_multi_kind kind, size_t count DeeAst__DBG_PARAMS);
#define DeeAst_PackMulti(self, kind, count) DeeAst_PackMulti_(self, kind, count DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((4)) DeeAst *DCALL DeeAst_NewLoop_(Dee_ast_loop_flag flag, /*inherit(always)*/ DeeAst *opt_cond_or_elem, /*inherit(always)*/ DeeAst *opt_next_or_iter, /*inherit(always)*/ DeeAst *body DeeAst__DBG_PARAMS);
#define DeeAst_NewLoop(flag, /*inherit(always)*/ opt_cond_or_elem, /*inherit(always)*/ opt_next_or_iter, /*inherit(always)*/ body) DeeAst_NewLoop_(flag, opt_cond_or_elem, opt_next_or_iter, body DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2)) DeeAst *DCALL DeeAst_NewIf_(Dee_ast_if_flag flag, /*inherit(always)*/ DeeAst *cond, /*inherit(always)*/ DeeAst *opt_tt, /*inherit(always)*/ DeeAst *opt_ff DeeAst__DBG_PARAMS);
#define DeeAst_NewIf(flag, /*inherit(always)*/ cond, /*inherit(always)*/ opt_tt, /*inherit(always)*/ opt_ff) DeeAst_NewIf_(flag, cond, opt_tt, opt_ff DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_NewSwitchStmt_(/*inherit(always)*/ DeeAst *__restrict expr, /*inherit(always)*/ DeeAst *__restrict body DeeAst__DBG_PARAMS);
#define DeeAst_NewSwitchStmt(/*inherit(always)*/ expr, /*inherit(always)*/ body) DeeAst_NewSwitchStmt_(expr, body DeeAst__DBG_ARGS)
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_PackSwitchExpr_(/*inherit(always)*/ DeeAst *__restrict self, /*inherit(always)*/ DeeAst *__restrict expr, size_t defcase, size_t casec DeeAst__DBG_PARAMS);
#define DeeAst_PackSwitchExpr(/*inherit(always)*/ self, /*inherit(always)*/ expr, defcase, casec) DeeAst_PackSwitchExpr_(self, expr, defcase, casec DeeAst__DBG_ARGS)
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_PackCatch_(/*inherit(always)*/ DeeAst *__restrict self, /*inherit(always)*/ DeeAst *__restrict body, size_t handlerc DeeAst__DBG_PARAMS);
#define DeeAst_PackCatch(/*inherit(always)*/ self, /*inherit(always)*/ body, handlerc) DeeAst_PackCatch_(self, body, handlerc DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_NewFinally_(/*inherit(always)*/ DeeAst *__restrict body, /*inherit(always)*/ DeeAst *__restrict finally_ DeeAst__DBG_PARAMS);
#define DeeAst_NewFinally(/*inherit(always)*/ body, /*inherit(always)*/ finally_) DeeAst_NewFinally_(body, finally_ DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_NewYield_(/*inherit(always)*/ DeeAst *__restrict expr DeeAst__DBG_PARAMS);
#define DeeAst_NewYield(/*inherit(always)*/ expr) DeeAst_NewYield_(expr DeeAst__DBG_ARGS)
INTDEF WUNUSED DeeAst *DCALL DeeAst_NewReturn_(/*inherit(always)*/ DeeAst *opt_expr DeeAst__DBG_PARAMS);
#define DeeAst_NewReturn(/*inherit(always)*/ opt_expr) DeeAst_NewReturn_(opt_expr DeeAst__DBG_ARGS)
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 4)) DeeAst *DCALL
DeeAst_PackFunction_(/*inherit(always)*/ DeeAst *__restrict self,
                     Dee_ast_function_flag flag, tpp_keyword const *opt_name,
                     /*inherit(always)*/ DeeAst *__restrict body,
                     /*inherit(always)*/ struct Dee_ast_function_param_base *opt_varargs,
                     /*inherit(always)*/ struct Dee_ast_function_param_base *opt_varkwds,
                     size_t argc_min, size_t argc_max DeeAst__DBG_PARAMS);
#define DeeAst_PackFunction(/*inherit(always)*/ self, flag, opt_name, /*inherit(always)*/ body, /*inherit(always)*/ opt_varargs, /*inherit(always)*/ opt_varkwds, argc_min, argc_max) \
	DeeAst_PackFunction_(self, flag, opt_name, body, opt_varargs, opt_varkwds, argc_min, argc_max DeeAst__DBG_ARGS)
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 3)) DeeAst *DCALL DeeAst_PackClass_(/*inherit(always)*/ DeeAst *__restrict self, /*inherit(always)*/ DeeAst *opt_base, /*inherit(always)*/ DeeAst *__restrict desc, size_t memberc DeeAst__DBG_PARAMS);
#define DeeAst_PackClass(/*inherit(always)*/ self, /*inherit(always)*/ opt_base, /*inherit(always)*/ desc, memberc) DeeAst_PackClass_(self, opt_base, desc, memberc DeeAst__DBG_ARGS)
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_PackUnpackWr_(/*inherit(always)*/ DeeAst *__restrict self,
                     /*inherit(always)*/ DeeAst *__restrict expr,
                     size_t targetc DeeAst__DBG_PARAMS);
#define DeeAst_PackUnpackWr(/*inherit(always)*/ self, /*inherit(always)*/ expr, targetc) \
	DeeAst_PackUnpackWr_(self, expr, targetc DeeAst__DBG_ARGS)
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_PackIAsm_(/*inherit(always)*/ DeeAst *__restrict self, Dee_ast_iasm_flag flag,
                 tpp_char const *text, tpp_size text_size, tpp_string *opt_text_chunk,
                 tpp_lcinfo text_chunk_lc, size_t opc_o, size_t opc_i, size_t opc_l DeeAst__DBG_PARAMS);
#define DeeAst_PackIAsm(/*inherit(always)*/ self, flag, text, text_size, opt_text_chunk, text_chunk_lc, opc_o, opc_i, opc_l) \
	DeeAst_PackIAsm_(self, flag, text, text_size, opt_text_chunk, text_chunk_lc, opc_o, opc_i, opc_l DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 3)) DeeAst *DCALL DeeAst_NewOperatorEx1_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid, Dee_ast_operator_flag flag, /*inherit(always)*/ DeeAst *__restrict arg0 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperatorEx1(/*inherit(always)*/ opid, flag, /*inherit(always)*/ arg0) DeeAst_NewOperatorEx1_(opid, flag, arg0 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 3, 4)) DeeAst *DCALL DeeAst_NewOperatorEx2_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid, Dee_ast_operator_flag flag, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperatorEx2(/*inherit(always)*/ opid, flag, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1) DeeAst_NewOperatorEx2_(opid, flag, arg0, arg1 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 3, 4, 5)) DeeAst *DCALL DeeAst_NewOperatorEx3_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid, Dee_ast_operator_flag flag, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1, /*inherit(always)*/ DeeAst *__restrict arg2 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperatorEx3(/*inherit(always)*/ opid, flag, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1, /*inherit(always)*/ arg2) DeeAst_NewOperatorEx3_(opid, flag, arg0, arg1, arg2 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) DeeAst *DCALL DeeAst_NewOperatorEx4_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid, Dee_ast_operator_flag flag, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1, /*inherit(always)*/ DeeAst *__restrict arg2, /*inherit(always)*/ DeeAst *__restrict arg3 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperatorEx4(/*inherit(always)*/ opid, flag, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1, /*inherit(always)*/ arg2, /*inherit(always)*/ arg3) DeeAst_NewOperatorEx4_(opid, flag, arg0, arg1, arg2, arg3 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2)) DeeAst *DCALL DeeAst_NewOperator1_(Dee_operator_t op, /*inherit(always)*/ DeeAst *__restrict arg0 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperator1(op, /*inherit(always)*/ arg0) DeeAst_NewOperator1_(op, arg0 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_NewOperator2_(Dee_operator_t op, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperator2(op, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1) DeeAst_NewOperator2_(op, arg0, arg1 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3, 4)) DeeAst *DCALL DeeAst_NewOperator3_(Dee_operator_t op, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1, /*inherit(always)*/ DeeAst *__restrict arg2 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperator3(op, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1, /*inherit(always)*/ arg2) DeeAst_NewOperator3_(op, arg0, arg1, arg2 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3, 4, 5)) DeeAst *DCALL DeeAst_NewOperator4_(Dee_operator_t op, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1, /*inherit(always)*/ DeeAst *__restrict arg2, /*inherit(always)*/ DeeAst *__restrict arg3 DeeAst__DBG_PARAMS);
#define DeeAst_NewOperator4(op, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1, /*inherit(always)*/ arg2, /*inherit(always)*/ arg3) DeeAst_NewOperator4_(op, arg0, arg1, arg2, arg3 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2)) DeeAst *DCALL DeeAst_NewAct1_(enum Dee_ast_kind act, /*inherit(always)*/ DeeAst *__restrict arg0 DeeAst__DBG_PARAMS);
#define DeeAst_NewAct1(act, /*inherit(always)*/ arg0) DeeAst_NewAct1_(act, arg0 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_NewAct2_(enum Dee_ast_kind act, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1 DeeAst__DBG_PARAMS);
#define DeeAst_NewAct2(act, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1) DeeAst_NewAct2_(act, arg0, arg1 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3, 4)) DeeAst *DCALL DeeAst_NewAct3_(enum Dee_ast_kind act, /*inherit(always)*/ DeeAst *__restrict arg0, /*inherit(always)*/ DeeAst *__restrict arg1, /*inherit(always)*/ DeeAst *__restrict arg2 DeeAst__DBG_PARAMS);
#define DeeAst_NewAct3(act, /*inherit(always)*/ arg0, /*inherit(always)*/ arg1, /*inherit(always)*/ arg2) DeeAst_NewAct3_(act, arg0, arg1, arg2 DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_NewGoto_(tpp_keyword const *label_name DeeAst__DBG_PARAMS);
#define DeeAst_NewGoto(label_name) DeeAst_NewGoto_(label_name DeeAst__DBG_ARGS)
INTDEF WUNUSED DeeAst *DCALL DeeAst_NewBreak_(tpp_keyword const *opt_label_name DeeAst__DBG_PARAMS);
#define DeeAst_NewBreak(opt_label_name) DeeAst_NewBreak_(opt_label_name DeeAst__DBG_ARGS)
INTDEF WUNUSED DeeAst *DCALL DeeAst_NewContinue_(tpp_keyword const *opt_label_name DeeAst__DBG_PARAMS);
#define DeeAst_NewContinue(opt_label_name) DeeAst_NewContinue_(opt_label_name DeeAst__DBG_ARGS)

/* Symbol functions */
INTDEF WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_NewSymRefByName_(tpp_keyword const *__restrict name, enum Dee_ast_symref_byname_kind kind, Dee_ast_symref_byname_flag flag DeeAst__DBG_PARAMS);
#define DeeAst_NewSymRefByName(name, kind, flag) DeeAst_NewSymRefByName_(name, kind, flag DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_NewSymRefNth_(tpp_keyword const *__restrict name, /*inherit(always)*/ DeeAst *__restrict nth DeeAst__DBG_PARAMS);
#define DeeAst_NewSymRefNth(name, /*inherit(always)*/ nth) DeeAst_NewSymRefNth_(name, nth DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_NewSymRefExtern_(/*inherit(always)*/ DREF struct Dee_module_object *__restrict mod, struct Dee_module_symbol *__restrict sym DeeAst__DBG_PARAMS);
#define DeeAst_NewSymRefExtern(/*inherit(always)*/ mod, sym) DeeAst_NewSymRefExtern_(mod, sym DeeAst__DBG_ARGS)
INTDEF WUNUSED DeeAst *DCALL DeeAst_NewSymRefSimple_(enum Dee_ast_symref_kind kind DeeAst__DBG_PARAMS);
#define DeeAst_NewSymRefThis()       DeeAst_NewSymRefSimple_(Dee_AST_SYMREF_KIND_THIS DeeAst__DBG_ARGS)
#define DeeAst_NewSymRefTpThis()     DeeAst_NewSymRefSimple_(Dee_AST_SYMREF_KIND_TP_THIS DeeAst__DBG_ARGS)
#define DeeAst_NewSymRefThisModule() DeeAst_NewSymRefSimple_(Dee_AST_SYMREF_KIND_THISMOD DeeAst__DBG_ARGS)
#define DeeAst_NewSymRefVarargs()    DeeAst_NewSymRefSimple_(Dee_AST_SYMREF_KIND_VARARGS DeeAst__DBG_ARGS)


/* Label functions */
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_AddGotoLabel_(/*inherit(always)*/ DeeAst *__restrict self, tpp_keyword const *__restrict name DeeAst__DBG_PARAMS);
#define DeeAst_AddGotoLabel(/*inherit(always)*/ self, name) DeeAst_AddGotoLabel_(self, name DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_AddCaseLabel_(/*inherit(always)*/ DeeAst *__restrict self, /*inherit(always)*/ DeeAst *__restrict expr DeeAst__DBG_PARAMS);
#define DeeAst_AddCaseLabel(/*inherit(always)*/ self, /*inherit(always)*/ expr) DeeAst_AddCaseLabel_(self, expr DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_AddDefaultLabel_(/*inherit(always)*/ DeeAst *__restrict self DeeAst__DBG_PARAMS);
#define DeeAst_AddDefaultLabel(/*inherit(always)*/ self) DeeAst_AddDefaultLabel_(self DeeAst__DBG_ARGS)


/* Misc. helpers... */
INTDEF WUNUSED NONNULL((2)) DeeAst *DCALL DeeAst_MakeCall_(/*0..1*/ DeeAstLoc const *loc, /*inherit(always)*/ DeeAst *__restrict func, size_t argc, /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS);
#define DeeAst_MakeCall(loc, /*inherit(always)*/ func, argc, /*inherit(always)*/ argv) DeeAst_MakeCall_(loc, func, argc, argv DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_MakeCallSymRefExtern_(/*0..1*/ DeeAstLoc const *loc, /*inherit(always)*/ DREF struct Dee_module_object *__restrict mod, struct Dee_module_symbol *__restrict sym, size_t argc, /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS);
#define DeeAst_MakeCallSymRefExtern(loc, /*inherit(always)*/ mod, sym, argc, /*inherit(always)*/ argv) DeeAst_MakeCallSymRefExtern_(loc, mod, sym, argc, argv DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2)) DeeAst *DCALL DeeAst_MakeImportModule_(/*0..1*/ DeeAstLoc const *loc, /*inherit(always)*/ DeeAst *__restrict module_name DeeAst__DBG_PARAMS); /* Generate a call to `deemon.import()` using `module_name` as argument */
#define DeeAst_MakeImportModule(loc, module_name) DeeAst_MakeImportModule_(loc, module_name DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_MakeSymRefExternByName_(/*0..1*/ DeeAstLoc const *loc, /*utf-8*/ char const *module_name, /*utf-8*/char const *symbol_name, DeeLexer *__restrict lexer DeeAst__DBG_PARAMS);
#define DeeAst_MakeSymRefExternByName(loc, module_name, symbol_name, lexer) DeeAst_MakeSymRefExternByName_(loc, module_name, symbol_name, lexer DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_MakeInstanceMethod_(/*0..1*/ DeeAstLoc const *loc, /*inherit(always)*/ DeeAst *__restrict method, /*inherit(always)*/ DeeAst *__restrict thisarg DeeAst__DBG_PARAMS); /* import.deemon.InstanceMethod({method}, {thisarg}) */
#define DeeAst_MakeInstanceMethod(loc, /*inherit(always)*/ method, /*inherit(always)*/ thisarg) DeeAst_MakeInstanceMethod_(loc, method, thisarg DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_MakeOperatorFunction_(/*0..1*/ DeeAstLoc const *loc, /*inherit(always)*/ struct Dee_ast_operator_id *__restrict opid, DeeLexer *__restrict lexer DeeAst__DBG_PARAMS);
#define DeeAst_MakeOperatorFunction(loc, /*inherit(always)*/ opid, lexer) DeeAst_MakeOperatorFunction_(loc, opid, lexer DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_MakeCallAttr_(/*0..1*/ DeeAstLoc const *loc, /*inherit(always)*/ DeeAst *__restrict obj, /*inherit(always)*/ DeeAst *__restrict attr, size_t argc, /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS); /* {obj}.operator . ({attr})({argv...}) */
#define DeeAst_MakeCallAttr(loc, /*inherit(always)*/ obj, /*inherit(always)*/ attr, argc, /*inherit(always)*/ argv) DeeAst_MakeCallAttr_(loc, obj, attr, argc, argv DeeAst__DBG_ARGS)
INTDEF WUNUSED NONNULL((2, 3)) DeeAst *DCALL DeeAst_MakeCallAttrConst_(/*0..1*/ DeeAstLoc const *loc, /*inherit(always)*/ DeeAst *__restrict obj, /*inherit(always)*/ /*string*/ DREF DeeObject *__restrict attr, size_t argc, /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS); /* {obj}.operator . ({attr})({argv...}) */
#define DeeAst_MakeCallAttrConst(loc, /*inherit(always)*/ obj, /*inherit(always)*/ /*string*/ attr, argc, /*inherit(always)*/ argv) DeeAst_MakeCallAttrConst_(loc, obj, attr, argc, argv DeeAst__DBG_ARGS)

#endif /* CONFIG_BUILDING_DEEMON */

#else /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

struct ast;


#undef CONFIG_AST_IS_STRUCT
//#define CONFIG_AST_IS_STRUCT /* Can't be done. - Otherwise, the GC visiting ASTs wouldn't work! */


struct catch_expr {
	DREF struct ast   *ce_mask;    /* [0..1] An optional expression evaluated before entry to check.
	                                *        This expression should evaluate to a type that describes
	                                *        a mask which should be used to determine if an error
	                                *        should be handled by this exception handler.
	                                *  NOTE: When this is an AST_MULTIPLE:AST_FMULTIPLE_TUPLE,
	                                *        or a constant tuple, then its elements are used as
	                                *        catch-masks instead. */
	DREF struct ast   *ce_code;    /* [1..1] The expression evaluated to handle the exception.
	                                *        When and how this block is evaluated depends on
	                                *        the `Dee_EXCEPTION_HANDLER_FFINALLY` flag in `ce_flags`:
	                                * When set this expression is evaluated:
	                                *    - during regular code-flow when `:t_guard` returns normally
	                                *    - when `:t_guard` invoked a `return` statement, or
	                                *      before destruction of a yield-iterator that stopped
	                                *      execution within `:t_guard`.
	                                *    - when `:t_guard` threw an exception, regardless of
	                                *      whether or not that exception had already been handled
	                                *      by other catch expressions.
	                                * When the flag is not set, the expression is evaluated:
	                                *    - When `:t_guard` threw an exception that had not
	                                *      been handled by another catch expression, and
	                                *      `ce_mask` is either `NULL`, or contains an expression
	                                *      for which the following code evaluates to true:
	                                *      `CURRENT_EXCEPTION() is ce_mask`.
	                                */
	uint16_t             ce_flags; /* Catch block flags (Set of `EXCEPTION_HANDLER_F*`). */
#define CATCH_EXPR_FNORMAL 0x0000  /* Normal catch processing. */
#define CATCH_EXPR_FSECOND 0x0001  /* If the catch-expression is a catch-handler (as opposed to being a finally-handler),
                                    * do not generate code to discard secondary exceptions if the handler throws an
                                    * additional primary exception:
                                    * >> try {
                                    * >>     throw "foo";
                                    * >> } catch (...) {
                                    * >>     throw "bar";
                                    * >> }
                                    * With this flag set, any exception thrown in the catch-handler (`"bar"'),
                                    * will cause the previous primary (then secondary) exception (`"foo"') to
                                    * be discarded before exception handling is continued normally.
                                    * However if this flag isn't set, both `"foo"' and `"bar"' remain active
                                    * exception, leaving `"bar"' to be discarded once the associated function
                                    * frame ends.
                                    * This flag is usually set by the AST optimizer if it is detected that
                                    * the handler branch never throws an exception, or is itself protected with
                                    * another handler capable of handling at least all non-interrupt exceptions.
                                    * Note however that if this flag is set and `ce_code` still manages to throw
                                    * an exception, both exceptions will remain active, with all but the one
                                    * thrown first then being discarded during stackframe cleanup. */
	uint16_t             ce_mode;  /* Catch mode flags (Set of `CATCH_EXPR_F*`). */
};

struct class_member {
	DREF struct ast *cm_ast;     /* [1..1] The AST that is evaluated and assigned to the member. */
	uint16_t         cm_index;   /* The member index/operator id of the member.
	                              * NOTE: When set to (uint16_t)-1, rather than initializing a
	                              *       class member, `cm_ast` is evaluated, with its result
	                              *       simply being discarded. */
	uint16_t         cm_pad[(sizeof(void *) / 2) - 1]; /* ... */
};

#ifndef CONFIG_LANGUAGE_NO_ASM
struct asm_text {
	TPP_REF tpp_string *at_text; /* [0..1] Assembly text string. */
};
#endif /* !CONFIG_LANGUAGE_NO_ASM */

struct asm_operand {
#ifndef CONFIG_LANGUAGE_NO_ASM
	tpp_keyword const     *ao_name;  /* [0..1] User-defined name for this operand. */
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	TPP_REF tpp_string    *ao_type;  /* [0..1][if((self - :as_opv) <  :as_num_o+:as_num_i, [1..1])]
	                                  *       [if((self - :as_opv) >= :as_num_o+:as_num_i, [0..0])]
	                                  * Allowed operand types (A string of `ASM_OP_*` from `genasm-userasm.c`).
	                                  * NOTE: Only, and always `NULL` for label operands. */
	union {
		DREF struct ast   *ao_expr;  /* [1..1][valid_if((self - :as_opv) < :as_num_o+:as_num_i)] Input/output operand expression. */
		struct text_label *ao_label; /* [1..1][valid_if((self - :as_opv) >= :as_num_o+:as_num_i)] Label operand.
		                              * NOTE: Also holds a reference to `tl_goto` */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define ao_expr  _dee_aunion.ao_expr  /*!export-*/
#define ao_label _dee_aunion.ao_label /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};
#define ASM_OPERAND_IS_INOUT(x) (tpp_string_str((x)->ao_type)[0] == '+')


struct ast {
#ifdef CONFIG_AST_IS_STRUCT
	Dee_refcnt_t         a_refcnt; /* [lock(DeeCompiler_Lock)] Reference counter. */
#else /* CONFIG_AST_IS_STRUCT */
	Dee_OBJECT_HEAD
#endif /* !CONFIG_AST_IS_STRUCT */
	/* XXX: Maybe not use an object for this, but rather a raw struct? */
	DREF DeeScopeObject *a_scope; /* [1..1] The scope in which this AST exists. */
	struct ast_loc       a_ddi;   /* [OVERRIDE(.l_file, REF(TPPFile_Decref) [0..1])]
	                               * Debug information describing the location of this AST. */
	uint16_t             a_type;  /* AST Type (One of `AST_*`) */
#define AST_FNORMAL      0x0000   /* Normal AST flags. */
	uint16_t             a_flag;  /* AST Flags (Set of `AST_F*`, dependent on `a_type`) */
	uint16_t             a_temp;  /* Temporary value used during assembly. */
	uint16_t             a_pad;   /* ... */
	union {

		/* Base expressions (symbol / constant) */
#define AST_CONSTEXPR        0x0000        /* `42` */
		DREF DeeObject      *a_constexpr;  /* [1..1] Constant (aka. compile-time) expression.
		                                    * NOTE: When set to `none`, this AST can be used as a noop expression. */

#define AST_SYM              0x0001        /* `foo` */
		/* TODO: Once `CONFIG_EXPERIMENTAL_USE_TPP3` has become mandatory, do another re-work of
		 *       how symbols work: during parsing, symbol references here should only consist of:
		 * - A symbol name (tpp_keyword)
		 * - Creation description (how the variable should be created if it doesn't already exist)
		 *
		 * Once a source file has been parsed, there needs to be another step prior to optimization
		 * that links symbol references to actual symbols (this is the step that *actually* creates
		 * scopes and adds symbols to them)
		 *
		 * Also: scopes must live *within* the AST tree, rather than outside of it (any AST-type
		 *       that wants to create additional scope(s) must do so by having a field `struct scope`
		 *       or something similar right here, within the AST tree)
		 *
		 * Once all of that has been done, it will become possible for this "symbol linker" to add
		 * support for forward-declarations of *any* kind of symbol (namely: by having the symbol
		 * linker perform 2 passes: 1. create symbols for references with "Creation description",
		 * and 2. link all symbols that aren't already linked to the nearest scope with a matching
		 * symbol name)
		 *
		 * Additionally, there must be a special `struct function_scope` that is derived from `scope`,
		 * but also keeps track of all symbols that need to be passed to the function as references! */
		struct symbol       *a_sym;        /* [1..1][REF(->s_nread) | REF(->s_wread)] Symbol referenced by this AST.
		                                    * NOTE: When a_flag is non-zero, then the symbol is being written to.
		                                    * NOTE: This symbol is guarantied to be reachable from `a_scope` */

#define AST_UNBIND           0x0002        /* `del foo` */
		struct symbol       *a_unbind;     /* [1..1][REF(->s_nwrite)] The symbol that should be unbound.
		                                    * NOTE: This symbol is guarantied to be reachable from `a_scope` */

#define AST_BOUND            0x0003        /* `bound(foo)` */
		struct symbol       *a_bound;      /* [1..1][REF(->s_nbound)] The symbol that should be checked for being bound.
		                                    * NOTE: This symbol is guarantied to be reachable from `a_scope` */

		/* Statement expressions. */
#define AST_MULTIPLE                    0x0004  /* Multiple, consecutive statements/expressions.
                                                 * Dependent on the context, multiple expressions
                                                 * behave special, such as when used as the element
                                                 * expression in a __foreach statement:
                                                 * >> __foreach(a, b, c: foo)
                                                 * ... in which case they will cause the compiler to
                                                 * generate code for unpacking yielded values and
                                                 * individually assigning them to `a`, `b` and `c`.
                                                 * NOTE: You may use `a_flag` to specify how
                                                 *       multiple expressions should be packed together. */
#   define AST_FMULTIPLE_KEEPLAST       0x0000  /* Default: Evaluate, but discard all but the last expression, which is then propagated as value.
                                                 *          If an `AST_EXPAND` expression appears in the sequence,  */
#   define AST_FMULTIPLE_TUPLE          0x1000  /* Pack all elements into a Tuple, correctly handling `AST_EXPAND` elements. */
#   define AST_FMULTIPLE_LIST           0x1001  /* Pack all elements into a List, correctly handling `AST_EXPAND` elements. */
#   define AST_FMULTIPLE_HASHSET        0x1002  /* Pack all elements into a HashSet, correctly handling `AST_EXPAND` elements. */
#   define AST_FMULTIPLE_DICT           0x1003  /* Pack all elements into a Dict, correctly handling `AST_EXPAND` elements.
                                                 * NOTE: This sequence class requires that `m_astc` is aligned
                                                 *       by 2, with every first used as key, and every second as item.
                                                 *       If it isn't aligned by 2, the last expression is ignored and not compiled. */
#   define AST_FMULTIPLE_GENERIC        0x1008  /* Pack elements into a sequence of unspecified typing (used to encode
                                                 * brace-initializer being encoded as `ASM_CALL_SEQ` instructions). */
#   define AST_FMULTIPLE_GENERIC_SET    0x100a  /* Pack elements into a set-like sequence of unspecified typing */
#   define AST_FMULTIPLE_GENERIC_MAP    0x100b  /* Pack elements into a mapping-like sequence of unspecified typing (used to
                                                 * encode brace-initializer being encoded as `ASM_CALL_MAP` instructions). */
#   define AST_FMULTIPLE_ISSEQUENCE(x) (((x)&0x1000) && ((x)&0x0003) <= 1)
#   define AST_FMULTIPLE_ISGENERIC(x)  (((x)&0x1008) == 0x1008)
#   define AST_FMULTIPLE_ISSET(x)      (((x)&0x1003) == 0x1002)
#   define AST_FMULTIPLE_ISMAP(x)      (((x)&0x1003) == 0x1003)
		struct {
			size_t            m_astc;      /* Amount of expressions. */
			DREF struct ast **m_astv;      /* [1..1][0..m_astc][owned] Vector of expressions. */
		}                     a_multiple;  /* Multiple, consecutive statements/expressions. */

#define AST_RETURN         0x0005    /* `return` / `return foo` */
		DREF struct ast   *a_return; /* [0..1] Return expression (If `NULL`, `none` is returned). */

#define AST_YIELD          0x0006    /* `yield foo` */
		DREF struct ast   *a_yield;  /* [1..1] Yield expression. */

#define AST_THROW          0x0007    /* `throw foo` */
		DREF struct ast   *a_throw;  /* [0..1] The expression being thrown. NOTE: When NULL, this is a `rethrow` statement. */

#define AST_TRY              0x0008  /* `try` statement. */
		struct {
			DREF struct ast    *t_guard;  /* [1..1] The expression that is being guarded. */
			size_t              t_catchc; /* Amount of catch/finally expression. */
			struct catch_expr  *t_catchv; /* [0..t_catchc][owned] Vector of catch/finally expressions.
			                               * NOTE: Multiple handlers are executed in order, lower-indexed
			                               *       handlers being evaluated before greater-index ones. */
		}                    a_try;       /* `try` statement. */

#define AST_LOOP             0x0009          /* Some type of loop statement. (for, while, do..while)
                                              * NOTE: As far as return values go, `AST_LOOP` always evaluates to `none`.
                                              * HINT: The initializer of a for() statement is encoded as an `AST_MULTIPLE`
                                              *       ast containing the initializer first, followed by the loop.
                                              * NOTE: If any of the expressions (`l_cond`, `l_next` or `a_loop`) contain
                                              *       a `continue` statement, control will always continue at `l_cond`.
                                              *       If any of them contain a `break` statement, control will continue after `a_loop`. */
#   define AST_FLOOP_NORMAL         0x0000   /* Normal loop flags. */
#   define AST_FLOOP_POSTCOND       0x0001   /* `l_cond` (when non-`NULL`) is evaluated after `a_loop`,
                                              *  rather than before, essentially resulting in `do..while` behavior. */
#   define AST_FLOOP_FOREACH        0x0002   /* This is a foreach loop:
                                              * `__foreach(x: y) foo(x)` statement.
                                              * HINT: The regular `for (x: y)` statement is implemented using
                                              *       this AST after applying the `__iterself__()` operator to `y`
                                              * NOTE: This flag supersedes `AST_FLOOP_POSTCOND` when both are set.
                                              * NOTE: As far as return values go, `AST_FOREACH` always returns `none`.
                                              * NOTE: If any expression (`l_elem`, `l_iter` or `a_loop`) contains a
                                              *       `continue` statement, control will jump to yield the next element
                                              *       from the iterator, meaning that `y` will be evaluated immediately after.
                                              *       If any of the expression contain a `break` statement,
                                              *       control will jump forward to continue after `a_loop`. */
#   define AST_FLOOP_UNLIKELY       0x8000   /* The loop is unlikely to be run and `a_loop.l_loop` should be written to the cold section. */
		struct {
			union {
				DREF struct ast  *l_elem; /* [0..1][valid_if(AST_FLOOP_FOREACH)]
				                           * The element expression (`x` in the example above).
				                           * NOTE: Just like AST_ACTION:AST_FACTION_STORE, this pointer also holds a write-reference to symbols. */
				DREF struct ast  *l_cond; /* [0..1][valid_if(!AST_FLOOP_FOREACH)] Loop condition. (Either evaluated before, or after the main loop)
				                           * NOTE: Setting this to `NULL` is the same as a constant true. */
			}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
			_dee_aunion
#define l_elem  _dee_aunion.l_elem /*!export-*/
#define l_cond  _dee_aunion.l_cond /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
			;
			union {
				DREF struct ast  *l_iter; /* [1..1][valid_if(AST_FLOOP_FOREACH)] The iterator expression (`y` in the example above).
				                           * NOTE: This expression is only evaluated once! */
				DREF struct ast  *l_next; /* [0..1][valid_if(!AST_FLOOP_FOREACH)] Loop advance expression (Executed after `a_loop`, unless `break` was used) */
			}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
			_dee_aunion
#define l_iter  _dee_aunion.l_iter /*!export-*/
#define l_next  _dee_aunion.l_next /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
			;
			DREF struct ast      *l_loop; /* [0..1] Loop block. */
		}                         a_loop; /* Loop statement. */

#define AST_LOOPCTL    0x000a          /* Special loop statement. */
#   define AST_FLOOPCTL_BRK 0x0000     /* `break` statement. */
#   define AST_FLOOPCTL_CON 0x0001     /* `continue` statement. */

#define AST_CONDITIONAL 0x000b         /* `if (foo) bar(); else baz()` / `foo ? bar() : baz()` */
#   define AST_FCOND_EVAL     0x0000   /* Use regular evaluation, returning the actual value of the chosen branch. */
#   define AST_FCOND_BOOL     0x0001   /* Force the result of the conditional expression to evaluate.
                                        * Using this flag, `AST_CONDITIONAL` is used to implement
                                        * logical and (`&&`) and or (`||`) as follows:
                                        * >> a && b; // (a ? b : ) // With `AST_FCOND_BOOL` set
                                        * >> a || b; // (a ?  : b) // With `AST_FCOND_BOOL` set */
#   define AST_FCOND_LIKELY   0x4000   /* When set, place text for `c_ff` in the cold text-section (at the end of the function). */
#   define AST_FCOND_UNLIKELY 0x8000   /* When set, place text for `c_tt` in the cold text-section (at the end of the function). */
		struct {
			DREF struct ast  *c_cond;  /* [1..1] Conditional expression. */
			DREF struct ast  *c_tt;    /* [0..1][(!= NULL) != (c_ff != NULL)]
			                            * Expression evaluated when the condition is met.
			                            * NOTE: In the event that the same object is assigned
			                            *       to this field, as is for `c_cond`, the expression
			                            *       is only evaluated once at runtime, with the uncast
			                            *       condition then returned as true value:
			                            *    >> `print get_string() ?: "The string was empty";'
			                            *       In the above example, `c_tt == c_cond`, which
			                            *       share the same object internally.
			                            *       With that in mind, the generated assembly will
			                            *       invoke `get_string`, duplicate the result, then
			                            *       check if that value evaluates to true, and if it
			                            *       does, returns the original return value of `get_string`,
			                            *       or otherwise the fallback text.
			                            * NOTE: When the `AST_FCOND_BOOL` flag is set, the expression
			                            *       will evaluate to the boolean representation of the
			                            *       result expression. */
			DREF struct ast  *c_ff;    /* [0..1][(!= NULL) != (c_tt != NULL)]
			                            * Expression evaluated when the condition isn't met.
			                            * NOTE: This branch may also be set to `c_cond`, in which
			                            *       case the result of a condition evaluating to false
			                            *       will be the uncasted condition. */
		}                    a_conditional;

		/* Unary/Binary/etc. Expressions. */
#define AST_BOOL             0x000c  /* `!foo` / `!!foo` */
#   define AST_FBOOL_NORMAL  0x0000  /* Directly convert to a boolean. */
#   define AST_FBOOL_NEGATE  0x0001  /* FLAG: Negate the boolean value. (`!` prefix) */
		DREF struct ast     *a_bool; /* [1..1] The ast to which the bool operator should be applied. */

#define AST_EXPAND           0x000d    /* `foo...` */
		DREF struct ast     *a_expand; /* [1..1] The ast affected by the expand expression. */

#define AST_FUNCTION         0x000e          /* A new function definition. */
		struct {
			DREF struct ast         *f_code;  /* [1..1] The code expression that is executed within this function. */
			DREF DeeBaseScopeObject *f_scope; /* The base scope of this function. */
		}                    a_function; /* A new function definition. */

#define AST_OPERATOR_FUNC    0x000f       /* An operator-function (either bound, or unbound)
                                           * >> x = operator +;   // AST_OPERATOR_POS_OR_ADD (unbound)
                                           * >> x = y.operator +; // AST_OPERATOR_POS_OR_ADD (bound)
                                           * `a_flag` is one of `OPERATOR_*`, or one of `AST_OPERATOR_*`
                                           * When used as function-operand of call-expression, this type
                                           * of AST is transformed into an `AST_OPERATOR` branch.
                                           * Otherwise, it is replaced with an external binding to one
                                           * of the symbols found in the `operators` module:
                                           * >> x = operator +;   // compiles (in this implementation) as `x = __pooad from operators;`
                                           * >> x = y.operator +; // compiles (in this implementation) as `x = (InstanceMethod from deemon)(y, __pooad from operators);` */
		struct {
			DREF struct ast  *of_binding; /* [0..1] The first argument of the operator (if it is bound).
			                               * When set, construct an InstanceMethod object that is bound to this expression. */
		} a_operator_func;


#define AST_OPERATOR         0x0010      /* Invocation of some kind of operator.
                                          * Which one specifically is encoded in `a_flag` as one of `OPERATOR_*`. */
		struct {
			DREF struct ast    *o_op0;   /* [1..1] First (aka. self) operand. */
			DREF struct ast    *o_op1;   /* [0..1] Second operand. */
			DREF struct ast    *o_op2;   /* [0..1] Third operand. */
			DREF struct ast    *o_op3;   /* [0..1] Fourth operand. */
#define AST_OPERATOR_FNORMAL    0x0000   /* Normal operator flags. */
#define AST_OPERATOR_FPOSTOP    0x0001   /* FLAG: Before the operation, create a copy of the self-operator
                                          *       and discard the return value of the operator, instead opting
                                          *       to exit with the self-copy ontop of the stack.
                                          * NOTE: No copy is created if the value of the expression itself isn't used.
                                          * HINT: This flag is used to implement `a++` as `({ __stack local temp = copy a; ++a; temp; })`  */
#define AST_OPERATOR_FVARARGS   0x0002   /* FLAG: The operator always has 2 arguments, the first of which
                                          *       is the self-operand and the second a tuple of arguments
                                          *       to pass to the operator callback. */
#define AST_OPERATOR_FMAYBEPFX  0x0004   /* FLAG: If the operator is an inplace operation, always generate code and don't
                                          *       cause a compiler error if the operation will cause an error at runtime.
                                          *       This flag is usually set for explicit operator invocations. */
#define AST_OPERATOR_FDONTOPT   0x8000   /* Don't perform constant propagation optimization on this branch.
                                          * Usually just set by the optimizer itself, so-as not to re-attempt
                                          * constant propagation after doing so failed before. */
			uint16_t            o_exflag;/* Set of `AST_OPERATOR_F*` */
		}                       a_operator; /* General purpose operator AST. */
		DREF struct ast        *a_operator_ops[4];

#define AST_ACTION              0x0011 /* Perform some special action using expressions. */
#   define AST_FACTION_KINDMASK 0x0fff /* MASK: The kind of action (unique). */
#   define AST_FACTION_ARGCMASK 0x7000 /* MASK: The amount of asts referenced by the action. */
#   define AST_FACTION_ARGCSHFT 12     /* SHIFT: Offset for the argument count. */
#   define AST_FACTION_ARGC_GT(x)      (((x)&AST_FACTION_ARGCMASK) >> AST_FACTION_ARGCSHFT)
#   define AST_FACTION_ARGC_ST(x)      (((x) << AST_FACTION_ARGCSHFT)&AST_FACTION_ARGCMASK)
#   define AST_FACTION_MAYBERUN 0x8000 /* FLAG: When set, optimization is allowed to remove branches
                                        *       of this AST even if they may have side-effects.
                                        *       The main intention of this flag is to implement
                                        *       better compile-time optimizations for `typeof()`
                                        *       and `.class`. */
#   define AST_FACTION_CELL0    0x0000 /* `<>`                        - Create an empty Cell. */
#   define AST_FACTION_CELL1    0x1001 /* `< <act0> >`                - Create an Cell for `<act0>`. */
#   define AST_FACTION_TYPEOF   0x1002 /* `type(<act0>)`              - Return the actual type of `<act0>` */
#   define AST_FACTION_CLASSOF  0x1003 /* `<act0>.class`              - Return the effective class of `<act0>` (Dereference super objects) */
#   define AST_FACTION_SUPEROF  0x1004 /* `<act0>.super`              - Return the super-object of `<act0>` */
#   define AST_FACTION_PRINT    0x1005 /* `print <act0>...,;`         - Print the elements of a sequence in `<act0>` (without linefeed) */
#   define AST_FACTION_PRINTLN  0x1006 /* `print <act0>...;`          - Print the elements of a sequence in `<act0>` */
#   define AST_FACTION_FPRINT   0x2007 /* `print <act0>: <act1>...,;` - Print the elements of a sequence in `<act1>` to a file in `<act0>` (without linefeed) */
#   define AST_FACTION_FPRINTLN 0x2008 /* `print <act0>: <act1>...;`  - Print the elements of a sequence in `<act1>` to a file in `<act0>` */
#   define AST_FACTION_RANGE    0x3009 /* `[<act0>:<act1>, <act2>]`   - Create a new range object for enumerating indices from `<act0>` to `<act1>`, using `<act2>` as step. (constexpr(none) is used to indicate unset action operands) */
#   define AST_FACTION_IS       0x200a /* `<act0> is <act1>`          - Generate a test to check if `<act0>` ~is~ `<act1>` (instanceof / is none) */
#   define AST_FACTION_IN       0x200b /* `<act0> in <act1>`          - A variant of `AST_OPERATOR:OPERATOR_CONTAINS` that evaluates arguments in reverse order. */
#   define AST_FACTION_AS       0x200c /* `<act0> as <act1>`          - Create a super wrapper object for `<act0>` of type `<act1>` */
#   define AST_FACTION_MIN      0x100d /* `<act0> < ...`              - Return the lowest element of a sequence `<act0>` */
#   define AST_FACTION_MAX      0x100e /* `<act0> > ...`              - Return the greatest element of a sequence `<act0>` */
#   define AST_FACTION_SUM      0x100f /* `<act0> + ...`              - Return the sum of all element in sequence `<act0>` */
#   define AST_FACTION_ANY      0x1010 /* `<act0> || ...`             - Return true if any element of a sequence `<act0>` is true. */
#   define AST_FACTION_ALL      0x1011 /* `<act0> && ...`             - Return true if all elements of a sequence `<act0>` are true. */
#   define AST_FACTION_STORE    0x2012 /* `<act0> = <act1>`           - Store an expression in <act1> into that found in <act0>
                                        *  NOTE: This kind of actions holds special references to every `AST_SYM`
                                        *        apart of `<act0>`, including any reached through AST_MULTIPLE:
                                        *        >> --sym_read;
                                        *        >> ++sym_write;
                                        *        This way, symbols can track how often they are written to, or read from. */
#   define AST_FACTION_ASSERT   0x1013 /* `assert <act0>`             - Assert that <act0> evaluates to true at runtime. */
#   define AST_FACTION_ASSERT_M 0x2014 /* `assert <act0>, <act1>`     - Assert that <act0> evaluates to true at runtime,
                                        *                               <act1> is a message that is evaluated and added
                                        *                               to the error message when the assertion fails. */
#   define AST_FACTION_BOUNDATTR 0x2015/* `<act0>.operator . (<act1>) is bound` - Check if the attribute `<act1>` of `<act0>` is currently bound. */
#   define AST_FACTION_BOUNDITEM 0x2016/* `<act0>.operator [] (<act1>) is bound` - Check if the item `<act1>` of `<act0>` is currently bound. */
#   define AST_FACTION_SAMEOBJ  0x2017 /* `<act0> === <act1>`         - Check if <act0> and <act1> are the same object */
#   define AST_FACTION_DIFFOBJ  0x2018 /* `<act0> !== <act1>`         - Check if <act0> and <act1> are the different objects */
#   define AST_FACTION_CALL_KW  0x3019 /* `<act0>(<act1>..., **<act2>)` - Call `act0` with `act1`, while also passing keywords from `act2` */
#   define AST_FACTION_DEEPCOPY 0x101a /* `deepcopy <act0>`           - Create a deep copy of `act0` */
		struct {
			DREF struct ast    *a_act0; /* [0..1] Primary action operand or NULL when not used. */
			DREF struct ast    *a_act1; /* [0..1] Secondary action operand or NULL when not used. */
			DREF struct ast    *a_act2; /* [0..1] Third action operand or NULL when not used. */
		}                       a_action;

#define AST_CLASS               0x0012        /* Create a new class object. */
#define AST_FCLASS_NORMAL       0x0000        /* Normal class flags. */
#define AST_FCLASS_NOWRITESUPER 0x0001        /* When set, don't write `c_base` to `c_supersym`. */
		struct {
			DREF struct ast    *c_base;       /* [0..1] Base/super type. */
			DREF struct ast    *c_desc;       /* [1..1] Class descriptor (usually a constant branch). */
			struct symbol      *c_classsym;   /* [0..1] The symbol to which to assign the class before initializing members.
			                                   * NOTE: This symbol is guarantied to be reachable from `a_scope` */
			struct symbol      *c_supersym;   /* [0..1] The symbol to which to assign the base before initializing members.
			                                   * NOTE: This symbol is guarantied to be reachable from `a_scope` */
			size_t              c_memberc;    /* Amount of member descriptors. */
			struct class_member*c_memberv;    /* [0..c_memberc][owned] Vector of member descriptors. */
		}                       a_class;

#define AST_LABEL               0x0013       /* Define a text/case label at the current address.
                                              * HINT: This AST type usually appears inside of another AST_MULTIPLE,
                                              *       followed by the actual expression that is being addressed. */
#define AST_FLABEL_NORMAL       0x0000       /* Normal label flags. */
#define AST_FLABEL_CASE         0x0001       /* FLAG: This is actually a case label.
                                              * -> `a_label->tl_expr` is valid, but `a_label->tl_name` isn't. */
		struct {
			struct text_label  *l_label;      /* [1..1] The text/case label that is being defined. */
			DREF DeeBaseScopeObject *l_base;  /* [1..1] A reference to the base-scope that owns the label. */
		}                       a_label;

#define AST_GOTO                0x0014       /* Jump to a given label. */
		struct {
			struct text_label       *g_label; /* [1..1] The text label to which to jump.
			                                   * NOTE: This field holds a reference to `->tl_goto` */
			DREF DeeBaseScopeObject *g_base;  /* [1..1] A reference to the base-scope that owns the label. */
		}                    a_goto;

#define AST_SWITCH              0x0015 /* A switch statement. */
#   define AST_FSWITCH_NORMAL   0x0000 /* Normal switch flags (automatically determine mode) */
#   define AST_FSWITCH_NOJMPTAB 0x0001 /* Never generate a jump table to perform the actual switch. Normally,
                                        * all cases that could be expressed as constant key- expressions of a
                                        * Dict, (or even more preferred, indices of a List) are packaged
                                        * together within a compile-time generated jump table that is stored
                                        * as a constant/static variable and used using one of the following
                                        * code patterns (depending on whether or not all of the switch's
                                        * targets share the same stack-alignment, as well as the typing of
                                        * case expressions):
                                        *
                                        * >>#if IDENTICAL_STACK_DEPTHS
                                        * >>     push     const @{ "foo": 1f.SP, "bar": 2f }
                                        * >>#else
                                        * >>     push     const @{ "foo": (1f.SP, 1f.PC), "bar": (2f.SP, 2f.PC) }
                                        * >>#endif
                                        * >>     push     <s_expr>
                                        * >>#if IDENTICAL_STACK_DEPTHS
                                        * >>     push     @3f.PC          // Default case
                                        * >>#else
                                        * >>     push     @(3f.SP, 3f.PC) // Default case
                                        * >>#endif
                                        * >>     callattr top, @"get", #2
                                        * >>#if IDENTICAL_STACK_DEPTHS
                                        * >>     jmp      pop
                                        * >>#else
                                        * >>     unpack   #2
                                        * >>     jmp      pop, #pop
                                        * >>#endif
                                        * >>1:   // case "foo":
                                        * >>     jmp      .done
                                        * >>2:   // case "bar":
                                        * >>     jmp      .done
                                        * >>3:   // default: (non-constant cases would go here, too)
                                        * >>     jmp      .done
                                        * >>.done:
                                        *
                                        * Or when all constant case labels are integer expressions:
                                        * >>     push     <s_expr>
                                        * >>     dup
                                        * >>     push     $3
                                        * >>     cmp      gr, top, pop
                                        * >>     jt       4f
                                        * >>     push     const @(1f, 2f, 3f)
                                        * >>     swap
                                        * >>     getitem  top, pop
                                        * >>     jmp      pop
                                        * >>1:   // case 0:
                                        * >>     jmp      .done
                                        * >>2:   // case 1:
                                        * >>     jmp      .done
                                        * >>3:   // case 2:
                                        * >>     jmp      .done
                                        * >>4:   // default: (non-constant cases would go here, too)
                                        * >>     jmp      .done
                                        * >>.done: */
		struct {
			DREF struct ast   *s_expr;    /* [1..1] The expression on which the switch is enacted. */
			DREF struct ast   *s_block;   /* [1..1] The expression containing all of the labels. */
			struct text_label *s_cases;   /* [0..1][CHAIN(->tl_next)][owned] Chain of case labels.
			                               * NOTE: Also holds a reference to `tl_goto` of every entry.
			                               * NOTE: This chain links cases in order of first -> last appearance. */
			struct text_label *s_default; /* [0..1][owned] The default label of the switch statement.
			                               * NOTE: Also holds a reference to `tl_goto`. */
		}                      a_switch;


#define AST_ASSEMBLY          0x0016   /* User-defined inline assembly (following GCC's __asm__ keyword).
                                        * >> __asm__ [volatile][goto]("text..."
                                        * >>                         [: <ouput_operands>
                                        * >>                         [: <input_operands>
                                        * >>                         [: <clobber_list>
                                        * >>                         [: <label_list>]]]]); */
#define AST_FASSEMBLY_NORMAL   0x0000  /* Normal assembly flags. */
#define AST_FASSEMBLY_FORMAT   0x0001  /* Format the assembly text before parsing it. */
#define AST_FASSEMBLY_VOLATILE 0x0002  /* Do not perform peephole optimization on assembly text.
                                        * Internally, this causes the assembler to create a dangling
                                        * symbol for every instruction that makes it appear as though
                                        * it was an anonymous jump target (which can't be optimized).
                                        * NOTE: This does not disable any optimizations
                                        *       on operands passed to the assembly text. */
#define AST_FASSEMBLY_MEMORY   0x0004  /* The assembly branch may not be exchanged with other nearby branches.
                                        * In other words: Act as a read/write compiler barrier. */
#define AST_FASSEMBLY_CLOBSP   0x0008  /* Don't warn if user-assembly leaves the stack in an unaligned state.
                                        * In any case: that state will be fixed by the assembler. */
#define AST_FASSEMBLY_REACH    0x0010  /* User-assembly can be reached through non-conventional means.
                                        * An example for this would be a jump into the assembly block from another one.
                                        * When this flag is set, the optimizer must assume that the block can always be
                                        * reached, even if that doesn't appear to be the case, similar to how a label
                                        * definition can return without the previous statement returning (only that in
                                        * this case, it's also impossible to determine where the block is reachable from). */
#define AST_FASSEMBLY_NORETURN 0x0020  /* The user-assembly statement doesn't return through normal means.
                                        * User-code should contain something along the lines of a `ret` instruction,
                                        * or something similar.
                                        * NOTE: To prevent confusion, this flag is ignored when user-assembly appears
                                        *       within a try-catch block, meaning that the assembly is allowed to set
                                        *       this flag if the intention is to never return by throwing an error.
                                        * NOTE: Setting this flag improperly will only result in weak undefined behavior,
                                        *       as deemon performs dead-code-elimination twice at `-O3`. Once at AST-level,
                                        *       and another time at assembly-level.
                                        *       The more efficient (and important) one is the assembly-level elimination
                                        *       done using peephole optimization. However the other pass done on ASTs
                                        *       themself is the one effect by this flag, and may cause other branches
                                        *       semantically located after the one with this flag to be deleted right
                                        *       there and then, in essence changing the code as though they were never
                                        *       there at all. */
		struct {
#ifndef CONFIG_LANGUAGE_NO_ASM
			struct asm_text    as_text;  /* The assembly text (not formatted yet). */
#endif /* !CONFIG_LANGUAGE_NO_ASM */
			size_t             as_num_o; /* Amount of output operands. */
			size_t             as_num_i; /* Amount of input operands. */
			size_t             as_num_l; /* Amount of input operands. */
			size_t             as_opc;   /* [== as_num_o + as_num_i + as_num_l] Total number of operands. */
			struct asm_operand*as_opv;   /* [0..as_opc][owned] Vector of assembly operands.
			                              * - 0..as_num_o-1 are output operands.
			                              * - as_num_o..as_num_o+as_num_i-1 are input operands.
			                              * - as_num_o+as_num_i..as_opc-1 are label operands. */
		}                      a_assembly;

	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define a_constexpr      _dee_aunion.a_constexpr     /*!export-*/
#define a_sym            _dee_aunion.a_sym           /*!export-*/
#define a_unbind         _dee_aunion.a_unbind        /*!export-*/
#define a_bound          _dee_aunion.a_bound         /*!export-*/
#define a_multiple       _dee_aunion.a_multiple      /*!export-*/
#define a_return         _dee_aunion.a_return        /*!export-*/
#define a_yield          _dee_aunion.a_yield         /*!export-*/
#define a_throw          _dee_aunion.a_throw         /*!export-*/
#define a_try            _dee_aunion.a_try           /*!export-*/
#define a_loop           _dee_aunion.a_loop          /*!export-*/
#define a_conditional    _dee_aunion.a_conditional   /*!export-*/
#define a_bool           _dee_aunion.a_bool          /*!export-*/
#define a_expand         _dee_aunion.a_expand        /*!export-*/
#define a_function       _dee_aunion.a_function      /*!export-*/
#define a_operator_func  _dee_aunion.a_operator_func /*!export-*/
#define a_operator       _dee_aunion.a_operator      /*!export-*/
#define a_operator_ops   _dee_aunion.a_operator_ops  /*!export-*/
#define a_action         _dee_aunion.a_action        /*!export-*/
#define a_class          _dee_aunion.a_class         /*!export-*/
#define a_label          _dee_aunion.a_label         /*!export-*/
#define a_goto           _dee_aunion.a_goto          /*!export-*/
#define a_switch         _dee_aunion.a_switch        /*!export-*/
#define a_assembly       _dee_aunion.a_assembly      /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#define ast_alloc()              DeeObject_MALLOC(struct ast)
#define ast_dbgalloc(file, line) DeeDbgObject_MALLOC(struct ast, file, line)
#define ast_free(p)              DeeObject_FREE(Dee_REQUIRES_TYPE(struct ast *, p))


/* Helper macros to identify asts */
#define ast_isoperator1(self, op)                                                            \
	((self)->a_type == AST_OPERATOR && (self)->a_flag == (op) && (self)->a_operator.o_op0 && \
	 !((self)->a_operator.o_exflag & (AST_OPERATOR_FVARARGS | AST_OPERATOR_FPOSTOP)))
#define ast_isoperator2(self, op)                                                            \
	((self)->a_type == AST_OPERATOR && (self)->a_flag == (op) && (self)->a_operator.o_op1 && \
	 !((self)->a_operator.o_exflag & (AST_OPERATOR_FVARARGS | AST_OPERATOR_FPOSTOP)))
#define ast_isoperator3(self, op)                                                            \
	((self)->a_type == AST_OPERATOR && (self)->a_flag == (op) && (self)->a_operator.o_op2 && \
	 !((self)->a_operator.o_exflag & (AST_OPERATOR_FVARARGS | AST_OPERATOR_FPOSTOP)))
#define ast_isoperator4(self, op)                                                            \
	((self)->a_type == AST_OPERATOR && (self)->a_flag == (op) && (self)->a_operator.o_op3 && \
	 !((self)->a_operator.o_exflag & (AST_OPERATOR_FVARARGS | AST_OPERATOR_FPOSTOP)))


#define ast_isconstexpr(self, v) ((self)->a_constexpr == Dee_AsObject(v))
#define ast_is_deemon_symbol(self, symbol_id)                 \
	((self)->a_type == AST_SYM &&                             \
	 (self)->a_sym->s_type == SYMBOL_TYPE_EXTERN &&           \
	 (self)->a_sym->s_extern.e_module == &DeeModule_Deemon && \
	 Dee_module_symbol_getindex((self)->a_sym->s_extern.e_symbol) == (symbol_id))



/* Automatically generate moveassign operations when the
 * right-hand-size has been modulated using a copy/deepcopy operator. */
#define AST_SHOULD_MOVEASSIGN(x)                                      \
	(((x)->a_type == AST_OPERATOR && (x)->a_flag == OPERATOR_COPY) || \
	 ((x)->a_type == AST_ACTION && (x)->a_flag == AST_FACTION_DEEPCOPY))
#define AST_ISNONE(x) ((x)->a_type == AST_CONSTEXPR && DeeNone_Check((x)->a_constexpr))
#define AST_HASDDI(x) ((x)->ast_ddi.l_file != NULL)

#ifdef CONFIG_BUILDING_DEEMON

#ifdef CONFIG_AST_IS_STRUCT
INTDEF NONNULL((1)) void DCALL ast_destroy(struct ast *__restrict self);
#define ast_shared(x)           ((x)->a_refcnt > 1)
#define ast_incref(x)           (Dee_ASSERT((x)->a_refcnt), ++(x)->a_refcnt)
#define ast_decref(x)           (Dee_ASSERT((x)->a_refcnt), --(x)->a_refcnt ? (void)0 : ast_destroy(x))
#define ast_decrefv             ast_decrefv
#define ast_decref_likely(x)    (Dee_ASSERT((x)->a_refcnt), unlikely(--(x)->a_refcnt) ? (void)0 : ast_destroy(x))
#define ast_decref_unlikely(x)  (Dee_ASSERT((x)->a_refcnt), likely(--(x)->a_refcnt) ? (void)0 : ast_destroy(x))
#define ast_decref_nokill(x)    (Dee_ASSERT((x)->a_refcnt >= 2), --(x)->a_refcnt)
#define ast_xincref(x)          ((x) ? (void)(Dee_ASSERT((x)->a_refcnt), ++(x)->a_refcnt) : (void)0)
#define ast_xdecref(x)          ((x) ? (Dee_ASSERT((x)->a_refcnt), --(x)->a_refcnt ? (void)0 : ast_destroy(x)) : (void)0)
#define ast_xdecref_unlikely(x) ((x) ? (Dee_ASSERT((x)->a_refcnt), --(x)->a_refcnt ? (void)0 : ast_destroy(x)) : (void)0)
#define ASSERT_AST(ob)          Dee_ASSERT((ob)->a_refcnt >= 1)
#define ASSERT_AST_OPT(ob)      Dee_ASSERT(!(ob) || ((ob)->a_refcnt >= 1))
LOCAL void (DCALL ast_decrefv)(struct ast *const *__restrict p, size_t n) {
	while (n--) {
		struct ast *a = p[n];
		ast_decref(a);
	}
}
#else /* CONFIG_AST_IS_STRUCT */
#define ast_shared(x)          DeeObject_IsShared(x)
/* XXX: I don't think we need to use atomic instructions in reference counters for asts! */
#define ast_incref(x)          Dee_Incref(x)
#define ast_decref(x)          Dee_Decref(x)
#define ast_decrefv(p, n)      (void)Dee_Decrefv(p, n)
#define ast_decref_likely(x)   Dee_Decref_likely(x)
#define ast_decref_unlikely(x) Dee_Decref_unlikely(x)
#define ast_decref_nokill(x)   Dee_DecrefNokill(x)
#define ast_xincref(x)         Dee_XIncref(x)
#define ast_xdecref(x)         Dee_XDecref(x)
#define ast_xdecref_unlikely(x)Dee_XDecref_unlikely(x)
#define ASSERT_AST(ob)         ASSERT_OBJECT_TYPE_EXACT(ob, &DeeAst_Type)
#define ASSERT_AST_OPT(ob)     ASSERT_OBJECT_TYPE_EXACT_OPT(ob, &DeeAst_Type)
INTDEF DeeTypeObject DeeAst_Type;
#endif /* !CONFIG_AST_IS_STRUCT */


/* Set debug information for a given AST.
 * NOTE: This function does not expect the caller to be holding
 *       an actual reference to the file pointed to in `info`.
 *       Instead, it checks if `info->l_file` is equal to the
 *       current source file and only sets debug info when it is.
 *       Otherwise, the assigned debug information will mirror
 *       the current LC location in the active source file.
 * HINT: This function behaves as a noop when `self` is NULL
 * @return: * : == self */
INTDEF NONNULL((2)) struct ast *DFCALL ast_setddi(struct ast *self, struct ast_loc *__restrict info);
INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL ast_sethere(DeeLexer *lexer, /*inherit(always)*/DREF struct ast *self);
/* Same as the set functions above, but don't override existing debug information. */
INTDEF NONNULL((2)) struct ast *DFCALL ast_putddi(struct ast *self, struct ast_loc *__restrict info);
INTDEF WUNUSED NONNULL((1)) struct ast *DFCALL ast_puthere(DeeLexer *lexer, struct ast *self);

/* NOTE: Functions below use `current_scope` to initialize `a_scope`, meaning
 *       that all of them expect the caller to be holding the compiler lock.
 *       Besides that, debug information is initialized to NULL. */

#if defined(NDEBUG) || defined(__INTELLISENSE__)
#define CONFIG_NO_AST_DEBUG
#define DECLARE_AST_GENERATOR(attr, name, args) \
	INTDEF WUNUSED attr DREF struct ast *DCALL name args
#else /* NDEBUG || __INTELLISENSE__ */
#define PRIVATE_AST_GENERATOR_UNPACK_ARGS(...) (__VA_ARGS__, char const *file, int line)
#define DECLARE_AST_GENERATOR(attr, name, args) \
	INTDEF WUNUSED attr DREF struct ast *DCALL name##_d PRIVATE_AST_GENERATOR_UNPACK_ARGS args
#endif /* !NDEBUG && !__INTELLISENSE__ */


/* [AST_CONSTEXPR] */
DECLARE_AST_GENERATOR(NONNULL((1)), ast_constexpr, (DeeObject * __restrict constant_expression));
/* [AST_SYM] */
DECLARE_AST_GENERATOR(NONNULL((1)), ast_sym, (struct symbol * __restrict sym));
/* [AST_UNBIND] */
DECLARE_AST_GENERATOR(NONNULL((1)), ast_unbind, (struct symbol * __restrict sym));
/* [AST_BOUND] */
DECLARE_AST_GENERATOR(NONNULL((1)), ast_bound, (struct symbol * __restrict sym));
/* [AST_MULTIPLE] WARNING: Inherits a heap-allocated vector `exprv` upon success; @param: flags: Set of `AST_FMULTIPLE_*` */
DECLARE_AST_GENERATOR(, ast_multiple, (uint16_t flags, size_t exprc, /*inherit(on_success)*/ DREF struct ast **__restrict exprv));
/* [AST_RETURN] NOTE: `return_expr` may be `NULL` */
DECLARE_AST_GENERATOR(, ast_return, (struct ast *return_expr));
/* [AST_YIELD] */
DECLARE_AST_GENERATOR(NONNULL((1)), ast_yield, (struct ast *__restrict yield_expr));
/* [AST_THROW] NOTE: `throw_expr` may be `NULL` */
DECLARE_AST_GENERATOR(, ast_throw, (struct ast *throw_expr));
/* [AST_TRY] WARNING: Inherits a heap-allocated vector `catchv` upon success */
DECLARE_AST_GENERATOR(NONNULL((1)), ast_try, (struct ast *__restrict guarded_expression, size_t catchc,
                                             /*inherit*/ struct catch_expr *catchv));
/* [AST_TRY] Helper function to create a simple try-finally expression using `a_try` */
DECLARE_AST_GENERATOR(NONNULL((1, 2)), ast_tryfinally, (struct ast *__restrict guarded_expression,
                                                       struct ast *__restrict finally_expression));
/* [AST_LOOP] @parma: flags: Set of `AST_FLOOP_*` */
DECLARE_AST_GENERATOR(, ast_loop, (uint16_t flags, struct ast *elem_or_cond, struct ast *iter_or_next, struct ast *loop));
/* [AST_LOOPCTL] @parma: flags: Set of `AST_FLOOPCTL_**` */
DECLARE_AST_GENERATOR(, ast_loopctl, (uint16_t flags));
/* [AST_CONDITIONAL] @parma: flags: Set of `AST_FCOND_**` */
DECLARE_AST_GENERATOR(NONNULL((2)), ast_conditional, (uint16_t flags, struct ast *cond,
                                                     struct ast *tt_expr, struct ast *ff_expr));
/* [AST_BOOL] @param: flags: Set of `AST_FBOOL_*` */
DECLARE_AST_GENERATOR(NONNULL((2)), ast_bool, (uint16_t flags, struct ast *__restrict expr));
/* [AST_EXPAND] */
DECLARE_AST_GENERATOR(NONNULL((1)), ast_expand, (struct ast *__restrict expr));
/* [AST_FUNCTION] */
DECLARE_AST_GENERATOR(NONNULL((1, 2)), ast_function, (struct ast *__restrict function_code, DeeBaseScopeObject *__restrict scope));
/* [AST_OPERATOR_FUNC] */
DECLARE_AST_GENERATOR(, ast_operator_func, (Dee_operator_t operator_name, struct ast *binding));
/* [AST_OPERATOR] @param: operator_name: One of `OPERATOR_*`.
 *                @param: flags: Set of `AST_OPERATOR_F*` */
DECLARE_AST_GENERATOR(NONNULL((3)), ast_operator1, (Dee_operator_t operator_name, uint16_t flags, struct ast *__restrict opa));
DECLARE_AST_GENERATOR(NONNULL((3, 4)), ast_operator2, (Dee_operator_t operator_name, uint16_t flags, struct ast *__restrict opa, struct ast *__restrict opb));
DECLARE_AST_GENERATOR(NONNULL((3, 4, 5)), ast_operator3, (Dee_operator_t operator_name, uint16_t flags, struct ast *__restrict opa, struct ast *__restrict opb, struct ast *__restrict opc));
DECLARE_AST_GENERATOR(NONNULL((3, 4, 5, 6)), ast_operator4, (Dee_operator_t operator_name, uint16_t flags, struct ast *__restrict opa, struct ast *__restrict opb, struct ast *__restrict opc, struct ast *__restrict opd));
/* [AST_ACTION]  @param: action_flags: One of `AST_FACTION_*` (see above). */
DECLARE_AST_GENERATOR(, ast_action0, (uint16_t action_flags));
DECLARE_AST_GENERATOR(NONNULL((2)), ast_action1, (uint16_t action_flags, struct ast *__restrict act0));
DECLARE_AST_GENERATOR(NONNULL((2, 3)), ast_action2, (uint16_t action_flags, struct ast *__restrict act0, struct ast *__restrict act1));
DECLARE_AST_GENERATOR(NONNULL((2, 3, 4)), ast_action3, (uint16_t action_flags, struct ast *__restrict act0, struct ast *__restrict act1, struct ast *__restrict act2));
/* [AST_CLASS]   @param: class_flags: Set of `TP_F* & 0xf`.
 * WARNING: Inherits a heap-allocated vector `memberv` upon success. */
DECLARE_AST_GENERATOR(NONNULL((2)), ast_class, (struct ast *base, struct ast *__restrict descriptor,
                                               struct symbol *class_symbol, struct symbol *super_symbol,
                                               size_t memberc, struct class_member *memberv));
/* [AST_LABEL] @param: flags: Set of `AST_FLABEL_*`. */
DECLARE_AST_GENERATOR(NONNULL((2, 3)), ast_label, (uint16_t flags, struct text_label *__restrict lbl, DeeBaseScopeObject *__restrict base_scope));
/* [AST_GOTO] */
DECLARE_AST_GENERATOR(NONNULL((2, 3)), ast_goto, (struct text_label * __restrict lbl, DeeBaseScopeObject *__restrict base_scope));
/* [AST_SWITCH] @param: flags: Set of `AST_FSWITCH_*`.
 * WARNING: Inherits both `cases` and `default_case` upon success. */
DECLARE_AST_GENERATOR(NONNULL((2, 3)), ast_switch, (uint16_t flags, struct ast *__restrict expr, struct ast *__restrict block,
                                                   struct text_label *cases, struct text_label *default_case));
/* [AST_ASSEMBLY] WARNING: Inherits a heap-allocated vector `opv` upon success; @param: flags: Set of `AST_FASSEMBLY_*` */
#ifdef CONFIG_LANGUAGE_NO_ASM
DECLARE_AST_GENERATOR(, ast_assembly, (uint16_t flags, size_t num_o, size_t num_i, size_t num_l, /*inherit*/ struct asm_operand *opv));
#else /* !CONFIG_LANGUAGE_NO_ASM */
DECLARE_AST_GENERATOR(NONNULL((2)), ast_assembly, (uint16_t flags, tpp_string *__restrict text, size_t num_o, size_t num_i, size_t num_l, /*inherit*/ struct asm_operand *opv));
#endif /* !CONFIG_LANGUAGE_NO_ASM */

#undef DECLARE_AST_GENERATOR

#ifndef CONFIG_NO_AST_DEBUG
#define ast_constexpr(constant_expression)                          ast_constexpr_d(constant_expression, __FILE__, __LINE__)
#define ast_sym(sym)                                                ast_sym_d(sym, __FILE__, __LINE__)
#define ast_unbind(sym)                                             ast_unbind_d(sym, __FILE__, __LINE__)
#define ast_bound(sym)                                              ast_bound_d(sym, __FILE__, __LINE__)
#define ast_multiple(flags, exprc, exprv)                           ast_multiple_d(flags, exprc, exprv, __FILE__, __LINE__)
#define ast_return(return_expr)                                     ast_return_d(return_expr, __FILE__, __LINE__)
#define ast_yield(yield_expr)                                       ast_yield_d(yield_expr, __FILE__, __LINE__)
#define ast_throw(throw_expr)                                       ast_throw_d(throw_expr, __FILE__, __LINE__)
#define ast_try(guarded_expression, catchc, catchv)                 ast_try_d(guarded_expression, catchc, catchv, __FILE__, __LINE__)
#define ast_tryfinally(guarded_expression, finally_expression)      ast_tryfinally_d(guarded_expression, finally_expression, __FILE__, __LINE__)
#define ast_loop(flags, elem_or_cond, iter_or_next, loop)           ast_loop_d(flags, elem_or_cond, iter_or_next, loop, __FILE__, __LINE__)
#define ast_loopctl(flags)                                          ast_loopctl_d(flags, __FILE__, __LINE__)
#define ast_conditional(flags, cond, tt_expr, ff_expr)              ast_conditional_d(flags, cond, tt_expr, ff_expr, __FILE__, __LINE__)
#define ast_bool(flags, expr)                                       ast_bool_d(flags, expr, __FILE__, __LINE__)
#define ast_expand(expr)                                            ast_expand_d(expr, __FILE__, __LINE__)
#define ast_function(function_code, scope)                          ast_function_d(function_code, scope, __FILE__, __LINE__)
#define ast_operator_func(operator_name, binding)                   ast_operator_func_d(operator_name, binding, __FILE__, __LINE__)
#define ast_operator1(operator_name, flags, opa)                    ast_operator1_d(operator_name, flags, opa, __FILE__, __LINE__)
#define ast_operator2(operator_name, flags, opa, opb)               ast_operator2_d(operator_name, flags, opa, opb, __FILE__, __LINE__)
#define ast_operator3(operator_name, flags, opa, opb, opc)          ast_operator3_d(operator_name, flags, opa, opb, opc, __FILE__, __LINE__)
#define ast_operator4(operator_name, flags, opa, opb, opc, opd)     ast_operator4_d(operator_name, flags, opa, opb, opc, opd, __FILE__, __LINE__)
#define ast_action0(action_flags)                                   ast_action0_d(action_flags, __FILE__, __LINE__)
#define ast_action1(action_flags, act0)                             ast_action1_d(action_flags, act0, __FILE__, __LINE__)
#define ast_action2(action_flags, act0, act1)                       ast_action2_d(action_flags, act0, act1, __FILE__, __LINE__)
#define ast_action3(action_flags, act0, act1, act2)                 ast_action3_d(action_flags, act0, act1, act2, __FILE__, __LINE__)
#define ast_class(base, descriptor, class_symbol, super_symbol, memberc, memberv) \
	ast_class_d(base, descriptor, class_symbol, super_symbol, memberc, memberv, __FILE__, __LINE__)
#define ast_label(flags, lbl, base_scope)                           ast_label_d(flags, lbl, base_scope, __FILE__, __LINE__)
#define ast_goto(lbl, base_scope)                                   ast_goto_d(lbl, base_scope, __FILE__, __LINE__)
#define ast_switch(flags, expr, block, cases, default_case)         ast_switch_d(flags, expr, block, cases, default_case, __FILE__, __LINE__)
#ifdef CONFIG_LANGUAGE_NO_ASM
#define ast_assembly(flags, num_o, num_i, num_l, opv)               ast_assembly_d(flags, num_o, num_i, num_l, opv, __FILE__, __LINE__)
#else /* CONFIG_LANGUAGE_NO_ASM */
#define ast_assembly(flags, text, num_o, num_i, num_l, opv)         ast_assembly_d(flags, text, num_o, num_i, num_l, opv, __FILE__, __LINE__)
#endif /* !CONFIG_LANGUAGE_NO_ASM */
#endif /* !CONFIG_NO_AST_DEBUG */


/* Encode some higher-level operations using ASTs. */
#define ast_continue()  ast_loopctl(AST_FLOOPCTL_CON)         /* `continue` */
#define ast_break()     ast_loopctl(AST_FLOOPCTL_BRK)         /* `break` */
#define ast_land(a, b)  ast_conditional(AST_FCOND_BOOL, a, b, a) /* `a && b` */
#define ast_lor(a, b)   ast_conditional(AST_FCOND_BOOL, a, a, b) /* `a || b` */

#endif /* !CONFIG_BUILDING_DEEMON */
#endif /* !CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_AST_H */
