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
/*!export **/
/*!export Dee*ErrorObject*/
#ifndef GUARD_DEEMON_ERROR_TYPES_H
#define GUARD_DEEMON_ERROR_TYPES_H 1 /*!export-*/

#include "api.h"

#include "error.h" /* Dee_ERROR_OBJECT_HEAD */
#include "types.h" /* Dee_OBJECT_HEAD */

#include <stddef.h> /* size_t */
#ifdef CONFIG_HOST_WINDOWS
#include <stdint.h> /* uint32_t */
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

typedef struct Dee_system_error_object {
	Dee_ERROR_OBJECT_HEAD
	/*errno_t*/ int se_errno;     /* A system-specific error code, or `Dee_SYSTEM_ERROR_UNKNOWN` when not known. */
#ifdef CONFIG_HOST_WINDOWS
	uint32_t        se_lasterror; /* The windows-specific error code (as returned by `GetLastError()`)
	                               * Set to `NO_ERROR` if unused. */
#endif /* CONFIG_HOST_WINDOWS */
} DeeSystemErrorObject;

typedef struct Dee_nomemory_error_object {
	Dee_ERROR_OBJECT_HEAD
	size_t nm_allocsize; /* The size of the allocation that failed (in bytes).
	                      * Set to `0` when not known. */
} DeeNoMemoryErrorObject;

typedef struct Dee_signal_object {
	Dee_OBJECT_HEAD
} DeeSignalObject;

#ifdef GUARD_DEEMON_OBJECTS_ERROR_TYPES_C
DDATDEF DeeNoMemoryErrorObject DeeError_NoMemory_instance; /*!export-*/
DDATDEF DeeSignalObject DeeError_StopIteration_instance;   /*!export-*/
DDATDEF DeeSignalObject DeeError_Interrupt_instance;       /*!export-*/
#endif /* GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */

DECL_END

#endif /* !GUARD_DEEMON_ERROR_TYPES_H */
