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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_H
#define GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_H 1

#include <deemon/api.h>

#include <deemon/string.h> /* Dee_string_utf */
#include <deemon/types.h>  /* Dee_OBJECT_HEAD, Dee_hash_t */

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Config option: how are latin-1 1-char strings cached? */
#if (!defined(CONFIG_STRING_LATIN1_STATIC) && \
     !defined(CONFIG_STRING_LATIN1_CACHED) && \
     !defined(CONFIG_STRING_LATIN1_NORMAL))
#if defined(CONFIG_TINY_DEEMON) && defined(__OPTIMIZE__)
#define CONFIG_STRING_LATIN1_CACHED /* Latin-1 characters are created dynamically, but then cached */
#elif defined(CONFIG_TINY_DEEMON)
#define CONFIG_STRING_LATIN1_NORMAL /* Latin-1 characters are created dynamically */
#else /* ... */
#define CONFIG_STRING_LATIN1_STATIC /* Statically define all latin-1 characters */
#endif /* !... */
#endif /* !CONFIG_STRING_LATIN1_... */

#ifdef CONFIG_STRING_LATIN1_STATIC
typedef struct {
	Dee_OBJECT_HEAD
	struct Dee_string_utf *s_data;
	Dee_hash_t             s_hash;
	size_t                 s_len;
	unsigned char          s_str[2];
} DeeStringObject1Char;

INTDEF DeeStringObject1Char DeeString_Latin1[256];
#endif /* CONFIG_STRING_LATIN1_STATIC */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_H */
