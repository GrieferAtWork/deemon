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
#ifndef GUARD_DEEMON_NONE_OPERATOR_H
#define GUARD_DEEMON_NONE_OPERATOR_H 1

#include "api.h"
/**/

#include "types.h"

DECL_BEGIN

#ifdef DCALL_CALLER_CLEANUP
DECL_END
#include "none.h"
DECL_BEGIN
#define _DeeNone_NewRef1 (*(DREF DeeObject *(DCALL *)(void *))(Dee_funptr_t)&DeeNone_NewRef)
#define _DeeNone_NewRef2 (*(DREF DeeObject *(DCALL *)(void *, void *))(Dee_funptr_t)&DeeNone_NewRef)
#define _DeeNone_NewRef3 (*(DREF DeeObject *(DCALL *)(void *, void *, void *))(Dee_funptr_t)&DeeNone_NewRef)
#define _DeeNone_NewRef4 (*(DREF DeeObject *(DCALL *)(void *, void *, void *, void *))(Dee_funptr_t)&DeeNone_NewRef)
#define _DeeNone_NewRef5 (*(DREF DeeObject *(DCALL *)(void *, void *, void *, void *, void *))(Dee_funptr_t)&DeeNone_NewRef)
#define _DeeNone_NewRef6 (*(DREF DeeObject *(DCALL *)(void *, void *, void *, void *, void *, void *))(Dee_funptr_t)&DeeNone_NewRef)
#define _DeeNone_NewRef7 (*(DREF DeeObject *(DCALL *)(void *, void *, void *, void *, void *, void *, void *))(Dee_funptr_t)&DeeNone_NewRef)
DFUNDEF size_t (DCALL _DeeNone_rets0)(void); /* Always returns "0" */
DFUNDEF size_t (DCALL _DeeNone_rets1)(void); /* Always returns "1" */
DFUNDEF size_t (DCALL _DeeNone_retsm1)(void); /* Always returns "(size_t)-1" */
#if defined(DCALL_RETURN_COMMON) || __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define _DeeNone_reti0 (*(int (DCALL *)(void))&_DeeNone_rets0)
#define _DeeNone_reti1 (*(int (DCALL *)(void))&_DeeNone_rets1)
#else /* DCALL_RETURN_COMMON || __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
DFUNDEF int (DCALL _DeeNone_reti0)(void); /* Always returns "0" */
DFUNDEF int (DCALL _DeeNone_reti1)(void); /* Always returns "1" */
#endif /* !DCALL_RETURN_COMMON && __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */
#define _DeeNone_reti0_0  (*(int (DCALL *)(void))&_DeeNone_reti0)
#define _DeeNone_reti0_1  (*(int (DCALL *)(void *))&_DeeNone_reti0)
#define _DeeNone_reti0_2  (*(int (DCALL *)(void *, void *))&_DeeNone_reti0)
#define _DeeNone_reti0_3  (*(int (DCALL *)(void *, void *, void *))&_DeeNone_reti0)
#define _DeeNone_reti0_4  (*(int (DCALL *)(void *, void *, void *, void *))&_DeeNone_reti0)
#define _DeeNone_reti0_5  (*(int (DCALL *)(void *, void *, void *, void *, void *))&_DeeNone_reti0)
#define _DeeNone_reti0_6  (*(int (DCALL *)(void *, void *, void *, void *, void *, void *))&_DeeNone_reti0)
#define _DeeNone_reti1_1  (*(int (DCALL *)(void *))&_DeeNone_reti1)
#define _DeeNone_reti1_2  (*(int (DCALL *)(void *, void *))&_DeeNone_reti1)
#define _DeeNone_reti1_3  (*(int (DCALL *)(void *, void *, void *))&_DeeNone_reti1)
#define _DeeNone_reti1_4  (*(int (DCALL *)(void *, void *, void *, void *))&_DeeNone_reti1)
#define _DeeNone_rets0_1  (*(size_t (DCALL *)(void *))&_DeeNone_rets0)
#define _DeeNone_rets0_2  (*(size_t (DCALL *)(void *, void *))&_DeeNone_rets0)
#define _DeeNone_rets0_3  (*(size_t (DCALL *)(void *, void *, void *))&_DeeNone_rets0)
#define _DeeNone_rets0_4  (*(size_t (DCALL *)(void *, void *, void *, void *))&_DeeNone_rets0)
#define _DeeNone_rets0_5  (*(size_t (DCALL *)(void *, void *, void *, void *, void *))&_DeeNone_rets0)
#define _DeeNone_rets0_6  (*(size_t (DCALL *)(void *, void *, void *, void *, void *, void *))&_DeeNone_rets0)
#define _DeeNone_retsm1_1 (*(size_t (DCALL *)(void *))&_DeeNone_retsm1)
#define _DeeNone_retsm1_2 (*(size_t (DCALL *)(void *, void *))&_DeeNone_retsm1)
#define _DeeNone_retsm1_3 (*(size_t (DCALL *)(void *, void *, void *))&_DeeNone_retsm1)
#define _DeeNone_retsm1_4 (*(size_t (DCALL *)(void *, void *, void *, void *))&_DeeNone_retsm1)
#define _DeeNone_retsm1_5 (*(size_t (DCALL *)(void *, void *, void *, void *, void *))&_DeeNone_retsm1)
#define _DeeNone_rets1_1  (*(size_t (DCALL *)(void *))&_DeeNone_rets1)
#else /* DCALL_CALLER_CLEANUP */
DFUNDEF ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef1)(void *);
DFUNDEF ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef2)(void *, void *);
DFUNDEF ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef3)(void *, void *, void *);
DFUNDEF ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef4)(void *, void *, void *, void *);
DFUNDEF ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef5)(void *, void *, void *, void *, void *);
DFUNDEF ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef6)(void *, void *, void *, void *, void *, void *);
DFUNDEF ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef7)(void *, void *, void *, void *, void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_rets0_1)(void *);
DFUNDEF size_t (DCALL _DeeNone_rets0_2)(void *, void *);
DFUNDEF size_t (DCALL _DeeNone_rets0_3)(void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_rets0_4)(void *, void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_rets0_5)(void *, void *, void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_rets0_6)(void *, void *, void *, void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_rets1_1)(void *);
DFUNDEF int (DCALL _DeeNone_reti1_2)(void *, void *);
DFUNDEF int (DCALL _DeeNone_reti1_3)(void *, void *, void *);
DFUNDEF int (DCALL _DeeNone_reti1_4)(void *, void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_retsm1_1)(void *);
DFUNDEF size_t (DCALL _DeeNone_retsm1_2)(void *, void *);
DFUNDEF size_t (DCALL _DeeNone_retsm1_3)(void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_retsm1_4)(void *, void *, void *, void *);
DFUNDEF size_t (DCALL _DeeNone_retsm1_5)(void *, void *, void *, void *, void *);
#if defined(DCALL_RETURN_COMMON) || __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define _DeeNone_reti1_1 (*(int (DCALL *)(void *))&_DeeNone_rets1_1)
#define _DeeNone_reti0_1 (*(int (DCALL *)(void *))&_DeeNone_rets0_1)
#define _DeeNone_reti0_2 (*(int (DCALL *)(void *, void *))&_DeeNone_rets0_2)
#define _DeeNone_reti0_3 (*(int (DCALL *)(void *, void *, void *))&_DeeNone_rets0_3)
#define _DeeNone_reti0_4 (*(int (DCALL *)(void *, void *, void *, void *))&_DeeNone_rets0_4)
#define _DeeNone_reti0_5 (*(int (DCALL *)(void *, void *, void *, void *, void *))&_DeeNone_rets0_5)
#else /* DCALL_RETURN_COMMON || __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
DFUNDEF int (DCALL _DeeNone_reti1_1)(void *);
DFUNDEF int (DCALL _DeeNone_reti0_1)(void *);
DFUNDEF int (DCALL _DeeNone_reti0_2)(void *, void *);
DFUNDEF int (DCALL _DeeNone_reti0_3)(void *, void *, void *);
DFUNDEF int (DCALL _DeeNone_reti0_4)(void *, void *, void *, void *);
DFUNDEF int (DCALL _DeeNone_reti0_5)(void *, void *, void *, void *, void *);
#endif /* !DCALL_RETURN_COMMON && __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */
DFUNDEF int (DCALL _DeeNone_reti0_6)(void *, void *, void *, void *, void *, void *);
#endif /* !DCALL_CALLER_CLEANUP */

/* Default no-op constructor callbacks (these should be used by TP_FABSTRACT types) */
#define DeeNone_OperatorCtor     (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define DeeNone_OperatorCopy     (*(int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&_DeeNone_reti0_2)
#define DeeNone_OperatorWriteDec (*(int (DCALL *)(struct Dee_dec_writer *__restrict, DeeObject *__restrict, Dee_dec_addr_t))&_DeeNone_reti0_3)

DECL_END

#endif /* !GUARD_DEEMON_NONE_OPERATOR_H */
