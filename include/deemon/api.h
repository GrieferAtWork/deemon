/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_API_H
#define GUARD_DEEMON_API_H 1
#ifndef __KOS__
#define __NO_KOS_SYSTEM_HEADERS__ 1
#endif /* !__KOS__ */

/* Try to expose various UNIX features in headers.
 * These are always enabled, so that `deemon/system-features.h'
 * is always correct, so-long as this header is #included before
 * any system header is #included. */
#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE 1
#endif /* !_ATFILE_SOURCE */
#ifndef _KOS_SOURCE
#define _KOS_SOURCE 1
#endif /* !_KOS_SOURCE */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /* !_GNU_SOURCE */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif /* !_XOPEN_SOURCE */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* !_POSIX_C_SOURCE */
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE 1
#endif /* !_LARGEFILE_SOURCE */
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif /* !_LARGEFILE64_SOURCE */
#ifndef _CTYPE_MACRO_SOURCE
#define _CTYPE_MACRO_SOURCE 1
#endif /* !_CTYPE_MACRO_SOURCE */
#ifndef _TIME64_SOURCE
#define _TIME64_SOURCE 1
#endif /* !_TIME64_SOURCE */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif /* !_BSD_SOURCE */
#ifndef _SVID_SOURCE
#define _SVID_SOURCE 1
#endif /* !_SVID_SOURCE */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif /* !_DEFAULT_SOURCE */
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif /* !_XOPEN_SOURCE_EXTENDED */
#ifndef _DOS_SOURCE
#define _DOS_SOURCE 1
#endif /* !_DOS_SOURCE */
/* Ask KOS headers to provide the empty-needle-is-NULL variant of `memmem()' */
#ifndef _MEMMEM_EMPTY_NEEDLE_NULL_SOURCE
#define _MEMMEM_EMPTY_NEEDLE_NULL_SOURCE 1
#endif /* !_MEMMEM_EMPTY_NEEDLE_NULL_SOURCE */



/* Expose definitions that don't comply with the deemon C symbol namespace.
 * That namespace being anything matching `dee_*', `DEE_*', `Dee*' or `_Dee*'. */
#if !defined(DEE_SOURCE) && defined(CONFIG_BUILDING_DEEMON)
#define DEE_SOURCE 1
#endif /* !DEE_SOURCE && CONFIG_BUILDING_DEEMON */

/* Disable garbage */
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_SECURE_NO_WARNINGS  1
#define _CRT_NONSTDC_NO_WARNINGS 1

#include <__stdinc.h> /* __CC__ */

#ifdef __CC__
#include <stddef.h>
#endif /* __CC__ */

#ifndef __has_include
#define __NO_has_include 1
#define __has_include(x) 0
#endif /* !__has_include */

#ifdef CONFIG_NO_CRTDBG_H
#undef CONFIG_HAVE_CRTDBG_H
#elif (!defined(CONFIG_HAVE_CRTDBG_H) && \
       (__has_include(<crtdbg.h>) ||     \
        (defined(__NO_has_include) &&    \
         (defined(_MSC_VER) || defined(__KOS_SYSTEM_HEADERS__)))))
#define CONFIG_HAVE_CRTDBG_H 1
#endif /* ... */

#if defined(CONFIG_HAVE_CRTDBG_H) && !defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC 1 /* Enable debug-malloc */
#endif /* CONFIG_HAVE_CRTDBG_H && !NDEBUG */

#define DEE_VERSION_API      200
#define DEE_VERSION_COMPILER 200
#define DEE_VERSION_REVISION 0

#include <hybrid/compiler.h>

#include <hybrid/debug-alignment.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

#ifdef __CC__
#include <stdarg.h>

#if (defined(_CRTDBG_MAP_ALLOC) && \
     defined(CONFIG_HAVE_CRTDBG_H) && !defined(NDEBUG))
#include <crtdbg.h>
#endif /* _CRTDBG_MAP_ALLOC && CONFIG_HAVE_CRTDBG_H && !NDEBUG */
#endif /* __CC__ */


/* Disable some problematic compiler warnings when DEE_SOURCE is defined.
 * The later is the case for the deemon core, and (usually) dex-modules. */
#ifdef DEE_SOURCE

#ifdef _MSC_VER
#pragma warning(disable: 4054) /* Cast from function pointer to `void *' */
#pragma warning(disable: 4152) /* Literally the same thing as `4054', but emit for static initializers. */
#endif /* _MSC_VER */

#ifdef __GNUC__
#if __GNUC__ >= 8
/* Disable warnings about casting incompatible function pointers (for now)
 * While I really welcome these warnings, they pose one big problem with the
 * way in which I've introduced support for keyword-enabled functions, and
 * with how casts to `dfunptr_t' work. */
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif /* __GNUC__ >= 8 */
#if __GNUC__ >= 4
/* When declaring DeeTypeObject objects and the like, we often skip
 * the initializers for various fields that have no reason of being
 * explicitly initialized by the static initializer. This mainly affects
 * `tp_cache' and `tp_class_cache'. However, the default (zero-/NULL-)
 * initializer already does what we need it to do, meaning that
 * initializing it explicitly would just add unnecessary code bloat!
 *
 * As such, disable warnings about static struct initializers that
 * initialize some fields, but don't initialize _all_ fields. */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif /* __GNUC__ >= 4 */
#endif /* __GNUC__ */
#endif /* DEE_SOURCE */


/* Evaluate `expr' at runtime, and instruct compile-time optimizations
 * under the assumption that it always evaluates to `value'. Used to
 * wrap functions that always return the same value.
 *
 * For example: if the compiler knows that a function's return values
 * is _always_ `-1', it can (rightfully so) assume the contents of the
 * return value register, which can then lead to further optimizations
 * where it won't need to re-load the return value for code like:
 * >> int foo() {
 * >>      if (a) {
 * >>          DeeError_Throw(...);
 * >>          goto err;
 * >>      }
 * >>      if (b) {
 * >>          DeeError_Throw(...);
 * >>          goto err;
 * >>      }
 * >>      return 0;
 * >>  err:
 * >>      return -1;
 * >> }
 *
 * If the compiler knows that `goto err' is only ever called with the
 * return value register already containing `-1', then it won't have
 * to load that value back into the register yet again! */
#ifndef Dee_ASSUMED_VALUE
#ifndef __NO_builtin_unreachable
#define Dee_ASSUMED_VALUE(expr, value) ((expr) == (value) ? (value) : (__builtin_unreachable(), value))
#else /* __NO_builtin_unreachable */
#define Dee_ASSUMED_VALUE_IS_NOOP 1
#define Dee_ASSUMED_VALUE(expr, value) (expr)
#endif /* !__NO_builtin_unreachable */
#endif /* !Dee_ASSUMED_VALUE */


DECL_BEGIN

/* #define CONFIG_NO_THREADS 1 */

/* CONFIG:  Enable tracing of all incref()s and decref()s that
 *          happen to an object over the course of its lifetime.
 *          When deemon shuts down, dump that reference count
 *          history for all dynamically allocated objects that
 *          still haven't been destroyed.
 *       -> Since this includes every incref and decref operation
 *          ever performed on the object, as well as the reference
 *          counter values at that point in time, it becomes fairly
 *          easy to spot the point when the reference counter become
 *          unsynchronized due to the lack of a required decref.
 * WARNING: Since every object in existence is tracked when this is
 *          enabled, combined with the open-ended-ness of the journal
 *          being kept, this option can become quite expensive to leave
 *          enabled, and should only be enabled to help tracking down
 *          reference leaks.
 * WARNING: Don't leave this option enabled when you don't need it!
 *          Using this option disables various fast-pass code options,
 *          as well as induce a _huge_ overhead caused by practically
 *          any kind of operation with objects, as well as significant
 *          hang-times when used with larger code bases (especially
 *          ones with a lot of code-reuse and cross-dependencies)
 *       -> This is only meant for testing small example applications
 *          that have proven to cause reference leaks, in order to
 *          analyze what exactly is causing them. */
#if (!defined(CONFIG_TRACE_REFCHANGES) && \
     !defined(CONFIG_NO_TRACE_REFCHANGES))
#if !defined(NDEBUG) && 0
#define CONFIG_TRACE_REFCHANGES 1
#else /* !NDEBUG */
#define CONFIG_NO_TRACE_REFCHANGES 1 
#endif /* NDEBUG */
#endif /* !CONFIG_TRACE_REFCHANGES && !CONFIG_NO_TRACE_REFCHANGES */

#if (!defined(CONFIG_NO_BADREFCNT_CHECKS) && \
     !defined(CONFIG_BADREFCNT_CHECKS))
#ifdef NDEBUG
#define CONFIG_NO_BADREFCNT_CHECKS 1
#else /* NDEBUG */
#define CONFIG_BADREFCNT_CHECKS 1
#endif /* !NDEBUG */
#endif /* !CONFIG_NO_BADREFCNT_CHECKS && !CONFIG_BADREFCNT_CHECKS */


#ifdef CONFIG_TRACE_REFCHANGES
/* Assembly interpreters do not implement the additional
 * overhead required to properly track reference counts.
 * -> So just disable them! */
#undef CONFIG_HAVE_EXEC_ASM
#else /* CONFIG_TRACE_REFCHANGES */
#define CONFIG_NO_TRACE_REFCHANGES 1
#endif /* !CONFIG_TRACE_REFCHANGES */


#if (!defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && \
     !defined(CONFIG_NO_CALLTUPLE_OPTIMIZATIONS))
#ifndef __OPTIMIZE_SIZE__
#define CONFIG_CALLTUPLE_OPTIMIZATIONS 1
#else /* !__OPTIMIZE_SIZE__ */
#define CONFIG_NO_CALLTUPLE_OPTIMIZATIONS 1
#endif /* __OPTIMIZE_SIZE__ */
#endif /* !CONFIG_[NO_]CALLTUPLE_OPTIMIZATIONS */

#if (!defined(CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS) && \
     !defined(CONFIG_NO_NOBASE_OPTIMIZED_CLASS_OPERATORS))
#ifndef __OPTIMIZE_SIZE__
#define CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS 1
#else /* !__OPTIMIZE_SIZE__ */
#define CONFIG_NO_NOBASE_OPTIMIZED_CLASS_OPERATORS 1
#endif /* __OPTIMIZE_SIZE__ */
#endif /* !CONFIG_[NO_]NOBASE_OPTIMIZED_CLASS_OPERATORS */


/* Configure option:
 *     CONFIG_DEFAULT_MESSAGE_FORMAT_(MSVC|GCC)
 * Select the default format for file+line encoding in messages:
 *     MSVC:  file(line) : Message    file(line, column) : Message
 *     GCC:   file:line: Message      file:line:column: Message
 * When not pre-defined via ./configure, default to using the same
 * convention as the hosting compiler (i.e.: MSVC for _MSC_VER, and
 * GCC for everything else) */
#ifdef CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
#if (CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC + 0) == 0
#undef CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
#ifdef CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#if (CONFIG_DEFAULT_MESSAGE_FORMAT_GCC + 0) == 0
#undef CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#ifdef _MSC_VER
#define CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC 1
#else /* _MSC_VER */
#define CONFIG_DEFAULT_MESSAGE_FORMAT_GCC 1
#endif /* !_MSC_VER */
#endif /* !CONFIG_DEFAULT_MESSAGE_FORMAT_GCC */
#else /* CONFIG_DEFAULT_MESSAGE_FORMAT_GCC */
#define CONFIG_DEFAULT_MESSAGE_FORMAT_GCC 1
#endif /* !CONFIG_DEFAULT_MESSAGE_FORMAT_GCC */
#else /* !CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC */
#undef CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#endif /* CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC */
#elif defined(CONFIG_DEFAULT_MESSAGE_FORMAT_GCC)
#if (CONFIG_DEFAULT_MESSAGE_FORMAT_GCC + 0) == 0
#undef CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#define CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC 1
#endif
#elif defined(_MSC_VER)
#define CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC 1
#else /* ... */
#define CONFIG_DEFAULT_MESSAGE_FORMAT_GCC 1
#endif /* !... */



#if (defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__WINDOWS__) ||            \
     defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) ||          \
     defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
     defined(_WIN32_WCE) || defined(WIN32_WCE))
#define CONFIG_HOST_WINDOWS 1
#endif /* Windows... */
#if (defined(__unix__) || defined(__unix) || defined(unix) ||           \
     defined(__linux__) || defined(__linux) || defined(linux) ||        \
     defined(__KOS__) || defined(__NetBSD__) || defined(__FreeBSD__) || \
     defined(__solaris__) || defined(__DragonFly__))
#define CONFIG_HOST_UNIX 1
#endif /* Unix... */


#if ((!defined(__i386__) && !defined(__x86_64__)) || \
     (!defined(CONFIG_HOST_WINDOWS) && !defined(CONFIG_HOST_UNIX)))
#undef CONFIG_NO_OBJECT_SLABS
#define CONFIG_NO_OBJECT_SLABS 1 /* Unrecognized environment (disable slabs) */
#endif /* ... */


#ifdef CONFIG_HOST_WINDOWS
#ifndef _WIN32_WINNT
/* Limit windows headers to only provide XP stuff. */
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#endif /* !_WIN32_WINNT */
#endif /* CONFIG_HOST_WINDOWS */


#ifdef CONFIG_BUILDING_DEEMON
#if ((defined(__i386__) && !defined(__x86_64__)) && \
     defined(CONFIG_HOST_WINDOWS))
#if 0
#define ASSEMBLY_NAME(x, s) PP_CAT4(__USER_LABEL_PREFIX__, x, @, s)
#else
#define ASSEMBLY_NAME(x, s) PP_CAT2(__USER_LABEL_PREFIX__, x@s)
#endif
#else
#define ASSEMBLY_NAME(x, s) PP_CAT2(__USER_LABEL_PREFIX__, x)
#endif
#endif /* CONFIG_BUILDING_DEEMON */

#if defined(__i386__) && !defined(__x86_64__)
/* The `va_list' structure is simply a pointer into the argument list,
 * where arguments can be indexed by alignment of at least sizeof(void *).
 * This allows one to do the following:
 * >> void DCALL function_a(size_t argc, void **argv) {
 * >>      size_t i;
 * >>      for (i = 0; i < argc; ++i)
 * >>          printf("argv[%lu] = %p\n", (unsigned long)i, argv[i]);
 * >> }
 * >> #ifndef __NO_DEFINE_ALIAS
 * >> DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(function_b, 8),
 * >>                     ASSEMBLY_NAME(function_a, 8));
 * >> #else // !__NO_DEFINE_ALIAS
 * >> void DCALL function_b(size_t argc, va_list args) {
 * >>      function_a(argc, (void **)args);
 * >> }
 * >> #endif // __NO_DEFINE_ALIAS
 * >> void function_c(size_t argc, ...) {
 * >>      va_list args;
 * >>      va_start(args, argc);
 * >>      function_b(argc, args);
 * >>      va_end(args, argc);
 * >> }
 * Using this internally, we can greatly optimize calls to functions
 * like `DeeObject_CallPack()' by not needing to pack everything together
 * into a temporary vector that would have to be allocated on the heap.
 */
#define CONFIG_VA_LIST_IS_STACK_POINTER 1
#endif



#ifdef __CC__

#if defined(__INTELLISENSE__) && defined(__cplusplus)
/* Highlight invalid usage of `NULL' in functions returning `int' */
#undef NULL
#define NULL nullptr
#endif /* __INTELLISENSE__ && __cplusplus */

#ifndef Dee_BREAKPOINT
#ifdef _MSC_VER
extern void (__debugbreak)(void);
#pragma intrinsic(__debugbreak)
#define Dee_BREAKPOINT() __debugbreak()
#elif defined(__COMPILER_HAVE_GCC_ASM) && (defined(__i386__) || defined(__x86_64__))
#ifdef __NO_XBLOCK
#define Dee_BREAKPOINT() __asm__ __volatile__("int {$}3" : )
#else /* __NO_XBLOCK */
#define Dee_BREAKPOINT() XBLOCK({ __asm__ __volatile__("int {$}3" : ); (void)0; })
#endif /* !__NO_XBLOCK */
#else /* ... */
#define Dee_BREAKPOINT() (void)0
#endif /* !... */
#endif /* !Dee_BREAKPOINT */

#ifdef CONFIG_BUILDING_DEEMON
#   define DFUNDEF __EXPDEF
#   define DDATDEF __EXPDEF
#else /* CONFIG_BUILDING_DEEMON */
#   define DFUNDEF __IMPDEF
#   define DDATDEF __IMPDEF
#endif /* !CONFIG_BUILDING_DEEMON */

#ifdef __GNUC__
/* Define if the compiler allows labels to be
 * addressed: `foo: printf("foo = %p", &&foo);'
 * ... as well as such addresses to be used
 * by `goto': `void *ip = &&foo; goto *ip;' */
#define CONFIG_COMPILER_HAVE_ADDRESSIBLE_LABELS 1
#endif /* __GNUC__ */

/* Calling convention used for the deemon API */
#ifndef DCALL
#if defined(__i386__) && !defined(__x86_64__)
#define DCALL __ATTR_STDCALL
#else /* __i386__ && !__x86_64__ */
#define DCALL /* nothing (use default calling convention) */
#endif /* !__i386__ || __x86_64__ */
#endif /* !DCALL */

/* Calling convention for `dformatprinter' */
#ifndef DPRINTER_CC
#if defined(__KOS__) && __KOS_VERSION__ >= 400
/* We want to be ABI-compatible with KOS's native `pformatprinter' system. */
#if defined(__KOS_SYSTEM_HEADERS__) || __has_include(<bits/crt/format-printer.h>)
#include <bits/crt/format-printer.h>
#define DPRINTER_CC __FORMATPRINTER_CC
#elif __has_include(<format-printer.h>)
#include <format-printer.h>
#define DPRINTER_CC FORMATPRINTER_CC
#endif /* ... */
#endif /* __KOS__ && __KOS_VERSION__ >= 400 */
#ifndef DPRINTER_CC
#define DPRINTER_CC DCALL
#endif /* !DPRINTER_CC */
#endif /* !DPRINTER_CC */

#ifndef DREF
#define DREF  /* Annotation for pointer: transfer/storage of a reference.
               * NOTE: When returned by a function, a return value
               *       of NULL indicates a newly raised exception. */
#endif /* !DREF */
#ifndef DWEAK
#define DWEAK /* Annotation for data that is thread-volatile. */
#endif /* !DWEAK */


#ifdef DEE_SOURCE
#ifndef FCALL
#if defined(__i386__) && !defined(__x86_64__)
#define FCALL __ATTR_FASTCALL /* arg0: %ecx, arg1: %edx, argN: 4+N*4(%esp) (callee-cleanup) */
#else /* __i386__ && !__x86_64__ */
#define FCALL /* nothing (use default calling convention) */
#endif /* !__i386__ || __x86_64__ */
#endif /* !FCALL */
#endif /* DEE_SOURCE */


#ifdef _MSC_VER
#pragma warning(disable: 4201)
#pragma warning(disable: 4510)
#pragma warning(disable: 4512)
#pragma warning(disable: 4565)
#pragma warning(disable: 4610)
#endif /* _MSC_VER */

#if !defined(NDEBUG) && !defined(CONFIG_NO_CHECKMEMORY) && defined(_DEBUG)
#ifdef CONFIG_HOST_WINDOWS
#ifdef _MSC_VER
#define Dee_CHECKMEMORY() (DBG_ALIGNMENT_DISABLE(), (_CrtCheckMemory)(), DBG_ALIGNMENT_ENABLE())
#if !defined(_MSC_VER) || defined(_DLL)
extern __ATTR_DLLIMPORT int (ATTR_CDECL _CrtCheckMemory)(void);
#else /* !_MSC_VER || _DLL */
extern int (ATTR_CDECL _CrtCheckMemory)(void);
#endif /* _MSC_VER && !_DLL */
#endif /* _MSC_VER */
#endif /* CONFIG_HOST_WINDOWS */
#endif /* !NDEBUG */

#ifdef __INTELLISENSE__
extern "C++" template<class T> T ____INTELLISENSE_req_type(T x);
#define Dee_REQUIRES_TYPE(T, x)  ____INTELLISENSE_req_type<T>(x)
#else /* __INTELLISENSE__ */
#define Dee_REQUIRES_TYPE(T, x) (x)
#endif /* !__INTELLISENSE__ */

#ifndef NDEBUG
#define Dee_DPRINT(message)        (_Dee_dprint_enabled ? _Dee_dprint(message) : (void)0)
#define Dee_DPRINTER               _Dee_dprinter
#define Dee_DPRINTF(...)           (_Dee_dprint_enabled ? _Dee_dprintf(__VA_ARGS__) : (void)0)
#define Dee_VDPRINTF(format, args) _Dee_vdprintf(format, args) /* Always invoke because `format' may mandate a decref() operation! */
DDATDEF int _Dee_dprint_enabled;
DFUNDEF void (DCALL _Dee_dprint)(char const *__restrict message);
DFUNDEF void (_Dee_dprintf)(char const *__restrict format, ...);
DFUNDEF void (DCALL _Dee_vdprintf)(char const *__restrict format, va_list args);
DFUNDEF __SSIZE_TYPE__ (DPRINTER_CC _Dee_dprinter)(void *arg, char const *__restrict data, size_t datalen);
#endif /* !NDEBUG */


#ifndef Dee_ASSERT
#ifndef NDEBUG
DFUNDEF void (DCALL _DeeAssert_Fail)(char const *expr, char const *file, int line);
DFUNDEF void (_DeeAssert_Failf)(char const *expr, char const *file, int line, char const *format, ...);
#define Dee_ASSERT(expr)       (void)((expr) || (_DeeAssert_Fail(#expr, __FILE__, __LINE__), Dee_BREAKPOINT(), 0))
#define Dee_ASSERTF(expr, ...) (void)((expr) || (_DeeAssert_Failf(#expr, __FILE__, __LINE__, __VA_ARGS__), Dee_BREAKPOINT(), 0))
#elif !defined(__NO_builtin_assume)
#define Dee_ASSERT(expr)       __builtin_assume(expr)
#define Dee_ASSERTF(expr, ...) __builtin_assume(expr)
#else /* ... */
#define Dee_ASSERT(expr)       (void)0
#define Dee_ASSERTF(expr, ...) (void)0
#endif /* !... */
#endif /* !Dee_ASSERT */
#ifndef Dee_ASSERTF
#define Dee_ASSERTF(expr, ...) Dee_ASSERT(expr)
#endif /* !Dee_ASSERTF */

#ifdef DEE_SOURCE
#undef ASSERT
#undef ASSERTF
#define ASSERT  Dee_ASSERT
#define ASSERTF Dee_ASSERTF
#endif /* DEE_SOURCE */


#ifndef Dee_DPRINT
#define DEE_NO_DPRINTF             1
#define Dee_DPRINT(message)        (void)0
#define Dee_DPRINTF(...)           (void)0
#define Dee_VDPRINTF(format, args) (void)0
#endif /* !Dee_DPRINT */

#ifndef Dee_CHECKMEMORY
#define DEE_NO_CHECKMEMORY 1
#define Dee_CHECKMEMORY()  (void)0
#endif /* !Dee_CHECKMEMORY */

#endif /* __CC__ */



/* Doc string formatting:
 * TODO: REMOVE ME! REPLACED BY THE FORMAT FOUND IN /include/deemon/compiler/doctext.h
 *
 *
 *    - \   Used as escape character to prevent the next character from being
 *          recognized as a special documentation token.
 *          This character is deleted from the generated text, but is not
 *          recognized as an escape character within an specific doc option.
 *          e.g.: "Therefor\\:this is an example"
 *          The following characters must be escaped:
 *              \ : { } @ # $ % & ^ ?
 *
 *    - :ident
 *    - :{ident}
 *        - `ident' may only contain `UNICODE_FSYMSTRT->UNICODE_FSYMCONT'
 *          characters, as well as `:' and `.' characters.
 *          If this could lead to ambiguity (e.g.: a reference at the end of a
 *          sentence, meaning that the next text-character is a `.'), `ident'
 *          may be wrapped by {...}, as shown above.
 *        - Reference to `ident' (a symbol name), searched for in the following
 *          places, in order. If the `:' character is not followed by a `UNICODE_FSYMSTRT'
 *          character, no reference should be detected and the `:' should be included
 *          in the documentation text.
 *          e.g.: "Because it inherits from :Sequence a lot of predefined functions are available"
 *        - If `ident' contains another `:' character, it separates the identifier into 2 parts:
 *             `:{module:item}'  --> `module' and `item'
 *          The left part is interpreted as the module name as also accepted by the `import()' expression.
 *          It may however also be a relative module name (starting with a `.') which then refers
 *          to another module that is relative to the one containing the doc string.
 *        - If no `:' is used to explicitly state a module, the object referenced is search for as follows:
 *            #1: Search the module of the type referring to the documentation string.
 *            #2: Search all modules imported by that module, but don't do so recursively.
 *            #3: Search the builtin `deemon' module if it wasn't already searched.
 *            #4: Recursively search all builtin errors reachable from `Error from deemon' and `Signal from deemon'
 *            #5: Try to open a module with the specified name.
 *        - The `item' part, or `ident' itself when no `module' was specified may contain
 *          additional `.' characters to specify a specific attribute which should be
 *          referenced instead.
 *          e.g.: "This implementation differs from :deemon:Sequence.find in that it accepts @start and @end arguments."
 *        - When an attribute is known for both the type and instance, the type's is preferred.
 *          e.g.: "This refers to the type and not the method: :Dict.keys"
 *
 *    - @ident
 *    - @{ident}
 *        - `ident' may only contain `UNICODE_FSYMSTRT->UNICODE_FSYMCONT' characters.
 *        - Used to refer to an argument while inside the documentation text of a function.
 *          `ident' should be the name of the argument in this case.
 *          Additionally, `ident' may be `this' to refer to the self-argument in
 *          a member function that is implemented as a this-call.
 *
 *    - ?#ident
 *    - ?#{ident}
 *        - `ident' may only contain `UNICODE_FSYMSTRT->UNICODE_FSYMCONT' characters.
 *        - Used to refer to another attribute `ident' within the
 *          same type (member-doc) / module (module-symbol-doc)
 *
 *    - $code
 *    - ${code}
 *    - [\n[optional_whitespace]$code\n]...
 *    - [\n[optional_whitespace]>code\n]...
 *        - The last form will automatically remove whitespace before `$' and
 *          any amount of whitespace following that is shared by all lines before
 *          appending everything together.
 *          Note that the form of `${code}' counts the number of `{' and `}'
 *          characters, allowing code printing of code line `${ { local x = foo; } }'
 *          Also: Any leading/trailing whitespace of code is automatically removed.
 *          e.g.: "(string other) -> int\n"
 *                " Find @other within @this\n"
 *                " $print \"foo\".find("o"); // 1\n"
 *                " $print \"foo\".find("r"); // -1\n"
 *          WARNING: Code blocks are still subject to processing rules of any other
 *                   text, meaning that they may be terminated prematurely by
 *                   incorrect documentation strings.
 *          Code portions of documentation strings are still subject to documentation
 *          interpretation, meaning that if the documentation parser expects a line-feed,
 *          the code block may be terminated prematurely.
 *
 *    - %ident
 *    - %{ident}
 *        Documentation format command:
 *        - %{table ['|' ~~ COLUMN_NAMES...] \n
 *            \n ~~ ['|' ~~ COLUMN_TEXT...]...}
 *          Both `COLUMN_NAMES' and `COLUMN_TEXT' are processed as regular text.
 *          The only special thing is that `|' is used to switch to the next column,
 *          and line-feeds (if not escaped) are used to go to the next row.
 *          Whitespace surrounding '|' is removed
 *        - %{html text...}
 *          Emit the whitespace-stripped text from `text...' as HTML
 *        - %{href link text...}
 *          Emit a hyperlink to `link', with `text' being the link's text.
 *
 *
 *    - \n[optional_whitespace]@ident[:][...]\n
 *        - An optional `:' character may be part of `ident' must is simply discarded.
 *        - Special parsing of tags found at the start of a line.
 *          Note that if any text found in following lines with
 *          a leading indentation that reaches at least until the end of the
 *          tag name is considered part of the line and re-formatted as follows:
 *          >> "@return: -1: The item could not be found\n"
 *             "        The reason for this is that it doesn't exist\n"
 *             "       This line is no longer part of the return-doc text\n"
 *          Same as:
 *          >> "@return: -1: The item could not be found. The reason for this is that it doesn't exist\n"
 *             "      This line is no longer part of the return-doc text\n"
 *          NOTE: If the first non-whitespace character of the following line
 *                is written in uppercase, a missing '.' is appended to the
 *                previous line before the two are added together.
 *                Additionally, any trailing whitespace is removed from the first
 *                line, while any leading whitespace is removed from the second,
 *                before a single space-character is inserted in-between.
 *          NOTE: Tab characters count as 4 space characters for this calculation.
 *        - Note that this kind of recognition supersedes references
 *          to arguments in function documentation strings.
 *        - Based on `ident', the leading portion of the text
 *          associated with the tag may be parsed differently.
 *          For this reason, the following tags are recognized specifically:
 *          - @throw [type :] text
 *          - @throws [type :] text
 *            Process `type' as an object type that may be thrown by the documented
 *            object, processing it as though it was written as `:{type}'
 *            Note that `type' may not contain any whitespace characters unless
 *            it is followed by another ':', in which case everything but the
 *            following `:' and optional whitespace immediate before are part of the type.
 *            When no ':' is found after at most 4 individual groups of whitespace characters
 *            have been encountered, the tag is assumed not to contain a type reference
 *            and the entirety of the text following the @throw[s][:] tag is interpreted
 *            as a human-readable string describing behavior when any kind of object is
 *            thrown by the function.
 *            NOTE: Because `:' may also appear in `type' naturally because it
 *                  may be referring to an object in another module, `:' should
 *                  be followed by whitespace, should it be used to name a
 *                  specific type that is thrown:
 *               OK:    >> @throws deemon:Error: An error
 *               OK:    >> @throws Error: An error
 *               Also OK (because only a single ':' is parsed as part of `type'):
 *                      >> @throws deemon:Error:An error
 *               WRONG: >> @throws Error:An error
 *          - @return [expr :] text
 *          - @returns [expr :] text
 *            Parsing of `expr' and `text' follows the same rules as the parsing
 *            or `type' and `text' in the @throw tag, except that the resulting
 *            `expr' is instead processed as though it was written as `${expr}'
 *            NOTE: When `expr' after being stripped of whitespace is equal to
 *                  a single '*' character, the tag behaves the same as though
 *                 `expr' wasn't given (aka. documenting a general-purpose return value)
 *          - @arg[:] ident [:] text
 *          - @param[:] ident [:] text
 *            Document a specific argument taken by a function.
 *            `ident' may not chain any whitespace and is
 *            interpreted as though it was written as `@{ident}'.
 *          - @interrupt
 *            Same as `@throw Signal.Interrupt: The calling :thread was interrupted'
 *
 *
 *
 * ----------------------  Documentation parsing:
 *
 * // How to read (I know that a doc needing a doc is bad, but it's really not that complicated...):
 * //  x       ::= y; - Define rule x as y
 * //  @x      ::= y; - Define a special rule x as y, where its meaning can be deduced by the human-readable name `x'
 * //  x{args} ::= y; - Define rule x as y, alongside a comma-separated list of names which are replaced within `y' each time `x' is invoked.
 * //  x              - Reference to another rule (imagine it being replaced with the other rule's content)
 * //  x{args}        - Replace with another rule `x{args}', using comma-separated specs from `args' to fill in occurences in `y' of operands named in the definition.
 * //  <x>            - x should be interpreted as human-readable text describing some special behavior.
 * //  'x'            - x is a character / character sequence
 * //  (x)            - Parenthesis to prevent ambiguity
 * //  [x]            - x is optional
 * //  a b            - Characters or tokens a and b are whitespace separated
 * //  a ## b         - a and b follow each other directly (not whitespace separated)
 * //  x ~~ y...      - Only starting at the second occurrence of y, y must always be preceded by x
 * //  x ~~ y##...    - Only starting at the second occurrence of y, y must always be preceded by x. Instances are not whitespace separated
 * //  x...           - x can be repeated an unlimited about of times (at least once)
 * //  x##...         - x can be repeated an unlimited about of times (at least once). Instances are not whitespace separated
 * //  a ## b...      - same as "a ## ('' ~~ b...)"
 * //  a... ## b...   - same as "('' ~~ a...) ## ('' ~~ b...)"
 * //  a... ## b      - same as "('' ~~ a...) ## b"
 * //  'a'...'b'      - Range of characters between a and b (including a and b) (same as a|a+1|a+2|a+3|...|b)
 * //  x | y          - Either x or y
 *
 * // NOTES:
 * //   - Any number of whitespace/linefeed characters must be appended to the
 * //     end of a documentation string if doing so should prevent a parser error.
 * //     Additionally, any number of superfluous whitespace/linefeed at the end
 * //     should be ignored.
 * //   - Leading whitespace-only or empty lines are removed
 * //     from, and should be ignored in doc strings.
 *
 *
 * get_function_name() -> <The name of the attribute/operator/type/module documented by this string>
 *
 * text                ::= <Any sequence of characters not matching the following token.
 *                          Additionally, `\' may be used to escape the following character
 *                          to prevent it being detected as matching the next token.
 *                          Furthermore, text is subjected to the formatting described above.
 *                          The interpreter may choose to append a '.' character to terminate
 *                          a sentence if it deems doing so appropriate>
 *                     ;
 *
 * type                ::= text; // A type string.
 *                               // Should be interpreted as text reformatted as `:{text}'. (aka. as a reference)
 *                               // NOTE: If this string starts with `:', `$' or `@', it is interpreted as-is (and not reformatted)
 *
 * rule_prototype      ::= [ident = get_function_name()]
 *                         ['(' [ ',' ~~ ([type ' ']... text ['=' text]) ')')...]
 *                         //              ^            ^         ^
 *                         //              |            |         +- Default argument value (Interpreted as `')
 *                         //              |            +----------- argument name (The last string before `,' or `)'
 *                         //              |                         following a list of other white-space separated strings)
 *                         //              +------------------------ argument type (When not given, default to `object')
 *                         ['->' type] // Return type. When omit, this may also be deduced from its context.
 *                                     //              e.g.: `operator str() -> string', `class my_class { this() -> my_class }', etc.
 *                                     //              Otherwise, it defaults to `-> object'
 *                                     // NOTE: When the non-reformatted `type' starts with `:', `$' or `@', an implicit
 *                                     //       documentation line is added to the associated text, so long that that text
 *                                     //       does not already contain a `@return' / `@returns' tag:
 *                                     //       "@return * : Always returns type" -- where `type' is replaced with `type' found after `->'
 *                                     // e.g.: "() -> ${42}" -- formatted to "() -> ${42}\n@return * : Always returns ${42}"
 *                         '\n'
 *                     ;
 *
 * rule_overload_group{prefix}
 *                     ::= ([prefix] rule_prototype)...
 *                         [(text '\n')...] // Documentation text for the prototypes listed before.
 *                                          // Lines are appended to each other and each line must
 *                                          // start with at least a single whitespace character.
 *                                          // The amount of space removed is `(for (local x: lines) #x-#x.lstrip()) < ...'
 *                                          // or in other words: the least amount of whitespace found at the start of any line.
 *                         '\n'             // Terminate with a line-feed, meaning that prototype-groups are separated by an empty line
 *                     ;
 *
 * // Documentation string for `DeeTypeObject::tp_doc'
 * @rule_type_doc      ::= [(text '\n')...]                           // Generic text describing this type
 *                         [(rule_overload_group { 'class' | 'this' } // Documentation for constructor overloads
 *                                                                    // NOTE: Also invoked when the name of the class is used as prefix.
 *                         | rule_overload_group { 'operator' }       // Documentation for operators implemented
 *                                                                    // NOTE: These overloads require `get_function_name()'
 *                                                                    //       to be used, which should be interpreted as
 *                                                                    //       either the `oi_uname' or `oi_sname' as obtainable
 *                                                                    //       using the `Dee_OperatorInfo()' API function.
 *                           )...]
 *                     ;
 *
 * // Documentation string for methods (e.g.: `struct type_method::m_doc')
 * @rule_method_doc    ::= rule_overload_group { 'function' };
 *
 * // Documentation string for getsets (e.g.: `struct type_getset::gs_doc')
 * @rule_getset_doc    ::= ['->' type '\n'] // Type of object referred to by the getset
 *                         [(text '\n')...] // Documentation text.
 *                                          // Any amount of leading whitespace shared by all lines is yet again removed.
 *                     ;
 *
 * // Documentation string for members (e.g.: `struct type_member::m_doc')
 * @rule_member_doc    ::= ['->' type '\n'] // The type that a human may reasonably expect this member to refer to
 *                                          // Note that the types of constant/structure members don't necessarily
 *                                          // need to specify this information, as their typing can already be
 *                                          // determined from other factors, unless their type is `STRUCT_OBJECT'.
 *                         [(text '\n')...] // Documentation text.
 *                                          // Any amount of leading whitespace shared by all lines is yet again removed.
 *                     ;
 *
 *
 * ---------------------------------------------------------------------------------
 */

/* NOTE: This config option only affects internal documentation strings. */
#ifndef CONFIG_NO_DOC
#define Dee_DOC(x)           x
#define Dee_DOC_DEF(name, x) INTERN_CONST char const name[] = x
#define Dee_DOC_REF(name)    INTDEF char const name[]
#define Dee_DOC_GET(name)    name
#else /* !CONFIG_NO_DOC */
#define Dee_DOC(x)           ((char *)NULL)
#define Dee_DOC_DEF(name, x) /* nothing */
#define Dee_DOC_REF(name)    /* nothing */
#define Dee_DOC_GET(name)    ((char *)NULL)
#endif /* CONFIG_NO_DOC */

#ifdef DEE_SOURCE
#define DOC     Dee_DOC
#define DOC_DEF Dee_DOC_DEF
#define DOC_REF Dee_DOC_REF
#define DOC_GET Dee_DOC_GET
#endif /* DEE_SOURCE */

DECL_END

#endif /* !GUARD_DEEMON_API_H */
