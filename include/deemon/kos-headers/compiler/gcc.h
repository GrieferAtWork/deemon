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

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __GNUC_MINOR__
#   define __GNUC_MINOR__ 0
#endif
#ifndef __GNUC_PATCH__
#ifdef __GNUC_PATCHLEVEL__
#   define __GNUC_PATCH__ __GNUC_PATCHLEVEL__
#else
#   define __GNUC_PATCH__ 0
#endif
#endif
#define __GCC_VERSION_NUM    (__GNUC__*10000+__GNUC_MINOR__*100+__GNUC_PATCH__)
#define __GCC_VERSION(a,b,c) (__GCC_VERSION_NUM >= ((a)*10000+(b)*100+(c)))

#ifdef __STDC__
#   define __P(x) x
#else
#   define __NO_PROTOTYPES 1
#   define __P(x) ()
#endif

#ifndef __INTEL_VERSION__
#ifdef __INTEL_COMPILER
#if __INTEL_COMPILER == 9999
#   define __INTEL_VERSION__ 1200
#else
#   define __INTEL_VERSION__ __INTEL_COMPILER
#endif
#elif defined(__ICL)
#   define __INTEL_VERSION__ __ICL
#elif defined(__ICC)
#   define __INTEL_VERSION__ __ICC
#elif defined(__ECC)
#   define __INTEL_VERSION__ __ECC
#endif
#endif /* !__INTEL_VERSION__ */


#ifndef __likely
#if __has_builtin(__builtin_expect) || \
  (!defined(__clang__) && (!defined(__INTEL_VERSION__) || __INTEL_VERSION__ >= 800))
#   define __likely(x)   (__builtin_expect(!!(x),1))
#   define __unlikely(x) (__builtin_expect(!!(x),0))
#else
#   define __builtin_expect(x,y) (x)
#   define __NO_builtin_expect 1
#   define __likely      /* Nothing */
#   define __unlikely    /* Nothing */
#endif
#endif /* !__likely */

#if defined(__clang__) || !defined(__DARWIN_NO_LONG_LONG)
#define __COMPILER_HAVE_LONGLONG 1
#endif
#define __COMPILER_HAVE_LONGDOUBLE 1
#define __COMPILER_HAVE_TRANSPARENT_STRUCT 1
#define __COMPILER_HAVE_TRANSPARENT_UNION 1
#define __COMPILER_HAVE_PRAGMA_PUSHMACRO 1
#if __has_feature(__tpp_pragma_deprecated__)
#define __COMPILER_HAVE_PRAGMA_DEPRECATED 1
#endif
#ifdef __CC__
#define __COMPILER_HAVE_PRAGMA_PACK 1
#endif
#define __COMPILER_HAVE_GCC_ASM 1
#ifdef __cplusplus
#define __COMPILER_ASM_BUFFER(T,s,p) (*(T(*)[s])(p))
#else
#define __COMPILER_ASM_BUFFER(T,s,p) (*(struct { __extension__ T __d[s]; } *)(p))
#endif

#ifdef __CPROTO__
#include "cproto.h"
#endif

#if 1
/* XXX: When was this added in C? */
#   define __COMPILER_HAVE_AUTOTYPE 1
#elif __has_feature(cxx_auto_type) || \
     (defined(__cplusplus) && __GCC_VERSION(4,4,0))
#     define __auto_type              auto
#     define __COMPILER_HAVE_AUTOTYPE 1
#endif

#if __has_feature(cxx_static_assert) || \
   (__GCC_VERSION(4,3,0) && (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L))
#   define __STATIC_ASSERT(expr)         static_assert(expr,#expr)
#   define __STATIC_ASSERT_MSG(expr,msg) static_assert(expr,msg)
#elif defined(_Static_assert) || __has_feature(c_static_assert) || \
     (!defined(__cplusplus) && ( \
     (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 201112L) || \
     (__GCC_VERSION(4,6,0) && !defined(__STRICT_ANSI__))))
#   define __STATIC_ASSERT(expr)         _Static_assert(expr,#expr)
#   define __STATIC_ASSERT_MSG(expr,msg) _Static_assert(expr,msg)
#elif defined(__TPP_COUNTER)
#   define __STATIC_ASSERT(expr)         extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__TPP_COUNTER(__static_assert))[(expr)?1:-1]
#   define __STATIC_ASSERT_MSG(expr,msg) extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__TPP_COUNTER(__static_assert))[(expr)?1:-1]
#elif defined(__COUNTER__)
#   define __STATIC_ASSERT(expr)         extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__COUNTER__)[(expr)?1:-1]
#   define __STATIC_ASSERT_MSG(expr,msg) extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__COUNTER__)[(expr)?1:-1]
#else
#   define __STATIC_ASSERT(expr)         extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__LINE__)[(expr)?1:-1]
#   define __STATIC_ASSERT_MSG(expr,msg) extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__LINE__)[(expr)?1:-1]
#endif
#ifdef __INTELLISENSE__
#   define __ASMNAME(x)   /* Nothing */
#else
#   define __ASMNAME(x)   __asm__(x)
#endif
//#define __NO_ASMNAME 1 /* TO-DO: Remove me */
#if !__GCC_VERSION(2,7,0)
#ifndef __attribute__
#   define __attribute__(x) /* Nothing */
#endif /* !__attribute__ */
#endif
#if !__GCC_VERSION(2,8,0)
#ifndef __extension__
#   define __extension__
#endif /* !__extension__ */
#endif
#define __COMPILER_HAVE_TYPEOF 1
#if __GCC_VERSION(3,1,0)
#   define __ATTR_NOINLINE         __attribute__((__noinline__))
#else
#   define __NO_ATTR_NOINLINE      1
#   define __ATTR_NOINLINE         /* Nothing */
#endif
#if __GCC_VERSION(2,5,0)
#   define __ATTR_NORETURN         __attribute__((__noreturn__))
#else
#   define __NO_ATTR_NORETURN      1
#   define __ATTR_NORETURN         /* Nothing */
#endif
#if defined(__cplusplus) && __has_cpp_attribute(fallthrough)
#define __ATTR_FALLTHROUGH         [[fallthrough]];
#elif __GCC_VERSION(7,0,0) || __has_attribute(fallthrough)
#define __ATTR_FALLTHROUGH         __attribute__((__fallthrough__));
#else
#define __NO_ATTR_FALLTHROUGH      1
#define __ATTR_FALLTHROUGH         /* Nothing */
#endif
#define __NO_ATTR_W64              1
#define __ATTR_W64                 /* Nothing */
#if (defined(__i386__) || defined(__i386)) && \
    !defined(__x86_64__) && !defined(__x86_64)
#   define __ATTR_FASTCALL         __attribute__((__fastcall__))
#   define __ATTR_STDCALL          __attribute__((__stdcall__))
#if !defined(__INTELLISENSE__)
#   define __ATTR_CDECL            /* [default] */
#else
#   define __ATTR_CDECL            __attribute__((__cdecl__))
#endif
#else
#   define __NO_ATTR_FASTCALL      1
#   define __ATTR_FASTCALL         /* Nothing */
#   define __NO_ATTR_STDCALL       1
#   define __ATTR_STDCALL          /* Nothing */
#   define __NO_ATTR_CDECL         1
#   define __ATTR_CDECL            /* Nothing */
#endif
#if defined(__x86_64__) || defined(__x86_64)
#   define __VA_LIST_IS_ARRAY      1
#   define __ATTR_MSABI            __attribute__((__ms_abi__))
#   define __ATTR_SYSVABI          __attribute__((__sysv_abi__))
#else
#   define __NO_ATTR_MSABI         1
#   define __ATTR_MSABI            /* Nothing */
#   define __NO_ATTR_SYSVABI       1
#   define __ATTR_SYSVABI          /* Nothing */
#endif
#if __GCC_VERSION(2,96,0)
#   define __ATTR_PURE             __attribute__((__pure__))
#else
#   define __NO_ATTR_PURE          1
#   define __ATTR_PURE             /* Nothing */
#endif
#if __GCC_VERSION(2,5,0)
#   define __ATTR_CONST            __attribute__ ((__const__))
#else
#   define __NO_ATTR_CONST         1
#   define __ATTR_CONST            /* Nothing */
#endif
#if __GCC_VERSION(3,0,0) /* __GCC_VERSION(2,96,0) */
#   define __ATTR_MALLOC           __attribute__((__malloc__))
#else
#   define __NO_ATTR_MALLOC        1
#   define __ATTR_MALLOC           /* Nothing */
#endif
#if __GCC_VERSION(4,3,0)
#   define __ATTR_ALLOC_SIZE(ppars) __attribute__((__alloc_size__ ppars))
#else
#   define __NO_ATTR_ALLOC_SIZE     1
#   define __ATTR_ALLOC_SIZE(ppars) /* Nothing */
#endif
#if   __GCC_VERSION(2,7,0)
#   define __ATTR_UNUSED           __attribute__((__unused__))
#else
#   define __NO_ATTR_UNUSED        1
#   define __ATTR_UNUSED           /* Nothing */
#endif
#if   __GCC_VERSION(3,1,0)
#   define __ATTR_USED             __attribute__((__used__))
#else
#   define __NO_ATTR_USED          1
#   define __ATTR_USED             /* Nothing */
#endif
#ifdef __INTELLISENSE__
#   define __ATTR_DEPRECATED_      __declspec(deprecated)
#   define __ATTR_DEPRECATED(text) __declspec(deprecated(text))
#elif __GCC_VERSION(3,1,0)
/*  - __GCC_VERSION(3,2,0)
 *  - __GCC_VERSION(3,5,0)
 * The internet isn't unanimous about this one... */
#   define __ATTR_DEPRECATED_      __attribute__((__deprecated__))
#if __GCC_VERSION(4,5,0)
#   define __ATTR_DEPRECATED(text) __attribute__((__deprecated__(text)))
#else
#   define __ATTR_DEPRECATED(text) __attribute__((__deprecated__))
#endif
#else
#   define __NO_ATTR_DEPRECATED    1
#   define __ATTR_DEPRECATED_      /* Nothing */
#   define __ATTR_DEPRECATED(text) /* Nothing */
#endif
#if __GCC_VERSION(3,5,0)
#   define __ATTR_SENTINEL         __attribute__((__sentinel__))
#ifdef __INTELLISENSE__
#   define __ATTR_SENTINEL_O(x)    __attribute__((__sentinel__))
#else
#   define __ATTR_SENTINEL_O(x)    __attribute__((__sentinel__(x)))
#endif
#else
#   define __NO_ATTR_SENTINEL      1
#   define __NO_ATTR_SENTINEL_O    1
#   define __ATTR_SENTINEL         /* Nothing */
#   define __ATTR_SENTINEL_O(x)    /* Nothing */
#endif
#if __GCC_VERSION(4,3,0)
#   define __ATTR_HOT              __attribute__((__hot__))
#   define __ATTR_COLD             __attribute__((__cold__))
#else
#   define __NO_ATTR_HOT           1
#   define __ATTR_HOT              /* Nothing */
#   define __NO_ATTR_COLD          1
#   define __ATTR_COLD             /* Nothing */
#endif
#if __GCC_VERSION(4,5,0)
#   define __ATTR_NOCLONE          __attribute__((__noclone__))
#else
#   define __NO_ATTR_NOCLONE       1
#   define __ATTR_NOCLONE          /* Nothing */
#endif
#if __GCC_VERSION(4,8,0)
#   define __ATTR_THREAD           __thread
#else
#   define __NO_ATTR_THREAD        1
#   define __ATTR_THREAD           /* Nothing */
#endif
#if __GCC_VERSION(4,9,0)
#   define __ATTR_ASSUME_ALIGNED(n) __attribute__((__assume_aligned__(n)))
#else
#   define __NO_ATTR_ASSUME_ALIGNED 1
#   define __ATTR_ASSUME_ALIGNED(n) /* Nothing */
#endif
#if __GCC_VERSION(5,4,0)
#   define __ATTR_ALLOC_ALIGN(pari) __attribute__((__alloc_align__(pari)))
#else
#   define __NO_ATTR_ALLOC_ALIGN   1
#   define __ATTR_ALLOC_ALIGN(pari) /* Nothing */
#endif
#if __GCC_VERSION(3,3,0)
#   define __ATTR_NOTHROW        __attribute__((__nothrow__))
#else
#   define __NO_ATTR_NOTHROW     1
#   define __ATTR_NOTHROW        /* Nothing */
#endif
#if __GCC_VERSION(4,4,0)
#   define __ATTR_OPTIMIZE(opt)  __attribute__((__optimize__(opt)))
#else
#   define __NO_ATTR_OPTIMIZE    1
#   define __ATTR_OPTIMIZE(opt)  /* Nothing */
#endif
#if __GCC_VERSION(2,0,0) && !defined(__cplusplus)
#   define __ATTR_TRANSPARENT_UNION __attribute__((__transparent_union__))
#else
#   define __NO_ATTR_TRANSPARENT_UNION 1
#   define __ATTR_TRANSPARENT_UNION    /* Nothing */
#endif
/* format-printer attributes. */
#if __GCC_VERSION(2,3,0)
#   define __ATTR_FORMAT_PRINTF(fmt,args) __attribute__((__format__(__printf__,fmt,args)))
#else
#   define __NO_ATTR_FORMAT_PRINTF        1
#   define __ATTR_FORMAT_PRINTF(fmt,args) /* Nothing */
#endif
#if !defined(__NO_ATTR_FORMAT_PRINTF) /* TODO: There were added later. - But when exactly? */
#   define __ATTR_FORMAT_SCANF(fmt,args)    __attribute__((__format__(__scanf__,fmt,args)))
#   define __ATTR_FORMAT_STRFMON(fmt,args)  __attribute__((__format__(__strfmon__,fmt,args)))
#   define __ATTR_FORMAT_STRFTIME(fmt,args) __attribute__((__format__(__strftime__,fmt,0)))
#else
#   define __NO_ATTR_FORMAT_SCANF           1
#   define __NO_ATTR_FORMAT_STRFMON         1
#   define __NO_ATTR_FORMAT_STRFTIME        1
#   define __ATTR_FORMAT_SCANF(fmt,args)    /* Nothing */
#   define __ATTR_FORMAT_STRFMON(fmt,args)  /* Nothing */
#   define __ATTR_FORMAT_STRFTIME(fmt,args) /* Nothing */
#endif
#if !defined(__ELF__) && \
    (defined(__PE__) || defined(_WIN32) || defined(__CYGWIN__))
#   define __ATTR_DLLIMPORT      __attribute__((__dllimport__))
#   define __ATTR_DLLEXPORT      __attribute__((__dllexport__))
#else
#   define __NO_ATTR_DLLIMPORT   1
#   define __ATTR_DLLIMPORT      /* Nothing */
#   define __NO_ATTR_DLLEXPORT   1
#   define __ATTR_DLLEXPORT      /* Nothing */
#endif
#if __GCC_VERSION(3,3,0)
#   define __NONNULL(ppars)      __attribute__((__nonnull__ ppars))
#else
#   define __NO_NONNULL          1
#   define __NONNULL(ppars)      /* Nothing */
#endif
#if __GCC_VERSION(3,3,0) /* __GCC_VERSION(3,4,0) */
#   define __WUNUSED             __attribute__((__warn_unused_result__))
#else
#   define __NO_WUNUSED          1
#   define __WUNUSED             /* Nothing */
#endif

#define __ATTR_WARNING(text)     __attribute__((__warning__(text)))
#define __ATTR_ERROR(text)       __attribute__((__error__(text)))
#define __ATTR_SECTION(name)     __attribute__((__section__(name)))
#define __ATTR_RETNONNULL        __attribute__((__returns_nonnull__))
#define __ATTR_PACKED            __attribute__((__packed__))
#define __ATTR_ALIAS(name)       __attribute__((__alias__(name)))
#define __ATTR_ALIGNED(n)        __attribute__((__aligned__(n)))
#define __ATTR_WEAK              __attribute__((__weak__))
#define __ATTR_RETURNS_TWICE     __attribute__((__returns_twice__))
#define __ATTR_EXTERNALLY_VISIBLE __attribute__((__externally_visible__))
#define __ATTR_VISIBILITY(vis)   __attribute__((__visibility__(vis)))

#ifdef __INTELLISENSE__
#   define __XBLOCK(...)      (([&]__VA_ARGS__)())
#   define __XRETURN             return
#   define __builtin_assume(x)   __assume(x)
#else
#if __GCC_VERSION(4,4,0) || defined(__TPP_VERSION__)
#   define __PRIVATE_PRAGMA(...) _Pragma(#__VA_ARGS__)
#   define __pragma(...) __PRIVATE_PRAGMA(__VA_ARGS__)
#else
#   define __NO_pragma   1
#   define __pragma(...) /* Nothing */
#endif
#   define __XBLOCK              __extension__
#   define __XRETURN             /* Nothing */
#if !__has_builtin(__builtin_unreachable)
#   define __NO_builtin_assume   1
#   define __builtin_assume(x)  (void)0
#endif
#endif
#if __GCC_VERSION(4,3,0) && (!defined(__GCCXML__) && \
   !defined(__clang__) && !defined(unix) && \
   !defined(__unix__)) || defined(__LP64__)
#   define __COMPILER_ALIGNOF    __alignof__
#elif defined(__clang__)
#   define __COMPILER_ALIGNOF    __alignof
#elif defined(__cplusplus)
extern "C++" { template<class T> struct __compiler_alignof { char __x; T __y; }; }
#   define __COMPILER_ALIGNOF(T) (sizeof(__compiler_alignof< T >)-sizeof(T))
#else
#   define __COMPILER_ALIGNOF(T) ((__SIZE_TYPE__)&((struct{ char __x; T __y; } *)0)->__y)
#endif
#if defined(__NO_INLINE__) && 0
#   define __NO_ATTR_INLINE 1
#   define __ATTR_INLINE    /* Nothing */
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ > 199901L
#   define __ATTR_INLINE    inline
#elif __GCC_VERSION(2,7,0)
#   define __ATTR_INLINE    __inline__
#else
#   define __NO_ATTR_INLINE 1
#   define __ATTR_INLINE    /* Nothing */
#endif
#if __GCC_VERSION(3,0,0)
#   define __ATTR_FORCEINLINE __inline__ __attribute__((__always_inline__))
#elif __GCC_VERSION(2,7,0)
#   define __NO_ATTR_FORCEINLINE 1
#   define __ATTR_FORCEINLINE __inline__
#else
#   define __NO_ATTR_FORCEINLINE 1
#   define __ATTR_FORCEINLINE /* Nothing */
#endif
#define __LOCAL       static __ATTR_INLINE
#define __FORCELOCAL  static __ATTR_FORCEINLINE
#ifndef __LONGLONG
#ifdef __CC__
__extension__ typedef long long __longlong_t;
__extension__ typedef unsigned long long __ulonglong_t;
#define __LONGLONG   __longlong_t
#define __ULONGLONG  __ulonglong_t
#endif
#endif /* !__LONGLONG */

#if !__GCC_VERSION(2,92,0) /* !__GCC_VERSION(2,95,0) */
#ifndef __restrict
#if defined(restrict) || \
   (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 199901L)
#   define __restrict restrict
#else
#   define __restrict /* Nothing */
#endif
#endif /* !__restrict */
#ifndef __restrict__
#define __restrict__   __restrict
#endif /* !__restrict__ */
#endif

#if __GCC_VERSION(3,1,0) && !defined(__GNUG__)
#   define __restrict_arr __restrict
#else
#   define __restrict_arr /* Nothing */
#endif
#if 1
#   define __empty_arr(T,x) __extension__ T x[0]
#else
#   define __empty_arr(T,x) T x[1]
#endif

#define __STATIC_IF(x)   if(x)
#define __STATIC_ELSE(x) if(!(x))
#ifdef __cplusplus
#define __IF0     if(false)
#define __IF1     if(true)
#define __WHILE0  while(false)
#define __WHILE1  while(true)
#else
#define __IF0     if(0)
#define __IF1     if(1)
#define __WHILE0  while(0)
#define __WHILE1  while(1)
#endif

#ifdef __cplusplus
#if !defined(__INTEL_VERSION__) || __INTEL_VERSION__ >= 600 || \
    (_WCHAR_T_DEFINED+0 != 0) || (_WCHAR_T+0 != 0)
#define __native_wchar_t_defined 1
#define __wchar_t_defined 1
#endif
#endif

#ifndef __INTELLISENSE__
#define __FUNCTION__   __extension__ __FUNCTION__
#endif

#if __GCC_VERSION(4,7,0)
#   define __COMPILER_BARRIER()       __atomic_signal_fence(__ATOMIC_ACQ_REL)
#   define __COMPILER_READ_BARRIER()  __atomic_signal_fence(__ATOMIC_ACQUIRE)
#   define __COMPILER_WRITE_BARRIER() __atomic_signal_fence(__ATOMIC_RELEASE)
#elif defined(__COMPILER_HAVE_GCC_ASM)
#   define __COMPILER_BARRIERS_ALL_IDENTICAL 1
#   define __COMPILER_BARRIER()       __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#   define __COMPILER_READ_BARRIER()  __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#   define __COMPILER_WRITE_BARRIER() __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#else
#   define __COMPILER_BARRIERS_ALL_IDENTICAL 1
#   define __COMPILER_BARRIER()       __sync_synchronize()
#   define __COMPILER_READ_BARRIER()  __sync_synchronize()
#   define __COMPILER_WRITE_BARRIER() __sync_synchronize()
#endif

#if 1
#define __COMPILER_IGNORE_UNINITIALIZED(var) var=var
#endif

#ifdef __cplusplus
#ifdef __INTELLISENSE__
#   define __NULLPTR    nullptr
#else
#   define __NULLPTR          0
#endif
#else
#   define __NULLPTR ((void *)0)
#endif

