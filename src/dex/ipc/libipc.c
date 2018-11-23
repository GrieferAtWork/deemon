/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_IPC_LIBIPC_C
#define GUARD_DEX_IPC_LIBIPC_C 1
#define _KOS_SOURCE 1

#include "libipc.h"

#include <deemon/api.h>
#include <deemon/dex.h>

DECL_BEGIN

PRIVATE struct dex_symbol symbols[] = {
    { "Process", (DeeObject *)&DeeProcess_Type },
    { "enumproc", (DeeObject *)&DeeProcEnum_Type },
    { "Pipe", (DeeObject *)&DeePipe_Type },
    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols      = */symbols,
    /* .d_init         = */NULL,
    /* .d_fini         = */NULL,
    /* .d_import_names = */{ NULL }
};

DECL_END

#ifndef __INTELLISENSE__
#ifdef CONFIG_HOST_WINDOWS
#   include "windows.c.inl"
#elif defined(CONFIG_HOST_UNIX)
#   include "unix.c.inl"
#else
#   include "generic.c.inl"
#endif
#endif

#endif /* !GUARD_DEX_IPC_LIBIPC_C */
