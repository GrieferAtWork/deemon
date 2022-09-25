/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FS_HOST_C
#define GUARD_DEX_FS_HOST_C 1
#define DEE_SOURCE

#include <deemon/file.h> /* DEESYSTEM_FILE_USE_* */

/* TODO: Select implementations on a per-function basis! */

/* Select the host-specific filesystem implementation. */

#ifdef DEESYSTEM_FILE_USE_WINDOWS
#include "windows.c.inl"
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
#include "unix.c.inl"
#endif /* DEESYSTEM_FILE_USE_UNIX */

#ifdef DEESYSTEM_FILE_USE_STDIO
#include "stdio.c.inl"
#endif /* DEESYSTEM_FILE_USE_STDIO */

#ifdef DEESYSTEM_FILE_USE_STUB
#include "generic.c.inl"
#endif /* DEESYSTEM_FILE_USE_STUB */

#endif /* !GUARD_DEX_FS_HOST_C */
