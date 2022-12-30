/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_REGEX_H
#define GUARD_DEEMON_REGEX_H 1

#include "api.h"

#include "object.h"

DECL_BEGIN

/************************************************************************/
/* Regex compile                                                        */
/************************************************************************/

struct DeeRegexCode; /* Anonymous; is actually libregex's `struct re_code' */

/* Possible flags for `DeeString_GetRegex()' */
#define DEE_REGEX_COMPILE_NORMAL 0x0000 /* Normal regex compiler flags */
#define DEE_REGEX_COMPILE_ICASE  0x0001 /* Produce a case-insensitive pattern */
#define DEE_REGEX_COMPILE_NOUTF8 0x0002 /* Disable utf-8 processing; pattern is parsed and matched as byte-only */

/* Lazily compile `self' as a deemon regex pattern.
 * Regex patterns for strings are compiled once, and cached thereafter,
 * before being destroyed at the same time as the corresponding string.
 * @param: compile_flags: Set of `DEE_REGEX_COMPILE_*'
 * @return: * :   The compiled regex pattern.
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) struct DeeRegexCode *DCALL
DeeString_GetRegex(DeeObject *__restrict self, unsigned int compile_flags);



/************************************************************************/
/* Regex execute                                                        */
/************************************************************************/

struct DeeRegexMatch {
	size_t rm_so; /* [<= rm_eo] Group starting offset (offset of first byte within the group)
	               * - Set to `(size_t)-1' if the group was never encountered. */
	size_t rm_eo; /* [>= rm_so] Group end offset (offset of first byte past the group)
	               * - Set to `(size_t)-1' if the group was never encountered. */
};

/* Flags for `struct DeeRegexExec::rx_eflags' */
#define DEE_RE_EXEC_NOTBOL 0x0001 /* '^' (REOP_AT_SOL) doesn't match at the start of the input buffer (but only at an actual begin-of-line) */
#define DEE_RE_EXEC_NOTEOL 0x0002 /* '$' (REOP_AT_EOL) doesn't match at the end of the input buffer (but only before an actual line-feed) */

struct DeeRegexExec {
	struct DeeRegexCode const *rx_code;     /* [1..1] Regex code */
	size_t                     rx_nmatch;   /* Max # of group matches to write to `rx_pmatch' (at most `rx_code->rc_ngrps' will ever be written) */
	struct DeeRegexMatch      *rx_pmatch;   /* [?..rx_nmatch] Output buffer for group matches
	                                         * - Up to the first `rx_nmatch' groups are written, but only on success
	                                         * - Upon failure, the contents of this buffer are left in an undefined state
	                                         * - Offsets written INCLUDE `rx_startoff' (i.e. are always `>= rx_startoff') */
	void const                *rx_inbase;   /* [0..rx_insize][valid_if(rx_startoff < rx_endoff)] Input data to scan
	                                         * When `rx_code' was compiled with `DEE_REGEX_COMPILE_NOUTF8', this data
	                                         * is treated as raw bytes; otherwise, it is treated as a utf-8 string.
	                                         * In either case, `rx_insize' is the # of bytes within this buffer. */
	size_t                     rx_insize;   /* Total # of bytes starting at `rx_inbase' */
	size_t                     rx_startoff; /* Starting byte offset into `rx_inbase' of data to match. */
	size_t                     rx_endoff;   /* Ending byte offset into `rx_inbase' of data to match. */
	unsigned int               rx_eflags;   /* Execution-flags (set of `DEE_RE_EXEC_*') */
};

/* Special return values for functions below. */
#define DEE_RE_STATUS_NOMATCH (-1) /* Nothing was matched */
#define DEE_RE_STATUS_ERROR   (-2) /* An error occurred */


/* Perform a regex match
 * @return: >= 0: The # of matched bytes starting at `exec->rx_startoff'
 * @return: DEE_RE_STATUS_NOMATCH: Nothing was matched
 * @return: DEE_RE_STATUS_ERROR:   An error occurred */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeRegex_Match(struct DeeRegexExec const *__restrict exec);

/* Similar to `DeeRegex_Match', try to match a pattern against the given input buffer. Do this
 * with increasing offsets for the first `search_range' bytes, meaning at most `search_range'
 * regex matches will be performed.
 * @param: search_range: One plus the max starting  byte offset (from `exec->rx_startoff')  to
 *                       check. Too great values for `search_range' are automatically clamped.
 * @param: p_match_size: When non-NULL, set to the # of bytes that were actually matched.
 *                       This would have  been the return  value of  `re_exec_match(3R)'.
 * @return: >= 0:        The offset where the matched area starts (in `[exec->rx_startoff, exec->rx_startoff + search_range)').
 * @return: DEE_RE_STATUS_NOMATCH: Nothing was matched
 * @return: DEE_RE_STATUS_ERROR:   An error occurred */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeRegex_Search(struct DeeRegexExec const *__restrict exec,
                size_t search_range, size_t *p_match_size);

/* Same as `DeeRegex_Search', but perform searching with starting
 * offsets in `[exec->rx_endoff - search_range, exec->rx_endoff)'
 * Too great values for `search_range' are automatically clamped.
 * The return value will thus be the greatest byte-offset where
 * the given pattern matches that is still within that range. */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t
DeeRegex_RSearch(struct DeeRegexExec const *__restrict exec,
                 size_t search_range, size_t *p_match_size);


DECL_END

#endif /* !GUARD_DEEMON_REGEX_H */
