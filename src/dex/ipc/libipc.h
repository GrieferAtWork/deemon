/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_IPC_LIBIPC_H
#define GUARD_DEX_IPC_LIBIPC_H 1

#include <deemon/api.h>

#include <deemon/dex.h>
#include <deemon/file.h>
#include <deemon/object.h>

DECL_BEGIN

INTDEF DeeFileTypeObject DeePipe_Type; /* Extends `_SystemFile' */
INTDEF DeeFileTypeObject DeePipeReader_Type;
INTDEF DeeFileTypeObject DeePipeWriter_Type;
INTDEF DeeTypeObject DeeProcess_Type;

/* On-demand functions (Need to `#define WANT_<name-of-function>') */
INTDEF WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
DeeObject_AsFileSystemPathString(DeeObject *__restrict self);

INTDEF ATTR_COLD int DCALL ipc_unimplemented(void);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_attribute_string(DeeTypeObject *__restrict tp, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_file_not_found_string(char const *__restrict filename);

DECL_END

#endif /* !GUARD_DEX_IPC_LIBIPC_H */
