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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_H
#define GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */
#include <deemon/string.h> /* DeeStringObject, Dee_charptr_const */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	/* A proxy object for viewing the characters of a string as an array of unsigned
	 * integers representing the unicode character codes for each character. */
	PROXY_OBJECT_HEAD_EX(DeeStringObject, so_str)   /* [1..1][const] The string who's character ordinals are being viewed. */
	unsigned int                          so_width; /* [const][== DeeString_WIDTH(so_str)] The string's character width. */
	union Dee_charptr_const               so_ptr;   /* [const][== DeeString_WSTR(so_str)] The effective character array. */
} StringOrdinals;

INTDEF DeeTypeObject StringOrdinals_Type;

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Ordinals(DeeStringObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_H */
