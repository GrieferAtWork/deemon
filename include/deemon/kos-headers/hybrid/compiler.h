/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef __GUARD_HYBRID_COMPILER_H
#define __GUARD_HYBRID_COMPILER_H 1

#include <__stdinc.h>
#ifndef __KOS_VERSION__
#define __KOS_VERSION__ 200 /* Legacy model. */
#endif

#define ATTR_NOINLINE       __ATTR_NOINLINE
#define ATTR_NORETURN       __ATTR_NORETURN
#define ATTR_FASTCALL       __ATTR_FASTCALL
#define ATTR_STDCALL        __ATTR_STDCALL
#define ATTR_CDECL          __ATTR_CDECL
#define ATTR_SYSVABI        __ATTR_SYSVABI
#define ATTR_MSABI          __ATTR_MSABI
#define ATTR_PURE           __ATTR_PURE
#define ATTR_CONST          __ATTR_CONST
#define ATTR_MALLOC         __ATTR_MALLOC
#define ATTR_HOT            __ATTR_HOT
#define ATTR_COLD           __ATTR_COLD
#define ATTR_WEAK           __ATTR_WEAK
#define ATTR_ALLOC_SIZE     __ATTR_ALLOC_SIZE
#define ATTR_ASSUME_ALIGNED __ATTR_ASSUME_ALIGNED
#define ATTR_ALLOC_ALIGN    __ATTR_ALLOC_ALIGN
#define ATTR_NOTHROW        __ATTR_NOTHROW
#define ATTR_DLLIMPORT      __ATTR_DLLIMPORT
#define ATTR_DLLEXPORT      __ATTR_DLLEXPORT
#define ATTR_NOCLONE        __ATTR_NOCLONE
#define ATTR_USED           __ATTR_USED
#define ATTR_UNUSED         __ATTR_UNUSED
#define ATTR_SENTINEL       __ATTR_SENTINEL
#define ATTR_SENTINEL_O     __ATTR_SENTINEL_O
#define ATTR_THREAD         __ATTR_THREAD
#define ATTR_DEPRECATED     __ATTR_DEPRECATED
#define ATTR_WARNING        __ATTR_WARNING
#define ATTR_ERROR          __ATTR_ERROR
#define ATTR_SECTION        __ATTR_SECTION
#define ATTR_RETNONNULL     __ATTR_RETNONNULL
#define ATTR_ALIGNED        __ATTR_ALIGNED
#define ATTR_ALIAS          __ATTR_ALIAS
#define ATTR_INLINE         __ATTR_INLINE
#define ATTR_FORCEINLINE    __ATTR_FORCEINLINE
#define ATTR_FALLTHROUGH    __ATTR_FALLTHROUGH

#define COMPILER_LENOF      __COMPILER_LENOF
#define COMPILER_ENDOF      __COMPILER_ENDOF
#define COMPILER_STRLEN     __COMPILER_STRLEN
#define COMPILER_STREND     __COMPILER_STREND
#define COMPILER_UNUSED     __COMPILER_UNUSED
#define COMPILER_UNIPOINTER __COMPILER_UNIPOINTER
#define COMPILER_ALIGNOF    __COMPILER_ALIGNOF
#define COMPILER_OFFSETOF   __builtin_offsetof
#define COMPILER_OFFSETAFTER __COMPILER_OFFSETAFTER
#define COMPILER_CONTAINER_OF __COMPILER_CONTAINER_OF
#define COMPILER_BARRIER    __COMPILER_BARRIER
#define COMPILER_READ_BARRIER __COMPILER_READ_BARRIER
#define COMPILER_WRITE_BARRIER __COMPILER_WRITE_BARRIER
#define COMPILER_IGNORE_UNINITIALIZED __COMPILER_IGNORE_UNINITIALIZED

#define DEFINE_PRIVATE_ALIAS      __DEFINE_PRIVATE_ALIAS
#define DEFINE_PUBLIC_ALIAS       __DEFINE_PUBLIC_ALIAS
#define DEFINE_INTERN_ALIAS       __DEFINE_INTERN_ALIAS
#define DEFINE_PRIVATE_WEAK_ALIAS __DEFINE_PRIVATE_WEAK_ALIAS
#define DEFINE_PUBLIC_WEAK_ALIAS  __DEFINE_PUBLIC_WEAK_ALIAS
#define DEFINE_INTERN_WEAK_ALIAS  __DEFINE_INTERN_WEAK_ALIAS

#define likely              __likely
#define unlikely            __unlikely

#define DECL_BEGIN          __DECL_BEGIN
#define DECL_END            __DECL_END
//#define SYSDECL_BEGIN     __SYSDECL_BEGIN /* Not defined to discourage use in anything but system headers. */
//#define SYSDECL_END       __SYSDECL_END
#define ASMNAME             __ASMNAME
#define PACKED              __ATTR_PACKED
#define FCALL               __FCALL
#define KCALL               __KCALL
#define XBLOCK              __XBLOCK
#define XRETURN             __XRETURN
#define NOTHROW             __NOTHROW

#ifdef __REDIRECT_WSUPPRESS_BEGIN
/* If defined by the compiler, suppress specific warnings within redirections. */
#define REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)                                     __REDIRECT_WSUPPRESS_BEGIN __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) __REDIRECT_WSUPPRESS_END
#define REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                             __REDIRECT_WSUPPRESS_BEGIN __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) __REDIRECT_WSUPPRESS_END
#define REDIRECT_VOID(decl,attr,cc,name,param,asmname,args)                                        __REDIRECT_WSUPPRESS_BEGIN __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) __REDIRECT_WSUPPRESS_END
#define REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                __REDIRECT_WSUPPRESS_BEGIN __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) __REDIRECT_WSUPPRESS_END
#define VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         __REDIRECT_WSUPPRESS_BEGIN __VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __REDIRECT_WSUPPRESS_END
#define VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __REDIRECT_WSUPPRESS_BEGIN __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __REDIRECT_WSUPPRESS_END
#define VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            __REDIRECT_WSUPPRESS_BEGIN __VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __REDIRECT_WSUPPRESS_END
#define VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    __REDIRECT_WSUPPRESS_BEGIN __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __REDIRECT_WSUPPRESS_END
#define XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code)                                    __REDIRECT_WSUPPRESS_BEGIN __XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code) __REDIRECT_WSUPPRESS_END
#define XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                            __REDIRECT_WSUPPRESS_BEGIN __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code) __REDIRECT_WSUPPRESS_END
#define XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code)                                       __REDIRECT_WSUPPRESS_BEGIN __XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code) __REDIRECT_WSUPPRESS_END
#define XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                               __REDIRECT_WSUPPRESS_BEGIN __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code) __REDIRECT_WSUPPRESS_END
#else
#define REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#define REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)
#define REDIRECT_VOID(decl,attr,cc,name,param,asmname,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args)
#define REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)
#define VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         __VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)
#define VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)
#define VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            __VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)
#define VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)
#define XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code)
#define XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)
#define XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code)
#define XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)
#endif

#define PP_PRIVATE_STR      __PP_PRIVATE_STR
#define PP_STR              __PP_STR
#define PP_PRIVATE_CAT2     __PP_PRIVATE_CAT2
#define PP_PRIVATE_CAT3     __PP_PRIVATE_CAT3
#define PP_PRIVATE_CAT4     __PP_PRIVATE_CAT4
#define PP_CAT2             __PP_CAT2
#define PP_CAT3             __PP_CAT3
#define PP_CAT4             __PP_CAT4
#define PP_PRIVATE_MUL8     __PP_PRIVATE_MUL8
#define PP_MUL8             __PP_MUL8
#define STATIC_ASSERT       __STATIC_ASSERT
#define STATIC_ASSERT_MSG   __STATIC_ASSERT_MSG

#define NONNULL             __NONNULL
#define WUNUSED             __WUNUSED
#define UNUSED              __UNUSED

#define CLEARED             __CLEARED      /* Annotation for allocators returning zero-initialized memory. */
#define WEAK                __WEAK         /* Annotation for weakly referenced data/data updated randomly with both the old/new state remaining valid forever. */
#define REF                 __REF          /* Annotation for reference holders. */
#define ATOMIC_DATA         __ATOMIC_DATA  /* Annotation for atomic data. */
#define PAGE_ALIGNED        __PAGE_ALIGNED /* Annotation for page-aligned pointers. */
#define USER                __USER         /* Annotation for user-space memory (default outside kernel). */
#define HOST                __HOST         /* Annotation for kernel-space memory (default within kernel). */
#define VIRT                __VIRT         /* Annotation for virtual memory (default). */
#define PHYS                __PHYS         /* Annotation for physical memory. */
#define MMIO                __MMIO         /* Annotation for memory-mapped I/O-port pointers. */
#define NOIRQ               __NOIRQ        /* Annotation for functions that require interrupts to be disabled. */
#define NOMP                __NOMP         /* Annotation for functions that are not thread-safe and require caller-synchronization. */
#define ASMCALL             __ASMCALL      /* Annotation for functions that are implemented in assembly and require a custom calling convention. */
#define INITDATA            __INITDATA     /* Annotation for data that apart of .free sections, meaning that accessing it is illegal after some specific point in time. */
#define INITCALL            __INITCALL     /* Annotation for functions that apart of .free sections, meaning that calling them is illegal after some specific point in time. */
#if __KOS_VERSION__ >= 300
#define ASYNCSAFE                          /* Annotation for functions that are safe to be called from interrupt handlers. */
#define UNCHECKED                          /* For use with `USER': The pointer has not been checked with `validate_*' functions from `<kernel/user.h>' */
#define CHECKED                            /* For use with `USER': The pointer has already been checked with `validate_*' functions from `<kernel/user.h>' */
#endif
#if __KOS_VERSION__ < 300
#define CRIT                __CRIT         /* Annotation for functions that require `TASK_ISCRIT()' (When called from within the kernel). */
#define SAFE                        /* Annotation for functions that require `TASK_ISSAFE()' (When called from within the kernel). */
#define PERCPU              __PERCPU       /* Annotation for variables that must be accessed using the per-cpu API. */
#ifdef __KERNEL__
#define KPD                                /* Annotation for functions that may only be called when the active page directory
                                            * maps all dynamic physical memory 1-on-1 (aka. all pointers marked as 'PHYS').
                                            * HINT: `pdir_kernel' is (the only?) directory applicable for this. */
#endif
#endif

#define IMPDEF              __IMPDEF
#define EXPDEF              __EXPDEF
#define FUNDEF              __PUBDEF
#define DATDEF              __PUBDEF
#define PUBLIC              __PUBLIC
#define INTERN              __INTERN
#define INTDEF              __INTDEF
#define PRIVATE             __PRIVATE
#define FORCELOCAL          __FORCELOCAL
#define LOCAL               __LOCAL
#define LIBCCALL            __LIBCCALL


#ifdef CONFIG_STRALIGN
#   define ATTR_STRALIGN    __ATTR_ALIGNED(CONFIG_STRALIGN)
#else
#   define ATTR_STRALIGN    /* Nothing */
#endif
#define SECTION_STRING(section,str) \
 (*(char(*)[sizeof(str)/sizeof(char)])XBLOCK({ \
    PRIVATE ATTR_STRALIGN ATTR_SECTION(section) char const _s[] = str; \
    XRETURN &_s; }))

#define __PRIVATE_ARG_PLACEHOLDER_1    ,
#define __PRIVATE_ARG_PLACEHOLDER_true ,
#define __PRIVATE_ARG_PLACEHOLDER_yes  ,
#define __PRIVATE_ARG_PLACEHOLDER_ok   ,
#define __PRIVATE_TAKE_SECOND_ARG_IMPL(x,val,...) val
#define __PRIVATE_TAKE_SECOND_ARG(x) __PRIVATE_TAKE_SECOND_ARG_IMPL x
#define __PRIVATE_IS_TRUE2(x) __PRIVATE_TAKE_SECOND_ARG((x 1,0))
#define __PRIVATE_IS_TRUE(x) __PRIVATE_IS_TRUE2(__PRIVATE_ARG_PLACEHOLDER_##x)
#define __PRIVATE_OR_0(y)   __PRIVATE_IS_TRUE(y)
#define __PRIVATE_OR_1(y)   1
#define __PRIVATE_OR2(x,y)  __PRIVATE_OR_##x(y)
#define __PRIVATE_OR(x,y)   __PRIVATE_OR2(x,y)
#define __PRIVATE_AND_0(y)  0
#define __PRIVATE_AND_1(y)  __PRIVATE_IS_TRUE(y)
#define __PRIVATE_AND2(x,y) __PRIVATE_OR_##x(y)
#define __PRIVATE_AND(x,y)  __PRIVATE_OR2(x,y)

#define __is_true(x)    __PRIVATE_IS_TRUE(x)
#define __or(x,y)       __PRIVATE_OR(__PRIVATE_IS_TRUE(x),y)
#define __and(x,y)      __PRIVATE_AND(__PRIVATE_IS_TRUE(x),y)

#ifdef __KERNEL__
#   define IS_BUILTIN(x) __PRIVATE_IS_TRUE(x)
#ifdef CONFIG_BUILDING_KERNEL_CORE
#   define IS_ENABLED(x) __PRIVATE_IS_TRUE(x)
#else
#   define IS_ENABLED(x) __or(x,x##_MODULE)
#endif
#endif


#ifdef __GUARD_HYBRID_LIMITS_H
#ifndef PAGESIZE
#ifdef __PAGESIZE
#   define PAGESIZE         __PAGESIZE
#endif
#endif /* !PAGESIZE */
#ifdef __CACHELINE
#   define CACHELINE        __CACHELINE
#   define CACHELINE_ALIGNED ATTR_ALIGNED(CACHELINE)
#else
#   define CACHELINE_ALIGNED /* Nothing */
#endif
#endif /* __GUARD_HYBRID_LIMITS_H */
#ifdef __CC__
#ifdef __GUARD_HYBRID_DEBUGINFO_H
#   define DEBUGINFO         __DEBUGINFO
#   define DEBUGINFO_GEN     __DEBUGINFO_GEN
#   define DEBUGINFO_MUNUSED __DEBUGINFO_MUNUSED
#   define DEBUGINFO_UNUSED  __DEBUGINFO_UNUSED
#   define DEBUGINFO_FWD     __DEBUGINFO_FWD
#   define DEBUGINFO_NUL     __DEBUGINFO_NUL
#   define DEBUGINFO_MK(file,line,func) __DEBUGINFO_MK(file,line,func)
#endif /* __GUARD_HYBRID_DEBUGINFO_H */
#ifdef __GUARD_HYBRID_CHECK_H
#   define OK_USER_TEXT     __OK_USER_TEXT
#   define OK_HOST_TEXT     __OK_HOST_TEXT
#   define OK_USER_DATA     __OK_USER_DATA
#   define OK_HOST_DATA     __OK_HOST_DATA
#   define CHECK_HOST_TEXT  __CHECK_HOST_TEXT
#   define CHECK_HOST_DATA  __CHECK_HOST_DATA
#   define CHECK_USER_TEXT  __CHECK_USER_TEXT
#   define CHECK_USER_DATA  __CHECK_USER_DATA
#   define CHECK_USER_TOBJ  __CHECK_USER_TOBJ
#   define CHECK_HOST_TOBJ  __CHECK_HOST_TOBJ
#   define CHECK_USER_DOBJ  __CHECK_USER_DOBJ
#   define CHECK_HOST_DOBJ  __CHECK_HOST_DOBJ
#endif /* __GUARD_HYBRID_CHECK_H */
#ifdef __GUARD_HYBRID_TIMESPEC_H
#   define TIMESPEC_ADD           __TIMESPEC_ADD
#   define TIMESPEC_SUB           __TIMESPEC_SUB
#   define TIMESPEC_LOWER         __TIMESPEC_LOWER
#   define TIMESPEC_LOWER_EQUAL   __TIMESPEC_LOWER_EQUAL
#   define TIMESPEC_EQUAL         __TIMESPEC_EQUAL
#   define TIMESPEC_NOT_EQUAL     __TIMESPEC_NOT_EQUAL
#   define TIMESPEC_GREATER       __TIMESPEC_GREATER
#   define TIMESPEC_GREATER_EQUAL __TIMESPEC_GREATER_EQUAL
#endif /* __GUARD_HYBRID_TIMESPEC_H */
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_COMPILER_H */
