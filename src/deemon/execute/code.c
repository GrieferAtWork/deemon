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
#ifndef GUARD_DEEMON_EXECUTE_CODE_C
#define GUARD_DEEMON_EXECUTE_CODE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/dec.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/seq.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/host.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

#include <stdint.h>

#include "../objects/seq/svec.h"
#include "../runtime/kwlist.h"
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



#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE

/* Hidden C-API of _hostasm
 * NOTE: These values must match the definitions from `libhostasm.h' */
#define HOST_CC_F_KW    1 /* Take an an extra `DeeObject *kw' parameter for keyword arguments */
#define HOST_CC_F_TUPLE 2 /* Instead of `size_t argc, DeeObject *const *argv', take `DeeObject *args' */
#define HOST_CC_F_FUNC  4 /* Don't hard-code "ref" or "this_function" operands (for lambda functions) */
#define HOST_CC_F_THIS  8 /* Take an an extra `DeeObject *thisarg' parameter for "this" */
typedef uint8_t host_cc_t;

union host_rawfunc_entry {
	void                   *hfe_addr;
	DREF DeeObject *(DCALL *hfe_call)(size_t argc, DeeObject *const *argv);                                                                         /* HOST_CC_CALL */
	DREF DeeObject *(DCALL *hfe_call_kw)(size_t argc, DeeObject *const *argv, DeeObject *kw);                                                       /* HOST_CC_CALL_KW */
	DREF DeeObject *(DCALL *hfe_call_tuple)(DeeObject *args);                                                                                       /* HOST_CC_CALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_call_tuple_kw)(DeeObject *args, DeeObject *kw);                                                                     /* HOST_CC_CALL_TUPLE_KW */
	DREF DeeObject *(DCALL *hfe_func_call)(DeeFunctionObject *func, size_t argc, DeeObject *const *argv);                                           /* HOST_CC_FUNC_CALL */
	DREF DeeObject *(DCALL *hfe_func_call_kw)(DeeFunctionObject *func, size_t argc, DeeObject *const *argv, DeeObject *kw);                         /* HOST_CC_FUNC_CALL_KW */
	DREF DeeObject *(DCALL *hfe_func_call_tuple)(DeeFunctionObject *func, DeeObject *args);                                                         /* HOST_CC_FUNC_CALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_func_call_tuple_kw)(DeeFunctionObject *func, DeeObject *args, DeeObject *kw);                                       /* HOST_CC_FUNC_CALL_TUPLE_KW */
	DREF DeeObject *(DCALL *hfe_thiscall)(DeeObject *thisarg, size_t argc, DeeObject *const *argv);                                                 /* HOST_CC_THISCALL */
	DREF DeeObject *(DCALL *hfe_thiscall_kw)(DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);                               /* HOST_CC_THISCALL_KW */
	DREF DeeObject *(DCALL *hfe_thiscall_tuple)(DeeObject *thisarg, DeeObject *args);                                                               /* HOST_CC_THISCALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_thiscall_tuple_kw)(DeeObject *thisarg, DeeObject *args, DeeObject *kw);                                             /* HOST_CC_THISCALL_TUPLE_KW */
	DREF DeeObject *(DCALL *hfe_func_thiscall)(DeeFunctionObject *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv);                   /* HOST_CC_FUNC_THISCALL */
	DREF DeeObject *(DCALL *hfe_func_thiscall_kw)(DeeFunctionObject *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw); /* HOST_CC_FUNC_THISCALL_KW */
	DREF DeeObject *(DCALL *hfe_func_thiscall_tuple)(DeeFunctionObject *func, DeeObject *thisarg, DeeObject *args);                                 /* HOST_CC_FUNC_THISCALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_func_thiscall_tuple_kw)(DeeFunctionObject *func, DeeObject *thisarg, DeeObject *args, DeeObject *kw);               /* HOST_CC_FUNC_THISCALL_TUPLE_KW */
};

struct hostfunc {
	union host_rawfunc_entry hf_entry; /* Function entry point. */
	/* ... extra fields here... */
};

/* Create a host assembly function for `code' */
typedef WUNUSED_T NONNULL_T((2)) struct hostfunc *
(DCALL *LPHOSTASM_HOSTFUNC_NEW)(/*0..1*/ DeeFunctionObject *function,
                                /*1..1*/ DeeCodeObject *code,
                                host_cc_t cc);
typedef NONNULL_T((1)) void (DCALL *LPHOSTASM_HOSTFUNC_DESTROY)(struct hostfunc *func);
PRIVATE LPHOSTASM_HOSTFUNC_NEW pdyn_hostasm_hostfunc_new = NULL;
PRIVATE LPHOSTASM_HOSTFUNC_DESTROY pdyn_hostasm_hostfunc_destroy = NULL;

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE DEFINE_STRING(str__hostasm, "_hostasm");
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

/* Load the _hostasm API
 * @return: 1 : Unable to load API. In this case, `DeeCode_OptimizeCallThreshold'
 *              has automatically been set to `(size_t)-1'.
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED int DCALL hostasm_loadapi(void) {
	DREF DeeObject *mod_hostasm;
	LPHOSTASM_HOSTFUNC_NEW sym_hostasm_hostfunc_new;
	LPHOSTASM_HOSTFUNC_DESTROY sym_hostasm_hostfunc_destroy;
	COMPILER_READ_BARRIER();
	if (pdyn_hostasm_hostfunc_new)
		return 0;
	COMPILER_READ_BARRIER();
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	mod_hostasm = DeeModule_ImportEx("_hostasm", 8, NULL, 0, DeeModule_IMPORT_F_ENOENT, NULL);
	if unlikely(!ITER_ISOK(mod_hostasm)) {
		if unlikely(mod_hostasm == NULL)
			goto err;
		goto not_available;
	}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	mod_hostasm = DeeModule_OpenGlobal((DeeObject *)&str__hostasm, NULL, false);
	if unlikely(!ITER_ISOK(mod_hostasm)) {
		if unlikely(mod_hostasm == NULL)
			goto err;
		goto not_available;
	}
	if unlikely(DeeModule_RunInit(mod_hostasm) < 0)
		goto err_mod_hostasm;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	*(void **)&sym_hostasm_hostfunc_new     = DeeModule_GetNativeSymbol(mod_hostasm, "hostasm_hostfunc_new");
	*(void **)&sym_hostasm_hostfunc_destroy = DeeModule_GetNativeSymbol(mod_hostasm, "hostasm_hostfunc_destroy");
	Dee_Decref(mod_hostasm);
	if unlikely(!sym_hostasm_hostfunc_new)
		goto not_available;
	if unlikely(!sym_hostasm_hostfunc_destroy)
		goto not_available;
	COMPILER_WRITE_BARRIER();
	pdyn_hostasm_hostfunc_destroy = sym_hostasm_hostfunc_destroy;
	COMPILER_WRITE_BARRIER();
	pdyn_hostasm_hostfunc_new = sym_hostasm_hostfunc_new;
	COMPILER_WRITE_BARRIER();
	return 0;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err_mod_hostasm:
	Dee_Decref(mod_hostasm);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
err:
	return -1;
not_available:
#ifdef CONFIG_HAVE_CODE_METRICS
	atomic_write(&DeeCode_OptimizeCallThreshold, (size_t)-1);
#endif /* CONFIG_HAVE_CODE_METRICS */
	return 1;
}


#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define HOSTASM_FUNCTION_COUNT ((HOST_CC_F_KW | HOST_CC_F_TUPLE) + 1)
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define HOSTASM_FUNCTION_COUNT ((HOST_CC_F_KW) + 1)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */

struct Dee_hostasm_code_data {
	struct hostfunc *hcd_functions[HOSTASM_FUNCTION_COUNT]; /* [0..1][lock(WRITE_ONCE)][*] */
};
struct Dee_hostasm_function_data {
	struct hostfunc *hfd_functions[HOSTASM_FUNCTION_COUNT]; /* [0..1][lock(WRITE_ONCE)][*] */
};

INTERN NONNULL((1)) void DCALL
Dee_hostasm_code_data_destroy(struct Dee_hostasm_code_data *__restrict self) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(self->hcd_functions); ++i) {
		struct hostfunc *hfunc = self->hcd_functions[i];
		if (hfunc != NULL)
			(*pdyn_hostasm_hostfunc_destroy)(hfunc);
	}
	Dee_Free(self);
}

INTERN NONNULL((1)) void DCALL
Dee_hostasm_function_data_destroy(struct Dee_hostasm_function_data *__restrict self) {
	STATIC_ASSERT(COMPILER_LENOF(((struct Dee_hostasm_code_data *)0)->hcd_functions) ==
	              COMPILER_LENOF(((struct Dee_hostasm_function_data *)0)->hfd_functions));
	Dee_hostasm_code_data_destroy((struct Dee_hostasm_code_data *)self);
}


/* Lazily re-compile `self' as per `cc'
 * @return: 1 : Unable to recompile (_hostasm wasn't found, or didn't export the correct
 *              functions). In this case, `DeeCode_OptimizeCallThreshold' has automatically
 *              been set to `(size_t)-1'. Alternatively (when `allow_async == true'), the
 *              function might be getting optimized asynchronously, in which case the
 *              caller should execute it using the normal bytecode interpreter for the
 *              time being (once recomp finishes, it will automatically get hooked in the
 *              function/code).
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE NONNULL((1)) int DCALL
DeeFunction_HostAsmRecompile(DeeFunctionObject *__restrict self,
                             host_cc_t cc, bool allow_async) {
	int status;
	host_cc_t data_cc = cc & (HOSTASM_FUNCTION_COUNT - 1);
	struct Dee_hostasm_code_data *data, **p_data;
	DeeCodeObject *code = self->fo_code;
	struct hostfunc *hfunc;
	status = hostasm_loadapi();
	if (status != 0)
		return status; /* Error, or not available. */
	if (cc & HOST_CC_F_FUNC) {
		p_data = &code->co_hostasm.haco_data;
	} else {
		STATIC_ASSERT(sizeof(struct Dee_hostasm_code_data) ==
		              sizeof(struct Dee_hostasm_function_data));
		p_data = (struct Dee_hostasm_code_data **)&self->fo_hostasm.hafu_data;
	}
again_read_data:
	data = atomic_read(p_data);
	if (!data) {
		/* Lazily allocate data holder. */
		data = (struct Dee_hostasm_code_data *)Dee_Calloc(sizeof(struct Dee_hostasm_code_data));
		if unlikely(!data)
			goto err;
		if unlikely(!atomic_cmpxch(p_data, NULL, data)) {
			Dee_Free(data);
			goto again_read_data;
		}
	}

	/* Lazily compile the function variant in question. */
again_read_hfunc:
	hfunc = atomic_read(&data->hcd_functions[data_cc]);
	if likely(hfunc == NULL) {
		if (allow_async) {
#ifndef CONFIG_NO_THREADS
			/* TODO: Enqueue the recompilation operation to happen in a different thread. */
#endif /* !CONFIG_NO_THREADS */
		}

		/* Recompile the function in question. */
		hfunc = (*pdyn_hostasm_hostfunc_new)(cc & HOST_CC_F_FUNC ? NULL : self, code, cc);
		if unlikely(!hfunc) {
			/* Check for special case: _hostasm throws `IllegalInstruction' if it's unable
			 * to re-compile the function due to it using some sort of functionality that
			 * it is unable to handle (yet). */
			if (DeeError_Catch(&DeeError_IllegalInstruction)) {
				/* Remember that this code object can't be optimized. */
				atomic_or(&code->co_flags, CODE_FNOOPTIMIZE);
				return 1;
			}
			goto err;
		}
		if unlikely(!atomic_cmpxch(&data->hcd_functions[data_cc], NULL, hfunc)) {
			(*pdyn_hostasm_hostfunc_destroy)(hfunc);
			goto again_read_hfunc;
		}
	}

	/* Store the produced function pointer in its proper location. */
	switch (cc) {
	case 0:
	case HOST_CC_F_THIS:
		atomic_write(&self->fo_hostasm.hafu_call.c_norm, hfunc->hf_entry.hfe_call);
		break;
	case HOST_CC_F_KW:
	case HOST_CC_F_KW | HOST_CC_F_THIS:
		atomic_write(&self->fo_hostasm.hafu_call_kw.c_norm, hfunc->hf_entry.hfe_call_kw);
		break;
	case HOST_CC_F_FUNC:
	case HOST_CC_F_FUNC | HOST_CC_F_THIS:
		atomic_write(&code->co_hostasm.haco_call.c_norm, hfunc->hf_entry.hfe_func_call);
		break;
	case HOST_CC_F_FUNC | HOST_CC_F_KW:
	case HOST_CC_F_FUNC | HOST_CC_F_KW | HOST_CC_F_THIS:
		atomic_write(&code->co_hostasm.haco_call_kw.c_norm, hfunc->hf_entry.hfe_func_call_kw);
		break;
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	case HOST_CC_F_TUPLE:
	case HOST_CC_F_TUPLE | HOST_CC_F_THIS:
		atomic_write(&self->fo_hostasm.hafu_call_tuple.c_norm, hfunc->hf_entry.hfe_call_tuple);
		break;
	case HOST_CC_F_TUPLE | HOST_CC_F_KW:
	case HOST_CC_F_TUPLE | HOST_CC_F_KW | HOST_CC_F_THIS:
		atomic_write(&self->fo_hostasm.hafu_call_tuple_kw.c_norm, hfunc->hf_entry.hfe_call_tuple_kw);
		break;
	case HOST_CC_F_TUPLE | HOST_CC_F_FUNC:
	case HOST_CC_F_TUPLE | HOST_CC_F_FUNC | HOST_CC_F_THIS:
		atomic_write(&code->co_hostasm.haco_call_tuple.c_norm, hfunc->hf_entry.hfe_func_call_tuple);
		break;
	case HOST_CC_F_TUPLE | HOST_CC_F_FUNC | HOST_CC_F_KW:
	case HOST_CC_F_TUPLE | HOST_CC_F_FUNC | HOST_CC_F_KW | HOST_CC_F_THIS:
		atomic_write(&code->co_hostasm.haco_call_tuple_kw.c_norm, hfunc->hf_entry.hfe_func_call_tuple_kw);
		break;
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	default: __builtin_unreachable();
	}
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) int DCALL
DeeCode_HostAsmRecompile(DeeCodeObject *__restrict self,
                         host_cc_t cc, bool allow_async) {
	DeeCodeObject *_buf = self;
	DeeFunctionObject *func = COMPILER_CONTAINER_OF(&_buf, DeeFunctionObject, fo_code);
	ASSERT(cc & HOST_CC_F_FUNC);
	return DeeFunction_HostAsmRecompile(func, cc, allow_async);
}

#ifdef CONFIG_HAVE_CODE_METRICS
#ifndef DEFAULT_HOSTASM_RECOMPILE_CALL_THRESHOLD
#if 1 /* TODO: Enable this feature by default. */
#define DEFAULT_HOSTASM_RECOMPILE_CALL_THRESHOLD ((size_t)-1)
#else
#define DEFAULT_HOSTASM_RECOMPILE_CALL_THRESHOLD 128
#endif
#endif /* !DEFAULT_HOSTASM_RECOMPILE_CALL_THRESHOLD */

INTERN size_t DeeCode_OptimizeCallThreshold = DEFAULT_HOSTASM_RECOMPILE_CALL_THRESHOLD;

/* Get/set the threshold after which a code object
 * gets automatically recompiled into host assembly.
 *
 * Special values:
 * - 0 :         Functions are always optimized immediately.
 * - (size_t)-1: Functions are never optimized (when trying to
 *               optimize a function, and doing so fails because
 *               `_hostasm' can't be loaded, this value gets set
 *               automatically) */
#undef DeeCode_GetOptimizeCallThreshold
PUBLIC ATTR_PURE WUNUSED size_t DCALL
DeeCode_GetOptimizeCallThreshold(void) {
	return atomic_read(&DeeCode_OptimizeCallThreshold);
}

#undef DeeCode_SetOptimizeCallThreshold
PUBLIC size_t DCALL
DeeCode_SetOptimizeCallThreshold(size_t new_threshold) {
	return atomic_xch(&DeeCode_OptimizeCallThreshold, new_threshold);
}
#define DEFINED_DeeCode_GetOptimizeCallThreshold
#endif /* CONFIG_HAVE_CODE_METRICS */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */


PRIVATE ATTR_COLD int DCALL
err_code_optimization_disabled(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI, "Code optimization is not available/possible");
}

INTERN NONNULL((1)) DREF DeeObject *DCALL
function_optimize(DeeFunctionObject *__restrict self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("optimize", params: """
	bool tuple = false;
	bool kwds = false;
	bool async = false;
""", docStringPrefix: "function");]]]*/
#define function_optimize_params "tuple=!f,kwds=!f,async=!f"
	struct {
		bool tuple;
		bool kwds;
		bool async;
	} args;
	args.tuple = false;
	args.kwds = false;
	args.async = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__tuple_kwds_async, "|bbb:optimize", &args))
		goto err;
/*[[[end]]]*/
	(void)self;
#ifndef CONFIG_CALLTUPLE_OPTIMIZATIONS
	if (args.tuple) {
		DeeError_Throwf(&DeeError_ValueError, "Cannot optimize function with `tuple=true'");
		goto err;
	}
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	{
		int status;
		host_cc_t cc = 0;
		if (self->fo_code->co_flags & CODE_FTHISCALL)
			cc |= HOST_CC_F_THIS;
		if (args.kwds)
			cc |= HOST_CC_F_KW;
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
		if (args.tuple)
			cc |= HOST_CC_F_TUPLE;
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
		status = DeeFunction_HostAsmRecompile(self, cc, args.async);
		if unlikely(status < 0)
			goto err;
		if likely(status == 0)
			return_none;
	}
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	err_code_optimization_disabled();
err:
	return NULL;
}

INTERN NONNULL((1)) DREF DeeObject *DCALL
code_optimize(DeeCodeObject *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("optimize", params: """
	bool tuple = false;
	bool kwds = false;
	bool async = false;
""", docStringPrefix: "code");]]]*/
#define code_optimize_params "tuple=!f,kwds=!f,async=!f"
	struct {
		bool tuple;
		bool kwds;
		bool async;
	} args;
	args.tuple = false;
	args.kwds = false;
	args.async = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__tuple_kwds_async, "|bbb:optimize", &args))
		goto err;
/*[[[end]]]*/
	(void)self;
#ifndef CONFIG_CALLTUPLE_OPTIMIZATIONS
	if (args.tuple) {
		DeeError_Throwf(&DeeError_ValueError, "Cannot optimize function with `tuple=true'");
		goto err;
	}
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	{
		int status;
		host_cc_t cc = HOST_CC_F_FUNC;
		if (self->co_flags & CODE_FTHISCALL)
			cc |= HOST_CC_F_THIS;
		if (args.kwds)
			cc |= HOST_CC_F_KW;
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
		if (args.tuple)
			cc |= HOST_CC_F_TUPLE;
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
		status = DeeCode_HostAsmRecompile(self, cc, args.async);
		if unlikely(status < 0)
			goto err;
		if likely(status == 0)
			return_none;
	}
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	err_code_optimization_disabled();
err:
	return NULL;
}

INTERN NONNULL((1)) DREF DeeObject *DCALL
function_optimized(DeeFunctionObject *__restrict self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("optimized", params: """
	bool tuple = false;
	bool kwds = false;
""", docStringPrefix: "function");]]]*/
#define function_optimized_params "tuple=!f,kwds=!f"
	struct {
		bool tuple;
		bool kwds;
	} args;
	args.tuple = false;
	args.kwds = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__tuple_kwds, "|bb:optimized", &args))
		goto err;
/*[[[end]]]*/
	(void)self;
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	if (!args.tuple) {
		if (args.kwds ? (atomic_read(&self->fo_hostasm.hafu_call_kw.c_norm) != NULL)
		              : (atomic_read(&self->fo_hostasm.hafu_call.c_norm) != NULL))
			return_true;
	} else {
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
		if (args.kwds ? (atomic_read(&self->fo_hostasm.hafu_call_tuple_kw.c_norm) != NULL)
		              : (atomic_read(&self->fo_hostasm.hafu_call_tuple.c_norm) != NULL))
			return_true;
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	}
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	return_false;
err:
	return NULL;
}

INTERN NONNULL((1)) DREF DeeObject *DCALL
code_optimized(DeeCodeObject *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("optimized", params: """
	bool tuple = false;
	bool kwds = false;
""", docStringPrefix: "code");]]]*/
#define code_optimized_params "tuple=!f,kwds=!f"
	struct {
		bool tuple;
		bool kwds;
	} args;
	args.tuple = false;
	args.kwds = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__tuple_kwds, "|bb:optimized", &args))
		goto err;
/*[[[end]]]*/
	(void)self;
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	if (!args.tuple) {
		if (args.kwds ? (atomic_read(&self->co_hostasm.haco_call_kw.c_norm) != NULL)
		              : (atomic_read(&self->co_hostasm.haco_call.c_norm) != NULL))
			return_true;
	} else {
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
		if (args.kwds ? (atomic_read(&self->co_hostasm.haco_call_tuple_kw.c_norm) != NULL)
		              : (atomic_read(&self->co_hostasm.haco_call_tuple.c_norm) != NULL))
			return_true;
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	}
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	return_false;
err:
	return NULL;
}




#ifndef DEFINED_DeeCode_GetOptimizeCallThreshold
/* Get/set the threshold after which a code object
 * gets automatically recompiled into host assembly.
 *
 * Special values:
 * - 0 :         Functions are always optimized immediately.
 * - (size_t)-1: Functions are never optimized (when trying to
 *               optimize a function, and doing so fails because
 *               `_hostasm' can't be loaded, this value gets set
 *               automatically) */
#undef DeeCode_GetOptimizeCallThreshold
PUBLIC ATTR_PURE WUNUSED size_t DCALL
DeeCode_GetOptimizeCallThreshold(void) {
	COMPILER_IMPURE();
	return (size_t)-1;
}

#undef DeeCode_SetOptimizeCallThreshold
PUBLIC size_t DCALL
DeeCode_SetOptimizeCallThreshold(size_t new_threshold) {
	(void)new_threshold;
	COMPILER_IMPURE();
	return (size_t)-1;
}
#endif /* !DEFINED_DeeCode_GetOptimizeCallThreshold */



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



PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeCode_GetASymbolName(DeeObject const *__restrict self, uint16_t aid) {
	/* Argument */
	DeeCodeObject const *me = (DeeCodeObject const *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeCode_Type);
	if (me->co_keywords && aid < me->co_argc_max)
		return DeeString_STR(me->co_keywords[aid]);
	return NULL;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
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

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeCode_GetDDIString(DeeObject const *__restrict self, uint16_t id) {
	/* DDI String */
	DeeDDIObject const *ddi;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeCode_Type);
	ddi = ((DeeCodeObject const *)self)->co_ddi;
	if (id < ddi->d_nstring)
		return DeeString_STR(ddi->d_strtab) + ddi->d_strings[id];
	return NULL;
}





/* Define the special `DeeCode_Empty' object. */
INTERN DEFINE_CODE(DeeCode_Empty,
                   /* co_flags:      */ CODE_FCOPYABLE,
                   /* co_localc:     */ 0,
                   /* co_constc:     */ 0,
                   /* co_refc:       */ 0,
                   /* co_refstaticc: */ 0,
                   /* co_exceptc:    */ 0,
                   /* co_argc_min:   */ 0,
                   /* co_argc_max:   */ 0,
                   /* co_framesize:  */ 0,
                   /* co_codebytes:  */ sizeof(instruction_t),
                   /* co_module:     */ &DeeModule_Empty,
                   /* co_keywords:   */ NULL,
                   /* co_defaultv:   */ NULL,
                   /* co_constv:     */ NULL,
                   /* co_exceptv:    */ NULL,
                   /* co_ddi:        */ &DeeDDI_Empty,
                   /* co_code:       */ { ASM_RET_NONE });




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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	ASSERTF(!self->co_module ||
	        !DeeModule_Check(self->co_module) ||
	        self != atomic_read(&self->co_module->mo_moddata.mo_rootcode) ||
	        self->co_module->ob_refcnt == 0,
	        "Cannot destroy the root code object of a module, "
	        /**/ "before the module has been destroyed");
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	ASSERTF(!self->co_module ||
	        !DeeModule_Check(self->co_module) ||
	        self != self->co_module->mo_root ||
	        self->co_module->ob_refcnt == 0,
	        "Cannot destroy the root code object of a module, "
	        /**/ "before the module has been destroyed");
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	if (self->co_hostasm.haco_data)
		Dee_hostasm_code_data_destroy(self->co_hostasm.haco_data);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */

	ASSERT(self->co_argc_min <= self->co_argc_max);
	/* Clear default argument objects. */
	if (self->co_argc_max != self->co_argc_min) {
		uint16_t count = self->co_argc_max - self->co_argc_min;
		Dee_XDecrefv(self->co_defaultv, count);
	}

	/* Clear constants. */
	Dee_Decrefv(self->co_constv, self->co_constc);

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
	Dee_Free((void *)self->co_constv);
	Dee_Free((void *)self->co_exceptv);
}


PRIVATE NONNULL((1, 2)) void DCALL
code_visit(DeeCodeObject *__restrict self,
           Dee_visit_t proc, void *arg) {
	size_t i;

	/* Visit the accompanying module.
	 * NOTE: We must use `Dee_XVisit()' here because the pointer
	 *       may still be NULL when it still represents the next
	 *       element in the chain of code objects associated with
	 *       the module currently being compiled. */
	Dee_XVisit(self->co_module);

	/* Visit default variables. */
	Dee_XVisitv(self->co_defaultv, (uint16_t)(self->co_argc_max - self->co_argc_min));

	/* Visit constants. */
	Dee_Visitv(self->co_constv, self->co_constc);

	/* Visit exception information. */
	for (i = 0; i < self->co_exceptc; ++i)
		Dee_XVisit(self->co_exceptv[i].eh_mask);

	/* Visit debug information. */
	Dee_Visit(self->co_ddi);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_kwds(DeeCodeObject *__restrict self) {
	if likely(self->co_keywords) {
		return DeeRefVector_NewReadonly((DeeObject *)self,
		                                (size_t)self->co_argc_max,
		                                (DeeObject *const *)self->co_keywords);
	}
	if (self->co_argc_max == 0)
		return DeeSeq_NewEmpty();
	return DeeRT_ErrUnboundAttr((DeeObject *)self, (DeeObject *)&str___kwds__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
code_bound_kwds(DeeCodeObject *__restrict self) {
	return Dee_BOUND_FROMBOOL(self->co_keywords || self->co_argc_max == 0);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_getdefaults(DeeCodeObject *__restrict self) {
	ASSERT(self->co_argc_min <= self->co_argc_max);
	return DeeRefVector_NewReadonly((DeeObject *)self,
	                                (size_t)(self->co_argc_max - self->co_argc_min),
	                                self->co_defaultv);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_getconstants(DeeCodeObject *__restrict self) {
	ASSERT(self->co_argc_min <= self->co_argc_max);
	return DeeRefVector_NewReadonly((DeeObject *)self,
	                                self->co_constc,
	                                self->co_constv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_nstatic(DeeCodeObject *__restrict self) {
	ASSERT(self->co_refstaticc >= self->co_refc);
	return DeeInt_NewUInt16(self->co_refstaticc - self->co_refc);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_name(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_doc);
	if likely(info.fi_name)
		return (DREF DeeObject *)info.fi_name;
	DeeRT_ErrUnboundAttr((DeeObject *)self, (DeeObject *)&str___name__);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
code_bound_name(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	return Dee_BOUND_FROMBOOL(info.fi_name);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_doc(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	if likely(info.fi_doc)
		return (DREF DeeObject *)info.fi_doc;
	return_none;
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
	if likely(info.fi_type)
		return info.fi_type;
	DeeRT_ErrUnboundAttr((DeeObject *)self, (DeeObject *)&str___type__);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
code_bound_type(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	return Dee_BOUND_FROMBOOL(info.fi_type);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_operator(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_opname != (Dee_operator_t)-1)
		return DeeInt_NewUInt16(info.fi_opname);
	DeeRT_ErrUnboundAttrCStr((DeeObject *)self, "__operator__");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
code_bound_operator(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	return Dee_BOUND_FROMBOOL(info.fi_opname != (Dee_operator_t)-1);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
code_get_operatorname(DeeCodeObject *__restrict self) {
	struct function_info info;
	struct opinfo const *op;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_opname != (Dee_operator_t)-1) {
		op = DeeTypeType_GetOperatorById(info.fi_type ? Dee_TYPE(info.fi_type)
		                                              : &DeeType_Type,
		                                 info.fi_opname);
		Dee_XDecref(info.fi_type);
		if (!op)
			return DeeInt_NewUInt16(info.fi_opname);
		return DeeString_New(op->oi_sname);
	}
	Dee_XDecref(info.fi_type);
	DeeRT_ErrUnboundAttrCStr((DeeObject *)self, "__operatorname__");
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
	if (info.fi_getset != (uint16_t)-1)
		return DeeInt_NewUInt16(info.fi_getset);
	DeeRT_ErrUnboundAttrCStr((DeeObject *)self, "__property__");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
code_bound_property(DeeCodeObject *__restrict self) {
	struct function_info info;
	if (DeeCode_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	Dee_XDecref(info.fi_type);
	return Dee_BOUND_FROMBOOL(info.fi_getset != (uint16_t)-1);
err:
	return Dee_BOUND_ERR;
}

PRIVATE struct type_member tpconst code_members[] = {
	TYPE_MEMBER_FIELD_DOC("__ddi__", STRUCT_OBJECT, offsetof(DeeCodeObject, co_ddi),
	                      "->?Ert:Ddi\n"
	                      "The DDI (DeemonDebugInformation) data block"),
	TYPE_MEMBER_FIELD_DOC(STR___module__, STRUCT_OBJECT, offsetof(DeeCodeObject, co_module),
	                      "->?DModule"),
	TYPE_MEMBER_FIELD_DOC("__argc_min__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_argc_min),
	                      "Min amount of arguments required to execute @this ?."),
	TYPE_MEMBER_FIELD_DOC("__argc_max__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_argc_max),
	                      "Max amount of arguments accepted by @this ?. (excluding a varargs or varkwds argument)"),
	TYPE_MEMBER_BITFIELD_DOC("isyielding", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FYIELDING,
	                         "Check if @this ?. is for a yield-function"),
	TYPE_MEMBER_BITFIELD_DOC("iscopyable", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FCOPYABLE,
	                         "Check if execution frames of @this ?. can be copied"),
	TYPE_MEMBER_BITFIELD_DOC("hasvarargs", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FVARARGS,
	                         "Check if @this ?. accepts variable arguments as overflow"),
	TYPE_MEMBER_BITFIELD_DOC("hasvarkwds", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FVARKWDS,
	                         "Check if @this ?. accepts variable keyword arguments as overflow"),
	TYPE_MEMBER_BITFIELD_DOC("__isthiscall__", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FTHISCALL,
	                         "Check if @this ?. takes an extra leading $this-argument"),
	TYPE_MEMBER_BITFIELD_DOC("__hasassembly__", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FASSEMBLY,
	                         "Check if assembly of @this ?. is executed in safe-mode"),
	TYPE_MEMBER_BITFIELD_DOC("__islenient__", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FLENIENT,
	                         "Check if the runtime stack allocation allows for leniency"),
	TYPE_MEMBER_BITFIELD_DOC("__hasheapframe__", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FHEAPFRAME,
	                         "Check if the runtime stack-frame must be allocated on the heap"),
	TYPE_MEMBER_BITFIELD_DOC("__hasfinally__", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FFINALLY,
	                         "True if execution will jump to the nearest finally-block when a return instruction is encountered\n"
	                         "Note that this does not necessarily guaranty, or deny the presence of a try...finally statement in the "
	                         /**/ "user's source code, as the compiler may try to optimize this flag away to speed up runtime execution"),
	TYPE_MEMBER_BITFIELD_DOC("__isconstructor__", STRUCT_CONST, DeeCodeObject, co_flags, CODE_FCONSTRUCTOR,
	                         "True for class constructor ?. objects. - When set, don't include the this-argument in "
	                         /**/ "tracebacks, thus preventing incomplete instances from being leaked when the constructor "
	                         /**/ "causes some sort of exception to be thrown"),
#ifdef CONFIG_HAVE_CODE_METRICS
	TYPE_MEMBER_FIELD_DOC("__stat_functions__", STRUCT_CONST | STRUCT_SIZE_T,
	                      offsetof(DeeCodeObject, co_metrics.com_functions),
	                      "The number of times @this ?. was used to construct a ?DFunction"),
	TYPE_MEMBER_FIELD_DOC("__stat_call__", STRUCT_CONST | STRUCT_SIZE_T,
	                      offsetof(DeeCodeObject, co_metrics.com_call),
	                      "The number of times @this ?. was called without args-tuple and without keywords"),
	TYPE_MEMBER_FIELD_DOC("__stat_call_kw__", STRUCT_CONST | STRUCT_SIZE_T,
	                      offsetof(DeeCodeObject, co_metrics.com_call_kw),
	                      "The number of times @this ?. was called without args-tuple, but with keywords"),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	TYPE_MEMBER_FIELD_DOC("__stat_call_tuple__", STRUCT_CONST | STRUCT_SIZE_T,
	                      offsetof(DeeCodeObject, co_metrics.com_call_tuple),
	                      "The number of times @this ?. was called with args-tuple, but without keywords"),
	TYPE_MEMBER_FIELD_DOC("__stat_call_tuple_kw__", STRUCT_CONST | STRUCT_SIZE_T,
	                      offsetof(DeeCodeObject, co_metrics.com_call_tuple_kw),
	                      "The number of times @this ?. was called with args-tuple, and with keywords"),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* CONFIG_HAVE_CODE_METRICS */
	TYPE_MEMBER_FIELD_DOC("__nlocal__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_localc),
	                      "Number of available local variables during execution"),
	TYPE_MEMBER_FIELD_DOC("__nconst__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_constc),
	                      "Number of constant objects during execution"),
	TYPE_MEMBER_FIELD_DOC("__nref__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_refc),
	                      "Number of referenced objects during execution"),
	TYPE_MEMBER_FIELD_DOC("__nexcept__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeCodeObject, co_exceptc),
	                      "Number of exception handlers"),
	TYPE_MEMBER_END
};

#ifndef CONFIG_NO_DOC
INTERN_CONST char const code_optimize_doc[] =
"(" code_optimize_params ")\n"
"#pasync{When !t, allow the optimization to happen asynchronously"
#ifdef CONFIG_HAVE_CODE_METRICS
/*   */ " (this is the default when optimization happens as a result of"
/*   */ " ?A__stat_call__?DCode exceeding ?Ert:getcalloptimizethreshold)"
#endif /* CONFIG_HAVE_CODE_METRICS */
/*   */ "}"
"#tValueError{@tuple is true, but the tuple call optimizations are disabled ("
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
/*        */ "never"
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
/*        */ "always"
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
/*        */ " happens in the current implemented)}"
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
"#tUnsupportedAPI{Unable to load the ?M_hostasm module}"
#else /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
"#tUnsupportedAPI{Always thrown because automatic call optimizations "
/*            */ "have been disabled when building deemon}"
#endif /* !CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
"Try to optimize this ?. for the purpose of being able to run faster\n"
"#T{Arguments|Optimize calling convention~"
/**/ "${tuple=false,kwds=false}|${this(a, b, c)}&"
/**/ "${tuple=false,kwds=true}|${this(a, foo: b, bar: c)}, ${this(a, **kwds)}&"
/**/ "${tuple=true,kwds=false}|${this(args...)}&"
/**/ "${tuple=true,kwds=true}|${this(args..., **kwds)}"
"}";

INTERN_CONST char const code_optimized_doc[] =
"(" code_optimized_params ")->?Dbool\n"
"Check if @this ?. has been optimized for the @tuple and @kwds (s.a. ?#optimize)";
#endif /* !CONFIG_NO_DOC */

PRIVATE struct type_method tpconst code_methods[] = {
	TYPE_KWMETHOD("optimize", &code_optimize, DOC_GET(code_optimize_doc)),
	TYPE_KWMETHOD("optimized", &code_optimized, DOC_GET(code_optimized_doc)),
	TYPE_METHOD_END
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
	TYPE_GETTER_BOUND_F(STR___name__, &code_get_name, &code_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the name of @this ?. (s.a. ?A__name__?DFunction)"),
	TYPE_GETTER_AB_F(STR___doc__, &code_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "Returns the documentation string of @this ?., or ?N if unknown (s.a. ?A__doc__?DFunction)"),
	TYPE_GETTER_BOUND_F(STR___type__, &code_get_type, &code_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "Determine if @this ?. is defined as part of a user-defined class, "
	                    /**/ "and if it is, return that class type (s.a. ?A__type__?DFunction)"),
	TYPE_GETTER_BOUND_F(STR___kwds__, &code_get_kwds, &code_bound_kwds,
	                    METHOD_FCONSTCALL,
	                    "->?S?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Returns a sequence of keyword argument names accepted by @this ?.\n"
	                    "If @this ?. doesn't accept keyword arguments, throw :UnboundAttribute"),
	TYPE_GETTER_BOUND_F("__operator__", &code_get_operator, &code_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Try to determine if @this ?. is defined as part of a user-defined class, "
	                    /**/ "and if so, if it is used to define an operator callback. If that is the case, "
	                    /**/ "return the internal ID of the operator that @this ?. provides, or throw "
	                    /**/ ":UnboundAttribute if that class couldn't be found, @this ?. is defined "
	                    /**/ "as stand-alone, or defined as a class- or instance-method (s.a. ?A__operator__?DFunction)"),
	TYPE_GETTER_BOUND_F("__operatorname__", &code_get_operatorname, &code_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?X2?Dstring?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Same as ?#__operator__, but instead try to return the unambiguous name of the "
	                    /**/ "operator, though still return its ID if the operator isn't recognized as being "
	                    /**/ "part of the standard (s.a. ?A__operatorname__?DFunction)"),
	TYPE_GETTER_BOUND_F("__property__", &code_get_property, &code_bound_property,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Returns an integer describing the kind if @this ?. is part of a property or getset, "
	                    /**/ "or throw :UnboundAttribute if the function's property could not be found, or if "
	                    /**/ "the function isn't declared as a property callback (s.a. ?A__property__?DFunction)"),
	TYPE_GETTER_AB_F("__defaults__", &code_getdefaults, METHOD_FCONSTCALL,
	                "->?S?O\n"
	                "Access to the default values of arguments"),
	TYPE_GETTER_AB_F("__constants__", &code_getconstants, METHOD_FCONSTCALL,
	                "->?S?O\n"
	                "Access to the constants of @this ?."),
	TYPE_GETTER_AB_F("__nstatic__", &code_get_nstatic, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                "->?Dint\n"
	                "Number of static variables during execution"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
code_hash(DeeCodeObject *__restrict self) {
	Dee_hash_t result;
	result = Dee_HashPtr(DeeObject_DATA(self),
	                     COMPILER_OFFSETAFTER(DeeCodeObject, co_codebytes) -
	                     sizeof(DeeObject));
	if (self->co_module)
		result = Dee_HashCombine(result, DeeObject_Hash(self->co_module));
	if (self->co_keywords) {
		uint16_t i;
		for (i = 0; i < self->co_argc_max; ++i) {
			DeeStringObject *kwd = self->co_keywords[i];
			result = Dee_HashCombine(result, DeeString_Hash(kwd));
		}
	}
	if (self->co_defaultv) {
		uint16_t i;
		for (i = 0; i < (self->co_argc_max - self->co_argc_min); ++i) {
			DeeObject *def = self->co_defaultv[i];
			if (def != NULL)
				result = Dee_HashCombine(result, DeeObject_Hash(def));
		}
	}
	if (self->co_constv)
		result = Dee_HashCombine(result, DeeObject_Hashv(self->co_constv, self->co_constc));
	if (self->co_exceptv) {
		uint16_t i;
		for (i = 0; i < self->co_exceptc; ++i) {
			Dee_hash_t spec;
			DeeTypeObject *mask;
			spec = Dee_HashPtr(&self->co_exceptv[i].eh_start,
			                   sizeof(struct except_handler) -
			                   offsetof(struct except_handler, eh_start));
			result = Dee_HashCombine(result, spec);
			mask   = self->co_exceptv[i].eh_mask;
			if (mask != NULL)
				result = Dee_HashCombine(result, DeeObject_Hash(mask));
		}
	}
	result = Dee_HashCombine(result, DeeObject_Hash(self->co_ddi));
	result = Dee_HashCombine(result, Dee_HashPtr(self->co_code, self->co_codebytes));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
code_compare_eq_impl(DeeCodeObject *self, DeeCodeObject *other) {
	if (self == other)
		return Dee_COMPARE_EQ;
	if (self->co_flags != other->co_flags)
		goto nope;
	if (self->co_localc != other->co_localc)
		goto nope;
	if (self->co_constc != other->co_constc)
		goto nope;
	if (self->co_refc != other->co_refc)
		goto nope;
	if (self->co_refstaticc != other->co_refstaticc)
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
				int temp;
				if (other->co_defaultv[i] == NULL)
					goto nope;
				temp = DeeObject_TryCompareEq(self->co_defaultv[i],
				                                       other->co_defaultv[i]);
				if (temp != 0)
					return temp;
			}
		}
	}
	if (self->co_constv) {
		uint16_t i;
		for (i = 0; i < self->co_constc; ++i) {
			int temp = DeeObject_TryCompareEq(self->co_constv[i], other->co_constv[i]);
			if (temp != 0)
				return temp;
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
	if (bcmp(self->co_code, other->co_code, self->co_codebytes) != 0)
		goto nope;
	return DeeObject_TryCompareEq((DeeObject *)self->co_ddi,
	                              (DeeObject *)other->co_ddi);
nope:
	return Dee_COMPARE_NE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
code_compare_eq(DeeCodeObject *self, DeeCodeObject *other) {
	if (DeeObject_AssertTypeExact(other, &DeeCode_Type))
		goto err;
	return code_compare_eq_impl(self, other);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
code_trycompare_eq(DeeCodeObject *self, DeeCodeObject *other) {
	if (!DeeCode_Check(other))
		return Dee_COMPARE_NE;
	return code_compare_eq_impl(self, other);
}

PRIVATE struct type_cmp code_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&code_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&code_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&code_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE WUNUSED DREF DeeCodeObject *DCALL code_ctor(void) {
	return_reference_((DeeCodeObject *)&DeeCode_Empty);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
code_copy(DeeCodeObject *__restrict self) {
	DREF DeeCodeObject *result;
	result = DeeCode_Malloc(self->co_codebytes);
	if unlikely(!result)
		goto done;
	memcpy(result, self, offsetof(DeeCodeObject, co_code) + self->co_codebytes);
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
	ASSERT((result->co_constc != 0) ==
	       (result->co_constv != NULL));
	if (result->co_constv) {
		result->co_constv = (DREF DeeObject **)Dee_Mallocc(result->co_constc,
		                                                    sizeof(DREF DeeObject *));
		if unlikely(!result->co_constv)
			goto err_r_default;
		Dee_Movrefv(result->co_constv, self->co_constv, result->co_constc);
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
done:
	return result;
err_r_static:
	if (result->co_constv) {
		Dee_Decrefv(result->co_constv, result->co_constc);
		Dee_Free((void *)result->co_constv);
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
	if (result->co_constv) {
		for (i = 0; i < result->co_constc; ++i) {
			if (DeeObject_InplaceDeepCopy((DeeObject **)&result->co_constv[i]))
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

struct except_handler_specs {
	struct except_handler *exsp_v; /* [0..exsp_c|ALLOC(exsp_a)] Except handler vector */
	uint16_t               exsp_c; /* # of initialized fields in `exsp_v' */
	uint16_t               exsp_a; /* Allocated size of `exsp_v' */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
except_handler_specs_init_foreach_cb(void *arg, DeeObject *item) {
	struct except_handler_specs *data;
	struct except_handler *slot;
	data = (struct except_handler_specs *)arg;
	ASSERT(data->exsp_c <= data->exsp_a);

	/* Ensure sufficient buffer space */
	if (data->exsp_c >= data->exsp_a) {
		struct except_handler *new_exspv;
		uint16_t new_alloc = data->exsp_a * 2;
		if (!data->exsp_a) {
			new_alloc = 2;
		} else if unlikely(new_alloc <= data->exsp_a) {
			if unlikely(data->exsp_a == (uint16_t)-1) {
				return DeeError_Throwf(&DeeError_IntegerOverflow,
				                       "Too many exception handlers");
			}
			new_alloc = (uint16_t)-1;
		}
		new_exspv = (struct except_handler *)Dee_TryReallocc(data->exsp_v, new_alloc,
		                                                     sizeof(struct except_handler));
		if unlikely(!new_exspv) {
			new_alloc = data->exsp_c + 1;
			new_exspv = (struct except_handler *)Dee_Reallocc(data->exsp_v, new_alloc,
			                                                  sizeof(struct except_handler));
			if unlikely(!new_exspv)
				goto err;
		}
		data->exsp_v = new_exspv;
		data->exsp_a = new_alloc;
	}
	slot = &data->exsp_v[data->exsp_c];
	if unlikely(unpack_exception_descriptor(slot, item))
		goto err;
	++data->exsp_c;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
except_handler_specs_init(struct except_handler_specs *__restrict self,
                          DeeObject *except_specs) {
	Dee_ssize_t status;
	self->exsp_v = NULL;
	self->exsp_c = 0;
	self->exsp_a = 0;
#ifndef __OPTIMIZE_SIZE__
	{
		size_t hint = DeeObject_SizeFast(except_specs);
		if (hint != (size_t)-1 && hint <= 0xffff) {
			struct except_handler *buffer;
			buffer = (struct except_handler *)Dee_TryMallocc(hint, sizeof(struct except_handler));
			if likely(buffer) {
				self->exsp_v = buffer;
				self->exsp_a = (uint16_t)hint;
			}
		}
	}
#endif /* !__OPTIMIZE_SIZE__ */
	status = DeeObject_Foreach(except_specs, &except_handler_specs_init_foreach_cb, self);
	if unlikely(status)
		goto err;
	if (self->exsp_a > self->exsp_c) {
		struct except_handler *new_except_v;
		new_except_v = (struct except_handler *)Dee_TryReallocc(self->exsp_v, self->exsp_c,
		                                                        sizeof(struct except_handler));
		if likely(new_except_v)
			self->exsp_v = new_except_v;
	}
	return 0;
err:
	while (self->exsp_c--)
		Dee_XDecref(self->exsp_v[self->exsp_c].eh_mask);
	Dee_Free(self->exsp_v);
	return -1;
}



PRIVATE WUNUSED DREF DeeCodeObject *DCALL
code_init_kw(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeBuffer text_buf;
	DREF DeeCodeObject *result;
	/* (text:?DBytes=!N,module:?DModule=!N,constants:?S?O=!N,
	 *  except:?S?X2?T5?Dint?Dint?Dint?Dint?X2?Dstring?Dint?T6?Dint?Dint?Dint?Dint?X2?Dstring?Dint?DType=!N,
	 *  nlocal=!0,nstack=!0,nref=!0,nstatic=!0,argc=!0,keywords:?S?Dstring=!N,defaults:?S?O=!N,
	 *  flags:?X2?Dstring?Dint=!P{lenient},ddi:?Ert:Ddi=!N) */
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("_Code", params: "
		DeeObject *text = Dee_None;
		DeeModuleObject *module = (DeeModuleObject *)Dee_None;
		DeeObject *constants = Dee_None;
		DeeObject *except = Dee_None;
		uint16_t nlocal = 0;
		uint16_t nstack = 0;
		uint16_t nref = 0;
		uint16_t nstatic = 0;
		uint16_t argc = 0;
		DeeObject *keywords = Dee_None;
		DeeObject *defaults = Dee_None;
		DeeObject *flags = Dee_None;
		DeeDDIObject *ddi = (DeeDDIObject *)Dee_None;
");]]]*/
	struct {
		DeeObject *text;
		DeeModuleObject *module_;
		DeeObject *constants;
		DeeObject *except;
		uint16_t nlocal;
		uint16_t nstack;
		uint16_t nref;
		uint16_t nstatic;
		uint16_t argc;
		DeeObject *keywords;
		DeeObject *defaults;
		DeeObject *flags;
		DeeDDIObject *ddi;
	} args;
	args.text = Dee_None;
	args.module_ = (DeeModuleObject *)Dee_None;
	args.constants = Dee_None;
	args.except = Dee_None;
	args.nlocal = 0;
	args.nstack = 0;
	args.nref = 0;
	args.nstatic = 0;
	args.argc = 0;
	args.keywords = Dee_None;
	args.defaults = Dee_None;
	args.flags = Dee_None;
	args.ddi = (DeeDDIObject *)Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__text_module_constants_except_nlocal_nstack_nref_nstatic_argc_keywords_defaults_flags_ddi, "|oooo" UNPu16 UNPu16 UNPu16 UNPu16 UNPu16 "oooo:_Code", &args))
		goto err;
/*[[[end]]]*/
	if (DeeNone_Check(args.flags))
		args.flags = Dee_EmptyString;
	if (DeeObject_GetBuf(args.text, &text_buf, Dee_BUFFER_FREADONLY))
		goto err;
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(text_buf.bb_size > (code_size_t)-1) {
		DeeRT_ErrIntegerOverflowU(text_buf.bb_size, __UINT32_MAX__);
		goto err_buf;
	}
#endif /* __SIZEOF_SIZE_T__ > 4 */
	result = DeeCode_Malloc(text_buf.bb_size + INSTRLEN_MAX);
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
	DeeObject_PutBuf(args.text, &text_buf, Dee_BUFFER_FREADONLY);
	/* Load keyword arguments */
	result->co_keywords = NULL;
	if (!DeeNone_Check(args.keywords)) {
		DREF DeeStringObject **keyword_vec;
		uint16_t i;
		if (!args.argc) {
			/* Automatically determine the argument count. */
			size_t keyword_count;
			keyword_vec = (DREF DeeStringObject **)DeeSeq_AsHeapVector(args.keywords,
			                                                           &keyword_count);
			if unlikely(!keyword_vec)
				goto err_r;
			if unlikely(keyword_count > (uint16_t)-1) {
				DeeError_Throwf(&DeeError_IntegerOverflow,
				                "Too many arguments %" PRFuSIZ " for when at most 0xffff can be used",
				                keyword_count);
				goto err_r;
			}
			args.argc = (uint16_t)keyword_count;
		} else {
			keyword_vec = (DREF DeeStringObject **)Dee_Mallocc(args.argc,
			                                                   sizeof(DREF DeeStringObject *));
			if unlikely(!keyword_vec)
				goto err_r;
			if unlikely(DeeSeq_Unpack(args.keywords, args.argc, (DeeObject **)keyword_vec)) {
				Dee_Free(keyword_vec);
				goto err_r;
			}
		}
		result->co_keywords = keyword_vec; /* Inherit */
		/* Ensure that all elements are strings. */
		for (i = 0; i < args.argc; ++i) {
			if (DeeObject_AssertTypeExact(keyword_vec[i], &DeeString_Type))
				goto err_r_keywords;
		}
	}

	result->co_argc_min = args.argc;
	result->co_argc_max = args.argc;
	result->co_defaultv = NULL;
	/* Load default arguments */
	if (!DeeNone_Check(args.defaults)) {
		size_t default_c;
		DREF DeeObject **default_vec;
		default_c = DeeObject_Size(args.defaults);
		if unlikely(default_c == (size_t)-1)
			goto err_r_keywords;
		if unlikely(default_c > args.argc) {
			DeeError_Throwf(&DeeError_IntegerOverflow,
			                "Too many default arguments (%" PRFuSIZ ") for "
			                "code only taking %" PRFu16 " arguments at most",
			                default_c, args.argc);
			goto err_r_keywords;
		}
		default_vec = (DREF DeeObject **)Dee_Mallocc(default_c, sizeof(DREF DeeObject *));
		if unlikely(!default_vec)
			goto err_r_keywords;
		if unlikely(DeeObject_InvokeMethodHint(seq_unpack_ub, args.defaults, default_c,
		                                       default_c, default_vec) == (size_t)-1) {
			Dee_Free(default_vec);
			goto err_r_keywords;
		}
		result->co_defaultv = default_vec;
		result->co_argc_min = (uint16_t)(args.argc - (uint16_t)default_c);
	}
	result->co_constc = 0;
	result->co_constv = NULL;
	if (!DeeNone_Check(args.constants)) {
		DREF DeeObject **constants_vec;
		size_t constants_cnt;
		constants_vec = DeeSeq_AsHeapVector(args.constants, &constants_cnt);
		if unlikely(!constants_vec)
			goto err_r_defaultv;
		if unlikely(constants_cnt > (uint16_t)-1) {
			Dee_Decrefv(constants_vec, constants_cnt);
			Dee_Free(constants_vec);
			DeeRT_ErrIntegerOverflowU(constants_cnt, __UINT16_MAX__);
			goto err_r_defaultv;
		}
		result->co_constc = (uint16_t)constants_cnt;
		result->co_constv = constants_vec;
	}
	if (DeeNone_Check(args.module_)) {
		DeeThreadObject *ts = DeeThread_Self();
		if unlikely(!ts->t_execsz) {
			DeeError_Throwf(&DeeError_TypeError,
			                "No module given, when the current "
			                "module could not be determined");
			goto err_r_constv;
		}
		ASSERT(ts->t_exec);
		ASSERT(ts->t_exec->cf_func);
		ASSERT(ts->t_exec->cf_func->fo_code);
		ASSERT(ts->t_exec->cf_func->fo_code->co_module);
		args.module_ = ts->t_exec->cf_func->fo_code->co_module;
	}
	/* NOTE: Always check this, so prevent stuff like interactive
	 *       modules to leaking into generic code objects. */
	if (DeeObject_AssertTypeExact(args.module_, &DeeModule_Type))
		goto err_r_constv;

	/* Generate exception handlers. */
	result->co_exceptc = 0;
	result->co_exceptv = NULL;
	if (!DeeNone_Check(args.except)) {
		struct except_handler_specs specs;
		if unlikely(except_handler_specs_init(&specs, args.except))
			goto err_r_constv;
		result->co_exceptc = specs.exsp_c;
		result->co_exceptv = specs.exsp_v; /* Inherit */
	}

	/* Load custom code flags */
	result->co_flags = CODE_FASSEMBLY;
	if (!DeeNone_Check(args.flags)) {
		if (DeeString_Check(args.flags)) {
			char const *s = DeeString_STR(args.flags);
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
			if (DeeObject_AsUInt16(args.flags, &result->co_flags))
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
	if (DeeNone_Check(args.ddi)) {
		args.ddi = &DeeDDI_Empty;
	} else {
		if (DeeObject_AssertTypeExact(args.ddi, &DeeDDI_Type))
			goto err_r_except;
	}

	/* Fill in remaining fields. */
	result->co_ddi = args.ddi;
	Dee_Incref(args.ddi);
	result->co_module = args.module_;
	Dee_Incref(args.module_);
	result->co_localc = args.nlocal;
	result->co_refc   = args.nref;
	if (OVERFLOW_UADD(args.nref, args.nstatic, &result->co_refstaticc)) {
		DeeError_Throwf(&DeeError_ValueError, "Too many references or static variables");
		goto err_r_ddi;
	}
	result->co_framesize  = (args.nlocal + args.nstack) * sizeof(DREF DeeObject *);
	if (result->co_framesize > CODE_LARGEFRAME_THRESHOLD)
		result->co_flags |= CODE_FHEAPFRAME;
#ifdef CONFIG_HAVE_CODE_METRICS
	Dee_code_metrics_init(&result->co_metrics);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	Dee_hostasm_code_init(&result->co_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */

	/* Initialize the new code object, and start tracking it. */
	DeeObject_Init(result, &DeeCode_Type);
	return result;
err_r_ddi:
	Dee_Decref(result->co_ddi);
err_r_except:
	if (result->co_exceptv) {
		while (result->co_exceptc--)
			Dee_XDecref(result->co_exceptv[result->co_exceptc].eh_mask);
		Dee_Free((void *)result->co_exceptv);
	}
err_r_constv:
	if (result->co_constv) {
		Dee_Decrefv(result->co_constv, result->co_constc);
		Dee_Free((void *)result->co_constv);
	}
err_r_defaultv:
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
	DeeObject_PutBuf(args.text, &text_buf, Dee_BUFFER_FREADONLY);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
code_print(DeeCodeObject *__restrict self,
           Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *name = code_get_name(self);
	if unlikely(!name) {
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return DeeFormat_PRINT(printer, arg, "<code for <anonymous>>");
		goto err;
	}
	result = DeeFormat_Printf(printer, arg, "<code for %r>", name);
	Dee_Decref(name);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
code_printrepr(DeeCodeObject *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
#define DO(x)                         \
	do {                              \
		if unlikely((temp = (x)) < 0) \
			goto err;                 \
		result += temp;               \
	}	__WHILE0
	Dee_ssize_t temp, result;
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

	if (self->co_constc > 0) {
		uint16_t i;
		DO(DeeFormat_PRINT(printer, arg, ", constants: { "));
		for (i = 0; i < self->co_constc; ++i) {
			DeeObject *ob;
			if (i != 0)
				DO(DeeFormat_PRINT(printer, arg, ", "));
			ob = self->co_constv[i];
			temp = DeeFormat_PrintObjectRepr(printer, arg, ob);
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
		DO(DeeFormat_Printf(printer, arg, ", nref: %" PRFu16, self->co_refc));
	if (self->co_refstaticc > self->co_refc)
		DO(DeeFormat_Printf(printer, arg, ", nstatic: %" PRFu16, self->co_refstaticc - self->co_refc));
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
	if (self->co_ddi != &DeeDDI_Empty) {
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


/* Allocate a duplicate of `objv...+=objc' and put pointer to base of that vector
 * at `addr'. NULL-elements within `objv' are replicated within the duplicate.
 * @return: * : Address of duplicated vector
 * @return: 0 : An error was thrown. */
DFUNDEF WUNUSED ATTR_INS(3, 4) NONNULL((1)) Dee_dec_addr_t
(DCALL DeeDecWriter_XPutObjectVectorDup)(DeeDecWriter *__restrict self, Dee_dec_addr_t addr,
                                         DeeObject *const *objv, size_t objc) {
	size_t i;
	Dee_dec_addr_t md_addr = DeeDecWriter_Malloc(self, objc * sizeof(DREF DeeObject *));
	if unlikely(!md_addr)
		goto err;
	for (i = 0; i < objc; ++i) {
		Dee_dec_addr_t addrof_i = md_addr + i * sizeof(DREF DeeObject *);
		if (DeeDecWriter_XPutObject(self, addrof_i, objv[i]))
			goto err;
	}
	if (DeeDecWriter_PutRel(self, addr, md_addr))
		goto err;
	return md_addr;
err:
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_dec_addr_t DCALL
code_writedec(DeeDecWriter *__restrict writer,
              DeeCodeObject *__restrict self) {
	DeeCodeObject *out;
	size_t sizeof_code = offsetof(DeeCodeObject, co_code) + self->co_codebytes;
	Dee_dec_addr_t addr = DeeDecWriter_Object_Malloc(writer, sizeof_code, self);
	if unlikely(!addr)
		goto err;
	out = DeeDecWriter_Addr2Mem(writer, addr, DeeCodeObject);
	out->co_flags      = self->co_flags;
	out->co_localc     = self->co_localc;
	out->co_constc     = self->co_constc;
	out->co_refc       = self->co_refc;
	out->co_refstaticc = self->co_refstaticc;
	out->co_exceptc    = self->co_exceptc;
	out->co_argc_min   = self->co_argc_min;
	out->co_argc_max   = self->co_argc_max;
	out->co_framesize  = self->co_framesize;
	out->co_codebytes  = self->co_codebytes;
	out->co_keywords   = NULL; /* Overwritten below (if necessary) */
	out->co_defaultv   = NULL; /* Overwritten below (if necessary) */
	out->co_constv     = NULL; /* Overwritten below (if necessary) */
	out->co_exceptv    = NULL; /* Overwritten below (if necessary) */
#ifdef CONFIG_HAVE_CODE_METRICS
	bzero(&out->co_metrics, sizeof(out->co_metrics)); /* Don't serialize metrics */
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	bzero(&out->co_hostasm, sizeof(out->co_hostasm)); /* Don't serialize hostasm */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	memcpy(out->co_code, self->co_code, self->co_codebytes);
	if (self->co_keywords) {
		if unlikely(!DeeDecWriter_XPutObjectVectorDup(writer, addr + offsetof(DeeCodeObject, co_keywords),
		                                              (DeeObject *const *)self->co_keywords,
		                                              self->co_argc_max))
			goto err;
	}
	if (self->co_defaultv) {
		if unlikely(!DeeDecWriter_XPutObjectVectorDup(writer, addr + offsetof(DeeCodeObject, co_defaultv),
		                                              (DeeObject *const *)self->co_defaultv,
		                                              self->co_argc_max - self->co_argc_min))
			goto err;
	}
	if (self->co_constv) {
		if unlikely(!DeeDecWriter_XPutObjectVectorDup(writer, addr + offsetof(DeeCodeObject, co_constv),
		                                              (DeeObject *const *)self->co_constv,
		                                              self->co_constc))
			goto err;
	}
	if (self->co_exceptv) {
		struct except_handler *out_exceptv, *in_exceptv;
		uint16_t i, exceptc = self->co_exceptc;
		size_t sizeof_exceptv = exceptc * sizeof(struct except_handler);
		Dee_dec_addr_t addrof_out_exceptv = DeeDecWriter_Malloc(writer, sizeof_exceptv);
		if unlikely(!addrof_out_exceptv)
			goto err;
		out_exceptv = DeeDecWriter_Addr2Mem(writer, addrof_out_exceptv, struct except_handler);
		in_exceptv  = self->co_exceptv;
		for (i = 0; i < exceptc; ++i) {
			out_exceptv[i] = in_exceptv[i];
			if (in_exceptv[i].eh_mask) {
				Dee_dec_addr_t addrof_out_exceptv_item_mask;
				addrof_out_exceptv_item_mask = addrof_out_exceptv +
				                               i * sizeof(struct except_handler) +
				                               offsetof(struct except_handler, eh_mask);
				if (DeeDecWriter_PutObject(writer, addrof_out_exceptv_item_mask, in_exceptv[i].eh_mask))
					goto err;
				out_exceptv = DeeDecWriter_Addr2Mem(writer, addrof_out_exceptv, struct except_handler);
			}
		}
		if (DeeDecWriter_PutRel(writer, addr + offsetof(DeeCodeObject, co_exceptv), addrof_out_exceptv))
			goto err;
	}
	if (DeeDecWriter_PutObject(writer, addr + offsetof(DeeCodeObject, co_module), self->co_module))
		goto err;
	if (DeeDecWriter_PutObject(writer, addr + offsetof(DeeCodeObject, co_ddi), self->co_ddi))
		goto err;
	return addr;
err:
	return 0;
}

PUBLIC DeeTypeObject DeeCode_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Code",
	/* .tp_doc      = */ DOC("()\n"
	                         "Return a singleton, stub code object that always returns ?N\n"
	                         "\n"
	                         "("
	                         /**/ "text:?X2?DBytes?N=!N,module:?X2?DModule?N=!N,constants:?S?O=!N,"
	                         /**/ "except:?S?X3?T4?Dint?Dint?Dint?Dint"
	                         /**/ /*       */ "?T5?Dint?Dint?Dint?Dint?X2?Dstring?Dint"
	                         /**/ /*       */ "?T6?Dint?Dint?Dint?Dint?X2?Dstring?Dint?DType"
	                         /**/ /*       */ "=!N,"
	                         /**/ "nlocal=!0,nstack=!0,nref=!0,nstatic=!0,argc=!0,keywords:?S?Dstring=!N,"
	                         /**/ "defaults:?S?O=!N,flags:?X2?Dstring?Dint=!P{},ddi:?X2?Ert:Ddi?N=!N"
	                         ")\n"
	                         "#tIntegerOverflow{One of the specified arguments exceeds its associated implementation limit (the "
	                         /*                  */ "usual limit is $0xffff for most arguments, and $0xffffffff for the length of @text)}"
	                         "#tValueError{The given @flags, or the flags associated with a given @except are invalid}"
	                         "#ptext{The bytecode that should be executed by the code}"
	                         "#pmodule{The module to-be used as the declaring module}"
	                         "#pconstants{An indexable sequence containing the constants that are to be made available to the code}"
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
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&code_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&code_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&code_deepcopy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				/* .tp_free      = */ (Dee_funptr_t)NULL, { NULL },
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&code_init_kw,
				/* .tp_writedec    = */ (Dee_funptr_t)&code_writedec
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&code_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&code_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&code_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&code_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &code_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ code_methods,
	/* .tp_getsets       = */ code_getsets,
	/* .tp_members       = */ code_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
