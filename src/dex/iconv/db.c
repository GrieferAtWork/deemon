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
#ifndef GUARD_DEX_ICONV_DB_C
#define GUARD_DEX_ICONV_DB_C 1
#define CONFIG_BUILDING_LIBICONV
#define DEE_SOURCE

#include "iconv.h"
/**/

#include <deemon/api.h>

#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* DeeSystem_DEFINE_rawmemrchr */

DECL_BEGIN

#ifndef CONFIG_HAVE_rawmemrchr
#define CONFIG_HAVE_rawmemrchr
#undef rawmemrchr
#define rawmemrchr dee_rawmemrchr
DeeSystem_DEFINE_rawmemrchr(dee_rawmemrchr)
#endif /* !CONFIG_HAVE_rawmemrchr */

DECL_END

#undef DEFINE_PUBLIC_ALIAS
#define DEFINE_PUBLIC_ALIAS(new, old) /* Disable exports */

/* Include libiconv sources */
/* clang-format off */
#include "../../libiconv/codecs.c"
#include "../../libiconv/cp.c"
#include "../../libiconv/cp-7h.c"
#include "../../libiconv/cp-7l.c"
#include "../../libiconv/cp-iso646.c"
#include "../../libiconv/mbcs/cpdb.c"
#include "../../libiconv/stateful/cpdb.c"
/* clang-format on */

DECL_BEGIN

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
do_iconv_normalize_codec_name(DeeStringObject *__restrict str) {
	char buf[CODE_NAME_MAXLEN + 1];
	if (!libiconv_normalize_codec_name(buf, DeeString_STR(str), DeeString_SIZE(str)))
		return_none;
	return DeeString_New(buf);
}

DECL_END

#endif /* !GUARD_DEX_ICONV_DB_C */
