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
#ifndef GUARD_DEX_ICONV_ICONV_H
#define GUARD_DEX_ICONV_ICONV_H 1

#include <deemon/api.h>
/**/

/* Configure libiconv */
#define LIBICONV_DECL INTDEF
#define LIBICONV_CC   DFCALL
#undef LIBICONV_WANT_PROTOTYPES
#undef __FCALL
#undef FCALL
#define __FCALL DFCALL
#define FCALL   DFCALL
#undef LIBICONV_SETERRNO
#define LIBICONV_SETERRNO(v) (void)0

#define LIBICONV_NO_SYSTEM_INCLUDES
#define LIBICONV_SINGLE_LIBRARY
#define LIBICONV_EXPOSE_INTERNAL
#define LIBICONV_NO_ICONV_ERR_ERRNO
#define ICONV_ERR_ERROR   0
#define ICONV_ERR_DISCARD 1
#define ICONV_ERR_REPLACE 2
#define ICONV_ERR_IGNORE  3

/* clang-format off */
/* Enable KOS compatibility */
#include <deemon/util/kos-compat.h>
/* Some misc. headers needed by libiconv */
#include <hybrid/host.h>
#include <hybrid/typecore.h>
/* clang-format on */

/* Emulation of KOS's mbstate API */
#ifndef CONFIG_HAVE_UNICODE_H
#include "kos-mbstate.h"
#undef __mbstate
#undef mbstate_t
#define __mbstate libiconv_mbstate
#define mbstate_t struct libiconv_mbstate
#endif /* !CONFIG_HAVE_UNICODE_H */

/* Include libiconv headers */
/* clang-format off */
#include "../../libiconv/include/api.h"
#include "../../libiconv/include/codec.h"
#include "../../libiconv/include/iconv.h"
#include "../../libiconv/include/transliterate.h"

#include "../../libiconv/api.h"
#include "../../libiconv/codecs.h"
#include "../../libiconv/convert.h"
#include "../../libiconv/detect.h"
#include "../../libiconv/iconv.h"
#include "../../libiconv/transliterate.h"
#include "../../libiconv/mbcs/cp-mbcs.h"
#include "../../libiconv/stateful/cp-stateful.h"
/* clang-format on */

#endif /* !GUARD_DEX_ICONV_ICONV_H */
