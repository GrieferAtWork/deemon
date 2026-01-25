/*[[[vs
ClCompile.CompileAs = CompileAsC
]]]*/
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
#ifndef GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_C
#define GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_C 1
#define DEE_SOURCE

#ifdef __cplusplus
#error "This won't work; sqlite.c can only be compiled under as C, but not C++"
#else /* __cplusplus */

#include <deemon/api.h>

#ifndef __ATTR_NORETURN_IS__Noreturn
#undef _Noreturn
#define _Noreturn ATTR_NORETURN
#endif /* !__ATTR_NORETURN_IS__Noreturn */

#ifdef _MSC_VER
#pragma warning(disable: 4996) /* Prevent deprecation warnings */
#endif /* _MSC_VER */

#include <deemon/system-features.h> /* CONFIG_HAVE_*, _msize, alloca, malloc_usable_size, strcmp */

#include <hybrid/__alloca.h>  /* __hybrid_alloca, __hybrid_alloca_IS_alloca */
#include <hybrid/byteorder.h> /* __BYTE_ORDER__ */
#include <hybrid/typecore.h>  /* __*_TYPE__ */

#include "sqlite3-external.h"

/* Configure SQLite */
#define SQLITE_PRIVATE PRIVATE


#ifdef __BYTE_ORDER__
#define SQLITE_BYTEORDER __BYTE_ORDER__
#else /* __BYTE_ORDER__ */
#define SQLITE_BYTEORDER 0
#endif /* !__BYTE_ORDER__ */

#define UINT32_TYPE __UINT32_TYPE__
#define UINT16_TYPE __UINT16_TYPE__
#define UINT8_TYPE  __UINT8_TYPE__
#define INT32_TYPE  __INT32_TYPE__
#define INT16_TYPE  __INT16_TYPE__
#define INT8_TYPE   __INT8_TYPE__


/* Tell sqlite3 what we known regarding supported host system features. */
#undef HAVE_LOG2
#ifdef CONFIG_HAVE_log2
#define HAVE_LOG2 1
#else /* CONFIG_HAVE_log2 */
#define HAVE_LOG2 0
#endif /* !CONFIG_HAVE_log2 */

#undef HAVE_LOG10
#ifdef CONFIG_HAVE_log10
#define HAVE_LOG10 1
#else /* CONFIG_HAVE_log10 */
#define HAVE_LOG10 0
#endif /* !CONFIG_HAVE_log10 */

#undef HAVE_ISNAN
#ifdef CONFIG_HAVE_isnan
#define HAVE_ISNAN 1
#else /* CONFIG_HAVE_isnan */
#define HAVE_ISNAN 0
#endif /* !CONFIG_HAVE_isnan */

#undef HAVE_STRCHRNUL
#ifdef CONFIG_HAVE_strchrnul
#define HAVE_STRCHRNUL 1
#else /* CONFIG_HAVE_strchrnul */
#define HAVE_STRCHRNUL 0
#endif /* !CONFIG_HAVE_strchrnul */

#undef HAVE_USLEEP
#ifdef CONFIG_HAVE_usleep
#define HAVE_USLEEP 1
#else /* CONFIG_HAVE_usleep */
#define HAVE_USLEEP 0
#endif /* !CONFIG_HAVE_usleep */

#undef HAVE_NANOSLEEP
#ifdef CONFIG_HAVE_nanosleep
#define HAVE_NANOSLEEP 1
#else /* CONFIG_HAVE_nanosleep */
#define HAVE_NANOSLEEP 0
#endif /* !CONFIG_HAVE_nanosleep */

#undef HAVE_GMTIME_R
#ifdef CONFIG_HAVE_gmtime_r
#define HAVE_GMTIME_R 1
#else /* CONFIG_HAVE_gmtime_r */
#define HAVE_GMTIME_R 0
#endif /* !CONFIG_HAVE_gmtime_r */

#undef HAVE_LOCALTIME_S
#ifdef CONFIG_HAVE_localtime_s
#define HAVE_LOCALTIME_S 1
#else /* CONFIG_HAVE_localtime_s */
#define HAVE_LOCALTIME_S 0
#endif /* !CONFIG_HAVE_localtime_s */

#undef HAVE_MALLOC_H
#ifdef CONFIG_HAVE_MALLOC_H
#define HAVE_MALLOC_H 1
#else /* CONFIG_HAVE_MALLOC_H */
#define HAVE_MALLOC_H 0
#endif /* !CONFIG_HAVE_MALLOC_H */

#undef HAVE_MALLOC_USABLE_SIZE
#if (defined(CONFIG_HAVE_malloc_usable_size) &&             \
     (!defined(CONFIG_HAVE_malloc_usable_size_IS__msize) || \
      !defined(_MSC_VER)))
#define HAVE_MALLOC_USABLE_SIZE 1
#else /* ... */
#define HAVE_MALLOC_USABLE_SIZE 0
#endif /* !... */

/* Deemon always needs a big stack, so SQLite can make use of that, too. */
#undef SQLITE_USE_ALLOCA
#ifdef __hybrid_alloca
#define SQLITE_USE_ALLOCA 1
#ifndef __hybrid_alloca_IS_alloca
#undef alloca
#define alloca __hybrid_alloca
#endif /* !__hybrid_alloca_IS_alloca */
#endif /* __hybrid_alloca */

#undef SQLITE_SYSTEM_MALLOC
#define SQLITE_SYSTEM_MALLOC 1

#undef NDEBUG
#define NDEBUG

#undef SQLITE_MALLOCSIZE
#if !defined(__APPLE__) || defined(SQLITE_WITHOUT_ZONEMALLOC)
#if defined(CONFIG_HAVE_malloc_usable_size) && !defined(CONFIG_HAVE_malloc_usable_size_IS__msize)
#define SQLITE_MALLOCSIZE(x) malloc_usable_size(x)
#elif defined(CONFIG_HAVE__msize)
#define SQLITE_MALLOCSIZE(x) _msize(x)
#endif /* ... */
#endif /* ... */


/* Enable some compatibility aliases originally
 * meant for KOS, but also useful for SQLite */
#include <deemon/util/kos-compat.h>


/* Delete some stuff that SQLite doesn't like */
#undef likely
#undef unlikely
#ifdef __cplusplus
#define new new_
#endif /* __cplusplus */

#define SQLITE_CUSTOM_STMT_DATA Vdbe *pNextFreeStmt;

__pragma_GCC_diagnostic_ignored(Wunused_parameter)
__pragma_GCC_diagnostic_ignored(Wsign_compare)
__pragma_GCC_diagnostic_ignored(Wunused_function)
__pragma_GCC_diagnostic_ignored(Wunused_variable)

/* Include SQLite source */
#ifndef __INTELLISENSE__
#include "../../external/sqlite-amalgamation-3500400/sqlite3.c"
#endif /* !__INTELLISENSE__ */

/* For some reason, this doesn't get defined because of "SQLITE_AMALGAMATION" ? */
SQLITE_API const char sqlite3_version[] = SQLITE_VERSION;


/* Custom function: like `sqlite3_bind_parameter_index', but "zName"
 * doesn't have to include the leading `:', `$' or `@'. */
SQLITE_API int sqlite3_bind_parameter_index__without_prefix(sqlite3_stmt *pStmt, char const *zName) {
	VList *pIn = ((Vdbe*)pStmt)->pVList;
	int i, mx;
	if (pIn == 0)
		return 0;
	mx = pIn[1];
	i  = 2;
	do {
		char const *z = (char const *)&pIn[i + 2];
		if ((*z == ':' || *z == '$' || *z == '@') && strcmp(z + 1, zName) == 0)
			return pIn[i];
		i += pIn[i + 1];
	} while (i < mx);
	return 0;
}

SQLITE_API sqlite3_stmt *sqlite3_stmt_getnextfree(sqlite3_stmt *pStmt) {
	return (sqlite3_stmt *)((Vdbe*)pStmt)->pNextFreeStmt; /* ON-COMPILE-ERROR: Did you forget to patch sqlite3 with "SQLITE_CUSTOM_STMT_DATA"? */
}
SQLITE_API void sqlite3_stmt_setnextfree(sqlite3_stmt *pStmt, sqlite3_stmt *pNext) {
	((Vdbe*)pStmt)->pNextFreeStmt = (Vdbe*)pNext; /* ON-COMPILE-ERROR: Did you forget to patch sqlite3 with "SQLITE_CUSTOM_STMT_DATA"? */
}

#endif /* !__cplusplus */

#endif /* !GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_C */
