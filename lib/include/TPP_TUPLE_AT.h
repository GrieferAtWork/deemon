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
#pragma once
#ifndef TPP_TUPLE_AT
#include "__TPP_STDINC.h"

#define __TPP_PRIVATE_TUPLE_AT_0(i,first, ...) TPP_TUPLE_AT((__VA_ARGS__),i-1)
#define __TPP_PRIVATE_TUPLE_AT_1(i,first, ...) first
#define __TPP_PRIVATE_TUPLE_AT(i, ...)\
__TPP_BASIC_CAT(__TPP_PRIVATE_TUPLE_AT_,__TPP_EVAL((i) <= 0))(i,__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
// Returns the i'th element of "tpl"
// - Expands to nothing, if "i" is too large
// >> TPP_TUPLE_AT((10,20),0) // Expands to [10]
// >> TPP_TUPLE_AT((10,20),1) // Expands to [20]
// >> TPP_TUPLE_AT((10,20),2) // Expands to <nothing>
#define TPP_TUPLE_AT(tpl,i) __TPP_PRIVATE_TUPLE_AT(i,__TPP_FORCE_EXPAND tpl)

#endif /* !TPP_TUPLE_AT */
