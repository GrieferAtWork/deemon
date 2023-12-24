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
#ifndef GUARD_DEEMON_EXECUTE_CODE_C
#define GUARD_DEEMON_EXECUTE_CODE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/host.h>
#include <hybrid/unaligned.h>

#include <stdint.h>

#include "../objects/seq/svec.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifdef CONFIG_HAVE_PATHS_H
#include <paths.h> /* _PATH_DEVNULL */
#endif /* CONFIG_HAVE_PATHS_H */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */


#ifdef CONFIG_HAVE_EXEC_ALTSTACK

#ifndef __USER_LABEL_PREFIX__
#ifdef CONFIG_HOST_WINDOWS
#define __USER_LABEL_PREFIX__ _
#else /* CONFIG_HOST_WINDOWS */
#define __USER_LABEL_PREFIX__ /* nothing */
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !__USER_LABEL_PREFIX__ */


#undef EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc
#undef EXEC_ALTSTACK_ALLOC_USE_mmap
#undef EXEC_ALTSTACK_ALLOC_USE_malloc
#undef EXEC_ALTSTACK_ALLOC_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc
#elif (defined(CONFIG_HAVE_mmap) || defined(CONFIG_HAVE_mmap64)) && \
      (defined(CONFIG_HAVE_MAP_ANONYMOUS) || defined(CONFIG_HAVE_open))
#define EXEC_ALTSTACK_ALLOC_USE_mmap
#elif 1
#define EXEC_ALTSTACK_ALLOC_USE_malloc
#else /* ... */
#define EXEC_ALTSTACK_ALLOC_USE_STUB
#endif /* !... */


/* Figure out how we're going to implement the inline assembly portion. */
#undef EXEC_ALTSTACK_ASM_USE_EXTERNAL
#undef EXEC_ALTSTACK_ASM_USE_GCC
#undef EXEC_ALTSTACK_ASM_USE_MSVC
#undef EXEC_ALTSTACK_ASM_USE_STUB
#if defined(EXEC_ALTSTACK_ALLOC_USE_STUB)
#define EXEC_ALTSTACK_ASM_USE_STUB
#elif defined(_MSC_VER) && defined(__x86_64__)
#define EXEC_ALTSTACK_ASM_USE_EXTERNAL /* The x86_64+msvc version is implemented in `asm/altstack.ms-x64.S' */
#elif defined(__COMPILER_HAVE_GCC_ASM) && \
     (defined(__x86_64__) || defined(__i386__))
#define EXEC_ALTSTACK_ASM_USE_GCC
#elif defined(_MSC_VER) && defined(__i386__)
#define EXEC_ALTSTACK_ASM_USE_MSVC
#else /* ... */
#define EXEC_ALTSTACK_ASM_USE_STUB
#endif /* !... */

/* If we can't implement the assembly, no need to implement the allocators. */
#ifdef EXEC_ALTSTACK_ASM_USE_STUB
#undef EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc
#undef EXEC_ALTSTACK_ALLOC_USE_mmap
#undef EXEC_ALTSTACK_ALLOC_USE_malloc
#undef EXEC_ALTSTACK_ALLOC_USE_STUB
#define EXEC_ALTSTACK_ALLOC_USE_STUB
#endif /* EXEC_ALTSTACK_ASM_USE_STUB */


#ifdef EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc
#include <Windows.h>
#undef THIS
#endif /* !EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc */


/* When the exec-altstack functions are defined externally, we need
 * to make the stack allocator functions visible to them, so in that
 * case we must declare then as INTERN, rather than PRIVATE */
#ifdef EXEC_ALTSTACK_ASM_USE_EXTERNAL
#define STACK_ALLOCATOR_DECL INTERN
#else /* EXEC_ALTSTACK_ASM_USE_EXTERNAL */
#define STACK_ALLOCATOR_DECL PRIVATE
#endif /* !EXEC_ALTSTACK_ASM_USE_EXTERNAL */


/* Substitute some features. */
#ifdef EXEC_ALTSTACK_ALLOC_USE_mmap

#ifndef CONFIG_HAVE_MAP_PRIVATE
#define CONFIG_HAVE_MAP_PRIVATE
#define MAP_PRIVATE 0
#endif /* !CONFIG_HAVE_MAP_PRIVATE */

#ifndef CONFIG_HAVE_MAP_GROWSUP
#define CONFIG_HAVE_MAP_GROWSUP
#define MAP_GROWSUP 0
#endif /* !CONFIG_HAVE_MAP_GROWSUP */

#ifndef CONFIG_HAVE_MAP_GROWSDOWN
#define CONFIG_HAVE_MAP_GROWSDOWN
#define MAP_GROWSDOWN 0
#endif /* !CONFIG_HAVE_MAP_GROWSDOWN */

#ifndef CONFIG_HAVE_MAP_FILE
#define CONFIG_HAVE_MAP_FILE
#define MAP_FILE 0
#endif /* !CONFIG_HAVE_MAP_FILE */

#ifndef CONFIG_HAVE_MAP_STACK
#define CONFIG_HAVE_MAP_STACK
#define MAP_STACK 0
#endif /* !CONFIG_HAVE_MAP_STACK */

#ifndef CONFIG_HAVE_PROT_READ
#define CONFIG_HAVE_PROT_READ
#define PROT_READ 0
#endif /* !CONFIG_HAVE_PROT_READ */

#ifndef CONFIG_HAVE_PROT_WRITE
#define CONFIG_HAVE_PROT_WRITE
#define PROT_WRITE 0
#endif /* !CONFIG_HAVE_PROT_WRITE */

#ifndef CONFIG_HAVE_MAP_UNINITIALIZED
#define CONFIG_HAVE_MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif /* !CONFIG_HAVE_MAP_UNINITIALIZED */

#ifndef CONFIG_HAVE_mmap
#define CONFIG_HAVE_mmap
#define mmap mmap64
#endif /* !CONFIG_HAVE_mmap */

#ifdef __ARCH_STACK_GROWS_DOWNWARDS
#define MMAP_STACK_FLAGS MAP_GROWSDOWN
#else /* __ARCH_STACK_GROWS_DOWNWARDS */
#define MMAP_STACK_FLAGS MAP_GROWSUP
#endif /* !__ARCH_STACK_GROWS_DOWNWARDS */

#endif /* EXEC_ALTSTACK_ALLOC_USE_mmap */



DECL_BEGIN

/************************************************************************/
/* STACK ALLOCATOR FUNCTIONS                                            */
/************************************************************************/
#ifndef EXEC_ALTSTACK_ALLOC_USE_STUB

/* Figure out the error return value for `[try]alloc_altstack()' */
#ifdef EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc
#define ALTSTACK_ALLOC_FAILED NULL
#elif defined(EXEC_ALTSTACK_ALLOC_USE_mmap)
#ifdef MAP_FAILED
#define ALTSTACK_ALLOC_FAILED MAP_FAILED
#else /* MAP_FAILED */
#define ALTSTACK_ALLOC_FAILED ((void *)(uintptr_t)-1)
#endif /* !MAP_FAILED */
#elif defined(EXEC_ALTSTACK_ALLOC_USE_malloc)
#define ALTSTACK_ALLOC_FAILED NULL
#else /* ... */
#define ALTSTACK_ALLOC_FAILED NULL
#endif /* !... */

#ifndef _PATH_DEVNULL
#define _PATH_DEVNULL "/dev/null"
#endif /* !_PATH_DEVNULL */

LOCAL void *DCALL tryalloc_altstack(void) {
#ifdef EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc
	return VirtualAlloc(NULL,
	                    DEE_EXEC_ALTSTACK_SIZE,
	                    MEM_COMMIT | MEM_RESERVE,
	                    PAGE_READWRITE);
#endif /* EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc */

#ifdef EXEC_ALTSTACK_ALLOC_USE_mmap
#ifdef MAP_ANONYMOUS
	return mmap(NULL,
	            DEE_EXEC_ALTSTACK_SIZE,
	            PROT_READ | PROT_WRITE,
	            MAP_ANONYMOUS | MAP_PRIVATE |
	            MMAP_STACK_FLAGS | MAP_STACK |
	            MAP_UNINITIALIZED,
	            -1, 0);
#else /* MAP_ANONYMOUS */
	void *result;
	int fd = open(_PATH_DEVNULL, O_RDONLY);
	if unlikely(fd < 0)
		return ALTSTACK_ALLOC_FAILED;
	result = mmap(NULL,
	              DEE_EXEC_ALTSTACK_SIZE,
	              PROT_READ | PROT_WRITE,
	              MAP_FILE | MAP_PRIVATE |
	              MMAP_STACK_FLAGS | MAP_STACK,
	              fd, 0);
#ifdef CONFIG_HAVE_close
	(void)close(fd);
#endif /* CONFIG_HAVE_close */
	return result;
#endif /* !MAP_ANONYMOUS */
#endif /* EXEC_ALTSTACK_ALLOC_USE_mmap */

#ifdef EXEC_ALTSTACK_ALLOC_USE_malloc
	return Dee_TryMalloc(DEE_EXEC_ALTSTACK_SIZE);
#endif /* EXEC_ALTSTACK_ALLOC_USE_malloc */

#ifdef EXEC_ALTSTACK_ALLOC_USE_STUB
	return ALTSTACK_ALLOC_FAILED;
#endif /* EXEC_ALTSTACK_ALLOC_USE_STUB */
}

STACK_ALLOCATOR_DECL void DCALL free_altstack(void *stack) {
#ifdef EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc
	(void)VirtualFree(stack, /*DEE_EXEC_ALTSTACK_SIZE*/ 0, MEM_RELEASE);
#endif /* EXEC_ALTSTACK_ALLOC_USE_VirtualAlloc */

#ifdef EXEC_ALTSTACK_ALLOC_USE_mmap
#ifdef CONFIG_HAVE_munmap
	(void)munmap(stack, DEE_EXEC_ALTSTACK_SIZE);
#else /* CONFIG_HAVE_munmap */
	(void)stack;
#endif /* !CONFIG_HAVE_munmap */
#endif /* EXEC_ALTSTACK_ALLOC_USE_mmap */

#ifdef EXEC_ALTSTACK_ALLOC_USE_malloc
	Dee_Free(stack);
#endif /* EXEC_ALTSTACK_ALLOC_USE_malloc */

#ifdef EXEC_ALTSTACK_ALLOC_USE_STUB
	(void)stack;
#endif /* EXEC_ALTSTACK_ALLOC_USE_STUB */
}

STACK_ALLOCATOR_DECL void *DCALL alloc_altstack(void) {
	void *result;
again:
	result = tryalloc_altstack();
	if unlikely(result == ALTSTACK_ALLOC_FAILED) {
		if (DeeMem_ClearCaches((size_t)-1))
			goto again;
		Dee_BadAlloc(DEE_EXEC_ALTSTACK_SIZE);
	}
	return result;
}
#endif /* !EXEC_ALTSTACK_ALLOC_USE_STUB */






/************************************************************************/
/* ALT-STACK EXECUTION FUNCTIONS                                        */
/************************************************************************/
#ifndef EXEC_ALTSTACK_ASM_USE_EXTERNAL

/* GCC */
#ifdef EXEC_ALTSTACK_ASM_USE_GCC
#ifdef __x86_64__
#define WRAP_SYMBOL(s) PP_STR(__USER_LABEL_PREFIX__) #s
#define CALL_WITH_STACK(result, func, frame, new_stack)                     \
	__asm__("push{q} {%%rbp|rbp}\n\t"                                       \
	        "mov{q}  {%%rsp, %%rbp|rbp, rsp}\n\t"                           \
	        "lea{q}  {" PP_STR(DEE_EXEC_ALTSTACK_SIZE) "(%2), %%rsp|"       \
	                  "rsp, [%2 + " PP_STR(DEE_EXEC_ALTSTACK_SIZE) "]}\n\t" \
	        "call    " WRAP_SYMBOL(func) "\n\t"                             \
	        "mov{q}  {%%rbp, %%rsp|rsp, rbp}\n\t"                           \
	        "pop{q}  {%%rbp|rbp}\n\t"                                       \
	        : "=a" (result)                                                 \
	        : "c" (frame)                                                   \
	        , "r" (new_stack)                                               \
	        : "memory", "cc")
#elif defined(__i386__)
#ifdef CONFIG_HOST_WINDOWS
#define WRAP_SYMBOL(s) "@" #s "@4"
#else /* CONFIG_HOST_WINDOWS */
#define WRAP_SYMBOL(s) PP_STR(__USER_LABEL_PREFIX__) #s
#endif /* !CONFIG_HOST_WINDOWS */
#define CALL_WITH_STACK(result, func, frame, new_stack)                     \
	__asm__("push{l} {%%ebp|ebp}\n\t"                                       \
	        "mov{l}  {%%esp, %%ebp|ebp, esp}\n\t"                           \
	        "lea{l}  {" PP_STR(DEE_EXEC_ALTSTACK_SIZE) "(%2), %%esp|"       \
	                  "esp, [%2 + " PP_STR(DEE_EXEC_ALTSTACK_SIZE) "]}\n\t" \
	        "call    " WRAP_SYMBOL(func) "\n\t"                             \
	        "mov{l}  {%%ebp, %%esp|esp, ebp}\n\t"                           \
	        "pop{l}  {%%ebp|ebp}\n\t"                                       \
	        : "=a" (result)                                                 \
	        : "c" (frame)                                                   \
	        , "r" (new_stack)                                               \
	        : "memory", "cc")
#else /* Arch... */
#error "Unsupported Architecture (please check the `#define EXEC_ALTSTACK_ASM_USE_GCC 1' above)"
#endif /* !Arch... */
#endif /* EXEC_ALTSTACK_ASM_USE_GCC */

/* MSVC */
#ifdef EXEC_ALTSTACK_ASM_USE_MSVC
#ifdef __x86_64__
#define CALL_WITH_STACK(result, func, frame, new_stack) \
	__asm {                                             \
		__asm PUSH RBX                                  \
		__asm MOV  RAX, new_stack                       \
		__asm MOV  RCX, frame                           \
		__asm MOV  RBX, RSP                             \
		__asm LEA  RSP, [RAX + DEE_EXEC_ALTSTACK_SIZE]  \
		__asm CALL func                                 \
		__asm MOV  RSP, RBX                             \
		__asm POP  RBX                                  \
		__asm MOV  result, RAX                          \
	}
#elif defined(__i386__)
#define CALL_WITH_STACK(result, func, frame, new_stack) \
	__asm {                                             \
		__asm PUSH EBX                                  \
		__asm MOV  EAX, new_stack                       \
		__asm MOV  ECX, frame                           \
		__asm MOV  EBX, ESP                             \
		__asm LEA  ESP, [EAX + DEE_EXEC_ALTSTACK_SIZE]  \
		__asm CALL func                                 \
		__asm MOV  ESP, EBX                             \
		__asm POP  EBX                                  \
		__asm MOV  result, EAX                          \
	}
#else /* Arch... */
#error "Unsupported Architecture (please check the `#define EXEC_ALTSTACK_ASM_USE_MSVC 1' above)"
#endif /* !Arch... */
#endif /* EXEC_ALTSTACK_ASM_USE_MSVC */





/* Implement the actual altstack call functions. */
PUBLIC NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameFastAltStack(struct code_frame *__restrict frame) {
#ifdef EXEC_ALTSTACK_ASM_USE_STUB
	return DeeCode_ExecFrameFast(frame);
#else /* EXEC_ALTSTACK_ASM_USE_STUB */
	DREF DeeObject *result;
	void *new_stack = alloc_altstack();
	if unlikely(new_stack == ALTSTACK_ALLOC_FAILED)
		goto err;
	CALL_WITH_STACK(result, DeeCode_ExecFrameFast, frame, new_stack);
	free_altstack(new_stack);
	return result;
err:
	return NULL;
#endif /* !EXEC_ALTSTACK_ASM_USE_STUB */
}

PUBLIC NONNULL((1)) DREF DeeObject *ATTR_FASTCALL
DeeCode_ExecFrameSafeAltStack(struct code_frame *__restrict frame) {
#ifdef EXEC_ALTSTACK_ASM_USE_STUB
	return DeeCode_ExecFrameSafe(frame);
#else /* EXEC_ALTSTACK_ASM_USE_STUB */
	DREF DeeObject *result;
	void *new_stack = alloc_altstack();
	if unlikely(new_stack == ALTSTACK_ALLOC_FAILED)
		goto err;
	CALL_WITH_STACK(result, DeeCode_ExecFrameSafe, frame, new_stack);
	free_altstack(new_stack);
	return result;
err:
	return NULL;
#endif /* !EXEC_ALTSTACK_ASM_USE_STUB */
}
#endif /* !EXEC_ALTSTACK_ASM_USE_EXTERNAL */

DECL_END
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */



DECL_BEGIN

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
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeCode_HandleBreakpoint(struct code_frame *__restrict frame) {
	/* TODO: Add some sort of hook that allows for debugging. */
	(void)frame;
	return TRIGGER_BREAKPOINT_CONTINUE;
}



PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char *DCALL
DeeCode_GetASymbolName(DeeObject const *__restrict self, uint16_t aid) {
	/* Argument */
	DeeCodeObject const *me = (DeeCodeObject const *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeCode_Type);
	if (me->co_keywords && aid < me->co_argc_max)
		return DeeString_STR(me->co_keywords[aid]);
	return NULL;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char *DCALL
DeeCode_GetSSymbolName(DeeObject const *__restrict self, uint16_t sid) {
	/* Static symbol name */
	DeeDDIObject const *ddi;
	uint8_t const *reader;
	uint32_t offset;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeCode_Type);
	ddi = ((DeeCodeObject const *)self)->co_ddi;
	if (ddi->d_exdat) {
		reader = ddi->d_exdat->dx_data;
		for (;;) {
			uint8_t op = *reader++;
			switch (op) {

			case DDI_EXDAT_O_END:
				goto done_exdat;

			case DDI_EXDAT_O_SNAM | DDI_EXDAT_OP8:
				if (UNALIGNED_GETLE8(reader + 0) == sid) {
					offset = UNALIGNED_GETLE8(reader + 1);
					goto return_strtab_offset;
				}
				reader += 1 + 1;
				break;

			case DDI_EXDAT_O_SNAM | DDI_EXDAT_OP16:
				if (UNALIGNED_GETLE16(reader + 0) == sid) {
					offset = UNALIGNED_GETLE16(reader + 2);
					goto return_strtab_offset;
				}
				reader += 2 + 2;
				break;

			case DDI_EXDAT_O_SNAM | DDI_EXDAT_OP32:
				if (UNALIGNED_GETLE16(reader + 0) == sid) {
					offset = UNALIGNED_GETLE32(reader + 2);
					goto return_strtab_offset;
				}
				reader += 2 + 4;
				break;

			default:
				switch (op & DDI_EXDAT_OPMASK) {

				case DDI_EXDAT_OP8:
					reader += 1 + 1;
					break;

				case DDI_EXDAT_OP16:
					reader += 2 + 2;
					break;

				case DDI_EXDAT_OP32:
					reader += 2 + 4;
					break;

				default: break;
				}
				break;
			}
		}
	}
done_exdat:
	return NULL;
return_strtab_offset:
	return DeeString_STR(ddi->d_strtab) + offset;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char *DCALL
DeeCode_GetRSymbolName(DeeObject const *__restrict self, uint16_t rid) {
	/* Reference symbol name */
	DeeDDIObject const *ddi;
	uint8_t const *reader;
	uint32_t offset;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeCode_Type);
	ddi = ((DeeCodeObject const *)self)->co_ddi;
	if (ddi->d_exdat) {
		reader = ddi->d_exdat->dx_data;
		for (;;) {
			uint8_t op = *reader++;
			switch (op) {

			case DDI_EXDAT_O_END:
				goto done_exdat;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP8:
				if (UNALIGNED_GETLE8(reader + 0) == rid) {
					offset = UNALIGNED_GETLE8(reader + 1);
					goto return_strtab_offset;
				}
				reader += 1 + 1;
				break;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP16:
				if (UNALIGNED_GETLE16(reader + 0) == rid) {
					offset = UNALIGNED_GETLE16(reader + 2);
					goto return_strtab_offset;
				}
				reader += 2 + 2;
				break;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP32:
				if (UNALIGNED_GETLE16(reader + 0) == rid) {
					offset = UNALIGNED_GETLE32(reader + 2);
					goto return_strtab_offset;
				}
				reader += 2 + 4;
				break;

			default:
				switch (op & DDI_EXDAT_OPMASK) {

				case DDI_EXDAT_OP8:
					reader += 1 + 1;
					break;

				case DDI_EXDAT_OP16:
					reader += 2 + 2;
					break;

				case DDI_EXDAT_OP32:
					reader += 2 + 4;
					break;

				default: break;
				}
				break;
			}
		}
	}
done_exdat:
	return NULL;
return_strtab_offset:
	return DeeString_STR(ddi->d_strtab) + offset;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char *DCALL
DeeCode_GetDDIString(DeeObject const *__restrict self, uint16_t id) {
	/* DDI String */
	DeeDDIObject const *ddi;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeCode_Type);
	ddi = ((DeeCodeObject const *)self)->co_ddi;
	if (id < ddi->d_nstring)
		return DeeString_STR(ddi->d_strtab) + ddi->d_strings[id];
	return NULL;
}





/* Define the special `empty_code' object. */
#define empty_code empty_code_head.ob
INTERN DEFINE_CODE(empty_code_head,
                   /* co_flags:     */ CODE_FCOPYABLE,
                   /* co_localc:    */ 0,
                   /* co_staticc:   */ 0,
                   /* co_refc:      */ 0,
                   /* co_exceptc:   */ 0,
                   /* co_argc_min:  */ 0,
                   /* co_argc_max:  */ 0,
                   /* co_framesize: */ 0,
                   /* co_codebytes: */ sizeof(instruction_t),
                   /* co_module:    */ &DeeModule_Empty,
                   /* co_keywords:  */ NULL,
                   /* co_defaultv:  */ NULL,
                   /* co_staticv:   */ NULL,
                   /* co_exceptv:   */ NULL,
                   /* co_ddi:       */ &empty_ddi,
                   /* co_code:      */ { ASM_RET_NONE });




/* @return: 0: The code is not contained.
 * @return: 1: The code is contained.
 * @return: 2: The frame chain is incomplete, but code wasn't found thus far. */
PRIVATE WUNUSED NONNULL((3)) int DCALL
frame_chain_contains_code(struct code_frame *iter, uint16_t count,
                          DeeCodeObject *__restrict code) {
	while (count--) {
		if (!iter || iter == CODE_FRAME_NOT_EXECUTING)
			return 2;
		if (iter->cf_func->fo_code == code)
			return 1;
		iter = iter->cf_prev;
	}
	return 0;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeCode_SetAssembly(/*Code*/ DeeObject *__restrict self) {
	DeeCodeObject *me = (DeeCodeObject *)self;
	DeeThreadObject *caller;
	ASSERT_OBJECT_TYPE(self, &DeeCode_Type);

	/* Simple case: the assembly flag is already set. */
	if (me->co_flags & CODE_FASSEMBLY)
		return 0;
	caller = DeeThread_Self();

	/* Assume that the calling thread's execution chain
	 * is consistent (which it _really_ should be). */
	if (frame_chain_contains_code(caller->t_exec, caller->t_execsz, me))
		goto already_executing;

#ifndef CONFIG_NO_THREADS
	/* Here comes the dangerous part: Checking the other threads... */
	{
		DeeThreadObject *threads;
check_other_threads:
		/* Check for interrupts. */
		if (DeeThread_CheckInterruptSelf(caller))
			goto err;
#define WANT_err
		threads = DeeThread_SuspendAll();
		if unlikely(!threads)
			goto err;
#define WANT_err
		DeeThread_FOREACH(threads) {
			int temp;
			if (threads == caller)
				continue; /* Skip the calling thread. */
			temp = frame_chain_contains_code(threads->t_exec,
			                                 threads->t_execsz,
			                                 me);
			if (!temp)
				continue; /* Unused. */
			COMPILER_READ_BARRIER();

			/* Resume execution of all the other threads. */
			DeeThread_ResumeAll();

			/* Unclear, or is being used. */
			if (temp == 2) {
				/* Unclear. - Wait for the thread to turn its stack consistent, then try again. */
				if (DeeThread_CheckInterruptSelf(caller))
					goto err;
#define WANT_err
				DeeThread_SleepNoInt(100);
				goto check_other_threads;
			}

			/* The code object is currently being executed (or at least was being...) */
			goto already_executing;
		}

		/* Having confirmed that the code object isn't running, set the assembly
		 * flag before resuming all the other threads so we can still ensure that
		 * it will become visible as soon as the other threads start running again. */
		me->co_flags |= CODE_FASSEMBLY;
		COMPILER_WRITE_BARRIER(); /* Don't move the flag modification before this point. */
		DeeThread_ResumeAll();
	}
#else /* CONFIG_NO_THREADS */
	/* Simple case: Without any other threads to worry about, as well as the
	 *              fact that the caller isn't using the code object, we can
	 *              simply set the assembly flag and indicate success. */
	me->co_flags |= CODE_FASSEMBLY;
	COMPILER_WRITE_BARRIER();
#endif /* CONFIG_NO_THREADS */
	return 0;
already_executing:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot set assembly mode for code "
	                "object %k while it is being executed",
	                self);
#ifdef WANT_err
#undef WANT_err
err:
#endif /* WANT_err */
	return -1;
}


PRIVATE NONNULL((1)) void DCALL
code_fini(DeeCodeObject *__restrict self) {
	ASSERTF(!self->co_module ||
	        !DeeModule_Check(self->co_module) ||
	        self != self->co_module->mo_root ||
	        self->co_module->ob_refcnt == 0,
	        "Cannot destroy the root code object of a module");

	ASSERT(self->co_argc_max >= self->co_argc_min);
	/* Clear default argument objects. */
	if (self->co_argc_max != self->co_argc_min) {
		uint16_t count = self->co_argc_max - self->co_argc_min;
		Dee_XDecrefv(self->co_defaultv, count);
	}

	/* Clear static variables/constants. */
	Dee_Decrefv(self->co_staticv, self->co_staticc);

	/* Clear exception handlers. */
	{
		uint16_t i;
		for (i = 0; i < self->co_exceptc; ++i)
			Dee_XDecref(self->co_exceptv[i].eh_mask);
	}

	/* Clear debug information. */
	Dee_Decref(self->co_ddi);

	/* Clear keyword names. */
	if (self->co_keywords) {
		Dee_Decrefv(self->co_keywords, self->co_argc_max);
		Dee_Free((void *)self->co_keywords);
	}

	/* Clear module information. */
	Dee_XDecref(self->co_module);

	/* Free vectors. */
	Dee_Free((void *)self->co_defaultv);
	Dee_Free((void *)self->co_staticv);
	Dee_Free((void *)self->co_exceptv);
}

PRIVATE NONNULL((1, 2)) void DCALL
code_visit(DeeCodeObject *__restrict self,
           dvisit_t proc, void *arg) {
	size_t i;
	/* Visit the accompanying module.
	 * NOTE: We must use `Dee_XVisit()' here because the pointer
	 *       may still be NULL when it still represents the next
	 *       element in the chain of code objects associated with
	 *       the module currently being compiled. */
	Dee_XVisit(self->co_module);

	/* Visit default variables. */
	Dee_XVisitv(self->co_defaultv, (uint16_t)(self->co_argc_max - self->co_argc_min));

	/* Visit static variables. */
	DeeCode_StaticLockRead(self);
	Dee_Visitv(self->co_staticv, self->co_staticc);
	DeeCode_StaticLockEndRead(self);

	/* Visit exception information. */
	for (i = 0; i < self->co_exceptc; ++i)
		Dee_XVisit(self->co_exceptv[i].eh_mask);

	/* Visit debug information. */
	Dee_Visit(self->co_ddi);
}

PRIVATE NONNULL((1)) void DCALL
code_clear(DeeCodeObject *__restrict self) {
	DREF DeeObject *buffer[16];
	size_t i, bufi = 0;
	/* Clear out static variables. */
restart:
	DeeCode_StaticLockWrite(self);
	for (i = 0; i < self->co_staticc; ++i) {
		DREF DeeObject *ob = self->co_staticv[i];
		if (DeeNone_Check(ob) || DeeString_Check(ob))
			continue;
		if (DeeCode_Check(ob)) {
			/* Assembly may rely on certain constants being code objects!
			 * XXX: Speaking of which, what if this code is being executed right now?
			 *      After all: accessing constants normally doesn't require locks,
			 *      but us meddling with _all_ of them (and not just statics) breaks
			 *      some of the assumptions such code makes... */
			self->co_staticv[i] = (DeeObject *)&empty_code;
		} else {
			self->co_staticv[i] = Dee_None;
		}
		Dee_Incref(self->co_staticv[i]);
		if (!Dee_DecrefIfNotOne(ob)) {
			buffer[bufi] = ob; /* Inherit reference */
			++bufi;
			if (bufi >= COMPILER_LENOF(buffer)) {
				DeeCode_StaticLockEndWrite(self);
				Dee_Decrefv(buffer, bufi);
				bufi = 0;
				goto restart;
			}
		}
	}
	DeeCode_StaticLockEndWrite(self);
	Dee_Decrefv(buffer, bufi);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_kwds(DeeCodeObject *__restrict self) {
	ASSERT(self->co_argc_max >= self->co_argc_min);
	if (!self->co_keywords)
		return_empty_seq;
	return DeeRefVector_NewReadonly((DeeObject *)self,
	                                (size_t)self->co_argc_max,
	                                (DeeObject *const *)self->co_keywords);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_getdefault(DeeCodeObject *__restrict self) {
	ASSERT(self->co_argc_max >= self->co_argc_min);
	return DeeRefVector_NewReadonly((DeeObject *)self,
	                                (size_t)(self->co_argc_max - self->co_argc_min),
	                                self->co_defaultv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_getstatic(DeeCodeObject *__restrict self) {
	ASSERT(self->co_argc_max >= self->co_argc_min);
	return DeeRefVector_NewReadonly((DeeObject *)self,
	                                self->co_staticc,
	                                self->co_staticv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_name(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_doc);
	if (!info.fi_name)
		return_none;
	return (DREF DeeObject *)info.fi_name;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_doc(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	if (!info.fi_doc)
		return_none;
	return (DREF DeeObject *)info.fi_doc;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
code_get_type(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (!info.fi_type) {
		info.fi_type = (DREF DeeTypeObject *)Dee_None;
		Dee_Incref(Dee_None);
	}
	return info.fi_type;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_operator(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_opname == (uint16_t)-1)
		return_none;
	return DeeInt_NewUInt16(info.fi_opname);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_operatorname(DeeCodeObject *__restrict self) {
	struct function_info info;
	struct opinfo const *op;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_opname == (uint16_t)-1) {
		Dee_XDecref(info.fi_type);
		return_none;
	}
	op = Dee_OperatorInfo(Dee_TYPE(info.fi_type), info.fi_opname);
	Dee_XDecref(info.fi_type);
	if (!op)
		return DeeInt_NewUInt16(info.fi_opname);
	return DeeString_New(op->oi_sname);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_property(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	Dee_XDecref(info.fi_type);
	if (info.fi_getset == (uint16_t)-1)
		return_none;
	return DeeInt_NewUInt16(info.fi_getset);
err:
	return NULL;
}

PRIVATE struct type_member tpconst code_members[] = {
	TYPE_MEMBER_FIELD_DOC("__ddi__", STRUCT_OBJECT, offsetof(DeeCodeObject, co_ddi),
	                      "->?Ert:Ddi\n"
	                      "The DDI (DeemonDebugInformation) data block"),
	TYPE_MEMBER_FIELD_DOC(STR___module__, STRUCT_OBJECT, offsetof(DeeCodeObject, co_module),
	                      "->?DModule"),
	TYPE_MEMBER_FIELD_DOC("__argc_min__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_argc_min),
	                      "Min amount of arguments required to execute @this code"),
	TYPE_MEMBER_FIELD_DOC("__argc_max__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_argc_max),
	                      "Max amount of arguments accepted by @this code (excluding a varargs or varkwds argument)"),
	TYPE_MEMBER_BITFIELD_DOC("isyielding", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FYIELDING,
	                         "Check if @this code object is for a yield-function"),
	TYPE_MEMBER_BITFIELD_DOC("iscopyable", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FCOPYABLE,
	                         "Check if execution frames of @this code object can be copied"),
	TYPE_MEMBER_BITFIELD_DOC("hasassembly", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FASSEMBLY,
	                         "Check if assembly of @this code object is executed in safe-mode"),
	TYPE_MEMBER_BITFIELD_DOC("islenient", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FLENIENT,
	                         "Check if the runtime stack allocation allows for leniency"),
	TYPE_MEMBER_BITFIELD_DOC("hasvarargs", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FVARARGS,
	                         "Check if @this code object accepts variable arguments as overflow"),
	TYPE_MEMBER_BITFIELD_DOC("hasvarkwds", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FVARKWDS,
	                         "Check if @this code object accepts variable keyword arguments as overflow"),
	TYPE_MEMBER_BITFIELD_DOC("isthiscall", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FTHISCALL,
	                         "Check if @this code object requires a hidden leading this-argument"),
	TYPE_MEMBER_BITFIELD_DOC("hasheapframe", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FHEAPFRAME,
	                         "Check if the runtime stack-frame must be allocated on the heap"),
	TYPE_MEMBER_BITFIELD_DOC("hasfinally", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FFINALLY,
	                         "True if execution will jump to the nearest finally-block when a return instruction is encountered\n"
	                         "Note that this does not necessarily guaranty, or deny the presence of a try...finally statement in "
	                         "the user's source code, as the compiler may try to optimize this flag away to speed up runtime execution"),
	TYPE_MEMBER_BITFIELD_DOC("isconstructor", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FCONSTRUCTOR,
	                         "True for class constructor code objects. - When set, don't include the this-argument in "
	                         "tracebacks, thus preventing incomplete instances from being leaked when the constructor "
	                         "causes some sort of exception to be thrown"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst code_getsets[] = {
	/* General-purpose callable object RTTI interface implementation.
	 * These functions are mangled with leading/trailing underscores,
	 * as they also appear in other (standard-compliant) runtime types,
	 * where they must be kept hidden from the basic namespace provided
	 * for standard-compliant deemon code.
	 * Properties matching these names can be found in a variety of other
	 * types, including `Function', `ObjMethod', etc.
	 */
	TYPE_GETTER(STR___name__, &code_get_name,
	            "->?X2?Dstring?N\n"
	            "Returns the name of @this code object, or ?N if unknown (s.a. ?A__name__?DFunction)"),
	TYPE_GETTER(STR___doc__, &code_get_doc,
	            "->?X2?Dstring?N\n"
	            "Returns the documentation string of @this code object, or ?N if unknown (s.a. ?A__doc__?DFunction)"),
	TYPE_GETTER(STR___type__, &code_get_type,
	            "->?X2?DType?N\n"
	            "Try to determine if @this code object is defined as part of a user-defined class, "
	            /**/ "and if it is, return that class type, or ?N if that class couldn't be found, "
	            /**/ "or if @this code object is defined as stand-alone (s.a. ?A__type__?DFunction)"),
	TYPE_GETTER(STR___kwds__, &code_get_kwds,
	            "->?S?Dstring\n"
	            "Returns a sequence of keyword argument names accepted by @this code object\n"
	            "If @this code doesn't accept keyword arguments, an empty sequence is returned"),
	TYPE_GETTER("__operator__", &code_get_operator,
	            "->?X2?Dint?N\n"
	            "Try to determine if @this code object is defined as part of a user-defined class, "
	            /**/ "and if so, if it is used to define an operator callback. If that is the case, "
	            /**/ "return the internal ID of the operator that @this code object provides, or ?N "
	            /**/ "if that class couldn't be found, @this code object is defined as stand-alone, or "
	            /**/ "defined as a class- or instance-method (s.a. ?A__operator__?DFunction)"),
	TYPE_GETTER("__operatorname__", &code_get_operatorname,
	            "->?X3?Dstring?Dint?N\n"
	            "Same as ?#__operator__, but instead try to return the unambiguous name of the "
	            /**/ "operator, though still return its ID if the operator isn't recognized as being "
	            /**/ "part of the standard (s.a. ?A__operatorname__?DFunction)"),
	TYPE_GETTER("__property__", &code_get_property,
	            "->?X2?Dint?N\n"
	            "Returns an integer describing the kind if @this code is part of a property or getset, "
	            /**/ "or returns ?N if the function's property could not be found, or if the function isn't "
	            /**/ "declared as a property callback (s.a. ?A__property__?DFunction)"),
	TYPE_GETTER("__default__", &code_getdefault,
	            "->?S?O\n"
	            "Access to the default values of arguments"),
	/* Code-specific RTTI fields don't have leading/trailing underscores,
	 * because they don't need to match the ABI also provided by numerous
	 * other types (such as `Function', `ObjMethod', etc.) */
	TYPE_GETTER("statics", &code_getstatic,
	            "->?S?O\n"
	            "Access to the static values of @this code object"),
	TYPE_GETSET_END
};

PRIVATE struct type_gc tpconst code_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&code_clear
};

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
code_hash(DeeCodeObject *__restrict self) {
	dhash_t result;
	result = Dee_HashPtr(DeeObject_DATA(self),
	                     COMPILER_OFFSETAFTER(DeeCodeObject, co_codebytes) -
	                     sizeof(DeeObject));
	if (self->co_module)
		result = Dee_HashCombine(result, DeeObject_Hash((DeeObject *)self->co_module));
	if (self->co_keywords) {
		uint16_t i;
		for (i = 0; i < self->co_argc_max; ++i)
			result = Dee_HashCombine(result, DeeString_Hash((DeeObject *)self->co_keywords[i]));
	}
	if (self->co_defaultv) {
		uint16_t i;
		for (i = 0; i < (self->co_argc_max - self->co_argc_min); ++i) {
			if ((DeeObject *)self->co_defaultv[i])
				result = Dee_HashCombine(result, DeeObject_Hash((DeeObject *)self->co_defaultv[i]));
		}
	}
	if (self->co_staticv) {
		uint16_t i;
		for (i = 0; i < self->co_staticc; ++i) {
			DREF DeeObject *ob;
			DeeCode_StaticLockRead(self);
			ob = self->co_staticv[i];
			Dee_Incref(ob);
			DeeCode_StaticLockEndRead(self);
			result = Dee_HashCombine(result, DeeObject_Hash(ob));
			Dee_Decref(ob);
		}
	}
	if (self->co_exceptv) {
		uint16_t i;
		for (i = 0; i < self->co_exceptc; ++i) {
			dhash_t spec;
			spec = Dee_HashPtr(&self->co_exceptv[i].eh_start,
			                   sizeof(struct except_handler) -
			                   offsetof(struct except_handler, eh_start));
			result = Dee_HashCombine(result, spec);
			if (self->co_exceptv[i].eh_mask)
				result = Dee_HashCombine(result, DeeObject_Hash((DeeObject *)self->co_exceptv[i].eh_mask));
		}
	}
	result = Dee_HashCombine(result, DeeObject_Hash((DeeObject *)self->co_ddi));
	result = Dee_HashCombine(result, Dee_HashPtr(self->co_code, self->co_codebytes));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
code_eq_impl(DeeCodeObject *__restrict self,
             DeeCodeObject *__restrict other) {
	int temp;
	if (DeeObject_AssertTypeExact(other, &DeeCode_Type))
		goto err;
	if (self == other)
		return 1;
	if (self->co_flags != other->co_flags)
		goto nope;
	if (self->co_localc != other->co_localc)
		goto nope;
	if (self->co_staticc != other->co_staticc)
		goto nope;
	if (self->co_refc != other->co_refc)
		goto nope;
	if (self->co_exceptc != other->co_exceptc)
		goto nope;
	if (self->co_argc_min != other->co_argc_min)
		goto nope;
	if (self->co_argc_max != other->co_argc_max)
		goto nope;
	if (self->co_framesize != other->co_framesize)
		goto nope;
	if (self->co_codebytes != other->co_codebytes)
		goto nope;
	if (self->co_module != other->co_module)
		goto nope;
	if (self->co_keywords) {
		uint16_t i;
		if (!other->co_keywords)
			goto nope;
		for (i = 0; i < self->co_argc_max; ++i) {
			if (!DeeString_EqualsSTR(self->co_keywords[i],
			                          other->co_keywords[i]))
				goto nope;
		}
	} else if (other->co_keywords) {
		goto nope;
	}
	if (self->co_defaultv) {
		uint16_t i;
		for (i = 0; i < (self->co_argc_max - self->co_argc_min); ++i) {
			if (self->co_defaultv[i] == NULL) {
				if (other->co_defaultv[i] != NULL)
					goto nope;
			} else {
				if (other->co_defaultv[i] == NULL)
					goto nope;
				temp = DeeObject_CompareEq(self->co_defaultv[i],
				                           other->co_defaultv[i]);
				if (temp <= 0)
					goto err_temp;
			}
		}
	}
	if (self->co_staticv) {
		uint16_t i;
		for (i = 0; i < self->co_staticc; ++i) {
			DREF DeeObject *lhs, *rhs;
			DeeCode_StaticLockRead(self);
			lhs = self->co_staticv[i];
			Dee_Incref(lhs);
			DeeCode_StaticLockEndRead(self);
			DeeCode_StaticLockRead(other);
			rhs = other->co_staticv[i];
			Dee_Incref(rhs);
			DeeCode_StaticLockEndRead(other);
			temp = DeeObject_CompareEq(lhs, rhs);
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			if (temp <= 0)
				goto err_temp;
		}
	}
	if (self->co_exceptv) {
		uint16_t i;
		for (i = 0; i < self->co_exceptc; ++i) {
			if (self->co_exceptv[i].eh_start != other->co_exceptv[i].eh_start)
				goto nope;
			if (self->co_exceptv[i].eh_end != other->co_exceptv[i].eh_end)
				goto nope;
			if (self->co_exceptv[i].eh_addr != other->co_exceptv[i].eh_addr)
				goto nope;
			if (self->co_exceptv[i].eh_stack != other->co_exceptv[i].eh_stack)
				goto nope;
			if (self->co_exceptv[i].eh_flags != other->co_exceptv[i].eh_flags)
				goto nope;
			if (self->co_exceptv[i].eh_mask != other->co_exceptv[i].eh_mask)
				goto nope;
		}
	}
	temp = DeeObject_CompareEq((DeeObject *)self->co_ddi,
	                           (DeeObject *)other->co_ddi);
	if (temp <= 0)
		goto err_temp;
	if (bcmp(self->co_code, other->co_code, self->co_codebytes) != 0)
		goto nope;
	return 1;
err_temp:
	return temp;
nope:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
code_eq(DeeCodeObject *self,
        DeeCodeObject *other) {
	int result;
	result = code_eq_impl(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
code_ne(DeeCodeObject *self,
        DeeCodeObject *other) {
	int result;
	result = code_eq_impl(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result == 0);
err:
	return NULL;
}

PRIVATE struct type_cmp code_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&code_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&code_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&code_ne
};

PRIVATE WUNUSED DREF DeeObject *DCALL code_ctor(void) {
	return_reference_((DeeObject *)&empty_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
code_copy(DeeCodeObject *__restrict self) {
	DREF DeeCodeObject *result;
	result = (DREF DeeCodeObject *)DeeGCObject_Malloc(offsetof(DeeCodeObject, co_code) +
	                                                  self->co_codebytes);
	if unlikely(!result)
		goto done;
	memcpy(result, self, offsetof(DeeCodeObject, co_code) + self->co_codebytes);
	Dee_atomic_rwlock_init(&result->co_static_lock);
	if (result->co_keywords) {
		if (!result->co_argc_max) {
			result->co_keywords = NULL;
		} else {
			result->co_keywords = (DREF DeeStringObject **)Dee_Mallocc(result->co_argc_max,
			                                                           sizeof(DREF DeeStringObject *));
			if unlikely(!result->co_keywords)
				goto err_r;
			Dee_Movrefv((DREF DeeObject **)result->co_keywords,
			            self->co_keywords, result->co_argc_max);
		}
	}
	ASSERT(result->co_argc_max >= result->co_argc_min);
	ASSERT((result->co_defaultv == NULL) ==
	       (result->co_argc_max == result->co_argc_min));
	if (result->co_defaultv) {
		uint16_t n          = result->co_argc_max - result->co_argc_min;
		result->co_defaultv = (DREF DeeObject **)Dee_Mallocc(n, sizeof(DREF DeeObject *));
		if unlikely(!result->co_defaultv)
			goto err_r_keywords;
		Dee_XMovrefv((DREF DeeObject **)result->co_defaultv, self->co_defaultv, n);
	}
	ASSERT((result->co_staticc != 0) ==
	       (result->co_staticv != NULL));
	if (result->co_staticv) {
		result->co_staticv = (DREF DeeObject **)Dee_Mallocc(result->co_staticc,
		                                                    sizeof(DREF DeeObject *));
		if unlikely(!result->co_staticv)
			goto err_r_default;
		DeeCode_StaticLockRead(self);
		Dee_Movrefv(result->co_staticv, self->co_staticv, result->co_staticc);
		DeeCode_StaticLockEndRead(self);
	}
	if (!result->co_exceptc) {
		result->co_exceptv = NULL;
	} else {
		uint16_t i;
		result->co_exceptv = (struct except_handler *)Dee_Mallocc(result->co_exceptc,
		                                                          sizeof(struct except_handler));
		if unlikely(!result->co_exceptv)
			goto err_r_static;
		memcpyc(result->co_exceptv,
		        self->co_exceptv,
		        result->co_exceptc,
		        sizeof(struct except_handler));
		for (i = 0; i < result->co_exceptc; ++i)
			Dee_XIncref(result->co_exceptv[i].eh_mask);
	}

	Dee_XIncref(result->co_ddi);
	Dee_XIncref(result->co_module);
	DeeObject_Init(result, &DeeCode_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return result;
err_r_static:
	if (result->co_staticv) {
		Dee_Decrefv(result->co_staticv, result->co_staticc);
		Dee_Free(result->co_staticv);
	}
err_r_default:
	if (result->co_defaultv) {
		uint16_t n = result->co_argc_max - result->co_argc_min;
		Dee_XDecrefv(result->co_defaultv, n);
		Dee_Free((void *)result->co_defaultv);
	}
err_r_keywords:
	if (result->co_keywords) {
		Dee_Decrefv(result->co_keywords, result->co_argc_max);
		Dee_Free((void *)result->co_keywords);
	}
err_r:
	DeeGCObject_Free(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
code_deepcopy(DeeCodeObject *__restrict self) {
	DREF DeeCodeObject *result;
	uint16_t i;
	result = code_copy(self);
	if unlikely(!result)
		goto done;
	if (result->co_defaultv) {
		uint16_t n = result->co_argc_max - result->co_argc_min;
		for (i = 0; i < n; ++i) {
			if (DeeObject_XInplaceDeepCopy((DeeObject **)&result->co_defaultv[i]))
				goto err_r;
		}
	}
	if (result->co_staticv) {
		for (i = 0; i < result->co_staticc; ++i) {
			if (DeeObject_InplaceDeepCopyWithLock((DeeObject **)&result->co_staticv[i],
			                                      &result->co_static_lock))
				goto err_r;
		}
	}
#if 0 /* Debug information wouldn't change... */
	if (DeeObject_InplaceDeepCopy((DeeObject **)&result->co_ddi))
		goto err_r;
#endif
#if 0 /* Types are singletons. */
	if (result->co_exceptv) {
		for (i = 0; i < result->co_exceptc; ++i) {
			if (DeeObject_XInplaceDeepCopy((DeeObject **)&result->co_exceptv[i].eh_mask))
				goto err_r;
		}
	}
#endif
#if 0 /* Modules are singletons! - If they weren't, we'd be corrupting their filesystem namespace... */
	if (DeeObject_XInplaceDeepCopy((DeeObject **)&result->co_module))
		goto err_r;
#endif
done:
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

struct except_flag {
	char     ef_name[14]; /* Flag name. */
	uint16_t ef_flag;     /* Flag value. */
};

struct code_flag {
	char     cf_name[14]; /* Flag name. */
	uint16_t cf_flag;     /* Flag value. */
};

PRIVATE struct except_flag const except_flags_db[] = {
	{ "finally", EXCEPTION_HANDLER_FFINALLY },
	{ "interrupt", EXCEPTION_HANDLER_FINTERPT },
	{ "handled", EXCEPTION_HANDLER_FHANDLED },
};

PRIVATE struct code_flag const code_flags_db[] = {
	{ "yielding", CODE_FYIELDING },
	{ "copyable", CODE_FCOPYABLE },
	{ "assembly", CODE_FASSEMBLY },
	{ "lenient", CODE_FLENIENT },
	{ "varargs", CODE_FVARARGS },
	{ "varkwds", CODE_FVARKWDS },
	{ "thiscall", CODE_FTHISCALL },
	{ "heapframe", CODE_FHEAPFRAME },
	{ "finally", CODE_FFINALLY },
	{ "constructor", CODE_FCONSTRUCTOR },
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
unpack_exception_descriptor(struct except_handler *__restrict self,
                            DeeObject *__restrict desc) {
	DeeObject *flags = Dee_None;
	self->eh_mask    = NULL;
	if (Dee_Unpackf(desc,
	                "("
	                UNPuN(DEE_SIZEOF_CODE_ADDR_T)
	                UNPuN(DEE_SIZEOF_CODE_ADDR_T)
	                UNPuN(DEE_SIZEOF_CODE_ADDR_T)
	                UNPu16
	                "|oo)",
	                &self->eh_start,
	                &self->eh_end,
	                &self->eh_addr,
	                &self->eh_stack,
	                &flags,
	                &self->eh_mask))
		goto err;
	if (DeeNone_Check(self->eh_mask))
		self->eh_mask = NULL;
	if (self->eh_mask && DeeObject_AssertType(self->eh_mask, &DeeType_Type))
		goto err;
	self->eh_flags = EXCEPTION_HANDLER_FNORMAL;
	if (!DeeNone_Check(flags)) {
		if (DeeString_Check(flags)) {
			char const *s = DeeString_STR(flags);
			while (*s) {
				char const *next = strchr(s, ',');
				size_t i, len = next ? (size_t)(next - s) : strlen(s);
				if likely(len < COMPILER_LENOF(except_flags_db[0].ef_name)) {
					for (i = 0; i < COMPILER_LENOF(except_flags_db); ++i) {
						if (except_flags_db[i].ef_name[len] == '\0' &&
						    bcmpc(except_flags_db[i].ef_name, s, len, sizeof(char)) == 0) {
							self->eh_flags |= except_flags_db[i].ef_flag;
							goto got_flag;
						}
					}
					if unlikely(!len)
						goto got_flag;
				}
				DeeError_Throwf(&DeeError_ValueError,
				                "Unknown exception handler flag: %$q",
				                (size_t)len, s);
				goto err;
got_flag:
				if (!next)
					break;
				s = next + 1;
			}
		} else {
			if (DeeObject_AsUInt16(flags, &self->eh_flags))
				goto err;
#if EXCEPTION_HANDLER_FMASK != 0xffff
			if (self->eh_flags & ~EXCEPTION_HANDLER_FMASK) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid bits enabled in exception handler flags %#I16x",
				                self->eh_flags);
				goto err;
			}
#endif /* EXCEPTION_HANDLER_FMASK != 0xffff */
		}
	}
	Dee_XIncref(self->eh_mask);
	return 0;
err:
	return -1;
}




PRIVATE WUNUSED DREF DeeCodeObject *DCALL
code_init_kw(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeCodeObject *result;
	DeeObject *flags        = Dee_None;
	DeeObject *except       = Dee_None;
	DeeObject *statics      = Dee_None;
	DeeObject *text         = Dee_None;
	DeeObject *keywords     = Dee_None;
	DeeObject *defaults     = Dee_None;
	DeeModuleObject *module = (DeeModuleObject *)Dee_None;
	DeeDDIObject *ddi       = (DeeDDIObject *)Dee_None;
	uint16_t nlocal         = 0;
	uint16_t nstack         = 0;
	uint16_t refc           = 0;
	uint16_t coargc         = 0;
	DeeBuffer text_buf;
	/* (text:?DBytes=!N,module:?DModule=!N,statics:?S?O=!N,
	 *  except:?S?X2?T5?Dint?Dint?Dint?Dint?X2?Dstring?Dint?T6?Dint?Dint?Dint?Dint?X2?Dstring?Dint?DType=!N,
	 *  nlocal=!0,nstack=!0,refc=!0,argc=!0,keywords:?S?Dstring=!N,defaults:?S?O=!N,
	 *  flags:?X2?Dstring?Dint=!P{lenient},ddi:?Ert:Ddi=!N) */
	PRIVATE DEFINE_KWLIST(kwlist, { K(text),
	                                K(module),
	                                K(statics),
	                                K(except),
	                                K(nlocal),
	                                K(nstack),
	                                K(refc),
	                                K(argc),
	                                K(keywords),
	                                K(defaults),
	                                K(flags),
	                                K(ddi),
	                                KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "|"
	                    "o"    /* text */
	                    "o"    /* module */
	                    "o"    /* statics */
	                    "o"    /* except */
	                    "I16u" /* nlocal */
	                    "I16u" /* nstack */
	                    "I16u" /* refc */
	                    "I16u" /* argc */
	                    "o"    /* keywords */
	                    "o"    /* defaults */
	                    "o"    /* flags */
	                    "o"    /* ddi */
	                    ":_Code",
	                    &text,
	                    &module,
	                    &statics,
	                    &except,
	                    &nlocal,
	                    &nstack,
	                    &refc,
	                    &coargc,
	                    &keywords,
	                    &defaults,
	                    &flags,
	                    &ddi))
		goto err;
	if (DeeNone_Check(flags))
		flags = Dee_EmptyString;
	if (DeeObject_GetBuf(text, &text_buf, Dee_BUFFER_FREADONLY))
		goto err;
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(text_buf.bb_size > (code_size_t)-1) {
		err_integer_overflow_i(32, true);
		goto err_buf;
	}
#endif /* __SIZEOF_SIZE_T__ > 4 */
	result = (DREF DeeCodeObject *)DeeGCObject_Malloc(offsetof(DeeCodeObject, co_code) +
	                                                  text_buf.bb_size + INSTRLEN_MAX);
	if unlikely(!result)
		goto err_buf;
	/* Copy text bytes. */
	result->co_codebytes = (code_size_t)text_buf.bb_size;
	{
		instruction_t *endp;
		endp = (instruction_t *)mempcpyc(result->co_code, text_buf.bb_base,
		                                 text_buf.bb_size, sizeof(instruction_t));
#if ASM_RET_NONE == 0
		bzero(endp, INSTRLEN_MAX);
#else /* ASM_RET_NONE == 0 */
		memset(endp, ASM_RET_NONE, INSTRLEN_MAX);
#endif /* ASM_RET_NONE != 0 */
	}
	DeeObject_PutBuf(text, &text_buf, Dee_BUFFER_FREADONLY);
	/* Load keyword arguments */
	result->co_keywords = NULL;
	if (!DeeNone_Check(keywords)) {
		DREF DeeStringObject **keyword_vec;
		uint16_t i;
		if (!coargc) {
			/* Automatically determine the argument count. */
			size_t keyword_count;
			keyword_vec = (DREF DeeStringObject **)DeeSeq_AsHeapVector(keywords,
			                                                           &keyword_count);
			if unlikely(!keyword_vec)
				goto err_r;
			if unlikely(keyword_count > (uint16_t)-1) {
				DeeError_Throwf(&DeeError_IntegerOverflow,
				                "Too many arguments %" PRFuSIZ " for when at most 0xffff can be used",
				                keyword_count);
				goto err_r;
			}
			coargc = (uint16_t)keyword_count;
		} else {
			keyword_vec = (DREF DeeStringObject **)Dee_Mallocc(coargc,
			                                                   sizeof(DREF DeeStringObject *));
			if unlikely(!keyword_vec)
				goto err_r;
			if unlikely(DeeObject_Unpack(keywords, coargc, (DeeObject **)keyword_vec)) {
				Dee_Free(keyword_vec);
				goto err_r;
			}
		}
		result->co_keywords = keyword_vec; /* Inherit */
		/* Ensure that all elements are strings. */
		for (i = 0; i < coargc; ++i) {
			if (DeeObject_AssertTypeExact(keyword_vec[i], &DeeString_Type))
				goto err_r_keywords;
		}
	}

	result->co_argc_min = coargc;
	result->co_argc_max = coargc;
	result->co_defaultv = NULL;
	/* Load default arguments */
	if (!DeeNone_Check(defaults)) {
		size_t i, default_c;
		DREF DeeObject **default_vec;
		default_c = DeeObject_Size(defaults);
		if unlikely(default_c == (size_t)-1)
			goto err_r_keywords;
		if unlikely(default_c > coargc) {
			DeeError_Throwf(&DeeError_IntegerOverflow,
			                "Too many default arguments (%" PRFuSIZ ") for "
			                "code only taking %" PRFu16 " arguments at most",
			                default_c, coargc);
			goto err_r_keywords;
		}
		default_vec = (DREF DeeObject **)Dee_Mallocc(default_c, sizeof(DREF DeeObject *));
		if unlikely(!default_vec)
			goto err_r_keywords;
		for (i = 0; i < default_c; ++i) {
			DREF DeeObject *elem;
			elem = DeeObject_GetItemIndex(defaults, i);
			if (elem) {
				default_vec[i] = elem; /* Inherit reference */
			} else if (DeeError_Catch(&DeeError_UnboundItem)) {
				default_vec[i] = NULL; /* Optional argument */
			} else {
				Dee_XDecrefv(default_vec, i);
				Dee_Free(default_vec);
				goto err_r_keywords;
			}
		}
		result->co_defaultv = default_vec;
		result->co_argc_min = (uint16_t)(coargc - (uint16_t)default_c);
	}
	result->co_staticc = 0;
	result->co_staticv = NULL;
	if (!DeeNone_Check(statics)) {
		DREF DeeObject **static_vec;
		size_t static_cnt;
		static_vec = DeeSeq_AsHeapVector(statics, &static_cnt);
		if unlikely(!static_vec)
			goto err_r_default_v;
		if unlikely(static_cnt > (uint16_t)-1) {
			while (static_cnt--)
				Dee_Decref(static_vec[static_cnt]);
			Dee_Free(static_vec);
			err_integer_overflow_i(16, true);
			goto err_r_default_v;
		}
		result->co_staticc = (uint16_t)static_cnt;
		result->co_staticv = static_vec;
	}
	if (DeeNone_Check(module)) {
		DeeThreadObject *ts = DeeThread_Self();
		if unlikely(!ts->t_execsz) {
			DeeError_Throwf(&DeeError_TypeError,
			                "No module given, when the current "
			                "module could not be determined");
			goto err_r_statics;
		}
		ASSERT(ts->t_exec);
		ASSERT(ts->t_exec->cf_func);
		ASSERT(ts->t_exec->cf_func->fo_code);
		ASSERT(ts->t_exec->cf_func->fo_code->co_module);
		module = ts->t_exec->cf_func->fo_code->co_module;
	}
	/* NOTE: Always check this, so prevent stuff like interactive
	 *       modules to leaking into generic code objects. */
	if (DeeObject_AssertTypeExact(module, &DeeModule_Type))
		goto err_r_statics;

	/* Generate exception handlers. */
	result->co_exceptc = 0;
	result->co_exceptv = NULL;
	if (!DeeNone_Check(except)) {
		uint16_t except_c               = 0;
		uint16_t except_a               = 0;
		struct except_handler *except_v = NULL;
		struct except_handler *new_except_v;
		DREF DeeObject *iter, *elem;
		iter = DeeObject_IterSelf(except);
		if unlikely(!iter)
			goto err_r_statics;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			ASSERT(except_c <= except_a);
			if (except_c >= except_a) {
				uint16_t new_except_a = except_a * 2;
				if (!except_a) {
					new_except_a = 2;
				} else if unlikely(new_except_a <= except_a) {
					if unlikely(except_a == (uint16_t)-1) {
						DeeError_Throwf(&DeeError_IntegerOverflow,
						                "Too many exception handlers");
err_r_except_temp_iter_elem:
						Dee_Decref(elem);
err_r_except_temp_iter:
						Dee_Decref(iter);
						while (except_c--)
							Dee_XDecref(except_v[except_c].eh_mask);
						Dee_Free(except_v);
						goto err_r_statics;
					}
					new_except_a = (uint16_t)-1;
				}
				new_except_v = (struct except_handler *)Dee_TryReallocc(except_v, new_except_a,
				                                                        sizeof(struct except_handler));
				if unlikely(!new_except_v) {
					new_except_a = except_c + 1;
					new_except_v = (struct except_handler *)Dee_Reallocc(except_v, new_except_a,
					                                                     sizeof(struct except_handler));
					if unlikely(!new_except_v)
						goto err_r_except_temp_iter_elem;
				}
				except_v = new_except_v;
				except_a = new_except_a;
			}
			if unlikely(unpack_exception_descriptor(&except_v[except_c], elem))
				goto err_r_except_temp_iter_elem;
			Dee_Decref(elem);
			++except_c;
		}
		if unlikely(!elem)
			goto err_r_except_temp_iter;
		Dee_Decref(iter);
		if (except_a > except_c) {
			new_except_v = (struct except_handler *)Dee_TryReallocc(except_v, except_c,
			                                                        sizeof(struct except_handler));
			if likely(new_except_v)
				except_v = new_except_v;
		}
		result->co_exceptc = except_c;
		result->co_exceptv = except_v;
	}

	/* Load custom code flags */
	result->co_flags = CODE_FASSEMBLY;
	if (!DeeNone_Check(flags)) {
		if (DeeString_Check(flags)) {
			char const *s = DeeString_STR(flags);
			while (*s) {
				char const *next = strchr(s, ',');
				size_t i, len = next ? (size_t)(next - s) : strlen(s);
				if likely(len < COMPILER_LENOF(code_flags_db[0].cf_name)) {
					for (i = 0; i < COMPILER_LENOF(code_flags_db); ++i) {
						if (code_flags_db[i].cf_name[len] == '\0' &&
						    bcmpc(code_flags_db[i].cf_name, s, len, sizeof(char)) == 0) {
							result->co_flags |= code_flags_db[i].cf_flag;
							goto got_flag;
						}
					}
					if unlikely(!len)
						goto got_flag;
				}
				DeeError_Throwf(&DeeError_ValueError,
				                "Unknown code flag: %$q",
				                (size_t)len, s);
				goto err_r_except;
got_flag:
				if (!next)
					break;
				s = next + 1;
			}
		} else {
			if (DeeObject_AsUInt16(flags, &result->co_flags))
				goto err_r_except;
#if CODE_FMASK != 0xffff
			if (result->co_flags & ~CODE_FMASK) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid bits enabled in code flags %#I16x",
				                result->co_flags);
				goto err_r_except;
			}
#endif /* CODE_FMASK != 0xffff */
			result->co_flags |= CODE_FASSEMBLY;
		}
	}
	if (DeeNone_Check(ddi)) {
		ddi = &empty_ddi;
	} else {
		if (DeeObject_AssertTypeExact(ddi, &DeeDDI_Type))
			goto err_r_except;
	}

	/* Fill in remaining fields. */
	result->co_ddi = ddi;
	Dee_Incref(ddi);
	result->co_module = module;
	Dee_Incref(module);
	result->co_localc    = nlocal;
	result->co_refc      = refc;
	result->co_framesize = (nlocal + nstack) * sizeof(DREF DeeObject *);
	if (result->co_framesize > CODE_LARGEFRAME_THRESHOLD)
		result->co_flags |= CODE_FHEAPFRAME;
	Dee_atomic_rwlock_init(&result->co_static_lock);

	/* Initialize the new code object, and start tracking it. */
	DeeObject_Init(result, &DeeCode_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_r_except:
	if (result->co_exceptv) {
		while (result->co_exceptc--) {
			Dee_XDecref(result->co_exceptv[result->co_exceptc].eh_mask);
		}
		Dee_Free((void *)result->co_exceptv);
	}
err_r_statics:
	if (result->co_staticv) {
		Dee_Decrefv(result->co_staticv, result->co_staticc);
		Dee_Free((void *)result->co_staticv);
	}
err_r_default_v:
	if (result->co_defaultv) {
		result->co_argc_max -= result->co_argc_min;
		Dee_XDecrefv(result->co_defaultv, result->co_argc_max);
		Dee_Free((void *)result->co_defaultv);
	}
err_r_keywords:
	if (result->co_keywords) {
		Dee_Decrefv(result->co_keywords, result->co_argc_max);
		Dee_Free((void *)result->co_keywords);
	}
err_r:
	DeeGCObject_Free(result);
	goto err;
err_buf:
	DeeObject_PutBuf(text, &text_buf, Dee_BUFFER_FREADONLY);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
code_print(DeeCodeObject *__restrict self,
           dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *name = code_get_name(self);
	if unlikely(!name)
		goto err;
	if (DeeNone_Check(name)) {
		result = DeeFormat_PRINT(printer, arg, "<code for <anonymous>>");
	} else {
		result = DeeFormat_Printf(printer, arg, "<code for %r>", name);
	}
	Dee_Decref(name);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
code_printrepr(DeeCodeObject *__restrict self,
               dformatprinter printer, void *arg) {
#define DO(x)                         \
	do {                              \
		if unlikely((temp = (x)) < 0) \
			goto err;                 \
		result += temp;               \
	}	__WHILE0
	dssize_t temp, result;
	result = DeeFormat_Printf(printer, arg, "Code(text: { ");
	if unlikely(result < 0)
		goto done;
	{
		code_size_t i;
		for (i = 0; i < self->co_codebytes; ++i) {
			if (i != 0)
				DO(DeeFormat_PRINT(printer, arg, ", "));
			DO(DeeFormat_Printf(printer, arg, "%#" PRFx8, self->co_code[i]));
		}
	}
	if (self->co_codebytes != 0)
		DO(DeeFormat_PRINT(printer, arg, " "));
	DO(DeeFormat_Printf(printer, arg,
	                    "}, module: %r",
	                    self->co_module));

	if (self->co_staticc > 0) {
		uint16_t i;
		DO(DeeFormat_PRINT(printer, arg, ", statics: { "));
		for (i = 0; i < self->co_staticc; ++i) {
			DREF DeeObject *ob;
			if (i != 0)
				DO(DeeFormat_PRINT(printer, arg, ", "));
			DeeCode_StaticLockRead(self);
			ob = self->co_staticv[i];
			Dee_Incref(ob);
			DeeCode_StaticLockEndRead(self);
			temp = DeeFormat_PrintObjectRepr(printer, arg, ob);
			Dee_Decref_unlikely(ob);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		DO(DeeFormat_PRINT(printer, arg, " }"));
	}
	if (self->co_exceptc > 0) {
		uint16_t i;
		DO(DeeFormat_PRINT(printer, arg, ", except: { "));
		for (i = 0; i < self->co_exceptc; ++i) {
			struct except_handler const *hand;
			if (i != 0)
				DO(DeeFormat_PRINT(printer, arg, ", "));
			hand = &self->co_exceptv[i];
			DO(DeeFormat_Printf(printer, arg,
			                    "("
			                    "%#" PRFxN(DEE_SIZEOF_CODE_ADDR_T) ", "
			                    "%#" PRFxN(DEE_SIZEOF_CODE_ADDR_T) ", "
			                    "%#" PRFxN(DEE_SIZEOF_CODE_ADDR_T) ", "
			                    "%" PRFu16,
			                    hand->eh_start,
			                    hand->eh_end,
			                    hand->eh_addr,
			                    hand->eh_stack));
			if (hand->eh_mask || hand->eh_flags) {
				DO(DeeFormat_PRINT(printer, arg, ", "));
				if (hand->eh_flags & ~(EXCEPTION_HANDLER_FFINALLY |
				                       EXCEPTION_HANDLER_FINTERPT |
				                       EXCEPTION_HANDLER_FHANDLED)) {
					DO(DeeFormat_Printf(printer, arg, "%#" PRFx16, hand->eh_flags));
				} else {
					unsigned int flag_i;
					bool is_first = true;
					DO(DeeFormat_PRINT(printer, arg, "\""));
					for (flag_i = 0; flag_i < COMPILER_LENOF(except_flags_db); ++flag_i) {
						if (!(hand->eh_flags & except_flags_db[flag_i].ef_flag))
							continue;
						if (!is_first)
							DO(DeeFormat_PRINT(printer, arg, ","));
						DO(DeeFormat_PrintStr(printer, arg, except_flags_db[flag_i].ef_name));
						is_first = false;
					}
					DO(DeeFormat_PRINT(printer, arg, "\""));
				}
				if (hand->eh_mask)
					DO(DeeFormat_Printf(printer, arg, ", %r", hand->eh_mask));
			}
			DO(DeeFormat_PRINT(printer, arg, ")"));
		}
		DO(DeeFormat_PRINT(printer, arg, " }"));
	}
	if (self->co_localc > 0)
		DO(DeeFormat_Printf(printer, arg, ", nlocal: %" PRFu16, self->co_localc));
	{
		uint16_t nstack;
		nstack = (uint16_t)((self->co_framesize / sizeof(DREF DeeObject *)) - self->co_localc);
		if (nstack > 0)
			DO(DeeFormat_Printf(printer, arg, ", nstack: %" PRFu16, nstack));
	}
	if (self->co_refc > 0)
		DO(DeeFormat_Printf(printer, arg, ", refc: %" PRFu16, self->co_refc));
	if (self->co_argc_max > 0)
		DO(DeeFormat_Printf(printer, arg, ", argc: %" PRFu16, self->co_argc_max));
	if (self->co_keywords != NULL) {
		uint16_t i;
		DO(DeeFormat_PRINT(printer, arg, ", keywords: { "));
		for (i = 0; i < self->co_argc_max; ++i) {
			if (i != 0)
				DO(DeeFormat_PRINT(printer, arg, ", "));
			DO(DeeString_PrintUtf8((DeeObject *)self->co_keywords[i], printer, arg));
		}
		DO(DeeFormat_PRINT(printer, arg, " }"));
	}
	if (self->co_argc_min < self->co_argc_max) {
		uint16_t i, defaultc = self->co_argc_max - self->co_argc_min;
		DO(DeeFormat_PRINT(printer, arg, ", defaults: { "));
		for (i = 0; i < defaultc; ++i) {
			if (i != 0)
				DO(DeeFormat_PRINT(printer, arg, ", "));
			if (self->co_defaultv[i]) {
				DO(DeeFormat_PrintObjectRepr(printer, arg, self->co_defaultv[i]));
			} else {
				DO(DeeFormat_PRINT(printer, arg, "<unbound>"));
			}
		}
		DO(DeeFormat_PRINT(printer, arg, " }"));
	}
	if (self->co_flags != CODE_FASSEMBLY) {
		DO(DeeFormat_PRINT(printer, arg, ", flags: "));
		if (self->co_flags & ~(CODE_FYIELDING | CODE_FCOPYABLE | CODE_FASSEMBLY |
		                       CODE_FVARARGS | CODE_FVARKWDS | CODE_FTHISCALL |
		                       CODE_FHEAPFRAME | CODE_FCONSTRUCTOR)) {
			DO(DeeFormat_Printf(printer, arg, "%#" PRFx16, self->co_flags));
		} else {
			unsigned int flag_i;
			bool is_first = true;
			DO(DeeFormat_PRINT(printer, arg, "\""));
			for (flag_i = 0; flag_i < COMPILER_LENOF(code_flags_db); ++flag_i) {
				if (!(self->co_flags & code_flags_db[flag_i].cf_flag))
					continue;
				if (!is_first)
					DO(DeeFormat_PRINT(printer, arg, ","));
				DO(DeeFormat_PrintStr(printer, arg, code_flags_db[flag_i].cf_name));
				is_first = false;
			}
			DO(DeeFormat_PRINT(printer, arg, "\""));
		}
	}
	if (self->co_ddi != &empty_ddi) {
		DO(DeeFormat_Printf(printer, arg,
		                    ", ddi: %r",
		                    self->co_ddi));
	}
	DO(DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err:
	return temp;
#undef DO
}


PUBLIC DeeTypeObject DeeCode_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Code",
	/* .tp_doc      = */ DOC("()\n"
	                         "Return a singleton, stub code object that always returns ?N\n"
	                         "\n"
	                         "("
	                         /**/ "text:?DBytes=!N,module:?DModule=!N,statics:?S?O=!N,"
	                         /**/ "except:?S?X3?T4?Dint?Dint?Dint?Dint"
	                         /**/ /*       */ "?T5?Dint?Dint?Dint?Dint?X2?Dstring?Dint"
	                         /**/ /*       */ "?T6?Dint?Dint?Dint?Dint?X2?Dstring?Dint?DType"
	                         /**/ /*       */ "=!N,"
	                         /**/ "nlocal=!0,nstack=!0,refc=!0,argc=!0,keywords:?S?Dstring=!N,"
	                         /**/ "defaults:?S?O=!N,flags:?X2?Dstring?Dint=!P{},ddi:?Ert:Ddi=!N"
	                         ")\n"
	                         "#tIntegerOverflow{One of the specified arguments exceeds its associated implementation limit (the "
	                         /*                  */ "usual limit is $0xffff for most arguments, and $0xffffffff for the length of @text)}"
	                         "#tValueError{The given @flags, or the flags associated with a given @except are invalid}"
	                         "#ptext{The bytecode that should be executed by the code}"
	                         "#pmodule{The module to-be used as the declaring module}"
	                         "#pstatics{An indexable sequence containing the static variables that are to be made available to the code}"
	                         "#pexcept{A sequence of ${(startpc#: int, endpc#: int, entrypc#: int, entrysp#: int, flags#: string #| int = \"\", mask#: Type = none)}-"
	                         /*         */ "?D{Tuple}s, with `flags' being a comma-separated string of $\"finally\", $\"interrupt\", $\"handled\"}"
	                         "#pnlocal{The number of local variables to-be allocated for every frame}"
	                         "#pnstack{The amount of stack space to be allocated for every frame}"
	                         "#pargc{The max number of dedicated arguments taken by the function (must be >= ${##defaults} and == ${##keywords} if those are given). "
	                         /*       */ "Alternatively, you may omit this parameter, or pass 0 and provide @keywords in order to use ${##keywords} instead}"
	                         "#pkeywords{A sequence of strings describing the names of positional arguments (the length must be equal to ${##keywords})}"
	                         "#pdefaults{An indexable sequence describing the values to-be used for argument default values "
	                         /*           */ "Unbound items of this sequence translate to the corresponding argument being optional}"
	                         "#pflags{A comma-separated string of $\"yielding\", $\"copyable\", $\"lenient\", $\"varargs\", "
	                         /*        */ "$\"varkwds\", $\"thiscall\", $\"heapframe\", $\"finally\", $\"constructor\"}"
	                         "#pddi{The debug information descriptor that should be used for providing assembly meta-information}"
	                         "Construct a new code object from the given arguments\n"
	                         "Note that the returned code object always has the assembly tag enabled"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&code_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&code_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&code_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL,
				/* .tp_pad       = */ { (dfunptr_t)NULL },
				/* .tp_any_ctor_kw = */ (dfunptr_t)&code_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&code_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&code_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&code_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&code_visit,
	/* .tp_gc            = */ &code_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &code_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ code_getsets,
	/* .tp_members       = */ code_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

/* Pull in the actual code execution runtime. */
#ifndef __INTELLISENSE__
#define EXEC_SAFE 1
#include "code-exec.c.inl"
#undef EXEC_SAFE
#ifndef CONFIG_HAVE_EXEC_ASM
#define EXEC_FAST 1
#include "code-exec.c.inl"
#undef EXEC_FAST
#endif /* !CONFIG_HAVE_EXEC_ASM */
#endif /* !__INTELLISENSE__ */

#ifndef __INTELLISENSE__
#undef CALL_THIS
#undef CALL_TUPLE
#undef CALL_KW

#include "code-invoke.c.inl"
#define CALL_KW 1
#include "code-invoke.c.inl"
#define CALL_THIS 1
#include "code-invoke.c.inl"
#define CALL_THIS 1
#define CALL_KW 1
#include "code-invoke.c.inl"

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define CALL_TUPLE 1
#include "code-invoke.c.inl"
#define CALL_TUPLE 1
#define CALL_KW 1
#include "code-invoke.c.inl"
#define CALL_TUPLE 1
#define CALL_THIS 1
#include "code-invoke.c.inl"
#define CALL_TUPLE 1
#define CALL_THIS 1
#define CALL_KW 1
#include "code-invoke.c.inl"
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_EXECUTE_CODE_C */
