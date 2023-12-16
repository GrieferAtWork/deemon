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

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/system-features.h>
#include <deemon/util/lock.h>

#include <hybrid/align.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifdef CONFIG_NO_LIBHOSTASM
#undef CONFIG_NO_LIBHOSTASM
#undef CONFIG_HAVE_LIBHOSTASM
#else /* CONFIG_NO_LIBHOSTASM */
#define CONFIG_HAVE_LIBHOSTASM

/* Check for ARCH support */
#if !(defined(__i386__) || defined(__x86_64__))
#undef CONFIG_HAVE_LIBHOSTASM
#endif /* ... */

/* Check for OS support */
#undef CONFIG_hostfunc_USES_VirtualAlloc
#undef CONFIG_hostfunc_USES_mmap
#if defined(CONFIG_HOST_WINDOWS)
#define CONFIG_hostfunc_USES_VirtualAlloc
#elif ((defined(CONFIG_HAVE_mmap64) || defined(CONFIG_HAVE_mmap)) &&        \
       defined(CONFIG_HAVE_mprotect) && defined(CONFIG_HAVE_MAP_ANON) &&    \
       defined(CONFIG_HAVE_PROT_READ) && defined(CONFIG_HAVE_PROT_WRITE) && \
       defined(CONFIG_HAVE_PROT_EXEC))
#ifndef CONFIG_HAVE_mmap
#define CONFIG_HAVE_mmap
#undef mmap
#define mmap mmap64
#endif /* !CONFIG_HAVE_mmap */
#define CONFIG_hostfunc_USES_mmap
#else /* ... */
#undef CONFIG_HAVE_LIBHOSTASM
#endif /* !... */
#endif /* !CONFIG_NO_LIBHOSTASM */

#ifdef CONFIG_HAVE_LIBHOSTASM

/* Select the arch for which to generate code. */
#if defined(__i386__) || defined(__x86_64__)
#define HOSTASM_X86
#ifdef __x86_64__
#define HOSTASM_X86_64
#ifdef CONFIG_HOST_WINDOWS
#define HOSTASM_X86_64_MSABI
#else /* CONFIG_HOST_WINDOWS */
#define HOSTASM_X86_64_SYSVABI
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* __x86_64__ */
#else /* ... */
#error "Unsupported architecture"
#endif /* !... */

#ifdef HOSTASM_X86_64
#define HOST_SIZEOF_POINTER 8
#elif defined(HOSTASM_X86)
#define HOST_SIZEOF_POINTER 4
#endif /* ... */


#ifdef HOSTASM_X86
#undef LIBGEN86_TARGET_BITS
#define LIBGEN86_TARGET_BITS (HOST_SIZEOF_POINTER * 8)
#include "libgen86/register.h"
#endif /* HOSTASM_X86 */

#define HOSTASM_STACK_GROWS_DOWN


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
 * - Hostasm re-compilation should take place in a separate thread that
 *   operates as a lazily-launched deamon, and takes re-compilation jobs
 *   as they come up.
 * - Add support for functions with custom exception handlers/finally
 * - Add support for functions with CODE_FASSEMBLY (by adding extra asserts in generated x86 code)
 * - Add an extra mapper for x86 assembly address -> bytecode address,
 *   as well as stack/locals -> x86 CFA-offset/register, so that the
 *   original set of DDI instrumentation can still be used, and (e.g.)
 *   an lid can be converted to an x86 CFA-offset/register at runtime.
 * - Make it possible to re-compile yield functions by:
 *   - Generating code differently such that the deemon stack and locals
 *     aren't used %Psp-relative, but (e.g.) %Pbp.
 *   - Calculate the max stack/locals blob size at (re-)compile-time
 *   - When a yield function returns, it gives back 2 values
 *     - DREF DeeObject * -- The yielded value
 *     - void *           -- The "resume PC"
 *     The "resume PC" must be loaded by:
 *     - Loading the stack/locals blob into %Pbp
 *     - Pushing the return PC onto %Psp
 *     - Jumping to the "resume PC"
 *   - The stack/locals blob is allocated/owned by 
 *
 */


DECL_BEGIN

/* Figure out how many scratch registers this host gives us. */
#ifdef HOSTASM_X86_64_MSABI
#define HOST_REGISTER_RETURN HOST_REGISTER_RAX
#define HOST_REGISTER_RAX    0
#define HOST_REGISTER_RCX    1
#define HOST_REGISTER_RDX    2
#define HOST_REGISTER_R8     3
#define HOST_REGISTER_R9     4
#define HOST_REGISTER_R10    5
#define HOST_REGISTER_R11    6
#define HOST_REGISTER_COUNT  7 /* %rax, %rcx, %rdx, %r8, %r9, %r10, %r11 */
#elif defined(HOSTASM_X86_64_SYSVABI)
#define HOST_REGISTER_RETURN HOST_REGISTER_RAX
#define HOST_REGISTER_RAX    0
#define HOST_REGISTER_RCX    1
#define HOST_REGISTER_RDX    2
#define HOST_REGISTER_RDI    3
#define HOST_REGISTER_RSI    4
#define HOST_REGISTER_R8     5
#define HOST_REGISTER_R9     6
#define HOST_REGISTER_R10    7
#define HOST_REGISTER_R11    8
#define HOST_REGISTER_COUNT  9 /* %rax, %rcx, %rdx, %rdi, %rsi, %r8, %r9, %r10, %r11 */
#elif defined(HOSTASM_X86)
#define HOST_REGISTER_RETURN HOST_REGISTER_EAX
#define HOST_REGISTER_EAX    0
#define HOST_REGISTER_ECX    1
#define HOST_REGISTER_EDX    2
#define HOST_REGISTER_COUNT  3 /* %eax, %ecx, %edx */
#endif /* ... */

typedef uint8_t Dee_host_register_t;

/* Host function assembly calling convention (one of `HOSTFUNC_CC_*'). */
typedef uint8_t Dee_hostfunc_cc_t;

/* Possible calling convention flags. */
#define HOSTFUNC_CC_F_KW    1
#define HOSTFUNC_CC_F_THIS  2
#define HOSTFUNC_CC_F_TUPLE 4

/* Possible calling conventions. */
#define HOSTFUNC_CC_CALL              0 /* DREF DeeObject *(DCALL *)(size_t argc, DeeObject *const *argv); */
#define HOSTFUNC_CC_CALL_KW           1 /* DREF DeeObject *(DCALL *)(size_t argc, DeeObject *const *argv, DeeObject *kw); */
#define HOSTFUNC_CC_THISCALL          2 /* DREF DeeObject *(DCALL *)(DeeObject *self, size_t argc, DeeObject *const *argv); */
#define HOSTFUNC_CC_THISCALL_KW       3 /* DREF DeeObject *(DCALL *)(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw); */
#define HOSTFUNC_CC_CALL_TUPLE        4 /* DREF DeeObject *(DCALL *)(DeeObject *args); */
#define HOSTFUNC_CC_CALL_TUPLE_KW     5 /* DREF DeeObject *(DCALL *)(DeeObject *args, DeeObject *kw); */
#define HOSTFUNC_CC_THISCALL_TUPLE    6 /* DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *args); */
#define HOSTFUNC_CC_THISCALL_TUPLE_KW 7 /* DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *args, DeeObject *kw); */
#define HOSTFUNC_CC_COUNT             8
union Dee_hostfunc_entry {
	DREF DeeObject *(DCALL *hfe_call)(size_t argc, DeeObject *const *argv);                                        /* HOSTFUNC_CC_CALL */
	DREF DeeObject *(DCALL *hfe_call_kw)(size_t argc, DeeObject *const *argv, DeeObject *kw);                      /* HOSTFUNC_CC_CALL_KW */
	DREF DeeObject *(DCALL *hfe_thiscall)(DeeObject *self, size_t argc, DeeObject *const *argv);                   /* HOSTFUNC_CC_THISCALL */
	DREF DeeObject *(DCALL *hfe_thiscall_kw)(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw); /* HOSTFUNC_CC_THISCALL_KW */
	DREF DeeObject *(DCALL *hfe_call_tuple)(DeeObject *args);                                                      /* HOSTFUNC_CC_CALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_call_tuple_kw)(DeeObject *args, DeeObject *kw);                                    /* HOSTFUNC_CC_CALL_TUPLE_KW */
	DREF DeeObject *(DCALL *hfe_thiscall_tuple)(DeeObject *self, DeeObject *args);                                 /* HOSTFUNC_CC_THISCALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_thiscall_tuple_kw)(DeeObject *self, DeeObject *args, DeeObject *kw);               /* HOSTFUNC_CC_THISCALL_TUPLE_KW */
};


struct Dee_hostfunc {
	union Dee_hostfunc_entry hf_entry; /* Function entry point. */
	void                   *_hf_base;  /* Mmap base address. */
	size_t                  _hf_size;  /* Mmap size address. */
};

#ifdef __CYGWIN__
/* Cygwin's `getpagesize' is broken in that it returns the
 * allocation granularity instead of the actual page-size. */
#undef getpagesize
#define getpagesize() 4096
#elif !defined(CONFIG_HAVE_getpagesize)
#ifdef __ARCH_PAGESIZE
#define getpagesize() __ARCH_PAGESIZE
#elif defined(PAGESIZE)
#define getpagesize() PAGESIZE
#elif defined(PAGE_SIZE)
#define getpagesize() PAGE_SIZE
#elif defined(EXEC_PAGESIZE)
#define getpagesize() EXEC_PAGESIZE
#elif defined(NBPG) && defined(CLSIZE)
#define getpagesize() (NBPG * CLSIZE)
#elif defined(NBPG)
#define getpagesize() NBPG
#elif defined(_SC_PAGESIZE)
#define getpagesize() sysconf(_SC_PAGESIZE)
#elif defined(_SC_PAGE_SIZE)
#define getpagesize() sysconf(_SC_PAGE_SIZE)
#elif defined(CONFIG_HOST_WINDOWS)
#define getpagesize() dee_nt_getpagesize()
PRIVATE ATTR_CONST size_t DCALL dee_nt_getpagesize(void) {
	static size_t ps = 0;
	if (ps == 0) {
		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		ps = system_info.dwPageSize;
		if unlikely(ps == 0)
			ps = 1;
	}
	return ps;
}
#else /* ... */
#define getpagesize() 4096 /* Just guess... */
#endif /* !... */
#endif /* !CONFIG_HAVE_getpagesize */


#if defined(CONFIG_hostfunc_USES_VirtualAlloc)
#define Dee_hostfunc_init(self, size)                      \
	(((self)->_hf_size = CEIL_ALIGN(size, getpagesize())), \
	 ((self)->_hf_base = VirtualAlloc(NULL, (self)->_hf_size, MEM_COMMIT, PAGE_READWRITE)) != NULL ? 0 : -1)
#define Dee_hostfunc_fini(self)   (void)VirtualFree((self)->_hf_base, 0, MEM_RELEASE)
#define Dee_hostfunc_mkexec(self) Dee_hostfunc_mkexec(self)
LOCAL WUNUSED NONNULL((1)) int (DCALL Dee_hostfunc_mkexec)(struct Dee_hostfunc *__restrict self) {
	DWORD dwTemp;
	return VirtualProtect(self->_hf_base, self->_hf_size, PAGE_EXECUTE_READ, &dwTemp) ? 0 : -1;
}
#elif defined(CONFIG_hostfunc_USES_mmap)
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)(uintptr_t)-1)
#endif /* !MAP_FAILED */
#define Dee_hostfunc_init(self, size) \
	(((self)->_hf_size = CEIL_ALIGN(size, getpagesize())), \
	 ((self)->_hf_base = mmap(NULL, (self)->_hf_size, PROT_READ | PROT_WRITE, \
	                          MAP_PRIVATE | MAP_ANON, -1, 0)) != MAP_FAILED ? 0 : -1)
#define Dee_hostfunc_fini(self)   (void)munmap((self)->_hf_base, (self)->_hf_size)
#define Dee_hostfunc_mkexec(self) mprotect((self)->_hf_base, (self)->_hf_size, PROT_READ | PROT_EXEC)
#else /* ... */
#define Dee_hostfunc_init(self, size)                      \
	(((self)->_hf_size = CEIL_ALIGN(size, getpagesize())), \
	 ((self)->_hf_base = Dee_TryMalloc((self)->_hf_size)) != NULL ? 0 : -1)
#define Dee_hostfunc_fini(self)   Dee_Free((self)->_hf_base)
#define Dee_hostfunc_mkexec(self) 0
#endif /* !... */


struct Dee_memloc {
#define MEMLOC_F_NORMAL        0x0000
#define MEMLOC_F_NOREF         0x0001 /* Slot contains no reference */
#define MEMLOC_M_LOCAL_BSTATE  0xc000 /* Mask for the bound-ness of a local variable */
#define MEMLOC_F_LOCAL_UNKNOWN 0x0000 /* Local variable bound-ness is unknown */
#define MEMLOC_F_LOCAL_BOUND   0x4000 /* Local variable is bound */
#define MEMLOC_F_LOCAL_UNBOUND 0x8000 /* Local variable is unbound */
	uint16_t ml_flags; /* Location flags (set of `MEMLOC_F_*') */
#define MEMLOC_TYPE_HSTACK  0 /* Host stack (CFA offset) */
#define MEMLOC_TYPE_HREG    1 /* Host register */
#define MEMLOC_TYPE_ARG     2 /* Function argument */
#define MEMLOC_TYPE_CONST   3 /* Constant deemon object */
#define MEMLOC_TYPE_UNALLOC 4 /* Not allocated (only valid for local variables) */
	uint16_t ml_where; /* Location kind (one of `MEMLOC_TYPE_*') */
	union {
		uintptr_t          _ml_data;
		Dee_host_register_t ml_hreg;   /* [valid_if(ml_where == MEMLOC_TYPE_HREG)] Host register number (< HOST_REGISTER_COUNT) */
		uintptr_t           ml_hstack; /* [valid_if(ml_where == MEMLOC_TYPE_HSTACK)][ALIGNED(HOST_SIZEOF_POINTER)] Host stack CFA offset */
		uint16_t            ml_harg;   /* [valid_if(ml_where == MEMLOC_TYPE_ARG)] Function argument number */
		DeeObject          *ml_const;  /* [valid_if(ml_where == MEMLOC_TYPE_CONST)][1..1] Constant object */
	} ml_value;
};

/* Check if `a' and `b' describe the same host memory location (i.e. are aliasing each other). */
#define Dee_memloc_sameloc(a, b)       \
	((a)->ml_where == (b)->ml_where && \
	 (a)->ml_value._ml_data == (b)->ml_value._ml_data)
#define Dee_memloc_isconst(a, value) \
	((a)->ml_where == MEMLOC_TYPE_CONST && (a)->ml_value.ml_const == (value))



/* Possible values for `Dee_memstate::ms_regs' */
#define REGISTER_USAGE_GENERIC 0x00 /* Register usage is defined by `ms_stackv' and `ms_localv'. */
#define REGISTER_USAGE_THIS    0x01 /* Register contains: argc / DeeTuple_SIZE(args) */
#define REGISTER_USAGE_ARGC    0x02 /* Register contains: argc / DeeTuple_SIZE(args) */
#define REGISTER_USAGE_ARGV    0x03 /* Register contains: argv / DeeTuple_ELEM(args) */
#define REGISTER_USAGE_ARGS    0x04 /* Register contains: args */
#define REGISTER_USAGE_KW      0x05 /* Register contains: kw */
typedef uint8_t Dee_host_regusage_t;


struct Dee_memstate {
	Dee_refcnt_t                               ms_refcnt;          /* Reference counter for the mem-state (state becomes read-only when >1) */
	uintptr_t                                  ms_host_cfa_offset; /* Delta between SP to CFA (Canonical Frame Address) */
	uint16_t                                   ms_localc;          /* [== :co_localc] Number of local variables. */
	uint16_t                                   ms_stackc;          /* Number of (currently) used deemon stack slots in use. */
	uint16_t                                   ms_stacka;          /* Allocated number of deemon stack slots in use. */
	Dee_host_regusage_t                        ms_regs[HOST_REGISTER_COUNT]; /* Register usage (set to `REGISTER_USAGE_GENERIC' if register appears in a `Dee_memloc') */
	struct Dee_memloc                         *ms_stackv;          /* [0..ms_stackc][owned] Deemon stack memory locations. */
#ifdef __INTELLISENSE__
	struct Dee_memloc                          ms_localv[1024];    /* [0..ms_localc] Deemon locals memory locations. */
#else /* __INTELLISENSE__ */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_memloc, ms_localv);         /* [0..ms_localc] Deemon locals memory locations. */
#endif /* !__INTELLISENSE__ */
};

#define Dee_memstate_alloc(localc)                                                \
	((struct Dee_memstate *)Dee_Malloc(offsetof(struct Dee_memstate, ms_localv) + \
	                                   (localc) * sizeof(struct Dee_memloc)))
#define Dee_memstate_free(self) Dee_Free(self)
INTDEF NONNULL((1)) void DCALL Dee_memstate_destroy(struct Dee_memstate *__restrict self);
#define Dee_memstate_incref(self) (void)(++(self)->ms_refcnt)
#define Dee_memstate_decref(self) (void)(--(self)->ms_refcnt || (Dee_memstate_destroy(self), 0))

/* Replace `*p_self' with a copy of itself
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_memstate_inplace_copy(struct Dee_memstate **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) DREF struct Dee_memstate *DCALL
Dee_memstate_copy(struct Dee_memstate *__restrict self);
#define Dee_memstate_isshared(self) ((self)->ms_refcnt > 1)
#define Dee_memstate_unshare(p_self) \
	unlikely(Dee_memstate_isshared(*(p_self)) ? Dee_memstate_inplace_copy(p_self) : 0)

/* Ensure that at least `min_alloc' stack slots are allocated. */
INTDEF NONNULL((1)) int DCALL
Dee_memstate_reqvstack(struct Dee_memstate *__restrict self,
                       uint16_t min_alloc);


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
 *                                  `ms_regs[return] != REGISTER_USAGE_GENERIC' */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused(struct Dee_memstate const *__restrict self,
                               bool accept_if_with_regusage);

/* Same as `Dee_memstate_hregs_find_unused(self, true)', but don't return `not_these',
 * which is an array of register numbers terminated by one `>= HOST_REGISTER_COUNT'.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent. */
INTDEF WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused_ex(struct Dee_memstate *__restrict self,
                                  Dee_host_register_t const *not_these);

/* Try to find a `n_bytes'-large free section of host stack memory.
 * @return: * :            The base-CFA offset of the free section of memory
 * @return: (uintptr_t)-1: There is no free section of at least `n_bytes' bytes.
 *                         In this case, allocate using `Dee_memstate_hstack_alloca()' */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
Dee_memstate_hstack_find(struct Dee_memstate const *__restrict self, size_t n_bytes);

/* Try to free unused stack memory near the top of the stack.
 * @return: true:  The CFA offset was reduced.
 * @return: false: The CFA offset remains the same. */
INTDEF NONNULL((1)) bool DCALL
Dee_memstate_hstack_free(struct Dee_memstate *__restrict self);

/* Constrain `self' with `other', such that it is possible to generate code to
 * transition from `other' to `self', as well as any other mem-state that might
 * be the result of further constraints applied to `self'. */
INTDEF WUNUSED NONNULL((1, 2)) void DCALL
Dee_memstate_constrainwith(struct Dee_memstate *__restrict self,
                           struct Dee_memstate const *__restrict other);

/* Functions to manipulate the virtual deemon object stack. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vswap(struct Dee_memstate *__restrict self); /* ASM_SWAP */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vlrot(struct Dee_memstate *__restrict self, size_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vrrot(struct Dee_memstate *__restrict self, size_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush(struct Dee_memstate *__restrict self, struct Dee_memloc *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_memstate_vpush_const(struct Dee_memstate *__restrict self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vpush_arg(struct Dee_memstate *__restrict self, uint16_t aid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_memstate_vdup_n(struct Dee_memstate *__restrict self, size_t n);
#define Dee_memstate_vdup(self) Dee_memstate_vdup_n(self, 1)


struct Dee_host_jmpdesc {
	uintptr_t hjd_bb_offset; /* Offset from the start of generated code to where this host-jmp-instruction resides.
	                          * If this points to some delta, or some other part of the instruction is arch-specific,
	                          * and the only portable guaranty is that it points somewhere into [instr-start,instr-end)
	                          *
	                          * X86: Points to the start of the signed 32-bit delta of the jcc or jmp instruction. */
};

struct Dee_jump_descriptor {
	Dee_instruction_t const  *jd_from;   /* [1..1][const] Deemon instruction where the jump originates from. */
	struct Dee_basic_block   *jd_to;     /* [1..1][const] Basic block that this jump goes to. */
#ifdef __INTELLISENSE__
	struct Dee_memstate      *jd_stat;   /* [0..1] Memory state at the point where `jd_from' performs its jump (or NULL if not yet generated). */
#else /* __INTELLISENSE__ */
	DREF struct Dee_memstate *jd_stat;   /* [0..1] Memory state at the point where `jd_from' performs its jump (or NULL if not yet generated). */
#endif /* !__INTELLISENSE__ */
	struct Dee_host_jmpdesc   jd_host;   /* Host-specific jmp data, including where the jmp is located in generated code. */
};

#define Dee_jump_descriptor_alloc()    ((struct Dee_jump_descriptor *)Dee_Malloc(sizeof(struct Dee_jump_descriptor)))
#define Dee_jump_descriptor_free(self) Dee_Free(self)
#define Dee_jump_descriptor_fini(self) (void)(!(self)->jd_stat || (Dee_memstate_decref((self)->jd_stat), 0))
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
	struct Dee_jump_descriptors bb_entries;      /* All of the possible ways this basic block can be entered (at `bb_deemon_start' / `bb_host_start'; this one owns descriptors). */
	struct Dee_jump_descriptors bb_exits;        /* All of the possible ways this basic block can be exited. */
	struct Dee_basic_block     *bb_next;         /* [0..1] Fallthru exit of this basic block (or NULL if there is none, which happens for the last block and blocks that end with NORETURN instructions) */
#ifdef __INTELLISENSE__
	struct Dee_memstate        *bb_mem_start;    /* [0..1] Memory state at start of basic block (or NULL if not yet assembled) */
	struct Dee_memstate        *bb_mem_end;      /* [0..1] Memory state at end of basic block (or NULL if not yet assembled) */
#else /* __INTELLISENSE__ */
	DREF struct Dee_memstate   *bb_mem_start;    /* [0..1] Memory state at start of basic block (or NULL if not yet assembled) */
	DREF struct Dee_memstate   *bb_mem_end;      /* [0..1] Memory state at end of basic block (or NULL if not yet assembled) */
#endif /* !__INTELLISENSE__ */
	byte_t                     *bb_host_start;   /* [0..bb_host_size][owned] Start of host assembly */
	byte_t                     *bb_host_end;     /* [0..bb_host_size] End of host assembly */
	size_t                      bb_host_free;    /* Amount of unused, trailing memory in `bb_host_start' */
	/* TODO: Host text relocations (for API calls). */
};

#define Dee_basic_block_alloc()    ((struct Dee_basic_block *)Dee_Malloc(sizeof(struct Dee_basic_block)))
#define Dee_basic_block_free(self) Dee_Free(self)

INTDEF NONNULL((1)) void DCALL
Dee_basic_block_destroy(struct Dee_basic_block *__restrict self);

/* Split this basic block at `addr' (which must be `> bb_deemon_start'),
 * and move all jumps from `bb_exits' into the new basic block, as needed.
 * @return: * :   A new basic block that starts at `addr'
 * @return: NULL: Error */
INTDEF WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_basic_block_splitat(struct Dee_basic_block *__restrict self,
                        Dee_instruction_t const *addr);

/* Ensure that at least `num_bytes' of host text memory are available.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_reqhost(struct Dee_basic_block *__restrict self,
                         size_t num_bytes);
#define Dee_basic_block_reqhost(self, num_bytes) \
	((self)->bb_host_free >= (num_bytes) ? 0 : _Dee_basic_block_reqhost(self, num_bytes))



struct Dee_function_assembler {
	DeeFunctionObject       *fa_function; /* [1..1][const] The function being assembled */
	DeeCodeObject           *fa_code;     /* [1..1][const][== fa_function->fo_code] The code being assembled */
	struct Dee_basic_block **fa_blockv;   /* [owned][0..fa_blockc][owned] Vector of basic blocks (stored by `bb_deemon_start'). */
	size_t                   fa_blockc;   /* Number of basic blocks. */
	size_t                   fa_blocka;   /* Allocated number of basic blocks. */
	Dee_hostfunc_cc_t        fa_cc;       /* [const] Calling convention. */
};

/* Return the extra CFA addend that needs to be freed on function return */
#ifdef HOSTASM_X86_64_SYSVABI
INTDEF size_t const _Dee_function_assembler_cfa_addend[HOSTFUNC_CC_COUNT];
#define Dee_function_assembler_get_cfa_addend(self) _Dee_function_assembler_cfa_addend[(self)->fa_cc]
#else /* HOSTASM_X86_64_SYSVABI */
#define Dee_function_assembler_get_cfa_addend(self) 0
#endif /* !HOSTASM_X86_64_SYSVABI */



#define Dee_function_assembler_addrof(self, addr) \
	((Dee_code_addr_t)((addr) - (self)->fa_code->co_code))

#define Dee_function_assembler_init(self, function, cc)        \
	(void)((self)->fa_function = (function),                   \
	       (self)->fa_code     = (self)->fa_function->fo_code, \
	       (self)->fa_blockv   = NULL,                         \
	       (self)->fa_blockc   = 0,                            \
	       (self)->fa_blocka   = 0,                            \
	       (self)->fa_cc       = (cc))
INTDEF NONNULL((1)) void DCALL
Dee_function_assembler_fini(struct Dee_function_assembler *__restrict self);

/* ================ Helpers ================ */

/* Ensure that the basic block containing `deemon_addr' also *starts* at that address.
 * This function is used during the initial code-pass where basic blocks are identified
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




/* ================ Loaders ================ */

struct Dee_function_generator {
	struct Dee_function_assembler *fg_assembler; /* [1..1][const] Assembler. */
	struct Dee_basic_block        *fg_block;     /* [1..1][const] Output basic block. */
	DREF struct Dee_memstate      *fg_state;     /* [1..1] Current memory state. */
};

#define Dee_function_generator_state_unshare(self) Dee_memstate_unshare(&(self)->fg_state)

/* Code generator helpers to manipulate the V-stack. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vswap(struct Dee_function_generator *__restrict self); /* ASM_SWAP */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vlrot(struct Dee_function_generator *__restrict self, size_t n);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vrrot(struct Dee_function_generator *__restrict self, size_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush(struct Dee_function_generator *__restrict self, struct Dee_memloc *loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_const(struct Dee_function_generator *__restrict self, DeeObject *value);
#define Dee_function_generator_vpush_addr(self, addr) Dee_function_generator_vpush_const(self, (DeeObject *)(void *)(addr))
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_arg(struct Dee_function_generator *__restrict self, uint16_t aid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_local(struct Dee_function_generator *__restrict self, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdup_n(struct Dee_function_generator *__restrict self, size_t n);
#define Dee_function_generator_vdup(self) Dee_function_generator_vdup_n(self, 1)
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_n(struct Dee_function_generator *__restrict self, size_t n);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vpush_usage(struct Dee_function_generator *__restrict self, Dee_host_regusage_t usage);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_local(struct Dee_function_generator *__restrict self, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_local(struct Dee_function_generator *__restrict self, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vop(struct Dee_function_generator *__restrict self, uint16_t operator_name, uint16_t argc);

/* Take the base address of the top-most `DeeObject' from the object-stack, add `offset' to that
 * base address (possibly at runtime), then re-interpret that address as `(DeeObject **)<addr>'
 * and dereference it, before storing the resulting value back into the to-most stack item. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vind(struct Dee_function_generator *__restrict self, ptrdiff_t offset);

/* >> *(SECOND + offset) = FIRST; POP(); POP(); */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_ind(struct Dee_function_generator *__restrict self, ptrdiff_t offset);
/* >> temp = *(SECOND + offset); *(SECOND + offset) = FIRST; FIRST = temp; */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vxch_ind(struct Dee_function_generator *__restrict self, ptrdiff_t offset);

/* Ensure that the top-most `DeeObject' from the object-stack is a reference. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vref(struct Dee_function_generator *__restrict self);

/* Generate code to push a global variable onto the virtual stack. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_global(struct Dee_function_generator *__restrict self, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_global(struct Dee_function_generator *__restrict self, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_global(struct Dee_function_generator *__restrict self, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vdel_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpush_static(struct Dee_function_generator *__restrict self, uint16_t sid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vpop_static(struct Dee_function_generator *__restrict self, uint16_t sid);

/* Return and pass the top-most stack element as function result.
 * This function will clear the stack and unbind all local variables. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_vret(struct Dee_function_generator *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_vthrow(struct Dee_function_generator *__restrict self);

/* Generate host text to invoke `api_function' with the top-most `argc' items from the stack.
 * @param: cc: One of `VCALLOP_CC_*', describing the calling-convention of `api_function' */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallop(struct Dee_function_generator *__restrict self,
                               void *api_function, unsigned int cc, size_t argc);
#define VCALLOP_CC_OBJECT  0 /* DREF DeeObject *(DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]); */
#define VCALLOP_CC_INT     1 /* int (DCALL *api_function)(DeeObject *, [DeeObject *, [DeeObject *, [...]]]); (leaving an implicit `Dee_None') */
#define VCALLOP_CC_INPLACE 2 /* int (DCALL *api_function)(DeeObject **, [DeeObject *, [DeeObject *, [...]]]); */

/* Generate a call to a C-function `c_function' with `argc'
 * pointer-sized arguments whose values are taken from `argv'. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gcall_c_function(struct Dee_function_generator *__restrict self,
                                        void *c_function, size_t argc,
                                        struct Dee_memloc const *argv);


/* Code generators. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gincref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gdecref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxincref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gxdecref(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc);

#define Dee_function_generator_grwlock_read(self, lock)     _Dee_function_generator_grwlock_read(self, lock)
#define Dee_function_generator_grwlock_write(self, lock)    _Dee_function_generator_grwlock_write(self, lock)
#define Dee_function_generator_grwlock_endwrite(self, lock) _Dee_function_generator_grwlock_endwrite(self, lock)
#define Dee_function_generator_grwlock_endread(self, lock)  _Dee_function_generator_grwlock_endread(self, lock)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_read(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_write(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_endwrite(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL _Dee_function_generator_grwlock_endread(struct Dee_function_generator *__restrict self, Dee_atomic_rwlock_t *__restrict lock);

/* Allocate/deallocate memory from the host stack.
 * If stack memory gets allocated, zero-initialize it. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_adjust(struct Dee_function_generator *__restrict self, ptrdiff_t cfa_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_pushreg(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_ghstack_pushconst(struct Dee_function_generator *__restrict self, DeeObject *value);
#define _Dee_function_generator_ghstack_adjust(self, sp_delta)   _Dee_basic_block_ghstack_adjust((self)->fg_block, sp_delta)
#define _Dee_function_generator_ghstack_pushreg(self, src_regno) _Dee_basic_block_ghstack_pushreg((self)->fg_block, src_regno)
#define _Dee_function_generator_ghstack_pushconst(self, value)   _Dee_basic_block_ghstack_pushconst((self)->fg_block, value)
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_ghstack_adjust(struct Dee_basic_block *__restrict self, ptrdiff_t sp_delta);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_ghstack_pushreg(struct Dee_basic_block *__restrict self, Dee_host_register_t src_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_ghstack_pushconst(struct Dee_basic_block *__restrict self, DeeObject *value);
#define Dee_function_generator_gmov_reg2hstack(self, src_regno, cfa_offset)            _Dee_function_generator_gmov_reg2hstack(self, src_regno, cfa_offset)
#define Dee_function_generator_gmov_hstack2reg(self, cfa_offset, dst_regno)            (unlikely(_Dee_function_generator_gmov_hstack2reg(self, Dee_memstate_hstack_cfa2sp((self)->fg_state, cfa_offset), dst_regno)) ? -1 : ((self)->fg_state->ms_regs[dst_regno] = REGISTER_USAGE_GENERIC, 0))
#define Dee_function_generator_gmov_const2reg(self, value, dst_regno)                  (unlikely(_Dee_function_generator_gmov_const2reg(self, value, dst_regno)) ? -1 : ((self)->fg_state->ms_regs[dst_regno] = REGISTER_USAGE_GENERIC, 0))
#define Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno)                (unlikely(_Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno)) ? -1 : ((self)->fg_state->ms_regs[dst_regno] = (self)->fg_state->ms_regs[src_regno], 0))
#define Dee_function_generator_gmov_regind2reg(self, src_regno, src_delta, dst_regno)  (unlikely(_Dee_function_generator_gmov_regind2reg(self, src_regno, src_delta, dst_regno)) ? -1 : ((self)->fg_state->ms_regs[dst_regno] = REGISTER_USAGE_GENERIC, 0))
#define Dee_function_generator_gmov_reg2regind(self, src_regno, dst_regno, dst_delta)  _Dee_function_generator_gmov_reg2regind(self, src_regno, dst_regno, dst_delta)
#define Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno)             (unlikely(_Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno)) ? -1 : ((self)->fg_state->ms_regs[dst_regno] = REGISTER_USAGE_GENERIC, 0))
#define Dee_function_generator_gmov_reg2constind(self, src_regno, p_value)             _Dee_function_generator_gmov_reg2constind(self, src_regno, p_value)
#define _Dee_function_generator_gmov_reg2hstack(self, src_regno, sp_offset)            _Dee_basic_block_gmov_reg2hstack((self)->fg_block, src_regno, sp_offset)
#define _Dee_function_generator_gmov_hstack2reg(self, sp_offset, dst_regno)            _Dee_basic_block_gmov_hstack2reg((self)->fg_block, sp_offset, dst_regno)
#define _Dee_function_generator_gmov_const2reg(self, value, dst_regno)                 _Dee_basic_block_gmov_const2reg((self)->fg_block, value, dst_regno)
#define _Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno)               _Dee_basic_block_gmov_reg2reg((self)->fg_block, src_regno, dst_regno)
#define _Dee_function_generator_gmov_regind2reg(self, src_regno, src_delta, dst_regno) _Dee_basic_block_gmov_regind2reg((self)->fg_block, src_regno, src_delta, dst_regno)
#define _Dee_function_generator_gmov_reg2regind(self, src_regno, dst_regno, dst_delta) _Dee_basic_block_gmov_reg2regind((self)->fg_block, src_regno, dst_regno, dst_delta)
#define _Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno)            _Dee_basic_block_gmov_constind2reg((self)->fg_block, p_value, dst_regno)
#define _Dee_function_generator_gmov_reg2constind(self, src_regno, p_value)            _Dee_basic_block_gmov_reg2constind((self)->fg_block, src_regno, p_value)
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_reg2hstack(struct Dee_basic_block *__restrict self, Dee_host_register_t src_regno, ptrdiff_t sp_offset);                                /* *(SP + sp_offset) = dst_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_hstack2reg(struct Dee_basic_block *__restrict self, ptrdiff_t sp_offset, Dee_host_register_t dst_regno);                                /* dst_regno = *(SP + sp_offset); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_const2reg(struct Dee_basic_block *__restrict self, DeeObject *value, Dee_host_register_t dst_regno);                                    /* dst_regno = <value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_reg2reg(struct Dee_basic_block *__restrict self, Dee_host_register_t src_regno, Dee_host_register_t dst_regno);                         /* dst_regno = src_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_regind2reg(struct Dee_basic_block *__restrict self, Dee_host_register_t src_regno, ptrdiff_t src_delta, Dee_host_register_t dst_regno); /* dst_regno = *(src_regno + src_delta); */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_reg2regind(struct Dee_basic_block *__restrict self, Dee_host_register_t src_regno, Dee_host_register_t dst_regno, ptrdiff_t dst_delta); /* *(dst_regno * dst_delta) = src_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_constind2reg(struct Dee_basic_block *__restrict self, DeeObject **p_value, Dee_host_register_t dst_regno);                              /* dst_regno = *<p_value>; */
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_basic_block_gmov_reg2constind(struct Dee_basic_block *__restrict self, Dee_host_register_t src_regno, DeeObject **p_value);                              /* *<p_value> = src_regno; */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_arg2reg(struct Dee_function_generator *__restrict self, uint16_t aid, Dee_host_register_t dst_regno);                             /* dst_regno = <ARGV[aid]>; */
#define Dee_function_generator_gmov_reg2loc(self, src_regno, dst_loc) Dee_basic_block_gmov_reg2loc((self)->fg_block, src_regno, dst_loc)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_basic_block_gmov_reg2loc(struct Dee_basic_block *__restrict self, Dee_host_register_t src_regno, struct Dee_memloc const *__restrict dst_loc);               /* <dst_loc> = src_regno; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_loc2reg(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, Dee_host_register_t dst_regno); /* dst_regno = <src_loc>; */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gmov_locind2reg(struct Dee_function_generator *__restrict self, struct Dee_memloc const *__restrict src_loc, ptrdiff_t src_delta, Dee_host_register_t dst_regno); /* dst_regno = *<src_loc + src_delta>; */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Dee_function_generator_gmov_reg2locind(struct Dee_function_generator *__restrict self, Dee_host_register_t src_regno, struct Dee_memloc const *__restrict dst_loc, ptrdiff_t dst_delta); /* *<dst_loc + dst_delta> = src_regno; */

/* Load special runtime values into `dst_regno' */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gmov_usage2reg(struct Dee_function_generator *__restrict self, Dee_host_regusage_t usage, Dee_host_register_t dst_regno);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gmov_usage2reg(struct Dee_function_generator *__restrict self, Dee_host_regusage_t usage, Dee_host_register_t dst_regno);

/* Generate code to return `loc'. No extra code to decref stack/locals is generated. If you
 * want that extra code to be generated, you need to use `Dee_function_generator_vret()'. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gret(struct Dee_function_generator *__restrict self, /*inherit_ref*/ struct Dee_memloc *__restrict loc);
INTDEF WUNUSED NONNULL((1)) int DCALL _Dee_function_generator_gret(struct Dee_function_generator *__restrict self);

/* Allocate at host register, possibly flushing other an already used register to stack.
 * @param: not_these: Array of registers not to allocated, terminated by one `>= HOST_REGISTER_COUNT'.
 * @return: * : The allocated register
 * @return: >= HOST_REGISTER_COUNT: Error */
INTDEF WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gallocreg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t const *not_these);
#define Dee_function_generator_gtryallocreg(self, not_these) \
	Dee_memstate_hregs_find_unused_ex((self)->fg_state, not_these)


/* Generate code to flush all registers used by the deemon stack/locals into the host stack.
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushregs(struct Dee_function_generator *__restrict self,
                                  size_t ignore_top_n_stack_if_not_ref);

/* Generate code to assert that location `loc' is non-NULL:
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_LOCAL, lid, <ignored>, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_GLOBAL, gid, <ignored>, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_EXTERN, mid, gid, NULL);
 * The `kind', `id1' and `id2' arguments simply select `Dee_function_generator_gthrow_*_unbound()'
 * @param: opt_endread_before_throw: When non-NULL, emit `Dee_function_generator_grwlock_endread()'
 *                                   before the `Dee_function_generator_gthrow_*_unbound' code. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gassert_bound(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc *loc, uint8_t kind,
                                     uint16_t id1, uint16_t id2,
                                     Dee_atomic_rwlock_t *opt_endread_before_throw);
#define Dee_function_generator_gassert_local_bound(self, lid) \
	((lid) >= (self)->fg_state->ms_localc                     \
	 ? err_illegal_lid(lid)                                   \
	 : Dee_function_generator_gassert_local_bound_ex(self, &(self)->fg_state->ms_localv[lid], lid))
#define Dee_function_generator_gassert_local_bound_ex(self, loc, lid) \
	Dee_function_generator_gassert_bound(self, loc, ASM_LOCAL, lid, 0, NULL)
#define Dee_function_generator_gassert_global_bound(self, loc, gid) \
	Dee_function_generator_gassert_bound(self, loc, ASM_GLOBAL, gid, 0, NULL)
#define Dee_function_generator_gassert_extern_bound(self, loc, mid, gid) \
	Dee_function_generator_gassert_bound(self, loc, ASM_EXTERN, mid, gid, NULL)

/* Generate code to throw an error indicating that local variable `lid'
 * is unbound. This includes any necessary jump for the purpose of entering
 * an exception handler. */
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_local_unbound(struct Dee_function_generator *__restrict self, uint16_t lid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_global_unbound(struct Dee_function_generator *__restrict self, uint16_t gid);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gthrow_extern_unbound(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid);

/* Generate checks to enter exception handling mode. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gexcept_if_zero(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Dee_function_generator_gexcept_if_nonzero(struct Dee_function_generator *__restrict self, struct Dee_memloc *__restrict loc);
INTDEF WUNUSED NONNULL((1)) int DCALL Dee_function_generator_gexcept(struct Dee_function_generator *__restrict self);

/* Convert a single deemon instruction `instr' to host assembly and adjust the host memory
 * state according to the instruction in question. This is the core function to parse deemon
 * code and convert it to host assembly. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_geninstr(struct Dee_function_generator *__restrict self,
                                Dee_instruction_t const *instr);

/* Wrapper around `Dee_function_generator_geninstr()' to generate the entire basic block. */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_genall(struct Dee_function_generator *__restrict self);




/* Step #1: Load basic blocks. Fills in:
 * - self->fa_blockc
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

/* Step #2: Scan all basic blocks to determine the bound-flags of
 *          local variables at the start of every block. Fills in:
 * - self->fa_blockv[*]->bb_mem_start  (but only `ms_localc+ms_localv'; ms_stackv is left NULL/empty)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_loadboundlocals(struct Dee_function_assembler *__restrict self);

/* Step #3: Trim jumps whose origin has been determined to be unreachable.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF NONNULL((1)) int DCALL
Dee_function_assembler_trimunused(struct Dee_function_assembler *__restrict self);

/* Step #4: Compile basic blocks and determine memory states. Fills in:
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_stat
 * - self->fa_blockv[*]->bb_mem_start->* (everything not already done by `Dee_function_assembler_loadboundlocals()')
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

/* Step #5: Link blocks into an executable function blob.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_assembler_linkblocks(struct Dee_function_assembler *__restrict self,
                                  struct Dee_hostfunc *__restrict result);



/* Error throwing helper functions. */
INTDEF ATTR_COLD int DCALL err_illegal_stack_effect(void);
INTDEF ATTR_COLD int DCALL err_illegal_lid(uint16_t lid);
INTDEF ATTR_COLD int DCALL err_illegal_mid(uint16_t mid);
INTDEF ATTR_COLD int DCALL err_illegal_aid(uint16_t aid);
INTDEF ATTR_COLD int DCALL err_illegal_cid(uint16_t cid);
#define err_illegal_sid(sid) err_illegal_cid(sid)
INTDEF ATTR_COLD int DCALL err_illegal_rid(uint16_t rid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_illegal_gid(DeeModuleObject *__restrict mod, uint16_t gid);


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_LIBHOSTASM_H */
