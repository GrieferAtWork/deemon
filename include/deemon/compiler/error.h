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
/*!export *_PARSER_CALLBACK*/
/*!export parser_**/
#ifndef GUARD_DEEMON_COMPILER_ERROR_H
#define GUARD_DEEMON_COMPILER_ERROR_H 1 /*!export-*/

#include "../api.h"
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */

#ifdef CONFIG_BUILDING_DEEMON
#include "../error.h"           /* Dee_ERROR_OBJECT_HEAD */
#include "../object.h"          /* DREF, DeeObject, DeeObject_Print, Dee_WEAKREF_SUPPORT, Dee_formatprinter_t, Dee_ssize_t */
#include "../system-features.h" /* bzero, memcpy */
#include "../types.h"           /* DREF, DeeObject, Dee_WEAKREF_SUPPORT, Dee_formatprinter_t, Dee_ssize_t */
#include "../util/weakref.h"    /* Dee_WEAKREF */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

struct Dee_compiler_error_object;
struct parser_errors {
	size_t                                  pe_errora; /* Allocated vector size. */
	size_t                                  pe_errorc; /* Total amount of errors/warnings. */
	DREF struct Dee_compiler_error_object **pe_errorv; /* [1..1][0..ce_errorc|ALLOC(ce_errora)][owned] Vector of compiler errors. */
	struct Dee_compiler_error_object       *pe_master; /* [0..1][in(ce_errors)]
	                                                    * The current master error, or NULL when no
	                                                    * errors, or only warnings have been emit thus far. */
	uint16_t                                pe_except; /* Old exception recursion.
	                                                    * To allow for recursive parsers, as well as not throw a compiler error if
	                                                    * something went wrong during parsing, we keep track of the active exception
	                                                    * recursion recursion before compilation started.
	                                                    * Later, we compare the old recursion to the new and analyze all errors that occurred in-between.
	                                                    * Any object derived from `DeeError_CompilerError` is appended to `pe_errors`.
	                                                    * If after doing this, `pe_except` doesn't match the then active exception
	                                                    * recursion, all compiler errors are discarded before all errors except for
	                                                    * the first (at index `pe_except`) are discarded, while interrupts are re-scheduled.
	                                                    * This way, we can keep the regular exception system functioning like normal. */
};

INTDEF struct parser_errors current_parser_errors;
#define parser_errors_init(self) bzero(self, sizeof(struct parser_errors))
INTDEF void DCALL parser_errors_fini(struct parser_errors *__restrict self);

/* Invoke a user-defined compiler error handler and save the
 * given compiler error in `current_parser_errors` if necessary.
 * @return:  1: The error should cause the compiler to abort.
 *              It, as well as all other warnings/errors will be
 *              thrown the next time `parser_rethrow` is invoked.
 * @return:  0: Compilation can continue normally.
 * @return: -1: An error occurred and was thrown (using DeeError_Throw(); e.g.: `NoMemory()`). */
INTDEF WUNUSED NONNULL((1)) int DCALL parser_throw(struct Dee_compiler_error_object *__restrict error);

/* Check/pack/throw errors. What exactly is done
 * is documented in `parser_errors::pe_except`
 * @return: -1: Compilation has failed and the caller should discard
 *              whatever it is they thought to have retrieved as far
 *              as information goes.
 *              NOTE: Always returned when `must_fail` is true.
 * @return:  0: Compilation was successful. */
INTDEF int DCALL parser_rethrow(bool must_fail);

/* Save the current exception context in the current-parser-errors structure. */
INTDEF void DCALL parser_start(void);

/* Wrapper for sub-parser-error groups.
 * The main group is controlled by the active compiler. */
#define BEGIN_PARSER_CALLBACK()                                                     \
	do {                                                                            \
		struct parser_errors _old_errors;                                           \
		memcpy(&_old_errors, &current_parser_errors, sizeof(struct parser_errors)); \
		parser_errors_init(&current_parser_errors)
#define END_PARSER_CALLBACK()                                                       \
		parser_errors_fini(&current_parser_errors);                                 \
		memcpy(&current_parser_errors, &_old_errors, sizeof(struct parser_errors)); \
	}	__WHILE0


struct TPPFile;
struct Dee_compiler_error_loc {
	struct Dee_compiler_error_loc *cl_prev; /* [0..1][OVERRIDE(->cl_file, [1..1])]
	                                         * Calling compiler location (might be used
	                                         * when the parser was inside of a macro) */
	/*ref*/ struct TPPFile        *cl_file; /* [0..1] The file in which the error occurred
	                                         * (when `NULL`, no location information is available) */
	int                            cl_line; /* The line within `cl_file` (0-based) */
	int                            cl_col;  /* The column within that `cl_line` (0-based) */
};

typedef struct Dee_compiler_error_object {
	Dee_ERROR_OBJECT_HEAD
	Dee_WEAKREF_SUPPORT
	int                                           ce_mode;   /* Fatality mode (One of `COMPILER_ERROR_FATALITY_*`). */
	int                                           ce_wnum;   /* [const] The TPP-assigned warning ID of this error (One of `W_*`). */
	struct Dee_compiler_error_loc                 ce_locs;   /* [const] The parser location where the error occurred. */
	struct Dee_compiler_error_loc                *ce_loc;    /* [0..1][const] The main compiler location (that is the first text-file that can be encountered when walking `ce_locs`) */
	Dee_WEAKREF(struct Dee_compiler_error_object) ce_master; /* Weak reference to the master compiler error. */
	size_t                                        ce_errorc; /* [const] Number of contained compiler errors. */
	DREF struct Dee_compiler_error_object       **ce_errorv; /* [1..1][REF_IF(!= self)][const][0..ce_errorc][owned][const]
	                                                          * Vector of other errors/warnings that occurred, leading up to this one.
	                                                          * NOTE: The master compiler error (aka. `this` error) is
	                                                          *       the error that caused compilation to actually fail,
	                                                          *       meaning that it is the first matching error in the
	                                                          *       following list of conditions:
	                                                          *        - ce_mode == Dee_COMPILER_ERROR_FATALITY_FORCEFATAL
	                                                          *        - ce_mode == Dee_COMPILER_ERROR_FATALITY_FATAL
	                                                          *        - ce_mode == Dee_COMPILER_ERROR_FATALITY_ERROR */
} DeeCompilerErrorObject;

#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeCompilerError_Print(DeeObject *__restrict self,
                       Dee_formatprinter_t printer, void *arg);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeCompilerError_Print(self, printer, arg) \
	DeeObject_Print(self, printer, arg)
#endif /* !CONFIG_BUILDING_DEEMON */

DECL_END
#endif /* CONFIG_BUILDING_DEEMON */
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

#endif /* !GUARD_DEEMON_COMPILER_ERROR_H */
