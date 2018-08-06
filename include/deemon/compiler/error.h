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
#ifndef GUARD_DEEMON_COMPILER_ERROR_H
#define GUARD_DEEMON_COMPILER_ERROR_H 1

#include "../api.h"

#define PARSE_FNORMAL 0x0000 /* Normal parser flags. */

#ifdef CONFIG_BUILDING_DEEMON
#include "../thread.h"

#include <string.h>

DECL_BEGIN

struct compiler_error_object;
struct parser_errors {
    size_t                              pe_errora; /* Allocated vector size. */
    size_t                              pe_errorc; /* Total amount of errors/warnings. */
    DREF struct compiler_error_object **pe_errorv; /* [1..1][0..ce_errorc|ALLOC(ce_errora)][owned] Vector of compiler errors. */
    struct compiler_error_object       *pe_master; /* [0..1][in(ce_errors)]
                                                    * The current master error, or NULL when no
                                                    * errors, or only warnings have been emit thus far. */
    uint16_t                            pe_except; /* Old exception recursion.
                                                    * To allow for recursive parsers, as well as not throw a compiler error if
                                                    * something went wrong during parsing, we keep track of the active exception
                                                    * recursion recursion before compilation started.
                                                    * Later, we compare the old recursion to the new and analyze all errors that occurred in-between.
                                                    * Any object derived from `DeeError_CompilerError' is appended to `pe_errors'.
                                                    * If after doing this, `pe_except' doesn't match the then active exception
                                                    * recursion, all compiler errors are discarded before all errors except for
                                                    * the first (at index `pe_except') are discarded, while interrupts are re-scheduled.
                                                    * This way, we can keep the regular exception system functioning like normal. */
};

INTDEF struct parser_errors current_parser_errors;
#define parser_errors_init(self) memset(self,0,sizeof(struct parser_errors))
INTDEF void DCALL parser_errors_fini(struct parser_errors *__restrict self);

/* Invoke a user-defined compiler error handler and save the
 * given compiler error in `current_parser_errors' if necessary.
 * @return:  1: The error should cause the compiler to abort.
 *              It, as well as all other warnings/errors will be
 *              thrown the next time `parser_rethrow' is invoked.
 * @return:  0: Compilation can continue normally.
 * @return: -1: An error occurred and was thrown (using DeeError_Throw(); e.g.: `NoMemory()'). */
INTDEF int DCALL parser_throw(struct compiler_error_object *__restrict error);

/* Check/pack/throw errors. What exactly is done
 * is documented in `parser_errors::pe_except'
 * @return: -1: Compilation has failed and the caller should discard
 *              whatever it is they thought to have retrieved as far
 *              as information goes.
 *              NOTE: Always returned when `must_fail' is true.
 * @return:  0: Compilation was successful. */
INTDEF int DCALL parser_rethrow(bool must_fail);

/* Save the current exception context in the current-parser-errors structure. */
INTDEF void DCALL parser_start(void);

/* Wrapper for sub-parser-error groups.
 * The main group is controlled by the active compiler. */
#define BEGIN_PARSER_CALLBACK() \
  do{ struct parser_errors _old_errors; \
      memcpy(&_old_errors,&current_parser_errors,sizeof(struct parser_errors)); \
      parser_errors_init(&current_parser_errors)
#define END_PARSER_CALLBACK() \
      parser_errors_fini(&current_parser_errors); \
      memcpy(&current_parser_errors,&_old_errors,sizeof(struct parser_errors)); \
  }__WHILE0

DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_ERROR_H */
