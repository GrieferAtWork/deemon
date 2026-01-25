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
#ifndef GUARD_DEX_WIN32_LIBWIN32_H
#define GUARD_DEX_WIN32_LIBWIN32_H 1

#include <deemon/api.h>
#ifdef CONFIG_HOST_WINDOWS

#include <Windows.h>

DECL_BEGIN

#if 0 /* TODO */
typedef struct nt_reg_key NTRegKeyObject;

struct nt_reg_key {
	OBJECT_HEAD
	HKEY                  rk_key;  /* [0..1][lock(rk_lock)] A handle for the key being accessed.
	                                * When `NULL', the handle must lazily be allocated when it is first needed. */
	DREF DeeStringObject *rk_name; /* [1..1][const] Name of the key, possibly in relation to `rk_rel'. */
	DREF NTRegKeyObject  *rk_rel;  /* [0..1][const] Underlying base-key, describing where `rk_name' originates from. */
};
#endif


DECL_END
#endif /* CONFIG_HOST_WINDOWS */

#endif /* !GUARD_DEX_WIN32_LIBWIN32_H */
