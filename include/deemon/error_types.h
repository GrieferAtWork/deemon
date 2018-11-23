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
#ifndef GUARD_DEEMON_ERROR_TYPES_H
#define GUARD_DEEMON_ERROR_TYPES_H 1

#include "api.h"
#include "object.h"
#include "error.h"

DECL_BEGIN

typedef struct system_error_object DeeSystemErrorObject;
typedef struct nomemory_error_object DeeNoMemoryErrorObject;
typedef struct compiler_error_object DeeCompilerErrorObject;
typedef DeeObject DeeSignalObject;

struct system_error_object {
    ERROR_OBJECT_HEAD
    syserrno_t se_errno; /* A system-specific error code, or `SYSTEM_ERROR_UNKNOWN' when not known. */
};


struct nomemory_error_object {
    ERROR_OBJECT_HEAD
    size_t nm_allocsize; /* The size of the allocation that failed (in bytes).
                          * Set to `0' when not known. */
};

struct TPPFile;
struct compiler_error_loc {
    struct compiler_error_loc *cl_prev; /* [0..1][OVERRIDE(->cl_file,[1..1])]
                                         * Calling compiler location (might be used
                                         * when the parser was inside of a macro) */
    /*ref*/struct TPPFile     *cl_file; /* [0..1] The file in which the error occurred
                                         * (when `NULL', no location information is available) */
    int                        cl_line; /* The line within `cl_file' (0-based) */
    int                        cl_col;  /* The column within that `cl_line' (0-based) */
};

struct compiler_error_object {
    ERROR_OBJECT_HEAD
    WEAKREF_SUPPORT
    int                             ce_mode;   /* Fatality mode (One of `COMPILER_ERROR_FATALITY_*'). */
    int                             ce_wnum;   /* [const] The TPP-assigned warning ID of this error (One of `W_*'). */
    struct compiler_error_loc       ce_locs;   /* [const] The parser location where the error occurred. */
    struct compiler_error_loc      *ce_loc;    /* [1..1][const] The main compiler location (that is the first text-file that can be encountered when walking `ce_locs') */
    WEAKREF(DeeCompilerErrorObject) ce_master; /* Weak reference to the master compiler error. */
    size_t                          ce_errorc; /* [const] Number of contained compiler errors. */
    DREF DeeCompilerErrorObject   **ce_errorv; /* [1..1][REF_IF(!= self)][const][0..ce_errorc][owned][const]
                                                * Vector of other errors/warnings that occurred, leading up to this one.
                                                * NOTE: The master compiler error (aka. `this' error) is
                                                *       the error that caused compilation to actually fail,
                                                *       meaning that it is the first matching error in the
                                                *       following list of conditions:
                                                *        - ce_mode == COMPILER_ERROR_FATALITY_FORCEFATAL
                                                *        - ce_mode == COMPILER_ERROR_FATALITY_FATAL
                                                *        - ce_mode == COMPILER_ERROR_FATALITY_ERROR */
};


#ifdef GUARD_DEEMON_OBJECTS_ERROR_TYPES_C
DDATDEF DeeNoMemoryErrorObject DeeError_NoMemory_instance;
DDATDEF DeeSignalObject DeeError_StopIteration_instance;
DDATDEF DeeSignalObject DeeError_Interrupt_instance;
#endif /* GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */

DECL_END

#endif /* !GUARD_DEEMON_ERROR_TYPES_H */
