/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_BSEARCH_C
#define GUARD_DEEMON_OBJECTS_SEQ_BSEARCH_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/object.h>

#ifndef CHAR_BIT
#include <hybrid/typecore.h>
#define CHAR_BIT __CHAR_BIT__
#endif /* !CHAR_BIT */

DECL_BEGIN

INTDEF ATTR_COLD int DCALL
err_no_generic_sequence(DeeObject *__restrict self);

DECL_END

#ifndef __INTELLISENSE__
#define DEFINE_DeeSeq_BFind
#include "bsearch.c.inl"
#define DEFINE_DeeSeq_BFindRange
#include "bsearch.c.inl"
#define DEFINE_DeeSeq_BFindPosition
#include "bsearch.c.inl"
#define DEFINE_DeeSeq_BLocate
#include "bsearch.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_BSEARCH_C */
