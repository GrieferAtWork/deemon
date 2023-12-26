/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HOSTASM_LIBHOSTASM_H
#define GUARD_DEX_HOSTASM_LIBHOSTASM_H 1

#include "host.h"
/**/

#include <deemon/api.h>
#include <deemon/code.h>

#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>

/* Convert compiled deemon code to host machine assembly (currently: only x86)
 *
 *
 *
 * ============= RESTRICTIONS =============
 *
 * - Deemon code must not make use of flexible stack depths
 * - Yield-functions (CoRoutines) cannot be converted
 * - try- (and consequently with-) related instructions cannot be used (yet?)
 * - ASM_JMP_POP and ASM_JMP_POP_POP can only be compiled if the
 *   relevant jump table is a constant _RoDict, and that fact can
 *   be proven by simply looking at assembly.
 *
 *
 *
 * ============= IMPLEMENTATION =============
 *
 * - Split assembly into basic blocks
 * - Each block starts with a memory state (describing where deemon stack/locals
 *   exist, which can be either %eax, or CFA stack-offsets, as well as if that
 *   location is currently holding a reference)
 *   - Current x86 %esp-offset from CFA
 *   - Current deemon stack size
 *   - For every stack slot (that is in use):
 *     - Flag: Slot contains a reference (DREF):
 *     - Select (storage kind):
 *       - Slot contains a constant address $0x1234578 (address of deemon object)
 *       - Slot references a function argument i (index of argument)
 *       - Slot is stored in register: [%eax, %ecx, %edx]
 *       - Slot is stored on x86 stack (offset from CFA)
 *   - For every local slot:
 *     - Tristate: bound/unbound/unknown:
 *       - Slot is known to be bound/unbound
 *     - Flag: Slot contains a reference (DREF): [...]  (identical to stack)
 *     - Select (storage kind): [...]  (identical to stack)
 * - A Basic block ends when a jmp-label-, or the end of the containing code is reached
 * - A Basic block can contain as many branching instructions as it wants
 * - Generated assembly is callable as `DREF DeeObject *(DCALL *)(size_t argc, DeeObject *const *argv)'
 *
 *
 *
 * ============= FLAWS =============
 *
 * How will this integrate with `Dee_code_frame'?
 * -> It would probably be best to not use it, and use some sort of
 *    new instrumentation in order to unwind the stack and construct
 *    Traceback objects as the stack is unwound.
 *    Only problem here is that this kind-of changes the rules where
 *    >> print deemon.Traceback();
 *    won't work, and
 *    >> try {
 *    >>     ...
 *    >> } catch (...) {
 *    >>     print deemon.Traceback.current;
 *    >> }
 *    will only be able to print the traceback up until the containing
 *    function (though probably not even including it).
 * -> Work-arounds: Lots of extra os-specific integration by essentially
 *    porting KOS's libunwind for ELF targets, and writing a custom
 *    unwind function for PE targets. But this can easily get *really*
 *    finicky...
 *
 *
 *
 * ============= FUTURE DIRECTIONS =============
 *
 * - Turn `libgen86' into a git module that is shared between deemon and KOS
 * - Add a heuristic to DeeFunctionObject that keeps track of how often
 *   the function has been called. If that count exceeds some limit,
 *   automatically replace the function with its hostasm version.
 * - Hostasm re-compilation should take place in a separate thread that
 *   operates as a lazily-launched deamon, and takes re-compilation jobs
 *   as they come up.
 * - Add support for functions with custom exception handlers/finally
 * - Add support for functions with CODE_FASSEMBLY (by adding extra asserts in generated x86 code)
 * - Add an extra mapper for x86 assembly address -> Dee_code_addr_t+Dee_memstate,
 *   so that the original set of DDI instrumentation can still be used, and (e.g.)
 *   an lid can be converted to an x86 CFA-offset/register at runtime.
 * - Make it possible to re-compile yield functions by:
 *   - Generating code differently such that the deemon stack and locals
 *     aren't used %Psp-relative, but (e.g.) %Pbp.
 *   - Calculate the max stack/locals blob size at (re-)compile-time
 *   - When a yield function yields, it first saves all registers to the blob
 *   - When a yield function yields, it gives back 2 values
 *     - DREF DeeObject * -- The yielded value
 *     - void *           -- The "resume PC"
 *     The "resume PC" must be loaded by:
 *     - Loading the stack/locals blob into %Pbp
 *     - Pushing the return PC onto %Psp
 *     - Jumping to the "resume PC"
 *     - Code at the "resume PC" then loads saved registers from the blob
 *   - The stack/locals blob is then allocated/owned by the caller
 * - When code does a call to an extern/global symbol (that is already
 *   assigned, is final, *and* points to another Code object), it may
 *   be possible to actually hard-code calls between deemon functions
 *   without any overhead normally related to dynamic calls. Actually,
 *   this same thing goes for any operator invocation where the object
 *   the object gets invoked on is assigned at compile-time, and its
 *   container is final/immutable.
 */

#ifdef CONFIG_HAVE_LIBHOSTASM
DECL_BEGIN

typedef uint16_t Dee_vstackaddr_t;
typedef int16_t Dee_vstackoff_t;


union Dee_memloc_value {
	uintptr_t _v_data[2];
	struct {
		Dee_host_register_t r_regno; /* Host register number */
		byte_t             _r_pad[sizeof(uintptr_t) - sizeof(Dee_host_register_t)];
		ptrdiff_t           r_off;   /* Register value addend */
		ptrdiff_t           r_voff;  /* [valid_if(MEMLOC_TYPE_HREGIND)] Register value addend (after indirection) */
	} v_hreg;           /* [valid_if(MEMLOC_TYPE_HREG, MEMLOC_TYPE_HREGIND)] */
	struct {
		byte_t   _s_pad[sizeof(uintptr_t)];
		uintptr_t s_cfa; /* [ALIGNED(HOST_SIZEOF_POINTER)] Stack CFA offset */
		ptrdiff_t s_off; /* [valid_if(MEMLOC_TYPE_HSTACKIND)] Extra offset applied to indirect value */
	} v_hstack; /* [valid_if(MEMLOC_TYPE_HSTACK, MEMLOC_TYPE_HSTACKIND)] */
	DeeObject *v_const; /* [1..1][valid_if(MEMLOC_TYPE_CONST)] */
};

struct Dee_memloc {
#define MEMLOC_F_NORMAL        0x0000
#define MEMLOC_F_NOREF         0x0001 /* Slot contains no reference */
#define MEMLOC_M_LOCAL_BSTATE  0xc000 /* Mask for the bound-ness of a local variable */
#define MEMLOC_F_LOCAL_UNKNOWN 0x0000 /* Local variable bound-ness is unknown */
#define MEMLOC_F_LOCAL_BOUND   0x4000 /* Local variable is bound */
#define MEMLOC_F_LOCAL_UNBOUND 0x8000 /* Local variable is unbound */
	uint16_t               ml_flags; /* Location flags (set of `MEMLOC_F_*') */
	/* TODO: value-proxy indirection (applied on-top of `ml_type'). e.g.: `value = DeeBool_For(value)' */

	/* NOTE: *IND location types are *never* used as store targets (they only act as lazily loaded cache locations) */
#define MEMLOC_TYPE_HREG      0 /* >> value = %v_hreg.r_regno + v_hreg.r_off; */
#define MEMLOC_TYPE_HREGIND   1 /* >> value = *(byte_t **)(%v_hreg.r_regno + v_hreg.r_off) + v_hreg.r_voff; */
#define MEMLOC_TYPE_HSTACK    2 /* >> value = CFA(%sp, v_hstack.s_cfa); */
#define MEMLOC_TYPE_HSTACKIND 3 /* >> value = *(byte_t **)CFA(%sp, v_hstack.s_cfa) + v_hstack.s_off; */
#define MEMLOC_TYPE_CONST     4 /* >> value = v_const; */
#define MEMLOC_TYPE_UNALLOC   5 /* >> value = ???;  // Not allocated (only valid for local variables) */
#define MEMLOC_TYPE_UNDEFINED 6 /* >> value = ???;  // Not defined (value can be anything at runtime) */
#define MEMLOC_TYPE_HASREG(typ) ((typ) <= MEMLOC_TYPE_HREGIND)
#define MEMLOC_TYPE_CASEREG     case MEMLOC_TYPE_HREG: case MEMLOC_TYPE_HREGIND
	uint16_t               ml_type;  /* Location kind (one of `MEMLOC_TYPE_*') */
	union Dee_memloc_value ml_value; /* Location value */
	/* TODO: DeeTypeObject *ml_valtyp; // [0..1] If non-null, the guarantied correct object type of this location (for inlining operator calls) */
};

/* Check if `a' and `b' describe the same host memory location (i.e. are aliasing each other). */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memloc_sameloc(struct Dee_memloc const *a, struct Dee_memloc const *b);

/* Similar to `Dee_memloc_samemem()', but disregard differences in value-offsets. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memloc_samemem(struct Dee_memloc const *a, struct Dee_memloc const *b);



/* Possible values for `Dee_memstate::ms_rusage' */
#define DEE_HOST_REGUSAGE_GENERIC 0x00 /* Register usage is defined by `ms_stackv' and `ms_localv'. */
#define DEE_HOST_REGUSAGE_THREAD  0x01 /* Register contains: DeeThread_Self() */
typedef uint8_t Dee_host_regusage_t;


/* Extra local variable IDs always present in `ms_localv'
 * These indices appear after "normal" locals. */
#define DEE_MEMSTATE_EXTRA_LOCAL_A_THIS     0 /* Caller-argument: `DeeObject *this'      (only for `HOSTFUNC_CC_F_THIS') */
#define DEE_MEMSTATE_EXTRA_LOCAL_A_ARGC     1 /* Caller-argument: `size_t argc'          (only for `!HOSTFUNC_CC_F_TUPLE') */
#define DEE_MEMSTATE_EXTRA_LOCAL_A_ARGS     2 /* Caller-argument: `DeeTupleObject *args' (only for `HOSTFUNC_CC_F_TUPLE') */
#define DEE_MEMSTATE_EXTRA_LOCAL_A_ARGV     2 /* Caller-argument: `DeeObject **argv'     (only for `!HOSTFUNC_CC_F_TUPLE') */
#define DEE_MEMSTATE_EXTRA_LOCAL_A_KW       3 /* Caller-argument: `DeeObject *kw'        (only for `HOSTFUNC_CC_F_KW') */
#define DEE_MEMSTATE_EXTRA_LOCAL_VARARGS    4 /* Varargs (s.a. `struct Dee_code_frame::cf_vargs') */
#define DEE_MEMSTATE_EXTRA_LOCAL_VARKWDS    5 /* Varkwds (s.a. `struct Dee_code_frame_kwds::fk_varkwds') */
#define DEE_MEMSTATE_EXTRA_LOCAL_STDOUT     6 /* Temporary slot for a cached version of `deemon.File.stdout' (to speed up `ASM_PRINT' & friends) */
#define DEE_MEMSTATE_EXTRA_LOCAL_MINCOUNT   7 /* Min number of extra locals */
#define DEE_MEMSTATE_EXTRA_LOCAL_DEFARG_MIN DEE_MEMSTATE_EXTRA_LOCAL_MINCOUNT
#define DEE_MEMSTATE_EXTRA_LOCAL_DEFARG(i)  (DEE_MEMSTATE_EXTRA_LOCAL_DEFARG_MIN + (i)) /* Start of cached optional arguments. */

struct Dee_memstate {
	Dee_refcnt_t                               ms_refcnt;          /* Reference counter for the mem-state (state becomes read-only when >1) */
	uintptr_t                                  ms_host_cfa_offset; /* Delta between SP to CFA (Canonical Frame Address) */
	size_t                                     ms_localc;          /* [== :co_localc+DEE_MEMSTATE_EXTRA_LOCAL_MINCOUNT+(:co_argc_max-:co_argc_min)]
	                                                                * Number of local variables + extra slots. NOTE: Never 0! */
	Dee_vstackaddr_t                           ms_stackc;          /* Number of (currently) used deemon stack slots in use. */
	Dee_vstackaddr_t                           ms_stacka;          /* Allocated number of deemon stack slots in use. */
	size_t                                     ms_rinuse[HOST_REGISTER_COUNT]; /* Number of times each register is referenced by `ms_stackv' and `ms_localv' */
	Dee_host_regusage_t                        ms_rusage[HOST_REGISTER_COUNT]; /* Meaning of registers (set to `DEE_HOST_REGUSAGE_GENERIC' if clobbered) */
	struct Dee_memloc                         *ms_stackv;          /* [0..ms_stackc][owned] Deemon stack memory locations. */
#ifdef __INTELLISENSE__
	struct Dee_memloc                          ms_localv[1024];    /* [0..ms_localc] Deemon locals memory locations. */
#else /* __INTELLISENSE__ */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_memloc, ms_localv);         /* [0..ms_localc] Deemon locals memory locations. */
#endif /* !__INTELLISENSE__ */
};

/* Helper macro to enumerate all `struct Dee_memloc *loc' of a `struct Dee_memstate *self':
 * >> struct Dee_memloc *loc;
 * >> Dee_memstate_foreach(loc, state) {
 * >>     ...
 * >> }
 * >> Dee_memstate_foreach_end;
 */
#define Dee_memstate_foreach_isstack(loc, self) (_dmfe_v != (self)->ms_localv)
#define Dee_memstate_foreach_islocal(loc, self) (_dmfe_v == (self)->ms_localv)
#define Dee_memstate_foreach(loc, self)                                 \
	do {                                                                \
		struct Dee_memloc *_dmfe_v = (self)->ms_stackv;                 \
		size_t _dmfe_i = 0, _dmfe_c = (self)->ms_stackc;                \
		do                                                              \
			if (_dmfe_i >= _dmfe_c                                      \
			    ? (_dmfe_v == (self)->ms_stackv                         \
			       ? (_dmfe_i = 0,                                      \
			          _dmfe_v = (struct Dee_memloc *)(self)->ms_localv, \
			          _dmfe_c = (self)->ms_localc,                      \
			          0)                                                \
			       : 1)                                                 \
			    : 0) {                                                  \
				break;                                                  \
			} else if (((loc) = &_dmfe_v[_dmfe_i++], 0))                \
				;                                                       \
			else
#define Dee_memstate_foreach_end \
		__WHILE1;                \
	}	__WHILE0


#define Dee_memstate_alloc(localc)                                                \
	((struct Dee_memstate *)Dee_Malloc(offsetof(struct Dee_memstate, ms_localv) + \
	                                   (localc) * sizeof(struct Dee_memloc)))
#define Dee_memstate_free(self) Dee_Free(self)
INTDEF NONNULL((1)) void DCALL Dee_memstate_destroy(struct Dee_memstate *__restrict self);
#define Dee_memstate_incref(self) (void)(++(self)->ms_refcnt)
#define Dee_memstate_decref(self) (void)(--(self)->ms_refcnt || (Dee_memstate_destroy(self), 0))
#define Dee_memstate_decref_nokill(self) (void)(ASSERT((self)->ms_refcnt >= 2), --(self)->ms_refcnt)

/* Replace `*p_self' with a copy of itself
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_memstate_inplace_copy_because_shared(struct Dee_memstate **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) DREF struct Dee_memstate *DCALL
Dee_memstate_copy(struct Dee_memstate *__restrict self);
#define Dee_memstate_isshared(self) ((self)->ms_refcnt > 1)
#define Dee_memstate_unshare(p_self)                             \
	(Dee_memstate_verifyrinuse(*(p_self)), /* TODO: REMOVE ME */ \
	 unlikely(Dee_memstate_isshared(*(p_self))                   \
	          ? Dee_memstate_inplace_copy_because_shared(p_self) \
	          : 0))

/* Ensure that at least `min_alloc' stack slots are allocated. */
INTDEF NONNULL((1)) int DCALL
Dee_memstate_reqvstack(struct Dee_memstate *__restrict self,
                       Dee_vstackaddr_t min_alloc);

/* Account for register usage. */
#define Dee_memstate_incrinuse(self, regno) (void)++(self)->ms_rinuse[regno]
#define Dee_memstate_decrinuse(self, regno) (void)(ASSERT((self)->ms_rinuse[regno] > 0), --(self)->ms_rinuse[regno])
#ifdef NDEBUG
#define Dee_memstate_verifyrinuse(self) (void)0
#else /* NDEBUG */
#define Dee_memstate_verifyrinuse(self) Dee_memstate_verifyrinuse_d(self)
INTDEF NONNULL((1)) void DCALL Dee_memstate_verifyrinuse_d(struct Dee_memstate const *__restrict self);
#endif /* !NDEBUG */


#ifdef HOSTASM_STACK_GROWS_DOWN
#define Dee_memstate_hstack_cfa2sp(self, cfa_offset) (ptrdiff_t)((self)->ms_host_cfa_offset - (uintptr_t)(cfa_offset))
#define Dee_memstate_hstack_sp2cfa(self, sp_offset)  ((ptrdiff_t)(self)->ms_host_cfa_offset - (ptrdiff_t)(sp_offset))
#else /* HOSTASM_STACK_GROWS_DOWN */
#define Dee_memstate_hstack_cfa2sp(self, cfa_offset) (ptrdiff_t)((uintptr_t)(cfa_offset) - (self)->ms_host_cfa_offset)
#define Dee_memstate_hstack_sp2cfa(self, sp_offset)  ((ptrdiff_t)(self)->ms_host_cfa_offset + (ptrdiff_t)(sp_offset))
#endif /* !HOSTASM_STACK_GROWS_DOWN */

/* Check if there is a register that contains `usage'.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_usage(struct Dee_memstate const *__restrict self,
                              Dee_host_regusage_t usage);

/* Allocate extra memory for the host stack (caller must still generate code) */
#ifdef HOSTASM_STACK_GROWS_DOWN
#define Dee_memstate_hstack_alloca(self, n_bytes) ((self)->ms_host_cfa_offset += (n_bytes))
#else /* HOSTASM_STACK_GROWS_DOWN */
#define Dee_memstate_hstack_alloca(self, n_bytes) (((self)->ms_host_cfa_offset += (n_bytes)) - (n_bytes))
#endif /* !HOSTASM_STACK_GROWS_DOWN */

/* Check if there is a register that is completely unused.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent.
 * @param: accept_if_with_regusage: When true, allowed to return registers with
 *                                  `ms_rusage[return] != DEE_HOST_REGUSAGE_GENERIC' */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused(struct Dee_memstate const *__restrict self,
                               bool accept_if_with_regusage);

/* Check if `regno' is used by stack/locals (ignores `ms_rusage') */
#define Dee_memstate_hregs_isused(self, regno) ((self)->ms_rinuse[regno] > 0)

/* Same as `Dee_memstate_hregs_find_unused(self, true)', but don't return `not_these',
 * which is an array of register numbers terminated by one `>= HOST_REGISTER_COUNT'.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent. */
INTDEF WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused_ex(struct Dee_memstate *__restrict self,
                                  Dee_host_register_t const *not_these);

/* Adjust register-related memory locations to account for `%regno = %regno + delta' */
INTDEF NONNULL((1)) void DCALL
Dee_memstate_hregs_adjust_delta(struct Dee_memstate *__restrict self,
                                Dee_host_register_t regno, ptrdiff_t delta);

/* Set all members of `self->ms_rusage' to `DEE_HOST_REGUSAGE_GENERIC' */
#if DEE_HOST_REGUSAGE_GENERIC == 0
#define Dee_memstate_hregs_clear_usage(self) \
	bzero((self)->ms_rusage, sizeof((self)->ms_rusage))
#else /* DEE_HOST_REGUSAGE_GENERIC == 0 */
#define Dee_memstate_hregs_clear_usage(self)                                       \
	do {                                                                           \
		Dee_host_regusage_t _mhrcu_regno;                                          \
		for (_mhrcu_regno = 0; _mhrcu_regno < HOST_REGISTER_COUNT; ++_mhrcu_regno) \
			(self)->ms_rusage[_mhrcu_regno] = DEE_HOST_REGUSAGE_GENERIC;           \
	}	__WHILE0
#endif /* DEE_HOST_REGUSAGE_GENERIC != 0 */
	


/* Try to find a `n_bytes'-large free section of host stack memory.
 * @param: hstack_reserved: When non-NULL, only consider locations that are *also* free in here
 * @return: * :            The base-CFA offset of the free section of memory
 * @return: (uintptr_t)-1: There is no free section of at least `n_bytes' bytes.
 *                         In this case, allocate using `Dee_memstate_hstack_alloca()' */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
Dee_memstate_hstack_find(struct Dee_memstate const *__restrict self,
                         struct Dee_memstate const *hstack_reserved,
                         size_t n_bytes);

/* Check if a pointer-sized blob at `cfa_offset' is being used by something. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_memstate_hstack_isused(struct Dee_memstate const *__restrict self,
                           uintptr_t cfa_offset);

/* Try to free unused stack memory near the top of the stack.
 * @return: true:  The CFA offset was reduced.
 * @return: false: The CFA offset remains the same. */
INTDEF NONNULL((1)) bool DCALL
Dee_memstate_hstack_free(struct Dee_memstate *__restrict self);

/* Constrain `self' with `other', such that it is possible to generate code to
 * transition from `other' to `self', as well as any other mem-state that might
 * be the result of further constraints applied to `self'.
 * @return: true:  State become more constrained
 * @return: false: State didn't change */
INTDEF NONNULL((1, 2)) bool DCALL
Dee_memstate_constrainwith(struct Dee_memstate *__restrict self,
                           struct Dee_memstate const *__restrict other);

/* Find the next alias (that is holding a reference) for `loc' after `after'
 * @return: * :   The newly discovered alias
 * @return: NULL: No (more) aliases exist for `loc' */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) struct Dee_memloc *DCALL
Dee_memstate_findrefalias(struct Dee_memstate *__restrict self,
                          struct Dee_memloc const *loc,
                          struct Dee_memloc *after);

/* Functions to manipulate the virtual deemon object stack. */
#define Dee_memstate_vtop(self) (&(self)->ms_stackv[(self)->ms_stackc - 1])
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vswap(struct Dee_memstate *__restrict self); /* ASM_SWAP */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vlrot(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vrrot(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush(struct Dee_memstate *__restrict self, struct Dee_memloc *loc);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_undefined(struct Dee_memstate *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_const(struct Dee_memstate *__restrict self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_reg(struct Dee_memstate *__restrict self, Dee_host_register_t regno, ptrdiff_t delta);                             /* (MEMLOC_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_regind(struct Dee_memstate *__restrict self, Dee_host_register_t regno, ptrdiff_t ind_delta, ptrdiff_t val_delta); /* (MEMLOC_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_hstack(struct Dee_memstate *__restrict self, uintptr_t cfa_offset);                                                /* (MEMLOC_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_hstackind(struct Dee_memstate *__restrict self, uintptr_t cfa_offset, ptrdiff_t val_delta);                        /* (MEMLOC_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vdup_n(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n);
#define Dee_memstate_vdup(self) Dee_memstate_vdup_n(self, 1)




/* Host text symbol */
struct Dee_jump_descriptor;
struct Dee_host_section;
struct Dee_host_symbol {
	struct Dee_host_symbol *_hs_next; /* [0..1][owned] Next symbol (used internally for chaining) */
#define DEE_HOST_SYMBOL_UNDEF 0 /* Not yet defined */
#define DEE_HOST_SYMBOL_ABS   1 /* Absolute value (e.g. for API functions) */
#define DEE_HOST_SYMBOL_JUMP  2 /* Pass-through via a `struct Dee_jump_descriptor' (or fast-forward to start of section) */
#define DEE_HOST_SYMBOL_SECT  3 /* Offset into a `struct Dee_host_section' */
	uintptr_t hs_type; /* Symbol type (one of `DEE_HOST_SYMBOL_*') */
	union {
		void const                 *sv_abs;  /* [?..?][valid_if(DEE_HOST_SYMBOL_ABS)] */
		struct Dee_jump_descriptor *sv_jump; /* [1..1][valid_if(DEE_HOST_SYMBOL_JUMP)] */
		struct {
			struct Dee_host_section *ss_sect; /* [1..1] Target section */
			uintptr_t                ss_off;  /* Offset into `ss_sect' */
		} sv_sect; /* [valid_if(DEE_HOST_SYMBOL_SECT)] */
	} hs_value;
};

#define _Dee_host_symbol_alloc()    ((struct Dee_host_symbol *)Dee_Malloc(sizeof(struct Dee_host_symbol)))
#define _Dee_host_symbol_free(self) Dee_Free(self)

#define Dee_host_symbol_isdefined(self) ((self)->hs_type == DEE_HOST_SYMBOL_UNDEF)
#define Dee_host_symbol_setabs(self, addr)                \
	(void)((self)->hs_type         = DEE_HOST_SYMBOL_ABS, \
	       (self)->hs_value.sv_abs = (addr))
#define Dee_host_symbol_setjump(self, desc)                \
	(void)((self)->hs_type          = DEE_HOST_SYMBOL_JMP, \
	       (self)->hs_value.sv_jump = (desc))
#define Dee_host_symbol_setsect(self, sect) \
	Dee_host_symbol_setsect_ex(self, sect, Dee_host_section_size(sect))
#define Dee_host_symbol_setsect_ex(self, sect, offset)              \
	(void)((self)->hs_type                  = DEE_HOST_SYMBOL_SECT, \
	       (self)->hs_value.sv_sect.ss_sect = (sect),               \
	       (self)->hs_value.sv_sect.ss_off  = (offset))



/* Host relocation types. */
#define DEE_HOST_RELOC_NONE  0
#ifdef HOSTASM_X86
#define DEE_HOST_RELOC_REL32 1 /* *(int32_t *)<ADDR> += (<VALUE> - <ADDR>) */
#endif /* HOSTASM_X86 */

struct Dee_host_reloc {
	uint32_t                hr_offset; /* Offset from `bb_host_start' to where the relocation takes place. */
	uint16_t                hr_rtype;  /* Relocation type (one of `DEE_HOST_RELOC_*') */
#define DEE_HOST_RELOCVALUE_SYM  0     /* Relocate against a symbol */
#define DEE_HOST_RELOCVALUE_ABS  1     /* Relocate against an absolute value (for API calls) */
#define DEE_HOST_RELOCVALUE_SECT 2     /* Relocate against a section base address */
	uint16_t                hr_vtype;  /* Value type (one of `DEE_HOST_RELOCVALUE_*') */
	union {
		struct Dee_host_symbol  *rv_sym;  /* [1..1][valid_if(DEE_HOST_RELOCVALUE_SYM)] Relocation symbol. */
		void const              *rv_abs;  /* [?..?][valid_if(DEE_HOST_RELOCVALUE_ABS)] Relocation value. */
		struct Dee_host_section *rv_sect; /* [1..1][valid_if(DEE_HOST_RELOCVALUE_SECT)] Relocation section. */
	} hr_value;
};

/* Fill in `self->hr_vtype' and `self->hr_value' based on `sym'
 * If `sym' has already been defined as absolute or pointing to
 * the start of a section, directly inline it. */
INTDEF NONNULL((1, 2)) void DCALL
Dee_host_reloc_setsym(struct Dee_host_reloc *__restrict self,
                      struct Dee_host_symbol *__restrict sym);

struct Dee_host_section {
	byte_t                      *hs_start; /* [<= hs_end][owned] Start of host assembly */
	byte_t                      *hs_end;   /* [>= hs_start && <= hs_alend] End of host assembly */
	byte_t                      *hs_alend; /* [>= hs_start] End of allocated host assembly */
	struct Dee_host_reloc       *hs_relv;  /* [0..hs_relc][owned] Vector of host relocations. */
	size_t                       hs_relc;  /* Number of host relocations. */
	size_t                       hs_rela;  /* Allocated number of host relocations. */
	LIST_ENTRY(Dee_host_section) hs_link;  /* [0..1] Position of this section in the final output. */
};

#define Dee_host_section_init(self) bzero(self, sizeof(struct Dee_host_section))
#define Dee_host_section_fini(self) (Dee_Free((self)->hs_start), Dee_Free((self)->hs_relv))
#define Dee_host_section_clear(self) \
	(void)((self)->hs_end = (self)->hs_start, (self)->hs_relc = 0)
#define Dee_host_section_size(self) \
	(size_t)((self)->hs_end - (self)->hs_start)

/* Ensure that at least `num_bytes' of host text memory are available.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_reqhost(struct Dee_host_section *__restrict self,
                          size_t num_bytes);
#define _Dee_host_section_hostavail(self) \
	((size_t)((self)->hs_alend - (self)->hs_end))
#define Dee_host_section_reqhost(self, num_bytes) \
	(_Dee_host_section_hostavail(self) >= (num_bytes) ? 0 : _Dee_host_section_reqhost(self, num_bytes))

/* Allocate and return a new host relocation. The caller is responsible
 * for filling in said relocation, and the returned pointer only remains
 * valid until the next call to this function with the same `self'.
 * @return: * :   The (uninitialized) host relocation
 * @return: NULL: Error  */
INTDEF WUNUSED NONNULL((1)) struct Dee_host_reloc *DCALL
Dee_host_section_newhostrel(struct Dee_host_section *__restrict self);



struct Dee_jump_descriptor {
	Dee_instruction_t const  *jd_from;   /* [1..1][const] Deemon instruction where the jump originates from. */
	struct Dee_basic_block   *jd_to;     /* [1..1][const] Basic block that this jump goes to. */
#ifdef __INTELLISENSE__
	struct Dee_memstate      *jd_stat;   /* [0..1] Memory state at the point where `jd_from' performs its jump (or NULL if not yet generated). */
#else /* __INTELLISENSE__ */
	DREF struct Dee_memstate *jd_stat;   /* [0..1] Memory state at the point where `jd_from' performs its jump (or NULL if not yet generated). */
#endif /* !__INTELLISENSE__ */
	struct Dee_host_section   jd_morph;  /* Text section to morph the memory state from `jd_stat' to `jd_to->bb_mem_start' */
};

#define Dee_jump_descriptor_alloc() \
	((struct Dee_jump_descriptor *)Dee_Malloc(sizeof(struct Dee_jump_descriptor)))
#define Dee_jump_descriptor_free(self) Dee_Free(self)
#define Dee_jump_descriptor_fini(self)                                    \
	(void)(!(self)->jd_stat || (Dee_memstate_decref((self)->jd_stat), 0), \
	       Dee_host_section_fini(&(self)->jd_morph))
#define Dee_jump_descriptor_destroy(self) \
	(Dee_jump_descriptor_fini(self), Dee_jump_descriptor_free(self))

struct Dee_jump_descriptors {
	struct Dee_jump_descriptor **jds_list;  /* [owned_if(this == :bb_entries)][0..jds_size][owned]
	                                         * List of jump descriptors, sorted by `jd_from' */
	size_t                       jds_size;  /* Number of jump descriptors. */
	size_t                       jds_alloc; /* Allocated number of jump descriptors. */
};

#define Dee_jump_descriptors_init(self) \
	((self)->jds_list = NULL,           \
	 (self)->jds_size = 0,              \
	 (self)->jds_alloc = 0)

/* Lookup the jump descriptor for `deemon_from'
 * @return: * :   The jump descriptor in question.
 * @return: NULL: No such jump descriptor. */
INTDEF WUNUSED NONNULL((1)) struct Dee_jump_descriptor *DCALL
Dee_jump_descriptors_lookup(struct Dee_jump_descriptors const *__restrict self,
                            Dee_instruction_t const *deemon_from);

/* Insert a new jump descriptor into `self'
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_jump_descriptors_insert(struct Dee_jump_descriptors *__restrict self,
                            struct Dee_jump_descriptor *__restrict descriptor);

/* Remove `descriptor' from `self' (said descriptor *must* be part of `self') */
INTDEF WUNUSED NONNULL((1, 2)) void DCALL
Dee_jump_descriptors_remove(struct Dee_jump_descriptors *__restrict self,
                            struct Dee_jump_descriptor *__restrict descriptor);


struct Dee_basic_block {
	Dee_instruction_t const    *bb_deemon_start; /* [1..1][<= bb_deemon_end][const] Start of deemon assembly */
	Dee_instruction_t const    *bb_deemon_end;   /* [1..1][>= bb_deemon_start][const] End of deemon assembly */
	Dee_instruction_t const    *bb_deemon_end_r; /* [1..1][>= bb_deemon_start][const] Real end of deemon assembly */
	struct Dee_jump_descriptors bb_entries;      /* All of the possible ways this basic block can be entered (at `bb_deemon_start' / `bb_host_start'; this one owns descriptors). */
	struct Dee_jump_descriptors bb_exits;        /* All of the possible ways this basic block can be exited (via deemon code). */
	struct Dee_basic_block     *bb_next;         /* [0..1] Fallthru exit of this basic block (or NULL if there is none, which happens for the last block and blocks that end with NORETURN instructions) */
#ifdef __INTELLISENSE__
	struct Dee_memstate        *bb_mem_start;    /* [0..1] Memory state at start of basic block (or NULL if not yet assembled) */
	struct Dee_memstate        *bb_mem_end;      /* [0..1] Memory state at end of basic block (or NULL if not yet assembled) */
#else /* __INTELLISENSE__ */
	DREF struct Dee_memstate   *bb_mem_start;    /* [0..1] Memory state at start of basic block (or NULL if not yet assembled) */
	DREF struct Dee_memstate   *bb_mem_end;      /* [0..1] Memory state at end of basic block (or NULL if not yet assembled) */
#endif /* !__INTELLISENSE__ */
	struct Dee_host_section     bb_htext;        /* Host assembly text */
	struct Dee_host_section     bb_hcold;        /* Extra assembly text that should be placed at the end of the function */
	/* TODO: Have an extra pass before `Dee_function_assembler_compileblocks()' that checks when's
	 *       the last time that a local variable (including extra locals; e.g. the last time any
	 *       argument is accessed affects DEE_MEMSTATE_EXTRA_LOCAL_A_ARGV) gets used.
	 * With that extra information, emit calls to `Dee_function_generator_vdel_local()' as code gets
	 * processed. Using this, we can dispose of local variables earlier, which then makes it possible
	 * to reduce the overall hreg/hstack usage, as well as simplify exception cleanup.
	 * - For this purpose, have a bitset for all locals that say "this basic block uses these vars"
	 * - In an initial pass, simply scan the deemon code of the block and mark used locals
	 * - Recursively merge bitsets from `bb_next' and `bb_exits.jds_list[*]->jd_to' with the source block
	 * - With these steps, information is now available for "block still needs local to be alive"
	 * - While generating code any instruction that uses a local must then:
	 *   - Find the next-reached entry from the block `bb_exits', or `bb_next' if there are none
	 *     - If a next-block exists, but indicates that it also uses a local, do nothing
	 *     - else, see if the current block uses the local between the current instruction and where
	 *       the jump to the next-block happens (if the next block is `bb_next', search until the end
	 *       of the current basic block)
	 *       - If the local will still be needed, do nothing
	 *       - else, delete the local (using `Dee_function_generator_vdel_local()') */
};

#define Dee_basic_block_alloc()    ((struct Dee_basic_block *)Dee_Malloc(sizeof(struct Dee_basic_block)))
#define Dee_basic_block_free(self) Dee_Free(self)

/* Initialize common fields of `self'. The caller must still initialize:
 * - self->bb_deemon_start
 * - self->bb_deemon_end
 * - self->bb_exits */
#define Dee_basic_block_init_common(self)            \
	(Dee_jump_descriptors_init(&(self)->bb_entries), \
	 (self)->bb_next      = NULL,                    \
	 (self)->bb_mem_start = NULL,                    \
	 (self)->bb_mem_end   = NULL,                    \
	 Dee_host_section_init(&(self)->bb_htext),       \
	 Dee_host_section_init(&(self)->bb_hcold))

/* Destroy the given basic block `self'. */
INTDEF NONNULL((1)) void DCALL
Dee_basic_block_destroy(struct Dee_basic_block *__restrict self);

/* Split this basic block at `addr' (which must be `> bb_deemon_start'),
 * and move all jumps from `bb_exits' into the new basic block, as needed.
 * @return: * :   A new basic block that starts at `addr'
 * @return: NULL: Error */
INTDEF WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_basic_block_splitat(struct Dee_basic_block *__restrict self,
                        Dee_instruction_t const *addr);

/* Constrain or assign `self->bb_mem_start' with the memory state `state'
 * @param: self_start_addr: The starting-address of `self' (for error messages)
 * @return: 1 : State become more constrained
 * @return: 0 : State didn't change 
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_basic_block_constrainwith(struct Dee_basic_block *__restrict self,
                              struct Dee_memstate *__restrict state,
                              code_addr_t self_start_addr);

/* Remove exits from `self' that have origins beyond `self->bb_deemon_end' */
INTDEF NONNULL((1)) void DCALL
Dee_basic_block_trim_unused_exits(struct Dee_basic_block *__restrict self);



/* Small descriptor for what needs to be cleaned up in a `struct Dee_memstate' */
struct Dee_except_exitinfo {
	struct Dee_basic_block           *exi_block;                     /* [1..1][owned] Block implementing this exit state. */
	uintptr_t                         exi_cfa_offset;                /* [== exi_block->bb_mem_start->ms_host_cfa_offset]. */
#define DEE_EXCEPT_EXITINFO_NULLFLAG 0x8000 /* If or'd with `exi_regs[*]' or `exi_stack[*]', means that location may be null */
	uint16_t                          exi_regs[HOST_REGISTER_COUNT]; /* How often each register needs to be decref'd */
	COMPILER_FLEXIBLE_ARRAY(uint16_t, exi_stack);                    /* [exi_cfa_offset / HOST_SIZEOF_POINTER]
	                                                                  * How often CFA offsets need to be decref'd */
};

/* Check if `self' has been compiled. */
#define Dee_except_exitinfo_compiled(self)                                          \
	(((self)->exi_block->bb_htext.hs_start < (self)->exi_block->bb_htext.hs_end) || \
	 ((self)->exi_block->bb_next != NULL))

/* Convert CFA offsets <==> index into `struct Dee_except_exitinfo::exi_stack' */
#ifdef HOSTASM_STACK_GROWS_DOWN
#define Dee_except_exitinfo_cfa2index(cfa_offset) (((cfa_offset)-HOST_SIZEOF_POINTER) / HOST_SIZEOF_POINTER)
#define Dee_except_exitinfo_index2cfa(index)      (((index)*HOST_SIZEOF_POINTER) + HOST_SIZEOF_POINTER)
#else /* HOSTASM_STACK_GROWS_DOWN */
#define Dee_except_exitinfo_cfa2index(cfa_offset) ((cfa_offset) / HOST_SIZEOF_POINTER)
#define Dee_except_exitinfo_index2cfa(index)      ((index)*HOST_SIZEOF_POINTER)
#endif /* !HOSTASM_STACK_GROWS_DOWN */

#define Dee_except_exitinfo_alloc(sizeof) ((struct Dee_except_exitinfo *)Dee_Malloc(sizeof))
#define Dee_except_exitinfo_free(self)    Dee_Free(self)
#define Dee_except_exitinfo_destroy(self) (Dee_basic_block_destroy((self)->exi_block), Dee_except_exitinfo_free(self))
#define Dee_except_exitinfo_sizeof(cfa_offset) \
	(offsetof(struct Dee_except_exitinfo, exi_stack) + ((cfa_offset) / HOST_SIZEOF_POINTER) * sizeof(uint16_t))
#define _Dee_except_exitinfo_cmp_baseof(x) (&(x)->exi_cfa_offset)
#define _Dee_except_exitinfo_cmp_sizeof(x)                    \
	((offsetof(struct Dee_except_exitinfo, exi_stack) -       \
	  offsetof(struct Dee_except_exitinfo, exi_cfa_offset)) + \
	 ((x)->exi_cfa_offset / HOST_SIZEOF_POINTER) * sizeof(uint16_t))
#define Dee_except_exitinfo_cmp(a, b)               \
	((a)->exi_cfa_offset < (b)->exi_cfa_offset      \
	 ? -1                                           \
	 : (a)->exi_cfa_offset > (b)->exi_cfa_offset    \
	   ? 1                                          \
	   : memcmp(_Dee_except_exitinfo_cmp_baseof(a), \
	            _Dee_except_exitinfo_cmp_baseof(b), \
	            _Dee_except_exitinfo_cmp_sizeof(a)))

/* Initialize `self' from `state' (with the exception of `self->exi_block')
 * @return: 0 : Success
 * @return: -1: Error (you're holding a reference to an argument/constant; why?) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_except_exitinfo_init(struct Dee_except_exitinfo *__restrict self,
                         struct Dee_memstate *__restrict state);

/* Calculate the "distance" score that determines the complexity of the
 * transitioning code needed to morph from `from' to `to'. When ordering
 * exception cleanup code, exit descriptors should be ordered such that
 * the fallthru of one to the next always yields the lowest distance
 * score.
 * @return: * : The distance scrore for morphing from `from' to `to' */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
Dee_except_exitinfo_distance(struct Dee_except_exitinfo const *__restrict from,
                             struct Dee_except_exitinfo const *__restrict to);



struct Dee_function_assembler {
	DeeFunctionObject           *fa_function;     /* [1..1][const] The function being assembled */
	DeeCodeObject               *fa_code;         /* [1..1][const][== fa_function->fo_code] The code being assembled */
	struct Dee_host_section      fa_prolog;       /* Function prolog (output even before `fa_blockv[0]'; verify arguments & set-up initial memstate) */
	DREF struct Dee_memstate    *fa_prolog_end;   /* [0..1] Memory state at the end of the prolog (or `NULL' if `Dee_function_assembler_compileblocks()' wasn't called, yet) */
	struct Dee_basic_block     **fa_blockv;       /* [owned][0..fa_blockc][owned] Vector of basic blocks (sorted by `bb_deemon_start'). */
	size_t                       fa_blockc;       /* Number of basic blocks. */
	size_t                       fa_blocka;       /* Allocated number of basic blocks. */
	struct Dee_except_exitinfo **fa_except_exitv; /* [owned][0..fa_except_exitc][owned] Vector of exception exit basic blocks (sorted by `Dee_except_exitinfo_cmp()') */
	size_t                       fa_except_exitc; /* Number of exception exit basic blocks. */
	size_t                       fa_except_exita; /* Allocated number of exception exit basic blocks. */
	struct Dee_host_symbol      *fa_symbols;      /* [0..1][owned] Chain of allocated symbols. */
#define DEE_FUNCTION_ASSEMBLER_F_NORMAL 0x0000
#define DEE_FUNCTION_ASSEMBLER_F_OSIZE  0x0001    /* Optimize for size (generally means: try not to use cold sections) */
	uint16_t                     fa_flags;        /* [const] Code generation flags (set of `DEE_FUNCTION_ASSEMBLER_F_*'). */
	uint16_t                     fa_localc;       /* [const][== fa_code->co_localc] */
	Dee_hostfunc_cc_t            fa_cc;           /* [const] Calling convention. */
};

#define Dee_function_assembler_addrof(self, addr) \
	((Dee_code_addr_t)((addr) - (self)->fa_code->co_code))

#define Dee_function_assembler_init(self, function, cc)               \
	(void)((self)->fa_function = (function),                          \
	       (self)->fa_code     = (function)->fo_code,                 \
	       Dee_host_section_init(&(self)->fa_prolog),                 \
	       (self)->fa_prolog_end   = NULL,                            \
	       (self)->fa_blockv       = NULL,                            \
	       (self)->fa_blockc       = 0,                               \
	       (self)->fa_blocka       = 0,                               \
	       (self)->fa_except_exitv = NULL,                            \
	       (self)->fa_except_exitc = 0,                               \
	       (self)->fa_except_exita = 0,                               \
	       (self)->fa_symbols      = NULL,                            \
	       (self)->fa_flags        = DEE_FUNCTION_ASSEMBLER_F_NORMAL, \
	       (self)->fa_localc       = (self)->fa_code->co_localc,      \
	       (self)->fa_cc           = (cc))
INTDEF NONNULL((1)) void DCALL
Dee_function_assembler_fini(struct Dee_function_assembler *__restrict self);

/* ================ Helpers ================ */

/* Ensure that the basic block containing `deemon_addr' also *starts* at that address.
 * This function is used during the initial scan-pass where basic blocks are identified
 * and created.
 * @return: * :   The basic block in question.
 * @return: NULL: An error occurred (OOM or address is out-of-bounds). */
INTDEF WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_function_assembler_splitblock(struct Dee_function_assembler *__restrict self,
                                  Dee_instruction_t const *deemon_addr);

/* Locate the basic block that contains `deemon_addr'
 * @return: * :   The basic block in question.
 * @return: NULL: Address is out-of-bounds. */
INTDEF WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_function_assembler_locateblock(struct Dee_function_assembler const *__restrict self,
                                   Dee_instruction_t const *deemon_addr);

/* Lookup/allocate an exception-exit basic block that can be used to clean
 * up `state' and then return `NULL' to the caller of the generated function.
 * @return: * :   The basic block to which to jump in order to clean up `state'.
 * @return: NULL: Error. */
INTDEF WUNUSED NONNULL((1, 2)) struct Dee_except_exitinfo *DCALL
Dee_function_assembler_except_exit(struct Dee_function_assembler *__restrict self,
                                   struct Dee_memstate *__restrict state);

/* Allocate a new host text symbol and return it.
 * @return: * :   The newly allocated host text symbol
 * @return: NULL: Error */
INTDEF WUNUSED NONNULL((1)) struct Dee_host_symbol *DCALL
Dee_function_assembler_newsym(struct Dee_function_assembler *__restrict self);





/* ================ Loaders ================ */

struct Dee_function_generator {
	struct Dee_function_assembler *fg_assembler;        /* [1..1][const] Assembler. */
	struct Dee_basic_block        *fg_block;            /* [1..1][const] Output basic block. */
	struct Dee_host_section       *fg_sect;             /* [1..1] Output section (usually `fg_block->bb_htext' or `fg_block->bb_hcold').
	                                                     * NOTE: If you alter this, you must also (and *always*) restore it. */
	DREF struct Dee_memstate      *fg_state;            /* [1..1] Current memory state. */
	struct Dee_memstate const     *fg_state_hstack_res; /* [0..1] State defining some extra reserved hstack locations (s.a. `Dee_memstate_hstack_find()'). */
};

#define Dee_function_generator_state_unshare(self) Dee_memstate_unshare(&(self)->fg_state)

/* Return a basic block that should be jumped to in order to handle a exception. */
#define Dee_function_generator_except_exit(self) \
	Dee_function_assembler_except_exit((self)->fg_assembler, (self)->fg_state)

#define Dee_function_generator_gadjust_cfa_offset(self, delta) \
	(void)((self)->fg_state->ms_host_cfa_offset += (uintptr_t)(ptrdiff_t)(delta))

#define Dee_function_generator_newsym(self) Dee_function_assembler_newsym((self)->fg_assembler)


/* Code generator helpers to manipulate the V-stack. */
#define Dee_function_generator_vtop(self) Dee_memstate_vtop((self)->fg_state)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vswap(struct Dee_function_generator *__restrict self); /* ASM_SWAP */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vlrot(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vrrot(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush(struct Dee_function_generator *__restrict self, struct Dee_memloc *loc);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_undefined(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_const(struct Dee_function_generator *__restrict self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_cid(struct Dee_function_generator *__restrict self, uint16_t cid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_rid(struct Dee_function_generator *__restrict self, uint16_t rid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_reg(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t delta);                             /* %regno + delta                    (MEMLOC_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_regind(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t ind_delta, ptrdiff_t val_delta); /* *(%regno + ind_delta) + val_delta (MEMLOC_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_hstack(struct Dee_function_generator *__restrict self, uintptr_t cfa_offset);                                                /* (MEMLOC_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_hstackind(struct Dee_function_generator *__restrict self, uintptr_t cfa_offset, ptrdiff_t val_delta);                        /* (MEMLOC_F_NOREF) */
#define Dee_function_generator_vpush_addr(self, addr)     Dee_function_generator_vpush_const(self, (DeeObject *)(void *)(addr))
#define Dee_function_generator_vpush_imm8(self, imm8)     Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(uint8_t)(imm8))
#define Dee_function_generator_vpush_Simm8(self, Simm8)   Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(intptr_t)(int8_t)(Simm8))
#define Dee_function_generator_vpush_imm16(self, imm16)   Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(uint16_t)(imm16))
#define Dee_function_generator_vpush_Simm16(self, Simm16) Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(intptr_t)(int16_t)(Simm16))
#define Dee_function_generator_vpush_imm32(self, imm32)   Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(uint32_t)(imm32))
#define Dee_function_generator_vpush_Simm32(self, Simm32) Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(intptr_t)(int32_t)(Simm32))
#define Dee_function_generator_vpush_immSIZ(self, immSIZ) Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(size_t)(immSIZ))
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_arg(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, uint16_t aid); /* `instr' is needed for `libhostasm_rt_err_unbound_arg' */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_arg_present(struct Dee_function_generator *__restrict self, uint16_t aid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_local(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, size_t lid); /* `instr' is needed for `libhostasm_rt_err_unbound_local' (if NULL, no bound-check is done) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdup_n(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
#define Dee_function_generator_vdup(self) Dee_function_generator_vdup_n(self, 1)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpopmany(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_n(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_local(struct Dee_function_generator *__restrict self, size_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_local(struct Dee_function_generator *__restrict self, size_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vop(struct Dee_function_generator *__restrict self, uint16_t operator_name, Dee_vstackaddr_t argc);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopv(struct Dee_function_generator *__restrict self, uint16_t operator_name, Dee_vstackaddr_t argc); /* doesn't leave result on-stack */

/* Helper wrappers to do checked operations on local variables as usercode sees them. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_ulocal(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_ulocal(struct Dee_function_generator *__restrict self, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_ulocal(struct Dee_function_generator *__restrict self, uint16_t lid);

/* Helper macros for operating on "extra" locals (s.a. `DEE_MEMSTATE_EXTRA_LOCAL_*') */
#define Dee_function_generator_vpush_xlocal(self, xlid) Dee_function_generator_vpush_local(self, NULL, (size_t)(self)->fg_assembler->fa_localc + (xlid))
#define Dee_function_generator_vpop_xlocal(self, xlid)  Dee_function_generator_vpop_local(self, (size_t)(self)->fg_assembler->fa_localc + (xlid))
#define Dee_function_generator_vdel_xlocal(self, xlid)  Dee_function_generator_vdel_local(self, (size_t)(self)->fg_assembler->fa_localc + (xlid))

/* Push the "this" argument of thiscall functions. */
#define Dee_function_generator_vpush_this(self) Dee_function_generator_vpush_xlocal(self, DEE_MEMSTATE_EXTRA_LOCAL_A_THIS)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_argc(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_argv(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_usage(struct Dee_function_generator *__restrict self, Dee_host_regusage_t usage);

/* Perform a conditional jump to `desc' based on `jump_if_true'
 * @param: instr: Pointer to start of deemon jmp-instruction (for bb-truncation, and error message)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vjcc(struct Dee_function_generator *__restrict self,
                            struct Dee_jump_descriptor *desc,
                            Dee_instruction_t const *instr, bool jump_if_true);

/* >> TOP = *(TOP + ind_delta); */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vind(struct Dee_function_generator *__restrict self, ptrdiff_t ind_delta);

/* >> *(SECOND + ind_delta) = POP(); */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpopind(struct Dee_function_generator *__restrict self, ptrdiff_t ind_delta);

/* >> TOP = TOP + val_delta; */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdelta(struct Dee_function_generator *__restrict self, ptrdiff_t val_delta);

/* >> temp = *(SECOND + ind_delta);
 * >> *(SECOND + ind_delta) = FIRST;
 * >> FIRST = temp; */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vxch_ind(struct Dee_function_generator *__restrict self, ptrdiff_t ind_delta);

/* Ensure that the top-most `DeeObject' from the object-stack is a reference. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vref(struct Dee_function_generator *__restrict self);

/* Force vtop into a register (ensuring it has type `MEMLOC_TYPE_HREG') */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vreg(struct Dee_function_generator *__restrict self, Dee_host_register_t const *not_these);
/* Force vtop onto the stack (ensuring it has type `MEMLOC_TYPE_HSTACKIND, v_hstack.s_off = 0') */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vflush(struct Dee_function_generator *__restrict self);

/* Generate code to push a global variable onto the virtual stack. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_global(struct Dee_function_generator *__restrict self, uint16_t gid, bool ref);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid, bool ref);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_global(struct Dee_function_generator *__restrict self, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_global(struct Dee_function_generator *__restrict self, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_static(struct Dee_function_generator *__restrict self, uint16_t sid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_static(struct Dee_function_generator *__restrict self, uint16_t sid);

/* Check if `loc' differs from vtop, and if so: move vtop
 * *into* `loc', the assign the *exact* given `loc' to vtop. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vsetloc(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc const *loc);

/* Return and pass the top-most stack element as function result.
 * This function will clear the stack and unbind all local variables. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vret(struct Dee_function_generator *__restrict self);

/* Generate host text to invoke `api_function' with the top-most `argc' items from the stack.
 * @param: cc: One of `VCALLOP_CC_*', describing the calling-convention of `api_function' */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallapi_(struct Dee_function_generator *__restrict self,
                                 void const *api_function, unsigned int cc,
                                 Dee_vstackaddr_t argc);
#define Dee_function_generator_vcallapi(self, api_function, cc, argc) \
	Dee_function_generator_vcallapi_(self, (void const *)(api_function), cc, argc)
#define VCALLOP_CC_OBJECT          0 /* DREF DeeObject *(DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]); */
#define VCALLOP_CC_INT             1 /* int (DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]);      (doesn't push anything onto the vstack) */
#define VCALLOP_CC_RAWINT          2 /* intptr_t (DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]); (Push the returned value onto the vstack) */
#define VCALLOP_CC_RAWINT_KEEPARGS 3 /* intptr_t (DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]); (Push the returned value onto the vstack, and don't pop arguments) */
#define VCALLOP_CC_VOID            4 /* void (DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]);     (doesn't push anything onto the vstack) */
#define VCALLOP_CC_EXCEPT          5 /* ? (DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]);        (always jumps to exception handling) */

/* After a call to `Dee_function_generator_vcallapi()' with `VCALLOP_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALLOP_CC_OBJECT'
 * The difference to directly passing `VCALLOP_CC_OBJECT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckobj(struct Dee_function_generator *__restrict self);

/* After a call to `Dee_function_generator_vcallapi()' with `VCALLOP_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALLOP_CC_INT'
 * The difference to directly passing `VCALLOP_CC_INT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 * NOTE: This function pops one element from the V-stack.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckint(struct Dee_function_generator *__restrict self);

/* Arrange the top `argc' stack-items linearly, such that they all appear somewhere in memory
 * (probably on the host-stack), in consecutive order (with `vtop' at the greatest address,
 * and STACK[SIZE-argc] appearing at the lowest address). Once that has been accomplished,
 * push a value onto the vstack that describes the base-address (that is a `DeeObject **'
 * pointing to `STACK[SIZE-argc]') of the linear vector.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vlinear(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t argc);

/* Generate a call to a C-function `api_function' with `argc'
 * pointer-sized arguments whose values are taken from `argv'.
 * NOTE: This function is allowed to modify `argv' to keep track of internal temporaries
 * NOTE: The given `api_function' is assumed to use the `DCALL' calling convention. */
INTDEF WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gcallapi(struct Dee_function_generator *__restrict self,
                                 void const *api_function, size_t argc,
                                 struct Dee_memloc *argv);


/* Object reference count incref/decref */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gincref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc, Dee_refcnt_t n);  /* NOTE: Might alter `loc' into a register! */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gdecref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc, Dee_refcnt_t n);  /* NOTE: Might alter `loc' into a register! */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gdecref_nokill(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc, Dee_refcnt_t n);  /* NOTE: Might alter `loc' into a register! */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxincref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc, Dee_refcnt_t n); /* NOTE: Might alter `loc' into a register! */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxdecref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc, Dee_refcnt_t n); /* NOTE: Might alter `loc' into a register! */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxdecref_nokill(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc, Dee_refcnt_t n); /* NOTE: Might alter `loc' into a register! */
#define _Dee_function_generator_gincref_regx(self, regno, reg_offset, n)         _Dee_host_section_gincref_regx((self)->fg_sect, regno, reg_offset, n)
#define _Dee_function_generator_gdecref_regx_nokill(self, regno, reg_offset, n)  _Dee_host_section_gdecref_regx_nokill((self)->fg_sect, regno, reg_offset, n)
#define _Dee_function_generator_gincref_const(self, value, n)                    _Dee_host_section_gincref_const((self)->fg_sect, value, n)
#define _Dee_function_generator_gdecref_const(self, value, n)                    _Dee_host_section_gdecref_const((self)->fg_sect, value, n)
#define _Dee_function_generator_gxincref_regx(self, regno, reg_offset, n)        _Dee_host_section_gxincref_regx((self)->fg_sect, regno, reg_offset, n)
#define _Dee_function_generator_gxdecref_regx_nokill(self, regno, reg_offset, n) _Dee_host_section_gxdecref_regx_nokill((self)->fg_sect, regno, reg_offset, n)
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gincref_regx(struct Dee_host_section *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gdecref_regx_nokill(struct Dee_host_section *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gdecref_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gincref_const(struct Dee_host_section *__restrict self, DeeObject *value, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gdecref_const(struct Dee_host_section *__restrict self, DeeObject *value, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gxincref_regx(struct Dee_host_section *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gxdecref_regx_nokill(struct Dee_host_section *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gxdecref_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);

/* Change `loc' into the value of `<loc> = *(<loc> + ind_delta)'
 * Note that unlike the `Dee_function_generator_gmov*' functions, this
 * one may use `MEMLOC_TYPE_*IND' to defer the indirection until later. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gind(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *loc, ptrdiff_t ind_delta);

/* Force `loc' to become a register (`MEMLOC_TYPE_HREG'). */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_greg(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *loc,
                            Dee_host_register_t const *not_these);

/* Force `loc' to reside on the stack, giving it an address (`MEMLOC_TYPE_HSTACKIND, v_hstack.s_off = 0'). */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gflush(struct Dee_function_generator *__restrict self,
                              struct Dee_memloc *loc);

/* Controls for operating with R/W-locks (as needed for accessing global/extern variables) */
#define Dee_function_generator_grwlock_read(self, lock)     _Dee_function_generator_grwlock_read(self, lock)
#define Dee_function_generator_grwlock_write(self, lock)    _Dee_function_generator_grwlock_write(self, lock)
#define Dee_function_generator_grwlock_endread(self, lock)  _Dee_function_generator_grwlock_endread(self, lock)
#define Dee_function_generator_grwlock_endwrite(self, lock) _Dee_function_generator_grwlock_endwrite(self, lock)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_read(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_write(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_endread(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_endwrite(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);

/* Allocate/deallocate memory from the host stack.
 * If stack memory gets allocated, zero-initialize it. */
#define Dee_function_generator_ghstack_adjust(self, cfa_delta)                 (unlikely(_Dee_function_generator_ghstack_adjust(self, cfa_delta)) ? -1 : (Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER), 0))
#define Dee_function_generator_ghstack_pushreg(self, src_regno)                (unlikely(_Dee_function_generator_ghstack_pushreg(self, src_regno)) ? -1 : (Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER), 0))
#define Dee_function_generator_ghstack_pushregind(self, src_regno, src_delta)  (unlikely(_Dee_function_generator_ghstack_pushregind(self, src_regno, src_delta)) ? -1 : (Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER), 0))
#define Dee_function_generator_ghstack_pushconst(self, value)                  (unlikely(_Dee_function_generator_ghstack_pushconst(self, value)) ? -1 : (Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER), 0))
#define Dee_function_generator_ghstack_pushhstackind(self, cfa_offset)         (unlikely(_Dee_function_generator_ghstack_pushhstackind(self, cfa_offset)) ? -1 : (Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER), 0))
#define Dee_function_generator_ghstack_popreg(self, dst_regno)                 (unlikely(_Dee_function_generator_ghstack_popreg(self, dst_regno)) ? -1 : (Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER), 0))
#define _Dee_function_generator_ghstack_adjust(self, alloc_delta)              _Dee_host_section_ghstack_adjust((self)->fg_sect, alloc_delta)
#define _Dee_function_generator_ghstack_pushreg(self, src_regno)               _Dee_host_section_ghstack_pushreg((self)->fg_sect, src_regno)
#define _Dee_function_generator_ghstack_pushregind(self, src_regno, src_delta) _Dee_host_section_ghstack_pushregind((self)->fg_sect, src_regno, src_delta)
#define _Dee_function_generator_ghstack_pushconst(self, value)                 _Dee_host_section_ghstack_pushconst((self)->fg_sect, value)
#define _Dee_function_generator_ghstack_pushhstackind(self, sp_offset)         _Dee_host_section_ghstack_pushhstackind((self)->fg_sect, sp_offset) /* `sp_offset' is as it would be *before* the push */
#define _Dee_function_generator_ghstack_popreg(self, dst_regno)                _Dee_host_section_ghstack_popreg((self)->fg_sect, dst_regno)
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_adjust(struct Dee_host_section *__restrict self, ptrdiff_t alloc_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushreg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushregind(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushconst(struct Dee_host_section *__restrict self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushhstackind(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset); /* `sp_offset' is as it would be *before* the push */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_popreg(struct Dee_host_section *__restrict self, Dee_host_register_t dst_regno);
#define Dee_function_generator_gmov_reg2hstackind(self, src_regno, cfa_offset)         _Dee_function_generator_gmov_reg2hstackind(self, src_regno, Dee_memstate_hstack_cfa2sp((self)->fg_state, cfa_offset))
#define Dee_function_generator_gmov_hstack2reg(self, cfa_offset, dst_regno)            (unlikely(_Dee_function_generator_gmov_hstack2reg(self, Dee_memstate_hstack_cfa2sp((self)->fg_state, cfa_offset), dst_regno)) ? -1 : ((self)->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC, 0))
#define Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno)         (unlikely(_Dee_function_generator_gmov_hstackind2reg(self, Dee_memstate_hstack_cfa2sp((self)->fg_state, cfa_offset), dst_regno)) ? -1 : ((self)->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC, 0))
#define Dee_function_generator_gmov_const2reg(self, value, dst_regno)                  (unlikely(_Dee_function_generator_gmov_const2reg(self, value, dst_regno)) ? -1 : ((self)->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC, 0))
#define Dee_function_generator_gmov_const2regind(self, value, dst_regno, dst_delta)    _Dee_function_generator_gmov_const2regind(self, value, dst_regno, dst_delta)
#define Dee_function_generator_gmov_const2hstackind(self, value, cfa_offset)           _Dee_function_generator_gmov_const2hstackind(self, value, Dee_memstate_hstack_cfa2sp((self)->fg_state, cfa_offset))
#define Dee_function_generator_gmov_const2constind(self, value, p_value)               _Dee_function_generator_gmov_const2constind(self, value, p_value)
#define Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno)                (unlikely(_Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno)) ? -1 : ((self)->fg_state->ms_rusage[dst_regno] = (self)->fg_state->ms_rusage[src_regno], 0))
#define Dee_function_generator_gmov_regx2reg(self, src_regno, src_delta, dst_regno)    (unlikely(_Dee_function_generator_gmov_regx2reg(self, src_regno, src_delta, dst_regno)) ? -1 : ((self)->fg_state->ms_rusage[dst_regno] = (src_delta) == 0 ? (self)->fg_state->ms_rusage[src_regno] : DEE_HOST_REGUSAGE_GENERIC, 0))
#define Dee_function_generator_gmov_regind2reg(self, src_regno, src_delta, dst_regno)  (unlikely(_Dee_function_generator_gmov_regind2reg(self, src_regno, src_delta, dst_regno)) ? -1 : ((self)->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC, 0))
#define Dee_function_generator_gmov_reg2regind(self, src_regno, dst_regno, dst_delta)  _Dee_function_generator_gmov_reg2regind(self, src_regno, dst_regno, dst_delta)
#define Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno)             (unlikely(_Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno)) ? -1 : ((self)->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC, 0))
#define Dee_function_generator_gmov_reg2constind(self, src_regno, p_value)             _Dee_function_generator_gmov_reg2constind(self, src_regno, p_value)
#define _Dee_function_generator_gmov_reg2hstackind(self, src_regno, sp_offset)         _Dee_host_section_gmov_reg2hstackind((self)->fg_sect, src_regno, sp_offset)
#define _Dee_function_generator_gmov_hstack2reg(self, sp_offset, dst_regno)            _Dee_host_section_gmov_hstack2reg((self)->fg_sect, sp_offset, dst_regno)
#define _Dee_function_generator_gmov_hstackind2reg(self, sp_offset, dst_regno)         _Dee_host_section_gmov_hstackind2reg((self)->fg_sect, sp_offset, dst_regno)
#define _Dee_function_generator_gmov_const2reg(self, value, dst_regno)                 _Dee_host_section_gmov_const2reg((self)->fg_sect, value, dst_regno)
#define _Dee_function_generator_gmov_const2regind(self, value, dst_regno, dst_delta)   _Dee_host_section_gmov_const2regind((self)->fg_sect, value, dst_regno, dst_delta)
#define _Dee_function_generator_gmov_const2hstackind(self, value, sp_offset)           _Dee_host_section_gmov_const2hstackind((self)->fg_sect, value, sp_offset)
#define _Dee_function_generator_gmov_const2constind(self, value, p_value)              _Dee_host_section_gmov_const2constind((self)->fg_sect, value, p_value)
#define _Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno)               _Dee_host_section_gmov_reg2reg((self)->fg_sect, src_regno, dst_regno)
#define _Dee_function_generator_gmov_regx2reg(self, src_regno, src_delta, dst_regno)   _Dee_host_section_gmov_regx2reg((self)->fg_sect, src_regno, src_delta, dst_regno)
#define _Dee_function_generator_gmov_regind2reg(self, src_regno, src_delta, dst_regno) _Dee_host_section_gmov_regind2reg((self)->fg_sect, src_regno, src_delta, dst_regno)
#define _Dee_function_generator_gmov_reg2regind(self, src_regno, dst_regno, dst_delta) _Dee_host_section_gmov_reg2regind((self)->fg_sect, src_regno, dst_regno, dst_delta)
#define _Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno)            _Dee_host_section_gmov_constind2reg((self)->fg_sect, p_value, dst_regno)
#define _Dee_function_generator_gmov_reg2constind(self, src_regno, p_value)            _Dee_host_section_gmov_reg2constind((self)->fg_sect, src_regno, p_value)
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_reg2hstackind(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t sp_offset);                             /* *(SP + sp_offset) = dst_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_hstack2reg(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset, Dee_host_register_t dst_regno);                                /* dst_regno = (SP + sp_offset); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_hstackind2reg(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset, Dee_host_register_t dst_regno);                             /* dst_regno = *(SP + sp_offset); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_const2reg(struct Dee_host_section *__restrict self, DeeObject *value, Dee_host_register_t dst_regno);                                    /* dst_regno = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_const2regind(struct Dee_host_section *__restrict self, DeeObject *value, Dee_host_register_t dst_regno, ptrdiff_t dst_delta);            /* *(dst_regno + dst_delta) = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_const2hstackind(struct Dee_host_section *__restrict self, DeeObject *value, ptrdiff_t sp_offset);                                        /* *(SP + sp_offset) = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_const2constind(struct Dee_host_section *__restrict self, DeeObject *value, DeeObject **p_value);                                         /* *<p_value> = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_regx2reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno);   /* dst_regno = src_regno + src_delta; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_regind2reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno); /* dst_regno = *(src_regno + src_delta); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_reg2regind(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, Dee_host_register_t dst_regno, ptrdiff_t dst_delta); /* *(dst_regno * dst_delta) = src_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_constind2reg(struct Dee_host_section *__restrict self, DeeObject **p_value, Dee_host_register_t dst_regno);                              /* dst_regno = *<p_value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_reg2constind(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, DeeObject **p_value);                              /* *<p_value> = src_regno; */
#define _Dee_host_section_gmov_reg2reg(self, src_regno, dst_regno) _Dee_host_section_gmov_regx2reg(self, src_regno, 0, dst_regno)

INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_const2loc(struct Dee_function_generator *__restrict self, DeeObject *value, struct Dee_memloc const *__restrict dst_loc);                                                   /* <dst_loc> = value; */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL Dee_function_generator_gmov_regx2loc(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, struct Dee_memloc const *__restrict dst_loc);                  /* <dst_loc> = src_regno + src_delta; */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_hstack2loc(struct Dee_function_generator *__restrict self, uintptr_t cfa_offset, struct Dee_memloc const *__restrict dst_loc);                                              /* <dst_loc> = (SP ... cfa_offset); */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_loc2regx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, Dee_host_register_t dst_regno, ptrdiff_t dst_delta);                  /* dst_regno = <src_loc> - dst_delta; */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL Dee_function_generator_gmov_loc2regy(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, Dee_host_register_t dst_regno, ptrdiff_t *__restrict p_dst_delta); /* dst_regno = <src_loc> - *p_dst_delta; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_locind2reg(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, ptrdiff_t src_delta, Dee_host_register_t dst_regno);                /* dst_regno = *(<src_loc> + src_delta); */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_reg2locind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, struct Dee_memloc const *__restrict dst_loc, ptrdiff_t dst_delta);                /* *(<dst_loc> + dst_delta) = src_regno; */
#define Dee_function_generator_gmov_reg2loc(self, src_regno, dst_loc) Dee_function_generator_gmov_regx2loc(self, src_regno, 0, dst_loc)
#define Dee_function_generator_gmov_loc2reg(self, src_loc, dst_regno) Dee_function_generator_gmov_loc2regx(self, src_loc, dst_regno, 0)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_ghstack_pushlocx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, ptrdiff_t dst_delta);                              /* PUSH(<src_loc> - dst_delta); */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_loc2hstackindx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, uintptr_t dst_cfa_offset, ptrdiff_t dst_delta); /* *<SP@dst_cfa_offset> = <src_loc> - dst_delta; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_loc2constind(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, DeeObject **p_value, ptrdiff_t dst_delta);        /* *<p_value> = <src_loc> - dst_delta; */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_const2locind(struct Dee_function_generator *__restrict self, DeeObject *value, struct Dee_memloc const *__restrict dst_loc, ptrdiff_t dst_delta); /* *(<dst_loc> + dst_delta) = value; */
#define Dee_function_generator_ghstack_pushloc(self, src_loc)                    Dee_function_generator_ghstack_pushlocx(self, src_loc, 0)
#define Dee_function_generator_gmov_loc2hstackind(self, src_loc, dst_cfa_offset) Dee_function_generator_gmov_loc2hstackindx(self, src_loc, dst_cfa_offset, 0)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_gmov_loc2loc(struct Dee_function_generator *__restrict self, struct Dee_memloc *src_loc, struct Dee_memloc const *dst_loc);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_gmov_loc2locind(struct Dee_function_generator *__restrict self, struct Dee_memloc *src_loc, struct Dee_memloc const *dst_loc, ptrdiff_t ind_delta);

/* Generate code to return `loc'. No extra code to decref stack/locals is generated. If you
 * want that extra code to be generated, you need to use `Dee_function_generator_vret()'. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gret(struct Dee_function_generator *__restrict self, /*inherit_ref*/ struct Dee_memloc *__restrict loc);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gret(struct Dee_function_generator *__restrict self);

/* Allocate at host register, possibly flushing an already used register to stack.
 * @param: not_these: Array of registers not to allocated, terminated by one `>= HOST_REGISTER_COUNT'.
 * @return: * : The allocated register
 * @return: >= HOST_REGISTER_COUNT: Error */
INTDEF WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gallocreg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t const *not_these);
#define Dee_function_generator_gtryallocreg(self, not_these) \
	Dee_memstate_hregs_find_unused_ex((self)->fg_state, not_these)

/* Helper that returns a register that's been populated for `usage' */
INTDEF WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gusagereg(struct Dee_function_generator *__restrict self,
                                 Dee_host_regusage_t usage,
                                 Dee_host_register_t const *dont_alloc_these);


/* Generate code to flush all registers used by the deemon stack/locals into the host stack.
 * NOTE: Usage-registers are cleared by arch-specific code (e.g. `_Dee_function_generator_gcallapi()')
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references.
 * @param: only_if_reference: Only flush locations that contain references. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushregs(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                  bool only_if_reference);

/* Generate code to assert that location `loc' is non-NULL:
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_LOCAL, lid, <ignored>, NULL, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_GLOBAL, gid, <ignored>, NULL, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_EXTERN, mid, gid, NULL, NULL);
 * The `kind', `id1' and `id2' arguments simply select `Dee_function_generator_gthrow_*_unbound()'
 * @param: opt_endread_before_throw: When non-NULL, emit `Dee_function_generator_grwlock_endread()'
 *                                   before the `Dee_function_generator_gthrow_*_unbound' code. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gassert_bound(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc *loc, Dee_instruction_t const *instr,
                                     uint8_t kind, uint16_t id1, uint16_t id2,
                                     Dee_atomic_rwlock_t *opt_endread_before_throw,
                                     Dee_atomic_rwlock_t *opt_endwrite_before_throw);
#define Dee_function_generator_gassert_local_bound(self, instr, lid) \
	((lid) >= (self)->fg_state->ms_localc                            \
	 ? err_illegal_lid(lid)                                          \
	 : Dee_function_generator_gassert_local_bound_ex(self, &(self)->fg_state->ms_localv[lid], instr, lid))
#define Dee_function_generator_gassert_local_bound_ex(self, loc, instr, lid) \
	Dee_function_generator_gassert_bound(self, loc, instr, ASM_LOCAL, lid, 0, NULL, NULL)
#define Dee_function_generator_gassert_global_bound(self, loc, gid) \
	Dee_function_generator_gassert_bound(self, loc, NULL, ASM_GLOBAL, gid, 0, NULL, NULL)
#define Dee_function_generator_gassert_extern_bound(self, loc, mid, gid) \
	Dee_function_generator_gassert_bound(self, loc, NULL, ASM_EXTERN, mid, gid, NULL, NULL)

/* Generate code to throw an error indicating that local variable `lid'
 * is unbound. This includes any necessary jump for the purpose of entering
 * an exception handler. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_arg_unbound(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, uint16_t aid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_local_unbound(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_global_unbound(struct Dee_function_generator *__restrict self, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_extern_unbound(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);

/* Generate jumps. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL _Dee_function_generator_gjz(struct Dee_function_generator *__restrict self, struct Dee_memloc const *test_loc, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL _Dee_function_generator_gjnz(struct Dee_function_generator *__restrict self, struct Dee_memloc const *test_loc, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_host_section_gjmp(struct Dee_host_section *__restrict self, struct Dee_host_symbol *__restrict dst);
#define _Dee_function_generator_gjmp(self, dst) _Dee_host_section_gjmp((self)->fg_sect, dst)

/* Emit conditional jump(s) based on `<lhs> <=> <rhs>'
 * NOTE: This function may clobber `lhs' and `rhs', and may flush/shift local/stack locations. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gjcmp(struct Dee_function_generator *__restrict self,
                              struct Dee_memloc *lhs, struct Dee_memloc *rhs, bool signed_cmp,
                              struct Dee_host_symbol *dst_lo,  /* Jump here if `<lhs> < <rhs>' */
                              struct Dee_host_symbol *dst_eq,  /* Jump here if `<lhs> == <rhs>' */
                              struct Dee_host_symbol *dst_gr); /* Jump here if `<lhs> > <rhs>' */

/* Generate checks to enter exception handling mode. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gjz_except(struct Dee_function_generator *__restrict self, struct Dee_memloc *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gjnz_except(struct Dee_function_generator *__restrict self, struct Dee_memloc *loc);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gjmp_except(struct Dee_function_generator *__restrict self);

/* Generate code in `self->fg_sect' to morph `self->fg_state' into `new_state' */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vmorph(struct Dee_function_generator *__restrict self,
                              struct Dee_memstate const *new_state);


/* Convert a single deemon instruction `instr' to host assembly and adjust the host memory
 * state according to the instruction in question. This is the core function to parse deemon
 * code and convert it to host assembly.
 * @param: p_next_instr: [inout] Pointer to the next instruction (may be overwritten if the
 *                               generated instruction was merged with its successor, as is
 *                               the case for `ASM_REPR' when followed by print-instructions)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_geninstr(struct Dee_function_generator *__restrict self,
                                Dee_instruction_t const *instr,
                                Dee_instruction_t const **p_next_instr);

/* Wrapper around `Dee_function_generator_geninstr()' to generate the entire basic block.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_genall(struct Dee_function_generator *__restrict self);






/************************************************************************/
/* Core functions to perform the different (re-)compilation steps       */
/************************************************************************/

/* Step #1: Load basic blocks. Fills in:
 * - self->fa_blockc
 * - self->fa_blockv
 * - self->fa_blockv[*]->bb_deemon_start
 * - self->fa_blockv[*]->bb_deemon_end
 * - self->fa_blockv[*]->bb_entries+bb_exits
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_from
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_to
 * - self->fa_blockv[*]->bb_next
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_loadblocks(struct Dee_function_assembler *__restrict self);

/* Step #2: Compile basic blocks and determine memory states. Fills in:
 * - self->fa_prolog
 * - self->fa_prolog_end
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_stat
 * - self->fa_blockv[*]->bb_mem_start
 * - self->fa_blockv[*]->bb_mem_end
 * - self->fa_blockv[*]->bb_host_start
 * - self->fa_blockv[*]->bb_host_end
 * Also makes sure that memory states at start/end of basic blocks are
 * always identical (or compatible; i.e.: MEMLOC_F_LOCAL_UNKNOWN can
 * be set at the start of a block, but doesn't need to be set at the
 * end of a preceding block). When not compatible, extra block(s) are
 * inserted with `bb_deemon_start==bb_deemon_end', but non-empty host
 * assembly, which serves the purpose of transforming memory states.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_compileblocks(struct Dee_function_assembler *__restrict self);

/* Step #3: Trim basic blocks (and parts thereof) that turned out to be unreachable.
 * This extra step is needed for when part of a basic block only turns out to be
 * unreachable during the compileblocks-phase, as would be the case in code like this:
 *       >>     push true
 *       >>     jf   pop, 1f
 *       >>     ret
 *       >> 1:  print @"NEVER REACHED", nl
 *       >>     ret
 * This function also gets rid of exception handling basic blocks that are never used
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_trimdead(struct Dee_function_assembler *__restrict self);

/* Step #4: Generate morph instruction sequences to perform memory state transitions.
 * This also extends the host text of basic blocks that fall through to some
 * other basic block with an extra instructions needed for morphing:
 * - self->fa_prolog                                (to transition )
 * - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph
 * - self->fa_blockv[*]->bb_htext                   (extend with transition code so that `bb_mem_end == bb_next->bb_mem_start')
 * - self->fa_except_exitv[*]->exi_block->bb_htext  (generate morph-code to transition to an empty stack, or fall into another exit block)
 * - self->fa_except_exitv[*]->exi_block->bb_next   (set if intend is to fall into another exit block)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_compilemorph(struct Dee_function_assembler *__restrict self);

/* Step #5: Generate missing unconditional jumps to jump from one block to the next
 * - Find loops of blocks that "fall through" back on each other in a loop, and
 *   append a jump-to-the-start on all blocks that "fall through" to themselves.
 *   For one of these blocks, also generate a call to `DeeThread_CheckInterrupt()'
 * - For all blocks that have more than 1 fallthru predecessors, take all but
 *   1 of those predecessors and append unconditional jumps to them, then set
 *   the `bb_next' field of those blocks to `NULL'.
 * - Also fills in:
 *   - self->fa_prolog.hs_link
 *   - self->fa_blockv[*]->bb_htext.hs_link
 *   - self->fa_blockv[*]->bb_hcold.hs_link
 *   - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph.hs_link
 *   - self->fa_except_exitv[*]->exi_block->bb_htext.hs_link
 *   - self->fa_except_exitv[*]->exi_block->bb_hcold.hs_link
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_stitchblocks(struct Dee_function_assembler *__restrict self);

/* Step #6: Link blocks into an executable function blob.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_assembler_linkblocks(struct Dee_function_assembler *__restrict self,
                                  struct Dee_hostfunc *__restrict result);




/* Debug helpers/config */
#undef NO_HOSTASM_DEBUG_PRINT
#if defined(Dee_DPRINT_IS_NOOP) || 0
#define NO_HOSTASM_DEBUG_PRINT
#endif /* ... */
#ifndef NO_HOSTASM_DEBUG_PRINT
INTDEF NONNULL((1)) void DCALL
_Dee_memstate_debug_print(struct Dee_memstate const *__restrict self,
                          struct Dee_function_assembler *assembler,
                          Dee_instruction_t const *instr);
#endif /* !NO_HOSTASM_DEBUG_PRINT */



/* Error throwing helper functions. */
INTDEF ATTR_COLD int DCALL err_illegal_stack_effect(void);
INTDEF ATTR_COLD int DCALL err_illegal_lid(uint16_t lid);
INTDEF ATTR_COLD int DCALL err_illegal_mid(uint16_t mid);
INTDEF ATTR_COLD int DCALL err_illegal_aid(uint16_t aid);
INTDEF ATTR_COLD int DCALL err_illegal_cid(uint16_t cid);
#define err_illegal_sid(sid) err_illegal_cid(sid)
INTDEF ATTR_COLD int DCALL err_illegal_rid(uint16_t rid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_illegal_gid(struct Dee_module_object *__restrict mod, uint16_t gid);


/************************************************************************/
/* RUNTIME API FUNCTIONS CALLED BY GENERATED CODE                       */
/************************************************************************/
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_unbound_global(struct Dee_module_object *__restrict mod, uint16_t global_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL libhostasm_rt_err_unbound_local(struct code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL libhostasm_rt_err_unbound_arg(struct code_object *code, void *ip, uint16_t arg_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL libhostasm_rt_err_illegal_instruction(DeeCodeObject *code, void *ip);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL libhostasm_rt_DeeObject_ShlRepr(DeeObject *lhs, DeeObject *rhs);

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_LIBHOSTASM_H */
