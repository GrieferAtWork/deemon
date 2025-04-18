/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASSEMBLER_H
#define GUARD_DEEMON_COMPILER_ASSEMBLER_H 1

#include "../api.h"

#ifdef CONFIG_BUILDING_DEEMON
#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>
/**/

#include "../asm.h"
#include "../code.h"
#include "../object.h" /* Dee_operator_t */
#include "../types.h"
#include "ast.h"
#include "symbol.h"
#include "tpp.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* intN_t, uintN_t, uintptr_t */

DECL_BEGIN

struct ascii_printer;

#define R_DMN_NONE      0 /* `// Nothing' */
#define R_DMN_STATIC16  1 /* `u16 = u16 + a_refc;' */
#define R_DMN_ABS8      2 /* `u8  = u8 + ar_sym->as_addr;' */
#define R_DMN_ABS16     3 /* `u16 = u16 + ar_sym->as_addr;' */
#define R_DMN_ABS32     4 /* `u32 = u32 + ar_sym->as_addr;' */
#define R_DMN_DISP8     5 /* `s8  = s8 + (ar_sym->as_addr - ar_addr);' */
#define R_DMN_DISP16    6 /* `s16 = s16 + (ar_sym->as_addr - ar_addr);' */
#define R_DMN_DISP32    7 /* `s32 = s32 + (ar_sym->as_addr - ar_addr);' */
#define R_DMN_STCK8     8 /* `s8  = ar_sym->as_stck - s8;' */
#define R_DMN_STCK16    9 /* `s16 = ar_sym->as_stck - s16;' */
#define R_DMN_STCKA8   10 /* `u8  = u8 + ar_sym->as_stck;' */
#define R_DMN_STCKA16  11 /* `u16 = u16 + ar_sym->as_stck;' */
#define R_DMN_DELHAND  12 /* Special relocation: placed at the start of an instruction chain
                           * consisting of `ASM_ENDCATCH', `ASM_ENDFINALLY' and `ASM_ENDFINALLY_N'
                           * instruction. When executed, do the following:
                           * >> if (ar_value < ar_sym->as_hand) DONE();
                           * >> num_keep   = ar_value - ar_sym->as_hand;
                           * >> num_delete = ar_value - num_keep;
                           * >> iter       = target;
                           * >> while (num_keep--)   iter = DeeAsm_NextInstr(iter);
                           * >> while (num_delete--) target = iter, iter = DeeAsm_NextInstr(iter), DELETE_INSTRUCTION(target);
                           * During code generation, this is used to adjust for the exception
                           * handler offset required for cleaning up active exceptions/finally
                           * blocks before performing a jump to an unknown target:
                           * >> try {
                           * >>     throw 42;
                           * >> } catch (...) {
                           * >>     try {
                           * >>         print "Hello";
                           * >>     } finally {
                           * >>         // This goto will generate the following assembly:
                           * >>         // >> .rel R_DMN_DELHAND, my_label, 2
                           * >>         // >> end       finally, #1
                           * >>         // >> end       catch
                           * >>         // >> adjstack #my_label.SP
                           * >>         // >> jmp       my_label.PC
                           * >>         goto my_label;
                           * >>     }
                           * >> }
                           * NOTE: This relocation is executed prior to peephole optimizations.
                           */
#define R_DMN_COUNT    13 /* Number of relocation types. */
#define R_DMN_FUSER    0x8000 /* FLAG: Set for user-defined relocations. */
#define REL_HASSYM(x) ((x) >= 2)


/* Section indices. */
#define SECTION_TEXT      0 /* Regular text. (Default section) */
#define SECTION_COLD      1 /* Cold text. (During linking, this text will be placed after `a_text') */
#define SECTION_TEXTCOUNT 2 /* The number of text sections. */
#define SECTION_COUNT     2 /* The total number of sections. */
#define SECTION_INVALID   0xffff

struct asm_sym;
struct asm_rel;
struct asm_sec;
struct asm_exc;

struct string_object;
struct module_object;
SLIST_HEAD(asm_sym_slist, asm_sym);

struct asm_sym {
	SLIST_ENTRY(asm_sym) as_link;  /* [0..1][owned] Next symbol. */
	code_addr_t          as_addr;  /* [valid_if(as_sect != SECTION_INVALID)] Address at which this symbol is defined. */
#ifndef CONFIG_LANGUAGE_NO_ASM
	struct TPPKeyword   *as_uname; /* [0..1] Symbol name. */
	struct asm_sym      *as_uhnxt; /* [0..1] Next symbol with the same hash. */
	struct asm_sym      *as_uprev; /* [0..1] Previous iteration of this symbol (used by symbols that can be re-defined; aka. indexed symbols) */
#endif /* !CONFIG_LANGUAGE_NO_ASM */
#define ASM_SYM_STCK_INVALID 0xffff
	uint16_t             as_stck;  /* [valid_if(!= ASM_SYM_STCK_INVALID)]
	                                * The execution stack depth where this symbol is defined. */
	uint16_t             as_sect;  /* Section index or `SECTION_INVALID' when not defined. */
	uint16_t             as_hand;  /* The exception handler depth active where the symbol is defined. */
	uint16_t             as_used;  /* The number of times that this symbols is used in relocations. */
#ifndef NDEBUG
	char const          *as_file;  /* The file where this symbol was allocated. */
	int                  as_line;  /* The line where this symbol was allocated. */
#endif /* !NDEBUG */
};
#define ASM_SYM_DEFINED(x) ((x)->as_sect != SECTION_INVALID)


#if __SIZEOF_POINTER__ == 4
#define CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER
#endif /* __SIZEOF_POINTER__ == 4 */

struct asm_exc {
	DREF DeeTypeObject *ex_mask;  /* [0..1] s.a.: `struct except_handler::eh_mask'. */
	struct asm_sym     *ex_start; /* [1..1] s.a.: `struct except_handler::eh_start' (NOTE: Holds a reference to `as_used'). */
	struct asm_sym     *ex_end;   /* [1..1] s.a.: `struct except_handler::eh_end' (NOTE: Holds a reference to `as_used'). */
	struct asm_sym     *ex_addr;  /* [1..1] s.a.: `struct except_handler::eh_addr' (NOTE: Holds a reference to `as_used'). */
#ifdef CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER
	uint16_t            ex_pad;   /* ... */
#endif /* CONFIG_SIZEOF_ASM_EXC_MATCHES_SIZEOF_EXCEPT_HANDLER */
	uint16_t            ex_flags; /* s.a.: `struct except_handler::eh_flags'. (Set of `EXCEPTION_HANDLER_F*') */
#if __SIZEOF_POINTER__ > 4
	uint32_t            ex_pad2;
#endif /* __SIZEOF_POINTER__ > 4 */
};

struct asm_rel {
	struct asm_sym *ar_sym;    /* [?..1][REF(->as_used)] Symbol to relocate against (Undefined when not used by the relocation). */
	code_addr_t     ar_addr;   /* Address to which the relocation is applied. */
	uint16_t        ar_type;   /* The type of relocation (One of `R_DMN_*'). */
	uint16_t        ar_value;  /* [valid_if(?)] An optional relocation value used by some relocation types.
	                            * When not used by the relocation type, this field is undefined. */
};


/* Delete a given relocation. */
LOCAL NONNULL((1)) void DCALL
asm_reldel(struct asm_rel *__restrict self) {
	if (REL_HASSYM(self->ar_type)) {
		Dee_ASSERT(self->ar_sym);
		Dee_ASSERT(self->ar_sym->as_used);
		--self->ar_sym->as_used;
#ifndef NDEBUG
#if __SIZEOF_POINTER__ == 4
		self->ar_sym = (struct asm_sym *)0xcccccccclu;
#elif __SIZEOF_POINTER__ == 8
		self->ar_sym = (struct asm_sym *)0xccccccccccccccccllu;
#endif /* __SIZEOF_POINTER__... */
#endif /* !NDEBUG */
	}
	self->ar_type = R_DMN_NONE;
}


struct asm_sec {
	DeeCodeObject  *sec_code;  /* [0..1][owned] The final code object to-be returned.
	                            * NOTE: When non-NULL, this is used as buffer for written text.
	                            * NOTE: Code assumes that only the first section has this field always
	                            *       set to non-NULL, while all other sections always use NULL here. */
	instruction_t  *sec_begin; /* [0..1][<= sec_end][owned_if(!sec_code)][if(sec_code, == sec_code->co_code)]
	                            * The base address of allocated section memory. */
	instruction_t  *sec_iter;  /* [0..1][in(sec_begin...sec_end)] The current code position. */
	instruction_t  *sec_end;   /* [0..1][>= sec_begin] Allocated end address. */
	size_t          sec_rela;  /* Allocated amount of relocations. */
	size_t          sec_relc;  /* Amount of relocations currently in use. */
	struct asm_rel *sec_relv;  /* [0..sec_relc|ALLOC(sec_rela)][owned][sort(ASCENDING(ar_addr))]
	                            * Ordered vector of relocations. */
};
#define ASM_SEC_ADDR(x)    ((code_addr_t)((x).sec_iter - (x).sec_begin))
#define ASM_SEC_ISEMPTY(x) ((x)->sec_iter <= (x)->sec_begin)

struct ddi_binding {
#define DDI_BINDING_CLASS_LOCAL 0x0000 /* The binding refers to a local variable. */
#define DDI_BINDING_CLASS_STACK 0x0001 /* The binding refers to a stack variable. */
	uint16_t           db_class; /* The symbol binding class (One of `DDI_BINDING_CLASS_*') */
	uint16_t           db_index; /* The symbol binding index (stack-address, or LID) */
	struct TPPKeyword *db_name;  /* [0..1][const] Name of the symbol (when `NULL', the symbol must be unbound). */
};

struct ddi_checkpoint {
#ifdef GUARD_DEEMON_COMPILER_DDI_C
	union{
	code_addr_t        dc_addr; /* The absolute text address of the symbol.
	                             * In order to reduce indirection, DDI checkpoints
	                             * are updated to contain the actual code addresses
	                             * before debug informations are generated.
	                             * This member is never exposed and only used during an internal assembly phase. */
#endif /* GUARD_DEEMON_COMPILER_DDI_C */
	struct asm_sym    *dc_sym;  /* [1..1] The affected user-instruction.
	                             * NOTE: This pointer also holds a reference to `as_used',
	                             *       meaning that debug information can prevent certain
	                             *       types of peephole optimizations when enabled. */
#ifdef GUARD_DEEMON_COMPILER_DDI_C
	};
#endif /* GUARD_DEEMON_COMPILER_DDI_C */
	struct ast_loc      dc_loc;  /* Source location at this checkpoint.
	                              * NOTE: This member does _NOT_ hold a reference to `.l_file'!
	                              */
	uint16_t            dc_sp;   /* The stack depth at this location. */
	uint16_t            dc_bndc; /* The number of symbol bindings defined by this checkpoint. */
	struct ddi_binding *dc_bndv; /* [0..dc_bndc][owned] Vector of symbol bindings. */
};

struct ddi_assembler {
	size_t                 da_checkc; /* Amount of defined check points. */
	size_t                 da_checka; /* Allocated amount of check points. */
	struct ddi_checkpoint *da_checkv; /* [0..da_checkc|ALLOC(da_checka)][owned] Vector of check points. */
	code_addr_t            da_last;   /* Text address of the last checkpoint (Used to quickly discard
	                                   * redundant debug information during the early assembly phase).
	                                   * NOTE: Duplicate checkpoints are checked for again at a later
	                                   *       time, but since we're creating a lot of these internally,
	                                   *       it shouldn't hurt to get rid of some of them early on. */
	struct asm_sec        *da_slast;  /* The section associated with `da_last'. */
	DREF struct TPPFile   *da_files;  /* [0..1][CHAIN(->f_prev)]
	                                   * Chain of fake DDI files used to implement custom file names. */
	uint16_t               da_bndc;   /* The number of symbol bindings. */
	uint16_t               da_bnda;   /* The allocated number of symbol bindings. */
	struct ddi_binding    *da_bndv;   /* [0..dc_bndc|ALLOC(da_bnda)][owned] Vector of symbol bindings. */
};

/* Allocate and return a new DDI checkpoint.
 * NOTE: Before using this function, the caller should first check if debug
 *       information should be generated by testing the `ASM_FNODDI' flag.
 * WARNING: Upon success, the memory pointed to by the result is in
 *          an undefined state and must be initialized by the caller.
 * HINT: If in the end there are multiple checkpoints for the same address,
 *       debug information provided by the one created first is used. */
INTDEF WUNUSED struct ddi_checkpoint *DCALL asm_newddi(void);

/* Lookup, or construct a fake TPP file for use in DDI checkpoints. */
INTDEF WUNUSED NONNULL((1)) struct TPPFile *DCALL
ddi_newfile(char const *__restrict filename,
            size_t filename_length);

struct handler_frame {
	struct handler_frame *hf_prev;  /* [0..1] Previous exception handler descriptor, or NULL when not set. */
	uint16_t              hf_flags; /* Set of `EXCEPTION_HANDLER_F*' for this handler. */
	uint16_t             _hf_pad;   /* ... */
};

#ifndef CONFIG_LANGUAGE_NO_ASM
#define USERLABEL_PREFIX ".L" /* Prefix */

struct asm_symtab {
	size_t           st_size;  /* Amount of symbols located within the table. */
	size_t           st_alloc; /* Allocated table size. */
	struct asm_sym **st_map;   /* [0..1][CHAIN(->as_hnxt)][0..st_alloc][owned] Symbol table. */
};

struct user_assembler {
	size_t              ua_labelc; /* Amount of user-labels. */
	struct asm_operand *ua_labelv; /* [0..ua_labelc] Vector of user-labels. */
	size_t              ua_asmuid; /* Unique identification number used by `%=' */
	struct asm_symtab   ua_symtab; /* Table for user-defined symbols. */
	uint16_t            ua_flags;  /* Set of `AST_FASSEMBLY_*' describing how user-assembly should be processed. */
	uint16_t            ua_lasti;  /* The last-written user-instruction.
	                                * Used by `.adjstack' directives to determine whether or
	                                * not peephole must be disabled due to unpredictable stack
	                                * miss-alignment.
	                                * NOTE: Set to `ASM_DELOP' when symbols are defined. */
#define USER_ASM_FNORMAL 0x0000    /* Normal user-assembly flags. */
#define USER_ASM_FSTKINV 0x0001    /* The stack has entered an undefined state (can happen when `adjstack' is used with an undefined symbol)
	                                * During this mode, only instructions with a -0/+0 stack effect can be used,
	                                * and the only way to exit this mode is to use a .adjstack instruction. */
	uint16_t            ua_mode;   /* Current user-assembly mode (Set of `USER_ASM_F*'). */
	uint16_t           _ua_pad;    /* ... */
};

struct asm_intexpr {
	tint_t          ie_val; /* Constant expression addend. */
	struct asm_sym *ie_sym; /* [0..1] Symbol who's address is added to the result. */
	uint16_t        ie_rel; /* Relocation mode / value type (One of `ASM_OVERLOAD_FREL*' or
	                         * `ASM_OVERLOAD_FSTK', or (uint16_t)-1 if not defined) */
	uint16_t       _ie_pad; /* ... */
};


/* The max number of operands during mnemonic invocation. */
#define ASM_MAX_INSTRUCTION_OPERANDS 4 /* ASM_SETRANGE has 4 operands. */

struct asm_invoke_operand {
#define OPERAND_CLASS_FMASK            0x00ff     /* MASK: Mask of the effective operand class. */
#define OPERAND_CLASS_FIMMVAL          0x0100     /* FLAG: The operand is prefixed by dollar `$' */
#define OPERAND_CLASS_FBRACKETFLAG     0x0200     /* FLAG: The operand is surrounded by `[...]' */
#define OPERAND_CLASS_FBRACEFLAG       0x0400     /* FLAG: The operand is surrounded by `{...}' */
#define OPERAND_CLASS_FDOTSFLAG        0x0800     /* FLAG: The operand is followed by dots `...' */
#define OPERAND_CLASS_FSTACKFLAG       0x1000     /* FLAG: The operand is prefixed by hash `#' */
#define OPERAND_CLASS_FSTACKFLAG2      0x2000     /* FLAG: The operand is prefixed by a second hash `#' */
#define OPERAND_CLASS_FSPADD           0x4000     /* FLAG: Encode the operand as `SP + imm'. (Only used by for instruction encodings; not invocation) */
#define OPERAND_CLASS_FSPSUB           0x8000     /* FLAG: Encode the operand as `SP - imm'. (Only used by for instruction encodings; not invocation) */
#define OPERAND_CLASS_FSUBSP           0xc000     /* FLAG: Encode the operand as `imm - SP'. (Only used by for instruction encodings; not invocation) */

#define OPERAND_CLASS_UNUSED           0x0000     /* ---Unused operand--- */
#define OPERAND_CLASS_POP              0x0001     /* `pop' */
#define OPERAND_CLASS_TOP              0x0002     /* `top' */
#define OPERAND_CLASS_POP_OR_TOP       0x0003     /* `pop' or `top' */
#define OPERAND_CLASS_REF              0x0004     /* `ref <io_symid>' */
#define OPERAND_CLASS_ARG              0x0005     /* `arg <io_symid>' */
#define OPERAND_CLASS_CONST            0x0006     /* `const <io_symid>' */
#define OPERAND_CLASS_STATIC           0x0007     /* `static <io_symid>' */
#define OPERAND_CLASS_MODULE           0x0008     /* `module <io_symid>' */
#define OPERAND_CLASS_EXTERN           0x0009     /* `extern <io_modid>:<io_symid>' */
#define OPERAND_CLASS_GLOBAL           0x000a     /* `global <io_symid>' */
#define OPERAND_CLASS_LOCAL            0x000b     /* `local <io_symid>' */
#define OPERAND_CLASS_PREFIX           0x000c     /* Any operand accepted by a prefix instruction. */

#define OPERAND_CLASS_ISDISP(x)                           \
	(((x)&OPERAND_CLASS_FMASK) >= OPERAND_CLASS_SDISP8 && \
	 ((x)&OPERAND_CLASS_FMASK) <= OPERAND_CLASS_DISP16_HALF)
#define OPERAND_CLASS_SDISP8           0x0010     /* `<io_intexpr>' (8-bit, signed integral expression) */
#define OPERAND_CLASS_SDISP16          0x0011     /* `<io_intexpr>' (16-bit, signed integral expression) */
#define OPERAND_CLASS_SDISP32          0x0012     /* `<io_intexpr>' (32-bit, signed integral expression) */
#define OPERAND_CLASS_DISP8            0x0013     /* `<io_intexpr>' (8-bit, unsigned integral expression) */
#define OPERAND_CLASS_DISP16           0x0014     /* `<io_intexpr>' (16-bit, unsigned integral expression) */
#define OPERAND_CLASS_DISP32           0x0015     /* `<io_intexpr>' (32-bit, unsigned integral expression) */
#define OPERAND_CLASS_DISP_EQ_N2       0x0016     /* <io_intexpr> == `-2' */
#define OPERAND_CLASS_DISP_EQ_N1       0x0017     /* <io_intexpr> == `-1' */
#define OPERAND_CLASS_DISP_EQ_0        0x0018     /* <io_intexpr> == `0' */
#define OPERAND_CLASS_DISP_EQ_1        0x0019     /* <io_intexpr> == `1' */
#define OPERAND_CLASS_DISP_EQ_2        0x001a     /* <io_intexpr> == `2' */
#define OPERAND_CLASS_DISP_EQ_VALUE(x) ((int8_t)((x)&0x7f)-OPERAND_CLASS_DISP_EQ_0)
#define OPERAND_CLASS_DISP8_HALF       0x001b     /* `<io_intexpr> * 2' (A multiple of 2 who's half can fit into 8 unsigned bits) */
#define OPERAND_CLASS_DISP16_HALF      0x001c     /* `<io_intexpr> * 2' (A multiple of 2 who's half can fit into 16 unsigned bits) */

#define OPERAND_CLASS_NONE             0x0080     /* `none' */
#define OPERAND_CLASS_FOREACH          0x0081     /* `foreach' */
#define OPERAND_CLASS_EXCEPT           0x0082     /* `except' */
#define OPERAND_CLASS_CATCH            0x0083     /* `catch' */
#define OPERAND_CLASS_FINALLY          0x0084     /* `finally' */
#define OPERAND_CLASS_THIS             0x0085     /* `this' */
#define OPERAND_CLASS_THIS_MODULE      0x0086     /* `this_module' */
#define OPERAND_CLASS_THIS_FUNCTION    0x0087     /* `this_function' */
#define OPERAND_CLASS_TRUE             0x0088     /* `true' */
#define OPERAND_CLASS_FALSE            0x0089     /* `false' */
#define OPERAND_CLASS_LIST             0x008a     /* `List' */
#define OPERAND_CLASS_TUPLE            0x008b     /* `Tuple' */
#define OPERAND_CLASS_HASHSET          0x008c     /* `HashSet' */
#define OPERAND_CLASS_DICT             0x008d     /* `Dict' */
#define OPERAND_CLASS_SEQUENCE         0x008e     /* `Dict' */
#define OPERAND_CLASS_INT              0x008f     /* `int' */
#define OPERAND_CLASS_BOOL             0x0090     /* `bool' */
#define OPERAND_CLASS_EQ               0x0091     /* `eq' */
#define OPERAND_CLASS_NE               0x0092     /* `ne' */
#define OPERAND_CLASS_LO               0x0093     /* `lo' */
#define OPERAND_CLASS_LE               0x0094     /* `le' */
#define OPERAND_CLASS_GR               0x0095     /* `gr' */
#define OPERAND_CLASS_GE               0x0096     /* `ge' */
#define OPERAND_CLASS_SO               0x0097     /* `so' */
#define OPERAND_CLASS_DO               0x0098     /* `do' */
#define OPERAND_CLASS_BREAK            0x0099     /* `break' */
#define OPERAND_CLASS_MIN              0x009a     /* `min' */
#define OPERAND_CLASS_MAX              0x009b     /* `max' */
#define OPERAND_CLASS_SUM              0x009c     /* `sum' */
#define OPERAND_CLASS_ANY              0x009d     /* `any' */
#define OPERAND_CLASS_ALL              0x009e     /* `all' */
#define OPERAND_CLASS_SP               0x009f     /* `sp' */
#define OPERAND_CLASS_NL               0x00a0     /* `nl' */
#define OPERAND_CLASS_MOVE             0x00a1     /* `move' */
#define OPERAND_CLASS_DEFAULT          0x00a2     /* `default' */
#define OPERAND_CLASS_VARARGS          0x00a3     /* `varargs' */
#define OPERAND_CLASS_VARKWDS          0x00a4     /* `varkwds' */
#define OPERAND_CLASS_UNBOUND          0x00a5     /* `unbound' */
#define OPERAND_CLASS_LOCK             0x00a6     /* `lock' */
	uint16_t                           io_class;  /* Operand class (One of `OPERAND_CLASS_*'). */
	union {
		uint16_t                       io_symid;  /* Symbol id. */
		struct {
			uint16_t                   io_symid;  /* Symbol id. */
			uint16_t                   io_modid;  /* Module id. */
		}                              io_extern; /* `OPERAND_CLASS_EXTERN' */
		struct asm_intexpr             io_intexpr;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define io_symid   _dee_aunion.io_symid
#define io_extern  _dee_aunion.io_extern
#define io_intexpr _dee_aunion.io_intexpr
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

/* Print a human-readable representation of `self' to `printer' */
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL
asm_invoke_operand_print(struct asm_invoke_operand *__restrict self,
                         struct ascii_printer *__restrict printer);

#define OPERAND_CLASS_POP_DOTS       (OPERAND_CLASS_FDOTSFLAG | OPERAND_CLASS_POP) /* `pop...' */
#define OPERAND_CLASS_SPPOP          (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_POP) /* `#pop' */
#define OPERAND_CLASS_SPADDIMM_EQ_N2 (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_DISP_EQ_N2) /* `#SP - 2' */
#define OPERAND_CLASS_SPADDIMM_EQ_N1 (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_DISP_EQ_N1) /* `#SP - 1' */
#define OPERAND_CLASS_SPADDIMM_EQ_0  (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_DISP_EQ_0) /* `#SP + 0' */
#define OPERAND_CLASS_SPADDIMM_EQ_1  (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_DISP_EQ_1) /* `#SP + 1' */
#define OPERAND_CLASS_SPADDIMM_EQ_2  (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_DISP_EQ_2) /* `#SP + 2' */
#define OPERAND_CLASS_SPADDIMM8      (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_DISP8) /* `#SP + <io_intexpr>' */
#define OPERAND_CLASS_SPADDSIMM8     (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_SDISP8) /* `#SP + <io_intexpr>' */
#define OPERAND_CLASS_SPSUBIMM_EQ_N2 (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_DISP_EQ_N2) /* `#SP + 2' */
#define OPERAND_CLASS_SPSUBIMM_EQ_N1 (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_DISP_EQ_N1) /* `#SP + 1' */
#define OPERAND_CLASS_SPSUBIMM_EQ_0  (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_DISP_EQ_0) /* `#SP - 0' */
#define OPERAND_CLASS_SPSUBIMM_EQ_1  (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_DISP_EQ_1) /* `#SP - 1' */
#define OPERAND_CLASS_SPSUBIMM_EQ_2  (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_DISP_EQ_2) /* `#SP - 2' */
#define OPERAND_CLASS_SPSUBIMM8      (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_DISP8) /* `#SP - <io_intexpr>' */
#define OPERAND_CLASS_SPSUBSIMM8     (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_SDISP8) /* `#SP - <io_intexpr>' */
#define OPERAND_CLASS_SPIMM8         (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP8) /* `#<io_intexpr>' (An absolute address on the stack, or a number of stack entries by which to adjust) */
#define OPERAND_CLASS_SPSIMM8        (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_SDISP8) /* `#<io_intexpr>' (An absolute address on the stack, or a number of stack entries by which to adjust) */
#define OPERAND_CLASS_BRSPIMM8       (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACKETFLAG | OPERAND_CLASS_DISP8) /* `[#<io_intexpr>]' (Used by `ASM_CALL_SEQ') */
#define OPERAND_CLASS_BRSPSIMM8      (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACKETFLAG | OPERAND_CLASS_SDISP8) /* `[#<io_intexpr>]' (Used by `ASM_CALL_SEQ') */
#define OPERAND_CLASS_BCSPIMM8       (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACEFLAG | OPERAND_CLASS_DISP8) /* `{#<io_intexpr>}' (Used by `ASM_CALL_MAP') */
#define OPERAND_CLASS_BCSPSIMM8      (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACEFLAG | OPERAND_CLASS_SDISP8) /* `{#<io_intexpr>}' (Used by `ASM_CALL_MAP') */
#define OPERAND_CLASS_SPADDIMM16     (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_DISP16) /* `#SP + <io_intexpr>' */
#define OPERAND_CLASS_SPADDSIMM16    (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPADD | OPERAND_CLASS_SDISP16) /* `#SP + <io_intexpr>' */
#define OPERAND_CLASS_SPSUBIMM16     (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_DISP16) /* `#SP - <io_intexpr>' */
#define OPERAND_CLASS_SPSUBSIMM16    (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_SDISP16) /* `#SP - <io_intexpr>' */
#define OPERAND_CLASS_SPIMM16        (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP16) /* `#<io_intexpr>' (An absolute address on the stack, or a number of stack entries by which to adjust) */
#define OPERAND_CLASS_SPSIMM16       (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_SDISP16) /* `#<io_intexpr>' (An absolute address on the stack, or a number of stack entries by which to adjust) */
#define OPERAND_CLASS_BRSPIMM16      (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACKETFLAG | OPERAND_CLASS_DISP16) /* `[#<io_intexpr>]' (Used by `ASM_CALL_SEQ') */
#define OPERAND_CLASS_BRSPSIMM16     (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACKETFLAG | OPERAND_CLASS_SDISP16) /* `[#<io_intexpr>]' (Used by `ASM_CALL_SEQ') */
#define OPERAND_CLASS_BCSPIMM16      (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACEFLAG | OPERAND_CLASS_DISP16) /* `{#<io_intexpr>}' (Used by `ASM_CALL_MAP') */
#define OPERAND_CLASS_BCSPSIMM16     (OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_FBRACEFLAG | OPERAND_CLASS_SDISP16) /* `{#<io_intexpr>}' (Used by `ASM_CALL_MAP') */
#define OPERAND_CLASS_SIMM8          (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_SDISP8)
#define OPERAND_CLASS_SIMM16         (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_SDISP16)
#define OPERAND_CLASS_SIMM32         (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_SDISP32)
#define OPERAND_CLASS_IMM8           (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP8)
#define OPERAND_CLASS_IMM16          (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP16)
#define OPERAND_CLASS_IMM32          (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP32)
#define OPERAND_CLASS_IMM_EQ_2       (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP_EQ_2)
#define OPERAND_CLASS_IMM_EQ_1       (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP_EQ_1)
#define OPERAND_CLASS_IMM_EQ_0       (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP_EQ_0)
#define OPERAND_CLASS_IMM_EQ_N1      (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP_EQ_N1)
#define OPERAND_CLASS_IMM_EQ_N2      (OPERAND_CLASS_FIMMVAL | OPERAND_CLASS_DISP_EQ_N2)


struct asm_invocation {
#define INVOKE_FNORMAL    0x0000      /* Normal invocation flags. */
#define INVOKE_FPUSH      0x0010      /* Push the result (instruction is prefixed with `push')
	                                   * NOTE: The raw `push' instruction does not have this flag
	                                   *       set, as otherwise that would mean that `push' was
	                                   *       prefixed by another `push'. */
#define INVOKE_FPREFIX    0x0020      /* The instruction is prefixed by a symbol. */
	uint16_t           ai_flags;      /* Invocation flags (Set of `INVOKE_F*') */
	uint8_t            ai_opcount;    /* [<= ASM_MAX_INSTRUCTION_OPERANDS] The total number of operands. */
	uint8_t            ai_prefix;     /* [valid_if(INVOKE_FPREFIX)] The type of prefix (One of `ASM_*'; aka. the prefixed instruction id without F0 prefix). */
	uint16_t           ai_prefix_id1; /* [valid_if(INVOKE_FPREFIX)] The first prefix id. */
	uint16_t           ai_prefix_id2; /* [valid_if(INVOKE_FPREFIX)] The second prefix id (Only used by `ASM_EXTERN') */
	struct asm_invoke_operand ai_ops[ASM_MAX_INSTRUCTION_OPERANDS]; /* [FOR([*], valid_if(* < ai_opcount))] Invocation operands. */
};

struct asm_mnemonic;

/* Print a human-readable representation of `self' to `printer' */
INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
asm_invocation_print(struct asm_invocation *__restrict self,
                     struct asm_mnemonic *__restrict instr,
                     struct ascii_printer *__restrict printer);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
asm_invocation_tostring(struct asm_invocation *__restrict self,
                        struct asm_mnemonic *__restrict instr);




#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(push, 1)
#endif /* __COMPILER_HAVE_PRAGMA_PACK */
struct ATTR_PACKED asm_overload_operand {
	uint16_t  aoo_class;  /* The operand class. - One of `OPERAND_CLASS_*' */
	int8_t    aoo_disp;   /* A disposition added to the operand before it is encoded. */
};
struct ATTR_PACKED asm_overload {
	uint16_t                     ao_instr;   /* The instruction id, encoded in big-endian.
	                                          * When the most significant 8 bits are clear, write as 8-bits.
	                                          * NOTE: When equal to ASM_DELOP, no instruction should be generated. */
#define ASM_OVERLOAD_FRELABS     0x0000      /* FLAG: Generate ABSolute address relocations for operands. NOTE: _MUST_ always be ZERO(0) */
#define ASM_OVERLOAD_FRELDSP     0x0001      /* FLAG: Generate DiSPositional address relocations for operands. */
#define ASM_OVERLOAD_FSTKABS     0x0002      /* FLAG: Generate ABSolute stack relocations for operands. */
#define ASM_OVERLOAD_FSTKDSP     0x0003      /* FLAG: Generate DiSPositional stack relocations for operands. */
#define ASM_OVERLOAD_FRELMSK     0x0003      /* MASK: The mask for the relocations mode. */
#define ASM_OVERLOAD_FREL_DSPBIT 0x0001      /* BIT: When set, the relocation is DiSPositional. */
#define ASM_OVERLOAD_FREL_STKBIT 0x0002      /* BIT: When set, the relocation is STacK-dependent. */
#define ASM_OVERLOAD_FNORMAL     0x0000      /* Normal overload flags. */
#define ASM_OVERLOAD_FPUSH       0x0010      /* The instruction must be prefixed by `push' */
#define ASM_OVERLOAD_FPREFIX     0x0020      /* The overload must be used with a prefix. */
#define ASM_OVERLOAD_F16BIT      0x0040      /* Same as `ASM_OVERLOAD_FF0', however no prefix is written. */
#define ASM_OVERLOAD_FF0         0x0080      /* When set, `ao_instr' may be prefixed by `0xf0' to
	                                          * double the bit-limits of io_symid-related operands. */
#define ASM_OVERLOAD_FF0_IMM     0x0100      /* An extension to `ASM_OVERLOAD_FF0': Immediate operands are also extended. */
#define ASM_OVERLOAD_FRET        0x0200      /* The overload is only available in non-yielding functions. */
#define ASM_OVERLOAD_FYLD        0x0400      /* The overload is only available in yielding functions. */
#define ASM_OVERLOAD_FCONSTIMM   0x0800      /* Immediate operands are encoded as constants. */
	uint16_t                     ao_flags;   /* Set of `ASM_OVERLOAD_F*' */
	uint8_t                      ao_opcount; /* The exact number of operands accepted by this overload. */
	struct asm_overload_operand  ao_ops[ASM_MAX_INSTRUCTION_OPERANDS];
	                                         /* Operands accepted by this overload. */
#if (ASM_MAX_INSTRUCTION_OPERANDS % 2) == 0
	uint8_t                      ao_pad;     /* Ensure 2-byte alignment. */
#endif /* (ASM_MAX_INSTRUCTION_OPERANDS % 2) == 0 */
};

#define ASM_MNEMONIC_MAXNAME 14
struct ATTR_PACKED asm_mnemonic {
	char                                         am_name[ASM_MNEMONIC_MAXNAME]; /* Name of this instruction */
	uint16_t                                     am_num_overloads; /* [!0] The amount of overloads (NOTE: ZERO(0) is used to identify the sentinel) */
	COMPILER_FLEXIBLE_ARRAY(struct asm_overload, am_overloads);    /* [am_num_overloads] The individual overloads of this mnemonic. */
};
#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif /* __COMPILER_HAVE_PRAGMA_PACK */

/* Returns a pointer to the next mnemonic, following after `x' */
#define ASM_MNEMONIC_NEXT(x) \
	((struct asm_mnemonic *)((x)->am_overloads + (x)->am_num_overloads))
#define ASM_MNEMONIC_FOREACH(x)                      \
	for ((x) = (struct asm_mnemonic *)asm_mnemonics; \
	     (x)->am_num_overloads;                      \
	     (x) = ASM_MNEMONIC_NEXT(x))


#ifndef GUARD_DEEMON_COMPILER_ASM_USERDB_C
/* List of all known instruction mnemonics. */
INTDEF __BYTE_TYPE__ asm_mnemonics[];
/* The total number of mnemonics before the sentinel is encountered. */
INTDEF size_t const asm_mnemonics_count;
/* The total size (in bytes) of the mnemonic database, excluding the sentinel. */
INTDEF size_t const asm_mnemonics_size;
#endif /* !GUARD_DEEMON_COMPILER_ASM_USERDB_C */


/* Lookup an instruction mnemonic by name.
 * NOTE: The second variation will automatically attempt to
 *       cache the mnemonic's descriptor within the keyword.
 *       However if this is not possible, or if the cache
 *       slot is already used by something different, it will
 *       always perform a string lookup on the mnemonic. */
INTDEF WUNUSED NONNULL((1)) struct asm_mnemonic *DCALL
asm_mnemonic_lookup_str(char const *__restrict name);
INTDEF WUNUSED NONNULL((1)) struct asm_mnemonic *DCALL
asm_mnemonic_lookup(struct TPPKeyword *__restrict name);


INTDEF void DCALL userassembler_init(void);
INTDEF void DCALL userassembler_fini(void);
#ifdef __INTELLISENSE__
INTDEF struct user_assembler current_userasm;
#else /* __INTELLISENSE__ */
#define current_userasm      current_assembler.a_userasm
#endif /* !__INTELLISENSE__ */

/* Define a symbol from user-assembly.
 * WARNING: The caller must still check if the symbol had already been defined before. */
#define uasm_defsym(sym) (void)(asm_defsym(sym), current_userasm.ua_lasti = ASM_DELOP)

/* Parse (process tokens from TPP) user-define assembly. */
INTDEF WUNUSED int DFCALL uasm_parse(void);
INTDEF WUNUSED int DFCALL uasm_parse_instruction(void);
INTDEF WUNUSED int DFCALL uasm_parse_directive(void);
INTDEF WUNUSED struct TPPKeyword *DFCALL uasm_parse_symnam(void);
/* Parse an operand. NOTE: The caller is responsible for ZERO-initializing `result' beforehand. */
INTDEF WUNUSED NONNULL((1)) int DFCALL uasm_parse_operand(struct asm_invoke_operand *__restrict result);
/* Parse an integer expression as found in operands, following a `$' token.
 * @param: features: Set of `UASM_INTEXPR_F*' */
INTDEF WUNUSED NONNULL((1)) int DFCALL uasm_parse_intexpr(struct asm_intexpr *result, uint16_t features);
#define UASM_INTEXPR_FNORMAL 0x0000
#define UASM_INTEXPR_FHASSP  0x0001 /* Recognize case-insensitive `SP' as expanding to `current_assembler.a_stackcur' */

/* Parse and return a 16-bit unsigned integer.
 * @return: -1: An error was thrown.
 * @param: features: Set of `UASM_INTEXPR_F*' */
INTDEF WUNUSED int32_t DFCALL uasm_parse_imm16(uint16_t features);

/* Invoke a given `instr' using data from `invoc'.
 * NOTE: This function also sets the `CODE_FASSEMBLY' flag in the current base scope. */
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
uasm_invoke(struct asm_mnemonic *__restrict instr,
            struct asm_invocation *__restrict invoc);

/* Check if the given `name' refers to a
 * user-label that is active in the current context.
 * User-labels are formatted as `.L%Iu', where the integer
 * refers to an index in the `current_userasm.ua_labelv' vector.
 * If the given name does not start with `.L', or if the following
 * integer is greater than `current_userasm.ua_labelc', NULL is
 * returned, but no error is set. */
INTDEF WUNUSED NONNULL((1)) struct asm_sym *DFCALL
uasm_label_symbol(struct TPPKeyword *__restrict name);

/* Lookup a user-assembly symbol, given its name.
 * If the symbol doesn't already exist, it is automatically created. */
INTDEF WUNUSED NONNULL((1)) struct asm_sym *DFCALL
uasm_symbol(struct TPPKeyword *__restrict name);

/* Same as `uasm_symbol', but create/lookup symbols using forward/backward semantics. */
INTDEF WUNUSED NONNULL((1)) struct asm_sym *DFCALL
uasm_fbsymbol(struct TPPKeyword *__restrict name, bool return_back_symbol);

/* Return an F/B symbol suitable to be defined at some specific address. */
INTDEF WUNUSED NONNULL((1)) struct asm_sym *DFCALL
uasm_fbsymbol_def(struct TPPKeyword *__restrict name);

#endif /* !CONFIG_LANGUAGE_NO_ASM */



struct asm_symbol_ref {
	/* Symbol reference in the new symbol format are somewhat different:
	 *   - Instead of having its own symbol class, symbol references
	 *     are automatically created for any kind of symbol that must
	 *     be referenced, when that symbol appears within a context
	 *     where this is the case.
	 *   - This greatly simplifies the entire process of working with
	 *     symbols, as well as interchanging symbols from different
	 *     scopes, as well as re-linking symbols at times where that
	 *     symbol is already being used.
	 *   - During assembly, references are created on an as-needed basis,
	 *     before the invoker of the assembler is then responsible to
	 *     provide those references when constructing the containing
	 *     function.
	 * NOTES:
	 *   - In order to allow for recursion, as well as maintain a fast
	 *     way of remembering reference IDs inside symbols themself,
	 *    `s_refid' is used to save the ID of the referenced variable.
	 *     However, since the variable may again be referenced by the
	 *     caller of that particular piece of assembly, the original
	 *    `s_refid' and `s_flag' (which is used to carry the
	 *    `SYMBOL_FALLOCREF', indicating if `s_refid' is valid), are
	 *     preserved and must be restored when an assembler instance
	 *     finishes.
	 *   - For more information on when this is required, take a look
	 *     at the test `/util/test/compiler-recursive-references.dee'
	 */
	struct symbol         *sr_sym;         /* [1..1][const]
	                                        * [->s_flag & SYMBOL_FALLOCREF]
	                                        * [->s_refid == INDEXOF(self, :a_refv)]
	                                        * The symbol being referenced. */
	uint16_t               sr_orig_refid;  /* [const] The original value of `sr_sym->s_refid', before that symbol was re-referenced */
	uint16_t               sr_orig_flag;   /* [const] The original value of `sr_sym->s_flag', who's `SYMBOL_FALLOCREF' bit must be restored. */
};

struct asm_symbol_static {
	struct symbol         *ss_sym;  /* [0..1] The symbol descriptor associated with this state symbol. */
};

struct assembler {
	struct asm_sec         a_sect[SECTION_COUNT]; /* Assembly sections. */
	struct asm_sec        *a_curr;     /* [1..1][in(a_sect)] The current target section. */
	struct asm_sym_slist   a_syms;     /* [0..1][owned] Chain of assembly symbols allocated by the assembler. */
	struct asm_sym        *a_finsym;   /* [0..1] The address of the current, top finally handler. */
	uint8_t               *a_localuse; /* [0..((a_localc+7)/8)|ALLOC(a_locala)][if(!ASM_FREUSELOC, == NULL)][owned] Bitset of local variables that are currently in use. */
	DREF DeeObject       **a_constv;   /* [1..1][0..a_constc|ALLOC(a_consta)][owned] Vector of constant variables. */
	struct asm_symbol_static
	                      *a_staticv;  /* [1..1][0..a_staticc|ALLOC(a_statica)][owned] Vector of static variable initializers. */
	struct asm_symbol_ref *a_refv;     /* [1..1][0..a_refc|ALLOC(a_refa)][owned] Vector of symbol references used from the previous base scope.
	                                    * NOTE: These are the symbols _FROM_ the previous scope. - _NOT_ those ref-symbols from the current,
	                                    *       meaning that when creating the reference vector, these symbols should be pushed by the caller
	                                    *       directly (Preferrably using `asm_gpush_symbol()')! */
	struct symbol        **a_argrefv;  /* [1..1][0..a_argrefc|ALLOC(a_argrefa)][owned] Vector of argument references used from the previous base scope.
	                                    * Only used when assembling in `ASM_FARGREFS' mode (s.a. `asm_gcall_func' in `gencall.c') */
	struct asm_exc        *a_exceptv;  /* [0..a_exceptc|ALLOC(a_excepta)][owned] Assembly definitions for exception handlers. */
#define ASM_LOOPCTL_BRK    0           /* Loop-break symbol. */
#define ASM_LOOPCTL_CON    1           /* Loop-continue symbol. */
#define ASM_LOOPCTL_COUNT  2           /* Amount of loopctl symbols. */
	struct asm_sym        *a_loopctl[ASM_LOOPCTL_COUNT]; /* Loop control symbols. */
	DeeScopeObject        *a_scope;    /* [0..1] The scope of the last AST (Used for tracking stack-based variables) */
	struct ast_loc        *a_error;    /* [0..1] An AST location that is used as context for displaying assembler messages/warnings. */
#define ASM_ERR(...)       parser_erratrf(current_assembler.a_error, __VA_ARGS__)
#define ASM_WARN(...)      parser_warnatrf(current_assembler.a_error, __VA_ARGS__)
	struct ddi_assembler   a_ddi;      /* Deemon debug information assembler subsystem. */
	struct handler_frame  *a_handler;  /* [0..1][(!= NULL) == (a_handlerc != 0)] Chain of active exception handlers. */
#ifndef CONFIG_LANGUAGE_NO_ASM
	struct user_assembler  a_userasm;  /* User-assembly descriptor/environment. */
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	uint16_t               a_handlerc; /* [(!= 0) == (a_handler != NULL)] Amount of active exception handlers. */
	uint16_t               a_localc;   /* Amount of required local variables. */
	uint16_t               a_locala;   /* [if(!ASM_FREUSELOC, == 0)] Allocated size of the `a_localuse' bitset. */
	uint16_t               a_constc;   /* Amount of required constant variables. */
	uint16_t               a_consta;   /* Allocated amount of constant variables. */
	uint16_t               a_staticc;  /* Amount of required static variables. */
	uint16_t               a_statica;  /* Allocated amount of static variables. */
	uint16_t               a_refc;     /* Amount of required reference variables. */
	uint16_t               a_refa;     /* Allocated amount of required reference variables. */
	uint16_t               a_argrefc;  /* Amount of required reference-through-argument variables. */
	uint16_t               a_argrefa;  /* Allocated amount of required reference-through-argument variables. */
	uint16_t               a_exceptc;  /* Amount of required exception handlers. */
	uint16_t               a_excepta;  /* Allocated amount of exception handlers. */
	uint16_t               a_stackcur; /* Current stack depth. */
	uint16_t               a_stackmax; /* Greatest stack depth ever recorded. */
#define ASM_FINFLAG_NORMAL 0x0000      /* Normal finally-flags. */
#define ASM_FINFLAG_USED   0x0001      /* Set when `a_finsym' is being used. */
#define ASM_FINFLAG_NOLOOP 0x0002      /* Set when `a_finsym' is located outside of the current loop, and symbols from `a_loopctl'. */
	uint16_t               a_finflag;  /* Finally-block flags (Set of `ASM_FINFLAG_*'). */
#define ASM_FNORMAL        0x0000      /* Normal assembler operation. */
#define ASM_FBIGCODE       0x0001      /* Generate assembly for large code targets.
                                        * When this flag is set, the assembler will attempt to
                                        * generate code capable of handling a very large assembly.
                                        * In essence: in this mode, all jmp instructions are
                                        * encoded as `ASM32_JMP' opcodes, allowing for more than
                                        * a total of 2^16 bytes of text.
                                        * HINT: 32-bit conditional jumps are encoded as follows:
                                        * 8/16-bit:
                                        * >>    ...
                                        * >>    jt    foo
                                        * >>    ...
                                        * 32-bit:
                                        * >>    ...
                                        * >>    jf    1f
                                        * >>    jmp32 foo
                                        * >>1:  ...
                                        * Because of this, bigcode is only generated when the linker
                                        * failed because a jump target would have been truncated, in
                                        * which case assembly automatically starts over with this flag
                                        * set. */
#define ASM_FOPTIMIZE      0x0002      /* Optimize generated assembly and perform minor improvements
                                        * during generation, so long as they don't impact DDI. */
#define ASM_FOPTIMIZE_SIZE 0x0100      /* When the decision between smaller vs. faster code comes up, choose smaller code. */
#define ASM_FREUSELOC      0x0004      /* Allow local variables to be re-used. (May make manual debugging more difficult) */
#define ASM_FPEEPHOLE      0x0008      /* Perform peephole optimizations. */
#define ASM_FSTACKDISP     0x0010      /* The stack may get displaced between instruction at (seemingly) random intervals.
                                        * When this flag is set, much more optimal code can be generated for stack variables,
                                        * but peephole & minjmp optimizations are expected to do a lot of cleanup.
                                        * Essentially, this flag allows stack variables to be lazily initialized
                                        * and only be popped once their scope ends.
                                        * However, this means that before having been assigned, a stack variable
                                        * doesn't have a storage location associated with it, meaning that a lot
                                        * of (usually meaningless) code to adjust the stack has to be generated:
                                        * REMINDER: Uninitialized stack variables have a value of `none'
                                        *
                                        * >> for (__stack local x: [:10]) {
                                        * >>      print x;
                                        * >> }
                                        *
                                        * Without this flag set, the following code would be generated.
                                        * If you read this code, you will notice that stack variables are pre-allocated
                                        * and cleaned before their scope is left, while stack duplication instructions are
                                        * used every time they are interacted with, similar to how all other variables
                                        * are operated upon.
                                        * >>    adjstack #SP + 1   // Allocate stack memory for `x' (preinitializes to `none'; actually assembled as `push none')
                                        * >>    range    $0, $10
                                        * >>    iterself top
                                        * >>2:  foreach  top, 1f
                                        * >>    pop      #3   // Save the result of `foreach' in `x'
                                        * >>    dup      #2   // Load `x' for printing
                                        * >>    print    pop, nl // Print `x'
                                        * >>    jmp      2b
                                        * >>1:  adjstack #SP - 1   // Cleanup `x' (actually assembled as `pop')
                                        *
                                        * However when this flag _is_ set, no stack-space must be pre-allocated,
                                        * and the value that was meant to be assigned initially can simply be
                                        * kept around and be re-referenced every time the variable is used,
                                        * essentially meaning that unused stack variables never take up memory
                                        * at runtime, as well as stack variables that _were_ used never needing
                                        * to be assigned to (as their initially assigned value is always the
                                        * result pushed onto the stack by their instruction of origin)
                                        * Despite all of this, stack variables still remain consistent, with
                                        * their absolute stack-offset at runtime already known at compile-time.
                                        * >>    range    $0, $10
                                        * >>    iterself top
                                        * >>2:  foreach  top, 1f
                                        * >>    // Internal: Assign `top' as the stack location for `x'
                                        * >>    dup      #0      // Copy `x' for print
                                        * >>    print    pop, nl // Print `x'
                                        * >>    adjstack #SP - 1 // Adjust to fix the stack prior to jumping. (actually assembled as `pop')
                                        * >>    jmp      2b
                                        * >>1:
                                        * And after applying peephole optimizations, it will look like this:
                                        * >>    range    $0, $10
                                        * >>    iterself top
                                        * >>2:  foreach  top, 1f
                                        * >>    print    pop, nl // Print `x'
                                        * >>    jmp      2b
                                        * >>1: */
#define ASM_FNOASSERT      0x0020      /* Replace all assert statements with a compile-time constant `true'. */
#define ASM_FNODEC         0x0040      /* Do not create a `*.dec' file once the module has been compiled. */
#define ASM_FNOREUSECONST  0x0080      /* Do not re-use constants. */
#define ASM_FREDUCEREFS    0x0100      /* Try to minimize use of references when accessing class/instance members.
                                        * Enabling this isn't a good idea, as it slows down access to members inside
                                        * of classes, while only speeding up construction of said class.
                                        * Additionally, problems may ensue when the class's symbol is overwritten,
                                        * as member function may not be holding their own references to the class,
                                        * but be using global symbols instead. */
#define ASM_FARGREFS       0x0200      /* [INTERNAL] Make use of references-through-arguments */
#define ASM_FNODDIXDAT     0x4000      /* Do not generate extended DDI debug information. */
#define ASM_FNODDI         0x8000      /* Do not generate DDI debug information.
                                        * Setting this flag allows peephole optimizations to
                                        * be more effective than when this flag wasn't set. */
	uint16_t               a_flag;     /* Assembler operation flags (Set of `ASM_F*') */
};

#define ASM_SYMBOL_MAY_REFERENCE(x)               \
	((current_assembler.a_flag & ASM_FREDUCEREFS) \
	 ? SYMBOL_MUST_REFERENCE(x)                   \
	 : SYMBOL_MAY_REFERENCE(x))

/* Initialize/Finalize the given assembler. */
INTDEF void DCALL assembler_init(void);
INTDEF NONNULL((1, 2)) void DCALL
assembler_init_reuse(DeeCodeObject *__restrict code_obj,
                     instruction_t *__restrict text_end);
INTDEF void DCALL assembler_fini(void);

/* The active assembly context */
INTDEF struct assembler current_assembler;


/* Define a new DDI checkpoint at the current text position.
 * This function is a no-op when the `ASM_FNODDI' flag is set.
 * NOTE: This function also creates a new assembly symbol,
 *       meaning that when `ASM_FNODDI' isn't set, peephole
 *       optimization may suffer at the current location.
 * NOTE: If `ast->ast_ddi.l_file' is `NULL', this function is a no-op.
 * NOTE: This function should be called _before_ the code
 *       that it is meant to be describing, and will continue
 *       to describe code until the next checkpoint. */
#ifdef NDEBUG
INTDEF WUNUSED NONNULL((1)) int DCALL asm_putddi(struct ast *__restrict self);
#else /* NDEBUG */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_putddi_dbg(struct ast *__restrict self, char const *file, int line);
#define asm_putddi(self) asm_putddi_dbg(self, __FILE__, __LINE__)
#endif /* !NDEBUG */

/* Generate symbol binding information as part of the next DDI checkpoint. */
INTDEF WUNUSED int DCALL asm_putddi_bind(uint16_t ddi_class, uint16_t index, struct TPPKeyword *name);
#define asm_putddi_sbind(index, name) asm_putddi_bind(DDI_BINDING_CLASS_STACK, index, name)
#define asm_putddi_lbind(index, name) asm_putddi_bind(DDI_BINDING_CLASS_LOCAL, index, name)
#define asm_putddi_sunbind(index)     asm_putddi_sbind(index, NULL)
#define asm_putddi_lunbind(index)     asm_putddi_lbind(index, NULL)


/* Remove all `ASM_DELOP' instructions and adjust relocations accordingly.
 * NOTE: The caller is responsible to merge sections first (i.e. call `asm_mergetext()')
 * @return: true:  At least one `ASM_DELOP' was removed.
 * @return: false: No `ASM_DELOP' were removed, or `ASM_FOPTIMIZE' was not set. */
INTDEF bool DCALL asm_rmdelop(void);

/* Go through all relocations and check if they can fit into smaller opcodes:
 * >> All of deemon's jump instructions offer at least an 8-bit and a 16-bit
 *    version. Unless `ASM_FBIGCODE' is set (in which case 32-bit `jmp' is
 *    always used), the assembler will always generate 16-bit jumps by default.
 *    This works good, but once all symbols have been defined, and while we
 *    still have all the relocations at hand, we can optimize this by checking
 *    each of them for being able to fit into the 8-bit version.
 *    `jmp <Sdisp16>'           -- `11 RR RR'
 *    When `Sdisp16' can fit into a single, signed byte, replace with the following:
 *    `jmp <Sdisp8>; ASM_DELOP' -- `10 RR 8F'
 * >> The intended use of this function is as follows:
 *    >> // Keep shrinking `jmp' and deleting DELOP instructions while stuff happens
 *    >> while (asm_minjmp() || asm_rmdelop() || asm_peephole());
 * NOTE: The caller is responsible to merge sections first (i.e. call `asm_mergetext()')
 * @return: true:  At least one `jmp' instruction could be compressed into a lower class.
 * @return: false: No `jmp' instructions could be truncated, or `ASM_FOPTIMIZE' was not set. */
INTDEF bool DCALL asm_minjmp(void);

/* Perform peephole optimizations.
 * @return:  1: Optimizations have been performed.
 * @return:  0: Nothing was optimized.
 * @return: -1: An error occurred. */
INTDEF WUNUSED int DCALL asm_peephole(void);

/* Delete unused symbols, thus improving future peephole optimizations. */
INTDEF bool DCALL asm_delunusedsyms(void);

/* Merge all of the assembler's section into the first
 * section, adjusting relocations in the process. */
INTDEF WUNUSED int DCALL asm_mergetext(void);

/* Merge constant and static variables into only
 * constants, resolving all `R_DMN_STATIC16' relocations.
 * NOTE: This function must be called after `asm_mergetext()' */
INTDEF WUNUSED int DCALL asm_mergestatic(void);

INTDEF WUNUSED int DCALL asm_check_user_labels_defined(void);
INTDEF WUNUSED int DCALL asm_applyconstrel(void);

/* Resolve jump relocations.
 * This function should be called as the last step before
 * code is generated and will do the final resolution of
 * jump targets, also ensuring that offsets are not too big.
 * @return:  1: At least one relocation target would have been truncated.
 *              -> The caller should start over reassemble
 *                 everything with the `ASM_FBIGCODE' flag set.
 * @return:  0: Text was successfully linked.
 * @return: -1: An error occurred. */
INTDEF WUNUSED int DCALL asm_linktext(void);

/* A sub-set of `asm_linktext' that only links relocations concerning stack addresses. */
INTDEF WUNUSED int DCALL asm_linkstack(void);

/* Allocate and return a new assembly symbol. */
#ifdef NDEBUG
INTDEF WUNUSED struct asm_sym *(DCALL asm_newsym)(void);
#else /* NDEBUG */
INTDEF WUNUSED struct asm_sym *(DCALL asm_newsym_dbg)(char const *file, int line);
#define asm_newsym() asm_newsym_dbg(__FILE__, __LINE__)
#endif /* !NDEBUG */

/* Allocate and return a new assembly exception handler.
 * WARNING: The returned pointer only remains valid
 *          until the next time this function is called!
 * NOTE: The handler is created with max-priority. */
INTDEF WUNUSED struct asm_exc *(DCALL asm_newexc)(void);

/* Same as `asm_newexc()', but the handler is inserted
 * at index `priority' within the handler vector.
 * In other words: the greatest possible priority at any point
 *                 in time is `current_assembler.a_exceptc',
 *                 while the lowest is always `0'
 *                 And remember that every time a new handler
 *                 is allocated, `current_assembler.a_exceptc'
 *                 will increase by ONE(1). */
INTDEF WUNUSED struct asm_exc *(DCALL asm_newexc_at)(uint16_t priority);

/* Define the given symbol `self' at the current text address. */
INTDEF NONNULL((1)) void (DCALL asm_defsym)(struct asm_sym *__restrict self);

/* Generate a finalized code object from written assembly.
 * NOTE: The caller is required to ensure that the assembler is ready
 *       for this, as well as that `asm_mergetext()' has been called! */
INTDEF WUNUSED DREF DeeCodeObject *(DCALL asm_gencode)(void);
INTDEF WUNUSED struct except_handler *(DCALL asm_pack_exceptv)(void);


/* Set the current text section.
 * @param: section_id: One of `SECTION_*' */
#define asm_setcur(section_id) \
	(void)(current_assembler.a_curr = &current_assembler.a_sect[section_id])
#define asm_getcur() \
	((uint16_t)(current_assembler.a_curr - current_assembler.a_sect))

/* Adjust the current stack depth. */
#define asm_addsp(n)                                                                  \
	(void)((current_assembler.a_stackmax >= (current_assembler.a_stackcur += (n))) || \
	       (current_assembler.a_stackmax = current_assembler.a_stackcur, 1))
#define asm_subsp(n) \
	(void)(Dee_ASSERT(current_assembler.a_stackcur >= (n)), current_assembler.a_stackcur -= (n))
#define asm_incsp()                                                            \
	(void)((current_assembler.a_stackmax >= ++current_assembler.a_stackcur) || \
	       (current_assembler.a_stackmax = current_assembler.a_stackcur, 1))
#define asm_decsp()    (void)(Dee_ASSERT(current_assembler.a_stackcur), --current_assembler.a_stackcur)
#define asm_dicsp()    (void)(Dee_ASSERT(current_assembler.a_stackcur >= 1)) /* asm_decsp(), asm_incsp() */
#define asm_ddcsp()    (void)(Dee_ASSERT(current_assembler.a_stackcur >= 2), current_assembler.a_stackcur -= 2) /* asm_decsp(), asm_decsp() */
#define asm_dddcsp()   (void)(Dee_ASSERT(current_assembler.a_stackcur >= 3), current_assembler.a_stackcur -= 3) /* asm_decsp(), asm_decsp(), asm_decsp() */
#define asm_ddddcsp()  (void)(Dee_ASSERT(current_assembler.a_stackcur >= 4), current_assembler.a_stackcur -= 4) /* asm_decsp(), asm_decsp(), asm_decsp(), asm_decsp() */
#define asm_ddicsp()   (void)(Dee_ASSERT(current_assembler.a_stackcur >= 2), --current_assembler.a_stackcur) /* asm_decsp(), asm_decsp(), asm_incsp() */
#define asm_dddicsp()  (void)(Dee_ASSERT(current_assembler.a_stackcur >= 3), current_assembler.a_stackcur -= 2) /* asm_decsp(), asm_decsp(), asm_decsp(), asm_incsp() */
#define asm_ddddicsp() (void)(Dee_ASSERT(current_assembler.a_stackcur >= 4), current_assembler.a_stackcur -= 3) /* asm_decsp(), asm_decsp(), asm_decsp(), asm_decsp(), asm_incsp() */
#define asm_ddiicsp()  (void)(Dee_ASSERT(current_assembler.a_stackcur >= 2)) /* asm_decsp(), asm_decsp(), asm_incsp(), asm_incsp() */
#define asm_dNiNcsp(n) (void)(Dee_ASSERT(current_assembler.a_stackcur >= (n))) /* [asm_decsp()]*n, [asm_incsp()]*n */
#define asm_diicsp()   (void)(Dee_ASSERT(current_assembler.a_stackcur >= 1), asm_incsp()) /* asm_decsp(), asm_incsp(), asm_incsp() */

/* Allocate a new, uninitialized relocation within the current text section. */
INTDEF WUNUSED struct asm_rel *DFCALL asm_allocrel(void);

/* Place a new relocation at the current text position. */
INTDEF WUNUSED int DCALL asm_putrel(uint16_t type, struct asm_sym *sym, uint16_t value);

/* Return the current instruction pointer address. */
#define asm_ip()                                       \
	(code_addr_t)(current_assembler.a_curr->sec_iter - \
	              current_assembler.a_curr->sec_begin)
#define asm_secip(section_id)                                     \
	(code_addr_t)(current_assembler.a_sect[section_id].sec_iter - \
	              current_assembler.a_sect[section_id].sec_begin)

/* Write data to the current text section.
 * @return: -1|NULL: An error occurred. */
INTDEF WUNUSED instruction_t *DFCALL asm_alloc(size_t n_bytes);
INTDEF WUNUSED int DCALL asm_put(instruction_t instr);
INTDEF WUNUSED int DCALL asm_put16(uint16_t instr); /* 16-bit, big-endian encoded instruction, or 8-bit when top 8 bits are clear. */
INTDEF WUNUSED int DCALL asm_putimm8(instruction_t instr, uint8_t imm8);
INTDEF WUNUSED int DCALL asm_putimm8_8(instruction_t instr, uint8_t imm8_1, uint8_t imm8_2);
INTDEF WUNUSED int DCALL asm_putimm8_8_8(instruction_t instr, uint8_t imm8_1, uint8_t imm8_2, uint8_t imm8_3);
INTDEF WUNUSED int DCALL asm_putimm8_16(instruction_t instr, uint8_t imm8_1, uint16_t imm16_2);
INTDEF WUNUSED int DCALL asm_putimm16(instruction_t instr, uint16_t imm16);
INTDEF WUNUSED int DCALL asm_putimm16_8(instruction_t instr, uint16_t imm16_1, uint8_t imm8_2);
INTDEF WUNUSED int DCALL asm_putimm16_16(instruction_t instr, uint16_t imm16_1, uint16_t imm16_2);
INTDEF WUNUSED int DCALL asm_putimm16_16_8(instruction_t instr, uint16_t imm16_1, uint16_t imm16_2, uint8_t imm8_3);
INTDEF WUNUSED int DCALL asm_putimm16_16_16(instruction_t instr, uint16_t imm16_1, uint16_t imm16_2, uint16_t imm16_3);
INTDEF WUNUSED int DCALL asm_putimm16_8_16(instruction_t instr, uint16_t imm16_1, uint8_t imm8_2, uint16_t imm16_3);
INTDEF WUNUSED int DCALL asm_putimm32(instruction_t instr, uint32_t imm32);
/* Encode an instruction followed by a 16-bit static variable id (including the required relocation). */
INTDEF WUNUSED int DCALL asm_putsid16(uint16_t instr, uint16_t sid);



#ifdef __INTELLISENSE__
/* Given a host-endian data word, encode it as little endian and write it to the current text position. */
INTDEF WUNUSED int DCALL asm_put_data8(uint8_t data);
#else /* __INTELLISENSE__ */
#define asm_put_data8(data) asm_put(data)
#endif /* !__INTELLISENSE__ */
INTDEF WUNUSED int DCALL asm_put_data16(uint16_t data);
INTDEF WUNUSED int DCALL asm_put_data32(uint32_t data);
INTDEF WUNUSED int DCALL asm_put_data64(uint64_t data);

/* Push the absolute address of an assembly symbol + offset onto the stack. */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gpush_abs(struct asm_sym *__restrict sym); /* PC */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gpush_stk(struct asm_sym *__restrict sym); /* SP */

/* Add the given `constvalue' to the set of
 * constant variables and return its 16-bit index.
 * If another constant identical to the given is already apart
 * of the constant variable, its index is returned instead.
 * NOTE: The return type is 32-bits to allow for -1 to be returned on error. */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_newconst(DeeObject *__restrict constvalue);
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_newconst_string(char const *__restrict str, size_t len);

/* Check if a given constant value can safely appear in constant variable slots.
 * If this is not the case, `asm_gpush_constexpr' should be used to automatically
 * generate code capable of pushing the given value onto the stack. */
INTDEF WUNUSED NONNULL((1)) bool DCALL asm_allowconst(DeeObject *__restrict constvalue);

/* Create a new static variable ID and return it.
 * @param: sym: The symbol with which to associated the static variable, or NULL if anonymous.
 * NOTE: The caller must encode the returned index alongside a `R_DMN_STATIC16' relocation.
 *       This can easily be achieved using the `asm_putsid16()' function. */
INTDEF WUNUSED int32_t DCALL asm_newstatic(struct symbol *sym);

/* Allocate a new local variable index. */
INTDEF WUNUSED int32_t DCALL asm_newlocal(void);
INTDEF WUNUSED int32_t DCALL asm_newlocal_noreuse(void);

/* Mark a given local variable as no longer being in use.
 * Once this is done, later calls to `asm_newlocal()' are allowed
 * to re-use `index' when the `ASM_FREUSELOC' assembler flag is set.
 * NOTE: This function does not generate any code.
 *       The caller is responsible for generating code if this is wanted. */
INTDEF void DCALL asm_dellocal(uint16_t index);

/* Make sure that `mod' is being imported by the current
 * root-scope, if necessary adding it as a new dependency. */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_newmodule(struct module_object *__restrict mod);

/* Ensure that a given symbol has been allocated and return its index.
 * NOTE: These function automatically save the symbol index within the symbol itself,
 *       thus ensuring that successive calls to these functions will quickly yield the same value. */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_gsymid(struct symbol *__restrict sym);   /* `SYM_CLASS_VAR:SYM_FVAR_GLOBAL' */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_lsymid(struct symbol *__restrict sym);   /* `SYM_CLASS_VAR:SYM_FVAR_LOCAL' */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_ssymid(struct symbol *__restrict sym);   /* `SYM_CLASS_VAR:SYM_FVAR_STATIC' */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_esymid(struct symbol *__restrict sym);   /* `SYMBOL_TYPE_EXTERN' (Returns the module index in the current root-scope's import vector)
                                                                                        *  NOTE: This function will dereference external aliases. */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_msymid(struct symbol *__restrict sym);   /* `SYMBOL_TYPE_MODULE' */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_rsymid(struct symbol *__restrict sym);   /* Reference a symbol for a lower base-scope. */
INTDEF WUNUSED NONNULL((1)) int32_t DCALL asm_asymid_r(struct symbol *__restrict sym); /* For use in `ASM_FARGREFS' mode: Return an argument ID for a referenced symbol. */

/* Search the export table of the builtin `deemon' module for `constval'.
 * If the object could be found, return an anonymous `SYMBOL_TYPE_EXTERN'
 * symbol (allocated as part of `current_rootscope') that is bound to
 * that specific export.
 * @return: * :                              A `SYMBOL_TYPE_EXTERN' symbol, bound to `constval'
 * @return: ASM_BIND_DEEMON_EXPORT_NOTFOUND: `constval' wasn't found in `deemon's exports
 * @return: NULL:                            An error occurred. */
INTDEF WUNUSED NONNULL((1)) struct symbol *DCALL
asm_bind_deemon_export(DeeObject *__restrict constval);
#define ASM_BIND_DEEMON_EXPORT_NOTFOUND ((struct symbol *)(uintptr_t)-1)

/* These versions emit read-before-write warnings if the symbol hadn't been allocated, yet. */
INTDEF WUNUSED NONNULL((1, 2)) int32_t DCALL
asm_gsymid_for_read(struct symbol *__restrict sym, /* `SYM_CLASS_VAR:SYM_FVAR_GLOBAL' */
                    struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1, 2)) int32_t DCALL
asm_lsymid_for_read(struct symbol *__restrict sym, /* `SYM_CLASS_VAR:SYM_FVAR_LOCAL' */
                    struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1, 2)) int32_t DCALL
asm_ssymid_for_read(struct symbol *__restrict sym, /* `SYM_CLASS_VAR:SYM_FVAR_STATIC' */
                    struct ast *__restrict warn_ast);


#ifndef UINT8_MAX
#define UINT8_MAX 0xff
#endif /* !UINT8_MAX */
#ifndef INT8_MAX
#define INT8_MAX  127
#endif /* !INT8_MAX */

#define asm_put816(op, imm816)            \
	((imm816) <= UINT8_MAX                \
	 ? asm_putimm8(op, (uint8_t)(imm816)) \
	 : (asm_put(ASM_EXTENDED1) || asm_putimm16(op, imm816)))
#define asm_put816_8(op, imm816, imm8)            \
	((imm816) <= UINT8_MAX                        \
	 ? asm_putimm8_8(op, (uint8_t)(imm816), imm8) \
	 : (asm_put(ASM_EXTENDED1) || asm_putimm16_8(op, imm816, imm8)))
#define asm_put8_816(op, imm8, imm816)            \
	((imm816) <= UINT8_MAX                        \
	 ? asm_putimm8_8(op, imm8, (uint8_t)(imm816)) \
	 : (asm_put(ASM_EXTENDED1) || asm_putimm8_16(op, imm8, imm816)))
#define asm_put816_8_816(op, imm816a, imm8, imm816b)                     \
	((imm816a) <= UINT8_MAX && (imm816b) <= UINT8_MAX                    \
	 ? asm_putimm8_8_8(op, (uint8_t)(imm816a), imm8, (uint8_t)(imm816b)) \
	 : (asm_put(ASM_EXTENDED1) || asm_putimm16_8_16(op, imm816a, imm8, imm816b)))
#define asm_put881616(op, imma, immb)                      \
	(((imma) <= UINT8_MAX && (immb) <= UINT8_MAX)          \
	 ? asm_putimm8_8(op, (uint8_t)(imma), (uint8_t)(immb)) \
	 : (asm_put(ASM_EXTENDED1) || asm_putimm16_16(op, imma, immb)))
#define asm_put888161616(op, imma, immb, immc)                                \
	(((imma) <= UINT8_MAX && (immb) <= UINT8_MAX && (immc) <= UINT8_MAX)      \
	 ? asm_putimm8_8_8(op, (uint8_t)(imma), (uint8_t)(immb), (uint8_t)(immc)) \
	 : (asm_put(ASM_EXTENDED1) || asm_putimm16_16_16(op, imma, immb, immc)))
#define asm_put881616_8(op, imma, immb, immc)                      \
	(((imma) <= UINT8_MAX && (immb) <= UINT8_MAX)                  \
	 ? asm_putimm8_8_8(op, (uint8_t)(imma), (uint8_t)(immb), immc) \
	 : (asm_put(ASM_EXTENDED1) || asm_putimm16_16_8(op, imma, immb, immc)))
#define asm_put1616(op8, op16, imm816)                                              \
	((imm816) <= UINT8_MAX                                                          \
	 ? (asm_put(((op8)&0xff00) >> 8) || asm_putimm8((op8)&0xff, (uint8_t)(imm816))) \
	 : (asm_put(((op16)&0xff00) >> 8) || asm_putimm16((op16)&0xff, imm816)))
#define asm_put16161616(op8, op16, imm816a, imm816b)                                                       \
	((imm816a) <= UINT8_MAX && (imm816b) <= UINT8_MAX                                                      \
	 ? (asm_put(((op8)&0xff00) >> 8) || asm_putimm8_8((op8)&0xff, (uint8_t)(imm816a), (uint8_t)(imm816b))) \
	 : (asm_put(((op16)&0xff00) >> 8) || asm_putimm16_16((op16)&0xff, imm816a, imm816b)))
#define asm_put16161616_8(op8, op16, imm816a, imm816b, imm8c)                                                       \
	((imm816a) <= UINT8_MAX && (imm816b) <= UINT8_MAX                                                               \
	 ? (asm_put(((op8)&0xff00) >> 8) || asm_putimm8_8_8((op8)&0xff, (uint8_t)(imm816a), (uint8_t)(imm816b), imm8c)) \
	 : (asm_put(((op16)&0xff00) >> 8) || asm_putimm16_16_8((op16)&0xff, imm816a, imm816b, imm8c)))


/* Generate a jump to the given target, automatically choosing
 * the proper mechanism and creating the associated relocation.
 * The caller is required to ensure that the given `target' is defined at some point.
 * @param: instr: One of `ASM_JMP', `ASM_JT', `ASM_JF' or `ASM_FOREACH' */
INTDEF WUNUSED NONNULL((2)) int DCALL
asm_gjmp(instruction_t instr, struct asm_sym *__restrict target);
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL
asm_gjcc(struct ast *cond, instruction_t instr,
         struct asm_sym *__restrict target,
         struct ast *ddi_ast);

/* Similar to `asm_gjmp(ASM_JMP)', but generate code to adjust adjust the stack beforehand, as well
 * as code to adjust for potential exception handlers, also creating a `R_DMN_DELHAND' relocation. */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gjmps(struct asm_sym *__restrict target);

/* Generate code to adjust for the exception handler level at a given symbol.
 * This function is internally called by `asm_gjmps()'. */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gadjhand(struct asm_sym *__restrict target);

/* Generate code to unwind all active exception handlers.
 * This must be called before `return'-ing from within a catch/finally handler. */
INTDEF WUNUSED int DCALL asm_gunwind(void);

#define asm_private_gfunction_ii(code_cid, n_refs)                                    \
	((code_cid) <= UINT8_MAX                                                          \
	 ? ((n_refs) <= UINT8_MAX                                                         \
	    ? asm_putimm8_8(ASM_FUNCTION_C, (uint8_t)(code_cid), (uint8_t)(n_refs))       \
	    : asm_putimm8_16(ASM_FUNCTION_C_16, (uint8_t)(code_cid), (uint16_t)(n_refs))) \
	 : (asm_put(ASM_EXTENDED1) ||                                                     \
	    ((n_refs) <= UINT8_MAX                                                        \
	     ? asm_putimm16_8(ASM_FUNCTION_C, (uint16_t)(code_cid), (uint8_t)(n_refs))    \
	     : asm_putimm16_16(ASM_FUNCTION_C_16, (uint16_t)(code_cid), (uint16_t)(n_refs)))))


/* Prefix instructions. */
#define asm_pstack(sto)               (asm_put816(ASM_STACK, sto))
#define asm_pstatic(sid)              (asm_putsid16(ASM16_STATIC, sid))
#define asm_pextern(mid, gid)         (asm_put881616(ASM_EXTERN, mid, gid))
#define asm_pglobal(gid)              (asm_put816(ASM_GLOBAL, gid))
#define asm_plocal(lid)               (asm_put816(ASM_LOCAL, lid))

/* Macros for generating instructions. */
#define asm_gret_none()               (asm_put(ASM_RET_NONE))
#define asm_gret()                    (asm_decsp(), asm_put(ASM_RET))
#define asm_gret_p()                  (asm_put(ASM_RET))
#define asm_gsetret()                 (asm_decsp(), asm_put(ASM_SETRET))
#define asm_gsetret_p()               (asm_put(ASM_SETRET))
#define asm_gyield()                  (asm_decsp(), asm_put(ASM_YIELD))
#define asm_gyield_p()                (asm_put(ASM_YIELD))
#define asm_gyieldall()               (asm_decsp(), asm_put(ASM_YIELDALL))
#define asm_gyieldall_p()             (asm_put(ASM_YIELDALL))
#define asm_gthrow()                  (asm_decsp(), asm_put(ASM_THROW))
#define asm_gthrow_p()                (asm_put(ASM_THROW))
#define asm_grethrow()                (asm_put(ASM_RETHROW))
#define asm_gendcatch()               (asm_put(ASM_ENDCATCH))
#define asm_gendcatch_n(n)            ((n) ? (asm_put((ASM_ENDCATCH_N&0xff00) >> 8) || asm_putimm8(ASM_ENDCATCH_N&0xff, (uint8_t)((n) - 1))) : asm_gendcatch())
#define asm_gendfinally()             (asm_put(ASM_ENDFINALLY))
#define asm_gendfinally_except()      (asm_put16(ASM_ENDFINALLY_EXCEPT))
#define asm_gendfinally_n(n)          ((n) ? (asm_put((ASM_ENDFINALLY_N&0xff00) >> 8) || asm_putimm8(ASM_ENDFINALLY_N&0xff, (uint8_t)((n) - 1))) : asm_gendfinally())
#define asm_gpush_bnd_arg(aid)        (asm_incsp(), asm_put816(ASM_PUSH_BND_ARG, aid))
#define asm_gpush_bnd_extern(mid, gid) (asm_incsp(), asm_put881616(ASM_PUSH_BND_EXTERN, mid, gid))
#define asm_gpush_bnd_static(sid)     (asm_incsp(), asm_putsid16(ASM16_PUSH_BND_STATIC, sid))
#define asm_gpush_bnd_global(gid)     (asm_incsp(), asm_put816(ASM_PUSH_BND_GLOBAL, gid))
#define asm_gpush_bnd_local(lid)      (asm_incsp(), asm_put816(ASM_PUSH_BND_LOCAL, lid))
#define asm_gjf(target)               (asm_gjmp(ASM_JF, target))
#define asm_gjt(target)               (asm_gjmp(ASM_JT, target))
#define asm_gforeach(target)          (asm_diicsp(), asm_gjmp(ASM_FOREACH, target))
#define asm_gcall(n)                  (asm_subsp((n) + 1), asm_incsp(), asm_putimm8(ASM_CALL, n))
#define asm_gcall_seq(n)              (asm_subsp((n) + 1), asm_incsp(), (asm_put((ASM_CALL_SEQ & 0xff00) >> 8) || asm_putimm8(ASM_CALL_SEQ & 0xff, n)))
#define asm_gcall_map(n)              (asm_subsp(((n)*2) + 1), asm_incsp(), (asm_put((ASM_CALL_MAP & 0xff00) >> 8) || asm_putimm8(ASM_CALL_MAP & 0xff, n)))
#define asm_gcallattr_const_seq(cid, n) (asm_subsp((n) + 1), asm_incsp(), asm_put816_8(ASM_CALLATTR_C_SEQ, cid, n))
#define asm_gcallattr_const_map(cid, n) (asm_subsp(((n)*2) + 1), asm_incsp(), asm_put816_8(ASM_CALLATTR_C_MAP, cid, n))
#define asm_gcall_tuple()             (asm_ddicsp(), asm_put(ASM_CALL_TUPLE))
#define asm_gthiscall_tuple()         (asm_dddicsp(), asm_put16(ASM_THISCALL_TUPLE))
#define asm_gjmp_pop()                (asm_decsp(), asm_put(ASM_JMP_POP))
#define asm_gjmp_pop_pop()            (asm_ddcsp(), asm_put16(ASM_JMP_POP_POP))
#define asm_gdel_static(sid)          (asm_putsid16(ASM16_DEL_STATIC, sid))
#define asm_gdel_global(gid)          (asm_put816(ASM_DEL_GLOBAL, gid))
#define asm_gdel_local(lid)           (asm_put816(ASM_DEL_LOCAL, lid))
#define asm_gswap()                   (asm_ddiicsp(), asm_put(ASM_SWAP))
#define _asm_glrot(n)                 (asm_dNiNcsp(n), (n) <= UINT8_MAX ? asm_putimm8(ASM_LROT, (uint8_t)(n)) : (asm_put((ASM16_LROT & 0xff00) >> 8) || asm_putimm16(ASM16_LROT & 0xff, (uint16_t)(n))))
#define _asm_grrot(n)                 (asm_dNiNcsp(n), (n) <= UINT8_MAX ? asm_putimm8(ASM_RROT, (uint8_t)(n)) : (asm_put((ASM16_RROT & 0xff00) >> 8) || asm_putimm16(ASM16_RROT & 0xff, (uint16_t)(n))))
#define asm_gdup()                    (asm_diicsp(), asm_put(ASM_DUP))
#define asm_gdup_p()                  (asm_dicsp(), asm_put(ASM_DUP))
#define asm_gdup_n(n)                 (asm_dNiNcsp(n), asm_incsp(), (n) <= UINT8_MAX ? asm_putimm8(ASM_DUP_N, (uint8_t)(n)) : (asm_put((ASM16_DUP_N & 0xff00) >> 8) || asm_putimm16(ASM16_DUP_N & 0xff, (uint16_t)(n))))
#define asm_gdup_n_p(n)               (asm_dNiNcsp(n), (n) <= UINT8_MAX ? asm_putimm8(ASM_DUP_N, (uint8_t)(n)) : (asm_put((ASM16_DUP_N & 0xff00) >> 8) || asm_putimm16(ASM16_DUP_N & 0xff, (uint16_t)(n))))
#define asm_gpop()                    (asm_decsp(), asm_put(ASM_POP))
#define asm_gpop_p()                  (asm_put(ASM_POP))
#define asm_gpop_n(n)                 (asm_dNiNcsp((n) + 1), asm_decsp(), (n) <= UINT8_MAX ? asm_putimm8(ASM_POP_N, (uint8_t)(n)) : (asm_put((ASM16_POP_N & 0xff00) >> 8) || asm_putimm16(ASM16_POP_N & 0xff, (uint16_t)(n))))
#define asm_gpop_n_p(n)               (asm_dNiNcsp((n) + 1), (n) <= UINT8_MAX ? asm_putimm8(ASM_POP_N, (uint8_t)(n)) : (asm_put((ASM16_POP_N & 0xff00) >> 8) || asm_putimm16(ASM16_POP_N & 0xff, (uint16_t)(n))))
#define _asm_gadjstack(diff)          (current_assembler.a_stackcur += (diff), ((diff) >= INT8_MIN && (diff) <= INT8_MAX) ? asm_putimm8(ASM_ADJSTACK, (uint8_t)(int8_t)(diff)) : (asm_put((ASM16_ADJSTACK & 0xff00) >> 8) || asm_putimm16((ASM16_ADJSTACK & 0xff), (uint16_t)(int16_t)(diff))))
#define asm_gpop_static(sid)          (asm_decsp(), asm_putsid16(ASM16_POP_STATIC, sid))
#define asm_gpop_static_p(sid)        (asm_putsid16(ASM16_POP_STATIC, sid))
#define asm_gpop_extern(mid, gid)     (asm_decsp(), asm_put881616(ASM_POP_EXTERN, mid, gid))
#define asm_gpop_extern_p(mid, gid)   (asm_put881616(ASM_POP_EXTERN, mid, gid))
#define asm_gpop_global(gid)          (asm_decsp(), asm_put816(ASM_POP_GLOBAL, gid))
#define asm_gpop_global_p(gid)        (asm_put816(ASM_POP_GLOBAL, gid))
#define asm_gpop_local(lid)           (asm_decsp(), asm_put816(ASM_POP_LOCAL, lid))
#define asm_gpop_local_p(lid)         (asm_put816(ASM_POP_LOCAL, lid))
#define asm_gpush_bool(tt)            (asm_incsp(), asm_put16((tt) ? ASM_PUSH_TRUE : ASM_PUSH_FALSE))
#define asm_gpush_bool_p(tt)          (asm_put16((tt) ? ASM_PUSH_TRUE : ASM_PUSH_FALSE))
#define asm_gpush_true()              (asm_incsp(), asm_put16(ASM_PUSH_TRUE))
#define asm_gpush_true_p()            (asm_put16(ASM_PUSH_TRUE))
#define asm_gpush_false()             (asm_incsp(), asm_put16(ASM_PUSH_FALSE))
#define asm_gpush_false_p()           (asm_put16(ASM_PUSH_FALSE))
#define asm_gpush_this_function()     (asm_incsp(), asm_put16(ASM_PUSH_THIS_FUNCTION))
#define asm_gpush_this_function_p()   (asm_put16(ASM_PUSH_THIS_FUNCTION))
#define asm_gpush_except()            (asm_incsp(), asm_put16(ASM_PUSH_EXCEPT))
#define asm_gpush_except_p()          (asm_put16(ASM_PUSH_EXCEPT))
#define asm_gpush_this()              (asm_incsp(), asm_put16(ASM_PUSH_THIS))
#define asm_gpush_this_p()            (asm_put16(ASM_PUSH_THIS))
#define asm_gpush_this_module()       (asm_incsp(), asm_put16(ASM_PUSH_THIS_MODULE))
#define asm_gpush_this_module_p()     (asm_put16(ASM_PUSH_THIS_MODULE))
#define asm_gpush_ref(rid)            (asm_incsp(), asm_put816(ASM_PUSH_REF, rid))
#define asm_gpush_ref_p(rid)          (asm_put816(ASM_PUSH_REF, rid))
#define asm_gpush_arg(aid)            (asm_incsp(), asm_put816(ASM_PUSH_ARG, aid))
#define asm_gpush_arg_p(aid)          (asm_put816(ASM_PUSH_ARG, aid))
#define asm_gpush_varargs()           (asm_incsp(), asm_put(ASM_PUSH_VARARGS))
#define asm_gpush_varargs_p()         (asm_put(ASM_PUSH_VARARGS))
#define asm_gpush_varkwds()           (asm_incsp(), asm_put(ASM_PUSH_VARKWDS))
#define asm_gpush_varkwds_p()         (asm_put(ASM_PUSH_VARKWDS))
#define asm_gpush_const(cid)          (asm_incsp(), asm_put816(ASM_PUSH_CONST, cid))
#define asm_gpush_const_p(cid)        (asm_put816(ASM_PUSH_CONST, cid))
#define asm_gpush_const8(cid)         (asm_incsp(), asm_putimm8(ASM_PUSH_CONST, cid))
#define asm_gpush_const8_p(cid)       (asm_putimm8(ASM_PUSH_CONST, cid))
#define asm_gpush_static(sid)         (asm_incsp(), asm_putsid16(ASM16_PUSH_STATIC, sid))
#define asm_gpush_static_p(sid)       (asm_putsid16(ASM16_PUSH_STATIC, sid))
#define asm_gpush_extern(mid, gid)    (asm_incsp(), asm_put881616(ASM_PUSH_EXTERN, mid, gid))
#define asm_gpush_extern_p(mid, gid)  (asm_put881616(ASM_PUSH_EXTERN, mid, gid))
#define asm_gpush_global(gid)         (asm_incsp(), asm_put816(ASM_PUSH_GLOBAL, gid))
#define asm_gpush_global_p(gid)       (asm_put816(ASM_PUSH_GLOBAL, gid))
#define asm_gpush_global8(gid)        (asm_incsp(), asm_putimm8(ASM_PUSH_GLOBAL, gid))
#define asm_gpush_global8_p(gid)      (asm_putimm8(ASM_PUSH_GLOBAL, gid))
#define asm_gpush_local(lid)          (asm_incsp(), asm_put816(ASM_PUSH_LOCAL, lid))
#define asm_gpush_local_p(lid)        (asm_put816(ASM_PUSH_LOCAL, lid))
#define asm_gpush_local8(lid)         (asm_incsp(), asm_putimm8(ASM_PUSH_LOCAL, lid))
#define asm_gpush_local8_p(lid)       (asm_putimm8(ASM_PUSH_LOCAL, lid))
#define asm_gpack_tuple(n)            (asm_subsp(n), asm_incsp(), asm_put816(ASM_PACK_TUPLE, n))
#define asm_gpack_list(n)             (asm_subsp(n), asm_incsp(), asm_put816(ASM_PACK_LIST, n))
#define asm_gpack_hashset(n)          (asm_subsp(n), asm_incsp(), asm_put((ASM_PACK_HASHSET & 0xff00) >> 8) || ((n) <= UINT8_MAX ? asm_putimm8((ASM_PACK_HASHSET & 0xff), (uint8_t)(n)) : asm_putimm16((ASM16_PACK_HASHSET & 0xff), (uint16_t)(n))))
#define asm_gpack_dict(n)             (asm_subsp((n)*2), asm_incsp(), asm_put((ASM_PACK_DICT & 0xff00) >> 8) || ((n) <= UINT8_MAX ? asm_putimm8((ASM_PACK_DICT & 0xff), (uint8_t)(n)) : asm_putimm16((ASM16_PACK_DICT & 0xff), (uint16_t)(n))))
#define asm_gpack_one()               (asm_dicsp(), asm_put(ASM_PACK_ONE))
#define asm_gunpack(n)                (asm_decsp(), asm_addsp(n), asm_put816(ASM_UNPACK, n))
#define asm_gunpack_p(n)              (asm_addsp(n), asm_put816(ASM_UNPACK, n))
#define asm_gvarargs_unpack(n)        (asm_addsp(n), asm_put((ASM_VARARGS_UNPACK & 0xff00) >> 8) || asm_putimm8(ASM_VARARGS_UNPACK & 0xff, (uint8_t)(n)))
#define asm_gvarargs_getitem()        (asm_dicsp(), asm_put16(ASM_VARARGS_GETITEM))
#define asm_gvarargs_getitem_i(index) (asm_incsp(), asm_put((ASM_VARARGS_GETITEM_I & 0xff00) >> 8) || asm_putimm8(ASM_VARARGS_GETITEM_I & 0xff, (uint8_t)(index)))
#define asm_gcast_tuple()             (asm_dicsp(), asm_put(ASM_CAST_TUPLE))
#define asm_gcast_list()              (asm_dicsp(), asm_put(ASM_CAST_LIST))
#define asm_gcast_hashset()           (asm_dicsp(), asm_put((ASM_CAST_HASHSET & 0xff00) >> 8) || asm_put(ASM_CAST_HASHSET & 0xff))
#define asm_gcast_dict()              (asm_dicsp(), asm_put((ASM_CAST_DICT & 0xff00) >> 8) || asm_put(ASM_CAST_DICT & 0xff))
#define asm_gcast_int()               (asm_dicsp(), asm_put(ASM_CAST_INT))
#define asm_gcast_bool()              (asm_dicsp(), asm_put(ASM_BOOL))
#define asm_gcast_varkwds()           (asm_dicsp(), asm_put(ASM_CAST_VARKWDS))
#define asm_gpush_none()              (asm_incsp(), asm_put(ASM_PUSH_NONE))
#define asm_gpush_none_p()            (asm_put(ASM_PUSH_NONE))
#define asm_gpush_module(mid)         (asm_incsp(), asm_put816(ASM_PUSH_MODULE, mid))
#define asm_gpush_module_p(mid)       (asm_put816(ASM_PUSH_MODULE, mid))
#define asm_gconcat()                 (asm_ddicsp(), asm_put(ASM_CONCAT))
#define asm_gextend(n)                                                 \
	(asm_subsp((n) + 1), asm_incsp(),                                  \
	 (n) <= UINT8_MAX ? (asm_putimm8(ASM_EXTEND, (uint8_t)(n)))        \
	                  : (asm_put((ASM16_PACK_TUPLE & 0xff00) >> 8) ||  \
	                     asm_putimm16((ASM16_PACK_TUPLE & 0xff), n) || \
	                     asm_put(ASM_CONCAT)))
#define asm_gtypeof()                 (asm_dicsp(), asm_put(ASM_TYPEOF))
#define asm_gclassof()                (asm_dicsp(), asm_put(ASM_CLASSOF))
#define asm_gsuperof()                (asm_dicsp(), asm_put(ASM_SUPEROF))
#define asm_gsuper()                  (asm_ddicsp(), asm_put(ASM_SUPER))
#define asm_gsuper_this_r(rid)        (asm_incsp(), asm_put816(ASM_SUPER_THIS_R, rid))
#define asm_ginstanceof()             (asm_ddicsp(), asm_put(ASM_INSTANCEOF))
#define asm_gimplements()             (asm_ddicsp(), asm_put(ASM_IMPLEMENTS))
#define asm_gisnone()                 (asm_dicsp(), asm_put(ASM_ISNONE))
#define asm_gstr()                    (asm_dicsp(), asm_put(ASM_STR))
#define asm_grepr()                   (asm_dicsp(), asm_put(ASM_REPR))
#define asm_gbool(invert)             (asm_dicsp(), asm_put(ASM_BOOL ^ (instruction_t)!!(invert)))
#define asm_gbool_varkwds()           (asm_incsp(), asm_put16(ASM_PUSH_VARKWDS_NE))
#define asm_gassign()                 (asm_ddcsp(), asm_put(ASM_ASSIGN))
#define asm_gmove_assign()            (asm_ddcsp(), asm_put(ASM_MOVE_ASSIGN))
#define asm_gcopy()                   (asm_dicsp(), asm_put(ASM_COPY))
#define asm_gdeepcopy()               (asm_dicsp(), asm_put(ASM_DEEPCOPY))
#define asm_gcmp_eq()                 (asm_ddicsp(), asm_put(ASM_CMP_EQ))
#define asm_gcmp_ne()                 (asm_ddicsp(), asm_put(ASM_CMP_NE))
#define asm_gcmp_lo()                 (asm_ddicsp(), asm_put(ASM_CMP_LO))
#define asm_gcmp_le()                 (asm_ddicsp(), asm_put(ASM_CMP_LE))
#define asm_gcmp_gr()                 (asm_ddicsp(), asm_put(ASM_CMP_GR))
#define asm_gcmp_ge()                 (asm_ddicsp(), asm_put(ASM_CMP_GE))
#define asm_gcmp_eq_varargs_sz(sz)    (asm_incsp(), asm_put((ASM_VARARGS_CMP_EQ_SZ & 0xff00) >> 8) || asm_putimm8(ASM_VARARGS_CMP_EQ_SZ & 0xff, sz))
#define asm_gcmp_gr_varargs_sz(sz)    (asm_incsp(), asm_put((ASM_VARARGS_CMP_GR_SZ & 0xff00) >> 8) || asm_putimm8(ASM_VARARGS_CMP_GR_SZ & 0xff, sz))
#define asm_gfunction_ii(code_cid, n_refs) (asm_subsp(n_refs), asm_incsp(), asm_private_gfunction_ii(code_cid, n_refs))
#define asm_gfunction_ii_prefixed(code_cid, n_refs) (asm_subsp(n_refs), asm_private_gfunction_ii(code_cid, n_refs))
#define asm_ginv()                    (asm_dicsp(), asm_put(ASM_INV))
#define asm_gpos()                    (asm_dicsp(), asm_put(ASM_POS))
#define asm_gneg()                    (asm_dicsp(), asm_put(ASM_NEG))
#define asm_gadd()                    (asm_ddicsp(), asm_put(ASM_ADD))
#define asm_gsub()                    (asm_ddicsp(), asm_put(ASM_SUB))
#define asm_gmul()                    (asm_ddicsp(), asm_put(ASM_MUL))
#define asm_gdiv()                    (asm_ddicsp(), asm_put(ASM_DIV))
#define asm_gmod()                    (asm_ddicsp(), asm_put(ASM_MOD))
#define asm_gshl()                    (asm_ddicsp(), asm_put(ASM_SHL))
#define asm_gshr()                    (asm_ddicsp(), asm_put(ASM_SHR))
#define asm_gand()                    (asm_ddicsp(), asm_put(ASM_AND))
#define asm_gor()                     (asm_ddicsp(), asm_put(ASM_OR))
#define asm_gxor()                    (asm_ddicsp(), asm_put(ASM_XOR))
#define asm_gpow()                    (asm_ddicsp(), asm_put(ASM_POW))
#define asm_gadd_simm8(val)           (asm_dicsp(), asm_putimm8(ASM_ADD_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gadd_imm32(val)           (asm_dicsp(), ((uint32_t)(val) < INT8_MAX ? asm_putimm8(ASM_ADD_SIMM8, (uint8_t)(uint32_t)(val)) : asm_putimm32(ASM_ADD_IMM32, (uint32_t)(val))))
#define asm_gsub_simm8(val)           (asm_dicsp(), asm_putimm8(ASM_SUB_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gsub_imm32(val)           (asm_dicsp(), ((uint32_t)(val) < INT8_MAX ? asm_putimm8(ASM_SUB_SIMM8, (uint8_t)(uint32_t)(val)) : asm_putimm32(ASM_SUB_IMM32, (uint32_t)(val))))
#define asm_gmul_simm8(val)           (asm_dicsp(), asm_putimm8(ASM_MUL_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gdiv_simm8(val)           (asm_dicsp(), asm_putimm8(ASM_DIV_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gmod_simm8(val)           (asm_dicsp(), asm_putimm8(ASM_MOD_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gand_imm32(val)           (asm_dicsp(), asm_putimm32(ASM_AND_IMM32, (uint32_t)(val)))
#define asm_gor_imm32(val)            (asm_dicsp(), asm_putimm32(ASM_OR_IMM32, (uint32_t)(val)))
#define asm_gxor_imm32(val)           (asm_dicsp(), asm_putimm32(ASM_XOR_IMM32, (uint32_t)(val)))
#define asm_gshl_imm8(val)            (asm_dicsp(), asm_putimm8(ASM_SHL_IMM8, (uint8_t)(val)))
#define asm_gshr_imm8(val)            (asm_dicsp(), asm_putimm8(ASM_SHR_IMM8, (uint8_t)(val)))
#define asm_gadd_inplace_simm8(val)   (asm_putimm8(ASM_ADD_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gadd_inplace_imm32(val)   (((uint32_t)(val) < INT8_MAX ? asm_putimm8(ASM_ADD_SIMM8, (uint8_t)(uint32_t)(val)) : asm_putimm32(ASM_ADD_IMM32, (uint32_t)(val))))
#define asm_gsub_inplace_simm8(val)   (asm_putimm8(ASM_SUB_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gsub_inplace_imm32(val)   (((uint32_t)(val) < INT8_MAX ? asm_putimm8(ASM_SUB_SIMM8, (uint8_t)(uint32_t)(val)) : asm_putimm32(ASM_SUB_IMM32, (uint32_t)(val))))
#define asm_gmul_inplace_simm8(val)   (asm_putimm8(ASM_MUL_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gdiv_inplace_simm8(val)   (asm_putimm8(ASM_DIV_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gmod_inplace_simm8(val)   (asm_putimm8(ASM_MOD_SIMM8, (uint8_t)(int8_t)(val)))
#define asm_gand_inplace_imm32(val)   (asm_putimm32(ASM_AND_IMM32, (uint32_t)(val)))
#define asm_gor_inplace_imm32(val)    (asm_putimm32(ASM_OR_IMM32, (uint32_t)(val)))
#define asm_gxor_inplace_imm32(val)   (asm_putimm32(ASM_XOR_IMM32, (uint32_t)(val)))
#define asm_gshl_inplace_imm8(val)    (asm_putimm8(ASM_SHL_IMM8, (uint8_t)(val)))
#define asm_gshr_inplace_imm8(val)    (asm_putimm8(ASM_SHR_IMM8, (uint8_t)(val)))
#define asm_gnoop()                   (asm_put(ASM_NOP))
#define asm_gprint()                  (asm_decsp(), asm_put(ASM_PRINT))
#define asm_gprint_sp()               (asm_decsp(), asm_put(ASM_PRINT_SP))
#define asm_gprint_nl()               (asm_decsp(), asm_put(ASM_PRINT_NL))
#define asm_gprintnl()                (asm_put(ASM_PRINTNL))
#define asm_gprintall()               (asm_decsp(), asm_put(ASM_PRINTALL))
#define asm_gprintall_sp()            (asm_decsp(), asm_put(ASM_PRINTALL_SP))
#define asm_gprintall_nl()            (asm_decsp(), asm_put(ASM_PRINTALL_NL))
#define asm_gfprint()                 (asm_ddicsp(), asm_put(ASM_FPRINT))
#define asm_gfprint_sp()              (asm_ddicsp(), asm_put(ASM_FPRINT_SP))
#define asm_gfprint_nl()              (asm_ddicsp(), asm_put(ASM_FPRINT_NL))
#define asm_gfprintall()              (asm_ddicsp(), asm_put(ASM_FPRINTALL))
#define asm_gfprintall_sp()           (asm_ddicsp(), asm_put(ASM_FPRINTALL_SP))
#define asm_gfprintall_nl()           (asm_ddicsp(), asm_put(ASM_FPRINTALL_NL))
#define asm_gprint_const(cid)         (asm_put816(ASM_PRINT_C, cid))
#define asm_gprint_const_sp(cid)      (asm_put816(ASM_PRINT_C_SP, cid))
#define asm_gprint_const_nl(cid)      (asm_put816(ASM_PRINT_C_NL, cid))
#define asm_gfprint_const(cid)        (asm_dicsp(), asm_put816(ASM_FPRINT_C, cid))
#define asm_gfprint_const_sp(cid)     (asm_dicsp(), asm_put816(ASM_FPRINT_C_SP, cid))
#define asm_gfprint_const_nl(cid)     (asm_dicsp(), asm_put816(ASM_FPRINT_C_NL, cid))
#define asm_ggetsize()                (asm_dicsp(), asm_put(ASM_GETSIZE))
#define asm_ggetsize_varargs()        (asm_incsp(), asm_put16(ASM_VARARGS_GETSIZE))
#define asm_gcontains()               (asm_ddicsp(), asm_put(ASM_CONTAINS))
#define asm_gcontains_const(cid)      (asm_dicsp(), asm_put816(ASM_CONTAINS_C, cid))
#define asm_ggetitem()                (asm_ddicsp(), asm_put(ASM_GETITEM))
#define asm_ggetitem_index(i16)       (asm_dicsp(), asm_putimm16(ASM_GETITEM_I, i16))
#define asm_ggetitem_const(cid)       (asm_dicsp(), asm_put816(ASM_GETITEM_C, cid))
#define asm_gsetitem()                (asm_dddcsp(), asm_put(ASM_SETITEM))
#define asm_gsetitem_index(i16)       (asm_ddcsp(), asm_putimm16(ASM_SETITEM_I, i16))
#define asm_gsetitem_const(cid)       (asm_ddcsp(), asm_put816(ASM_SETITEM_C, cid))
#define asm_gdelitem()                (asm_ddcsp(), asm_put(ASM_DELITEM))
#define asm_ggetrange()               (asm_dddicsp(), asm_put(ASM_GETRANGE))
#define asm_ggetrange_pn()            (asm_ddicsp(), asm_put(ASM_GETRANGE_PN))
#define asm_ggetrange_np()            (asm_ddicsp(), asm_put(ASM_GETRANGE_NP))
#define asm_ggetrange_pi(Si16)        (asm_ddicsp(), asm_putimm16(ASM_GETRANGE_PI, (uint16_t)(int16_t)(Si16)))
#define asm_ggetrange_ip(Si16)        (asm_ddicsp(), asm_putimm16(ASM_GETRANGE_IP, (uint16_t)(int16_t)(Si16)))
#define asm_ggetrange_ni(Si16)        (asm_dicsp(), asm_putimm16(ASM_GETRANGE_NI, (uint16_t)(int16_t)(Si16)))
#define asm_ggetrange_in(Si16)        (asm_dicsp(), asm_putimm16(ASM_GETRANGE_IN, (uint16_t)(int16_t)(Si16)))
#define asm_ggetrange_ii(Sb16, Se16)  (asm_dicsp(), asm_putimm16_16(ASM_GETRANGE_II, (uint16_t)(int16_t)(Sb16), (uint16_t)(int16_t)(Se16)))
#define asm_gdelrange()               (asm_dddcsp(), asm_put(ASM_DELRANGE))
#define asm_gsetrange()               (asm_ddddcsp(), asm_put(ASM_SETRANGE))
#define asm_gsetrange_pn()            (asm_dddcsp(), asm_put(ASM_SETRANGE_PN))
#define asm_gsetrange_np()            (asm_dddcsp(), asm_put(ASM_SETRANGE_NP))
#define asm_gsetrange_pi(Si16)        (asm_dddcsp(), asm_putimm16(ASM_SETRANGE_PI, (uint16_t)(int16_t)(Si16)))
#define asm_gsetrange_ip(Si16)        (asm_dddcsp(), asm_putimm16(ASM_SETRANGE_IP, (uint16_t)(int16_t)(Si16)))
#define asm_gsetrange_ni(Si16)        (asm_ddcsp(), asm_putimm16(ASM_SETRANGE_NI, (uint16_t)(int16_t)(Si16)))
#define asm_gsetrange_in(Si16)        (asm_ddcsp(), asm_putimm16(ASM_SETRANGE_IN, (uint16_t)(int16_t)(Si16)))
#define asm_gsetrange_ii(Sb16, Se16)  (asm_ddcsp(), asm_putimm16_16(ASM_SETRANGE_II, (uint16_t)(int16_t)(Sb16), (uint16_t)(int16_t)(Se16)))
#define asm_gbreakpoint()             (asm_put(ASM_BREAKPOINT))
#define asm_giterself()               (asm_dicsp(), asm_put(ASM_ITERSELF))
#define asm_giternext()               (asm_dicsp(), asm_put16(ASM_ITERNEXT))
#define asm_gcallattr(n)              (asm_subsp((n)+2), asm_incsp(), asm_putimm8(ASM_CALLATTR, n))
#define asm_gcallattr_tuple()         (asm_dddicsp(), asm_put(ASM_CALLATTR_TUPLE))
#define asm_gcallattr_const(cid, n)   (asm_subsp((n) + 1), asm_incsp(), asm_put816_8(ASM_CALLATTR_C, cid, n))
#define asm_gcallattr_const_tuple(cid) (asm_ddicsp(), asm_put816(ASM_CALLATTR_C_TUPLE, cid))
#define asm_gcallattr_this_const(cid, n) (asm_subsp(n), asm_incsp(), asm_put816_8(ASM_CALLATTR_THIS_C, cid, n))
#define asm_gcallattr_this_const_tuple(cid) (asm_dicsp(), asm_put816(ASM_CALLATTR_THIS_C_TUPLE, cid))
#define asm_gcall_extern(mid, gid, n) (asm_subsp(n), asm_incsp(), asm_put881616_8(ASM_CALL_EXTERN, mid, gid, n))
#define asm_gcall_global(gid, n)      (asm_subsp(n), asm_incsp(), asm_put816_8(ASM_CALL_GLOBAL, gid, n))
#define asm_gcall_local(lid, n)       (asm_subsp(n), asm_incsp(), asm_put816_8(ASM_CALL_LOCAL, lid, n))
#define asm_ggetattr()                (asm_ddicsp(), asm_put(ASM_GETATTR))
#define asm_gdelattr()                (asm_ddcsp(), asm_put(ASM_DELATTR))
#define asm_gsetattr()                (asm_dddcsp(), asm_put(ASM_SETATTR))
#define asm_gboundattr()              (asm_ddicsp(), asm_put(ASM_BOUNDATTR))
#define asm_gbounditem()              (asm_ddicsp(), asm_put16(ASM_BOUNDITEM))
#define asm_gsameobj()                (asm_ddicsp(), asm_put16(ASM_CMP_SO))
#define asm_gdiffobj()                (asm_ddicsp(), asm_put16(ASM_CMP_DO))
#define asm_ggetattr_const(cid)       (asm_dicsp(), asm_put816(ASM_GETATTR_C, cid))
#define asm_gdelattr_const(cid)       (asm_decsp(), asm_put816(ASM_DELATTR_C, cid))
#define asm_gsetattr_const(cid)       (asm_ddcsp(), asm_put816(ASM_SETATTR_C, cid))
#define asm_ggetattr_this_const(cid)  (asm_incsp(), asm_put816(ASM_GETATTR_THIS_C, cid))
#define asm_gdelattr_this_const(cid)  (asm_put816(ASM_DELATTR_THIS_C, cid))
#define asm_gsetattr_this_const(cid)  (asm_decsp(), asm_put816(ASM_SETATTR_THIS_C, cid))
#define asm_ggetmember(iid)           (asm_ddicsp(), asm_put((ASM_GETMEMBER & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_GETMEMBER & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_GETMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_gdelmember(iid)           (asm_ddcsp(), asm_put((ASM_DELMEMBER & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_DELMEMBER & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_DELMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_gsetmember(iid)           (asm_dddcsp(), asm_put((ASM_SETMEMBER & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_SETMEMBER & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_SETMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_gboundmember(iid)         (asm_ddicsp(), asm_put((ASM_BOUNDMEMBER & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_BOUNDMEMBER & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_BOUNDMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_ggetmember_this(iid)      (asm_dicsp(), asm_put((ASM_GETMEMBER_THIS & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_GETMEMBER_THIS & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_GETMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_gdelmember_this(iid)      (asm_decsp(), asm_put((ASM_DELMEMBER_THIS & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_DELMEMBER_THIS & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_DELMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_gsetmember_this(iid)      (asm_ddcsp(), asm_put((ASM_SETMEMBER_THIS & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_SETMEMBER_THIS & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_SETMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_gboundmember_this(iid)    (asm_dicsp(), asm_put((ASM_BOUNDMEMBER_THIS & 0xff00) >> 8) || ((iid) <= UINT8_MAX ? asm_putimm8(ASM_BOUNDMEMBER_THIS & 0xff, (uint8_t)(iid)) : asm_putimm16(ASM16_BOUNDMEMBER_THIS & 0xff, (uint16_t)(iid))))
#define asm_ggetmember_this_r(rid, iid) (asm_incsp(), asm_put881616(ASM_GETMEMBER_THIS_R, rid, iid))
#define asm_gdelmember_this_r(rid, iid) (asm_put881616(ASM_DELMEMBER_THIS_R, rid, iid))
#define asm_gsetmember_this_r(rid, iid) (asm_decsp(), asm_put881616(ASM_SETMEMBER_THIS_R, rid, iid))
#define asm_gsetmemberi_this_r(rid, iid) (asm_decsp(), asm_put881616(ASM_SETMEMBERI_THIS_R, rid, iid))
#define asm_gboundmember_this_r(rid, iid) (asm_incsp(), asm_put881616(ASM_BOUNDMEMBER_THIS_R, rid, iid))
#define asm_goperator(oid, n)                                            \
	(asm_subsp((n) + 1), asm_incsp(),                                    \
	 (oid) <= UINT8_MAX ? asm_putimm8_8(ASM_OPERATOR, (uint8_t)(oid), n) \
	                    : (asm_put((ASM16_OPERATOR & 0xff00) >> 8) ||    \
	                       asm_putimm16_8((ASM16_OPERATOR & 0xff), oid, n)))
#define asm_goperator_tuple(oid)                                          \
	(asm_ddicsp(),                                                        \
	 (oid) <= UINT8_MAX ? asm_putimm8(ASM_OPERATOR_TUPLE, (uint8_t)(oid)) \
	                    : (asm_put((ASM16_OPERATOR & 0xff00) >> 8) ||     \
	                       asm_putimm16((ASM16_OPERATOR & 0xff), oid)))
#define asm_genter()                  (asm_dicsp(), asm_put(ASM_ENTER))
#define asm_gleave()                  (asm_decsp(), asm_put(ASM_LEAVE))
#define asm_grange()                  (asm_ddicsp(), asm_put(ASM_RANGE))
#define asm_grange_0()                (asm_dicsp(), asm_put(ASM_RANGE_DEF))
#define asm_grange_step()             (asm_dddicsp(), asm_put(ASM_RANGE_STEP))
#define asm_grange_step_0()           (asm_ddicsp(), asm_put(ASM_RANGE_STEP_DEF))
#define asm_grange_0_i(end)           (asm_incsp(), (end) <= UINT16_MAX ? asm_putimm16(ASM_RANGE_0_I16, (uint16_t)(end)) : (asm_put((ASM_RANGE_0_I32 & 0xff00) >> 8) || asm_putimm32((ASM_RANGE_0_I32 & 0xff), (uint32_t)(end))))
#define asm_greduce_min()             (asm_dicsp(), asm_put16(ASM_REDUCE_MIN))
#define asm_greduce_max()             (asm_dicsp(), asm_put16(ASM_REDUCE_MAX))
#define asm_greduce_sum()             (asm_dicsp(), asm_put16(ASM_REDUCE_SUM))
#define asm_greduce_any()             (asm_dicsp(), asm_put16(ASM_REDUCE_ANY))
#define asm_greduce_all()             (asm_dicsp(), asm_put16(ASM_REDUCE_ALL))
#define asm_gclass()                  (asm_ddicsp(), asm_put16(ASM_CLASS))
#define asm_gclass_c(cid)             (asm_dicsp(), asm_put816(ASM_CLASS_C, cid))
#define asm_gclass_gc(gid, cid)       (asm_incsp(), asm_put881616(ASM_CLASS_GC, gid, cid))
#define asm_gclass_ec(mid, gid, cid)  (asm_incsp(), asm_put888161616(ASM_CLASS_EC, mid, gid, cid))
#define asm_gdefcmember(id)           (asm_ddicsp(), asm_put816(ASM_DEFCMEMBER, id))
#define asm_ggetcmember(id)           (asm_dicsp(), asm_put((ASM16_GETCMEMBER & 0xff00) >> 8) || asm_putimm16(ASM16_GETCMEMBER & 0xff, id))
#define asm_ggetcmember_r(rid, id)    (asm_incsp(), asm_put881616(ASM_GETCMEMBER_R, rid, id))
#define asm_gcallcmember_this_r(rid, id, n) (asm_subsp(n), asm_incsp(), asm_put881616_8(ASM_CALLCMEMBER_THIS_R, rid, id, n))

/* Super attribute access (optimizing the expression `(this as REF(rid)).operator . (CONST(cid))') */
#define asm_gsupergetattr_this_rc(rid, cid)    (asm_incsp(), asm_put16161616(ASM_SUPERGETATTR_THIS_RC, ASM16_SUPERGETATTR_THIS_RC, rid, cid))
#define asm_gsupercallattr_this_rc(rid, cid, n) (asm_subsp(n), asm_incsp(), asm_put16161616_8(ASM_SUPERCALLATTR_THIS_RC, ASM16_SUPERCALLATTR_THIS_RC, rid, cid, n))

/* Call with keyword list instructions. */
#define asm_gcall_kw(n, kwd_cid)                       (asm_subsp((n) + 1), asm_incsp(), asm_put8_816(ASM_CALL_KW, n, kwd_cid))
#define asm_gcall_tuple_kw(kwd_cid)                    (asm_ddicsp(), asm_put816(ASM_CALL_TUPLE_KW, kwd_cid))
#define asm_gcallattr_const_kw(att_cid, n, kwd_cid)    (asm_subsp((n) + 1), asm_incsp(), asm_put816_8_816(ASM_CALLATTR_C_KW, att_cid, n, kwd_cid))
#define asm_gcallattr_const_tuple_kw(att_cid, kwd_cid) (asm_ddicsp(), asm_put881616(ASM_CALLATTR_C_TUPLE_KW, att_cid, kwd_cid))
#define asm_gcall_tuple_kwds()                         (asm_dddicsp(), asm_put16(ASM_CALL_TUPLE_KWDS))
#define asm_gcallattr_kwds(n)                          (asm_subsp((n)+3), asm_incsp(), (asm_put(ASM_EXTENDED1) || asm_putimm8(ASM_CALLATTR_KWDS & 0xff, n)))
#define asm_gcallattr_tuple_kwds()                     (asm_ddddicsp(), asm_put16(ASM_CALLATTR_TUPLE_KWDS))

/* Instructions available with and without a Prefix instruction.
 * NOTE: With prefix, these instructions will operate as in-place. */
#define asm_ginplace_add()            (asm_decsp(), asm_put(ASM_ADD))
#define asm_ginplace_sub()            (asm_decsp(), asm_put(ASM_SUB))
#define asm_ginplace_mul()            (asm_decsp(), asm_put(ASM_MUL))
#define asm_ginplace_div()            (asm_decsp(), asm_put(ASM_DIV))
#define asm_ginplace_mod()            (asm_decsp(), asm_put(ASM_MOD))
#define asm_ginplace_shl()            (asm_decsp(), asm_put(ASM_SHL))
#define asm_ginplace_shr()            (asm_decsp(), asm_put(ASM_SHR))
#define asm_ginplace_and()            (asm_decsp(), asm_put(ASM_AND))
#define asm_ginplace_or()             (asm_decsp(), asm_put(ASM_OR))
#define asm_ginplace_xor()            (asm_decsp(), asm_put(ASM_XOR))
#define asm_ginplace_pow()            (asm_decsp(), asm_put(ASM_POW))
#define asm_ginplace_operator(oid, n)                                    \
	(asm_subsp(n), asm_incsp(),                                          \
	 (oid) <= UINT8_MAX ? asm_putimm8_8(ASM_OPERATOR, (uint8_t)(oid), n) \
	                    : (asm_put((ASM16_OPERATOR & 0xff00) >> 8) ||    \
	                       asm_putimm16_8((ASM16_OPERATOR & 0xff), oid, n)))
#define asm_ginplace_operator_tuple(oid)                                  \
	(asm_dicsp(),                                                         \
	 (oid) <= UINT8_MAX ? asm_putimm8(ASM_OPERATOR_TUPLE, (uint8_t)(oid)) \
	                    : (asm_put((ASM16_OPERATOR & 0xff00) >> 8) ||     \
	                       asm_putimm16((ASM16_OPERATOR & 0xff), oid)))

/* Instructions only available after a Prefix instruction. */
#define asm_ginc()                    (asm_put(ASM_INC))
#define asm_gdec()                    (asm_put(ASM_DEC))
#define asm_gincpost()                (asm_incsp(), asm_put16(ASM_INCPOST))
#define asm_gdecpost()                (asm_incsp(), asm_put16(ASM_DECPOST))

/* Instructions for compare-and-exchange of stack values. */
#define asm_gcmpxch_top_none_c(cid) (asm_dicsp(), asm_put1616(ASM_CMPXCH_UB_C, ASM16_CMPXCH_UB_C, cid))
#define asm_gcmpxch_top_none_pop()  (asm_ddicsp(), asm_put16(ASM_CMPXCH_UB_POP))
#define asm_gcmpxch_top_pop_none()  (asm_ddicsp(), asm_put16(ASM_CMPXCH_POP_UB))
#define asm_gcmpxch_top_pop_pop()   (asm_dddicsp(), asm_put16(ASM_CMPXCH_POP_POP))

/* Instructions for atomic compare-and-exchange of prefixable symbols. */
#define asm_gcmpxch_ub_c_p(cid) (asm_incsp(), asm_put1616(ASM_CMPXCH_UB_C, ASM16_CMPXCH_UB_C, cid))
#define asm_gcmpxch_ub_lock_p() (asm_incsp(), asm_put16(ASM_CMPXCH_UB_LOCK))
#define asm_gcmpxch_ub_pop_p()  (asm_dicsp(), asm_put16(ASM_CMPXCH_UB_POP))
#define asm_gcmpxch_pop_ub_p()  (asm_dicsp(), asm_put16(ASM_CMPXCH_POP_UB))
#define asm_gcmpxch_pop_pop_p() (asm_ddicsp(), asm_put16(ASM_CMPXCH_POP_POP))

/* Push/pop the current top value into a location on the stack, given its absolute address.
 * HINT: These functions are useful because `asm_gdup_n()' / `asm_gpop_n()' use relative addresses. */
INTDEF WUNUSED int DCALL asm_gpush_stack(uint16_t absolute_stack_addr);
INTDEF WUNUSED int DCALL asm_gpop_stack(uint16_t absolute_stack_addr);
/* Same as `_asm_gadjstack()', but automatically select other opcodes to generate minimal assembly. */
INTDEF WUNUSED int DCALL asm_gadjstack(int16_t offset);
/* Similar to `asm_gadjstack()', but set the absolute stack size. */
INTDEF WUNUSED int DCALL asm_gsetstack(uint16_t absolute_stack_size);
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gsetstack_s(struct asm_sym *__restrict target);

/* High-level encoding of l/r-rot instructions. */
INTDEF WUNUSED int DCALL asm_glrot(uint16_t num_slots);
INTDEF WUNUSED int DCALL asm_grrot(uint16_t num_slots);

/* Push the virtual argument known as `argid' */
INTDEF WUNUSED int DCALL asm_gpush_varg(uint16_t argid);

/* Store the value of the virtual argument `argid' in `dst' */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL
asm_gmov_varg(struct symbol *__restrict dst, uint16_t argid,
              struct ast *__restrict warn_ast,
              bool ignore_unbound);


/* Push the given value as an integer constant onto the stack. */
INTDEF WUNUSED int DCALL asm_gpush_u32(uint32_t value);
INTDEF WUNUSED int DCALL asm_gpush_s32(int32_t value);
#define asm_gpush_u16(value) asm_gpush_u32(value)
#define asm_gpush_s16(value) asm_gpush_s32(value)

/* High-level assembly generator functions.
 * NOTE: Unlike functions above, there are
 *       allowed to write multiple instructions. */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gpush_constexpr(DeeObject *__restrict value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL asm_gpush_symbol(struct symbol *__restrict sym, struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL asm_gcall_symbol_n(struct symbol *__restrict function, uint8_t argc, struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL asm_gprefix_symbol(struct symbol *__restrict sym, struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL asm_gprefix_symbol_for_read(struct symbol *__restrict sym, struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1)) bool DCALL asm_can_prefix_symbol(struct symbol *__restrict sym);
INTDEF WUNUSED NONNULL((1)) bool DCALL asm_can_prefix_symbol_for_read(struct symbol *__restrict sym);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL asm_gpush_bnd_symbol(struct symbol *__restrict sym, struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL asm_gdel_symbol(struct symbol *__restrict sym, struct ast *__restrict warn_ast);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL asm_gpop_symbol(struct symbol *__restrict sym, struct ast *__restrict warn_ast);

/* Check if `sym' is accessible from the current
 * source location, returning `false' if it isn't. */
#define asm_symbol_accessible(sym) \
	symbol_reachable(sym, current_assembler.a_scope)


/* Returns `true' if pushing `sym' is more expensive  */
INTDEF WUNUSED NONNULL((1)) bool DCALL
asm_gpush_symbol_is_expensive(struct symbol *__restrict sym);

/* Generate code to pop the stack-top value into the given AST. */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gpop_expr(struct ast *__restrict self);

/* Same as `asm_gpop_expr()', but pop `astc' values, one in each AST. */
INTDEF WUNUSED int DCALL asm_gpop_expr_multiple(size_t astc, struct ast **astv);

/* Generate code before and after the source expression when
 * trying to store an expression in a given destination `ast'
 * -> asm_gpop_expr_enter(dst);
 * -> ast_genasm(src, ASM_G_FPUSHRES);
 * -> asm_gpop_expr_leave(dst);
 * In `a[b] = c':
 * -> diff = asm_gpop_expr_enter(dst); // push @a; push @b;
 * -> ast_genasm(src, ASM_G_FPUSHRES); // push @c;
 * -> asm_gpop_expr_leave(dst, diff);  // setrange pop, pop, pop;
 */
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gpop_expr_enter(struct ast *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL asm_gpop_expr_leave(struct ast *__restrict self, unsigned int gflags);

INTDEF WUNUSED NONNULL((1)) int DCALL asm_enter_scope(DeeScopeObject *__restrict scope);
INTDEF WUNUSED int DCALL asm_leave_scope(DeeScopeObject *old_scope, uint16_t num_preserve);

#define ASM_PUSH_SCOPE(scope, err)                              \
	do {                                                        \
		DeeScopeObject *_old_scope = current_assembler.a_scope; \
		if (asm_enter_scope(scope))                             \
			goto err
/* @param: num_preserve: The amount of stack-entries to keep when returning. */
#define ASM_BREAK_SCOPE(num_preserve, err)             \
		if (asm_leave_scope(_old_scope, num_preserve)) \
			goto err
#define ASM_POP_SCOPE(num_preserve, err)    \
		ASM_BREAK_SCOPE(num_preserve, err); \
	}	__WHILE0

#define ASM_PUSH_LOC(loc)                                     \
	do {                                                      \
		struct ast_loc *_old_loc = current_assembler.a_error; \
		if ((loc)->l_file)                                    \
			current_assembler.a_error = (loc)
#define ASM_BREAK_LOC() \
		current_assembler.a_error = _old_loc
#define ASM_POP_LOC()    \
		ASM_BREAK_LOC(); \
	}	__WHILE0


/* Assembly code generation flags (gflags) */
#define ASM_G_FNORMAL   0x0000 /* Normal asm G-flags. */
#define ASM_G_FPUSHRES  0x0001 /* Push the result of the expression. */
#define ASM_G_FLAZYBOOL 0x0002 /* The result of the ast will be used as a boolean value. */


/* Generate a store expression `dst = src' */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
asm_gstore(struct ast *__restrict dst,
           struct ast *__restrict src,
           struct ast *ddi_ast,
           unsigned int gflags);

/* Unpack the given expression into `num_targets' stack slots. */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL
asm_gunpack_expr(struct ast *__restrict src,
                 uint16_t num_targets,
                 struct ast *__restrict ddi_ast);

/* Generate code to throw RuntimeError when `lid' is bound at runtime. */
INTDEF WUNUSED int DCALL asm_gcheck_final_local_bound(uint16_t lid);


/* Generate attribute-, item- and range-store operations. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
ast_gen_getattr(struct ast *__restrict base,
                struct ast *__restrict name,
                struct ast *ddi_ast,
                unsigned int gflags);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
ast_gen_boundattr(struct ast *__restrict base,
                  struct ast *__restrict name,
                  struct ast *ddi_ast,
                  unsigned int gflags);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
ast_gen_delattr(struct ast *__restrict base,
                struct ast *__restrict name,
                struct ast *ddi_ast);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
ast_gen_setattr(struct ast *__restrict base,
                struct ast *__restrict name,
                struct ast *__restrict value,
                struct ast *ddi_ast,
                unsigned int gflags);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
ast_gen_setitem(struct ast *__restrict sequence,
                struct ast *__restrict index,
                struct ast *__restrict value,
                struct ast *ddi_ast,
                unsigned int gflags);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
ast_gen_setrange(struct ast *__restrict sequence,
                 struct ast *__restrict begin,
                 struct ast *__restrict end,
                 struct ast *__restrict value,
                 struct ast *ddi_ast,
                 unsigned int gflags);
INTDEF WUNUSED NONNULL((2)) int DCALL
ast_gen_operator_func(struct ast *binding,
                      struct ast *ddi_ast,
                      Dee_operator_t operator_name);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL
ast_gen_symbol_inplace(struct symbol *__restrict sym,
                       struct ast *operand,
                       struct ast *ddi_ast,
                       Dee_operator_t inplace_operator_name,
                       bool is_post_operator,
                       unsigned int gflags);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL
ast_gen_setattr_inplace(struct ast *__restrict base,
                        struct ast *__restrict name,
                        struct ast *operand,
                        struct ast *ddi_ast,
                        Dee_operator_t inplace_operator_name,
                        bool is_post_operator,
                        unsigned int gflags);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL
ast_gen_setitem_inplace(struct ast *__restrict base,
                        struct ast *__restrict index,
                        struct ast *operand,
                        struct ast *ddi_ast,
                        Dee_operator_t inplace_operator_name,
                        bool is_post_operator,
                        unsigned int gflags);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL
ast_gen_setrange_inplace(struct ast *__restrict base,
                         struct ast *__restrict start,
                         struct ast *__restrict end,
                         struct ast *operand,
                         struct ast *ddi_ast,
                         Dee_operator_t inplace_operator_name,
                         bool is_post_operator,
                         unsigned int gflags);


/* Generate code to invoke a function `func' using arguments from `args'. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
asm_gcall_expr(struct ast *__restrict func,
               struct ast *__restrict args,
               struct ast *ddi_ast,
               unsigned int gflags);

/* Generate code to invoke a function `func' using
 * arguments from `args', and keywords from `kwds'. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
asm_gcall_kw_expr(struct ast *__restrict func,
                  struct ast *__restrict args,
                  struct ast *__restrict kwds,
                  struct ast *ddi_ast,
                  unsigned int gflags);

/* Return usage information about a given local/static variable by the instruction pointed to by `ip'.
 * @return: * : Set of `ASM_USING_*' */
INTDEF WUNUSED NONNULL((1)) unsigned int DCALL asm_uses_local(instruction_t const *__restrict ip, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) unsigned int DCALL asm_uses_static(instruction_t const *__restrict ip, uint16_t sid);
#define ASM_USING_READ  0x01 /* The variable is being read from. */
#define ASM_USING_WRITE 0x02 /* The variable is being written to. */


#if 0
struct register_effect {
#define REGISTER_EFFECT_CLASS_NONE   0x0000 /* No effect (Only found in _trailing_ register effects) */
#define REGISTER_EFFECT_CLASS_STACK  0x0001 /* Stack register effect */
#define REGISTER_EFFECT_CLASS_LOCAL  0x0002 /* Local variable effect */
#define REGISTER_EFFECT_CLASS_GLOBAL 0x0003 /* Global variable effect */
#define REGISTER_EFFECT_CLASS_CONST  0x0004 /* Constant variable effect (Never has the `REGISTER_EFFECT_FWRITE' flag set) */
#define REGISTER_EFFECT_CLASS_STATIC 0x0005 /* Static variable effect */
#define REGISTER_EFFECT_CLASS_EXTERN 0x0006 /* External variable effect */
#define REGISTER_EFFECT_CLASS_MODULE 0x0007 /* Module (import) variable effect */
#define REGISTER_EFFECT_CLASS_REF    0x0008 /* Reference variable effect (Never has the `REGISTER_EFFECT_FWRITE' flag set) */
#define REGISTER_EFFECT_CLASS_ARG    0x0009 /* Argument variable effect (Never has the `REGISTER_EFFECT_FWRITE' flag set) */
#define REGISTER_EFFECT_CLASS_MISC   0x000a /* Miscellaneous effect. */
	uint16_t         re_class;              /* Register effect class (One of `REGISTER_EFFECT_CLASS_*'). */
#define REGISTER_EFFECT_FREAD        0x0001 /* FLAG: The register is read from. */
#define REGISTER_EFFECT_FWRITE       0x0002 /* FLAG: The register is written to. */
	uint16_t         re_kind;               /* How registers are affected (Set of `REGISTER_EFFECT_F*').
	                                         * NOTE: At least one effect flag is guarantied to be set for this field. */
	union {
		uint16_t     re_index;              /* Static, constant, global, or local register index. */
		struct {
			uint16_t s_begin;               /* First affected, absolute stack index. */
			uint16_t s_end;                 /* First unaffected, absolute stack index. */
		}            re_stack;              /* REGISTER_EFFECT_CLASS_STACK */
		struct {
			uint16_t e_gid;                 /* Global index in `e_mid'. */
			uint16_t e_mid;                 /* Module index. */
		}            re_extern;             /* REGISTER_EFFECT_CLASS_EXTERN */
#define REGISTER_MISC_EXCEPT         0x0000 /* The currently thrown exception. */
#define REGISTER_MISC_THIS           0x0001 /* The this-argument. */
#define REGISTER_MISC_THIS_FUNCTION  0x0002 /* The current function. */
#define REGISTER_MISC_THIS_MODULE    0x0003 /* The current module. */
		uint16_t     re_misc;               /* REGISTER_EFFECT_CLASS_MISC (One of `REGISTER_MISC_*') */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define re_index  _dee_aunion.re_index
#define re_stack  _dee_aunion.re_stack
#define re_extern _dee_aunion.re_extern
#define re_misc   _dee_aunion.re_misc
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#define INSTRUCTION_EFFECT_MAXREGS  3 /* The max number of register-effects achievable by an instruction. */
struct instruction_effect {
	uint16_t                        ie_size;  /* Size (in bytes) of the text associated with the instruction (including any prefix) */
	uint16_t                        ie_spadd; /* Positive instruction stack-offset */
	uint16_t                        ie_spsub; /* Negative instruction stack-offset */
#define INSTRUCTION_EFFECT_FNORMAL  0x0000    /* No special behavior. */
#define INSTRUCTION_EFFECT_FJMP     0x0001    /* The instructions contains a jump to another address. */
#define INSTRUCTION_EFFECT_FRET     0x0002    /* The instruction returns to the caller. */
#define INSTRUCTION_EFFECT_FYLD     0x0004    /* The instruction yields to the caller. */
#define INSTRUCTION_EFFECT_FTHROW   0x0008    /* The instruction always throws an exception. */
#define INSTRUCTION_EFFECT_FCOND    0x8000    /* For `INSTRUCTION_EFFECT_FJMP': The jump is conditional. */
#define INSTRUCTION_EFFECT_FUD      0xffff    /* VALUE: Undefined instruction */
	uint16_t                        ie_flags; /* Special-instruction flags (Set of `INSTRUCTION_EFFECT_F*'). */
	struct register_effect          ie_regs[INSTRUCTION_EFFECT_MAXREGS]; /* Affected registers (array; terminated by either
	                                                                     * `REGISTER_EFFECT_CLASS_NONE', or reaching `INSTRUCTION_EFFECT_MAXREGS') */
	instruction_t                  *ie_jump;  /* [0..1][valid_if(INSTRUCTION_EFFECT_FJUMPS)]
	                                           * The target address to where a jumping instruction leads.
	                                           * NOTE: NULL if the jump cannot be followed. */
};

/* Decode the effect of an instruction found at `ip', store
 * that effect in `*effect' (when non-NULL), update `*p_stacksz'
 * in accordance to the instruction's stack-effect, and finally
 * return a pointer to the end of the decoded instruction (which
 * is usually a pointer to the next instruction, should there be one)
 * @param: code_flags: Set of `CODE_F*' */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) instruction_t *DCALL
DeeInstruction_Decode(instruction_t const *__restrict ip,
                      uint16_t *__restrict p_stacksz,
                      uint16_t code_flags,
                      struct instruction_effect *effect);
#endif




/* Generate assembly for the given AST. */
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_genasm(struct ast *__restrict self, unsigned int gflags);

/* Same as `ast_genasm()', but emit a compiler error and forcefully
 * re-adjust the stack if generated assembly produces more than 0/1
 * stack value(s) depending on `ASM_G_FPUSHRES' having been given. */
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_genasm_one(struct ast *__restrict self, unsigned int gflags);

/* Variants of `ast_genasm()' that will attempt to emit the expression
 * as either an AbstractSequeceProxy, or as a Set. In either case, if
 * no such optimization can be performed, or if the result isn't being
 * used, then the call is forwarded to `ast_genasm()'.
 *  - ast_genasm_asp() is used to generate the sequence-expression
 *    in a foreach-type loop, allowing unnecessary type-casts to be
 *    omitted: >> `for (local x: list(get_items()))' -> `for (local x: get_items())'
 *    Note however that this is only done for known sequence types,
 *    such as lists, or tuples, however is not done for user-defined
 *    sequence types.
 *  - ast_genasm_set() is sued to generate the sequence-expression
 *    in an in/contains expression, allowing a constant sequence of
 *    the expression to be compiled as an _RoSet, maximizing runtime
 *    performance.
 *    Additionally, ast_genasm_set() will try to strip unnecessary sequence
 *    casts from the expression, the same way `ast_genasm_asp()' would. */
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_genasm_asp(struct ast *__restrict self, unsigned int gflags);
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_genasm_set(struct ast *__restrict self, unsigned int gflags);
INTDEF WUNUSED NONNULL((1)) int DCALL
ast_genasm_set_one(struct ast *__restrict self, unsigned int gflags);

/* Same as `DeeRoSet_FromSequence()', but has special handling for when `self' is a Mapping */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoSet_FromSequenceOrMappingForContains(DeeObject *__restrict self);

/* Strip sequence-style cast expressions from `ast' and return an inner sequence.
 * If `ast' is no sequence expression, re-return it directly. */
INTDEF WUNUSED NONNULL((1)) struct ast *DCALL ast_strip_seqcast(struct ast *__restrict self);

/* Generate text for a given `AST_SWITCH' branch. */
INTDEF WUNUSED NONNULL((1)) int DCALL ast_genasm_switch(struct ast *__restrict self);

/* Generate user-assembly for a given `AST_ASSEMBLY' branch. */
INTDEF WUNUSED NONNULL((1)) int DCALL ast_genasm_userasm(struct ast *__restrict self);

/* Compile a DDI object for use by generated code. */
INTDEF WUNUSED DREF DeeDDIObject *DCALL ddi_compile(void);

/* Compile a new function, using `current_basescope'
 * as scope, and the given expression as code.
 * @param: flags: Set of `ASM_F*' (Assembly flags; see above)
 * @param: first_function: True if the function is the first one to-be compiled.
 *                         When set, don't propagate reference scopes normally
 *                         required to allow for recursive assembly and inter-
 *                         text-segment symbol referencing.
 * @param: p_refc: The amount of required references to create a function object.
 * @param: p_refv: Upon success, a vector of symbols that must passed to the
 *                code object when a function is being created. */
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeCodeObject *DCALL
code_compile(struct ast *__restrict code_ast, uint16_t flags,
             bool first_function, uint16_t *__restrict p_refc,
             /*out:inherit*/ struct asm_symbol_ref **__restrict p_refv);

/* Similar to `code_compile()', however instructs the assembly to try to use
 * anonymous arguments in order to compile the code, meaning that rather than
 * being passed through references, referenced objects are passed through
 * arguments (following arguments that were already passed regularly)
 * This is mainly used when a function branch is directly invoked, as is the
 * case for generator expressions, thus potentially allowing the generated
 * code to be called as a constant function object (without the need of
 * creating a new, temporary function object in addition to the yield-function
 * and its wrapper), leading to faster code.
 * NOTE: Because it's unfeasible to enforce the use of arguments instead of
 *       references for all places where references may be used, an argrefs
 *       code object may still make use of regular references, which is why
 *       there are still `p_refc' and `p_refv' arguments.
 *       The main culprit here is user-defined assembly, which may require
 *       the creation of references.
 * @param: p_argv: Filled with a pointer to the a vector of extended symbols
 *                 that must be passed to the function during invocation,
 *                 following the regular argument list.
 *                 Upon success, the caller must `Dee_Free()' this vector.
 * @param: p_argc: Same as `p_argv', but filled with the number of reference-arguments. */
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) DREF DeeCodeObject *DCALL
code_compile_argrefs(struct ast *__restrict code_ast, uint16_t flags,
                     uint16_t *__restrict p_refc, /*out:inherit*/ struct asm_symbol_ref **__restrict p_refv,
                     uint16_t *__restrict p_argc, /*out:inherit*/ struct symbol ***__restrict p_argv);



INTDEF WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
code_docompile(struct ast *__restrict code_ast);

/* Compile a new module, using `current_rootscope' for module information,
 * and the given code object as root code executed when the module is loaded.
 * WARNING: During this process a lot of data is directly inherited from
 *         `current_rootscope' by the returned module object, meaning that the
 *          root scope will have been reset to an empty (or near empty) state.
 * @param: flags: Set of `ASM_F*' (Assembly flags; see above) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
module_compile(DREF struct module_object *__restrict module,
               DeeCodeObject *__restrict root_code,
               uint16_t flags);


INTDEF WUNUSED NONNULL((1)) int DCALL
asm_gpush_function(struct ast *__restrict function_ast);

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
asm_gmov_function(struct symbol *__restrict dst,
                  struct ast *function_ast,
                  struct ast *dst_warn_ast);

/* Move the given symbol `src_sym' into `dst_sym'.
 * NOTE: `asm_can_prefix_symbol(dst_sym)' must be true. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL asm_gmov_sym_sym)(struct symbol *__restrict dst_sym,
                         struct symbol *__restrict src_sym,
                         struct ast *dst_ast,
                         struct ast *src_ast);

/* Store the expression in `src' into `dst'.
 * NOTE: `asm_can_prefix_symbol(dst_sym)' must be true. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int
(DCALL asm_gmov_sym_ast)(struct symbol *__restrict dst_sym,
                         struct ast *src,
                         struct ast *dst_ast);

/* Store the symbol `src_sym' into the expression `dst'. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int
(DCALL asm_gmov_ast_sym)(struct ast *dst,
                         struct symbol *__restrict src_sym,
                         struct ast *src_ast);

/* @param: loop_flags:   Set of `AST_FLOOP_*'
 * @param: elem_or_cond: The loop element target ([0..1] in a foreach loop),
 *                       or the loop-continue condition ([0..1] in other loop types).
 * @param: iter_or_next: The loop iterator ([1..1] in a foreach loop),
 *                       or an optional expression executed at the end
 *                       of each iteration, and jumped to by `continue' ([0..1])
 * @param: block:        The main loop block executed in each iteration ([0..1])
 * @param: ddi_ast:      A branch used for debug information.
 * @return: * :         `loop_break' -- This symbol must be defined immediately
 *                       after the loop, however after variables allocated by
 *                       the scope have been disposed of. */
INTDEF WUNUSED NONNULL((5)) struct asm_sym *
(DCALL asm_genloop)(uint16_t loop_flags,
                    struct ast *elem_or_cond,
                    struct ast *iter_or_next,
                    struct ast *block,
                    struct ast *ddi_ast);

/* Sub-routine for `ast_genasm' for `AST_CLASS' */
INTDEF WUNUSED NONNULL((1)) int
(DCALL asm_genclass)(struct ast *__restrict class_ast,
                     unsigned int gflags);

/* Sub-routine for `ast_genasm' for `AST_TRY' */
INTDEF WUNUSED NONNULL((1)) int
(DCALL asm_gentry)(struct ast *__restrict try_ast,
                   unsigned int gflags);

/* Generate text for an assertion. */
INTDEF WUNUSED NONNULL((1, 3)) int
(DCALL asm_genassert)(struct ast *__restrict expr,
                      struct ast *message,
                      struct ast *ddi_ast,
                      unsigned int gflags);


/* Available print modes. */
#define PRINT_MODE_NORMAL (ASM_PRINT - ASM_PRINT)    /* Mode: print. */
#define PRINT_MODE_SP     (ASM_PRINT_SP - ASM_PRINT) /* Mode: print, followed by a space. */
#define PRINT_MODE_NL     (ASM_PRINT_NL - ASM_PRINT) /* Mode: print, followed by a new-line. */
#define PRINT_MODE_FILE   (ASM_FPRINT - ASM_PRINT)   /* FLAG: Print to a file. */
#define PRINT_MODE_ALL    (ASM_PRINTALL - ASM_PRINT) /* FLAG: Print elements from a sequence. */

/* Generate code for the expression `print print_expression...;'
 * @param: mode: The print mode. NOTE: When `PRINT_MODE_FILE' is set,
 *               then the caller must first push the file to print to. */
INTDEF WUNUSED NONNULL((2, 3)) int DCALL
ast_genprint(instruction_t mode,
             struct ast *print_expression,
             struct ast *ddi_ast);

/* Same as `ast_genprint()', but print the repr of `print_expression' */
INTDEF WUNUSED NONNULL((2, 3)) int DCALL
ast_genprint_repr(instruction_t mode,
                  struct ast *print_expression,
                  struct ast *ddi_ast);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#ifdef NDEBUG
#define asm_putddi(loc)                                    __builtin_expect(asm_putddi(loc), 0)
#else /* NDEBUG */
#define asm_putddi_dbg(loc, file, line)                    __builtin_expect(asm_putddi_dbg(loc, file, line), 0)
#endif /* !NDEBUG */
#define asm_gadjhand(target)                               __builtin_expect(asm_gadjhand(target), 0)
#define asm_gunwind()                                      __builtin_expect(asm_gunwind(), 0)
#define asm_gjmp(instr, target)                            __builtin_expect(asm_gjmp(instr, target), 0)
#define asm_gjcc(cond, instr, target, ddi_ast)             __builtin_expect(asm_gjcc(cond, instr, target, ddi_ast), 0)
#define asm_put(instr)                                     __builtin_expect(asm_put(instr), 0)
#define asm_putimm8(instr, imm8)                           __builtin_expect(asm_putimm8(instr, imm8), 0)
#define asm_putimm8_8(instr, imm8_1, imm8_2)               __builtin_expect(asm_putimm8_8(instr, imm8_1, imm8_2), 0)
#define asm_putimm8_8_8(instr, imm8_1, imm8_2, imm8_3)     __builtin_expect(asm_putimm8_8_8(instr, imm8_1, imm8_2, imm8_3), 0)
#define asm_putimm8_16(instr, imm8_1, imm16_2)             __builtin_expect(asm_putimm8_16(instr, imm8_1, imm16_2), 0)
#define asm_putimm16(instr, imm16)                         __builtin_expect(asm_putimm16(instr, imm16), 0)
#define asm_putimm16_8(instr, imm16_1, imm8_2)             __builtin_expect(asm_putimm16_8(instr, imm16_1, imm8_2), 0)
#define asm_putimm16_16(instr, imm16_1, imm16_2)           __builtin_expect(asm_putimm16_16(instr, imm16_1, imm16_2), 0)
#define asm_putimm16_16_8(instr, imm16_1, imm16_2, imm8_3) __builtin_expect(asm_putimm16_16_8(instr, imm16_1, imm16_2, imm8_3), 0)
#define asm_putimm16_8_16(instr, imm16_1, imm8_2, imm16_3) __builtin_expect(asm_putimm16_8_16(instr, imm16_1, imm8_2, imm16_3), 0)
#define asm_putimm32(instr, imm32)                         __builtin_expect(asm_putimm32(instr, imm32), 0)
#define asm_putsid16(instr, sid)                           __builtin_expect(asm_putsid16(instr, sid), 0)
#define asm_gpush_constexpr(value)                         __builtin_expect(asm_gpush_constexpr(value), 0)
#define asm_gpush_symbol(sym, warn_ast)                    __builtin_expect(asm_gpush_symbol(sym, warn_ast), 0)
#define asm_gprefix_symbol(sym, warn_ast)                  __builtin_expect(asm_gprefix_symbol(sym, warn_ast), 0)
#define asm_gprefix_symbol_for_read(sym, warn_ast)         __builtin_expect(asm_gprefix_symbol_for_read(sym, warn_ast), 0)
#define asm_gpush_bnd_symbol(sym, warn_ast)                __builtin_expect(asm_gpush_bnd_symbol(sym, warn_ast), 0)
#define asm_gdel_symbol(sym, warn_ast)                     __builtin_expect(asm_gdel_symbol(sym, warn_ast), 0)
#define asm_gpop_symbol(sym, warn_ast)                     __builtin_expect(asm_gpop_symbol(sym, warn_ast), 0)
#define ast_genasm(ast, gflags)                            __builtin_expect(ast_genasm(ast, gflags), 0)
#define ast_genasm_one(ast, gflags)                        __builtin_expect(ast_genasm_one(ast, gflags), 0)
#define asm_putrel(type, sym, value)                       __builtin_expect(asm_putrel(type, sym, value), 0)
#ifndef asm_put_data16
#define asm_put_data16(data)                               __builtin_expect(asm_put_data16(data), 0)
#endif /* !asm_put_data16 */
#define asm_put_data32(data)                               __builtin_expect(asm_put_data32(data), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


typedef struct {
	OBJECT_HEAD
	struct asm_sym *ri_sym;  /* [1..1][REF(->as_used)] Symbol added to relocation integer. */
	tint_t          ri_add;  /* Addend added to the value of `ri_sym'. */
#define RELINT_MODE_FADDR 0x0000 /* Use the address of `ri_sym' */
#define RELINT_MODE_FSTCK 0x0001 /* Use the stack-depth of `ri_sym' */
	uint16_t        ri_mode; /* The mode in which `ri_sym' is used. (One of `RELINT_MODE_F*') */
} DeeRelIntObject;

INTDEF DeeTypeObject DeeRelInt_Type;

/* Construct and register a new relocation-integer as a constant.
 * If `sym' is NULL, a regular integer is created instead. */
INTDEF WUNUSED int32_t DCALL
asm_newrelint(struct asm_sym *sym, tint_t addend, uint16_t mode);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRelInt_New(struct asm_sym *__restrict sym,
              tint_t addend, uint16_t mode);


DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_ASSEMBLER_H */
