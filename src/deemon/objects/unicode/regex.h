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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REGEX_H
#define GUARD_DEEMON_OBJECTS_UNICODE_REGEX_H 1

#include <deemon/api.h>

#include <deemon/regex.h>  /* DeeRegexCode, DeeRegexExec */
#include <deemon/string.h> /* DeeStringObject */
#include <deemon/types.h>  /* DREF */

#include <stddef.h> /* size_t */

DECL_BEGIN

struct DeeRegexBaseExec {
	DREF DeeStringObject      *rx_pattern;  /* [1..1] Pattern string (only a reference within objects in "./reproxy.c.inl") */
	struct DeeRegexCode const *rx_code;     /* [1..1] Regex code */
	void const                *rx_inbase;   /* [0..rx_insize][valid_if(rx_startoff < rx_endoff)] Input data to scan
	                                         * When `rx_code' was compiled with `Dee_RE_COMPILE_NOUTF8', this data
	                                         * is treated as raw bytes; otherwise, it is treated as a utf-8 string.
	                                         * In either case, `rx_insize' is the # of bytes within this buffer. */
	size_t                     rx_insize;   /* Total # of bytes starting at `rx_inbase' */
	size_t                     rx_startoff; /* Starting byte offset into `rx_inbase' of data to match. */
	size_t                     rx_endoff;   /* Ending byte offset into `rx_inbase' of data to match. */
	unsigned int               rx_eflags;   /* Execution-flags (set of `Dee_RE_EXEC_*') */
};

struct DeeRegexExecWithRange {
	struct DeeRegexExec rewr_exec;    /* Normal exec args */
	size_t              rewr_range;   /* Max # of search attempts to perform (in bytes) */
	DeeStringObject    *rewr_pattern; /* [1..1] Pattern string that is being used */
	DeeStringObject    *rewr_rules;   /* [0..1] Pattern rules */
};

/* Destroy the regex cache associated with `self'.
 * Called from `DeeString_Type.tp_fini' when `Dee_STRING_UTF_FFINIHOOK' was set. */
INTDEF NONNULL((1)) void DCALL
DeeString_DestroyRegex(DeeStringObject const *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGEX_H */
