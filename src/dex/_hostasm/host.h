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
#ifndef GUARD_DEX_HOSTASM_HOST_H
#define GUARD_DEX_HOSTASM_HOST_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/system-features.h>

#include <hybrid/align.h>
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
#define HOST_SIZEOF_POINTER 8
#elif defined(HOSTASM_X86)
#define HOST_SIZEOF_POINTER 4
#endif /* ... */


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


/* Arguments registers in the `DCALL' ABI */
#ifdef HOSTASM_X86_64_MSABI
#define HOST_REGISTER_R_ARG0 HOST_REGISTER_RCX
#define HOST_REGISTER_R_ARG1 HOST_REGISTER_RDX
#define HOST_REGISTER_R_ARG2 HOST_REGISTER_R8
#define HOST_REGISTER_R_ARG3 HOST_REGISTER_R9
#elif defined(HOSTASM_X86_64_SYSVABI)
#define HOST_REGISTER_R_ARG0 HOST_REGISTER_RDI
#define HOST_REGISTER_R_ARG1 HOST_REGISTER_RSI
#define HOST_REGISTER_R_ARG2 HOST_REGISTER_RDX
#define HOST_REGISTER_R_ARG3 HOST_REGISTER_RCX
#endif /* ... */


DECL_BEGIN

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
	void                   *hfe_addr;
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
	DREF DeeObject         **hf_refs;  /* [1..1][0..n][owned] Inlined object references (terminated by a trailing NULL). */
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
#define _Dee_hostfunc_fini(self)  (void)VirtualFree((self)->_hf_base, 0, MEM_RELEASE)
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
#define _Dee_hostfunc_fini(self)  (void)munmap((self)->_hf_base, (self)->_hf_size)
#define Dee_hostfunc_mkexec(self) mprotect((self)->_hf_base, (self)->_hf_size, PROT_READ | PROT_EXEC)
#else /* ... */
#define Dee_hostfunc_init(self, size)                      \
	(((self)->_hf_size = CEIL_ALIGN(size, getpagesize())), \
	 ((self)->_hf_base = Dee_TryMalloc((self)->_hf_size)) != NULL ? 0 : -1)
#define _Dee_hostfunc_fini(self)  Dee_Free((self)->_hf_base)
#define Dee_hostfunc_mkexec(self) 0
#endif /* !... */

LOCAL NONNULL((1)) void DCALL
Dee_hostfunc_fini(struct Dee_hostfunc *__restrict self) {
	_Dee_hostfunc_fini(self);
#ifndef __INTELLISENSE__
	if (self->hf_refs) {
		size_t i;
		DREF DeeObject *ob;
		for (i = 0; (ob = self->hf_refs[i]) != NULL; ++i)
			Dee_Decref(ob);
		Dee_Free(self->hf_refs);
	}
#endif /* !__INTELLISENSE__ */
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_HOST_H */
