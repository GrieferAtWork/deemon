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
#ifndef GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_H
#define GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_H 1

#include <deemon/api.h>
/**/

#include <hybrid/typecore.h>

/* Configure sqlite */
#define SQLITE_EXTERN INTDEF
#define SQLITE_API    INTERN
#define SQLITE_OMIT_COMPILEOPTION_DIAGS
#define SQLITE_OMIT_LOAD_EXTENSION
#define SQLITE_OMIT_AUTOINIT
/*#define SQLITE_ENABLE_MEMORY_MANAGEMENT*/

#ifdef __INT64_TYPE__
#define SQLITE_INT64_TYPE  __INT64_TYPE__
#define SQLITE_UINT64_TYPE __UINT64_TYPE__
#endif /* __INT64_TYPE__ */

/* Include sqlite */
#include "../../external/sqlite-amalgamation-3500400/sqlite3.h"

#endif /* !GUARD_DEX_SQLITE3_SQLITE3_EXTERNAL_H */
