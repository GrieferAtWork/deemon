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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_H
#define GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */
#include <deemon/string.h> /* DeeStringObject, Dee_charptr_const */

#include "../generic-proxy.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeStringObject, s_str, /* [1..1][const] The string that is being split. */
	                      DeeStringObject, s_sep) /* [1..1][const][!DeeString_IsEmpty] The string to search for. */
} StringSplit;

typedef struct {
	PROXY_OBJECT_HEAD_EX(StringSplit, s_split) /* [1..1][const] The split descriptor object. */
	union Dee_charptr_const           s_next;  /* [0..1][atomic] Pointer to the starting address of the next split
	                                            *                (points into the s_enc-specific string of `s_split->s_str')
	                                            *                When the iterator is exhausted, this pointer is set to `NULL'. */
	union Dee_charptr_const           s_start; /* [1..1][const] The starting address of the width string of `s_split->s_str'. */
	union Dee_charptr_const           s_end;   /* [1..1][const] The end address of the width string of `s_split->s_str'. */
	union Dee_charptr_const           s_sep;   /* [1..1][const] The starting address of the `s_enc'-encoded string of `s_split->s_sep'. */
	size_t                            s_sepsz; /* [1..1][const][== WSTR_LENGTH(s_sep)] The length of separator string. */
	int                               s_width; /* [const] The width of `s_split->s_str' */
} StringSplitIterator;

INTDEF DeeTypeObject StringSplit_Type;
INTDEF DeeTypeObject StringSplitIterator_Type;
INTDEF DeeTypeObject StringCaseSplit_Type;
INTDEF DeeTypeObject StringCaseSplitIterator_Type;

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Split(DeeStringObject *self,
                DeeStringObject *separator);

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_CaseSplit(DeeStringObject *self,
                    DeeStringObject *separator);




/************************************************************************/
/* LINE SPLIT                                                           */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeStringObject, ls_str)  /* [1..1][const] The string that is being split into lines. */
	bool                                  ls_keep; /* [const] True if line-ends should be kept in resulting strings. */
} LineSplit;

typedef struct {
	PROXY_OBJECT_HEAD_EX(LineSplit, ls_split) /* [1..1][const] The split descriptor object. */
	union Dee_charptr_const         ls_next;  /* [0..1][atomic] Pointer to the starting address of the next split
	                                           *                (points into the s_enc-specific string of `ls_split->ls_str')
	                                           *                When the iterator is exhausted, this pointer is set to NULL. */
	union Dee_charptr_const         ls_begin; /* [1..1][const] The starting address of the width string of `ls_split->ls_str'. */
	union Dee_charptr_const         ls_end;   /* [1..1][const] The end address of the width string of `ls_split->ls_str'. */
	int                             ls_width; /* [const] The width of `ls_split->ls_str' */
	bool                            ls_keep;  /* [const] True if line-ends should be kept in resulting strings. */
} LineSplitIterator;

INTDEF DeeTypeObject StringLineSplit_Type;
INTDEF DeeTypeObject StringLineSplitIterator_Type;

/* @return: An abstract sequence type for enumerating
 *          the segments of a string split into lines. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_SplitLines(DeeStringObject *__restrict self, bool keepends);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_H */
