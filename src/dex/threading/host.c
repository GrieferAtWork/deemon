/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEX_THREADING_HOST_C
#define GUARD_DEX_THREADING_HOST_C 1
#define CONFIG_BUILDING_LIBTHREADING 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include <deemon/api.h>

#ifdef CONFIG_NO_THREADS
#include "nothread.c.inl"
#elif defined(CONFIG_HOST_WINDOWS)
#include "windows.c.inl"
#elif defined(CONFIG_HOST_UNIX)
#include "unix.c.inl"
#elif 1
/* -> The nothread variant uses atomics to try and implement thread support */
#include "nothread.c.inl"
#else
/* XXX: c++11 standard headers? */
/* XXX: atomic-only implementation? */
#error "No libthreading support available"
#endif

#endif /* !GUARD_DEX_THREADING_HOST_C */
