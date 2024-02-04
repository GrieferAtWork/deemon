/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
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
#include <deemon/object.h>
#include <deemon/objmethod.h> /* Dee_cmethod_t */
#include <deemon/system-features.h>

#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>

#include <stdbool.h>
#include <stdint.h>

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
 * - Add a heuristic to DeeFunctionObject that keeps track of how often
 *   the function has been called. If that count exceeds some limit,
 *   automatically replace the function with its hostasm version.
 *   - When a Function is called "n" times, re-compile it w/o HOSTFUNC_CC_F_FUNC
 *   - When a Code is used to create a Function "n" times, re-compile it w/ HOSTFUNC_CC_F_FUNC
 * - Hostasm re-compilation should take place in a separate thread that
 *   operates as a lazily-launched daemon, and takes re-compilation jobs
 *   as they come up.
 * - Add support for functions with custom exception handlers/finally
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

#ifdef DEE_SOURCE
#define Dee_code_object     code_object
#define Dee_module_object   module_object
#define Dee_module_symbol   module_symbol
#define Dee_class_attribute class_attribute
#endif /* DEE_SOURCE */

/* Debug helpers/config */
#undef NO_HOSTASM_DEBUG_PRINT
#if defined(Dee_DPRINT_IS_NOOP) || 0
#define NO_HOSTASM_DEBUG_PRINT
#endif /* ... */
#ifndef NO_HOSTASM_DEBUG_PRINT
struct Dee_memstate;
struct Dee_function_assembler;
struct Dee_memadr;
struct Dee_memloc;
struct Dee_memobj;
struct Dee_memval;
struct Dee_memequiv;
struct Dee_memequivs;
INTDEF NONNULL((1)) void DCALL
_Dee_memstate_debug_print(struct Dee_memstate const *__restrict self,
                          struct Dee_function_assembler *assembler,
                          Dee_instruction_t const *instr);
INTDEF NONNULL((1)) void DCALL _Dee_memadr_debug_print(struct Dee_memadr const *__restrict self, ptrdiff_t val_delta);
INTDEF NONNULL((1)) void DCALL _Dee_memloc_debug_print(struct Dee_memloc const *__restrict self);
INTDEF NONNULL((1)) void DCALL _Dee_memobj_debug_print(struct Dee_memobj const *__restrict self, bool is_local, bool noref);
INTDEF NONNULL((1)) void DCALL _Dee_memval_debug_print(struct Dee_memval const *__restrict self, bool is_local);
#define _Dee_memequiv_debug_print(self) _Dee_memloc_debug_print(&(self)->meq_loc)
INTDEF NONNULL((1)) void DCALL _Dee_memequivs_debug_print(struct Dee_memequivs const *__restrict self);
#define HA_print(s)    Dee_DPRINT("hostasm:" s)
#define HA_printf(...) Dee_DPRINTF("hostasm:" __VA_ARGS__)
#else /* !NO_HOSTASM_DEBUG_PRINT */
#define HA_print(s)    (void)0
#define HA_printf(...) (void)0
#endif /* NO_HOSTASM_DEBUG_PRINT */


#define RANGES_OVERLAP(a_start, a_end, b_start, b_end) \
	((a_end) > (b_start) && (a_start) < (b_end))


/* TODO: Add a dedicated type for CFA offsets (and then make it be signed) */
typedef intptr_t Dee_cfa_t;
typedef uint32_t Dee_vstackaddr_t;
typedef int32_t Dee_vstackoff_t;
typedef uint16_t Dee_aid_t;
typedef uint32_t Dee_lid_t;
typedef uint16_t Dee_ulid_t;

struct Dee_memadr; /* Low level value address */
struct Dee_memloc; /* Low level value location (address of optional value delta) */
struct Dee_memobj; /* High-level object value (encapsulates Dee_memloc and adds extra deemon-object related meta-data) */
struct Dee_memval; /* High-level value (encapsulates 1..n Dee_memloc that may be morphed into a deemon value) */

struct Dee_memadr {
	/* Low level value address */
#define MEMADR_TYPE_CONST     0 /* >> value = ma_val.v_const; */
/*      MEMADR_TYPE_CONSTIND  1  * >> value = *(byte_t **)ma_val.v_const; */
#define MEMADR_TYPE_UNDEFINED 2 /* >> value = ???;  // Not defined (value can be anything at runtime) */
/*      MEMADR_TYPE_          3  * ... */
#define MEMADR_TYPE_HSTACK    4 /* >> value = CFA(%sp, ma_val.v_cfa); */
#define MEMADR_TYPE_HSTACKIND 5 /* >> value = *(byte_t **)CFA(%sp, ma_val.v_cfa); */
#define MEMADR_TYPE_HREG      6 /* >> value = %ma_reg; */
#define MEMADR_TYPE_HREGIND   7 /* >> value = *(byte_t **)(%ma_reg + ma_val.v_indoff); */
#define MEMADR_TYPE_HASREG(typ) ((typ) >= MEMADR_TYPE_HREG)
#define MEMADR_TYPE_CASEREG     case MEMADR_TYPE_HREG: case MEMADR_TYPE_HREGIND
	uint8_t             ma_typ;     /* Location kind (one of `MEMADR_TYPE_*') */
	Dee_host_register_t ma_reg;     /* [valid_if(MEMADR_TYPE_HREG, MEMADR_TYPE_HREGIND)] Register number (or 0 if not valid). */
	uint8_t            _ma_zro[sizeof(void *) - 2]; /* Extra padding space (must be 0-initialized) */
	union {
		uintptr_t      _v_zero;     /* [valid_if(ma_type in [MEMADR_TYPE_HREG, MEMADR_TYPE_UNDEFINED, MEMADR_TYPE_UNALLOC])] Always zero */
		void const     *v_const;    /* [valid_if(ma_type == MEMADR_TYPE_CONST)] Known, constant value */
		DeeObject      *v_constobj; /* [valid_if(ma_type == MEMADR_TYPE_CONST)] Known, constant value */
		ptrdiff_t       v_indoff;   /* [valid_if(ma_type == MEMADR_TYPE_HREGIND)] indirection offset. */
		Dee_cfa_t       v_cfa;      /* [valid_if(ma_type in [MEMADR_TYPE_HSTACK, MEMADR_TYPE_HSTACKIND])] CFA offset. */
		struct Dee_memadr *_v_nextadr; /* Used internally */
		struct Dee_memobj *_v_nextobj; /* Used internally */
		struct Dee_memloc *_v_nextloc; /* Used internally */
		struct Dee_memval *_v_nextval; /* Used internally */
	}                   ma_val;     /* Location description value. */
};

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _Dee_memadr_init_base(self, type) (((uintptr_t *)(self))[0] = (uintptr_t)((uint8_t)(type)))
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define _Dee_memadr_init_base(self, type) (((uintptr_t *)(self))[0] = (uintptr_t)((uint8_t)(type) << 24))
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
#define Dee_memadr_init_hreg(self, ma_regno_)             \
	(void)(Dee_ASSERT((ma_regno_) < HOST_REGISTER_COUNT), \
	       _Dee_memadr_init_base(self, MEMADR_TYPE_HREG), \
	       (self)->ma_reg         = (ma_regno_),          \
	       (self)->ma_val._v_zero = 0)
#define Dee_memadr_init_hregind(self, ma_regno_, v_indoff_)  \
	(void)(Dee_ASSERT((ma_regno_) < HOST_REGISTER_COUNT),    \
	       _Dee_memadr_init_base(self, MEMADR_TYPE_HREGIND), \
	       (self)->ma_reg          = (ma_regno_),            \
	       (self)->ma_val.v_indoff = (v_indoff_))
#define Dee_memadr_init_hstack(self, v_cfa_)                \
	(void)(_Dee_memadr_init_base(self, MEMADR_TYPE_HSTACK), \
	       (self)->ma_val.v_cfa = (v_cfa_))
#define Dee_memadr_init_hstackind(self, v_cfa_)                \
	(void)(_Dee_memadr_init_base(self, MEMADR_TYPE_HSTACKIND), \
	       (self)->ma_val.v_cfa = (v_cfa_))
#define Dee_memadr_init_const(self, v_const_)              \
	(void)(_Dee_memadr_init_base(self, MEMADR_TYPE_CONST), \
	       (self)->ma_val.v_const = (v_const_))
#define Dee_memadr_init_undefined(self)                        \
	(void)(_Dee_memadr_init_base(self, MEMADR_TYPE_UNDEFINED), \
	       (self)->ma_val._v_zero = 0)

#ifdef __INTELLISENSE__
#define Dee_memadr_hashof(a)     ((uintptr_t)(a)->ma_typ)
#define Dee_memadr_sameadr(a, b) ((a)->ma_typ == (b)->ma_typ)
#else /* __INTELLISENSE__ */
#define Dee_memadr_hashof(a) \
	(((uintptr_t const *)(a))[0] ^ ((uintptr_t const *)(a))[1])
#define Dee_memadr_sameadr(a, b)                                   \
	(((uintptr_t const *)(a))[0] == ((uintptr_t const *)(b))[0] && \
	 ((uintptr_t const *)(a))[1] == ((uintptr_t const *)(b))[1])
#endif /* !__INTELLISENSE__ */


/* Return the CFA start/end addresses. */
#ifdef HOSTASM_STACK_GROWS_DOWN
#define Dee_memadr_getcfastart(self) ((self)->ma_val.v_cfa - HOST_SIZEOF_POINTER)
#define Dee_memadr_getcfaend(self)   ((self)->ma_val.v_cfa)
#else /* HOSTASM_STACK_GROWS_DOWN */
#define Dee_memadr_getcfastart(self) ((self)->ma_val.v_cfa)
#define Dee_memadr_getcfaend(self)   ((self)->ma_val.v_cfa + HOST_SIZEOF_POINTER)
#endif /* !HOSTASM_STACK_GROWS_DOWN */

struct Dee_memloc {
	/* Low level value location (address of optional value delta) */
	struct Dee_memadr ml_adr; /* >> value = VALUE_OF(ml_adr) + ml_off; */
	ptrdiff_t         ml_off; /* [if(ml_adr.ma_typ !in [MEMADR_TYPE_HREG, MEMADR_TYPE_HREGIND, MEMADR_TYPE_HSTACKIND], [== 0])]
	                           * Extra addend added to the effective location value. Must be set to `0' for location
	                           * types where doing so doesn't make sense (see list above of allowed types). */
};

#define _Dee_memloc_init_impl(self, _initbase, ml_off_) \
	(void)(_initbase, (self)->ml_off = (ml_off_))
#define Dee_memloc_init_memadr(self, adr, ml_off_) \
	_Dee_memloc_init_impl(self, (self)->ml_adr = *(adr), ml_off_)
#define Dee_memloc_init_hreg(self, ma_regno_, ml_off_) \
	_Dee_memloc_init_impl(self, Dee_memadr_init_hreg(&(self)->ml_adr, ma_regno_), ml_off_)
#define Dee_memloc_init_hregind(self, ma_regno_, v_indoff_, ml_off_) \
	_Dee_memloc_init_impl(self, Dee_memadr_init_hregind(&(self)->ml_adr, ma_regno_, v_indoff_), ml_off_)
#define Dee_memloc_init_hstack(self, v_cfa_) \
	_Dee_memloc_init_impl(self, Dee_memadr_init_hstack(&(self)->ml_adr, v_cfa_), 0)
#define Dee_memloc_init_hstackind(self, v_cfa_, ml_off_) \
	_Dee_memloc_init_impl(self, Dee_memadr_init_hstackind(&(self)->ml_adr, v_cfa_), ml_off_)
#define Dee_memloc_init_const(self, v_const_) \
	_Dee_memloc_init_impl(self, Dee_memadr_init_const(&(self)->ml_adr, v_const_), 0)
#define Dee_memloc_init_undefined(self) \
	_Dee_memloc_init_impl(self, Dee_memadr_init_undefined(&(self)->ml_adr), 0)

#define Dee_memloc_hashofadr(a)  Dee_memadr_hashof(&(a)->ml_adr)
#define Dee_memloc_sameadr(a, b) Dee_memadr_sameadr(&(a)->ml_adr, &(b)->ml_adr)
#define Dee_memloc_sameloc(a, b) (Dee_memloc_sameadr(a, b) && (a)->ml_off == (b)->ml_off)

#define Dee_memloc_getoff(self) ((ptrdiff_t)(self)->ml_off)
#define Dee_memloc_setoff(self, val)                                   \
	(void)(Dee_ASSERT((self)->ml_adr.ma_typ == MEMADR_TYPE_HREG ||     \
	                  (self)->ml_adr.ma_typ == MEMADR_TYPE_HREGIND ||  \
	                  (self)->ml_adr.ma_typ == MEMADR_TYPE_HSTACKIND), \
	       (self)->ml_off = (val))


#define Dee_memloc_gettyp(self) ((self)->ml_adr.ma_typ)
#define Dee_memloc_getadr(self) (&(self)->ml_adr)

#define Dee_memloc_hasreg(self) MEMADR_TYPE_HASREG(Dee_memloc_gettyp(self))
#define Dee_memloc_getreg(self) ((self)->ml_adr.ma_reg)

#define Dee_memloc_isconst(self)         (Dee_memloc_gettyp(self) == MEMADR_TYPE_CONST)
#define Dee_memloc_isundefined(self)     (Dee_memloc_gettyp(self) == MEMADR_TYPE_UNDEFINED)
#define Dee_memloc_isconstobj(self, obj) (Dee_memloc_isconst(self) && (self)->ml_adr.ma_val.v_const == (obj))
#define Dee_memloc_isnull(self)          Dee_memloc_isconstobj(self, NULL)
#define Dee_memloc_isnone(self)          Dee_memloc_isconstobj(self, Dee_None)

#define Dee_memloc_getcfastart(self) Dee_memadr_getcfastart(&(self)->ml_adr)
#define Dee_memloc_getcfaend(self)   Dee_memadr_getcfaend(&(self)->ml_adr)

#define Dee_memloc_const_getobj(self)             ((DeeObject *)(self)->ml_adr.ma_val.v_const)
#define Dee_memloc_const_setobj(self, v)          (void)((self)->ml_adr.ma_val.v_const = (v))
#define Dee_memloc_const_getaddr(self)            ((byte_t const *)(self)->ml_adr.ma_val.v_const)
#define Dee_memloc_const_setaddr(self, v)         (void)((self)->ml_adr.ma_val.v_const = (v))
#define Dee_memloc_hreg_getreg(self)              Dee_memloc_getreg(self)
#define Dee_memloc_hreg_getvaloff(self)           Dee_memloc_getoff(self)
#define Dee_memloc_hreg_setvaloff(self, val)      Dee_memloc_setoff(self, val)
#define Dee_memloc_hregind_getreg(self)           Dee_memloc_getreg(self)
#define Dee_memloc_hregind_getindoff(self)        ((self)->ml_adr.ma_val.v_indoff)
#define Dee_memloc_hregind_getvaloff(self)        Dee_memloc_getoff(self)
#define Dee_memloc_hregind_setvaloff(self, val)   Dee_memloc_setoff(self, val)
#define Dee_memloc_hstack_getcfa(self)            ((self)->ml_adr.ma_val.v_cfa)
#define Dee_memloc_hstackind_getcfa(self)         ((self)->ml_adr.ma_val.v_cfa)
#define Dee_memloc_hstackind_getvaloff(self)      Dee_memloc_getoff(self)
#define Dee_memloc_hstackind_setvaloff(self, val) Dee_memloc_getoff(self, val)

/* Similar to `Dee_memloc_setoff()', but works for *all* location types. */
LOCAL NONNULL((1)) void DCALL
Dee_memloc_adjoff(struct Dee_memloc *__restrict self, ptrdiff_t val_delta) {
	switch (Dee_memloc_gettyp(self)) {
	case MEMADR_TYPE_UNDEFINED:
		Dee_ASSERT(self->ml_off == 0);
		break;
	case MEMADR_TYPE_CONST:
		self->ml_adr.ma_val.v_const = (byte_t const *)self->ml_adr.ma_val.v_const + val_delta;
		Dee_ASSERT(self->ml_off == 0);
		break;
	case MEMADR_TYPE_HSTACK:
#ifdef HOSTASM_STACK_GROWS_DOWN
		self->ml_adr.ma_val.v_cfa -= val_delta;
#else /* HOSTASM_STACK_GROWS_DOWN */
		self->ml_adr.ma_val.v_cfa += val_delta;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		Dee_ASSERT(self->ml_off == 0);
		break;
	case MEMADR_TYPE_HSTACKIND:
	case MEMADR_TYPE_HREG:
	case MEMADR_TYPE_HREGIND:
		self->ml_off += val_delta;
		break;
	default: __builtin_unreachable();
	}
}






/* Memory location equivalence description.
 * NOTES:
 * - Any equivalence class can only ever contain at most 1 MEMEQUIV_TYPE_CONST item
 * - When a equivalence class contains only 1 item, that item gets removed
 */
#define MEMEQUIV_TYPE_UNUSED      (MEMADR_TYPE_UNDEFINED + 0) /* Unused slot (*never* exists in any value ring) */
#define MEMEQUIV_TYPE_DUMMY       (MEMADR_TYPE_UNDEFINED + 1) /* Dummy slot (*never* exists in any value ring; used for hashvector) */
#define MEMEQUIV_TYPE_HREG        MEMADR_TYPE_HREG            /* Register location */
#define MEMEQUIV_TYPE_HREGIND     MEMADR_TYPE_HREGIND         /* Indirect register location */
#define MEMEQUIV_TYPE_HSTACKIND   MEMADR_TYPE_HSTACKIND       /* Indirect host-stack location */
#define MEMEQUIV_TYPE_CONST       MEMADR_TYPE_CONST           /* Constant */
#define MEMEQUIV_TYPE_HASREG(typ) MEMADR_TYPE_HASREG(typ)

/* Check if "typ" is one of:
 * - MEMADR_TYPE_HREG
 * - MEMADR_TYPE_HREGIND
 * - MEMADR_TYPE_HSTACKIND
 * - MEMADR_TYPE_CONST */
#define MEMEQUIV_TYPE_SUPPORTED(typ) \
	((typ) == MEMADR_TYPE_CONST || (typ) >= MEMADR_TYPE_HSTACKIND)

struct Dee_memequiv {
	struct Dee_memloc         meq_loc;   /* Location described by this equivalence entry. */
	RINGQ_ENTRY(Dee_memequiv) meq_class; /* [valid_if(meq_loc.ma_type !in [MEMEQUIV_TYPE_UNUSED, MEMEQUIV_TYPE_DUMMY])]
	                                      * Other equivalent locations of this class. */
};
#define Dee_memequiv_hashof(self)      Dee_memloc_hashofadr(&(self)->meq_loc)
#define Dee_memequiv_equals(a, b)      Dee_memloc_sameadr(&(a)->meq_loc, &(b)->meq_loc)
#define Dee_memequiv_next(self)        RINGQ_NEXT(self, meq_class)
#define Dee_memequiv_getcfastart(self) Dee_memloc_getcfastart(&(self)->meq_loc)
#define Dee_memequiv_getcfaend(self)   Dee_memloc_getcfaend(&(self)->meq_loc)

LOCAL NONNULL((1, 2)) void DCALL
Dee_memequiv_next_asloc(struct Dee_memequiv const *__restrict self,
                        /*out*/ struct Dee_memloc *__restrict loc) {
	ptrdiff_t valoff;
	ASSERT(Dee_memequiv_next(self) != NULL);
	ASSERT(Dee_memequiv_next(self) != self);
	valoff = Dee_memloc_getoff(&self->meq_loc);
	*loc = Dee_memequiv_next(self)->meq_loc;
	Dee_memloc_adjoff(loc, -valoff);
}


struct Dee_memequivs {
	size_t               meqs_mask; /* Hash-mask of `meqs_list' */
	size_t               meqs_size; /* # of non-MEMEQUIV_TYPE_UNUSED items in `meqs_list' */
	size_t               meqs_used; /* # of non-MEMEQUIV_TYPE_UNUSED/MEMEQUIV_TYPE_DUMMY items in `meqs_list' */
	struct Dee_memequiv *meqs_list; /* [1..meqs_mask+1][owned] Memory equivalence hash-vector. */
	size_t               meqs_regs[HOST_REGISTER_COUNT]; /* # of MEMEQUIV_TYPE_HREG/MEMEQUIV_TYPE_HREGIND entries for reach register. */
};

INTDEF struct Dee_memequiv const Dee_memequivs_dummy_list[1];

/* Hash-iteration control. */
#define Dee_memequivs_hashst(self, hash)  ((hash) & (self)->meqs_mask)
#define Dee_memequivs_hashnx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define Dee_memequivs_hashit(self, i)     ((self)->meqs_list + ((i) & (self)->meqs_mask))

#define Dee_memequivs_init(self)                                                \
	(void)((self)->meqs_mask = 0,                                               \
	       (self)->meqs_size = 0,                                               \
	       (self)->meqs_used = 0,                                               \
	       (self)->meqs_list = (struct Dee_memequiv *)Dee_memequivs_dummy_list, \
	       bzero((self)->meqs_regs, sizeof((self)->meqs_regs)))
#define Dee_memequivs_fini(self)  \
	(void)((self)->meqs_list == Dee_memequivs_dummy_list || (Dee_Free((self)->meqs_list), 0))

#define _Dee_memequivs_incrinuse(self, regno) (void)++(self)->meqs_regs[regno]
#define _Dee_memequivs_decrinuse(self, regno) (void)(ASSERT((self)->meqs_regs[regno] > 0), --(self)->meqs_regs[regno])
#if defined(NDEBUG) || 0
#define _Dee_memequivs_verifyrinuse(self) (void)0
#else /* NDEBUG */
#define HAVE__Dee_memequivs_verifyrinuse_d
#define _Dee_memequivs_verifyrinuse(self) _Dee_memequivs_verifyrinuse_d(self)
INTDEF NONNULL((1)) void DCALL _Dee_memequivs_verifyrinuse_d(struct Dee_memequivs const *__restrict self);
#endif /* !NDEBUG */


/* Inplace-replace `self->meqs_list' with a copy of itself. */
INTDEF WUNUSED NONNULL((1)) int DCALL
_Dee_memequivs_inplace_copy(struct Dee_memequivs *__restrict self);

/* Constrain equivalences in `self' by deleting all that aren't also present in `other'
 * @return: true:  At least 1 equivalence had to be deleted.
 * @return: false: Everything is good! */
INTDEF NONNULL((1, 2)) bool DCALL
Dee_memequivs_constrainwith(struct Dee_memequivs *__restrict self,
                            struct Dee_memequivs const *__restrict other);

/* Remember that "to" now contains the same value as "from".
 * In the even that "to" was already part of another equivalence
 * class, it will first be removed from that class the same way
 * a call to `Dee_memequivs_undefined(self, to)' would.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_memequivs_movevalue(struct Dee_memequivs *__restrict self,
                        struct Dee_memloc const *__restrict from,
                        struct Dee_memloc const *__restrict to);

/* Remember that a value change happened: "loc = loc + delta" */
INTDEF NONNULL((1, 2)) void DCALL
Dee_memequivs_deltavalue(struct Dee_memequivs *__restrict self,
                         struct Dee_memadr const *__restrict loc,
                         ptrdiff_t delta);

/* Check if "self" might use register locations. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_memequivs_hasregs(struct Dee_memequivs const *__restrict self);

/* Remember that "loc" contains an undefined value (remove
 * from its equivalence class, should that class still exist). */
INTDEF NONNULL((1, 2)) void DCALL
Dee_memequivs_undefined(struct Dee_memequivs *__restrict self,
                        struct Dee_memadr const *__restrict loc);

/* Mark all HREG and HREGIND locations as undefined. */
INTDEF NONNULL((1)) void DCALL
Dee_memequivs_undefined_allregs(struct Dee_memequivs *__restrict self);

/* Mark all HSTACKIND locations with CFA offsets `>= min_cfa_offset' as undefined. */
INTDEF NONNULL((1)) void DCALL
Dee_memequivs_undefined_hstackind_after(struct Dee_memequivs *__restrict self,
                                        Dee_cfa_t min_cfa_offset);

/* Mark all HSTACKIND locations where [Dee_memequiv_getcfastart()...Dee_memequiv_getcfaend())
 * overlaps with [start_cfa_offset, end_cfa_offset) as undefined. */
INTDEF NONNULL((1)) void DCALL
Dee_memequivs_undefined_hstackind_inrange(struct Dee_memequivs *__restrict self,
                                          Dee_cfa_t start_cfa_offset,
                                          Dee_cfa_t end_cfa_offset);

/* Return a pointer to the equivalence location of "loc" (ignoring
 * value offsets), or `NULL' if there aren't any additional locations
 * that are known to be equivalent to "loc". */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) struct Dee_memequiv *DCALL
Dee_memequivs_getclassof(struct Dee_memequivs const *__restrict self,
                         struct Dee_memadr const *__restrict loc);

/* Check if "a" and "b" contain identical values according to known equivalences. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2, 3)) bool DCALL
Dee_memequivs_equals(struct Dee_memequivs const *__restrict self,
                     struct Dee_memloc const *a,
                     struct Dee_memloc const *b);


/* Equivalence API helpers */
LOCAL ATTR_PURE WUNUSED NONNULL((1)) struct Dee_memequiv *DCALL
Dee_memequivs_getclassof_reg(struct Dee_memequivs const *__restrict self,
                             Dee_host_register_t regno) {
	struct Dee_memadr adr;
	Dee_memadr_init_hreg(&adr, regno);
	return Dee_memequivs_getclassof(self, &adr);
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) struct Dee_memequiv *DCALL
Dee_memequivs_getclassof_regind(struct Dee_memequivs const *__restrict self,
                                Dee_host_register_t regno, ptrdiff_t ind_delta) {
	struct Dee_memadr adr;
	Dee_memadr_init_hregind(&adr, regno, ind_delta);
	return Dee_memequivs_getclassof(self, &adr);
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) struct Dee_memequiv *DCALL
Dee_memequivs_getclassof_hstackind(struct Dee_memequivs const *__restrict self,
                                   Dee_cfa_t cfa_offset) {
	struct Dee_memadr adr;
	Dee_memadr_init_hstackind(&adr, cfa_offset);
	return Dee_memequivs_getclassof(self, &adr);
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) struct Dee_memequiv *DCALL
Dee_memequivs_getclassof_const(struct Dee_memequivs const *__restrict self,
                               void const *value) {
	struct Dee_memadr adr;
	Dee_memadr_init_const(&adr, value);
	return Dee_memequivs_getclassof(self, &adr);
}




LOCAL NONNULL((1)) void DCALL
Dee_memequivs_undefined_reg(struct Dee_memequivs *__restrict self,
                            Dee_host_register_t regno) {
	struct Dee_memadr adr;
	Dee_memadr_init_hreg(&adr, regno);
	Dee_memequivs_undefined(self, &adr);
}

LOCAL NONNULL((1)) void DCALL
Dee_memequivs_undefined_regind(struct Dee_memequivs *__restrict self,
                               Dee_host_register_t regno, ptrdiff_t ind_delta) {
	struct Dee_memadr adr;
	Dee_memadr_init_hregind(&adr, regno, ind_delta);
	Dee_memequivs_undefined(self, &adr);
}

LOCAL NONNULL((1)) void DCALL
Dee_memequivs_undefined_hstackind(struct Dee_memequivs *__restrict self,
                                  Dee_cfa_t cfa_offset) {
	struct Dee_memadr adr;
	Dee_memadr_init_hstackind(&adr, cfa_offset);
	Dee_memequivs_undefined(self, &adr);
}

#define Dee_memequivs_undefined_constind(self, p_value) (void)0

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_reg2hstackind(struct Dee_memequivs *__restrict self,
                                      Dee_host_register_t src_regno,
                                      Dee_cfa_t cfa_offset) {
	struct Dee_memloc from, to;
	Dee_memloc_init_hreg(&from, src_regno, 0);
	Dee_memloc_init_hstackind(&to, cfa_offset, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_movevalue_regind2hstackind(struct Dee_memequivs *__restrict self,
                                                   Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                                   Dee_cfa_t cfa_offset) {
	struct Dee_memloc from, to;
	Dee_memloc_init_hregind(&from, src_regno, src_delta, 0);
	Dee_memloc_init_hstackind(&to, cfa_offset, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}
#define Dee_memequivs_movevalue_hstack2reg(self, cfa_offset, dst_regno) \
	(Dee_memequivs_undefined_reg(self, dst_regno), 0)

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_hstackind2reg(struct Dee_memequivs *__restrict self,
                                      Dee_cfa_t cfa_offset,
                                      Dee_host_register_t dst_regno) {
	struct Dee_memloc from, to;
	Dee_memloc_init_hstackind(&from, cfa_offset, 0);
	Dee_memloc_init_hreg(&to, dst_regno, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_const2reg(struct Dee_memequivs *__restrict self,
                                  void const *value, Dee_host_register_t dst_regno) {
	struct Dee_memloc from, to;
	Dee_memloc_init_const(&from, value);
	Dee_memloc_init_hreg(&to, dst_regno, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_const2regind(struct Dee_memequivs *__restrict self,
                                     void const *value, Dee_host_register_t dst_regno,
                                     ptrdiff_t dst_delta) {
	struct Dee_memloc from, to;
	Dee_memloc_init_const(&from, value);
	Dee_memloc_init_hregind(&to, dst_regno, dst_delta, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_const2hstackind(struct Dee_memequivs *__restrict self,
                                        void const *value, Dee_cfa_t cfa_offset) {
	struct Dee_memloc from, to;
	Dee_memloc_init_const(&from, value);
	Dee_memloc_init_hstackind(&to, cfa_offset, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

#define Dee_memequivs_movevalue_hstack2hstackind(self, src_cfa_offset, dst_cfa_offset) \
	(Dee_memequivs_undefined_hstackind(self, dst_cfa_offset), 0)

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_hstackind2hstackind(struct Dee_memequivs *__restrict self,
                                            Dee_cfa_t src_cfa_offset,
                                            Dee_cfa_t dst_cfa_offset) {
	struct Dee_memloc from, to;
	Dee_memloc_init_hstackind(&from, src_cfa_offset, 0);
	Dee_memloc_init_hstackind(&to, dst_cfa_offset, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

#define Dee_memequivs_movevalue_const2constind(self, value, p_value) \
	(Dee_memequivs_undefined_constind(self, p_value), 0)

#define Dee_memequivs_movevalue_reg2reg(self, src_regno, dst_regno) \
	Dee_memequivs_movevalue_regx2reg(self, src_regno, 0, dst_regno)

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_regx2reg(struct Dee_memequivs *__restrict self,
                                 Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                 Dee_host_register_t dst_regno) {
	struct Dee_memloc from, to;
	Dee_memloc_init_hreg(&from, src_regno, src_delta);
	Dee_memloc_init_hreg(&to, dst_regno, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_regind2reg(struct Dee_memequivs *__restrict self,
                                   Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                   Dee_host_register_t dst_regno) {
	struct Dee_memloc from, to;
	Dee_memloc_init_hregind(&from, src_regno, src_delta, 0);
	Dee_memloc_init_hreg(&to, dst_regno, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_memequivs_movevalue_reg2regind(struct Dee_memequivs *__restrict self,
                                   Dee_host_register_t src_regno,
                                   Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	struct Dee_memloc from, to;
	Dee_memloc_init_hreg(&from, src_regno, 0);
	Dee_memloc_init_hregind(&to, dst_regno, dst_delta, 0);
	return Dee_memequivs_movevalue(self, &from, &to);
}
#define Dee_memequivs_movevalue_constind2reg(self, p_value, dst_regno) \
	(Dee_memequivs_undefined_reg(self, dst_regno), 0)
#define Dee_memequivs_movevalue_reg2constind(self, src_regno, p_value) \
	(Dee_memequivs_undefined_constind(self, p_value), 0)




/* Possible flags for `struct Dee_memobj::mo_flags' */
#define MEMOBJ_F_NORMAL       0x00
#define MEMOBJ_F_ISREF        0x01 /* DREF: A reference is being held to the objec in `mo_loc' */
#define MEMOBJ_F_ONEREF       0x02 /* [valid_if(MEMOBJ_F_ISREF)] The reference being held by `mo_loc' has not yet escaped (on decref, can use decref_dokill instead) */
#define MEMOBJ_F_LINEAR       0x04 /* When mo_loc is `MEMADR_TYPE_HSTACKIND': location is part of a linear vector. It must not be moved to a different cfa offset (only allowed if memobj is part of vstack) */
#define MEMOBJ_F_MAYBEUNBOUND 0x80 /* Location may be NULL, meaning it may not be bound (only allowed if memobj is used by MEMVAL_VMORPH_DIRECT of a local variable memval)
                                    * NOTE: This flag is combined with `MEMADR_TYPE_CONST,NULL' to represent uninitialized locals. */

struct Dee_memobj {
	/* High-level object value (encapsulates Dee_memloc and adds extra deemon-object related meta-data) */
	struct Dee_memloc mo_loc;    /* Object location */
	DeeTypeObject    *mo_typeof; /* [0..1] If non-null, the guarantied correct object type of this location (assumed value
	                              * for the "ob_type" of this memory location, and used for inlining operator calls).
	                              * DON'T SET THIS TO SOMETHING STUPID LIKE "DeeObject_Type" -- NO BASE CLASSES ALLOWED!
	                              * NOTE: Only used by `Dee_function_generator_v*' function! */
	uint8_t           mo_flags;  /* Object flags (set of `MEMOBJ_F_*') */
};

#define _Dee_memobj_init_impl(self, _initbase, mo_typeof_, mo_flags_) \
	(void)(_initbase, (self)->mo_typeof = (mo_typeof_), (self)->mo_flags = (mo_flags_))
#define Dee_memobj_init_memadr(self, adr, ml_off_, mo_typeof_, mo_flags_) \
	_Dee_memobj_init_impl(self, Dee_memloc_init_memadr(&(self)->mo_loc, adr, ml_off_), mo_typeof_, mo_flags_)
#define Dee_memobj_init_memloc(self, loc, mo_typeof_, mo_flags_) \
	_Dee_memobj_init_impl(self, (self)->mo_loc = *(loc), mo_typeof_, mo_flags_)
#define Dee_memobj_init_hreg(self, ma_regno_, ml_off_, mo_typeof_, mo_flags_) \
	_Dee_memobj_init_impl(self, Dee_memloc_init_hreg(&(self)->mo_loc, ma_regno_, ml_off_), mo_typeof_, mo_flags_)
#define Dee_memobj_init_hregind(self, ma_regno_, v_indoff_, ml_off_, mo_typeof_, mo_flags_) \
	_Dee_memobj_init_impl(self, Dee_memloc_init_hregind(&(self)->mo_loc, ma_regno_, v_indoff_, ml_off_), mo_typeof_, mo_flags_)
#define Dee_memobj_init_hstack(self, v_cfa_, mo_typeof_, mo_flags_) \
	_Dee_memobj_init_impl(self, Dee_memloc_init_hstack(&(self)->mo_loc, v_cfa_), mo_typeof_, mo_flags_)
#define Dee_memobj_init_hstackind(self, v_cfa_, ml_off_, mo_typeof_, mo_flags_) \
	_Dee_memobj_init_impl(self, Dee_memloc_init_hstackind(&(self)->mo_loc, v_cfa_, ml_off_), mo_typeof_, mo_flags_)
#define Dee_memobj_init_const_ex(self, v_const_, mo_typeof_, mo_flags_) \
	_Dee_memobj_init_impl(self, Dee_memloc_init_const(&(self)->mo_loc, v_const_), mo_typeof_, mo_flags_)
#define Dee_memobj_init_const(self, v_const_, mo_typeof_) \
	Dee_memobj_init_const_ex(self, v_const_, mo_typeof_, MEMOBJ_F_NORMAL)
#define Dee_memobj_init_undefined(self) \
	_Dee_memobj_init_impl(self, Dee_memloc_init_undefined(&(self)->mo_loc), NULL, MEMOBJ_F_NORMAL)
#define Dee_memobj_init_local_unbound(self) \
	Dee_memobj_init_const_ex(self, NULL, NULL, MEMOBJ_F_MAYBEUNBOUND)
#define Dee_memobj_init_constaddr(self, value) Dee_memobj_init_const(self, value, NULL)
#define Dee_memobj_init_constobj(self, value)  Dee_memobj_init_const(self, value, Dee_TYPE(value))

#define Dee_memobj_hashofadr(a)  Dee_memloc_hashofadr(&(a)->mo_loc)
#define Dee_memobj_sameadr(a, b) Dee_memloc_sameadr(&(a)->mo_loc, &(b)->mo_loc)
#define Dee_memobj_sameloc(a, b) Dee_memloc_sameloc(&(a)->mo_loc, &(b)->mo_loc)

#define Dee_memobj_getloc(self)       (&(self)->mo_loc)
#define Dee_memobj_gettyp(self)       Dee_memloc_gettyp(Dee_memobj_getloc(self))
#define Dee_memobj_typeof(self)       (self)->mo_typeof
#define Dee_memobj_settypeof(self, t) (void)((self)->mo_typeof = (t))
#define Dee_memobj_isref(self)        ((self)->mo_flags & MEMOBJ_F_ISREF)
#define Dee_memobj_setref(self)       (void)((self)->mo_flags |= MEMOBJ_F_ISREF)
#define Dee_memobj_clearref(self)     (void)((self)->mo_flags &= ~(MEMOBJ_F_ISREF | MEMOBJ_F_ONEREF))
#define Dee_memobj_isoneref(self)     ((self)->mo_flags & MEMOBJ_F_ONEREF)
#define Dee_memobj_setoneref(self)    (void)((self)->mo_flags |= (MEMOBJ_F_ISREF | MEMOBJ_F_ONEREF))
#define Dee_memobj_clearoneref(self)  (void)((self)->mo_flags &= ~MEMOBJ_F_ONEREF)

#define Dee_memobj_isconst(self)         Dee_memloc_isconst(Dee_memobj_getloc(self))
#define Dee_memobj_isconstobj(self, obj) Dee_memloc_isconstobj(Dee_memobj_getloc(self), obj)
#define Dee_memobj_isnull(self)          Dee_memobj_isconstobj(self, NULL)
#define Dee_memobj_iszero(self)          Dee_memobj_isconstobj(self, 0)
#define Dee_memobj_isnone(self)          (Dee_memobj_typeof(self) == &DeeNone_Type) /*Dee_memobj_isconstobj(self, Dee_None)*/
#define Dee_memobj_isundefined(self)     Dee_memloc_isundefined(Dee_memobj_getloc(self))

#define Dee_memobj_const_getaddr(self)            Dee_memloc_const_getaddr(Dee_memobj_getloc(self))
#define Dee_memobj_const_setaddr_keeptyp(self, v) Dee_memloc_const_setaddr(Dee_memobj_getloc(self), v)
#define Dee_memobj_const_setaddr_ex(self, v, t)   (void)(Dee_memobj_const_setaddr_keeptyp(self, v), (self)->mo_typeof = (t))
#define Dee_memobj_const_setaddr(self, v)         (void)(Dee_memobj_const_setaddr_keeptyp(self, v), (self)->mo_typeof = NULL)
#define Dee_memobj_const_getobj(self)             Dee_memloc_const_getobj(Dee_memobj_getloc(self))
#define Dee_memobj_const_setobj_keeptyp(self, v)  Dee_memloc_const_setobj(Dee_memobj_getloc(self), v)
#define Dee_memobj_const_setobj_ex(self, v, t)    (void)(Dee_memobj_const_setobj_keeptyp(self, v), (self)->mo_typeof = (t))
#define Dee_memobj_const_setobj(self, v)          (void)(Dee_memobj_const_setobj_keeptyp(self, v), (self)->mo_typeof = Dee_TYPE(Dee_memobj_const_getobj(self)))

#define Dee_memobj_getoff(self)                   Dee_memloc_getoff(Dee_memobj_getloc(self))
#define Dee_memobj_setoff(self, val)              Dee_memloc_setoff(Dee_memobj_getloc(self), val)
#define Dee_memobj_gettyp(self)                   Dee_memloc_gettyp(Dee_memobj_getloc(self))
#define Dee_memobj_getadr(self)                   Dee_memloc_getadr(Dee_memobj_getloc(self))
#define Dee_memobj_hasreg(self)                   Dee_memloc_hasreg(Dee_memobj_getloc(self))
#define Dee_memobj_getreg(self)                   Dee_memloc_getreg(Dee_memobj_getloc(self))
#define Dee_memobj_hreg_getreg(self)              Dee_memloc_hreg_getreg(Dee_memobj_getloc(self))
#define Dee_memobj_hreg_getvaloff(self)           Dee_memloc_hreg_getvaloff(Dee_memobj_getloc(self))
#define Dee_memobj_hreg_setvaloff(self, val)      Dee_memloc_hreg_setvaloff(Dee_memobj_getloc(self), val)
#define Dee_memobj_hregind_getreg(self)           Dee_memloc_hregind_getreg(Dee_memobj_getloc(self))
#define Dee_memobj_hregind_getindoff(self)        Dee_memloc_hregind_getindoff(Dee_memobj_getloc(self))
#define Dee_memobj_hregind_getvaloff(self)        Dee_memloc_hregind_getvaloff(Dee_memobj_getloc(self))
#define Dee_memobj_hregind_setvaloff(self, val)   Dee_memloc_hregind_setvaloff(Dee_memobj_getloc(self), val)
#define Dee_memobj_hstack_getcfa(self)            Dee_memloc_hstack_getcfa(Dee_memobj_getloc(self))
#define Dee_memobj_hstackind_getcfa(self)         Dee_memloc_hstackind_getcfa(Dee_memobj_getloc(self))
#define Dee_memobj_hstackind_getvaloff(self)      Dee_memloc_hstackind_getvaloff(Dee_memobj_getloc(self))
#define Dee_memobj_hstackind_setvaloff(self, val) Dee_memloc_hstackind_setvaloff(Dee_memobj_getloc(self), val)

#define Dee_memobj_local_alwaysbound(self) (!((self)->mo_flags & MEMOBJ_F_MAYBEUNBOUND))
#define Dee_memobj_local_neverbound(self)  (Dee_memobj_isconstobj(self, NULL) /*&& !Dee_memobj_local_alwaysbound(self)*/)
#define Dee_memobj_local_setbound(self)    (void)((self)->mo_flags &= ~MEMOBJ_F_MAYBEUNBOUND)
#define Dee_memobj_local_setunbound(self)  (void)((self)->mo_flags |= MEMOBJ_F_MAYBEUNBOUND, Dee_memloc_init_const(Dee_memobj_getloc(self), NULL), Dee_memobj_settypeof(self, NULL))

#define Dee_memobj_getcfastart(self) Dee_memloc_getcfastart(&(self)->mo_loc)
#define Dee_memobj_getcfaend(self)   Dee_memloc_getcfaend(&(self)->mo_loc)




/* Value-proxy indirection (applied on-top of `ml_adr.ma_typ'). e.g.: `value = DeeBool_For(value)'
 * NOTE: All of the `Dee_function_generator_g*' function are allowed to assume `MEMVAL_VMORPH_ISDIRECT'.
 *       Only the `Dee_function_generator_v*' functions actually check for `mv_vmorph'! */
#define MEMVAL_VMORPH_DIRECT     0 /* >> value = mv_obj.mvo_0; // Location contains a direct value (usually an object pointer, but can also be anything else) */
#define MEMVAL_VMORPH_DIRECT_01  1 /* >> value = mv_obj.mvo_0; __assume(value == 0 || value == 1); */
#define MEMVAL_VMORPH_ISDIRECT(vmorph)      ((vmorph) <= MEMVAL_VMORPH_DIRECT_01)
#define MEMVAL_VMORPH_TESTZ(direct_vmorph)  ((direct_vmorph) | MEMVAL_VMORPH_BOOL_Z)  /* Assume that `MEMVAL_VMORPH_ISDIRECT(direct_vmorph)' */
#define MEMVAL_VMORPH_TESTNZ(direct_vmorph) ((direct_vmorph) | MEMVAL_VMORPH_BOOL_NZ) /* Assume that `MEMVAL_VMORPH_ISDIRECT(direct_vmorph)' */
#define MEMVAL_VMORPH_ISBOOL(vmorph)        ((vmorph) >= MEMVAL_VMORPH_BOOL_Z && (vmorph) <= MEMVAL_VMORPH_BOOL_GZ)
#define MEMVAL_VMORPH_BOOL_Z     2 /* >> value = DeeBool_For(mv_obj.mvo_0 == 0 ? 1 : 0); */
#define MEMVAL_VMORPH_BOOL_Z_01  3 /* >> value = DeeBool_For({1,0}[mv_obj.mvo_0]); */
#define MEMVAL_VMORPH_BOOL_NZ    4 /* >> value = DeeBool_For(mv_obj.mvo_0 != 0 ? 1 : 0); */
#define MEMVAL_VMORPH_BOOL_NZ_01 5 /* >> value = DeeBool_For(mv_obj.mvo_0); */
#define MEMVAL_VMORPH_BOOL_LZ    6 /* >> value = DeeBool_For((intptr_t)mv_obj.mvo_0 < 0); */
#define MEMVAL_VMORPH_BOOL_GZ    7 /* >> value = DeeBool_For((intptr_t)mv_obj.mvo_0 > 0); */
#define MEMVAL_VMORPH_INT        8 /* >> value = DeeInt_NewIntptr(mv_obj.mvo_0); */
#define MEMVAL_VMORPH_UINT       9 /* >> value = DeeInt_NewUIntptr(mv_obj.mvo_0); */
#define MEMVAL_VMORPH_NULLABLE  10 /* >> value = mv_obj.mvo_0 ?: HANDLE_EXCEPT(); */
#define MEMVAL_VMORPH_HASOBJ0(x) ((x) < 16)
#define MEMVAL_VMORPH_HASOBJN(x) ((x) >= 16)
/* TODO: Multi-object morphs: */
/* TODO: MEMVAL_VMORPH_LIST */
/* TODO: MEMVAL_VMORPH_TUPLE */
/* TODO: MEMVAL_VMORPH_HASHSET */
/* TODO: MEMVAL_VMORPH_ROSET */
/* TODO: MEMVAL_VMORPH_DICT */
/* TODO: MEMVAL_VMORPH_RODICT */
/* TODO: MEMVAL_VMORPH_SUPER */


struct Dee_memobjs {
	/* NOTE: This structure is *always* immutable!
	 *
	 * If you want to inplace-assign a new Dee_memobjs-descriptor to some memval, you
	 * must *ALWAYS* first create that new Dee_memobjs-descriptor, then enumerate the
	 * current mem-state, and search for identical memvals that also appear within the
	 * same mos_copies as the old Dee_memobjs's ring of copies, and then assign your
	 * new Dee_memobjs to *all* of those memvals. */
	Dee_refcnt_t                               mos_refcnt; /* Reference counter (when >1, this struct is used by multiple `Dee_memval'-s) */
	RINGQ_ENTRY(Dee_memobjs)                   mos_copies; /* Ring of copies that have been made of this set of mem objects
	                                                        * All Dee_memobjs items refer to the same virtual object at runtime.
	                                                        * Aside from `mos_objv[*].mo_flags', all items of this ring are
	                                                        * completely identical, and if changes are made, those changes must
	                                                        * be made on all elements at the same time! */
	size_t                                     mos_objc;   /* # of objects */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_memobj, mos_objv);  /* [mos_objc] Vector of objects. */
	/* TODO: Bitset of objects that should be in-place expanded:
	 * >> return [10, foo..., 20];
	 * ASM:
	 * >> push   @10
	 * >> push   pack List, #1   // MEMVAL_VMORPH_LIST: {CONST(10)}    expand:{}
	 * >> push   @foo
	 * >> concat top, pop        // MEMVAL_VMORPH_LIST: {CONST(10), LOC(foo)}    expand:{foo}
	 * >> push   @20
	 * >> extend top, #1         // MEMVAL_VMORPH_LIST: {CONST(10), LOC(foo), CONST(20)}    expand:{foo}
	 *
	 * When the type of "foo" is known (e.g.: is a tuple), then we
	 * can perfectly allocate the final list right from the start!
	 */
};

INTDEF NONNULL((1)) void DCALL Dee_memobjs_destroy(struct Dee_memobjs *__restrict self);
#define Dee_memobjs_incref(self)        (void)(++(self)->mos_refcnt)
#define Dee_memobjs_decref(self)        (void)(--(self)->mos_refcnt || (Dee_memobjs_destroy(self), 0))
#define Dee_memobjs_isshared(self)      ((self)->mos_refcnt > 1)
#define Dee_memobjs_decref_nokill(self) (void)(ASSERT((self)->mos_refcnt >= 2), --(self)->mos_refcnt)

LOCAL ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memobjs_copies_contains(struct Dee_memobjs const *ring_of_this,
                            struct Dee_memobjs const *contains_this) {
	struct Dee_memobjs const *iter = ring_of_this;
	do {
		if (iter == contains_this)
			return true;
	} while ((iter = RINGQ_NEXT(iter, mos_copies)) != ring_of_this);
	return false;
}



/* Possible values for `struct Dee_memval::mv_flags' */
#define MEMVAL_F_NORMAL 0x00 /* Normal flags */
#define MEMVAL_F_NOREF  0x01 /* Ignore `MEMOBJ_F_ISREF' of objecst (may only be set when `Dee_memval_hasobjn()') */

#undef DEE_DEFINE_MEMVAL_FOR_IDE
#ifdef __INTELLISENSE__
#define DEE_DEFINE_MEMVAL_FOR_IDE
#endif /* __INTELLISENSE__ */

struct Dee_memval {
	/* High-level value (encapsulates 1..n Dee_memloc that may be morphed into a deemon value) */
	union {
#ifndef DEE_DEFINE_MEMVAL_FOR_IDE
		struct {
			struct Dee_memloc  _mv0_loc;    /* Alias for "mv_obj.mvo_0.mo_loc" */
			DeeTypeObject     *_mv0_typeof; /* Alias for "mv_obj.mvo_0.mo_typeof" */
			uint8_t            _mv0_flags;  /* Alias for "mv_obj.mvo_0.mo_flags" */
			uint8_t            _mv_vmorph;  /* Location value morph type (one of `MEMVAL_VMORPH_*') */
			uint8_t            _mv_flags;   /* Extra mem value flags (set of `MEMVAL_F_*'). */
			uint8_t           __mv_pad[sizeof(void *) - 3]; /* Padding... */
		} _mvo_val;
#endif /* !DEE_DEFINE_MEMVAL_FOR_IDE */
		struct Dee_memobj       mvo_0;     /* [valid_if(Dee_memval_hasobj0(this))] Base location */
#ifdef DEE_DEFINE_MEMVAL_FOR_IDE
		struct Dee_memobj      *mvo_n;     /* [valid_if(Dee_memval_hasobjn(this))][1..1] Pointer to a `struct Dee_memobjs::mos_objv' */
#else /* DEE_DEFINE_MEMVAL_FOR_IDE */
		DREF struct Dee_memobj *mvo_n;     /* [valid_if(Dee_memval_hasobjn(this))][1..1] Pointer to a `struct Dee_memobjs::mos_objv' */
#endif /* !DEE_DEFINE_MEMVAL_FOR_IDE */
	} mv_obj; /* Object */
#ifdef DEE_DEFINE_MEMVAL_FOR_IDE
	uint8_t             mv_vmorph;  /* Location value morph type (one of `MEMVAL_VMORPH_*') */
	uint8_t             mv_flags;   /* Extra mem value flags (set of `MEMVAL_F_*'). */
	uint8_t            _mv_pad[sizeof(void *) - 3]; /* Padding... */
#else /* DEE_DEFINE_MEMVAL_FOR_IDE */
#define mv_vmorph mv_obj._mvo_val._mv_vmorph
#define mv_flags  mv_obj._mvo_val._mv_flags
#endif /* !DEE_DEFINE_MEMVAL_FOR_IDE */
};


#ifdef NDEBUG
#define _Dee_memval_fini_DBG_memset(self) (void)0
#else /* NDEBUG */
#define _Dee_memval_fini_DBG_memset(self) (void)memset(self, 0xcc, sizeof(struct Dee_memval))
#endif /* !NDEBUG */
#define Dee_memval_fini_direct(self) (void)0 /* Always a no-op, but may be used for easier code readability */
#define Dee_memval_fini(self)                                                        \
	(!Dee_memval_hasobjn(self) || (Dee_memobjs_decref(Dee_memval_getobjn(self)), 0), \
	 _Dee_memval_fini_DBG_memset(self))

/* Create a copy of a Dee_memval. Never fails, but may incref a pointed-to
 * struct, meaning you have to call `Dee_memval_fini()', unless you know
 * that the `vmorph' of `other' doesn't require such a thing. */
#define Dee_memval_initcopy(self, other) \
	(void)(*(self) = *(other),           \
	       !Dee_memval_hasobjn(self) || (Dee_memobjs_incref(Dee_memval_getobjn(self)), 0))

/* Move the value from "src" into "dst".
 * Semantically the same as:
 * >> Dee_memval_initcopy(dst, src);
 * >> Dee_memval_fini(src); */
#define Dee_memval_initmove(dst, src) \
	(void)(*(dst) = *(src), _Dee_memval_fini_DBG_memset(src))
#undef Dee_memval_initmove_IS_MEMCPY
#define Dee_memval_initmove_IS_MEMCPY /* If defined, you can use memcpy/memmove to emulate `Dee_memval_initmove()' */

/* Basic Dee_memval initializers.
 * NOTE: NONE OF THESE REQUIRE USE OF "Dee_memval_fini"! */
#define _Dee_memval_init_impl(self, _initbase) \
	(void)(_initbase, (self)->mv_vmorph = MEMVAL_VMORPH_DIRECT, (self)->mv_flags = MEMVAL_F_NORMAL)
#define Dee_memval_init_memadr(self, adr, ml_off_, mv_valtyp_, mv_flags_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_memadr(&(self)->mv_obj.mvo_0, adr, ml_off_, mv_valtyp_, mv_flags_))
#define Dee_memval_init_memloc(self, loc, mv_valtyp_, mv_flags_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_memloc(&(self)->mv_obj.mvo_0, loc, mv_valtyp_, mv_flags_))
#define Dee_memval_init_memobj(self, obj) \
	_Dee_memval_init_impl(self, (self)->mv_obj.mvo_0 = *(obj))
#define Dee_memval_init_hreg(self, ma_regno_, ml_off_, mv_valtyp_, mv_flags_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_hreg(&(self)->mv_obj.mvo_0, ma_regno_, ml_off_, mv_valtyp_, mv_flags_))
#define Dee_memval_init_hregind(self, ma_regno_, v_indoff_, ml_off_, mv_valtyp_, mv_flags_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_hregind(&(self)->mv_obj.mvo_0, ma_regno_, v_indoff_, ml_off_, mv_valtyp_, mv_flags_))
#define Dee_memval_init_hstack(self, v_cfa_, mv_valtyp_, mv_flags_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_hstack(&(self)->mv_obj.mvo_0, v_cfa_, mv_valtyp_, mv_flags_))
#define Dee_memval_init_hstackind(self, v_cfa_, ml_off_, mv_valtyp_, mv_flags_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_hstackind(&(self)->mv_obj.mvo_0, v_cfa_, ml_off_, mv_valtyp_, mv_flags_))
#define Dee_memval_init_const(self, v_const_, mv_valtyp_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_const(&(self)->mv_obj.mvo_0, v_const_, mv_valtyp_))
#define Dee_memval_init_const_ex(self, v_const_, mv_valtyp_, mv_flags_) \
	_Dee_memval_init_impl(self, Dee_memobj_init_const_ex(&(self)->mv_obj.mvo_0, v_const_, mv_valtyp_, mv_flags_))
#define Dee_memval_init_undefined(self) \
	_Dee_memval_init_impl(self, Dee_memobj_init_undefined(&(self)->mv_obj.mvo_0))
#define Dee_memval_init_local_unbound(self) \
	_Dee_memval_init_impl(self, Dee_memobj_init_local_unbound(&(self)->mv_obj.mvo_0))
#define Dee_memval_init_constaddr(self, value) Dee_memval_init_const(self, value, NULL)
#define Dee_memval_init_constobj(self, value)  Dee_memval_init_const(self, value, Dee_TYPE(value))


#define Dee_memval_hasobj0(self)              MEMVAL_VMORPH_HASOBJ0((self)->mv_vmorph)
#define Dee_memval_getobj0(self)              (&(self)->mv_obj.mvo_0)
#define Dee_memval_obj0_getobj(self)          Dee_memval_getobj0(self)
#define Dee_memval_obj0_getloc(self)          Dee_memobj_getloc(Dee_memval_obj0_getobj(self))
#define Dee_memval_obj0_gettyp(self)          Dee_memobj_gettyp(Dee_memval_obj0_getobj(self))
#define Dee_memval_obj0_isconst(self)         Dee_memobj_isconst(Dee_memval_obj0_getobj(self))
#define Dee_memval_obj0_isundefined(self)     Dee_memobj_isundefined(Dee_memval_obj0_getobj(self))
#define Dee_memval_obj0_isconstobj(self, obj) Dee_memobj_isconstobj(Dee_memval_obj0_getobj(self), obj)
#define Dee_memval_obj0_isnull(self)          Dee_memval_obj0_isconstobj(self, NULL)

#define Dee_memval_obj0_const_getaddr(self)            Dee_memobj_const_getaddr(Dee_memval_obj0_getobj(self))
#define Dee_memval_obj0_const_setaddr_keeptyp(self, v) Dee_memobj_const_setaddr_keeptyp(Dee_memval_obj0_getobj(self), v)
#define Dee_memval_obj0_const_setaddr_ex(self, v, t)   Dee_memobj_const_setaddr_ex(Dee_memval_obj0_getobj(self), v, t)
#define Dee_memval_obj0_const_setaddr(self, v)         Dee_memobj_const_setaddr(Dee_memval_obj0_getobj(self), v)
#define Dee_memval_obj0_const_getobj(self)             Dee_memobj_const_getobj(Dee_memval_obj0_getobj(self))
#define Dee_memval_obj0_const_setobj_keeptyp(self, v)  Dee_memobj_const_setobj_keeptyp(Dee_memval_obj0_getobj(self), v)
#define Dee_memval_obj0_const_setobj_ex(self, v, t)    Dee_memobj_const_setobj_ex(Dee_memval_obj0_getobj(self), v, t)
#define Dee_memval_obj0_const_setobj(self, v)          Dee_memobj_const_setobj(Dee_memval_obj0_getobj(self), v)

#define Dee_memval_direct_isref(self)             Dee_memobj_isref(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_setref(self)            Dee_memobj_setref(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_clearref(self)          Dee_memobj_clearref(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_isoneref(self)          Dee_memobj_isoneref(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_setoneref(self)         Dee_memobj_setoneref(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_clearoneref(self)       Dee_memobj_clearoneref(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_local_alwaysbound(self) Dee_memobj_local_alwaysbound(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_local_neverbound(self)  Dee_memobj_local_neverbound(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_local_setbound(self)    Dee_memobj_local_setbound(Dee_memval_obj0_getobj(self))
#define Dee_memval_direct_local_setunbound(self)  Dee_memobj_local_setunbound(Dee_memval_obj0_getobj(self))

#define Dee_memval_nullable_makedirect(self)      (void)((self)->mv_vmorph = MEMVAL_VMORPH_DIRECT)
#define Dee_memval_nullable_getobj(self)          Dee_memval_obj0_getobj(self)
#define Dee_memval_nullable_getloc(self)          Dee_memval_obj0_getloc(self)

#define Dee_memval_direct_initcopy(self, other)          (void)(*(self) = *(other))
#define Dee_memval_direct_getobj(self)                   Dee_memval_obj0_getobj(self)
#define Dee_memval_direct_getloc(self)                   Dee_memval_obj0_getloc(self)
#define Dee_memval_direct_typeof(self)                   Dee_memobj_typeof(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_settypeof(self, t)             Dee_memobj_settypeof(Dee_memval_direct_getobj(self), t)
#define Dee_memval_direct_isconst(self)                  Dee_memobj_isconst(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_isconstobj(self, obj)          Dee_memobj_isconstobj(Dee_memval_direct_getobj(self), obj)
#define Dee_memval_direct_isnull(self)                   Dee_memobj_isnull(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_iszero(self)                   Dee_memobj_iszero(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_isnone(self)                   Dee_memobj_isnone(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_isundefined(self)              Dee_memobj_isundefined(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_getoff(self)                   Dee_memobj_getoff(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_setoff(self, val)              Dee_memobj_setoff(Dee_memval_direct_getobj(self), val)
#define Dee_memval_direct_gettyp(self)                   Dee_memobj_gettyp(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_getadr(self)                   Dee_memobj_getadr(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hasreg(self)                   Dee_memobj_hasreg(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_getreg(self)                   Dee_memobj_getreg(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hreg_getreg(self)              Dee_memobj_hreg_getreg(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hreg_getvaloff(self)           Dee_memobj_hreg_getvaloff(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hreg_setvaloff(self, val)      Dee_memobj_hreg_setvaloff(Dee_memval_direct_getobj(self), val)
#define Dee_memval_direct_hregind_getreg(self)           Dee_memobj_hregind_getreg(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hregind_getindoff(self)        Dee_memobj_hregind_getindoff(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hregind_getvaloff(self)        Dee_memobj_hregind_getvaloff(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hregind_setvaloff(self, val)   Dee_memobj_hregind_setvaloff(Dee_memval_direct_getobj(self), val)
#define Dee_memval_direct_hstack_getcfa(self)            Dee_memobj_hstack_getcfa(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hstackind_getcfa(self)         Dee_memobj_hstackind_getcfa(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hstackind_getvaloff(self)      Dee_memobj_hstackind_getvaloff(Dee_memval_direct_getobj(self))
#define Dee_memval_direct_hstackind_setvaloff(self, val) Dee_memobj_hstackind_setvaloff(Dee_memval_direct_getobj(self), val)

#define Dee_memval_const_typeof(self)             Dee_memval_direct_typeof(self) /* or Dee_TYPE(Dee_memval_const_getobj(self)) */
#define Dee_memval_const_getloc(self)             Dee_memval_direct_getloc(self)
#define Dee_memval_const_getaddr(self)            Dee_memobj_const_getaddr(Dee_memval_direct_getobj(self))
#define Dee_memval_const_setaddr(self, v)         Dee_memobj_const_setaddr(Dee_memval_direct_getobj(self), v)
#define Dee_memval_const_setaddr_ex(self, v, t)   Dee_memobj_const_setaddr_ex(Dee_memval_direct_getobj(self), v, t)
#define Dee_memval_const_setaddr_keeptyp(self, v) Dee_memobj_const_setaddr_keeptyp(Dee_memval_direct_getobj(self), v)
#define Dee_memval_const_getobj(self)             Dee_memobj_const_getobj(Dee_memval_direct_getobj(self))
#define Dee_memval_const_setobj(self, v)          Dee_memobj_const_setobj(Dee_memval_direct_getobj(self), v)
#define Dee_memval_const_setobj_ex(self, v, t)    Dee_memobj_const_setobj_ex(Dee_memval_direct_getobj(self), v, t)
#define Dee_memval_const_setobj_keeptyp(self, v)  Dee_memobj_const_setobj_keeptyp(Dee_memval_direct_getobj(self), v)

#define Dee_memval_isnullable(self)      ((self)->mv_vmorph == MEMVAL_VMORPH_NULLABLE)
#define Dee_memval_isdirect(self)        MEMVAL_VMORPH_ISDIRECT((self)->mv_vmorph)
#define Dee_memval_isconst(self)         (Dee_memval_direct_isconst(self) && likely(Dee_memval_isdirect(self)))
#define Dee_memval_isconstobj(self, obj) (Dee_memval_isdirect(self) && Dee_memval_direct_isconstobj(self, obj))
#define Dee_memval_isnull(self)          (Dee_memval_isdirect(self) && Dee_memval_direct_isnull(self))
#define Dee_memval_iszero(self)          (Dee_memval_isdirect(self) && Dee_memval_direct_iszero(self))
#define Dee_memval_isnone(self)          (Dee_memval_isdirect(self) && Dee_memval_direct_isnone(self))
#define Dee_memval_isundefined(self)     (Dee_memval_isdirect(self) && Dee_memval_direct_isundefined(self))

#define Dee_memval_direct_hashofadr(a)  Dee_memloc_hashofadr(Dee_memval_direct_getloc(a))
#define Dee_memval_direct_sameadr(a, b) Dee_memloc_sameadr(Dee_memval_direct_getloc(a), Dee_memval_direct_getloc(b))
#define Dee_memval_direct_sameloc(a, b) Dee_memloc_sameloc(Dee_memval_direct_getloc(a), Dee_memval_direct_getloc(b))

/* Try to figure out the guarantied runtime object type of `vdirect()' */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
Dee_memval_typeof(struct Dee_memval const *self);


#define Dee_memval_hasobjn(self)     MEMVAL_VMORPH_HASOBJN((self)->mv_vmorph)
#define Dee_memval_getobjn(self)     COMPILER_CONTAINER_OF((self)->mv_obj.mvo_n, struct Dee_memobjs, mos_objv)
#define Dee_memval_objn_getcnt(self) (Dee_memval_getobjn(self)->mos_objc)
#define Dee_memval_objn_getvec(self) (self)->mv_obj.mvo_n /* Dee_memval_getobjn(self)->mos_objv */
#define Dee_memval_objn_shared(self) Dee_memobjs_isshared(Dee_memval_getobjn(self))

/* Enumerate all memory objects used by "self" */
#define _Dee_memval_foreach_obj(_mvfe, obj, self)                                                       \
	do {                                                                                                \
		size_t _mvfe##_obji, _mvfe##_objc = 1;                                                          \
		struct Dee_memobj *_mvfe##_objv = (struct Dee_memobj *)&(self)->mv_obj.mvo_0;                   \
		if unlikely(Dee_memval_hasobjn(self)) {                                                         \
			_mvfe##_objv = *(struct Dee_memobj **)_mvfe##_objv;                                         \
			_mvfe##_objc = COMPILER_CONTAINER_OF(_mvfe##_objv, struct Dee_memobjs, mos_objv)->mos_objc; \
		}                                                                                               \
		for (_mvfe##_obji = 0; _mvfe##_obji < _mvfe##_objc; ++_mvfe##_obji)                             \
			if (((obj) = &_mvfe##_objv[_mvfe##_obji], 0))                                               \
				;                                                                                       \
			else
#define Dee_memval_foreach_obj(obj, self) _Dee_memval_foreach_obj(_mvfe, obj, self)
#define Dee_memval_foreach_obj_end \
	} __WHILE0

#define Dee_memval_getobjc(self) (Dee_memval_hasobjn(self) ? Dee_memval_objn_getcnt(self) : 1)
#define Dee_memval_getobjv(self) (Dee_memval_hasobjn(self) ? Dee_memval_objn_getvec(self) : Dee_memval_getobj0(self))

INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_memval_do_objn_unshare(struct Dee_memval *__restrict self);
#define Dee_memval_objv_shared(self) \
	(Dee_memval_hasobjn(self) && Dee_memval_objn_shared(self))
#define Dee_memval_objv_unshare(self) \
	(Dee_memval_objv_shared(self) ? Dee_memval_do_objn_unshare(self) : 0)

/* Remember that "self" isn't holding *any* references */
#define Dee_memval_clearref(self)                                             \
	(Dee_memval_hasobj0(self) ? Dee_memobj_clearref(Dee_memval_getobj0(self)) \
	                          : (void)((self)->mv_flags |= MEMVAL_F_NOREF))

/* Clear the buffered "MEMVAL_F_NOREF" flag, by unsharing memobjs,
 * and clearing the MEMOBJ_F_ISREF flags of all references objects. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_memval_do_clear_MEMVAL_F_NOREF(struct Dee_memval *__restrict self);

/* Check if "a" and "b" represent the same effective object, in that
 * a (theoretical) change to the runtime object of "a" must then also
 * be reflected in "b". */
LOCAL ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memval_sameval(struct Dee_memval const *a,
                   struct Dee_memval const *b) {
	size_t i;
	struct Dee_memobjs const *a_objs, *b_objs;
	if (a->mv_vmorph != b->mv_vmorph)
		return false;
	if (Dee_memval_hasobj0(a))
		return Dee_memloc_sameloc(Dee_memval_obj0_getloc(a), Dee_memval_obj0_getloc(b));
	a_objs = Dee_memval_getobjn(a);
	b_objs = Dee_memval_getobjn(b);
	if (a_objs->mos_objc != b_objs->mos_objc)
		return false;
	if (!Dee_memobjs_copies_contains(a_objs, b_objs))
		return false;
	for (i = 0; i < a_objs->mos_objc; ++i) {
		if (!Dee_memobj_sameloc(&a_objs->mos_objv[i],
		                        &b_objs->mos_objv[i]))
			return false;
	}
	return true;
}

/* Check if "a" and "b" represent the same effective.
 * NOTE: This doesn't necessarily mean that "a === b", but *does* mean
 *       that vdirect() of "a" and "b" produces identical logical code,
 *       and that the result represents identical logical objects, and
 *       that "a === b" is *allowed* to be true at runtime.
 *
 * This is the same as `Dee_memval_sameval()', but can be used if the
 * caller is OK with 2 yet-to-be-created instances of identical objects
 * (such as 2 Tuples with identical elements) end up being merged. */
LOCAL ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memval_sameval_mayalias(struct Dee_memval const *a,
                            struct Dee_memval const *b) {
	size_t i;
	struct Dee_memobjs const *a_objs, *b_objs;
	if (a->mv_vmorph != b->mv_vmorph)
		return false;
	if (Dee_memval_hasobj0(a))
		return Dee_memloc_sameloc(Dee_memval_obj0_getloc(a), Dee_memval_obj0_getloc(b));
	a_objs = Dee_memval_getobjn(a);
	b_objs = Dee_memval_getobjn(b);
	if (a_objs->mos_objc != b_objs->mos_objc)
		return false;
	for (i = 0; i < a_objs->mos_objc; ++i) {
		if (!Dee_memobj_sameloc(&a_objs->mos_objv[i],
		                        &b_objs->mos_objv[i]))
			return false;
	}
	return true;
}





/* Possible values for `Dee_memstate::ms_rusage' */
#define DEE_HOST_REGUSAGE_GENERIC 0x00 /* Register usage is defined by `ms_stackv' and `ms_localv'. */
#define DEE_HOST_REGUSAGE_THREAD  0x01 /* Register contains: DeeThread_Self() */
typedef uint8_t Dee_host_regusage_t;


/* Extra local variable IDs always present in `ms_localv'
 * These indices appear after "normal" locals. */
#define MEMSTATE_XLOCAL_A_FUNC     0 /* Caller-argument: `DeeFunctionObject *func' (only for `HOSTFUNC_CC_F_FUNC') */
#define MEMSTATE_XLOCAL_A_THIS     1 /* Caller-argument: `DeeObject *this'         (only for `HOSTFUNC_CC_F_THIS') */
#define MEMSTATE_XLOCAL_A_ARGC     2 /* Caller-argument: `size_t argc'             (only for `!HOSTFUNC_CC_F_TUPLE') */
#define MEMSTATE_XLOCAL_A_ARGS     3 /* Caller-argument: `DeeTupleObject *args'    (only for `HOSTFUNC_CC_F_TUPLE') */
#define MEMSTATE_XLOCAL_A_ARGV     3 /* Caller-argument: `DeeObject **argv'        (only for `!HOSTFUNC_CC_F_TUPLE') */
#define MEMSTATE_XLOCAL_A_KW       4 /* Caller-argument: `DeeObject *kw'           (only for `HOSTFUNC_CC_F_KW') */
#define MEMSTATE_XLOCAL_VARARGS    5 /* Varargs (s.a. `struct Dee_code_frame::cf_vargs') */
#define MEMSTATE_XLOCAL_VARKWDS    6 /* Varkwds (s.a. `struct Dee_code_frame_kwds::fk_varkwds') */
#define MEMSTATE_XLOCAL_STDOUT     7 /* Temporary slot for a cached version of `deemon.File.stdout' (to speed up `ASM_PRINT' & friends) */
#define MEMSTATE_XLOCAL_POPITER    8 /* Temporary slot used by `ASM_FOREACH' to decref the iterator when ITER_DONE is returned. DON'T USE FOR ANYTHING ELSE! */
#define MEMSTATE_XLOCAL_MINCOUNT   9 /* Min number of extra locals */
#define MEMSTATE_XLOCAL_DEFARG_MIN MEMSTATE_XLOCAL_MINCOUNT
#define MEMSTATE_XLOCAL_DEFARG(opt_aid) (MEMSTATE_XLOCAL_DEFARG_MIN + (opt_aid)) /* Start of cached optional arguments. */

/* Mem-state flags. */
#define MEMSTATE_F_NORMAL      0x0000 /* Normal flags */
#define MEMSTATE_F_GOTEXCEPT   0x0001 /* It's known that `DeeThread_Self()->t_except != NULL' */
#if 0 /* TODO */
#define MEMSTATE_F_GOTNULLABLE 0x0002 /* There exactly 1 Dee_memval with `mv_vmorph == MEMVAL_VMORPH_NULLABLE' */
#endif

struct Dee_memstate {
	Dee_refcnt_t                               ms_refcnt;          /* Reference counter for the mem-state (state becomes read-only when >1) */
	Dee_cfa_t                                  ms_host_cfa_offset; /* Delta between SP to CFA (Canonical Frame Address) */
	Dee_lid_t                                  ms_localc;          /* [== :co_localc+MEMSTATE_XLOCAL_MINCOUNT+(:co_argc_max-:co_argc_min)]
	                                                                * Number of local variables + extra slots. NOTE: Never 0! */
	Dee_vstackaddr_t                           ms_stackc;          /* Number of (currently) used deemon stack slots in use. */
	Dee_vstackaddr_t                           ms_stacka;          /* Allocated number of deemon stack slots in use. */
	uintptr_t                                  ms_flags;           /* Special state flags (set of `MEMSTATE_F_*' and'd when constraining states; initialized to `0') */
	size_t                                     ms_uargc_min;       /* Lower bound for the `argc' passed to the generated function (can be used to skip argc-checks) */
	size_t                                     ms_rinuse[HOST_REGISTER_COUNT]; /* Number of times each register is referenced by `ms_stackv' and `ms_localv' */
	Dee_host_regusage_t                        ms_rusage[HOST_REGISTER_COUNT]; /* Meaning of registers (set to `DEE_HOST_REGUSAGE_GENERIC' if clobbered) */
	/* Keep track of memory locations that contain the same values.
	 * primarily: when loading a HSTACKIND into a HREG, the stack location
	 *            is now available for allocation, but until that happens,
	 *            it still contains the original value. So when needing to
	 *            flush the register before the stack location is used for
	 *            something else, it can be re-used without any extra code
	 *            needing to be generated.
	 * This could also make register flushing more resilient by having the
	 * generator-arch code check for value aliases to use a register alias
	 * so-long as that alias wasn't clobbered by some later flush.
	 *
	 * For this purpose, use something similar to <partition.h>, as
	 * implemented on KOS. Only that rather than being fixed-length,
	 * this one will be variable-length, with classes:
	 * - Having an optional known constant value (e.g. when we no something equals Dee_None)
	 * - Containing a variable number of HREG, HREGIND or HSTACKIND elements. */
	struct Dee_memequivs                       ms_memequiv;        /* Known equivalent memory locations. */
	struct Dee_memval                         *ms_stackv;          /* [0..ms_stackc][owned] Deemon stack memory locations. */
#ifdef __INTELLISENSE__
	struct Dee_memval                          ms_localv[1024];    /* [0..ms_localc] Deemon locals memory locations. */
#else /* __INTELLISENSE__ */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_memval, ms_localv);         /* [0..ms_localc] Deemon locals memory locations. */
#endif /* !__INTELLISENSE__ */
};

/* Helper macro to enumerate all `struct Dee_memval *mval' of a `struct Dee_memstate *self':
 * >> struct Dee_memval *mval;
 * >> Dee_memstate_foreach(mval, state) {
 * >>     ...
 * >> }
 * >> Dee_memstate_foreach_end;
 */
#define Dee_memstate_foreach_isstack(mval, self) (_dmfe_v != (self)->ms_localv)
#define Dee_memstate_foreach_islocal(mval, self) (_dmfe_v == (self)->ms_localv)
#define Dee_memstate_foreach(mval, self)                                \
	do {                                                                \
		struct Dee_memval *_dmfe_v = (self)->ms_stackv;                 \
		Dee_lid_t _dmfe_i = 0, _dmfe_c = (self)->ms_stackc;             \
		do                                                              \
			if (_dmfe_i >= _dmfe_c                                      \
			    ? (_dmfe_v == (self)->ms_stackv                         \
			       ? (_dmfe_i = 0,                                      \
			          _dmfe_v = (struct Dee_memval *)(self)->ms_localv, \
			          _dmfe_c = (self)->ms_localc,                      \
			          0)                                                \
			       : 1)                                                 \
			    : 0) {                                                  \
				break;                                                  \
			} else if (((mval) = &_dmfe_v[_dmfe_i++], 0))               \
				;                                                       \
			else
#define Dee_memstate_foreach_end \
		__WHILE1;                \
	}	__WHILE0

#define Dee_memstate_sizeof(localc) \
	(offsetof(struct Dee_memstate, ms_localv) + (localc) * sizeof(struct Dee_memval))
#define Dee_memstate_alloc(localc) \
	((struct Dee_memstate *)Dee_Malloc(Dee_memstate_sizeof(localc)))
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
	(_Dee_memstate_verifyrinuse(*(p_self)),                      \
	 unlikely(Dee_memstate_isshared(*(p_self))                   \
	          ? Dee_memstate_inplace_copy_because_shared(p_self) \
	          : 0))
#define Dee_memstate_dounshare(p_self)         \
	(_Dee_memstate_verifyrinuse(*(p_self)),    \
	 ASSERT(Dee_memstate_isshared(*(p_self))), \
	 Dee_memstate_inplace_copy_because_shared(p_self))

/* Ensure that at least `min_alloc' stack slots are allocated. */
INTDEF NONNULL((1)) int DCALL
Dee_memstate_reqvstack(struct Dee_memstate *__restrict self,
                       Dee_vstackaddr_t min_alloc);

/* Account for register usage. */
#define Dee_memstate_incrinuse(self, regno) (void)++(self)->ms_rinuse[regno]
#define Dee_memstate_decrinuse(self, regno) (void)(ASSERT((self)->ms_rinuse[regno] > 0), --(self)->ms_rinuse[regno])
#if defined(NDEBUG) || 0
#define _Dee_memstate_verifyrinuse(self) (void)0
#else /* NDEBUG */
#define HAVE__Dee_memstate_verifyrinuse_d
#define _Dee_memstate_verifyrinuse(self) _Dee_memstate_verifyrinuse_d(self)
INTDEF NONNULL((1)) void DCALL _Dee_memstate_verifyrinuse_d(struct Dee_memstate const *__restrict self);
#endif /* !NDEBUG */

#define Dee_memstate_incrinuse_for_memadr(self, adr) \
	(MEMADR_TYPE_HASREG((adr)->ma_typ) ? Dee_memstate_incrinuse(self, (adr)->ma_reg) : (void)0)
#define Dee_memstate_decrinuse_for_memadr(self, adr) \
	(MEMADR_TYPE_HASREG((adr)->ma_typ) ? Dee_memstate_decrinuse(self, (adr)->ma_reg) : (void)0)
#define Dee_memstate_incrinuse_for_memloc(self, loc) Dee_memstate_incrinuse_for_memadr(self, Dee_memloc_getadr(loc))
#define Dee_memstate_decrinuse_for_memloc(self, loc) Dee_memstate_decrinuse_for_memadr(self, Dee_memloc_getadr(loc))
#define Dee_memstate_incrinuse_for_memobj(self, loc) Dee_memstate_incrinuse_for_memloc(self, Dee_memobj_getloc(loc))
#define Dee_memstate_decrinuse_for_memobj(self, loc) Dee_memstate_decrinuse_for_memloc(self, Dee_memobj_getloc(loc))
#define Dee_memstate_incrinuse_for_memval(self, val)         \
	do {                                                     \
		struct Dee_memobj const *_iiobj;                     \
		Dee_memval_foreach_obj(_iiobj, val) {                \
			Dee_memstate_incrinuse_for_memobj(self, _iiobj); \
		}                                                    \
		Dee_memval_foreach_obj_end;                          \
	}	__WHILE0
#define Dee_memstate_decrinuse_for_memval(self, val)         \
	do {                                                     \
		struct Dee_memobj const *_diobj;                     \
		Dee_memval_foreach_obj(_diobj, val) {                \
			Dee_memstate_decrinuse_for_memobj(self, _diobj); \
		}                                                    \
		Dee_memval_foreach_obj_end;                          \
	}	__WHILE0
#define Dee_memstate_incrinuse_for_direct_memval(self, val) Dee_memstate_incrinuse_for_memobj(self, Dee_memval_direct_getobj(val))
#define Dee_memstate_decrinuse_for_direct_memval(self, val) Dee_memstate_decrinuse_for_memobj(self, Dee_memval_direct_getobj(val))

/* Memory equivalence helpers. */
#define Dee_memstate_remember_getclassof(self, loc)                                               Dee_memequivs_getclassof(&(self)->ms_memequiv, loc)
#define Dee_memstate_remember_getclassof_reg(self, regno)                                         Dee_memequivs_getclassof_reg(&(self)->ms_memequiv, regno)
#define Dee_memstate_remember_getclassof_regind(self, regno, ind_delta)                           Dee_memequivs_getclassof_regind(&(self)->ms_memequiv, regno, ind_delta)
#define Dee_memstate_remember_getclassof_hstackind(self, cfa_offset)                              Dee_memequivs_getclassof_hstackind(&(self)->ms_memequiv, cfa_offset)
#define Dee_memstate_remember_movevalue(self, from, to)                                           Dee_memequivs_movevalue(&(self)->ms_memequiv, from, to)
#define Dee_memstate_remember_deltavalue(self, loc, delta)                                        Dee_memequivs_deltavalue(&(self)->ms_memequiv, loc, delta)
#define Dee_memstate_remember_undefined(self, loc)                                                Dee_memequivs_undefined(&(self)->ms_memequiv, loc)
#define Dee_memstate_remember_undefined_allregs(self)                                             Dee_memequivs_undefined_allregs(&(self)->ms_memequiv)
#define Dee_memstate_remember_undefined_reg(self, regno)                                          Dee_memequivs_undefined_reg(&(self)->ms_memequiv, regno)
#define Dee_memstate_remember_undefined_regind(self, regno, ind_delta)                            Dee_memequivs_undefined_regind(&(self)->ms_memequiv, regno, ind_delta)
#define Dee_memstate_remember_undefined_hstackind(self, cfa_offset)                               Dee_memequivs_undefined_hstackind(&(self)->ms_memequiv, cfa_offset)
#define Dee_memstate_remember_undefined_constind(self, p_value)                                   Dee_memequivs_undefined_constind(&(self)->ms_memequiv, p_value)
#define Dee_memstate_remember_movevalue_reg2hstackind(self, src_regno, cfa_offset)                Dee_memequivs_movevalue_reg2hstackind(&(self)->ms_memequiv, src_regno, cfa_offset)
#define Dee_memstate_remember_movevalue_regind2hstackind(self, src_regno, src_delta, cfa_offset)  Dee_memequivs_movevalue_movevalue_regind2hstackind(&(self)->ms_memequiv, src_regno, src_delta, cfa_offset)
#define Dee_memstate_remember_movevalue_hstack2reg(self, cfa_offset, dst_regno)                   Dee_memequivs_movevalue_hstack2reg(&(self)->ms_memequiv, cfa_offset, dst_regno)
#define Dee_memstate_remember_movevalue_hstackind2reg(self, cfa_offset, dst_regno)                Dee_memequivs_movevalue_hstackind2reg(&(self)->ms_memequiv, cfa_offset, dst_regno)
#define Dee_memstate_remember_movevalue_const2reg(self, value, dst_regno)                         Dee_memequivs_movevalue_const2reg(&(self)->ms_memequiv, value, dst_regno)
#define Dee_memstate_remember_movevalue_const2regind(self, value, dst_regno, dst_delta)           Dee_memequivs_movevalue_const2regind(&(self)->ms_memequiv, value, dst_regno, dst_delta)
#define Dee_memstate_remember_movevalue_const2hstackind(self, value, cfa_offset)                  Dee_memequivs_movevalue_const2hstackind(&(self)->ms_memequiv, value, cfa_offset)
#define Dee_memstate_remember_movevalue_hstack2hstackind(self, src_cfa_offset, dst_cfa_offset)    Dee_memequivs_movevalue_hstack2hstackind(&(self)->ms_memequiv, src_cfa_offset, dst_cfa_offset)
#define Dee_memstate_remember_movevalue_hstackind2hstackind(self, src_cfa_offset, dst_cfa_offset) Dee_memequivs_movevalue_hstackind2hstackind(&(self)->ms_memequiv, src_cfa_offset, dst_cfa_offset)
#define Dee_memstate_remember_movevalue_const2constind(self, value, p_value)                      Dee_memequivs_movevalue_const2constind(&(self)->ms_memequiv, value, p_value)
#define Dee_memstate_remember_movevalue_reg2reg(self, src_regno, dst_regno)                       Dee_memequivs_movevalue_reg2reg(&(self)->ms_memequiv, src_regno, dst_regno)
#define Dee_memstate_remember_movevalue_regx2reg(self, src_regno, src_delta, dst_regno)           Dee_memequivs_movevalue_regx2reg(&(self)->ms_memequiv, src_regno, src_delta, dst_regno)
#define Dee_memstate_remember_movevalue_regind2reg(self, src_regno, src_delta, dst_regno)         Dee_memequivs_movevalue_regind2reg(&(self)->ms_memequiv, src_regno, src_delta, dst_regno)
#define Dee_memstate_remember_movevalue_reg2regind(self, src_regno, dst_regno, dst_delta)         Dee_memequivs_movevalue_reg2regind(&(self)->ms_memequiv, src_regno, dst_regno, dst_delta)
#define Dee_memstate_remember_movevalue_constind2reg(self, p_value, dst_regno)                    Dee_memequivs_movevalue_constind2reg(&(self)->ms_memequiv, p_value, dst_regno)
#define Dee_memstate_remember_movevalue_reg2constind(self, src_regno, p_value)                    Dee_memequivs_movevalue_reg2constind(&(self)->ms_memequiv, src_regno, p_value)

/* Mark all register equivalences undefined for registers that are not in active use:
 * >> FOREACH REGNO DO
 * >>     IF self->ms_rinuse[REGNO] == 0 THEN
 * >>         Dee_memequivs_undefined_reg(&self->ms_memequiv, REGNO);
 * >>     FI
 * >> DONE */
INTDEF NONNULL((1)) void DCALL
Dee_memstate_remember_undefined_unusedregs(struct Dee_memstate *__restrict self);




#ifdef HOSTASM_STACK_GROWS_DOWN
#define HA_cfa_offset_PLUS_sp_offset(cfa_offset, sp_offset) ((cfa_offset) - (sp_offset))
#define Dee_memstate_hstack_cfa2sp(self, cfa_offset)        (ptrdiff_t)((self)->ms_host_cfa_offset - (Dee_cfa_t)(cfa_offset))
#define Dee_memstate_hstack_sp2cfa(self, sp_offset)         (Dee_cfa_t)((ptrdiff_t)(self)->ms_host_cfa_offset - (ptrdiff_t)(sp_offset))
#define Dee_memstate_hstack_canpop(self, cfa_offset)        ((self)->ms_host_cfa_offset == (Dee_cfa_t)(cfa_offset))
#define Dee_memstate_hstack_canpush(self, cfa_offset)       ((self)->ms_host_cfa_offset + HOST_SIZEOF_POINTER == (Dee_cfa_t)(cfa_offset))
#define Dee_memstate_hstack_mustpush(self, cfa_offset)      ((self)->ms_host_cfa_offset + HOST_SIZEOF_POINTER <= (Dee_cfa_t)(cfa_offset))
#define Dee_memstate_hstack_mustpush_skip(self, cfa_offset) (ptrdiff_t)((Dee_cfa_t)(cfa_offset) - (self)->ms_host_cfa_offset)
#else /* HOSTASM_STACK_GROWS_DOWN */
#define HA_cfa_offset_PLUS_sp_offset(cfa_offset, sp_offset) ((cfa_offset) + (sp_offset))
#define Dee_memstate_hstack_cfa2sp(self, cfa_offset)        (ptrdiff_t)((Dee_cfa_t)(cfa_offset) - (self)->ms_host_cfa_offset)
#define Dee_memstate_hstack_sp2cfa(self, sp_offset)         (Dee_cfa_t)((ptrdiff_t)(self)->ms_host_cfa_offset + (ptrdiff_t)(sp_offset))
#define Dee_memstate_hstack_canpop(self, cfa_offset)        ((self)->ms_host_cfa_offset - HOST_SIZEOF_POINTER == (Dee_cfa_t)(cfa_offset))
#define Dee_memstate_hstack_canpush(self, cfa_offset)       ((self)->ms_host_cfa_offset == (Dee_cfa_t)(cfa_offset))
#define Dee_memstate_hstack_mustpush(self, cfa_offset)      ((self)->ms_host_cfa_offset <= (Dee_cfa_t)(cfa_offset))
#define Dee_memstate_hstack_mustpush_skip(self, cfa_offset) (ptrdiff_t)((Dee_cfa_t)(cfa_offset) - ((self)->ms_host_cfa_offset + HOST_SIZEOF_POINTER))
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
 * @return: (Dee_cfa_t)-1: There is no free section of at least `n_bytes' bytes.
 *                         In this case, allocate using `Dee_memstate_hstack_alloca()' */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_cfa_t DCALL
Dee_memstate_hstack_find(struct Dee_memstate const *__restrict self,
                         struct Dee_memstate const *hstack_reserved,
                         size_t n_bytes);

/* Check if a pointer-sized blob at `cfa_offset' is being used by something. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_memstate_hstack_isused(struct Dee_memstate const *__restrict self,
                           Dee_cfa_t cfa_offset);

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

/* Check if a reference is being held by `mval' or some other location that may be aliasing it. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memstate_hasref(struct Dee_memstate const *__restrict self,
                    struct Dee_memval const *mval);

/* Check if `mval' has an alias. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memstate_hasalias(struct Dee_memstate const *__restrict self,
                      struct Dee_memval const *mval);
#define Dee_memstate_isoneref_noalias(self, mval) \
	(Dee_memval_isdirect(mval) && Dee_memval_direct_isref(mval) && !Dee_memstate_hasalias(self, mval))

/* Functions to manipulate the virtual deemon object stack. */
#define Dee_memstate_vtop(self) (&(self)->ms_stackv[(self)->ms_stackc - 1])
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vswap(struct Dee_memstate *__restrict self); /* ASM_SWAP */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vlrot(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vrrot(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vmirror(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush_memadr(struct Dee_memstate *__restrict self, struct Dee_memadr const *adr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush_memloc(struct Dee_memstate *__restrict self, struct Dee_memloc const *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush_memobj(struct Dee_memstate *__restrict self, struct Dee_memobj const *obj);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush_memval(struct Dee_memstate *__restrict self, struct Dee_memval const *val);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_undefined(struct Dee_memstate *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_addr(struct Dee_memstate *__restrict self, void const *addr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush_const(struct Dee_memstate *__restrict self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_hreg(struct Dee_memstate *__restrict self, Dee_host_register_t regno, ptrdiff_t val_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_hregind(struct Dee_memstate *__restrict self, Dee_host_register_t regno, ptrdiff_t ind_delta, ptrdiff_t val_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_hstack(struct Dee_memstate *__restrict self, Dee_cfa_t cfa_offset);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_hstackind(struct Dee_memstate *__restrict self, Dee_cfa_t cfa_offset, ptrdiff_t val_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vdup_n(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n);
#define Dee_memstate_vdup(self) Dee_memstate_vdup_n(self, 1)


#undef HAVE_DEE_HOST_SYMBOL_ALLOC_INFO
#if !defined(NDEBUG) && 1
#define HAVE_DEE_HOST_SYMBOL_ALLOC_INFO
#endif /* !NDEBUG */



/* Host text symbol */
struct Dee_jump_descriptor;
struct Dee_host_section;
struct Dee_host_symbol {
	struct Dee_host_symbol *_hs_next; /* [0..1][owned] Next symbol (used internally for chaining) */
#ifndef NO_HOSTASM_DEBUG_PRINT
	char const              *hs_name; /* [0..1] Symbol name (for debug logs) */
#endif /* !NO_HOSTASM_DEBUG_PRINT */
#ifdef HAVE_DEE_HOST_SYMBOL_ALLOC_INFO
	char const              *hs_file;
	int                      hs_line;
#endif /* HAVE_DEE_HOST_SYMBOL_ALLOC_INFO */
#define DEE_HOST_SYMBOL_UNDEF 0 /* Not yet defined */
#define DEE_HOST_SYMBOL_ABS   1 /* Absolute value (e.g. for API functions) */
#define DEE_HOST_SYMBOL_JUMP  2 /* Pass-through via a `struct Dee_jump_descriptor' (or fast-forward to start of section) */
#define DEE_HOST_SYMBOL_SECT  3 /* Offset into a `struct Dee_host_section' */
	uintptr_t                hs_type; /* Symbol type (one of `DEE_HOST_SYMBOL_*') */
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

#define Dee_host_symbol_initcommon_named(self, name) Dee_host_symbol_setname(self, name)
#define Dee_host_symbol_initcommon(self)             Dee_host_symbol_initcommon_named(self, NULL)
#ifdef NO_HOSTASM_DEBUG_PRINT
#define Dee_host_symbol_setname(self, name) (void)0
#else /* NO_HOSTASM_DEBUG_PRINT */
#define Dee_host_symbol_setname(self, name) (void)((self)->hs_name = (name))
#endif /* !NO_HOSTASM_DEBUG_PRINT */

#define Dee_host_symbol_isdefined(self) ((self)->hs_type == DEE_HOST_SYMBOL_UNDEF)
#define Dee_host_symbol_setabs(self, addr)                \
	(void)((self)->hs_type         = DEE_HOST_SYMBOL_ABS, \
	       (self)->hs_value.sv_abs = (addr))
#define Dee_host_symbol_setjump(self, desc)                 \
	(void)((self)->hs_type          = DEE_HOST_SYMBOL_JUMP, \
	       (self)->hs_value.sv_jump = (desc))
#define Dee_host_symbol_setsect(self, sect)                       \
	(HA_printf("%s:\n", (self)->hs_name ? (self)->hs_name : "?"), \
	 _Dee_host_symbol_setsect(self, sect))
#define _Dee_host_symbol_setsect(self, sect) \
	_Dee_host_symbol_setsect_ex(self, sect, Dee_host_section_size(sect))
#define Dee_host_symbol_setsect_ex(self, sect, offset)                                        \
	(HA_printf("%s = .%+Id\n", (self)->hs_name ? (self)->hs_name : "?", (ptrdiff_t)(offset)), \
	 _Dee_host_symbol_setsect_ex(self, sect, offset))
#define _Dee_host_symbol_setsect_ex(self, sect, offset)             \
	(void)((self)->hs_type                  = DEE_HOST_SYMBOL_SECT, \
	       (self)->hs_value.sv_sect.ss_sect = (sect),               \
	       (self)->hs_value.sv_sect.ss_off  = (offset))

/* Calculate and return the value of `self'
 * Only returns valid values after `hs_base' have been assigned. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
Dee_host_symbol_value(struct Dee_host_symbol const *__restrict self);



/* Host relocation types. */
#define DEE_HOST_RELOC_NONE    0 /* No-op / deleted */
#ifdef HOSTASM_X86
#define DEE_HOST_RELOC_PCREL32 1 /* *(int32_t *)<ADDR> += (<VALUE> - <ADDR>) */
#define DEE_HOST_RELOC_PCREL8  2 /* *(int8_t *)<ADDR> += (<VALUE> - <ADDR>) */
#endif /* HOSTASM_X86 */

struct Dee_host_reloc {
	uint32_t                hr_offset; /* Offset from `bb_host_start' to where the relocation takes place. */
	uint16_t                hr_rtype;  /* Relocation type (one of `DEE_HOST_RELOC_*') */
#define DEE_HOST_RELOCVALUE_SYM  0     /* Relocate against a symbol */
#define DEE_HOST_RELOCVALUE_ABS  1     /* Relocate against an absolute value (for API calls) */
#ifndef HOSTASM_HAVE_SHRINKJUMPS /* shrinkjumps requires all section references to use symbols */
#define DEE_HOST_RELOCVALUE_SECT 2     /* Relocate against a section base address */
#endif /* !HOSTASM_HAVE_SHRINKJUMPS */
	uint16_t                hr_vtype;  /* Value type (one of `DEE_HOST_RELOCVALUE_*') */
	union {
		struct Dee_host_symbol  *rv_sym;  /* [1..1][valid_if(DEE_HOST_RELOCVALUE_SYM)] Relocation symbol. */
		void const              *rv_abs;  /* [?..?][valid_if(DEE_HOST_RELOCVALUE_ABS)] Relocation value. */
#ifdef DEE_HOST_RELOCVALUE_SECT
		struct Dee_host_section *rv_sect; /* [1..1][valid_if(DEE_HOST_RELOCVALUE_SECT)] Relocation section. */
#endif /* DEE_HOST_RELOCVALUE_SECT */
	} hr_value;
};

/* Fill in `self->hr_vtype' and `self->hr_value' based on `sym'
 * If `sym' has already been defined as absolute or pointing to
 * the start of a section, directly inline it. */
INTDEF NONNULL((1, 2)) void DCALL
Dee_host_reloc_setsym(struct Dee_host_reloc *__restrict self,
                      struct Dee_host_symbol *__restrict sym);

/* Calculate and return the value of `self'
 * Only returns valid values after `hs_base' have been assigned. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
Dee_host_reloc_value(struct Dee_host_reloc const *__restrict self);

struct Dee_host_section;
TAILQ_HEAD(Dee_host_section_tailq, Dee_host_section);
struct Dee_host_section {
	byte_t                       *hs_start;   /* [<= hs_end][owned] Start of host assembly */
	byte_t                       *hs_end;     /* [>= hs_start && <= hs_alend] End of host assembly */
#ifdef HOSTASM_HAVE_SHRINKJUMPS
	union {
		byte_t                   *hs_alend;   /* [>= hs_start] End of allocated host assembly */
		struct Dee_host_symbol   *hs_symbols; /* [0..1][owned][valid_if(TAILQ_ISBOUND(self, hs_link))]
		                                       * First symbol defined as part of this section */
		struct Dee_host_section  *hs_fallthru; /* [0..1] Used internally by `Dee_function_assembler_ordersections()' */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_hs_u1
#define hs_alend    _hs_u1.hs_alend
#define hs_symbols  _hs_u1.hs_symbols
#define hs_fallthru _hs_u1.hs_fallthru
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
#else /* HOSTASM_HAVE_SHRINKJUMPS */
	byte_t                       *hs_alend;   /* [>= hs_start] End of allocated host assembly */
#endif /* !HOSTASM_HAVE_SHRINKJUMPS */
	TAILQ_ENTRY(Dee_host_section) hs_link;    /* [0..1] Position of this section in the final output. */
	struct Dee_host_reloc        *hs_relv;    /* [0..hs_relc][owned] Vector of host relocations. */
	size_t                        hs_relc;    /* Number of host relocations. */
	union {
#undef hs_rela
#undef hs_base
#undef hs_badr
		size_t                    hs_rela;    /* [valid_if(NEVER_CALLED(Dee_function_assembler_output))] Allocated number of host relocations. */
		byte_t                   *hs_base;    /* [valid_if(EVER_CALLED(Dee_function_assembler_output))] Base address in latest output */
#ifdef HOSTASM_HAVE_SHRINKJUMPS
		uintptr_t                 hs_badr;    /* Used internally by `Dee_function_assembler_shrinkjumps()' */
#endif /* HOSTASM_HAVE_SHRINKJUMPS */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_hs_u2
#define hs_rela _hs_u2.hs_rela
#define hs_base _hs_u2.hs_base
#ifdef HOSTASM_HAVE_SHRINKJUMPS
#define hs_badr _hs_u2.hs_badr
#endif /* HOSTASM_HAVE_SHRINKJUMPS */
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#define Dee_host_section_init(self) bzero(self, sizeof(struct Dee_host_section))
INTDEF NONNULL((1)) void DCALL Dee_host_section_fini(struct Dee_host_section *__restrict self);
#define Dee_host_section_clear(self) \
	(void)((self)->hs_end = (self)->hs_start, (self)->hs_relc = 0)
#define Dee_host_section_size(self) \
	(size_t)((self)->hs_end - (self)->hs_start)

#define Dee_host_section_islinked(self) TAILQ_ISBOUND(self, hs_link)

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


struct Dee_basic_block_loclastread {
	Dee_instruction_t const *bbl_instr; /* [1..1] The instruction that reads the local for the final time
	                                     * (before noreturn or another instruction writing to the variable) */
	Dee_lid_t                bbl_lid;   /* ID of the variable in question */
};

struct Dee_basic_block {
	Dee_instruction_t const    *bb_deemon_start; /* [1..1][<= bb_deemon_end][const] Start of deemon assembly */
	Dee_instruction_t const    *bb_deemon_end;   /* [1..1][>= bb_deemon_start][const] End of deemon assembly */
	Dee_instruction_t const    *bb_deemon_end_r; /* [1..1][>= bb_deemon_start][const] Real end of deemon assembly */
	struct Dee_jump_descriptors bb_entries;      /* All of the possible ways this basic block can be entered (at `bb_deemon_start' / `bb_host_start'; this one owns descriptors). */
	struct Dee_jump_descriptors bb_exits;        /* All of the possible ways this basic block can be exited (via deemon code). */
	struct Dee_basic_block     *bb_next;         /* [0..1] Fallthru exit of this basic block (or NULL if there is none, which happens for the last block and blocks that end with NORETURN instructions) */
	struct Dee_basic_block     *bb_next_r;       /* [0..1] Real fallthru exit of this basic block */
#ifdef __INTELLISENSE__
	struct Dee_memstate        *bb_mem_start;    /* [0..1] Memory state at start of basic block (or NULL if not yet assembled) */
	struct Dee_memstate        *bb_mem_end;      /* [0..1] Memory state at end of basic block (or NULL if not yet assembled) */
#else /* __INTELLISENSE__ */
	DREF struct Dee_memstate   *bb_mem_start;    /* [0..1] Memory state at start of basic block (or NULL if not yet assembled) */
	DREF struct Dee_memstate   *bb_mem_end;      /* [0..1] Memory state at end of basic block (or NULL if not yet assembled) */
#endif /* !__INTELLISENSE__ */
	struct Dee_host_section     bb_htext;        /* Host assembly text */
	struct Dee_host_section     bb_hcold;        /* Extra assembly text that should be placed after all regular text. */

	/* Load variable usage data */
	struct Dee_basic_block_loclastread *bb_locreadv; /* [SORT(bbl_instr)][0..bb_locreadc][owned] Information about the final times a variable is read (+ trailing entry with `bbl_instr=-1') */
	size_t                              bb_locreadc; /* # of times a local variable is read for the final time in this block. */
	COMPILER_FLEXIBLE_ARRAY(byte_t,     bb_locuse);  /* [CEILDIV(bb_mem_start->ms_localc, 8)][valid_if(bb_deemon_start < bb_deemon_end)]
	                                                  * Bitset of locals read-from before being written to by this basic block (including
	                                                  * any branch taken prior to writing a local). NOTE: The [valid_if] is correct, but
	                                                  * technically, this bitset only exists for `Dee_function_assembler::fa_blockv', but
	                                                  * not `Dee_except_exitinfo::exi_block'. */
};

#define Dee_basic_block_alloc(n_locals)                                                 \
	((struct Dee_basic_block *)Dee_Malloc(offsetof(struct Dee_basic_block, bb_locuse) + \
	                                      ((n_locals) + __CHAR_BIT__ - 1) / __CHAR_BIT__))
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
	 Dee_host_section_init(&(self)->bb_hcold),       \
	 (self)->bb_locreadv = NULL,                     \
	 (self)->bb_locreadc = 0)

/* Destroy the given basic block `self'. */
INTDEF NONNULL((1)) void DCALL
Dee_basic_block_destroy(struct Dee_basic_block *__restrict self);

/* Split this basic block at `addr' (which must be `> bb_deemon_start'),
 * and move all jumps from `bb_exits' into the new basic block, as needed.
 * @return: * :   A new basic block that starts at `addr'
 * @return: NULL: Error */
INTDEF WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_basic_block_splitat(struct Dee_basic_block *__restrict self,
                        Dee_instruction_t const *addr,
                        Dee_lid_t n_locals);

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



/* Flags for `struct Dee_memref::mr_flags' */
#define MEMREF_F_NORMAL   0x00 /* Normal flags */
#define MEMREF_F_NULLABLE 0x01 /* Location may contain NULL */
#if 0 /* Leads to problems down the line... */
#define MEMREF_F_DOKILL   0x02 /* Allowed to use Dee_DecrefDoKill() */
#endif
#define MEMREF_F_NOKILL   0x04 /* Allowed to use Dee_DecrefNoKill() */
#define _MEMREF_F_DONE    0x40 /* Used internally */
#define _MEMREF_F_NOSRC   0x80 /* Used internally */

/* Descriptor for a held reference
 * NOTE: This structure is designed to be able to impersonate
 *       a `struct Dee_memval' in a pinch, and act as if it was
 *       a DIRECT object (only the mo_typeof field is broken) */
struct Dee_memref {
	struct Dee_memloc mr_loc;       /* Underlying memory location */
	uintptr_t         mr_refc;      /* [>= 1] # of references held to `mr_loc' */
	uint8_t          _mr_always0_1; /* Always 0 */
	uint8_t          _mr_always0_2; /* Always 0 */
	uint8_t          _mr_always0_3; /* Always 0 */
	uint8_t           mr_flags;     /* Special flags (set of `MEMREF_F_*') */
#if __SIZEOF_POINTER__ > 4
	uint8_t          _mr_pad[sizeof(void *) - 4]; /* Padding (uninitialized) */
#endif /* __SIZEOF_POINTER__ > 4 */
};

/* Compare "a" and "b". This is the function used to sort `exi_memrefv' */
#define Dee_memref_compare(a, b) \
	memcmp(&(a)->mr_loc, &(b)->mr_loc, sizeof(struct Dee_memloc))

/* Compare "a" and "b". This is the function used by `Dee_except_exitinfo_id_compare' */
#define Dee_memref_compare2(a, b) \
	memcmp(a, b, offsetof(struct Dee_memref, _mr_always0_1))

struct Dee_except_exitinfo_id {
	Dee_cfa_t                                  exi_cfa_offset; /* CFA offset on entry to this block. */
	Dee_vstackaddr_t                           exi_memrefc;    /* # of references held */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_memref, exi_memrefv);   /* [0..exi_memrefc] Vector of held object references (sorted by `Dee_memref_compare()'). */
};

/* Small descriptor for what needs to be cleaned up in a `struct Dee_memstate' */
struct Dee_except_exitinfo {
	struct Dee_host_section                    exi_text;       /* Host assembly text */
	struct Dee_except_exitinfo                *exi_next;       /* [0..1] Exception handler that this one falls into. */
	Dee_cfa_t                                  exi_cfa_offset; /* CFA offset on entry to this block. */
	Dee_vstackaddr_t                           exi_memrefc;    /* # of references held */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_memref, exi_memrefv);   /* [0..exi_memrefc] Vector of held object references (sorted by `Dee_memref_compare()'). */
};

#define Dee_except_exitinfo_asid(self) \
	((struct Dee_except_exitinfo_id *)&(self)->exi_cfa_offset)

/* Check if `self' has been compiled. */
#define Dee_except_exitinfo_wascompiled(self) \
	((self)->exi_text.hs_start < (self)->exi_text.hs_end || (self)->exi_next)

#define Dee_except_exitinfo_alloc(sizeof) ((struct Dee_except_exitinfo *)Dee_Malloc(sizeof))
#define Dee_except_exitinfo_free(self)    Dee_Free(self)
#define Dee_except_exitinfo_destroy(self) (Dee_host_section_fini(&(self)->exi_text), Dee_Free(self))
#define _Dee_except_exitinfo_cmp_baseof(x) (&(x)->exi_cfa_offset)
#define _Dee_except_exitinfo_cmp_sizeof(x)                    \
	((offsetof(struct Dee_except_exitinfo, exi_memrefv) -     \
	  offsetof(struct Dee_except_exitinfo, exi_cfa_offset)) + \
	 ((x)->exi_memrefc * sizeof(struct Dee_memref)))

/* Compare all of the memory locations and reference counts between "a" and "b" */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) int DCALL
Dee_except_exitinfo_id_compare(struct Dee_except_exitinfo_id const *__restrict a,
                               struct Dee_except_exitinfo_id const *__restrict b);

/* Return the upper bound for the required buffer size in order to represent "state" */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
Dee_except_exitinfo_id_sizefor(struct Dee_memstate const *__restrict state);

/* Initialize `self' from `state'
 * @return: * : Always re-returns `self' */
INTDEF NONNULL((1, 2)) struct Dee_except_exitinfo_id *DCALL
Dee_except_exitinfo_id_init(struct Dee_except_exitinfo_id *__restrict self,
                            struct Dee_memstate const *__restrict state);

/* Calculate the "distance" score that determines the complexity of the
 * transitioning code needed to morph from `oldinfo' to `newinfo'. When
 * ordering exception cleanup code, exit descriptors should be ordered
 * such that the fallthru of one to the next always yields the lowest
 * distance score.
 * @return: * : The distance scrore for morphing from `oldinfo' to `newinfo' */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
Dee_except_exitinfo_id_distance(struct Dee_except_exitinfo_id const *__restrict oldinfo,
                                struct Dee_except_exitinfo_id const *__restrict newinfo);





struct Dee_inlined_references {
	/* TODO: Need some way to find out which objects actually end up being used
	 *       by the final host code. When a basic block needs to be re-compiled,
	 *       or when constants end up unused because they could be propagated
	 *       further, then they still remain in this set, which is sub-optimal.
	 * IMPORTANT: Given a constant tuple "t", host text referencing `DeeTuple_ELEM(t)'
	 *            is *also* a valid usage! */
	size_t           ir_mask; /* [> ir_size || ir_mask == 0] Allocated set size. */
	size_t           ir_size; /* [< ir_mask || ir_mask == 0] Amount of non-NULL keys. */
#ifdef __INTELLISENSE__
	DeeObject      **ir_elem; /* [0..ir_size|ALLOC(ir_mask+2)] Set keys (+ one extra slot for a trailing NULL to-be added later). */
#else /* __INTELLISENSE__ */
	DREF DeeObject **ir_elem; /* [0..ir_size|ALLOC(ir_mask+2)] Set keys (+ one extra slot for a trailing NULL to-be added later). */
#endif /* !__INTELLISENSE__ */
};

#define Dee_inlined_references_init(self) bzero(self, sizeof(struct Dee_inlined_references))
INTDEF NONNULL((1)) void DCALL Dee_inlined_references_fini(struct Dee_inlined_references *__restrict self);

#define Dee_inlined_references_hashof(obj)          DeeObject_HashGeneric(obj)
#define Dee_inlined_references_hashst(self, hash)  ((hash) & (self)->ir_mask)
#define Dee_inlined_references_hashnx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define Dee_inlined_references_hashit(self, i)     ((self)->ir_elem + ((i) & (self)->ir_mask))

/* Make sure that `inherit_me' appears in `self', thus inheriting a reference to it.
 * @return: inherit_me: Success: `self' now owns the reference to `inherit_me', and you can use it lazily
 * @return: NULL:       Error */
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL
Dee_inlined_references_ref(struct Dee_inlined_references *__restrict self,
                           /*inherit(always)*/ DREF DeeObject *inherit_me);


struct Dee_function_assembler {
	DeeFunctionObject            *fa_function;     /* [1..1][const][valid_if(!HOSTFUNC_CC_F_FUNC)] The function being assembled */
	DeeCodeObject                *fa_code;         /* [1..1][const][== fa_function->fo_code] The code being assembled */
	struct Dee_host_section       fa_prolog;       /* Function prolog (output even before `fa_blockv[0]'; verify arguments & set-up initial memstate) */
	DREF struct Dee_memstate     *fa_prolog_end;   /* [0..1] Memory state at the end of the prolog (or `NULL' if `Dee_function_assembler_compileblocks()' wasn't called, yet) */
	struct Dee_basic_block       *fa_deleted;      /* [0..n][owned] Chain (via `bb_next') of blocks deleted by `Dee_function_assembler_trimdead()' (need to keep around because of unused symbols referencing these blocks) */
	struct Dee_basic_block      **fa_blockv;       /* [owned][0..fa_blockc][owned] Vector of basic blocks (sorted by `bb_deemon_start'). */
	size_t                        fa_blockc;       /* Number of basic blocks. */
	size_t                        fa_blocka;       /* Allocated number of basic blocks. */
	struct Dee_except_exitinfo   *fa_except_del;   /* [0..n][owned] Chain of deleted exception exits */
	struct Dee_except_exitinfo  **fa_except_exitv; /* [owned][0..fa_except_exitc][owned] Vector of exception exits (sorted by `Dee_except_exitinfo_id_compare()') */
	size_t                        fa_except_exitc; /* Number of exception exit basic blocks. */
	size_t                        fa_except_exita; /* Allocated number of exception exit basic blocks. */
	struct Dee_except_exitinfo   *fa_except_first; /* [0..1] The first except exit descriptor (used by `Dee_function_assembler_ordersections()') */
	struct Dee_host_symbol       *fa_symbols;      /* [0..1][owned] Chain of allocated symbols. */
#define DEE_FUNCTION_ASSEMBLER_F_NORMAL     0x0000 /* Normal flags */
#define DEE_FUNCTION_ASSEMBLER_F_OSIZE      0x0001 /* Optimize for size (generally means: try not to use cold sections or inlines) */
#define DEE_FUNCTION_ASSEMBLER_F_SAFE       0x0002 /* Generate "safe" code (for `CODE_FASSEMBLY' code) */
#define DEE_FUNCTION_ASSEMBLER_F_NOROINLINE 0x0004 /* Don't inline references to already-bound class members/globals, even if the location is `Dee_CLASS_ATTRIBUTE_FREADONLY' / `Dee_MODSYM_FREADONLY' */
#define DEE_FUNCTION_ASSEMBLER_F_NOEARLYDEL 0x0008 /* Don't delete local variables as early as possible (when set, code behaves more closely to original byte-code, but at a significant overhead) */
#define DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE 0x0010 /* Don't use RTTI from dex modules and the deemon core for the purpose of figuring out object types (set to work around buggy RTTI) */
#define DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR 0x0020 /* Don't exit early when illegal operations are detected. Instead, generate code that produces the correct runtime error. */
#ifdef HOSTASM_X86_64
#define DEE_FUNCTION_ASSEMBLER_F_MCLARGE    0x8000 /* Generate code for a large memory model (supporting .text outside the -2Gib+2Gib range) */
#endif /* !HOSTASM_X86_64 */
	uint16_t                      fa_flags;        /* [const] Code generation flags (set of `DEE_FUNCTION_ASSEMBLER_F_*'). */
	Dee_ulid_t                    fa_localc;       /* [const][== fa_code->co_localc] */
	Dee_lid_t                     fa_xlocalc;      /* [const][== fa_code->co_localc + MEMSTATE_XLOCAL_MINCOUNT + (fa_code->co_argc_max - fa_code->co_argc_min)] */
	Dee_hostfunc_cc_t             fa_cc;           /* [const] Calling convention. */
	struct Dee_inlined_references fa_irefs;        /* Inlined object references (must be ) */
	struct Dee_host_section_tailq fa_sections;     /* [0..n] Linked list of output sections (via `hs_link') */
	size_t                        fa_sectsize;     /* Total size of all sections combined */
};

#define Dee_function_assembler_addrof(self, addr) \
	((Dee_code_addr_t)((addr) - (self)->fa_code->co_code))

#define Dee_function_assembler_init(self, function, code, cc, flags) \
	(void)((self)->fa_function = (function),                         \
	       (self)->fa_code     = (code),                             \
	       Dee_host_section_init(&(self)->fa_prolog),                \
	       (self)->fa_prolog_end   = NULL,                           \
	       (self)->fa_deleted      = NULL,                           \
	       (self)->fa_blockv       = NULL,                           \
	       (self)->fa_blockc       = 0,                              \
	       (self)->fa_blocka       = 0,                              \
	       (self)->fa_except_del   = NULL,                           \
	       (self)->fa_except_exitv = NULL,                           \
	       (self)->fa_except_exitc = 0,                              \
	       (self)->fa_except_exita = 0,                              \
	       (self)->fa_except_first = NULL,                           \
	       (self)->fa_symbols      = NULL,                           \
	       (self)->fa_flags        = (flags),                        \
	       (self)->fa_localc       = (self)->fa_code->co_localc,     \
	       (self)->fa_xlocalc = ((self)->fa_localc +                 \
	                             MEMSTATE_XLOCAL_MINCOUNT +          \
	                             (self)->fa_code->co_argc_max -      \
	                             (self)->fa_code->co_argc_min),      \
	       (self)->fa_cc = (cc),                                     \
	       Dee_inlined_references_init(&(self)->fa_irefs))
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
                                   struct Dee_memstate const *__restrict state);

/* Allocate a new host text symbol and return it.
 * @return: * :   The newly allocated host text symbol
 * @return: NULL: Error */
#ifdef HAVE_DEE_HOST_SYMBOL_ALLOC_INFO
#ifdef NO_HOSTASM_DEBUG_PRINT
INTDEF WUNUSED NONNULL((1)) struct Dee_host_symbol *DCALL
Dee_function_assembler_newsym_dbg(struct Dee_function_assembler *__restrict self,
                                  char const *file, int line);
#define Dee_function_assembler_newsym_named_dbg(self, name, file, line) Dee_function_assembler_newsym_dbg(self, __FILE__, __LINE__)
#define Dee_function_assembler_newsym_named(self, name)                 Dee_function_assembler_newsym_dbg(self, __FILE__, __LINE__)
#define Dee_function_assembler_newsym(self)                             Dee_function_assembler_newsym_dbg(self, __FILE__, __LINE__)
#else /* NO_HOSTASM_DEBUG_PRINT */
INTDEF WUNUSED NONNULL((1)) struct Dee_host_symbol *DCALL
Dee_function_assembler_newsym_named_dbg(struct Dee_function_assembler *__restrict self,
                                        char const *name, char const *file, int line);
#define Dee_function_assembler_newsym_named(self, name)     Dee_function_assembler_newsym_named_dbg(self, name, __FILE__, __LINE__)
#define Dee_function_assembler_newsym_dbg(self, file, line) Dee_function_assembler_newsym_named_dbg(self, NULL, __FILE__, __LINE__)
#define Dee_function_assembler_newsym(self)                 Dee_function_assembler_newsym_named_dbg(self, NULL, __FILE__, __LINE__)
#endif /* !NO_HOSTASM_DEBUG_PRINT */
#elif defined(NO_HOSTASM_DEBUG_PRINT)
INTDEF WUNUSED NONNULL((1)) struct Dee_host_symbol *DCALL
Dee_function_assembler_newsym(struct Dee_function_assembler *__restrict self);
#define Dee_function_assembler_newsym_dbg(self, file, line)             Dee_function_assembler_newsym(self)
#define Dee_function_assembler_newsym_named(self, name)                 Dee_function_assembler_newsym(self)
#define Dee_function_assembler_newsym_named_dbg(self, name, file, line) Dee_function_assembler_newsym(self)
#else /* ... */
INTDEF WUNUSED NONNULL((1)) struct Dee_host_symbol *DCALL
Dee_function_assembler_newsym_named(struct Dee_function_assembler *__restrict self);
#define Dee_function_assembler_newsym(self, name)                       Dee_function_assembler_newsym_named(self, name)
#define Dee_function_assembler_newsym_dbg(self, file, line)             Dee_function_assembler_newsym_named(self, NULL)
#define Dee_function_assembler_newsym_named_dbg(self, name, file, line) Dee_function_assembler_newsym_named(self, name)
#endif /* !... */

#define Dee_function_assembler_inlineref(self, inherit_me) \
	Dee_inlined_references_ref(&(self)->fa_irefs, inherit_me)


#ifdef DEE_HOST_RELOCVALUE_SECT
#define Dee_function_assembler_DEFINE_Dee_host_symbol_section(self, Lerr, name, sect, off) \
	struct Dee_host_symbol _##name, *name = &_##name;                                      \
	_##name.hs_type                  = DEE_HOST_SYMBOL_SECT;                               \
	_##name.hs_value.sv_sect.ss_sect = (sect);                                             \
	_##name.hs_value.sv_sect.ss_off  = (off);                                              \
	Dee_host_symbol_setname(&_##name, "." #name)
#else /* DEE_HOST_RELOCVALUE_SECT */
#define Dee_function_assembler_DEFINE_Dee_host_symbol_section(self, Lerr, name, sect, off) \
	struct Dee_host_symbol *name = Dee_function_assembler_newsym_named(self, "." #name);   \
	if unlikely(!name)                                                                     \
		goto Lerr;                                                                         \
	name->hs_type                  = DEE_HOST_SYMBOL_SECT;                                 \
	name->hs_value.sv_sect.ss_sect = (sect);                                               \
	name->hs_value.sv_sect.ss_off  = (off)
#endif /* !DEE_HOST_RELOCVALUE_SECT */
#define Dee_function_generator_DEFINE_Dee_host_symbol_section(self, Lerr, name, sect, off) \
	Dee_function_assembler_DEFINE_Dee_host_symbol_section((self)->fg_assembler, Lerr, name, sect, off)



/* ================ Loaders ================ */

struct Dee_function_generator;
struct Dee_function_exceptinject {
	struct Dee_function_exceptinject *fei_next;  /* [0..1] Next exception injection. */
	WUNUSED_T NONNULL_T((1, 2))                  /* [1..1] Function to call in order to produce injected code. */
	int (DCALL *fei_inject)(struct Dee_function_generator *__restrict self,
	                        struct Dee_function_exceptinject *__restrict inject);
	Dee_vstackaddr_t                  fei_stack; /* Expected v-stack depth for `fei_inject' (must do vpop() until reached) */
};

struct Dee_function_generator {
	struct Dee_function_assembler      *fg_assembler;        /* [1..1][const] Assembler. */
	struct Dee_basic_block             *fg_block;            /* [1..1][const] Output basic block. */
	struct Dee_host_section            *fg_sect;             /* [1..1] Output section (usually `fg_block->bb_htext' or `fg_block->bb_hcold').
	                                                          * NOTE: If you alter this, you must also (and *always*) restore it. */
#ifdef __INTELLISENSE__
	struct Dee_memstate                *fg_state;            /* [1..1] Current memory state. */
#else /* __INTELLISENSE__ */
	DREF struct Dee_memstate           *fg_state;            /* [1..1] Current memory state. */
#endif /* !__INTELLISENSE__ */
	struct Dee_memstate const          *fg_state_hstack_res; /* [0..1] State defining some extra reserved hstack locations (s.a. `Dee_memstate_hstack_find()'). */
	struct Dee_basic_block_loclastread *fg_nextlastloc;      /* [0..1] The next time some local will be read for the last time (only for `Dee_function_generator_geninstr()') */
	struct Dee_function_exceptinject   *fg_exceptinject;     /* [0..1] Chain of extra code that needs to be injected for exception handlers. */
};

#define _Dee_function_generator_initcommon(self) \
	(void)((self)->fg_state_hstack_res = NULL, (self)->fg_exceptinject = NULL)

#define Dee_function_generator_state_unshare(self)   Dee_memstate_unshare(&(self)->fg_state)
#define Dee_function_generator_state_dounshare(self) Dee_memstate_dounshare(&(self)->fg_state)

/* Return a basic block that should be jumped to in order to handle a exception. */
#define Dee_function_generator_except_exit(self) \
	Dee_function_assembler_except_exit((self)->fg_assembler, (self)->fg_state)

#define Dee_function_generator_gadjust_cfa_offset(self, delta) \
	(void)((self)->fg_state->ms_host_cfa_offset += (ptrdiff_t)(delta))

/* Allocate new host symbols. */
#define Dee_function_generator_newsym(self)                             Dee_function_assembler_newsym((self)->fg_assembler)
#define Dee_function_generator_newsym_named(self, name)                 Dee_function_assembler_newsym_named((self)->fg_assembler, name)
#define Dee_function_generator_newsym_dbg(self, file, line)             Dee_function_assembler_newsym_dbg((self)->fg_assembler, file, line)
#define Dee_function_generator_newsym_named_dbg(self, name, file, line) Dee_function_assembler_newsym_named_dbg((self)->fg_assembler, name, file, line)

/* Generate an inlined reference. */
#define Dee_function_generator_inlineref(self, inherit_me) \
	Dee_function_assembler_inlineref((self)->fg_assembler, inherit_me)

/* Push/pop exception handler injections. */
#define Dee_function_generator_xinject_push(self, ij)             \
	(void)((ij)->fei_next          = (self)->fg_exceptinject,     \
	       (ij)->fei_stack         = (self)->fg_state->ms_stackc, \
	       (self)->fg_exceptinject = (ij))
#define Dee_function_generator_xinject_pop(self, ij)    \
	(void)(Dee_ASSERT((self)->fg_exceptinject == (ij)), \
	       (self)->fg_exceptinject = (ij)->fei_next)

/* Memory equivalence helpers. */
#define Dee_function_generator_remember_getclassof(self, loc)                                               Dee_memstate_remember_getclassof((self)->fg_state, loc)
#define Dee_function_generator_remember_getclassof_reg(self, regno)                                         Dee_memstate_remember_getclassof_reg((self)->fg_state, regno)
#define Dee_function_generator_remember_getclassof_regind(self, regno, ind_delta)                           Dee_memstate_remember_getclassof_regind((self)->fg_state, regno, ind_delta)
#define Dee_function_generator_remember_getclassof_hstackind(self, cfa_offset)                              Dee_memstate_remember_getclassof_hstackind((self)->fg_state, cfa_offset)
#define Dee_function_generator_remember_movevalue(self, from, to)                                           Dee_memstate_remember_movevalue((self)->fg_state, from, to)
#define Dee_function_generator_remember_deltavalue(self, loc, delta)                                        Dee_memstate_remember_deltavalue((self)->fg_state, loc, delta)
#define Dee_function_generator_remember_undefined(self, loc)                                                Dee_memstate_remember_undefined((self)->fg_state, loc)
#define Dee_function_generator_remember_undefined_allregs(self)                                             Dee_memstate_remember_undefined_allregs((self)->fg_state)
#define Dee_function_generator_remember_undefined_unusedregs(self)                                          Dee_memstate_remember_undefined_unusedregs((self)->fg_state)
#define Dee_function_generator_remember_undefined_reg(self, regno)                                          Dee_memstate_remember_undefined_reg((self)->fg_state, regno)
#define Dee_function_generator_remember_undefined_regind(self, regno, ind_delta)                            Dee_memstate_remember_undefined_regind((self)->fg_state, regno, ind_delta)
#define Dee_function_generator_remember_undefined_hstackind(self, cfa_offset)                               Dee_memstate_remember_undefined_hstackind((self)->fg_state, cfa_offset)
#define Dee_function_generator_remember_undefined_constind(self, p_value)                                   Dee_memstate_remember_undefined_constind((self)->fg_state, p_value)
#define Dee_function_generator_remember_movevalue_reg2hstackind(self, src_regno, cfa_offset)                Dee_memstate_remember_movevalue_reg2hstackind((self)->fg_state, src_regno, cfa_offset)
#define Dee_function_generator_remember_movevalue_regind2hstackind(self, src_regno, src_delta, cfa_offset)  Dee_memstate_remember_movevalue_regind2hstackind((self)->fg_state, src_regno, src_delta, cfa_offset)
#define Dee_function_generator_remember_movevalue_hstack2reg(self, cfa_offset, dst_regno)                   Dee_memstate_remember_movevalue_hstack2reg((self)->fg_state, cfa_offset, dst_regno)
#define Dee_function_generator_remember_movevalue_hstackind2reg(self, cfa_offset, dst_regno)                Dee_memstate_remember_movevalue_hstackind2reg((self)->fg_state, cfa_offset, dst_regno)
#define Dee_function_generator_remember_movevalue_const2reg(self, value, dst_regno)                         Dee_memstate_remember_movevalue_const2reg((self)->fg_state, value, dst_regno)
#define Dee_function_generator_remember_movevalue_const2regind(self, value, dst_regno, dst_delta)           Dee_memstate_remember_movevalue_const2regind((self)->fg_state, value, dst_regno, dst_delta)
#define Dee_function_generator_remember_movevalue_const2hstackind(self, value, cfa_offset)                  Dee_memstate_remember_movevalue_const2hstackind((self)->fg_state, value, cfa_offset)
#define Dee_function_generator_remember_movevalue_hstack2hstackind(self, src_cfa_offset, dst_cfa_offset)    Dee_memstate_remember_movevalue_hstack2hstackind((self)->fg_state, src_cfa_offset, dst_cfa_offset)
#define Dee_function_generator_remember_movevalue_hstackind2hstackind(self, src_cfa_offset, dst_cfa_offset) Dee_memstate_remember_movevalue_hstackind2hstackind((self)->fg_state, src_cfa_offset, dst_cfa_offset)
#define Dee_function_generator_remember_movevalue_const2constind(self, value, p_value)                      Dee_memstate_remember_movevalue_const2constind((self)->fg_state, value, p_value)
#define Dee_function_generator_remember_movevalue_reg2reg(self, src_regno, dst_regno)                       Dee_memstate_remember_movevalue_reg2reg((self)->fg_state, src_regno, dst_regno)
#define Dee_function_generator_remember_movevalue_regx2reg(self, src_regno, src_delta, dst_regno)           Dee_memstate_remember_movevalue_regx2reg((self)->fg_state, src_regno, src_delta, dst_regno)
#define Dee_function_generator_remember_movevalue_regind2reg(self, src_regno, src_delta, dst_regno)         Dee_memstate_remember_movevalue_regind2reg((self)->fg_state, src_regno, src_delta, dst_regno)
#define Dee_function_generator_remember_movevalue_reg2regind(self, src_regno, dst_regno, dst_delta)         Dee_memstate_remember_movevalue_reg2regind((self)->fg_state, src_regno, dst_regno, dst_delta)
#define Dee_function_generator_remember_movevalue_constind2reg(self, p_value, dst_regno)                    Dee_memstate_remember_movevalue_constind2reg((self)->fg_state, p_value, dst_regno)
#define Dee_function_generator_remember_movevalue_reg2constind(self, src_regno, p_value)                    Dee_memstate_remember_movevalue_reg2constind((self)->fg_state, src_regno, p_value)


/* Code generator helpers to manipulate the V-stack. */
#define Dee_function_generator_vtop(self)     Dee_memstate_vtop((self)->fg_state)
#define Dee_function_generator_vtopdobj(self) Dee_memval_direct_getobj(Dee_function_generator_vtop(self))
#define Dee_function_generator_vtopdloc(self) Dee_memval_direct_getloc(Dee_function_generator_vtop(self))
#define Dee_function_generator_vtoptype(self) Dee_memval_typeof(Dee_function_generator_vtop(self))
#define Dee_function_generator_vtop_isdirect(self) Dee_memval_isdirect(Dee_function_generator_vtop(self))
#define Dee_function_generator_vtop_direct_isref(self)      Dee_memobj_isref(Dee_memval_direct_getobj(Dee_function_generator_vtop(self)))
#define Dee_function_generator_vtop_direct_setref(self)     Dee_memobj_setref(Dee_memval_direct_getobj(Dee_function_generator_vtop(self)))
#define Dee_function_generator_vtop_direct_clearref(self)   Dee_memobj_clearref(Dee_memval_direct_getobj(Dee_function_generator_vtop(self)))
#define Dee_function_generator_isoneref_noalias(self, mval) Dee_memstate_isoneref_noalias((self)->fg_state, mval)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vswap(struct Dee_function_generator *__restrict self); /* ASM_SWAP */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vlrot(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vrrot(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vmirror(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n); /* a,b,c,d -> d,c,b,a */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_memadr(struct Dee_function_generator *__restrict self, struct Dee_memadr const *adr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_memloc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_memobj(struct Dee_function_generator *__restrict self, struct Dee_memobj const *obj);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_memval(struct Dee_function_generator *__restrict self, struct Dee_memval const *val);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_undefined(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_addr(struct Dee_function_generator *__restrict self, void const *addr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_const_(struct Dee_function_generator *__restrict self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_cid(struct Dee_function_generator *__restrict self, uint16_t cid);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_vpush_cid_t(struct Dee_function_generator *__restrict self, uint16_t cid, DeeTypeObject *__restrict type);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_rid(struct Dee_function_generator *__restrict self, uint16_t rid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_hreg(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t val_delta);                         /* %regno + val_delta                    (MEMOBJ_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_hregind(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t ind_delta, ptrdiff_t val_delta); /* *(%regno + ind_delta) + val_delta (MEMOBJ_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_hstack(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset);                                                 /* (MEMOBJ_F_NOREF) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_hstackind(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset, ptrdiff_t val_delta);                         /* (MEMOBJ_F_NOREF) */
#define Dee_function_generator_vpush_none(self)           Dee_function_generator_vpush_const_(self, Dee_None)
#define Dee_function_generator_vpush_const(self, value)   Dee_function_generator_vpush_const_(self, (DeeObject *)Dee_REQUIRES_OBJECT(value))
#define Dee_function_generator_vpush_NULL(self)           Dee_function_generator_vpush_addr(self, NULL)
#define Dee_function_generator_vpush_imm8(self, imm8)     Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(uint8_t)(imm8))
#define Dee_function_generator_vpush_Simm8(self, Simm8)   Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(intptr_t)(int8_t)(Simm8))
#define Dee_function_generator_vpush_imm16(self, imm16)   Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(uint16_t)(imm16))
#define Dee_function_generator_vpush_Simm16(self, Simm16) Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(intptr_t)(int16_t)(Simm16))
#define Dee_function_generator_vpush_imm32(self, imm32)   Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(uint32_t)(imm32))
#define Dee_function_generator_vpush_Simm32(self, Simm32) Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(intptr_t)(int32_t)(Simm32))
#define Dee_function_generator_vpush_immSIZ(self, immSIZ) Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(size_t)(immSIZ))
#define Dee_function_generator_vpush_immINT(self, immINT) Dee_function_generator_vpush_addr(self, (void *)(uintptr_t)(intptr_t)(int)(immINT))
#define Dee_function_generator_vpush_WEAKREF_SUPPORT_INIT(self) Dee_function_generator_vpush_NULL(self)
#ifndef CONFIG_NO_THREADS
#define Dee_function_generator_vpush_ATOMIC_RWLOCK_INIT(self) Dee_function_generator_vpush_NULL(self)
#endif /* !CONFIG_NO_THREADS */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_arg(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_aid_t aid); /* `instr' is needed for `libhostasm_rt_err_unbound_arg' */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vbound_arg(struct Dee_function_generator *__restrict self, Dee_aid_t aid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_arg_present(struct Dee_function_generator *__restrict self, Dee_aid_t aid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_local(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_lid_t lid); /* `instr' is needed for `libhostasm_rt_err_unbound_local' (if NULL, no bound-check is done) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vbound_local(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_lid_t lid); /* `instr' is needed for automatic deletion of unused locals */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdup_n(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
#define Dee_function_generator_vdup(self) Dee_function_generator_vdup_n(self, 1)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpopmany(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_n(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_local(struct Dee_function_generator *__restrict self, Dee_lid_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_local(struct Dee_function_generator *__restrict self, Dee_lid_t lid);
INTDEF WUNUSED NONNULL((1)) bool DCALL Dee_function_generator_vallconst(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n); /* Check if top `n' elements are all `MEMADR_TYPE_CONST' */
INTDEF WUNUSED NONNULL((1)) bool DCALL Dee_function_generator_vallconst_noref(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n); /* Check if top `n' elements are all `MEMADR_TYPE_CONST' and have the `MEMOBJ_F_NOREF' flag set. */


/* Generate code needed to drop references held by `mval' (where `mval' must be a vstack item,
 * or a local variable that is unconditionally bound or non-direct).
 * NOTE: This function is somewhere between the v* and g* APIs, though it does *NOT* unshare or
 *       realloc memstate components. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vgdecref_vstack(struct Dee_function_generator *__restrict self,
                                       struct Dee_memval *mval);

/* Generate code needed to drop references held by `mval' (where `mval' must point into locals)
 * NOTE: This function is somewhere between the v* and g* APIs, though it does *NOT* unshare or
 *       realloc memstate components. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vgdecref_local(struct Dee_function_generator *__restrict self,
                                      struct Dee_memval *__restrict mval);

#if 0
/* Wrapper around:
 * - Dee_function_generator_vgdecref_vstack
 * - Dee_function_generator_vgdecref_local
 * ... that automatically checks if `mval' points into the current mem-state's
 * local variable list to see which function needs to be used. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vgdecref(struct Dee_function_generator *__restrict self,
                                struct Dee_memval *__restrict mval);
#endif

/* Remember that VTOP, as well as any other memory location
 * that might be aliasing it is an instance of "type" at runtime. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vsettyp(struct Dee_function_generator *__restrict self, DeeTypeObject *type);
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vsettyp_noalias(struct Dee_function_generator *__restrict self, DeeTypeObject *type);

/* Helpers for invoking certain operators. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcall(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc);       /* func, [args...]           -> result -- Invoke `DeeObject_Call()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallkw(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc);     /* func, [args...], kw       -> result -- Invoke `DeeObject_CallKw()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcalltuple(struct Dee_function_generator *__restrict self);                         /* func, args                -> result -- Invoke `DeeObject_CallTuple()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcalltuplekw(struct Dee_function_generator *__restrict self);                       /* func, args, kw            -> result -- Invoke `DeeObject_CallTupleKw()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopthiscall(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc);   /* func, this, [args...]     -> result -- Invoke `DeeObject_ThisCall()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopthiscallkw(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc); /* func, this, [args...], kw -> result -- Invoke `DeeObject_ThisCallKw()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopthiscalltuple(struct Dee_function_generator *__restrict self);                     /* func, this, args          -> result -- Invoke `DeeObject_ThisCallTuple()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopthiscalltuplekw(struct Dee_function_generator *__restrict self);                   /* func, this, args, kw      -> result -- Invoke `DeeObject_ThisCallTupleKw()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallattr(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc);   /* this, attr, [args...]     -> result -- Invoke `DeeObject_CallAttr()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallattrkw(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t argc); /* this, attr, [args...], kw -> result -- Invoke `DeeObject_CallAttrKw()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallattrtuple(struct Dee_function_generator *__restrict self);                     /* this, attr, args          -> result -- Invoke `DeeObject_CallAttrTuple()' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallattrtuplekw(struct Dee_function_generator *__restrict self);                   /* this, attr, args, kw      -> result -- Invoke `DeeObject_CallAttrTupleKw()' and push the result */

INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallseq(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t itemc);     /* func, [items...]              -> result -- Invoke `DeeObject_Call(func, DeeSharedVector_NewShared(...))' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallmap(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t pairc);     /* func, [[key, value]...]       -> result -- Invoke `DeeObject_Call(func, DeeSharedMap_NewShared(...))' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallattrseq(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t itemc); /* func, attr, [items...]        -> result -- Invoke `DeeObject_CallAttr(func, attr, DeeSharedVector_NewShared(...))' and push the result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopcallattrmap(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t pairc); /* func, attr, [[key, value]...] -> result -- Invoke `DeeObject_CallAttr(func, attr, DeeSharedMap_NewShared(...))' and push the result */

INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopgetattr(struct Dee_function_generator *__restrict self);   /* this, attr        -> result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vophasattr(struct Dee_function_generator *__restrict self);   /* this, attr        -> hasattr */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopboundattr(struct Dee_function_generator *__restrict self); /* this, attr        -> bound */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopdelattr(struct Dee_function_generator *__restrict self);   /* this, attr        -> N/A */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopsetattr(struct Dee_function_generator *__restrict self);   /* this, attr, value -> N/A */

INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopgetitemdef(struct Dee_function_generator *__restrict self); /* seq, key_or_index, def -> result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopbounditem(struct Dee_function_generator *__restrict self);  /* seq, key_or_index      -> bound */

#define VOPBOOL_F_NORMAL      0x0000 /* Normal flags */
#define VOPBOOL_F_FORCE_MORPH 0x0001 /* Ensure that vtop is a constant Dee_True/Dee_False or MEMVAL_VMORPH_ISBOOL */
#define VOPBOOL_F_NOFALLBACK  0x0002 /* Instead of generating a call to `tp_bool' (when not noexcept) or `DeeObject_Bool', return "1" */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopbool(struct Dee_function_generator *__restrict self, unsigned int flags); /* value -> bool */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopnot(struct Dee_function_generator *__restrict self);                      /* value -> !bool */

/* Misc operators that can be optimized-, or behave in special ways. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopsize(struct Dee_function_generator *__restrict self); /* value -> size */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopint(struct Dee_function_generator *__restrict self);  /* value -> int */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopstr(struct Dee_function_generator *__restrict self);  /* value -> string */

/* Helpers to implement "is" */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopinstanceof(struct Dee_function_generator *__restrict self); /* this, type -> bool */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopimplements(struct Dee_function_generator *__restrict self); /* this, type -> bool */

/* Helpers to evaluate an object into a C integer */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vmorph_int(struct Dee_function_generator *__restrict self);  /* value -> DeeObject_AsIntptr(value) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vmorph_uint(struct Dee_function_generator *__restrict self); /* value -> DeeObject_AsUIntptr(value) */

/* Helpers for wrapping values as simple object morphs */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_vmakemorph(struct Dee_function_generator *__restrict self, uint8_t morph); /* value -> ...(value) */
#define Dee_function_generator_vcall_DeeInt_NewIntptr(self)  _Dee_function_generator_vmakemorph(self, MEMVAL_VMORPH_INT)
#define Dee_function_generator_vcall_DeeInt_NewUIntptr(self) _Dee_function_generator_vmakemorph(self, MEMVAL_VMORPH_UINT)
#define Dee_function_generator_vcall_DeeBool_For(self)       _Dee_function_generator_vmakemorph(self, MEMVAL_VMORPH_BOOL_NZ)

INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* [elems...] -> seq (seq_type must be &DeeList_Type or &DeeTuple_Type) */
Dee_function_generator_vpackseq(struct Dee_function_generator *__restrict self,
                                DeeTypeObject *__restrict seq_type, Dee_vstackaddr_t elemc);

/* [args...]  ->  result (flags == VOP_F_PUSHRES)
 * [args...]  ->  N/A    (flags == VOP_F_NORMAL) */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vop(struct Dee_function_generator *__restrict self,
                           uint16_t operator_name, Dee_vstackaddr_t argc,
                           unsigned int flags);
#define VOP_F_NORMAL      0x0000 /* Normal flags */
#define VOP_F_PUSHRES     0x0001 /* Push the operator's result */
#define VOP_F_ALLOWNATIVE 0x0002 /* Allow (e.g.) use of `DeeObject_SetRangeIndex()' instead of `DeeObject_SetRange()' */

/* this, args  ->  result (flags == VOP_F_PUSHRES)
 * this, args  ->  N/A    (flags == VOP_F_NORMAL)
 * Same as `Dee_function_generator_vop()', but arguments are given as via what
 * should be a tuple-object (the type is asserted by this function) in vtop.
 * NOTE: A tuple-type check is only generated if DEE_FUNCTION_ASSEMBLER_F_SAFE is set. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_voptuple(struct Dee_function_generator *__restrict self,
                                uint16_t operator_name, unsigned int flags);

/* [ref]:this, [args...]  ->  [ref]:this, result (flags == VOP_F_PUSHRES)
 * [ref]:this, [args...]  ->  [ref]:this         (flags == VOP_F_NORMAL)
 * NOTE: If "this" isn't a constant, it is the caller's responsibility to
 *       ensure that "this" doesn't have any aliases. Otherwise, aliases
 *       might inadvertently also receive the updated object. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vinplaceop(struct Dee_function_generator *__restrict self,
                                  uint16_t operator_name, Dee_vstackaddr_t argc,
                                  unsigned int flags);

/* [ref]:this, args  ->  [ref]:this, result (flags == VOP_F_PUSHRES)
 * [ref]:this, args  ->  [ref]:this         (flags == VOP_F_NORMAL)
 * NOTE: If "this" isn't a constant, it is the caller's responsibility to
 *       ensure that "this" doesn't have any aliases. Otherwise, aliases
 *       might inadvertently also receive the updated object. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vinplaceoptuple(struct Dee_function_generator *__restrict self,
                                       uint16_t operator_name, unsigned int flags);


INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopunpack(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);      /* seq -> [elems...] */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopconcat(struct Dee_function_generator *__restrict self);                          /* lhs, rhs -> result */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopextend(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);      /* seq, [elems...] -> seq */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_voptypeof(struct Dee_function_generator *__restrict self, bool ref);                /* ob -> type(ob) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopclassof(struct Dee_function_generator *__restrict self, bool ref);               /* ob -> ob.class */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopsuperof(struct Dee_function_generator *__restrict self);                         /* ob -> ob.super */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vopsuper(struct Dee_function_generator *__restrict self);                           /* ob, type -> ob as type */

/* Implement type casts. These functions should only be used for built-in, known types. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vopcast(struct Dee_function_generator *__restrict self, DeeTypeObject *newtype); /* obj -> newtype(obj) */
/* Like Dee_function_generator_vopcast(), but return 1 if the fallback (1-arg ctor-call) would be used. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vopcast_nofallback(struct Dee_function_generator *__restrict self, DeeTypeObject *newtype);

/* Helpers to perform certain operations. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vcall_DeeObject_Init(struct Dee_function_generator *__restrict self);          /* instance, type -> instance */
#define Dee_function_generator_vcall_DeeObject_Init_c(self, type) \
	(unlikely(Dee_function_generator_vpush_const(self, type)) ||  \
	 unlikely(Dee_function_generator_vcall_DeeObject_Init(self)))

/* Helpers for accessing C-level "struct type_member". NOTE: These don't do type assertions! */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_vpush_type_member(struct Dee_function_generator *__restrict self, DeeTypeObject *type, struct Dee_type_member const *__restrict desc, bool ref); /* this -> value */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_vpush_type_member(struct Dee_function_generator *__restrict self, struct Dee_type_member const *__restrict desc, bool ref);                     /* this -> value  (doesn't look at the doc string to determine typing) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vbound_type_member(struct Dee_function_generator *__restrict self, struct Dee_type_member const *__restrict desc);                               /* this -> bound */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vdel_type_member(struct Dee_function_generator *__restrict self, struct Dee_type_member const *__restrict desc);                                 /* this -> N/A */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpop_type_member(struct Dee_function_generator *__restrict self, struct Dee_type_member const *__restrict desc);                                 /* this, value -> N/A */

/* Helpers for accessing Module-level "struct Dee_module_symbol". */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vpush_module_symbol(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, struct Dee_module_symbol const *sym, bool ref); /* N/A -> value */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vbound_module_symbol(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, struct Dee_module_symbol const *sym);          /* N/A -> bound */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vdel_module_symbol(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, struct Dee_module_symbol const *sym);            /* N/A -> N/A */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vpop_module_symbol(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, struct Dee_module_symbol const *sym);            /* value -> N/A */

/* Helpers for accessing Class-level "struct Dee_class_attribute". NOTE: These don't do type assertions! */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vpush_instance_attr(struct Dee_function_generator *__restrict self, DeeTypeObject *type, struct Dee_class_attribute const *attr, bool ref);                /* this -> value */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vbound_instance_attr(struct Dee_function_generator *__restrict self, DeeTypeObject *type, struct Dee_class_attribute const *attr);                         /* this -> bound */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vdel_instance_attr(struct Dee_function_generator *__restrict self, DeeTypeObject *type, struct Dee_class_attribute const *attr);                           /* this -> N/A */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vpop_instance_attr(struct Dee_function_generator *__restrict self, DeeTypeObject *type, struct Dee_class_attribute const *attr);                           /* this, value -> N/A */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_vcall_instance_attrkw(struct Dee_function_generator *__restrict self, DeeTypeObject *type, struct Dee_class_attribute const *attr, Dee_vstackaddr_t argc); /* this, [args...], kw -> result */
#define Dee_function_generator_vcall_instance_attr(self, type, attr, argc) \
	(Dee_function_generator_vpush_addr(self, NULL) || Dee_function_generator_vcall_instance_attrkw(self, type, attr, argc))

INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_veqconstaddr(struct Dee_function_generator *__restrict self, void const *value); /* VTOP = VTOP == <value> */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_veqaddr(struct Dee_function_generator *__restrict self);                         /* PUSH(POP() == POP()); // Based on address */

/* >> if (THIRD == SECOND) // Based on address
 * >>     THIRD = FIRST;
 * >> POP();
 * >> POP(); */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vcoalesce(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vcoalesce_c(struct Dee_function_generator *__restrict self, void const *from, void const *to);

/* Force VTOP to become a direct object. Any memory locations that aliases it is also changed.
 * NOTE: This function is usually called automatically by other `Dee_function_generator_v*' functions. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdirect1(struct Dee_function_generator *__restrict self);

/* Same as (but requires that "n >= 1"):
 * >> Dee_function_generator_vlrot(self, n);
 * >> Dee_function_generator_vdirect1(self);
 * >> Dee_function_generator_vrrot(self, n); */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdirect_at(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);

/* Same as (though the order in which objects are made direct is undefined):
 * >> for (i = 0; i < n; ++i) {
 * >>     Dee_function_generator_vlrot(self, n);
 * >>     Dee_function_generator_vdirect1(self);
 * >> } */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdirect(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);

/* Make sure that "val" is direct. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vdirect_memval(struct Dee_function_generator *__restrict self,
                                      struct Dee_memval *val);

/* Clear the `MEMOBJ_F_ONEREF' flag for the top `n' v-stack elements,
 * as well as any other memory location that might be aliasing them. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vnotoneref(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vnotoneref_at(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t off);

/* Same as `Dee_function_generator_vnotoneref()', but only clear when the
 * types aren't known, or the type's `operator_name' lets references escape.
 * NOTE: You can pass `OPERATOR_SEQ_ENUMERATE' to see if `for (none: seq);' might let references escape. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vnotoneref_if_operator(struct Dee_function_generator *__restrict self, uint16_t operator_name, Dee_vstackaddr_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vnotoneref_if_operator_at(struct Dee_function_generator *__restrict self, uint16_t operator_name, Dee_vstackaddr_t off);

/* Set the `MEMOBJ_F_ONEREF' flag for VTOP. */
#define Dee_function_generator_voneref_noalias(self)                    \
	(Dee_ASSERT(Dee_memval_hasobj0(Dee_function_generator_vtop(self))), \
	 Dee_memobj_setoneref(Dee_memval_getobj0(Dee_function_generator_vtop(self))), 0)

/* Helper wrappers to do checked operations on local variables as usercode sees them. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_ulocal(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_ulid_t ulid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vbound_ulocal(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_ulid_t ulid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_ulocal(struct Dee_function_generator *__restrict self, Dee_ulid_t ulid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_ulocal(struct Dee_function_generator *__restrict self, Dee_ulid_t ulid);

/* Helper macros for operating on "extra" locals (s.a. `MEMSTATE_XLOCAL_*') */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_xlocal(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_lid_t xlid);
#define _Dee_function_generator_vpush_xlocal(self, instr, xlid)  Dee_function_generator_vpush_local(self, instr, (Dee_lid_t)(self)->fg_assembler->fa_localc + (xlid))
#define _Dee_function_generator_vbound_xlocal(self, instr, xlid) Dee_function_generator_vbound_local(self, instr, (Dee_lid_t)(self)->fg_assembler->fa_localc + (xlid))
#define Dee_function_generator_vpop_xlocal(self, xlid)           Dee_function_generator_vpop_local(self, (Dee_lid_t)(self)->fg_assembler->fa_localc + (xlid))
#define Dee_function_generator_vdel_xlocal(self, xlid)           Dee_function_generator_vdel_local(self, (Dee_lid_t)(self)->fg_assembler->fa_localc + (xlid))

/* Push the "this" argument of thiscall functions. */
#define Dee_function_generator_vpush_this(self, instr) _Dee_function_generator_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_A_THIS)
#define Dee_function_generator_vpush_args(self, instr) _Dee_function_generator_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_A_ARGS)
#define Dee_function_generator_vpush_kw(self, instr)   _Dee_function_generator_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_A_KW)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_this_function(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_argc(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_argv(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_usage(struct Dee_function_generator *__restrict self, Dee_host_regusage_t usage);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_except(struct Dee_function_generator *__restrict self);

/* Class/instance member access helpers.
 * @param: flags: Set of `DEE_FUNCTION_GENERATOR_CIMEMBER_F_*' */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_cmember(struct Dee_function_generator *__restrict self, uint16_t addr, unsigned int flags);  /* type -> value */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vbound_cmember(struct Dee_function_generator *__restrict self, uint16_t addr, unsigned int flags); /* type -> bound */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_imember(struct Dee_function_generator *__restrict self, uint16_t addr, unsigned int flags);  /* this, type -> value */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vbound_imember(struct Dee_function_generator *__restrict self, uint16_t addr, unsigned int flags); /* this, type -> bound */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_imember(struct Dee_function_generator *__restrict self, uint16_t addr, unsigned int flags);   /* this, type -> N/A */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_imember(struct Dee_function_generator *__restrict self, uint16_t addr, unsigned int flags);   /* this, type, value -> N/A */
#define DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL 0x0000 /* Normal flags */
#define DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF    0x0001 /* Always push a reference when reading a member (only for `Dee_function_generator_vpush_[ic]member') */
#define DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE   0x0002 /* Force "safe" access if it needs to happen at runtime (verify object type & addr being in-bounds). */

/* Generate code equivalent to `DeeObject_AssertTypeExact(VTOP, type)', but don't pop `VTOP' from the v-stack. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type_exact_c(struct Dee_function_generator *__restrict self,
                                            DeeTypeObject *__restrict type);
/* Generate code equivalent to `DeeObject_AssertType(VTOP, type)', but don't pop `VTOP' from the v-stack. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type_c(struct Dee_function_generator *__restrict self,
                                      DeeTypeObject *__restrict type);
#define Dee_function_generator_vassert_type_exact_if_safe_c(self, type) \
	((self)->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE     \
	 ? Dee_function_generator_vassert_type_exact_c(self, type)          \
	 : Dee_function_generator_vsettyp(self, type))
#define Dee_function_generator_vassert_type_if_safe_c(self, type)   \
	((self)->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE \
	 ? Dee_function_generator_vassert_type_c(self, type)            \
	 : 0)

INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vassert_type(struct Dee_function_generator *__restrict self);       /* obj, type -> N/A */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vassert_type_exact(struct Dee_function_generator *__restrict self); /* obj, type -> N/A */

/* Perform a conditional jump to `desc' based on `jump_if_true'
 * @param: instr: Pointer to start of deemon jmp-instruction (for bb-truncation, and error message)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vjcc(struct Dee_function_generator *__restrict self,
                            struct Dee_jump_descriptor *desc,
                            Dee_instruction_t const *instr,
                            bool jump_if_true);

/* Implement a ASM_FOREACH-style jump to `desc'
 * @param: instr:               Pointer to start of deemon jmp-instruction (for bb-truncation, and error message)
 * @param: always_pop_iterator: When true, the iterator is also popped during the jump to `desc'
 *                              This is needed to implement ASM_FOREACH when used with a prefix.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vforeach(struct Dee_function_generator *__restrict self,
                                struct Dee_jump_descriptor *desc,
                                bool always_pop_iterator);

/* >> TOP = *(TOP + ind_delta); */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vind(struct Dee_function_generator *__restrict self, ptrdiff_t ind_delta);

/* >> *(SECOND + ind_delta) = POP(); // NOTE: Ignores `mv_vmorph' in SECOND */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpopind(struct Dee_function_generator *__restrict self, ptrdiff_t ind_delta);

/* >> TOP = TOP + val_delta; // NOTE: Ignores `mv_vmorph' */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdelta(struct Dee_function_generator *__restrict self, ptrdiff_t val_delta);

/* >> temp = *(SECOND + ind_delta);
 * >> *(SECOND + ind_delta) = FIRST;
 * >> POP();
 * >> POP();
 * >> PUSH(temp, MEMOBJ_F_NOREF); */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vswapind(struct Dee_function_generator *__restrict self, ptrdiff_t ind_delta);

/* Ensure that the top-most `DeeObject' from the object-stack is a reference. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vref(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vref_noconst(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vref_noalias(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vref_noconst_noalias(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vref2(struct Dee_function_generator *__restrict self, Dee_vstackaddr_t dont_steal_from_vtop_n);

/* Ensure that `mobj' is holding a reference. If said location has aliases,
 * and isn't a constant, then also ensure that at least one of those aliases
 * also contains a second reference.
 * @param: dont_steal_from_vtop_n: Ignore the top n v-stack items when searching for aliases. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gref2(struct Dee_function_generator *__restrict self, struct Dee_memobj *mobj, Dee_vstackaddr_t dont_steal_from_vtop_n);

/* Force vtop into a register (ensuring it has type `MEMADR_TYPE_HREG' for all locations used by VTOP) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vreg(struct Dee_function_generator *__restrict self, Dee_host_register_t const *not_these);
/* Force vtop onto the stack (ensuring it has type `MEMADR_TYPE_HSTACKIND, Dee_memloc_hstackind_getvaloff = 0' for all locations used by VTOP) */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vflush(struct Dee_function_generator *__restrict self, bool require_valoff_0);

/* Generate code to push a global variable onto the virtual stack. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_mod_global(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, uint16_t gid, bool ref);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vbound_mod_global(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, uint16_t gid);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vdel_mod_global(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, uint16_t gid);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpop_mod_global(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid, bool ref);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vbound_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
#define Dee_function_generator_vpush_global(self, gid, ref) Dee_function_generator_vpush_mod_global(self, (self)->fg_assembler->fa_code->co_module, gid, ref)
#define Dee_function_generator_vbound_global(self, gid)     Dee_function_generator_vbound_mod_global(self, (self)->fg_assembler->fa_code->co_module, gid)
#define Dee_function_generator_vdel_global(self, gid)       Dee_function_generator_vdel_mod_global(self, (self)->fg_assembler->fa_code->co_module, gid)
#define Dee_function_generator_vpop_global(self, gid)       Dee_function_generator_vpop_mod_global(self, (self)->fg_assembler->fa_code->co_module, gid)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_static(struct Dee_function_generator *__restrict self, uint16_t sid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_static(struct Dee_function_generator *__restrict self, uint16_t sid);

INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vrwlock_read(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vrwlock_write(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vrwlock_endread(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vrwlock_endwrite(struct Dee_function_generator *__restrict self);

/* Check if `loc' differs from vtop, and if so: move vtop
 * *into* `loc', the assign the *exact* given `loc' to vtop. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vsetloc(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc const *loc);

/* Return and pass the top-most stack element as function result.
 * This function will clear the stack and unbind all local variables. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vret(struct Dee_function_generator *__restrict self);

/* Generate host text to invoke `api_function' with the top-most `argc' items from the stack.
 * @param: cc: One of `VCALL_CC_*', describing the calling-convention of `api_function'.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallapi_(struct Dee_function_generator *__restrict self,
                                 void const *api_function, unsigned int cc,
                                 Dee_vstackaddr_t argc);
#define Dee_function_generator_vcallapi(self, api_function, cc, argc) \
	Dee_function_generator_vcallapi_(self, (void const *)(api_function), cc, argc)
#define VCALL_CC_OBJECT             0 /* DREF DeeObject *(DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result   ## Error if NULL/zero (via `MEMVAL_VMORPH_NULLABLE'), also MEMOBJ_F_NOREF is clear */
#define VCALL_CC_INT                1 /* int             (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> N/A      ## Error if non-zero */
#define VCALL_CC_INTPTR             1 /* intptr_t        (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> N/A      ## Error if non-zero */
#define VCALL_CC_RAWINT             2 /* int             (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> UNCHECKED(result) */
#define VCALL_CC_RAWINTPTR          2 /* intptr_t        (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> UNCHECKED(result) */
#define VCALL_CC_RAWINT_KEEPARGS    3 /* int             (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> [args...], UNCHECKED(result) */
#define VCALL_CC_RAWINTPTR_KEEPARGS 3 /* intptr_t        (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> [args...], UNCHECKED(result) */
#define VCALL_CC_NEGINT             4 /* int             (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result   ## Error if negative */
#define VCALL_CC_NEGINTPTR          4 /* intptr_t        (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result   ## Error if negative */
#define VCALL_CC_M1INT              5 /* int             (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result   ## Error if -1 */
#define VCALL_CC_M1INTPTR           5 /* intptr_t        (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result   ## Error if -1 */
#define VCALL_CC_VOID               6 /* void            (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> N/A */
#define VCALL_CC_EXCEPT             7 /* ?               (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> N/A      ## Always an error */
#define VCALL_CC_BOOL               8 /* bool            (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result */
#define VCALL_CC_MORPH_INTPTR       9 /* intptr_t        (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result */
#define VCALL_CC_MORPH_UINTPTR     10 /* uintptr_t       (DCALL *api_function)(void *, [void *, [void *, [...]]]); [args...] -> result */


/* [args...], funcaddr -> ...
 * Same as `Dee_function_generator_vcallapi()', but after the normal argument list,
 * there is an additional item "funcaddr" that contains the (possibly) runtime-
 * evaluated address of the function that should be called. Also note that said
 * "funcaddr" location is *always* popped.
 * @param: cc: One of `VCALL_CC_*', describing the calling-convention of `api_function' 
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcalldynapi(struct Dee_function_generator *__restrict self,
                                   unsigned int cc, Dee_vstackaddr_t argc);


/* After a call to `Dee_function_generator_vcallapi()' with `VCALL_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALL_CC_OBJECT'
 * The difference to directly passing `VCALL_CC_OBJECT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckobj(struct Dee_function_generator *__restrict self);

/* After a call to `Dee_function_generator_vcallapi()' with `VCALL_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALL_CC_INT'
 * The difference to directly passing `VCALL_CC_INT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 * NOTE: This function pops one element from the V-stack.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckint(struct Dee_function_generator *__restrict self);

/* Branch to exception handling if `vtop' is equal to `except_val' */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckerr(struct Dee_function_generator *__restrict self,
                                 intptr_t except_val);

/* Generate a call to `DeeObject_MALLOC()' to allocate an uninitialized object that
 * provides for "alloc_size" bytes of memory. If possible, try to dispatch against
 * a slap allocator instead (just like the real DeeObject_MALLOC also does).
 * NOTE: The value pushed onto the V-stack...
 *       - ... already has its MEMOBJ_F_NOREF flag CLEAR!
 *       - ... has already been NULL-checked (i.e. already is a direct value)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcall_DeeObject_MALLOC(struct Dee_function_generator *__restrict self,
                                              size_t alloc_size, bool do_calloc);
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcall_DeeObject_Malloc(struct Dee_function_generator *__restrict self,
                                              size_t alloc_size, bool do_calloc);

/* Arrange the top `argc' stack-items linearly, such that they all appear somewhere in memory
 * (probably on the host-stack), in consecutive order (with `vtop' at the greatest address,
 * and STACK[SIZE-argc] appearing at the lowest address). Once that has been accomplished,
 * push a value onto the vstack that describes the base-address (that is a `DeeObject **'
 * pointing to `STACK[SIZE-argc]') of the linear vector.
 * @param: readonly: Special case to allow the `DeeObject **' vector being generated
 *                   as `DeeObject *const *'. This in turn makes it possible to not
 *                   have to construct argument vectors on-stack when all arguments
 *                   are (re-)compile-time constants.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vlinear(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t argc, bool readonly);


/* Pre-defined exception injectors. */
struct Dee_function_exceptinject_callvoidapi {
	struct Dee_function_exceptinject fei_cva_base; /* Underlying injector */
	void const                      *fei_cva_func; /* [1..1] API function to call (with `VCALL_CC_VOID' semantics) */
	Dee_vstackaddr_t                 fei_cva_argc; /* # of arguments taken by `fei_cva_func' */
};
#define Dee_function_generator_xinject_push_callvoidapi(self, ij, api_func, argc) \
	((ij)->fei_cva_base.fei_inject = &Dee_function_exceptinject_callvoidapi_f,    \
	 (ij)->fei_cva_func            = (void const *)(api_func),                    \
	 (ij)->fei_cva_argc            = (argc),                                      \
	 Dee_function_generator_xinject_push(self, &(ij)->fei_cva_base))
#define Dee_function_generator_xinject_pop_callvoidapi(self, ij) \
	Dee_function_generator_xinject_pop(self, &(ij)->fei_cva_base)

INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* `fei_inject' value for `struct Dee_function_exceptinject_callvoidapi' */
Dee_function_exceptinject_callvoidapi_f(struct Dee_function_generator *__restrict self,
                                        struct Dee_function_exceptinject *__restrict inject);


/* Clear the `MEMOBJ_F_ONEREF' flag from `mobj', as well
 * as any other memory location that might be aliasing it. */
INTDEF /*WUNUSED*/ NONNULL((1, 2)) int DCALL
Dee_function_generator_gnotoneref_impl(struct Dee_function_generator *__restrict self,
                                       struct Dee_memobj *mobj);
#define Dee_function_generator_gnotoneref(self, mobj) \
	(((mobj)->mo_flags & MEMOBJ_F_ONEREF) ? Dee_function_generator_gnotoneref_impl(self, mobj) : 0)


/* Generate a call to a C-function.
 * @param: locv: An argc+1-long vector locations (the first is for the
 *               function to call, the rest are pointer-sized arguments)
 * WARNING: This function is allowed to modify `argv' to keep track of internal temporaries.
 * NOTE: The given `api_function' is assumed to use the `DCALL' calling convention. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gcallapi(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *locv, size_t argc);
INTDEF WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gcallapi(struct Dee_function_generator *__restrict self,
                                 struct Dee_memloc *locv, size_t argc);


/* Object reference count incref/decref */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gincref_loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gdecref_loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gdecref_dokill_loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gdecref_nokill_loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxincref_loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxdecref_loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxdecref_nokill_loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc, Dee_refcnt_t n);

INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gincref_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gdecref_nokill_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gdecref_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gdecref_regx_dokill(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gxincref_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gxdecref_nokill_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gxdecref_regx(struct Dee_function_generator *__restrict self, Dee_host_register_t regno, ptrdiff_t reg_offset, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gincref_const(struct Dee_host_section *__restrict self, DeeObject *value, Dee_refcnt_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gdecref_const(struct Dee_host_section *__restrict self, DeeObject *value, Dee_refcnt_t n);
#ifdef HOSTASM_X86_64
#define _Dee_host_section_gincref_const_MAYFAIL /* `_Dee_host_section_gincref_const()' returns `1' if the constant is too large */
#define _Dee_host_section_gdecref_const_MAYFAIL /* `_Dee_host_section_gdecref_const()' returns `1' if the constant is too large */
#endif /* HOSTASM_X86_64 */

#define _Dee_function_generator_gincref_const(self, value, n) _Dee_host_section_gincref_const((self)->fg_sect, value, n)
#define _Dee_function_generator_gdecref_const(self, value, n) _Dee_host_section_gdecref_const((self)->fg_sect, value, n)
#ifdef _Dee_host_section_gincref_const_MAYFAIL
#define _Dee_function_generator_gincref_const_MAYFAIL /* `_Dee_function_generator_gincref_const()' returns `1' if the constant is too large */
#endif /* _Dee_host_section_gincref_const_MAYFAIL */
#ifdef _Dee_host_section_gdecref_const_MAYFAIL
#define _Dee_function_generator_gdecref_const_MAYFAIL /* `_Dee_function_generator_gdecref_const()' returns `1' if the constant is too large */
#endif /* _Dee_host_section_gdecref_const_MAYFAIL */

#define Dee_function_generator_gincref_const(self, value, n) _Dee_function_generator_gincref_const(self, value, n)
#define Dee_function_generator_gdecref_const(self, value, n) _Dee_function_generator_gdecref_const(self, value, n)
#ifdef _Dee_function_generator_gincref_const_MAYFAIL
#define Dee_function_generator_gincref_const_MAYFAIL /* `Dee_function_generator_gincref_const()' returns `1' if the constant is too large */
#endif /* _Dee_function_generator_gincref_const_MAYFAIL */
#ifdef _Dee_function_generator_gdecref_const_MAYFAIL
#define Dee_function_generator_gdecref_const_MAYFAIL /* `Dee_function_generator_gdecref_const()' returns `1' if the constant is too large */
#endif /* _Dee_function_generator_gdecref_const_MAYFAIL */

#define Dee_function_generator_gincref_regx(self, regno, reg_offset, n)         _Dee_function_generator_gincref_regx(self, regno, reg_offset, n)
#define Dee_function_generator_gdecref_nokill_regx(self, regno, reg_offset, n)  _Dee_function_generator_gdecref_nokill_regx(self, regno, reg_offset, n)
#define Dee_function_generator_gxincref_regx(self, regno, reg_offset, n)        _Dee_function_generator_gxincref_regx(self, regno, reg_offset, n)
#define Dee_function_generator_gxdecref_nokill_regx(self, regno, reg_offset, n) _Dee_function_generator_gxdecref_nokill_regx(self, regno, reg_offset, n)
#define Dee_function_generator_gdecref_regx(self, regno, reg_offset, n)         _Dee_function_generator_gdecref_regx(self, regno, reg_offset, n)
#define Dee_function_generator_gdecref_regx_dokill(self, regno, reg_offset)     _Dee_function_generator_gdecref_regx_dokill(self, regno, reg_offset)
#define Dee_function_generator_gxdecref_regx(self, regno, reg_offset, n)        _Dee_function_generator_gxdecref_regx(self, regno, reg_offset, n)

/* Change `loc' into the value of `<result> = *(<loc> + ind_delta)'
 * Note that unlike the `Dee_function_generator_gmov*' functions, this
 * one may use `MEMADR_TYPE_*IND' to defer the indirection until later. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gasind(struct Dee_function_generator *__restrict self,
                              /*in*/ struct Dee_memloc const *loc,
                              /*out*/ struct Dee_memloc *result,
                              ptrdiff_t ind_delta);

/* Force `loc' to become a register (`MEMADR_TYPE_HREG'). */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gasreg(struct Dee_function_generator *__restrict self,
                              /*in*/ struct Dee_memloc const *loc,
                              /*out*/ struct Dee_memloc *result,
                              Dee_host_register_t const *not_these);

/* Force `loc' to reside on the stack, giving it an address
 * (`MEMADR_TYPE_HSTACKIND, Dee_memloc_hstackind_getvaloff = 0').
 * @param: require_valoff_0: When false, forgo the exit requirement
 *                           of `Dee_memloc_hstackind_getvaloff = 0' */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gasflush(struct Dee_function_generator *__restrict self,
                                /*in*/ struct Dee_memloc const *loc,
                                /*out*/ struct Dee_memloc *result,
                                bool require_valoff_0);

/* Controls for operating with R/W-locks (as needed for accessing global/extern variables) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_grwlock_read_const(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_grwlock_write_const(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_grwlock_endread_const(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_grwlock_endwrite_const(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
#define Dee_function_generator_grwlock_read(self, loc)     _Dee_function_generator_grwlock_read(self, loc)
#define Dee_function_generator_grwlock_write(self, loc)    _Dee_function_generator_grwlock_write(self, loc)
#define Dee_function_generator_grwlock_endread(self, loc)  _Dee_function_generator_grwlock_endread(self, loc)
#define Dee_function_generator_grwlock_endwrite(self, loc) _Dee_function_generator_grwlock_endwrite(self, loc)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_read(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_write(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_endread(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_endwrite(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict loc);

INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_adjust(struct Dee_host_section *__restrict self, ptrdiff_t sp_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushreg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_ghstack_pushregind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushconst(struct Dee_host_section *__restrict self, void const *value);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushhstack_at_cfa_boundary_np(struct Dee_host_section *__restrict self); /* Pushes the address of `(self)->fg_state->ms_host_cfa_offset' (as it was before the push) */
#define HAVE__Dee_host_section_ghstack_pushhstack_at_cfa_boundary_np
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_pushhstackind(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset); /* `sp_offset' is as it would be *before* the push */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_ghstack_popreg(struct Dee_host_section *__restrict self, Dee_host_register_t dst_regno);
#define _Dee_function_generator_ghstack_adjust(self, sp_delta)         _Dee_host_section_ghstack_adjust((self)->fg_sect, sp_delta)
#define _Dee_function_generator_ghstack_pushreg(self, src_regno)       _Dee_host_section_ghstack_pushreg((self)->fg_sect, src_regno)
#define _Dee_function_generator_ghstack_pushconst(self, value)         _Dee_host_section_ghstack_pushconst((self)->fg_sect, value)
#define _Dee_function_generator_ghstack_pushhstackind(self, sp_offset) _Dee_host_section_ghstack_pushhstackind((self)->fg_sect, sp_offset) /* `sp_offset' is as it would be *before* the push */
#define _Dee_function_generator_ghstack_popreg(self, dst_regno)        _Dee_host_section_ghstack_popreg((self)->fg_sect, dst_regno)
#ifdef HAVE__Dee_host_section_ghstack_pushhstack_at_cfa_boundary_np
#define HAVE__Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np
#define _Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np(self) _Dee_host_section_ghstack_pushhstack_at_cfa_boundary_np((self)->fg_sect)
#endif /* HAVE__Dee_host_section_ghstack_pushhstack_at_cfa_boundary_np */


INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_reg2hstackind(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t sp_offset);                             /* *(SP + sp_offset) = dst_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_hstack2reg(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset, Dee_host_register_t dst_regno);                                /* dst_regno = (SP + sp_offset); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_hstackind2reg(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset, Dee_host_register_t dst_regno);                             /* dst_regno = *(SP + sp_offset); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_const2reg(struct Dee_host_section *__restrict self, void const *value, Dee_host_register_t dst_regno);                                   /* dst_regno = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmov_const2regind(struct Dee_function_generator *__restrict self, void const *value, Dee_host_register_t dst_regno, ptrdiff_t dst_delta);           /* *(dst_regno + dst_delta) = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_const2hstackind(struct Dee_host_section *__restrict self, void const *value, ptrdiff_t sp_offset);                                       /* *(SP + sp_offset) = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_const2constind(struct Dee_host_section *__restrict self, void const *value, void const **p_value);                                       /* *<p_value> = <value>; */
#define _Dee_host_section_gmov_reg2reg(self, src_regno, dst_regno) _Dee_host_section_gmov_regx2reg(self, src_regno, 0, dst_regno)                                                                     /* dst_regno = src_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_regx2reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno);   /* dst_regno = src_regno + src_delta; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmov_regind2reg(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno); /* dst_regno = *(src_regno + src_delta); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmov_reg2regind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, Dee_host_register_t dst_regno, ptrdiff_t dst_delta); /* *(dst_regno * dst_delta) = src_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_constind2reg(struct Dee_host_section *__restrict self, void const **p_value, Dee_host_register_t dst_regno);                             /* dst_regno = *<p_value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmov_reg2constind(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, void const **p_value);                             /* *<p_value> = src_regno; */
#ifdef HOSTASM_X86_64
#define _Dee_function_generator_gmov_const2regind_MAYFAIL /* `_Dee_function_generator_gmov_const2regind()' returns `1' if the constant is too large */
#define _Dee_host_section_gmov_const2hstackind_MAYFAIL    /* `_Dee_host_section_gmov_const2hstackind()' returns `1' if the constant is too large */
#define _Dee_host_section_gmov_const2constind_MAYFAIL     /* `_Dee_host_section_gmov_const2constind()' returns `1' if "value" is too large, and `2' if "p_value" is too large */
#define _Dee_host_section_gmov_constind2reg_MAYFAIL       /* `_Dee_host_section_gmov_constind2reg()' returns `1' if "value" is too large, and `2' if "p_value" is too large */
#define _Dee_host_section_gmov_reg2constind_MAYFAIL       /* `_Dee_host_section_gmov_reg2constind()' returns `1' if "value" is too large, and `2' if "p_value" is too large */
#endif /* HOSTASM_X86_64 */

#define _Dee_function_generator_gmov_reg2hstackind(self, src_regno, sp_offset)       _Dee_host_section_gmov_reg2hstackind((self)->fg_sect, src_regno, sp_offset)
#define _Dee_function_generator_gmov_hstack2reg(self, sp_offset, dst_regno)          _Dee_host_section_gmov_hstack2reg((self)->fg_sect, sp_offset, dst_regno)
#define _Dee_function_generator_gmov_hstackind2reg(self, sp_offset, dst_regno)       _Dee_host_section_gmov_hstackind2reg((self)->fg_sect, sp_offset, dst_regno)
#define _Dee_function_generator_gmov_const2reg(self, value, dst_regno)               _Dee_host_section_gmov_const2reg((self)->fg_sect, value, dst_regno)
#define _Dee_function_generator_gmov_const2hstackind(self, value, sp_offset)         _Dee_host_section_gmov_const2hstackind((self)->fg_sect, value, sp_offset)
#define _Dee_function_generator_gmov_const2constind(self, value, p_value)            _Dee_host_section_gmov_const2constind((self)->fg_sect, value, p_value)
#define _Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno)             _Dee_host_section_gmov_reg2reg((self)->fg_sect, src_regno, dst_regno)
#define _Dee_function_generator_gmov_regx2reg(self, src_regno, src_delta, dst_regno) _Dee_host_section_gmov_regx2reg((self)->fg_sect, src_regno, src_delta, dst_regno)
#define _Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno)          _Dee_host_section_gmov_constind2reg((self)->fg_sect, p_value, dst_regno)
#define _Dee_function_generator_gmov_reg2constind(self, src_regno, p_value)          _Dee_host_section_gmov_reg2constind((self)->fg_sect, src_regno, p_value)
#ifdef _Dee_host_section_gmov_const2hstackind_MAYFAIL
#define _Dee_function_generator_gmov_const2hstackind_MAYFAIL /* `_Dee_function_generator_gmov_const2hstackind()' returns `1' if the constant is too large */
#endif /* _Dee_host_section_gmov_const2hstackind_MAYFAIL */
#ifdef _Dee_host_section_gmov_const2constind_MAYFAIL
#define _Dee_function_generator_gmov_const2constind_MAYFAIL /* `_Dee_function_generator_gmov_const2constind()' returns `1' if "value" is too large, and `2' if "p_value" is too large */
#endif /* _Dee_host_section_gmov_const2constind_MAYFAIL */
#ifdef _Dee_host_section_gmov_constind2reg_MAYFAIL
#define _Dee_function_generator_gmov_constind2reg_MAYFAIL /* `_Dee_function_generator_gmov_constind2reg()' returns `1' if "value" is too large, and `2' if "p_value" is too large */
#endif /* _Dee_host_section_gmov_constind2reg_MAYFAIL */
#ifdef _Dee_host_section_gmov_reg2constind_MAYFAIL
#define _Dee_function_generator_gmov_reg2constind_MAYFAIL /* `_Dee_host_section_gmov_reg2constind()' returns `1' if "value" is too large, and `2' if "p_value" is too large */
#endif /* _Dee_host_section_gmov_reg2constind_MAYFAIL */

/* Allocate/deallocate memory from the host stack.
 * If stack memory gets allocated, zero-initialize it. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_adjust(struct Dee_function_generator *__restrict self, ptrdiff_t cfa_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_pushreg(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_pushregind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_pushconst(struct Dee_function_generator *__restrict self, void const *value);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_pushhstackind(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset);
#ifdef HAVE__Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np
#define HAVE_Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np(struct Dee_function_generator *__restrict self);
#endif /* HAVE__Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_popreg(struct Dee_function_generator *__restrict self, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_reg2hstackind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, Dee_cfa_t cfa_offset);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_hstack2reg(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_hstackind2reg(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_const2reg(struct Dee_function_generator *__restrict self, void const *value, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_const2regind(struct Dee_function_generator *__restrict self, void const *value, Dee_host_register_t dst_regno, ptrdiff_t dst_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_const2hstackind(struct Dee_function_generator *__restrict self, void const *value, Dee_cfa_t cfa_offset);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_const2constind(struct Dee_function_generator *__restrict self, void const *value, void const **p_value);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_reg2reg(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_regx2reg(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_regind2reg(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_reg2regind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, Dee_host_register_t dst_regno, ptrdiff_t dst_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_constind2reg(struct Dee_function_generator *__restrict self, void const **p_value, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_reg2constind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, void const **p_value);


/* Arithmetic helpers. */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gadd_regreg2reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno1, Dee_host_register_t src_regno2, Dee_host_register_t dst_regno); /* dst_regno = src_regno1 + src_regno2; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gumul_regconst2reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, uintptr_t n, Dee_host_register_t dst_regno);                  /* dst_regno = src_regno * n; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gadd_const2hstackind(struct Dee_host_section *__restrict self, void const *value, ptrdiff_t sp_offset);                                                   /* *(SP + sp_offset) = *(SP + sp_offset) + <value>; */
#define HAVE__Dee_host_section_gadd_const2hstackind
#ifdef HOSTASM_X86_64
#define _Dee_host_section_gadd_const2hstackind_MAYFAIL /* `_Dee_host_section_gadd_const2hstackind()' returns `1' if the constant is too large */
#endif /* HOSTASM_X86_64 */

INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_const2loc(struct Dee_function_generator *__restrict self, void const *value, struct Dee_memloc const *__restrict dst_loc);                                                  /* <dst_loc> = value; */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL Dee_function_generator_gmov_regx2loc(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, struct Dee_memloc const *__restrict dst_loc);                  /* <dst_loc> = src_regno + src_delta; */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_hstack2loc(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset, struct Dee_memloc const *__restrict dst_loc);                                              /* <dst_loc> = (SP ... cfa_offset); */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_loc2regx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, Dee_host_register_t dst_regno, ptrdiff_t dst_delta);                  /* dst_regno = <src_loc> - dst_delta; */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL Dee_function_generator_gmov_loc2regy(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, Dee_host_register_t dst_regno, ptrdiff_t *__restrict p_dst_delta); /* dst_regno = <src_loc> - *p_dst_delta; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_locind2reg(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, ptrdiff_t src_delta, Dee_host_register_t dst_regno);                /* dst_regno = *(<src_loc> + src_delta); */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_reg2locind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, struct Dee_memloc const *__restrict dst_loc, ptrdiff_t dst_delta);                /* *(<dst_loc> + dst_delta) = src_regno; */
#define Dee_function_generator_gmov_reg2loc(self, src_regno, dst_loc) Dee_function_generator_gmov_regx2loc(self, src_regno, 0, dst_loc)
#define Dee_function_generator_gmov_loc2reg(self, src_loc, dst_regno) Dee_function_generator_gmov_loc2regx(self, src_loc, dst_regno, 0)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_ghstack_pushlocx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, ptrdiff_t dst_delta);                              /* PUSH(<src_loc> - dst_delta); */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_loc2hstackindx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, Dee_cfa_t dst_cfa_offset, ptrdiff_t dst_delta); /* *<SP@dst_cfa_offset> = <src_loc> - dst_delta; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_loc2constind(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, void const **p_value, ptrdiff_t dst_delta);       /* *<p_value> = <src_loc> - dst_delta; */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_const2locind(struct Dee_function_generator *__restrict self, void const *value, struct Dee_memloc const *__restrict dst_loc, ptrdiff_t dst_delta);          /* *(<dst_loc> + dst_delta) = value; */
#define Dee_function_generator_ghstack_pushloc(self, src_loc)                    Dee_function_generator_ghstack_pushlocx(self, src_loc, 0)
#define Dee_function_generator_gmov_loc2hstackind(self, src_loc, dst_cfa_offset) Dee_function_generator_gmov_loc2hstackindx(self, src_loc, dst_cfa_offset, 0)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_gmov_loc2loc(struct Dee_function_generator *__restrict self, struct Dee_memloc const *src_loc, struct Dee_memloc const *dst_loc);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_gmov_loc2locind(struct Dee_function_generator *__restrict self, struct Dee_memloc const *src_loc, struct Dee_memloc const *dst_loc, ptrdiff_t ind_delta);

/* Generate code to return `loc'. No extra code to decref stack/locals is generated. If you
 * want that extra code to be generated, you need to use `Dee_function_generator_vret()'. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gret(struct Dee_function_generator *__restrict self, /*inherit_ref*/ struct Dee_memloc const *__restrict loc);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gret(struct Dee_function_generator *__restrict self);

/* Helpers for transforming locations into deemon boolean objects. */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmorph_regx2reg01(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t dst_regno);                                                           /* dst_regno = (src_regno + src_delta) <CMP> 0 ? 1 : 0; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmorph_regxCreg2reg01(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno);                                    /* dst_regno = (src_regno + src_delta) <CMP> rhs_regno ? 1 : 0; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmorph_regind2reg01(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t dst_regno);                                    /* dst_regno = (*(src_regno + ind_delta) + val_delta) <CMP> 0 ? 1 : 0; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmorph_regindCreg2reg01(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno); /* dst_regno = (*(src_regno + ind_delta) + val_delta) <CMP> rhs_regno ? 1 : 0; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmorph_hstackind2reg01(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t dst_regno);                                                                            /* dst_regno = (*(SP + sp_offset) + val_delta) <CMP> 0 ? 1 : 0; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmorph_hstackindCreg2reg01(struct Dee_function_generator *__restrict self, ptrdiff_t sp_offset, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno);                             /* dst_regno = (*(SP + sp_offset) + val_delta) <CMP> rhs_regno ? 1 : 0; */
#define _Dee_function_generator_gmorph_regxCreg2reg01(self, src_regno, src_delta, cmp, rhs_regno, dst_regno) _Dee_host_section_gmorph_regxCreg2reg01((self)->fg_sect, src_regno, src_delta, cmp, rhs_regno, dst_regno)
#define _Dee_function_generator_gmorph_hstackind2reg01(self, sp_offset, val_delta, cmp, dst_regno)           _Dee_host_section_gmorph_hstackind2reg01((self)->fg_sect, sp_offset, val_delta, cmp, dst_regno)
#ifdef HOSTASM_X86_64
#define _Dee_function_generator_gmorph_regind2reg01_MAYFAIL /* `_Dee_function_generator_gmorph_regind2reg01()' returns `1' if "val_delta" is too large */
#define _Dee_host_section_gmorph_hstackind2reg01_MAYFAIL    /* `_Dee_host_section_gmorph_hstackind2reg01()' returns `1' if "val_delta" is too large */
#endif /* HOSTASM_X86_64 */
#ifdef _Dee_host_section_gmorph_hstackind2reg01_MAYFAIL
#define _Dee_function_generator_gmorph_hstackind2reg01_MAYFAIL /* `_Dee_function_generator_gmorph_hstackind2reg01()' returns `1' if "val_delta" is too large */
#endif /* _Dee_host_section_gmorph_hstackind2reg01_MAYFAIL */
#if defined(HOSTASM_X86) && !defined(HOSTASM_X86_64) && !defined(CONFIG_TRACE_REFCHANGES)
#define HAVE__Dee_host_section_gmorph_reg012regbool
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gmorph_reg012regbool(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno); /* dst_regno = &Dee_FalseTrue[src_regno + src_delta]; */
#endif /* HOSTASM_X86 && !HOSTASM_X86_64 && !CONFIG_TRACE_REFCHANGES */

INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmorph_regx2reg01(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmorph_regind2reg01(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmorph_hstackind2reg01(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmorph_regxCreg2reg01(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmorph_regindCreg2reg01(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmorph_hstackindCreg2reg01(struct Dee_function_generator *__restrict self, Dee_cfa_t cfa_offset, ptrdiff_t val_delta, unsigned int cmp, Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmorph_loc2reg01(struct Dee_function_generator *__restrict self, struct Dee_memloc const *src_loc, ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t dst_regno);                                          /* dst_regno = (src_loc + src_delta) <CMP> 0 ? 1 : 0; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmorph_locCreg2reg01(struct Dee_function_generator *__restrict self, struct Dee_memloc const *src_loc, ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t rhs_regno, Dee_host_register_t dst_regno);       /* dst_regno = (src_loc + src_delta) <CMP> rhs_regno ? 1 : 0; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmorph_locCloc2reg01(struct Dee_function_generator *__restrict self, struct Dee_memloc const *src_loc, ptrdiff_t src_delta, unsigned int cmp, struct Dee_memloc const *rhs_loc, Dee_host_register_t dst_regno);    /* dst_regno = (src_loc + src_delta) <CMP> rhs_loc ? 1 : 0; */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL Dee_function_generator_gmorph_loc2regbooly(struct Dee_function_generator *__restrict self, struct Dee_memloc const *src_loc, ptrdiff_t src_delta, unsigned int cmp, Dee_host_register_t dst_regno, ptrdiff_t *__restrict p_dst_delta); /* dst_regno = &Dee_FalseTrue[(src_loc + src_delta) <CMP> 0 ? 1 : 0] - *p_dst_delta; */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL Dee_function_generator_gmorph_loc012regbooly(struct Dee_function_generator *__restrict self, struct Dee_memloc const *src_loc, ptrdiff_t src_delta, Dee_host_register_t dst_regno, ptrdiff_t *__restrict p_dst_delta);                 /* dst_regno = &Dee_FalseTrue[src_loc + src_delta] - *p_dst_delta; */

/* dst_regno = &Dee_FalseTrue[src_regno + src_delta] - *p_dst_delta; */
#ifdef HAVE__Dee_host_section_gmorph_reg012regbool
#define Dee_function_generator_gmorph_reg012regbooly(self, src_regno, src_delta, dst_regno, p_dst_delta) \
	(*(p_dst_delta) = 0, _Dee_host_section_gmorph_reg012regbool((self)->fg_sect, src_regno, src_delta, dst_regno))
#else /* HAVE__Dee_host_section_gmorph_reg012regbool */
INTDEF WUNUSED NONNULL((1, 5)) int DCALL
Dee_function_generator_gmorph_reg012regbooly(struct Dee_function_generator *__restrict self,
                                             Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                             Dee_host_register_t dst_regno, ptrdiff_t *__restrict p_dst_delta);
#endif /* !HAVE__Dee_host_section_gmorph_reg012regbool */

#define GMORPHBOOL_CC_EQ 0 /* Compare: "==" */
#define GMORPHBOOL_CC_NE 1 /* Compare: "!=" */
#define GMORPHBOOL_CC_LO 2 /* Compare: "<" (signed) */
#define GMORPHBOOL_CC_GR 3 /* Compare: ">" (signed) */


INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_host_section_gjmp(struct Dee_host_section *__restrict self, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL _Dee_host_section_gjz_reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL _Dee_function_generator_gjz_regind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t ind_delta, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL _Dee_host_section_gjz_hstackind(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL _Dee_host_section_gjnz_reg(struct Dee_host_section *__restrict self, Dee_host_register_t src_regno, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL _Dee_function_generator_gjnz_regind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, ptrdiff_t ind_delta, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL _Dee_host_section_gjnz_hstackind(struct Dee_host_section *__restrict self, ptrdiff_t sp_offset, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gjcc_regCreg(struct Dee_host_section *__restrict self, Dee_host_register_t lhs_regno, Dee_host_register_t rhs_regno, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gjcc_regCconst(struct Dee_host_section *__restrict self, Dee_host_register_t lhs_regno, void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gjcc_regindCreg(struct Dee_function_generator *__restrict self, Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta, Dee_host_register_t rhs_regno, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gjcc_regindCconst(struct Dee_function_generator *__restrict self, Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta, void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gjcc_hstackindCreg(struct Dee_host_section *__restrict self, ptrdiff_t lhs_sp_offset, Dee_host_register_t rhs_regno, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_host_section_gjcc_hstackindCconst(struct Dee_host_section *__restrict self, ptrdiff_t lhs_sp_offset, void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
#ifdef HOSTASM_X86_64
#define _Dee_host_section_gjcc_regCconst_MAYFAIL          /* `_Dee_host_section_gjcc_regCconst()' returns `1' if "rhs_value" is too large */
#define _Dee_function_generator_gjcc_regindCconst_MAYFAIL /* `_Dee_function_generator_gjcc_regindCconst()' returns `1' if "rhs_value" is too large */
#define _Dee_host_section_gjcc_hstackindCconst_MAYFAIL    /* `_Dee_host_section_gjcc_hstackindCconst()' returns `1' if "rhs_value" is too large */
#endif /* HOSTASM_X86_64 */

#define _Dee_function_generator_gjmp(self, dst)                                                                          _Dee_host_section_gjmp((self)->fg_sect, dst)
#define _Dee_function_generator_gjz_reg(self, src_regno, dst)                                                            _Dee_host_section_gjz_reg((self)->fg_sect, src_regno, dst)
#define _Dee_function_generator_gjz_hstackind(self, sp_offset, dst)                                                      _Dee_host_section_gjz_hstackind((self)->fg_sect, sp_offset, dst)
#define _Dee_function_generator_gjnz_reg(self, src_regno, dst)                                                           _Dee_host_section_gjnz_reg((self)->fg_sect, src_regno, dst)
#define _Dee_function_generator_gjnz_hstackind(self, sp_offset, dst)                                                     _Dee_host_section_gjnz_hstackind((self)->fg_sect, sp_offset, dst)
#define _Dee_function_generator_gjcc_regCreg(self, lhs_regno, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)             _Dee_host_section_gjcc_regCreg((self)->fg_sect, lhs_regno, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)
#define _Dee_function_generator_gjcc_regCconst(self, lhs_regno, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr)           _Dee_host_section_gjcc_regCconst((self)->fg_sect, lhs_regno, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr)
#define _Dee_function_generator_gjcc_hstackindCreg(self, lhs_sp_offset, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)   _Dee_host_section_gjcc_hstackindCreg((self)->fg_sect, lhs_sp_offset, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)
#define _Dee_function_generator_gjcc_hstackindCconst(self, lhs_sp_offset, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr) _Dee_host_section_gjcc_hstackindCconst((self)->fg_sect, lhs_sp_offset, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr)
#ifdef _Dee_host_section_gjcc_regCconst_MAYFAIL
#define _Dee_function_generator_gjcc_regCconst_MAYFAIL /* `_Dee_function_generator_gjcc_regCconst()' returns `1' if "rhs_value" is too large */
#endif /* _Dee_host_section_gjcc_regCconst_MAYFAIL */
#ifdef _Dee_host_section_gjcc_hstackindCconst_MAYFAIL
#define _Dee_function_generator_gjcc_hstackindCconst_MAYFAIL /* `_Dee_function_generator_gjcc_hstackindCconst()' returns `1' if "rhs_value" is too large */
#endif /* _Dee_host_section_gjcc_hstackindCconst_MAYFAIL */

#define Dee_function_generator_gjmp(self, dst)                                                                                _Dee_function_generator_gjmp(self, dst)
#define Dee_function_generator_gjz_reg(self, src_regno, dst)                                                                  _Dee_function_generator_gjz_reg(self, src_regno, dst)
#define Dee_function_generator_gjz_regind(self, src_regno, ind_delta, dst)                                                    _Dee_function_generator_gjz_regind(self, src_regno, ind_delta, dst)
#define Dee_function_generator_gjz_hstackind(self, cfa_offset, dst)                                                           _Dee_function_generator_gjz_hstackind(self, Dee_memstate_hstack_cfa2sp((self)->fg_state, cfa_offset), dst)
#define Dee_function_generator_gjnz_reg(self, src_regno, dst)                                                                 _Dee_function_generator_gjnz_reg(self, src_regno, dst)
#define Dee_function_generator_gjnz_regind(self, src_regno, ind_delta, dst)                                                   _Dee_function_generator_gjnz_regind(self, src_regno, ind_delta, dst)
#define Dee_function_generator_gjnz_hstackind(self, cfa_offset, dst)                                                          _Dee_function_generator_gjnz_hstackind(self, Dee_memstate_hstack_cfa2sp((self)->fg_state, cfa_offset), dst)
#define Dee_function_generator_gjcc_regCreg(self, lhs_regno, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)                   _Dee_function_generator_gjcc_regCreg(self, lhs_regno, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)
#define Dee_function_generator_gjcc_regindCreg(self, lhs_regno, lhs_ind_delta, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr) _Dee_function_generator_gjcc_regindCreg(self, lhs_regno, lhs_ind_delta, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)
#define Dee_function_generator_gjcc_hstackindCreg(self, lhs_cfa_offset, rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)        _Dee_function_generator_gjcc_hstackindCreg(self, Dee_memstate_hstack_cfa2sp((self)->fg_state, lhs_cfa_offset), rhs_regno, signed_cmp, dst_lo, dst_eq, dst_gr)
#ifndef _Dee_function_generator_gjcc_regindCconst_MAYFAIL
#define Dee_function_generator_gjcc_regindCconst(self, lhs_regno, lhs_ind_delta, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr) _Dee_function_generator_gjcc_regindCconst(self, lhs_regno, lhs_ind_delta, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr)
#else /* _Dee_function_generator_gjcc_regindCconst_MAYFAIL */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gjcc_regindCconst(struct Dee_function_generator *__restrict self, Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta, void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
#endif /* !_Dee_function_generator_gjcc_regindCconst_MAYFAIL */
#ifndef _Dee_function_generator_gjcc_regCconst_MAYFAIL
#define Dee_function_generator_gjcc_regCconst(self, lhs_regno, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr) _Dee_function_generator_gjcc_regCconst(self, lhs_regno, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr)
#else /* _Dee_function_generator_gjcc_regCconst_MAYFAIL */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gjcc_regCconst(struct Dee_function_generator *__restrict self, Dee_host_register_t lhs_regno, void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
#endif /* !_Dee_function_generator_gjcc_regCconst_MAYFAIL */
#ifndef _Dee_function_generator_gjcc_hstackindCconst_MAYFAIL
#define Dee_function_generator_gjcc_hstackindCconst(self, lhs_cfa_offset, rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr) _Dee_function_generator_gjcc_hstackindCconst(self, Dee_memstate_hstack_cfa2sp((self)->fg_state, lhs_cfa_offset), rhs_value, signed_cmp, dst_lo, dst_eq, dst_gr)
#else /* _Dee_function_generator_gjcc_hstackindCconst_MAYFAIL */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gjcc_hstackindCconst(struct Dee_function_generator *__restrict self, Dee_cfa_t lhs_cfa_offset, void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
#endif /* !_Dee_function_generator_gjcc_hstackindCconst_MAYFAIL */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gjcc_locCregx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *lhs, Dee_host_register_t rhs_regno, ptrdiff_t rhs_val_offset, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gjcc_locCconst(struct Dee_function_generator *__restrict self, struct Dee_memloc const *lhs, void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr);





/* Generate jumps. */
#define Dee_function_generator_gjmp(self, dst) _Dee_function_generator_gjmp(self, dst)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_gjz(struct Dee_function_generator *__restrict self, struct Dee_memloc const *test_loc, struct Dee_host_symbol *__restrict dst);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL Dee_function_generator_gjnz(struct Dee_function_generator *__restrict self, struct Dee_memloc const *test_loc, struct Dee_host_symbol *__restrict dst);

/* Emit conditional jump(s) based on `<lhs> <=> <rhs>'
 * NOTE: This function may clobber `lhs' and `rhs', and may flush/shift local/stack locations. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gjcc(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc const *lhs, struct Dee_memloc const *rhs, bool signed_cmp,
                            struct Dee_host_symbol *dst_lo,  /* Jump here if `<lhs> < <rhs>' */
                            struct Dee_host_symbol *dst_eq,  /* Jump here if `<lhs> == <rhs>' */
                            struct Dee_host_symbol *dst_gr); /* Jump here if `<lhs> > <rhs>' */



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
 * NOTE: Usage-registers are cleared by arch-specific code (e.g. `Dee_function_generator_gcallapi()')
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references.
 * @param: only_if_reference: Only flush locations that contain references. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushregs(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                  bool only_if_reference);

/* Flush memory locations that make use of `regno' onto the hstack. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushreg(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                 bool only_if_reference, Dee_host_register_t regno);

/* Generate code to assert that location `loc' is non-NULL:
 * >> Dee_function_generator_gassert_bound(self, loc, instr, NULL, lid, NULL, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, instr, mod, gid, NULL, NULL);
 * @param: opt_endread_before_throw: When non-NULL, emit `Dee_function_generator_grwlock_endread()'
 *                                   before the `Dee_function_generator_gthrow_*_unbound' code. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gassert_bound(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc const *loc, Dee_instruction_t const *instr,
                                     struct Dee_module_object *mod, uint16_t id,
                                     Dee_atomic_rwlock_t *opt_endread_before_throw,
                                     Dee_atomic_rwlock_t *opt_endwrite_before_throw);
#define Dee_function_generator_gassert_local_bound(self, instr, ulid) \
	((ulid) >= (self)->fg_assembler->fa_localc                        \
	 ? err_illegal_ulid(ulid)                                         \
	 : !Dee_memval_isdirect(&(self)->fg_state->ms_localv[ulid])       \
	   ? 0                                                            \
	   : Dee_function_generator_gassert_local_bound_ex(self, Dee_memval_direct_getloc(&(self)->fg_state->ms_localv[ulid]), instr, ulid))
#define Dee_function_generator_gassert_local_bound_ex(self, loc, instr, ulid) \
	Dee_function_generator_gassert_bound(self, loc, instr, NULL, ulid, NULL, NULL)
#define Dee_function_generator_gassert_global_bound(self, loc, mod, gid) \
	Dee_function_generator_gassert_bound(self, loc, NULL, mod, gid, NULL, NULL)

/* Generate code to throw an error indicating that local variable `lid'
 * is unbound. This includes any necessary jump for the purpose of entering
 * an exception handler. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_arg_unbound(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_aid_t aid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_local_unbound(struct Dee_function_generator *__restrict self, Dee_instruction_t const *instr, Dee_ulid_t lid);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gthrow_global_unbound(struct Dee_function_generator *__restrict self, struct Dee_module_object *mod, uint16_t gid);

/* Generate checks to enter exception handling mode. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gjz_except(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gjnz_except(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gjcmp_except(struct Dee_function_generator *__restrict self, struct Dee_memloc const *loc, intptr_t threshold, unsigned int flags);
#define Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO       0x01
#define Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ       0x02
#define Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR       0x04
#define Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED 0x08
#define Dee_function_generator_gjeq_except(self, loc, except_val)     Dee_function_generator_gjcmp_except(self, loc, except_val, Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ)
#define Dee_function_generator_gjne_except(self, loc, not_except_val) Dee_function_generator_gjcmp_except(self, loc, not_except_val, Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO | Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gjmp_except(struct Dee_function_generator *__restrict self);

/* Generate code in `self->fg_sect' to morph `self->fg_state' into `new_state' */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vmorph_no_constrain_equivalences(struct Dee_function_generator *__restrict self,
                                                        struct Dee_memstate const *new_state);

/* Same as `Dee_function_generator_vmorph_no_constrain_equivalences()', but also
 * generate code to constrain the equivalences from `new_state' into "self". */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vmorph(struct Dee_function_generator *__restrict self,
                              struct Dee_memstate const *new_state);

/* Generate code to morph the reference state from "oldinfo" to "newinfo" */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_xmorph(struct Dee_function_generator *__restrict self,
                              struct Dee_except_exitinfo_id const *__restrict oldinfo,
                              struct Dee_except_exitinfo_id *__restrict newinfo);


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
/* Dedicated type method/getset optimizations                           */
/************************************************************************/

/* this, [args...] -> result             // For call/operator generators
 * this, [args...] -> N/A                // For operator generators where the operator has no return value
 * this, [args...] -> this, [this]       // For inplace operator generators
 * @return: 0 : Optimization applied
 * @return: 1 : Optimization not possible
 * @return: -1: Error */
typedef WUNUSED_T NONNULL_T((1)) int
(DCALL *Dee_ccall_optigen_t)(struct Dee_function_generator *__restrict self,
                             Dee_vstackaddr_t argc);
struct Dee_ccall_optimization {
	union {
		char const     *n_attr;    /* [1..1] Expected function name (attribute name). */
		uintptr_t       n_opname;  /* Expected operator ID. */
	}                   tcco_name; /* Name of the attribute/operator being optimized. */
	Dee_ccall_optigen_t tcco_func; /* [1..1] Optimized generator. */
	Dee_vstackaddr_t    tcco_argc; /* Expected argument count, or `Dee_CCALL_ARGC_ANY'. */
#define Dee_CCALL_ARGC_ANY    ((Dee_vstackaddr_t)-1)
#define Dee_CCALL_ARGC_GETTER ((Dee_vstackaddr_t)-2)
#define Dee_CCALL_ARGC_DELETE ((Dee_vstackaddr_t)-3)
#define Dee_CCALL_ARGC_SETTER ((Dee_vstackaddr_t)-4)
#define Dee_CCALL_ARGC_BOUND  ((Dee_vstackaddr_t)-5)
#define Dee_CCALL_ARGC_MAX    ((Dee_vstackaddr_t)-6)
};

/* Try to find a dedicated optimization for `INSTANCEOF(<type>).<name>(argc...)' */
INTDEF WUNUSED NONNULL((1, 2)) struct Dee_ccall_optimization const *DCALL
Dee_ccall_find_attr_optimization(DeeTypeObject *__restrict type,
                                 char const *name, Dee_vstackaddr_t argc);

/* Try to find a dedicated optimization for `INSTANCEOF(<type>).operator <operator_name> (argc...)'
 * NOTE: Optimizations returned type this one may or may not push a result onto the stack,
 *       depending on the operator in question (`operator_name')! Because of this, if the
 *       operator is generic, the caller needs to check how the vstack depth is altered.
 *       For inplace operators, the same applies, but the "this" argument always remains
 *       on-stack as well! */
INTDEF WUNUSED NONNULL((1)) struct Dee_ccall_optimization const *DCALL
Dee_ccall_find_operator_optimization(DeeTypeObject *__restrict type,
                                     uint16_t operator_name, Dee_vstackaddr_t argc);




/************************************************************************/
/* Type traits                                                          */
/************************************************************************/

/* Returns `true' if operator `operator_name' of `self' can be invoked without
 * unintended side-effects, which includes the possibility of other threads
 * accessing any an instance of the type at the same time, which must *NOT*
 * affect the result of the operator being invoked (iow: `List.operator +' is
 * not constexpr). */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_IsOperatorConstexpr(DeeTypeObject const *__restrict self,
                            uint16_t operator_name);

/* Check if operator `operator_name' of `self' doesn't let references to the "this" argument escape.
 * NOTE: You can pass `OPERATOR_SEQ_ENUMERATE' to see if `for (none: seq);' might let references escape. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_IsOperatorNoRefEscape(DeeTypeObject const *__restrict self,
                              uint16_t operator_name);
#define OPERATOR_SEQ_ENUMERATE OPERATOR_VISIT /* Special operator to check if references leak if the object is enumerated
                                               * via OPERATOR_ITER (though the created iterator is destroyed at the end). */

/* Check if C-method attached to objects are constant expressions. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_IsGetMethodConstexpr(DeeTypeObject const *__restrict self,
                             Dee_getmethod_t getter);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_IsBoundMethodConstexpr(DeeTypeObject const *__restrict self,
                               Dee_boundmethod_t bound);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2, 3)) bool DCALL /* Also usable for Dee_kwobjmethod_t */
DeeType_IsObjMethodConstexpr(DeeTypeObject *decl_type, Dee_objmethod_t method,
                             DeeObject *thisarg, size_t argc,
                             DeeObject *const *argv, DeeObject *kw);
INTDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL /* Also usable for Dee_kwcmethod_t */
DeeCMethod_IsConstExpr(Dee_cmethod_t method, size_t argc,
                       DeeObject *const *argv, DeeObject *kw);





/************************************************************************/
/* C-Callable host function output data structure                       */
/************************************************************************/

struct Dee_hostfunc {
	struct Dee_rawhostfunc hf_raw;  /* Raw function information */
	DREF DeeObject       **hf_refs; /* [1..1][const][0..n][const][owned] Vector of extra objects referenced by host assembly. */
	/* TODO: Save information about where/how to call the function whilst skipping
	 *       its prolog. That way, when hostasm code calls another deemon function
	 *       that has already been re-compiled into hostasm, argument/keyword checks
	 *       can potentially be performed at (re-)compile-time, rather than having
	 *       to pack/unpack arguments and go through `DeeObject_Call()' */
	/* TODO: Save debug information to encode locations of stack/locals */
};

INTDEF NONNULL((1)) void DCALL
Dee_hostfunc_fini(struct Dee_hostfunc *__restrict self);





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

/* Step #1.1 [optional; disabled by: `DEE_FUNCTION_ASSEMBLER_F_NOEARLYDEL']
 * Figure out all the instructions that read from a local the last time before
 * the function ends, or the variable gets written to again. By using this info,
 * `Dee_function_generator_geninstr()' emits extra instrumentation in order to
 * delete local variables earlier than usual, which in turn significantly lowers
 * the overhead associated with keeping objects alive longer than strictly
 * necessary. When this step is skipped, local simply aren't deleted early.
 * - self->fa_blockv[*]->bb_locuse
 * - self->fa_blockv[*]->bb_locreadv
 * - self->fa_blockv[*]->bb_locreadc
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_loadlocuse(struct Dee_function_assembler *__restrict self);

/* Step #2: Compile basic blocks and determine memory states. Fills in:
 * - self->fa_prolog
 * - self->fa_prolog_end
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_stat
 * - self->fa_blockv[*]->bb_mem_start
 * - self->fa_blockv[*]->bb_mem_end
 * - self->fa_blockv[*]->bb_host_start
 * - self->fa_blockv[*]->bb_host_end
 * Also makes sure that memory states at start/end of basic blocks are
 * always identical (or compatible; i.e.: MEMVAL_F_LOCAL_UNKNOWN can
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
 * - self->fa_except_first
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
 *   - self->fa_sections
 *   - self->fa_prolog.hs_link+hs_symbols
 *   - self->fa_blockv[*]->bb_htext.hs_link+hs_symbols
 *   - self->fa_blockv[*]->bb_hcold.hs_link+hs_symbols
 *   - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph.hs_link+hs_symbols
 *   - self->fa_except_exitv[*]->exi_block->bb_htext.hs_link+hs_symbols
 *   - self->fa_except_exitv[*]->exi_block->bb_hcold.hs_link+hs_symbols
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_ordersections(struct Dee_function_assembler *__restrict self);

#ifdef HOSTASM_HAVE_SHRINKJUMPS
/* Step #6: Try to shrink large in generated host text with smaller ones.
 * This is an arch-specific step. On x86 it replaces `jmpl' with `jmp8' (if possible) */
INTDEF NONNULL((1)) void DCALL
Dee_function_assembler_shrinkjumps(struct Dee_function_assembler *__restrict self);
#endif /* HOSTASM_HAVE_SHRINKJUMPS */

/* Step #7: Link blocks into an executable function blob.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_assembler_output(struct Dee_function_assembler *__restrict self,
                              struct Dee_hostfunc *__restrict result);


/* High-level wrapper function to fully assemble `function' into its host-asm equivalent.
 * @param: cc:    Calling convention of the generated function
 * @param: flags: Set of `DEE_FUNCTION_ASSEMBLER_F_*'
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_assemble(DeeFunctionObject *__restrict function,
             struct Dee_hostfunc *__restrict result,
             Dee_hostfunc_cc_t cc, uint16_t flags);



/* Error throwing helper functions. */
INTDEF ATTR_COLD int DCALL err_illegal_stack_effect(void);
INTDEF ATTR_COLD int DCALL err_illegal_ulid(Dee_ulid_t lid);
INTDEF ATTR_COLD int DCALL err_illegal_mid(uint16_t mid);
INTDEF ATTR_COLD int DCALL err_illegal_aid(Dee_aid_t aid);
INTDEF ATTR_COLD int DCALL err_illegal_cid(uint16_t cid);
#define err_illegal_sid(sid) err_illegal_cid(sid)
INTDEF ATTR_COLD int DCALL err_illegal_rid(uint16_t rid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_illegal_gid(struct Dee_module_object *__restrict mod, uint16_t gid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unsupported_opcode(DeeCodeObject *__restrict code, Dee_instruction_t const *instr);


/************************************************************************/
/* RUNTIME API FUNCTIONS CALLED BY GENERATED CODE                       */
/************************************************************************/
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_unbound_global(struct Dee_module_object *__restrict mod, uint16_t global_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL libhostasm_rt_err_unbound_local(struct Dee_code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL libhostasm_rt_err_unbound_arg(struct Dee_code_object *code, void *ip, uint16_t arg_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL libhostasm_rt_err_illegal_instruction(DeeCodeObject *code, void *ip);
INTDEF ATTR_COLD int DCALL libhostasm_rt_err_no_active_exception(void);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL libhostasm_rt_err_unbound_attribute_string(DeeTypeObject *__restrict tp, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_unbound_class_member(DeeTypeObject *__restrict class_type, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_unbound_instance_member(DeeTypeObject *__restrict class_type, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_requires_class(DeeTypeObject *__restrict tp_self);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_invalid_class_addr(DeeTypeObject *__restrict tp_self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_invalid_instance_addr(DeeTypeObject *__restrict tp_self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_nonempty_kw(DeeObject *__restrict kw);
INTDEF ATTR_COLD int DCALL libhostasm_rt_err_cell_empty_ValueError(void);
INTDEF ATTR_COLD int DCALL libhostasm_rt_err_cell_empty_UnboundAttribute(void);
INTDEF ATTR_COLD int DCALL libhostasm_rt_err_cannot_lock_weakref(void);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_rt_err_unbound_index(DeeObject *__restrict self, size_t index);
INTDEF ATTR_COLD NONNULL((1)) int DCALL libhostasm_err_invalid_unpack_size(DeeObject *__restrict unpack_object, size_t need_size, size_t real_size);
INTDEF WUNUSED NONNULL((1)) int DCALL libhostasm_rt_assert_empty_kw(DeeObject *__restrict kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL libhostasm_rt_DeeObject_ShlRepr(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_FPU
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL libhostasm_rt_DeeObject_Float(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_FPU */


/* Helpers for quickly filling in dict/set items where the key wasn't a compile-time constant expression.
 * These functions operate in a situation where "self" hasn't been fully initialized yet (i.e. don't do
 * locking, and can assume that "self != key && self != value"), as well as that "self" always has enough
 * space to hold he extra key (and associated value, if any)
 * @return: * :   Always re-return `self' on success.
 * @return: NULL: Error (in this case "self" was freed). */
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL libhostasm_rt_DeeDict_InsertFast(/*inherit(on_error)*/ DeeObject *__restrict self, /*inherit(always)*/ DREF DeeObject *key, /*inherit(always)*/ DREF DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL libhostasm_rt_DeeRoDict_InsertFast(/*inherit(on_error)*/ DeeObject *__restrict self, /*inherit(always)*/ DREF DeeObject *key, /*inherit(always)*/ DREF DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL libhostasm_rt_DeeHashSet_InsertFast(/*inherit(on_error)*/ DeeObject *__restrict self, /*inherit(always)*/ DREF DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL libhostasm_rt_DeeRoSet_InsertFast(/*inherit(on_error)*/ DeeObject *__restrict self, /*inherit(always)*/ DREF DeeObject *key);

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_LIBHOSTASM_H */
