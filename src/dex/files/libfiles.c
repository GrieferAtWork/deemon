/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FILES_LIBFILES_C
#define GUARD_DEX_FILES_LIBFILES_C 1
#define DEE_SOURCE

#include "libfiles.h"
/**/

#include <deemon/api.h>
#include <deemon/dex.h>

DECL_BEGIN

DEX_BEGIN
DEX_MEMBER_NODOC("Joined", &Joined_Type.ft_base),
/* TODO: DEX_MEMBER_NODOC("Decoder", &Decoder_Type.ft_base), */
/* TODO: DEX_MEMBER_NODOC("Printer", &Printer_Type.ft_base), */
/* ^ Used to easily construct a file that invokes a given callback for its write-callback:
 * >> local fp = files.Printer((data: Bytes) -> {
 * >>     print "Now printing:", repr data;
 * >> });
 * >> print fp: "Hello!";
 */
DEX_END(NULL, NULL, NULL);

DECL_END

#endif /* !GUARD_DEX_FILES_LIBFILES_C */
