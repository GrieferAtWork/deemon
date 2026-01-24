/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
/*!fixincludes no_include_comments*/
/*!fixincludes ignore_unnecessary_include*/
/*!always include <__stdinc.h>*/
/*!always include <hybrid/compiler.h>*/
/*!export **/
#ifndef GUARD_DEEMON_API_H
#define GUARD_DEEMON_API_H 1

/* Since we use and include a small portable sub-set of KOS system headers, we
 * still need to tell those headers that the remainder of the KOS system header
 * suite isn't available unless we're being hosted by true KOS. */
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
#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif /* !__EXTENSIONS__ */



/* Expose definitions that don't comply with the deemon C symbol namespace.
 * That namespace being anything matching `dee_*', `DEE_*', `Dee*' or `_Dee*'. */
#if !defined(DEE_SOURCE) && defined(CONFIG_BUILDING_DEEMON)
#define DEE_SOURCE
#endif /* !DEE_SOURCE && CONFIG_BUILDING_DEEMON */

/* Disable MSVC garbage */
#define _CRT_SECURE_NO_DEPRECATE   1
#define _CRT_SECURE_NO_WARNINGS    1
#define _CRT_NONSTDC_NO_WARNINGS   1
#define _CRT_DECLARE_NONSTDC_NAMES 1

#ifndef __DEEMON__
#include <__stdinc.h> /* !always *//* __CC__ */

#if defined(__CC__) && !defined(__INTELLISENSE__)
#include <stddef.h> /* NULL, size_t */
#endif /* __CC__ && !__INTELLISENSE__ */
#endif /* !__DEEMON__ */

#ifndef __has_include
#define __NO_has_include 1
#define __has_include(x) 0
#endif /* !__has_include */

#if 1 /* TODO: Remove this block once "CONFIG_EXPERIMENTAL_CUSTOM_HEAP" becomes mandatory */
#ifdef CONFIG_NO_CRTDBG_H
#undef CONFIG_HAVE_CRTDBG_H
#elif (!defined(CONFIG_HAVE_CRTDBG_H) && \
       (__has_include(<crtdbg.h>) ||     \
        (defined(__NO_has_include) &&    \
         (defined(_MSC_VER) || defined(__KOS_SYSTEM_HEADERS__)))))
#define CONFIG_HAVE_CRTDBG_H
#endif /* ... */

#if defined(CONFIG_HAVE_CRTDBG_H) && !defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC 1 /* Enable debug-malloc */
#endif /* CONFIG_HAVE_CRTDBG_H && !NDEBUG */
#endif

#define DEE_VERSION_API      200
#define DEE_VERSION_COMPILER 200
#define DEE_VERSION_REVISION 0

#ifndef __DEEMON__
#include <hybrid/__assert.h> /* __hybrid_assert, __hybrid_assertf, __hybrid_assertion_failed, __hybrid_assertion_failedf */
#include <hybrid/compiler.h> /* TODO: Get rid of this include (<__stdinc.h> is fine, but this one
                              *       clobbers too much of the global namespace and can break compat
                              *       with 3rd party code that uses identifiers defined here) */
#include <hybrid/host.h>     /* _WIN16, _WIN32, _WIN64, __arm__, __i386__, __linux__, __pic__, __unix__, __x86_64__ */
#include <hybrid/typecore.h> /* __SIZE_TYPE__, __SSIZE_TYPE__ */

#if defined(__CC__) && !defined(__INTELLISENSE__)
#include <stdarg.h> /* va_list */

#if 1 /* TODO: Remove this block once "CONFIG_EXPERIMENTAL_CUSTOM_HEAP" becomes mandatory */
#if (defined(_CRTDBG_MAP_ALLOC) && \
     defined(CONFIG_HAVE_CRTDBG_H) && !defined(NDEBUG))
#include <crtdbg.h>
#endif /* _CRTDBG_MAP_ALLOC && CONFIG_HAVE_CRTDBG_H && !NDEBUG */
#endif
#endif /* __CC__ && !__INTELLISENSE__ */
#endif /* !__DEEMON__ */


/* Disable some problematic compiler warnings when DEE_SOURCE is defined.
 * The later is the case for the deemon core, and (usually) dex-modules. */
#ifdef DEE_SOURCE

/* Disable warnings about casting incompatible function pointers (for now)
 * While I really welcome these warnings, they pose one big problem with the
 * way in which I've introduced support for keyword-enabled functions, and
 * with how casts to `Dee_funptr_t' work. */
__pragma_GCC_diagnostic_ignored(Wcast_function_type)

/* When declaring DeeTypeObject objects and the like, we often skip
 * the initializers for various fields that have no reason for being
 * explicitly initialized by the static initializer. This mainly affects
 * `tp_cache' and `tp_class_cache'. However, the default (zero-/NULL-)
 * initializer already does what we need it to do, meaning that
 * initializing it explicitly would just add unnecessary code bloat!
 *
 * As such, disable warnings about static struct initializers that
 * initialize some fields, but don't initialize _all_ fields. */
__pragma_GCC_diagnostic_ignored(Wmissing_field_initializers)

/* This warning is utterly useless with how we're using
 * our heap when it comes to forcing downstream failures:
 * >> include/deemon/alloc.h:250:38: warning: argument 1 value ‘18446744073709551615’ exceeds maximum object size 9223372036854775807 [-Walloc-size-larger-than=]
 * >> [...]
 * >> src/deemon/objects/rodict.c:227:23: note: in expansion of macro ‘_RoDict_TryMalloc’
 * >>   227 |         dict        = _RoDict_TryMalloc(sizeof_dict);
 *
 * We intentionally set "sizeof_dict" to "SIZE_MAX" when the actual size-calculation would
 * overflow, so-as to cause an (intended) OOM error somewhere down-stream. However, since
 * we are now annotating our allocator functions, GCC doesn't seem to like that.
 *
 * Luckily there's a specific compiler warning for this, so we can just disable that one
 * specifically. */
__pragma_GCC_diagnostic_ignored(Walloc_size_larger_than)
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


/* #define CONFIG_NO_THREADS */

/* CONFIG:  Enable tracing of all incref()s and decref()s that
 *          happen to an object over the course of its lifetime.
 *          When deemon shuts down, dump that reference count
 *          history for all dynamically allocated objects that
 *          still haven't been destroyed.
 *       -> Since this includes every incref and decref operation
 *          ever performed on the object, as well as the reference
 *          counter values at that point in time, it becomes fairly
 *          easy to spot the point when the reference counter became
 *          desynchronized due to the lack of a required decref.
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
 *          that have been proven to cause reference leaks, in order
 *          to analyze what exactly is causing them. */
#if (!defined(CONFIG_TRACE_REFCHANGES) && \
     !defined(CONFIG_NO_TRACE_REFCHANGES))
#if !defined(NDEBUG) && 0
#define CONFIG_TRACE_REFCHANGES
#else /* !NDEBUG */
#define CONFIG_NO_TRACE_REFCHANGES 
#endif /* NDEBUG */
#endif /* !CONFIG_[NO_]TRACE_REFCHANGES */

#if (!defined(CONFIG_NO_BADREFCNT_CHECKS) && \
     !defined(CONFIG_BADREFCNT_CHECKS))
#ifdef NDEBUG
#define CONFIG_NO_BADREFCNT_CHECKS
#else /* NDEBUG */
#define CONFIG_BADREFCNT_CHECKS
#endif /* !NDEBUG */
#endif /* !CONFIG_[NO_]BADREFCNT_CHECKS */


#ifdef CONFIG_TRACE_REFCHANGES
/* Assembly interpreters do not implement the additional
 * overhead required to properly track reference counts.
 * -> So just disable them! */
#undef CONFIG_HAVE_EXEC_ASM
#else /* CONFIG_TRACE_REFCHANGES */
#define CONFIG_NO_TRACE_REFCHANGES
#endif /* !CONFIG_TRACE_REFCHANGES */


/* Config option: also write "*__unsupported" impls to operator slots */
#if (!defined(CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS) && \
     !defined(CONFIG_NO_CACHE_UNSUPPORTED_NATIVE_OPERATORS))
#if 1
#define CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS
#else
#define CONFIG_NO_CACHE_UNSUPPORTED_NATIVE_OPERATORS
#endif
#endif /* !CONFIG_[NO_]CACHE_UNSUPPORTED_NATIVE_OPERATORS */


/* Config option: provide optimized code-paths for `foo(args...)' expressions */
#if (!defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && \
     !defined(CONFIG_NO_CALLTUPLE_OPTIMIZATIONS))
#ifndef __OPTIMIZE_SIZE__
#define CONFIG_CALLTUPLE_OPTIMIZATIONS
#else /* !__OPTIMIZE_SIZE__ */
#define CONFIG_NO_CALLTUPLE_OPTIMIZATIONS
#endif /* __OPTIMIZE_SIZE__ */
#endif /* !CONFIG_[NO_]CALLTUPLE_OPTIMIZATIONS */


/* Config option: provide special optimizations for user-classes without bases */
#if (!defined(CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS) && \
     !defined(CONFIG_NO_NOBASE_OPTIMIZED_CLASS_OPERATORS))
#ifndef __OPTIMIZE_SIZE__
#define CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
#else /* !__OPTIMIZE_SIZE__ */
#define CONFIG_NO_NOBASE_OPTIMIZED_CLASS_OPERATORS
#endif /* __OPTIMIZE_SIZE__ */
#endif /* !CONFIG_[NO_]NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* Config option: how are latin-1 1-char strings cached? */
#if (!defined(CONFIG_STRING_LATIN1_STATIC) && \
     !defined(CONFIG_STRING_LATIN1_CACHED) && \
     !defined(CONFIG_STRING_LATIN1_NORMAL))
#if defined(__OPTIMIZE_SIZE__) && defined(__OPTIMIZE__)
#define CONFIG_STRING_LATIN1_CACHED /* Latin-1 characters are created dynamically, but then cached */
#elif defined(__OPTIMIZE_SIZE__)
#define CONFIG_STRING_LATIN1_NORMAL /* Latin-1 characters are created dynamically */
#else /* ... */
#define CONFIG_STRING_LATIN1_STATIC /* Statically define all latin-1 characters */
#endif /* !... */
#endif /* !CONFIG_STRING_LATIN1_... */


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
#define CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
#else /* _MSC_VER */
#define CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#endif /* !_MSC_VER */
#endif /* !CONFIG_DEFAULT_MESSAGE_FORMAT_GCC */
#else /* CONFIG_DEFAULT_MESSAGE_FORMAT_GCC */
#define CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#endif /* !CONFIG_DEFAULT_MESSAGE_FORMAT_GCC */
#else /* !CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC */
#undef CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#endif /* CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC */
#elif defined(CONFIG_DEFAULT_MESSAGE_FORMAT_GCC)
#if (CONFIG_DEFAULT_MESSAGE_FORMAT_GCC + 0) == 0
#undef CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#define CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
#endif
#elif defined(_MSC_VER)
#define CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
#else /* ... */
#define CONFIG_DEFAULT_MESSAGE_FORMAT_GCC
#endif /* !... */



#ifdef CONFIG_FORCE_HOST_WINDOWS
#define CONFIG_HOST_WINDOWS
#elif defined(CONFIG_FORCE_HOST_UNIX)
#define CONFIG_HOST_UNIX
#else /* CONFIG_FORCE_HOST_... */
#if (defined(__WINDOWS__) || defined(_WIN16) || defined(WIN16) ||    \
     defined(_WIN32) || defined(WIN32) || defined(_WIN64) ||         \
     defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
     defined(_WIN32_WCE) || defined(WIN32_WCE))
#define CONFIG_HOST_WINDOWS
#endif /* Windows... */
#if (defined(__CYGWIN__) || defined(__CYGWIN32__) ||                    \
     defined(__unix__) || defined(__unix) || defined(unix) ||           \
     defined(__linux__) || defined(__linux) || defined(linux) ||        \
     defined(__KOS__) || defined(__NetBSD__) || defined(__FreeBSD__) || \
     defined(__solaris__) || defined(__DragonFly__))
#define CONFIG_HOST_UNIX
#endif /* Unix... */
#endif /* !CONFIG_FORCE_HOST_... */


/* Include metrics and support for automatically re-compiling `DeeCodeObject'
 * and `DeeFunctionObject' objects into host assembly (using `_hostasm') once
 * - the same code object has used to create functions a given # of times
 * - the same function has been called a given # of times */
#if (!defined(CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE) && \
     !defined(CONFIG_NO_HOSTASM_AUTO_RECOMPILE))
#if ((defined(__i386__) || defined(__x86_64__)) && \
     (defined(CONFIG_HOST_WINDOWS) || defined(CONFIG_HOST_UNIX)))
#define CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#else /* ... */
#define CONFIG_NO_HOSTASM_AUTO_RECOMPILE
#endif /* !... */
#endif /* !CONFIG_[HAVE|NO]_HOSTASM_AUTO_RECOMPILE */

/* Enable support for metrics in DeeCodeObject and DeeFunctionObject */
#if (!defined(CONFIG_HAVE_CODE_METRICS) && \
     !defined(CONFIG_NO_CODE_METRICS))
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
#define CONFIG_HAVE_CODE_METRICS
#else /* ... */
#define CONFIG_NO_CODE_METRICS
#endif /* !... */
#endif /* !CONFIG_[HAVE|NO]CODE_METRICS */


#if ((!defined(__i386__) && !defined(__x86_64__)) || \
     (!defined(CONFIG_HOST_WINDOWS) && !defined(CONFIG_HOST_UNIX)))
#undef CONFIG_NO_OBJECT_SLABS
#define CONFIG_NO_OBJECT_SLABS /* Unrecognized environment (disable slabs) */
#endif /* ... */




/************************************************************************/
/* EXPERIMENTAL/WIP FEATURES                                            */
/************************************************************************/

/* Experimental feature switch: use altered status codes for "boundattr" / "bounditem".
 * When enabled, "hasitem"/"hasattr" can be implemented by aliasing "bounditem"/"boundattr" */
#if (!defined(CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS) && \
     !defined(CONFIG_NO_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS))
#if 1
#define CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#else
#define CONFIG_NO_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

/* Experimental feature switch: HashSet remembers the order in which items are inserted. */
#if (!defined(CONFIG_EXPERIMENTAL_ORDERED_HASHSET) && \
     !defined(CONFIG_NO_EXPERIMENTAL_ORDERED_HASHSET))
#if 0 /* TODO: Implementation is incomplete */
#define CONFIG_EXPERIMENTAL_ORDERED_HASHSET
#else
#define CONFIG_NO_EXPERIMENTAL_ORDERED_HASHSET
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_ORDERED_HASHSET */

/* Experimental feature switch: "Bytes" has an in-use counter that is
 * non-zero whenever some thread is accessing the pointed-to memory. */
#if (!defined(CONFIG_EXPERIMENTAL_BYTES_INUSE) && \
     !defined(CONFIG_NO_EXPERIMENTAL_BYTES_INUSE))
#if 0 /* TODO: Implementation is incomplete */
#define CONFIG_EXPERIMENTAL_BYTES_INUSE
#else
#define CONFIG_NO_EXPERIMENTAL_BYTES_INUSE
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_BYTES_INUSE */

/* Experimental feature switch: Use custom heap implementation based on
 * dlmalloc, rather than the host system's native malloc(3). (needed in
 * order to support `DeeHeap_RegionOf()') */
#if (!defined(CONFIG_EXPERIMENTAL_CUSTOM_HEAP) && \
     !defined(CONFIG_NO_EXPERIMENTAL_CUSTOM_HEAP))
#if 1
#define CONFIG_EXPERIMENTAL_CUSTOM_HEAP
#else
#define CONFIG_NO_EXPERIMENTAL_CUSTOM_HEAP
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_CUSTOM_HEAP */

/* Experimental feature switch: ST_ISDIR can be imported as a module, and modules
 * whose names match an equally named directory within the same containing directory
 * also expose all the symbols from that directory within themselves:
 * Files:
 * - mylib/foo.dee          >> global final symbolFromFoo = 42;
 * - mylib/foo/bar.dee      >> global final symbolFromBar = "Wow!";
 * - mylib/foobar/baz.dee   >> global final symbolFromBaz = "This is baz";
 * Code:
 * >> import mylib;
 * >> print mylib.foo.symbolFromFoo;        // 42
 * >> print mylib.foo.bar.symbolFromBar;    // Wow!
 * >> print mylib.foobar.baz.symbolFromBaz; // This is baz
 *
 * Additionally, this feature changes the builtin "import" function to an object,
 * that also overrides "operator ." as an alias for its call-operator, meaning you
 * can now write `import.deemon.string' instead of `string from deemon'
 *
 * NOTE: The ambiguity in the following is resolved by preferring symbols over directories:
 * - mylib/foo.dee     >> global final bar = "Oops. Overwrote the other module";
 * - mylib/foo/bar.dee >> global final notGonnaSeeThisOne = "Nope";
 * >> import mylib, mylib.foo.bar;
 * >> print mylib.foo.bar;          // "Oops. Overwrote the other module"
 * >> print bar.notGonnaSeeThisOne; // "Nope"
 */
#if (!defined(CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES) && \
     !defined(CONFIG_NO_EXPERIMENTAL_MODULE_DIRECTORIES))
#if 1 /* Enabled by default (seems stable enough at this point) */
#define CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#else
#define CONFIG_NO_EXPERIMENTAL_MODULE_DIRECTORIES
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_MODULE_DIRECTORIES */

/* Experimental feature switch: Use a new (mmap-able) file format for dec files */
#if (!defined(CONFIG_EXPERIMENTAL_CUSTOM_HEAP) || \
     !defined(CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES))
#undef CONFIG_EXPERIMENTAL_MMAP_DEC
#undef CONFIG_NO_EXPERIMENTAL_MMAP_DEC
#define CONFIG_NO_EXPERIMENTAL_MMAP_DEC
#elif (!defined(CONFIG_EXPERIMENTAL_MMAP_DEC) && \
       !defined(CONFIG_NO_EXPERIMENTAL_MMAP_DEC))
#if 1
#define CONFIG_EXPERIMENTAL_MMAP_DEC
#else
#define CONFIG_NO_EXPERIMENTAL_MMAP_DEC
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_MMAP_DEC */


/* Experimental feature switch: Use "tp_serialize" to implement "deepcopy" */
#if (!defined(CONFIG_EXPERIMENTAL_SERIALIZED_DEEPCOPY) && \
     !defined(CONFIG_NO_EXPERIMENTAL_SERIALIZED_DEEPCOPY))
#if 0 /* TODO: Doesn't work, yet */
#define CONFIG_EXPERIMENTAL_SERIALIZED_DEEPCOPY
#else
#define CONFIG_NO_EXPERIMENTAL_SERIALIZED_DEEPCOPY
#endif
#endif /* !CONFIG_[NO_]EXPERIMENTAL_SERIALIZED_DEEPCOPY */
/************************************************************************/


#ifdef CONFIG_HOST_WINDOWS
#ifndef _WIN32_WINNT
/* Limit windows headers to only provide XP stuff. */
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#endif /* !_WIN32_WINNT */
#endif /* CONFIG_HOST_WINDOWS */


#ifdef __CC__

#if (defined(__INTELLISENSE__) || defined(__VASSISTX_INSPECT__)) && defined(__cplusplus)
/* Highlight invalid usage of `NULL' in functions returning `int' */
#undef NULL
#define NULL __NULLPTR
#endif /* __INTELLISENSE__ && __cplusplus */

#ifndef Dee_BREAKPOINT
#ifdef _MSC_VER
DECL_BEGIN
extern void (__debugbreak)(void);
DECL_END
#pragma intrinsic(__debugbreak)
#define Dee_BREAKPOINT() __debugbreak()
#elif defined(__COMPILER_HAVE_GCC_ASM) && !defined(__NO_XBLOCK) && (defined(__i386__) || defined(__x86_64__))
#define Dee_BREAKPOINT() XBLOCK({ __asm__ __volatile__("int {$}3" : ); (void)0; })
#else /* ... */
#define Dee_BREAKPOINT_IS_NOOP
#define Dee_BREAKPOINT() (void)0
#endif /* !... */
#endif /* !Dee_BREAKPOINT */

#ifdef CONFIG_BUILDING_DEEMON
#define DFUNDEF __EXPDEF
#define DDATDEF __EXPDEF
#else /* CONFIG_BUILDING_DEEMON */
#define DFUNDEF __IMPDEF
#define DDATDEF __IMPDEF
#endif /* !CONFIG_BUILDING_DEEMON */

/* Const modifier for static type callback-table declaration */
#ifndef Dee_tpconst
#ifdef __INTELLISENSE__
#define Dee_tpconst_IS_const
#define Dee_tpconst const
#elif (defined(__PIC__) || defined(__PIE__) || \
       defined(__pic__) || defined(__pie__))
#undef Dee_tpconst_IS_const
#define Dee_tpconst /* nothing */
#elif defined(CONFIG_BUILDING_DEEMON)
#define Dee_tpconst_IS_const
#define Dee_tpconst const
#else /* ... */
#define Dee_tpconst_IS_const
#define Dee_tpconst const
#endif /* !... */
#endif /* !Dee_tpconst */

#ifdef DEE_SOURCE
#define tpconst Dee_tpconst
#ifdef Dee_tpconst_IS_const
#define INTERN_TPCONST INTERN_CONST
#define PUBLIC_TPCONST PUBLIC_CONST
#else /* Dee_tpconst_IS_const */
#define INTERN_TPCONST INTERN
#define PUBLIC_TPCONST PUBLIC
#endif /* !Dee_tpconst_IS_const */
#endif /* DEE_SOURCE */



#ifdef __GNUC__
/* Define if the compiler allows labels to be
 * addressed: `foo: printf("foo = %p", &&foo);'
 * ... as well as such addresses to be used
 * by `goto': `void *ip = &&foo; goto *ip;' */
#define CONFIG_COMPILER_HAVE_ADDRESSIBLE_LABELS
#endif /* __GNUC__ */

/* Calling convention used for the deemon API
 * DCALL_CALLER_CLEANUP: When defined, a function:
 *                       >> int DCALL foo(int a, int b);
 *                       can safely be called like:
 *                       >> int result = (*(int(DCALL *)(int, int, int, int, int))&foo)(10, 20, -1, -1, -1);
 *                  iow: any additional arguments to functions are silently ignored, and will not
 *                       cause any problems because the ABI specifies that any CLEANUP is done by
 *                       the CALLER of a function (and argument registers/locations are allocated
 *                       left-to-right).
 * DCALL_RETURN_COMMON:  When defined, a function:
 *                       >> size_t DCALL foo(int a, int b);
 *                       can safely be called like (so-long as the return value fits into an "int"):
 *                       >> int result = (*(int(DCALL *)(int, int))&foo)(10, 20);
 *                  iow: Integer return types <= sizeof(__REGISTER_TYPE__) are always passed via
 *                       the same register(s) (or registers that shadow/alias each other). */
#ifndef DCALL
#if defined(__i386__) && !defined(__x86_64__)
#define DCALL __ATTR_STDCALL
#undef DCALL_CALLER_CLEANUP
#define DCALL_RETURN_COMMON
#elif defined(__x86_64__) || defined(__arm__)
#define DCALL /* nothing (use default calling convention) */
#define DCALL_CALLER_CLEANUP /* Known to be the case on these architectures */
#define DCALL_RETURN_COMMON
#else /* ... */
#define DCALL /* nothing (use default calling convention) */
#undef DCALL_CALLER_CLEANUP
#undef DCALL_RETURN_COMMON
#endif /* !... */
#endif /* !DCALL */


/* Calling convention for short leaf functions with up to 2 arguments (e.g. `Dee_HashCombine'). */
#ifndef DFCALL
#if defined(__i386__) && !defined(__x86_64__)
#define DFCALL __ATTR_FASTCALL /* arg0: %ecx, arg1: %edx, argN: 4+N*4(%esp) (callee-cleanup) */
#else /* __i386__ && !__x86_64__ */
#define DFCALL /* nothing (use default calling convention) */
#endif /* !__i386__ || __x86_64__ */
#endif /* !DFCALL */

/* Calling convention for `Dee_formatprinter_t' */
#ifndef DPRINTER_CC
#if defined(__KOS__) && __KOS_VERSION__ >= 400
/* We want to be ABI-compatible with KOS's native `pformatprinter' system. */
#if defined(__KOS_SYSTEM_HEADERS__) || __has_include(<bits/crt/format-printer.h>)
#include <bits/crt/format-printer.h>
#define DPRINTER_CC_IS___FORMATPRINTER_CC
#define DPRINTER_CC __FORMATPRINTER_CC
#elif __has_include(<format-printer.h>)
#include <format-printer.h>
#define DPRINTER_CC_IS_FORMATPRINTER_CC
#define DPRINTER_CC FORMATPRINTER_CC
#endif /* ... */
#endif /* __KOS__ && __KOS_VERSION__ >= 400 */
#ifndef DPRINTER_CC
#define DPRINTER_CC DCALL
#endif /* !DPRINTER_CC */
#endif /* !DPRINTER_CC */

/* TODO: Get rid of `DWEAK' -- code should use comment-annotation "[lock(ATOMIC)]" instead! */
#ifndef DWEAK
#define DWEAK /* Annotation for data that is thread-volatile. */
#endif /* !DWEAK */

#if !defined(NDEBUG) && !defined(CONFIG_NO_CHECKMEMORY) && defined(_DEBUG)
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
DECL_BEGIN
DFUNDEF void DCALL DeeHeap_CheckMemory(void);
#define Dee_CHECKMEMORY() DeeHeap_CheckMemory()
DECL_END
#else /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
#if defined(CONFIG_HOST_WINDOWS) && defined(_MSC_VER)
#ifdef __INTELLISENSE__
#define Dee_CHECKMEMORY() Dee_ASSERT((_CrtCheckMemory)())
#else /* __INTELLISENSE__ */
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#define Dee_CHECKMEMORY() (DBG_ALIGNMENT_DISABLE(), Dee_ASSERT((_CrtCheckMemory)()), DBG_ALIGNMENT_ENABLE())
#endif /* !__INTELLISENSE__ */
DECL_BEGIN
#if !defined(_MSC_VER) || defined(_DLL)
extern __ATTR_DLLIMPORT int (ATTR_CDECL _CrtCheckMemory)(void); /*!export-*/
#else /* !_MSC_VER || _DLL */
extern int (ATTR_CDECL _CrtCheckMemory)(void);
#endif /* _MSC_VER && !_DLL */
DECL_END
#endif /* CONFIG_HOST_WINDOWS && _MSC_VER */
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
#endif /* !NDEBUG */

DECL_BEGIN
#if defined(__INTELLISENSE__) && defined(__cplusplus)
extern "C++" template<class __ITS_T> __ITS_T ____INTELLISENSE_req_type(__ITS_T x); /*!export-*/
#define Dee_REQUIRES_TYPE(T, x) ____INTELLISENSE_req_type<T>(x)
#else /* __INTELLISENSE__ && __cplusplus */
#define Dee_REQUIRES_TYPE(T, x) (x)
#endif /* !__INTELLISENSE__ || !__cplusplus */

#if !defined(NDEBUG) && !defined(NDEBUG_DPRINT)
#define Dee_DPRINT_SET_ENABLED(is) (void)(_Dee_dprint_enabled = (is) ? 1 : 0)
#define Dee_DPRINT(message)        (_Dee_dprint_enabled ? _Dee_dprint(message) : (void)0)
#define Dee_DPRINTER               _Dee_dprinter
#define Dee_DPRINTF(...)           (_Dee_dprint_enabled ? _Dee_dprintf(__VA_ARGS__) : (void)0)
#define Dee_VDPRINTF(format, args) _Dee_vdprintf(format, args) /* Always invoke because `format' may mandate a decref() operation! */
DDATDEF int _Dee_dprint_enabled;
DFUNDEF NONNULL((1)) void (DCALL _Dee_dprint)(char const *__restrict message);
DFUNDEF NONNULL((1)) void (_Dee_dprintf)(char const *__restrict format, ...);
#ifdef __INTELLISENSE__
DFUNDEF NONNULL((1)) void (DCALL _Dee_vdprintf)(char const *__restrict format, __builtin_va_list args);
DFUNDEF __SSIZE_TYPE__ (DPRINTER_CC _Dee_dprinter)(void *arg, char const *__restrict data, __SIZE_TYPE__ datalen);
#else /* __INTELLISENSE__ */
DFUNDEF NONNULL((1)) void (DCALL _Dee_vdprintf)(char const *__restrict format, va_list args);
DFUNDEF __SSIZE_TYPE__ (DPRINTER_CC _Dee_dprinter)(void *arg, char const *__restrict data, size_t datalen);
#endif /* !__INTELLISENSE__ */
#endif /* !NDEBUG && !NDEBUG_DPRINT */

/* Assertion handlers */
DFUNDEF void (DCALL _DeeAssert_Fail)(char const *expr, char const *file, int line);
DFUNDEF void (_DeeAssert_Failf)(char const *expr, char const *file, int line, char const *format, ...);
DFUNDEF ATTR_NORETURN void (DCALL _DeeAssert_XFail)(char const *expr, char const *file, int line);
DFUNDEF ATTR_NORETURN void (_DeeAssert_XFailf)(char const *expr, char const *file, int line, char const *format, ...);
DECL_END

#define _Dee_XFatal(expr)       _DeeAssert_XFail(expr, __FILE__, __LINE__)
#define _Dee_XFatalf(expr, ...) _DeeAssert_XFailf(expr, __FILE__, __LINE__, __VA_ARGS__)
#ifdef Dee_BREAKPOINT_IS_NOOP
#define _Dee_Fatal(expr)       _DeeAssert_XFail(expr, __FILE__, __LINE__)
#define _Dee_Fatalf(expr, ...) _DeeAssert_XFailf(expr, __FILE__, __LINE__, __VA_ARGS__)
#else /* Dee_BREAKPOINT_IS_NOOP */
#define _Dee_Fatal(expr)       (_DeeAssert_Fail(expr, __FILE__, __LINE__), Dee_BREAKPOINT())
#define _Dee_Fatalf(expr, ...) (_DeeAssert_Failf(expr, __FILE__, __LINE__, __VA_ARGS__), Dee_BREAKPOINT())
#endif /* !Dee_BREAKPOINT_IS_NOOP */
#define Dee_Fatal()      _Dee_Fatal(NULL)
#define Dee_Fatalf(...)  _Dee_Fatalf(NULL, __VA_ARGS__)
#define Dee_XFatal()     _Dee_XFatal(NULL)
#define Dee_XFatalf(...) _Dee_XFatalf(NULL, __VA_ARGS__)

#ifndef Dee_ASSERT
#if !defined(NDEBUG) && !defined(NDEBUG_ASSERT)
#ifdef Dee_BREAKPOINT_IS_NOOP
#define Dee_ASSERT(expr)       (void)((expr) || (_DeeAssert_XFail(#expr, __FILE__, __LINE__), 0))
#define Dee_ASSERTF(expr, ...) (void)((expr) || (_DeeAssert_XFailf(#expr, __FILE__, __LINE__, __VA_ARGS__), 0))
#else /* Dee_BREAKPOINT_IS_NOOP */
#define Dee_ASSERT(expr)       (void)((expr) || (_DeeAssert_Fail(#expr, __FILE__, __LINE__), Dee_BREAKPOINT(), 0))
#define Dee_ASSERTF(expr, ...) (void)((expr) || (_DeeAssert_Failf(#expr, __FILE__, __LINE__, __VA_ARGS__), Dee_BREAKPOINT(), 0))
#endif /* !Dee_BREAKPOINT_IS_NOOP */
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

/* Override hooks from `<hybrid/__assert.h>' */
#ifndef __GUARD_HYBRID___ASSERT_H
#define __GUARD_HYBRID___ASSERT_H
#endif /* !__GUARD_HYBRID___ASSERT_H */
#undef __hybrid_assert
#undef __hybrid_assertf
#undef __hybrid_assertion_failed
#undef __hybrid_assertion_failedf
#define __hybrid_assertion_failed  _Dee_Fatal
#define __hybrid_assertion_failedf _Dee_Fatalf
#define __hybrid_assert            Dee_ASSERT
#define __hybrid_assertf           Dee_ASSERTF
#endif /* DEE_SOURCE */


#ifndef Dee_DPRINT
#define Dee_DPRINT_IS_NOOP
#define Dee_DPRINT_SET_ENABLED(is) (void)0
#define Dee_DPRINT(message)        (void)0
#define Dee_DPRINTF(...)           (void)0
#define Dee_VDPRINTF(format, args) (void)0
#endif /* !Dee_DPRINT */

#ifndef Dee_CHECKMEMORY
#define Dee_CHECKMEMORY_IS_NOOP
#define Dee_CHECKMEMORY() (void)0
#endif /* !Dee_CHECKMEMORY */

#endif /* __CC__ */


#ifdef CONFIG_BUILDING_DEEMON
/* Expand to the assembly symbol name of a INTERN/PUBLIC function declared via "DCALL". */
#if (defined(__i386__) && !defined(__x86_64__)) && defined(__PE__)
#if 0
#define DCALL_ASSEMBLY_NAME(x, s) PP_CAT4(__USER_LABEL_PREFIX__, x, @, s)
#else
#define DCALL_ASSEMBLY_NAME(x, s) PP_CAT2(__USER_LABEL_PREFIX__, x@s)
#endif
#else /* ... */
#define DCALL_ASSEMBLY_NAME(x, s) PP_CAT2(__USER_LABEL_PREFIX__, x)
#endif /* !... */
#endif /* CONFIG_BUILDING_DEEMON */


/* NOTE: This config option only affects internal documentation strings. */
#ifndef CONFIG_NO_DOC
#define Dee_DOC(x)           x
#define Dee_DOC_DEF(name, x) INTERN_CONST char const name[] = x
#define Dee_DOC_REF(name)    INTDEF char const name[]
#define Dee_DOC_GET(name)    name
#else /* !CONFIG_NO_DOC */
#define Dee_DOC(x)           ((char const *)NULL)
#define Dee_DOC_DEF(name, x) /* nothing */
#define Dee_DOC_REF(name)    /* nothing */
#define Dee_DOC_GET(name)    ((char const *)NULL)
#endif /* CONFIG_NO_DOC */

#ifdef DEE_SOURCE
#define DOC     Dee_DOC
#define DOC_DEF Dee_DOC_DEF
#define DOC_REF Dee_DOC_REF
#define DOC_GET Dee_DOC_GET
#endif /* DEE_SOURCE */

#endif /* !GUARD_DEEMON_API_H */
