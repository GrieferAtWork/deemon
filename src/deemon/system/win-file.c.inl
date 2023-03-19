/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_SYSTEM_WIN_FILE_C_INL
#define GUARD_DEEMON_SYSTEM_WIN_FILE_C_INL 1

#include <Windows.h> /* _MUST_ be included first! */
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <hybrid/byteorder.h>
#include <hybrid/host.h>
#include <hybrid/minmax.h>
#include <hybrid/unaligned.h>

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */

#ifndef UINT32_MAX
#include <hybrid/limitcore.h>
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */

DECL_BEGIN

typedef DeeSystemFileObject SystemFile;

DECL_END

#endif /* !GUARD_DEEMON_SYSTEM_WIN_FILE_C_INL */
