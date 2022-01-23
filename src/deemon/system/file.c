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
#ifndef GUARD_DEEMON_SYSTEM_FILE_C
#define GUARD_DEEMON_SYSTEM_FILE_C 1

#include <deemon/api.h>
#include <deemon/filetypes.h>

#ifdef DEESYSTEM_FILE_USE_WINDOWS
#include "win-file.c.inl"
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
#include "unix-file.c.inl"
#endif /* DEESYSTEM_FILE_USE_UNIX */

#ifdef DEESYSTEM_FILE_USE_STDIO
#include "stdio-file.c.inl"
#endif /* DEESYSTEM_FILE_USE_STDIO */

#ifdef DEESYSTEM_FILE_USE_STUB
#include "generic-file.c.inl"
#endif /* DEESYSTEM_FILE_USE_STUB */

#endif /* !GUARD_DEEMON_SYSTEM_FILE_C */
