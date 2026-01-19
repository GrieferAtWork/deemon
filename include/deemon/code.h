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
#ifndef GUARD_DEEMON_CODE_H
#define GUARD_DEEMON_CODE_H 1

#include "api.h"

#include <hybrid/typecore.h>

#include "types.h"
#include "util/lock.h"
#include "util/rlock.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintptr_t */

DECL_BEGIN

/* Explanation: Global variables:
 *   Global variables are stored in the current module.
 *   They are addressed using the immediate operand as an
 *   index into the current module's `mo_globalv' vector.
 *    - Global variables can be used as l-values.
 *    - Global variables can either be bound, or unbound.
 *
 * Explanation: Extern variables:
 *   Extern variables are used to address the global
 *   variables of a different module than the current.
 *   They are addressed using a pair of immediate values,
 *   written as <immX>:<immX>, where the first immediate
 *   value acts as an index into the `mo_importv' vector
 *   of the current module, while the second acts the
 *   same way that a global immediate index acts by then
 *   indexing into `mo_globalv'.
 *    - Extern variables can be used as l-values.
 *    - Extern variables can either be bound, or unbound.
 * 
 * Explanation: Local variables:
 *   Local variables are stored in code frames and are
 *   unique during each execution of accompanying code.
 *    - Local variables can be used as l-values.
 *    - Local variables can either be bound, or unbound.
 *   
 * Explanation: Reference variables:
 *   Reference variables are stored in the associated function object (not code!)
 *   and are created as fixed objects when the function is created by the surrounding
 *   scope. Their existence becomes important in lambda expression and local functions
 *   using variables from surrounding scopes that are not the global scope:
 *   >> function ref_demo() {
 *   >>     local ref_value = "foobar";
 *   >>     function get_ref_value() {
 *   >>         // Use an out-of-scope variable that is not part of the global scope.
 *   >>         return ref_value;
 *   >>     }
 *   >>     return get_ref_value;
 *   >> }
 *   >> print ref_demo()(); // `"foobar"'
 *   NOTES:
 *    - Reference variables cannot be modified, or used as l-values.
 *    - Reference variables are always bound.
 *    - The originating scope can still use the referenced variable
 *      normally, however overwriting its storage location will not
 *      update the value referenced by lambda functions that were
 *      constructed prior to the variable changing.
 *
 * Explanation: Argument variables:
 *   Used to access arguments passed to a function.
 *   Access to such variables undergoes special transformations in
 *   order to pack varargs into tuples, extract the `this' argument,
 *   and substitute default parameters.
 *   WARNING: Argument variables cannot be modified, or used as l-values.
 *            User-code is able to (seemingly) write to argument variables,
 *            however the compiler will generate stubs to copy arguments
 *            into hidden locals, and it is those that are then used for
 *            the duration of the function call.
 *   WARNING: Argument variables are always bound (optional arguments
 *            that *appear* to be unbound are achived by clever use of
 *            default arguments (which can be unbound), and variable
 *            arguments (where an argument is bound based on the number
 *            of variable arguments passed to a function)).
 *      TLDR: - Don't pass NULL in `argv' vectors
 *            - `argv' vectors mustn't be modified by callees
 *            - `function foo(a?) {}' is compiler magic,
 *              and is-bound-ness is determined by `argc'
 *
 * Explanation: Static/const variables:
 *   Stored in the code object itself, static/const variables are
 *   intended as extension space for storing pre-defined constants,
 *   as well as variables to which changes remain consistent across
 *   multiple calls to the same function (or rather underlying code object).
 *   Special instructions are available for accessing static variables
 *   as constants, rather than variables.
 *    - Static variables can be used as l-values.
 *    - Static variables can either be bound, or unbound.
 *    - Constant variables must not be modified or contain mutable objects.
 *    - Constant variables are always bound.
 *   There is no way to determine if a slot in the static/const vector
 *   belongs to a constant or static variable, other than by inspecting
 *   how the associated code object uses the slot. (Note that for forward
 *   compatibility, don't rely on DDI information to determine if a slot
 *   has a name, and then assume that if it does, it's a static variable)
 */

#ifdef DEE_SOURCE
#define Dee_code_frame                      code_frame
#define Dee_tuple_object                    tuple_object
#define Dee_string_object                   string_object
#define Dee_module_object                   module_object
#define Dee_code_object                     code_object
#define Dee_function_object                 function_object
#define Dee_yield_function_object           yield_function_object
#define Dee_yield_function_iterator_object  yield_function_iterator_object
#define Dee_ddi_object                      ddi_object
#define Dee_except_handler                  except_handler
#define Dee_ddi_regs                        ddi_regs
#define Dee_ddi_xregs                       ddi_xregs
#define Dee_ddi_saved                       ddi_saved
#define Dee_ddi_state                       ddi_state
#define Dee_ddi_exdat                       ddi_exdat
#define EXCEPTION_HANDLER_FNORMAL           Dee_EXCEPTION_HANDLER_FNORMAL
#define EXCEPTION_HANDLER_FFINALLY          Dee_EXCEPTION_HANDLER_FFINALLY
#define EXCEPTION_HANDLER_FINTERPT          Dee_EXCEPTION_HANDLER_FINTERPT
#define EXCEPTION_HANDLER_FHANDLED          Dee_EXCEPTION_HANDLER_FHANDLED
#define EXCEPTION_HANDLER_FMASK             Dee_EXCEPTION_HANDLER_FMASK
#define DDI_REGS_FNORMAL                    Dee_DDI_REGS_FNORMAL
#define DDI_REGS_FISSTMT                    Dee_DDI_REGS_FISSTMT
#define DDI_REGS_FSECOND                    Dee_DDI_REGS_FSECOND
#define DDI_REGS_FSECONE                    Dee_DDI_REGS_FSECONE
#define DDI_REGS_FMASK                      Dee_DDI_REGS_FMASK
#define DDI_REGS_UNBOUND_NAME               Dee_DDI_REGS_UNBOUND_NAME
#define DDI_STATE_DO                        Dee_DDI_STATE_DO
#define DDI_STATE_WHILE                     Dee_DDI_STATE_WHILE
#define DDI_STATE_FNORMAL                   Dee_DDI_STATE_FNORMAL
#define DDI_STATE_FNOTHROW                  Dee_DDI_STATE_FNOTHROW
#define DDI_STATE_FNONAMES                  Dee_DDI_STATE_FNONAMES
#define DDI_STATE_FNOEXCEPT                 Dee_DDI_STATE_FNOEXCEPT
#define DDI_NEXT_DONE                       Dee_DDI_NEXT_DONE
#define DDI_NEXT_ERR                        Dee_DDI_NEXT_ERR
#define DDI_ISOK                            Dee_DDI_ISOK
#define DEFINE_DDI                          Dee_DEFINE_DDI
#define Dee_code_frame_kwds                 code_frame_kwds
#define Dee_function_info                   function_info
#define function_info_fini                  Dee_function_info_fini
#define DEFINE_CODE                         Dee_DEFINE_CODE
#define DEFINE_FUNCTION                     Dee_DEFINE_FUNCTION
#define DEFINE_FUNCTION_NOREFS              Dee_DEFINE_FUNCTION_NOREFS
#define DEFINE_YIELD_FUNCTION               Dee_DEFINE_YIELD_FUNCTION
#define DEFINE_YIELD_FUNCTION_NOARGS        Dee_DEFINE_YIELD_FUNCTION_NOARGS
#endif /* DEE_SOURCE */

struct Dee_code_frame;
struct Dee_function_object;
struct Dee_tuple_object;
struct Dee_string_object;
struct Dee_module_object;
typedef struct Dee_code_object DeeCodeObject;
typedef struct Dee_function_object DeeFunctionObject;
typedef struct Dee_yield_function_object DeeYieldFunctionObject;
typedef struct Dee_yield_function_iterator_object DeeYieldFunctionIteratorObject;
typedef struct Dee_ddi_object DeeDDIObject;
#ifndef DEE_INSTRUCTION_T_DEFINED
#define DEE_INSTRUCTION_T_DEFINED 1
typedef __BYTE_TYPE__ Dee_instruction_t;
#endif /* !DEE_INSTRUCTION_T_DEFINED */
#define DEE_SIZEOF_CODE_ADDR_T 4
#define DEE_SIZEOF_CODE_SIZE_T 4
typedef uint32_t Dee_code_addr_t;
typedef uint32_t Dee_code_size_t;
typedef int32_t Dee_code_saddr_t;

#ifdef DEE_SOURCE
#ifndef INSTRUCTION_T_DEFINED
#define INSTRUCTION_T_DEFINED 1
typedef Dee_instruction_t instruction_t;
#endif /* !INSTRUCTION_T_DEFINED */
typedef Dee_code_addr_t   code_addr_t;
typedef Dee_code_size_t   code_size_t;
typedef Dee_code_saddr_t  code_saddr_t;
#endif /* DEE_SOURCE */

struct Dee_except_handler {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	DREF DeeTypeObject            *eh_mask;   /* [0..1][const] When set, only jump to this handler when the
	                                           *               last raised exception is an instance of `eh_mask'. */
	Dee_code_addr_t                eh_start;  /* [const][<= eh_end] Exception handler protection start address. */
	Dee_code_addr_t                eh_end;    /* [const][>= eh_start] Exception handler protection end address. */
	Dee_code_addr_t                eh_addr;   /* [const][< eh_start && >= eh_end] Exception handler entry point. */
	uint16_t                       eh_stack;  /* [const] Stack depth that must be ensured when this handler is executed.
	                                           * NOTE: When greater than the depth at the time of the exception
	                                           *       happening, the stack is filled with `none'. */
#define Dee_EXCEPTION_HANDLER_FNORMAL  0x0000 /* Normal exception handler flags. */
#define Dee_EXCEPTION_HANDLER_FFINALLY 0x0001 /* This handler must be executed as a `finally' handler. */
#define Dee_EXCEPTION_HANDLER_FINTERPT 0x0002 /* This handler is allowed to handle interrupt signals.
	                                           * In other words: It's allowed to catch instances of `Signal.Interrupt' */
#define Dee_EXCEPTION_HANDLER_FHANDLED 0x0004 /* Enter the exception handler with the exception already discarded.
	                                           * This flag is useful for exception handlers that wouldn't
	                                           * actually contain any code other than an `end catch'
	                                           * instruction, which this exception handler flag emulates
	                                           * before jumping to the handler entry point, which would
	                                           * then point back into regular text, after the empty handler. */
#define Dee_EXCEPTION_HANDLER_FMASK    0x0007 /* Mask of known exception handler flags. */
	uint16_t                       eh_flags;  /* Exception handler flags (Set of `EXCEPTION_HANDLER_F*') */
};


#define Dee_DDI_REGS_FNORMAL 0x0000 /* Normal flags. */
#define Dee_DDI_REGS_FISSTMT 0x0001 /* The current assembly is part of a statement
                                     * This flag should be used as a hint for debuggers
                                     * when setting breakpoints on a given line number,
                                     * in that code points with this register set should
                                     * be preferred over ones without. */
#define Dee_DDI_REGS_FSECOND 0xff00 /* Used internally */
#define Dee_DDI_REGS_FSECONE 0x0100 /* Used internally */
#define Dee_DDI_REGS_FMASK   0x0001 /* Mask of known register flags */

#define Dee_DDI_REGS_UNBOUND_NAME  ((uint16_t)-1) /* Symbol name address used for unbound slots. */
struct Dee_ddi_regs {
	Dee_code_addr_t      dr_uip;    /* The current user instruction. */
	uint16_t             dr_usp;    /* The current stack alignment/depth.
	                                 * NOTE: If a debugger chooses to adjust PC to manually jump to `dr_uip', it
	                                 *       must also match a stack depth of `dr_usp' before starting/continuing
	                                 *       execution of code.
	                                 *       Otherwise, the interpreter is not in a consistent state and
	                                 *       will either crash (FAST-mode), or throw an error (SAFE-mode). */
	uint16_t             dr_flags;  /* Set of `DDI_REGS_F*' */
	uint16_t             dr_path;   /* The current path number. (NOTE: ZERO indicates no path and all other values
	                                 *                                 are used as index-1 with `DeeCode_GetDDIString()') */
	uint16_t             dr_file;   /* The current file number. (for use with `DeeCode_GetDDIString()') */
	uint16_t             dr_name;   /* The current function name. (for use with `DeeCode_GetDDIString()') */
#if (__SIZEOF_POINTER__ - (14 % __SIZEOF_POINTER__)) != 0
	uint16_t            _dr_pad[(__SIZEOF_POINTER__ - (14 % __SIZEOF_POINTER__))/2]; /* ... */
#endif /* (__SIZEOF_POINTER__ - (14 % __SIZEOF_POINTER__)) != 0 */
	int                  dr_col;    /* The current column number within the active line (0-based). */
	int                  dr_lno;    /* Line number (0-based). */
};

struct Dee_ddi_xregs {
	struct Dee_ddi_regs  dx_base;   /* The normal register set. */
	uint16_t             dx_spnama; /* Allocated amount of stack-item names. */
	uint16_t             dx_lcnamc; /* [const][== ::co_localc] Number of local variable name registers. */
#if __SIZEOF_POINTER__ > 4
	uint16_t            _dx_pad[(sizeof(void *) / 2) - 2];
#endif /* __SIZEOF_POINTER__ > 4 */
	uint16_t            *dx_spnamv; /* [0..MIN(dx_base.dr_usp, dx_spnama)|ALLOC(dx_spnama)]
	                                 * [owned] Vector of stack-symbol name registers.
	                                 * NOTE: Unallocated indices should be assumed to be unbound. */
	uint16_t            *dx_lcnamv; /* [0..dx_lcnamc][owned] Vector of local variable name registers. */
};

struct Dee_ddi_saved {
	struct Dee_ddi_xregs  s_save;    /* Saved register state. NOTE: `dr_uip' is undefined. */
	struct Dee_ddi_saved *s_prev;    /* [0..1][owned] Previous, saved register state */
};

struct Dee_ddi_state {
	union {
		struct Dee_ddi_regs  rs_regs;   /* The current register state. */
		struct Dee_ddi_xregs rs_xregs;  /* The current register state (in extended form). */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define rs_regs  _dee_aunion.rs_regs
#define rs_xregs _dee_aunion.rs_xregs
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	struct Dee_ddi_saved    *rs_save;   /* [0..1][owned] Chain of saved register states.
	                                     * When enumerating for a traceback, this
	                                     * the list of inline function calls. */
};

/* Enumerate all the frames of a given DDI-state from most- to least-recent:
 * >> struct Dee_ddi_state state;
 * >> struct Dee_ddi_xregs *iter;
 * >> DDI_STATE_DO(iter, &state) {
 * >>     printf("line = %d\n", iter->dx_base.dr_lno);
 * >> } DDI_STATE_WHILE(iter, &state);
 */
#define Dee_DDI_STATE_DO(ddi_xregs_iter, self) \
	(ddi_xregs_iter) = &(self)->rs_xregs;      \
	do
#define Dee_DDI_STATE_WHILE(ddi_xregs_iter, self) \
	while (((ddi_xregs_iter) = &__COMPILER_CONTAINER_OF(ddi_xregs_iter, struct Dee_ddi_saved, s_save)->s_prev->s_save) != NULL)



/* Possible flag values for:
 * - Dee_ddi_state_init()
 * - Dee_ddi_next_state()
 * - DeeCode_FindDDI() */
#define Dee_DDI_STATE_FNORMAL   0x0000 /* Normal DDI State iteration flags. */
#define Dee_DDI_STATE_FNOTHROW  0x0001 /* Don't throw errors when something goes wrong, but rely on weak undefined behavior.
                                        * This flag should be set when DDI is being enumerated to generate a traceback. */
#define Dee_DDI_STATE_FNONAMES  0x0002 /* Don't keep track of bound symbol names */
#define Dee_DDI_STATE_FNOEXCEPT 0x0004 /* Similar to `Dee_DDI_STATE_FNOTHROW', but simply prevents calls to `DeeError_Throw()',
                                        * or anything else that might cause user-code to be invoked. */

/* Special return values for:
 * - Dee_ddi_state_init()
 * - Dee_ddi_next_simple()
 * - Dee_ddi_next_regs()
 * - Dee_ddi_next_state()
 * - DeeCode_FindDDI() */
#define Dee_DDI_NEXT_DONE ((uint8_t *)0)  /* DDI Iteration stopped */
#define Dee_DDI_NEXT_ERR  ((uint8_t *)(uintptr_t)-1) /* An error occurred. */

/* Check that `x != DDI_NEXT_DONE && x != DDI_NEXT_ERR' */
#define Dee_DDI_ISOK(x)  (((uintptr_t)(x) - 1) < (uintptr_t)-2l)


/* Initialize the given DDI register state from `code'.
 * @param: flags:          Set of `DDI_STATE_F*'
 * @return: * :            Successfully initialized the register state.
 *                         A pointer to the next DDI instruction.
 *                         This pointer can be used to enumerate DDI information.
 * @return: DDI_NEXT_ERR:  [!DDI_STATE_FNOTHROW] An error occurred.
 * NOTE: Upon error (return == DDI_NEXT_DONE || return == DDI_NEXT_ERR),
 *       the given ddi-state `self' is initialized to a no-op state that
 *       can still be used in a call to `Dee_ddi_state_fini()'! */
DFUNDEF WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_state_init(struct Dee_ddi_state *__restrict self,
                   DeeObject *__restrict code,
                   unsigned int flags);
DFUNDEF NONNULL((1)) void DCALL
Dee_ddi_state_fini(struct Dee_ddi_state *__restrict self);




/* Execute DDI instructions and update `regs' until the next checkpoint.
 * Return the new instruction point, pointing after the that checkpoint,
 * or return `NULL' when `DDI_STOP' has been encountered.
 * @param: ip:                The instruction point from which DDI assembly should be read.
 *                            During the first call, a pointer to `d_ddi' should be passed.
 * @param: regs:     [IN|OUT] The current DDI state machine register settings.
 *                            This register structure should not be modified
 *                            between successive calls to this function.
 *                            Before the first call, this structure should be
 *                            initialized by copying its contents from `d_start'.
 * @param: flags:             Set of `DDI_STATE_F*'
 * @return: * :               A pointer to the next DDI instruction.
 *                            This pointer, alongside the updated state of `regs'
 *                            can be used in successive calls to continue
 *                            enumerating/searching the data stream.
 * @return: DDI_NEXT_ERR:    [Dee_ddi_next_state && !DDI_STATE_FNOTHROW] An error occurred.
 * @return: DDI_NEXT_DONE:    The DDI information stream has ended after `DDI_STOP' was read. */
DFUNDEF WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_next_simple(uint8_t *__restrict ip,
                    Dee_code_addr_t *__restrict p_uip);
DFUNDEF WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_next_regs(uint8_t *__restrict ip,
                  struct Dee_ddi_regs *__restrict regs);
DFUNDEF WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_next_state(uint8_t *__restrict ip,
                   struct Dee_ddi_state *__restrict regs,
                   unsigned int flags);


/* DDI extended data opcode flags. */
#define DDI_EXDAT_OP8     0x40 /*  8-bit extended data field. (+1+1) (operands are little-endian) */
#define DDI_EXDAT_OP16    0x80 /* 16-bit extended data field. (+2+2) (operands are little-endian) */
#define DDI_EXDAT_OP32    0xc0 /* 32-bit extended data field. (+2+4) (operands are little-endian) */
#define DDI_EXDAT_OPMASK  0xc0 /* Operand size mask. */
#define DDI_EXDAT_MAXSIZE 7    /* Max size (in bytes) of a single DDI extension data block. */
#define DDI_EXDAT_O_END   0x00 /* End of extended data. */
#define DDI_EXDAT_O_RNAM  0x00 /* | DDI_EXDAT_OP*; [id, strtab_offset] */

struct Dee_ddi_exdat {
	uint32_t                         dx_size;  /* Amount of extended data bytes. */
	COMPILER_FLEXIBLE_ARRAY(uint8_t, dx_data); /* [dx_size] Extended DDI data. */
};

struct Dee_ddi_object {
	Dee_OBJECT_HEAD
	uint32_t const                  *d_strings; /* [OFFSET(d_strtab->s_str)][0..d_nstring][owned][const] Vector of DDI string offsets. */
	DREF struct Dee_string_object   *d_strtab;  /* [1..1][const] String table of NUL-terminated strings.
	                                             *               All offsets above point into this table. */
	struct Dee_ddi_exdat const      *d_exdat;   /* [0..1][owned][const] Extended DDI data */
	uint32_t                         d_ddisize; /* [const] Amount of DDI instruction bytes stored in `d_ddi' */
	uint32_t                         d_nstring; /* [const] Amount of static variable names. */
	uint16_t                         d_ddiinit; /* [const] Amount of leading DDI instruction bytes that are used for state initialization */
	uint16_t                         d_pad;     /* ... */
	struct Dee_ddi_regs              d_start;   /* [const] The initial DDI register state. */
	COMPILER_FLEXIBLE_ARRAY(uint8_t, d_ddi);    /* [d_ddisize][const] DDI bytecode (s.a.: `DDI_*') */
};

/* Define a statically allocated DDI object. */
#define Dee_DEFINE_DDI(name, d_strings_, d_strtab_, d_exdat_, d_ddisize_, \
                       d_nstring_, d_ddiinit_, ...)                       \
	struct {                                                              \
		Dee_OBJECT_HEAD                                                   \
		uint32_t const                *d_strings;                         \
		DREF struct Dee_string_object *d_strtab;                          \
		struct Dee_ddi_exdat const    *d_exdat;                           \
		uint32_t                       d_ddisize;                         \
		uint32_t                       d_nstring;                         \
		uint16_t                       d_ddiinit;                         \
		uint16_t                       d_pad;                             \
		struct Dee_ddi_regs            d_start;                           \
		uint8_t                        d_ddi[d_ddisize_];                 \
	} name = {                                                            \
		Dee_OBJECT_HEAD_INIT(&DeeDDI_Type),                               \
		d_strings_,                                                       \
		d_strtab_,                                                        \
		d_exdat_,                                                         \
		d_ddisize_,                                                       \
		d_nstring_,                                                       \
		d_ddiinit_,                                                       \
		0,                                                                \
		__VA_ARGS__                                                       \
	}




/* Query DDI information for a given code address.
 * @param: self:            The code object for which DDI information should be queried.
 * @param: state:     [out] DDI information for the closest checkpoint below `uip'
 * @param: opt_endip: [out] When non-NULL, filled with the UIP of the closest checkpoint above `uip'
 * @param: flags:           Set of `DDI_STATE_F*'
 * @return: * :             Successfully found the DDI state describing `uip'
 * @return: DDI_NEXT_ERR:   [!DDI_STATE_FNOTHROW] An error occurred.
 * @return: DDI_NEXT_DONE:  The DDI information stream has ended after `DDI_STOP' was read. */
DFUNDEF WUNUSED NONNULL((1, 2)) uint8_t *DCALL
DeeCode_FindDDI(DeeObject *__restrict self,
                struct Dee_ddi_state *__restrict start_state,
                Dee_code_addr_t *opt_endip, Dee_code_addr_t uip,
                unsigned int flags);

/* Return the name for a specific assembly symbol which may be found in code. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeCode_GetASymbolName(DeeObject const *__restrict self, uint16_t aid); /* Argument */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeCode_GetRSymbolName(DeeObject const *__restrict self, uint16_t rid); /* Reference */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeCode_GetDDIString(DeeObject const *__restrict self, uint16_t id); /* DDI symbol/local/path/file */

#define DeeCode_NAME(x)                          \
	DeeCode_GetDDIString((DeeObject const *)(x), \
	                     Dee_REQUIRES_OBJECT(DeeCodeObject, x)->co_ddi->d_start.dr_name)

#define DeeDDI_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeDDI_Type) /* `_DDI' is final. */
#define DeeDDI_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeDDI_Type)
DDATDEF DeeTypeObject DeeDDI_Type;

/* A stub ddi-object that contains no actual information. */
#ifndef GUARD_DEEMON_EXECUTE_DDI_C
DDATDEF DeeDDIObject DeeDDI_Empty;
#endif /* !GUARD_DEEMON_EXECUTE_DDI_C */



#ifdef DEE_SOURCE
/* Flags for `DeeCodeObject::co_flags' */
#define CODE_FNORMAL         0x0000          /* Normal code object flags. */
#define CODE_FYIELDING       0x0001          /* The code is part of a yield-function. */
#define CODE_FCOPYABLE       0x0002          /* Allow stackframes generated by this code object to be copied. */
#define CODE_FASSEMBLY       0x0004          /* The assembly of this code object cannot be trusted not to behave
                                              * unexpectedly. This is the case for code object received by
                                              * marshaling, or when custom instruction sequences have been encoded.
                                              * Additionally, `thread.check_interrupt()' is called before every jump
                                              * instruction. */
#define CODE_FLENIENT        0x0008          /* For use with `CODE_FASSEMBLY':
                                              * Leniently accept out-of-bound access to stack variables, dynamically
                                              * adding more memory as necessary when out-of-bound accesses remain
                                              * within ~reasonable~ limits (reasonable being determined at runtime).
                                              * This flag is usually set when the code either contains, or is fully
                                              * written by hand in a manner that can't fully be understood by the compiler. */
#define CODE_FVARARGS        0x0010          /* The code accepts a variable number of arguments, requiring at least `co_argc_min',
                                              * but taking any number greater, with the remaining, unused arguments then being
                                              * accessible as a tuple at argument index == `co_argc_max'.
                                              * Note that this tuple is created on-the-fly and saved in `cf_vargs'. */
#define CODE_FVARKWDS        0x0020          /* The code accepts a variable number of keywords. */
#define CODE_FTHISCALL       0x0040          /* The code must be executed using `thiscall' calling conventions (s.a.: `struct Dee_code_frame::cf_this'). */
#define CODE_FHEAPFRAME      0x0080          /* Frame memory should be allocated on the heap, rather than the stack. */
#define CODE_FFINALLY        0x0100          /* Finally handlers exist that must be executed before `return'.
                                              * WARNING: This flag must only be set when `co_exceptc != 0' */
#define CODE_FCONSTRUCTOR    0x0200          /* Don't track the this-argument, or references to it located in locals, the stack, or arguments when
                                              * generating tracebacks. This is required to prevent the traceback from keeping the this-argument alive
                                              * when an error occurs, which would otherwise cause the constructor wrapper to discard that error on
                                              * the ground of not being able to undo construction. */
#define CODE_FMASK           0x03ff          /* Mask of known code flags. */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#define CODE_FNOOPTIMIZE     0x0400          /* Code object cannot be optimized by `_hostasm' */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#define DEC_CODE_F8BIT       0x8000          /* Used by DEC. - Does not actually appear in runtime code object, but must remain reserved */

/* Threshold of `co_framesize' for `CODE_FHEAPFRAME' */
#define CODE_LARGEFRAME_THRESHOLD 0x100      /* When `co_framesize' turns out to be larger than this,
                                              * it is suggested that the `CODE_FHEAPFRAME' flag be set. */
#endif /* DEE_SOURCE */


#ifdef CONFIG_HAVE_CODE_METRICS
struct Dee_code_metrics {
	size_t com_functions;     /* [lock(atomic)] # of times this code object has been used to construct a function */
	size_t com_call;          /* [lock(atomic)] # of times a function invoked this code object. */
	size_t com_call_kw;       /* [lock(atomic)] # of times a function invoked this code object (w/ keyword arguments). */
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	size_t com_call_tuple;    /* [lock(atomic)] # of times a function invoked this code object (w/ args-tuple). */
	size_t com_call_tuple_kw; /* [lock(atomic)] # of times a function invoked this code object (w/ args-tuple & keyword arguments). */
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DEE_CODE_METRICS_INIT { 0, 0, 0, 0, 0 }
#define Dee_code_metrics_init(self)       \
	(void)((self)->com_functions     = 0, \
	       (self)->com_call          = 0, \
	       (self)->com_call_kw       = 0, \
	       (self)->com_call_tuple    = 0, \
	       (self)->com_call_tuple_kw = 0)
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DEE_CODE_METRICS_INIT { 0, 0, 0 }
#define Dee_code_metrics_init(self)   \
	(void)((self)->com_functions = 0, \
	       (self)->com_call      = 0, \
	       (self)->com_call_kw   = 0)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* CONFIG_HAVE_CODE_METRICS */

#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
struct Dee_hostasm_code_data;
struct Dee_hostasm_function_data;
#ifdef CONFIG_BUILDING_DEEMON
INTDEF NONNULL((1)) void DCALL Dee_hostasm_code_data_destroy(struct Dee_hostasm_code_data *__restrict self);
INTDEF NONNULL((1)) void DCALL Dee_hostasm_function_data_destroy(struct Dee_hostasm_function_data *__restrict self);
#endif /* CONFIG_BUILDING_DEEMON */

struct Dee_hostasm_code {
	/* [0..1][owned][lock(WRITE_ONCE)]
	 * Data blob for holding extra meta-data referenced by compiled host assembly. */
	struct Dee_hostasm_code_data *haco_data;

	/* [0..1][lock(WRITE_ONCE)]
	 * Compiled callbacks for invocation with a function. */
	union {
		DREF DeeObject *(DCALL *c_norm)(DeeFunctionObject *func, size_t argc, DeeObject *const *argv);
		DREF DeeObject *(DCALL *c_this)(DeeFunctionObject *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv);
	} haco_call;
	union {
		DREF DeeObject *(DCALL *c_norm)(DeeFunctionObject *func, size_t argc, DeeObject *const *argv, DeeObject *kw);
		DREF DeeObject *(DCALL *c_this)(DeeFunctionObject *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);
	} haco_call_kw;
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	union {
		DREF DeeObject *(DCALL *c_norm)(DeeFunctionObject *func, DeeObject *args);
		DREF DeeObject *(DCALL *c_this)(DeeFunctionObject *func, DeeObject *thisarg, DeeObject *args);
	} haco_call_tuple;
	union {
		DREF DeeObject *(DCALL *c_norm)(DeeFunctionObject *func, DeeObject *args, DeeObject *kw);
		DREF DeeObject *(DCALL *c_this)(DeeFunctionObject *func, DeeObject *thisarg, DeeObject *args, DeeObject *kw);
	} haco_call_tuple_kw;
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DEE_HOSTASM_CODE_INIT { NULL, { NULL }, { NULL }, { NULL }, { NULL } }
#define Dee_hostasm_code_init(self)                  \
	(void)((self)->haco_data                 = NULL, \
	       (self)->haco_call.c_norm          = NULL, \
	       (self)->haco_call_kw.c_norm       = NULL, \
	       (self)->haco_call_tuple.c_norm    = NULL, \
	       (self)->haco_call_tuple_kw.c_norm = NULL)
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DEE_HOSTASM_CODE_INIT { NULL, { NULL }, { NULL } }
#define Dee_hostasm_code_init(self)            \
	(void)((self)->haco_data           = NULL, \
	       (self)->haco_call.c_norm    = NULL, \
	       (self)->haco_call_kw.c_norm = NULL)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */

struct Dee_hostasm_function {
	/* [0..1][owned][lock(WRITE_ONCE)]
	 * Data blob for holding extra meta-data referenced by compiled host assembly. */
	struct Dee_hostasm_function_data *hafu_data;

	/* [0..1][lock(WRITE_ONCE)]
	 * Compiled callbacks for invocation with a function. */
	union {
		DREF DeeObject *(DCALL *c_norm)(size_t argc, DeeObject *const *argv);
		DREF DeeObject *(DCALL *c_this)(DeeObject *thisarg, size_t argc, DeeObject *const *argv);
	} hafu_call;
	union {
		DREF DeeObject *(DCALL *c_norm)(size_t argc, DeeObject *const *argv, DeeObject *kw);
		DREF DeeObject *(DCALL *c_this)(DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);
	} hafu_call_kw;
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	union {
		DREF DeeObject *(DCALL *c_norm)(DeeObject *args);
		DREF DeeObject *(DCALL *c_this)(DeeObject *thisarg, DeeObject *args);
	} hafu_call_tuple;
	union {
		DREF DeeObject *(DCALL *c_norm)(DeeObject *args, DeeObject *kw);
		DREF DeeObject *(DCALL *c_this)(DeeObject *thisarg, DeeObject *args, DeeObject *kw);
	} hafu_call_tuple_kw;
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DEE_HOSTASM_FUNCTION_INIT { NULL, { NULL }, { NULL }, { NULL }, { NULL } }
#define Dee_hostasm_function_init(self)              \
	(void)((self)->hafu_data                 = NULL, \
	       (self)->hafu_call.c_norm          = NULL, \
	       (self)->hafu_call_kw.c_norm       = NULL, \
	       (self)->hafu_call_tuple.c_norm    = NULL, \
	       (self)->hafu_call_tuple_kw.c_norm = NULL)
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DEE_HOSTASM_FUNCTION_INIT { NULL, { NULL }, { NULL } }
#define Dee_hostasm_function_init(self)        \
	(void)((self)->hafu_data           = NULL, \
	       (self)->hafu_call.c_norm    = NULL, \
	       (self)->hafu_call_kw.c_norm = NULL)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */


struct Dee_code_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD
	uint16_t                 co_flags;       /* Code flags (Set of `CODE_F*') */
	uint16_t                 co_localc;      /* [const] Amount of local variables used by code. */
	uint16_t                 co_constc;      /* [const] Amount of constant variables. */
	uint16_t                 co_refc;        /* [const] Amount of reference variables used by this code. */
	uint16_t                 co_refstaticc;  /* [const][>= co_refc] Amount of references, plus # of extra static variables (that come after references) */
	uint16_t                 co_exceptc;     /* [const] Amount of exception handlers. */
	uint16_t                 co_argc_min;    /* [const] Min amount of arguments required to execute this code. */
	uint16_t                 co_argc_max;    /* [const][>= co_argc_min] Max amount of arguments accepted by this code (excluding a varargs argument). */
	uint32_t                 co_framesize;   /* [const][== (co_localc + X) * sizeof(DeeObject *)]
	                                          * Min amount of bytes of local storage required by assembly of this code object.
	                                          * NOTE: `X' is the minimum stack depth required by this code object. */
	Dee_code_size_t          co_codebytes;   /* [const] The total number of code bytes. */
	union {
		DREF struct Dee_module_object
		                    *co_module;      /* [1..1] The module in which this code object was defined.
		                                      * NOTE: Running code may assume that this field is always non-NULL,
		                                      *       yet during compilation it is NULL or some other object until
		                                      *       the module object surrounding some code object has been defined.
		                                      * HINT: At some point during compilation, this field may also point
		                                      *       to another code object, forming a linked list to track all
		                                      *       code objects of a given module before that module has actually
		                                      *       been created.
		                                      * TODO: This field should be [0..1] and allowed to be NULL if "co_code"
		                                      *       doesn't need the current module in order to run. */
		DREF DeeCodeObject  *co_next;        /* [0..1] Only used during compilation: Pointer to another code object. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define co_module _dee_aunion.co_module
#define co_next   _dee_aunion.co_next
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	DREF struct Dee_string_object      *const *co_keywords; /* [1..1][const][0..co_argc_max || NULL][owned][const] Argument keywords (or NULL if not known). */
	DREF DeeObject                     *const *co_defaultv; /* [0..1][const][0..(co_argc_max-co_argc_min)][owned][const] Vector of default argument values.
	                                                         * NOTE: NULL entries refer to optional arguments, producing an error
	                                                         *       when attempted to be loaded without a user override. */
	DREF DeeObject                     *const *co_constv;   /* [1..1][0..co_constc][owned] Vector of constants. */
	/* NOTE: Exception handlers are execute in order of last -> first, meaning that later handler overwrite prior ones. */
	struct Dee_except_handler                 *co_exceptv;  /* [0..co_excptc][owned] Vector of exception handler descriptors. */
	DREF DeeDDIObject                         *co_ddi;      /* [1..1][const] Debug line information. */
#ifdef CONFIG_HAVE_CODE_METRICS
	struct Dee_code_metrics                    co_metrics;  /* [lock(atomic)] Code metrics */
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	struct Dee_hostasm_code                    co_hostasm;  /* Host assembly functions.  */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	COMPILER_FLEXIBLE_ARRAY(Dee_instruction_t, co_code);    /* [co_codebytes][const] The actual instructions encoding this code object's behavior.
	                                                         * WARNING: The safe code executor assumes that the code vector does not
	                                                         *          stop prematurely, or is capable of exceeding its natural ending
	                                                         *          by means of simply continuing execution and overflowing past the end.
	                                                         *          To prevent problems arising from this, unchecked/unpredictable code
	                                                         *          should be padded with `INSTRLEN_MAX' repeated `ASM_RET_NONE' instructions,
	                                                         *          thereby ensuring that upon natural completion, at least one `ASM_RET_NONE'
	                                                         *          instruction is always executed, no matter what. */
};

#define DeeCode_Malloc(co_codebytes)          ((DREF DeeCodeObject *)DeeObject_Malloc(offsetof(DeeCodeObject, co_code) + (co_codebytes)))
#define DeeCode_Realloc(ptr, co_codebytes)    ((DREF DeeCodeObject *)DeeObject_Realloc(ptr, offsetof(DeeCodeObject, co_code) + (co_codebytes)))
#define DeeCode_TryMalloc(co_codebytes)       ((DREF DeeCodeObject *)DeeObject_TryMalloc(offsetof(DeeCodeObject, co_code) + (co_codebytes)))
#define DeeCode_TryRealloc(ptr, co_codebytes) ((DREF DeeCodeObject *)DeeObject_TryRealloc(ptr, offsetof(DeeCodeObject, co_code) + (co_codebytes)))
#define DeeCode_Free(ptr)                     DeeObject_Free(ptr)

#ifdef CONFIG_HAVE_CODE_METRICS
#define _DEE_CODE_CO_METRICS_FIELD struct Dee_code_metrics co_metrics;
#define _DEE_CODE_CO_METRICS_INIT  DEE_CODE_METRICS_INIT,
#else /* CONFIG_HAVE_CODE_METRICS */
#define _DEE_CODE_CO_METRICS_FIELD /* nothing */
#define _DEE_CODE_CO_METRICS_INIT  /* nothing */
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#define _DEE_CODE_CO_HOSTASM_FIELD struct Dee_hostasm_code co_hostasm;
#define _DEE_CODE_CO_HOSTASM_INIT  DEE_HOSTASM_CODE_INIT,
#else /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#define _DEE_CODE_CO_HOSTASM_FIELD /* nothing */
#define _DEE_CODE_CO_HOSTASM_INIT  /* nothing */
#endif /* !CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */


/* Define a statically allocated code object. */
#define Dee_DEFINE_CODE(name, co_flags_, co_localc_, co_constc_,   \
                        co_refc_, co_refstaticc_, co_exceptc_,     \
                        co_argc_min_, co_argc_max_, co_framesize_, \
                        co_codebytes_, co_module_, co_keywords_,   \
                        co_defaultv_, co_staticv_, co_exceptv_,    \
                        co_ddi_, ...)                              \
	struct {                                                       \
		Dee_OBJECT_HEAD                                            \
		uint16_t                              co_flags;            \
		uint16_t                              co_localc;           \
		uint16_t                              co_constc;           \
		uint16_t                              co_refc;             \
		uint16_t                              co_refstaticc;       \
		uint16_t                              co_exceptc;          \
		uint16_t                              co_argc_min;         \
		uint16_t                              co_argc_max;         \
		uint32_t                              co_framesize;        \
		Dee_code_size_t                       co_codebytes;        \
		DREF struct Dee_module_object        *co_module;           \
		DREF struct Dee_string_object *const *co_keywords;         \
		DREF DeeObject                *const *co_defaultv;         \
		DREF DeeObject                *const *co_constv;           \
		struct Dee_except_handler            *co_exceptv;          \
		DREF DeeDDIObject                    *co_ddi;              \
		_DEE_CODE_CO_METRICS_FIELD                                 \
		_DEE_CODE_CO_HOSTASM_FIELD                                 \
		Dee_instruction_t co_code[co_codebytes_];                  \
	} name = {                                                     \
		Dee_OBJECT_HEAD_INIT(&DeeCode_Type),                       \
		co_flags_,                                                 \
		co_localc_,                                                \
		co_constc_,                                                \
		co_refc_,                                                  \
		co_refstaticc_,                                            \
		co_exceptc_,                                               \
		co_argc_min_,                                              \
		co_argc_max_,                                              \
		co_framesize_,                                             \
		co_codebytes_,                                             \
		co_module_,                                                \
		co_keywords_,                                              \
		co_defaultv_,                                              \
		co_staticv_,                                               \
		co_exceptv_,                                               \
		co_ddi_,                                                   \
		_DEE_CODE_CO_METRICS_INIT                                  \
		_DEE_CODE_CO_HOSTASM_INIT                                  \
		__VA_ARGS__                                                \
	}




/* Returns the max number of stack objects */
#define DeeCode_StackDepth(x) \
	(((x)->co_framesize / sizeof(DeeObject *)) - (x)->co_localc)

#ifdef CONFIG_BUILDING_DEEMON
#ifndef GUARD_DEEMON_EXECUTE_CODE_C
/* A stub code-object that contains a single, `ret none' instruction. */
INTDEF DeeCodeObject DeeCode_Empty;
#endif /* !GUARD_DEEMON_EXECUTE_CODE_C */
#endif /* CONFIG_BUILDING_DEEMON */

DDATDEF DeeTypeObject DeeCode_Type;
#define DeeCode_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeCode_Type) /* `Code' is final */
#define DeeCode_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeCode_Type) /* `Code' is final */


/* Attempts to set the assembly flag of the given code object if it wasn't set already.
 * Code objects with this flag set are robust to otherwise unexpected behavior, but
 * safely setting this flag usually proves to be quite difficult as it involves ensuring
 * that the code isn't already being executed by some other thread.
 * For that reason, this helper function exists, which will do whatever is necessary to
 * ensure that the code isn't running or is apart of any of the execution stacks of other
 * threads, temporarily suspending the execution of all threads but the caller's.
 * This however is just an implementation detail that the caller must not concern themselves with.
 * What the caller should be concerned about however is the error-return case:
 *     In the event that the given code object is actively being executed, either by
 *     the calling, or by any other thread, a ValueError is thrown and -1 is returned.
 *     Otherwise when 0 is returned, the caller may assume that `CODE_FASSEMBLY' has
 *     been set, and that modifying `co_code' to their liking, while still subject
 *     to potential code-tearing, as well as the resulting inconsistencies that may
 *     cause running code to throw errors, but not cause the interpreter to crash.
 * Note that this function may also fail because an interrupt was send to the calling thread!
 * @return: 0 : Success
 * @return: -1: Error */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeCode_SetAssembly(/*Code*/ DeeObject *__restrict self);


/* Extended frame information for functions invoked with keyword arguments.
 * >> function foo(x?, y?, z?, **w) { }
 * >> foo(10, z: 20, foobar: "barfoo");
 * VALUES:
 *    cf_argc         = 1
 *    cf_argv         = { int(10) }     (with out-of-bounds extension `{ int(20), string("barfoo") }')
 *    cf_kw->fk_kwds  = <SPECIAL_MAPPING_TYPE> { "foobar" : "barfoo" }  (generated upon first use)
 *    cf_kw->fk_kw    = DeeKwdsObject { "z": 0, "foobar": 1 }
 *    cf_kw->fk_kargv = { NULL, int(20) }
 * Position argument lookup then functions as follows:
 * >> GET_ARG(i):
 * >>    DeeObject *result;
 * >>    Dee_ASSERT(i < code->co_argc_max);
 * >>    if (i < frame->cf_argc)
 * >>        return frame->cf_argv[i]; // Regular, positional arguments
 * >>    if (frame->cf_kw) {
 * >>        result = frame->cf_kw->fk_kargv[i - frame->cf_argc];
 * >>        if (result)
 * >>            return result;
 * >>    }
 * >>    Dee_ASSERT(i >= code->co_argc_min);
 * >>    result = code->co_defaultv[i - code->co_argc_min];
 * >>    if (result)
 * >>        return result;
 * >>    ERROR_UNBOUND_ARGUMENT(i);
 */
struct Dee_code_frame_kwds {
	DREF DeeObject                      *fk_varkwds; /* [0..1][valid_if(:cf_func->fo_code->co_flags & CODE_FVARKWDS)]
	                                                  * [lock(WRITE_ONCE)] Variable keyword arguments.
	                                                  * NOTE: May only be accessed by a code interpreter when the associated
	                                                  *       code object has the `CODE_FVARKWDS' flag set (otherwise, this
	                                                  *       field may not actually exist)
	                                                  * WARNING: Certain object types which can appear in this field require
	                                                  *          special actions to be taken before being decref'd by their
	                                                  *          creator / stack-owner. */
	DREF DeeObject                      *fk_kw;      /* [1..1][const] The original `kw' object that was passed to the function.
	                                                  * NOTE: When this is a DeeKwdsObject, its values are mapped to `:cf_argv + :cf_argc',
	                                                  *       aka. at the end of the standard-accessible argument vector.
	                                                  * NOTE: May only be accessed by a code interpreter when the associated
	                                                  *       code object has the `CODE_FVARKWDS' flag set (otherwise, this
	                                                  *       field may not actually exist) */
	COMPILER_FLEXIBLE_ARRAY(DeeObject *, fk_kargv);  /* [0..1][const][1..(:cf_func->fo_code->co_argc_max - :cf_argc)]
	                                                  * Overlay of additional, non-positional arguments which were
	                                                  * passed via keyword arguments. */
};


#define Dee_CODE_FRAME_NOT_EXECUTING  ((struct Dee_code_frame *)(uintptr_t)-1)
#ifdef DEE_SOURCE
#define CODE_FRAME_NOT_EXECUTING      Dee_CODE_FRAME_NOT_EXECUTING
#endif /* DEE_SOURCE */

/* Execution frame of a deemon code object.
 * NOTE: This structure is usually allocated on the host's stack. */
struct Dee_code_frame {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	struct Dee_code_frame    *cf_prev;    /* [0..1] Previous execution frame.
	                                       * NOTE: Set to `CODE_FRAME_NOT_EXECUTING' while
	                                       *       the frame is not being executed. */
	DeeFunctionObject        *cf_func;    /* [1..1] The function running within in this frame. */
	size_t                    cf_argc;    /* [const] Amount of input arguments. */
	DREF DeeObject    *const *cf_argv;    /* [1..1][const][0..cf_argc][const] Vector of input arguments. */
	struct Dee_code_frame_kwds
	                         *cf_kw;      /* [0..1][const] Keyword argument extension data. */
	DREF DeeObject          **cf_frame;   /* [0..1][cf_func->fo_code->co_framesize / sizeof(DeeObject *)][owned] Frame-local work-memory used during execution. */
	DREF DeeObject          **cf_stack;   /* [?..1][(cf_func->fo_code->co_framesize - cf_func->fo_code->co_localc * sizeof(DeeObject *)) / sizeof(DeeObject *)] Base address for the stack.
	                                       * NOTE: When `cf_stacksz != 0', then this vector is allocated on the heap. */
	DeeObject               **cf_sp;      /* [?..1][1..1] Pointer to the location where the next-to-be pushed object is stored.
	                                       * NOTE: The stack pointer grows UPWARDS, meaning that
	                                       *       the used object-range is `[cf_stack, cf_sp)' */
	Dee_instruction_t        *cf_ip;      /* [1..1][in(cf_func->fo_code->co_code)] Current instruction pointer. */
	DREF struct Dee_tuple_object
	                         *cf_vargs;   /* [0..1][lock(write_once)] Saved var-args object. */
	DeeObject                *cf_this;    /* [1..1][valid_if(cf_func->fo_code->co_flags & CODE_FTHISCALL)][const]
	                                       * The `this' argument passed for this-calls. */
	DeeObject                *cf_result;  /* [0..1] Storage location of the frame's currently set return value.
	                                       *        The caller of the frame should pre-initialize this field to NULL.
	                                       * NOTE: May be set to ITER_DONE in yield functions! */
	uint16_t                  cf_stacksz; /* [valid_if(DeeCode_ExecFrameSafe)] Size of the heap-allocated stack.
	                                       * HINT: This field is not used by code running in fast mode
	                                       *       (aka. Code without the `CODE_FASSEMBLY' flag set). */
	uint16_t                  cf_flags;   /* Frame flags (Only used by yield-function-iterators; set of `CODE_F*') */
#if __SIZEOF_POINTER__ > 4
	uint16_t                  cf_padding[2]; /* ... */
#endif /* __SIZEOF_POINTER__ > 4 */
};

#define Dee_code_frame_getipaddr(self)       (Dee_code_addr_t)((self)->cf_ip - (self)->cf_func->fo_code->co_code)
#define Dee_code_frame_setipaddr(self, addr) (void)((self)->cf_ip = (self)->cf_func->fo_code->co_code + (addr))
#define Dee_code_frame_getspaddr(self)       (uint16_t)((self)->cf_sp - (self)->cf_stack)
#define Dee_code_frame_setspaddr(self, addr) (void)((self)->cf_sp = (self)->cf_stack + (addr))


/* Continue execution of the given code frame until it returns.
 * NOTE: `DeeCode_ExecFrameFast' should be used unless the `CODE_FASSEMBLY'
 *       flag is set in the code object associated with the frame.
 * @return: * :         A new reference to the object returned by the code.
 * @return: NULL:       An error occurred that could not be handled by user-code.
 * @return: ITER_DONE: [CODE_FYIELDING] The iterator of the frame as finished.
 *                HINT: Attempting to execute the frame again will yield `ITER_DONE' once more. */
DFUNDEF NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameFast(struct Dee_code_frame *__restrict frame);
DFUNDEF NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameSafe(struct Dee_code_frame *__restrict frame);


#if defined(__i386__) || defined(__x86_64__)
#define CONFIG_HAVE_EXEC_ALTSTACK
#endif /* __i386__ || __x86_64__ */

#ifdef CONFIG_NO_EXEC_ALTSTACK
#undef CONFIG_HAVE_EXEC_ALTSTACK
#endif /* CONFIG_NO_EXEC_ALTSTACK */

#ifdef CONFIG_HAVE_EXEC_ALTSTACK

#ifndef DEE_EXEC_ALTSTACK_PERIOD
#if defined(__i386__)
#define DEE_EXEC_ALTSTACK_PERIOD  1024 /* NOTE: Changes must be mirrored in `exec.gas-386.S' */
#elif defined(__x86_64__)
#define DEE_EXEC_ALTSTACK_PERIOD  1024
#endif
#endif /* !DEE_EXEC_ALTSTACK_PERIOD */

/* Switch to an alternate stack every `DEE_EXEC_ALTSTACK_PERIOD' recursions. */
#ifndef DEE_EXEC_ALTSTACK_PERIOD
#define DEE_EXEC_ALTSTACK_PERIOD  1024
#endif /* !DEE_EXEC_ALTSTACK_PERIOD */

#ifndef DEE_EXEC_ALTSTACK_SIZE
 /* NOTE: Changes must be mirrored in `altstack.ms-x64.S' */
#define DEE_EXEC_ALTSTACK_SIZE (__SIZEOF_POINTER__ * 1024 * 512)
#endif /* !DEE_EXEC_ALTSTACK_SIZE */

/* Same as the functions above, however execute the frame
 * on a new, heap-allocated stack, before switching back
 * to the caller's original stack before returning.
 * These functions are highly platform- and arch-specific, and are meant
 * to provide some way of preventing a true stack overflow when user-code
 * has increased `DeeExec_StackLimit' to unreasonable heights. */
DFUNDEF NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameFastAltStack(struct Dee_code_frame *__restrict frame);
DFUNDEF NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameSafeAltStack(struct Dee_code_frame *__restrict frame);
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */



/* Handle a breakpoint having been triggered in `frame'.
 * NOTE: This function is called to deal with an encounter
 *       of a breakpoint during execution of code.
 * @param: frame: [in|out][OVERRIDE(->cf_result, DREF)]
 *                The execution frame that triggered the breakpoint.
 *                Anything and everything about the state described
 *                within this frame is subject to change by this
 *                function, including PC/SP, as well as the running
 *                code itself.
 *                 - The stack pointer is the stack location as
 *                   it is at the breakpoint instruction, as well
 *                   as the instruction following thereafter.
 *                 - The instruction pointer points to the instruction
 *                   following the breakpoint, meaning that no further
 *                   adjustment is required if all that's supposed to
 *                   happen is execution continuing normally.
 *                 - The valid stack size is always stored in `cf_stacksz'
 * @return: * :   One of `TRIGGER_BREAKPOINT_*' describing how execution
 *                should continue once the breakpoint has been dealt with. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeCode_HandleBreakpoint(struct Dee_code_frame *__restrict frame);

/* Breakpoint execution modes. */
#ifdef DEE_SOURCE
#define TRIGGER_BREAKPOINT_EXCEPT_EXIT (-2) /* Similar to `TRIGGER_BREAKPOINT_EXCEPT', but don't execute exception handlers in the calling code. */
#define TRIGGER_BREAKPOINT_EXCEPT   (-1) /* Handle an exception triggered at `frame->cf_ip' (new value when changed) */
#define TRIGGER_BREAKPOINT_CONTINUE   0  /* Continue execution normally at `frame->cf_ip' (new value when changed) */
#define TRIGGER_BREAKPOINT_CONTSAFE   1  /* Same as `TRIGGER_BREAKPOINT_CONTINUE', but if execution was running in
                                          * fast-mode (`DeeCode_ExecFrameFast()'), continue after switching to safe-mode.
                                          * This value should be returned if unpredictable changes were made to the
                                          * execution context that may result in the frame becoming unstable.
                                          * Such unpredictable changes include:
                                          *   - Adding/removing/modifying any object located on the stack:
                                          *     - Add/remove: The stack-depth may have become unstable.
                                          *     - Modify:     The modified value may have been designated for use by `ASM_JMP_POP',
                                          *                   or some other instruction expecting to find a specific type of object.
                                          *   - Changing the running assembly in an unpredictable manner.
                                          *     - Obviously changing the assembly can be dangerous...
                                          *   - Changing the running PC
                                          *     - The code located at the new PC may expect a different stack-depth.
                                          * WARNING: Any sort of changes made to PC/SP must be validated by the caller.
                                          *          The breakpoint instruction will not perform such checks, as it is
                                          *          expected that whoever hooked the breakpoint knows what they're doing. */
#define TRIGGER_BREAKPOINT_RETURN     2  /* Yield/Return from the calling function normally.
                                          * The breakpoint library may assign a reference to-be returned to
                                          * `cf_result' before executing this instruction, though if no value
                                          * has been assigned (`cf_result == NULL'), the interpreter will simply
                                          * return `none' (or signal `ITER_DONE' in yield-functions) instead. */
#define TRIGGER_BREAKPOINT_EXIT       3  /* Same as `TRIGGER_BREAKPOINT_RETURN' for non-yielding functions,
                                          * yet for yielding functions, exit from the function by returning
                                          * `ITER_DONE' after decref()-ing `cf_result' if it is
                                          * neither `NULL' nor `ITER_DONE'
                                          * WARNING: When this code is used to indicate the end of an iterator,
                                          *          it should be noted that attempting to yield the iterator
                                          *          again will once again execute code, potentially leading to
                                          *          a scenario where an iterator that already claimed to have
                                          *          been exhausted can once again be spun. */
#define TRIGGER_BREAKPOINT_EXIT_NOFIN 4  /* Same as `TRIGGER_BREAKPOINT_EXIT', but don't execute
                                          * finally handlers and immediately return to the caller. */
#endif /* DEE_SOURCE */


struct Dee_function_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD /* GC Object. */
	DREF DeeCodeObject                       *fo_code;    /* [1..1][const] Associated code object. */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	struct Dee_hostasm_function               fo_hostasm; /* Host assembly functions.  */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                       fo_reflock; /* Lock used by `ASM_STATIC', `ASM_PUSH_STATIC' and `ASM_POP_STATIC' instructions when accessing `fo_refv'. */
#endif /* !CONFIG_NO_THREADS */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, fo_refv);   /* [if(. <  fo_code->co_refc), [1..1, const]]
	                                                       * [if(. >= fo_code->co_refc), [0..1|ITER_DONE, lock(fo_reflock)]]
	                                                       * [fo_code->co_refstaticc]
	                                                       * Vector of referenced objects & static variables
	                                                       * Values may be set to ITER_DONE by `ASM_CMPXCH_UB_LOCK'. */
};

#define DeeFunction_CODE(x) Dee_REQUIRES_OBJECT(DeeFunctionObject const, x)->fo_code
#define DeeFunction_REFV(x) Dee_REQUIRES_OBJECT(DeeFunctionObject const, x)->fo_refv

#define DeeFunction_RefLockReading(self)    Dee_atomic_rwlock_reading(&(self)->fo_reflock)
#define DeeFunction_RefLockWriting(self)    Dee_atomic_rwlock_writing(&(self)->fo_reflock)
#define DeeFunction_RefLockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->fo_reflock)
#define DeeFunction_RefLockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->fo_reflock)
#define DeeFunction_RefLockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->fo_reflock)
#define DeeFunction_RefLockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->fo_reflock)
#define DeeFunction_RefLockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->fo_reflock)
#define DeeFunction_RefLockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->fo_reflock)
#define DeeFunction_RefLockRead(self)       Dee_atomic_rwlock_read(&(self)->fo_reflock)
#define DeeFunction_RefLockWrite(self)      Dee_atomic_rwlock_write(&(self)->fo_reflock)
#define DeeFunction_RefLockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->fo_reflock)
#define DeeFunction_RefLockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->fo_reflock)
#define DeeFunction_RefLockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->fo_reflock)
#define DeeFunction_RefLockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->fo_reflock)
#define DeeFunction_RefLockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->fo_reflock)
#define DeeFunction_RefLockEnd(self)        Dee_atomic_rwlock_end(&(self)->fo_reflock)


#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#define _DEE_FUNCTION_FO_HOSTASM_FIELD struct Dee_hostasm_function fo_hostasm;
#define _DEE_FUNCTION_FO_HOSTASM_INIT  DEE_HOSTASM_FUNCTION_INIT,
#else /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#define _DEE_FUNCTION_FO_HOSTASM_FIELD /* nothing */
#define _DEE_FUNCTION_FO_HOSTASM_INIT  /* nothing */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
#ifndef CONFIG_NO_THREADS
#define _DEE_FUNCTION_FO_REFLOCK_FIELD Dee_atomic_rwlock_t fo_reflock;
#define _DEE_FUNCTION_FO_REFLOCK_INIT  Dee_ATOMIC_RWLOCK_INIT,
#else /* !CONFIG_NO_THREADS */
#define _DEE_FUNCTION_FO_REFLOCK_FIELD /* nothing */
#define _DEE_FUNCTION_FO_REFLOCK_INIT  /* nothing */
#endif /* CONFIG_NO_THREADS */

#define Dee_DEFINE_FUNCTION(name, fo_code_, fo_refc_, ...) \
	struct {                                               \
		struct gc_head_link _gc_head_data;                 \
		struct {                                           \
			Dee_OBJECT_HEAD                                \
			DREF DeeCodeObject *fo_code;                   \
			_DEE_FUNCTION_FO_HOSTASM_FIELD                 \
			_DEE_FUNCTION_FO_REFLOCK_FIELD                 \
			DREF DeeObject *fo_refv[fo_refc_];             \
		} ob;                                              \
	} name = {                                             \
		{ NULL, NULL },                                    \
		{ Dee_OBJECT_HEAD_INIT(&DeeFunction_Type),         \
		  fo_code_,                                        \
		  _DEE_FUNCTION_FO_HOSTASM_INIT                    \
		  _DEE_FUNCTION_FO_REFLOCK_INIT                    \
		  __VA_ARGS__ }                                    \
	}
#define Dee_DEFINE_FUNCTION_NOREFS(name, fo_code_) \
	struct {                                       \
		struct gc_head_link _gc_head_data;         \
		struct {                                   \
			Dee_OBJECT_HEAD                        \
			DREF DeeCodeObject *fo_code;           \
			_DEE_FUNCTION_FO_HOSTASM_FIELD         \
			_DEE_FUNCTION_FO_REFLOCK_FIELD         \
		} ob;                                      \
	} name = {                                     \
		{ NULL, NULL },                            \
		{ Dee_OBJECT_HEAD_INIT(&DeeFunction_Type), \
		  fo_code_,                                \
		  _DEE_FUNCTION_FO_HOSTASM_INIT            \
		  _DEE_FUNCTION_FO_REFLOCK_INIT }          \
	}
	



struct Dee_yield_function_object {
	Dee_OBJECT_HEAD 
	DREF DeeFunctionObject                   *yf_func;  /* [1..1][const] The function we are derived from. */
	struct Dee_code_frame_kwds               *yf_kw;    /* [0..1][owned][const] Keyword arguments. */
	DREF DeeObject                           *yf_this;  /* [0..1][const] 'this' object during callback. */
	size_t                                    yf_pargc; /* [<= yf_argc][const] Positional argument count */
	size_t                                    yf_argc;  /* [const] Argument count (including keyword values when `DeeKwds_Check(yf_kw->fk_kw)') */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, yf_argv); /* [1..1][const][yf_argc] Argument vector*/
};

#define Dee_DEFINE_YIELD_FUNCTION(name, yf_func_, yf_kw_, \
                                  yf_this_, yf_pargc_,    \
                                  yf_argc_, ...)          \
	struct {                                              \
		Dee_OBJECT_HEAD                                   \
		DREF DeeFunctionObject     *yf_func;              \
		struct Dee_code_frame_kwds *yf_kw;                \
		DREF DeeObject             *yf_this;              \
		size_t                      yf_pargc;             \
		size_t                      yf_argc;              \
		DREF DeeObject             *yf_argv[yf_argc_];    \
	} name = {                                            \
		Dee_OBJECT_HEAD_INIT(&DeeYieldFunction_Type),     \
		yf_func_, yf_kw_, yf_this_, yf_pargc_, yf_argc_,  \
		__VA_ARGS__                                       \
	}
#define Dee_DEFINE_YIELD_FUNCTION_NOARGS(name, yf_func_, yf_kw_, yf_this_) \
	struct {                                                               \
		Dee_OBJECT_HEAD                                                    \
		DREF DeeFunctionObject     *yf_func;                               \
		struct Dee_code_frame_kwds *yf_kw;                                 \
		DREF DeeObject             *yf_this;                               \
		size_t                      yf_pargc;                              \
		size_t                      yf_argc;                               \
	} name = {                                                             \
		Dee_OBJECT_HEAD_INIT(&DeeYieldFunction_Type),                      \
		yf_func_, yf_kw_, yf_this_, 0, 0                                   \
	}


#ifdef CONFIG_BUILDING_DEEMON
#define DeeYieldFunction_Sizeof(argc) (offsetof(DeeYieldFunctionObject, yf_argv) + ((argc) * sizeof(DREF DeeObject *)))
#endif /* CONFIG_BUILDING_DEEMON */

struct Dee_yield_function_iterator_object {
	Dee_OBJECT_HEAD /* GC Object. */
	DREF DeeYieldFunctionObject *yi_func;  /* [0..1][lock(yi_lock)] The yield function instance that created us.
	                                        * NOTE: May be set to `NULL' when the iterator is cleared by the GC. */
	struct Dee_code_frame        yi_frame; /* [lock(yi_lock)]
	                                        * [owned(.cf_frame)]
	                                        * [owned_if(.cf_stack, .cf_stacksz)]
	                                        * [OVERRIDE(.cf_func, DREF [0..1][(!= NULL) == (:yi_func != NULL)])]
	                                        * [OVERRIDE(.cf_argv, DREF [0..1][(!= NULL) == (:yi_func != NULL)])]
	                                        * [OVERRIDE(.cf_ip, [?..1][valid_if(:yi_func != NULL)])]
	                                        * [OVERRIDE(.cf_this, DREF [0..1])]
	                                        * [.cf_argc == yi_func->yf_argc || 0]
	                                        * [.cf_argv == yi_func->yf_argv || NULL]
	                                        * [.cf_this == yi_func->yf_this || NULL]
	                                        * Execution frame of this iterator. */
#ifndef CONFIG_NO_THREADS
	Dee_rshared_rwlock_t         yi_lock;  /* Lock held while executing the frame of this iterator.
	                                        * NOTE: This lock needs to be recursive to allow for
	                                        *       GC-visit/frame-copy while the frame is executing. */
#endif /* !CONFIG_NO_THREADS */
};

#define DeeYieldFunctionIterator_LockReading(self)        Dee_rshared_rwlock_reading(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockWriting(self)        Dee_rshared_rwlock_writing(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockTryRead(self)        Dee_rshared_rwlock_tryread(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockTryWrite(self)       Dee_rshared_rwlock_trywrite(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockCanRead(self)        Dee_rshared_rwlock_canread(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockCanWrite(self)       Dee_rshared_rwlock_canwrite(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockWaitRead(self)       Dee_rshared_rwlock_waitread(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockWaitReadNoInt(self)  Dee_rshared_rwlock_waitread_noint(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockWaitWrite(self)      Dee_rshared_rwlock_waitwrite(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockWaitWriteNoInt(self) Dee_rshared_rwlock_waitwrite_noint(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockRead(self)           Dee_rshared_rwlock_read(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockReadNoInt(self)      Dee_rshared_rwlock_read_noint(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockWrite(self)          Dee_rshared_rwlock_write(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockWriteNoInt(self)     Dee_rshared_rwlock_write_noint(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockTryUpgrade(self)     Dee_rshared_rwlock_tryupgrade(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockUpgrade(self)        Dee_rshared_rwlock_upgrade(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockUpgradeNoInt(self)   Dee_rshared_rwlock_upgrade_noint(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockDowngrade(self)      Dee_rshared_rwlock_downgrade(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockEndWrite(self)       Dee_rshared_rwlock_endwrite(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockEndRead(self)        Dee_rshared_rwlock_endread(&(self)->yi_lock)
#define DeeYieldFunctionIterator_LockEnd(self)            Dee_rshared_rwlock_end(&(self)->yi_lock)

DDATDEF DeeTypeObject DeeFunction_Type;              /* foo; */
DDATDEF DeeTypeObject DeeYieldFunction_Type;         /* foo(); */
DDATDEF DeeTypeObject DeeYieldFunctionIterator_Type; /* foo().operator iter(); */
#define DeeFunction_Check(ob)                   DeeObject_InstanceOfExact(ob, &DeeFunction_Type) /* `Function' is final */
#define DeeFunction_CheckExact(ob)              DeeObject_InstanceOfExact(ob, &DeeFunction_Type)
#define DeeYieldFunction_Check(ob)              DeeObject_InstanceOfExact(ob, &DeeYieldFunction_Type) /* `YieldFunction' is final */
#define DeeYieldFunction_CheckExact(ob)         DeeObject_InstanceOfExact(ob, &DeeYieldFunction_Type)
#define DeeYieldFunctionIterator_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeYieldFunctionIterator_Type) /* `YieldFunction.Iterator' is final */
#define DeeYieldFunctionIterator_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeYieldFunctionIterator_Type)


/* Create a new function object. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_New(DeeCodeObject *code, size_t refc,
                DeeObject *const *refv);

#ifndef Dee_operator_t_DEFINED
#define Dee_operator_t_DEFINED
typedef uint16_t Dee_operator_t;
#endif /* !Dee_operator_t_DEFINED */

struct Dee_function_info {
	DREF DeeTypeObject            *fi_type;   /* [0..1] The type as part of which a function is implemented. */
	DREF struct Dee_string_object *fi_name;   /* [0..1] The name of the function. */
	DREF struct Dee_string_object *fi_doc;    /* [0..1] A documentation string for the function. */
	Dee_operator_t                 fi_opname; /* When the function is implementing an operator, the name of that operator.
	                                           * Otherwise, this field is set to `(Dee_operator_t)-1'
	                                           * NOTE: When this field is set, `fi_name' is usually set to `NULL' */
	uint16_t                       fi_getset; /* When the function is a getset callback, one of `CLASS_GETSET_*'.
	                                           * Otherwise, this field is set to `(uint16_t)-1' */
};
#define Dee_function_info_fini(x) \
	(Dee_XDecref((x)->fi_type),   \
	 Dee_XDecref((x)->fi_name),   \
	 Dee_XDecref((x)->fi_doc))

/* Search for information surrounding the given function/code object(s).
 * Information that cannot be determined is filled in as described by the comments above.
 * @return:  0: The function could be located (though which information became available must still be checked)
 * @return:  1: The function couldn't be found (all fields in `info' are set to indicate <unknown>)
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeFunction_GetInfo(/*Function*/ DeeObject *__restrict self,
                    /*[out]*/ struct Dee_function_info *__restrict info);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeCode_GetInfo(/*Code*/ DeeObject *__restrict self,
                /*[out]*/ struct Dee_function_info *__restrict info);



#if defined(CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE) && defined(CONFIG_HAVE_CODE_METRICS)
/* Get/set the threshold after which a code object
 * gets automatically recompiled into host assembly.
 *
 * Special values:
 * - 0 :         Functions are always optimized immediately.
 * - (size_t)-1: Functions are never optimized (when trying to
 *               optimize a function, and doing so fails because
 *               `_hostasm' can't be loaded, this value gets set
 *               automatically) */
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeCode_GetOptimizeCallThreshold(void);
DFUNDEF size_t DCALL DeeCode_SetOptimizeCallThreshold(size_t new_threshold);

#ifdef CONFIG_BUILDING_DEEMON
INTDEF size_t DeeCode_OptimizeCallThreshold;

INTDEF NONNULL((1)) DREF DeeObject *DCALL DeeFunction_OptimizeAndCall(DeeFunctionObject *self, size_t argc, DeeObject *const *argv);
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL DeeFunction_OptimizeAndThisCall(DeeFunctionObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv);
INTDEF NONNULL((1, 4)) DREF DeeObject *DCALL DeeFunction_OptimizeAndCallKw(DeeFunctionObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeFunction_OptimizeAndThisCallKw(DeeFunctionObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL DeeFunction_OptimizeAndCallTuple(DeeFunctionObject *self, DeeObject *args);
INTDEF NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeFunction_OptimizeAndThisCallTuple(DeeFunctionObject *self, DeeObject *thisarg, DeeObject *args);
INTDEF NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeFunction_OptimizeAndCallTupleKw(DeeFunctionObject *self, DeeObject *args, DeeObject *kw);
INTDEF NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeFunction_OptimizeAndThisCallTupleKw(DeeFunctionObject *self, DeeObject *thisarg, DeeObject *args, DeeObject *kw);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* CONFIG_BUILDING_DEEMON */
#else /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE && CONFIG_HAVE_CODE_METRICS */
#define DeeCode_GetOptimizeCallThreshold()              ((size_t)-1)
#define DeeCode_SetOptimizeCallThreshold(new_threshold) ((size_t)-1)
#endif /* !CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE || !CONFIG_HAVE_CODE_METRICS */


#ifndef CONFIG_BUILDING_DEEMON
#define DeeFunction_NewNoRefs(code)                            DeeFunction_New(code, 0, NULL)
#define DeeFunction_ThisCall(self, this_arg, argc, argv)       DeeObject_ThisCall(Dee_AsObject(self), this_arg, argc, argv)
#define DeeFunction_ThisCallKw(self, this_arg, argc, argv, kw) DeeObject_ThisCallKw(Dee_AsObject(self), this_arg, argc, argv, kw)
#define DeeFunction_CallTuple(self, args)                      DeeObject_CallTuple(Dee_AsObject(self), args)
#define DeeFunction_CallTupleKw(self, args, kw)                DeeObject_CallTupleKw(Dee_AsObject(self), args, kw)
#define DeeFunction_ThisCallTuple(self, this_arg, args)        DeeObject_ThisCallTuple(Dee_AsObject(self), this_arg, args)
#define DeeFunction_ThisCallTupleKw(self, this_arg, args, kw)  DeeObject_ThisCallTupleKw(Dee_AsObject(self), this_arg, args, kw)
#else /* !CONFIG_BUILDING_DEEMON */
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeFunction_NewInherited(DeeCodeObject *code, size_t refc,
                         /*inherit(on_success)*/ DREF DeeObject *const *__restrict refv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_NewNoRefs(DeeCodeObject *__restrict code);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_Call(DeeFunctionObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_CallKw(DeeFunctionObject *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw);

/* Optimized operator for calling a `function' object using the `thiscall' calling convention.
 * NOTE: Potentially required conversions are performed by this function automatically! */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeFunction_ThisCall(DeeFunctionObject *self, DeeObject *this_arg,
                     size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeFunction_ThisCallKw(DeeFunctionObject *self, DeeObject *this_arg,
                       size_t argc, DeeObject *const *argv,
                       DeeObject *kw);

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeFunction_CallTuple(DeeFunctionObject *self,
                      DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeFunction_CallTupleKw(DeeFunctionObject *self,
                        DeeObject *args,
                        DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeFunction_ThisCallTuple(DeeFunctionObject *self,
                          DeeObject *this_arg,
                          DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeFunction_ThisCallTupleKw(DeeFunctionObject *self,
                            DeeObject *this_arg,
                            DeeObject *args,
                            DeeObject *kw);
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DeeFunction_CallTuple(self, args) \
	DeeFunction_Call(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeFunction_CallTupleKw(self, args, kw) \
	DeeFunction_CallKw(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeFunction_ThisCallTuple(self, this_arg, args) \
	DeeFunction_ThisCall(self, this_arg, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeFunction_ThisCallTupleKw(self, this_arg, args, kw) \
	DeeFunction_ThisCallKw(self, this_arg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_CODE_H */
