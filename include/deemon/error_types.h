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
/*!export DeeError_*_instance*/
#ifndef GUARD_DEEMON_ERROR_TYPES_H
#define GUARD_DEEMON_ERROR_TYPES_H 1 /*!export-*/

#include "api.h"

#include "error.h"        /* Dee_ERROR_OBJECT_HEAD */
#include "object.h"       /* DeeObject_Print */
#include "types.h"        /* DREF, DeeObject, Dee_OBJECT_HEAD, Dee_WEAKREF_SUPPORT, Dee_formatprinter_t, Dee_ssize_t */
#include "util/weakref.h" /* Dee_WEAKREF */

#include <stddef.h> /* size_t */
#ifdef CONFIG_HOST_WINDOWS
#include <stdint.h> /* uint32_t */
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

typedef struct Dee_system_error_object {
	Dee_ERROR_OBJECT_HEAD
	/*errno_t*/ int se_errno;     /* A system-specific error code, or `Dee_SYSTEM_ERROR_UNKNOWN' when not known. */
#ifdef CONFIG_HOST_WINDOWS
	uint32_t        se_lasterror; /* The windows-specific error code (as returned by `GetLastError()')
	                               * Set to `NO_ERROR' if unused. */
#endif /* CONFIG_HOST_WINDOWS */
} DeeSystemErrorObject;

typedef struct Dee_nomemory_error_object {
	Dee_ERROR_OBJECT_HEAD
	size_t nm_allocsize; /* The size of the allocation that failed (in bytes).
	                      * Set to `0' when not known. */
} DeeNoMemoryErrorObject;

typedef struct Dee_signal_object {
	Dee_OBJECT_HEAD
} DeeSignalObject;

struct TPPFile;
struct Dee_compiler_error_loc {
	struct Dee_compiler_error_loc *cl_prev; /* [0..1][OVERRIDE(->cl_file, [1..1])]
	                                         * Calling compiler location (might be used
	                                         * when the parser was inside of a macro) */
	/*ref*/ struct TPPFile        *cl_file; /* [0..1] The file in which the error occurred
	                                         * (when `NULL', no location information is available) */
	int                            cl_line; /* The line within `cl_file' (0-based) */
	int                            cl_col;  /* The column within that `cl_line' (0-based) */
};

typedef struct Dee_compiler_error_object {
	Dee_ERROR_OBJECT_HEAD
	Dee_WEAKREF_SUPPORT
	int                                           ce_mode;   /* Fatality mode (One of `COMPILER_ERROR_FATALITY_*'). */
	int                                           ce_wnum;   /* [const] The TPP-assigned warning ID of this error (One of `W_*'). */
	struct Dee_compiler_error_loc                 ce_locs;   /* [const] The parser location where the error occurred. */
	struct Dee_compiler_error_loc                *ce_loc;    /* [0..1][const] The main compiler location (that is the first text-file that can be encountered when walking `ce_locs') */
	Dee_WEAKREF(struct Dee_compiler_error_object) ce_master; /* Weak reference to the master compiler error. */
	size_t                                        ce_errorc; /* [const] Number of contained compiler errors. */
	DREF struct Dee_compiler_error_object       **ce_errorv; /* [1..1][REF_IF(!= self)][const][0..ce_errorc][owned][const]
	                                                          * Vector of other errors/warnings that occurred, leading up to this one.
	                                                          * NOTE: The master compiler error (aka. `this' error) is
	                                                          *       the error that caused compilation to actually fail,
	                                                          *       meaning that it is the first matching error in the
	                                                          *       following list of conditions:
	                                                          *        - ce_mode == Dee_COMPILER_ERROR_FATALITY_FORCEFATAL
	                                                          *        - ce_mode == Dee_COMPILER_ERROR_FATALITY_FATAL
	                                                          *        - ce_mode == Dee_COMPILER_ERROR_FATALITY_ERROR */
} DeeCompilerErrorObject;


#ifdef GUARD_DEEMON_OBJECTS_ERROR_TYPES_C
DDATDEF DeeNoMemoryErrorObject DeeError_NoMemory_instance;
DDATDEF DeeSignalObject DeeError_StopIteration_instance;
DDATDEF DeeSignalObject DeeError_Interrupt_instance;
#endif /* GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */

#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeCompilerError_Print(DeeObject *__restrict self,
                       Dee_formatprinter_t printer, void *arg);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeCompilerError_Print(self, printer, arg) \
	DeeObject_Print(self, printer, arg)
#endif /* !CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_ERROR_TYPES_H */
