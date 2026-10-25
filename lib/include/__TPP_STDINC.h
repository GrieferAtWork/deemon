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
#ifndef __TPP_VERSION__
#error This header library is only meant to be used with TPP!
#endif
#ifndef __TPP_EVAL
#error This library requires, that tpp was compiled \
       with at least "TPP_CONFIG_HAVE___TPP_EVAL" defined
#endif /* !__TPP_EVAL */
#define __TPP_BASIC_CAT2(a,b)         a ## b
#define __TPP_BASIC_CAT(a,b)          __TPP_BASIC_CAT2(a,b)
#define __TPP_FORCE_EXPAND(...)       __VA_ARGS__
#define __TPP_BASIC_EXPAND_TUPLE(tpl) __TPP_FORCE_EXPAND tpl
#define __TPP_EAT_ARGS(...)           /* nothing */
#define __TPP_SUPPRESS_WARNING(id)    __pragma(warning(suppress: id))

//////////////////////////////////////////////////////////////////////////
// Returns the string representation of __VA_ARGS__
// >> TPP_STR(foobar) // Expands to ["foobar"]
#define TPP_STR(...)  TPP_STR_NOEXPAND(__VA_ARGS__)
#define TPP_STR_NOEXPAND(...) #__VA_ARGS__
