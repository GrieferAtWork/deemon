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
#pragma once
#include "__cdefault.dee"

#ifndef offsetof
#define offsetof(t, m) size_t((t).ptr(none).deref.m.ref)
#endif

#pragma push_macro(undef, "import", "from", "ctypes", "type", "none")
#ifndef __ptrdiff_t_defined
#define __ptrdiff_t_defined 1
import ptrdiff_t = "ptrdiff_t" from ctypes;
#endif
#ifndef __size_t_defined
#define __size_t_defined 1
import size_t = "size_t" from ctypes;
#endif
#ifndef __ssize_t_defined
#define __ssize_t_defined 1
import ssize_t = "ssize_t" from ctypes;
#endif
#ifndef __nullptr_t_defined
#define __nullptr_t_defined 1
local nullptr_t = type none;
#endif
#pragma pop_macro("import", "from", "ctypes", "type", "none")

#ifndef NULL
#define NULL none
#endif
