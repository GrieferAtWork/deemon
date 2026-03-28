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
#ifndef GUARD_DEX_ICONV_ICONV_C
#define GUARD_DEX_ICONV_ICONV_C 1
#define CONFIG_BUILDING_LIBICONV
#define DEE_SOURCE

#include "iconv.h"
/**/

#include <deemon/api.h>
#include <deemon/system-features.h>

DECL_BEGIN

#ifndef CONFIG_HAVE_rawmemrchr
#define CONFIG_HAVE_rawmemrchr
#undef rawmemrchr
#define rawmemrchr dee_rawmemrchr
DeeSystem_DEFINE_rawmemrchr(dee_rawmemrchr)
#endif /* !CONFIG_HAVE_rawmemrchr */

#ifndef CONFIG_HAVE_fuzzy_memcmpl
#define CONFIG_HAVE_fuzzy_memcmpl
#undef fuzzy_memcmpl
#define fuzzy_memcmpl dee_fuzzy_memcmpl
INTERN ATTR_PURE WUNUSED ATTR_IN(1) ATTR_IN(3) size_t
NOTHROW_NCX(DCALL dee_fuzzy_memcmpl)(void const *s1, size_t s1_dwords,
                                     void const *s2, size_t s2_dwords) {
	size_t *v0, *v1, i, j, cost, temp;
	if unlikely(!s1_dwords)
		return s2_dwords;
	if unlikely(!s2_dwords)
		return s1_dwords;
	if (s2_dwords > s1_dwords) {
		{
			void const *xch_temp;
			xch_temp = s1;
			s1       = s2;
			s2       = xch_temp;
		}
		{
			size_t xch_temp;
			xch_temp  = s1_dwords;
			s1_dwords = s2_dwords;
			s2_dwords = xch_temp;
		}
	}
	Dee_TryMallocaNoFail(v0, (s2_dwords+1) * sizeof(size_t));
	Dee_TryMallocaNoFail(v1, (s2_dwords+1) * sizeof(size_t));
	for (i = 0; i < s2_dwords; ++i)
		v0[i] = i;
	for (i = 0; i < s1_dwords; ++i) {
		v1[0] = i + 1;
		for (j = 0; j < s2_dwords; j++) {
			cost = ((uint32_t const *)s1)[i] != ((uint32_t const *)s2)[j];
			cost += v0[j];
			temp  = v1[j] + 1;
			if (cost > temp)
				cost = temp;
			temp  = v0[j + 1] + 1;
			if (cost > temp)
				cost = temp;
			v1[j + 1] = cost;
		}
		memcpyc(v0, v1, s2_dwords, sizeof(size_t));
	}
	temp = v1[s2_dwords];
	Dee_Freea(v1);
	Dee_Freea(v0);
	return temp;
}
#endif /* !CONFIG_HAVE_fuzzy_memcmpl */

DECL_END

#undef DEFINE_PUBLIC_ALIAS
#define DEFINE_PUBLIC_ALIAS(new, old) /* Disable exports */

/* Extra dependencies */
/* clang-format off */
#include <deemon/util/atomic.h>
#include <deemon/util/kos-compat.h>
#include <hybrid/align.h>
#include <hybrid/byteswap.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

#ifndef CONFIG_HAVE_UNICODE_H
#include "kos-mbstate.c.inl"
#undef mbstate_init
#define mbstate_init libiconv_mbstate_init
#undef mbstate_isempty
#define mbstate_isempty libiconv_mbstate_isempty
#undef unicode_c8toc32
#define unicode_c8toc32 libiconv_unicode_c8toc32
#undef unicode_c8toc16
#define unicode_c8toc16 libiconv_unicode_c8toc16
#undef unicode_c16toc8
#define unicode_c16toc8 libiconv_unicode_c16toc8
#endif /* !CONFIG_HAVE_UNICODE_H */
/* clang-format on */

/* Include libiconv sources */
/* clang-format off */
#include "../../libiconv/convert.c"
#include "../../libiconv/convert-xml.c"
#include "../../libiconv/detect.c" /* fuzzy_memcmpl */
#include "../../libiconv/iconv.c"
#include "../../libiconv/transliterate.c"
#include "../../libiconv/mbcs/cp-mbcs.c"
#include "../../libiconv/stateful/cp-stateful.c"
/* clang-format on */

/* Restore normal binding macros */
#undef DEFINE_PUBLIC_ALIAS
#define DEFINE_PUBLIC_ALIAS __DEFINE_PUBLIC_ALIAS

#endif /* !GUARD_DEX_ICONV_ICONV_C */
