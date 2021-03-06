/* Copyright (c) 2019-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef __GUARD_HYBRID_ATOMIC_H
#define __GUARD_HYBRID_ATOMIC_H 1

#include "../__stdinc.h"
#include "__atomic.h"
#include "host.h"

#ifdef __CC__

#define OATOMIC_LOAD(x, order)                         __hybrid_atomic_load(x, order)
#define OATOMIC_STORE(x, v, order)                     __hybrid_atomic_store(x, v, order)
#define OATOMIC_XCH(x, v, order)                       __hybrid_atomic_xch(x, v, order)
#define OATOMIC_CMPXCH(x, oldv, newv, succ, fail)      __hybrid_atomic_cmpxch(x, oldv, newv, succ, fail)
#define OATOMIC_CMPXCH_WEAK(x, oldv, newv, succ, fail) __hybrid_atomic_cmpxch_weak(x, oldv, newv, succ, fail)
#define OATOMIC_CMPXCH_VAL(x, oldv, newv, succ, fail)  __hybrid_atomic_cmpxch_val(x, oldv, newv, succ, fail)
#define OATOMIC_ADDFETCH(x, v, order)                  __hybrid_atomic_addfetch(x, v, order)
#define OATOMIC_SUBFETCH(x, v, order)                  __hybrid_atomic_subfetch(x, v, order)
#define OATOMIC_ANDFETCH(x, v, order)                  __hybrid_atomic_andfetch(x, v, order)
#define OATOMIC_ORFETCH(x, v, order)                   __hybrid_atomic_orfetch(x, v, order)
#define OATOMIC_XORFETCH(x, v, order)                  __hybrid_atomic_xorfetch(x, v, order)
#define OATOMIC_NANDFETCH(x, v, order)                 __hybrid_atomic_nandfetch(x, v, order)
#define OATOMIC_INCFETCH(x, order)                     __hybrid_atomic_incfetch(x, order)
#define OATOMIC_DECFETCH(x, order)                     __hybrid_atomic_decfetch(x, order)
#define OATOMIC_FETCHADD(x, v, order)                  __hybrid_atomic_fetchadd(x, v, order)
#define OATOMIC_FETCHSUB(x, v, order)                  __hybrid_atomic_fetchsub(x, v, order)
#define OATOMIC_FETCHAND(x, v, order)                  __hybrid_atomic_fetchand(x, v, order)
#define OATOMIC_FETCHOR(x, v, order)                   __hybrid_atomic_fetchor(x, v, order)
#define OATOMIC_FETCHXOR(x, v, order)                  __hybrid_atomic_fetchxor(x, v, order)
#define OATOMIC_FETCHNAND(x, v, order)                 __hybrid_atomic_fetchnand(x, v, order)
#define OATOMIC_FETCHINC(x, order)                     __hybrid_atomic_fetchinc(x, order)
#define OATOMIC_FETCHDEC(x, order)                     __hybrid_atomic_fetchdec(x, order)
#define OATOMIC_ADD(x, v, order)                       __hybrid_atomic_add(x, v, order)
#define OATOMIC_SUB(x, v, order)                       __hybrid_atomic_sub(x, v, order)
#define OATOMIC_AND(x, v, order)                       __hybrid_atomic_and(x, v, order)
#define OATOMIC_OR(x, v, order)                        __hybrid_atomic_or(x, v, order)
#define OATOMIC_XOR(x, v, order)                       __hybrid_atomic_xor(x, v, order)
#define OATOMIC_NAND(x, v, order)                      __hybrid_atomic_nand(x, v, order)
#define OATOMIC_INC(x, order)                          __hybrid_atomic_inc(x, order)
#define OATOMIC_DEC(x, order)                          __hybrid_atomic_dec(x, order)

#define ATOMIC_LOAD(x)                    OATOMIC_LOAD(x, __ATOMIC_SEQ_CST)
#define ATOMIC_STORE(x, v)                OATOMIC_STORE(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_XCH(x, v)                  OATOMIC_XCH(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_CMPXCH(x, oldv, newv)      OATOMIC_CMPXCH(x, oldv, newv, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define ATOMIC_CMPXCH_WEAK(x, oldv, newv) OATOMIC_CMPXCH_WEAK(x, oldv, newv, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define ATOMIC_CMPXCH_VAL(x, oldv, newv)  OATOMIC_CMPXCH_VAL(x, oldv, newv, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define ATOMIC_ADDFETCH(x, v)             OATOMIC_ADDFETCH(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_SUBFETCH(x, v)             OATOMIC_SUBFETCH(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_ANDFETCH(x, v)             OATOMIC_ANDFETCH(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_ORFETCH(x, v)              OATOMIC_ORFETCH(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_XORFETCH(x, v)             OATOMIC_XORFETCH(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_NANDFETCH(x, v)            OATOMIC_NANDFETCH(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_INCFETCH(x)                OATOMIC_INCFETCH(x, __ATOMIC_SEQ_CST)
#define ATOMIC_DECFETCH(x)                OATOMIC_DECFETCH(x, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHADD(x, v)             OATOMIC_FETCHADD(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHSUB(x, v)             OATOMIC_FETCHSUB(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHAND(x, v)             OATOMIC_FETCHAND(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHOR(x, v)              OATOMIC_FETCHOR(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHXOR(x, v)             OATOMIC_FETCHXOR(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHNAND(x, v)            OATOMIC_FETCHNAND(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHINC(x)                OATOMIC_FETCHINC(x, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCHDEC(x)                OATOMIC_FETCHDEC(x, __ATOMIC_SEQ_CST)
#define ATOMIC_ADD(x, v)                  OATOMIC_ADD(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_SUB(x, v)                  OATOMIC_SUB(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_AND(x, v)                  OATOMIC_AND(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_OR(x, v)                   OATOMIC_OR(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_XOR(x, v)                  OATOMIC_XOR(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_NAND(x, v)                 OATOMIC_NAND(x, v, __ATOMIC_SEQ_CST)
#define ATOMIC_INC(x)                     OATOMIC_INC(x, __ATOMIC_SEQ_CST)
#define ATOMIC_DEC(x)                     OATOMIC_DEC(x, __ATOMIC_SEQ_CST)

/* Simplified atomic read/write  functions that only  guaranty correct  ordering
 * of  reads/writes respectively,  as well as  that reads and  writes are always
 * completed  as a whole (i.e. reading a 64-bit value is always done in a single
 * instruction, preventing  the possibility  of some  part of  a value  changing
 * after it had already been read, but before all other parts were read as well) */
#define ATOMIC_READ(x)     OATOMIC_LOAD(x, __ATOMIC_ACQUIRE)
#define ATOMIC_WRITE(x, v) OATOMIC_STORE(x, v, __ATOMIC_RELEASE)

#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_ATOMIC_H */
