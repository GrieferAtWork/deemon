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
#ifndef GUARD_DEX_HOSTASM_HOST_H
#define GUARD_DEX_HOSTASM_HOST_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/system-features.h>

#include <hybrid/align.h>
#include <hybrid/byteorder.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

#include <stdint.h>

#ifndef __INTELLISENSE__
#include <deemon/alloc.h>
#endif /* !__INTELLISENSE__ */

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
#ifdef CONFIG_hostfunc_USES_VirtualAlloc
#include <Windows.h>
#endif /* CONFIG_hostfunc_USES_VirtualAlloc */


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
#define HOST_SIZEOF_POINTER     8
#define HOSTASM_STACK_ALIGNMENT 16
#ifdef HOSTASM_X86_64_SYSVABI
#define HOSTASM_REDZONE_SIZE 128
#endif /* HOSTASM_X86_64_SYSVABI */
#ifdef HOSTASM_X86_64_MSABI
#define HOSTASM_SCRACHAREA_SIZE 32
#endif /* HOSTASM_X86_64_MSABI */
#define HOST_BYTEORDER __ORDER_LITTLE_ENDIAN__
#elif defined(HOSTASM_X86)
#define HOST_SIZEOF_POINTER 4
#define HOST_BYTEORDER __ORDER_LITTLE_ENDIAN__
#endif /* ... */

#ifndef HOST_BYTEORDER
#define HOST_BYTEORDER __BYTE_ORDER__
#endif /* !HOST_BYTEORDER */


#undef HOSTASM_HAVE_SHRINKJUMPS
#ifdef HOSTASM_X86
#define HOSTASM_HAVE_SHRINKJUMPS
#endif /* HOSTASM_X86 */

#ifdef HOSTASM_X86
#undef LIBGEN86_TARGET_BITS
#define LIBGEN86_TARGET_BITS (HOST_SIZEOF_POINTER * 8)
#include "libgen86/register.h"
#endif /* HOSTASM_X86 */

#define HOSTASM_STACK_GROWS_DOWN

#ifndef HOSTASM_STACK_ALIGNMENT
#define HOSTASM_STACK_ALIGNMENT HOST_SIZEOF_POINTER
#endif /* !HOSTASM_STACK_ALIGNMENT */
#ifndef HOSTASM_SCRACHAREA_SIZE
#define HOSTASM_SCRACHAREA_SIZE 0
#endif /* !HOSTASM_SCRACHAREA_SIZE */

#ifndef HOSTASM_REDZONE_SIZE
#define HOSTASM_REDZONE_SIZE 0
#elif defined(__KERNEL__)
#undef HOSTASM_REDZONE_SIZE
#define HOSTASM_REDZONE_SIZE 0
#endif /* ... */


/* Figure out how many scratch registers this host gives us. */
#ifdef HOSTASM_X86_64_MSABI
#define HOST_REGNO_RETURN HOST_REGNO_RAX
#define HOST_REGNO_RAX    0
#define HOST_REGNO_RCX    1
#define HOST_REGNO_RDX    2
#define HOST_REGNO_R8     3
#define HOST_REGNO_R9     4
#define HOST_REGNO_R10    5
#define HOST_REGNO_R11    6
#define HOST_REGNO_COUNT  7 /* %rax, %rcx, %rdx, %r8, %r9, %r10, %r11 */
#elif defined(HOSTASM_X86_64_SYSVABI)
#define HOST_REGNO_RETURN HOST_REGNO_RAX
#define HOST_REGNO_RAX    0
#define HOST_REGNO_RCX    1
#define HOST_REGNO_RDX    2
#define HOST_REGNO_RDI    3
#define HOST_REGNO_RSI    4
#define HOST_REGNO_R8     5
#define HOST_REGNO_R9     6
#define HOST_REGNO_R10    7
#define HOST_REGNO_R11    8
#define HOST_REGNO_COUNT  9 /* %rax, %rcx, %rdx, %rdi, %rsi, %r8, %r9, %r10, %r11 */
#elif defined(HOSTASM_X86)
#define HOST_REGNO_RETURN HOST_REGNO_EAX
#define HOST_REGNO_EAX    0
#define HOST_REGNO_ECX    1
#define HOST_REGNO_EDX    2
#define HOST_REGNO_COUNT  3 /* %eax, %ecx, %edx */
#endif /* ... */


/* Arguments registers in the `DCALL' ABI */
#ifdef HOSTASM_X86_64_MSABI
#define HOST_REGNO_R_ARG0 HOST_REGNO_RCX
#define HOST_REGNO_R_ARG1 HOST_REGNO_RDX
#define HOST_REGNO_R_ARG2 HOST_REGNO_R8
#define HOST_REGNO_R_ARG3 HOST_REGNO_R9
#elif defined(HOSTASM_X86_64_SYSVABI)
#define HOST_REGNO_R_ARG0 HOST_REGNO_RDI
#define HOST_REGNO_R_ARG1 HOST_REGNO_RSI
#define HOST_REGNO_R_ARG2 HOST_REGNO_RDX
#define HOST_REGNO_R_ARG3 HOST_REGNO_RCX
#define HOST_REGNO_R_ARG4 HOST_REGNO_R8
#define HOST_REGNO_R_ARG5 HOST_REGNO_R9
#endif /* ... */


DECL_BEGIN

#define Dee_function_object function_object
struct Dee_function_object;


typedef uint8_t host_regno_t;

/* Host function assembly calling convention (one of `HOST_CC_*'). */
typedef uint8_t host_cc_t;

/* Possible calling convention flags. */
#define HOST_CC_F_KW    1 /* Take an an extra `DeeObject *kw' parameter for keyword arguments */
#define HOST_CC_F_TUPLE 2 /* Instead of `size_t argc, DeeObject *const *argv', take `DeeObject *args' */
#define HOST_CC_F_FUNC  4 /* Don't hard-code "ref" or "this_function" operands (for lambda functions) */
#define HOST_CC_F_THIS  8 /* Take an an extra `DeeObject *thisarg' parameter for "this" */

/* Possible calling conventions. */
#define HOST_CC_CALL                   0  /* DREF DeeObject *(DCALL *)(size_t argc, DeeObject *const *argv); */
#define HOST_CC_CALL_KW                1  /* DREF DeeObject *(DCALL *)(size_t argc, DeeObject *const *argv, DeeObject *kw); */
#define HOST_CC_CALL_TUPLE             2  /* DREF DeeObject *(DCALL *)(DeeObject *args); */
#define HOST_CC_CALL_TUPLE_KW          3  /* DREF DeeObject *(DCALL *)(DeeObject *args, DeeObject *kw); */
#define HOST_CC_FUNC_CALL              4  /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, size_t argc, DeeObject *const *argv); */
#define HOST_CC_FUNC_CALL_KW           5  /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, size_t argc, DeeObject *const *argv, DeeObject *kw); */
#define HOST_CC_FUNC_CALL_TUPLE        6  /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, DeeObject *args); */
#define HOST_CC_FUNC_CALL_TUPLE_KW     7  /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, DeeObject *args, DeeObject *kw); */
#define HOST_CC_THISCALL               8  /* DREF DeeObject *(DCALL *)(DeeObject *thisarg, size_t argc, DeeObject *const *argv); */
#define HOST_CC_THISCALL_KW            9  /* DREF DeeObject *(DCALL *)(DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw); */
#define HOST_CC_THISCALL_TUPLE         10 /* DREF DeeObject *(DCALL *)(DeeObject *thisarg, DeeObject *args); */
#define HOST_CC_THISCALL_TUPLE_KW      11 /* DREF DeeObject *(DCALL *)(DeeObject *thisarg, DeeObject *args, DeeObject *kw); */
#define HOST_CC_FUNC_THISCALL          12 /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv); */
#define HOST_CC_FUNC_THISCALL_KW       13 /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw); */
#define HOST_CC_FUNC_THISCALL_TUPLE    14 /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, DeeObject *thisarg, DeeObject *args); */
#define HOST_CC_FUNC_THISCALL_TUPLE_KW 15 /* DREF DeeObject *(DCALL *)(DeeFunctionObject *func, DeeObject *thisarg, DeeObject *args, DeeObject *kw); */
#define HOST_CC_COUNT                  16
union host_rawfunc_entry {
	void                   *hfe_addr;
	DREF DeeObject *(DCALL *hfe_call)(size_t argc, DeeObject *const *argv);                                                                              /* HOST_CC_CALL */
	DREF DeeObject *(DCALL *hfe_call_kw)(size_t argc, DeeObject *const *argv, DeeObject *kw);                                                            /* HOST_CC_CALL_KW */
	DREF DeeObject *(DCALL *hfe_call_tuple)(DeeObject *args);                                                                                            /* HOST_CC_CALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_call_tuple_kw)(DeeObject *args, DeeObject *kw);                                                                          /* HOST_CC_CALL_TUPLE_KW */
	DREF DeeObject *(DCALL *hfe_func_call)(struct function_object *func, size_t argc, DeeObject *const *argv);                                           /* HOST_CC_FUNC_CALL */
	DREF DeeObject *(DCALL *hfe_func_call_kw)(struct function_object *func, size_t argc, DeeObject *const *argv, DeeObject *kw);                         /* HOST_CC_FUNC_CALL_KW */
	DREF DeeObject *(DCALL *hfe_func_call_tuple)(struct function_object *func, DeeObject *args);                                                         /* HOST_CC_FUNC_CALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_func_call_tuple_kw)(struct function_object *func, DeeObject *args, DeeObject *kw);                                       /* HOST_CC_FUNC_CALL_TUPLE_KW */
	DREF DeeObject *(DCALL *hfe_thiscall)(DeeObject *thisarg, size_t argc, DeeObject *const *argv);                                                      /* HOST_CC_THISCALL */
	DREF DeeObject *(DCALL *hfe_thiscall_kw)(DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);                                    /* HOST_CC_THISCALL_KW */
	DREF DeeObject *(DCALL *hfe_thiscall_tuple)(DeeObject *thisarg, DeeObject *args);                                                                    /* HOST_CC_THISCALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_thiscall_tuple_kw)(DeeObject *thisarg, DeeObject *args, DeeObject *kw);                                                  /* HOST_CC_THISCALL_TUPLE_KW */
	DREF DeeObject *(DCALL *hfe_func_thiscall)(struct function_object *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv);                   /* HOST_CC_FUNC_THISCALL */
	DREF DeeObject *(DCALL *hfe_func_thiscall_kw)(struct function_object *func, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw); /* HOST_CC_FUNC_THISCALL_KW */
	DREF DeeObject *(DCALL *hfe_func_thiscall_tuple)(struct function_object *func, DeeObject *thisarg, DeeObject *args);                                 /* HOST_CC_FUNC_THISCALL_TUPLE */
	DREF DeeObject *(DCALL *hfe_func_thiscall_tuple_kw)(struct function_object *func, DeeObject *thisarg, DeeObject *args, DeeObject *kw);               /* HOST_CC_FUNC_THISCALL_TUPLE_KW */
};


struct host_rawfunc {
	union host_rawfunc_entry rhf_entry; /* Function entry point. */
	void                   *_rhf_base;  /* Mmap base address. */
	size_t                  _rhf_size;  /* Mmap size address. */
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
#define host_rawfunc_init(self, size)                       \
	(((self)->_rhf_size = CEIL_ALIGN(size, getpagesize())), \
	 ((self)->_rhf_base = VirtualAlloc(NULL, (self)->_rhf_size, MEM_COMMIT, PAGE_READWRITE)) != NULL ? 0 : -1)
#define host_rawfunc_fini(self)   (void)VirtualFree((self)->_rhf_base, 0, MEM_RELEASE)
#define host_rawfunc_mkexec(self) host_rawfunc_mkexec(self)
LOCAL WUNUSED NONNULL((1)) int (DCALL host_rawfunc_mkexec)(struct host_rawfunc *__restrict self) {
	DWORD dwTemp;
	return VirtualProtect(self->_rhf_base, self->_rhf_size, PAGE_EXECUTE_READ, &dwTemp) ? 0 : -1;
}
#elif defined(CONFIG_hostfunc_USES_mmap)
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)(uintptr_t)-1)
#endif /* !MAP_FAILED */
#define host_rawfunc_init(self, size)                                           \
	(((self)->_rhf_size = CEIL_ALIGN(size, getpagesize())),                     \
	 ((self)->_rhf_base = mmap(NULL, (self)->_rhf_size, PROT_READ | PROT_WRITE, \
	                           MAP_PRIVATE | MAP_ANON, -1, 0)) != MAP_FAILED ? 0 : -1)
#define host_rawfunc_fini(self)   (void)munmap((self)->_rhf_base, (self)->_rhf_size)
#define host_rawfunc_mkexec(self) mprotect((self)->_rhf_base, (self)->_rhf_size, PROT_READ | PROT_EXEC)
#else /* ... */
#define host_rawfunc_init(self, size)                       \
	(((self)->_rhf_size = CEIL_ALIGN(size, getpagesize())), \
	 ((self)->_rhf_base = Dee_TryMalloc((self)->_rhf_size)) != NULL ? 0 : -1)
#define host_rawfunc_fini(self)   Dee_Free((self)->_rhf_base)
#define host_rawfunc_mkexec(self) 0
#endif /* !... */

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_HOST_H */
