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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_CFORMAT_H
#define GUARD_DEEMON_OBJECTS_UNICODE_CFORMAT_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DeeObject, Dee_formatprinter_t, Dee_ssize_t */

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Format a given `format' string subject to printf-style formatting rules.
 * NOTE: This is the function called by `operator %' for strings. */
INTDEF WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeString_CFormat(Dee_formatprinter_t printer,
                  Dee_formatprinter_t format_printer, void *arg,
                  /*utf-8*/ char const *__restrict format, size_t format_len,
                  size_t argc, DeeObject *const *argv);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_CFORMAT_H */
