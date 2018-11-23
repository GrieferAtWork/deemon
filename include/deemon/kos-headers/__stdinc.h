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
#ifndef ___STDINC_H
#define ___STDINC_H 1

#ifndef __NO_KOS_SYSTEM_HEADERS__
/* Indicator for user-applications that KOS's system headers are available.
 * >> #include <string.h> // Will define `__KOS_SYSTEM_HEADERS__'
 * >> #ifdef __KOS_SYSTEM_HEADERS__
 * >> #include <hybrid/compiler.h> // Pull in KOS-specific headers without relying on `__has_include'.
 * >> #endif
 */
#define __KOS_SYSTEM_HEADERS__ 1
#endif /* !__NO_KOS_SYSTEM_HEADERS__ */

/* ... */

#if defined(__cplusplus) || defined(__INTELLISENSE__) || \
  (!defined(__LINKER__) && !defined(__ASSEMBLY__) && \
   !defined(__ASSEMBLER__) && !defined(__assembler) && \
   !defined(__DEEMON__))
#define __CC__ 1 /* C Compiler. */
#define __CCAST(T) (T)
#else
#define __CCAST(T) /* nothing */
#endif

#include "compiler/pp-generic.h"

#define __COMPILER_LENOF(arr)          (sizeof(arr)/sizeof(*(arr)))
#define __COMPILER_ENDOF(arr)   ((arr)+(sizeof(arr)/sizeof(*(arr))))
#define __COMPILER_STRLEN(str)         (sizeof(str)/sizeof(char)-1)
#define __COMPILER_STREND(str)  ((str)+(sizeof(str)/sizeof(char)-1))

#if !defined(__CC__)
#   include "compiler/other.h"
#elif defined(__GNUC__)
#   include "compiler/gcc.h"
#elif defined(_MSC_VER)
#   include "compiler/msvc.h"
#else
#   include "compiler/generic.h"
#endif

#ifdef __cplusplus
#   include "compiler/c++.h"
#else
#   include "compiler/c.h"
#endif

#ifndef __has_include
#define __NO_has_include 1
#ifdef __PREPROCESSOR_HAVE_VA_ARGS
#define __has_include(...) 0
#else
#define __has_include(x)   0
#endif
#endif
#ifndef __has_include_next
#define __NO_has_include_next 1
#ifdef __PREPROCESSOR_HAVE_VA_ARGS
#define __has_include_next(...) 0
#else
#define __has_include_next(x)   0
#endif
#endif


#ifndef __has_builtin
#define __NO_has_builtin 1
#define __has_builtin(x) 0
#endif
#ifndef __has_feature
#define __NO_has_feature 1
#define __has_feature(x) 0
#endif
#ifndef __has_extension
#define __NO_has_extension 1
#define __has_extension  __has_feature
#endif
#ifndef __has_attribute
#define __NO_has_attribute 1
#define __has_attribute(x) 0
#endif
#ifndef __has_declspec_attribute
#define __NO_has_declspec_attribute 1
#define __has_declspec_attribute(x) 0
#endif
#ifndef __has_cpp_attribute
#define __NO_has_cpp_attribute 1
#define __has_cpp_attribute(x) 0
#endif

#ifndef __SYSDECL_BEGIN
#define __SYSDECL_BEGIN __DECL_BEGIN
#define __SYSDECL_END   __DECL_END
#endif /* !__SYSDECL_BEGIN */

#ifdef __INTELLISENSE__
#   define __NOTHROW       /* Nothing */
#elif defined(__cplusplus)
#   define __NOTHROW(prot) prot __CXX_NOEXCEPT
#elif defined(__NO_ATTR_NOTHROW)
#   define __NOTHROW(prot) prot
//#elif defined(__NO_ATTR_NOTHROW_SUFFIX)
//# define __NOTHROW(prot) __ATTR_NOTHROW prot
#else
#   define __NOTHROW(prot) __ATTR_NOTHROW prot
#endif

#define __FCALL                  __ATTR_FASTCALL
#define __KCALL                  __ATTR_STDCALL

#if defined(__COMPILER_HAVE_AUTOTYPE) && !defined(__NO_XBLOCK)
#   define __COMPILER_UNUSED(expr)  __XBLOCK({ __auto_type __expr = (expr); __expr; })
#elif defined(__COMPILER_HAVE_TYPEOF) && !defined(__NO_XBLOCK)
#   define __COMPILER_UNUSED(expr)  __XBLOCK({ __typeof__(expr) __expr = (expr); __expr; })
#else
#   define __COMPILER_UNUSED(expr) (expr)
#endif

#define __COMPILER_OFFSETAFTER(s,m) ((__SIZE_TYPE__)(&((s *)0)->m+1))
#define __COMPILER_CONTAINER_OF(ptr,type,member) \
  ((type *)((__UINTPTR_TYPE__)(ptr)-__builtin_offsetof(type,member)))
#ifndef __DEFINE_PUBLIC_ALIAS
#ifdef __COMPILER_HAVE_GCC_ASM
#   define __DEFINE_ALIAS_STR(x) #x
#   define __DEFINE_PRIVATE_ALIAS(new,old) __asm__(".local " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_PUBLIC_ALIAS(new,old)  __asm__(".global " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_INTERN_ALIAS(new,old)  __asm__(".global " __DEFINE_ALIAS_STR(new) "\n.hidden " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#else
#   define __NO_DEFINE_ALIAS 1
#endif
#endif /* !__DEFINE_PUBLIC_ALIAS */
#ifdef __NO_ATTR_ALIAS
#   define __ALIAS_IMPL(old,args) { return old args; }
#   define __ALIAS_FUNC(decl_new,new,decl_old,old,param,args) \
           decl_old old param; \
           decl_new new param { return old args; }
#   define __ALIAS_SYMBOL      __DEFINE_PUBLIC_ALIAS
#else
#   define __ALIAS_IMPL(new,args) __ATTR_ALIAS(#new)
#   define __ALIAS_FUNC(decl_new,new,decl_old,old,param,args) \
           decl_old old param; \
           decl_new new param __ATTR_ALIAS(#old)
#   define __ALIAS_SYMBOL(new,old) \
           __typeof__(old) new __ATTR_ALIAS(#old)
#endif

#if !defined(__PE__) && !defined(__ELF__)
/* Try to determine current binary format using other platform
 * identifiers. (When KOS headers are used on other systems) */
#if defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__MINGW32__)
#   define __PE__  1
#elif defined(__linux__) || defined(__linux) || defined(linux) || \
      defined(__unix__) || defined(__unix) || defined(unix)
#   define __ELF__ 1
#elif defined(__WINDOWS__) || \
      defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
      defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
      defined(_WIN32_WCE) || defined(WIN32_WCE)
#   define __PE__  1
#else
#   warning "Target binary format not defined. - Assuming `__ELF__'"
#   define __ELF__ 1
#endif
#endif

#ifndef __CC__
#   define __IMPDEF        /* Nothing */
#   define __EXPDEF        /* Nothing */
#   define __PUBDEF        /* Nothing */
#   define __PRIVATE       /* Nothing */
#   define __INTDEF        /* Nothing */
#   define __PUBLIC        /* Nothing */
#   define __INTERN        /* Nothing */
#   define __PUBLIC_CONST  /* Nothing */
#   define __INTERN_CONST  /* Nothing */
#elif defined(__PE__)
#   define __IMPDEF        extern __ATTR_DLLIMPORT
#   define __EXPDEF        extern __ATTR_DLLEXPORT
#   define __PUBDEF        extern
#   define __PRIVATE       static
#   define __INTDEF        extern
#   define __PUBLIC        __ATTR_DLLEXPORT
#   define __INTERN        /* Nothing */
#ifdef __cplusplus
#   define __PUBLIC_CONST  extern __ATTR_DLLEXPORT
#   define __INTERN_CONST  extern /* Nothing */
#else
#   define __PUBLIC_CONST  __ATTR_DLLEXPORT
#   define __INTERN_CONST  /* Nothing */
#endif
#else
#   define __IMPDEF        extern __ATTR_VISIBILITY("default")
#   define __EXPDEF        extern __ATTR_VISIBILITY("default")
#   define __PUBDEF        extern __ATTR_VISIBILITY("default")
#   define __PUBLIC               __ATTR_VISIBILITY("default")
#   define __PRIVATE       static
#   define __INTDEF        extern __ATTR_VISIBILITY("hidden")
#   define __INTERN               __ATTR_VISIBILITY("hidden")
#ifdef __cplusplus
#   define __PUBLIC_CONST  extern __ATTR_VISIBILITY("default")
#   define __INTERN_CONST  extern __ATTR_VISIBILITY("hidden")
#else
#   define __PUBLIC_CONST         __ATTR_VISIBILITY("default")
#   define __INTERN_CONST         __ATTR_VISIBILITY("hidden")
#endif
#endif


#ifdef __INTELLISENSE__
#   define __UNUSED         /* Nothing */
#elif defined(__cplusplus) || defined(__DEEMON__)
#   define __UNUSED(name)   /* Nothing */
#elif !defined(__NO_ATTR_UNUSED)
#   define __UNUSED(name)   name __ATTR_UNUSED
#elif defined(__LCLINT__)
#   define __UNUSED(name)   /*@unused@*/ name
#elif defined(_MSC_VER)
#   define __UNUSED(name)   name
#   pragma warning(disable: 4100)
#else
#   define __UNUSED(name)   name
#endif

#define __IGNORE_REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#define __IGNORE_REDIRECT_VOID(decl,attr,cc,name,param,asmname,args)
#define __NOREDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn (cc name) __P(param);
#define __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW((cc name) __P(param));
#define __NOREDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void (cc name) __P(param);
#define __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW((cc name) __P(param));

/* General purpose redirection implementation. */
#ifndef __REDIRECT
#ifdef __INTELLISENSE__
/* Only declare the functions for intellisense to minimize IDE lag. */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn (cc name) param;
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW((cc name) param);
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void (cc name) param;
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW((cc name) param);
#elif !defined(__NO_ASMNAME)
/* Use GCC family's assembly name extension. */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn cc name __P(param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW(cc name __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void cc name __P(param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW(cc name __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmname));
#elif defined(__cplusplus)
/* In C++, we can use use namespaces to prevent collisions with incompatible prototypes. */
#define __REDIRECT_UNIQUE  __PP_CAT2(__u,__LINE__)
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn (cc asmname) param; } } } \
__LOCAL attr Treturn (cc name) param { \
    return (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn __NOTHROW((cc asmname) param); } } } \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    return (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl void (cc asmname) param; } } } \
__LOCAL attr void (cc name) param { \
    (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
namespace __int { namespace __REDIRECT_UNIQUE { extern "C" { decl void __NOTHROW((cc asmname) param); } } } \
__LOCAL attr void __NOTHROW((cc name) param) { \
    (__int::__REDIRECT_UNIQUE:: asmname) args; \
}
#else
/* Fallback: Assume that the compiler supports scoped declarations,
 *           as well as deleting them once the scope ends.
 * NOTE: GCC actually doesn't support this one, somehow keeping
 *       track of the C declaration types even after the scope ends,
 *       causing it to fail fatal()-style with incompatible-prototype errors.
 * HINT: Function implementation does how ever work for MSVC when compiling for C.
 */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
__LOCAL attr Treturn (cc name) param { \
    decl Treturn (cc asmname) param; \
    return (asmname) args; \
}
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    decl Treturn __NOTHROW((cc asmname) param); \
    return (asmname) args; \
}
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
__LOCAL attr void (cc name) param { \
    decl void (cc asmname) param; \
    (asmname) args; \
}
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
__LOCAL attr void __NOTHROW((cc name) param) { \
    decl void __NOTHROW((cc asmname) param); \
    (asmname) args; \
}
#endif
#endif /* !__REDIRECT */


#ifdef __KOS_SYSTEM_HEADERS__
#ifdef __KERNEL__
#   undef NDEBUG
#ifndef CONFIG_DEBUG
#   define NDEBUG 1
#endif
#else
#ifdef __CC__
__NAMESPACE_STD_BEGIN struct __IO_FILE;
__NAMESPACE_STD_END
#endif /* __CC__ */
#ifndef __FILE
#define __FILE     struct __NAMESPACE_STD_SYM __IO_FILE
#endif /* !__FILE */
#endif
#endif /* __KOS_SYSTEM_HEADERS__ */

#ifndef __LIBCCALL
#ifdef __KERNEL__
#   define __LIBCCALL __KCALL
#else
#   define __LIBCCALL /* Nothing */
#   define __LIBCCALL_CALLER_CLEANUP 1
#endif
#endif

#ifndef __LIBC
#define __LIBC    __IMPDEF
#endif

/* Annotations */
#define __CLEARED      /* Annotation for allocators returning zero-initialized memory. */
#define __WEAK         /* Annotation for weakly referenced data/data updated randomly with both the old/new state remaining valid forever. */
#define __REF          /* Annotation for reference holders. */
#define __ATOMIC_DATA  /* Annotation for atomic data. */
#define __PAGE_ALIGNED /* Annotation for page-aligned pointers. */
#define __USER         /* Annotation for user-space memory (default outside kernel). */
#define __HOST         /* Annotation for kernel-space memory (default within kernel). */
#define __VIRT         /* Annotation for virtual memory (default). */
#define __PHYS         /* Annotation for physical memory. */
#define __MMIO         /* Annotation for memory-mapped I/O-port pointers. */
#define __CRIT         /* Annotation for functions that require `TASK_ISCRIT()' (When called from within the kernel). */
#define __SAFE         /* Annotation for functions that require `TASK_ISSAFE()' (When called from within the kernel). */
#define __NOIRQ        /* Annotation for functions that require interrupts to be disabled. */
#define __NOMP         /* Annotation for functions that are not thread-safe and require caller-synchronization. */
#define __PERCPU       /* Annotation for variables that must be accessed using the per-cpu API. */
#define __ASMCALL      /* Annotation for functions that are implemented in assembly and require a custom calling convention. */
#define __INITDATA     /* Annotation for data that apart of .free sections, meaning that accessing it is illegal after some specific point in time. */
#define __INITCALL     /* Annotation for functions that apart of .free sections, meaning that calling it is illegal after some specific point in time. */

#endif /* !___STDINC_H */
