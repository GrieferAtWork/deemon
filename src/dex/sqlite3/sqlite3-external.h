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
#ifndef GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_H
#define GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_H 1

#include <deemon/api.h>
/**/

#include <hybrid/typecore.h>

/* Configure sqlite */
#define SQLITE_EXTERN                   INTDEF
#define SQLITE_API                      INTERN
#define SQLITE_OMIT_COMPILEOPTION_DIAGS   /* Don't need APIs to query compile options */
#define SQLITE_OMIT_LOAD_EXTENSION        /* We never care about extension loading */
#define SQLITE_OMIT_AUTOINIT              /* We do our own initialization */
#define SQLITE_OMIT_DEPRECATED            /* Don't need deprecated APIs */
#define SQLITE_UNTESTABLE                 /* Omit code used by sqlite's built-in self-test */
#define SQLITE_OMIT_SHARED_CACHE          /* recommended: https://sqlite.org/sharedcache.html#dontuse */
#define SQLITE_LIKE_DOESNT_MATCH_BLOBS    /* recommended: https://sqlite.org/compile.html#like_doesnt_match_blobs */
#define SQLITE_DQS                      0 /* recommended: https://sqlite.org/compile.html#dqs */
#define SQLITE_DEFAULT_FOREIGN_KEYS     1 /* Enable foreign key enforcement by default */
#undef SQLITE_DEBUG
/*#define SQLITE_ENABLE_MEMORY_MANAGEMENT*/

/* Enable some lightweight/useful extensions */
#define SQLITE_ENABLE_STMTVTAB
#define SQLITE_ENABLE_FTS3
#define SQLITE_ENABLE_FTS4
#define SQLITE_ENABLE_FTS5


/* Tell sqlite3 that we do our own locking (with blackjack & hookers)
 * Reason: The way that sqlite3 does locking, there is a situation where
 *         DeeThread_Wake() will be unable to interrupt certain threads:
 * - Thread #1: holding lock to DB mutex, and in long-running query
 * - Thread #2: trying to acquire lock to DB mutex
 * - Thread #3: Sends interrupt to Thread #2
 * - Thread #2: Unable to receive interrupt because sqlite3's mutex can't indicate interrupts
 *
 * Solution: can't rely on sqlite3's mutex code -- must do our
 *           own and compile sqlite3 with SQLITE_THREADSAFE=2 */
#ifdef CONFIG_NO_THREADS
#define SQLITE_THREADSAFE 0
#else /* CONFIG_NO_THREADS */
#define SQLITE_THREADSAFE 2
#endif /* !CONFIG_NO_THREADS */


#ifdef __INT64_TYPE__
#define SQLITE_INT64_TYPE  __INT64_TYPE__
#define SQLITE_UINT64_TYPE __UINT64_TYPE__
#endif /* __INT64_TYPE__ */

/* Include sqlite */
#ifndef NDEBUG
#define NDEBUG
#define DEE_PRIVATE_NDEBUG_FOR_EXTERNAL_SQLITE3_H
#endif /* !NDEBUG */
#include "../../external/sqlite-amalgamation-3500400/sqlite3.h"
#ifdef DEE_PRIVATE_NDEBUG_FOR_EXTERNAL_SQLITE3_H
#undef DEE_PRIVATE_NDEBUG_FOR_EXTERNAL_SQLITE3_H
#undef NDEBUG
#endif /* DEE_PRIVATE_NDEBUG_FOR_EXTERNAL_SQLITE3_H */

#endif /* !GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_H */
