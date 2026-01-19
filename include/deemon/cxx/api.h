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
#ifndef GUARD_DEEMON_CXX_API_H
#define GUARD_DEEMON_CXX_API_H 1

#include "../api.h"

#if !defined(__cplusplus) && !defined(__DEEMON__)
#error "C++ only API. Use the base C api instead"
#endif /* !__cplusplus && !__DEEMON__ */

#include <__stdcxx.h>

#define DEE_CXX_BEGIN   __CXXDECL_BEGIN namespace deemon {
#define DEE_CXX_END     } __CXXDECL_END
#ifdef __COMPILER_HAVE_CXX11_NOEXCEPT
#define DEE_CXX_NOTHROW noexcept
#else /* __COMPILER_HAVE_CXX11_NOEXCEPT */
#define DEE_CXX_NOTHROW throw()
#endif /* !__COMPILER_HAVE_CXX11_NOEXCEPT */

#endif /* !GUARD_DEEMON_CXX_API_H */
