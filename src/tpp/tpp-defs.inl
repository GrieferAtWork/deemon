/* Copyright (c) 2017-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2017-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef KWD
#define TPP_DEFS_DEFINES_KWD
#define KWD(name, str)
#endif /* !KWD */
#ifndef KWD_FLAGS
#define TPP_DEFS_DEFINES_KWD_FLAGS
#define KWD_FLAGS(kwd, flags)
#endif /* !KWD_FLAGS */
#ifndef WGROUP
#define TPP_DEFS_DEFINES_WGROUP
#define WGROUP(name, str, default)
#endif /* !WGROUP */
#ifndef WARNING
#define TPP_DEFS_DEFINES_WARNING
#define WARNING(name, groups, default)
#endif /* !WARNING */
#ifndef WARNING_NAMESPACE
#define TPP_DEFS_DEFINES_WARNING_NAMESPACE
#define WARNING_NAMESPACE(name, start)
#endif /* !WARNING_NAMESPACE */
#ifndef MACRO
#define TPP_DEFS_DEFINES_MACRO
/* Reference a keyword that would behave as a predefined
 * macro only defined when `if' evaluates to true at runtime.
 * NOTE: Such a macro can still be re-defined by the user, but may
 *       later be #undef'd again to restore its original meaning. */
#define MACRO(name, if)
#endif /* !MACRO */
#ifndef BUILTIN_MACRO
#define TPP_DEFS_DEFINES_BUILTIN_MACRO
#define BUILTIN_MACRO(name, value)
#endif /* !BUILTIN_MACRO */
#ifndef RT_BUILTIN_MACRO
#define TPP_DEFS_DEFINES_RT_BUILTIN_MACRO
#define RT_BUILTIN_MACRO(name, value)
#endif /* !RT_BUILTIN_MACRO */
#ifndef BUILTIN_FUNCTION
#define TPP_DEFS_DEFINES_BUILTIN_FUNCTION
#define BUILTIN_FUNCTION(name, argc, expr)
#endif /* !BUILTIN_FUNCTION */
#ifndef WARNING_MESSAGE
#define TPP_DEFS_DEFINES_WARNING_MESSAGE
#define WARNING_MESSAGE(name, expr)
#endif /* !WARNING_MESSAGE */
#ifndef EXTENSION
#define TPP_DEFS_DEFINES_EXTENSION
#define EXTENSION(name, str, default)
#endif /* !EXTENSION */



#define DEF_K(name)          KWD(KWD_##name, #name)
#define DEF_M(name)          KWD(KWD_##name, #name) MACRO(KWD_##name, 1)
#define DEF_M_IF(name, expr) KWD(KWD_##name, #name) MACRO(KWD_##name, expr)
#define DEF_WARNING(name, groups, default, expr) \
	WARNING(name, groups, default)               \
	WARNING_MESSAGE(name, expr)

#if TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS
#define HAS_BUILTIN_IF(name, if)   KWD_FLAGS(name, (if) ? TPP_KEYWORDFLAG_HAS_BUILTIN : 0u)
#define HAS_FEATURE_IF(name, if)   KWD_FLAGS(name, (if) ? TPP_KEYWORDFLAG_HAS_FEATURE : 0u)
#define HAS_EXTENSION_IF(name, if) KWD_FLAGS(name, (if) ? TPP_KEYWORDFLAG_HAS_EXTENSION : 0u)
#else /* TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS */
#define HAS_BUILTIN_IF(name, if)   KWD(KWD_##name, #name) KWD_FLAGS(KWD_##name, (if) ? TPP_KEYWORDFLAG_HAS_BUILTIN : 0u)
#define HAS_FEATURE_IF(name, if)   KWD(KWD_##name, #name) KWD_FLAGS(KWD_##name, (if) ? TPP_KEYWORDFLAG_HAS_FEATURE : 0u)
#define HAS_EXTENSION_IF(name, if) KWD(KWD_##name, #name) KWD_FLAGS(KWD_##name, (if) ? TPP_KEYWORDFLAG_HAS_EXTENSION : 0u)
#endif /* !TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS */

#define PREDEFINED_KWDMACRO_IF(name, str, if, value)                 KWD(name, str) MACRO(name, if) BUILTIN_MACRO(name, value)
#define PREDEFINED_KWDMACRO(name, str, value)                        KWD(name, str) MACRO(name, 1) BUILTIN_MACRO(name, value)
#define PREDEFINED_MACRO_IF(name, if, value)                         PREDEFINED_KWDMACRO_IF(KWD_##name, #name, if, value)
#define PREDEFINED_MACRO(name, value)                                PREDEFINED_KWDMACRO(KWD_##name, #name, value)
#define PREDEFINED_RT_KWDMACRO_IF(name, str, if, value)              KWD(name, str) MACRO(name, if) RT_BUILTIN_MACRO(name, value)
#define PREDEFINED_RT_KWDMACRO(name, str, value)                     KWD(name, str) MACRO(name, 1) RT_BUILTIN_MACRO(name, value)
#define PREDEFINED_RT_MACRO_IF(name, if, value)                      PREDEFINED_RT_KWDMACRO_IF(KWD_##name, #name, if, value)
#define PREDEFINED_RT_MACRO(name, value)                             PREDEFINED_RT_KWDMACRO(KWD_##name, #name, value)
#define PREDEFINED_KWDFUNCTION_IF(name, str, if, argc, expr)         KWD(name, str) BUILTIN_FUNCTION(name, (if) ? (int)(argc) : -1, expr)
#define PREDEFINED_KWDFUNCTION(name, str, argc, expr)                KWD(name, str) BUILTIN_FUNCTION(name, argc, expr)
#define PREDEFINED_FUNCTION_IF(name, if, argc, expr)                 PREDEFINED_KWDFUNCTION_IF(KWD_##name, #name, if, argc, expr)
#define PREDEFINED_FUNCTION(name, argc, expr)                        PREDEFINED_KWDFUNCTION_IF(KWD_##name, #name, 1, argc, expr)
#if TPP_CONFIG_MINGCCFUNC < 2
#define PREDEFINED_BUILTIN_KWDFUNCTION_IF(name, str, if, argc, expr) KWD(name, str) KWD_FLAGS(name, (HAS(EXT_BUILTIN_FUNCTIONS) && (if)) ? (TPP_KEYWORDFLAG_HAS_BUILTIN|TPP_KEYWORDFLAG_HAS_TPP_BUILTIN) : 0) BUILTIN_FUNCTION(name, (if) ? (int)(argc) : -1, expr)
#define PREDEFINED_BUILTIN_KWDFUNCTION(name, str, argc, expr)        KWD(name, str) KWD_FLAGS(name, HAS(EXT_BUILTIN_FUNCTIONS) ? (TPP_KEYWORDFLAG_HAS_BUILTIN|TPP_KEYWORDFLAG_HAS_TPP_BUILTIN) : 0) BUILTIN_FUNCTION(name, argc, expr)
#else /* TPP_CONFIG_MINGCCFUNC < 2 */
#define PREDEFINED_BUILTIN_KWDFUNCTION_IF(name, str, if, argc, expr) KWD(name, str) KWD_FLAGS(name, (if) ? (TPP_KEYWORDFLAG_HAS_BUILTIN|TPP_KEYWORDFLAG_HAS_TPP_BUILTIN) : 0) BUILTIN_FUNCTION(name, (if) ? (int)(argc) : -1, expr)
#define PREDEFINED_BUILTIN_KWDFUNCTION(name, str, argc, expr)        KWD(name, str) KWD_FLAGS(name, TPP_KEYWORDFLAG_HAS_BUILTIN|TPP_KEYWORDFLAG_HAS_TPP_BUILTIN) BUILTIN_FUNCTION(name, argc, expr)
#endif /* TPP_CONFIG_MINGCCFUNC >= 2 */
#define PREDEFINED_BUILTIN_FUNCTION_IF(name, if, argc, expr)         PREDEFINED_BUILTIN_KWDFUNCTION_IF(KWD_##name, #name, if, argc, expr)
#define PREDEFINED_BUILTIN_FUNCTION(name, argc, expr)                PREDEFINED_BUILTIN_KWDFUNCTION_IF(KWD_##name, #name, 1, argc, expr)

#define TPP_PP_STR2(x) #x
#define TPP_PP_STR(x)  TPP_PP_STR2(x)

/* Keywords for known preprocessor directives. */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_IF) && !defined(TPP_CONFIG_USERDEFINED_KWD_if)
DEF_K(if)           /* #if defined(FOO) && FOO == 10. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_IF && !TPP_CONFIG_USERDEFINED_KWD_if */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_IFDEF) && !defined(TPP_CONFIG_USERDEFINED_KWD_ifdef)
DEF_K(ifdef)        /* #ifdef FOOBAR. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_IFDEF && !TPP_CONFIG_USERDEFINED_KWD_ifdef */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_IFNDEF) && !defined(TPP_CONFIG_USERDEFINED_KWD_ifndef)
DEF_K(ifndef)       /* #ifndef FOOBAR. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_IFNDEF && !TPP_CONFIG_USERDEFINED_KWD_ifndef */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_ELIF) && !defined(TPP_CONFIG_USERDEFINED_KWD_elif)
DEF_K(elif)         /* #elif defined(BAR) && BAR == 20. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_ELIF && !TPP_CONFIG_USERDEFINED_KWD_elif */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_ELSE) && !defined(TPP_CONFIG_USERDEFINED_KWD_else)
DEF_K(else)         /* #else. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_ELSE && !TPP_CONFIG_USERDEFINED_KWD_else */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_ENDIF) && !defined(TPP_CONFIG_USERDEFINED_KWD_endif)
DEF_K(endif)        /* #endif. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_ENDIF && !TPP_CONFIG_USERDEFINED_KWD_endif */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_DEFINE) && !defined(TPP_CONFIG_USERDEFINED_KWD_define)
DEF_K(define)       /* #define FOO 42. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_DEFINE && !TPP_CONFIG_USERDEFINED_KWD_define */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_DEFINED) && !defined(TPP_CONFIG_USERDEFINED_KWD_defined)
DEF_K(defined)      /* #if defined(FOO). */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_DEFINED && !TPP_CONFIG_USERDEFINED_KWD_defined */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_UNDEF) && !defined(TPP_CONFIG_USERDEFINED_KWD_undef)
DEF_K(undef)        /* #undef FOO. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_UNDEF && !TPP_CONFIG_USERDEFINED_KWD_undef */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_INCLUDE) && !defined(TPP_CONFIG_USERDEFINED_KWD_include)
DEF_K(include)      /* #include "header.h". */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_INCLUDE && !TPP_CONFIG_USERDEFINED_KWD_include */
#if !defined(TPP_CONFIG_EXTENSION_INCLUDE_NEXT) || TPP_CONFIG_EXTENSION_INCLUDE_NEXT
#if !defined(TPP_CONFIG_USERDEFINED_KWD_INCLUDE_NEXT) && !defined(TPP_CONFIG_USERDEFINED_KWD_include_next)
DEF_K(include_next) /* #include_next <stdlib.h>. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_INCLUDE_NEXT && !TPP_CONFIG_USERDEFINED_KWD_include_next */
#endif /* TPP_CONFIG_EXTENSION_INCLUDE_NEXT */
#if !defined(TPP_CONFIG_EXTENSION_IMPORT) || TPP_CONFIG_EXTENSION_IMPORT
#if !defined(TPP_CONFIG_USERDEFINED_KWD_IMPORT) && !defined(TPP_CONFIG_USERDEFINED_KWD_import)
DEF_K(import)       /* #import "header.h". */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_IMPORT && !TPP_CONFIG_USERDEFINED_KWD_import */
#endif /* TPP_CONFIG_EXTENSION_IMPORT */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_LINE) && !defined(TPP_CONFIG_USERDEFINED_KWD_line)
DEF_K(line)         /* #line 42 "foo.h". */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_LINE && !TPP_CONFIG_USERDEFINED_KWD_line */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_ERROR) && !defined(TPP_CONFIG_USERDEFINED_KWD_error)
DEF_K(error)        /* #error C5A9. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_ERROR && !TPP_CONFIG_USERDEFINED_KWD_error */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_WARNING) && !defined(TPP_CONFIG_USERDEFINED_KWD_warning)
DEF_K(warning)      /* #warning $H17. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_WARNING && !TPP_CONFIG_USERDEFINED_KWD_warning */
#if !defined(TPP_CONFIG_EXTENSION_IDENT_SCCS) || TPP_CONFIG_EXTENSION_IDENT_SCCS
#if !defined(TPP_CONFIG_USERDEFINED_KWD_IDENT) && !defined(TPP_CONFIG_USERDEFINED_KWD_ident)
DEF_K(ident)        /* #ident "text". */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_IDENT && !TPP_CONFIG_USERDEFINED_KWD_ident */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_SCCS) && !defined(TPP_CONFIG_USERDEFINED_KWD_sccs)
DEF_K(sccs)         /* #sccs "text". */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_SCCS && !TPP_CONFIG_USERDEFINED_KWD_sccs */
#endif /* TPP_CONFIG_EXTENSION_IDENT_SCCS */
#if !defined(TPP_CONFIG_EXTENSION_ASSERTIONS) || TPP_CONFIG_EXTENSION_ASSERTIONS
#if !defined(TPP_CONFIG_USERDEFINED_KWD_ASSERT) && !defined(TPP_CONFIG_USERDEFINED_KWD_assert)
DEF_K(assert)       /* #assert machine(i386). */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_ASSERT && !TPP_CONFIG_USERDEFINED_KWD_assert */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_UNASSERT) && !defined(TPP_CONFIG_USERDEFINED_KWD_unassert)
DEF_K(unassert)     /* #unassert machine. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_UNASSERT && !TPP_CONFIG_USERDEFINED_KWD_unassert */
#endif /* TPP_CONFIG_EXTENSION_ASSERTIONS */

/* Various names for #pragma-directives (TPP supports everything) */
#if !defined(TPP_CONFIG_USERDEFINED_KWD_PRAGMA) && !defined(TPP_CONFIG_USERDEFINED_KWD_pragma)
DEF_K(pragma)   /* #pragma once. */
#endif /* !TPP_CONFIG_USERDEFINED_KWD_PRAGMA && !TPP_CONFIG_USERDEFINED_KWD_pragma */
#if !defined(TPP_CONFIG_USERDEFINED_KWD__PRAGMA) && !defined(TPP_CONFIG_USERDEFINED_KWD__Pragma)
DEF_M(_Pragma)  /* GCC defines _Pragma as a macro... So I'll just do that as well! */
#endif /* !TPP_CONFIG_USERDEFINED_KWD__PRAGMA && !TPP_CONFIG_USERDEFINED_KWD__Pragma */
#if !defined(TPP_CONFIG_EXTENSION_MSVC_PRAGMA) || TPP_CONFIG_EXTENSION_MSVC_PRAGMA
#if !defined(TPP_CONFIG_USERDEFINED_KWD___PRAGMA) && !defined(TPP_CONFIG_USERDEFINED_KWD___pragma)
DEF_M(__pragma) /* I promised this would be defined as a macro... */
#endif /* !TPP_CONFIG_USERDEFINED_KWD__PRAGMA && !TPP_CONFIG_USERDEFINED_KWD___pragma */
#endif /* TPP_CONFIG_EXTENSION_MSVC_PRAGMA */

/* STD-C predefined macros. */
DEF_M(__FILE__)
DEF_M(__LINE__)
DEF_M(__TIME__)
DEF_M(__DATE__)
#if !defined(TPP_CONFIG_EXTENSION_BASEFILE) || TPP_CONFIG_EXTENSION_BASEFILE
DEF_M_IF(__BASE_FILE__,     HAVE_EXTENSION_BASEFILE)
#endif /* TPP_CONFIG_EXTENSION_BASEFILE */
#if !defined(TPP_CONFIG_EXTENSION_INCLUDE_LEVEL) || TPP_CONFIG_EXTENSION_INCLUDE_LEVEL
DEF_M_IF(__INCLUDE_LEVEL__, HAVE_EXTENSION_INCLUDE_LEVEL)
DEF_M_IF(__INCLUDE_DEPTH__, HAVE_EXTENSION_INCLUDE_LEVEL)
#endif /* TPP_CONFIG_EXTENSION_INCLUDE_LEVEL */
#if !defined(TPP_CONFIG_EXTENSION_COUNTER) || TPP_CONFIG_EXTENSION_COUNTER
DEF_M_IF(__COUNTER__,       HAVE_EXTENSION_COUNTER)
#endif /* TPP_CONFIG_EXTENSION_COUNTER */
#if !defined(TPP_CONFIG_EXTENSION_TIMESTAMP) || TPP_CONFIG_EXTENSION_TIMESTAMP
DEF_M_IF(__TIMESTAMP__,     HAVE_EXTENSION_TIMESTAMP)
#endif /* TPP_CONFIG_EXTENSION_TIMESTAMP */
#if !defined(TPP_CONFIG_EXTENSION_COLUMN) || TPP_CONFIG_EXTENSION_COLUMN
DEF_M_IF(__COLUMN__,        HAVE_EXTENSION_COLUMN)
#endif /* TPP_CONFIG_EXTENSION_COLUMN */

#if !defined(TPP_CONFIG_EXTENSION_CLANG_FEATURES) || TPP_CONFIG_EXTENSION_CLANG_FEATURES
DEF_M_IF(__is_identifier,          HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__is_builtin_identifier,  HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__is_deprecated,          HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__is_poisoned,            HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__has_attribute,          HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__has_builtin,            HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__has_tpp_builtin,        HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__has_cpp_attribute,      HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__has_declspec_attribute, HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__has_feature,            HAVE_EXTENSION_CLANG_FEATURES)
DEF_M_IF(__has_extension,          HAVE_EXTENSION_CLANG_FEATURES) /* __has_extension(name) / __has_extension("-fname"): Query the state of an extension (returns 0 for disabled/unknown extensions). */
DEF_M_IF(__has_warning,            HAVE_EXTENSION_CLANG_FEATURES) /* __has_warning("-Wname"): Query the state of a warning (returns 0 for unknown warnings). */
DEF_M_IF(__has_known_extension,    HAVE_EXTENSION_CLANG_FEATURES) /* __has_known_extension("-fname"): Returns 0/1 if `name' is a known extension. */
DEF_M_IF(__has_known_warning,      HAVE_EXTENSION_CLANG_FEATURES) /* __has_known_warning(id) / __has_known_warning("-Wname"): Returns 0/1 if 'id'/`name' is a known warning. */
#endif /* TPP_CONFIG_EXTENSION_CLANG_FEATURES */
#if !defined(TPP_CONFIG_EXTENSION_HAS_INCLUDE) || TPP_CONFIG_EXTENSION_HAS_INCLUDE
DEF_M_IF(__has_include,            HAVE_EXTENSION_HAS_INCLUDE)
DEF_M_IF(__has_include_next,       HAVE_EXTENSION_HAS_INCLUDE)
#endif /* TPP_CONFIG_EXTENSION_HAS_INCLUDE */

/* Helper keywords used to implement variadic macros & extensions.
 * HINT: `__VA_NARGS__' is something new I added when I began reviving TPP:
 *    >> #define _sum0()       0
 *    >> #define _sum1(a)      a
 *    >> #define _sum2(a,b)    a+b
 *    >> #define _sum3(a,b,c)  a+b+c
 *    >> #define sum(args...)  _sum##__VA_NARGS__(args)
 *    >>
 *    >> // Overloading macros by argument count was never this easy!
 *    >> sum()       // []
 *    >> sum( )      // [ ] (or `[]' depending on `TPPLEXER_FLAG_KEEP_ARG_WHITESPACE')
 *    >> sum(42)     // [42]
 *    >> sum(42,16)  // [42][+][16]
 */
#if !defined(TPP_CONFIG_EXTENSION_VA_ARGS) || TPP_CONFIG_EXTENSION_VA_ARGS
KWD(KWD___VA_ARGS__, "__VA_ARGS__")
#endif /* TPP_CONFIG_EXTENSION_VA_ARGS */
#if !defined(TPP_CONFIG_EXTENSION_VA_COMMA) || TPP_CONFIG_EXTENSION_VA_COMMA
KWD(KWD___VA_COMMA__, "__VA_COMMA__")
#endif /* TPP_CONFIG_EXTENSION_VA_COMMA */
#if !defined(TPP_CONFIG_EXTENSION_VA_NARGS) || TPP_CONFIG_EXTENSION_VA_NARGS
KWD(KWD___VA_NARGS__, "__VA_NARGS__")
#endif /* TPP_CONFIG_EXTENSION_VA_NARGS */
#if !defined(TPP_CONFIG_EXTENSION_VA_OPT) || TPP_CONFIG_EXTENSION_VA_OPT
KWD(KWD___VA_OPT__, "__VA_OPT__")
#endif /* TPP_CONFIG_EXTENSION_VA_OPT */

/* TPP extension macros. */
#if !defined(TPP_CONFIG_EXTENSION_TPP_EVAL) || TPP_CONFIG_EXTENSION_TPP_EVAL
DEF_M_IF(__TPP_EVAL,          HAVE_EXTENSION_TPP_EVAL)
#endif /* TPP_CONFIG_EXTENSION_TPP_EVAL */
#if !defined(TPP_CONFIG_EXTENSION_TPP_LOAD_FILE) || TPP_CONFIG_EXTENSION_TPP_LOAD_FILE
DEF_M_IF(__TPP_LOAD_FILE,     HAVE_EXTENSION_TPP_LOAD_FILE)
#endif /* TPP_CONFIG_EXTENSION_TPP_LOAD_FILE */
#if !defined(TPP_CONFIG_EXTENSION_TPP_COUNTER) || TPP_CONFIG_EXTENSION_TPP_COUNTER
DEF_M_IF(__TPP_COUNTER,       HAVE_EXTENSION_TPP_COUNTER)
#endif /* TPP_CONFIG_EXTENSION_TPP_COUNTER */
#if !defined(TPP_CONFIG_EXTENSION_TPP_RANDOM) || TPP_CONFIG_EXTENSION_TPP_RANDOM
DEF_M_IF(__TPP_RANDOM,        HAVE_EXTENSION_TPP_RANDOM)
#endif /* TPP_CONFIG_EXTENSION_TPP_RANDOM */
#if !defined(TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE) || TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE
DEF_M_IF(__TPP_STR_DECOMPILE, HAVE_EXTENSION_TPP_STR_DECOMPILE)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE */
#if !defined(TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR) || TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR
DEF_M_IF(__TPP_STR_AT,        HAVE_EXTENSION_TPP_STR_SUBSTR)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR */
#if !defined(TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR) || TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR
DEF_M_IF(__TPP_STR_SUBSTR,    HAVE_EXTENSION_TPP_STR_SUBSTR)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR */
#if !defined(TPP_CONFIG_EXTENSION_TPP_STR_PACK) || TPP_CONFIG_EXTENSION_TPP_STR_PACK
DEF_M_IF(__TPP_STR_PACK,      HAVE_EXTENSION_TPP_STR_PACK)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_PACK */
#if !defined(TPP_CONFIG_EXTENSION_TPP_STR_SIZE) || TPP_CONFIG_EXTENSION_TPP_STR_SIZE
DEF_M_IF(__TPP_STR_SIZE,      HAVE_EXTENSION_TPP_STR_SIZE)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_SIZE */
#if !defined(TPP_CONFIG_EXTENSION_TPP_UNIQUE) || TPP_CONFIG_EXTENSION_TPP_UNIQUE
DEF_M_IF(__TPP_UNIQUE,        HAVE_EXTENSION_TPP_UNIQUE)
#endif /* TPP_CONFIG_EXTENSION_TPP_UNIQUE */
#if !defined(TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS) || TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS
DEF_M_IF(__TPP_COUNT_TOKENS,  HAVE_EXTENSION_TPP_COUNT_TOKENS)
#endif /* TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS */
#if !defined(TPP_CONFIG_EXTENSION_TPP_IDENTIFIER) || TPP_CONFIG_EXTENSION_TPP_IDENTIFIER
DEF_M_IF(__TPP_IDENTIFIER,    HAVE_EXTENSION_TPP_IDENTIFIER)
#endif /* TPP_CONFIG_EXTENSION_TPP_IDENTIFIER */

#if !defined(TPP_CONFIG_EXTENSION_DATEUTILS) || TPP_CONFIG_EXTENSION_DATEUTILS
DEF_M_IF(__DATE_DAY__,   HAVE_EXTENSION_DATEUTILS)
DEF_M_IF(__DATE_WDAY__,  HAVE_EXTENSION_DATEUTILS)
DEF_M_IF(__DATE_YDAY__,  HAVE_EXTENSION_DATEUTILS)
DEF_M_IF(__DATE_MONTH__, HAVE_EXTENSION_DATEUTILS)
DEF_M_IF(__DATE_YEAR__,  HAVE_EXTENSION_DATEUTILS)
#endif /* TPP_CONFIG_EXTENSION_DATEUTILS */
#if !defined(TPP_CONFIG_EXTENSION_TIMEUTILS) || TPP_CONFIG_EXTENSION_TIMEUTILS
DEF_M_IF(__TIME_SEC__,   HAVE_EXTENSION_TIMEUTILS)
DEF_M_IF(__TIME_MIN__,   HAVE_EXTENSION_TIMEUTILS)
DEF_M_IF(__TIME_HOUR__,  HAVE_EXTENSION_TIMEUTILS)
#endif /* TPP_CONFIG_EXTENSION_TIMEUTILS */


#if !defined(TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA) || TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA
HAS_EXTENSION_IF(tpp_dollar_is_alpha,              HAVE_EXTENSION_DOLLAR_IS_ALPHA)
#endif /* TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA */
#if !defined(TPP_CONFIG_EXTENSION_VA_ARGS) || TPP_CONFIG_EXTENSION_VA_ARGS
HAS_EXTENSION_IF(tpp_va_args,                      HAVE_EXTENSION_VA_ARGS)
#endif /* TPP_CONFIG_EXTENSION_VA_ARGS */
#if !defined(TPP_CONFIG_EXTENSION_GCC_VA_ARGS) || TPP_CONFIG_EXTENSION_GCC_VA_ARGS
HAS_EXTENSION_IF(tpp_named_va_args,                HAVE_EXTENSION_GCC_VA_ARGS)
#endif /* TPP_CONFIG_EXTENSION_GCC_VA_ARGS */
#if !defined(TPP_CONFIG_EXTENSION_VA_COMMA) || TPP_CONFIG_EXTENSION_VA_COMMA
HAS_EXTENSION_IF(tpp_va_comma,                     HAVE_EXTENSION_VA_COMMA)
#endif /* TPP_CONFIG_EXTENSION_VA_COMMA */
#if !defined(TPP_CONFIG_EXTENSION_MSVC_FIXED_INT) || TPP_CONFIG_EXTENSION_MSVC_FIXED_INT
HAS_EXTENSION_IF(tpp_msvc_integer_suffix,          HAVE_EXTENSION_MSVC_FIXED_INT)
#endif /* TPP_CONFIG_EXTENSION_MSVC_FIXED_INT */
#if !defined(TPP_CONFIG_EXTENSION_HASH_AT) || TPP_CONFIG_EXTENSION_HASH_AT
HAS_EXTENSION_IF(tpp_charize_operator,             HAVE_EXTENSION_HASH_AT)
#endif /* TPP_CONFIG_EXTENSION_HASH_AT */
#if !defined(TPP_CONFIG_FEATURE_TRIGRAPHS) || TPP_CONFIG_FEATURE_TRIGRAPHS
HAS_EXTENSION_IF(tpp_trigraphs,                    HAVE_FEATURE_TRIGRAPHS)
#endif /* TPP_CONFIG_FEATURE_TRIGRAPHS */
#if !defined(TPP_CONFIG_FEATURE_DIGRAPHS) || TPP_CONFIG_FEATURE_DIGRAPHS
HAS_EXTENSION_IF(tpp_digraphs,                     HAVE_FEATURE_DIGRAPHS)
#endif /* TPP_CONFIG_FEATURE_DIGRAPHS */
HAS_EXTENSION_IF(tpp_reemit_unknown_pragmas,       (TPPLexer_Current->l_flags & TPPLEXER_FLAG_REEMIT_UNKNOWN_PRAGMA))
HAS_EXTENSION_IF(tpp_pragma_push_macro,            TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_pop_macro,             TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_region,                TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_endregion,             TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_warning,               TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_message,               TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_error,                 TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_once,                  TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_tpp_exec,              TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_deprecated,            TPP_PREPROCESSOR_VERSION >= 200)
HAS_EXTENSION_IF(tpp_pragma_tpp_set_keyword_flags, TPP_PREPROCESSOR_VERSION >= 200)
#if !defined(TPP_CONFIG_EXTENSION_INCLUDE_NEXT) || TPP_CONFIG_EXTENSION_INCLUDE_NEXT
HAS_EXTENSION_IF(tpp_directive_include_next,       HAVE_EXTENSION_INCLUDE_NEXT)
#endif /* TPP_CONFIG_EXTENSION_INCLUDE_NEXT */
#if !defined(TPP_CONFIG_EXTENSION_IMPORT) || TPP_CONFIG_EXTENSION_IMPORT
HAS_EXTENSION_IF(tpp_directive_import,             HAVE_EXTENSION_IMPORT)
#endif /* TPP_CONFIG_EXTENSION_IMPORT */
#if !defined(TPP_CONFIG_EXTENSION_WARNING) || TPP_CONFIG_EXTENSION_WARNING
HAS_EXTENSION_IF(tpp_directive_warning,            HAVE_EXTENSION_WARNING)
#endif /* TPP_CONFIG_EXTENSION_WARNING */
#if !defined(TPP_CONFIG_EXTENSION_LXOR) || TPP_CONFIG_EXTENSION_LXOR
HAS_EXTENSION_IF(tpp_lxor,                         HAVE_EXTENSION_LXOR)
#endif /* TPP_CONFIG_EXTENSION_LXOR */
HAS_EXTENSION_IF(tpp_token_tilde_tilde,            TPPLexer_Current->l_extokens & TPPLEXER_TOKEN_TILDETILDE)
HAS_EXTENSION_IF(tpp_token_pow,                    TPPLexer_Current->l_extokens & TPPLEXER_TOKEN_STARSTAR)
HAS_EXTENSION_IF(tpp_token_lxor,                   TPPLexer_Current->l_extokens & TPPLEXER_TOKEN_ROOFROOF)
HAS_EXTENSION_IF(tpp_token_arrow,                  TPPLexer_Current->l_extokens & TPPLEXER_TOKEN_ARROW)
HAS_EXTENSION_IF(tpp_token_colon_assign,           TPPLexer_Current->l_extokens & TPPLEXER_TOKEN_COLONASSIGN)
HAS_EXTENSION_IF(tpp_token_colon_colon,            TPPLexer_Current->l_extokens & TPPLEXER_TOKEN_COLONCOLON)
#if !defined(TPP_CONFIG_EXTENSION_ALTMAC) || TPP_CONFIG_EXTENSION_ALTMAC
HAS_EXTENSION_IF(tpp_macro_calling_conventions,    HAVE_EXTENSION_ALTMAC)
#endif /* TPP_CONFIG_EXTENSION_ALTMAC */
HAS_EXTENSION_IF(tpp_strict_whitespace,            HAVE_EXTENSION_ARGSPACE)
HAS_EXTENSION_IF(tpp_strict_integer_overflow,      TPP_WSTATE_ISENABLED(TPPLexer_GetWarning(W_INTEGRAL_OVERFLOW)) ||
                                                   TPP_WSTATE_ISENABLED(TPPLexer_GetWarning(W_INTEGRAL_CLAMPED)))
HAS_EXTENSION_IF(tpp_support_ansi_characters,      0) /* TODO: (Re-)add support for this. */
HAS_EXTENSION_IF(tpp_emit_lf_after_directive,      TPPLexer_Current->l_flags & TPPLEXER_FLAG_DIRECTIVE_NOOWN_LF)
#if !defined(TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR) || TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR
HAS_EXTENSION_IF(tpp_if_cond_expression,           HAVE_EXTENSION_IFELSE_IN_EXPR)
#endif /* TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR */
HAS_EXTENSION_IF(tpp_debug,                        TPP_CONFIG_DEBUG)

/* Predefined macros and their values.
 * NOTE: These behave like other predefined macros, allowing
 *       the user to redefine them, but returning when they
 *       #undef their versions again. */
PREDEFINED_MACRO(__TPP_VERSION__, TPP_PP_STR(TPP_PREPROCESSOR_VERSION))

#if !TPP_CONFIG_MINMACRO
/* Pull in GCC-specific definitions. */
#include "tpp-gcc-defs.inl"
#endif /* !TPP_CONFIG_MINMACRO */

#if TPP_CONFIG_GCCFUNC
#if TPP_CONFIG_MINGCCFUNC == 0
/* Builtin functions recognized in expressions. */
#ifdef DEFINE_GLOBAL_SYMBOLS
PRIVATE tint_t tpp_ffs(tint_t i) {
	tint_t result;
	if (!i)
		return 0;
	for (result = 1; !(i & 1); ++result)
		i >>= 1;
	return result;
}

PRIVATE tint_t tpp_clz(tint_t i) {
	tint_t mask   = ~(((tint_t)-1) / 2); /* Only set MSB (e.g.: `0x80000000'). */
	tint_t result = 0;
	while (!(i & mask))
		++result, mask >>= 1;
	return result;
}

PRIVATE tint_t tpp_ctz(tint_t i) {
	tint_t mask   = 1; /* Only set LSB. */
	tint_t result = 0;
	while (!(i & mask))
		++result, mask <<= 1;
	return result;
}

PRIVATE tint_t tpp_clrsb(tint_t i) {
	tint_t mask   = ~(((tint_t)-1) / 2); /* Only set MSB (e.g.: `0x80000000'). */
	tint_t result = 0;
	int msb       = !!(i & mask); /* Extract the MSB bit. */
	for (;;) {
		mask >>= 1;
		if (!!(i & mask) != msb)
			break;
		++result;
	}
	return result;
}

PRIVATE tint_t tpp_popcount(tint_t i) {
	tint_t result = 0;
	while (i) {
		if (i & 1)
			++result;
		i >>= 1;
	}
	return result;
}

PRIVATE tint_t tpp_parity(tint_t i) {
	tint_t result = 0;
	while (i) {
		if (i & 1)
			result ^= 1;
		i >>= 1;
	}
	return result;
}
#endif /* DEFINE_GLOBAL_SYMBOLS */
#endif /* TPP_CONFIG_MINGCCFUNC == 0 */

#if TPP_CONFIG_MINGCCFUNC < 2
/* Special functions that require designated preprocessor support.
 * NOTE: These are not part of the min-gcc-func configuration. */
HAS_BUILTIN_IF(__builtin_constant_p, HAS(EXT_BUILTIN_FUNCTIONS))
HAS_BUILTIN_IF(__builtin_choose_expr, HAS(EXT_BUILTIN_FUNCTIONS))
#if TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS
DEF_K(__builtin_constant_p)
DEF_K(__builtin_choose_expr)
#endif /* TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS */
#endif /* TPP_CONFIG_MINGCCFUNC < 2 */

#if TPP_CONFIG_MINGCCFUNC == 0
/* Define regular builtin functions. */
PREDEFINED_BUILTIN_FUNCTION(__builtin_ffs, 1, { RETURN_INT(tpp_ffs(INT(0) & ((int)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_ffsl, 1, { RETURN_INT(tpp_ffs(INT(0) & ((long)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_ffsll, 1, { RETURN_INT(tpp_ffs(INT(0) & ((llong_t)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_strlen, 1, { RETURN_INT(IS_STRING(0) ? strlen(STRING(0)->s_text) : 0); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_bswap16, 1, { RETURN_INT(bswap_16((uint16_t)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_bswap32, 1, { RETURN_INT(bswap_32((uint32_t)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_bswap64, 1, { RETURN_INT(bswap_64((uint64_t)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_strcmp, 2, { RETURN_INT((IS_STRING(0) && IS_STRING(1)) ? strcmp(STRING(0)->s_text, STRING(1)->s_text) : -1); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_strncmp, 3, { RETURN_INT((IS_STRING(0) && IS_STRING(1)) ? strncmp(STRING(0)->s_text, STRING(1)->s_text, (size_t)INT(2)) : -1); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_strcasecmp, 2, { RETURN_INT((IS_STRING(0) && IS_STRING(1)) ? strcasecmp(STRING(0)->s_text, STRING(1)->s_text) : -1); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_strncasecmp, 3, { RETURN_INT((IS_STRING(0) && IS_STRING(1)) ? strncasecmp(STRING(0)->s_text, STRING(1)->s_text, (size_t)INT(2)) : -1); })
//PREDEFINED_BUILTIN_FUNCTION(__builtin_strchr, 2, { RETURN_INT(IS_STRING(0) ? !!strchr(STRING(0)->s_text, (int)INT(1)) : 0); })
//PREDEFINED_BUILTIN_FUNCTION(__builtin_strrchr, 2, { RETURN_INT(IS_STRING(0) ? !!strchr(STRING(0)->s_text, (int)INT(1)) : 0); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_expect, 2, { RETURN_COPY(A(0)); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_assume_aligned, 2, { RETURN_COPY(A(0)); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_abs, 1, { tint_t x = INT(0)&((int)-1); RETURN_INT(x < 0 ? -x : x); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_labs, 1, { tint_t x = INT(0)&((long)-1); RETURN_INT(x < 0 ? -x : x); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_llabs, 1, { tint_t x = INT(0)&((llong_t)-1); RETURN_INT(x < 0 ? -x : x); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_clz, 1, { RETURN_INT(tpp_clz(INT(0) & ((int)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_clzl, 1, { RETURN_INT(tpp_clz(INT(0) & ((long)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_clzll, 1, { RETURN_INT(tpp_clz(INT(0) & ((llong_t)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_ctz, 1, { RETURN_INT(tpp_ctz(INT(0) & ((int)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_ctzl, 1, { RETURN_INT(tpp_ctz(INT(0) & ((long)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_ctzll, 1, { RETURN_INT(tpp_ctz(INT(0) & ((llong_t)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_clrsb, 1, { RETURN_INT(tpp_clrsb(INT(0) & ((int)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_clrsbl, 1, { RETURN_INT(tpp_clrsb(INT(0) & ((long)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_clrsbll, 1, { RETURN_INT(tpp_clrsb(INT(0) & ((llong_t)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_popcount, 1, { RETURN_INT(tpp_popcount(INT(0) & ((int)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_popcountl, 1, { RETURN_INT(tpp_popcount(INT(0) & ((long)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_popcountll, 1, { RETURN_INT(tpp_popcount(INT(0) & ((llong_t)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_parity, 1, { RETURN_INT(tpp_parity(INT(0) & ((int)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_parityl, 1, { RETURN_INT(tpp_parity(INT(0) & ((long)-1))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_parityll, 1, { RETURN_INT(tpp_parity(INT(0) & ((llong_t)-1))); })
// TODO: __builtin_isfinite
// TODO: __builtin_isinf
// TODO: __builtin_isinff
// TODO: __builtin_isinfl
// TODO: __builtin_isnan
// TODO: __builtin_isnanf
// TODO: __builtin_isnanl
// TODO: __builtin_isnormal
// TODO: __builtin_isgreater
// TODO: __builtin_isgreaterequal
// TODO: __builtin_isless
// TODO: __builtin_islessequal
// TODO: __builtin_islessgreater
// TODO: __builtin_isunordered
// TODO: ... (Floating point math.h functions)
PREDEFINED_BUILTIN_FUNCTION(__builtin_FILE, 0, {
	size_t length;
	char const *filename = TPPLexer_FILE(&length);
	struct TPPString *s  = TPPString_New(filename, length);
	if unlikely(!s)
		TPPLexer_SetErr();
	RETURN_STRING(s);
})
PREDEFINED_BUILTIN_FUNCTION(__builtin_LINE, 0, { RETURN_INT(TPPLexer_LINE()); })
// TODO?: char const *__builtin_FUNCTION(void);
PREDEFINED_BUILTIN_FUNCTION(__builtin_isalnum, 1, { RETURN_INT(isalnum((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isalpha, 1, { RETURN_INT(isalpha((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isascii, 1, { RETURN_INT(isascii((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isblank, 1, { RETURN_INT(isblank((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_iscntrl, 1, { RETURN_INT(iscntrl((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isdigit, 1, { RETURN_INT(isdigit((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isgraph, 1, { RETURN_INT(isgraph((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_islower, 1, { RETURN_INT(islower((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isprint, 1, { RETURN_INT(isprint((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_ispunct, 1, { RETURN_INT(ispunct((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isspace, 1, { RETURN_INT(isspace((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isupper, 1, { RETURN_INT(isupper((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_isxdigit, 1, { RETURN_INT(isxdigit((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_toascii, 1, { RETURN_INT(toascii((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_tolower, 1, { RETURN_INT(tolower((int)INT(0))); })
PREDEFINED_BUILTIN_FUNCTION(__builtin_toupper, 1, { RETURN_INT(toupper((int)INT(0))); })
#endif /* TPP_CONFIG_MINGCCFUNC == 0 */
#endif /* TPP_CONFIG_GCCFUNC */

/* Declare builtin extensions. */
#ifndef TPP_CONFIG_FEATURE_TRIGRAPHS
EXTENSION(EXT_TRIGRAPHS,         "trigraphs",                     TPP_CONFIG_FEATURE_TRIGRAPHS_DEFAULT)
#endif /* TPP_CONFIG_FEATURE_TRIGRAPHS */
#ifndef TPP_CONFIG_FEATURE_DIGRAPHS
EXTENSION(EXT_DIGRAPHS,          "digraphs",                      TPP_CONFIG_FEATURE_DIGRAPHS_DEFAULT)
#endif /* TPP_CONFIG_FEATURE_DIGRAPHS */
#ifndef TPP_CONFIG_EXTENSION_GCC_VA_ARGS
EXTENSION(EXT_GCC_VA_ARGS,       "named-varargs-in-macros",       TPP_CONFIG_EXTENSION_GCC_VA_ARGS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_GCC_VA_ARGS */
#ifndef TPP_CONFIG_EXTENSION_GCC_VA_COMMA
EXTENSION(EXT_GCC_VA_COMMA,      "glue-comma-in-macros",          TPP_CONFIG_EXTENSION_GCC_VA_COMMA_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_GCC_VA_COMMA */
#ifndef TPP_CONFIG_EXTENSION_GCC_IFELSE
EXTENSION(EXT_GCC_IFELSE,        "if-else-optional-true",         TPP_CONFIG_EXTENSION_GCC_IFELSE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_GCC_IFELSE */
#ifndef TPP_CONFIG_EXTENSION_VA_COMMA
EXTENSION(EXT_VA_COMMA,          "va-comma-in-macros",            TPP_CONFIG_EXTENSION_VA_COMMA_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_VA_COMMA */
#ifndef TPP_CONFIG_EXTENSION_VA_OPT
EXTENSION(EXT_VA_OPT,            "va-opt-in-macros",              TPP_CONFIG_EXTENSION_VA_OPT_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_VA_OPT */
#ifndef TPP_CONFIG_EXTENSION_VA_NARGS
EXTENSION(EXT_VA_NARGS,          "va-nargs-in-macros",            TPP_CONFIG_EXTENSION_VA_NARGS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_VA_NARGS */
#ifndef TPP_CONFIG_EXTENSION_VA_ARGS
EXTENSION(EXT_VA_ARGS,           "va-args-in-macros",             TPP_CONFIG_EXTENSION_VA_ARGS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_VA_ARGS */
#ifndef TPP_CONFIG_EXTENSION_STR_E
EXTENSION(EXT_STR_E,             "escape-e-in-strings",           TPP_CONFIG_EXTENSION_STR_E_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_STR_E */
#ifndef TPP_CONFIG_EXTENSION_ALTMAC
EXTENSION(EXT_ALTMAC,            "alternative-macro-parenthesis", TPP_CONFIG_EXTENSION_ALTMAC_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_ALTMAC */
#ifndef TPP_CONFIG_EXTENSION_RECMAC
EXTENSION(EXT_RECMAC,            "macro-recursion",               TPP_CONFIG_EXTENSION_RECMAC_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_RECMAC */
#ifndef TPP_CONFIG_EXTENSION_ARGSPACE
EXTENSION(EXT_ARGSPACE,          "macro-argument-whitespace",     TPP_CONFIG_EXTENSION_ARGSPACE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_ARGSPACE */
#ifndef TPP_CONFIG_EXTENSION_BININTEGRAL
EXTENSION(EXT_BININTEGRAL,       "binary-literals",               TPP_CONFIG_EXTENSION_BININTEGRAL_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_BININTEGRAL */
#ifndef TPP_CONFIG_EXTENSION_MSVC_PRAGMA
EXTENSION(EXT_MSVC_PRAGMA,       "msvc-pragma-support",           TPP_CONFIG_EXTENSION_MSVC_PRAGMA_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_MSVC_PRAGMA */
#ifndef TPP_CONFIG_EXTENSION_STRINGOPS
EXTENSION(EXT_STRINGOPS,         "strings-in-expressions",        TPP_CONFIG_EXTENSION_STRINGOPS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_STRINGOPS */
#ifndef TPP_CONFIG_EXTENSION_HASH_AT
EXTENSION(EXT_HASH_AT,           "charize-macro-argument",        TPP_CONFIG_EXTENSION_HASH_AT_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_HASH_AT */
#ifndef TPP_CONFIG_EXTENSION_HASH_XCLAIM
EXTENSION(EXT_HASH_XCLAIM,       "dont-expand-macro-argument",    TPP_CONFIG_EXTENSION_HASH_XCLAIM_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_HASH_XCLAIM */
#ifndef TPP_CONFIG_EXTENSION_WARNING
EXTENSION(EXT_WARNING,           "warning-directives",            TPP_CONFIG_EXTENSION_WARNING_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_WARNING */
#ifndef TPP_CONFIG_EXTENSION_SHEBANG
EXTENSION(EXT_SHEBANG,           "shebang-directives",            TPP_CONFIG_EXTENSION_SHEBANG_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_SHEBANG */
#ifndef TPP_CONFIG_EXTENSION_INCLUDE_NEXT
EXTENSION(EXT_INCLUDE_NEXT,      "include-next-directives",       TPP_CONFIG_EXTENSION_INCLUDE_NEXT_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_INCLUDE_NEXT */
#ifndef TPP_CONFIG_EXTENSION_IMPORT
EXTENSION(EXT_IMPORT,            "import-directives",             TPP_CONFIG_EXTENSION_IMPORT_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_IMPORT */
#ifndef TPP_CONFIG_EXTENSION_IDENT_SCCS
EXTENSION(EXT_IDENT_SCCS,        "ident-directives",              TPP_CONFIG_EXTENSION_IDENT_SCCS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_IDENT_SCCS */
#ifndef TPP_CONFIG_EXTENSION_BASEFILE
EXTENSION(EXT_BASEFILE,          "basefile-macro",                TPP_CONFIG_EXTENSION_BASEFILE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_BASEFILE */
#ifndef TPP_CONFIG_EXTENSION_INCLUDE_LEVEL
EXTENSION(EXT_INCLUDE_LEVEL,     "include-level-macro",           TPP_CONFIG_EXTENSION_INCLUDE_LEVEL_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_INCLUDE_LEVEL */
#ifndef TPP_CONFIG_EXTENSION_COUNTER
EXTENSION(EXT_COUNTER,           "counter-macro",                 TPP_CONFIG_EXTENSION_COUNTER_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_COUNTER */
#ifndef TPP_CONFIG_EXTENSION_CLANG_FEATURES
EXTENSION(EXT_CLANG_FEATURES,    "has-feature-macros",            TPP_CONFIG_EXTENSION_CLANG_FEATURES_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_CLANG_FEATURES */
#ifndef TPP_CONFIG_EXTENSION_HAS_INCLUDE
EXTENSION(EXT_HAS_INCLUDE,       "has-include-macros",            TPP_CONFIG_EXTENSION_HAS_INCLUDE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_HAS_INCLUDE */
#ifndef TPP_CONFIG_EXTENSION_LXOR
EXTENSION(EXT_LXOR,              "logical-xor-in-expressions",    TPP_CONFIG_EXTENSION_LXOR_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_LXOR */
#ifndef TPP_CONFIG_EXTENSION_MULTICHAR_CONST
EXTENSION(EXT_MULTICHAR_CONST,   "multichar-constants",           TPP_CONFIG_EXTENSION_MULTICHAR_CONST_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_MULTICHAR_CONST */
#ifndef TPP_CONFIG_EXTENSION_DATEUTILS
EXTENSION(EXT_DATEUTILS,         "numeric-date-macros",           TPP_CONFIG_EXTENSION_MULTICHAR_CONST_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_DATEUTILS */
#ifndef TPP_CONFIG_EXTENSION_TIMEUTILS
EXTENSION(EXT_TIMEUTILS,         "numeric-time-macros",           TPP_CONFIG_EXTENSION_TIMEUTILS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TIMEUTILS */
#ifndef TPP_CONFIG_EXTENSION_TIMESTAMP
EXTENSION(EXT_TIMESTAMP,         "timestamp-macro",               TPP_CONFIG_EXTENSION_TIMESTAMP_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TIMESTAMP */
#ifndef TPP_CONFIG_EXTENSION_COLUMN
EXTENSION(EXT_COLUMN,            "column-macro",                  TPP_CONFIG_EXTENSION_COLUMN_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_COLUMN */
#ifndef TPP_CONFIG_EXTENSION_TPP_EVAL
EXTENSION(EXT_TPP_EVAL,          "tpp-eval-macro",                TPP_CONFIG_EXTENSION_TPP_EVAL_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_EVAL */
#ifndef TPP_CONFIG_EXTENSION_TPP_UNIQUE
EXTENSION(EXT_TPP_UNIQUE,        "tpp-unique-macro",              TPP_CONFIG_EXTENSION_TPP_UNIQUE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_UNIQUE */
#ifndef TPP_CONFIG_EXTENSION_TPP_LOAD_FILE
EXTENSION(EXT_TPP_LOAD_FILE,     "tpp-load-file-macro",           TPP_CONFIG_EXTENSION_TPP_LOAD_FILE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_LOAD_FILE */
#ifndef TPP_CONFIG_EXTENSION_TPP_COUNTER
EXTENSION(EXT_TPP_COUNTER,       "tpp-counter-macro",             TPP_CONFIG_EXTENSION_TPP_COUNTER_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_COUNTER */
#ifndef TPP_CONFIG_EXTENSION_TPP_RANDOM
EXTENSION(EXT_TPP_RANDOM,        "tpp-random-macro",              TPP_CONFIG_EXTENSION_TPP_RANDOM_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_RANDOM */
#ifndef TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE
EXTENSION(EXT_TPP_STR_DECOMPILE, "tpp-str-decompile-macro",       TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE */
#ifndef TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR
EXTENSION(EXT_TPP_STR_SUBSTR,    "tpp-str-substr-macro",          TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR */
#ifndef TPP_CONFIG_EXTENSION_TPP_STR_PACK
EXTENSION(EXT_TPP_STR_PACK,      "tpp-str-pack-macro",            TPP_CONFIG_EXTENSION_TPP_STR_PACK_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_PACK */
#ifndef TPP_CONFIG_EXTENSION_TPP_STR_SIZE
EXTENSION(EXT_TPP_STR_SIZE,      "tpp-str-size-macro",            TPP_CONFIG_EXTENSION_TPP_STR_SIZE_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_STR_SIZE */
#ifndef TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS
EXTENSION(EXT_TPP_COUNT_TOKENS,  "tpp-count-tokens-macro",        TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS */
#ifndef TPP_CONFIG_EXTENSION_TPP_IDENTIFIER
EXTENSION(EXT_TPP_IDENTIFIER,    "tpp-identifier-macro",          TPP_CONFIG_EXTENSION_TPP_IDENTIFIER_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TPP_IDENTIFIER */
#ifndef TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA
EXTENSION(EXT_DOLLAR_IS_ALPHA,   "dollars-in-identifiers",        TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA */
#ifndef TPP_CONFIG_EXTENSION_ASSERTIONS
EXTENSION(EXT_ASSERTIONS,        "assertions",                    TPP_CONFIG_EXTENSION_ASSERTIONS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_ASSERTIONS */
#ifndef TPP_CONFIG_EXTENSION_CANONICAL_HEADERS
EXTENSION(EXT_CANONICAL_HEADERS, "canonical-system-headers",      TPP_CONFIG_EXTENSION_CANONICAL_HEADERS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_CANONICAL_HEADERS */
#ifndef TPP_CONFIG_EXTENSION_EXT_ARE_FEATURES
EXTENSION(EXT_EXT_ARE_FEATURES,  "extensions-are-features",       TPP_CONFIG_EXTENSION_EXT_ARE_FEATURES_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_EXT_ARE_FEATURES */
#ifndef TPP_CONFIG_EXTENSION_MSVC_FIXED_INT
EXTENSION(EXT_MSVC_FIXED_INT,    "fixed-length-integrals",        TPP_CONFIG_EXTENSION_MSVC_FIXED_INT_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_MSVC_FIXED_INT */
#ifndef TPP_CONFIG_EXTENSION_NO_EXPAND_DEFINED
EXTENSION(EXT_NO_EXPAND_DEFINED, "dont-expand-defined",           TPP_CONFIG_EXTENSION_NO_EXPAND_DEFINED_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_NO_EXPAND_DEFINED */
#ifndef TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR
EXTENSION(EXT_IFELSE_IN_EXPR,    "ifelse-in-expressions",         TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR */
#ifndef TPP_CONFIG_EXTENSION_EXTENDED_IDENTS
EXTENSION(EXT_EXTENDED_IDENTS,   "extended-identifiers",          TPP_CONFIG_EXTENSION_EXTENDED_IDENTS_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_EXTENDED_IDENTS */
#ifndef TPP_CONFIG_EXTENSION_TRADITIONAL_MACRO /* Traditional macro expansion rules. */
EXTENSION(EXT_TRADITIONAL_MACRO, "traditional-macro",             TPP_CONFIG_EXTENSION_TRADITIONAL_MACRO_DEFAULT)
#endif /* TPP_CONFIG_EXTENSION_TRADITIONAL_MACRO */
#if TPP_CONFIG_MINGCCFUNC < 2
#if TPP_CONFIG_GCCFUNC
EXTENSION(EXT_BUILTIN_FUNCTIONS, "builtins-in-expressions",       1)
#endif /* TPP_CONFIG_GCCFUNC */
#if !TPP_CONFIG_MINMACRO
EXTENSION(EXT_CPU_MACROS,        "define-cpu-macros",             1)
EXTENSION(EXT_SYSTEM_MACROS,     "define-system-macros",          1)
EXTENSION(EXT_UTILITY_MACROS,    "define-utility-macros",         1)
#endif /* !TPP_CONFIG_MINMACRO */
#endif /* TPP_CONFIG_MINGCCFUNC < 2 */


/* Declare builtin warning groups. */
WGROUP(WG_COMMENT,              "comment",              WSTATE_FATAL)
WGROUP(WG_COMMENTS,             "comments",             WSTATE_FATAL)
WGROUP(WG_MACROS,               "macros",               WSTATE_FATAL)
WGROUP(WG_SYNTAX,               "syntax",               WSTATE_FATAL)
WGROUP(WG_USAGE,                "usage",                WSTATE_FATAL)
WGROUP(WG_VALUE,                "value",                WSTATE_FATAL)
WGROUP(WG_BOOLVALUE,            "boolean-value",        WSTATE_FATAL)
WGROUP(WG_USER,                 "user",                 WSTATE_FATAL)
WGROUP(WG_ENVIRON,              "environ",              WSTATE_FATAL)
WGROUP(WG_LIMIT,                "limit",                WSTATE_FATAL)
WGROUP(WG_UNDEF,                "undef",                WSTATE_WARN)
WGROUP(WG_TRIGRAPHS,            "trigraphs",            WSTATE_WARN)
WGROUP(WG_EXPANSION_TO_DEFINED, "expansion-to-defined", WSTATE_WARN)
WGROUP(WG_QUALITY,              "quality",              WSTATE_FATAL)
WGROUP(WG_DEPRECATED,           "deprecated",           WSTATE_FATAL)
WGROUP(WG_DEPENDENCY,           "dependency",           WSTATE_WARN)

/* NOTE: These warnings are arranged to mirror those from the old TPP. */
/* 0*/ DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_DEFINE, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after #define, but got " TOK_S, TOK_A))                         /* OLD(TPPWarn_ExpectedKeywordAfterDefine). */
/* 1*/ DEF_WARNING(W_REDEFINING_BUILTIN_KEYWORD, (WG_MACROS), WSTATE_DISABLED, WARNF("Defining macro for builtin keyword " Q("%s"), KWDNAME()))                        /* [struct TPPKeyword *] OLD(TPPWarn_RedefiningBuiltinKeyword). */
/* 2*/ DEF_WARNING(W_UNKNOWN_PREPROCESSOR_DIRECTIVE, (WG_SYNTAX), WSTATE_ERROR, WARNF("Unknown preprocessor directive " TOK_S, TOK_A))                                 /* OLD(TPPWarn_UnknownPreprocessorDirective). */
/* 3*/ DEF_WARNING(W_STARSLASH_OUTSIDE_OF_COMMENT, (WG_COMMENT, WG_COMMENTS), WSTATE_WARN, WARNF(Q("*" "/") " outside of comment"))                                    /* [char *] OLD(TPPWarn_StarSlashOutsideOfComment). */
/* 4*/ DEF_WARNING(W_ERROR, (WG_USER), WSTATE_ERROR, { char *temp = ARG(char *); WARNF("ERROR : %.*s", (int)ARG(size_t), temp); })                                     /* [char const *,size_t] OLD(TPPWarn_DirectiveError). */
/* 5*/ DEF_WARNING(W_WARNING, (WG_USER), WSTATE_WARN, { char *temp = ARG(char *); WARNF("WARNING : %.*s", (int)ARG(size_t), temp); })                                  /* [char const *,size_t] OLD(TPPWarn_DirectiveWarning). */
/* 6*/ DEF_WARNING(W_EXPECTED_KWDLPAR_AFTER_DEFINED, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword or " Q("(") " after " Q("defined") ", but got " TOK_S, TOK_A)) /* OLD(TPPWarn_ExpectedKeywordOrLParenAfterDefined). */
/* 7*/ DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_DEFINED, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after " Q("defined") ", but got " TOK_S, TOK_A))               /* OLD(TPPWarn_ExpectedKeywordAfterDefined). */
/* 8*/ DEF_WARNING(W_EXPECTED_RPAREN_AFTER_DEFINED, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(")") " after " Q("defined") ", but got " TOK_S, TOK_A))             /* OLD(TPPWarn_ExpectedRParenAfterDefined). */
/* 9*/ DEF_WARNING(W_EXPECTED_MACRO_ARGUMENT_NAME, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected argument name, but got " TOK_S, TOK_A))                                  /* OLD(TPPWarn_ExpectedKeywordForMacroArgument). */
/*10*/ DEF_WARNING(W_EXPECTED_ARGEND_AFTER_VARARGS, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected end of argument list after arg-args, but got " TOK_S, TOK_A))           /* OLD(TPPWarn_ExpectedRParenAfterVaArgs). */
/*11*/ DEF_WARNING(W_EXPECTED_COMMA_OR_ARGEND, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(",") " or end of argument list, but got " TOK_S, TOK_A))                 /* OLD(TPPWarn_ExpectedCommaOrRParenForMacroFuncDecl). */
/*12*/ DEF_WARNING(W_TOO_MANY_MACRO_ARGUMENTS, (WG_MACROS), WSTATE_ERROR, WARNF("Too many arguments for " Q("%s"), FILENAME()))                                        /* [struct TPPFile *] OLD( TPPWarn_InvalidMacroArgCount). */
/*13*/ DEF_WARNING(W_EOF_IN_MACRO_ARGUMENT_LIST, (WG_SYNTAX), WSTATE_ERROR, WARNF("EOF in macro argument list"))                                                       /* OLD(TPPWarn_UnexpectedEOFInMacroArgList). */
/*14*/ DEF_WARNING(W_EXPECTED_INCLUDE_STRING, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected #include-string, but got " TOK_S, TOK_A))                                     /* OLD(TPPWarn_ExpectedIncludeString). */
/*15*/ DEF_WARNING(W_FILE_NOT_FOUND, (WG_ENVIRON), WSTATE_ERROR, { char *temp = ARG(char *); WARNF("File not found: " Q("%.*s"), (int)ARG(size_t), temp); })           /* [char const *,size_t] OLD(TPPWarn_IncludeFileNotFound). */
/*16*/ WARNING(W_UNUSED_16, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_SysIncludeFileNotFound). */
/*17*/ WARNING(W_UNUSED_17, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedLParenAfterTPPCounter). */
/*18*/ WARNING(W_UNUSED_18, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedKeywordAfterTPPCounter). */
/*19*/ WARNING(W_UNUSED_19, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedRParenAfterTPPCounter). */
/*20*/ WARNING(W_UNUSED_20, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedLParenAfterTPPRandom). */
/*21*/ WARNING(W_UNUSED_21, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedIntegerAfterTPPRandom1). */
/*22*/ WARNING(W_UNUSED_22, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedCommaAfterTPPRandom). */
/*23*/ WARNING(W_UNUSED_23, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedIntegerAfterTPPRandom2). */
/*24*/ WARNING(W_UNUSED_24, (WG_VALUE), WSTATE_DISABLED)       /* OLD(TPPWarn_ExpectedRParenAfterTPPRandom). */
/*25*/ DEF_WARNING(W_REDEFINING_MACRO, (WG_MACROS), WSTATE_WARN, { /* [struct TPPKeyword *] OLD(TPPWarn_MacroAlreadyDefined). */
	struct TPPFile *textfile;
	struct TPPKeyword *redef_kwd = ARG(struct TPPKeyword *);
	assert(redef_kwd->k_macro);
	assert(redef_kwd->k_macro->f_kind == TPPFILE_KIND_MACRO);
	assert((redef_kwd->k_macro->f_macro.m_flags & TPP_MACROFILE_KIND) != TPP_MACROFILE_KIND_EXPANDED);
	WARNF("Redefining macro " Q("%s") "\n", redef_kwd->k_name);
	textfile = redef_kwd->k_macro->f_macro.m_deffile;
	if (textfile) {
		assert(textfile->f_kind == TPPFILE_KIND_TEXT);
		WARNF(TPPLexer_Current->l_flags & TPPLEXER_FLAG_MSVC_MESSAGEFORMAT
		      ? "%s(%d,%d) : "
		      : "%s:%d:%d: ",
		      textfile->f_textfile.f_usedname
		      ? textfile->f_textfile.f_usedname->s_text
		      : textfile->f_name,
		      (int)(redef_kwd->k_macro->f_macro.m_defloc.lc_line + 1),
		      (int)(redef_kwd->k_macro->f_macro.m_defloc.lc_col + 1));
		WARNF("See reference to previous definition");
	}
})
/*26*/ DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_UNDEF, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after " Q("#undef") ", but got " TOK_S, TOK_A)) /* OLD(TPPWarn_ExpectedKeywordAfterUndef). */
/*27*/ DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_IFDEF, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after " Q("#ifdef") ", but got " TOK_S, TOK_A)) /* OLD(TPPWarn_ExpectedKeywordAfterIfdef). */
/*28*/ DEF_WARNING(W_ELSE_WITHOUT_IF, (WG_SYNTAX), WSTATE_ERROR, WARNF("#else without #if"))                                                          /* OLD(TPPWarn_ElseWithoutIfdef). */
/*29*/ DEF_WARNING(W_ELSE_AFTER_ELSE, (WG_SYNTAX), WSTATE_ERROR, {
	/* [struct TPPIfdefStackSlot *] OLD(TPPWarn_ElseAfterElse). */
	struct TPPIfdefStackSlot *ifdef_slot;
	WARNF("#else after #else\n");
	ifdef_slot = ARG(struct TPPIfdefStackSlot *);
	WARNF(TPPLexer_Current->l_flags & TPPLEXER_FLAG_MSVC_MESSAGEFORMAT
	      ? "%s(%d) : "
	      : "%s:%d: ",
	      ifdef_slot->iss_file->f_name,
	      (int)(ifdef_slot->iss_line + 1));
	WARNF("See reference to previous #else");
})
/*30*/ DEF_WARNING(W_ELIF_WITHOUT_IF, (WG_SYNTAX), WSTATE_ERROR, WARNF("#elif without #if")) /* OLD(TPPWarn_ElifWithoutIfdef). */
/*31*/ DEF_WARNING(W_ELIF_AFTER_ELSE, (WG_SYNTAX), WSTATE_ERROR, {
	/* [struct TPPIfdefStackSlot *] OLD(TPPWarn_ElifAfterElse). */
	struct TPPIfdefStackSlot *ifdef_slot;
	WARNF("#elif after #else\n");
	ifdef_slot = ARG(struct TPPIfdefStackSlot *);
	WARNF(TPPLexer_Current->l_flags & TPPLEXER_FLAG_MSVC_MESSAGEFORMAT
	      ? "%s(%d) : "
	      : "%s:%d: ",
	      ifdef_slot->iss_file->f_name,
	      (int)(ifdef_slot->iss_line + 1));
	WARNF("See reference to #else");
})
/*32*/ DEF_WARNING(W_IF_WITHOUT_ENDIF, (WG_SYNTAX), WSTATE_ERROR, WARNF("#if without #endif"))                                                             /* [struct TPPIfdefStackSlot *] OLD(TPPWarn_IfdefWithoutEndif). */
/*33*/ DEF_WARNING(W_ENDIF_WITHOUT_IF, (WG_SYNTAX), WSTATE_ERROR, WARNF("#endif without #if"))                                                             /* OLD(TPPWarn_EndifWithoutIfdef). */
/*34*/ WARNING(W_UNUSED_34, (WG_VALUE), WSTATE_DISABLED)                                                                                                   /* OLD(TPPWarn_ExpectedIntAfterLine). */
/*35*/ DEF_WARNING(W_EXPECTED_STRING_AFTER_LINE, (WG_SYNTAX, WG_VALUE), WSTATE_ERROR, WARNF("Expected string after #line, but got " Q("%s"), CONST_STR())) /* [struct TPPConst *] OLD(TPPWarn_ExpectedStringAfterLine). */
/*36*/ DEF_WARNING(W_MACRO_NOT_DEFINED, (WG_MACROS), WSTATE_DISABLED, WARNF("Macro " Q("%s") " is not defined", KWDNAME()))                                /* [struct TPPKeyword *] OLD(TPPWarn_MacroDoesntExist). */
/*37*/ DEF_WARNING(W_CANT_UNDEF_BUILTIN_MACRO, (WG_MACROS), WSTATE_WARN, WARNF("Cannot #undef builtin macro " Q("%s"), KWDNAME()))                         /* [struct TPPKeyword *] OLD(TPPWarn_CantUndefBuiltinMacro). */
/*38*/ WARNING(W_UNUSED_38, (WG_VALUE), WSTATE_DISABLED)                                                                                                   /* OLD(TPPWarn_ExpectedLParenAfterHasInclude). */
/*39*/ WARNING(W_UNUSED_39, (WG_VALUE), WSTATE_DISABLED)                                                                                                   /* OLD(TPPWarn_ExpectedRParenAfterHasInclude). */
/*40*/ DEF_WARNING(W_EXPECTED_COLON_AFTER_QUESTION, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(":") " after " Q("?")))                                /* OLD(TPPWarn_ExpectedColonAfterQuestion). */
#ifndef W_EXPECTED_COLLON_AFTER_QUESTION /* Deprecated typo */
#define W_EXPECTED_COLLON_AFTER_QUESTION W_EXPECTED_COLON_AFTER_QUESTION
#endif /* !W_EXPECTED_COLLON_AFTER_QUESTION */
#if 1
/*41*/ DEF_WARNING(W_INVALID_INTEGER, (WG_SYNTAX), WSTATE_ERROR, WARNF("Invalid integer " TOK_S, TOK_A)) /* OLD(TPPWarn_ExpectedInteger). */
#else
/*41*/ DEF_WARNING(W_INVALID_INTEGER, (WG_SYNTAX), WSTATE_ERROR, { char *temp = ARG(char *); WARNF("Invalid integer suffix " Q("%.*s"), (int)ARG(size_t), temp); }) /* [char const *,size_t] OLD(TPPWarn_ExpectedInteger). */
#endif
/*42*/ DEF_WARNING(W_EXPECTED_RPAREN_IN_EXPRESSION, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(")") " in preprocessor expression, but got " TOK_S, TOK_A))                  /* OLD(TPPWarn_ExpectedRparenAfterLParen). */
/*43*/ DEF_WARNING(W_UNKNOWN_TOKEN_IN_EXPR_IS_ZERO, (WG_UNDEF, WG_SYNTAX), WSTATE_WARN, WARNF("Unrecognized token " TOK_S " is replaced with " Q("0") " in expression", TOK_A)) /* OLD(TPPWarn_UnexpectedTokenInConstExpr). */
/*44*/ WARNING(W_UNUSED_44, (WG_VALUE), WSTATE_DISABLED)                                                                                                                        /* OLD(TPPWarn_ExpectedLParenAfterTPPEval). */
/*45*/ WARNING(W_UNUSED_45, (WG_VALUE), WSTATE_DISABLED)                                                                                                                        /* OLD(TPPWarn_ExpectedRParenAfterTPPEval). */
/*46*/ WARNING(W_UNUSED_46, (WG_VALUE), WSTATE_DISABLED)                                                                                                                        /* OLD(TPPWarn_ExpectedLParenAfterPragma). */
/*47*/ DEF_WARNING(W_EXPECTED_STRING_AFTER_PRAGMA, (WG_SYNTAX, WG_VALUE), WSTATE_ERROR, WARNF("Expected string after _Pragma, but got " Q("%s"), CONST_STR()))                  /* [struct TPPConst *] OLD(TPPWarn_ExpectedStringAfterPragma). */
/*48*/ WARNING(W_UNUSED_48, (WG_VALUE), WSTATE_DISABLED)                                                                                                                        /* OLD(TPPWarn_ExpectedRParenAfterPragma). */
/*49*/ WARNING(W_UNUSED_49, (WG_VALUE), WSTATE_DISABLED)                                                                                                                        /* OLD(TPPWarn_UnexpectedToken). */
/*50*/ DEF_WARNING(W_INVALID_WARNING, (WG_VALUE), WSTATE_ERROR, {
	/* [struct TPPConst *] OLD(TPPWarn_InvalidWarning). */
	struct TPPConst *c = ARG(struct TPPConst *);
	if (c->c_kind == TPP_CONST_STRING) {
		char const *wname = c->c_data.c_string->s_text;
		if (*wname == '-')
			++wname;
		if (*wname == 'W')
			++wname;
		if (!memcmp(wname, "no-", 3))
			wname += 3;
		WARNF("Invalid warning " Q("%s") " (Did you mean " Q("%s") ")", wname, find_most_likely_warning(wname));
	} else if (c->c_kind == TPP_CONST_FLOAT) {
		WARNF("Invalid warning " Q("%f"), (double)c->c_data.c_float);
	} else if (c->c_kind == TPP_CONST_INTEGRAL) {
		WARNF("Invalid warning " Q("%ld"), (long)c->c_data.c_int);
	} else {
		WARNF("Invalid warning");
	}
})
/*51*/ DEF_WARNING(W_CANT_POP_WARNINGS, (WG_VALUE), WSTATE_ERROR, WARNF("Can't pop warnings"))                                                                       /* OLD(TPPWarn_CantPopWarnings). */
/*52*/ WARNING(W_UNUSED_52, (WG_VALUE), WSTATE_DISABLED)                                                                                                             /* OLD(TPPWarn_TPPLoadFileNotFound). */
/*53*/ DEF_WARNING(W_EXPECTED_STRING_AFTER_PUSHMACRO, (WG_SYNTAX, WG_VALUE), WSTATE_ERROR, WARNF("Expected string after push_macro, but got " Q("%s"), CONST_STR())) /* [struct TPPConst *] OLD(TPPWarn_ExpectedStringAfterPushMacro). */
/*54*/ WARNING(W_UNUSED_54, (WG_VALUE), WSTATE_DISABLED)                                                                                                             /* OLD(TPPWarn_CantPopMacro). */
/*55*/ WARNING(W_UNUSED_55, (WG_VALUE), WSTATE_DISABLED)                                                                                                             /* OLD(TPPWarn_ExpectedLParenAfterTPPStrDecompile). */
#if !defined(TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE) || TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE
/*56*/ DEF_WARNING(W_EXPECTED_STRING_AFTER_TPP_STRD, (WG_SYNTAX, WG_VALUE), WSTATE_ERROR, WARNF("Expected string after __TPP_STR_DECOMPILE, but got " Q("%s"), CONST_STR())) /* [struct TPPConst *] OLD(TPPWarn_ExpectedStringAfterTPPStrDecompile). */
#else /* TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE */
/*56*/ WARNING(W_UNUSED_56, (WG_SYNTAX), WSTATE_ERROR)
#endif /* !TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE */
/*57*/ WARNING(W_UNUSED_57, (WG_VALUE), WSTATE_DISABLED)                                                                      /* OLD(TPPWarn_ExpectedRParenAfterTPPStrDecompile). */
/*58*/ DEF_WARNING(W_EXPECTED_LPAREN, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q("(") ", but got " TOK_S, TOK_A))         /* OLD(TPPWarn_ExpectedLParen). */
/*59*/ DEF_WARNING(W_EXPECTED_RPAREN, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(")") ", but got " TOK_S, TOK_A))         /* OLD(TPPWarn_ExpectedRParen). */
/*60*/ DEF_WARNING(W_DEPRECATED_IDENTIFIER, (WG_DEPRECATED), WSTATE_WARN, WARNF("Deprecated identifier " Q("%s"), KWDNAME())) /* [struct TPPKeyword *] OLD(TPPWarn_DeprecatedKeyword). */
/*61*/ DEF_WARNING(W_NONPARTABLE_FILENAME_CASING, (WG_ENVIRON), WSTATE_WARN, {
	/* [char const *,char const *,size_t,char const *]. (path,wrong_begin,wrong_size,corrent_begin)
	 *  OLD(TPPWarn_InvalidPathCasing). */
	char *temp;
	char *temp2;
	size_t temp3;
	temp  = ARG(char *);
	temp2 = ARG(char *);
	temp3 = ARG(size_t);
	WARNF("Non-portable casing in " Q("%s") ": " Q("%.*s") " should be " Q("%s") " instead",
	      temp, (int)temp3, temp2, ARG(char *));
})
/*62*/ DEF_WARNING(W_DIVIDE_BY_ZERO, (WG_VALUE), WSTATE_ERROR, WARNF("Divide by ZERO")) /* OLD(TPPWarn_DivideByZero|TPPWarn_ModuloByZero). */
/*63*/ DEF_WARNING(W_ARGUMENT_NAMED_ALREADY_TAKEN, (WG_MACROS, WG_SYNTAX), WSTATE_ERROR, {
	/* [tok_t] OLD(TPPWarn_ReusedMacroParameter). */
	struct TPPKeyword *name_kwd;
	name_kwd = TPPLexer_LookupKeywordID(ARG(tok_t));
	WARNF("Argument name " Q("%s") " is already in use",
	      name_kwd ? name_kwd->k_name : (char *)"??" "?");
})
#if !defined(TPP_CONFIG_EXTENSION_VA_ARGS) || TPP_CONFIG_EXTENSION_VA_ARGS ||   \
    !defined(TPP_CONFIG_EXTENSION_VA_COMMA) || TPP_CONFIG_EXTENSION_VA_COMMA || \
    !defined(TPP_CONFIG_EXTENSION_VA_NARGS) || TPP_CONFIG_EXTENSION_VA_NARGS || \
    !defined(TPP_CONFIG_EXTENSION_VA_OPT) || TPP_CONFIG_EXTENSION_VA_OPT
/*64*/ DEF_WARNING(W_SPECIAL_ARGUMENT_NAME, (WG_MACROS), WSTATE_WARN, WARNF("Special keyword " Q("%s") " used as argument name", KWDNAME())) /* [struct TPPKeyword *] OLD(TPPWarn_VaArgsUsedAsMacroParameter). */
#else /* TPP_CONFIG_EXTENSION_VA_ARGS || TPP_CONFIG_EXTENSION_VA_COMMA || TPP_CONFIG_EXTENSION_VA_NARGS || TPP_CONFIG_EXTENSION_VA_OPT */
/*64*/ WARNING(W_UNUSED_64, (WG_MACROS), WSTATE_WARN) /* [struct TPPKeyword *] OLD(TPPWarn_VaArgsUsedAsMacroParameter). */
#endif /* !TPP_CONFIG_EXTENSION_VA_ARGS && !TPP_CONFIG_EXTENSION_VA_COMMA && !TPP_CONFIG_EXTENSION_VA_NARGS && !TPP_CONFIG_EXTENSION_VA_OPT */
/*65*/ WARNING(W_UNUSED_65, (WG_VALUE), WSTATE_WARN)                                  /* OLD(TPPWarn_VaCommaUsedAsMacroParameter). */
/*66*/ WARNING(W_UNUSED_66, (WG_VALUE), WSTATE_WARN)                                  /* OLD(TPPWarn_Unexpected). */
/*67*/ WARNING(W_UNUSED_67, (WG_VALUE), WSTATE_WARN)                                  /* OLD(TPPWarn_VaArgsMustBeLastParameter). */
/*68*/ WARNING(W_EXPECTED_BOOL, (WG_BOOLVALUE, WG_VALUE), WSTATE_DISABLED)            /* [struct TPPConst *] OLD(TPPWarn_ExpectedBoolExpression). */
/*69*/ WARNING(W_EXPECTED_BOOL_UNARY, (WG_BOOLVALUE, WG_VALUE), WSTATE_DISABLED)      /* [struct TPPConst *] OLD(TPPWarn_ExpectedBoolExpressionNot). */
/*70*/ WARNING(W_EXPECTED_BOOL_BINARY_LHS, (WG_BOOLVALUE, WG_VALUE), WSTATE_DISABLED) /* [struct TPPConst *] OLD(TPPWarn_ExpectedBoolExpressionLhsOP). */
/*71*/ WARNING(W_EXPECTED_BOOL_BINARY_RHS, (WG_BOOLVALUE, WG_VALUE), WSTATE_DISABLED) /* [struct TPPConst *] OLD(TPPWarn_ExpectedBoolExpressionRhsOP). */
/*72*/ WARNING(W_UNUSED_72, (WG_VALUE), WSTATE_WARN)                                  /* OLD(TPPWarn_ExpectedKeyword). */
#ifdef DECLARE_WARNING_MESSAGES
{
	char const *use; /* Warn about non-boolean integral. */
	if (FALSE) {
case W_EXPECTED_BOOL:
		use = "";
	}
	if (FALSE) {
case W_EXPECTED_BOOL_UNARY:
		use = " for operand";
	}
	if (FALSE) {
case W_EXPECTED_BOOL_BINARY_LHS:
		use = " for left operand";
	}
	if (FALSE) {
case W_EXPECTED_BOOL_BINARY_RHS:
		use = " for right operand";
	}
	WARNF("Expected boolean expression%s, but got %s",
	      use, CONST_STR());
}
break;
#endif /* DECLARE_WARNING_MESSAGES */

WARNING_NAMESPACE(WN_TPX, 100)

/* Warnings added by the new TPP. */
DEF_WARNING(W_SLASHSTAR_INSIDE_OF_COMMENT, (WG_COMMENTS, WG_COMMENT), WSTATE_WARN, WARNF(Q("/*") " repeated inside of comment"))                                /* [char *]. */
DEF_WARNING(W_LINE_COMMENT_CONTINUED, (WG_COMMENTS, WG_COMMENT), WSTATE_WARN, WARNF("Line-comment continued"))                                                  /* . */
DEF_WARNING(W_VA_KEYWORD_IN_REGULAR_MACRO, (WG_MACROS), WSTATE_WARN, WARNF("Variadic keyword " Q("%s") " used in regular macro", KWDNAME()))                    /* [struct TPPKeyword *]. */
DEF_WARNING(W_KEYWORD_MACRO_ALREADY_ONSTACK, (WG_MACROS), WSTATE_DISABLED, WARNF("Keyword-style macro " Q("%s") " is already being expanded", FILENAME()))      /* [struct TPPFile *]. */
DEF_WARNING(W_FUNCTION_MACRO_ALREADY_ONSTACK, (WG_MACROS), WSTATE_DISABLED, WARNF("Function-style macro " Q("%s") " is expanded to the same text", FILENAME())) /* [struct TPPFile *]. */
DEF_WARNING(W_NOT_ENGOUH_MACRO_ARGUMENTS, (WG_MACROS), WSTATE_ERROR, WARNF("Not enough enough arguments for " Q("%s"), FILENAME()))                             /* [struct TPPFile *]. */
DEF_WARNING(W_CHARACTER_TOO_LONG, (WG_VALUE), WSTATE_ERROR, WARNF("Character sequence is too long"))                                                            /* . */
DEF_WARNING(W_MULTICHAR_NOT_ALLOWED, (WG_VALUE), WSTATE_ERROR, { char *temp = ARG(char *); WARNF("The multi-character sequence " Q("%.*s") " is not not allowed", (int)ARG(size_t), temp); })                                                                                             /* [char const *,size_t]. */
DEF_WARNING(W_INDEX_OUT_OF_BOUNDS, (WG_VALUE), WSTATE_DISABLED, { struct TPPString *s = ARG(struct TPPString *); WARNF("Index %ld is out-of-bounds of 0..%lu", (unsigned long)s->s_size, (unsigned long)ARG(ptrdiff_t)); })                                                                                            /* [struct TPPString *,ptrdiff_t]. */
DEF_WARNING(W_STRING_TERMINATED_BY_LINEFEED, (WG_SYNTAX), WSTATE_ERROR, WARNF("String was terminated by a linefeed"))                                           /* . */
DEF_WARNING(W_STRING_TERMINATED_BY_EOF, (WG_SYNTAX), WSTATE_ERROR, WARNF("String was terminated by EOF"))                                                       /* . */
DEF_WARNING(W_COMMENT_TERMINATED_BY_EOF, (WG_SYNTAX), WSTATE_ERROR, WARNF("Comment was terminated by EOF"))                                                     /* . */
#if !defined(TPP_CONFIG_FEATURE_TRIGRAPHS) || TPP_CONFIG_FEATURE_TRIGRAPHS
DEF_WARNING(W_ENCOUNTERED_TRIGRAPH, (WG_TRIGRAPHS), WSTATE_WARN, WARNF("Encountered trigraph character sequence " Q("%.3s"), ARG(char *))) /* [char (*)[3]]. */
#else /* TPP_CONFIG_FEATURE_TRIGRAPHS */
WARNING(W_UNUSED_112, (WG_VALUE), WSTATE_ERROR)
#endif /* !TPP_CONFIG_FEATURE_TRIGRAPHS */
DEF_WARNING(W_EXPECTED_RBRACKET_IN_EXPRESSION, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q("]") " in expression, but got " TOK_S, TOK_A))                          /* . */
DEF_WARNING(W_EXPECTED_COLON_AFTER_WARNING, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(":") " after #pragma warning, but got " TOK_S, TOK_A))                    /* . */
#ifndef W_EXPECTED_COLLON_AFTER_WARNING /* Deprecated typo */
#define W_EXPECTED_COLLON_AFTER_WARNING W_EXPECTED_COLON_AFTER_WARNING
#endif /* !W_EXPECTED_COLLON_AFTER_WARNING */
DEF_WARNING(W_EXPECTED_COMMA, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(",") ", but got " TOK_S, TOK_A))                                                         /* . */
DEF_WARNING(W_EXPECTED_STRING_IN_EXPRESSION, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string in expression, but got " Q("%s"), CONST_STR()))                         /* [struct TPPConst *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_MESSAGE, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after message, but got " Q("%s"), CONST_STR()))                         /* [struct TPPConst *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_DEPRECATED, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after deprecated, but got " Q("%s"), CONST_STR()))                   /* [struct TPPConst *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_TPP_EXEC, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after tpp_exec, but got " Q("%s"), CONST_STR()))                       /* [struct TPPConst *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_TPP_SETF, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after tpp_set_keyword_flags, but got " Q("%s"), CONST_STR()))          /* [struct TPPConst *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_TPP_STRAT, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after __TPP_STR_AT|__TPP_STR_SUBSTR, but got " Q("%s"), CONST_STR())) /* [struct TPPConst *]. */
#if !defined(TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS) || TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS
DEF_WARNING(W_EXPECTED_STRING_AFTER_TPP_CNTTOK, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after __TPP_COUNT_TOKENS, but got " Q("%s"), CONST_STR())) /* [struct TPPConst *]. */
#else /* TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS */
WARNING(W_UNUSED_122, (WG_VALUE), WSTATE_ERROR)
#endif /* !TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS */
DEF_WARNING(W_EXPECTED_STRING_AFTER_PRGERROR, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after #pragma error, but got " Q("%s"), CONST_STR()))                                                     /* [struct TPPConst *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_EXTENSION, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after #pragma extension, but got " Q("%s"), CONST_STR()))                                                /* [struct TPPConst *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_TPP_INCPTH, (WG_VALUE), WSTATE_ERROR, WARNF("Expected string after #pragma TPP include_path, but got " Q("%s"), CONST_STR()))                                        /* [struct TPPConst *]. */
DEF_WARNING(W_MACRO_RECURSION_LIMIT_EXCEEDED, (WG_LIMIT), WSTATE_ERROR, WARNF("Macro recursion limit exceeded when expanding " Q("%s") " (Consider passing " Q("-fno-macro-recursion") ")", FILENAME())) /* [struct TPPFile *]. */
DEF_WARNING(W_INCLUDE_RECURSION_LIMIT_EXCEEDED, (WG_LIMIT), WSTATE_ERROR, WARNF("Include recursion limit exceeded when including " Q("%s"), FILENAME()))                                                 /* [struct TPPFile *]. */
DEF_WARNING(W_UNKNOWN_EXTENSION, (WG_VALUE), WSTATE_ERROR, { char *temp = ARG(char *); WARNF("Unknown extension " Q("%s") " (Did you mean " Q("%s") "?)", temp,find_most_likely_extension(temp)); })                                                                                                                                          /* [char const *]. */
DEF_WARNING(W_DEFINED_IN_MACRO_BODY, (WG_EXPANSION_TO_DEFINED), WSTATE_WARN, WARNF(Q("defined") " found in macro body"))
#if !defined(TPP_CONFIG_EXTENSION_IDENT_SCCS) || TPP_CONFIG_EXTENSION_IDENT_SCCS
#ifndef TPP_CONFIG_CALLBACK_INS_COMMENT
DEF_WARNING(W_IDENT_SCCS_IGNORED, (WG_USAGE), WSTATE_WARN, WARNF("#ident/sccs with " Q("%s") " is ignored", CONST_STR())) /* [struct TPPConst *]. */
#else /* !TPP_CONFIG_CALLBACK_INS_COMMENT */
WARNING(W_UNUSED_130, (WG_VALUE), WSTATE_WARN)
#endif /* TPP_CONFIG_CALLBACK_INS_COMMENT */
DEF_WARNING(W_EXPECTED_STRING_AFTER_IDENT, (WG_SYNTAX, WG_VALUE), WSTATE_WARN, WARNF("Expected string after #ident, but got " Q("%s"), CONST_STR())) /* [struct TPPConst *]. */
#else /* TPP_CONFIG_EXTENSION_IDENT_SCCS */
WARNING(W_UNUSED_130, (WG_VALUE), WSTATE_WARN)
WARNING(W_UNUSED_131, (WG_VALUE), WSTATE_WARN)
#endif /* !TPP_CONFIG_EXTENSION_IDENT_SCCS */
DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_ASSERT, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after #assert, but got " TOK_S, TOK_A))                                        /* . */
DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_PREDICATE, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after predicate " Q("%s") " in #assert, but got " TOK_S, KWDNAME(), TOK_A)) /* [struct TPPKeyword *]. */
#if !defined(TPP_CONFIG_EXTENSION_STRINGOPS) || TPP_CONFIG_EXTENSION_STRINGOPS
DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_EXPR_HASH, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after # in expression, but got " TOK_S, TOK_A)) /* . */
#else /* TPP_CONFIG_EXTENSION_STRINGOPS */
WARNING(W_UNUSED_134, (WG_SYNTAX), WSTATE_ERROR)
#endif /* !TPP_CONFIG_EXTENSION_STRINGOPS */
#if !defined(TPP_CONFIG_EXTENSION_ASSERTIONS) || TPP_CONFIG_EXTENSION_ASSERTIONS
DEF_WARNING(W_EXPECTED_KEYWORD_AFTER_EXPR_PRED, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected keyword after predicate " Q("%s") " in expression, but got " TOK_S, KWDNAME(), TOK_A)) /* . */
#else /* TPP_CONFIG_EXTENSION_ASSERTIONS */
WARNING(W_UNUSED_135, (WG_SYNTAX), WSTATE_ERROR)
#endif /* !TPP_CONFIG_EXTENSION_ASSERTIONS */
DEF_WARNING(W_UNKNOWN_ASSERTION, (WG_VALUE), WSTATE_DISABLED, { char const *temp = KWDNAME(); WARNF("Assertion " Q("%s") " does not contain a predicate " Q("%s"), temp, KWDNAME()); }) /* [struct TPPKeyword *,struct TPPKeyword *]. */
DEF_WARNING(W_EXPECTED_STRING_AFTER_GCC_DIAG, (WG_SYNTAX, WG_VALUE), WSTATE_ERROR, WARNF("Expected string after #pragma GCC diagnostic <mode>, but got " Q("%s"), CONST_STR())) /* [struct TPPConst *]. */
DEF_WARNING(W_CANT_POP_EXTENSIONS, (WG_VALUE), WSTATE_ERROR, WARNF("Can't pop extensions"))                                                                                     /* . */
DEF_WARNING(W_CANT_POP_INCLUDE_PATH, (WG_VALUE), WSTATE_ERROR, WARNF("Can't pop #include paths"))                                                                               /* . */
DEF_WARNING(W_CONSIDER_PAREN_AROUND_LAND, (WG_QUALITY), WSTATE_WARN, WARNF("Consider adding parenthesis around " Q("&&") " to prevent confusion with " Q("||")))                /* . */
#if TPP_HAVE_LONGLONG
DEF_WARNING(W_INTEGRAL_OVERFLOW, (WG_VALUE), WSTATE_WARN, { tint_t old = ARG(tint_t); WARNF("Integral constant overflow from " Q("%lld") " to " Q("%lld"), (long long)old, (long long)ARG(tint_t)); }) /* [tint_t,tint_t]. */
#else /* TPP_HAVE_LONGLONG */
DEF_WARNING(W_INTEGRAL_OVERFLOW, (WG_VALUE), WSTATE_WARN, { tint_t old = ARG(tint_t); WARNF("Integral constant overflow from " Q("%ld") " to " Q("%ld"), (long)old, (long)ARG(tint_t)); }) /* [tint_t,tint_t]. */
#endif /* !TPP_HAVE_LONGLONG */
DEF_WARNING(W_INTEGRAL_CLAMPED, (WG_VALUE), WSTATE_WARN, WARNF("Integral constant clamped to fit")) /* [tint_t,tint_t]. */
DEF_WARNING(W_UNKNOWN_INCLUDE_PATH, (WG_VALUE), WSTATE_WARN, { char *temp = ARG(char *); WARNF("Unknown system #include-path " Q("%.*s"), (int)ARG(size_t), temp); })                                   /* [char const *,size_t]. */
DEF_WARNING(W_INCLUDE_PATH_ALREADY_EXISTS, (WG_VALUE), WSTATE_WARN, { char *temp = ARG(char *); WARNF("System #include-path " Q("%.*s") " already exists", (int)ARG(size_t), temp); })                            /* [char const *,size_t]. */
#if !defined(TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR) || TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR
DEF_WARNING(W_EXPECTED_ELSE_IN_EXPRESSION, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q("else") " in expression, but got " TOK_S, TOK_A)) /* . */
#else /* TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR */
WARNING(W_UNUSED_145, (WG_SYNTAX), WSTATE_ERROR) /* . */
#endif /* !TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR */
DEF_WARNING(W_STATEMENT_IN_EXPRESSION, (WG_USAGE, WG_SYNTAX), WSTATE_WARN, WARNF("GCC-style statement " TOK_S " in expression is not understood", TOK_A))                                                  /* . */
DEF_WARNING(W_TYPECAST_IN_EXPRESSION, (WG_USAGE, WG_SYNTAX), WSTATE_WARN, WARNF("C-style type cast " TOK_S " in expression is not understood (Consider using bit-masks to narrow integral types)", TOK_A)) /* . */
DEF_WARNING(W_EXPECTED_RPAREN_AFTER_CAST, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q(")") " after casting type, but got " TOK_S, TOK_A))                                                               /* . */
DEF_WARNING(W_EXPECTED_RBRACE_AFTER_STATEMENT, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q("}") " after statement, but got " TOK_S, TOK_A))                                                             /* . */
DEF_WARNING(W_EXPECTED_WARNING_NAMEORID, (WG_SYNTAX, WG_VALUE), WSTATE_ERROR, WARNF("Expected warning name or id, but got " Q("%s"), CONST_STR()))                                                         /* [struct TPPConst *]. */
#if !defined(TPP_CONFIG_LOCKED_KEYWORDS) || TPP_CONFIG_LOCKED_KEYWORDS
DEF_WARNING(W_CANT_DEFINE_LOCKED_KEYWORD, (WG_VALUE), WSTATE_DISABLED, WARNF("Cannot #define macro for locked keyword " Q("%s"), KWDNAME())) /* [struct TPPKeyword *]. */
DEF_WARNING(W_CANT_UNDEF_LOCKED_KEYWORD, (WG_VALUE), WSTATE_DISABLED, WARNF("Cannot #undef macro for locked keyword " Q("%s"), KWDNAME()))   /* [struct TPPKeyword *]. */
#else /* TPP_CONFIG_LOCKED_KEYWORDS */
WARNING(W_UNUSED_151, (WG_SYNTAX), WSTATE_ERROR)
WARNING(W_UNUSED_152, (WG_SYNTAX), WSTATE_ERROR)
#endif /* !TPP_CONFIG_LOCKED_KEYWORDS */
DEF_WARNING(W_INVALID_FLOAT_SUFFIX, (WG_SYNTAX), WSTATE_ERROR, { char *temp = ARG(char *); WARNF("Invalid floating point suffix " Q("%.*s"), (int)ARG(size_t), temp); }) /* [char const *,size_t] */
DEF_WARNING(W_DEPENDENCY_CHANGED, (WG_DEPENDENCY), WSTATE_ERROR, {
	char *depnam         = ARG(char *);
	char *srcnam         = ARG(char *);
	char *reason         = ARG(char *);
	size_t reason_length = ARG(size_t);
	WARNF("Dependency " Q("%s") " changed after " Q("%s"), depnam, srcnam);
	if (reason_length)
		WARNF(" (" Q("%.*s") ")", (unsigned int)reason_length, reason);
})
#if !defined(TPP_CONFIG_EXTENSION_VA_OPT) || TPP_CONFIG_EXTENSION_VA_OPT
DEF_WARNING(W_EXPECTED_LPAREN_AFTER_VA_OPT, (WG_SYNTAX), WSTATE_ERROR, WARNF("Expected " Q("(") " after " Q("__VA_OPT__") ", but got " TOK_S, TOK_A))
DEF_WARNING(W_EXPECTED_RPAREN_AFTER_VA_OPT, (WG_SYNTAX), WSTATE_ERROR, WARNF("Missing " Q(")") " after " Q("__VA_OPT__(...")))
#else /* TPP_CONFIG_EXTENSION_VA_OPT */
WARNING(W_UNUSED_155, (WG_SYNTAX), WSTATE_ERROR) /* . */
WARNING(W_UNUSED_156, (WG_SYNTAX), WSTATE_ERROR) /* . */
#endif /* !TPP_CONFIG_EXTENSION_VA_OPT */

/* _always_ add new warnings here! */



/* Allow the user to provide additional definitions */
#ifdef TPP_USERDEFS
#include TPP_USERDEFS
#endif /* TPP_USERDEFS */

#undef TPP_PP_STR
#undef TPP_PP_STR2
#undef PREDEFINED_BUILTIN_FUNCTION
#undef PREDEFINED_BUILTIN_FUNCTION_IF
#undef PREDEFINED_BUILTIN_KWDFUNCTION
#undef PREDEFINED_BUILTIN_KWDFUNCTION_IF
#undef PREDEFINED_FUNCTION
#undef PREDEFINED_FUNCTION_IF
#undef PREDEFINED_KWDFUNCTION
#undef PREDEFINED_KWDFUNCTION_IF
#undef PREDEFINED_RT_MACRO
#undef PREDEFINED_RT_MACRO_IF
#undef PREDEFINED_RT_KWDMACRO
#undef PREDEFINED_RT_KWDMACRO_IF
#undef PREDEFINED_MACRO
#undef PREDEFINED_MACRO_IF
#undef PREDEFINED_KWDMACRO
#undef PREDEFINED_KWDMACRO_IF
#undef HAS_EXTENSION_IF
#undef HAS_FEATURE_IF
#undef HAS_BUILTIN_IF
#undef DEF_WARNING
#undef DEF_M_IF
#undef DEF_M
#undef DEF_K

#ifdef TPP_DEFS_DEFINES_EXTENSION
#undef TPP_DEFS_DEFINES_EXTENSION
#undef EXTENSION
#endif /* TPP_DEFS_DEFINES_EXTENSION */
#ifdef TPP_DEFS_DEFINES_WARNING_MESSAGE
#undef TPP_DEFS_DEFINES_WARNING_MESSAGE
#undef WARNING_MESSAGE
#endif /* TPP_DEFS_DEFINES_WARNING_MESSAGE */
#ifdef TPP_DEFS_DEFINES_BUILTIN_FUNCTION
#undef TPP_DEFS_DEFINES_BUILTIN_FUNCTION
#undef BUILTIN_FUNCTION
#endif /* TPP_DEFS_DEFINES_BUILTIN_FUNCTION */
#ifdef TPP_DEFS_DEFINES_RT_BUILTIN_MACRO
#undef TPP_DEFS_DEFINES_RT_BUILTIN_MACRO
#undef RT_BUILTIN_MACRO
#endif /* TPP_DEFS_DEFINES_RT_BUILTIN_MACRO */
#ifdef TPP_DEFS_DEFINES_BUILTIN_MACRO
#undef TPP_DEFS_DEFINES_BUILTIN_MACRO
#undef BUILTIN_MACRO
#endif /* TPP_DEFS_DEFINES_BUILTIN_MACRO */
#ifdef TPP_DEFS_DEFINES_MACRO
#undef TPP_DEFS_DEFINES_MACRO
#undef MACRO
#endif /* TPP_DEFS_DEFINES_MACRO */
#ifdef TPP_DEFS_DEFINES_WARNING_NAMESPACE
#undef TPP_DEFS_DEFINES_WARNING_NAMESPACE
#undef WARNING_NAMESPACE
#endif /* TPP_DEFS_DEFINES_WARNING_NAMESPACE */
#ifdef TPP_DEFS_DEFINES_WARNING
#undef TPP_DEFS_DEFINES_WARNING
#undef WARNING
#endif /* TPP_DEFS_DEFINES_WARNING */
#ifdef TPP_DEFS_DEFINES_WGROUP
#undef TPP_DEFS_DEFINES_WGROUP
#undef WGROUP
#endif /* TPP_DEFS_DEFINES_WGROUP */
#ifdef TPP_DEFS_DEFINES_KWD_FLAGS
#undef TPP_DEFS_DEFINES_KWD_FLAGS
#undef KWD_FLAGS
#endif /* TPP_DEFS_DEFINES_KWD_FLAGS */
#ifdef TPP_DEFS_DEFINES_KWD
#undef TPP_DEFS_DEFINES_KWD
#undef KWD
#endif /* TPP_DEFS_DEFINES_KWD */
