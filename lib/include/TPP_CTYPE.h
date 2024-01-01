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
#pragma once

#include "__TPP_STDINC.h"
#if __has_feature(__tpp_pragma_tpp_set_keyword_flags__)
// Enable the macros below as builtins
// >> __has_builtin(__builtin_tpp_isalnum) >> 1
#pragma tpp_set_keyword_flags("builtin_tpp_isalnum",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isalpha",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isblank",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_iscntrl",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isdigit",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isgraph",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_islower",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isprint",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_ispunct",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isspace",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isupper",0x02)
#pragma tpp_set_keyword_flags("builtin_tpp_isxdigit",0x02)
#endif

// Builtin character function equivalents from <ctype.h>
#define __builtin_tpp_isalnum(c) __TPP_EVAL(\
((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || \
((c) >= '0' && (c) <= '9') || ((c) == '_'))
#define __builtin_tpp_isalpha(c)  __TPP_EVAL(\
((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || ((c) == '_'))
#define __builtin_tpp_isblank(c)  __TPP_EVAL((c) == '\x09' || (c) == '\x20')
#define __builtin_tpp_iscntrl(c)  __TPP_EVAL((c) <= '\x1F' || (c) == '\x7F')
#define __builtin_tpp_isdigit(c)  __TPP_EVAL((c) >= '0' && (c) <= '9')
#define __builtin_tpp_isgraph(c)  __TPP_EVAL((c) >= '\x21' && (c) <= '\x7E')
#define __builtin_tpp_islower(c)  __TPP_EVAL((c) >= 'a' && (c) <= 'z')
#define __builtin_tpp_isprint(c)  __TPP_EVAL((c) >= '\x20' && (c) <= '\x7E')
#define __builtin_tpp_ispunct(c)  __TPP_EVAL(\
((c) >= '\x21' && (c) <= '\x2F') ||/* !"#$%&'()*+,-./ */\
((c) >= '\x3A' && (c) <= '\x40') ||/* :;<=>?@ */\
((c) >= '\x5C' && (c) <= '\x60') ||/* [\]^_` */\
((c) >= '\x7B' && (c) <= '\x7E')   /* {|}~ */)
#define __builtin_tpp_isspace(c)  __TPP_EVAL(\
((c) >= '\x09' && (c) <= '\x0D') || (c) == '\x20')
#define __builtin_tpp_isupper(c)  __TPP_EVAL((c) >= 'A' && (c) <= 'Z')
#define __builtin_tpp_isxdigit(c) __TPP_EVAL(\
((c) >= '0' && (c) <= '9') || \
((c) >= 'A' && (c) <= 'F') || \
((c) >= 'a' && (c) <= 'f'))

#define __builtin_tpp_tolower(c) __TPP_EVAL(\
((c) >= 'A' && (c) <= 'Z') ? ((c)-('A'-'a')) : (c))
#define __builtin_tpp_toupper(c) __TPP_EVAL(\
((c) >= 'a' && (c) <= 'z') ? ((c)-('a'-'A')) : (c))
#define __builtin_tpp_swapcase(c) __TPP_EVAL(\
__builtin_tpp_islower(c)?__builtin_tpp_toupper(c):__builtin_tpp_tolower(c))


